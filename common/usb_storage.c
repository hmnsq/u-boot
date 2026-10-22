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

#अन्तर्भाव <blk.h>
#अन्तर्भाव <bootdev.h>
#अन्तर्भाव <command.h>
#अन्तर्भाव <dm.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <mapmem.h>
#अन्तर्भाव <memalign.h>
#अन्तर्भाव <asm/byteorder.h>
#अन्तर्भाव <asm/cache.h>
#अन्तर्भाव <asm/processor.h>
#अन्तर्भाव <dm/device-internal.h>
#अन्तर्भाव <dm/lists.h>
#अन्तर्भाव <linux/delay.h>

#अन्तर्भाव <part.h>
#अन्तर्भाव <usb.h>

#undef BBB_COMDAT_TRACE
#undef BBB_XPORT_TRACE

#अन्तर्भाव <scsi.h>
/* direction table -- this indicates the direction of the data
 * transfer for each command code -- a 1 indicates input
 */
स्थिर अचर अचिन्हित वर्ण us_direction[256/8] = {
	0x28, 0x81, 0x14, 0x14, 0x20, 0x01, 0x90, 0x77,
	0x0C, 0x20, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#define US_DIRECTION(x) ((us_direction[x>>3] >> (x & 7)) & 1)

स्थिर संरचना scsi_cmd usb_ccb __aligned(ARCH_DMA_MINALIGN);
स्थिर __u32 CBWTag;

स्थिर पूर्णाङ्क usb_max_devs; /* number of highest available usb device */

#यदि !CONFIG_IS_ENABLED(BLK)
स्थिर संरचना blk_desc usb_dev_desc[USB_MAX_STOR_DEV];
#endif

संरचना us_data;
प्रकारनिर्देश पूर्णाङ्क (*trans_cmnd)(संरचना scsi_cmd *cb, संरचना us_data *data);
प्रकारनिर्देश पूर्णाङ्क (*trans_reset)(संरचना us_data *data);

संरचना us_data {
	संरचना usb_device *pusb_dev;	 /* this usb_device */

	अचिन्हित पूर्णाङ्क	flags;			/* from filter initially */
#	define USB_READY	(1 << 0)
	अचिन्हित वर्ण	ifnum;			/* interface number */
	अचिन्हित वर्ण	ep_in;			/* in endpoint */
	अचिन्हित वर्ण	ep_out;			/* out ....... */
	अचिन्हित वर्ण	ep_int;			/* interrupt . */
	अचिन्हित वर्ण	subclass;		/* as in overview */
	अचिन्हित वर्ण	protocol;		/* .............. */
	अचिन्हित वर्ण	attention_done;		/* force attn on first cmd */
	अचिन्हित लघु	ip_data;		/* interrupt data */
	पूर्णाङ्क		action;			/* what to do */
	पूर्णाङ्क		ip_wanted;		/* needed */
	पूर्णाङ्क		*irq_handle;		/* for USB int requests */
	अचिन्हित पूर्णाङ्क	irqpipe;		/* pipe for release_irq */
	अचिन्हित वर्ण	irqmaxp;		/* max packed for irq Pipe */
	अचिन्हित वर्ण	irqinterval;		/* Intervall for IRQ Pipe */
	संरचना scsi_cmd	*srb;			/* current srb */
	trans_reset	transport_reset;	/* reset routine */
	trans_cmnd	transport;		/* transport routine */
	अचिन्हित लघु	max_xfer_blk;		/* maximum transfer blocks */
	bool		cmd12;			/* use 12-byte commands (RBC/UFI) */
};

#यदि !CONFIG_IS_ENABLED(BLK)
स्थिर संरचना us_data usb_stor[USB_MAX_STOR_DEV];
#endif

#define USB_STOR_TRANSPORT_GOOD	   0
#define USB_STOR_TRANSPORT_FAILED -1
#define USB_STOR_TRANSPORT_ERROR  -2

पूर्णाङ्क usb_stor_get_info(संरचना usb_device *dev, संरचना us_data *us,
		      संरचना blk_desc *dev_desc);
पूर्णाङ्क usb_storage_probe(संरचना usb_device *dev, अचिन्हित पूर्णाङ्क ifnum,
		      संरचना us_data *ss);
#यदि CONFIG_IS_ENABLED(BLK)
स्थिर अचिन्हित दीर्घ usb_stor_read(संरचना udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, शून्य *buffer);
स्थिर अचिन्हित दीर्घ usb_stor_write(संरचना udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, अचर शून्य *buffer);
#अन्यथा
स्थिर अचिन्हित दीर्घ usb_stor_read(संरचना blk_desc *block_dev, lbaint_t blknr,
				   lbaint_t blkcnt, शून्य *buffer);
स्थिर अचिन्हित दीर्घ usb_stor_write(संरचना blk_desc *block_dev, lbaint_t blknr,
				    lbaint_t blkcnt, अचर शून्य *buffer);
#endif
शून्य uhci_show_temp_int_td(शून्य);

स्थिर शून्य usb_show_progress(शून्य)
{
	debug(".");
}

/*******************************************************************************
 * show info on storage devices; 'usb start/init' must be invoked earlier
 * as we only retrieve structures populated during devices initialization
 */
पूर्णाङ्क usb_stor_info(शून्य)
{
	पूर्णाङ्क गन्ती = 0;
#यदि CONFIG_IS_ENABLED(BLK)
	संरचना udevice *dev;

	लागि (blk_first_device(UCLASS_USB, &dev);
	     dev;
	     blk_next_device(&dev)) {
		संरचना blk_desc *desc = dev_get_uclass_plat(dev);

		म_लिखो("  Device %d: ", desc->devnum);
		dev_print(desc);
		गन्ती++;
	}
#अन्यथा
	पूर्णाङ्क i;

	यदि (usb_max_devs > 0) {
		लागि (i = 0; i < usb_max_devs; i++) {
			म_लिखो("  Device %d: ", i);
			dev_print(&usb_dev_desc[i]);
		}
		फर्काऊ 0;
	}
#endif
	यदि (!गन्ती) {
		म_लिखो("No storage devices, perhaps not 'usb start'ed..?\n");
		फर्काऊ 1;
	}

	फर्काऊ 0;
}

स्थिर अचिन्हित पूर्णाङ्क usb_get_max_lun(संरचना us_data *us)
{
	पूर्णाङ्क len;
	ALLOC_CACHE_ALIGN_BUFFER(अचिन्हित वर्ण, result, 1);
	len = usb_control_सन्देश(us->pusb_dev,
			      usb_rcvctrlpipe(us->pusb_dev, 0),
			      US_BBB_GET_MAX_LUN,
			      USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_IN,
			      0, us->ifnum,
			      result, परिमाण(वर्ण),
			      USB_CNTL_TIMEOUT * 5);
	debug("Get Max LUN -> len = %i, result = %i\n", len, (पूर्णाङ्क) *result);
	फर्काऊ (len > 0) ? *result : 0;
}

स्थिर पूर्णाङ्क usb_stor_probe_device(संरचना usb_device *udev)
{
	पूर्णाङ्क lun, max_lun;

#यदि CONFIG_IS_ENABLED(BLK)
	संरचना us_data *data;
	पूर्णाङ्क ret;
#अन्यथा
	पूर्णाङ्क start;

	यदि (udev == रिक्त)
		फर्काऊ -ENOENT; /* no more devices available */
#endif

	debug("\n\nProbing for storage\n");
#यदि CONFIG_IS_ENABLED(BLK)
	/*
	 * We store the us_data in the mass storage device's plat. It
	 * is shared by all LUNs (block devices) attached to this mass storage
	 * device.
	 */
	data = dev_get_plat(udev->dev);
	यदि (!usb_storage_probe(udev, 0, data))
		फर्काऊ 0;
	max_lun = usb_get_max_lun(data);
	लागि (lun = 0; lun <= max_lun; lun++) {
		संरचना blk_desc *blkdev;
		संरचना udevice *dev;
		वर्ण str[10];

		snprintf(str, परिमाण(str), "lun%d", lun);
		ret = blk_create_devicef(udev->dev, "usb_storage_blk", str,
					 UCLASS_USB, usb_max_devs,
					 DEFAULT_BLKSZ, 0, &dev);
		यदि (ret) {
			debug("Cannot bind driver\n");
			फर्काऊ ret;
		}

		blkdev = dev_get_uclass_plat(dev);
		blkdev->target = 0xff;
		blkdev->lun = lun;

		ret = usb_stor_get_info(udev, data, blkdev);
		यदि (ret == 1) {
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		} अन्यथा {
			debug("usb_stor_get_info: Invalid device\n");
			ret = device_unbind(dev);
			यदि (ret)
				फर्काऊ ret;
			जारी;
		}

		ret = blk_probe_or_unbind(dev);
		यदि (ret)
			फर्काऊ ret;

		ret = bootdev_setup_for_sibling_blk(dev, "usb_bootdev");
		यदि (ret) {
			पूर्णाङ्क ret2;

			ret2 = device_unbind(dev);
			यदि (ret2)
				फर्काऊ log_सन्देश_ret("bootdev", ret2);
			फर्काऊ log_सन्देश_ret("bootdev", ret);
		}
	}
#अन्यथा
	/* We don't have space to even probe if we hit the maximum */
	यदि (usb_max_devs == USB_MAX_STOR_DEV) {
		म_लिखो("max USB Storage Device reached: %d stopping\n",
		       usb_max_devs);
		फर्काऊ -ENOSPC;
	}

	यदि (!usb_storage_probe(udev, 0, &usb_stor[usb_max_devs]))
		फर्काऊ 0;

	/*
	 * OK, it's a storage device.  Iterate over its LUNs and populate
	 * usb_dev_desc'
	 */
	start = usb_max_devs;

	max_lun = usb_get_max_lun(&usb_stor[usb_max_devs]);
	लागि (lun = 0; lun <= max_lun && usb_max_devs < USB_MAX_STOR_DEV;
	     lun++) {
		संरचना blk_desc *blkdev;

		blkdev = &usb_dev_desc[usb_max_devs];
		स_रखो(blkdev, '\0', परिमाण(संरचना blk_desc));
		blkdev->uclass_id = UCLASS_USB;
		blkdev->devnum = usb_max_devs;
		blkdev->part_type = PART_TYPE_UNKNOWN;
		blkdev->target = 0xff;
		blkdev->type = DEV_TYPE_UNKNOWN;
		blkdev->block_read = usb_stor_read;
		blkdev->block_write = usb_stor_write;
		blkdev->lun = lun;
		blkdev->priv = udev;

		यदि (usb_stor_get_info(udev, &usb_stor[start],
				      &usb_dev_desc[usb_max_devs]) == 1) {
			debug("partype: %d\n", blkdev->part_type);
			part_init(blkdev);
			debug("partype: %d\n", blkdev->part_type);
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		}
	}
#endif

	फर्काऊ 0;
}

शून्य usb_stor_reset(शून्य)
{
	usb_max_devs = 0;
}

/*******************************************************************************
 * scan the usb and reports device info
 * to the user if mode = 1
 * returns current device or -1 if no
 */
पूर्णाङ्क usb_stor_scan(पूर्णाङ्क mode)
{
	यदि (mode == 1)
		म_लिखो("       scanning usb for storage devices... ");

#यदि !CONFIG_IS_ENABLED(DM_USB)
	अचिन्हित वर्ण i;

	usb_disable_asynch(1); /* asynch transfer not allowed */

	usb_stor_reset();
	लागि (i = 0; i < USB_MAX_DEVICE; i++) {
		संरचना usb_device *dev;

		dev = usb_get_dev_index(i); /* get device */
		debug("i=%d\n", i);
		यदि (usb_stor_probe_device(dev))
			रोक;
	} /* for */

	usb_disable_asynch(0); /* asynch transfer allowed */
#endif
	म_लिखो("%d Storage Device(s) found\n", usb_max_devs);
	यदि (usb_max_devs > 0)
		फर्काऊ 0;
	फर्काऊ -1;
}

स्थिर पूर्णाङ्क usb_stor_irq(संरचना usb_device *dev)
{
	संरचना us_data *us;
	us = (संरचना us_data *)dev->privptr;

	यदि (us->ip_wanted)
		us->ip_wanted = 0;
	फर्काऊ 0;
}

#ifdef	DEBUG

स्थिर शून्य usb_show_srb(संरचना scsi_cmd *pccb)
{
	पूर्णाङ्क i;
	म_लिखो("SRB: len %d datalen 0x%lX\n ", pccb->cmdlen, pccb->datalen);
	लागि (i = 0; i < pccb->cmdlen; i++)
		म_लिखो("%02X ", pccb->cmd[i]);
	म_लिखो("\n");
}

स्थिर शून्य display_int_status(अचिन्हित दीर्घ tmp)
{
	म_लिखो("Status: %s %s %s %s %s %s %s\n",
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

स्थिर पूर्णाङ्क us_one_transfer(संरचना us_data *us, पूर्णाङ्क pipe, वर्ण *buf, पूर्णाङ्क length)
{
	पूर्णाङ्क max_size;
	पूर्णाङ्क this_xfer;
	पूर्णाङ्क result;
	पूर्णाङ्क partial;
	पूर्णाङ्क maxtry;
	पूर्णाङ्क stat;

	/* determine the maximum packet size for these transfers */
	max_size = usb_maxpacket(us->pusb_dev, pipe) * 16;

	/* while we have data left to transfer */
	जबसम्म (length) {

		/* calculate how long this will be -- maximum or a remainder */
		this_xfer = length > max_size ? max_size : length;
		length -= this_xfer;

		/* setup the retry counter */
		maxtry = 10;

		/* set up the transfer loop */
		गर {
			/* transfer the data */
			debug("Bulk xfer 0x%lx(%d) try #%d\n",
			      (ulong)map_to_sysmem(buf), this_xfer,
			      11 - maxtry);
			result = usb_bulk_सन्देश(us->pusb_dev, pipe, buf,
					      this_xfer, &partial,
					      USB_CNTL_TIMEOUT * 5);
			debug("bulk_msg returned %d xferred %d/%d\n",
			      result, partial, this_xfer);
			यदि (us->pusb_dev->status != 0) {
				/* if we stall, we need to clear it before
				 * we go on
				 */
#ifdef DEBUG
				display_int_status(us->pusb_dev->status);
#endif
				यदि (us->pusb_dev->status & USB_ST_STALLED) {
					debug("stalled ->clearing endpoint" \
					      "halt for pipe 0x%x\n", pipe);
					stat = us->pusb_dev->status;
					usb_clear_halt(us->pusb_dev, pipe);
					us->pusb_dev->status = stat;
					यदि (this_xfer == partial) {
						debug("bulk transferred" \
						      "with error %lX," \
						      " but data ok\n",
						      us->pusb_dev->status);
						फर्काऊ 0;
					}
					अन्यथा
						फर्काऊ result;
				}
				यदि (us->pusb_dev->status & USB_ST_NAK_REC) {
					debug("Device NAKed bulk_msg\n");
					फर्काऊ result;
				}
				debug("bulk transferred with error");
				यदि (this_xfer == partial) {
					debug(" %ld, but data ok\n",
					      us->pusb_dev->status);
					फर्काऊ 0;
				}
				/* if our try counter reaches 0, bail out */
				debug(" %ld, data %d\n",
				      us->pusb_dev->status, partial);
				यदि (!maxtry--)
						फर्काऊ result;
			}
			/* update to show what data was transferred */
			this_xfer -= partial;
			buf += partial;
			/* continue until this transfer is done */
		} जबसम्म (this_xfer);
	}

	/* if we get here, we're done and successful */
	फर्काऊ 0;
}

स्थिर पूर्णाङ्क usb_stor_BBB_reset(संरचना us_data *us)
{
	पूर्णाङ्क result;
	अचिन्हित पूर्णाङ्क pipe;

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
	result = usb_control_सन्देश(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_BBB_RESET,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, रिक्त, 0, USB_CNTL_TIMEOUT * 5);

	यदि ((result < 0) && (us->pusb_dev->status & USB_ST_STALLED)) {
		debug("RESET:stall\n");
		फर्काऊ -1;
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
	फर्काऊ 0;
}

/* FIXME: this reset function doesn't really reset the port, and it
 * should. Actually it should probably do what it's doing here, and
 * reset the port physically
 */
स्थिर पूर्णाङ्क usb_stor_CB_reset(संरचना us_data *us)
{
	अचिन्हित वर्ण cmd[12];
	पूर्णाङ्क result;

	debug("CB_reset\n");
	स_रखो(cmd, 0xff, परिमाण(cmd));
	cmd[0] = SCSI_SEND_DIAG;
	cmd[1] = 4;
	result = usb_control_सन्देश(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_CBI_ADSC,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, cmd, परिमाण(cmd),
				 USB_CNTL_TIMEOUT * 5);

	/* long wait for reset */
	mdelay(1500);
	debug("CB_reset result %d: status %lX clearing endpoint halt\n",
	      result, us->pusb_dev->status);
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_in));
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_out));

	debug("CB_reset done\n");
	फर्काऊ 0;
}

