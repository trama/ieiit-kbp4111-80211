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
 *
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
#include "rc80211_sarf.h"
#include "rc80211_sarf_ht.h"

/* MICHELE **********************************************/

extern data_rclog_t data_rclog_buf[RCLOG_BUF_DIM];
extern int rclog_in;
/* MICHELE **********************************************/

/* number of data bits per symbol */
const unsigned int nbps_sarf[] = {54, 108, 162, 216, 324, 432, 486, 540};

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

static int
sarf_ht_higher_rate(struct sarf_sta_info *si, int curr_rate)
{
	if (curr_rate < si->n_rates - 1) 
		return curr_rate + 1;
	
	return curr_rate;
}

static int
sarf_ht_lower_rate(struct sarf_sta_info *si, int curr_rate)
{
	if (curr_rate > 0) 
		return curr_rate - 1;
	
	return curr_rate;
}

static void
sarf_ht_update_rates(struct sarf_priv *sp, struct sarf_sta_info *si)
{
	struct ieee80211_sta_rates *ratetbl;
	int succ, fail;
	int prev_rate, next_rate;
	
	ratetbl = kzalloc(sizeof(*ratetbl), GFP_ATOMIC);
	if (!ratetbl)
		return;
		
	/* Update current rate*/
	prev_rate = si->curr_rate;
	succ = si->r[prev_rate].success_count;
	fail = si->r[prev_rate].failures_count;
	if ((!si->success_last) && (fail==K_SARF)) {
		si->r[prev_rate].failures_count = 0;
		next_rate = sarf_ht_lower_rate(si, prev_rate);
		si->curr_rate = next_rate;
	}	
	if ((si->success_last) && (succ==N_SARF)) {
		si->r[prev_rate].success_count = 0;
		next_rate = sarf_ht_higher_rate(si, prev_rate);
		si->curr_rate = next_rate;
		if (next_rate != prev_rate) {
			si->r[next_rate].failures_count = K_SARF-1;
		}
	} 
	
	/* Use current rate once */
	ratetbl->rate[0].idx = si->r[si->curr_rate].rix;
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

	rate_control_set_rates(sp->hw, si->sta, ratetbl);
	
	return;
}

static void
sarf_ht_tx_status(void *priv, struct ieee80211_supported_band *sband,
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
	int t_idx;
	bool null_packet;				
	bool first_succ = false;

	success = !!(info->flags & IEEE80211_TX_STAT_ACK);
	null_packet = !!(info->flags & IEEE80211_TX_CTL_USE_MINRATE);
	
	if (null_packet)
	  return;
	
	t_idx = si->trasm_index;	
	first_succ = true;
	
	// Initialize
	si->t[t_idx].success = success;
	data_rclog_buf[rclog_in].success = success;
	data_rclog_buf[rclog_in].probe = 0;
	for (i = 0; i < 4; i++) {
		si->t[t_idx].rate_idx[i] = -1;
		si->t[t_idx].rate_count[i] = -1;
		data_rclog_buf[rclog_in].rate_idx[i] = -1;
		data_rclog_buf[rclog_in].rate_count[i] = -1;
	}
	
	for (i = 0; i < IEEE80211_TX_MAX_RATES; i++) {
		if (ar[i].idx < 0)
			break;

		ndx = rix_to_ndx(si, ar[i].idx);
		if (ndx < 0)
			continue;
		
		count = ar[i].count;	
		
		si->t[t_idx].rate_idx[r] = ndx;
		si->t[t_idx].rate_count[r] = count;
		data_rclog_buf[rclog_in].rate_idx[r] = ndx;
		data_rclog_buf[rclog_in].rate_count[r] = count;
		r++;				
		
		if (i != 0) {		
			first_succ = false;
		}
	}
	
	if (!first_succ) {
		if (si->curr_rate != rix_to_ndx(si, (int) si->lowest_rix))  {
			si->r[si->curr_rate].success_count = 0;
			si->r[si->curr_rate].failures_count += 1;
		} else {
			si->r[si->curr_rate].success_count = success;
		}
	} else {
		si->r[si->curr_rate].failures_count = 0;
		si->r[si->curr_rate].success_count += 1;
	}
	
	// Update
	t_idx = (t_idx + 1) % TX_RECORDED_SARF;
	si->trasm_index = t_idx;
	si->trasm_number++;
	rclog_in = (rclog_in+1)%RCLOG_BUF_DIM;
	
	si->success_last = first_succ;
	sarf_ht_update_rates(sp, si);
}

static void
sarf_ht_get_rate(void *priv, struct ieee80211_sta *sta,
		  void *priv_sta, struct ieee80211_tx_rate_control *txrc)
{
	struct sarf_sta_info *si = priv_sta;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(txrc->skb);

	/* management/no-ack frames do not use rate control */
	if (rate_control_send_low(sta, priv_sta, txrc))
		return;
		
	/* Enable STBC if possible */
	info->flags |= si->tx_flags;
	
}

static void
calc_rate_durations(enum ieee80211_band band, struct sarf_rate *r)
{	
	r->perfect_tx_time = (DIV_ROUND_UP(1200*8,nbps_sarf[r->rix]) * 4) + ieee80211_frame_duration(band, 0, 60, 1, 1, 0);
}

static void
sarf_ht_rate_init(void *priv, struct ieee80211_supported_band *sband,
		   struct cfg80211_chan_def *chandef,
		   struct ieee80211_sta *sta, void *priv_sta)
{
	struct sarf_sta_info *si = priv_sta;
	struct sarf_priv *sp = priv;
	u16 sta_cap = sta->ht_cap.cap;
	unsigned int i, stbc;
	
