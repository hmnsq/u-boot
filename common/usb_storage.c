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

#ضمن <blk.h>
#ضمن <bootdev.h>
#ضمن <command.h>
#ضمن <dm.h>
#ضمن <errno.h>
#ضمن <log.h>
#ضمن <mapmem.h>
#ضمن <memalign.h>
#ضمن <asm/byteorder.h>
#ضمن <asm/cache.h>
#ضمن <asm/processor.h>
#ضمن <dm/device-internal.h>
#ضمن <dm/lists.h>
#ضمن <linux/delay.h>

#ضمن <part.h>
#ضمن <usb.h>

#وصف_رد BBB_COMDAT_TRACE
#وصف_رد BBB_XPORT_TRACE

#ضمن <scsi.h>
/* direction table -- this indicates the direction of the data
 * transfer for each command code -- a 1 indicates input
 */
ساكن ثابت بنانشان حرف us_direction[256/8] = {
	0x28, 0x81, 0x14, 0x14, 0x20, 0x01, 0x90, 0x77,
	0x0C, 0x20, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#وصف_ڪريو US_DIRECTION(x) ((us_direction[x>>3] >> (x & 7)) & 1)

ساكن بنية scsi_cmd usb_ccb __aligned(ARCH_DMA_MINALIGN);
ساكن __u32 CBWTag;

ساكن صحيح usb_max_devs; /* number of highest available usb device */

#إذا !CONFIG_IS_ENABLED(BLK)
ساكن بنية blk_desc usb_dev_desc[USB_MAX_STOR_DEV];
#جيڪڏهن_پورو

بنية us_معلومات;
تعريفنوع صحيح (*trans_cmnd)(بنية scsi_cmd *cb, بنية us_معلومات *معلومات);
تعريفنوع صحيح (*trans_reset)(بنية us_معلومات *معلومات);

بنية us_معلومات {
	بنية usb_device *pusb_dev;	 /* this usb_device */

	بنانشان صحيح	flags;			/* from filter initially */
#	define USB_READY	(1 << 0)
	بنانشان حرف	ifnum;			/* interface number */
	بنانشان حرف	ep_اندريون;			/* in endpoint */
	بنانشان حرف	ep_ٻاهريون;			/* out ....... */
	بنانشان حرف	ep_int;			/* interrupt . */
	بنانشان حرف	subclass;		/* as in overview */
	بنانشان حرف	protocol;		/* .............. */
	بنانشان حرف	attention_پورو;		/* force attn on first cmd */
	بنانشان ننڍو	ip_معلومات;		/* interrupt data */
	صحيح		action;			/* what to do */
	صحيح		ip_wanted;		/* needed */
	صحيح		*irq_handle;		/* for USB int requests */
	بنانشان صحيح	irqpipe;		/* pipe for release_irq */
	بنانشان حرف	irqmaxp;		/* max packed for irq Pipe */
	بنانشان حرف	irqinterval;		/* Intervall for IRQ Pipe */
	بنية scsi_cmd	*srb;			/* current srb */
	trans_reset	transport_reset;	/* reset routine */
	trans_cmnd	transport;		/* transport routine */
	بنانشان ننڍو	max_xfer_blk;		/* maximum transfer blocks */
	bool		cmd12;			/* use 12-byte commands (RBC/UFI) */
};

#إذا !CONFIG_IS_ENABLED(BLK)
ساكن بنية us_معلومات usb_stor[USB_MAX_STOR_DEV];
#جيڪڏهن_پورو

#وصف_ڪريو USB_STOR_TRANSPORT_GOOD	   0
#وصف_ڪريو USB_STOR_TRANSPORT_FAILED -1
#وصف_ڪريو USB_STOR_TRANSPORT_ERROR  -2

صحيح usb_stor_get_info(بنية usb_device *dev, بنية us_معلومات *us,
		      بنية blk_desc *dev_desc);
صحيح usb_storage_probe(بنية usb_device *dev, بنانشان صحيح ifnum,
		      بنية us_معلومات *ss);
#إذا CONFIG_IS_ENABLED(BLK)
ساكن بنانشان ڊگهو usb_stor_read(بنية udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, فراغ *بفر);
ساكن بنانشان ڊگهو usb_stor_write(بنية udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, ثابت فراغ *بفر);
#وإلا
ساكن بنانشان ڊگهو usb_stor_read(بنية blk_desc *بلاڪ_dev, lbaint_t blknr,
				   lbaint_t blkcnt, فراغ *بفر);
ساكن بنانشان ڊگهو usb_stor_write(بنية blk_desc *بلاڪ_dev, lbaint_t blknr,
				    lbaint_t blkcnt, ثابت فراغ *بفر);
#جيڪڏهن_پورو
فراغ uhci_show_عارضي_int_td(فراغ);

ساكن فراغ usb_show_ترقي(فراغ)
{
	debug(".");
}

/*******************************************************************************
 * show info on storage devices; 'usb start/init' must be invoked earlier
 * as we only retrieve structures populated during devices initialization
 */
صحيح usb_stor_info(فراغ)
{
	صحيح ڳڻپ = 0;
#إذا CONFIG_IS_ENABLED(BLK)
	بنية udevice *dev;

	لكل (blk_پهريون_device(UCLASS_USB, &dev);
	     dev;
	     blk_ايندڙ_device(&dev)) {
		بنية blk_desc *desc = dev_get_uclass_plat(dev);

		لکو("  Device %d: ", desc->devnum);
		dev_print(desc);
		ڳڻپ++;
	}
#وإلا
	صحيح i;

	إذا (usb_max_devs > 0) {
		لكل (i = 0; i < usb_max_devs; i++) {
			لکو("  Device %d: ", i);
			dev_print(&usb_dev_desc[i]);
		}
		أرجع 0;
	}
#جيڪڏهن_پورو
	إذا (!ڳڻپ) {
		لکو("No storage devices, perhaps not 'usb start'ed..?\n");
		أرجع 1;
	}

	أرجع 0;
}

ساكن بنانشان صحيح usb_get_max_lun(بنية us_معلومات *us)
{
	صحيح len;
	ALLOC_CACHE_ALIGN_بفر(بنانشان حرف, نتيجو, 1);
	len = usb_control_نياپو(us->pusb_dev,
			      usb_rcvctrlpipe(us->pusb_dev, 0),
			      US_BBB_GET_MAX_LUN,
			      USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_اندريون,
			      0, us->ifnum,
			      نتيجو, حجم(حرف),
			      USB_CNTL_TIMEOUT * 5);
	debug("Get Max LUN -> len = %i, result = %i\n", len, (صحيح) *نتيجو);
	أرجع (len > 0) ? *نتيجو : 0;
}

ساكن صحيح usb_stor_probe_device(بنية usb_device *udev)
{
	صحيح lun, max_lun;

#إذا CONFIG_IS_ENABLED(BLK)
	بنية us_معلومات *معلومات;
	صحيح ret;
#وإلا
	صحيح شروعات;

	إذا (udev == عدم)
		أرجع -ENOENT; /* no more devices available */
#جيڪڏهن_پورو

	debug("\n\nProbing for storage\n");
#إذا CONFIG_IS_ENABLED(BLK)
	/*
	 * We store the us_data in the mass storage device's plat. It
	 * is shared by all LUNs (block devices) attached to this mass storage
	 * device.
	 */
	معلومات = dev_get_plat(udev->dev);
	إذا (!usb_storage_probe(udev, 0, معلومات))
		أرجع 0;
	max_lun = usb_get_max_lun(معلومات);
	لكل (lun = 0; lun <= max_lun; lun++) {
		بنية blk_desc *blkdev;
		بنية udevice *dev;
		حرف str[10];

		snprintf(str, حجم(str), "lun%d", lun);
		ret = blk_create_devicef(udev->dev, "usb_storage_blk", str,
					 UCLASS_USB, usb_max_devs,
					 DEFAULT_BLKSZ, 0, &dev);
		إذا (ret) {
			debug("Cannot bind driver\n");
			أرجع ret;
		}

		blkdev = dev_get_uclass_plat(dev);
		blkdev->هدف = 0xff;
		blkdev->lun = lun;

		ret = usb_stor_get_info(udev, معلومات, blkdev);
		إذا (ret == 1) {
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		} وإلا {
			debug("usb_stor_get_info: Invalid device\n");
			ret = device_unbind(dev);
			إذا (ret)
				أرجع ret;
			استمر;
		}

		ret = blk_probe_or_unbind(dev);
		إذا (ret)
			أرجع ret;

		ret = bootdev_تياري_for_sibling_blk(dev, "usb_bootdev");
		إذا (ret) {
			صحيح ret2;

			ret2 = device_unbind(dev);
			إذا (ret2)
				أرجع log_نياپو_ret("bootdev", ret2);
			أرجع log_نياپو_ret("bootdev", ret);
		}
	}
#وإلا
	/* We don't have space to even probe if we hit the maximum */
	إذا (usb_max_devs == USB_MAX_STOR_DEV) {
		لکو("max USB Storage Device reached: %d stopping\n",
		       usb_max_devs);
		أرجع -ENOSPC;
	}

	إذا (!usb_storage_probe(udev, 0, &usb_stor[usb_max_devs]))
		أرجع 0;

	/*
	 * OK, it's a storage device.  Iterate over its LUNs and populate
	 * usb_dev_desc'
	 */
	شروعات = usb_max_devs;

	max_lun = usb_get_max_lun(&usb_stor[usb_max_devs]);
	لكل (lun = 0; lun <= max_lun && usb_max_devs < USB_MAX_STOR_DEV;
	     lun++) {
		بنية blk_desc *blkdev;

		blkdev = &usb_dev_desc[usb_max_devs];
		memset(blkdev, '\0', حجم(بنية blk_desc));
		blkdev->uclass_id = UCLASS_USB;
		blkdev->devnum = usb_max_devs;
		blkdev->part_type = PART_TYPE_UNKNOWN;
		blkdev->هدف = 0xff;
		blkdev->type = DEV_TYPE_UNKNOWN;
		blkdev->بلاڪ_read = usb_stor_read;
		blkdev->بلاڪ_write = usb_stor_write;
		blkdev->lun = lun;
		blkdev->priv = udev;

		إذا (usb_stor_get_info(udev, &usb_stor[شروعات],
				      &usb_dev_desc[usb_max_devs]) == 1) {
			debug("partype: %d\n", blkdev->part_type);
			part_init(blkdev);
			debug("partype: %d\n", blkdev->part_type);
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		}
	}
#جيڪڏهن_پورو

	أرجع 0;
}

فراغ usb_stor_reset(فراغ)
{
	usb_max_devs = 0;
}

/*******************************************************************************
 * scan the usb and reports device info
 * to the user if mode = 1
 * returns current device or -1 if no
 */
صحيح usb_stor_جاچ(صحيح گهڻائي)
{
	إذا (گهڻائي == 1)
		لکو("       scanning usb for storage devices... ");

#إذا !CONFIG_IS_ENABLED(DM_USB)
	بنانشان حرف i;

	usb_disable_asynch(1); /* asynch transfer not allowed */

	usb_stor_reset();
	لكل (i = 0; i < USB_MAX_DEVICE; i++) {
		بنية usb_device *dev;

		dev = usb_get_dev_اشاريو(i); /* get device */
		debug("i=%d\n", i);
		إذا (usb_stor_probe_device(dev))
			اكسر;
	} /* for */

	usb_disable_asynch(0); /* asynch transfer allowed */
#جيڪڏهن_پورو
	لکو("%d Storage Device(s) found\n", usb_max_devs);
	إذا (usb_max_devs > 0)
		أرجع 0;
	أرجع -1;
}

ساكن صحيح usb_stor_irq(بنية usb_device *dev)
{
	بنية us_معلومات *us;
	us = (بنية us_معلومات *)dev->privptr;

	إذا (us->ip_wanted)
		us->ip_wanted = 0;
	أرجع 0;
}

#جيڪڏهن_وصف_ٿيل	DEBUG

ساكن فراغ usb_show_srb(بنية scsi_cmd *pccb)
{
	صحيح i;
	لکو("SRB: len %d datalen 0x%lX\n ", pccb->cmdlen, pccb->datalen);
	لكل (i = 0; i < pccb->cmdlen; i++)
		لکو("%02X ", pccb->cmd[i]);
	لکو("\n");
}

ساكن فراغ display_int_حالت(بنانشان ڊگهو tmp)
{
	لکو("Status: %s %s %s %s %s %s %s\n",
		(tmp & USB_ST_سرگرم) ? "Active" : "",
		(tmp & USB_ST_STALLED) ? "Stalled" : "",
		(tmp & USB_ST_BUF_ERR) ? "Buffer Error" : "",
		(tmp & USB_ST_BABBLE_DET) ? "Babble Det" : "",
		(tmp & USB_ST_NAK_REC) ? "NAKed" : "",
		(tmp & USB_ST_چڪري_جاچ_ERR) ? "CRC Error" : "",
		(tmp & USB_ST_ٻٽو_انگ_ERR) ? "Bitstuff Error" : "");
}
#جيڪڏهن_پورو
/***********************************************************************
 * Data transfer routines
 ***********************************************************************/

ساكن صحيح us_one_transfer(بنية us_معلومات *us, صحيح pipe, حرف *buf, صحيح ڊيگهائي)
{
	صحيح max_سائيز;
	صحيح this_xfer;
	صحيح نتيجو;
	صحيح partial;
	صحيح maxtry;
	صحيح stat;

	/* determine the maximum packet size for these transfers */
	max_سائيز = usb_maxpacket(us->pusb_dev, pipe) * 16;

	/* while we have data left to transfer */
	طالما (ڊيگهائي) {

		/* calculate how long this will be -- maximum or a remainder */
		this_xfer = ڊيگهائي > max_سائيز ? max_سائيز : ڊيگهائي;
		ڊيگهائي -= this_xfer;

		/* setup the retry counter */
		maxtry = 10;

		/* set up the transfer loop */
		افعل {
			/* transfer the data */
			debug("Bulk xfer 0x%lx(%d) try #%d\n",
			      (ulong)map_منزل_sysmem(buf), this_xfer,
			      11 - maxtry);
			نتيجو = usb_bulk_نياپو(us->pusb_dev, pipe, buf,
					      this_xfer, &partial,
					      USB_CNTL_TIMEOUT * 5);
			debug("bulk_msg returned %d xferred %d/%d\n",
			      نتيجو, partial, this_xfer);
			إذا (us->pusb_dev->حالت != 0) {
				/* if we stall, we need to clear it before
				 * we go on
				 */
#جيڪڏهن_وصف_ٿيل DEBUG
				display_int_حالت(us->pusb_dev->حالت);
#جيڪڏهن_پورو
				إذا (us->pusb_dev->حالت & USB_ST_STALLED) {
					debug("stalled ->clearing endpoint" \
					      "halt for pipe 0x%x\n", pipe);
					stat = us->pusb_dev->حالت;
					usb_clear_halt(us->pusb_dev, pipe);
					us->pusb_dev->حالت = stat;
					إذا (this_xfer == partial) {
						debug("bulk transferred" \
						      "with error %lX," \
						      " but data ok\n",
						      us->pusb_dev->حالت);
						أرجع 0;
					}
					وإلا
						أرجع نتيجو;
				}
				إذا (us->pusb_dev->حالت & USB_ST_NAK_REC) {
					debug("Device NAKed bulk_msg\n");
					أرجع نتيجو;
				}
				debug("bulk transferred with error");
				إذا (this_xfer == partial) {
					debug(" %ld, but data ok\n",
					      us->pusb_dev->حالت);
					أرجع 0;
				}
				/* if our try counter reaches 0, bail out */
				debug(" %ld, data %d\n",
				      us->pusb_dev->حالت, partial);
				إذا (!maxtry--)
						أرجع نتيجو;
			}
			/* update to show what data was transferred */
			this_xfer -= partial;
			buf += partial;
			/* continue until this transfer is done */
		} طالما (this_xfer);
	}

	/* if we get here, we're done and successful */
	أرجع 0;
}

ساكن صحيح usb_stor_BBB_reset(بنية us_معلومات *us)
{
	صحيح نتيجو;
	بنانشان صحيح pipe;

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
	نتيجو = usb_control_نياپو(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_BBB_RESET,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, عدم, 0, USB_CNTL_TIMEOUT * 5);

	إذا ((نتيجو < 0) && (us->pusb_dev->حالت & USB_ST_STALLED)) {
		debug("RESET:stall\n");
		أرجع -1;
	}

	/* long wait for reset */
	mdelay(150);
	debug("BBB_reset result %d: status %lX reset\n",
	      نتيجو, us->pusb_dev->حالت);
	pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_اندريون);
	نتيجو = usb_clear_halt(us->pusb_dev, pipe);
	/* long wait for reset */
	mdelay(150);
	debug("BBB_reset result %d: status %lX clearing IN endpoint\n",
	      نتيجو, us->pusb_dev->حالت);
	/* long wait for reset */
	pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_ٻاهريون);
	نتيجو = usb_clear_halt(us->pusb_dev, pipe);
	mdelay(150);
	debug("BBB_reset result %d: status %lX clearing OUT endpoint\n",
	      نتيجو, us->pusb_dev->حالت);
	debug("BBB_reset done\n");
	أرجع 0;
}

