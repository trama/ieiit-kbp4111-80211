/*
 * Copyright (C) 2010-2013 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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
#include "rc80211_minstrel2.h"

/* MICHELE **********************************************/

extern data_rclog_t data_rclog_buf[RCLOG_BUF_DIM];
extern int rclog_in;
/* MICHELE **********************************************/

/* number of data bits per symbol */
const unsigned int nbps_minstrel2[] = {54, 108, 162, 216, 324, 432, 486, 540};

static u8 sample_table[SAMPLE_COLUMNS][MCS_GROUP_RATES] __read_mostly;
static u8 ref_size;

static void
minstrel2_update_rates(struct minstrel_priv *mp, struct minstrel_sta_info *mi);

/*
 * Return current throughput based on the average A-MPDU length, taking into
 * account the expected number of retransmissions and their expected length
 */
int
minstrel2_get_tp_avg(struct minstrel_sta_info *mi, int rate,
		       int prob_ewma)
{
	unsigned int nsecs = 0;

	/* do not account throughput if sucess prob is below 10% */
	if (prob_ewma < MINSTREL_FRAC(10, 100))
		return 0;

	nsecs = 1000 * mi->overhead / MINSTREL_TRUNC(mi->avg_ampdu_len);
	nsecs += mi->rates[rate].duration;

	/*
	 * For the throughput calculation, limit the probability value to 90% to
	 * account for collision related packet error rate fluctuation
	 * (prob is scaled - see MINSTREL_FRAC above)
	 */
	if (prob_ewma > MINSTREL_FRAC(90, 100))
		return MINSTREL_TRUNC(100000 * ((MINSTREL_FRAC(90, 100) * 1000)
								      / nsecs));
	else
		return MINSTREL_TRUNC(100000 * ((prob_ewma * 1000) / nsecs));
}

/*
 * Find & sort topmost throughput rates
 *
 * If multiple rates provide equal throughput the sorting is based on their
 * current success probability. Higher success probability is preferred among
 * MCS groups, CCK rates do not provide aggregation and are therefore at last.
 */
static void
minstrel2_sort_best_tp_rates(struct minstrel_sta_info *mi, u16 index, u16 *tp_list)
{
	int cur_tp_avg, cur_prob;
	int tmp_idx, tmp_tp_avg, tmp_prob;
	int j = MAX_THR_RATES;

	cur_prob = mi->rates[index].prob_ewma[mi->add_idx];
	cur_tp_avg = minstrel2_get_tp_avg(mi, index, cur_prob);

	do {
		tmp_idx = tp_list[j - 1];
		tmp_prob = mi->rates[tmp_idx].prob_ewma[mi->add_idx];
		tmp_tp_avg = minstrel2_get_tp_avg(mi, tmp_idx,tmp_prob);
		if (cur_tp_avg < tmp_tp_avg || 
		    (cur_tp_avg == tmp_tp_avg && cur_prob <= tmp_prob))
			break;
		j--;
	} while (j > 0);

	if (j < MAX_THR_RATES - 1) {
		memmove(&tp_list[j + 1], &tp_list[j], (sizeof(*tp_list) *
		       (MAX_THR_RATES - (j + 1))));
	}
	if (j < MAX_THR_RATES)
		tp_list[j] = index;
}

/*
 * Find and set the topmost probability rate per sta and per group
 */
static void
minstrel2_set_best_prob_rate(struct minstrel_sta_info *mi, u16 index, int add_idx)
{
	struct minstrel_rate_stats *mrs;
	int tmp_tp_avg, tmp_prob;
	int cur_tp_avg;

	mrs = &mi->rates[index];
	tmp_prob = mi->rates[mi->max_prob_rate[add_idx]].prob_ewma[add_idx];
	tmp_tp_avg = minstrel2_get_tp_avg(mi, mi->max_prob_rate[add_idx], tmp_prob);

	if (mrs->prob_ewma[mi->add_idx] > MINSTREL_FRAC(75, 100)) {
		cur_tp_avg = minstrel2_get_tp_avg(mi, index, mrs->prob_ewma[add_idx]);
		if (cur_tp_avg > tmp_tp_avg)
			mi->max_prob_rate[add_idx] = index;
	} else {
		if (mrs->prob_ewma[add_idx] > tmp_prob)
			mi->max_prob_rate[add_idx] = index;
	}
}