/*
 * Set up the command for a BBB device. Note that the actual SCSI
 * command is copied into cbw.CBWCDB.
 */
स्थिर पूर्णाङ्क usb_stor_BBB_comdat(संरचना scsi_cmd *srb, संरचना us_data *us)
{
	पूर्णाङ्क result;
	पूर्णाङ्क actlen;
	पूर्णाङ्क dir_in;
	अचिन्हित पूर्णाङ्क pipe;
	ALLOC_CACHE_ALIGN_BUFFER(संरचना umass_bbb_cbw, cbw, 1);

	dir_in = US_DIRECTION(srb->cmd[0]);

#ifdef BBB_COMDAT_TRACE
	म_लिखो("dir %d lun %d cmdlen %d cmd %p datalen %lu pdata %p\n",
		dir_in, srb->lun, srb->cmdlen, srb->cmd, srb->datalen,
		srb->pdata);
	यदि (srb->cmdlen) {
		लागि (result = 0; result < srb->cmdlen; result++)
			म_लिखो("cmd[%d] %#x ", result, srb->cmd[result]);
		म_लिखो("\n");
	}
#endif
	/* sanity checks */
	यदि (!(srb->cmdlen <= CBWCDBLENGTH)) {
		debug("usb_stor_BBB_comdat:cmdlen too large\n");
		फर्काऊ -1;
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

	स_नकल(cbw->CBWCDB, srb->cmd, srb->cmdlen);
	result = usb_bulk_सन्देश(us->pusb_dev, pipe, cbw, UMASS_BBB_CBW_SIZE,
			      &actlen, USB_CNTL_TIMEOUT * 5);
	यदि (result < 0)
		debug("usb_stor_BBB_comdat:usb_bulk_msg error\n");
	फर्काऊ result;
}

/* FIXME: we also need a CBI_command which sets up the completion
 * interrupt, and waits for it
 */
स्थिर पूर्णाङ्क usb_stor_CB_comdat(संरचना scsi_cmd *srb, संरचना us_data *us)
{
	पूर्णाङ्क result = 0;
	पूर्णाङ्क dir_in, retry;
	अचिन्हित पूर्णाङ्क pipe;
	अचिन्हित दीर्घ status;

	retry = 5;
	dir_in = US_DIRECTION(srb->cmd[0]);

	यदि (dir_in)
		pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);
	अन्यथा
		pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_out);

	जबसम्म (retry--) {
		debug("CBI gets a command: Try %d\n", 5 - retry);
#ifdef DEBUG
		usb_show_srb(srb);
#endif
		/* let's send the command via the control pipe */
		result = usb_control_सन्देश(us->pusb_dev,
					 usb_sndctrlpipe(us->pusb_dev , 0),
					 US_CBI_ADSC,
					 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
					 0, us->ifnum,
					 srb->cmd, srb->cmdlen,
					 USB_CNTL_TIMEOUT * 5);
		debug("CB_transport: control msg returned %d, status %lX\n",
		      result, us->pusb_dev->status);
		/* check the return code for the command */
		यदि (result < 0) {
			यदि (us->pusb_dev->status & USB_ST_STALLED) {
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
			फर्काऊ result;
		}
		/* transfer the data payload for this command, if one exists*/

		debug("CB_transport: control msg returned %d," \
		      " direction is %s to go 0x%lx\n", result,
		      dir_in ? "IN" : "OUT", srb->datalen);
		यदि (srb->datalen) {
			result = us_one_transfer(us, pipe, (वर्ण *)srb->pdata,
						 srb->datalen);
			debug("CBI attempted to transfer data," \
			      " result is %d status %lX, len %d\n",
			      result, us->pusb_dev->status,
				us->pusb_dev->act_len);
			यदि (!(us->pusb_dev->status & USB_ST_NAK_REC))
				रोक;
		} /* if (srb->datalen) */
		अन्यथा
			रोक;
	}
	/* return result */

	फर्काऊ result;
}

