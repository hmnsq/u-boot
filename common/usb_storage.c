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

#ଅନ୍ତର୍ଭାଵ <blk.h>
#ଅନ୍ତର୍ଭାଵ <bootdev.h>
#ଅନ୍ତର୍ଭାଵ <command.h>
#ଅନ୍ତର୍ଭାଵ <dm.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <mapmem.h>
#ଅନ୍ତର୍ଭାଵ <memalign.h>
#ଅନ୍ତର୍ଭାଵ <asm/byteorder.h>
#ଅନ୍ତର୍ଭାଵ <asm/cache.h>
#ଅନ୍ତର୍ଭାଵ <asm/processor.h>
#ଅନ୍ତର୍ଭାଵ <dm/device-internal.h>
#ଅନ୍ତର୍ଭାଵ <dm/lists.h>
#ଅନ୍ତର୍ଭାଵ <linux/delay.h>

#ଅନ୍ତର୍ଭାଵ <part.h>
#ଅନ୍ତର୍ଭାଵ <usb.h>

#undef BBB_COMDAT_TRACE
#undef BBB_XPORT_TRACE

#ଅନ୍ତର୍ଭାଵ <scsi.h>
/* direction table -- this indicates the direction of the data
 * transfer for each command code -- a 1 indicates input
 */
ସ୍ଥିର ଅଚଳ unsigned ଅକ୍ଷର us_direction[256/8] = {
	0x28, 0x81, 0x14, 0x14, 0x20, 0x01, 0x90, 0x77,
	0x0C, 0x20, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#define US_DIRECTION(x) ((us_direction[x>>3] >> (x & 7)) & 1)

ସ୍ଥିର ସଂରଚନା scsi_cmd usb_ccb __aligned(ARCH_DMA_MINALIGN);
ସ୍ଥିର __u32 CBWTag;

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_max_devs; /* number of highest available usb device */

#ଯଦି !CONFIG_IS_ENABLED(BLK)
ସ୍ଥିର ସଂରଚନା blk_desc usb_dev_desc[USB_MAX_STOR_DEV];
#endif

ସଂରଚନା us_data;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*trans_cmnd)(ସଂରଚନା scsi_cmd *cb, ସଂରଚନା us_data *data);
ପ୍ରକାରବ୍ୟାଖ୍ୟା ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*trans_reset)(ସଂରଚନା us_data *data);

ସଂରଚନା us_data {
	ସଂରଚନା usb_device *pusb_dev;	 /* this usb_device */

	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା	flags;			/* from filter initially */
#	define USB_READY	(1 << 0)
	unsigned ଅକ୍ଷର	ifnum;			/* interface number */
	unsigned ଅକ୍ଷର	ep_in;			/* in endpoint */
	unsigned ଅକ୍ଷର	ep_out;			/* out ....... */
	unsigned ଅକ୍ଷର	ep_int;			/* interrupt . */
	unsigned ଅକ୍ଷର	subclass;		/* as in overview */
	unsigned ଅକ୍ଷର	protocol;		/* .............. */
	unsigned ଅକ୍ଷର	attention_done;		/* force attn on first cmd */
	unsigned short	ip_data;		/* interrupt data */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା		action;			/* what to do */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା		ip_wanted;		/* needed */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା		*irq_handle;		/* for USB int requests */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା	irqpipe;		/* pipe for release_irq */
	unsigned ଅକ୍ଷର	irqmaxp;		/* max packed for irq Pipe */
	unsigned ଅକ୍ଷର	irqinterval;		/* Intervall for IRQ Pipe */
	ସଂରଚନା scsi_cmd	*srb;			/* current srb */
	trans_reset	transport_reset;	/* reset routine */
	trans_cmnd	transport;		/* transport routine */
	unsigned short	max_xfer_blk;		/* maximum transfer blocks */
	bool		cmd12;			/* use 12-byte commands (RBC/UFI) */
};

#ଯଦି !CONFIG_IS_ENABLED(BLK)
ସ୍ଥିର ସଂରଚନା us_data usb_stor[USB_MAX_STOR_DEV];
#endif

#define USB_STOR_TRANSPORT_GOOD	   0
#define USB_STOR_TRANSPORT_FAILED -1
#define USB_STOR_TRANSPORT_ERROR  -2

ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_stor_get_info(ସଂରଚନା usb_device *dev, ସଂରଚନା us_data *us,
		      ସଂରଚନା blk_desc *dev_desc);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_storage_probe(ସଂରଚନା usb_device *dev, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ifnum,
		      ସଂରଚନା us_data *ss);
#ଯଦି CONFIG_IS_ENABLED(BLK)
ସ୍ଥିର unsigned long usb_stor_read(ସଂରଚନା udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, ଶୂନ୍ୟ *buffer);
ସ୍ଥିର unsigned long usb_stor_write(ସଂରଚନା udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, ଅଚଳ ଶୂନ୍ୟ *buffer);
#ଅନ୍ୟଥା
ସ୍ଥିର unsigned long usb_stor_read(ସଂରଚନା blk_desc *block_dev, lbaint_t blknr,
				   lbaint_t blkcnt, ଶୂନ୍ୟ *buffer);
ସ୍ଥିର unsigned long usb_stor_write(ସଂରଚନା blk_desc *block_dev, lbaint_t blknr,
				    lbaint_t blkcnt, ଅଚଳ ଶୂନ୍ୟ *buffer);
#endif
ଶୂନ୍ୟ uhci_show_temp_int_td(ଶୂନ୍ୟ);

ସ୍ଥିର ଶୂନ୍ୟ usb_show_progress(ଶୂନ୍ୟ)
{
	debug(".");
}

/*******************************************************************************
 * show info on storage devices; 'usb start/init' must be invoked earlier
 * as we only retrieve structures populated during devices initialization
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_stor_info(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = 0;
#ଯଦି CONFIG_IS_ENABLED(BLK)
	ସଂରଚନା udevice *dev;

	ପାଇଁ (blk_first_device(UCLASS_USB, &dev);
	     dev;
	     blk_next_device(&dev)) {
		ସଂରଚନା blk_desc *desc = dev_get_uclass_plat(dev);

		printf("  Device %d: ", desc->devnum);
		dev_print(desc);
		ଗଣନା_2++;
	}
#ଅନ୍ୟଥା
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ଯଦି (usb_max_devs > 0) {
		ପାଇଁ (i = 0; i < usb_max_devs; i++) {
			printf("  Device %d: ", i);
			dev_print(&usb_dev_desc[i]);
		}
		ଫେରନ୍ତୁ 0;
	}
#endif
	ଯଦି (!ଗଣନା_2) {
		printf("No storage devices, perhaps not 'usb start'ed..?\n");
		ଫେରନ୍ତୁ 1;
	}

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_get_max_lun(ସଂରଚନା us_data *us)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned ଅକ୍ଷର, result, 1);
	len = usb_control_ସନ୍ଦେଶ(us->pusb_dev,
			      usb_rcvctrlpipe(us->pusb_dev, 0),
			      US_BBB_GET_MAX_LUN,
			      USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_IN,
			      0, us->ifnum,
			      result, ଆକାର(ଅକ୍ଷର),
			      USB_CNTL_TIMEOUT * 5);
	debug("Get Max LUN -> len = %i, result = %i\n", len, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) *result);
	ଫେରନ୍ତୁ (len > 0) ? *result : 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_stor_probe_device(ସଂରଚନା usb_device *udev)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା lun, max_lun;

#ଯଦି CONFIG_IS_ENABLED(BLK)
	ସଂରଚନା us_data *data;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
#ଅନ୍ୟଥା
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା start;

	ଯଦି (udev == ଖାଲି)
		ଫେରନ୍ତୁ -ENOENT; /* no more devices available */
