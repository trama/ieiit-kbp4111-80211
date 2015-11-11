
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

#define FARF_SCALE  16
#define FARF_FRAC(val, div) (((val) << FARF_SCALE) / div)
#define FARF_TRUNC(val) ((val) >> FARF_SCALE)

/* number of transmissions to record*/
#define TX_RECORDED_FARF 100

// AGGIUNTO
struct farf_tx {		
	int rate_idx [4];
	int rate_count [4];
	bool success;
};

struct farf_rate {
	int bitrate;
	int rix;
	
	unsigned int success_count;
	
	unsigned int perfect_tx_time;
};

struct farf_sta_info {
	struct ieee80211_sta *sta;

	unsigned int lowest_rix;
	u8 curr_rate;
	
	bool success_last;

	int n_rates;
	int trasm_index;				
	int trasm_number;				
	struct farf_rate *r;
	struct farf_tx t[TX_RECORDED_FARF];		
	
#ifdef CPTCFG_MAC80211_DEBUGFS
	struct dentry *dbg_stats;
#endif
};

struct farf_priv {
	struct ieee80211_hw *hw;
	bool has_mrr;
	unsigned int max_retry;
	
	u8 cck_rates[4];
};

struct farf_debugfs_info {
	size_t len;
	char buf[];
};


extern const struct rate_control_ops mac80211_farf;
void farf_add_sta_debugfs(void *priv, void *priv_sta, struct dentry *dir);
void farf_remove_sta_debugfs(void *priv, void *priv_sta);

/* debugfs */
int farf_stats_open(struct inode *inode, struct file *file);
ssize_t farf_stats_read(struct file *file, char __user *buf, size_t len, loff_t *ppos);
int farf_stats_release(struct inode *inode, struct file *file);

#endif
