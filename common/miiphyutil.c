// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 */

/*
 * This provides a bit-banged interface to the ethernet MII management
 * channel.
 */

#അന്തര്ഭാവ <dm.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <miiphy.h>
#അന്തര്ഭാവ <phy.h>
#അന്തര്ഭാവ <linux/delay.h>

#അന്തര്ഭാവ <asm/types.h>
#അന്തര്ഭാവ <linux/list.h>
#അന്തര്ഭാവ <malloc.h>
#അന്തര്ഭാവ <net.h>

/* local debug macro */
#undef MII_DEBUG

#undef debug
#ifdef MII_DEBUG
#define debug(fmt, args...)	printf(fmt, ##args)
#അല്ലെങ്കിൽ
#define debug(fmt, args...)
#endif /* MII_DEBUG */

സ്ഥിതം LIST_HEAD(mii_devs);
സ്ഥിതം ഘടന mii_dev *current_mii;

/*
 * Lookup the mii_dev struct by the registered device name.
 */
ഘടന mii_dev *miiphy_get_dev_by_name(സ്ഥിരം അക്ഷരം *devname)
{
	ഘടന list_head *entry;
	ഘടന mii_dev *dev;

	എങ്കിൽ (!devname) {
		printf("NULL device name!\n");
		തിരികെ ശൂന്യ;
	}

	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, ഘടന mii_dev, link);
		എങ്കിൽ (strcmp(dev->name, devname) == 0)
			തിരികെ dev;
	}

	തിരികെ ശൂന്യ;
}

ഘടന mii_dev *mdio_alloc(ശൂന്യം)
{
	ഘടന mii_dev *bus;

	bus = malloc(വലുപ്പം(*bus));
	എങ്കിൽ (!bus)
		തിരികെ bus;

	memset(bus, 0, വലുപ്പം(*bus));

	/* initialize mii_dev struct fields */
	INIT_LIST_HEAD(&bus->link);

	തിരികെ bus;
}

ശൂന്യം mdio_free(ഘടന mii_dev *bus)
{
	സ്വതന്ത്രം(bus);
}

പൂർണ്ണസംഖ്യ mdio_register(ഘടന mii_dev *bus)
{
	എങ്കിൽ (!bus || !bus->പഠ || !bus->ലിഖ)
		തിരികെ -1;

	/* check if we have unique name */
	എങ്കിൽ (miiphy_get_dev_by_name(bus->name)) {
		printf("mdio_register: non unique device name '%s'\n",
			bus->name);
		തിരികെ -1;
	}

	/* add it to the list */
	list_add_tail(&bus->link, &mii_devs);

	എങ്കിൽ (!current_mii)
		current_mii = bus;

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ mdio_register_seq(ഘടന mii_dev *bus, പൂർണ്ണസംഖ്യ seq)
{
	പൂർണ്ണസംഖ്യ ret;

	/* Setup a unique name for each mdio bus */
	ret = snprintf(bus->name, MDIO_NAME_LEN, "eth%d", seq);
	എങ്കിൽ (ret < 0)
		തിരികെ ret;

	തിരികെ mdio_register(bus);
}

പൂർണ്ണസംഖ്യ mdio_unregister(ഘടന mii_dev *bus)
{
	എങ്കിൽ (!bus)
		തിരികെ 0;

	/* delete it from the list */
	list_del(&bus->link);

	എങ്കിൽ (current_mii == bus)
		current_mii = ശൂന്യ;

	തിരികെ 0;
}

ശൂന്യം mdio_list_devices(ശൂന്യം)
{
	ഘടന list_head *entry;

	list_for_each(entry, &mii_devs) {
		പൂർണ്ണസംഖ്യ i;
		ഘടന mii_dev *bus = list_entry(entry, ഘടന mii_dev, link);

		printf("%s:\n", bus->name);

		വേണ്ടി (i = 0; i < PHY_MAX_ADDR; i++) {
			ഘടന phy_device *phydev = bus->phymap[i];

			എങ്കിൽ (phydev) {
				printf("%x - %s", i, phydev->drv->name);

				എങ്കിൽ (phydev->dev)
					printf(" <--> %s\n", phydev->dev->name);
				അല്ലെങ്കിൽ
					printf("\n");
			}
		}
	}
}

പൂർണ്ണസംഖ്യ miiphy_set_current_dev(സ്ഥിരം അക്ഷരം *devname)
{
	ഘടന mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname);
	എങ്കിൽ (dev) {
		current_mii = dev;
		തിരികെ 0;
	}

	printf("No such device: %s\n", devname);

	തിരികെ 1;
}

ഘടന mii_dev *mdio_get_current_dev(ശൂന്യം)
{
	തിരികെ current_mii;
}

