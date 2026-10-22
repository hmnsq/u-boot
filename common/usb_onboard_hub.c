// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for onboard USB hubs
 *
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 *
 * Mostly inspired by Linux kernel v6.1 onboard_usb_hub driver
 */

#অন্তর্ভাব <asm/gpio.h>
#অন্তর্ভাব <dm.h>
#অন্তর্ভাব <dm/device_compat.h>
#অন্তর্ভাব <dm/uclass-internal.h>
#অন্তর্ভাব <i2c.h>
#অন্তর্ভাব <linux/delay.h>
#অন্তর্ভাব <power/regulator.h>

#define USB5744_COMMAND_ATTACH		0x0056
#define USB5744_COMMAND_ATTACH_LSB	0xAA
#define USB5744_CONFIG_REG_ACCESS	0x0037
#define USB5744_CONFIG_REG_ACCESS_LSB	0x99

#define MAX_SUPPLIES 2

কাঠামো onboard_hub {
	কাঠামো udevice *vdd[MAX_SUPPLIES];
	কাঠামো gpio_desc *reset_gpio;
};

কাঠামো onboard_hub_তথ্য {
	unsigned long reset_us;
	unsigned long power_on_delay_us;
	unsigned পূর্ণসংখ্যা অঙ্ক_supplies;
	ধ্রুবক অক্ষর * ধ্রুবক supply_names[MAX_SUPPLIES];
	পূর্ণসংখ্যা (*init)(কাঠামো udevice *dev);
};

স্থির পূর্ণসংখ্যা usb5744_i2c_init(কাঠামো udevice *dev)
{
	/*
	 *  Prevent the MCU from the putting the HUB in suspend mode through register write.
	 *  The BYPASS_UDC_SUSPEND bit (Bit 3) of the RuntimeFlags2 register at address
	 *  0x411D controls this aspect of the hub.
	 *  Format to write to hub registers via SMBus- 2D 00 00 05 00 01 41 1D 08
	 *  Byte 0: Address of slave 2D
	 *  Byte 1: Memory address 00
	 *  Byte 2: Memory address 00
	 *  Byte 3: Number of bytes to write to memory
	 *  Byte 4: Write configuration register (00)
	 *  Byte 5: Write the number of data bytes (01- 1 data byte)
	 *  Byte 6: LSB of register address 0x41
	 *  Byte 7: MSB of register address 0x1D
	 *  Byte 8: value to be written to the register
	 */
	u8 তথ্য_buf[8] = {0x0, 0x5, 0x0, 0x1, 0x41, 0x1D, 0x08};
	u8 config_reg_access_buf = USB5744_CONFIG_REG_ACCESS;
	কাঠামো udevice *i2c_bus = ফাঁকা, *i2c_dev;
	কাঠামো ofnode_phandle_args phandle;
	u8 buf = USB5744_COMMAND_ATTACH;
	কাঠামো dm_i2c_chip *i2c_chip;
	পূর্ণসংখ্যা ret, slave_addr;

	ret = dev_read_phandle_with_args(dev, "i2c-bus", ফাঁকা, 0, 0, &phandle);
	যদি (ret == -ENOENT) {
		dev_dbg(dev, "i2c-bus not specified\n");
		ফেরত 0;
	}

	যদি (ret) {
		dev_err(dev, "i2c-bus read failed\n");
		ফেরত ret;
	}

	ret = device_get_global_by_ofnode(ofnode_get_parent(phandle.গ্রন্থি), &i2c_bus);
	যদি (ret) {
		dev_err(dev, "Failed to get i2c node, err: %d\n", ret);
		ফেরত ret;
	}

	ret = ofnode_read_u32(phandle.গ্রন্থি, "reg", &slave_addr);
	যদি (ret)
		ফেরত ret;

	ret = i2c_get_chip(i2c_bus, slave_addr, 1, &i2c_dev);
	যদি (ret) {
		dev_err(dev, "%s: can't find i2c chip device for addr 0x%x\n", __func__,
			slave_addr);
		ফেরত ret;
	}

	i2c_chip = dev_get_parent_plat(i2c_dev);
	যদি (!i2c_chip) {
		dev_err(dev, "parent platform data not found\n");
		ফেরত -EINVAL;
	}

	i2c_chip->flags &= ~DM_I2C_CHIP_WR_ADDRESS;
	/* SMBus write command */
	ret = dm_i2c_write(i2c_dev, 0, (uint8_t *)&তথ্য_buf, 8);
	যদি (ret) {
		dev_err(dev, "data_buf i2c_write failed, err:%d\n", ret);
		ফেরত ret;
	}

	/* Configuration register access command */
	ret = dm_i2c_write(i2c_dev, USB5744_CONFIG_REG_ACCESS_LSB,
			   &config_reg_access_buf, 2);
	যদি (ret) {
		dev_err(dev, "config_reg_access i2c_write failed, err: %d\n", ret);
		ফেরত ret;
	}

	/* USB Attach with SMBus */
	ret = dm_i2c_write(i2c_dev, USB5744_COMMAND_ATTACH_LSB, &buf, 2);
	যদি (ret) {
		dev_err(dev, "usb_attach i2c_write failed, err: %d\n", ret);
		ফেরত ret;
	}

	ফেরত 0;
}

