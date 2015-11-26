/*
 * Copyright (C) 2008 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Based on minstrel.c:
 *   Copyright (C) 2005-2007 Derek Smithies <derek@indranet.co.nz>
 *   Sponsored by Indranet Technologies Ltd
 *
 * Based on sample.c:
 *   Copyright (c) 2005 John Bicket
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer,
 *      without modification.
 *   2. Redistributions in binary form must reproduce at minimum a disclaimer
 *      similar to the "NO WARRANTY" disclaimer below ("Disclaimer") and any
 *      redistribution must be conditioned upon including a substantially
 *      similar Disclaimer requirement for further binary redistribution.
 *   3. Neither the names of the above-listed copyright holders nor the names
 *      of any contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *   Alternatively, this software may be distributed under the terms of the
 *   GNU General Public License ("GPL") version 2 as published by the Free
 *   Software Foundation.
 *
 *   NO WARRANTY
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
 *   AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 *   THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
 *   OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 *   IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *   THE POSSIBILITY OF SUCH DAMAGES.
 */
#include <linux/netdevice.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/debugfs.h>
#include <linux/random.h>
#include <linux/moduleparam.h>
#include <linux/ieee80211.h>
#include <net/mac80211.h>
#include "rate.h"
#include "rc80211_indra.h"
#include "rc80211_indra_ht.h"

/* external logging */
extern data_rclog_t data_rclog_buf[RCLOG_BUF_DIM];
extern int rclog_in;

/* signals memory */
extern signals_memory signals[SIGNALS_MEMORY_SIZE];
extern int signals_index;

/* optimization logging */
extern data_optlog_t data_optlog_buf[OPTLOG_BUF_DIM];
extern int optlog_in;

