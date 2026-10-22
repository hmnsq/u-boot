// SPDX-License-Identifier: GPL-2.0+
/*
 * Most of this source has been derived from the Linux USB
 * project:
 *   (c) 1999-2002 Matthew Dharm (mdharm-usb@one-eyed-alien.net)
 *   (c) 2000 David L. Brown, Jr. (usb-storage@davidb.org)
 *   (c) 1999 Michael Gee (michael@linuxspecific.com)
 *   (c) 2000 Yggdrasil Computing, Inc.
 *
 *
 * Adapted for U-Boot:
 *   (C) Copyright 2001 Denis Peter, MPL AG Switzerland
 * Driver model conversion:
 *   (C) Copyright 2015 Google, Inc
 *
 * For BBB support (C) Copyright 2003
 * Gary Jennejohn, DENX Software Engineering <garyj@denx.de>
 *
 * BBB support based on /sys/dev/usb/umass.c from
 * FreeBSD.
 */

/* Note:
 * Currently only the CBI transport protocoll has been implemented, and it
 * is only tested with a TEAC USB Floppy. Other Massstorages with CBI or CB
 * transport protocoll may work as well.
 */
/*
 * New Note:
 * Support for USB Mass Storage Devices (BBB) has been added. It has
 * only been tested with USB memory sticks.
 */

#ಅನ್ತರ್ಭಾವ <blk.h>
#ಅನ್ತರ್ಭಾವ <bootdev.h>
#ಅನ್ತರ್ಭಾವ <command.h>
#ಅನ್ತರ್ಭಾವ <dm.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <memalign.h>
#ಅನ್ತರ್ಭಾವ <asm/byteorder.h>
#ಅನ್ತರ್ಭಾವ <asm/cache.h>
#ಅನ್ತರ್ಭಾವ <asm/processor.h>
#ಅನ್ತರ್ಭಾವ <dm/device-internal.h>
#ಅನ್ತರ್ಭಾವ <dm/lists.h>
#ಅನ್ತರ್ಭಾವ <linux/delay.h>

#ಅನ್ತರ್ಭಾವ <part.h>
#ಅನ್ತರ್ಭಾವ <usb.h>

#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು BBB_COMDAT_TRACE
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು BBB_XPORT_TRACE