পূর্ণসংখ্যা usb_onboard_hub_reset(কাঠামো udevice *dev)
{
	কাঠামো onboard_hub_তথ্য *তথ্য =
		(কাঠামো onboard_hub_তথ্য *)dev_get_driver_তথ্য(dev);
	কাঠামো onboard_hub *hub = dev_get_priv(dev);
	পূর্ণসংখ্যা ret;

	hub->reset_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_IS_OUT);

	/* property is optional, don't return error! */
	যদি (!hub->reset_gpio)
		ফেরত 0;

	ret = dm_gpio_set_মান(hub->reset_gpio, 1);
	যদি (ret)
		ফেরত ret;

	udelay(তথ্য->reset_us);

	ret = dm_gpio_set_মান(hub->reset_gpio, 0);
	যদি (ret)
		ফেরত ret;

	udelay(তথ্য->power_on_delay_us);

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা usb_onboard_hub_power_off(কাঠামো udevice *dev)
{
	কাঠামো onboard_hub_তথ্য *তথ্য =
		(কাঠামো onboard_hub_তথ্য *)dev_get_driver_তথ্য(dev);
	কাঠামো onboard_hub *hub = dev_get_priv(dev);
	পূর্ণসংখ্যা ret = 0, ret2;
	unsigned পূর্ণসংখ্যা i;

	জন্য (i = তথ্য->অঙ্ক_supplies; i > 0; i--) {
		যদি (hub->vdd[i-1]) {
			ret2 = regulator_set_enable_if_allowed(hub->vdd[i-1], false);
			যদি (ret2 && ret2 != -ENOSYS) {
				dev_err(dev, "can't disable %s: %d\n", তথ্য->supply_names[i-1], ret2);
				ret |= ret2;
			}
		}
	}

	ফেরত ret;
}

স্থির পূর্ণসংখ্যা usb_onboard_hub_probe(কাঠামো udevice *dev)
{
	কাঠামো onboard_hub_তথ্য *তথ্য =
		(কাঠামো onboard_hub_তথ্য *)dev_get_driver_তথ্য(dev);
	কাঠামো onboard_hub *hub = dev_get_priv(dev);
	unsigned পূর্ণসংখ্যা i;
	পূর্ণসংখ্যা ret;

	যদি (তথ্য->অঙ্ক_supplies > MAX_SUPPLIES) {
		dev_err(dev, "invalid supplies number, max supported: %d\n", MAX_SUPPLIES);
		ফেরত -EINVAL;
	}

	জন্য (i = 0; i < তথ্য->অঙ্ক_supplies; i++) {
		ret = device_get_supply_regulator(dev, তথ্য->supply_names[i], &hub->vdd[i]);
		যদি (ret && ret != -ENOENT && ret != -ENOSYS) {
			dev_err(dev, "can't get %s: %d\n", তথ্য->supply_names[i], ret);
			যাও err_supply;
		}

		যদি (hub->vdd[i]) {
			ret = regulator_set_enable_if_allowed(hub->vdd[i], true);
			যদি (ret && ret != -ENOSYS) {
				dev_err(dev, "can't enable %s: %d\n", তথ্য->supply_names[i], ret);
				যাও err_supply;
			}
		}
	}

	ret = usb_onboard_hub_reset(dev);
	যদি (ret)
		যাও err_supply;

	যদি (তথ্য->init) {
		ret = তথ্য->init(dev);
		যদি (ret) {
			dev_err(dev, "onboard i2c init failed: %d\n", ret);
			যাও err;
		}
	}
	ফেরত 0;
err:
	dm_gpio_set_মান(hub->reset_gpio, 0);
err_supply:
	usb_onboard_hub_power_off(dev);
	ফেরত ret;
}

