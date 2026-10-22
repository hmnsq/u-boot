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
#undef MII_DEBUG

#undef debug
#ifdef MII_DEBUG
#define debug(fmt, args...)	printf(fmt, ##args)
#ܐܚܪܢܐ
#define debug(fmt, args...)
#endif /* MII_DEBUG */

ساكن LIST_HEAD(mii_devs);
ساكن بنية mii_dev *current_mii;

/*
 * Lookup the mii_dev struct by the registered device name.
 */
بنية mii_dev *miiphy_get_dev_by_name(ثابت حرف *devname)
{
	بنية list_head *entry;
	بنية mii_dev *dev;

	ܐܢ (!devname) {
		printf("NULL device name!\n");
		ܦܢܐ عدم;
	}

	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, بنية mii_dev, link);
		ܐܢ (strcmp(dev->name, devname) == 0)
			ܦܢܐ dev;
	}

	ܦܢܐ عدم;
}

بنية mii_dev *mdio_alloc(فراغ)
{
	بنية mii_dev *bus;

	bus = malloc(حجم(*bus));
	ܐܢ (!bus)
		ܦܢܐ bus;

	memset(bus, 0, حجم(*bus));

	/* initialize mii_dev struct fields */
	INIT_LIST_HEAD(&bus->link);

	ܦܢܐ bus;
}

فراغ mdio_free(بنية mii_dev *bus)
{
	حرر(bus);
}

صحيح mdio_register(بنية mii_dev *bus)
{
	ܐܢ (!bus || !bus->اقرأ || !bus->اكتب)
		ܦܢܐ -1;

	/* check if we have unique name */
	ܐܢ (miiphy_get_dev_by_name(bus->name)) {
		printf("mdio_register: non unique device name '%s'\n",
			bus->name);
		ܦܢܐ -1;
	}

	/* add it to the list */
	list_add_tail(&bus->link, &mii_devs);

	ܐܢ (!current_mii)
		current_mii = bus;

	ܦܢܐ 0;
}

صحيح mdio_register_seq(بنية mii_dev *bus, صحيح seq)
{
	صحيح ret;

	/* Setup a unique name for each mdio bus */
	ret = snprintf(bus->name, MDIO_NAME_LEN, "eth%d", seq);
	ܐܢ (ret < 0)
		ܦܢܐ ret;

	ܦܢܐ mdio_register(bus);
}

صحيح mdio_unregister(بنية mii_dev *bus)
{
	ܐܢ (!bus)
		ܦܢܐ 0;

	/* delete it from the list */
	list_del(&bus->link);

	ܐܢ (current_mii == bus)
		current_mii = عدم;

	ܦܢܐ 0;
}

فراغ mdio_list_devices(فراغ)
{
	بنية list_head *entry;

	list_for_each(entry, &mii_devs) {
		صحيح i;
		بنية mii_dev *bus = list_entry(entry, بنية mii_dev, link);

		printf("%s:\n", bus->name);

		لكل (i = 0; i < PHY_MAX_ADDR; i++) {
			بنية phy_device *phydev = bus->phymap[i];

			ܐܢ (phydev) {
				printf("%x - %s", i, phydev->drv->name);

				ܐܢ (phydev->dev)
					printf(" <--> %s\n", phydev->dev->name);
				ܐܚܪܢܐ
					printf("\n");
			}
		}
	}
}

صحيح miiphy_set_current_dev(ثابت حرف *devname)
{
	بنية mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname);
	ܐܢ (dev) {
		current_mii = dev;
		ܦܢܐ 0;
	}

	printf("No such device: %s\n", devname);

	ܦܢܐ 1;
}

بنية mii_dev *mdio_get_current_dev(فراغ)
{
	ܦܢܐ current_mii;
}

بنية list_head *mdio_get_list_head(فراغ)
{
	ܦܢܐ &mii_devs;
}

