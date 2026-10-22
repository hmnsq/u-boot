// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Texas Instruments, <www.ti.com>
 *
 * Ravi B <ravibabu@ti.com>
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/compiler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <watchdog.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <console.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <g_dnl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <usb.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dfu.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/printk.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <pci_ep.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm/uclass.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <cpu_func.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/io.h>

/*
 * Macros define size of magic word and boot phase string
 * in bytes.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAGIC_ਸ਼ਬਦ_ਮਾਪ 4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BOOT_PHASE_STRING_ਮਾਪ 63

ਸਥਿਰ ਪੂਰਨ ਲੜੀਵਾਰ_dfu(ਪੂਰਨ usb_ਸੂਚਕਾਂਕ, ਅੱਖਰ *interface, ਅੱਖਰ *devstring)
{
	ਪੂਰਨ ret;

	ret = dfu_init_env_entities(interface, devstring);
	ਜੇ (ret) {
		dfu_free_entities();
		ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;
	}

	ਲੜੀਵਾਰ_usb_dnl_gadget(usb_ਸੂਚਕਾਂਕ, "usb_dnl_dfu");
ਬਾਹਰ_ਨਿਕਲੋ:
	dfu_free_entities();
	ਵਾਪਸ ret;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPL_PCI_DFU
ਸਥਿਰ ਪੂਰਨ dfu_over_pcie(ਖਾਲੀ)
{
	u32 ਵਿੱਥ, magic_ਸ਼ਬਦ;
	ਅਸਥਿਰ ਖਾਲੀ *addr;
	ਬਣਤਰ udevice *dev;
	ਬਣਤਰ pci_bar bar;
	ਬਣਤਰ pci_ep_ਸਿਰਲੇਖ hdr;
	uint fn = 0;
	ਪੂਰਨ ret;
	ਅੱਖਰ *bootphase;

	uclass_get_device_by_seq(UCLASS_PCI_EP, 0, &dev);
	ਜੇ (!dev) {
		pr_err("Failed to get pci ep device\n");
		ਵਾਪਸ -ENODEV;
	}

	hdr.deviceid = CONFIG_SPL_PCI_DFU_DEVICE_ID;
	hdr.vendorid = CONFIG_SPL_PCI_DFU_VENDOR_ID;
	hdr.baseclass_ਕੋਡ = PCI_ਅਧਾਰ_CLASS_ਸਮ੍ਰਿਤੀ;
	hdr.subclass_ਕੋਡ = PCI_CLASS_ਸਮ੍ਰਿਤੀ_RAM & 0xff;

	ret = pci_ep_write_ਸਿਰਲੇਖ(dev, fn, &hdr);
	ਜੇ (ret) {
		pr_err("Failed to write header: %d\n", ret);
		ਵਾਪਸ ret;
	}

	bar.barno = BAR_0;
	bar.phys_addr = (dma_addr_t)CONFIG_SPL_PCI_DFU_SPL_ਲੱਦੋ_FIT_ਪਤਾ;
	bar.flags = PCI_ਅਧਾਰ_ਪਤਾ_SPACE_ਸਮ੍ਰਿਤੀ |
			  PCI_ਅਧਾਰ_ਪਤਾ_MEM_TYPE_32 |
			  PCI_ਅਧਾਰ_ਪਤਾ_MEM_PREFETCH;

	bar.ਮਾਪ = CONFIG_SPL_PCI_DFU_BAR_ਮਾਪ;

	ret = pci_ep_ਸੈੱਟ_bar(dev, fn, &bar);
	ਜੇ (ret) {
		pr_err("Failed to set bar: %d\n", ret);
		ਵਾਪਸ ret;
	}

	ret = pci_ep_ਸ਼ੁਰੂ(dev);
	ਜੇ (ret) {
		pr_err("Failed to start ep: %d\n", ret);
		ਵਾਪਸ ret;
	}

	addr = (ਖਾਲੀ *)CONFIG_SPL_PCI_DFU_SPL_ਲੱਦੋ_FIT_ਪਤਾ;
	ਵਿੱਥ = CONFIG_SPL_PCI_DFU_BAR_ਮਾਪ - MAGIC_ਸ਼ਬਦ_ਮਾਪ;

	ਜੇ (ਆਕਾਰ(CONFIG_SPL_PCI_DFU_BOOT_PHASE) > BOOT_PHASE_STRING_ਮਾਪ) {
		pr_err("Not copying boot phase. String too long\n");
	} ਨਹੀਂਤਾਂ {
		bootphase = (ਅੱਖਰ *)(addr + CONFIG_SPL_PCI_DFU_BAR_ਮਾਪ -
				  (BOOT_PHASE_STRING_ਮਾਪ + MAGIC_ਸ਼ਬਦ_ਮਾਪ + 1));
		strlcpy(bootphase, CONFIG_SPL_PCI_DFU_BOOT_PHASE,
			ਆਕਾਰ(CONFIG_SPL_PCI_DFU_BOOT_PHASE) + 1);
	}

	addr = addr + ਵਿੱਥ;
	magic_ਸ਼ਬਦ = CONFIG_SPL_PCI_DFU_MAGIC_ਸ਼ਬਦ;
	(*(ਪੂਰਨ *)addr) = 0;
	flush_dcache_all();
	ਲਈ (;;) {
		ਜੇ (*(ਪੂਰਨ *)addr == magic_ਸ਼ਬਦ)
			ਰੋਕੋ;
		invalidate_dcache_all();
	}

	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

ਪੂਰਨ spl_dfu_cmd(ਪੂਰਨ usbctrl, ਅੱਖਰ *dfu_alt_info, ਅੱਖਰ *interface, ਅੱਖਰ *devstr)
{
	ਅੱਖਰ *str_env;
	ਪੂਰਨ ret;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPL_PCI_DFU
	ਜੇ (spl_boot_device() == BOOT_DEVICE_PCIE)
		ਵਾਪਸ dfu_over_pcie();
#ਜੇ_ਅੰਤ

	/* set default environment */
	env_ਸੈੱਟ_default(ਨਿਰਾ, 0);
	str_env = env_get(dfu_alt_info);
	ਜੇ (!str_env) {
		pr_err("\"%s\" env variable not defined!\n", dfu_alt_info);
		ਵਾਪਸ -EINVAL;
	}

	ret = env_ਸੈੱਟ("dfu_alt_info", str_env);
	ਜੇ (ret) {
		pr_err("unable to set env variable \"dfu_alt_info\"!\n");
		ਵਾਪਸ -EINVAL;
	}

	/* invoke dfu command */
	ਵਾਪਸ ਲੜੀਵਾਰ_dfu(usbctrl, interface, devstr);
}
