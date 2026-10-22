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
#define MAGIC_WORD_SIZE 4
#define BOOT_PHASE_STRING_SIZE 63

স্থির পূর্ণাঙ্ক run_dfu(পূর্ণাঙ্ক usb_index, বর্ণ *interface, বর্ণ *devstring)
{
	পূর্ণাঙ্ক ret;

	ret = dfu_init_env_entities(interface, devstring);
	যদি (ret) {
		dfu_free_entities();
		গচ্ছ exit;
	}

	run_usb_dnl_gadget(usb_index, "usb_dnl_dfu");
exit:
	dfu_free_entities();
	প্রত্যাগম ret;
}

#ifdef CONFIG_SPL_PCI_DFU
স্থির পূর্ণাঙ্ক dfu_over_pcie(শূন্য)
{
	u32 offset, magic_word;
	চঞ্চল শূন্য *addr;
	সংরচনা udevice *dev;
	সংরচনা pci_bar bar;
	সংরচনা pci_ep_header hdr;
	uint fn = 0;
	পূর্ণাঙ্ক ret;
	বর্ণ *bootphase;

	uclass_get_device_by_seq(UCLASS_PCI_EP, 0, &dev);
	যদি (!dev) {
		pr_err("Failed to get pci ep device\n");
		প্রত্যাগম -ENODEV;
	}

	hdr.deviceid = CONFIG_SPL_PCI_DFU_DEVICE_ID;
	hdr.vendorid = CONFIG_SPL_PCI_DFU_VENDOR_ID;
	hdr.baseclass_code = PCI_BASE_CLASS_MEMORY;
	hdr.subclass_code = PCI_CLASS_MEMORY_RAM & 0xff;

	ret = pci_ep_write_header(dev, fn, &hdr);
	যদি (ret) {
		pr_err("Failed to write header: %d\n", ret);
		প্রত্যাগম ret;
	}

	bar.barno = BAR_0;
	bar.phys_addr = (dma_addr_t)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	bar.flags = PCI_BASE_ADDRESS_SPACE_MEMORY |
			  PCI_BASE_ADDRESS_MEM_TYPE_32 |
			  PCI_BASE_ADDRESS_MEM_PREFETCH;

	bar.size = CONFIG_SPL_PCI_DFU_BAR_SIZE;

	ret = pci_ep_set_bar(dev, fn, &bar);
	যদি (ret) {
		pr_err("Failed to set bar: %d\n", ret);
		প্রত্যাগম ret;
	}

	ret = pci_ep_start(dev);
	যদি (ret) {
		pr_err("Failed to start ep: %d\n", ret);
		প্রত্যাগম ret;
	}

	addr = (শূন্য *)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	offset = CONFIG_SPL_PCI_DFU_BAR_SIZE - MAGIC_WORD_SIZE;

	যদি (পরিমাণ(CONFIG_SPL_PCI_DFU_BOOT_PHASE) > BOOT_PHASE_STRING_SIZE) {
		pr_err("Not copying boot phase. String too long\n");
	} অন্যথা {
		bootphase = (বর্ণ *)(addr + CONFIG_SPL_PCI_DFU_BAR_SIZE -
				  (BOOT_PHASE_STRING_SIZE + MAGIC_WORD_SIZE + 1));
		strlcpy(bootphase, CONFIG_SPL_PCI_DFU_BOOT_PHASE,
			পরিমাণ(CONFIG_SPL_PCI_DFU_BOOT_PHASE) + 1);
	}

	addr = addr + offset;
	magic_word = CONFIG_SPL_PCI_DFU_MAGIC_WORD;
	(*(পূর্ণাঙ্ক *)addr) = 0;
	flush_dcache_all();
	প্রতি (;;) {
		যদি (*(পূর্ণাঙ্ক *)addr == magic_word)
			ভঙ্গ;
		invalidate_dcache_all();
	}

	প্রত্যাগম 0;
}
#endif

পূর্ণাঙ্ক spl_dfu_cmd(পূর্ণাঙ্ক usbctrl, বর্ণ *dfu_alt_info, বর্ণ *interface, বর্ণ *devstr)
{
	বর্ণ *str_env;
	পূর্ণাঙ্ক ret;

#ifdef CONFIG_SPL_PCI_DFU
	যদি (spl_boot_device() == BOOT_DEVICE_PCIE)
		প্রত্যাগম dfu_over_pcie();
#endif

	/* set default environment */
	env_set_default(রিক্ত, 0);
	str_env = env_get(dfu_alt_info);
	যদি (!str_env) {
		pr_err("\"%s\" env variable not defined!\n", dfu_alt_info);
		প্রত্যাগম -EINVAL;
	}

	ret = env_set("dfu_alt_info", str_env);
	যদি (ret) {
		pr_err("unable to set env variable \"dfu_alt_info\"!\n");
		প্রত্যাগম -EINVAL;
	}

	/* invoke dfu command */
	প্রত্যাগম run_dfu(usbctrl, interface, devstr);
}
