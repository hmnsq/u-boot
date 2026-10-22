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

#અન્તર્ભાવ <blk.h>
#અન્તર્ભાવ <bootdev.h>
#અન્તર્ભાવ <command.h>
#અન્તર્ભાવ <dm.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <mapmem.h>
#અન્તર્ભાવ <memalign.h>
#અન્તર્ભાવ <asm/byteorder.h>
#અન્તર્ભાવ <asm/cache.h>
#અન્તર્ભાવ <asm/processor.h>
#અન્તર્ભાવ <dm/device-internal.h>
#અન્તર્ભાવ <dm/lists.h>
#અન્તર્ભાવ <linux/delay.h>

#અન્તર્ભાવ <part.h>
#અન્તર્ભાવ <usb.h>

#undef BBB_COMDAT_TRACE
#undef BBB_XPORT_TRACE

#અન્તર્ભાવ <scsi.h>
/* direction table -- this indicates the direction of the data
 * transfer for each command code -- a 1 indicates input
 */
સ્થિર અચલ unsigned અક્ષર us_direction[256/8] = {
	0x28, 0x81, 0x14, 0x14, 0x20, 0x01, 0x90, 0x77,
	0x0C, 0x20, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#define US_DIRECTION(x) ((us_direction[x>>3] >> (x & 7)) & 1)

સ્થિર સંરચના scsi_cmd usb_ccb __aligned(ARCH_DMA_MINALIGN);
સ્થિર __u32 CBWTag;

સ્થિર પૂર્ણાંક usb_max_devs; /* number of highest available usb device */

#જો !CONFIG_IS_ENABLED(BLK)
સ્થિર સંરચના blk_desc usb_dev_desc[USB_MAX_STOR_DEV];
#endif

સંરચના us_data;
પ્રકારવ્યાખ્યા પૂર્ણાંક (*trans_cmnd)(સંરચના scsi_cmd *cb, સંરચના us_data *data);
પ્રકારવ્યાખ્યા પૂર્ણાંક (*trans_reset)(સંરચના us_data *data);

સંરચના us_data {
	સંરચના usb_device *pusb_dev;	 /* this usb_device */

	unsigned પૂર્ણાંક	flags;			/* from filter initially */
#	define USB_READY	(1 << 0)
	unsigned અક્ષર	ifnum;			/* interface number */
	unsigned અક્ષર	ep_in;			/* in endpoint */
	unsigned અક્ષર	ep_out;			/* out ....... */
	unsigned અક્ષર	ep_int;			/* interrupt . */
	unsigned અક્ષર	subclass;		/* as in overview */
	unsigned અક્ષર	protocol;		/* .............. */
	unsigned અક્ષર	attention_done;		/* force attn on first cmd */
	unsigned short	ip_data;		/* interrupt data */
	પૂર્ણાંક		action;			/* what to do */
	પૂર્ણાંક		ip_wanted;		/* needed */
	પૂર્ણાંક		*irq_handle;		/* for USB int requests */
	unsigned પૂર્ણાંક	irqpipe;		/* pipe for release_irq */
	unsigned અક્ષર	irqmaxp;		/* max packed for irq Pipe */
	unsigned અક્ષર	irqinterval;		/* Intervall for IRQ Pipe */
	સંરચના scsi_cmd	*srb;			/* current srb */
	trans_reset	transport_reset;	/* reset routine */
	trans_cmnd	transport;		/* transport routine */
	unsigned short	max_xfer_blk;		/* maximum transfer blocks */
	bool		cmd12;			/* use 12-byte commands (RBC/UFI) */
};

#જો !CONFIG_IS_ENABLED(BLK)
સ્થિર સંરચના us_data usb_stor[USB_MAX_STOR_DEV];
#endif

#define USB_STOR_TRANSPORT_GOOD	   0
#define USB_STOR_TRANSPORT_FAILED -1
#define USB_STOR_TRANSPORT_ERROR  -2

પૂર્ણાંક usb_stor_get_info(સંરચના usb_device *dev, સંરચના us_data *us,
		      સંરચના blk_desc *dev_desc);
પૂર્ણાંક usb_storage_probe(સંરચના usb_device *dev, unsigned પૂર્ણાંક ifnum,
		      સંરચના us_data *ss);
#જો CONFIG_IS_ENABLED(BLK)
સ્થિર unsigned long usb_stor_read(સંરચના udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, શૂન્ય *buffer);
સ્થિર unsigned long usb_stor_write(સંરચના udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, અચલ શૂન્ય *buffer);
#નહીંતર
સ્થિર unsigned long usb_stor_read(સંરચના blk_desc *block_dev, lbaint_t blknr,
				   lbaint_t blkcnt, શૂન્ય *buffer);
સ્થિર unsigned long usb_stor_write(સંરચના blk_desc *block_dev, lbaint_t blknr,
				    lbaint_t blkcnt, અચલ શૂન્ય *buffer);
#endif
શૂન્ય uhci_show_temp_int_td(શૂન્ય);

સ્થિર શૂન્ય usb_show_progress(શૂન્ય)
{
	debug(".");
}

/*******************************************************************************
 * show info on storage devices; 'usb start/init' must be invoked earlier
 * as we only retrieve structures populated during devices initialization
 */
પૂર્ણાંક usb_stor_info(શૂન્ય)
{
	પૂર્ણાંક ગણતરી = 0;
#જો CONFIG_IS_ENABLED(BLK)
	સંરચના udevice *dev;

	માટે (blk_first_device(UCLASS_USB, &dev);
	     dev;
	     blk_next_device(&dev)) {
		સંરચના blk_desc *desc = dev_get_uclass_plat(dev);

		printf("  Device %d: ", desc->devnum);
		dev_print(desc);
		ગણતરી++;
	}
#નહીંતર
	પૂર્ણાંક i;

	જો (usb_max_devs > 0) {
		માટે (i = 0; i < usb_max_devs; i++) {
			printf("  Device %d: ", i);
			dev_print(&usb_dev_desc[i]);
		}
		પાછા 0;
	}
#endif
	જો (!ગણતરી) {
		printf("No storage devices, perhaps not 'usb start'ed..?\n");
		પાછા 1;
	}

	પાછા 0;
}

સ્થિર unsigned પૂર્ણાંક usb_get_max_lun(સંરચના us_data *us)
{
	પૂર્ણાંક len;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned અક્ષર, result, 1);
	len = usb_control_સંદેશ(us->pusb_dev,
			      usb_rcvctrlpipe(us->pusb_dev, 0),
			      US_BBB_GET_MAX_LUN,
			      USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_IN,
			      0, us->ifnum,
			      result, કદ(અક્ષર),
			      USB_CNTL_TIMEOUT * 5);
	debug("Get Max LUN -> len = %i, result = %i\n", len, (પૂર્ણાંક) *result);
	પાછા (len > 0) ? *result : 0;
}

