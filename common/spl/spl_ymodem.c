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
#অন্তর্ভাব <gzip.h>
#অন্তর্ভাব <image.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <spl.h>
#অন্তর্ভাব <xyzModem.h>
#অন্তর্ভাব <linux/libfdt.h>

#define BUF_আকার 1024

/*
 * Information required to load image using ymodem.
 *
 * @image_read: Now of bytes read from the image.
 * @buf: pointer to the previous read block.
 */
কাঠামো ymodem_fit_info {
	পূর্ণসংখ্যা image_read;
	অক্ষর *buf;
};

স্থির পূর্ণসংখ্যা getcymodem(শূন্য) {
	যদি (tstc())
		ফেরত (getchar());
	ফেরত -1;
}

স্থির ulong ymodem_read_fit(কাঠামো spl_load_info *load, ulong offset,
			     ulong আকার_2, শূন্য *addr)
{
	পূর্ণসংখ্যা res, err, buf_offset;
	কাঠামো ymodem_fit_info *info = load->priv;
	অক্ষর *buf = info->buf;
	ulong copy_আকার = আকার_2;

	যখন (info->image_read < offset) {
		res = xyzModem_stream_read(buf, BUF_আকার, &err);
		যদি (res <= 0)
			বিরতি;

		info->image_read += res;
	}

	যদি (info->image_read > offset) {
		res = info->image_read - offset;
		যদি (info->image_read % BUF_আকার)
			buf_offset = (info->image_read % BUF_আকার);
		অন্যথা
			buf_offset = BUF_আকার;

		যদি (res > copy_আকার) {
			memcpy(addr, &buf[buf_offset - res], copy_আকার);
			যাও done;
		}
		memcpy(addr, &buf[buf_offset - res], res);
		addr = addr + res;
		copy_আকার -= res;
	}

	যখন (info->image_read < offset + আকার_2) {
		res = xyzModem_stream_read(buf, BUF_আকার, &err);
		যদি (res <= 0)
			বিরতি;

		info->image_read += res;
		যদি (res > copy_আকার) {
			memcpy(addr, buf, copy_আকার);
			যাও done;
		}
		memcpy(addr, buf, res);
		addr += res;
		copy_আকার -= res;
	}

done:
	ফেরত আকার_2;
}

পূর্ণসংখ্যা spl_ymodem_load_image(কাঠামো spl_image_info *spl_image,
			  কাঠামো spl_boot_device *bootdev)
{
	ulong আকার_2 = 0;
	পূর্ণসংখ্যা err;
	পূর্ণসংখ্যা res;
	পূর্ণসংখ্যা ret;
	connection_info_t info;
	অক্ষর buf[BUF_আকার];
	কাঠামো legacy_img_hdr *ih = ফাঁকা;
	ulong addr = 0;

	info.mode = xyzModem_ymodem;
	ret = xyzModem_stream_open(&info, &err);
	যদি (ret) {
		printf("spl: ymodem err - %s\n", xyzModem_error(err));
		ফেরত ret;
	}

	res = xyzModem_stream_read(buf, BUF_আকার, &err);
	যদি (res <= 0)
		যাও end_stream;

	যদি (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL) &&
	    image_get_magic((কাঠামো legacy_img_hdr *)buf) == FDT_MAGIC) {
		addr = CONFIG_SYS_LOAD_ADDR;
		ih = (কাঠামো legacy_img_hdr *)addr;

		memcpy((শূন্য *)addr, buf, res);
		আকার_2 += res;
		addr += res;

		যখন ((res = xyzModem_stream_read(buf, BUF_আকার, &err)) > 0) {
			memcpy((শূন্য *)addr, buf, res);
			আকার_2 += res;
			addr += res;
		}

		ret = spl_parse_image_header(spl_image, bootdev, ih);
		যদি (ret)
			ফেরত ret;
	} অন্যথা যদি (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	    image_get_magic((কাঠামো legacy_img_hdr *)buf) == FDT_MAGIC) {
		কাঠামো spl_load_info load;
		কাঠামো ymodem_fit_info info;

		debug("Found FIT\n");
		spl_load_init(&load, ymodem_read_fit, (শূন্য *)&info, 1);
		info.buf = buf;
		info.image_read = BUF_আকার;
		ret = spl_load_simple_fit(spl_image, &load, 0, (শূন্য *)buf);
		আকার_2 = info.image_read;

		যখন ((res = xyzModem_stream_read(buf, BUF_আকার, &err)) > 0)
			আকার_2 += res;
	} অন্যথা {
		ih = (কাঠামো legacy_img_hdr *)buf;
		ret = spl_parse_image_header(spl_image, bootdev, ih);
		যদি (ret)
			যাও end_stream;
#ifdef CONFIG_SPL_GZIP
		যদি (ih->ih_comp == IH_COMP_GZIP)
			addr = CONFIG_SYS_LOAD_ADDR;
		অন্যথা
#endif
			addr = spl_image->load_addr;
		memcpy((শূন্য *)addr, buf, res);
		ih = (কাঠামো legacy_img_hdr *)addr;
		আকার_2 += res;
		addr += res;

		যখন ((res = xyzModem_stream_read(buf, BUF_আকার, &err)) > 0) {
			memcpy((শূন্য *)addr, buf, res);
			আকার_2 += res;
			addr += res;
		}
	}

end_stream:
	xyzModem_stream_close(&err);
	xyzModem_stream_terminate(false, &getcymodem);

	printf("Loaded %lu bytes\n", আকার_2);

#ifdef CONFIG_SPL_GZIP
	যদি (!(IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	      image_get_magic((কাঠামো legacy_img_hdr *)buf) == FDT_MAGIC) &&
	    (ih->ih_comp == IH_COMP_GZIP)) {
		যদি (gunzip((শূন্য *)(spl_image->load_addr + আকার(*ih)),
			   CONFIG_SYS_BOOTM_LEN,
			   (শূন্য *)(CONFIG_SYS_LOAD_ADDR + আকার(*ih)),
			   &আকার_2)) {
			puts("Uncompressing error\n");
			ফেরত -EIO;
		}
	}
#endif

	ফেরত ret;
}
SPL_LOAD_IMAGE_METHOD("UART", 0, BOOT_DEVICE_UART, spl_ymodem_load_image);
