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
#define MAGIC_WORD_SIZE 4
#define BOOT_PHASE_STRING_SIZE 63

ساكن صحيح run_dfu(صحيح usb_index, حرف *interface, حرف *devstring)
{
	صحيح ret;

	ret = dfu_init_env_entities(interface, devstring);
	إذا (ret) {
		dfu_free_entities();
		اذهب exit;
	}

	run_usb_dnl_gadget(usb_index, "usb_dnl_dfu");
exit:
	dfu_free_entities();
	أرجع ret;
}

#ifdef CONFIG_SPL_PCI_DFU
ساكن صحيح dfu_over_pcie(فراغ)
{
	u32 offset, magic_word;
	متطاير فراغ *addr;
	بنية udevice *dev;
	بنية pci_bar bar;
	بنية pci_ep_header hdr;
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
	hdr.baseclass_code = PCI_BASE_CLASS_MEMORY;
	hdr.subclass_code = PCI_CLASS_MEMORY_RAM & 0xff;

	ret = pci_ep_write_header(dev, fn, &hdr);
	إذا (ret) {
		pr_err("Failed to write header: %d\n", ret);
		أرجع ret;
	}

	bar.barno = BAR_0;
	bar.phys_addr = (dma_addr_t)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	bar.flags = PCI_BASE_ADDRESS_SPACE_MEMORY |
			  PCI_BASE_ADDRESS_MEM_TYPE_32 |
			  PCI_BASE_ADDRESS_MEM_PREFETCH;

	bar.size = CONFIG_SPL_PCI_DFU_BAR_SIZE;

	ret = pci_ep_set_bar(dev, fn, &bar);
	إذا (ret) {
		pr_err("Failed to set bar: %d\n", ret);
		أرجع ret;
	}

	ret = pci_ep_start(dev);
	إذا (ret) {
		pr_err("Failed to start ep: %d\n", ret);
		أرجع ret;
	}

	addr = (فراغ *)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	offset = CONFIG_SPL_PCI_DFU_BAR_SIZE - MAGIC_WORD_SIZE;

	إذا (حجم(CONFIG_SPL_PCI_DFU_BOOT_PHASE) > BOOT_PHASE_STRING_SIZE) {
		pr_err("Not copying boot phase. String too long\n");
	} وإلا {
		bootphase = (حرف *)(addr + CONFIG_SPL_PCI_DFU_BAR_SIZE -
				  (BOOT_PHASE_STRING_SIZE + MAGIC_WORD_SIZE + 1));
		strlcpy(bootphase, CONFIG_SPL_PCI_DFU_BOOT_PHASE,
			حجم(CONFIG_SPL_PCI_DFU_BOOT_PHASE) + 1);
	}

	addr = addr + offset;
	magic_word = CONFIG_SPL_PCI_DFU_MAGIC_WORD;
	(*(صحيح *)addr) = 0;
	flush_dcache_all();
	لكل (;;) {
		إذا (*(صحيح *)addr == magic_word)
			اكسر;
		invalidate_dcache_all();
	}

	أرجع 0;
}
#endif

صحيح spl_dfu_cmd(صحيح usbctrl, حرف *dfu_alt_info, حرف *interface, حرف *devstr)
{
	حرف *str_env;
	صحيح ret;

#ifdef CONFIG_SPL_PCI_DFU
	إذا (spl_boot_device() == BOOT_DEVICE_PCIE)
		أرجع dfu_over_pcie();
#endif

	/* set default environment */
	env_set_default(عدم, 0);
	str_env = env_get(dfu_alt_info);
	إذا (!str_env) {
		pr_err("\"%s\" env variable not defined!\n", dfu_alt_info);
		أرجع -EINVAL;
	}

	ret = env_set("dfu_alt_info", str_env);
	إذا (ret) {
		pr_err("unable to set env variable \"dfu_alt_info\"!\n");
		أرجع -EINVAL;
	}

	/* invoke dfu command */
	أرجع run_dfu(usbctrl, interface, devstr);
}
