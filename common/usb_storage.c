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

#شامل <blk.h>
#شامل <bootdev.h>
#شامل <command.h>
#شامل <dm.h>
#شامل <errno.h>
#شامل <log.h>
#شامل <mapmem.h>
#شامل <memalign.h>
#شامل <asm/byteorder.h>
#شامل <asm/cache.h>
#شامل <asm/processor.h>
#شامل <dm/device-internal.h>
#شامل <dm/lists.h>
#شامل <linux/delay.h>

#شامل <part.h>
#شامل <usb.h>

#تعریف_منسوخ BBB_COMDAT_TRACE
#تعریف_منسوخ BBB_XPORT_TRACE

#شامل <scsi.h>
/* direction table -- this indicates the direction of the data
 * transfer for each command code -- a 1 indicates input
 */
ساکن ثابت غیرعلامتی حرف us_direction[256/8] = {
	0x28, 0x81, 0x14, 0x14, 0x20, 0x01, 0x90, 0x77,
	0x0C, 0x20, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#تعریف_کرو US_DIRECTION(x) ((us_direction[x>>3] >> (x & 7)) & 1)

ساکن ساخت scsi_cmd usb_ccb __aligned(ARCH_DMA_MINALIGN);
ساکن __u32 CBWTag;

ساکن صحیح usb_max_devs; /* number of highest available usb device */

#اگر !CONFIG_IS_ENABLED(BLK)
ساکن ساخت blk_desc usb_dev_desc[USB_MAX_STOR_DEV];
#اگر_ختم

ساخت us_ڈیٹا;
قسمتعریف صحیح (*trans_cmnd)(ساخت scsi_cmd *cb, ساخت us_ڈیٹا *ڈیٹا);
قسمتعریف صحیح (*trans_reset)(ساخت us_ڈیٹا *ڈیٹا);

ساخت us_ڈیٹا {
	ساخت usb_device *pusb_dev;	 /* this usb_device */

	غیرعلامتی صحیح	flags;			/* from filter initially */
#	define USB_READY	(1 << 0)
	غیرعلامتی حرف	ifnum;			/* interface number */
	غیرعلامتی حرف	ep_اندر;			/* in endpoint */
	غیرعلامتی حرف	ep_برآمد;			/* out ....... */
	غیرعلامتی حرف	ep_int;			/* interrupt . */
	غیرعلامتی حرف	subclass;		/* as in overview */
	غیرعلامتی حرف	protocol;		/* .............. */
	غیرعلامتی حرف	attention_مکمل;		/* force attn on first cmd */
	غیرعلامتی مختصر	ip_ڈیٹا;		/* interrupt data */
	صحیح		action;			/* what to do */
	صحیح		ip_wanted;		/* needed */
	صحیح		*irq_handle;		/* for USB int requests */
	غیرعلامتی صحیح	irqpipe;		/* pipe for release_irq */
	غیرعلامتی حرف	irqmaxp;		/* max packed for irq Pipe */
	غیرعلامتی حرف	irqinterval;		/* Intervall for IRQ Pipe */
	ساخت scsi_cmd	*srb;			/* current srb */
	trans_reset	transport_reset;	/* reset routine */
	trans_cmnd	transport;		/* transport routine */
	غیرعلامتی مختصر	max_xfer_blk;		/* maximum transfer blocks */
	bool		cmd12;			/* use 12-byte commands (RBC/UFI) */
};

#اگر !CONFIG_IS_ENABLED(BLK)
ساکن ساخت us_ڈیٹا usb_stor[USB_MAX_STOR_DEV];
#اگر_ختم

#تعریف_کرو USB_STOR_TRANSPORT_GOOD	   0
#تعریف_کرو USB_STOR_TRANSPORT_FAILED -1
#تعریف_کرو USB_STOR_TRANSPORT_ERROR  -2

صحیح usb_stor_get_info(ساخت usb_device *dev, ساخت us_ڈیٹا *us,
		      ساخت blk_desc *dev_desc);
صحیح usb_storage_probe(ساخت usb_device *dev, غیرعلامتی صحیح ifnum,
		      ساخت us_ڈیٹا *ss);
#اگر CONFIG_IS_ENABLED(BLK)
ساکن غیرعلامتی طویل usb_stor_read(ساخت udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, خالی *بفر);
ساکن غیرعلامتی طویل usb_stor_write(ساخت udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, ثابت خالی *بفر);
#ورنہ
ساکن غیرعلامتی طویل usb_stor_read(ساخت blk_desc *بلاک_dev, lbaint_t blknr,
				   lbaint_t blkcnt, خالی *بفر);
ساکن غیرعلامتی طویل usb_stor_write(ساخت blk_desc *بلاک_dev, lbaint_t blknr,
				    lbaint_t blkcnt, ثابت خالی *بفر);
#اگر_ختم
خالی uhci_show_عارضی_int_td(خالی);

ساکن خالی usb_show_پیش_رفت(خالی)
{
	debug(".");
}

/*******************************************************************************
 * show info on storage devices; 'usb start/init' must be invoked earlier
 * as we only retrieve structures populated during devices initialization
 */
صحیح usb_stor_info(خالی)
{
	صحیح گنتی = 0;
#اگر CONFIG_IS_ENABLED(BLK)
	ساخت udevice *dev;

	برائے (blk_پہلا_device(UCLASS_USB, &dev);
	     dev;
	     blk_اگلا_device(&dev)) {
		ساخت blk_desc *desc = dev_get_uclass_plat(dev);

		لکھو("  Device %d: ", desc->devnum);
		dev_print(desc);
		گنتی++;
	}
#ورنہ
	صحیح i;

	اگر (usb_max_devs > 0) {
		برائے (i = 0; i < usb_max_devs; i++) {
			لکھو("  Device %d: ", i);
			dev_print(&usb_dev_desc[i]);
		}
		واپس 0;
	}
#اگر_ختم
	اگر (!گنتی) {
		لکھو("No storage devices, perhaps not 'usb start'ed..?\n");
		واپس 1;
	}

	واپس 0;
}

ساکن غیرعلامتی صحیح usb_get_max_lun(ساخت us_ڈیٹا *us)
{
	صحیح len;
	ALLOC_CACHE_ALIGN_بفر(غیرعلامتی حرف, نتیجہ, 1);
	len = usb_control_پیغام(us->pusb_dev,
			      usb_rcvctrlpipe(us->pusb_dev, 0),
			      US_BBB_GET_MAX_LUN,
			      USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_اندر,
			      0, us->ifnum,
			      نتیجہ, حجم(حرف),
			      USB_CNTL_TIMEOUT * 5);
	debug("Get Max LUN -> len = %i, result = %i\n", len, (صحیح) *نتیجہ);
	واپس (len > 0) ? *نتیجہ : 0;
}

ساکن صحیح usb_stor_probe_device(ساخت usb_device *udev)
{
	صحیح lun, max_lun;

#اگر CONFIG_IS_ENABLED(BLK)
	ساخت us_ڈیٹا *ڈیٹا;
	صحیح ret;
#ورنہ
	صحیح آغاز;

	اگر (udev == تہی)
		واپس -ENOENT; /* no more devices available */
#اگر_ختم

	debug("\n\nProbing for storage\n");
#اگر CONFIG_IS_ENABLED(BLK)
	/*
	 * We store the us_data in the mass storage device's plat. It
	 * is shared by all LUNs (block devices) attached to this mass storage
	 * device.
	 */
	ڈیٹا = dev_get_plat(udev->dev);
	اگر (!usb_storage_probe(udev, 0, ڈیٹا))
		واپس 0;
	max_lun = usb_get_max_lun(ڈیٹا);
	برائے (lun = 0; lun <= max_lun; lun++) {
		ساخت blk_desc *blkdev;
		ساخت udevice *dev;
		حرف str[10];

		snprintf(str, حجم(str), "lun%d", lun);
		ret = blk_create_devicef(udev->dev, "usb_storage_blk", str,
					 UCLASS_USB, usb_max_devs,
					 DEFAULT_BLKSZ, 0, &dev);
		اگر (ret) {
			debug("Cannot bind driver\n");
			واپس ret;
		}

		blkdev = dev_get_uclass_plat(dev);
		blkdev->ہدف = 0xff;
		blkdev->lun = lun;

		ret = usb_stor_get_info(udev, ڈیٹا, blkdev);
		اگر (ret == 1) {
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		} ورنہ {
			debug("usb_stor_get_info: Invalid device\n");
			ret = device_unbind(dev);
			اگر (ret)
				واپس ret;
			جاری;
		}

		ret = blk_probe_or_unbind(dev);
		اگر (ret)
			واپس ret;

		ret = bootdev_تیاری_for_sibling_blk(dev, "usb_bootdev");
		اگر (ret) {
			صحیح ret2;

			ret2 = device_unbind(dev);
			اگر (ret2)
				واپس log_پیغام_ret("bootdev", ret2);
			واپس log_پیغام_ret("bootdev", ret);
		}
	}
#ورنہ
	/* We don't have space to even probe if we hit the maximum */
	اگر (usb_max_devs == USB_MAX_STOR_DEV) {
		لکھو("max USB Storage Device reached: %d stopping\n",
		       usb_max_devs);
		واپس -ENOSPC;
	}

	اگر (!usb_storage_probe(udev, 0, &usb_stor[usb_max_devs]))
		واپس 0;

	/*
	 * OK, it's a storage device.  Iterate over its LUNs and populate
	 * usb_dev_desc'
	 */
	آغاز = usb_max_devs;

	max_lun = usb_get_max_lun(&usb_stor[usb_max_devs]);
	برائے (lun = 0; lun <= max_lun && usb_max_devs < USB_MAX_STOR_DEV;
	     lun++) {
		ساخت blk_desc *blkdev;

		blkdev = &usb_dev_desc[usb_max_devs];
		memset(blkdev, '\0', حجم(ساخت blk_desc));
		blkdev->uclass_id = UCLASS_USB;
		blkdev->devnum = usb_max_devs;
		blkdev->part_type = PART_TYPE_UNKNOWN;
		blkdev->ہدف = 0xff;
		blkdev->type = DEV_TYPE_UNKNOWN;
		blkdev->بلاک_read = usb_stor_read;
		blkdev->بلاک_write = usb_stor_write;
		blkdev->lun = lun;
		blkdev->priv = udev;

		اگر (usb_stor_get_info(udev, &usb_stor[آغاز],
				      &usb_dev_desc[usb_max_devs]) == 1) {
			debug("partype: %d\n", blkdev->part_type);
			part_init(blkdev);
			debug("partype: %d\n", blkdev->part_type);
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		}
	}
#اگر_ختم

	واپس 0;
}

خالی usb_stor_reset(خالی)
{
	usb_max_devs = 0;
}

/*******************************************************************************
 * scan the usb and reports device info
 * to the user if mode = 1
 * returns current device or -1 if no
 */
صحیح usb_stor_جائزہ(صحیح منوال)
{
	اگر (منوال == 1)
		لکھو("       scanning usb for storage devices... ");

#اگر !CONFIG_IS_ENABLED(DM_USB)
	غیرعلامتی حرف i;

	usb_disable_asynch(1); /* asynch transfer not allowed */

	usb_stor_reset();
	برائے (i = 0; i < USB_MAX_DEVICE; i++) {
		ساخت usb_device *dev;

		dev = usb_get_dev_اشاریہ(i); /* get device */
		debug("i=%d\n", i);
		اگر (usb_stor_probe_device(dev))
			توڑو;
	} /* for */

	usb_disable_asynch(0); /* asynch transfer allowed */
#اگر_ختم
	لکھو("%d Storage Device(s) found\n", usb_max_devs);
	اگر (usb_max_devs > 0)
		واپس 0;
	واپس -1;
}

ساکن صحیح usb_stor_irq(ساخت usb_device *dev)
{
	ساخت us_ڈیٹا *us;
	us = (ساخت us_ڈیٹا *)dev->privptr;

	اگر (us->ip_wanted)
		us->ip_wanted = 0;
	واپس 0;
}

#اگر_تعریف_شدہ	DEBUG

ساکن خالی usb_show_srb(ساخت scsi_cmd *pccb)
{
	صحیح i;
	لکھو("SRB: len %d datalen 0x%lX\n ", pccb->cmdlen, pccb->datalen);
	برائے (i = 0; i < pccb->cmdlen; i++)
		لکھو("%02X ", pccb->cmd[i]);
	لکھو("\n");
}

ساکن خالی display_int_حالت(غیرعلامتی طویل tmp)
{
	لکھو("Status: %s %s %s %s %s %s %s\n",
		(tmp & USB_ST_فعال) ? "Active" : "",
		(tmp & USB_ST_STALLED) ? "Stalled" : "",
		(tmp & USB_ST_BUF_ERR) ? "Buffer Error" : "",
		(tmp & USB_ST_BABBLE_DET) ? "Babble Det" : "",
		(tmp & USB_ST_NAK_REC) ? "NAKed" : "",
		(tmp & USB_ST_چکری_جانچ_ERR) ? "CRC Error" : "",
		(tmp & USB_ST_ثنائی_ہندسہ_ERR) ? "Bitstuff Error" : "");
}
#اگر_ختم
/***********************************************************************
 * Data transfer routines
 ***********************************************************************/

ساکن صحیح us_one_transfer(ساخت us_ڈیٹا *us, صحیح pipe, حرف *buf, صحیح لمبائی)
{
	صحیح max_سائز;
	صحیح this_xfer;
	صحیح نتیجہ;
	صحیح partial;
	صحیح maxtry;
	صحیح stat;

	/* determine the maximum packet size for these transfers */
	max_سائز = usb_maxpacket(us->pusb_dev, pipe) * 16;

	/* while we have data left to transfer */
	جب (لمبائی) {

		/* calculate how long this will be -- maximum or a remainder */
		this_xfer = لمبائی > max_سائز ? max_سائز : لمبائی;
		لمبائی -= this_xfer;

		/* setup the retry counter */
		maxtry = 10;

		/* set up the transfer loop */
		کرو {
			/* transfer the data */
			debug("Bulk xfer 0x%lx(%d) try #%d\n",
			      (ulong)map_مقصد_sysmem(buf), this_xfer,
			      11 - maxtry);
			نتیجہ = usb_bulk_پیغام(us->pusb_dev, pipe, buf,
					      this_xfer, &partial,
					      USB_CNTL_TIMEOUT * 5);
			debug("bulk_msg returned %d xferred %d/%d\n",
			      نتیجہ, partial, this_xfer);
			اگر (us->pusb_dev->حالت_2 != 0) {
				/* if we stall, we need to clear it before
				 * we go on
				 */
#اگر_تعریف_شدہ DEBUG
				display_int_حالت(us->pusb_dev->حالت_2);
#اگر_ختم
				اگر (us->pusb_dev->حالت_2 & USB_ST_STALLED) {
					debug("stalled ->clearing endpoint" \
					      "halt for pipe 0x%x\n", pipe);
					stat = us->pusb_dev->حالت_2;
					usb_clear_halt(us->pusb_dev, pipe);
					us->pusb_dev->حالت_2 = stat;
					اگر (this_xfer == partial) {
						debug("bulk transferred" \
						      "with error %lX," \
						      " but data ok\n",
						      us->pusb_dev->حالت_2);
						واپس 0;
					}
					ورنہ
						واپس نتیجہ;
				}
				اگر (us->pusb_dev->حالت_2 & USB_ST_NAK_REC) {
					debug("Device NAKed bulk_msg\n");
					واپس نتیجہ;
				}
				debug("bulk transferred with error");
				اگر (this_xfer == partial) {
					debug(" %ld, but data ok\n",
					      us->pusb_dev->حالت_2);
					واپس 0;
				}
				/* if our try counter reaches 0, bail out */
				debug(" %ld, data %d\n",
				      us->pusb_dev->حالت_2, partial);
				اگر (!maxtry--)
						واپس نتیجہ;
			}
			/* update to show what data was transferred */
			this_xfer -= partial;
			buf += partial;
			/* continue until this transfer is done */
		} جب (this_xfer);
	}

	/* if we get here, we're done and successful */
	واپس 0;
}

ساکن صحیح usb_stor_BBB_reset(ساخت us_ڈیٹا *us)
{
	صحیح نتیجہ;
	غیرعلامتی صحیح pipe;

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
	نتیجہ = usb_control_پیغام(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_BBB_RESET,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, تہی, 0, USB_CNTL_TIMEOUT * 5);

	اگر ((نتیجہ < 0) && (us->pusb_dev->حالت_2 & USB_ST_STALLED)) {
		debug("RESET:stall\n");
		واپس -1;
	}

	/* long wait for reset */
	mdelay(150);
	debug("BBB_reset result %d: status %lX reset\n",
	      نتیجہ, us->pusb_dev->حالت_2);
	pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_اندر);
	نتیجہ = usb_clear_halt(us->pusb_dev, pipe);
	/* long wait for reset */
	mdelay(150);
	debug("BBB_reset result %d: status %lX clearing IN endpoint\n",
	      نتیجہ, us->pusb_dev->حالت_2);
	/* long wait for reset */
	pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_برآمد);
	نتیجہ = usb_clear_halt(us->pusb_dev, pipe);
	mdelay(150);
	debug("BBB_reset result %d: status %lX clearing OUT endpoint\n",
	      نتیجہ, us->pusb_dev->حالت_2);
	debug("BBB_reset done\n");
	واپس 0;
}

