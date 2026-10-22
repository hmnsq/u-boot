// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2012
 * Ilya Yanok <ilya.yanok@gmail.com>
 */
#अन्तर्भाव <env.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <spl_load.h>
#अन्तर्भाव <net.h>
#अन्तर्भाव <linux/libfdt.h>

#यदि defined(CONFIG_SPL_ETH) || defined(CONFIG_SPL_USB_ETHER)
स्थिर ulong spl_net_आरोपय_read(संरचना spl_आरोपय_info *आरोपय, ulong sector,
			       ulong गणनम्, शून्य *buf)
{
	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, गणनम्, (ulong)buf);
	स्मृतिं_प्रतिलिख(buf, map_sysmem(image_आरोपय_addr + sector, गणनम्), गणनम्);
	प्रत्यागम गणनम्;
}

स्थिर पूर्णाङ्क spl_net_आरोपय_image(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev)
{
	संरचना spl_आरोपय_info आरोपय;
	पूर्णाङ्क rv;

	env_init();
	env_relocate();
	env_समुच्चयः("autoload", "yes");
	rv = eth_initialize();
	यदि (rv == 0) {
		प्रारूप_लिख("No Ethernet devices found\n");
		प्रत्यागम -ENODEV;
	}
	यदि (bootdev->boot_device_name)
		env_समुच्चयः("ethact", bootdev->boot_device_name);
	rv = net_loop(BOOTP);
	यदि (rv < 0) {
		प्रारूप_लिख("Problem booting with BOOTP\n");
		प्रत्यागम rv;
	}

	spl_आरोपय_init(&आरोपय, spl_net_आरोपय_read, रिक्त, 1);
	प्रत्यागम spl_आरोपय(spl_image, bootdev, &आरोपय, 0, 0);
}
#यदि_अन्तः

#यदि_परिभाषितम् CONFIG_SPL_ETH
पूर्णाङ्क spl_net_आरोपय_image_cpgmac(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev)
{
#यदि_परिभाषितम् CONFIG_SPL_ETH_DEVICE
	bootdev->boot_device_name = CONFIG_SPL_ETH_DEVICE;
#यदि_अन्तः

	प्रत्यागम spl_net_आरोपय_image(spl_image, bootdev);
}
SPL_आरोपय_IMAGE_विधानम्("eth device", 0, BOOT_DEVICE_CPGMAC,
		      spl_net_आरोपय_image_cpgmac);
#यदि_अन्तः

#यदि_परिभाषितम् CONFIG_SPL_USB_ETHER
पूर्णाङ्क spl_net_आरोपय_image_usb(संरचना spl_image_info *spl_image,
			   संरचना spl_boot_device *bootdev)
{
	bootdev->boot_device_name = "usb_ether";
#यदि CONFIG_IS_ENABLED(DM_USB_GADGET)
	usb_ether_init();
#यदि_अन्तः
	प्रत्यागम spl_net_आरोपय_image(spl_image, bootdev);
}
SPL_आरोपय_IMAGE_विधानम्("USB eth", 0, BOOT_DEVICE_USBETH, spl_net_आरोपय_image_usb);
#यदि_अन्तः
