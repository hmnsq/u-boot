// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Texas Instruments, <www.ti.com>
 *
 * Ravi B <ravibabu@ti.com>
 */
#অন্তর্ভাব <env.h>
#অন্তর্ভাব <spl.h>
#অন্তর্ভাব <linux/compiler.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <watchdog.h>
#অন্তর্ভাব <console.h>
#অন্তর্ভাব <g_dnl.h>
#অন্তর্ভাব <usb.h>
#অন্তর্ভাব <dfu.h>
#অন্তর্ভাব <linux/printk.h>
#অন্তর্ভাব <pci_ep.h>
#অন্তর্ভাব <dm/uclass.h>
#অন্তর্ভাব <cpu_func.h>
#অন্তর্ভাব <linux/io.h>

/*
 * Macros define size of magic word and boot phase string
 * in bytes.
 */
#define MAGIC_WORD_আকার 4
#define BOOT_PHASE_STRING_আকার 63

স্থির পূর্ণসংখ্যা run_dfu(পূর্ণসংখ্যা usb_সূচক, অক্ষর *interface, অক্ষর *devstring)
{
	পূর্ণসংখ্যা ret;

	ret = dfu_init_env_entities(interface, devstring);
	যদি (ret) {
		dfu_free_entities();
		যাও exit;
	}

	run_usb_dnl_gadget(usb_সূচক, "usb_dnl_dfu");
exit:
	dfu_free_entities();
	ফেরত ret;
}

#ifdef CONFIG_SPL_PCI_DFU
স্থির পূর্ণসংখ্যা dfu_over_pcie(শূন্য)
{
	u32 offset, magic_word;
	অস্থির শূন্য *addr;
	কাঠামো udevice *dev;
	কাঠামো pci_bar bar;
	কাঠামো pci_ep_header hdr;
	uint fn = 0;
	পূর্ণসংখ্যা ret;
	অক্ষর *bootphase;

	uclass_get_device_by_seq(UCLASS_PCI_EP, 0, &dev);
	যদি (!dev) {
		pr_err("Failed to get pci ep device\n");
		ফেরত -ENODEV;
	}

	hdr.deviceid = CONFIG_SPL_PCI_DFU_DEVICE_ID;
	hdr.vendorid = CONFIG_SPL_PCI_DFU_VENDOR_ID;
	hdr.baseclass_code = PCI_BASE_CLASS_MEMORY;
	hdr.subclass_code = PCI_CLASS_MEMORY_RAM & 0xff;

	ret = pci_ep_write_header(dev, fn, &hdr);
	যদি (ret) {
		pr_err("Failed to write header: %d\n", ret);
		ফেরত ret;
	}

	bar.barno = BAR_0;
	bar.phys_addr = (dma_addr_t)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	bar.flags = PCI_BASE_ADDRESS_SPACE_MEMORY |
			  PCI_BASE_ADDRESS_MEM_TYPE_32 |
			  PCI_BASE_ADDRESS_MEM_PREFETCH;

	bar.আকার_2 = CONFIG_SPL_PCI_DFU_BAR_আকার;

	ret = pci_ep_set_bar(dev, fn, &bar);
	যদি (ret) {
		pr_err("Failed to set bar: %d\n", ret);
		ফেরত ret;
	}

	ret = pci_ep_start(dev);
	যদি (ret) {
		pr_err("Failed to start ep: %d\n", ret);
		ফেরত ret;
	}

	addr = (শূন্য *)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	offset = CONFIG_SPL_PCI_DFU_BAR_আকার - MAGIC_WORD_আকার;

	যদি (আকার(CONFIG_SPL_PCI_DFU_BOOT_PHASE) > BOOT_PHASE_STRING_আকার) {
		pr_err("Not copying boot phase. String too long\n");
	} অন্যথা {
		bootphase = (অক্ষর *)(addr + CONFIG_SPL_PCI_DFU_BAR_আকার -
				  (BOOT_PHASE_STRING_আকার + MAGIC_WORD_আকার + 1));
		strlcpy(bootphase, CONFIG_SPL_PCI_DFU_BOOT_PHASE,
			আকার(CONFIG_SPL_PCI_DFU_BOOT_PHASE) + 1);
	}

	addr = addr + offset;
	magic_word = CONFIG_SPL_PCI_DFU_MAGIC_WORD;
	(*(পূর্ণসংখ্যা *)addr) = 0;
	flush_dcache_all();
	জন্য (;;) {
		যদি (*(পূর্ণসংখ্যা *)addr == magic_word)
			বিরতি;
		invalidate_dcache_all();
	}

	ফেরত 0;
}
#endif

পূর্ণসংখ্যা spl_dfu_cmd(পূর্ণসংখ্যা usbctrl, অক্ষর *dfu_alt_info, অক্ষর *interface, অক্ষর *devstr)
{
	অক্ষর *str_env;
	পূর্ণসংখ্যা ret;

#ifdef CONFIG_SPL_PCI_DFU
	যদি (spl_boot_device() == BOOT_DEVICE_PCIE)
		ফেরত dfu_over_pcie();
#endif

	/* set default environment */
	env_set_default(ফাঁকা, 0);
	str_env = env_get(dfu_alt_info);
	যদি (!str_env) {
		pr_err("\"%s\" env variable not defined!\n", dfu_alt_info);
		ফেরত -EINVAL;
	}

	ret = env_set("dfu_alt_info", str_env);
	যদি (ret) {
		pr_err("unable to set env variable \"dfu_alt_info\"!\n");
		ফেরত -EINVAL;
	}

	/* invoke dfu command */
	ফেরত run_dfu(usbctrl, interface, devstr);
}
