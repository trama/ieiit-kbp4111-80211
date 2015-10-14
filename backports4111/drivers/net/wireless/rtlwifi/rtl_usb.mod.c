#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xfc315ed4, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x67ae9e57, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0xe3c16138, __VMLINUX_SYMBOL_STR(rtl_deinit_deferred_work) },
	{ 0x3f9ca9b1, __VMLINUX_SYMBOL_STR(rtl_deinit_core) },
	{ 0xc5849e96, __VMLINUX_SYMBOL_STR(usb_get_from_anchor) },
	{ 0x88bfa7e, __VMLINUX_SYMBOL_STR(cancel_work_sync) },
	{ 0x448eac3e, __VMLINUX_SYMBOL_STR(kmemdup) },
	{ 0x1a99e4f0, __VMLINUX_SYMBOL_STR(ieee80211_unregister_hw) },
	{ 0x59b1ad64, __VMLINUX_SYMBOL_STR(__dev_kfree_skb_any) },
	{ 0x4629334c, __VMLINUX_SYMBOL_STR(__preempt_count) },
	{ 0xb8c7d434, __VMLINUX_SYMBOL_STR(usb_unanchor_urb) },
	{ 0xf7da0f71, __VMLINUX_SYMBOL_STR(__netdev_alloc_skb) },
	{ 0xf432dd3d, __VMLINUX_SYMBOL_STR(__init_waitqueue_head) },
	{ 0x6d0aba34, __VMLINUX_SYMBOL_STR(wait_for_completion) },
	{ 0x62a1fce6, __VMLINUX_SYMBOL_STR(skb_queue_purge) },
	{ 0x8f64aa4, __VMLINUX_SYMBOL_STR(_raw_spin_unlock_irqrestore) },
	{ 0x50ba1f0, __VMLINUX_SYMBOL_STR(ieee80211_alloc_hw_nm) },
	{ 0xdd04a3e9, __VMLINUX_SYMBOL_STR(__mutex_init) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x4eb01ee1, __VMLINUX_SYMBOL_STR(rtl_init_core) },
	{ 0xfaef0ed, __VMLINUX_SYMBOL_STR(__tasklet_schedule) },
	{ 0x39445718, __VMLINUX_SYMBOL_STR(rtl_action_proc) },
	{ 0x4bee5ce7, __VMLINUX_SYMBOL_STR(rtl_dbgp_flag_init) },
	{ 0xd68ea71d, __VMLINUX_SYMBOL_STR(usb_control_msg) },
	{ 0x16305289, __VMLINUX_SYMBOL_STR(warn_slowpath_null) },
	{ 0x54781aab, __VMLINUX_SYMBOL_STR(ieee80211_rx) },
	{ 0x8c03d20c, __VMLINUX_SYMBOL_STR(destroy_workqueue) },
	{ 0x6b4de810, __VMLINUX_SYMBOL_STR(rtl_lps_change_work_callback) },
	{ 0x95b44b58, __VMLINUX_SYMBOL_STR(rtl_init_rx_config) },
	{ 0x2fde371c, __VMLINUX_SYMBOL_STR(skb_pull) },
	{ 0x786ab9cc, __VMLINUX_SYMBOL_STR(usb_free_coherent) },
	{ 0x42160169, __VMLINUX_SYMBOL_STR(flush_workqueue) },
	{ 0x82072614, __VMLINUX_SYMBOL_STR(tasklet_kill) },
	{ 0xcda4999c, __VMLINUX_SYMBOL_STR(skb_queue_tail) },
	{ 0xdd71a0c, __VMLINUX_SYMBOL_STR(usb_submit_urb) },
	{ 0x37c6b2ad, __VMLINUX_SYMBOL_STR(usb_get_dev) },
	{ 0xff1076fc, __VMLINUX_SYMBOL_STR(usb_kill_anchored_urbs) },
	{ 0xdb7305a1, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0xeb69d4c7, __VMLINUX_SYMBOL_STR(usb_put_dev) },
	{ 0x421db7d9, __VMLINUX_SYMBOL_STR(ieee80211_tx_status_irqsafe) },
	{ 0x279198e5, __VMLINUX_SYMBOL_STR(rtl_ops) },
	{ 0xea166f3f, __VMLINUX_SYMBOL_STR(kfree_skb) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
	{ 0xd20c3e5, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x9327f5ce, __VMLINUX_SYMBOL_STR(_raw_spin_lock_irqsave) },
	{ 0xce290f0f, __VMLINUX_SYMBOL_STR(ieee80211_register_hw) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x69acdf38, __VMLINUX_SYMBOL_STR(memcpy) },
	{ 0xd2ef6f36, __VMLINUX_SYMBOL_STR(rtl_ips_nic_on) },
	{ 0x4d766d3d, __VMLINUX_SYMBOL_STR(ieee80211_free_hw) },
	{ 0xb63ca742, __VMLINUX_SYMBOL_STR(skb_dequeue) },
	{ 0x4b06d2e7, __VMLINUX_SYMBOL_STR(complete) },
	{ 0x8cd1fc97, __VMLINUX_SYMBOL_STR(usb_alloc_coherent) },
	{ 0xe6e5021c, __VMLINUX_SYMBOL_STR(skb_put) },
	{ 0x6170e903, __VMLINUX_SYMBOL_STR(usb_free_urb) },
	{ 0x52f0151d, __VMLINUX_SYMBOL_STR(rtl_beacon_statistic) },
	{ 0x6ed2be9c, __VMLINUX_SYMBOL_STR(usb_anchor_urb) },
	{ 0x4729d3f8, __VMLINUX_SYMBOL_STR(usb_alloc_urb) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=rtlwifi,mac80211";


MODULE_INFO(srcversion, "761FFB6B90B4ED0D3C253B5");
