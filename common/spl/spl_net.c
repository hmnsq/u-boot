// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2012
 * Ilya Yanok <ilya.yanok@gmail.com>
 */
#அந்தர்பாவ <env.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <image.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <spl_load.h>
#அந்தர்பாவ <net.h>
#அந்தர்பாவ <linux/libfdt.h>

#எனில் defined(CONFIG_SPL_ETH) || defined(CONFIG_SPL_USB_ETHER)
நிலையான ulong spl_net_load_read(கட்டமைப்பு spl_load_info *load, ulong sector,
			       ulong மொத்தம், வெறுமை *buf)
{
	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, மொத்தம், (ulong)buf);
	memcpy(buf, map_sysmem(image_load_addr + sector, மொத்தம்), மொத்தம்);
	திரும்பு மொத்தம்;
}

நிலையான முழு spl_net_load_image(கட்டமைப்பு spl_image_info *spl_image,
			      கட்டமைப்பு spl_boot_device *bootdev)
{
	கட்டமைப்பு spl_load_info load;
	முழு rv;

	env_init();
	env_relocate();
	env_set("autoload", "yes");
	rv = eth_initialize();
	எனில் (rv == 0) {
		printf("No Ethernet devices found\n");
		திரும்பு -ENODEV;
	}
	எனில் (bootdev->boot_device_name)
		env_set("ethact", bootdev->boot_device_name);
	rv = net_loop(BOOTP);
	எனில் (rv < 0) {
		printf("Problem booting with BOOTP\n");
		திரும்பு rv;
	}

	spl_load_init(&load, spl_net_load_read, வெற்று, 1);
	திரும்பு spl_load(spl_image, bootdev, &load, 0, 0);
}
#endif

#ifdef CONFIG_SPL_ETH
முழு spl_net_load_image_cpgmac(கட்டமைப்பு spl_image_info *spl_image,
			      கட்டமைப்பு spl_boot_device *bootdev)
{
#ifdef CONFIG_SPL_ETH_DEVICE
	bootdev->boot_device_name = CONFIG_SPL_ETH_DEVICE;
#endif

	திரும்பு spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("eth device", 0, BOOT_DEVICE_CPGMAC,
		      spl_net_load_image_cpgmac);
#endif

#ifdef CONFIG_SPL_USB_ETHER
முழு spl_net_load_image_usb(கட்டமைப்பு spl_image_info *spl_image,
			   கட்டமைப்பு spl_boot_device *bootdev)
{
	bootdev->boot_device_name = "usb_ether";
#எனில் CONFIG_IS_ENABLED(DM_USB_GADGET)
	usb_ether_init();
#endif
	திரும்பு spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("USB eth", 0, BOOT_DEVICE_USBETH, spl_net_load_image_usb);
#endif
