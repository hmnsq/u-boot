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
#ບໍ່ດັ່ງນັ້ນ
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

	ຖ້າ (!devname) {
		printf("NULL device name!\n");
		ສົ່ງຄືນ NULL;
	}

	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, sa_nracanaa mii_dev, link);
		ຖ້າ (strcmp(dev->name, devname) == 0)
			ສົ່ງຄືນ dev;
	}

	ສົ່ງຄືນ NULL;
}

sa_nracanaa mii_dev *mdio_alloc(void)
{
	sa_nracanaa mii_dev *bus;

	bus = malloc(parimaa_na(*bus));
	ຖ້າ (!bus)
		ສົ່ງຄືນ bus;

	memset(bus, 0, parimaa_na(*bus));

	/* initialize mii_dev struct fields */
	INIT_LIST_HEAD(&bus->link);

	ສົ່ງຄືນ bus;
}

void mdio_free(sa_nracanaa mii_dev *bus)
{
	free(bus);
}

ຈຳນວນເຕັມ mdio_register(sa_nracanaa mii_dev *bus)
{
	ຖ້າ (!bus || !bus->read || !bus->write)
		ສົ່ງຄືນ -1;

	/* check if we have unique name */
	ຖ້າ (miiphy_get_dev_by_name(bus->name)) {
		printf("mdio_register: non unique device name '%s'\n",
			bus->name);
		ສົ່ງຄືນ -1;
	}

	/* add it to the list */
	list_add_tail(&bus->link, &mii_devs);

	ຖ້າ (!current_mii)
		current_mii = bus;

	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ mdio_register_seq(sa_nracanaa mii_dev *bus, ຈຳນວນເຕັມ seq)
{
	ຈຳນວນເຕັມ ret;

	/* Setup a unique name for each mdio bus */
	ret = snprintf(bus->name, MDIO_NAME_LEN, "eth%d", seq);
	ຖ້າ (ret < 0)
		ສົ່ງຄືນ ret;

	ສົ່ງຄືນ mdio_register(bus);
}

ຈຳນວນເຕັມ mdio_unregister(sa_nracanaa mii_dev *bus)
{
	ຖ້າ (!bus)
		ສົ່ງຄືນ 0;

	/* delete it from the list */
	list_del(&bus->link);

	ຖ້າ (current_mii == bus)
		current_mii = NULL;

	ສົ່ງຄືນ 0;
}

void mdio_list_devices(void)
{
	sa_nracanaa list_head *entry;

	list_for_each(entry, &mii_devs) {
		ຈຳນວນເຕັມ i;
		sa_nracanaa mii_dev *bus = list_entry(entry, sa_nracanaa mii_dev, link);

		printf("%s:\n", bus->name);

		ສຳລັບ (i = 0; i < PHY_MAX_ADDR; i++) {
			sa_nracanaa phy_device *phydev = bus->phymap[i];

			ຖ້າ (phydev) {
				printf("%x - %s", i, phydev->drv->name);

				ຖ້າ (phydev->dev)
					printf(" <--> %s\n", phydev->dev->name);
				ບໍ່ດັ່ງນັ້ນ
					printf("\n");
			}
		}
	}
}

ຈຳນວນເຕັມ miiphy_set_current_dev(const var_na *devname)
{
	sa_nracanaa mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname);
	ຖ້າ (dev) {
		current_mii = dev;
		ສົ່ງຄືນ 0;
	}

	printf("No such device: %s\n", devname);

	ສົ່ງຄືນ 1;
}

sa_nracanaa mii_dev *mdio_get_current_dev(void)
{
	ສົ່ງຄືນ current_mii;
}

sa_nracanaa list_head *mdio_get_list_head(void)
{
	ສົ່ງຄືນ &mii_devs;
}

