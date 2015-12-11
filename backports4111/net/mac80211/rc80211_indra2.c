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
#include "rc80211_indra2.h"

/* external logging */
extern data_rclog_t data_rclog_buf[RCLOG_BUF_DIM];
extern int rclog_in;

/* number of data bits per symbol */
const unsigned int nbps_indra2[] = {54, 108, 162, 216, 324, 432, 486, 540};

/* per vs SNR map */
const unsigned int per_snr_map[MCS_RATES][SNR_RANGE] = {{999,981,893,688,424,244,166,125,101,70,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
														{1000,965,890,774,599,351,218,187,174,153,130,70,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
														{1000,1000,975,831,576,354,239,221,213,171,126,62,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
														{1000,1000,1000,1000,995,895,677,445,315,259,199,161,129,79,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
														{1000,1000,1000,1000,1000,1000,1000,991,895,689,459,335,316,292,200,143,117,93,51,12,0,0,0,0,0,0,0,0,0,0},
														{1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,999,954,831,621,395,231,149,140,135,130,125,120,90,70,19,0,0,0,0,0},
														{1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,982,908,735,488,291,234,228,203,178,149,122,97,51,11,0,0,0,0},
														{1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,955,826,606,369,240,183,150,142,140,120,85,23,0,0,0}};

/* reference payload size */
static u64 ref_size;

static void
explore_stages(struct indra_priv *ip, struct indra_sta_info *ii, int snr,
			unsigned int *rates_old, int i_max,
			unsigned int *found, unsigned int **rates_min,
			unsigned int *loss_min, int stage)
{
	int i, j;
	unsigned int sum1=0, sum2=0, sum_st=0;
	unsigned long long loss = 1000, loss_init;
	
	/* Create rates array, sums and products*/
	unsigned int rates[stage];
	for (i=0;i<stage-1;i++) {
		struct indra_rate *ir = &ii->r[rates_old[i]-1];
		rates[i] = rates_old[i];
		sum1 = sum1 + rates[i];
		sum2 = sum2 + rates_min[stage-1][i];
		sum_st = sum_st + ir->service_time[i];
		loss = INDRA_TRUNC(INDRA_FRAC(loss*per_snr_map[rates_old[i]-1][snr],1000));
	}	
	sum2 = sum2 + rates_min[stage-1][stage-1];
	loss_init = loss;
	
	/* Main loop */
	for (i = 0; i<=i_max; i++) {
		//if (found[stage-1] == 0) {
			struct indra_rate *ir = &ii->r[i];
			if (sum_st + ir->service_time[stage-1] <= ip->st_deadline) {
				loss = INDRA_TRUNC(INDRA_FRAC(loss_init*per_snr_map[i][snr],1000));
				if (loss < loss_min[stage-1]) {
					found[stage-1] = 1;
					loss_min[stage-1] = loss;
					for (j = 0; j<stage-1; j++) 
						rates_min[stage-1][j] = rates[j];
					rates_min[stage-1][stage-1] = i+1;
				}
				else if (loss == loss_min[stage-1]) {
					if (sum1+i+1 > sum2) {
						found[stage-1] = 1;
						loss_min[stage-1] = loss;
						for (j = 0; j<stage-1; j++) 
							rates_min[stage-1][j] = rates[j];
						rates_min[stage-1][stage-1] = i+1;
					}
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
indra2_set_rate(struct indra_priv *ip, struct indra_sta_info *ii,
                     struct ieee80211_sta_rates *ratetbl, int offset, int index)
{
	ratetbl->rate[offset].idx = index;
	ratetbl->rate[offset].count = 1;
	ratetbl->rate[offset].count_cts = 1;
	ratetbl->rate[offset].count_rts = 1;
	ratetbl->rate[offset].flags = ii->flags;
}

static void
indra2_optimization(struct indra_priv *ip, struct indra_sta_info *ii)
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
		for (i = 0; i < MCS_RATES; i++) {
			struct indra_rate *ir;
			ir = &ii->r[i];
			/* Check service time bound */
			if (ir->service_time[0]>ip->st_deadline) 
				continue;
			loss = per_snr_map[i][snr];
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
			//if (snr==29) {
				//printk("MICHELE: snr=29, idx=%i, ",i);
				//for (j = 0; j <= i; j++) 
					//printk("r[%i]=%u, ",j,rates_min[i][j]-1);
				//printk("\n");
			//}
			if ((found[i] != 0) && (loss_min[i]<=loss)) { 
				idx = i;
				loss = loss_min[i];
			}
		}
		
		printk("MICHELE: SNR=%i, index=%i, ",snr,idx);
		
		/* Save optimization results */
		ii->optimal_indexes[snr] = idx;
		for (j = 0; j<=idx; j++) {
			ii->optimal_rates[snr][j] = rates_min[idx][j];
			printk("r%i = %i, ",j,rates_min[idx][j]);
		} 
		printk(" loss: %u\n", loss);
		//printk("MICHELE: SNR=%i, first attempt rate = %i\n",snr,ii->r[ii->optimal_rates[snr][0]-1].rix);
		
		/* Reinitialize temporary arrays */
		for (i = 0; i < ip->max_retry; i++) {
			found[i] = 0;
			loss_min[i] = UINT_MAX;
			for (j = 0; j < ip->max_retry; j++) {
				rates_min[i][j] = 0;
			}
		}
	}
	
	/* Free memory */
	kfree(found);
	kfree(loss_min);
	for (i = 0; i < ip->max_retry; i++) {
		kfree(rates_min[i]);
	}
	kfree(rates_min);
	
}

static void
indra2_update_rates(struct indra_priv *ip, struct indra_sta_info *ii)
{
	struct ieee80211_sta_rates *ratetbl;
	int i,j, idx_min = -1, idx_eq = 0;
	unsigned long long w;
	unsigned long err_min = ULONG_MAX, err;
	
	/* Allocate rate table */
	ratetbl = kzalloc(sizeof(*ratetbl), GFP_ATOMIC);
	if (!ratetbl)
		return;
		
	/* SNR estimation */	
	for (i = 0; i < SNR_RANGE; i++) {
		
		err = 0;
		w = 0;
		for (j = 0; j < MCS_RATES; j++) {
			struct indra_rate *ir;
			ir = &ii->r[j];
			
			/* Only compute error if weight is positive */
			if (ir->weight > 0) {
				w += ir->weight;
				err += ir->weight*(per_snr_map[j][i]-ir->prob)*(per_snr_map[j][i]-ir->prob);
				//printk("MICHELE: MCS %i: prob=%u, weight=%u\n",j,ir->prob,ir->weight);
			}
		}
		if (w > 0)
			err = INDRA_TRUNC(INDRA_FRAC(err,w));
		else 
			err = ULONG_MAX;
		//printk("MICHELE: SNR %i: err=%lu\n",i,err);
		
		/* Check if we have found minimum */
		if (err < err_min) {
			idx_eq = 0;
			idx_min = i;
			err_min = err;
		}	
		else if (err == err_min) {
			idx_eq++;
		}
	}
	if (idx_min == -1)
		ii->estimated_snr = 0;
	else {
		//ii->estimated_snr = idx_min + DIV_ROUND_UP(idx_eq,2);
		ii->estimated_snr = idx_min;
	}
	printk("MICHELE: estimated SNR = %i, opt_ind = %i\n",ii->estimated_snr, ii->optimal_indexes[ii->estimated_snr]);

	/* Write rate table */
	for (i = 0; i<=ii->optimal_indexes[ii->estimated_snr]; i++) 
		indra2_set_rate(ip, ii, ratetbl, i, ii->r[ii->optimal_rates[ii->estimated_snr][i]-1].rix);
	//printk("first attempt rate = %i\n",ii->r[ii->optimal_rates[ii->estimated_snr][0]-1].rix); 
	//ratetbl->rate[i+1].idx = -1;
	rate_control_set_rates(ip->hw, ii->sta, ratetbl);
}

static void
indra2_update_stats(struct indra_priv *ip, struct indra_sta_info *ii)
{
	struct indra_rate *ir;
	int i, j, att, alpha_p, alpha_w;
	unsigned long long p,w;
	//printk("MICHELE: update\n");
	
	/* Compute total attempts */
	att = 0;
	for (i = 0; i < MCS_RATES; i++) 
		att += ii->r[i].attempts[0];
	ii->attempts[0] = att;
	
	/* Update stats for each rate */
	for (i = 0; i < MCS_RATES; i++) {
		
		ir = &ii->r[i];
		
		p = 0;
		w = 0;
		alpha_p = 0;
		alpha_w = 0;
		//printk("MICHELE: w: ");
		for (j = 0; j < MEMORY_SIZE; j++) {
			if (ir->attempts[j]>0) {
				if (ir->success[j] == 0) {
					p += ii->alpha[j]*1000;
				}
				else {
					p += ii->alpha[j]*INDRA_TRUNC(INDRA_FRAC(1000*(ir->attempts[j]-ir->success[j]),ir->attempts[j]));
				}
				alpha_p += ii->alpha[j];
			}
			if (ii->attempts[j]>0) {
				w += ii->alpha[j]*INDRA_TRUNC(INDRA_FRAC(100*ir->attempts[j],ii->attempts[j]));
				alpha_w += ii->alpha[j];
				//printk("%i: a=%u, al=%u, w=%lu, ",j,ir->attempts[j],ii->attempts[j],w);
			}
		}
		//printk("\n");
		if (alpha_w > 0) {
			ir->weight = INDRA_TRUNC(INDRA_FRAC(w, alpha_w));
		}
		if (alpha_p > 0) {
			ir->prob = INDRA_TRUNC(INDRA_FRAC(p, alpha_p));
			//printk("MICHELE: att_hist=%i, MCS %i: succ=%u, att=%u, prob=%llu, weight=%llu, p=%llu, w=%llu\n",ii->att_hist,i,ir->success[0],ir->attempts[0],ir->prob,ir->weight,p,w);
		}
		else {
			ir->prob = 0;
			ir->weight = 0;
		}
		ir->att_hist += ir->attempts[0];
		ir->succ_hist += ir->success[0];
		
		//printk("MICHELE: history: ");
		for (j = MEMORY_SIZE-1; j > 0; j--) {
			ir->attempts[j] = ir->attempts[j-1];
			ir->success[j] = ir->success[j-1];
			//printk("%i: s=%i, a=%i, ",j,ir->attempts[j],ir->attempts[j]);
		}
		//printk("\n");
		ir->attempts[0] = 0;
		ir->success[0] = 0;
	}
	
	/* Update global attempts count */
	ii->att_hist += ii->attempts[0];
	for (i = MEMORY_SIZE-1; i > 0; i--) 
			ii->attempts[i] = ii->attempts[i-1];
	ii->attempts[0] = 0;
		
	/* Reset update timer */
	ii->last_stats_update = jiffies;
}

static void
indra2_tx_status(void *priv, struct ieee80211_supported_band *sband,
		   struct ieee80211_sta *sta, void *priv_sta,
		   struct sk_buff *skb)
{
	struct indra_sta_info *ii = priv_sta;
	struct indra_priv *ip = priv;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	struct ieee80211_tx_rate *ar = info->status.rates;
	int i, count = 0, r = 0, ndx = 0;			
	int success, null;

	/* This packet was aggregated but doesn't carry status info */
	if ((info->flags & IEEE80211_TX_CTL_AMPDU) &&
	    !(info->flags & IEEE80211_TX_STAT_AMPDU))
		return;

	/* Check transmission status: successful or not */
	success = !!(info->flags & IEEE80211_TX_STAT_ACK);
	
	/* Check if this is a null transmission: if so, exit */
	null = !!(info->flags & IEEE80211_TX_CTL_USE_MINRATE);
	if (null)
	  return;
	
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
		count += ar[i].count;
		ii->r[ndx].attempts[0] += ar[i].count;
		
		/* Write in the rclog device */		
		data_rclog_buf[rclog_in].rate_idx[r] = ndx;
		data_rclog_buf[rclog_in].rate_count[r] = ar[i].count;
		r++;				
		
		/* Update number of successes for the 
		 * retrieved rate */ 
		if ((i != IEEE80211_TX_MAX_RATES - 1) && (ar[i + 1].idx < 0))
			ii->r[ndx].success[0] += success;
	}
	
	/* Point to new entry in rclog character device */
	rclog_in = (rclog_in+1)%RCLOG_BUF_DIM;
	
	//* Update rates if timer has expired */
	if (time_after(jiffies, ii->last_stats_update +
				(ip->update_interval / 2 * HZ) / 1000)) {
		indra2_update_stats(ip, ii);
		indra2_update_rates(ip, ii);
	}

}

static void
indra2_get_rate(void *priv, struct ieee80211_sta *sta,
		  void *priv_sta, struct ieee80211_tx_rate_control *txrc)
{
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(txrc->skb);
	struct indra_sta_info *ii = priv_sta;
	
	/* management/no-ack frames do not use rate control */
	if (rate_control_send_low(sta, priv_sta, txrc))
		return;
		
	/* Enable STBC if possible */
	info->flags |= ii->tx_flags;
	
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
	
	r->perfect_tx_time = (DIV_ROUND_UP(ip->ref_payload_size*8,nbps_indra2[r->rix]) * 4) + ieee80211_frame_duration(band, 0, 60, 1, 1, 0);
	r->ack_time = ieee80211_frame_duration(band, 10, 60, 1, 1, 0);	/* this is not really correct....*/
	
	for (i=0; i<7; i++) {
		if (i==0) {
			r->service_time[i] = 2*t_slot + r->perfect_tx_time + r->ack_time;
		}
		else {
			r->service_time[i] = 2*t_slot + r->perfect_tx_time + r->ack_time + t_slot*(pow[i-1]*(ip->cw_min+1)-1);
		} 
		//printk("MICHELE: rate %i (d=%u, a=%u), attempt %i, time %u\n",r->rix,r->perfect_tx_time,r->ack_time,i,r->service_time[i]);
	}
}

static void
indra2_rate_init(void *priv, struct ieee80211_supported_band *sband,
		   struct cfg80211_chan_def *chandef,
		   struct ieee80211_sta *sta, void *priv_sta)
{
	struct indra_sta_info *ii = priv_sta;
	struct indra_priv *ip = priv;
	u16 sta_cap = sta->ht_cap.cap;
	unsigned int i, s, st = 0, stbc; 
	
	/* Check that HT and 40 MHz are supported */
	if ((!sta->ht_cap.ht_supported) || (sta->bandwidth < IEEE80211_STA_RX_BW_40)) {
		printk("HT not supported by STA\n");
		return;
	}
		
	/* Check for STBC and LDPC capabilities and update tx flags */
	stbc = (sta_cap & IEEE80211_HT_CAP_RX_STBC) >>
			IEEE80211_HT_CAP_RX_STBC_SHIFT;
	ii->tx_flags |= stbc << IEEE80211_TX_CTL_STBC_SHIFT;

	if (sta_cap & IEEE80211_HT_CAP_LDPC_CODING)
		ii->tx_flags |= IEEE80211_TX_CTL_LDPC;
				
	/* Initializations */	
	ii->sta = sta;
	
	/* Initialize alphas */
	// TO DO: check numerical issues in computation of alpha
	ii->alpha[0] = 24;
	ii->alpha[1] = 20;
	ii->alpha[2] = 15;
	ii->alpha[3] = 12;
	ii->alpha[4] = 10;
	ii->alpha[5] = 8;
	ii->alpha[6] = 6;
	ii->alpha[7] = 5;
	
	/* Define flags: 
	 * IEEE80211_TX_RC_MCS -> if present use HT rates
	 * IEEE80211_TX_RC_40_MHZ_WIDTH -> if present use 40MHz channels (otherwise 20MHz ones)
	 * IEEE80211_TX_RC_SHORT_GI -> if present use short GI (otherwise standard GI)
	 * */
	ii->flags = IEEE80211_TX_RC_MCS | IEEE80211_TX_RC_40_MHZ_WIDTH;
	
	/* Allocate rates */
	for (i = 0; i < MCS_RATES; i++) {
		struct indra_rate *ir = &ii->r[i];
		memset(ir, 0, sizeof(*ir));
		ir->rix = i;
		
		calc_service_time_values(ip, sband->band, ir, chandef);
	}
	
	/* Check if max retry is compliant with constraint on service time */
	for (s = 1; s <= ip->max_retry; s++) {
		struct indra_rate *ir = &ii->r[MCS_RATES-1];
		st = st + ir->service_time[s-1];
		if (st>ip->st_deadline) {
			ip->max_retry = s - 1;
			break;
		}
	}
	printk("MICHELE: rate_init, ref_size=%llu, deadline=%u, max_retry=%i\n",ref_size,ip->st_deadline,ip->max_retry);
	
	/* Initialize optimal rates array */
	for (i = 0; i < SNR_RANGE; i++) 
		ii->optimal_rates[i] = kzalloc(sizeof(*ii->optimal_rates[i]) * ip->max_retry,GFP_ATOMIC);

	/* Update */
	indra2_update_stats(ip, ii);
	indra2_optimization(ip, ii);
	indra2_update_rates(ip, ii);
	
	return;
	
}

static void *
indra2_alloc_sta(void *priv, struct ieee80211_sta *sta, gfp_t gfp)
{
	struct indra_sta_info *ii;

	ii = kzalloc(sizeof(struct indra_sta_info), gfp);
	if (!ii)
		return NULL;

	ii->r = kzalloc(sizeof(struct indra_rate) * MCS_RATES, gfp);
	if (!ii->r)
		goto error;
		
	ii->optimal_indexes = kzalloc(sizeof(*ii->optimal_indexes)*SNR_RANGE,GFP_ATOMIC);
	ii->optimal_rates = kzalloc(sizeof(*ii->optimal_rates)*SNR_RANGE,GFP_ATOMIC);
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
indra2_free_sta(void *priv, struct ieee80211_sta *sta, void *priv_sta)
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
indra2_alloc(struct ieee80211_hw *hw, struct dentry *debugfsdir)
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
	ip->st_deadline = (u32) 1000;
	ip->dbg_st_deadline = debugfs_create_u32("deadline",
			S_IRUGO | S_IWUGO, debugfsdir, &ip->st_deadline);
	ip->ref_payload_size = (u32) 50;											
	ip->dbg_ref_payload_size = debugfs_create_u32("ref_payload_size",			
			S_IRUGO | S_IWUGO, debugfsdir, &ip->ref_payload_size);	
	ip->update_interval = (u32) 1000;											
	ip->dbg_update_interval = debugfs_create_u32("update_interval",			
			S_IRUGO | S_IWUGO, debugfsdir, &ip->update_interval);				
	ref_size = ip->ref_payload_size;
#else
	ref_size = 50;
#endif
	return ip;
}

static void
indra2_free(void *priv)
{
	kfree(priv);
}

static u32 indra2_get_expected_throughput(void *priv_sta)
{
	struct indra_sta_info *ii = priv_sta;
	int prob;
	unsigned int usecs;

	prob = ii->r[ii->optimal_rates[ii->optimal_indexes[ii->estimated_snr]][0]-1].prob;
	usecs = ii->r[ii->optimal_rates[ii->optimal_indexes[ii->estimated_snr]][0]-1].service_time[0];

	return INDRA_TRUNC(prob / usecs) * ref_size * 8 / 1024;
	
}

const struct rate_control_ops mac80211_indra2 = {
	.name = "indra2",
	.tx_status = indra2_tx_status,
	.get_rate = indra2_get_rate,
	.rate_init = indra2_rate_init,
	.alloc = indra2_alloc,
	.free = indra2_free,
	.alloc_sta = indra2_alloc_sta,
	.free_sta = indra2_free_sta,
	.get_expected_throughput = indra2_get_expected_throughput,
};

int __init
rc80211_indra2_init(void)
{
	return ieee80211_rate_control_register(&mac80211_indra2);
}

void
rc80211_indra2_exit(void)
{
	ieee80211_rate_control_unregister(&mac80211_indra2);
}

