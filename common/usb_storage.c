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

#అన్తర్భావ <blk.h>
#అన్తర్భావ <bootdev.h>
#అన్తర్భావ <command.h>
#అన్తర్భావ <dm.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <mapmem.h>
#అన్తర్భావ <memalign.h>
#అన్తర్భావ <asm/byteorder.h>
#అన్తర్భావ <asm/cache.h>
#అన్తర్భావ <asm/processor.h>
#అన్తర్భావ <dm/device-internal.h>
#అన్తర్భావ <dm/lists.h>
#అన్తర్భావ <linux/delay.h>

#అన్తర్భావ <part.h>
#అన్తర్భావ <usb.h>

#నిర్వచనం_రద్దు BBB_COMDAT_TRACE
#నిర్వచనం_రద్దు BBB_XPORT_TRACE

#అన్తర్భావ <scsi.h>
/* direction table -- this indicates the direction of the data
 * transfer for each command code -- a 1 indicates input
 */
స్థిర స్థిరరాశి గుర్తులేని అక్షరం us_direction[256/8] = {
	0x28, 0x81, 0x14, 0x14, 0x20, 0x01, 0x90, 0x77,
	0x0C, 0x20, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#నిర్వచించు US_DIRECTION(x) ((us_direction[x>>3] >> (x & 7)) & 1)

స్థిర నిర్మాణం scsi_cmd usb_ccb __aligned(ARCH_DMA_MINALIGN);
స్థిర __u32 CBWTag;

స్థిర పూర్ణాంకం usb_max_devs; /* number of highest available usb device */

#ఒకవేళ !CONFIG_IS_ENABLED(BLK)
స్థిర నిర్మాణం blk_desc usb_dev_desc[USB_MAX_STOR_DEV];
#అయితే_ముగింపు

నిర్మాణం us_దత్తాంశం;
రకనిర్వచనం పూర్ణాంకం (*trans_cmnd)(నిర్మాణం scsi_cmd *cb, నిర్మాణం us_దత్తాంశం *దత్తాంశం);
రకనిర్వచనం పూర్ణాంకం (*trans_reset)(నిర్మాణం us_దత్తాంశం *దత్తాంశం);

నిర్మాణం us_దత్తాంశం {
	నిర్మాణం usb_device *pusb_dev;	 /* this usb_device */

	గుర్తులేని పూర్ణాంకం	flags;			/* from filter initially */
#	define USB_READY	(1 << 0)
	గుర్తులేని అక్షరం	ifnum;			/* interface number */
	గుర్తులేని అక్షరం	ep_లోపల;			/* in endpoint */
	గుర్తులేని అక్షరం	ep_బహిర్గతం;			/* out ....... */
	గుర్తులేని అక్షరం	ep_int;			/* interrupt . */
	గుర్తులేని అక్షరం	subclass;		/* as in overview */
	గుర్తులేని అక్షరం	protocol;		/* .............. */
	గుర్తులేని అక్షరం	attention_పూర్తి;		/* force attn on first cmd */
	గుర్తులేని పొట్టి	ip_దత్తాంశం;		/* interrupt data */
	పూర్ణాంకం		action;			/* what to do */
	పూర్ణాంకం		ip_wanted;		/* needed */
	పూర్ణాంకం		*irq_handle;		/* for USB int requests */
	గుర్తులేని పూర్ణాంకం	irqpipe;		/* pipe for release_irq */
	గుర్తులేని అక్షరం	irqmaxp;		/* max packed for irq Pipe */
	గుర్తులేని అక్షరం	irqinterval;		/* Intervall for IRQ Pipe */
	నిర్మాణం scsi_cmd	*srb;			/* current srb */
	trans_reset	transport_reset;	/* reset routine */
	trans_cmnd	transport;		/* transport routine */
	గుర్తులేని పొట్టి	max_xfer_blk;		/* maximum transfer blocks */
	bool		cmd12;			/* use 12-byte commands (RBC/UFI) */
};

#ఒకవేళ !CONFIG_IS_ENABLED(BLK)
స్థిర నిర్మాణం us_దత్తాంశం usb_stor[USB_MAX_STOR_DEV];
#అయితే_ముగింపు

#నిర్వచించు USB_STOR_TRANSPORT_GOOD	   0
#నిర్వచించు USB_STOR_TRANSPORT_FAILED -1
#నిర్వచించు USB_STOR_TRANSPORT_ERROR  -2

పూర్ణాంకం usb_stor_get_info(నిర్మాణం usb_device *dev, నిర్మాణం us_దత్తాంశం *us,
		      నిర్మాణం blk_desc *dev_desc);
పూర్ణాంకం usb_storage_probe(నిర్మాణం usb_device *dev, గుర్తులేని పూర్ణాంకం ifnum,
		      నిర్మాణం us_దత్తాంశం *ss);
#ఒకవేళ CONFIG_IS_ENABLED(BLK)
స్థిర గుర్తులేని దీర్ఘ usb_stor_read(నిర్మాణం udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, శూన్య *బఫరు);
స్థిర గుర్తులేని దీర్ఘ usb_stor_write(నిర్మాణం udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, స్థిరరాశి శూన్య *బఫరు);
#లేకపోతే
స్థిర గుర్తులేని దీర్ఘ usb_stor_read(నిర్మాణం blk_desc *ఖండం_dev, lbaint_t blknr,
				   lbaint_t blkcnt, శూన్య *బఫరు);
స్థిర గుర్తులేని దీర్ఘ usb_stor_write(నిర్మాణం blk_desc *ఖండం_dev, lbaint_t blknr,
				    lbaint_t blkcnt, స్థిరరాశి శూన్య *బఫరు);
#అయితే_ముగింపు
శూన్య uhci_show_తాత్కాలిక_int_td(శూన్య);

స్థిర శూన్య usb_show_పురోగతి(శూన్య)
{
	debug(".");
}

/*******************************************************************************
 * show info on storage devices; 'usb start/init' must be invoked earlier
 * as we only retrieve structures populated during devices initialization
 */
పూర్ణాంకం usb_stor_info(శూన్య)
{
	పూర్ణాంకం లెక్క = 0;
#ఒకవేళ CONFIG_IS_ENABLED(BLK)
	నిర్మాణం udevice *dev;

	కొరకు (blk_మొదటి_device(UCLASS_USB, &dev);
	     dev;
	     blk_తదుపరి_device(&dev)) {
		నిర్మాణం blk_desc *desc = dev_get_uclass_plat(dev);

		రాయి("  Device %d: ", desc->devnum);
		dev_print(desc);
		లెక్క++;
	}
#లేకపోతే
	పూర్ణాంకం i;

	ఒకవేళ (usb_max_devs > 0) {
		కొరకు (i = 0; i < usb_max_devs; i++) {
			రాయి("  Device %d: ", i);
			dev_print(&usb_dev_desc[i]);
		}
		తిరిగి 0;
	}
#అయితే_ముగింపు
	ఒకవేళ (!లెక్క) {
		రాయి("No storage devices, perhaps not 'usb start'ed..?\n");
		తిరిగి 1;
	}

	తిరిగి 0;
}

స్థిర గుర్తులేని పూర్ణాంకం usb_get_max_lun(నిర్మాణం us_దత్తాంశం *us)
{
	పూర్ణాంకం len;
	ALLOC_CACHE_ALIGN_బఫరు(గుర్తులేని అక్షరం, ఫలితం, 1);
	len = usb_control_సందేశం(us->pusb_dev,
			      usb_rcvctrlpipe(us->pusb_dev, 0),
			      US_BBB_GET_MAX_LUN,
			      USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_లోపల,
			      0, us->ifnum,
			      ఫలితం, పరిమాణం(అక్షరం),
			      USB_CNTL_TIMEOUT * 5);
	debug("Get Max LUN -> len = %i, result = %i\n", len, (పూర్ణాంకం) *ఫలితం);
	తిరిగి (len > 0) ? *ఫలితం : 0;
}

స్థిర పూర్ణాంకం usb_stor_probe_device(నిర్మాణం usb_device *udev)
{
	పూర్ణాంకం lun, max_lun;

#ఒకవేళ CONFIG_IS_ENABLED(BLK)
	నిర్మాణం us_దత్తాంశం *దత్తాంశం;
	పూర్ణాంకం ret;
#లేకపోతే
	పూర్ణాంకం ప్రారంభం;

	ఒకవేళ (udev == ఖాళీ)
		తిరిగి -ENOENT; /* no more devices available */
#అయితే_ముగింపు

	debug("\n\nProbing for storage\n");
#ఒకవేళ CONFIG_IS_ENABLED(BLK)
	/*
	 * We store the us_data in the mass storage device's plat. It
	 * is shared by all LUNs (block devices) attached to this mass storage
	 * device.
	 */
	దత్తాంశం = dev_get_plat(udev->dev);
	ఒకవేళ (!usb_storage_probe(udev, 0, దత్తాంశం))
		తిరిగి 0;
	max_lun = usb_get_max_lun(దత్తాంశం);
	కొరకు (lun = 0; lun <= max_lun; lun++) {
		నిర్మాణం blk_desc *blkdev;
		నిర్మాణం udevice *dev;
		అక్షరం str[10];

		snprintf(str, పరిమాణం(str), "lun%d", lun);
		ret = blk_create_devicef(udev->dev, "usb_storage_blk", str,
					 UCLASS_USB, usb_max_devs,
					 DEFAULT_BLKSZ, 0, &dev);
		ఒకవేళ (ret) {
			debug("Cannot bind driver\n");
			తిరిగి ret;
		}

		blkdev = dev_get_uclass_plat(dev);
		blkdev->లక్ష్యం = 0xff;
		blkdev->lun = lun;

		ret = usb_stor_get_info(udev, దత్తాంశం, blkdev);
		ఒకవేళ (ret == 1) {
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		} లేకపోతే {
			debug("usb_stor_get_info: Invalid device\n");
			ret = device_unbind(dev);
			ఒకవేళ (ret)
				తిరిగి ret;
			కొనసాగు;
		}

		ret = blk_probe_or_unbind(dev);
		ఒకవేళ (ret)
			తిరిగి ret;

		ret = bootdev_అమరిక_for_sibling_blk(dev, "usb_bootdev");
		ఒకవేళ (ret) {
			పూర్ణాంకం ret2;

			ret2 = device_unbind(dev);
			ఒకవేళ (ret2)
				తిరిగి log_సందేశం_ret("bootdev", ret2);
			తిరిగి log_సందేశం_ret("bootdev", ret);
		}
	}
#లేకపోతే
	/* We don't have space to even probe if we hit the maximum */
	ఒకవేళ (usb_max_devs == USB_MAX_STOR_DEV) {
		రాయి("max USB Storage Device reached: %d stopping\n",
		       usb_max_devs);
		తిరిగి -ENOSPC;
	}

	ఒకవేళ (!usb_storage_probe(udev, 0, &usb_stor[usb_max_devs]))
		తిరిగి 0;

	/*
	 * OK, it's a storage device.  Iterate over its LUNs and populate
	 * usb_dev_desc'
	 */
	ప్రారంభం = usb_max_devs;

	max_lun = usb_get_max_lun(&usb_stor[usb_max_devs]);
	కొరకు (lun = 0; lun <= max_lun && usb_max_devs < USB_MAX_STOR_DEV;
	     lun++) {
		నిర్మాణం blk_desc *blkdev;

		blkdev = &usb_dev_desc[usb_max_devs];
		memset(blkdev, '\0', పరిమాణం(నిర్మాణం blk_desc));
		blkdev->uclass_id = UCLASS_USB;
		blkdev->devnum = usb_max_devs;
		blkdev->part_type = PART_TYPE_UNKNOWN;
		blkdev->లక్ష్యం = 0xff;
		blkdev->type = DEV_TYPE_UNKNOWN;
		blkdev->ఖండం_read = usb_stor_read;
		blkdev->ఖండం_write = usb_stor_write;
		blkdev->lun = lun;
		blkdev->priv = udev;

		ఒకవేళ (usb_stor_get_info(udev, &usb_stor[ప్రారంభం],
				      &usb_dev_desc[usb_max_devs]) == 1) {
			debug("partype: %d\n", blkdev->part_type);
			part_init(blkdev);
			debug("partype: %d\n", blkdev->part_type);
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		}
	}
#అయితే_ముగింపు

	తిరిగి 0;
}

శూన్య usb_stor_reset(శూన్య)
{
	usb_max_devs = 0;
}

/*******************************************************************************
 * scan the usb and reports device info
 * to the user if mode = 1
 * returns current device or -1 if no
 */
పూర్ణాంకం usb_stor_పరిశీలన(పూర్ణాంకం బహుళకం)
{
	ఒకవేళ (బహుళకం == 1)
		రాయి("       scanning usb for storage devices... ");

#ఒకవేళ !CONFIG_IS_ENABLED(DM_USB)
	గుర్తులేని అక్షరం i;

	usb_disable_asynch(1); /* asynch transfer not allowed */

	usb_stor_reset();
	కొరకు (i = 0; i < USB_MAX_DEVICE; i++) {
		నిర్మాణం usb_device *dev;

		dev = usb_get_dev_సూచీ(i); /* get device */
		debug("i=%d\n", i);
		ఒకవేళ (usb_stor_probe_device(dev))
			ఆపు;
	} /* for */

	usb_disable_asynch(0); /* asynch transfer allowed */
#అయితే_ముగింపు
	రాయి("%d Storage Device(s) found\n", usb_max_devs);
	ఒకవేళ (usb_max_devs > 0)
		తిరిగి 0;
	తిరిగి -1;
}

స్థిర పూర్ణాంకం usb_stor_irq(నిర్మాణం usb_device *dev)
{
	నిర్మాణం us_దత్తాంశం *us;
	us = (నిర్మాణం us_దత్తాంశం *)dev->privptr;

	ఒకవేళ (us->ip_wanted)
		us->ip_wanted = 0;
	తిరిగి 0;
}

#నిర్వచితమైతే	DEBUG

స్థిర శూన్య usb_show_srb(నిర్మాణం scsi_cmd *pccb)
{
	పూర్ణాంకం i;
	రాయి("SRB: len %d datalen 0x%lX\n ", pccb->cmdlen, pccb->datalen);
	కొరకు (i = 0; i < pccb->cmdlen; i++)
		రాయి("%02X ", pccb->cmd[i]);
	రాయి("\n");
}

స్థిర శూన్య display_int_స్థితి(గుర్తులేని దీర్ఘ tmp)
{
	రాయి("Status: %s %s %s %s %s %s %s\n",
		(tmp & USB_ST_చురుకైన) ? "Active" : "",
		(tmp & USB_ST_STALLED) ? "Stalled" : "",
		(tmp & USB_ST_BUF_ERR) ? "Buffer Error" : "",
		(tmp & USB_ST_BABBLE_DET) ? "Babble Det" : "",
		(tmp & USB_ST_NAK_REC) ? "NAKed" : "",
		(tmp & USB_ST_చక్రీయ_తనిఖీ_ERR) ? "CRC Error" : "",
		(tmp & USB_ST_బిట్టు_ERR) ? "Bitstuff Error" : "");
}
#అయితే_ముగింపు
/***********************************************************************
 * Data transfer routines
 ***********************************************************************/

స్థిర పూర్ణాంకం us_one_transfer(నిర్మాణం us_దత్తాంశం *us, పూర్ణాంకం pipe, అక్షరం *buf, పూర్ణాంకం నిడివి)
{
	పూర్ణాంకం max_కొలత;
	పూర్ణాంకం this_xfer;
	పూర్ణాంకం ఫలితం;
	పూర్ణాంకం partial;
	పూర్ణాంకం maxtry;
	పూర్ణాంకం stat;

	/* determine the maximum packet size for these transfers */
	max_కొలత = usb_maxpacket(us->pusb_dev, pipe) * 16;

	/* while we have data left to transfer */
	అయితే (నిడివి) {

		/* calculate how long this will be -- maximum or a remainder */
		this_xfer = నిడివి > max_కొలత ? max_కొలత : నిడివి;
		నిడివి -= this_xfer;

		/* setup the retry counter */
		maxtry = 10;

		/* set up the transfer loop */
		చేయి {
			/* transfer the data */
			debug("Bulk xfer 0x%lx(%d) try #%d\n",
			      (ulong)map_గమ్యం_sysmem(buf), this_xfer,
			      11 - maxtry);
			ఫలితం = usb_bulk_సందేశం(us->pusb_dev, pipe, buf,
					      this_xfer, &partial,
					      USB_CNTL_TIMEOUT * 5);
			debug("bulk_msg returned %d xferred %d/%d\n",
			      ఫలితం, partial, this_xfer);
			ఒకవేళ (us->pusb_dev->స్థితి_2 != 0) {
				/* if we stall, we need to clear it before
				 * we go on
				 */
#నిర్వచితమైతే DEBUG
				display_int_స్థితి(us->pusb_dev->స్థితి_2);
#అయితే_ముగింపు
				ఒకవేళ (us->pusb_dev->స్థితి_2 & USB_ST_STALLED) {
					debug("stalled ->clearing endpoint" \
					      "halt for pipe 0x%x\n", pipe);
					stat = us->pusb_dev->స్థితి_2;
					usb_clear_halt(us->pusb_dev, pipe);
					us->pusb_dev->స్థితి_2 = stat;
					ఒకవేళ (this_xfer == partial) {
						debug("bulk transferred" \
						      "with error %lX," \
						      " but data ok\n",
						      us->pusb_dev->స్థితి_2);
						తిరిగి 0;
					}
					లేకపోతే
						తిరిగి ఫలితం;
				}
				ఒకవేళ (us->pusb_dev->స్థితి_2 & USB_ST_NAK_REC) {
					debug("Device NAKed bulk_msg\n");
					తిరిగి ఫలితం;
				}
				debug("bulk transferred with error");
				ఒకవేళ (this_xfer == partial) {
					debug(" %ld, but data ok\n",
					      us->pusb_dev->స్థితి_2);
					తిరిగి 0;
				}
				/* if our try counter reaches 0, bail out */
				debug(" %ld, data %d\n",
				      us->pusb_dev->స్థితి_2, partial);
				ఒకవేళ (!maxtry--)
						తిరిగి ఫలితం;
			}
			/* update to show what data was transferred */
			this_xfer -= partial;
			buf += partial;
			/* continue until this transfer is done */
		} అయితే (this_xfer);
	}

	/* if we get here, we're done and successful */
	తిరిగి 0;
}

స్థిర పూర్ణాంకం usb_stor_BBB_reset(నిర్మాణం us_దత్తాంశం *us)
{
	పూర్ణాంకం ఫలితం;
	గుర్తులేని పూర్ణాంకం pipe;

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
	ఫలితం = usb_control_సందేశం(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_BBB_RESET,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, ఖాళీ, 0, USB_CNTL_TIMEOUT * 5);

	ఒకవేళ ((ఫలితం < 0) && (us->pusb_dev->స్థితి_2 & USB_ST_STALLED)) {
		debug("RESET:stall\n");
		తిరిగి -1;
	}

	/* long wait for reset */
	mdelay(150);
	debug("BBB_reset result %d: status %lX reset\n",
	      ఫలితం, us->pusb_dev->స్థితి_2);
	pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_లోపల);
	ఫలితం = usb_clear_halt(us->pusb_dev, pipe);
	/* long wait for reset */
	mdelay(150);
	debug("BBB_reset result %d: status %lX clearing IN endpoint\n",
	      ఫలితం, us->pusb_dev->స్థితి_2);
	/* long wait for reset */
	pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_బహిర్గతం);
	ఫలితం = usb_clear_halt(us->pusb_dev, pipe);
	mdelay(150);
	debug("BBB_reset result %d: status %lX clearing OUT endpoint\n",
	      ఫలితం, us->pusb_dev->స్థితి_2);
	debug("BBB_reset done\n");
	తిరిగి 0;
}