/* FIXME: this reset function doesn't really reset the port, and it
 * should. Actually it should probably do what it's doing here, and
 * reset the port physically
 */
ساکن صحیح usb_stor_CB_reset(ساخت us_ڈیٹا *us)
{
	غیرعلامتی حرف cmd[12];
	صحیح نتیجہ;

	debug("CB_reset\n");
	memset(cmd, 0xff, حجم(cmd));
	cmd[0] = SCSI_SEND_DIAG;
	cmd[1] = 4;
	نتیجہ = usb_control_پیغام(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_CBI_ADSC,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, cmd, حجم(cmd),
				 USB_CNTL_TIMEOUT * 5);

	/* long wait for reset */
	mdelay(1500);
	debug("CB_reset result %d: status %lX clearing endpoint halt\n",
	      نتیجہ, us->pusb_dev->حالت_2);
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_اندر));
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_برآمد));

	debug("CB_reset done\n");
	واپس 0;
}

/*
 * Set up the command for a BBB device. Note that the actual SCSI
 * command is copied into cbw.CBWCDB.
 */
ساکن صحیح usb_stor_BBB_comdat(ساخت scsi_cmd *srb, ساخت us_ڈیٹا *us)
{
	صحیح نتیجہ;
	صحیح actlen;
	صحیح dir_اندر;
	غیرعلامتی صحیح pipe;
	ALLOC_CACHE_ALIGN_بفر(ساخت umass_bbb_cbw, cbw, 1);

	dir_اندر = US_DIRECTION(srb->cmd[0]);

#اگر_تعریف_شدہ BBB_COMDAT_TRACE
	لکھو("dir %d lun %d cmdlen %d cmd %p datalen %lu pdata %p\n",
		dir_اندر, srb->lun, srb->cmdlen, srb->cmd, srb->datalen,
		srb->pdata);
	اگر (srb->cmdlen) {
		برائے (نتیجہ = 0; نتیجہ < srb->cmdlen; نتیجہ++)
			لکھو("cmd[%d] %#x ", نتیجہ, srb->cmd[نتیجہ]);
		لکھو("\n");
	}
#اگر_ختم
	/* sanity checks */
	اگر (!(srb->cmdlen <= CBWCDBLENGTH)) {
		debug("usb_stor_BBB_comdat:cmdlen too large\n");
		واپس -1;
	}

	/* always OUT to the ep */
	pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_برآمد);

	cbw->dCBWSignature = cpu_مقصد_le32(CBWSIGNATURE);
	cbw->dCBWTag = cpu_مقصد_le32(CBWTag++);
	cbw->d_CBWData_Transfer_لمبائی = cpu_مقصد_le32(srb->datalen);
	cbw->bCBWFlags = (dir_اندر ? CBWFLAGS_اندر : CBWFLAGS_برآمد);
	cbw->bCBWLUN = srb->lun;
	cbw->bCDBLength = srb->cmdlen;
	/* copy the command data into the CBW command data buffer */
	/* DST SRC LEN!!! */

	حافظہ_نقل(cbw->CBWCDB, srb->cmd, srb->cmdlen);
	نتیجہ = usb_bulk_پیغام(us->pusb_dev, pipe, cbw, UMASS_BBB_CBW_سائز,
			      &actlen, USB_CNTL_TIMEOUT * 5);
	اگر (نتیجہ < 0)
		debug("usb_stor_BBB_comdat:usb_bulk_msg error\n");
	واپس نتیجہ;
}

