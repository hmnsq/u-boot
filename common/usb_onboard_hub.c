// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for onboard USB hubs
 *
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 *
 * Mostly inspired by Linux kernel v6.1 onboard_usb_hub driver
 */

#அந்தர்பாவ <asm/gpio.h>
#அந்தர்பாவ <dm.h>
#அந்தர்பாவ <dm/device_compat.h>
#அந்தர்பாவ <dm/uclass-internal.h>
#அந்தர்பாவ <i2c.h>
#அந்தர்பாவ <linux/delay.h>
#அந்தர்பாவ <power/regulator.h>

#define USB5744_COMMAND_ATTACH		0x0056
#define USB5744_COMMAND_ATTACH_LSB	0xAA
#define USB5744_CONFIG_REG_ACCESS	0x0037
#define USB5744_CONFIG_REG_ACCESS_LSB	0x99

#define MAX_SUPPLIES 2

கட்டமைப்பு onboard_hub {
	கட்டமைப்பு udevice *vdd[MAX_SUPPLIES];
	கட்டமைப்பு gpio_desc *reset_gpio;
};

கட்டமைப்பு onboard_hub_data {
	unsigned long reset_us;
	unsigned long power_on_delay_us;
	unsigned முழு இலக்கம்_supplies;
	மாறிலி எழுத்து * மாறிலி supply_names[MAX_SUPPLIES];
	முழு (*init)(கட்டமைப்பு udevice *dev);
};

நிலையான முழு usb5744_i2c_init(கட்டமைப்பு udevice *dev)
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
	கட்டமைப்பு udevice *i2c_bus = வெற்று, *i2c_dev;
	கட்டமைப்பு ofnode_phandle_args phandle;
	u8 buf = USB5744_COMMAND_ATTACH;
	கட்டமைப்பு dm_i2c_chip *i2c_chip;
	முழு ret, slave_addr;

	ret = dev_read_phandle_with_args(dev, "i2c-bus", வெற்று, 0, 0, &phandle);
	எனில் (ret == -ENOENT) {
		dev_dbg(dev, "i2c-bus not specified\n");
		திரும்பு 0;
	}

	எனில் (ret) {
		dev_err(dev, "i2c-bus read failed\n");
		திரும்பு ret;
	}

	ret = device_get_global_by_ofnode(ofnode_get_parent(phandle.முனை), &i2c_bus);
	எனில் (ret) {
		dev_err(dev, "Failed to get i2c node, err: %d\n", ret);
		திரும்பு ret;
	}

	ret = ofnode_read_u32(phandle.முனை, "reg", &slave_addr);
	எனில் (ret)
		திரும்பு ret;

	ret = i2c_get_chip(i2c_bus, slave_addr, 1, &i2c_dev);
	எனில் (ret) {
		dev_err(dev, "%s: can't find i2c chip device for addr 0x%x\n", __func__,
			slave_addr);
		திரும்பு ret;
	}

	i2c_chip = dev_get_parent_plat(i2c_dev);
	எனில் (!i2c_chip) {
		dev_err(dev, "parent platform data not found\n");
		திரும்பு -EINVAL;
	}

	i2c_chip->flags &= ~DM_I2C_CHIP_WR_ADDRESS;
	/* SMBus write command */
	ret = dm_i2c_write(i2c_dev, 0, (uint8_t *)&data_buf, 8);
	எனில் (ret) {
		dev_err(dev, "data_buf i2c_write failed, err:%d\n", ret);
		திரும்பு ret;
	}

	/* Configuration register access command */
	ret = dm_i2c_write(i2c_dev, USB5744_CONFIG_REG_ACCESS_LSB,
			   &config_reg_access_buf, 2);
	எனில் (ret) {
		dev_err(dev, "config_reg_access i2c_write failed, err: %d\n", ret);
		திரும்பு ret;
	}

	/* USB Attach with SMBus */
	ret = dm_i2c_write(i2c_dev, USB5744_COMMAND_ATTACH_LSB, &buf, 2);
	எனில் (ret) {
		dev_err(dev, "usb_attach i2c_write failed, err: %d\n", ret);
		திரும்பு ret;
	}

	திரும்பு 0;
}

முழு usb_onboard_hub_reset(கட்டமைப்பு udevice *dev)
{
	கட்டமைப்பு onboard_hub_data *data =
		(கட்டமைப்பு onboard_hub_data *)dev_get_driver_data(dev);
	கட்டமைப்பு onboard_hub *hub = dev_get_priv(dev);
	முழு ret;

	hub->reset_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_IS_OUT);

	/* property is optional, don't return error! */
	எனில் (!hub->reset_gpio)
		திரும்பு 0;

	ret = dm_gpio_set_மதிப்பு(hub->reset_gpio, 1);
	எனில் (ret)
		திரும்பு ret;

	udelay(data->reset_us);

	ret = dm_gpio_set_மதிப்பு(hub->reset_gpio, 0);
	எனில் (ret)
		திரும்பு ret;

	udelay(data->power_on_delay_us);

	திரும்பு 0;
}