#ಅನ್ತರ್ಭಾವ <scsi.h>
/* direction table -- this indicates the direction of the data
 * transfer for each command code -- a 1 indicates input
 */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ us_direction[256/8] = {
	0x28, 0x81, 0x14, 0x14, 0x20, 0x01, 0x90, 0x77,
	0x0C, 0x20, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#ವ್ಯಾಖ್ಯಾನಿಸು US_DIRECTION(x) ((us_direction[x>>3] >> (x & 7)) & 1)

ಸ್ಥಿರ ರಚನೆ scsi_cmd usb_ccb __aligned(ARCH_DMA_MINALIGN);
ಸ್ಥಿರ __u32 CBWTag;

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_max_devs; /* number of highest available usb device */

#ಆದರೆ !CONFIG_IS_ENABLED(BLK)
ಸ್ಥಿರ ರಚನೆ blk_desc usb_dev_desc[USB_MAX_STOR_DEV];
#ಆದರೆ_ಅಂತ್ಯ

ರಚನೆ us_ದತ್ತಾಂಶ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ (*trans_cmnd)(ರಚನೆ scsi_cmd *cb, ರಚನೆ us_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ);
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ (*trans_reset)(ರಚನೆ us_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ);

ರಚನೆ us_ದತ್ತಾಂಶ {
	ರಚನೆ usb_device *pusb_dev;	 /* this usb_device */

	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ	flags;			/* from filter initially */
#	define USB_READY	(1 << 0)
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ	ifnum;			/* interface number */
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ	ep_ಒಳ;			/* in endpoint */
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ	ep_ಹೊರಹೋಗು;			/* out ....... */
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ	ep_int;			/* interrupt . */
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ	subclass;		/* as in overview */
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ	protocol;		/* .............. */
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ	attention_ಮುಗಿದ;		/* force attn on first cmd */
	ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ	ip_ದತ್ತಾಂಶ;		/* interrupt data */
	ಪೂರ್ಣಾಂಕ		action;			/* what to do */
	ಪೂರ್ಣಾಂಕ		ip_wanted;		/* needed */
	ಪೂರ್ಣಾಂಕ		*irq_handle;		/* for USB int requests */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ	irqpipe;		/* pipe for release_irq */
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ	irqmaxp;		/* max packed for irq Pipe */
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ	irqinterval;		/* Intervall for IRQ Pipe */
	ರಚನೆ scsi_cmd	*srb;			/* current srb */
	trans_reset	transport_reset;	/* reset routine */
	trans_cmnd	transport;		/* transport routine */
	ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ	max_xfer_blk;		/* maximum transfer blocks */
	bool		cmd12;			/* use 12-byte commands (RBC/UFI) */
};

#ಆದರೆ !CONFIG_IS_ENABLED(BLK)
ಸ್ಥಿರ ರಚನೆ us_ದತ್ತಾಂಶ usb_stor[USB_MAX_STOR_DEV];
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು USB_STOR_TRANSPORT_GOOD	   0
#ವ್ಯಾಖ್ಯಾನಿಸು USB_STOR_TRANSPORT_FAILED -1
#ವ್ಯಾಖ್ಯಾನಿಸು USB_STOR_TRANSPORT_ERROR  -2

ಪೂರ್ಣಾಂಕ usb_stor_get_info(ರಚನೆ usb_device *dev, ರಚನೆ us_ದತ್ತಾಂಶ *us,
		      ರಚನೆ blk_desc *dev_desc);
ಪೂರ್ಣಾಂಕ usb_storage_probe(ರಚನೆ usb_device *dev, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ifnum,
		      ರಚನೆ us_ದತ್ತಾಂಶ *ss);
#ಆದರೆ CONFIG_IS_ENABLED(BLK)
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ usb_stor_read(ರಚನೆ udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, ಶೂನ್ಯ *ಬಫರ್);
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ usb_stor_write(ರಚನೆ udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಬಫರ್);
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ usb_stor_read(ರಚನೆ blk_desc *ಖಂಡ_dev, lbaint_t blknr,
				   lbaint_t blkcnt, ಶೂನ್ಯ *ಬಫರ್);
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ usb_stor_write(ರಚನೆ blk_desc *ಖಂಡ_dev, lbaint_t blknr,
				    lbaint_t blkcnt, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಬಫರ್);
#ಆದರೆ_ಅಂತ್ಯ
ಶೂನ್ಯ uhci_show_ತಾತ್ಕಾಲಿಕ_int_td(ಶೂನ್ಯ);

ಸ್ಥಿರ ಶೂನ್ಯ usb_show_ಪ್ರಗತಿ(ಶೂನ್ಯ)
{
	debug(".");
}

/*******************************************************************************
 * show info on storage devices; 'usb start/init' must be invoked earlier
 * as we only retrieve structures populated during devices initialization
 */
ಪೂರ್ಣಾಂಕ usb_stor_info(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = 0;
#ಆದರೆ CONFIG_IS_ENABLED(BLK)
	ರಚನೆ udevice *dev;

	ಗಾಗಿ (blk_ಮೊದಲ_device(UCLASS_USB, &dev);
	     dev;
	     blk_ಮುಂದಿನ_device(&dev)) {
		ರಚನೆ blk_desc *desc = dev_get_uclass_plat(dev);

		ಬರೆ("  Device %d: ", desc->devnum);
		dev_print(desc);
		ಎಣಿಕೆ_2++;
	}
#ಇಲ್ಲದಿದ್ದರೆ
	ಪೂರ್ಣಾಂಕ i;

	ಆದರೆ (usb_max_devs > 0) {
		ಗಾಗಿ (i = 0; i < usb_max_devs; i++) {
			ಬರೆ("  Device %d: ", i);
			dev_print(&usb_dev_desc[i]);
		}
		ಹಿಂತಿರುಗಿ 0;
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (!ಎಣಿಕೆ_2) {
		ಬರೆ("No storage devices, perhaps not 'usb start'ed..?\n");
		ಹಿಂತಿರುಗಿ 1;
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ usb_get_max_lun(ರಚನೆ us_ದತ್ತಾಂಶ *us)
{
	ಪೂರ್ಣಾಂಕ len;
	ALLOC_CACHE_ALIGN_ಬಫರ್(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ, ಫಲಿತಾಂಶ, 1);
	len = usb_control_ಸಂದೇಶ(us->pusb_dev,
			      usb_rcvctrlpipe(us->pusb_dev, 0),
			      US_BBB_GET_MAX_LUN,
			      USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_ಒಳ,
			      0, us->ifnum,
			      ಫಲಿತಾಂಶ, ಗಾತ್ರ(ಅಕ್ಷರ),
			      USB_CNTL_TIMEOUT * 5);
	debug("Get Max LUN -> len = %i, result = %i\n", len, (ಪೂರ್ಣಾಂಕ) *ಫಲಿತಾಂಶ);
	ಹಿಂತಿರುಗಿ (len > 0) ? *ಫಲಿತಾಂಶ : 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_stor_probe_device(ರಚನೆ usb_device *udev)
{
	ಪೂರ್ಣಾಂಕ lun, max_lun;

#ಆದರೆ CONFIG_IS_ENABLED(BLK)
	ರಚನೆ us_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ;
	ಪೂರ್ಣಾಂಕ ret;
#ಇಲ್ಲದಿದ್ದರೆ
	ಪೂರ್ಣಾಂಕ ಆರಂಭ;

	ಆದರೆ (udev == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ -ENOENT; /* no more devices available */
#ಆದರೆ_ಅಂತ್ಯ

	debug("\n\nProbing for storage\n");
#ಆದರೆ CONFIG_IS_ENABLED(BLK)
	/*
	 * We store the us_data in the mass storage device's plat. It
	 * is shared by all LUNs (block devices) attached to this mass storage
	 * device.
	 */
	ದತ್ತಾಂಶ = dev_get_plat(udev->dev);
	ಆದರೆ (!usb_storage_probe(udev, 0, ದತ್ತಾಂಶ))
		ಹಿಂತಿರುಗಿ 0;
	max_lun = usb_get_max_lun(ದತ್ತಾಂಶ);
	ಗಾಗಿ (lun = 0; lun <= max_lun; lun++) {
		ರಚನೆ blk_desc *blkdev;
		ರಚನೆ udevice *dev;
		ಅಕ್ಷರ str[10];

		snprintf(str, ಗಾತ್ರ(str), "lun%d", lun);
		ret = blk_create_devicef(udev->dev, "usb_storage_blk", str,
					 UCLASS_USB, usb_max_devs,
					 DEFAULT_BLKSZ, 0, &dev);
		ಆದರೆ (ret) {
			debug("Cannot bind driver\n");
			ಹಿಂತಿರುಗಿ ret;
		}

		blkdev = dev_get_uclass_plat(dev);
		blkdev->ಗುರಿ = 0xff;
		blkdev->lun = lun;

		ret = usb_stor_get_info(udev, ದತ್ತಾಂಶ, blkdev);
		ಆದರೆ (ret == 1) {
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		} ಇಲ್ಲದಿದ್ದರೆ {
			debug("usb_stor_get_info: Invalid device\n");
			ret = device_unbind(dev);
			ಆದರೆ (ret)
				ಹಿಂತಿರುಗಿ ret;
			ಮುಂದುವರಿ;
		}

		ret = blk_probe_or_unbind(dev);
		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ ret;

		ret = bootdev_ಸಿದ್ಧತೆ_for_sibling_blk(dev, "usb_bootdev");
		ಆದರೆ (ret) {
			ಪೂರ್ಣಾಂಕ ret2;

			ret2 = device_unbind(dev);
			ಆದರೆ (ret2)
				ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("bootdev", ret2);
			ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("bootdev", ret);
		}
	}
#ಇಲ್ಲದಿದ್ದರೆ
	/* We don't have space to even probe if we hit the maximum */
	ಆದರೆ (usb_max_devs == USB_MAX_STOR_DEV) {
		ಬರೆ("max USB Storage Device reached: %d stopping\n",
		       usb_max_devs);
		ಹಿಂತಿರುಗಿ -ENOSPC;
	}

	ಆದರೆ (!usb_storage_probe(udev, 0, &usb_stor[usb_max_devs]))
		ಹಿಂತಿರುಗಿ 0;

	/*
	 * OK, it's a storage device.  Iterate over its LUNs and populate
	 * usb_dev_desc'
	 */
	ಆರಂಭ = usb_max_devs;

	max_lun = usb_get_max_lun(&usb_stor[usb_max_devs]);
	ಗಾಗಿ (lun = 0; lun <= max_lun && usb_max_devs < USB_MAX_STOR_DEV;
	     lun++) {
		ರಚನೆ blk_desc *blkdev;

		blkdev = &usb_dev_desc[usb_max_devs];
		memset(blkdev, '\0', ಗಾತ್ರ(ರಚನೆ blk_desc));
		blkdev->uclass_id = UCLASS_USB;
		blkdev->devnum = usb_max_devs;
		blkdev->part_type = PART_TYPE_UNKNOWN;
		blkdev->ಗುರಿ = 0xff;
		blkdev->type = DEV_TYPE_UNKNOWN;
		blkdev->ಖಂಡ_read = usb_stor_read;
		blkdev->ಖಂಡ_write = usb_stor_write;
		blkdev->lun = lun;
		blkdev->priv = udev;

		ಆದರೆ (usb_stor_get_info(udev, &usb_stor[ಆರಂಭ],
				      &usb_dev_desc[usb_max_devs]) == 1) {
			debug("partype: %d\n", blkdev->part_type);
			part_init(blkdev);
			debug("partype: %d\n", blkdev->part_type);
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		}
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ usb_stor_reset(ಶೂನ್ಯ)
{
	usb_max_devs = 0;
}

/*******************************************************************************
 * scan the usb and reports device info
 * to the user if mode = 1
 * returns current device or -1 if no
 */
ಪೂರ್ಣಾಂಕ usb_stor_ಪರಿಶೀಲನೆ(ಪೂರ್ಣಾಂಕ ಬಹುಲಕ)
{
	ಆದರೆ (ಬಹುಲಕ == 1)
		ಬರೆ("       scanning usb for storage devices... ");

#ಆದರೆ !CONFIG_IS_ENABLED(DM_USB)
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ i;

	usb_disable_asynch(1); /* asynch transfer not allowed */

	usb_stor_reset();
	ಗಾಗಿ (i = 0; i < USB_MAX_DEVICE; i++) {
		ರಚನೆ usb_device *dev;

		dev = usb_get_dev_ಸೂಚ್ಯಂಕ(i); /* get device */
		debug("i=%d\n", i);
		ಆದರೆ (usb_stor_probe_device(dev))
			ನಿಲ್ಲಿಸು;
	} /* for */

	usb_disable_asynch(0); /* asynch transfer allowed */
#ಆದರೆ_ಅಂತ್ಯ
	ಬರೆ("%d Storage Device(s) found\n", usb_max_devs);
	ಆದರೆ (usb_max_devs > 0)
		ಹಿಂತಿರುಗಿ 0;
	ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_stor_irq(ರಚನೆ usb_device *dev)
{
	ರಚನೆ us_ದತ್ತಾಂಶ *us;
	us = (ರಚನೆ us_ದತ್ತಾಂಶ *)dev->privptr;

	ಆದರೆ (us->ip_wanted)
		us->ip_wanted = 0;
	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ	DEBUG

ಸ್ಥಿರ ಶೂನ್ಯ usb_show_srb(ರಚನೆ scsi_cmd *pccb)
{
	ಪೂರ್ಣಾಂಕ i;
	ಬರೆ("SRB: len %d datalen 0x%lX\n ", pccb->cmdlen, pccb->datalen);
	ಗಾಗಿ (i = 0; i < pccb->cmdlen; i++)
		ಬರೆ("%02X ", pccb->cmd[i]);
	ಬರೆ("\n");
}

ಸ್ಥಿರ ಶೂನ್ಯ display_int_ಸ್ಥಿತಿ(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ tmp)
{
	ಬರೆ("Status: %s %s %s %s %s %s %s\n",
		(tmp & USB_ST_ಸಕ್ರಿಯ) ? "Active" : "",
		(tmp & USB_ST_STALLED) ? "Stalled" : "",
		(tmp & USB_ST_BUF_ERR) ? "Buffer Error" : "",
		(tmp & USB_ST_BABBLE_DET) ? "Babble Det" : "",
		(tmp & USB_ST_NAK_REC) ? "NAKed" : "",
		(tmp & USB_ST_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ERR) ? "CRC Error" : "",
		(tmp & USB_ST_ಬಿಟ್_ERR) ? "Bitstuff Error" : "");
}
#ಆದರೆ_ಅಂತ್ಯ
/***********************************************************************
 * Data transfer routines
 ***********************************************************************/

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ us_one_transfer(ರಚನೆ us_ದತ್ತಾಂಶ *us, ಪೂರ್ಣಾಂಕ pipe, ಅಕ್ಷರ *buf, ಪೂರ್ಣಾಂಕ ಉದ್ದಳತೆ)
{
	ಪೂರ್ಣಾಂಕ max_ಗಾತ್ರ;
	ಪೂರ್ಣಾಂಕ this_xfer;
	ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ;
	ಪೂರ್ಣಾಂಕ partial;
	ಪೂರ್ಣಾಂಕ maxtry;
	ಪೂರ್ಣಾಂಕ stat;

	/* determine the maximum packet size for these transfers */
	max_ಗಾತ್ರ = usb_maxpacket(us->pusb_dev, pipe) * 16;

	/* while we have data left to transfer */
	ಆಗ (ಉದ್ದಳತೆ) {

		/* calculate how long this will be -- maximum or a remainder */
		this_xfer = ಉದ್ದಳತೆ > max_ಗಾತ್ರ ? max_ಗಾತ್ರ : ಉದ್ದಳತೆ;
		ಉದ್ದಳತೆ -= this_xfer;

		/* setup the retry counter */
		maxtry = 10;

		/* set up the transfer loop */
		ಮಾಡು {
			/* transfer the data */
			debug("Bulk xfer 0x%lx(%d) try #%d\n",
			      (ulong)map_ಗಮ್ಯ_sysmem(buf), this_xfer,
			      11 - maxtry);
			ಫಲಿತಾಂಶ = usb_bulk_ಸಂದೇಶ(us->pusb_dev, pipe, buf,
					      this_xfer, &partial,
					      USB_CNTL_TIMEOUT * 5);
			debug("bulk_msg returned %d xferred %d/%d\n",
			      ಫಲಿತಾಂಶ, partial, this_xfer);
			ಆದರೆ (us->pusb_dev->ಸ್ಥಿತಿ_2 != 0) {
				/* if we stall, we need to clear it before
				 * we go on
				 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DEBUG
				display_int_ಸ್ಥಿತಿ(us->pusb_dev->ಸ್ಥಿತಿ_2);
#ಆದರೆ_ಅಂತ್ಯ
				ಆದರೆ (us->pusb_dev->ಸ್ಥಿತಿ_2 & USB_ST_STALLED) {
					debug("stalled ->clearing endpoint" \
					      "halt for pipe 0x%x\n", pipe);
					stat = us->pusb_dev->ಸ್ಥಿತಿ_2;
					usb_clear_halt(us->pusb_dev, pipe);
					us->pusb_dev->ಸ್ಥಿತಿ_2 = stat;
					ಆದರೆ (this_xfer == partial) {
						debug("bulk transferred" \
						      "with error %lX," \
						      " but data ok\n",
						      us->pusb_dev->ಸ್ಥಿತಿ_2);
						ಹಿಂತಿರುಗಿ 0;
					}
					ಇಲ್ಲದಿದ್ದರೆ
						ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
				}
				ಆದರೆ (us->pusb_dev->ಸ್ಥಿತಿ_2 & USB_ST_NAK_REC) {
					debug("Device NAKed bulk_msg\n");
					ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
				}
				debug("bulk transferred with error");
				ಆದರೆ (this_xfer == partial) {
					debug(" %ld, but data ok\n",
					      us->pusb_dev->ಸ್ಥಿತಿ_2);
					ಹಿಂತಿರುಗಿ 0;
				}
				/* if our try counter reaches 0, bail out */
				debug(" %ld, data %d\n",
				      us->pusb_dev->ಸ್ಥಿತಿ_2, partial);
				ಆದರೆ (!maxtry--)
						ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
			}
			/* update to show what data was transferred */
			this_xfer -= partial;
			buf += partial;
			/* continue until this transfer is done */
		} ಆಗ (this_xfer);
	}

	/* if we get here, we're done and successful */
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_stor_BBB_reset(ರಚನೆ us_ದತ್ತಾಂಶ *us)
{
	ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ pipe;

	/*
	 * Reset recovery (5.3.4 in Universal Serial Bus Mass Storage Class)
	 *
	 * For Reset Recovery the host shall issue in the following order:
	 * a) a Bulk-Only Mass Storage Reset
	 * b) a Clear Feature HALT to the Bulk-In endpoint
	 * c) a Clear Feature HALT to the Bulk-Out endpoint
	 *
	 * This is done in 3 steps.
	 *
	 * If the reset doesn't succeed, the device should be port reset.
	 *
	 * This comment stolen from FreeBSD's /sys/dev/usb/umass.c.
	 */
	debug("BBB_reset\n");
	ಫಲಿತಾಂಶ = usb_control_ಸಂದೇಶ(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_BBB_RESET,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, ಖಾಲಿ, 0, USB_CNTL_TIMEOUT * 5);

	ಆದರೆ ((ಫಲಿತಾಂಶ < 0) && (us->pusb_dev->ಸ್ಥಿತಿ_2 & USB_ST_STALLED)) {
		debug("RESET:stall\n");
		ಹಿಂತಿರುಗಿ -1;
	}

	/* long wait for reset */
	mdelay(150);
	debug("BBB_reset result %d: status %lX reset\n",
	      ಫಲಿತಾಂಶ, us->pusb_dev->ಸ್ಥಿತಿ_2);
	pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_ಒಳ);
	ಫಲಿತಾಂಶ = usb_clear_halt(us->pusb_dev, pipe);
	/* long wait for reset */
	mdelay(150);
	debug("BBB_reset result %d: status %lX clearing IN endpoint\n",
	      ಫಲಿತಾಂಶ, us->pusb_dev->ಸ್ಥಿತಿ_2);
	/* long wait for reset */
	pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_ಹೊರಹೋಗು);
	ಫಲಿತಾಂಶ = usb_clear_halt(us->pusb_dev, pipe);
	mdelay(150);
	debug("BBB_reset result %d: status %lX clearing OUT endpoint\n",
	      ಫಲಿತಾಂಶ, us->pusb_dev->ಸ್ಥಿತಿ_2);
	debug("BBB_reset done\n");
	ಹಿಂತಿರುಗಿ 0;
}

/* FIXME: this reset function doesn't really reset the port, and it
 * should. Actually it should probably do what it's doing here, and
 * reset the port physically
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_stor_CB_reset(ರಚನೆ us_ದತ್ತಾಂಶ *us)
{
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ cmd[12];
	ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ;

	debug("CB_reset\n");
	memset(cmd, 0xff, ಗಾತ್ರ(cmd));
	cmd[0] = SCSI_SEND_DIAG;
	cmd[1] = 4;
	ಫಲಿತಾಂಶ = usb_control_ಸಂದೇಶ(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_CBI_ADSC,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, cmd, ಗಾತ್ರ(cmd),
				 USB_CNTL_TIMEOUT * 5);

	/* long wait for reset */
	mdelay(1500);
	debug("CB_reset result %d: status %lX clearing endpoint halt\n",
	      ಫಲಿತಾಂಶ, us->pusb_dev->ಸ್ಥಿತಿ_2);
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_ಒಳ));
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_ಹೊರಹೋಗು));

	debug("CB_reset done\n");
	ಹಿಂತಿರುಗಿ 0;
}

/*
 * Set up the command for a BBB device. Note that the actual SCSI
 * command is copied into cbw.CBWCDB.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_stor_BBB_comdat(ರಚನೆ scsi_cmd *srb, ರಚನೆ us_ದತ್ತಾಂಶ *us)
{
	ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ;
	ಪೂರ್ಣಾಂಕ actlen;
	ಪೂರ್ಣಾಂಕ dir_ಒಳ;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ pipe;
	ALLOC_CACHE_ALIGN_ಬಫರ್(ರಚನೆ umass_bbb_cbw, cbw, 1);

	dir_ಒಳ = US_DIRECTION(srb->cmd[0]);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BBB_COMDAT_TRACE
	ಬರೆ("dir %d lun %d cmdlen %d cmd %p datalen %lu pdata %p\n",
		dir_ಒಳ, srb->lun, srb->cmdlen, srb->cmd, srb->datalen,
		srb->pdata);
	ಆದರೆ (srb->cmdlen) {
		ಗಾಗಿ (ಫಲಿತಾಂಶ = 0; ಫಲಿತಾಂಶ < srb->cmdlen; ಫಲಿತಾಂಶ++)
			ಬರೆ("cmd[%d] %#x ", ಫಲಿತಾಂಶ, srb->cmd[ಫಲಿತಾಂಶ]);
		ಬರೆ("\n");
	}
#ಆದರೆ_ಅಂತ್ಯ
	/* sanity checks */
	ಆದರೆ (!(srb->cmdlen <= CBWCDBLENGTH)) {
		debug("usb_stor_BBB_comdat:cmdlen too large\n");
		ಹಿಂತಿರುಗಿ -1;
	}

	/* always OUT to the ep */
	pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_ಹೊರಹೋಗು);

	cbw->dCBWSignature = cpu_ಗಮ್ಯ_le32(CBWSIGNATURE);
	cbw->dCBWTag = cpu_ಗಮ್ಯ_le32(CBWTag++);
	cbw->d_CBWData_Transfer_ಉದ್ದಳತೆ = cpu_ಗಮ್ಯ_le32(srb->datalen);
	cbw->bCBWFlags = (dir_ಒಳ ? CBWFLAGS_ಒಳ : CBWFLAGS_ಹೊರಹೋಗು);
	cbw->bCBWLUN = srb->lun;
	cbw->bCDBLength = srb->cmdlen;
	/* copy the command data into the CBW command data buffer */
	/* DST SRC LEN!!! */

	ಸ್ಮರಣೆ_ನಕಲು(cbw->CBWCDB, srb->cmd, srb->cmdlen);
	ಫಲಿತಾಂಶ = usb_bulk_ಸಂದೇಶ(us->pusb_dev, pipe, cbw, UMASS_BBB_CBW_ಗಾತ್ರ,
			      &actlen, USB_CNTL_TIMEOUT * 5);
	ಆದರೆ (ಫಲಿತಾಂಶ < 0)
		debug("usb_stor_BBB_comdat:usb_bulk_msg error\n");
	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

/* FIXME: we also need a CBI_command which sets up the completion
 * interrupt, and waits for it
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_stor_CB_comdat(ರಚನೆ scsi_cmd *srb, ರಚನೆ us_ದತ್ತಾಂಶ *us)
{
	ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ = 0;
	ಪೂರ್ಣಾಂಕ dir_ಒಳ, retry;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ pipe;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಸ್ಥಿತಿ_2;

	retry = 5;
	dir_ಒಳ = US_DIRECTION(srb->cmd[0]);

	ಆದರೆ (dir_ಒಳ)
		pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_ಒಳ);
	ಇಲ್ಲದಿದ್ದರೆ
		pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_ಹೊರಹೋಗು);

	ಆಗ (retry--) {
		debug("CBI gets a command: Try %d\n", 5 - retry);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DEBUG
		usb_show_srb(srb);
#ಆದರೆ_ಅಂತ್ಯ
		/* let's send the command via the control pipe */
		ಫಲಿತಾಂಶ = usb_control_ಸಂದೇಶ(us->pusb_dev,
					 usb_sndctrlpipe(us->pusb_dev , 0),
					 US_CBI_ADSC,
					 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
					 0, us->ifnum,
					 srb->cmd, srb->cmdlen,
					 USB_CNTL_TIMEOUT * 5);
		debug("CB_transport: control msg returned %d, status %lX\n",
		      ಫಲಿತಾಂಶ, us->pusb_dev->ಸ್ಥಿತಿ_2);
		/* check the return code for the command */
		ಆದರೆ (ಫಲಿತಾಂಶ < 0) {
			ಆದರೆ (us->pusb_dev->ಸ್ಥಿತಿ_2 & USB_ST_STALLED) {
				ಸ್ಥಿತಿ_2 = us->pusb_dev->ಸ್ಥಿತಿ_2;
				debug(" stall during command found," \
				      " clear pipe\n");
				usb_clear_halt(us->pusb_dev,
					      usb_sndctrlpipe(us->pusb_dev, 0));
				us->pusb_dev->ಸ್ಥಿತಿ_2 = ಸ್ಥಿತಿ_2;
			}
			debug(" error during command %02X" \
			      " Stat = %lX\n", srb->cmd[0],
			      us->pusb_dev->ಸ್ಥಿತಿ_2);
			ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
		}
		/* transfer the data payload for this command, if one exists*/

		debug("CB_transport: control msg returned %d," \
		      " direction is %s to go 0x%lx\n", ಫಲಿತಾಂಶ,
		      dir_ಒಳ ? "IN" : "OUT", srb->datalen);
		ಆದರೆ (srb->datalen) {
			ಫಲಿತಾಂಶ = us_one_transfer(us, pipe, (ಅಕ್ಷರ *)srb->pdata,
						 srb->datalen);
			debug("CBI attempted to transfer data," \
			      " result is %d status %lX, len %d\n",
			      ಫಲಿತಾಂಶ, us->pusb_dev->ಸ್ಥಿತಿ_2,
				us->pusb_dev->act_len);
			ಆದರೆ (!(us->pusb_dev->ಸ್ಥಿತಿ_2 & USB_ST_NAK_REC))
				ನಿಲ್ಲಿಸು;
		} /* if (srb->datalen) */
		ಇಲ್ಲದಿದ್ದರೆ
			ನಿಲ್ಲಿಸು;
	}
	/* return result */

	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_stor_CBI_get_ಸ್ಥಿತಿ(ರಚನೆ scsi_cmd *srb, ರಚನೆ us_ದತ್ತಾಂಶ *us)
{
	ಪೂರ್ಣಾಂಕ timeout;

	us->ip_wanted = 1;
	usb_int_ಸಂದೇಶ(us->pusb_dev, us->irqpipe,
		    (ಶೂನ್ಯ *)&us->ip_ದತ್ತಾಂಶ, us->irqmaxp, us->irqinterval, false);
	timeout = 1000;
	ಆಗ (timeout--) {
		ಆದರೆ (us->ip_wanted == 0)
			ನಿಲ್ಲಿಸು;
		mdelay(10);
	}
	ಆದರೆ (us->ip_wanted) {
		ಬರೆ("	Did not get interrupt on CBI\n");
		us->ip_wanted = 0;
		ಹಿಂತಿರುಗಿ USB_STOR_TRANSPORT_ERROR;
	}
	debug("Got interrupt data 0x%x, transferred %d status 0x%lX\n",
	      us->ip_ದತ್ತಾಂಶ, us->pusb_dev->irq_act_len,
	      us->pusb_dev->irq_ಸ್ಥಿತಿ);
	/* UFI gives us ASC and ASCQ, like a request sense */
	ಆದರೆ (us->subclass == US_SC_UFI) {
		ಆದರೆ (srb->cmd[0] == SCSI_REQ_SENSE ||
		    srb->cmd[0] == SCSI_INQUIRY)
			ಹಿಂತಿರುಗಿ USB_STOR_TRANSPORT_GOOD; /* Good */
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (us->ip_ದತ್ತಾಂಶ)
			ಹಿಂತಿರುಗಿ USB_STOR_TRANSPORT_FAILED;
		ಇಲ್ಲದಿದ್ದರೆ
			ಹಿಂತಿರುಗಿ USB_STOR_TRANSPORT_GOOD;
	}
	/* otherwise, we interpret the data normally */
	ಆಯ್ಕೆ (us->ip_ದತ್ತಾಂಶ) {
	ಸ್ಥಿತಿ 0x0001:
		ಹಿಂತಿರುಗಿ USB_STOR_TRANSPORT_GOOD;
	ಸ್ಥಿತಿ 0x0002:
		ಹಿಂತಿರುಗಿ USB_STOR_TRANSPORT_FAILED;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಹಿಂತಿರುಗಿ USB_STOR_TRANSPORT_ERROR;
	}			/* switch */
	ಹಿಂತಿರುಗಿ USB_STOR_TRANSPORT_ERROR;
}

#ವ್ಯಾಖ್ಯಾನಿಸು USB_TRANSPORT_UNKNOWN_RETRY 5
#ವ್ಯಾಖ್ಯಾನಿಸು USB_TRANSPORT_NOT_READY_RETRY 10

/* clear a stall on an endpoint - special for BBB devices */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_stor_BBB_clear_endpt_stall(ರಚನೆ us_ದತ್ತಾಂಶ *us, __u8 endpt)
{
	/* ENDPOINT_HALT = 0, so set value to 0 */
	ಹಿಂತಿರುಗಿ usb_control_ಸಂದೇಶ(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
			       USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
			       endpt, ಖಾಲಿ, 0, USB_CNTL_TIMEOUT * 5);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_stor_BBB_transport(ರಚನೆ scsi_cmd *srb, ರಚನೆ us_ದತ್ತಾಂಶ *us)
{
	ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ, retry;
	ಪೂರ್ಣಾಂಕ dir_ಒಳ;
	ಪೂರ್ಣಾಂಕ actlen, ದತ್ತಾಂಶ_actlen;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ pipe, pipein, pipeout;
	ALLOC_CACHE_ALIGN_ಬಫರ್(ರಚನೆ umass_bbb_csw, csw, 1);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BBB_XPORT_TRACE
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ptr;
	ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ;
#ಆದರೆ_ಅಂತ್ಯ

	dir_ಒಳ = US_DIRECTION(srb->cmd[0]);

	/* COMMAND phase */
	debug("COMMAND phase\n");
	ಫಲಿತಾಂಶ = usb_stor_BBB_comdat(srb, us);
	ಆದರೆ (ಫಲಿತಾಂಶ < 0) {
		debug("failed to send CBW status %ld\n",
		      us->pusb_dev->ಸ್ಥಿತಿ_2);
		usb_stor_BBB_reset(us);
		ಹಿಂತಿರುಗಿ USB_STOR_TRANSPORT_FAILED;
	}
	ಆದರೆ (!(us->flags & USB_READY))
		mdelay(5);
	pipein = usb_rcvbulkpipe(us->pusb_dev, us->ep_ಒಳ);
	pipeout = usb_sndbulkpipe(us->pusb_dev, us->ep_ಹೊರಹೋಗು);
	/* DATA phase + error handling */
	ದತ್ತಾಂಶ_actlen = 0;
	/* no data, go immediately to the STATUS phase */
	ಆದರೆ (srb->datalen == 0)
		ಹೋಗು st;
	debug("DATA phase\n");
	ಆದರೆ (dir_ಒಳ)
		pipe = pipein;
	ಇಲ್ಲದಿದ್ದರೆ
		pipe = pipeout;

	ಫಲಿತಾಂಶ = usb_bulk_ಸಂದೇಶ(us->pusb_dev, pipe, srb->pdata, srb->datalen,
			      &ದತ್ತಾಂಶ_actlen, USB_CNTL_TIMEOUT * 5);
	/* special handling of STALL in DATA phase */
	ಆದರೆ ((ಫಲಿತಾಂಶ < 0) && (us->pusb_dev->ಸ್ಥಿತಿ_2 & USB_ST_STALLED)) {
		debug("DATA:stall\n");
		/* clear the STALL on the endpoint */
		ಫಲಿತಾಂಶ = usb_stor_BBB_clear_endpt_stall(us,
					dir_ಒಳ ? us->ep_ಒಳ : us->ep_ಹೊರಹೋಗು);
		ಆದರೆ (ಫಲಿತಾಂಶ >= 0)
			/* continue on to STATUS phase */
			ಹೋಗು st;
	}
	ಆದರೆ (ಫಲಿತಾಂಶ < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->ಸ್ಥಿತಿ_2);
		usb_stor_BBB_reset(us);
		ಹಿಂತಿರುಗಿ USB_STOR_TRANSPORT_FAILED;
	}
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BBB_XPORT_TRACE
	ಗಾಗಿ (ಸೂಚ್ಯಂಕ = 0; ಸೂಚ್ಯಂಕ < ದತ್ತಾಂಶ_actlen; ಸೂಚ್ಯಂಕ++)
		ಬರೆ("pdata[%d] %#x ", ಸೂಚ್ಯಂಕ, srb->pdata[ಸೂಚ್ಯಂಕ]);
	ಬರೆ("\n");
#ಆದರೆ_ಅಂತ್ಯ
	/* STATUS phase + error handling */
st:
	retry = 0;
again:
	debug("STATUS phase\n");
	ಫಲಿತಾಂಶ = usb_bulk_ಸಂದೇಶ(us->pusb_dev, pipein, csw, UMASS_BBB_CSW_ಗಾತ್ರ,
				&actlen, USB_CNTL_TIMEOUT*5);

	/* special handling of STALL in STATUS phase */
	ಆದರೆ ((ಫಲಿತಾಂಶ < 0) && (retry < 1) &&
	    (us->pusb_dev->ಸ್ಥಿತಿ_2 & USB_ST_STALLED)) {
		debug("STATUS:stall\n");
		/* clear the STALL on the endpoint */
		ಫಲಿತಾಂಶ = usb_stor_BBB_clear_endpt_stall(us, us->ep_ಒಳ);
		ಆದರೆ (ಫಲಿತಾಂಶ >= 0 && (retry++ < 1))
			/* do a retry */
			ಹೋಗು again;
	}
	ಆದರೆ (ಫಲಿತಾಂಶ < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->ಸ್ಥಿತಿ_2);
		usb_stor_BBB_reset(us);
		ಹಿಂತಿರುಗಿ USB_STOR_TRANSPORT_FAILED;
	}
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BBB_XPORT_TRACE
	ptr = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)csw;
	ಗಾಗಿ (ಸೂಚ್ಯಂಕ = 0; ಸೂಚ್ಯಂಕ < UMASS_BBB_CSW_ಗಾತ್ರ; ಸೂಚ್ಯಂಕ++)
		ಬರೆ("ptr[%d] %#x ", ಸೂಚ್ಯಂಕ, ptr[ಸೂಚ್ಯಂಕ]);
	ಬರೆ("\n");
#ಆದರೆ_ಅಂತ್ಯ
	/* misuse pipe to get the residue */
	pipe = le32_ಗಮ್ಯ_cpu(csw->dCSWDataResidue);
	ಆದರೆ (pipe == 0 && srb->datalen != 0 && srb->datalen - ದತ್ತಾಂಶ_actlen != 0)
		pipe = srb->datalen - ದತ್ತಾಂಶ_actlen;
	ಆದರೆ (CSWSIGNATURE != le32_ಗಮ್ಯ_cpu(csw->dCSWSignature)) {
		debug("!CSWSIGNATURE\n");
		usb_stor_BBB_reset(us);
		ಹಿಂತಿರುಗಿ USB_STOR_TRANSPORT_FAILED;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((CBWTag - 1) != le32_ಗಮ್ಯ_cpu(csw->dCSWTag)) {
		debug("!Tag\n");
		usb_stor_BBB_reset(us);
		ಹಿಂತಿರುಗಿ USB_STOR_TRANSPORT_FAILED;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (csw->bCSWStatus > CSWSTATUS_PHASE) {
		debug(">PHASE\n");
		usb_stor_BBB_reset(us);
		ಹಿಂತಿರುಗಿ USB_STOR_TRANSPORT_FAILED;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (csw->bCSWStatus == CSWSTATUS_PHASE) {
		debug("=PHASE\n");
		usb_stor_BBB_reset(us);
		ಹಿಂತಿರುಗಿ USB_STOR_TRANSPORT_FAILED;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ದತ್ತಾಂಶ_actlen > srb->datalen) {
		debug("transferred %dB instead of %ldB\n",
		      ದತ್ತಾಂಶ_actlen, srb->datalen);
		ಹಿಂತಿರುಗಿ USB_STOR_TRANSPORT_FAILED;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (csw->bCSWStatus == CSWSTATUS_FAILED) {
		debug("FAILED\n");
		ಹಿಂತಿರುಗಿ USB_STOR_TRANSPORT_FAILED;
	}

	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_stor_CB_transport(ರಚನೆ scsi_cmd *srb, ರಚನೆ us_ದತ್ತಾಂಶ *us)
{
	ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ, ಸ್ಥಿತಿ_2;
	ರಚನೆ scsi_cmd *psrb;
	ರಚನೆ scsi_cmd reqsrb;
	ಪೂರ್ಣಾಂಕ retry, notready;

	psrb = &reqsrb;
	ಸ್ಥಿತಿ_2 = USB_STOR_TRANSPORT_GOOD;
	retry = 0;
	notready = 0;
	/* issue the command */
do_retry:
	ಫಲಿತಾಂಶ = usb_stor_CB_comdat(srb, us);
	debug("command / Data returned %d, status %lX\n",
	      ಫಲಿತಾಂಶ, us->pusb_dev->ಸ್ಥಿತಿ_2);
	/* if this is an CBI Protocol, get IRQ */
	ಆದರೆ (us->protocol == US_PR_CBI) {
		ಸ್ಥಿತಿ_2 = usb_stor_CBI_get_ಸ್ಥಿತಿ(srb, us);
		/* if the status is error, report it */
		ಆದರೆ (ಸ್ಥಿತಿ_2 == USB_STOR_TRANSPORT_ERROR) {
			debug(" USB CBI Command Error\n");
			ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
		}
		srb->sense_buf[12] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(us->ip_ದತ್ತಾಂಶ >> 8);
		srb->sense_buf[13] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(us->ip_ದತ್ತಾಂಶ & 0xff);
		ಆದರೆ (!us->ip_ದತ್ತಾಂಶ) {
			/* if the status is good, report it */
			ಆದರೆ (ಸ್ಥಿತಿ_2 == USB_STOR_TRANSPORT_GOOD) {
				debug(" USB CBI Command Good\n");
				ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
			}
		}
	}
	/* do we have to issue an auto request? */
	/* HERE we have to check the result */
	ಆದರೆ ((ಫಲಿತಾಂಶ < 0) && !(us->pusb_dev->ಸ್ಥಿತಿ_2 & USB_ST_STALLED)) {
		debug("ERROR %lX\n", us->pusb_dev->ಸ್ಥಿತಿ_2);
		us->transport_reset(us);
		ಹಿಂತಿರುಗಿ USB_STOR_TRANSPORT_ERROR;
	}
	ಆದರೆ ((us->protocol == US_PR_CBI) &&
	    ((srb->cmd[0] == SCSI_REQ_SENSE) ||
	    (srb->cmd[0] == SCSI_INQUIRY))) {
		/* do not issue an autorequest after request sense */
		debug("No auto request and good\n");
		ಹಿಂತಿರುಗಿ USB_STOR_TRANSPORT_GOOD;
	}
	/* issue an request_sense */
	memset(&psrb->cmd[0], 0, 12);
	psrb->cmd[0] = SCSI_REQ_SENSE;
	psrb->cmd[1] = srb->lun << 5;
	psrb->cmd[4] = 18;
	psrb->datalen = 18;
	psrb->pdata = &srb->sense_buf[0];
	psrb->cmdlen = us->cmd12 ? 12 : 6;
	/* issue the command */
	ಫಲಿತಾಂಶ = usb_stor_CB_comdat(psrb, us);
	debug("auto request returned %d\n", ಫಲಿತಾಂಶ);
	/* if this is an CBI Protocol, get IRQ */
	ಆದರೆ (us->protocol == US_PR_CBI)
		ಸ್ಥಿತಿ_2 = usb_stor_CBI_get_ಸ್ಥಿತಿ(psrb, us);

	ಆದರೆ ((ಫಲಿತಾಂಶ < 0) && !(us->pusb_dev->ಸ್ಥಿತಿ_2 & USB_ST_STALLED)) {
		debug(" AUTO REQUEST ERROR %ld\n",
		      us->pusb_dev->ಸ್ಥಿತಿ_2);
		ಹಿಂತಿರುಗಿ USB_STOR_TRANSPORT_ERROR;
	}
	debug("autorequest returned 0x%02X 0x%02X 0x%02X 0x%02X\n",
	      srb->sense_buf[0], srb->sense_buf[2],
	      srb->sense_buf[12], srb->sense_buf[13]);
	/* Check the auto request result */
	ಆದರೆ ((srb->sense_buf[2] == 0) &&
	    (srb->sense_buf[12] == 0) &&
	    (srb->sense_buf[13] == 0)) {
		/* ok, no sense */
		ಹಿಂತಿರುಗಿ USB_STOR_TRANSPORT_GOOD;
	}

	/* Check the auto request result */
	ಆಯ್ಕೆ (srb->sense_buf[2]) {
	ಸ್ಥಿತಿ 0x01:
		/* Recovered Error */
		ಹಿಂತಿರುಗಿ USB_STOR_TRANSPORT_GOOD;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 0x02:
		/* Not Ready */
		ಆದರೆ (notready++ > USB_TRANSPORT_NOT_READY_RETRY) {
			ಬರೆ("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X (NOT READY)\n", srb->cmd[0],
				srb->sense_buf[0], srb->sense_buf[2],
				srb->sense_buf[12], srb->sense_buf[13]);
			ಹಿಂತಿರುಗಿ USB_STOR_TRANSPORT_FAILED;
		} ಇಲ್ಲದಿದ್ದರೆ {
			mdelay(100);
			ಹೋಗು do_retry;
		}
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಆದರೆ (retry++ > USB_TRANSPORT_UNKNOWN_RETRY) {
			ಬರೆ("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X\n", srb->cmd[0], srb->sense_buf[0],
				srb->sense_buf[2], srb->sense_buf[12],
				srb->sense_buf[13]);
			ಹಿಂತಿರುಗಿ USB_STOR_TRANSPORT_FAILED;
		} ಇಲ್ಲದಿದ್ದರೆ
			ಹೋಗು do_retry;
		ನಿಲ್ಲಿಸು;
	}
	ಹಿಂತಿರುಗಿ USB_STOR_TRANSPORT_FAILED;
}

ಸ್ಥಿರ ಶೂನ್ಯ usb_stor_ಗಣ_max_xfer_blk(ರಚನೆ usb_device *udev,
				      ರಚನೆ us_ದತ್ತಾಂಶ *us)
{
	/*
	 * Limit the total size of a transfer to 120 KB.
	 *
	 * Some devices are known to choke with anything larger. It seems like
	 * the problem stems from the fact that original IDE controllers had
	 * only an 8-bit register to hold the number of sectors in one transfer
	 * and even those couldn't handle a full 256 sectors.
	 *
	 * Because we want to make sure we interoperate with as many devices as
	 * possible, we will maintain a 240 sector transfer size limit for USB
	 * Mass Storage devices.
	 *
	 * Tests show that other operating have similar limits with Microsoft
	 * Windows 7 limiting transfers to 128 sectors for both USB2 and USB3
	 * and Apple Mac OS X 10.11 limiting transfers to 256 sectors for USB2
	 * and 2048 for USB3 devices.
	 */
	ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ blk = 240;

#ಆದರೆ CONFIG_IS_ENABLED(DM_USB)
	size_t ಗಾತ್ರ_2;
	ಪೂರ್ಣಾಂಕ ret;

	ret = usb_get_max_xfer_ಗಾತ್ರ(udev, (size_t *)&ಗಾತ್ರ_2);
	ಆದರೆ ((ret >= 0) && (ಗಾತ್ರ_2 < blk * 512))
		blk = ಗಾತ್ರ_2 / 512;
#ಆದರೆ_ಅಂತ್ಯ

	us->max_xfer_blk = blk;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_inquiry(ರಚನೆ scsi_cmd *srb, ರಚನೆ us_ದತ್ತಾಂಶ *ss)
{
	ಪೂರ್ಣಾಂಕ retry, i;
	retry = 5;
	ಮಾಡು {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_INQUIRY;
		srb->cmd[1] = srb->lun << 5;
		srb->cmd[4] = 36;
		srb->datalen = 36;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		i = ss->transport(srb, ss);
		debug("inquiry returns %d\n", i);
		ಆದರೆ (i == 0)
			ನಿಲ್ಲಿಸು;
	} ಆಗ (--retry);

	ಆದರೆ (!retry) {
		ಬರೆ("error in inquiry\n");
		ಹಿಂತಿರುಗಿ -1;
	}
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_request_sense(ರಚನೆ scsi_cmd *srb, ರಚನೆ us_ದತ್ತಾಂಶ *ss)
{
	ಅಕ್ಷರ *ptr;

	ptr = (ಅಕ್ಷರ *)srb->pdata;
	memset(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_REQ_SENSE;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[4] = 18;
	srb->datalen = 18;
	srb->pdata = &srb->sense_buf[0];
	srb->cmdlen = ss->cmd12 ? 12 : 6;
	ss->transport(srb, ss);
	debug("Request Sense returned %02X %02X %02X\n",
	      srb->sense_buf[2], srb->sense_buf[12],
	      srb->sense_buf[13]);
	srb->pdata = (uchar *)ptr;
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_test_unit_ready(ರಚನೆ scsi_cmd *srb, ರಚನೆ us_ದತ್ತಾಂಶ *ss)
{
	ಪೂರ್ಣಾಂಕ retries = 10;

	ಮಾಡು {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_TST_U_RDY;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 0;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		ಆದರೆ (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD) {
			ss->flags |= USB_READY;
			ಹಿಂತಿರುಗಿ 0;
		}
		usb_request_sense(srb, ss);
		/*
		 * Check the Key Code Qualifier, if it matches
		 * "Not Ready - medium not present"
		 * (the sense Key equals 0x2 and the ASC is 0x3a)
		 * return immediately as the medium being absent won't change
		 * unless there is a user action.
		 */
		ಆದರೆ ((srb->sense_buf[2] == 0x02) &&
		    (srb->sense_buf[12] == 0x3a))
			ಹಿಂತಿರುಗಿ -1;
		mdelay(100);
	} ಆಗ (retries--);

	ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_read_ಸಾಮರ್ಥ್ಯ(ರಚನೆ scsi_cmd *srb, ರಚನೆ us_ದತ್ತಾಂಶ *ss)
{
	ಪೂರ್ಣಾಂಕ retry;
	/* XXX retries */
	retry = 3;
	ಮಾಡು {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_RD_CAPAC;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 8;
		srb->cmdlen = ss->cmd12 ? 12 : 10;
		ಆದರೆ (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD)
			ಹಿಂತಿರುಗಿ 0;
	} ಆಗ (retry--);

	ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_read_10(ರಚನೆ scsi_cmd *srb, ರಚನೆ us_ದತ್ತಾಂಶ *ss,
		       ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಆರಂಭ, ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ ಖಂಡಗಳು)
{
	memset(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_READ10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) (ಆರಂಭ >> 24)) & 0xff;
	srb->cmd[3] = ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) (ಆರಂಭ >> 16)) & 0xff;
	srb->cmd[4] = ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) (ಆರಂಭ >> 8)) & 0xff;
	srb->cmd[5] = ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) (ಆರಂಭ)) & 0xff;
	srb->cmd[7] = ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) (ಖಂಡಗಳು >> 8)) & 0xff;
	srb->cmd[8] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) ಖಂಡಗಳು & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("read10: start %lx blocks %x\n", ಆರಂಭ, ಖಂಡಗಳು);
	ಹಿಂತಿರುಗಿ ss->transport(srb, ss);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_write_10(ರಚನೆ scsi_cmd *srb, ರಚನೆ us_ದತ್ತಾಂಶ *ss,
			ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಆರಂಭ, ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ ಖಂಡಗಳು)
{
	memset(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_WRITE10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) (ಆರಂಭ >> 24)) & 0xff;
	srb->cmd[3] = ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) (ಆರಂಭ >> 16)) & 0xff;
	srb->cmd[4] = ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) (ಆರಂಭ >> 8)) & 0xff;
	srb->cmd[5] = ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) (ಆರಂಭ)) & 0xff;
	srb->cmd[7] = ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) (ಖಂಡಗಳು >> 8)) & 0xff;
	srb->cmd[8] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) ಖಂಡಗಳು & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("write10: start %lx blocks %x\n", ಆರಂಭ, ಖಂಡಗಳು);
	ಹಿಂತಿರುಗಿ ss->transport(srb, ss);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USB_BIN_FIXUP
