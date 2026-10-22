// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2012
 * Ilya Yanok <ilya.yanok@gmail.com>
 */
#شامل <env.h>
#شامل <errno.h>
#شامل <image.h>
#شامل <log.h>
#شامل <spl.h>
#شامل <spl_load.h>
#شامل <net.h>
#شامل <linux/libfdt.h>

#اگر defined(CONFIG_SPL_ETH) || defined(CONFIG_SPL_USB_ETHER)
ایستا ulong spl_net_load_read(ساختار spl_load_info *load, ulong sector,
			       ulong شمار, تهی *buf)
{
	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, شمار, (ulong)buf);
	memcpy(buf, map_sysmem(image_load_addr + sector, شمار), شمار);
	بازگشت شمار;
}

ایستا صحیح spl_net_load_image(ساختار spl_image_info *spl_image,
			      ساختار spl_boot_device *bootdev)
{
	ساختار spl_load_info load;
	صحیح rv;

	env_init();
	env_relocate();
	env_set("autoload", "yes");
	rv = eth_initialize();
	اگر (rv == 0) {
		printf("No Ethernet devices found\n");
		بازگشت -ENODEV;
	}
	اگر (bootdev->boot_device_name)
		env_set("ethact", bootdev->boot_device_name);
	rv = net_loop(BOOTP);
	اگر (rv < 0) {
		printf("Problem booting with BOOTP\n");
		بازگشت rv;
	}

	spl_load_init(&load, spl_net_load_read, پوچ, 1);
	بازگشت spl_load(spl_image, bootdev, &load, 0, 0);
}
#endif

#ifdef CONFIG_SPL_ETH
صحیح spl_net_load_image_cpgmac(ساختار spl_image_info *spl_image,
			      ساختار spl_boot_device *bootdev)
{
#ifdef CONFIG_SPL_ETH_DEVICE
	bootdev->boot_device_name = CONFIG_SPL_ETH_DEVICE;
#endif

	بازگشت spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("eth device", 0, BOOT_DEVICE_CPGMAC,
		      spl_net_load_image_cpgmac);
#endif

#ifdef CONFIG_SPL_USB_ETHER
صحیح spl_net_load_image_usb(ساختار spl_image_info *spl_image,
			   ساختار spl_boot_device *bootdev)
{
	bootdev->boot_device_name = "usb_ether";
#اگر CONFIG_IS_ENABLED(DM_USB_GADGET)
	usb_ether_init();
#endif
	بازگشت spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("USB eth", 0, BOOT_DEVICE_USBETH, spl_net_load_image_usb);
#endif
