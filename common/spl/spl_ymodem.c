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
#ಅನ್ತರ್ಭಾವ <gzip.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <xyzModem.h>
#ಅನ್ತರ್ಭಾವ <linux/libfdt.h>

#define BUF_SIZE 1024

/*
 * Information required to load image using ymodem.
 *
 * @image_read: Now of bytes read from the image.
 * @buf: pointer to the previous read block.
 */
ಸಂರಚನಾ ymodem_fit_info {
	ಪೂರ್ಣಾಙ್ಕ image_read;
	ವರ್ಣ *buf;
};

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ getcymodem(ಶೂನ್ಯ) {
	ಯದಿ (tstc())
		ಪ್ರತ್ಯಾಗಮ (getchar());
	ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ulong ymodem_read_fit(ಸಂರಚನಾ spl_load_info *load, ulong offset,
			     ulong size, ಶೂನ್ಯ *addr)
{
	ಪೂರ್ಣಾಙ್ಕ res, err, buf_offset;
	ಸಂರಚನಾ ymodem_fit_info *info = load->priv;
	ವರ್ಣ *buf = info->buf;
	ulong copy_size = size;

	ಯಾವತ್ (info->image_read < offset) {
		res = xyzModem_stream_read(buf, BUF_SIZE, &err);
		ಯದಿ (res <= 0)
			ಭಙ್ಗ;

		info->image_read += res;
	}

	ಯದಿ (info->image_read > offset) {
		res = info->image_read - offset;
		ಯದಿ (info->image_read % BUF_SIZE)
			buf_offset = (info->image_read % BUF_SIZE);
		ಅನ್ಯಥಾ
			buf_offset = BUF_SIZE;

		ಯದಿ (res > copy_size) {
			memcpy(addr, &buf[buf_offset - res], copy_size);
			ಗಚ್ಛ done;
		}
		memcpy(addr, &buf[buf_offset - res], res);
		addr = addr + res;
		copy_size -= res;
	}

	ಯಾವತ್ (info->image_read < offset + size) {
		res = xyzModem_stream_read(buf, BUF_SIZE, &err);
		ಯದಿ (res <= 0)
			ಭಙ್ಗ;

		info->image_read += res;
		ಯದಿ (res > copy_size) {
			memcpy(addr, buf, copy_size);
			ಗಚ್ಛ done;
		}
		memcpy(addr, buf, res);
		addr += res;
		copy_size -= res;
	}

done:
	ಪ್ರತ್ಯಾಗಮ size;
}

ಪೂರ್ಣಾಙ್ಕ spl_ymodem_load_image(ಸಂರಚನಾ spl_image_info *spl_image,
			  ಸಂರಚನಾ spl_boot_device *bootdev)
{
	ulong size = 0;
	ಪೂರ್ಣಾಙ್ಕ err;
	ಪೂರ್ಣಾಙ್ಕ res;
	ಪೂರ್ಣಾಙ್ಕ ret;
	connection_info_t info;
	ವರ್ಣ buf[BUF_SIZE];
	ಸಂರಚನಾ legacy_img_hdr *ih = ರಿಕ್ತ;
	ulong addr = 0;

	info.mode = xyzModem_ymodem;
	ret = xyzModem_stream_open(&info, &err);
	ಯದಿ (ret) {
		printf("spl: ymodem err - %s\n", xyzModem_error(err));
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	res = xyzModem_stream_read(buf, BUF_SIZE, &err);
	ಯದಿ (res <= 0)
		ಗಚ್ಛ end_stream;

	ಯದಿ (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL) &&
	    image_get_magic((ಸಂರಚನಾ legacy_img_hdr *)buf) == FDT_MAGIC) {
		addr = CONFIG_SYS_LOAD_ADDR;
		ih = (ಸಂರಚನಾ legacy_img_hdr *)addr;

		memcpy((ಶೂನ್ಯ *)addr, buf, res);
		size += res;
		addr += res;

		ಯಾವತ್ ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
			memcpy((ಶೂನ್ಯ *)addr, buf, res);
			size += res;
			addr += res;
		}

		ret = spl_parse_image_header(spl_image, bootdev, ih);
		ಯದಿ (ret)
			ಪ್ರತ್ಯಾಗಮ ret;
	} ಅನ್ಯಥಾ ಯದಿ (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	    image_get_magic((ಸಂರಚನಾ legacy_img_hdr *)buf) == FDT_MAGIC) {
		ಸಂರಚನಾ spl_load_info load;
		ಸಂರಚನಾ ymodem_fit_info info;

		debug("Found FIT\n");
		spl_load_init(&load, ymodem_read_fit, (ಶೂನ್ಯ *)&info, 1);
		info.buf = buf;
		info.image_read = BUF_SIZE;
		ret = spl_load_simple_fit(spl_image, &load, 0, (ಶೂನ್ಯ *)buf);
		size = info.image_read;

		ಯಾವತ್ ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0)
			size += res;
	} ಅನ್ಯಥಾ {
		ih = (ಸಂರಚನಾ legacy_img_hdr *)buf;
		ret = spl_parse_image_header(spl_image, bootdev, ih);
		ಯದಿ (ret)
			ಗಚ್ಛ end_stream;
#ifdef CONFIG_SPL_GZIP
		ಯದಿ (ih->ih_comp == IH_COMP_GZIP)
			addr = CONFIG_SYS_LOAD_ADDR;
		ಅನ್ಯಥಾ
#endif
			addr = spl_image->load_addr;
		memcpy((ಶೂನ್ಯ *)addr, buf, res);
		ih = (ಸಂರಚನಾ legacy_img_hdr *)addr;
		size += res;
		addr += res;

		ಯಾವತ್ ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
			memcpy((ಶೂನ್ಯ *)addr, buf, res);
			size += res;
			addr += res;
		}
	}

end_stream:
	xyzModem_stream_close(&err);
	xyzModem_stream_terminate(false, &getcymodem);

	printf("Loaded %lu bytes\n", size);

#ifdef CONFIG_SPL_GZIP
	ಯದಿ (!(IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	      image_get_magic((ಸಂರಚನಾ legacy_img_hdr *)buf) == FDT_MAGIC) &&
	    (ih->ih_comp == IH_COMP_GZIP)) {
		ಯದಿ (gunzip((ಶೂನ್ಯ *)(spl_image->load_addr + ಪರಿಮಾಣ(*ih)),
			   CONFIG_SYS_BOOTM_LEN,
			   (ಶೂನ್ಯ *)(CONFIG_SYS_LOAD_ADDR + ಪರಿಮಾಣ(*ih)),
			   &size)) {
			puts("Uncompressing error\n");
			ಪ್ರತ್ಯಾಗಮ -EIO;
		}
	}
#endif

	ಪ್ರತ್ಯಾಗಮ ret;
}
SPL_LOAD_IMAGE_METHOD("UART", 0, BOOT_DEVICE_UART, spl_ymodem_load_image);