நிலையான முழு usb_onboard_hub_power_off(கட்டமைப்பு udevice *dev)
{
	கட்டமைப்பு onboard_hub_data *data =
		(கட்டமைப்பு onboard_hub_data *)dev_get_driver_data(dev);
	கட்டமைப்பு onboard_hub *hub = dev_get_priv(dev);
	முழு ret = 0, ret2;
	unsigned முழு i;

	க்கு (i = data->இலக்கம்_supplies; i > 0; i--) {
		எனில் (hub->vdd[i-1]) {
			ret2 = regulator_set_enable_if_allowed(hub->vdd[i-1], false);
			எனில் (ret2 && ret2 != -ENOSYS) {
				dev_err(dev, "can't disable %s: %d\n", data->supply_names[i-1], ret2);
				ret |= ret2;
			}
		}
	}

	திரும்பு ret;
}

நிலையான முழு usb_onboard_hub_probe(கட்டமைப்பு udevice *dev)
{
	கட்டமைப்பு onboard_hub_data *data =
		(கட்டமைப்பு onboard_hub_data *)dev_get_driver_data(dev);
	கட்டமைப்பு onboard_hub *hub = dev_get_priv(dev);
	unsigned முழு i;
	முழு ret;

	எனில் (data->இலக்கம்_supplies > MAX_SUPPLIES) {
		dev_err(dev, "invalid supplies number, max supported: %d\n", MAX_SUPPLIES);
		திரும்பு -EINVAL;
	}

	க்கு (i = 0; i < data->இலக்கம்_supplies; i++) {
		ret = device_get_supply_regulator(dev, data->supply_names[i], &hub->vdd[i]);
		எனில் (ret && ret != -ENOENT && ret != -ENOSYS) {
			dev_err(dev, "can't get %s: %d\n", data->supply_names[i], ret);
			செல் err_supply;
		}

		எனில் (hub->vdd[i]) {
			ret = regulator_set_enable_if_allowed(hub->vdd[i], true);
			எனில் (ret && ret != -ENOSYS) {
				dev_err(dev, "can't enable %s: %d\n", data->supply_names[i], ret);
				செல் err_supply;
			}
		}
	}

	ret = usb_onboard_hub_reset(dev);
	எனில் (ret)
		செல் err_supply;

	எனில் (data->init) {
		ret = data->init(dev);
		எனில் (ret) {
			dev_err(dev, "onboard i2c init failed: %d\n", ret);
			செல் err;
		}
	}
	திரும்பு 0;
err:
	dm_gpio_set_மதிப்பு(hub->reset_gpio, 0);
err_supply:
	usb_onboard_hub_power_off(dev);
	திரும்பு ret;
}

நிலையான முழு usb_onboard_hub_bind(கட்டமைப்பு udevice *dev)
{
	கட்டமைப்பு ofnode_phandle_args phandle;
	கட்டமைப்பு udevice *peerdev;
	முழு ret;

	ret = dev_read_phandle_with_args(dev, "peer-hub", வெற்று, 0, 0, &phandle);
	எனில் (ret == -ENOENT) {
		dev_dbg(dev, "peer-hub property not present\n");
		திரும்பு 0;
	}

	எனில் (ret) {
		dev_err(dev, "peer-hub not specified\n");
		திரும்பு ret;
	}

	ret = uclass_find_device_by_ofnode(UCLASS_USB_HUB, phandle.முனை, &peerdev);
	எனில் (ret) {
		dev_dbg(dev, "binding before peer-hub %s\n",
			ofnode_get_name(phandle.முனை));
		திரும்பு 0;
	}

	dev_dbg(dev, "peer-hub %s has been bound\n", peerdev->name);

	திரும்பு -ENODEV;
}

நிலையான முழு usb_onboard_hub_remove(கட்டமைப்பு udevice *dev)
{
	கட்டமைப்பு onboard_hub *hub = dev_get_priv(dev);
	முழு ret = 0;

	எனில் (hub->reset_gpio) {
		ret = dm_gpio_set_மதிப்பு(hub->reset_gpio, 1);
		எனில் (ret)
			dev_err(dev, "can't set gpio %s: %d\n", hub->reset_gpio->dev->name,
				ret);
	}

	ret |= usb_onboard_hub_power_off(dev);
	திரும்பு ret;
}

நிலையான மாறிலி கட்டமைப்பு onboard_hub_data corechips_sl6341_data = {
	.reset_us = 10000,
	.இலக்கம்_supplies = 2,
	.supply_names = { "vdd1v1-supply", "vdd3v3-supply" },
};

நிலையான மாறிலி கட்டமைப்பு onboard_hub_data usb2514_data = {
	.power_on_delay_us = 500,
	.reset_us = 1,
	.இலக்கம்_supplies = 1,
	.supply_names = { "vdd-supply" },
};

நிலையான மாறிலி கட்டமைப்பு onboard_hub_data usb5744_data = {
	.init = usb5744_i2c_init,
	.power_on_delay_us = 1000,
	.reset_us = 5,
	.இலக்கம்_supplies = 1,
	.supply_names = { "vdd-supply" },
};

நிலையான மாறிலி கட்டமைப்பு onboard_hub_data usbhx3_data = {
	.reset_us = 10000,
	.இலக்கம்_supplies = 2,
	.supply_names = { "vdd-supply", "vdd2-supply" },
};

நிலையான மாறிலி கட்டமைப்பு udevice_id usb_onboard_hub_ids[] = {
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
	.priv_auto = அளவு(கட்டமைப்பு onboard_hub),
};
