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

#undef BBB_COMDAT_TRACE
#undef BBB_XPORT_TRACE

#ಅನ್ತರ್ಭಾವ <scsi.h>
/* direction table -- this indicates the direction of the data
 * transfer for each command code -- a 1 indicates input
 */
ಸ್ಥಿರ ಧ್ರುವ unsigned ವರ್ಣ us_direction[256/8] = {
	0x28, 0x81, 0x14, 0x14, 0x20, 0x01, 0x90, 0x77,
	0x0C, 0x20, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#define US_DIRECTION(x) ((us_direction[x>>3] >> (x & 7)) & 1)

ಸ್ಥಿರ ಸಂರಚನಾ scsi_cmd usb_ccb __aligned(ARCH_DMA_MINALIGN);
ಸ್ಥಿರ __u32 CBWTag;

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_max_devs; /* number of highest available usb device */

#ಯದಿ !CONFIG_IS_ENABLED(BLK)
ಸ್ಥಿರ ಸಂರಚನಾ blk_desc usb_dev_desc[USB_MAX_STOR_DEV];
#endif

ಸಂರಚನಾ us_data;
ಪ್ರಕಾರನಿರ್ದೇಶ ಪೂರ್ಣಾಙ್ಕ (*trans_cmnd)(ಸಂರಚನಾ scsi_cmd *cb, ಸಂರಚನಾ us_data *data);
ಪ್ರಕಾರನಿರ್ದೇಶ ಪೂರ್ಣಾಙ್ಕ (*trans_reset)(ಸಂರಚನಾ us_data *data);

ಸಂರಚನಾ us_data {
	ಸಂರಚನಾ usb_device *pusb_dev;	 /* this usb_device */

	unsigned ಪೂರ್ಣಾಙ್ಕ	flags;			/* from filter initially */
#	define USB_READY	(1 << 0)
	unsigned ವರ್ಣ	ifnum;			/* interface number */
	unsigned ವರ್ಣ	ep_in;			/* in endpoint */
	unsigned ವರ್ಣ	ep_out;			/* out ....... */
	unsigned ವರ್ಣ	ep_int;			/* interrupt . */
	unsigned ವರ್ಣ	subclass;		/* as in overview */
	unsigned ವರ್ಣ	protocol;		/* .............. */
	unsigned ವರ್ಣ	attention_done;		/* force attn on first cmd */
	unsigned short	ip_data;		/* interrupt data */
	ಪೂರ್ಣಾಙ್ಕ		action;			/* what to do */
	ಪೂರ್ಣಾಙ್ಕ		ip_wanted;		/* needed */
	ಪೂರ್ಣಾಙ್ಕ		*irq_handle;		/* for USB int requests */
	unsigned ಪೂರ್ಣಾಙ್ಕ	irqpipe;		/* pipe for release_irq */
	unsigned ವರ್ಣ	irqmaxp;		/* max packed for irq Pipe */
	unsigned ವರ್ಣ	irqinterval;		/* Intervall for IRQ Pipe */
	ಸಂರಚನಾ scsi_cmd	*srb;			/* current srb */
	trans_reset	transport_reset;	/* reset routine */
	trans_cmnd	transport;		/* transport routine */
	unsigned short	max_xfer_blk;		/* maximum transfer blocks */
	bool		cmd12;			/* use 12-byte commands (RBC/UFI) */
};

#ಯದಿ !CONFIG_IS_ENABLED(BLK)
ಸ್ಥಿರ ಸಂರಚನಾ us_data usb_stor[USB_MAX_STOR_DEV];
#endif

#define USB_STOR_TRANSPORT_GOOD	   0
#define USB_STOR_TRANSPORT_FAILED -1
#define USB_STOR_TRANSPORT_ERROR  -2

ಪೂರ್ಣಾಙ್ಕ usb_stor_get_info(ಸಂರಚನಾ usb_device *dev, ಸಂರಚನಾ us_data *us,
		      ಸಂರಚನಾ blk_desc *dev_desc);
ಪೂರ್ಣಾಙ್ಕ usb_storage_probe(ಸಂರಚನಾ usb_device *dev, unsigned ಪೂರ್ಣಾಙ್ಕ ifnum,
		      ಸಂರಚನಾ us_data *ss);
#ಯದಿ CONFIG_IS_ENABLED(BLK)
ಸ್ಥಿರ unsigned long usb_stor_read(ಸಂರಚನಾ udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, ಶೂನ್ಯ *buffer);
ಸ್ಥಿರ unsigned long usb_stor_write(ಸಂರಚನಾ udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, ಧ್ರುವ ಶೂನ್ಯ *buffer);
#ಅನ್ಯಥಾ
ಸ್ಥಿರ unsigned long usb_stor_read(ಸಂರಚನಾ blk_desc *block_dev, lbaint_t blknr,
				   lbaint_t blkcnt, ಶೂನ್ಯ *buffer);
ಸ್ಥಿರ unsigned long usb_stor_write(ಸಂರಚನಾ blk_desc *block_dev, lbaint_t blknr,
				    lbaint_t blkcnt, ಧ್ರುವ ಶೂನ್ಯ *buffer);
#endif
ಶೂನ್ಯ uhci_show_temp_int_td(ಶೂನ್ಯ);

ಸ್ಥಿರ ಶೂನ್ಯ usb_show_progress(ಶೂನ್ಯ)
{
	debug(".");
}

/*******************************************************************************
 * show info on storage devices; 'usb start/init' must be invoked earlier
 * as we only retrieve structures populated during devices initialization
 */
ಪೂರ್ಣಾಙ್ಕ usb_stor_info(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = 0;
#ಯದಿ CONFIG_IS_ENABLED(BLK)
	ಸಂರಚನಾ udevice *dev;

	ಪ್ರತಿ (blk_first_device(UCLASS_USB, &dev);
	     dev;
	     blk_next_device(&dev)) {
		ಸಂರಚನಾ blk_desc *desc = dev_get_uclass_plat(dev);

		printf("  Device %d: ", desc->devnum);
		dev_print(desc);
		ಎಣಿಕೆ++;
	}
#ಅನ್ಯಥಾ
	ಪೂರ್ಣಾಙ್ಕ i;

	ಯದಿ (usb_max_devs > 0) {
		ಪ್ರತಿ (i = 0; i < usb_max_devs; i++) {
			printf("  Device %d: ", i);
			dev_print(&usb_dev_desc[i]);
		}
		ಪ್ರತ್ಯಾಗಮ 0;
	}
#endif
	ಯದಿ (!ಎಣಿಕೆ) {
		printf("No storage devices, perhaps not 'usb start'ed..?\n");
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ unsigned ಪೂರ್ಣಾಙ್ಕ usb_get_max_lun(ಸಂರಚನಾ us_data *us)
{
	ಪೂರ್ಣಾಙ್ಕ len;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned ವರ್ಣ, result, 1);
	len = usb_control_ಸಂದೇಶ(us->pusb_dev,
			      usb_rcvctrlpipe(us->pusb_dev, 0),
			      US_BBB_GET_MAX_LUN,
			      USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_IN,
			      0, us->ifnum,
			      result, ಪರಿಮಾಣ(ವರ್ಣ),
			      USB_CNTL_TIMEOUT * 5);
	debug("Get Max LUN -> len = %i, result = %i\n", len, (ಪೂರ್ಣಾಙ್ಕ) *result);
	ಪ್ರತ್ಯಾಗಮ (len > 0) ? *result : 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_stor_probe_device(ಸಂರಚನಾ usb_device *udev)
{
	ಪೂರ್ಣಾಙ್ಕ lun, max_lun;

#ಯದಿ CONFIG_IS_ENABLED(BLK)
	ಸಂರಚನಾ us_data *data;
	ಪೂರ್ಣಾಙ್ಕ ret;
#ಅನ್ಯಥಾ
	ಪೂರ್ಣಾಙ್ಕ start;

	ಯದಿ (udev == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ -ENOENT; /* no more devices available */
#endif

	debug("\n\nProbing for storage\n");
#ಯದಿ CONFIG_IS_ENABLED(BLK)
	/*
	 * We store the us_data in the mass storage device's plat. It
	 * is shared by all LUNs (block devices) attached to this mass storage
	 * device.
	 */
	data = dev_get_plat(udev->dev);
	ಯದಿ (!usb_storage_probe(udev, 0, data))
		ಪ್ರತ್ಯಾಗಮ 0;
	max_lun = usb_get_max_lun(data);
	ಪ್ರತಿ (lun = 0; lun <= max_lun; lun++) {
		ಸಂರಚನಾ blk_desc *blkdev;
		ಸಂರಚನಾ udevice *dev;
		ವರ್ಣ str[10];

		snprintf(str, ಪರಿಮಾಣ(str), "lun%d", lun);
		ret = blk_create_devicef(udev->dev, "usb_storage_blk", str,
					 UCLASS_USB, usb_max_devs,
					 DEFAULT_BLKSZ, 0, &dev);
		ಯದಿ (ret) {
			debug("Cannot bind driver\n");
			ಪ್ರತ್ಯಾಗಮ ret;
		}

		blkdev = dev_get_uclass_plat(dev);
		blkdev->target = 0xff;
		blkdev->lun = lun;

		ret = usb_stor_get_info(udev, data, blkdev);
		ಯದಿ (ret == 1) {
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		} ಅನ್ಯಥಾ {
			debug("usb_stor_get_info: Invalid device\n");
			ret = device_unbind(dev);
			ಯದಿ (ret)
				ಪ್ರತ್ಯಾಗಮ ret;
			ಅನುವರ್ತ;
		}

		ret = blk_probe_or_unbind(dev);
		ಯದಿ (ret)
			ಪ್ರತ್ಯಾಗಮ ret;

		ret = bootdev_setup_for_sibling_blk(dev, "usb_bootdev");
		ಯದಿ (ret) {
			ಪೂರ್ಣಾಙ್ಕ ret2;

			ret2 = device_unbind(dev);
			ಯದಿ (ret2)
				ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("bootdev", ret2);
			ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("bootdev", ret);
		}
	}
#ಅನ್ಯಥಾ
	/* We don't have space to even probe if we hit the maximum */
	ಯದಿ (usb_max_devs == USB_MAX_STOR_DEV) {
		printf("max USB Storage Device reached: %d stopping\n",
		       usb_max_devs);
		ಪ್ರತ್ಯಾಗಮ -ENOSPC;
	}

	ಯದಿ (!usb_storage_probe(udev, 0, &usb_stor[usb_max_devs]))
		ಪ್ರತ್ಯಾಗಮ 0;

	/*
	 * OK, it's a storage device.  Iterate over its LUNs and populate
	 * usb_dev_desc'
	 */
	start = usb_max_devs;

	max_lun = usb_get_max_lun(&usb_stor[usb_max_devs]);
	ಪ್ರತಿ (lun = 0; lun <= max_lun && usb_max_devs < USB_MAX_STOR_DEV;
	     lun++) {
		ಸಂರಚನಾ blk_desc *blkdev;

		blkdev = &usb_dev_desc[usb_max_devs];
		memset(blkdev, '\0', ಪರಿಮಾಣ(ಸಂರಚನಾ blk_desc));
		blkdev->uclass_id = UCLASS_USB;
		blkdev->devnum = usb_max_devs;
		blkdev->part_type = PART_TYPE_UNKNOWN;
		blkdev->target = 0xff;
		blkdev->type = DEV_TYPE_UNKNOWN;
		blkdev->block_read = usb_stor_read;
		blkdev->block_write = usb_stor_write;
		blkdev->lun = lun;
		blkdev->priv = udev;

		ಯದಿ (usb_stor_get_info(udev, &usb_stor[start],
				      &usb_dev_desc[usb_max_devs]) == 1) {
			debug("partype: %d\n", blkdev->part_type);
			part_init(blkdev);
			debug("partype: %d\n", blkdev->part_type);
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		}
	}
#endif

	ಪ್ರತ್ಯಾಗಮ 0;
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
ಪೂರ್ಣಾಙ್ಕ usb_stor_scan(ಪೂರ್ಣಾಙ್ಕ mode)
{
	ಯದಿ (mode == 1)
		printf("       scanning usb for storage devices... ");

#ಯದಿ !CONFIG_IS_ENABLED(DM_USB)
	unsigned ವರ್ಣ i;

	usb_disable_asynch(1); /* asynch transfer not allowed */

	usb_stor_reset();
	ಪ್ರತಿ (i = 0; i < USB_MAX_DEVICE; i++) {
		ಸಂರಚನಾ usb_device *dev;

		dev = usb_get_dev_index(i); /* get device */
		debug("i=%d\n", i);
		ಯದಿ (usb_stor_probe_device(dev))
			ಭಙ್ಗ;
	} /* for */

	usb_disable_asynch(0); /* asynch transfer allowed */
#endif
	printf("%d Storage Device(s) found\n", usb_max_devs);
	ಯದಿ (usb_max_devs > 0)
		ಪ್ರತ್ಯಾಗಮ 0;
	ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_stor_irq(ಸಂರಚನಾ usb_device *dev)
{
	ಸಂರಚನಾ us_data *us;
	us = (ಸಂರಚನಾ us_data *)dev->privptr;

	ಯದಿ (us->ip_wanted)
		us->ip_wanted = 0;
	ಪ್ರತ್ಯಾಗಮ 0;
}

#ifdef	DEBUG

ಸ್ಥಿರ ಶೂನ್ಯ usb_show_srb(ಸಂರಚನಾ scsi_cmd *pccb)
{
	ಪೂರ್ಣಾಙ್ಕ i;
	printf("SRB: len %d datalen 0x%lX\n ", pccb->cmdlen, pccb->datalen);
	ಪ್ರತಿ (i = 0; i < pccb->cmdlen; i++)
		printf("%02X ", pccb->cmd[i]);
	printf("\n");
}

ಸ್ಥಿರ ಶೂನ್ಯ display_int_status(unsigned long tmp)
{
	printf("Status: %s %s %s %s %s %s %s\n",
		(tmp & USB_ST_ACTIVE) ? "Active" : "",
		(tmp & USB_ST_STALLED) ? "Stalled" : "",
		(tmp & USB_ST_BUF_ERR) ? "Buffer Error" : "",
		(tmp & USB_ST_BABBLE_DET) ? "Babble Det" : "",
		(tmp & USB_ST_NAK_REC) ? "NAKed" : "",
		(tmp & USB_ST_CRC_ERR) ? "CRC Error" : "",
		(tmp & USB_ST_BIT_ERR) ? "Bitstuff Error" : "");
}
#endif
/***********************************************************************
 * Data transfer routines
 ***********************************************************************/

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ us_one_transfer(ಸಂರಚನಾ us_data *us, ಪೂರ್ಣಾಙ್ಕ pipe, ವರ್ಣ *buf, ಪೂರ್ಣಾಙ್ಕ length)
{
	ಪೂರ್ಣಾಙ್ಕ max_size;
	ಪೂರ್ಣಾಙ್ಕ this_xfer;
	ಪೂರ್ಣಾಙ್ಕ result;
	ಪೂರ್ಣಾಙ್ಕ partial;
	ಪೂರ್ಣಾಙ್ಕ maxtry;
	ಪೂರ್ಣಾಙ್ಕ stat;

	/* determine the maximum packet size for these transfers */
	max_size = usb_maxpacket(us->pusb_dev, pipe) * 16;

	/* while we have data left to transfer */
	ಯಾವತ್ (length) {

		/* calculate how long this will be -- maximum or a remainder */
		this_xfer = length > max_size ? max_size : length;
		length -= this_xfer;

		/* setup the retry counter */
		maxtry = 10;

		/* set up the transfer loop */
		ಕುರು {
			/* transfer the data */
			debug("Bulk xfer 0x%lx(%d) try #%d\n",
			      (ulong)map_to_sysmem(buf), this_xfer,
			      11 - maxtry);
			result = usb_bulk_ಸಂದೇಶ(us->pusb_dev, pipe, buf,
					      this_xfer, &partial,
					      USB_CNTL_TIMEOUT * 5);
			debug("bulk_msg returned %d xferred %d/%d\n",
			      result, partial, this_xfer);
			ಯದಿ (us->pusb_dev->status != 0) {
				/* if we stall, we need to clear it before
				 * we go on
				 */
#ifdef DEBUG
				display_int_status(us->pusb_dev->status);
#endif
				ಯದಿ (us->pusb_dev->status & USB_ST_STALLED) {
					debug("stalled ->clearing endpoint" \
					      "halt for pipe 0x%x\n", pipe);
					stat = us->pusb_dev->status;
					usb_clear_halt(us->pusb_dev, pipe);
					us->pusb_dev->status = stat;
					ಯದಿ (this_xfer == partial) {
						debug("bulk transferred" \
						      "with error %lX," \
						      " but data ok\n",
						      us->pusb_dev->status);
						ಪ್ರತ್ಯಾಗಮ 0;
					}
					ಅನ್ಯಥಾ
						ಪ್ರತ್ಯಾಗಮ result;
				}
				ಯದಿ (us->pusb_dev->status & USB_ST_NAK_REC) {
					debug("Device NAKed bulk_msg\n");
					ಪ್ರತ್ಯಾಗಮ result;
				}
				debug("bulk transferred with error");
				ಯದಿ (this_xfer == partial) {
					debug(" %ld, but data ok\n",
					      us->pusb_dev->status);
					ಪ್ರತ್ಯಾಗಮ 0;
				}
				/* if our try counter reaches 0, bail out */
				debug(" %ld, data %d\n",
				      us->pusb_dev->status, partial);
				ಯದಿ (!maxtry--)
						ಪ್ರತ್ಯಾಗಮ result;
			}
			/* update to show what data was transferred */
			this_xfer -= partial;
			buf += partial;
			/* continue until this transfer is done */
		} ಯಾವತ್ (this_xfer);
	}

	/* if we get here, we're done and successful */
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_stor_BBB_reset(ಸಂರಚನಾ us_data *us)
{
	ಪೂರ್ಣಾಙ್ಕ result;
	unsigned ಪೂರ್ಣಾಙ್ಕ pipe;

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
	result = usb_control_ಸಂದೇಶ(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_BBB_RESET,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, ರಿಕ್ತ, 0, USB_CNTL_TIMEOUT * 5);

	ಯದಿ ((result < 0) && (us->pusb_dev->status & USB_ST_STALLED)) {
		debug("RESET:stall\n");
		ಪ್ರತ್ಯಾಗಮ -1;
	}

	/* long wait for reset */
	mdelay(150);
	debug("BBB_reset result %d: status %lX reset\n",
	      result, us->pusb_dev->status);
	pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);
	result = usb_clear_halt(us->pusb_dev, pipe);
	/* long wait for reset */
	mdelay(150);
	debug("BBB_reset result %d: status %lX clearing IN endpoint\n",
	      result, us->pusb_dev->status);
	/* long wait for reset */
	pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_out);
	result = usb_clear_halt(us->pusb_dev, pipe);
	mdelay(150);
	debug("BBB_reset result %d: status %lX clearing OUT endpoint\n",
	      result, us->pusb_dev->status);
	debug("BBB_reset done\n");
	ಪ್ರತ್ಯಾಗಮ 0;
}

/* FIXME: this reset function doesn't really reset the port, and it
 * should. Actually it should probably do what it's doing here, and
 * reset the port physically
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_stor_CB_reset(ಸಂರಚನಾ us_data *us)
{
	unsigned ವರ್ಣ cmd[12];
	ಪೂರ್ಣಾಙ್ಕ result;

	debug("CB_reset\n");
	memset(cmd, 0xff, ಪರಿಮಾಣ(cmd));
	cmd[0] = SCSI_SEND_DIAG;
	cmd[1] = 4;
	result = usb_control_ಸಂದೇಶ(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_CBI_ADSC,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, cmd, ಪರಿಮಾಣ(cmd),
				 USB_CNTL_TIMEOUT * 5);

	/* long wait for reset */
	mdelay(1500);
	debug("CB_reset result %d: status %lX clearing endpoint halt\n",
	      result, us->pusb_dev->status);
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_in));
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_out));

	debug("CB_reset done\n");
	ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * Set up the command for a BBB device. Note that the actual SCSI
 * command is copied into cbw.CBWCDB.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_stor_BBB_comdat(ಸಂರಚನಾ scsi_cmd *srb, ಸಂರಚನಾ us_data *us)
{
	ಪೂರ್ಣಾಙ್ಕ result;
	ಪೂರ್ಣಾಙ್ಕ actlen;
	ಪೂರ್ಣಾಙ್ಕ dir_in;
	unsigned ಪೂರ್ಣಾಙ್ಕ pipe;
	ALLOC_CACHE_ALIGN_BUFFER(ಸಂರಚನಾ umass_bbb_cbw, cbw, 1);

	dir_in = US_DIRECTION(srb->cmd[0]);

#ifdef BBB_COMDAT_TRACE
	printf("dir %d lun %d cmdlen %d cmd %p datalen %lu pdata %p\n",
		dir_in, srb->lun, srb->cmdlen, srb->cmd, srb->datalen,
		srb->pdata);
	ಯದಿ (srb->cmdlen) {
		ಪ್ರತಿ (result = 0; result < srb->cmdlen; result++)
			printf("cmd[%d] %#x ", result, srb->cmd[result]);
		printf("\n");
	}
#endif
	/* sanity checks */
	ಯದಿ (!(srb->cmdlen <= CBWCDBLENGTH)) {
		debug("usb_stor_BBB_comdat:cmdlen too large\n");
		ಪ್ರತ್ಯಾಗಮ -1;
	}

	/* always OUT to the ep */
	pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_out);

	cbw->dCBWSignature = cpu_to_le32(CBWSIGNATURE);
	cbw->dCBWTag = cpu_to_le32(CBWTag++);
	cbw->dCBWDataTransferLength = cpu_to_le32(srb->datalen);
	cbw->bCBWFlags = (dir_in ? CBWFLAGS_IN : CBWFLAGS_OUT);
	cbw->bCBWLUN = srb->lun;
	cbw->bCDBLength = srb->cmdlen;
	/* copy the command data into the CBW command data buffer */
	/* DST SRC LEN!!! */

	memcpy(cbw->CBWCDB, srb->cmd, srb->cmdlen);
	result = usb_bulk_ಸಂದೇಶ(us->pusb_dev, pipe, cbw, UMASS_BBB_CBW_SIZE,
			      &actlen, USB_CNTL_TIMEOUT * 5);
	ಯದಿ (result < 0)
		debug("usb_stor_BBB_comdat:usb_bulk_msg error\n");
	ಪ್ರತ್ಯಾಗಮ result;
}

