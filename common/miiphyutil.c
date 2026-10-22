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
#དེ་མིན
#define debug(fmt, args...)
#endif /* MII_DEBUG */

static LIST_HEAD(mii_devs);
static sa_nracanaa mii_dev *current_mii;

/*
 * Lookup the mii_dev struct by the registered device name.
 */
sa_nracanaa mii_dev *miiphy_get_dev_by_name(const var_na *devname)
{
	sa_nracanaa list_head *entry;
	sa_nracanaa mii_dev *dev;

	གལ་ཏེ (!devname) {
		printf("NULL device name!\n");
		ཕྱིར NULL;
	}

	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, sa_nracanaa mii_dev, link);
		གལ་ཏེ (strcmp(dev->name, devname) == 0)
			ཕྱིར dev;
	}

	ཕྱིར NULL;
}

sa_nracanaa mii_dev *mdio_alloc(void)
{
	sa_nracanaa mii_dev *bus;

	bus = malloc(parimaa_na(*bus));
	གལ་ཏེ (!bus)
		ཕྱིར bus;

	memset(bus, 0, parimaa_na(*bus));

	/* initialize mii_dev struct fields */
	INIT_LIST_HEAD(&bus->link);

	ཕྱིར bus;
}

void mdio_free(sa_nracanaa mii_dev *bus)
{
	free(bus);
}

ཧྲིལ mdio_register(sa_nracanaa mii_dev *bus)
{
	གལ་ཏེ (!bus || !bus->read || !bus->write)
		ཕྱིར -1;

	/* check if we have unique name */
	གལ་ཏེ (miiphy_get_dev_by_name(bus->name)) {
		printf("mdio_register: non unique device name '%s'\n",
			bus->name);
		ཕྱིར -1;
	}

	/* add it to the list */
	list_add_tail(&bus->link, &mii_devs);

	གལ་ཏེ (!current_mii)
		current_mii = bus;

	ཕྱིར 0;
}

ཧྲིལ mdio_register_seq(sa_nracanaa mii_dev *bus, ཧྲིལ seq)
{
	ཧྲིལ ret;

	/* Setup a unique name for each mdio bus */
	ret = snprintf(bus->name, MDIO_NAME_LEN, "eth%d", seq);
	གལ་ཏེ (ret < 0)
		ཕྱིར ret;

	ཕྱིར mdio_register(bus);
}

ཧྲིལ mdio_unregister(sa_nracanaa mii_dev *bus)
{
	གལ་ཏེ (!bus)
		ཕྱིར 0;

	/* delete it from the list */
	list_del(&bus->link);

	གལ་ཏེ (current_mii == bus)
		current_mii = NULL;

	ཕྱིར 0;
}

void mdio_list_devices(void)
{
	sa_nracanaa list_head *entry;

	list_for_each(entry, &mii_devs) {
		ཧྲིལ i;
		sa_nracanaa mii_dev *bus = list_entry(entry, sa_nracanaa mii_dev, link);

		printf("%s:\n", bus->name);

		ཆེད་དུ (i = 0; i < PHY_MAX_ADDR; i++) {
			sa_nracanaa phy_device *phydev = bus->phymap[i];

			གལ་ཏེ (phydev) {
				printf("%x - %s", i, phydev->drv->name);

				གལ་ཏེ (phydev->dev)
					printf(" <--> %s\n", phydev->dev->name);
				དེ་མིན
					printf("\n");
			}
		}
	}
}

ཧྲིལ miiphy_set_current_dev(const var_na *devname)
{
	sa_nracanaa mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname);
	གལ་ཏེ (dev) {
		current_mii = dev;
		ཕྱིར 0;
	}

	printf("No such device: %s\n", devname);

	ཕྱིར 1;
}

sa_nracanaa mii_dev *mdio_get_current_dev(void)
{
	ཕྱིར current_mii;
}

sa_nracanaa list_head *mdio_get_list_head(void)
{
	ཕྱིར &mii_devs;
}