بنية phy_device *mdio_phydev_for_ethname(ثابت حرف *ethname)
{
	بنية list_head *entry;
	بنية mii_dev *bus;

	list_for_each(entry, &mii_devs) {
		صحيح i;
		bus = list_entry(entry, بنية mii_dev, link);

		لكل (i = 0; i < PHY_MAX_ADDR; i++) {
			ܐܢ (!bus->phymap[i] || !bus->phymap[i]->dev)
				استمر;

			ܐܢ (strcmp(bus->phymap[i]->dev->name, ethname) == 0)
				ܦܢܐ bus->phymap[i];
		}
	}

	printf("%s is not a known ethernet\n", ethname);
	ܦܢܐ عدم;
}

ثابت حرف *miiphy_get_current_dev(فراغ)
{
	ܐܢ (current_mii)
		ܦܢܐ current_mii->name;

	ܦܢܐ عدم;
}

ساكن بنية mii_dev *miiphy_get_active_dev(ثابت حرف *devname)
{
	/* If the current mii is the one we want, return it */
	ܐܢ (current_mii)
		ܐܢ (strcmp(current_mii->name, devname) == 0)
			ܦܢܐ current_mii;

	/* Otherwise, set the active one to the one we want */
	ܐܢ (miiphy_set_current_dev(devname))
		ܦܢܐ عدم;
	ܐܚܪܢܐ
		ܦܢܐ current_mii;
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
صحيح miiphy_read(ثابت حرف *devname, unsigned حرف addr, unsigned حرف reg,
		 unsigned short *value)
{
	بنية mii_dev *bus;
	صحيح ret;

	bus = miiphy_get_active_dev(devname);
	ܐܢ (!bus)
		ܦܢܐ 1;

	ret = bus->اقرأ(bus, addr, MDIO_DEVAD_NONE, reg);
	ܐܢ (ret < 0)
		ܦܢܐ 1;

	*value = (unsigned short)ret;
	ܦܢܐ 0;
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
صحيح miiphy_write(ثابت حرف *devname, unsigned حرف addr, unsigned حرف reg,
		  unsigned short value)
{
	بنية mii_dev *bus;

	bus = miiphy_get_active_dev(devname);
	ܐܢ (bus)
		ܦܢܐ bus->اكتب(bus, addr, MDIO_DEVAD_NONE, reg, value);

	ܦܢܐ 1;
}

/*****************************************************************************
 *
 * Print out list of registered MII capable devices.
 */
فراغ miiphy_listdev(فراغ)
{
	بنية list_head *entry;
	بنية mii_dev *dev;

	puts("MII devices: ");
	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, بنية mii_dev, link);
		printf("'%s' ", dev->name);
	}
	puts("\n");

	ܐܢ (current_mii)
		printf("Current device: '%s'\n", current_mii->name);
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
صحيح miiphy_info(ثابت حرف *devname, unsigned حرف addr, unsigned صحيح *oui,
		 unsigned حرف *model, unsigned حرف *rev)
{
	unsigned صحيح reg = 0;
	unsigned short tmp;

	ܐܢ (miiphy_read(devname, addr, MII_PHYSID2, &tmp) != 0) {
		debug("PHY ID register 2 read failed\n");
		ܦܢܐ -1;
	}
	reg = tmp;

	debug("MII_PHYSID2 @ 0x%x = 0x%04x\n", addr, reg);

	ܐܢ (reg == 0xFFFF) {
		/* No physical device present at this address */
		ܦܢܐ -1;
	}

	ܐܢ (miiphy_read(devname, addr, MII_PHYSID1, &tmp) != 0) {
		debug("PHY ID register 1 read failed\n");
		ܦܢܐ -1;
	}
	reg |= tmp << 16;
	debug("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (unsigned حرف)((reg >> 4) & 0x0000003F);
	*rev = (unsigned حرف)(reg & 0x0000000F);
	ܦܢܐ 0;
}

#ifndef CONFIG_PHYLIB
/*****************************************************************************
 *
 * Reset the PHY.
 *
 * This API is deprecated. Use PHYLIB.
 *
 * Returns:
 *   0 on success
 */
صحيح miiphy_reset(ثابت حرف *devname, unsigned حرف addr)
{
	unsigned short reg;
	صحيح timeout = 500;

	ܐܢ (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
		debug("PHY status read failed\n");
		ܦܢܐ -1;
	}
	ܐܢ (miiphy_write(devname, addr, MII_BMCR, reg | BMCR_RESET) != 0) {
		debug("PHY reset failed\n");
		ܦܢܐ -1;
	}
#ܐܢ CONFIG_PHY_RESET_DELAY > 0
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#endif
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = 0x8000;
	ܟܡܐ (((reg & 0x8000) != 0) && timeout--) {
		ܐܢ (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			ܦܢܐ -1;
		}
		udelay(1000);
	}
	ܐܢ ((reg & 0x8000) == 0) {
		ܦܢܐ 0;
	} ܐܚܪܢܐ {
		puts("PHY reset timed out\n");
		ܦܢܐ -1;
	}
	ܦܢܐ 0;
}
#endif /* !PHYLIB */

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
صحيح miiphy_speed(ثابت حرف *devname, unsigned حرف addr)
{
	u16 bmcr, anlpar, adv;

#ܐܢ defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	ܐܢ (miiphy_is_1000base_x(devname, addr))
		ܦܢܐ _1000BASET;

	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	ܐܢ (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		اذهب miiphy_read_failed;
	}
	ܐܢ (btsr != 0xFFFF &&
			(btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)))
		ܦܢܐ _1000BASET;
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	ܐܢ (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		printf("PHY speed");
		اذهب miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	ܐܢ (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		ܐܢ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("PHY AN speed");
			اذهب miiphy_read_failed;
		}

		ܐܢ (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv speed");
			اذهب miiphy_read_failed;
		}
		ܦܢܐ ((anlpar & adv) & LPA_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	ܦܢܐ (bmcr & BMCR_SPEED100) ? _100BASET : _10BASET;

miiphy_read_failed:
	printf(" read failed, assuming 10BASE-T\n");
	ܦܢܐ _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
صحيح miiphy_duplex(ثابت حرف *devname, unsigned حرف addr)
{
	u16 bmcr, anlpar, adv;

#ܐܢ defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	ܐܢ (miiphy_is_1000base_x(devname, addr)) {
		/* 1000BASE-X */
		ܐܢ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("1000BASE-X PHY AN duplex");
			اذهب miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	ܐܢ (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		اذهب miiphy_read_failed;
	}
	ܐܢ (btsr != 0xFFFF) {
		ܐܢ (btsr & PHY_1000BTSR_1000FD) {
			ܦܢܐ FULL;
		} ܐܚܪܢܐ ܐܢ (btsr & PHY_1000BTSR_1000HD) {
			ܦܢܐ HALF;
		}
	}
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	ܐܢ (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		puts("PHY duplex");
		اذهب miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	ܐܢ (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		ܐܢ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			puts("PHY AN duplex");
			اذهب miiphy_read_failed;
		}

		ܐܢ (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv duplex");
			اذهب miiphy_read_failed;
		}
		ܦܢܐ ((anlpar & adv) & (LPA_10FULL | LPA_100FULL)) ?
		    FULL : HALF;
	}
	/* Get speed from basic control settings. */
	ܦܢܐ (bmcr & BMCR_FULLDPLX) ? FULL : HALF;

miiphy_read_failed:
	printf(" read failed, assuming half duplex\n");
	ܦܢܐ HALF;
}

/*****************************************************************************
 *
 * Return 1 if PHY supports 1000BASE-X, 0 if PHY supports 10BASE-T/100BASE-TX/
 * 1000BASE-T, or on error.
 */
صحيح miiphy_is_1000base_x(ثابت حرف *devname, unsigned حرف addr)
{
#ܐܢ defined(CONFIG_PHY_GIGE)
	u16 exsr;

	ܐܢ (miiphy_read(devname, addr, MII_ESTATUS, &exsr)) {
		printf("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		ܦܢܐ 0;
	}
	ܦܢܐ 0 != (exsr & (ESTATUS_1000XF | ESTATUS_1000XH));
#ܐܚܪܢܐ
	ܦܢܐ 0;
#endif
}
