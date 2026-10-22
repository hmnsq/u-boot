// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for onboard USB hubs
 *
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 *
 * Mostly inspired by Linux kernel v6.1 onboard_usb_hub driver
 */

#అన్తర్భావ <asm/gpio.h>
#అన్తర్భావ <dm.h>
#అన్తర్భావ <dm/device_compat.h>
#అన్తర్భావ <dm/uclass-internal.h>
#అన్తర్భావ <i2c.h>
#అన్తర్భావ <linux/delay.h>
#అన్తర్భావ <power/regulator.h>

#నిర్వచించు USB5744_COMMAND_ATTACH		0x0056
#నిర్వచించు USB5744_COMMAND_ATTACH_LSB	0xAA
#నిర్వచించు USB5744_CONFIG_REG_ACCESS	0x0037
#నిర్వచించు USB5744_CONFIG_REG_ACCESS_LSB	0x99

#నిర్వచించు MAX_SUPPLIES 2

నిర్మాణం onboard_hub {
	నిర్మాణం udevice *vdd[MAX_SUPPLIES];
	నిర్మాణం gpio_desc *reset_gpio;
};

నిర్మాణం onboard_hub_దత్తాంశం {
	గుర్తులేని దీర్ఘ reset_us;
	గుర్తులేని దీర్ఘ power_on_delay_us;
	గుర్తులేని పూర్ణాంకం అంకె_supplies;
	స్థిరరాశి అక్షరం * స్థిరరాశి supply_పేర్లు[MAX_SUPPLIES];
	పూర్ణాంకం (*init)(నిర్మాణం udevice *dev);
};

స్థిర పూర్ణాంకం usb5744_i2c_init(నిర్మాణం udevice *dev)
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
	u8 దత్తాంశం_buf[8] = {0x0, 0x5, 0x0, 0x1, 0x41, 0x1D, 0x08};
	u8 config_reg_access_buf = USB5744_CONFIG_REG_ACCESS;
	నిర్మాణం udevice *i2c_bus = ఖాళీ, *i2c_dev;
	నిర్మాణం ofnode_phandle_వాదనలు phandle;
	u8 buf = USB5744_COMMAND_ATTACH;
	నిర్మాణం dm_i2c_chip *i2c_chip;
	పూర్ణాంకం ret, slave_addr;

	ret = dev_read_phandle_with_వాదనలు(dev, "i2c-bus", ఖాళీ, 0, 0, &phandle);
	ఒకవేళ (ret == -ENOENT) {
		dev_dbg(dev, "i2c-bus not specified\n");
		తిరిగి 0;
	}

	ఒకవేళ (ret) {
		dev_err(dev, "i2c-bus read failed\n");
		తిరిగి ret;
	}

	ret = device_get_global_by_ofnode(ofnode_get_మాతృ(phandle.కణుపు), &i2c_bus);
	ఒకవేళ (ret) {
		dev_err(dev, "Failed to get i2c node, err: %d\n", ret);
		తిరిగి ret;
	}

	ret = ofnode_read_u32(phandle.కణుపు, "reg", &slave_addr);
	ఒకవేళ (ret)
		తిరిగి ret;

	ret = i2c_get_chip(i2c_bus, slave_addr, 1, &i2c_dev);
	ఒకవేళ (ret) {
		dev_err(dev, "%s: can't find i2c chip device for addr 0x%x\n", __func__,
			slave_addr);
		తిరిగి ret;
	}

	i2c_chip = dev_get_మాతృ_plat(i2c_dev);
	ఒకవేళ (!i2c_chip) {
		dev_err(dev, "parent platform data not found\n");
		తిరిగి -EINVAL;
	}

	i2c_chip->flags &= ~DM_I2_C_CHIP_WR_చిరునామా;
	/* SMBus write command */
	ret = dm_i2c_write(i2c_dev, 0, (uint8_t *)&దత్తాంశం_buf, 8);
	ఒకవేళ (ret) {
		dev_err(dev, "data_buf i2c_write failed, err:%d\n", ret);
		తిరిగి ret;
	}

	/* Configuration register access command */
	ret = dm_i2c_write(i2c_dev, USB5744_CONFIG_REG_ACCESS_LSB,
			   &config_reg_access_buf, 2);
	ఒకవేళ (ret) {
		dev_err(dev, "config_reg_access i2c_write failed, err: %d\n", ret);
		తిరిగి ret;
	}

	/* USB Attach with SMBus */
	ret = dm_i2c_write(i2c_dev, USB5744_COMMAND_ATTACH_LSB, &buf, 2);
	ఒకవేళ (ret) {
		dev_err(dev, "usb_attach i2c_write failed, err: %d\n", ret);
		తిరిగి ret;
	}

	తిరిగి 0;
}