/* FIXME: this reset function doesn't really reset the port, and it
 * should. Actually it should probably do what it's doing here, and
 * reset the port physically
 */
ساكن صحيح usb_stor_CB_reset(بنية us_معلومات *us)
{
	بنانشان حرف cmd[12];
	صحيح نتيجو;

	debug("CB_reset\n");
	memset(cmd, 0xff, حجم(cmd));
	cmd[0] = SCSI_SEND_DIAG;
	cmd[1] = 4;
	نتيجو = usb_control_نياپو(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_CBI_ADSC,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, cmd, حجم(cmd),
				 USB_CNTL_TIMEOUT * 5);

	/* long wait for reset */
	mdelay(1500);
	debug("CB_reset result %d: status %lX clearing endpoint halt\n",
	      نتيجو, us->pusb_dev->حالت);
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_اندريون));
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_ٻاهريون));

	debug("CB_reset done\n");
	أرجع 0;
}

/*
 * Set up the command for a BBB device. Note that the actual SCSI
 * command is copied into cbw.CBWCDB.
 */
ساكن صحيح usb_stor_BBB_comdat(بنية scsi_cmd *srb, بنية us_معلومات *us)
{
	صحيح نتيجو;
	صحيح actlen;
	صحيح dir_اندريون;
	بنانشان صحيح pipe;
	ALLOC_CACHE_ALIGN_بفر(بنية umass_bbb_cbw, cbw, 1);

	dir_اندريون = US_DIRECTION(srb->cmd[0]);

#جيڪڏهن_وصف_ٿيل BBB_COMDAT_TRACE
	لکو("dir %d lun %d cmdlen %d cmd %p datalen %lu pdata %p\n",
		dir_اندريون, srb->lun, srb->cmdlen, srb->cmd, srb->datalen,
		srb->pdata);
	إذا (srb->cmdlen) {
		لكل (نتيجو = 0; نتيجو < srb->cmdlen; نتيجو++)
			لکو("cmd[%d] %#x ", نتيجو, srb->cmd[نتيجو]);
		لکو("\n");
	}
#جيڪڏهن_پورو
	/* sanity checks */
	إذا (!(srb->cmdlen <= CBWCDBLENGTH)) {
		debug("usb_stor_BBB_comdat:cmdlen too large\n");
		أرجع -1;
	}

	/* always OUT to the ep */
	pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_ٻاهريون);

	cbw->dCBWSignature = cpu_منزل_le32(CBWSIGNATURE);
	cbw->dCBWTag = cpu_منزل_le32(CBWTag++);
	cbw->d_CBWData_Transfer_ڊيگهائي = cpu_منزل_le32(srb->datalen);
	cbw->bCBWFlags = (dir_اندريون ? CBWFLAGS_اندريون : CBWFLAGS_ٻاهريون);
	cbw->bCBWLUN = srb->lun;
	cbw->bCDBLength = srb->cmdlen;
	/* copy the command data into the CBW command data buffer */
	/* DST SRC LEN!!! */

	يادگيري_نقل(cbw->CBWCDB, srb->cmd, srb->cmdlen);
	نتيجو = usb_bulk_نياپو(us->pusb_dev, pipe, cbw, UMASS_BBB_CBW_سائيز,
			      &actlen, USB_CNTL_TIMEOUT * 5);
	إذا (نتيجو < 0)
		debug("usb_stor_BBB_comdat:usb_bulk_msg error\n");
	أرجع نتيجو;
}

