// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2012
 * Ilya Yanok <ilya.yanok@gmail.com>
 */
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <spl_load.h>
#ಅನ್ತರ್ಭಾವ <net.h>
#ಅನ್ತರ್ಭಾವ <linux/libfdt.h>

#ಆದರೆ defined(CONFIG_SPL_ETH) || defined(CONFIG_SPL_USB_ETHER)
ಸ್ಥಿರ ulong spl_net_ತುಂಬು_read(ರಚನೆ spl_ತುಂಬು_info *ತುಂಬು, ulong sector,
			       ulong ಎಣಿಕೆ_2, ಶೂನ್ಯ *buf)
{
	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, ಎಣಿಕೆ_2, (ulong)buf);
	ಸ್ಮರಣೆ_ನಕಲು(buf, map_sysmem(image_ತುಂಬು_addr + sector, ಎಣಿಕೆ_2), ಎಣಿಕೆ_2);
	ಹಿಂತಿರುಗಿ ಎಣಿಕೆ_2;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_net_ತುಂಬು_image(ರಚನೆ spl_image_info *spl_image,
			      ರಚನೆ spl_boot_device *bootdev)
{
	ರಚನೆ spl_ತುಂಬು_info ತುಂಬು;
	ಪೂರ್ಣಾಂಕ rv;

	env_init();
	env_relocate();
	env_ಗಣ("autoload", "yes");
	rv = eth_initialize();
	ಆದರೆ (rv == 0) {
		ಬರೆ("No Ethernet devices found\n");
		ಹಿಂತಿರುಗಿ -ENODEV;
	}
	ಆದರೆ (bootdev->boot_device_name)
		env_ಗಣ("ethact", bootdev->boot_device_name);
	rv = net_loop(BOOTP);
	ಆದರೆ (rv < 0) {
		ಬರೆ("Problem booting with BOOTP\n");
		ಹಿಂತಿರುಗಿ rv;
	}

	spl_ತುಂಬು_init(&ತುಂಬು, spl_net_ತುಂಬು_read, ಖಾಲಿ, 1);
	ಹಿಂತಿರುಗಿ spl_ತುಂಬು(spl_image, bootdev, &ತುಂಬು, 0, 0);
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPL_ETH
ಪೂರ್ಣಾಂಕ spl_net_ತುಂಬು_image_cpgmac(ರಚನೆ spl_image_info *spl_image,
			      ರಚನೆ spl_boot_device *bootdev)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPL_ETH_DEVICE
	bootdev->boot_device_name = CONFIG_SPL_ETH_DEVICE;
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ spl_net_ತುಂಬು_image(spl_image, bootdev);
}
SPL_ತುಂಬು_IMAGE_ವಿಧಾನ("eth device", 0, BOOT_DEVICE_CPGMAC,
		      spl_net_ತುಂಬು_image_cpgmac);
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPL_USB_ETHER
ಪೂರ್ಣಾಂಕ spl_net_ತುಂಬು_image_usb(ರಚನೆ spl_image_info *spl_image,
			   ರಚನೆ spl_boot_device *bootdev)
{
	bootdev->boot_device_name = "usb_ether";
#ಆದರೆ CONFIG_IS_ENABLED(DM_USB_GADGET)
	usb_ether_init();
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ spl_net_ತುಂಬು_image(spl_image, bootdev);
}
SPL_ತುಂಬು_IMAGE_ವಿಧಾನ("USB eth", 0, BOOT_DEVICE_USBETH, spl_net_ತುಂಬು_image_usb);
#ಆದರೆ_ಅಂತ್ಯ
