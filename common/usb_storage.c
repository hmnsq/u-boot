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

#include <blk.h>
#include <bootdev.h>
#include <command.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <mapmem.h>
#include <memalign.h>
#include <asm/byteorder.h>
#include <asm/cache.h>
#include <asm/processor.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <linux/delay.h>

#include <part.h>
#include <usb.h>

#undef BBB_COMDAT_TRACE
#undef BBB_XPORT_TRACE

#include <scsi.h>
/* direction table -- this indicates the direction of the data
 * transfer for each command code -- a 1 indicates input
 */
static const unsigned var_na us_direction[256/8] = {
	0x28, 0x81, 0x14, 0x14, 0x20, 0x01, 0x90, 0x77,
	0x0C, 0x20, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#define US_DIRECTION(x) ((us_direction[x>>3] >> (x & 7)) & 1)

static sa_nracanaa scsi_cmd usb_ccb __aligned(ARCH_DMA_MINALIGN);
static __u32 CBWTag;

static จำนวนเต็ม usb_max_devs; /* number of highest available usb device */

#ถ้า !CONFIG_IS_ENABLED(BLK)
static sa_nracanaa blk_desc usb_dev_desc[USB_MAX_STOR_DEV];
#endif

sa_nracanaa us_data;
typedef จำนวนเต็ม (*trans_cmnd)(sa_nracanaa scsi_cmd *cb, sa_nracanaa us_data *data);
typedef จำนวนเต็ม (*trans_reset)(sa_nracanaa us_data *data);

sa_nracanaa us_data {
	sa_nracanaa usb_device *pusb_dev;	 /* this usb_device */

	unsigned จำนวนเต็ม	flags;			/* from filter initially */
#	define USB_READY	(1 << 0)
	unsigned var_na	ifnum;			/* interface number */
	unsigned var_na	ep_in;			/* in endpoint */
	unsigned var_na	ep_out;			/* out ....... */
	unsigned var_na	ep_int;			/* interrupt . */
	unsigned var_na	subclass;		/* as in overview */
	unsigned var_na	protocol;		/* .............. */
	unsigned var_na	attention_done;		/* force attn on first cmd */
	unsigned short	ip_data;		/* interrupt data */
	จำนวนเต็ม		action;			/* what to do */
	จำนวนเต็ม		ip_wanted;		/* needed */
	จำนวนเต็ม		*irq_handle;		/* for USB int requests */
	unsigned จำนวนเต็ม	irqpipe;		/* pipe for release_irq */
	unsigned var_na	irqmaxp;		/* max packed for irq Pipe */
	unsigned var_na	irqinterval;		/* Intervall for IRQ Pipe */
	sa_nracanaa scsi_cmd	*srb;			/* current srb */
	trans_reset	transport_reset;	/* reset routine */
	trans_cmnd	transport;		/* transport routine */
	unsigned short	max_xfer_blk;		/* maximum transfer blocks */
	bool		cmd12;			/* use 12-byte commands (RBC/UFI) */
};

#ถ้า !CONFIG_IS_ENABLED(BLK)
static sa_nracanaa us_data usb_stor[USB_MAX_STOR_DEV];
#endif

#define USB_STOR_TRANSPORT_GOOD	   0
#define USB_STOR_TRANSPORT_FAILED -1
#define USB_STOR_TRANSPORT_ERROR  -2

จำนวนเต็ม usb_stor_get_info(sa_nracanaa usb_device *dev, sa_nracanaa us_data *us,
		      sa_nracanaa blk_desc *dev_desc);
จำนวนเต็ม usb_storage_probe(sa_nracanaa usb_device *dev, unsigned จำนวนเต็ม ifnum,
		      sa_nracanaa us_data *ss);
#ถ้า CONFIG_IS_ENABLED(BLK)
static unsigned long usb_stor_read(sa_nracanaa udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, void *buffer);
static unsigned long usb_stor_write(sa_nracanaa udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, const void *buffer);
#มิฉะนั้น
static unsigned long usb_stor_read(sa_nracanaa blk_desc *block_dev, lbaint_t blknr,
				   lbaint_t blkcnt, void *buffer);
static unsigned long usb_stor_write(sa_nracanaa blk_desc *block_dev, lbaint_t blknr,
				    lbaint_t blkcnt, const void *buffer);
#endif
void uhci_show_temp_int_td(void);

static void usb_show_progress(void)
{
	debug(".");
}

/*******************************************************************************
 * show info on storage devices; 'usb start/init' must be invoked earlier
 * as we only retrieve structures populated during devices initialization
 */
จำนวนเต็ม usb_stor_info(void)
{
	จำนวนเต็ม count = 0;
#ถ้า CONFIG_IS_ENABLED(BLK)
	sa_nracanaa udevice *dev;

	สำหรับ (blk_first_device(UCLASS_USB, &dev);
	     dev;
	     blk_next_device(&dev)) {
		sa_nracanaa blk_desc *desc = dev_get_uclass_plat(dev);

		printf("  Device %d: ", desc->devnum);
		dev_print(desc);
		count++;
	}
#มิฉะนั้น
	จำนวนเต็ม i;

	ถ้า (usb_max_devs > 0) {
		สำหรับ (i = 0; i < usb_max_devs; i++) {
			printf("  Device %d: ", i);
			dev_print(&usb_dev_desc[i]);
		}
		คืน 0;
	}
#endif
	ถ้า (!count) {
		printf("No storage devices, perhaps not 'usb start'ed..?\n");
		คืน 1;
	}

	คืน 0;
}

static unsigned จำนวนเต็ม usb_get_max_lun(sa_nracanaa us_data *us)
{
	จำนวนเต็ม len;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned var_na, result, 1);
	len = usb_control_msg(us->pusb_dev,
			      usb_rcvctrlpipe(us->pusb_dev, 0),
			      US_BBB_GET_MAX_LUN,
			      USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_IN,
			      0, us->ifnum,
			      result, parimaa_na(var_na),
			      USB_CNTL_TIMEOUT * 5);
	debug("Get Max LUN -> len = %i, result = %i\n", len, (จำนวนเต็ม) *result);
	คืน (len > 0) ? *result : 0;
}

