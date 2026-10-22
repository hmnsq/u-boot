// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 */

/*
 * This provides a bit-banged interface to the ethernet MII management
 * channel.
 */

#అన్తర్భావ <dm.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <miiphy.h>
#అన్తర్భావ <phy.h>
#అన్తర్భావ <linux/delay.h>

#అన్తర్భావ <asm/types.h>
#అన్తర్భావ <linux/list.h>
#అన్తర్భావ <malloc.h>
#అన్తర్భావ <net.h>

/* local debug macro */
#నిర్వచనం_రద్దు MII_DEBUG

#నిర్వచనం_రద్దు debug
#నిర్వచితమైతే MII_DEBUG
#నిర్వచించు debug(fmt, వాదనలు...)	రాయి(fmt, ##args)
#లేకపోతే
#నిర్వచించు debug(fmt, వాదనలు...)
#అయితే_ముగింపు /* MII_DEBUG */

స్థిర LIST_తల(mii_devs);
స్థిర నిర్మాణం mii_dev *current_mii;

/*
 * Lookup the mii_dev struct by the registered device name.
 */
నిర్మాణం mii_dev *miiphy_get_dev_by_name(స్థిరరాశి అక్షరం *devname)
{
	నిర్మాణం list_తల *నమోదు_2;
	నిర్మాణం mii_dev *dev;

	ఒకవేళ (!devname) {
		రాయి("NULL device name!\n");
		తిరిగి ఖాళీ;
	}

	list_for_each(నమోదు_2, &mii_devs) {
		dev = list_నమోదు(నమోదు_2, నిర్మాణం mii_dev, link);
		ఒకవేళ (strcmp(dev->name, devname) == 0)
			తిరిగి dev;
	}

	తిరిగి ఖాళీ;
}

నిర్మాణం mii_dev *mdio_alloc(శూన్య)
{
	నిర్మాణం mii_dev *bus;

	bus = జ్ఞాపకం_ఇవ్వు(పరిమాణం(*bus));
	ఒకవేళ (!bus)
		తిరిగి bus;

	memset(bus, 0, పరిమాణం(*bus));

	/* initialize mii_dev struct fields */
	INIT_LIST_తల(&bus->link);

	తిరిగి bus;
}

శూన్య mdio_free(నిర్మాణం mii_dev *bus)
{
	విడుదల(bus);
}

పూర్ణాంకం mdio_రిజిస్టరు(నిర్మాణం mii_dev *bus)
{
	ఒకవేళ (!bus || !bus->పఠ || !bus->లిఖ)
		తిరిగి -1;

	/* check if we have unique name */
	ఒకవేళ (miiphy_get_dev_by_name(bus->name)) {
		రాయి("mdio_register: non unique device name '%s'\n",
			bus->name);
		తిరిగి -1;
	}

	/* add it to the list */
	list_కూడు_తోక(&bus->link, &mii_devs);

	ఒకవేళ (!current_mii)
		current_mii = bus;

	తిరిగి 0;
}

పూర్ణాంకం mdio_రిజిస్టరు_seq(నిర్మాణం mii_dev *bus, పూర్ణాంకం seq)
{
	పూర్ణాంకం ret;

	/* Setup a unique name for each mdio bus */
	ret = snprintf(bus->name, MDIO_NAME_LEN, "eth%d", seq);
	ఒకవేళ (ret < 0)
		తిరిగి ret;

	తిరిగి mdio_రిజిస్టరు(bus);
}

పూర్ణాంకం mdio_unregister(నిర్మాణం mii_dev *bus)
{
	ఒకవేళ (!bus)
		తిరిగి 0;

	/* delete it from the list */
	list_del(&bus->link);

	ఒకవేళ (current_mii == bus)
		current_mii = ఖాళీ;

	తిరిగి 0;
}

శూన్య mdio_list_devices(శూన్య)
{
	నిర్మాణం list_తల *నమోదు_2;

	list_for_each(నమోదు_2, &mii_devs) {
		పూర్ణాంకం i;
		నిర్మాణం mii_dev *bus = list_నమోదు(నమోదు_2, నిర్మాణం mii_dev, link);

		రాయి("%s:\n", bus->name);

		కొరకు (i = 0; i < PHY_MAX_ADDR; i++) {
			నిర్మాణం phy_device *phydev = bus->phymap[i];

			ఒకవేళ (phydev) {
				రాయి("%x - %s", i, phydev->drv->name);

				ఒకవేళ (phydev->dev)
					రాయి(" <--> %s\n", phydev->dev->name);
				లేకపోతే
					రాయి("\n");
			}
		}
	}
}

పూర్ణాంకం miiphy_సమితి_current_dev(స్థిరరాశి అక్షరం *devname)
{
	నిర్మాణం mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname);
	ఒకవేళ (dev) {
		current_mii = dev;
		తిరిగి 0;
	}

	రాయి("No such device: %s\n", devname);

	తిరిగి 1;
}

నిర్మాణం mii_dev *mdio_get_current_dev(శూన్య)
{
	తిరిగి current_mii;
}