स्थिर पूर्णाङ्क usb_stor_CBI_get_status(संरचना scsi_cmd *srb, संरचना us_data *us)
{
	पूर्णाङ्क timeout;

	us->ip_wanted = 1;
	usb_int_सन्देश(us->pusb_dev, us->irqpipe,
		    (शून्य *)&us->ip_data, us->irqmaxp, us->irqinterval, false);
	timeout = 1000;
	जबसम्म (timeout--) {
		यदि (us->ip_wanted == 0)
			रोक;
		mdelay(10);
	}
	यदि (us->ip_wanted) {
		म_लिखो("	Did not get interrupt on CBI\n");
		us->ip_wanted = 0;
		फर्काऊ USB_STOR_TRANSPORT_ERROR;
	}
	debug("Got interrupt data 0x%x, transferred %d status 0x%lX\n",
	      us->ip_data, us->pusb_dev->irq_act_len,
	      us->pusb_dev->irq_status);
	/* UFI gives us ASC and ASCQ, like a request sense */
	यदि (us->subclass == US_SC_UFI) {
		यदि (srb->cmd[0] == SCSI_REQ_SENSE ||
		    srb->cmd[0] == SCSI_INQUIRY)
			फर्काऊ USB_STOR_TRANSPORT_GOOD; /* Good */
		अन्यथा यदि (us->ip_data)
			फर्काऊ USB_STOR_TRANSPORT_FAILED;
		अन्यथा
			फर्काऊ USB_STOR_TRANSPORT_GOOD;
	}
	/* otherwise, we interpret the data normally */
	विकल्प (us->ip_data) {
	पक्ष 0x0001:
		फर्काऊ USB_STOR_TRANSPORT_GOOD;
	पक्ष 0x0002:
		फर्काऊ USB_STOR_TRANSPORT_FAILED;
	स्वभाव:
		फर्काऊ USB_STOR_TRANSPORT_ERROR;
	}			/* switch */
	फर्काऊ USB_STOR_TRANSPORT_ERROR;
}

