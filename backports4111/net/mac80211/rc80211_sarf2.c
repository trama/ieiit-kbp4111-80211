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
#include "rc80211_sarf2.h"

/* MICHELE **********************************************/

extern data_rclog_t data_rclog_buf[RCLOG_BUF_DIM];
extern int rclog_in;
/* MICHELE **********************************************/

/* number of data bits per symbol */
const unsigned int nbps_sarf2[] = {54, 108, 162, 216, 324, 432, 486, 540};

/* convert mac80211 rate index to local array index */
static inline int
rix_to_ndx(struct sarf_sta_info *si, int rix)
{
	int i = rix;
	for (i = rix; i >= 0; i--)
		if (si->r[i].rix == rix)
			break;
	return i;
}

/* Upgrade rate (higher one if not already the highest one) */
static int
sarf2_higher_rate(struct sarf_sta_info *si, int curr_rate)
{
	if (curr_rate < si->n_rates - 1) 
		return curr_rate + 1;
	
	return curr_rate;
}

/* Downgrade rate (lower one if not already the lowest one) */
static int
sarf2_lower_rate(struct sarf_sta_info *si, int curr_rate)
{
	if (curr_rate > 0) 
		return curr_rate - 1;
	
	return curr_rate;
}

/* Elaborate information on last transmission outcome to select next rate */
static void
sarf2_update_rates(struct sarf_priv *sp, struct sarf_sta_info *si)
{
	int succ, fail, prev_rate, next_rate;
	
	/* Retrieve current rate and number of failures and successes */
	prev_rate = si->curr_rate[si->add_idx];
	succ = si->r[prev_rate].success_count[si->add_idx];
	fail = si->r[prev_rate].failures_count[si->add_idx];
	/* Check if rate has to be downgraded */
	if ((!si->success_last[si->add_idx]) && (fail==K_SARF)) {
		si->r[prev_rate].failures_count[si->add_idx] = 0;
		next_rate = sarf2_lower_rate(si, prev_rate);
		si->curr_rate[si->add_idx] = next_rate;
	}	
	/* Check if rate has to be upgraded */
	if ((si->success_last[si->add_idx]) && (succ==N_SARF)) {
		si->r[prev_rate].success_count[si->add_idx] = 0;
		next_rate = sarf2_higher_rate(si, prev_rate);
		si->curr_rate[si->add_idx] = next_rate;
		if (next_rate != prev_rate) {
			si->r[next_rate].failures_count[si->add_idx] = K_SARF-1;
		}
	} 
	
	
}

/* Register last transmission outcome */
static void
sarf2_tx_status(void *priv, struct ieee80211_supported_band *sband,
		   struct ieee80211_sta *sta, void *priv_sta,
		   struct sk_buff *skb)
{
	struct sarf_priv *sp = priv;
	struct sarf_sta_info *si = priv_sta;
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
		ndx = rix_to_ndx(si, ar[i].idx);
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
		if (si->curr_rate[si->add_idx] != rix_to_ndx(si, (int) si->lowest_rix))  {
			si->r[si->curr_rate[si->add_idx]].success_count[si->add_idx] = 0;
			si->r[si->curr_rate[si->add_idx]].failures_count[si->add_idx] += 1;
		} else {
			si->r[si->curr_rate[si->add_idx]].success_count[si->add_idx] = success;
		}
	} else {
		si->r[si->curr_rate[si->add_idx]].success_count[si->add_idx] += 1;
		si->r[si->curr_rate[si->add_idx]].failures_count[si->add_idx] = 0;
	}
	
	/* Logging */
	rclog_in = (rclog_in+1)%RCLOG_BUF_DIM;
		
	/* Register if this transmission was successful at first attempt */	
	si->success_last[si->add_idx] = first_succ;
	
	/* Update rate based on the retrieved information */
	sarf2_update_rates(sp, si);
}

/* Get rate for current frame */
static void
sarf2_get_rate(void *priv, struct ieee80211_sta *sta,
		  void *priv_sta, struct ieee80211_tx_rate_control *txrc)
{
	struct ieee80211_sta_rates *ratetbl;
	struct sarf_sta_info *si = priv_sta;
	struct sarf_priv *sp = priv;
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
	info->flags |= si->tx_flags;
		
	/* Allocate rate table */	
	ratetbl = kzalloc(sizeof(*ratetbl), GFP_ATOMIC);
	if (!ratetbl)
		return;
		
	/* Search destination among already recorded addresses */
	memcpy(dst, ieee80211_get_DA(hdr), ETH_ALEN);
	for (record = 0; record < si->new_add_idx; record++) {
		found = true;
		for (i = 0; i < ETH_ALEN; i++){
			if (si->addresses[record].add[i] != dst[i]) {
				found = false;  
				break;
			}
		}
		if (found) break;
	}
	
	/* If necessary, add new address */
	if (!found) {
		record = si->new_add_idx;
		si->new_add_idx = (si->new_add_idx + 1) % SARF_ADDRESSES;
		for (i = 0; i < ETH_ALEN; i++)	{
			si->addresses[record].add[i] = dst[i];
		}	  
	}
	
	/* Update add_idx */
	si->add_idx = record;
	//printk("MICHELE: get_rate called with packet for %2x:%2x:%2x:%2x:%2x:%2x, add_idx=%u, new_add_idx=%u\n",dst[0],dst[1],dst[2],dst[3],dst[4],dst[5],si->add_idx,si->new_add_idx);
	
	/* Use current rate once */
	ratetbl->rate[0].idx = si->r[si->curr_rate[si->add_idx]].rix;
	ratetbl->rate[0].count = 1;
	ratetbl->rate[0].count_cts = 1;
	ratetbl->rate[0].count_rts = 1;
	ratetbl->rate[0].flags = si->flags;

	/* Use lowest rate then */
	ratetbl->rate[1].idx = si->lowest_rix;
	ratetbl->rate[1].count = sp->max_retry;
	ratetbl->rate[1].count_cts = sp->max_retry;
	ratetbl->rate[1].count_rts = sp->max_retry;
	ratetbl->rate[1].flags = si->flags;

	/* Set rates */
	rate_control_set_rates(sp->hw, si->sta, ratetbl);
	
}