/* FIXME: we also need a CBI_command which sets up the completion
 * interrupt, and waits for it
 */
ساكن صحيح usb_stor_CB_comdat(بنية scsi_cmd *srb, بنية us_معلومات *us)
{
	صحيح نتيجو = 0;
	صحيح dir_اندريون, retry;
	بنانشان صحيح pipe;
	بنانشان ڊگهو حالت;

	retry = 5;
	dir_اندريون = US_DIRECTION(srb->cmd[0]);

	إذا (dir_اندريون)
		pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_اندريون);
	وإلا
		pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_ٻاهريون);

	طالما (retry--) {
		debug("CBI gets a command: Try %d\n", 5 - retry);
#جيڪڏهن_وصف_ٿيل DEBUG
		usb_show_srb(srb);
#جيڪڏهن_پورو
		/* let's send the command via the control pipe */
		نتيجو = usb_control_نياپو(us->pusb_dev,
					 usb_sndctrlpipe(us->pusb_dev , 0),
					 US_CBI_ADSC,
					 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
					 0, us->ifnum,
					 srb->cmd, srb->cmdlen,
					 USB_CNTL_TIMEOUT * 5);
		debug("CB_transport: control msg returned %d, status %lX\n",
		      نتيجو, us->pusb_dev->حالت);
		/* check the return code for the command */
		إذا (نتيجو < 0) {
			إذا (us->pusb_dev->حالت & USB_ST_STALLED) {
				حالت = us->pusb_dev->حالت;
				debug(" stall during command found," \
				      " clear pipe\n");
				usb_clear_halt(us->pusb_dev,
					      usb_sndctrlpipe(us->pusb_dev, 0));
				us->pusb_dev->حالت = حالت;
			}
			debug(" error during command %02X" \
			      " Stat = %lX\n", srb->cmd[0],
			      us->pusb_dev->حالت);
			أرجع نتيجو;
		}
		/* transfer the data payload for this command, if one exists*/

		debug("CB_transport: control msg returned %d," \
		      " direction is %s to go 0x%lx\n", نتيجو,
		      dir_اندريون ? "IN" : "OUT", srb->datalen);
		إذا (srb->datalen) {
			نتيجو = us_one_transfer(us, pipe, (حرف *)srb->pdata,
						 srb->datalen);
			debug("CBI attempted to transfer data," \
			      " result is %d status %lX, len %d\n",
			      نتيجو, us->pusb_dev->حالت,
				us->pusb_dev->act_len);
			إذا (!(us->pusb_dev->حالت & USB_ST_NAK_REC))
				اكسر;
		} /* if (srb->datalen) */
		وإلا
			اكسر;
	}
	/* return result */

	أرجع نتيجو;
}

