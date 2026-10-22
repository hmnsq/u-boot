// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for onboard USB hubs
 *
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 *
 * Mostly inspired by Linux kernel v6.1 onboard_usb_hub driver
 */

#ଅନ୍ତର୍ଭାଵ <asm/gpio.h>
#ଅନ୍ତର୍ଭାଵ <dm.h>
#ଅନ୍ତର୍ଭାଵ <dm/device_compat.h>
#ଅନ୍ତର୍ଭାଵ <dm/uclass-internal.h>
#ଅନ୍ତର୍ଭାଵ <i2c.h>
#ଅନ୍ତର୍ଭାଵ <linux/delay.h>
#ଅନ୍ତର୍ଭାଵ <power/regulator.h>

#define USB5744_COMMAND_ATTACH		0x0056
#define USB5744_COMMAND_ATTACH_LSB	0xAA
#define USB5744_CONFIG_REG_ACCESS	0x0037
#define USB5744_CONFIG_REG_ACCESS_LSB	0x99

#define MAX_SUPPLIES 2

ସଂରଚନା onboard_hub {
	ସଂରଚନା udevice *vdd[MAX_SUPPLIES];
	ସଂରଚନା gpio_desc *reset_gpio;
};

ସଂରଚନା onboard_hub_data {
	unsigned long reset_us;
	unsigned long power_on_delay_us;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_supplies;
	ଅଚଳ ଅକ୍ଷର * ଅଚଳ supply_names[MAX_SUPPLIES];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*init)(ସଂରଚନା udevice *dev);
};

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb5744_i2c_init(ସଂରଚନା udevice *dev)
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
	ସଂରଚନା udevice *i2c_bus = ଖାଲି, *i2c_dev;
	ସଂରଚନା ofnode_phandle_args phandle;
	u8 buf = USB5744_COMMAND_ATTACH;
	ସଂରଚନା dm_i2c_chip *i2c_chip;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret, slave_addr;

	ret = dev_read_phandle_with_args(dev, "i2c-bus", ଖାଲି, 0, 0, &phandle);
	ଯଦି (ret == -ENOENT) {
		dev_dbg(dev, "i2c-bus not specified\n");
		ଫେରନ୍ତୁ 0;
	}

	ଯଦି (ret) {
		dev_err(dev, "i2c-bus read failed\n");
		ଫେରନ୍ତୁ ret;
	}

	ret = device_get_global_by_ofnode(ofnode_get_parent(phandle.ଗଣ୍ଠି), &i2c_bus);
	ଯଦି (ret) {
		dev_err(dev, "Failed to get i2c node, err: %d\n", ret);
		ଫେରନ୍ତୁ ret;
	}

	ret = ofnode_read_u32(phandle.ଗଣ୍ଠି, "reg", &slave_addr);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;

	ret = i2c_get_chip(i2c_bus, slave_addr, 1, &i2c_dev);
	ଯଦି (ret) {
		dev_err(dev, "%s: can't find i2c chip device for addr 0x%x\n", __func__,
			slave_addr);
		ଫେରନ୍ତୁ ret;
	}

	i2c_chip = dev_get_parent_plat(i2c_dev);
	ଯଦି (!i2c_chip) {
		dev_err(dev, "parent platform data not found\n");
		ଫେରନ୍ତୁ -EINVAL;
	}

	i2c_chip->flags &= ~DM_I2C_CHIP_WR_ADDRESS;
	/* SMBus write command */
	ret = dm_i2c_write(i2c_dev, 0, (uint8_t *)&data_buf, 8);
	ଯଦି (ret) {
		dev_err(dev, "data_buf i2c_write failed, err:%d\n", ret);
		ଫେରନ୍ତୁ ret;
	}

	/* Configuration register access command */
	ret = dm_i2c_write(i2c_dev, USB5744_CONFIG_REG_ACCESS_LSB,
			   &config_reg_access_buf, 2);
	ଯଦି (ret) {
		dev_err(dev, "config_reg_access i2c_write failed, err: %d\n", ret);
		ଫେରନ୍ତୁ ret;
	}

	/* USB Attach with SMBus */
	ret = dm_i2c_write(i2c_dev, USB5744_COMMAND_ATTACH_LSB, &buf, 2);
	ଯଦି (ret) {
		dev_err(dev, "usb_attach i2c_write failed, err: %d\n", ret);
		ଫେରନ୍ତୁ ret;
	}

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_onboard_hub_reset(ସଂରଚନା udevice *dev)
{
	ସଂରଚନା onboard_hub_data *data =
		(ସଂରଚନା onboard_hub_data *)dev_get_driver_data(dev);
	ସଂରଚନା onboard_hub *hub = dev_get_priv(dev);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	hub->reset_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_IS_OUT);

	/* property is optional, don't return error! */
	ଯଦି (!hub->reset_gpio)
		ଫେରନ୍ତୁ 0;

	ret = dm_gpio_set_ମୂଲ୍ୟ(hub->reset_gpio, 1);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;

	udelay(data->reset_us);

	ret = dm_gpio_set_ମୂଲ୍ୟ(hub->reset_gpio, 0);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;

	udelay(data->power_on_delay_us);

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_onboard_hub_power_off(ସଂରଚନା udevice *dev)
{
	ସଂରଚନା onboard_hub_data *data =
		(ସଂରଚନା onboard_hub_data *)dev_get_driver_data(dev);
	ସଂରଚନା onboard_hub *hub = dev_get_priv(dev);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0, ret2;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପାଇଁ (i = data->ଅଙ୍କ_supplies; i > 0; i--) {
		ଯଦି (hub->vdd[i-1]) {
			ret2 = regulator_set_enable_if_allowed(hub->vdd[i-1], false);
			ଯଦି (ret2 && ret2 != -ENOSYS) {
				dev_err(dev, "can't disable %s: %d\n", data->supply_names[i-1], ret2);
				ret |= ret2;
			}
		}
	}

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_onboard_hub_probe(ସଂରଚନା udevice *dev)
{
	ସଂରଚନା onboard_hub_data *data =
		(ସଂରଚନା onboard_hub_data *)dev_get_driver_data(dev);
	ସଂରଚନା onboard_hub *hub = dev_get_priv(dev);
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ଯଦି (data->ଅଙ୍କ_supplies > MAX_SUPPLIES) {
		dev_err(dev, "invalid supplies number, max supported: %d\n", MAX_SUPPLIES);
		ଫେରନ୍ତୁ -EINVAL;
	}

	ପାଇଁ (i = 0; i < data->ଅଙ୍କ_supplies; i++) {
		ret = device_get_supply_regulator(dev, data->supply_names[i], &hub->vdd[i]);
		ଯଦି (ret && ret != -ENOENT && ret != -ENOSYS) {
			dev_err(dev, "can't get %s: %d\n", data->supply_names[i], ret);
			ଯାଅ err_supply;
		}

		ଯଦି (hub->vdd[i]) {
			ret = regulator_set_enable_if_allowed(hub->vdd[i], true);
			ଯଦି (ret && ret != -ENOSYS) {
				dev_err(dev, "can't enable %s: %d\n", data->supply_names[i], ret);
				ଯାଅ err_supply;
			}
		}
	}

	ret = usb_onboard_hub_reset(dev);
	ଯଦି (ret)
		ଯାଅ err_supply;

	ଯଦି (data->init) {
		ret = data->init(dev);
		ଯଦି (ret) {
			dev_err(dev, "onboard i2c init failed: %d\n", ret);
			ଯାଅ err;
		}
	}
	ଫେରନ୍ତୁ 0;
err:
	dm_gpio_set_ମୂଲ୍ୟ(hub->reset_gpio, 0);
err_supply:
	usb_onboard_hub_power_off(dev);
	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_onboard_hub_bind(ସଂରଚନା udevice *dev)
{
	ସଂରଚନା ofnode_phandle_args phandle;
	ସଂରଚନା udevice *peerdev;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ret = dev_read_phandle_with_args(dev, "peer-hub", ଖାଲି, 0, 0, &phandle);
	ଯଦି (ret == -ENOENT) {
		dev_dbg(dev, "peer-hub property not present\n");
		ଫେରନ୍ତୁ 0;
	}

	ଯଦି (ret) {
		dev_err(dev, "peer-hub not specified\n");
		ଫେରନ୍ତୁ ret;
	}

	ret = uclass_find_device_by_ofnode(UCLASS_USB_HUB, phandle.ଗଣ୍ଠି, &peerdev);
	ଯଦି (ret) {
		dev_dbg(dev, "binding before peer-hub %s\n",
			ofnode_get_name(phandle.ଗଣ୍ଠି));
		ଫେରନ୍ତୁ 0;
	}

	dev_dbg(dev, "peer-hub %s has been bound\n", peerdev->name);

	ଫେରନ୍ତୁ -ENODEV;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_onboard_hub_remove(ସଂରଚନା udevice *dev)
{
	ସଂରଚନା onboard_hub *hub = dev_get_priv(dev);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;

	ଯଦି (hub->reset_gpio) {
		ret = dm_gpio_set_ମୂଲ୍ୟ(hub->reset_gpio, 1);
		ଯଦି (ret)
			dev_err(dev, "can't set gpio %s: %d\n", hub->reset_gpio->dev->name,
				ret);
	}

	ret |= usb_onboard_hub_power_off(dev);
	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ଅଚଳ ସଂରଚନା onboard_hub_data corechips_sl6341_data = {
	.reset_us = 10000,
	.ଅଙ୍କ_supplies = 2,
	.supply_names = { "vdd1v1-supply", "vdd3v3-supply" },
};

ସ୍ଥିର ଅଚଳ ସଂରଚନା onboard_hub_data usb2514_data = {
	.power_on_delay_us = 500,
	.reset_us = 1,
	.ଅଙ୍କ_supplies = 1,
	.supply_names = { "vdd-supply" },
};

ସ୍ଥିର ଅଚଳ ସଂରଚନା onboard_hub_data usb5744_data = {
	.init = usb5744_i2c_init,
	.power_on_delay_us = 1000,
	.reset_us = 5,
	.ଅଙ୍କ_supplies = 1,
	.supply_names = { "vdd-supply" },
};

ସ୍ଥିର ଅଚଳ ସଂରଚନା onboard_hub_data usbhx3_data = {
	.reset_us = 10000,
	.ଅଙ୍କ_supplies = 2,
	.supply_names = { "vdd-supply", "vdd2-supply" },
};

ସ୍ଥିର ଅଚଳ ସଂରଚନା udevice_id usb_onboard_hub_ids[] = {
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
	.priv_auto = ଆକାର(ସଂରଚନା onboard_hub),
};