/* FIXME: we also need a CBI_command which sets up the completion
 * interrupt, and waits for it
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_stor_CB_comdat(ಸಂರಚನಾ scsi_cmd *srb, ಸಂರಚನಾ us_data *us)
{
	ಪೂರ್ಣಾಙ್ಕ result = 0;
	ಪೂರ್ಣಾಙ್ಕ dir_in, retry;
	unsigned ಪೂರ್ಣಾಙ್ಕ pipe;
	unsigned long status;

	retry = 5;
	dir_in = US_DIRECTION(srb->cmd[0]);

	ಯದಿ (dir_in)
		pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);
	ಅನ್ಯಥಾ
		pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_out);

	ಯಾವತ್ (retry--) {
		debug("CBI gets a command: Try %d\n", 5 - retry);
#ifdef DEBUG
		usb_show_srb(srb);
#endif
		/* let's send the command via the control pipe */
		result = usb_control_ಸಂದೇಶ(us->pusb_dev,
					 usb_sndctrlpipe(us->pusb_dev , 0),
					 US_CBI_ADSC,
					 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
					 0, us->ifnum,
					 srb->cmd, srb->cmdlen,
					 USB_CNTL_TIMEOUT * 5);
		debug("CB_transport: control msg returned %d, status %lX\n",
		      result, us->pusb_dev->status);
		/* check the return code for the command */
		ಯದಿ (result < 0) {
			ಯದಿ (us->pusb_dev->status & USB_ST_STALLED) {
				status = us->pusb_dev->status;
				debug(" stall during command found," \
				      " clear pipe\n");
				usb_clear_halt(us->pusb_dev,
					      usb_sndctrlpipe(us->pusb_dev, 0));
				us->pusb_dev->status = status;
			}
			debug(" error during command %02X" \
			      " Stat = %lX\n", srb->cmd[0],
			      us->pusb_dev->status);
			ಪ್ರತ್ಯಾಗಮ result;
		}
		/* transfer the data payload for this command, if one exists*/

		debug("CB_transport: control msg returned %d," \
		      " direction is %s to go 0x%lx\n", result,
		      dir_in ? "IN" : "OUT", srb->datalen);
		ಯದಿ (srb->datalen) {
			result = us_one_transfer(us, pipe, (ವರ್ಣ *)srb->pdata,
						 srb->datalen);
			debug("CBI attempted to transfer data," \
			      " result is %d status %lX, len %d\n",
			      result, us->pusb_dev->status,
				us->pusb_dev->act_len);
			ಯದಿ (!(us->pusb_dev->status & USB_ST_NAK_REC))
				ಭಙ್ಗ;
		} /* if (srb->datalen) */
		ಅನ್ಯಥಾ
			ಭಙ್ಗ;
	}
	/* return result */

	ಪ್ರತ್ಯಾಗಮ result;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_stor_CBI_get_status(ಸಂರಚನಾ scsi_cmd *srb, ಸಂರಚನಾ us_data *us)
{
	ಪೂರ್ಣಾಙ್ಕ timeout;

	us->ip_wanted = 1;
	usb_int_ಸಂದೇಶ(us->pusb_dev, us->irqpipe,
		    (ಶೂನ್ಯ *)&us->ip_data, us->irqmaxp, us->irqinterval, false);
	timeout = 1000;
	ಯಾವತ್ (timeout--) {
		ಯದಿ (us->ip_wanted == 0)
			ಭಙ್ಗ;
		mdelay(10);
	}
	ಯದಿ (us->ip_wanted) {
		printf("	Did not get interrupt on CBI\n");
		us->ip_wanted = 0;
		ಪ್ರತ್ಯಾಗಮ USB_STOR_TRANSPORT_ERROR;
	}
	debug("Got interrupt data 0x%x, transferred %d status 0x%lX\n",
	      us->ip_data, us->pusb_dev->irq_act_len,
	      us->pusb_dev->irq_status);
	/* UFI gives us ASC and ASCQ, like a request sense */
	ಯದಿ (us->subclass == US_SC_UFI) {
		ಯದಿ (srb->cmd[0] == SCSI_REQ_SENSE ||
		    srb->cmd[0] == SCSI_INQUIRY)
			ಪ್ರತ್ಯಾಗಮ USB_STOR_TRANSPORT_GOOD; /* Good */
		ಅನ್ಯಥಾ ಯದಿ (us->ip_data)
			ಪ್ರತ್ಯಾಗಮ USB_STOR_TRANSPORT_FAILED;
		ಅನ್ಯಥಾ
			ಪ್ರತ್ಯಾಗಮ USB_STOR_TRANSPORT_GOOD;
	}
	/* otherwise, we interpret the data normally */
	ವಿಕಲ್ಪ (us->ip_data) {
	ಪಕ್ಷ 0x0001:
		ಪ್ರತ್ಯಾಗಮ USB_STOR_TRANSPORT_GOOD;
	ಪಕ್ಷ 0x0002:
		ಪ್ರತ್ಯಾಗಮ USB_STOR_TRANSPORT_FAILED;
	ಸ್ವಭಾವ:
		ಪ್ರತ್ಯಾಗಮ USB_STOR_TRANSPORT_ERROR;
	}			/* switch */
	ಪ್ರತ್ಯಾಗಮ USB_STOR_TRANSPORT_ERROR;
}