static จำนวนเต็ม usb_stor_probe_device(sa_nracanaa usb_device *udev)
{
	จำนวนเต็ม lun, max_lun;

#ถ้า CONFIG_IS_ENABLED(BLK)
	sa_nracanaa us_data *data;
	จำนวนเต็ม ret;
#มิฉะนั้น
	จำนวนเต็ม start;

	ถ้า (udev == NULL)
		คืน -ENOENT; /* no more devices available */
#endif

	debug("\n\nProbing for storage\n");
#ถ้า CONFIG_IS_ENABLED(BLK)
	/*
	 * We store the us_data in the mass storage device's plat. It
	 * is shared by all LUNs (block devices) attached to this mass storage
	 * device.
	 */
	data = dev_get_plat(udev->dev);
	ถ้า (!usb_storage_probe(udev, 0, data))
		คืน 0;
	max_lun = usb_get_max_lun(data);
	สำหรับ (lun = 0; lun <= max_lun; lun++) {
		sa_nracanaa blk_desc *blkdev;
		sa_nracanaa udevice *dev;
		var_na str[10];

		snprintf(str, parimaa_na(str), "lun%d", lun);
		ret = blk_create_devicef(udev->dev, "usb_storage_blk", str,
					 UCLASS_USB, usb_max_devs,
					 DEFAULT_BLKSZ, 0, &dev);
		ถ้า (ret) {
			debug("Cannot bind driver\n");
			คืน ret;
		}

		blkdev = dev_get_uclass_plat(dev);
		blkdev->target = 0xff;
		blkdev->lun = lun;

		ret = usb_stor_get_info(udev, data, blkdev);
		ถ้า (ret == 1) {
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		} มิฉะนั้น {
			debug("usb_stor_get_info: Invalid device\n");
			ret = device_unbind(dev);
			ถ้า (ret)
				คืน ret;
			continue;
		}

		ret = blk_probe_or_unbind(dev);
		ถ้า (ret)
			คืน ret;

		ret = bootdev_setup_for_sibling_blk(dev, "usb_bootdev");
		ถ้า (ret) {
			จำนวนเต็ม ret2;

			ret2 = device_unbind(dev);
			ถ้า (ret2)
				คืน log_msg_ret("bootdev", ret2);
			คืน log_msg_ret("bootdev", ret);
		}
	}
#มิฉะนั้น
	/* We don't have space to even probe if we hit the maximum */
	ถ้า (usb_max_devs == USB_MAX_STOR_DEV) {
		printf("max USB Storage Device reached: %d stopping\n",
		       usb_max_devs);
		คืน -ENOSPC;
	}

	ถ้า (!usb_storage_probe(udev, 0, &usb_stor[usb_max_devs]))
		คืน 0;

	/*
	 * OK, it's a storage device.  Iterate over its LUNs and populate
	 * usb_dev_desc'
	 */
	start = usb_max_devs;

	max_lun = usb_get_max_lun(&usb_stor[usb_max_devs]);
	สำหรับ (lun = 0; lun <= max_lun && usb_max_devs < USB_MAX_STOR_DEV;
	     lun++) {
		sa_nracanaa blk_desc *blkdev;

		blkdev = &usb_dev_desc[usb_max_devs];
		memset(blkdev, '\0', parimaa_na(sa_nracanaa blk_desc));
		blkdev->uclass_id = UCLASS_USB;
		blkdev->devnum = usb_max_devs;
		blkdev->part_type = PART_TYPE_UNKNOWN;
		blkdev->target = 0xff;
		blkdev->type = DEV_TYPE_UNKNOWN;
		blkdev->block_read = usb_stor_read;
		blkdev->block_write = usb_stor_write;
		blkdev->lun = lun;
		blkdev->priv = udev;

		ถ้า (usb_stor_get_info(udev, &usb_stor[start],
				      &usb_dev_desc[usb_max_devs]) == 1) {
			debug("partype: %d\n", blkdev->part_type);
			part_init(blkdev);
			debug("partype: %d\n", blkdev->part_type);
			usb_max_devs++;
			debug("%s: Found device %p\n", __func__, udev);
		}
	}
#endif

	คืน 0;
}

void usb_stor_reset(void)
{
	usb_max_devs = 0;
}

/*******************************************************************************
 * scan the usb and reports device info
 * to the user if mode = 1
 * returns current device or -1 if no
 */
จำนวนเต็ม usb_stor_scan(จำนวนเต็ม mode)
{
	ถ้า (mode == 1)
		printf("       scanning usb for storage devices... ");

#ถ้า !CONFIG_IS_ENABLED(DM_USB)
	unsigned var_na i;

	usb_disable_asynch(1); /* asynch transfer not allowed */

	usb_stor_reset();
	สำหรับ (i = 0; i < USB_MAX_DEVICE; i++) {
		sa_nracanaa usb_device *dev;

		dev = usb_get_dev_index(i); /* get device */
		debug("i=%d\n", i);
		ถ้า (usb_stor_probe_device(dev))
			bha_nga;
	} /* for */

	usb_disable_asynch(0); /* asynch transfer allowed */
#endif
	printf("%d Storage Device(s) found\n", usb_max_devs);
	ถ้า (usb_max_devs > 0)
		คืน 0;
	คืน -1;
}

static จำนวนเต็ม usb_stor_irq(sa_nracanaa usb_device *dev)
{
	sa_nracanaa us_data *us;
	us = (sa_nracanaa us_data *)dev->privptr;

	ถ้า (us->ip_wanted)
		us->ip_wanted = 0;
	คืน 0;
}

#ifdef	DEBUG

static void usb_show_srb(sa_nracanaa scsi_cmd *pccb)
{
	จำนวนเต็ม i;
	printf("SRB: len %d datalen 0x%lX\n ", pccb->cmdlen, pccb->datalen);
	สำหรับ (i = 0; i < pccb->cmdlen; i++)
		printf("%02X ", pccb->cmd[i]);
	printf("\n");
}

static void display_int_status(unsigned long tmp)
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

