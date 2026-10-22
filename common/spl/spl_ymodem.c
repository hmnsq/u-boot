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
#அந்தர்பாவ <gzip.h>
#அந்தர்பாவ <image.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <xyzModem.h>
#அந்தர்பாவ <linux/libfdt.h>

#define BUF_SIZE 1024

/*
 * Information required to load image using ymodem.
 *
 * @image_read: Now of bytes read from the image.
 * @buf: pointer to the previous read block.
 */
கட்டமைப்பு ymodem_fit_info {
	முழு image_read;
	எழுத்து *buf;
};

நிலையான முழு getcymodem(வெறுமை) {
	எனில் (tstc())
		திரும்பு (getchar());
	திரும்பு -1;
}

நிலையான ulong ymodem_read_fit(கட்டமைப்பு spl_load_info *load, ulong offset,
			     ulong size, வெறுமை *addr)
{
	முழு res, err, buf_offset;
	கட்டமைப்பு ymodem_fit_info *info = load->priv;
	எழுத்து *buf = info->buf;
	ulong copy_size = size;

	போது (info->image_read < offset) {
		res = xyzModem_stream_read(buf, BUF_SIZE, &err);
		எனில் (res <= 0)
			நிறுத்து;

		info->image_read += res;
	}

	எனில் (info->image_read > offset) {
		res = info->image_read - offset;
		எனில் (info->image_read % BUF_SIZE)
			buf_offset = (info->image_read % BUF_SIZE);
		இல்லையெனில்
			buf_offset = BUF_SIZE;

		எனில் (res > copy_size) {
			memcpy(addr, &buf[buf_offset - res], copy_size);
			செல் done;
		}
		memcpy(addr, &buf[buf_offset - res], res);
		addr = addr + res;
		copy_size -= res;
	}

	போது (info->image_read < offset + size) {
		res = xyzModem_stream_read(buf, BUF_SIZE, &err);
		எனில் (res <= 0)
			நிறுத்து;

		info->image_read += res;
		எனில் (res > copy_size) {
			memcpy(addr, buf, copy_size);
			செல் done;
		}
		memcpy(addr, buf, res);
		addr += res;
		copy_size -= res;
	}

done:
	திரும்பு size;
}

முழு spl_ymodem_load_image(கட்டமைப்பு spl_image_info *spl_image,
			  கட்டமைப்பு spl_boot_device *bootdev)
{
	ulong size = 0;
	முழு err;
	முழு res;
	முழு ret;
	connection_info_t info;
	எழுத்து buf[BUF_SIZE];
	கட்டமைப்பு legacy_img_hdr *ih = வெற்று;
	ulong addr = 0;

	info.mode = xyzModem_ymodem;
	ret = xyzModem_stream_open(&info, &err);
	எனில் (ret) {
		printf("spl: ymodem err - %s\n", xyzModem_error(err));
		திரும்பு ret;
	}

	res = xyzModem_stream_read(buf, BUF_SIZE, &err);
	எனில் (res <= 0)
		செல் end_stream;

	எனில் (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL) &&
	    image_get_magic((கட்டமைப்பு legacy_img_hdr *)buf) == FDT_MAGIC) {
		addr = CONFIG_SYS_LOAD_ADDR;
		ih = (கட்டமைப்பு legacy_img_hdr *)addr;

		memcpy((வெறுமை *)addr, buf, res);
		size += res;
		addr += res;

		போது ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
			memcpy((வெறுமை *)addr, buf, res);
			size += res;
			addr += res;
		}

		ret = spl_parse_image_header(spl_image, bootdev, ih);
		எனில் (ret)
			திரும்பு ret;
	} இல்லையெனில் எனில் (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	    image_get_magic((கட்டமைப்பு legacy_img_hdr *)buf) == FDT_MAGIC) {
		கட்டமைப்பு spl_load_info load;
		கட்டமைப்பு ymodem_fit_info info;

		debug("Found FIT\n");
		spl_load_init(&load, ymodem_read_fit, (வெறுமை *)&info, 1);
		info.buf = buf;
		info.image_read = BUF_SIZE;
		ret = spl_load_simple_fit(spl_image, &load, 0, (வெறுமை *)buf);
		size = info.image_read;

		போது ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0)
			size += res;
	} இல்லையெனில் {
		ih = (கட்டமைப்பு legacy_img_hdr *)buf;
		ret = spl_parse_image_header(spl_image, bootdev, ih);
		எனில் (ret)
			செல் end_stream;
#ifdef CONFIG_SPL_GZIP
		எனில் (ih->ih_comp == IH_COMP_GZIP)
			addr = CONFIG_SYS_LOAD_ADDR;
		இல்லையெனில்
#endif
			addr = spl_image->load_addr;
		memcpy((வெறுமை *)addr, buf, res);
		ih = (கட்டமைப்பு legacy_img_hdr *)addr;
		size += res;
		addr += res;

		போது ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
			memcpy((வெறுமை *)addr, buf, res);
			size += res;
			addr += res;
		}
	}

end_stream:
	xyzModem_stream_close(&err);
	xyzModem_stream_terminate(false, &getcymodem);

	printf("Loaded %lu bytes\n", size);

#ifdef CONFIG_SPL_GZIP
	எனில் (!(IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	      image_get_magic((கட்டமைப்பு legacy_img_hdr *)buf) == FDT_MAGIC) &&
	    (ih->ih_comp == IH_COMP_GZIP)) {
		எனில் (gunzip((வெறுமை *)(spl_image->load_addr + அளவு(*ih)),
			   CONFIG_SYS_BOOTM_LEN,
			   (வெறுமை *)(CONFIG_SYS_LOAD_ADDR + அளவு(*ih)),
			   &size)) {
			puts("Uncompressing error\n");
			திரும்பு -EIO;
		}
	}
#endif

	திரும்பு ret;
}
SPL_LOAD_IMAGE_METHOD("UART", 0, BOOT_DEVICE_UART, spl_ymodem_load_image);