#define USB_TRANSPORT_UNKNOWN_RETRY 5
#define USB_TRANSPORT_NOT_READY_RETRY 10

/* clear a stall on an endpoint - special for BBB devices */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_stor_BBB_clear_endpt_stall(ಸಂರಚನಾ us_data *us, __u8 endpt)
{
	/* ENDPOINT_HALT = 0, so set value to 0 */
	ಪ್ರತ್ಯಾಗಮ usb_control_ಸಂದೇಶ(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
			       USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
			       endpt, ರಿಕ್ತ, 0, USB_CNTL_TIMEOUT * 5);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_stor_BBB_transport(ಸಂರಚನಾ scsi_cmd *srb, ಸಂರಚನಾ us_data *us)
{
	ಪೂರ್ಣಾಙ್ಕ result, retry;
	ಪೂರ್ಣಾಙ್ಕ dir_in;
	ಪೂರ್ಣಾಙ್ಕ actlen, data_actlen;
	unsigned ಪೂರ್ಣಾಙ್ಕ pipe, pipein, pipeout;
	ALLOC_CACHE_ALIGN_BUFFER(ಸಂರಚನಾ umass_bbb_csw, csw, 1);
#ifdef BBB_XPORT_TRACE
	unsigned ವರ್ಣ *ptr;
	ಪೂರ್ಣಾಙ್ಕ index;
#endif

	dir_in = US_DIRECTION(srb->cmd[0]);

	/* COMMAND phase */
	debug("COMMAND phase\n");
	result = usb_stor_BBB_comdat(srb, us);
	ಯದಿ (result < 0) {
		debug("failed to send CBW status %ld\n",
		      us->pusb_dev->status);
		usb_stor_BBB_reset(us);
		ಪ್ರತ್ಯಾಗಮ USB_STOR_TRANSPORT_FAILED;
	}
	ಯದಿ (!(us->flags & USB_READY))
		mdelay(5);
	pipein = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);
	pipeout = usb_sndbulkpipe(us->pusb_dev, us->ep_out);
	/* DATA phase + error handling */
	data_actlen = 0;
	/* no data, go immediately to the STATUS phase */
	ಯದಿ (srb->datalen == 0)
		ಗಚ್ಛ st;
	debug("DATA phase\n");
	ಯದಿ (dir_in)
		pipe = pipein;
	ಅನ್ಯಥಾ
		pipe = pipeout;

	result = usb_bulk_ಸಂದೇಶ(us->pusb_dev, pipe, srb->pdata, srb->datalen,
			      &data_actlen, USB_CNTL_TIMEOUT * 5);
	/* special handling of STALL in DATA phase */
	ಯದಿ ((result < 0) && (us->pusb_dev->status & USB_ST_STALLED)) {
		debug("DATA:stall\n");
		/* clear the STALL on the endpoint */
		result = usb_stor_BBB_clear_endpt_stall(us,
					dir_in ? us->ep_in : us->ep_out);
		ಯದಿ (result >= 0)
			/* continue on to STATUS phase */
			ಗಚ್ಛ st;
	}
	ಯದಿ (result < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->status);
		usb_stor_BBB_reset(us);
		ಪ್ರತ್ಯಾಗಮ USB_STOR_TRANSPORT_FAILED;
	}