/* FIXME: we also need a CBI_command which sets up the completion
 * interrupt, and waits for it
 */
ساکن صحیح usb_stor_CB_comdat(ساخت scsi_cmd *srb, ساخت us_ڈیٹا *us)
{
	صحیح نتیجہ = 0;
	صحیح dir_اندر, retry;
	غیرعلامتی صحیح pipe;
	غیرعلامتی طویل حالت_2;

	retry = 5;
	dir_اندر = US_DIRECTION(srb->cmd[0]);

	اگر (dir_اندر)
		pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_اندر);
	ورنہ
		pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_برآمد);

	جب (retry--) {
		debug("CBI gets a command: Try %d\n", 5 - retry);
#اگر_تعریف_شدہ DEBUG
		usb_show_srb(srb);
#اگر_ختم
		/* let's send the command via the control pipe */
		نتیجہ = usb_control_پیغام(us->pusb_dev,
					 usb_sndctrlpipe(us->pusb_dev , 0),
					 US_CBI_ADSC,
					 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
					 0, us->ifnum,
					 srb->cmd, srb->cmdlen,
					 USB_CNTL_TIMEOUT * 5);
		debug("CB_transport: control msg returned %d, status %lX\n",
		      نتیجہ, us->pusb_dev->حالت_2);
		/* check the return code for the command */
		اگر (نتیجہ < 0) {
			اگر (us->pusb_dev->حالت_2 & USB_ST_STALLED) {
				حالت_2 = us->pusb_dev->حالت_2;
				debug(" stall during command found," \
				      " clear pipe\n");
				usb_clear_halt(us->pusb_dev,
					      usb_sndctrlpipe(us->pusb_dev, 0));
				us->pusb_dev->حالت_2 = حالت_2;
			}
			debug(" error during command %02X" \
			      " Stat = %lX\n", srb->cmd[0],
			      us->pusb_dev->حالت_2);
			واپس نتیجہ;
		}
		/* transfer the data payload for this command, if one exists*/

		debug("CB_transport: control msg returned %d," \
		      " direction is %s to go 0x%lx\n", نتیجہ,
		      dir_اندر ? "IN" : "OUT", srb->datalen);
		اگر (srb->datalen) {
			نتیجہ = us_one_transfer(us, pipe, (حرف *)srb->pdata,
						 srb->datalen);
			debug("CBI attempted to transfer data," \
			      " result is %d status %lX, len %d\n",
			      نتیجہ, us->pusb_dev->حالت_2,
				us->pusb_dev->act_len);
			اگر (!(us->pusb_dev->حالت_2 & USB_ST_NAK_REC))
				توڑو;
		} /* if (srb->datalen) */
		ورنہ
			توڑو;
	}
	/* return result */

	واپس نتیجہ;
}