/* FIXME: this reset function doesn't really reset the port, and it
 * should. Actually it should probably do what it's doing here, and
 * reset the port physically
 */
స్థిర పూర్ణాంకం usb_stor_CB_reset(నిర్మాణం us_దత్తాంశం *us)
{
	గుర్తులేని అక్షరం cmd[12];
	పూర్ణాంకం ఫలితం;

	debug("CB_reset\n");
	memset(cmd, 0xff, పరిమాణం(cmd));
	cmd[0] = SCSI_SEND_DIAG;
	cmd[1] = 4;
	ఫలితం = usb_control_సందేశం(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_CBI_ADSC,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, cmd, పరిమాణం(cmd),
				 USB_CNTL_TIMEOUT * 5);

	/* long wait for reset */
	mdelay(1500);
	debug("CB_reset result %d: status %lX clearing endpoint halt\n",
	      ఫలితం, us->pusb_dev->స్థితి_2);
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_లోపల));
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_బహిర్గతం));

	debug("CB_reset done\n");
	తిరిగి 0;
}

/*
 * Set up the command for a BBB device. Note that the actual SCSI
 * command is copied into cbw.CBWCDB.
 */
స్థిర పూర్ణాంకం usb_stor_BBB_comdat(నిర్మాణం scsi_cmd *srb, నిర్మాణం us_దత్తాంశం *us)
{
	పూర్ణాంకం ఫలితం;
	పూర్ణాంకం actlen;
	పూర్ణాంకం dir_లోపల;
	గుర్తులేని పూర్ణాంకం pipe;
	ALLOC_CACHE_ALIGN_బఫరు(నిర్మాణం umass_bbb_cbw, cbw, 1);

	dir_లోపల = US_DIRECTION(srb->cmd[0]);

#నిర్వచితమైతే BBB_COMDAT_TRACE
	రాయి("dir %d lun %d cmdlen %d cmd %p datalen %lu pdata %p\n",
		dir_లోపల, srb->lun, srb->cmdlen, srb->cmd, srb->datalen,
		srb->pdata);
	ఒకవేళ (srb->cmdlen) {
		కొరకు (ఫలితం = 0; ఫలితం < srb->cmdlen; ఫలితం++)
			రాయి("cmd[%d] %#x ", ఫలితం, srb->cmd[ఫలితం]);
		రాయి("\n");
	}
#అయితే_ముగింపు
	/* sanity checks */
	ఒకవేళ (!(srb->cmdlen <= CBWCDBLENGTH)) {
		debug("usb_stor_BBB_comdat:cmdlen too large\n");
		తిరిగి -1;
	}

	/* always OUT to the ep */
	pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_బహిర్గతం);

	cbw->dCBWSignature = cpu_గమ్యం_le32(CBWSIGNATURE);
	cbw->dCBWTag = cpu_గమ్యం_le32(CBWTag++);
	cbw->d_CBWData_Transfer_నిడివి = cpu_గమ్యం_le32(srb->datalen);
	cbw->bCBWFlags = (dir_లోపల ? CBWFLAGS_లోపల : CBWFLAGS_బహిర్గతం);
	cbw->bCBWLUN = srb->lun;
	cbw->bCDBLength = srb->cmdlen;
	/* copy the command data into the CBW command data buffer */
	/* DST SRC LEN!!! */

	జ్ఞాపకం_నకలు(cbw->CBWCDB, srb->cmd, srb->cmdlen);
	ఫలితం = usb_bulk_సందేశం(us->pusb_dev, pipe, cbw, UMASS_BBB_CBW_కొలత,
			      &actlen, USB_CNTL_TIMEOUT * 5);
	ఒకవేళ (ఫలితం < 0)
		debug("usb_stor_BBB_comdat:usb_bulk_msg error\n");
	తిరిగి ఫలితం;
}

