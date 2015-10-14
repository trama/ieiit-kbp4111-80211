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
	{ 0x67ae9e57, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x619cb7dd, __VMLINUX_SYMBOL_STR(simple_read_from_buffer) },
	{ 0xb43a926b, __VMLINUX_SYMBOL_STR(backport_dependency_symbol) },
	{ 0x754d539c, __VMLINUX_SYMBOL_STR(strlen) },
	{ 0x60a13e90, __VMLINUX_SYMBOL_STR(rcu_barrier) },
	{ 0x27571922, __VMLINUX_SYMBOL_STR(relay_file_operations) },
	{ 0x8e1c00b7, __VMLINUX_SYMBOL_STR(ath9k_hw_getchan_noise) },
	{ 0x7ef39823, __VMLINUX_SYMBOL_STR(ieee80211_hdrlen) },
	{ 0xdd905bef, __VMLINUX_SYMBOL_STR(ath9k_hw_setrxfilter) },
	{ 0x583ada4, __VMLINUX_SYMBOL_STR(debugfs_create_file) },
	{ 0x7a2af7b4, __VMLINUX_SYMBOL_STR(cpu_number) },
	{ 0x91715312, __VMLINUX_SYMBOL_STR(sprintf) },
	{ 0x99fad3b5, __VMLINUX_SYMBOL_STR(relay_switch_subbuf) },
	{ 0x6fff62a6, __VMLINUX_SYMBOL_STR(ath_printk) },
	{ 0x3c80c06c, __VMLINUX_SYMBOL_STR(kstrtoull) },
	{ 0x8d9fab5, __VMLINUX_SYMBOL_STR(relay_close) },
	{ 0xf62e29d3, __VMLINUX_SYMBOL_STR(ath9k_hw_gettsf64) },
	{ 0x9e8eb6bc, __VMLINUX_SYMBOL_STR(default_llseek) },
	{ 0x8c8e9c4f, __VMLINUX_SYMBOL_STR(ath9k_hw_getrxfilter) },
	{ 0x22574c5, __VMLINUX_SYMBOL_STR(debugfs_remove) },
	{ 0x16305289, __VMLINUX_SYMBOL_STR(warn_slowpath_null) },
	{ 0x2fde371c, __VMLINUX_SYMBOL_STR(skb_pull) },
	{ 0xe9b97762, __VMLINUX_SYMBOL_STR(simple_open) },
	{ 0x78764f4e, __VMLINUX_SYMBOL_STR(pv_irq_ops) },
	{ 0xdb7305a1, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
	{ 0xd20c3e5, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x39195055, __VMLINUX_SYMBOL_STR(ieee80211_get_hdrlen_from_skb) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x69acdf38, __VMLINUX_SYMBOL_STR(memcpy) },
	{ 0x4ca9669f, __VMLINUX_SYMBOL_STR(scnprintf) },
	{ 0x562560e1, __VMLINUX_SYMBOL_STR(relay_open) },
	{ 0xb0e602eb, __VMLINUX_SYMBOL_STR(memmove) },
	{ 0x35f53dcb, __VMLINUX_SYMBOL_STR(devm_kmalloc) },
	{ 0x4f6b400b, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0x80ee8af4, __VMLINUX_SYMBOL_STR(ath_hw_keyreset) },
	{ 0xbe76c095, __VMLINUX_SYMBOL_STR(ath9k_hw_set_txpowerlimit) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=compat,ath9k_hw,cfg80211,ath";


MODULE_INFO(srcversion, "038CA4E184C7F6B8B156CA6");