ساكن صحيح usb_stor_CBI_get_حالت(بنية scsi_cmd *srb, بنية us_معلومات *us)
{
	صحيح timeout;

	us->ip_wanted = 1;
	usb_int_نياپو(us->pusb_dev, us->irqpipe,
		    (فراغ *)&us->ip_معلومات, us->irqmaxp, us->irqinterval, false);
	timeout = 1000;
	طالما (timeout--) {
		إذا (us->ip_wanted == 0)
			اكسر;
		mdelay(10);
	}
	إذا (us->ip_wanted) {
		لکو("	Did not get interrupt on CBI\n");
		us->ip_wanted = 0;
		أرجع USB_STOR_TRANSPORT_ERROR;
	}
	debug("Got interrupt data 0x%x, transferred %d status 0x%lX\n",
	      us->ip_معلومات, us->pusb_dev->irq_act_len,
	      us->pusb_dev->irq_حالت);
	/* UFI gives us ASC and ASCQ, like a request sense */
	إذا (us->subclass == US_SC_UFI) {
		إذا (srb->cmd[0] == SCSI_REQ_SENSE ||
		    srb->cmd[0] == SCSI_INQUIRY)
			أرجع USB_STOR_TRANSPORT_GOOD; /* Good */
		وإلا إذا (us->ip_معلومات)
			أرجع USB_STOR_TRANSPORT_FAILED;
		وإلا
			أرجع USB_STOR_TRANSPORT_GOOD;
	}
	/* otherwise, we interpret the data normally */
	بدل (us->ip_معلومات) {
	حالة 0x0001:
		أرجع USB_STOR_TRANSPORT_GOOD;
	حالة 0x0002:
		أرجع USB_STOR_TRANSPORT_FAILED;
	افتراضي:
		أرجع USB_STOR_TRANSPORT_ERROR;
	}			/* switch */
	أرجع USB_STOR_TRANSPORT_ERROR;
}

#وصف_ڪريو USB_TRANSPORT_UNKNOWN_RETRY 5
#وصف_ڪريو USB_TRANSPORT_NOT_READY_RETRY 10

/* clear a stall on an endpoint - special for BBB devices */
ساكن صحيح usb_stor_BBB_clear_endpt_stall(بنية us_معلومات *us, __u8 endpt)
{
	/* ENDPOINT_HALT = 0, so set value to 0 */
	أرجع usb_control_نياپو(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
			       USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
			       endpt, عدم, 0, USB_CNTL_TIMEOUT * 5);
}

ساكن صحيح usb_stor_BBB_transport(بنية scsi_cmd *srb, بنية us_معلومات *us)
{
	صحيح نتيجو, retry;
	صحيح dir_اندريون;
	صحيح actlen, معلومات_actlen;
	بنانشان صحيح pipe, pipein, pipeout;
	ALLOC_CACHE_ALIGN_بفر(بنية umass_bbb_csw, csw, 1);
#جيڪڏهن_وصف_ٿيل BBB_XPORT_TRACE
	بنانشان حرف *ptr;
	صحيح اشاريو;
#جيڪڏهن_پورو

	dir_اندريون = US_DIRECTION(srb->cmd[0]);

	/* COMMAND phase */
	debug("COMMAND phase\n");
	نتيجو = usb_stor_BBB_comdat(srb, us);
	إذا (نتيجو < 0) {
		debug("failed to send CBW status %ld\n",
		      us->pusb_dev->حالت);
		usb_stor_BBB_reset(us);
		أرجع USB_STOR_TRANSPORT_FAILED;
	}
	إذا (!(us->flags & USB_READY))
		mdelay(5);
	pipein = usb_rcvbulkpipe(us->pusb_dev, us->ep_اندريون);
	pipeout = usb_sndbulkpipe(us->pusb_dev, us->ep_ٻاهريون);
	/* DATA phase + error handling */
	معلومات_actlen = 0;
	/* no data, go immediately to the STATUS phase */
	إذا (srb->datalen == 0)
		اذهب st;
	debug("DATA phase\n");
	إذا (dir_اندريون)
		pipe = pipein;
	وإلا
		pipe = pipeout;

	نتيجو = usb_bulk_نياپو(us->pusb_dev, pipe, srb->pdata, srb->datalen,
			      &معلومات_actlen, USB_CNTL_TIMEOUT * 5);
	/* special handling of STALL in DATA phase */
	إذا ((نتيجو < 0) && (us->pusb_dev->حالت & USB_ST_STALLED)) {
		debug("DATA:stall\n");
		/* clear the STALL on the endpoint */
		نتيجو = usb_stor_BBB_clear_endpt_stall(us,
					dir_اندريون ? us->ep_اندريون : us->ep_ٻاهريون);
		إذا (نتيجو >= 0)
			/* continue on to STATUS phase */
			اذهب st;
	}
	إذا (نتيجو < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->حالت);
		usb_stor_BBB_reset(us);
		أرجع USB_STOR_TRANSPORT_FAILED;
	}
#جيڪڏهن_وصف_ٿيل BBB_XPORT_TRACE
	لكل (اشاريو = 0; اشاريو < معلومات_actlen; اشاريو++)
		لکو("pdata[%d] %#x ", اشاريو, srb->pdata[اشاريو]);
	لکو("\n");
#جيڪڏهن_پورو
	/* STATUS phase + error handling */
st:
	retry = 0;
