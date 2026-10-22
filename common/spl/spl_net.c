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

#အကယ်၍ defined(CONFIG_SPL_ETH) || defined(CONFIG_SPL_USB_ETHER)
static ulong spl_net_load_read(sa_nracanaa spl_load_info *load, ulong sector,
			       ulong count, void *buf)
{
	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, count, (ulong)buf);
	memcpy(buf, map_sysmem(image_load_addr + sector, count), count);
	ပြန်ပေး count;
}

static ကိန်းပြည့် spl_net_load_image(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev)
{
	sa_nracanaa spl_load_info load;
	ကိန်းပြည့် rv;

	env_init();
	env_relocate();
	env_set("autoload", "yes");
	rv = eth_initialize();
	အကယ်၍ (rv == 0) {
		printf("No Ethernet devices found\n");
		ပြန်ပေး -ENODEV;
	}
	အကယ်၍ (bootdev->boot_device_name)
		env_set("ethact", bootdev->boot_device_name);
	rv = net_loop(BOOTP);
	အကယ်၍ (rv < 0) {
		printf("Problem booting with BOOTP\n");
		ပြန်ပေး rv;
	}

	spl_load_init(&load, spl_net_load_read, NULL, 1);
	ပြန်ပေး spl_load(spl_image, bootdev, &load, 0, 0);
}
#endif

#ifdef CONFIG_SPL_ETH
ကိန်းပြည့် spl_net_load_image_cpgmac(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev)
{
#ifdef CONFIG_SPL_ETH_DEVICE
	bootdev->boot_device_name = CONFIG_SPL_ETH_DEVICE;
#endif

	ပြန်ပေး spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("eth device", 0, BOOT_DEVICE_CPGMAC,
		      spl_net_load_image_cpgmac);
#endif

#ifdef CONFIG_SPL_USB_ETHER
ကိန်းပြည့် spl_net_load_image_usb(sa_nracanaa spl_image_info *spl_image,
			   sa_nracanaa spl_boot_device *bootdev)
{
	bootdev->boot_device_name = "usb_ether";
#အကယ်၍ CONFIG_IS_ENABLED(DM_USB_GADGET)
	usb_ether_init();
#endif
	ပြန်ပေး spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("USB eth", 0, BOOT_DEVICE_USBETH, spl_net_load_image_usb);
#endif
