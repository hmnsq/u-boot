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
#अन्तर्भाव <gzip.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <xyzModem.h>
#अन्तर्भाव <linux/libfdt.h>

#define BUF_SIZE 1024

/*
 * Information required to load image using ymodem.
 *
 * @image_read: Now of bytes read from the image.
 * @buf: pointer to the previous read block.
 */
संरचना ymodem_fit_info {
	पूर्णाङ्क image_read;
	वर्ण *buf;
};

स्थिर पूर्णाङ्क getcymodem(शून्य) {
	सचे (tstc())
		पटिनिवत्त (getchar());
	पटिनिवत्त -1;
}

स्थिर ulong ymodem_read_fit(संरचना spl_load_info *load, ulong offset,
			     ulong size, शून्य *addr)
{
	पूर्णाङ्क res, err, buf_offset;
	संरचना ymodem_fit_info *info = load->priv;
	वर्ण *buf = info->buf;
	ulong copy_size = size;

	याव (info->image_read < offset) {
		res = xyzModem_stream_read(buf, BUF_SIZE, &err);
		सचे (res <= 0)
			भङ्ग;

		info->image_read += res;
	}

	सचे (info->image_read > offset) {
		res = info->image_read - offset;
		सचे (info->image_read % BUF_SIZE)
			buf_offset = (info->image_read % BUF_SIZE);
		अञ्ञथा
			buf_offset = BUF_SIZE;

		सचे (res > copy_size) {
			memcpy(addr, &buf[buf_offset - res], copy_size);
			गच्छ done;
		}
		memcpy(addr, &buf[buf_offset - res], res);
		addr = addr + res;
		copy_size -= res;
	}

	याव (info->image_read < offset + size) {
		res = xyzModem_stream_read(buf, BUF_SIZE, &err);
		सचे (res <= 0)
			भङ्ग;

		info->image_read += res;
		सचे (res > copy_size) {
			memcpy(addr, buf, copy_size);
			गच्छ done;
		}
		memcpy(addr, buf, res);
		addr += res;
		copy_size -= res;
	}

done:
	पटिनिवत्त size;
}

पूर्णाङ्क spl_ymodem_load_image(संरचना spl_image_info *spl_image,
			  संरचना spl_boot_device *bootdev)
{
	ulong size = 0;
	पूर्णाङ्क err;
	पूर्णाङ्क res;
	पूर्णाङ्क ret;
	connection_info_t info;
	वर्ण buf[BUF_SIZE];
	संरचना legacy_img_hdr *ih = रिक्त;
	ulong addr = 0;

	info.mode = xyzModem_ymodem;
	ret = xyzModem_stream_open(&info, &err);
	सचे (ret) {
		printf("spl: ymodem err - %s\n", xyzModem_error(err));
		पटिनिवत्त ret;
	}

	res = xyzModem_stream_read(buf, BUF_SIZE, &err);
	सचे (res <= 0)
		गच्छ end_stream;

	सचे (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL) &&
	    image_get_magic((संरचना legacy_img_hdr *)buf) == FDT_MAGIC) {
		addr = CONFIG_SYS_LOAD_ADDR;
		ih = (संरचना legacy_img_hdr *)addr;

		memcpy((शून्य *)addr, buf, res);
		size += res;
		addr += res;

		याव ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
			memcpy((शून्य *)addr, buf, res);
			size += res;
			addr += res;
		}

		ret = spl_parse_image_header(spl_image, bootdev, ih);
		सचे (ret)
			पटिनिवत्त ret;
	} अञ्ञथा सचे (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	    image_get_magic((संरचना legacy_img_hdr *)buf) == FDT_MAGIC) {
		संरचना spl_load_info load;
		संरचना ymodem_fit_info info;

		debug("Found FIT\n");
		spl_load_init(&load, ymodem_read_fit, (शून्य *)&info, 1);
		info.buf = buf;
		info.image_read = BUF_SIZE;
		ret = spl_load_simple_fit(spl_image, &load, 0, (शून्य *)buf);
		size = info.image_read;

		याव ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0)
			size += res;
	} अञ्ञथा {
		ih = (संरचना legacy_img_hdr *)buf;
		ret = spl_parse_image_header(spl_image, bootdev, ih);
		सचे (ret)
			गच्छ end_stream;
#ifdef CONFIG_SPL_GZIP
		सचे (ih->ih_comp == IH_COMP_GZIP)
			addr = CONFIG_SYS_LOAD_ADDR;
		अञ्ञथा
#endif
			addr = spl_image->load_addr;
		memcpy((शून्य *)addr, buf, res);
		ih = (संरचना legacy_img_hdr *)addr;
		size += res;
		addr += res;

		याव ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
			memcpy((शून्य *)addr, buf, res);
			size += res;
			addr += res;
		}
	}

end_stream:
	xyzModem_stream_close(&err);
	xyzModem_stream_terminate(false, &getcymodem);

	printf("Loaded %lu bytes\n", size);

#ifdef CONFIG_SPL_GZIP
	सचे (!(IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	      image_get_magic((संरचना legacy_img_hdr *)buf) == FDT_MAGIC) &&
	    (ih->ih_comp == IH_COMP_GZIP)) {
		सचे (gunzip((शून्य *)(spl_image->load_addr + परिमाण(*ih)),
			   CONFIG_SYS_BOOTM_LEN,
			   (शून्य *)(CONFIG_SYS_LOAD_ADDR + परिमाण(*ih)),
			   &size)) {
			puts("Uncompressing error\n");
			पटिनिवत्त -EIO;
		}
	}
#endif

	पटिनिवत्त ret;
}
SPL_LOAD_IMAGE_METHOD("UART", 0, BOOT_DEVICE_UART, spl_ymodem_load_image);
