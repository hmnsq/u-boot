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
ساكن ulong spl_net_بارول_read(بنية spl_بارول_info *بارول, ulong sector,
			       ulong شمېر, فراغ *buf)
{
	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, شمېر, (ulong)buf);
	حافظه_نقل(buf, map_sysmem(image_بارول_addr + sector, شمېر), شمېر);
	أرجع شمېر;
}

ساكن صحيح spl_net_بارول_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	بنية spl_بارول_info بارول;
	صحيح rv;

	env_init();
	env_relocate();
	env_سېټ("autoload", "yes");
	rv = eth_initialize();
	إذا (rv == 0) {
		چاپ_کړه("No Ethernet devices found\n");
		أرجع -ENODEV;
	}
	إذا (bootdev->boot_device_name)
		env_سېټ("ethact", bootdev->boot_device_name);
	rv = net_loop(BOOTP);
	إذا (rv < 0) {
		چاپ_کړه("Problem booting with BOOTP\n");
		أرجع rv;
	}

	spl_بارول_init(&بارول, spl_net_بارول_read, عدم, 1);
	أرجع spl_بارول(spl_image, bootdev, &بارول, 0, 0);
}
#د_که_پای

#که_تعریف_شوی CONFIG_SPL_ETH
صحيح spl_net_بارول_image_cpgmac(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
#که_تعریف_شوی CONFIG_SPL_ETH_DEVICE
	bootdev->boot_device_name = CONFIG_SPL_ETH_DEVICE;
#د_که_پای

	أرجع spl_net_بارول_image(spl_image, bootdev);
}
SPL_بارول_IMAGE_لاره("eth device", 0, BOOT_DEVICE_CPGMAC,
		      spl_net_بارول_image_cpgmac);
#د_که_پای

#که_تعریف_شوی CONFIG_SPL_USB_ETHER
صحيح spl_net_بارول_image_usb(بنية spl_image_info *spl_image,
			   بنية spl_boot_device *bootdev)
{
	bootdev->boot_device_name = "usb_ether";
#إذا CONFIG_IS_ENABLED(DM_USB_GADGET)
	usb_ether_init();
#د_که_پای
	أرجع spl_net_بارول_image(spl_image, bootdev);
}
SPL_بارول_IMAGE_لاره("USB eth", 0, BOOT_DEVICE_USBETH, spl_net_بارول_image_usb);
#د_که_پای
