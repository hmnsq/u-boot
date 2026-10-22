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

#அந்தர்பாவ <blk.h>
#அந்தர்பாவ <bootdev.h>
#அந்தர்பாவ <command.h>
#அந்தர்பாவ <dm.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <mapmem.h>
#அந்தர்பாவ <memalign.h>
#அந்தர்பாவ <asm/byteorder.h>
#அந்தர்பாவ <asm/cache.h>
#அந்தர்பாவ <asm/processor.h>
#அந்தர்பாவ <dm/device-internal.h>
#அந்தர்பாவ <dm/lists.h>
#அந்தர்பாவ <linux/delay.h>

#அந்தர்பாவ <part.h>
#அந்தர்பாவ <usb.h>

#undef BBB_COMDAT_TRACE
#undef BBB_XPORT_TRACE

#அந்தர்பாவ <scsi.h>
/* direction table -- this indicates the direction of the data
 * transfer for each command code -- a 1 indicates input
 */
நிலையான மாறிலி unsigned எழுத்து us_direction[256/8] = {
	0x28, 0x81, 0x14, 0x14, 0x20, 0x01, 0x90, 0x77,
	0x0C, 0x20, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#define US_DIRECTION(x) ((us_direction[x>>3] >> (x & 7)) & 1)

நிலையான கட்டமைப்பு scsi_cmd usb_ccb __aligned(ARCH_DMA_MINALIGN);
நிலையான __u32 CBWTag;

நிலையான முழு usb_max_devs; /* number of highest available usb device */

#எனில் !CONFIG_IS_ENABLED(BLK)
நிலையான கட்டமைப்பு blk_desc usb_dev_desc[USB_MAX_STOR_DEV];
#endif

கட்டமைப்பு us_data;
வகைவரை முழு (*trans_cmnd)(கட்டமைப்பு scsi_cmd *cb, கட்டமைப்பு us_data *data);
வகைவரை முழு (*trans_reset)(கட்டமைப்பு us_data *data);

கட்டமைப்பு us_data {
	கட்டமைப்பு usb_device *pusb_dev;	 /* this usb_device */

	unsigned முழு	flags;			/* from filter initially */
#	define USB_READY	(1 << 0)
	unsigned எழுத்து	ifnum;			/* interface number */
	unsigned எழுத்து	ep_in;			/* in endpoint */
	unsigned எழுத்து	ep_out;			/* out ....... */
	unsigned எழுத்து	ep_int;			/* interrupt . */
	unsigned எழுத்து	subclass;		/* as in overview */
	unsigned எழுத்து	protocol;		/* .............. */
	unsigned எழுத்து	attention_done;		/* force attn on first cmd */
	unsigned short	ip_data;		/* interrupt data */
	முழு		action;			/* what to do */
	முழு		ip_wanted;		/* needed */
	முழு		*irq_handle;		/* for USB int requests */
	unsigned முழு	irqpipe;		/* pipe for release_irq */
	unsigned எழுத்து	irqmaxp;		/* max packed for irq Pipe */
	unsigned எழுத்து	irqinterval;		/* Intervall for IRQ Pipe */
	கட்டமைப்பு scsi_cmd	*srb;			/* current srb */
	trans_reset	transport_reset;	/* reset routine */
	trans_cmnd	transport;		/* transport routine */
	unsigned short	max_xfer_blk;		/* maximum transfer blocks */
	bool		cmd12;			/* use 12-byte commands (RBC/UFI) */
};

#எனில் !CONFIG_IS_ENABLED(BLK)
நிலையான கட்டமைப்பு us_data usb_stor[USB_MAX_STOR_DEV];
#endif

#define USB_STOR_TRANSPORT_GOOD	   0
#define USB_STOR_TRANSPORT_FAILED -1
#define USB_STOR_TRANSPORT_ERROR  -2

முழு usb_stor_get_info(கட்டமைப்பு usb_device *dev, கட்டமைப்பு us_data *us,
		      கட்டமைப்பு blk_desc *dev_desc);
முழு usb_storage_probe(கட்டமைப்பு usb_device *dev, unsigned முழு ifnum,
		      கட்டமைப்பு us_data *ss);
#எனில் CONFIG_IS_ENABLED(BLK)
நிலையான unsigned long usb_stor_read(கட்டமைப்பு udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, வெறுமை *buffer);
நிலையான unsigned long usb_stor_write(கட்டமைப்பு udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, மாறிலி வெறுமை *buffer);
#இல்லையெனில்
நிலையான unsigned long usb_stor_read(கட்டமைப்பு blk_desc *block_dev, lbaint_t blknr,
				   lbaint_t blkcnt, வெறுமை *buffer);
நிலையான unsigned long usb_stor_write(கட்டமைப்பு blk_desc *block_dev, lbaint_t blknr,
				    lbaint_t blkcnt, மாறிலி வெறுமை *buffer);
#endif
வெறுமை uhci_show_temp_int_td(வெறுமை);

நிலையான வெறுமை usb_show_progress(வெறுமை)
{
	debug(".");
}

/*******************************************************************************
 * show info on storage devices; 'usb start/init' must be invoked earlier
 * as we only retrieve structures populated during devices initialization
 */
முழு usb_stor_info(வெறுமை)
{
	முழு மொத்தம் = 0;
#எனில் CONFIG_IS_ENABLED(BLK)
	கட்டமைப்பு udevice *dev;

	க்கு (blk_first_device(UCLASS_USB, &dev);
	     dev;
	     blk_next_device(&dev)) {
		கட்டமைப்பு blk_desc *desc = dev_get_uclass_plat(dev);

		printf("  Device %d: ", desc->devnum);
		dev_print(desc);
		மொத்தம்++;
	}
#இல்லையெனில்
	முழு i;

	எனில் (usb_max_devs > 0) {
		க்கு (i = 0; i < usb_max_devs; i++) {
			printf("  Device %d: ", i);
			dev_print(&usb_dev_desc[i]);
		}
		திரும்பு 0;
	}
#endif
	எனில் (!மொத்தம்) {
		printf("No storage devices, perhaps not 'usb start'ed..?\n");
		திரும்பு 1;
	}

	திரும்பு 0;
}

நிலையான unsigned முழு usb_get_max_lun(கட்டமைப்பு us_data *us)
{
	முழு len;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned எழுத்து, result, 1);
	len = usb_control_செய்தி(us->pusb_dev,
			      usb_rcvctrlpipe(us->pusb_dev, 0),
			      US_BBB_GET_MAX_LUN,
			      USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_IN,
			      0, us->ifnum,
			      result, அளவு(எழுத்து),
			      USB_CNTL_TIMEOUT * 5);
	debug("Get Max LUN -> len = %i, result = %i\n", len, (முழு) *result);
	திரும்பு (len > 0) ? *result : 0;
}

