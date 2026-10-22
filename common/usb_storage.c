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

#অন্তর্ভাব <blk.h>
#অন্তর্ভাব <bootdev.h>
#অন্তর্ভাব <command.h>
#অন্তর্ভাব <dm.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <mapmem.h>
#অন্তর্ভাব <memalign.h>
#অন্তর্ভাব <asm/byteorder.h>
#অন্তর্ভাব <asm/cache.h>
#অন্তর্ভাব <asm/processor.h>
#অন্তর্ভাব <dm/device-internal.h>
#অন্তর্ভাব <dm/lists.h>
#অন্তর্ভাব <linux/delay.h>

#অন্তর্ভাব <part.h>
#অন্তর্ভাব <usb.h>

#undef BBB_COMDAT_TRACE
#undef BBB_XPORT_TRACE

#অন্তর্ভাব <scsi.h>
/* direction table -- this indicates the direction of the data
 * transfer for each command code -- a 1 indicates input
 */
স্থির ধ্রুবক unsigned অক্ষর us_direction[256/8] = {
	0x28, 0x81, 0x14, 0x14, 0x20, 0x01, 0x90, 0x77,
	0x0C, 0x20, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#define US_DIRECTION(x) ((us_direction[x>>3] >> (x & 7)) & 1)

স্থির কাঠামো scsi_cmd usb_ccb __aligned(ARCH_DMA_MINALIGN);
স্থির __u32 CBWTag;

স্থির পূর্ণসংখ্যা usb_max_devs; /* number of highest available usb device */

#যদি !CONFIG_IS_ENABLED(BLK)
স্থির কাঠামো blk_desc usb_dev_desc[USB_MAX_STOR_DEV];
#endif

কাঠামো us_তথ্য;
প্রকারলেখ পূর্ণসংখ্যা (*trans_cmnd)(কাঠামো scsi_cmd *cb, কাঠামো us_তথ্য *তথ্য);
প্রকারলেখ পূর্ণসংখ্যা (*trans_reset)(কাঠামো us_তথ্য *তথ্য);

কাঠামো us_তথ্য {
	কাঠামো usb_device *pusb_dev;	 /* this usb_device */

	unsigned পূর্ণসংখ্যা	flags;			/* from filter initially */
#	define USB_READY	(1 << 0)
	unsigned অক্ষর	ifnum;			/* interface number */
	unsigned অক্ষর	ep_in;			/* in endpoint */
	unsigned অক্ষর	ep_out;			/* out ....... */
	unsigned অক্ষর	ep_int;			/* interrupt . */
	unsigned অক্ষর	subclass;		/* as in overview */
	unsigned অক্ষর	protocol;		/* .............. */
	unsigned অক্ষর	attention_done;		/* force attn on first cmd */
	unsigned short	ip_তথ্য;		/* interrupt data */
	পূর্ণসংখ্যা		action;			/* what to do */
	পূর্ণসংখ্যা		ip_wanted;		/* needed */
	পূর্ণসংখ্যা		*irq_handle;		/* for USB int requests */
	unsigned পূর্ণসংখ্যা	irqpipe;		/* pipe for release_irq */
	unsigned অক্ষর	irqmaxp;		/* max packed for irq Pipe */
	unsigned অক্ষর	irqinterval;		/* Intervall for IRQ Pipe */
	কাঠামো scsi_cmd	*srb;			/* current srb */
	trans_reset	transport_reset;	/* reset routine */
	trans_cmnd	transport;		/* transport routine */
	unsigned short	max_xfer_blk;		/* maximum transfer blocks */
	bool		cmd12;			/* use 12-byte commands (RBC/UFI) */
};

#যদি !CONFIG_IS_ENABLED(BLK)
স্থির কাঠামো us_তথ্য usb_stor[USB_MAX_STOR_DEV];
#endif

#define USB_STOR_TRANSPORT_GOOD	   0
#define USB_STOR_TRANSPORT_FAILED -1
#define USB_STOR_TRANSPORT_ERROR  -2

পূর্ণসংখ্যা usb_stor_get_info(কাঠামো usb_device *dev, কাঠামো us_তথ্য *us,
		      কাঠামো blk_desc *dev_desc);
পূর্ণসংখ্যা usb_storage_probe(কাঠামো usb_device *dev, unsigned পূর্ণসংখ্যা ifnum,
		      কাঠামো us_তথ্য *ss);
#যদি CONFIG_IS_ENABLED(BLK)
স্থির unsigned long usb_stor_read(কাঠামো udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, শূন্য *বাফার);
স্থির unsigned long usb_stor_write(কাঠামো udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, ধ্রুবক শূন্য *বাফার);
#অন্যথা
স্থির unsigned long usb_stor_read(কাঠামো blk_desc *block_dev, lbaint_t blknr,
				   lbaint_t blkcnt, শূন্য *বাফার);
স্থির unsigned long usb_stor_write(কাঠামো blk_desc *block_dev, lbaint_t blknr,
				    lbaint_t blkcnt, ধ্রুবক শূন্য *বাফার);
#endif
শূন্য uhci_show_অস্থায়ী_int_td(শূন্য);

স্থির শূন্য usb_show_progress(শূন্য)
{
	debug(".");
}

/*******************************************************************************
 * show info on storage devices; 'usb start/init' must be invoked earlier
 * as we only retrieve structures populated during devices initialization
 */
পূর্ণসংখ্যা usb_stor_info(শূন্য)
{
	পূর্ণসংখ্যা গুনতি = 0;
#যদি CONFIG_IS_ENABLED(BLK)
	কাঠামো udevice *dev;

	জন্য (blk_first_device(UCLASS_USB, &dev);
	     dev;
	     blk_পরবর্তী_device(&dev)) {
		কাঠামো blk_desc *desc = dev_get_uclass_plat(dev);

		printf("  Device %d: ", desc->devnum);
		dev_print(desc);
		গুনতি++;
	}
#অন্যথা
	পূর্ণসংখ্যা i;

	যদি (usb_max_devs > 0) {
		জন্য (i = 0; i < usb_max_devs; i++) {
			printf("  Device %d: ", i);
			dev_print(&usb_dev_desc[i]);
		}
		ফেরত 0;
	}
#endif
	যদি (!গুনতি) {
		printf("No storage devices, perhaps not 'usb start'ed..?\n");
		ফেরত 1;
	}

	ফেরত 0;
}

স্থির unsigned পূর্ণসংখ্যা usb_get_max_lun(কাঠামো us_তথ্য *us)
{
	পূর্ণসংখ্যা len;
	ALLOC_CACHE_ALIGN_বাফার(unsigned অক্ষর, ফলাফল, 1);
	len = usb_control_বার্তা(us->pusb_dev,
			      usb_rcvctrlpipe(us->pusb_dev, 0),
			      US_BBB_GET_MAX_LUN,
			      USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_IN,
			      0, us->ifnum,
			      ফলাফল, আকার(অক্ষর),
			      USB_CNTL_TIMEOUT * 5);
	debug("Get Max LUN -> len = %i, result = %i\n", len, (পূর্ণসংখ্যা) *ফলাফল);
	ফেরত (len > 0) ? *ফলাফল : 0;
}

স্থির পূর্ণসংখ্যা usb_stor_probe_device(কাঠামো usb_device *udev)
{
	পূর্ণসংখ্যা lun, max_lun;

#যদি CONFIG_IS_ENABLED(BLK)
	কাঠামো us_তথ্য *তথ্য;
	পূর্ণসংখ্যা ret;
#অন্যথা
	পূর্ণসংখ্যা start;

	যদি (udev == ফাঁকা)
		ফেরত -ENOENT; /* no more devices available */
#endif

	debug("\n\nProbing for storage\n");
#যদি CONFIG_IS_ENABLED(BLK)
	/*
	 * We store the us_data in the mass storage device's plat. It
	 * is shared by all LUNs (block devices) attached to this mass storage
	 * device.
	 */
	তথ্য = dev_get_plat(udev->dev);
	যদি (!usb_storage_probe(udev, 0, তথ্য))
		ফেরত 0;
	max_lun = usb_get_max_lun(তথ্য);
	জন্য (lun = 0; lun <= max_lun; lun++) {
		কাঠামো blk_desc *blkdev;
		কাঠামো udevice *dev;
		অক্ষর str[10];

		snprintf(str, আকার(str), "lun%d", lun);
		ret = blk_create_devicef(udev->dev, "usb_storage_blk", str,
					 UCLASS_USB, usb_max_devs,
					 DEFAULT_BLKSZ, 0, &dev);
		যদি (ret) {
			debug("Cannot bind driver\n");
			ফেরত ret;
		}

		blkdev = dev_get_uclass_plat(dev);
		blkdev->target = 0xff;
		blkdev->lun = lun;

		ret = usb_stor_get_info(udev, তথ্য, blkdev);
		যদি (ret == 1) {
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		} অন্যথা {
			debug("usb_stor_get_info: Invalid device\n");
			ret = device_unbind(dev);
			যদি (ret)
				ফেরত ret;
			চালু;
		}

		ret = blk_probe_or_unbind(dev);
		যদি (ret)
			ফেরত ret;

		ret = bootdev_setup_for_sibling_blk(dev, "usb_bootdev");
		যদি (ret) {
			পূর্ণসংখ্যা ret2;

			ret2 = device_unbind(dev);
			যদি (ret2)
				ফেরত log_বার্তা_ret("bootdev", ret2);
			ফেরত log_বার্তা_ret("bootdev", ret);
		}
	}
#অন্যথা
	/* We don't have space to even probe if we hit the maximum */
	যদি (usb_max_devs == USB_MAX_STOR_DEV) {
		printf("max USB Storage Device reached: %d stopping\n",
		       usb_max_devs);
		ফেরত -ENOSPC;
	}

	যদি (!usb_storage_probe(udev, 0, &usb_stor[usb_max_devs]))
		ফেরত 0;

	/*
	 * OK, it's a storage device.  Iterate over its LUNs and populate
	 * usb_dev_desc'
	 */
	start = usb_max_devs;

	max_lun = usb_get_max_lun(&usb_stor[usb_max_devs]);
	জন্য (lun = 0; lun <= max_lun && usb_max_devs < USB_MAX_STOR_DEV;
	     lun++) {
		কাঠামো blk_desc *blkdev;

		blkdev = &usb_dev_desc[usb_max_devs];
		memset(blkdev, '\0', আকার(কাঠামো blk_desc));
		blkdev->uclass_id = UCLASS_USB;
		blkdev->devnum = usb_max_devs;
		blkdev->part_type = PART_TYPE_UNKNOWN;
		blkdev->target = 0xff;
		blkdev->type = DEV_TYPE_UNKNOWN;
		blkdev->block_read = usb_stor_read;
		blkdev->block_write = usb_stor_write;
		blkdev->lun = lun;
		blkdev->priv = udev;

		যদি (usb_stor_get_info(udev, &usb_stor[start],
				      &usb_dev_desc[usb_max_devs]) == 1) {
			debug("partype: %d\n", blkdev->part_type);
			part_init(blkdev);
			debug("partype: %d\n", blkdev->part_type);
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		}
	}
#endif

	ফেরত 0;
}

শূন্য usb_stor_reset(শূন্য)
{
	usb_max_devs = 0;
}

/*******************************************************************************
 * scan the usb and reports device info
 * to the user if mode = 1
 * returns current device or -1 if no
 */
পূর্ণসংখ্যা usb_stor_scan(পূর্ণসংখ্যা mode)
{
	যদি (mode == 1)
		printf("       scanning usb for storage devices... ");

#যদি !CONFIG_IS_ENABLED(DM_USB)
	unsigned অক্ষর i;

	usb_disable_asynch(1); /* asynch transfer not allowed */

	usb_stor_reset();
	জন্য (i = 0; i < USB_MAX_DEVICE; i++) {
		কাঠামো usb_device *dev;

		dev = usb_get_dev_সূচক(i); /* get device */
		debug("i=%d\n", i);
		যদি (usb_stor_probe_device(dev))
			বিরতি;
	} /* for */

	usb_disable_asynch(0); /* asynch transfer allowed */
#endif
	printf("%d Storage Device(s) found\n", usb_max_devs);
	যদি (usb_max_devs > 0)
		ফেরত 0;
	ফেরত -1;
}

স্থির পূর্ণসংখ্যা usb_stor_irq(কাঠামো usb_device *dev)
{
	কাঠামো us_তথ্য *us;
	us = (কাঠামো us_তথ্য *)dev->privptr;

	যদি (us->ip_wanted)
		us->ip_wanted = 0;
	ফেরত 0;
}

#ifdef	DEBUG

স্থির শূন্য usb_show_srb(কাঠামো scsi_cmd *pccb)
{
	পূর্ণসংখ্যা i;
	printf("SRB: len %d datalen 0x%lX\n ", pccb->cmdlen, pccb->datalen);
	জন্য (i = 0; i < pccb->cmdlen; i++)
		printf("%02X ", pccb->cmd[i]);
	printf("\n");
}

স্থির শূন্য display_int_status(unsigned long tmp)
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

স্থির পূর্ণসংখ্যা us_one_transfer(কাঠামো us_তথ্য *us, পূর্ণসংখ্যা pipe, অক্ষর *buf, পূর্ণসংখ্যা দৈর্ঘ্য)
{
	পূর্ণসংখ্যা max_আকার;
	পূর্ণসংখ্যা this_xfer;
	পূর্ণসংখ্যা ফলাফল;
	পূর্ণসংখ্যা partial;
	পূর্ণসংখ্যা maxtry;
	পূর্ণসংখ্যা stat;

	/* determine the maximum packet size for these transfers */
	max_আকার = usb_maxpacket(us->pusb_dev, pipe) * 16;

	/* while we have data left to transfer */
	যখন (দৈর্ঘ্য) {

		/* calculate how long this will be -- maximum or a remainder */
		this_xfer = দৈর্ঘ্য > max_আকার ? max_আকার : দৈর্ঘ্য;
		দৈর্ঘ্য -= this_xfer;

		/* setup the retry counter */
		maxtry = 10;

		/* set up the transfer loop */
		করো {
			/* transfer the data */
			debug("Bulk xfer 0x%lx(%d) try #%d\n",
			      (ulong)map_to_sysmem(buf), this_xfer,
			      11 - maxtry);
			ফলাফল = usb_bulk_বার্তা(us->pusb_dev, pipe, buf,
					      this_xfer, &partial,
					      USB_CNTL_TIMEOUT * 5);
			debug("bulk_msg returned %d xferred %d/%d\n",
			      ফলাফল, partial, this_xfer);
			যদি (us->pusb_dev->status != 0) {
				/* if we stall, we need to clear it before
				 * we go on
				 */
#ifdef DEBUG
				display_int_status(us->pusb_dev->status);
#endif
				যদি (us->pusb_dev->status & USB_ST_STALLED) {
					debug("stalled ->clearing endpoint" \
					      "halt for pipe 0x%x\n", pipe);
					stat = us->pusb_dev->status;
					usb_clear_halt(us->pusb_dev, pipe);
					us->pusb_dev->status = stat;
					যদি (this_xfer == partial) {
						debug("bulk transferred" \
						      "with error %lX," \
						      " but data ok\n",
						      us->pusb_dev->status);
						ফেরত 0;
					}
					অন্যথা
						ফেরত ফলাফল;
				}
				যদি (us->pusb_dev->status & USB_ST_NAK_REC) {
					debug("Device NAKed bulk_msg\n");
					ফেরত ফলাফল;
				}
				debug("bulk transferred with error");
				যদি (this_xfer == partial) {
					debug(" %ld, but data ok\n",
					      us->pusb_dev->status);
					ফেরত 0;
				}
				/* if our try counter reaches 0, bail out */
				debug(" %ld, data %d\n",
				      us->pusb_dev->status, partial);
				যদি (!maxtry--)
						ফেরত ফলাফল;
			}
			/* update to show what data was transferred */
			this_xfer -= partial;
			buf += partial;
			/* continue until this transfer is done */
		} যখন (this_xfer);
	}

	/* if we get here, we're done and successful */
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা usb_stor_BBB_reset(কাঠামো us_তথ্য *us)
{
	পূর্ণসংখ্যা ফলাফল;
	unsigned পূর্ণসংখ্যা pipe;

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
	ফলাফল = usb_control_বার্তা(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_BBB_RESET,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, ফাঁকা, 0, USB_CNTL_TIMEOUT * 5);

	যদি ((ফলাফল < 0) && (us->pusb_dev->status & USB_ST_STALLED)) {
		debug("RESET:stall\n");
		ফেরত -1;
	}

	/* long wait for reset */
	mdelay(150);
	debug("BBB_reset result %d: status %lX reset\n",
	      ফলাফল, us->pusb_dev->status);
	pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);
	ফলাফল = usb_clear_halt(us->pusb_dev, pipe);
	/* long wait for reset */
	mdelay(150);
	debug("BBB_reset result %d: status %lX clearing IN endpoint\n",
	      ফলাফল, us->pusb_dev->status);
	/* long wait for reset */
	pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_out);
	ফলাফল = usb_clear_halt(us->pusb_dev, pipe);
	mdelay(150);
	debug("BBB_reset result %d: status %lX clearing OUT endpoint\n",
	      ফলাফল, us->pusb_dev->status);
	debug("BBB_reset done\n");
	ফেরত 0;
}