સ્થિર પૂર્ણાંક usb_stor_probe_device(સંરચના usb_device *udev)
{
	પૂર્ણાંક lun, max_lun;

#જો CONFIG_IS_ENABLED(BLK)
	સંરચના us_data *data;
	પૂર્ણાંક ret;
#નહીંતર
	પૂર્ણાંક start;

	જો (udev == ખાલી)
		પાછા -ENOENT; /* no more devices available */
#endif

	debug("\n\nProbing for storage\n");
#જો CONFIG_IS_ENABLED(BLK)
	/*
	 * We store the us_data in the mass storage device's plat. It
	 * is shared by all LUNs (block devices) attached to this mass storage
	 * device.
	 */
	data = dev_get_plat(udev->dev);
	જો (!usb_storage_probe(udev, 0, data))
		પાછા 0;
	max_lun = usb_get_max_lun(data);
	માટે (lun = 0; lun <= max_lun; lun++) {
		સંરચના blk_desc *blkdev;
		સંરચના udevice *dev;
		અક્ષર str[10];

		snprintf(str, કદ(str), "lun%d", lun);
		ret = blk_create_devicef(udev->dev, "usb_storage_blk", str,
					 UCLASS_USB, usb_max_devs,
					 DEFAULT_BLKSZ, 0, &dev);
		જો (ret) {
			debug("Cannot bind driver\n");
			પાછા ret;
		}

		blkdev = dev_get_uclass_plat(dev);
		blkdev->target = 0xff;
		blkdev->lun = lun;

		ret = usb_stor_get_info(udev, data, blkdev);
		જો (ret == 1) {
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		} નહીંતર {
			debug("usb_stor_get_info: Invalid device\n");
			ret = device_unbind(dev);
			જો (ret)
				પાછા ret;
			ચાલુ;
		}

		ret = blk_probe_or_unbind(dev);
		જો (ret)
			પાછા ret;

		ret = bootdev_setup_for_sibling_blk(dev, "usb_bootdev");
		જો (ret) {
			પૂર્ણાંક ret2;

			ret2 = device_unbind(dev);
			જો (ret2)
				પાછા log_સંદેશ_ret("bootdev", ret2);
			પાછા log_સંદેશ_ret("bootdev", ret);
		}
	}
#નહીંતર
	/* We don't have space to even probe if we hit the maximum */
	જો (usb_max_devs == USB_MAX_STOR_DEV) {
		printf("max USB Storage Device reached: %d stopping\n",
		       usb_max_devs);
		પાછા -ENOSPC;
	}

	જો (!usb_storage_probe(udev, 0, &usb_stor[usb_max_devs]))
		પાછા 0;

	/*
	 * OK, it's a storage device.  Iterate over its LUNs and populate
	 * usb_dev_desc'
	 */
	start = usb_max_devs;

	max_lun = usb_get_max_lun(&usb_stor[usb_max_devs]);
	માટે (lun = 0; lun <= max_lun && usb_max_devs < USB_MAX_STOR_DEV;
	     lun++) {
		સંરચના blk_desc *blkdev;

		blkdev = &usb_dev_desc[usb_max_devs];
		memset(blkdev, '\0', કદ(સંરચના blk_desc));
		blkdev->uclass_id = UCLASS_USB;
		blkdev->devnum = usb_max_devs;
		blkdev->part_type = PART_TYPE_UNKNOWN;
		blkdev->target = 0xff;
		blkdev->type = DEV_TYPE_UNKNOWN;
		blkdev->block_read = usb_stor_read;
		blkdev->block_write = usb_stor_write;
		blkdev->lun = lun;
		blkdev->priv = udev;

		જો (usb_stor_get_info(udev, &usb_stor[start],
				      &usb_dev_desc[usb_max_devs]) == 1) {
			debug("partype: %d\n", blkdev->part_type);
			part_init(blkdev);
			debug("partype: %d\n", blkdev->part_type);
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		}
	}
#endif

	પાછા 0;
}

શૂન્ય usb_stor_reset(શૂન્ય)
{
	usb_max_devs = 0;
}

/*******************************************************************************
 * scan the usb and reports device info
 * to the user if mode = 1
 * returns current device or -1 if no
 */
પૂર્ણાંક usb_stor_scan(પૂર્ણાંક mode)
{
	જો (mode == 1)
		printf("       scanning usb for storage devices... ");

#જો !CONFIG_IS_ENABLED(DM_USB)
	unsigned અક્ષર i;

	usb_disable_asynch(1); /* asynch transfer not allowed */

	usb_stor_reset();
	માટે (i = 0; i < USB_MAX_DEVICE; i++) {
		સંરચના usb_device *dev;

		dev = usb_get_dev_index(i); /* get device */
		debug("i=%d\n", i);
		જો (usb_stor_probe_device(dev))
			તોડો;
	} /* for */

	usb_disable_asynch(0); /* asynch transfer allowed */
#endif
	printf("%d Storage Device(s) found\n", usb_max_devs);
	જો (usb_max_devs > 0)
		પાછા 0;
	પાછા -1;
}

સ્થિર પૂર્ણાંક usb_stor_irq(સંરચના usb_device *dev)
{
	સંરચના us_data *us;
	us = (સંરચના us_data *)dev->privptr;

	જો (us->ip_wanted)
		us->ip_wanted = 0;
	પાછા 0;
}

#ifdef	DEBUG

સ્થિર શૂન્ય usb_show_srb(સંરચના scsi_cmd *pccb)
{
	પૂર્ણાંક i;
	printf("SRB: len %d datalen 0x%lX\n ", pccb->cmdlen, pccb->datalen);
	માટે (i = 0; i < pccb->cmdlen; i++)
		printf("%02X ", pccb->cmd[i]);
	printf("\n");
}

સ્થિર શૂન્ય display_int_status(unsigned long tmp)
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