நிலையான முழு usb_stor_probe_device(கட்டமைப்பு usb_device *udev)
{
	முழு lun, max_lun;

#எனில் CONFIG_IS_ENABLED(BLK)
	கட்டமைப்பு us_data *data;
	முழு ret;
#இல்லையெனில்
	முழு start;

	எனில் (udev == வெற்று)
		திரும்பு -ENOENT; /* no more devices available */
#endif

	debug("\n\nProbing for storage\n");
#எனில் CONFIG_IS_ENABLED(BLK)
	/*
	 * We store the us_data in the mass storage device's plat. It
	 * is shared by all LUNs (block devices) attached to this mass storage
	 * device.
	 */
	data = dev_get_plat(udev->dev);
	எனில் (!usb_storage_probe(udev, 0, data))
		திரும்பு 0;
	max_lun = usb_get_max_lun(data);
	க்கு (lun = 0; lun <= max_lun; lun++) {
		கட்டமைப்பு blk_desc *blkdev;
		கட்டமைப்பு udevice *dev;
		எழுத்து str[10];

		snprintf(str, அளவு(str), "lun%d", lun);
		ret = blk_create_devicef(udev->dev, "usb_storage_blk", str,
					 UCLASS_USB, usb_max_devs,
					 DEFAULT_BLKSZ, 0, &dev);
		எனில் (ret) {
			debug("Cannot bind driver\n");
			திரும்பு ret;
		}

		blkdev = dev_get_uclass_plat(dev);
		blkdev->target = 0xff;
		blkdev->lun = lun;

		ret = usb_stor_get_info(udev, data, blkdev);
		எனில் (ret == 1) {
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		} இல்லையெனில் {
			debug("usb_stor_get_info: Invalid device\n");
			ret = device_unbind(dev);
			எனில் (ret)
				திரும்பு ret;
			தொடர்;
		}

		ret = blk_probe_or_unbind(dev);
		எனில் (ret)
			திரும்பு ret;

		ret = bootdev_setup_for_sibling_blk(dev, "usb_bootdev");
		எனில் (ret) {
			முழு ret2;

			ret2 = device_unbind(dev);
			எனில் (ret2)
				திரும்பு log_செய்தி_ret("bootdev", ret2);
			திரும்பு log_செய்தி_ret("bootdev", ret);
		}
	}
#இல்லையெனில்
	/* We don't have space to even probe if we hit the maximum */
	எனில் (usb_max_devs == USB_MAX_STOR_DEV) {
		printf("max USB Storage Device reached: %d stopping\n",
		       usb_max_devs);
		திரும்பு -ENOSPC;
	}

	எனில் (!usb_storage_probe(udev, 0, &usb_stor[usb_max_devs]))
		திரும்பு 0;

	/*
	 * OK, it's a storage device.  Iterate over its LUNs and populate
	 * usb_dev_desc'
	 */
	start = usb_max_devs;

	max_lun = usb_get_max_lun(&usb_stor[usb_max_devs]);
	க்கு (lun = 0; lun <= max_lun && usb_max_devs < USB_MAX_STOR_DEV;
	     lun++) {
		கட்டமைப்பு blk_desc *blkdev;

		blkdev = &usb_dev_desc[usb_max_devs];
		memset(blkdev, '\0', அளவு(கட்டமைப்பு blk_desc));
		blkdev->uclass_id = UCLASS_USB;
		blkdev->devnum = usb_max_devs;
		blkdev->part_type = PART_TYPE_UNKNOWN;
		blkdev->target = 0xff;
		blkdev->type = DEV_TYPE_UNKNOWN;
		blkdev->block_read = usb_stor_read;
		blkdev->block_write = usb_stor_write;
		blkdev->lun = lun;
		blkdev->priv = udev;

		எனில் (usb_stor_get_info(udev, &usb_stor[start],
				      &usb_dev_desc[usb_max_devs]) == 1) {
			debug("partype: %d\n", blkdev->part_type);
			part_init(blkdev);
			debug("partype: %d\n", blkdev->part_type);
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		}
	}
#endif

	திரும்பு 0;
}

வெறுமை usb_stor_reset(வெறுமை)
{
	usb_max_devs = 0;
}

/*******************************************************************************
 * scan the usb and reports device info
 * to the user if mode = 1
 * returns current device or -1 if no
 */
முழு usb_stor_scan(முழு mode)
{
	எனில் (mode == 1)
		printf("       scanning usb for storage devices... ");

#எனில் !CONFIG_IS_ENABLED(DM_USB)
	unsigned எழுத்து i;

	usb_disable_asynch(1); /* asynch transfer not allowed */

	usb_stor_reset();
	க்கு (i = 0; i < USB_MAX_DEVICE; i++) {
		கட்டமைப்பு usb_device *dev;

		dev = usb_get_dev_index(i); /* get device */
		debug("i=%d\n", i);
		எனில் (usb_stor_probe_device(dev))
			நிறுத்து;
	} /* for */

	usb_disable_asynch(0); /* asynch transfer allowed */
#endif
	printf("%d Storage Device(s) found\n", usb_max_devs);
	எனில் (usb_max_devs > 0)
		திரும்பு 0;
	திரும்பு -1;
}

நிலையான முழு usb_stor_irq(கட்டமைப்பு usb_device *dev)
{
	கட்டமைப்பு us_data *us;
	us = (கட்டமைப்பு us_data *)dev->privptr;

	எனில் (us->ip_wanted)
		us->ip_wanted = 0;
	திரும்பு 0;
}

#ifdef	DEBUG

நிலையான வெறுமை usb_show_srb(கட்டமைப்பு scsi_cmd *pccb)
{
	முழு i;
	printf("SRB: len %d datalen 0x%lX\n ", pccb->cmdlen, pccb->datalen);
	க்கு (i = 0; i < pccb->cmdlen; i++)
		printf("%02X ", pccb->cmd[i]);
	printf("\n");
}

நிலையான வெறுமை display_int_status(unsigned long tmp)
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

