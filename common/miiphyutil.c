// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 */

/*
 * This provides a bit-banged interface to the ethernet MII management
 * channel.
 */

#شامل <dm.h>
#شامل <log.h>
#شامل <miiphy.h>
#شامل <phy.h>
#شامل <linux/delay.h>

#شامل <asm/types.h>
#شامل <linux/list.h>
#شامل <malloc.h>
#شامل <net.h>

/* local debug macro */
#undef MII_DEBUG

#undef debug
#ifdef MII_DEBUG
#define debug(fmt, args...)	printf(fmt, ##args)
#وگرنه
#define debug(fmt, args...)
#endif /* MII_DEBUG */

ایستا LIST_HEAD(mii_devs);
ایستا ساختار mii_dev *current_mii;

/*
 * Lookup the mii_dev struct by the registered device name.
 */
ساختار mii_dev *miiphy_get_dev_by_name(ثابت نویسه *devname)
{
	ساختار list_head *entry;
	ساختار mii_dev *dev;

	اگر (!devname) {
		printf("NULL device name!\n");
		بازگشت پوچ;
	}

	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, ساختار mii_dev, link);
		اگر (strcmp(dev->name, devname) == 0)
			بازگشت dev;
	}

	بازگشت پوچ;
}

ساختار mii_dev *mdio_alloc(تهی)
{
	ساختار mii_dev *bus;

	bus = malloc(اندازه(*bus));
	اگر (!bus)
		بازگشت bus;

	memset(bus, 0, اندازه(*bus));

	/* initialize mii_dev struct fields */
	INIT_LIST_HEAD(&bus->link);

	بازگشت bus;
}

تهی mdio_free(ساختار mii_dev *bus)
{
	آزاد(bus);
}

صحیح mdio_register(ساختار mii_dev *bus)
{
	اگر (!bus || !bus->اقرأ || !bus->اكتب)
		بازگشت -1;

	/* check if we have unique name */
	اگر (miiphy_get_dev_by_name(bus->name)) {
		printf("mdio_register: non unique device name '%s'\n",
			bus->name);
		بازگشت -1;
	}

	/* add it to the list */
	list_add_tail(&bus->link, &mii_devs);

	اگر (!current_mii)
		current_mii = bus;

	بازگشت 0;
}

صحیح mdio_register_seq(ساختار mii_dev *bus, صحیح seq)
{
	صحیح ret;

	/* Setup a unique name for each mdio bus */
	ret = snprintf(bus->name, MDIO_NAME_LEN, "eth%d", seq);
	اگر (ret < 0)
		بازگشت ret;

	بازگشت mdio_register(bus);
}

صحیح mdio_unregister(ساختار mii_dev *bus)
{
	اگر (!bus)
		بازگشت 0;

	/* delete it from the list */
	list_del(&bus->link);

	اگر (current_mii == bus)
		current_mii = پوچ;

	بازگشت 0;
}

تهی mdio_list_devices(تهی)
{
	ساختار list_head *entry;

	list_for_each(entry, &mii_devs) {
		صحیح i;
		ساختار mii_dev *bus = list_entry(entry, ساختار mii_dev, link);

		printf("%s:\n", bus->name);

		برای (i = 0; i < PHY_MAX_ADDR; i++) {
			ساختار phy_device *phydev = bus->phymap[i];

			اگر (phydev) {
				printf("%x - %s", i, phydev->drv->name);

				اگر (phydev->dev)
					printf(" <--> %s\n", phydev->dev->name);
				وگرنه
					printf("\n");
			}
		}
	}
}

صحیح miiphy_set_current_dev(ثابت نویسه *devname)
{
	ساختار mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname);
	اگر (dev) {
		current_mii = dev;
		بازگشت 0;
	}

	printf("No such device: %s\n", devname);

	بازگشت 1;
}

ساختار mii_dev *mdio_get_current_dev(تهی)
{
	بازگشت current_mii;
}

ساختار list_head *mdio_get_list_head(تهی)
{
	بازگشت &mii_devs;
}

ساختار phy_device *mdio_phydev_for_ethname(ثابت نویسه *ethname)
{
	ساختار list_head *entry;
	ساختار mii_dev *bus;

	list_for_each(entry, &mii_devs) {
		صحیح i;
		bus = list_entry(entry, ساختار mii_dev, link);

		برای (i = 0; i < PHY_MAX_ADDR; i++) {
			اگر (!bus->phymap[i] || !bus->phymap[i]->dev)
				ادامه;

			اگر (strcmp(bus->phymap[i]->dev->name, ethname) == 0)
				بازگشت bus->phymap[i];
		}
	}

	printf("%s is not a known ethernet\n", ethname);
	بازگشت پوچ;
}

ثابت نویسه *miiphy_get_current_dev(تهی)
{
	اگر (current_mii)
		بازگشت current_mii->name;

	بازگشت پوچ;
}

