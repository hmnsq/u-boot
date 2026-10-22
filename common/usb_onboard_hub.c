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

সংরচনা onboard_hub {
	সংরচনা udevice *vdd[MAX_SUPPLIES];
	সংরচনা gpio_desc *reset_gpio;
};

সংরচনা onboard_hub_data {
	unsigned long reset_us;
	unsigned long power_on_delay_us;
	unsigned পূর্ণাঙ্ক অঙ্ক_supplies;
	ধ্রুব বর্ণ * ধ্রুব supply_names[MAX_SUPPLIES];
	পূর্ণাঙ্ক (*init)(সংরচনা udevice *dev);
};

স্থির পূর্ণাঙ্ক usb5744_i2c_init(সংরচনা udevice *dev)
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
	u8 data_buf[8] = {0x0, 0x5, 0x0, 0x1, 0x41, 0x1D, 0x08};
	u8 config_reg_access_buf = USB5744_CONFIG_REG_ACCESS;
	সংরচনা udevice *i2c_bus = রিক্ত, *i2c_dev;
	সংরচনা ofnode_phandle_args phandle;
	u8 buf = USB5744_COMMAND_ATTACH;
	সংরচনা dm_i2c_chip *i2c_chip;
	পূর্ণাঙ্ক ret, slave_addr;

	ret = dev_read_phandle_with_args(dev, "i2c-bus", রিক্ত, 0, 0, &phandle);
	যদি (ret == -ENOENT) {
		dev_dbg(dev, "i2c-bus not specified\n");
		প্রত্যাগম 0;
	}

	যদি (ret) {
		dev_err(dev, "i2c-bus read failed\n");
		প্রত্যাগম ret;
	}

	ret = device_get_global_by_ofnode(ofnode_get_parent(phandle.গ্রন্থি), &i2c_bus);
	যদি (ret) {
		dev_err(dev, "Failed to get i2c node, err: %d\n", ret);
		প্রত্যাগম ret;
	}

	ret = ofnode_read_u32(phandle.গ্রন্থি, "reg", &slave_addr);
	যদি (ret)
		প্রত্যাগম ret;

	ret = i2c_get_chip(i2c_bus, slave_addr, 1, &i2c_dev);
	যদি (ret) {
		dev_err(dev, "%s: can't find i2c chip device for addr 0x%x\n", __func__,
			slave_addr);
		প্রত্যাগম ret;
	}

	i2c_chip = dev_get_parent_plat(i2c_dev);
	যদি (!i2c_chip) {
		dev_err(dev, "parent platform data not found\n");
		প্রত্যাগম -EINVAL;
	}

	i2c_chip->flags &= ~DM_I2C_CHIP_WR_ADDRESS;
	/* SMBus write command */
	ret = dm_i2c_write(i2c_dev, 0, (uint8_t *)&data_buf, 8);
	যদি (ret) {
		dev_err(dev, "data_buf i2c_write failed, err:%d\n", ret);
		প্রত্যাগম ret;
	}

	/* Configuration register access command */
	ret = dm_i2c_write(i2c_dev, USB5744_CONFIG_REG_ACCESS_LSB,
			   &config_reg_access_buf, 2);
	যদি (ret) {
		dev_err(dev, "config_reg_access i2c_write failed, err: %d\n", ret);
		প্রত্যাগম ret;
	}

	/* USB Attach with SMBus */
	ret = dm_i2c_write(i2c_dev, USB5744_COMMAND_ATTACH_LSB, &buf, 2);
	যদি (ret) {
		dev_err(dev, "usb_attach i2c_write failed, err: %d\n", ret);
		প্রত্যাগম ret;
	}

	প্রত্যাগম 0;
}

পূর্ণাঙ্ক usb_onboard_hub_reset(সংরচনা udevice *dev)
{
	সংরচনা onboard_hub_data *data =
		(সংরচনা onboard_hub_data *)dev_get_driver_data(dev);
	সংরচনা onboard_hub *hub = dev_get_priv(dev);
	পূর্ণাঙ্ক ret;

	hub->reset_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_IS_OUT);

	/* property is optional, don't return error! */
	যদি (!hub->reset_gpio)
		প্রত্যাগম 0;

	ret = dm_gpio_set_মান(hub->reset_gpio, 1);
	যদি (ret)
		প্রত্যাগম ret;

	udelay(data->reset_us);

	ret = dm_gpio_set_মান(hub->reset_gpio, 0);
	যদি (ret)
		প্রত্যাগম ret;

	udelay(data->power_on_delay_us);

	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক usb_onboard_hub_power_off(সংরচনা udevice *dev)
{
	সংরচনা onboard_hub_data *data =
		(সংরচনা onboard_hub_data *)dev_get_driver_data(dev);
	সংরচনা onboard_hub *hub = dev_get_priv(dev);
	পূর্ণাঙ্ক ret = 0, ret2;
	unsigned পূর্ণাঙ্ক i;

	প্রতি (i = data->অঙ্ক_supplies; i > 0; i--) {
		যদি (hub->vdd[i-1]) {
			ret2 = regulator_set_enable_if_allowed(hub->vdd[i-1], false);
			যদি (ret2 && ret2 != -ENOSYS) {
				dev_err(dev, "can't disable %s: %d\n", data->supply_names[i-1], ret2);
				ret |= ret2;
			}
		}
	}

	প্রত্যাগম ret;
}

