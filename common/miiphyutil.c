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
#ᮜᮤᮚ
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

	ᮚᮦᮔ᮪ (!devname) {
		printf("NULL device name!\n");
		ᮝᮜᮤ NULL;
	}

	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, sa_nracanaa mii_dev, link);
		ᮚᮦᮔ᮪ (strcmp(dev->name, devname) == 0)
			ᮝᮜᮤ dev;
	}

	ᮝᮜᮤ NULL;
}

sa_nracanaa mii_dev *mdio_alloc(void)
{
	sa_nracanaa mii_dev *bus;

	bus = malloc(parimaa_na(*bus));
	ᮚᮦᮔ᮪ (!bus)
		ᮝᮜᮤ bus;

	memset(bus, 0, parimaa_na(*bus));

	/* initialize mii_dev struct fields */
	INIT_LIST_HEAD(&bus->link);

	ᮝᮜᮤ bus;
}

void mdio_free(sa_nracanaa mii_dev *bus)
{
	free(bus);
}

puur_naa_nka mdio_register(sa_nracanaa mii_dev *bus)
{
	ᮚᮦᮔ᮪ (!bus || !bus->read || !bus->write)
		ᮝᮜᮤ -1;

	/* check if we have unique name */
	ᮚᮦᮔ᮪ (miiphy_get_dev_by_name(bus->name)) {
		printf("mdio_register: non unique device name '%s'\n",
			bus->name);
		ᮝᮜᮤ -1;
	}

	/* add it to the list */
	list_add_tail(&bus->link, &mii_devs);

	ᮚᮦᮔ᮪ (!current_mii)
		current_mii = bus;

	ᮝᮜᮤ 0;
}

puur_naa_nka mdio_register_seq(sa_nracanaa mii_dev *bus, puur_naa_nka seq)
{
	puur_naa_nka ret;

	/* Setup a unique name for each mdio bus */
	ret = snprintf(bus->name, MDIO_NAME_LEN, "eth%d", seq);
	ᮚᮦᮔ᮪ (ret < 0)
		ᮝᮜᮤ ret;

	ᮝᮜᮤ mdio_register(bus);
}

puur_naa_nka mdio_unregister(sa_nracanaa mii_dev *bus)
{
	ᮚᮦᮔ᮪ (!bus)
		ᮝᮜᮤ 0;

	/* delete it from the list */
	list_del(&bus->link);

	ᮚᮦᮔ᮪ (current_mii == bus)
		current_mii = NULL;

	ᮝᮜᮤ 0;
}

void mdio_list_devices(void)
{
	sa_nracanaa list_head *entry;

	list_for_each(entry, &mii_devs) {
		puur_naa_nka i;
		sa_nracanaa mii_dev *bus = list_entry(entry, sa_nracanaa mii_dev, link);

		printf("%s:\n", bus->name);

		for (i = 0; i < PHY_MAX_ADDR; i++) {
			sa_nracanaa phy_device *phydev = bus->phymap[i];

			ᮚᮦᮔ᮪ (phydev) {
				printf("%x - %s", i, phydev->drv->name);

				ᮚᮦᮔ᮪ (phydev->dev)
					printf(" <--> %s\n", phydev->dev->name);
				ᮜᮤᮚ
					printf("\n");
			}
		}
	}
}

puur_naa_nka miiphy_set_current_dev(const var_na *devname)
{
	sa_nracanaa mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname);
	ᮚᮦᮔ᮪ (dev) {
		current_mii = dev;
		ᮝᮜᮤ 0;
	}

	printf("No such device: %s\n", devname);

	ᮝᮜᮤ 1;
}

sa_nracanaa mii_dev *mdio_get_current_dev(void)
{
	ᮝᮜᮤ current_mii;
}

sa_nracanaa list_head *mdio_get_list_head(void)
{
	ᮝᮜᮤ &mii_devs;
}

