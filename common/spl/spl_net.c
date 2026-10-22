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

#जर defined(CONFIG_SPL_ETH) || defined(CONFIG_SPL_USB_ETHER)
स्थिर ulong spl_net_load_read(संरचना spl_load_info *load, ulong sector,
			       ulong मोजणी, शून्य *buf)
{
	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, मोजणी, (ulong)buf);
	स_नकल(buf, map_sysmem(image_load_addr + sector, मोजणी), मोजणी);
	परत मोजणी;
}

स्थिर पूर्णांक spl_net_load_image(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev)
{
	संरचना spl_load_info load;
	पूर्णांक rv;

	env_init();
	env_relocate();
	env_set("autoload", "yes");
	rv = eth_initialize();
	जर (rv == 0) {
		म_लिखो("No Ethernet devices found\n");
		परत -ENODEV;
	}
	जर (bootdev->boot_device_name)
		env_set("ethact", bootdev->boot_device_name);
	rv = net_loop(BOOTP);
	जर (rv < 0) {
		म_लिखो("Problem booting with BOOTP\n");
		परत rv;
	}

	spl_load_init(&load, spl_net_load_read, रिक्त, 1);
	परत spl_load(spl_image, bootdev, &load, 0, 0);
}
#endif

#ifdef CONFIG_SPL_ETH
पूर्णांक spl_net_load_image_cpgmac(संरचना spl_image_info *spl_image,
			      संरचना spl_boot_device *bootdev)
{
#ifdef CONFIG_SPL_ETH_DEVICE
	bootdev->boot_device_name = CONFIG_SPL_ETH_DEVICE;
#endif

	परत spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("eth device", 0, BOOT_DEVICE_CPGMAC,
		      spl_net_load_image_cpgmac);
#endif

#ifdef CONFIG_SPL_USB_ETHER
पूर्णांक spl_net_load_image_usb(संरचना spl_image_info *spl_image,
			   संरचना spl_boot_device *bootdev)
{
	bootdev->boot_device_name = "usb_ether";
#जर CONFIG_IS_ENABLED(DM_USB_GADGET)
	usb_ether_init();
#endif
	परत spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("USB eth", 0, BOOT_DEVICE_USBETH, spl_net_load_image_usb);
#endif
