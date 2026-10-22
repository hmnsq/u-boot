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

#تعریف_کړه BUF_اندازه 1024

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

ساكن ulong ymodem_read_fit(بنية spl_بارول_info *بارول, ulong دننه_واټن,
			     ulong اندازه, فراغ *addr)
{
	صحيح res, err, buf_دننه_واټن;
	بنية ymodem_fit_info *info = بارول->priv;
	حرف *buf = info->buf;
	ulong copy_اندازه = اندازه;

	طالما (info->image_read < دننه_واټن) {
		res = xyz_Modem_بهیر_read(buf, BUF_اندازه, &err);
		إذا (res <= 0)
			اكسر;

		info->image_read += res;
	}

	إذا (info->image_read > دننه_واټن) {
		res = info->image_read - دننه_واټن;
		إذا (info->image_read % BUF_اندازه)
			buf_دننه_واټن = (info->image_read % BUF_اندازه);
		وإلا
			buf_دننه_واټن = BUF_اندازه;

		إذا (res > copy_اندازه) {
			حافظه_نقل(addr, &buf[buf_دننه_واټن - res], copy_اندازه);
			اذهب بشپړ;
		}
		حافظه_نقل(addr, &buf[buf_دننه_واټن - res], res);
		addr = addr + res;
		copy_اندازه -= res;
	}

	طالما (info->image_read < دننه_واټن + اندازه) {
		res = xyz_Modem_بهیر_read(buf, BUF_اندازه, &err);
		إذا (res <= 0)
			اكسر;

		info->image_read += res;
		إذا (res > copy_اندازه) {
			حافظه_نقل(addr, buf, copy_اندازه);
			اذهب بشپړ;
		}
		حافظه_نقل(addr, buf, res);
		addr += res;
		copy_اندازه -= res;
	}

بشپړ:
	أرجع اندازه;
}

صحيح spl_ymodem_بارول_image(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev)
{
	ulong اندازه = 0;
	صحيح err;
	صحيح res;
	صحيح ret;
	connection_info_t info;
	حرف buf[BUF_اندازه];
	بنية legacy_img_hdr *ih = عدم;
	ulong addr = 0;

	info.ډېرپېښ = xyzModem_ymodem;
	ret = xyz_Modem_بهیر_open(&info, &err);
	إذا (ret) {
		چاپ_کړه("spl: ymodem err - %s\n", xyzModem_error(err));
		أرجع ret;
	}

	res = xyz_Modem_بهیر_read(buf, BUF_اندازه, &err);
	إذا (res <= 0)
		اذهب پای_ټکی_بهیر;

	إذا (IS_ENABLED(CONFIG_SPL_بارول_FIT_ډک) &&
	    image_get_magic((بنية legacy_img_hdr *)buf) == FDT_MAGIC) {
		addr = CONFIG_SYS_بارول_ADDR;
		ih = (بنية legacy_img_hdr *)addr;

		حافظه_نقل((فراغ *)addr, buf, res);
		اندازه += res;
		addr += res;

		طالما ((res = xyz_Modem_بهیر_read(buf, BUF_اندازه, &err)) > 0) {
			حافظه_نقل((فراغ *)addr, buf, res);
			اندازه += res;
			addr += res;
		}

		ret = spl_parse_image_سرلیک(spl_image, bootdev, ih);
		إذا (ret)
			أرجع ret;
	} وإلا إذا (IS_ENABLED(CONFIG_SPL_بارول_FIT) &&
	    image_get_magic((بنية legacy_img_hdr *)buf) == FDT_MAGIC) {
		بنية spl_بارول_info بارول;
		بنية ymodem_fit_info info;

		debug("Found FIT\n");
		spl_بارول_init(&بارول, ymodem_read_fit, (فراغ *)&info, 1);
		info.buf = buf;
		info.image_read = BUF_اندازه;
		ret = spl_بارول_simple_fit(spl_image, &بارول, 0, (فراغ *)buf);
		اندازه = info.image_read;

		طالما ((res = xyz_Modem_بهیر_read(buf, BUF_اندازه, &err)) > 0)
			اندازه += res;
	} وإلا {
		ih = (بنية legacy_img_hdr *)buf;
		ret = spl_parse_image_سرلیک(spl_image, bootdev, ih);
		إذا (ret)
			اذهب پای_ټکی_بهیر;
#که_تعریف_شوی CONFIG_SPL_GZIP
		إذا (ih->ih_comp == IH_COMP_GZIP)
			addr = CONFIG_SYS_بارول_ADDR;
		وإلا
#د_که_پای
			addr = spl_image->بارول_addr;
		حافظه_نقل((فراغ *)addr, buf, res);
		ih = (بنية legacy_img_hdr *)addr;
		اندازه += res;
		addr += res;

		طالما ((res = xyz_Modem_بهیر_read(buf, BUF_اندازه, &err)) > 0) {
			حافظه_نقل((فراغ *)addr, buf, res);
			اندازه += res;
			addr += res;
		}
	}

پای_ټکی_بهیر:
	xyz_Modem_بهیر_close(&err);
	xyz_Modem_بهیر_terminate(false, &getcymodem);

	چاپ_کړه("Loaded %lu bytes\n", اندازه);

#که_تعریف_شوی CONFIG_SPL_GZIP
	إذا (!(IS_ENABLED(CONFIG_SPL_بارول_FIT) &&
	      image_get_magic((بنية legacy_img_hdr *)buf) == FDT_MAGIC) &&
	    (ih->ih_comp == IH_COMP_GZIP)) {
		إذا (gunzip((فراغ *)(spl_image->بارول_addr + حجم(*ih)),
			   CONFIG_SYS_BOOTM_LEN,
			   (فراغ *)(CONFIG_SYS_بارول_ADDR + حجم(*ih)),
			   &اندازه)) {
			puts("Uncompressing error\n");
			أرجع -EIO;
		}
	}
#د_که_پای

	أرجع ret;
}
SPL_بارول_IMAGE_لاره("UART", 0, BOOT_DEVICE_UART, spl_ymodem_بارول_image);
