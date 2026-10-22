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
#شامل <gzip.h>
#شامل <image.h>
#شامل <log.h>
#شامل <spl.h>
#شامل <xyzModem.h>
#شامل <linux/libfdt.h>

#define BUF_SIZE 1024

/*
 * Information required to load image using ymodem.
 *
 * @image_read: Now of bytes read from the image.
 * @buf: pointer to the previous read block.
 */
ساختار ymodem_fit_info {
	صحیح image_read;
	نویسه *buf;
};

ایستا صحیح getcymodem(تهی) {
	اگر (tstc())
		بازگشت (getchar());
	بازگشت -1;
}

ایستا ulong ymodem_read_fit(ساختار spl_load_info *load, ulong offset,
			     ulong size, تهی *addr)
{
	صحیح res, err, buf_offset;
	ساختار ymodem_fit_info *info = load->priv;
	نویسه *buf = info->buf;
	ulong copy_size = size;

	وقتی (info->image_read < offset) {
		res = xyzModem_stream_read(buf, BUF_SIZE, &err);
		اگر (res <= 0)
			بشکن;

		info->image_read += res;
	}

	اگر (info->image_read > offset) {
		res = info->image_read - offset;
		اگر (info->image_read % BUF_SIZE)
			buf_offset = (info->image_read % BUF_SIZE);
		وگرنه
			buf_offset = BUF_SIZE;

		اگر (res > copy_size) {
			memcpy(addr, &buf[buf_offset - res], copy_size);
			برو done;
		}
		memcpy(addr, &buf[buf_offset - res], res);
		addr = addr + res;
		copy_size -= res;
	}

	وقتی (info->image_read < offset + size) {
		res = xyzModem_stream_read(buf, BUF_SIZE, &err);
		اگر (res <= 0)
			بشکن;

		info->image_read += res;
		اگر (res > copy_size) {
			memcpy(addr, buf, copy_size);
			برو done;
		}
		memcpy(addr, buf, res);
		addr += res;
		copy_size -= res;
	}

done:
	بازگشت size;
}

صحیح spl_ymodem_load_image(ساختار spl_image_info *spl_image,
			  ساختار spl_boot_device *bootdev)
{
	ulong size = 0;
	صحیح err;
	صحیح res;
	صحیح ret;
	connection_info_t info;
	نویسه buf[BUF_SIZE];
	ساختار legacy_img_hdr *ih = پوچ;
	ulong addr = 0;

	info.mode = xyzModem_ymodem;
	ret = xyzModem_stream_open(&info, &err);
	اگر (ret) {
		printf("spl: ymodem err - %s\n", xyzModem_error(err));
		بازگشت ret;
	}

	res = xyzModem_stream_read(buf, BUF_SIZE, &err);
	اگر (res <= 0)
		برو end_stream;

	اگر (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL) &&
	    image_get_magic((ساختار legacy_img_hdr *)buf) == FDT_MAGIC) {
		addr = CONFIG_SYS_LOAD_ADDR;
		ih = (ساختار legacy_img_hdr *)addr;

		memcpy((تهی *)addr, buf, res);
		size += res;
		addr += res;

		وقتی ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
			memcpy((تهی *)addr, buf, res);
			size += res;
			addr += res;
		}

		ret = spl_parse_image_header(spl_image, bootdev, ih);
		اگر (ret)
			بازگشت ret;
	} وگرنه اگر (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	    image_get_magic((ساختار legacy_img_hdr *)buf) == FDT_MAGIC) {
		ساختار spl_load_info load;
		ساختار ymodem_fit_info info;

		debug("Found FIT\n");
		spl_load_init(&load, ymodem_read_fit, (تهی *)&info, 1);
		info.buf = buf;
		info.image_read = BUF_SIZE;
		ret = spl_load_simple_fit(spl_image, &load, 0, (تهی *)buf);
		size = info.image_read;

		وقتی ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0)
			size += res;
	} وگرنه {
		ih = (ساختار legacy_img_hdr *)buf;
		ret = spl_parse_image_header(spl_image, bootdev, ih);
		اگر (ret)
			برو end_stream;
#ifdef CONFIG_SPL_GZIP
		اگر (ih->ih_comp == IH_COMP_GZIP)
			addr = CONFIG_SYS_LOAD_ADDR;
		وگرنه
#endif
			addr = spl_image->load_addr;
		memcpy((تهی *)addr, buf, res);
		ih = (ساختار legacy_img_hdr *)addr;
		size += res;
		addr += res;

		وقتی ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
			memcpy((تهی *)addr, buf, res);
			size += res;
			addr += res;
		}
	}

end_stream:
	xyzModem_stream_close(&err);
	xyzModem_stream_terminate(false, &getcymodem);

	printf("Loaded %lu bytes\n", size);

#ifdef CONFIG_SPL_GZIP
	اگر (!(IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	      image_get_magic((ساختار legacy_img_hdr *)buf) == FDT_MAGIC) &&
	    (ih->ih_comp == IH_COMP_GZIP)) {
		اگر (gunzip((تهی *)(spl_image->load_addr + اندازه(*ih)),
			   CONFIG_SYS_BOOTM_LEN,
			   (تهی *)(CONFIG_SYS_LOAD_ADDR + اندازه(*ih)),
			   &size)) {
			puts("Uncompressing error\n");
			بازگشت -EIO;
		}
	}
#endif

	بازگشت ret;
}
SPL_LOAD_IMAGE_METHOD("UART", 0, BOOT_DEVICE_UART, spl_ymodem_load_image);
