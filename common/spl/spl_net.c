// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2012
 * Ilya Yanok <ilya.yanok@gmail.com>
 */
#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <image.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <spl_load.h>
#અન્તર્ભાવ <net.h>
#અન્તર્ભાવ <linux/libfdt.h>

#જો defined(CONFIG_SPL_ETH) || defined(CONFIG_SPL_USB_ETHER)
સ્થિર ulong spl_net_load_read(સંરચના spl_load_info *load, ulong sector,
			       ulong ગણતરી, શૂન્ય *buf)
{
	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, ગણતરી, (ulong)buf);
	memcpy(buf, map_sysmem(image_load_addr + sector, ગણતરી), ગણતરી);
	પાછા ગણતરી;
}

સ્થિર પૂર્ણાંક spl_net_load_image(સંરચના spl_image_info *spl_image,
			      સંરચના spl_boot_device *bootdev)
{
	સંરચના spl_load_info load;
	પૂર્ણાંક rv;

	env_init();
	env_relocate();
	env_set("autoload", "yes");
	rv = eth_initialize();
	જો (rv == 0) {
		printf("No Ethernet devices found\n");
		પાછા -ENODEV;
	}
	જો (bootdev->boot_device_name)
		env_set("ethact", bootdev->boot_device_name);
	rv = net_loop(BOOTP);
	જો (rv < 0) {
		printf("Problem booting with BOOTP\n");
		પાછા rv;
	}

	spl_load_init(&load, spl_net_load_read, ખાલી, 1);
	પાછા spl_load(spl_image, bootdev, &load, 0, 0);
}
#endif

#ifdef CONFIG_SPL_ETH
પૂર્ણાંક spl_net_load_image_cpgmac(સંરચના spl_image_info *spl_image,
			      સંરચના spl_boot_device *bootdev)
{
#ifdef CONFIG_SPL_ETH_DEVICE
	bootdev->boot_device_name = CONFIG_SPL_ETH_DEVICE;
#endif

	પાછા spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("eth device", 0, BOOT_DEVICE_CPGMAC,
		      spl_net_load_image_cpgmac);
#endif

#ifdef CONFIG_SPL_USB_ETHER
પૂર્ણાંક spl_net_load_image_usb(સંરચના spl_image_info *spl_image,
			   સંરચના spl_boot_device *bootdev)
{
	bootdev->boot_device_name = "usb_ether";
#જો CONFIG_IS_ENABLED(DM_USB_GADGET)
	usb_ether_init();
#endif
	પાછા spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("USB eth", 0, BOOT_DEVICE_USBETH, spl_net_load_image_usb);
#endif
