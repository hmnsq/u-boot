// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 */

/*
 * This provides a bit-banged interface to the ethernet MII management
 * channel.
 */

#அந்தர்பாவ <dm.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <miiphy.h>
#அந்தர்பாவ <phy.h>
#அந்தர்பாவ <linux/delay.h>

#அந்தர்பாவ <asm/types.h>
#அந்தர்பாவ <linux/list.h>
#அந்தர்பாவ <malloc.h>
#அந்தர்பாவ <net.h>

/* local debug macro */
#undef MII_DEBUG

#undef debug
#ifdef MII_DEBUG
#define debug(fmt, args...)	printf(fmt, ##args)
#இல்லையெனில்
#define debug(fmt, args...)
#endif /* MII_DEBUG */

நிலையான LIST_HEAD(mii_devs);
நிலையான கட்டமைப்பு mii_dev *current_mii;

/*
 * Lookup the mii_dev struct by the registered device name.
 */
கட்டமைப்பு mii_dev *miiphy_get_dev_by_name(மாறிலி எழுத்து *devname)
{
	கட்டமைப்பு list_head *entry;
	கட்டமைப்பு mii_dev *dev;

	எனில் (!devname) {
		printf("NULL device name!\n");
		திரும்பு வெற்று;
	}

	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, கட்டமைப்பு mii_dev, link);
		எனில் (strcmp(dev->name, devname) == 0)
			திரும்பு dev;
	}

	திரும்பு வெற்று;
}

கட்டமைப்பு mii_dev *mdio_alloc(வெறுமை)
{
	கட்டமைப்பு mii_dev *bus;

	bus = malloc(அளவு(*bus));
	எனில் (!bus)
		திரும்பு bus;

	memset(bus, 0, அளவு(*bus));

	/* initialize mii_dev struct fields */
	INIT_LIST_HEAD(&bus->link);

	திரும்பு bus;
}

வெறுமை mdio_free(கட்டமைப்பு mii_dev *bus)
{
	விடுவி(bus);
}

முழு mdio_register(கட்டமைப்பு mii_dev *bus)
{
	எனில் (!bus || !bus->பட || !bus->லிக)
		திரும்பு -1;

	/* check if we have unique name */
	எனில் (miiphy_get_dev_by_name(bus->name)) {
		printf("mdio_register: non unique device name '%s'\n",
			bus->name);
		திரும்பு -1;
	}

	/* add it to the list */
	list_add_tail(&bus->link, &mii_devs);

	எனில் (!current_mii)
		current_mii = bus;

	திரும்பு 0;
}

முழு mdio_register_seq(கட்டமைப்பு mii_dev *bus, முழு seq)
{
	முழு ret;

	/* Setup a unique name for each mdio bus */
	ret = snprintf(bus->name, MDIO_NAME_LEN, "eth%d", seq);
	எனில் (ret < 0)
		திரும்பு ret;

	திரும்பு mdio_register(bus);
}

முழு mdio_unregister(கட்டமைப்பு mii_dev *bus)
{
	எனில் (!bus)
		திரும்பு 0;

	/* delete it from the list */
	list_del(&bus->link);

	எனில் (current_mii == bus)
		current_mii = வெற்று;

	திரும்பு 0;
}

வெறுமை mdio_list_devices(வெறுமை)
{
	கட்டமைப்பு list_head *entry;

	list_for_each(entry, &mii_devs) {
		முழு i;
		கட்டமைப்பு mii_dev *bus = list_entry(entry, கட்டமைப்பு mii_dev, link);

		printf("%s:\n", bus->name);

		க்கு (i = 0; i < PHY_MAX_ADDR; i++) {
			கட்டமைப்பு phy_device *phydev = bus->phymap[i];

			எனில் (phydev) {
				printf("%x - %s", i, phydev->drv->name);

				எனில் (phydev->dev)
					printf(" <--> %s\n", phydev->dev->name);
				இல்லையெனில்
					printf("\n");
			}
		}
	}
}

முழு miiphy_set_current_dev(மாறிலி எழுத்து *devname)
{
	கட்டமைப்பு mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname);
	எனில் (dev) {
		current_mii = dev;
		திரும்பு 0;
	}

	printf("No such device: %s\n", devname);

	திரும்பு 1;
}

