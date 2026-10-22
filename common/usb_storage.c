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

#لغو_تعریف BBB_COMDAT_TRACE
#لغو_تعریف BBB_XPORT_TRACE

#ضمن <scsi.h>
/* direction table -- this indicates the direction of the data
 * transfer for each command code -- a 1 indicates input
 */
ساكن ثابت بیعلامت حرف us_direction[256/8] = {
	0x28, 0x81, 0x14, 0x14, 0x20, 0x01, 0x90, 0x77,
	0x0C, 0x20, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#تعریف_کن US_DIRECTION(x) ((us_direction[x>>3] >> (x & 7)) & 1)

ساكن بنية scsi_cmd usb_ccb __aligned(ARCH_DMA_MINALIGN);
ساكن __u32 CBWTag;

ساكن صحيح usb_max_devs; /* number of highest available usb device */

#إذا !CONFIG_IS_ENABLED(BLK)
ساكن بنية blk_desc usb_dev_desc[USB_MAX_STOR_DEV];
#پایان_اگر

بنية us_داده;
تعريفنوع صحيح (*trans_cmnd)(بنية scsi_cmd *cb, بنية us_داده *داده);
تعريفنوع صحيح (*trans_reset)(بنية us_داده *داده);

بنية us_داده {
	بنية usb_device *pusb_dev;	 /* this usb_device */

	بیعلامت صحيح	flags;			/* from filter initially */
#	define USB_READY	(1 << 0)
	بیعلامت حرف	ifnum;			/* interface number */
	بیعلامت حرف	ep_درون;			/* in endpoint */
	بیعلامت حرف	ep_بیرون;			/* out ....... */
	بیعلامت حرف	ep_int;			/* interrupt . */
	بیعلامت حرف	subclass;		/* as in overview */
	بیعلامت حرف	protocol;		/* .............. */
	بیعلامت حرف	attention_انجامشده;		/* force attn on first cmd */
	بیعلامت کوتاه	ip_داده;		/* interrupt data */
	صحيح		action;			/* what to do */
	صحيح		ip_wanted;		/* needed */
	صحيح		*irq_handle;		/* for USB int requests */
	بیعلامت صحيح	irqpipe;		/* pipe for release_irq */
	بیعلامت حرف	irqmaxp;		/* max packed for irq Pipe */
	بیعلامت حرف	irqinterval;		/* Intervall for IRQ Pipe */
	بنية scsi_cmd	*srb;			/* current srb */
	trans_reset	transport_reset;	/* reset routine */
	trans_cmnd	transport;		/* transport routine */
	بیعلامت کوتاه	max_xfer_blk;		/* maximum transfer blocks */
	bool		cmd12;			/* use 12-byte commands (RBC/UFI) */
};

#إذا !CONFIG_IS_ENABLED(BLK)
ساكن بنية us_داده usb_stor[USB_MAX_STOR_DEV];
#پایان_اگر

#تعریف_کن USB_STOR_TRANSPORT_GOOD	   0
#تعریف_کن USB_STOR_TRANSPORT_FAILED -1
#تعریف_کن USB_STOR_TRANSPORT_ERROR  -2

صحيح usb_stor_get_info(بنية usb_device *dev, بنية us_داده *us,
		      بنية blk_desc *dev_desc);
صحيح usb_storage_probe(بنية usb_device *dev, بیعلامت صحيح ifnum,
		      بنية us_داده *ss);
#إذا CONFIG_IS_ENABLED(BLK)
ساكن بیعلامت دراز usb_stor_read(بنية udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, فراغ *میانگیر);
ساكن بیعلامت دراز usb_stor_write(بنية udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, ثابت فراغ *میانگیر);
#وإلا
ساكن بیعلامت دراز usb_stor_read(بنية blk_desc *بلوک_dev, lbaint_t blknr,
				   lbaint_t blkcnt, فراغ *میانگیر);
ساكن بیعلامت دراز usb_stor_write(بنية blk_desc *بلوک_dev, lbaint_t blknr,
				    lbaint_t blkcnt, ثابت فراغ *میانگیر);
#پایان_اگر
فراغ uhci_show_موقت_int_td(فراغ);

ساكن فراغ usb_show_پیشرفت(فراغ)
{
	debug(".");
}

/*******************************************************************************
 * show info on storage devices; 'usb start/init' must be invoked earlier
 * as we only retrieve structures populated during devices initialization
 */
صحيح usb_stor_info(فراغ)
{
	صحيح شمار = 0;
#إذا CONFIG_IS_ENABLED(BLK)
	بنية udevice *dev;

	لكل (blk_اولی_device(UCLASS_USB, &dev);
	     dev;
	     blk_بعدی_device(&dev)) {
		بنية blk_desc *desc = dev_get_uclass_plat(dev);

		چاپ_کن("  Device %d: ", desc->devnum);
		dev_print(desc);
		شمار++;
	}
#وإلا
	صحيح i;

	إذا (usb_max_devs > 0) {
		لكل (i = 0; i < usb_max_devs; i++) {
			چاپ_کن("  Device %d: ", i);
			dev_print(&usb_dev_desc[i]);
		}
		أرجع 0;
	}
#پایان_اگر
	إذا (!شمار) {
		چاپ_کن("No storage devices, perhaps not 'usb start'ed..?\n");
		أرجع 1;
	}

	أرجع 0;
}

ساكن بیعلامت صحيح usb_get_max_lun(بنية us_داده *us)
{
	صحيح len;
	ALLOC_CACHE_ALIGN_میانگیر(بیعلامت حرف, نتیجه, 1);
	len = usb_control_پیام(us->pusb_dev,
			      usb_rcvctrlpipe(us->pusb_dev, 0),
			      US_BBB_GET_MAX_LUN,
			      USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_درون,
			      0, us->ifnum,
			      نتیجه, حجم(حرف),
			      USB_CNTL_TIMEOUT * 5);
	debug("Get Max LUN -> len = %i, result = %i\n", len, (صحيح) *نتیجه);
	أرجع (len > 0) ? *نتیجه : 0;
}

ساكن صحيح usb_stor_probe_device(بنية usb_device *udev)
{
	صحيح lun, max_lun;

#إذا CONFIG_IS_ENABLED(BLK)
	بنية us_داده *داده;
	صحيح ret;
#وإلا
	صحيح شروع;

	إذا (udev == عدم)
		أرجع -ENOENT; /* no more devices available */
#پایان_اگر

	debug("\n\nProbing for storage\n");
#إذا CONFIG_IS_ENABLED(BLK)
	/*
	 * We store the us_data in the mass storage device's plat. It
	 * is shared by all LUNs (block devices) attached to this mass storage
	 * device.
	 */
	داده = dev_get_plat(udev->dev);
	إذا (!usb_storage_probe(udev, 0, داده))
		أرجع 0;
	max_lun = usb_get_max_lun(داده);
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

		ret = usb_stor_get_info(udev, داده, blkdev);
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

		ret = bootdev_راهاندازی_for_sibling_blk(dev, "usb_bootdev");
		إذا (ret) {
			صحيح ret2;

			ret2 = device_unbind(dev);
			إذا (ret2)
				أرجع log_پیام_ret("bootdev", ret2);
			أرجع log_پیام_ret("bootdev", ret);
		}
	}
#وإلا
	/* We don't have space to even probe if we hit the maximum */
	إذا (usb_max_devs == USB_MAX_STOR_DEV) {
		چاپ_کن("max USB Storage Device reached: %d stopping\n",
		       usb_max_devs);
		أرجع -ENOSPC;
	}

	إذا (!usb_storage_probe(udev, 0, &usb_stor[usb_max_devs]))
		أرجع 0;

	/*
	 * OK, it's a storage device.  Iterate over its LUNs and populate
	 * usb_dev_desc'
	 */
	شروع = usb_max_devs;

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
		blkdev->بلوک_read = usb_stor_read;
		blkdev->بلوک_write = usb_stor_write;
		blkdev->lun = lun;
		blkdev->priv = udev;

		إذا (usb_stor_get_info(udev, &usb_stor[شروع],
				      &usb_dev_desc[usb_max_devs]) == 1) {
			debug("partype: %d\n", blkdev->part_type);
			part_init(blkdev);
			debug("partype: %d\n", blkdev->part_type);
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		}
	}
