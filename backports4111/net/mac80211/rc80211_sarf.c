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
#include <linux/ieee80211.h>
#include <linux/slab.h>
#include <net/mac80211.h>
#include "rate.h"
#include "rc80211_sarf.h"

/* MICHELE **********************************************/

extern data_rclog_t data_rclog_buf[RCLOG_BUF_DIM];
extern int rclog_in;
/* MICHELE **********************************************/

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
sarf_higher_rate(struct sarf_sta_info *si, int curr_rate)
{
	if (curr_rate < si->n_rates - 1) 
		return curr_rate + 1;
	
	return curr_rate;
}

static int
sarf_lower_rate(struct sarf_sta_info *si, int curr_rate)
{
	if (curr_rate > 0) 
		return curr_rate - 1;
	
	return curr_rate;
}

static void
sarf_update_rates(struct sarf_priv *sp, struct sarf_sta_info *si)
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
	//printk("Curr rate = %i, succ_last = %i, succ_count = %i, fail_count =%i\n",prev_rate,(int) si->success_last,succ,fail);
	if ((!si->success_last) && (fail==K_SARF)) {
		si->r[prev_rate].failures_count = 0;
		next_rate = sarf_lower_rate(si, prev_rate);
		si->curr_rate = next_rate;
	}	
	if ((si->success_last) && (succ==N_SARF)) {
		si->r[prev_rate].success_count = 0;
		next_rate = sarf_higher_rate(si, prev_rate);
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

	/* Use lowest rate then */
	ratetbl->rate[1].idx = si->lowest_rix;
	ratetbl->rate[1].count = sp->max_retry;
	ratetbl->rate[1].count_cts = sp->max_retry;
	ratetbl->rate[1].count_rts = sp->max_retry;

	rate_control_set_rates(sp->hw, si->sta, ratetbl);
	
	return;
}

static void
sarf_tx_status(void *priv, struct ieee80211_supported_band *sband,
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
	sarf_update_rates(sp, si);
}

static void
sarf_get_rate(void *priv, struct ieee80211_sta *sta,
		  void *priv_sta, struct ieee80211_tx_rate_control *txrc)
{
	//struct sk_buff *skb = txrc->skb;
	//struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	//struct ieee80211_tx_rate *rate = &info->control.rates[0];
	//struct sarf_sta_info *si = priv_sta;

	/* management/no-ack frames do not use rate control */
	if (rate_control_send_low(sta, priv_sta, txrc))
		return;

	//rate->idx = si->r[si->curr_rate].rix;
	//rate->count = 1;
}

static void
calc_rate_durations(enum ieee80211_band band,
		    struct sarf_rate *d,
		    struct ieee80211_rate *rate,
		    struct cfg80211_chan_def *chandef)
{
	int erp = !!(rate->flags & IEEE80211_RATE_ERP_G);
	int shift = ieee80211_chandef_get_shift(chandef);

	d->perfect_tx_time = ieee80211_frame_duration(band, 1200,
			DIV_ROUND_UP(rate->bitrate, 1 << shift), erp, 1,
			shift);
}

static void
sarf_rate_init(void *priv, struct ieee80211_supported_band *sband,
		   struct cfg80211_chan_def *chandef,
		   struct ieee80211_sta *sta, void *priv_sta)
{
	struct sarf_sta_info *si = priv_sta;
	struct sarf_priv *sp = priv;
	struct ieee80211_rate *ctl_rate;
	unsigned int i, n = 0;
	u32 rate_flags;

	si->sta = sta;
	si->lowest_rix = rate_lowest_index(sband, sta);
	ctl_rate = &sband->bitrates[si->lowest_rix];

	rate_flags = ieee80211_chandef_rate_flags(&sp->hw->conf.chandef);
	si->curr_rate = 0;
	si->success_last = true;

	for (i = 0; i < sband->n_bitrates; i++) {
		struct sarf_rate *sr = &si->r[n];

		if (!rate_supported(sta, sband->band, i))
			continue;
		if ((rate_flags & sband->bitrates[i].flags) != rate_flags)
			continue;

		n++;
		memset(sr, 0, sizeof(*sr));
		sr->rix = i;
		
		sr->success_count = 0;
		sr->failures_count = 0;
		
		calc_rate_durations(sband->band, sr, &sband->bitrates[i],
				    chandef);	
	}

	for (i = n; i < sband->n_bitrates; i++) {
		struct sarf_rate *sr = &si->r[i];
		sr->rix = -1;
	}
	
	si->trasm_index = 0;
	si->trasm_number = 0;
	memset(si->t, 0, sizeof(si->t));

	si->n_rates = n;
	sarf_update_rates(sp, si);
}

static void *
sarf_alloc_sta(void *priv, struct ieee80211_sta *sta, gfp_t gfp)
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
sarf_free_sta(void *priv, struct ieee80211_sta *sta, void *priv_sta)
{
	struct sarf_sta_info *si = priv_sta;
	
	//kfree(si->t);
	kfree(si->r);
	kfree(si);
}

static void
sarf_init_cck_rates(struct sarf_priv *sp)
{
	static const int bitrates[4] = { 10, 20, 55, 110 };
	struct ieee80211_supported_band *sband;
	u32 rate_flags = ieee80211_chandef_rate_flags(&sp->hw->conf.chandef);
	int i, j;

	sband = sp->hw->wiphy->bands[IEEE80211_BAND_2GHZ];
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

			sp->cck_rates[j] = i;
			break;
		}
	}
}

static void *
sarf_alloc(struct ieee80211_hw *hw, struct dentry *debugfsdir)
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
	
	sarf_init_cck_rates(sp);

	return sp;
}

static void
sarf_free(void *priv)
{
	kfree(priv);
}

static u32 sarf_get_expected_throughput(void *priv_sta)
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

const struct rate_control_ops mac80211_sarf = {
	.name = "sarf",
	.tx_status = sarf_tx_status,
	.get_rate = sarf_get_rate,
	.rate_init = sarf_rate_init,
	.alloc = sarf_alloc,
	.free = sarf_free,
	.alloc_sta = sarf_alloc_sta,
	.free_sta = sarf_free_sta,
#ifdef CPTCFG_MAC80211_DEBUGFS
	.add_sta_debugfs = sarf_add_sta_debugfs,
	.remove_sta_debugfs = sarf_remove_sta_debugfs,
#endif
	.get_expected_throughput = sarf_get_expected_throughput,
};

int __init
rc80211_sarf_init(void)
{
	return ieee80211_rate_control_register(&mac80211_sarf);
}

void
rc80211_sarf_exit(void)
{
	ieee80211_rate_control_unregister(&mac80211_sarf);
}
