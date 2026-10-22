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

#ವ್ಯಾಖ್ಯಾನಿಸು USB5744_COMMAND_ATTACH		0x0056
#ವ್ಯಾಖ್ಯಾನಿಸು USB5744_COMMAND_ATTACH_LSB	0xAA
#ವ್ಯಾಖ್ಯಾನಿಸು USB5744_CONFIG_REG_ACCESS	0x0037
#ವ್ಯಾಖ್ಯಾನಿಸು USB5744_CONFIG_REG_ACCESS_LSB	0x99

#ವ್ಯಾಖ್ಯಾನಿಸು MAX_SUPPLIES 2

ರಚನೆ onboard_hub {
	ರಚನೆ udevice *vdd[MAX_SUPPLIES];
	ರಚನೆ gpio_desc *reset_gpio;
};

ರಚನೆ onboard_hub_ದತ್ತಾಂಶ {
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ reset_us;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ power_on_delay_us;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಅಂಕೆ_supplies;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ supply_ಹೆಸರುಗಳು[MAX_SUPPLIES];
	ಪೂರ್ಣಾಂಕ (*init)(ರಚನೆ udevice *dev);
};

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb5744_i2c_init(ರಚನೆ udevice *dev)
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
	u8 ದತ್ತಾಂಶ_buf[8] = {0x0, 0x5, 0x0, 0x1, 0x41, 0x1D, 0x08};
	u8 config_reg_access_buf = USB5744_CONFIG_REG_ACCESS;
	ರಚನೆ udevice *i2c_bus = ಖಾಲಿ, *i2c_dev;
	ರಚನೆ ofnode_phandle_ವಾದಗಳು phandle;
	u8 buf = USB5744_COMMAND_ATTACH;
	ರಚನೆ dm_i2c_chip *i2c_chip;
	ಪೂರ್ಣಾಂಕ ret, slave_addr;

	ret = dev_read_phandle_with_ವಾದಗಳು(dev, "i2c-bus", ಖಾಲಿ, 0, 0, &phandle);
	ಆದರೆ (ret == -ENOENT) {
		dev_dbg(dev, "i2c-bus not specified\n");
		ಹಿಂತಿರುಗಿ 0;
	}

	ಆದರೆ (ret) {
		dev_err(dev, "i2c-bus read failed\n");
		ಹಿಂತಿರುಗಿ ret;
	}

	ret = device_get_global_by_ofnode(ofnode_get_ಪೋಷಕ(phandle.ಗಂಟು), &i2c_bus);
	ಆದರೆ (ret) {
		dev_err(dev, "Failed to get i2c node, err: %d\n", ret);
		ಹಿಂತಿರುಗಿ ret;
	}

	ret = ofnode_read_u32(phandle.ಗಂಟು, "reg", &slave_addr);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;

	ret = i2c_get_chip(i2c_bus, slave_addr, 1, &i2c_dev);
	ಆದರೆ (ret) {
		dev_err(dev, "%s: can't find i2c chip device for addr 0x%x\n", __func__,
			slave_addr);
		ಹಿಂತಿರುಗಿ ret;
	}

	i2c_chip = dev_get_ಪೋಷಕ_plat(i2c_dev);
	ಆದರೆ (!i2c_chip) {
		dev_err(dev, "parent platform data not found\n");
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	i2c_chip->flags &= ~DM_I2_C_CHIP_WR_ವಿಳಾಸ;
	/* SMBus write command */
	ret = dm_i2c_write(i2c_dev, 0, (uint8_t *)&ದತ್ತಾಂಶ_buf, 8);
	ಆದರೆ (ret) {
		dev_err(dev, "data_buf i2c_write failed, err:%d\n", ret);
		ಹಿಂತಿರುಗಿ ret;
	}

	/* Configuration register access command */
	ret = dm_i2c_write(i2c_dev, USB5744_CONFIG_REG_ACCESS_LSB,
			   &config_reg_access_buf, 2);
	ಆದರೆ (ret) {
		dev_err(dev, "config_reg_access i2c_write failed, err: %d\n", ret);
		ಹಿಂತಿರುಗಿ ret;
	}

	/* USB Attach with SMBus */
	ret = dm_i2c_write(i2c_dev, USB5744_COMMAND_ATTACH_LSB, &buf, 2);
	ಆದರೆ (ret) {
		dev_err(dev, "usb_attach i2c_write failed, err: %d\n", ret);
		ಹಿಂತಿರುಗಿ ret;
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ usb_onboard_hub_reset(ರಚನೆ udevice *dev)
{
	ರಚನೆ onboard_hub_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ =
		(ರಚನೆ onboard_hub_ದತ್ತಾಂಶ *)dev_get_driver_ದತ್ತಾಂಶ(dev);
	ರಚನೆ onboard_hub *hub = dev_get_priv(dev);
	ಪೂರ್ಣಾಂಕ ret;

	hub->reset_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_IS_ಹೊರಹೋಗು);

	/* property is optional, don't return error! */
	ಆದರೆ (!hub->reset_gpio)
		ಹಿಂತಿರುಗಿ 0;

	ret = dm_gpio_ಗಣ_ಮೌಲ್ಯ(hub->reset_gpio, 1);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;

	udelay(ದತ್ತಾಂಶ->reset_us);

	ret = dm_gpio_ಗಣ_ಮೌಲ್ಯ(hub->reset_gpio, 0);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;

	udelay(ದತ್ತಾಂಶ->power_on_delay_us);

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_onboard_hub_power_off(ರಚನೆ udevice *dev)
{
	ರಚನೆ onboard_hub_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ =
		(ರಚನೆ onboard_hub_ದತ್ತಾಂಶ *)dev_get_driver_ದತ್ತಾಂಶ(dev);
	ರಚನೆ onboard_hub *hub = dev_get_priv(dev);
	ಪೂರ್ಣಾಂಕ ret = 0, ret2;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = ದತ್ತಾಂಶ->ಅಂಕೆ_supplies; i > 0; i--) {
		ಆದರೆ (hub->vdd[i-1]) {
			ret2 = regulator_ಗಣ_enable_if_allowed(hub->vdd[i-1], false);
			ಆದರೆ (ret2 && ret2 != -ENOSYS) {
				dev_err(dev, "can't disable %s: %d\n", ದತ್ತಾಂಶ->supply_ಹೆಸರುಗಳು[i-1], ret2);
				ret |= ret2;
			}
		}
	}

	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_onboard_hub_probe(ರಚನೆ udevice *dev)
{
	ರಚನೆ onboard_hub_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ =
		(ರಚನೆ onboard_hub_ದತ್ತಾಂಶ *)dev_get_driver_ದತ್ತಾಂಶ(dev);
	ರಚನೆ onboard_hub *hub = dev_get_priv(dev);
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
	ಪೂರ್ಣಾಂಕ ret;

	ಆದರೆ (ದತ್ತಾಂಶ->ಅಂಕೆ_supplies > MAX_SUPPLIES) {
		dev_err(dev, "invalid supplies number, max supported: %d\n", MAX_SUPPLIES);
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	ಗಾಗಿ (i = 0; i < ದತ್ತಾಂಶ->ಅಂಕೆ_supplies; i++) {
		ret = device_get_supply_regulator(dev, ದತ್ತಾಂಶ->supply_ಹೆಸರುಗಳು[i], &hub->vdd[i]);
		ಆದರೆ (ret && ret != -ENOENT && ret != -ENOSYS) {
			dev_err(dev, "can't get %s: %d\n", ದತ್ತಾಂಶ->supply_ಹೆಸರುಗಳು[i], ret);
			ಹೋಗು err_supply;
		}

		ಆದರೆ (hub->vdd[i]) {
			ret = regulator_ಗಣ_enable_if_allowed(hub->vdd[i], true);
			ಆದರೆ (ret && ret != -ENOSYS) {
				dev_err(dev, "can't enable %s: %d\n", ದತ್ತಾಂಶ->supply_ಹೆಸರುಗಳು[i], ret);
				ಹೋಗು err_supply;
			}
		}
	}

	ret = usb_onboard_hub_reset(dev);
	ಆದರೆ (ret)
		ಹೋಗು err_supply;

	ಆದರೆ (ದತ್ತಾಂಶ->init) {
		ret = ದತ್ತಾಂಶ->init(dev);
		ಆದರೆ (ret) {
			dev_err(dev, "onboard i2c init failed: %d\n", ret);
			ಹೋಗು err;
		}
	}
	ಹಿಂತಿರುಗಿ 0;
err:
	dm_gpio_ಗಣ_ಮೌಲ್ಯ(hub->reset_gpio, 0);
err_supply:
	usb_onboard_hub_power_off(dev);
	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_onboard_hub_bind(ರಚನೆ udevice *dev)
{
	ರಚನೆ ofnode_phandle_ವಾದಗಳು phandle;
	ರಚನೆ udevice *peerdev;
	ಪೂರ್ಣಾಂಕ ret;

	ret = dev_read_phandle_with_ವಾದಗಳು(dev, "peer-hub", ಖಾಲಿ, 0, 0, &phandle);
	ಆದರೆ (ret == -ENOENT) {
		dev_dbg(dev, "peer-hub property not present\n");
		ಹಿಂತಿರುಗಿ 0;
	}

	ಆದರೆ (ret) {
		dev_err(dev, "peer-hub not specified\n");
		ಹಿಂತಿರುಗಿ ret;
	}

	ret = uclass_ಕಂಡುಹಿಡಿ_device_by_ofnode(UCLASS_USB_HUB, phandle.ಗಂಟು, &peerdev);
	ಆದರೆ (ret) {
		dev_dbg(dev, "binding before peer-hub %s\n",
			ofnode_get_name(phandle.ಗಂಟು));
		ಹಿಂತಿರುಗಿ 0;
	}

	dev_dbg(dev, "peer-hub %s has been bound\n", peerdev->name);

	ಹಿಂತಿರುಗಿ -ENODEV;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_onboard_hub_remove(ರಚನೆ udevice *dev)
{
	ರಚನೆ onboard_hub *hub = dev_get_priv(dev);
	ಪೂರ್ಣಾಂಕ ret = 0;

	ಆದರೆ (hub->reset_gpio) {
		ret = dm_gpio_ಗಣ_ಮೌಲ್ಯ(hub->reset_gpio, 1);
		ಆದರೆ (ret)
			dev_err(dev, "can't set gpio %s: %d\n", hub->reset_gpio->dev->name,
				ret);
	}

	ret |= usb_onboard_hub_power_off(dev);
	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ onboard_hub_ದತ್ತಾಂಶ corechips_sl6341_ದತ್ತಾಂಶ = {
	.reset_us = 10000,
	.ಅಂಕೆ_supplies = 2,
	.supply_ಹೆಸರುಗಳು = { "vdd1v1-supply", "vdd3v3-supply" },
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ onboard_hub_ದತ್ತಾಂಶ usb2514_ದತ್ತಾಂಶ = {
	.power_on_delay_us = 500,
	.reset_us = 1,
	.ಅಂಕೆ_supplies = 1,
	.supply_ಹೆಸರುಗಳು = { "vdd-supply" },
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ onboard_hub_ದತ್ತಾಂಶ usb5744_ದತ್ತಾಂಶ = {
	.init = usb5744_i2c_init,
	.power_on_delay_us = 1000,
	.reset_us = 5,
	.ಅಂಕೆ_supplies = 1,
	.supply_ಹೆಸರುಗಳು = { "vdd-supply" },
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ onboard_hub_ದತ್ತಾಂಶ usbhx3_ದತ್ತಾಂಶ = {
	.reset_us = 10000,
	.ಅಂಕೆ_supplies = 2,
	.supply_ಹೆಸರುಗಳು = { "vdd-supply", "vdd2-supply" },
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ udevice_id usb_onboard_hub_ids[] = {
	/* Use generic usbVID,PID dt-bindings (usb-device.yaml) */
	{	.compatible = "usb3431,6241",	/* Corechips SL6341 USB 2.0 */
		.ದತ್ತಾಂಶ = (ulong)&corechips_sl6341_ದತ್ತಾಂಶ,
	}, {
		.compatible = "usb3431,6341",	/* Corechips SL6341 USB 3.0 */
		.ದತ್ತಾಂಶ = (ulong)&corechips_sl6341_ದತ್ತಾಂಶ,
	}, {
		.compatible = "usb424,2514",	/* USB2514B USB 2.0 */
		.ದತ್ತಾಂಶ = (ulong)&usb2514_ದತ್ತಾಂಶ,
	}, {
		.compatible = "usb424,2744",	/* USB2744 USB 2.0 */
		.ದತ್ತಾಂಶ = (ulong)&usb5744_ದತ್ತಾಂಶ,
	}, {
		.compatible = "usb424,5744",	/* USB5744 USB 3.0 */
		.ದತ್ತಾಂಶ = (ulong)&usb5744_ದತ್ತಾಂಶ,
	}, {
		.compatible = "usb4b4,6504",	/* Cypress HX3 USB 3.0 */
		.ದತ್ತಾಂಶ = (ulong)&usbhx3_ದತ್ತಾಂಶ,
	}, {
		.compatible = "usb4b4,6506",	/* Cypress HX3 USB 2.0 */
		.ದತ್ತಾಂಶ = (ulong)&usbhx3_ದತ್ತಾಂಶ,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(usb_onboard_hub) = {
	.name	= "usb_onboard_hub",
	.id	= UCLASS_USB_HUB,
	.bind   = usb_onboard_hub_bind,
	.probe = usb_onboard_hub_probe,
	.remove = usb_onboard_hub_remove,
	.ದ_ಹೊಂದಿಕೆ = usb_onboard_hub_ids,
	.priv_auto = ಗಾತ್ರ(ರಚನೆ onboard_hub),
};
