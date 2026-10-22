// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Texas Instruments, <www.ti.com>
 *
 * Ravi B <ravibabu@ti.com>
 */
#समावेश <env.h>
#समावेश <spl.h>
#समावेश <linux/compiler.h>
#समावेश <errno.h>
#समावेश <watchdog.h>
#समावेश <console.h>
#समावेश <g_dnl.h>
#समावेश <usb.h>
#समावेश <dfu.h>
#समावेश <linux/printk.h>
#समावेश <pci_ep.h>
#समावेश <dm/uclass.h>
#समावेश <cpu_func.h>
#समावेश <linux/io.h>

/*
 * Macros define size of magic word and boot phase string
 * in bytes.
 */
#घोषणा MAGIC_WORD_आकार 4
#घोषणा BOOT_PHASE_STRING_आकार 63

अटल पूर्णांक run_dfu(पूर्णांक usb_सूचकांक, अक्षर *interface, अक्षर *devstring)
{
	पूर्णांक ret;

	ret = dfu_init_env_entities(interface, devstring);
	अगर (ret) {
		dfu_free_entities();
		जाओ निकास;
	}

	run_usb_dnl_gadget(usb_सूचकांक, "usb_dnl_dfu");
निकास:
	dfu_free_entities();
	वापस ret;
}

#अगर_घोषित CONFIG_SPL_PCI_DFU
अटल पूर्णांक dfu_over_pcie(व्योम)
{
	u32 offset, magic_word;
	अस्थिर व्योम *addr;
	काष्ठा udevice *dev;
	काष्ठा pci_bar bar;
	काष्ठा pci_ep_header hdr;
	uint fn = 0;
	पूर्णांक ret;
	अक्षर *bootphase;

	uclass_get_device_by_seq(UCLASS_PCI_EP, 0, &dev);
	अगर (!dev) {
		pr_err("Failed to get pci ep device\n");
		वापस -ENODEV;
	}

	hdr.deviceid = CONFIG_SPL_PCI_DFU_DEVICE_ID;
	hdr.vendorid = CONFIG_SPL_PCI_DFU_VENDOR_ID;
	hdr.baseclass_code = PCI_BASE_CLASS_MEMORY;
	hdr.subclass_code = PCI_CLASS_MEMORY_RAM & 0xff;

	ret = pci_ep_write_header(dev, fn, &hdr);
	अगर (ret) {
		pr_err("Failed to write header: %d\n", ret);
		वापस ret;
	}

	bar.barno = BAR_0;
	bar.phys_addr = (dma_addr_t)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	bar.flags = PCI_BASE_ADDRESS_SPACE_MEMORY |
			  PCI_BASE_ADDRESS_MEM_TYPE_32 |
			  PCI_BASE_ADDRESS_MEM_PREFETCH;

	bar.आकार = CONFIG_SPL_PCI_DFU_BAR_आकार;

	ret = pci_ep_set_bar(dev, fn, &bar);
	अगर (ret) {
		pr_err("Failed to set bar: %d\n", ret);
		वापस ret;
	}

	ret = pci_ep_start(dev);
	अगर (ret) {
		pr_err("Failed to start ep: %d\n", ret);
		वापस ret;
	}

	addr = (व्योम *)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	offset = CONFIG_SPL_PCI_DFU_BAR_आकार - MAGIC_WORD_आकार;

	अगर (माप(CONFIG_SPL_PCI_DFU_BOOT_PHASE) > BOOT_PHASE_STRING_आकार) {
		pr_err("Not copying boot phase. String too long\n");
	} अन्यथा {
		bootphase = (अक्षर *)(addr + CONFIG_SPL_PCI_DFU_BAR_आकार -
				  (BOOT_PHASE_STRING_आकार + MAGIC_WORD_आकार + 1));
		strlcpy(bootphase, CONFIG_SPL_PCI_DFU_BOOT_PHASE,
			माप(CONFIG_SPL_PCI_DFU_BOOT_PHASE) + 1);
	}

	addr = addr + offset;
	magic_word = CONFIG_SPL_PCI_DFU_MAGIC_WORD;
	(*(पूर्णांक *)addr) = 0;
	flush_dcache_all();
	क्रम (;;) {
		अगर (*(पूर्णांक *)addr == magic_word)
			अवरोध;
		invalidate_dcache_all();
	}

	वापस 0;
}
#पूर्ण_अगर

पूर्णांक spl_dfu_cmd(पूर्णांक usbctrl, अक्षर *dfu_alt_info, अक्षर *interface, अक्षर *devstr)
{
	अक्षर *str_env;
	पूर्णांक ret;

#अगर_घोषित CONFIG_SPL_PCI_DFU
	अगर (spl_boot_device() == BOOT_DEVICE_PCIE)
		वापस dfu_over_pcie();
#पूर्ण_अगर

	/* set default environment */
	env_set_default(शून्य, 0);
	str_env = env_get(dfu_alt_info);
	अगर (!str_env) {
		pr_err("\"%s\" env variable not defined!\n", dfu_alt_info);
		वापस -EINVAL;
	}

	ret = env_set("dfu_alt_info", str_env);
	अगर (ret) {
		pr_err("unable to set env variable \"dfu_alt_info\"!\n");
		वापस -EINVAL;
	}

	/* invoke dfu command */
	वापस run_dfu(usbctrl, interface, devstr);
}