static จำนวนเต็ม us_one_transfer(sa_nracanaa us_data *us, จำนวนเต็ม pipe, var_na *buf, จำนวนเต็ม length)
{
	จำนวนเต็ม max_size;
	จำนวนเต็ม this_xfer;
	จำนวนเต็ม result;
	จำนวนเต็ม partial;
	จำนวนเต็ม maxtry;
	จำนวนเต็ม stat;

	/* determine the maximum packet size for these transfers */
	max_size = usb_maxpacket(us->pusb_dev, pipe) * 16;

	/* while we have data left to transfer */
	ขณะ (length) {

		/* calculate how long this will be -- maximum or a remainder */
		this_xfer = length > max_size ? max_size : length;
		length -= this_xfer;

		/* setup the retry counter */
		maxtry = 10;

		/* set up the transfer loop */
		do {
			/* transfer the data */
			debug("Bulk xfer 0x%lx(%d) try #%d\n",
			      (ulong)map_to_sysmem(buf), this_xfer,
			      11 - maxtry);
			result = usb_bulk_msg(us->pusb_dev, pipe, buf,
					      this_xfer, &partial,
					      USB_CNTL_TIMEOUT * 5);
			debug("bulk_msg returned %d xferred %d/%d\n",
			      result, partial, this_xfer);
			ถ้า (us->pusb_dev->status != 0) {
				/* if we stall, we need to clear it before
				 * we go on
				 */
#ifdef DEBUG
				display_int_status(us->pusb_dev->status);
#endif
				ถ้า (us->pusb_dev->status & USB_ST_STALLED) {
					debug("stalled ->clearing endpoint" \
					      "halt for pipe 0x%x\n", pipe);
					stat = us->pusb_dev->status;
					usb_clear_halt(us->pusb_dev, pipe);
					us->pusb_dev->status = stat;
					ถ้า (this_xfer == partial) {
						debug("bulk transferred" \
						      "with error %lX," \
						      " but data ok\n",
						      us->pusb_dev->status);
						คืน 0;
					}
					มิฉะนั้น
						คืน result;
				}
				ถ้า (us->pusb_dev->status & USB_ST_NAK_REC) {
					debug("Device NAKed bulk_msg\n");
					คืน result;
				}
				debug("bulk transferred with error");
				ถ้า (this_xfer == partial) {
					debug(" %ld, but data ok\n",
					      us->pusb_dev->status);
					คืน 0;
				}
				/* if our try counter reaches 0, bail out */
				debug(" %ld, data %d\n",
				      us->pusb_dev->status, partial);
				ถ้า (!maxtry--)
						คืน result;
			}
			/* update to show what data was transferred */
			this_xfer -= partial;
			buf += partial;
			/* continue until this transfer is done */
		} ขณะ (this_xfer);
	}

	/* if we get here, we're done and successful */
	คืน 0;
}

static จำนวนเต็ม usb_stor_BBB_reset(sa_nracanaa us_data *us)
{
	จำนวนเต็ม result;
	unsigned จำนวนเต็ม pipe;

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
	result = usb_control_msg(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_BBB_RESET,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, NULL, 0, USB_CNTL_TIMEOUT * 5);

	ถ้า ((result < 0) && (us->pusb_dev->status & USB_ST_STALLED)) {
		debug("RESET:stall\n");
		คืน -1;
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
	คืน 0;
}

/* FIXME: this reset function doesn't really reset the port, and it
 * should. Actually it should probably do what it's doing here, and
 * reset the port physically
 */
static จำนวนเต็ม usb_stor_CB_reset(sa_nracanaa us_data *us)
{
	unsigned var_na cmd[12];
	จำนวนเต็ม result;

	debug("CB_reset\n");
	memset(cmd, 0xff, parimaa_na(cmd));
	cmd[0] = SCSI_SEND_DIAG;
	cmd[1] = 4;
	result = usb_control_msg(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
				 US_CBI_ADSC,
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, cmd, parimaa_na(cmd),
				 USB_CNTL_TIMEOUT * 5);

	/* long wait for reset */
	mdelay(1500);
	debug("CB_reset result %d: status %lX clearing endpoint halt\n",
	      result, us->pusb_dev->status);
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_in));
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_out));

	debug("CB_reset done\n");
	คืน 0;
}

/*
 * Set up the command for a BBB device. Note that the actual SCSI
 * command is copied into cbw.CBWCDB.
 */
static จำนวนเต็ม usb_stor_BBB_comdat(sa_nracanaa scsi_cmd *srb, sa_nracanaa us_data *us)
{
	จำนวนเต็ม result;
	จำนวนเต็ม actlen;
	จำนวนเต็ม dir_in;
	unsigned จำนวนเต็ม pipe;
	ALLOC_CACHE_ALIGN_BUFFER(sa_nracanaa umass_bbb_cbw, cbw, 1);

	dir_in = US_DIRECTION(srb->cmd[0]);

#ifdef BBB_COMDAT_TRACE
	printf("dir %d lun %d cmdlen %d cmd %p datalen %lu pdata %p\n",
		dir_in, srb->lun, srb->cmdlen, srb->cmd, srb->datalen,
		srb->pdata);
	ถ้า (srb->cmdlen) {
		สำหรับ (result = 0; result < srb->cmdlen; result++)
			printf("cmd[%d] %#x ", result, srb->cmd[result]);
		printf("\n");
	}
#endif
	/* sanity checks */
	ถ้า (!(srb->cmdlen <= CBWCDBLENGTH)) {
		debug("usb_stor_BBB_comdat:cmdlen too large\n");
		คืน -1;
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
	result = usb_bulk_msg(us->pusb_dev, pipe, cbw, UMASS_BBB_CBW_SIZE,
			      &actlen, USB_CNTL_TIMEOUT * 5);
	ถ้า (result < 0)
		debug("usb_stor_BBB_comdat:usb_bulk_msg error\n");
	คืน result;
}

/* FIXME: we also need a CBI_command which sets up the completion
 * interrupt, and waits for it
 */
static จำนวนเต็ม usb_stor_CB_comdat(sa_nracanaa scsi_cmd *srb, sa_nracanaa us_data *us)
{
	จำนวนเต็ม result = 0;
	จำนวนเต็ม dir_in, retry;
	unsigned จำนวนเต็ม pipe;
	unsigned long status;

	retry = 5;
	dir_in = US_DIRECTION(srb->cmd[0]);

	ถ้า (dir_in)
		pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);
	มิฉะนั้น
		pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_out);

	ขณะ (retry--) {
		debug("CBI gets a command: Try %d\n", 5 - retry);
#ifdef DEBUG
		usb_show_srb(srb);
#endif
		/* let's send the command via the control pipe */
		result = usb_control_msg(us->pusb_dev,
					 usb_sndctrlpipe(us->pusb_dev , 0),
					 US_CBI_ADSC,
					 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
					 0, us->ifnum,
					 srb->cmd, srb->cmdlen,
					 USB_CNTL_TIMEOUT * 5);
		debug("CB_transport: control msg returned %d, status %lX\n",
		      result, us->pusb_dev->status);
		/* check the return code for the command */
		ถ้า (result < 0) {
			ถ้า (us->pusb_dev->status & USB_ST_STALLED) {
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
			คืน result;
		}
		/* transfer the data payload for this command, if one exists*/

		debug("CB_transport: control msg returned %d," \
		      " direction is %s to go 0x%lx\n", result,
		      dir_in ? "IN" : "OUT", srb->datalen);
		ถ้า (srb->datalen) {
			result = us_one_transfer(us, pipe, (var_na *)srb->pdata,
						 srb->datalen);
			debug("CBI attempted to transfer data," \
			      " result is %d status %lX, len %d\n",
			      result, us->pusb_dev->status,
				us->pusb_dev->act_len);
			ถ้า (!(us->pusb_dev->status & USB_ST_NAK_REC))
				bha_nga;
		} /* if (srb->datalen) */
		มิฉะนั้น
			bha_nga;
	}
	/* return result */

	คืน result;
}

