
/*
 * Copyright (C) 2008 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __RC_SARF2_H
#define __RC_SARF2_H

#define N_SARF	10	/* number of consecutive successes to increase rate */
#define K_SARF	2	/* number of consecutive failures to decrease rate */
#define SARF_ADDRESSES 10	/* number of addresses allowed */
#define MCS_GROUP_RATES 8	/* number of adopted MCS */

#define SARF_SCALE  16
#define SARF_FRAC(val, div) (((val) << SARF_SCALE) / div)
#define SARF_TRUNC(val) ((val) >> SARF_SCALE)

struct sarf_address {
	u8 add[ETH_ALEN];
};

struct sarf_rate {
	int bitrate;
	int rix;	
	unsigned int success_count[SARF_ADDRESSES];
	unsigned int failures_count[SARF_ADDRESSES];
	
	unsigned int perfect_tx_time;
};

struct sarf_sta_info {
	struct ieee80211_sta *sta;

	unsigned int lowest_rix;
	u8 curr_rate[SARF_ADDRESSES];
	
	struct sarf_address addresses[SARF_ADDRESSES];
	u8 add_idx;
	u8 new_add_idx;
	
	u32 flags;
	u32 tx_flags;
	
	bool success_last[SARF_ADDRESSES];

	int n_rates;				
	struct sarf_rate *r;
};

struct sarf_priv {
	struct ieee80211_hw *hw;
	unsigned int max_retry;
};

extern const struct rate_control_ops mac80211_sarf2;

#endif