ساکن صحیح usb_stor_CBI_get_حالت(ساخت scsi_cmd *srb, ساخت us_ڈیٹا *us)
{
	صحیح timeout;

	us->ip_wanted = 1;
	usb_int_پیغام(us->pusb_dev, us->irqpipe,
		    (خالی *)&us->ip_ڈیٹا, us->irqmaxp, us->irqinterval, false);
	timeout = 1000;
	جب (timeout--) {
		اگر (us->ip_wanted == 0)
			توڑو;
		mdelay(10);
	}
	اگر (us->ip_wanted) {
		لکھو("	Did not get interrupt on CBI\n");
		us->ip_wanted = 0;
		واپس USB_STOR_TRANSPORT_ERROR;
	}
	debug("Got interrupt data 0x%x, transferred %d status 0x%lX\n",
	      us->ip_ڈیٹا, us->pusb_dev->irq_act_len,
	      us->pusb_dev->irq_حالت);
	/* UFI gives us ASC and ASCQ, like a request sense */
	اگر (us->subclass == US_SC_UFI) {
		اگر (srb->cmd[0] == SCSI_REQ_SENSE ||
		    srb->cmd[0] == SCSI_INQUIRY)
			واپس USB_STOR_TRANSPORT_GOOD; /* Good */
		ورنہ اگر (us->ip_ڈیٹا)
			واپس USB_STOR_TRANSPORT_FAILED;
		ورنہ
			واپس USB_STOR_TRANSPORT_GOOD;
	}
	/* otherwise, we interpret the data normally */
	انتخاب (us->ip_ڈیٹا) {
	حالت 0x0001:
		واپس USB_STOR_TRANSPORT_GOOD;
	حالت 0x0002:
		واپس USB_STOR_TRANSPORT_FAILED;
	طےشدہ:
		واپس USB_STOR_TRANSPORT_ERROR;
	}			/* switch */
	واپس USB_STOR_TRANSPORT_ERROR;
}

#تعریف_کرو USB_TRANSPORT_UNKNOWN_RETRY 5
#تعریف_کرو USB_TRANSPORT_NOT_READY_RETRY 10

/* clear a stall on an endpoint - special for BBB devices */
ساکن صحیح usb_stor_BBB_clear_endpt_stall(ساخت us_ڈیٹا *us, __u8 endpt)
{
	/* ENDPOINT_HALT = 0, so set value to 0 */
	واپس usb_control_پیغام(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
			       USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
			       endpt, تہی, 0, USB_CNTL_TIMEOUT * 5);
}

ساکن صحیح usb_stor_BBB_transport(ساخت scsi_cmd *srb, ساخت us_ڈیٹا *us)
{
	صحیح نتیجہ, retry;
	صحیح dir_اندر;
	صحیح actlen, ڈیٹا_actlen;
	غیرعلامتی صحیح pipe, pipein, pipeout;
	ALLOC_CACHE_ALIGN_بفر(ساخت umass_bbb_csw, csw, 1);
#اگر_تعریف_شدہ BBB_XPORT_TRACE
	غیرعلامتی حرف *ptr;
	صحیح اشاریہ;
#اگر_ختم

	dir_اندر = US_DIRECTION(srb->cmd[0]);

	/* COMMAND phase */
	debug("COMMAND phase\n");
	نتیجہ = usb_stor_BBB_comdat(srb, us);
	اگر (نتیجہ < 0) {
		debug("failed to send CBW status %ld\n",
		      us->pusb_dev->حالت_2);
		usb_stor_BBB_reset(us);
		واپس USB_STOR_TRANSPORT_FAILED;
	}
	اگر (!(us->flags & USB_READY))
		mdelay(5);
	pipein = usb_rcvbulkpipe(us->pusb_dev, us->ep_اندر);
	pipeout = usb_sndbulkpipe(us->pusb_dev, us->ep_برآمد);
	/* DATA phase + error handling */
	ڈیٹا_actlen = 0;
	/* no data, go immediately to the STATUS phase */
	اگر (srb->datalen == 0)
		جاؤ st;
	debug("DATA phase\n");
	اگر (dir_اندر)
		pipe = pipein;
	ورنہ
		pipe = pipeout;

	نتیجہ = usb_bulk_پیغام(us->pusb_dev, pipe, srb->pdata, srb->datalen,
			      &ڈیٹا_actlen, USB_CNTL_TIMEOUT * 5);
	/* special handling of STALL in DATA phase */
	اگر ((نتیجہ < 0) && (us->pusb_dev->حالت_2 & USB_ST_STALLED)) {
		debug("DATA:stall\n");
		/* clear the STALL on the endpoint */
		نتیجہ = usb_stor_BBB_clear_endpt_stall(us,
					dir_اندر ? us->ep_اندر : us->ep_برآمد);
		اگر (نتیجہ >= 0)
			/* continue on to STATUS phase */
			جاؤ st;
	}
	اگر (نتیجہ < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->حالت_2);
		usb_stor_BBB_reset(us);
		واپس USB_STOR_TRANSPORT_FAILED;
	}
#اگر_تعریف_شدہ BBB_XPORT_TRACE
	برائے (اشاریہ = 0; اشاریہ < ڈیٹا_actlen; اشاریہ++)
		لکھو("pdata[%d] %#x ", اشاریہ, srb->pdata[اشاریہ]);
	لکھو("\n");
#اگر_ختم
	/* STATUS phase + error handling */
st:
	retry = 0;
