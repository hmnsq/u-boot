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
स्थिर स्थिरांक अचिन्हित अक्षर us_direction[256/8] = {
	0x28, 0x81, 0x14, 0x14, 0x20, 0x01, 0x90, 0x77,
	0x0C, 0x20, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#define US_DIRECTION(x) ((us_direction[x>>3] >> (x & 7)) & 1)

स्थिर संरचना scsi_cmd usb_ccb __aligned(ARCH_DMA_MINALIGN);
स्थिर __u32 CBWTag;

स्थिर पूर्णांक usb_max_devs; /* number of highest available usb device */

#जर !CONFIG_IS_ENABLED(BLK)
स्थिर संरचना blk_desc usb_dev_desc[USB_MAX_STOR_DEV];
#endif

संरचना us_data;
प्रकारव्याख्या पूर्णांक (*trans_cmnd)(संरचना scsi_cmd *cb, संरचना us_data *data);
प्रकारव्याख्या पूर्णांक (*trans_reset)(संरचना us_data *data);

संरचना us_data {
	संरचना usb_device *pusb_dev;	 /* this usb_device */

	अचिन्हित पूर्णांक	flags;			/* from filter initially */
#	define USB_READY	(1 << 0)
	अचिन्हित अक्षर	ifnum;			/* interface number */
	अचिन्हित अक्षर	ep_in;			/* in endpoint */
	अचिन्हित अक्षर	ep_out;			/* out ....... */
	अचिन्हित अक्षर	ep_int;			/* interrupt . */
	अचिन्हित अक्षर	subclass;		/* as in overview */
	अचिन्हित अक्षर	protocol;		/* .............. */
	अचिन्हित अक्षर	attention_done;		/* force attn on first cmd */
	अचिन्हित लघु	ip_data;		/* interrupt data */
	पूर्णांक		action;			/* what to do */
	पूर्णांक		ip_wanted;		/* needed */
	पूर्णांक		*irq_handle;		/* for USB int requests */
	अचिन्हित पूर्णांक	irqpipe;		/* pipe for release_irq */
	अचिन्हित अक्षर	irqmaxp;		/* max packed for irq Pipe */
	अचिन्हित अक्षर	irqinterval;		/* Intervall for IRQ Pipe */
	संरचना scsi_cmd	*srb;			/* current srb */
	trans_reset	transport_reset;	/* reset routine */
	trans_cmnd	transport;		/* transport routine */
	अचिन्हित लघु	max_xfer_blk;		/* maximum transfer blocks */
	bool		cmd12;			/* use 12-byte commands (RBC/UFI) */
};

#जर !CONFIG_IS_ENABLED(BLK)
स्थिर संरचना us_data usb_stor[USB_MAX_STOR_DEV];
#endif

#define USB_STOR_TRANSPORT_GOOD	   0
#define USB_STOR_TRANSPORT_FAILED -1
#define USB_STOR_TRANSPORT_ERROR  -2

पूर्णांक usb_stor_get_info(संरचना usb_device *dev, संरचना us_data *us,
		      संरचना blk_desc *dev_desc);
पूर्णांक usb_storage_probe(संरचना usb_device *dev, अचिन्हित पूर्णांक ifnum,
		      संरचना us_data *ss);
#जर CONFIG_IS_ENABLED(BLK)
स्थिर अचिन्हित दीर्घ usb_stor_read(संरचना udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, शून्य *buffer);
स्थिर अचिन्हित दीर्घ usb_stor_write(संरचना udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, स्थिरांक शून्य *buffer);
#अन्यथा
स्थिर अचिन्हित दीर्घ usb_stor_read(संरचना blk_desc *block_dev, lbaint_t blknr,
				   lbaint_t blkcnt, शून्य *buffer);
स्थिर अचिन्हित दीर्घ usb_stor_write(संरचना blk_desc *block_dev, lbaint_t blknr,
				    lbaint_t blkcnt, स्थिरांक शून्य *buffer);
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
पूर्णांक usb_stor_info(शून्य)
{
	पूर्णांक मोजणी = 0;
#जर CONFIG_IS_ENABLED(BLK)
	संरचना udevice *dev;

	साठी (blk_first_device(UCLASS_USB, &dev);
	     dev;
	     blk_next_device(&dev)) {
		संरचना blk_desc *desc = dev_get_uclass_plat(dev);

		म_लिखो("  Device %d: ", desc->devnum);
		dev_print(desc);
		मोजणी++;
	}
#अन्यथा
	पूर्णांक i;

	जर (usb_max_devs > 0) {
		साठी (i = 0; i < usb_max_devs; i++) {
			म_लिखो("  Device %d: ", i);
			dev_print(&usb_dev_desc[i]);
		}
		परत 0;
	}
#endif
	जर (!मोजणी) {
		म_लिखो("No storage devices, perhaps not 'usb start'ed..?\n");
		परत 1;
	}

	परत 0;
}

स्थिर अचिन्हित पूर्णांक usb_get_max_lun(संरचना us_data *us)
{
	पूर्णांक len;
	ALLOC_CACHE_ALIGN_BUFFER(अचिन्हित अक्षर, result, 1);
	len = usb_control_संदेश(us->pusb_dev,
			      usb_rcvctrlpipe(us->pusb_dev, 0),
			      US_BBB_GET_MAX_LUN,
			      USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_IN,
			      0, us->ifnum,
			      result, आकार(अक्षर),
			      USB_CNTL_TIMEOUT * 5);
	debug("Get Max LUN -> len = %i, result = %i\n", len, (पूर्णांक) *result);
	परत (len > 0) ? *result : 0;
}

स्थिर पूर्णांक usb_stor_probe_device(संरचना usb_device *udev)
{
	पूर्णांक lun, max_lun;

#जर CONFIG_IS_ENABLED(BLK)
	संरचना us_data *data;
	पूर्णांक ret;
#अन्यथा
	पूर्णांक start;

	जर (udev == रिक्त)
		परत -ENOENT; /* no more devices available */
#endif

	debug("\n\nProbing for storage\n");
#जर CONFIG_IS_ENABLED(BLK)
	/*
	 * We store the us_data in the mass storage device's plat. It
	 * is shared by all LUNs (block devices) attached to this mass storage
	 * device.
	 */
	data = dev_get_plat(udev->dev);
	जर (!usb_storage_probe(udev, 0, data))
		परत 0;
	max_lun = usb_get_max_lun(data);
	साठी (lun = 0; lun <= max_lun; lun++) {
		संरचना blk_desc *blkdev;
		संरचना udevice *dev;
		अक्षर str[10];

		snprintf(str, आकार(str), "lun%d", lun);
		ret = blk_create_devicef(udev->dev, "usb_storage_blk", str,
					 UCLASS_USB, usb_max_devs,
					 DEFAULT_BLKSZ, 0, &dev);
		जर (ret) {
			debug("Cannot bind driver\n");
			परत ret;
		}

		blkdev = dev_get_uclass_plat(dev);
		blkdev->target = 0xff;
		blkdev->lun = lun;

		ret = usb_stor_get_info(udev, data, blkdev);
		जर (ret == 1) {
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		} अन्यथा {
			debug("usb_stor_get_info: Invalid device\n");
			ret = device_unbind(dev);
			जर (ret)
				परत ret;
			सुरूठेवा;
		}

		ret = blk_probe_or_unbind(dev);
		जर (ret)
			परत ret;

		ret = bootdev_setup_for_sibling_blk(dev, "usb_bootdev");
		जर (ret) {
			पूर्णांक ret2;

			ret2 = device_unbind(dev);
			जर (ret2)
				परत log_संदेश_ret("bootdev", ret2);
			परत log_संदेश_ret("bootdev", ret);
		}
	}
#अन्यथा
	/* We don't have space to even probe if we hit the maximum */
	जर (usb_max_devs == USB_MAX_STOR_DEV) {
		म_लिखो("max USB Storage Device reached: %d stopping\n",
		       usb_max_devs);
		परत -ENOSPC;
	}

	जर (!usb_storage_probe(udev, 0, &usb_stor[usb_max_devs]))
		परत 0;

	/*
	 * OK, it's a storage device.  Iterate over its LUNs and populate
	 * usb_dev_desc'
	 */
	start = usb_max_devs;

	max_lun = usb_get_max_lun(&usb_stor[usb_max_devs]);
	साठी (lun = 0; lun <= max_lun && usb_max_devs < USB_MAX_STOR_DEV;
	     lun++) {
		संरचना blk_desc *blkdev;

		blkdev = &usb_dev_desc[usb_max_devs];
		स_रखो(blkdev, '\0', आकार(संरचना blk_desc));
		blkdev->uclass_id = UCLASS_USB;
		blkdev->devnum = usb_max_devs;
		blkdev->part_type = PART_TYPE_UNKNOWN;
		blkdev->target = 0xff;
		blkdev->type = DEV_TYPE_UNKNOWN;
		blkdev->block_read = usb_stor_read;
		blkdev->block_write = usb_stor_write;
		blkdev->lun = lun;
		blkdev->priv = udev;

		जर (usb_stor_get_info(udev, &usb_stor[start],
				      &usb_dev_desc[usb_max_devs]) == 1) {
			debug("partype: %d\n", blkdev->part_type);
			part_init(blkdev);
			debug("partype: %d\n", blkdev->part_type);
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		}
	}
#endif

	परत 0;
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
पूर्णांक usb_stor_scan(पूर्णांक mode)
{
	जर (mode == 1)
		म_लिखो("       scanning usb for storage devices... ");

#जर !CONFIG_IS_ENABLED(DM_USB)
	अचिन्हित अक्षर i;

	usb_disable_asynch(1); /* asynch transfer not allowed */

	usb_stor_reset();
	साठी (i = 0; i < USB_MAX_DEVICE; i++) {
		संरचना usb_device *dev;

		dev = usb_get_dev_index(i); /* get device */
		debug("i=%d\n", i);
		जर (usb_stor_probe_device(dev))
			थांबा;
	} /* for */

	usb_disable_asynch(0); /* asynch transfer allowed */
#endif
	म_लिखो("%d Storage Device(s) found\n", usb_max_devs);
	जर (usb_max_devs > 0)
		परत 0;
	परत -1;
}

स्थिर पूर्णांक usb_stor_irq(संरचना usb_device *dev)
{
	संरचना us_data *us;
	us = (संरचना us_data *)dev->privptr;

	जर (us->ip_wanted)
		us->ip_wanted = 0;
	परत 0;
}

#ifdef	DEBUG

स्थिर शून्य usb_show_srb(संरचना scsi_cmd *pccb)
{
	पूर्णांक i;
	म_लिखो("SRB: len %d datalen 0x%lX\n ", pccb->cmdlen, pccb->datalen);
	साठी (i = 0; i < pccb->cmdlen; i++)
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

स्थिर पूर्णांक us_one_transfer(संरचना us_data *us, पूर्णांक pipe, अक्षर *buf, पूर्णांक length)
{
	पूर्णांक max_size;
	पूर्णांक this_xfer;
	पूर्णांक result;
	पूर्णांक partial;
	पूर्णांक maxtry;
	पूर्णांक stat;

	/* determine the maximum packet size for these transfers */
	max_size = usb_maxpacket(us->pusb_dev, pipe) * 16;

	/* while we have data left to transfer */
	पर्यंत (length) {

		/* calculate how long this will be -- maximum or a remainder */
		this_xfer = length > max_size ? max_size : length;
		length -= this_xfer;

		/* setup the retry counter */
		maxtry = 10;

		/* set up the transfer loop */
		करा {
			/* transfer the data */
			debug("Bulk xfer 0x%lx(%d) try #%d\n",
			      (ulong)map_to_sysmem(buf), this_xfer,
			      11 - maxtry);
			result = usb_bulk_संदेश(us->pusb_dev, pipe, buf,
					      this_xfer, &partial,
					      USB_CNTL_TIMEOUT * 5);
			debug("bulk_msg returned %d xferred %d/%d\n",
			      result, partial, this_xfer);
			जर (us->pusb_dev->status != 0) {
				/* if we stall, we need to clear it before
				 * we go on
				 */
#ifdef DEBUG
				display_int_status(us->pusb_dev->status);
#endif
				जर (us->pusb_dev->status & USB_ST_STALLED) {
					debug("stalled ->clearing endpoint" \
					      "halt for pipe 0x%x\n", pipe);
					stat = us->pusb_dev->status;
					usb_clear_halt(us->pusb_dev, pipe);
					us->pusb_dev->status = stat;
					जर (this_xfer == partial) {
						debug("bulk transferred" \
						      "with error %lX," \
						      " but data ok\n",
						      us->pusb_dev->status);
						परत 0;
					}
					अन्यथा
						परत result;
				}
				जर (us->pusb_dev->status & USB_ST_NAK_REC) {
					debug("Device NAKed bulk_msg\n");
					परत result;
				}
				debug("bulk transferred with error");
				जर (this_xfer == partial) {
					debug(" %ld, but data ok\n",
					      us->pusb_dev->status);
					परत 0;
				}
				/* if our try counter reaches 0, bail out */
				debug(" %ld, data %d\n",
				      us->pusb_dev->status, partial);
				जर (!maxtry--)
						परत result;
			}
			/* update to show what data was transferred */
			this_xfer -= partial;
			buf += partial;
			/* continue until this transfer is done */
		} पर्यंत (this_xfer);
	}

	/* if we get here, we're done and successful */
	परत 0;
}

स्थिर पूर्णांक usb_stor_BBB_reset(संरचना us_data *us)
{
	पूर्णांक result;
	अचिन्हित पूर्णांक pipe;

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
	result = usb_control_संदेश(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_BBB_RESET,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, रिक्त, 0, USB_CNTL_TIMEOUT * 5);

	जर ((result < 0) && (us->pusb_dev->status & USB_ST_STALLED)) {
		debug("RESET:stall\n");
		परत -1;
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
	परत 0;
}

/* FIXME: this reset function doesn't really reset the port, and it
 * should. Actually it should probably do what it's doing here, and
 * reset the port physically
 */
स्थिर पूर्णांक usb_stor_CB_reset(संरचना us_data *us)
{
	अचिन्हित अक्षर cmd[12];
	पूर्णांक result;

	debug("CB_reset\n");
	स_रखो(cmd, 0xff, आकार(cmd));
	cmd[0] = SCSI_SEND_DIAG;
	cmd[1] = 4;
	result = usb_control_संदेश(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_CBI_ADSC,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, cmd, आकार(cmd),
				 USB_CNTL_TIMEOUT * 5);

	/* long wait for reset */
	mdelay(1500);
	debug("CB_reset result %d: status %lX clearing endpoint halt\n",
	      result, us->pusb_dev->status);
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_in));
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_out));

	debug("CB_reset done\n");
	परत 0;
}