நிலையான முழு us_one_transfer(கட்டமைப்பு us_data *us, முழு pipe, எழுத்து *buf, முழு length)
{
	முழு max_size;
	முழு this_xfer;
	முழு result;
	முழு partial;
	முழு maxtry;
	முழு stat;

	/* determine the maximum packet size for these transfers */
	max_size = usb_maxpacket(us->pusb_dev, pipe) * 16;

	/* while we have data left to transfer */
	போது (length) {

		/* calculate how long this will be -- maximum or a remainder */
		this_xfer = length > max_size ? max_size : length;
		length -= this_xfer;

		/* setup the retry counter */
		maxtry = 10;

		/* set up the transfer loop */
		செய் {
			/* transfer the data */
			debug("Bulk xfer 0x%lx(%d) try #%d\n",
			      (ulong)map_to_sysmem(buf), this_xfer,
			      11 - maxtry);
			result = usb_bulk_செய்தி(us->pusb_dev, pipe, buf,
					      this_xfer, &partial,
					      USB_CNTL_TIMEOUT * 5);
			debug("bulk_msg returned %d xferred %d/%d\n",
			      result, partial, this_xfer);
			எனில் (us->pusb_dev->status != 0) {
				/* if we stall, we need to clear it before
				 * we go on
				 */
#ifdef DEBUG
				display_int_status(us->pusb_dev->status);
#endif
				எனில் (us->pusb_dev->status & USB_ST_STALLED) {
					debug("stalled ->clearing endpoint" \
					      "halt for pipe 0x%x\n", pipe);
					stat = us->pusb_dev->status;
					usb_clear_halt(us->pusb_dev, pipe);
					us->pusb_dev->status = stat;
					எனில் (this_xfer == partial) {
						debug("bulk transferred" \
						      "with error %lX," \
						      " but data ok\n",
						      us->pusb_dev->status);
						திரும்பு 0;
					}
					இல்லையெனில்
						திரும்பு result;
				}
				எனில் (us->pusb_dev->status & USB_ST_NAK_REC) {
					debug("Device NAKed bulk_msg\n");
					திரும்பு result;
				}
				debug("bulk transferred with error");
				எனில் (this_xfer == partial) {
					debug(" %ld, but data ok\n",
					      us->pusb_dev->status);
					திரும்பு 0;
				}
				/* if our try counter reaches 0, bail out */
				debug(" %ld, data %d\n",
				      us->pusb_dev->status, partial);
				எனில் (!maxtry--)
						திரும்பு result;
			}
			/* update to show what data was transferred */
			this_xfer -= partial;
			buf += partial;
			/* continue until this transfer is done */
		} போது (this_xfer);
	}

	/* if we get here, we're done and successful */
	திரும்பு 0;
}

நிலையான முழு usb_stor_BBB_reset(கட்டமைப்பு us_data *us)
{
	முழு result;
	unsigned முழு pipe;

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
	result = usb_control_செய்தி(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_BBB_RESET,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, வெற்று, 0, USB_CNTL_TIMEOUT * 5);

	எனில் ((result < 0) && (us->pusb_dev->status & USB_ST_STALLED)) {
		debug("RESET:stall\n");
		திரும்பு -1;
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
	திரும்பு 0;
}

/* FIXME: this reset function doesn't really reset the port, and it
 * should. Actually it should probably do what it's doing here, and
 * reset the port physically
 */
நிலையான முழு usb_stor_CB_reset(கட்டமைப்பு us_data *us)
{
	unsigned எழுத்து cmd[12];
	முழு result;

	debug("CB_reset\n");
	memset(cmd, 0xff, அளவு(cmd));
	cmd[0] = SCSI_SEND_DIAG;
	cmd[1] = 4;
	result = usb_control_செய்தி(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_CBI_ADSC,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, cmd, அளவு(cmd),
				 USB_CNTL_TIMEOUT * 5);

	/* long wait for reset */
	mdelay(1500);
	debug("CB_reset result %d: status %lX clearing endpoint halt\n",
	      result, us->pusb_dev->status);
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_in));
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_out));

	debug("CB_reset done\n");
	திரும்பு 0;
}

/*
 * Set up the command for a BBB device. Note that the actual SCSI
 * command is copied into cbw.CBWCDB.
 */
நிலையான முழு usb_stor_BBB_comdat(கட்டமைப்பு scsi_cmd *srb, கட்டமைப்பு us_data *us)
{
	முழு result;
	முழு actlen;
	முழு dir_in;
	unsigned முழு pipe;
	ALLOC_CACHE_ALIGN_BUFFER(கட்டமைப்பு umass_bbb_cbw, cbw, 1);

	dir_in = US_DIRECTION(srb->cmd[0]);

#ifdef BBB_COMDAT_TRACE
	printf("dir %d lun %d cmdlen %d cmd %p datalen %lu pdata %p\n",
		dir_in, srb->lun, srb->cmdlen, srb->cmd, srb->datalen,
		srb->pdata);
	எனில் (srb->cmdlen) {
		க்கு (result = 0; result < srb->cmdlen; result++)
			printf("cmd[%d] %#x ", result, srb->cmd[result]);
		printf("\n");
	}
#endif
	/* sanity checks */
	எனில் (!(srb->cmdlen <= CBWCDBLENGTH)) {
		debug("usb_stor_BBB_comdat:cmdlen too large\n");
		திரும்பு -1;
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
	result = usb_bulk_செய்தி(us->pusb_dev, pipe, cbw, UMASS_BBB_CBW_SIZE,
			      &actlen, USB_CNTL_TIMEOUT * 5);
	எனில் (result < 0)
		debug("usb_stor_BBB_comdat:usb_bulk_msg error\n");
	திரும்பு result;
}

/* FIXME: we also need a CBI_command which sets up the completion
 * interrupt, and waits for it
 */
நிலையான முழு usb_stor_CB_comdat(கட்டமைப்பு scsi_cmd *srb, கட்டமைப்பு us_data *us)
{
	முழு result = 0;
	முழு dir_in, retry;
	unsigned முழு pipe;
	unsigned long status;

	retry = 5;
	dir_in = US_DIRECTION(srb->cmd[0]);

	எனில் (dir_in)
		pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);
	இல்லையெனில்
		pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_out);

	போது (retry--) {
		debug("CBI gets a command: Try %d\n", 5 - retry);
#ifdef DEBUG
		usb_show_srb(srb);
#endif
		/* let's send the command via the control pipe */
		result = usb_control_செய்தி(us->pusb_dev,
					 usb_sndctrlpipe(us->pusb_dev , 0),
					 US_CBI_ADSC,
					 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
					 0, us->ifnum,
					 srb->cmd, srb->cmdlen,
					 USB_CNTL_TIMEOUT * 5);
		debug("CB_transport: control msg returned %d, status %lX\n",
		      result, us->pusb_dev->status);
		/* check the return code for the command */
		எனில் (result < 0) {
			எனில் (us->pusb_dev->status & USB_ST_STALLED) {
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
			திரும்பு result;
		}
		/* transfer the data payload for this command, if one exists*/

		debug("CB_transport: control msg returned %d," \
		      " direction is %s to go 0x%lx\n", result,
		      dir_in ? "IN" : "OUT", srb->datalen);
		எனில் (srb->datalen) {
			result = us_one_transfer(us, pipe, (எழுத்து *)srb->pdata,
						 srb->datalen);
			debug("CBI attempted to transfer data," \
			      " result is %d status %lX, len %d\n",
			      result, us->pusb_dev->status,
				us->pusb_dev->act_len);
			எனில் (!(us->pusb_dev->status & USB_ST_NAK_REC))
				நிறுத்து;
		} /* if (srb->datalen) */
		இல்லையெனில்
			நிறுத்து;
	}
	/* return result */

	திரும்பு result;
}