સ્થિર પૂર્ણાંક us_one_transfer(સંરચના us_data *us, પૂર્ણાંક pipe, અક્ષર *buf, પૂર્ણાંક length)
{
	પૂર્ણાંક max_size;
	પૂર્ણાંક this_xfer;
	પૂર્ણાંક result;
	પૂર્ણાંક partial;
	પૂર્ણાંક maxtry;
	પૂર્ણાંક stat;

	/* determine the maximum packet size for these transfers */
	max_size = usb_maxpacket(us->pusb_dev, pipe) * 16;

	/* while we have data left to transfer */
	જ્યારે (length) {

		/* calculate how long this will be -- maximum or a remainder */
		this_xfer = length > max_size ? max_size : length;
		length -= this_xfer;

		/* setup the retry counter */
		maxtry = 10;

		/* set up the transfer loop */
		કરો {
			/* transfer the data */
			debug("Bulk xfer 0x%lx(%d) try #%d\n",
			      (ulong)map_to_sysmem(buf), this_xfer,
			      11 - maxtry);
			result = usb_bulk_સંદેશ(us->pusb_dev, pipe, buf,
					      this_xfer, &partial,
					      USB_CNTL_TIMEOUT * 5);
			debug("bulk_msg returned %d xferred %d/%d\n",
			      result, partial, this_xfer);
			જો (us->pusb_dev->status != 0) {
				/* if we stall, we need to clear it before
				 * we go on
				 */
#ifdef DEBUG
				display_int_status(us->pusb_dev->status);
#endif
				જો (us->pusb_dev->status & USB_ST_STALLED) {
					debug("stalled ->clearing endpoint" \
					      "halt for pipe 0x%x\n", pipe);
					stat = us->pusb_dev->status;
					usb_clear_halt(us->pusb_dev, pipe);
					us->pusb_dev->status = stat;
					જો (this_xfer == partial) {
						debug("bulk transferred" \
						      "with error %lX," \
						      " but data ok\n",
						      us->pusb_dev->status);
						પાછા 0;
					}
					નહીંતર
						પાછા result;
				}
				જો (us->pusb_dev->status & USB_ST_NAK_REC) {
					debug("Device NAKed bulk_msg\n");
					પાછા result;
				}
				debug("bulk transferred with error");
				જો (this_xfer == partial) {
					debug(" %ld, but data ok\n",
					      us->pusb_dev->status);
					પાછા 0;
				}
				/* if our try counter reaches 0, bail out */
				debug(" %ld, data %d\n",
				      us->pusb_dev->status, partial);
				જો (!maxtry--)
						પાછા result;
			}
			/* update to show what data was transferred */
			this_xfer -= partial;
			buf += partial;
			/* continue until this transfer is done */
		} જ્યારે (this_xfer);
	}

	/* if we get here, we're done and successful */
	પાછા 0;
}

સ્થિર પૂર્ણાંક usb_stor_BBB_reset(સંરચના us_data *us)
{
	પૂર્ણાંક result;
	unsigned પૂર્ણાંક pipe;

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
	result = usb_control_સંદેશ(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_BBB_RESET,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, ખાલી, 0, USB_CNTL_TIMEOUT * 5);

	જો ((result < 0) && (us->pusb_dev->status & USB_ST_STALLED)) {
		debug("RESET:stall\n");
		પાછા -1;
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
	પાછા 0;
}

/* FIXME: this reset function doesn't really reset the port, and it
 * should. Actually it should probably do what it's doing here, and
 * reset the port physically
 */
સ્થિર પૂર્ણાંક usb_stor_CB_reset(સંરચના us_data *us)
{
	unsigned અક્ષર cmd[12];
	પૂર્ણાંક result;

	debug("CB_reset\n");
	memset(cmd, 0xff, કદ(cmd));
	cmd[0] = SCSI_SEND_DIAG;
	cmd[1] = 4;
	result = usb_control_સંદેશ(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_CBI_ADSC,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, cmd, કદ(cmd),
				 USB_CNTL_TIMEOUT * 5);

	/* long wait for reset */
	mdelay(1500);
	debug("CB_reset result %d: status %lX clearing endpoint halt\n",
	      result, us->pusb_dev->status);
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_in));
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_out));

	debug("CB_reset done\n");
	પાછા 0;
}

/*
 * Set up the command for a BBB device. Note that the actual SCSI
 * command is copied into cbw.CBWCDB.
 */
સ્થિર પૂર્ણાંક usb_stor_BBB_comdat(સંરચના scsi_cmd *srb, સંરચના us_data *us)
{
	પૂર્ણાંક result;
	પૂર્ણાંક actlen;
	પૂર્ણાંક dir_in;
	unsigned પૂર્ણાંક pipe;
	ALLOC_CACHE_ALIGN_BUFFER(સંરચના umass_bbb_cbw, cbw, 1);

	dir_in = US_DIRECTION(srb->cmd[0]);

#ifdef BBB_COMDAT_TRACE
	printf("dir %d lun %d cmdlen %d cmd %p datalen %lu pdata %p\n",
		dir_in, srb->lun, srb->cmdlen, srb->cmd, srb->datalen,
		srb->pdata);
	જો (srb->cmdlen) {
		માટે (result = 0; result < srb->cmdlen; result++)
			printf("cmd[%d] %#x ", result, srb->cmd[result]);
		printf("\n");
	}
#endif
	/* sanity checks */
	જો (!(srb->cmdlen <= CBWCDBLENGTH)) {
		debug("usb_stor_BBB_comdat:cmdlen too large\n");
		પાછા -1;
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
	result = usb_bulk_સંદેશ(us->pusb_dev, pipe, cbw, UMASS_BBB_CBW_SIZE,
			      &actlen, USB_CNTL_TIMEOUT * 5);
	જો (result < 0)
		debug("usb_stor_BBB_comdat:usb_bulk_msg error\n");
	પાછા result;
}

/* FIXME: we also need a CBI_command which sets up the completion
 * interrupt, and waits for it
 */
સ્થિર પૂર્ણાંક usb_stor_CB_comdat(સંરચના scsi_cmd *srb, સંરચના us_data *us)
{
	પૂર્ણાંક result = 0;
	પૂર્ણાંક dir_in, retry;
	unsigned પૂર્ણાંક pipe;
	unsigned long status;

	retry = 5;
	dir_in = US_DIRECTION(srb->cmd[0]);

	જો (dir_in)
		pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);
	નહીંતર
		pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_out);

	જ્યારે (retry--) {
		debug("CBI gets a command: Try %d\n", 5 - retry);
#ifdef DEBUG
		usb_show_srb(srb);
#endif
		/* let's send the command via the control pipe */
		result = usb_control_સંદેશ(us->pusb_dev,
					 usb_sndctrlpipe(us->pusb_dev , 0),
					 US_CBI_ADSC,
					 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
					 0, us->ifnum,
					 srb->cmd, srb->cmdlen,
					 USB_CNTL_TIMEOUT * 5);
		debug("CB_transport: control msg returned %d, status %lX\n",
		      result, us->pusb_dev->status);
		/* check the return code for the command */
		જો (result < 0) {
			જો (us->pusb_dev->status & USB_ST_STALLED) {
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
			પાછા result;
		}
		/* transfer the data payload for this command, if one exists*/

		debug("CB_transport: control msg returned %d," \
		      " direction is %s to go 0x%lx\n", result,
		      dir_in ? "IN" : "OUT", srb->datalen);
		જો (srb->datalen) {
			result = us_one_transfer(us, pipe, (અક્ષર *)srb->pdata,
						 srb->datalen);
			debug("CBI attempted to transfer data," \
			      " result is %d status %lX, len %d\n",
			      result, us->pusb_dev->status,
				us->pusb_dev->act_len);
			જો (!(us->pusb_dev->status & USB_ST_NAK_REC))
				તોડો;
		} /* if (srb->datalen) */
		નહીંતર
			તોડો;
	}
	/* return result */

	પાછા result;
}

