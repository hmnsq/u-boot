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
#અન્તર્ભાવ <gzip.h>
#અન્તર્ભાવ <image.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <xyzModem.h>
#અન્તર્ભાવ <linux/libfdt.h>

#define BUF_SIZE 1024

/*
 * Information required to load image using ymodem.
 *
 * @image_read: Now of bytes read from the image.
 * @buf: pointer to the previous read block.
 */
સંરચના ymodem_fit_info {
	પૂર્ણાંક image_read;
	અક્ષર *buf;
};

સ્થિર પૂર્ણાંક getcymodem(શૂન્ય) {
	જો (tstc())
		પાછા (getchar());
	પાછા -1;
}

સ્થિર ulong ymodem_read_fit(સંરચના spl_load_info *load, ulong offset,
			     ulong size, શૂન્ય *addr)
{
	પૂર્ણાંક res, err, buf_offset;
	સંરચના ymodem_fit_info *info = load->priv;
	અક્ષર *buf = info->buf;
	ulong copy_size = size;

	જ્યારે (info->image_read < offset) {
		res = xyzModem_stream_read(buf, BUF_SIZE, &err);
		જો (res <= 0)
			તોડો;

		info->image_read += res;
	}

	જો (info->image_read > offset) {
		res = info->image_read - offset;
		જો (info->image_read % BUF_SIZE)
			buf_offset = (info->image_read % BUF_SIZE);
		નહીંતર
			buf_offset = BUF_SIZE;

		જો (res > copy_size) {
			memcpy(addr, &buf[buf_offset - res], copy_size);
			જાઓ done;
		}
		memcpy(addr, &buf[buf_offset - res], res);
		addr = addr + res;
		copy_size -= res;
	}

	જ્યારે (info->image_read < offset + size) {
		res = xyzModem_stream_read(buf, BUF_SIZE, &err);
		જો (res <= 0)
			તોડો;

		info->image_read += res;
		જો (res > copy_size) {
			memcpy(addr, buf, copy_size);
			જાઓ done;
		}
		memcpy(addr, buf, res);
		addr += res;
		copy_size -= res;
	}

done:
	પાછા size;
}

પૂર્ણાંક spl_ymodem_load_image(સંરચના spl_image_info *spl_image,
			  સંરચના spl_boot_device *bootdev)
{
	ulong size = 0;
	પૂર્ણાંક err;
	પૂર્ણાંક res;
	પૂર્ણાંક ret;
	connection_info_t info;
	અક્ષર buf[BUF_SIZE];
	સંરચના legacy_img_hdr *ih = ખાલી;
	ulong addr = 0;

	info.mode = xyzModem_ymodem;
	ret = xyzModem_stream_open(&info, &err);
	જો (ret) {
		printf("spl: ymodem err - %s\n", xyzModem_error(err));
		પાછા ret;
	}

	res = xyzModem_stream_read(buf, BUF_SIZE, &err);
	જો (res <= 0)
		જાઓ end_stream;

	જો (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL) &&
	    image_get_magic((સંરચના legacy_img_hdr *)buf) == FDT_MAGIC) {
		addr = CONFIG_SYS_LOAD_ADDR;
		ih = (સંરચના legacy_img_hdr *)addr;

		memcpy((શૂન્ય *)addr, buf, res);
		size += res;
		addr += res;

		જ્યારે ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
			memcpy((શૂન્ય *)addr, buf, res);
			size += res;
			addr += res;
		}

		ret = spl_parse_image_header(spl_image, bootdev, ih);
		જો (ret)
			પાછા ret;
	} નહીંતર જો (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	    image_get_magic((સંરચના legacy_img_hdr *)buf) == FDT_MAGIC) {
		સંરચના spl_load_info load;
		સંરચના ymodem_fit_info info;

		debug("Found FIT\n");
		spl_load_init(&load, ymodem_read_fit, (શૂન્ય *)&info, 1);
		info.buf = buf;
		info.image_read = BUF_SIZE;
		ret = spl_load_simple_fit(spl_image, &load, 0, (શૂન્ય *)buf);
		size = info.image_read;

		જ્યારે ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0)
			size += res;
	} નહીંતર {
		ih = (સંરચના legacy_img_hdr *)buf;
		ret = spl_parse_image_header(spl_image, bootdev, ih);
		જો (ret)
			જાઓ end_stream;
#ifdef CONFIG_SPL_GZIP
		જો (ih->ih_comp == IH_COMP_GZIP)
			addr = CONFIG_SYS_LOAD_ADDR;
		નહીંતર
#endif
			addr = spl_image->load_addr;
		memcpy((શૂન્ય *)addr, buf, res);
		ih = (સંરચના legacy_img_hdr *)addr;
		size += res;
		addr += res;

		જ્યારે ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
			memcpy((શૂન્ય *)addr, buf, res);
			size += res;
			addr += res;
		}
	}

end_stream:
	xyzModem_stream_close(&err);
	xyzModem_stream_terminate(false, &getcymodem);

	printf("Loaded %lu bytes\n", size);

#ifdef CONFIG_SPL_GZIP
	જો (!(IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	      image_get_magic((સંરચના legacy_img_hdr *)buf) == FDT_MAGIC) &&
	    (ih->ih_comp == IH_COMP_GZIP)) {
		જો (gunzip((શૂન્ય *)(spl_image->load_addr + કદ(*ih)),
			   CONFIG_SYS_BOOTM_LEN,
			   (શૂન્ય *)(CONFIG_SYS_LOAD_ADDR + કદ(*ih)),
			   &size)) {
			puts("Uncompressing error\n");
			પાછા -EIO;
		}
	}
#endif

	પાછા ret;
}
SPL_LOAD_IMAGE_METHOD("UART", 0, BOOT_DEVICE_UART, spl_ymodem_load_image);
