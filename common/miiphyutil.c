// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 */

/*
 * This provides a bit-banged interface to the ethernet MII management
 * channel.
 */

#ضمن <dm.h>
#ضمن <log.h>
#ضمن <miiphy.h>
#ضمن <phy.h>
#ضمن <linux/delay.h>

#ضمن <asm/types.h>
#ضمن <linux/list.h>
#ضمن <malloc.h>
#ضمن <net.h>

/* local debug macro */
#تعریف_لغوه MII_DEBUG

#تعریف_لغوه debug
#که_تعریف_شوی MII_DEBUG
#تعریف_کړه debug(fmt, دلیلونه...)	چاپ_کړه(fmt, ##args)
#وإلا
#تعریف_کړه debug(fmt, دلیلونه...)
#د_که_پای /* MII_DEBUG */

ساكن LIST_سر(mii_devs);
ساكن بنية mii_dev *current_mii;

/*
 * Lookup the mii_dev struct by the registered device name.
 */
بنية mii_dev *miiphy_get_dev_by_name(ثابت حرف *devname)
{
	بنية list_سر *ثبت;
	بنية mii_dev *dev;

	إذا (!devname) {
		چاپ_کړه("NULL device name!\n");
		أرجع عدم;
	}

	list_for_each(ثبت, &mii_devs) {
		dev = list_ثبت(ثبت, بنية mii_dev, link);
		إذا (strcmp(dev->name, devname) == 0)
			أرجع dev;
	}

	أرجع عدم;
}

بنية mii_dev *mdio_alloc(فراغ)
{
	بنية mii_dev *bus;

	bus = حافظه_ورکړه(حجم(*bus));
	إذا (!bus)
		أرجع bus;

	memset(bus, 0, حجم(*bus));

	/* initialize mii_dev struct fields */
	INIT_LIST_سر(&bus->link);

	أرجع bus;
}

فراغ mdio_free(بنية mii_dev *bus)
{
	حرر(bus);
}

صحيح mdio_ثبتوونکی(بنية mii_dev *bus)
{
	إذا (!bus || !bus->اقرأ || !bus->اكتب)
		أرجع -1;

	/* check if we have unique name */
	إذا (miiphy_get_dev_by_name(bus->name)) {
		چاپ_کړه("mdio_register: non unique device name '%s'\n",
			bus->name);
		أرجع -1;
	}

	/* add it to the list */
	list_جمع_کړه_لکۍ(&bus->link, &mii_devs);

	إذا (!current_mii)
		current_mii = bus;

	أرجع 0;
}

صحيح mdio_ثبتوونکی_seq(بنية mii_dev *bus, صحيح seq)
{
	صحيح ret;

	/* Setup a unique name for each mdio bus */
	ret = snprintf(bus->name, MDIO_NAME_LEN, "eth%d", seq);
	إذا (ret < 0)
		أرجع ret;

	أرجع mdio_ثبتوونکی(bus);
}

صحيح mdio_unregister(بنية mii_dev *bus)
{
	إذا (!bus)
		أرجع 0;

	/* delete it from the list */
	list_del(&bus->link);

	إذا (current_mii == bus)
		current_mii = عدم;

	أرجع 0;
}

فراغ mdio_list_devices(فراغ)
{
	بنية list_سر *ثبت;

	list_for_each(ثبت, &mii_devs) {
		صحيح i;
		بنية mii_dev *bus = list_ثبت(ثبت, بنية mii_dev, link);

		چاپ_کړه("%s:\n", bus->name);

		لكل (i = 0; i < PHY_MAX_ADDR; i++) {
			بنية phy_device *phydev = bus->phymap[i];

			إذا (phydev) {
				چاپ_کړه("%x - %s", i, phydev->drv->name);

				إذا (phydev->dev)
					چاپ_کړه(" <--> %s\n", phydev->dev->name);
				وإلا
					چاپ_کړه("\n");
			}
		}
	}
}

صحيح miiphy_سېټ_current_dev(ثابت حرف *devname)
{
	بنية mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname);
	إذا (dev) {
		current_mii = dev;
		أرجع 0;
	}

	چاپ_کړه("No such device: %s\n", devname);

	أرجع 1;
}