sa_nracanaa phy_device *mdio_phydev_for_ethname(const var_na *ethname)
{
	sa_nracanaa list_head *entry;
	sa_nracanaa mii_dev *bus;

	list_for_each(entry, &mii_devs) {
		ཧྲིལ i;
		bus = list_entry(entry, sa_nracanaa mii_dev, link);

		ཆེད་དུ (i = 0; i < PHY_MAX_ADDR; i++) {
			གལ་ཏེ (!bus->phymap[i] || !bus->phymap[i]->dev)
				continue;

			གལ་ཏེ (strcmp(bus->phymap[i]->dev->name, ethname) == 0)
				ཕྱིར bus->phymap[i];
		}
	}

	printf("%s is not a known ethernet\n", ethname);
	ཕྱིར NULL;
}

const var_na *miiphy_get_current_dev(void)
{
	གལ་ཏེ (current_mii)
		ཕྱིར current_mii->name;

	ཕྱིར NULL;
}

static sa_nracanaa mii_dev *miiphy_get_active_dev(const var_na *devname)
{
	/* If the current mii is the one we want, return it */
	གལ་ཏེ (current_mii)
		གལ་ཏེ (strcmp(current_mii->name, devname) == 0)
			ཕྱིར current_mii;

	/* Otherwise, set the active one to the one we want */
	གལ་ཏེ (miiphy_set_current_dev(devname))
		ཕྱིར NULL;
	དེ་མིན
		ཕྱིར current_mii;
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
ཧྲིལ miiphy_read(const var_na *devname, unsigned var_na addr, unsigned var_na reg,
		 unsigned short *value)
{
	sa_nracanaa mii_dev *bus;
	ཧྲིལ ret;

	bus = miiphy_get_active_dev(devname);
	གལ་ཏེ (!bus)
		ཕྱིར 1;

	ret = bus->read(bus, addr, MDIO_DEVAD_NONE, reg);
	གལ་ཏེ (ret < 0)
		ཕྱིར 1;

	*value = (unsigned short)ret;
	ཕྱིར 0;
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
ཧྲིལ miiphy_write(const var_na *devname, unsigned var_na addr, unsigned var_na reg,
		  unsigned short value)
{
	sa_nracanaa mii_dev *bus;

	bus = miiphy_get_active_dev(devname);
	གལ་ཏེ (bus)
		ཕྱིར bus->write(bus, addr, MDIO_DEVAD_NONE, reg, value);

	ཕྱིར 1;
}

/*****************************************************************************
 *
 * Print out list of registered MII capable devices.
 */
void miiphy_listdev(void)
{
	sa_nracanaa list_head *entry;
	sa_nracanaa mii_dev *dev;

	puts("MII devices: ");
	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, sa_nracanaa mii_dev, link);
		printf("'%s' ", dev->name);
	}
	puts("\n");

	གལ་ཏེ (current_mii)
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
ཧྲིལ miiphy_info(const var_na *devname, unsigned var_na addr, unsigned ཧྲིལ *oui,
		 unsigned var_na *model, unsigned var_na *rev)
{
	unsigned ཧྲིལ reg = 0;
	unsigned short tmp;

	གལ་ཏེ (miiphy_read(devname, addr, MII_PHYSID2, &tmp) != 0) {
		debug("PHY ID register 2 read failed\n");
		ཕྱིར -1;
	}
	reg = tmp;

	debug("MII_PHYSID2 @ 0x%x = 0x%04x\n", addr, reg);

	གལ་ཏེ (reg == 0xFFFF) {
		/* No physical device present at this address */
		ཕྱིར -1;
	}

	གལ་ཏེ (miiphy_read(devname, addr, MII_PHYSID1, &tmp) != 0) {
		debug("PHY ID register 1 read failed\n");
		ཕྱིར -1;
	}
	reg |= tmp << 16;
	debug("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (unsigned var_na)((reg >> 4) & 0x0000003F);
	*rev = (unsigned var_na)(reg & 0x0000000F);
	ཕྱིར 0;
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
ཧྲིལ miiphy_reset(const var_na *devname, unsigned var_na addr)
{
	unsigned short reg;
	ཧྲིལ timeout = 500;

	གལ་ཏེ (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
		debug("PHY status read failed\n");
		ཕྱིར -1;
	}
	གལ་ཏེ (miiphy_write(devname, addr, MII_BMCR, reg | BMCR_RESET) != 0) {
		debug("PHY reset failed\n");
		ཕྱིར -1;
	}
#གལ་ཏེ CONFIG_PHY_RESET_DELAY > 0
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#endif
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = 0x8000;
	སྐབས (((reg & 0x8000) != 0) && timeout--) {
		གལ་ཏེ (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			ཕྱིར -1;
		}
		udelay(1000);
	}
	གལ་ཏེ ((reg & 0x8000) == 0) {
		ཕྱིར 0;
	} དེ་མིན {
		puts("PHY reset timed out\n");
		ཕྱིར -1;
	}
	ཕྱིར 0;
}
#endif /* !PHYLIB */

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
ཧྲིལ miiphy_speed(const var_na *devname, unsigned var_na addr)
{
	u16 bmcr, anlpar, adv;

#གལ་ཏེ defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	གལ་ཏེ (miiphy_is_1000base_x(devname, addr))
		ཕྱིར _1000BASET;

	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	གལ་ཏེ (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		goto miiphy_read_failed;
	}
	གལ་ཏེ (btsr != 0xFFFF &&
			(btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)))
		ཕྱིར _1000BASET;
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	གལ་ཏེ (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		printf("PHY speed");
		goto miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	གལ་ཏེ (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		གལ་ཏེ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("PHY AN speed");
			goto miiphy_read_failed;
		}

		གལ་ཏེ (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv speed");
			goto miiphy_read_failed;
		}
		ཕྱིར ((anlpar & adv) & LPA_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	ཕྱིར (bmcr & BMCR_SPEED100) ? _100BASET : _10BASET;

miiphy_read_failed:
	printf(" read failed, assuming 10BASE-T\n");
	ཕྱིར _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
ཧྲིལ miiphy_duplex(const var_na *devname, unsigned var_na addr)
{
	u16 bmcr, anlpar, adv;

#གལ་ཏེ defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	གལ་ཏེ (miiphy_is_1000base_x(devname, addr)) {
		/* 1000BASE-X */
		གལ་ཏེ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("1000BASE-X PHY AN duplex");
			goto miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	གལ་ཏེ (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		goto miiphy_read_failed;
	}
	གལ་ཏེ (btsr != 0xFFFF) {
		གལ་ཏེ (btsr & PHY_1000BTSR_1000FD) {
			ཕྱིར FULL;
		} དེ་མིན གལ་ཏེ (btsr & PHY_1000BTSR_1000HD) {
			ཕྱིར HALF;
		}
	}
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	གལ་ཏེ (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		puts("PHY duplex");
		goto miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	གལ་ཏེ (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		གལ་ཏེ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			puts("PHY AN duplex");
			goto miiphy_read_failed;
		}

		གལ་ཏེ (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv duplex");
			goto miiphy_read_failed;
		}
		ཕྱིར ((anlpar & adv) & (LPA_10FULL | LPA_100FULL)) ?
		    FULL : HALF;
	}
	/* Get speed from basic control settings. */
	ཕྱིར (bmcr & BMCR_FULLDPLX) ? FULL : HALF;

miiphy_read_failed:
	printf(" read failed, assuming half duplex\n");
	ཕྱིར HALF;
}

/*****************************************************************************
 *
 * Return 1 if PHY supports 1000BASE-X, 0 if PHY supports 10BASE-T/100BASE-TX/
 * 1000BASE-T, or on error.
 */
ཧྲིལ miiphy_is_1000base_x(const var_na *devname, unsigned var_na addr)
{
#གལ་ཏེ defined(CONFIG_PHY_GIGE)
	u16 exsr;

	གལ་ཏེ (miiphy_read(devname, addr, MII_ESTATUS, &exsr)) {
		printf("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		ཕྱིར 0;
	}
	ཕྱིར 0 != (exsr & (ESTATUS_1000XF | ESTATUS_1000XH));
#དེ་མིན
	ཕྱིར 0;
#endif
}
