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

#ਅਨ੍ਤਰ੍ਭਾਵ <blk.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <bootdev.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <command.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mapmem.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <memalign.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/byteorder.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/cache.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/processor.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm/device-internal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm/lists.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/delay.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <part.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <usb.h>

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ BBB_COMDAT_TRACE
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ BBB_XPORT_TRACE

#ਅਨ੍ਤਰ੍ਭਾਵ <scsi.h>
/* direction table -- this indicates the direction of the data
 * transfer for each command code -- a 1 indicates input
 */
ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ us_direction[256/8] = {
	0x28, 0x81, 0x14, 0x14, 0x20, 0x01, 0x90, 0x77,
	0x0C, 0x20, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ US_DIRECTION(x) ((us_direction[x>>3] >> (x & 7)) & 1)

ਸਥਿਰ ਬਣਤਰ scsi_cmd usb_ccb __aligned(ARCH_DMA_MINALIGN);
ਸਥਿਰ __u32 CBWTag;

ਸਥਿਰ ਪੂਰਨ usb_max_devs; /* number of highest available usb device */

#ਜੇ !CONFIG_IS_ENABLED(BLK)
ਸਥਿਰ ਬਣਤਰ blk_desc usb_dev_desc[USB_MAX_STOR_DEV];
#ਜੇ_ਅੰਤ

ਬਣਤਰ us_ਅੰਕੜੇ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ (*trans_cmnd)(ਬਣਤਰ scsi_cmd *cb, ਬਣਤਰ us_ਅੰਕੜੇ *ਅੰਕੜੇ);
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ (*trans_reset)(ਬਣਤਰ us_ਅੰਕੜੇ *ਅੰਕੜੇ);

ਬਣਤਰ us_ਅੰਕੜੇ {
	ਬਣਤਰ usb_device *pusb_dev;	 /* this usb_device */

	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ	flags;			/* from filter initially */
#	define USB_READY	(1 << 0)
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ	ifnum;			/* interface number */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ	ep_ਵਿੱਚ;			/* in endpoint */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ	ep_ਬਾਹਰਲਾ;			/* out ....... */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ	ep_int;			/* interrupt . */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ	subclass;		/* as in overview */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ	protocol;		/* .............. */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ	attention_ਪੂਰਾ;		/* force attn on first cmd */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ	ip_ਅੰਕੜੇ;		/* interrupt data */
	ਪੂਰਨ		action;			/* what to do */
	ਪੂਰਨ		ip_wanted;		/* needed */
	ਪੂਰਨ		*irq_handle;		/* for USB int requests */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ	irqpipe;		/* pipe for release_irq */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ	irqmaxp;		/* max packed for irq Pipe */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ	irqinterval;		/* Intervall for IRQ Pipe */
	ਬਣਤਰ scsi_cmd	*srb;			/* current srb */
	trans_reset	transport_reset;	/* reset routine */
	trans_cmnd	transport;		/* transport routine */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ	max_xfer_blk;		/* maximum transfer blocks */
	bool		cmd12;			/* use 12-byte commands (RBC/UFI) */
};

#ਜੇ !CONFIG_IS_ENABLED(BLK)
ਸਥਿਰ ਬਣਤਰ us_ਅੰਕੜੇ usb_stor[USB_MAX_STOR_DEV];
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_STOR_TRANSPORT_GOOD	   0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_STOR_TRANSPORT_FAILED -1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_STOR_TRANSPORT_ERROR  -2

ਪੂਰਨ usb_stor_get_info(ਬਣਤਰ usb_device *dev, ਬਣਤਰ us_ਅੰਕੜੇ *us,
		      ਬਣਤਰ blk_desc *dev_desc);
ਪੂਰਨ usb_storage_probe(ਬਣਤਰ usb_device *dev, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ifnum,
		      ਬਣਤਰ us_ਅੰਕੜੇ *ss);
#ਜੇ CONFIG_IS_ENABLED(BLK)
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ usb_stor_read(ਬਣਤਰ udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, ਖਾਲੀ *ਬਫਰ);
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ usb_stor_write(ਬਣਤਰ udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, ਅਚਲ ਖਾਲੀ *ਬਫਰ);
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ usb_stor_read(ਬਣਤਰ blk_desc *ਖੰਡ_dev, lbaint_t blknr,
				   lbaint_t blkcnt, ਖਾਲੀ *ਬਫਰ);
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ usb_stor_write(ਬਣਤਰ blk_desc *ਖੰਡ_dev, lbaint_t blknr,
				    lbaint_t blkcnt, ਅਚਲ ਖਾਲੀ *ਬਫਰ);
#ਜੇ_ਅੰਤ
ਖਾਲੀ uhci_show_ਆਰਜ਼ੀ_int_td(ਖਾਲੀ);

ਸਥਿਰ ਖਾਲੀ usb_show_ਤਰੱਕੀ(ਖਾਲੀ)
{
	debug(".");
}

/*******************************************************************************
 * show info on storage devices; 'usb start/init' must be invoked earlier
 * as we only retrieve structures populated during devices initialization
 */
ਪੂਰਨ usb_stor_info(ਖਾਲੀ)
{
	ਪੂਰਨ ਗਿਣਤੀ_2 = 0;
#ਜੇ CONFIG_IS_ENABLED(BLK)
	ਬਣਤਰ udevice *dev;

	ਲਈ (blk_ਪਹਿਲਾ_device(UCLASS_USB, &dev);
	     dev;
	     blk_ਅਗਲਾ_device(&dev)) {
		ਬਣਤਰ blk_desc *desc = dev_get_uclass_plat(dev);

		ਲਿਖੋ("  Device %d: ", desc->devnum);
		dev_print(desc);
		ਗਿਣਤੀ_2++;
	}
#ਨਹੀਂਤਾਂ
	ਪੂਰਨ i;

	ਜੇ (usb_max_devs > 0) {
		ਲਈ (i = 0; i < usb_max_devs; i++) {
			ਲਿਖੋ("  Device %d: ", i);
			dev_print(&usb_dev_desc[i]);
		}
		ਵਾਪਸ 0;
	}
#ਜੇ_ਅੰਤ
	ਜੇ (!ਗਿਣਤੀ_2) {
		ਲਿਖੋ("No storage devices, perhaps not 'usb start'ed..?\n");
		ਵਾਪਸ 1;
	}

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ usb_get_max_lun(ਬਣਤਰ us_ਅੰਕੜੇ *us)
{
	ਪੂਰਨ len;
	ALLOC_CACHE_ALIGN_ਬਫਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ, ਨਤੀਜਾ, 1);
	len = usb_control_ਸੁਨੇਹਾ(us->pusb_dev,
			      usb_rcvctrlpipe(us->pusb_dev, 0),
			      US_BBB_GET_MAX_LUN,
			      USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_ਵਿੱਚ,
			      0, us->ifnum,
			      ਨਤੀਜਾ, ਆਕਾਰ(ਅੱਖਰ),
			      USB_CNTL_TIMEOUT * 5);
	debug("Get Max LUN -> len = %i, result = %i\n", len, (ਪੂਰਨ) *ਨਤੀਜਾ);
	ਵਾਪਸ (len > 0) ? *ਨਤੀਜਾ : 0;
}

ਸਥਿਰ ਪੂਰਨ usb_stor_probe_device(ਬਣਤਰ usb_device *udev)
{
	ਪੂਰਨ lun, max_lun;

#ਜੇ CONFIG_IS_ENABLED(BLK)
	ਬਣਤਰ us_ਅੰਕੜੇ *ਅੰਕੜੇ;
	ਪੂਰਨ ret;
#ਨਹੀਂਤਾਂ
	ਪੂਰਨ ਸ਼ੁਰੂ;

	ਜੇ (udev == ਨਿਰਾ)
		ਵਾਪਸ -ENOENT; /* no more devices available */
#ਜੇ_ਅੰਤ

	debug("\n\nProbing for storage\n");
#ਜੇ CONFIG_IS_ENABLED(BLK)
	/*
	 * We store the us_data in the mass storage device's plat. It
	 * is shared by all LUNs (block devices) attached to this mass storage
	 * device.
	 */
	ਅੰਕੜੇ = dev_get_plat(udev->dev);
	ਜੇ (!usb_storage_probe(udev, 0, ਅੰਕੜੇ))
		ਵਾਪਸ 0;
	max_lun = usb_get_max_lun(ਅੰਕੜੇ);
	ਲਈ (lun = 0; lun <= max_lun; lun++) {
		ਬਣਤਰ blk_desc *blkdev;
		ਬਣਤਰ udevice *dev;
		ਅੱਖਰ str[10];

		snprintf(str, ਆਕਾਰ(str), "lun%d", lun);
		ret = blk_create_devicef(udev->dev, "usb_storage_blk", str,
					 UCLASS_USB, usb_max_devs,
					 DEFAULT_BLKSZ, 0, &dev);
		ਜੇ (ret) {
			debug("Cannot bind driver\n");
			ਵਾਪਸ ret;
		}

		blkdev = dev_get_uclass_plat(dev);
		blkdev->ਨਿਸ਼ਾਨਾ = 0xff;
		blkdev->lun = lun;

		ret = usb_stor_get_info(udev, ਅੰਕੜੇ, blkdev);
		ਜੇ (ret == 1) {
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		} ਨਹੀਂਤਾਂ {
			debug("usb_stor_get_info: Invalid device\n");
			ret = device_unbind(dev);
			ਜੇ (ret)
				ਵਾਪਸ ret;
			ਜਾਰੀ;
		}

		ret = blk_probe_or_unbind(dev);
		ਜੇ (ret)
			ਵਾਪਸ ret;

		ret = bootdev_ਤਿਆਰੀ_for_sibling_blk(dev, "usb_bootdev");
		ਜੇ (ret) {
			ਪੂਰਨ ret2;

			ret2 = device_unbind(dev);
			ਜੇ (ret2)
				ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("bootdev", ret2);
			ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("bootdev", ret);
		}
	}
#ਨਹੀਂਤਾਂ
	/* We don't have space to even probe if we hit the maximum */
	ਜੇ (usb_max_devs == USB_MAX_STOR_DEV) {
		ਲਿਖੋ("max USB Storage Device reached: %d stopping\n",
		       usb_max_devs);
		ਵਾਪਸ -ENOSPC;
	}

	ਜੇ (!usb_storage_probe(udev, 0, &usb_stor[usb_max_devs]))
		ਵਾਪਸ 0;

	/*
	 * OK, it's a storage device.  Iterate over its LUNs and populate
	 * usb_dev_desc'
	 */
	ਸ਼ੁਰੂ = usb_max_devs;

	max_lun = usb_get_max_lun(&usb_stor[usb_max_devs]);
	ਲਈ (lun = 0; lun <= max_lun && usb_max_devs < USB_MAX_STOR_DEV;
	     lun++) {
		ਬਣਤਰ blk_desc *blkdev;

		blkdev = &usb_dev_desc[usb_max_devs];
		memset(blkdev, '\0', ਆਕਾਰ(ਬਣਤਰ blk_desc));
		blkdev->uclass_id = UCLASS_USB;
		blkdev->devnum = usb_max_devs;
		blkdev->part_type = PART_TYPE_UNKNOWN;
		blkdev->ਨਿਸ਼ਾਨਾ = 0xff;
		blkdev->type = DEV_TYPE_UNKNOWN;
		blkdev->ਖੰਡ_read = usb_stor_read;
		blkdev->ਖੰਡ_write = usb_stor_write;
		blkdev->lun = lun;
		blkdev->priv = udev;

		ਜੇ (usb_stor_get_info(udev, &usb_stor[ਸ਼ੁਰੂ],
				      &usb_dev_desc[usb_max_devs]) == 1) {
			debug("partype: %d\n", blkdev->part_type);
			part_init(blkdev);
			debug("partype: %d\n", blkdev->part_type);
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		}
	}
#ਜੇ_ਅੰਤ

	ਵਾਪਸ 0;
}

ਖਾਲੀ usb_stor_reset(ਖਾਲੀ)
{
	usb_max_devs = 0;
}

/*******************************************************************************
 * scan the usb and reports device info
 * to the user if mode = 1
 * returns current device or -1 if no
 */
ਪੂਰਨ usb_stor_ਜਾਂਚ(ਪੂਰਨ ਬਹੁਲਕ)
{
	ਜੇ (ਬਹੁਲਕ == 1)
		ਲਿਖੋ("       scanning usb for storage devices... ");

#ਜੇ !CONFIG_IS_ENABLED(DM_USB)
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ i;

	usb_disable_asynch(1); /* asynch transfer not allowed */

	usb_stor_reset();
	ਲਈ (i = 0; i < USB_MAX_DEVICE; i++) {
		ਬਣਤਰ usb_device *dev;

		dev = usb_get_dev_ਸੂਚਕਾਂਕ(i); /* get device */
		debug("i=%d\n", i);
		ਜੇ (usb_stor_probe_device(dev))
			ਰੋਕੋ;
	} /* for */

	usb_disable_asynch(0); /* asynch transfer allowed */
#ਜੇ_ਅੰਤ
	ਲਿਖੋ("%d Storage Device(s) found\n", usb_max_devs);
	ਜੇ (usb_max_devs > 0)
		ਵਾਪਸ 0;
	ਵਾਪਸ -1;
}

ਸਥਿਰ ਪੂਰਨ usb_stor_irq(ਬਣਤਰ usb_device *dev)
{
	ਬਣਤਰ us_ਅੰਕੜੇ *us;
	us = (ਬਣਤਰ us_ਅੰਕੜੇ *)dev->privptr;

	ਜੇ (us->ip_wanted)
		us->ip_wanted = 0;
	ਵਾਪਸ 0;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ	DEBUG

ਸਥਿਰ ਖਾਲੀ usb_show_srb(ਬਣਤਰ scsi_cmd *pccb)
{
	ਪੂਰਨ i;
	ਲਿਖੋ("SRB: len %d datalen 0x%lX\n ", pccb->cmdlen, pccb->datalen);
	ਲਈ (i = 0; i < pccb->cmdlen; i++)
		ਲਿਖੋ("%02X ", pccb->cmd[i]);
	ਲਿਖੋ("\n");
}

ਸਥਿਰ ਖਾਲੀ display_int_ਸਥਿਤੀ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ tmp)
{
	ਲਿਖੋ("Status: %s %s %s %s %s %s %s\n",
		(tmp & USB_ST_ਸਰਗਰਮ) ? "Active" : "",
		(tmp & USB_ST_STALLED) ? "Stalled" : "",
		(tmp & USB_ST_BUF_ERR) ? "Buffer Error" : "",
		(tmp & USB_ST_BABBLE_DET) ? "Babble Det" : "",
		(tmp & USB_ST_NAK_REC) ? "NAKed" : "",
		(tmp & USB_ST_ਚੱਕਰੀ_ਜਾਂਚ_ERR) ? "CRC Error" : "",
		(tmp & USB_ST_ਦੋਆਧਾਰੀ_ਅੰਕ_ERR) ? "Bitstuff Error" : "");
}
#ਜੇ_ਅੰਤ
/***********************************************************************
 * Data transfer routines
 ***********************************************************************/

ਸਥਿਰ ਪੂਰਨ us_one_transfer(ਬਣਤਰ us_ਅੰਕੜੇ *us, ਪੂਰਨ pipe, ਅੱਖਰ *buf, ਪੂਰਨ ਲੰਮਾਈ)
{
	ਪੂਰਨ max_ਮਾਪ;
	ਪੂਰਨ this_xfer;
	ਪੂਰਨ ਨਤੀਜਾ;
	ਪੂਰਨ partial;
	ਪੂਰਨ maxtry;
	ਪੂਰਨ stat;

	/* determine the maximum packet size for these transfers */
	max_ਮਾਪ = usb_maxpacket(us->pusb_dev, pipe) * 16;

	/* while we have data left to transfer */
	ਜਦੋਂ (ਲੰਮਾਈ) {

		/* calculate how long this will be -- maximum or a remainder */
		this_xfer = ਲੰਮਾਈ > max_ਮਾਪ ? max_ਮਾਪ : ਲੰਮਾਈ;
		ਲੰਮਾਈ -= this_xfer;

		/* setup the retry counter */
		maxtry = 10;

		/* set up the transfer loop */
		ਕਰੋ {
			/* transfer the data */
			debug("Bulk xfer 0x%lx(%d) try #%d\n",
			      (ulong)map_ਮੰਜ਼ਿਲ_sysmem(buf), this_xfer,
			      11 - maxtry);
			ਨਤੀਜਾ = usb_bulk_ਸੁਨੇਹਾ(us->pusb_dev, pipe, buf,
					      this_xfer, &partial,
					      USB_CNTL_TIMEOUT * 5);
			debug("bulk_msg returned %d xferred %d/%d\n",
			      ਨਤੀਜਾ, partial, this_xfer);
			ਜੇ (us->pusb_dev->ਸਥਿਤੀ != 0) {
				/* if we stall, we need to clear it before
				 * we go on
				 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DEBUG
				display_int_ਸਥਿਤੀ(us->pusb_dev->ਸਥਿਤੀ);
#ਜੇ_ਅੰਤ
				ਜੇ (us->pusb_dev->ਸਥਿਤੀ & USB_ST_STALLED) {
					debug("stalled ->clearing endpoint" \
					      "halt for pipe 0x%x\n", pipe);
					stat = us->pusb_dev->ਸਥਿਤੀ;
					usb_clear_halt(us->pusb_dev, pipe);
					us->pusb_dev->ਸਥਿਤੀ = stat;
					ਜੇ (this_xfer == partial) {
						debug("bulk transferred" \
						      "with error %lX," \
						      " but data ok\n",
						      us->pusb_dev->ਸਥਿਤੀ);
						ਵਾਪਸ 0;
					}
					ਨਹੀਂਤਾਂ
						ਵਾਪਸ ਨਤੀਜਾ;
				}
				ਜੇ (us->pusb_dev->ਸਥਿਤੀ & USB_ST_NAK_REC) {
					debug("Device NAKed bulk_msg\n");
					ਵਾਪਸ ਨਤੀਜਾ;
				}
				debug("bulk transferred with error");
				ਜੇ (this_xfer == partial) {
					debug(" %ld, but data ok\n",
					      us->pusb_dev->ਸਥਿਤੀ);
					ਵਾਪਸ 0;
				}
				/* if our try counter reaches 0, bail out */
				debug(" %ld, data %d\n",
				      us->pusb_dev->ਸਥਿਤੀ, partial);
				ਜੇ (!maxtry--)
						ਵਾਪਸ ਨਤੀਜਾ;
			}
			/* update to show what data was transferred */
			this_xfer -= partial;
			buf += partial;
			/* continue until this transfer is done */
		} ਜਦੋਂ (this_xfer);
	}

	/* if we get here, we're done and successful */
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ usb_stor_BBB_reset(ਬਣਤਰ us_ਅੰਕੜੇ *us)
{
	ਪੂਰਨ ਨਤੀਜਾ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ pipe;

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
	ਨਤੀਜਾ = usb_control_ਸੁਨੇਹਾ(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_BBB_RESET,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, ਨਿਰਾ, 0, USB_CNTL_TIMEOUT * 5);

	ਜੇ ((ਨਤੀਜਾ < 0) && (us->pusb_dev->ਸਥਿਤੀ & USB_ST_STALLED)) {
		debug("RESET:stall\n");
		ਵਾਪਸ -1;
	}

	/* long wait for reset */
	mdelay(150);
	debug("BBB_reset result %d: status %lX reset\n",
	      ਨਤੀਜਾ, us->pusb_dev->ਸਥਿਤੀ);
	pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_ਵਿੱਚ);
	ਨਤੀਜਾ = usb_clear_halt(us->pusb_dev, pipe);
	/* long wait for reset */
	mdelay(150);
	debug("BBB_reset result %d: status %lX clearing IN endpoint\n",
	      ਨਤੀਜਾ, us->pusb_dev->ਸਥਿਤੀ);
	/* long wait for reset */
	pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_ਬਾਹਰਲਾ);
	ਨਤੀਜਾ = usb_clear_halt(us->pusb_dev, pipe);
	mdelay(150);
	debug("BBB_reset result %d: status %lX clearing OUT endpoint\n",
	      ਨਤੀਜਾ, us->pusb_dev->ਸਥਿਤੀ);
	debug("BBB_reset done\n");
	ਵਾਪਸ 0;
}