/* FIXME: we also need a CBI_command which sets up the completion
 * interrupt, and waits for it
 */
స్థిర పూర్ణాంకం usb_stor_CB_comdat(నిర్మాణం scsi_cmd *srb, నిర్మాణం us_దత్తాంశం *us)
{
	పూర్ణాంకం ఫలితం = 0;
	పూర్ణాంకం dir_లోపల, retry;
	గుర్తులేని పూర్ణాంకం pipe;
	గుర్తులేని దీర్ఘ స్థితి_2;

	retry = 5;
	dir_లోపల = US_DIRECTION(srb->cmd[0]);

	ఒకవేళ (dir_లోపల)
		pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_లోపల);
	లేకపోతే
		pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_బహిర్గతం);

	అయితే (retry--) {
		debug("CBI gets a command: Try %d\n", 5 - retry);
#నిర్వచితమైతే DEBUG
		usb_show_srb(srb);
#అయితే_ముగింపు
		/* let's send the command via the control pipe */
		ఫలితం = usb_control_సందేశం(us->pusb_dev,
					 usb_sndctrlpipe(us->pusb_dev , 0),
					 US_CBI_ADSC,
					 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
					 0, us->ifnum,
					 srb->cmd, srb->cmdlen,
					 USB_CNTL_TIMEOUT * 5);
		debug("CB_transport: control msg returned %d, status %lX\n",
		      ఫలితం, us->pusb_dev->స్థితి_2);
		/* check the return code for the command */
		ఒకవేళ (ఫలితం < 0) {
			ఒకవేళ (us->pusb_dev->స్థితి_2 & USB_ST_STALLED) {
				స్థితి_2 = us->pusb_dev->స్థితి_2;
				debug(" stall during command found," \
				      " clear pipe\n");
				usb_clear_halt(us->pusb_dev,
					      usb_sndctrlpipe(us->pusb_dev, 0));
				us->pusb_dev->స్థితి_2 = స్థితి_2;
			}
			debug(" error during command %02X" \
			      " Stat = %lX\n", srb->cmd[0],
			      us->pusb_dev->స్థితి_2);
			తిరిగి ఫలితం;
		}
		/* transfer the data payload for this command, if one exists*/

		debug("CB_transport: control msg returned %d," \
		      " direction is %s to go 0x%lx\n", ఫలితం,
		      dir_లోపల ? "IN" : "OUT", srb->datalen);
		ఒకవేళ (srb->datalen) {
			ఫలితం = us_one_transfer(us, pipe, (అక్షరం *)srb->pdata,
						 srb->datalen);
			debug("CBI attempted to transfer data," \
			      " result is %d status %lX, len %d\n",
			      ఫలితం, us->pusb_dev->స్థితి_2,
				us->pusb_dev->act_len);
			ఒకవేళ (!(us->pusb_dev->స్థితి_2 & USB_ST_NAK_REC))
				ఆపు;
		} /* if (srb->datalen) */
		లేకపోతే
			ఆపు;
	}
	/* return result */

	తిరిగి ఫలితం;
}

