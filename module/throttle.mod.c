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

#ifdef RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xc2996440, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0xc072ae1f, __VMLINUX_SYMBOL_STR(simple_attr_release) },
	{ 0xffbba05a, __VMLINUX_SYMBOL_STR(simple_attr_write) },
	{ 0xffe4d8da, __VMLINUX_SYMBOL_STR(simple_attr_read) },
	{ 0x3119d8e3, __VMLINUX_SYMBOL_STR(generic_file_llseek) },
	{ 0x7b7bc644, __VMLINUX_SYMBOL_STR(debugfs_remove_recursive) },
	{ 0xc7c2ba8, __VMLINUX_SYMBOL_STR(debugfs_create_u32) },
	{ 0x6ac59749, __VMLINUX_SYMBOL_STR(debugfs_create_file_unsafe) },
	{ 0xc2044f08, __VMLINUX_SYMBOL_STR(debugfs_create_dir) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xf24b3dfe, __VMLINUX_SYMBOL_STR(__ioremap) },
	{ 0x28a1ff42, __VMLINUX_SYMBOL_STR(simple_attr_open) },
	{ 0x1fdc7df2, __VMLINUX_SYMBOL_STR(_mcount) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