#endif

	debug("\n\nProbing for storage\n");
#ଯଦି CONFIG_IS_ENABLED(BLK)
	/*
	 * We store the us_data in the mass storage device's plat. It
	 * is shared by all LUNs (block devices) attached to this mass storage
	 * device.
	 */
	data = dev_get_plat(udev->dev);
	ଯଦି (!usb_storage_probe(udev, 0, data))
		ଫେରନ୍ତୁ 0;
	max_lun = usb_get_max_lun(data);
	ପାଇଁ (lun = 0; lun <= max_lun; lun++) {
		ସଂରଚନା blk_desc *blkdev;
		ସଂରଚନା udevice *dev;
		ଅକ୍ଷର str[10];

		snprintf(str, ଆକାର(str), "lun%d", lun);
		ret = blk_create_devicef(udev->dev, "usb_storage_blk", str,
					 UCLASS_USB, usb_max_devs,
					 DEFAULT_BLKSZ, 0, &dev);
		ଯଦି (ret) {
			debug("Cannot bind driver\n");
			ଫେରନ୍ତୁ ret;
		}

		blkdev = dev_get_uclass_plat(dev);
		blkdev->target = 0xff;
		blkdev->lun = lun;

		ret = usb_stor_get_info(udev, data, blkdev);
		ଯଦି (ret == 1) {
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		} ଅନ୍ୟଥା {
			debug("usb_stor_get_info: Invalid device\n");
			ret = device_unbind(dev);
			ଯଦି (ret)
				ଫେରନ୍ତୁ ret;
			ଜାରି;
		}

		ret = blk_probe_or_unbind(dev);
		ଯଦି (ret)
			ଫେରନ୍ତୁ ret;

		ret = bootdev_setup_for_sibling_blk(dev, "usb_bootdev");
		ଯଦି (ret) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret2;

			ret2 = device_unbind(dev);
			ଯଦି (ret2)
				ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("bootdev", ret2);
			ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("bootdev", ret);
		}
	}
#ଅନ୍ୟଥା
	/* We don't have space to even probe if we hit the maximum */
	ଯଦି (usb_max_devs == USB_MAX_STOR_DEV) {
		printf("max USB Storage Device reached: %d stopping\n",
		       usb_max_devs);
		ଫେରନ୍ତୁ -ENOSPC;
	}

	ଯଦି (!usb_storage_probe(udev, 0, &usb_stor[usb_max_devs]))
		ଫେରନ୍ତୁ 0;

	/*
	 * OK, it's a storage device.  Iterate over its LUNs and populate
	 * usb_dev_desc'
	 */
	start = usb_max_devs;

	max_lun = usb_get_max_lun(&usb_stor[usb_max_devs]);
	ପାଇଁ (lun = 0; lun <= max_lun && usb_max_devs < USB_MAX_STOR_DEV;
	     lun++) {
		ସଂରଚନା blk_desc *blkdev;

		blkdev = &usb_dev_desc[usb_max_devs];
		memset(blkdev, '\0', ଆକାର(ସଂରଚନା blk_desc));
		blkdev->uclass_id = UCLASS_USB;
		blkdev->devnum = usb_max_devs;
		blkdev->part_type = PART_TYPE_UNKNOWN;
		blkdev->target = 0xff;
		blkdev->type = DEV_TYPE_UNKNOWN;
		blkdev->block_read = usb_stor_read;
		blkdev->block_write = usb_stor_write;
		blkdev->lun = lun;
		blkdev->priv = udev;

		ଯଦି (usb_stor_get_info(udev, &usb_stor[start],
				      &usb_dev_desc[usb_max_devs]) == 1) {
			debug("partype: %d\n", blkdev->part_type);
			part_init(blkdev);
			debug("partype: %d\n", blkdev->part_type);
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		}
	}
#endif

	ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ usb_stor_reset(ଶୂନ୍ୟ)
{
	usb_max_devs = 0;
}

/*******************************************************************************
 * scan the usb and reports device info
 * to the user if mode = 1
 * returns current device or -1 if no
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_stor_scan(ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode)
{
	ଯଦି (mode == 1)
		printf("       scanning usb for storage devices... ");

#ଯଦି !CONFIG_IS_ENABLED(DM_USB)
	unsigned ଅକ୍ଷର i;

	usb_disable_asynch(1); /* asynch transfer not allowed */

	usb_stor_reset();
	ପାଇଁ (i = 0; i < USB_MAX_DEVICE; i++) {
		ସଂରଚନା usb_device *dev;

		dev = usb_get_dev_index(i); /* get device */
		debug("i=%d\n", i);
		ଯଦି (usb_stor_probe_device(dev))
			ଭାଙ୍ଗ;
	} /* for */

	usb_disable_asynch(0); /* asynch transfer allowed */
#endif
	printf("%d Storage Device(s) found\n", usb_max_devs);
	ଯଦି (usb_max_devs > 0)
		ଫେରନ୍ତୁ 0;
	ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_stor_irq(ସଂରଚନା usb_device *dev)
{
	ସଂରଚନା us_data *us;
	us = (ସଂରଚନା us_data *)dev->privptr;

	ଯଦି (us->ip_wanted)
		us->ip_wanted = 0;
	ଫେରନ୍ତୁ 0;
}

#ifdef	DEBUG

ସ୍ଥିର ଶୂନ୍ୟ usb_show_srb(ସଂରଚନା scsi_cmd *pccb)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	printf("SRB: len %d datalen 0x%lX\n ", pccb->cmdlen, pccb->datalen);
	ପାଇଁ (i = 0; i < pccb->cmdlen; i++)
		printf("%02X ", pccb->cmd[i]);
	printf("\n");
}