સ્થિર પૂર્ણાંક usb_stor_CBI_get_status(સંરચના scsi_cmd *srb, સંરચના us_data *us)
{
	પૂર્ણાંક timeout;

	us->ip_wanted = 1;
	usb_int_સંદેશ(us->pusb_dev, us->irqpipe,
		    (શૂન્ય *)&us->ip_data, us->irqmaxp, us->irqinterval, false);
	timeout = 1000;
	જ્યારે (timeout--) {
		જો (us->ip_wanted == 0)
			તોડો;
		mdelay(10);
	}
	જો (us->ip_wanted) {
		printf("	Did not get interrupt on CBI\n");
		us->ip_wanted = 0;
		પાછા USB_STOR_TRANSPORT_ERROR;
	}
	debug("Got interrupt data 0x%x, transferred %d status 0x%lX\n",
	      us->ip_data, us->pusb_dev->irq_act_len,
	      us->pusb_dev->irq_status);
	/* UFI gives us ASC and ASCQ, like a request sense */
	જો (us->subclass == US_SC_UFI) {
		જો (srb->cmd[0] == SCSI_REQ_SENSE ||
		    srb->cmd[0] == SCSI_INQUIRY)
			પાછા USB_STOR_TRANSPORT_GOOD; /* Good */
		નહીંતર જો (us->ip_data)
			પાછા USB_STOR_TRANSPORT_FAILED;
		નહીંતર
			પાછા USB_STOR_TRANSPORT_GOOD;
	}
	/* otherwise, we interpret the data normally */
	પસંદગી (us->ip_data) {
	સ્થિતિ 0x0001:
		પાછા USB_STOR_TRANSPORT_GOOD;
	સ્થિતિ 0x0002:
		પાછા USB_STOR_TRANSPORT_FAILED;
	મૂળભૂત:
		પાછા USB_STOR_TRANSPORT_ERROR;
	}			/* switch */
	પાછા USB_STOR_TRANSPORT_ERROR;
}

#define USB_TRANSPORT_UNKNOWN_RETRY 5
#define USB_TRANSPORT_NOT_READY_RETRY 10

/* clear a stall on an endpoint - special for BBB devices */
સ્થિર પૂર્ણાંક usb_stor_BBB_clear_endpt_stall(સંરચના us_data *us, __u8 endpt)
{
	/* ENDPOINT_HALT = 0, so set value to 0 */
	પાછા usb_control_સંદેશ(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
			       USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
			       endpt, ખાલી, 0, USB_CNTL_TIMEOUT * 5);
}

સ્થિર પૂર્ણાંક usb_stor_BBB_transport(સંરચના scsi_cmd *srb, સંરચના us_data *us)
{
	પૂર્ણાંક result, retry;
	પૂર્ણાંક dir_in;
	પૂર્ણાંક actlen, data_actlen;
	unsigned પૂર્ણાંક pipe, pipein, pipeout;
	ALLOC_CACHE_ALIGN_BUFFER(સંરચના umass_bbb_csw, csw, 1);
#ifdef BBB_XPORT_TRACE
	unsigned અક્ષર *ptr;
	પૂર્ણાંક index;
#endif

	dir_in = US_DIRECTION(srb->cmd[0]);

	/* COMMAND phase */
	debug("COMMAND phase\n");
	result = usb_stor_BBB_comdat(srb, us);
	જો (result < 0) {
		debug("failed to send CBW status %ld\n",
		      us->pusb_dev->status);
		usb_stor_BBB_reset(us);
		પાછા USB_STOR_TRANSPORT_FAILED;
	}
	જો (!(us->flags & USB_READY))
		mdelay(5);
	pipein = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);
	pipeout = usb_sndbulkpipe(us->pusb_dev, us->ep_out);
	/* DATA phase + error handling */
	data_actlen = 0;
	/* no data, go immediately to the STATUS phase */
	જો (srb->datalen == 0)
		જાઓ st;
	debug("DATA phase\n");
	જો (dir_in)
		pipe = pipein;
	નહીંતર
		pipe = pipeout;

	result = usb_bulk_સંદેશ(us->pusb_dev, pipe, srb->pdata, srb->datalen,
			      &data_actlen, USB_CNTL_TIMEOUT * 5);
	/* special handling of STALL in DATA phase */
	જો ((result < 0) && (us->pusb_dev->status & USB_ST_STALLED)) {
		debug("DATA:stall\n");
		/* clear the STALL on the endpoint */
		result = usb_stor_BBB_clear_endpt_stall(us,
					dir_in ? us->ep_in : us->ep_out);
		જો (result >= 0)
			/* continue on to STATUS phase */
			જાઓ st;
	}
	જો (result < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->status);
		usb_stor_BBB_reset(us);
		પાછા USB_STOR_TRANSPORT_FAILED;
	}
#ifdef BBB_XPORT_TRACE
	માટે (index = 0; index < data_actlen; index++)
		printf("pdata[%d] %#x ", index, srb->pdata[index]);
	printf("\n");
#endif
	/* STATUS phase + error handling */
st:
	retry = 0;