/*
 * Set up the command for a BBB device. Note that the actual SCSI
 * command is copied into cbw.CBWCDB.
 */
स्थिर पूर्णांक usb_stor_BBB_comdat(संरचना scsi_cmd *srb, संरचना us_data *us)
{
	पूर्णांक result;
	पूर्णांक actlen;
	पूर्णांक dir_in;
	अचिन्हित पूर्णांक pipe;
	ALLOC_CACHE_ALIGN_BUFFER(संरचना umass_bbb_cbw, cbw, 1);

	dir_in = US_DIRECTION(srb->cmd[0]);

#ifdef BBB_COMDAT_TRACE
	म_लिखो("dir %d lun %d cmdlen %d cmd %p datalen %lu pdata %p\n",
		dir_in, srb->lun, srb->cmdlen, srb->cmd, srb->datalen,
		srb->pdata);
	जर (srb->cmdlen) {
		साठी (result = 0; result < srb->cmdlen; result++)
			म_लिखो("cmd[%d] %#x ", result, srb->cmd[result]);
		म_लिखो("\n");
	}
#endif
	/* sanity checks */
	जर (!(srb->cmdlen <= CBWCDBLENGTH)) {
		debug("usb_stor_BBB_comdat:cmdlen too large\n");
		परत -1;
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
	result = usb_bulk_संदेश(us->pusb_dev, pipe, cbw, UMASS_BBB_CBW_SIZE,
			      &actlen, USB_CNTL_TIMEOUT * 5);
	जर (result < 0)
		debug("usb_stor_BBB_comdat:usb_bulk_msg error\n");
	परत result;
}

/* FIXME: we also need a CBI_command which sets up the completion
 * interrupt, and waits for it
 */
स्थिर पूर्णांक usb_stor_CB_comdat(संरचना scsi_cmd *srb, संरचना us_data *us)
{
	पूर्णांक result = 0;
	पूर्णांक dir_in, retry;
	अचिन्हित पूर्णांक pipe;
	अचिन्हित दीर्घ status;

	retry = 5;
	dir_in = US_DIRECTION(srb->cmd[0]);

	जर (dir_in)
		pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);
	अन्यथा
		pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_out);

	पर्यंत (retry--) {
		debug("CBI gets a command: Try %d\n", 5 - retry);
#ifdef DEBUG
		usb_show_srb(srb);
#endif
		/* let's send the command via the control pipe */
		result = usb_control_संदेश(us->pusb_dev,
					 usb_sndctrlpipe(us->pusb_dev , 0),
					 US_CBI_ADSC,
					 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
					 0, us->ifnum,
					 srb->cmd, srb->cmdlen,
					 USB_CNTL_TIMEOUT * 5);
		debug("CB_transport: control msg returned %d, status %lX\n",
		      result, us->pusb_dev->status);
		/* check the return code for the command */
		जर (result < 0) {
			जर (us->pusb_dev->status & USB_ST_STALLED) {
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
			परत result;
		}
		/* transfer the data payload for this command, if one exists*/

		debug("CB_transport: control msg returned %d," \
		      " direction is %s to go 0x%lx\n", result,
		      dir_in ? "IN" : "OUT", srb->datalen);
		जर (srb->datalen) {
			result = us_one_transfer(us, pipe, (अक्षर *)srb->pdata,
						 srb->datalen);
			debug("CBI attempted to transfer data," \
			      " result is %d status %lX, len %d\n",
			      result, us->pusb_dev->status,
				us->pusb_dev->act_len);
			जर (!(us->pusb_dev->status & USB_ST_NAK_REC))
				थांबा;
		} /* if (srb->datalen) */
		अन्यथा
			थांबा;
	}
	/* return result */

	परत result;
}

