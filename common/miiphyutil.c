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
#undef MII_DEBUG

#undef debug
#ifdef MII_DEBUG
#define debug(fmt, args...)	म_लिखो(fmt, ##args)
#अन्यथा
#define debug(fmt, args...)
#endif /* MII_DEBUG */

स्थिर LIST_HEAD(mii_devs);
स्थिर संरचना mii_dev *current_mii;

/*
 * Lookup the mii_dev struct by the registered device name.
 */
संरचना mii_dev *miiphy_get_dev_by_name(अचर वर्ण *devname)
{
	संरचना list_head *entry;
	संरचना mii_dev *dev;

	यदि (!devname) {
		म_लिखो("NULL device name!\n");
		फर्काऊ रिक्त;
	}

	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, संरचना mii_dev, link);
		यदि (म_भेद(dev->name, devname) == 0)
			फर्काऊ dev;
	}

	फर्काऊ रिक्त;
}

संरचना mii_dev *mdio_alloc(शून्य)
{
	संरचना mii_dev *bus;

	bus = दो_स्मृति(परिमाण(*bus));
	यदि (!bus)
		फर्काऊ bus;

	स_रखो(bus, 0, परिमाण(*bus));

	/* initialize mii_dev struct fields */
	INIT_LIST_HEAD(&bus->link);

	फर्काऊ bus;
}

शून्य mdio_free(संरचना mii_dev *bus)
{
	मुक्त(bus);
}

पूर्णाङ्क mdio_register(संरचना mii_dev *bus)
{
	यदि (!bus || !bus->पठ || !bus->लिख)
		फर्काऊ -1;

	/* check if we have unique name */
	यदि (miiphy_get_dev_by_name(bus->name)) {
		म_लिखो("mdio_register: non unique device name '%s'\n",
			bus->name);
		फर्काऊ -1;
	}

	/* add it to the list */
	list_add_tail(&bus->link, &mii_devs);

	यदि (!current_mii)
		current_mii = bus;

	फर्काऊ 0;
}

पूर्णाङ्क mdio_register_seq(संरचना mii_dev *bus, पूर्णाङ्क seq)
{
	पूर्णाङ्क ret;

	/* Setup a unique name for each mdio bus */
	ret = snprintf(bus->name, MDIO_NAME_LEN, "eth%d", seq);
	यदि (ret < 0)
		फर्काऊ ret;

	फर्काऊ mdio_register(bus);
}

पूर्णाङ्क mdio_unregister(संरचना mii_dev *bus)
{
	यदि (!bus)
		फर्काऊ 0;

	/* delete it from the list */
	list_del(&bus->link);

	यदि (current_mii == bus)
		current_mii = रिक्त;

	फर्काऊ 0;
}

शून्य mdio_list_devices(शून्य)
{
	संरचना list_head *entry;

	list_for_each(entry, &mii_devs) {
		पूर्णाङ्क i;
		संरचना mii_dev *bus = list_entry(entry, संरचना mii_dev, link);

		म_लिखो("%s:\n", bus->name);

		लागि (i = 0; i < PHY_MAX_ADDR; i++) {
			संरचना phy_device *phydev = bus->phymap[i];

			यदि (phydev) {
				म_लिखो("%x - %s", i, phydev->drv->name);

				यदि (phydev->dev)
					म_लिखो(" <--> %s\n", phydev->dev->name);
				अन्यथा
					म_लिखो("\n");
			}
		}
	}
}

पूर्णाङ्क miiphy_set_current_dev(अचर वर्ण *devname)
{
	संरचना mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname);
	यदि (dev) {
		current_mii = dev;
		फर्काऊ 0;
	}

	म_लिखो("No such device: %s\n", devname);

	फर्काऊ 1;
}

संरचना mii_dev *mdio_get_current_dev(शून्य)
{
	फर्काऊ current_mii;
}

संरचना list_head *mdio_get_list_head(शून्य)
{
	फर्काऊ &mii_devs;
}

