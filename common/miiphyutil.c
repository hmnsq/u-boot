// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 */

/*
 * This provides a bit-banged interface to the ethernet MII management
 * channel.
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <dm.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <miiphy.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <phy.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/delay.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <asm/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/list.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <net.h>

/* local debug macro */
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ MII_DEBUG

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ debug
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MII_DEBUG
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug(fmt, ਦਲੀਲਾਂ...)	ਲਿਖੋ(fmt, ##args)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug(fmt, ਦਲੀਲਾਂ...)
#ਜੇ_ਅੰਤ /* MII_DEBUG */

ਸਥਿਰ LIST_ਸਿਰ(mii_devs);
ਸਥਿਰ ਬਣਤਰ mii_dev *current_mii;

/*
 * Lookup the mii_dev struct by the registered device name.
 */
ਬਣਤਰ mii_dev *miiphy_get_dev_by_name(ਅਚਲ ਅੱਖਰ *devname)
{
	ਬਣਤਰ list_ਸਿਰ *ਇੰਦਰਾਜ;
	ਬਣਤਰ mii_dev *dev;

	ਜੇ (!devname) {
		ਲਿਖੋ("NULL device name!\n");
		ਵਾਪਸ ਨਿਰਾ;
	}

	list_for_each(ਇੰਦਰਾਜ, &mii_devs) {
		dev = list_ਇੰਦਰਾਜ(ਇੰਦਰਾਜ, ਬਣਤਰ mii_dev, link);
		ਜੇ (strcmp(dev->name, devname) == 0)
			ਵਾਪਸ dev;
	}

	ਵਾਪਸ ਨਿਰਾ;
}

ਬਣਤਰ mii_dev *mdio_alloc(ਖਾਲੀ)
{
	ਬਣਤਰ mii_dev *bus;

	bus = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(*bus));
	ਜੇ (!bus)
		ਵਾਪਸ bus;

	memset(bus, 0, ਆਕਾਰ(*bus));

	/* initialize mii_dev struct fields */
	INIT_LIST_ਸਿਰ(&bus->link);

	ਵਾਪਸ bus;
}

ਖਾਲੀ mdio_free(ਬਣਤਰ mii_dev *bus)
{
	ਮੁਕਤ(bus);
}

ਪੂਰਨ mdio_ਰਜਿਸਟਰ(ਬਣਤਰ mii_dev *bus)
{
	ਜੇ (!bus || !bus->ਪਠ || !bus->ਲਿਖ)
		ਵਾਪਸ -1;

	/* check if we have unique name */
	ਜੇ (miiphy_get_dev_by_name(bus->name)) {
		ਲਿਖੋ("mdio_register: non unique device name '%s'\n",
			bus->name);
		ਵਾਪਸ -1;
	}

	/* add it to the list */
	list_ਜੋੜੋ_ਪੂਛ(&bus->link, &mii_devs);

	ਜੇ (!current_mii)
		current_mii = bus;

	ਵਾਪਸ 0;
}

ਪੂਰਨ mdio_ਰਜਿਸਟਰ_seq(ਬਣਤਰ mii_dev *bus, ਪੂਰਨ seq)
{
	ਪੂਰਨ ret;

	/* Setup a unique name for each mdio bus */
	ret = snprintf(bus->name, MDIO_NAME_LEN, "eth%d", seq);
	ਜੇ (ret < 0)
		ਵਾਪਸ ret;

	ਵਾਪਸ mdio_ਰਜਿਸਟਰ(bus);
}

ਪੂਰਨ mdio_unregister(ਬਣਤਰ mii_dev *bus)
{
	ਜੇ (!bus)
		ਵਾਪਸ 0;

	/* delete it from the list */
	list_del(&bus->link);

	ਜੇ (current_mii == bus)
		current_mii = ਨਿਰਾ;

	ਵਾਪਸ 0;
}