/*
* Recalculate statistics and counters of a given rate
*/
void
minstrel2_calc_rate_stats(struct minstrel_rate_stats *mrs, int idx)
{
	if (unlikely(mrs->attempts[idx] > 0)) {
		mrs->sample_skipped[idx] = 0;
		mrs->cur_prob[idx] = MINSTREL_FRAC(mrs->success[idx], mrs->attempts[idx]);
		if (unlikely(!mrs->att_hist[idx])) {
			mrs->prob_ewma[idx] = mrs->cur_prob[idx];
		} else {
			/* update exponential weighted moving variance */
			mrs->prob_ewmsd[idx] = minstrel2_ewmsd(mrs->prob_ewmsd[idx],
							 mrs->cur_prob[idx],
							 mrs->prob_ewma[idx],
							 EWMA_LEVEL);

			/*update exponential weighted moving avarage */
			mrs->prob_ewma[idx] = minstrel2_ewma(mrs->prob_ewma[idx],
						       mrs->cur_prob[idx],
						       EWMA_LEVEL);
		}
		mrs->att_hist[idx] += mrs->attempts[idx];
		mrs->succ_hist[idx] += mrs->success[idx];
	} else {
		mrs->sample_skipped[idx]++;
	}

	mrs->last_success[idx]= mrs->success[idx];
	mrs->last_attempts[idx] = mrs->attempts[idx];
	mrs->success[idx] = 0;
	mrs->attempts[idx] = 0;
}

/*
 * Update rate statistics and select new primary rates
 *
 * Rule for rate selection:
 *  - as long as the max prob rate has a probability of more than 75%, pick
 *    higher throughput rates, even if the probablity is a bit lower
 */
static void
minstrel2_update_stats(struct minstrel_priv *mp, struct minstrel_sta_info *mi)
{
	struct minstrel_rate_stats *mrs;
	int i, j, cur_prob;
	u16 tmp_mcs_tp_rate[MAX_THR_RATES];

	if (mi->ampdu_packets > 0) {
		mi->avg_ampdu_len = minstrel2_ewma(mi->avg_ampdu_len,
			MINSTREL_FRAC(mi->ampdu_len, mi->ampdu_packets), EWMA_LEVEL);
		mi->ampdu_len = 0;
		mi->ampdu_packets = 0;
	}

	mi->sample_slow = 0;
	mi->sample_count = 0;

	/* Initialize global rate indexes */
	for(j = 0; j < MAX_THR_RATES; j++){
		tmp_mcs_tp_rate[j] = 0;
	}

	mi->sample_count++;
	
	/* Find best rate sets */
	for (i = 0; i < MCS_GROUP_RATES; i++) {
		
		mrs = &mi->rates[i];
		for (j = 0; j < mi->new_add_idx; j++) {
			mrs->retry_updated[j] = false;
			minstrel2_calc_rate_stats(mrs, j);
			cur_prob = mrs->prob_ewma[j];

			if (minstrel2_get_tp_avg(mi, i, cur_prob) == 0)
				continue;

			/* Find max throughput rate set */
			minstrel2_sort_best_tp_rates(mi, i, tmp_mcs_tp_rate);

			/* Find max probability rate per group and global */
			minstrel2_set_best_prob_rate(mi, i, j); 
		}
	}
	
	for (j = 0; j < mi->new_add_idx; j++) {
		/* Assign new rate set per sta */
		memcpy(mi->max_tp_rate[j], tmp_mcs_tp_rate, sizeof(mi->max_tp_rate[j]));

#ifdef CPTCFG_MAC80211_DEBUGFS
		/* use fixed index if set */
		if (mp->fixed_rate_idx != -1) {
			for (i = 0; i < 4; i++)
				mi->max_tp_rate[j][i] = mp->fixed_rate_idx;
			mi->max_prob_rate[j] = mp->fixed_rate_idx;
		}
#endif

	}

	/* Reset update timer */
	mi->last_stats_update = jiffies;
}

