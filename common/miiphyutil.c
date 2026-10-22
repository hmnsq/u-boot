// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 */

/*
 * This provides a bit-banged interface to the ethernet MII management
 * channel.
 */

#ଅନ୍ତର୍ଭାଵ <dm.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <miiphy.h>
#ଅନ୍ତର୍ଭାଵ <phy.h>
#ଅନ୍ତର୍ଭାଵ <linux/delay.h>

#ଅନ୍ତର୍ଭାଵ <asm/types.h>
#ଅନ୍ତର୍ଭାଵ <linux/list.h>
#ଅନ୍ତର୍ଭାଵ <malloc.h>
#ଅନ୍ତର୍ଭାଵ <net.h>

/* local debug macro */
#undef MII_DEBUG

#undef debug
#ifdef MII_DEBUG
#define debug(fmt, args...)	printf(fmt, ##args)
#ଅନ୍ୟଥା
#define debug(fmt, args...)
#endif /* MII_DEBUG */

ସ୍ଥିର LIST_HEAD(mii_devs);
ସ୍ଥିର ସଂରଚନା mii_dev *current_mii;

/*
 * Lookup the mii_dev struct by the registered device name.
 */
ସଂରଚନା mii_dev *miiphy_get_dev_by_name(ଅଚଳ ଅକ୍ଷର *devname)
{
	ସଂରଚନା list_head *entry;
	ସଂରଚନା mii_dev *dev;

	ଯଦି (!devname) {
		printf("NULL device name!\n");
		ଫେରନ୍ତୁ ଖାଲି;
	}

	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, ସଂରଚନା mii_dev, link);
		ଯଦି (strcmp(dev->name, devname) == 0)
			ଫେରନ୍ତୁ dev;
	}

	ଫେରନ୍ତୁ ଖାଲି;
}

ସଂରଚନା mii_dev *mdio_alloc(ଶୂନ୍ୟ)
{
	ସଂରଚନା mii_dev *bus;

	bus = malloc(ଆକାର(*bus));
	ଯଦି (!bus)
		ଫେରନ୍ତୁ bus;

	memset(bus, 0, ଆକାର(*bus));

	/* initialize mii_dev struct fields */
	INIT_LIST_HEAD(&bus->link);

	ଫେରନ୍ତୁ bus;
}