ସ୍ଥିର ଶୂନ୍ୟ display_int_status(unsigned long tmp)
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

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା us_one_transfer(ସଂରଚନା us_data *us, ପୂର୍ଣ୍ଣସଂଖ୍ୟା pipe, ଅକ୍ଷର *buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା length)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_size;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା this_xfer;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା result;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା partial;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxtry;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା stat;

	/* determine the maximum packet size for these transfers */
	max_size = usb_maxpacket(us->pusb_dev, pipe) * 16;

	/* while we have data left to transfer */
	ଯେତେବେଳେ (length) {

		/* calculate how long this will be -- maximum or a remainder */
		this_xfer = length > max_size ? max_size : length;
		length -= this_xfer;

		/* setup the retry counter */
		maxtry = 10;

		/* set up the transfer loop */
		କର {
			/* transfer the data */
			debug("Bulk xfer 0x%lx(%d) try #%d\n",
			      (ulong)map_to_sysmem(buf), this_xfer,
			      11 - maxtry);
			result = usb_bulk_ସନ୍ଦେଶ(us->pusb_dev, pipe, buf,
					      this_xfer, &partial,
					      USB_CNTL_TIMEOUT * 5);
			debug("bulk_msg returned %d xferred %d/%d\n",
			      result, partial, this_xfer);
			ଯଦି (us->pusb_dev->status != 0) {
				/* if we stall, we need to clear it before
				 * we go on
				 */
#ifdef DEBUG
				display_int_status(us->pusb_dev->status);
#endif
				ଯଦି (us->pusb_dev->status & USB_ST_STALLED) {
					debug("stalled ->clearing endpoint" \
					      "halt for pipe 0x%x\n", pipe);
					stat = us->pusb_dev->status;
					usb_clear_halt(us->pusb_dev, pipe);
					us->pusb_dev->status = stat;
					ଯଦି (this_xfer == partial) {
						debug("bulk transferred" \
						      "with error %lX," \
						      " but data ok\n",
						      us->pusb_dev->status);
						ଫେରନ୍ତୁ 0;
					}
					ଅନ୍ୟଥା
						ଫେରନ୍ତୁ result;
				}
				ଯଦି (us->pusb_dev->status & USB_ST_NAK_REC) {
					debug("Device NAKed bulk_msg\n");
					ଫେରନ୍ତୁ result;
				}
				debug("bulk transferred with error");
				ଯଦି (this_xfer == partial) {
					debug(" %ld, but data ok\n",
					      us->pusb_dev->status);
					ଫେରନ୍ତୁ 0;
				}
				/* if our try counter reaches 0, bail out */
				debug(" %ld, data %d\n",
				      us->pusb_dev->status, partial);
				ଯଦି (!maxtry--)
						ଫେରନ୍ତୁ result;
			}
			/* update to show what data was transferred */
			this_xfer -= partial;
			buf += partial;
			/* continue until this transfer is done */
		} ଯେତେବେଳେ (this_xfer);
	}

	/* if we get here, we're done and successful */
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_stor_BBB_reset(ସଂରଚନା us_data *us)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା result;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା pipe;

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
	result = usb_control_ସନ୍ଦେଶ(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_BBB_RESET,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, ଖାଲି, 0, USB_CNTL_TIMEOUT * 5);

	ଯଦି ((result < 0) && (us->pusb_dev->status & USB_ST_STALLED)) {
		debug("RESET:stall\n");
		ଫେରନ୍ତୁ -1;
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
	ଫେରନ୍ତୁ 0;
}

/* FIXME: this reset function doesn't really reset the port, and it
 * should. Actually it should probably do what it's doing here, and
 * reset the port physically
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_stor_CB_reset(ସଂରଚନା us_data *us)
{
	unsigned ଅକ୍ଷର cmd[12];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା result;

	debug("CB_reset\n");
	memset(cmd, 0xff, ଆକାର(cmd));
	cmd[0] = SCSI_SEND_DIAG;
	cmd[1] = 4;
	result = usb_control_ସନ୍ଦେଶ(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_CBI_ADSC,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, cmd, ଆକାର(cmd),
				 USB_CNTL_TIMEOUT * 5);

	/* long wait for reset */
	mdelay(1500);
	debug("CB_reset result %d: status %lX clearing endpoint halt\n",
	      result, us->pusb_dev->status);
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_in));
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_out));

	debug("CB_reset done\n");
	ଫେରନ୍ତୁ 0;
}

/*
 * Set up the command for a BBB device. Note that the actual SCSI
 * command is copied into cbw.CBWCDB.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_stor_BBB_comdat(ସଂରଚନା scsi_cmd *srb, ସଂରଚନା us_data *us)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା result;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା actlen;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir_in;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା pipe;
	ALLOC_CACHE_ALIGN_BUFFER(ସଂରଚନା umass_bbb_cbw, cbw, 1);

	dir_in = US_DIRECTION(srb->cmd[0]);

#ifdef BBB_COMDAT_TRACE
	printf("dir %d lun %d cmdlen %d cmd %p datalen %lu pdata %p\n",
		dir_in, srb->lun, srb->cmdlen, srb->cmd, srb->datalen,
		srb->pdata);
	ଯଦି (srb->cmdlen) {
		ପାଇଁ (result = 0; result < srb->cmdlen; result++)
			printf("cmd[%d] %#x ", result, srb->cmd[result]);
		printf("\n");
	}
#endif
	/* sanity checks */
	ଯଦି (!(srb->cmdlen <= CBWCDBLENGTH)) {
		debug("usb_stor_BBB_comdat:cmdlen too large\n");
		ଫେରନ୍ତୁ -1;
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
	result = usb_bulk_ସନ୍ଦେଶ(us->pusb_dev, pipe, cbw, UMASS_BBB_CBW_SIZE,
			      &actlen, USB_CNTL_TIMEOUT * 5);
	ଯଦି (result < 0)
		debug("usb_stor_BBB_comdat:usb_bulk_msg error\n");
	ଫେରନ୍ତୁ result;
}