static bool
minstrel2_txstat_valid(struct minstrel_priv *mp, struct ieee80211_tx_rate *rate)
{
	if (rate->idx < 0)
		return false;

	if (!rate->count)
		return false;

	return true;
}

static void
minstrel2_set_next_sample_idx(struct minstrel_sta_info *mi)
{

	for (;;) {
		if (++mi->sample_row >= MCS_GROUP_RATES) {
			mi->sample_row = 0;
			if (++mi->sample_column >= SAMPLE_COLUMNS)
				mi->sample_column = 0;
		}
		break;
	}
}

static void
minstrel2_tx_status(void *priv, struct ieee80211_supported_band *sband,
                      struct ieee80211_sta *sta, void *priv_sta,
                      struct ieee80211_tx_info *info)
{
	struct minstrel_sta_info *mi = priv_sta;
	struct ieee80211_tx_rate *ar = info->status.rates;
	struct minstrel_rate_stats *rate;
	struct minstrel_priv *mp = priv;
	bool last, update = false, null;
	int i, success, r = 0;

	/* This packet was aggregated but doesn't carry status info */
	if ((info->flags & IEEE80211_TX_CTL_AMPDU) &&
	    !(info->flags & IEEE80211_TX_STAT_AMPDU))
		return;
		
	success = !!(info->flags & IEEE80211_TX_STAT_ACK);
		
	null = !!(info->flags & IEEE80211_TX_CTL_USE_MINRATE);
	if (null)
	  return;

	if (!(info->flags & IEEE80211_TX_STAT_AMPDU)) {
		info->status.ampdu_ack_len =
			(info->flags & IEEE80211_TX_STAT_ACK ? 1 : 0);
		info->status.ampdu_len = 1;
	}

	mi->ampdu_packets++;
	mi->ampdu_len += info->status.ampdu_len;

	if (!mi->sample_wait && !mi->sample_tries && mi->sample_count > 0) {
		mi->sample_wait = 16 + 2 * MINSTREL_TRUNC(mi->avg_ampdu_len);
		mi->sample_tries = 1;
		mi->sample_count--;
	}

	if (info->flags & IEEE80211_TX_CTL_RATE_CTRL_PROBE)
		mi->sample_packets += info->status.ampdu_len;
		
	data_rclog_buf[rclog_in].success = success;
	data_rclog_buf[rclog_in].probe = mi->isSampling;  
	for (i = 0; i < 4; i++) {
		data_rclog_buf[rclog_in].rate_idx[i] = -1;
		data_rclog_buf[rclog_in].rate_count[i] = -1;
	}

	last = !minstrel2_txstat_valid(mp, &ar[0]);
	for (i = 0; !last; i++) {
		last = (i == IEEE80211_TX_MAX_RATES - 1) ||
		       !minstrel2_txstat_valid(mp, &ar[i + 1]);

		rate = &mi->rates[ar[i].idx];
		data_rclog_buf[rclog_in].rate_idx[r] = ar[i].idx;
		data_rclog_buf[rclog_in].rate_count[r] = ar[i].count;
		r++;
		
		if (last)
			rate->success[mi->add_idx] += info->status.ampdu_ack_len;

		rate->attempts[mi->add_idx] += ar[i].count * info->status.ampdu_len;
	}
	rclog_in = (rclog_in+1)%RCLOG_BUF_DIM;

	if (time_after(jiffies, mi->last_stats_update +
				(mp->update_interval / 2 * HZ) / 1000)) {
		update = true;
		minstrel2_update_stats(mp, mi);
	}

	if (update)
		minstrel2_update_rates(mp, mi);
}

