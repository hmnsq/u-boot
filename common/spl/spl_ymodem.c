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
#ਅਨ੍ਤਰ੍ਭਾਵ <gzip.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <xyzModem.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/libfdt.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUF_ਮਾਪ 1024

/*
 * Information required to load image using ymodem.
 *
 * @image_read: Now of bytes read from the image.
 * @buf: pointer to the previous read block.
 */
ਬਣਤਰ ymodem_fit_info {
	ਪੂਰਨ image_read;
	ਅੱਖਰ *buf;
};

ਸਥਿਰ ਪੂਰਨ getcymodem(ਖਾਲੀ) {
	ਜੇ (tstc())
		ਵਾਪਸ (getchar());
	ਵਾਪਸ -1;
}

ਸਥਿਰ ulong ymodem_read_fit(ਬਣਤਰ spl_ਲੱਦੋ_info *ਲੱਦੋ, ulong ਵਿੱਥ,
			     ulong ਮਾਪ, ਖਾਲੀ *addr)
{
	ਪੂਰਨ res, err, buf_ਵਿੱਥ;
	ਬਣਤਰ ymodem_fit_info *info = ਲੱਦੋ->priv;
	ਅੱਖਰ *buf = info->buf;
	ulong copy_ਮਾਪ = ਮਾਪ;

	ਜਦੋਂ (info->image_read < ਵਿੱਥ) {
		res = xyz_Modem_ਧਾਰਾ_read(buf, BUF_ਮਾਪ, &err);
		ਜੇ (res <= 0)
			ਰੋਕੋ;

		info->image_read += res;
	}

	ਜੇ (info->image_read > ਵਿੱਥ) {
		res = info->image_read - ਵਿੱਥ;
		ਜੇ (info->image_read % BUF_ਮਾਪ)
			buf_ਵਿੱਥ = (info->image_read % BUF_ਮਾਪ);
		ਨਹੀਂਤਾਂ
			buf_ਵਿੱਥ = BUF_ਮਾਪ;

		ਜੇ (res > copy_ਮਾਪ) {
			ਸਮ੍ਰਿਤੀ_ਨਕਲ(addr, &buf[buf_ਵਿੱਥ - res], copy_ਮਾਪ);
			ਜਾਓ ਪੂਰਾ;
		}
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(addr, &buf[buf_ਵਿੱਥ - res], res);
		addr = addr + res;
		copy_ਮਾਪ -= res;
	}

	ਜਦੋਂ (info->image_read < ਵਿੱਥ + ਮਾਪ) {
		res = xyz_Modem_ਧਾਰਾ_read(buf, BUF_ਮਾਪ, &err);
		ਜੇ (res <= 0)
			ਰੋਕੋ;

		info->image_read += res;
		ਜੇ (res > copy_ਮਾਪ) {
			ਸਮ੍ਰਿਤੀ_ਨਕਲ(addr, buf, copy_ਮਾਪ);
			ਜਾਓ ਪੂਰਾ;
		}
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(addr, buf, res);
		addr += res;
		copy_ਮਾਪ -= res;
	}

ਪੂਰਾ:
	ਵਾਪਸ ਮਾਪ;
}

