// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 */

/*
 * This provides a bit-banged interface to the ethernet MII management
 * channel.
 */

#شامل <dm.h>
#شامل <log.h>
#شامل <miiphy.h>
#شامل <phy.h>
#شامل <linux/delay.h>

#شامل <asm/types.h>
#شامل <linux/list.h>
#شامل <malloc.h>
#شامل <net.h>

/* local debug macro */
#تعریف_منسوخ MII_DEBUG

#تعریف_منسوخ debug
#اگر_تعریف_شدہ MII_DEBUG
#تعریف_کرو debug(fmt, دلائل...)	لکھو(fmt, ##args)
#ورنہ
#تعریف_کرو debug(fmt, دلائل...)
#اگر_ختم /* MII_DEBUG */

ساکن LIST_سر(mii_devs);
ساکن ساخت mii_dev *current_mii;

/*
 * Lookup the mii_dev struct by the registered device name.
 */
ساخت mii_dev *miiphy_get_dev_by_name(ثابت حرف *devname)
{
	ساخت list_سر *اندراج_2;
	ساخت mii_dev *dev;

	اگر (!devname) {
		لکھو("NULL device name!\n");
		واپس تہی;
	}

	list_for_each(اندراج_2, &mii_devs) {
		dev = list_اندراج(اندراج_2, ساخت mii_dev, link);
		اگر (strcmp(dev->name, devname) == 0)
			واپس dev;
	}

	واپس تہی;
}

ساخت mii_dev *mdio_alloc(خالی)
{
	ساخت mii_dev *bus;

	bus = حافظہ_دو(حجم(*bus));
	اگر (!bus)
		واپس bus;

	memset(bus, 0, حجم(*bus));

	/* initialize mii_dev struct fields */
	INIT_LIST_سر(&bus->link);

	واپس bus;
}

خالی mdio_free(ساخت mii_dev *bus)
{
	آزاد(bus);
}

صحیح mdio_رجسٹر(ساخت mii_dev *bus)
{
	اگر (!bus || !bus->اقرأ || !bus->اكتب)
		واپس -1;

	/* check if we have unique name */
	اگر (miiphy_get_dev_by_name(bus->name)) {
		لکھو("mdio_register: non unique device name '%s'\n",
			bus->name);
		واپس -1;
	}

	/* add it to the list */
	list_جوڑو_دم(&bus->link, &mii_devs);

	اگر (!current_mii)
		current_mii = bus;

	واپس 0;
}

صحیح mdio_رجسٹر_seq(ساخت mii_dev *bus, صحیح seq)
{
	صحیح ret;

	/* Setup a unique name for each mdio bus */
	ret = snprintf(bus->name, MDIO_NAME_LEN, "eth%d", seq);
	اگر (ret < 0)
		واپس ret;

	واپس mdio_رجسٹر(bus);
}

صحیح mdio_unregister(ساخت mii_dev *bus)
{
	اگر (!bus)
		واپس 0;

	/* delete it from the list */
	list_del(&bus->link);

	اگر (current_mii == bus)
		current_mii = تہی;

	واپس 0;
}

خالی mdio_list_devices(خالی)
{
	ساخت list_سر *اندراج_2;

	list_for_each(اندراج_2, &mii_devs) {
		صحیح i;
		ساخت mii_dev *bus = list_اندراج(اندراج_2, ساخت mii_dev, link);

		لکھو("%s:\n", bus->name);

		برائے (i = 0; i < PHY_MAX_ADDR; i++) {
			ساخت phy_device *phydev = bus->phymap[i];

			اگر (phydev) {
				لکھو("%x - %s", i, phydev->drv->name);

				اگر (phydev->dev)
					لکھو(" <--> %s\n", phydev->dev->name);
				ورنہ
					لکھو("\n");
			}
		}
	}
}

صحیح miiphy_سیٹ_current_dev(ثابت حرف *devname)
{
	ساخت mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname);
	اگر (dev) {
		current_mii = dev;
		واپس 0;
	}

	لکھو("No such device: %s\n", devname);

	واپس 1;
}

ساخت mii_dev *mdio_get_current_dev(خالی)
{
	واپس current_mii;
}

ساخت list_سر *mdio_get_list_سر(خالی)
{
	واپس &mii_devs;
}