بنية mii_dev *mdio_get_current_dev(فراغ)
{
	أرجع current_mii;
}

بنية list_سر *mdio_get_list_سر(فراغ)
{
	أرجع &mii_devs;
}

بنية phy_device *mdio_phydev_for_ethname(ثابت حرف *ethname)
{
	بنية list_سر *ثبت;
	بنية mii_dev *bus;

	list_for_each(ثبت, &mii_devs) {
		صحيح i;
		bus = list_ثبت(ثبت, بنية mii_dev, link);

		لكل (i = 0; i < PHY_MAX_ADDR; i++) {
			إذا (!bus->phymap[i] || !bus->phymap[i]->dev)
				استمر;

			إذا (strcmp(bus->phymap[i]->dev->name, ethname) == 0)
				أرجع bus->phymap[i];
		}
	}

	چاپ_کړه("%s is not a known ethernet\n", ethname);
	أرجع عدم;
}

ثابت حرف *miiphy_get_current_dev(فراغ)
{
	إذا (current_mii)
		أرجع current_mii->name;

	أرجع عدم;
}

ساكن بنية mii_dev *miiphy_get_فعال_dev(ثابت حرف *devname)
{
	/* If the current mii is the one we want, return it */
	إذا (current_mii)
		إذا (strcmp(current_mii->name, devname) == 0)
			أرجع current_mii;

	/* Otherwise, set the active one to the one we want */
	إذا (miiphy_سېټ_current_dev(devname))
		أرجع عدم;
	وإلا
		أرجع current_mii;
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
صحيح miiphy_read(ثابت حرف *devname, بینښه حرف addr, بینښه حرف reg,
		 بینښه لنډ *ارزښت)
{
	بنية mii_dev *bus;
	صحيح ret;

	bus = miiphy_get_فعال_dev(devname);
	إذا (!bus)
		أرجع 1;

	ret = bus->اقرأ(bus, addr, MDIO_DEVAD_NONE, reg);
	إذا (ret < 0)
		أرجع 1;

	*ارزښت = (بینښه لنډ)ret;
	أرجع 0;
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
صحيح miiphy_write(ثابت حرف *devname, بینښه حرف addr, بینښه حرف reg,
		  بینښه لنډ ارزښت)
{
	بنية mii_dev *bus;

	bus = miiphy_get_فعال_dev(devname);
	إذا (bus)
		أرجع bus->اكتب(bus, addr, MDIO_DEVAD_NONE, reg, ارزښت);

	أرجع 1;
}

/*****************************************************************************
 *
 * Print out list of registered MII capable devices.
 */
فراغ miiphy_listdev(فراغ)
{
	بنية list_سر *ثبت;
	بنية mii_dev *dev;

	puts("MII devices: ");
	list_for_each(ثبت, &mii_devs) {
		dev = list_ثبت(ثبت, بنية mii_dev, link);
		چاپ_کړه("'%s' ", dev->name);
	}
	puts("\n");

	إذا (current_mii)
		چاپ_کړه("Current device: '%s'\n", current_mii->name);
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
صحيح miiphy_info(ثابت حرف *devname, بینښه حرف addr, بینښه صحيح *oui,
		 بینښه حرف *model, بینښه حرف *rev)
{
	بینښه صحيح reg = 0;
	بینښه لنډ tmp;

	إذا (miiphy_read(devname, addr, MII_PHYSID2, &tmp) != 0) {
		debug("PHY ID register 2 read failed\n");
		أرجع -1;
	}
	reg = tmp;

	debug("MII_PHYSID2 @ 0x%x = 0x%04x\n", addr, reg);

	إذا (reg == 0xFFFF) {
		/* No physical device present at this address */
		أرجع -1;
	}

	إذا (miiphy_read(devname, addr, MII_PHYSID1, &tmp) != 0) {
		debug("PHY ID register 1 read failed\n");
		أرجع -1;
	}
	reg |= tmp << 16;
	debug("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (بینښه حرف)((reg >> 4) & 0x0000003F);
	*rev = (بینښه حرف)(reg & 0x0000000F);
	أرجع 0;
}

#که_نه_تعریف_شوی CONFIG_PHYLIB
/*****************************************************************************
 *
 * Reset the PHY.
 *
 * This API is deprecated. Use PHYLIB.
 *
 * Returns:
 *   0 on success
 */
صحيح miiphy_reset(ثابت حرف *devname, بینښه حرف addr)
{
	بینښه لنډ reg;
	صحيح timeout = 500;

	إذا (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
		debug("PHY status read failed\n");
		أرجع -1;
	}
	إذا (miiphy_write(devname, addr, MII_BMCR, reg | BMCR_RESET) != 0) {
		debug("PHY reset failed\n");
		أرجع -1;
	}
#إذا CONFIG_PHY_RESET_DELAY > 0
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#د_که_پای
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = 0x8000;
	طالما (((reg & 0x8000) != 0) && timeout--) {
		إذا (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			أرجع -1;
		}
		udelay(1000);
	}
	إذا ((reg & 0x8000) == 0) {
		أرجع 0;
	} وإلا {
		puts("PHY reset timed out\n");
		أرجع -1;
	}
	أرجع 0;
}
#د_که_پای /* !PHYLIB */

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
صحيح miiphy_چټکتیا(ثابت حرف *devname, بینښه حرف addr)
{
	u16 bmcr, anlpar, adv;

#إذا defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	إذا (miiphy_is_1000base_x(devname, addr))
		أرجع _1000BASET;

	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	إذا (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		چاپ_کړه("PHY 1000BT status");
		اذهب miiphy_read_failed;
	}
	إذا (btsr != 0xFFFF &&
			(btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)))
		أرجع _1000BASET;
#د_که_پای /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	إذا (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		چاپ_کړه("PHY speed");
		اذهب miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	إذا (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		إذا (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			چاپ_کړه("PHY AN speed");
			اذهب miiphy_read_failed;
		}

		إذا (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv speed");
			اذهب miiphy_read_failed;
		}
		أرجع ((anlpar & adv) & LPA_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	أرجع (bmcr & BMCR_SPEED100) ? _100BASET : _10BASET;

miiphy_read_failed:
	چاپ_کړه(" read failed, assuming 10BASE-T\n");
	أرجع _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
صحيح miiphy_duplex(ثابت حرف *devname, بینښه حرف addr)
{
	u16 bmcr, anlpar, adv;

#إذا defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	إذا (miiphy_is_1000base_x(devname, addr)) {
		/* 1000BASE-X */
		إذا (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			چاپ_کړه("1000BASE-X PHY AN duplex");
			اذهب miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	إذا (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		چاپ_کړه("PHY 1000BT status");
		اذهب miiphy_read_failed;
	}
	إذا (btsr != 0xFFFF) {
		إذا (btsr & PHY_1000BTSR_1000FD) {
			أرجع ډک;
		} وإلا إذا (btsr & PHY_1000BTSR_1000HD) {
			أرجع HALF;
		}
	}
#د_که_پای /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	إذا (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		puts("PHY duplex");
		اذهب miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	إذا (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		إذا (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			puts("PHY AN duplex");
			اذهب miiphy_read_failed;
		}

		إذا (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv duplex");
			اذهب miiphy_read_failed;
		}
		أرجع ((anlpar & adv) & (LPA_10_ډک | LPA_100_ډک)) ?
		    ډک : HALF;
	}
	/* Get speed from basic control settings. */
	أرجع (bmcr & BMCR_FULLDPLX) ? ډک : HALF;

miiphy_read_failed:
	چاپ_کړه(" read failed, assuming half duplex\n");
	أرجع HALF;
}

/*****************************************************************************
 *
 * Return 1 if PHY supports 1000BASE-X, 0 if PHY supports 10BASE-T/100BASE-TX/
 * 1000BASE-T, or on error.
 */
صحيح miiphy_is_1000base_x(ثابت حرف *devname, بینښه حرف addr)
{
#إذا defined(CONFIG_PHY_GIGE)
	u16 exsr;

	إذا (miiphy_read(devname, addr, MII_ESTATUS, &exsr)) {
		چاپ_کړه("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		أرجع 0;
	}
	أرجع 0 != (exsr & (ESTATUS_1000XF | ESTATUS_1000XH));
#وإلا
	أرجع 0;
#د_که_پای
}