/* FIXME: this reset function doesn't really reset the port, and it
 * should. Actually it should probably do what it's doing here, and
 * reset the port physically
 */
ਸਥਿਰ ਪੂਰਨ usb_stor_CB_reset(ਬਣਤਰ us_ਅੰਕੜੇ *us)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ cmd[12];
	ਪੂਰਨ ਨਤੀਜਾ;

	debug("CB_reset\n");
	memset(cmd, 0xff, ਆਕਾਰ(cmd));
	cmd[0] = SCSI_SEND_DIAG;
	cmd[1] = 4;
	ਨਤੀਜਾ = usb_control_ਸੁਨੇਹਾ(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_CBI_ADSC,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, cmd, ਆਕਾਰ(cmd),
				 USB_CNTL_TIMEOUT * 5);

	/* long wait for reset */
	mdelay(1500);
	debug("CB_reset result %d: status %lX clearing endpoint halt\n",
	      ਨਤੀਜਾ, us->pusb_dev->ਸਥਿਤੀ);
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_ਵਿੱਚ));
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_ਬਾਹਰਲਾ));

	debug("CB_reset done\n");
	ਵਾਪਸ 0;
}

/*
 * Set up the command for a BBB device. Note that the actual SCSI
 * command is copied into cbw.CBWCDB.
 */
