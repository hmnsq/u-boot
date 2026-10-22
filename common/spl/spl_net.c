// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2012
 * Ilya Yanok <ilya.yanok@gmail.com>
 */
#ضمن <env.h>
#ضمن <errno.h>
#ضمن <image.h>
#ضمن <log.h>
#ضمن <spl.h>
#ضمن <spl_load.h>
#ضمن <net.h>
#ضمن <linux/libfdt.h>

#إذا defined(CONFIG_SPL_ETH) || defined(CONFIG_SPL_USB_ETHER)
ساكن ulong spl_net_load_read(بنية spl_load_info *load, ulong sector,
			       ulong گنتی, فراغ *buf)
{
	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, گنتی, (ulong)buf);
	memcpy(buf, map_sysmem(image_load_addr + sector, گنتی), گنتی);
	أرجع گنتی;
}

ساكن صحيح spl_net_load_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	بنية spl_load_info load;
	صحيح rv;

	env_init();
	env_relocate();
	env_set("autoload", "yes");
	rv = eth_initialize();
	إذا (rv == 0) {
		printf("No Ethernet devices found\n");
		أرجع -ENODEV;
	}
	إذا (bootdev->boot_device_name)
		env_set("ethact", bootdev->boot_device_name);
	rv = net_loop(BOOTP);
	إذا (rv < 0) {
		printf("Problem booting with BOOTP\n");
		أرجع rv;
	}

	spl_load_init(&load, spl_net_load_read, عدم, 1);
	أرجع spl_load(spl_image, bootdev, &load, 0, 0);
}
#endif

#ifdef CONFIG_SPL_ETH
صحيح spl_net_load_image_cpgmac(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
#ifdef CONFIG_SPL_ETH_DEVICE
	bootdev->boot_device_name = CONFIG_SPL_ETH_DEVICE;
#endif

	أرجع spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("eth device", 0, BOOT_DEVICE_CPGMAC,
		      spl_net_load_image_cpgmac);
#endif

#ifdef CONFIG_SPL_USB_ETHER
صحيح spl_net_load_image_usb(بنية spl_image_info *spl_image,
			   بنية spl_boot_device *bootdev)
{
	bootdev->boot_device_name = "usb_ether";
#إذا CONFIG_IS_ENABLED(DM_USB_GADGET)
	usb_ether_init();
#endif
	أرجع spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("USB eth", 0, BOOT_DEVICE_USBETH, spl_net_load_image_usb);
#endif
