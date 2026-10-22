// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 */

/*
 * This provides a bit-banged interface to the ethernet MII management
 * channel.
 */

#अन्तर्भाव <dm.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <miiphy.h>
#अन्तर्भाव <phy.h>
#अन्तर्भाव <linux/delay.h>

#अन्तर्भाव <asm/types.h>
#अन्तर्भाव <linux/list.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <net.h>

/* local debug macro */
#अपरिभाषय MII_DEBUG

#अपरिभाषय debug
#यदि_परिभाषितम् MII_DEBUG
#परिभाषय debug(fmt, तर्काः...)	प्रारूप_लिख(fmt, ##args)
#अन्यथा
#परिभाषय debug(fmt, तर्काः...)
#यदि_अन्तः /* MII_DEBUG */

स्थिर LIST_शीर्ष(mii_devs);
स्थिर संरचना mii_dev *current_mii;

/*
 * Lookup the mii_dev struct by the registered device name.
 */
संरचना mii_dev *miiphy_get_dev_by_name(ध्रुव वर्ण *devname)
{
	संरचना list_शीर्ष *प्रविष्टिः;
	संरचना mii_dev *dev;

	यदि (!devname) {
		प्रारूप_लिख("NULL device name!\n");
		प्रत्यागम रिक्त;
	}

	list_for_each(प्रविष्टिः, &mii_devs) {
		dev = list_प्रविष्टिः(प्रविष्टिः, संरचना mii_dev, link);
		यदि (वाक्यं_तुलय(dev->name, devname) == 0)
			प्रत्यागम dev;
	}

	प्रत्यागम रिक्त;
}

संरचना mii_dev *mdio_alloc(शून्य)
{
	संरचना mii_dev *bus;

	bus = स्मृतिं_देहि(परिमाण(*bus));
	यदि (!bus)
		प्रत्यागम bus;

	स्मृतिं_पूरय(bus, 0, परिमाण(*bus));

	/* initialize mii_dev struct fields */
	INIT_LIST_शीर्ष(&bus->link);

	प्रत्यागम bus;
}

शून्य mdio_free(संरचना mii_dev *bus)
{
	मुक्त(bus);
}

पूर्णाङ्क mdio_पञ्जिकाकोशः(संरचना mii_dev *bus)
{
	यदि (!bus || !bus->पठ || !bus->लिख)
		प्रत्यागम -1;

	/* check if we have unique name */
	यदि (miiphy_get_dev_by_name(bus->name)) {
		प्रारूप_लिख("mdio_register: non unique device name '%s'\n",
			bus->name);
		प्रत्यागम -1;
	}

	/* add it to the list */
	list_सङ्कलय_पुच्छ(&bus->link, &mii_devs);

	यदि (!current_mii)
		current_mii = bus;

	प्रत्यागम 0;
}

पूर्णाङ्क mdio_पञ्जिकाकोशः_seq(संरचना mii_dev *bus, पूर्णाङ्क seq)
{
	पूर्णाङ्क ret;

	/* Setup a unique name for each mdio bus */
	ret = snprintf(bus->name, MDIO_NAME_LEN, "eth%d", seq);
	यदि (ret < 0)
		प्रत्यागम ret;

	प्रत्यागम mdio_पञ्जिकाकोशः(bus);
}

पूर्णाङ्क mdio_unregister(संरचना mii_dev *bus)
{
	यदि (!bus)
		प्रत्यागम 0;

	/* delete it from the list */
	list_del(&bus->link);

	यदि (current_mii == bus)
		current_mii = रिक्त;

	प्रत्यागम 0;
}

शून्य mdio_list_devices(शून्य)
{
	संरचना list_शीर्ष *प्रविष्टिः;

	list_for_each(प्रविष्टिः, &mii_devs) {
		पूर्णाङ्क i;
		संरचना mii_dev *bus = list_प्रविष्टिः(प्रविष्टिः, संरचना mii_dev, link);

		प्रारूप_लिख("%s:\n", bus->name);

		प्रति (i = 0; i < PHY_MAX_ADDR; i++) {
			संरचना phy_device *phydev = bus->phymap[i];

			यदि (phydev) {
				प्रारूप_लिख("%x - %s", i, phydev->drv->name);

				यदि (phydev->dev)
					प्रारूप_लिख(" <--> %s\n", phydev->dev->name);
				अन्यथा
					प्रारूप_लिख("\n");
			}
		}
	}
}

पूर्णाङ्क miiphy_समुच्चयः_current_dev(ध्रुव वर्ण *devname)
{
	संरचना mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname);
	यदि (dev) {
		current_mii = dev;
		प्रत्यागम 0;
	}

	प्रारूप_लिख("No such device: %s\n", devname);

	प्रत्यागम 1;
}

संरचना mii_dev *mdio_get_current_dev(शून्य)
{
	प्रत्यागम current_mii;
}

