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
#include "rc80211_farf.h"
#include "rc80211_farf_ht.h"

/* MICHELE **********************************************/

extern data_rclog_t data_rclog_buf[RCLOG_BUF_DIM];
extern int rclog_in;
/* MICHELE **********************************************/

/* number of data bits per symbol */
const unsigned int nbps_farf[] = {54, 108, 162, 216, 324, 432, 486, 540};

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

static int
farf_ht_next_rate(struct farf_sta_info *fi, int curr_rate)
{
	if (curr_rate < fi->n_rates - 1) 
		return curr_rate + 1;
	
	return curr_rate;
}

static void
farf_ht_update_rates(struct farf_priv *fp, struct farf_sta_info *fi)
{
	struct ieee80211_sta_rates *ratetbl;
	int succ;
	
	ratetbl = kzalloc(sizeof(*ratetbl), GFP_ATOMIC);
	if (!ratetbl)
		return;
		
	/* Update current rate*/
	if (!fi->success_last) {
		fi->curr_rate = rix_to_ndx(fi, (int) fi->lowest_rix);
	}	
	succ = fi->r[fi->curr_rate].success_count;
	if (succ == N_FARF) {
		fi->r[fi->curr_rate].success_count = 0;
		fi->curr_rate = farf_ht_next_rate(fi, fi->curr_rate);
	} 
	
	/* Use current rate once */
	ratetbl->rate[0].idx = fi->r[fi->curr_rate].rix;
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

	rate_control_set_rates(fp->hw, fi->sta, ratetbl);
}

static void
farf_ht_tx_status(void *priv, struct ieee80211_supported_band *sband,
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
	int t_idx;				
	bool first_succ = false, null;

	success = !!(info->flags & IEEE80211_TX_STAT_ACK);
	null = !!(info->flags & IEEE80211_TX_CTL_USE_MINRATE);
	
	if (null)
	  return;
	  
	t_idx = fi->trasm_index;	
	
	fi->t[t_idx].success = success;
	data_rclog_buf[rclog_in].success = success;
	data_rclog_buf[rclog_in].probe = 0;  
	for (i = 0; i < 4; i++) {
		fi->t[t_idx].rate_idx[i] = -1;
		fi->t[t_idx].rate_count[i] = -1;
		data_rclog_buf[rclog_in].rate_idx[i] = -1;
		data_rclog_buf[rclog_in].rate_count[i] = -1;
	}
	
	for (i = 0; i < IEEE80211_TX_MAX_RATES; i++) {
		if (ar[i].idx < 0)
			break;

		ndx = rix_to_ndx(fi, ar[i].idx);
		if (ndx < 0)
			continue;
		
		count = ar[i].count;
		if (i==0) {
			first_succ = true;
		}	
				
		fi->t[t_idx].rate_idx[r] = ndx;
		fi->t[t_idx].rate_count[r] = count;
		data_rclog_buf[rclog_in].rate_idx[r] = ndx;
		data_rclog_buf[rclog_in].rate_count[r] = count;
		r++;				
		
		if (i != 0) {		
			first_succ = false;
		}
	}
	
	if (!first_succ) {
		if (fi->curr_rate != rix_to_ndx(fi, (int) fi->lowest_rix))  {
			fi->r[fi->curr_rate].success_count = 0;
		} else {
			fi->r[fi->curr_rate].success_count = success;
		}
	} else {
		fi->r[fi->curr_rate].success_count += 1;
	}
	//printk("MICHELE: curr_rate=%u\n",fi->curr_rate);
	
	t_idx = (t_idx + 1) % TX_RECORDED_FARF;
	fi->trasm_index = t_idx;
	fi->trasm_number++;
	rclog_in = (rclog_in+1)%RCLOG_BUF_DIM;
		
	fi->success_last = first_succ;
	farf_ht_update_rates(fp, fi);
}

static void
farf_ht_get_rate(void *priv, struct ieee80211_sta *sta,
		  void *priv_sta, struct ieee80211_tx_rate_control *txrc)
{
	/* Enable STBC if possible */
	struct farf_sta_info *fi = priv_sta;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(txrc->skb);
	info->flags |= fi->tx_flags;
	
	/* management/no-ack frames do not use rate control */
	if (rate_control_send_low(sta, priv_sta, txrc))
		return;
}

static void
calc_rate_durations(enum ieee80211_band band, struct farf_rate *r)
{	
	r->perfect_tx_time = (DIV_ROUND_UP(1200*8,nbps_farf[r->rix]) * 4) + ieee80211_frame_duration(band, 0, 60, 1, 1, 0);
}

static void
farf_ht_rate_init(void *priv, struct ieee80211_supported_band *sband,
		   struct cfg80211_chan_def *chandef,
		   struct ieee80211_sta *sta, void *priv_sta)
{
	struct farf_sta_info *fi = priv_sta;
	struct farf_priv *fp = priv;
	u16 sta_cap = sta->ht_cap.cap;
	unsigned int i, stbc;
	
	/* fall back to the old FARF for legacy stations */
	if (!sta->ht_cap.ht_supported)
		goto use_legacy;
	