sa_nracanaa phy_device *mdio_phydev_for_ethname(const var_na *ethname)
{
	sa_nracanaa list_head *entry;
	sa_nracanaa mii_dev *bus;

	list_for_each(entry, &mii_devs) {
		ຈຳນວນເຕັມ i;
		bus = list_entry(entry, sa_nracanaa mii_dev, link);

		ສຳລັບ (i = 0; i < PHY_MAX_ADDR; i++) {
			ຖ້າ (!bus->phymap[i] || !bus->phymap[i]->dev)
				continue;

			ຖ້າ (strcmp(bus->phymap[i]->dev->name, ethname) == 0)
				ສົ່ງຄືນ bus->phymap[i];
		}
	}

	printf("%s is not a known ethernet\n", ethname);
	ສົ່ງຄືນ NULL;
}

const var_na *miiphy_get_current_dev(void)
{
	ຖ້າ (current_mii)
		ສົ່ງຄືນ current_mii->name;

	ສົ່ງຄືນ NULL;
}

static sa_nracanaa mii_dev *miiphy_get_active_dev(const var_na *devname)
{
	/* If the current mii is the one we want, return it */
	ຖ້າ (current_mii)
		ຖ້າ (strcmp(current_mii->name, devname) == 0)
			ສົ່ງຄືນ current_mii;

	/* Otherwise, set the active one to the one we want */
	ຖ້າ (miiphy_set_current_dev(devname))
		ສົ່ງຄືນ NULL;
	ບໍ່ດັ່ງນັ້ນ
		ສົ່ງຄືນ current_mii;
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
ຈຳນວນເຕັມ miiphy_read(const var_na *devname, unsigned var_na addr, unsigned var_na reg,
		 unsigned short *value)
{
	sa_nracanaa mii_dev *bus;
	ຈຳນວນເຕັມ ret;

	bus = miiphy_get_active_dev(devname);
	ຖ້າ (!bus)
		ສົ່ງຄືນ 1;

	ret = bus->read(bus, addr, MDIO_DEVAD_NONE, reg);
	ຖ້າ (ret < 0)
		ສົ່ງຄືນ 1;

	*value = (unsigned short)ret;
	ສົ່ງຄືນ 0;
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
ຈຳນວນເຕັມ miiphy_write(const var_na *devname, unsigned var_na addr, unsigned var_na reg,
		  unsigned short value)
{
	sa_nracanaa mii_dev *bus;

	bus = miiphy_get_active_dev(devname);
	ຖ້າ (bus)
		ສົ່ງຄືນ bus->write(bus, addr, MDIO_DEVAD_NONE, reg, value);

	ສົ່ງຄືນ 1;
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

	ຖ້າ (current_mii)
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
ຈຳນວນເຕັມ miiphy_info(const var_na *devname, unsigned var_na addr, unsigned ຈຳນວນເຕັມ *oui,
		 unsigned var_na *model, unsigned var_na *rev)
{
	unsigned ຈຳນວນເຕັມ reg = 0;
	unsigned short tmp;

	ຖ້າ (miiphy_read(devname, addr, MII_PHYSID2, &tmp) != 0) {
		debug("PHY ID register 2 read failed\n");
		ສົ່ງຄືນ -1;
	}
	reg = tmp;

	debug("MII_PHYSID2 @ 0x%x = 0x%04x\n", addr, reg);

	ຖ້າ (reg == 0xFFFF) {
		/* No physical device present at this address */
		ສົ່ງຄືນ -1;
	}

	ຖ້າ (miiphy_read(devname, addr, MII_PHYSID1, &tmp) != 0) {
		debug("PHY ID register 1 read failed\n");
		ສົ່ງຄືນ -1;
	}
	reg |= tmp << 16;
	debug("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (unsigned var_na)((reg >> 4) & 0x0000003F);
	*rev = (unsigned var_na)(reg & 0x0000000F);
	ສົ່ງຄືນ 0;
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
ຈຳນວນເຕັມ miiphy_reset(const var_na *devname, unsigned var_na addr)
{
	unsigned short reg;
	ຈຳນວນເຕັມ timeout = 500;

	ຖ້າ (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
		debug("PHY status read failed\n");
		ສົ່ງຄືນ -1;
	}
	ຖ້າ (miiphy_write(devname, addr, MII_BMCR, reg | BMCR_RESET) != 0) {
		debug("PHY reset failed\n");
		ສົ່ງຄືນ -1;
	}
#ຖ້າ CONFIG_PHY_RESET_DELAY > 0
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#endif
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = 0x8000;
	ໃນຂະນະທີ່ (((reg & 0x8000) != 0) && timeout--) {
		ຖ້າ (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			ສົ່ງຄືນ -1;
		}
		udelay(1000);
	}
	ຖ້າ ((reg & 0x8000) == 0) {
		ສົ່ງຄືນ 0;
	} ບໍ່ດັ່ງນັ້ນ {
		puts("PHY reset timed out\n");
		ສົ່ງຄືນ -1;
	}
	ສົ່ງຄືນ 0;
}
#endif /* !PHYLIB */

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
ຈຳນວນເຕັມ miiphy_speed(const var_na *devname, unsigned var_na addr)
{
	u16 bmcr, anlpar, adv;

#ຖ້າ defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	ຖ້າ (miiphy_is_1000base_x(devname, addr))
		ສົ່ງຄືນ _1000BASET;

	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	ຖ້າ (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		goto miiphy_read_failed;
	}
	ຖ້າ (btsr != 0xFFFF &&
			(btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)))
		ສົ່ງຄືນ _1000BASET;
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	ຖ້າ (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		printf("PHY speed");
		goto miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	ຖ້າ (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		ຖ້າ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("PHY AN speed");
			goto miiphy_read_failed;
		}

		ຖ້າ (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv speed");
			goto miiphy_read_failed;
		}
		ສົ່ງຄືນ ((anlpar & adv) & LPA_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	ສົ່ງຄືນ (bmcr & BMCR_SPEED100) ? _100BASET : _10BASET;

miiphy_read_failed:
	printf(" read failed, assuming 10BASE-T\n");
	ສົ່ງຄືນ _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
ຈຳນວນເຕັມ miiphy_duplex(const var_na *devname, unsigned var_na addr)
{
	u16 bmcr, anlpar, adv;

#ຖ້າ defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	ຖ້າ (miiphy_is_1000base_x(devname, addr)) {
		/* 1000BASE-X */
		ຖ້າ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("1000BASE-X PHY AN duplex");
			goto miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	ຖ້າ (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		goto miiphy_read_failed;
	}
	ຖ້າ (btsr != 0xFFFF) {
		ຖ້າ (btsr & PHY_1000BTSR_1000FD) {
			ສົ່ງຄືນ FULL;
		} ບໍ່ດັ່ງນັ້ນ ຖ້າ (btsr & PHY_1000BTSR_1000HD) {
			ສົ່ງຄືນ HALF;
		}
	}
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	ຖ້າ (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		puts("PHY duplex");
		goto miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	ຖ້າ (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		ຖ້າ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			puts("PHY AN duplex");
			goto miiphy_read_failed;
		}

		ຖ້າ (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv duplex");
			goto miiphy_read_failed;
		}
		ສົ່ງຄືນ ((anlpar & adv) & (LPA_10FULL | LPA_100FULL)) ?
		    FULL : HALF;
	}
	/* Get speed from basic control settings. */
	ສົ່ງຄືນ (bmcr & BMCR_FULLDPLX) ? FULL : HALF;

miiphy_read_failed:
	printf(" read failed, assuming half duplex\n");
	ສົ່ງຄືນ HALF;
}

/*****************************************************************************
 *
 * Return 1 if PHY supports 1000BASE-X, 0 if PHY supports 10BASE-T/100BASE-TX/
 * 1000BASE-T, or on error.
 */
ຈຳນວນເຕັມ miiphy_is_1000base_x(const var_na *devname, unsigned var_na addr)
{
#ຖ້າ defined(CONFIG_PHY_GIGE)
	u16 exsr;

	ຖ້າ (miiphy_read(devname, addr, MII_ESTATUS, &exsr)) {
		printf("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		ສົ່ງຄືນ 0;
	}
	ສົ່ງຄືນ 0 != (exsr & (ESTATUS_1000XF | ESTATUS_1000XH));
#ບໍ່ດັ່ງນັ້ນ
	ສົ່ງຄືນ 0;
#endif
}