/* FIXME: this reset function doesn't really reset the port, and it
 * should. Actually it should probably do what it's doing here, and
 * reset the port physically
 */
স্থির পূর্ণসংখ্যা usb_stor_CB_reset(কাঠামো us_তথ্য *us)
{
	unsigned অক্ষর cmd[12];
	পূর্ণসংখ্যা ফলাফল;

	debug("CB_reset\n");
	memset(cmd, 0xff, আকার(cmd));
	cmd[0] = SCSI_SEND_DIAG;
	cmd[1] = 4;
	ফলাফল = usb_control_বার্তা(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_CBI_ADSC,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, cmd, আকার(cmd),
				 USB_CNTL_TIMEOUT * 5);

	/* long wait for reset */
	mdelay(1500);
	debug("CB_reset result %d: status %lX clearing endpoint halt\n",
	      ফলাফল, us->pusb_dev->status);
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_in));
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_out));

	debug("CB_reset done\n");
	ফেরত 0;
}

/*
 * Set up the command for a BBB device. Note that the actual SCSI
 * command is copied into cbw.CBWCDB.
 */
স্থির পূর্ণসংখ্যা usb_stor_BBB_comdat(কাঠামো scsi_cmd *srb, কাঠামো us_তথ্য *us)
{
	পূর্ণসংখ্যা ফলাফল;
	পূর্ণসংখ্যা actlen;
	পূর্ণসংখ্যা dir_in;
	unsigned পূর্ণসংখ্যা pipe;
	ALLOC_CACHE_ALIGN_বাফার(কাঠামো umass_bbb_cbw, cbw, 1);

	dir_in = US_DIRECTION(srb->cmd[0]);

#ifdef BBB_COMDAT_TRACE
	printf("dir %d lun %d cmdlen %d cmd %p datalen %lu pdata %p\n",
		dir_in, srb->lun, srb->cmdlen, srb->cmd, srb->datalen,
		srb->pdata);
	যদি (srb->cmdlen) {
		জন্য (ফলাফল = 0; ফলাফল < srb->cmdlen; ফলাফল++)
			printf("cmd[%d] %#x ", ফলাফল, srb->cmd[ফলাফল]);
		printf("\n");
	}
#endif
	/* sanity checks */
	যদি (!(srb->cmdlen <= CBWCDBLENGTH)) {
		debug("usb_stor_BBB_comdat:cmdlen too large\n");
		ফেরত -1;
	}

	/* always OUT to the ep */
	pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_out);

	cbw->dCBWSignature = cpu_to_le32(CBWSIGNATURE);
	cbw->dCBWTag = cpu_to_le32(CBWTag++);
	cbw->d_CBWData_Transfer_দৈর্ঘ্য = cpu_to_le32(srb->datalen);
	cbw->bCBWFlags = (dir_in ? CBWFLAGS_IN : CBWFLAGS_OUT);
	cbw->bCBWLUN = srb->lun;
	cbw->bCDBLength = srb->cmdlen;
	/* copy the command data into the CBW command data buffer */
	/* DST SRC LEN!!! */

	memcpy(cbw->CBWCDB, srb->cmd, srb->cmdlen);
	ফলাফল = usb_bulk_বার্তা(us->pusb_dev, pipe, cbw, UMASS_BBB_CBW_আকার,
			      &actlen, USB_CNTL_TIMEOUT * 5);
	যদি (ফলাফল < 0)
		debug("usb_stor_BBB_comdat:usb_bulk_msg error\n");
	ফেরত ফলাফল;
}