స్థిర పూర్ణాంకం usb_stor_CBI_get_స్థితి(నిర్మాణం scsi_cmd *srb, నిర్మాణం us_దత్తాంశం *us)
{
	పూర్ణాంకం timeout;

	us->ip_wanted = 1;
	usb_int_సందేశం(us->pusb_dev, us->irqpipe,
		    (శూన్య *)&us->ip_దత్తాంశం, us->irqmaxp, us->irqinterval, false);
	timeout = 1000;
	అయితే (timeout--) {
		ఒకవేళ (us->ip_wanted == 0)
			ఆపు;
		mdelay(10);
	}
	ఒకవేళ (us->ip_wanted) {
		రాయి("	Did not get interrupt on CBI\n");
		us->ip_wanted = 0;
		తిరిగి USB_STOR_TRANSPORT_ERROR;
	}
	debug("Got interrupt data 0x%x, transferred %d status 0x%lX\n",
	      us->ip_దత్తాంశం, us->pusb_dev->irq_act_len,
	      us->pusb_dev->irq_స్థితి);
	/* UFI gives us ASC and ASCQ, like a request sense */
	ఒకవేళ (us->subclass == US_SC_UFI) {
		ఒకవేళ (srb->cmd[0] == SCSI_REQ_SENSE ||
		    srb->cmd[0] == SCSI_INQUIRY)
			తిరిగి USB_STOR_TRANSPORT_GOOD; /* Good */
		లేకపోతే ఒకవేళ (us->ip_దత్తాంశం)
			తిరిగి USB_STOR_TRANSPORT_FAILED;
		లేకపోతే
			తిరిగి USB_STOR_TRANSPORT_GOOD;
	}
	/* otherwise, we interpret the data normally */
	ఎంపిక (us->ip_దత్తాంశం) {
	స్థితి 0x0001:
		తిరిగి USB_STOR_TRANSPORT_GOOD;
	స్థితి 0x0002:
		తిరిగి USB_STOR_TRANSPORT_FAILED;
	అప్రమేయ:
		తిరిగి USB_STOR_TRANSPORT_ERROR;
	}			/* switch */
	తిరిగి USB_STOR_TRANSPORT_ERROR;
}

#నిర్వచించు USB_TRANSPORT_UNKNOWN_RETRY 5
#నిర్వచించు USB_TRANSPORT_NOT_READY_RETRY 10

/* clear a stall on an endpoint - special for BBB devices */
స్థిర పూర్ణాంకం usb_stor_BBB_clear_endpt_stall(నిర్మాణం us_దత్తాంశం *us, __u8 endpt)
{
	/* ENDPOINT_HALT = 0, so set value to 0 */
	తిరిగి usb_control_సందేశం(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
			       USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
			       endpt, ఖాళీ, 0, USB_CNTL_TIMEOUT * 5);
}

స్థిర పూర్ణాంకం usb_stor_BBB_transport(నిర్మాణం scsi_cmd *srb, నిర్మాణం us_దత్తాంశం *us)
{
	పూర్ణాంకం ఫలితం, retry;
	పూర్ణాంకం dir_లోపల;
	పూర్ణాంకం actlen, దత్తాంశం_actlen;
	గుర్తులేని పూర్ణాంకం pipe, pipein, pipeout;
	ALLOC_CACHE_ALIGN_బఫరు(నిర్మాణం umass_bbb_csw, csw, 1);
#నిర్వచితమైతే BBB_XPORT_TRACE
	గుర్తులేని అక్షరం *ptr;
	పూర్ణాంకం సూచీ;
#అయితే_ముగింపు

	dir_లోపల = US_DIRECTION(srb->cmd[0]);

	/* COMMAND phase */
	debug("COMMAND phase\n");
	ఫలితం = usb_stor_BBB_comdat(srb, us);
	ఒకవేళ (ఫలితం < 0) {
		debug("failed to send CBW status %ld\n",
		      us->pusb_dev->స్థితి_2);
		usb_stor_BBB_reset(us);
		తిరిగి USB_STOR_TRANSPORT_FAILED;
	}
	ఒకవేళ (!(us->flags & USB_READY))
		mdelay(5);
	pipein = usb_rcvbulkpipe(us->pusb_dev, us->ep_లోపల);
	pipeout = usb_sndbulkpipe(us->pusb_dev, us->ep_బహిర్గతం);
	/* DATA phase + error handling */
	దత్తాంశం_actlen = 0;
	/* no data, go immediately to the STATUS phase */
	ఒకవేళ (srb->datalen == 0)
		వెళ్ళు st;
	debug("DATA phase\n");
	ఒకవేళ (dir_లోపల)
		pipe = pipein;
	లేకపోతే
		pipe = pipeout;

	ఫలితం = usb_bulk_సందేశం(us->pusb_dev, pipe, srb->pdata, srb->datalen,
			      &దత్తాంశం_actlen, USB_CNTL_TIMEOUT * 5);
	/* special handling of STALL in DATA phase */
	ఒకవేళ ((ఫలితం < 0) && (us->pusb_dev->స్థితి_2 & USB_ST_STALLED)) {
		debug("DATA:stall\n");
		/* clear the STALL on the endpoint */
		ఫలితం = usb_stor_BBB_clear_endpt_stall(us,
					dir_లోపల ? us->ep_లోపల : us->ep_బహిర్గతం);
		ఒకవేళ (ఫలితం >= 0)
			/* continue on to STATUS phase */
			వెళ్ళు st;
	}
	ఒకవేళ (ఫలితం < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->స్థితి_2);
		usb_stor_BBB_reset(us);
		తిరిగి USB_STOR_TRANSPORT_FAILED;
	}
#నిర్వచితమైతే BBB_XPORT_TRACE
	కొరకు (సూచీ = 0; సూచీ < దత్తాంశం_actlen; సూచీ++)
		రాయి("pdata[%d] %#x ", సూచీ, srb->pdata[సూచీ]);
	రాయి("\n");
#అయితే_ముగింపు
	/* STATUS phase + error handling */
st:
	retry = 0;
