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
#تعریف_کړه MAGIC_کلمه_اندازه 4
#تعریف_کړه BOOT_PHASE_STRING_اندازه 63

ساكن صحيح پرلپسې_dfu(صحيح usb_شاخص, حرف *interface, حرف *devstring)
{
	صحيح ret;

	ret = dfu_init_env_entities(interface, devstring);
	إذا (ret) {
		dfu_free_entities();
		اذهب وتل;
	}

	پرلپسې_usb_dnl_gadget(usb_شاخص, "usb_dnl_dfu");
وتل:
	dfu_free_entities();
	أرجع ret;
}

#که_تعریف_شوی CONFIG_SPL_PCI_DFU
ساكن صحيح dfu_over_pcie(فراغ)
{
	u32 دننه_واټن, magic_کلمه;
	متطاير فراغ *addr;
	بنية udevice *dev;
	بنية pci_bar bar;
	بنية pci_ep_سرلیک hdr;
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
	hdr.baseclass_کوډ = PCI_بنسټ_CLASS_حافظه;
	hdr.subclass_کوډ = PCI_CLASS_حافظه_RAM & 0xff;

	ret = pci_ep_write_سرلیک(dev, fn, &hdr);
	إذا (ret) {
		pr_err("Failed to write header: %d\n", ret);
		أرجع ret;
	}

	bar.barno = BAR_0;
	bar.phys_addr = (dma_addr_t)CONFIG_SPL_PCI_DFU_SPL_بارول_FIT_پته;
	bar.flags = PCI_بنسټ_پته_SPACE_حافظه |
			  PCI_بنسټ_پته_MEM_TYPE_32 |
			  PCI_بنسټ_پته_MEM_PREFETCH;

	bar.اندازه = CONFIG_SPL_PCI_DFU_BAR_اندازه;

	ret = pci_ep_سېټ_bar(dev, fn, &bar);
	إذا (ret) {
		pr_err("Failed to set bar: %d\n", ret);
		أرجع ret;
	}

	ret = pci_ep_پیل(dev);
	إذا (ret) {
		pr_err("Failed to start ep: %d\n", ret);
		أرجع ret;
	}

	addr = (فراغ *)CONFIG_SPL_PCI_DFU_SPL_بارول_FIT_پته;
	دننه_واټن = CONFIG_SPL_PCI_DFU_BAR_اندازه - MAGIC_کلمه_اندازه;

	إذا (حجم(CONFIG_SPL_PCI_DFU_BOOT_PHASE) > BOOT_PHASE_STRING_اندازه) {
		pr_err("Not copying boot phase. String too long\n");
	} وإلا {
		bootphase = (حرف *)(addr + CONFIG_SPL_PCI_DFU_BAR_اندازه -
				  (BOOT_PHASE_STRING_اندازه + MAGIC_کلمه_اندازه + 1));
		strlcpy(bootphase, CONFIG_SPL_PCI_DFU_BOOT_PHASE,
			حجم(CONFIG_SPL_PCI_DFU_BOOT_PHASE) + 1);
	}

	addr = addr + دننه_واټن;
	magic_کلمه = CONFIG_SPL_PCI_DFU_MAGIC_کلمه;
	(*(صحيح *)addr) = 0;
	flush_dcache_all();
	لكل (;;) {
		إذا (*(صحيح *)addr == magic_کلمه)
			اكسر;
		invalidate_dcache_all();
	}

	أرجع 0;
}
#د_که_پای

صحيح spl_dfu_cmd(صحيح usbctrl, حرف *dfu_alt_info, حرف *interface, حرف *devstr)
{
	حرف *str_env;
	صحيح ret;

#که_تعریف_شوی CONFIG_SPL_PCI_DFU
	إذا (spl_boot_device() == BOOT_DEVICE_PCIE)
		أرجع dfu_over_pcie();
#د_که_پای

	/* set default environment */
	env_سېټ_default(عدم, 0);
	str_env = env_get(dfu_alt_info);
	إذا (!str_env) {
		pr_err("\"%s\" env variable not defined!\n", dfu_alt_info);
		أرجع -EINVAL;
	}

	ret = env_سېټ("dfu_alt_info", str_env);
	إذا (ret) {
		pr_err("unable to set env variable \"dfu_alt_info\"!\n");
		أرجع -EINVAL;
	}

	/* invoke dfu command */
	أرجع پرلپسې_dfu(usbctrl, interface, devstr);
}
