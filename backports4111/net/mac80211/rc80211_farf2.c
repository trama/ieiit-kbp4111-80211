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
#include <linux/slab.h>
#include <net/mac80211.h>
#include "rate.h"
#include "rc80211_farf2.h"

/* MICHELE **********************************************/

extern data_rclog_t data_rclog_buf[RCLOG_BUF_DIM];
extern int rclog_in;
/* MICHELE **********************************************/

/* number of data bits per symbol */
const unsigned int nbps_farf2[] = {54, 108, 162, 216, 324, 432, 486, 540};

/* convert mac80211 rate index to local array index */
static inline int
rix_to_ndx(struct farf_sta_info *fi, int rix)
{
	int i = rix;
	for (i = rix; i >= 0; i--)
		if (fi->r[i].rix == rix)
			break;
	return i;
}

/* Get next rate (higher one if not already the highest one) */
static int
farf2_next_rate(struct farf_sta_info *fi, int curr_rate)
{
	if (curr_rate < fi->n_rates - 1) 
		return curr_rate + 1;
	
	return curr_rate;
}

/* Elaborate information on last transmission outcome to select next rate */
static void
farf2_update_rates(struct farf_priv *fp, struct farf_sta_info *fi)
{
	int succ;
		
	/* Set to lowest rate if not success */
	if (!fi->success_last[fi->add_idx]) {
		fi->curr_rate[fi->add_idx] = rix_to_ndx(fi, (int) fi->lowest_rix);
	}	
	/* Retrieve number of consecutive successes and possibly update rate */
	succ = fi->r[fi->curr_rate[fi->add_idx]].success_count[fi->add_idx];
	if (succ == N_FARF) {
		fi->r[fi->curr_rate[fi->add_idx]].success_count[fi->add_idx] = 0;
		fi->curr_rate[fi->add_idx] = farf2_next_rate(fi, fi->curr_rate[fi->add_idx]);
	} 
	
}

/* Register last transmission outcome */
static void
farf2_tx_status(void *priv, struct ieee80211_supported_band *sband,
		   struct ieee80211_sta *sta, void *priv_sta,
		   struct sk_buff *skb)
{
	struct farf_priv *fp = priv;
	struct farf_sta_info *fi = priv_sta;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	struct ieee80211_tx_rate *ar = info->status.rates;
	int i, ndx = 0;			
	int success;
	int count, r = 0;				
	bool first_succ = false, null;
	
	/* Check if overall transmission was successful (i.e. has been acknowledged) */
	success = !!(info->flags & IEEE80211_TX_STAT_ACK);
	
	/* Discard null packets */
	null = !!(info->flags & IEEE80211_TX_CTL_USE_MINRATE);
	if (null)
	  return;
	 
	/* Logging */
	data_rclog_buf[rclog_in].success = success;
	data_rclog_buf[rclog_in].probe = 0;  
	for (i = 0; i < 4; i++) {
		data_rclog_buf[rclog_in].rate_idx[i] = -1;
		data_rclog_buf[rclog_in].rate_count[i] = -1;
	}
	
	/* Loop through adopted rates */
	for (i = 0; i < IEEE80211_TX_MAX_RATES; i++) {
		if (ar[i].idx < 0)
			break;

		/* Retrieve rate index */
		ndx = rix_to_ndx(fi, ar[i].idx);
		if (ndx < 0)
			continue;
		
		/* Retrieve number of attempts */
		count = ar[i].count;
		if (i==0) {
			first_succ = true;		// successful at first attempt!
		}	
				
		/* Logging */		
		data_rclog_buf[rclog_in].rate_idx[r] = ndx;
		data_rclog_buf[rclog_in].rate_count[r] = count;
		r++;				
		
		/* Not successful at first attempt */
		if (i != 0) {		
			first_succ = false;
		}
	}
	
	/* Update success count for the adopted rate(s) */
	if (!first_succ) {
		if (fi->curr_rate[fi->add_idx] != rix_to_ndx(fi, (int) fi->lowest_rix))  {
			fi->r[fi->curr_rate[fi->add_idx]].success_count[fi->add_idx] = 0;
		} else {
			fi->r[fi->curr_rate[fi->add_idx]].success_count[fi->add_idx] = success;
		}
	} else {
		fi->r[fi->curr_rate[fi->add_idx]].success_count[fi->add_idx] += 1;
	}
	