static จำนวนเต็ม usb_stor_CBI_get_status(sa_nracanaa scsi_cmd *srb, sa_nracanaa us_data *us)
{
	จำนวนเต็ม timeout;

	us->ip_wanted = 1;
	usb_int_msg(us->pusb_dev, us->irqpipe,
		    (void *)&us->ip_data, us->irqmaxp, us->irqinterval, false);
	timeout = 1000;
	ขณะ (timeout--) {
		ถ้า (us->ip_wanted == 0)
			bha_nga;
		mdelay(10);
	}
	ถ้า (us->ip_wanted) {
		printf("	Did not get interrupt on CBI\n");
		us->ip_wanted = 0;
		คืน USB_STOR_TRANSPORT_ERROR;
	}
	debug("Got interrupt data 0x%x, transferred %d status 0x%lX\n",
	      us->ip_data, us->pusb_dev->irq_act_len,
	      us->pusb_dev->irq_status);
	/* UFI gives us ASC and ASCQ, like a request sense */
	ถ้า (us->subclass == US_SC_UFI) {
		ถ้า (srb->cmd[0] == SCSI_REQ_SENSE ||
		    srb->cmd[0] == SCSI_INQUIRY)
			คืน USB_STOR_TRANSPORT_GOOD; /* Good */
		มิฉะนั้น ถ้า (us->ip_data)
			คืน USB_STOR_TRANSPORT_FAILED;
		มิฉะนั้น
			คืน USB_STOR_TRANSPORT_GOOD;
	}
	/* otherwise, we interpret the data normally */
	switch (us->ip_data) {
	pak_sa 0x0001:
		คืน USB_STOR_TRANSPORT_GOOD;
	pak_sa 0x0002:
		คืน USB_STOR_TRANSPORT_FAILED;
	default:
		คืน USB_STOR_TRANSPORT_ERROR;
	}			/* switch */
	คืน USB_STOR_TRANSPORT_ERROR;
}

#define USB_TRANSPORT_UNKNOWN_RETRY 5
#define USB_TRANSPORT_NOT_READY_RETRY 10

/* clear a stall on an endpoint - special for BBB devices */
static จำนวนเต็ม usb_stor_BBB_clear_endpt_stall(sa_nracanaa us_data *us, __u8 endpt)
{
	/* ENDPOINT_HALT = 0, so set value to 0 */
	คืน usb_control_msg(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
			       USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
			       endpt, NULL, 0, USB_CNTL_TIMEOUT * 5);
}

static จำนวนเต็ม usb_stor_BBB_transport(sa_nracanaa scsi_cmd *srb, sa_nracanaa us_data *us)
{
	จำนวนเต็ม result, retry;
	จำนวนเต็ม dir_in;
	จำนวนเต็ม actlen, data_actlen;
	unsigned จำนวนเต็ม pipe, pipein, pipeout;
	ALLOC_CACHE_ALIGN_BUFFER(sa_nracanaa umass_bbb_csw, csw, 1);
#ifdef BBB_XPORT_TRACE
	unsigned var_na *ptr;
	จำนวนเต็ม index;
#endif

	dir_in = US_DIRECTION(srb->cmd[0]);

	/* COMMAND phase */
	debug("COMMAND phase\n");
	result = usb_stor_BBB_comdat(srb, us);
	ถ้า (result < 0) {
		debug("failed to send CBW status %ld\n",
		      us->pusb_dev->status);
		usb_stor_BBB_reset(us);
		คืน USB_STOR_TRANSPORT_FAILED;
	}
	ถ้า (!(us->flags & USB_READY))
		mdelay(5);
	pipein = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);
	pipeout = usb_sndbulkpipe(us->pusb_dev, us->ep_out);
	/* DATA phase + error handling */
	data_actlen = 0;
	/* no data, go immediately to the STATUS phase */
	ถ้า (srb->datalen == 0)
		goto st;
	debug("DATA phase\n");
	ถ้า (dir_in)
		pipe = pipein;
	มิฉะนั้น
		pipe = pipeout;

	result = usb_bulk_msg(us->pusb_dev, pipe, srb->pdata, srb->datalen,
			      &data_actlen, USB_CNTL_TIMEOUT * 5);
	/* special handling of STALL in DATA phase */
	ถ้า ((result < 0) && (us->pusb_dev->status & USB_ST_STALLED)) {
		debug("DATA:stall\n");
		/* clear the STALL on the endpoint */
		result = usb_stor_BBB_clear_endpt_stall(us,
					dir_in ? us->ep_in : us->ep_out);
		ถ้า (result >= 0)
			/* continue on to STATUS phase */
			goto st;
	}
	ถ้า (result < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->status);
		usb_stor_BBB_reset(us);
		คืน USB_STOR_TRANSPORT_FAILED;
	}
#ifdef BBB_XPORT_TRACE
	สำหรับ (index = 0; index < data_actlen; index++)
		printf("pdata[%d] %#x ", index, srb->pdata[index]);
	printf("\n");
#endif
	/* STATUS phase + error handling */
st:
	retry = 0;
again:
	debug("STATUS phase\n");
	result = usb_bulk_msg(us->pusb_dev, pipein, csw, UMASS_BBB_CSW_SIZE,
				&actlen, USB_CNTL_TIMEOUT*5);

	/* special handling of STALL in STATUS phase */
	ถ้า ((result < 0) && (retry < 1) &&
	    (us->pusb_dev->status & USB_ST_STALLED)) {
		debug("STATUS:stall\n");
		/* clear the STALL on the endpoint */
		result = usb_stor_BBB_clear_endpt_stall(us, us->ep_in);
		ถ้า (result >= 0 && (retry++ < 1))
			/* do a retry */
			goto again;
	}
	ถ้า (result < 0) {
		debug("usb_bulk_msg error status %ld\n",
		      us->pusb_dev->status);
		usb_stor_BBB_reset(us);
		คืน USB_STOR_TRANSPORT_FAILED;
	}
