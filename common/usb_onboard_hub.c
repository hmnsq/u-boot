// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for onboard USB hubs
 *
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 *
 * Mostly inspired by Linux kernel v6.1 onboard_usb_hub driver
 */

#समावेश <asm/gpio.h>
#समावेश <dm.h>
#समावेश <dm/device_compat.h>
#समावेश <dm/uclass-internal.h>
#समावेश <i2c.h>
#समावेश <linux/delay.h>
#समावेश <power/regulator.h>

#घोषणा USB5744_COMMAND_ATTACH		0x0056
#घोषणा USB5744_COMMAND_ATTACH_LSB	0xAA
#घोषणा USB5744_CONFIG_REG_ACCESS	0x0037
#घोषणा USB5744_CONFIG_REG_ACCESS_LSB	0x99

#घोषणा MAX_SUPPLIES 2

काष्ठा onboard_hub {
	काष्ठा udevice *vdd[MAX_SUPPLIES];
	काष्ठा gpio_desc *reset_gpio;
};

काष्ठा onboard_hub_तथ्य {
	अचिन्हित दीर्घ reset_us;
	अचिन्हित दीर्घ power_on_delay_us;
	अचिन्हित पूर्णांक अंक_supplies;
	स्थिर अक्षर * स्थिर supply_names[MAX_SUPPLIES];
	पूर्णांक (*init)(काष्ठा udevice *dev);
};

अटल पूर्णांक usb5744_i2c_init(काष्ठा udevice *dev)
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
	u8 तथ्य_buf[8] = {0x0, 0x5, 0x0, 0x1, 0x41, 0x1D, 0x08};
	u8 config_reg_access_buf = USB5744_CONFIG_REG_ACCESS;
	काष्ठा udevice *i2c_bus = शून्य, *i2c_dev;
	काष्ठा ofnode_phandle_args phandle;
	u8 buf = USB5744_COMMAND_ATTACH;
	काष्ठा dm_i2c_chip *i2c_chip;
	पूर्णांक ret, slave_addr;

	ret = dev_read_phandle_with_args(dev, "i2c-bus", शून्य, 0, 0, &phandle);
	अगर (ret == -ENOENT) {
		dev_dbg(dev, "i2c-bus not specified\n");
		वापस 0;
	}

	अगर (ret) {
		dev_err(dev, "i2c-bus read failed\n");
		वापस ret;
	}

	ret = device_get_global_by_ofnode(ofnode_get_parent(phandle.गाँठ), &i2c_bus);
	अगर (ret) {
		dev_err(dev, "Failed to get i2c node, err: %d\n", ret);
		वापस ret;
	}

	ret = ofnode_read_u32(phandle.गाँठ, "reg", &slave_addr);
	अगर (ret)
		वापस ret;

	ret = i2c_get_chip(i2c_bus, slave_addr, 1, &i2c_dev);
	अगर (ret) {
		dev_err(dev, "%s: can't find i2c chip device for addr 0x%x\n", __func__,
			slave_addr);
		वापस ret;
	}

	i2c_chip = dev_get_parent_plat(i2c_dev);
	अगर (!i2c_chip) {
		dev_err(dev, "parent platform data not found\n");
		वापस -EINVAL;
	}

	i2c_chip->flags &= ~DM_I2C_CHIP_WR_ADDRESS;
	/* SMBus write command */
	ret = dm_i2c_write(i2c_dev, 0, (uint8_t *)&तथ्य_buf, 8);
	अगर (ret) {
		dev_err(dev, "data_buf i2c_write failed, err:%d\n", ret);
		वापस ret;
	}

	/* Configuration register access command */
	ret = dm_i2c_write(i2c_dev, USB5744_CONFIG_REG_ACCESS_LSB,
			   &config_reg_access_buf, 2);
	अगर (ret) {
		dev_err(dev, "config_reg_access i2c_write failed, err: %d\n", ret);
		वापस ret;
	}

	/* USB Attach with SMBus */
	ret = dm_i2c_write(i2c_dev, USB5744_COMMAND_ATTACH_LSB, &buf, 2);
	अगर (ret) {
		dev_err(dev, "usb_attach i2c_write failed, err: %d\n", ret);
		वापस ret;
	}

	वापस 0;
}

