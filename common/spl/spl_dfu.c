// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Texas Instruments, <www.ti.com>
 *
 * Ravi B <ravibabu@ti.com>
 */
#అన్తర్భావ <env.h>
#అన్తర్భావ <spl.h>
#అన్తర్భావ <linux/compiler.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <watchdog.h>
#అన్తర్భావ <console.h>
#అన్తర్భావ <g_dnl.h>
#అన్తర్భావ <usb.h>
#అన్తర్భావ <dfu.h>
#అన్తర్భావ <linux/printk.h>
#అన్తర్భావ <pci_ep.h>
#అన్తర్భావ <dm/uclass.h>
#అన్తర్భావ <cpu_func.h>
#అన్తర్భావ <linux/io.h>

/*
 * Macros define size of magic word and boot phase string
 * in bytes.
 */
#నిర్వచించు MAGIC_మాట_కొలత 4
#నిర్వచించు BOOT_PHASE_STRING_కొలత 63

స్థిర పూర్ణాంకం నిరంతరం_dfu(పూర్ణాంకం usb_సూచీ, అక్షరం *interface, అక్షరం *devstring)
{
	పూర్ణాంకం ret;

	ret = dfu_init_env_entities(interface, devstring);
	ఒకవేళ (ret) {
		dfu_free_entities();
		వెళ్ళు నిష్క్రమించు;
	}

	నిరంతరం_usb_dnl_gadget(usb_సూచీ, "usb_dnl_dfu");
నిష్క్రమించు:
	dfu_free_entities();
	తిరిగి ret;
}

#నిర్వచితమైతే CONFIG_SPL_PCI_DFU
స్థిర పూర్ణాంకం dfu_over_pcie(శూన్య)
{
	u32 స్థానభ్రంశం, magic_మాట;
	అస్థిర శూన్య *addr;
	నిర్మాణం udevice *dev;
	నిర్మాణం pci_bar bar;
	నిర్మాణం pci_ep_శీర్షిక hdr;
	uint fn = 0;
	పూర్ణాంకం ret;
	అక్షరం *bootphase;

	uclass_get_device_by_seq(UCLASS_PCI_EP, 0, &dev);
	ఒకవేళ (!dev) {
		pr_err("Failed to get pci ep device\n");
		తిరిగి -ENODEV;
	}

	hdr.deviceid = CONFIG_SPL_PCI_DFU_DEVICE_ID;
	hdr.vendorid = CONFIG_SPL_PCI_DFU_VENDOR_ID;
	hdr.baseclass_సంకేతం = PCI_పునాది_CLASS_జ్ఞాపకం;
	hdr.subclass_సంకేతం = PCI_CLASS_జ్ఞాపకం_RAM & 0xff;

	ret = pci_ep_write_శీర్షిక(dev, fn, &hdr);
	ఒకవేళ (ret) {
		pr_err("Failed to write header: %d\n", ret);
		తిరిగి ret;
	}

	bar.barno = BAR_0;
	bar.phys_addr = (dma_addr_t)CONFIG_SPL_PCI_DFU_SPL_ఎక్కించు_FIT_చిరునామా;
	bar.flags = PCI_పునాది_చిరునామా_SPACE_జ్ఞాపకం |
			  PCI_పునాది_చిరునామా_MEM_TYPE_32 |
			  PCI_పునాది_చిరునామా_MEM_PREFETCH;

	bar.కొలత = CONFIG_SPL_PCI_DFU_BAR_కొలత;

	ret = pci_ep_సమితి_bar(dev, fn, &bar);
	ఒకవేళ (ret) {
		pr_err("Failed to set bar: %d\n", ret);
		తిరిగి ret;
	}

	ret = pci_ep_ప్రారంభం(dev);
	ఒకవేళ (ret) {
		pr_err("Failed to start ep: %d\n", ret);
		తిరిగి ret;
	}

	addr = (శూన్య *)CONFIG_SPL_PCI_DFU_SPL_ఎక్కించు_FIT_చిరునామా;
	స్థానభ్రంశం = CONFIG_SPL_PCI_DFU_BAR_కొలత - MAGIC_మాట_కొలత;

	ఒకవేళ (పరిమాణం(CONFIG_SPL_PCI_DFU_BOOT_PHASE) > BOOT_PHASE_STRING_కొలత) {
		pr_err("Not copying boot phase. String too long\n");
	} లేకపోతే {
		bootphase = (అక్షరం *)(addr + CONFIG_SPL_PCI_DFU_BAR_కొలత -
				  (BOOT_PHASE_STRING_కొలత + MAGIC_మాట_కొలత + 1));
		strlcpy(bootphase, CONFIG_SPL_PCI_DFU_BOOT_PHASE,
			పరిమాణం(CONFIG_SPL_PCI_DFU_BOOT_PHASE) + 1);
	}

	addr = addr + స్థానభ్రంశం;
	magic_మాట = CONFIG_SPL_PCI_DFU_MAGIC_మాట;
	(*(పూర్ణాంకం *)addr) = 0;
	flush_dcache_all();
	కొరకు (;;) {
		ఒకవేళ (*(పూర్ణాంకం *)addr == magic_మాట)
			ఆపు;
		invalidate_dcache_all();
	}

	తిరిగి 0;
}
#అయితే_ముగింపు

పూర్ణాంకం spl_dfu_cmd(పూర్ణాంకం usbctrl, అక్షరం *dfu_alt_info, అక్షరం *interface, అక్షరం *devstr)
{
	అక్షరం *str_env;
	పూర్ణాంకం ret;

#నిర్వచితమైతే CONFIG_SPL_PCI_DFU
	ఒకవేళ (spl_boot_device() == BOOT_DEVICE_PCIE)
		తిరిగి dfu_over_pcie();
#అయితే_ముగింపు

	/* set default environment */
	env_సమితి_default(ఖాళీ, 0);
	str_env = env_get(dfu_alt_info);
	ఒకవేళ (!str_env) {
		pr_err("\"%s\" env variable not defined!\n", dfu_alt_info);
		తిరిగి -EINVAL;
	}

	ret = env_సమితి("dfu_alt_info", str_env);
	ఒకవేళ (ret) {
		pr_err("unable to set env variable \"dfu_alt_info\"!\n");
		తిరిగి -EINVAL;
	}

	/* invoke dfu command */
	తిరిగి నిరంతరం_dfu(usbctrl, interface, devstr);
}
