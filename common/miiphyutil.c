// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 */

/*
 * This provides a bit-banged interface to the ethernet MII management
 * channel.
 */

#অন্তর্ভাব <dm.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <miiphy.h>
#অন্তর্ভাব <phy.h>
#অন্তর্ভাব <linux/delay.h>

#অন্তর্ভাব <asm/types.h>
#অন্তর্ভাব <linux/list.h>
#অন্তর্ভাব <malloc.h>
#অন্তর্ভাব <net.h>

/* local debug macro */
#undef MII_DEBUG

#undef debug
#ifdef MII_DEBUG
#define debug(fmt, args...)	printf(fmt, ##args)
#অন্যথা
#define debug(fmt, args...)
#endif /* MII_DEBUG */

স্থির LIST_HEAD(mii_devs);
স্থির সংরচনা mii_dev *current_mii;

/*
 * Lookup the mii_dev struct by the registered device name.
 */
সংরচনা mii_dev *miiphy_get_dev_by_name(ধ্রুব বর্ণ *devname)
{
	সংরচনা list_head *entry;
	সংরচনা mii_dev *dev;

	যদি (!devname) {
		printf("NULL device name!\n");
		প্রত্যাগম রিক্ত;
	}

	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, সংরচনা mii_dev, link);
		যদি (strcmp(dev->name, devname) == 0)
			প্রত্যাগম dev;
	}

	প্রত্যাগম রিক্ত;
}

সংরচনা mii_dev *mdio_alloc(শূন্য)
{
	সংরচনা mii_dev *bus;

	bus = malloc(পরিমাণ(*bus));
	যদি (!bus)
		প্রত্যাগম bus;

	memset(bus, 0, পরিমাণ(*bus));

	/* initialize mii_dev struct fields */
	INIT_LIST_HEAD(&bus->link);

	প্রত্যাগম bus;
}

শূন্য mdio_free(সংরচনা mii_dev *bus)
{
	মুক্ত(bus);
}

পূর্ণাঙ্ক mdio_register(সংরচনা mii_dev *bus)
{
	যদি (!bus || !bus->পঠ || !bus->লিখ)
		প্রত্যাগম -1;

	/* check if we have unique name */
	যদি (miiphy_get_dev_by_name(bus->name)) {
		printf("mdio_register: non unique device name '%s'\n",
			bus->name);
		প্রত্যাগম -1;
	}

	/* add it to the list */
	list_add_tail(&bus->link, &mii_devs);

	যদি (!current_mii)
		current_mii = bus;

	প্রত্যাগম 0;
}

পূর্ণাঙ্ক mdio_register_seq(সংরচনা mii_dev *bus, পূর্ণাঙ্ক seq)
{
	পূর্ণাঙ্ক ret;

	/* Setup a unique name for each mdio bus */
	ret = snprintf(bus->name, MDIO_NAME_LEN, "eth%d", seq);
	যদি (ret < 0)
		প্রত্যাগম ret;

	প্রত্যাগম mdio_register(bus);
}

পূর্ণাঙ্ক mdio_unregister(সংরচনা mii_dev *bus)
{
	যদি (!bus)
		প্রত্যাগম 0;

	/* delete it from the list */
	list_del(&bus->link);

	যদি (current_mii == bus)
		current_mii = রিক্ত;

	প্রত্যাগম 0;
}

শূন্য mdio_list_devices(শূন্য)
{
	সংরচনা list_head *entry;

	list_for_each(entry, &mii_devs) {
		পূর্ণাঙ্ক i;
		সংরচনা mii_dev *bus = list_entry(entry, সংরচনা mii_dev, link);

		printf("%s:\n", bus->name);

		প্রতি (i = 0; i < PHY_MAX_ADDR; i++) {
			সংরচনা phy_device *phydev = bus->phymap[i];

			যদি (phydev) {
				printf("%x - %s", i, phydev->drv->name);

				যদি (phydev->dev)
					printf(" <--> %s\n", phydev->dev->name);
				অন্যথা
					printf("\n");
			}
		}
	}
}

পূর্ণাঙ্ক miiphy_set_current_dev(ধ্রুব বর্ণ *devname)
{
	সংরচনা mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname);
	যদি (dev) {
		current_mii = dev;
		প্রত্যাগম 0;
	}

	printf("No such device: %s\n", devname);

	প্রত্যাগম 1;
}

