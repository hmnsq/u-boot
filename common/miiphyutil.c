// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 */

/*
 * This provides a bit-banged interface to the ethernet MII management
 * channel.
 */

#ಅನ್ತರ್ಭಾವ <dm.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <miiphy.h>
#ಅನ್ತರ್ಭಾವ <phy.h>
#ಅನ್ತರ್ಭಾವ <linux/delay.h>

#ಅನ್ತರ್ಭಾವ <asm/types.h>
#ಅನ್ತರ್ಭಾವ <linux/list.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <net.h>

/* local debug macro */
#undef MII_DEBUG

#undef debug
#ifdef MII_DEBUG
#define debug(fmt, args...)	printf(fmt, ##args)
#ಅನ್ಯಥಾ
#define debug(fmt, args...)
#endif /* MII_DEBUG */

ಸ್ಥಿರ LIST_HEAD(mii_devs);
ಸ್ಥಿರ ಸಂರಚನಾ mii_dev *current_mii;

/*
 * Lookup the mii_dev struct by the registered device name.
 */
ಸಂರಚನಾ mii_dev *miiphy_get_dev_by_name(ಧ್ರುವ ವರ್ಣ *devname)
{
	ಸಂರಚನಾ list_head *entry;
	ಸಂರಚನಾ mii_dev *dev;

	ಯದಿ (!devname) {
		printf("NULL device name!\n");
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, ಸಂರಚನಾ mii_dev, link);
		ಯದಿ (strcmp(dev->name, devname) == 0)
			ಪ್ರತ್ಯಾಗಮ dev;
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸಂರಚನಾ mii_dev *mdio_alloc(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ mii_dev *bus;

	bus = malloc(ಪರಿಮಾಣ(*bus));
	ಯದಿ (!bus)
		ಪ್ರತ್ಯಾಗಮ bus;

	memset(bus, 0, ಪರಿಮಾಣ(*bus));

	/* initialize mii_dev struct fields */
	INIT_LIST_HEAD(&bus->link);

	ಪ್ರತ್ಯಾಗಮ bus;
}

ಶೂನ್ಯ mdio_free(ಸಂರಚನಾ mii_dev *bus)
{
	ಮುಕ್ತ(bus);
}

ಪೂರ್ಣಾಙ್ಕ mdio_register(ಸಂರಚನಾ mii_dev *bus)
{
	ಯದಿ (!bus || !bus->ಪಠ || !bus->ಲಿಖ)
		ಪ್ರತ್ಯಾಗಮ -1;

	/* check if we have unique name */
	ಯದಿ (miiphy_get_dev_by_name(bus->name)) {
		printf("mdio_register: non unique device name '%s'\n",
			bus->name);
		ಪ್ರತ್ಯಾಗಮ -1;
	}

	/* add it to the list */
	list_add_tail(&bus->link, &mii_devs);

	ಯದಿ (!current_mii)
		current_mii = bus;

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ mdio_register_seq(ಸಂರಚನಾ mii_dev *bus, ಪೂರ್ಣಾಙ್ಕ seq)
{
	ಪೂರ್ಣಾಙ್ಕ ret;

	/* Setup a unique name for each mdio bus */
	ret = snprintf(bus->name, MDIO_NAME_LEN, "eth%d", seq);
	ಯದಿ (ret < 0)
		ಪ್ರತ್ಯಾಗಮ ret;

	ಪ್ರತ್ಯಾಗಮ mdio_register(bus);
}

ಪೂರ್ಣಾಙ್ಕ mdio_unregister(ಸಂರಚನಾ mii_dev *bus)
{
	ಯದಿ (!bus)
		ಪ್ರತ್ಯಾಗಮ 0;

	/* delete it from the list */
	list_del(&bus->link);

	ಯದಿ (current_mii == bus)
		current_mii = ರಿಕ್ತ;

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಶೂನ್ಯ mdio_list_devices(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ list_head *entry;

	list_for_each(entry, &mii_devs) {
		ಪೂರ್ಣಾಙ್ಕ i;
		ಸಂರಚನಾ mii_dev *bus = list_entry(entry, ಸಂರಚನಾ mii_dev, link);

		printf("%s:\n", bus->name);

		ಪ್ರತಿ (i = 0; i < PHY_MAX_ADDR; i++) {
			ಸಂರಚನಾ phy_device *phydev = bus->phymap[i];

			ಯದಿ (phydev) {
				printf("%x - %s", i, phydev->drv->name);

				ಯದಿ (phydev->dev)
					printf(" <--> %s\n", phydev->dev->name);
				ಅನ್ಯಥಾ
					printf("\n");
			}
		}
	}
}

ಪೂರ್ಣಾಙ್ಕ miiphy_set_current_dev(ಧ್ರುವ ವರ್ಣ *devname)
{
	ಸಂರಚನಾ mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname);
	ಯದಿ (dev) {
		current_mii = dev;
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	printf("No such device: %s\n", devname);

	ಪ್ರತ್ಯಾಗಮ 1;
}

ಸಂರಚನಾ mii_dev *mdio_get_current_dev(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ current_mii;
}

ಸಂರಚನಾ list_head *mdio_get_list_head(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ &mii_devs;
}

ಸಂರಚನಾ phy_device *mdio_phydev_for_ethname(ಧ್ರುವ ವರ್ಣ *ethname)
{
	ಸಂರಚನಾ list_head *entry;
	ಸಂರಚನಾ mii_dev *bus;

	list_for_each(entry, &mii_devs) {
		ಪೂರ್ಣಾಙ್ಕ i;
		bus = list_entry(entry, ಸಂರಚನಾ mii_dev, link);

		ಪ್ರತಿ (i = 0; i < PHY_MAX_ADDR; i++) {
			ಯದಿ (!bus->phymap[i] || !bus->phymap[i]->dev)
				ಅನುವರ್ತ;

			ಯದಿ (strcmp(bus->phymap[i]->dev->name, ethname) == 0)
				ಪ್ರತ್ಯಾಗಮ bus->phymap[i];
		}
	}

	printf("%s is not a known ethernet\n", ethname);
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಧ್ರುವ ವರ್ಣ *miiphy_get_current_dev(ಶೂನ್ಯ)
{
	ಯದಿ (current_mii)
		ಪ್ರತ್ಯಾಗಮ current_mii->name;

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸ್ಥಿರ ಸಂರಚನಾ mii_dev *miiphy_get_active_dev(ಧ್ರುವ ವರ್ಣ *devname)
{
	/* If the current mii is the one we want, return it */
	ಯದಿ (current_mii)
		ಯದಿ (strcmp(current_mii->name, devname) == 0)
			ಪ್ರತ್ಯಾಗಮ current_mii;

	/* Otherwise, set the active one to the one we want */
	ಯದಿ (miiphy_set_current_dev(devname))
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	ಅನ್ಯಥಾ
		ಪ್ರತ್ಯಾಗಮ current_mii;
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
ಪೂರ್ಣಾಙ್ಕ miiphy_read(ಧ್ರುವ ವರ್ಣ *devname, unsigned ವರ್ಣ addr, unsigned ವರ್ಣ reg,
		 unsigned short *ಮೌಲ್ಯ)
{
	ಸಂರಚನಾ mii_dev *bus;
	ಪೂರ್ಣಾಙ್ಕ ret;

	bus = miiphy_get_active_dev(devname);
	ಯದಿ (!bus)
		ಪ್ರತ್ಯಾಗಮ 1;

	ret = bus->ಪಠ(bus, addr, MDIO_DEVAD_NONE, reg);
	ಯದಿ (ret < 0)
		ಪ್ರತ್ಯಾಗಮ 1;

	*ಮೌಲ್ಯ = (unsigned short)ret;
	ಪ್ರತ್ಯಾಗಮ 0;
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
ಪೂರ್ಣಾಙ್ಕ miiphy_write(ಧ್ರುವ ವರ್ಣ *devname, unsigned ವರ್ಣ addr, unsigned ವರ್ಣ reg,
		  unsigned short ಮೌಲ್ಯ)
{
	ಸಂರಚನಾ mii_dev *bus;

	bus = miiphy_get_active_dev(devname);
	ಯದಿ (bus)
		ಪ್ರತ್ಯಾಗಮ bus->ಲಿಖ(bus, addr, MDIO_DEVAD_NONE, reg, ಮೌಲ್ಯ);

	ಪ್ರತ್ಯಾಗಮ 1;
}

/*****************************************************************************
 *
 * Print out list of registered MII capable devices.
 */
ಶೂನ್ಯ miiphy_listdev(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ list_head *entry;
	ಸಂರಚನಾ mii_dev *dev;

	puts("MII devices: ");
	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, ಸಂರಚನಾ mii_dev, link);
		printf("'%s' ", dev->name);
	}
	puts("\n");

	ಯದಿ (current_mii)
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
ಪೂರ್ಣಾಙ್ಕ miiphy_info(ಧ್ರುವ ವರ್ಣ *devname, unsigned ವರ್ಣ addr, unsigned ಪೂರ್ಣಾಙ್ಕ *oui,
		 unsigned ವರ್ಣ *model, unsigned ವರ್ಣ *rev)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ reg = 0;
	unsigned short tmp;

	ಯದಿ (miiphy_read(devname, addr, MII_PHYSID2, &tmp) != 0) {
		debug("PHY ID register 2 read failed\n");
		ಪ್ರತ್ಯಾಗಮ -1;
	}
	reg = tmp;

	debug("MII_PHYSID2 @ 0x%x = 0x%04x\n", addr, reg);

	ಯದಿ (reg == 0xFFFF) {
		/* No physical device present at this address */
		ಪ್ರತ್ಯಾಗಮ -1;
	}

	ಯದಿ (miiphy_read(devname, addr, MII_PHYSID1, &tmp) != 0) {
		debug("PHY ID register 1 read failed\n");
		ಪ್ರತ್ಯಾಗಮ -1;
	}
	reg |= tmp << 16;
	debug("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (unsigned ವರ್ಣ)((reg >> 4) & 0x0000003F);
	*rev = (unsigned ವರ್ಣ)(reg & 0x0000000F);
	ಪ್ರತ್ಯಾಗಮ 0;
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
ಪೂರ್ಣಾಙ್ಕ miiphy_reset(ಧ್ರುವ ವರ್ಣ *devname, unsigned ವರ್ಣ addr)
{
	unsigned short reg;
	ಪೂರ್ಣಾಙ್ಕ timeout = 500;

	ಯದಿ (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
		debug("PHY status read failed\n");
		ಪ್ರತ್ಯಾಗಮ -1;
	}
	ಯದಿ (miiphy_write(devname, addr, MII_BMCR, reg | BMCR_RESET) != 0) {
		debug("PHY reset failed\n");
		ಪ್ರತ್ಯಾಗಮ -1;
	}
#ಯದಿ CONFIG_PHY_RESET_DELAY > 0
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#endif
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = 0x8000;
	ಯಾವತ್ (((reg & 0x8000) != 0) && timeout--) {
		ಯದಿ (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			ಪ್ರತ್ಯಾಗಮ -1;
		}
		udelay(1000);
	}
	ಯದಿ ((reg & 0x8000) == 0) {
		ಪ್ರತ್ಯಾಗಮ 0;
	} ಅನ್ಯಥಾ {
		puts("PHY reset timed out\n");
		ಪ್ರತ್ಯಾಗಮ -1;
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif /* !PHYLIB */

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
ಪೂರ್ಣಾಙ್ಕ miiphy_speed(ಧ್ರುವ ವರ್ಣ *devname, unsigned ವರ್ಣ addr)
{
	u16 bmcr, anlpar, adv;

#ಯದಿ defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	ಯದಿ (miiphy_is_1000base_x(devname, addr))
		ಪ್ರತ್ಯಾಗಮ _1000BASET;

	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	ಯದಿ (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		ಗಚ್ಛ miiphy_read_failed;
	}
	ಯದಿ (btsr != 0xFFFF &&
			(btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)))
		ಪ್ರತ್ಯಾಗಮ _1000BASET;
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	ಯದಿ (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		printf("PHY speed");
		ಗಚ್ಛ miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	ಯದಿ (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		ಯದಿ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("PHY AN speed");
			ಗಚ್ಛ miiphy_read_failed;
		}

		ಯದಿ (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv speed");
			ಗಚ್ಛ miiphy_read_failed;
		}
		ಪ್ರತ್ಯಾಗಮ ((anlpar & adv) & LPA_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	ಪ್ರತ್ಯಾಗಮ (bmcr & BMCR_SPEED100) ? _100BASET : _10BASET;

miiphy_read_failed:
	printf(" read failed, assuming 10BASE-T\n");
	ಪ್ರತ್ಯಾಗಮ _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
ಪೂರ್ಣಾಙ್ಕ miiphy_duplex(ಧ್ರುವ ವರ್ಣ *devname, unsigned ವರ್ಣ addr)
{
	u16 bmcr, anlpar, adv;

#ಯದಿ defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	ಯದಿ (miiphy_is_1000base_x(devname, addr)) {
		/* 1000BASE-X */
		ಯದಿ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("1000BASE-X PHY AN duplex");
			ಗಚ್ಛ miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	ಯದಿ (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		ಗಚ್ಛ miiphy_read_failed;
	}
	ಯದಿ (btsr != 0xFFFF) {
		ಯದಿ (btsr & PHY_1000BTSR_1000FD) {
			ಪ್ರತ್ಯಾಗಮ FULL;
		} ಅನ್ಯಥಾ ಯದಿ (btsr & PHY_1000BTSR_1000HD) {
			ಪ್ರತ್ಯಾಗಮ HALF;
		}
	}
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	ಯದಿ (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		puts("PHY duplex");
		ಗಚ್ಛ miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	ಯದಿ (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		ಯದಿ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			puts("PHY AN duplex");
			ಗಚ್ಛ miiphy_read_failed;
		}

		ಯದಿ (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv duplex");
			ಗಚ್ಛ miiphy_read_failed;
		}
		ಪ್ರತ್ಯಾಗಮ ((anlpar & adv) & (LPA_10FULL | LPA_100FULL)) ?
		    FULL : HALF;
	}
	/* Get speed from basic control settings. */
	ಪ್ರತ್ಯಾಗಮ (bmcr & BMCR_FULLDPLX) ? FULL : HALF;

miiphy_read_failed:
	printf(" read failed, assuming half duplex\n");
	ಪ್ರತ್ಯಾಗಮ HALF;
}

/*****************************************************************************
 *
 * Return 1 if PHY supports 1000BASE-X, 0 if PHY supports 10BASE-T/100BASE-TX/
 * 1000BASE-T, or on error.
 */
ಪೂರ್ಣಾಙ್ಕ miiphy_is_1000base_x(ಧ್ರುವ ವರ್ಣ *devname, unsigned ವರ್ಣ addr)
{
#ಯದಿ defined(CONFIG_PHY_GIGE)
	u16 exsr;

	ಯದಿ (miiphy_read(devname, addr, MII_ESTATUS, &exsr)) {
		printf("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		ಪ್ರತ್ಯಾಗಮ 0;
	}
	ಪ್ರತ್ಯಾಗಮ 0 != (exsr & (ESTATUS_1000XF | ESTATUS_1000XH));
#ಅನ್ಯಥಾ
	ಪ್ರತ್ಯಾಗಮ 0;
#endif
}