#ifdef BBB_XPORT_TRACE
	ptr = (unsigned var_na *)csw;
	สำหรับ (index = 0; index < UMASS_BBB_CSW_SIZE; index++)
		printf("ptr[%d] %#x ", index, ptr[index]);
	printf("\n");
#endif
	/* misuse pipe to get the residue */
	pipe = le32_to_cpu(csw->dCSWDataResidue);
	ถ้า (pipe == 0 && srb->datalen != 0 && srb->datalen - data_actlen != 0)
		pipe = srb->datalen - data_actlen;
	ถ้า (CSWSIGNATURE != le32_to_cpu(csw->dCSWSignature)) {
		debug("!CSWSIGNATURE\n");
		usb_stor_BBB_reset(us);
		คืน USB_STOR_TRANSPORT_FAILED;
	} มิฉะนั้น ถ้า ((CBWTag - 1) != le32_to_cpu(csw->dCSWTag)) {
		debug("!Tag\n");
		usb_stor_BBB_reset(us);
		คืน USB_STOR_TRANSPORT_FAILED;
	} มิฉะนั้น ถ้า (csw->bCSWStatus > CSWSTATUS_PHASE) {
		debug(">PHASE\n");
		usb_stor_BBB_reset(us);
		คืน USB_STOR_TRANSPORT_FAILED;
	} มิฉะนั้น ถ้า (csw->bCSWStatus == CSWSTATUS_PHASE) {
		debug("=PHASE\n");
		usb_stor_BBB_reset(us);
		คืน USB_STOR_TRANSPORT_FAILED;
	} มิฉะนั้น ถ้า (data_actlen > srb->datalen) {
		debug("transferred %dB instead of %ldB\n",
		      data_actlen, srb->datalen);
		คืน USB_STOR_TRANSPORT_FAILED;
	} มิฉะนั้น ถ้า (csw->bCSWStatus == CSWSTATUS_FAILED) {
		debug("FAILED\n");
		คืน USB_STOR_TRANSPORT_FAILED;
	}

	คืน result;
}

static จำนวนเต็ม usb_stor_CB_transport(sa_nracanaa scsi_cmd *srb, sa_nracanaa us_data *us)
{
	จำนวนเต็ม result, status;
	sa_nracanaa scsi_cmd *psrb;
	sa_nracanaa scsi_cmd reqsrb;
	จำนวนเต็ม retry, notready;

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
	ถ้า (us->protocol == US_PR_CBI) {
		status = usb_stor_CBI_get_status(srb, us);
		/* if the status is error, report it */
		ถ้า (status == USB_STOR_TRANSPORT_ERROR) {
			debug(" USB CBI Command Error\n");
			คืน status;
		}
		srb->sense_buf[12] = (unsigned var_na)(us->ip_data >> 8);
		srb->sense_buf[13] = (unsigned var_na)(us->ip_data & 0xff);
		ถ้า (!us->ip_data) {
			/* if the status is good, report it */
			ถ้า (status == USB_STOR_TRANSPORT_GOOD) {
				debug(" USB CBI Command Good\n");
				คืน status;
			}
		}
	}
	/* do we have to issue an auto request? */
	/* HERE we have to check the result */
	ถ้า ((result < 0) && !(us->pusb_dev->status & USB_ST_STALLED)) {
		debug("ERROR %lX\n", us->pusb_dev->status);
		us->transport_reset(us);
		คืน USB_STOR_TRANSPORT_ERROR;
	}
	ถ้า ((us->protocol == US_PR_CBI) &&
	    ((srb->cmd[0] == SCSI_REQ_SENSE) ||
	    (srb->cmd[0] == SCSI_INQUIRY))) {
		/* do not issue an autorequest after request sense */
		debug("No auto request and good\n");
		คืน USB_STOR_TRANSPORT_GOOD;
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
	ถ้า (us->protocol == US_PR_CBI)
		status = usb_stor_CBI_get_status(psrb, us);

	ถ้า ((result < 0) && !(us->pusb_dev->status & USB_ST_STALLED)) {
		debug(" AUTO REQUEST ERROR %ld\n",
		      us->pusb_dev->status);
		คืน USB_STOR_TRANSPORT_ERROR;
	}
	debug("autorequest returned 0x%02X 0x%02X 0x%02X 0x%02X\n",
	      srb->sense_buf[0], srb->sense_buf[2],
	      srb->sense_buf[12], srb->sense_buf[13]);
	/* Check the auto request result */
	ถ้า ((srb->sense_buf[2] == 0) &&
	    (srb->sense_buf[12] == 0) &&
	    (srb->sense_buf[13] == 0)) {
		/* ok, no sense */
		คืน USB_STOR_TRANSPORT_GOOD;
	}

	/* Check the auto request result */
	switch (srb->sense_buf[2]) {
	pak_sa 0x01:
		/* Recovered Error */
		คืน USB_STOR_TRANSPORT_GOOD;
		bha_nga;
	pak_sa 0x02:
		/* Not Ready */
		ถ้า (notready++ > USB_TRANSPORT_NOT_READY_RETRY) {
			printf("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X (NOT READY)\n", srb->cmd[0],
				srb->sense_buf[0], srb->sense_buf[2],
				srb->sense_buf[12], srb->sense_buf[13]);
			คืน USB_STOR_TRANSPORT_FAILED;
		} มิฉะนั้น {
			mdelay(100);
			goto do_retry;
		}
		bha_nga;
	default:
		ถ้า (retry++ > USB_TRANSPORT_UNKNOWN_RETRY) {
			printf("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
			       " 0x%02X\n", srb->cmd[0], srb->sense_buf[0],
				srb->sense_buf[2], srb->sense_buf[12],
				srb->sense_buf[13]);
			คืน USB_STOR_TRANSPORT_FAILED;
		} มิฉะนั้น
			goto do_retry;
		bha_nga;
	}
	คืน USB_STOR_TRANSPORT_FAILED;
}

static void usb_stor_set_max_xfer_blk(sa_nracanaa usb_device *udev,
				      sa_nracanaa us_data *us)
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

#ถ้า CONFIG_IS_ENABLED(DM_USB)
	size_t size;
	จำนวนเต็ม ret;

	ret = usb_get_max_xfer_size(udev, (size_t *)&size);
	ถ้า ((ret >= 0) && (size < blk * 512))
		blk = size / 512;
#endif

	us->max_xfer_blk = blk;
}

