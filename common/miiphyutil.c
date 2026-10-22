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
#define debug(fmt, args...)	printf(fmt, ##args)
#अञ्ञथा
#define debug(fmt, args...)
#endif /* MII_DEBUG */

स्थिर LIST_HEAD(mii_devs);
स्थिर संरचना mii_dev *current_mii;

/*
 * Lookup the mii_dev struct by the registered device name.
 */
संरचना mii_dev *miiphy_get_dev_by_name(ध्रुव वर्ण *devname)
{
	संरचना list_head *entry;
	संरचना mii_dev *dev;

	सचे (!devname) {
		printf("NULL device name!\n");
		पटिनिवत्त रिक्त;
	}

	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, संरचना mii_dev, link);
		सचे (strcmp(dev->name, devname) == 0)
			पटिनिवत्त dev;
	}

	पटिनिवत्त रिक्त;
}

संरचना mii_dev *mdio_alloc(शून्य)
{
	संरचना mii_dev *bus;

	bus = malloc(परिमाण(*bus));
	सचे (!bus)
		पटिनिवत्त bus;

	memset(bus, 0, परिमाण(*bus));

	/* initialize mii_dev struct fields */
	INIT_LIST_HEAD(&bus->link);

	पटिनिवत्त bus;
}

शून्य mdio_free(संरचना mii_dev *bus)
{
	मुक्त(bus);
}

पूर्णाङ्क mdio_register(संरचना mii_dev *bus)
{
	सचे (!bus || !bus->पठ || !bus->लिख)
		पटिनिवत्त -1;

	/* check if we have unique name */
	सचे (miiphy_get_dev_by_name(bus->name)) {
		printf("mdio_register: non unique device name '%s'\n",
			bus->name);
		पटिनिवत्त -1;
	}

	/* add it to the list */
	list_add_tail(&bus->link, &mii_devs);

	सचे (!current_mii)
		current_mii = bus;

	पटिनिवत्त 0;
}

पूर्णाङ्क mdio_register_seq(संरचना mii_dev *bus, पूर्णाङ्क seq)
{
	पूर्णाङ्क ret;

	/* Setup a unique name for each mdio bus */
	ret = snprintf(bus->name, MDIO_NAME_LEN, "eth%d", seq);
	सचे (ret < 0)
		पटिनिवत्त ret;

	पटिनिवत्त mdio_register(bus);
}

पूर्णाङ्क mdio_unregister(संरचना mii_dev *bus)
{
	सचे (!bus)
		पटिनिवत्त 0;

	/* delete it from the list */
	list_del(&bus->link);

	सचे (current_mii == bus)
		current_mii = रिक्त;

	पटिनिवत्त 0;
}

शून्य mdio_list_devices(शून्य)
{
	संरचना list_head *entry;

	list_for_each(entry, &mii_devs) {
		पूर्णाङ्क i;
		संरचना mii_dev *bus = list_entry(entry, संरचना mii_dev, link);

		printf("%s:\n", bus->name);

		अत्थाय (i = 0; i < PHY_MAX_ADDR; i++) {
			संरचना phy_device *phydev = bus->phymap[i];

			सचे (phydev) {
				printf("%x - %s", i, phydev->drv->name);

				सचे (phydev->dev)
					printf(" <--> %s\n", phydev->dev->name);
				अञ्ञथा
					printf("\n");
			}
		}
	}
}

पूर्णाङ्क miiphy_set_current_dev(ध्रुव वर्ण *devname)
{
	संरचना mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname);
	सचे (dev) {
		current_mii = dev;
		पटिनिवत्त 0;
	}

	printf("No such device: %s\n", devname);

	पटिनिवत्त 1;
}

संरचना mii_dev *mdio_get_current_dev(शून्य)
{
	पटिनिवत्त current_mii;
}

संरचना list_head *mdio_get_list_head(शून्य)
{
	पटिनिवत्त &mii_devs;
}