ساخت phy_device *mdio_phydev_for_ethname(ثابت حرف *ethname)
{
	ساخت list_سر *اندراج_2;
	ساخت mii_dev *bus;

	list_for_each(اندراج_2, &mii_devs) {
		صحیح i;
		bus = list_اندراج(اندراج_2, ساخت mii_dev, link);

		برائے (i = 0; i < PHY_MAX_ADDR; i++) {
			اگر (!bus->phymap[i] || !bus->phymap[i]->dev)
				جاری;

			اگر (strcmp(bus->phymap[i]->dev->name, ethname) == 0)
				واپس bus->phymap[i];
		}
	}

	لکھو("%s is not a known ethernet\n", ethname);
	واپس تہی;
}

ثابت حرف *miiphy_get_current_dev(خالی)
{
	اگر (current_mii)
		واپس current_mii->name;

	واپس تہی;
}

ساکن ساخت mii_dev *miiphy_get_فعال_dev(ثابت حرف *devname)
{
	/* If the current mii is the one we want, return it */
	اگر (current_mii)
		اگر (strcmp(current_mii->name, devname) == 0)
			واپس current_mii;

	/* Otherwise, set the active one to the one we want */
	اگر (miiphy_سیٹ_current_dev(devname))
		واپس تہی;
	ورنہ
		واپس current_mii;
}

/*****************************************************************************
 *
 * Read to variable <value> from the PHY attached to device <devname>,
 * use PHY address <addr> and register <reg>.
 *
 * This API is deprecated. Use phy_read on a phy_device found via phy_connect
 *
 * Returns:
 *   0 on success
 */
صحیح miiphy_read(ثابت حرف *devname, غیرعلامتی حرف addr, غیرعلامتی حرف reg,
		 غیرعلامتی مختصر *قدر)
{
	ساخت mii_dev *bus;
	صحیح ret;

	bus = miiphy_get_فعال_dev(devname);
	اگر (!bus)
		واپس 1;

	ret = bus->اقرأ(bus, addr, MDIO_DEVAD_NONE, reg);
	اگر (ret < 0)
		واپس 1;

	*قدر = (غیرعلامتی مختصر)ret;
	واپس 0;
}

/*****************************************************************************
 *
 * Write <value> to the PHY attached to device <devname>,
 * use PHY address <addr> and register <reg>.
 *
 * This API is deprecated. Use phy_write on a phy_device found by phy_connect
 *
 * Returns:
 *   0 on success
 */
صحیح miiphy_write(ثابت حرف *devname, غیرعلامتی حرف addr, غیرعلامتی حرف reg,
		  غیرعلامتی مختصر قدر)
{
	ساخت mii_dev *bus;

	bus = miiphy_get_فعال_dev(devname);
	اگر (bus)
		واپس bus->اكتب(bus, addr, MDIO_DEVAD_NONE, reg, قدر);

	واپس 1;
}

/*****************************************************************************
 *
 * Print out list of registered MII capable devices.
 */
خالی miiphy_listdev(خالی)
{
	ساخت list_سر *اندراج_2;
	ساخت mii_dev *dev;

	puts("MII devices: ");
	list_for_each(اندراج_2, &mii_devs) {
		dev = list_اندراج(اندراج_2, ساخت mii_dev, link);
		لکھو("'%s' ", dev->name);
	}
	puts("\n");

	اگر (current_mii)
		لکھو("Current device: '%s'\n", current_mii->name);
}

/*****************************************************************************
 *
 * Read the OUI, manufacture's model number, and revision number.
 *
 * OUI:     22 bits (unsigned int)
 * Model:    6 bits (unsigned char)
 * Revision: 4 bits (unsigned char)
 *
 * This API is deprecated.
 *
 * Returns:
 *   0 on success
 */