static จำนวนเต็ม usb_inquiry(sa_nracanaa scsi_cmd *srb, sa_nracanaa us_data *ss)
{
	จำนวนเต็ม retry, i;
	retry = 5;
	do {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_INQUIRY;
		srb->cmd[1] = srb->lun << 5;
		srb->cmd[4] = 36;
		srb->datalen = 36;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		i = ss->transport(srb, ss);
		debug("inquiry returns %d\n", i);
		ถ้า (i == 0)
			bha_nga;
	} ขณะ (--retry);

	ถ้า (!retry) {
		printf("error in inquiry\n");
		คืน -1;
	}
	คืน 0;
}

static จำนวนเต็ม usb_request_sense(sa_nracanaa scsi_cmd *srb, sa_nracanaa us_data *ss)
{
	var_na *ptr;

	ptr = (var_na *)srb->pdata;
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
	คืน 0;
}

static จำนวนเต็ม usb_test_unit_ready(sa_nracanaa scsi_cmd *srb, sa_nracanaa us_data *ss)
{
	จำนวนเต็ม retries = 10;

	do {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_TST_U_RDY;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 0;
		srb->cmdlen = ss->cmd12 ? 12 : 6;
		ถ้า (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD) {
			ss->flags |= USB_READY;
			คืน 0;
		}
		usb_request_sense(srb, ss);
		/*
		 * Check the Key Code Qualifier, if it matches
		 * "Not Ready - medium not present"
		 * (the sense Key equals 0x2 and the ASC is 0x3a)
		 * return immediately as the medium being absent won't change
		 * unless there is a user action.
		 */
		ถ้า ((srb->sense_buf[2] == 0x02) &&
		    (srb->sense_buf[12] == 0x3a))
			คืน -1;
		mdelay(100);
	} ขณะ (retries--);

	คืน -1;
}

static จำนวนเต็ม usb_read_capacity(sa_nracanaa scsi_cmd *srb, sa_nracanaa us_data *ss)
{
	จำนวนเต็ม retry;
	/* XXX retries */
	retry = 3;
	do {
		memset(&srb->cmd[0], 0, 12);
		srb->cmd[0] = SCSI_RD_CAPAC;
		srb->cmd[1] = srb->lun << 5;
		srb->datalen = 8;
		srb->cmdlen = ss->cmd12 ? 12 : 10;
		ถ้า (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD)
			คืน 0;
	} ขณะ (retry--);

	คืน -1;
}