/* FIXME: we also need a CBI_command which sets up the completion
 * interrupt, and waits for it
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_stor_CB_comdat(ସଂରଚନା scsi_cmd *srb, ସଂରଚନା us_data *us)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା result = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir_in, retry;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା pipe;
	unsigned long status;

	retry = 5;
	dir_in = US_DIRECTION(srb->cmd[0]);

	ଯଦି (dir_in)
		pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);
	ଅନ୍ୟଥା
		pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_out);

	ଯେତେବେଳେ (retry--) {
		debug("CBI gets a command: Try %d\n", 5 - retry);
#ifdef DEBUG
		usb_show_srb(srb);
#endif
		/* let's send the command via the control pipe */
		result = usb_control_ସନ୍ଦେଶ(us->pusb_dev,
					 usb_sndctrlpipe(us->pusb_dev , 0),
					 US_CBI_ADSC,
					 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
					 0, us->ifnum,
					 srb->cmd, srb->cmdlen,
					 USB_CNTL_TIMEOUT * 5);
		debug("CB_transport: control msg returned %d, status %lX\n",
		      result, us->pusb_dev->status);
		/* check the return code for the command */
		ଯଦି (result < 0) {
			ଯଦି (us->pusb_dev->status & USB_ST_STALLED) {
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
			ଫେରନ୍ତୁ result;
		}
		/* transfer the data payload for this command, if one exists*/

		debug("CB_transport: control msg returned %d," \
		      " direction is %s to go 0x%lx\n", result,
		      dir_in ? "IN" : "OUT", srb->datalen);
		ଯଦି (srb->datalen) {
			result = us_one_transfer(us, pipe, (ଅକ୍ଷର *)srb->pdata,
						 srb->datalen);
			debug("CBI attempted to transfer data," \
			      " result is %d status %lX, len %d\n",
			      result, us->pusb_dev->status,
				us->pusb_dev->act_len);
			ଯଦି (!(us->pusb_dev->status & USB_ST_NAK_REC))
				ଭାଙ୍ଗ;
		} /* if (srb->datalen) */
		ଅନ୍ୟଥା
			ଭାଙ୍ଗ;
	}
	/* return result */

	ଫେରନ୍ତୁ result;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_stor_CBI_get_status(ସଂରଚନା scsi_cmd *srb, ସଂରଚନା us_data *us)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା timeout;

	us->ip_wanted = 1;
	usb_int_ସନ୍ଦେଶ(us->pusb_dev, us->irqpipe,
		    (ଶୂନ୍ୟ *)&us->ip_data, us->irqmaxp, us->irqinterval, false);
	timeout = 1000;
	ଯେତେବେଳେ (timeout--) {
		ଯଦି (us->ip_wanted == 0)
			ଭାଙ୍ଗ;
		mdelay(10);
	}
	ଯଦି (us->ip_wanted) {
		printf("	Did not get interrupt on CBI\n");
		us->ip_wanted = 0;
		ଫେରନ୍ତୁ USB_STOR_TRANSPORT_ERROR;
	}
	debug("Got interrupt data 0x%x, transferred %d status 0x%lX\n",
	      us->ip_data, us->pusb_dev->irq_act_len,
	      us->pusb_dev->irq_status);
	/* UFI gives us ASC and ASCQ, like a request sense */
	ଯଦି (us->subclass == US_SC_UFI) {
		ଯଦି (srb->cmd[0] == SCSI_REQ_SENSE ||
		    srb->cmd[0] == SCSI_INQUIRY)
			ଫେରନ୍ତୁ USB_STOR_TRANSPORT_GOOD; /* Good */
		ଅନ୍ୟଥା ଯଦି (us->ip_data)
			ଫେରନ୍ତୁ USB_STOR_TRANSPORT_FAILED;
		ଅନ୍ୟଥା
			ଫେରନ୍ତୁ USB_STOR_TRANSPORT_GOOD;
	}
	/* otherwise, we interpret the data normally */
	ବାଛ (us->ip_data) {
	ସ୍ଥିତି 0x0001:
		ଫେରନ୍ତୁ USB_STOR_TRANSPORT_GOOD;
	ସ୍ଥିତି 0x0002:
		ଫେରନ୍ତୁ USB_STOR_TRANSPORT_FAILED;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଫେରନ୍ତୁ USB_STOR_TRANSPORT_ERROR;
	}			/* switch */
	ଫେରନ୍ତୁ USB_STOR_TRANSPORT_ERROR;
}

#define USB_TRANSPORT_UNKNOWN_RETRY 5
#define USB_TRANSPORT_NOT_READY_RETRY 10

/* clear a stall on an endpoint - special for BBB devices */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_stor_BBB_clear_endpt_stall(ସଂରଚନା us_data *us, __u8 endpt)
{
	/* ENDPOINT_HALT = 0, so set value to 0 */
	ଫେରନ୍ତୁ usb_control_ସନ୍ଦେଶ(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
			       USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
			       endpt, ଖାଲି, 0, USB_CNTL_TIMEOUT * 5);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_stor_BBB_transport(ସଂରଚନା scsi_cmd *srb, ସଂରଚନା us_data *us)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା result, retry;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir_in;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା actlen, data_actlen;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା pipe, pipein, pipeout;
	ALLOC_CACHE_ALIGN_BUFFER(ସଂରଚନା umass_bbb_csw, csw, 1);
#ifdef BBB_XPORT_TRACE
	unsigned ଅକ୍ଷର *ptr;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା index;
#endif

	dir_in = US_DIRECTION(srb->cmd[0]);

	/* COMMAND phase */
	debug("COMMAND phase\n");
	result = usb_stor_BBB_comdat(srb, us);
	ଯଦି (result < 0) {
		debug("failed to send CBW status %ld\n",
		      us->pusb_dev->status);
		usb_stor_BBB_reset(us);
		ଫେରନ୍ତୁ USB_STOR_TRANSPORT_FAILED;
	}
	ଯଦି (!(us->flags & USB_READY))
		mdelay(5);
	pipein = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);
	pipeout = usb_sndbulkpipe(us->pusb_dev, us->ep_out);
	/* DATA phase + error handling */
	data_actlen = 0;
	/* no data, go immediately to the STATUS phase */
	ଯଦି (srb->datalen == 0)
		ଯାଅ st;
	debug("DATA phase\n");
	ଯଦି (dir_in)
		pipe = pipein;
	ଅନ୍ୟଥା
		pipe = pipeout;

	result = usb_bulk_ସନ୍ଦେଶ(us->pusb_dev, pipe, srb->pdata, srb->datalen,
			      &data_actlen, USB_CNTL_TIMEOUT * 5);
	/* special handling of STALL in DATA phase */
	ଯଦି ((result < 0) && (us->pusb_dev->status & USB_ST_STALLED)) {
		debug("DATA:stall\n");
		/* clear the STALL on the endpoint */
		result = usb_stor_BBB_clear_endpt_stall(us,
					dir_in ? us->ep_in : us->ep_out);
		ଯଦି (result >= 0)
			/* continue on to STATUS phase */
			ଯାଅ st;
	}
	ଯଦି (result < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->status);
		usb_stor_BBB_reset(us);
		ଫେରନ୍ତୁ USB_STOR_TRANSPORT_FAILED;
	}
#ifdef BBB_XPORT_TRACE
	ପାଇଁ (index = 0; index < data_actlen; index++)
		printf("pdata[%d] %#x ", index, srb->pdata[index]);
	printf("\n");
#endif
	/* STATUS phase + error handling */
st:
	retry = 0;
again:
	debug("STATUS phase\n");
	result = usb_bulk_ସନ୍ଦେଶ(us->pusb_dev, pipein, csw, UMASS_BBB_CSW_SIZE,
				&actlen, USB_CNTL_TIMEOUT*5);

	/* special handling of STALL in STATUS phase */
	ଯଦି ((result < 0) && (retry < 1) &&
	    (us->pusb_dev->status & USB_ST_STALLED)) {
		debug("STATUS:stall\n");
		/* clear the STALL on the endpoint */
		result = usb_stor_BBB_clear_endpt_stall(us, us->ep_in);
		ଯଦି (result >= 0 && (retry++ < 1))
			/* do a retry */
			ଯାଅ again;
	}
	ଯଦି (result < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->status);
		usb_stor_BBB_reset(us);
		ଫେରନ୍ତୁ USB_STOR_TRANSPORT_FAILED;
	}