स्थिर पूर्णांक usb_stor_CBI_get_status(संरचना scsi_cmd *srb, संरचना us_data *us)
{
	पूर्णांक timeout;

	us->ip_wanted = 1;
	usb_int_संदेश(us->pusb_dev, us->irqpipe,
		    (शून्य *)&us->ip_data, us->irqmaxp, us->irqinterval, false);
	timeout = 1000;
	पर्यंत (timeout--) {
		जर (us->ip_wanted == 0)
			थांबा;
		mdelay(10);
	}
	जर (us->ip_wanted) {
		म_लिखो("	Did not get interrupt on CBI\n");
		us->ip_wanted = 0;
		परत USB_STOR_TRANSPORT_ERROR;
	}
	debug("Got interrupt data 0x%x, transferred %d status 0x%lX\n",
	      us->ip_data, us->pusb_dev->irq_act_len,
	      us->pusb_dev->irq_status);
	/* UFI gives us ASC and ASCQ, like a request sense */
	जर (us->subclass == US_SC_UFI) {
		जर (srb->cmd[0] == SCSI_REQ_SENSE ||
		    srb->cmd[0] == SCSI_INQUIRY)
			परत USB_STOR_TRANSPORT_GOOD; /* Good */
		अन्यथा जर (us->ip_data)
			परत USB_STOR_TRANSPORT_FAILED;
		अन्यथा
			परत USB_STOR_TRANSPORT_GOOD;
	}
	/* otherwise, we interpret the data normally */
	निवड (us->ip_data) {
	स्थिती 0x0001:
		परत USB_STOR_TRANSPORT_GOOD;
	स्थिती 0x0002:
		परत USB_STOR_TRANSPORT_FAILED;
	पूर्वनिर्धारित:
		परत USB_STOR_TRANSPORT_ERROR;
	}			/* switch */
	परत USB_STOR_TRANSPORT_ERROR;
}