static จำนวนเต็ม usb_read_10(sa_nracanaa scsi_cmd *srb, sa_nracanaa us_data *ss,
		       unsigned long start, unsigned short blocks)
{
	memset(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_READ10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((unsigned var_na) (start >> 24)) & 0xff;
	srb->cmd[3] = ((unsigned var_na) (start >> 16)) & 0xff;
	srb->cmd[4] = ((unsigned var_na) (start >> 8)) & 0xff;
	srb->cmd[5] = ((unsigned var_na) (start)) & 0xff;
	srb->cmd[7] = ((unsigned var_na) (blocks >> 8)) & 0xff;
	srb->cmd[8] = (unsigned var_na) blocks & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("read10: start %lx blocks %x\n", start, blocks);
	คืน ss->transport(srb, ss);
}

static จำนวนเต็ม usb_write_10(sa_nracanaa scsi_cmd *srb, sa_nracanaa us_data *ss,
			unsigned long start, unsigned short blocks)
{
	memset(&srb->cmd[0], 0, 12);
	srb->cmd[0] = SCSI_WRITE10;
	srb->cmd[1] = srb->lun << 5;
	srb->cmd[2] = ((unsigned var_na) (start >> 24)) & 0xff;
	srb->cmd[3] = ((unsigned var_na) (start >> 16)) & 0xff;
	srb->cmd[4] = ((unsigned var_na) (start >> 8)) & 0xff;
	srb->cmd[5] = ((unsigned var_na) (start)) & 0xff;
	srb->cmd[7] = ((unsigned var_na) (blocks >> 8)) & 0xff;
	srb->cmd[8] = (unsigned var_na) blocks & 0xff;
	srb->cmdlen = ss->cmd12 ? 12 : 10;
	debug("write10: start %lx blocks %x\n", start, blocks);
	คืน ss->transport(srb, ss);
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
static void usb_bin_fixup(sa_nracanaa usb_device_descriptor descriptor,
				unsigned var_na vendor[],
				unsigned var_na product[]) {
	const unsigned var_na max_vendor_len = 40;
	const unsigned var_na max_product_len = 20;
	ถ้า (descriptor.idVendor == 0x0424 && descriptor.idProduct == 0x223a) {
		strncpy((var_na *)vendor, "SMSC", max_vendor_len);
		strncpy((var_na *)product, "Flash Media Cntrller",
			max_product_len);
	}
}
#endif /* CONFIG_USB_BIN_FIXUP */

#ถ้า CONFIG_IS_ENABLED(BLK)
static unsigned long usb_stor_read(sa_nracanaa udevice *dev, lbaint_t blknr,
				   lbaint_t blkcnt, void *buffer)
#มิฉะนั้น
static unsigned long usb_stor_read(sa_nracanaa blk_desc *block_dev, lbaint_t blknr,
				   lbaint_t blkcnt, void *buffer)
#endif
{
	lbaint_t start, blks;
	uintptr_t buf_addr;
	unsigned short smallblks;
	sa_nracanaa usb_device *udev;
	sa_nracanaa us_data *ss;
	จำนวนเต็ม retry;
	sa_nracanaa scsi_cmd *srb = &usb_ccb;
#ถ้า CONFIG_IS_ENABLED(BLK)
	sa_nracanaa blk_desc *block_dev;
#endif

	ถ้า (blkcnt == 0)
		คืน 0;
	/* Setup  device */
#ถ้า CONFIG_IS_ENABLED(BLK)
	block_dev = dev_get_uclass_plat(dev);
	udev = dev_get_parent_priv(dev_get_parent(dev));
	debug("\nusb_read: udev %d\n", block_dev->devnum);
#มิฉะนั้น
	debug("\nusb_read: udev %d\n", block_dev->devnum);
	udev = usb_dev_desc[block_dev->devnum].priv;
	ถ้า (!udev) {
		debug("%s: No device\n", __func__);
		คืน 0;
	}
#endif
	ss = (sa_nracanaa us_data *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_lock_async(udev, 1);
	srb->lun = block_dev->lun;
	buf_addr = (uintptr_t)buffer;
	start = blknr;
	blks = blkcnt;

	debug("\nusb_read: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      block_dev->devnum, start, blks, buf_addr);

	do {
		/* XXX need some comment here */
		retry = 2;
		srb->pdata = (unsigned var_na *)buf_addr;
		ถ้า (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		มิฉะนั้น
			smallblks = (unsigned short) blks;
retry_it:
		ถ้า (smallblks == ss->max_xfer_blk)
			usb_show_progress();
		srb->datalen = block_dev->blksz * smallblks;
		srb->pdata = (unsigned var_na *)buf_addr;
		ถ้า (usb_read_10(srb, ss, start, smallblks)) {
			debug("Read ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			ถ้า (retry--)
				goto retry_it;
			blkcnt -= blks;
			bha_nga;
		}
		start += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} ขณะ (blks != 0);

	debug("usb_read: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      start, smallblks, buf_addr);

	usb_lock_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	ถ้า (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	คืน blkcnt;
}

#ถ้า CONFIG_IS_ENABLED(BLK)
static unsigned long usb_stor_write(sa_nracanaa udevice *dev, lbaint_t blknr,
				    lbaint_t blkcnt, const void *buffer)
#มิฉะนั้น
static unsigned long usb_stor_write(sa_nracanaa blk_desc *block_dev, lbaint_t blknr,
				    lbaint_t blkcnt, const void *buffer)
#endif
{
	lbaint_t start, blks;
	uintptr_t buf_addr;
	unsigned short smallblks;
	sa_nracanaa usb_device *udev;
	sa_nracanaa us_data *ss;
	จำนวนเต็ม retry;
	sa_nracanaa scsi_cmd *srb = &usb_ccb;
#ถ้า CONFIG_IS_ENABLED(BLK)
	sa_nracanaa blk_desc *block_dev;
#endif

	ถ้า (blkcnt == 0)
		คืน 0;

	/* Setup  device */
#ถ้า CONFIG_IS_ENABLED(BLK)
	block_dev = dev_get_uclass_plat(dev);
	udev = dev_get_parent_priv(dev_get_parent(dev));
	debug("\nusb_read: udev %d\n", block_dev->devnum);
#มิฉะนั้น
	debug("\nusb_read: udev %d\n", block_dev->devnum);
	udev = usb_dev_desc[block_dev->devnum].priv;
	ถ้า (!udev) {
		debug("%s: No device\n", __func__);
		คืน 0;
	}
#endif
	ss = (sa_nracanaa us_data *)udev->privptr;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	usb_lock_async(udev, 1);

	srb->lun = block_dev->lun;
	buf_addr = (uintptr_t)buffer;
	start = blknr;
	blks = blkcnt;

	debug("\nusb_write: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      block_dev->devnum, start, blks, buf_addr);

	do {
		/* If write fails retry for max retry count else
		 * return with number of blocks written successfully.
		 */
		retry = 2;
		srb->pdata = (unsigned var_na *)buf_addr;
		ถ้า (blks > ss->max_xfer_blk)
			smallblks = ss->max_xfer_blk;
		มิฉะนั้น
			smallblks = (unsigned short) blks;
retry_it:
		ถ้า (smallblks == ss->max_xfer_blk)
			usb_show_progress();
		srb->datalen = block_dev->blksz * smallblks;
		srb->pdata = (unsigned var_na *)buf_addr;
		ถ้า (usb_write_10(srb, ss, start, smallblks)) {
			debug("Write ERROR\n");
			ss->flags &= ~USB_READY;
			usb_request_sense(srb, ss);
			ถ้า (retry--)
				goto retry_it;
			blkcnt -= blks;
			bha_nga;
		}
		start += smallblks;
		blks -= smallblks;
		buf_addr += srb->datalen;
	} ขณะ (blks != 0);

	debug("usb_write: end startblk " LBAF ", blccnt %x buffer %lx\n",
	      start, smallblks, buf_addr);

	usb_lock_async(udev, 0);
	usb_disable_asynch(0); /* asynch transfer allowed */
	ถ้า (blkcnt >= ss->max_xfer_blk)
		debug("\n");
	คืน blkcnt;

}

/* Probe to see if a new device is actually a Storage device */
จำนวนเต็ม usb_storage_probe(sa_nracanaa usb_device *dev, unsigned จำนวนเต็ม ifnum,
		      sa_nracanaa us_data *ss)
{
	sa_nracanaa usb_interface *iface;
	จำนวนเต็ม i;
	sa_nracanaa usb_endpoint_descriptor *ep_desc;
	unsigned จำนวนเต็ม flags = 0;

	/* let's examine the device now */
	iface = &dev->config.if_desc[ifnum];

	ถ้า (dev->descriptor.bDeviceClass != 0 ||
			iface->desc.bInterfaceClass != USB_CLASS_MASS_STORAGE ||
			iface->desc.bInterfaceSubClass < US_SC_MIN ||
			iface->desc.bInterfaceSubClass > US_SC_MAX) {
		debug("Not mass storage\n");
		/* if it's not a mass storage, we go no further */
		คืน 0;
	}

	memset(ss, 0, parimaa_na(sa_nracanaa us_data));

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
	switch (ss->protocol) {
	pak_sa US_PR_CB:
		debug("Control/Bulk\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		bha_nga;

	pak_sa US_PR_CBI:
		debug("Control/Bulk/Interrupt\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		bha_nga;
	pak_sa US_PR_BULK:
		debug("Bulk/Bulk/Bulk\n");
		ss->transport = usb_stor_BBB_transport;
		ss->transport_reset = usb_stor_BBB_reset;
		bha_nga;
	default:
		printf("USB Storage Transport unknown / not yet implemented\n");
		คืน 0;
		bha_nga;
	}

	/*
	 * We are expecting a minimum of 2 endpoints - in and out (bulk).
	 * An optional interrupt is OK (necessary for CBI protocol).
	 * We will ignore any others.
	 */
	สำหรับ (i = 0; i < iface->desc.bNumEndpoints; i++) {
		ep_desc = &iface->ep_desc[i];
		/* is it an BULK endpoint? */
		ถ้า ((ep_desc->bmAttributes &
		     USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK) {
			ถ้า (ep_desc->bEndpointAddress & USB_DIR_IN)
				ss->ep_in = ep_desc->bEndpointAddress &
						USB_ENDPOINT_NUMBER_MASK;
			มิฉะนั้น
				ss->ep_out =
					ep_desc->bEndpointAddress &
					USB_ENDPOINT_NUMBER_MASK;
		}

		/* is it an interrupt endpoint? */
		ถ้า ((ep_desc->bmAttributes &
		     USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT) {
			ss->ep_int = ep_desc->bEndpointAddress &
						USB_ENDPOINT_NUMBER_MASK;
			ss->irqinterval = ep_desc->bInterval;
		}
	}
	debug("Endpoints In %d Out %d Int %d\n",
	      ss->ep_in, ss->ep_out, ss->ep_int);

	/* Do some basic sanity checks, and bail if we find a problem */
	ถ้า (usb_set_interface(dev, iface->desc.bInterfaceNumber, 0) ||
	    !ss->ep_in || !ss->ep_out ||
	    (ss->protocol == US_PR_CBI && ss->ep_int == 0)) {
		debug("Problems with device\n");
		คืน 0;
	}
	/* set class specific stuff */
	/* We only handle certain protocols.  Currently, these are
	 * the only ones.
	 * The SFF8070 accepts the requests used in u-boot
	 */
	ถ้า (ss->subclass != US_SC_UFI && ss->subclass != US_SC_SCSI &&
	    ss->subclass != US_SC_8070) {
		printf("Sorry, protocol %d not yet supported.\n", ss->subclass);
		คืน 0;
	}

	/* UFI uses 12-byte commands (like RBC, unlike SCSI) */
	ถ้า (ss->subclass == US_SC_UFI)
		ss->cmd12 = true;

	ถ้า (ss->ep_int) {
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

	dev->privptr = (void *)ss;
	คืน 1;
}

จำนวนเต็ม usb_stor_get_info(sa_nracanaa usb_device *dev, sa_nracanaa us_data *ss,
		      sa_nracanaa blk_desc *dev_desc)
{
	unsigned var_na perq, modi;
	ALLOC_CACHE_ALIGN_BUFFER(u32, cap, 2);
	ALLOC_CACHE_ALIGN_BUFFER(u8, usb_stor_buf, 36);
	u32 capacity, blksz;
	sa_nracanaa scsi_cmd *pccb = &usb_ccb;

	pccb->pdata = usb_stor_buf;

	dev_desc->target = dev->devnum;
	pccb->lun = dev_desc->lun;
	debug(" address %d\n", dev_desc->target);

	ถ้า (usb_inquiry(pccb, ss)) {
		debug("%s: usb_inquiry() failed\n", __func__);
		คืน -1;
	}

	perq = usb_stor_buf[0];
	modi = usb_stor_buf[1];

	/*
	 * Skip unknown devices (0x1f) and enclosure service devices (0x0d),
	 * they would not respond to test_unit_ready .
	 */
	ถ้า (((perq & 0x1f) == 0x1f) || ((perq & 0x1f) == 0x0d)) {
		debug("%s: unknown/unsupported device\n", __func__);
		คืน 0;
	}
	ถ้า ((modi&0x80) == 0x80) {
		/* drive is removable */
		dev_desc->removable = 1;
	}
	memcpy(dev_desc->vendor, (const void *)&usb_stor_buf[8], 8);
	memcpy(dev_desc->product, (const void *)&usb_stor_buf[16], 16);
	memcpy(dev_desc->revision, (const void *)&usb_stor_buf[32], 4);
	dev_desc->vendor[8] = 0;
	dev_desc->product[16] = 0;
	dev_desc->revision[4] = 0;
#ifdef CONFIG_USB_BIN_FIXUP
	usb_bin_fixup(dev->descriptor, (uchar *)dev_desc->vendor,
		      (uchar *)dev_desc->product);
#endif /* CONFIG_USB_BIN_FIXUP */
	debug("ISO Vers %X, Response Data %X\n", usb_stor_buf[2],
	      usb_stor_buf[3]);
	ถ้า (usb_test_unit_ready(pccb, ss)) {
		printf("Device NOT ready\n"
		       "   Request Sense returned %02X %02X %02X\n",
		       pccb->sense_buf[2], pccb->sense_buf[12],
		       pccb->sense_buf[13]);
		ถ้า (dev_desc->removable == 1)
			dev_desc->type = perq;
		คืน 0;
	}
	pccb->pdata = (unsigned var_na *)cap;
	memset(pccb->pdata, 0, 8);
	ถ้า (usb_read_capacity(pccb, ss) != 0) {
		printf("READ_CAP ERROR\n");
		ss->flags &= ~USB_READY;
		cap[0] = 2880;
		cap[1] = 0x200;
	}
	debug("Read Capacity returns: 0x%08x, 0x%08x\n", cap[0], cap[1]);
#ถ้า 0
	ถ้า (cap[0] > (0x200000 * 10)) /* greater than 10 GByte */
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

	คืน 1;
}

#ถ้า CONFIG_IS_ENABLED(DM_USB)

static จำนวนเต็ม usb_mass_storage_probe(sa_nracanaa udevice *dev)
{
	sa_nracanaa usb_device *udev = dev_get_parent_priv(dev);
	จำนวนเต็ม ret;

	usb_disable_asynch(1); /* asynch transfer not allowed */
	ret = usb_stor_probe_device(udev);
	usb_disable_asynch(0); /* asynch transfer allowed */

	คืน ret;
}

static const sa_nracanaa udevice_id usb_mass_storage_ids[] = {
	{ .compatible = "usb-mass-storage" },
	{ }
};

U_BOOT_DRIVER(usb_mass_storage) = {
	.name	= "usb_mass_storage",
	.id	= UCLASS_MASS_STORAGE,
	.of_match = usb_mass_storage_ids,
	.probe = usb_mass_storage_probe,
#ถ้า CONFIG_IS_ENABLED(BLK)
	.plat_auto	= parimaa_na(sa_nracanaa us_data),
#endif
};

UCLASS_DRIVER(usb_mass_storage) = {
	.id		= UCLASS_MASS_STORAGE,
	.name		= "usb_mass_storage",
};

static const sa_nracanaa usb_device_id mass_storage_id_table[] = {
	{
		.match_flags = USB_DEVICE_ID_MATCH_INT_CLASS,
		.bInterfaceClass = USB_CLASS_MASS_STORAGE
	},
	{ }		/* Terminating entry */
};

U_BOOT_USB_DEVICE(usb_mass_storage, mass_storage_id_table);
#endif

#ถ้า CONFIG_IS_ENABLED(BLK)
static const sa_nracanaa blk_ops usb_storage_ops = {
	.read	= usb_stor_read,
	.write	= usb_stor_write,
};

U_BOOT_DRIVER(usb_storage_blk) = {
	.name		= "usb_storage_blk",
	.id		= UCLASS_BLK,
	.ops		= &usb_storage_ops,
};
#มิฉะนั้น
U_BOOT_LEGACY_BLK(usb) = {
	.uclass_idname	= "usb",
	.uclass_id	= UCLASS_USB,
	.max_devs	= USB_MAX_STOR_DEV,
	.desc		= usb_dev_desc,
};
#endif
