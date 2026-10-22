// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for onboard USB hubs
 *
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 *
 * Mostly inspired by Linux kernel v6.1 onboard_usb_hub driver
 */

#شامل <asm/gpio.h>
#شامل <dm.h>
#شامل <dm/device_compat.h>
#شامل <dm/uclass-internal.h>
#شامل <i2c.h>
#شامل <linux/delay.h>
#شامل <power/regulator.h>

#تعریف_کرو USB5744_COMMAND_ATTACH		0x0056
#تعریف_کرو USB5744_COMMAND_ATTACH_LSB	0xAA
#تعریف_کرو USB5744_CONFIG_REG_ACCESS	0x0037
#تعریف_کرو USB5744_CONFIG_REG_ACCESS_LSB	0x99

#تعریف_کرو MAX_SUPPLIES 2

ساخت onboard_hub {
	ساخت udevice *vdd[MAX_SUPPLIES];
	ساخت gpio_desc *reset_gpio;
};

ساخت onboard_hub_ڈیٹا {
	غیرعلامتی طویل reset_us;
	غیرعلامتی طویل power_on_delay_us;
	غیرعلامتی صحیح عدد_supplies;
	ثابت حرف * ثابت supply_نام[MAX_SUPPLIES];
	صحیح (*init)(ساخت udevice *dev);
};

ساکن صحیح usb5744_i2c_init(ساخت udevice *dev)
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
	u8 ڈیٹا_buf[8] = {0x0, 0x5, 0x0, 0x1, 0x41, 0x1D, 0x08};
	u8 config_reg_access_buf = USB5744_CONFIG_REG_ACCESS;
	ساخت udevice *i2c_bus = تہی, *i2c_dev;
	ساخت ofnode_phandle_دلائل phandle;
	u8 buf = USB5744_COMMAND_ATTACH;
	ساخت dm_i2c_chip *i2c_chip;
	صحیح ret, slave_addr;

	ret = dev_read_phandle_with_دلائل(dev, "i2c-bus", تہی, 0, 0, &phandle);
	اگر (ret == -ENOENT) {
		dev_dbg(dev, "i2c-bus not specified\n");
		واپس 0;
	}

	اگر (ret) {
		dev_err(dev, "i2c-bus read failed\n");
		واپس ret;
	}

	ret = device_get_global_by_ofnode(ofnode_get_والد(phandle.گرہ), &i2c_bus);
	اگر (ret) {
		dev_err(dev, "Failed to get i2c node, err: %d\n", ret);
		واپس ret;
	}

	ret = ofnode_read_u32(phandle.گرہ, "reg", &slave_addr);
	اگر (ret)
		واپس ret;

	ret = i2c_get_chip(i2c_bus, slave_addr, 1, &i2c_dev);
	اگر (ret) {
		dev_err(dev, "%s: can't find i2c chip device for addr 0x%x\n", __func__,
			slave_addr);
		واپس ret;
	}

	i2c_chip = dev_get_والد_plat(i2c_dev);
	اگر (!i2c_chip) {
		dev_err(dev, "parent platform data not found\n");
		واپس -EINVAL;
	}

	i2c_chip->flags &= ~DM_I2_C_CHIP_WR_پتہ;
	/* SMBus write command */
	ret = dm_i2c_write(i2c_dev, 0, (uint8_t *)&ڈیٹا_buf, 8);
	اگر (ret) {
		dev_err(dev, "data_buf i2c_write failed, err:%d\n", ret);
		واپس ret;
	}

	/* Configuration register access command */
	ret = dm_i2c_write(i2c_dev, USB5744_CONFIG_REG_ACCESS_LSB,
			   &config_reg_access_buf, 2);
	اگر (ret) {
		dev_err(dev, "config_reg_access i2c_write failed, err: %d\n", ret);
		واپس ret;
	}

	/* USB Attach with SMBus */
	ret = dm_i2c_write(i2c_dev, USB5744_COMMAND_ATTACH_LSB, &buf, 2);
	اگر (ret) {
		dev_err(dev, "usb_attach i2c_write failed, err: %d\n", ret);
		واپس ret;
	}

	واپس 0;
}