static void
minstrel2_calc_retransmit(struct minstrel_priv *mp, struct minstrel_sta_info *mi,
                         int index)
{
	struct minstrel_rate_stats *mrs;
	unsigned int tx_time, tx_time_rtscts, tx_time_data;
	unsigned int cw = mp->cw_min;
	unsigned int ctime = 0;
	unsigned int t_slot = 9; /* FIXME */
	unsigned int ampdu_len = MINSTREL_TRUNC(mi->avg_ampdu_len);
	unsigned int overhead = 0, overhead_rtscts = 0;

	mrs = &mi->rates[index];
	if (mrs->prob_ewma[mi->add_idx] < MINSTREL_FRAC(1, 10)) {
		mrs->retry_count[mi->add_idx] = 1;
		mrs->retry_count_rtscts[mi->add_idx] = 1;
		return;
	}

	mrs->retry_count[mi->add_idx] = 2;
	mrs->retry_count_rtscts[mi->add_idx] = 2;
	mrs->retry_updated[mi->add_idx] = true;

	tx_time_data = mrs->duration * ampdu_len / 1000;

	/* Contention time for first 2 tries */
	ctime = (t_slot * cw) >> 1;
	cw = min((cw << 1) | 1, mp->cw_max);
	ctime += (t_slot * cw) >> 1;
	cw = min((cw << 1) | 1, mp->cw_max);
	
	overhead = mi->overhead;
	overhead_rtscts = mi->overhead_rtscts;

	/* Total TX time for data and Contention after first 2 tries */
	tx_time = ctime + 2 * (overhead + tx_time_data);
	tx_time_rtscts = ctime + 2 * (overhead_rtscts + tx_time_data);

	/* See how many more tries we can fit inside segment size */
	do {
		/* Contention time for this try */
		ctime = (t_slot * cw) >> 1;
		cw = min((cw << 1) | 1, mp->cw_max);

		/* Total TX time after this try */
		tx_time += ctime + overhead + tx_time_data;
		tx_time_rtscts += ctime + overhead_rtscts + tx_time_data;

		if (tx_time_rtscts < mp->segment_size)
			mrs->retry_count_rtscts[mi->add_idx]++;
	} while ((tx_time < mp->segment_size) &&
	         (++mrs->retry_count[mi->add_idx] < mp->max_retry));
}


static void
minstrel2_set_rate(struct minstrel_priv *mp, struct minstrel_sta_info *mi,
                     struct ieee80211_sta_rates *ratetbl, int offset, int index)
{
	struct minstrel_rate_stats *mrs;
	u16 flags = mi->flags;

	mrs = &mi->rates[index];
	if (!mrs->retry_updated[mi->add_idx])
		minstrel2_calc_retransmit(mp, mi, index);

	if (mrs->prob_ewma[mi->add_idx] < MINSTREL_FRAC(20, 100) || !mrs->retry_count[mi->add_idx]) {
		ratetbl->rate[offset].count = 2;
		ratetbl->rate[offset].count_rts = 2;
		ratetbl->rate[offset].count_cts = 2;
	} else {
		ratetbl->rate[offset].count = mrs->retry_count[mi->add_idx];
		ratetbl->rate[offset].count_cts = mrs->retry_count[mi->add_idx];
		ratetbl->rate[offset].count_rts = mrs->retry_count_rtscts[mi->add_idx];
	}

	if (offset > 0) {
		ratetbl->rate[offset].count = ratetbl->rate[offset].count_rts;
		flags |= IEEE80211_TX_RC_USE_RTS_CTS;
	}

	ratetbl->rate[offset].idx = index;
	ratetbl->rate[offset].flags = flags;
}