again:
	debug("STATUS phase\n");
	result = usb_bulk_સંદેશ(us->pusb_dev, pipein, csw, UMASS_BBB_CSW_SIZE,
				&actlen, USB_CNTL_TIMEOUT*5);

	/* special handling of STALL in STATUS phase */
	જો ((result < 0) && (retry < 1) &&
	    (us->pusb_dev->status & USB_ST_STALLED)) {
		debug("STATUS:stall\n");
		/* clear the STALL on the endpoint */
		result = usb_stor_BBB_clear_endpt_stall(us, us->ep_in);
		જો (result >= 0 && (retry++ < 1))
			/* do a retry */
			જાઓ again;
	}
	જો (result < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->status);
		usb_stor_BBB_reset(us);
		પાછા USB_STOR_TRANSPORT_FAILED;
	}
#ifdef BBB_XPORT_TRACE
	ptr = (unsigned અક્ષર *)csw;
	માટે (index = 0; index < UMASS_BBB_CSW_SIZE; index++)
		printf("ptr[%d] %#x ", index, ptr[index]);
	printf("\n");
#endif
	/* misuse pipe to get the residue */
	pipe = le32_to_cpu(csw->dCSWDataResidue);
	જો (pipe == 0 && srb->datalen != 0 && srb->datalen - data_actlen != 0)
		pipe = srb->datalen - data_actlen;
	જો (CSWSIGNATURE != le32_to_cpu(csw->dCSWSignature)) {
		debug("!CSWSIGNATURE\n");
		usb_stor_BBB_reset(us);
		પાછા USB_STOR_TRANSPORT_FAILED;
	} નહીંતર જો ((CBWTag - 1) != le32_to_cpu(csw->dCSWTag)) {
		debug("!Tag\n");
		usb_stor_BBB_reset(us);
		પાછા USB_STOR_TRANSPORT_FAILED;
	} નહીંતર જો (csw->bCSWStatus > CSWSTATUS_PHASE) {
		debug(">PHASE\n");
		usb_stor_BBB_reset(us);
		પાછા USB_STOR_TRANSPORT_FAILED;
	} નહીંતર જો (csw->bCSWStatus == CSWSTATUS_PHASE) {
		debug("=PHASE\n");
		usb_stor_BBB_reset(us);
		પાછા USB_STOR_TRANSPORT_FAILED;
	} નહીંતર જો (data_actlen > srb->datalen) {
		debug("transferred %dB instead of %ldB\n",
		      data_actlen, srb->datalen);
		પાછા USB_STOR_TRANSPORT_FAILED;
	} નહીંતર જો (csw->bCSWStatus == CSWSTATUS_FAILED) {
		debug("FAILED\n");
		પાછા USB_STOR_TRANSPORT_FAILED;
	}

	પાછા result;
}

સ્થિર પૂર્ણાંક usb_stor_CB_transport(સંરચના scsi_cmd *srb, સંરચના us_data *us)
{
	પૂર્ણાંક result, status;
	સંરચના scsi_cmd *psrb;
	સંરચના scsi_cmd reqsrb;
	પૂર્ણાંક retry, notready;

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
	જો (us->protocol == US_PR_CBI) {
		status = usb_stor_CBI_get_status(srb, us);
		/* if the status is error, report it */
		જો (status == USB_STOR_TRANSPORT_ERROR) {
			debug(" USB CBI Command Error\n");
			પાછા status;
		}
		srb->sense_buf[12] = (unsigned અક્ષર)(us->ip_data >> 8);
		srb->sense_buf[13] = (unsigned અક્ષર)(us->ip_data & 0xff);
		જો (!us->ip_data) {
			/* if the status is good, report it */
			જો (status == USB_STOR_TRANSPORT_GOOD) {
				debug(" USB CBI Command Good\n");
				પાછા status;
			}
		}
	}
	/* do we have to issue an auto request? */
	/* HERE we have to check the result */
	જો ((result < 0) && !(us->pusb_dev->status & USB_ST_STALLED)) {
		debug("ERROR %lX\n", us->pusb_dev->status);
		us->transport_reset(us);
		પાછા USB_STOR_TRANSPORT_ERROR;
	}
	જો ((us->protocol == US_PR_CBI) &&
	    ((srb->cmd[0] == SCSI_REQ_SENSE) ||
	    (srb->cmd[0] == SCSI_INQUIRY))) {
		/* do not issue an autorequest after request sense */
		debug("No auto request and good\n");
		પાછા USB_STOR_TRANSPORT_GOOD;
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
	જો (us->protocol == US_PR_CBI)
		status = usb_stor_CBI_get_status(psrb, us);

	જો ((result < 0) && !(us->pusb_dev->status & USB_ST_STALLED)) {
		debug(" AUTO REQUEST ERROR %ld\n",
		      us->pusb_dev->status);
		પાછા USB_STOR_TRANSPORT_ERROR;
	}
	debug("autorequest returned 0x%02X 0x%02X 0x%02X 0x%02X\n",
	      srb->sense_buf[0], srb->sense_buf[2],
	      srb->sense_buf[12], srb->sense_buf[13]);
	/* Check the auto request result */
	જો ((srb->sense_buf[2] == 0) &&
	    (srb->sense_buf[12] == 0) &&
	    (srb->sense_buf[13] == 0)) {
		/* ok, no sense */
		પાછા USB_STOR_TRANSPORT_GOOD;
	}

	/* Check the auto request result */
	પસંદગી (srb->sense_buf[2]) {
	સ્થિતિ 0x01:
		/* Recovered Error */
		પાછા USB_STOR_TRANSPORT_GOOD;
		તોડો;
	સ્થિતિ 0x02:
		/* Not Ready */
		જો (notready++ > USB_TRANSPORT_NOT_READY_RETRY) {
			printf("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X (NOT READY)\n", srb->cmd[0],
				srb->sense_buf[0], srb->sense_buf[2],
				srb->sense_buf[12], srb->sense_buf[13]);
			પાછા USB_STOR_TRANSPORT_FAILED;
		} નહીંતર {
			mdelay(100);
			જાઓ do_retry;
		}
		તોડો;
	મૂળભૂત:
		જો (retry++ > USB_TRANSPORT_UNKNOWN_RETRY) {
			printf("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X\n", srb->cmd[0], srb->sense_buf[0],
				srb->sense_buf[2], srb->sense_buf[12],
				srb->sense_buf[13]);
			પાછા USB_STOR_TRANSPORT_FAILED;
		} નહીંતર
			જાઓ do_retry;
		તોડો;
	}
	પાછા USB_STOR_TRANSPORT_FAILED;
}

સ્થિર શૂન્ય usb_stor_set_max_xfer_blk(સંરચના usb_device *udev,
				      સંરચના us_data *us)
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

#જો CONFIG_IS_ENABLED(DM_USB)
	size_t size;
	પૂર્ણાંક ret;

	ret = usb_get_max_xfer_size(udev, (size_t *)&size);
	જો ((ret >= 0) && (size < blk * 512))
		blk = size / 512;
#endif

	us->max_xfer_blk = blk;
}

