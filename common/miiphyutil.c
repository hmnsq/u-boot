// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 */

/*
 * This provides a bit-banged interface to the ethernet MII management
 * channel.
 */

#include <dm.h>
#include <log.h>
#include <miiphy.h>
#include <phy.h>
#include <linux/delay.h>

#include <asm/types.h>
#include <linux/list.h>
#include <malloc.h>
#include <net.h>

/* local debug macro */
#undef MII_DEBUG

#undef debug
#ifdef MII_DEBUG
#define debug(fmt, args...)	printf(fmt, ##args)
#инако
#define debug(fmt, args...)
#endif /* MII_DEBUG */

static LIST_HEAD(mii_devs);
static struct mii_dev *current_mii;

/*
 * Lookup the mii_dev struct by the registered device name.
 */
struct mii_dev *miiphy_get_dev_by_name(const char *devname)
{
	struct list_head *entry;
	struct mii_dev *dev;

	ако (!devname) {
		printf("NULL device name!\n");
		врати NULL;
	}

	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, struct mii_dev, link);
		ако (strcmp(dev->name, devname) == 0)
			врати dev;
	}

	врати NULL;
}

struct mii_dev *mdio_alloc(void)
{
	struct mii_dev *bus;

	bus = malloc(sizeof(*bus));
	ако (!bus)
		врати bus;

	memset(bus, 0, sizeof(*bus));

	/* initialize mii_dev struct fields */
	INIT_LIST_HEAD(&bus->link);

	врати bus;
}

void mdio_free(struct mii_dev *bus)
{
	free(bus);
}

цео mdio_register(struct mii_dev *bus)
{
	ако (!bus || !bus->read || !bus->write)
		врати -1;

	/* check if we have unique name */
	ако (miiphy_get_dev_by_name(bus->name)) {
		printf("mdio_register: non unique device name '%s'\n",
			bus->name);
		врати -1;
	}

	/* add it to the list */
	list_add_tail(&bus->link, &mii_devs);

	ако (!current_mii)
		current_mii = bus;

	врати 0;
}

цео mdio_register_seq(struct mii_dev *bus, цео seq)
{
	цео ret;

	/* Setup a unique name for each mdio bus */
	ret = snprintf(bus->name, MDIO_NAME_LEN, "eth%d", seq);
	ако (ret < 0)
		врати ret;

	врати mdio_register(bus);
}

цео mdio_unregister(struct mii_dev *bus)
{
	ако (!bus)
		врати 0;

	/* delete it from the list */
	list_del(&bus->link);

	ако (current_mii == bus)
		current_mii = NULL;

	врати 0;
}

void mdio_list_devices(void)
{
	struct list_head *entry;

	list_for_each(entry, &mii_devs) {
		цео i;
		struct mii_dev *bus = list_entry(entry, struct mii_dev, link);

		printf("%s:\n", bus->name);

		за (i = 0; i < PHY_MAX_ADDR; i++) {
			struct phy_device *phydev = bus->phymap[i];

			ако (phydev) {
				printf("%x - %s", i, phydev->drv->name);

				ако (phydev->dev)
					printf(" <--> %s\n", phydev->dev->name);
				инако
					printf("\n");
			}
		}
	}
}

цео miiphy_set_current_dev(const char *devname)
{
	struct mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname);
	ако (dev) {
		current_mii = dev;
		врати 0;
	}

	printf("No such device: %s\n", devname);

	врати 1;
}

struct mii_dev *mdio_get_current_dev(void)
{
	врати current_mii;
}

struct list_head *mdio_get_list_head(void)
{
	врати &mii_devs;
}

struct phy_device *mdio_phydev_for_ethname(const char *ethname)
{
	struct list_head *entry;
	struct mii_dev *bus;

	list_for_each(entry, &mii_devs) {
		цео i;
		bus = list_entry(entry, struct mii_dev, link);

		за (i = 0; i < PHY_MAX_ADDR; i++) {
			ако (!bus->phymap[i] || !bus->phymap[i]->dev)
				continue;

			ако (strcmp(bus->phymap[i]->dev->name, ethname) == 0)
				врати bus->phymap[i];
		}
	}

	printf("%s is not a known ethernet\n", ethname);
	врати NULL;
}

const char *miiphy_get_current_dev(void)
{
	ако (current_mii)
		врати current_mii->name;

	врати NULL;
}

static struct mii_dev *miiphy_get_active_dev(const char *devname)
{
	/* If the current mii is the one we want, return it */
	ако (current_mii)
		ако (strcmp(current_mii->name, devname) == 0)
			врати current_mii;

