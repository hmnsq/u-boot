// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for onboard USB hubs
 *
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 *
 * Mostly inspired by Linux kernel v6.1 onboard_usb_hub driver
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <asm/gpio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm/device_compat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm/uclass-internal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <i2c.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/delay.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <power/regulator.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB5744_COMMAND_ATTACH		0x0056
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB5744_COMMAND_ATTACH_LSB	0xAA
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB5744_CONFIG_REG_ACCESS	0x0037
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB5744_CONFIG_REG_ACCESS_LSB	0x99

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_SUPPLIES 2

ਬਣਤਰ onboard_hub {
	ਬਣਤਰ udevice *vdd[MAX_SUPPLIES];
	ਬਣਤਰ gpio_desc *reset_gpio;
};

ਬਣਤਰ onboard_hub_ਅੰਕੜੇ {
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ reset_us;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ power_on_delay_us;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਅੰਕ_supplies;
	ਅਚਲ ਅੱਖਰ * ਅਚਲ supply_ਨਾਂ[MAX_SUPPLIES];
	ਪੂਰਨ (*init)(ਬਣਤਰ udevice *dev);
};

ਸਥਿਰ ਪੂਰਨ usb5744_i2c_init(ਬਣਤਰ udevice *dev)
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
	u8 ਅੰਕੜੇ_buf[8] = {0x0, 0x5, 0x0, 0x1, 0x41, 0x1D, 0x08};
	u8 config_reg_access_buf = USB5744_CONFIG_REG_ACCESS;
	ਬਣਤਰ udevice *i2c_bus = ਨਿਰਾ, *i2c_dev;
	ਬਣਤਰ ofnode_phandle_ਦਲੀਲਾਂ phandle;
	u8 buf = USB5744_COMMAND_ATTACH;
	ਬਣਤਰ dm_i2c_chip *i2c_chip;
	ਪੂਰਨ ret, slave_addr;

	ret = dev_read_phandle_with_ਦਲੀਲਾਂ(dev, "i2c-bus", ਨਿਰਾ, 0, 0, &phandle);
	ਜੇ (ret == -ENOENT) {
		dev_dbg(dev, "i2c-bus not specified\n");
		ਵਾਪਸ 0;
	}

	ਜੇ (ret) {
		dev_err(dev, "i2c-bus read failed\n");
		ਵਾਪਸ ret;
	}

	ret = device_get_global_by_ofnode(ofnode_get_ਮਾਪਾ(phandle.ਗੰਢ), &i2c_bus);
	ਜੇ (ret) {
		dev_err(dev, "Failed to get i2c node, err: %d\n", ret);
		ਵਾਪਸ ret;
	}

	ret = ofnode_read_u32(phandle.ਗੰਢ, "reg", &slave_addr);
	ਜੇ (ret)
		ਵਾਪਸ ret;

	ret = i2c_get_chip(i2c_bus, slave_addr, 1, &i2c_dev);
	ਜੇ (ret) {
		dev_err(dev, "%s: can't find i2c chip device for addr 0x%x\n", __func__,
			slave_addr);
		ਵਾਪਸ ret;
	}

	i2c_chip = dev_get_ਮਾਪਾ_plat(i2c_dev);
	ਜੇ (!i2c_chip) {
		dev_err(dev, "parent platform data not found\n");
		ਵਾਪਸ -EINVAL;
	}

	i2c_chip->flags &= ~DM_I2_C_CHIP_WR_ਪਤਾ;
	/* SMBus write command */
	ret = dm_i2c_write(i2c_dev, 0, (uint8_t *)&ਅੰਕੜੇ_buf, 8);
	ਜੇ (ret) {
		dev_err(dev, "data_buf i2c_write failed, err:%d\n", ret);
		ਵਾਪਸ ret;
	}

	/* Configuration register access command */
	ret = dm_i2c_write(i2c_dev, USB5744_CONFIG_REG_ACCESS_LSB,
			   &config_reg_access_buf, 2);
	ਜੇ (ret) {
		dev_err(dev, "config_reg_access i2c_write failed, err: %d\n", ret);
		ਵਾਪਸ ret;
	}

	/* USB Attach with SMBus */
	ret = dm_i2c_write(i2c_dev, USB5744_COMMAND_ATTACH_LSB, &buf, 2);
	ਜੇ (ret) {
		dev_err(dev, "usb_attach i2c_write failed, err: %d\n", ret);
		ਵਾਪਸ ret;
	}

	ਵਾਪਸ 0;
}