संरचना phy_device *mdio_phydev_for_ethname(ध्रुव वर्ण *ethname)
{
	संरचना list_head *entry;
	संरचना mii_dev *bus;

	list_for_each(entry, &mii_devs) {
		पूर्णाङ्क i;
		bus = list_entry(entry, संरचना mii_dev, link);

		अत्थाय (i = 0; i < PHY_MAX_ADDR; i++) {
			सचे (!bus->phymap[i] || !bus->phymap[i]->dev)
				अनुवर्त;

			सचे (strcmp(bus->phymap[i]->dev->name, ethname) == 0)
				पटिनिवत्त bus->phymap[i];
		}
	}

	printf("%s is not a known ethernet\n", ethname);
	पटिनिवत्त रिक्त;
}

ध्रुव वर्ण *miiphy_get_current_dev(शून्य)
{
	सचे (current_mii)
		पटिनिवत्त current_mii->name;

	पटिनिवत्त रिक्त;
}

स्थिर संरचना mii_dev *miiphy_get_active_dev(ध्रुव वर्ण *devname)
{
	/* If the current mii is the one we want, return it */
	सचे (current_mii)
		सचे (strcmp(current_mii->name, devname) == 0)
			पटिनिवत्त current_mii;

	/* Otherwise, set the active one to the one we want */
	सचे (miiphy_set_current_dev(devname))
		पटिनिवत्त रिक्त;
	अञ्ञथा
		पटिनिवत्त current_mii;
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
पूर्णाङ्क miiphy_read(ध्रुव वर्ण *devname, unsigned वर्ण addr, unsigned वर्ण reg,
		 unsigned short *मानम्)
{
	संरचना mii_dev *bus;
	पूर्णाङ्क ret;

	bus = miiphy_get_active_dev(devname);
	सचे (!bus)
		पटिनिवत्त 1;

	ret = bus->पठ(bus, addr, MDIO_DEVAD_NONE, reg);
	सचे (ret < 0)
		पटिनिवत्त 1;

	*मानम् = (unsigned short)ret;
	पटिनिवत्त 0;
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
पूर्णाङ्क miiphy_write(ध्रुव वर्ण *devname, unsigned वर्ण addr, unsigned वर्ण reg,
		  unsigned short मानम्)
{
	संरचना mii_dev *bus;

	bus = miiphy_get_active_dev(devname);
	सचे (bus)
		पटिनिवत्त bus->लिख(bus, addr, MDIO_DEVAD_NONE, reg, मानम्);

	पटिनिवत्त 1;
}

/*****************************************************************************
 *
 * Print out list of registered MII capable devices.
 */
शून्य miiphy_listdev(शून्य)
{
	संरचना list_head *entry;
	संरचना mii_dev *dev;

	puts("MII devices: ");
	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, संरचना mii_dev, link);
		printf("'%s' ", dev->name);
	}
	puts("\n");

	सचे (current_mii)
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
पूर्णाङ्क miiphy_info(ध्रुव वर्ण *devname, unsigned वर्ण addr, unsigned पूर्णाङ्क *oui,
		 unsigned वर्ण *model, unsigned वर्ण *rev)
{
	unsigned पूर्णाङ्क reg = 0;
	unsigned short tmp;

	सचे (miiphy_read(devname, addr, MII_PHYSID2, &tmp) != 0) {
		debug("PHY ID register 2 read failed\n");
		पटिनिवत्त -1;
	}
	reg = tmp;

	debug("MII_PHYSID2 @ 0x%x = 0x%04x\n", addr, reg);

	सचे (reg == 0xFFFF) {
		/* No physical device present at this address */
		पटिनिवत्त -1;
	}

	सचे (miiphy_read(devname, addr, MII_PHYSID1, &tmp) != 0) {
		debug("PHY ID register 1 read failed\n");
		पटिनिवत्त -1;
	}
	reg |= tmp << 16;
	debug("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (unsigned वर्ण)((reg >> 4) & 0x0000003F);
	*rev = (unsigned वर्ण)(reg & 0x0000000F);
	पटिनिवत्त 0;
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
पूर्णाङ्क miiphy_reset(ध्रुव वर्ण *devname, unsigned वर्ण addr)
{
	unsigned short reg;
	पूर्णाङ्क timeout = 500;

	सचे (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
		debug("PHY status read failed\n");
		पटिनिवत्त -1;
	}
	सचे (miiphy_write(devname, addr, MII_BMCR, reg | BMCR_RESET) != 0) {
		debug("PHY reset failed\n");
		पटिनिवत्त -1;
	}
#सचे CONFIG_PHY_RESET_DELAY > 0
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#endif
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = 0x8000;
	याव (((reg & 0x8000) != 0) && timeout--) {
		सचे (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			पटिनिवत्त -1;
		}
		udelay(1000);
	}
	सचे ((reg & 0x8000) == 0) {
		पटिनिवत्त 0;
	} अञ्ञथा {
		puts("PHY reset timed out\n");
		पटिनिवत्त -1;
	}
	पटिनिवत्त 0;
}
#endif /* !PHYLIB */

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
पूर्णाङ्क miiphy_speed(ध्रुव वर्ण *devname, unsigned वर्ण addr)
{
	u16 bmcr, anlpar, adv;

#सचे defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	सचे (miiphy_is_1000base_x(devname, addr))
		पटिनिवत्त _1000BASET;

	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	सचे (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		गच्छ miiphy_read_failed;
	}
	सचे (btsr != 0xFFFF &&
			(btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)))
		पटिनिवत्त _1000BASET;
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	सचे (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		printf("PHY speed");
		गच्छ miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	सचे (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		सचे (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("PHY AN speed");
			गच्छ miiphy_read_failed;
		}

		सचे (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv speed");
			गच्छ miiphy_read_failed;
		}
		पटिनिवत्त ((anlpar & adv) & LPA_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	पटिनिवत्त (bmcr & BMCR_SPEED100) ? _100BASET : _10BASET;

miiphy_read_failed:
	printf(" read failed, assuming 10BASE-T\n");
	पटिनिवत्त _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
पूर्णाङ्क miiphy_duplex(ध्रुव वर्ण *devname, unsigned वर्ण addr)
{
	u16 bmcr, anlpar, adv;

#सचे defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	सचे (miiphy_is_1000base_x(devname, addr)) {
		/* 1000BASE-X */
		सचे (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("1000BASE-X PHY AN duplex");
			गच्छ miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	सचे (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		गच्छ miiphy_read_failed;
	}
	सचे (btsr != 0xFFFF) {
		सचे (btsr & PHY_1000BTSR_1000FD) {
			पटिनिवत्त FULL;
		} अञ्ञथा सचे (btsr & PHY_1000BTSR_1000HD) {
			पटिनिवत्त HALF;
		}
	}
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	सचे (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		puts("PHY duplex");
		गच्छ miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	सचे (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		सचे (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			puts("PHY AN duplex");
			गच्छ miiphy_read_failed;
		}

		सचे (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv duplex");
			गच्छ miiphy_read_failed;
		}
		पटिनिवत्त ((anlpar & adv) & (LPA_10FULL | LPA_100FULL)) ?
		    FULL : HALF;
	}
	/* Get speed from basic control settings. */
	पटिनिवत्त (bmcr & BMCR_FULLDPLX) ? FULL : HALF;

miiphy_read_failed:
	printf(" read failed, assuming half duplex\n");
	पटिनिवत्त HALF;
}

/*****************************************************************************
 *
 * Return 1 if PHY supports 1000BASE-X, 0 if PHY supports 10BASE-T/100BASE-TX/
 * 1000BASE-T, or on error.
 */
पूर्णाङ्क miiphy_is_1000base_x(ध्रुव वर्ण *devname, unsigned वर्ण addr)
{
#सचे defined(CONFIG_PHY_GIGE)
	u16 exsr;

	सचे (miiphy_read(devname, addr, MII_ESTATUS, &exsr)) {
		printf("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		पटिनिवत्त 0;
	}
	पटिनिवत्त 0 != (exsr & (ESTATUS_1000XF | ESTATUS_1000XH));
#अञ्ञथा
	पटिनिवत्त 0;
#endif
}