again:
	debug("STATUS phase\n");
	نتیجہ = usb_bulk_پیغام(us->pusb_dev, pipein, csw, UMASS_BBB_CSW_سائز,
				&actlen, USB_CNTL_TIMEOUT*5);

	/* special handling of STALL in STATUS phase */
	اگر ((نتیجہ < 0) && (retry < 1) &&
	    (us->pusb_dev->حالت_2 & USB_ST_STALLED)) {
		debug("STATUS:stall\n");
		/* clear the STALL on the endpoint */
		نتیجہ = usb_stor_BBB_clear_endpt_stall(us, us->ep_اندر);
		اگر (نتیجہ >= 0 && (retry++ < 1))
			/* do a retry */
			جاؤ again;
	}
	اگر (نتیجہ < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->حالت_2);
		usb_stor_BBB_reset(us);
		واپس USB_STOR_TRANSPORT_FAILED;
	}
#اگر_تعریف_شدہ BBB_XPORT_TRACE
	ptr = (غیرعلامتی حرف *)csw;
	برائے (اشاریہ = 0; اشاریہ < UMASS_BBB_CSW_سائز; اشاریہ++)
		لکھو("ptr[%d] %#x ", اشاریہ, ptr[اشاریہ]);
	لکھو("\n");
#اگر_ختم
	/* misuse pipe to get the residue */
	pipe = le32_مقصد_cpu(csw->dCSWDataResidue);
	اگر (pipe == 0 && srb->datalen != 0 && srb->datalen - ڈیٹا_actlen != 0)
		pipe = srb->datalen - ڈیٹا_actlen;
	اگر (CSWSIGNATURE != le32_مقصد_cpu(csw->dCSWSignature)) {
		debug("!CSWSIGNATURE\n");
		usb_stor_BBB_reset(us);
		واپس USB_STOR_TRANSPORT_FAILED;
	} ورنہ اگر ((CBWTag - 1) != le32_مقصد_cpu(csw->dCSWTag)) {
		debug("!Tag\n");
		usb_stor_BBB_reset(us);
		واپس USB_STOR_TRANSPORT_FAILED;
	} ورنہ اگر (csw->bCSWStatus > CSWSTATUS_PHASE) {
		debug(">PHASE\n");
		usb_stor_BBB_reset(us);
		واپس USB_STOR_TRANSPORT_FAILED;
	} ورنہ اگر (csw->bCSWStatus == CSWSTATUS_PHASE) {
		debug("=PHASE\n");
		usb_stor_BBB_reset(us);
		واپس USB_STOR_TRANSPORT_FAILED;
	} ورنہ اگر (ڈیٹا_actlen > srb->datalen) {
		debug("transferred %dB instead of %ldB\n",
		      ڈیٹا_actlen, srb->datalen);
		واپس USB_STOR_TRANSPORT_FAILED;
	} ورنہ اگر (csw->bCSWStatus == CSWSTATUS_FAILED) {
		debug("FAILED\n");
		واپس USB_STOR_TRANSPORT_FAILED;
	}

	واپس نتیجہ;
}

ساکن صحیح usb_stor_CB_transport(ساخت scsi_cmd *srb, ساخت us_ڈیٹا *us)
{
	صحیح نتیجہ, حالت_2;
	ساخت scsi_cmd *psrb;
	ساخت scsi_cmd reqsrb;
	صحیح retry, notready;

	psrb = &reqsrb;
	حالت_2 = USB_STOR_TRANSPORT_GOOD;
	retry = 0;
	notready = 0;
	/* issue the command */
do_retry:
	نتیجہ = usb_stor_CB_comdat(srb, us);
	debug("command / Data returned %d, status %lX\n",
	      نتیجہ, us->pusb_dev->حالت_2);
	/* if this is an CBI Protocol, get IRQ */
	اگر (us->protocol == US_PR_CBI) {
		حالت_2 = usb_stor_CBI_get_حالت(srb, us);
		/* if the status is error, report it */
		اگر (حالت_2 == USB_STOR_TRANSPORT_ERROR) {
			debug(" USB CBI Command Error\n");
			واپس حالت_2;
		}
		srb->sense_buf[12] = (غیرعلامتی حرف)(us->ip_ڈیٹا >> 8);
		srb->sense_buf[13] = (غیرعلامتی حرف)(us->ip_ڈیٹا & 0xff);
		اگر (!us->ip_ڈیٹا) {
			/* if the status is good, report it */
			اگر (حالت_2 == USB_STOR_TRANSPORT_GOOD) {
				debug(" USB CBI Command Good\n");
				واپس حالت_2;
			}
		}
	}
	/* do we have to issue an auto request? */
	/* HERE we have to check the result */
	اگر ((نتیجہ < 0) && !(us->pusb_dev->حالت_2 & USB_ST_STALLED)) {
		debug("ERROR %lX\n", us->pusb_dev->حالت_2);
		us->transport_reset(us);
		واپس USB_STOR_TRANSPORT_ERROR;
	}
	اگر ((us->protocol == US_PR_CBI) &&
	    ((srb->cmd[0] == SCSI_REQ_SENSE) ||
	    (srb->cmd[0] == SCSI_INQUIRY))) {
		/* do not issue an autorequest after request sense */
		debug("No auto request and good\n");
		واپس USB_STOR_TRANSPORT_GOOD;
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
	نتیجہ = usb_stor_CB_comdat(psrb, us);
	debug("auto request returned %d\n", نتیجہ);
	/* if this is an CBI Protocol, get IRQ */
	اگر (us->protocol == US_PR_CBI)
		حالت_2 = usb_stor_CBI_get_حالت(psrb, us);

	اگر ((نتیجہ < 0) && !(us->pusb_dev->حالت_2 & USB_ST_STALLED)) {
		debug(" AUTO REQUEST ERROR %ld\n",
		      us->pusb_dev->حالت_2);
		واپس USB_STOR_TRANSPORT_ERROR;
	}
	debug("autorequest returned 0x%02X 0x%02X 0x%02X 0x%02X\n",
	      srb->sense_buf[0], srb->sense_buf[2],
	      srb->sense_buf[12], srb->sense_buf[13]);
	/* Check the auto request result */
	اگر ((srb->sense_buf[2] == 0) &&
	    (srb->sense_buf[12] == 0) &&
	    (srb->sense_buf[13] == 0)) {
		/* ok, no sense */
		واپس USB_STOR_TRANSPORT_GOOD;
	}

	/* Check the auto request result */
	انتخاب (srb->sense_buf[2]) {
	حالت 0x01:
		/* Recovered Error */
		واپس USB_STOR_TRANSPORT_GOOD;
		توڑو;
	حالت 0x02:
		/* Not Ready */
		اگر (notready++ > USB_TRANSPORT_NOT_READY_RETRY) {
			لکھو("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X (NOT READY)\n", srb->cmd[0],
				srb->sense_buf[0], srb->sense_buf[2],
				srb->sense_buf[12], srb->sense_buf[13]);
			واپس USB_STOR_TRANSPORT_FAILED;
		} ورنہ {
			mdelay(100);
			جاؤ do_retry;
		}
		توڑو;
	طےشدہ:
		اگر (retry++ > USB_TRANSPORT_UNKNOWN_RETRY) {
			لکھو("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X\n", srb->cmd[0], srb->sense_buf[0],
				srb->sense_buf[2], srb->sense_buf[12],
				srb->sense_buf[13]);
			واپس USB_STOR_TRANSPORT_FAILED;
		} ورنہ
			جاؤ do_retry;
		توڑو;
	}
	واپس USB_STOR_TRANSPORT_FAILED;
}

ساکن خالی usb_stor_سیٹ_max_xfer_blk(ساخت usb_device *udev,
				      ساخت us_ڈیٹا *us)
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
	غیرعلامتی مختصر blk = 240;

#اگر CONFIG_IS_ENABLED(DM_USB)
	size_t سائز;
	صحیح ret;

	ret = usb_get_max_xfer_سائز(udev, (size_t *)&سائز);
	اگر ((ret >= 0) && (سائز < blk * 512))
		blk = سائز / 512;
#اگر_ختم

	us->max_xfer_blk = blk;
}