/* FIXME: we also need a CBI_command which sets up the completion
 * interrupt, and waits for it
 */
স্থির পূর্ণসংখ্যা usb_stor_CB_comdat(কাঠামো scsi_cmd *srb, কাঠামো us_তথ্য *us)
{
	পূর্ণসংখ্যা ফলাফল = 0;
	পূর্ণসংখ্যা dir_in, retry;
	unsigned পূর্ণসংখ্যা pipe;
	unsigned long status;

	retry = 5;
	dir_in = US_DIRECTION(srb->cmd[0]);

	যদি (dir_in)
		pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);
	অন্যথা
		pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_out);

	যখন (retry--) {
		debug("CBI gets a command: Try %d\n", 5 - retry);
#ifdef DEBUG
		usb_show_srb(srb);
#endif
		/* let's send the command via the control pipe */
		ফলাফল = usb_control_বার্তা(us->pusb_dev,
					 usb_sndctrlpipe(us->pusb_dev , 0),
					 US_CBI_ADSC,
					 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
					 0, us->ifnum,
					 srb->cmd, srb->cmdlen,
					 USB_CNTL_TIMEOUT * 5);
		debug("CB_transport: control msg returned %d, status %lX\n",
		      ফলাফল, us->pusb_dev->status);
		/* check the return code for the command */
		যদি (ফলাফল < 0) {
			যদি (us->pusb_dev->status & USB_ST_STALLED) {
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
			ফেরত ফলাফল;
		}
		/* transfer the data payload for this command, if one exists*/

		debug("CB_transport: control msg returned %d," \
		      " direction is %s to go 0x%lx\n", ফলাফল,
		      dir_in ? "IN" : "OUT", srb->datalen);
		যদি (srb->datalen) {
			ফলাফল = us_one_transfer(us, pipe, (অক্ষর *)srb->pdata,
						 srb->datalen);
			debug("CBI attempted to transfer data," \
			      " result is %d status %lX, len %d\n",
			      ফলাফল, us->pusb_dev->status,
				us->pusb_dev->act_len);
			যদি (!(us->pusb_dev->status & USB_ST_NAK_REC))
				বিরতি;
		} /* if (srb->datalen) */
		অন্যথা
			বিরতি;
	}
	/* return result */

	ফেরত ফলাফল;
}