#define USB_TRANSPORT_UNKNOWN_RETRY 5
#define USB_TRANSPORT_NOT_READY_RETRY 10

/* clear a stall on an endpoint - special for BBB devices */
स्थिर पूर्णाङ्क usb_stor_BBB_clear_endpt_stall(संरचना us_data *us, __u8 endpt)
{
	/* ENDPOINT_HALT = 0, so set value to 0 */
	फर्काऊ usb_control_सन्देश(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
			       USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
			       endpt, रिक्त, 0, USB_CNTL_TIMEOUT * 5);
}

स्थिर पूर्णाङ्क usb_stor_BBB_transport(संरचना scsi_cmd *srb, संरचना us_data *us)
{
	पूर्णाङ्क result, retry;
	पूर्णाङ्क dir_in;
	पूर्णाङ्क actlen, data_actlen;
	अचिन्हित पूर्णाङ्क pipe, pipein, pipeout;
	ALLOC_CACHE_ALIGN_BUFFER(संरचना umass_bbb_csw, csw, 1);
#ifdef BBB_XPORT_TRACE
	अचिन्हित वर्ण *ptr;
	पूर्णाङ्क index;
#endif

	dir_in = US_DIRECTION(srb->cmd[0]);

	/* COMMAND phase */
	debug("COMMAND phase\n");
	result = usb_stor_BBB_comdat(srb, us);
	यदि (result < 0) {
		debug("failed to send CBW status %ld\n",
		      us->pusb_dev->status);
		usb_stor_BBB_reset(us);
		फर्काऊ USB_STOR_TRANSPORT_FAILED;
	}
	यदि (!(us->flags & USB_READY))
		mdelay(5);
	pipein = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);
	pipeout = usb_sndbulkpipe(us->pusb_dev, us->ep_out);
	/* DATA phase + error handling */
	data_actlen = 0;
	/* no data, go immediately to the STATUS phase */
	यदि (srb->datalen == 0)
		गच्छ st;
	debug("DATA phase\n");
	यदि (dir_in)
		pipe = pipein;
	अन्यथा
		pipe = pipeout;

	result = usb_bulk_सन्देश(us->pusb_dev, pipe, srb->pdata, srb->datalen,
			      &data_actlen, USB_CNTL_TIMEOUT * 5);
	/* special handling of STALL in DATA phase */
	यदि ((result < 0) && (us->pusb_dev->status & USB_ST_STALLED)) {
		debug("DATA:stall\n");
		/* clear the STALL on the endpoint */
		result = usb_stor_BBB_clear_endpt_stall(us,
					dir_in ? us->ep_in : us->ep_out);
		यदि (result >= 0)
			/* continue on to STATUS phase */
			गच्छ st;
	}
	यदि (result < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->status);
		usb_stor_BBB_reset(us);
		फर्काऊ USB_STOR_TRANSPORT_FAILED;
	}
#ifdef BBB_XPORT_TRACE
	लागि (index = 0; index < data_actlen; index++)
		म_लिखो("pdata[%d] %#x ", index, srb->pdata[index]);
	म_लिखो("\n");
#endif
	/* STATUS phase + error handling */
st:
	retry = 0;
