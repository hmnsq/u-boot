// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 */

/*
 * This provides a bit-banged interface to the ethernet MII management
 * channel.
 */

#समावेश <dm.h>
#समावेश <log.h>
#समावेश <miiphy.h>
#समावेश <phy.h>
#समावेश <linux/delay.h>

#समावेश <asm/types.h>
#समावेश <linux/list.h>
#समावेश <malloc.h>
#समावेश <net.h>

/* local debug macro */
#अघोषित MII_DEBUG

#अघोषित debug
#अगर_घोषित MII_DEBUG
#घोषणा debug(fmt, args...)	म_लिखो(fmt, ##args)
#अन्यथा
#घोषणा debug(fmt, args...)
#पूर्ण_अगर /* MII_DEBUG */

अटल LIST_शीर्ष(mii_devs);
अटल काष्ठा mii_dev *current_mii;

/*
 * Lookup the mii_dev struct by the registered device name.
 */
काष्ठा mii_dev *miiphy_get_dev_by_name(स्थिर अक्षर *devname)
{
	काष्ठा list_शीर्ष *entry;
	काष्ठा mii_dev *dev;

	अगर (!devname) {
		म_लिखो("NULL device name!\n");
		वापस शून्य;
	}

	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, काष्ठा mii_dev, link);
		अगर (म_भेद(dev->name, devname) == 0)
			वापस dev;
	}

	वापस शून्य;
}

काष्ठा mii_dev *mdio_alloc(व्योम)
{
	काष्ठा mii_dev *bus;

	bus = दो_स्मृति(माप(*bus));
	अगर (!bus)
		वापस bus;

	स_रखो(bus, 0, माप(*bus));

	/* initialize mii_dev struct fields */
	INIT_LIST_शीर्ष(&bus->link);

	वापस bus;
}

व्योम mdio_free(काष्ठा mii_dev *bus)
{
	मुक्त(bus);
}

पूर्णांक mdio_register(काष्ठा mii_dev *bus)
{
	अगर (!bus || !bus->पढ़ो || !bus->ग_लिखो)
		वापस -1;

	/* check if we have unique name */
	अगर (miiphy_get_dev_by_name(bus->name)) {
		म_लिखो("mdio_register: non unique device name '%s'\n",
			bus->name);
		वापस -1;
	}

	/* add it to the list */
	list_add_पुच्छ(&bus->link, &mii_devs);

	अगर (!current_mii)
		current_mii = bus;

	वापस 0;
}

पूर्णांक mdio_register_seq(काष्ठा mii_dev *bus, पूर्णांक seq)
{
	पूर्णांक ret;

	/* Setup a unique name for each mdio bus */
	ret = snprintf(bus->name, MDIO_NAME_LEN, "eth%d", seq);
	अगर (ret < 0)
		वापस ret;

	वापस mdio_register(bus);
}

पूर्णांक mdio_unregister(काष्ठा mii_dev *bus)
{
	अगर (!bus)
		वापस 0;

	/* delete it from the list */
	list_del(&bus->link);

	अगर (current_mii == bus)
		current_mii = शून्य;

	वापस 0;
}

व्योम mdio_list_devices(व्योम)
{
	काष्ठा list_शीर्ष *entry;

	list_for_each(entry, &mii_devs) {
		पूर्णांक i;
		काष्ठा mii_dev *bus = list_entry(entry, काष्ठा mii_dev, link);

		म_लिखो("%s:\n", bus->name);

		क्रम (i = 0; i < PHY_MAX_ADDR; i++) {
			काष्ठा phy_device *phydev = bus->phymap[i];

			अगर (phydev) {
				म_लिखो("%x - %s", i, phydev->drv->name);

				अगर (phydev->dev)
					म_लिखो(" <--> %s\n", phydev->dev->name);
				अन्यथा
					म_लिखो("\n");
			}
		}
	}
}

पूर्णांक miiphy_set_current_dev(स्थिर अक्षर *devname)
{
	काष्ठा mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname);
	अगर (dev) {
		current_mii = dev;
		वापस 0;
	}

	म_लिखो("No such device: %s\n", devname);

	वापस 1;
}

काष्ठा mii_dev *mdio_get_current_dev(व्योम)
{
	वापस current_mii;
}