पूर्णांक usb_onboard_hub_reset(काष्ठा udevice *dev)
{
	काष्ठा onboard_hub_तथ्य *तथ्य =
		(काष्ठा onboard_hub_तथ्य *)dev_get_driver_तथ्य(dev);
	काष्ठा onboard_hub *hub = dev_get_priv(dev);
	पूर्णांक ret;

	hub->reset_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_IS_OUT);

	/* property is optional, don't return error! */
	अगर (!hub->reset_gpio)
		वापस 0;

	ret = dm_gpio_set_मूल्य(hub->reset_gpio, 1);
	अगर (ret)
		वापस ret;

	udelay(तथ्य->reset_us);

	ret = dm_gpio_set_मूल्य(hub->reset_gpio, 0);
	अगर (ret)
		वापस ret;

	udelay(तथ्य->power_on_delay_us);

	वापस 0;
}

अटल पूर्णांक usb_onboard_hub_power_off(काष्ठा udevice *dev)
{
	काष्ठा onboard_hub_तथ्य *तथ्य =
		(काष्ठा onboard_hub_तथ्य *)dev_get_driver_तथ्य(dev);
	काष्ठा onboard_hub *hub = dev_get_priv(dev);
	पूर्णांक ret = 0, ret2;
	अचिन्हित पूर्णांक i;

	क्रम (i = तथ्य->अंक_supplies; i > 0; i--) {
		अगर (hub->vdd[i-1]) {
			ret2 = regulator_set_enable_if_allowed(hub->vdd[i-1], false);
			अगर (ret2 && ret2 != -ENOSYS) {
				dev_err(dev, "can't disable %s: %d\n", तथ्य->supply_names[i-1], ret2);
				ret |= ret2;
			}
		}
	}

	वापस ret;
}

अटल पूर्णांक usb_onboard_hub_probe(काष्ठा udevice *dev)
{
	काष्ठा onboard_hub_तथ्य *तथ्य =
		(काष्ठा onboard_hub_तथ्य *)dev_get_driver_तथ्य(dev);
	काष्ठा onboard_hub *hub = dev_get_priv(dev);
	अचिन्हित पूर्णांक i;
	पूर्णांक ret;

	अगर (तथ्य->अंक_supplies > MAX_SUPPLIES) {
		dev_err(dev, "invalid supplies number, max supported: %d\n", MAX_SUPPLIES);
		वापस -EINVAL;
	}

	क्रम (i = 0; i < तथ्य->अंक_supplies; i++) {
		ret = device_get_supply_regulator(dev, तथ्य->supply_names[i], &hub->vdd[i]);
		अगर (ret && ret != -ENOENT && ret != -ENOSYS) {
			dev_err(dev, "can't get %s: %d\n", तथ्य->supply_names[i], ret);
			जाओ err_supply;
		}

		अगर (hub->vdd[i]) {
			ret = regulator_set_enable_if_allowed(hub->vdd[i], true);
			अगर (ret && ret != -ENOSYS) {
				dev_err(dev, "can't enable %s: %d\n", तथ्य->supply_names[i], ret);
				जाओ err_supply;
			}
		}
	}

	ret = usb_onboard_hub_reset(dev);
	अगर (ret)
		जाओ err_supply;

	अगर (तथ्य->init) {
		ret = तथ्य->init(dev);
		अगर (ret) {
			dev_err(dev, "onboard i2c init failed: %d\n", ret);
			जाओ err;
		}
	}
	वापस 0;
err:
	dm_gpio_set_मूल्य(hub->reset_gpio, 0);
err_supply:
	usb_onboard_hub_power_off(dev);
	वापस ret;
}