#پایان_اگر

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
صحيح usb_stor_پیمایش(صحيح نما)
{
	إذا (نما == 1)
		چاپ_کن("       scanning usb for storage devices... ");

#إذا !CONFIG_IS_ENABLED(DM_USB)
	بیعلامت حرف i;

	usb_disable_asynch(1); /* asynch transfer not allowed */

	usb_stor_reset();
	لكل (i = 0; i < USB_MAX_DEVICE; i++) {
		بنية usb_device *dev;

		dev = usb_get_dev_شاخص(i); /* get device */
		debug("i=%d\n", i);
		إذا (usb_stor_probe_device(dev))
			اكسر;
	} /* for */

	usb_disable_asynch(0); /* asynch transfer allowed */
#پایان_اگر
	چاپ_کن("%d Storage Device(s) found\n", usb_max_devs);
	إذا (usb_max_devs > 0)
		أرجع 0;
	أرجع -1;
}

ساكن صحيح usb_stor_irq(بنية usb_device *dev)
{
	بنية us_داده *us;
	us = (بنية us_داده *)dev->privptr;

	إذا (us->ip_wanted)
		us->ip_wanted = 0;
	أرجع 0;
}

#اگر_تعریف_شده	DEBUG

ساكن فراغ usb_show_srb(بنية scsi_cmd *pccb)
{
	صحيح i;
	چاپ_کن("SRB: len %d datalen 0x%lX\n ", pccb->cmdlen, pccb->datalen);
	لكل (i = 0; i < pccb->cmdlen; i++)
		چاپ_کن("%02X ", pccb->cmd[i]);
	چاپ_کن("\n");
}

ساكن فراغ display_int_وضعیت(بیعلامت دراز tmp)
{
	چاپ_کن("Status: %s %s %s %s %s %s %s\n",
		(tmp & USB_ST_فعال) ? "Active" : "",
		(tmp & USB_ST_STALLED) ? "Stalled" : "",
		(tmp & USB_ST_BUF_ERR) ? "Buffer Error" : "",
		(tmp & USB_ST_BABBLE_DET) ? "Babble Det" : "",
		(tmp & USB_ST_NAK_REC) ? "NAKed" : "",
		(tmp & USB_ST_بررسی_چرخهای_ERR) ? "CRC Error" : "",
		(tmp & USB_ST_بیت_ERR) ? "Bitstuff Error" : "");
}
#پایان_اگر
/***********************************************************************
 * Data transfer routines
 ***********************************************************************/

