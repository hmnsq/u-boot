// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2012
 * Ilya Yanok <ilya.yanok@gmail.com>
 */
#అన్తర్భావ <env.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <spl.h>
#అన్తర్భావ <spl_load.h>
#అన్తర్భావ <net.h>
#అన్తర్భావ <linux/libfdt.h>

#ఒకవేళ defined(CONFIG_SPL_ETH) || defined(CONFIG_SPL_USB_ETHER)
స్థిర ulong spl_net_ఎక్కించు_read(నిర్మాణం spl_ఎక్కించు_info *ఎక్కించు, ulong sector,
			       ulong లెక్క, శూన్య *buf)
{
	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, లెక్క, (ulong)buf);
	జ్ఞాపకం_నకలు(buf, map_sysmem(image_ఎక్కించు_addr + sector, లెక్క), లెక్క);
	తిరిగి లెక్క;
}

స్థిర పూర్ణాంకం spl_net_ఎక్కించు_image(నిర్మాణం spl_image_info *spl_image,
			      నిర్మాణం spl_boot_device *bootdev)
{
	నిర్మాణం spl_ఎక్కించు_info ఎక్కించు;
	పూర్ణాంకం rv;

	env_init();
	env_relocate();
	env_సమితి("autoload", "yes");
	rv = eth_initialize();
	ఒకవేళ (rv == 0) {
		రాయి("No Ethernet devices found\n");
		తిరిగి -ENODEV;
	}
	ఒకవేళ (bootdev->boot_device_name)
		env_సమితి("ethact", bootdev->boot_device_name);
	rv = net_loop(BOOTP);
	ఒకవేళ (rv < 0) {
		రాయి("Problem booting with BOOTP\n");
		తిరిగి rv;
	}

	spl_ఎక్కించు_init(&ఎక్కించు, spl_net_ఎక్కించు_read, ఖాళీ, 1);
	తిరిగి spl_ఎక్కించు(spl_image, bootdev, &ఎక్కించు, 0, 0);
}
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_SPL_ETH
పూర్ణాంకం spl_net_ఎక్కించు_image_cpgmac(నిర్మాణం spl_image_info *spl_image,
			      నిర్మాణం spl_boot_device *bootdev)
{
#నిర్వచితమైతే CONFIG_SPL_ETH_DEVICE
	bootdev->boot_device_name = CONFIG_SPL_ETH_DEVICE;
#అయితే_ముగింపు

	తిరిగి spl_net_ఎక్కించు_image(spl_image, bootdev);
}
SPL_ఎక్కించు_IMAGE_పద్ధతి("eth device", 0, BOOT_DEVICE_CPGMAC,
		      spl_net_ఎక్కించు_image_cpgmac);
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_SPL_USB_ETHER
పూర్ణాంకం spl_net_ఎక్కించు_image_usb(నిర్మాణం spl_image_info *spl_image,
			   నిర్మాణం spl_boot_device *bootdev)
{
	bootdev->boot_device_name = "usb_ether";
#ఒకవేళ CONFIG_IS_ENABLED(DM_USB_GADGET)
	usb_ether_init();
#అయితే_ముగింపు
	తిరిగి spl_net_ఎక్కించు_image(spl_image, bootdev);
}
SPL_ఎక్కించు_IMAGE_పద్ధతి("USB eth", 0, BOOT_DEVICE_USBETH, spl_net_ఎక్కించు_image_usb);
#అయితే_ముగింపు
