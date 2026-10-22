// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for onboard USB hubs
 *
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 *
 * Mostly inspired by Linux kernel v6.1 onboard_usb_hub driver
 */

#include <asm/gpio.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/uclass-internal.h>
#include <i2c.h>
#include <linux/delay.h>
#include <power/regulator.h>

#define USB5744_COMMAND_ATTACH		0x0056
#define USB5744_COMMAND_ATTACH_LSB	0xAA
#define USB5744_CONFIG_REG_ACCESS	0x0037
#define USB5744_CONFIG_REG_ACCESS_LSB	0x99

#define MAX_SUPPLIES 2

struct onboard_hub {
	struct udevice *vdd[MAX_SUPPLIES];
	struct gpio_desc *reset_gpio;
};

struct onboard_hub_data {
	unsigned long reset_us;
	unsigned long power_on_delay_us;
	unsigned 整数 num_supplies;
	const char * const supply_names[MAX_SUPPLIES];
	整数 (*init)(struct udevice *dev);
};

static 整数 usb5744_i2c_init(struct udevice *dev)
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
	struct udevice *i2c_bus = NULL, *i2c_dev;
	struct ofnode_phandle_args phandle;
	u8 buf = USB5744_COMMAND_ATTACH;
	struct dm_i2c_chip *i2c_chip;
	整数 ret, slave_addr;

	ret = dev_read_phandle_with_args(dev, "i2c-bus", NULL, 0, 0, &phandle);
	もし (ret == -ENOENT) {
		dev_dbg(dev, "i2c-bus not specified\n");
		戻る 0;
	}

	もし (ret) {
		dev_err(dev, "i2c-bus read failed\n");
		戻る ret;
	}

	ret = device_get_global_by_ofnode(ofnode_get_parent(phandle.node), &i2c_bus);
	もし (ret) {
		dev_err(dev, "Failed to get i2c node, err: %d\n", ret);
		戻る ret;
	}

	ret = ofnode_read_u32(phandle.node, "reg", &slave_addr);
	もし (ret)
		戻る ret;

	ret = i2c_get_chip(i2c_bus, slave_addr, 1, &i2c_dev);
	もし (ret) {
		dev_err(dev, "%s: can't find i2c chip device for addr 0x%x\n", __func__,
			slave_addr);
		戻る ret;
	}

	i2c_chip = dev_get_parent_plat(i2c_dev);
	もし (!i2c_chip) {
		dev_err(dev, "parent platform data not found\n");
		戻る -EINVAL;
	}

	i2c_chip->flags &= ~DM_I2C_CHIP_WR_ADDRESS;
	/* SMBus write command */
	ret = dm_i2c_write(i2c_dev, 0, (uint8_t *)&data_buf, 8);
	もし (ret) {
		dev_err(dev, "data_buf i2c_write failed, err:%d\n", ret);
		戻る ret;
	}

	/* Configuration register access command */
	ret = dm_i2c_write(i2c_dev, USB5744_CONFIG_REG_ACCESS_LSB,
			   &config_reg_access_buf, 2);
	もし (ret) {
		dev_err(dev, "config_reg_access i2c_write failed, err: %d\n", ret);
		戻る ret;
	}

	/* USB Attach with SMBus */
	ret = dm_i2c_write(i2c_dev, USB5744_COMMAND_ATTACH_LSB, &buf, 2);
	もし (ret) {
		dev_err(dev, "usb_attach i2c_write failed, err: %d\n", ret);
		戻る ret;
	}

	戻る 0;
}

整数 usb_onboard_hub_reset(struct udevice *dev)
{
	struct onboard_hub_data *data =
		(struct onboard_hub_data *)dev_get_driver_data(dev);
	struct onboard_hub *hub = dev_get_priv(dev);
	整数 ret;

	hub->reset_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_IS_OUT);

	/* property is optional, don't return error! */
	もし (!hub->reset_gpio)
		戻る 0;

	ret = dm_gpio_set_value(hub->reset_gpio, 1);
	もし (ret)
		戻る ret;

	udelay(data->reset_us);

	ret = dm_gpio_set_value(hub->reset_gpio, 0);
	もし (ret)
		戻る ret;

	udelay(data->power_on_delay_us);

	戻る 0;
}