	/* Logging */
	rclog_in = (rclog_in+1)%RCLOG_BUF_DIM;
		
	/* Register if this transmission was successful at first attempt */	
	fi->success_last[fi->add_idx] = first_succ;
	
	/* Update rate based on the retrieved information */
	farf2_update_rates(fp, fi);
}

/* Get rate for current frame */
static void
farf2_get_rate(void *priv, struct ieee80211_sta *sta,
		  void *priv_sta, struct ieee80211_tx_rate_control *txrc)
{
	struct ieee80211_sta_rates *ratetbl;
	struct farf_sta_info *fi = priv_sta;
	struct farf_priv *fp = priv;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(txrc->skb);
	struct sk_buff *skb = txrc->skb;
	struct ieee80211_hdr *hdr = (void *)skb->data;
	u8 dst[ETH_ALEN];
	bool found = false;
	int record, i;
	
	/* Management/no-ack frames do not use rate control */
	if (rate_control_send_low(sta, priv_sta, txrc))
		return;
		
	/* Enable STBC if possible for data frames */
	info->flags |= fi->tx_flags;
		
	/* Allocate rate table */	
	ratetbl = kzalloc(sizeof(*ratetbl), GFP_ATOMIC);
	if (!ratetbl)
		return;
		
	/* Search destination among already recorded addresses */
	memcpy(dst, ieee80211_get_DA(hdr), ETH_ALEN);
	for (record = 0; record < fi->new_add_idx; record++) {
		found = true;
		for (i = 0; i < ETH_ALEN; i++){
			if (fi->addresses[record].add[i] != dst[i]) {
				found = false;  
				break;
			}
		}
		if (found) break;
	}
	
	/* If necessary, add new address */
	if (!found) {
		record = fi->new_add_idx;
		fi->new_add_idx = (fi->new_add_idx + 1) % FARF_ADDRESSES;
		for (i = 0; i < ETH_ALEN; i++)	{
			fi->addresses[record].add[i] = dst[i];
		}	  
	}
	
	/* Update add_idx */
	fi->add_idx = record;
	//printk("MICHELE: get_rate called with packet for %2x:%2x:%2x:%2x:%2x:%2x, add_idx=%u, new_add_idx=%u\n",dst[0],dst[1],dst[2],dst[3],dst[4],dst[5],fi->add_idx,fi->new_add_idx);
	
	/* Use current rate once */
	ratetbl->rate[0].idx = fi->r[fi->curr_rate[fi->add_idx]].rix;
	ratetbl->rate[0].count = 1;
	ratetbl->rate[0].count_cts = 1;
	ratetbl->rate[0].count_rts = 1;
	ratetbl->rate[0].flags = fi->flags;

	/* Use lowest rate then */
	ratetbl->rate[1].idx = fi->lowest_rix;
	ratetbl->rate[1].count = fp->max_retry;
	ratetbl->rate[1].count_cts = fp->max_retry;
	ratetbl->rate[1].count_rts = fp->max_retry;
	ratetbl->rate[1].flags = fi->flags;

	/* Set rates */
	rate_control_set_rates(fp->hw, fi->sta, ratetbl);
	
}

/* Compute duration of the transmission of a 1200 bytes frame */
static void
calc_rate_durations(enum ieee80211_band band, struct farf_rate *r)
{	
	r->perfect_tx_time = (DIV_ROUND_UP(1200*8,nbps_farf2[r->rix]) * 4) + ieee80211_frame_duration(band, 0, 60, 1, 1, 0);
}

/* Initialize rates */
static void
farf2_rate_init(void *priv, struct ieee80211_supported_band *sband,
		   struct cfg80211_chan_def *chandef,
		   struct ieee80211_sta *sta, void *priv_sta)
{
	struct farf_sta_info *fi = priv_sta;
	struct farf_priv *fp = priv;
	u16 sta_cap = sta->ht_cap.cap;
	unsigned int i, stbc;
	
	/* Check that station supports HT and 40MHz channels */
	if ((!sta->ht_cap.ht_supported) || (sta->bandwidth < IEEE80211_STA_RX_BW_40)) {
		printk("HT not supported by STA\n");
		return;
	}
		
	/* Check for STBC and LDPC capabilities and update tx flags */
	stbc = (sta_cap & IEEE80211_HT_CAP_RX_STBC) >>
			IEEE80211_HT_CAP_RX_STBC_SHIFT;
	fi->tx_flags |= stbc << IEEE80211_TX_CTL_STBC_SHIFT;

