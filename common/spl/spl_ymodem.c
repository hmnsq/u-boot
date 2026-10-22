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
#ضمن <gzip.h>
#ضمن <image.h>
#ضمن <log.h>
#ضمن <spl.h>
#ضمن <xyzModem.h>
#ضمن <linux/libfdt.h>

#define BUF_SIZE 1024

/*
 * Information required to load image using ymodem.
 *
 * @image_read: Now of bytes read from the image.
 * @buf: pointer to the previous read block.
 */
بنية ymodem_fit_info {
	صحيح image_read;
	حرف *buf;
};

ساكن صحيح getcymodem(فراغ) {
	𐡄𐡍 (tstc())
		أرجع (getchar());
	أرجع -1;
}

ساكن ulong ymodem_read_fit(بنية spl_load_info *load, ulong offset,
			     ulong size, فراغ *addr)
{
	صحيح res, err, buf_offset;
	بنية ymodem_fit_info *info = load->priv;
	حرف *buf = info->buf;
	ulong copy_size = size;

	طالما (info->image_read < offset) {
		res = xyzModem_stream_read(buf, BUF_SIZE, &err);
		𐡄𐡍 (res <= 0)
			اكسر;

		info->image_read += res;
	}

	𐡄𐡍 (info->image_read > offset) {
		res = info->image_read - offset;
		𐡄𐡍 (info->image_read % BUF_SIZE)
			buf_offset = (info->image_read % BUF_SIZE);
		وإلا
			buf_offset = BUF_SIZE;

		𐡄𐡍 (res > copy_size) {
			memcpy(addr, &buf[buf_offset - res], copy_size);
			اذهب done;
		}
		memcpy(addr, &buf[buf_offset - res], res);
		addr = addr + res;
		copy_size -= res;
	}

	طالما (info->image_read < offset + size) {
		res = xyzModem_stream_read(buf, BUF_SIZE, &err);
		𐡄𐡍 (res <= 0)
			اكسر;

		info->image_read += res;
		𐡄𐡍 (res > copy_size) {
			memcpy(addr, buf, copy_size);
			اذهب done;
		}
		memcpy(addr, buf, res);
		addr += res;
		copy_size -= res;
	}

done:
	أرجع size;
}

صحيح spl_ymodem_load_image(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev)
{
	ulong size = 0;
	صحيح err;
	صحيح res;
	صحيح ret;
	connection_info_t info;
	حرف buf[BUF_SIZE];
	بنية legacy_img_hdr *ih = عدم;
	ulong addr = 0;

	info.mode = xyzModem_ymodem;
	ret = xyzModem_stream_open(&info, &err);
	𐡄𐡍 (ret) {
		printf("spl: ymodem err - %s\n", xyzModem_error(err));
		أرجع ret;
	}

	res = xyzModem_stream_read(buf, BUF_SIZE, &err);
	𐡄𐡍 (res <= 0)
		اذهب end_stream;

	𐡄𐡍 (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL) &&
	    image_get_magic((بنية legacy_img_hdr *)buf) == FDT_MAGIC) {
		addr = CONFIG_SYS_LOAD_ADDR;
		ih = (بنية legacy_img_hdr *)addr;

		memcpy((فراغ *)addr, buf, res);
		size += res;
		addr += res;

		طالما ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
			memcpy((فراغ *)addr, buf, res);
			size += res;
			addr += res;
		}

		ret = spl_parse_image_header(spl_image, bootdev, ih);
		𐡄𐡍 (ret)
			أرجع ret;
	} وإلا 𐡄𐡍 (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	    image_get_magic((بنية legacy_img_hdr *)buf) == FDT_MAGIC) {
		بنية spl_load_info load;
		بنية ymodem_fit_info info;

		debug("Found FIT\n");
		spl_load_init(&load, ymodem_read_fit, (فراغ *)&info, 1);
		info.buf = buf;
		info.image_read = BUF_SIZE;
		ret = spl_load_simple_fit(spl_image, &load, 0, (فراغ *)buf);
		size = info.image_read;

		طالما ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0)
			size += res;
	} وإلا {
		ih = (بنية legacy_img_hdr *)buf;
		ret = spl_parse_image_header(spl_image, bootdev, ih);
		𐡄𐡍 (ret)
			اذهب end_stream;
#ifdef CONFIG_SPL_GZIP
		𐡄𐡍 (ih->ih_comp == IH_COMP_GZIP)
			addr = CONFIG_SYS_LOAD_ADDR;
		وإلا
#endif
			addr = spl_image->load_addr;
		memcpy((فراغ *)addr, buf, res);
		ih = (بنية legacy_img_hdr *)addr;
		size += res;
		addr += res;

		طالما ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
			memcpy((فراغ *)addr, buf, res);
			size += res;
			addr += res;
		}
	}

end_stream:
	xyzModem_stream_close(&err);
	xyzModem_stream_terminate(false, &getcymodem);

	printf("Loaded %lu bytes\n", size);

#ifdef CONFIG_SPL_GZIP
	𐡄𐡍 (!(IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	      image_get_magic((بنية legacy_img_hdr *)buf) == FDT_MAGIC) &&
	    (ih->ih_comp == IH_COMP_GZIP)) {
		𐡄𐡍 (gunzip((فراغ *)(spl_image->load_addr + حجم(*ih)),
			   CONFIG_SYS_BOOTM_LEN,
			   (فراغ *)(CONFIG_SYS_LOAD_ADDR + حجم(*ih)),
			   &size)) {
			puts("Uncompressing error\n");
			أرجع -EIO;
		}
	}
#endif

	أرجع ret;
}
SPL_LOAD_IMAGE_METHOD("UART", 0, BOOT_DEVICE_UART, spl_ymodem_load_image);
