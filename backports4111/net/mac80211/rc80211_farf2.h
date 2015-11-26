
/*
 * Copyright (C) 2008 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __RC_FARF_H
#define __RC_FARF_H

#define N_FARF	10	/* number of consecutive successes to increase rate */
#define FARF_ADDRESSES 10	/* number of addresses allowed */
#define MCS_GROUP_RATES 8	/* number of adopted MCS */

#define FARF_SCALE  16
#define FARF_FRAC(val, div) (((val) << FARF_SCALE) / div)
#define FARF_TRUNC(val) ((val) >> FARF_SCALE)

struct farf_address {
	u8 add[ETH_ALEN];
};

struct farf_rate {
	int bitrate;
	int rix;	
	unsigned int success_count[FARF_ADDRESSES];
	
	unsigned int perfect_tx_time;
};

struct farf_sta_info {
	struct ieee80211_sta *sta;

	unsigned int lowest_rix;
	u8 curr_rate[FARF_ADDRESSES];
	
	struct farf_address addresses[FARF_ADDRESSES];
	u8 add_idx;
	u8 new_add_idx;
	
	u32 flags;
	u32 tx_flags;
	
	bool success_last[FARF_ADDRESSES];

	int n_rates;				
	struct farf_rate *r;
};

struct farf_priv {
	struct ieee80211_hw *hw;
	unsigned int max_retry;
};

extern const struct rate_control_ops mac80211_farf2;

#endif