static 整数 usb_onboard_hub_power_off(struct udevice *dev)
{
	struct onboard_hub_data *data =
		(struct onboard_hub_data *)dev_get_driver_data(dev);
	struct onboard_hub *hub = dev_get_priv(dev);
	整数 ret = 0, ret2;
	unsigned 整数 i;

	ために (i = data->num_supplies; i > 0; i--) {
		もし (hub->vdd[i-1]) {
			ret2 = regulator_set_enable_if_allowed(hub->vdd[i-1], false);
			もし (ret2 && ret2 != -ENOSYS) {
				dev_err(dev, "can't disable %s: %d\n", data->supply_names[i-1], ret2);
				ret |= ret2;
			}
		}
	}

	戻る ret;
}

static 整数 usb_onboard_hub_probe(struct udevice *dev)
{
	struct onboard_hub_data *data =
		(struct onboard_hub_data *)dev_get_driver_data(dev);
	struct onboard_hub *hub = dev_get_priv(dev);
	unsigned 整数 i;
	整数 ret;

	もし (data->num_supplies > MAX_SUPPLIES) {
		dev_err(dev, "invalid supplies number, max supported: %d\n", MAX_SUPPLIES);
		戻る -EINVAL;
	}

	ために (i = 0; i < data->num_supplies; i++) {
		ret = device_get_supply_regulator(dev, data->supply_names[i], &hub->vdd[i]);
		もし (ret && ret != -ENOENT && ret != -ENOSYS) {
			dev_err(dev, "can't get %s: %d\n", data->supply_names[i], ret);
			goto err_supply;
		}

		もし (hub->vdd[i]) {
			ret = regulator_set_enable_if_allowed(hub->vdd[i], true);
			もし (ret && ret != -ENOSYS) {
				dev_err(dev, "can't enable %s: %d\n", data->supply_names[i], ret);
				goto err_supply;
			}
		}
	}

	ret = usb_onboard_hub_reset(dev);
	もし (ret)
		goto err_supply;

	もし (data->init) {
		ret = data->init(dev);
		もし (ret) {
			dev_err(dev, "onboard i2c init failed: %d\n", ret);
			goto err;
		}
	}
	戻る 0;
err:
	dm_gpio_set_value(hub->reset_gpio, 0);
err_supply:
	usb_onboard_hub_power_off(dev);
	戻る ret;
}

static 整数 usb_onboard_hub_bind(struct udevice *dev)
{
	struct ofnode_phandle_args phandle;
	struct udevice *peerdev;
	整数 ret;

	ret = dev_read_phandle_with_args(dev, "peer-hub", NULL, 0, 0, &phandle);
	もし (ret == -ENOENT) {
		dev_dbg(dev, "peer-hub property not present\n");
		戻る 0;
	}

	もし (ret) {
		dev_err(dev, "peer-hub not specified\n");
		戻る ret;
	}

	ret = uclass_find_device_by_ofnode(UCLASS_USB_HUB, phandle.node, &peerdev);
	もし (ret) {
		dev_dbg(dev, "binding before peer-hub %s\n",
			ofnode_get_name(phandle.node));
		戻る 0;
	}

	dev_dbg(dev, "peer-hub %s has been bound\n", peerdev->name);

	戻る -ENODEV;
}

static 整数 usb_onboard_hub_remove(struct udevice *dev)
{
	struct onboard_hub *hub = dev_get_priv(dev);
	整数 ret = 0;

	もし (hub->reset_gpio) {
		ret = dm_gpio_set_value(hub->reset_gpio, 1);
		もし (ret)
			dev_err(dev, "can't set gpio %s: %d\n", hub->reset_gpio->dev->name,
				ret);
	}

	ret |= usb_onboard_hub_power_off(dev);
	戻る ret;
}

static const struct onboard_hub_data corechips_sl6341_data = {
	.reset_us = 10000,
	.num_supplies = 2,
	.supply_names = { "vdd1v1-supply", "vdd3v3-supply" },
};

static const struct onboard_hub_data usb2514_data = {
	.power_on_delay_us = 500,
	.reset_us = 1,
	.num_supplies = 1,
	.supply_names = { "vdd-supply" },
};

static const struct onboard_hub_data usb5744_data = {
	.init = usb5744_i2c_init,
	.power_on_delay_us = 1000,
	.reset_us = 5,
	.num_supplies = 1,
	.supply_names = { "vdd-supply" },
};

static const struct onboard_hub_data usbhx3_data = {
	.reset_us = 10000,
	.num_supplies = 2,
	.supply_names = { "vdd-supply", "vdd2-supply" },
};

static const struct udevice_id usb_onboard_hub_ids[] = {
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
	.priv_auto = sizeof(struct onboard_hub),
};