ਸਥਿਰ ਪੂਰਨ usb_stor_BBB_comdat(ਬਣਤਰ scsi_cmd *srb, ਬਣਤਰ us_ਅੰਕੜੇ *us)
{
	ਪੂਰਨ ਨਤੀਜਾ;
	ਪੂਰਨ actlen;
	ਪੂਰਨ dir_ਵਿੱਚ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ pipe;
	ALLOC_CACHE_ALIGN_ਬਫਰ(ਬਣਤਰ umass_bbb_cbw, cbw, 1);

	dir_ਵਿੱਚ = US_DIRECTION(srb->cmd[0]);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BBB_COMDAT_TRACE
	ਲਿਖੋ("dir %d lun %d cmdlen %d cmd %p datalen %lu pdata %p\n",
		dir_ਵਿੱਚ, srb->lun, srb->cmdlen, srb->cmd, srb->datalen,
		srb->pdata);
	ਜੇ (srb->cmdlen) {
		ਲਈ (ਨਤੀਜਾ = 0; ਨਤੀਜਾ < srb->cmdlen; ਨਤੀਜਾ++)
			ਲਿਖੋ("cmd[%d] %#x ", ਨਤੀਜਾ, srb->cmd[ਨਤੀਜਾ]);
		ਲਿਖੋ("\n");
	}
#ਜੇ_ਅੰਤ
	/* sanity checks */
	ਜੇ (!(srb->cmdlen <= CBWCDBLENGTH)) {
		debug("usb_stor_BBB_comdat:cmdlen too large\n");
		ਵਾਪਸ -1;
	}

	/* always OUT to the ep */
	pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_ਬਾਹਰਲਾ);

	cbw->dCBWSignature = cpu_ਮੰਜ਼ਿਲ_le32(CBWSIGNATURE);
	cbw->dCBWTag = cpu_ਮੰਜ਼ਿਲ_le32(CBWTag++);
	cbw->d_CBWData_Transfer_ਲੰਮਾਈ = cpu_ਮੰਜ਼ਿਲ_le32(srb->datalen);
	cbw->bCBWFlags = (dir_ਵਿੱਚ ? CBWFLAGS_ਵਿੱਚ : CBWFLAGS_ਬਾਹਰਲਾ);
	cbw->bCBWLUN = srb->lun;
	cbw->bCDBLength = srb->cmdlen;
	/* copy the command data into the CBW command data buffer */
	/* DST SRC LEN!!! */

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(cbw->CBWCDB, srb->cmd, srb->cmdlen);
	ਨਤੀਜਾ = usb_bulk_ਸੁਨੇਹਾ(us->pusb_dev, pipe, cbw, UMASS_BBB_CBW_ਮਾਪ,
			      &actlen, USB_CNTL_TIMEOUT * 5);
	ਜੇ (ਨਤੀਜਾ < 0)
		debug("usb_stor_BBB_comdat:usb_bulk_msg error\n");
	ਵਾਪਸ ਨਤੀਜਾ;
}

/* FIXME: we also need a CBI_command which sets up the completion
 * interrupt, and waits for it
 */