ساكن صحيح us_one_transfer(بنية us_داده *us, صحيح pipe, حرف *buf, صحيح درازی)
{
	صحيح max_اندازه;
	صحيح this_xfer;
	صحيح نتیجه;
	صحيح partial;
	صحيح maxtry;
	صحيح stat;

	/* determine the maximum packet size for these transfers */
	max_اندازه = usb_maxpacket(us->pusb_dev, pipe) * 16;

	/* while we have data left to transfer */
	طالما (درازی) {

		/* calculate how long this will be -- maximum or a remainder */
		this_xfer = درازی > max_اندازه ? max_اندازه : درازی;
		درازی -= this_xfer;

		/* setup the retry counter */
		maxtry = 10;

		/* set up the transfer loop */
		افعل {
			/* transfer the data */
			debug("Bulk xfer 0x%lx(%d) try #%d\n",
			      (ulong)map_مقصد_sysmem(buf), this_xfer,
			      11 - maxtry);
			نتیجه = usb_bulk_پیام(us->pusb_dev, pipe, buf,
					      this_xfer, &partial,
					      USB_CNTL_TIMEOUT * 5);
			debug("bulk_msg returned %d xferred %d/%d\n",
			      نتیجه, partial, this_xfer);
			إذا (us->pusb_dev->وضعیت != 0) {
				/* if we stall, we need to clear it before
				 * we go on
				 */
#اگر_تعریف_شده DEBUG
				display_int_وضعیت(us->pusb_dev->وضعیت);
#پایان_اگر
				إذا (us->pusb_dev->وضعیت & USB_ST_STALLED) {
					debug("stalled ->clearing endpoint" \
					      "halt for pipe 0x%x\n", pipe);
					stat = us->pusb_dev->وضعیت;
					usb_clear_halt(us->pusb_dev, pipe);
					us->pusb_dev->وضعیت = stat;
					إذا (this_xfer == partial) {
						debug("bulk transferred" \
						      "with error %lX," \
						      " but data ok\n",
						      us->pusb_dev->وضعیت);
						أرجع 0;
					}
					وإلا
						أرجع نتیجه;
				}
				إذا (us->pusb_dev->وضعیت & USB_ST_NAK_REC) {
					debug("Device NAKed bulk_msg\n");
					أرجع نتیجه;
				}
				debug("bulk transferred with error");
				إذا (this_xfer == partial) {
					debug(" %ld, but data ok\n",
					      us->pusb_dev->وضعیت);
					أرجع 0;
				}
				/* if our try counter reaches 0, bail out */
				debug(" %ld, data %d\n",
				      us->pusb_dev->وضعیت, partial);
				إذا (!maxtry--)
						أرجع نتیجه;
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

ساكن صحيح usb_stor_BBB_reset(بنية us_داده *us)
{
	صحيح نتیجه;
	بیعلامت صحيح pipe;

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
	نتیجه = usb_control_پیام(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_BBB_RESET,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, عدم, 0, USB_CNTL_TIMEOUT * 5);

	إذا ((نتیجه < 0) && (us->pusb_dev->وضعیت & USB_ST_STALLED)) {
		debug("RESET:stall\n");
		أرجع -1;
	}

	/* long wait for reset */
	mdelay(150);
	debug("BBB_reset result %d: status %lX reset\n",
	      نتیجه, us->pusb_dev->وضعیت);
	pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_درون);
	نتیجه = usb_clear_halt(us->pusb_dev, pipe);
	/* long wait for reset */
	mdelay(150);
	debug("BBB_reset result %d: status %lX clearing IN endpoint\n",
	      نتیجه, us->pusb_dev->وضعیت);
	/* long wait for reset */
	pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_بیرون);
	نتیجه = usb_clear_halt(us->pusb_dev, pipe);
	mdelay(150);
	debug("BBB_reset result %d: status %lX clearing OUT endpoint\n",
	      نتیجه, us->pusb_dev->وضعیت);
	debug("BBB_reset done\n");
	أرجع 0;
}

/* FIXME: this reset function doesn't really reset the port, and it
 * should. Actually it should probably do what it's doing here, and
 * reset the port physically
 */
ساكن صحيح usb_stor_CB_reset(بنية us_داده *us)
{
	بیعلامت حرف cmd[12];
	صحيح نتیجه;

	debug("CB_reset\n");
	memset(cmd, 0xff, حجم(cmd));
	cmd[0] = SCSI_SEND_DIAG;
	cmd[1] = 4;
	نتیجه = usb_control_پیام(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_CBI_ADSC,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, cmd, حجم(cmd),
				 USB_CNTL_TIMEOUT * 5);

	/* long wait for reset */
	mdelay(1500);
	debug("CB_reset result %d: status %lX clearing endpoint halt\n",
	      نتیجه, us->pusb_dev->وضعیت);
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_درون));
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_بیرون));

	debug("CB_reset done\n");
	أرجع 0;
}

/*
 * Set up the command for a BBB device. Note that the actual SCSI
 * command is copied into cbw.CBWCDB.
 */
ساكن صحيح usb_stor_BBB_comdat(بنية scsi_cmd *srb, بنية us_داده *us)
{
	صحيح نتیجه;
	صحيح actlen;
	صحيح dir_درون;
	بیعلامت صحيح pipe;
	ALLOC_CACHE_ALIGN_میانگیر(بنية umass_bbb_cbw, cbw, 1);

	dir_درون = US_DIRECTION(srb->cmd[0]);

#اگر_تعریف_شده BBB_COMDAT_TRACE
	چاپ_کن("dir %d lun %d cmdlen %d cmd %p datalen %lu pdata %p\n",
		dir_درون, srb->lun, srb->cmdlen, srb->cmd, srb->datalen,
		srb->pdata);
	إذا (srb->cmdlen) {
		لكل (نتیجه = 0; نتیجه < srb->cmdlen; نتیجه++)
			چاپ_کن("cmd[%d] %#x ", نتیجه, srb->cmd[نتیجه]);
		چاپ_کن("\n");
	}
#پایان_اگر
	/* sanity checks */
	إذا (!(srb->cmdlen <= CBWCDBLENGTH)) {
		debug("usb_stor_BBB_comdat:cmdlen too large\n");
		أرجع -1;
	}

	/* always OUT to the ep */
	pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_بیرون);

	cbw->dCBWSignature = cpu_مقصد_le32(CBWSIGNATURE);
	cbw->dCBWTag = cpu_مقصد_le32(CBWTag++);
	cbw->d_CBWData_Transfer_درازی = cpu_مقصد_le32(srb->datalen);
	cbw->bCBWFlags = (dir_درون ? CBWFLAGS_درون : CBWFLAGS_بیرون);
	cbw->bCBWLUN = srb->lun;
	cbw->bCDBLength = srb->cmdlen;
	/* copy the command data into the CBW command data buffer */
	/* DST SRC LEN!!! */

	کپی_حافظه(cbw->CBWCDB, srb->cmd, srb->cmdlen);
	نتیجه = usb_bulk_پیام(us->pusb_dev, pipe, cbw, UMASS_BBB_CBW_اندازه,
			      &actlen, USB_CNTL_TIMEOUT * 5);
	إذا (نتیجه < 0)
		debug("usb_stor_BBB_comdat:usb_bulk_msg error\n");
	أرجع نتیجه;
}