সংরচনা mii_dev *mdio_get_current_dev(শূন্য)
{
	প্রত্যাগম current_mii;
}

সংরচনা list_head *mdio_get_list_head(শূন্য)
{
	প্রত্যাগম &mii_devs;
}

সংরচনা phy_device *mdio_phydev_for_ethname(ধ্রুব বর্ণ *ethname)
{
	সংরচনা list_head *entry;
	সংরচনা mii_dev *bus;

	list_for_each(entry, &mii_devs) {
		পূর্ণাঙ্ক i;
		bus = list_entry(entry, সংরচনা mii_dev, link);

		প্রতি (i = 0; i < PHY_MAX_ADDR; i++) {
			যদি (!bus->phymap[i] || !bus->phymap[i]->dev)
				অনুবর্ত;

			যদি (strcmp(bus->phymap[i]->dev->name, ethname) == 0)
				প্রত্যাগম bus->phymap[i];
		}
	}

	printf("%s is not a known ethernet\n", ethname);
	প্রত্যাগম রিক্ত;
}

ধ্রুব বর্ণ *miiphy_get_current_dev(শূন্য)
{
	যদি (current_mii)
		প্রত্যাগম current_mii->name;

	প্রত্যাগম রিক্ত;
}

স্থির সংরচনা mii_dev *miiphy_get_active_dev(ধ্রুব বর্ণ *devname)
{
	/* If the current mii is the one we want, return it */
	যদি (current_mii)
		যদি (strcmp(current_mii->name, devname) == 0)
			প্রত্যাগম current_mii;

	/* Otherwise, set the active one to the one we want */
	যদি (miiphy_set_current_dev(devname))
		প্রত্যাগম রিক্ত;
	অন্যথা
		প্রত্যাগম current_mii;
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
পূর্ণাঙ্ক miiphy_read(ধ্রুব বর্ণ *devname, unsigned বর্ণ addr, unsigned বর্ণ reg,
		 unsigned short *মান)
{
	সংরচনা mii_dev *bus;
	পূর্ণাঙ্ক ret;

	bus = miiphy_get_active_dev(devname);
	যদি (!bus)
		প্রত্যাগম 1;

	ret = bus->পঠ(bus, addr, MDIO_DEVAD_NONE, reg);
	যদি (ret < 0)
		প্রত্যাগম 1;

	*মান = (unsigned short)ret;
	প্রত্যাগম 0;
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
পূর্ণাঙ্ক miiphy_write(ধ্রুব বর্ণ *devname, unsigned বর্ণ addr, unsigned বর্ণ reg,
		  unsigned short মান)
{
	সংরচনা mii_dev *bus;

	bus = miiphy_get_active_dev(devname);
	যদি (bus)
		প্রত্যাগম bus->লিখ(bus, addr, MDIO_DEVAD_NONE, reg, মান);

	প্রত্যাগম 1;
}

/*****************************************************************************
 *
 * Print out list of registered MII capable devices.
 */
শূন্য miiphy_listdev(শূন্য)
{
	সংরচনা list_head *entry;
	সংরচনা mii_dev *dev;

	puts("MII devices: ");
	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, সংরচনা mii_dev, link);
		printf("'%s' ", dev->name);
	}
	puts("\n");

	যদি (current_mii)
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
পূর্ণাঙ্ক miiphy_info(ধ্রুব বর্ণ *devname, unsigned বর্ণ addr, unsigned পূর্ণাঙ্ক *oui,
		 unsigned বর্ণ *model, unsigned বর্ণ *rev)
{
	unsigned পূর্ণাঙ্ক reg = 0;
	unsigned short tmp;

	যদি (miiphy_read(devname, addr, MII_PHYSID2, &tmp) != 0) {
		debug("PHY ID register 2 read failed\n");
		প্রত্যাগম -1;
	}
	reg = tmp;

	debug("MII_PHYSID2 @ 0x%x = 0x%04x\n", addr, reg);

	যদি (reg == 0xFFFF) {
		/* No physical device present at this address */
		প্রত্যাগম -1;
	}

	যদি (miiphy_read(devname, addr, MII_PHYSID1, &tmp) != 0) {
		debug("PHY ID register 1 read failed\n");
		প্রত্যাগম -1;
	}
	reg |= tmp << 16;
	debug("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (unsigned বর্ণ)((reg >> 4) & 0x0000003F);
	*rev = (unsigned বর্ণ)(reg & 0x0000000F);
	প্রত্যাগম 0;
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
পূর্ণাঙ্ক miiphy_reset(ধ্রুব বর্ণ *devname, unsigned বর্ণ addr)
{
	unsigned short reg;
	পূর্ণাঙ্ক timeout = 500;

	যদি (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
		debug("PHY status read failed\n");
		প্রত্যাগম -1;
	}
	যদি (miiphy_write(devname, addr, MII_BMCR, reg | BMCR_RESET) != 0) {
		debug("PHY reset failed\n");
		প্রত্যাগম -1;
	}
#যদি CONFIG_PHY_RESET_DELAY > 0
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#endif
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = 0x8000;
	যাবত্ (((reg & 0x8000) != 0) && timeout--) {
		যদি (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			প্রত্যাগম -1;
		}
		udelay(1000);
	}
	যদি ((reg & 0x8000) == 0) {
		প্রত্যাগম 0;
	} অন্যথা {
		puts("PHY reset timed out\n");
		প্রত্যাগম -1;
	}
	প্রত্যাগম 0;
}
#endif /* !PHYLIB */

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
পূর্ণাঙ্ক miiphy_speed(ধ্রুব বর্ণ *devname, unsigned বর্ণ addr)
{
	u16 bmcr, anlpar, adv;

#যদি defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	যদি (miiphy_is_1000base_x(devname, addr))
		প্রত্যাগম _1000BASET;

	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	যদি (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		গচ্ছ miiphy_read_failed;
	}
	যদি (btsr != 0xFFFF &&
			(btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)))
		প্রত্যাগম _1000BASET;
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	যদি (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		printf("PHY speed");
		গচ্ছ miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	যদি (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		যদি (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("PHY AN speed");
			গচ্ছ miiphy_read_failed;
		}

		যদি (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv speed");
			গচ্ছ miiphy_read_failed;
		}
		প্রত্যাগম ((anlpar & adv) & LPA_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	প্রত্যাগম (bmcr & BMCR_SPEED100) ? _100BASET : _10BASET;

miiphy_read_failed:
	printf(" read failed, assuming 10BASE-T\n");
	প্রত্যাগম _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
পূর্ণাঙ্ক miiphy_duplex(ধ্রুব বর্ণ *devname, unsigned বর্ণ addr)
{
	u16 bmcr, anlpar, adv;

#যদি defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	যদি (miiphy_is_1000base_x(devname, addr)) {
		/* 1000BASE-X */
		যদি (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("1000BASE-X PHY AN duplex");
			গচ্ছ miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	যদি (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		গচ্ছ miiphy_read_failed;
	}
	যদি (btsr != 0xFFFF) {
		যদি (btsr & PHY_1000BTSR_1000FD) {
			প্রত্যাগম FULL;
		} অন্যথা যদি (btsr & PHY_1000BTSR_1000HD) {
			প্রত্যাগম HALF;
		}
	}
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	যদি (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		puts("PHY duplex");
		গচ্ছ miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	যদি (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		যদি (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			puts("PHY AN duplex");
			গচ্ছ miiphy_read_failed;
		}

		যদি (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv duplex");
			গচ্ছ miiphy_read_failed;
		}
		প্রত্যাগম ((anlpar & adv) & (LPA_10FULL | LPA_100FULL)) ?
		    FULL : HALF;
	}
	/* Get speed from basic control settings. */
	প্রত্যাগম (bmcr & BMCR_FULLDPLX) ? FULL : HALF;

miiphy_read_failed:
	printf(" read failed, assuming half duplex\n");
	প্রত্যাগম HALF;
}

/*****************************************************************************
 *
 * Return 1 if PHY supports 1000BASE-X, 0 if PHY supports 10BASE-T/100BASE-TX/
 * 1000BASE-T, or on error.
 */
পূর্ণাঙ্ক miiphy_is_1000base_x(ধ্রুব বর্ণ *devname, unsigned বর্ণ addr)
{
#যদি defined(CONFIG_PHY_GIGE)
	u16 exsr;

	যদি (miiphy_read(devname, addr, MII_ESTATUS, &exsr)) {
		printf("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		প্রত্যাগম 0;
	}
	প্রত্যাগম 0 != (exsr & (ESTATUS_1000XF | ESTATUS_1000XH));
#অন্যথা
	প্রত্যাগম 0;
#endif
}