ایستا ساختار mii_dev *miiphy_get_active_dev(ثابت نویسه *devname)
{
	/* If the current mii is the one we want, return it */
	اگر (current_mii)
		اگر (strcmp(current_mii->name, devname) == 0)
			بازگشت current_mii;

	/* Otherwise, set the active one to the one we want */
	اگر (miiphy_set_current_dev(devname))
		بازگشت پوچ;
	وگرنه
		بازگشت current_mii;
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
صحیح miiphy_read(ثابت نویسه *devname, unsigned نویسه addr, unsigned نویسه reg,
		 unsigned short *مقدار)
{
	ساختار mii_dev *bus;
	صحیح ret;

	bus = miiphy_get_active_dev(devname);
	اگر (!bus)
		بازگشت 1;

	ret = bus->اقرأ(bus, addr, MDIO_DEVAD_NONE, reg);
	اگر (ret < 0)
		بازگشت 1;

	*مقدار = (unsigned short)ret;
	بازگشت 0;
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
صحیح miiphy_write(ثابت نویسه *devname, unsigned نویسه addr, unsigned نویسه reg,
		  unsigned short مقدار)
{
	ساختار mii_dev *bus;

	bus = miiphy_get_active_dev(devname);
	اگر (bus)
		بازگشت bus->اكتب(bus, addr, MDIO_DEVAD_NONE, reg, مقدار);

	بازگشت 1;
}

/*****************************************************************************
 *
 * Print out list of registered MII capable devices.
 */
تهی miiphy_listdev(تهی)
{
	ساختار list_head *entry;
	ساختار mii_dev *dev;

	puts("MII devices: ");
	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, ساختار mii_dev, link);
		printf("'%s' ", dev->name);
	}
	puts("\n");

	اگر (current_mii)
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
صحیح miiphy_info(ثابت نویسه *devname, unsigned نویسه addr, unsigned صحیح *oui,
		 unsigned نویسه *model, unsigned نویسه *rev)
{
	unsigned صحیح reg = 0;
	unsigned short tmp;

	اگر (miiphy_read(devname, addr, MII_PHYSID2, &tmp) != 0) {
		debug("PHY ID register 2 read failed\n");
		بازگشت -1;
	}
	reg = tmp;

	debug("MII_PHYSID2 @ 0x%x = 0x%04x\n", addr, reg);

	اگر (reg == 0xFFFF) {
		/* No physical device present at this address */
		بازگشت -1;
	}

	اگر (miiphy_read(devname, addr, MII_PHYSID1, &tmp) != 0) {
		debug("PHY ID register 1 read failed\n");
		بازگشت -1;
	}
	reg |= tmp << 16;
	debug("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (unsigned نویسه)((reg >> 4) & 0x0000003F);
	*rev = (unsigned نویسه)(reg & 0x0000000F);
	بازگشت 0;
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
صحیح miiphy_reset(ثابت نویسه *devname, unsigned نویسه addr)
{
	unsigned short reg;
	صحیح timeout = 500;

	اگر (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
		debug("PHY status read failed\n");
		بازگشت -1;
	}
	اگر (miiphy_write(devname, addr, MII_BMCR, reg | BMCR_RESET) != 0) {
		debug("PHY reset failed\n");
		بازگشت -1;
	}
#اگر CONFIG_PHY_RESET_DELAY > 0
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#endif
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = 0x8000;
	وقتی (((reg & 0x8000) != 0) && timeout--) {
		اگر (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			بازگشت -1;
		}
		udelay(1000);
	}
	اگر ((reg & 0x8000) == 0) {
		بازگشت 0;
	} وگرنه {
		puts("PHY reset timed out\n");
		بازگشت -1;
	}
	بازگشت 0;
}
#endif /* !PHYLIB */

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
صحیح miiphy_speed(ثابت نویسه *devname, unsigned نویسه addr)
{
	u16 bmcr, anlpar, adv;

#اگر defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	اگر (miiphy_is_1000base_x(devname, addr))
		بازگشت _1000BASET;

	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	اگر (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		برو miiphy_read_failed;
	}
	اگر (btsr != 0xFFFF &&
			(btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)))
		بازگشت _1000BASET;
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	اگر (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		printf("PHY speed");
		برو miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	اگر (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		اگر (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("PHY AN speed");
			برو miiphy_read_failed;
		}

		اگر (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv speed");
			برو miiphy_read_failed;
		}
		بازگشت ((anlpar & adv) & LPA_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	بازگشت (bmcr & BMCR_SPEED100) ? _100BASET : _10BASET;

miiphy_read_failed:
	printf(" read failed, assuming 10BASE-T\n");
	بازگشت _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
صحیح miiphy_duplex(ثابت نویسه *devname, unsigned نویسه addr)
{
	u16 bmcr, anlpar, adv;

#اگر defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	اگر (miiphy_is_1000base_x(devname, addr)) {
		/* 1000BASE-X */
		اگر (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("1000BASE-X PHY AN duplex");
			برو miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	اگر (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		برو miiphy_read_failed;
	}
	اگر (btsr != 0xFFFF) {
		اگر (btsr & PHY_1000BTSR_1000FD) {
			بازگشت FULL;
		} وگرنه اگر (btsr & PHY_1000BTSR_1000HD) {
			بازگشت HALF;
		}
	}
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	اگر (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		puts("PHY duplex");
		برو miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	اگر (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		اگر (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			puts("PHY AN duplex");
			برو miiphy_read_failed;
		}

		اگر (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv duplex");
			برو miiphy_read_failed;
		}
		بازگشت ((anlpar & adv) & (LPA_10FULL | LPA_100FULL)) ?
		    FULL : HALF;
	}
	/* Get speed from basic control settings. */
	بازگشت (bmcr & BMCR_FULLDPLX) ? FULL : HALF;

miiphy_read_failed:
	printf(" read failed, assuming half duplex\n");
	بازگشت HALF;
}

/*****************************************************************************
 *
 * Return 1 if PHY supports 1000BASE-X, 0 if PHY supports 10BASE-T/100BASE-TX/
 * 1000BASE-T, or on error.
 */
صحیح miiphy_is_1000base_x(ثابت نویسه *devname, unsigned نویسه addr)
{
#اگر defined(CONFIG_PHY_GIGE)
	u16 exsr;

	اگر (miiphy_read(devname, addr, MII_ESTATUS, &exsr)) {
		printf("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		بازگشت 0;
	}
	بازگشت 0 != (exsr & (ESTATUS_1000XF | ESTATUS_1000XH));
#وگرنه
	بازگشت 0;
#endif
}