ساکن صحیح usb_inquiry(ساخت scsi_cmd *srb, ساخت us_ڈیٹا *ss)
{
	صحیح retry, i;
	retry = 5;
	کرو {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_INQUIRY;
		srb->cmd[1] = srb->lun << 5;
		srb->cmd[4] = 36;
		srb->datalen = 36;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		i = ss->transport(srb, ss);
		debug("inquiry returns %d\n", i);
		اگر (i == 0)
			توڑو;
	} جب (--retry);

	اگر (!retry) {
		لکھو("error in inquiry\n");
		واپس -1;
	}
	واپس 0;
}

ساکن صحیح usb_request_sense(ساخت scsi_cmd *srb, ساخت us_ڈیٹا *ss)
{
	حرف *ptr;

	ptr = (حرف *)srb->pdata;
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
	واپس 0;
}

ساکن صحیح usb_test_unit_ready(ساخت scsi_cmd *srb, ساخت us_ڈیٹا *ss)
{
	صحیح retries = 10;

	کرو {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_TST_U_RDY;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 0;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		اگر (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD) {
			ss->flags |= USB_READY;
			واپس 0;
		}
		usb_request_sense(srb, ss);
		/*
		 * Check the Key Code Qualifier, if it matches
		 * "Not Ready - medium not present"
		 * (the sense Key equals 0x2 and the ASC is 0x3a)
		 * return immediately as the medium being absent won't change
		 * unless there is a user action.
		 */
		اگر ((srb->sense_buf[2] == 0x02) &&
		    (srb->sense_buf[12] == 0x3a))
			واپس -1;
		mdelay(100);
	} جب (retries--);

	واپس -1;
}

ساکن صحیح usb_read_استعداد(ساخت scsi_cmd *srb, ساخت us_ڈیٹا *ss)
{
	صحیح retry;
	/* XXX retries */
	retry = 3;
	کرو {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_RD_CAPAC;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 8;
		srb->cmdlen = ss->cmd12 ? 12 : 10;
		اگر (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD)
			واپس 0;
	} جب (retry--);

	واپس -1;
}

