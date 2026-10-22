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

#якщо defined(CONFIG_SPL_ETH) || defined(CONFIG_SPL_USB_ETHER)
static ulong spl_net_load_read(struct spl_load_info *load, ulong sector,
			       ulong count, void *buf)
{
	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, count, (ulong)buf);
	memcpy(buf, map_sysmem(image_load_addr + sector, count), count);
	повернути count;
}

static ціле spl_net_load_image(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev)
{
	struct spl_load_info load;
	ціле rv;

	env_init();
	env_relocate();
	env_set("autoload", "yes");
	rv = eth_initialize();
	якщо (rv == 0) {
		printf("No Ethernet devices found\n");
		повернути -ENODEV;
	}
	якщо (bootdev->boot_device_name)
		env_set("ethact", bootdev->boot_device_name);
	rv = net_loop(BOOTP);
	якщо (rv < 0) {
		printf("Problem booting with BOOTP\n");
		повернути rv;
	}

	spl_load_init(&load, spl_net_load_read, NULL, 1);
	повернути spl_load(spl_image, bootdev, &load, 0, 0);
}
#endif

#ifdef CONFIG_SPL_ETH
ціле spl_net_load_image_cpgmac(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev)
{
#ifdef CONFIG_SPL_ETH_DEVICE
	bootdev->boot_device_name = CONFIG_SPL_ETH_DEVICE;
#endif

	повернути spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("eth device", 0, BOOT_DEVICE_CPGMAC,
		      spl_net_load_image_cpgmac);
#endif

#ifdef CONFIG_SPL_USB_ETHER
ціле spl_net_load_image_usb(struct spl_image_info *spl_image,
			   struct spl_boot_device *bootdev)
{
	bootdev->boot_device_name = "usb_ether";
#якщо CONFIG_IS_ENABLED(DM_USB_GADGET)
	usb_ether_init();
#endif
	повернути spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("USB eth", 0, BOOT_DEVICE_USBETH, spl_net_load_image_usb);
#endif