ਖਾਲੀ mdio_list_devices(ਖਾਲੀ)
{
	ਬਣਤਰ list_ਸਿਰ *ਇੰਦਰਾਜ;

	list_for_each(ਇੰਦਰਾਜ, &mii_devs) {
		ਪੂਰਨ i;
		ਬਣਤਰ mii_dev *bus = list_ਇੰਦਰਾਜ(ਇੰਦਰਾਜ, ਬਣਤਰ mii_dev, link);

		ਲਿਖੋ("%s:\n", bus->name);

		ਲਈ (i = 0; i < PHY_MAX_ADDR; i++) {
			ਬਣਤਰ phy_device *phydev = bus->phymap[i];

			ਜੇ (phydev) {
				ਲਿਖੋ("%x - %s", i, phydev->drv->name);

				ਜੇ (phydev->dev)
					ਲਿਖੋ(" <--> %s\n", phydev->dev->name);
				ਨਹੀਂਤਾਂ
					ਲਿਖੋ("\n");
			}
		}
	}
}

ਪੂਰਨ miiphy_ਸੈੱਟ_current_dev(ਅਚਲ ਅੱਖਰ *devname)
{
	ਬਣਤਰ mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname);
	ਜੇ (dev) {
		current_mii = dev;
		ਵਾਪਸ 0;
	}

	ਲਿਖੋ("No such device: %s\n", devname);

	ਵਾਪਸ 1;
}

ਬਣਤਰ mii_dev *mdio_get_current_dev(ਖਾਲੀ)
{
	ਵਾਪਸ current_mii;
}

ਬਣਤਰ list_ਸਿਰ *mdio_get_list_ਸਿਰ(ਖਾਲੀ)
{
	ਵਾਪਸ &mii_devs;
}

ਬਣਤਰ phy_device *mdio_phydev_for_ethname(ਅਚਲ ਅੱਖਰ *ethname)
{
	ਬਣਤਰ list_ਸਿਰ *ਇੰਦਰਾਜ;
	ਬਣਤਰ mii_dev *bus;

	list_for_each(ਇੰਦਰਾਜ, &mii_devs) {
		ਪੂਰਨ i;
		bus = list_ਇੰਦਰਾਜ(ਇੰਦਰਾਜ, ਬਣਤਰ mii_dev, link);

		ਲਈ (i = 0; i < PHY_MAX_ADDR; i++) {
			ਜੇ (!bus->phymap[i] || !bus->phymap[i]->dev)
				ਜਾਰੀ;

			ਜੇ (strcmp(bus->phymap[i]->dev->name, ethname) == 0)
				ਵਾਪਸ bus->phymap[i];
		}
	}

	ਲਿਖੋ("%s is not a known ethernet\n", ethname);
	ਵਾਪਸ ਨਿਰਾ;
}

ਅਚਲ ਅੱਖਰ *miiphy_get_current_dev(ਖਾਲੀ)
{
	ਜੇ (current_mii)
		ਵਾਪਸ current_mii->name;

	ਵਾਪਸ ਨਿਰਾ;
}

