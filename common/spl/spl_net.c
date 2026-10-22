// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2012
 * Ilya Yanok <ilya.yanok@gmail.com>
 */
#समावेश <env.h>
#समावेश <errno.h>
#समावेश <image.h>
#समावेश <log.h>
#समावेश <spl.h>
#समावेश <spl_load.h>
#समावेश <net.h>
#समावेश <linux/libfdt.h>

#अगर defined(CONFIG_SPL_ETH) || defined(CONFIG_SPL_USB_ETHER)
अटल ulong spl_net_load_read(काष्ठा spl_load_info *load, ulong sector,
			       ulong गिनती, व्योम *buf)
{
	debug("%s: sector %lx, count %lx, buf %lx\n",
	      __func__, sector, गिनती, (ulong)buf);
	स_नकल(buf, map_sysmem(image_load_addr + sector, गिनती), गिनती);
	वापस गिनती;
}

अटल पूर्णांक spl_net_load_image(काष्ठा spl_image_info *spl_image,
			      काष्ठा spl_boot_device *bootdev)
{
	काष्ठा spl_load_info load;
	पूर्णांक rv;

	env_init();
	env_relocate();
	env_set("autoload", "yes");
	rv = eth_initialize();
	अगर (rv == 0) {
		म_लिखो("No Ethernet devices found\n");
		वापस -ENODEV;
	}
	अगर (bootdev->boot_device_name)
		env_set("ethact", bootdev->boot_device_name);
	rv = net_loop(BOOTP);
	अगर (rv < 0) {
		म_लिखो("Problem booting with BOOTP\n");
		वापस rv;
	}

	spl_load_init(&load, spl_net_load_read, शून्य, 1);
	वापस spl_load(spl_image, bootdev, &load, 0, 0);
}
#पूर्ण_अगर

#अगर_घोषित CONFIG_SPL_ETH
पूर्णांक spl_net_load_image_cpgmac(काष्ठा spl_image_info *spl_image,
			      काष्ठा spl_boot_device *bootdev)
{
#अगर_घोषित CONFIG_SPL_ETH_DEVICE
	bootdev->boot_device_name = CONFIG_SPL_ETH_DEVICE;
#पूर्ण_अगर

	वापस spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("eth device", 0, BOOT_DEVICE_CPGMAC,
		      spl_net_load_image_cpgmac);
#पूर्ण_अगर

#अगर_घोषित CONFIG_SPL_USB_ETHER
पूर्णांक spl_net_load_image_usb(काष्ठा spl_image_info *spl_image,
			   काष्ठा spl_boot_device *bootdev)
{
	bootdev->boot_device_name = "usb_ether";
#अगर CONFIG_IS_ENABLED(DM_USB_GADGET)
	usb_ether_init();
#पूर्ण_अगर
	वापस spl_net_load_image(spl_image, bootdev);
}
SPL_LOAD_IMAGE_METHOD("USB eth", 0, BOOT_DEVICE_USBETH, spl_net_load_image_usb);
#पूर्ण_अगर