ਪੂਰਨ spl_ymodem_ਲੱਦੋ_image(ਬਣਤਰ spl_image_info *spl_image,
			  ਬਣਤਰ spl_boot_device *bootdev)
{
	ulong ਮਾਪ = 0;
	ਪੂਰਨ err;
	ਪੂਰਨ res;
	ਪੂਰਨ ret;
	connection_info_t info;
	ਅੱਖਰ buf[BUF_ਮਾਪ];
	ਬਣਤਰ legacy_img_hdr *ih = ਨਿਰਾ;
	ulong addr = 0;

	info.ਬਹੁਲਕ = xyzModem_ymodem;
	ret = xyz_Modem_ਧਾਰਾ_open(&info, &err);
	ਜੇ (ret) {
		ਲਿਖੋ("spl: ymodem err - %s\n", xyzModem_error(err));
		ਵਾਪਸ ret;
	}

	res = xyz_Modem_ਧਾਰਾ_read(buf, BUF_ਮਾਪ, &err);
	ਜੇ (res <= 0)
		ਜਾਓ ਸਿਰਾ_ਧਾਰਾ;

	ਜੇ (IS_ENABLED(CONFIG_SPL_ਲੱਦੋ_FIT_ਭਰਿਆ) &&
	    image_get_magic((ਬਣਤਰ legacy_img_hdr *)buf) == FDT_MAGIC) {
		addr = CONFIG_SYS_ਲੱਦੋ_ADDR;
		ih = (ਬਣਤਰ legacy_img_hdr *)addr;

		ਸਮ੍ਰਿਤੀ_ਨਕਲ((ਖਾਲੀ *)addr, buf, res);
		ਮਾਪ += res;
		addr += res;

		ਜਦੋਂ ((res = xyz_Modem_ਧਾਰਾ_read(buf, BUF_ਮਾਪ, &err)) > 0) {
			ਸਮ੍ਰਿਤੀ_ਨਕਲ((ਖਾਲੀ *)addr, buf, res);
			ਮਾਪ += res;
			addr += res;
		}

		ret = spl_parse_image_ਸਿਰਲੇਖ(spl_image, bootdev, ih);
		ਜੇ (ret)
			ਵਾਪਸ ret;
	} ਨਹੀਂਤਾਂ ਜੇ (IS_ENABLED(CONFIG_SPL_ਲੱਦੋ_FIT) &&
	    image_get_magic((ਬਣਤਰ legacy_img_hdr *)buf) == FDT_MAGIC) {
		ਬਣਤਰ spl_ਲੱਦੋ_info ਲੱਦੋ;
		ਬਣਤਰ ymodem_fit_info info;

		debug("Found FIT\n");
		spl_ਲੱਦੋ_init(&ਲੱਦੋ, ymodem_read_fit, (ਖਾਲੀ *)&info, 1);
		info.buf = buf;
		info.image_read = BUF_ਮਾਪ;
		ret = spl_ਲੱਦੋ_simple_fit(spl_image, &ਲੱਦੋ, 0, (ਖਾਲੀ *)buf);
		ਮਾਪ = info.image_read;

		ਜਦੋਂ ((res = xyz_Modem_ਧਾਰਾ_read(buf, BUF_ਮਾਪ, &err)) > 0)
			ਮਾਪ += res;
	} ਨਹੀਂਤਾਂ {
		ih = (ਬਣਤਰ legacy_img_hdr *)buf;
		ret = spl_parse_image_ਸਿਰਲੇਖ(spl_image, bootdev, ih);
		ਜੇ (ret)
			ਜਾਓ ਸਿਰਾ_ਧਾਰਾ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPL_GZIP
		ਜੇ (ih->ih_comp == IH_COMP_GZIP)
			addr = CONFIG_SYS_ਲੱਦੋ_ADDR;
		ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
			addr = spl_image->ਲੱਦੋ_addr;
		ਸਮ੍ਰਿਤੀ_ਨਕਲ((ਖਾਲੀ *)addr, buf, res);
		ih = (ਬਣਤਰ legacy_img_hdr *)addr;
		ਮਾਪ += res;
		addr += res;

		ਜਦੋਂ ((res = xyz_Modem_ਧਾਰਾ_read(buf, BUF_ਮਾਪ, &err)) > 0) {
			ਸਮ੍ਰਿਤੀ_ਨਕਲ((ਖਾਲੀ *)addr, buf, res);
			ਮਾਪ += res;
			addr += res;
		}
	}

ਸਿਰਾ_ਧਾਰਾ:
	xyz_Modem_ਧਾਰਾ_close(&err);
	xyz_Modem_ਧਾਰਾ_terminate(false, &getcymodem);

	ਲਿਖੋ("Loaded %lu bytes\n", ਮਾਪ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPL_GZIP
	ਜੇ (!(IS_ENABLED(CONFIG_SPL_ਲੱਦੋ_FIT) &&
	      image_get_magic((ਬਣਤਰ legacy_img_hdr *)buf) == FDT_MAGIC) &&
	    (ih->ih_comp == IH_COMP_GZIP)) {
		ਜੇ (gunzip((ਖਾਲੀ *)(spl_image->ਲੱਦੋ_addr + ਆਕਾਰ(*ih)),
			   CONFIG_SYS_BOOTM_LEN,
			   (ਖਾਲੀ *)(CONFIG_SYS_ਲੱਦੋ_ADDR + ਆਕਾਰ(*ih)),
			   &ਮਾਪ)) {
			puts("Uncompressing error\n");
			ਵਾਪਸ -EIO;
		}
	}
#ਜੇ_ਅੰਤ

	ਵਾਪਸ ret;
}
SPL_ਲੱਦੋ_IMAGE_ਵਿਧੀ("UART", 0, BOOT_DEVICE_UART, spl_ymodem_ਲੱਦੋ_image);
