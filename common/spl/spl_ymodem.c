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

#تعریف_کرو BUF_ناپ 1024

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
	إذا (tstc())
		أرجع (getchar());
	أرجع -1;
}

ساكن ulong ymodem_read_fit(بنية spl_لدو_info *لدو, ulong وتھ,
			     ulong ناپ, فراغ *addr)
{
	صحيح res, err, buf_وتھ;
	بنية ymodem_fit_info *info = لدو->priv;
	حرف *buf = info->buf;
	ulong copy_ناپ = ناپ;

	طالما (info->image_read < وتھ) {
		res = xyz_Modem_دھارا_read(buf, BUF_ناپ, &err);
		إذا (res <= 0)
			اكسر;

		info->image_read += res;
	}

	إذا (info->image_read > وتھ) {
		res = info->image_read - وتھ;
		إذا (info->image_read % BUF_ناپ)
			buf_وتھ = (info->image_read % BUF_ناپ);
		وإلا
			buf_وتھ = BUF_ناپ;

		إذا (res > copy_ناپ) {
			حافظہ_نقل(addr, &buf[buf_وتھ - res], copy_ناپ);
			اذهب پورا;
		}
		حافظہ_نقل(addr, &buf[buf_وتھ - res], res);
		addr = addr + res;
		copy_ناپ -= res;
	}

	طالما (info->image_read < وتھ + ناپ) {
		res = xyz_Modem_دھارا_read(buf, BUF_ناپ, &err);
		إذا (res <= 0)
			اكسر;

		info->image_read += res;
		إذا (res > copy_ناپ) {
			حافظہ_نقل(addr, buf, copy_ناپ);
			اذهب پورا;
		}
		حافظہ_نقل(addr, buf, res);
		addr += res;
		copy_ناپ -= res;
	}

پورا:
	أرجع ناپ;
}

صحيح spl_ymodem_لدو_image(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev)
{
	ulong ناپ = 0;
	صحيح err;
	صحيح res;
	صحيح ret;
	connection_info_t info;
	حرف buf[BUF_ناپ];
	بنية legacy_img_hdr *ih = عدم;
	ulong addr = 0;

	info.منوال = xyzModem_ymodem;
	ret = xyz_Modem_دھارا_open(&info, &err);
	إذا (ret) {
		لکھو("spl: ymodem err - %s\n", xyzModem_error(err));
		أرجع ret;
	}

	res = xyz_Modem_دھارا_read(buf, BUF_ناپ, &err);
	إذا (res <= 0)
		اذهب سرا_دھارا;

	إذا (IS_ENABLED(CONFIG_SPL_لدو_FIT_بھریا) &&
	    image_get_magic((بنية legacy_img_hdr *)buf) == FDT_MAGIC) {
		addr = CONFIG_SYS_لدو_ADDR;
		ih = (بنية legacy_img_hdr *)addr;

		حافظہ_نقل((فراغ *)addr, buf, res);
		ناپ += res;
		addr += res;

		طالما ((res = xyz_Modem_دھارا_read(buf, BUF_ناپ, &err)) > 0) {
			حافظہ_نقل((فراغ *)addr, buf, res);
			ناپ += res;
			addr += res;
		}

		ret = spl_parse_image_سرخی(spl_image, bootdev, ih);
		إذا (ret)
			أرجع ret;
	} وإلا إذا (IS_ENABLED(CONFIG_SPL_لدو_FIT) &&
	    image_get_magic((بنية legacy_img_hdr *)buf) == FDT_MAGIC) {
		بنية spl_لدو_info لدو;
		بنية ymodem_fit_info info;

		debug("Found FIT\n");
		spl_لدو_init(&لدو, ymodem_read_fit, (فراغ *)&info, 1);
		info.buf = buf;
		info.image_read = BUF_ناپ;
		ret = spl_لدو_simple_fit(spl_image, &لدو, 0, (فراغ *)buf);
		ناپ = info.image_read;

		طالما ((res = xyz_Modem_دھارا_read(buf, BUF_ناپ, &err)) > 0)
			ناپ += res;
	} وإلا {
		ih = (بنية legacy_img_hdr *)buf;
		ret = spl_parse_image_سرخی(spl_image, bootdev, ih);
		إذا (ret)
			اذهب سرا_دھارا;
#جے_تعریف_شدہ CONFIG_SPL_GZIP
		إذا (ih->ih_comp == IH_COMP_GZIP)
			addr = CONFIG_SYS_لدو_ADDR;
		وإلا
#جے_انت
			addr = spl_image->لدو_addr;
		حافظہ_نقل((فراغ *)addr, buf, res);
		ih = (بنية legacy_img_hdr *)addr;
		ناپ += res;
		addr += res;

		طالما ((res = xyz_Modem_دھارا_read(buf, BUF_ناپ, &err)) > 0) {
			حافظہ_نقل((فراغ *)addr, buf, res);
			ناپ += res;
			addr += res;
		}
	}

سرا_دھارا:
	xyz_Modem_دھارا_close(&err);
	xyz_Modem_دھارا_terminate(false, &getcymodem);

	لکھو("Loaded %lu bytes\n", ناپ);

#جے_تعریف_شدہ CONFIG_SPL_GZIP
	إذا (!(IS_ENABLED(CONFIG_SPL_لدو_FIT) &&
	      image_get_magic((بنية legacy_img_hdr *)buf) == FDT_MAGIC) &&
	    (ih->ih_comp == IH_COMP_GZIP)) {
		إذا (gunzip((فراغ *)(spl_image->لدو_addr + حجم(*ih)),
			   CONFIG_SYS_BOOTM_LEN,
			   (فراغ *)(CONFIG_SYS_لدو_ADDR + حجم(*ih)),
			   &ناپ)) {
			puts("Uncompressing error\n");
			أرجع -EIO;
		}
	}
#جے_انت

	أرجع ret;
}
SPL_لدو_IMAGE_طریقہ("UART", 0, BOOT_DEVICE_UART, spl_ymodem_لدو_image);