again:
	debug("STATUS phase\n");
	نتيجو = usb_bulk_نياپو(us->pusb_dev, pipein, csw, UMASS_BBB_CSW_سائيز,
				&actlen, USB_CNTL_TIMEOUT*5);

	/* special handling of STALL in STATUS phase */
	إذا ((نتيجو < 0) && (retry < 1) &&
	    (us->pusb_dev->حالت & USB_ST_STALLED)) {
		debug("STATUS:stall\n");
		/* clear the STALL on the endpoint */
		نتيجو = usb_stor_BBB_clear_endpt_stall(us, us->ep_اندريون);
		إذا (نتيجو >= 0 && (retry++ < 1))
			/* do a retry */
			اذهب again;
	}
	إذا (نتيجو < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->حالت);
		usb_stor_BBB_reset(us);
		أرجع USB_STOR_TRANSPORT_FAILED;
	}
#جيڪڏهن_وصف_ٿيل BBB_XPORT_TRACE
	ptr = (بنانشان حرف *)csw;
	لكل (اشاريو = 0; اشاريو < UMASS_BBB_CSW_سائيز; اشاريو++)
		لکو("ptr[%d] %#x ", اشاريو, ptr[اشاريو]);
	لکو("\n");
#جيڪڏهن_پورو
	/* misuse pipe to get the residue */
	pipe = le32_منزل_cpu(csw->dCSWDataResidue);
	إذا (pipe == 0 && srb->datalen != 0 && srb->datalen - معلومات_actlen != 0)
		pipe = srb->datalen - معلومات_actlen;
	إذا (CSWSIGNATURE != le32_منزل_cpu(csw->dCSWSignature)) {
		debug("!CSWSIGNATURE\n");
		usb_stor_BBB_reset(us);
		أرجع USB_STOR_TRANSPORT_FAILED;
	} وإلا إذا ((CBWTag - 1) != le32_منزل_cpu(csw->dCSWTag)) {
		debug("!Tag\n");
		usb_stor_BBB_reset(us);
		أرجع USB_STOR_TRANSPORT_FAILED;
	} وإلا إذا (csw->bCSWStatus > CSWSTATUS_PHASE) {
		debug(">PHASE\n");
		usb_stor_BBB_reset(us);
		أرجع USB_STOR_TRANSPORT_FAILED;
	} وإلا إذا (csw->bCSWStatus == CSWSTATUS_PHASE) {
		debug("=PHASE\n");
		usb_stor_BBB_reset(us);
		أرجع USB_STOR_TRANSPORT_FAILED;
	} وإلا إذا (معلومات_actlen > srb->datalen) {
		debug("transferred %dB instead of %ldB\n",
		      معلومات_actlen, srb->datalen);
		أرجع USB_STOR_TRANSPORT_FAILED;
	} وإلا إذا (csw->bCSWStatus == CSWSTATUS_FAILED) {
		debug("FAILED\n");
		أرجع USB_STOR_TRANSPORT_FAILED;
	}

	أرجع نتيجو;
}

ساكن صحيح usb_stor_CB_transport(بنية scsi_cmd *srb, بنية us_معلومات *us)
{
	صحيح نتيجو, حالت;
	بنية scsi_cmd *psrb;
	بنية scsi_cmd reqsrb;
	صحيح retry, notready;

	psrb = &reqsrb;
	حالت = USB_STOR_TRANSPORT_GOOD;
	retry = 0;
	notready = 0;
	/* issue the command */
do_retry:
	نتيجو = usb_stor_CB_comdat(srb, us);
	debug("command / Data returned %d, status %lX\n",
	      نتيجو, us->pusb_dev->حالت);
	/* if this is an CBI Protocol, get IRQ */
	إذا (us->protocol == US_PR_CBI) {
		حالت = usb_stor_CBI_get_حالت(srb, us);
		/* if the status is error, report it */
		إذا (حالت == USB_STOR_TRANSPORT_ERROR) {
			debug(" USB CBI Command Error\n");
			أرجع حالت;
		}
		srb->sense_buf[12] = (بنانشان حرف)(us->ip_معلومات >> 8);
		srb->sense_buf[13] = (بنانشان حرف)(us->ip_معلومات & 0xff);
		إذا (!us->ip_معلومات) {
			/* if the status is good, report it */
			إذا (حالت == USB_STOR_TRANSPORT_GOOD) {
				debug(" USB CBI Command Good\n");
				أرجع حالت;
			}
		}
	}
	/* do we have to issue an auto request? */
	/* HERE we have to check the result */
	إذا ((نتيجو < 0) && !(us->pusb_dev->حالت & USB_ST_STALLED)) {
		debug("ERROR %lX\n", us->pusb_dev->حالت);
		us->transport_reset(us);
		أرجع USB_STOR_TRANSPORT_ERROR;
	}
	إذا ((us->protocol == US_PR_CBI) &&
	    ((srb->cmd[0] == SCSI_REQ_SENSE) ||
	    (srb->cmd[0] == SCSI_INQUIRY))) {
		/* do not issue an autorequest after request sense */
		debug("No auto request and good\n");
		أرجع USB_STOR_TRANSPORT_GOOD;
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
	نتيجو = usb_stor_CB_comdat(psrb, us);
	debug("auto request returned %d\n", نتيجو);
	/* if this is an CBI Protocol, get IRQ */
	إذا (us->protocol == US_PR_CBI)
		حالت = usb_stor_CBI_get_حالت(psrb, us);

	إذا ((نتيجو < 0) && !(us->pusb_dev->حالت & USB_ST_STALLED)) {
		debug(" AUTO REQUEST ERROR %ld\n",
		      us->pusb_dev->حالت);
		أرجع USB_STOR_TRANSPORT_ERROR;
	}
	debug("autorequest returned 0x%02X 0x%02X 0x%02X 0x%02X\n",
	      srb->sense_buf[0], srb->sense_buf[2],
	      srb->sense_buf[12], srb->sense_buf[13]);
	/* Check the auto request result */
	إذا ((srb->sense_buf[2] == 0) &&
	    (srb->sense_buf[12] == 0) &&
	    (srb->sense_buf[13] == 0)) {
		/* ok, no sense */
		أرجع USB_STOR_TRANSPORT_GOOD;
	}

	/* Check the auto request result */
	بدل (srb->sense_buf[2]) {
	حالة 0x01:
		/* Recovered Error */
		أرجع USB_STOR_TRANSPORT_GOOD;
		اكسر;
	حالة 0x02:
		/* Not Ready */
		إذا (notready++ > USB_TRANSPORT_NOT_READY_RETRY) {
			لکو("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X (NOT READY)\n", srb->cmd[0],
				srb->sense_buf[0], srb->sense_buf[2],
				srb->sense_buf[12], srb->sense_buf[13]);
			أرجع USB_STOR_TRANSPORT_FAILED;
		} وإلا {
			mdelay(100);
			اذهب do_retry;
		}
		اكسر;
	افتراضي:
		إذا (retry++ > USB_TRANSPORT_UNKNOWN_RETRY) {
			لکو("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X\n", srb->cmd[0], srb->sense_buf[0],
				srb->sense_buf[2], srb->sense_buf[12],
				srb->sense_buf[13]);
			أرجع USB_STOR_TRANSPORT_FAILED;
		} وإلا
			اذهب do_retry;
		اكسر;
	}
	أرجع USB_STOR_TRANSPORT_FAILED;
}

ساكن فراغ usb_stor_سيٽ_max_xfer_blk(بنية usb_device *udev,
				      بنية us_معلومات *us)
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
	بنانشان ننڍو blk = 240;

#إذا CONFIG_IS_ENABLED(DM_USB)
	size_t سائيز;
	صحيح ret;

	ret = usb_get_max_xfer_سائيز(udev, (size_t *)&سائيز);
	إذا ((ret >= 0) && (سائيز < blk * 512))
		blk = سائيز / 512;
#جيڪڏهن_پورو

	us->max_xfer_blk = blk;
}