স্থির পূর্ণসংখ্যা usb_onboard_hub_bind(কাঠামো udevice *dev)
{
	কাঠামো ofnode_phandle_args phandle;
	কাঠামো udevice *peerdev;
	পূর্ণসংখ্যা ret;

	ret = dev_read_phandle_with_args(dev, "peer-hub", ফাঁকা, 0, 0, &phandle);
	যদি (ret == -ENOENT) {
		dev_dbg(dev, "peer-hub property not present\n");
		ফেরত 0;
	}

	যদি (ret) {
		dev_err(dev, "peer-hub not specified\n");
		ফেরত ret;
	}

	ret = uclass_find_device_by_ofnode(UCLASS_USB_HUB, phandle.গ্রন্থি, &peerdev);
	যদি (ret) {
		dev_dbg(dev, "binding before peer-hub %s\n",
			ofnode_get_name(phandle.গ্রন্থি));
		ফেরত 0;
	}

	dev_dbg(dev, "peer-hub %s has been bound\n", peerdev->name);

	ফেরত -ENODEV;
}

স্থির পূর্ণসংখ্যা usb_onboard_hub_remove(কাঠামো udevice *dev)
{
	কাঠামো onboard_hub *hub = dev_get_priv(dev);
	পূর্ণসংখ্যা ret = 0;

	যদি (hub->reset_gpio) {
		ret = dm_gpio_set_মান(hub->reset_gpio, 1);
		যদি (ret)
			dev_err(dev, "can't set gpio %s: %d\n", hub->reset_gpio->dev->name,
				ret);
	}

	ret |= usb_onboard_hub_power_off(dev);
	ফেরত ret;
}

স্থির ধ্রুবক কাঠামো onboard_hub_তথ্য corechips_sl6341_তথ্য = {
	.reset_us = 10000,
	.অঙ্ক_supplies = 2,
	.supply_names = { "vdd1v1-supply", "vdd3v3-supply" },
};

স্থির ধ্রুবক কাঠামো onboard_hub_তথ্য usb2514_তথ্য = {
	.power_on_delay_us = 500,
	.reset_us = 1,
	.অঙ্ক_supplies = 1,
	.supply_names = { "vdd-supply" },
};

স্থির ধ্রুবক কাঠামো onboard_hub_তথ্য usb5744_তথ্য = {
	.init = usb5744_i2c_init,
	.power_on_delay_us = 1000,
	.reset_us = 5,
	.অঙ্ক_supplies = 1,
	.supply_names = { "vdd-supply" },
};

স্থির ধ্রুবক কাঠামো onboard_hub_তথ্য usbhx3_তথ্য = {
	.reset_us = 10000,
	.অঙ্ক_supplies = 2,
	.supply_names = { "vdd-supply", "vdd2-supply" },
};

স্থির ধ্রুবক কাঠামো udevice_id usb_onboard_hub_ids[] = {
	/* Use generic usbVID,PID dt-bindings (usb-device.yaml) */
	{	.compatible = "usb3431,6241",	/* Corechips SL6341 USB 2.0 */
		.তথ্য = (ulong)&corechips_sl6341_তথ্য,
	}, {
		.compatible = "usb3431,6341",	/* Corechips SL6341 USB 3.0 */
		.তথ্য = (ulong)&corechips_sl6341_তথ্য,
	}, {
		.compatible = "usb424,2514",	/* USB2514B USB 2.0 */
		.তথ্য = (ulong)&usb2514_তথ্য,
	}, {
		.compatible = "usb424,2744",	/* USB2744 USB 2.0 */
		.তথ্য = (ulong)&usb5744_তথ্য,
	}, {
		.compatible = "usb424,5744",	/* USB5744 USB 3.0 */
		.তথ্য = (ulong)&usb5744_তথ্য,
	}, {
		.compatible = "usb4b4,6504",	/* Cypress HX3 USB 3.0 */
		.তথ্য = (ulong)&usbhx3_তথ্য,
	}, {
		.compatible = "usb4b4,6506",	/* Cypress HX3 USB 2.0 */
		.তথ্য = (ulong)&usbhx3_তথ্য,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(usb_onboard_hub) = {
	.name	= "usb_onboard_hub",
	.id	= UCLASS_USB_HUB,
	.bind   = usb_onboard_hub_bind,
	.probe = usb_onboard_hub_probe,
	.remove = usb_onboard_hub_remove,
	.of_match = usb_onboard_hub_ids,
	.priv_auto = আকার(কাঠামো onboard_hub),
};