static void
minstrel2_update_rates(struct minstrel_priv *mp, struct minstrel_sta_info *mi)
{
	struct ieee80211_sta_rates *rates;
	int i = 0;

	rates = kzalloc(sizeof(*rates), GFP_ATOMIC);
	if (!rates)
		return;

	/* Start with max_tp_rate[0] */
	minstrel2_set_rate(mp, mi, rates, i++, mi->max_tp_rate[mi->add_idx][0]);

	if (mp->hw->max_rates >= 3) {
		/* At least 3 tx rates supported, use max_tp_rate[1] next */
		minstrel2_set_rate(mp, mi, rates, i++, mi->max_tp_rate[mi->add_idx][1]);
	}

	if (mp->hw->max_rates >= 2) {
		/*
		 * At least 2 tx rates supported, use max_prob_rate next */
		minstrel2_set_rate(mp, mi, rates, i++, mi->max_prob_rate[mi->add_idx]);
	}

	rates->rate[i].idx = -1;
	rate_control_set_rates(mp->hw, mi->sta, rates);
}

static inline int
minstrel2_get_duration(int index)
{
	return (DIV_ROUND_UP(ref_size*8,nbps_minstrel2[index]) * 4000);
}

static int
minstrel2_get_sample_rate(struct minstrel_priv *mp, struct minstrel_sta_info *mi)
{
	struct minstrel_rate_stats *mrs;
	unsigned int sample_dur;
	int sample_idx = 0;

	if (mi->sample_wait > 0) {
		mi->sample_wait--;
		return -1;
	}

	if (!mi->sample_tries)
		return -1;

	sample_idx = sample_table[mi->sample_column][mi->sample_row];
	minstrel2_set_next_sample_idx(mi);

	mrs = &mi->rates[sample_idx];

	/*
	 * Sampling might add some overhead (RTS, no aggregation)
	 * to the frame. Hence, don't use sampling for the currently
	 * used rates.
	 */
	if (sample_idx == mi->max_tp_rate[mi->add_idx][0] ||
	    sample_idx == mi->max_tp_rate[mi->add_idx][1] ||
	    sample_idx == mi->max_prob_rate[mi->add_idx])
		return -1;

	/*
	 * Do not sample if the probability is already higher than 95%
	 * to avoid wasting airtime.
	 */
	if (mrs->prob_ewma[mi->add_idx] > MINSTREL_FRAC(95, 100))
		return -1;

	/*
	 * Make sure that lower rates get sampled only occasionally,
	 * if the link is working perfectly.
	 */
	sample_dur = minstrel2_get_duration(sample_idx);
	if (sample_dur >= minstrel2_get_duration(mi->max_tp_rate[mi->add_idx][1]) &&
	    (sample_dur >= minstrel2_get_duration(mi->max_prob_rate[mi->add_idx]))) {
		if (mrs->sample_skipped[mi->add_idx] < 20)
			return -1;

		if (mi->sample_slow++ > 2)
			return -1;
	}
	mi->sample_tries--;

	return sample_idx;
}

static void
minstrel2_get_rate(void *priv, struct ieee80211_sta *sta, void *priv_sta,
                     struct ieee80211_tx_rate_control *txrc)
{
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(txrc->skb);
	struct ieee80211_tx_rate *rate = &info->status.rates[0]; 
	struct minstrel_sta_info *mi = priv_sta;
	struct minstrel_priv *mp = priv;
	struct sk_buff *skb = txrc->skb;
	struct ieee80211_hdr *hdr = (void *)skb->data;
	u8 dst[ETH_ALEN];
	bool found = false;
	int record, i, sample_idx;
	
	/* Management/no-ack frames do not use rate control */
	if (rate_control_send_low(sta, priv_sta, txrc))
		return;
		
	/* Initialize */
	mi->isSampling = false; 

	/* Do not sample if rate is fixed */
#ifdef CPTCFG_MAC80211_DEBUGFS
	if (mp->fixed_rate_idx != -1)
		return;
#endif

	/* Enable STBC if possible for data frames */
	info->flags |= mi->tx_flags;
	