काष्ठा list_शीर्ष *mdio_get_list_शीर्ष(व्योम)
{
	वापस &mii_devs;
}

काष्ठा phy_device *mdio_phydev_for_ethname(स्थिर अक्षर *ethname)
{
	काष्ठा list_शीर्ष *entry;
	काष्ठा mii_dev *bus;

	list_for_each(entry, &mii_devs) {
		पूर्णांक i;
		bus = list_entry(entry, काष्ठा mii_dev, link);

		क्रम (i = 0; i < PHY_MAX_ADDR; i++) {
			अगर (!bus->phymap[i] || !bus->phymap[i]->dev)
				जारी;

			अगर (म_भेद(bus->phymap[i]->dev->name, ethname) == 0)
				वापस bus->phymap[i];
		}
	}

	म_लिखो("%s is not a known ethernet\n", ethname);
	वापस शून्य;
}

स्थिर अक्षर *miiphy_get_current_dev(व्योम)
{
	अगर (current_mii)
		वापस current_mii->name;

	वापस शून्य;
}

अटल काष्ठा mii_dev *miiphy_get_active_dev(स्थिर अक्षर *devname)
{
	/* If the current mii is the one we want, return it */
	अगर (current_mii)
		अगर (म_भेद(current_mii->name, devname) == 0)
			वापस current_mii;

	/* Otherwise, set the active one to the one we want */
	अगर (miiphy_set_current_dev(devname))
		वापस शून्य;
	अन्यथा
		वापस current_mii;
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
पूर्णांक miiphy_read(स्थिर अक्षर *devname, अचिन्हित अक्षर addr, अचिन्हित अक्षर reg,
		 अचिन्हित लघु *मूल्य)
{
	काष्ठा mii_dev *bus;
	पूर्णांक ret;

	bus = miiphy_get_active_dev(devname);
	अगर (!bus)
		वापस 1;

	ret = bus->पढ़ो(bus, addr, MDIO_DEVAD_NONE, reg);
	अगर (ret < 0)
		वापस 1;

	*मूल्य = (अचिन्हित लघु)ret;
	वापस 0;
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
पूर्णांक miiphy_write(स्थिर अक्षर *devname, अचिन्हित अक्षर addr, अचिन्हित अक्षर reg,
		  अचिन्हित लघु मूल्य)
{
	काष्ठा mii_dev *bus;

	bus = miiphy_get_active_dev(devname);
	अगर (bus)
		वापस bus->ग_लिखो(bus, addr, MDIO_DEVAD_NONE, reg, मूल्य);

	वापस 1;
}

/*****************************************************************************
 *
 * Print out list of registered MII capable devices.
 */
व्योम miiphy_listdev(व्योम)
{
	काष्ठा list_शीर्ष *entry;
	काष्ठा mii_dev *dev;

	माला_दो("MII devices: ");
	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, काष्ठा mii_dev, link);
		म_लिखो("'%s' ", dev->name);
	}
	माला_दो("\n");

	अगर (current_mii)
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
पूर्णांक miiphy_info(स्थिर अक्षर *devname, अचिन्हित अक्षर addr, अचिन्हित पूर्णांक *oui,
		 अचिन्हित अक्षर *model, अचिन्हित अक्षर *rev)
{
	अचिन्हित पूर्णांक reg = 0;
	अचिन्हित लघु tmp;

	अगर (miiphy_read(devname, addr, MII_PHYSID2, &tmp) != 0) {
		debug("PHY ID register 2 read failed\n");
		वापस -1;
	}
	reg = tmp;

	debug("MII_PHYSID2 @ 0x%x = 0x%04x\n", addr, reg);

	अगर (reg == 0xFFFF) {
		/* No physical device present at this address */
		वापस -1;
	}

	अगर (miiphy_read(devname, addr, MII_PHYSID1, &tmp) != 0) {
		debug("PHY ID register 1 read failed\n");
		वापस -1;
	}
	reg |= tmp << 16;
	debug("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (अचिन्हित अक्षर)((reg >> 4) & 0x0000003F);
	*rev = (अचिन्हित अक्षर)(reg & 0x0000000F);
	वापस 0;
}

#अगर_अघोषित CONFIG_PHYLIB
/*****************************************************************************
 *
 * Reset the PHY.
 *
 * This API is deprecated. Use PHYLIB.
 *
 * Returns:
 *   0 on success
 */
पूर्णांक miiphy_reset(स्थिर अक्षर *devname, अचिन्हित अक्षर addr)
{
	अचिन्हित लघु reg;
	पूर्णांक timeout = 500;

	अगर (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
		debug("PHY status read failed\n");
		वापस -1;
	}
	अगर (miiphy_write(devname, addr, MII_BMCR, reg | BMCR_RESET) != 0) {
		debug("PHY reset failed\n");
		वापस -1;
	}
#अगर CONFIG_PHY_RESET_DELAY > 0
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#पूर्ण_अगर
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = 0x8000;
	जबतक (((reg & 0x8000) != 0) && timeout--) {
		अगर (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			वापस -1;
		}
		udelay(1000);
	}
	अगर ((reg & 0x8000) == 0) {
		वापस 0;
	} अन्यथा {
		माला_दो("PHY reset timed out\n");
		वापस -1;
	}
	वापस 0;
}
#पूर्ण_अगर /* !PHYLIB */

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
पूर्णांक miiphy_speed(स्थिर अक्षर *devname, अचिन्हित अक्षर addr)
{
	u16 bmcr, anlpar, adv;

#अगर defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	अगर (miiphy_is_1000base_x(devname, addr))
		वापस _1000BASET;

	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	अगर (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		म_लिखो("PHY 1000BT status");
		जाओ miiphy_read_failed;
	}
	अगर (btsr != 0xFFFF &&
			(btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)))
		वापस _1000BASET;
#पूर्ण_अगर /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	अगर (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		म_लिखो("PHY speed");
		जाओ miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	अगर (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		अगर (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			म_लिखो("PHY AN speed");
			जाओ miiphy_read_failed;
		}

		अगर (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			माला_दो("PHY AN adv speed");
			जाओ miiphy_read_failed;
		}
		वापस ((anlpar & adv) & LPA_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	वापस (bmcr & BMCR_SPEED100) ? _100BASET : _10BASET;

miiphy_read_failed:
	म_लिखो(" read failed, assuming 10BASE-T\n");
	वापस _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
पूर्णांक miiphy_duplex(स्थिर अक्षर *devname, अचिन्हित अक्षर addr)
{
	u16 bmcr, anlpar, adv;

#अगर defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	अगर (miiphy_is_1000base_x(devname, addr)) {
		/* 1000BASE-X */
		अगर (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			म_लिखो("1000BASE-X PHY AN duplex");
			जाओ miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	अगर (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		म_लिखो("PHY 1000BT status");
		जाओ miiphy_read_failed;
	}
	अगर (btsr != 0xFFFF) {
		अगर (btsr & PHY_1000BTSR_1000FD) {
			वापस FULL;
		} अन्यथा अगर (btsr & PHY_1000BTSR_1000HD) {
			वापस HALF;
		}
	}
#पूर्ण_अगर /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	अगर (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		माला_दो("PHY duplex");
		जाओ miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	अगर (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		अगर (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			माला_दो("PHY AN duplex");
			जाओ miiphy_read_failed;
		}

		अगर (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			माला_दो("PHY AN adv duplex");
			जाओ miiphy_read_failed;
		}
		वापस ((anlpar & adv) & (LPA_10FULL | LPA_100FULL)) ?
		    FULL : HALF;
	}
	/* Get speed from basic control settings. */
	वापस (bmcr & BMCR_FULLDPLX) ? FULL : HALF;

miiphy_read_failed:
	म_लिखो(" read failed, assuming half duplex\n");
	वापस HALF;
}

/*****************************************************************************
 *
 * Return 1 if PHY supports 1000BASE-X, 0 if PHY supports 10BASE-T/100BASE-TX/
 * 1000BASE-T, or on error.
 */
पूर्णांक miiphy_is_1000base_x(स्थिर अक्षर *devname, अचिन्हित अक्षर addr)
{
#अगर defined(CONFIG_PHY_GIGE)
	u16 exsr;

	अगर (miiphy_read(devname, addr, MII_ESTATUS, &exsr)) {
		म_लिखो("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		वापस 0;
	}
	वापस 0 != (exsr & (ESTATUS_1000XF | ESTATUS_1000XH));
#अन्यथा
	वापस 0;
#पूर्ण_अगर
}
