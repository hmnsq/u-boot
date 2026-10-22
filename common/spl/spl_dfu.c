// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Texas Instruments, <www.ti.com>
 *
 * Ravi B <ravibabu@ti.com>
 */
#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <linux/compiler.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <watchdog.h>
#ଅନ୍ତର୍ଭାଵ <console.h>
#ଅନ୍ତର୍ଭାଵ <g_dnl.h>
#ଅନ୍ତର୍ଭାଵ <usb.h>
#ଅନ୍ତର୍ଭାଵ <dfu.h>
#ଅନ୍ତର୍ଭାଵ <linux/printk.h>
#ଅନ୍ତର୍ଭାଵ <pci_ep.h>
#ଅନ୍ତର୍ଭାଵ <dm/uclass.h>
#ଅନ୍ତର୍ଭାଵ <cpu_func.h>
#ଅନ୍ତର୍ଭାଵ <linux/io.h>

/*
 * Macros define size of magic word and boot phase string
 * in bytes.
 */
#define MAGIC_WORD_SIZE 4
#define BOOT_PHASE_STRING_SIZE 63

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_dfu(ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_index, ଅକ୍ଷର *interface, ଅକ୍ଷର *devstring)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ret = dfu_init_env_entities(interface, devstring);
	ଯଦି (ret) {
		dfu_free_entities();
		ଯାଅ exit;
	}

	run_usb_dnl_gadget(usb_index, "usb_dnl_dfu");
exit:
	dfu_free_entities();
	ଫେରନ୍ତୁ ret;
}

#ifdef CONFIG_SPL_PCI_DFU
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dfu_over_pcie(ଶୂନ୍ୟ)
{
	u32 offset, magic_word;
	ଅସ୍ଥିର ଶୂନ୍ୟ *addr;
	ସଂରଚନା udevice *dev;
	ସଂରଚନା pci_bar bar;
	ସଂରଚନା pci_ep_header hdr;
	uint fn = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
	ଅକ୍ଷର *bootphase;

	uclass_get_device_by_seq(UCLASS_PCI_EP, 0, &dev);
	ଯଦି (!dev) {
		pr_err("Failed to get pci ep device\n");
		ଫେରନ୍ତୁ -ENODEV;
	}

	hdr.deviceid = CONFIG_SPL_PCI_DFU_DEVICE_ID;
	hdr.vendorid = CONFIG_SPL_PCI_DFU_VENDOR_ID;
	hdr.baseclass_code = PCI_BASE_CLASS_MEMORY;
	hdr.subclass_code = PCI_CLASS_MEMORY_RAM & 0xff;

	ret = pci_ep_write_header(dev, fn, &hdr);
	ଯଦି (ret) {
		pr_err("Failed to write header: %d\n", ret);
		ଫେରନ୍ତୁ ret;
	}

	bar.barno = BAR_0;
	bar.phys_addr = (dma_addr_t)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	bar.flags = PCI_BASE_ADDRESS_SPACE_MEMORY |
			  PCI_BASE_ADDRESS_MEM_TYPE_32 |
			  PCI_BASE_ADDRESS_MEM_PREFETCH;

	bar.size = CONFIG_SPL_PCI_DFU_BAR_SIZE;

	ret = pci_ep_set_bar(dev, fn, &bar);
	ଯଦି (ret) {
		pr_err("Failed to set bar: %d\n", ret);
		ଫେରନ୍ତୁ ret;
	}

	ret = pci_ep_start(dev);
	ଯଦି (ret) {
		pr_err("Failed to start ep: %d\n", ret);
		ଫେରନ୍ତୁ ret;
	}

	addr = (ଶୂନ୍ୟ *)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	offset = CONFIG_SPL_PCI_DFU_BAR_SIZE - MAGIC_WORD_SIZE;

	ଯଦି (ଆକାର(CONFIG_SPL_PCI_DFU_BOOT_PHASE) > BOOT_PHASE_STRING_SIZE) {
		pr_err("Not copying boot phase. String too long\n");
	} ଅନ୍ୟଥା {
		bootphase = (ଅକ୍ଷର *)(addr + CONFIG_SPL_PCI_DFU_BAR_SIZE -
				  (BOOT_PHASE_STRING_SIZE + MAGIC_WORD_SIZE + 1));
		strlcpy(bootphase, CONFIG_SPL_PCI_DFU_BOOT_PHASE,
			ଆକାର(CONFIG_SPL_PCI_DFU_BOOT_PHASE) + 1);
	}

	addr = addr + offset;
	magic_word = CONFIG_SPL_PCI_DFU_MAGIC_WORD;
	(*(ପୂର୍ଣ୍ଣସଂଖ୍ୟା *)addr) = 0;
	flush_dcache_all();
	ପାଇଁ (;;) {
		ଯଦି (*(ପୂର୍ଣ୍ଣସଂଖ୍ୟା *)addr == magic_word)
			ଭାଙ୍ଗ;
		invalidate_dcache_all();
	}

	ଫେରନ୍ତୁ 0;
}
#endif

ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_dfu_cmd(ପୂର୍ଣ୍ଣସଂଖ୍ୟା usbctrl, ଅକ୍ଷର *dfu_alt_info, ଅକ୍ଷର *interface, ଅକ୍ଷର *devstr)
{
	ଅକ୍ଷର *str_env;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

#ifdef CONFIG_SPL_PCI_DFU
	ଯଦି (spl_boot_device() == BOOT_DEVICE_PCIE)
		ଫେରନ୍ତୁ dfu_over_pcie();
#endif

	/* set default environment */
	env_set_default(ଖାଲି, 0);
	str_env = env_get(dfu_alt_info);
	ଯଦି (!str_env) {
		pr_err("\"%s\" env variable not defined!\n", dfu_alt_info);
		ଫେରନ୍ତୁ -EINVAL;
	}

	ret = env_set("dfu_alt_info", str_env);
	ଯଦି (ret) {
		pr_err("unable to set env variable \"dfu_alt_info\"!\n");
		ଫେରନ୍ତୁ -EINVAL;
	}

	/* invoke dfu command */
	ଫେରନ୍ତୁ run_dfu(usbctrl, interface, devstr);
}
