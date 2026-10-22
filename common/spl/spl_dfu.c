// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Texas Instruments, <www.ti.com>
 *
 * Ravi B <ravibabu@ti.com>
 */
#include <env.h>
#include <spl.h>
#include <linux/compiler.h>
#include <errno.h>
#include <watchdog.h>
#include <console.h>
#include <g_dnl.h>
#include <usb.h>
#include <dfu.h>
#include <linux/printk.h>
#include <pci_ep.h>
#include <dm/uclass.h>
#include <cpu_func.h>
#include <linux/io.h>

/*
 * Macros define size of magic word and boot phase string
 * in bytes.
 */
#define MAGIC_WORD_SIZE 4
#define BOOT_PHASE_STRING_SIZE 63

static số_nguyên run_dfu(số_nguyên usb_index, char *interface, char *devstring)
{
	số_nguyên ret;

	ret = dfu_init_env_entities(interface, devstring);
	nếu (ret) {
		dfu_free_entities();
		goto exit;
	}

	run_usb_dnl_gadget(usb_index, "usb_dnl_dfu");
exit:
	dfu_free_entities();
	trả_về ret;
}

#ifdef CONFIG_SPL_PCI_DFU
static số_nguyên dfu_over_pcie(void)
{
	u32 offset, magic_word;
	volatile void *addr;
	struct udevice *dev;
	struct pci_bar bar;
	struct pci_ep_header hdr;
	uint fn = 0;
	số_nguyên ret;
	char *bootphase;

	uclass_get_device_by_seq(UCLASS_PCI_EP, 0, &dev);
	nếu (!dev) {
		pr_err("Failed to get pci ep device\n");
		trả_về -ENODEV;
	}

	hdr.deviceid = CONFIG_SPL_PCI_DFU_DEVICE_ID;
	hdr.vendorid = CONFIG_SPL_PCI_DFU_VENDOR_ID;
	hdr.baseclass_code = PCI_BASE_CLASS_MEMORY;
	hdr.subclass_code = PCI_CLASS_MEMORY_RAM & 0xff;

	ret = pci_ep_write_header(dev, fn, &hdr);
	nếu (ret) {
		pr_err("Failed to write header: %d\n", ret);
		trả_về ret;
	}

	bar.barno = BAR_0;
	bar.phys_addr = (dma_addr_t)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	bar.flags = PCI_BASE_ADDRESS_SPACE_MEMORY |
			  PCI_BASE_ADDRESS_MEM_TYPE_32 |
			  PCI_BASE_ADDRESS_MEM_PREFETCH;

	bar.size = CONFIG_SPL_PCI_DFU_BAR_SIZE;

	ret = pci_ep_set_bar(dev, fn, &bar);
	nếu (ret) {
		pr_err("Failed to set bar: %d\n", ret);
		trả_về ret;
	}

	ret = pci_ep_start(dev);
	nếu (ret) {
		pr_err("Failed to start ep: %d\n", ret);
		trả_về ret;
	}

	addr = (void *)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	offset = CONFIG_SPL_PCI_DFU_BAR_SIZE - MAGIC_WORD_SIZE;

	nếu (sizeof(CONFIG_SPL_PCI_DFU_BOOT_PHASE) > BOOT_PHASE_STRING_SIZE) {
		pr_err("Not copying boot phase. String too long\n");
	} không_thì {
		bootphase = (char *)(addr + CONFIG_SPL_PCI_DFU_BAR_SIZE -
				  (BOOT_PHASE_STRING_SIZE + MAGIC_WORD_SIZE + 1));
		strlcpy(bootphase, CONFIG_SPL_PCI_DFU_BOOT_PHASE,
			sizeof(CONFIG_SPL_PCI_DFU_BOOT_PHASE) + 1);
	}

	addr = addr + offset;
	magic_word = CONFIG_SPL_PCI_DFU_MAGIC_WORD;
	(*(số_nguyên *)addr) = 0;
	flush_dcache_all();
	cho (;;) {
		nếu (*(số_nguyên *)addr == magic_word)
			break;
		invalidate_dcache_all();
	}

	trả_về 0;
}
#endif

số_nguyên spl_dfu_cmd(số_nguyên usbctrl, char *dfu_alt_info, char *interface, char *devstr)
{
	char *str_env;
	số_nguyên ret;

#ifdef CONFIG_SPL_PCI_DFU
	nếu (spl_boot_device() == BOOT_DEVICE_PCIE)
		trả_về dfu_over_pcie();
#endif

	/* set default environment */
	env_set_default(NULL, 0);
	str_env = env_get(dfu_alt_info);
	nếu (!str_env) {
		pr_err("\"%s\" env variable not defined!\n", dfu_alt_info);
		trả_về -EINVAL;
	}

	ret = env_set("dfu_alt_info", str_env);
	nếu (ret) {
		pr_err("unable to set env variable \"dfu_alt_info\"!\n");
		trả_về -EINVAL;
	}

	/* invoke dfu command */
	trả_về run_dfu(usbctrl, interface, devstr);
}
