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
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು MII_DEBUG

#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು debug
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ MII_DEBUG
#ವ್ಯಾಖ್ಯಾನಿಸು debug(fmt, ವಾದಗಳು...)	ಬರೆ(fmt, ##args)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು debug(fmt, ವಾದಗಳು...)
#ಆದರೆ_ಅಂತ್ಯ /* MII_DEBUG */

ಸ್ಥಿರ LIST_ತಲೆ(mii_devs);
ಸ್ಥಿರ ರಚನೆ mii_dev *current_mii;

/*
 * Lookup the mii_dev struct by the registered device name.
 */
ರಚನೆ mii_dev *miiphy_get_dev_by_name(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *devname)
{
	ರಚನೆ list_ತಲೆ *ನಮೂದು;
	ರಚನೆ mii_dev *dev;

	ಆದರೆ (!devname) {
		ಬರೆ("NULL device name!\n");
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	list_for_each(ನಮೂದು, &mii_devs) {
		dev = list_ನಮೂದು(ನಮೂದು, ರಚನೆ mii_dev, link);
		ಆದರೆ (strcmp(dev->name, devname) == 0)
			ಹಿಂತಿರುಗಿ dev;
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ರಚನೆ mii_dev *mdio_alloc(ಶೂನ್ಯ)
{
	ರಚನೆ mii_dev *bus;

	bus = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(*bus));
	ಆದರೆ (!bus)
		ಹಿಂತಿರುಗಿ bus;

	memset(bus, 0, ಗಾತ್ರ(*bus));

	/* initialize mii_dev struct fields */
	INIT_LIST_ತಲೆ(&bus->link);

	ಹಿಂತಿರುಗಿ bus;
}

ಶೂನ್ಯ mdio_free(ರಚನೆ mii_dev *bus)
{
	ಬಿಡುಗಡೆ(bus);
}

ಪೂರ್ಣಾಂಕ mdio_ರಿಜಿಸ್ಟರ್(ರಚನೆ mii_dev *bus)
{
	ಆದರೆ (!bus || !bus->ಪಠ || !bus->ಲಿಖ)
		ಹಿಂತಿರುಗಿ -1;

	/* check if we have unique name */
	ಆದರೆ (miiphy_get_dev_by_name(bus->name)) {
		ಬರೆ("mdio_register: non unique device name '%s'\n",
			bus->name);
		ಹಿಂತಿರುಗಿ -1;
	}

	/* add it to the list */
	list_ಕೂಡಿಸು_ಬಾಲ(&bus->link, &mii_devs);

	ಆದರೆ (!current_mii)
		current_mii = bus;

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ mdio_ರಿಜಿಸ್ಟರ್_seq(ರಚನೆ mii_dev *bus, ಪೂರ್ಣಾಂಕ seq)
{
	ಪೂರ್ಣಾಂಕ ret;

	/* Setup a unique name for each mdio bus */
	ret = snprintf(bus->name, MDIO_NAME_LEN, "eth%d", seq);
	ಆದರೆ (ret < 0)
		ಹಿಂತಿರುಗಿ ret;

	ಹಿಂತಿರುಗಿ mdio_ರಿಜಿಸ್ಟರ್(bus);
}

ಪೂರ್ಣಾಂಕ mdio_unregister(ರಚನೆ mii_dev *bus)
{
	ಆದರೆ (!bus)
		ಹಿಂತಿರುಗಿ 0;

	/* delete it from the list */
	list_del(&bus->link);

	ಆದರೆ (current_mii == bus)
		current_mii = ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ mdio_list_devices(ಶೂನ್ಯ)
{
	ರಚನೆ list_ತಲೆ *ನಮೂದು;

	list_for_each(ನಮೂದು, &mii_devs) {
		ಪೂರ್ಣಾಂಕ i;
		ರಚನೆ mii_dev *bus = list_ನಮೂದು(ನಮೂದು, ರಚನೆ mii_dev, link);

		ಬರೆ("%s:\n", bus->name);

		ಗಾಗಿ (i = 0; i < PHY_MAX_ADDR; i++) {
			ರಚನೆ phy_device *phydev = bus->phymap[i];

			ಆದರೆ (phydev) {
				ಬರೆ("%x - %s", i, phydev->drv->name);

				ಆದರೆ (phydev->dev)
					ಬರೆ(" <--> %s\n", phydev->dev->name);
				ಇಲ್ಲದಿದ್ದರೆ
					ಬರೆ("\n");
			}
		}
	}
}

ಪೂರ್ಣಾಂಕ miiphy_ಗಣ_current_dev(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *devname)
{
	ರಚನೆ mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname);
	ಆದರೆ (dev) {
		current_mii = dev;
		ಹಿಂತಿರುಗಿ 0;
	}

	ಬರೆ("No such device: %s\n", devname);

	ಹಿಂತಿರುಗಿ 1;
}

ರಚನೆ mii_dev *mdio_get_current_dev(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ current_mii;
}

ರಚನೆ list_ತಲೆ *mdio_get_list_ತಲೆ(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ &mii_devs;
}

ರಚನೆ phy_device *mdio_phydev_for_ethname(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ethname)
{
	ರಚನೆ list_ತಲೆ *ನಮೂದು;
	ರಚನೆ mii_dev *bus;

	list_for_each(ನಮೂದು, &mii_devs) {
		ಪೂರ್ಣಾಂಕ i;
		bus = list_ನಮೂದು(ನಮೂದು, ರಚನೆ mii_dev, link);

		ಗಾಗಿ (i = 0; i < PHY_MAX_ADDR; i++) {
			ಆದರೆ (!bus->phymap[i] || !bus->phymap[i]->dev)
				ಮುಂದುವರಿ;

			ಆದರೆ (strcmp(bus->phymap[i]->dev->name, ethname) == 0)
				ಹಿಂತಿರುಗಿ bus->phymap[i];
		}
	}

	ಬರೆ("%s is not a known ethernet\n", ethname);
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *miiphy_get_current_dev(ಶೂನ್ಯ)
{
	ಆದರೆ (current_mii)
		ಹಿಂತಿರುಗಿ current_mii->name;

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಸ್ಥಿರ ರಚನೆ mii_dev *miiphy_get_ಸಕ್ರಿಯ_dev(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *devname)
{
	/* If the current mii is the one we want, return it */
	ಆದರೆ (current_mii)
		ಆದರೆ (strcmp(current_mii->name, devname) == 0)
			ಹಿಂತಿರುಗಿ current_mii;

	/* Otherwise, set the active one to the one we want */
	ಆದರೆ (miiphy_ಗಣ_current_dev(devname))
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	ಇಲ್ಲದಿದ್ದರೆ
		ಹಿಂತಿರುಗಿ current_mii;
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
ಪೂರ್ಣಾಂಕ miiphy_read(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *devname, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ addr, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ reg,
		 ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ *ಮೌಲ್ಯ)
{
	ರಚನೆ mii_dev *bus;
	ಪೂರ್ಣಾಂಕ ret;

	bus = miiphy_get_ಸಕ್ರಿಯ_dev(devname);
	ಆದರೆ (!bus)
		ಹಿಂತಿರುಗಿ 1;

	ret = bus->ಪಠ(bus, addr, MDIO_DEVAD_NONE, reg);
	ಆದರೆ (ret < 0)
		ಹಿಂತಿರುಗಿ 1;

	*ಮೌಲ್ಯ = (ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ)ret;
	ಹಿಂತಿರುಗಿ 0;
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
ಪೂರ್ಣಾಂಕ miiphy_write(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *devname, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ addr, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ reg,
		  ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ ಮೌಲ್ಯ)
{
	ರಚನೆ mii_dev *bus;

	bus = miiphy_get_ಸಕ್ರಿಯ_dev(devname);
	ಆದರೆ (bus)
		ಹಿಂತಿರುಗಿ bus->ಲಿಖ(bus, addr, MDIO_DEVAD_NONE, reg, ಮೌಲ್ಯ);

	ಹಿಂತಿರುಗಿ 1;
}

/*****************************************************************************
 *
 * Print out list of registered MII capable devices.
 */
ಶೂನ್ಯ miiphy_listdev(ಶೂನ್ಯ)
{
	ರಚನೆ list_ತಲೆ *ನಮೂದು;
	ರಚನೆ mii_dev *dev;

	puts("MII devices: ");
	list_for_each(ನಮೂದು, &mii_devs) {
		dev = list_ನಮೂದು(ನಮೂದು, ರಚನೆ mii_dev, link);
		ಬರೆ("'%s' ", dev->name);
	}
	puts("\n");

	ಆದರೆ (current_mii)
		ಬರೆ("Current device: '%s'\n", current_mii->name);
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
ಪೂರ್ಣಾಂಕ miiphy_info(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *devname, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ addr, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *oui,
		 ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *model, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *rev)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ reg = 0;
	ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ tmp;

	ಆದರೆ (miiphy_read(devname, addr, MII_PHYSID2, &tmp) != 0) {
		debug("PHY ID register 2 read failed\n");
		ಹಿಂತಿರುಗಿ -1;
	}
	reg = tmp;

	debug("MII_PHYSID2 @ 0x%x = 0x%04x\n", addr, reg);

	ಆದರೆ (reg == 0xFFFF) {
		/* No physical device present at this address */
		ಹಿಂತಿರುಗಿ -1;
	}

	ಆದರೆ (miiphy_read(devname, addr, MII_PHYSID1, &tmp) != 0) {
		debug("PHY ID register 1 read failed\n");
		ಹಿಂತಿರುಗಿ -1;
	}
	reg |= tmp << 16;
	debug("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)((reg >> 4) & 0x0000003F);
	*rev = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(reg & 0x0000000F);
	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_PHYLIB
/*****************************************************************************
 *
 * Reset the PHY.
 *
 * This API is deprecated. Use PHYLIB.
 *
 * Returns:
 *   0 on success
 */
ಪೂರ್ಣಾಂಕ miiphy_reset(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *devname, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ addr)
{
	ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ reg;
	ಪೂರ್ಣಾಂಕ timeout = 500;

	ಆದರೆ (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
		debug("PHY status read failed\n");
		ಹಿಂತಿರುಗಿ -1;
	}
	ಆದರೆ (miiphy_write(devname, addr, MII_BMCR, reg | BMCR_RESET) != 0) {
		debug("PHY reset failed\n");
		ಹಿಂತಿರುಗಿ -1;
	}
#ಆದರೆ CONFIG_PHY_RESET_DELAY > 0
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#ಆದರೆ_ಅಂತ್ಯ
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = 0x8000;
	ಆಗ (((reg & 0x8000) != 0) && timeout--) {
		ಆದರೆ (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			ಹಿಂತಿರುಗಿ -1;
		}
		udelay(1000);
	}
	ಆದರೆ ((reg & 0x8000) == 0) {
		ಹಿಂತಿರುಗಿ 0;
	} ಇಲ್ಲದಿದ್ದರೆ {
		puts("PHY reset timed out\n");
		ಹಿಂತಿರುಗಿ -1;
	}
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ /* !PHYLIB */

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
ಪೂರ್ಣಾಂಕ miiphy_ವೇಗ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *devname, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ addr)
{
	u16 bmcr, anlpar, adv;

#ಆದರೆ defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	ಆದರೆ (miiphy_is_1000base_x(devname, addr))
		ಹಿಂತಿರುಗಿ _1000BASET;

	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	ಆದರೆ (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		ಬರೆ("PHY 1000BT status");
		ಹೋಗು miiphy_read_failed;
	}
	ಆದರೆ (btsr != 0xFFFF &&
			(btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)))
		ಹಿಂತಿರುಗಿ _1000BASET;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	ಆದರೆ (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		ಬರೆ("PHY speed");
		ಹೋಗು miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	ಆದರೆ (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		ಆದರೆ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			ಬರೆ("PHY AN speed");
			ಹೋಗು miiphy_read_failed;
		}

		ಆದರೆ (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv speed");
			ಹೋಗು miiphy_read_failed;
		}
		ಹಿಂತಿರುಗಿ ((anlpar & adv) & LPA_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	ಹಿಂತಿರುಗಿ (bmcr & BMCR_SPEED100) ? _100BASET : _10BASET;

miiphy_read_failed:
	ಬರೆ(" read failed, assuming 10BASE-T\n");
	ಹಿಂತಿರುಗಿ _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
ಪೂರ್ಣಾಂಕ miiphy_duplex(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *devname, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ addr)
{
	u16 bmcr, anlpar, adv;

#ಆದರೆ defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	ಆದರೆ (miiphy_is_1000base_x(devname, addr)) {
		/* 1000BASE-X */
		ಆದರೆ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			ಬರೆ("1000BASE-X PHY AN duplex");
			ಹೋಗು miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	ಆದರೆ (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		ಬರೆ("PHY 1000BT status");
		ಹೋಗು miiphy_read_failed;
	}
	ಆದರೆ (btsr != 0xFFFF) {
		ಆದರೆ (btsr & PHY_1000BTSR_1000FD) {
			ಹಿಂತಿರುಗಿ ತುಂಬಿದ;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (btsr & PHY_1000BTSR_1000HD) {
			ಹಿಂತಿರುಗಿ HALF;
		}
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	ಆದರೆ (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		puts("PHY duplex");
		ಹೋಗು miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	ಆದರೆ (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		ಆದರೆ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			puts("PHY AN duplex");
			ಹೋಗು miiphy_read_failed;
		}

		ಆದರೆ (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv duplex");
			ಹೋಗು miiphy_read_failed;
		}
		ಹಿಂತಿರುಗಿ ((anlpar & adv) & (LPA_10_ತುಂಬಿದ | LPA_100_ತುಂಬಿದ)) ?
		    ತುಂಬಿದ : HALF;
	}
	/* Get speed from basic control settings. */
	ಹಿಂತಿರುಗಿ (bmcr & BMCR_FULLDPLX) ? ತುಂಬಿದ : HALF;

miiphy_read_failed:
	ಬರೆ(" read failed, assuming half duplex\n");
	ಹಿಂತಿರುಗಿ HALF;
}

/*****************************************************************************
 *
 * Return 1 if PHY supports 1000BASE-X, 0 if PHY supports 10BASE-T/100BASE-TX/
 * 1000BASE-T, or on error.
 */
ಪೂರ್ಣಾಂಕ miiphy_is_1000base_x(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *devname, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ addr)
{
#ಆದರೆ defined(CONFIG_PHY_GIGE)
	u16 exsr;

	ಆದರೆ (miiphy_read(devname, addr, MII_ESTATUS, &exsr)) {
		ಬರೆ("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		ಹಿಂತಿರುಗಿ 0;
	}
	ಹಿಂತಿರುಗಿ 0 != (exsr & (ESTATUS_1000XF | ESTATUS_1000XH));
#ಇಲ್ಲದಿದ್ದರೆ
	ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
}
