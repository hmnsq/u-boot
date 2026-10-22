// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for onboard USB hubs
 *
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 *
 * Mostly inspired by Linux kernel v6.1 onboard_usb_hub driver
 */

#अन्तर्भाव <asm/gpio.h>
#अन्तर्भाव <dm.h>
#अन्तर्भाव <dm/device_compat.h>
#अन्तर्भाव <dm/uclass-internal.h>
#अन्तर्भाव <i2c.h>
#अन्तर्भाव <linux/delay.h>
#अन्तर्भाव <power/regulator.h>

#define USB5744_COMMAND_ATTACH		0x0056
#define USB5744_COMMAND_ATTACH_LSB	0xAA
#define USB5744_CONFIG_REG_ACCESS	0x0037
#define USB5744_CONFIG_REG_ACCESS_LSB	0x99

#define MAX_SUPPLIES 2

संरचना onboard_hub {
	संरचना udevice *vdd[MAX_SUPPLIES];
	संरचना gpio_desc *reset_gpio;
};

संरचना onboard_hub_data {
	अचिन्हित दीर्घ reset_us;
	अचिन्हित दीर्घ power_on_delay_us;
	अचिन्हित पूर्णाङ्क अङ्क_supplies;
	अचर वर्ण * अचर supply_names[MAX_SUPPLIES];
	पूर्णाङ्क (*init)(संरचना udevice *dev);
};

स्थिर पूर्णाङ्क usb5744_i2c_init(संरचना udevice *dev)
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
	संरचना udevice *i2c_bus = रिक्त, *i2c_dev;
	संरचना ofnode_phandle_args phandle;
	u8 buf = USB5744_COMMAND_ATTACH;
	संरचना dm_i2c_chip *i2c_chip;
	पूर्णाङ्क ret, slave_addr;

	ret = dev_read_phandle_with_args(dev, "i2c-bus", रिक्त, 0, 0, &phandle);
	यदि (ret == -ENOENT) {
		dev_dbg(dev, "i2c-bus not specified\n");
		फर्काऊ 0;
	}

	यदि (ret) {
		dev_err(dev, "i2c-bus read failed\n");
		फर्काऊ ret;
	}

	ret = device_get_global_by_ofnode(ofnode_get_parent(phandle.गाँठो), &i2c_bus);
	यदि (ret) {
		dev_err(dev, "Failed to get i2c node, err: %d\n", ret);
		फर्काऊ ret;
	}

	ret = ofnode_read_u32(phandle.गाँठो, "reg", &slave_addr);
	यदि (ret)
		फर्काऊ ret;

	ret = i2c_get_chip(i2c_bus, slave_addr, 1, &i2c_dev);
	यदि (ret) {
		dev_err(dev, "%s: can't find i2c chip device for addr 0x%x\n", __func__,
			slave_addr);
		फर्काऊ ret;
	}

	i2c_chip = dev_get_parent_plat(i2c_dev);
	यदि (!i2c_chip) {
		dev_err(dev, "parent platform data not found\n");
		फर्काऊ -EINVAL;
	}

	i2c_chip->flags &= ~DM_I2C_CHIP_WR_ADDRESS;
	/* SMBus write command */
	ret = dm_i2c_write(i2c_dev, 0, (uint8_t *)&data_buf, 8);
	यदि (ret) {
		dev_err(dev, "data_buf i2c_write failed, err:%d\n", ret);
		फर्काऊ ret;
	}

	/* Configuration register access command */
	ret = dm_i2c_write(i2c_dev, USB5744_CONFIG_REG_ACCESS_LSB,
			   &config_reg_access_buf, 2);
	यदि (ret) {
		dev_err(dev, "config_reg_access i2c_write failed, err: %d\n", ret);
		फर्काऊ ret;
	}

	/* USB Attach with SMBus */
	ret = dm_i2c_write(i2c_dev, USB5744_COMMAND_ATTACH_LSB, &buf, 2);
	यदि (ret) {
		dev_err(dev, "usb_attach i2c_write failed, err: %d\n", ret);
		फर्काऊ ret;
	}

	फर्काऊ 0;
}

पूर्णाङ्क usb_onboard_hub_reset(संरचना udevice *dev)
{
	संरचना onboard_hub_data *data =
		(संरचना onboard_hub_data *)dev_get_driver_data(dev);
	संरचना onboard_hub *hub = dev_get_priv(dev);
	पूर्णाङ्क ret;

	hub->reset_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_IS_OUT);

	/* property is optional, don't return error! */
	यदि (!hub->reset_gpio)
		फर्काऊ 0;

	ret = dm_gpio_set_मान(hub->reset_gpio, 1);
	यदि (ret)
		फर्काऊ ret;

	udelay(data->reset_us);

	ret = dm_gpio_set_मान(hub->reset_gpio, 0);
	यदि (ret)
		फर्काऊ ret;

	udelay(data->power_on_delay_us);

	फर्काऊ 0;
}