#define USB_TRANSPORT_UNKNOWN_RETRY 5
#define USB_TRANSPORT_NOT_READY_RETRY 10

/* clear a stall on an endpoint - special for BBB devices */
स्थिर पूर्णांक usb_stor_BBB_clear_endpt_stall(संरचना us_data *us, __u8 endpt)
{
	/* ENDPOINT_HALT = 0, so set value to 0 */
	परत usb_control_संदेश(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
			       USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
			       endpt, रिक्त, 0, USB_CNTL_TIMEOUT * 5);
}

स्थिर पूर्णांक usb_stor_BBB_transport(संरचना scsi_cmd *srb, संरचना us_data *us)
{
	पूर्णांक result, retry;
	पूर्णांक dir_in;
	पूर्णांक actlen, data_actlen;
	अचिन्हित पूर्णांक pipe, pipein, pipeout;
	ALLOC_CACHE_ALIGN_BUFFER(संरचना umass_bbb_csw, csw, 1);
#ifdef BBB_XPORT_TRACE
	अचिन्हित अक्षर *ptr;
	पूर्णांक index;
#endif

	dir_in = US_DIRECTION(srb->cmd[0]);

	/* COMMAND phase */
	debug("COMMAND phase\n");
	result = usb_stor_BBB_comdat(srb, us);
	जर (result < 0) {
		debug("failed to send CBW status %ld\n",
		      us->pusb_dev->status);
		usb_stor_BBB_reset(us);
		परत USB_STOR_TRANSPORT_FAILED;
	}
	जर (!(us->flags & USB_READY))
		mdelay(5);
	pipein = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);
	pipeout = usb_sndbulkpipe(us->pusb_dev, us->ep_out);
	/* DATA phase + error handling */
	data_actlen = 0;
	/* no data, go immediately to the STATUS phase */
	जर (srb->datalen == 0)
		जा st;
	debug("DATA phase\n");
	जर (dir_in)
		pipe = pipein;
	अन्यथा
		pipe = pipeout;

	result = usb_bulk_संदेश(us->pusb_dev, pipe, srb->pdata, srb->datalen,
			      &data_actlen, USB_CNTL_TIMEOUT * 5);
	/* special handling of STALL in DATA phase */
	जर ((result < 0) && (us->pusb_dev->status & USB_ST_STALLED)) {
		debug("DATA:stall\n");
		/* clear the STALL on the endpoint */
		result = usb_stor_BBB_clear_endpt_stall(us,
					dir_in ? us->ep_in : us->ep_out);
		जर (result >= 0)
			/* continue on to STATUS phase */
			जा st;
	}
	जर (result < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->status);
		usb_stor_BBB_reset(us);
		परत USB_STOR_TRANSPORT_FAILED;
	}
#ifdef BBB_XPORT_TRACE
	साठी (index = 0; index < data_actlen; index++)
		म_लिखो("pdata[%d] %#x ", index, srb->pdata[index]);
	म_लिखो("\n");
#endif
	/* STATUS phase + error handling */
st:
	retry = 0;