again:
	debug("STATUS phase\n");
	ఫలితం = usb_bulk_సందేశం(us->pusb_dev, pipein, csw, UMASS_BBB_CSW_కొలత,
				&actlen, USB_CNTL_TIMEOUT*5);

	/* special handling of STALL in STATUS phase */
	ఒకవేళ ((ఫలితం < 0) && (retry < 1) &&
	    (us->pusb_dev->స్థితి_2 & USB_ST_STALLED)) {
		debug("STATUS:stall\n");
		/* clear the STALL on the endpoint */
		ఫలితం = usb_stor_BBB_clear_endpt_stall(us, us->ep_లోపల);
		ఒకవేళ (ఫలితం >= 0 && (retry++ < 1))
			/* do a retry */
			వెళ్ళు again;
	}
	ఒకవేళ (ఫలితం < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->స్థితి_2);
		usb_stor_BBB_reset(us);
		తిరిగి USB_STOR_TRANSPORT_FAILED;
	}
#నిర్వచితమైతే BBB_XPORT_TRACE
	ptr = (గుర్తులేని అక్షరం *)csw;
	కొరకు (సూచీ = 0; సూచీ < UMASS_BBB_CSW_కొలత; సూచీ++)
		రాయి("ptr[%d] %#x ", సూచీ, ptr[సూచీ]);
	రాయి("\n");
#అయితే_ముగింపు
	/* misuse pipe to get the residue */
	pipe = le32_గమ్యం_cpu(csw->dCSWDataResidue);
	ఒకవేళ (pipe == 0 && srb->datalen != 0 && srb->datalen - దత్తాంశం_actlen != 0)
		pipe = srb->datalen - దత్తాంశం_actlen;
	ఒకవేళ (CSWSIGNATURE != le32_గమ్యం_cpu(csw->dCSWSignature)) {
		debug("!CSWSIGNATURE\n");
		usb_stor_BBB_reset(us);
		తిరిగి USB_STOR_TRANSPORT_FAILED;
	} లేకపోతే ఒకవేళ ((CBWTag - 1) != le32_గమ్యం_cpu(csw->dCSWTag)) {
		debug("!Tag\n");
		usb_stor_BBB_reset(us);
		తిరిగి USB_STOR_TRANSPORT_FAILED;
	} లేకపోతే ఒకవేళ (csw->bCSWStatus > CSWSTATUS_PHASE) {
		debug(">PHASE\n");
		usb_stor_BBB_reset(us);
		తిరిగి USB_STOR_TRANSPORT_FAILED;
	} లేకపోతే ఒకవేళ (csw->bCSWStatus == CSWSTATUS_PHASE) {
		debug("=PHASE\n");
		usb_stor_BBB_reset(us);
		తిరిగి USB_STOR_TRANSPORT_FAILED;
	} లేకపోతే ఒకవేళ (దత్తాంశం_actlen > srb->datalen) {
		debug("transferred %dB instead of %ldB\n",
		      దత్తాంశం_actlen, srb->datalen);
		తిరిగి USB_STOR_TRANSPORT_FAILED;
	} లేకపోతే ఒకవేళ (csw->bCSWStatus == CSWSTATUS_FAILED) {
		debug("FAILED\n");
		తిరిగి USB_STOR_TRANSPORT_FAILED;
	}

	తిరిగి ఫలితం;
}

స్థిర పూర్ణాంకం usb_stor_CB_transport(నిర్మాణం scsi_cmd *srb, నిర్మాణం us_దత్తాంశం *us)
{
	పూర్ణాంకం ఫలితం, స్థితి_2;
	నిర్మాణం scsi_cmd *psrb;
	నిర్మాణం scsi_cmd reqsrb;
	పూర్ణాంకం retry, notready;

	psrb = &reqsrb;
	స్థితి_2 = USB_STOR_TRANSPORT_GOOD;
	retry = 0;
	notready = 0;
	/* issue the command */
do_retry:
	ఫలితం = usb_stor_CB_comdat(srb, us);
	debug("command / Data returned %d, status %lX\n",
	      ఫలితం, us->pusb_dev->స్థితి_2);
	/* if this is an CBI Protocol, get IRQ */
	ఒకవేళ (us->protocol == US_PR_CBI) {
		స్థితి_2 = usb_stor_CBI_get_స్థితి(srb, us);
		/* if the status is error, report it */
		ఒకవేళ (స్థితి_2 == USB_STOR_TRANSPORT_ERROR) {
			debug(" USB CBI Command Error\n");
			తిరిగి స్థితి_2;
		}
		srb->sense_buf[12] = (గుర్తులేని అక్షరం)(us->ip_దత్తాంశం >> 8);
		srb->sense_buf[13] = (గుర్తులేని అక్షరం)(us->ip_దత్తాంశం & 0xff);
		ఒకవేళ (!us->ip_దత్తాంశం) {
			/* if the status is good, report it */
			ఒకవేళ (స్థితి_2 == USB_STOR_TRANSPORT_GOOD) {
				debug(" USB CBI Command Good\n");
				తిరిగి స్థితి_2;
			}
		}
	}
	/* do we have to issue an auto request? */
	/* HERE we have to check the result */
	ఒకవేళ ((ఫలితం < 0) && !(us->pusb_dev->స్థితి_2 & USB_ST_STALLED)) {
		debug("ERROR %lX\n", us->pusb_dev->స్థితి_2);
		us->transport_reset(us);
		తిరిగి USB_STOR_TRANSPORT_ERROR;
	}
	ఒకవేళ ((us->protocol == US_PR_CBI) &&
	    ((srb->cmd[0] == SCSI_REQ_SENSE) ||
	    (srb->cmd[0] == SCSI_INQUIRY))) {
		/* do not issue an autorequest after request sense */
		debug("No auto request and good\n");
		తిరిగి USB_STOR_TRANSPORT_GOOD;
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
	ఫలితం = usb_stor_CB_comdat(psrb, us);
	debug("auto request returned %d\n", ఫలితం);
	/* if this is an CBI Protocol, get IRQ */
	ఒకవేళ (us->protocol == US_PR_CBI)
		స్థితి_2 = usb_stor_CBI_get_స్థితి(psrb, us);

	ఒకవేళ ((ఫలితం < 0) && !(us->pusb_dev->స్థితి_2 & USB_ST_STALLED)) {
		debug(" AUTO REQUEST ERROR %ld\n",
		      us->pusb_dev->స్థితి_2);
		తిరిగి USB_STOR_TRANSPORT_ERROR;
	}
	debug("autorequest returned 0x%02X 0x%02X 0x%02X 0x%02X\n",
	      srb->sense_buf[0], srb->sense_buf[2],
	      srb->sense_buf[12], srb->sense_buf[13]);
	/* Check the auto request result */
	ఒకవేళ ((srb->sense_buf[2] == 0) &&
	    (srb->sense_buf[12] == 0) &&
	    (srb->sense_buf[13] == 0)) {
		/* ok, no sense */
		తిరిగి USB_STOR_TRANSPORT_GOOD;
	}

	/* Check the auto request result */
	ఎంపిక (srb->sense_buf[2]) {
	స్థితి 0x01:
		/* Recovered Error */
		తిరిగి USB_STOR_TRANSPORT_GOOD;
		ఆపు;
	స్థితి 0x02:
		/* Not Ready */
		ఒకవేళ (notready++ > USB_TRANSPORT_NOT_READY_RETRY) {
			రాయి("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X (NOT READY)\n", srb->cmd[0],
				srb->sense_buf[0], srb->sense_buf[2],
				srb->sense_buf[12], srb->sense_buf[13]);
			తిరిగి USB_STOR_TRANSPORT_FAILED;
		} లేకపోతే {
			mdelay(100);
			వెళ్ళు do_retry;
		}
		ఆపు;
	అప్రమేయ:
		ఒకవేళ (retry++ > USB_TRANSPORT_UNKNOWN_RETRY) {
			రాయి("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X\n", srb->cmd[0], srb->sense_buf[0],
				srb->sense_buf[2], srb->sense_buf[12],
				srb->sense_buf[13]);
			తిరిగి USB_STOR_TRANSPORT_FAILED;
		} లేకపోతే
			వెళ్ళు do_retry;
		ఆపు;
	}
	తిరిగి USB_STOR_TRANSPORT_FAILED;
}

స్థిర శూన్య usb_stor_సమితి_max_xfer_blk(నిర్మాణం usb_device *udev,
				      నిర్మాణం us_దత్తాంశం *us)
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
	గుర్తులేని పొట్టి blk = 240;

#ఒకవేళ CONFIG_IS_ENABLED(DM_USB)
	size_t కొలత;
	పూర్ణాంకం ret;

	ret = usb_get_max_xfer_కొలత(udev, (size_t *)&కొలత);
	ఒకవేళ ((ret >= 0) && (కొలత < blk * 512))
		blk = కొలత / 512;
#అయితే_ముగింపు

	us->max_xfer_blk = blk;
}