நிலையான முழு usb_stor_CBI_get_status(கட்டமைப்பு scsi_cmd *srb, கட்டமைப்பு us_data *us)
{
	முழு timeout;

	us->ip_wanted = 1;
	usb_int_செய்தி(us->pusb_dev, us->irqpipe,
		    (வெறுமை *)&us->ip_data, us->irqmaxp, us->irqinterval, false);
	timeout = 1000;
	போது (timeout--) {
		எனில் (us->ip_wanted == 0)
			நிறுத்து;
		mdelay(10);
	}
	எனில் (us->ip_wanted) {
		printf("	Did not get interrupt on CBI\n");
		us->ip_wanted = 0;
		திரும்பு USB_STOR_TRANSPORT_ERROR;
	}
	debug("Got interrupt data 0x%x, transferred %d status 0x%lX\n",
	      us->ip_data, us->pusb_dev->irq_act_len,
	      us->pusb_dev->irq_status);
	/* UFI gives us ASC and ASCQ, like a request sense */
	எனில் (us->subclass == US_SC_UFI) {
		எனில் (srb->cmd[0] == SCSI_REQ_SENSE ||
		    srb->cmd[0] == SCSI_INQUIRY)
			திரும்பு USB_STOR_TRANSPORT_GOOD; /* Good */
		இல்லையெனில் எனில் (us->ip_data)
			திரும்பு USB_STOR_TRANSPORT_FAILED;
		இல்லையெனில்
			திரும்பு USB_STOR_TRANSPORT_GOOD;
	}
	/* otherwise, we interpret the data normally */
	தேர்வு (us->ip_data) {
	நிலை 0x0001:
		திரும்பு USB_STOR_TRANSPORT_GOOD;
	நிலை 0x0002:
		திரும்பு USB_STOR_TRANSPORT_FAILED;
	இயல்பு:
		திரும்பு USB_STOR_TRANSPORT_ERROR;
	}			/* switch */
	திரும்பு USB_STOR_TRANSPORT_ERROR;
}

#define USB_TRANSPORT_UNKNOWN_RETRY 5
#define USB_TRANSPORT_NOT_READY_RETRY 10

/* clear a stall on an endpoint - special for BBB devices */
நிலையான முழு usb_stor_BBB_clear_endpt_stall(கட்டமைப்பு us_data *us, __u8 endpt)
{
	/* ENDPOINT_HALT = 0, so set value to 0 */
	திரும்பு usb_control_செய்தி(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
			       USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
			       endpt, வெற்று, 0, USB_CNTL_TIMEOUT * 5);
}

நிலையான முழு usb_stor_BBB_transport(கட்டமைப்பு scsi_cmd *srb, கட்டமைப்பு us_data *us)
{
	முழு result, retry;
	முழு dir_in;
	முழு actlen, data_actlen;
	unsigned முழு pipe, pipein, pipeout;
	ALLOC_CACHE_ALIGN_BUFFER(கட்டமைப்பு umass_bbb_csw, csw, 1);
#ifdef BBB_XPORT_TRACE
	unsigned எழுத்து *ptr;
	முழு index;
#endif

	dir_in = US_DIRECTION(srb->cmd[0]);

	/* COMMAND phase */
	debug("COMMAND phase\n");
	result = usb_stor_BBB_comdat(srb, us);
	எனில் (result < 0) {
		debug("failed to send CBW status %ld\n",
		      us->pusb_dev->status);
		usb_stor_BBB_reset(us);
		திரும்பு USB_STOR_TRANSPORT_FAILED;
	}
	எனில் (!(us->flags & USB_READY))
		mdelay(5);
	pipein = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);
	pipeout = usb_sndbulkpipe(us->pusb_dev, us->ep_out);
	/* DATA phase + error handling */
	data_actlen = 0;
	/* no data, go immediately to the STATUS phase */
	எனில் (srb->datalen == 0)
		செல் st;
	debug("DATA phase\n");
	எனில் (dir_in)
		pipe = pipein;
	இல்லையெனில்
		pipe = pipeout;

	result = usb_bulk_செய்தி(us->pusb_dev, pipe, srb->pdata, srb->datalen,
			      &data_actlen, USB_CNTL_TIMEOUT * 5);
	/* special handling of STALL in DATA phase */
	எனில் ((result < 0) && (us->pusb_dev->status & USB_ST_STALLED)) {
		debug("DATA:stall\n");
		/* clear the STALL on the endpoint */
		result = usb_stor_BBB_clear_endpt_stall(us,
					dir_in ? us->ep_in : us->ep_out);
		எனில் (result >= 0)
			/* continue on to STATUS phase */
			செல் st;
	}
	எனில் (result < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->status);
		usb_stor_BBB_reset(us);
		திரும்பு USB_STOR_TRANSPORT_FAILED;
	}
#ifdef BBB_XPORT_TRACE
	க்கு (index = 0; index < data_actlen; index++)
		printf("pdata[%d] %#x ", index, srb->pdata[index]);
	printf("\n");
#endif
	/* STATUS phase + error handling */
st:
	retry = 0;
again:
	debug("STATUS phase\n");
	result = usb_bulk_செய்தி(us->pusb_dev, pipein, csw, UMASS_BBB_CSW_SIZE,
				&actlen, USB_CNTL_TIMEOUT*5);

	/* special handling of STALL in STATUS phase */
	எனில் ((result < 0) && (retry < 1) &&
	    (us->pusb_dev->status & USB_ST_STALLED)) {
		debug("STATUS:stall\n");
		/* clear the STALL on the endpoint */
		result = usb_stor_BBB_clear_endpt_stall(us, us->ep_in);
		எனில் (result >= 0 && (retry++ < 1))
			/* do a retry */
			செல் again;
	}
	எனில் (result < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->status);
		usb_stor_BBB_reset(us);
		திரும்பு USB_STOR_TRANSPORT_FAILED;
	}