/* Compute duration of the transmission of a 1200 bytes frame */
static void
calc_rate_durations(enum ieee80211_band band, struct sarf_rate *r)
{	
	r->perfect_tx_time = (DIV_ROUND_UP(1200*8,nbps_sarf2[r->rix]) * 4) + ieee80211_frame_duration(band, 0, 60, 1, 1, 0);
}

/* Initialize rates */
static void
sarf2_rate_init(void *priv, struct ieee80211_supported_band *sband,
		   struct cfg80211_chan_def *chandef,
		   struct ieee80211_sta *sta, void *priv_sta)
{
	struct sarf_sta_info *si = priv_sta;
	struct sarf_priv *sp = priv;
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
	si->tx_flags |= stbc << IEEE80211_TX_CTL_STBC_SHIFT;

	if (sta_cap & IEEE80211_HT_CAP_LDPC_CODING)
		si->tx_flags |= IEEE80211_TX_CTL_LDPC;

	/* Initialize variables */
	si->sta = sta;
	si->lowest_rix = 0;
	si->n_rates = MCS_GROUP_RATES;
	si->add_idx = 0;
	si->new_add_idx = 0;
	memset(si->curr_rate, 0, sizeof(si->curr_rate));
	memset(si->success_last, 0, sizeof(si->success_last));
	
	/* Define flags: 
	 * IEEE80211_TX_RC_MCS -> if present use HT rates
	 * IEEE80211_TX_RC_40_MHZ_WIDTH -> if present use 40MHz channels (otherwise 20MHz ones)
	 * IEEE80211_TX_RC_SHORT_GI -> if present use short GI (otherwise standard GI)
	 * */
	si->flags = IEEE80211_TX_RC_MCS | IEEE80211_TX_RC_40_MHZ_WIDTH;

	/* Allocate rates */
	for (i = 0; i < MCS_GROUP_RATES; i++) {
		struct sarf_rate *sr = &si->r[i];
		memset(sr, 0, sizeof(*sr));
		
		/* Initialize rate variables */
		sr->rix = i;
		memset(sr->success_count, 0, sizeof(sr->success_count));
		memset(sr->failures_count, 0, sizeof(sr->failures_count));
		
		/* Compute rate durations */
		calc_rate_durations(sband->band, sr);
	}

	/* Update current rates */
	sarf2_update_rates(sp, si);
	
}

/* Allocate a new STA */
static void *
sarf2_alloc_sta(void *priv, struct ieee80211_sta *sta, gfp_t gfp)
{
	struct sarf_sta_info *si;

	/* Allocate memory */
	si = kzalloc(sizeof(struct sarf_sta_info), gfp);
	if (!si)
		return NULL;

	/* Allocate memory for rates */
	si->r = kzalloc(sizeof(struct sarf_rate) * MCS_GROUP_RATES, gfp);
	if (!si->r)
		goto error;
		
	return si;
	
error:
	kfree(si);
	return NULL;
}

/* Free STA */
static void
sarf2_free_sta(void *priv, struct ieee80211_sta *sta, void *priv_sta)
{
	struct sarf_sta_info *si = priv_sta;
	
	/* Free memory */
	kfree(si->r);
	kfree(si);
}

/* Allocate priv */
static void *
sarf2_alloc(struct ieee80211_hw *hw, struct dentry *debugfsdir)
{
	struct sarf_priv *sp;

	/* Allocate memory */
	sp = kzalloc(sizeof(struct sarf_priv), GFP_ATOMIC);
	if (!sp)
		return NULL;

	/* Determine maximum number of retries */
	if (hw->max_rate_tries > 0)
		sp->max_retry = hw->max_rate_tries;
	else
		/* safe default, does not necessarily have to match hw properties */
		sp->max_retry = 7;

	/* Initialize hardware */
	sp->hw = hw;

	return sp;
}

/* Free priv */
static void
sarf2_free(void *priv)
{
	kfree(priv);
}

/* Get expected throughput for current rate */
static u32 sarf2_get_expected_throughput(void *priv_sta)
{
	struct sarf_sta_info *si = priv_sta;
	u32 usecs;
	u32 tp;

	/* Compute duration of a 1200 bytes frame transmission */
	usecs = si->r[si->curr_rate[si->add_idx]].perfect_tx_time;
	if (!usecs)
		usecs = 1000000;
			
	/* Compute throughput */
	tp = (1000000 / usecs);
	return SARF_TRUNC(tp) * 1200 * 8 / 1024;
	
}

const struct rate_control_ops mac80211_sarf2 = {
	.name = "sarf2",
	.tx_status = sarf2_tx_status,
	.get_rate = sarf2_get_rate,
	.rate_init = sarf2_rate_init,
	.alloc = sarf2_alloc,
	.free = sarf2_free,
	.alloc_sta = sarf2_alloc_sta,
	.free_sta = sarf2_free_sta,
	.get_expected_throughput = sarf2_get_expected_throughput,
};

int __init
rc80211_sarf2_init(void)
{
	return ieee80211_rate_control_register(&mac80211_sarf2);
}

void
rc80211_sarf2_exit(void)
{
	ieee80211_rate_control_unregister(&mac80211_sarf2);
}