ഘടന list_head *mdio_get_list_head(ശൂന്യം)
{
	തിരികെ &mii_devs;
}

ഘടന phy_device *mdio_phydev_for_ethname(സ്ഥിരം അക്ഷരം *ethname)
{
	ഘടന list_head *entry;
	ഘടന mii_dev *bus;

	list_for_each(entry, &mii_devs) {
		പൂർണ്ണസംഖ്യ i;
		bus = list_entry(entry, ഘടന mii_dev, link);

		വേണ്ടി (i = 0; i < PHY_MAX_ADDR; i++) {
			എങ്കിൽ (!bus->phymap[i] || !bus->phymap[i]->dev)
				തുടരുക;

			എങ്കിൽ (strcmp(bus->phymap[i]->dev->name, ethname) == 0)
				തിരികെ bus->phymap[i];
		}
	}

	printf("%s is not a known ethernet\n", ethname);
	തിരികെ ശൂന്യ;
}

സ്ഥിരം അക്ഷരം *miiphy_get_current_dev(ശൂന്യം)
{
	എങ്കിൽ (current_mii)
		തിരികെ current_mii->name;

	തിരികെ ശൂന്യ;
}

സ്ഥിതം ഘടന mii_dev *miiphy_get_active_dev(സ്ഥിരം അക്ഷരം *devname)
{
	/* If the current mii is the one we want, return it */
	എങ്കിൽ (current_mii)
		എങ്കിൽ (strcmp(current_mii->name, devname) == 0)
			തിരികെ current_mii;

	/* Otherwise, set the active one to the one we want */
	എങ്കിൽ (miiphy_set_current_dev(devname))
		തിരികെ ശൂന്യ;
	അല്ലെങ്കിൽ
		തിരികെ current_mii;
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
പൂർണ്ണസംഖ്യ miiphy_read(സ്ഥിരം അക്ഷരം *devname, unsigned അക്ഷരം addr, unsigned അക്ഷരം reg,
		 unsigned short *മൂല്യം)
{
	ഘടന mii_dev *bus;
	പൂർണ്ണസംഖ്യ ret;

	bus = miiphy_get_active_dev(devname);
	എങ്കിൽ (!bus)
		തിരികെ 1;

	ret = bus->പഠ(bus, addr, MDIO_DEVAD_NONE, reg);
	എങ്കിൽ (ret < 0)
		തിരികെ 1;

	*മൂല്യം = (unsigned short)ret;
	തിരികെ 0;
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
പൂർണ്ണസംഖ്യ miiphy_write(സ്ഥിരം അക്ഷരം *devname, unsigned അക്ഷരം addr, unsigned അക്ഷരം reg,
		  unsigned short മൂല്യം)
{
	ഘടന mii_dev *bus;

	bus = miiphy_get_active_dev(devname);
	എങ്കിൽ (bus)
		തിരികെ bus->ലിഖ(bus, addr, MDIO_DEVAD_NONE, reg, മൂല്യം);

	തിരികെ 1;
}

/*****************************************************************************
 *
 * Print out list of registered MII capable devices.
 */
ശൂന്യം miiphy_listdev(ശൂന്യം)
{
	ഘടന list_head *entry;
	ഘടന mii_dev *dev;

	puts("MII devices: ");
	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, ഘടന mii_dev, link);
		printf("'%s' ", dev->name);
	}
	puts("\n");

	എങ്കിൽ (current_mii)
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
പൂർണ്ണസംഖ്യ miiphy_info(സ്ഥിരം അക്ഷരം *devname, unsigned അക്ഷരം addr, unsigned പൂർണ്ണസംഖ്യ *oui,
		 unsigned അക്ഷരം *model, unsigned അക്ഷരം *rev)
{
	unsigned പൂർണ്ണസംഖ്യ reg = 0;
	unsigned short tmp;

	എങ്കിൽ (miiphy_read(devname, addr, MII_PHYSID2, &tmp) != 0) {
		debug("PHY ID register 2 read failed\n");
		തിരികെ -1;
	}
	reg = tmp;

	debug("MII_PHYSID2 @ 0x%x = 0x%04x\n", addr, reg);

	എങ്കിൽ (reg == 0xFFFF) {
		/* No physical device present at this address */
		തിരികെ -1;
	}

	എങ്കിൽ (miiphy_read(devname, addr, MII_PHYSID1, &tmp) != 0) {
		debug("PHY ID register 1 read failed\n");
		തിരികെ -1;
	}
	reg |= tmp << 16;
	debug("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (unsigned അക്ഷരം)((reg >> 4) & 0x0000003F);
	*rev = (unsigned അക്ഷരം)(reg & 0x0000000F);
	തിരികെ 0;
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
പൂർണ്ണസംഖ്യ miiphy_reset(സ്ഥിരം അക്ഷരം *devname, unsigned അക്ഷരം addr)
{
	unsigned short reg;
	പൂർണ്ണസംഖ്യ timeout = 500;

	എങ്കിൽ (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
		debug("PHY status read failed\n");
		തിരികെ -1;
	}
	എങ്കിൽ (miiphy_write(devname, addr, MII_BMCR, reg | BMCR_RESET) != 0) {
		debug("PHY reset failed\n");
		തിരികെ -1;
	}
#എങ്കിൽ CONFIG_PHY_RESET_DELAY > 0
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#endif
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = 0x8000;
	ആകുമ്പോൾ (((reg & 0x8000) != 0) && timeout--) {
		എങ്കിൽ (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			തിരികെ -1;
		}
		udelay(1000);
	}
	എങ്കിൽ ((reg & 0x8000) == 0) {
		തിരികെ 0;
	} അല്ലെങ്കിൽ {
		puts("PHY reset timed out\n");
		തിരികെ -1;
	}
	തിരികെ 0;
}
#endif /* !PHYLIB */

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
പൂർണ്ണസംഖ്യ miiphy_speed(സ്ഥിരം അക്ഷരം *devname, unsigned അക്ഷരം addr)
{
	u16 bmcr, anlpar, adv;

#എങ്കിൽ defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	എങ്കിൽ (miiphy_is_1000base_x(devname, addr))
		തിരികെ _1000BASET;

	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	എങ്കിൽ (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		പോകുക miiphy_read_failed;
	}
	എങ്കിൽ (btsr != 0xFFFF &&
			(btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)))
		തിരികെ _1000BASET;
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	എങ്കിൽ (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		printf("PHY speed");
		പോകുക miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	എങ്കിൽ (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		എങ്കിൽ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("PHY AN speed");
			പോകുക miiphy_read_failed;
		}

		എങ്കിൽ (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv speed");
			പോകുക miiphy_read_failed;
		}
		തിരികെ ((anlpar & adv) & LPA_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	തിരികെ (bmcr & BMCR_SPEED100) ? _100BASET : _10BASET;

miiphy_read_failed:
	printf(" read failed, assuming 10BASE-T\n");
	തിരികെ _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
പൂർണ്ണസംഖ്യ miiphy_duplex(സ്ഥിരം അക്ഷരം *devname, unsigned അക്ഷരം addr)
{
	u16 bmcr, anlpar, adv;

#എങ്കിൽ defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	എങ്കിൽ (miiphy_is_1000base_x(devname, addr)) {
		/* 1000BASE-X */
		എങ്കിൽ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("1000BASE-X PHY AN duplex");
			പോകുക miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	എങ്കിൽ (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		പോകുക miiphy_read_failed;
	}
	എങ്കിൽ (btsr != 0xFFFF) {
		എങ്കിൽ (btsr & PHY_1000BTSR_1000FD) {
			തിരികെ FULL;
		} അല്ലെങ്കിൽ എങ്കിൽ (btsr & PHY_1000BTSR_1000HD) {
			തിരികെ HALF;
		}
	}
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	എങ്കിൽ (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		puts("PHY duplex");
		പോകുക miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	എങ്കിൽ (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		എങ്കിൽ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			puts("PHY AN duplex");
			പോകുക miiphy_read_failed;
		}

		എങ്കിൽ (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv duplex");
			പോകുക miiphy_read_failed;
		}
		തിരികെ ((anlpar & adv) & (LPA_10FULL | LPA_100FULL)) ?
		    FULL : HALF;
	}
	/* Get speed from basic control settings. */
	തിരികെ (bmcr & BMCR_FULLDPLX) ? FULL : HALF;

miiphy_read_failed:
	printf(" read failed, assuming half duplex\n");
	തിരികെ HALF;
}

/*****************************************************************************
 *
 * Return 1 if PHY supports 1000BASE-X, 0 if PHY supports 10BASE-T/100BASE-TX/
 * 1000BASE-T, or on error.
 */
പൂർണ്ണസംഖ്യ miiphy_is_1000base_x(സ്ഥിരം അക്ഷരം *devname, unsigned അക്ഷരം addr)
{
#എങ്കിൽ defined(CONFIG_PHY_GIGE)
	u16 exsr;

	എങ്കിൽ (miiphy_read(devname, addr, MII_ESTATUS, &exsr)) {
		printf("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		തിരികെ 0;
	}
	തിരികെ 0 != (exsr & (ESTATUS_1000XF | ESTATUS_1000XH));
#അല്ലെങ്കിൽ
	തിരികെ 0;
#endif
}