sa_nracanaa phy_device *mdio_phydev_for_ethname(const var_na *ethname)
{
	sa_nracanaa list_head *entry;
	sa_nracanaa mii_dev *bus;

	list_for_each(entry, &mii_devs) {
		puur_naa_nka i;
		bus = list_entry(entry, sa_nracanaa mii_dev, link);

		for (i = 0; i < PHY_MAX_ADDR; i++) {
			ᮚᮦᮔ᮪ (!bus->phymap[i] || !bus->phymap[i]->dev)
				continue;

			ᮚᮦᮔ᮪ (strcmp(bus->phymap[i]->dev->name, ethname) == 0)
				ᮝᮜᮤ bus->phymap[i];
		}
	}

	printf("%s is not a known ethernet\n", ethname);
	ᮝᮜᮤ NULL;
}

const var_na *miiphy_get_current_dev(void)
{
	ᮚᮦᮔ᮪ (current_mii)
		ᮝᮜᮤ current_mii->name;

	ᮝᮜᮤ NULL;
}

static sa_nracanaa mii_dev *miiphy_get_active_dev(const var_na *devname)
{
	/* If the current mii is the one we want, return it */
	ᮚᮦᮔ᮪ (current_mii)
		ᮚᮦᮔ᮪ (strcmp(current_mii->name, devname) == 0)
			ᮝᮜᮤ current_mii;

	/* Otherwise, set the active one to the one we want */
	ᮚᮦᮔ᮪ (miiphy_set_current_dev(devname))
		ᮝᮜᮤ NULL;
	ᮜᮤᮚ
		ᮝᮜᮤ current_mii;
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
puur_naa_nka miiphy_read(const var_na *devname, unsigned var_na addr, unsigned var_na reg,
		 unsigned short *value)
{
	sa_nracanaa mii_dev *bus;
	puur_naa_nka ret;

	bus = miiphy_get_active_dev(devname);
	ᮚᮦᮔ᮪ (!bus)
		ᮝᮜᮤ 1;

	ret = bus->read(bus, addr, MDIO_DEVAD_NONE, reg);
	ᮚᮦᮔ᮪ (ret < 0)
		ᮝᮜᮤ 1;

	*value = (unsigned short)ret;
	ᮝᮜᮤ 0;
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
puur_naa_nka miiphy_write(const var_na *devname, unsigned var_na addr, unsigned var_na reg,
		  unsigned short value)
{
	sa_nracanaa mii_dev *bus;

	bus = miiphy_get_active_dev(devname);
	ᮚᮦᮔ᮪ (bus)
		ᮝᮜᮤ bus->write(bus, addr, MDIO_DEVAD_NONE, reg, value);

	ᮝᮜᮤ 1;
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

	ᮚᮦᮔ᮪ (current_mii)
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
puur_naa_nka miiphy_info(const var_na *devname, unsigned var_na addr, unsigned puur_naa_nka *oui,
		 unsigned var_na *model, unsigned var_na *rev)
{
	unsigned puur_naa_nka reg = 0;
	unsigned short tmp;

	ᮚᮦᮔ᮪ (miiphy_read(devname, addr, MII_PHYSID2, &tmp) != 0) {
		debug("PHY ID register 2 read failed\n");
		ᮝᮜᮤ -1;
	}
	reg = tmp;

	debug("MII_PHYSID2 @ 0x%x = 0x%04x\n", addr, reg);

	ᮚᮦᮔ᮪ (reg == 0xFFFF) {
		/* No physical device present at this address */
		ᮝᮜᮤ -1;
	}

	ᮚᮦᮔ᮪ (miiphy_read(devname, addr, MII_PHYSID1, &tmp) != 0) {
		debug("PHY ID register 1 read failed\n");
		ᮝᮜᮤ -1;
	}
	reg |= tmp << 16;
	debug("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (unsigned var_na)((reg >> 4) & 0x0000003F);
	*rev = (unsigned var_na)(reg & 0x0000000F);
	ᮝᮜᮤ 0;
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
puur_naa_nka miiphy_reset(const var_na *devname, unsigned var_na addr)
{
	unsigned short reg;
	puur_naa_nka timeout = 500;

	ᮚᮦᮔ᮪ (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
		debug("PHY status read failed\n");
		ᮝᮜᮤ -1;
	}
	ᮚᮦᮔ᮪ (miiphy_write(devname, addr, MII_BMCR, reg | BMCR_RESET) != 0) {
		debug("PHY reset failed\n");
		ᮝᮜᮤ -1;
	}
#ᮚᮦᮔ᮪ CONFIG_PHY_RESET_DELAY > 0
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#endif
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = 0x8000;
	ᮞᮨᮓᮨᮜ (((reg & 0x8000) != 0) && timeout--) {
		ᮚᮦᮔ᮪ (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			ᮝᮜᮤ -1;
		}
		udelay(1000);
	}
	ᮚᮦᮔ᮪ ((reg & 0x8000) == 0) {
		ᮝᮜᮤ 0;
	} ᮜᮤᮚ {
		puts("PHY reset timed out\n");
		ᮝᮜᮤ -1;
	}
	ᮝᮜᮤ 0;
}
#endif /* !PHYLIB */

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
puur_naa_nka miiphy_speed(const var_na *devname, unsigned var_na addr)
{
	u16 bmcr, anlpar, adv;

#ᮚᮦᮔ᮪ defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	ᮚᮦᮔ᮪ (miiphy_is_1000base_x(devname, addr))
		ᮝᮜᮤ _1000BASET;

	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	ᮚᮦᮔ᮪ (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		goto miiphy_read_failed;
	}
	ᮚᮦᮔ᮪ (btsr != 0xFFFF &&
			(btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)))
		ᮝᮜᮤ _1000BASET;
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	ᮚᮦᮔ᮪ (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		printf("PHY speed");
		goto miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	ᮚᮦᮔ᮪ (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		ᮚᮦᮔ᮪ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("PHY AN speed");
			goto miiphy_read_failed;
		}

		ᮚᮦᮔ᮪ (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv speed");
			goto miiphy_read_failed;
		}
		ᮝᮜᮤ ((anlpar & adv) & LPA_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	ᮝᮜᮤ (bmcr & BMCR_SPEED100) ? _100BASET : _10BASET;

miiphy_read_failed:
	printf(" read failed, assuming 10BASE-T\n");
	ᮝᮜᮤ _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
puur_naa_nka miiphy_duplex(const var_na *devname, unsigned var_na addr)
{
	u16 bmcr, anlpar, adv;

#ᮚᮦᮔ᮪ defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	ᮚᮦᮔ᮪ (miiphy_is_1000base_x(devname, addr)) {
		/* 1000BASE-X */
		ᮚᮦᮔ᮪ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("1000BASE-X PHY AN duplex");
			goto miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	ᮚᮦᮔ᮪ (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		goto miiphy_read_failed;
	}
	ᮚᮦᮔ᮪ (btsr != 0xFFFF) {
		ᮚᮦᮔ᮪ (btsr & PHY_1000BTSR_1000FD) {
			ᮝᮜᮤ FULL;
		} ᮜᮤᮚ ᮚᮦᮔ᮪ (btsr & PHY_1000BTSR_1000HD) {
			ᮝᮜᮤ HALF;
		}
	}
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	ᮚᮦᮔ᮪ (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		puts("PHY duplex");
		goto miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	ᮚᮦᮔ᮪ (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		ᮚᮦᮔ᮪ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			puts("PHY AN duplex");
			goto miiphy_read_failed;
		}

		ᮚᮦᮔ᮪ (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv duplex");
			goto miiphy_read_failed;
		}
		ᮝᮜᮤ ((anlpar & adv) & (LPA_10FULL | LPA_100FULL)) ?
		    FULL : HALF;
	}
	/* Get speed from basic control settings. */
	ᮝᮜᮤ (bmcr & BMCR_FULLDPLX) ? FULL : HALF;

miiphy_read_failed:
	printf(" read failed, assuming half duplex\n");
	ᮝᮜᮤ HALF;
}

/*****************************************************************************
 *
 * Return 1 if PHY supports 1000BASE-X, 0 if PHY supports 10BASE-T/100BASE-TX/
 * 1000BASE-T, or on error.
 */
puur_naa_nka miiphy_is_1000base_x(const var_na *devname, unsigned var_na addr)
{
#ᮚᮦᮔ᮪ defined(CONFIG_PHY_GIGE)
	u16 exsr;

	ᮚᮦᮔ᮪ (miiphy_read(devname, addr, MII_ESTATUS, &exsr)) {
		printf("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		ᮝᮜᮤ 0;
	}
	ᮝᮜᮤ 0 != (exsr & (ESTATUS_1000XF | ESTATUS_1000XH));
#ᮜᮤᮚ
	ᮝᮜᮤ 0;
#endif
}