	/* Otherwise, set the active one to the one we want */
	ако (miiphy_set_current_dev(devname))
		врати NULL;
	инако
		врати current_mii;
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
цео miiphy_read(const char *devname, unsigned char addr, unsigned char reg,
		 unsigned short *value)
{
	struct mii_dev *bus;
	цео ret;

	bus = miiphy_get_active_dev(devname);
	ако (!bus)
		врати 1;

	ret = bus->read(bus, addr, MDIO_DEVAD_NONE, reg);
	ако (ret < 0)
		врати 1;

	*value = (unsigned short)ret;
	врати 0;
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
цео miiphy_write(const char *devname, unsigned char addr, unsigned char reg,
		  unsigned short value)
{
	struct mii_dev *bus;

	bus = miiphy_get_active_dev(devname);
	ако (bus)
		врати bus->write(bus, addr, MDIO_DEVAD_NONE, reg, value);

	врати 1;
}

/*****************************************************************************
 *
 * Print out list of registered MII capable devices.
 */
void miiphy_listdev(void)
{
	struct list_head *entry;
	struct mii_dev *dev;

	puts("MII devices: ");
	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, struct mii_dev, link);
		printf("'%s' ", dev->name);
	}
	puts("\n");

	ако (current_mii)
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
цео miiphy_info(const char *devname, unsigned char addr, unsigned цео *oui,
		 unsigned char *model, unsigned char *rev)
{
	unsigned цео reg = 0;
	unsigned short tmp;

	ако (miiphy_read(devname, addr, MII_PHYSID2, &tmp) != 0) {
		debug("PHY ID register 2 read failed\n");
		врати -1;
	}
	reg = tmp;

	debug("MII_PHYSID2 @ 0x%x = 0x%04x\n", addr, reg);

	ако (reg == 0xFFFF) {
		/* No physical device present at this address */
		врати -1;
	}

	ако (miiphy_read(devname, addr, MII_PHYSID1, &tmp) != 0) {
		debug("PHY ID register 1 read failed\n");
		врати -1;
	}
	reg |= tmp << 16;
	debug("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (unsigned char)((reg >> 4) & 0x0000003F);
	*rev = (unsigned char)(reg & 0x0000000F);
	врати 0;
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
цео miiphy_reset(const char *devname, unsigned char addr)
{
	unsigned short reg;
	цео timeout = 500;

	ако (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
		debug("PHY status read failed\n");
		врати -1;
	}
	ако (miiphy_write(devname, addr, MII_BMCR, reg | BMCR_RESET) != 0) {
		debug("PHY reset failed\n");
		врати -1;
	}
#ако CONFIG_PHY_RESET_DELAY > 0
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#endif
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = 0x8000;
	док (((reg & 0x8000) != 0) && timeout--) {
		ако (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			врати -1;
		}
		udelay(1000);
	}
	ако ((reg & 0x8000) == 0) {
		врати 0;
	} инако {
		puts("PHY reset timed out\n");
		врати -1;
	}
	врати 0;
}
#endif /* !PHYLIB */

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
цео miiphy_speed(const char *devname, unsigned char addr)
{
	u16 bmcr, anlpar, adv;

#ако defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	ако (miiphy_is_1000base_x(devname, addr))
		врати _1000BASET;

	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	ако (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		goto miiphy_read_failed;
	}
	ако (btsr != 0xFFFF &&
			(btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)))
		врати _1000BASET;
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	ако (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		printf("PHY speed");
		goto miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	ако (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		ако (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("PHY AN speed");
			goto miiphy_read_failed;
		}

		ако (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv speed");
			goto miiphy_read_failed;
		}
		врати ((anlpar & adv) & LPA_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	врати (bmcr & BMCR_SPEED100) ? _100BASET : _10BASET;

miiphy_read_failed:
	printf(" read failed, assuming 10BASE-T\n");
	врати _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
цео miiphy_duplex(const char *devname, unsigned char addr)
{
	u16 bmcr, anlpar, adv;

#ако defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	ако (miiphy_is_1000base_x(devname, addr)) {
		/* 1000BASE-X */
		ако (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("1000BASE-X PHY AN duplex");
			goto miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	ако (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		goto miiphy_read_failed;
	}
	ако (btsr != 0xFFFF) {
		ако (btsr & PHY_1000BTSR_1000FD) {
			врати FULL;
		} инако ако (btsr & PHY_1000BTSR_1000HD) {
			врати HALF;
		}
	}
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	ако (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		puts("PHY duplex");
		goto miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	ако (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		ако (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			puts("PHY AN duplex");
			goto miiphy_read_failed;
		}

		ако (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv duplex");
			goto miiphy_read_failed;
		}
		врати ((anlpar & adv) & (LPA_10FULL | LPA_100FULL)) ?
		    FULL : HALF;
	}
	/* Get speed from basic control settings. */
	врати (bmcr & BMCR_FULLDPLX) ? FULL : HALF;

miiphy_read_failed:
	printf(" read failed, assuming half duplex\n");
	врати HALF;
}

/*****************************************************************************
 *
 * Return 1 if PHY supports 1000BASE-X, 0 if PHY supports 10BASE-T/100BASE-TX/
 * 1000BASE-T, or on error.
 */
цео miiphy_is_1000base_x(const char *devname, unsigned char addr)
{
#ако defined(CONFIG_PHY_GIGE)
	u16 exsr;

	ако (miiphy_read(devname, addr, MII_ESTATUS, &exsr)) {
		printf("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		врати 0;
	}
	врати 0 != (exsr & (ESTATUS_1000XF | ESTATUS_1000XH));
#инако
	врати 0;
#endif
}