संरचना list_शीर्ष *mdio_get_list_शीर्ष(शून्य)
{
	प्रत्यागम &mii_devs;
}

संरचना phy_device *mdio_phydev_for_ethname(ध्रुव वर्ण *ethname)
{
	संरचना list_शीर्ष *प्रविष्टिः;
	संरचना mii_dev *bus;

	list_for_each(प्रविष्टिः, &mii_devs) {
		पूर्णाङ्क i;
		bus = list_प्रविष्टिः(प्रविष्टिः, संरचना mii_dev, link);

		प्रति (i = 0; i < PHY_MAX_ADDR; i++) {
			यदि (!bus->phymap[i] || !bus->phymap[i]->dev)
				अनुवर्त;

			यदि (वाक्यं_तुलय(bus->phymap[i]->dev->name, ethname) == 0)
				प्रत्यागम bus->phymap[i];
		}
	}

	प्रारूप_लिख("%s is not a known ethernet\n", ethname);
	प्रत्यागम रिक्त;
}

ध्रुव वर्ण *miiphy_get_current_dev(शून्य)
{
	यदि (current_mii)
		प्रत्यागम current_mii->name;

	प्रत्यागम रिक्त;
}

स्थिर संरचना mii_dev *miiphy_get_सक्रिय_dev(ध्रुव वर्ण *devname)
{
	/* If the current mii is the one we want, return it */
	यदि (current_mii)
		यदि (वाक्यं_तुलय(current_mii->name, devname) == 0)
			प्रत्यागम current_mii;

	/* Otherwise, set the active one to the one we want */
	यदि (miiphy_समुच्चयः_current_dev(devname))
		प्रत्यागम रिक्त;
	अन्यथा
		प्रत्यागम current_mii;
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
पूर्णाङ्क miiphy_read(ध्रुव वर्ण *devname, अचिह्नित वर्ण addr, अचिह्नित वर्ण reg,
		 अचिह्नित ह्रस्व *मानम्)
{
	संरचना mii_dev *bus;
	पूर्णाङ्क ret;

	bus = miiphy_get_सक्रिय_dev(devname);
	यदि (!bus)
		प्रत्यागम 1;

	ret = bus->पठ(bus, addr, MDIO_DEVAD_NONE, reg);
	यदि (ret < 0)
		प्रत्यागम 1;

	*मानम् = (अचिह्नित ह्रस्व)ret;
	प्रत्यागम 0;
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
पूर्णाङ्क miiphy_write(ध्रुव वर्ण *devname, अचिह्नित वर्ण addr, अचिह्नित वर्ण reg,
		  अचिह्नित ह्रस्व मानम्)
{
	संरचना mii_dev *bus;

	bus = miiphy_get_सक्रिय_dev(devname);
	यदि (bus)
		प्रत्यागम bus->लिख(bus, addr, MDIO_DEVAD_NONE, reg, मानम्);

	प्रत्यागम 1;
}

/*****************************************************************************
 *
 * Print out list of registered MII capable devices.
 */
शून्य miiphy_listdev(शून्य)
{
	संरचना list_शीर्ष *प्रविष्टिः;
	संरचना mii_dev *dev;

	पङ्क्तिं_लिख("MII devices: ");
	list_for_each(प्रविष्टिः, &mii_devs) {
		dev = list_प्रविष्टिः(प्रविष्टिः, संरचना mii_dev, link);
		प्रारूप_लिख("'%s' ", dev->name);
	}
	पङ्क्तिं_लिख("\n");

	यदि (current_mii)
		प्रारूप_लिख("Current device: '%s'\n", current_mii->name);
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
पूर्णाङ्क miiphy_info(ध्रुव वर्ण *devname, अचिह्नित वर्ण addr, अचिह्नित पूर्णाङ्क *oui,
		 अचिह्नित वर्ण *model, अचिह्नित वर्ण *rev)
{
	अचिह्नित पूर्णाङ्क reg = 0;
	अचिह्नित ह्रस्व tmp;

	यदि (miiphy_read(devname, addr, MII_PHYSID2, &tmp) != 0) {
		debug("PHY ID register 2 read failed\n");
		प्रत्यागम -1;
	}
	reg = tmp;

	debug("MII_PHYSID2 @ 0x%x = 0x%04x\n", addr, reg);

	यदि (reg == 0xFFFF) {
		/* No physical device present at this address */
		प्रत्यागम -1;
	}

	यदि (miiphy_read(devname, addr, MII_PHYSID1, &tmp) != 0) {
		debug("PHY ID register 1 read failed\n");
		प्रत्यागम -1;
	}
	reg |= tmp << 16;
	debug("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (अचिह्नित वर्ण)((reg >> 4) & 0x0000003F);
	*rev = (अचिह्नित वर्ण)(reg & 0x0000000F);
	प्रत्यागम 0;
}

#यदि_अपरिभाषितम् CONFIG_PHYLIB
/*****************************************************************************
 *
 * Reset the PHY.
 *
 * This API is deprecated. Use PHYLIB.
 *
 * Returns:
 *   0 on success
 */
पूर्णाङ्क miiphy_reset(ध्रुव वर्ण *devname, अचिह्नित वर्ण addr)
{
	अचिह्नित ह्रस्व reg;
	पूर्णाङ्क timeout = 500;

	यदि (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
		debug("PHY status read failed\n");
		प्रत्यागम -1;
	}
	यदि (miiphy_write(devname, addr, MII_BMCR, reg | BMCR_RESET) != 0) {
		debug("PHY reset failed\n");
		प्रत्यागम -1;
	}
#यदि CONFIG_PHY_RESET_DELAY > 0
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#यदि_अन्तः
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = 0x8000;
	यावत् (((reg & 0x8000) != 0) && timeout--) {
		यदि (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			प्रत्यागम -1;
		}
		udelay(1000);
	}
	यदि ((reg & 0x8000) == 0) {
		प्रत्यागम 0;
	} अन्यथा {
		पङ्क्तिं_लिख("PHY reset timed out\n");
		प्रत्यागम -1;
	}
	प्रत्यागम 0;
}
#यदि_अन्तः /* !PHYLIB */

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
पूर्णाङ्क miiphy_जवः(ध्रुव वर्ण *devname, अचिह्नित वर्ण addr)
{
	u16 bmcr, anlpar, adv;

#यदि defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	यदि (miiphy_is_1000base_x(devname, addr))
		प्रत्यागम _1000BASET;

	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	यदि (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		प्रारूप_लिख("PHY 1000BT status");
		गच्छ miiphy_read_failed;
	}
	यदि (btsr != 0xFFFF &&
			(btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)))
		प्रत्यागम _1000BASET;
#यदि_अन्तः /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	यदि (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		प्रारूप_लिख("PHY speed");
		गच्छ miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	यदि (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		यदि (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			प्रारूप_लिख("PHY AN speed");
			गच्छ miiphy_read_failed;
		}

		यदि (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			पङ्क्तिं_लिख("PHY AN adv speed");
			गच्छ miiphy_read_failed;
		}
		प्रत्यागम ((anlpar & adv) & LPA_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	प्रत्यागम (bmcr & BMCR_SPEED100) ? _100BASET : _10BASET;

miiphy_read_failed:
	प्रारूप_लिख(" read failed, assuming 10BASE-T\n");
	प्रत्यागम _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
पूर्णाङ्क miiphy_duplex(ध्रुव वर्ण *devname, अचिह्नित वर्ण addr)
{
	u16 bmcr, anlpar, adv;

#यदि defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	यदि (miiphy_is_1000base_x(devname, addr)) {
		/* 1000BASE-X */
		यदि (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			प्रारूप_लिख("1000BASE-X PHY AN duplex");
			गच्छ miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	यदि (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		प्रारूप_लिख("PHY 1000BT status");
		गच्छ miiphy_read_failed;
	}
	यदि (btsr != 0xFFFF) {
		यदि (btsr & PHY_1000BTSR_1000FD) {
			प्रत्यागम पूर्णम्;
		} अन्यथा यदि (btsr & PHY_1000BTSR_1000HD) {
			प्रत्यागम HALF;
		}
	}
#यदि_अन्तः /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	यदि (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		पङ्क्तिं_लिख("PHY duplex");
		गच्छ miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	यदि (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		यदि (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			पङ्क्तिं_लिख("PHY AN duplex");
			गच्छ miiphy_read_failed;
		}

		यदि (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			पङ्क्तिं_लिख("PHY AN adv duplex");
			गच्छ miiphy_read_failed;
		}
		प्रत्यागम ((anlpar & adv) & (LPA_10_पूर्णम् | LPA_100_पूर्णम्)) ?
		    पूर्णम् : HALF;
	}
	/* Get speed from basic control settings. */
	प्रत्यागम (bmcr & BMCR_FULLDPLX) ? पूर्णम् : HALF;

miiphy_read_failed:
	प्रारूप_लिख(" read failed, assuming half duplex\n");
	प्रत्यागम HALF;
}

/*****************************************************************************
 *
 * Return 1 if PHY supports 1000BASE-X, 0 if PHY supports 10BASE-T/100BASE-TX/
 * 1000BASE-T, or on error.
 */
पूर्णाङ्क miiphy_is_1000base_x(ध्रुव वर्ण *devname, अचिह्नित वर्ण addr)
{
#यदि defined(CONFIG_PHY_GIGE)
	u16 exsr;

	यदि (miiphy_read(devname, addr, MII_ESTATUS, &exsr)) {
		प्रारूप_लिख("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		प्रत्यागम 0;
	}
	प्रत्यागम 0 != (exsr & (ESTATUS_1000XF | ESTATUS_1000XH));
#अन्यथा
	प्रत्यागम 0;
#यदि_अन्तः
}