अटल पूर्णांक usb_onboard_hub_bind(काष्ठा udevice *dev)
{
	काष्ठा ofnode_phandle_args phandle;
	काष्ठा udevice *peerdev;
	पूर्णांक ret;

	ret = dev_read_phandle_with_args(dev, "peer-hub", शून्य, 0, 0, &phandle);
	अगर (ret == -ENOENT) {
		dev_dbg(dev, "peer-hub property not present\n");
		वापस 0;
	}

	अगर (ret) {
		dev_err(dev, "peer-hub not specified\n");
		वापस ret;
	}

	ret = uclass_find_device_by_ofnode(UCLASS_USB_HUB, phandle.गाँठ, &peerdev);
	अगर (ret) {
		dev_dbg(dev, "binding before peer-hub %s\n",
			ofnode_get_name(phandle.गाँठ));
		वापस 0;
	}

	dev_dbg(dev, "peer-hub %s has been bound\n", peerdev->name);

	वापस -ENODEV;
}

अटल पूर्णांक usb_onboard_hub_remove(काष्ठा udevice *dev)
{
	काष्ठा onboard_hub *hub = dev_get_priv(dev);
	पूर्णांक ret = 0;

	अगर (hub->reset_gpio) {
		ret = dm_gpio_set_मूल्य(hub->reset_gpio, 1);
		अगर (ret)
			dev_err(dev, "can't set gpio %s: %d\n", hub->reset_gpio->dev->name,
				ret);
	}

	ret |= usb_onboard_hub_power_off(dev);
	वापस ret;
}

अटल स्थिर काष्ठा onboard_hub_तथ्य corechips_sl6341_तथ्य = {
	.reset_us = 10000,
	.अंक_supplies = 2,
	.supply_names = { "vdd1v1-supply", "vdd3v3-supply" },
};

अटल स्थिर काष्ठा onboard_hub_तथ्य usb2514_तथ्य = {
	.power_on_delay_us = 500,
	.reset_us = 1,
	.अंक_supplies = 1,
	.supply_names = { "vdd-supply" },
};

अटल स्थिर काष्ठा onboard_hub_तथ्य usb5744_तथ्य = {
	.init = usb5744_i2c_init,
	.power_on_delay_us = 1000,
	.reset_us = 5,
	.अंक_supplies = 1,
	.supply_names = { "vdd-supply" },
};

अटल स्थिर काष्ठा onboard_hub_तथ्य usbhx3_तथ्य = {
	.reset_us = 10000,
	.अंक_supplies = 2,
	.supply_names = { "vdd-supply", "vdd2-supply" },
};

अटल स्थिर काष्ठा udevice_id usb_onboard_hub_ids[] = {
	/* Use generic usbVID,PID dt-bindings (usb-device.yaml) */
	{	.compatible = "usb3431,6241",	/* Corechips SL6341 USB 2.0 */
		.तथ्य = (ulong)&corechips_sl6341_तथ्य,
	}, {
		.compatible = "usb3431,6341",	/* Corechips SL6341 USB 3.0 */
		.तथ्य = (ulong)&corechips_sl6341_तथ्य,
	}, {
		.compatible = "usb424,2514",	/* USB2514B USB 2.0 */
		.तथ्य = (ulong)&usb2514_तथ्य,
	}, {
		.compatible = "usb424,2744",	/* USB2744 USB 2.0 */
		.तथ्य = (ulong)&usb5744_तथ्य,
	}, {
		.compatible = "usb424,5744",	/* USB5744 USB 3.0 */
		.तथ्य = (ulong)&usb5744_तथ्य,
	}, {
		.compatible = "usb4b4,6504",	/* Cypress HX3 USB 3.0 */
		.तथ्य = (ulong)&usbhx3_तथ्य,
	}, {
		.compatible = "usb4b4,6506",	/* Cypress HX3 USB 2.0 */
		.तथ्य = (ulong)&usbhx3_तथ्य,
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
	.priv_auto = माप(काष्ठा onboard_hub),
};
