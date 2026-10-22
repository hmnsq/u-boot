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
संरचना mii_dev *miiphy_get_dev_by_name(स्थिरांक अक्षर *devname)
{
	संरचना list_head *entry;
	संरचना mii_dev *dev;

	जर (!devname) {
		म_लिखो("NULL device name!\n");
		परत रिक्त;
	}

	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, संरचना mii_dev, link);
		जर (म_भेद(dev->name, devname) == 0)
			परत dev;
	}

	परत रिक्त;
}

संरचना mii_dev *mdio_alloc(शून्य)
{
	संरचना mii_dev *bus;

	bus = दो_स्मृति(आकार(*bus));
	जर (!bus)
		परत bus;

	स_रखो(bus, 0, आकार(*bus));

	/* initialize mii_dev struct fields */
	INIT_LIST_HEAD(&bus->link);

	परत bus;
}

शून्य mdio_free(संरचना mii_dev *bus)
{
	मुक्त(bus);
}

पूर्णांक mdio_register(संरचना mii_dev *bus)
{
	जर (!bus || !bus->पठ || !bus->लिख)
		परत -1;

	/* check if we have unique name */
	जर (miiphy_get_dev_by_name(bus->name)) {
		म_लिखो("mdio_register: non unique device name '%s'\n",
			bus->name);
		परत -1;
	}

	/* add it to the list */
	list_add_tail(&bus->link, &mii_devs);

	जर (!current_mii)
		current_mii = bus;

	परत 0;
}

पूर्णांक mdio_register_seq(संरचना mii_dev *bus, पूर्णांक seq)
{
	पूर्णांक ret;

	/* Setup a unique name for each mdio bus */
	ret = snprintf(bus->name, MDIO_NAME_LEN, "eth%d", seq);
	जर (ret < 0)
		परत ret;

	परत mdio_register(bus);
}

पूर्णांक mdio_unregister(संरचना mii_dev *bus)
{
	जर (!bus)
		परत 0;

	/* delete it from the list */
	list_del(&bus->link);

	जर (current_mii == bus)
		current_mii = रिक्त;

	परत 0;
}

शून्य mdio_list_devices(शून्य)
{
	संरचना list_head *entry;

	list_for_each(entry, &mii_devs) {
		पूर्णांक i;
		संरचना mii_dev *bus = list_entry(entry, संरचना mii_dev, link);

		म_लिखो("%s:\n", bus->name);

		साठी (i = 0; i < PHY_MAX_ADDR; i++) {
			संरचना phy_device *phydev = bus->phymap[i];

			जर (phydev) {
				म_लिखो("%x - %s", i, phydev->drv->name);

				जर (phydev->dev)
					म_लिखो(" <--> %s\n", phydev->dev->name);
				अन्यथा
					म_लिखो("\n");
			}
		}
	}
}

पूर्णांक miiphy_set_current_dev(स्थिरांक अक्षर *devname)
{
	संरचना mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname);
	जर (dev) {
		current_mii = dev;
		परत 0;
	}

	म_लिखो("No such device: %s\n", devname);

	परत 1;
}

संरचना mii_dev *mdio_get_current_dev(शून्य)
{
	परत current_mii;
}

संरचना list_head *mdio_get_list_head(शून्य)
{
	परत &mii_devs;
}

संरचना phy_device *mdio_phydev_for_ethname(स्थिरांक अक्षर *ethname)
{
	संरचना list_head *entry;
	संरचना mii_dev *bus;

	list_for_each(entry, &mii_devs) {
		पूर्णांक i;
		bus = list_entry(entry, संरचना mii_dev, link);

		साठी (i = 0; i < PHY_MAX_ADDR; i++) {
			जर (!bus->phymap[i] || !bus->phymap[i]->dev)
				सुरूठेवा;

			जर (म_भेद(bus->phymap[i]->dev->name, ethname) == 0)
				परत bus->phymap[i];
		}
	}

	म_लिखो("%s is not a known ethernet\n", ethname);
	परत रिक्त;
}

स्थिरांक अक्षर *miiphy_get_current_dev(शून्य)
{
	जर (current_mii)
		परत current_mii->name;

	परत रिक्त;
}