#ifdef BBB_XPORT_TRACE
	ಪ್ರತಿ (index = 0; index < data_actlen; index++)
		printf("pdata[%d] %#x ", index, srb->pdata[index]);
	printf("\n");
#endif
	/* STATUS phase + error handling */
st:
	retry = 0;
again:
	debug("STATUS phase\n");
	result = usb_bulk_ಸಂದೇಶ(us->pusb_dev, pipein, csw, UMASS_BBB_CSW_SIZE,
				&actlen, USB_CNTL_TIMEOUT*5);

	/* special handling of STALL in STATUS phase */
	ಯದಿ ((result < 0) && (retry < 1) &&
	    (us->pusb_dev->status & USB_ST_STALLED)) {
		debug("STATUS:stall\n");
		/* clear the STALL on the endpoint */
		result = usb_stor_BBB_clear_endpt_stall(us, us->ep_in);
		ಯದಿ (result >= 0 && (retry++ < 1))
			/* do a retry */
			ಗಚ್ಛ again;
	}
	ಯದಿ (result < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->status);
		usb_stor_BBB_reset(us);
		ಪ್ರತ್ಯಾಗಮ USB_STOR_TRANSPORT_FAILED;
	}
#ifdef BBB_XPORT_TRACE
	ptr = (unsigned ವರ್ಣ *)csw;
	ಪ್ರತಿ (index = 0; index < UMASS_BBB_CSW_SIZE; index++)
		printf("ptr[%d] %#x ", index, ptr[index]);
	printf("\n");