	/* Search destination among already recorded addresses */
	memcpy(dst, ieee80211_get_DA(hdr), ETH_ALEN);
	for (record = 0; record < mi->new_add_idx; record++) {
		found = true;
		for (i = 0; i < ETH_ALEN; i++){
			if (mi->addresses[record].add[i] != dst[i]) {
				found = false;  
				break;
			}
		}
		if (found) break;
	}
	
	/* If necessary, add new address */
	if (!found) {
		record = mi->new_add_idx;
		mi->new_add_idx = (mi->new_add_idx + 1) % MINSTREL_ADDRESSES;
		for (i = 0; i < ETH_ALEN; i++)	{
			mi->addresses[record].add[i] = dst[i];
		}	  
	}
	
	/* Update add_idx */
	mi->add_idx = record;
	printk("MICHELE: get_rate called with packet for %2x:%2x:%2x:%2x:%2x:%2x, add_idx=%u, new_add_idx=%u\n",dst[0],dst[1],dst[2],dst[3],dst[4],dst[5],mi->add_idx,mi->new_add_idx);
	
	/* Retrieve next sample index */
	if (mp->hw->max_rates == 1 &&
	    (info->control.flags & IEEE80211_TX_CTRL_PORT_CTRL_PROTO))		// Don't use EAPOL frames for sampling on non-mrr hw 
		sample_idx = -1;					
	else
		sample_idx = minstrel2_get_sample_rate(mp, mi);

	/* Increase sum packet counter */
	mi->total_packets++;
	if (mi->total_packets == ~0) {
		mi->total_packets = 0;
		mi->sample_packets = 0;
	}

	/* Not sampling */
	if (sample_idx < 0) 
		return;

	/* Sampling */
	mi->isSampling = true;
	info->flags |= IEEE80211_TX_CTL_RATE_CTRL_PROBE;
	rate->count = 1;
	rate->idx = sample_idx;
	rate->flags = mi->flags;

}

static void
minstrel2_update_caps(void *priv, struct ieee80211_supported_band *sband,
			struct cfg80211_chan_def *chandef,
                        struct ieee80211_sta *sta, void *priv_sta)
{
	struct minstrel_priv *mp = priv;
	struct minstrel_sta_info *mi = priv_sta;
	u16 sta_cap = sta->ht_cap.cap;
	int ack_dur;
	int stbc;
	int i;

	/* Check that station supports HT and 40MHz channels */
	if ((!sta->ht_cap.ht_supported) || (sta->bandwidth < IEEE80211_STA_RX_BW_40)) {
		printk("HT not supported by STA\n");
		return;
	}
	
	/* Check for STBC and LDPC capabilities and update tx flags */
	stbc = (sta_cap & IEEE80211_HT_CAP_RX_STBC) >>
			IEEE80211_HT_CAP_RX_STBC_SHIFT;
	mi->tx_flags |= stbc << IEEE80211_TX_CTL_STBC_SHIFT;
	if (sta_cap & IEEE80211_HT_CAP_LDPC_CODING)
		mi->tx_flags |= IEEE80211_TX_CTL_LDPC;

	/* Initialize variables */
	mi->sta = sta;
	mi->last_stats_update = jiffies;
	ack_dur = ieee80211_frame_duration(sband->band, 10, 60, 1, 1, 0);
	mi->overhead = ieee80211_frame_duration(sband->band, 0, 60, 1, 1, 0);
	mi->overhead += ack_dur;
	mi->overhead_rtscts = mi->overhead + 2 * ack_dur;
	mi->avg_ampdu_len = MINSTREL_FRAC(1, 1);
	mi->add_idx = 0;
	mi->new_add_idx = 0;

	/* Sampling variables: when using MRR, sample more on the first attempt, without delay */
	if (mp->has_mrr) {
		mi->sample_count = 16;
		mi->sample_wait = 0;
	} else {
		mi->sample_count = 8;
		mi->sample_wait = 8;
	}
	mi->sample_tries = 4;
	mi->sample_column = 0;
	mi->sample_row = 0;