/* FIXME: we also need a CBI_command which sets up the completion
 * interrupt, and waits for it
 */
ساكن صحيح usb_stor_CB_comdat(بنية scsi_cmd *srb, بنية us_داده *us)
{
	صحيح نتیجه = 0;
	صحيح dir_درون, retry;
	بیعلامت صحيح pipe;
	بیعلامت دراز وضعیت;

	retry = 5;
	dir_درون = US_DIRECTION(srb->cmd[0]);

	إذا (dir_درون)
		pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_درون);
	وإلا
		pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_بیرون);

	طالما (retry--) {
		debug("CBI gets a command: Try %d\n", 5 - retry);
#اگر_تعریف_شده DEBUG
		usb_show_srb(srb);
#پایان_اگر
		/* let's send the command via the control pipe */
		نتیجه = usb_control_پیام(us->pusb_dev,
					 usb_sndctrlpipe(us->pusb_dev , 0),
					 US_CBI_ADSC,
					 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
					 0, us->ifnum,
					 srb->cmd, srb->cmdlen,
					 USB_CNTL_TIMEOUT * 5);
		debug("CB_transport: control msg returned %d, status %lX\n",
		      نتیجه, us->pusb_dev->وضعیت);
		/* check the return code for the command */
		إذا (نتیجه < 0) {
			إذا (us->pusb_dev->وضعیت & USB_ST_STALLED) {
				وضعیت = us->pusb_dev->وضعیت;
				debug(" stall during command found," \
				      " clear pipe\n");
				usb_clear_halt(us->pusb_dev,
					      usb_sndctrlpipe(us->pusb_dev, 0));
				us->pusb_dev->وضعیت = وضعیت;
			}
			debug(" error during command %02X" \
			      " Stat = %lX\n", srb->cmd[0],
			      us->pusb_dev->وضعیت);
			أرجع نتیجه;
		}
		/* transfer the data payload for this command, if one exists*/

		debug("CB_transport: control msg returned %d," \
		      " direction is %s to go 0x%lx\n", نتیجه,
		      dir_درون ? "IN" : "OUT", srb->datalen);
		إذا (srb->datalen) {
			نتیجه = us_one_transfer(us, pipe, (حرف *)srb->pdata,
						 srb->datalen);
			debug("CBI attempted to transfer data," \
			      " result is %d status %lX, len %d\n",
			      نتیجه, us->pusb_dev->وضعیت,
				us->pusb_dev->act_len);
			إذا (!(us->pusb_dev->وضعیت & USB_ST_NAK_REC))
				اكسر;
		} /* if (srb->datalen) */
		وإلا
			اكسر;
	}
	/* return result */

	أرجع نتیجه;
}

ساكن صحيح usb_stor_CBI_get_وضعیت(بنية scsi_cmd *srb, بنية us_داده *us)
{
	صحيح timeout;

	us->ip_wanted = 1;
	usb_int_پیام(us->pusb_dev, us->irqpipe,
		    (فراغ *)&us->ip_داده, us->irqmaxp, us->irqinterval, false);
	timeout = 1000;
	طالما (timeout--) {
		إذا (us->ip_wanted == 0)
			اكسر;
		mdelay(10);
	}
	إذا (us->ip_wanted) {
		چاپ_کن("	Did not get interrupt on CBI\n");
		us->ip_wanted = 0;
		أرجع USB_STOR_TRANSPORT_ERROR;
	}
	debug("Got interrupt data 0x%x, transferred %d status 0x%lX\n",
	      us->ip_داده, us->pusb_dev->irq_act_len,
	      us->pusb_dev->irq_وضعیت);
	/* UFI gives us ASC and ASCQ, like a request sense */
	إذا (us->subclass == US_SC_UFI) {
		إذا (srb->cmd[0] == SCSI_REQ_SENSE ||
		    srb->cmd[0] == SCSI_INQUIRY)
			أرجع USB_STOR_TRANSPORT_GOOD; /* Good */
		وإلا إذا (us->ip_داده)
			أرجع USB_STOR_TRANSPORT_FAILED;
		وإلا
			أرجع USB_STOR_TRANSPORT_GOOD;
	}
	/* otherwise, we interpret the data normally */
	بدل (us->ip_داده) {
	حالة 0x0001:
		أرجع USB_STOR_TRANSPORT_GOOD;
	حالة 0x0002:
		أرجع USB_STOR_TRANSPORT_FAILED;
	افتراضي:
		أرجع USB_STOR_TRANSPORT_ERROR;
	}			/* switch */
	أرجع USB_STOR_TRANSPORT_ERROR;
}

#تعریف_کن USB_TRANSPORT_UNKNOWN_RETRY 5
#تعریف_کن USB_TRANSPORT_NOT_READY_RETRY 10

