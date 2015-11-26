/*
 * Copyright (C) 2010 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __RC_MINSTREL2_H
#define __RC_MINSTREL2_H

#define EWMA_LEVEL	96	/* ewma weighting factor [/EWMA_DIV] */
#define EWMA_DIV	128
#define SAMPLE_COLUMNS	10	/* number of columns in sample table */

/* scaled fraction values */
#define MINSTREL_SCALE  16
#define MINSTREL_FRAC(val, div) (((val) << MINSTREL_SCALE) / div)
#define MINSTREL_TRUNC(val) ((val) >> MINSTREL_SCALE)

#define MAX_THR_RATES 		4	/* number of highest throughput rates to consider*/
#define MCS_GROUP_RATES		8	
#define MINSTREL_ADDRESSES 10	/* number of addresses allowed */

/*
 * Perform EWMA (Exponentially Weighted Moving Average) calculation
 */
static inline int
minstrel2_ewma(int old, int new, int weight)
{
	int diff, incr;

	diff = new - old;
	incr = (EWMA_DIV - weight) * diff / EWMA_DIV;

	return old + incr;
}

/*
 * Perform EWMSD (Exponentially Weighted Moving Standard Deviation) calculation
 */
static inline int
minstrel2_ewmsd(int old_ewmsd, int cur_prob, int prob_ewma, int weight)
{
	int diff, incr, tmp_var;

	/* calculate exponential weighted moving variance */
	diff = MINSTREL_TRUNC((cur_prob - prob_ewma) * 1000000);
	incr = (EWMA_DIV - weight) * diff / EWMA_DIV;
	tmp_var = old_ewmsd * old_ewmsd;
	tmp_var = weight * (tmp_var + diff * incr / 1000000) / EWMA_DIV;

	/* return standard deviation */
	return (u16) int_sqrt(tmp_var);
}

struct minstrel_address {
	u8 add[ETH_ALEN];
};

struct minstrel_rate_stats {
	
	unsigned int duration;
	
	/* current / last sampling period attempts/success counters */
	u16 attempts[MINSTREL_ADDRESSES], last_attempts[MINSTREL_ADDRESSES];
	u16 success[MINSTREL_ADDRESSES], last_success[MINSTREL_ADDRESSES];

	/* total attempts/success counters */
	u64 att_hist[MINSTREL_ADDRESSES], succ_hist[MINSTREL_ADDRESSES];

	/* statistis of packet delivery probability
	 *  cur_prob  - current prob within last update intervall
	 *  prob_ewma - exponential weighted moving average of prob
	 *  prob_ewmsd - exp. weighted moving standard deviation of prob */
	unsigned int cur_prob[MINSTREL_ADDRESSES];
	unsigned int prob_ewma[MINSTREL_ADDRESSES];
	u16 prob_ewmsd[MINSTREL_ADDRESSES];

	/* maximum retry counts */
	u8 retry_count[MINSTREL_ADDRESSES];
	u8 retry_count_rtscts[MINSTREL_ADDRESSES];

	u8 sample_skipped[MINSTREL_ADDRESSES];
	bool retry_updated[MINSTREL_ADDRESSES];

};

struct minstrel_sta_info {
	
	struct ieee80211_sta *sta;

	/* ampdu length (average, per sampling interval) */
	unsigned int ampdu_len;
	unsigned int ampdu_packets;

	/* ampdu length (EWMA) */
	unsigned int avg_ampdu_len;

	/* overall sorted rate set */
	u16 max_tp_rate[MINSTREL_ADDRESSES][MAX_THR_RATES];
	u16 max_prob_rate[MINSTREL_ADDRESSES];

	/* time of last status update */
	unsigned long last_stats_update;

	/* overhead time in usec for each frame */
	unsigned int overhead;
	unsigned int overhead_rtscts;

	unsigned int total_packets;
	unsigned int sample_packets;

	bool isSampling;
	
	struct minstrel_address addresses[MINSTREL_ADDRESSES];
	u8 add_idx;
	u8 new_add_idx;

	/* tx flags to add for frames for this sta */
	u32 tx_flags;
	u32 flags;

	u8 sample_wait;
	u8 sample_tries;
	u8 sample_count;
	u8 sample_slow;
	u8 sample_row;
	u8 sample_column;

	/* MCS rate statistics */
	struct minstrel_rate_stats rates[MCS_GROUP_RATES];				

};

struct minstrel_priv {
	struct ieee80211_hw *hw;
	bool has_mrr;
	unsigned int cw_min;
	unsigned int cw_max;
	unsigned int max_retry;

#ifdef CPTCFG_MAC80211_DEBUGFS
	/*
	 * enable fixed rate processing per RC
	 *   - write static index to debugfs:ieee80211/phyX/rc/fixed_rate_idx
	 *   - write -1 to enable RC processing again
	 *   - setting will be applied on next update
	 */
	u32 fixed_rate_idx;
	struct dentry *dbg_fixed_rate;
	u32 lookaround_rate;			// AGGIUNTO
	struct dentry *dbg_lookaround_rate;		// AGGIUNTO
	u32 lookaround_rate_mrr;		// AGGIUNTO
	struct dentry *dbg_lookaround_rate_mrr;	// AGGIUNTO
	u32 segment_size;		// AGGIUNTO
	struct dentry *dbg_segment_size;	// AGGIUNTO
	u32 update_interval;		// AGGIUNTO
	struct dentry *dbg_update_interval;	// AGGIUNTO	
	u32 ref_payload_size;		// AGGIUNTO
	struct dentry *dbg_ref_payload_size;	// AGGIUNTO
#endif
};

extern const struct rate_control_ops mac80211_minstrel2;

#endif