ਸਥਿਰ ਪੂਰਨ usb_stor_CB_comdat(ਬਣਤਰ scsi_cmd *srb, ਬਣਤਰ us_ਅੰਕੜੇ *us)
{
	ਪੂਰਨ ਨਤੀਜਾ = 0;
	ਪੂਰਨ dir_ਵਿੱਚ, retry;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ pipe;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਸਥਿਤੀ;

	retry = 5;
	dir_ਵਿੱਚ = US_DIRECTION(srb->cmd[0]);

	ਜੇ (dir_ਵਿੱਚ)
		pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_ਵਿੱਚ);
	ਨਹੀਂਤਾਂ
		pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_ਬਾਹਰਲਾ);

	ਜਦੋਂ (retry--) {
		debug("CBI gets a command: Try %d\n", 5 - retry);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DEBUG
		usb_show_srb(srb);
#ਜੇ_ਅੰਤ
		/* let's send the command via the control pipe */
		ਨਤੀਜਾ = usb_control_ਸੁਨੇਹਾ(us->pusb_dev,
					 usb_sndctrlpipe(us->pusb_dev , 0),
					 US_CBI_ADSC,
					 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
					 0, us->ifnum,
					 srb->cmd, srb->cmdlen,
					 USB_CNTL_TIMEOUT * 5);
		debug("CB_transport: control msg returned %d, status %lX\n",
		      ਨਤੀਜਾ, us->pusb_dev->ਸਥਿਤੀ);
		/* check the return code for the command */
		ਜੇ (ਨਤੀਜਾ < 0) {
			ਜੇ (us->pusb_dev->ਸਥਿਤੀ & USB_ST_STALLED) {
				ਸਥਿਤੀ = us->pusb_dev->ਸਥਿਤੀ;
				debug(" stall during command found," \
				      " clear pipe\n");
				usb_clear_halt(us->pusb_dev,
					      usb_sndctrlpipe(us->pusb_dev, 0));
				us->pusb_dev->ਸਥਿਤੀ = ਸਥਿਤੀ;
			}
			debug(" error during command %02X" \
			      " Stat = %lX\n", srb->cmd[0],
			      us->pusb_dev->ਸਥਿਤੀ);
			ਵਾਪਸ ਨਤੀਜਾ;
		}
		/* transfer the data payload for this command, if one exists*/

		debug("CB_transport: control msg returned %d," \
		      " direction is %s to go 0x%lx\n", ਨਤੀਜਾ,
		      dir_ਵਿੱਚ ? "IN" : "OUT", srb->datalen);
		ਜੇ (srb->datalen) {
			ਨਤੀਜਾ = us_one_transfer(us, pipe, (ਅੱਖਰ *)srb->pdata,
						 srb->datalen);
			debug("CBI attempted to transfer data," \
			      " result is %d status %lX, len %d\n",
			      ਨਤੀਜਾ, us->pusb_dev->ਸਥਿਤੀ,
				us->pusb_dev->act_len);
			ਜੇ (!(us->pusb_dev->ਸਥਿਤੀ & USB_ST_NAK_REC))
				ਰੋਕੋ;
		} /* if (srb->datalen) */
		ਨਹੀਂਤਾਂ
			ਰੋਕੋ;
	}
	/* return result */

	ਵਾਪਸ ਨਤੀਜਾ;
}

ਸਥਿਰ ਪੂਰਨ usb_stor_CBI_get_ਸਥਿਤੀ(ਬਣਤਰ scsi_cmd *srb, ਬਣਤਰ us_ਅੰਕੜੇ *us)
{
	ਪੂਰਨ timeout;

	us->ip_wanted = 1;
	usb_int_ਸੁਨੇਹਾ(us->pusb_dev, us->irqpipe,
		    (ਖਾਲੀ *)&us->ip_ਅੰਕੜੇ, us->irqmaxp, us->irqinterval, false);
	timeout = 1000;
	ਜਦੋਂ (timeout--) {
		ਜੇ (us->ip_wanted == 0)
			ਰੋਕੋ;
		mdelay(10);
	}
	ਜੇ (us->ip_wanted) {
		ਲਿਖੋ("	Did not get interrupt on CBI\n");
		us->ip_wanted = 0;
		ਵਾਪਸ USB_STOR_TRANSPORT_ERROR;
	}
	debug("Got interrupt data 0x%x, transferred %d status 0x%lX\n",
	      us->ip_ਅੰਕੜੇ, us->pusb_dev->irq_act_len,
	      us->pusb_dev->irq_ਸਥਿਤੀ);
	/* UFI gives us ASC and ASCQ, like a request sense */
	ਜੇ (us->subclass == US_SC_UFI) {
		ਜੇ (srb->cmd[0] == SCSI_REQ_SENSE ||
		    srb->cmd[0] == SCSI_INQUIRY)
			ਵਾਪਸ USB_STOR_TRANSPORT_GOOD; /* Good */
		ਨਹੀਂਤਾਂ ਜੇ (us->ip_ਅੰਕੜੇ)
			ਵਾਪਸ USB_STOR_TRANSPORT_FAILED;
		ਨਹੀਂਤਾਂ
			ਵਾਪਸ USB_STOR_TRANSPORT_GOOD;
	}
	/* otherwise, we interpret the data normally */
	ਚੋਣ (us->ip_ਅੰਕੜੇ) {
	ਹਾਲਤ 0x0001:
		ਵਾਪਸ USB_STOR_TRANSPORT_GOOD;
	ਹਾਲਤ 0x0002:
		ਵਾਪਸ USB_STOR_TRANSPORT_FAILED;
	ਮੂਲ:
		ਵਾਪਸ USB_STOR_TRANSPORT_ERROR;
	}			/* switch */
	ਵਾਪਸ USB_STOR_TRANSPORT_ERROR;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_TRANSPORT_UNKNOWN_RETRY 5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_TRANSPORT_NOT_READY_RETRY 10

/* clear a stall on an endpoint - special for BBB devices */
ਸਥਿਰ ਪੂਰਨ usb_stor_BBB_clear_endpt_stall(ਬਣਤਰ us_ਅੰਕੜੇ *us, __u8 endpt)
{
	/* ENDPOINT_HALT = 0, so set value to 0 */
	ਵਾਪਸ usb_control_ਸੁਨੇਹਾ(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
			       USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
			       endpt, ਨਿਰਾ, 0, USB_CNTL_TIMEOUT * 5);
}

ਸਥਿਰ ਪੂਰਨ usb_stor_BBB_transport(ਬਣਤਰ scsi_cmd *srb, ਬਣਤਰ us_ਅੰਕੜੇ *us)
{
	ਪੂਰਨ ਨਤੀਜਾ, retry;
	ਪੂਰਨ dir_ਵਿੱਚ;
	ਪੂਰਨ actlen, ਅੰਕੜੇ_actlen;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ pipe, pipein, pipeout;
	ALLOC_CACHE_ALIGN_ਬਫਰ(ਬਣਤਰ umass_bbb_csw, csw, 1);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BBB_XPORT_TRACE
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ptr;
	ਪੂਰਨ ਸੂਚਕਾਂਕ;
#ਜੇ_ਅੰਤ

	dir_ਵਿੱਚ = US_DIRECTION(srb->cmd[0]);

	/* COMMAND phase */
	debug("COMMAND phase\n");
	ਨਤੀਜਾ = usb_stor_BBB_comdat(srb, us);
	ਜੇ (ਨਤੀਜਾ < 0) {
		debug("failed to send CBW status %ld\n",
		      us->pusb_dev->ਸਥਿਤੀ);
		usb_stor_BBB_reset(us);
		ਵਾਪਸ USB_STOR_TRANSPORT_FAILED;
	}
	ਜੇ (!(us->flags & USB_READY))
		mdelay(5);
	pipein = usb_rcvbulkpipe(us->pusb_dev, us->ep_ਵਿੱਚ);
	pipeout = usb_sndbulkpipe(us->pusb_dev, us->ep_ਬਾਹਰਲਾ);
	/* DATA phase + error handling */
	ਅੰਕੜੇ_actlen = 0;
	/* no data, go immediately to the STATUS phase */
	ਜੇ (srb->datalen == 0)
		ਜਾਓ st;
	debug("DATA phase\n");
	ਜੇ (dir_ਵਿੱਚ)
		pipe = pipein;
	ਨਹੀਂਤਾਂ
		pipe = pipeout;

	ਨਤੀਜਾ = usb_bulk_ਸੁਨੇਹਾ(us->pusb_dev, pipe, srb->pdata, srb->datalen,
			      &ਅੰਕੜੇ_actlen, USB_CNTL_TIMEOUT * 5);
	/* special handling of STALL in DATA phase */
	ਜੇ ((ਨਤੀਜਾ < 0) && (us->pusb_dev->ਸਥਿਤੀ & USB_ST_STALLED)) {
		debug("DATA:stall\n");
		/* clear the STALL on the endpoint */
		ਨਤੀਜਾ = usb_stor_BBB_clear_endpt_stall(us,
					dir_ਵਿੱਚ ? us->ep_ਵਿੱਚ : us->ep_ਬਾਹਰਲਾ);
		ਜੇ (ਨਤੀਜਾ >= 0)
			/* continue on to STATUS phase */
			ਜਾਓ st;
	}
	ਜੇ (ਨਤੀਜਾ < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->ਸਥਿਤੀ);
		usb_stor_BBB_reset(us);
		ਵਾਪਸ USB_STOR_TRANSPORT_FAILED;
	}
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BBB_XPORT_TRACE
	ਲਈ (ਸੂਚਕਾਂਕ = 0; ਸੂਚਕਾਂਕ < ਅੰਕੜੇ_actlen; ਸੂਚਕਾਂਕ++)
		ਲਿਖੋ("pdata[%d] %#x ", ਸੂਚਕਾਂਕ, srb->pdata[ਸੂਚਕਾਂਕ]);
	ਲਿਖੋ("\n");
#ਜੇ_ਅੰਤ
	/* STATUS phase + error handling */
st:
	retry = 0;
again:
	debug("STATUS phase\n");
	ਨਤੀਜਾ = usb_bulk_ਸੁਨੇਹਾ(us->pusb_dev, pipein, csw, UMASS_BBB_CSW_ਮਾਪ,
				&actlen, USB_CNTL_TIMEOUT*5);

	/* special handling of STALL in STATUS phase */
	ਜੇ ((ਨਤੀਜਾ < 0) && (retry < 1) &&
	    (us->pusb_dev->ਸਥਿਤੀ & USB_ST_STALLED)) {
		debug("STATUS:stall\n");
		/* clear the STALL on the endpoint */
		ਨਤੀਜਾ = usb_stor_BBB_clear_endpt_stall(us, us->ep_ਵਿੱਚ);
		ਜੇ (ਨਤੀਜਾ >= 0 && (retry++ < 1))
			/* do a retry */
			ਜਾਓ again;
	}
	ਜੇ (ਨਤੀਜਾ < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->ਸਥਿਤੀ);
		usb_stor_BBB_reset(us);
		ਵਾਪਸ USB_STOR_TRANSPORT_FAILED;
	}
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BBB_XPORT_TRACE
	ptr = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)csw;
	ਲਈ (ਸੂਚਕਾਂਕ = 0; ਸੂਚਕਾਂਕ < UMASS_BBB_CSW_ਮਾਪ; ਸੂਚਕਾਂਕ++)
		ਲਿਖੋ("ptr[%d] %#x ", ਸੂਚਕਾਂਕ, ptr[ਸੂਚਕਾਂਕ]);
	ਲਿਖੋ("\n");
