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
	{ 0x2d3385d3, __VMLINUX_SYMBOL_STR(system_wq) },
	{ 0x78eb8af3, __VMLINUX_SYMBOL_STR(ath9k_hw_set_txq_props) },
	{ 0x88db881e, __VMLINUX_SYMBOL_STR(ath9k_hw_init) },
	{ 0x67ae9e57, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0xdbeefb5d, __VMLINUX_SYMBOL_STR(ath9k_cmn_get_channel) },
	{ 0x1d67b4dd, __VMLINUX_SYMBOL_STR(ath9k_hw_deinit) },
	{ 0x619cb7dd, __VMLINUX_SYMBOL_STR(simple_read_from_buffer) },
	{ 0x2abe2f9, __VMLINUX_SYMBOL_STR(ath9k_hw_cfg_output) },
	{ 0xda3e43d1, __VMLINUX_SYMBOL_STR(_raw_spin_unlock) },
	{ 0xb43a926b, __VMLINUX_SYMBOL_STR(backport_dependency_symbol) },
	{ 0x80edbc39, __VMLINUX_SYMBOL_STR(debugfs_create_dir) },
	{ 0x51eafc8e, __VMLINUX_SYMBOL_STR(param_ops_int) },
	{ 0xe50a565b, __VMLINUX_SYMBOL_STR(device_release_driver) },
	{ 0x60a13e90, __VMLINUX_SYMBOL_STR(rcu_barrier) },
	{ 0xeb0ab504, __VMLINUX_SYMBOL_STR(ath9k_hw_set_gpio) },
	{ 0x25742092, __VMLINUX_SYMBOL_STR(ath9k_cmn_init_crypto) },
	{ 0xcf5eb0df, __VMLINUX_SYMBOL_STR(ieee80211_queue_work) },
	{ 0xa91faee5, __VMLINUX_SYMBOL_STR(ath9k_cmn_process_rate) },
	{ 0x87315d2f, __VMLINUX_SYMBOL_STR(led_classdev_register) },
	{ 0xc41512a3, __VMLINUX_SYMBOL_STR(ath9k_hw_btcoex_enable) },
	{ 0xa6da51d2, __VMLINUX_SYMBOL_STR(ath9k_hw_wait) },
	{ 0xba1dbaec, __VMLINUX_SYMBOL_STR(ath9k_cmn_get_hw_crypto_keytype) },
	{ 0x5db7c2e4, __VMLINUX_SYMBOL_STR(ath9k_hw_stopdmarecv) },
	{ 0xdad2e753, __VMLINUX_SYMBOL_STR(ath_key_delete) },
	{ 0x6cecac5c, __VMLINUX_SYMBOL_STR(ath9k_cmn_update_txpow) },
	{ 0x1637ff0f, __VMLINUX_SYMBOL_STR(_raw_spin_lock_bh) },
	{ 0x7ef39823, __VMLINUX_SYMBOL_STR(ieee80211_hdrlen) },
	{ 0x6b06fdce, __VMLINUX_SYMBOL_STR(delayed_work_timer_fn) },
	{ 0xf7903bc2, __VMLINUX_SYMBOL_STR(ieee80211_beacon_get_tim) },
	{ 0x1fbfaccb, __VMLINUX_SYMBOL_STR(ath9k_hw_gpio_get) },
	{ 0x7906bb92, __VMLINUX_SYMBOL_STR(ath_regd_init) },
	{ 0x88bfa7e, __VMLINUX_SYMBOL_STR(cancel_work_sync) },
	{ 0xb2c4e2e6, __VMLINUX_SYMBOL_STR(usb_kill_urb) },
	{ 0x448eac3e, __VMLINUX_SYMBOL_STR(kmemdup) },
	{ 0x1a99e4f0, __VMLINUX_SYMBOL_STR(ieee80211_unregister_hw) },
	{ 0x59b1ad64, __VMLINUX_SYMBOL_STR(__dev_kfree_skb_any) },
	{ 0xeae3dfd6, __VMLINUX_SYMBOL_STR(__const_udelay) },
	{ 0x593a99b, __VMLINUX_SYMBOL_STR(init_timer_key) },
	{ 0x797c8fa9, __VMLINUX_SYMBOL_STR(cancel_delayed_work_sync) },
	{ 0xd7804f5e, __VMLINUX_SYMBOL_STR(mutex_unlock) },
	{ 0x55eb19be, __VMLINUX_SYMBOL_STR(ieee80211_iterate_active_interfaces_atomic) },
	{ 0xdd905bef, __VMLINUX_SYMBOL_STR(ath9k_hw_setrxfilter) },
	{ 0x1541b210, __VMLINUX_SYMBOL_STR(ath9k_hw_get_txq_props) },
	{ 0xbcc7bdd3, __VMLINUX_SYMBOL_STR(ath9k_hw_releasetxqueue) },
	{ 0x52758d2a, __VMLINUX_SYMBOL_STR(ath9k_hw_reset_tsf) },
	{ 0x1a58dcd1, __VMLINUX_SYMBOL_STR(ath9k_cmn_spectral_init_debug) },
	{ 0x583ada4, __VMLINUX_SYMBOL_STR(debugfs_create_file) },
	{ 0x85f0ff9e, __VMLINUX_SYMBOL_STR(wiphy_rfkill_start_polling) },
	{ 0x6defada5, __VMLINUX_SYMBOL_STR(ath9k_cmn_reload_chainmask) },
	{ 0x91715312, __VMLINUX_SYMBOL_STR(sprintf) },
	{ 0xf208bfa2, __VMLINUX_SYMBOL_STR(ath9k_cmn_spectral_deinit_debug) },
	{ 0x7d11c268, __VMLINUX_SYMBOL_STR(jiffies) },
	{ 0xf5aef041, __VMLINUX_SYMBOL_STR(ath9k_cmn_init_channels_rates) },
	{ 0xc48335ca, __VMLINUX_SYMBOL_STR(skb_trim) },
	{ 0x1bc14663, __VMLINUX_SYMBOL_STR(ieee80211_stop_queues) },
	{ 0xb8c7d434, __VMLINUX_SYMBOL_STR(usb_unanchor_urb) },
	{ 0x40256835, __VMLINUX_SYMBOL_STR(complete_all) },
	{ 0xf7da0f71, __VMLINUX_SYMBOL_STR(__netdev_alloc_skb) },
	{ 0x6a41ccdf, __VMLINUX_SYMBOL_STR(ieee80211_tx_status) },
	{ 0x6fff62a6, __VMLINUX_SYMBOL_STR(ath_printk) },
	{ 0xf432dd3d, __VMLINUX_SYMBOL_STR(__init_waitqueue_head) },
	{ 0xd7b9c91b, __VMLINUX_SYMBOL_STR(ath9k_hw_setopmode) },
	{ 0x6d0aba34, __VMLINUX_SYMBOL_STR(wait_for_completion) },
	{ 0xbdc79f4f, __VMLINUX_SYMBOL_STR(ath9k_hw_disable) },
	{ 0xd5f2172f, __VMLINUX_SYMBOL_STR(del_timer_sync) },
	{ 0x3c80c06c, __VMLINUX_SYMBOL_STR(kstrtoull) },
	{ 0x7d54ff60, __VMLINUX_SYMBOL_STR(ath9k_hw_resettxqueue) },
	{ 0xf62e29d3, __VMLINUX_SYMBOL_STR(ath9k_hw_gettsf64) },
	{ 0xd6feeb9c, __VMLINUX_SYMBOL_STR(ath_cmn_process_fft) },
	{ 0x9e8eb6bc, __VMLINUX_SYMBOL_STR(default_llseek) },
	{ 0x50d0024b, __VMLINUX_SYMBOL_STR(dev_err) },
	{ 0x8f64aa4, __VMLINUX_SYMBOL_STR(_raw_spin_unlock_irqrestore) },
	{ 0x37befc70, __VMLINUX_SYMBOL_STR(jiffies_to_msecs) },
	{ 0x9e7c6d7b, __VMLINUX_SYMBOL_STR(usb_deregister) },
	{ 0x50ba1f0, __VMLINUX_SYMBOL_STR(ieee80211_alloc_hw_nm) },
	{ 0xdd04a3e9, __VMLINUX_SYMBOL_STR(__mutex_init) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xa57c2c0e, __VMLINUX_SYMBOL_STR(ath9k_hw_set_sta_beacon_timers) },
	{ 0xef6de515, __VMLINUX_SYMBOL_STR(ath9k_hw_set_tsfadjust) },
	{ 0x7f6d3353, __VMLINUX_SYMBOL_STR(ath9k_cmn_beacon_config_ap) },
	{ 0xd66dfb6f, __VMLINUX_SYMBOL_STR(ieee80211_wake_queues) },
	{ 0x2d15177d, __VMLINUX_SYMBOL_STR(ath9k_cmn_debug_stat_rx) },
	{ 0xfaef0ed, __VMLINUX_SYMBOL_STR(__tasklet_schedule) },
	{ 0x6aad3a07, __VMLINUX_SYMBOL_STR(ath9k_hw_btcoex_disable) },
	{ 0x8c8e9c4f, __VMLINUX_SYMBOL_STR(ath9k_hw_getrxfilter) },
	{ 0x78282e47, __VMLINUX_SYMBOL_STR(ath9k_hw_ani_monitor) },
	{ 0x7b5f5a1f, __VMLINUX_SYMBOL_STR(ath_is_mybeacon) },
	{ 0xc718b428, __VMLINUX_SYMBOL_STR(ath9k_cmn_beacon_config_adhoc) },
	{ 0xd68ea71d, __VMLINUX_SYMBOL_STR(usb_control_msg) },
	{ 0x3dcdf0d1, __VMLINUX_SYMBOL_STR(ath9k_cmn_process_rssi) },
	{ 0xaafdb617, __VMLINUX_SYMBOL_STR(ath_is_world_regd) },
	{ 0xfa8cbf1, __VMLINUX_SYMBOL_STR(ath9k_cmn_debug_base_eeprom) },
	{ 0x16305289, __VMLINUX_SYMBOL_STR(warn_slowpath_null) },
	{ 0x54781aab, __VMLINUX_SYMBOL_STR(ieee80211_rx) },
	{ 0xa9a8bb8a, __VMLINUX_SYMBOL_STR(skb_push) },
	{ 0x4e9eed93, __VMLINUX_SYMBOL_STR(mutex_lock) },
	{ 0x9545af6d, __VMLINUX_SYMBOL_STR(tasklet_init) },
	{ 0x8834396c, __VMLINUX_SYMBOL_STR(mod_timer) },
	{ 0x26dc8766, __VMLINUX_SYMBOL_STR(ath9k_cmn_debug_recv) },
	{ 0x2fde371c, __VMLINUX_SYMBOL_STR(skb_pull) },
	{ 0xe9b97762, __VMLINUX_SYMBOL_STR(simple_open) },
	{ 0x467c8b14, __VMLINUX_SYMBOL_STR(wiphy_rfkill_stop_polling) },
	{ 0xecdda41c, __VMLINUX_SYMBOL_STR(request_firmware_nowait) },
	{ 0x2972b8d6, __VMLINUX_SYMBOL_STR(ath9k_hw_write_associd) },
	{ 0xc1ef9323, __VMLINUX_SYMBOL_STR(ieee80211_queue_delayed_work) },
	{ 0xf11543ff, __VMLINUX_SYMBOL_STR(find_first_zero_bit) },
	{ 0xb503b591, __VMLINUX_SYMBOL_STR(ath_reg_notifier_apply) },
	{ 0x70204bc8, __VMLINUX_SYMBOL_STR(wiphy_to_ieee80211_hw) },
	{ 0x82072614, __VMLINUX_SYMBOL_STR(tasklet_kill) },
	{ 0xb7e6f32, __VMLINUX_SYMBOL_STR(ath9k_hw_init_btcoex_hw) },
	{ 0xbfc43095, __VMLINUX_SYMBOL_STR(ieee80211_stop_tx_ba_cb_irqsafe) },
	{ 0x62227c, __VMLINUX_SYMBOL_STR(ath9k_cmn_debug_phy_err) },
	{ 0xcda4999c, __VMLINUX_SYMBOL_STR(skb_queue_tail) },
	{ 0xabdae699, __VMLINUX_SYMBOL_STR(ath9k_hw_beaconq_setup) },
	{ 0x30efb07a, __VMLINUX_SYMBOL_STR(_dev_info) },
	{ 0xdd71a0c, __VMLINUX_SYMBOL_STR(usb_submit_urb) },
	{ 0x27f632e3, __VMLINUX_SYMBOL_STR(ath9k_hw_name) },
	{ 0x41dfb661, __VMLINUX_SYMBOL_STR(ath9k_cmn_spectral_scan_trigger) },
	{ 0xb94ece57, __VMLINUX_SYMBOL_STR(ath9k_hw_init_global_settings) },
	{ 0x23028586, __VMLINUX_SYMBOL_STR(__alloc_skb) },
	{ 0x37c6b2ad, __VMLINUX_SYMBOL_STR(usb_get_dev) },
	{ 0xff1076fc, __VMLINUX_SYMBOL_STR(usb_kill_anchored_urbs) },
	{ 0x5152551c, __VMLINUX_SYMBOL_STR(ath9k_hw_settsf64) },
	{ 0xba63339c, __VMLINUX_SYMBOL_STR(_raw_spin_unlock_bh) },
	{ 0xdb7305a1, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0x2bc771b4, __VMLINUX_SYMBOL_STR(wiphy_rfkill_set_hw_state) },
	{ 0x3bd1b1f6, __VMLINUX_SYMBOL_STR(msecs_to_jiffies) },
	{ 0x3e364fef, __VMLINUX_SYMBOL_STR(usb_bulk_msg) },
	{ 0xeb69d4c7, __VMLINUX_SYMBOL_STR(usb_put_dev) },
	{ 0xa202a8e5, __VMLINUX_SYMBOL_STR(kmalloc_order_trace) },
	{ 0xea166f3f, __VMLINUX_SYMBOL_STR(kfree_skb) },
	{ 0x5279940c, __VMLINUX_SYMBOL_STR(ath9k_hw_beaconinit) },
	{ 0x69542b01, __VMLINUX_SYMBOL_STR(ieee80211_find_sta) },
	{ 0x4dfedc29, __VMLINUX_SYMBOL_STR(ieee80211_get_buffered_bc) },
	{ 0x7c8338c8, __VMLINUX_SYMBOL_STR(usb_interrupt_msg) },
	{ 0x133c9380, __VMLINUX_SYMBOL_STR(ath9k_hw_btcoex_bt_stomp) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
	{ 0x67e3122, __VMLINUX_SYMBOL_STR(ath9k_hw_setrxabort) },
	{ 0x904f3a0f, __VMLINUX_SYMBOL_STR(ath9k_cmn_beacon_config_sta) },
	{ 0xd20c3e5, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0xd52bf1ce, __VMLINUX_SYMBOL_STR(_raw_spin_lock) },
	{ 0xa210a69c, __VMLINUX_SYMBOL_STR(ath_hw_setbssidmask) },
	{ 0x9327f5ce, __VMLINUX_SYMBOL_STR(_raw_spin_lock_irqsave) },
	{ 0x3e499648, __VMLINUX_SYMBOL_STR(ath9k_cmn_rx_skb_postprocess) },
	{ 0x9d9f7fbe, __VMLINUX_SYMBOL_STR(ath9k_hw_phy_disable) },
	{ 0x90054a7d, __VMLINUX_SYMBOL_STR(__ieee80211_get_radio_led_name) },
	{ 0x39195055, __VMLINUX_SYMBOL_STR(ieee80211_get_hdrlen_from_skb) },
	{ 0xbdba2b49, __VMLINUX_SYMBOL_STR(ath9k_hw_setpower) },
	{ 0x555e58a5, __VMLINUX_SYMBOL_STR(__ieee80211_create_tpt_led_trigger) },
	{ 0xce290f0f, __VMLINUX_SYMBOL_STR(ieee80211_register_hw) },
	{ 0xb1977a3d, __VMLINUX_SYMBOL_STR(led_classdev_unregister) },
	{ 0xe30a635d, __VMLINUX_SYMBOL_STR(ath9k_hw_btcoex_set_weight) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x3b19a86e, __VMLINUX_SYMBOL_STR(regulatory_hint) },
	{ 0x69acdf38, __VMLINUX_SYMBOL_STR(memcpy) },
	{ 0xee14cc09, __VMLINUX_SYMBOL_STR(ath9k_hw_setmcastfilter) },
	{ 0x31246b57, __VMLINUX_SYMBOL_STR(ieee80211_start_tx_ba_session) },
	{ 0x7e136f83, __VMLINUX_SYMBOL_STR(ath9k_hw_startpcureceive) },
	{ 0xc256ebf9, __VMLINUX_SYMBOL_STR(ath9k_hw_setuptxqueue) },
	{ 0x4ca9669f, __VMLINUX_SYMBOL_STR(scnprintf) },
	{ 0x8a9b577a, __VMLINUX_SYMBOL_STR(usb_register_driver) },
	{ 0x2e8708cb, __VMLINUX_SYMBOL_STR(request_firmware) },
	{ 0xa8d887b6, __VMLINUX_SYMBOL_STR(ath9k_hw_reset) },
	{ 0x4d766d3d, __VMLINUX_SYMBOL_STR(ieee80211_free_hw) },
	{ 0x57256291, __VMLINUX_SYMBOL_STR(ath9k_cmn_rx_accept) },
	{ 0x63c4d61f, __VMLINUX_SYMBOL_STR(__bitmap_weight) },
	{ 0xb63ca742, __VMLINUX_SYMBOL_STR(skb_dequeue) },
	{ 0x69a42310, __VMLINUX_SYMBOL_STR(usb_ifnum_to_if) },
	{ 0x2e0d2f7f, __VMLINUX_SYMBOL_STR(queue_work_on) },
	{ 0x4b06d2e7, __VMLINUX_SYMBOL_STR(complete) },
	{ 0x28318305, __VMLINUX_SYMBOL_STR(snprintf) },
	{ 0xb0e602eb, __VMLINUX_SYMBOL_STR(memmove) },
	{ 0xe62240aa, __VMLINUX_SYMBOL_STR(ath9k_hw_btcoex_init_3wire) },
	{ 0xc0dc6a06, __VMLINUX_SYMBOL_STR(ath_key_config) },
	{ 0x4f188877, __VMLINUX_SYMBOL_STR(ath9k_cmn_debug_modal_eeprom) },
	{ 0xe6e5021c, __VMLINUX_SYMBOL_STR(skb_put) },
	{ 0x53f6ffbc, __VMLINUX_SYMBOL_STR(wait_for_completion_timeout) },
	{ 0x4f6b400b, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0x149c7752, __VMLINUX_SYMBOL_STR(param_ops_uint) },
	{ 0x77591a0, __VMLINUX_SYMBOL_STR(ath9k_hw_reset_calvalid) },
	{ 0x6170e903, __VMLINUX_SYMBOL_STR(usb_free_urb) },
	{ 0xff46504d, __VMLINUX_SYMBOL_STR(release_firmware) },
	{ 0x3a4970c7, __VMLINUX_SYMBOL_STR(ieee80211_start_tx_ba_cb_irqsafe) },
	{ 0x6ed2be9c, __VMLINUX_SYMBOL_STR(usb_anchor_urb) },
	{ 0x4729d3f8, __VMLINUX_SYMBOL_STR(usb_alloc_urb) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=ath9k_hw,ath9k_common,compat,mac80211,ath,cfg80211";

MODULE_ALIAS("usb:v0CF3p9271d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v0CF3p1006d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v0846p9030d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v07D1p3A10d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v13D3p3327d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v13D3p3328d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v13D3p3346d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v13D3p3348d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v13D3p3349d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v13D3p3350d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v04CAp4605d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v040Dp3801d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v0CF3pB003d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v0CF3pB002d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v057Cp8403d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v0471p209Ed*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v0CF3p7015d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v1668p1200d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v0CF3p7010d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v0846p9018d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v083ApA704d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v0411p017Fd*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v0411p0197d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v04DAp3904d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v0CF3p20FFd*dc*dsc*dp*ic*isc*ip*in*");

MODULE_INFO(srcversion, "D4831E0797D4B3551B9EC44");