स्थिर संरचना mii_dev *miiphy_get_active_dev(स्थिरांक अक्षर *devname)
{
	/* If the current mii is the one we want, return it */
	जर (current_mii)
		जर (म_भेद(current_mii->name, devname) == 0)
			परत current_mii;

	/* Otherwise, set the active one to the one we want */
	जर (miiphy_set_current_dev(devname))
		परत रिक्त;
	अन्यथा
		परत current_mii;
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
पूर्णांक miiphy_read(स्थिरांक अक्षर *devname, अचिन्हित अक्षर addr, अचिन्हित अक्षर reg,
		 अचिन्हित लघु *मूल्य)
{
	संरचना mii_dev *bus;
	पूर्णांक ret;

	bus = miiphy_get_active_dev(devname);
	जर (!bus)
		परत 1;

	ret = bus->पठ(bus, addr, MDIO_DEVAD_NONE, reg);
	जर (ret < 0)
		परत 1;

	*मूल्य = (अचिन्हित लघु)ret;
	परत 0;
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
पूर्णांक miiphy_write(स्थिरांक अक्षर *devname, अचिन्हित अक्षर addr, अचिन्हित अक्षर reg,
		  अचिन्हित लघु मूल्य)
{
	संरचना mii_dev *bus;

	bus = miiphy_get_active_dev(devname);
	जर (bus)
		परत bus->लिख(bus, addr, MDIO_DEVAD_NONE, reg, मूल्य);

	परत 1;
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

	जर (current_mii)
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
पूर्णांक miiphy_info(स्थिरांक अक्षर *devname, अचिन्हित अक्षर addr, अचिन्हित पूर्णांक *oui,
		 अचिन्हित अक्षर *model, अचिन्हित अक्षर *rev)
{
	अचिन्हित पूर्णांक reg = 0;
	अचिन्हित लघु tmp;

	जर (miiphy_read(devname, addr, MII_PHYSID2, &tmp) != 0) {
		debug("PHY ID register 2 read failed\n");
		परत -1;
	}
	reg = tmp;

	debug("MII_PHYSID2 @ 0x%x = 0x%04x\n", addr, reg);

	जर (reg == 0xFFFF) {
		/* No physical device present at this address */
		परत -1;
	}

	जर (miiphy_read(devname, addr, MII_PHYSID1, &tmp) != 0) {
		debug("PHY ID register 1 read failed\n");
		परत -1;
	}
	reg |= tmp << 16;
	debug("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (अचिन्हित अक्षर)((reg >> 4) & 0x0000003F);
	*rev = (अचिन्हित अक्षर)(reg & 0x0000000F);
	परत 0;
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
पूर्णांक miiphy_reset(स्थिरांक अक्षर *devname, अचिन्हित अक्षर addr)
{
	अचिन्हित लघु reg;
	पूर्णांक timeout = 500;

	जर (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
		debug("PHY status read failed\n");
		परत -1;
	}
	जर (miiphy_write(devname, addr, MII_BMCR, reg | BMCR_RESET) != 0) {
		debug("PHY reset failed\n");
		परत -1;
	}
#जर CONFIG_PHY_RESET_DELAY > 0
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#endif
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = 0x8000;
	पर्यंत (((reg & 0x8000) != 0) && timeout--) {
		जर (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			परत -1;
		}
		udelay(1000);
	}
	जर ((reg & 0x8000) == 0) {
		परत 0;
	} अन्यथा {
		माला_दो("PHY reset timed out\n");
		परत -1;
	}
	परत 0;
}
#endif /* !PHYLIB */

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
पूर्णांक miiphy_speed(स्थिरांक अक्षर *devname, अचिन्हित अक्षर addr)
{
	u16 bmcr, anlpar, adv;

#जर defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	जर (miiphy_is_1000base_x(devname, addr))
		परत _1000BASET;

	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	जर (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		म_लिखो("PHY 1000BT status");
		जा miiphy_read_failed;
	}
	जर (btsr != 0xFFFF &&
			(btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)))
		परत _1000BASET;
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	जर (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		म_लिखो("PHY speed");
		जा miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	जर (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		जर (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			म_लिखो("PHY AN speed");
			जा miiphy_read_failed;
		}

		जर (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			माला_दो("PHY AN adv speed");
			जा miiphy_read_failed;
		}
		परत ((anlpar & adv) & LPA_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	परत (bmcr & BMCR_SPEED100) ? _100BASET : _10BASET;

miiphy_read_failed:
	म_लिखो(" read failed, assuming 10BASE-T\n");
	परत _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
पूर्णांक miiphy_duplex(स्थिरांक अक्षर *devname, अचिन्हित अक्षर addr)
{
	u16 bmcr, anlpar, adv;

#जर defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	जर (miiphy_is_1000base_x(devname, addr)) {
		/* 1000BASE-X */
		जर (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			म_लिखो("1000BASE-X PHY AN duplex");
			जा miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	जर (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		म_लिखो("PHY 1000BT status");
		जा miiphy_read_failed;
	}
	जर (btsr != 0xFFFF) {
		जर (btsr & PHY_1000BTSR_1000FD) {
			परत FULL;
		} अन्यथा जर (btsr & PHY_1000BTSR_1000HD) {
			परत HALF;
		}
	}
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	जर (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		माला_दो("PHY duplex");
		जा miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	जर (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		जर (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			माला_दो("PHY AN duplex");
			जा miiphy_read_failed;
		}

		जर (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			माला_दो("PHY AN adv duplex");
			जा miiphy_read_failed;
		}
		परत ((anlpar & adv) & (LPA_10FULL | LPA_100FULL)) ?
		    FULL : HALF;
	}
	/* Get speed from basic control settings. */
	परत (bmcr & BMCR_FULLDPLX) ? FULL : HALF;

miiphy_read_failed:
	म_लिखो(" read failed, assuming half duplex\n");
	परत HALF;
}

/*****************************************************************************
 *
 * Return 1 if PHY supports 1000BASE-X, 0 if PHY supports 10BASE-T/100BASE-TX/
 * 1000BASE-T, or on error.
 */
पूर्णांक miiphy_is_1000base_x(स्थिरांक अक्षर *devname, अचिन्हित अक्षर addr)
{
#जर defined(CONFIG_PHY_GIGE)
	u16 exsr;

	जर (miiphy_read(devname, addr, MII_ESTATUS, &exsr)) {
		म_लिखो("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		परत 0;
	}
	परत 0 != (exsr & (ESTATUS_1000XF | ESTATUS_1000XH));
#अन्यथा
	परत 0;
#endif
}
