#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xfc315ed4, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x3ce4ca6f, __VMLINUX_SYMBOL_STR(disable_irq) },
	{ 0x351cafac, __VMLINUX_SYMBOL_STR(ar9003_paprd_is_done) },
	{ 0x78eb8af3, __VMLINUX_SYMBOL_STR(ath9k_hw_set_txq_props) },
	{ 0x82ddfceb, __VMLINUX_SYMBOL_STR(ieee80211_csa_finish) },
	{ 0x88db881e, __VMLINUX_SYMBOL_STR(ath9k_hw_init) },
	{ 0x67ae9e57, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0xe95c95a5, __VMLINUX_SYMBOL_STR(pci_bus_read_config_byte) },
	{ 0xdbeefb5d, __VMLINUX_SYMBOL_STR(ath9k_cmn_get_channel) },
	{ 0x1d67b4dd, __VMLINUX_SYMBOL_STR(ath9k_hw_deinit) },
	{ 0x619cb7dd, __VMLINUX_SYMBOL_STR(simple_read_from_buffer) },
	{ 0x2abe2f9, __VMLINUX_SYMBOL_STR(ath9k_hw_cfg_output) },
	{ 0xda3e43d1, __VMLINUX_SYMBOL_STR(_raw_spin_unlock) },
	{ 0xb43a926b, __VMLINUX_SYMBOL_STR(backport_dependency_symbol) },
	{ 0x80edbc39, __VMLINUX_SYMBOL_STR(debugfs_create_dir) },
	{ 0xd6ee688f, __VMLINUX_SYMBOL_STR(vmalloc) },
	{ 0xa27190d2, __VMLINUX_SYMBOL_STR(ar9003_mci_send_wlan_channels) },
	{ 0x4570bf10, __VMLINUX_SYMBOL_STR(pcim_enable_device) },
	{ 0x1e3ffc6d, __VMLINUX_SYMBOL_STR(debugfs_create_u8) },
	{ 0x51eafc8e, __VMLINUX_SYMBOL_STR(param_ops_int) },
	{ 0x4a264c7, __VMLINUX_SYMBOL_STR(ath9k_hw_setslottime) },
	{ 0x33a98b49, __VMLINUX_SYMBOL_STR(ath9k_hw_setantenna) },
	{ 0x754d539c, __VMLINUX_SYMBOL_STR(strlen) },
	{ 0x60a13e90, __VMLINUX_SYMBOL_STR(rcu_barrier) },
	{ 0xeb0ab504, __VMLINUX_SYMBOL_STR(ath9k_hw_set_gpio) },
	{ 0x25742092, __VMLINUX_SYMBOL_STR(ath9k_cmn_init_crypto) },
	{ 0xcf5eb0df, __VMLINUX_SYMBOL_STR(ieee80211_queue_work) },
	{ 0xa91faee5, __VMLINUX_SYMBOL_STR(ath9k_cmn_process_rate) },
	{ 0x87315d2f, __VMLINUX_SYMBOL_STR(led_classdev_register) },
	{ 0x1cc97ce5, __VMLINUX_SYMBOL_STR(pcim_iomap_table) },
	{ 0xe8b3c1b7, __VMLINUX_SYMBOL_STR(ieee80211_csa_is_complete) },
	{ 0xffd02408, __VMLINUX_SYMBOL_STR(ath9k_hw_get_tsf_offset) },
	{ 0x25302744, __VMLINUX_SYMBOL_STR(ath9k_hw_numtxpending) },
	{ 0xb24e0012, __VMLINUX_SYMBOL_STR(ar9003_paprd_setup_gain_table) },
	{ 0xc41512a3, __VMLINUX_SYMBOL_STR(ath9k_hw_btcoex_enable) },
	{ 0xfc145662, __VMLINUX_SYMBOL_STR(dma_set_mask) },
	{ 0xa6da51d2, __VMLINUX_SYMBOL_STR(ath9k_hw_wait) },
	{ 0xe0f1a200, __VMLINUX_SYMBOL_STR(ath9k_hw_set_interrupts) },
	{ 0xba1dbaec, __VMLINUX_SYMBOL_STR(ath9k_cmn_get_hw_crypto_keytype) },
	{ 0x5db7c2e4, __VMLINUX_SYMBOL_STR(ath9k_hw_stopdmarecv) },
	{ 0x3fb8a666, __VMLINUX_SYMBOL_STR(seq_puts) },
	{ 0xb3dd3831, __VMLINUX_SYMBOL_STR(ath9k_hw_btcoex_init_mci) },
	{ 0xefc56354, __VMLINUX_SYMBOL_STR(cfg80211_chandef_create) },
	{ 0xaf499753, __VMLINUX_SYMBOL_STR(ar9003_paprd_enable) },
	{ 0x8e1c00b7, __VMLINUX_SYMBOL_STR(ath9k_hw_getchan_noise) },
	{ 0xdad2e753, __VMLINUX_SYMBOL_STR(ath_key_delete) },
	{ 0x6cecac5c, __VMLINUX_SYMBOL_STR(ath9k_cmn_update_txpow) },
	{ 0xf2a269c4, __VMLINUX_SYMBOL_STR(ath9k_hw_computetxtime) },
	{ 0x1637ff0f, __VMLINUX_SYMBOL_STR(_raw_spin_lock_bh) },
	{ 0x6f485117, __VMLINUX_SYMBOL_STR(ath9k_hw_disable_interrupts) },
	{ 0x7ef39823, __VMLINUX_SYMBOL_STR(ieee80211_hdrlen) },
	{ 0x6b06fdce, __VMLINUX_SYMBOL_STR(delayed_work_timer_fn) },
	{ 0xf6238873, __VMLINUX_SYMBOL_STR(ath9k_hw_bstuck_nfcal) },
	{ 0xf7903bc2, __VMLINUX_SYMBOL_STR(ieee80211_beacon_get_tim) },
	{ 0x1fbfaccb, __VMLINUX_SYMBOL_STR(ath9k_hw_gpio_get) },
	{ 0x91831d70, __VMLINUX_SYMBOL_STR(seq_printf) },
	{ 0x7906bb92, __VMLINUX_SYMBOL_STR(ath_regd_init) },
	{ 0x88bfa7e, __VMLINUX_SYMBOL_STR(cancel_work_sync) },
	{ 0xb82aa34, __VMLINUX_SYMBOL_STR(ath9k_hw_gettxbuf) },
	{ 0x88f635f8, __VMLINUX_SYMBOL_STR(dfs_pattern_detector_init) },
	{ 0x1a99e4f0, __VMLINUX_SYMBOL_STR(ieee80211_unregister_hw) },
	{ 0x59b1ad64, __VMLINUX_SYMBOL_STR(__dev_kfree_skb_any) },
	{ 0x593a99b, __VMLINUX_SYMBOL_STR(init_timer_key) },
	{ 0xbd8b17d8, __VMLINUX_SYMBOL_STR(pcie_capability_clear_and_set_word) },
	{ 0x797c8fa9, __VMLINUX_SYMBOL_STR(cancel_delayed_work_sync) },
	{ 0xd7804f5e, __VMLINUX_SYMBOL_STR(mutex_unlock) },
	{ 0x55eb19be, __VMLINUX_SYMBOL_STR(ieee80211_iterate_active_interfaces_atomic) },
	{ 0x999e8297, __VMLINUX_SYMBOL_STR(vfree) },
	{ 0xdd905bef, __VMLINUX_SYMBOL_STR(ath9k_hw_setrxfilter) },
	{ 0x1541b210, __VMLINUX_SYMBOL_STR(ath9k_hw_get_txq_props) },
	{ 0xbcc7bdd3, __VMLINUX_SYMBOL_STR(ath9k_hw_releasetxqueue) },
	{ 0x52758d2a, __VMLINUX_SYMBOL_STR(ath9k_hw_reset_tsf) },
	{ 0x1a58dcd1, __VMLINUX_SYMBOL_STR(ath9k_cmn_spectral_init_debug) },
	{ 0x583ada4, __VMLINUX_SYMBOL_STR(debugfs_create_file) },
	{ 0x85f0ff9e, __VMLINUX_SYMBOL_STR(wiphy_rfkill_start_polling) },
	{ 0xcced53b9, __VMLINUX_SYMBOL_STR(ath9k_hw_set_csma_flag) },
	{ 0x6defada5, __VMLINUX_SYMBOL_STR(ath9k_cmn_reload_chainmask) },
	{ 0x91715312, __VMLINUX_SYMBOL_STR(sprintf) },
	{ 0x3f409a6b, __VMLINUX_SYMBOL_STR(ath9k_hw_cfg_gpio_input) },
	{ 0x4bf9d508, __VMLINUX_SYMBOL_STR(ath9k_hw_kill_interrupts) },
	{ 0xf208bfa2, __VMLINUX_SYMBOL_STR(ath9k_cmn_spectral_deinit_debug) },
	{ 0x7d11c268, __VMLINUX_SYMBOL_STR(jiffies) },
	{ 0xf5aef041, __VMLINUX_SYMBOL_STR(ath9k_cmn_init_channels_rates) },
	{ 0xda0a4080, __VMLINUX_SYMBOL_STR(ath9k_hw_request_gpio) },
	{ 0xc48335ca, __VMLINUX_SYMBOL_STR(skb_trim) },
	{ 0x1bc14663, __VMLINUX_SYMBOL_STR(ieee80211_stop_queues) },
	{ 0x94a7cc9e, __VMLINUX_SYMBOL_STR(ieee80211_stop_queue) },
	{ 0x6a41ccdf, __VMLINUX_SYMBOL_STR(ieee80211_tx_status) },
	{ 0x6fff62a6, __VMLINUX_SYMBOL_STR(ath_printk) },
	{ 0xf432dd3d, __VMLINUX_SYMBOL_STR(__init_waitqueue_head) },
	{ 0xc97f0486, __VMLINUX_SYMBOL_STR(ath9k_hw_process_rxdesc_edma) },
	{ 0x2f04ad29, __VMLINUX_SYMBOL_STR(ar9003_paprd_populate_single_table) },
	{ 0x7f8f8eab, __VMLINUX_SYMBOL_STR(debugfs_create_u32) },
	{ 0xd7b9c91b, __VMLINUX_SYMBOL_STR(ath9k_hw_setopmode) },
	{ 0xe29ad244, __VMLINUX_SYMBOL_STR(ath9k_hw_get_sifs_time) },
	{ 0x6d0aba34, __VMLINUX_SYMBOL_STR(wait_for_completion) },
	{ 0x2d371f94, __VMLINUX_SYMBOL_STR(pci_set_master) },
	{ 0xbdc79f4f, __VMLINUX_SYMBOL_STR(ath9k_hw_disable) },
	{ 0xd5f2172f, __VMLINUX_SYMBOL_STR(del_timer_sync) },
	{ 0x3c80c06c, __VMLINUX_SYMBOL_STR(kstrtoull) },
	{ 0x7d54ff60, __VMLINUX_SYMBOL_STR(ath9k_hw_resettxqueue) },
	{ 0xbfc597b0, __VMLINUX_SYMBOL_STR(ath_gen_timer_isr) },
	{ 0x30676770, __VMLINUX_SYMBOL_STR(ath9k_hw_rxprocdesc) },
	{ 0xf62e29d3, __VMLINUX_SYMBOL_STR(ath9k_hw_gettsf64) },
	{ 0xfb578fc5, __VMLINUX_SYMBOL_STR(memset) },
	{ 0xd6feeb9c, __VMLINUX_SYMBOL_STR(ath_cmn_process_fft) },
	{ 0x9e8eb6bc, __VMLINUX_SYMBOL_STR(default_llseek) },
	{ 0x50d0024b, __VMLINUX_SYMBOL_STR(dev_err) },
	{ 0x8f64aa4, __VMLINUX_SYMBOL_STR(_raw_spin_unlock_irqrestore) },
	{ 0x6e1904a7, __VMLINUX_SYMBOL_STR(ath9k_hw_btcoex_init_2wire) },
	{ 0x37befc70, __VMLINUX_SYMBOL_STR(jiffies_to_msecs) },
	{ 0x50ba1f0, __VMLINUX_SYMBOL_STR(ieee80211_alloc_hw_nm) },
	{ 0xdd04a3e9, __VMLINUX_SYMBOL_STR(__mutex_init) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x6c5e9b33, __VMLINUX_SYMBOL_STR(ieee80211_free_txskb) },
	{ 0xa57c2c0e, __VMLINUX_SYMBOL_STR(ath9k_hw_set_sta_beacon_timers) },
	{ 0x6ef18399, __VMLINUX_SYMBOL_STR(ar9003_hw_bb_watchdog_dbg_info) },
	{ 0xef4f80d3, __VMLINUX_SYMBOL_STR(ar9003_paprd_create_curve) },
	{ 0xef6de515, __VMLINUX_SYMBOL_STR(ath9k_hw_set_tsfadjust) },
	{ 0x7f6d3353, __VMLINUX_SYMBOL_STR(ath9k_cmn_beacon_config_ap) },
	{ 0x4c9d28b0, __VMLINUX_SYMBOL_STR(phys_base) },
	{ 0xd66dfb6f, __VMLINUX_SYMBOL_STR(ieee80211_wake_queues) },
	{ 0x2d15177d, __VMLINUX_SYMBOL_STR(ath9k_cmn_debug_stat_rx) },
	{ 0xfaef0ed, __VMLINUX_SYMBOL_STR(__tasklet_schedule) },
	{ 0x6aad3a07, __VMLINUX_SYMBOL_STR(ath9k_hw_btcoex_disable) },
	{ 0xa1c76e0a, __VMLINUX_SYMBOL_STR(_cond_resched) },
	{ 0x8c8e9c4f, __VMLINUX_SYMBOL_STR(ath9k_hw_getrxfilter) },
	{ 0x78282e47, __VMLINUX_SYMBOL_STR(ath9k_hw_ani_monitor) },
	{ 0x7b5f5a1f, __VMLINUX_SYMBOL_STR(ath_is_mybeacon) },
	{ 0xc718b428, __VMLINUX_SYMBOL_STR(ath9k_cmn_beacon_config_adhoc) },
	{ 0x3dcdf0d1, __VMLINUX_SYMBOL_STR(ath9k_cmn_process_rssi) },
	{ 0xaafdb617, __VMLINUX_SYMBOL_STR(ath_is_world_regd) },
	{ 0xfa8cbf1, __VMLINUX_SYMBOL_STR(ath9k_cmn_debug_base_eeprom) },
	{ 0x16305289, __VMLINUX_SYMBOL_STR(warn_slowpath_null) },
	{ 0xb24e30fd, __VMLINUX_SYMBOL_STR(pci_bus_write_config_dword) },
	{ 0x54781aab, __VMLINUX_SYMBOL_STR(ieee80211_rx) },
	{ 0xa9a8bb8a, __VMLINUX_SYMBOL_STR(skb_push) },
	{ 0x4e9eed93, __VMLINUX_SYMBOL_STR(mutex_lock) },
	{ 0x4e602b2c, __VMLINUX_SYMBOL_STR(ath9k_hw_addrxbuf_edma) },
	{ 0xc9f73bd9, __VMLINUX_SYMBOL_STR(ieee80211_find_sta_by_ifaddr) },
	{ 0x9545af6d, __VMLINUX_SYMBOL_STR(tasklet_init) },
	{ 0x8834396c, __VMLINUX_SYMBOL_STR(mod_timer) },
	{ 0x923ab119, __VMLINUX_SYMBOL_STR(ath9k_hw_setup_statusring) },
	{ 0x26dc8766, __VMLINUX_SYMBOL_STR(ath9k_cmn_debug_recv) },
	{ 0x2072ee9b, __VMLINUX_SYMBOL_STR(request_threaded_irq) },
	{ 0xc5ac1360, __VMLINUX_SYMBOL_STR(ath9k_hw_get_csma_flag) },
	{ 0x2fde371c, __VMLINUX_SYMBOL_STR(skb_pull) },
	{ 0xe9b97762, __VMLINUX_SYMBOL_STR(simple_open) },
	{ 0x467c8b14, __VMLINUX_SYMBOL_STR(wiphy_rfkill_stop_polling) },
	{ 0xbaba28d6, __VMLINUX_SYMBOL_STR(debugfs_create_bool) },
	{ 0xecdda41c, __VMLINUX_SYMBOL_STR(request_firmware_nowait) },
	{ 0xa45eb222, __VMLINUX_SYMBOL_STR(ar9003_mci_get_next_gpm_offset) },
	{ 0x986b052f, __VMLINUX_SYMBOL_STR(ath9k_hw_get_ack_timeout) },
	{ 0x2972b8d6, __VMLINUX_SYMBOL_STR(ath9k_hw_write_associd) },
	{ 0x9389494f, __VMLINUX_SYMBOL_STR(ath9k_hw_reset_csma_flag) },
	{ 0xc1ef9323, __VMLINUX_SYMBOL_STR(ieee80211_queue_delayed_work) },
	{ 0x803f6f93, __VMLINUX_SYMBOL_STR(pcim_iomap_regions) },
	{ 0xe523ad75, __VMLINUX_SYMBOL_STR(synchronize_irq) },
	{ 0xb503b591, __VMLINUX_SYMBOL_STR(ath_reg_notifier_apply) },
	{ 0x4b0d0212, __VMLINUX_SYMBOL_STR(ath9k_hw_puttxbuf) },
	{ 0x70204bc8, __VMLINUX_SYMBOL_STR(wiphy_to_ieee80211_hw) },
	{ 0x82072614, __VMLINUX_SYMBOL_STR(tasklet_kill) },
	{ 0x6e6d8a57, __VMLINUX_SYMBOL_STR(ar9003_mci_state) },
	{ 0x757dcbab, __VMLINUX_SYMBOL_STR(ath9k_hw_btcoex_init_scheme) },
	{ 0xb7e6f32, __VMLINUX_SYMBOL_STR(ath9k_hw_init_btcoex_hw) },
	{ 0xbfc43095, __VMLINUX_SYMBOL_STR(ieee80211_stop_tx_ba_cb_irqsafe) },
	{ 0x62227c, __VMLINUX_SYMBOL_STR(ath9k_cmn_debug_phy_err) },
	{ 0xabdae699, __VMLINUX_SYMBOL_STR(ath9k_hw_beaconq_setup) },
	{ 0x30efb07a, __VMLINUX_SYMBOL_STR(_dev_info) },
	{ 0x27f632e3, __VMLINUX_SYMBOL_STR(ath9k_hw_name) },
	{ 0x41dfb661, __VMLINUX_SYMBOL_STR(ath9k_cmn_spectral_scan_trigger) },
	{ 0x4ca54305, __VMLINUX_SYMBOL_STR(ath9k_hw_abortpcurecv) },
	{ 0xb94ece57, __VMLINUX_SYMBOL_STR(ath9k_hw_init_global_settings) },
	{ 0x23028586, __VMLINUX_SYMBOL_STR(__alloc_skb) },
	{ 0xc8feead1, __VMLINUX_SYMBOL_STR(pci_bus_read_config_dword) },
	{ 0x5152551c, __VMLINUX_SYMBOL_STR(ath9k_hw_settsf64) },
	{ 0xae906dda, __VMLINUX_SYMBOL_STR(ath9k_hw_set_tx_filter) },
	{ 0xba63339c, __VMLINUX_SYMBOL_STR(_raw_spin_unlock_bh) },
	{ 0xe30367ef, __VMLINUX_SYMBOL_STR(debugfs_create_devm_seqfile) },
	{ 0xdb7305a1, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0x2bc771b4, __VMLINUX_SYMBOL_STR(wiphy_rfkill_set_hw_state) },
	{ 0x3bd1b1f6, __VMLINUX_SYMBOL_STR(msecs_to_jiffies) },
	{ 0x8019a229, __VMLINUX_SYMBOL_STR(ieee80211_sta_set_buffered) },
	{ 0xd62c833f, __VMLINUX_SYMBOL_STR(schedule_timeout) },
	{ 0x5a458e8f, __VMLINUX_SYMBOL_STR(ar9003_mci_get_interrupt) },
	{ 0xea166f3f, __VMLINUX_SYMBOL_STR(kfree_skb) },
	{ 0x5279940c, __VMLINUX_SYMBOL_STR(ath9k_hw_beaconinit) },
	{ 0xf1142a3d, __VMLINUX_SYMBOL_STR(ieee80211_send_bar) },
	{ 0x9c12c8bc, __VMLINUX_SYMBOL_STR(ath9k_hw_updatetxtriglevel) },
	{ 0x9f81e467, __VMLINUX_SYMBOL_STR(ath_hw_cycle_counters_update) },
	{ 0xfd086854, __VMLINUX_SYMBOL_STR(ar9003_mci_set_bt_version) },
	{ 0x67db8c4b, __VMLINUX_SYMBOL_STR(ath9k_hw_set_rx_bufsize) },
	{ 0x53d517ad, __VMLINUX_SYMBOL_STR(ieee80211_get_tx_rates) },
	{ 0x124d7159, __VMLINUX_SYMBOL_STR(ar9003_is_paprd_enabled) },
	{ 0xe4fcf305, __VMLINUX_SYMBOL_STR(ar9003_get_pll_sqsum_dvc) },
	{ 0x2500dbe6, __VMLINUX_SYMBOL_STR(ar9003_mci_send_message) },
	{ 0x4dfedc29, __VMLINUX_SYMBOL_STR(ieee80211_get_buffered_bc) },
	{ 0x5a385236, __VMLINUX_SYMBOL_STR(ar9003_mci_cleanup) },
	{ 0xaaca079, __VMLINUX_SYMBOL_STR(ath9k_hw_getslottime) },
	{ 0x8f4c808d, __VMLINUX_SYMBOL_STR(pskb_expand_head) },
	{ 0x133c9380, __VMLINUX_SYMBOL_STR(ath9k_hw_btcoex_bt_stomp) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
	{ 0xc1f4670a, __VMLINUX_SYMBOL_STR(ath9k_hw_txstart) },
	{ 0x67e3122, __VMLINUX_SYMBOL_STR(ath9k_hw_setrxabort) },
	{ 0x73d185bc, __VMLINUX_SYMBOL_STR(ath9k_hw_check_alive) },
	{ 0x3268293e, __VMLINUX_SYMBOL_STR(pci_unregister_driver) },
	{ 0x904f3a0f, __VMLINUX_SYMBOL_STR(ath9k_cmn_beacon_config_sta) },
	{ 0xd20c3e5, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0xd52bf1ce, __VMLINUX_SYMBOL_STR(_raw_spin_lock) },
	{ 0xdf7fd9ae, __VMLINUX_SYMBOL_STR(ath9k_hw_set_ack_timeout) },
	{ 0x9212475e, __VMLINUX_SYMBOL_STR(ath9k_hw_btcoex_set_concur_txprio) },
	{ 0xa210a69c, __VMLINUX_SYMBOL_STR(ath_hw_setbssidmask) },
	{ 0x9327f5ce, __VMLINUX_SYMBOL_STR(_raw_spin_lock_irqsave) },
	{ 0x1079badf, __VMLINUX_SYMBOL_STR(ieee80211_wake_queue) },
	{ 0x3e499648, __VMLINUX_SYMBOL_STR(ath9k_cmn_rx_skb_postprocess) },
	{ 0x9d9f7fbe, __VMLINUX_SYMBOL_STR(ath9k_hw_phy_disable) },
	{ 0xcf21d241, __VMLINUX_SYMBOL_STR(__wake_up) },
	{ 0x90054a7d, __VMLINUX_SYMBOL_STR(__ieee80211_get_radio_led_name) },
	{ 0x143a2024, __VMLINUX_SYMBOL_STR(ar9003_mci_setup) },
	{ 0xbdba2b49, __VMLINUX_SYMBOL_STR(ath9k_hw_setpower) },
	{ 0x293aefae, __VMLINUX_SYMBOL_STR(ieee80211_sta_eosp) },
	{ 0x34f22f94, __VMLINUX_SYMBOL_STR(prepare_to_wait_event) },
	{ 0x555e58a5, __VMLINUX_SYMBOL_STR(__ieee80211_create_tpt_led_trigger) },
	{ 0x9157aed, __VMLINUX_SYMBOL_STR(pci_bus_write_config_byte) },
	{ 0xb1977a3d, __VMLINUX_SYMBOL_STR(led_classdev_unregister) },
	{ 0xce290f0f, __VMLINUX_SYMBOL_STR(ieee80211_register_hw) },
	{ 0xe30a635d, __VMLINUX_SYMBOL_STR(ath9k_hw_btcoex_set_weight) },
	{ 0xfcec0987, __VMLINUX_SYMBOL_STR(enable_irq) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x3b19a86e, __VMLINUX_SYMBOL_STR(regulatory_hint) },
	{ 0xee14cc09, __VMLINUX_SYMBOL_STR(ath9k_hw_setmcastfilter) },
	{ 0x69acdf38, __VMLINUX_SYMBOL_STR(memcpy) },
	{ 0xb70d5014, __VMLINUX_SYMBOL_STR(ath9k_hw_check_nav) },
	{ 0xc1c3cf94, __VMLINUX_SYMBOL_STR(ath9k_hw_putrxbuf) },
	{ 0xf9afa67f, __VMLINUX_SYMBOL_STR(dma_supported) },
	{ 0x529780ec, __VMLINUX_SYMBOL_STR(ath_rxbuf_alloc) },
	{ 0x323d5e7b, __VMLINUX_SYMBOL_STR(ar9003_paprd_init_table) },
	{ 0xc5060904, __VMLINUX_SYMBOL_STR(__pci_register_driver) },
	{ 0x7e136f83, __VMLINUX_SYMBOL_STR(ath9k_hw_startpcureceive) },
	{ 0x751e4136, __VMLINUX_SYMBOL_STR(ath9k_hw_setuprxdesc) },
	{ 0xc256ebf9, __VMLINUX_SYMBOL_STR(ath9k_hw_setuptxqueue) },
	{ 0x4ca9669f, __VMLINUX_SYMBOL_STR(scnprintf) },
	{ 0xc65e22ce, __VMLINUX_SYMBOL_STR(ath9k_hw_abort_tx_dma) },
	{ 0xa8d887b6, __VMLINUX_SYMBOL_STR(ath9k_hw_reset) },
	{ 0x4d766d3d, __VMLINUX_SYMBOL_STR(ieee80211_free_hw) },
	{ 0xfa66f77c, __VMLINUX_SYMBOL_STR(finish_wait) },
	{ 0x57256291, __VMLINUX_SYMBOL_STR(ath9k_cmn_rx_accept) },
	{ 0x475bc128, __VMLINUX_SYMBOL_STR(ath9k_hw_set_sifs_time) },
	{ 0x4b06d2e7, __VMLINUX_SYMBOL_STR(complete) },
	{ 0x28318305, __VMLINUX_SYMBOL_STR(snprintf) },
	{ 0xb0e602eb, __VMLINUX_SYMBOL_STR(memmove) },
	{ 0xe62240aa, __VMLINUX_SYMBOL_STR(ath9k_hw_btcoex_init_3wire) },
	{ 0xddf1bc9c, __VMLINUX_SYMBOL_STR(consume_skb) },
	{ 0xc0dc6a06, __VMLINUX_SYMBOL_STR(ath_key_config) },
	{ 0x59b42362, __VMLINUX_SYMBOL_STR(ath_opmode_to_string) },
	{ 0xe43c22fd, __VMLINUX_SYMBOL_STR(ar9003_hw_bb_watchdog_check) },
	{ 0x52a50e88, __VMLINUX_SYMBOL_STR(ath9k_hw_enable_interrupts) },
	{ 0x436c2179, __VMLINUX_SYMBOL_STR(iowrite32) },
	{ 0x4f188877, __VMLINUX_SYMBOL_STR(ath9k_cmn_debug_modal_eeprom) },
	{ 0xe6e5021c, __VMLINUX_SYMBOL_STR(skb_put) },
	{ 0x35f53dcb, __VMLINUX_SYMBOL_STR(devm_kmalloc) },
	{ 0x53f6ffbc, __VMLINUX_SYMBOL_STR(wait_for_completion_timeout) },
	{ 0x4166fb7b, __VMLINUX_SYMBOL_STR(ath9k_hw_intrpend) },
	{ 0x4f6b400b, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0x149c7752, __VMLINUX_SYMBOL_STR(param_ops_uint) },
	{ 0x77591a0, __VMLINUX_SYMBOL_STR(ath9k_hw_reset_calvalid) },
	{ 0xff46504d, __VMLINUX_SYMBOL_STR(release_firmware) },
	{ 0x3a4970c7, __VMLINUX_SYMBOL_STR(ieee80211_start_tx_ba_cb_irqsafe) },
	{ 0x7a9f0d74, __VMLINUX_SYMBOL_STR(dma_ops) },
	{ 0xe484e35f, __VMLINUX_SYMBOL_STR(ioread32) },
	{ 0xcb0fc9bc, __VMLINUX_SYMBOL_STR(pcie_capability_read_word) },
	{ 0xf20dabd8, __VMLINUX_SYMBOL_STR(free_irq) },
	{ 0x58556e32, __VMLINUX_SYMBOL_STR(dmam_alloc_coherent) },
	{ 0xbe76c095, __VMLINUX_SYMBOL_STR(ath9k_hw_set_txpowerlimit) },
	{ 0x7afdf64e, __VMLINUX_SYMBOL_STR(ath9k_hw_stop_dma_queue) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=ath9k_hw,mac80211,ath9k_common,compat,cfg80211,ath";

MODULE_ALIAS("pci:v0000168Cd00000023sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000024sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000027sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000029sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd0000002Asv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd0000002Asv00001A3Bsd00001C71bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd0000002Asv0000105Bsd0000E01Fbc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd0000002Asv000011ADsd00006632bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd0000002Asv000011ADsd00006642bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd0000002Asv00001A32sd00000306bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd0000002Asv0000185Fsd0000309Dbc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd0000002Asv000010CFsd0000147Cbc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd0000002Asv000010CFsd0000147Dbc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd0000002Asv000010CFsd00001536bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd0000002Bsv00001A3Bsd00002C37bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd0000002Bsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd0000002Csv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd0000002Dsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd0000002Esv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000030sv00001A56sd00002000bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000030sv00001A56sd00002001bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000030sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv00001A3Bsd00002086bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv00001A3Bsd00001237bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv00001A3Bsd00002126bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv00001A3Bsd0000126Abc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv00001A3Bsd00002152bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv0000105Bsd0000E075bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv0000168Csd00003119bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv0000168Csd00003122bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv0000185Fsd00003119bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv0000185Fsd00003027bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv0000144Dsd00004105bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv0000144Dsd00004106bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv0000144Dsd0000410Dbc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv0000144Dsd0000410Ebc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv0000144Dsd0000410Fbc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv0000144Dsd0000C706bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv0000144Dsd0000C680bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv0000144Dsd0000C708bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv000017AAsd00003218bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv000017AAsd00003219bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv00001A3Bsd00002C97bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv00001A3Bsd00002100bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv00001C56sd00004001bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv000011ADsd00006627bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv000011ADsd00006628bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv0000105Bsd0000E04Ebc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv0000105Bsd0000E04Fbc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv0000144Fsd00007197bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv00001B9Asd00002000bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv00001B9Asd00002001bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv00001A3Bsd00001186bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv00001A3Bsd00001F86bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv00001A3Bsd00001195bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv00001A3Bsd00001F95bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv00001B9Asd00001C00bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv00001B9Asd00001C01bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv00001043sd0000850Dbc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000032sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000033sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000034sv00001A3Bsd00002116bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000034sv000011ADsd00006661bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000034sv0000168Csd00003117bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000034sv000017AAsd00003214bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000034sv00001969sd00000091bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000034sv00001A3Bsd00002110bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000034sv00001043sd0000850Ebc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000034sv000011ADsd00006631bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000034sv000011ADsd00006641bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000034sv0000103Csd00001864bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000034sv000014CDsd00000063bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000034sv000014CDsd00000064bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000034sv000010CFsd00001783bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000030sv00001A56sd00002003bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000034sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000037sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000168Csd00003028bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv00001A3Bsd00002176bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000105Bsd0000E068bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000185Fsd0000A119bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv000011ADsd00000632bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv000011ADsd000006B2bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv000011ADsd00000842bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv000011ADsd00001842bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv000011ADsd00006671bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv00001B9Asd00002811bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv00001B9Asd00002812bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv00001B9Asd000028A1bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv00001B9Asd000028A3bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv00001A3Bsd0000218Abc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv00001A3Bsd00002F8Abc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000168Csd00003025bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000168Csd00003026bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000168Csd0000302Bbc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000105Bsd0000E069bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000185Fsd00003028bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv000011ADsd00000622bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv000011ADsd00000672bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv000011ADsd00000662bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv000011ADsd000006A2bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv000011ADsd00000682bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv00001A3Bsd0000213Abc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv00001A3Bsd0000213Cbc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000103Csd000018E3bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000103Csd0000217Fbc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000103Csd00002005bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv00001028sd0000020Cbc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000144Dsd0000411Abc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000144Dsd0000411Bbc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000144Dsd0000411Cbc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000144Dsd0000411Dbc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000144Dsd0000411Ebc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000144Dsd00004129bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000144Dsd0000412Abc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000168Csd00003027bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000168Csd0000302Cbc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv000011ADsd00000642bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv000011ADsd00000652bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv000011ADsd00000612bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv000011ADsd00000832bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv000011ADsd00001832bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv000011ADsd00000692bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv000011ADsd00000803bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv000011ADsd00000813bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv00001A3Bsd00002130bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv00001A3Bsd0000213Bbc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv00001A3Bsd00002182bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv00001A3Bsd0000218Bbc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv00001A3Bsd0000218Cbc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv00001A3Bsd00002F82bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000144Fsd00007202bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv00001B9Asd00002810bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv00001B9Asd00002813bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv00001B9Asd000028A2bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv00001B9Asd000028A4bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000185Fsd00003027bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000185Fsd0000A120bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000105Bsd0000E07Fbc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000105Bsd0000E08Fbc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000105Bsd0000E081bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000105Bsd0000E091bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv0000105Bsd0000E099bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv000017AAsd00003026bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv000017AAsd00004026bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv00001043sd000085F2bc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv00001028sd0000020Ebc*sc*i*");
MODULE_ALIAS("pci:v0000168Cd00000036sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "5967D201E619C2AC55C6EC7");