स्थिर पूर्णाङ्क usb_onboard_hub_power_off(संरचना udevice *dev)
{
	संरचना onboard_hub_data *data =
		(संरचना onboard_hub_data *)dev_get_driver_data(dev);
	संरचना onboard_hub *hub = dev_get_priv(dev);
	पूर्णाङ्क ret = 0, ret2;
	अचिन्हित पूर्णाङ्क i;

	लागि (i = data->अङ्क_supplies; i > 0; i--) {
		यदि (hub->vdd[i-1]) {
			ret2 = regulator_set_enable_if_allowed(hub->vdd[i-1], false);
			यदि (ret2 && ret2 != -ENOSYS) {
				dev_err(dev, "can't disable %s: %d\n", data->supply_names[i-1], ret2);
				ret |= ret2;
			}
		}
	}

	फर्काऊ ret;
}

स्थिर पूर्णाङ्क usb_onboard_hub_probe(संरचना udevice *dev)
{
	संरचना onboard_hub_data *data =
		(संरचना onboard_hub_data *)dev_get_driver_data(dev);
	संरचना onboard_hub *hub = dev_get_priv(dev);
	अचिन्हित पूर्णाङ्क i;
	पूर्णाङ्क ret;

	यदि (data->अङ्क_supplies > MAX_SUPPLIES) {
		dev_err(dev, "invalid supplies number, max supported: %d\n", MAX_SUPPLIES);
		फर्काऊ -EINVAL;
	}

	लागि (i = 0; i < data->अङ्क_supplies; i++) {
		ret = device_get_supply_regulator(dev, data->supply_names[i], &hub->vdd[i]);
		यदि (ret && ret != -ENOENT && ret != -ENOSYS) {
			dev_err(dev, "can't get %s: %d\n", data->supply_names[i], ret);
			गच्छ err_supply;
		}

		यदि (hub->vdd[i]) {
			ret = regulator_set_enable_if_allowed(hub->vdd[i], true);
			यदि (ret && ret != -ENOSYS) {
				dev_err(dev, "can't enable %s: %d\n", data->supply_names[i], ret);
				गच्छ err_supply;
			}
		}
	}

	ret = usb_onboard_hub_reset(dev);
	यदि (ret)
		गच्छ err_supply;

	यदि (data->init) {
		ret = data->init(dev);
		यदि (ret) {
			dev_err(dev, "onboard i2c init failed: %d\n", ret);
			गच्छ err;
		}
	}
	फर्काऊ 0;
err:
	dm_gpio_set_मान(hub->reset_gpio, 0);
err_supply:
	usb_onboard_hub_power_off(dev);
	फर्काऊ ret;
}

स्थिर पूर्णाङ्क usb_onboard_hub_bind(संरचना udevice *dev)
{
	संरचना ofnode_phandle_args phandle;
	संरचना udevice *peerdev;
	पूर्णाङ्क ret;

	ret = dev_read_phandle_with_args(dev, "peer-hub", रिक्त, 0, 0, &phandle);
	यदि (ret == -ENOENT) {
		dev_dbg(dev, "peer-hub property not present\n");
		फर्काऊ 0;
	}

	यदि (ret) {
		dev_err(dev, "peer-hub not specified\n");
		फर्काऊ ret;
	}

	ret = uclass_find_device_by_ofnode(UCLASS_USB_HUB, phandle.गाँठो, &peerdev);
	यदि (ret) {
		dev_dbg(dev, "binding before peer-hub %s\n",
			ofnode_get_name(phandle.गाँठो));
		फर्काऊ 0;
	}

	dev_dbg(dev, "peer-hub %s has been bound\n", peerdev->name);

	फर्काऊ -ENODEV;
}

स्थिर पूर्णाङ्क usb_onboard_hub_remove(संरचना udevice *dev)
{
	संरचना onboard_hub *hub = dev_get_priv(dev);
	पूर्णाङ्क ret = 0;

	यदि (hub->reset_gpio) {
		ret = dm_gpio_set_मान(hub->reset_gpio, 1);
		यदि (ret)
			dev_err(dev, "can't set gpio %s: %d\n", hub->reset_gpio->dev->name,
				ret);
	}

	ret |= usb_onboard_hub_power_off(dev);
	फर्काऊ ret;
}

स्थिर अचर संरचना onboard_hub_data corechips_sl6341_data = {
	.reset_us = 10000,
	.अङ्क_supplies = 2,
	.supply_names = { "vdd1v1-supply", "vdd3v3-supply" },
};

स्थिर अचर संरचना onboard_hub_data usb2514_data = {
	.power_on_delay_us = 500,
	.reset_us = 1,
	.अङ्क_supplies = 1,
	.supply_names = { "vdd-supply" },
};

स्थिर अचर संरचना onboard_hub_data usb5744_data = {
	.init = usb5744_i2c_init,
	.power_on_delay_us = 1000,
	.reset_us = 5,
	.अङ्क_supplies = 1,
	.supply_names = { "vdd-supply" },
};

स्थिर अचर संरचना onboard_hub_data usbhx3_data = {
	.reset_us = 10000,
	.अङ्क_supplies = 2,
	.supply_names = { "vdd-supply", "vdd2-supply" },
};

स्थिर अचर संरचना udevice_id usb_onboard_hub_ids[] = {
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
	.हटाओ = usb_onboard_hub_remove,
	.of_match = usb_onboard_hub_ids,
	.priv_auto = परिमाण(संरचना onboard_hub),
};