నిర్మాణం list_తల *mdio_get_list_తల(శూన్య)
{
	తిరిగి &mii_devs;
}

నిర్మాణం phy_device *mdio_phydev_for_ethname(స్థిరరాశి అక్షరం *ethname)
{
	నిర్మాణం list_తల *నమోదు_2;
	నిర్మాణం mii_dev *bus;

	list_for_each(నమోదు_2, &mii_devs) {
		పూర్ణాంకం i;
		bus = list_నమోదు(నమోదు_2, నిర్మాణం mii_dev, link);

		కొరకు (i = 0; i < PHY_MAX_ADDR; i++) {
			ఒకవేళ (!bus->phymap[i] || !bus->phymap[i]->dev)
				కొనసాగు;

			ఒకవేళ (strcmp(bus->phymap[i]->dev->name, ethname) == 0)
				తిరిగి bus->phymap[i];
		}
	}

	రాయి("%s is not a known ethernet\n", ethname);
	తిరిగి ఖాళీ;
}

స్థిరరాశి అక్షరం *miiphy_get_current_dev(శూన్య)
{
	ఒకవేళ (current_mii)
		తిరిగి current_mii->name;

	తిరిగి ఖాళీ;
}

స్థిర నిర్మాణం mii_dev *miiphy_get_చురుకైన_dev(స్థిరరాశి అక్షరం *devname)
{
	/* If the current mii is the one we want, return it */
	ఒకవేళ (current_mii)
		ఒకవేళ (strcmp(current_mii->name, devname) == 0)
			తిరిగి current_mii;

	/* Otherwise, set the active one to the one we want */
	ఒకవేళ (miiphy_సమితి_current_dev(devname))
		తిరిగి ఖాళీ;
	లేకపోతే
		తిరిగి current_mii;
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
పూర్ణాంకం miiphy_read(స్థిరరాశి అక్షరం *devname, గుర్తులేని అక్షరం addr, గుర్తులేని అక్షరం reg,
		 గుర్తులేని పొట్టి *విలువ)
{
	నిర్మాణం mii_dev *bus;
	పూర్ణాంకం ret;

	bus = miiphy_get_చురుకైన_dev(devname);
	ఒకవేళ (!bus)
		తిరిగి 1;

	ret = bus->పఠ(bus, addr, MDIO_DEVAD_NONE, reg);
	ఒకవేళ (ret < 0)
		తిరిగి 1;

	*విలువ = (గుర్తులేని పొట్టి)ret;
	తిరిగి 0;
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
పూర్ణాంకం miiphy_write(స్థిరరాశి అక్షరం *devname, గుర్తులేని అక్షరం addr, గుర్తులేని అక్షరం reg,
		  గుర్తులేని పొట్టి విలువ)
{
	నిర్మాణం mii_dev *bus;

	bus = miiphy_get_చురుకైన_dev(devname);
	ఒకవేళ (bus)
		తిరిగి bus->లిఖ(bus, addr, MDIO_DEVAD_NONE, reg, విలువ);

	తిరిగి 1;
}

/*****************************************************************************
 *
 * Print out list of registered MII capable devices.
 */
శూన్య miiphy_listdev(శూన్య)
{
	నిర్మాణం list_తల *నమోదు_2;
	నిర్మాణం mii_dev *dev;

	puts("MII devices: ");
	list_for_each(నమోదు_2, &mii_devs) {
		dev = list_నమోదు(నమోదు_2, నిర్మాణం mii_dev, link);
		రాయి("'%s' ", dev->name);
	}
	puts("\n");

	ఒకవేళ (current_mii)
		రాయి("Current device: '%s'\n", current_mii->name);
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
పూర్ణాంకం miiphy_info(స్థిరరాశి అక్షరం *devname, గుర్తులేని అక్షరం addr, గుర్తులేని పూర్ణాంకం *oui,
		 గుర్తులేని అక్షరం *model, గుర్తులేని అక్షరం *rev)
{
	గుర్తులేని పూర్ణాంకం reg = 0;
	గుర్తులేని పొట్టి tmp;

	ఒకవేళ (miiphy_read(devname, addr, MII_PHYSID2, &tmp) != 0) {
		debug("PHY ID register 2 read failed\n");
		తిరిగి -1;
	}
	reg = tmp;

	debug("MII_PHYSID2 @ 0x%x = 0x%04x\n", addr, reg);

	ఒకవేళ (reg == 0xFFFF) {
		/* No physical device present at this address */
		తిరిగి -1;
	}

	ఒకవేళ (miiphy_read(devname, addr, MII_PHYSID1, &tmp) != 0) {
		debug("PHY ID register 1 read failed\n");
		తిరిగి -1;
	}
	reg |= tmp << 16;
	debug("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (గుర్తులేని అక్షరం)((reg >> 4) & 0x0000003F);
	*rev = (గుర్తులేని అక్షరం)(reg & 0x0000000F);
	తిరిగి 0;
}

#నిర్వచితం_కాకపోతే CONFIG_PHYLIB
/*****************************************************************************
 *
 * Reset the PHY.
 *
 * This API is deprecated. Use PHYLIB.
 *
 * Returns:
 *   0 on success
 */
పూర్ణాంకం miiphy_reset(స్థిరరాశి అక్షరం *devname, గుర్తులేని అక్షరం addr)
{
	గుర్తులేని పొట్టి reg;
	పూర్ణాంకం timeout = 500;

	ఒకవేళ (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
		debug("PHY status read failed\n");
		తిరిగి -1;
	}
	ఒకవేళ (miiphy_write(devname, addr, MII_BMCR, reg | BMCR_RESET) != 0) {
		debug("PHY reset failed\n");
		తిరిగి -1;
	}
#ఒకవేళ CONFIG_PHY_RESET_DELAY > 0
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#అయితే_ముగింపు
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = 0x8000;
	అయితే (((reg & 0x8000) != 0) && timeout--) {
		ఒకవేళ (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			తిరిగి -1;
		}
		udelay(1000);
	}
	ఒకవేళ ((reg & 0x8000) == 0) {
		తిరిగి 0;
	} లేకపోతే {
		puts("PHY reset timed out\n");
		తిరిగి -1;
	}
	తిరిగి 0;
}
#అయితే_ముగింపు /* !PHYLIB */

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
పూర్ణాంకం miiphy_వేగం(స్థిరరాశి అక్షరం *devname, గుర్తులేని అక్షరం addr)
{
	u16 bmcr, anlpar, adv;

#ఒకవేళ defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	ఒకవేళ (miiphy_is_1000base_x(devname, addr))
		తిరిగి _1000BASET;

	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	ఒకవేళ (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		రాయి("PHY 1000BT status");
		వెళ్ళు miiphy_read_failed;
	}
	ఒకవేళ (btsr != 0xFFFF &&
			(btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)))
		తిరిగి _1000BASET;
#అయితే_ముగింపు /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	ఒకవేళ (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		రాయి("PHY speed");
		వెళ్ళు miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	ఒకవేళ (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		ఒకవేళ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			రాయి("PHY AN speed");
			వెళ్ళు miiphy_read_failed;
		}

		ఒకవేళ (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv speed");
			వెళ్ళు miiphy_read_failed;
		}
		తిరిగి ((anlpar & adv) & LPA_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	తిరిగి (bmcr & BMCR_SPEED100) ? _100BASET : _10BASET;

miiphy_read_failed:
	రాయి(" read failed, assuming 10BASE-T\n");
	తిరిగి _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
పూర్ణాంకం miiphy_duplex(స్థిరరాశి అక్షరం *devname, గుర్తులేని అక్షరం addr)
{
	u16 bmcr, anlpar, adv;

#ఒకవేళ defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	ఒకవేళ (miiphy_is_1000base_x(devname, addr)) {
		/* 1000BASE-X */
		ఒకవేళ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			రాయి("1000BASE-X PHY AN duplex");
			వెళ్ళు miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	ఒకవేళ (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		రాయి("PHY 1000BT status");
		వెళ్ళు miiphy_read_failed;
	}
	ఒకవేళ (btsr != 0xFFFF) {
		ఒకవేళ (btsr & PHY_1000BTSR_1000FD) {
			తిరిగి నిండిన;
		} లేకపోతే ఒకవేళ (btsr & PHY_1000BTSR_1000HD) {
			తిరిగి HALF;
		}
	}
#అయితే_ముగింపు /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	ఒకవేళ (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		puts("PHY duplex");
		వెళ్ళు miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	ఒకవేళ (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		ఒకవేళ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			puts("PHY AN duplex");
			వెళ్ళు miiphy_read_failed;
		}

		ఒకవేళ (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv duplex");
			వెళ్ళు miiphy_read_failed;
		}
		తిరిగి ((anlpar & adv) & (LPA_10_నిండిన | LPA_100_నిండిన)) ?
		    నిండిన : HALF;
	}
	/* Get speed from basic control settings. */
	తిరిగి (bmcr & BMCR_FULLDPLX) ? నిండిన : HALF;

miiphy_read_failed:
	రాయి(" read failed, assuming half duplex\n");
	తిరిగి HALF;
}

/*****************************************************************************
 *
 * Return 1 if PHY supports 1000BASE-X, 0 if PHY supports 10BASE-T/100BASE-TX/
 * 1000BASE-T, or on error.
 */
పూర్ణాంకం miiphy_is_1000base_x(స్థిరరాశి అక్షరం *devname, గుర్తులేని అక్షరం addr)
{
#ఒకవేళ defined(CONFIG_PHY_GIGE)
	u16 exsr;

	ఒకవేళ (miiphy_read(devname, addr, MII_ESTATUS, &exsr)) {
		రాయి("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		తిరిగి 0;
	}
	తిరిగి 0 != (exsr & (ESTATUS_1000XF | ESTATUS_1000XH));
#లేకపోతే
	తిరిగి 0;
#అయితే_ముగింపు
}