স্থির পূর্ণসংখ্যা usb_stor_CBI_get_status(কাঠামো scsi_cmd *srb, কাঠামো us_তথ্য *us)
{
	পূর্ণসংখ্যা timeout;

	us->ip_wanted = 1;
	usb_int_বার্তা(us->pusb_dev, us->irqpipe,
		    (শূন্য *)&us->ip_তথ্য, us->irqmaxp, us->irqinterval, false);
	timeout = 1000;
	যখন (timeout--) {
		যদি (us->ip_wanted == 0)
			বিরতি;
		mdelay(10);
	}
	যদি (us->ip_wanted) {
		printf("	Did not get interrupt on CBI\n");
		us->ip_wanted = 0;
		ফেরত USB_STOR_TRANSPORT_ERROR;
	}
	debug("Got interrupt data 0x%x, transferred %d status 0x%lX\n",
	      us->ip_তথ্য, us->pusb_dev->irq_act_len,
	      us->pusb_dev->irq_status);
	/* UFI gives us ASC and ASCQ, like a request sense */
	যদি (us->subclass == US_SC_UFI) {
		যদি (srb->cmd[0] == SCSI_REQ_SENSE ||
		    srb->cmd[0] == SCSI_INQUIRY)
			ফেরত USB_STOR_TRANSPORT_GOOD; /* Good */
		অন্যথা যদি (us->ip_তথ্য)
			ফেরত USB_STOR_TRANSPORT_FAILED;
		অন্যথা
			ফেরত USB_STOR_TRANSPORT_GOOD;
	}
	/* otherwise, we interpret the data normally */
	নির্বাচন (us->ip_তথ্য) {
	ক্ষেত্র 0x0001:
		ফেরত USB_STOR_TRANSPORT_GOOD;
	ক্ষেত্র 0x0002:
		ফেরত USB_STOR_TRANSPORT_FAILED;
	পূর্বনির্ধারিত:
		ফেরত USB_STOR_TRANSPORT_ERROR;
	}			/* switch */
	ফেরত USB_STOR_TRANSPORT_ERROR;
}

#define USB_TRANSPORT_UNKNOWN_RETRY 5
#define USB_TRANSPORT_NOT_READY_RETRY 10

/* clear a stall on an endpoint - special for BBB devices */
স্থির পূর্ণসংখ্যা usb_stor_BBB_clear_endpt_stall(কাঠামো us_তথ্য *us, __u8 endpt)
{
	/* ENDPOINT_HALT = 0, so set value to 0 */
	ফেরত usb_control_বার্তা(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
			       USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
			       endpt, ফাঁকা, 0, USB_CNTL_TIMEOUT * 5);
}

স্থির পূর্ণসংখ্যা usb_stor_BBB_transport(কাঠামো scsi_cmd *srb, কাঠামো us_তথ্য *us)
{
	পূর্ণসংখ্যা ফলাফল, retry;
	পূর্ণসংখ্যা dir_in;
	পূর্ণসংখ্যা actlen, তথ্য_actlen;
	unsigned পূর্ণসংখ্যা pipe, pipein, pipeout;
	ALLOC_CACHE_ALIGN_বাফার(কাঠামো umass_bbb_csw, csw, 1);
#ifdef BBB_XPORT_TRACE
	unsigned অক্ষর *ptr;
	পূর্ণসংখ্যা সূচক;
#endif

	dir_in = US_DIRECTION(srb->cmd[0]);

	/* COMMAND phase */
	debug("COMMAND phase\n");
	ফলাফল = usb_stor_BBB_comdat(srb, us);
	যদি (ফলাফল < 0) {
		debug("failed to send CBW status %ld\n",
		      us->pusb_dev->status);
		usb_stor_BBB_reset(us);
		ফেরত USB_STOR_TRANSPORT_FAILED;
	}
	যদি (!(us->flags & USB_READY))
		mdelay(5);
	pipein = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);
	pipeout = usb_sndbulkpipe(us->pusb_dev, us->ep_out);
	/* DATA phase + error handling */
	তথ্য_actlen = 0;
	/* no data, go immediately to the STATUS phase */
	যদি (srb->datalen == 0)
		যাও st;
	debug("DATA phase\n");
	যদি (dir_in)
		pipe = pipein;
	অন্যথা
		pipe = pipeout;

	ফলাফল = usb_bulk_বার্তা(us->pusb_dev, pipe, srb->pdata, srb->datalen,
			      &তথ্য_actlen, USB_CNTL_TIMEOUT * 5);
	/* special handling of STALL in DATA phase */
	যদি ((ফলাফল < 0) && (us->pusb_dev->status & USB_ST_STALLED)) {
		debug("DATA:stall\n");
		/* clear the STALL on the endpoint */
		ফলাফল = usb_stor_BBB_clear_endpt_stall(us,
					dir_in ? us->ep_in : us->ep_out);
		যদি (ফলাফল >= 0)
			/* continue on to STATUS phase */
			যাও st;
	}
	যদি (ফলাফল < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->status);
		usb_stor_BBB_reset(us);
		ফেরত USB_STOR_TRANSPORT_FAILED;
	}
