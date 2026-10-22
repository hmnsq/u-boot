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
স্থির ulong spl_net_load_read(কাঠামো spl_load_info *load, ulong sector,
			       ulong গুনতি, শূন্য *buf)
{
	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, গুনতি, (ulong)buf);
	memcpy(buf, map_sysmem(image_load_addr + sector, গুনতি), গুনতি);
	ফেরত গুনতি;
}

স্থির পূর্ণসংখ্যা spl_net_load_image(কাঠামো spl_image_info *spl_image,
			      কাঠামো spl_boot_device *bootdev)
{
	কাঠামো spl_load_info load;
	পূর্ণসংখ্যা rv;

	env_init();
	env_relocate();
	env_set("autoload", "yes");
	rv = eth_initialize();
	যদি (rv == 0) {
		printf("No Ethernet devices found\n");
		ফেরত -ENODEV;
	}
	যদি (bootdev->boot_device_name)
		env_set("ethact", bootdev->boot_device_name);
	rv = net_loop(BOOTP);
	যদি (rv < 0) {
		printf("Problem booting with BOOTP\n");
		ফেরত rv;
	}

	spl_load_init(&load, spl_net_load_read, ফাঁকা, 1);
	ফেরত spl_load(spl_image, bootdev, &load, 0, 0);
}
#endif

#ifdef CONFIG_SPL_ETH
পূর্ণসংখ্যা spl_net_load_image_cpgmac(কাঠামো spl_image_info *spl_image,
			      কাঠামো spl_boot_device *bootdev)
{
#ifdef CONFIG_SPL_ETH_DEVICE
	bootdev->boot_device_name = CONFIG_SPL_ETH_DEVICE;
#endif

	ফেরত spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("eth device", 0, BOOT_DEVICE_CPGMAC,
		      spl_net_load_image_cpgmac);
#endif

#ifdef CONFIG_SPL_USB_ETHER
পূর্ণসংখ্যা spl_net_load_image_usb(কাঠামো spl_image_info *spl_image,
			   কাঠামো spl_boot_device *bootdev)
{
	bootdev->boot_device_name = "usb_ether";
#যদি CONFIG_IS_ENABLED(DM_USB_GADGET)
	usb_ether_init();
#endif
	ফেরত spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("USB eth", 0, BOOT_DEVICE_USBETH, spl_net_load_image_usb);
#endif
