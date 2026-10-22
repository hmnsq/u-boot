// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Texas Instruments, <www.ti.com>
 *
 * Ravi B <ravibabu@ti.com>
 */
#അന്തര്ഭാവ <env.h>
#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <linux/compiler.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <watchdog.h>
#അന്തര്ഭാവ <console.h>
#അന്തര്ഭാവ <g_dnl.h>
#അന്തര്ഭാവ <usb.h>
#അന്തര്ഭാവ <dfu.h>
#അന്തര്ഭാവ <linux/printk.h>
#അന്തര്ഭാവ <pci_ep.h>
#അന്തര്ഭാവ <dm/uclass.h>
#അന്തര്ഭാവ <cpu_func.h>
#അന്തര്ഭാവ <linux/io.h>

/*
 * Macros define size of magic word and boot phase string
 * in bytes.
 */
#define MAGIC_WORD_SIZE 4
#define BOOT_PHASE_STRING_SIZE 63

സ്ഥിതം പൂർണ്ണസംഖ്യ run_dfu(പൂർണ്ണസംഖ്യ usb_index, അക്ഷരം *interface, അക്ഷരം *devstring)
{
	പൂർണ്ണസംഖ്യ ret;

	ret = dfu_init_env_entities(interface, devstring);
	എങ്കിൽ (ret) {
		dfu_free_entities();
		പോകുക exit;
	}

	run_usb_dnl_gadget(usb_index, "usb_dnl_dfu");
exit:
	dfu_free_entities();
	തിരികെ ret;
}

#ifdef CONFIG_SPL_PCI_DFU
സ്ഥിതം പൂർണ്ണസംഖ്യ dfu_over_pcie(ശൂന്യം)
{
	u32 offset, magic_word;
	അസ്ഥിര ശൂന്യം *addr;
	ഘടന udevice *dev;
	ഘടന pci_bar bar;
	ഘടന pci_ep_header hdr;
	uint fn = 0;
	പൂർണ്ണസംഖ്യ ret;
	അക്ഷരം *bootphase;

	uclass_get_device_by_seq(UCLASS_PCI_EP, 0, &dev);
	എങ്കിൽ (!dev) {
		pr_err("Failed to get pci ep device\n");
		തിരികെ -ENODEV;
	}

	hdr.deviceid = CONFIG_SPL_PCI_DFU_DEVICE_ID;
	hdr.vendorid = CONFIG_SPL_PCI_DFU_VENDOR_ID;
	hdr.baseclass_code = PCI_BASE_CLASS_MEMORY;
	hdr.subclass_code = PCI_CLASS_MEMORY_RAM & 0xff;

	ret = pci_ep_write_header(dev, fn, &hdr);
	എങ്കിൽ (ret) {
		pr_err("Failed to write header: %d\n", ret);
		തിരികെ ret;
	}

	bar.barno = BAR_0;
	bar.phys_addr = (dma_addr_t)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	bar.flags = PCI_BASE_ADDRESS_SPACE_MEMORY |
			  PCI_BASE_ADDRESS_MEM_TYPE_32 |
			  PCI_BASE_ADDRESS_MEM_PREFETCH;

	bar.size = CONFIG_SPL_PCI_DFU_BAR_SIZE;

	ret = pci_ep_set_bar(dev, fn, &bar);
	എങ്കിൽ (ret) {
		pr_err("Failed to set bar: %d\n", ret);
		തിരികെ ret;
	}

	ret = pci_ep_start(dev);
	എങ്കിൽ (ret) {
		pr_err("Failed to start ep: %d\n", ret);
		തിരികെ ret;
	}

	addr = (ശൂന്യം *)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	offset = CONFIG_SPL_PCI_DFU_BAR_SIZE - MAGIC_WORD_SIZE;

	എങ്കിൽ (വലുപ്പം(CONFIG_SPL_PCI_DFU_BOOT_PHASE) > BOOT_PHASE_STRING_SIZE) {
		pr_err("Not copying boot phase. String too long\n");
	} അല്ലെങ്കിൽ {
		bootphase = (അക്ഷരം *)(addr + CONFIG_SPL_PCI_DFU_BAR_SIZE -
				  (BOOT_PHASE_STRING_SIZE + MAGIC_WORD_SIZE + 1));
		strlcpy(bootphase, CONFIG_SPL_PCI_DFU_BOOT_PHASE,
			വലുപ്പം(CONFIG_SPL_PCI_DFU_BOOT_PHASE) + 1);
	}

	addr = addr + offset;
	magic_word = CONFIG_SPL_PCI_DFU_MAGIC_WORD;
	(*(പൂർണ്ണസംഖ്യ *)addr) = 0;
	flush_dcache_all();
	വേണ്ടി (;;) {
		എങ്കിൽ (*(പൂർണ്ണസംഖ്യ *)addr == magic_word)
			നിർത്തുക;
		invalidate_dcache_all();
	}

	തിരികെ 0;
}
#endif

പൂർണ്ണസംഖ്യ spl_dfu_cmd(പൂർണ്ണസംഖ്യ usbctrl, അക്ഷരം *dfu_alt_info, അക്ഷരം *interface, അക്ഷരം *devstr)
{
	അക്ഷരം *str_env;
	പൂർണ്ണസംഖ്യ ret;

#ifdef CONFIG_SPL_PCI_DFU
	എങ്കിൽ (spl_boot_device() == BOOT_DEVICE_PCIE)
		തിരികെ dfu_over_pcie();
#endif

	/* set default environment */
	env_set_default(ശൂന്യ, 0);
	str_env = env_get(dfu_alt_info);
	എങ്കിൽ (!str_env) {
		pr_err("\"%s\" env variable not defined!\n", dfu_alt_info);
		തിരികെ -EINVAL;
	}

	ret = env_set("dfu_alt_info", str_env);
	എങ്കിൽ (ret) {
		pr_err("unable to set env variable \"dfu_alt_info\"!\n");
		തിരികെ -EINVAL;
	}

	/* invoke dfu command */
	തിരികെ run_dfu(usbctrl, interface, devstr);
}