સ્થિર પૂર્ણાંક usb_inquiry(સંરચના scsi_cmd *srb, સંરચના us_data *ss)
{
	પૂર્ણાંક retry, i;
	retry = 5;
	કરો {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_INQUIRY;
		srb->cmd[1] = srb->lun << 5;
		srb->cmd[4] = 36;
		srb->datalen = 36;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		i = ss->transport(srb, ss);
		debug("inquiry returns %d\n", i);
		જો (i == 0)
			તોડો;
	} જ્યારે (--retry);

	જો (!retry) {
		printf("error in inquiry\n");
		પાછા -1;
	}
	પાછા 0;
}

સ્થિર પૂર્ણાંક usb_request_sense(સંરચના scsi_cmd *srb, સંરચના us_data *ss)
{
	અક્ષર *ptr;

	ptr = (અક્ષર *)srb->pdata;
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
	પાછા 0;
}

સ્થિર પૂર્ણાંક usb_test_unit_ready(સંરચના scsi_cmd *srb, સંરચના us_data *ss)
{
	પૂર્ણાંક retries = 10;

	કરો {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_TST_U_RDY;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 0;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		જો (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD) {
			ss->flags |= USB_READY;
			પાછા 0;
		}
		usb_request_sense(srb, ss);
		/*
		 * Check the Key Code Qualifier, if it matches
		 * "Not Ready - medium not present"
		 * (the sense Key equals 0x2 and the ASC is 0x3a)
		 * return immediately as the medium being absent won't change
		 * unless there is a user action.
		 */
		જો ((srb->sense_buf[2] == 0x02) &&
		    (srb->sense_buf[12] == 0x3a))
			પાછા -1;
		mdelay(100);
	} જ્યારે (retries--);

	પાછા -1;
}

સ્થિર પૂર્ણાંક usb_read_capacity(સંરચના scsi_cmd *srb, સંરચના us_data *ss)
{
	પૂર્ણાંક retry;
	/* XXX retries */
	retry = 3;
	કરો {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_RD_CAPAC;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 8;
		srb->cmdlen = ss->cmd12 ? 12 : 10;
		જો (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD)
			પાછા 0;
	} જ્યારે (retry--);

	પાછા -1;
}