/* clear a stall on an endpoint - special for BBB devices */
ساكن صحيح usb_stor_BBB_clear_endpt_stall(بنية us_داده *us, __u8 endpt)
{
	/* ENDPOINT_HALT = 0, so set value to 0 */
	أرجع usb_control_پیام(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
			       USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
			       endpt, عدم, 0, USB_CNTL_TIMEOUT * 5);
}

ساكن صحيح usb_stor_BBB_transport(بنية scsi_cmd *srb, بنية us_داده *us)
{
	صحيح نتیجه, retry;
	صحيح dir_درون;
	صحيح actlen, داده_actlen;
	بیعلامت صحيح pipe, pipein, pipeout;
	ALLOC_CACHE_ALIGN_میانگیر(بنية umass_bbb_csw, csw, 1);
#اگر_تعریف_شده BBB_XPORT_TRACE
	بیعلامت حرف *ptr;
	صحيح شاخص;
#پایان_اگر

	dir_درون = US_DIRECTION(srb->cmd[0]);

	/* COMMAND phase */
	debug("COMMAND phase\n");
	نتیجه = usb_stor_BBB_comdat(srb, us);
	إذا (نتیجه < 0) {
		debug("failed to send CBW status %ld\n",
		      us->pusb_dev->وضعیت);
		usb_stor_BBB_reset(us);
		أرجع USB_STOR_TRANSPORT_FAILED;
	}
	إذا (!(us->flags & USB_READY))
		mdelay(5);
	pipein = usb_rcvbulkpipe(us->pusb_dev, us->ep_درون);
	pipeout = usb_sndbulkpipe(us->pusb_dev, us->ep_بیرون);
	/* DATA phase + error handling */
	داده_actlen = 0;
	/* no data, go immediately to the STATUS phase */
	إذا (srb->datalen == 0)
		اذهب st;
	debug("DATA phase\n");
	إذا (dir_درون)
		pipe = pipein;
	وإلا
		pipe = pipeout;

	نتیجه = usb_bulk_پیام(us->pusb_dev, pipe, srb->pdata, srb->datalen,
			      &داده_actlen, USB_CNTL_TIMEOUT * 5);
	/* special handling of STALL in DATA phase */
	إذا ((نتیجه < 0) && (us->pusb_dev->وضعیت & USB_ST_STALLED)) {
		debug("DATA:stall\n");
		/* clear the STALL on the endpoint */
		نتیجه = usb_stor_BBB_clear_endpt_stall(us,
					dir_درون ? us->ep_درون : us->ep_بیرون);
		إذا (نتیجه >= 0)
			/* continue on to STATUS phase */
			اذهب st;
	}
	إذا (نتیجه < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->وضعیت);
		usb_stor_BBB_reset(us);
		أرجع USB_STOR_TRANSPORT_FAILED;
	}
#اگر_تعریف_شده BBB_XPORT_TRACE
	لكل (شاخص = 0; شاخص < داده_actlen; شاخص++)
		چاپ_کن("pdata[%d] %#x ", شاخص, srb->pdata[شاخص]);
	چاپ_کن("\n");
#پایان_اگر
	/* STATUS phase + error handling */
st:
	retry = 0;
again:
	debug("STATUS phase\n");
	نتیجه = usb_bulk_پیام(us->pusb_dev, pipein, csw, UMASS_BBB_CSW_اندازه,
				&actlen, USB_CNTL_TIMEOUT*5);

	/* special handling of STALL in STATUS phase */
	إذا ((نتیجه < 0) && (retry < 1) &&
	    (us->pusb_dev->وضعیت & USB_ST_STALLED)) {
		debug("STATUS:stall\n");
		/* clear the STALL on the endpoint */
		نتیجه = usb_stor_BBB_clear_endpt_stall(us, us->ep_درون);
		إذا (نتیجه >= 0 && (retry++ < 1))
			/* do a retry */
			اذهب again;
	}
	إذا (نتیجه < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->وضعیت);
		usb_stor_BBB_reset(us);
		أرجع USB_STOR_TRANSPORT_FAILED;
	}
#اگر_تعریف_شده BBB_XPORT_TRACE
	ptr = (بیعلامت حرف *)csw;
	لكل (شاخص = 0; شاخص < UMASS_BBB_CSW_اندازه; شاخص++)
		چاپ_کن("ptr[%d] %#x ", شاخص, ptr[شاخص]);
	چاپ_کن("\n");
#پایان_اگر
	/* misuse pipe to get the residue */
	pipe = le32_مقصد_cpu(csw->dCSWDataResidue);
	إذا (pipe == 0 && srb->datalen != 0 && srb->datalen - داده_actlen != 0)
		pipe = srb->datalen - داده_actlen;
	إذا (CSWSIGNATURE != le32_مقصد_cpu(csw->dCSWSignature)) {
		debug("!CSWSIGNATURE\n");
		usb_stor_BBB_reset(us);
		أرجع USB_STOR_TRANSPORT_FAILED;
	} وإلا إذا ((CBWTag - 1) != le32_مقصد_cpu(csw->dCSWTag)) {
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
	} وإلا إذا (داده_actlen > srb->datalen) {
		debug("transferred %dB instead of %ldB\n",
		      داده_actlen, srb->datalen);
		أرجع USB_STOR_TRANSPORT_FAILED;
	} وإلا إذا (csw->bCSWStatus == CSWSTATUS_FAILED) {
		debug("FAILED\n");
		أرجع USB_STOR_TRANSPORT_FAILED;
	}

	أرجع نتیجه;
}