#ifdef BBB_XPORT_TRACE
	ptr = (unsigned ଅକ୍ଷର *)csw;
	ପାଇଁ (index = 0; index < UMASS_BBB_CSW_SIZE; index++)
		printf("ptr[%d] %#x ", index, ptr[index]);
	printf("\n");
#endif
	/* misuse pipe to get the residue */
	pipe = le32_to_cpu(csw->dCSWDataResidue);
	ଯଦି (pipe == 0 && srb->datalen != 0 && srb->datalen - data_actlen != 0)
		pipe = srb->datalen - data_actlen;
	ଯଦି (CSWSIGNATURE != le32_to_cpu(csw->dCSWSignature)) {
		debug("!CSWSIGNATURE\n");
		usb_stor_BBB_reset(us);
		ଫେରନ୍ତୁ USB_STOR_TRANSPORT_FAILED;
	} ଅନ୍ୟଥା ଯଦି ((CBWTag - 1) != le32_to_cpu(csw->dCSWTag)) {
		debug("!Tag\n");
		usb_stor_BBB_reset(us);
		ଫେରନ୍ତୁ USB_STOR_TRANSPORT_FAILED;
	} ଅନ୍ୟଥା ଯଦି (csw->bCSWStatus > CSWSTATUS_PHASE) {
		debug(">PHASE\n");
		usb_stor_BBB_reset(us);
		ଫେରନ୍ତୁ USB_STOR_TRANSPORT_FAILED;
	} ଅନ୍ୟଥା ଯଦି (csw->bCSWStatus == CSWSTATUS_PHASE) {
		debug("=PHASE\n");
		usb_stor_BBB_reset(us);
		ଫେରନ୍ତୁ USB_STOR_TRANSPORT_FAILED;
	} ଅନ୍ୟଥା ଯଦି (data_actlen > srb->datalen) {
		debug("transferred %dB instead of %ldB\n",
		      data_actlen, srb->datalen);
		ଫେରନ୍ତୁ USB_STOR_TRANSPORT_FAILED;
	} ଅନ୍ୟଥା ଯଦି (csw->bCSWStatus == CSWSTATUS_FAILED) {
		debug("FAILED\n");
		ଫେରନ୍ତୁ USB_STOR_TRANSPORT_FAILED;
	}

	ଫେରନ୍ତୁ result;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_stor_CB_transport(ସଂରଚନା scsi_cmd *srb, ସଂରଚନା us_data *us)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା result, status;
	ସଂରଚନା scsi_cmd *psrb;
	ସଂରଚନା scsi_cmd reqsrb;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retry, notready;

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
	ଯଦି (us->protocol == US_PR_CBI) {
		status = usb_stor_CBI_get_status(srb, us);
		/* if the status is error, report it */
		ଯଦି (status == USB_STOR_TRANSPORT_ERROR) {
			debug(" USB CBI Command Error\n");
			ଫେରନ୍ତୁ status;
		}
		srb->sense_buf[12] = (unsigned ଅକ୍ଷର)(us->ip_data >> 8);
		srb->sense_buf[13] = (unsigned ଅକ୍ଷର)(us->ip_data & 0xff);
		ଯଦି (!us->ip_data) {
			/* if the status is good, report it */
			ଯଦି (status == USB_STOR_TRANSPORT_GOOD) {
				debug(" USB CBI Command Good\n");
				ଫେରନ୍ତୁ status;
			}
		}
	}
	/* do we have to issue an auto request? */
	/* HERE we have to check the result */
	ଯଦି ((result < 0) && !(us->pusb_dev->status & USB_ST_STALLED)) {
		debug("ERROR %lX\n", us->pusb_dev->status);
		us->transport_reset(us);
		ଫେରନ୍ତୁ USB_STOR_TRANSPORT_ERROR;
	}
	ଯଦି ((us->protocol == US_PR_CBI) &&
	    ((srb->cmd[0] == SCSI_REQ_SENSE) ||
	    (srb->cmd[0] == SCSI_INQUIRY))) {
		/* do not issue an autorequest after request sense */
		debug("No auto request and good\n");
		ଫେରନ୍ତୁ USB_STOR_TRANSPORT_GOOD;
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
	ଯଦି (us->protocol == US_PR_CBI)
		status = usb_stor_CBI_get_status(psrb, us);

	ଯଦି ((result < 0) && !(us->pusb_dev->status & USB_ST_STALLED)) {
		debug(" AUTO REQUEST ERROR %ld\n",
		      us->pusb_dev->status);
		ଫେରନ୍ତୁ USB_STOR_TRANSPORT_ERROR;
	}
	debug("autorequest returned 0x%02X 0x%02X 0x%02X 0x%02X\n",
	      srb->sense_buf[0], srb->sense_buf[2],
	      srb->sense_buf[12], srb->sense_buf[13]);
	/* Check the auto request result */
	ଯଦି ((srb->sense_buf[2] == 0) &&
	    (srb->sense_buf[12] == 0) &&
	    (srb->sense_buf[13] == 0)) {
		/* ok, no sense */
		ଫେରନ୍ତୁ USB_STOR_TRANSPORT_GOOD;
	}

	/* Check the auto request result */
	ବାଛ (srb->sense_buf[2]) {
	ସ୍ଥିତି 0x01:
		/* Recovered Error */
		ଫେରନ୍ତୁ USB_STOR_TRANSPORT_GOOD;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି 0x02:
		/* Not Ready */
		ଯଦି (notready++ > USB_TRANSPORT_NOT_READY_RETRY) {
			printf("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X (NOT READY)\n", srb->cmd[0],
				srb->sense_buf[0], srb->sense_buf[2],
				srb->sense_buf[12], srb->sense_buf[13]);
			ଫେରନ୍ତୁ USB_STOR_TRANSPORT_FAILED;
		} ଅନ୍ୟଥା {
			mdelay(100);
			ଯାଅ do_retry;
		}
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଯଦି (retry++ > USB_TRANSPORT_UNKNOWN_RETRY) {
			printf("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X\n", srb->cmd[0], srb->sense_buf[0],
				srb->sense_buf[2], srb->sense_buf[12],
				srb->sense_buf[13]);
			ଫେରନ୍ତୁ USB_STOR_TRANSPORT_FAILED;
		} ଅନ୍ୟଥା
			ଯାଅ do_retry;
		ଭାଙ୍ଗ;
	}
	ଫେରନ୍ତୁ USB_STOR_TRANSPORT_FAILED;
}

ସ୍ଥିର ଶୂନ୍ୟ usb_stor_set_max_xfer_blk(ସଂରଚନା usb_device *udev,
				      ସଂରଚନା us_data *us)
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