#ifdef BBB_XPORT_TRACE
	জন্য (সূচক = 0; সূচক < তথ্য_actlen; সূচক++)
		printf("pdata[%d] %#x ", সূচক, srb->pdata[সূচক]);
	printf("\n");
#endif
	/* STATUS phase + error handling */
st:
	retry = 0;
again:
	debug("STATUS phase\n");
	ফলাফল = usb_bulk_বার্তা(us->pusb_dev, pipein, csw, UMASS_BBB_CSW_আকার,
				&actlen, USB_CNTL_TIMEOUT*5);

	/* special handling of STALL in STATUS phase */
	যদি ((ফলাফল < 0) && (retry < 1) &&
	    (us->pusb_dev->status & USB_ST_STALLED)) {
		debug("STATUS:stall\n");
		/* clear the STALL on the endpoint */
		ফলাফল = usb_stor_BBB_clear_endpt_stall(us, us->ep_in);
		যদি (ফলাফল >= 0 && (retry++ < 1))
			/* do a retry */
			যাও again;
	}
	যদি (ফলাফল < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->status);
		usb_stor_BBB_reset(us);
		ফেরত USB_STOR_TRANSPORT_FAILED;
	}
#ifdef BBB_XPORT_TRACE
	ptr = (unsigned অক্ষর *)csw;
	জন্য (সূচক = 0; সূচক < UMASS_BBB_CSW_আকার; সূচক++)
		printf("ptr[%d] %#x ", সূচক, ptr[সূচক]);
	printf("\n");
#endif
	/* misuse pipe to get the residue */
	pipe = le32_to_cpu(csw->dCSWDataResidue);
	যদি (pipe == 0 && srb->datalen != 0 && srb->datalen - তথ্য_actlen != 0)
		pipe = srb->datalen - তথ্য_actlen;
	যদি (CSWSIGNATURE != le32_to_cpu(csw->dCSWSignature)) {
		debug("!CSWSIGNATURE\n");
		usb_stor_BBB_reset(us);
		ফেরত USB_STOR_TRANSPORT_FAILED;
	} অন্যথা যদি ((CBWTag - 1) != le32_to_cpu(csw->dCSWTag)) {
		debug("!Tag\n");
		usb_stor_BBB_reset(us);
		ফেরত USB_STOR_TRANSPORT_FAILED;
	} অন্যথা যদি (csw->bCSWStatus > CSWSTATUS_PHASE) {
		debug(">PHASE\n");
		usb_stor_BBB_reset(us);
		ফেরত USB_STOR_TRANSPORT_FAILED;
	} অন্যথা যদি (csw->bCSWStatus == CSWSTATUS_PHASE) {
		debug("=PHASE\n");
		usb_stor_BBB_reset(us);
		ফেরত USB_STOR_TRANSPORT_FAILED;
	} অন্যথা যদি (তথ্য_actlen > srb->datalen) {
		debug("transferred %dB instead of %ldB\n",
		      তথ্য_actlen, srb->datalen);
		ফেরত USB_STOR_TRANSPORT_FAILED;
	} অন্যথা যদি (csw->bCSWStatus == CSWSTATUS_FAILED) {
		debug("FAILED\n");
		ফেরত USB_STOR_TRANSPORT_FAILED;
	}

	ফেরত ফলাফল;
}

স্থির পূর্ণসংখ্যা usb_stor_CB_transport(কাঠামো scsi_cmd *srb, কাঠামো us_তথ্য *us)
{
	পূর্ণসংখ্যা ফলাফল, status;
	কাঠামো scsi_cmd *psrb;
	কাঠামো scsi_cmd reqsrb;
	পূর্ণসংখ্যা retry, notready;

	psrb = &reqsrb;
	status = USB_STOR_TRANSPORT_GOOD;
	retry = 0;
	notready = 0;
	/* issue the command */
do_retry:
	ফলাফল = usb_stor_CB_comdat(srb, us);
	debug("command / Data returned %d, status %lX\n",
	      ফলাফল, us->pusb_dev->status);
	/* if this is an CBI Protocol, get IRQ */
	যদি (us->protocol == US_PR_CBI) {
		status = usb_stor_CBI_get_status(srb, us);
		/* if the status is error, report it */
		যদি (status == USB_STOR_TRANSPORT_ERROR) {
			debug(" USB CBI Command Error\n");
			ফেরত status;
		}
		srb->sense_buf[12] = (unsigned অক্ষর)(us->ip_তথ্য >> 8);
		srb->sense_buf[13] = (unsigned অক্ষর)(us->ip_তথ্য & 0xff);
		যদি (!us->ip_তথ্য) {
			/* if the status is good, report it */
			যদি (status == USB_STOR_TRANSPORT_GOOD) {
				debug(" USB CBI Command Good\n");
				ফেরত status;
			}
		}
	}
	/* do we have to issue an auto request? */
	/* HERE we have to check the result */
	যদি ((ফলাফল < 0) && !(us->pusb_dev->status & USB_ST_STALLED)) {
		debug("ERROR %lX\n", us->pusb_dev->status);
		us->transport_reset(us);
		ফেরত USB_STOR_TRANSPORT_ERROR;
	}
	যদি ((us->protocol == US_PR_CBI) &&
	    ((srb->cmd[0] == SCSI_REQ_SENSE) ||
	    (srb->cmd[0] == SCSI_INQUIRY))) {
		/* do not issue an autorequest after request sense */
		debug("No auto request and good\n");
		ফেরত USB_STOR_TRANSPORT_GOOD;
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
	ফলাফল = usb_stor_CB_comdat(psrb, us);
	debug("auto request returned %d\n", ফলাফল);
	/* if this is an CBI Protocol, get IRQ */
	যদি (us->protocol == US_PR_CBI)
		status = usb_stor_CBI_get_status(psrb, us);

	যদি ((ফলাফল < 0) && !(us->pusb_dev->status & USB_ST_STALLED)) {
		debug(" AUTO REQUEST ERROR %ld\n",
		      us->pusb_dev->status);
		ফেরত USB_STOR_TRANSPORT_ERROR;
	}
	debug("autorequest returned 0x%02X 0x%02X 0x%02X 0x%02X\n",
	      srb->sense_buf[0], srb->sense_buf[2],
	      srb->sense_buf[12], srb->sense_buf[13]);
	/* Check the auto request result */
	যদি ((srb->sense_buf[2] == 0) &&
	    (srb->sense_buf[12] == 0) &&
	    (srb->sense_buf[13] == 0)) {
		/* ok, no sense */
		ফেরত USB_STOR_TRANSPORT_GOOD;
	}

	/* Check the auto request result */
	নির্বাচন (srb->sense_buf[2]) {
	ক্ষেত্র 0x01:
		/* Recovered Error */
		ফেরত USB_STOR_TRANSPORT_GOOD;
		বিরতি;
	ক্ষেত্র 0x02:
		/* Not Ready */
		যদি (notready++ > USB_TRANSPORT_NOT_READY_RETRY) {
			printf("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X (NOT READY)\n", srb->cmd[0],
				srb->sense_buf[0], srb->sense_buf[2],
				srb->sense_buf[12], srb->sense_buf[13]);
			ফেরত USB_STOR_TRANSPORT_FAILED;
		} অন্যথা {
			mdelay(100);
			যাও do_retry;
		}
		বিরতি;
	পূর্বনির্ধারিত:
		যদি (retry++ > USB_TRANSPORT_UNKNOWN_RETRY) {
			printf("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X\n", srb->cmd[0], srb->sense_buf[0],
				srb->sense_buf[2], srb->sense_buf[12],
				srb->sense_buf[13]);
			ফেরত USB_STOR_TRANSPORT_FAILED;
		} অন্যথা
			যাও do_retry;
		বিরতি;
	}
	ফেরত USB_STOR_TRANSPORT_FAILED;
}

স্থির শূন্য usb_stor_set_max_xfer_blk(কাঠামো usb_device *udev,
				      কাঠামো us_তথ্য *us)
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

#যদি CONFIG_IS_ENABLED(DM_USB)
	size_t আকার_2;
	পূর্ণসংখ্যা ret;

	ret = usb_get_max_xfer_আকার(udev, (size_t *)&আকার_2);
	যদি ((ret >= 0) && (আকার_2 < blk * 512))
		blk = আকার_2 / 512;
#endif

	us->max_xfer_blk = blk;
}

