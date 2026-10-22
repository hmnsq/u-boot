// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 */

/*
 * This provides a bit-banged interface to the ethernet MII management
 * channel.
 */

#અન્તર્ભાવ <dm.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <miiphy.h>
#અન્તર્ભાવ <phy.h>
#અન્તર્ભાવ <linux/delay.h>

#અન્તર્ભાવ <asm/types.h>
#અન્તર્ભાવ <linux/list.h>
#અન્તર્ભાવ <malloc.h>
#અન્તર્ભાવ <net.h>

/* local debug macro */
#undef MII_DEBUG

#undef debug
#ifdef MII_DEBUG
#define debug(fmt, args...)	printf(fmt, ##args)
#નહીંતર
#define debug(fmt, args...)
#endif /* MII_DEBUG */

સ્થિર LIST_HEAD(mii_devs);
સ્થિર સંરચના mii_dev *current_mii;

/*
 * Lookup the mii_dev struct by the registered device name.
 */
સંરચના mii_dev *miiphy_get_dev_by_name(અચલ અક્ષર *devname)
{
	સંરચના list_head *entry;
	સંરચના mii_dev *dev;

	જો (!devname) {
		printf("NULL device name!\n");
		પાછા ખાલી;
	}

	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, સંરચના mii_dev, link);
		જો (strcmp(dev->name, devname) == 0)
			પાછા dev;
	}

	પાછા ખાલી;
}

સંરચના mii_dev *mdio_alloc(શૂન્ય)
{
	સંરચના mii_dev *bus;

	bus = malloc(કદ(*bus));
	જો (!bus)
		પાછા bus;

	memset(bus, 0, કદ(*bus));

	/* initialize mii_dev struct fields */
	INIT_LIST_HEAD(&bus->link);

	પાછા bus;
}

શૂન્ય mdio_free(સંરચના mii_dev *bus)
{
	મુક્ત(bus);
}

પૂર્ણાંક mdio_register(સંરચના mii_dev *bus)
{
	જો (!bus || !bus->પઠ || !bus->લિખ)
		પાછા -1;

	/* check if we have unique name */
	જો (miiphy_get_dev_by_name(bus->name)) {
		printf("mdio_register: non unique device name '%s'\n",
			bus->name);
		પાછા -1;
	}

	/* add it to the list */
	list_add_tail(&bus->link, &mii_devs);

	જો (!current_mii)
		current_mii = bus;

	પાછા 0;
}

પૂર્ણાંક mdio_register_seq(સંરચના mii_dev *bus, પૂર્ણાંક seq)
{
	પૂર્ણાંક ret;

	/* Setup a unique name for each mdio bus */
	ret = snprintf(bus->name, MDIO_NAME_LEN, "eth%d", seq);
	જો (ret < 0)
		પાછા ret;

	પાછા mdio_register(bus);
}

પૂર્ણાંક mdio_unregister(સંરચના mii_dev *bus)
{
	જો (!bus)
		પાછા 0;

	/* delete it from the list */
	list_del(&bus->link);

	જો (current_mii == bus)
		current_mii = ખાલી;

	પાછા 0;
}

શૂન્ય mdio_list_devices(શૂન્ય)
{
	સંરચના list_head *entry;

	list_for_each(entry, &mii_devs) {
		પૂર્ણાંક i;
		સંરચના mii_dev *bus = list_entry(entry, સંરચના mii_dev, link);

		printf("%s:\n", bus->name);

		માટે (i = 0; i < PHY_MAX_ADDR; i++) {
			સંરચના phy_device *phydev = bus->phymap[i];

			જો (phydev) {
				printf("%x - %s", i, phydev->drv->name);

				જો (phydev->dev)
					printf(" <--> %s\n", phydev->dev->name);
				નહીંતર
					printf("\n");
			}
		}
	}
}

પૂર્ણાંક miiphy_set_current_dev(અચલ અક્ષર *devname)
{
	સંરચના mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname);
	જો (dev) {
		current_mii = dev;
		પાછા 0;
	}

	printf("No such device: %s\n", devname);

	પાછા 1;
}