/* get rtdsc register value */
static __inline__ unsigned long long rdtsc(void) {
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

/* number of data bits per symbol */
const unsigned int nbps[] = {54, 108, 162, 216, 324, 432, 486, 540};

/* per vs SNR for different MCSs */
const unsigned int per_mcs_0[] = {1000,981,893,688,424,244,166,125,101,70,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
const unsigned int per_mcs_1[] = {1000,965,890,774,599,351,218,187,174,153,130,70,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
const unsigned int per_mcs_2[] = {1000,1000,975,831,576,354,239,221,213,171,126,62,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
const unsigned int per_mcs_3[] = {1000,1000,1000,1000,995,895,677,445,315,259,199,161,129,79,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
const unsigned int per_mcs_4[] = {1000,1000,1000,1000,1000,1000,1000,991,895,689,459,335,316,292,200,143,117,93,51,12,0,0,0,0,0,0,0,0,0,0};
const unsigned int per_mcs_5[] = {1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,999,954,831,621,395,231,149,140,135,130,125,120,90,70,19,0,0,0,0,0};
const unsigned int per_mcs_6[] = {1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,982,908,735,488,291,234,228,203,178,149,122,97,51,11,0,0,0,0};
const unsigned int per_mcs_7[] = {1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,955,826,606,369,240,183,150,142,140,120,85,23,0,0,0};

static void
explore_stages(struct indra_priv *ip, struct indra_sta_info *ii, int snr,
			unsigned int *rates_old, int i_max,
			unsigned int *found, unsigned int **rates_min,
			unsigned int *loss_min, int stage)
{
	int i, j;
	unsigned int sum1=0, sum2=0, sum_st=0;
	unsigned int loss = 1000;
	
	/* Create rates array, sums and products*/
	unsigned int rates[stage];
	for (i=0;i<stage-1;i++) {
		struct indra_rate *ir = &ii->r[rates_old[i]-1];
		rates[i] = rates_old[i];
		sum1 = sum1 + rates[i];
		sum2 = sum2 + rates_min[stage-1][i];
		sum_st = sum_st + ir->attempts[i];
		loss = INDRA_TRUNC(INDRA_FRAC(loss*ir->per_snr_map[snr],1000));
	}	
	sum2 = sum2 + rates_min[stage-1][stage-1];
	//printk("MICHELE: explore: stage=%i, found[stage-1]=%u, loss_min[stage-1]=%u, rates[stage-2]=%u\n",stage,found[stage-1],loss_min[stage-1],rates[stage-2]);
	
	/* Main loop */
	for (i = 0; i<=i_max; i++) {
		//if ((found[stage-1] == 0) || (sum1+i+1<=sum2)) {
			struct indra_rate *ir = &ii->r[i];
			if (sum_st + ir->attempts[stage-1] <= ip->st_deadline) {
				loss = loss*ir->per_snr_map[snr];
				if (loss<=loss_min[stage-1]) {
					found[stage-1] = 1;
					loss_min[stage-1] = loss;
					for (j = 0; j<stage-1; j++) 
						rates_min[stage-1][j] = rates[j];
					rates_min[stage-1][stage-1] = i+1;
				}
			}
		//}
		rates[stage-1] = i+1;
		if (stage < ip->max_retry) 
			explore_stages(ip,ii,snr,rates,i,found,rates_min,loss_min,stage+1);
		rates[stage-1] = 0;
	}
}

static void
indra_ht_update_rates(struct indra_priv *ip, struct indra_sta_info *ii)
{
	unsigned int *found;
	unsigned int *loss_min;
	unsigned int **rates_min;
	unsigned int rates[1];
	unsigned int loss;
	int i,j, idx, snr;

	/* Allocate and initialize temporary arrays */
	found = kzalloc(ip->max_retry*sizeof(*found),GFP_ATOMIC);
	loss_min = kzalloc(ip->max_retry*sizeof(*loss_min),GFP_ATOMIC);
	rates_min = kzalloc(ip->max_retry*sizeof(*rates_min),GFP_ATOMIC);
	for (i = 0; i < ip->max_retry; i++) {
		rates_min[i] = kzalloc(ip->max_retry*sizeof(*rates_min[i]),GFP_ATOMIC);
		loss_min[i] = UINT_MAX;
	}
		
	/* Optimize for all SNR values */
	for (snr=0;snr<SNR_RANGE;snr++) {
		
		/* Optimization for this SNR*/	
		for (i = 0; i < ii->n_rates; i++) {
			struct indra_rate *ir = &ii->r[i];
			/* Check service time bound */
			if (ir->attempts[0]>ip->st_deadline) 
				continue;
			loss = ir->per_snr_map[snr];
			if (loss<=loss_min[0]) {
				/* Minimal combination found */
				found[0] = 1;
				rates_min[0][0] = i+1;
				loss_min[0] = loss;
			}
			/* Explore following stages */
			rates[0] = i+1;
			explore_stages(ip,ii,snr,rates,i,found,rates_min,loss_min,2);
		}

		/* Find optimal index */
		loss = UINT_MAX;
		idx = 0;
		for (i = 0; i<ip->max_retry; i++) {
			if ((found[i] != 0) && (loss_min[i]<loss)) { 
				idx = i;
				loss = loss_min[i];
			}
		}
		
		/* Save optimization results */
		ii->optimal_indexes[snr] = idx;
		for (j = 0; j<=idx; j++) {
			ii->optimal_rates[snr][j] = rates_min[idx][j];
		} 
		
		/* Reinitialize temporary arrays */
		for (i = 0; i < ip->max_retry; i++) {
			found[i] = 0;
			loss_min[i] = UINT_MAX;
			for (j = 0; j < ip->max_retry; j++) {
				rates_min[i][j] = 0;
			}
		}
	}
	
	/* Save last update */
	ii->last_stats_update = jiffies;
	
	/* Free memory */
	kfree(found);
	kfree(loss_min);
	for (i = 0; i < ip->max_retry; i++) {
		kfree(rates_min[i]);
	}
	kfree(rates_min);
	
}

static void

indra_ht_tx_status(void *priv, struct ieee80211_supported_band *sband,
		   struct ieee80211_sta *sta, void *priv_sta,
		   struct sk_buff *skb)
{
	struct indra_sta_info *ii = priv_sta;
	struct indra_priv *ip = priv;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	struct ieee80211_tx_rate *ar = info->status.rates;
	struct ieee80211_hdr *hdr = (void *)skb->data;
	u8 dst[ETH_ALEN];
	bool found = false;
	int i, s, count, record, r = 0, ndx = 0, snr_index = SNR_RANGE-1;			
	int success, null;

	/* Check transmission status: successful or not */
	success = !!(info->flags & IEEE80211_TX_STAT_ACK);
	
	/* Check if this is a null transmission: if so, exit */
	null = !!(info->flags & IEEE80211_TX_CTL_USE_MINRATE);
	if (null)
	  return;
	
	/* Reset number of attempts and successes for each rate */
	for (i = 0; i < ii->n_rates; i++) {
		ii->r[i].last_attempts = 0;
		ii->r[i].last_success = 0;
	}
	
	/* Initialize new entry in rclog character device */  
	data_rclog_buf[rclog_in].success = success;
	data_rclog_buf[rclog_in].probe = 0;  
	for (i = 0; i < 4; i++) {
		data_rclog_buf[rclog_in].rate_idx[i] = -1;
		data_rclog_buf[rclog_in].rate_count[i] = -1;
	}
	
	/* Check used rates */
	for (i = 0; i < IEEE80211_TX_MAX_RATES; i++) {			
		/* Retrieve used rate */
		if (ar[i].idx < 0)
			break;
		ndx = ar[i].idx;
		if (ndx < 0)
			continue;
		
		/* Retrieve and update number of attempts
		 * for the retrieved rate */
		count = ar[i].count;
		ii->r[ndx].last_attempts = ar[i].count;
		
		/* Retrieve maximum SNR index with PER over the 
		 * threshold for the retrieved rate */
		for (s = snr_index; s >= 0; s--) {
			if (ii->r[ndx].per_snr_map[s] >= PER_TH_INDRA) {
				snr_index = s;
				break;
			}
		} 
		
		/* Write in the rclog device */		
		data_rclog_buf[rclog_in].rate_idx[r] = ndx;
		data_rclog_buf[rclog_in].rate_count[r] = count;
		r++;				
		
		/* Update number of successes for the 
		 * retrieved rate */ 
		if ((i != IEEE80211_TX_MAX_RATES - 1) && (ar[i + 1].idx < 0))
			ii->r[ndx].last_success += success;
	}
	
	/* Point to new entry in rclog character device */
	rclog_in = (rclog_in+1)%RCLOG_BUF_DIM;
		
	/* Update estimated SNR */	
	ii->last_snr = snr_index;
	
	/* Retrieve receiver and update status (waiting for packet
	 * or not)*/
	memcpy(dst, ieee80211_get_DA(hdr), ETH_ALEN);
	for (record = 0; record < signals_index; record++) {
		found = true;
		for (i = 0; i < ETH_ALEN; i++){
			if (signals[record].address[i] != dst[i]) {
				found = false;  
				break;
			}
		}
		if (found) break;
	}
	if (found) {
		signals[record].status = success;
	}
	
	//* Update rates if timer has expired */
	if (time_after(jiffies, ii->last_stats_update +
				(ip->update_interval / 2 * HZ) / 1000)) {
		indra_ht_update_rates(ip, ii);
	}

}

static void
indra_ht_get_rate(void *priv, struct ieee80211_sta *sta,
		  void *priv_sta, struct ieee80211_tx_rate_control *txrc)
{
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(txrc->skb);
	struct indra_priv *ip = priv;
	struct indra_sta_info *ii = priv_sta;
	struct sk_buff *skb = txrc->skb;
	struct ieee80211_hdr *hdr = (void *)skb->data;
	struct ieee80211_sta_rates *ratetbl;
	u8 dst[ETH_ALEN];
	int record, i, status = 0, snr;
	unsigned int per, previous_per;
	bool found = false;
	char* p_data;
	
	/* management/no-ack frames do not use rate control */
	if (rate_control_send_low(sta, priv_sta, txrc))
		return;
		
	/* log start time */
	data_optlog_buf[optlog_in].start = rdtsc();
	data_optlog_buf[optlog_in].optimized = 0;
		
	/* Allocate rate table */
	ratetbl = kzalloc(sizeof(*ratetbl), GFP_ATOMIC);
	if (!ratetbl)
		return;
	
	/* Search for entry in signals memory from the same 
	 * node that this packet is destined to */
	memcpy(dst, ieee80211_get_DA(hdr), ETH_ALEN);
	for (record=0; record<signals_index; record++) {
		found = true;
		//printk("MICHELE: d=%2x, r=%2x, ",dst[0],signals[record].address[0]);
		for (i=0; i<ETH_ALEN; i++){
			if (signals[record].address[i] != dst[i]) {
				found = false;  
				break;
			}
		}
		if (found) break;
	}
	//printk("\n");
	
	/* Write received signals data on the new packet and retrieve 
	 * SNR index at the destination node */
	p_data = (char*)skb->data;
    p_data = p_data + 42;
	if (found) {
		p_data[0] = signals[record].rx_signal;
		p_data[1] = signals[record].rx_noise;
		status = signals[record].status;
		if ((status == 2) && (signals[record].tx_signal != 0)) {
			snr = signals[record].tx_signal - signals[record].tx_noise;
			ii->last_snr = snr - SNR_MIN;
			if (ii->last_snr < 0)
				ii->last_snr = 0;
			if (ii->last_snr > SNR_RANGE - 1)
				ii->last_snr = SNR_RANGE - 1;
			//printk("MICHELE: tx sig: %i, tx noise: %i, SNR index = %i\n",signals[record].tx_signal,signals[record].tx_noise,ii->last_snr);
		}
	}
	
	/* Update SNR-PER map */
	if ((status == 2) && (signals[record].tx_signal != 0)) {
		for (i = 0; i < ii->n_rates; i++) {
			if (ii->r[i].last_attempts != 0) {
				per = INDRA_TRUNC(INDRA_FRAC(1000*(ii->r[i].last_attempts-ii->r[i].last_success),ii->r[i].last_attempts));
				previous_per = ii->r[i].per_snr_map[ii->last_snr];
				ii->r[i].per_snr_map[ii->last_snr] = INDRA_TRUNC(INDRA_FRAC(ALPHA_INDRA*previous_per + (100-ALPHA_INDRA)*per,100));
				//printk("MICHELE: per = %u, previous per = %u, new per = %u\n",per,previous_per,ii->r[i].per_snr_map[ii->last_snr]);
				ii->r[i].last_attempts = 0;
				ii->r[i].last_success = 0;
			}
		}
	}
		
	/* Enable STBC if possible */
	info->flags |= ii->tx_flags;
	
	/* Write rate table */
	for (i = 0; i<=ii->optimal_indexes[ii->last_snr]; i++) {
		struct indra_rate *ir = &ii->r[ii->optimal_rates[ii->optimal_indexes[ii->last_snr]][i]-1];
		ratetbl->rate[i].idx = ir->rix;
		ratetbl->rate[i].count = 1;
		ratetbl->rate[i].count_cts = 1;
		ratetbl->rate[i].count_rts = 1;
		ratetbl->rate[i].flags = ii->flags;
	}
	rate_control_set_rates(ip->hw, ii->sta, ratetbl);
	
	/* log final time */
	data_optlog_buf[optlog_in].stop = rdtsc();
	optlog_in = (optlog_in+1) % OPTLOG_BUF_DIM;
}

static void
calc_service_time_values(void *priv, enum ieee80211_band band,
		    struct indra_rate *r,
		    struct cfg80211_chan_def *chandef)
{
	struct indra_priv *ip = priv;
	unsigned int t_slot = 9; 								/* FIXME: get real slot time */
	int i;
	unsigned int pow[6] = {1,2,4,8,16,32};	
	
	r->perfect_tx_time = (DIV_ROUND_UP(ip->ref_payload_size*8,nbps[r->rix]) * 4) + ieee80211_frame_duration(band, 0, 60, 1, 1, 0);
	r->ack_time = ieee80211_frame_duration(band, 10, 60, 1, 1, 0);	/* this is not really correct....*/
	
	for (i=0; i<7; i++) {
		if (i==0) {
			r->attempts[i] = 2*t_slot + r->perfect_tx_time + r->ack_time;
		}
		else {
			r->attempts[i] = 2*t_slot + r->perfect_tx_time + r->ack_time + t_slot*(pow[i-1]*(ip->cw_min+1)-1);
		} 
		//printk("MICHELE: rate %i (d=%u, a=%u), attempt %i, time %u\n",r->rix,r->perfect_tx_time,r->ack_time,i,r->attempts[i]);
	}
}

static void
initialize_map(struct indra_rate *r,
			int idx,
		    struct cfg80211_chan_def *chandef)
{
	int i;
	for (i=0; i < SNR_RANGE; i++) {
		//r->per_snr_map[i] = 0;			/* Original definition
		switch (r->rix) {
			case 0:
				r->per_snr_map[i] = per_mcs_0[i];
				break;
			case 1:
				r->per_snr_map[i] = per_mcs_1[i];
				break;
			case 2:
				r->per_snr_map[i] = per_mcs_2[i];
				break;
			case 3:
				r->per_snr_map[i] = per_mcs_3[i];
				break;
			case 4:
				r->per_snr_map[i] = per_mcs_4[i];
				break;
			case 5:
				r->per_snr_map[i] = per_mcs_5[i];
				break;
			case 6:
				r->per_snr_map[i] = per_mcs_6[i];
				break;
			case 7:
				r->per_snr_map[i] = per_mcs_7[i];
				break;
			}	
	}
}

static void
indra_ht_rate_init(void *priv, struct ieee80211_supported_band *sband,
		   struct cfg80211_chan_def *chandef,
		   struct ieee80211_sta *sta, void *priv_sta)
{
	struct indra_sta_info *ii = priv_sta;
	struct indra_priv *ip = priv;
	u16 sta_cap = sta->ht_cap.cap;
	unsigned int i, s, st = 0,stbc;
	
	/* fall back to the old indra for legacy stations */
	if (!sta->ht_cap.ht_supported)
		goto use_legacy;
		
	/* Check that 40 MHz is supported */
	if (sta->bandwidth < IEEE80211_STA_RX_BW_40)
		goto use_legacy;
		
	/* Check for STBC and LDPC capabilities and update tx flags */
	stbc = (sta_cap & IEEE80211_HT_CAP_RX_STBC) >>
			IEEE80211_HT_CAP_RX_STBC_SHIFT;
	ii->tx_flags |= stbc << IEEE80211_TX_CTL_STBC_SHIFT;

	if (sta_cap & IEEE80211_HT_CAP_LDPC_CODING)
		ii->tx_flags |= IEEE80211_TX_CTL_LDPC;
				
	/* Initializations */	
	ii->sta = sta;
	ii->lowest_rix = rate_lowest_index(sband, sta);
	ii->n_rates = MCS_GROUP_RATES;
	ii->last_snr = 0;
	
	/* Define flags: 
	 * IEEE80211_TX_RC_MCS -> if present use HT rates
	 * IEEE80211_TX_RC_40_MHZ_WIDTH -> if present use 40MHz channels (otherwise 20MHz ones)
	 * IEEE80211_TX_RC_SHORT_GI -> if present use short GI (otherwise standard GI)
	 * */
	ii->flags = IEEE80211_TX_RC_MCS | IEEE80211_TX_RC_40_MHZ_WIDTH;
	
	/* Allocate rates */
	for (i = 0; i < MCS_GROUP_RATES; i++) {
		struct indra_rate *ir = &ii->r[i];
		memset(ir, 0, sizeof(*ir));
		ir->rix = i;
		ir->last_attempts = 0;
		ir->last_success = 0;
		ir->bitrate = 0;
		
		calc_service_time_values(ip, sband->band, ir, chandef);
		initialize_map(ir,i,chandef);
	}
	
	/* Check if max retry is compliant with constraint on service time */
	for (s = 1; s <= ip->max_retry; s++) {
		struct indra_rate *ir = &ii->r[MCS_GROUP_RATES-1];
		st = st + ir->attempts[s-1];
		if (st>ip->st_deadline) {
			ip->max_retry = s - 1;
			break;
		}
	}
	//ip->max_retry=1;
	//printk("MICHELE: max_retry=%i, n_rates=%i\n",ip->max_retry,ii->n_rates);
	
	/* Initialize optimal rates array */
	for (i = 0; i < SNR_RANGE; i++) 
		ii->optimal_rates[i] = kzalloc(ip->max_retry*sizeof(*ii->optimal_rates[i]),GFP_ATOMIC);

	/* Update */
	indra_ht_update_rates(ip, ii);
	
	return;
	
use_legacy:
	return mac80211_indra.rate_init(priv, sband, chandef, sta,
					   priv_sta);
}

static void *
indra_ht_alloc_sta(void *priv, struct ieee80211_sta *sta, gfp_t gfp)
{
	struct ieee80211_supported_band *sband;
	struct indra_sta_info *ii;
	struct indra_priv *ip = priv;
	struct ieee80211_hw *hw = ip->hw;
	int max_rates = 0;
	int i;

	ii = kzalloc(sizeof(struct indra_sta_info), gfp);
	if (!ii)
		return NULL;
	
	for (i = 0; i < IEEE80211_NUM_BANDS; i++) {
		sband = hw->wiphy->bands[i];
		if (sband && sband->n_bitrates > max_rates)
			max_rates = sband->n_bitrates;
	}

	ii->r = kzalloc(sizeof(struct indra_rate) * max_rates, gfp);
	if (!ii->r)
		goto error;
		
	ii->optimal_indexes = kzalloc(SNR_RANGE*sizeof(*ii->optimal_indexes),GFP_ATOMIC);
	ii->optimal_rates = kzalloc(SNR_RANGE*sizeof(*ii->optimal_rates),GFP_ATOMIC);
	if ((!ii->optimal_indexes) || (!ii->optimal_rates))
		goto error1;
		
	ii->last_stats_update = jiffies;
	return ii;
	
error1:
	kfree(ii->r);
error:
	kfree(ii);
	return NULL;
}

static void
indra_ht_free_sta(void *priv, struct ieee80211_sta *sta, void *priv_sta)
{
	struct indra_sta_info *ii = priv_sta;
	struct indra_priv *ip = priv;
	int i;
	
	kfree(ii->optimal_indexes);
	for (i = 0; i < ip->max_retry; i++) {
		kfree(ii->optimal_rates[i]);
	}
	kfree(ii->optimal_rates);
	kfree(ii->r);
	kfree(ii);
}

static void *
indra_ht_alloc(struct ieee80211_hw *hw, struct dentry *debugfsdir)
{
	struct indra_priv *ip;

	ip = kzalloc(sizeof(struct indra_priv), GFP_ATOMIC);
	if (!ip)
		return NULL;
		
	ip->cw_min = 15;

	if (hw->max_rate_tries > 0)
		ip->max_retry = hw->max_rate_tries;
	else
		/* safe default, does not necessarily have to match hw properties */
		ip->max_retry = 7;

	if (hw->max_rates >= 4)
		ip->has_mrr = true;

	ip->hw = hw;
	
#ifdef CPTCFG_MAC80211_DEBUGFS
	ip->st_deadline = (u32) 2000;
	ip->dbg_st_deadline = debugfs_create_u32("deadline",
			S_IRUGO | S_IWUGO, debugfsdir, &ip->st_deadline);
	ip->ref_payload_size = (u32) 50;											
	ip->dbg_ref_payload_size = debugfs_create_u32("ref_payload_size",			
			S_IRUGO | S_IWUGO, debugfsdir, &ip->ref_payload_size);	
	ip->update_interval = (u32) 100;											
	ip->dbg_update_interval = debugfs_create_u32("update_interval",			
			S_IRUGO | S_IWUGO, debugfsdir, &ip->update_interval);				
#endif
	return ip;
}

static void
indra_ht_free(void *priv)
{
	kfree(priv);
}

static u32 indra_ht_get_expected_throughput(void *priv_sta)
{
	u32 usecs;
	u32 tp;

	usecs = 1000000;
			
	tp = (1000000 / usecs);
	return INDRA_TRUNC(tp) * 1200 * 8 / 1024;
	
}

static const struct rate_control_ops mac80211_indra_ht = {
	.name = "indra_ht",
	.tx_status = indra_ht_tx_status,
	.get_rate = indra_ht_get_rate,
	.rate_init = indra_ht_rate_init,
	.alloc = indra_ht_alloc,
	.free = indra_ht_free,
	.alloc_sta = indra_ht_alloc_sta,
	.free_sta = indra_ht_free_sta,
#ifdef CPTCFG_MAC80211_DEBUGFS
	.add_sta_debugfs = indra_add_sta_debugfs,
	.remove_sta_debugfs = indra_remove_sta_debugfs,
#endif
	.get_expected_throughput = indra_ht_get_expected_throughput,
};

int __init
rc80211_indra_ht_init(void)
{
	return ieee80211_rate_control_register(&mac80211_indra_ht);
}

void
rc80211_indra_ht_exit(void)
{
	ieee80211_rate_control_unregister(&mac80211_indra_ht);
}

