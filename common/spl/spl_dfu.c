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
#تعریف_کن MAGIC_کلمه_اندازه 4
#تعریف_کن BOOT_PHASE_STRING_اندازه 63

ساكن صحيح پیاپی_dfu(صحيح usb_شاخص, حرف *interface, حرف *devstring)
{
	صحيح ret;

	ret = dfu_init_env_entities(interface, devstring);
	إذا (ret) {
		dfu_free_entities();
		اذهب خروج;
	}

	پیاپی_usb_dnl_gadget(usb_شاخص, "usb_dnl_dfu");
خروج:
	dfu_free_entities();
	أرجع ret;
}

#اگر_تعریف_شده CONFIG_SPL_PCI_DFU
ساكن صحيح dfu_over_pcie(فراغ)
{
	u32 جابه_جایی, magic_کلمه;
	متطاير فراغ *addr;
	بنية udevice *dev;
	بنية pci_bar bar;
	بنية pci_ep_سرآیند hdr;
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
	hdr.baseclass_کد = PCI_پایه_CLASS_حافظه;
	hdr.subclass_کد = PCI_CLASS_حافظه_RAM & 0xff;

	ret = pci_ep_write_سرآیند(dev, fn, &hdr);
	إذا (ret) {
		pr_err("Failed to write header: %d\n", ret);
		أرجع ret;
	}

	bar.barno = BAR_0;
	bar.phys_addr = (dma_addr_t)CONFIG_SPL_PCI_DFU_SPL_بارگیری_FIT_آدرس;
	bar.flags = PCI_پایه_آدرس_SPACE_حافظه |
			  PCI_پایه_آدرس_MEM_TYPE_32 |
			  PCI_پایه_آدرس_MEM_PREFETCH;

	bar.اندازه = CONFIG_SPL_PCI_DFU_BAR_اندازه;

	ret = pci_ep_مجموعه_bar(dev, fn, &bar);
	إذا (ret) {
		pr_err("Failed to set bar: %d\n", ret);
		أرجع ret;
	}

	ret = pci_ep_شروع(dev);
	إذا (ret) {
		pr_err("Failed to start ep: %d\n", ret);
		أرجع ret;
	}

	addr = (فراغ *)CONFIG_SPL_PCI_DFU_SPL_بارگیری_FIT_آدرس;
	جابه_جایی = CONFIG_SPL_PCI_DFU_BAR_اندازه - MAGIC_کلمه_اندازه;

	إذا (حجم(CONFIG_SPL_PCI_DFU_BOOT_PHASE) > BOOT_PHASE_STRING_اندازه) {
		pr_err("Not copying boot phase. String too long\n");
	} وإلا {
		bootphase = (حرف *)(addr + CONFIG_SPL_PCI_DFU_BAR_اندازه -
				  (BOOT_PHASE_STRING_اندازه + MAGIC_کلمه_اندازه + 1));
		strlcpy(bootphase, CONFIG_SPL_PCI_DFU_BOOT_PHASE,
			حجم(CONFIG_SPL_PCI_DFU_BOOT_PHASE) + 1);
	}

	addr = addr + جابه_جایی;
	magic_کلمه = CONFIG_SPL_PCI_DFU_MAGIC_کلمه;
	(*(صحيح *)addr) = 0;
	flush_dcache_all();
	لكل (;;) {
		إذا (*(صحيح *)addr == magic_کلمه)
			اكسر;
		invalidate_dcache_all();
	}

	أرجع 0;
}
#پایان_اگر

صحيح spl_dfu_cmd(صحيح usbctrl, حرف *dfu_alt_info, حرف *interface, حرف *devstr)
{
	حرف *str_env;
	صحيح ret;

#اگر_تعریف_شده CONFIG_SPL_PCI_DFU
	إذا (spl_boot_device() == BOOT_DEVICE_PCIE)
		أرجع dfu_over_pcie();
#پایان_اگر

	/* set default environment */
	env_مجموعه_default(عدم, 0);
	str_env = env_get(dfu_alt_info);
	إذا (!str_env) {
		pr_err("\"%s\" env variable not defined!\n", dfu_alt_info);
		أرجع -EINVAL;
	}

	ret = env_مجموعه("dfu_alt_info", str_env);
	إذا (ret) {
		pr_err("unable to set env variable \"dfu_alt_info\"!\n");
		أرجع -EINVAL;
	}

	/* invoke dfu command */
	أرجع پیاپی_dfu(usbctrl, interface, devstr);
}
