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
#অন্তর্ভাৱ <gzip.h>
#অন্তর্ভাৱ <image.h>
#অন্তর্ভাৱ <log.h>
#অন্তর্ভাৱ <spl.h>
#অন্তর্ভাৱ <xyzModem.h>
#অন্তর্ভাৱ <linux/libfdt.h>

#define BUF_SIZE 1024

/*
 * Information required to load image using ymodem.
 *
 * @image_read: Now of bytes read from the image.
 * @buf: pointer to the previous read block.
 */
সংরচনা ymodem_fit_info {
	পূর্ণাঙ্ক image_read;
	ৱর্ণ *buf;
};

স্থির পূর্ণাঙ্ক getcymodem(শূন্য) {
	যদি (tstc())
		ঘূৰাই (getchar());
	ঘূৰাই -1;
}

স্থির ulong ymodem_read_fit(সংরচনা spl_load_info *load, ulong offset,
			     ulong size, শূন্য *addr)
{
	পূর্ণাঙ্ক res, err, buf_offset;
	সংরচনা ymodem_fit_info *info = load->priv;
	ৱর্ণ *buf = info->buf;
	ulong copy_size = size;

	যেতিয়ালৈ (info->image_read < offset) {
		res = xyzModem_stream_read(buf, BUF_SIZE, &err);
		যদি (res <= 0)
			ভঙ্গ;

		info->image_read += res;
	}

	যদি (info->image_read > offset) {
		res = info->image_read - offset;
		যদি (info->image_read % BUF_SIZE)
			buf_offset = (info->image_read % BUF_SIZE);
		অন্যথা
			buf_offset = BUF_SIZE;

		যদি (res > copy_size) {
			memcpy(addr, &buf[buf_offset - res], copy_size);
			গচ্ছ done;
		}
		memcpy(addr, &buf[buf_offset - res], res);
		addr = addr + res;
		copy_size -= res;
	}

	যেতিয়ালৈ (info->image_read < offset + size) {
		res = xyzModem_stream_read(buf, BUF_SIZE, &err);
		যদি (res <= 0)
			ভঙ্গ;

		info->image_read += res;
		যদি (res > copy_size) {
			memcpy(addr, buf, copy_size);
			গচ্ছ done;
		}
		memcpy(addr, buf, res);
		addr += res;
		copy_size -= res;
	}

done:
	ঘূৰাই size;
}

পূর্ণাঙ্ক spl_ymodem_load_image(সংরচনা spl_image_info *spl_image,
			  সংরচনা spl_boot_device *bootdev)
{
	ulong size = 0;
	পূর্ণাঙ্ক err;
	পূর্ণাঙ্ক res;
	পূর্ণাঙ্ক ret;
	connection_info_t info;
	ৱর্ণ buf[BUF_SIZE];
	সংরচনা legacy_img_hdr *ih = রিক্ত;
	ulong addr = 0;

	info.mode = xyzModem_ymodem;
	ret = xyzModem_stream_open(&info, &err);
	যদি (ret) {
		printf("spl: ymodem err - %s\n", xyzModem_error(err));
		ঘূৰাই ret;
	}

	res = xyzModem_stream_read(buf, BUF_SIZE, &err);
	যদি (res <= 0)
		গচ্ছ end_stream;

	যদি (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL) &&
	    image_get_magic((সংরচনা legacy_img_hdr *)buf) == FDT_MAGIC) {
		addr = CONFIG_SYS_LOAD_ADDR;
		ih = (সংরচনা legacy_img_hdr *)addr;

		memcpy((শূন্য *)addr, buf, res);
		size += res;
		addr += res;

		যেতিয়ালৈ ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
			memcpy((শূন্য *)addr, buf, res);
			size += res;
			addr += res;
		}

		ret = spl_parse_image_header(spl_image, bootdev, ih);
		যদি (ret)
			ঘূৰাই ret;
	} অন্যথা যদি (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	    image_get_magic((সংরচনা legacy_img_hdr *)buf) == FDT_MAGIC) {
		সংরচনা spl_load_info load;
		সংরচনা ymodem_fit_info info;

		debug("Found FIT\n");
		spl_load_init(&load, ymodem_read_fit, (শূন্য *)&info, 1);
		info.buf = buf;
		info.image_read = BUF_SIZE;
		ret = spl_load_simple_fit(spl_image, &load, 0, (শূন্য *)buf);
		size = info.image_read;

		যেতিয়ালৈ ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0)
			size += res;
	} অন্যথা {
		ih = (সংরচনা legacy_img_hdr *)buf;
		ret = spl_parse_image_header(spl_image, bootdev, ih);
		যদি (ret)
			গচ্ছ end_stream;
#ifdef CONFIG_SPL_GZIP
		যদি (ih->ih_comp == IH_COMP_GZIP)
			addr = CONFIG_SYS_LOAD_ADDR;
		অন্যথা
#endif
			addr = spl_image->load_addr;
		memcpy((শূন্য *)addr, buf, res);
		ih = (সংরচনা legacy_img_hdr *)addr;
		size += res;
		addr += res;

		যেতিয়ালৈ ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
			memcpy((শূন্য *)addr, buf, res);
			size += res;
			addr += res;
		}
	}

end_stream:
	xyzModem_stream_close(&err);
	xyzModem_stream_terminate(false, &getcymodem);

	printf("Loaded %lu bytes\n", size);

#ifdef CONFIG_SPL_GZIP
	যদি (!(IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	      image_get_magic((সংরচনা legacy_img_hdr *)buf) == FDT_MAGIC) &&
	    (ih->ih_comp == IH_COMP_GZIP)) {
		যদি (gunzip((শূন্য *)(spl_image->load_addr + পরিমাণ(*ih)),
			   CONFIG_SYS_BOOTM_LEN,
			   (শূন্য *)(CONFIG_SYS_LOAD_ADDR + পরিমাণ(*ih)),
			   &size)) {
			puts("Uncompressing error\n");
			ঘূৰাই -EIO;
		}
	}
#endif

	ঘূৰাই ret;
}
SPL_LOAD_IMAGE_METHOD("UART", 0, BOOT_DEVICE_UART, spl_ymodem_load_image);
