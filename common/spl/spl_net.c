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
ساكن ulong spl_net_بارگیری_read(بنية spl_بارگیری_info *بارگیری, ulong sector,
			       ulong شمار, فراغ *buf)
{
	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, شمار, (ulong)buf);
	کپی_حافظه(buf, map_sysmem(image_بارگیری_addr + sector, شمار), شمار);
	أرجع شمار;
}

ساكن صحيح spl_net_بارگیری_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	بنية spl_بارگیری_info بارگیری;
	صحيح rv;

	env_init();
	env_relocate();
	env_مجموعه("autoload", "yes");
	rv = eth_initialize();
	إذا (rv == 0) {
		چاپ_کن("No Ethernet devices found\n");
		أرجع -ENODEV;
	}
	إذا (bootdev->boot_device_name)
		env_مجموعه("ethact", bootdev->boot_device_name);
	rv = net_loop(BOOTP);
	إذا (rv < 0) {
		چاپ_کن("Problem booting with BOOTP\n");
		أرجع rv;
	}

	spl_بارگیری_init(&بارگیری, spl_net_بارگیری_read, عدم, 1);
	أرجع spl_بارگیری(spl_image, bootdev, &بارگیری, 0, 0);
}
#پایان_اگر

#اگر_تعریف_شده CONFIG_SPL_ETH
صحيح spl_net_بارگیری_image_cpgmac(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
#اگر_تعریف_شده CONFIG_SPL_ETH_DEVICE
	bootdev->boot_device_name = CONFIG_SPL_ETH_DEVICE;
#پایان_اگر

	أرجع spl_net_بارگیری_image(spl_image, bootdev);
}
SPL_بارگیری_IMAGE_روش("eth device", 0, BOOT_DEVICE_CPGMAC,
		      spl_net_بارگیری_image_cpgmac);
#پایان_اگر

#اگر_تعریف_شده CONFIG_SPL_USB_ETHER
صحيح spl_net_بارگیری_image_usb(بنية spl_image_info *spl_image,
			   بنية spl_boot_device *bootdev)
{
	bootdev->boot_device_name = "usb_ether";
#إذا CONFIG_IS_ENABLED(DM_USB_GADGET)
	usb_ether_init();
#پایان_اگر
	أرجع spl_net_بارگیری_image(spl_image, bootdev);
}
SPL_بارگیری_IMAGE_روش("USB eth", 0, BOOT_DEVICE_USBETH, spl_net_بارگیری_image_usb);
#پایان_اگر