#endif
	/* misuse pipe to get the residue */
	pipe = le32_to_cpu(csw->dCSWDataResidue);
	ಯದಿ (pipe == 0 && srb->datalen != 0 && srb->datalen - data_actlen != 0)
		pipe = srb->datalen - data_actlen;
	ಯದಿ (CSWSIGNATURE != le32_to_cpu(csw->dCSWSignature)) {
		debug("!CSWSIGNATURE\n");
		usb_stor_BBB_reset(us);
		ಪ್ರತ್ಯಾಗಮ USB_STOR_TRANSPORT_FAILED;
	} ಅನ್ಯಥಾ ಯದಿ ((CBWTag - 1) != le32_to_cpu(csw->dCSWTag)) {
		debug("!Tag\n");
		usb_stor_BBB_reset(us);
		ಪ್ರತ್ಯಾಗಮ USB_STOR_TRANSPORT_FAILED;
	} ಅನ್ಯಥಾ ಯದಿ (csw->bCSWStatus > CSWSTATUS_PHASE) {
		debug(">PHASE\n");
		usb_stor_BBB_reset(us);
		ಪ್ರತ್ಯಾಗಮ USB_STOR_TRANSPORT_FAILED;
	} ಅನ್ಯಥಾ ಯದಿ (csw->bCSWStatus == CSWSTATUS_PHASE) {
		debug("=PHASE\n");
		usb_stor_BBB_reset(us);
		ಪ್ರತ್ಯಾಗಮ USB_STOR_TRANSPORT_FAILED;
	} ಅನ್ಯಥಾ ಯದಿ (data_actlen > srb->datalen) {
		debug("transferred %dB instead of %ldB\n",
		      data_actlen, srb->datalen);
		ಪ್ರತ್ಯಾಗಮ USB_STOR_TRANSPORT_FAILED;
	} ಅನ್ಯಥಾ ಯದಿ (csw->bCSWStatus == CSWSTATUS_FAILED) {
		debug("FAILED\n");
		ಪ್ರತ್ಯಾಗಮ USB_STOR_TRANSPORT_FAILED;
	}

	ಪ್ರತ್ಯಾಗಮ result;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_stor_CB_transport(ಸಂರಚನಾ scsi_cmd *srb, ಸಂರಚನಾ us_data *us)
{
	ಪೂರ್ಣಾಙ್ಕ result, status;
	ಸಂರಚನಾ scsi_cmd *psrb;
	ಸಂರಚನಾ scsi_cmd reqsrb;
	ಪೂರ್ಣಾಙ್ಕ retry, notready;

	psrb = &reqsrb;
	status = USB_STOR_TRANSPORT_GOOD;
	retry = 0;
	notready = 0;
	/* issue the command */
do_retry:
	result = usb_stor_CB_comdat(srb, us);
	debug("command / Data returned %d, status %lX\n",
	      result, us->pusb_dev->status);
	/* if this is an CBI Protocol, get IRQ */
	ಯದಿ (us->protocol == US_PR_CBI) {
		status = usb_stor_CBI_get_status(srb, us);
		/* if the status is error, report it */
		ಯದಿ (status == USB_STOR_TRANSPORT_ERROR) {
			debug(" USB CBI Command Error\n");
			ಪ್ರತ್ಯಾಗಮ status;
		}
		srb->sense_buf[12] = (unsigned ವರ್ಣ)(us->ip_data >> 8);
		srb->sense_buf[13] = (unsigned ವರ್ಣ)(us->ip_data & 0xff);
		ಯದಿ (!us->ip_data) {
			/* if the status is good, report it */
			ಯದಿ (status == USB_STOR_TRANSPORT_GOOD) {
				debug(" USB CBI Command Good\n");
				ಪ್ರತ್ಯಾಗಮ status;
			}
		}
	}
	/* do we have to issue an auto request? */
	/* HERE we have to check the result */
	ಯದಿ ((result < 0) && !(us->pusb_dev->status & USB_ST_STALLED)) {
		debug("ERROR %lX\n", us->pusb_dev->status);
		us->transport_reset(us);
		ಪ್ರತ್ಯಾಗಮ USB_STOR_TRANSPORT_ERROR;
	}
	ಯದಿ ((us->protocol == US_PR_CBI) &&
	    ((srb->cmd[0] == SCSI_REQ_SENSE) ||
	    (srb->cmd[0] == SCSI_INQUIRY))) {
		/* do not issue an autorequest after request sense */
		debug("No auto request and good\n");
		ಪ್ರತ್ಯಾಗಮ USB_STOR_TRANSPORT_GOOD;
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
	result = usb_stor_CB_comdat(psrb, us);
	debug("auto request returned %d\n", result);
	/* if this is an CBI Protocol, get IRQ */
	ಯದಿ (us->protocol == US_PR_CBI)
		status = usb_stor_CBI_get_status(psrb, us);

	ಯದಿ ((result < 0) && !(us->pusb_dev->status & USB_ST_STALLED)) {
		debug(" AUTO REQUEST ERROR %ld\n",
		      us->pusb_dev->status);
		ಪ್ರತ್ಯಾಗಮ USB_STOR_TRANSPORT_ERROR;
	}
	debug("autorequest returned 0x%02X 0x%02X 0x%02X 0x%02X\n",
	      srb->sense_buf[0], srb->sense_buf[2],
	      srb->sense_buf[12], srb->sense_buf[13]);
	/* Check the auto request result */
	ಯದಿ ((srb->sense_buf[2] == 0) &&
	    (srb->sense_buf[12] == 0) &&
	    (srb->sense_buf[13] == 0)) {
		/* ok, no sense */
		ಪ್ರತ್ಯಾಗಮ USB_STOR_TRANSPORT_GOOD;
	}

	/* Check the auto request result */
	ವಿಕಲ್ಪ (srb->sense_buf[2]) {
	ಪಕ್ಷ 0x01:
		/* Recovered Error */
		ಪ್ರತ್ಯಾಗಮ USB_STOR_TRANSPORT_GOOD;
		ಭಙ್ಗ;
	ಪಕ್ಷ 0x02:
		/* Not Ready */
		ಯದಿ (notready++ > USB_TRANSPORT_NOT_READY_RETRY) {
			printf("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X (NOT READY)\n", srb->cmd[0],
				srb->sense_buf[0], srb->sense_buf[2],
				srb->sense_buf[12], srb->sense_buf[13]);
			ಪ್ರತ್ಯಾಗಮ USB_STOR_TRANSPORT_FAILED;
		} ಅನ್ಯಥಾ {
			mdelay(100);
			ಗಚ್ಛ do_retry;
		}
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಯದಿ (retry++ > USB_TRANSPORT_UNKNOWN_RETRY) {
			printf("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X\n", srb->cmd[0], srb->sense_buf[0],
				srb->sense_buf[2], srb->sense_buf[12],
				srb->sense_buf[13]);
			ಪ್ರತ್ಯಾಗಮ USB_STOR_TRANSPORT_FAILED;
		} ಅನ್ಯಥಾ
			ಗಚ್ಛ do_retry;
		ಭಙ್ಗ;
	}
	ಪ್ರತ್ಯಾಗಮ USB_STOR_TRANSPORT_FAILED;
}

ಸ್ಥಿರ ಶೂನ್ಯ usb_stor_set_max_xfer_blk(ಸಂರಚನಾ usb_device *udev,
				      ಸಂರಚನಾ us_data *us)
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
	unsigned short blk = 240;

#ಯದಿ CONFIG_IS_ENABLED(DM_USB)
	size_t size;
	ಪೂರ್ಣಾಙ್ಕ ret;

	ret = usb_get_max_xfer_size(udev, (size_t *)&size);
	ಯದಿ ((ret >= 0) && (size < blk * 512))
		blk = size / 512;
#endif

	us->max_xfer_blk = blk;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_inquiry(ಸಂರಚನಾ scsi_cmd *srb, ಸಂರಚನಾ us_data *ss)
{
	ಪೂರ್ಣಾಙ್ಕ retry, i;
	retry = 5;
	ಕುರು {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_INQUIRY;
		srb->cmd[1] = srb->lun << 5;
		srb->cmd[4] = 36;
		srb->datalen = 36;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		i = ss->transport(srb, ss);
		debug("inquiry returns %d\n", i);
		ಯದಿ (i == 0)
			ಭಙ್ಗ;
	} ಯಾವತ್ (--retry);

	ಯದಿ (!retry) {
		printf("error in inquiry\n");
		ಪ್ರತ್ಯಾಗಮ -1;
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_request_sense(ಸಂರಚನಾ scsi_cmd *srb, ಸಂರಚನಾ us_data *ss)
{
	ವರ್ಣ *ptr;

	ptr = (ವರ್ಣ *)srb->pdata;
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
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_test_unit_ready(ಸಂರಚನಾ scsi_cmd *srb, ಸಂರಚನಾ us_data *ss)
{
	ಪೂರ್ಣಾಙ್ಕ retries = 10;

	ಕುರು {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_TST_U_RDY;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 0;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		ಯದಿ (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD) {
			ss->flags |= USB_READY;
			ಪ್ರತ್ಯಾಗಮ 0;
		}
		usb_request_sense(srb, ss);
		/*
		 * Check the Key Code Qualifier, if it matches
		 * "Not Ready - medium not present"
		 * (the sense Key equals 0x2 and the ASC is 0x3a)
		 * return immediately as the medium being absent won't change
		 * unless there is a user action.
		 */
		ಯದಿ ((srb->sense_buf[2] == 0x02) &&
		    (srb->sense_buf[12] == 0x3a))
			ಪ್ರತ್ಯಾಗಮ -1;
		mdelay(100);
	} ಯಾವತ್ (retries--);

	ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_read_capacity(ಸಂರಚನಾ scsi_cmd *srb, ಸಂರಚನಾ us_data *ss)
{
	ಪೂರ್ಣಾಙ್ಕ retry;
	/* XXX retries */
	retry = 3;
	ಕುರು {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_RD_CAPAC;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 8;
		srb->cmdlen = ss->cmd12 ? 12 : 10;
		ಯದಿ (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD)
			ಪ್ರತ್ಯಾಗಮ 0;
	} ಯಾವತ್ (retry--);

	ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_read_10(ಸಂರಚನಾ scsi_cmd *srb, ಸಂರಚನಾ us_data *ss,
		       unsigned long start, unsigned short blocks)
{
	memset(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_READ10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((unsigned ವರ್ಣ) (start >> 24)) & 0xff;
	srb->cmd[3] = ((unsigned ವರ್ಣ) (start >> 16)) & 0xff;
	srb->cmd[4] = ((unsigned ವರ್ಣ) (start >> 8)) & 0xff;
	srb->cmd[5] = ((unsigned ವರ್ಣ) (start)) & 0xff;
	srb->cmd[7] = ((unsigned ವರ್ಣ) (blocks >> 8)) & 0xff;
	srb->cmd[8] = (unsigned ವರ್ಣ) blocks & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("read10: start %lx blocks %x\n", start, blocks);
	ಪ್ರತ್ಯಾಗಮ ss->transport(srb, ss);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_write_10(ಸಂರಚನಾ scsi_cmd *srb, ಸಂರಚನಾ us_data *ss,
			unsigned long start, unsigned short blocks)
{
	memset(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_WRITE10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((unsigned ವರ್ಣ) (start >> 24)) & 0xff;
	srb->cmd[3] = ((unsigned ವರ್ಣ) (start >> 16)) & 0xff;
	srb->cmd[4] = ((unsigned ವರ್ಣ) (start >> 8)) & 0xff;
	srb->cmd[5] = ((unsigned ವರ್ಣ) (start)) & 0xff;
	srb->cmd[7] = ((unsigned ವರ್ಣ) (blocks >> 8)) & 0xff;
	srb->cmd[8] = (unsigned ವರ್ಣ) blocks & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("write10: start %lx blocks %x\n", start, blocks);
	ಪ್ರತ್ಯಾಗಮ ss->transport(srb, ss);
}

#ifdef CONFIG_USB_BIN_FIXUP
/*
 * Some USB storage devices queried for SCSI identification data respond with
 * binary strings, which if output to the console freeze the terminal. The
 * workaround is to modify the vendor and product strings read from such
 * device with proper values (as reported by 'usb info').
 *
 * Vendor and product length limits are taken from the definition of
 * struct blk_desc in include/part.h.
 */
ಸ್ಥಿರ ಶೂನ್ಯ usb_bin_fixup(ಸಂರಚನಾ usb_device_descriptor descriptor,
				unsigned ವರ್ಣ vendor[],
				unsigned ವರ್ಣ product[]) {
	ಧ್ರುವ unsigned ವರ್ಣ max_vendor_len = 40;
	ಧ್ರುವ unsigned ವರ್ಣ max_product_len = 20;
	ಯದಿ (descriptor.idVendor == 0x0424 && descriptor.idProduct == 0x223a) {
		strncpy((ವರ್ಣ *)vendor, "SMSC", max_vendor_len);
		strncpy((ವರ್ಣ *)product, "Flash Media Cntrller",
			max_product_len);
	}
}
#endif /* CONFIG_USB_BIN_FIXUP */

#ಯದಿ CONFIG_IS_ENABLED(BLK)
ಸ್ಥಿರ unsigned long usb_stor_read(ಸಂರಚನಾ udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, ಶೂನ್ಯ *buffer)
#ಅನ್ಯಥಾ
ಸ್ಥಿರ unsigned long usb_stor_read(ಸಂರಚನಾ blk_desc *block_dev, lbaint_t blknr,
				   lbaint_t blkcnt, ಶೂನ್ಯ *buffer)
#endif
{
	lbaint_t start, blks;
	uintptr_t buf_addr;
	unsigned short smallblks;
	ಸಂರಚನಾ usb_device *udev;
	ಸಂರಚನಾ us_data *ss;
	ಪೂರ್ಣಾಙ್ಕ retry;
	ಸಂರಚನಾ scsi_cmd *srb = &usb_ccb;
#ಯದಿ CONFIG_IS_ENABLED(BLK)
	ಸಂರಚನಾ blk_desc *block_dev;
#endif

	ಯದಿ (blkcnt == 0)
		ಪ್ರತ್ಯಾಗಮ 0;
	/* Setup  device */
#ಯದಿ CONFIG_IS_ENABLED(BLK)
	block_dev = dev_get_uclass_plat(dev);
	udev = dev_get_parent_priv(dev_get_parent(dev));
	debug("\nusb_read: udev %d\n", block_dev->devnum);
#ಅನ್ಯಥಾ
	debug("\nusb_read: udev %d\n", block_dev->devnum);
	udev = usb_dev_desc[block_dev->devnum].priv;
	ಯದಿ (!udev) {
		debug("%s: No device\n", __func__);
		ಪ್ರತ್ಯಾಗಮ 0;
	}
#endif
	ss = (ಸಂರಚನಾ us_data *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_lock_async(udev, 1);
	srb->lun = block_dev->lun;
	buf_addr = (uintptr_t)buffer;
	start = blknr;
	blks = blkcnt;

	debug("\nusb_read: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      block_dev->devnum, start, blks, buf_addr);

	ಕುರು {
		/* XXX need some comment here */
		retry = 2;
		srb->pdata = (unsigned ವರ್ಣ *)buf_addr;
		ಯದಿ (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		ಅನ್ಯಥಾ
			smallblks = (unsigned short) blks;
retry_it:
		ಯದಿ (smallblks == ss->max_xfer_blk)
			usb_show_progress();
		srb->datalen = block_dev->blksz * smallblks;
		srb->pdata = (unsigned ವರ್ಣ *)buf_addr;
		ಯದಿ (usb_read_10(srb, ss, start, smallblks)) {
			debug("Read ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			ಯದಿ (retry--)
				ಗಚ್ಛ retry_it;
			blkcnt -= blks;
			ಭಙ್ಗ;
		}
		start += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} ಯಾವತ್ (blks != 0);

	debug("usb_read: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      start, smallblks, buf_addr);

	usb_lock_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	ಯದಿ (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	ಪ್ರತ್ಯಾಗಮ blkcnt;
}

#ಯದಿ CONFIG_IS_ENABLED(BLK)
ಸ್ಥಿರ unsigned long usb_stor_write(ಸಂರಚನಾ udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, ಧ್ರುವ ಶೂನ್ಯ *buffer)
#ಅನ್ಯಥಾ
ಸ್ಥಿರ unsigned long usb_stor_write(ಸಂರಚನಾ blk_desc *block_dev, lbaint_t blknr,
				    lbaint_t blkcnt, ಧ್ರುವ ಶೂನ್ಯ *buffer)
#endif
{
	lbaint_t start, blks;
	uintptr_t buf_addr;
	unsigned short smallblks;
	ಸಂರಚನಾ usb_device *udev;
	ಸಂರಚನಾ us_data *ss;
	ಪೂರ್ಣಾಙ್ಕ retry;
	ಸಂರಚನಾ scsi_cmd *srb = &usb_ccb;
#ಯದಿ CONFIG_IS_ENABLED(BLK)
	ಸಂರಚನಾ blk_desc *block_dev;
#endif

	ಯದಿ (blkcnt == 0)
		ಪ್ರತ್ಯಾಗಮ 0;

	/* Setup  device */
#ಯದಿ CONFIG_IS_ENABLED(BLK)
	block_dev = dev_get_uclass_plat(dev);
	udev = dev_get_parent_priv(dev_get_parent(dev));
	debug("\nusb_read: udev %d\n", block_dev->devnum);
#ಅನ್ಯಥಾ
	debug("\nusb_read: udev %d\n", block_dev->devnum);
	udev = usb_dev_desc[block_dev->devnum].priv;
	ಯದಿ (!udev) {
		debug("%s: No device\n", __func__);
		ಪ್ರತ್ಯಾಗಮ 0;
	}
#endif
	ss = (ಸಂರಚನಾ us_data *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_lock_async(udev, 1);

	srb->lun = block_dev->lun;
	buf_addr = (uintptr_t)buffer;
	start = blknr;
	blks = blkcnt;

	debug("\nusb_write: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      block_dev->devnum, start, blks, buf_addr);

	ಕುರು {
		/* If write fails retry for max retry count else
		 * return with number of blocks written successfully.
		 */
		retry = 2;
		srb->pdata = (unsigned ವರ್ಣ *)buf_addr;
		ಯದಿ (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		ಅನ್ಯಥಾ
			smallblks = (unsigned short) blks;
retry_it:
		ಯದಿ (smallblks == ss->max_xfer_blk)
			usb_show_progress();
		srb->datalen = block_dev->blksz * smallblks;
		srb->pdata = (unsigned ವರ್ಣ *)buf_addr;
		ಯದಿ (usb_write_10(srb, ss, start, smallblks)) {
			debug("Write ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			ಯದಿ (retry--)
				ಗಚ್ಛ retry_it;
			blkcnt -= blks;
			ಭಙ್ಗ;
		}
		start += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} ಯಾವತ್ (blks != 0);

	debug("usb_write: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      start, smallblks, buf_addr);

	usb_lock_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	ಯದಿ (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	ಪ್ರತ್ಯಾಗಮ blkcnt;

}

/* Probe to see if a new device is actually a Storage device */
ಪೂರ್ಣಾಙ್ಕ usb_storage_probe(ಸಂರಚನಾ usb_device *dev, unsigned ಪೂರ್ಣಾಙ್ಕ ifnum,
		      ಸಂರಚನಾ us_data *ss)
{
	ಸಂರಚನಾ usb_interface *iface;
	ಪೂರ್ಣಾಙ್ಕ i;
	ಸಂರಚನಾ usb_endpoint_descriptor *ep_desc;
	unsigned ಪೂರ್ಣಾಙ್ಕ flags = 0;

	/* let's examine the device now */
	iface = &dev->config.if_desc[ifnum];

	ಯದಿ (dev->descriptor.bDeviceClass != 0 ||
			iface->desc.bInterfaceClass != USB_CLASS_MASS_STORAGE ||
			iface->desc.bInterfaceSubClass < US_SC_MIN ||
			iface->desc.bInterfaceSubClass > US_SC_MAX) {
		debug("Not mass storage\n");
		/* if it's not a mass storage, we go no further */
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	memset(ss, 0, ಪರಿಮಾಣ(ಸಂರಚನಾ us_data));

	/* At this point, we know we've got a live one */
	debug("\n\nUSB Mass Storage device detected\n");

	/* Initialize the us_data structure with some useful info */
	ss->flags = flags;
	ss->ifnum = ifnum;
	ss->pusb_dev = dev;
	ss->attention_done = 0;
	ss->subclass = iface->desc.bInterfaceSubClass;
	ss->protocol = iface->desc.bInterfaceProtocol;

	/* set the handler pointers based on the protocol */
	debug("Transport: ");
	ವಿಕಲ್ಪ (ss->protocol) {
	ಪಕ್ಷ US_PR_CB:
		debug("Control/Bulk\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		ಭಙ್ಗ;

	ಪಕ್ಷ US_PR_CBI:
		debug("Control/Bulk/Interrupt\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		ಭಙ್ಗ;
	ಪಕ್ಷ US_PR_BULK:
		debug("Bulk/Bulk/Bulk\n");
		ss->transport = usb_stor_BBB_transport;
		ss->transport_reset = usb_stor_BBB_reset;
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		printf("USB Storage Transport unknown / not yet implemented\n");
		ಪ್ರತ್ಯಾಗಮ 0;
		ಭಙ್ಗ;
	}

	/*
	 * We are expecting a minimum of 2 endpoints - in and out (bulk).
	 * An optional interrupt is OK (necessary for CBI protocol).
	 * We will ignore any others.
	 */
	ಪ್ರತಿ (i = 0; i < iface->desc.b_ಅಂಕೆ_Endpoints; i++) {
		ep_desc = &iface->ep_desc[i];
		/* is it an BULK endpoint? */
		ಯದಿ ((ep_desc->bmAttributes &
		     USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK) {
			ಯದಿ (ep_desc->bEndpointAddress & USB_DIR_IN)
				ss->ep_in = ep_desc->bEndpointAddress &
						USB_ENDPOINT_ಸಂಖ್ಯೆ_MASK;
			ಅನ್ಯಥಾ
				ss->ep_out =
					ep_desc->bEndpointAddress &
					USB_ENDPOINT_ಸಂಖ್ಯೆ_MASK;
		}

		/* is it an interrupt endpoint? */
		ಯದಿ ((ep_desc->bmAttributes &
		     USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT) {
			ss->ep_int = ep_desc->bEndpointAddress &
						USB_ENDPOINT_ಸಂಖ್ಯೆ_MASK;
			ss->irqinterval = ep_desc->bInterval;
		}
	}
	debug("Endpoints In %d Out %d Int %d\n",
	      ss->ep_in, ss->ep_out, ss->ep_int);

	/* Do some basic sanity checks, and bail if we find a problem */
	ಯದಿ (usb_set_interface(dev, iface->desc.b_Interface_ಸಂಖ್ಯೆ, 0) ||
	    !ss->ep_in || !ss->ep_out ||
	    (ss->protocol == US_PR_CBI && ss->ep_int == 0)) {
		debug("Problems with device\n");
		ಪ್ರತ್ಯಾಗಮ 0;
	}
	/* set class specific stuff */
	/* We only handle certain protocols.  Currently, these are
	 * the only ones.
	 * The SFF8070 accepts the requests used in u-boot
	 */
	ಯದಿ (ss->subclass != US_SC_UFI && ss->subclass != US_SC_SCSI &&
	    ss->subclass != US_SC_8070) {
		printf("Sorry, protocol %d not yet supported.\n", ss->subclass);
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	/* UFI uses 12-byte commands (like RBC, unlike SCSI) */
	ಯದಿ (ss->subclass == US_SC_UFI)
		ss->cmd12 = true;

	ಯದಿ (ss->ep_int) {
		/* we had found an interrupt endpoint, prepare irq pipe
		 * set up the IRQ pipe and handler
		 */
		ss->irqinterval = (ss->irqinterval > 0) ? ss->irqinterval : 255;
		ss->irqpipe = usb_rcvintpipe(ss->pusb_dev, ss->ep_int);
		ss->irqmaxp = usb_maxpacket(dev, ss->irqpipe);
		dev->irq_handle = usb_stor_irq;
	}

	/* Set the maximum transfer size per host controller setting */
	usb_stor_set_max_xfer_blk(dev, ss);

	dev->privptr = (ಶೂನ್ಯ *)ss;
	ಪ್ರತ್ಯಾಗಮ 1;
}

ಪೂರ್ಣಾಙ್ಕ usb_stor_get_info(ಸಂರಚನಾ usb_device *dev, ಸಂರಚನಾ us_data *ss,
		      ಸಂರಚನಾ blk_desc *dev_desc)
{
	unsigned ವರ್ಣ perq, modi;
	ALLOC_CACHE_ALIGN_BUFFER(u32, cap, 2);
	ALLOC_CACHE_ALIGN_BUFFER(u8, usb_stor_buf, 36);
	u32 capacity, blksz;
	ಸಂರಚನಾ scsi_cmd *pccb = &usb_ccb;

	pccb->pdata = usb_stor_buf;

	dev_desc->target = dev->devnum;
	pccb->lun = dev_desc->lun;
	debug(" address %d\n", dev_desc->target);

	ಯದಿ (usb_inquiry(pccb, ss)) {
		debug("%s: usb_inquiry() failed\n", __func__);
		ಪ್ರತ್ಯಾಗಮ -1;
	}

	perq = usb_stor_buf[0];
	modi = usb_stor_buf[1];

	/*
	 * Skip unknown devices (0x1f) and enclosure service devices (0x0d),
	 * they would not respond to test_unit_ready .
	 */
	ಯದಿ (((perq & 0x1f) == 0x1f) || ((perq & 0x1f) == 0x0d)) {
		debug("%s: unknown/unsupported device\n", __func__);
		ಪ್ರತ್ಯಾಗಮ 0;
	}
	ಯದಿ ((modi&0x80) == 0x80) {
		/* drive is removable */
		dev_desc->removable = 1;
	}
	memcpy(dev_desc->vendor, (ಧ್ರುವ ಶೂನ್ಯ *)&usb_stor_buf[8], 8);
	memcpy(dev_desc->product, (ಧ್ರುವ ಶೂನ್ಯ *)&usb_stor_buf[16], 16);
	memcpy(dev_desc->revision, (ಧ್ರುವ ಶೂನ್ಯ *)&usb_stor_buf[32], 4);
	dev_desc->vendor[8] = 0;
	dev_desc->product[16] = 0;
	dev_desc->revision[4] = 0;
#ifdef CONFIG_USB_BIN_FIXUP
	usb_bin_fixup(dev->descriptor, (uchar *)dev_desc->vendor,
		      (uchar *)dev_desc->product);
#endif /* CONFIG_USB_BIN_FIXUP */
	debug("ISO Vers %X, Response Data %X\n", usb_stor_buf[2],
	      usb_stor_buf[3]);
	ಯದಿ (usb_test_unit_ready(pccb, ss)) {
		printf("Device NOT ready\n"
		       "   Request Sense returned %02X %02X %02X\n",
		       pccb->sense_buf[2], pccb->sense_buf[12],
		       pccb->sense_buf[13]);
		ಯದಿ (dev_desc->removable == 1)
			dev_desc->type = perq;
		ಪ್ರತ್ಯಾಗಮ 0;
	}
	pccb->pdata = (unsigned ವರ್ಣ *)cap;
	memset(pccb->pdata, 0, 8);
	ಯದಿ (usb_read_capacity(pccb, ss) != 0) {
		printf("READ_CAP ERROR\n");
		ss->flags &= ~USB_READY;
		cap[0] = 2880;
		cap[1] = 0x200;
	}
	debug("Read Capacity returns: 0x%08x, 0x%08x\n", cap[0], cap[1]);
#ಯದಿ 0
	ಯದಿ (cap[0] > (0x200000 * 10)) /* greater than 10 GByte */
		cap[0] >>= 16;

	cap[0] = cpu_to_be32(cap[0]);
	cap[1] = cpu_to_be32(cap[1]);
#endif

	capacity = be32_to_cpu(cap[0]) + 1;
	blksz = be32_to_cpu(cap[1]);

	debug("Capacity = 0x%08x, blocksz = 0x%08x\n", capacity, blksz);
	dev_desc->lba = capacity;
	dev_desc->blksz = blksz;
	dev_desc->log2blksz = LOG2(dev_desc->blksz);
	dev_desc->type = perq;
	debug(" address %d\n", dev_desc->target);

	ಪ್ರತ್ಯಾಗಮ 1;
}

#ಯದಿ CONFIG_IS_ENABLED(DM_USB)

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ usb_mass_storage_probe(ಸಂರಚನಾ udevice *dev)
{
	ಸಂರಚನಾ usb_device *udev = dev_get_parent_priv(dev);
	ಪೂರ್ಣಾಙ್ಕ ret;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	ret = usb_stor_probe_device(udev);
	usb_disable_asynch(0); /* asynch transfer allowed */

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ udevice_id usb_mass_storage_ids[] = {
	{ .compatible = "usb-mass-storage" },
	{ }
};

U_BOOT_DRIVER(usb_mass_storage) = {
	.name	= "usb_mass_storage",
	.id	= UCLASS_MASS_STORAGE,
	.of_match = usb_mass_storage_ids,
	.probe = usb_mass_storage_probe,
#ಯದಿ CONFIG_IS_ENABLED(BLK)
	.plat_auto	= ಪರಿಮಾಣ(ಸಂರಚನಾ us_data),
#endif
};

UCLASS_DRIVER(usb_mass_storage) = {
	.id		= UCLASS_MASS_STORAGE,
	.name		= "usb_mass_storage",
};

ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ usb_device_id mass_storage_id_table[] = {
	{
		.match_flags = USB_DEVICE_ID_MATCH_INT_CLASS,
		.bInterfaceClass = USB_CLASS_MASS_STORAGE
	},
	{ }		/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_mass_storage, mass_storage_id_table);
#endif

#ಯದಿ CONFIG_IS_ENABLED(BLK)
ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ blk_ops usb_storage_ops = {
	.ಪಠ	= usb_stor_read,
	.ಲಿಖ	= usb_stor_write,
};

U_BOOT_DRIVER(usb_storage_blk) = {
	.name		= "usb_storage_blk",
	.id		= UCLASS_BLK,
	.ops		= &usb_storage_ops,
};
#ಅನ್ಯಥಾ
U_BOOT_LEGACY_BLK(usb) = {
	.uclass_idname	= "usb",
	.uclass_id	= UCLASS_USB,
	.max_devs	= USB_MAX_STOR_DEV,
	.desc		= usb_dev_desc,
};
#endif