ساكن صحيح usb_stor_CB_transport(بنية scsi_cmd *srb, بنية us_داده *us)
{
	صحيح نتیجه, وضعیت;
	بنية scsi_cmd *psrb;
	بنية scsi_cmd reqsrb;
	صحيح retry, notready;

	psrb = &reqsrb;
	وضعیت = USB_STOR_TRANSPORT_GOOD;
	retry = 0;
	notready = 0;
	/* issue the command */
do_retry:
	نتیجه = usb_stor_CB_comdat(srb, us);
	debug("command / Data returned %d, status %lX\n",
	      نتیجه, us->pusb_dev->وضعیت);
	/* if this is an CBI Protocol, get IRQ */
	إذا (us->protocol == US_PR_CBI) {
		وضعیت = usb_stor_CBI_get_وضعیت(srb, us);
		/* if the status is error, report it */
		إذا (وضعیت == USB_STOR_TRANSPORT_ERROR) {
			debug(" USB CBI Command Error\n");
			أرجع وضعیت;
		}
		srb->sense_buf[12] = (بیعلامت حرف)(us->ip_داده >> 8);
		srb->sense_buf[13] = (بیعلامت حرف)(us->ip_داده & 0xff);
		إذا (!us->ip_داده) {
			/* if the status is good, report it */
			إذا (وضعیت == USB_STOR_TRANSPORT_GOOD) {
				debug(" USB CBI Command Good\n");
				أرجع وضعیت;
			}
		}
	}
	/* do we have to issue an auto request? */
	/* HERE we have to check the result */
	إذا ((نتیجه < 0) && !(us->pusb_dev->وضعیت & USB_ST_STALLED)) {
		debug("ERROR %lX\n", us->pusb_dev->وضعیت);
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
	نتیجه = usb_stor_CB_comdat(psrb, us);
	debug("auto request returned %d\n", نتیجه);
	/* if this is an CBI Protocol, get IRQ */
	إذا (us->protocol == US_PR_CBI)
		وضعیت = usb_stor_CBI_get_وضعیت(psrb, us);

	إذا ((نتیجه < 0) && !(us->pusb_dev->وضعیت & USB_ST_STALLED)) {
		debug(" AUTO REQUEST ERROR %ld\n",
		      us->pusb_dev->وضعیت);
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
			چاپ_کن("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
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
			چاپ_کن("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
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

ساكن فراغ usb_stor_مجموعه_max_xfer_blk(بنية usb_device *udev,
				      بنية us_داده *us)
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
	بیعلامت کوتاه blk = 240;

#إذا CONFIG_IS_ENABLED(DM_USB)
	size_t اندازه;
	صحيح ret;

	ret = usb_get_max_xfer_اندازه(udev, (size_t *)&اندازه);
	إذا ((ret >= 0) && (اندازه < blk * 512))
		blk = اندازه / 512;
#پایان_اگر

	us->max_xfer_blk = blk;
}

ساكن صحيح usb_inquiry(بنية scsi_cmd *srb, بنية us_داده *ss)
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
		چاپ_کن("error in inquiry\n");
		أرجع -1;
	}
	أرجع 0;
}

ساكن صحيح usb_request_sense(بنية scsi_cmd *srb, بنية us_داده *ss)
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

ساكن صحيح usb_test_unit_ready(بنية scsi_cmd *srb, بنية us_داده *ss)
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

ساكن صحيح usb_read_ظرفیت(بنية scsi_cmd *srb, بنية us_داده *ss)
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

ساكن صحيح usb_read_10(بنية scsi_cmd *srb, بنية us_داده *ss,
		       بیعلامت دراز شروع, بیعلامت کوتاه بلوکها)
{
	memset(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_READ10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((بیعلامت حرف) (شروع >> 24)) & 0xff;
	srb->cmd[3] = ((بیعلامت حرف) (شروع >> 16)) & 0xff;
	srb->cmd[4] = ((بیعلامت حرف) (شروع >> 8)) & 0xff;
	srb->cmd[5] = ((بیعلامت حرف) (شروع)) & 0xff;
	srb->cmd[7] = ((بیعلامت حرف) (بلوکها >> 8)) & 0xff;
	srb->cmd[8] = (بیعلامت حرف) بلوکها & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("read10: start %lx blocks %x\n", شروع, بلوکها);
	أرجع ss->transport(srb, ss);
}

ساكن صحيح usb_write_10(بنية scsi_cmd *srb, بنية us_داده *ss,
			بیعلامت دراز شروع, بیعلامت کوتاه بلوکها)
{
	memset(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_WRITE10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((بیعلامت حرف) (شروع >> 24)) & 0xff;
	srb->cmd[3] = ((بیعلامت حرف) (شروع >> 16)) & 0xff;
	srb->cmd[4] = ((بیعلامت حرف) (شروع >> 8)) & 0xff;
	srb->cmd[5] = ((بیعلامت حرف) (شروع)) & 0xff;
	srb->cmd[7] = ((بیعلامت حرف) (بلوکها >> 8)) & 0xff;
	srb->cmd[8] = (بیعلامت حرف) بلوکها & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("write10: start %lx blocks %x\n", شروع, بلوکها);
	أرجع ss->transport(srb, ss);
}

#اگر_تعریف_شده CONFIG_USB_BIN_FIXUP
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
				بیعلامت حرف vendor[],
				بیعلامت حرف حاصلضرب[]) {
	ثابت بیعلامت حرف max_vendor_len = 40;
	ثابت بیعلامت حرف max_حاصلضرب_len = 20;
	إذا (descriptor.idVendor == 0x0424 && descriptor.id_حاصلضرب_2 == 0x223a) {
		strncpy((حرف *)vendor, "SMSC", max_vendor_len);
		strncpy((حرف *)حاصلضرب, "Flash Media Cntrller",
			max_حاصلضرب_len);
	}
}
#پایان_اگر /* CONFIG_USB_BIN_FIXUP */

#إذا CONFIG_IS_ENABLED(BLK)
ساكن بیعلامت دراز usb_stor_read(بنية udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, فراغ *میانگیر)
#وإلا
ساكن بیعلامت دراز usb_stor_read(بنية blk_desc *بلوک_dev, lbaint_t blknr,
				   lbaint_t blkcnt, فراغ *میانگیر)
#پایان_اگر
{
	lbaint_t شروع, blks;
	uintptr_t buf_addr;
	بیعلامت کوتاه smallblks;
	بنية usb_device *udev;
	بنية us_داده *ss;
	صحيح retry;
	بنية scsi_cmd *srb = &usb_ccb;
#إذا CONFIG_IS_ENABLED(BLK)
	بنية blk_desc *بلوک_dev;
#پایان_اگر

	إذا (blkcnt == 0)
		أرجع 0;
	/* Setup  device */
#إذا CONFIG_IS_ENABLED(BLK)
	بلوک_dev = dev_get_uclass_plat(dev);
	udev = dev_get_والد_priv(dev_get_والد(dev));
	debug("\nusb_read: udev %d\n", بلوک_dev->devnum);
#وإلا
	debug("\nusb_read: udev %d\n", بلوک_dev->devnum);
	udev = usb_dev_desc[بلوک_dev->devnum].priv;
	إذا (!udev) {
		debug("%s: No device\n", __func__);
		أرجع 0;
	}
#پایان_اگر
	ss = (بنية us_داده *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_قفل_async(udev, 1);
	srb->lun = بلوک_dev->lun;
	buf_addr = (uintptr_t)میانگیر;
	شروع = blknr;
	blks = blkcnt;

	debug("\nusb_read: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      بلوک_dev->devnum, شروع, blks, buf_addr);

	افعل {
		/* XXX need some comment here */
		retry = 2;
		srb->pdata = (بیعلامت حرف *)buf_addr;
		إذا (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		وإلا
			smallblks = (بیعلامت کوتاه) blks;
retry_it:
		إذا (smallblks == ss->max_xfer_blk)
			usb_show_پیشرفت();
		srb->datalen = بلوک_dev->blksz * smallblks;
		srb->pdata = (بیعلامت حرف *)buf_addr;
		إذا (usb_read_10(srb, ss, شروع, smallblks)) {
			debug("Read ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			إذا (retry--)
				اذهب retry_it;
			blkcnt -= blks;
			اكسر;
		}
		شروع += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} طالما (blks != 0);

	debug("usb_read: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      شروع, smallblks, buf_addr);

	usb_قفل_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	إذا (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	أرجع blkcnt;
}

#إذا CONFIG_IS_ENABLED(BLK)
ساكن بیعلامت دراز usb_stor_write(بنية udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, ثابت فراغ *میانگیر)
#وإلا
ساكن بیعلامت دراز usb_stor_write(بنية blk_desc *بلوک_dev, lbaint_t blknr,
				    lbaint_t blkcnt, ثابت فراغ *میانگیر)
#پایان_اگر
{
	lbaint_t شروع, blks;
	uintptr_t buf_addr;
	بیعلامت کوتاه smallblks;
	بنية usb_device *udev;
	بنية us_داده *ss;
	صحيح retry;
	بنية scsi_cmd *srb = &usb_ccb;
#إذا CONFIG_IS_ENABLED(BLK)
	بنية blk_desc *بلوک_dev;
#پایان_اگر

	إذا (blkcnt == 0)
		أرجع 0;

	/* Setup  device */
#إذا CONFIG_IS_ENABLED(BLK)
	بلوک_dev = dev_get_uclass_plat(dev);
	udev = dev_get_والد_priv(dev_get_والد(dev));
	debug("\nusb_read: udev %d\n", بلوک_dev->devnum);
#وإلا
	debug("\nusb_read: udev %d\n", بلوک_dev->devnum);
	udev = usb_dev_desc[بلوک_dev->devnum].priv;
	إذا (!udev) {
		debug("%s: No device\n", __func__);
		أرجع 0;
	}
#پایان_اگر
	ss = (بنية us_داده *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_قفل_async(udev, 1);

	srb->lun = بلوک_dev->lun;
	buf_addr = (uintptr_t)میانگیر;
	شروع = blknr;
	blks = blkcnt;

	debug("\nusb_write: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      بلوک_dev->devnum, شروع, blks, buf_addr);

	افعل {
		/* If write fails retry for max retry count else
		 * return with number of blocks written successfully.
		 */
		retry = 2;
		srb->pdata = (بیعلامت حرف *)buf_addr;
		إذا (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		وإلا
			smallblks = (بیعلامت کوتاه) blks;
retry_it:
		إذا (smallblks == ss->max_xfer_blk)
			usb_show_پیشرفت();
		srb->datalen = بلوک_dev->blksz * smallblks;
		srb->pdata = (بیعلامت حرف *)buf_addr;
		إذا (usb_write_10(srb, ss, شروع, smallblks)) {
			debug("Write ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			إذا (retry--)
				اذهب retry_it;
			blkcnt -= blks;
			اكسر;
		}
		شروع += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} طالما (blks != 0);

	debug("usb_write: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      شروع, smallblks, buf_addr);

	usb_قفل_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	إذا (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	أرجع blkcnt;

}

/* Probe to see if a new device is actually a Storage device */
صحيح usb_storage_probe(بنية usb_device *dev, بیعلامت صحيح ifnum,
		      بنية us_داده *ss)
{
	بنية usb_interface *iface;
	صحيح i;
	بنية usb_endpoint_descriptor *ep_desc;
	بیعلامت صحيح flags = 0;

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

	memset(ss, 0, حجم(بنية us_داده));

	/* At this point, we know we've got a live one */
	debug("\n\nUSB Mass Storage device detected\n");

	/* Initialize the us_data structure with some useful info */
	ss->flags = flags;
	ss->ifnum = ifnum;
	ss->pusb_dev = dev;
	ss->attention_انجامشده = 0;
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
		چاپ_کن("USB Storage Transport unknown / not yet implemented\n");
		أرجع 0;
		اكسر;
	}

	/*
	 * We are expecting a minimum of 2 endpoints - in and out (bulk).
	 * An optional interrupt is OK (necessary for CBI protocol).
	 * We will ignore any others.
	 */
	لكل (i = 0; i < iface->desc.b_رقم_Endpoints; i++) {
		ep_desc = &iface->ep_desc[i];
		/* is it an BULK endpoint? */
		إذا ((ep_desc->bm_صفات &
		     USB_ENDPOINT_XFERTYPE_ماسک) == USB_ENDPOINT_XFER_BULK) {
			إذا (ep_desc->b_Endpoint_آدرس & USB_DIR_درون)
				ss->ep_درون = ep_desc->b_Endpoint_آدرس &
						USB_ENDPOINT_عدد_ماسک;
			وإلا
				ss->ep_بیرون =
					ep_desc->b_Endpoint_آدرس &
					USB_ENDPOINT_عدد_ماسک;
		}

		/* is it an interrupt endpoint? */
		إذا ((ep_desc->bm_صفات &
		     USB_ENDPOINT_XFERTYPE_ماسک) == USB_ENDPOINT_XFER_INT) {
			ss->ep_int = ep_desc->b_Endpoint_آدرس &
						USB_ENDPOINT_عدد_ماسک;
			ss->irqinterval = ep_desc->bInterval;
		}
	}
	debug("Endpoints In %d Out %d Int %d\n",
	      ss->ep_درون, ss->ep_بیرون, ss->ep_int);

	/* Do some basic sanity checks, and bail if we find a problem */
	إذا (usb_مجموعه_interface(dev, iface->desc.b_Interface_عدد, 0) ||
	    !ss->ep_درون || !ss->ep_بیرون ||
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
		چاپ_کن("Sorry, protocol %d not yet supported.\n", ss->subclass);
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
	usb_stor_مجموعه_max_xfer_blk(dev, ss);

	dev->privptr = (فراغ *)ss;
	أرجع 1;
}

صحيح usb_stor_get_info(بنية usb_device *dev, بنية us_داده *ss,
		      بنية blk_desc *dev_desc)
{
	بیعلامت حرف perq, modi;
	ALLOC_CACHE_ALIGN_میانگیر(u32, cap, 2);
	ALLOC_CACHE_ALIGN_میانگیر(u8, usb_stor_buf, 36);
	u32 ظرفیت, blksz;
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
	کپی_حافظه(dev_desc->vendor, (ثابت فراغ *)&usb_stor_buf[8], 8);
	کپی_حافظه(dev_desc->حاصلضرب, (ثابت فراغ *)&usb_stor_buf[16], 16);
	کپی_حافظه(dev_desc->revision, (ثابت فراغ *)&usb_stor_buf[32], 4);
	dev_desc->vendor[8] = 0;
	dev_desc->حاصلضرب[16] = 0;
	dev_desc->revision[4] = 0;
#اگر_تعریف_شده CONFIG_USB_BIN_FIXUP
	usb_bin_fixup(dev->descriptor, (uchar *)dev_desc->vendor,
		      (uchar *)dev_desc->حاصلضرب);
#پایان_اگر /* CONFIG_USB_BIN_FIXUP */
	debug("ISO Vers %X, Response Data %X\n", usb_stor_buf[2],
	      usb_stor_buf[3]);
	إذا (usb_test_unit_ready(pccb, ss)) {
		چاپ_کن("Device NOT ready\n"
		       "   Request Sense returned %02X %02X %02X\n",
		       pccb->sense_buf[2], pccb->sense_buf[12],
		       pccb->sense_buf[13]);
		إذا (dev_desc->removable == 1)
			dev_desc->type = perq;
		أرجع 0;
	}
	pccb->pdata = (بیعلامت حرف *)cap;
	memset(pccb->pdata, 0, 8);
	إذا (usb_read_ظرفیت(pccb, ss) != 0) {
		چاپ_کن("READ_CAP ERROR\n");
		ss->flags &= ~USB_READY;
		cap[0] = 2880;
		cap[1] = 0x200;
	}
	debug("Read Capacity returns: 0x%08x, 0x%08x\n", cap[0], cap[1]);
#إذا 0
	إذا (cap[0] > (0x200000 * 10)) /* greater than 10 GByte */
		cap[0] >>= 16;

	cap[0] = cpu_مقصد_be32(cap[0]);
	cap[1] = cpu_مقصد_be32(cap[1]);
#پایان_اگر

	ظرفیت = be32_مقصد_cpu(cap[0]) + 1;
	blksz = be32_مقصد_cpu(cap[1]);

	debug("Capacity = 0x%08x, blocksz = 0x%08x\n", ظرفیت, blksz);
	dev_desc->lba = ظرفیت;
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
	.از_تطابق = usb_mass_storage_ids,
	.probe = usb_mass_storage_probe,
#إذا CONFIG_IS_ENABLED(BLK)
	.plat_auto	= حجم(بنية us_داده),
#پایان_اگر
};

UCLASS_DRIVER(usb_mass_storage) = {
	.id		= UCLASS_MASS_STORAGE,
	.name		= "usb_mass_storage",
};

ساكن ثابت بنية usb_device_id mass_storage_id_جدول[] = {
	{
		.تطابق_flags = USB_DEVICE_ID_تطابق_INT_CLASS,
		.bInterfaceClass = USB_CLASS_MASS_STORAGE
	},
	{ }		/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_mass_storage, mass_storage_id_جدول);
#پایان_اگر

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
#پایان_اگر