	printk("MICHELE: here\n");
	
	/* fall back to the old FARF for legacy stations */
	if (!sta->ht_cap.ht_supported)
		goto use_legacy;
	
	/* Check that 40 MHz is supported */
	if (sta->bandwidth < IEEE80211_STA_RX_BW_40)
		goto use_legacy;
		
	/* Check for STBC and LDPC capabilities and update tx flags */
	stbc = (sta_cap & IEEE80211_HT_CAP_RX_STBC) >>
			IEEE80211_HT_CAP_RX_STBC_SHIFT;
	si->tx_flags |= stbc << IEEE80211_TX_CTL_STBC_SHIFT;

	if (sta_cap & IEEE80211_HT_CAP_LDPC_CODING)
		si->tx_flags |= IEEE80211_TX_CTL_LDPC;

	si->sta = sta;
	si->lowest_rix = 0;
	si->n_rates = MCS_GROUP_RATES;
	si->curr_rate = 0;
	si->success_last = true;
	
	/* IEEE80211_TX_RC_MCS -> if present use HT rates
	 * IEEE80211_TX_RC_40_MHZ_WIDTH -> if present use 40MHz channels (otherwise 20MHz ones)
	 * IEEE80211_TX_RC_SHORT_GI -> if present use short GI (otherwise standard GI)
	 * */
	si->flags = IEEE80211_TX_RC_MCS | IEEE80211_TX_RC_40_MHZ_WIDTH;

	for (i = 0; i < MCS_GROUP_RATES; i++) {
		struct sarf_rate *sr = &si->r[i];
		memset(sr, 0, sizeof(*sr));
		sr->rix = i;
		sr->success_count = 0;
		sr->failures_count = 0;
		
		calc_rate_durations(sband->band, sr);
	}
	
	si->trasm_index = 0;
	si->trasm_number = 0;
	memset(si->t, 0, sizeof(si->t));

	sarf_ht_update_rates(sp, si);
	return;
	
use_legacy:
	return mac80211_sarf.rate_init(priv, sband, chandef, sta,
					   priv_sta);
}


static void *
sarf_ht_alloc_sta(void *priv, struct ieee80211_sta *sta, gfp_t gfp)
{
	struct ieee80211_supported_band *sband;
	struct sarf_sta_info *si;
	struct sarf_priv *sp = priv;
	struct ieee80211_hw *hw = sp->hw;
	int max_rates = 0;
	int i;

	si = kzalloc(sizeof(struct sarf_sta_info), gfp);
	if (!si)
		return NULL;
	
	for (i = 0; i < IEEE80211_NUM_BANDS; i++) {
		sband = hw->wiphy->bands[i];
		if (sband && sband->n_bitrates > max_rates)
			max_rates = sband->n_bitrates;
	}

	si->r = kzalloc(sizeof(struct sarf_rate) * max_rates, gfp);
	if (!si->r)
		goto error;
		
	return si;
	
error:
	kfree(si);
	return NULL;
}

static void
sarf_ht_free_sta(void *priv, struct ieee80211_sta *sta, void *priv_sta)
{
	struct sarf_sta_info *si = priv_sta;
	
	kfree(si->r);
	kfree(si);
}

static void *
sarf_ht_alloc(struct ieee80211_hw *hw, struct dentry *debugfsdir)
{
	struct sarf_priv *sp;

	sp = kzalloc(sizeof(struct sarf_priv), GFP_ATOMIC);
	if (!sp)
		return NULL;

	if (hw->max_rate_tries > 0)
		sp->max_retry = hw->max_rate_tries;
	else
		/* safe default, does not necessarily have to match hw properties */
		sp->max_retry = 7;

	if (hw->max_rates >= 4)
		sp->has_mrr = true;

	sp->hw = hw;

	return sp;
}

static void
sarf_ht_free(void *priv)
{
	kfree(priv);
}

static u32 sarf_ht_get_expected_throughput(void *priv_sta)
{
	struct sarf_sta_info *si = priv_sta;
	u32 usecs;
	u32 tp;

	usecs = si->r[si->curr_rate].perfect_tx_time;
	if (!usecs)
		usecs = 1000000;
			
	tp = (1000000 / usecs);
	return SARF_TRUNC(tp) * 1200 * 8 / 1024;
	
}

const struct rate_control_ops mac80211_sarf_ht = {
	.name = "sarf_ht",
	.tx_status = sarf_ht_tx_status,
	.get_rate = sarf_ht_get_rate,
	.rate_init = sarf_ht_rate_init,
	.alloc = sarf_ht_alloc,
	.free = sarf_ht_free,
	.alloc_sta = sarf_ht_alloc_sta,
	.free_sta = sarf_ht_free_sta,
#ifdef CPTCFG_MAC80211_DEBUGFS
	.add_sta_debugfs = sarf_add_sta_debugfs,
	.remove_sta_debugfs = sarf_remove_sta_debugfs,
#endif
	.get_expected_throughput = sarf_ht_get_expected_throughput,
};

int __init
rc80211_sarf_ht_init(void)
{
	return ieee80211_rate_control_register(&mac80211_sarf_ht);
}

void
rc80211_sarf_ht_exit(void)
{
	ieee80211_rate_control_unregister(&mac80211_sarf_ht);
}