again:
	debug("STATUS phase\n");
	result = usb_bulk_संदेश(us->pusb_dev, pipein, csw, UMASS_BBB_CSW_SIZE,
				&actlen, USB_CNTL_TIMEOUT*5);

	/* special handling of STALL in STATUS phase */
	जर ((result < 0) && (retry < 1) &&
	    (us->pusb_dev->status & USB_ST_STALLED)) {
		debug("STATUS:stall\n");
		/* clear the STALL on the endpoint */
		result = usb_stor_BBB_clear_endpt_stall(us, us->ep_in);
		जर (result >= 0 && (retry++ < 1))
			/* do a retry */
			जा again;
	}
	जर (result < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->status);
		usb_stor_BBB_reset(us);
		परत USB_STOR_TRANSPORT_FAILED;
	}
#ifdef BBB_XPORT_TRACE
	ptr = (अचिन्हित अक्षर *)csw;
	साठी (index = 0; index < UMASS_BBB_CSW_SIZE; index++)
		म_लिखो("ptr[%d] %#x ", index, ptr[index]);
	म_लिखो("\n");
#endif
	/* misuse pipe to get the residue */
	pipe = le32_to_cpu(csw->dCSWDataResidue);
	जर (pipe == 0 && srb->datalen != 0 && srb->datalen - data_actlen != 0)
		pipe = srb->datalen - data_actlen;
	जर (CSWSIGNATURE != le32_to_cpu(csw->dCSWSignature)) {
		debug("!CSWSIGNATURE\n");
		usb_stor_BBB_reset(us);
		परत USB_STOR_TRANSPORT_FAILED;
	} अन्यथा जर ((CBWTag - 1) != le32_to_cpu(csw->dCSWTag)) {
		debug("!Tag\n");
		usb_stor_BBB_reset(us);
		परत USB_STOR_TRANSPORT_FAILED;
	} अन्यथा जर (csw->bCSWStatus > CSWSTATUS_PHASE) {
		debug(">PHASE\n");
		usb_stor_BBB_reset(us);
		परत USB_STOR_TRANSPORT_FAILED;
	} अन्यथा जर (csw->bCSWStatus == CSWSTATUS_PHASE) {
		debug("=PHASE\n");
		usb_stor_BBB_reset(us);
		परत USB_STOR_TRANSPORT_FAILED;
	} अन्यथा जर (data_actlen > srb->datalen) {
		debug("transferred %dB instead of %ldB\n",
		      data_actlen, srb->datalen);
		परत USB_STOR_TRANSPORT_FAILED;
	} अन्यथा जर (csw->bCSWStatus == CSWSTATUS_FAILED) {
		debug("FAILED\n");
		परत USB_STOR_TRANSPORT_FAILED;
	}

	परत result;
}

