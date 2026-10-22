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
ساكن ulong spl_net_لدو_read(بنية spl_لدو_info *لدو, ulong sector,
			       ulong گنتی, فراغ *buf)
{
	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, گنتی, (ulong)buf);
	حافظہ_نقل(buf, map_sysmem(image_لدو_addr + sector, گنتی), گنتی);
	أرجع گنتی;
}

ساكن صحيح spl_net_لدو_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	بنية spl_لدو_info لدو;
	صحيح rv;

	env_init();
	env_relocate();
	env_سیٹ("autoload", "yes");
	rv = eth_initialize();
	إذا (rv == 0) {
		لکھو("No Ethernet devices found\n");
		أرجع -ENODEV;
	}
	إذا (bootdev->boot_device_name)
		env_سیٹ("ethact", bootdev->boot_device_name);
	rv = net_loop(BOOTP);
	إذا (rv < 0) {
		لکھو("Problem booting with BOOTP\n");
		أرجع rv;
	}

	spl_لدو_init(&لدو, spl_net_لدو_read, عدم, 1);
	أرجع spl_لدو(spl_image, bootdev, &لدو, 0, 0);
}
#جے_انت

#جے_تعریف_شدہ CONFIG_SPL_ETH
صحيح spl_net_لدو_image_cpgmac(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
#جے_تعریف_شدہ CONFIG_SPL_ETH_DEVICE
	bootdev->boot_device_name = CONFIG_SPL_ETH_DEVICE;
#جے_انت

	أرجع spl_net_لدو_image(spl_image, bootdev);
}
SPL_لدو_IMAGE_طریقہ("eth device", 0, BOOT_DEVICE_CPGMAC,
		      spl_net_لدو_image_cpgmac);
#جے_انت

#جے_تعریف_شدہ CONFIG_SPL_USB_ETHER
صحيح spl_net_لدو_image_usb(بنية spl_image_info *spl_image,
			   بنية spl_boot_device *bootdev)
{
	bootdev->boot_device_name = "usb_ether";
#إذا CONFIG_IS_ENABLED(DM_USB_GADGET)
	usb_ether_init();
#جے_انت
	أرجع spl_net_لدو_image(spl_image, bootdev);
}
SPL_لدو_IMAGE_طریقہ("USB eth", 0, BOOT_DEVICE_USBETH, spl_net_لدو_image_usb);
#جے_انت