	/* Define flags: 
	 * IEEE80211_TX_RC_MCS -> if present use HT rates
	 * IEEE80211_TX_RC_40_MHZ_WIDTH -> if present use 40MHz channels (otherwise 20MHz ones)
	 * IEEE80211_TX_RC_SHORT_GI -> if present use short GI (otherwise standard GI)
	 * */
	mi->flags = IEEE80211_TX_RC_MCS | IEEE80211_TX_RC_40_MHZ_WIDTH;

	/* Allocate rates */
	for (i = 0; i < MCS_GROUP_RATES; i++) {
		struct minstrel_rate_stats *mr = &mi->rates[i];
		memset(mr, 0, sizeof(*mr));
		
		mr->duration = minstrel2_get_duration(i);
		memset(mr->attempts, 0, sizeof(mr->attempts));
		memset(mr->last_attempts, 0, sizeof(mr->last_attempts));
		memset(mr->success, 0, sizeof(mr->success));
		memset(mr->last_success, 0, sizeof(mr->last_success));
		memset(mr->att_hist, 0, sizeof(mr->att_hist));
		memset(mr->succ_hist, 0, sizeof(mr->succ_hist));
		memset(mr->cur_prob, 0, sizeof(mr->cur_prob));
		memset(mr->prob_ewma, 0, sizeof(mr->prob_ewma));
		memset(mr->prob_ewmsd, 0, sizeof(mr->prob_ewmsd));
		memset(mr->retry_count, 0, sizeof(mr->retry_count));
		memset(mr->retry_count_rtscts, 0, sizeof(mr->retry_count_rtscts));
		memset(mr->sample_skipped, 0, sizeof(mr->sample_skipped));
		memset(mr->retry_updated, 0, sizeof(mr->retry_updated));
	}

	/* Create an initial rate table with the lowest supported rates */
	minstrel2_update_stats(mp, mi);
	minstrel2_update_rates(mp, mi);

}

static void
minstrel2_rate_init(void *priv, struct ieee80211_supported_band *sband,
		      struct cfg80211_chan_def *chandef,
                      struct ieee80211_sta *sta, void *priv_sta)
{
	minstrel2_update_caps(priv, sband, chandef, sta, priv_sta);
}

static void
minstrel2_rate_update(void *priv, struct ieee80211_supported_band *sband,
			struct cfg80211_chan_def *chandef,
                        struct ieee80211_sta *sta, void *priv_sta,
                        u32 changed)
{
	minstrel2_update_caps(priv, sband, chandef, sta, priv_sta);
}

static void *
minstrel2_alloc_sta(void *priv, struct ieee80211_sta *sta, gfp_t gfp)
{
	struct minstrel_sta_info *mi;
	printk("MICHELE: alloc_sta\n");

	mi = kzalloc(sizeof(*mi), gfp);
	if (!mi)
		return NULL;

	return mi;

}

static void
minstrel2_free_sta(void *priv, struct ieee80211_sta *sta, void *priv_sta)
{
	struct minstrel_sta_info *mi= priv_sta;
	
	kfree(mi);
}