संरचना phy_device *mdio_phydev_for_ethname(अचर वर्ण *ethname)
{
	संरचना list_head *entry;
	संरचना mii_dev *bus;

	list_for_each(entry, &mii_devs) {
		पूर्णाङ्क i;
		bus = list_entry(entry, संरचना mii_dev, link);

		लागि (i = 0; i < PHY_MAX_ADDR; i++) {
			यदि (!bus->phymap[i] || !bus->phymap[i]->dev)
				जारी;

			यदि (म_भेद(bus->phymap[i]->dev->name, ethname) == 0)
				फर्काऊ bus->phymap[i];
		}
	}

	म_लिखो("%s is not a known ethernet\n", ethname);
	फर्काऊ रिक्त;
}

अचर वर्ण *miiphy_get_current_dev(शून्य)
{
	यदि (current_mii)
		फर्काऊ current_mii->name;

	फर्काऊ रिक्त;
}

स्थिर संरचना mii_dev *miiphy_get_active_dev(अचर वर्ण *devname)
{
	/* If the current mii is the one we want, return it */
	यदि (current_mii)
		यदि (म_भेद(current_mii->name, devname) == 0)
			फर्काऊ current_mii;

	/* Otherwise, set the active one to the one we want */
	यदि (miiphy_set_current_dev(devname))
		फर्काऊ रिक्त;
	अन्यथा
		फर्काऊ current_mii;
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
पूर्णाङ्क miiphy_read(अचर वर्ण *devname, अचिन्हित वर्ण addr, अचिन्हित वर्ण reg,
		 अचिन्हित लघु *मान)
{
	संरचना mii_dev *bus;
	पूर्णाङ्क ret;

	bus = miiphy_get_active_dev(devname);
	यदि (!bus)
		फर्काऊ 1;

	ret = bus->पठ(bus, addr, MDIO_DEVAD_NONE, reg);
	यदि (ret < 0)
		फर्काऊ 1;

	*मान = (अचिन्हित लघु)ret;
	फर्काऊ 0;
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
पूर्णाङ्क miiphy_write(अचर वर्ण *devname, अचिन्हित वर्ण addr, अचिन्हित वर्ण reg,
		  अचिन्हित लघु मान)
{
	संरचना mii_dev *bus;

	bus = miiphy_get_active_dev(devname);
	यदि (bus)
		फर्काऊ bus->लिख(bus, addr, MDIO_DEVAD_NONE, reg, मान);

	फर्काऊ 1;
}

/*****************************************************************************
 *
 * Print out list of registered MII capable devices.
 */
शून्य miiphy_listdev(शून्य)
{
	संरचना list_head *entry;
	संरचना mii_dev *dev;

	माला_दो("MII devices: ");
	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, संरचना mii_dev, link);
		म_लिखो("'%s' ", dev->name);
	}
	माला_दो("\n");

	यदि (current_mii)
		म_लिखो("Current device: '%s'\n", current_mii->name);
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
पूर्णाङ्क miiphy_info(अचर वर्ण *devname, अचिन्हित वर्ण addr, अचिन्हित पूर्णाङ्क *oui,
		 अचिन्हित वर्ण *model, अचिन्हित वर्ण *rev)
{
	अचिन्हित पूर्णाङ्क reg = 0;
	अचिन्हित लघु tmp;

	यदि (miiphy_read(devname, addr, MII_PHYSID2, &tmp) != 0) {
		debug("PHY ID register 2 read failed\n");
		फर्काऊ -1;
	}
	reg = tmp;

	debug("MII_PHYSID2 @ 0x%x = 0x%04x\n", addr, reg);

	यदि (reg == 0xFFFF) {
		/* No physical device present at this address */
		फर्काऊ -1;
	}

	यदि (miiphy_read(devname, addr, MII_PHYSID1, &tmp) != 0) {
		debug("PHY ID register 1 read failed\n");
		फर्काऊ -1;
	}
	reg |= tmp << 16;
	debug("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (अचिन्हित वर्ण)((reg >> 4) & 0x0000003F);
	*rev = (अचिन्हित वर्ण)(reg & 0x0000000F);
	फर्काऊ 0;
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
पूर्णाङ्क miiphy_reset(अचर वर्ण *devname, अचिन्हित वर्ण addr)
{
	अचिन्हित लघु reg;
	पूर्णाङ्क timeout = 500;

	यदि (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
		debug("PHY status read failed\n");
		फर्काऊ -1;
	}
	यदि (miiphy_write(devname, addr, MII_BMCR, reg | BMCR_RESET) != 0) {
		debug("PHY reset failed\n");
		फर्काऊ -1;
	}
#यदि CONFIG_PHY_RESET_DELAY > 0
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#endif
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = 0x8000;
	जबसम्म (((reg & 0x8000) != 0) && timeout--) {
		यदि (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			फर्काऊ -1;
		}
		udelay(1000);
	}
	यदि ((reg & 0x8000) == 0) {
		फर्काऊ 0;
	} अन्यथा {
		माला_दो("PHY reset timed out\n");
		फर्काऊ -1;
	}
	फर्काऊ 0;
}
#endif /* !PHYLIB */

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
पूर्णाङ्क miiphy_speed(अचर वर्ण *devname, अचिन्हित वर्ण addr)
{
	u16 bmcr, anlpar, adv;

#यदि defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	यदि (miiphy_is_1000base_x(devname, addr))
		फर्काऊ _1000BASET;

	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	यदि (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		म_लिखो("PHY 1000BT status");
		गच्छ miiphy_read_failed;
	}
	यदि (btsr != 0xFFFF &&
			(btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)))
		फर्काऊ _1000BASET;
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	यदि (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		म_लिखो("PHY speed");
		गच्छ miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	यदि (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		यदि (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			म_लिखो("PHY AN speed");
			गच्छ miiphy_read_failed;
		}

		यदि (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			माला_दो("PHY AN adv speed");
			गच्छ miiphy_read_failed;
		}
		फर्काऊ ((anlpar & adv) & LPA_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	फर्काऊ (bmcr & BMCR_SPEED100) ? _100BASET : _10BASET;

miiphy_read_failed:
	म_लिखो(" read failed, assuming 10BASE-T\n");
	फर्काऊ _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
पूर्णाङ्क miiphy_duplex(अचर वर्ण *devname, अचिन्हित वर्ण addr)
{
	u16 bmcr, anlpar, adv;

#यदि defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	यदि (miiphy_is_1000base_x(devname, addr)) {
		/* 1000BASE-X */
		यदि (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			म_लिखो("1000BASE-X PHY AN duplex");
			गच्छ miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	यदि (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		म_लिखो("PHY 1000BT status");
		गच्छ miiphy_read_failed;
	}
	यदि (btsr != 0xFFFF) {
		यदि (btsr & PHY_1000BTSR_1000FD) {
			फर्काऊ FULL;
		} अन्यथा यदि (btsr & PHY_1000BTSR_1000HD) {
			फर्काऊ HALF;
		}
	}
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	यदि (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		माला_दो("PHY duplex");
		गच्छ miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	यदि (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		यदि (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			माला_दो("PHY AN duplex");
			गच्छ miiphy_read_failed;
		}

		यदि (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			माला_दो("PHY AN adv duplex");
			गच्छ miiphy_read_failed;
		}
		फर्काऊ ((anlpar & adv) & (LPA_10FULL | LPA_100FULL)) ?
		    FULL : HALF;
	}
	/* Get speed from basic control settings. */
	फर्काऊ (bmcr & BMCR_FULLDPLX) ? FULL : HALF;

miiphy_read_failed:
	म_लिखो(" read failed, assuming half duplex\n");
	फर्काऊ HALF;
}

/*****************************************************************************
 *
 * Return 1 if PHY supports 1000BASE-X, 0 if PHY supports 10BASE-T/100BASE-TX/
 * 1000BASE-T, or on error.
 */
पूर्णाङ्क miiphy_is_1000base_x(अचर वर्ण *devname, अचिन्हित वर्ण addr)
{
#यदि defined(CONFIG_PHY_GIGE)
	u16 exsr;

	यदि (miiphy_read(devname, addr, MII_ESTATUS, &exsr)) {
		म_लिखो("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		फर्काऊ 0;
	}
	फर्काऊ 0 != (exsr & (ESTATUS_1000XF | ESTATUS_1000XH));
#अन्यथा
	फर्काऊ 0;
#endif
}
