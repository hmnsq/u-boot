// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Texas Instruments, <www.ti.com>
 *
 * Ravi B <ravibabu@ti.com>
 */
#अन्तर्भाव <env.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <linux/compiler.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <watchdog.h>
#अन्तर्भाव <console.h>
#अन्तर्भाव <g_dnl.h>
#अन्तर्भाव <usb.h>
#अन्तर्भाव <dfu.h>
#अन्तर्भाव <linux/printk.h>
#अन्तर्भाव <pci_ep.h>
#अन्तर्भाव <dm/uclass.h>
#अन्तर्भाव <cpu_func.h>
#अन्तर्भाव <linux/io.h>

/*
 * Macros define size of magic word and boot phase string
 * in bytes.
 */
#परिभाषय MAGIC_WORD_आकार 4
#परिभाषय BOOT_PHASE_STRING_आकार 63

स्थिर पूर्णाङ्क सततम्_dfu(पूर्णाङ्क usb_सूचकांक, वर्ण *interface, वर्ण *devstring)
{
	पूर्णाङ्क ret;

	ret = dfu_init_env_entities(interface, devstring);
	यदि (ret) {
		dfu_free_entities();
		गच्छ निर्गच्छ;
	}

	सततम्_usb_dnl_gadget(usb_सूचकांक, "usb_dnl_dfu");
निर्गच्छ:
	dfu_free_entities();
	प्रत्यागम ret;
}

#यदि_परिभाषितम् CONFIG_SPL_PCI_DFU
स्थिर पूर्णाङ्क dfu_over_pcie(शून्य)
{
	u32 विस्थापनम्, magic_word;
	चञ्चल शून्य *addr;
	संरचना udevice *dev;
	संरचना pci_bar bar;
	संरचना pci_ep_शीर्षकम् hdr;
	uint fn = 0;
	पूर्णाङ्क ret;
	वर्ण *bootphase;

	uclass_get_device_by_seq(UCLASS_PCI_EP, 0, &dev);
	यदि (!dev) {
		pr_err("Failed to get pci ep device\n");
		प्रत्यागम -ENODEV;
	}

	hdr.deviceid = CONFIG_SPL_PCI_DFU_DEVICE_ID;
	hdr.vendorid = CONFIG_SPL_PCI_DFU_VENDOR_ID;
	hdr.baseclass_संहिता = PCI_आधारः_CLASS_स्मृतिः;
	hdr.subclass_संहिता = PCI_CLASS_स्मृतिः_RAM & 0xff;

	ret = pci_ep_write_शीर्षकम्(dev, fn, &hdr);
	यदि (ret) {
		pr_err("Failed to write header: %d\n", ret);
		प्रत्यागम ret;
	}

	bar.barno = BAR_0;
	bar.phys_addr = (dma_addr_t)CONFIG_SPL_PCI_DFU_SPL_आरोपय_FIT_स्थानाङ्कः;
	bar.flags = PCI_आधारः_स्थानाङ्कः_SPACE_स्मृतिः |
			  PCI_आधारः_स्थानाङ्कः_MEM_TYPE_32 |
			  PCI_आधारः_स्थानाङ्कः_MEM_PREFETCH;

	bar.आकार = CONFIG_SPL_PCI_DFU_BAR_आकार;

	ret = pci_ep_समुच्चयः_bar(dev, fn, &bar);
	यदि (ret) {
		pr_err("Failed to set bar: %d\n", ret);
		प्रत्यागम ret;
	}

	ret = pci_ep_start(dev);
	यदि (ret) {
		pr_err("Failed to start ep: %d\n", ret);
		प्रत्यागम ret;
	}

	addr = (शून्य *)CONFIG_SPL_PCI_DFU_SPL_आरोपय_FIT_स्थानाङ्कः;
	विस्थापनम् = CONFIG_SPL_PCI_DFU_BAR_आकार - MAGIC_WORD_आकार;

	यदि (परिमाण(CONFIG_SPL_PCI_DFU_BOOT_PHASE) > BOOT_PHASE_STRING_आकार) {
		pr_err("Not copying boot phase. String too long\n");
	} अन्यथा {
		bootphase = (वर्ण *)(addr + CONFIG_SPL_PCI_DFU_BAR_आकार -
				  (BOOT_PHASE_STRING_आकार + MAGIC_WORD_आकार + 1));
		strlcpy(bootphase, CONFIG_SPL_PCI_DFU_BOOT_PHASE,
			परिमाण(CONFIG_SPL_PCI_DFU_BOOT_PHASE) + 1);
	}

	addr = addr + विस्थापनम्;
	magic_word = CONFIG_SPL_PCI_DFU_MAGIC_WORD;
	(*(पूर्णाङ्क *)addr) = 0;
	flush_dcache_all();
	प्रति (;;) {
		यदि (*(पूर्णाङ्क *)addr == magic_word)
			भङ्ग;
		invalidate_dcache_all();
	}

	प्रत्यागम 0;
}
#यदि_अन्तः

पूर्णाङ्क spl_dfu_cmd(पूर्णाङ्क usbctrl, वर्ण *dfu_alt_info, वर्ण *interface, वर्ण *devstr)
{
	वर्ण *str_env;
	पूर्णाङ्क ret;

#यदि_परिभाषितम् CONFIG_SPL_PCI_DFU
	यदि (spl_boot_device() == BOOT_DEVICE_PCIE)
		प्रत्यागम dfu_over_pcie();
#यदि_अन्तः

	/* set default environment */
	env_समुच्चयः_default(रिक्त, 0);
	str_env = env_get(dfu_alt_info);
	यदि (!str_env) {
		pr_err("\"%s\" env variable not defined!\n", dfu_alt_info);
		प्रत्यागम -EINVAL;
	}

	ret = env_समुच्चयः("dfu_alt_info", str_env);
	यदि (ret) {
		pr_err("unable to set env variable \"dfu_alt_info\"!\n");
		प्रत्यागम -EINVAL;
	}

	/* invoke dfu command */
	प्रत्यागम सततम्_dfu(usbctrl, interface, devstr);
}