#ifdef BBB_XPORT_TRACE
	ptr = (unsigned எழுத்து *)csw;
	க்கு (index = 0; index < UMASS_BBB_CSW_SIZE; index++)
		printf("ptr[%d] %#x ", index, ptr[index]);
	printf("\n");
#endif
	/* misuse pipe to get the residue */
	pipe = le32_to_cpu(csw->dCSWDataResidue);
	எனில் (pipe == 0 && srb->datalen != 0 && srb->datalen - data_actlen != 0)
		pipe = srb->datalen - data_actlen;
	எனில் (CSWSIGNATURE != le32_to_cpu(csw->dCSWSignature)) {
		debug("!CSWSIGNATURE\n");
		usb_stor_BBB_reset(us);
		திரும்பு USB_STOR_TRANSPORT_FAILED;
	} இல்லையெனில் எனில் ((CBWTag - 1) != le32_to_cpu(csw->dCSWTag)) {
		debug("!Tag\n");
		usb_stor_BBB_reset(us);
		திரும்பு USB_STOR_TRANSPORT_FAILED;
	} இல்லையெனில் எனில் (csw->bCSWStatus > CSWSTATUS_PHASE) {
		debug(">PHASE\n");
		usb_stor_BBB_reset(us);
		திரும்பு USB_STOR_TRANSPORT_FAILED;
	} இல்லையெனில் எனில் (csw->bCSWStatus == CSWSTATUS_PHASE) {
		debug("=PHASE\n");
		usb_stor_BBB_reset(us);
		திரும்பு USB_STOR_TRANSPORT_FAILED;
	} இல்லையெனில் எனில் (data_actlen > srb->datalen) {
		debug("transferred %dB instead of %ldB\n",
		      data_actlen, srb->datalen);
		திரும்பு USB_STOR_TRANSPORT_FAILED;
	} இல்லையெனில் எனில் (csw->bCSWStatus == CSWSTATUS_FAILED) {
		debug("FAILED\n");
		திரும்பு USB_STOR_TRANSPORT_FAILED;
	}

	திரும்பு result;
}

நிலையான முழு usb_stor_CB_transport(கட்டமைப்பு scsi_cmd *srb, கட்டமைப்பு us_data *us)
{
	முழு result, status;
	கட்டமைப்பு scsi_cmd *psrb;
	கட்டமைப்பு scsi_cmd reqsrb;
	முழு retry, notready;

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
	எனில் (us->protocol == US_PR_CBI) {
		status = usb_stor_CBI_get_status(srb, us);
		/* if the status is error, report it */
		எனில் (status == USB_STOR_TRANSPORT_ERROR) {
			debug(" USB CBI Command Error\n");
			திரும்பு status;
		}
		srb->sense_buf[12] = (unsigned எழுத்து)(us->ip_data >> 8);
		srb->sense_buf[13] = (unsigned எழுத்து)(us->ip_data & 0xff);
		எனில் (!us->ip_data) {
			/* if the status is good, report it */
			எனில் (status == USB_STOR_TRANSPORT_GOOD) {
				debug(" USB CBI Command Good\n");
				திரும்பு status;
			}
		}
	}
	/* do we have to issue an auto request? */
	/* HERE we have to check the result */
	எனில் ((result < 0) && !(us->pusb_dev->status & USB_ST_STALLED)) {
		debug("ERROR %lX\n", us->pusb_dev->status);
		us->transport_reset(us);
		திரும்பு USB_STOR_TRANSPORT_ERROR;
	}
	எனில் ((us->protocol == US_PR_CBI) &&
	    ((srb->cmd[0] == SCSI_REQ_SENSE) ||
	    (srb->cmd[0] == SCSI_INQUIRY))) {
		/* do not issue an autorequest after request sense */
		debug("No auto request and good\n");
		திரும்பு USB_STOR_TRANSPORT_GOOD;
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
	எனில் (us->protocol == US_PR_CBI)
		status = usb_stor_CBI_get_status(psrb, us);

	எனில் ((result < 0) && !(us->pusb_dev->status & USB_ST_STALLED)) {
		debug(" AUTO REQUEST ERROR %ld\n",
		      us->pusb_dev->status);
		திரும்பு USB_STOR_TRANSPORT_ERROR;
	}
	debug("autorequest returned 0x%02X 0x%02X 0x%02X 0x%02X\n",
	      srb->sense_buf[0], srb->sense_buf[2],
	      srb->sense_buf[12], srb->sense_buf[13]);
	/* Check the auto request result */
	எனில் ((srb->sense_buf[2] == 0) &&
	    (srb->sense_buf[12] == 0) &&
	    (srb->sense_buf[13] == 0)) {
		/* ok, no sense */
		திரும்பு USB_STOR_TRANSPORT_GOOD;
	}

	/* Check the auto request result */
	தேர்வு (srb->sense_buf[2]) {
	நிலை 0x01:
		/* Recovered Error */
		திரும்பு USB_STOR_TRANSPORT_GOOD;
		நிறுத்து;
	நிலை 0x02:
		/* Not Ready */
		எனில் (notready++ > USB_TRANSPORT_NOT_READY_RETRY) {
			printf("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X (NOT READY)\n", srb->cmd[0],
				srb->sense_buf[0], srb->sense_buf[2],
				srb->sense_buf[12], srb->sense_buf[13]);
			திரும்பு USB_STOR_TRANSPORT_FAILED;
		} இல்லையெனில் {
			mdelay(100);
			செல் do_retry;
		}
		நிறுத்து;
	இயல்பு:
		எனில் (retry++ > USB_TRANSPORT_UNKNOWN_RETRY) {
			printf("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X\n", srb->cmd[0], srb->sense_buf[0],
				srb->sense_buf[2], srb->sense_buf[12],
				srb->sense_buf[13]);
			திரும்பு USB_STOR_TRANSPORT_FAILED;
		} இல்லையெனில்
			செல் do_retry;
		நிறுத்து;
	}
	திரும்பு USB_STOR_TRANSPORT_FAILED;
}

நிலையான வெறுமை usb_stor_set_max_xfer_blk(கட்டமைப்பு usb_device *udev,
				      கட்டமைப்பு us_data *us)
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

#எனில் CONFIG_IS_ENABLED(DM_USB)
	size_t size;
	முழு ret;

	ret = usb_get_max_xfer_size(udev, (size_t *)&size);
	எனில் ((ret >= 0) && (size < blk * 512))
		blk = size / 512;
#endif

	us->max_xfer_blk = blk;
}

