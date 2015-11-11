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
#include <linux/ieee80211.h>
#include <linux/slab.h>
#include <net/mac80211.h>
#include "rate.h"
#include "rc80211_indra.h"

/* MICHELE **********************************************/

extern data_rclog_t data_rclog_buf[RCLOG_BUF_DIM];
extern int rclog_in;
extern signals_memory signals[SIGNALS_MEMORY_SIZE];
extern int signals_index;
/* MICHELE **********************************************/

/* convert mac80211 rate index to local array index */
static inline int
rix_to_ndx(struct indra_sta_info *ii, int rix)
{
	int i = rix;
	for (i = rix; i >= 0; i--)
		if (ii->r[i].rix == rix)
			break;
	return i;
}

static void
explore_stages(struct indra_priv *ip, struct indra_sta_info *ii,
			unsigned int *rates_old, int i_max,
			unsigned int *found, unsigned int **rates_min,
			unsigned int *loss_min, int stage)
{
	int i, j;
	unsigned int sum1=0, sum2=0, sum_st=0;
	unsigned int loss = 1;
	
	/* Create rates array, sums and products*/
	unsigned int rates[stage];
	for (i=0;i<stage-1;i++) {
		struct indra_rate *ir = &ii->r[rates_old[i]-1];
		rates[i] = rates_old[i];
		sum1 = sum1 + rates[i];
		sum2 = sum2 + rates_min[stage-1][i];
		sum_st = sum_st + ir->attempts[i];
		loss = loss*ir->per_snr_map[ii->last_snr];
	}	
	sum2 = sum2 + rates_min[stage-1][stage-1];
	//printk("MICHELE: explore: stage=%i, found[stage-1]=%u, loss_min[stage-1]=%u, rates[stage-2]=%u\n",stage,found[stage-1],loss_min[stage-1],rates[stage-2]);
	
	/* Main loop */
	for (i = 0; i<=i_max; i++) {
		//if ((found[stage-1] == 0) || (sum1+i+1<=sum2)) {
			struct indra_rate *ir = &ii->r[i];
			if (sum_st + ir->attempts[stage-1] <= ip->st_deadline) {
				loss = loss*ir->per_snr_map[ii->last_snr];
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
			explore_stages(ip,ii,rates,i,found,rates_min,loss_min,stage+1);
		rates[stage-1] = 0;
	}
}

static void
indra_update_rates(struct indra_priv *ip, struct indra_sta_info *ii)
{
	struct ieee80211_sta_rates *ratetbl;
	unsigned int *found;
	unsigned int *loss_min;
	unsigned int **rates_min;
	unsigned int rates[1];
	unsigned int loss;
	int i,j;

	/* Allocate rate table */
	ratetbl = kzalloc(sizeof(*ratetbl), GFP_ATOMIC);
	if (!ratetbl)
		return;
		
	/* Before snr is initialized */	
	if (!ii->last_snr) {
		/* Use lowest rate always */
		ratetbl->rate[0].idx = ii->lowest_rix;
		ratetbl->rate[0].count = ip->max_retry;
		ratetbl->rate[0].count_cts = ip->max_retry;
		ratetbl->rate[0].count_rts = ip->max_retry;

		rate_control_set_rates(ip->hw, ii->sta, ratetbl);
		return;
	}
	
	/* Allocate and initialize temporary arrays */
	found = kzalloc(ip->max_retry*sizeof(*found),GFP_ATOMIC);
	loss_min = kzalloc(ip->max_retry*sizeof(*loss_min),GFP_ATOMIC);
	rates_min = kzalloc(ip->max_retry*sizeof(*rates_min),GFP_ATOMIC);
	for (i = 0; i < ip->max_retry; i++) {
		rates_min[i] = kzalloc(ip->max_retry*sizeof(*rates_min[i]),GFP_ATOMIC);
		loss_min[i] = UINT_MAX;
	}
		
	printk("MICHELE: opt_started\n");	
	/* Optimization */	
	for (i = 0; i < ii->n_rates; i++) {
		struct indra_rate *ir = &ii->r[i];
		/* Check service time bound */
		if (ir->attempts[0]>ip->st_deadline) 
			continue;
		loss = ir->per_snr_map[ii->last_snr];
		if (loss<=loss_min[0]) {
			/* Minimal combination found */
			found[0] = 1;
			rates_min[0][0] = i+1;
			loss_min[0] = loss;
		}
		/* Explore following stages */
		rates[0] = i+1;
		explore_stages(ip,ii,rates,i,found,rates_min,loss_min,2);
	}
	printk("MICHELE: opt_ended\n");	
	//printk("MICHELE: min_rate(1) = %u\n",rates_min[0][0]);
	//printk("MICHELE: min_rate(2) = %u,%u\n",rates_min[1][0],rates_min[1][1]);
	//printk("MICHELE: min_rate(3) = %u,%u,%u\n",rates_min[2][0],rates_min[2][1],rates_min[2][2]);
	//printk("MICHELE: min_rate(4) = %u,%u,%u,%u\n",rates_min[3][0],rates_min[3][1],rates_min[3][2],rates_min[3][3]);
	//printk("MICHELE: min_rate(5) = %u,%u,%u,%u,%u\n",rates_min[4][0],rates_min[4][1],rates_min[4][2],rates_min[4][3],rates_min[4][4]);
	//printk("MICHELE: min_rate(6) = %u,%u,%u,%u,%u,%u\n",rates_min[5][0],rates_min[5][1],rates_min[5][2],rates_min[5][3],rates_min[5][4],rates_min[5][5]);
	//printk("MICHELE: min_rate(7) = %u,%u,%u,%u,%u,%u,%u\n",rates_min[6][0],rates_min[6][1],rates_min[6][2],rates_min[6][3],rates_min[6][4],rates_min[6][5],rates_min[6][6]);
	
	/* Save optimization results in ratetbl */
	for (i = 0; i<ip->max_retry; i++) {
		if (found[i] == 0)
			break;
	}
	for (j = 0; j<i; j++) {
		struct indra_rate *ir = &ii->r[rates_min[i-1][j]-1];
		ratetbl->rate[j].idx = ir->rix;
		ratetbl->rate[j].count = 1;
		ratetbl->rate[j].count_cts = 1;
		ratetbl->rate[j].count_rts = 1;
	}
	rate_control_set_rates(ip->hw, ii->sta, ratetbl);
	
	/* Free memory */
	kfree(found);
	kfree(loss_min);
	for (i = 0; i < ip->max_retry; i++) {
		kfree(rates_min[i]);
	}
	kfree(rates_min);
	
}

static void
indra_tx_status(void *priv, struct ieee80211_supported_band *sband,
		   struct ieee80211_sta *sta, void *priv_sta,
		   struct sk_buff *skb)
{
	struct indra_sta_info *ii = priv_sta;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	struct ieee80211_tx_rate *ar = info->status.rates;
	struct ieee80211_hdr *hdr = (void *)skb->data;
	u8 dst[ETH_ALEN];
	bool found = false;
	int i, s, count, record, r = 0, ndx = 0, snr_index = SNR_RANGE-1;			
	int success, null;
	unsigned int per_threshold = INDRA_FRAC(PER_TH_INDRA,100);

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
		ndx = rix_to_ndx(ii, ar[i].idx);
		if (ndx < 0)
			continue;
		
		/* Retrieve and update number of attempts
		 * for the retrieved rate */
		count = ar[i].count;
		ii->r[ndx].last_attempts = ar[i].count;
		
		/* Retrieve maximum SNR index with PER over the 
		 * threshold for the retrieved rate */
		for (s = snr_index; s >= 0; s--) {
			if (ii->r[ndx].per_snr_map[s] >= per_threshold) {
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

}

static void
indra_get_rate(void *priv, struct ieee80211_sta *sta,
		  void *priv_sta, struct ieee80211_tx_rate_control *txrc)
{
	struct indra_priv *ip = priv;
	struct indra_sta_info *ii = priv_sta;
	struct sk_buff *skb = txrc->skb;
	struct ieee80211_hdr *hdr = (void *)skb->data;
	u8 dst[ETH_ALEN];
	int record, i, status = 0, snr;
	unsigned int per, previous_per;
	bool found = false;
	char* p_data;
	
	/* Search for entry in signals memory from the same 
	 * node that this packet is destined to */
	memcpy(dst, ieee80211_get_DA(hdr), ETH_ALEN);
	for (record=0; record<signals_index; record++) {
		found = true;
		for (i=0; i<ETH_ALEN; i++){
			if (signals[record].address[i] != dst[i]) {
				found = false;  
				break;
			}
		}
		if (found) break;
	}
	
	/* Write received signals data on the new packet and retrieve 
	 * SNR index at the destination node */
	p_data = (char*)skb->data;
    p_data = p_data + 42;
	if (found) {
		p_data[0] = signals[record].rx_signal;
		p_data[1] = signals[record].rx_noise;
		status = signals[record].status;
		if (status == 2) {
			snr = signals[record].tx_signal - signals[record].tx_noise;
			ii->last_snr = snr + SNR_MIN;
			if (ii->last_snr < 0)
				ii->last_snr = 0;
			if (ii->last_snr > SNR_RANGE - 1)
				ii->last_snr = SNR_RANGE - 1;
			printk("MICHELE: SNR index = %i\n",ii->last_snr);
		}
	}
	
	/* Update SNR-PER map */
	if (status == 2) {
		for (i = 0; i < ii->n_rates; i++) {
			if (ii->r[i].last_attempts != 0) {
				per = INDRA_TRUNC(INDRA_FRAC(ii->r[i].last_attempts-ii->r[i].last_success,ii->r[i].last_attempts));
				previous_per = ii->r[i].per_snr_map[ii->last_snr];
				ii->r[i].per_snr_map[ii->last_snr] = INDRA_TRUNC(INDRA_FRAC(ALPHA_INDRA,100)*previous_per) + INDRA_TRUNC(INDRA_FRAC(100-ALPHA_INDRA,100)*per);
				printk("MICHELE: per = %u, previous per = %u, new per = %u\n",per,previous_per,ii->r[i].per_snr_map[ii->last_snr]);
				ii->r[i].last_attempts = 0;
				ii->r[i].last_success = 0;
			}
		}
	}
	
	/* management/no-ack frames do not use rate control */
	if (rate_control_send_low(sta, priv_sta, txrc))
		return;
		
	/* Rate selection algorithm */
	indra_update_rates(ip, ii);

}

static void
calc_service_time_values(void *priv, enum ieee80211_band band,
		    struct indra_rate *r,
		    struct ieee80211_rate *rate,
		    struct cfg80211_chan_def *chandef)
{
	struct indra_priv *ip = priv;
	unsigned int t_slot = 9; 								/* FIXME: get real slot time */
	unsigned int t_sifs = 10;								/* FIXME: get real SIFS time */
	unsigned int t_difs = 2*t_slot + t_sifs;
	int erp = !!(rate->flags & IEEE80211_RATE_ERP_G);
	int shift = ieee80211_chandef_get_shift(chandef);
	int i;
	unsigned int pow[6] = {1,2,4,8,16,32};	
	
	r->perfect_tx_time = ieee80211_frame_duration(band, ip->ref_payload_size,
			DIV_ROUND_UP(rate->bitrate, 1 << shift), erp, 1, shift);
	r->ack_time = ieee80211_frame_duration(band, 10,		/* this is not really correct....*/
			DIV_ROUND_UP(rate->bitrate, 1 << shift), erp, 1, shift);
	
	for (i=0; i<7; i++) {
		if (i==0) {
			r->attempts[i] = t_difs + r->perfect_tx_time + t_sifs + r->ack_time;
		}
		else {
			r->attempts[i] = t_difs + r->perfect_tx_time + t_sifs + r->ack_time + t_slot*(pow[i-1]*(ip->cw_min+1)-1);
		} 
		printk("MICHELE: rate %i (d=%u, a=%u), attempt %i, time %u\n",r->rix,r->perfect_tx_time,r->ack_time,i,r->attempts[i]);
	}
}

static void
initialize_map(struct indra_rate *r,
			struct ieee80211_rate *rate,
		    struct cfg80211_chan_def *chandef)
{
	//int shift = ieee80211_chandef_get_shift(chandef);
	int i;
	for (i=0; i < SNR_RANGE; i++) {
		r->per_snr_map[i] = 0;			/* TO DO: BETTER DEFINITION */
	}
}

static void
indra_rate_init(void *priv, struct ieee80211_supported_band *sband,
		   struct cfg80211_chan_def *chandef,
		   struct ieee80211_sta *sta, void *priv_sta)
{
	struct indra_sta_info *ii = priv_sta;
	struct indra_priv *ip = priv;
	struct ieee80211_rate *ctl_rate;
	unsigned int i, n = 0, s, st = 0;
	u32 rate_flags;

	ii->sta = sta;
	ii->lowest_rix = rate_lowest_index(sband, sta);
	ctl_rate = &sband->bitrates[ii->lowest_rix];

	rate_flags = ieee80211_chandef_rate_flags(&ip->hw->conf.chandef);

	for (i = 0; i < sband->n_bitrates; i++) {
		struct indra_rate *ir = &ii->r[n];

		if (!rate_supported(sta, sband->band, i))
			continue;
		if ((rate_flags & sband->bitrates[i].flags) != rate_flags)
			continue;

		n++;
		
		memset(ir, 0, sizeof(*ir));
		ir->rix = i;
		ir->last_attempts = 0;
		ir->last_success = 0;
		
		calc_service_time_values(ip,sband->band, ir, &sband->bitrates[i],
				    chandef);
		initialize_map(ir,&sband->bitrates[i],chandef);
	}

	for (i = n; i < sband->n_bitrates; i++) {
		struct indra_rate *ir = &ii->r[i];
		ir->rix = -1;
	}
	
	/* Check if max retry is compliant with constraint on service time */
	for (s = 1; s <= ip->max_retry; s++) {
		struct indra_rate *ir = &ii->r[n-1];
		st = st + ir->attempts[s-1];
		if (st>ip->st_deadline) {
			ip->max_retry = s - 1;
			break;
		}
	}

	ii->n_rates = n;
	ii->last_snr = 0;
	indra_update_rates(ip, ii);
}

static void *
indra_alloc_sta(void *priv, struct ieee80211_sta *sta, gfp_t gfp)
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
		
	return ii;
	
error:
	kfree(ii);
	return NULL;
}

static void
indra_free_sta(void *priv, struct ieee80211_sta *sta, void *priv_sta)
{
	struct indra_sta_info *ii = priv_sta;
	
	kfree(ii->r);
	kfree(ii);
}

static void
indra_init_cck_rates(struct indra_priv *ip)
{
	static const int bitrates[4] = { 10, 20, 55, 110 };
	struct ieee80211_supported_band *sband;
	u32 rate_flags = ieee80211_chandef_rate_flags(&ip->hw->conf.chandef);
	int i, j;

	sband = ip->hw->wiphy->bands[IEEE80211_BAND_2GHZ];
	if (!sband)
		return;

	for (i = 0, j = 0; i < sband->n_bitrates; i++) {
		struct ieee80211_rate *rate = &sband->bitrates[i];

		if (rate->flags & IEEE80211_RATE_ERP_G)
			continue;

		if ((rate_flags & sband->bitrates[i].flags) != rate_flags)
			continue;

		for (j = 0; j < ARRAY_SIZE(bitrates); j++) {
			if (rate->bitrate != bitrates[j])
				continue;

			ip->cck_rates[j] = i;
			break;
		}
	}
}

static void *
indra_alloc(struct ieee80211_hw *hw, struct dentry *debugfsdir)
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
	ip->ref_payload_size = (u32) 100;											
	ip->dbg_ref_payload_size = debugfs_create_u32("ref_payload_size",			
			S_IRUGO | S_IWUGO, debugfsdir, &ip->ref_payload_size);				
#endif

	indra_init_cck_rates(ip);

	return ip;
}

static void
indra_free(void *priv)
{
	kfree(priv);
}

static u32 indra_get_expected_throughput(void *priv_sta)
{
	u32 usecs;
	u32 tp;

	usecs = 1000000;
			
	tp = (1000000 / usecs);
	return INDRA_TRUNC(tp) * 1200 * 8 / 1024;
	
}

const struct rate_control_ops mac80211_indra = {
	.name = "indra",
	.tx_status = indra_tx_status,
	.get_rate = indra_get_rate,
	.rate_init = indra_rate_init,
	.alloc = indra_alloc,
	.free = indra_free,
	.alloc_sta = indra_alloc_sta,
	.free_sta = indra_free_sta,
#ifdef CPTCFG_MAC80211_DEBUGFS
	.add_sta_debugfs = indra_add_sta_debugfs,
	.remove_sta_debugfs = indra_remove_sta_debugfs,
#endif
	.get_expected_throughput = indra_get_expected_throughput,
};

int __init
rc80211_indra_init(void)
{
	return ieee80211_rate_control_register(&mac80211_indra);
}

void
rc80211_indra_exit(void)
{
	ieee80211_rate_control_unregister(&mac80211_indra);
}