ਪੂਰਨ usb_onboard_hub_reset(ਬਣਤਰ udevice *dev)
{
	ਬਣਤਰ onboard_hub_ਅੰਕੜੇ *ਅੰਕੜੇ =
		(ਬਣਤਰ onboard_hub_ਅੰਕੜੇ *)dev_get_driver_ਅੰਕੜੇ(dev);
	ਬਣਤਰ onboard_hub *hub = dev_get_priv(dev);
	ਪੂਰਨ ret;

	hub->reset_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_IS_ਬਾਹਰਲਾ);

	/* property is optional, don't return error! */
	ਜੇ (!hub->reset_gpio)
		ਵਾਪਸ 0;

	ret = dm_gpio_ਸੈੱਟ_ਮੁੱਲ(hub->reset_gpio, 1);
	ਜੇ (ret)
		ਵਾਪਸ ret;

	udelay(ਅੰਕੜੇ->reset_us);

	ret = dm_gpio_ਸੈੱਟ_ਮੁੱਲ(hub->reset_gpio, 0);
	ਜੇ (ret)
		ਵਾਪਸ ret;

	udelay(ਅੰਕੜੇ->power_on_delay_us);

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ usb_onboard_hub_power_off(ਬਣਤਰ udevice *dev)
{
	ਬਣਤਰ onboard_hub_ਅੰਕੜੇ *ਅੰਕੜੇ =
		(ਬਣਤਰ onboard_hub_ਅੰਕੜੇ *)dev_get_driver_ਅੰਕੜੇ(dev);
	ਬਣਤਰ onboard_hub *hub = dev_get_priv(dev);
	ਪੂਰਨ ret = 0, ret2;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;

	ਲਈ (i = ਅੰਕੜੇ->ਅੰਕ_supplies; i > 0; i--) {
		ਜੇ (hub->vdd[i-1]) {
			ret2 = regulator_ਸੈੱਟ_enable_if_allowed(hub->vdd[i-1], false);
			ਜੇ (ret2 && ret2 != -ENOSYS) {
				dev_err(dev, "can't disable %s: %d\n", ਅੰਕੜੇ->supply_ਨਾਂ[i-1], ret2);
				ret |= ret2;
			}
		}
	}

	ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ usb_onboard_hub_probe(ਬਣਤਰ udevice *dev)
{
	ਬਣਤਰ onboard_hub_ਅੰਕੜੇ *ਅੰਕੜੇ =
		(ਬਣਤਰ onboard_hub_ਅੰਕੜੇ *)dev_get_driver_ਅੰਕੜੇ(dev);
	ਬਣਤਰ onboard_hub *hub = dev_get_priv(dev);
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
	ਪੂਰਨ ret;

	ਜੇ (ਅੰਕੜੇ->ਅੰਕ_supplies > MAX_SUPPLIES) {
		dev_err(dev, "invalid supplies number, max supported: %d\n", MAX_SUPPLIES);
		ਵਾਪਸ -EINVAL;
	}

	ਲਈ (i = 0; i < ਅੰਕੜੇ->ਅੰਕ_supplies; i++) {
		ret = device_get_supply_regulator(dev, ਅੰਕੜੇ->supply_ਨਾਂ[i], &hub->vdd[i]);
		ਜੇ (ret && ret != -ENOENT && ret != -ENOSYS) {
			dev_err(dev, "can't get %s: %d\n", ਅੰਕੜੇ->supply_ਨਾਂ[i], ret);
			ਜਾਓ err_supply;
		}

		ਜੇ (hub->vdd[i]) {
			ret = regulator_ਸੈੱਟ_enable_if_allowed(hub->vdd[i], true);
			ਜੇ (ret && ret != -ENOSYS) {
				dev_err(dev, "can't enable %s: %d\n", ਅੰਕੜੇ->supply_ਨਾਂ[i], ret);
				ਜਾਓ err_supply;
			}
		}
	}

	ret = usb_onboard_hub_reset(dev);
	ਜੇ (ret)
		ਜਾਓ err_supply;

	ਜੇ (ਅੰਕੜੇ->init) {
		ret = ਅੰਕੜੇ->init(dev);
		ਜੇ (ret) {
			dev_err(dev, "onboard i2c init failed: %d\n", ret);
			ਜਾਓ err;
		}
	}
	ਵਾਪਸ 0;
err:
	dm_gpio_ਸੈੱਟ_ਮੁੱਲ(hub->reset_gpio, 0);
err_supply:
	usb_onboard_hub_power_off(dev);
	ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ usb_onboard_hub_bind(ਬਣਤਰ udevice *dev)
{
	ਬਣਤਰ ofnode_phandle_ਦਲੀਲਾਂ phandle;
	ਬਣਤਰ udevice *peerdev;
	ਪੂਰਨ ret;

	ret = dev_read_phandle_with_ਦਲੀਲਾਂ(dev, "peer-hub", ਨਿਰਾ, 0, 0, &phandle);
	ਜੇ (ret == -ENOENT) {
		dev_dbg(dev, "peer-hub property not present\n");
		ਵਾਪਸ 0;
	}

	ਜੇ (ret) {
		dev_err(dev, "peer-hub not specified\n");
		ਵਾਪਸ ret;
	}

	ret = uclass_ਲੱਭੋ_device_by_ofnode(UCLASS_USB_HUB, phandle.ਗੰਢ, &peerdev);
	ਜੇ (ret) {
		dev_dbg(dev, "binding before peer-hub %s\n",
			ofnode_get_name(phandle.ਗੰਢ));
		ਵਾਪਸ 0;
	}

	dev_dbg(dev, "peer-hub %s has been bound\n", peerdev->name);

	ਵਾਪਸ -ENODEV;
}

ਸਥਿਰ ਪੂਰਨ usb_onboard_hub_remove(ਬਣਤਰ udevice *dev)
{
	ਬਣਤਰ onboard_hub *hub = dev_get_priv(dev);
	ਪੂਰਨ ret = 0;

	ਜੇ (hub->reset_gpio) {
		ret = dm_gpio_ਸੈੱਟ_ਮੁੱਲ(hub->reset_gpio, 1);
		ਜੇ (ret)
			dev_err(dev, "can't set gpio %s: %d\n", hub->reset_gpio->dev->name,
				ret);
	}

	ret |= usb_onboard_hub_power_off(dev);
	ਵਾਪਸ ret;
}

ਸਥਿਰ ਅਚਲ ਬਣਤਰ onboard_hub_ਅੰਕੜੇ corechips_sl6341_ਅੰਕੜੇ = {
	.reset_us = 10000,
	.ਅੰਕ_supplies = 2,
	.supply_ਨਾਂ = { "vdd1v1-supply", "vdd3v3-supply" },
};

ਸਥਿਰ ਅਚਲ ਬਣਤਰ onboard_hub_ਅੰਕੜੇ usb2514_ਅੰਕੜੇ = {
	.power_on_delay_us = 500,
	.reset_us = 1,
	.ਅੰਕ_supplies = 1,
	.supply_ਨਾਂ = { "vdd-supply" },
};

ਸਥਿਰ ਅਚਲ ਬਣਤਰ onboard_hub_ਅੰਕੜੇ usb5744_ਅੰਕੜੇ = {
	.init = usb5744_i2c_init,
	.power_on_delay_us = 1000,
	.reset_us = 5,
	.ਅੰਕ_supplies = 1,
	.supply_ਨਾਂ = { "vdd-supply" },
};

ਸਥਿਰ ਅਚਲ ਬਣਤਰ onboard_hub_ਅੰਕੜੇ usbhx3_ਅੰਕੜੇ = {
	.reset_us = 10000,
	.ਅੰਕ_supplies = 2,
	.supply_ਨਾਂ = { "vdd-supply", "vdd2-supply" },
};

ਸਥਿਰ ਅਚਲ ਬਣਤਰ udevice_id usb_onboard_hub_ids[] = {
	/* Use generic usbVID,PID dt-bindings (usb-device.yaml) */
	{	.compatible = "usb3431,6241",	/* Corechips SL6341 USB 2.0 */
		.ਅੰਕੜੇ = (ulong)&corechips_sl6341_ਅੰਕੜੇ,
	}, {
		.compatible = "usb3431,6341",	/* Corechips SL6341 USB 3.0 */
		.ਅੰਕੜੇ = (ulong)&corechips_sl6341_ਅੰਕੜੇ,
	}, {
		.compatible = "usb424,2514",	/* USB2514B USB 2.0 */
		.ਅੰਕੜੇ = (ulong)&usb2514_ਅੰਕੜੇ,
	}, {
		.compatible = "usb424,2744",	/* USB2744 USB 2.0 */
		.ਅੰਕੜੇ = (ulong)&usb5744_ਅੰਕੜੇ,
	}, {
		.compatible = "usb424,5744",	/* USB5744 USB 3.0 */
		.ਅੰਕੜੇ = (ulong)&usb5744_ਅੰਕੜੇ,
	}, {
		.compatible = "usb4b4,6504",	/* Cypress HX3 USB 3.0 */
		.ਅੰਕੜੇ = (ulong)&usbhx3_ਅੰਕੜੇ,
	}, {
		.compatible = "usb4b4,6506",	/* Cypress HX3 USB 2.0 */
		.ਅੰਕੜੇ = (ulong)&usbhx3_ਅੰਕੜੇ,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(usb_onboard_hub) = {
	.name	= "usb_onboard_hub",
	.id	= UCLASS_USB_HUB,
	.bind   = usb_onboard_hub_bind,
	.probe = usb_onboard_hub_probe,
	.remove = usb_onboard_hub_remove,
	.ਦਾ_ਮੇਲ = usb_onboard_hub_ids,
	.priv_auto = ਆਕਾਰ(ਬਣਤਰ onboard_hub),
};
