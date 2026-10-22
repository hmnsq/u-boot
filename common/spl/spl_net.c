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
ساکن ulong spl_net_بھرو_read(ساخت spl_بھرو_info *بھرو, ulong sector,
			       ulong گنتی, خالی *buf)
{
	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, گنتی, (ulong)buf);
	حافظہ_نقل(buf, map_sysmem(image_بھرو_addr + sector, گنتی), گنتی);
	واپس گنتی;
}

ساکن صحیح spl_net_بھرو_image(ساخت spl_image_info *spl_image,
			      ساخت spl_boot_device *bootdev)
{
	ساخت spl_بھرو_info بھرو;
	صحیح rv;

	env_init();
	env_relocate();
	env_سیٹ("autoload", "yes");
	rv = eth_initialize();
	اگر (rv == 0) {
		لکھو("No Ethernet devices found\n");
		واپس -ENODEV;
	}
	اگر (bootdev->boot_device_name)
		env_سیٹ("ethact", bootdev->boot_device_name);
	rv = net_loop(BOOTP);
	اگر (rv < 0) {
		لکھو("Problem booting with BOOTP\n");
		واپس rv;
	}

	spl_بھرو_init(&بھرو, spl_net_بھرو_read, تہی, 1);
	واپس spl_بھرو(spl_image, bootdev, &بھرو, 0, 0);
}
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_SPL_ETH
صحیح spl_net_بھرو_image_cpgmac(ساخت spl_image_info *spl_image,
			      ساخت spl_boot_device *bootdev)
{
#اگر_تعریف_شدہ CONFIG_SPL_ETH_DEVICE
	bootdev->boot_device_name = CONFIG_SPL_ETH_DEVICE;
#اگر_ختم

	واپس spl_net_بھرو_image(spl_image, bootdev);
}
SPL_بھرو_IMAGE_طریقہ("eth device", 0, BOOT_DEVICE_CPGMAC,
		      spl_net_بھرو_image_cpgmac);
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_SPL_USB_ETHER
صحیح spl_net_بھرو_image_usb(ساخت spl_image_info *spl_image,
			   ساخت spl_boot_device *bootdev)
{
	bootdev->boot_device_name = "usb_ether";
#اگر CONFIG_IS_ENABLED(DM_USB_GADGET)
	usb_ether_init();
#اگر_ختم
	واپس spl_net_بھرو_image(spl_image, bootdev);
}
SPL_بھرو_IMAGE_طریقہ("USB eth", 0, BOOT_DEVICE_USBETH, spl_net_بھرو_image_usb);
#اگر_ختم
