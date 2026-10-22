// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2012
 * Ilya Yanok <ilya.yanok@gmail.com>
 */
#include <env.h>
#include <errno.h>
#include <image.h>
#include <log.h>
#include <spl.h>
#include <spl_load.h>
#include <net.h>
#include <linux/libfdt.h>

#如果 defined(CONFIG_SPL_ETH) || defined(CONFIG_SPL_USB_ETHER)
static ulong spl_net_load_read(struct spl_load_info *load, ulong sector,
			       ulong count, void *buf)
{
	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, count, (ulong)buf);
	memcpy(buf, map_sysmem(image_load_addr + sector, count), count);
	返回 count;
}

static 整数 spl_net_load_image(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev)
{
	struct spl_load_info load;
	整数 rv;

	env_init();
	env_relocate();
	env_set("autoload", "yes");
	rv = eth_initialize();
	如果 (rv == 0) {
		printf("No Ethernet devices found\n");
		返回 -ENODEV;
	}
	如果 (bootdev->boot_device_name)
		env_set("ethact", bootdev->boot_device_name);
	rv = net_loop(BOOTP);
	如果 (rv < 0) {
		printf("Problem booting with BOOTP\n");
		返回 rv;
	}

	spl_load_init(&load, spl_net_load_read, NULL, 1);
	返回 spl_load(spl_image, bootdev, &load, 0, 0);
}
#endif

#ifdef CONFIG_SPL_ETH
整数 spl_net_load_image_cpgmac(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev)
{
#ifdef CONFIG_SPL_ETH_DEVICE
	bootdev->boot_device_name = CONFIG_SPL_ETH_DEVICE;
#endif

	返回 spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("eth device", 0, BOOT_DEVICE_CPGMAC,
		      spl_net_load_image_cpgmac);
#endif

#ifdef CONFIG_SPL_USB_ETHER
整数 spl_net_load_image_usb(struct spl_image_info *spl_image,
			   struct spl_boot_device *bootdev)
{
	bootdev->boot_device_name = "usb_ether";
#如果 CONFIG_IS_ENABLED(DM_USB_GADGET)
	usb_ether_init();
#endif
	返回 spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("USB eth", 0, BOOT_DEVICE_USBETH, spl_net_load_image_usb);
#endif