صحیح miiphy_info(ثابت حرف *devname, غیرعلامتی حرف addr, غیرعلامتی صحیح *oui,
		 غیرعلامتی حرف *model, غیرعلامتی حرف *rev)
{
	غیرعلامتی صحیح reg = 0;
	غیرعلامتی مختصر tmp;

	اگر (miiphy_read(devname, addr, MII_PHYSID2, &tmp) != 0) {
		debug("PHY ID register 2 read failed\n");
		واپس -1;
	}
	reg = tmp;

	debug("MII_PHYSID2 @ 0x%x = 0x%04x\n", addr, reg);

	اگر (reg == 0xFFFF) {
		/* No physical device present at this address */
		واپس -1;
	}

	اگر (miiphy_read(devname, addr, MII_PHYSID1, &tmp) != 0) {
		debug("PHY ID register 1 read failed\n");
		واپس -1;
	}
	reg |= tmp << 16;
	debug("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (غیرعلامتی حرف)((reg >> 4) & 0x0000003F);
	*rev = (غیرعلامتی حرف)(reg & 0x0000000F);
	واپس 0;
}

#اگر_غیر_تعریف_شدہ CONFIG_PHYLIB
/*****************************************************************************
 *
 * Reset the PHY.
 *
 * This API is deprecated. Use PHYLIB.
 *
 * Returns:
 *   0 on success
 */
صحیح miiphy_reset(ثابت حرف *devname, غیرعلامتی حرف addr)
{
	غیرعلامتی مختصر reg;
	صحیح timeout = 500;

	اگر (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
		debug("PHY status read failed\n");
		واپس -1;
	}
	اگر (miiphy_write(devname, addr, MII_BMCR, reg | BMCR_RESET) != 0) {
		debug("PHY reset failed\n");
		واپس -1;
	}
#اگر CONFIG_PHY_RESET_DELAY > 0
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#اگر_ختم
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = 0x8000;
	جب (((reg & 0x8000) != 0) && timeout--) {
		اگر (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			واپس -1;
		}
		udelay(1000);
	}
	اگر ((reg & 0x8000) == 0) {
		واپس 0;
	} ورنہ {
		puts("PHY reset timed out\n");
		واپس -1;
	}
	واپس 0;
}
#اگر_ختم /* !PHYLIB */

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
صحیح miiphy_رفتار(ثابت حرف *devname, غیرعلامتی حرف addr)
{
	u16 bmcr, anlpar, adv;

#اگر defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	اگر (miiphy_is_1000base_x(devname, addr))
		واپس _1000BASET;

	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	اگر (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		لکھو("PHY 1000BT status");
		جاؤ miiphy_read_failed;
	}
	اگر (btsr != 0xFFFF &&
			(btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)))
		واپس _1000BASET;
#اگر_ختم /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	اگر (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		لکھو("PHY speed");
		جاؤ miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	اگر (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		اگر (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			لکھو("PHY AN speed");
			جاؤ miiphy_read_failed;
		}

		اگر (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv speed");
			جاؤ miiphy_read_failed;
		}
		واپس ((anlpar & adv) & LPA_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	واپس (bmcr & BMCR_SPEED100) ? _100BASET : _10BASET;

miiphy_read_failed:
	لکھو(" read failed, assuming 10BASE-T\n");
	واپس _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
صحیح miiphy_duplex(ثابت حرف *devname, غیرعلامتی حرف addr)
{
	u16 bmcr, anlpar, adv;

#اگر defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	اگر (miiphy_is_1000base_x(devname, addr)) {
		/* 1000BASE-X */
		اگر (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			لکھو("1000BASE-X PHY AN duplex");
			جاؤ miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	اگر (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		لکھو("PHY 1000BT status");
		جاؤ miiphy_read_failed;
	}
	اگر (btsr != 0xFFFF) {
		اگر (btsr & PHY_1000BTSR_1000FD) {
			واپس بھرا;
		} ورنہ اگر (btsr & PHY_1000BTSR_1000HD) {
			واپس HALF;
		}
	}
#اگر_ختم /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	اگر (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		puts("PHY duplex");
		جاؤ miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	اگر (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		اگر (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			puts("PHY AN duplex");
			جاؤ miiphy_read_failed;
		}

		اگر (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv duplex");
			جاؤ miiphy_read_failed;
		}
		واپس ((anlpar & adv) & (LPA_10_بھرا | LPA_100_بھرا)) ?
		    بھرا : HALF;
	}
	/* Get speed from basic control settings. */
	واپس (bmcr & BMCR_FULLDPLX) ? بھرا : HALF;

miiphy_read_failed:
	لکھو(" read failed, assuming half duplex\n");
	واپس HALF;
}

/*****************************************************************************
 *
 * Return 1 if PHY supports 1000BASE-X, 0 if PHY supports 10BASE-T/100BASE-TX/
 * 1000BASE-T, or on error.
 */
صحیح miiphy_is_1000base_x(ثابت حرف *devname, غیرعلامتی حرف addr)
{
#اگر defined(CONFIG_PHY_GIGE)
	u16 exsr;

	اگر (miiphy_read(devname, addr, MII_ESTATUS, &exsr)) {
		لکھو("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		واپس 0;
	}
	واپس 0 != (exsr & (ESTATUS_1000XF | ESTATUS_1000XH));
#ورنہ
	واپس 0;
#اگر_ختم
}