ਸਥਿਰ ਬਣਤਰ mii_dev *miiphy_get_ਸਰਗਰਮ_dev(ਅਚਲ ਅੱਖਰ *devname)
{
	/* If the current mii is the one we want, return it */
	ਜੇ (current_mii)
		ਜੇ (strcmp(current_mii->name, devname) == 0)
			ਵਾਪਸ current_mii;

	/* Otherwise, set the active one to the one we want */
	ਜੇ (miiphy_ਸੈੱਟ_current_dev(devname))
		ਵਾਪਸ ਨਿਰਾ;
	ਨਹੀਂਤਾਂ
		ਵਾਪਸ current_mii;
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
ਪੂਰਨ miiphy_read(ਅਚਲ ਅੱਖਰ *devname, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ addr, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ reg,
		 ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ *ਮੁੱਲ)
{
	ਬਣਤਰ mii_dev *bus;
	ਪੂਰਨ ret;

	bus = miiphy_get_ਸਰਗਰਮ_dev(devname);
	ਜੇ (!bus)
		ਵਾਪਸ 1;

	ret = bus->ਪਠ(bus, addr, MDIO_DEVAD_NONE, reg);
	ਜੇ (ret < 0)
		ਵਾਪਸ 1;

	*ਮੁੱਲ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ)ret;
	ਵਾਪਸ 0;
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
ਪੂਰਨ miiphy_write(ਅਚਲ ਅੱਖਰ *devname, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ addr, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ reg,
		  ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ ਮੁੱਲ)
{
	ਬਣਤਰ mii_dev *bus;

	bus = miiphy_get_ਸਰਗਰਮ_dev(devname);
	ਜੇ (bus)
		ਵਾਪਸ bus->ਲਿਖ(bus, addr, MDIO_DEVAD_NONE, reg, ਮੁੱਲ);

	ਵਾਪਸ 1;
}

/*****************************************************************************
 *
 * Print out list of registered MII capable devices.
 */
ਖਾਲੀ miiphy_listdev(ਖਾਲੀ)
{
	ਬਣਤਰ list_ਸਿਰ *ਇੰਦਰਾਜ;
	ਬਣਤਰ mii_dev *dev;

	puts("MII devices: ");
	list_for_each(ਇੰਦਰਾਜ, &mii_devs) {
		dev = list_ਇੰਦਰਾਜ(ਇੰਦਰਾਜ, ਬਣਤਰ mii_dev, link);
		ਲਿਖੋ("'%s' ", dev->name);
	}
	puts("\n");

	ਜੇ (current_mii)
		ਲਿਖੋ("Current device: '%s'\n", current_mii->name);
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
ਪੂਰਨ miiphy_info(ਅਚਲ ਅੱਖਰ *devname, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ addr, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *oui,
		 ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *model, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *rev)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ reg = 0;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ tmp;

	ਜੇ (miiphy_read(devname, addr, MII_PHYSID2, &tmp) != 0) {
		debug("PHY ID register 2 read failed\n");
		ਵਾਪਸ -1;
	}
	reg = tmp;

	debug("MII_PHYSID2 @ 0x%x = 0x%04x\n", addr, reg);

	ਜੇ (reg == 0xFFFF) {
		/* No physical device present at this address */
		ਵਾਪਸ -1;
	}

	ਜੇ (miiphy_read(devname, addr, MII_PHYSID1, &tmp) != 0) {
		debug("PHY ID register 1 read failed\n");
		ਵਾਪਸ -1;
	}
	reg |= tmp << 16;
	debug("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)((reg >> 4) & 0x0000003F);
	*rev = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(reg & 0x0000000F);
	ਵਾਪਸ 0;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_PHYLIB
/*****************************************************************************
 *
 * Reset the PHY.
 *
 * This API is deprecated. Use PHYLIB.
 *
 * Returns:
 *   0 on success
 */
ਪੂਰਨ miiphy_reset(ਅਚਲ ਅੱਖਰ *devname, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ addr)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ reg;
	ਪੂਰਨ timeout = 500;

	ਜੇ (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
		debug("PHY status read failed\n");
		ਵਾਪਸ -1;
	}
	ਜੇ (miiphy_write(devname, addr, MII_BMCR, reg | BMCR_RESET) != 0) {
		debug("PHY reset failed\n");
		ਵਾਪਸ -1;
	}
#ਜੇ CONFIG_PHY_RESET_DELAY > 0
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#ਜੇ_ਅੰਤ
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = 0x8000;
	ਜਦੋਂ (((reg & 0x8000) != 0) && timeout--) {
		ਜੇ (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			ਵਾਪਸ -1;
		}
		udelay(1000);
	}
	ਜੇ ((reg & 0x8000) == 0) {
		ਵਾਪਸ 0;
	} ਨਹੀਂਤਾਂ {
		puts("PHY reset timed out\n");
		ਵਾਪਸ -1;
	}
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ /* !PHYLIB */

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
ਪੂਰਨ miiphy_ਰਫਤਾਰ(ਅਚਲ ਅੱਖਰ *devname, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ addr)
{
	u16 bmcr, anlpar, adv;

#ਜੇ defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	ਜੇ (miiphy_is_1000base_x(devname, addr))
		ਵਾਪਸ _1000BASET;

	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	ਜੇ (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		ਲਿਖੋ("PHY 1000BT status");
		ਜਾਓ miiphy_read_failed;
	}
	ਜੇ (btsr != 0xFFFF &&
			(btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)))
		ਵਾਪਸ _1000BASET;
#ਜੇ_ਅੰਤ /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	ਜੇ (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		ਲਿਖੋ("PHY speed");
		ਜਾਓ miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	ਜੇ (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		ਜੇ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			ਲਿਖੋ("PHY AN speed");
			ਜਾਓ miiphy_read_failed;
		}

		ਜੇ (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv speed");
			ਜਾਓ miiphy_read_failed;
		}
		ਵਾਪਸ ((anlpar & adv) & LPA_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	ਵਾਪਸ (bmcr & BMCR_SPEED100) ? _100BASET : _10BASET;

miiphy_read_failed:
	ਲਿਖੋ(" read failed, assuming 10BASE-T\n");
	ਵਾਪਸ _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
ਪੂਰਨ miiphy_duplex(ਅਚਲ ਅੱਖਰ *devname, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ addr)
{
	u16 bmcr, anlpar, adv;

#ਜੇ defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	ਜੇ (miiphy_is_1000base_x(devname, addr)) {
		/* 1000BASE-X */
		ਜੇ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			ਲਿਖੋ("1000BASE-X PHY AN duplex");
			ਜਾਓ miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	ਜੇ (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		ਲਿਖੋ("PHY 1000BT status");
		ਜਾਓ miiphy_read_failed;
	}
	ਜੇ (btsr != 0xFFFF) {
		ਜੇ (btsr & PHY_1000BTSR_1000FD) {
			ਵਾਪਸ ਭਰਿਆ;
		} ਨਹੀਂਤਾਂ ਜੇ (btsr & PHY_1000BTSR_1000HD) {
			ਵਾਪਸ HALF;
		}
	}
#ਜੇ_ਅੰਤ /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	ਜੇ (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		puts("PHY duplex");
		ਜਾਓ miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	ਜੇ (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		ਜੇ (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			puts("PHY AN duplex");
			ਜਾਓ miiphy_read_failed;
		}

		ਜੇ (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv duplex");
			ਜਾਓ miiphy_read_failed;
		}
		ਵਾਪਸ ((anlpar & adv) & (LPA_10_ਭਰਿਆ | LPA_100_ਭਰਿਆ)) ?
		    ਭਰਿਆ : HALF;
	}
	/* Get speed from basic control settings. */
	ਵਾਪਸ (bmcr & BMCR_FULLDPLX) ? ਭਰਿਆ : HALF;

miiphy_read_failed:
	ਲਿਖੋ(" read failed, assuming half duplex\n");
	ਵਾਪਸ HALF;
}

/*****************************************************************************
 *
 * Return 1 if PHY supports 1000BASE-X, 0 if PHY supports 10BASE-T/100BASE-TX/
 * 1000BASE-T, or on error.
 */
ਪੂਰਨ miiphy_is_1000base_x(ਅਚਲ ਅੱਖਰ *devname, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ addr)
{
#ਜੇ defined(CONFIG_PHY_GIGE)
	u16 exsr;

	ਜੇ (miiphy_read(devname, addr, MII_ESTATUS, &exsr)) {
		ਲਿਖੋ("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		ਵਾਪਸ 0;
	}
	ਵਾਪਸ 0 != (exsr & (ESTATUS_1000XF | ESTATUS_1000XH));
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
}
