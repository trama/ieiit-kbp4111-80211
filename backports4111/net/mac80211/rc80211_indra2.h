
/*
 * Copyright (C) 2008 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __RC_INDRA2_H
#define __RC_INDRA2_H

#define MCS_RATES	8
#define FORG_FACTOR 80
#define MEMORY_SIZE 8	
#define SNR_RANGE	30

#define INDRA_SCALE  16
#define INDRA_FRAC(val, div) (((val) << INDRA_SCALE) / div)
#define INDRA_TRUNC(val) ((val) >> INDRA_SCALE)

struct indra_rate {
	int rix;
	
	/* current attempts/success counters */
	u16 attempts[MEMORY_SIZE], success[MEMORY_SIZE];
	
	/* total attempts/success counters */
	u64 att_hist, succ_hist;

	/* current error probability within last update interval */
	unsigned long long prob;
	
	/* weight to assign to this rate in SNR estimation within this update interval */
	unsigned long long weight;

	unsigned int perfect_tx_time;
	unsigned int ack_time;
	unsigned int service_time[7];
};

struct indra_sta_info {
	struct ieee80211_sta *sta;

	unsigned int lowest_rix;
	int estimated_snr;
	
	u16 attempts[MEMORY_SIZE];
	u64 att_hist;
	
	u32 flags;
	u32 tx_flags;
	
	unsigned long last_stats_update;
	
	struct indra_rate *r;	
	unsigned int alpha[MEMORY_SIZE];
	
	unsigned int *optimal_indexes;
	unsigned int **optimal_rates;
	
};

struct indra_priv {
	struct ieee80211_hw *hw;
	bool has_mrr;
	unsigned int cw_min;
	unsigned int max_retry;
	
#ifdef CPTCFG_MAC80211_DEBUGFS
	u32 st_deadline;		
	struct dentry *dbg_st_deadline;
	u32 ref_payload_size;		
	struct dentry *dbg_ref_payload_size;
	u32 update_interval;		
	struct dentry *dbg_update_interval;
#endif
	
};

extern const struct rate_control_ops mac80211_indra2;

#endif
