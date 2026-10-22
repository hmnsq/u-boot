// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Texas Instruments, <www.ti.com>
 *
 * Ravi B <ravibabu@ti.com>
 */
#ضمن <env.h>
#ضمن <spl.h>
#ضمن <linux/compiler.h>
#ضمن <errno.h>
#ضمن <watchdog.h>
#ضمن <console.h>
#ضمن <g_dnl.h>
#ضمن <usb.h>
#ضمن <dfu.h>
#ضمن <linux/printk.h>
#ضمن <pci_ep.h>
#ضمن <dm/uclass.h>
#ضمن <cpu_func.h>
#ضمن <linux/io.h>

/*
 * Macros define size of magic word and boot phase string
 * in bytes.
 */
#وصف_ڪريو MAGIC_لفظ_سائيز 4
#وصف_ڪريو BOOT_PHASE_STRING_سائيز 63

ساكن صحيح لاڳيتو_dfu(صحيح usb_اشاريو, حرف *interface, حرف *devstring)
{
	صحيح ret;

	ret = dfu_init_env_entities(interface, devstring);
	إذا (ret) {
		dfu_free_entities();
		اذهب ٻاهر_نڪرو;
	}

	لاڳيتو_usb_dnl_gadget(usb_اشاريو, "usb_dnl_dfu");
ٻاهر_نڪرو:
	dfu_free_entities();
	أرجع ret;
}

#جيڪڏهن_وصف_ٿيل CONFIG_SPL_PCI_DFU
ساكن صحيح dfu_over_pcie(فراغ)
{
	u32 وٿي, magic_لفظ;
	متطاير فراغ *addr;
	بنية udevice *dev;
	بنية pci_bar bar;
	بنية pci_ep_سرو hdr;
	uint fn = 0;
	صحيح ret;
	حرف *bootphase;

	uclass_get_device_by_seq(UCLASS_PCI_EP, 0, &dev);
	إذا (!dev) {
		pr_err("Failed to get pci ep device\n");
		أرجع -ENODEV;
	}

	hdr.deviceid = CONFIG_SPL_PCI_DFU_DEVICE_ID;
	hdr.vendorid = CONFIG_SPL_PCI_DFU_VENDOR_ID;
	hdr.baseclass_ڪوڊ = PCI_بنياد_CLASS_يادگيري;
	hdr.subclass_ڪوڊ = PCI_CLASS_يادگيري_RAM & 0xff;

	ret = pci_ep_write_سرو(dev, fn, &hdr);
	إذا (ret) {
		pr_err("Failed to write header: %d\n", ret);
		أرجع ret;
	}

	bar.barno = BAR_0;
	bar.phys_addr = (dma_addr_t)CONFIG_SPL_PCI_DFU_SPL_ڀريو_FIT_پتو;
	bar.flags = PCI_بنياد_پتو_SPACE_يادگيري |
			  PCI_بنياد_پتو_MEM_TYPE_32 |
			  PCI_بنياد_پتو_MEM_PREFETCH;

	bar.سائيز = CONFIG_SPL_PCI_DFU_BAR_سائيز;

	ret = pci_ep_سيٽ_bar(dev, fn, &bar);
	إذا (ret) {
		pr_err("Failed to set bar: %d\n", ret);
		أرجع ret;
	}

	ret = pci_ep_شروعات(dev);
	إذا (ret) {
		pr_err("Failed to start ep: %d\n", ret);
		أرجع ret;
	}

	addr = (فراغ *)CONFIG_SPL_PCI_DFU_SPL_ڀريو_FIT_پتو;
	وٿي = CONFIG_SPL_PCI_DFU_BAR_سائيز - MAGIC_لفظ_سائيز;

	إذا (حجم(CONFIG_SPL_PCI_DFU_BOOT_PHASE) > BOOT_PHASE_STRING_سائيز) {
		pr_err("Not copying boot phase. String too long\n");
	} وإلا {
		bootphase = (حرف *)(addr + CONFIG_SPL_PCI_DFU_BAR_سائيز -
				  (BOOT_PHASE_STRING_سائيز + MAGIC_لفظ_سائيز + 1));
		strlcpy(bootphase, CONFIG_SPL_PCI_DFU_BOOT_PHASE,
			حجم(CONFIG_SPL_PCI_DFU_BOOT_PHASE) + 1);
	}

	addr = addr + وٿي;
	magic_لفظ = CONFIG_SPL_PCI_DFU_MAGIC_لفظ;
	(*(صحيح *)addr) = 0;
	flush_dcache_all();
	لكل (;;) {
		إذا (*(صحيح *)addr == magic_لفظ)
			اكسر;
		invalidate_dcache_all();
	}

	أرجع 0;
}
#جيڪڏهن_پورو

صحيح spl_dfu_cmd(صحيح usbctrl, حرف *dfu_alt_info, حرف *interface, حرف *devstr)
{
	حرف *str_env;
	صحيح ret;

#جيڪڏهن_وصف_ٿيل CONFIG_SPL_PCI_DFU
	إذا (spl_boot_device() == BOOT_DEVICE_PCIE)
		أرجع dfu_over_pcie();
#جيڪڏهن_پورو

	/* set default environment */
	env_سيٽ_default(عدم, 0);
	str_env = env_get(dfu_alt_info);
	إذا (!str_env) {
		pr_err("\"%s\" env variable not defined!\n", dfu_alt_info);
		أرجع -EINVAL;
	}

	ret = env_سيٽ("dfu_alt_info", str_env);
	إذا (ret) {
		pr_err("unable to set env variable \"dfu_alt_info\"!\n");
		أرجع -EINVAL;
	}

	/* invoke dfu command */
	أرجع لاڳيتو_dfu(usbctrl, interface, devstr);
}