again:
	debug("STATUS phase\n");
	result = usb_bulk_सन्देश(us->pusb_dev, pipein, csw, UMASS_BBB_CSW_SIZE,
				&actlen, USB_CNTL_TIMEOUT*5);

	/* special handling of STALL in STATUS phase */
	यदि ((result < 0) && (retry < 1) &&
	    (us->pusb_dev->status & USB_ST_STALLED)) {
		debug("STATUS:stall\n");
		/* clear the STALL on the endpoint */
		result = usb_stor_BBB_clear_endpt_stall(us, us->ep_in);
		यदि (result >= 0 && (retry++ < 1))
			/* do a retry */
			गच्छ again;
	}
	यदि (result < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->status);
		usb_stor_BBB_reset(us);
		फर्काऊ USB_STOR_TRANSPORT_FAILED;
	}
#ifdef BBB_XPORT_TRACE
	ptr = (अचिन्हित वर्ण *)csw;
	लागि (index = 0; index < UMASS_BBB_CSW_SIZE; index++)
		म_लिखो("ptr[%d] %#x ", index, ptr[index]);
	म_लिखो("\n");
#endif
	/* misuse pipe to get the residue */
	pipe = le32_to_cpu(csw->dCSWDataResidue);
	यदि (pipe == 0 && srb->datalen != 0 && srb->datalen - data_actlen != 0)
		pipe = srb->datalen - data_actlen;
	यदि (CSWSIGNATURE != le32_to_cpu(csw->dCSWSignature)) {
		debug("!CSWSIGNATURE\n");
		usb_stor_BBB_reset(us);
		फर्काऊ USB_STOR_TRANSPORT_FAILED;
	} अन्यथा यदि ((CBWTag - 1) != le32_to_cpu(csw->dCSWTag)) {
		debug("!Tag\n");
		usb_stor_BBB_reset(us);
		फर्काऊ USB_STOR_TRANSPORT_FAILED;
	} अन्यथा यदि (csw->bCSWStatus > CSWSTATUS_PHASE) {
		debug(">PHASE\n");
		usb_stor_BBB_reset(us);
		फर्काऊ USB_STOR_TRANSPORT_FAILED;
	} अन्यथा यदि (csw->bCSWStatus == CSWSTATUS_PHASE) {
		debug("=PHASE\n");
		usb_stor_BBB_reset(us);
		फर्काऊ USB_STOR_TRANSPORT_FAILED;
	} अन्यथा यदि (data_actlen > srb->datalen) {
		debug("transferred %dB instead of %ldB\n",
		      data_actlen, srb->datalen);
		फर्काऊ USB_STOR_TRANSPORT_FAILED;
	} अन्यथा यदि (csw->bCSWStatus == CSWSTATUS_FAILED) {
		debug("FAILED\n");
		फर्काऊ USB_STOR_TRANSPORT_FAILED;
	}

	फर्काऊ result;
}

स्थिर पूर्णाङ्क usb_stor_CB_transport(संरचना scsi_cmd *srb, संरचना us_data *us)
{
	पूर्णाङ्क result, status;
	संरचना scsi_cmd *psrb;
	संरचना scsi_cmd reqsrb;
	पूर्णाङ्क retry, notready;

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
	यदि (us->protocol == US_PR_CBI) {
		status = usb_stor_CBI_get_status(srb, us);
		/* if the status is error, report it */
		यदि (status == USB_STOR_TRANSPORT_ERROR) {
			debug(" USB CBI Command Error\n");
			फर्काऊ status;
		}
		srb->sense_buf[12] = (अचिन्हित वर्ण)(us->ip_data >> 8);
		srb->sense_buf[13] = (अचिन्हित वर्ण)(us->ip_data & 0xff);
		यदि (!us->ip_data) {
			/* if the status is good, report it */
			यदि (status == USB_STOR_TRANSPORT_GOOD) {
				debug(" USB CBI Command Good\n");
				फर्काऊ status;
			}
		}
	}
	/* do we have to issue an auto request? */
	/* HERE we have to check the result */
	यदि ((result < 0) && !(us->pusb_dev->status & USB_ST_STALLED)) {
		debug("ERROR %lX\n", us->pusb_dev->status);
		us->transport_reset(us);
		फर्काऊ USB_STOR_TRANSPORT_ERROR;
	}
	यदि ((us->protocol == US_PR_CBI) &&
	    ((srb->cmd[0] == SCSI_REQ_SENSE) ||
	    (srb->cmd[0] == SCSI_INQUIRY))) {
		/* do not issue an autorequest after request sense */
		debug("No auto request and good\n");
		फर्काऊ USB_STOR_TRANSPORT_GOOD;
	}
	/* issue an request_sense */
	स_रखो(&psrb->cmd[0], 0, 12);
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
	यदि (us->protocol == US_PR_CBI)
		status = usb_stor_CBI_get_status(psrb, us);

	यदि ((result < 0) && !(us->pusb_dev->status & USB_ST_STALLED)) {
		debug(" AUTO REQUEST ERROR %ld\n",
		      us->pusb_dev->status);
		फर्काऊ USB_STOR_TRANSPORT_ERROR;
	}
	debug("autorequest returned 0x%02X 0x%02X 0x%02X 0x%02X\n",
	      srb->sense_buf[0], srb->sense_buf[2],
	      srb->sense_buf[12], srb->sense_buf[13]);
	/* Check the auto request result */
	यदि ((srb->sense_buf[2] == 0) &&
	    (srb->sense_buf[12] == 0) &&
	    (srb->sense_buf[13] == 0)) {
		/* ok, no sense */
		फर्काऊ USB_STOR_TRANSPORT_GOOD;
	}

	/* Check the auto request result */
	विकल्प (srb->sense_buf[2]) {
	पक्ष 0x01:
		/* Recovered Error */
		फर्काऊ USB_STOR_TRANSPORT_GOOD;
		रोक;
	पक्ष 0x02:
		/* Not Ready */
		यदि (notready++ > USB_TRANSPORT_NOT_READY_RETRY) {
			म_लिखो("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X (NOT READY)\n", srb->cmd[0],
				srb->sense_buf[0], srb->sense_buf[2],
				srb->sense_buf[12], srb->sense_buf[13]);
			फर्काऊ USB_STOR_TRANSPORT_FAILED;
		} अन्यथा {
			mdelay(100);
			गच्छ do_retry;
		}
		रोक;
	स्वभाव:
		यदि (retry++ > USB_TRANSPORT_UNKNOWN_RETRY) {
			म_लिखो("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X\n", srb->cmd[0], srb->sense_buf[0],
				srb->sense_buf[2], srb->sense_buf[12],
				srb->sense_buf[13]);
			फर्काऊ USB_STOR_TRANSPORT_FAILED;
		} अन्यथा
			गच्छ do_retry;
		रोक;
	}
	फर्काऊ USB_STOR_TRANSPORT_FAILED;
}

