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

স্থির LIST_শীর্ষ(mii_devs);
স্থির কাঠামো mii_dev *current_mii;

/*
 * Lookup the mii_dev struct by the registered device name.
 */
কাঠামো mii_dev *miiphy_get_dev_by_name(ধ্রুবক অক্ষর *devname)
{
	কাঠামো list_শীর্ষ *entry;
	কাঠামো mii_dev *dev;

	যদি (!devname) {
		printf("NULL device name!\n");
		ফেরত ফাঁকা;
	}

	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, কাঠামো mii_dev, link);
		যদি (strcmp(dev->name, devname) == 0)
			ফেরত dev;
	}

	ফেরত ফাঁকা;
}

কাঠামো mii_dev *mdio_alloc(শূন্য)
{
	কাঠামো mii_dev *bus;

	bus = malloc(আকার(*bus));
	যদি (!bus)
		ফেরত bus;

	memset(bus, 0, আকার(*bus));

	/* initialize mii_dev struct fields */
	INIT_LIST_শীর্ষ(&bus->link);

	ফেরত bus;
}

শূন্য mdio_free(কাঠামো mii_dev *bus)
{
	মুক্ত(bus);
}

পূর্ণসংখ্যা mdio_register(কাঠামো mii_dev *bus)
{
	যদি (!bus || !bus->পঠ || !bus->লিখ)
		ফেরত -1;

	/* check if we have unique name */
	যদি (miiphy_get_dev_by_name(bus->name)) {
		printf("mdio_register: non unique device name '%s'\n",
			bus->name);
		ফেরত -1;
	}

	/* add it to the list */
	list_add_লেজ(&bus->link, &mii_devs);

	যদি (!current_mii)
		current_mii = bus;

	ফেরত 0;
}

পূর্ণসংখ্যা mdio_register_seq(কাঠামো mii_dev *bus, পূর্ণসংখ্যা seq)
{
	পূর্ণসংখ্যা ret;

	/* Setup a unique name for each mdio bus */
	ret = snprintf(bus->name, MDIO_NAME_LEN, "eth%d", seq);
	যদি (ret < 0)
		ফেরত ret;

	ফেরত mdio_register(bus);
}

পূর্ণসংখ্যা mdio_unregister(কাঠামো mii_dev *bus)
{
	যদি (!bus)
		ফেরত 0;

	/* delete it from the list */
	list_del(&bus->link);

	যদি (current_mii == bus)
		current_mii = ফাঁকা;

	ফেরত 0;
}

