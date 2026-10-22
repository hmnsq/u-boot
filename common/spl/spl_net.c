// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2012
 * Ilya Yanok <ilya.yanok@gmail.com>
 */
#അന്തര്ഭാവ <env.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <image.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <spl_load.h>
#അന്തര്ഭാവ <net.h>
#അന്തര്ഭാവ <linux/libfdt.h>

#എങ്കിൽ defined(CONFIG_SPL_ETH) || defined(CONFIG_SPL_USB_ETHER)
സ്ഥിതം ulong spl_net_load_read(ഘടന spl_load_info *load, ulong sector,
			       ulong എണ്ണം, ശൂന്യം *buf)
{
	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, എണ്ണം, (ulong)buf);
	memcpy(buf, map_sysmem(image_load_addr + sector, എണ്ണം), എണ്ണം);
	തിരികെ എണ്ണം;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_net_load_image(ഘടന spl_image_info *spl_image,
			      ഘടന spl_boot_device *bootdev)
{
	ഘടന spl_load_info load;
	പൂർണ്ണസംഖ്യ rv;

	env_init();
	env_relocate();
	env_set("autoload", "yes");
	rv = eth_initialize();
	എങ്കിൽ (rv == 0) {
		printf("No Ethernet devices found\n");
		തിരികെ -ENODEV;
	}
	എങ്കിൽ (bootdev->boot_device_name)
		env_set("ethact", bootdev->boot_device_name);
	rv = net_loop(BOOTP);
	എങ്കിൽ (rv < 0) {
		printf("Problem booting with BOOTP\n");
		തിരികെ rv;
	}

	spl_load_init(&load, spl_net_load_read, ശൂന്യ, 1);
	തിരികെ spl_load(spl_image, bootdev, &load, 0, 0);
}
#endif

#ifdef CONFIG_SPL_ETH
പൂർണ്ണസംഖ്യ spl_net_load_image_cpgmac(ഘടന spl_image_info *spl_image,
			      ഘടന spl_boot_device *bootdev)
{
#ifdef CONFIG_SPL_ETH_DEVICE
	bootdev->boot_device_name = CONFIG_SPL_ETH_DEVICE;
#endif

	തിരികെ spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("eth device", 0, BOOT_DEVICE_CPGMAC,
		      spl_net_load_image_cpgmac);
#endif

#ifdef CONFIG_SPL_USB_ETHER
പൂർണ്ണസംഖ്യ spl_net_load_image_usb(ഘടന spl_image_info *spl_image,
			   ഘടന spl_boot_device *bootdev)
{
	bootdev->boot_device_name = "usb_ether";
#എങ്കിൽ CONFIG_IS_ENABLED(DM_USB_GADGET)
	usb_ether_init();
#endif
	തിരികെ spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("USB eth", 0, BOOT_DEVICE_USBETH, spl_net_load_image_usb);
#endif
