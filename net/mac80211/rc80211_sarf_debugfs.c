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
#include <linux/ieee80211.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <net/mac80211.h>
#include "rc80211_sarf.h"

int
sarf_stats_open(struct inode *inode, struct file *file)
{
	struct sarf_sta_info *si = inode->i_private;
	struct sarf_debugfs_info *ss;
	unsigned int i;
	int n;
	char *p;

	ss = kmalloc(sizeof(*ss) + 4096, GFP_KERNEL);
	if (!ss)
		return -ENOMEM;

	file->private_data = ss;
	p = ss->buf;
	
	//p += sprintf(p, "\nTRANSMISSION ATTEMPTS\n\n");
	//p += sprintf(p, "number rate index retry success\n");
	if (si->trasm_number < TX_RECORDED_SARF)  {
		n = 1;
		for (i = 0; i < si->trasm_index; i++) {
			struct sarf_tx *st = &si->t[i];
	
			p += sprintf(p, "%6i %3i %3i %3i %3i %3i %3i %3i %3i %5i\n",
					n,
					st->rate_idx[0],
					st->rate_count[0],
					st->rate_idx[1],
					st->rate_count[1],
					st->rate_idx[2],
					st->rate_count[2],
					st->rate_idx[3],
					st->rate_count[3],
					(int) st->success);
			n++;
		}
	} else {
		n = si->trasm_number-TX_RECORDED_SARF+1;
		for (i = si->trasm_index; i < TX_RECORDED_SARF; i++) {
			struct sarf_tx *st = &si->t[i];
	
			p += sprintf(p, "%6i %3i %3i %3i %3i %3i %3i %3i %3i %5i\n",
					n,
					st->rate_idx[0],
					st->rate_count[0],
					st->rate_idx[1],
					st->rate_count[1],
					st->rate_idx[2],
					st->rate_count[2],
					st->rate_idx[3],
					st->rate_count[3],
					(int) st->success);
			n++;
		}
		for (i = 0; i < si->trasm_index; i++) {
			struct sarf_tx *st = &si->t[i];
	
			p += sprintf(p, "%6i %3i %3i %3i %3i %3i %3i %3i %3i %5i\n",
					n,
					st->rate_idx[0],
					st->rate_count[0],
					st->rate_idx[1],
					st->rate_count[1],
					st->rate_idx[2],
					st->rate_count[2],
					st->rate_idx[3],
					st->rate_count[3],
					(int) st->success);
			n++;
		}
		
	}
	
	ss->len = p - ss->buf;
	
	return 0;
}

ssize_t
sarf_stats_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
	struct sarf_debugfs_info *ss;

	ss = file->private_data;
	return simple_read_from_buffer(buf, len, ppos, ss->buf, ss->len);
}

int
sarf_stats_release(struct inode *inode, struct file *file)
{
	kfree(file->private_data);
	return 0;
}

static const struct file_operations sarf_stat_fops = {
	.owner = THIS_MODULE,
	.open = sarf_stats_open,
	.read = sarf_stats_read,
	.release = sarf_stats_release,
	.llseek = default_llseek,
};

void
sarf_add_sta_debugfs(void *priv, void *priv_sta, struct dentry *dir)
{
	struct sarf_sta_info *si = priv_sta;

	si->dbg_stats = debugfs_create_file("rc_stats", S_IRUGO, dir, si,
			&sarf_stat_fops);
}

void
sarf_remove_sta_debugfs(void *priv, void *priv_sta)
{
	struct sarf_sta_info *si = priv_sta;

	debugfs_remove(si->dbg_stats);
	
}
