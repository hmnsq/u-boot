// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2012
 * Ilya Yanok <ilya.yanok@gmail.com>
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl_load.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <net.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/libfdt.h>

#ਜੇ defined(CONFIG_SPL_ETH) || defined(CONFIG_SPL_USB_ETHER)
ਸਥਿਰ ulong spl_net_ਲੱਦੋ_read(ਬਣਤਰ spl_ਲੱਦੋ_info *ਲੱਦੋ, ulong sector,
			       ulong ਗਿਣਤੀ_2, ਖਾਲੀ *buf)
{
	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, ਗਿਣਤੀ_2, (ulong)buf);
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(buf, map_sysmem(image_ਲੱਦੋ_addr + sector, ਗਿਣਤੀ_2), ਗਿਣਤੀ_2);
	ਵਾਪਸ ਗਿਣਤੀ_2;
}

ਸਥਿਰ ਪੂਰਨ spl_net_ਲੱਦੋ_image(ਬਣਤਰ spl_image_info *spl_image,
			      ਬਣਤਰ spl_boot_device *bootdev)
{
	ਬਣਤਰ spl_ਲੱਦੋ_info ਲੱਦੋ;
	ਪੂਰਨ rv;

	env_init();
	env_relocate();
	env_ਸੈੱਟ("autoload", "yes");
	rv = eth_initialize();
	ਜੇ (rv == 0) {
		ਲਿਖੋ("No Ethernet devices found\n");
		ਵਾਪਸ -ENODEV;
	}
	ਜੇ (bootdev->boot_device_name)
		env_ਸੈੱਟ("ethact", bootdev->boot_device_name);
	rv = net_loop(BOOTP);
	ਜੇ (rv < 0) {
		ਲਿਖੋ("Problem booting with BOOTP\n");
		ਵਾਪਸ rv;
	}

	spl_ਲੱਦੋ_init(&ਲੱਦੋ, spl_net_ਲੱਦੋ_read, ਨਿਰਾ, 1);
	ਵਾਪਸ spl_ਲੱਦੋ(spl_image, bootdev, &ਲੱਦੋ, 0, 0);
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPL_ETH
ਪੂਰਨ spl_net_ਲੱਦੋ_image_cpgmac(ਬਣਤਰ spl_image_info *spl_image,
			      ਬਣਤਰ spl_boot_device *bootdev)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPL_ETH_DEVICE
	bootdev->boot_device_name = CONFIG_SPL_ETH_DEVICE;
#ਜੇ_ਅੰਤ

	ਵਾਪਸ spl_net_ਲੱਦੋ_image(spl_image, bootdev);
}
SPL_ਲੱਦੋ_IMAGE_ਵਿਧੀ("eth device", 0, BOOT_DEVICE_CPGMAC,
		      spl_net_ਲੱਦੋ_image_cpgmac);
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPL_USB_ETHER
ਪੂਰਨ spl_net_ਲੱਦੋ_image_usb(ਬਣਤਰ spl_image_info *spl_image,
			   ਬਣਤਰ spl_boot_device *bootdev)
{
	bootdev->boot_device_name = "usb_ether";
#ਜੇ CONFIG_IS_ENABLED(DM_USB_GADGET)
	usb_ether_init();
#ਜੇ_ਅੰਤ
	ਵਾਪਸ spl_net_ਲੱਦੋ_image(spl_image, bootdev);
}
SPL_ਲੱਦੋ_IMAGE_ਵਿਧੀ("USB eth", 0, BOOT_DEVICE_USBETH, spl_net_ਲੱਦੋ_image_usb);
#ਜੇ_ਅੰਤ