పూర్ణాంకం usb_onboard_hub_reset(నిర్మాణం udevice *dev)
{
	నిర్మాణం onboard_hub_దత్తాంశం *దత్తాంశం =
		(నిర్మాణం onboard_hub_దత్తాంశం *)dev_get_driver_దత్తాంశం(dev);
	నిర్మాణం onboard_hub *hub = dev_get_priv(dev);
	పూర్ణాంకం ret;

	hub->reset_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_IS_బహిర్గతం);

	/* property is optional, don't return error! */
	ఒకవేళ (!hub->reset_gpio)
		తిరిగి 0;

	ret = dm_gpio_సమితి_విలువ(hub->reset_gpio, 1);
	ఒకవేళ (ret)
		తిరిగి ret;

	udelay(దత్తాంశం->reset_us);

	ret = dm_gpio_సమితి_విలువ(hub->reset_gpio, 0);
	ఒకవేళ (ret)
		తిరిగి ret;

	udelay(దత్తాంశం->power_on_delay_us);

	తిరిగి 0;
}

స్థిర పూర్ణాంకం usb_onboard_hub_power_off(నిర్మాణం udevice *dev)
{
	నిర్మాణం onboard_hub_దత్తాంశం *దత్తాంశం =
		(నిర్మాణం onboard_hub_దత్తాంశం *)dev_get_driver_దత్తాంశం(dev);
	నిర్మాణం onboard_hub *hub = dev_get_priv(dev);
	పూర్ణాంకం ret = 0, ret2;
	గుర్తులేని పూర్ణాంకం i;

	కొరకు (i = దత్తాంశం->అంకె_supplies; i > 0; i--) {
		ఒకవేళ (hub->vdd[i-1]) {
			ret2 = regulator_సమితి_enable_if_allowed(hub->vdd[i-1], false);
			ఒకవేళ (ret2 && ret2 != -ENOSYS) {
				dev_err(dev, "can't disable %s: %d\n", దత్తాంశం->supply_పేర్లు[i-1], ret2);
				ret |= ret2;
			}
		}
	}

	తిరిగి ret;
}

స్థిర పూర్ణాంకం usb_onboard_hub_probe(నిర్మాణం udevice *dev)
{
	నిర్మాణం onboard_hub_దత్తాంశం *దత్తాంశం =
		(నిర్మాణం onboard_hub_దత్తాంశం *)dev_get_driver_దత్తాంశం(dev);
	నిర్మాణం onboard_hub *hub = dev_get_priv(dev);
	గుర్తులేని పూర్ణాంకం i;
	పూర్ణాంకం ret;

	ఒకవేళ (దత్తాంశం->అంకె_supplies > MAX_SUPPLIES) {
		dev_err(dev, "invalid supplies number, max supported: %d\n", MAX_SUPPLIES);
		తిరిగి -EINVAL;
	}

	కొరకు (i = 0; i < దత్తాంశం->అంకె_supplies; i++) {
		ret = device_get_supply_regulator(dev, దత్తాంశం->supply_పేర్లు[i], &hub->vdd[i]);
		ఒకవేళ (ret && ret != -ENOENT && ret != -ENOSYS) {
			dev_err(dev, "can't get %s: %d\n", దత్తాంశం->supply_పేర్లు[i], ret);
			వెళ్ళు err_supply;
		}

		ఒకవేళ (hub->vdd[i]) {
			ret = regulator_సమితి_enable_if_allowed(hub->vdd[i], true);
			ఒకవేళ (ret && ret != -ENOSYS) {
				dev_err(dev, "can't enable %s: %d\n", దత్తాంశం->supply_పేర్లు[i], ret);
				వెళ్ళు err_supply;
			}
		}
	}

	ret = usb_onboard_hub_reset(dev);
	ఒకవేళ (ret)
		వెళ్ళు err_supply;

	ఒకవేళ (దత్తాంశం->init) {
		ret = దత్తాంశం->init(dev);
		ఒకవేళ (ret) {
			dev_err(dev, "onboard i2c init failed: %d\n", ret);
			వెళ్ళు err;
		}
	}
	తిరిగి 0;
err:
	dm_gpio_సమితి_విలువ(hub->reset_gpio, 0);
err_supply:
	usb_onboard_hub_power_off(dev);
	తిరిగి ret;
}