நிலையான முழு usb_inquiry(கட்டமைப்பு scsi_cmd *srb, கட்டமைப்பு us_data *ss)
{
	முழு retry, i;
	retry = 5;
	செய் {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_INQUIRY;
		srb->cmd[1] = srb->lun << 5;
		srb->cmd[4] = 36;
		srb->datalen = 36;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		i = ss->transport(srb, ss);
		debug("inquiry returns %d\n", i);
		எனில் (i == 0)
			நிறுத்து;
	} போது (--retry);

	எனில் (!retry) {
		printf("error in inquiry\n");
		திரும்பு -1;
	}
	திரும்பு 0;
}

நிலையான முழு usb_request_sense(கட்டமைப்பு scsi_cmd *srb, கட்டமைப்பு us_data *ss)
{
	எழுத்து *ptr;

	ptr = (எழுத்து *)srb->pdata;
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
	திரும்பு 0;
}

நிலையான முழு usb_test_unit_ready(கட்டமைப்பு scsi_cmd *srb, கட்டமைப்பு us_data *ss)
{
	முழு retries = 10;

	செய் {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_TST_U_RDY;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 0;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		எனில் (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD) {
			ss->flags |= USB_READY;
			திரும்பு 0;
		}
		usb_request_sense(srb, ss);
		/*
		 * Check the Key Code Qualifier, if it matches
		 * "Not Ready - medium not present"
		 * (the sense Key equals 0x2 and the ASC is 0x3a)
		 * return immediately as the medium being absent won't change
		 * unless there is a user action.
		 */
		எனில் ((srb->sense_buf[2] == 0x02) &&
		    (srb->sense_buf[12] == 0x3a))
			திரும்பு -1;
		mdelay(100);
	} போது (retries--);

	திரும்பு -1;
}

நிலையான முழு usb_read_capacity(கட்டமைப்பு scsi_cmd *srb, கட்டமைப்பு us_data *ss)
{
	முழு retry;
	/* XXX retries */
	retry = 3;
	செய் {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_RD_CAPAC;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 8;
		srb->cmdlen = ss->cmd12 ? 12 : 10;
		எனில் (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD)
			திரும்பு 0;
	} போது (retry--);

	திரும்பு -1;
}

