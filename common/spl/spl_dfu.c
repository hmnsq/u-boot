// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Texas Instruments, <www.ti.com>
 *
 * Ravi B <ravibabu@ti.com>
 */
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <linux/compiler.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <watchdog.h>
#ಅನ್ತರ್ಭಾವ <console.h>
#ಅನ್ತರ್ಭಾವ <g_dnl.h>
#ಅನ್ತರ್ಭಾವ <usb.h>
#ಅನ್ತರ್ಭಾವ <dfu.h>
#ಅನ್ತರ್ಭಾವ <linux/printk.h>
#ಅನ್ತರ್ಭಾವ <pci_ep.h>
#ಅನ್ತರ್ಭಾವ <dm/uclass.h>
#ಅನ್ತರ್ಭಾವ <cpu_func.h>
#ಅನ್ತರ್ಭಾವ <linux/io.h>

/*
 * Macros define size of magic word and boot phase string
 * in bytes.
 */
#define MAGIC_WORD_SIZE 4
#define BOOT_PHASE_STRING_SIZE 63

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ run_dfu(ಪೂರ್ಣಾಙ್ಕ usb_index, ವರ್ಣ *interface, ವರ್ಣ *devstring)
{
	ಪೂರ್ಣಾಙ್ಕ ret;

	ret = dfu_init_env_entities(interface, devstring);
	ಯದಿ (ret) {
		dfu_free_entities();
		ಗಚ್ಛ exit;
	}

	run_usb_dnl_gadget(usb_index, "usb_dnl_dfu");
exit:
	dfu_free_entities();
	ಪ್ರತ್ಯಾಗಮ ret;
}

#ifdef CONFIG_SPL_PCI_DFU
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ dfu_over_pcie(ಶೂನ್ಯ)
{
	u32 offset, magic_word;
	ಚಞ್ಚಲ ಶೂನ್ಯ *addr;
	ಸಂರಚನಾ udevice *dev;
	ಸಂರಚನಾ pci_bar bar;
	ಸಂರಚನಾ pci_ep_header hdr;
	uint fn = 0;
	ಪೂರ್ಣಾಙ್ಕ ret;
	ವರ್ಣ *bootphase;

	uclass_get_device_by_seq(UCLASS_PCI_EP, 0, &dev);
	ಯದಿ (!dev) {
		pr_err("Failed to get pci ep device\n");
		ಪ್ರತ್ಯಾಗಮ -ENODEV;
	}

	hdr.deviceid = CONFIG_SPL_PCI_DFU_DEVICE_ID;
	hdr.vendorid = CONFIG_SPL_PCI_DFU_VENDOR_ID;
	hdr.baseclass_code = PCI_BASE_CLASS_MEMORY;
	hdr.subclass_code = PCI_CLASS_MEMORY_RAM & 0xff;

	ret = pci_ep_write_header(dev, fn, &hdr);
	ಯದಿ (ret) {
		pr_err("Failed to write header: %d\n", ret);
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	bar.barno = BAR_0;
	bar.phys_addr = (dma_addr_t)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	bar.flags = PCI_BASE_ADDRESS_SPACE_MEMORY |
			  PCI_BASE_ADDRESS_MEM_TYPE_32 |
			  PCI_BASE_ADDRESS_MEM_PREFETCH;

	bar.size = CONFIG_SPL_PCI_DFU_BAR_SIZE;

	ret = pci_ep_set_bar(dev, fn, &bar);
	ಯದಿ (ret) {
		pr_err("Failed to set bar: %d\n", ret);
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	ret = pci_ep_start(dev);
	ಯದಿ (ret) {
		pr_err("Failed to start ep: %d\n", ret);
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	addr = (ಶೂನ್ಯ *)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	offset = CONFIG_SPL_PCI_DFU_BAR_SIZE - MAGIC_WORD_SIZE;

	ಯದಿ (ಪರಿಮಾಣ(CONFIG_SPL_PCI_DFU_BOOT_PHASE) > BOOT_PHASE_STRING_SIZE) {
		pr_err("Not copying boot phase. String too long\n");
	} ಅನ್ಯಥಾ {
		bootphase = (ವರ್ಣ *)(addr + CONFIG_SPL_PCI_DFU_BAR_SIZE -
				  (BOOT_PHASE_STRING_SIZE + MAGIC_WORD_SIZE + 1));
		strlcpy(bootphase, CONFIG_SPL_PCI_DFU_BOOT_PHASE,
			ಪರಿಮಾಣ(CONFIG_SPL_PCI_DFU_BOOT_PHASE) + 1);
	}

	addr = addr + offset;
	magic_word = CONFIG_SPL_PCI_DFU_MAGIC_WORD;
	(*(ಪೂರ್ಣಾಙ್ಕ *)addr) = 0;
	flush_dcache_all();
	ಪ್ರತಿ (;;) {
		ಯದಿ (*(ಪೂರ್ಣಾಙ್ಕ *)addr == magic_word)
			ಭಙ್ಗ;
		invalidate_dcache_all();
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif

ಪೂರ್ಣಾಙ್ಕ spl_dfu_cmd(ಪೂರ್ಣಾಙ್ಕ usbctrl, ವರ್ಣ *dfu_alt_info, ವರ್ಣ *interface, ವರ್ಣ *devstr)
{
	ವರ್ಣ *str_env;
	ಪೂರ್ಣಾಙ್ಕ ret;

#ifdef CONFIG_SPL_PCI_DFU
	ಯದಿ (spl_boot_device() == BOOT_DEVICE_PCIE)
		ಪ್ರತ್ಯಾಗಮ dfu_over_pcie();
#endif

	/* set default environment */
	env_set_default(ರಿಕ್ತ, 0);
	str_env = env_get(dfu_alt_info);
	ಯದಿ (!str_env) {
		pr_err("\"%s\" env variable not defined!\n", dfu_alt_info);
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	ret = env_set("dfu_alt_info", str_env);
	ಯದಿ (ret) {
		pr_err("unable to set env variable \"dfu_alt_info\"!\n");
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	/* invoke dfu command */
	ಪ್ರತ್ಯಾಗಮ run_dfu(usbctrl, interface, devstr);
}