ساكن صحيح usb_inquiry(بنية scsi_cmd *srb, بنية us_معلومات *ss)
{
	صحيح retry, i;
	retry = 5;
	افعل {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_INQUIRY;
		srb->cmd[1] = srb->lun << 5;
		srb->cmd[4] = 36;
		srb->datalen = 36;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		i = ss->transport(srb, ss);
		debug("inquiry returns %d\n", i);
		إذا (i == 0)
			اكسر;
	} طالما (--retry);

	إذا (!retry) {
		لکو("error in inquiry\n");
		أرجع -1;
	}
	أرجع 0;
}

ساكن صحيح usb_request_sense(بنية scsi_cmd *srb, بنية us_معلومات *ss)
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
	أرجع 0;
}

ساكن صحيح usb_test_unit_ready(بنية scsi_cmd *srb, بنية us_معلومات *ss)
{
	صحيح retries = 10;

	افعل {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_TST_U_RDY;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 0;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		إذا (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD) {
			ss->flags |= USB_READY;
			أرجع 0;
		}
		usb_request_sense(srb, ss);
		/*
		 * Check the Key Code Qualifier, if it matches
		 * "Not Ready - medium not present"
		 * (the sense Key equals 0x2 and the ASC is 0x3a)
		 * return immediately as the medium being absent won't change
		 * unless there is a user action.
		 */
		إذا ((srb->sense_buf[2] == 0x02) &&
		    (srb->sense_buf[12] == 0x3a))
			أرجع -1;
		mdelay(100);
	} طالما (retries--);

	أرجع -1;
}

ساكن صحيح usb_read_صلاحيت(بنية scsi_cmd *srb, بنية us_معلومات *ss)
{
	صحيح retry;
	/* XXX retries */
	retry = 3;
	افعل {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_RD_CAPAC;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 8;
		srb->cmdlen = ss->cmd12 ? 12 : 10;
		إذا (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD)
			أرجع 0;
	} طالما (retry--);

	أرجع -1;
}

