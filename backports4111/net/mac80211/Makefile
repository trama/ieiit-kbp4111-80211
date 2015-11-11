obj-$(CPTCFG_MAC80211) += mac80211.o

# mac80211 objects
mac80211-y := \
	main.o status.o \
	sta_info.o \
	wep.o \
	wpa.o \
	scan.o offchannel.o \
	ht.o agg-tx.o agg-rx.o \
	vht.o \
	ibss.o \
	iface.o \
	rate.o \
	michael.o \
	tkip.o \
	aes_ccm.o \
	aes_gcm.o \
	aes_cmac.o \
	aes_gmac.o \
	cfg.o \
	ethtool.o \
	rx.o \
	spectmgmt.o \
	tx.o \
	key.o \
	util.o \
	wme.o \
	event.o \
	chan.o \
	trace.o mlme.o \
	tdls.o \
	ocb.o

mac80211-$(CPTCFG_MAC80211_LEDS) += led.o
mac80211-$(CPTCFG_MAC80211_DEBUGFS) += \
	debugfs.o \
	debugfs_sta.o \
	debugfs_netdev.o \
	debugfs_key.o

mac80211-$(CPTCFG_MAC80211_MESH) += \
	mesh.o \
	mesh_pathtbl.o \
	mesh_plink.o \
	mesh_hwmp.o \
	mesh_sync.o \
	mesh_ps.o

mac80211-$(CONFIG_PM) += pm.o

CFLAGS_trace.o := -I$(src)

rc80211_indra-y := rc80211_indra.o
rc80211_indra-$(CPTCFG_MAC80211_DEBUGFS) += rc80211_indra_debugfs.o

rc80211_sarf-y := rc80211_sarf.o
rc80211_sarf-$(CPTCFG_MAC80211_DEBUGFS) += rc80211_sarf_debugfs.o

rc80211_farf-y := rc80211_farf.o
rc80211_farf-$(CPTCFG_MAC80211_DEBUGFS) += rc80211_farf_debugfs.o

rc80211_minstrel-y := rc80211_minstrel.o
rc80211_minstrel-$(CPTCFG_MAC80211_DEBUGFS) += rc80211_minstrel_debugfs.o

rc80211_minstrel_ht-y := rc80211_minstrel_ht.o
rc80211_minstrel_ht-$(CPTCFG_MAC80211_DEBUGFS) += rc80211_minstrel_ht_debugfs.o

mac80211-$(CPTCFG_MAC80211_RC_INDRA) += $(rc80211_indra-y)
mac80211-$(CPTCFG_MAC80211_RC_SARF) += $(rc80211_sarf-y)
mac80211-$(CPTCFG_MAC80211_RC_FARF) += $(rc80211_farf-y)
mac80211-$(CPTCFG_MAC80211_RC_MINSTREL) += $(rc80211_minstrel-y)
mac80211-$(CPTCFG_MAC80211_RC_MINSTREL_HT) += $(rc80211_minstrel_ht-y)

ccflags-y += -D__CHECK_ENDIAN__ -DDEBUG