#ਜੇ_ਅੰਤ
	/* misuse pipe to get the residue */
	pipe = le32_ਮੰਜ਼ਿਲ_cpu(csw->dCSWDataResidue);
	ਜੇ (pipe == 0 && srb->datalen != 0 && srb->datalen - ਅੰਕੜੇ_actlen != 0)
		pipe = srb->datalen - ਅੰਕੜੇ_actlen;
	ਜੇ (CSWSIGNATURE != le32_ਮੰਜ਼ਿਲ_cpu(csw->dCSWSignature)) {
		debug("!CSWSIGNATURE\n");
		usb_stor_BBB_reset(us);
		ਵਾਪਸ USB_STOR_TRANSPORT_FAILED;
	} ਨਹੀਂਤਾਂ ਜੇ ((CBWTag - 1) != le32_ਮੰਜ਼ਿਲ_cpu(csw->dCSWTag)) {
		debug("!Tag\n");
		usb_stor_BBB_reset(us);
		ਵਾਪਸ USB_STOR_TRANSPORT_FAILED;
	} ਨਹੀਂਤਾਂ ਜੇ (csw->bCSWStatus > CSWSTATUS_PHASE) {
		debug(">PHASE\n");
		usb_stor_BBB_reset(us);
		ਵਾਪਸ USB_STOR_TRANSPORT_FAILED;
	} ਨਹੀਂਤਾਂ ਜੇ (csw->bCSWStatus == CSWSTATUS_PHASE) {
		debug("=PHASE\n");
		usb_stor_BBB_reset(us);
		ਵਾਪਸ USB_STOR_TRANSPORT_FAILED;
	} ਨਹੀਂਤਾਂ ਜੇ (ਅੰਕੜੇ_actlen > srb->datalen) {
		debug("transferred %dB instead of %ldB\n",
		      ਅੰਕੜੇ_actlen, srb->datalen);
		ਵਾਪਸ USB_STOR_TRANSPORT_FAILED;
	} ਨਹੀਂਤਾਂ ਜੇ (csw->bCSWStatus == CSWSTATUS_FAILED) {
		debug("FAILED\n");
		ਵਾਪਸ USB_STOR_TRANSPORT_FAILED;
	}

	ਵਾਪਸ ਨਤੀਜਾ;
}

ਸਥਿਰ ਪੂਰਨ usb_stor_CB_transport(ਬਣਤਰ scsi_cmd *srb, ਬਣਤਰ us_ਅੰਕੜੇ *us)
{
	ਪੂਰਨ ਨਤੀਜਾ, ਸਥਿਤੀ;
	ਬਣਤਰ scsi_cmd *psrb;
	ਬਣਤਰ scsi_cmd reqsrb;
	ਪੂਰਨ retry, notready;

	psrb = &reqsrb;
	ਸਥਿਤੀ = USB_STOR_TRANSPORT_GOOD;
	retry = 0;
	notready = 0;
	/* issue the command */
do_retry:
	ਨਤੀਜਾ = usb_stor_CB_comdat(srb, us);
	debug("command / Data returned %d, status %lX\n",
	      ਨਤੀਜਾ, us->pusb_dev->ਸਥਿਤੀ);
	/* if this is an CBI Protocol, get IRQ */
	ਜੇ (us->protocol == US_PR_CBI) {
		ਸਥਿਤੀ = usb_stor_CBI_get_ਸਥਿਤੀ(srb, us);
		/* if the status is error, report it */
		ਜੇ (ਸਥਿਤੀ == USB_STOR_TRANSPORT_ERROR) {
			debug(" USB CBI Command Error\n");
			ਵਾਪਸ ਸਥਿਤੀ;
		}
		srb->sense_buf[12] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(us->ip_ਅੰਕੜੇ >> 8);
		srb->sense_buf[13] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(us->ip_ਅੰਕੜੇ & 0xff);
		ਜੇ (!us->ip_ਅੰਕੜੇ) {
			/* if the status is good, report it */
			ਜੇ (ਸਥਿਤੀ == USB_STOR_TRANSPORT_GOOD) {
				debug(" USB CBI Command Good\n");
				ਵਾਪਸ ਸਥਿਤੀ;
			}
		}
	}
	/* do we have to issue an auto request? */
	/* HERE we have to check the result */
	ਜੇ ((ਨਤੀਜਾ < 0) && !(us->pusb_dev->ਸਥਿਤੀ & USB_ST_STALLED)) {
		debug("ERROR %lX\n", us->pusb_dev->ਸਥਿਤੀ);
		us->transport_reset(us);
		ਵਾਪਸ USB_STOR_TRANSPORT_ERROR;
	}
	ਜੇ ((us->protocol == US_PR_CBI) &&
	    ((srb->cmd[0] == SCSI_REQ_SENSE) ||
	    (srb->cmd[0] == SCSI_INQUIRY))) {
		/* do not issue an autorequest after request sense */
		debug("No auto request and good\n");
		ਵਾਪਸ USB_STOR_TRANSPORT_GOOD;
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
	ਨਤੀਜਾ = usb_stor_CB_comdat(psrb, us);
	debug("auto request returned %d\n", ਨਤੀਜਾ);
	/* if this is an CBI Protocol, get IRQ */
	ਜੇ (us->protocol == US_PR_CBI)
		ਸਥਿਤੀ = usb_stor_CBI_get_ਸਥਿਤੀ(psrb, us);

	ਜੇ ((ਨਤੀਜਾ < 0) && !(us->pusb_dev->ਸਥਿਤੀ & USB_ST_STALLED)) {
		debug(" AUTO REQUEST ERROR %ld\n",
		      us->pusb_dev->ਸਥਿਤੀ);
		ਵਾਪਸ USB_STOR_TRANSPORT_ERROR;
	}
	debug("autorequest returned 0x%02X 0x%02X 0x%02X 0x%02X\n",
	      srb->sense_buf[0], srb->sense_buf[2],
	      srb->sense_buf[12], srb->sense_buf[13]);
	/* Check the auto request result */
	ਜੇ ((srb->sense_buf[2] == 0) &&
	    (srb->sense_buf[12] == 0) &&
	    (srb->sense_buf[13] == 0)) {
		/* ok, no sense */
		ਵਾਪਸ USB_STOR_TRANSPORT_GOOD;
	}

	/* Check the auto request result */
	ਚੋਣ (srb->sense_buf[2]) {
	ਹਾਲਤ 0x01:
		/* Recovered Error */
		ਵਾਪਸ USB_STOR_TRANSPORT_GOOD;
		ਰੋਕੋ;
	ਹਾਲਤ 0x02:
		/* Not Ready */
		ਜੇ (notready++ > USB_TRANSPORT_NOT_READY_RETRY) {
			ਲਿਖੋ("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X (NOT READY)\n", srb->cmd[0],
				srb->sense_buf[0], srb->sense_buf[2],
				srb->sense_buf[12], srb->sense_buf[13]);
			ਵਾਪਸ USB_STOR_TRANSPORT_FAILED;
		} ਨਹੀਂਤਾਂ {
			mdelay(100);
			ਜਾਓ do_retry;
		}
		ਰੋਕੋ;
	ਮੂਲ:
		ਜੇ (retry++ > USB_TRANSPORT_UNKNOWN_RETRY) {
			ਲਿਖੋ("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X\n", srb->cmd[0], srb->sense_buf[0],
				srb->sense_buf[2], srb->sense_buf[12],
				srb->sense_buf[13]);
			ਵਾਪਸ USB_STOR_TRANSPORT_FAILED;
		} ਨਹੀਂਤਾਂ
			ਜਾਓ do_retry;
		ਰੋਕੋ;
	}
	ਵਾਪਸ USB_STOR_TRANSPORT_FAILED;
}

ਸਥਿਰ ਖਾਲੀ usb_stor_ਸੈੱਟ_max_xfer_blk(ਬਣਤਰ usb_device *udev,
				      ਬਣਤਰ us_ਅੰਕੜੇ *us)
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
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ blk = 240;

#ਜੇ CONFIG_IS_ENABLED(DM_USB)
	size_t ਮਾਪ;
	ਪੂਰਨ ret;

	ret = usb_get_max_xfer_ਮਾਪ(udev, (size_t *)&ਮਾਪ);
	ਜੇ ((ret >= 0) && (ਮਾਪ < blk * 512))
		blk = ਮਾਪ / 512;
#ਜੇ_ਅੰਤ

	us->max_xfer_blk = blk;
}