સંરચના mii_dev *mdio_get_current_dev(શૂન્ય)
{
	પાછા current_mii;
}

સંરચના list_head *mdio_get_list_head(શૂન્ય)
{
	પાછા &mii_devs;
}

સંરચના phy_device *mdio_phydev_for_ethname(અચલ અક્ષર *ethname)
{
	સંરચના list_head *entry;
	સંરચના mii_dev *bus;

	list_for_each(entry, &mii_devs) {
		પૂર્ણાંક i;
		bus = list_entry(entry, સંરચના mii_dev, link);

		માટે (i = 0; i < PHY_MAX_ADDR; i++) {
			જો (!bus->phymap[i] || !bus->phymap[i]->dev)
				ચાલુ;

			જો (strcmp(bus->phymap[i]->dev->name, ethname) == 0)
				પાછા bus->phymap[i];
		}
	}

	printf("%s is not a known ethernet\n", ethname);
	પાછા ખાલી;
}

અચલ અક્ષર *miiphy_get_current_dev(શૂન્ય)
{
	જો (current_mii)
		પાછા current_mii->name;

	પાછા ખાલી;
}

સ્થિર સંરચના mii_dev *miiphy_get_active_dev(અચલ અક્ષર *devname)
{
	/* If the current mii is the one we want, return it */
	જો (current_mii)
		જો (strcmp(current_mii->name, devname) == 0)
			પાછા current_mii;

	/* Otherwise, set the active one to the one we want */
	જો (miiphy_set_current_dev(devname))
		પાછા ખાલી;
	નહીંતર
		પાછા current_mii;
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
પૂર્ણાંક miiphy_read(અચલ અક્ષર *devname, unsigned અક્ષર addr, unsigned અક્ષર reg,
		 unsigned short *કિંમત)
{
	સંરચના mii_dev *bus;
	પૂર્ણાંક ret;

	bus = miiphy_get_active_dev(devname);
	જો (!bus)
		પાછા 1;

	ret = bus->પઠ(bus, addr, MDIO_DEVAD_NONE, reg);
	જો (ret < 0)
		પાછા 1;

	*કિંમત = (unsigned short)ret;
	પાછા 0;
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
પૂર્ણાંક miiphy_write(અચલ અક્ષર *devname, unsigned અક્ષર addr, unsigned અક્ષર reg,
		  unsigned short કિંમત)
{
	સંરચના mii_dev *bus;

	bus = miiphy_get_active_dev(devname);
	જો (bus)
		પાછા bus->લિખ(bus, addr, MDIO_DEVAD_NONE, reg, કિંમત);

	પાછા 1;
}

/*****************************************************************************
 *
 * Print out list of registered MII capable devices.
 */
શૂન્ય miiphy_listdev(શૂન્ય)
{
	સંરચના list_head *entry;
	સંરચના mii_dev *dev;

	puts("MII devices: ");
	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, સંરચના mii_dev, link);
		printf("'%s' ", dev->name);
	}
	puts("\n");

	જો (current_mii)
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
પૂર્ણાંક miiphy_info(અચલ અક્ષર *devname, unsigned અક્ષર addr, unsigned પૂર્ણાંક *oui,
		 unsigned અક્ષર *model, unsigned અક્ષર *rev)
{
	unsigned પૂર્ણાંક reg = 0;
	unsigned short tmp;

	જો (miiphy_read(devname, addr, MII_PHYSID2, &tmp) != 0) {
		debug("PHY ID register 2 read failed\n");
		પાછા -1;
	}
	reg = tmp;

	debug("MII_PHYSID2 @ 0x%x = 0x%04x\n", addr, reg);

	જો (reg == 0xFFFF) {
		/* No physical device present at this address */
		પાછા -1;
	}

	જો (miiphy_read(devname, addr, MII_PHYSID1, &tmp) != 0) {
		debug("PHY ID register 1 read failed\n");
		પાછા -1;
	}
	reg |= tmp << 16;
	debug("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (unsigned અક્ષર)((reg >> 4) & 0x0000003F);
	*rev = (unsigned અક્ષર)(reg & 0x0000000F);
	પાછા 0;
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
પૂર્ણાંક miiphy_reset(અચલ અક્ષર *devname, unsigned અક્ષર addr)
{
	unsigned short reg;
	પૂર્ણાંક timeout = 500;

	જો (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
		debug("PHY status read failed\n");
		પાછા -1;
	}
	જો (miiphy_write(devname, addr, MII_BMCR, reg | BMCR_RESET) != 0) {
		debug("PHY reset failed\n");
		પાછા -1;
	}
#જો CONFIG_PHY_RESET_DELAY > 0
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#endif
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = 0x8000;
	જ્યારે (((reg & 0x8000) != 0) && timeout--) {
		જો (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			પાછા -1;
		}
		udelay(1000);
	}
	જો ((reg & 0x8000) == 0) {
		પાછા 0;
	} નહીંતર {
		puts("PHY reset timed out\n");
		પાછા -1;
	}
	પાછા 0;
}
#endif /* !PHYLIB */

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
પૂર્ણાંક miiphy_speed(અચલ અક્ષર *devname, unsigned અક્ષર addr)
{
	u16 bmcr, anlpar, adv;

#જો defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	જો (miiphy_is_1000base_x(devname, addr))
		પાછા _1000BASET;

	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	જો (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		જાઓ miiphy_read_failed;
	}
	જો (btsr != 0xFFFF &&
			(btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)))
		પાછા _1000BASET;
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	જો (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		printf("PHY speed");
		જાઓ miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	જો (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		જો (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("PHY AN speed");
			જાઓ miiphy_read_failed;
		}

		જો (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv speed");
			જાઓ miiphy_read_failed;
		}
		પાછા ((anlpar & adv) & LPA_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	પાછા (bmcr & BMCR_SPEED100) ? _100BASET : _10BASET;

miiphy_read_failed:
	printf(" read failed, assuming 10BASE-T\n");
	પાછા _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
પૂર્ણાંક miiphy_duplex(અચલ અક્ષર *devname, unsigned અક્ષર addr)
{
	u16 bmcr, anlpar, adv;

#જો defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	જો (miiphy_is_1000base_x(devname, addr)) {
		/* 1000BASE-X */
		જો (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("1000BASE-X PHY AN duplex");
			જાઓ miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	જો (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		જાઓ miiphy_read_failed;
	}
	જો (btsr != 0xFFFF) {
		જો (btsr & PHY_1000BTSR_1000FD) {
			પાછા FULL;
		} નહીંતર જો (btsr & PHY_1000BTSR_1000HD) {
			પાછા HALF;
		}
	}
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	જો (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		puts("PHY duplex");
		જાઓ miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	જો (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		જો (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			puts("PHY AN duplex");
			જાઓ miiphy_read_failed;
		}

		જો (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv duplex");
			જાઓ miiphy_read_failed;
		}
		પાછા ((anlpar & adv) & (LPA_10FULL | LPA_100FULL)) ?
		    FULL : HALF;
	}
	/* Get speed from basic control settings. */
	પાછા (bmcr & BMCR_FULLDPLX) ? FULL : HALF;

miiphy_read_failed:
	printf(" read failed, assuming half duplex\n");
	પાછા HALF;
}

/*****************************************************************************
 *
 * Return 1 if PHY supports 1000BASE-X, 0 if PHY supports 10BASE-T/100BASE-TX/
 * 1000BASE-T, or on error.
 */
પૂર્ણાંક miiphy_is_1000base_x(અચલ અક્ષર *devname, unsigned અક્ષર addr)
{
#જો defined(CONFIG_PHY_GIGE)
	u16 exsr;

	જો (miiphy_read(devname, addr, MII_ESTATUS, &exsr)) {
		printf("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		પાછા 0;
	}
	પાછા 0 != (exsr & (ESTATUS_1000XF | ESTATUS_1000XH));
#નહીંતર
	પાછા 0;
#endif
}
