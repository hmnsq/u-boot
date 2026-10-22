// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Texas Instruments, <www.ti.com>
 *
 * Ravi B <ravibabu@ti.com>
 */
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <linux/compiler.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <watchdog.h>
#ಅನ್ತರ್ಭಾವ <console.h>
#ಅನ್ತರ್ಭಾವ <g_dnl.h>
#ಅನ್ತರ್ಭಾವ <usb.h>
#ಅನ್ತರ್ಭಾವ <dfu.h>
#ಅನ್ತರ್ಭಾವ <linux/printk.h>
#ಅನ್ತರ್ಭಾವ <pci_ep.h>
#ಅನ್ತರ್ಭಾವ <dm/uclass.h>
#ಅನ್ತರ್ಭಾವ <cpu_func.h>
#ಅನ್ತರ್ಭಾವ <linux/io.h>

/*
 * Macros define size of magic word and boot phase string
 * in bytes.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು MAGIC_ಶಬ್ದ_ಗಾತ್ರ 4
#ವ್ಯಾಖ್ಯಾನಿಸು BOOT_PHASE_STRING_ಗಾತ್ರ 63

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸತತ_dfu(ಪೂರ್ಣಾಂಕ usb_ಸೂಚ್ಯಂಕ, ಅಕ್ಷರ *interface, ಅಕ್ಷರ *devstring)
{
	ಪೂರ್ಣಾಂಕ ret;

	ret = dfu_init_env_entities(interface, devstring);
	ಆದರೆ (ret) {
		dfu_free_entities();
		ಹೋಗು ನಿರ್ಗಮಿಸು;
	}

	ಸತತ_usb_dnl_gadget(usb_ಸೂಚ್ಯಂಕ, "usb_dnl_dfu");
ನಿರ್ಗಮಿಸು:
	dfu_free_entities();
	ಹಿಂತಿರುಗಿ ret;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPL_PCI_DFU
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dfu_over_pcie(ಶೂನ್ಯ)
{
	u32 ಸ್ಥಾನಾಂತರ, magic_ಶಬ್ದ;
	ಅಸ್ಥಿರ ಶೂನ್ಯ *addr;
	ರಚನೆ udevice *dev;
	ರಚನೆ pci_bar bar;
	ರಚನೆ pci_ep_ಶೀರ್ಷಿಕೆ hdr;
	uint fn = 0;
	ಪೂರ್ಣಾಂಕ ret;
	ಅಕ್ಷರ *bootphase;

	uclass_get_device_by_seq(UCLASS_PCI_EP, 0, &dev);
	ಆದರೆ (!dev) {
		pr_err("Failed to get pci ep device\n");
		ಹಿಂತಿರುಗಿ -ENODEV;
	}

	hdr.deviceid = CONFIG_SPL_PCI_DFU_DEVICE_ID;
	hdr.vendorid = CONFIG_SPL_PCI_DFU_VENDOR_ID;
	hdr.baseclass_ಸಂಕೇತ = PCI_ತಳ_CLASS_ಸ್ಮರಣೆ;
	hdr.subclass_ಸಂಕೇತ = PCI_CLASS_ಸ್ಮರಣೆ_RAM & 0xff;

	ret = pci_ep_write_ಶೀರ್ಷಿಕೆ(dev, fn, &hdr);
	ಆದರೆ (ret) {
		pr_err("Failed to write header: %d\n", ret);
		ಹಿಂತಿರುಗಿ ret;
	}

	bar.barno = BAR_0;
	bar.phys_addr = (dma_addr_t)CONFIG_SPL_PCI_DFU_SPL_ತುಂಬು_FIT_ವಿಳಾಸ;
	bar.flags = PCI_ತಳ_ವಿಳಾಸ_SPACE_ಸ್ಮರಣೆ |
			  PCI_ತಳ_ವಿಳಾಸ_MEM_TYPE_32 |
			  PCI_ತಳ_ವಿಳಾಸ_MEM_PREFETCH;

	bar.ಗಾತ್ರ_2 = CONFIG_SPL_PCI_DFU_BAR_ಗಾತ್ರ;

	ret = pci_ep_ಗಣ_bar(dev, fn, &bar);
	ಆದರೆ (ret) {
		pr_err("Failed to set bar: %d\n", ret);
		ಹಿಂತಿರುಗಿ ret;
	}

	ret = pci_ep_ಆರಂಭ(dev);
	ಆದರೆ (ret) {
		pr_err("Failed to start ep: %d\n", ret);
		ಹಿಂತಿರುಗಿ ret;
	}

	addr = (ಶೂನ್ಯ *)CONFIG_SPL_PCI_DFU_SPL_ತುಂಬು_FIT_ವಿಳಾಸ;
	ಸ್ಥಾನಾಂತರ = CONFIG_SPL_PCI_DFU_BAR_ಗಾತ್ರ - MAGIC_ಶಬ್ದ_ಗಾತ್ರ;

	ಆದರೆ (ಗಾತ್ರ(CONFIG_SPL_PCI_DFU_BOOT_PHASE) > BOOT_PHASE_STRING_ಗಾತ್ರ) {
		pr_err("Not copying boot phase. String too long\n");
	} ಇಲ್ಲದಿದ್ದರೆ {
		bootphase = (ಅಕ್ಷರ *)(addr + CONFIG_SPL_PCI_DFU_BAR_ಗಾತ್ರ -
				  (BOOT_PHASE_STRING_ಗಾತ್ರ + MAGIC_ಶಬ್ದ_ಗಾತ್ರ + 1));
		strlcpy(bootphase, CONFIG_SPL_PCI_DFU_BOOT_PHASE,
			ಗಾತ್ರ(CONFIG_SPL_PCI_DFU_BOOT_PHASE) + 1);
	}

	addr = addr + ಸ್ಥಾನಾಂತರ;
	magic_ಶಬ್ದ = CONFIG_SPL_PCI_DFU_MAGIC_ಶಬ್ದ;
	(*(ಪೂರ್ಣಾಂಕ *)addr) = 0;
	flush_dcache_all();
	ಗಾಗಿ (;;) {
		ಆದರೆ (*(ಪೂರ್ಣಾಂಕ *)addr == magic_ಶಬ್ದ)
			ನಿಲ್ಲಿಸು;
		invalidate_dcache_all();
	}

	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ spl_dfu_cmd(ಪೂರ್ಣಾಂಕ usbctrl, ಅಕ್ಷರ *dfu_alt_info, ಅಕ್ಷರ *interface, ಅಕ್ಷರ *devstr)
{
	ಅಕ್ಷರ *str_env;
	ಪೂರ್ಣಾಂಕ ret;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPL_PCI_DFU
	ಆದರೆ (spl_boot_device() == BOOT_DEVICE_PCIE)
		ಹಿಂತಿರುಗಿ dfu_over_pcie();
#ಆದರೆ_ಅಂತ್ಯ

	/* set default environment */
	env_ಗಣ_default(ಖಾಲಿ, 0);
	str_env = env_get(dfu_alt_info);
	ಆದರೆ (!str_env) {
		pr_err("\"%s\" env variable not defined!\n", dfu_alt_info);
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	ret = env_ಗಣ("dfu_alt_info", str_env);
	ಆದರೆ (ret) {
		pr_err("unable to set env variable \"dfu_alt_info\"!\n");
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	/* invoke dfu command */
	ಹಿಂತಿರುಗಿ ಸತತ_dfu(usbctrl, interface, devstr);
}