static void *
minstrel2_alloc(struct ieee80211_hw *hw, struct dentry *debugfsdir)
{
	struct minstrel_priv *mp;
	mp = kzalloc(sizeof(struct minstrel_priv), GFP_ATOMIC);
	if (!mp)
		return NULL;

	/* contention window settings
	 * Just an approximation. Using the per-queue values would complicate
	 * the calculations and is probably unnecessary */
	mp->cw_min = 15;
	mp->cw_max = 1023;

	if (hw->max_rate_tries > 0)
		mp->max_retry = hw->max_rate_tries;
	else
		/* safe default, does not necessarily have to match hw properties */
		mp->max_retry = 7;

	if (hw->max_rates >= 4)
		mp->has_mrr = true;

	mp->hw = hw;

#ifdef CPTCFG_MAC80211_DEBUGFS
	mp->fixed_rate_idx = (u32) -1;
	mp->dbg_fixed_rate = debugfs_create_u32("fixed_rate_idx",
			S_IRUGO | S_IWUGO, debugfsdir, &mp->fixed_rate_idx);
	mp->lookaround_rate = (u32) 5;												// AGGIUNTO
	mp->dbg_lookaround_rate = debugfs_create_u32("lookaround_rate",				// AGGIUNTO
			S_IRUGO | S_IWUGO, debugfsdir, &mp->lookaround_rate);				// AGGIUNTO
	mp->lookaround_rate_mrr = (u32) 10;											// AGGIUNTO
	mp->dbg_lookaround_rate_mrr = debugfs_create_u32("lookaround_rate_mrr",		// AGGIUNTO
			S_IRUGO | S_IWUGO, debugfsdir, &mp->lookaround_rate_mrr);			// AGGIUNTO
	mp->segment_size = (u32) 6000;	  											// AGGIUNTO
	mp->dbg_segment_size = debugfs_create_u32("segment_size",					// AGGIUNTO
			S_IRUGO | S_IWUGO, debugfsdir, &mp->segment_size);					// AGGIUNTO
	mp->update_interval = (u32) 100;											// AGGIUNTO
	mp->dbg_update_interval = debugfs_create_u32("update_interval",				// AGGIUNTO
			S_IRUGO | S_IWUGO, debugfsdir, &mp->update_interval);				// AGGIUNTO
	mp->ref_payload_size = (u32) 1200;											// AGGIUNTO
	mp->dbg_ref_payload_size = debugfs_create_u32("ref_payload_size",			// AGGIUNTO
			S_IRUGO | S_IWUGO, debugfsdir, &mp->ref_payload_size);				// AGGIUNTO
	ref_size = mp->ref_payload_size;
#else
	ref_size = 1200;
#endif

	return mp;
}

static void
minstrel2_free(void *priv)
{
#ifdef CPTCFG_MAC80211_DEBUGFS
	debugfs_remove(((struct minstrel_priv *)priv)->dbg_fixed_rate);
#endif
	kfree(priv);
}

static u32 minstrel2_get_expected_throughput(void *priv_sta)
{
	struct minstrel_sta_info *mi = priv_sta;
	int prob, tp_avg;

	prob = mi->rates[mi->max_tp_rate[mi->add_idx][0]].prob_ewma[mi->add_idx];

	/* convert tp_avg from pkt per second in kbps */
	tp_avg = minstrel2_get_tp_avg(mi, mi->max_tp_rate[mi->add_idx][0], prob) * ref_size * 8 / 1024;

	return tp_avg;
}

const struct rate_control_ops mac80211_minstrel2 = {
	.name = "minstrel2",
	.tx_status_noskb = minstrel2_tx_status,
	.get_rate = minstrel2_get_rate,
	.rate_init = minstrel2_rate_init,
	.rate_update = minstrel2_rate_update,
	.alloc_sta = minstrel2_alloc_sta,
	.free_sta = minstrel2_free_sta,
	.alloc = minstrel2_alloc,
	.free = minstrel2_free,
	.get_expected_throughput = minstrel2_get_expected_throughput,
};

static void __init init_sample_table(void)
{
	int col, i, new_idx;
	u8 rnd[MCS_GROUP_RATES];

	memset(sample_table, 0xff, sizeof(sample_table));
	for (col = 0; col < SAMPLE_COLUMNS; col++) {
		prandom_bytes(rnd, sizeof(rnd));
		for (i = 0; i < MCS_GROUP_RATES; i++) {
			new_idx = (i + rnd[i]) % MCS_GROUP_RATES;
			while (sample_table[col][new_idx] != 0xff)
				new_idx = (new_idx + 1) % MCS_GROUP_RATES;

			sample_table[col][new_idx] = i;
		}
	}
}

int __init
rc80211_minstrel2_init(void)
{
	init_sample_table();
	return ieee80211_rate_control_register(&mac80211_minstrel2);
}

void
rc80211_minstrel2_exit(void)
{
	ieee80211_rate_control_unregister(&mac80211_minstrel2);
}