صحیح usb_onboard_hub_reset(ساخت udevice *dev)
{
	ساخت onboard_hub_ڈیٹا *ڈیٹا =
		(ساخت onboard_hub_ڈیٹا *)dev_get_driver_ڈیٹا(dev);
	ساخت onboard_hub *hub = dev_get_priv(dev);
	صحیح ret;

	hub->reset_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_IS_برآمد);

	/* property is optional, don't return error! */
	اگر (!hub->reset_gpio)
		واپس 0;

	ret = dm_gpio_سیٹ_قدر(hub->reset_gpio, 1);
	اگر (ret)
		واپس ret;

	udelay(ڈیٹا->reset_us);

	ret = dm_gpio_سیٹ_قدر(hub->reset_gpio, 0);
	اگر (ret)
		واپس ret;

	udelay(ڈیٹا->power_on_delay_us);

	واپس 0;
}

ساکن صحیح usb_onboard_hub_power_off(ساخت udevice *dev)
{
	ساخت onboard_hub_ڈیٹا *ڈیٹا =
		(ساخت onboard_hub_ڈیٹا *)dev_get_driver_ڈیٹا(dev);
	ساخت onboard_hub *hub = dev_get_priv(dev);
	صحیح ret = 0, ret2;
	غیرعلامتی صحیح i;

	برائے (i = ڈیٹا->عدد_supplies; i > 0; i--) {
		اگر (hub->vdd[i-1]) {
			ret2 = regulator_سیٹ_enable_if_allowed(hub->vdd[i-1], false);
			اگر (ret2 && ret2 != -ENOSYS) {
				dev_err(dev, "can't disable %s: %d\n", ڈیٹا->supply_نام[i-1], ret2);
				ret |= ret2;
			}
		}
	}

	واپس ret;
}

ساکن صحیح usb_onboard_hub_probe(ساخت udevice *dev)
{
	ساخت onboard_hub_ڈیٹا *ڈیٹا =
		(ساخت onboard_hub_ڈیٹا *)dev_get_driver_ڈیٹا(dev);
	ساخت onboard_hub *hub = dev_get_priv(dev);
	غیرعلامتی صحیح i;
	صحیح ret;

	اگر (ڈیٹا->عدد_supplies > MAX_SUPPLIES) {
		dev_err(dev, "invalid supplies number, max supported: %d\n", MAX_SUPPLIES);
		واپس -EINVAL;
	}

	برائے (i = 0; i < ڈیٹا->عدد_supplies; i++) {
		ret = device_get_supply_regulator(dev, ڈیٹا->supply_نام[i], &hub->vdd[i]);
		اگر (ret && ret != -ENOENT && ret != -ENOSYS) {
			dev_err(dev, "can't get %s: %d\n", ڈیٹا->supply_نام[i], ret);
			جاؤ err_supply;
		}

		اگر (hub->vdd[i]) {
			ret = regulator_سیٹ_enable_if_allowed(hub->vdd[i], true);
			اگر (ret && ret != -ENOSYS) {
				dev_err(dev, "can't enable %s: %d\n", ڈیٹا->supply_نام[i], ret);
				جاؤ err_supply;
			}
		}
	}

	ret = usb_onboard_hub_reset(dev);
	اگر (ret)
		جاؤ err_supply;

	اگر (ڈیٹا->init) {
		ret = ڈیٹا->init(dev);
		اگر (ret) {
			dev_err(dev, "onboard i2c init failed: %d\n", ret);
			جاؤ err;
		}
	}
	واپس 0;
err:
	dm_gpio_سیٹ_قدر(hub->reset_gpio, 0);
err_supply:
	usb_onboard_hub_power_off(dev);
	واپس ret;
}