ساکن صحیح usb_read_10(ساخت scsi_cmd *srb, ساخت us_ڈیٹا *ss,
		       غیرعلامتی طویل آغاز, غیرعلامتی مختصر بلاکیں)
{
	memset(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_READ10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((غیرعلامتی حرف) (آغاز >> 24)) & 0xff;
	srb->cmd[3] = ((غیرعلامتی حرف) (آغاز >> 16)) & 0xff;
	srb->cmd[4] = ((غیرعلامتی حرف) (آغاز >> 8)) & 0xff;
	srb->cmd[5] = ((غیرعلامتی حرف) (آغاز)) & 0xff;
	srb->cmd[7] = ((غیرعلامتی حرف) (بلاکیں >> 8)) & 0xff;
	srb->cmd[8] = (غیرعلامتی حرف) بلاکیں & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("read10: start %lx blocks %x\n", آغاز, بلاکیں);
	واپس ss->transport(srb, ss);
}

ساکن صحیح usb_write_10(ساخت scsi_cmd *srb, ساخت us_ڈیٹا *ss,
			غیرعلامتی طویل آغاز, غیرعلامتی مختصر بلاکیں)
{
	memset(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_WRITE10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((غیرعلامتی حرف) (آغاز >> 24)) & 0xff;
	srb->cmd[3] = ((غیرعلامتی حرف) (آغاز >> 16)) & 0xff;
	srb->cmd[4] = ((غیرعلامتی حرف) (آغاز >> 8)) & 0xff;
	srb->cmd[5] = ((غیرعلامتی حرف) (آغاز)) & 0xff;
	srb->cmd[7] = ((غیرعلامتی حرف) (بلاکیں >> 8)) & 0xff;
	srb->cmd[8] = (غیرعلامتی حرف) بلاکیں & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("write10: start %lx blocks %x\n", آغاز, بلاکیں);
	واپس ss->transport(srb, ss);
}

#اگر_تعریف_شدہ CONFIG_USB_BIN_FIXUP
/*
 * Some USB storage devices queried for SCSI identification data respond with
 * binary strings, which if output to the console freeze the terminal. The
 * workaround is to modify the vendor and product strings read from such
 * device with proper values (as reported by 'usb info').
 *
 * Vendor and product length limits are taken from the definition of
 * struct blk_desc in include/part.h.
 */
ساکن خالی usb_bin_fixup(ساخت usb_device_descriptor descriptor,
				غیرعلامتی حرف vendor[],
				غیرعلامتی حرف حاصل_ضرب[]) {
	ثابت غیرعلامتی حرف max_vendor_len = 40;
	ثابت غیرعلامتی حرف max_حاصل_ضرب_len = 20;
	اگر (descriptor.idVendor == 0x0424 && descriptor.id_حاصل_ضرب_2 == 0x223a) {
		strncpy((حرف *)vendor, "SMSC", max_vendor_len);
		strncpy((حرف *)حاصل_ضرب, "Flash Media Cntrller",
			max_حاصل_ضرب_len);
	}
}
#اگر_ختم /* CONFIG_USB_BIN_FIXUP */

#اگر CONFIG_IS_ENABLED(BLK)
ساکن غیرعلامتی طویل usb_stor_read(ساخت udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, خالی *بفر)
#ورنہ
ساکن غیرعلامتی طویل usb_stor_read(ساخت blk_desc *بلاک_dev, lbaint_t blknr,
				   lbaint_t blkcnt, خالی *بفر)
#اگر_ختم
{
	lbaint_t آغاز, blks;
	uintptr_t buf_addr;
	غیرعلامتی مختصر smallblks;
	ساخت usb_device *udev;
	ساخت us_ڈیٹا *ss;
	صحیح retry;
	ساخت scsi_cmd *srb = &usb_ccb;
#اگر CONFIG_IS_ENABLED(BLK)
	ساخت blk_desc *بلاک_dev;
#اگر_ختم

	اگر (blkcnt == 0)
		واپس 0;
	/* Setup  device */
#اگر CONFIG_IS_ENABLED(BLK)
	بلاک_dev = dev_get_uclass_plat(dev);
	udev = dev_get_والد_priv(dev_get_والد(dev));
	debug("\nusb_read: udev %d\n", بلاک_dev->devnum);
#ورنہ
	debug("\nusb_read: udev %d\n", بلاک_dev->devnum);
	udev = usb_dev_desc[بلاک_dev->devnum].priv;
	اگر (!udev) {
		debug("%s: No device\n", __func__);
		واپس 0;
	}
#اگر_ختم
	ss = (ساخت us_ڈیٹا *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_تالا_async(udev, 1);
	srb->lun = بلاک_dev->lun;
	buf_addr = (uintptr_t)بفر;
	آغاز = blknr;
	blks = blkcnt;

	debug("\nusb_read: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      بلاک_dev->devnum, آغاز, blks, buf_addr);

	کرو {
		/* XXX need some comment here */
		retry = 2;
		srb->pdata = (غیرعلامتی حرف *)buf_addr;
		اگر (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		ورنہ
			smallblks = (غیرعلامتی مختصر) blks;
retry_it:
		اگر (smallblks == ss->max_xfer_blk)
			usb_show_پیش_رفت();
		srb->datalen = بلاک_dev->blksz * smallblks;
		srb->pdata = (غیرعلامتی حرف *)buf_addr;
		اگر (usb_read_10(srb, ss, آغاز, smallblks)) {
			debug("Read ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			اگر (retry--)
				جاؤ retry_it;
			blkcnt -= blks;
			توڑو;
		}
		آغاز += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} جب (blks != 0);

	debug("usb_read: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      آغاز, smallblks, buf_addr);

	usb_تالا_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	اگر (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	واپس blkcnt;
}

#اگر CONFIG_IS_ENABLED(BLK)
ساکن غیرعلامتی طویل usb_stor_write(ساخت udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, ثابت خالی *بفر)
#ورنہ
ساکن غیرعلامتی طویل usb_stor_write(ساخت blk_desc *بلاک_dev, lbaint_t blknr,
				    lbaint_t blkcnt, ثابت خالی *بفر)
#اگر_ختم
{
	lbaint_t آغاز, blks;
	uintptr_t buf_addr;
	غیرعلامتی مختصر smallblks;
	ساخت usb_device *udev;
	ساخت us_ڈیٹا *ss;
	صحیح retry;
	ساخت scsi_cmd *srb = &usb_ccb;
#اگر CONFIG_IS_ENABLED(BLK)
	ساخت blk_desc *بلاک_dev;
#اگر_ختم

	اگر (blkcnt == 0)
		واپس 0;

	/* Setup  device */
#اگر CONFIG_IS_ENABLED(BLK)
	بلاک_dev = dev_get_uclass_plat(dev);
	udev = dev_get_والد_priv(dev_get_والد(dev));
	debug("\nusb_read: udev %d\n", بلاک_dev->devnum);
#ورنہ
	debug("\nusb_read: udev %d\n", بلاک_dev->devnum);
	udev = usb_dev_desc[بلاک_dev->devnum].priv;
	اگر (!udev) {
		debug("%s: No device\n", __func__);
		واپس 0;
	}
#اگر_ختم
	ss = (ساخت us_ڈیٹا *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_تالا_async(udev, 1);

	srb->lun = بلاک_dev->lun;
	buf_addr = (uintptr_t)بفر;
	آغاز = blknr;
	blks = blkcnt;

	debug("\nusb_write: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      بلاک_dev->devnum, آغاز, blks, buf_addr);

	کرو {
		/* If write fails retry for max retry count else
		 * return with number of blocks written successfully.
		 */
		retry = 2;
		srb->pdata = (غیرعلامتی حرف *)buf_addr;
		اگر (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		ورنہ
			smallblks = (غیرعلامتی مختصر) blks;
retry_it:
		اگر (smallblks == ss->max_xfer_blk)
			usb_show_پیش_رفت();
		srb->datalen = بلاک_dev->blksz * smallblks;
		srb->pdata = (غیرعلامتی حرف *)buf_addr;
		اگر (usb_write_10(srb, ss, آغاز, smallblks)) {
			debug("Write ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			اگر (retry--)
				جاؤ retry_it;
			blkcnt -= blks;
			توڑو;
		}
		آغاز += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} جب (blks != 0);

	debug("usb_write: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      آغاز, smallblks, buf_addr);

	usb_تالا_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	اگر (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	واپس blkcnt;

}

/* Probe to see if a new device is actually a Storage device */
صحیح usb_storage_probe(ساخت usb_device *dev, غیرعلامتی صحیح ifnum,
		      ساخت us_ڈیٹا *ss)
{
	ساخت usb_interface *iface;
	صحیح i;
	ساخت usb_endpoint_descriptor *ep_desc;
	غیرعلامتی صحیح flags = 0;

	/* let's examine the device now */
	iface = &dev->config.if_desc[ifnum];

	اگر (dev->descriptor.bDeviceClass != 0 ||
			iface->desc.bInterfaceClass != USB_CLASS_MASS_STORAGE ||
			iface->desc.bInterfaceSubClass < US_SC_MIN ||
			iface->desc.bInterfaceSubClass > US_SC_MAX) {
		debug("Not mass storage\n");
		/* if it's not a mass storage, we go no further */
		واپس 0;
	}

	memset(ss, 0, حجم(ساخت us_ڈیٹا));

	/* At this point, we know we've got a live one */
	debug("\n\nUSB Mass Storage device detected\n");

	/* Initialize the us_data structure with some useful info */
	ss->flags = flags;
	ss->ifnum = ifnum;
	ss->pusb_dev = dev;
	ss->attention_مکمل = 0;
	ss->subclass = iface->desc.bInterfaceSubClass;
	ss->protocol = iface->desc.bInterfaceProtocol;

	/* set the handler pointers based on the protocol */
	debug("Transport: ");
	انتخاب (ss->protocol) {
	حالت US_PR_CB:
		debug("Control/Bulk\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		توڑو;

	حالت US_PR_CBI:
		debug("Control/Bulk/Interrupt\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		توڑو;
	حالت US_PR_BULK:
		debug("Bulk/Bulk/Bulk\n");
		ss->transport = usb_stor_BBB_transport;
		ss->transport_reset = usb_stor_BBB_reset;
		توڑو;
	طےشدہ:
		لکھو("USB Storage Transport unknown / not yet implemented\n");
		واپس 0;
		توڑو;
	}

	/*
	 * We are expecting a minimum of 2 endpoints - in and out (bulk).
	 * An optional interrupt is OK (necessary for CBI protocol).
	 * We will ignore any others.
	 */
	برائے (i = 0; i < iface->desc.b_عدد_Endpoints; i++) {
		ep_desc = &iface->ep_desc[i];
		/* is it an BULK endpoint? */
		اگر ((ep_desc->bm_اوصاف &
		     USB_ENDPOINT_XFERTYPE_نقاب) == USB_ENDPOINT_XFER_BULK) {
			اگر (ep_desc->b_Endpoint_پتہ & USB_DIR_اندر)
				ss->ep_اندر = ep_desc->b_Endpoint_پتہ &
						USB_ENDPOINT_نمبر_نقاب;
			ورنہ
				ss->ep_برآمد =
					ep_desc->b_Endpoint_پتہ &
					USB_ENDPOINT_نمبر_نقاب;
		}

		/* is it an interrupt endpoint? */
		اگر ((ep_desc->bm_اوصاف &
		     USB_ENDPOINT_XFERTYPE_نقاب) == USB_ENDPOINT_XFER_INT) {
			ss->ep_int = ep_desc->b_Endpoint_پتہ &
						USB_ENDPOINT_نمبر_نقاب;
			ss->irqinterval = ep_desc->bInterval;
		}
	}
	debug("Endpoints In %d Out %d Int %d\n",
	      ss->ep_اندر, ss->ep_برآمد, ss->ep_int);

	/* Do some basic sanity checks, and bail if we find a problem */
	اگر (usb_سیٹ_interface(dev, iface->desc.b_Interface_نمبر, 0) ||
	    !ss->ep_اندر || !ss->ep_برآمد ||
	    (ss->protocol == US_PR_CBI && ss->ep_int == 0)) {
		debug("Problems with device\n");
		واپس 0;
	}
	/* set class specific stuff */
	/* We only handle certain protocols.  Currently, these are
	 * the only ones.
	 * The SFF8070 accepts the requests used in u-boot
	 */
	اگر (ss->subclass != US_SC_UFI && ss->subclass != US_SC_SCSI &&
	    ss->subclass != US_SC_8070) {
		لکھو("Sorry, protocol %d not yet supported.\n", ss->subclass);
		واپس 0;
	}

	/* UFI uses 12-byte commands (like RBC, unlike SCSI) */
	اگر (ss->subclass == US_SC_UFI)
		ss->cmd12 = true;

	اگر (ss->ep_int) {
		/* we had found an interrupt endpoint, prepare irq pipe
		 * set up the IRQ pipe and handler
		 */
		ss->irqinterval = (ss->irqinterval > 0) ? ss->irqinterval : 255;
		ss->irqpipe = usb_rcvintpipe(ss->pusb_dev, ss->ep_int);
		ss->irqmaxp = usb_maxpacket(dev, ss->irqpipe);
		dev->irq_handle = usb_stor_irq;
	}

	/* Set the maximum transfer size per host controller setting */
	usb_stor_سیٹ_max_xfer_blk(dev, ss);

	dev->privptr = (خالی *)ss;
	واپس 1;
}

صحیح usb_stor_get_info(ساخت usb_device *dev, ساخت us_ڈیٹا *ss,
		      ساخت blk_desc *dev_desc)
{
	غیرعلامتی حرف perq, modi;
	ALLOC_CACHE_ALIGN_بفر(u32, cap, 2);
	ALLOC_CACHE_ALIGN_بفر(u8, usb_stor_buf, 36);
	u32 استعداد, blksz;
	ساخت scsi_cmd *pccb = &usb_ccb;

	pccb->pdata = usb_stor_buf;

	dev_desc->ہدف = dev->devnum;
	pccb->lun = dev_desc->lun;
	debug(" address %d\n", dev_desc->ہدف);

	اگر (usb_inquiry(pccb, ss)) {
		debug("%s: usb_inquiry() failed\n", __func__);
		واپس -1;
	}

	perq = usb_stor_buf[0];
	modi = usb_stor_buf[1];

	/*
	 * Skip unknown devices (0x1f) and enclosure service devices (0x0d),
	 * they would not respond to test_unit_ready .
	 */
	اگر (((perq & 0x1f) == 0x1f) || ((perq & 0x1f) == 0x0d)) {
		debug("%s: unknown/unsupported device\n", __func__);
		واپس 0;
	}
	اگر ((modi&0x80) == 0x80) {
		/* drive is removable */
		dev_desc->removable = 1;
	}
	حافظہ_نقل(dev_desc->vendor, (ثابت خالی *)&usb_stor_buf[8], 8);
	حافظہ_نقل(dev_desc->حاصل_ضرب, (ثابت خالی *)&usb_stor_buf[16], 16);
	حافظہ_نقل(dev_desc->revision, (ثابت خالی *)&usb_stor_buf[32], 4);
	dev_desc->vendor[8] = 0;
	dev_desc->حاصل_ضرب[16] = 0;
	dev_desc->revision[4] = 0;
#اگر_تعریف_شدہ CONFIG_USB_BIN_FIXUP
	usb_bin_fixup(dev->descriptor, (uchar *)dev_desc->vendor,
		      (uchar *)dev_desc->حاصل_ضرب);
#اگر_ختم /* CONFIG_USB_BIN_FIXUP */
	debug("ISO Vers %X, Response Data %X\n", usb_stor_buf[2],
	      usb_stor_buf[3]);
	اگر (usb_test_unit_ready(pccb, ss)) {
		لکھو("Device NOT ready\n"
		       "   Request Sense returned %02X %02X %02X\n",
		       pccb->sense_buf[2], pccb->sense_buf[12],
		       pccb->sense_buf[13]);
		اگر (dev_desc->removable == 1)
			dev_desc->type = perq;
		واپس 0;
	}
	pccb->pdata = (غیرعلامتی حرف *)cap;
	memset(pccb->pdata, 0, 8);
	اگر (usb_read_استعداد(pccb, ss) != 0) {
		لکھو("READ_CAP ERROR\n");
		ss->flags &= ~USB_READY;
		cap[0] = 2880;
		cap[1] = 0x200;
	}
	debug("Read Capacity returns: 0x%08x, 0x%08x\n", cap[0], cap[1]);
#اگر 0
	اگر (cap[0] > (0x200000 * 10)) /* greater than 10 GByte */
		cap[0] >>= 16;

	cap[0] = cpu_مقصد_be32(cap[0]);
	cap[1] = cpu_مقصد_be32(cap[1]);
#اگر_ختم

	استعداد = be32_مقصد_cpu(cap[0]) + 1;
	blksz = be32_مقصد_cpu(cap[1]);

	debug("Capacity = 0x%08x, blocksz = 0x%08x\n", استعداد, blksz);
	dev_desc->lba = استعداد;
	dev_desc->blksz = blksz;
	dev_desc->log2blksz = LOG2(dev_desc->blksz);
	dev_desc->type = perq;
	debug(" address %d\n", dev_desc->ہدف);

	واپس 1;
}

#اگر CONFIG_IS_ENABLED(DM_USB)

ساکن صحیح usb_mass_storage_probe(ساخت udevice *dev)
{
	ساخت usb_device *udev = dev_get_والد_priv(dev);
	صحیح ret;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	ret = usb_stor_probe_device(udev);
	usb_disable_asynch(0); /* asynch transfer allowed */

	واپس ret;
}

ساکن ثابت ساخت udevice_id usb_mass_storage_ids[] = {
	{ .compatible = "usb-mass-storage" },
	{ }
};

U_BOOT_DRIVER(usb_mass_storage) = {
	.name	= "usb_mass_storage",
	.id	= UCLASS_MASS_STORAGE,
	.کا_مطابقت = usb_mass_storage_ids,
	.probe = usb_mass_storage_probe,
#اگر CONFIG_IS_ENABLED(BLK)
	.plat_auto	= حجم(ساخت us_ڈیٹا),
#اگر_ختم
};

UCLASS_DRIVER(usb_mass_storage) = {
	.id		= UCLASS_MASS_STORAGE,
	.name		= "usb_mass_storage",
};

ساکن ثابت ساخت usb_device_id mass_storage_id_جدول[] = {
	{
		.مطابقت_flags = USB_DEVICE_ID_مطابقت_INT_CLASS,
		.bInterfaceClass = USB_CLASS_MASS_STORAGE
	},
	{ }		/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_mass_storage, mass_storage_id_جدول);
#اگر_ختم

#اگر CONFIG_IS_ENABLED(BLK)
ساکن ثابت ساخت blk_ops usb_storage_ops = {
	.اقرأ	= usb_stor_read,
	.اكتب	= usb_stor_write,
};

U_BOOT_DRIVER(usb_storage_blk) = {
	.name		= "usb_storage_blk",
	.id		= UCLASS_BLK,
	.ops		= &usb_storage_ops,
};
#ورنہ
U_BOOT_LEGACY_BLK(usb) = {
	.uclass_idname	= "usb",
	.uclass_id	= UCLASS_USB,
	.max_devs	= USB_MAX_STOR_DEV,
	.desc		= usb_dev_desc,
};
#اگر_ختم
