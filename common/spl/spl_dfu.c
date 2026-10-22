// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Texas Instruments, <www.ti.com>
 *
 * Ravi B <ravibabu@ti.com>
 */
#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <linux/compiler.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <watchdog.h>
#અન્તર્ભાવ <console.h>
#અન્તર્ભાવ <g_dnl.h>
#અન્તર્ભાવ <usb.h>
#અન્તર્ભાવ <dfu.h>
#અન્તર્ભાવ <linux/printk.h>
#અન્તર્ભાવ <pci_ep.h>
#અન્તર્ભાવ <dm/uclass.h>
#અન્તર્ભાવ <cpu_func.h>
#અન્તર્ભાવ <linux/io.h>

/*
 * Macros define size of magic word and boot phase string
 * in bytes.
 */
#define MAGIC_WORD_SIZE 4
#define BOOT_PHASE_STRING_SIZE 63

સ્થિર પૂર્ણાંક run_dfu(પૂર્ણાંક usb_index, અક્ષર *interface, અક્ષર *devstring)
{
	પૂર્ણાંક ret;

	ret = dfu_init_env_entities(interface, devstring);
	જો (ret) {
		dfu_free_entities();
		જાઓ exit;
	}

	run_usb_dnl_gadget(usb_index, "usb_dnl_dfu");
exit:
	dfu_free_entities();
	પાછા ret;
}

#ifdef CONFIG_SPL_PCI_DFU
સ્થિર પૂર્ણાંક dfu_over_pcie(શૂન્ય)
{
	u32 offset, magic_word;
	અસ્થિર શૂન્ય *addr;
	સંરચના udevice *dev;
	સંરચના pci_bar bar;
	સંરચના pci_ep_header hdr;
	uint fn = 0;
	પૂર્ણાંક ret;
	અક્ષર *bootphase;

	uclass_get_device_by_seq(UCLASS_PCI_EP, 0, &dev);
	જો (!dev) {
		pr_err("Failed to get pci ep device\n");
		પાછા -ENODEV;
	}

	hdr.deviceid = CONFIG_SPL_PCI_DFU_DEVICE_ID;
	hdr.vendorid = CONFIG_SPL_PCI_DFU_VENDOR_ID;
	hdr.baseclass_code = PCI_BASE_CLASS_MEMORY;
	hdr.subclass_code = PCI_CLASS_MEMORY_RAM & 0xff;

	ret = pci_ep_write_header(dev, fn, &hdr);
	જો (ret) {
		pr_err("Failed to write header: %d\n", ret);
		પાછા ret;
	}

	bar.barno = BAR_0;
	bar.phys_addr = (dma_addr_t)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	bar.flags = PCI_BASE_ADDRESS_SPACE_MEMORY |
			  PCI_BASE_ADDRESS_MEM_TYPE_32 |
			  PCI_BASE_ADDRESS_MEM_PREFETCH;

	bar.size = CONFIG_SPL_PCI_DFU_BAR_SIZE;

	ret = pci_ep_set_bar(dev, fn, &bar);
	જો (ret) {
		pr_err("Failed to set bar: %d\n", ret);
		પાછા ret;
	}

	ret = pci_ep_start(dev);
	જો (ret) {
		pr_err("Failed to start ep: %d\n", ret);
		પાછા ret;
	}

	addr = (શૂન્ય *)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	offset = CONFIG_SPL_PCI_DFU_BAR_SIZE - MAGIC_WORD_SIZE;

	જો (કદ(CONFIG_SPL_PCI_DFU_BOOT_PHASE) > BOOT_PHASE_STRING_SIZE) {
		pr_err("Not copying boot phase. String too long\n");
	} નહીંતર {
		bootphase = (અક્ષર *)(addr + CONFIG_SPL_PCI_DFU_BAR_SIZE -
				  (BOOT_PHASE_STRING_SIZE + MAGIC_WORD_SIZE + 1));
		strlcpy(bootphase, CONFIG_SPL_PCI_DFU_BOOT_PHASE,
			કદ(CONFIG_SPL_PCI_DFU_BOOT_PHASE) + 1);
	}

	addr = addr + offset;
	magic_word = CONFIG_SPL_PCI_DFU_MAGIC_WORD;
	(*(પૂર્ણાંક *)addr) = 0;
	flush_dcache_all();
	માટે (;;) {
		જો (*(પૂર્ણાંક *)addr == magic_word)
			તોડો;
		invalidate_dcache_all();
	}

	પાછા 0;
}
#endif

પૂર્ણાંક spl_dfu_cmd(પૂર્ણાંક usbctrl, અક્ષર *dfu_alt_info, અક્ષર *interface, અક્ષર *devstr)
{
	અક્ષર *str_env;
	પૂર્ણાંક ret;

#ifdef CONFIG_SPL_PCI_DFU
	જો (spl_boot_device() == BOOT_DEVICE_PCIE)
		પાછા dfu_over_pcie();
#endif

	/* set default environment */
	env_set_default(ખાલી, 0);
	str_env = env_get(dfu_alt_info);
	જો (!str_env) {
		pr_err("\"%s\" env variable not defined!\n", dfu_alt_info);
		પાછા -EINVAL;
	}

	ret = env_set("dfu_alt_info", str_env);
	જો (ret) {
		pr_err("unable to set env variable \"dfu_alt_info\"!\n");
		પાછા -EINVAL;
	}

	/* invoke dfu command */
	પાછા run_dfu(usbctrl, interface, devstr);
}
