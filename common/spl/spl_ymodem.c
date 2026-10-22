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
#അന്തര്ഭാവ <gzip.h>
#അന്തര്ഭാവ <image.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <xyzModem.h>
#അന്തര്ഭാവ <linux/libfdt.h>

#define BUF_SIZE 1024

/*
 * Information required to load image using ymodem.
 *
 * @image_read: Now of bytes read from the image.
 * @buf: pointer to the previous read block.
 */
ഘടന ymodem_fit_info {
	പൂർണ്ണസംഖ്യ image_read;
	അക്ഷരം *buf;
};

സ്ഥിതം പൂർണ്ണസംഖ്യ getcymodem(ശൂന്യം) {
	എങ്കിൽ (tstc())
		തിരികെ (getchar());
	തിരികെ -1;
}

സ്ഥിതം ulong ymodem_read_fit(ഘടന spl_load_info *load, ulong offset,
			     ulong size, ശൂന്യം *addr)
{
	പൂർണ്ണസംഖ്യ res, err, buf_offset;
	ഘടന ymodem_fit_info *info = load->priv;
	അക്ഷരം *buf = info->buf;
	ulong copy_size = size;

	ആകുമ്പോൾ (info->image_read < offset) {
		res = xyzModem_stream_read(buf, BUF_SIZE, &err);
		എങ്കിൽ (res <= 0)
			നിർത്തുക;

		info->image_read += res;
	}

	എങ്കിൽ (info->image_read > offset) {
		res = info->image_read - offset;
		എങ്കിൽ (info->image_read % BUF_SIZE)
			buf_offset = (info->image_read % BUF_SIZE);
		അല്ലെങ്കിൽ
			buf_offset = BUF_SIZE;

		എങ്കിൽ (res > copy_size) {
			memcpy(addr, &buf[buf_offset - res], copy_size);
			പോകുക done;
		}
		memcpy(addr, &buf[buf_offset - res], res);
		addr = addr + res;
		copy_size -= res;
	}

	ആകുമ്പോൾ (info->image_read < offset + size) {
		res = xyzModem_stream_read(buf, BUF_SIZE, &err);
		എങ്കിൽ (res <= 0)
			നിർത്തുക;

		info->image_read += res;
		എങ്കിൽ (res > copy_size) {
			memcpy(addr, buf, copy_size);
			പോകുക done;
		}
		memcpy(addr, buf, res);
		addr += res;
		copy_size -= res;
	}

done:
	തിരികെ size;
}

പൂർണ്ണസംഖ്യ spl_ymodem_load_image(ഘടന spl_image_info *spl_image,
			  ഘടന spl_boot_device *bootdev)
{
	ulong size = 0;
	പൂർണ്ണസംഖ്യ err;
	പൂർണ്ണസംഖ്യ res;
	പൂർണ്ണസംഖ്യ ret;
	connection_info_t info;
	അക്ഷരം buf[BUF_SIZE];
	ഘടന legacy_img_hdr *ih = ശൂന്യ;
	ulong addr = 0;

	info.mode = xyzModem_ymodem;
	ret = xyzModem_stream_open(&info, &err);
	എങ്കിൽ (ret) {
		printf("spl: ymodem err - %s\n", xyzModem_error(err));
		തിരികെ ret;
	}

	res = xyzModem_stream_read(buf, BUF_SIZE, &err);
	എങ്കിൽ (res <= 0)
		പോകുക end_stream;

	എങ്കിൽ (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL) &&
	    image_get_magic((ഘടന legacy_img_hdr *)buf) == FDT_MAGIC) {
		addr = CONFIG_SYS_LOAD_ADDR;
		ih = (ഘടന legacy_img_hdr *)addr;

		memcpy((ശൂന്യം *)addr, buf, res);
		size += res;
		addr += res;

		ആകുമ്പോൾ ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
			memcpy((ശൂന്യം *)addr, buf, res);
			size += res;
			addr += res;
		}

		ret = spl_parse_image_header(spl_image, bootdev, ih);
		എങ്കിൽ (ret)
			തിരികെ ret;
	} അല്ലെങ്കിൽ എങ്കിൽ (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	    image_get_magic((ഘടന legacy_img_hdr *)buf) == FDT_MAGIC) {
		ഘടന spl_load_info load;
		ഘടന ymodem_fit_info info;

		debug("Found FIT\n");
		spl_load_init(&load, ymodem_read_fit, (ശൂന്യം *)&info, 1);
		info.buf = buf;
		info.image_read = BUF_SIZE;
		ret = spl_load_simple_fit(spl_image, &load, 0, (ശൂന്യം *)buf);
		size = info.image_read;

		ആകുമ്പോൾ ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0)
			size += res;
	} അല്ലെങ്കിൽ {
		ih = (ഘടന legacy_img_hdr *)buf;
		ret = spl_parse_image_header(spl_image, bootdev, ih);
		എങ്കിൽ (ret)
			പോകുക end_stream;
#ifdef CONFIG_SPL_GZIP
		എങ്കിൽ (ih->ih_comp == IH_COMP_GZIP)
			addr = CONFIG_SYS_LOAD_ADDR;
		അല്ലെങ്കിൽ
#endif
			addr = spl_image->load_addr;
		memcpy((ശൂന്യം *)addr, buf, res);
		ih = (ഘടന legacy_img_hdr *)addr;
		size += res;
		addr += res;

		ആകുമ്പോൾ ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
			memcpy((ശൂന്യം *)addr, buf, res);
			size += res;
			addr += res;
		}
	}

end_stream:
	xyzModem_stream_close(&err);
	xyzModem_stream_terminate(false, &getcymodem);

	printf("Loaded %lu bytes\n", size);

#ifdef CONFIG_SPL_GZIP
	എങ്കിൽ (!(IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	      image_get_magic((ഘടന legacy_img_hdr *)buf) == FDT_MAGIC) &&
	    (ih->ih_comp == IH_COMP_GZIP)) {
		എങ്കിൽ (gunzip((ശൂന്യം *)(spl_image->load_addr + വലുപ്പം(*ih)),
			   CONFIG_SYS_BOOTM_LEN,
			   (ശൂന്യം *)(CONFIG_SYS_LOAD_ADDR + വലുപ്പം(*ih)),
			   &size)) {
			puts("Uncompressing error\n");
			തിരികെ -EIO;
		}
	}
#endif

	തിരികെ ret;
}
SPL_LOAD_IMAGE_METHOD("UART", 0, BOOT_DEVICE_UART, spl_ymodem_load_image);
