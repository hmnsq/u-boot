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
#define MAGIC_WORD_SIZE 4
#define BOOT_PHASE_STRING_SIZE 63

स्थिर पूर्णांक run_dfu(पूर्णांक usb_index, अक्षर *interface, अक्षर *devstring)
{
	पूर्णांक ret;

	ret = dfu_init_env_entities(interface, devstring);
	जर (ret) {
		dfu_free_entities();
		जा निकास;
	}

	run_usb_dnl_gadget(usb_index, "usb_dnl_dfu");
निकास:
	dfu_free_entities();
	परत ret;
}

#ifdef CONFIG_SPL_PCI_DFU
स्थिर पूर्णांक dfu_over_pcie(शून्य)
{
	u32 offset, magic_word;
	अस्थिर शून्य *addr;
	संरचना udevice *dev;
	संरचना pci_bar bar;
	संरचना pci_ep_header hdr;
	uint fn = 0;
	पूर्णांक ret;
	अक्षर *bootphase;

	uclass_get_device_by_seq(UCLASS_PCI_EP, 0, &dev);
	जर (!dev) {
		pr_err("Failed to get pci ep device\n");
		परत -ENODEV;
	}

	hdr.deviceid = CONFIG_SPL_PCI_DFU_DEVICE_ID;
	hdr.vendorid = CONFIG_SPL_PCI_DFU_VENDOR_ID;
	hdr.baseclass_code = PCI_BASE_CLASS_MEMORY;
	hdr.subclass_code = PCI_CLASS_MEMORY_RAM & 0xff;

	ret = pci_ep_write_header(dev, fn, &hdr);
	जर (ret) {
		pr_err("Failed to write header: %d\n", ret);
		परत ret;
	}

	bar.barno = BAR_0;
	bar.phys_addr = (dma_addr_t)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	bar.flags = PCI_BASE_ADDRESS_SPACE_MEMORY |
			  PCI_BASE_ADDRESS_MEM_TYPE_32 |
			  PCI_BASE_ADDRESS_MEM_PREFETCH;

	bar.size = CONFIG_SPL_PCI_DFU_BAR_SIZE;

	ret = pci_ep_set_bar(dev, fn, &bar);
	जर (ret) {
		pr_err("Failed to set bar: %d\n", ret);
		परत ret;
	}

	ret = pci_ep_start(dev);
	जर (ret) {
		pr_err("Failed to start ep: %d\n", ret);
		परत ret;
	}

	addr = (शून्य *)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	offset = CONFIG_SPL_PCI_DFU_BAR_SIZE - MAGIC_WORD_SIZE;

	जर (आकार(CONFIG_SPL_PCI_DFU_BOOT_PHASE) > BOOT_PHASE_STRING_SIZE) {
		pr_err("Not copying boot phase. String too long\n");
	} अन्यथा {
		bootphase = (अक्षर *)(addr + CONFIG_SPL_PCI_DFU_BAR_SIZE -
				  (BOOT_PHASE_STRING_SIZE + MAGIC_WORD_SIZE + 1));
		strlcpy(bootphase, CONFIG_SPL_PCI_DFU_BOOT_PHASE,
			आकार(CONFIG_SPL_PCI_DFU_BOOT_PHASE) + 1);
	}

	addr = addr + offset;
	magic_word = CONFIG_SPL_PCI_DFU_MAGIC_WORD;
	(*(पूर्णांक *)addr) = 0;
	flush_dcache_all();
	साठी (;;) {
		जर (*(पूर्णांक *)addr == magic_word)
			थांबा;
		invalidate_dcache_all();
	}

	परत 0;
}
#endif

पूर्णांक spl_dfu_cmd(पूर्णांक usbctrl, अक्षर *dfu_alt_info, अक्षर *interface, अक्षर *devstr)
{
	अक्षर *str_env;
	पूर्णांक ret;

#ifdef CONFIG_SPL_PCI_DFU
	जर (spl_boot_device() == BOOT_DEVICE_PCIE)
		परत dfu_over_pcie();
#endif

	/* set default environment */
	env_set_default(रिक्त, 0);
	str_env = env_get(dfu_alt_info);
	जर (!str_env) {
		pr_err("\"%s\" env variable not defined!\n", dfu_alt_info);
		परत -EINVAL;
	}

	ret = env_set("dfu_alt_info", str_env);
	जर (ret) {
		pr_err("unable to set env variable \"dfu_alt_info\"!\n");
		परत -EINVAL;
	}

	/* invoke dfu command */
	परत run_dfu(usbctrl, interface, devstr);
}