స్థిర పూర్ణాంకం usb_inquiry(నిర్మాణం scsi_cmd *srb, నిర్మాణం us_దత్తాంశం *ss)
{
	పూర్ణాంకం retry, i;
	retry = 5;
	చేయి {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_INQUIRY;
		srb->cmd[1] = srb->lun << 5;
		srb->cmd[4] = 36;
		srb->datalen = 36;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		i = ss->transport(srb, ss);
		debug("inquiry returns %d\n", i);
		ఒకవేళ (i == 0)
			ఆపు;
	} అయితే (--retry);

	ఒకవేళ (!retry) {
		రాయి("error in inquiry\n");
		తిరిగి -1;
	}
	తిరిగి 0;
}

స్థిర పూర్ణాంకం usb_request_sense(నిర్మాణం scsi_cmd *srb, నిర్మాణం us_దత్తాంశం *ss)
{
	అక్షరం *ptr;

	ptr = (అక్షరం *)srb->pdata;
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
	తిరిగి 0;
}

స్థిర పూర్ణాంకం usb_test_unit_ready(నిర్మాణం scsi_cmd *srb, నిర్మాణం us_దత్తాంశం *ss)
{
	పూర్ణాంకం retries = 10;

	చేయి {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_TST_U_RDY;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 0;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		ఒకవేళ (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD) {
			ss->flags |= USB_READY;
			తిరిగి 0;
		}
		usb_request_sense(srb, ss);
		/*
		 * Check the Key Code Qualifier, if it matches
		 * "Not Ready - medium not present"
		 * (the sense Key equals 0x2 and the ASC is 0x3a)
		 * return immediately as the medium being absent won't change
		 * unless there is a user action.
		 */
		ఒకవేళ ((srb->sense_buf[2] == 0x02) &&
		    (srb->sense_buf[12] == 0x3a))
			తిరిగి -1;
		mdelay(100);
	} అయితే (retries--);

	తిరిగి -1;
}

స్థిర పూర్ణాంకం usb_read_సామర్థ్యం(నిర్మాణం scsi_cmd *srb, నిర్మాణం us_దత్తాంశం *ss)
{
	పూర్ణాంకం retry;
	/* XXX retries */
	retry = 3;
	చేయి {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_RD_CAPAC;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 8;
		srb->cmdlen = ss->cmd12 ? 12 : 10;
		ఒకవేళ (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD)
			తిరిగి 0;
	} అయితే (retry--);

	తిరిగి -1;
}