স্থির পূর্ণসংখ্যা usb_inquiry(কাঠামো scsi_cmd *srb, কাঠামো us_তথ্য *ss)
{
	পূর্ণসংখ্যা retry, i;
	retry = 5;
	করো {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_INQUIRY;
		srb->cmd[1] = srb->lun << 5;
		srb->cmd[4] = 36;
		srb->datalen = 36;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		i = ss->transport(srb, ss);
		debug("inquiry returns %d\n", i);
		যদি (i == 0)
			বিরতি;
	} যখন (--retry);

	যদি (!retry) {
		printf("error in inquiry\n");
		ফেরত -1;
	}
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা usb_request_sense(কাঠামো scsi_cmd *srb, কাঠামো us_তথ্য *ss)
{
	অক্ষর *ptr;

	ptr = (অক্ষর *)srb->pdata;
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
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা usb_test_unit_ready(কাঠামো scsi_cmd *srb, কাঠামো us_তথ্য *ss)
{
	পূর্ণসংখ্যা retries = 10;

	করো {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_TST_U_RDY;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 0;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		যদি (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD) {
			ss->flags |= USB_READY;
			ফেরত 0;
		}
		usb_request_sense(srb, ss);
		/*
		 * Check the Key Code Qualifier, if it matches
		 * "Not Ready - medium not present"
		 * (the sense Key equals 0x2 and the ASC is 0x3a)
		 * return immediately as the medium being absent won't change
		 * unless there is a user action.
		 */
		যদি ((srb->sense_buf[2] == 0x02) &&
		    (srb->sense_buf[12] == 0x3a))
			ফেরত -1;
		mdelay(100);
	} যখন (retries--);

	ফেরত -1;
}

স্থির পূর্ণসংখ্যা usb_read_capacity(কাঠামো scsi_cmd *srb, কাঠামো us_তথ্য *ss)
{
	পূর্ণসংখ্যা retry;
	/* XXX retries */
	retry = 3;
	করো {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_RD_CAPAC;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 8;
		srb->cmdlen = ss->cmd12 ? 12 : 10;
		যদি (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD)
			ফেরত 0;
	} যখন (retry--);

	ফেরত -1;
}

স্থির পূর্ণসংখ্যা usb_read_10(কাঠামো scsi_cmd *srb, কাঠামো us_তথ্য *ss,
		       unsigned long start, unsigned short blocks)
{
	memset(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_READ10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((unsigned অক্ষর) (start >> 24)) & 0xff;
	srb->cmd[3] = ((unsigned অক্ষর) (start >> 16)) & 0xff;
	srb->cmd[4] = ((unsigned অক্ষর) (start >> 8)) & 0xff;
	srb->cmd[5] = ((unsigned অক্ষর) (start)) & 0xff;
	srb->cmd[7] = ((unsigned অক্ষর) (blocks >> 8)) & 0xff;
	srb->cmd[8] = (unsigned অক্ষর) blocks & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("read10: start %lx blocks %x\n", start, blocks);
	ফেরত ss->transport(srb, ss);
}

স্থির পূর্ণসংখ্যা usb_write_10(কাঠামো scsi_cmd *srb, কাঠামো us_তথ্য *ss,
			unsigned long start, unsigned short blocks)
{
	memset(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_WRITE10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((unsigned অক্ষর) (start >> 24)) & 0xff;
	srb->cmd[3] = ((unsigned অক্ষর) (start >> 16)) & 0xff;
	srb->cmd[4] = ((unsigned অক্ষর) (start >> 8)) & 0xff;
	srb->cmd[5] = ((unsigned অক্ষর) (start)) & 0xff;
	srb->cmd[7] = ((unsigned অক্ষর) (blocks >> 8)) & 0xff;
	srb->cmd[8] = (unsigned অক্ষর) blocks & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("write10: start %lx blocks %x\n", start, blocks);
	ফেরত ss->transport(srb, ss);
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
স্থির শূন্য usb_bin_fixup(কাঠামো usb_device_descriptor descriptor,
				unsigned অক্ষর vendor[],
				unsigned অক্ষর product[]) {
	ধ্রুবক unsigned অক্ষর max_vendor_len = 40;
	ধ্রুবক unsigned অক্ষর max_product_len = 20;
	যদি (descriptor.idVendor == 0x0424 && descriptor.idProduct == 0x223a) {
		strncpy((অক্ষর *)vendor, "SMSC", max_vendor_len);
		strncpy((অক্ষর *)product, "Flash Media Cntrller",
			max_product_len);
	}
}
#endif /* CONFIG_USB_BIN_FIXUP */

#যদি CONFIG_IS_ENABLED(BLK)
স্থির unsigned long usb_stor_read(কাঠামো udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, শূন্য *বাফার)
#অন্যথা
স্থির unsigned long usb_stor_read(কাঠামো blk_desc *block_dev, lbaint_t blknr,
				   lbaint_t blkcnt, শূন্য *বাফার)
#endif
{
	lbaint_t start, blks;
	uintptr_t buf_addr;
	unsigned short smallblks;
	কাঠামো usb_device *udev;
	কাঠামো us_তথ্য *ss;
	পূর্ণসংখ্যা retry;
	কাঠামো scsi_cmd *srb = &usb_ccb;
#যদি CONFIG_IS_ENABLED(BLK)
	কাঠামো blk_desc *block_dev;
#endif

	যদি (blkcnt == 0)
		ফেরত 0;
	/* Setup  device */
#যদি CONFIG_IS_ENABLED(BLK)
	block_dev = dev_get_uclass_plat(dev);
	udev = dev_get_parent_priv(dev_get_parent(dev));
	debug("\nusb_read: udev %d\n", block_dev->devnum);
#অন্যথা
	debug("\nusb_read: udev %d\n", block_dev->devnum);
	udev = usb_dev_desc[block_dev->devnum].priv;
	যদি (!udev) {
		debug("%s: No device\n", __func__);
		ফেরত 0;
	}
#endif
	ss = (কাঠামো us_তথ্য *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_lock_async(udev, 1);
	srb->lun = block_dev->lun;
	buf_addr = (uintptr_t)বাফার;
	start = blknr;
	blks = blkcnt;

	debug("\nusb_read: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      block_dev->devnum, start, blks, buf_addr);

	করো {
		/* XXX need some comment here */
		retry = 2;
		srb->pdata = (unsigned অক্ষর *)buf_addr;
		যদি (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		অন্যথা
			smallblks = (unsigned short) blks;
retry_it:
		যদি (smallblks == ss->max_xfer_blk)
			usb_show_progress();
		srb->datalen = block_dev->blksz * smallblks;
		srb->pdata = (unsigned অক্ষর *)buf_addr;
		যদি (usb_read_10(srb, ss, start, smallblks)) {
			debug("Read ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			যদি (retry--)
				যাও retry_it;
			blkcnt -= blks;
			বিরতি;
		}
		start += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} যখন (blks != 0);

	debug("usb_read: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      start, smallblks, buf_addr);

	usb_lock_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	যদি (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	ফেরত blkcnt;
}

#যদি CONFIG_IS_ENABLED(BLK)
স্থির unsigned long usb_stor_write(কাঠামো udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, ধ্রুবক শূন্য *বাফার)
#অন্যথা
স্থির unsigned long usb_stor_write(কাঠামো blk_desc *block_dev, lbaint_t blknr,
				    lbaint_t blkcnt, ধ্রুবক শূন্য *বাফার)
#endif
{
	lbaint_t start, blks;
	uintptr_t buf_addr;
	unsigned short smallblks;
	কাঠামো usb_device *udev;
	কাঠামো us_তথ্য *ss;
	পূর্ণসংখ্যা retry;
	কাঠামো scsi_cmd *srb = &usb_ccb;
#যদি CONFIG_IS_ENABLED(BLK)
	কাঠামো blk_desc *block_dev;
#endif

	যদি (blkcnt == 0)
		ফেরত 0;

	/* Setup  device */
#যদি CONFIG_IS_ENABLED(BLK)
	block_dev = dev_get_uclass_plat(dev);
	udev = dev_get_parent_priv(dev_get_parent(dev));
	debug("\nusb_read: udev %d\n", block_dev->devnum);
#অন্যথা
	debug("\nusb_read: udev %d\n", block_dev->devnum);
	udev = usb_dev_desc[block_dev->devnum].priv;
	যদি (!udev) {
		debug("%s: No device\n", __func__);
		ফেরত 0;
	}
#endif
	ss = (কাঠামো us_তথ্য *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_lock_async(udev, 1);

	srb->lun = block_dev->lun;
	buf_addr = (uintptr_t)বাফার;
	start = blknr;
	blks = blkcnt;

	debug("\nusb_write: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      block_dev->devnum, start, blks, buf_addr);

	করো {
		/* If write fails retry for max retry count else
		 * return with number of blocks written successfully.
		 */
		retry = 2;
		srb->pdata = (unsigned অক্ষর *)buf_addr;
		যদি (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		অন্যথা
			smallblks = (unsigned short) blks;
retry_it:
		যদি (smallblks == ss->max_xfer_blk)
			usb_show_progress();
		srb->datalen = block_dev->blksz * smallblks;
		srb->pdata = (unsigned অক্ষর *)buf_addr;
		যদি (usb_write_10(srb, ss, start, smallblks)) {
			debug("Write ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			যদি (retry--)
				যাও retry_it;
			blkcnt -= blks;
			বিরতি;
		}
		start += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} যখন (blks != 0);

	debug("usb_write: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      start, smallblks, buf_addr);

	usb_lock_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	যদি (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	ফেরত blkcnt;

}

/* Probe to see if a new device is actually a Storage device */
পূর্ণসংখ্যা usb_storage_probe(কাঠামো usb_device *dev, unsigned পূর্ণসংখ্যা ifnum,
		      কাঠামো us_তথ্য *ss)
{
	কাঠামো usb_interface *iface;
	পূর্ণসংখ্যা i;
	কাঠামো usb_endpoint_descriptor *ep_desc;
	unsigned পূর্ণসংখ্যা flags = 0;

	/* let's examine the device now */
	iface = &dev->config.if_desc[ifnum];

	যদি (dev->descriptor.bDeviceClass != 0 ||
			iface->desc.bInterfaceClass != USB_CLASS_MASS_STORAGE ||
			iface->desc.bInterfaceSubClass < US_SC_MIN ||
			iface->desc.bInterfaceSubClass > US_SC_MAX) {
		debug("Not mass storage\n");
		/* if it's not a mass storage, we go no further */
		ফেরত 0;
	}

	memset(ss, 0, আকার(কাঠামো us_তথ্য));

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
	নির্বাচন (ss->protocol) {
	ক্ষেত্র US_PR_CB:
		debug("Control/Bulk\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		বিরতি;

	ক্ষেত্র US_PR_CBI:
		debug("Control/Bulk/Interrupt\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		বিরতি;
	ক্ষেত্র US_PR_BULK:
		debug("Bulk/Bulk/Bulk\n");
		ss->transport = usb_stor_BBB_transport;
		ss->transport_reset = usb_stor_BBB_reset;
		বিরতি;
	পূর্বনির্ধারিত:
		printf("USB Storage Transport unknown / not yet implemented\n");
		ফেরত 0;
		বিরতি;
	}

	/*
	 * We are expecting a minimum of 2 endpoints - in and out (bulk).
	 * An optional interrupt is OK (necessary for CBI protocol).
	 * We will ignore any others.
	 */
	জন্য (i = 0; i < iface->desc.b_অঙ্ক_Endpoints; i++) {
		ep_desc = &iface->ep_desc[i];
		/* is it an BULK endpoint? */
		যদি ((ep_desc->bmAttributes &
		     USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK) {
			যদি (ep_desc->bEndpointAddress & USB_DIR_IN)
				ss->ep_in = ep_desc->bEndpointAddress &
						USB_ENDPOINT_সংখ্যা_MASK;
			অন্যথা
				ss->ep_out =
					ep_desc->bEndpointAddress &
					USB_ENDPOINT_সংখ্যা_MASK;
		}

		/* is it an interrupt endpoint? */
		যদি ((ep_desc->bmAttributes &
		     USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT) {
			ss->ep_int = ep_desc->bEndpointAddress &
						USB_ENDPOINT_সংখ্যা_MASK;
			ss->irqinterval = ep_desc->bInterval;
		}
	}
	debug("Endpoints In %d Out %d Int %d\n",
	      ss->ep_in, ss->ep_out, ss->ep_int);

	/* Do some basic sanity checks, and bail if we find a problem */
	যদি (usb_set_interface(dev, iface->desc.b_Interface_সংখ্যা, 0) ||
	    !ss->ep_in || !ss->ep_out ||
	    (ss->protocol == US_PR_CBI && ss->ep_int == 0)) {
		debug("Problems with device\n");
		ফেরত 0;
	}
	/* set class specific stuff */
	/* We only handle certain protocols.  Currently, these are
	 * the only ones.
	 * The SFF8070 accepts the requests used in u-boot
	 */
	যদি (ss->subclass != US_SC_UFI && ss->subclass != US_SC_SCSI &&
	    ss->subclass != US_SC_8070) {
		printf("Sorry, protocol %d not yet supported.\n", ss->subclass);
		ফেরত 0;
	}

	/* UFI uses 12-byte commands (like RBC, unlike SCSI) */
	যদি (ss->subclass == US_SC_UFI)
		ss->cmd12 = true;

	যদি (ss->ep_int) {
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

	dev->privptr = (শূন্য *)ss;
	ফেরত 1;
}

পূর্ণসংখ্যা usb_stor_get_info(কাঠামো usb_device *dev, কাঠামো us_তথ্য *ss,
		      কাঠামো blk_desc *dev_desc)
{
	unsigned অক্ষর perq, modi;
	ALLOC_CACHE_ALIGN_বাফার(u32, cap, 2);
	ALLOC_CACHE_ALIGN_বাফার(u8, usb_stor_buf, 36);
	u32 capacity, blksz;
	কাঠামো scsi_cmd *pccb = &usb_ccb;

	pccb->pdata = usb_stor_buf;

	dev_desc->target = dev->devnum;
	pccb->lun = dev_desc->lun;
	debug(" address %d\n", dev_desc->target);

	যদি (usb_inquiry(pccb, ss)) {
		debug("%s: usb_inquiry() failed\n", __func__);
		ফেরত -1;
	}

	perq = usb_stor_buf[0];
	modi = usb_stor_buf[1];

	/*
	 * Skip unknown devices (0x1f) and enclosure service devices (0x0d),
	 * they would not respond to test_unit_ready .
	 */
	যদি (((perq & 0x1f) == 0x1f) || ((perq & 0x1f) == 0x0d)) {
		debug("%s: unknown/unsupported device\n", __func__);
		ফেরত 0;
	}
	যদি ((modi&0x80) == 0x80) {
		/* drive is removable */
		dev_desc->removable = 1;
	}
	memcpy(dev_desc->vendor, (ধ্রুবক শূন্য *)&usb_stor_buf[8], 8);
	memcpy(dev_desc->product, (ধ্রুবক শূন্য *)&usb_stor_buf[16], 16);
	memcpy(dev_desc->revision, (ধ্রুবক শূন্য *)&usb_stor_buf[32], 4);
	dev_desc->vendor[8] = 0;
	dev_desc->product[16] = 0;
	dev_desc->revision[4] = 0;
#ifdef CONFIG_USB_BIN_FIXUP
	usb_bin_fixup(dev->descriptor, (uchar *)dev_desc->vendor,
		      (uchar *)dev_desc->product);
#endif /* CONFIG_USB_BIN_FIXUP */
	debug("ISO Vers %X, Response Data %X\n", usb_stor_buf[2],
	      usb_stor_buf[3]);
	যদি (usb_test_unit_ready(pccb, ss)) {
		printf("Device NOT ready\n"
		       "   Request Sense returned %02X %02X %02X\n",
		       pccb->sense_buf[2], pccb->sense_buf[12],
		       pccb->sense_buf[13]);
		যদি (dev_desc->removable == 1)
			dev_desc->type = perq;
		ফেরত 0;
	}
	pccb->pdata = (unsigned অক্ষর *)cap;
	memset(pccb->pdata, 0, 8);
	যদি (usb_read_capacity(pccb, ss) != 0) {
		printf("READ_CAP ERROR\n");
		ss->flags &= ~USB_READY;
		cap[0] = 2880;
		cap[1] = 0x200;
	}
	debug("Read Capacity returns: 0x%08x, 0x%08x\n", cap[0], cap[1]);
#যদি 0
	যদি (cap[0] > (0x200000 * 10)) /* greater than 10 GByte */
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

	ফেরত 1;
}

#যদি CONFIG_IS_ENABLED(DM_USB)

স্থির পূর্ণসংখ্যা usb_mass_storage_probe(কাঠামো udevice *dev)
{
	কাঠামো usb_device *udev = dev_get_parent_priv(dev);
	পূর্ণসংখ্যা ret;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	ret = usb_stor_probe_device(udev);
	usb_disable_asynch(0); /* asynch transfer allowed */

	ফেরত ret;
}

স্থির ধ্রুবক কাঠামো udevice_id usb_mass_storage_ids[] = {
	{ .compatible = "usb-mass-storage" },
	{ }
};

U_BOOT_DRIVER(usb_mass_storage) = {
	.name	= "usb_mass_storage",
	.id	= UCLASS_MASS_STORAGE,
	.of_match = usb_mass_storage_ids,
	.probe = usb_mass_storage_probe,
#যদি CONFIG_IS_ENABLED(BLK)
	.plat_auto	= আকার(কাঠামো us_তথ্য),
#endif
};

UCLASS_DRIVER(usb_mass_storage) = {
	.id		= UCLASS_MASS_STORAGE,
	.name		= "usb_mass_storage",
};

স্থির ধ্রুবক কাঠামো usb_device_id mass_storage_id_table[] = {
	{
		.match_flags = USB_DEVICE_ID_MATCH_INT_CLASS,
		.bInterfaceClass = USB_CLASS_MASS_STORAGE
	},
	{ }		/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_mass_storage, mass_storage_id_table);
#endif

#যদি CONFIG_IS_ENABLED(BLK)
স্থির ধ্রুবক কাঠামো blk_ops usb_storage_ops = {
	.পঠ	= usb_stor_read,
	.লিখ	= usb_stor_write,
};

U_BOOT_DRIVER(usb_storage_blk) = {
	.name		= "usb_storage_blk",
	.id		= UCLASS_BLK,
	.ops		= &usb_storage_ops,
};
#অন্যথা
U_BOOT_LEGACY_BLK(usb) = {
	.uclass_idname	= "usb",
	.uclass_id	= UCLASS_USB,
	.max_devs	= USB_MAX_STOR_DEV,
	.desc		= usb_dev_desc,
};
#endif
