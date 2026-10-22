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

#تعریف_کن BUF_اندازه 1024

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

ساكن ulong ymodem_read_fit(بنية spl_بارگیری_info *بارگیری, ulong جابه_جایی,
			     ulong اندازه, فراغ *addr)
{
	صحيح res, err, buf_جابه_جایی;
	بنية ymodem_fit_info *info = بارگیری->priv;
	حرف *buf = info->buf;
	ulong copy_اندازه = اندازه;

	طالما (info->image_read < جابه_جایی) {
		res = xyz_Modem_جریان_read(buf, BUF_اندازه, &err);
		إذا (res <= 0)
			اكسر;

		info->image_read += res;
	}

	إذا (info->image_read > جابه_جایی) {
		res = info->image_read - جابه_جایی;
		إذا (info->image_read % BUF_اندازه)
			buf_جابه_جایی = (info->image_read % BUF_اندازه);
		وإلا
			buf_جابه_جایی = BUF_اندازه;

		إذا (res > copy_اندازه) {
			کپی_حافظه(addr, &buf[buf_جابه_جایی - res], copy_اندازه);
			اذهب انجامشده;
		}
		کپی_حافظه(addr, &buf[buf_جابه_جایی - res], res);
		addr = addr + res;
		copy_اندازه -= res;
	}

	طالما (info->image_read < جابه_جایی + اندازه) {
		res = xyz_Modem_جریان_read(buf, BUF_اندازه, &err);
		إذا (res <= 0)
			اكسر;

		info->image_read += res;
		إذا (res > copy_اندازه) {
			کپی_حافظه(addr, buf, copy_اندازه);
			اذهب انجامشده;
		}
		کپی_حافظه(addr, buf, res);
		addr += res;
		copy_اندازه -= res;
	}

انجامشده:
	أرجع اندازه;
}

صحيح spl_ymodem_بارگیری_image(بنية spl_image_info *spl_image,
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

	info.نما = xyzModem_ymodem;
	ret = xyz_Modem_جریان_open(&info, &err);
	إذا (ret) {
		چاپ_کن("spl: ymodem err - %s\n", xyzModem_error(err));
		أرجع ret;
	}

	res = xyz_Modem_جریان_read(buf, BUF_اندازه, &err);
	إذا (res <= 0)
		اذهب انتها_جریان;

	إذا (IS_ENABLED(CONFIG_SPL_بارگیری_FIT_پر) &&
	    image_get_magic((بنية legacy_img_hdr *)buf) == FDT_MAGIC) {
		addr = CONFIG_SYS_بارگیری_ADDR;
		ih = (بنية legacy_img_hdr *)addr;

		کپی_حافظه((فراغ *)addr, buf, res);
		اندازه += res;
		addr += res;

		طالما ((res = xyz_Modem_جریان_read(buf, BUF_اندازه, &err)) > 0) {
			کپی_حافظه((فراغ *)addr, buf, res);
			اندازه += res;
			addr += res;
		}

		ret = spl_parse_image_سرآیند(spl_image, bootdev, ih);
		إذا (ret)
			أرجع ret;
	} وإلا إذا (IS_ENABLED(CONFIG_SPL_بارگیری_FIT) &&
	    image_get_magic((بنية legacy_img_hdr *)buf) == FDT_MAGIC) {
		بنية spl_بارگیری_info بارگیری;
		بنية ymodem_fit_info info;

		debug("Found FIT\n");
		spl_بارگیری_init(&بارگیری, ymodem_read_fit, (فراغ *)&info, 1);
		info.buf = buf;
		info.image_read = BUF_اندازه;
		ret = spl_بارگیری_simple_fit(spl_image, &بارگیری, 0, (فراغ *)buf);
		اندازه = info.image_read;

		طالما ((res = xyz_Modem_جریان_read(buf, BUF_اندازه, &err)) > 0)
			اندازه += res;
	} وإلا {
		ih = (بنية legacy_img_hdr *)buf;
		ret = spl_parse_image_سرآیند(spl_image, bootdev, ih);
		إذا (ret)
			اذهب انتها_جریان;
#اگر_تعریف_شده CONFIG_SPL_GZIP
		إذا (ih->ih_comp == IH_COMP_GZIP)
			addr = CONFIG_SYS_بارگیری_ADDR;
		وإلا
#پایان_اگر
			addr = spl_image->بارگیری_addr;
		کپی_حافظه((فراغ *)addr, buf, res);
		ih = (بنية legacy_img_hdr *)addr;
		اندازه += res;
		addr += res;

		طالما ((res = xyz_Modem_جریان_read(buf, BUF_اندازه, &err)) > 0) {
			کپی_حافظه((فراغ *)addr, buf, res);
			اندازه += res;
			addr += res;
		}
	}

انتها_جریان:
	xyz_Modem_جریان_close(&err);
	xyz_Modem_جریان_terminate(false, &getcymodem);

	چاپ_کن("Loaded %lu bytes\n", اندازه);

#اگر_تعریف_شده CONFIG_SPL_GZIP
	إذا (!(IS_ENABLED(CONFIG_SPL_بارگیری_FIT) &&
	      image_get_magic((بنية legacy_img_hdr *)buf) == FDT_MAGIC) &&
	    (ih->ih_comp == IH_COMP_GZIP)) {
		إذا (gunzip((فراغ *)(spl_image->بارگیری_addr + حجم(*ih)),
			   CONFIG_SYS_BOOTM_LEN,
			   (فراغ *)(CONFIG_SYS_بارگیری_ADDR + حجم(*ih)),
			   &اندازه)) {
			puts("Uncompressing error\n");
			أرجع -EIO;
		}
	}
#پایان_اگر

	أرجع ret;
}
SPL_بارگیری_IMAGE_روش("UART", 0, BOOT_DEVICE_UART, spl_ymodem_بارگیری_image);