ਸਥਿਰ ਪੂਰਨ usb_inquiry(ਬਣਤਰ scsi_cmd *srb, ਬਣਤਰ us_ਅੰਕੜੇ *ss)
{
	ਪੂਰਨ retry, i;
	retry = 5;
	ਕਰੋ {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_INQUIRY;
		srb->cmd[1] = srb->lun << 5;
		srb->cmd[4] = 36;
		srb->datalen = 36;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		i = ss->transport(srb, ss);
		debug("inquiry returns %d\n", i);
		ਜੇ (i == 0)
			ਰੋਕੋ;
	} ਜਦੋਂ (--retry);

	ਜੇ (!retry) {
		ਲਿਖੋ("error in inquiry\n");
		ਵਾਪਸ -1;
	}
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ usb_request_sense(ਬਣਤਰ scsi_cmd *srb, ਬਣਤਰ us_ਅੰਕੜੇ *ss)
{
	ਅੱਖਰ *ptr;

	ptr = (ਅੱਖਰ *)srb->pdata;
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
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ usb_test_unit_ready(ਬਣਤਰ scsi_cmd *srb, ਬਣਤਰ us_ਅੰਕੜੇ *ss)
{
	ਪੂਰਨ retries = 10;

	ਕਰੋ {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_TST_U_RDY;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 0;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		ਜੇ (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD) {
			ss->flags |= USB_READY;
			ਵਾਪਸ 0;
		}
		usb_request_sense(srb, ss);
		/*
		 * Check the Key Code Qualifier, if it matches
		 * "Not Ready - medium not present"
		 * (the sense Key equals 0x2 and the ASC is 0x3a)
		 * return immediately as the medium being absent won't change
		 * unless there is a user action.
		 */
		ਜੇ ((srb->sense_buf[2] == 0x02) &&
		    (srb->sense_buf[12] == 0x3a))
			ਵਾਪਸ -1;
		mdelay(100);
	} ਜਦੋਂ (retries--);

	ਵਾਪਸ -1;
}

ਸਥਿਰ ਪੂਰਨ usb_read_ਸਮਰੱਥਾ(ਬਣਤਰ scsi_cmd *srb, ਬਣਤਰ us_ਅੰਕੜੇ *ss)
{
	ਪੂਰਨ retry;
	/* XXX retries */
	retry = 3;
	ਕਰੋ {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_RD_CAPAC;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 8;
		srb->cmdlen = ss->cmd12 ? 12 : 10;
		ਜੇ (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD)
			ਵਾਪਸ 0;
	} ਜਦੋਂ (retry--);

	ਵਾਪਸ -1;
}

ਸਥਿਰ ਪੂਰਨ usb_read_10(ਬਣਤਰ scsi_cmd *srb, ਬਣਤਰ us_ਅੰਕੜੇ *ss,
		       ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਸ਼ੁਰੂ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ ਖੰਡਾਂ)
{
	memset(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_READ10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) (ਸ਼ੁਰੂ >> 24)) & 0xff;
	srb->cmd[3] = ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) (ਸ਼ੁਰੂ >> 16)) & 0xff;
	srb->cmd[4] = ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) (ਸ਼ੁਰੂ >> 8)) & 0xff;
	srb->cmd[5] = ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) (ਸ਼ੁਰੂ)) & 0xff;
	srb->cmd[7] = ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) (ਖੰਡਾਂ >> 8)) & 0xff;
	srb->cmd[8] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) ਖੰਡਾਂ & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("read10: start %lx blocks %x\n", ਸ਼ੁਰੂ, ਖੰਡਾਂ);
	ਵਾਪਸ ss->transport(srb, ss);
}

ਸਥਿਰ ਪੂਰਨ usb_write_10(ਬਣਤਰ scsi_cmd *srb, ਬਣਤਰ us_ਅੰਕੜੇ *ss,
			ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਸ਼ੁਰੂ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ ਖੰਡਾਂ)
{
	memset(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_WRITE10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) (ਸ਼ੁਰੂ >> 24)) & 0xff;
	srb->cmd[3] = ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) (ਸ਼ੁਰੂ >> 16)) & 0xff;
	srb->cmd[4] = ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) (ਸ਼ੁਰੂ >> 8)) & 0xff;
	srb->cmd[5] = ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) (ਸ਼ੁਰੂ)) & 0xff;
	srb->cmd[7] = ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) (ਖੰਡਾਂ >> 8)) & 0xff;
	srb->cmd[8] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) ਖੰਡਾਂ & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("write10: start %lx blocks %x\n", ਸ਼ੁਰੂ, ਖੰਡਾਂ);
	ਵਾਪਸ ss->transport(srb, ss);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USB_BIN_FIXUP
/*
 * Some USB storage devices queried for SCSI identification data respond with
 * binary strings, which if output to the console freeze the terminal. The
 * workaround is to modify the vendor and product strings read from such
 * device with proper values (as reported by 'usb info').
 *
 * Vendor and product length limits are taken from the definition of
 * struct blk_desc in include/part.h.
 */
ਸਥਿਰ ਖਾਲੀ usb_bin_fixup(ਬਣਤਰ usb_device_descriptor descriptor,
				ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ vendor[],
				ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਗੁਣਨਫਲ[]) {
	ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ max_vendor_len = 40;
	ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ max_ਗੁਣਨਫਲ_len = 20;
	ਜੇ (descriptor.idVendor == 0x0424 && descriptor.id_ਗੁਣਨਫਲ_2 == 0x223a) {
		strncpy((ਅੱਖਰ *)vendor, "SMSC", max_vendor_len);
		strncpy((ਅੱਖਰ *)ਗੁਣਨਫਲ, "Flash Media Cntrller",
			max_ਗੁਣਨਫਲ_len);
	}
}
#ਜੇ_ਅੰਤ /* CONFIG_USB_BIN_FIXUP */

#ਜੇ CONFIG_IS_ENABLED(BLK)
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ usb_stor_read(ਬਣਤਰ udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, ਖਾਲੀ *ਬਫਰ)
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ usb_stor_read(ਬਣਤਰ blk_desc *ਖੰਡ_dev, lbaint_t blknr,
				   lbaint_t blkcnt, ਖਾਲੀ *ਬਫਰ)
