#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x888b8f57, "strcmp" },
	{ 0xcb8b6ec6, "kfree" },
	{ 0xe8213e80, "_printk" },
	{ 0x9e3a8e47, "_raw_write_unlock" },
	{ 0xbd03ed67, "random_kmalloc_seed" },
	{ 0xa62b1cc9, "kmalloc_caches" },
	{ 0xd1f07d8f, "__kmalloc_cache_noprof" },
	{ 0xc609ff70, "strncpy" },
	{ 0x90a48d82, "__ubsan_handle_out_of_bounds" },
	{ 0x9e3a8e47, "_raw_read_lock" },
	{ 0x40a621c5, "snprintf" },
	{ 0x9e3a8e47, "_raw_read_unlock" },
	{ 0x092a35a2, "_copy_to_user" },
	{ 0x2435d559, "strncmp" },
	{ 0x9479a1e8, "strnlen" },
	{ 0xe54e0a6b, "__fortify_panic" },
	{ 0x89258034, "device_destroy" },
	{ 0x7c77f2d5, "class_destroy" },
	{ 0x52b15b3b, "__unregister_chrdev" },
	{ 0x3c98aa3c, "__register_chrdev" },
	{ 0xfb3de43c, "class_create" },
	{ 0x773c4019, "device_create" },
	{ 0xac16aca0, "kern_path" },
	{ 0xd272d446, "__fentry__" },
	{ 0x5a844b26, "__x86_indirect_thunk_rax" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0xa61fd7aa, "__check_object_size" },
	{ 0x092a35a2, "_copy_from_user" },
	{ 0x173ec8da, "sscanf" },
	{ 0x9e3a8e47, "_raw_write_lock" },
	{ 0xab006604, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0x888b8f57,
	0xcb8b6ec6,
	0xe8213e80,
	0x9e3a8e47,
	0xbd03ed67,
	0xa62b1cc9,
	0xd1f07d8f,
	0xc609ff70,
	0x90a48d82,
	0x9e3a8e47,
	0x40a621c5,
	0x9e3a8e47,
	0x092a35a2,
	0x2435d559,
	0x9479a1e8,
	0xe54e0a6b,
	0x89258034,
	0x7c77f2d5,
	0x52b15b3b,
	0x3c98aa3c,
	0xfb3de43c,
	0x773c4019,
	0xac16aca0,
	0xd272d446,
	0x5a844b26,
	0xd272d446,
	0xd272d446,
	0xa61fd7aa,
	0x092a35a2,
	0x173ec8da,
	0x9e3a8e47,
	0xab006604,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"strcmp\0"
	"kfree\0"
	"_printk\0"
	"_raw_write_unlock\0"
	"random_kmalloc_seed\0"
	"kmalloc_caches\0"
	"__kmalloc_cache_noprof\0"
	"strncpy\0"
	"__ubsan_handle_out_of_bounds\0"
	"_raw_read_lock\0"
	"snprintf\0"
	"_raw_read_unlock\0"
	"_copy_to_user\0"
	"strncmp\0"
	"strnlen\0"
	"__fortify_panic\0"
	"device_destroy\0"
	"class_destroy\0"
	"__unregister_chrdev\0"
	"__register_chrdev\0"
	"class_create\0"
	"device_create\0"
	"kern_path\0"
	"__fentry__\0"
	"__x86_indirect_thunk_rax\0"
	"__x86_return_thunk\0"
	"__stack_chk_fail\0"
	"__check_object_size\0"
	"_copy_from_user\0"
	"sscanf\0"
	"_raw_write_lock\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "4E745AD9D57D3646D11D3EA");