கட்டமைப்பு mii_dev *mdio_get_current_dev(வெறுமை)
{
	திரும்பு current_mii;
}

கட்டமைப்பு list_head *mdio_get_list_head(வெறுமை)
{
	திரும்பு &mii_devs;
}

கட்டமைப்பு phy_device *mdio_phydev_for_ethname(மாறிலி எழுத்து *ethname)
{
	கட்டமைப்பு list_head *entry;
	கட்டமைப்பு mii_dev *bus;

	list_for_each(entry, &mii_devs) {
		முழு i;
		bus = list_entry(entry, கட்டமைப்பு mii_dev, link);

		க்கு (i = 0; i < PHY_MAX_ADDR; i++) {
			எனில் (!bus->phymap[i] || !bus->phymap[i]->dev)
				தொடர்;

			எனில் (strcmp(bus->phymap[i]->dev->name, ethname) == 0)
				திரும்பு bus->phymap[i];
		}
	}

	printf("%s is not a known ethernet\n", ethname);
	திரும்பு வெற்று;
}

மாறிலி எழுத்து *miiphy_get_current_dev(வெறுமை)
{
	எனில் (current_mii)
		திரும்பு current_mii->name;

	திரும்பு வெற்று;
}

நிலையான கட்டமைப்பு mii_dev *miiphy_get_active_dev(மாறிலி எழுத்து *devname)
{
	/* If the current mii is the one we want, return it */
	எனில் (current_mii)
		எனில் (strcmp(current_mii->name, devname) == 0)
			திரும்பு current_mii;

	/* Otherwise, set the active one to the one we want */
	எனில் (miiphy_set_current_dev(devname))
		திரும்பு வெற்று;
	இல்லையெனில்
		திரும்பு current_mii;
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
முழு miiphy_read(மாறிலி எழுத்து *devname, unsigned எழுத்து addr, unsigned எழுத்து reg,
		 unsigned short *மதிப்பு)
{
	கட்டமைப்பு mii_dev *bus;
	முழு ret;

	bus = miiphy_get_active_dev(devname);
	எனில் (!bus)
		திரும்பு 1;

	ret = bus->பட(bus, addr, MDIO_DEVAD_NONE, reg);
	எனில் (ret < 0)
		திரும்பு 1;

	*மதிப்பு = (unsigned short)ret;
	திரும்பு 0;
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
முழு miiphy_write(மாறிலி எழுத்து *devname, unsigned எழுத்து addr, unsigned எழுத்து reg,
		  unsigned short மதிப்பு)
{
	கட்டமைப்பு mii_dev *bus;

	bus = miiphy_get_active_dev(devname);
	எனில் (bus)
		திரும்பு bus->லிக(bus, addr, MDIO_DEVAD_NONE, reg, மதிப்பு);

	திரும்பு 1;
}

/*****************************************************************************
 *
 * Print out list of registered MII capable devices.
 */
வெறுமை miiphy_listdev(வெறுமை)
{
	கட்டமைப்பு list_head *entry;
	கட்டமைப்பு mii_dev *dev;

	puts("MII devices: ");
	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, கட்டமைப்பு mii_dev, link);
		printf("'%s' ", dev->name);
	}
	puts("\n");

	எனில் (current_mii)
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
முழு miiphy_info(மாறிலி எழுத்து *devname, unsigned எழுத்து addr, unsigned முழு *oui,
		 unsigned எழுத்து *model, unsigned எழுத்து *rev)
{
	unsigned முழு reg = 0;
	unsigned short tmp;

	எனில் (miiphy_read(devname, addr, MII_PHYSID2, &tmp) != 0) {
		debug("PHY ID register 2 read failed\n");
		திரும்பு -1;
	}
	reg = tmp;

	debug("MII_PHYSID2 @ 0x%x = 0x%04x\n", addr, reg);

	எனில் (reg == 0xFFFF) {
		/* No physical device present at this address */
		திரும்பு -1;
	}

	எனில் (miiphy_read(devname, addr, MII_PHYSID1, &tmp) != 0) {
		debug("PHY ID register 1 read failed\n");
		திரும்பு -1;
	}
	reg |= tmp << 16;
	debug("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (unsigned எழுத்து)((reg >> 4) & 0x0000003F);
	*rev = (unsigned எழுத்து)(reg & 0x0000000F);
	திரும்பு 0;
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
முழு miiphy_reset(மாறிலி எழுத்து *devname, unsigned எழுத்து addr)
{
	unsigned short reg;
	முழு timeout = 500;

	எனில் (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
		debug("PHY status read failed\n");
		திரும்பு -1;
	}
	எனில் (miiphy_write(devname, addr, MII_BMCR, reg | BMCR_RESET) != 0) {
		debug("PHY reset failed\n");
		திரும்பு -1;
	}
#எனில் CONFIG_PHY_RESET_DELAY > 0
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#endif
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = 0x8000;
	போது (((reg & 0x8000) != 0) && timeout--) {
		எனில் (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			திரும்பு -1;
		}
		udelay(1000);
	}
	எனில் ((reg & 0x8000) == 0) {
		திரும்பு 0;
	} இல்லையெனில் {
		puts("PHY reset timed out\n");
		திரும்பு -1;
	}
	திரும்பு 0;
}
#endif /* !PHYLIB */

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
முழு miiphy_speed(மாறிலி எழுத்து *devname, unsigned எழுத்து addr)
{
	u16 bmcr, anlpar, adv;

#எனில் defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	எனில் (miiphy_is_1000base_x(devname, addr))
		திரும்பு _1000BASET;

	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	எனில் (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		செல் miiphy_read_failed;
	}
	எனில் (btsr != 0xFFFF &&
			(btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)))
		திரும்பு _1000BASET;
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	எனில் (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		printf("PHY speed");
		செல் miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	எனில் (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		எனில் (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("PHY AN speed");
			செல் miiphy_read_failed;
		}

		எனில் (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv speed");
			செல் miiphy_read_failed;
		}
		திரும்பு ((anlpar & adv) & LPA_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	திரும்பு (bmcr & BMCR_SPEED100) ? _100BASET : _10BASET;

miiphy_read_failed:
	printf(" read failed, assuming 10BASE-T\n");
	திரும்பு _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
முழு miiphy_duplex(மாறிலி எழுத்து *devname, unsigned எழுத்து addr)
{
	u16 bmcr, anlpar, adv;

#எனில் defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	எனில் (miiphy_is_1000base_x(devname, addr)) {
		/* 1000BASE-X */
		எனில் (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("1000BASE-X PHY AN duplex");
			செல் miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	எனில் (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		செல் miiphy_read_failed;
	}
	எனில் (btsr != 0xFFFF) {
		எனில் (btsr & PHY_1000BTSR_1000FD) {
			திரும்பு FULL;
		} இல்லையெனில் எனில் (btsr & PHY_1000BTSR_1000HD) {
			திரும்பு HALF;
		}
	}
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	எனில் (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		puts("PHY duplex");
		செல் miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	எனில் (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		எனில் (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			puts("PHY AN duplex");
			செல் miiphy_read_failed;
		}

		எனில் (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv duplex");
			செல் miiphy_read_failed;
		}
		திரும்பு ((anlpar & adv) & (LPA_10FULL | LPA_100FULL)) ?
		    FULL : HALF;
	}
	/* Get speed from basic control settings. */
	திரும்பு (bmcr & BMCR_FULLDPLX) ? FULL : HALF;

miiphy_read_failed:
	printf(" read failed, assuming half duplex\n");
	திரும்பு HALF;
}

/*****************************************************************************
 *
 * Return 1 if PHY supports 1000BASE-X, 0 if PHY supports 10BASE-T/100BASE-TX/
 * 1000BASE-T, or on error.
 */
முழு miiphy_is_1000base_x(மாறிலி எழுத்து *devname, unsigned எழுத்து addr)
{
#எனில் defined(CONFIG_PHY_GIGE)
	u16 exsr;

	எனில் (miiphy_read(devname, addr, MII_ESTATUS, &exsr)) {
		printf("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		திரும்பு 0;
	}
	திரும்பு 0 != (exsr & (ESTATUS_1000XF | ESTATUS_1000XH));
#இல்லையெனில்
	திரும்பு 0;
#endif
}