ଶୂନ୍ୟ mdio_free(ସଂରଚନା mii_dev *bus)
{
	ମୁକ୍ତ(bus);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା mdio_register(ସଂରଚନା mii_dev *bus)
{
	ଯଦି (!bus || !bus->ପଠ || !bus->ଲିଖ)
		ଫେରନ୍ତୁ -1;

	/* check if we have unique name */
	ଯଦି (miiphy_get_dev_by_name(bus->name)) {
		printf("mdio_register: non unique device name '%s'\n",
			bus->name);
		ଫେରନ୍ତୁ -1;
	}

	/* add it to the list */
	list_add_tail(&bus->link, &mii_devs);

	ଯଦି (!current_mii)
		current_mii = bus;

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା mdio_register_seq(ସଂରଚନା mii_dev *bus, ପୂର୍ଣ୍ଣସଂଖ୍ୟା seq)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	/* Setup a unique name for each mdio bus */
	ret = snprintf(bus->name, MDIO_NAME_LEN, "eth%d", seq);
	ଯଦି (ret < 0)
		ଫେରନ୍ତୁ ret;

	ଫେରନ୍ତୁ mdio_register(bus);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା mdio_unregister(ସଂରଚନା mii_dev *bus)
{
	ଯଦି (!bus)
		ଫେରନ୍ତୁ 0;

	/* delete it from the list */
	list_del(&bus->link);

	ଯଦି (current_mii == bus)
		current_mii = ଖାଲି;

	ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ mdio_list_devices(ଶୂନ୍ୟ)
{
	ସଂରଚନା list_head *entry;

	list_for_each(entry, &mii_devs) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
		ସଂରଚନା mii_dev *bus = list_entry(entry, ସଂରଚନା mii_dev, link);

		printf("%s:\n", bus->name);

		ପାଇଁ (i = 0; i < PHY_MAX_ADDR; i++) {
			ସଂରଚନା phy_device *phydev = bus->phymap[i];

			ଯଦି (phydev) {
				printf("%x - %s", i, phydev->drv->name);

				ଯଦି (phydev->dev)
					printf(" <--> %s\n", phydev->dev->name);
				ଅନ୍ୟଥା
					printf("\n");
			}
		}
	}
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା miiphy_set_current_dev(ଅଚଳ ଅକ୍ଷର *devname)
{
	ସଂରଚନା mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname);
	ଯଦି (dev) {
		current_mii = dev;
		ଫେରନ୍ତୁ 0;
	}

	printf("No such device: %s\n", devname);

	ଫେରନ୍ତୁ 1;
}

ସଂରଚନା mii_dev *mdio_get_current_dev(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ current_mii;
}

ସଂରଚନା list_head *mdio_get_list_head(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ &mii_devs;
}

ସଂରଚନା phy_device *mdio_phydev_for_ethname(ଅଚଳ ଅକ୍ଷର *ethname)
{
	ସଂରଚନା list_head *entry;
	ସଂରଚନା mii_dev *bus;

	list_for_each(entry, &mii_devs) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
		bus = list_entry(entry, ସଂରଚନା mii_dev, link);

		ପାଇଁ (i = 0; i < PHY_MAX_ADDR; i++) {
			ଯଦି (!bus->phymap[i] || !bus->phymap[i]->dev)
				ଜାରି;

			ଯଦି (strcmp(bus->phymap[i]->dev->name, ethname) == 0)
				ଫେରନ୍ତୁ bus->phymap[i];
		}
	}

	printf("%s is not a known ethernet\n", ethname);
	ଫେରନ୍ତୁ ଖାଲି;
}

ଅଚଳ ଅକ୍ଷର *miiphy_get_current_dev(ଶୂନ୍ୟ)
{
	ଯଦି (current_mii)
		ଫେରନ୍ତୁ current_mii->name;

	ଫେରନ୍ତୁ ଖାଲି;
}

ସ୍ଥିର ସଂରଚନା mii_dev *miiphy_get_active_dev(ଅଚଳ ଅକ୍ଷର *devname)
{
	/* If the current mii is the one we want, return it */
	ଯଦି (current_mii)
		ଯଦି (strcmp(current_mii->name, devname) == 0)
			ଫେରନ୍ତୁ current_mii;

	/* Otherwise, set the active one to the one we want */
	ଯଦି (miiphy_set_current_dev(devname))
		ଫେରନ୍ତୁ ଖାଲି;
	ଅନ୍ୟଥା
		ଫେରନ୍ତୁ current_mii;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା miiphy_read(ଅଚଳ ଅକ୍ଷର *devname, unsigned ଅକ୍ଷର addr, unsigned ଅକ୍ଷର reg,
		 unsigned short *ମୂଲ୍ୟ)
{
	ସଂରଚନା mii_dev *bus;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	bus = miiphy_get_active_dev(devname);
	ଯଦି (!bus)
		ଫେରନ୍ତୁ 1;

	ret = bus->ପଠ(bus, addr, MDIO_DEVAD_NONE, reg);
	ଯଦି (ret < 0)
		ଫେରନ୍ତୁ 1;

	*ମୂଲ୍ୟ = (unsigned short)ret;
	ଫେରନ୍ତୁ 0;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା miiphy_write(ଅଚଳ ଅକ୍ଷର *devname, unsigned ଅକ୍ଷର addr, unsigned ଅକ୍ଷର reg,
		  unsigned short ମୂଲ୍ୟ)
{
	ସଂରଚନା mii_dev *bus;

	bus = miiphy_get_active_dev(devname);
	ଯଦି (bus)
		ଫେରନ୍ତୁ bus->ଲିଖ(bus, addr, MDIO_DEVAD_NONE, reg, ମୂଲ୍ୟ);

	ଫେରନ୍ତୁ 1;
}

/*****************************************************************************
 *
 * Print out list of registered MII capable devices.
 */
ଶୂନ୍ୟ miiphy_listdev(ଶୂନ୍ୟ)
{
	ସଂରଚନା list_head *entry;
	ସଂରଚନା mii_dev *dev;

	puts("MII devices: ");
	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, ସଂରଚନା mii_dev, link);
		printf("'%s' ", dev->name);
	}
	puts("\n");

	ଯଦି (current_mii)
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା miiphy_info(ଅଚଳ ଅକ୍ଷର *devname, unsigned ଅକ୍ଷର addr, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *oui,
		 unsigned ଅକ୍ଷର *model, unsigned ଅକ୍ଷର *rev)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା reg = 0;
	unsigned short tmp;

	ଯଦି (miiphy_read(devname, addr, MII_PHYSID2, &tmp) != 0) {
		debug("PHY ID register 2 read failed\n");
		ଫେରନ୍ତୁ -1;
	}
	reg = tmp;

	debug("MII_PHYSID2 @ 0x%x = 0x%04x\n", addr, reg);

	ଯଦି (reg == 0xFFFF) {
		/* No physical device present at this address */
		ଫେରନ୍ତୁ -1;
	}

	ଯଦି (miiphy_read(devname, addr, MII_PHYSID1, &tmp) != 0) {
		debug("PHY ID register 1 read failed\n");
		ଫେରନ୍ତୁ -1;
	}
	reg |= tmp << 16;
	debug("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (unsigned ଅକ୍ଷର)((reg >> 4) & 0x0000003F);
	*rev = (unsigned ଅକ୍ଷର)(reg & 0x0000000F);
	ଫେରନ୍ତୁ 0;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା miiphy_reset(ଅଚଳ ଅକ୍ଷର *devname, unsigned ଅକ୍ଷର addr)
{
	unsigned short reg;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା timeout = 500;

	ଯଦି (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
		debug("PHY status read failed\n");
		ଫେରନ୍ତୁ -1;
	}
	ଯଦି (miiphy_write(devname, addr, MII_BMCR, reg | BMCR_RESET) != 0) {
		debug("PHY reset failed\n");
		ଫେରନ୍ତୁ -1;
	}
#ଯଦି CONFIG_PHY_RESET_DELAY > 0
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#endif
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = 0x8000;
	ଯେତେବେଳେ (((reg & 0x8000) != 0) && timeout--) {
		ଯଦି (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			ଫେରନ୍ତୁ -1;
		}
		udelay(1000);
	}
	ଯଦି ((reg & 0x8000) == 0) {
		ଫେରନ୍ତୁ 0;
	} ଅନ୍ୟଥା {
		puts("PHY reset timed out\n");
		ଫେରନ୍ତୁ -1;
	}
	ଫେରନ୍ତୁ 0;
}
#endif /* !PHYLIB */

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା miiphy_speed(ଅଚଳ ଅକ୍ଷର *devname, unsigned ଅକ୍ଷର addr)
{
	u16 bmcr, anlpar, adv;

#ଯଦି defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	ଯଦି (miiphy_is_1000base_x(devname, addr))
		ଫେରନ୍ତୁ _1000BASET;

	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	ଯଦି (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		ଯାଅ miiphy_read_failed;
	}
	ଯଦି (btsr != 0xFFFF &&
			(btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)))
		ଫେରନ୍ତୁ _1000BASET;
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	ଯଦି (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		printf("PHY speed");
		ଯାଅ miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	ଯଦି (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		ଯଦି (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("PHY AN speed");
			ଯାଅ miiphy_read_failed;
		}

		ଯଦି (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv speed");
			ଯାଅ miiphy_read_failed;
		}
		ଫେରନ୍ତୁ ((anlpar & adv) & LPA_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	ଫେରନ୍ତୁ (bmcr & BMCR_SPEED100) ? _100BASET : _10BASET;

miiphy_read_failed:
	printf(" read failed, assuming 10BASE-T\n");
	ଫେରନ୍ତୁ _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା miiphy_duplex(ଅଚଳ ଅକ୍ଷର *devname, unsigned ଅକ୍ଷର addr)
{
	u16 bmcr, anlpar, adv;

#ଯଦି defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	ଯଦି (miiphy_is_1000base_x(devname, addr)) {
		/* 1000BASE-X */
		ଯଦି (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("1000BASE-X PHY AN duplex");
			ଯାଅ miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	ଯଦି (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		ଯାଅ miiphy_read_failed;
	}
	ଯଦି (btsr != 0xFFFF) {
		ଯଦି (btsr & PHY_1000BTSR_1000FD) {
			ଫେରନ୍ତୁ FULL;
		} ଅନ୍ୟଥା ଯଦି (btsr & PHY_1000BTSR_1000HD) {
			ଫେରନ୍ତୁ HALF;
		}
	}
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	ଯଦି (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		puts("PHY duplex");
		ଯାଅ miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	ଯଦି (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		ଯଦି (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			puts("PHY AN duplex");
			ଯାଅ miiphy_read_failed;
		}

		ଯଦି (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv duplex");
			ଯାଅ miiphy_read_failed;
		}
		ଫେରନ୍ତୁ ((anlpar & adv) & (LPA_10FULL | LPA_100FULL)) ?
		    FULL : HALF;
	}
	/* Get speed from basic control settings. */
	ଫେରନ୍ତୁ (bmcr & BMCR_FULLDPLX) ? FULL : HALF;

miiphy_read_failed:
	printf(" read failed, assuming half duplex\n");
	ଫେରନ୍ତୁ HALF;
}

/*****************************************************************************
 *
 * Return 1 if PHY supports 1000BASE-X, 0 if PHY supports 10BASE-T/100BASE-TX/
 * 1000BASE-T, or on error.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା miiphy_is_1000base_x(ଅଚଳ ଅକ୍ଷର *devname, unsigned ଅକ୍ଷର addr)
{
#ଯଦି defined(CONFIG_PHY_GIGE)
	u16 exsr;

	ଯଦି (miiphy_read(devname, addr, MII_ESTATUS, &exsr)) {
		printf("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		ଫେରନ୍ତୁ 0;
	}
	ଫେରନ୍ତୁ 0 != (exsr & (ESTATUS_1000XF | ESTATUS_1000XH));
#ଅନ୍ୟଥା
	ଫେରନ୍ତୁ 0;
#endif
}
