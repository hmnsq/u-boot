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

static 정수 run_dfu(정수 usb_index, char *interface, char *devstring)
{
	정수 ret;

	ret = dfu_init_env_entities(interface, devstring);
	만약 (ret) {
		dfu_free_entities();
		goto exit;
	}

	run_usb_dnl_gadget(usb_index, "usb_dnl_dfu");
exit:
	dfu_free_entities();
	반환 ret;
}

#ifdef CONFIG_SPL_PCI_DFU
static 정수 dfu_over_pcie(void)
{
	u32 offset, magic_word;
	volatile void *addr;
	struct udevice *dev;
	struct pci_bar bar;
	struct pci_ep_header hdr;
	uint fn = 0;
	정수 ret;
	char *bootphase;

	uclass_get_device_by_seq(UCLASS_PCI_EP, 0, &dev);
	만약 (!dev) {
		pr_err("Failed to get pci ep device\n");
		반환 -ENODEV;
	}

	hdr.deviceid = CONFIG_SPL_PCI_DFU_DEVICE_ID;
	hdr.vendorid = CONFIG_SPL_PCI_DFU_VENDOR_ID;
	hdr.baseclass_code = PCI_BASE_CLASS_MEMORY;
	hdr.subclass_code = PCI_CLASS_MEMORY_RAM & 0xff;

	ret = pci_ep_write_header(dev, fn, &hdr);
	만약 (ret) {
		pr_err("Failed to write header: %d\n", ret);
		반환 ret;
	}

	bar.barno = BAR_0;
	bar.phys_addr = (dma_addr_t)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	bar.flags = PCI_BASE_ADDRESS_SPACE_MEMORY |
			  PCI_BASE_ADDRESS_MEM_TYPE_32 |
			  PCI_BASE_ADDRESS_MEM_PREFETCH;

	bar.size = CONFIG_SPL_PCI_DFU_BAR_SIZE;

	ret = pci_ep_set_bar(dev, fn, &bar);
	만약 (ret) {
		pr_err("Failed to set bar: %d\n", ret);
		반환 ret;
	}

	ret = pci_ep_start(dev);
	만약 (ret) {
		pr_err("Failed to start ep: %d\n", ret);
		반환 ret;
	}

	addr = (void *)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	offset = CONFIG_SPL_PCI_DFU_BAR_SIZE - MAGIC_WORD_SIZE;

	만약 (sizeof(CONFIG_SPL_PCI_DFU_BOOT_PHASE) > BOOT_PHASE_STRING_SIZE) {
		pr_err("Not copying boot phase. String too long\n");
	} 아니면 {
		bootphase = (char *)(addr + CONFIG_SPL_PCI_DFU_BAR_SIZE -
				  (BOOT_PHASE_STRING_SIZE + MAGIC_WORD_SIZE + 1));
		strlcpy(bootphase, CONFIG_SPL_PCI_DFU_BOOT_PHASE,
			sizeof(CONFIG_SPL_PCI_DFU_BOOT_PHASE) + 1);
	}

	addr = addr + offset;
	magic_word = CONFIG_SPL_PCI_DFU_MAGIC_WORD;
	(*(정수 *)addr) = 0;
	flush_dcache_all();
	위해 (;;) {
		만약 (*(정수 *)addr == magic_word)
			break;
		invalidate_dcache_all();
	}

	반환 0;
}
#endif

정수 spl_dfu_cmd(정수 usbctrl, char *dfu_alt_info, char *interface, char *devstr)
{
	char *str_env;
	정수 ret;

#ifdef CONFIG_SPL_PCI_DFU
	만약 (spl_boot_device() == BOOT_DEVICE_PCIE)
		반환 dfu_over_pcie();
#endif

	/* set default environment */
	env_set_default(NULL, 0);
	str_env = env_get(dfu_alt_info);
	만약 (!str_env) {
		pr_err("\"%s\" env variable not defined!\n", dfu_alt_info);
		반환 -EINVAL;
	}

	ret = env_set("dfu_alt_info", str_env);
	만약 (ret) {
		pr_err("unable to set env variable \"dfu_alt_info\"!\n");
		반환 -EINVAL;
	}

	/* invoke dfu command */
	반환 run_dfu(usbctrl, interface, devstr);
}
