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
ساكن ulong spl_net_ڀريو_read(بنية spl_ڀريو_info *ڀريو, ulong sector,
			       ulong ڳڻپ, فراغ *buf)
{
	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, ڳڻپ, (ulong)buf);
	يادگيري_نقل(buf, map_sysmem(image_ڀريو_addr + sector, ڳڻپ), ڳڻپ);
	أرجع ڳڻپ;
}

ساكن صحيح spl_net_ڀريو_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	بنية spl_ڀريو_info ڀريو;
	صحيح rv;

	env_init();
	env_relocate();
	env_سيٽ("autoload", "yes");
	rv = eth_initialize();
	إذا (rv == 0) {
		لکو("No Ethernet devices found\n");
		أرجع -ENODEV;
	}
	إذا (bootdev->boot_device_name)
		env_سيٽ("ethact", bootdev->boot_device_name);
	rv = net_loop(BOOTP);
	إذا (rv < 0) {
		لکو("Problem booting with BOOTP\n");
		أرجع rv;
	}

	spl_ڀريو_init(&ڀريو, spl_net_ڀريو_read, عدم, 1);
	أرجع spl_ڀريو(spl_image, bootdev, &ڀريو, 0, 0);
}
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل CONFIG_SPL_ETH
صحيح spl_net_ڀريو_image_cpgmac(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
#جيڪڏهن_وصف_ٿيل CONFIG_SPL_ETH_DEVICE
	bootdev->boot_device_name = CONFIG_SPL_ETH_DEVICE;
#جيڪڏهن_پورو

	أرجع spl_net_ڀريو_image(spl_image, bootdev);
}
SPL_ڀريو_IMAGE_طريقو("eth device", 0, BOOT_DEVICE_CPGMAC,
		      spl_net_ڀريو_image_cpgmac);
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل CONFIG_SPL_USB_ETHER
صحيح spl_net_ڀريو_image_usb(بنية spl_image_info *spl_image,
			   بنية spl_boot_device *bootdev)
{
	bootdev->boot_device_name = "usb_ether";
#إذا CONFIG_IS_ENABLED(DM_USB_GADGET)
	usb_ether_init();
#جيڪڏهن_پورو
	أرجع spl_net_ڀريو_image(spl_image, bootdev);
}
SPL_ڀريو_IMAGE_طريقو("USB eth", 0, BOOT_DEVICE_USBETH, spl_net_ڀريو_image_usb);
#جيڪڏهن_پورو