#ଯଦି CONFIG_IS_ENABLED(DM_USB)
	size_t size;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ret = usb_get_max_xfer_size(udev, (size_t *)&size);
	ଯଦି ((ret >= 0) && (size < blk * 512))
		blk = size / 512;
#endif

	us->max_xfer_blk = blk;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_inquiry(ସଂରଚନା scsi_cmd *srb, ସଂରଚନା us_data *ss)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retry, i;
	retry = 5;
	କର {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_INQUIRY;
		srb->cmd[1] = srb->lun << 5;
		srb->cmd[4] = 36;
		srb->datalen = 36;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		i = ss->transport(srb, ss);
		debug("inquiry returns %d\n", i);
		ଯଦି (i == 0)
			ଭାଙ୍ଗ;
	} ଯେତେବେଳେ (--retry);

	ଯଦି (!retry) {
		printf("error in inquiry\n");
		ଫେରନ୍ତୁ -1;
	}
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_request_sense(ସଂରଚନା scsi_cmd *srb, ସଂରଚନା us_data *ss)
{
	ଅକ୍ଷର *ptr;

	ptr = (ଅକ୍ଷର *)srb->pdata;
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
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_test_unit_ready(ସଂରଚନା scsi_cmd *srb, ସଂରଚନା us_data *ss)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retries = 10;

	କର {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_TST_U_RDY;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 0;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		ଯଦି (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD) {
			ss->flags |= USB_READY;
			ଫେରନ୍ତୁ 0;
		}
		usb_request_sense(srb, ss);
		/*
		 * Check the Key Code Qualifier, if it matches
		 * "Not Ready - medium not present"
		 * (the sense Key equals 0x2 and the ASC is 0x3a)
		 * return immediately as the medium being absent won't change
		 * unless there is a user action.
		 */
		ଯଦି ((srb->sense_buf[2] == 0x02) &&
		    (srb->sense_buf[12] == 0x3a))
			ଫେରନ୍ତୁ -1;
		mdelay(100);
	} ଯେତେବେଳେ (retries--);

	ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_read_capacity(ସଂରଚନା scsi_cmd *srb, ସଂରଚନା us_data *ss)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retry;
	/* XXX retries */
	retry = 3;
	କର {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_RD_CAPAC;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 8;
		srb->cmdlen = ss->cmd12 ? 12 : 10;
		ଯଦି (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD)
			ଫେରନ୍ତୁ 0;
	} ଯେତେବେଳେ (retry--);

	ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_read_10(ସଂରଚନା scsi_cmd *srb, ସଂରଚନା us_data *ss,
		       unsigned long start, unsigned short blocks)
{
	memset(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_READ10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((unsigned ଅକ୍ଷର) (start >> 24)) & 0xff;
	srb->cmd[3] = ((unsigned ଅକ୍ଷର) (start >> 16)) & 0xff;
	srb->cmd[4] = ((unsigned ଅକ୍ଷର) (start >> 8)) & 0xff;
	srb->cmd[5] = ((unsigned ଅକ୍ଷର) (start)) & 0xff;
	srb->cmd[7] = ((unsigned ଅକ୍ଷର) (blocks >> 8)) & 0xff;
	srb->cmd[8] = (unsigned ଅକ୍ଷର) blocks & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("read10: start %lx blocks %x\n", start, blocks);
	ଫେରନ୍ତୁ ss->transport(srb, ss);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_write_10(ସଂରଚନା scsi_cmd *srb, ସଂରଚନା us_data *ss,
			unsigned long start, unsigned short blocks)
{
	memset(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_WRITE10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((unsigned ଅକ୍ଷର) (start >> 24)) & 0xff;
	srb->cmd[3] = ((unsigned ଅକ୍ଷର) (start >> 16)) & 0xff;
	srb->cmd[4] = ((unsigned ଅକ୍ଷର) (start >> 8)) & 0xff;
	srb->cmd[5] = ((unsigned ଅକ୍ଷର) (start)) & 0xff;
	srb->cmd[7] = ((unsigned ଅକ୍ଷର) (blocks >> 8)) & 0xff;
	srb->cmd[8] = (unsigned ଅକ୍ଷର) blocks & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("write10: start %lx blocks %x\n", start, blocks);
	ଫେରନ୍ତୁ ss->transport(srb, ss);
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
ସ୍ଥିର ଶୂନ୍ୟ usb_bin_fixup(ସଂରଚନା usb_device_descriptor descriptor,
				unsigned ଅକ୍ଷର vendor[],
				unsigned ଅକ୍ଷର product[]) {
	ଅଚଳ unsigned ଅକ୍ଷର max_vendor_len = 40;
	ଅଚଳ unsigned ଅକ୍ଷର max_product_len = 20;
	ଯଦି (descriptor.idVendor == 0x0424 && descriptor.idProduct == 0x223a) {
		strncpy((ଅକ୍ଷର *)vendor, "SMSC", max_vendor_len);
		strncpy((ଅକ୍ଷର *)product, "Flash Media Cntrller",
			max_product_len);
	}
}
#endif /* CONFIG_USB_BIN_FIXUP */

#ଯଦି CONFIG_IS_ENABLED(BLK)
ସ୍ଥିର unsigned long usb_stor_read(ସଂରଚନା udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, ଶୂନ୍ୟ *buffer)
#ଅନ୍ୟଥା
ସ୍ଥିର unsigned long usb_stor_read(ସଂରଚନା blk_desc *block_dev, lbaint_t blknr,
				   lbaint_t blkcnt, ଶୂନ୍ୟ *buffer)
#endif
{
	lbaint_t start, blks;
	uintptr_t buf_addr;
	unsigned short smallblks;
	ସଂରଚନା usb_device *udev;
	ସଂରଚନା us_data *ss;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retry;
	ସଂରଚନା scsi_cmd *srb = &usb_ccb;
#ଯଦି CONFIG_IS_ENABLED(BLK)
	ସଂରଚନା blk_desc *block_dev;
#endif

	ଯଦି (blkcnt == 0)
		ଫେରନ୍ତୁ 0;
	/* Setup  device */
#ଯଦି CONFIG_IS_ENABLED(BLK)
	block_dev = dev_get_uclass_plat(dev);
	udev = dev_get_parent_priv(dev_get_parent(dev));
	debug("\nusb_read: udev %d\n", block_dev->devnum);
#ଅନ୍ୟଥା
	debug("\nusb_read: udev %d\n", block_dev->devnum);
	udev = usb_dev_desc[block_dev->devnum].priv;
	ଯଦି (!udev) {
		debug("%s: No device\n", __func__);
		ଫେରନ୍ତୁ 0;
	}
#endif
	ss = (ସଂରଚନା us_data *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_lock_async(udev, 1);
	srb->lun = block_dev->lun;
	buf_addr = (uintptr_t)buffer;
	start = blknr;
	blks = blkcnt;

	debug("\nusb_read: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      block_dev->devnum, start, blks, buf_addr);

	କର {
		/* XXX need some comment here */
		retry = 2;
		srb->pdata = (unsigned ଅକ୍ଷର *)buf_addr;
		ଯଦି (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		ଅନ୍ୟଥା
			smallblks = (unsigned short) blks;
retry_it:
		ଯଦି (smallblks == ss->max_xfer_blk)
			usb_show_progress();
		srb->datalen = block_dev->blksz * smallblks;
		srb->pdata = (unsigned ଅକ୍ଷର *)buf_addr;
		ଯଦି (usb_read_10(srb, ss, start, smallblks)) {
			debug("Read ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			ଯଦି (retry--)
				ଯାଅ retry_it;
			blkcnt -= blks;
			ଭାଙ୍ଗ;
		}
		start += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} ଯେତେବେଳେ (blks != 0);

	debug("usb_read: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      start, smallblks, buf_addr);

	usb_lock_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	ଯଦି (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	ଫେରନ୍ତୁ blkcnt;
}

#ଯଦି CONFIG_IS_ENABLED(BLK)
ସ୍ଥିର unsigned long usb_stor_write(ସଂରଚନା udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, ଅଚଳ ଶୂନ୍ୟ *buffer)
#ଅନ୍ୟଥା
ସ୍ଥିର unsigned long usb_stor_write(ସଂରଚନା blk_desc *block_dev, lbaint_t blknr,
				    lbaint_t blkcnt, ଅଚଳ ଶୂନ୍ୟ *buffer)
#endif
{
	lbaint_t start, blks;
	uintptr_t buf_addr;
	unsigned short smallblks;
	ସଂରଚନା usb_device *udev;
	ସଂରଚନା us_data *ss;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retry;
	ସଂରଚନା scsi_cmd *srb = &usb_ccb;
#ଯଦି CONFIG_IS_ENABLED(BLK)
	ସଂରଚନା blk_desc *block_dev;
#endif

	ଯଦି (blkcnt == 0)
		ଫେରନ୍ତୁ 0;

	/* Setup  device */
#ଯଦି CONFIG_IS_ENABLED(BLK)
	block_dev = dev_get_uclass_plat(dev);
	udev = dev_get_parent_priv(dev_get_parent(dev));
	debug("\nusb_read: udev %d\n", block_dev->devnum);
#ଅନ୍ୟଥା
	debug("\nusb_read: udev %d\n", block_dev->devnum);
	udev = usb_dev_desc[block_dev->devnum].priv;
	ଯଦି (!udev) {
		debug("%s: No device\n", __func__);
		ଫେରନ୍ତୁ 0;
	}
#endif
	ss = (ସଂରଚନା us_data *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_lock_async(udev, 1);

	srb->lun = block_dev->lun;
	buf_addr = (uintptr_t)buffer;
	start = blknr;
	blks = blkcnt;

	debug("\nusb_write: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      block_dev->devnum, start, blks, buf_addr);

	କର {
		/* If write fails retry for max retry count else
		 * return with number of blocks written successfully.
		 */
		retry = 2;
		srb->pdata = (unsigned ଅକ୍ଷର *)buf_addr;
		ଯଦି (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		ଅନ୍ୟଥା
			smallblks = (unsigned short) blks;
retry_it:
		ଯଦି (smallblks == ss->max_xfer_blk)
			usb_show_progress();
		srb->datalen = block_dev->blksz * smallblks;
		srb->pdata = (unsigned ଅକ୍ଷର *)buf_addr;
		ଯଦି (usb_write_10(srb, ss, start, smallblks)) {
			debug("Write ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			ଯଦି (retry--)
				ଯାଅ retry_it;
			blkcnt -= blks;
			ଭାଙ୍ଗ;
		}
		start += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} ଯେତେବେଳେ (blks != 0);

	debug("usb_write: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      start, smallblks, buf_addr);

	usb_lock_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	ଯଦି (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	ଫେରନ୍ତୁ blkcnt;

}

/* Probe to see if a new device is actually a Storage device */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_storage_probe(ସଂରଚନା usb_device *dev, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ifnum,
		      ସଂରଚନା us_data *ss)
{
	ସଂରଚନା usb_interface *iface;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ସଂରଚନା usb_endpoint_descriptor *ep_desc;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = 0;

	/* let's examine the device now */
	iface = &dev->config.if_desc[ifnum];

	ଯଦି (dev->descriptor.bDeviceClass != 0 ||
			iface->desc.bInterfaceClass != USB_CLASS_MASS_STORAGE ||
			iface->desc.bInterfaceSubClass < US_SC_MIN ||
			iface->desc.bInterfaceSubClass > US_SC_MAX) {
		debug("Not mass storage\n");
		/* if it's not a mass storage, we go no further */
		ଫେରନ୍ତୁ 0;
	}

	memset(ss, 0, ଆକାର(ସଂରଚନା us_data));

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
	ବାଛ (ss->protocol) {
	ସ୍ଥିତି US_PR_CB:
		debug("Control/Bulk\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		ଭାଙ୍ଗ;

	ସ୍ଥିତି US_PR_CBI:
		debug("Control/Bulk/Interrupt\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି US_PR_BULK:
		debug("Bulk/Bulk/Bulk\n");
		ss->transport = usb_stor_BBB_transport;
		ss->transport_reset = usb_stor_BBB_reset;
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		printf("USB Storage Transport unknown / not yet implemented\n");
		ଫେରନ୍ତୁ 0;
		ଭାଙ୍ଗ;
	}

	/*
	 * We are expecting a minimum of 2 endpoints - in and out (bulk).
	 * An optional interrupt is OK (necessary for CBI protocol).
	 * We will ignore any others.
	 */
	ପାଇଁ (i = 0; i < iface->desc.b_ଅଙ୍କ_Endpoints; i++) {
		ep_desc = &iface->ep_desc[i];
		/* is it an BULK endpoint? */
		ଯଦି ((ep_desc->bmAttributes &
		     USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK) {
			ଯଦି (ep_desc->bEndpointAddress & USB_DIR_IN)
				ss->ep_in = ep_desc->bEndpointAddress &
						USB_ENDPOINT_ସଂଖ୍ୟା_MASK;
			ଅନ୍ୟଥା
				ss->ep_out =
					ep_desc->bEndpointAddress &
					USB_ENDPOINT_ସଂଖ୍ୟା_MASK;
		}

		/* is it an interrupt endpoint? */
		ଯଦି ((ep_desc->bmAttributes &
		     USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT) {
			ss->ep_int = ep_desc->bEndpointAddress &
						USB_ENDPOINT_ସଂଖ୍ୟା_MASK;
			ss->irqinterval = ep_desc->bInterval;
		}
	}
	debug("Endpoints In %d Out %d Int %d\n",
	      ss->ep_in, ss->ep_out, ss->ep_int);

	/* Do some basic sanity checks, and bail if we find a problem */
	ଯଦି (usb_set_interface(dev, iface->desc.b_Interface_ସଂଖ୍ୟା, 0) ||
	    !ss->ep_in || !ss->ep_out ||
	    (ss->protocol == US_PR_CBI && ss->ep_int == 0)) {
		debug("Problems with device\n");
		ଫେରନ୍ତୁ 0;
	}
	/* set class specific stuff */
	/* We only handle certain protocols.  Currently, these are
	 * the only ones.
	 * The SFF8070 accepts the requests used in u-boot
	 */
	ଯଦି (ss->subclass != US_SC_UFI && ss->subclass != US_SC_SCSI &&
	    ss->subclass != US_SC_8070) {
		printf("Sorry, protocol %d not yet supported.\n", ss->subclass);
		ଫେରନ୍ତୁ 0;
	}

	/* UFI uses 12-byte commands (like RBC, unlike SCSI) */
	ଯଦି (ss->subclass == US_SC_UFI)
		ss->cmd12 = true;

	ଯଦି (ss->ep_int) {
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

	dev->privptr = (ଶୂନ୍ୟ *)ss;
	ଫେରନ୍ତୁ 1;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_stor_get_info(ସଂରଚନା usb_device *dev, ସଂରଚନା us_data *ss,
		      ସଂରଚନା blk_desc *dev_desc)
{
	unsigned ଅକ୍ଷର perq, modi;
	ALLOC_CACHE_ALIGN_BUFFER(u32, cap, 2);
	ALLOC_CACHE_ALIGN_BUFFER(u8, usb_stor_buf, 36);
	u32 capacity, blksz;
	ସଂରଚନା scsi_cmd *pccb = &usb_ccb;

	pccb->pdata = usb_stor_buf;

	dev_desc->target = dev->devnum;
	pccb->lun = dev_desc->lun;
	debug(" address %d\n", dev_desc->target);

	ଯଦି (usb_inquiry(pccb, ss)) {
		debug("%s: usb_inquiry() failed\n", __func__);
		ଫେରନ୍ତୁ -1;
	}

	perq = usb_stor_buf[0];
	modi = usb_stor_buf[1];

	/*
	 * Skip unknown devices (0x1f) and enclosure service devices (0x0d),
	 * they would not respond to test_unit_ready .
	 */
	ଯଦି (((perq & 0x1f) == 0x1f) || ((perq & 0x1f) == 0x0d)) {
		debug("%s: unknown/unsupported device\n", __func__);
		ଫେରନ୍ତୁ 0;
	}
	ଯଦି ((modi&0x80) == 0x80) {
		/* drive is removable */
		dev_desc->removable = 1;
	}
	memcpy(dev_desc->vendor, (ଅଚଳ ଶୂନ୍ୟ *)&usb_stor_buf[8], 8);
	memcpy(dev_desc->product, (ଅଚଳ ଶୂନ୍ୟ *)&usb_stor_buf[16], 16);
	memcpy(dev_desc->revision, (ଅଚଳ ଶୂନ୍ୟ *)&usb_stor_buf[32], 4);
	dev_desc->vendor[8] = 0;
	dev_desc->product[16] = 0;
	dev_desc->revision[4] = 0;
#ifdef CONFIG_USB_BIN_FIXUP
	usb_bin_fixup(dev->descriptor, (uchar *)dev_desc->vendor,
		      (uchar *)dev_desc->product);
#endif /* CONFIG_USB_BIN_FIXUP */
	debug("ISO Vers %X, Response Data %X\n", usb_stor_buf[2],
	      usb_stor_buf[3]);
	ଯଦି (usb_test_unit_ready(pccb, ss)) {
		printf("Device NOT ready\n"
		       "   Request Sense returned %02X %02X %02X\n",
		       pccb->sense_buf[2], pccb->sense_buf[12],
		       pccb->sense_buf[13]);
		ଯଦି (dev_desc->removable == 1)
			dev_desc->type = perq;
		ଫେରନ୍ତୁ 0;
	}
	pccb->pdata = (unsigned ଅକ୍ଷର *)cap;
	memset(pccb->pdata, 0, 8);
	ଯଦି (usb_read_capacity(pccb, ss) != 0) {
		printf("READ_CAP ERROR\n");
		ss->flags &= ~USB_READY;
		cap[0] = 2880;
		cap[1] = 0x200;
	}
	debug("Read Capacity returns: 0x%08x, 0x%08x\n", cap[0], cap[1]);
#ଯଦି 0
	ଯଦି (cap[0] > (0x200000 * 10)) /* greater than 10 GByte */
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

	ଫେରନ୍ତୁ 1;
}

#ଯଦି CONFIG_IS_ENABLED(DM_USB)

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା usb_mass_storage_probe(ସଂରଚନା udevice *dev)
{
	ସଂରଚନା usb_device *udev = dev_get_parent_priv(dev);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	ret = usb_stor_probe_device(udev);
	usb_disable_asynch(0); /* asynch transfer allowed */

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ଅଚଳ ସଂରଚନା udevice_id usb_mass_storage_ids[] = {
	{ .compatible = "usb-mass-storage" },
	{ }
};

U_BOOT_DRIVER(usb_mass_storage) = {
	.name	= "usb_mass_storage",
	.id	= UCLASS_MASS_STORAGE,
	.of_match = usb_mass_storage_ids,
	.probe = usb_mass_storage_probe,
#ଯଦି CONFIG_IS_ENABLED(BLK)
	.plat_auto	= ଆକାର(ସଂରଚନା us_data),
#endif
};

UCLASS_DRIVER(usb_mass_storage) = {
	.id		= UCLASS_MASS_STORAGE,
	.name		= "usb_mass_storage",
};

ସ୍ଥିର ଅଚଳ ସଂରଚନା usb_device_id mass_storage_id_table[] = {
	{
		.match_flags = USB_DEVICE_ID_MATCH_INT_CLASS,
		.bInterfaceClass = USB_CLASS_MASS_STORAGE
	},
	{ }		/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_mass_storage, mass_storage_id_table);
#endif

#ଯଦି CONFIG_IS_ENABLED(BLK)
ସ୍ଥିର ଅଚଳ ସଂରଚନା blk_ops usb_storage_ops = {
	.ପଠ	= usb_stor_read,
	.ଲିଖ	= usb_stor_write,
};

U_BOOT_DRIVER(usb_storage_blk) = {
	.name		= "usb_storage_blk",
	.id		= UCLASS_BLK,
	.ops		= &usb_storage_ops,
};
#ଅନ୍ୟଥା
U_BOOT_LEGACY_BLK(usb) = {
	.uclass_idname	= "usb",
	.uclass_id	= UCLASS_USB,
	.max_devs	= USB_MAX_STOR_DEV,
	.desc		= usb_dev_desc,
};
#endif
