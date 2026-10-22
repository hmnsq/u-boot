// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 */

/*
 * This provides a bit-banged interface to the ethernet MII management
 * channel.
 */

#ضمن <dm.h>
#ضمن <log.h>
#ضمن <miiphy.h>
#ضمن <phy.h>
#ضمن <linux/delay.h>

#ضمن <asm/types.h>
#ضمن <linux/list.h>
#ضمن <malloc.h>
#ضمن <net.h>

/* local debug macro */
#undef MII_DEBUG

#undef debug
#ifdef MII_DEBUG
#define debug(fmt, args...)	printf(fmt, ##args)
#אחרת
#define debug(fmt, args...)
#endif /* MII_DEBUG */

ساكن LIST_HEAD(mii_devs);
ساكن بنية mii_dev *current_mii;

/*
 * Lookup the mii_dev struct by the registered device name.
 */
بنية mii_dev *miiphy_get_dev_by_name(ثابت حرف *devname)
{
	بنية list_head *entry;
	بنية mii_dev *dev;

	אם (!devname) {
		printf("NULL device name!\n");
		החזר عدم;
	}

	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, بنية mii_dev, link);
		אם (strcmp(dev->name, devname) == 0)
			החזר dev;
	}

	החזר عدم;
}

بنية mii_dev *mdio_alloc(فراغ)
{
	بنية mii_dev *bus;

	bus = malloc(حجم(*bus));
	אם (!bus)
		החזר bus;

	memset(bus, 0, حجم(*bus));

	/* initialize mii_dev struct fields */
	INIT_LIST_HEAD(&bus->link);

	החזר bus;
}

فراغ mdio_free(بنية mii_dev *bus)
{
	حرر(bus);
}

صحيح mdio_register(بنية mii_dev *bus)
{
	אם (!bus || !bus->اقرأ || !bus->اكتب)
		החזר -1;

	/* check if we have unique name */
	אם (miiphy_get_dev_by_name(bus->name)) {
		printf("mdio_register: non unique device name '%s'\n",
			bus->name);
		החזר -1;
	}

	/* add it to the list */
	list_add_tail(&bus->link, &mii_devs);

	אם (!current_mii)
		current_mii = bus;

	החזר 0;
}

صحيح mdio_register_seq(بنية mii_dev *bus, صحيح seq)
{
	صحيح ret;

	/* Setup a unique name for each mdio bus */
	ret = snprintf(bus->name, MDIO_NAME_LEN, "eth%d", seq);
	אם (ret < 0)
		החזר ret;

	החזר mdio_register(bus);
}

صحيح mdio_unregister(بنية mii_dev *bus)
{
	אם (!bus)
		החזר 0;

	/* delete it from the list */
	list_del(&bus->link);

	אם (current_mii == bus)
		current_mii = عدم;

	החזר 0;
}

فراغ mdio_list_devices(فراغ)
{
	بنية list_head *entry;

	list_for_each(entry, &mii_devs) {
		صحيح i;
		بنية mii_dev *bus = list_entry(entry, بنية mii_dev, link);

		printf("%s:\n", bus->name);

		עבור (i = 0; i < PHY_MAX_ADDR; i++) {
			بنية phy_device *phydev = bus->phymap[i];

			אם (phydev) {
				printf("%x - %s", i, phydev->drv->name);

				אם (phydev->dev)
					printf(" <--> %s\n", phydev->dev->name);
				אחרת
					printf("\n");
			}
		}
	}
}

صحيح miiphy_set_current_dev(ثابت حرف *devname)
{
	بنية mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname);
	אם (dev) {
		current_mii = dev;
		החזר 0;
	}

	printf("No such device: %s\n", devname);

	החזר 1;
}

بنية mii_dev *mdio_get_current_dev(فراغ)
{
	החזר current_mii;
}

بنية list_head *mdio_get_list_head(فراغ)
{
	החזר &mii_devs;
}

بنية phy_device *mdio_phydev_for_ethname(ثابت حرف *ethname)
{
	بنية list_head *entry;
	بنية mii_dev *bus;

	list_for_each(entry, &mii_devs) {
		صحيح i;
		bus = list_entry(entry, بنية mii_dev, link);

		עבור (i = 0; i < PHY_MAX_ADDR; i++) {
			אם (!bus->phymap[i] || !bus->phymap[i]->dev)
				استمر;

			אם (strcmp(bus->phymap[i]->dev->name, ethname) == 0)
				החזר bus->phymap[i];
		}
	}

	printf("%s is not a known ethernet\n", ethname);
	החזר عدم;
}

ثابت حرف *miiphy_get_current_dev(فراغ)
{
	אם (current_mii)
		החזר current_mii->name;

	החזר عدم;
}

