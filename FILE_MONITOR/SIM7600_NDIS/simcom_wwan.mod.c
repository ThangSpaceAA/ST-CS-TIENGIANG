#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
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

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xcaec5711, "module_layout" },
	{ 0xdc2a1dc7, "usbnet_disconnect" },
	{ 0xf783bd33, "usbnet_probe" },
	{ 0xb9635609, "usb_deregister" },
	{ 0x1742c223, "usb_register_driver" },
	{ 0xe35f48a3, "skb_push" },
	{ 0xf311d29d, "__dev_kfree_skb_any" },
	{ 0x69972bcf, "_dev_err" },
	{ 0x70959bdb, "skb_pull" },
	{ 0x4ad37d04, "usbnet_suspend" },
	{ 0x5b2eaf64, "usbnet_resume" },
	{ 0xc962d6a0, "usb_control_msg" },
	{ 0xc5850110, "printk" },
	{ 0x497e164e, "_dev_info" },
	{ 0x9a7cfe63, "usbnet_get_endpoints" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("usb:v1E0Ep9025d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v1E0Ep9001d*dc*dsc*dp*ic*isc*ip*in*");

MODULE_INFO(srcversion, "ACED7068FFE31726F8F05EE");