ساكن صحيح usb_read_10(بنية scsi_cmd *srb, بنية us_معلومات *ss,
		       بنانشان ڊگهو شروعات, بنانشان ننڍو بلاڪون)
{
	memset(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_READ10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((بنانشان حرف) (شروعات >> 24)) & 0xff;
	srb->cmd[3] = ((بنانشان حرف) (شروعات >> 16)) & 0xff;
	srb->cmd[4] = ((بنانشان حرف) (شروعات >> 8)) & 0xff;
	srb->cmd[5] = ((بنانشان حرف) (شروعات)) & 0xff;
	srb->cmd[7] = ((بنانشان حرف) (بلاڪون >> 8)) & 0xff;
	srb->cmd[8] = (بنانشان حرف) بلاڪون & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("read10: start %lx blocks %x\n", شروعات, بلاڪون);
	أرجع ss->transport(srb, ss);
}

ساكن صحيح usb_write_10(بنية scsi_cmd *srb, بنية us_معلومات *ss,
			بنانشان ڊگهو شروعات, بنانشان ننڍو بلاڪون)
{
	memset(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_WRITE10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((بنانشان حرف) (شروعات >> 24)) & 0xff;
	srb->cmd[3] = ((بنانشان حرف) (شروعات >> 16)) & 0xff;
	srb->cmd[4] = ((بنانشان حرف) (شروعات >> 8)) & 0xff;
	srb->cmd[5] = ((بنانشان حرف) (شروعات)) & 0xff;
	srb->cmd[7] = ((بنانشان حرف) (بلاڪون >> 8)) & 0xff;
	srb->cmd[8] = (بنانشان حرف) بلاڪون & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("write10: start %lx blocks %x\n", شروعات, بلاڪون);
	أرجع ss->transport(srb, ss);
}

#جيڪڏهن_وصف_ٿيل CONFIG_USB_BIN_FIXUP
/*
 * Some USB storage devices queried for SCSI identification data respond with
 * binary strings, which if output to the console freeze the terminal. The
 * workaround is to modify the vendor and product strings read from such
 * device with proper values (as reported by 'usb info').
 *
 * Vendor and product length limits are taken from the definition of
 * struct blk_desc in include/part.h.
 */
ساكن فراغ usb_bin_fixup(بنية usb_device_descriptor descriptor,
				بنانشان حرف vendor[],
				بنانشان حرف حاصل_ضرب[]) {
	ثابت بنانشان حرف max_vendor_len = 40;
	ثابت بنانشان حرف max_حاصل_ضرب_len = 20;
	إذا (descriptor.idVendor == 0x0424 && descriptor.id_حاصل_ضرب_2 == 0x223a) {
		strncpy((حرف *)vendor, "SMSC", max_vendor_len);
		strncpy((حرف *)حاصل_ضرب, "Flash Media Cntrller",
			max_حاصل_ضرب_len);
	}
}
#جيڪڏهن_پورو /* CONFIG_USB_BIN_FIXUP */

#إذا CONFIG_IS_ENABLED(BLK)
ساكن بنانشان ڊگهو usb_stor_read(بنية udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, فراغ *بفر)
#وإلا
ساكن بنانشان ڊگهو usb_stor_read(بنية blk_desc *بلاڪ_dev, lbaint_t blknr,
				   lbaint_t blkcnt, فراغ *بفر)
#جيڪڏهن_پورو
{
	lbaint_t شروعات, blks;
	uintptr_t buf_addr;
	بنانشان ننڍو smallblks;
	بنية usb_device *udev;
	بنية us_معلومات *ss;
	صحيح retry;
	بنية scsi_cmd *srb = &usb_ccb;
#إذا CONFIG_IS_ENABLED(BLK)
	بنية blk_desc *بلاڪ_dev;
#جيڪڏهن_پورو

	إذا (blkcnt == 0)
		أرجع 0;
	/* Setup  device */
#إذا CONFIG_IS_ENABLED(BLK)
	بلاڪ_dev = dev_get_uclass_plat(dev);
	udev = dev_get_والد_priv(dev_get_والد(dev));
	debug("\nusb_read: udev %d\n", بلاڪ_dev->devnum);
#وإلا
	debug("\nusb_read: udev %d\n", بلاڪ_dev->devnum);
	udev = usb_dev_desc[بلاڪ_dev->devnum].priv;
	إذا (!udev) {
		debug("%s: No device\n", __func__);
		أرجع 0;
	}
#جيڪڏهن_پورو
	ss = (بنية us_معلومات *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_تالو_async(udev, 1);
	srb->lun = بلاڪ_dev->lun;
	buf_addr = (uintptr_t)بفر;
	شروعات = blknr;
	blks = blkcnt;

	debug("\nusb_read: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      بلاڪ_dev->devnum, شروعات, blks, buf_addr);

	افعل {
		/* XXX need some comment here */
		retry = 2;
		srb->pdata = (بنانشان حرف *)buf_addr;
		إذا (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		وإلا
			smallblks = (بنانشان ننڍو) blks;
retry_it:
		إذا (smallblks == ss->max_xfer_blk)
			usb_show_ترقي();
		srb->datalen = بلاڪ_dev->blksz * smallblks;
		srb->pdata = (بنانشان حرف *)buf_addr;
		إذا (usb_read_10(srb, ss, شروعات, smallblks)) {
			debug("Read ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			إذا (retry--)
				اذهب retry_it;
			blkcnt -= blks;
			اكسر;
		}
		شروعات += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} طالما (blks != 0);

	debug("usb_read: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      شروعات, smallblks, buf_addr);

	usb_تالو_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	إذا (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	أرجع blkcnt;
}

#إذا CONFIG_IS_ENABLED(BLK)
ساكن بنانشان ڊگهو usb_stor_write(بنية udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, ثابت فراغ *بفر)
#وإلا
ساكن بنانشان ڊگهو usb_stor_write(بنية blk_desc *بلاڪ_dev, lbaint_t blknr,
				    lbaint_t blkcnt, ثابت فراغ *بفر)
#جيڪڏهن_پورو
{
	lbaint_t شروعات, blks;
	uintptr_t buf_addr;
	بنانشان ننڍو smallblks;
	بنية usb_device *udev;
	بنية us_معلومات *ss;
	صحيح retry;
	بنية scsi_cmd *srb = &usb_ccb;
#إذا CONFIG_IS_ENABLED(BLK)
	بنية blk_desc *بلاڪ_dev;
#جيڪڏهن_پورو

	إذا (blkcnt == 0)
		أرجع 0;

	/* Setup  device */
#إذا CONFIG_IS_ENABLED(BLK)
	بلاڪ_dev = dev_get_uclass_plat(dev);
	udev = dev_get_والد_priv(dev_get_والد(dev));
	debug("\nusb_read: udev %d\n", بلاڪ_dev->devnum);
#وإلا
	debug("\nusb_read: udev %d\n", بلاڪ_dev->devnum);
	udev = usb_dev_desc[بلاڪ_dev->devnum].priv;
	إذا (!udev) {
		debug("%s: No device\n", __func__);
		أرجع 0;
	}
#جيڪڏهن_پورو
	ss = (بنية us_معلومات *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_تالو_async(udev, 1);

	srb->lun = بلاڪ_dev->lun;
	buf_addr = (uintptr_t)بفر;
	شروعات = blknr;
	blks = blkcnt;

	debug("\nusb_write: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      بلاڪ_dev->devnum, شروعات, blks, buf_addr);

	افعل {
		/* If write fails retry for max retry count else
		 * return with number of blocks written successfully.
		 */
		retry = 2;
		srb->pdata = (بنانشان حرف *)buf_addr;
		إذا (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		وإلا
			smallblks = (بنانشان ننڍو) blks;
retry_it:
		إذا (smallblks == ss->max_xfer_blk)
			usb_show_ترقي();
		srb->datalen = بلاڪ_dev->blksz * smallblks;
		srb->pdata = (بنانشان حرف *)buf_addr;
		إذا (usb_write_10(srb, ss, شروعات, smallblks)) {
			debug("Write ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			إذا (retry--)
				اذهب retry_it;
			blkcnt -= blks;
			اكسر;
		}
		شروعات += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} طالما (blks != 0);

	debug("usb_write: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      شروعات, smallblks, buf_addr);

	usb_تالو_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	إذا (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	أرجع blkcnt;

}

/* Probe to see if a new device is actually a Storage device */
صحيح usb_storage_probe(بنية usb_device *dev, بنانشان صحيح ifnum,
		      بنية us_معلومات *ss)
{
	بنية usb_interface *iface;
	صحيح i;
	بنية usb_endpoint_descriptor *ep_desc;
	بنانشان صحيح flags = 0;

	/* let's examine the device now */
	iface = &dev->config.if_desc[ifnum];

	إذا (dev->descriptor.bDeviceClass != 0 ||
			iface->desc.bInterfaceClass != USB_CLASS_MASS_STORAGE ||
			iface->desc.bInterfaceSubClass < US_SC_MIN ||
			iface->desc.bInterfaceSubClass > US_SC_MAX) {
		debug("Not mass storage\n");
		/* if it's not a mass storage, we go no further */
		أرجع 0;
	}

	memset(ss, 0, حجم(بنية us_معلومات));

	/* At this point, we know we've got a live one */
	debug("\n\nUSB Mass Storage device detected\n");

	/* Initialize the us_data structure with some useful info */
	ss->flags = flags;
	ss->ifnum = ifnum;
	ss->pusb_dev = dev;
	ss->attention_پورو = 0;
	ss->subclass = iface->desc.bInterfaceSubClass;
	ss->protocol = iface->desc.bInterfaceProtocol;

	/* set the handler pointers based on the protocol */
	debug("Transport: ");
	بدل (ss->protocol) {
	حالة US_PR_CB:
		debug("Control/Bulk\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		اكسر;

	حالة US_PR_CBI:
		debug("Control/Bulk/Interrupt\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		اكسر;
	حالة US_PR_BULK:
		debug("Bulk/Bulk/Bulk\n");
		ss->transport = usb_stor_BBB_transport;
		ss->transport_reset = usb_stor_BBB_reset;
		اكسر;
	افتراضي:
		لکو("USB Storage Transport unknown / not yet implemented\n");
		أرجع 0;
		اكسر;
	}

	/*
	 * We are expecting a minimum of 2 endpoints - in and out (bulk).
	 * An optional interrupt is OK (necessary for CBI protocol).
	 * We will ignore any others.
	 */
	لكل (i = 0; i < iface->desc.b_انگ_Endpoints; i++) {
		ep_desc = &iface->ep_desc[i];
		/* is it an BULK endpoint? */
		إذا ((ep_desc->bm_خاصيتون &
		     USB_ENDPOINT_XFERTYPE_نقاب) == USB_ENDPOINT_XFER_BULK) {
			إذا (ep_desc->b_Endpoint_پتو & USB_DIR_اندريون)
				ss->ep_اندريون = ep_desc->b_Endpoint_پتو &
						USB_ENDPOINT_عدد_نقاب;
			وإلا
				ss->ep_ٻاهريون =
					ep_desc->b_Endpoint_پتو &
					USB_ENDPOINT_عدد_نقاب;
		}

		/* is it an interrupt endpoint? */
		إذا ((ep_desc->bm_خاصيتون &
		     USB_ENDPOINT_XFERTYPE_نقاب) == USB_ENDPOINT_XFER_INT) {
			ss->ep_int = ep_desc->b_Endpoint_پتو &
						USB_ENDPOINT_عدد_نقاب;
			ss->irqinterval = ep_desc->bInterval;
		}
	}
	debug("Endpoints In %d Out %d Int %d\n",
	      ss->ep_اندريون, ss->ep_ٻاهريون, ss->ep_int);

	/* Do some basic sanity checks, and bail if we find a problem */
	إذا (usb_سيٽ_interface(dev, iface->desc.b_Interface_عدد, 0) ||
	    !ss->ep_اندريون || !ss->ep_ٻاهريون ||
	    (ss->protocol == US_PR_CBI && ss->ep_int == 0)) {
		debug("Problems with device\n");
		أرجع 0;
	}
	/* set class specific stuff */
	/* We only handle certain protocols.  Currently, these are
	 * the only ones.
	 * The SFF8070 accepts the requests used in u-boot
	 */
	إذا (ss->subclass != US_SC_UFI && ss->subclass != US_SC_SCSI &&
	    ss->subclass != US_SC_8070) {
		لکو("Sorry, protocol %d not yet supported.\n", ss->subclass);
		أرجع 0;
	}

	/* UFI uses 12-byte commands (like RBC, unlike SCSI) */
	إذا (ss->subclass == US_SC_UFI)
		ss->cmd12 = true;

	إذا (ss->ep_int) {
		/* we had found an interrupt endpoint, prepare irq pipe
		 * set up the IRQ pipe and handler
		 */
		ss->irqinterval = (ss->irqinterval > 0) ? ss->irqinterval : 255;
		ss->irqpipe = usb_rcvintpipe(ss->pusb_dev, ss->ep_int);
		ss->irqmaxp = usb_maxpacket(dev, ss->irqpipe);
		dev->irq_handle = usb_stor_irq;
	}

	/* Set the maximum transfer size per host controller setting */
	usb_stor_سيٽ_max_xfer_blk(dev, ss);

	dev->privptr = (فراغ *)ss;
	أرجع 1;
}

صحيح usb_stor_get_info(بنية usb_device *dev, بنية us_معلومات *ss,
		      بنية blk_desc *dev_desc)
{
	بنانشان حرف perq, modi;
	ALLOC_CACHE_ALIGN_بفر(u32, cap, 2);
	ALLOC_CACHE_ALIGN_بفر(u8, usb_stor_buf, 36);
	u32 صلاحيت, blksz;
	بنية scsi_cmd *pccb = &usb_ccb;

	pccb->pdata = usb_stor_buf;

	dev_desc->هدف = dev->devnum;
	pccb->lun = dev_desc->lun;
	debug(" address %d\n", dev_desc->هدف);

	إذا (usb_inquiry(pccb, ss)) {
		debug("%s: usb_inquiry() failed\n", __func__);
		أرجع -1;
	}

	perq = usb_stor_buf[0];
	modi = usb_stor_buf[1];

	/*
	 * Skip unknown devices (0x1f) and enclosure service devices (0x0d),
	 * they would not respond to test_unit_ready .
	 */
	إذا (((perq & 0x1f) == 0x1f) || ((perq & 0x1f) == 0x0d)) {
		debug("%s: unknown/unsupported device\n", __func__);
		أرجع 0;
	}
	إذا ((modi&0x80) == 0x80) {
		/* drive is removable */
		dev_desc->removable = 1;
	}
	يادگيري_نقل(dev_desc->vendor, (ثابت فراغ *)&usb_stor_buf[8], 8);
	يادگيري_نقل(dev_desc->حاصل_ضرب, (ثابت فراغ *)&usb_stor_buf[16], 16);
	يادگيري_نقل(dev_desc->revision, (ثابت فراغ *)&usb_stor_buf[32], 4);
	dev_desc->vendor[8] = 0;
	dev_desc->حاصل_ضرب[16] = 0;
	dev_desc->revision[4] = 0;
#جيڪڏهن_وصف_ٿيل CONFIG_USB_BIN_FIXUP
	usb_bin_fixup(dev->descriptor, (uchar *)dev_desc->vendor,
		      (uchar *)dev_desc->حاصل_ضرب);
#جيڪڏهن_پورو /* CONFIG_USB_BIN_FIXUP */
	debug("ISO Vers %X, Response Data %X\n", usb_stor_buf[2],
	      usb_stor_buf[3]);
	إذا (usb_test_unit_ready(pccb, ss)) {
		لکو("Device NOT ready\n"
		       "   Request Sense returned %02X %02X %02X\n",
		       pccb->sense_buf[2], pccb->sense_buf[12],
		       pccb->sense_buf[13]);
		إذا (dev_desc->removable == 1)
			dev_desc->type = perq;
		أرجع 0;
	}
	pccb->pdata = (بنانشان حرف *)cap;
	memset(pccb->pdata, 0, 8);
	إذا (usb_read_صلاحيت(pccb, ss) != 0) {
		لکو("READ_CAP ERROR\n");
		ss->flags &= ~USB_READY;
		cap[0] = 2880;
		cap[1] = 0x200;
	}
	debug("Read Capacity returns: 0x%08x, 0x%08x\n", cap[0], cap[1]);
#إذا 0
	إذا (cap[0] > (0x200000 * 10)) /* greater than 10 GByte */
		cap[0] >>= 16;

	cap[0] = cpu_منزل_be32(cap[0]);
	cap[1] = cpu_منزل_be32(cap[1]);
#جيڪڏهن_پورو

	صلاحيت = be32_منزل_cpu(cap[0]) + 1;
	blksz = be32_منزل_cpu(cap[1]);

	debug("Capacity = 0x%08x, blocksz = 0x%08x\n", صلاحيت, blksz);
	dev_desc->lba = صلاحيت;
	dev_desc->blksz = blksz;
	dev_desc->log2blksz = LOG2(dev_desc->blksz);
	dev_desc->type = perq;
	debug(" address %d\n", dev_desc->هدف);

	أرجع 1;
}

#إذا CONFIG_IS_ENABLED(DM_USB)

ساكن صحيح usb_mass_storage_probe(بنية udevice *dev)
{
	بنية usb_device *udev = dev_get_والد_priv(dev);
	صحيح ret;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	ret = usb_stor_probe_device(udev);
	usb_disable_asynch(0); /* asynch transfer allowed */

	أرجع ret;
}

ساكن ثابت بنية udevice_id usb_mass_storage_ids[] = {
	{ .compatible = "usb-mass-storage" },
	{ }
};

U_BOOT_DRIVER(usb_mass_storage) = {
	.name	= "usb_mass_storage",
	.id	= UCLASS_MASS_STORAGE,
	.جو_ميلاپ = usb_mass_storage_ids,
	.probe = usb_mass_storage_probe,
#إذا CONFIG_IS_ENABLED(BLK)
	.plat_auto	= حجم(بنية us_معلومات),
#جيڪڏهن_پورو
};

UCLASS_DRIVER(usb_mass_storage) = {
	.id		= UCLASS_MASS_STORAGE,
	.name		= "usb_mass_storage",
};

ساكن ثابت بنية usb_device_id mass_storage_id_جدول[] = {
	{
		.ميلاپ_flags = USB_DEVICE_ID_ميلاپ_INT_CLASS,
		.bInterfaceClass = USB_CLASS_MASS_STORAGE
	},
	{ }		/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_mass_storage, mass_storage_id_جدول);
#جيڪڏهن_پورو

#إذا CONFIG_IS_ENABLED(BLK)
ساكن ثابت بنية blk_ops usb_storage_ops = {
	.اقرأ	= usb_stor_read,
	.اكتب	= usb_stor_write,
};

U_BOOT_DRIVER(usb_storage_blk) = {
	.name		= "usb_storage_blk",
	.id		= UCLASS_BLK,
	.ops		= &usb_storage_ops,
};
#وإلا
U_BOOT_LEGACY_BLK(usb) = {
	.uclass_idname	= "usb",
	.uclass_id	= UCLASS_USB,
	.max_devs	= USB_MAX_STOR_DEV,
	.desc		= usb_dev_desc,
};
#جيڪڏهن_پورو
