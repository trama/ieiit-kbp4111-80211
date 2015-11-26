
/*
 * Copyright (C) 2008 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __RC_SARF_H
#define __RC_SARF_H

#define N_SARF	10	/* number of consecutive successes to increase rate */
#define K_SARF	2	/* number of consecutive failures to decrease rate */

#define SARF_SCALE  16
#define SARF_FRAC(val, div) (((val) << SARF_SCALE) / div)
#define SARF_TRUNC(val) ((val) >> SARF_SCALE)

/* number of transmissions to record*/
#define TX_RECORDED_SARF 100

// AGGIUNTO
struct sarf_tx {		
	int rate_idx [4];
	int rate_count [4];
	bool success;
};

struct sarf_rate {
	int bitrate;
	int rix;
	
	unsigned int success_count;
	unsigned int failures_count;
	
	unsigned int perfect_tx_time;
};

struct sarf_sta_info {
	struct ieee80211_sta *sta;

	unsigned int lowest_rix;
	u8 curr_rate;
	
	u32 flags;
	u32 tx_flags;
	
	bool success_last;

	int n_rates;
	int trasm_index;				
	int trasm_number;				
	struct sarf_rate *r;
	struct sarf_tx t[TX_RECORDED_SARF];		
	
#ifdef CPTCFG_MAC80211_DEBUGFS
	struct dentry *dbg_stats;
#endif
};

struct sarf_priv {
	struct ieee80211_hw *hw;
	bool has_mrr;
	unsigned int max_retry;
	
	u8 cck_rates[4];
};

struct sarf_debugfs_info {
	size_t len;
	char buf[];
};


extern const struct rate_control_ops mac80211_sarf;
void sarf_add_sta_debugfs(void *priv, void *priv_sta, struct dentry *dir);
void sarf_remove_sta_debugfs(void *priv, void *priv_sta);

/* debugfs */
int sarf_stats_open(struct inode *inode, struct file *file);
ssize_t sarf_stats_read(struct file *file, char __user *buf, size_t len, loff_t *ppos);
int sarf_stats_release(struct inode *inode, struct file *file);

#endif
