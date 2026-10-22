// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2011
 * Texas Instruments, <www.ti.com>
 *
 * Matt Porter <mporter@ti.com>
 */
#ଅନ୍ତର୍ଭାଵ <gzip.h>
#ଅନ୍ତର୍ଭାଵ <image.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <xyzModem.h>
#ଅନ୍ତର୍ଭାଵ <linux/libfdt.h>

#define BUF_SIZE 1024

/*
 * Information required to load image using ymodem.
 *
 * @image_read: Now of bytes read from the image.
 * @buf: pointer to the previous read block.
 */
ସଂରଚନା ymodem_fit_info {
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା image_read;
	ଅକ୍ଷର *buf;
};

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା getcymodem(ଶୂନ୍ୟ) {
	ଯଦି (tstc())
		ଫେରନ୍ତୁ (getchar());
	ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ulong ymodem_read_fit(ସଂରଚନା spl_load_info *load, ulong offset,
			     ulong size, ଶୂନ୍ୟ *addr)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା res, err, buf_offset;
	ସଂରଚନା ymodem_fit_info *info = load->priv;
	ଅକ୍ଷର *buf = info->buf;
	ulong copy_size = size;

	ଯେତେବେଳେ (info->image_read < offset) {
		res = xyzModem_stream_read(buf, BUF_SIZE, &err);
		ଯଦି (res <= 0)
			ଭାଙ୍ଗ;

		info->image_read += res;
	}

	ଯଦି (info->image_read > offset) {
		res = info->image_read - offset;
		ଯଦି (info->image_read % BUF_SIZE)
			buf_offset = (info->image_read % BUF_SIZE);
		ଅନ୍ୟଥା
			buf_offset = BUF_SIZE;

		ଯଦି (res > copy_size) {
			memcpy(addr, &buf[buf_offset - res], copy_size);
			ଯାଅ done;
		}
		memcpy(addr, &buf[buf_offset - res], res);
		addr = addr + res;
		copy_size -= res;
	}

	ଯେତେବେଳେ (info->image_read < offset + size) {
		res = xyzModem_stream_read(buf, BUF_SIZE, &err);
		ଯଦି (res <= 0)
			ଭାଙ୍ଗ;

		info->image_read += res;
		ଯଦି (res > copy_size) {
			memcpy(addr, buf, copy_size);
			ଯାଅ done;
		}
		memcpy(addr, buf, res);
		addr += res;
		copy_size -= res;
	}

done:
	ଫେରନ୍ତୁ size;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_ymodem_load_image(ସଂରଚନା spl_image_info *spl_image,
			  ସଂରଚନା spl_boot_device *bootdev)
{
	ulong size = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
	connection_info_t info;
	ଅକ୍ଷର buf[BUF_SIZE];
	ସଂରଚନା legacy_img_hdr *ih = ଖାଲି;
	ulong addr = 0;

	info.mode = xyzModem_ymodem;
	ret = xyzModem_stream_open(&info, &err);
	ଯଦି (ret) {
		printf("spl: ymodem err - %s\n", xyzModem_error(err));
		ଫେରନ୍ତୁ ret;
	}

	res = xyzModem_stream_read(buf, BUF_SIZE, &err);
	ଯଦି (res <= 0)
		ଯାଅ end_stream;

	ଯଦି (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL) &&
	    image_get_magic((ସଂରଚନା legacy_img_hdr *)buf) == FDT_MAGIC) {
		addr = CONFIG_SYS_LOAD_ADDR;
		ih = (ସଂରଚନା legacy_img_hdr *)addr;

		memcpy((ଶୂନ୍ୟ *)addr, buf, res);
		size += res;
		addr += res;

		ଯେତେବେଳେ ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
			memcpy((ଶୂନ୍ୟ *)addr, buf, res);
			size += res;
			addr += res;
		}

		ret = spl_parse_image_header(spl_image, bootdev, ih);
		ଯଦି (ret)
			ଫେରନ୍ତୁ ret;
	} ଅନ୍ୟଥା ଯଦି (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	    image_get_magic((ସଂରଚନା legacy_img_hdr *)buf) == FDT_MAGIC) {
		ସଂରଚନା spl_load_info load;
		ସଂରଚନା ymodem_fit_info info;

		debug("Found FIT\n");
		spl_load_init(&load, ymodem_read_fit, (ଶୂନ୍ୟ *)&info, 1);
		info.buf = buf;
		info.image_read = BUF_SIZE;
		ret = spl_load_simple_fit(spl_image, &load, 0, (ଶୂନ୍ୟ *)buf);
		size = info.image_read;

		ଯେତେବେଳେ ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0)
			size += res;
	} ଅନ୍ୟଥା {
		ih = (ସଂରଚନା legacy_img_hdr *)buf;
		ret = spl_parse_image_header(spl_image, bootdev, ih);
		ଯଦି (ret)
			ଯାଅ end_stream;
#ifdef CONFIG_SPL_GZIP
		ଯଦି (ih->ih_comp == IH_COMP_GZIP)
			addr = CONFIG_SYS_LOAD_ADDR;
		ଅନ୍ୟଥା
#endif
			addr = spl_image->load_addr;
		memcpy((ଶୂନ୍ୟ *)addr, buf, res);
		ih = (ସଂରଚନା legacy_img_hdr *)addr;
		size += res;
		addr += res;

		ଯେତେବେଳେ ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
			memcpy((ଶୂନ୍ୟ *)addr, buf, res);
			size += res;
			addr += res;
		}
	}

end_stream:
	xyzModem_stream_close(&err);
	xyzModem_stream_terminate(false, &getcymodem);

	printf("Loaded %lu bytes\n", size);

#ifdef CONFIG_SPL_GZIP
	ଯଦି (!(IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	      image_get_magic((ସଂରଚନା legacy_img_hdr *)buf) == FDT_MAGIC) &&
	    (ih->ih_comp == IH_COMP_GZIP)) {
		ଯଦି (gunzip((ଶୂନ୍ୟ *)(spl_image->load_addr + ଆକାର(*ih)),
			   CONFIG_SYS_BOOTM_LEN,
			   (ଶୂନ୍ୟ *)(CONFIG_SYS_LOAD_ADDR + ଆକାର(*ih)),
			   &size)) {
			puts("Uncompressing error\n");
			ଫେରନ୍ତୁ -EIO;
		}
	}
#endif

	ଫେରନ୍ତୁ ret;
}
SPL_LOAD_IMAGE_METHOD("UART", 0, BOOT_DEVICE_UART, spl_ymodem_load_image);
