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

#ಯದಿ defined(CONFIG_SPL_ETH) || defined(CONFIG_SPL_USB_ETHER)
ಸ್ಥಿರ ulong spl_net_load_read(ಸಂರಚನಾ spl_load_info *load, ulong sector,
			       ulong ಎಣಿಕೆ, ಶೂನ್ಯ *buf)
{
	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, ಎಣಿಕೆ, (ulong)buf);
	memcpy(buf, map_sysmem(image_load_addr + sector, ಎಣಿಕೆ), ಎಣಿಕೆ);
	ಪ್ರತ್ಯಾಗಮ ಎಣಿಕೆ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_net_load_image(ಸಂರಚನಾ spl_image_info *spl_image,
			      ಸಂರಚನಾ spl_boot_device *bootdev)
{
	ಸಂರಚನಾ spl_load_info load;
	ಪೂರ್ಣಾಙ್ಕ rv;

	env_init();
	env_relocate();
	env_set("autoload", "yes");
	rv = eth_initialize();
	ಯದಿ (rv == 0) {
		printf("No Ethernet devices found\n");
		ಪ್ರತ್ಯಾಗಮ -ENODEV;
	}
	ಯದಿ (bootdev->boot_device_name)
		env_set("ethact", bootdev->boot_device_name);
	rv = net_loop(BOOTP);
	ಯದಿ (rv < 0) {
		printf("Problem booting with BOOTP\n");
		ಪ್ರತ್ಯಾಗಮ rv;
	}

	spl_load_init(&load, spl_net_load_read, ರಿಕ್ತ, 1);
	ಪ್ರತ್ಯಾಗಮ spl_load(spl_image, bootdev, &load, 0, 0);
}
#endif

#ifdef CONFIG_SPL_ETH
ಪೂರ್ಣಾಙ್ಕ spl_net_load_image_cpgmac(ಸಂರಚನಾ spl_image_info *spl_image,
			      ಸಂರಚನಾ spl_boot_device *bootdev)
{
#ifdef CONFIG_SPL_ETH_DEVICE
	bootdev->boot_device_name = CONFIG_SPL_ETH_DEVICE;
#endif

	ಪ್ರತ್ಯಾಗಮ spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("eth device", 0, BOOT_DEVICE_CPGMAC,
		      spl_net_load_image_cpgmac);
#endif

#ifdef CONFIG_SPL_USB_ETHER
ಪೂರ್ಣಾಙ್ಕ spl_net_load_image_usb(ಸಂರಚನಾ spl_image_info *spl_image,
			   ಸಂರಚನಾ spl_boot_device *bootdev)
{
	bootdev->boot_device_name = "usb_ether";
#ಯದಿ CONFIG_IS_ENABLED(DM_USB_GADGET)
	usb_ether_init();
#endif
	ಪ್ರತ್ಯಾಗಮ spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("USB eth", 0, BOOT_DEVICE_USBETH, spl_net_load_image_usb);
#endif