स्थिर शून्य usb_stor_set_max_xfer_blk(संरचना usb_device *udev,
				      संरचना us_data *us)
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
	अचिन्हित लघु blk = 240;

#यदि CONFIG_IS_ENABLED(DM_USB)
	माप_प्रकार size;
	पूर्णाङ्क ret;

	ret = usb_get_max_xfer_size(udev, (माप_प्रकार *)&size);
	यदि ((ret >= 0) && (size < blk * 512))
		blk = size / 512;
#endif

	us->max_xfer_blk = blk;
}

स्थिर पूर्णाङ्क usb_inquiry(संरचना scsi_cmd *srb, संरचना us_data *ss)
{
	पूर्णाङ्क retry, i;
	retry = 5;
	गर {
		स_रखो(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_INQUIRY;
		srb->cmd[1] = srb->lun << 5;
		srb->cmd[4] = 36;
		srb->datalen = 36;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		i = ss->transport(srb, ss);
		debug("inquiry returns %d\n", i);
		यदि (i == 0)
			रोक;
	} जबसम्म (--retry);

	यदि (!retry) {
		म_लिखो("error in inquiry\n");
		फर्काऊ -1;
	}
	फर्काऊ 0;
}

स्थिर पूर्णाङ्क usb_request_sense(संरचना scsi_cmd *srb, संरचना us_data *ss)
{
	वर्ण *ptr;

	ptr = (वर्ण *)srb->pdata;
	स_रखो(&srb->cmd[0], 0, 12);
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
	फर्काऊ 0;
}

स्थिर पूर्णाङ्क usb_test_unit_ready(संरचना scsi_cmd *srb, संरचना us_data *ss)
{
	पूर्णाङ्क retries = 10;

	गर {
		स_रखो(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_TST_U_RDY;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 0;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		यदि (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD) {
			ss->flags |= USB_READY;
			फर्काऊ 0;
		}
		usb_request_sense(srb, ss);
		/*
		 * Check the Key Code Qualifier, if it matches
		 * "Not Ready - medium not present"
		 * (the sense Key equals 0x2 and the ASC is 0x3a)
		 * return immediately as the medium being absent won't change
		 * unless there is a user action.
		 */
		यदि ((srb->sense_buf[2] == 0x02) &&
		    (srb->sense_buf[12] == 0x3a))
			फर्काऊ -1;
		mdelay(100);
	} जबसम्म (retries--);

	फर्काऊ -1;
}

स्थिर पूर्णाङ्क usb_read_capacity(संरचना scsi_cmd *srb, संरचना us_data *ss)
{
	पूर्णाङ्क retry;
	/* XXX retries */
	retry = 3;
	गर {
		स_रखो(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_RD_CAPAC;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 8;
		srb->cmdlen = ss->cmd12 ? 12 : 10;
		यदि (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD)
			फर्काऊ 0;
	} जबसम्म (retry--);

	फर्काऊ -1;
}

स्थिर पूर्णाङ्क usb_read_10(संरचना scsi_cmd *srb, संरचना us_data *ss,
		       अचिन्हित दीर्घ start, अचिन्हित लघु blocks)
{
	स_रखो(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_READ10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((अचिन्हित वर्ण) (start >> 24)) & 0xff;
	srb->cmd[3] = ((अचिन्हित वर्ण) (start >> 16)) & 0xff;
	srb->cmd[4] = ((अचिन्हित वर्ण) (start >> 8)) & 0xff;
	srb->cmd[5] = ((अचिन्हित वर्ण) (start)) & 0xff;
	srb->cmd[7] = ((अचिन्हित वर्ण) (blocks >> 8)) & 0xff;
	srb->cmd[8] = (अचिन्हित वर्ण) blocks & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("read10: start %lx blocks %x\n", start, blocks);
	फर्काऊ ss->transport(srb, ss);
}

स्थिर पूर्णाङ्क usb_write_10(संरचना scsi_cmd *srb, संरचना us_data *ss,
			अचिन्हित दीर्घ start, अचिन्हित लघु blocks)
{
	स_रखो(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_WRITE10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((अचिन्हित वर्ण) (start >> 24)) & 0xff;
	srb->cmd[3] = ((अचिन्हित वर्ण) (start >> 16)) & 0xff;
	srb->cmd[4] = ((अचिन्हित वर्ण) (start >> 8)) & 0xff;
	srb->cmd[5] = ((अचिन्हित वर्ण) (start)) & 0xff;
	srb->cmd[7] = ((अचिन्हित वर्ण) (blocks >> 8)) & 0xff;
	srb->cmd[8] = (अचिन्हित वर्ण) blocks & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("write10: start %lx blocks %x\n", start, blocks);
	फर्काऊ ss->transport(srb, ss);
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
स्थिर शून्य usb_bin_fixup(संरचना usb_device_descriptor descriptor,
				अचिन्हित वर्ण vendor[],
				अचिन्हित वर्ण product[]) {
	अचर अचिन्हित वर्ण max_vendor_len = 40;
	अचर अचिन्हित वर्ण max_product_len = 20;
	यदि (descriptor.idVendor == 0x0424 && descriptor.idProduct == 0x223a) {
		म_नकलन((वर्ण *)vendor, "SMSC", max_vendor_len);
		म_नकलन((वर्ण *)product, "Flash Media Cntrller",
			max_product_len);
	}
}
#endif /* CONFIG_USB_BIN_FIXUP */

#यदि CONFIG_IS_ENABLED(BLK)
स्थिर अचिन्हित दीर्घ usb_stor_read(संरचना udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, शून्य *buffer)
#अन्यथा
स्थिर अचिन्हित दीर्घ usb_stor_read(संरचना blk_desc *block_dev, lbaint_t blknr,
				   lbaint_t blkcnt, शून्य *buffer)
#endif
{
	lbaint_t start, blks;
	uintptr_t buf_addr;
	अचिन्हित लघु smallblks;
	संरचना usb_device *udev;
	संरचना us_data *ss;
	पूर्णाङ्क retry;
	संरचना scsi_cmd *srb = &usb_ccb;
#यदि CONFIG_IS_ENABLED(BLK)
	संरचना blk_desc *block_dev;
#endif

	यदि (blkcnt == 0)
		फर्काऊ 0;
	/* Setup  device */
#यदि CONFIG_IS_ENABLED(BLK)
	block_dev = dev_get_uclass_plat(dev);
	udev = dev_get_parent_priv(dev_get_parent(dev));
	debug("\nusb_read: udev %d\n", block_dev->devnum);
#अन्यथा
	debug("\nusb_read: udev %d\n", block_dev->devnum);
	udev = usb_dev_desc[block_dev->devnum].priv;
	यदि (!udev) {
		debug("%s: No device\n", __func__);
		फर्काऊ 0;
	}
#endif
	ss = (संरचना us_data *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_lock_async(udev, 1);
	srb->lun = block_dev->lun;
	buf_addr = (uintptr_t)buffer;
	start = blknr;
	blks = blkcnt;

	debug("\nusb_read: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      block_dev->devnum, start, blks, buf_addr);

	गर {
		/* XXX need some comment here */
		retry = 2;
		srb->pdata = (अचिन्हित वर्ण *)buf_addr;
		यदि (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		अन्यथा
			smallblks = (अचिन्हित लघु) blks;
retry_it:
		यदि (smallblks == ss->max_xfer_blk)
			usb_show_progress();
		srb->datalen = block_dev->blksz * smallblks;
		srb->pdata = (अचिन्हित वर्ण *)buf_addr;
		यदि (usb_read_10(srb, ss, start, smallblks)) {
			debug("Read ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			यदि (retry--)
				गच्छ retry_it;
			blkcnt -= blks;
			रोक;
		}
		start += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} जबसम्म (blks != 0);

	debug("usb_read: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      start, smallblks, buf_addr);

	usb_lock_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	यदि (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	फर्काऊ blkcnt;
}

#यदि CONFIG_IS_ENABLED(BLK)
स्थिर अचिन्हित दीर्घ usb_stor_write(संरचना udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, अचर शून्य *buffer)
#अन्यथा
स्थिर अचिन्हित दीर्घ usb_stor_write(संरचना blk_desc *block_dev, lbaint_t blknr,
				    lbaint_t blkcnt, अचर शून्य *buffer)
#endif
{
	lbaint_t start, blks;
	uintptr_t buf_addr;
	अचिन्हित लघु smallblks;
	संरचना usb_device *udev;
	संरचना us_data *ss;
	पूर्णाङ्क retry;
	संरचना scsi_cmd *srb = &usb_ccb;
#यदि CONFIG_IS_ENABLED(BLK)
	संरचना blk_desc *block_dev;
#endif

	यदि (blkcnt == 0)
		फर्काऊ 0;

	/* Setup  device */
#यदि CONFIG_IS_ENABLED(BLK)
	block_dev = dev_get_uclass_plat(dev);
	udev = dev_get_parent_priv(dev_get_parent(dev));
	debug("\nusb_read: udev %d\n", block_dev->devnum);
#अन्यथा
	debug("\nusb_read: udev %d\n", block_dev->devnum);
	udev = usb_dev_desc[block_dev->devnum].priv;
	यदि (!udev) {
		debug("%s: No device\n", __func__);
		फर्काऊ 0;
	}
#endif
	ss = (संरचना us_data *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_lock_async(udev, 1);

	srb->lun = block_dev->lun;
	buf_addr = (uintptr_t)buffer;
	start = blknr;
	blks = blkcnt;

	debug("\nusb_write: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      block_dev->devnum, start, blks, buf_addr);

	गर {
		/* If write fails retry for max retry count else
		 * return with number of blocks written successfully.
		 */
		retry = 2;
		srb->pdata = (अचिन्हित वर्ण *)buf_addr;
		यदि (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		अन्यथा
			smallblks = (अचिन्हित लघु) blks;
retry_it:
		यदि (smallblks == ss->max_xfer_blk)
			usb_show_progress();
		srb->datalen = block_dev->blksz * smallblks;
		srb->pdata = (अचिन्हित वर्ण *)buf_addr;
		यदि (usb_write_10(srb, ss, start, smallblks)) {
			debug("Write ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			यदि (retry--)
				गच्छ retry_it;
			blkcnt -= blks;
			रोक;
		}
		start += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} जबसम्म (blks != 0);

	debug("usb_write: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      start, smallblks, buf_addr);

	usb_lock_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	यदि (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	फर्काऊ blkcnt;

}

/* Probe to see if a new device is actually a Storage device */
पूर्णाङ्क usb_storage_probe(संरचना usb_device *dev, अचिन्हित पूर्णाङ्क ifnum,
		      संरचना us_data *ss)
{
	संरचना usb_interface *iface;
	पूर्णाङ्क i;
	संरचना usb_endpoint_descriptor *ep_desc;
	अचिन्हित पूर्णाङ्क flags = 0;

	/* let's examine the device now */
	iface = &dev->config.if_desc[ifnum];

	यदि (dev->descriptor.bDeviceClass != 0 ||
			iface->desc.bInterfaceClass != USB_CLASS_MASS_STORAGE ||
			iface->desc.bInterfaceSubClass < US_SC_MIN ||
			iface->desc.bInterfaceSubClass > US_SC_MAX) {
		debug("Not mass storage\n");
		/* if it's not a mass storage, we go no further */
		फर्काऊ 0;
	}

	स_रखो(ss, 0, परिमाण(संरचना us_data));

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
	विकल्प (ss->protocol) {
	पक्ष US_PR_CB:
		debug("Control/Bulk\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		रोक;

	पक्ष US_PR_CBI:
		debug("Control/Bulk/Interrupt\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		रोक;
	पक्ष US_PR_BULK:
		debug("Bulk/Bulk/Bulk\n");
		ss->transport = usb_stor_BBB_transport;
		ss->transport_reset = usb_stor_BBB_reset;
		रोक;
	स्वभाव:
		म_लिखो("USB Storage Transport unknown / not yet implemented\n");
		फर्काऊ 0;
		रोक;
	}

	/*
	 * We are expecting a minimum of 2 endpoints - in and out (bulk).
	 * An optional interrupt is OK (necessary for CBI protocol).
	 * We will ignore any others.
	 */
	लागि (i = 0; i < iface->desc.b_अङ्क_Endpoints; i++) {
		ep_desc = &iface->ep_desc[i];
		/* is it an BULK endpoint? */
		यदि ((ep_desc->bmAttributes &
		     USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK) {
			यदि (ep_desc->bEndpointAddress & USB_DIR_IN)
				ss->ep_in = ep_desc->bEndpointAddress &
						USB_ENDPOINT_सङ्ख्या_MASK;
			अन्यथा
				ss->ep_out =
					ep_desc->bEndpointAddress &
					USB_ENDPOINT_सङ्ख्या_MASK;
		}

		/* is it an interrupt endpoint? */
		यदि ((ep_desc->bmAttributes &
		     USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT) {
			ss->ep_int = ep_desc->bEndpointAddress &
						USB_ENDPOINT_सङ्ख्या_MASK;
			ss->irqinterval = ep_desc->bInterval;
		}
	}
	debug("Endpoints In %d Out %d Int %d\n",
	      ss->ep_in, ss->ep_out, ss->ep_int);

	/* Do some basic sanity checks, and bail if we find a problem */
	यदि (usb_set_interface(dev, iface->desc.b_Interface_सङ्ख्या, 0) ||
	    !ss->ep_in || !ss->ep_out ||
	    (ss->protocol == US_PR_CBI && ss->ep_int == 0)) {
		debug("Problems with device\n");
		फर्काऊ 0;
	}
	/* set class specific stuff */
	/* We only handle certain protocols.  Currently, these are
	 * the only ones.
	 * The SFF8070 accepts the requests used in u-boot
	 */
	यदि (ss->subclass != US_SC_UFI && ss->subclass != US_SC_SCSI &&
	    ss->subclass != US_SC_8070) {
		म_लिखो("Sorry, protocol %d not yet supported.\n", ss->subclass);
		फर्काऊ 0;
	}

	/* UFI uses 12-byte commands (like RBC, unlike SCSI) */
	यदि (ss->subclass == US_SC_UFI)
		ss->cmd12 = true;

	यदि (ss->ep_int) {
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

	dev->privptr = (शून्य *)ss;
	फर्काऊ 1;
}

पूर्णाङ्क usb_stor_get_info(संरचना usb_device *dev, संरचना us_data *ss,
		      संरचना blk_desc *dev_desc)
{
	अचिन्हित वर्ण perq, modi;
	ALLOC_CACHE_ALIGN_BUFFER(u32, cap, 2);
	ALLOC_CACHE_ALIGN_BUFFER(u8, usb_stor_buf, 36);
	u32 capacity, blksz;
	संरचना scsi_cmd *pccb = &usb_ccb;

	pccb->pdata = usb_stor_buf;

	dev_desc->target = dev->devnum;
	pccb->lun = dev_desc->lun;
	debug(" address %d\n", dev_desc->target);

	यदि (usb_inquiry(pccb, ss)) {
		debug("%s: usb_inquiry() failed\n", __func__);
		फर्काऊ -1;
	}

	perq = usb_stor_buf[0];
	modi = usb_stor_buf[1];

	/*
	 * Skip unknown devices (0x1f) and enclosure service devices (0x0d),
	 * they would not respond to test_unit_ready .
	 */
	यदि (((perq & 0x1f) == 0x1f) || ((perq & 0x1f) == 0x0d)) {
		debug("%s: unknown/unsupported device\n", __func__);
		फर्काऊ 0;
	}
	यदि ((modi&0x80) == 0x80) {
		/* drive is removable */
		dev_desc->removable = 1;
	}
	स_नकल(dev_desc->vendor, (अचर शून्य *)&usb_stor_buf[8], 8);
	स_नकल(dev_desc->product, (अचर शून्य *)&usb_stor_buf[16], 16);
	स_नकल(dev_desc->revision, (अचर शून्य *)&usb_stor_buf[32], 4);
	dev_desc->vendor[8] = 0;
	dev_desc->product[16] = 0;
	dev_desc->revision[4] = 0;
#ifdef CONFIG_USB_BIN_FIXUP
	usb_bin_fixup(dev->descriptor, (uchar *)dev_desc->vendor,
		      (uchar *)dev_desc->product);
#endif /* CONFIG_USB_BIN_FIXUP */
	debug("ISO Vers %X, Response Data %X\n", usb_stor_buf[2],
	      usb_stor_buf[3]);
	यदि (usb_test_unit_ready(pccb, ss)) {
		म_लिखो("Device NOT ready\n"
		       "   Request Sense returned %02X %02X %02X\n",
		       pccb->sense_buf[2], pccb->sense_buf[12],
		       pccb->sense_buf[13]);
		यदि (dev_desc->removable == 1)
			dev_desc->type = perq;
		फर्काऊ 0;
	}
	pccb->pdata = (अचिन्हित वर्ण *)cap;
	स_रखो(pccb->pdata, 0, 8);
	यदि (usb_read_capacity(pccb, ss) != 0) {
		म_लिखो("READ_CAP ERROR\n");
		ss->flags &= ~USB_READY;
		cap[0] = 2880;
		cap[1] = 0x200;
	}
	debug("Read Capacity returns: 0x%08x, 0x%08x\n", cap[0], cap[1]);
#यदि 0
	यदि (cap[0] > (0x200000 * 10)) /* greater than 10 GByte */
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

	फर्काऊ 1;
}

#यदि CONFIG_IS_ENABLED(DM_USB)

स्थिर पूर्णाङ्क usb_mass_storage_probe(संरचना udevice *dev)
{
	संरचना usb_device *udev = dev_get_parent_priv(dev);
	पूर्णाङ्क ret;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	ret = usb_stor_probe_device(udev);
	usb_disable_asynch(0); /* asynch transfer allowed */

	फर्काऊ ret;
}

स्थिर अचर संरचना udevice_id usb_mass_storage_ids[] = {
	{ .compatible = "usb-mass-storage" },
	{ }
};

U_BOOT_DRIVER(usb_mass_storage) = {
	.name	= "usb_mass_storage",
	.id	= UCLASS_MASS_STORAGE,
	.of_match = usb_mass_storage_ids,
	.probe = usb_mass_storage_probe,
#यदि CONFIG_IS_ENABLED(BLK)
	.plat_auto	= परिमाण(संरचना us_data),
#endif
};

UCLASS_DRIVER(usb_mass_storage) = {
	.id		= UCLASS_MASS_STORAGE,
	.name		= "usb_mass_storage",
};

स्थिर अचर संरचना usb_device_id mass_storage_id_table[] = {
	{
		.match_flags = USB_DEVICE_ID_MATCH_INT_CLASS,
		.bInterfaceClass = USB_CLASS_MASS_STORAGE
	},
	{ }		/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_mass_storage, mass_storage_id_table);
#endif

#यदि CONFIG_IS_ENABLED(BLK)
स्थिर अचर संरचना blk_ops usb_storage_ops = {
	.पठ	= usb_stor_read,
	.लिख	= usb_stor_write,
};

U_BOOT_DRIVER(usb_storage_blk) = {
	.name		= "usb_storage_blk",
	.id		= UCLASS_BLK,
	.ops		= &usb_storage_ops,
};
#अन्यथा
U_BOOT_LEGACY_BLK(usb) = {
	.uclass_idname	= "usb",
	.uclass_id	= UCLASS_USB,
	.max_devs	= USB_MAX_STOR_DEV,
	.desc		= usb_dev_desc,
};
#endif
