// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2012
 * Ilya Yanok <ilya.yanok@gmail.com>
 */
#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <image.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <spl_load.h>
#ଅନ୍ତର୍ଭାଵ <net.h>
#ଅନ୍ତର୍ଭାଵ <linux/libfdt.h>

#ଯଦି defined(CONFIG_SPL_ETH) || defined(CONFIG_SPL_USB_ETHER)
ସ୍ଥିର ulong spl_net_load_read(ସଂରଚନା spl_load_info *load, ulong sector,
			       ulong ଗଣନା_2, ଶୂନ୍ୟ *buf)
{
	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, ଗଣନା_2, (ulong)buf);
	memcpy(buf, map_sysmem(image_load_addr + sector, ଗଣନା_2), ଗଣନା_2);
	ଫେରନ୍ତୁ ଗଣନା_2;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_net_load_image(ସଂରଚନା spl_image_info *spl_image,
			      ସଂରଚନା spl_boot_device *bootdev)
{
	ସଂରଚନା spl_load_info load;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rv;

	env_init();
	env_relocate();
	env_set("autoload", "yes");
	rv = eth_initialize();
	ଯଦି (rv == 0) {
		printf("No Ethernet devices found\n");
		ଫେରନ୍ତୁ -ENODEV;
	}
	ଯଦି (bootdev->boot_device_name)
		env_set("ethact", bootdev->boot_device_name);
	rv = net_loop(BOOTP);
	ଯଦି (rv < 0) {
		printf("Problem booting with BOOTP\n");
		ଫେରନ୍ତୁ rv;
	}

	spl_load_init(&load, spl_net_load_read, ଖାଲି, 1);
	ଫେରନ୍ତୁ spl_load(spl_image, bootdev, &load, 0, 0);
}
#endif

#ifdef CONFIG_SPL_ETH
ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_net_load_image_cpgmac(ସଂରଚନା spl_image_info *spl_image,
			      ସଂରଚନା spl_boot_device *bootdev)
{
#ifdef CONFIG_SPL_ETH_DEVICE
	bootdev->boot_device_name = CONFIG_SPL_ETH_DEVICE;
#endif

	ଫେରନ୍ତୁ spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("eth device", 0, BOOT_DEVICE_CPGMAC,
		      spl_net_load_image_cpgmac);
#endif

#ifdef CONFIG_SPL_USB_ETHER
ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_net_load_image_usb(ସଂରଚନା spl_image_info *spl_image,
			   ସଂରଚନା spl_boot_device *bootdev)
{
	bootdev->boot_device_name = "usb_ether";
#ଯଦି CONFIG_IS_ENABLED(DM_USB_GADGET)
	usb_ether_init();
#endif
	ଫେରନ୍ତୁ spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("USB eth", 0, BOOT_DEVICE_USBETH, spl_net_load_image_usb);
#endif