ساکن صحیح usb_onboard_hub_bind(ساخت udevice *dev)
{
	ساخت ofnode_phandle_دلائل phandle;
	ساخت udevice *peerdev;
	صحیح ret;

	ret = dev_read_phandle_with_دلائل(dev, "peer-hub", تہی, 0, 0, &phandle);
	اگر (ret == -ENOENT) {
		dev_dbg(dev, "peer-hub property not present\n");
		واپس 0;
	}

	اگر (ret) {
		dev_err(dev, "peer-hub not specified\n");
		واپس ret;
	}

	ret = uclass_ڈھونڈو_device_by_ofnode(UCLASS_USB_HUB, phandle.گرہ, &peerdev);
	اگر (ret) {
		dev_dbg(dev, "binding before peer-hub %s\n",
			ofnode_get_name(phandle.گرہ));
		واپس 0;
	}

	dev_dbg(dev, "peer-hub %s has been bound\n", peerdev->name);

	واپس -ENODEV;
}

ساکن صحیح usb_onboard_hub_remove(ساخت udevice *dev)
{
	ساخت onboard_hub *hub = dev_get_priv(dev);
	صحیح ret = 0;

	اگر (hub->reset_gpio) {
		ret = dm_gpio_سیٹ_قدر(hub->reset_gpio, 1);
		اگر (ret)
			dev_err(dev, "can't set gpio %s: %d\n", hub->reset_gpio->dev->name,
				ret);
	}

	ret |= usb_onboard_hub_power_off(dev);
	واپس ret;
}

ساکن ثابت ساخت onboard_hub_ڈیٹا corechips_sl6341_ڈیٹا = {
	.reset_us = 10000,
	.عدد_supplies = 2,
	.supply_نام = { "vdd1v1-supply", "vdd3v3-supply" },
};

ساکن ثابت ساخت onboard_hub_ڈیٹا usb2514_ڈیٹا = {
	.power_on_delay_us = 500,
	.reset_us = 1,
	.عدد_supplies = 1,
	.supply_نام = { "vdd-supply" },
};

ساکن ثابت ساخت onboard_hub_ڈیٹا usb5744_ڈیٹا = {
	.init = usb5744_i2c_init,
	.power_on_delay_us = 1000,
	.reset_us = 5,
	.عدد_supplies = 1,
	.supply_نام = { "vdd-supply" },
};

ساکن ثابت ساخت onboard_hub_ڈیٹا usbhx3_ڈیٹا = {
	.reset_us = 10000,
	.عدد_supplies = 2,
	.supply_نام = { "vdd-supply", "vdd2-supply" },
};

ساکن ثابت ساخت udevice_id usb_onboard_hub_ids[] = {
	/* Use generic usbVID,PID dt-bindings (usb-device.yaml) */
	{	.compatible = "usb3431,6241",	/* Corechips SL6341 USB 2.0 */
		.ڈیٹا = (ulong)&corechips_sl6341_ڈیٹا,
	}, {
		.compatible = "usb3431,6341",	/* Corechips SL6341 USB 3.0 */
		.ڈیٹا = (ulong)&corechips_sl6341_ڈیٹا,
	}, {
		.compatible = "usb424,2514",	/* USB2514B USB 2.0 */
		.ڈیٹا = (ulong)&usb2514_ڈیٹا,
	}, {
		.compatible = "usb424,2744",	/* USB2744 USB 2.0 */
		.ڈیٹا = (ulong)&usb5744_ڈیٹا,
	}, {
		.compatible = "usb424,5744",	/* USB5744 USB 3.0 */
		.ڈیٹا = (ulong)&usb5744_ڈیٹا,
	}, {
		.compatible = "usb4b4,6504",	/* Cypress HX3 USB 3.0 */
		.ڈیٹا = (ulong)&usbhx3_ڈیٹا,
	}, {
		.compatible = "usb4b4,6506",	/* Cypress HX3 USB 2.0 */
		.ڈیٹا = (ulong)&usbhx3_ڈیٹا,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(usb_onboard_hub) = {
	.name	= "usb_onboard_hub",
	.id	= UCLASS_USB_HUB,
	.bind   = usb_onboard_hub_bind,
	.probe = usb_onboard_hub_probe,
	.remove = usb_onboard_hub_remove,
	.کا_مطابقت = usb_onboard_hub_ids,
	.priv_auto = حجم(ساخت onboard_hub),
};
