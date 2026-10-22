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

static ကိန်းပြည့် run_dfu(ကိန်းပြည့် usb_index, var_na *interface, var_na *devstring)
{
	ကိန်းပြည့် ret;

	ret = dfu_init_env_entities(interface, devstring);
	အကယ်၍ (ret) {
		dfu_free_entities();
		goto exit;
	}

	run_usb_dnl_gadget(usb_index, "usb_dnl_dfu");
exit:
	dfu_free_entities();
	ပြန်ပေး ret;
}

#ifdef CONFIG_SPL_PCI_DFU
static ကိန်းပြည့် dfu_over_pcie(void)
{
	u32 offset, magic_word;
	ca_ncala void *addr;
	sa_nracanaa udevice *dev;
	sa_nracanaa pci_bar bar;
	sa_nracanaa pci_ep_header hdr;
	uint fn = 0;
	ကိန်းပြည့် ret;
	var_na *bootphase;

	uclass_get_device_by_seq(UCLASS_PCI_EP, 0, &dev);
	အကယ်၍ (!dev) {
		pr_err("Failed to get pci ep device\n");
		ပြန်ပေး -ENODEV;
	}

	hdr.deviceid = CONFIG_SPL_PCI_DFU_DEVICE_ID;
	hdr.vendorid = CONFIG_SPL_PCI_DFU_VENDOR_ID;
	hdr.baseclass_code = PCI_BASE_CLASS_MEMORY;
	hdr.subclass_code = PCI_CLASS_MEMORY_RAM & 0xff;

	ret = pci_ep_write_header(dev, fn, &hdr);
	အကယ်၍ (ret) {
		pr_err("Failed to write header: %d\n", ret);
		ပြန်ပေး ret;
	}

	bar.barno = BAR_0;
	bar.phys_addr = (dma_addr_t)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	bar.flags = PCI_BASE_ADDRESS_SPACE_MEMORY |
			  PCI_BASE_ADDRESS_MEM_TYPE_32 |
			  PCI_BASE_ADDRESS_MEM_PREFETCH;

	bar.size = CONFIG_SPL_PCI_DFU_BAR_SIZE;

	ret = pci_ep_set_bar(dev, fn, &bar);
	အကယ်၍ (ret) {
		pr_err("Failed to set bar: %d\n", ret);
		ပြန်ပေး ret;
	}

	ret = pci_ep_start(dev);
	အကယ်၍ (ret) {
		pr_err("Failed to start ep: %d\n", ret);
		ပြန်ပေး ret;
	}

	addr = (void *)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	offset = CONFIG_SPL_PCI_DFU_BAR_SIZE - MAGIC_WORD_SIZE;

	အကယ်၍ (parimaa_na(CONFIG_SPL_PCI_DFU_BOOT_PHASE) > BOOT_PHASE_STRING_SIZE) {
		pr_err("Not copying boot phase. String too long\n");
	} မဟုတ်ပါက {
		bootphase = (var_na *)(addr + CONFIG_SPL_PCI_DFU_BAR_SIZE -
				  (BOOT_PHASE_STRING_SIZE + MAGIC_WORD_SIZE + 1));
		strlcpy(bootphase, CONFIG_SPL_PCI_DFU_BOOT_PHASE,
			parimaa_na(CONFIG_SPL_PCI_DFU_BOOT_PHASE) + 1);
	}

	addr = addr + offset;
	magic_word = CONFIG_SPL_PCI_DFU_MAGIC_WORD;
	(*(ကိန်းပြည့် *)addr) = 0;
	flush_dcache_all();
	အတွက် (;;) {
		အကယ်၍ (*(ကိန်းပြည့် *)addr == magic_word)
			bha_nga;
		invalidate_dcache_all();
	}

	ပြန်ပေး 0;
}
#endif

ကိန်းပြည့် spl_dfu_cmd(ကိန်းပြည့် usbctrl, var_na *dfu_alt_info, var_na *interface, var_na *devstr)
{
	var_na *str_env;
	ကိန်းပြည့် ret;

#ifdef CONFIG_SPL_PCI_DFU
	အကယ်၍ (spl_boot_device() == BOOT_DEVICE_PCIE)
		ပြန်ပေး dfu_over_pcie();
#endif

	/* set default environment */
	env_set_default(NULL, 0);
	str_env = env_get(dfu_alt_info);
	အကယ်၍ (!str_env) {
		pr_err("\"%s\" env variable not defined!\n", dfu_alt_info);
		ပြန်ပေး -EINVAL;
	}

	ret = env_set("dfu_alt_info", str_env);
	အကယ်၍ (ret) {
		pr_err("unable to set env variable \"dfu_alt_info\"!\n");
		ပြန်ပေး -EINVAL;
	}

	/* invoke dfu command */
	ပြန်ပေး run_dfu(usbctrl, interface, devstr);
}