	if (sta_cap & IEEE80211_HT_CAP_LDPC_CODING)
		fi->tx_flags |= IEEE80211_TX_CTL_LDPC;

	/* Initialize variables */
	fi->sta = sta;
	fi->lowest_rix = 0;
	fi->n_rates = MCS_GROUP_RATES;
	fi->add_idx = 0;
	fi->new_add_idx = 0;
	memset(fi->curr_rate, 0, sizeof(fi->curr_rate));
	memset(fi->success_last, 0, sizeof(fi->success_last));
	
	/* Define flags: 
	 * IEEE80211_TX_RC_MCS -> if present use HT rates
	 * IEEE80211_TX_RC_40_MHZ_WIDTH -> if present use 40MHz channels (otherwise 20MHz ones)
	 * IEEE80211_TX_RC_SHORT_GI -> if present use short GI (otherwise standard GI)
	 * */
	fi->flags = IEEE80211_TX_RC_MCS | IEEE80211_TX_RC_40_MHZ_WIDTH;

	/* Allocate rates */
	for (i = 0; i < MCS_GROUP_RATES; i++) {
		struct farf_rate *fr = &fi->r[i];
		memset(fr, 0, sizeof(*fr));
		
		/* Initialize rate variables */
		fr->rix = i;
		memset(fr->success_count, 0, sizeof(fr->success_count));
		
		/* Compute rate durations */
		calc_rate_durations(sband->band, fr);
	}

	/* Update current rates */
	farf2_update_rates(fp, fi);
	
}

/* Allocate a new STA */
static void *
farf2_alloc_sta(void *priv, struct ieee80211_sta *sta, gfp_t gfp)
{
	struct farf_sta_info *fi;
	
	printk("MICHELE: alloc sta\n");

	/* Allocate memory */
	fi = kzalloc(sizeof(struct farf_sta_info), gfp);
	if (!fi)
		return NULL;

	/* Allocate memory for rates */
	fi->r = kzalloc(sizeof(struct farf_rate) * MCS_GROUP_RATES, gfp);
	if (!fi->r)
		goto error;
		
	return fi;
	
error:
	kfree(fi);
	return NULL;
}

/* Free STA */
static void
farf2_free_sta(void *priv, struct ieee80211_sta *sta, void *priv_sta)
{
	struct farf_sta_info *fi = priv_sta;
	
	/* Free memory */
	kfree(fi->r);
	kfree(fi);
}

/* Allocate priv */
static void *
farf2_alloc(struct ieee80211_hw *hw, struct dentry *debugfsdir)
{
	struct farf_priv *fp;

	/* Allocate memory */
	fp = kzalloc(sizeof(struct farf_priv), GFP_ATOMIC);
	if (!fp)
		return NULL;

	/* Determine maximum number of retries */
	if (hw->max_rate_tries > 0)
		fp->max_retry = hw->max_rate_tries;
	else
		/* safe default, does not necessarily have to match hw properties */
		fp->max_retry = 7;

	/* Initialize hardware */
	fp->hw = hw;

	return fp;
}

/* Free priv */
static void
farf2_free(void *priv)
{
	kfree(priv);
}

/* Get expected throughput for current rate */
static u32 farf2_get_expected_throughput(void *priv_sta)
{
	struct farf_sta_info *fi = priv_sta;
	u32 usecs;
	u32 tp;

	/* Compute duration of a 1200 bytes frame transmission */
	usecs = fi->r[fi->curr_rate[fi->add_idx]].perfect_tx_time;
	if (!usecs)
		usecs = 1000000;
			
	/* Compute throughput */
	tp = (1000000 / usecs);
	return FARF_TRUNC(tp) * 1200 * 8 / 1024;
	
}

const struct rate_control_ops mac80211_farf2 = {
	.name = "farf2",
	.tx_status = farf2_tx_status,
	.get_rate = farf2_get_rate,
	.rate_init = farf2_rate_init,
	.alloc = farf2_alloc,
	.free = farf2_free,
	.alloc_sta = farf2_alloc_sta,
	.free_sta = farf2_free_sta,
	.get_expected_throughput = farf2_get_expected_throughput,
};

int __init
rc80211_farf2_init(void)
{
	return ieee80211_rate_control_register(&mac80211_farf2);
}

void
rc80211_farf2_exit(void)
{
	ieee80211_rate_control_unregister(&mac80211_farf2);
}