స్థిర పూర్ణాంకం usb_onboard_hub_bind(నిర్మాణం udevice *dev)
{
	నిర్మాణం ofnode_phandle_వాదనలు phandle;
	నిర్మాణం udevice *peerdev;
	పూర్ణాంకం ret;

	ret = dev_read_phandle_with_వాదనలు(dev, "peer-hub", ఖాళీ, 0, 0, &phandle);
	ఒకవేళ (ret == -ENOENT) {
		dev_dbg(dev, "peer-hub property not present\n");
		తిరిగి 0;
	}

	ఒకవేళ (ret) {
		dev_err(dev, "peer-hub not specified\n");
		తిరిగి ret;
	}

	ret = uclass_కనుగొను_device_by_ofnode(UCLASS_USB_HUB, phandle.కణుపు, &peerdev);
	ఒకవేళ (ret) {
		dev_dbg(dev, "binding before peer-hub %s\n",
			ofnode_get_name(phandle.కణుపు));
		తిరిగి 0;
	}

	dev_dbg(dev, "peer-hub %s has been bound\n", peerdev->name);

	తిరిగి -ENODEV;
}

స్థిర పూర్ణాంకం usb_onboard_hub_remove(నిర్మాణం udevice *dev)
{
	నిర్మాణం onboard_hub *hub = dev_get_priv(dev);
	పూర్ణాంకం ret = 0;

	ఒకవేళ (hub->reset_gpio) {
		ret = dm_gpio_సమితి_విలువ(hub->reset_gpio, 1);
		ఒకవేళ (ret)
			dev_err(dev, "can't set gpio %s: %d\n", hub->reset_gpio->dev->name,
				ret);
	}

	ret |= usb_onboard_hub_power_off(dev);
	తిరిగి ret;
}

స్థిర స్థిరరాశి నిర్మాణం onboard_hub_దత్తాంశం corechips_sl6341_దత్తాంశం = {
	.reset_us = 10000,
	.అంకె_supplies = 2,
	.supply_పేర్లు = { "vdd1v1-supply", "vdd3v3-supply" },
};

స్థిర స్థిరరాశి నిర్మాణం onboard_hub_దత్తాంశం usb2514_దత్తాంశం = {
	.power_on_delay_us = 500,
	.reset_us = 1,
	.అంకె_supplies = 1,
	.supply_పేర్లు = { "vdd-supply" },
};

స్థిర స్థిరరాశి నిర్మాణం onboard_hub_దత్తాంశం usb5744_దత్తాంశం = {
	.init = usb5744_i2c_init,
	.power_on_delay_us = 1000,
	.reset_us = 5,
	.అంకె_supplies = 1,
	.supply_పేర్లు = { "vdd-supply" },
};

స్థిర స్థిరరాశి నిర్మాణం onboard_hub_దత్తాంశం usbhx3_దత్తాంశం = {
	.reset_us = 10000,
	.అంకె_supplies = 2,
	.supply_పేర్లు = { "vdd-supply", "vdd2-supply" },
};

స్థిర స్థిరరాశి నిర్మాణం udevice_id usb_onboard_hub_ids[] = {
	/* Use generic usbVID,PID dt-bindings (usb-device.yaml) */
	{	.compatible = "usb3431,6241",	/* Corechips SL6341 USB 2.0 */
		.దత్తాంశం = (ulong)&corechips_sl6341_దత్తాంశం,
	}, {
		.compatible = "usb3431,6341",	/* Corechips SL6341 USB 3.0 */
		.దత్తాంశం = (ulong)&corechips_sl6341_దత్తాంశం,
	}, {
		.compatible = "usb424,2514",	/* USB2514B USB 2.0 */
		.దత్తాంశం = (ulong)&usb2514_దత్తాంశం,
	}, {
		.compatible = "usb424,2744",	/* USB2744 USB 2.0 */
		.దత్తాంశం = (ulong)&usb5744_దత్తాంశం,
	}, {
		.compatible = "usb424,5744",	/* USB5744 USB 3.0 */
		.దత్తాంశం = (ulong)&usb5744_దత్తాంశం,
	}, {
		.compatible = "usb4b4,6504",	/* Cypress HX3 USB 3.0 */
		.దత్తాంశం = (ulong)&usbhx3_దత్తాంశం,
	}, {
		.compatible = "usb4b4,6506",	/* Cypress HX3 USB 2.0 */
		.దత్తాంశం = (ulong)&usbhx3_దత్తాంశం,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(usb_onboard_hub) = {
	.name	= "usb_onboard_hub",
	.id	= UCLASS_USB_HUB,
	.bind   = usb_onboard_hub_bind,
	.probe = usb_onboard_hub_probe,
	.remove = usb_onboard_hub_remove,
	.యొక్క_సరిపోలిక = usb_onboard_hub_ids,
	.priv_auto = పరిమాణం(నిర్మాణం onboard_hub),
};
