// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for onboard USB hubs
 *
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 *
 * Mostly inspired by Linux kernel v6.1 onboard_usb_hub driver
 */

#ಅನ್ತರ್ಭಾವ <asm/gpio.h>
#ಅನ್ತರ್ಭಾವ <dm.h>
#ಅನ್ತರ್ಭಾವ <dm/device_compat.h>
#ಅನ್ತರ್ಭಾವ <dm/uclass-internal.h>
#ಅನ್ತರ್ಭಾವ <i2c.h>
#ಅನ್ತರ್ಭಾವ <linux/delay.h>
#ಅನ್ತರ್ಭಾವ <power/regulator.h>

#define USB5744_COMMAND_ATTACH		0x0056
#define USB5744_COMMAND_ATTACH_LSB	0xAA
#define USB5744_CONFIG_REG_ACCESS	0x0037
#define USB5744_CONFIG_REG_ACCESS_LSB	0x99

#define MAX_SUPPLIES 2

ಸಂರಚನಾ onboard_hub {
	ಸಂರಚನಾ udevice *vdd[MAX_SUPPLIES];
	ಸಂರಚನಾ gpio_desc *reset_gpio;
};

ಸಂರಚನಾ onboard_hub_data {
	unsigned long reset_us;
	unsigned long power_on_delay_us;
	unsigned ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_supplies;
	ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ supply_names[MAX_SUPPLIES];
	ಪೂರ್ಣಾಙ್ಕ (*init)(ಸಂರಚನಾ udevice *dev);
};

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb5744_i2c_init(ಸಂರಚನಾ udevice *dev)
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
	ಸಂರಚನಾ udevice *i2c_bus = ರಿಕ್ತ, *i2c_dev;
	ಸಂರಚನಾ ofnode_phandle_args phandle;
	u8 buf = USB5744_COMMAND_ATTACH;
	ಸಂರಚನಾ dm_i2c_chip *i2c_chip;
	ಪೂರ್ಣಾಙ್ಕ ret, slave_addr;

	ret = dev_read_phandle_with_args(dev, "i2c-bus", ರಿಕ್ತ, 0, 0, &phandle);
	ಯದಿ (ret == -ENOENT) {
		dev_dbg(dev, "i2c-bus not specified\n");
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಯದಿ (ret) {
		dev_err(dev, "i2c-bus read failed\n");
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	ret = device_get_global_by_ofnode(ofnode_get_parent(phandle.ಗಂಟು), &i2c_bus);
	ಯದಿ (ret) {
		dev_err(dev, "Failed to get i2c node, err: %d\n", ret);
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	ret = ofnode_read_u32(phandle.ಗಂಟು, "reg", &slave_addr);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;

	ret = i2c_get_chip(i2c_bus, slave_addr, 1, &i2c_dev);
	ಯದಿ (ret) {
		dev_err(dev, "%s: can't find i2c chip device for addr 0x%x\n", __func__,
			slave_addr);
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	i2c_chip = dev_get_parent_plat(i2c_dev);
	ಯದಿ (!i2c_chip) {
		dev_err(dev, "parent platform data not found\n");
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	i2c_chip->flags &= ~DM_I2C_CHIP_WR_ADDRESS;
	/* SMBus write command */
	ret = dm_i2c_write(i2c_dev, 0, (uint8_t *)&data_buf, 8);
	ಯದಿ (ret) {
		dev_err(dev, "data_buf i2c_write failed, err:%d\n", ret);
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	/* Configuration register access command */
	ret = dm_i2c_write(i2c_dev, USB5744_CONFIG_REG_ACCESS_LSB,
			   &config_reg_access_buf, 2);
	ಯದಿ (ret) {
		dev_err(dev, "config_reg_access i2c_write failed, err: %d\n", ret);
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	/* USB Attach with SMBus */
	ret = dm_i2c_write(i2c_dev, USB5744_COMMAND_ATTACH_LSB, &buf, 2);
	ಯದಿ (ret) {
		dev_err(dev, "usb_attach i2c_write failed, err: %d\n", ret);
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ usb_onboard_hub_reset(ಸಂರಚನಾ udevice *dev)
{
	ಸಂರಚನಾ onboard_hub_data *data =
		(ಸಂರಚನಾ onboard_hub_data *)dev_get_driver_data(dev);
	ಸಂರಚನಾ onboard_hub *hub = dev_get_priv(dev);
	ಪೂರ್ಣಾಙ್ಕ ret;

	hub->reset_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_IS_OUT);

	/* property is optional, don't return error! */
	ಯದಿ (!hub->reset_gpio)
		ಪ್ರತ್ಯಾಗಮ 0;

	ret = dm_gpio_set_ಮೌಲ್ಯ(hub->reset_gpio, 1);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;

	udelay(data->reset_us);

	ret = dm_gpio_set_ಮೌಲ್ಯ(hub->reset_gpio, 0);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;

	udelay(data->power_on_delay_us);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_onboard_hub_power_off(ಸಂರಚನಾ udevice *dev)
{
	ಸಂರಚನಾ onboard_hub_data *data =
		(ಸಂರಚನಾ onboard_hub_data *)dev_get_driver_data(dev);
	ಸಂರಚನಾ onboard_hub *hub = dev_get_priv(dev);
	ಪೂರ್ಣಾಙ್ಕ ret = 0, ret2;
	unsigned ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = data->ಅಂಕೆ_supplies; i > 0; i--) {
		ಯದಿ (hub->vdd[i-1]) {
			ret2 = regulator_set_enable_if_allowed(hub->vdd[i-1], false);
			ಯದಿ (ret2 && ret2 != -ENOSYS) {
				dev_err(dev, "can't disable %s: %d\n", data->supply_names[i-1], ret2);
				ret |= ret2;
			}
		}
	}

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_onboard_hub_probe(ಸಂರಚನಾ udevice *dev)
{
	ಸಂರಚನಾ onboard_hub_data *data =
		(ಸಂರಚನಾ onboard_hub_data *)dev_get_driver_data(dev);
	ಸಂರಚನಾ onboard_hub *hub = dev_get_priv(dev);
	unsigned ಪೂರ್ಣಾಙ್ಕ i;
	ಪೂರ್ಣಾಙ್ಕ ret;

	ಯದಿ (data->ಅಂಕೆ_supplies > MAX_SUPPLIES) {
		dev_err(dev, "invalid supplies number, max supported: %d\n", MAX_SUPPLIES);
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	ಪ್ರತಿ (i = 0; i < data->ಅಂಕೆ_supplies; i++) {
		ret = device_get_supply_regulator(dev, data->supply_names[i], &hub->vdd[i]);
		ಯದಿ (ret && ret != -ENOENT && ret != -ENOSYS) {
			dev_err(dev, "can't get %s: %d\n", data->supply_names[i], ret);
			ಗಚ್ಛ err_supply;
		}

		ಯದಿ (hub->vdd[i]) {
			ret = regulator_set_enable_if_allowed(hub->vdd[i], true);
			ಯದಿ (ret && ret != -ENOSYS) {
				dev_err(dev, "can't enable %s: %d\n", data->supply_names[i], ret);
				ಗಚ್ಛ err_supply;
			}
		}
	}

	ret = usb_onboard_hub_reset(dev);
	ಯದಿ (ret)
		ಗಚ್ಛ err_supply;

	ಯದಿ (data->init) {
		ret = data->init(dev);
		ಯದಿ (ret) {
			dev_err(dev, "onboard i2c init failed: %d\n", ret);
			ಗಚ್ಛ err;
		}
	}
	ಪ್ರತ್ಯಾಗಮ 0;
err:
	dm_gpio_set_ಮೌಲ್ಯ(hub->reset_gpio, 0);
err_supply:
	usb_onboard_hub_power_off(dev);
	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_onboard_hub_bind(ಸಂರಚನಾ udevice *dev)
{
	ಸಂರಚನಾ ofnode_phandle_args phandle;
	ಸಂರಚನಾ udevice *peerdev;
	ಪೂರ್ಣಾಙ್ಕ ret;

	ret = dev_read_phandle_with_args(dev, "peer-hub", ರಿಕ್ತ, 0, 0, &phandle);
	ಯದಿ (ret == -ENOENT) {
		dev_dbg(dev, "peer-hub property not present\n");
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಯದಿ (ret) {
		dev_err(dev, "peer-hub not specified\n");
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	ret = uclass_find_device_by_ofnode(UCLASS_USB_HUB, phandle.ಗಂಟು, &peerdev);
	ಯದಿ (ret) {
		dev_dbg(dev, "binding before peer-hub %s\n",
			ofnode_get_name(phandle.ಗಂಟು));
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	dev_dbg(dev, "peer-hub %s has been bound\n", peerdev->name);

	ಪ್ರತ್ಯಾಗಮ -ENODEV;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_onboard_hub_remove(ಸಂರಚನಾ udevice *dev)
{
	ಸಂರಚನಾ onboard_hub *hub = dev_get_priv(dev);
	ಪೂರ್ಣಾಙ್ಕ ret = 0;

	ಯದಿ (hub->reset_gpio) {
		ret = dm_gpio_set_ಮೌಲ್ಯ(hub->reset_gpio, 1);
		ಯದಿ (ret)
			dev_err(dev, "can't set gpio %s: %d\n", hub->reset_gpio->dev->name,
				ret);
	}

	ret |= usb_onboard_hub_power_off(dev);
	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ onboard_hub_data corechips_sl6341_data = {
	.reset_us = 10000,
	.ಅಂಕೆ_supplies = 2,
	.supply_names = { "vdd1v1-supply", "vdd3v3-supply" },
};

ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ onboard_hub_data usb2514_data = {
	.power_on_delay_us = 500,
	.reset_us = 1,
	.ಅಂಕೆ_supplies = 1,
	.supply_names = { "vdd-supply" },
};

ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ onboard_hub_data usb5744_data = {
	.init = usb5744_i2c_init,
	.power_on_delay_us = 1000,
	.reset_us = 5,
	.ಅಂಕೆ_supplies = 1,
	.supply_names = { "vdd-supply" },
};

ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ onboard_hub_data usbhx3_data = {
	.reset_us = 10000,
	.ಅಂಕೆ_supplies = 2,
	.supply_names = { "vdd-supply", "vdd2-supply" },
};

ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ udevice_id usb_onboard_hub_ids[] = {
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
	.priv_auto = ಪರಿಮಾಣ(ಸಂರಚನಾ onboard_hub),
};