ساكن بنية mii_dev *miiphy_get_active_dev(ثابت حرف *devname)
{
	/* If the current mii is the one we want, return it */
	אם (current_mii)
		אם (strcmp(current_mii->name, devname) == 0)
			החזר current_mii;

	/* Otherwise, set the active one to the one we want */
	אם (miiphy_set_current_dev(devname))
		החזר عدم;
	אחרת
		החזר current_mii;
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
صحيح miiphy_read(ثابت حرف *devname, unsigned حرف addr, unsigned حرف reg,
		 unsigned short *value)
{
	بنية mii_dev *bus;
	صحيح ret;

	bus = miiphy_get_active_dev(devname);
	אם (!bus)
		החזר 1;

	ret = bus->اقرأ(bus, addr, MDIO_DEVAD_NONE, reg);
	אם (ret < 0)
		החזר 1;

	*value = (unsigned short)ret;
	החזר 0;
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
صحيح miiphy_write(ثابت حرف *devname, unsigned حرف addr, unsigned حرف reg,
		  unsigned short value)
{
	بنية mii_dev *bus;

	bus = miiphy_get_active_dev(devname);
	אם (bus)
		החזר bus->اكتب(bus, addr, MDIO_DEVAD_NONE, reg, value);

	החזר 1;
}

/*****************************************************************************
 *
 * Print out list of registered MII capable devices.
 */
فراغ miiphy_listdev(فراغ)
{
	بنية list_head *entry;
	بنية mii_dev *dev;

	puts("MII devices: ");
	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, بنية mii_dev, link);
		printf("'%s' ", dev->name);
	}
	puts("\n");

	אם (current_mii)
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
صحيح miiphy_info(ثابت حرف *devname, unsigned حرف addr, unsigned صحيح *oui,
		 unsigned حرف *model, unsigned حرف *rev)
{
	unsigned صحيح reg = 0;
	unsigned short tmp;

	אם (miiphy_read(devname, addr, MII_PHYSID2, &tmp) != 0) {
		debug("PHY ID register 2 read failed\n");
		החזר -1;
	}
	reg = tmp;

	debug("MII_PHYSID2 @ 0x%x = 0x%04x\n", addr, reg);

	אם (reg == 0xFFFF) {
		/* No physical device present at this address */
		החזר -1;
	}

	אם (miiphy_read(devname, addr, MII_PHYSID1, &tmp) != 0) {
		debug("PHY ID register 1 read failed\n");
		החזר -1;
	}
	reg |= tmp << 16;
	debug("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (unsigned حرف)((reg >> 4) & 0x0000003F);
	*rev = (unsigned حرف)(reg & 0x0000000F);
	החזר 0;
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
صحيح miiphy_reset(ثابت حرف *devname, unsigned حرف addr)
{
	unsigned short reg;
	صحيح timeout = 500;

	אם (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
		debug("PHY status read failed\n");
		החזר -1;
	}
	אם (miiphy_write(devname, addr, MII_BMCR, reg | BMCR_RESET) != 0) {
		debug("PHY reset failed\n");
		החזר -1;
	}
#אם CONFIG_PHY_RESET_DELAY > 0
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#endif
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = 0x8000;
	כלעוד (((reg & 0x8000) != 0) && timeout--) {
		אם (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			החזר -1;
		}
		udelay(1000);
	}
	אם ((reg & 0x8000) == 0) {
		החזר 0;
	} אחרת {
		puts("PHY reset timed out\n");
		החזר -1;
	}
	החזר 0;
}
#endif /* !PHYLIB */

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
صحيح miiphy_speed(ثابت حرف *devname, unsigned حرف addr)
{
	u16 bmcr, anlpar, adv;

#אם defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	אם (miiphy_is_1000base_x(devname, addr))
		החזר _1000BASET;

	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	אם (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		اذهب miiphy_read_failed;
	}
	אם (btsr != 0xFFFF &&
			(btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)))
		החזר _1000BASET;
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	אם (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		printf("PHY speed");
		اذهب miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	אם (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		אם (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("PHY AN speed");
			اذهب miiphy_read_failed;
		}

		אם (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv speed");
			اذهب miiphy_read_failed;
		}
		החזר ((anlpar & adv) & LPA_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	החזר (bmcr & BMCR_SPEED100) ? _100BASET : _10BASET;

miiphy_read_failed:
	printf(" read failed, assuming 10BASE-T\n");
	החזר _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
صحيح miiphy_duplex(ثابت حرف *devname, unsigned حرف addr)
{
	u16 bmcr, anlpar, adv;

#אם defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	אם (miiphy_is_1000base_x(devname, addr)) {
		/* 1000BASE-X */
		אם (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("1000BASE-X PHY AN duplex");
			اذهب miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	אם (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		اذهب miiphy_read_failed;
	}
	אם (btsr != 0xFFFF) {
		אם (btsr & PHY_1000BTSR_1000FD) {
			החזר FULL;
		} אחרת אם (btsr & PHY_1000BTSR_1000HD) {
			החזר HALF;
		}
	}
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	אם (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		puts("PHY duplex");
		اذهب miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	אם (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		אם (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			puts("PHY AN duplex");
			اذهب miiphy_read_failed;
		}

		אם (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv duplex");
			اذهب miiphy_read_failed;
		}
		החזר ((anlpar & adv) & (LPA_10FULL | LPA_100FULL)) ?
		    FULL : HALF;
	}
	/* Get speed from basic control settings. */
	החזר (bmcr & BMCR_FULLDPLX) ? FULL : HALF;

miiphy_read_failed:
	printf(" read failed, assuming half duplex\n");
	החזר HALF;
}

/*****************************************************************************
 *
 * Return 1 if PHY supports 1000BASE-X, 0 if PHY supports 10BASE-T/100BASE-TX/
 * 1000BASE-T, or on error.
 */
صحيح miiphy_is_1000base_x(ثابت حرف *devname, unsigned حرف addr)
{
#אם defined(CONFIG_PHY_GIGE)
	u16 exsr;

	אם (miiphy_read(devname, addr, MII_ESTATUS, &exsr)) {
		printf("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		החזר 0;
	}
	החזר 0 != (exsr & (ESTATUS_1000XF | ESTATUS_1000XH));
#אחרת
	החזר 0;
#endif
}