শূন্য mdio_list_devices(শূন্য)
{
	কাঠামো list_শীর্ষ *entry;

	list_for_each(entry, &mii_devs) {
		পূর্ণসংখ্যা i;
		কাঠামো mii_dev *bus = list_entry(entry, কাঠামো mii_dev, link);

		printf("%s:\n", bus->name);

		জন্য (i = 0; i < PHY_MAX_ADDR; i++) {
			কাঠামো phy_device *phydev = bus->phymap[i];

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

পূর্ণসংখ্যা miiphy_set_current_dev(ধ্রুবক অক্ষর *devname)
{
	কাঠামো mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname);
	যদি (dev) {
		current_mii = dev;
		ফেরত 0;
	}

	printf("No such device: %s\n", devname);

	ফেরত 1;
}

কাঠামো mii_dev *mdio_get_current_dev(শূন্য)
{
	ফেরত current_mii;
}

কাঠামো list_শীর্ষ *mdio_get_list_শীর্ষ(শূন্য)
{
	ফেরত &mii_devs;
}

কাঠামো phy_device *mdio_phydev_for_ethname(ধ্রুবক অক্ষর *ethname)
{
	কাঠামো list_শীর্ষ *entry;
	কাঠামো mii_dev *bus;

	list_for_each(entry, &mii_devs) {
		পূর্ণসংখ্যা i;
		bus = list_entry(entry, কাঠামো mii_dev, link);

		জন্য (i = 0; i < PHY_MAX_ADDR; i++) {
			যদি (!bus->phymap[i] || !bus->phymap[i]->dev)
				চালু;

			যদি (strcmp(bus->phymap[i]->dev->name, ethname) == 0)
				ফেরত bus->phymap[i];
		}
	}

	printf("%s is not a known ethernet\n", ethname);
	ফেরত ফাঁকা;
}

ধ্রুবক অক্ষর *miiphy_get_current_dev(শূন্য)
{
	যদি (current_mii)
		ফেরত current_mii->name;

	ফেরত ফাঁকা;
}

স্থির কাঠামো mii_dev *miiphy_get_active_dev(ধ্রুবক অক্ষর *devname)
{
	/* If the current mii is the one we want, return it */
	যদি (current_mii)
		যদি (strcmp(current_mii->name, devname) == 0)
			ফেরত current_mii;

	/* Otherwise, set the active one to the one we want */
	যদি (miiphy_set_current_dev(devname))
		ফেরত ফাঁকা;
	অন্যথা
		ফেরত current_mii;
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
পূর্ণসংখ্যা miiphy_read(ধ্রুবক অক্ষর *devname, unsigned অক্ষর addr, unsigned অক্ষর reg,
		 unsigned short *মান)
{
	কাঠামো mii_dev *bus;
	পূর্ণসংখ্যা ret;

	bus = miiphy_get_active_dev(devname);
	যদি (!bus)
		ফেরত 1;

	ret = bus->পঠ(bus, addr, MDIO_DEVAD_NONE, reg);
	যদি (ret < 0)
		ফেরত 1;

	*মান = (unsigned short)ret;
	ফেরত 0;
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
পূর্ণসংখ্যা miiphy_write(ধ্রুবক অক্ষর *devname, unsigned অক্ষর addr, unsigned অক্ষর reg,
		  unsigned short মান)
{
	কাঠামো mii_dev *bus;

	bus = miiphy_get_active_dev(devname);
	যদি (bus)
		ফেরত bus->লিখ(bus, addr, MDIO_DEVAD_NONE, reg, মান);

	ফেরত 1;
}

/*****************************************************************************
 *
 * Print out list of registered MII capable devices.
 */
শূন্য miiphy_listdev(শূন্য)
{
	কাঠামো list_শীর্ষ *entry;
	কাঠামো mii_dev *dev;

	puts("MII devices: ");
	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, কাঠামো mii_dev, link);
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
পূর্ণসংখ্যা miiphy_info(ধ্রুবক অক্ষর *devname, unsigned অক্ষর addr, unsigned পূর্ণসংখ্যা *oui,
		 unsigned অক্ষর *model, unsigned অক্ষর *rev)
{
	unsigned পূর্ণসংখ্যা reg = 0;
	unsigned short tmp;

	যদি (miiphy_read(devname, addr, MII_PHYSID2, &tmp) != 0) {
		debug("PHY ID register 2 read failed\n");
		ফেরত -1;
	}
	reg = tmp;

	debug("MII_PHYSID2 @ 0x%x = 0x%04x\n", addr, reg);

	যদি (reg == 0xFFFF) {
		/* No physical device present at this address */
		ফেরত -1;
	}

	যদি (miiphy_read(devname, addr, MII_PHYSID1, &tmp) != 0) {
		debug("PHY ID register 1 read failed\n");
		ফেরত -1;
	}
	reg |= tmp << 16;
	debug("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (unsigned অক্ষর)((reg >> 4) & 0x0000003F);
	*rev = (unsigned অক্ষর)(reg & 0x0000000F);
	ফেরত 0;
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
পূর্ণসংখ্যা miiphy_reset(ধ্রুবক অক্ষর *devname, unsigned অক্ষর addr)
{
	unsigned short reg;
	পূর্ণসংখ্যা timeout = 500;

	যদি (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
		debug("PHY status read failed\n");
		ফেরত -1;
	}
	যদি (miiphy_write(devname, addr, MII_BMCR, reg | BMCR_RESET) != 0) {
		debug("PHY reset failed\n");
		ফেরত -1;
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
	যখন (((reg & 0x8000) != 0) && timeout--) {
		যদি (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			ফেরত -1;
		}
		udelay(1000);
	}
	যদি ((reg & 0x8000) == 0) {
		ফেরত 0;
	} অন্যথা {
		puts("PHY reset timed out\n");
		ফেরত -1;
	}
	ফেরত 0;
}
#endif /* !PHYLIB */

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
পূর্ণসংখ্যা miiphy_speed(ধ্রুবক অক্ষর *devname, unsigned অক্ষর addr)
{
	u16 bmcr, anlpar, adv;

#যদি defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	যদি (miiphy_is_1000base_x(devname, addr))
		ফেরত _1000BASET;

	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	যদি (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		যাও miiphy_read_failed;
	}
	যদি (btsr != 0xFFFF &&
			(btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)))
		ফেরত _1000BASET;
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	যদি (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		printf("PHY speed");
		যাও miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	যদি (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		যদি (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("PHY AN speed");
			যাও miiphy_read_failed;
		}

		যদি (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv speed");
			যাও miiphy_read_failed;
		}
		ফেরত ((anlpar & adv) & LPA_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	ফেরত (bmcr & BMCR_SPEED100) ? _100BASET : _10BASET;

miiphy_read_failed:
	printf(" read failed, assuming 10BASE-T\n");
	ফেরত _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
পূর্ণসংখ্যা miiphy_duplex(ধ্রুবক অক্ষর *devname, unsigned অক্ষর addr)
{
	u16 bmcr, anlpar, adv;

#যদি defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	যদি (miiphy_is_1000base_x(devname, addr)) {
		/* 1000BASE-X */
		যদি (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("1000BASE-X PHY AN duplex");
			যাও miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	যদি (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		যাও miiphy_read_failed;
	}
	যদি (btsr != 0xFFFF) {
		যদি (btsr & PHY_1000BTSR_1000FD) {
			ফেরত FULL;
		} অন্যথা যদি (btsr & PHY_1000BTSR_1000HD) {
			ফেরত HALF;
		}
	}
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	যদি (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		puts("PHY duplex");
		যাও miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	যদি (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		যদি (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			puts("PHY AN duplex");
			যাও miiphy_read_failed;
		}

		যদি (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv duplex");
			যাও miiphy_read_failed;
		}
		ফেরত ((anlpar & adv) & (LPA_10FULL | LPA_100FULL)) ?
		    FULL : HALF;
	}
	/* Get speed from basic control settings. */
	ফেরত (bmcr & BMCR_FULLDPLX) ? FULL : HALF;

miiphy_read_failed:
	printf(" read failed, assuming half duplex\n");
	ফেরত HALF;
}

/*****************************************************************************
 *
 * Return 1 if PHY supports 1000BASE-X, 0 if PHY supports 10BASE-T/100BASE-TX/
 * 1000BASE-T, or on error.
 */
পূর্ণসংখ্যা miiphy_is_1000base_x(ধ্রুবক অক্ষর *devname, unsigned অক্ষর addr)
{
#যদি defined(CONFIG_PHY_GIGE)
	u16 exsr;

	যদি (miiphy_read(devname, addr, MII_ESTATUS, &exsr)) {
		printf("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		ফেরত 0;
	}
	ফেরত 0 != (exsr & (ESTATUS_1000XF | ESTATUS_1000XH));
#অন্যথা
	ফেরত 0;
#endif
}
