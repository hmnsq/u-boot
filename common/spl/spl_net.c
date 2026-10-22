// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2012
 * Ilya Yanok <ilya.yanok@gmail.com>
 */
#অন্তর্ভাব <env.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <image.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <spl.h>
#অন্তর্ভাব <spl_load.h>
#অন্তর্ভাব <net.h>
#অন্তর্ভাব <linux/libfdt.h>

#যদি defined(CONFIG_SPL_ETH) || defined(CONFIG_SPL_USB_ETHER)
স্থির ulong spl_net_load_read(সংরচনা spl_load_info *load, ulong sector,
			       ulong গুনতি, শূন্য *buf)
{
	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, গুনতি, (ulong)buf);
	memcpy(buf, map_sysmem(image_load_addr + sector, গুনতি), গুনতি);
	প্রত্যাগম গুনতি;
}

স্থির পূর্ণাঙ্ক spl_net_load_image(সংরচনা spl_image_info *spl_image,
			      সংরচনা spl_boot_device *bootdev)
{
	সংরচনা spl_load_info load;
	পূর্ণাঙ্ক rv;

	env_init();
	env_relocate();
	env_set("autoload", "yes");
	rv = eth_initialize();
	যদি (rv == 0) {
		printf("No Ethernet devices found\n");
		প্রত্যাগম -ENODEV;
	}
	যদি (bootdev->boot_device_name)
		env_set("ethact", bootdev->boot_device_name);
	rv = net_loop(BOOTP);
	যদি (rv < 0) {
		printf("Problem booting with BOOTP\n");
		প্রত্যাগম rv;
	}

	spl_load_init(&load, spl_net_load_read, রিক্ত, 1);
	প্রত্যাগম spl_load(spl_image, bootdev, &load, 0, 0);
}
#endif

#ifdef CONFIG_SPL_ETH
পূর্ণাঙ্ক spl_net_load_image_cpgmac(সংরচনা spl_image_info *spl_image,
			      সংরচনা spl_boot_device *bootdev)
{
#ifdef CONFIG_SPL_ETH_DEVICE
	bootdev->boot_device_name = CONFIG_SPL_ETH_DEVICE;
#endif

	প্রত্যাগম spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("eth device", 0, BOOT_DEVICE_CPGMAC,
		      spl_net_load_image_cpgmac);
#endif

#ifdef CONFIG_SPL_USB_ETHER
পূর্ণাঙ্ক spl_net_load_image_usb(সংরচনা spl_image_info *spl_image,
			   সংরচনা spl_boot_device *bootdev)
{
	bootdev->boot_device_name = "usb_ether";
#যদি CONFIG_IS_ENABLED(DM_USB_GADGET)
	usb_ether_init();
#endif
	প্রত্যাগম spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("USB eth", 0, BOOT_DEVICE_USBETH, spl_net_load_image_usb);
#endif
