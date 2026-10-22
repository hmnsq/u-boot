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
#تعریف_کرو MAGIC_لفظ_ناپ 4
#تعریف_کرو BOOT_PHASE_STRING_ناپ 63

ساكن صحيح لگاتار_dfu(صحيح usb_اشاریہ, حرف *interface, حرف *devstring)
{
	صحيح ret;

	ret = dfu_init_env_entities(interface, devstring);
	إذا (ret) {
		dfu_free_entities();
		اذهب باہر_نکلو;
	}

	لگاتار_usb_dnl_gadget(usb_اشاریہ, "usb_dnl_dfu");
باہر_نکلو:
	dfu_free_entities();
	أرجع ret;
}

#جے_تعریف_شدہ CONFIG_SPL_PCI_DFU
ساكن صحيح dfu_over_pcie(فراغ)
{
	u32 وتھ, magic_لفظ;
	متطاير فراغ *addr;
	بنية udevice *dev;
	بنية pci_bar bar;
	بنية pci_ep_سرخی hdr;
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
	hdr.baseclass_کوڈ = PCI_ادھار_CLASS_حافظہ;
	hdr.subclass_کوڈ = PCI_CLASS_حافظہ_RAM & 0xff;

	ret = pci_ep_write_سرخی(dev, fn, &hdr);
	إذا (ret) {
		pr_err("Failed to write header: %d\n", ret);
		أرجع ret;
	}

	bar.barno = BAR_0;
	bar.phys_addr = (dma_addr_t)CONFIG_SPL_PCI_DFU_SPL_لدو_FIT_پتہ;
	bar.flags = PCI_ادھار_پتہ_SPACE_حافظہ |
			  PCI_ادھار_پتہ_MEM_TYPE_32 |
			  PCI_ادھار_پتہ_MEM_PREFETCH;

	bar.ناپ = CONFIG_SPL_PCI_DFU_BAR_ناپ;

	ret = pci_ep_سیٹ_bar(dev, fn, &bar);
	إذا (ret) {
		pr_err("Failed to set bar: %d\n", ret);
		أرجع ret;
	}

	ret = pci_ep_شروع(dev);
	إذا (ret) {
		pr_err("Failed to start ep: %d\n", ret);
		أرجع ret;
	}

	addr = (فراغ *)CONFIG_SPL_PCI_DFU_SPL_لدو_FIT_پتہ;
	وتھ = CONFIG_SPL_PCI_DFU_BAR_ناپ - MAGIC_لفظ_ناپ;

	إذا (حجم(CONFIG_SPL_PCI_DFU_BOOT_PHASE) > BOOT_PHASE_STRING_ناپ) {
		pr_err("Not copying boot phase. String too long\n");
	} وإلا {
		bootphase = (حرف *)(addr + CONFIG_SPL_PCI_DFU_BAR_ناپ -
				  (BOOT_PHASE_STRING_ناپ + MAGIC_لفظ_ناپ + 1));
		strlcpy(bootphase, CONFIG_SPL_PCI_DFU_BOOT_PHASE,
			حجم(CONFIG_SPL_PCI_DFU_BOOT_PHASE) + 1);
	}

	addr = addr + وتھ;
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
#جے_انت

صحيح spl_dfu_cmd(صحيح usbctrl, حرف *dfu_alt_info, حرف *interface, حرف *devstr)
{
	حرف *str_env;
	صحيح ret;

#جے_تعریف_شدہ CONFIG_SPL_PCI_DFU
	إذا (spl_boot_device() == BOOT_DEVICE_PCIE)
		أرجع dfu_over_pcie();
#جے_انت

	/* set default environment */
	env_سیٹ_default(عدم, 0);
	str_env = env_get(dfu_alt_info);
	إذا (!str_env) {
		pr_err("\"%s\" env variable not defined!\n", dfu_alt_info);
		أرجع -EINVAL;
	}

	ret = env_سیٹ("dfu_alt_info", str_env);
	إذا (ret) {
		pr_err("unable to set env variable \"dfu_alt_info\"!\n");
		أرجع -EINVAL;
	}

	/* invoke dfu command */
	أرجع لگاتار_dfu(usbctrl, interface, devstr);
}