#ਜੇ_ਅੰਤ
{
	lbaint_t ਸ਼ੁਰੂ, blks;
	uintptr_t buf_addr;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ smallblks;
	ਬਣਤਰ usb_device *udev;
	ਬਣਤਰ us_ਅੰਕੜੇ *ss;
	ਪੂਰਨ retry;
	ਬਣਤਰ scsi_cmd *srb = &usb_ccb;
#ਜੇ CONFIG_IS_ENABLED(BLK)
	ਬਣਤਰ blk_desc *ਖੰਡ_dev;
#ਜੇ_ਅੰਤ

	ਜੇ (blkcnt == 0)
		ਵਾਪਸ 0;
	/* Setup  device */
#ਜੇ CONFIG_IS_ENABLED(BLK)
	ਖੰਡ_dev = dev_get_uclass_plat(dev);
	udev = dev_get_ਮਾਪਾ_priv(dev_get_ਮਾਪਾ(dev));
	debug("\nusb_read: udev %d\n", ਖੰਡ_dev->devnum);
#ਨਹੀਂਤਾਂ
	debug("\nusb_read: udev %d\n", ਖੰਡ_dev->devnum);
	udev = usb_dev_desc[ਖੰਡ_dev->devnum].priv;
	ਜੇ (!udev) {
		debug("%s: No device\n", __func__);
		ਵਾਪਸ 0;
	}
#ਜੇ_ਅੰਤ
	ss = (ਬਣਤਰ us_ਅੰਕੜੇ *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_ਤਾਲਾ_async(udev, 1);
	srb->lun = ਖੰਡ_dev->lun;
	buf_addr = (uintptr_t)ਬਫਰ;
	ਸ਼ੁਰੂ = blknr;
	blks = blkcnt;

	debug("\nusb_read: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      ਖੰਡ_dev->devnum, ਸ਼ੁਰੂ, blks, buf_addr);

	ਕਰੋ {
		/* XXX need some comment here */
		retry = 2;
		srb->pdata = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)buf_addr;
		ਜੇ (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		ਨਹੀਂਤਾਂ
			smallblks = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ) blks;
retry_it:
		ਜੇ (smallblks == ss->max_xfer_blk)
			usb_show_ਤਰੱਕੀ();
		srb->datalen = ਖੰਡ_dev->blksz * smallblks;
		srb->pdata = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)buf_addr;
		ਜੇ (usb_read_10(srb, ss, ਸ਼ੁਰੂ, smallblks)) {
			debug("Read ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			ਜੇ (retry--)
				ਜਾਓ retry_it;
			blkcnt -= blks;
			ਰੋਕੋ;
		}
		ਸ਼ੁਰੂ += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} ਜਦੋਂ (blks != 0);

	debug("usb_read: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      ਸ਼ੁਰੂ, smallblks, buf_addr);

	usb_ਤਾਲਾ_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	ਜੇ (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	ਵਾਪਸ blkcnt;
}

#ਜੇ CONFIG_IS_ENABLED(BLK)
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ usb_stor_write(ਬਣਤਰ udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, ਅਚਲ ਖਾਲੀ *ਬਫਰ)
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ usb_stor_write(ਬਣਤਰ blk_desc *ਖੰਡ_dev, lbaint_t blknr,
				    lbaint_t blkcnt, ਅਚਲ ਖਾਲੀ *ਬਫਰ)
#ਜੇ_ਅੰਤ
{
	lbaint_t ਸ਼ੁਰੂ, blks;
	uintptr_t buf_addr;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ smallblks;
	ਬਣਤਰ usb_device *udev;
	ਬਣਤਰ us_ਅੰਕੜੇ *ss;
	ਪੂਰਨ retry;
	ਬਣਤਰ scsi_cmd *srb = &usb_ccb;
#ਜੇ CONFIG_IS_ENABLED(BLK)
	ਬਣਤਰ blk_desc *ਖੰਡ_dev;
#ਜੇ_ਅੰਤ

	ਜੇ (blkcnt == 0)
		ਵਾਪਸ 0;

	/* Setup  device */
#ਜੇ CONFIG_IS_ENABLED(BLK)
	ਖੰਡ_dev = dev_get_uclass_plat(dev);
	udev = dev_get_ਮਾਪਾ_priv(dev_get_ਮਾਪਾ(dev));
	debug("\nusb_read: udev %d\n", ਖੰਡ_dev->devnum);
#ਨਹੀਂਤਾਂ
	debug("\nusb_read: udev %d\n", ਖੰਡ_dev->devnum);
	udev = usb_dev_desc[ਖੰਡ_dev->devnum].priv;
	ਜੇ (!udev) {
		debug("%s: No device\n", __func__);
		ਵਾਪਸ 0;
	}
#ਜੇ_ਅੰਤ
	ss = (ਬਣਤਰ us_ਅੰਕੜੇ *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_ਤਾਲਾ_async(udev, 1);

	srb->lun = ਖੰਡ_dev->lun;
	buf_addr = (uintptr_t)ਬਫਰ;
	ਸ਼ੁਰੂ = blknr;
	blks = blkcnt;

	debug("\nusb_write: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      ਖੰਡ_dev->devnum, ਸ਼ੁਰੂ, blks, buf_addr);

	ਕਰੋ {
		/* If write fails retry for max retry count else
		 * return with number of blocks written successfully.
		 */
		retry = 2;
		srb->pdata = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)buf_addr;
		ਜੇ (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		ਨਹੀਂਤਾਂ
			smallblks = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ) blks;
retry_it:
		ਜੇ (smallblks == ss->max_xfer_blk)
			usb_show_ਤਰੱਕੀ();
		srb->datalen = ਖੰਡ_dev->blksz * smallblks;
		srb->pdata = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)buf_addr;
		ਜੇ (usb_write_10(srb, ss, ਸ਼ੁਰੂ, smallblks)) {
			debug("Write ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			ਜੇ (retry--)
				ਜਾਓ retry_it;
			blkcnt -= blks;
			ਰੋਕੋ;
		}
		ਸ਼ੁਰੂ += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} ਜਦੋਂ (blks != 0);

	debug("usb_write: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      ਸ਼ੁਰੂ, smallblks, buf_addr);

	usb_ਤਾਲਾ_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	ਜੇ (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	ਵਾਪਸ blkcnt;

}

/* Probe to see if a new device is actually a Storage device */
ਪੂਰਨ usb_storage_probe(ਬਣਤਰ usb_device *dev, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ifnum,
		      ਬਣਤਰ us_ਅੰਕੜੇ *ss)
{
	ਬਣਤਰ usb_interface *iface;
	ਪੂਰਨ i;
	ਬਣਤਰ usb_endpoint_descriptor *ep_desc;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ flags = 0;

	/* let's examine the device now */
	iface = &dev->config.if_desc[ifnum];

	ਜੇ (dev->descriptor.bDeviceClass != 0 ||
			iface->desc.bInterfaceClass != USB_CLASS_MASS_STORAGE ||
			iface->desc.bInterfaceSubClass < US_SC_MIN ||
			iface->desc.bInterfaceSubClass > US_SC_MAX) {
		debug("Not mass storage\n");
		/* if it's not a mass storage, we go no further */
		ਵਾਪਸ 0;
	}

	memset(ss, 0, ਆਕਾਰ(ਬਣਤਰ us_ਅੰਕੜੇ));

	/* At this point, we know we've got a live one */
	debug("\n\nUSB Mass Storage device detected\n");

	/* Initialize the us_data structure with some useful info */
	ss->flags = flags;
	ss->ifnum = ifnum;
	ss->pusb_dev = dev;
	ss->attention_ਪੂਰਾ = 0;
	ss->subclass = iface->desc.bInterfaceSubClass;
	ss->protocol = iface->desc.bInterfaceProtocol;

	/* set the handler pointers based on the protocol */
	debug("Transport: ");
	ਚੋਣ (ss->protocol) {
	ਹਾਲਤ US_PR_CB:
		debug("Control/Bulk\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		ਰੋਕੋ;

	ਹਾਲਤ US_PR_CBI:
		debug("Control/Bulk/Interrupt\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		ਰੋਕੋ;
	ਹਾਲਤ US_PR_BULK:
		debug("Bulk/Bulk/Bulk\n");
		ss->transport = usb_stor_BBB_transport;
		ss->transport_reset = usb_stor_BBB_reset;
		ਰੋਕੋ;
	ਮੂਲ:
		ਲਿਖੋ("USB Storage Transport unknown / not yet implemented\n");
		ਵਾਪਸ 0;
		ਰੋਕੋ;
	}

	/*
	 * We are expecting a minimum of 2 endpoints - in and out (bulk).
	 * An optional interrupt is OK (necessary for CBI protocol).
	 * We will ignore any others.
	 */
	ਲਈ (i = 0; i < iface->desc.b_ਅੰਕ_Endpoints; i++) {
		ep_desc = &iface->ep_desc[i];
		/* is it an BULK endpoint? */
		ਜੇ ((ep_desc->bm_ਗੁਣ &
		     USB_ENDPOINT_XFERTYPE_ਨਕਾਬ) == USB_ENDPOINT_XFER_BULK) {
			ਜੇ (ep_desc->b_Endpoint_ਪਤਾ & USB_DIR_ਵਿੱਚ)
				ss->ep_ਵਿੱਚ = ep_desc->b_Endpoint_ਪਤਾ &
						USB_ENDPOINT_ਸੰਖਿਆ_ਨਕਾਬ;
			ਨਹੀਂਤਾਂ
				ss->ep_ਬਾਹਰਲਾ =
					ep_desc->b_Endpoint_ਪਤਾ &
					USB_ENDPOINT_ਸੰਖਿਆ_ਨਕਾਬ;
		}

		/* is it an interrupt endpoint? */
		ਜੇ ((ep_desc->bm_ਗੁਣ &
		     USB_ENDPOINT_XFERTYPE_ਨਕਾਬ) == USB_ENDPOINT_XFER_INT) {
			ss->ep_int = ep_desc->b_Endpoint_ਪਤਾ &
						USB_ENDPOINT_ਸੰਖਿਆ_ਨਕਾਬ;
			ss->irqinterval = ep_desc->bInterval;
		}
	}
	debug("Endpoints In %d Out %d Int %d\n",
	      ss->ep_ਵਿੱਚ, ss->ep_ਬਾਹਰਲਾ, ss->ep_int);

	/* Do some basic sanity checks, and bail if we find a problem */
	ਜੇ (usb_ਸੈੱਟ_interface(dev, iface->desc.b_Interface_ਸੰਖਿਆ, 0) ||
	    !ss->ep_ਵਿੱਚ || !ss->ep_ਬਾਹਰਲਾ ||
	    (ss->protocol == US_PR_CBI && ss->ep_int == 0)) {
		debug("Problems with device\n");
		ਵਾਪਸ 0;
	}
	/* set class specific stuff */
	/* We only handle certain protocols.  Currently, these are
	 * the only ones.
	 * The SFF8070 accepts the requests used in u-boot
	 */
	ਜੇ (ss->subclass != US_SC_UFI && ss->subclass != US_SC_SCSI &&
	    ss->subclass != US_SC_8070) {
		ਲਿਖੋ("Sorry, protocol %d not yet supported.\n", ss->subclass);
		ਵਾਪਸ 0;
	}

	/* UFI uses 12-byte commands (like RBC, unlike SCSI) */
	ਜੇ (ss->subclass == US_SC_UFI)
		ss->cmd12 = true;

	ਜੇ (ss->ep_int) {
		/* we had found an interrupt endpoint, prepare irq pipe
		 * set up the IRQ pipe and handler
		 */
		ss->irqinterval = (ss->irqinterval > 0) ? ss->irqinterval : 255;
		ss->irqpipe = usb_rcvintpipe(ss->pusb_dev, ss->ep_int);
		ss->irqmaxp = usb_maxpacket(dev, ss->irqpipe);
		dev->irq_handle = usb_stor_irq;
	}

	/* Set the maximum transfer size per host controller setting */
	usb_stor_ਸੈੱਟ_max_xfer_blk(dev, ss);

	dev->privptr = (ਖਾਲੀ *)ss;
	ਵਾਪਸ 1;
}

ਪੂਰਨ usb_stor_get_info(ਬਣਤਰ usb_device *dev, ਬਣਤਰ us_ਅੰਕੜੇ *ss,
		      ਬਣਤਰ blk_desc *dev_desc)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ perq, modi;
	ALLOC_CACHE_ALIGN_ਬਫਰ(u32, cap, 2);
	ALLOC_CACHE_ALIGN_ਬਫਰ(u8, usb_stor_buf, 36);
	u32 ਸਮਰੱਥਾ, blksz;
	ਬਣਤਰ scsi_cmd *pccb = &usb_ccb;

	pccb->pdata = usb_stor_buf;

	dev_desc->ਨਿਸ਼ਾਨਾ = dev->devnum;
	pccb->lun = dev_desc->lun;
	debug(" address %d\n", dev_desc->ਨਿਸ਼ਾਨਾ);

	ਜੇ (usb_inquiry(pccb, ss)) {
		debug("%s: usb_inquiry() failed\n", __func__);
		ਵਾਪਸ -1;
	}

	perq = usb_stor_buf[0];
	modi = usb_stor_buf[1];

	/*
	 * Skip unknown devices (0x1f) and enclosure service devices (0x0d),
	 * they would not respond to test_unit_ready .
	 */
	ਜੇ (((perq & 0x1f) == 0x1f) || ((perq & 0x1f) == 0x0d)) {
		debug("%s: unknown/unsupported device\n", __func__);
		ਵਾਪਸ 0;
	}
	ਜੇ ((modi&0x80) == 0x80) {
		/* drive is removable */
		dev_desc->removable = 1;
	}
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(dev_desc->vendor, (ਅਚਲ ਖਾਲੀ *)&usb_stor_buf[8], 8);
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(dev_desc->ਗੁਣਨਫਲ, (ਅਚਲ ਖਾਲੀ *)&usb_stor_buf[16], 16);
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(dev_desc->revision, (ਅਚਲ ਖਾਲੀ *)&usb_stor_buf[32], 4);
	dev_desc->vendor[8] = 0;
	dev_desc->ਗੁਣਨਫਲ[16] = 0;
	dev_desc->revision[4] = 0;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USB_BIN_FIXUP
	usb_bin_fixup(dev->descriptor, (uchar *)dev_desc->vendor,
		      (uchar *)dev_desc->ਗੁਣਨਫਲ);
#ਜੇ_ਅੰਤ /* CONFIG_USB_BIN_FIXUP */
	debug("ISO Vers %X, Response Data %X\n", usb_stor_buf[2],
	      usb_stor_buf[3]);
	ਜੇ (usb_test_unit_ready(pccb, ss)) {
		ਲਿਖੋ("Device NOT ready\n"
		       "   Request Sense returned %02X %02X %02X\n",
		       pccb->sense_buf[2], pccb->sense_buf[12],
		       pccb->sense_buf[13]);
		ਜੇ (dev_desc->removable == 1)
			dev_desc->type = perq;
		ਵਾਪਸ 0;
	}
	pccb->pdata = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)cap;
	memset(pccb->pdata, 0, 8);
	ਜੇ (usb_read_ਸਮਰੱਥਾ(pccb, ss) != 0) {
		ਲਿਖੋ("READ_CAP ERROR\n");
		ss->flags &= ~USB_READY;
		cap[0] = 2880;
		cap[1] = 0x200;
	}
	debug("Read Capacity returns: 0x%08x, 0x%08x\n", cap[0], cap[1]);
#ਜੇ 0
	ਜੇ (cap[0] > (0x200000 * 10)) /* greater than 10 GByte */
		cap[0] >>= 16;

	cap[0] = cpu_ਮੰਜ਼ਿਲ_be32(cap[0]);
	cap[1] = cpu_ਮੰਜ਼ਿਲ_be32(cap[1]);
#ਜੇ_ਅੰਤ

	ਸਮਰੱਥਾ = be32_ਮੰਜ਼ਿਲ_cpu(cap[0]) + 1;
	blksz = be32_ਮੰਜ਼ਿਲ_cpu(cap[1]);

	debug("Capacity = 0x%08x, blocksz = 0x%08x\n", ਸਮਰੱਥਾ, blksz);
	dev_desc->lba = ਸਮਰੱਥਾ;
	dev_desc->blksz = blksz;
	dev_desc->log2blksz = LOG2(dev_desc->blksz);
	dev_desc->type = perq;
	debug(" address %d\n", dev_desc->ਨਿਸ਼ਾਨਾ);

	ਵਾਪਸ 1;
}

#ਜੇ CONFIG_IS_ENABLED(DM_USB)

ਸਥਿਰ ਪੂਰਨ usb_mass_storage_probe(ਬਣਤਰ udevice *dev)
{
	ਬਣਤਰ usb_device *udev = dev_get_ਮਾਪਾ_priv(dev);
	ਪੂਰਨ ret;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	ret = usb_stor_probe_device(udev);
	usb_disable_asynch(0); /* asynch transfer allowed */

	ਵਾਪਸ ret;
}

ਸਥਿਰ ਅਚਲ ਬਣਤਰ udevice_id usb_mass_storage_ids[] = {
	{ .compatible = "usb-mass-storage" },
	{ }
};

U_BOOT_DRIVER(usb_mass_storage) = {
	.name	= "usb_mass_storage",
	.id	= UCLASS_MASS_STORAGE,
	.ਦਾ_ਮੇਲ = usb_mass_storage_ids,
	.probe = usb_mass_storage_probe,
#ਜੇ CONFIG_IS_ENABLED(BLK)
	.plat_auto	= ਆਕਾਰ(ਬਣਤਰ us_ਅੰਕੜੇ),
#ਜੇ_ਅੰਤ
};

UCLASS_DRIVER(usb_mass_storage) = {
	.id		= UCLASS_MASS_STORAGE,
	.name		= "usb_mass_storage",
};

ਸਥਿਰ ਅਚਲ ਬਣਤਰ usb_device_id mass_storage_id_ਸਾਰਣੀ[] = {
	{
		.ਮੇਲ_flags = USB_DEVICE_ID_ਮੇਲ_INT_CLASS,
		.bInterfaceClass = USB_CLASS_MASS_STORAGE
	},
	{ }		/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_mass_storage, mass_storage_id_ਸਾਰਣੀ);
#ਜੇ_ਅੰਤ

#ਜੇ CONFIG_IS_ENABLED(BLK)
ਸਥਿਰ ਅਚਲ ਬਣਤਰ blk_ops usb_storage_ops = {
	.ਪਠ	= usb_stor_read,
	.ਲਿਖ	= usb_stor_write,
};

U_BOOT_DRIVER(usb_storage_blk) = {
	.name		= "usb_storage_blk",
	.id		= UCLASS_BLK,
	.ops		= &usb_storage_ops,
};
#ਨਹੀਂਤਾਂ
U_BOOT_LEGACY_BLK(usb) = {
	.uclass_idname	= "usb",
	.uclass_id	= UCLASS_USB,
	.max_devs	= USB_MAX_STOR_DEV,
	.desc		= usb_dev_desc,
};
#ਜੇ_ਅੰਤ