	/* Check that 40 MHz is supported */
	if (sta->bandwidth < IEEE80211_STA_RX_BW_40)
		goto use_legacy;
		
	/* Check for STBC and LDPC capabilities and update tx flags */
	stbc = (sta_cap & IEEE80211_HT_CAP_RX_STBC) >>
			IEEE80211_HT_CAP_RX_STBC_SHIFT;
	fi->tx_flags |= stbc << IEEE80211_TX_CTL_STBC_SHIFT;

	if (sta_cap & IEEE80211_HT_CAP_LDPC_CODING)
		fi->tx_flags |= IEEE80211_TX_CTL_LDPC;

	fi->sta = sta;
	fi->lowest_rix = 0;
	fi->n_rates = MCS_GROUP_RATES;
	fi->curr_rate = 0;
	fi->success_last = true;
	
	/* IEEE80211_TX_RC_MCS -> if present use HT rates
	 * IEEE80211_TX_RC_40_MHZ_WIDTH -> if present use 40MHz channels (otherwise 20MHz ones)
	 * IEEE80211_TX_RC_SHORT_GI -> if present use short GI (otherwise standard GI)
	 * */
	fi->flags = IEEE80211_TX_RC_MCS | IEEE80211_TX_RC_40_MHZ_WIDTH;

	for (i = 0; i < MCS_GROUP_RATES; i++) {
		struct farf_rate *fr = &fi->r[i];
		memset(fr, 0, sizeof(*fr));
		fr->rix = i;
		fr->success_count = 0;
		
		calc_rate_durations(sband->band, fr);
	}
	
	fi->trasm_index = 0;
	fi->trasm_number = 0;
	memset(fi->t, 0, sizeof(fi->t));

	farf_ht_update_rates(fp, fi);
	return;
	
use_legacy:
	return mac80211_farf.rate_init(priv, sband, chandef, sta,
					   priv_sta);
}

static void *
farf_ht_alloc_sta(void *priv, struct ieee80211_sta *sta, gfp_t gfp)
{
	struct ieee80211_supported_band *sband;
	struct farf_sta_info *fi;
	struct farf_priv *fp = priv;
	struct ieee80211_hw *hw = fp->hw;
	int max_rates = 0;
	int i;

	fi = kzalloc(sizeof(struct farf_sta_info), gfp);
	if (!fi)
		return NULL;
	
	for (i = 0; i < IEEE80211_NUM_BANDS; i++) {
		sband = hw->wiphy->bands[i];
		if (sband && sband->n_bitrates > max_rates)
			max_rates = sband->n_bitrates;
	}

	fi->r = kzalloc(sizeof(struct farf_rate) * max_rates, gfp);
	if (!fi->r)
		goto error;
		
	return fi;
	
error:
	kfree(fi);
	return NULL;
}

static void
farf_ht_free_sta(void *priv, struct ieee80211_sta *sta, void *priv_sta)
{
	struct farf_sta_info *fi = priv_sta;
	
	kfree(fi->r);
	kfree(fi);
}

static void *
farf_ht_alloc(struct ieee80211_hw *hw, struct dentry *debugfsdir)
{
	struct farf_priv *fp;

	fp = kzalloc(sizeof(struct farf_priv), GFP_ATOMIC);
	if (!fp)
		return NULL;

	if (hw->max_rate_tries > 0)
		fp->max_retry = hw->max_rate_tries;
	else
		/* safe default, does not necessarily have to match hw properties */
		fp->max_retry = 7;

	if (hw->max_rates >= 4)
		fp->has_mrr = true;

	fp->hw = hw;

	return fp;
}

static void
farf_ht_free(void *priv)
{
	kfree(priv);
}

static u32 farf_ht_get_expected_throughput(void *priv_sta)
{
	struct farf_sta_info *fi = priv_sta;
	u32 usecs;
	u32 tp;

	usecs = fi->r[fi->curr_rate].perfect_tx_time;
	if (!usecs)
		usecs = 1000000;
			
	tp = (1000000 / usecs);
	return FARF_TRUNC(tp) * 1200 * 8 / 1024;
	
}

const struct rate_control_ops mac80211_farf_ht = {
	.name = "farf_ht",
	.tx_status = farf_ht_tx_status,
	.get_rate = farf_ht_get_rate,
	.rate_init = farf_ht_rate_init,
	.alloc = farf_ht_alloc,
	.free = farf_ht_free,
	.alloc_sta = farf_ht_alloc_sta,
	.free_sta = farf_ht_free_sta,
#ifdef CPTCFG_MAC80211_DEBUGFS
	.add_sta_debugfs = farf_add_sta_debugfs,
	.remove_sta_debugfs = farf_remove_sta_debugfs,
#endif
	.get_expected_throughput = farf_ht_get_expected_throughput,
};

int __init
rc80211_farf_ht_init(void)
{
	return ieee80211_rate_control_register(&mac80211_farf_ht);
}

void
rc80211_farf_ht_exit(void)
{
	ieee80211_rate_control_unregister(&mac80211_farf_ht);
}