स्थिर पूर्णांक usb_stor_CB_transport(संरचना scsi_cmd *srb, संरचना us_data *us)
{
	पूर्णांक result, status;
	संरचना scsi_cmd *psrb;
	संरचना scsi_cmd reqsrb;
	पूर्णांक retry, notready;

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
	जर (us->protocol == US_PR_CBI) {
		status = usb_stor_CBI_get_status(srb, us);
		/* if the status is error, report it */
		जर (status == USB_STOR_TRANSPORT_ERROR) {
			debug(" USB CBI Command Error\n");
			परत status;
		}
		srb->sense_buf[12] = (अचिन्हित अक्षर)(us->ip_data >> 8);
		srb->sense_buf[13] = (अचिन्हित अक्षर)(us->ip_data & 0xff);
		जर (!us->ip_data) {
			/* if the status is good, report it */
			जर (status == USB_STOR_TRANSPORT_GOOD) {
				debug(" USB CBI Command Good\n");
				परत status;
			}
		}
	}
	/* do we have to issue an auto request? */
	/* HERE we have to check the result */
	जर ((result < 0) && !(us->pusb_dev->status & USB_ST_STALLED)) {
		debug("ERROR %lX\n", us->pusb_dev->status);
		us->transport_reset(us);
		परत USB_STOR_TRANSPORT_ERROR;
	}
	जर ((us->protocol == US_PR_CBI) &&
	    ((srb->cmd[0] == SCSI_REQ_SENSE) ||
	    (srb->cmd[0] == SCSI_INQUIRY))) {
		/* do not issue an autorequest after request sense */
		debug("No auto request and good\n");
		परत USB_STOR_TRANSPORT_GOOD;
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
	जर (us->protocol == US_PR_CBI)
		status = usb_stor_CBI_get_status(psrb, us);

	जर ((result < 0) && !(us->pusb_dev->status & USB_ST_STALLED)) {
		debug(" AUTO REQUEST ERROR %ld\n",
		      us->pusb_dev->status);
		परत USB_STOR_TRANSPORT_ERROR;
	}
	debug("autorequest returned 0x%02X 0x%02X 0x%02X 0x%02X\n",
	      srb->sense_buf[0], srb->sense_buf[2],
	      srb->sense_buf[12], srb->sense_buf[13]);
	/* Check the auto request result */
	जर ((srb->sense_buf[2] == 0) &&
	    (srb->sense_buf[12] == 0) &&
	    (srb->sense_buf[13] == 0)) {
		/* ok, no sense */
		परत USB_STOR_TRANSPORT_GOOD;
	}

	/* Check the auto request result */
	निवड (srb->sense_buf[2]) {
	स्थिती 0x01:
		/* Recovered Error */
		परत USB_STOR_TRANSPORT_GOOD;
		थांबा;
	स्थिती 0x02:
		/* Not Ready */
		जर (notready++ > USB_TRANSPORT_NOT_READY_RETRY) {
			म_लिखो("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X (NOT READY)\n", srb->cmd[0],
				srb->sense_buf[0], srb->sense_buf[2],
				srb->sense_buf[12], srb->sense_buf[13]);
			परत USB_STOR_TRANSPORT_FAILED;
		} अन्यथा {
			mdelay(100);
			जा do_retry;
		}
		थांबा;
	पूर्वनिर्धारित:
		जर (retry++ > USB_TRANSPORT_UNKNOWN_RETRY) {
			म_लिखो("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X\n", srb->cmd[0], srb->sense_buf[0],
				srb->sense_buf[2], srb->sense_buf[12],
				srb->sense_buf[13]);
			परत USB_STOR_TRANSPORT_FAILED;
		} अन्यथा
			जा do_retry;
		थांबा;
	}
	परत USB_STOR_TRANSPORT_FAILED;
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

#जर CONFIG_IS_ENABLED(DM_USB)
	माप_प्रकार size;
	पूर्णांक ret;

	ret = usb_get_max_xfer_size(udev, (माप_प्रकार *)&size);
	जर ((ret >= 0) && (size < blk * 512))
		blk = size / 512;
#endif

	us->max_xfer_blk = blk;
}

स्थिर पूर्णांक usb_inquiry(संरचना scsi_cmd *srb, संरचना us_data *ss)
{
	पूर्णांक retry, i;
	retry = 5;
	करा {
		स_रखो(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_INQUIRY;
		srb->cmd[1] = srb->lun << 5;
		srb->cmd[4] = 36;
		srb->datalen = 36;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		i = ss->transport(srb, ss);
		debug("inquiry returns %d\n", i);
		जर (i == 0)
			थांबा;
	} पर्यंत (--retry);

	जर (!retry) {
		म_लिखो("error in inquiry\n");
		परत -1;
	}
	परत 0;
}

स्थिर पूर्णांक usb_request_sense(संरचना scsi_cmd *srb, संरचना us_data *ss)
{
	अक्षर *ptr;

	ptr = (अक्षर *)srb->pdata;
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
	परत 0;
}

स्थिर पूर्णांक usb_test_unit_ready(संरचना scsi_cmd *srb, संरचना us_data *ss)
{
	पूर्णांक retries = 10;

	करा {
		स_रखो(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_TST_U_RDY;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 0;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		जर (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD) {
			ss->flags |= USB_READY;
			परत 0;
		}
		usb_request_sense(srb, ss);
		/*
		 * Check the Key Code Qualifier, if it matches
		 * "Not Ready - medium not present"
		 * (the sense Key equals 0x2 and the ASC is 0x3a)
		 * return immediately as the medium being absent won't change
		 * unless there is a user action.
		 */
		जर ((srb->sense_buf[2] == 0x02) &&
		    (srb->sense_buf[12] == 0x3a))
			परत -1;
		mdelay(100);
	} पर्यंत (retries--);

	परत -1;
}

स्थिर पूर्णांक usb_read_capacity(संरचना scsi_cmd *srb, संरचना us_data *ss)
{
	पूर्णांक retry;
	/* XXX retries */
	retry = 3;
	करा {
		स_रखो(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_RD_CAPAC;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 8;
		srb->cmdlen = ss->cmd12 ? 12 : 10;
		जर (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD)
			परत 0;
	} पर्यंत (retry--);

	परत -1;
}

स्थिर पूर्णांक usb_read_10(संरचना scsi_cmd *srb, संरचना us_data *ss,
		       अचिन्हित दीर्घ start, अचिन्हित लघु blocks)
{
	स_रखो(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_READ10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((अचिन्हित अक्षर) (start >> 24)) & 0xff;
	srb->cmd[3] = ((अचिन्हित अक्षर) (start >> 16)) & 0xff;
	srb->cmd[4] = ((अचिन्हित अक्षर) (start >> 8)) & 0xff;
	srb->cmd[5] = ((अचिन्हित अक्षर) (start)) & 0xff;
	srb->cmd[7] = ((अचिन्हित अक्षर) (blocks >> 8)) & 0xff;
	srb->cmd[8] = (अचिन्हित अक्षर) blocks & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("read10: start %lx blocks %x\n", start, blocks);
	परत ss->transport(srb, ss);
}

स्थिर पूर्णांक usb_write_10(संरचना scsi_cmd *srb, संरचना us_data *ss,
			अचिन्हित दीर्घ start, अचिन्हित लघु blocks)
{
	स_रखो(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_WRITE10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((अचिन्हित अक्षर) (start >> 24)) & 0xff;
	srb->cmd[3] = ((अचिन्हित अक्षर) (start >> 16)) & 0xff;
	srb->cmd[4] = ((अचिन्हित अक्षर) (start >> 8)) & 0xff;
	srb->cmd[5] = ((अचिन्हित अक्षर) (start)) & 0xff;
	srb->cmd[7] = ((अचिन्हित अक्षर) (blocks >> 8)) & 0xff;
	srb->cmd[8] = (अचिन्हित अक्षर) blocks & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("write10: start %lx blocks %x\n", start, blocks);
	परत ss->transport(srb, ss);
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
				अचिन्हित अक्षर vendor[],
				अचिन्हित अक्षर product[]) {
	स्थिरांक अचिन्हित अक्षर max_vendor_len = 40;
	स्थिरांक अचिन्हित अक्षर max_product_len = 20;
	जर (descriptor.idVendor == 0x0424 && descriptor.idProduct == 0x223a) {
		म_नकलन((अक्षर *)vendor, "SMSC", max_vendor_len);
		म_नकलन((अक्षर *)product, "Flash Media Cntrller",
			max_product_len);
	}
}
#endif /* CONFIG_USB_BIN_FIXUP */

#जर CONFIG_IS_ENABLED(BLK)
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
	पूर्णांक retry;
	संरचना scsi_cmd *srb = &usb_ccb;
#जर CONFIG_IS_ENABLED(BLK)
	संरचना blk_desc *block_dev;
#endif

	जर (blkcnt == 0)
		परत 0;
	/* Setup  device */
#जर CONFIG_IS_ENABLED(BLK)
	block_dev = dev_get_uclass_plat(dev);
	udev = dev_get_parent_priv(dev_get_parent(dev));
	debug("\nusb_read: udev %d\n", block_dev->devnum);
#अन्यथा
	debug("\nusb_read: udev %d\n", block_dev->devnum);
	udev = usb_dev_desc[block_dev->devnum].priv;
	जर (!udev) {
		debug("%s: No device\n", __func__);
		परत 0;
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

	करा {
		/* XXX need some comment here */
		retry = 2;
		srb->pdata = (अचिन्हित अक्षर *)buf_addr;
		जर (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		अन्यथा
			smallblks = (अचिन्हित लघु) blks;
retry_it:
		जर (smallblks == ss->max_xfer_blk)
			usb_show_progress();
		srb->datalen = block_dev->blksz * smallblks;
		srb->pdata = (अचिन्हित अक्षर *)buf_addr;
		जर (usb_read_10(srb, ss, start, smallblks)) {
			debug("Read ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			जर (retry--)
				जा retry_it;
			blkcnt -= blks;
			थांबा;
		}
		start += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} पर्यंत (blks != 0);

	debug("usb_read: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      start, smallblks, buf_addr);

	usb_lock_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	जर (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	परत blkcnt;
}

#जर CONFIG_IS_ENABLED(BLK)
स्थिर अचिन्हित दीर्घ usb_stor_write(संरचना udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, स्थिरांक शून्य *buffer)
#अन्यथा
स्थिर अचिन्हित दीर्घ usb_stor_write(संरचना blk_desc *block_dev, lbaint_t blknr,
				    lbaint_t blkcnt, स्थिरांक शून्य *buffer)
#endif
{
	lbaint_t start, blks;
	uintptr_t buf_addr;
	अचिन्हित लघु smallblks;
	संरचना usb_device *udev;
	संरचना us_data *ss;
	पूर्णांक retry;
	संरचना scsi_cmd *srb = &usb_ccb;
#जर CONFIG_IS_ENABLED(BLK)
	संरचना blk_desc *block_dev;
#endif

	जर (blkcnt == 0)
		परत 0;

	/* Setup  device */
#जर CONFIG_IS_ENABLED(BLK)
	block_dev = dev_get_uclass_plat(dev);
	udev = dev_get_parent_priv(dev_get_parent(dev));
	debug("\nusb_read: udev %d\n", block_dev->devnum);
#अन्यथा
	debug("\nusb_read: udev %d\n", block_dev->devnum);
	udev = usb_dev_desc[block_dev->devnum].priv;
	जर (!udev) {
		debug("%s: No device\n", __func__);
		परत 0;
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

	करा {
		/* If write fails retry for max retry count else
		 * return with number of blocks written successfully.
		 */
		retry = 2;
		srb->pdata = (अचिन्हित अक्षर *)buf_addr;
		जर (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		अन्यथा
			smallblks = (अचिन्हित लघु) blks;
retry_it:
		जर (smallblks == ss->max_xfer_blk)
			usb_show_progress();
		srb->datalen = block_dev->blksz * smallblks;
		srb->pdata = (अचिन्हित अक्षर *)buf_addr;
		जर (usb_write_10(srb, ss, start, smallblks)) {
			debug("Write ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			जर (retry--)
				जा retry_it;
			blkcnt -= blks;
			थांबा;
		}
		start += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} पर्यंत (blks != 0);

	debug("usb_write: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      start, smallblks, buf_addr);

	usb_lock_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	जर (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	परत blkcnt;

}

/* Probe to see if a new device is actually a Storage device */
पूर्णांक usb_storage_probe(संरचना usb_device *dev, अचिन्हित पूर्णांक ifnum,
		      संरचना us_data *ss)
{
	संरचना usb_interface *iface;
	पूर्णांक i;
	संरचना usb_endpoint_descriptor *ep_desc;
	अचिन्हित पूर्णांक flags = 0;

	/* let's examine the device now */
	iface = &dev->config.if_desc[ifnum];

	जर (dev->descriptor.bDeviceClass != 0 ||
			iface->desc.bInterfaceClass != USB_CLASS_MASS_STORAGE ||
			iface->desc.bInterfaceSubClass < US_SC_MIN ||
			iface->desc.bInterfaceSubClass > US_SC_MAX) {
		debug("Not mass storage\n");
		/* if it's not a mass storage, we go no further */
		परत 0;
	}

	स_रखो(ss, 0, आकार(संरचना us_data));

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
	निवड (ss->protocol) {
	स्थिती US_PR_CB:
		debug("Control/Bulk\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		थांबा;

	स्थिती US_PR_CBI:
		debug("Control/Bulk/Interrupt\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		थांबा;
	स्थिती US_PR_BULK:
		debug("Bulk/Bulk/Bulk\n");
		ss->transport = usb_stor_BBB_transport;
		ss->transport_reset = usb_stor_BBB_reset;
		थांबा;
	पूर्वनिर्धारित:
		म_लिखो("USB Storage Transport unknown / not yet implemented\n");
		परत 0;
		थांबा;
	}

	/*
	 * We are expecting a minimum of 2 endpoints - in and out (bulk).
	 * An optional interrupt is OK (necessary for CBI protocol).
	 * We will ignore any others.
	 */
	साठी (i = 0; i < iface->desc.b_अंक_Endpoints; i++) {
		ep_desc = &iface->ep_desc[i];
		/* is it an BULK endpoint? */
		जर ((ep_desc->bmAttributes &
		     USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK) {
			जर (ep_desc->bEndpointAddress & USB_DIR_IN)
				ss->ep_in = ep_desc->bEndpointAddress &
						USB_ENDPOINT_संख्या_MASK;
			अन्यथा
				ss->ep_out =
					ep_desc->bEndpointAddress &
					USB_ENDPOINT_संख्या_MASK;
		}

		/* is it an interrupt endpoint? */
		जर ((ep_desc->bmAttributes &
		     USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT) {
			ss->ep_int = ep_desc->bEndpointAddress &
						USB_ENDPOINT_संख्या_MASK;
			ss->irqinterval = ep_desc->bInterval;
		}
	}
	debug("Endpoints In %d Out %d Int %d\n",
	      ss->ep_in, ss->ep_out, ss->ep_int);

	/* Do some basic sanity checks, and bail if we find a problem */
	जर (usb_set_interface(dev, iface->desc.b_Interface_संख्या, 0) ||
	    !ss->ep_in || !ss->ep_out ||
	    (ss->protocol == US_PR_CBI && ss->ep_int == 0)) {
		debug("Problems with device\n");
		परत 0;
	}
	/* set class specific stuff */
	/* We only handle certain protocols.  Currently, these are
	 * the only ones.
	 * The SFF8070 accepts the requests used in u-boot
	 */
	जर (ss->subclass != US_SC_UFI && ss->subclass != US_SC_SCSI &&
	    ss->subclass != US_SC_8070) {
		म_लिखो("Sorry, protocol %d not yet supported.\n", ss->subclass);
		परत 0;
	}

	/* UFI uses 12-byte commands (like RBC, unlike SCSI) */
	जर (ss->subclass == US_SC_UFI)
		ss->cmd12 = true;

	जर (ss->ep_int) {
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
	परत 1;
}

पूर्णांक usb_stor_get_info(संरचना usb_device *dev, संरचना us_data *ss,
		      संरचना blk_desc *dev_desc)
{
	अचिन्हित अक्षर perq, modi;
	ALLOC_CACHE_ALIGN_BUFFER(u32, cap, 2);
	ALLOC_CACHE_ALIGN_BUFFER(u8, usb_stor_buf, 36);
	u32 capacity, blksz;
	संरचना scsi_cmd *pccb = &usb_ccb;

	pccb->pdata = usb_stor_buf;

	dev_desc->target = dev->devnum;
	pccb->lun = dev_desc->lun;
	debug(" address %d\n", dev_desc->target);

	जर (usb_inquiry(pccb, ss)) {
		debug("%s: usb_inquiry() failed\n", __func__);
		परत -1;
	}

	perq = usb_stor_buf[0];
	modi = usb_stor_buf[1];

	/*
	 * Skip unknown devices (0x1f) and enclosure service devices (0x0d),
	 * they would not respond to test_unit_ready .
	 */
	जर (((perq & 0x1f) == 0x1f) || ((perq & 0x1f) == 0x0d)) {
		debug("%s: unknown/unsupported device\n", __func__);
		परत 0;
	}
	जर ((modi&0x80) == 0x80) {
		/* drive is removable */
		dev_desc->removable = 1;
	}
	स_नकल(dev_desc->vendor, (स्थिरांक शून्य *)&usb_stor_buf[8], 8);
	स_नकल(dev_desc->product, (स्थिरांक शून्य *)&usb_stor_buf[16], 16);
	स_नकल(dev_desc->revision, (स्थिरांक शून्य *)&usb_stor_buf[32], 4);
	dev_desc->vendor[8] = 0;
	dev_desc->product[16] = 0;
	dev_desc->revision[4] = 0;
#ifdef CONFIG_USB_BIN_FIXUP
	usb_bin_fixup(dev->descriptor, (uchar *)dev_desc->vendor,
		      (uchar *)dev_desc->product);
#endif /* CONFIG_USB_BIN_FIXUP */
	debug("ISO Vers %X, Response Data %X\n", usb_stor_buf[2],
	      usb_stor_buf[3]);
	जर (usb_test_unit_ready(pccb, ss)) {
		म_लिखो("Device NOT ready\n"
		       "   Request Sense returned %02X %02X %02X\n",
		       pccb->sense_buf[2], pccb->sense_buf[12],
		       pccb->sense_buf[13]);
		जर (dev_desc->removable == 1)
			dev_desc->type = perq;
		परत 0;
	}
	pccb->pdata = (अचिन्हित अक्षर *)cap;
	स_रखो(pccb->pdata, 0, 8);
	जर (usb_read_capacity(pccb, ss) != 0) {
		म_लिखो("READ_CAP ERROR\n");
		ss->flags &= ~USB_READY;
		cap[0] = 2880;
		cap[1] = 0x200;
	}
	debug("Read Capacity returns: 0x%08x, 0x%08x\n", cap[0], cap[1]);
#जर 0
	जर (cap[0] > (0x200000 * 10)) /* greater than 10 GByte */
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

	परत 1;
}

#जर CONFIG_IS_ENABLED(DM_USB)

स्थिर पूर्णांक usb_mass_storage_probe(संरचना udevice *dev)
{
	संरचना usb_device *udev = dev_get_parent_priv(dev);
	पूर्णांक ret;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	ret = usb_stor_probe_device(udev);
	usb_disable_asynch(0); /* asynch transfer allowed */

	परत ret;
}

स्थिर स्थिरांक संरचना udevice_id usb_mass_storage_ids[] = {
	{ .compatible = "usb-mass-storage" },
	{ }
};

U_BOOT_DRIVER(usb_mass_storage) = {
	.name	= "usb_mass_storage",
	.id	= UCLASS_MASS_STORAGE,
	.of_match = usb_mass_storage_ids,
	.probe = usb_mass_storage_probe,
#जर CONFIG_IS_ENABLED(BLK)
	.plat_auto	= आकार(संरचना us_data),
#endif
};

UCLASS_DRIVER(usb_mass_storage) = {
	.id		= UCLASS_MASS_STORAGE,
	.name		= "usb_mass_storage",
};

स्थिर स्थिरांक संरचना usb_device_id mass_storage_id_table[] = {
	{
		.match_flags = USB_DEVICE_ID_MATCH_INT_CLASS,
		.bInterfaceClass = USB_CLASS_MASS_STORAGE
	},
	{ }		/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_mass_storage, mass_storage_id_table);
#endif

#जर CONFIG_IS_ENABLED(BLK)
स्थिर स्थिरांक संरचना blk_ops usb_storage_ops = {
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