நிலையான முழு usb_read_10(கட்டமைப்பு scsi_cmd *srb, கட்டமைப்பு us_data *ss,
		       unsigned long start, unsigned short blocks)
{
	memset(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_READ10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((unsigned எழுத்து) (start >> 24)) & 0xff;
	srb->cmd[3] = ((unsigned எழுத்து) (start >> 16)) & 0xff;
	srb->cmd[4] = ((unsigned எழுத்து) (start >> 8)) & 0xff;
	srb->cmd[5] = ((unsigned எழுத்து) (start)) & 0xff;
	srb->cmd[7] = ((unsigned எழுத்து) (blocks >> 8)) & 0xff;
	srb->cmd[8] = (unsigned எழுத்து) blocks & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("read10: start %lx blocks %x\n", start, blocks);
	திரும்பு ss->transport(srb, ss);
}

நிலையான முழு usb_write_10(கட்டமைப்பு scsi_cmd *srb, கட்டமைப்பு us_data *ss,
			unsigned long start, unsigned short blocks)
{
	memset(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_WRITE10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((unsigned எழுத்து) (start >> 24)) & 0xff;
	srb->cmd[3] = ((unsigned எழுத்து) (start >> 16)) & 0xff;
	srb->cmd[4] = ((unsigned எழுத்து) (start >> 8)) & 0xff;
	srb->cmd[5] = ((unsigned எழுத்து) (start)) & 0xff;
	srb->cmd[7] = ((unsigned எழுத்து) (blocks >> 8)) & 0xff;
	srb->cmd[8] = (unsigned எழுத்து) blocks & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("write10: start %lx blocks %x\n", start, blocks);
	திரும்பு ss->transport(srb, ss);
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
நிலையான வெறுமை usb_bin_fixup(கட்டமைப்பு usb_device_descriptor descriptor,
				unsigned எழுத்து vendor[],
				unsigned எழுத்து product[]) {
	மாறிலி unsigned எழுத்து max_vendor_len = 40;
	மாறிலி unsigned எழுத்து max_product_len = 20;
	எனில் (descriptor.idVendor == 0x0424 && descriptor.idProduct == 0x223a) {
		strncpy((எழுத்து *)vendor, "SMSC", max_vendor_len);
		strncpy((எழுத்து *)product, "Flash Media Cntrller",
			max_product_len);
	}
}
#endif /* CONFIG_USB_BIN_FIXUP */

#எனில் CONFIG_IS_ENABLED(BLK)
நிலையான unsigned long usb_stor_read(கட்டமைப்பு udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, வெறுமை *buffer)
#இல்லையெனில்
நிலையான unsigned long usb_stor_read(கட்டமைப்பு blk_desc *block_dev, lbaint_t blknr,
				   lbaint_t blkcnt, வெறுமை *buffer)
#endif
{
	lbaint_t start, blks;
	uintptr_t buf_addr;
	unsigned short smallblks;
	கட்டமைப்பு usb_device *udev;
	கட்டமைப்பு us_data *ss;
	முழு retry;
	கட்டமைப்பு scsi_cmd *srb = &usb_ccb;
#எனில் CONFIG_IS_ENABLED(BLK)
	கட்டமைப்பு blk_desc *block_dev;
#endif

	எனில் (blkcnt == 0)
		திரும்பு 0;
	/* Setup  device */
#எனில் CONFIG_IS_ENABLED(BLK)
	block_dev = dev_get_uclass_plat(dev);
	udev = dev_get_parent_priv(dev_get_parent(dev));
	debug("\nusb_read: udev %d\n", block_dev->devnum);
#இல்லையெனில்
	debug("\nusb_read: udev %d\n", block_dev->devnum);
	udev = usb_dev_desc[block_dev->devnum].priv;
	எனில் (!udev) {
		debug("%s: No device\n", __func__);
		திரும்பு 0;
	}
#endif
	ss = (கட்டமைப்பு us_data *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_lock_async(udev, 1);
	srb->lun = block_dev->lun;
	buf_addr = (uintptr_t)buffer;
	start = blknr;
	blks = blkcnt;

	debug("\nusb_read: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      block_dev->devnum, start, blks, buf_addr);

	செய் {
		/* XXX need some comment here */
		retry = 2;
		srb->pdata = (unsigned எழுத்து *)buf_addr;
		எனில் (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		இல்லையெனில்
			smallblks = (unsigned short) blks;
retry_it:
		எனில் (smallblks == ss->max_xfer_blk)
			usb_show_progress();
		srb->datalen = block_dev->blksz * smallblks;
		srb->pdata = (unsigned எழுத்து *)buf_addr;
		எனில் (usb_read_10(srb, ss, start, smallblks)) {
			debug("Read ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			எனில் (retry--)
				செல் retry_it;
			blkcnt -= blks;
			நிறுத்து;
		}
		start += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} போது (blks != 0);

	debug("usb_read: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      start, smallblks, buf_addr);

	usb_lock_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	எனில் (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	திரும்பு blkcnt;
}

#எனில் CONFIG_IS_ENABLED(BLK)
நிலையான unsigned long usb_stor_write(கட்டமைப்பு udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, மாறிலி வெறுமை *buffer)
#இல்லையெனில்
நிலையான unsigned long usb_stor_write(கட்டமைப்பு blk_desc *block_dev, lbaint_t blknr,
				    lbaint_t blkcnt, மாறிலி வெறுமை *buffer)
#endif
{
	lbaint_t start, blks;
	uintptr_t buf_addr;
	unsigned short smallblks;
	கட்டமைப்பு usb_device *udev;
	கட்டமைப்பு us_data *ss;
	முழு retry;
	கட்டமைப்பு scsi_cmd *srb = &usb_ccb;
#எனில் CONFIG_IS_ENABLED(BLK)
	கட்டமைப்பு blk_desc *block_dev;
#endif

	எனில் (blkcnt == 0)
		திரும்பு 0;

	/* Setup  device */
#எனில் CONFIG_IS_ENABLED(BLK)
	block_dev = dev_get_uclass_plat(dev);
	udev = dev_get_parent_priv(dev_get_parent(dev));
	debug("\nusb_read: udev %d\n", block_dev->devnum);
#இல்லையெனில்
	debug("\nusb_read: udev %d\n", block_dev->devnum);
	udev = usb_dev_desc[block_dev->devnum].priv;
	எனில் (!udev) {
		debug("%s: No device\n", __func__);
		திரும்பு 0;
	}
#endif
	ss = (கட்டமைப்பு us_data *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_lock_async(udev, 1);

	srb->lun = block_dev->lun;
	buf_addr = (uintptr_t)buffer;
	start = blknr;
	blks = blkcnt;

	debug("\nusb_write: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      block_dev->devnum, start, blks, buf_addr);

	செய் {
		/* If write fails retry for max retry count else
		 * return with number of blocks written successfully.
		 */
		retry = 2;
		srb->pdata = (unsigned எழுத்து *)buf_addr;
		எனில் (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		இல்லையெனில்
			smallblks = (unsigned short) blks;
retry_it:
		எனில் (smallblks == ss->max_xfer_blk)
			usb_show_progress();
		srb->datalen = block_dev->blksz * smallblks;
		srb->pdata = (unsigned எழுத்து *)buf_addr;
		எனில் (usb_write_10(srb, ss, start, smallblks)) {
			debug("Write ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			எனில் (retry--)
				செல் retry_it;
			blkcnt -= blks;
			நிறுத்து;
		}
		start += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} போது (blks != 0);

	debug("usb_write: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      start, smallblks, buf_addr);

	usb_lock_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	எனில் (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	திரும்பு blkcnt;

}

/* Probe to see if a new device is actually a Storage device */
முழு usb_storage_probe(கட்டமைப்பு usb_device *dev, unsigned முழு ifnum,
		      கட்டமைப்பு us_data *ss)
{
	கட்டமைப்பு usb_interface *iface;
	முழு i;
	கட்டமைப்பு usb_endpoint_descriptor *ep_desc;
	unsigned முழு flags = 0;

	/* let's examine the device now */
	iface = &dev->config.if_desc[ifnum];

	எனில் (dev->descriptor.bDeviceClass != 0 ||
			iface->desc.bInterfaceClass != USB_CLASS_MASS_STORAGE ||
			iface->desc.bInterfaceSubClass < US_SC_MIN ||
			iface->desc.bInterfaceSubClass > US_SC_MAX) {
		debug("Not mass storage\n");
		/* if it's not a mass storage, we go no further */
		திரும்பு 0;
	}

	memset(ss, 0, அளவு(கட்டமைப்பு us_data));

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
	தேர்வு (ss->protocol) {
	நிலை US_PR_CB:
		debug("Control/Bulk\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		நிறுத்து;

	நிலை US_PR_CBI:
		debug("Control/Bulk/Interrupt\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		நிறுத்து;
	நிலை US_PR_BULK:
		debug("Bulk/Bulk/Bulk\n");
		ss->transport = usb_stor_BBB_transport;
		ss->transport_reset = usb_stor_BBB_reset;
		நிறுத்து;
	இயல்பு:
		printf("USB Storage Transport unknown / not yet implemented\n");
		திரும்பு 0;
		நிறுத்து;
	}

	/*
	 * We are expecting a minimum of 2 endpoints - in and out (bulk).
	 * An optional interrupt is OK (necessary for CBI protocol).
	 * We will ignore any others.
	 */
	க்கு (i = 0; i < iface->desc.b_இலக்கம்_Endpoints; i++) {
		ep_desc = &iface->ep_desc[i];
		/* is it an BULK endpoint? */
		எனில் ((ep_desc->bmAttributes &
		     USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK) {
			எனில் (ep_desc->bEndpointAddress & USB_DIR_IN)
				ss->ep_in = ep_desc->bEndpointAddress &
						USB_ENDPOINT_எண்_MASK;
			இல்லையெனில்
				ss->ep_out =
					ep_desc->bEndpointAddress &
					USB_ENDPOINT_எண்_MASK;
		}

		/* is it an interrupt endpoint? */
		எனில் ((ep_desc->bmAttributes &
		     USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT) {
			ss->ep_int = ep_desc->bEndpointAddress &
						USB_ENDPOINT_எண்_MASK;
			ss->irqinterval = ep_desc->bInterval;
		}
	}
	debug("Endpoints In %d Out %d Int %d\n",
	      ss->ep_in, ss->ep_out, ss->ep_int);

	/* Do some basic sanity checks, and bail if we find a problem */
	எனில் (usb_set_interface(dev, iface->desc.b_Interface_எண், 0) ||
	    !ss->ep_in || !ss->ep_out ||
	    (ss->protocol == US_PR_CBI && ss->ep_int == 0)) {
		debug("Problems with device\n");
		திரும்பு 0;
	}
	/* set class specific stuff */
	/* We only handle certain protocols.  Currently, these are
	 * the only ones.
	 * The SFF8070 accepts the requests used in u-boot
	 */
	எனில் (ss->subclass != US_SC_UFI && ss->subclass != US_SC_SCSI &&
	    ss->subclass != US_SC_8070) {
		printf("Sorry, protocol %d not yet supported.\n", ss->subclass);
		திரும்பு 0;
	}

	/* UFI uses 12-byte commands (like RBC, unlike SCSI) */
	எனில் (ss->subclass == US_SC_UFI)
		ss->cmd12 = true;

	எனில் (ss->ep_int) {
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

	dev->privptr = (வெறுமை *)ss;
	திரும்பு 1;
}

முழு usb_stor_get_info(கட்டமைப்பு usb_device *dev, கட்டமைப்பு us_data *ss,
		      கட்டமைப்பு blk_desc *dev_desc)
{
	unsigned எழுத்து perq, modi;
	ALLOC_CACHE_ALIGN_BUFFER(u32, cap, 2);
	ALLOC_CACHE_ALIGN_BUFFER(u8, usb_stor_buf, 36);
	u32 capacity, blksz;
	கட்டமைப்பு scsi_cmd *pccb = &usb_ccb;

	pccb->pdata = usb_stor_buf;

	dev_desc->target = dev->devnum;
	pccb->lun = dev_desc->lun;
	debug(" address %d\n", dev_desc->target);

	எனில் (usb_inquiry(pccb, ss)) {
		debug("%s: usb_inquiry() failed\n", __func__);
		திரும்பு -1;
	}

	perq = usb_stor_buf[0];
	modi = usb_stor_buf[1];

	/*
	 * Skip unknown devices (0x1f) and enclosure service devices (0x0d),
	 * they would not respond to test_unit_ready .
	 */
	எனில் (((perq & 0x1f) == 0x1f) || ((perq & 0x1f) == 0x0d)) {
		debug("%s: unknown/unsupported device\n", __func__);
		திரும்பு 0;
	}
	எனில் ((modi&0x80) == 0x80) {
		/* drive is removable */
		dev_desc->removable = 1;
	}
	memcpy(dev_desc->vendor, (மாறிலி வெறுமை *)&usb_stor_buf[8], 8);
	memcpy(dev_desc->product, (மாறிலி வெறுமை *)&usb_stor_buf[16], 16);
	memcpy(dev_desc->revision, (மாறிலி வெறுமை *)&usb_stor_buf[32], 4);
	dev_desc->vendor[8] = 0;
	dev_desc->product[16] = 0;
	dev_desc->revision[4] = 0;
#ifdef CONFIG_USB_BIN_FIXUP
	usb_bin_fixup(dev->descriptor, (uchar *)dev_desc->vendor,
		      (uchar *)dev_desc->product);
#endif /* CONFIG_USB_BIN_FIXUP */
	debug("ISO Vers %X, Response Data %X\n", usb_stor_buf[2],
	      usb_stor_buf[3]);
	எனில் (usb_test_unit_ready(pccb, ss)) {
		printf("Device NOT ready\n"
		       "   Request Sense returned %02X %02X %02X\n",
		       pccb->sense_buf[2], pccb->sense_buf[12],
		       pccb->sense_buf[13]);
		எனில் (dev_desc->removable == 1)
			dev_desc->type = perq;
		திரும்பு 0;
	}
	pccb->pdata = (unsigned எழுத்து *)cap;
	memset(pccb->pdata, 0, 8);
	எனில் (usb_read_capacity(pccb, ss) != 0) {
		printf("READ_CAP ERROR\n");
		ss->flags &= ~USB_READY;
		cap[0] = 2880;
		cap[1] = 0x200;
	}
	debug("Read Capacity returns: 0x%08x, 0x%08x\n", cap[0], cap[1]);
#எனில் 0
	எனில் (cap[0] > (0x200000 * 10)) /* greater than 10 GByte */
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

	திரும்பு 1;
}

#எனில் CONFIG_IS_ENABLED(DM_USB)

நிலையான முழு usb_mass_storage_probe(கட்டமைப்பு udevice *dev)
{
	கட்டமைப்பு usb_device *udev = dev_get_parent_priv(dev);
	முழு ret;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	ret = usb_stor_probe_device(udev);
	usb_disable_asynch(0); /* asynch transfer allowed */

	திரும்பு ret;
}

நிலையான மாறிலி கட்டமைப்பு udevice_id usb_mass_storage_ids[] = {
	{ .compatible = "usb-mass-storage" },
	{ }
};

U_BOOT_DRIVER(usb_mass_storage) = {
	.name	= "usb_mass_storage",
	.id	= UCLASS_MASS_STORAGE,
	.of_match = usb_mass_storage_ids,
	.probe = usb_mass_storage_probe,
#எனில் CONFIG_IS_ENABLED(BLK)
	.plat_auto	= அளவு(கட்டமைப்பு us_data),
#endif
};

UCLASS_DRIVER(usb_mass_storage) = {
	.id		= UCLASS_MASS_STORAGE,
	.name		= "usb_mass_storage",
};

நிலையான மாறிலி கட்டமைப்பு usb_device_id mass_storage_id_table[] = {
	{
		.match_flags = USB_DEVICE_ID_MATCH_INT_CLASS,
		.bInterfaceClass = USB_CLASS_MASS_STORAGE
	},
	{ }		/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_mass_storage, mass_storage_id_table);
#endif

#எனில் CONFIG_IS_ENABLED(BLK)
நிலையான மாறிலி கட்டமைப்பு blk_ops usb_storage_ops = {
	.பட	= usb_stor_read,
	.லிக	= usb_stor_write,
};

U_BOOT_DRIVER(usb_storage_blk) = {
	.name		= "usb_storage_blk",
	.id		= UCLASS_BLK,
	.ops		= &usb_storage_ops,
};
#இல்லையெனில்
U_BOOT_LEGACY_BLK(usb) = {
	.uclass_idname	= "usb",
	.uclass_id	= UCLASS_USB,
	.max_devs	= USB_MAX_STOR_DEV,
	.desc		= usb_dev_desc,
};
#endif
