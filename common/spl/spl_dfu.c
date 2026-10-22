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
#تعریف_کرو MAGIC_لفظ_سائز 4
#تعریف_کرو BOOT_PHASE_STRING_سائز 63

ساکن صحیح تسلسل_dfu(صحیح usb_اشاریہ, حرف *interface, حرف *devstring)
{
	صحیح ret;

	ret = dfu_init_env_entities(interface, devstring);
	اگر (ret) {
		dfu_free_entities();
		جاؤ باہر_نکلو;
	}

	تسلسل_usb_dnl_gadget(usb_اشاریہ, "usb_dnl_dfu");
باہر_نکلو:
	dfu_free_entities();
	واپس ret;
}

#اگر_تعریف_شدہ CONFIG_SPL_PCI_DFU
ساکن صحیح dfu_over_pcie(خالی)
{
	u32 انحراف, magic_لفظ;
	متلون خالی *addr;
	ساخت udevice *dev;
	ساخت pci_bar bar;
	ساخت pci_ep_سرخی hdr;
	uint fn = 0;
	صحیح ret;
	حرف *bootphase;

	uclass_get_device_by_seq(UCLASS_PCI_EP, 0, &dev);
	اگر (!dev) {
		pr_err("Failed to get pci ep device\n");
		واپس -ENODEV;
	}

	hdr.deviceid = CONFIG_SPL_PCI_DFU_DEVICE_ID;
	hdr.vendorid = CONFIG_SPL_PCI_DFU_VENDOR_ID;
	hdr.baseclass_رمز = PCI_بنیاد_CLASS_حافظہ;
	hdr.subclass_رمز = PCI_CLASS_حافظہ_RAM & 0xff;

	ret = pci_ep_write_سرخی(dev, fn, &hdr);
	اگر (ret) {
		pr_err("Failed to write header: %d\n", ret);
		واپس ret;
	}

	bar.barno = BAR_0;
	bar.phys_addr = (dma_addr_t)CONFIG_SPL_PCI_DFU_SPL_بھرو_FIT_پتہ;
	bar.flags = PCI_بنیاد_پتہ_SPACE_حافظہ |
			  PCI_بنیاد_پتہ_MEM_TYPE_32 |
			  PCI_بنیاد_پتہ_MEM_PREFETCH;

	bar.سائز = CONFIG_SPL_PCI_DFU_BAR_سائز;

	ret = pci_ep_سیٹ_bar(dev, fn, &bar);
	اگر (ret) {
		pr_err("Failed to set bar: %d\n", ret);
		واپس ret;
	}

	ret = pci_ep_آغاز(dev);
	اگر (ret) {
		pr_err("Failed to start ep: %d\n", ret);
		واپس ret;
	}

	addr = (خالی *)CONFIG_SPL_PCI_DFU_SPL_بھرو_FIT_پتہ;
	انحراف = CONFIG_SPL_PCI_DFU_BAR_سائز - MAGIC_لفظ_سائز;

	اگر (حجم(CONFIG_SPL_PCI_DFU_BOOT_PHASE) > BOOT_PHASE_STRING_سائز) {
		pr_err("Not copying boot phase. String too long\n");
	} ورنہ {
		bootphase = (حرف *)(addr + CONFIG_SPL_PCI_DFU_BAR_سائز -
				  (BOOT_PHASE_STRING_سائز + MAGIC_لفظ_سائز + 1));
		strlcpy(bootphase, CONFIG_SPL_PCI_DFU_BOOT_PHASE,
			حجم(CONFIG_SPL_PCI_DFU_BOOT_PHASE) + 1);
	}

	addr = addr + انحراف;
	magic_لفظ = CONFIG_SPL_PCI_DFU_MAGIC_لفظ;
	(*(صحیح *)addr) = 0;
	flush_dcache_all();
	برائے (;;) {
		اگر (*(صحیح *)addr == magic_لفظ)
			توڑو;
		invalidate_dcache_all();
	}

	واپس 0;
}
#اگر_ختم

صحیح spl_dfu_cmd(صحیح usbctrl, حرف *dfu_alt_info, حرف *interface, حرف *devstr)
{
	حرف *str_env;
	صحیح ret;

#اگر_تعریف_شدہ CONFIG_SPL_PCI_DFU
	اگر (spl_boot_device() == BOOT_DEVICE_PCIE)
		واپس dfu_over_pcie();
#اگر_ختم

	/* set default environment */
	env_سیٹ_default(تہی, 0);
	str_env = env_get(dfu_alt_info);
	اگر (!str_env) {
		pr_err("\"%s\" env variable not defined!\n", dfu_alt_info);
		واپس -EINVAL;
	}

	ret = env_سیٹ("dfu_alt_info", str_env);
	اگر (ret) {
		pr_err("unable to set env variable \"dfu_alt_info\"!\n");
		واپس -EINVAL;
	}

	/* invoke dfu command */
	واپس تسلسل_dfu(usbctrl, interface, devstr);
}