સ્થિર પૂર્ણાંક usb_read_10(સંરચના scsi_cmd *srb, સંરચના us_data *ss,
		       unsigned long start, unsigned short blocks)
{
	memset(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_READ10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((unsigned અક્ષર) (start >> 24)) & 0xff;
	srb->cmd[3] = ((unsigned અક્ષર) (start >> 16)) & 0xff;
	srb->cmd[4] = ((unsigned અક્ષર) (start >> 8)) & 0xff;
	srb->cmd[5] = ((unsigned અક્ષર) (start)) & 0xff;
	srb->cmd[7] = ((unsigned અક્ષર) (blocks >> 8)) & 0xff;
	srb->cmd[8] = (unsigned અક્ષર) blocks & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("read10: start %lx blocks %x\n", start, blocks);
	પાછા ss->transport(srb, ss);
}

સ્થિર પૂર્ણાંક usb_write_10(સંરચના scsi_cmd *srb, સંરચના us_data *ss,
			unsigned long start, unsigned short blocks)
{
	memset(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_WRITE10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((unsigned અક્ષર) (start >> 24)) & 0xff;
	srb->cmd[3] = ((unsigned અક્ષર) (start >> 16)) & 0xff;
	srb->cmd[4] = ((unsigned અક્ષર) (start >> 8)) & 0xff;
	srb->cmd[5] = ((unsigned અક્ષર) (start)) & 0xff;
	srb->cmd[7] = ((unsigned અક્ષર) (blocks >> 8)) & 0xff;
	srb->cmd[8] = (unsigned અક્ષર) blocks & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("write10: start %lx blocks %x\n", start, blocks);
	પાછા ss->transport(srb, ss);
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
સ્થિર શૂન્ય usb_bin_fixup(સંરચના usb_device_descriptor descriptor,
				unsigned અક્ષર vendor[],
				unsigned અક્ષર product[]) {
	અચલ unsigned અક્ષર max_vendor_len = 40;
	અચલ unsigned અક્ષર max_product_len = 20;
	જો (descriptor.idVendor == 0x0424 && descriptor.idProduct == 0x223a) {
		strncpy((અક્ષર *)vendor, "SMSC", max_vendor_len);
		strncpy((અક્ષર *)product, "Flash Media Cntrller",
			max_product_len);
	}
}
#endif /* CONFIG_USB_BIN_FIXUP */

#જો CONFIG_IS_ENABLED(BLK)
સ્થિર unsigned long usb_stor_read(સંરચના udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, શૂન્ય *buffer)
#નહીંતર
સ્થિર unsigned long usb_stor_read(સંરચના blk_desc *block_dev, lbaint_t blknr,
				   lbaint_t blkcnt, શૂન્ય *buffer)
#endif
{
	lbaint_t start, blks;
	uintptr_t buf_addr;
	unsigned short smallblks;
	સંરચના usb_device *udev;
	સંરચના us_data *ss;
	પૂર્ણાંક retry;
	સંરચના scsi_cmd *srb = &usb_ccb;
#જો CONFIG_IS_ENABLED(BLK)
	સંરચના blk_desc *block_dev;
#endif

	જો (blkcnt == 0)
		પાછા 0;
	/* Setup  device */
#જો CONFIG_IS_ENABLED(BLK)
	block_dev = dev_get_uclass_plat(dev);
	udev = dev_get_parent_priv(dev_get_parent(dev));
	debug("\nusb_read: udev %d\n", block_dev->devnum);
#નહીંતર
	debug("\nusb_read: udev %d\n", block_dev->devnum);
	udev = usb_dev_desc[block_dev->devnum].priv;
	જો (!udev) {
		debug("%s: No device\n", __func__);
		પાછા 0;
	}
#endif
	ss = (સંરચના us_data *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_lock_async(udev, 1);
	srb->lun = block_dev->lun;
	buf_addr = (uintptr_t)buffer;
	start = blknr;
	blks = blkcnt;

	debug("\nusb_read: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      block_dev->devnum, start, blks, buf_addr);

	કરો {
		/* XXX need some comment here */
		retry = 2;
		srb->pdata = (unsigned અક્ષર *)buf_addr;
		જો (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		નહીંતર
			smallblks = (unsigned short) blks;
retry_it:
		જો (smallblks == ss->max_xfer_blk)
			usb_show_progress();
		srb->datalen = block_dev->blksz * smallblks;
		srb->pdata = (unsigned અક્ષર *)buf_addr;
		જો (usb_read_10(srb, ss, start, smallblks)) {
			debug("Read ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			જો (retry--)
				જાઓ retry_it;
			blkcnt -= blks;
			તોડો;
		}
		start += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} જ્યારે (blks != 0);

	debug("usb_read: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      start, smallblks, buf_addr);

	usb_lock_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	જો (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	પાછા blkcnt;
}

#જો CONFIG_IS_ENABLED(BLK)
સ્થિર unsigned long usb_stor_write(સંરચના udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, અચલ શૂન્ય *buffer)
#નહીંતર
સ્થિર unsigned long usb_stor_write(સંરચના blk_desc *block_dev, lbaint_t blknr,
				    lbaint_t blkcnt, અચલ શૂન્ય *buffer)
#endif
{
	lbaint_t start, blks;
	uintptr_t buf_addr;
	unsigned short smallblks;
	સંરચના usb_device *udev;
	સંરચના us_data *ss;
	પૂર્ણાંક retry;
	સંરચના scsi_cmd *srb = &usb_ccb;
#જો CONFIG_IS_ENABLED(BLK)
	સંરચના blk_desc *block_dev;
#endif

	જો (blkcnt == 0)
		પાછા 0;

	/* Setup  device */
#જો CONFIG_IS_ENABLED(BLK)
	block_dev = dev_get_uclass_plat(dev);
	udev = dev_get_parent_priv(dev_get_parent(dev));
	debug("\nusb_read: udev %d\n", block_dev->devnum);
#નહીંતર
	debug("\nusb_read: udev %d\n", block_dev->devnum);
	udev = usb_dev_desc[block_dev->devnum].priv;
	જો (!udev) {
		debug("%s: No device\n", __func__);
		પાછા 0;
	}
#endif
	ss = (સંરચના us_data *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_lock_async(udev, 1);

	srb->lun = block_dev->lun;
	buf_addr = (uintptr_t)buffer;
	start = blknr;
	blks = blkcnt;

	debug("\nusb_write: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      block_dev->devnum, start, blks, buf_addr);

	કરો {
		/* If write fails retry for max retry count else
		 * return with number of blocks written successfully.
		 */
		retry = 2;
		srb->pdata = (unsigned અક્ષર *)buf_addr;
		જો (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		નહીંતર
			smallblks = (unsigned short) blks;
retry_it:
		જો (smallblks == ss->max_xfer_blk)
			usb_show_progress();
		srb->datalen = block_dev->blksz * smallblks;
		srb->pdata = (unsigned અક્ષર *)buf_addr;
		જો (usb_write_10(srb, ss, start, smallblks)) {
			debug("Write ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			જો (retry--)
				જાઓ retry_it;
			blkcnt -= blks;
			તોડો;
		}
		start += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} જ્યારે (blks != 0);

	debug("usb_write: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      start, smallblks, buf_addr);

	usb_lock_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	જો (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	પાછા blkcnt;

}

/* Probe to see if a new device is actually a Storage device */
પૂર્ણાંક usb_storage_probe(સંરચના usb_device *dev, unsigned પૂર્ણાંક ifnum,
		      સંરચના us_data *ss)
{
	સંરચના usb_interface *iface;
	પૂર્ણાંક i;
	સંરચના usb_endpoint_descriptor *ep_desc;
	unsigned પૂર્ણાંક flags = 0;

	/* let's examine the device now */
	iface = &dev->config.if_desc[ifnum];

	જો (dev->descriptor.bDeviceClass != 0 ||
			iface->desc.bInterfaceClass != USB_CLASS_MASS_STORAGE ||
			iface->desc.bInterfaceSubClass < US_SC_MIN ||
			iface->desc.bInterfaceSubClass > US_SC_MAX) {
		debug("Not mass storage\n");
		/* if it's not a mass storage, we go no further */
		પાછા 0;
	}

	memset(ss, 0, કદ(સંરચના us_data));

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
	પસંદગી (ss->protocol) {
	સ્થિતિ US_PR_CB:
		debug("Control/Bulk\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		તોડો;

	સ્થિતિ US_PR_CBI:
		debug("Control/Bulk/Interrupt\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		તોડો;
	સ્થિતિ US_PR_BULK:
		debug("Bulk/Bulk/Bulk\n");
		ss->transport = usb_stor_BBB_transport;
		ss->transport_reset = usb_stor_BBB_reset;
		તોડો;
	મૂળભૂત:
		printf("USB Storage Transport unknown / not yet implemented\n");
		પાછા 0;
		તોડો;
	}

	/*
	 * We are expecting a minimum of 2 endpoints - in and out (bulk).
	 * An optional interrupt is OK (necessary for CBI protocol).
	 * We will ignore any others.
	 */
	માટે (i = 0; i < iface->desc.b_અંક_Endpoints; i++) {
		ep_desc = &iface->ep_desc[i];
		/* is it an BULK endpoint? */
		જો ((ep_desc->bmAttributes &
		     USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK) {
			જો (ep_desc->bEndpointAddress & USB_DIR_IN)
				ss->ep_in = ep_desc->bEndpointAddress &
						USB_ENDPOINT_સંખ્યા_MASK;
			નહીંતર
				ss->ep_out =
					ep_desc->bEndpointAddress &
					USB_ENDPOINT_સંખ્યા_MASK;
		}

		/* is it an interrupt endpoint? */
		જો ((ep_desc->bmAttributes &
		     USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT) {
			ss->ep_int = ep_desc->bEndpointAddress &
						USB_ENDPOINT_સંખ્યા_MASK;
			ss->irqinterval = ep_desc->bInterval;
		}
	}
	debug("Endpoints In %d Out %d Int %d\n",
	      ss->ep_in, ss->ep_out, ss->ep_int);

	/* Do some basic sanity checks, and bail if we find a problem */
	જો (usb_set_interface(dev, iface->desc.b_Interface_સંખ્યા, 0) ||
	    !ss->ep_in || !ss->ep_out ||
	    (ss->protocol == US_PR_CBI && ss->ep_int == 0)) {
		debug("Problems with device\n");
		પાછા 0;
	}
	/* set class specific stuff */
	/* We only handle certain protocols.  Currently, these are
	 * the only ones.
	 * The SFF8070 accepts the requests used in u-boot
	 */
	જો (ss->subclass != US_SC_UFI && ss->subclass != US_SC_SCSI &&
	    ss->subclass != US_SC_8070) {
		printf("Sorry, protocol %d not yet supported.\n", ss->subclass);
		પાછા 0;
	}

	/* UFI uses 12-byte commands (like RBC, unlike SCSI) */
	જો (ss->subclass == US_SC_UFI)
		ss->cmd12 = true;

	જો (ss->ep_int) {
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

	dev->privptr = (શૂન્ય *)ss;
	પાછા 1;
}

પૂર્ણાંક usb_stor_get_info(સંરચના usb_device *dev, સંરચના us_data *ss,
		      સંરચના blk_desc *dev_desc)
{
	unsigned અક્ષર perq, modi;
	ALLOC_CACHE_ALIGN_BUFFER(u32, cap, 2);
	ALLOC_CACHE_ALIGN_BUFFER(u8, usb_stor_buf, 36);
	u32 capacity, blksz;
	સંરચના scsi_cmd *pccb = &usb_ccb;

	pccb->pdata = usb_stor_buf;

	dev_desc->target = dev->devnum;
	pccb->lun = dev_desc->lun;
	debug(" address %d\n", dev_desc->target);

	જો (usb_inquiry(pccb, ss)) {
		debug("%s: usb_inquiry() failed\n", __func__);
		પાછા -1;
	}

	perq = usb_stor_buf[0];
	modi = usb_stor_buf[1];

	/*
	 * Skip unknown devices (0x1f) and enclosure service devices (0x0d),
	 * they would not respond to test_unit_ready .
	 */
	જો (((perq & 0x1f) == 0x1f) || ((perq & 0x1f) == 0x0d)) {
		debug("%s: unknown/unsupported device\n", __func__);
		પાછા 0;
	}
	જો ((modi&0x80) == 0x80) {
		/* drive is removable */
		dev_desc->removable = 1;
	}
	memcpy(dev_desc->vendor, (અચલ શૂન્ય *)&usb_stor_buf[8], 8);
	memcpy(dev_desc->product, (અચલ શૂન્ય *)&usb_stor_buf[16], 16);
	memcpy(dev_desc->revision, (અચલ શૂન્ય *)&usb_stor_buf[32], 4);
	dev_desc->vendor[8] = 0;
	dev_desc->product[16] = 0;
	dev_desc->revision[4] = 0;
#ifdef CONFIG_USB_BIN_FIXUP
	usb_bin_fixup(dev->descriptor, (uchar *)dev_desc->vendor,
		      (uchar *)dev_desc->product);
#endif /* CONFIG_USB_BIN_FIXUP */
	debug("ISO Vers %X, Response Data %X\n", usb_stor_buf[2],
	      usb_stor_buf[3]);
	જો (usb_test_unit_ready(pccb, ss)) {
		printf("Device NOT ready\n"
		       "   Request Sense returned %02X %02X %02X\n",
		       pccb->sense_buf[2], pccb->sense_buf[12],
		       pccb->sense_buf[13]);
		જો (dev_desc->removable == 1)
			dev_desc->type = perq;
		પાછા 0;
	}
	pccb->pdata = (unsigned અક્ષર *)cap;
	memset(pccb->pdata, 0, 8);
	જો (usb_read_capacity(pccb, ss) != 0) {
		printf("READ_CAP ERROR\n");
		ss->flags &= ~USB_READY;
		cap[0] = 2880;
		cap[1] = 0x200;
	}
	debug("Read Capacity returns: 0x%08x, 0x%08x\n", cap[0], cap[1]);
#જો 0
	જો (cap[0] > (0x200000 * 10)) /* greater than 10 GByte */
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

	પાછા 1;
}

#જો CONFIG_IS_ENABLED(DM_USB)

સ્થિર પૂર્ણાંક usb_mass_storage_probe(સંરચના udevice *dev)
{
	સંરચના usb_device *udev = dev_get_parent_priv(dev);
	પૂર્ણાંક ret;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	ret = usb_stor_probe_device(udev);
	usb_disable_asynch(0); /* asynch transfer allowed */

	પાછા ret;
}

સ્થિર અચલ સંરચના udevice_id usb_mass_storage_ids[] = {
	{ .compatible = "usb-mass-storage" },
	{ }
};

U_BOOT_DRIVER(usb_mass_storage) = {
	.name	= "usb_mass_storage",
	.id	= UCLASS_MASS_STORAGE,
	.of_match = usb_mass_storage_ids,
	.probe = usb_mass_storage_probe,
#જો CONFIG_IS_ENABLED(BLK)
	.plat_auto	= કદ(સંરચના us_data),
#endif
};

UCLASS_DRIVER(usb_mass_storage) = {
	.id		= UCLASS_MASS_STORAGE,
	.name		= "usb_mass_storage",
};

સ્થિર અચલ સંરચના usb_device_id mass_storage_id_table[] = {
	{
		.match_flags = USB_DEVICE_ID_MATCH_INT_CLASS,
		.bInterfaceClass = USB_CLASS_MASS_STORAGE
	},
	{ }		/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_mass_storage, mass_storage_id_table);
#endif

#જો CONFIG_IS_ENABLED(BLK)
સ્થિર અચલ સંરચના blk_ops usb_storage_ops = {
	.પઠ	= usb_stor_read,
	.લિખ	= usb_stor_write,
};

U_BOOT_DRIVER(usb_storage_blk) = {
	.name		= "usb_storage_blk",
	.id		= UCLASS_BLK,
	.ops		= &usb_storage_ops,
};
#નહીંતર
U_BOOT_LEGACY_BLK(usb) = {
	.uclass_idname	= "usb",
	.uclass_id	= UCLASS_USB,
	.max_devs	= USB_MAX_STOR_DEV,
	.desc		= usb_dev_desc,
};
#endif