స్థిర పూర్ణాంకం usb_read_10(నిర్మాణం scsi_cmd *srb, నిర్మాణం us_దత్తాంశం *ss,
		       గుర్తులేని దీర్ఘ ప్రారంభం, గుర్తులేని పొట్టి ఖండాలు)
{
	memset(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_READ10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((గుర్తులేని అక్షరం) (ప్రారంభం >> 24)) & 0xff;
	srb->cmd[3] = ((గుర్తులేని అక్షరం) (ప్రారంభం >> 16)) & 0xff;
	srb->cmd[4] = ((గుర్తులేని అక్షరం) (ప్రారంభం >> 8)) & 0xff;
	srb->cmd[5] = ((గుర్తులేని అక్షరం) (ప్రారంభం)) & 0xff;
	srb->cmd[7] = ((గుర్తులేని అక్షరం) (ఖండాలు >> 8)) & 0xff;
	srb->cmd[8] = (గుర్తులేని అక్షరం) ఖండాలు & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("read10: start %lx blocks %x\n", ప్రారంభం, ఖండాలు);
	తిరిగి ss->transport(srb, ss);
}

స్థిర పూర్ణాంకం usb_write_10(నిర్మాణం scsi_cmd *srb, నిర్మాణం us_దత్తాంశం *ss,
			గుర్తులేని దీర్ఘ ప్రారంభం, గుర్తులేని పొట్టి ఖండాలు)
{
	memset(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_WRITE10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((గుర్తులేని అక్షరం) (ప్రారంభం >> 24)) & 0xff;
	srb->cmd[3] = ((గుర్తులేని అక్షరం) (ప్రారంభం >> 16)) & 0xff;
	srb->cmd[4] = ((గుర్తులేని అక్షరం) (ప్రారంభం >> 8)) & 0xff;
	srb->cmd[5] = ((గుర్తులేని అక్షరం) (ప్రారంభం)) & 0xff;
	srb->cmd[7] = ((గుర్తులేని అక్షరం) (ఖండాలు >> 8)) & 0xff;
	srb->cmd[8] = (గుర్తులేని అక్షరం) ఖండాలు & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("write10: start %lx blocks %x\n", ప్రారంభం, ఖండాలు);
	తిరిగి ss->transport(srb, ss);
}

#నిర్వచితమైతే CONFIG_USB_BIN_FIXUP
/*
 * Some USB storage devices queried for SCSI identification data respond with
 * binary strings, which if output to the console freeze the terminal. The
 * workaround is to modify the vendor and product strings read from such
 * device with proper values (as reported by 'usb info').
 *
 * Vendor and product length limits are taken from the definition of
 * struct blk_desc in include/part.h.
 */
స్థిర శూన్య usb_bin_fixup(నిర్మాణం usb_device_descriptor descriptor,
				గుర్తులేని అక్షరం vendor[],
				గుర్తులేని అక్షరం లబ్ధం[]) {
	స్థిరరాశి గుర్తులేని అక్షరం max_vendor_len = 40;
	స్థిరరాశి గుర్తులేని అక్షరం max_లబ్ధం_len = 20;
	ఒకవేళ (descriptor.idVendor == 0x0424 && descriptor.id_లబ్ధం_2 == 0x223a) {
		strncpy((అక్షరం *)vendor, "SMSC", max_vendor_len);
		strncpy((అక్షరం *)లబ్ధం, "Flash Media Cntrller",
			max_లబ్ధం_len);
	}
}
#అయితే_ముగింపు /* CONFIG_USB_BIN_FIXUP */

#ఒకవేళ CONFIG_IS_ENABLED(BLK)
స్థిర గుర్తులేని దీర్ఘ usb_stor_read(నిర్మాణం udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, శూన్య *బఫరు)
#లేకపోతే
స్థిర గుర్తులేని దీర్ఘ usb_stor_read(నిర్మాణం blk_desc *ఖండం_dev, lbaint_t blknr,
				   lbaint_t blkcnt, శూన్య *బఫరు)
#అయితే_ముగింపు
{
	lbaint_t ప్రారంభం, blks;
	uintptr_t buf_addr;
	గుర్తులేని పొట్టి smallblks;
	నిర్మాణం usb_device *udev;
	నిర్మాణం us_దత్తాంశం *ss;
	పూర్ణాంకం retry;
	నిర్మాణం scsi_cmd *srb = &usb_ccb;
#ఒకవేళ CONFIG_IS_ENABLED(BLK)
	నిర్మాణం blk_desc *ఖండం_dev;
#అయితే_ముగింపు

	ఒకవేళ (blkcnt == 0)
		తిరిగి 0;
	/* Setup  device */
#ఒకవేళ CONFIG_IS_ENABLED(BLK)
	ఖండం_dev = dev_get_uclass_plat(dev);
	udev = dev_get_మాతృ_priv(dev_get_మాతృ(dev));
	debug("\nusb_read: udev %d\n", ఖండం_dev->devnum);
#లేకపోతే
	debug("\nusb_read: udev %d\n", ఖండం_dev->devnum);
	udev = usb_dev_desc[ఖండం_dev->devnum].priv;
	ఒకవేళ (!udev) {
		debug("%s: No device\n", __func__);
		తిరిగి 0;
	}
#అయితే_ముగింపు
	ss = (నిర్మాణం us_దత్తాంశం *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_తాళం_async(udev, 1);
	srb->lun = ఖండం_dev->lun;
	buf_addr = (uintptr_t)బఫరు;
	ప్రారంభం = blknr;
	blks = blkcnt;

	debug("\nusb_read: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      ఖండం_dev->devnum, ప్రారంభం, blks, buf_addr);

	చేయి {
		/* XXX need some comment here */
		retry = 2;
		srb->pdata = (గుర్తులేని అక్షరం *)buf_addr;
		ఒకవేళ (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		లేకపోతే
			smallblks = (గుర్తులేని పొట్టి) blks;
retry_it:
		ఒకవేళ (smallblks == ss->max_xfer_blk)
			usb_show_పురోగతి();
		srb->datalen = ఖండం_dev->blksz * smallblks;
		srb->pdata = (గుర్తులేని అక్షరం *)buf_addr;
		ఒకవేళ (usb_read_10(srb, ss, ప్రారంభం, smallblks)) {
			debug("Read ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			ఒకవేళ (retry--)
				వెళ్ళు retry_it;
			blkcnt -= blks;
			ఆపు;
		}
		ప్రారంభం += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} అయితే (blks != 0);

	debug("usb_read: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      ప్రారంభం, smallblks, buf_addr);

	usb_తాళం_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	ఒకవేళ (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	తిరిగి blkcnt;
}

#ఒకవేళ CONFIG_IS_ENABLED(BLK)
స్థిర గుర్తులేని దీర్ఘ usb_stor_write(నిర్మాణం udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, స్థిరరాశి శూన్య *బఫరు)
#లేకపోతే
స్థిర గుర్తులేని దీర్ఘ usb_stor_write(నిర్మాణం blk_desc *ఖండం_dev, lbaint_t blknr,
				    lbaint_t blkcnt, స్థిరరాశి శూన్య *బఫరు)
#అయితే_ముగింపు
{
	lbaint_t ప్రారంభం, blks;
	uintptr_t buf_addr;
	గుర్తులేని పొట్టి smallblks;
	నిర్మాణం usb_device *udev;
	నిర్మాణం us_దత్తాంశం *ss;
	పూర్ణాంకం retry;
	నిర్మాణం scsi_cmd *srb = &usb_ccb;
#ఒకవేళ CONFIG_IS_ENABLED(BLK)
	నిర్మాణం blk_desc *ఖండం_dev;
#అయితే_ముగింపు

	ఒకవేళ (blkcnt == 0)
		తిరిగి 0;

	/* Setup  device */
#ఒకవేళ CONFIG_IS_ENABLED(BLK)
	ఖండం_dev = dev_get_uclass_plat(dev);
	udev = dev_get_మాతృ_priv(dev_get_మాతృ(dev));
	debug("\nusb_read: udev %d\n", ఖండం_dev->devnum);
#లేకపోతే
	debug("\nusb_read: udev %d\n", ఖండం_dev->devnum);
	udev = usb_dev_desc[ఖండం_dev->devnum].priv;
	ఒకవేళ (!udev) {
		debug("%s: No device\n", __func__);
		తిరిగి 0;
	}
#అయితే_ముగింపు
	ss = (నిర్మాణం us_దత్తాంశం *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_తాళం_async(udev, 1);

	srb->lun = ఖండం_dev->lun;
	buf_addr = (uintptr_t)బఫరు;
	ప్రారంభం = blknr;
	blks = blkcnt;

	debug("\nusb_write: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      ఖండం_dev->devnum, ప్రారంభం, blks, buf_addr);

	చేయి {
		/* If write fails retry for max retry count else
		 * return with number of blocks written successfully.
		 */
		retry = 2;
		srb->pdata = (గుర్తులేని అక్షరం *)buf_addr;
		ఒకవేళ (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		లేకపోతే
			smallblks = (గుర్తులేని పొట్టి) blks;
retry_it:
		ఒకవేళ (smallblks == ss->max_xfer_blk)
			usb_show_పురోగతి();
		srb->datalen = ఖండం_dev->blksz * smallblks;
		srb->pdata = (గుర్తులేని అక్షరం *)buf_addr;
		ఒకవేళ (usb_write_10(srb, ss, ప్రారంభం, smallblks)) {
			debug("Write ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			ఒకవేళ (retry--)
				వెళ్ళు retry_it;
			blkcnt -= blks;
			ఆపు;
		}
		ప్రారంభం += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} అయితే (blks != 0);

	debug("usb_write: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      ప్రారంభం, smallblks, buf_addr);

	usb_తాళం_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	ఒకవేళ (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	తిరిగి blkcnt;

}

/* Probe to see if a new device is actually a Storage device */
పూర్ణాంకం usb_storage_probe(నిర్మాణం usb_device *dev, గుర్తులేని పూర్ణాంకం ifnum,
		      నిర్మాణం us_దత్తాంశం *ss)
{
	నిర్మాణం usb_interface *iface;
	పూర్ణాంకం i;
	నిర్మాణం usb_endpoint_descriptor *ep_desc;
	గుర్తులేని పూర్ణాంకం flags = 0;

	/* let's examine the device now */
	iface = &dev->config.if_desc[ifnum];

	ఒకవేళ (dev->descriptor.bDeviceClass != 0 ||
			iface->desc.bInterfaceClass != USB_CLASS_MASS_STORAGE ||
			iface->desc.bInterfaceSubClass < US_SC_MIN ||
			iface->desc.bInterfaceSubClass > US_SC_MAX) {
		debug("Not mass storage\n");
		/* if it's not a mass storage, we go no further */
		తిరిగి 0;
	}

	memset(ss, 0, పరిమాణం(నిర్మాణం us_దత్తాంశం));

	/* At this point, we know we've got a live one */
	debug("\n\nUSB Mass Storage device detected\n");

	/* Initialize the us_data structure with some useful info */
	ss->flags = flags;
	ss->ifnum = ifnum;
	ss->pusb_dev = dev;
	ss->attention_పూర్తి = 0;
	ss->subclass = iface->desc.bInterfaceSubClass;
	ss->protocol = iface->desc.bInterfaceProtocol;

	/* set the handler pointers based on the protocol */
	debug("Transport: ");
	ఎంపిక (ss->protocol) {
	స్థితి US_PR_CB:
		debug("Control/Bulk\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		ఆపు;

	స్థితి US_PR_CBI:
		debug("Control/Bulk/Interrupt\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		ఆపు;
	స్థితి US_PR_BULK:
		debug("Bulk/Bulk/Bulk\n");
		ss->transport = usb_stor_BBB_transport;
		ss->transport_reset = usb_stor_BBB_reset;
		ఆపు;
	అప్రమేయ:
		రాయి("USB Storage Transport unknown / not yet implemented\n");
		తిరిగి 0;
		ఆపు;
	}

	/*
	 * We are expecting a minimum of 2 endpoints - in and out (bulk).
	 * An optional interrupt is OK (necessary for CBI protocol).
	 * We will ignore any others.
	 */
	కొరకు (i = 0; i < iface->desc.b_అంకె_Endpoints; i++) {
		ep_desc = &iface->ep_desc[i];
		/* is it an BULK endpoint? */
		ఒకవేళ ((ep_desc->bm_లక్షణాలు &
		     USB_ENDPOINT_XFERTYPE_ముసుగు) == USB_ENDPOINT_XFER_BULK) {
			ఒకవేళ (ep_desc->b_Endpoint_చిరునామా & USB_DIR_లోపల)
				ss->ep_లోపల = ep_desc->b_Endpoint_చిరునామా &
						USB_ENDPOINT_సంఖ్య_ముసుగు;
			లేకపోతే
				ss->ep_బహిర్గతం =
					ep_desc->b_Endpoint_చిరునామా &
					USB_ENDPOINT_సంఖ్య_ముసుగు;
		}

		/* is it an interrupt endpoint? */
		ఒకవేళ ((ep_desc->bm_లక్షణాలు &
		     USB_ENDPOINT_XFERTYPE_ముసుగు) == USB_ENDPOINT_XFER_INT) {
			ss->ep_int = ep_desc->b_Endpoint_చిరునామా &
						USB_ENDPOINT_సంఖ్య_ముసుగు;
			ss->irqinterval = ep_desc->bInterval;
		}
	}
	debug("Endpoints In %d Out %d Int %d\n",
	      ss->ep_లోపల, ss->ep_బహిర్గతం, ss->ep_int);

	/* Do some basic sanity checks, and bail if we find a problem */
	ఒకవేళ (usb_సమితి_interface(dev, iface->desc.b_Interface_సంఖ్య, 0) ||
	    !ss->ep_లోపల || !ss->ep_బహిర్గతం ||
	    (ss->protocol == US_PR_CBI && ss->ep_int == 0)) {
		debug("Problems with device\n");
		తిరిగి 0;
	}
	/* set class specific stuff */
	/* We only handle certain protocols.  Currently, these are
	 * the only ones.
	 * The SFF8070 accepts the requests used in u-boot
	 */
	ఒకవేళ (ss->subclass != US_SC_UFI && ss->subclass != US_SC_SCSI &&
	    ss->subclass != US_SC_8070) {
		రాయి("Sorry, protocol %d not yet supported.\n", ss->subclass);
		తిరిగి 0;
	}

	/* UFI uses 12-byte commands (like RBC, unlike SCSI) */
	ఒకవేళ (ss->subclass == US_SC_UFI)
		ss->cmd12 = true;

	ఒకవేళ (ss->ep_int) {
		/* we had found an interrupt endpoint, prepare irq pipe
		 * set up the IRQ pipe and handler
		 */
		ss->irqinterval = (ss->irqinterval > 0) ? ss->irqinterval : 255;
		ss->irqpipe = usb_rcvintpipe(ss->pusb_dev, ss->ep_int);
		ss->irqmaxp = usb_maxpacket(dev, ss->irqpipe);
		dev->irq_handle = usb_stor_irq;
	}

	/* Set the maximum transfer size per host controller setting */
	usb_stor_సమితి_max_xfer_blk(dev, ss);

	dev->privptr = (శూన్య *)ss;
	తిరిగి 1;
}

పూర్ణాంకం usb_stor_get_info(నిర్మాణం usb_device *dev, నిర్మాణం us_దత్తాంశం *ss,
		      నిర్మాణం blk_desc *dev_desc)
{
	గుర్తులేని అక్షరం perq, modi;
	ALLOC_CACHE_ALIGN_బఫరు(u32, cap, 2);
	ALLOC_CACHE_ALIGN_బఫరు(u8, usb_stor_buf, 36);
	u32 సామర్థ్యం, blksz;
	నిర్మాణం scsi_cmd *pccb = &usb_ccb;

	pccb->pdata = usb_stor_buf;

	dev_desc->లక్ష్యం = dev->devnum;
	pccb->lun = dev_desc->lun;
	debug(" address %d\n", dev_desc->లక్ష్యం);

	ఒకవేళ (usb_inquiry(pccb, ss)) {
		debug("%s: usb_inquiry() failed\n", __func__);
		తిరిగి -1;
	}

	perq = usb_stor_buf[0];
	modi = usb_stor_buf[1];

	/*
	 * Skip unknown devices (0x1f) and enclosure service devices (0x0d),
	 * they would not respond to test_unit_ready .
	 */
	ఒకవేళ (((perq & 0x1f) == 0x1f) || ((perq & 0x1f) == 0x0d)) {
		debug("%s: unknown/unsupported device\n", __func__);
		తిరిగి 0;
	}
	ఒకవేళ ((modi&0x80) == 0x80) {
		/* drive is removable */
		dev_desc->removable = 1;
	}
	జ్ఞాపకం_నకలు(dev_desc->vendor, (స్థిరరాశి శూన్య *)&usb_stor_buf[8], 8);
	జ్ఞాపకం_నకలు(dev_desc->లబ్ధం, (స్థిరరాశి శూన్య *)&usb_stor_buf[16], 16);
	జ్ఞాపకం_నకలు(dev_desc->revision, (స్థిరరాశి శూన్య *)&usb_stor_buf[32], 4);
	dev_desc->vendor[8] = 0;
	dev_desc->లబ్ధం[16] = 0;
	dev_desc->revision[4] = 0;
#నిర్వచితమైతే CONFIG_USB_BIN_FIXUP
	usb_bin_fixup(dev->descriptor, (uchar *)dev_desc->vendor,
		      (uchar *)dev_desc->లబ్ధం);
#అయితే_ముగింపు /* CONFIG_USB_BIN_FIXUP */
	debug("ISO Vers %X, Response Data %X\n", usb_stor_buf[2],
	      usb_stor_buf[3]);
	ఒకవేళ (usb_test_unit_ready(pccb, ss)) {
		రాయి("Device NOT ready\n"
		       "   Request Sense returned %02X %02X %02X\n",
		       pccb->sense_buf[2], pccb->sense_buf[12],
		       pccb->sense_buf[13]);
		ఒకవేళ (dev_desc->removable == 1)
			dev_desc->type = perq;
		తిరిగి 0;
	}
	pccb->pdata = (గుర్తులేని అక్షరం *)cap;
	memset(pccb->pdata, 0, 8);
	ఒకవేళ (usb_read_సామర్థ్యం(pccb, ss) != 0) {
		రాయి("READ_CAP ERROR\n");
		ss->flags &= ~USB_READY;
		cap[0] = 2880;
		cap[1] = 0x200;
	}
	debug("Read Capacity returns: 0x%08x, 0x%08x\n", cap[0], cap[1]);
#ఒకవేళ 0
	ఒకవేళ (cap[0] > (0x200000 * 10)) /* greater than 10 GByte */
		cap[0] >>= 16;

	cap[0] = cpu_గమ్యం_be32(cap[0]);
	cap[1] = cpu_గమ్యం_be32(cap[1]);
#అయితే_ముగింపు

	సామర్థ్యం = be32_గమ్యం_cpu(cap[0]) + 1;
	blksz = be32_గమ్యం_cpu(cap[1]);

	debug("Capacity = 0x%08x, blocksz = 0x%08x\n", సామర్థ్యం, blksz);
	dev_desc->lba = సామర్థ్యం;
	dev_desc->blksz = blksz;
	dev_desc->log2blksz = LOG2(dev_desc->blksz);
	dev_desc->type = perq;
	debug(" address %d\n", dev_desc->లక్ష్యం);

	తిరిగి 1;
}

#ఒకవేళ CONFIG_IS_ENABLED(DM_USB)

స్థిర పూర్ణాంకం usb_mass_storage_probe(నిర్మాణం udevice *dev)
{
	నిర్మాణం usb_device *udev = dev_get_మాతృ_priv(dev);
	పూర్ణాంకం ret;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	ret = usb_stor_probe_device(udev);
	usb_disable_asynch(0); /* asynch transfer allowed */

	తిరిగి ret;
}

స్థిర స్థిరరాశి నిర్మాణం udevice_id usb_mass_storage_ids[] = {
	{ .compatible = "usb-mass-storage" },
	{ }
};

U_BOOT_DRIVER(usb_mass_storage) = {
	.name	= "usb_mass_storage",
	.id	= UCLASS_MASS_STORAGE,
	.యొక్క_సరిపోలిక = usb_mass_storage_ids,
	.probe = usb_mass_storage_probe,
#ఒకవేళ CONFIG_IS_ENABLED(BLK)
	.plat_auto	= పరిమాణం(నిర్మాణం us_దత్తాంశం),
#అయితే_ముగింపు
};

UCLASS_DRIVER(usb_mass_storage) = {
	.id		= UCLASS_MASS_STORAGE,
	.name		= "usb_mass_storage",
};

స్థిర స్థిరరాశి నిర్మాణం usb_device_id mass_storage_id_పట్టిక[] = {
	{
		.సరిపోలిక_flags = USB_DEVICE_ID_సరిపోలిక_INT_CLASS,
		.bInterfaceClass = USB_CLASS_MASS_STORAGE
	},
	{ }		/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_mass_storage, mass_storage_id_పట్టిక);
#అయితే_ముగింపు

#ఒకవేళ CONFIG_IS_ENABLED(BLK)
స్థిర స్థిరరాశి నిర్మాణం blk_ops usb_storage_ops = {
	.పఠ	= usb_stor_read,
	.లిఖ	= usb_stor_write,
};

U_BOOT_DRIVER(usb_storage_blk) = {
	.name		= "usb_storage_blk",
	.id		= UCLASS_BLK,
	.ops		= &usb_storage_ops,
};
#లేకపోతే
U_BOOT_LEGACY_BLK(usb) = {
	.uclass_idname	= "usb",
	.uclass_id	= UCLASS_USB,
	.max_devs	= USB_MAX_STOR_DEV,
	.desc		= usb_dev_desc,
};
#అయితే_ముగింపు