/*
 * Some USB storage devices queried for SCSI identification data respond with
 * binary strings, which if output to the console freeze the terminal. The
 * workaround is to modify the vendor and product strings read from such
 * device with proper values (as reported by 'usb info').
 *
 * Vendor and product length limits are taken from the definition of
 * struct blk_desc in include/part.h.
 */
ಸ್ಥಿರ ಶೂನ್ಯ usb_bin_fixup(ರಚನೆ usb_device_descriptor descriptor,
				ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ vendor[],
				ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಗುಣಲಬ್ಧ[]) {
	ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ max_vendor_len = 40;
	ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ max_ಗುಣಲಬ್ಧ_len = 20;
	ಆದರೆ (descriptor.idVendor == 0x0424 && descriptor.id_ಗುಣಲಬ್ಧ_2 == 0x223a) {
		strncpy((ಅಕ್ಷರ *)vendor, "SMSC", max_vendor_len);
		strncpy((ಅಕ್ಷರ *)ಗುಣಲಬ್ಧ, "Flash Media Cntrller",
			max_ಗುಣಲಬ್ಧ_len);
	}
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USB_BIN_FIXUP */

#ಆದರೆ CONFIG_IS_ENABLED(BLK)
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ usb_stor_read(ರಚನೆ udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, ಶೂನ್ಯ *ಬಫರ್)
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ usb_stor_read(ರಚನೆ blk_desc *ಖಂಡ_dev, lbaint_t blknr,
				   lbaint_t blkcnt, ಶೂನ್ಯ *ಬಫರ್)
#ಆದರೆ_ಅಂತ್ಯ
{
	lbaint_t ಆರಂಭ, blks;
	uintptr_t buf_addr;
	ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ smallblks;
	ರಚನೆ usb_device *udev;
	ರಚನೆ us_ದತ್ತಾಂಶ *ss;
	ಪೂರ್ಣಾಂಕ retry;
	ರಚನೆ scsi_cmd *srb = &usb_ccb;
#ಆದರೆ CONFIG_IS_ENABLED(BLK)
	ರಚನೆ blk_desc *ಖಂಡ_dev;
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (blkcnt == 0)
		ಹಿಂತಿರುಗಿ 0;
	/* Setup  device */
#ಆದರೆ CONFIG_IS_ENABLED(BLK)
	ಖಂಡ_dev = dev_get_uclass_plat(dev);
	udev = dev_get_ಪೋಷಕ_priv(dev_get_ಪೋಷಕ(dev));
	debug("\nusb_read: udev %d\n", ಖಂಡ_dev->devnum);
#ಇಲ್ಲದಿದ್ದರೆ
	debug("\nusb_read: udev %d\n", ಖಂಡ_dev->devnum);
	udev = usb_dev_desc[ಖಂಡ_dev->devnum].priv;
	ಆದರೆ (!udev) {
		debug("%s: No device\n", __func__);
		ಹಿಂತಿರುಗಿ 0;
	}
#ಆದರೆ_ಅಂತ್ಯ
	ss = (ರಚನೆ us_ದತ್ತಾಂಶ *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_ಬೀಗ_async(udev, 1);
	srb->lun = ಖಂಡ_dev->lun;
	buf_addr = (uintptr_t)ಬಫರ್;
	ಆರಂಭ = blknr;
	blks = blkcnt;

	debug("\nusb_read: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      ಖಂಡ_dev->devnum, ಆರಂಭ, blks, buf_addr);

	ಮಾಡು {
		/* XXX need some comment here */
		retry = 2;
		srb->pdata = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)buf_addr;
		ಆದರೆ (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		ಇಲ್ಲದಿದ್ದರೆ
			smallblks = (ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ) blks;
retry_it:
		ಆದರೆ (smallblks == ss->max_xfer_blk)
			usb_show_ಪ್ರಗತಿ();
		srb->datalen = ಖಂಡ_dev->blksz * smallblks;
		srb->pdata = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)buf_addr;
		ಆದರೆ (usb_read_10(srb, ss, ಆರಂಭ, smallblks)) {
			debug("Read ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			ಆದರೆ (retry--)
				ಹೋಗು retry_it;
			blkcnt -= blks;
			ನಿಲ್ಲಿಸು;
		}
		ಆರಂಭ += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} ಆಗ (blks != 0);

	debug("usb_read: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      ಆರಂಭ, smallblks, buf_addr);

	usb_ಬೀಗ_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	ಆದರೆ (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	ಹಿಂತಿರುಗಿ blkcnt;
}

#ಆದರೆ CONFIG_IS_ENABLED(BLK)
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ usb_stor_write(ರಚನೆ udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಬಫರ್)
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ usb_stor_write(ರಚನೆ blk_desc *ಖಂಡ_dev, lbaint_t blknr,
				    lbaint_t blkcnt, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಬಫರ್)
#ಆದರೆ_ಅಂತ್ಯ
{
	lbaint_t ಆರಂಭ, blks;
	uintptr_t buf_addr;
	ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ smallblks;
	ರಚನೆ usb_device *udev;
	ರಚನೆ us_ದತ್ತಾಂಶ *ss;
	ಪೂರ್ಣಾಂಕ retry;
	ರಚನೆ scsi_cmd *srb = &usb_ccb;
#ಆದರೆ CONFIG_IS_ENABLED(BLK)
	ರಚನೆ blk_desc *ಖಂಡ_dev;
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (blkcnt == 0)
		ಹಿಂತಿರುಗಿ 0;

	/* Setup  device */
#ಆದರೆ CONFIG_IS_ENABLED(BLK)
	ಖಂಡ_dev = dev_get_uclass_plat(dev);
	udev = dev_get_ಪೋಷಕ_priv(dev_get_ಪೋಷಕ(dev));
	debug("\nusb_read: udev %d\n", ಖಂಡ_dev->devnum);
#ಇಲ್ಲದಿದ್ದರೆ
	debug("\nusb_read: udev %d\n", ಖಂಡ_dev->devnum);
	udev = usb_dev_desc[ಖಂಡ_dev->devnum].priv;
	ಆದರೆ (!udev) {
		debug("%s: No device\n", __func__);
		ಹಿಂತಿರುಗಿ 0;
	}
#ಆದರೆ_ಅಂತ್ಯ
	ss = (ರಚನೆ us_ದತ್ತಾಂಶ *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_ಬೀಗ_async(udev, 1);

	srb->lun = ಖಂಡ_dev->lun;
	buf_addr = (uintptr_t)ಬಫರ್;
	ಆರಂಭ = blknr;
	blks = blkcnt;

	debug("\nusb_write: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      ಖಂಡ_dev->devnum, ಆರಂಭ, blks, buf_addr);

	ಮಾಡು {
		/* If write fails retry for max retry count else
		 * return with number of blocks written successfully.
		 */
		retry = 2;
		srb->pdata = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)buf_addr;
		ಆದರೆ (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		ಇಲ್ಲದಿದ್ದರೆ
			smallblks = (ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ) blks;
retry_it:
		ಆದರೆ (smallblks == ss->max_xfer_blk)
			usb_show_ಪ್ರಗತಿ();
		srb->datalen = ಖಂಡ_dev->blksz * smallblks;
		srb->pdata = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)buf_addr;
		ಆದರೆ (usb_write_10(srb, ss, ಆರಂಭ, smallblks)) {
			debug("Write ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			ಆದರೆ (retry--)
				ಹೋಗು retry_it;
			blkcnt -= blks;
			ನಿಲ್ಲಿಸು;
		}
		ಆರಂಭ += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} ಆಗ (blks != 0);

	debug("usb_write: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      ಆರಂಭ, smallblks, buf_addr);

	usb_ಬೀಗ_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	ಆದರೆ (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	ಹಿಂತಿರುಗಿ blkcnt;

}

/* Probe to see if a new device is actually a Storage device */
ಪೂರ್ಣಾಂಕ usb_storage_probe(ರಚನೆ usb_device *dev, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ifnum,
		      ರಚನೆ us_ದತ್ತಾಂಶ *ss)
{
	ರಚನೆ usb_interface *iface;
	ಪೂರ್ಣಾಂಕ i;
	ರಚನೆ usb_endpoint_descriptor *ep_desc;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ flags = 0;

	/* let's examine the device now */
	iface = &dev->config.if_desc[ifnum];

	ಆದರೆ (dev->descriptor.bDeviceClass != 0 ||
			iface->desc.bInterfaceClass != USB_CLASS_MASS_STORAGE ||
			iface->desc.bInterfaceSubClass < US_SC_MIN ||
			iface->desc.bInterfaceSubClass > US_SC_MAX) {
		debug("Not mass storage\n");
		/* if it's not a mass storage, we go no further */
		ಹಿಂತಿರುಗಿ 0;
	}

	memset(ss, 0, ಗಾತ್ರ(ರಚನೆ us_ದತ್ತಾಂಶ));

	/* At this point, we know we've got a live one */
	debug("\n\nUSB Mass Storage device detected\n");

	/* Initialize the us_data structure with some useful info */
	ss->flags = flags;
	ss->ifnum = ifnum;
	ss->pusb_dev = dev;
	ss->attention_ಮುಗಿದ = 0;
	ss->subclass = iface->desc.bInterfaceSubClass;
	ss->protocol = iface->desc.bInterfaceProtocol;

	/* set the handler pointers based on the protocol */
	debug("Transport: ");
	ಆಯ್ಕೆ (ss->protocol) {
	ಸ್ಥಿತಿ US_PR_CB:
		debug("Control/Bulk\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ US_PR_CBI:
		debug("Control/Bulk/Interrupt\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ US_PR_BULK:
		debug("Bulk/Bulk/Bulk\n");
		ss->transport = usb_stor_BBB_transport;
		ss->transport_reset = usb_stor_BBB_reset;
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಬರೆ("USB Storage Transport unknown / not yet implemented\n");
		ಹಿಂತಿರುಗಿ 0;
		ನಿಲ್ಲಿಸು;
	}

	/*
	 * We are expecting a minimum of 2 endpoints - in and out (bulk).
	 * An optional interrupt is OK (necessary for CBI protocol).
	 * We will ignore any others.
	 */
	ಗಾಗಿ (i = 0; i < iface->desc.b_ಅಂಕೆ_Endpoints; i++) {
		ep_desc = &iface->ep_desc[i];
		/* is it an BULK endpoint? */
		ಆದರೆ ((ep_desc->bm_ಗುಣಲಕ್ಷಣಗಳು &
		     USB_ENDPOINT_XFERTYPE_ಮುಖವಾಡ) == USB_ENDPOINT_XFER_BULK) {
			ಆದರೆ (ep_desc->b_Endpoint_ವಿಳಾಸ & USB_DIR_ಒಳ)
				ss->ep_ಒಳ = ep_desc->b_Endpoint_ವಿಳಾಸ &
						USB_ENDPOINT_ಸಂಖ್ಯೆ_ಮುಖವಾಡ;
			ಇಲ್ಲದಿದ್ದರೆ
				ss->ep_ಹೊರಹೋಗು =
					ep_desc->b_Endpoint_ವಿಳಾಸ &
					USB_ENDPOINT_ಸಂಖ್ಯೆ_ಮುಖವಾಡ;
		}

		/* is it an interrupt endpoint? */
		ಆದರೆ ((ep_desc->bm_ಗುಣಲಕ್ಷಣಗಳು &
		     USB_ENDPOINT_XFERTYPE_ಮುಖವಾಡ) == USB_ENDPOINT_XFER_INT) {
			ss->ep_int = ep_desc->b_Endpoint_ವಿಳಾಸ &
						USB_ENDPOINT_ಸಂಖ್ಯೆ_ಮುಖವಾಡ;
			ss->irqinterval = ep_desc->bInterval;
		}
	}
	debug("Endpoints In %d Out %d Int %d\n",
	      ss->ep_ಒಳ, ss->ep_ಹೊರಹೋಗು, ss->ep_int);

	/* Do some basic sanity checks, and bail if we find a problem */
	ಆದರೆ (usb_ಗಣ_interface(dev, iface->desc.b_Interface_ಸಂಖ್ಯೆ, 0) ||
	    !ss->ep_ಒಳ || !ss->ep_ಹೊರಹೋಗು ||
	    (ss->protocol == US_PR_CBI && ss->ep_int == 0)) {
		debug("Problems with device\n");
		ಹಿಂತಿರುಗಿ 0;
	}
	/* set class specific stuff */
	/* We only handle certain protocols.  Currently, these are
	 * the only ones.
	 * The SFF8070 accepts the requests used in u-boot
	 */
	ಆದರೆ (ss->subclass != US_SC_UFI && ss->subclass != US_SC_SCSI &&
	    ss->subclass != US_SC_8070) {
		ಬರೆ("Sorry, protocol %d not yet supported.\n", ss->subclass);
		ಹಿಂತಿರುಗಿ 0;
	}

	/* UFI uses 12-byte commands (like RBC, unlike SCSI) */
	ಆದರೆ (ss->subclass == US_SC_UFI)
		ss->cmd12 = true;

	ಆದರೆ (ss->ep_int) {
		/* we had found an interrupt endpoint, prepare irq pipe
		 * set up the IRQ pipe and handler
		 */
		ss->irqinterval = (ss->irqinterval > 0) ? ss->irqinterval : 255;
		ss->irqpipe = usb_rcvintpipe(ss->pusb_dev, ss->ep_int);
		ss->irqmaxp = usb_maxpacket(dev, ss->irqpipe);
		dev->irq_handle = usb_stor_irq;
	}

	/* Set the maximum transfer size per host controller setting */
	usb_stor_ಗಣ_max_xfer_blk(dev, ss);

	dev->privptr = (ಶೂನ್ಯ *)ss;
	ಹಿಂತಿರುಗಿ 1;
}

ಪೂರ್ಣಾಂಕ usb_stor_get_info(ರಚನೆ usb_device *dev, ರಚನೆ us_ದತ್ತಾಂಶ *ss,
		      ರಚನೆ blk_desc *dev_desc)
{
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ perq, modi;
	ALLOC_CACHE_ALIGN_ಬಫರ್(u32, cap, 2);
	ALLOC_CACHE_ALIGN_ಬಫರ್(u8, usb_stor_buf, 36);
	u32 ಸಾಮರ್ಥ್ಯ, blksz;
	ರಚನೆ scsi_cmd *pccb = &usb_ccb;

	pccb->pdata = usb_stor_buf;

	dev_desc->ಗುರಿ = dev->devnum;
	pccb->lun = dev_desc->lun;
	debug(" address %d\n", dev_desc->ಗುರಿ);

	ಆದರೆ (usb_inquiry(pccb, ss)) {
		debug("%s: usb_inquiry() failed\n", __func__);
		ಹಿಂತಿರುಗಿ -1;
	}

	perq = usb_stor_buf[0];
	modi = usb_stor_buf[1];

	/*
	 * Skip unknown devices (0x1f) and enclosure service devices (0x0d),
	 * they would not respond to test_unit_ready .
	 */
	ಆದರೆ (((perq & 0x1f) == 0x1f) || ((perq & 0x1f) == 0x0d)) {
		debug("%s: unknown/unsupported device\n", __func__);
		ಹಿಂತಿರುಗಿ 0;
	}
	ಆದರೆ ((modi&0x80) == 0x80) {
		/* drive is removable */
		dev_desc->removable = 1;
	}
	ಸ್ಮರಣೆ_ನಕಲು(dev_desc->vendor, (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *)&usb_stor_buf[8], 8);
	ಸ್ಮರಣೆ_ನಕಲು(dev_desc->ಗುಣಲಬ್ಧ, (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *)&usb_stor_buf[16], 16);
	ಸ್ಮರಣೆ_ನಕಲು(dev_desc->revision, (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *)&usb_stor_buf[32], 4);
	dev_desc->vendor[8] = 0;
	dev_desc->ಗುಣಲಬ್ಧ[16] = 0;
	dev_desc->revision[4] = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USB_BIN_FIXUP
	usb_bin_fixup(dev->descriptor, (uchar *)dev_desc->vendor,
		      (uchar *)dev_desc->ಗುಣಲಬ್ಧ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USB_BIN_FIXUP */
	debug("ISO Vers %X, Response Data %X\n", usb_stor_buf[2],
	      usb_stor_buf[3]);
	ಆದರೆ (usb_test_unit_ready(pccb, ss)) {
		ಬರೆ("Device NOT ready\n"
		       "   Request Sense returned %02X %02X %02X\n",
		       pccb->sense_buf[2], pccb->sense_buf[12],
		       pccb->sense_buf[13]);
		ಆದರೆ (dev_desc->removable == 1)
			dev_desc->type = perq;
		ಹಿಂತಿರುಗಿ 0;
	}
	pccb->pdata = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)cap;
	memset(pccb->pdata, 0, 8);
	ಆದರೆ (usb_read_ಸಾಮರ್ಥ್ಯ(pccb, ss) != 0) {
		ಬರೆ("READ_CAP ERROR\n");
		ss->flags &= ~USB_READY;
		cap[0] = 2880;
		cap[1] = 0x200;
	}
	debug("Read Capacity returns: 0x%08x, 0x%08x\n", cap[0], cap[1]);
#ಆದರೆ 0
	ಆದರೆ (cap[0] > (0x200000 * 10)) /* greater than 10 GByte */
		cap[0] >>= 16;

	cap[0] = cpu_ಗಮ್ಯ_be32(cap[0]);
	cap[1] = cpu_ಗಮ್ಯ_be32(cap[1]);
#ಆದರೆ_ಅಂತ್ಯ

	ಸಾಮರ್ಥ್ಯ = be32_ಗಮ್ಯ_cpu(cap[0]) + 1;
	blksz = be32_ಗಮ್ಯ_cpu(cap[1]);

	debug("Capacity = 0x%08x, blocksz = 0x%08x\n", ಸಾಮರ್ಥ್ಯ, blksz);
	dev_desc->lba = ಸಾಮರ್ಥ್ಯ;
	dev_desc->blksz = blksz;
	dev_desc->log2blksz = LOG2(dev_desc->blksz);
	dev_desc->type = perq;
	debug(" address %d\n", dev_desc->ಗುರಿ);

	ಹಿಂತಿರುಗಿ 1;
}

#ಆದರೆ CONFIG_IS_ENABLED(DM_USB)

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ usb_mass_storage_probe(ರಚನೆ udevice *dev)
{
	ರಚನೆ usb_device *udev = dev_get_ಪೋಷಕ_priv(dev);
	ಪೂರ್ಣಾಂಕ ret;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	ret = usb_stor_probe_device(udev);
	usb_disable_asynch(0); /* asynch transfer allowed */

	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ udevice_id usb_mass_storage_ids[] = {
	{ .compatible = "usb-mass-storage" },
	{ }
};

U_BOOT_DRIVER(usb_mass_storage) = {
	.name	= "usb_mass_storage",
	.id	= UCLASS_MASS_STORAGE,
	.ದ_ಹೊಂದಿಕೆ = usb_mass_storage_ids,
	.probe = usb_mass_storage_probe,
#ಆದರೆ CONFIG_IS_ENABLED(BLK)
	.plat_auto	= ಗಾತ್ರ(ರಚನೆ us_ದತ್ತಾಂಶ),
#ಆದರೆ_ಅಂತ್ಯ
};

UCLASS_DRIVER(usb_mass_storage) = {
	.id		= UCLASS_MASS_STORAGE,
	.name		= "usb_mass_storage",
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ usb_device_id mass_storage_id_ಕೋಷ್ಟಕ[] = {
	{
		.ಹೊಂದಿಕೆ_flags = USB_DEVICE_ID_ಹೊಂದಿಕೆ_INT_CLASS,
		.bInterfaceClass = USB_CLASS_MASS_STORAGE
	},
	{ }		/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_mass_storage, mass_storage_id_ಕೋಷ್ಟಕ);
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ CONFIG_IS_ENABLED(BLK)
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ blk_ops usb_storage_ops = {
	.ಪಠ	= usb_stor_read,
	.ಲಿಖ	= usb_stor_write,
};

U_BOOT_DRIVER(usb_storage_blk) = {
	.name		= "usb_storage_blk",
	.id		= UCLASS_BLK,
	.ops		= &usb_storage_ops,
};
#ಇಲ್ಲದಿದ್ದರೆ
U_BOOT_LEGACY_BLK(usb) = {
	.uclass_idname	= "usb",
	.uclass_id	= UCLASS_USB,
	.max_devs	= USB_MAX_STOR_DEV,
	.desc		= usb_dev_desc,
};
#ಆದರೆ_ಅಂತ್ಯ
