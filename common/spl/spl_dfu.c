// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Texas Instruments, <www.ti.com>
 *
 * Ravi B <ravibabu@ti.com>
 */
#அந்தர்பாவ <env.h>
#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <linux/compiler.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <watchdog.h>
#அந்தர்பாவ <console.h>
#அந்தர்பாவ <g_dnl.h>
#அந்தர்பாவ <usb.h>
#அந்தர்பாவ <dfu.h>
#அந்தர்பாவ <linux/printk.h>
#அந்தர்பாவ <pci_ep.h>
#அந்தர்பாவ <dm/uclass.h>
#அந்தர்பாவ <cpu_func.h>
#அந்தர்பாவ <linux/io.h>

/*
 * Macros define size of magic word and boot phase string
 * in bytes.
 */
#define MAGIC_WORD_SIZE 4
#define BOOT_PHASE_STRING_SIZE 63

நிலையான முழு run_dfu(முழு usb_index, எழுத்து *interface, எழுத்து *devstring)
{
	முழு ret;

	ret = dfu_init_env_entities(interface, devstring);
	எனில் (ret) {
		dfu_free_entities();
		செல் exit;
	}

	run_usb_dnl_gadget(usb_index, "usb_dnl_dfu");
exit:
	dfu_free_entities();
	திரும்பு ret;
}

#ifdef CONFIG_SPL_PCI_DFU
நிலையான முழு dfu_over_pcie(வெறுமை)
{
	u32 offset, magic_word;
	நிலையற்ற வெறுமை *addr;
	கட்டமைப்பு udevice *dev;
	கட்டமைப்பு pci_bar bar;
	கட்டமைப்பு pci_ep_header hdr;
	uint fn = 0;
	முழு ret;
	எழுத்து *bootphase;

	uclass_get_device_by_seq(UCLASS_PCI_EP, 0, &dev);
	எனில் (!dev) {
		pr_err("Failed to get pci ep device\n");
		திரும்பு -ENODEV;
	}

	hdr.deviceid = CONFIG_SPL_PCI_DFU_DEVICE_ID;
	hdr.vendorid = CONFIG_SPL_PCI_DFU_VENDOR_ID;
	hdr.baseclass_code = PCI_BASE_CLASS_MEMORY;
	hdr.subclass_code = PCI_CLASS_MEMORY_RAM & 0xff;

	ret = pci_ep_write_header(dev, fn, &hdr);
	எனில் (ret) {
		pr_err("Failed to write header: %d\n", ret);
		திரும்பு ret;
	}

	bar.barno = BAR_0;
	bar.phys_addr = (dma_addr_t)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	bar.flags = PCI_BASE_ADDRESS_SPACE_MEMORY |
			  PCI_BASE_ADDRESS_MEM_TYPE_32 |
			  PCI_BASE_ADDRESS_MEM_PREFETCH;

	bar.size = CONFIG_SPL_PCI_DFU_BAR_SIZE;

	ret = pci_ep_set_bar(dev, fn, &bar);
	எனில் (ret) {
		pr_err("Failed to set bar: %d\n", ret);
		திரும்பு ret;
	}

	ret = pci_ep_start(dev);
	எனில் (ret) {
		pr_err("Failed to start ep: %d\n", ret);
		திரும்பு ret;
	}

	addr = (வெறுமை *)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	offset = CONFIG_SPL_PCI_DFU_BAR_SIZE - MAGIC_WORD_SIZE;

	எனில் (அளவு(CONFIG_SPL_PCI_DFU_BOOT_PHASE) > BOOT_PHASE_STRING_SIZE) {
		pr_err("Not copying boot phase. String too long\n");
	} இல்லையெனில் {
		bootphase = (எழுத்து *)(addr + CONFIG_SPL_PCI_DFU_BAR_SIZE -
				  (BOOT_PHASE_STRING_SIZE + MAGIC_WORD_SIZE + 1));
		strlcpy(bootphase, CONFIG_SPL_PCI_DFU_BOOT_PHASE,
			அளவு(CONFIG_SPL_PCI_DFU_BOOT_PHASE) + 1);
	}

	addr = addr + offset;
	magic_word = CONFIG_SPL_PCI_DFU_MAGIC_WORD;
	(*(முழு *)addr) = 0;
	flush_dcache_all();
	க்கு (;;) {
		எனில் (*(முழு *)addr == magic_word)
			நிறுத்து;
		invalidate_dcache_all();
	}

	திரும்பு 0;
}
#endif

முழு spl_dfu_cmd(முழு usbctrl, எழுத்து *dfu_alt_info, எழுத்து *interface, எழுத்து *devstr)
{
	எழுத்து *str_env;
	முழு ret;

#ifdef CONFIG_SPL_PCI_DFU
	எனில் (spl_boot_device() == BOOT_DEVICE_PCIE)
		திரும்பு dfu_over_pcie();
#endif

	/* set default environment */
	env_set_default(வெற்று, 0);
	str_env = env_get(dfu_alt_info);
	எனில் (!str_env) {
		pr_err("\"%s\" env variable not defined!\n", dfu_alt_info);
		திரும்பு -EINVAL;
	}

	ret = env_set("dfu_alt_info", str_env);
	எனில் (ret) {
		pr_err("unable to set env variable \"dfu_alt_info\"!\n");
		திரும்பு -EINVAL;
	}

	/* invoke dfu command */
	திரும்பு run_dfu(usbctrl, interface, devstr);
}
