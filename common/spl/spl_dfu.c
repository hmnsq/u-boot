// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Texas Instruments, <www.ti.com>
 *
 * Ravi B <ravibabu@ti.com>
 */
#شامل <env.h>
#شامل <spl.h>
#شامل <linux/compiler.h>
#شامل <errno.h>
#شامل <watchdog.h>
#شامل <console.h>
#شامل <g_dnl.h>
#شامل <usb.h>
#شامل <dfu.h>
#شامل <linux/printk.h>
#شامل <pci_ep.h>
#شامل <dm/uclass.h>
#شامل <cpu_func.h>
#شامل <linux/io.h>

/*
 * Macros define size of magic word and boot phase string
 * in bytes.
 */
#define MAGIC_WORD_SIZE 4
#define BOOT_PHASE_STRING_SIZE 63

ایستا صحیح run_dfu(صحیح usb_index, نویسه *interface, نویسه *devstring)
{
	صحیح ret;

	ret = dfu_init_env_entities(interface, devstring);
	اگر (ret) {
		dfu_free_entities();
		برو exit;
	}

	run_usb_dnl_gadget(usb_index, "usb_dnl_dfu");
exit:
	dfu_free_entities();
	بازگشت ret;
}

#ifdef CONFIG_SPL_PCI_DFU
ایستا صحیح dfu_over_pcie(تهی)
{
	u32 offset, magic_word;
	فرار تهی *addr;
	ساختار udevice *dev;
	ساختار pci_bar bar;
	ساختار pci_ep_header hdr;
	uint fn = 0;
	صحیح ret;
	نویسه *bootphase;

	uclass_get_device_by_seq(UCLASS_PCI_EP, 0, &dev);
	اگر (!dev) {
		pr_err("Failed to get pci ep device\n");
		بازگشت -ENODEV;
	}

	hdr.deviceid = CONFIG_SPL_PCI_DFU_DEVICE_ID;
	hdr.vendorid = CONFIG_SPL_PCI_DFU_VENDOR_ID;
	hdr.baseclass_code = PCI_BASE_CLASS_MEMORY;
	hdr.subclass_code = PCI_CLASS_MEMORY_RAM & 0xff;

	ret = pci_ep_write_header(dev, fn, &hdr);
	اگر (ret) {
		pr_err("Failed to write header: %d\n", ret);
		بازگشت ret;
	}

	bar.barno = BAR_0;
	bar.phys_addr = (dma_addr_t)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	bar.flags = PCI_BASE_ADDRESS_SPACE_MEMORY |
			  PCI_BASE_ADDRESS_MEM_TYPE_32 |
			  PCI_BASE_ADDRESS_MEM_PREFETCH;

	bar.size = CONFIG_SPL_PCI_DFU_BAR_SIZE;

	ret = pci_ep_set_bar(dev, fn, &bar);
	اگر (ret) {
		pr_err("Failed to set bar: %d\n", ret);
		بازگشت ret;
	}

	ret = pci_ep_start(dev);
	اگر (ret) {
		pr_err("Failed to start ep: %d\n", ret);
		بازگشت ret;
	}

	addr = (تهی *)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	offset = CONFIG_SPL_PCI_DFU_BAR_SIZE - MAGIC_WORD_SIZE;

	اگر (اندازه(CONFIG_SPL_PCI_DFU_BOOT_PHASE) > BOOT_PHASE_STRING_SIZE) {
		pr_err("Not copying boot phase. String too long\n");
	} وگرنه {
		bootphase = (نویسه *)(addr + CONFIG_SPL_PCI_DFU_BAR_SIZE -
				  (BOOT_PHASE_STRING_SIZE + MAGIC_WORD_SIZE + 1));
		strlcpy(bootphase, CONFIG_SPL_PCI_DFU_BOOT_PHASE,
			اندازه(CONFIG_SPL_PCI_DFU_BOOT_PHASE) + 1);
	}

	addr = addr + offset;
	magic_word = CONFIG_SPL_PCI_DFU_MAGIC_WORD;
	(*(صحیح *)addr) = 0;
	flush_dcache_all();
	برای (;;) {
		اگر (*(صحیح *)addr == magic_word)
			بشکن;
		invalidate_dcache_all();
	}

	بازگشت 0;
}
#endif

صحیح spl_dfu_cmd(صحیح usbctrl, نویسه *dfu_alt_info, نویسه *interface, نویسه *devstr)
{
	نویسه *str_env;
	صحیح ret;

#ifdef CONFIG_SPL_PCI_DFU
	اگر (spl_boot_device() == BOOT_DEVICE_PCIE)
		بازگشت dfu_over_pcie();
#endif

	/* set default environment */
	env_set_default(پوچ, 0);
	str_env = env_get(dfu_alt_info);
	اگر (!str_env) {
		pr_err("\"%s\" env variable not defined!\n", dfu_alt_info);
		بازگشت -EINVAL;
	}

	ret = env_set("dfu_alt_info", str_env);
	اگر (ret) {
		pr_err("unable to set env variable \"dfu_alt_info\"!\n");
		بازگشت -EINVAL;
	}

	/* invoke dfu command */
	بازگشت run_dfu(usbctrl, interface, devstr);
}
