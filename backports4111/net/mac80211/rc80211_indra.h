
/*
 * Copyright (C) 2008 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __RC_INDRA_H
#define __RC_INDRA_H

#define ALPHA_INDRA 99	/* EWMA parameter */
#define PER_TH_INDRA 990	/* Threshold on PER for computation of SNR when packet is lost */
#define SNR_RANGE 30 /* Values of SNR counted on the map */ 
#define SNR_MIN 7 /* Minimum value of SNR counted on the map */ 

#define INDRA_SCALE  16
#define INDRA_FRAC(val, div) (((val) << INDRA_SCALE) / div)
#define INDRA_TRUNC(val) ((val) >> INDRA_SCALE)

struct indra_rate {
	int bitrate;
	int rix;
	int last_attempts;
	int last_success;
	unsigned int per_snr_map[SNR_RANGE];
	unsigned int perfect_tx_time;
	unsigned int ack_time;
	unsigned int attempts[7];
};

struct indra_sta_info {
	struct ieee80211_sta *sta;

	unsigned int lowest_rix;
	int last_snr;

	u32 flags;
	u32 tx_flags;
	
	unsigned long last_stats_update;

	int n_rates;			
	struct indra_rate *r;	
	
	unsigned int *optimal_indexes;
	unsigned int **optimal_rates;
	
#ifdef CPTCFG_MAC80211_DEBUGFS
	struct dentry *dbg_stats;
#endif
};

struct indra_priv {
	struct ieee80211_hw *hw;
	bool has_mrr;
	unsigned int cw_min;
	unsigned int max_retry;
	
	u8 cck_rates[4];
	
#ifdef CPTCFG_MAC80211_DEBUGFS
	u32 st_deadline;		
	struct dentry *dbg_st_deadline;
	u32 ref_payload_size;		
	struct dentry *dbg_ref_payload_size;
	u32 update_interval;		
	struct dentry *dbg_update_interval;
#endif
	
};

struct indra_debugfs_info {
	size_t len;
	char buf[];
};


extern const struct rate_control_ops mac80211_indra;
void indra_add_sta_debugfs(void *priv, void *priv_sta, struct dentry *dir);
void indra_remove_sta_debugfs(void *priv, void *priv_sta);

/* debugfs */
int indra_stats_open(struct inode *inode, struct file *file);
ssize_t indra_stats_read(struct file *file, char __user *buf, size_t len, loff_t *ppos);
int indra_stats_release(struct inode *inode, struct file *file);

#endif