স্থির পূর্ণাঙ্ক usb_onboard_hub_probe(সংরচনা udevice *dev)
{
	সংরচনা onboard_hub_data *data =
		(সংরচনা onboard_hub_data *)dev_get_driver_data(dev);
	সংরচনা onboard_hub *hub = dev_get_priv(dev);
	unsigned পূর্ণাঙ্ক i;
	পূর্ণাঙ্ক ret;

	যদি (data->অঙ্ক_supplies > MAX_SUPPLIES) {
		dev_err(dev, "invalid supplies number, max supported: %d\n", MAX_SUPPLIES);
		প্রত্যাগম -EINVAL;
	}

	প্রতি (i = 0; i < data->অঙ্ক_supplies; i++) {
		ret = device_get_supply_regulator(dev, data->supply_names[i], &hub->vdd[i]);
		যদি (ret && ret != -ENOENT && ret != -ENOSYS) {
			dev_err(dev, "can't get %s: %d\n", data->supply_names[i], ret);
			গচ্ছ err_supply;
		}

		যদি (hub->vdd[i]) {
			ret = regulator_set_enable_if_allowed(hub->vdd[i], true);
			যদি (ret && ret != -ENOSYS) {
				dev_err(dev, "can't enable %s: %d\n", data->supply_names[i], ret);
				গচ্ছ err_supply;
			}
		}
	}

	ret = usb_onboard_hub_reset(dev);
	যদি (ret)
		গচ্ছ err_supply;

	যদি (data->init) {
		ret = data->init(dev);
		যদি (ret) {
			dev_err(dev, "onboard i2c init failed: %d\n", ret);
			গচ্ছ err;
		}
	}
	প্রত্যাগম 0;
err:
	dm_gpio_set_মান(hub->reset_gpio, 0);
err_supply:
	usb_onboard_hub_power_off(dev);
	প্রত্যাগম ret;
}

স্থির পূর্ণাঙ্ক usb_onboard_hub_bind(সংরচনা udevice *dev)
{
	সংরচনা ofnode_phandle_args phandle;
	সংরচনা udevice *peerdev;
	পূর্ণাঙ্ক ret;

	ret = dev_read_phandle_with_args(dev, "peer-hub", রিক্ত, 0, 0, &phandle);
	যদি (ret == -ENOENT) {
		dev_dbg(dev, "peer-hub property not present\n");
		প্রত্যাগম 0;
	}

	যদি (ret) {
		dev_err(dev, "peer-hub not specified\n");
		প্রত্যাগম ret;
	}

	ret = uclass_find_device_by_ofnode(UCLASS_USB_HUB, phandle.গ্রন্থি, &peerdev);
	যদি (ret) {
		dev_dbg(dev, "binding before peer-hub %s\n",
			ofnode_get_name(phandle.গ্রন্থি));
		প্রত্যাগম 0;
	}

	dev_dbg(dev, "peer-hub %s has been bound\n", peerdev->name);

	প্রত্যাগম -ENODEV;
}

স্থির পূর্ণাঙ্ক usb_onboard_hub_remove(সংরচনা udevice *dev)
{
	সংরচনা onboard_hub *hub = dev_get_priv(dev);
	পূর্ণাঙ্ক ret = 0;

	যদি (hub->reset_gpio) {
		ret = dm_gpio_set_মান(hub->reset_gpio, 1);
		যদি (ret)
			dev_err(dev, "can't set gpio %s: %d\n", hub->reset_gpio->dev->name,
				ret);
	}

	ret |= usb_onboard_hub_power_off(dev);
	প্রত্যাগম ret;
}

স্থির ধ্রুব সংরচনা onboard_hub_data corechips_sl6341_data = {
	.reset_us = 10000,
	.অঙ্ক_supplies = 2,
	.supply_names = { "vdd1v1-supply", "vdd3v3-supply" },
};

স্থির ধ্রুব সংরচনা onboard_hub_data usb2514_data = {
	.power_on_delay_us = 500,
	.reset_us = 1,
	.অঙ্ক_supplies = 1,
	.supply_names = { "vdd-supply" },
};

স্থির ধ্রুব সংরচনা onboard_hub_data usb5744_data = {
	.init = usb5744_i2c_init,
	.power_on_delay_us = 1000,
	.reset_us = 5,
	.অঙ্ক_supplies = 1,
	.supply_names = { "vdd-supply" },
};

স্থির ধ্রুব সংরচনা onboard_hub_data usbhx3_data = {
	.reset_us = 10000,
	.অঙ্ক_supplies = 2,
	.supply_names = { "vdd-supply", "vdd2-supply" },
};

স্থির ধ্রুব সংরচনা udevice_id usb_onboard_hub_ids[] = {
	/* Use generic usbVID,PID dt-bindings (usb-device.yaml) */
	{	.compatible = "usb3431,6241",	/* Corechips SL6341 USB 2.0 */
		.data = (ulong)&corechips_sl6341_data,
	}, {
		.compatible = "usb3431,6341",	/* Corechips SL6341 USB 3.0 */
		.data = (ulong)&corechips_sl6341_data,
	}, {
		.compatible = "usb424,2514",	/* USB2514B USB 2.0 */
		.data = (ulong)&usb2514_data,
	}, {
		.compatible = "usb424,2744",	/* USB2744 USB 2.0 */
		.data = (ulong)&usb5744_data,
	}, {
		.compatible = "usb424,5744",	/* USB5744 USB 3.0 */
		.data = (ulong)&usb5744_data,
	}, {
		.compatible = "usb4b4,6504",	/* Cypress HX3 USB 3.0 */
		.data = (ulong)&usbhx3_data,
	}, {
		.compatible = "usb4b4,6506",	/* Cypress HX3 USB 2.0 */
		.data = (ulong)&usbhx3_data,
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
	.priv_auto = পরিমাণ(সংরচনা onboard_hub),
};
