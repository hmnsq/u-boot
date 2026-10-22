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

#وصف_ڪريو BUF_سائيز 1024

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

ساكن ulong ymodem_read_fit(بنية spl_ڀريو_info *ڀريو, ulong وٿي,
			     ulong سائيز, فراغ *addr)
{
	صحيح res, err, buf_وٿي;
	بنية ymodem_fit_info *info = ڀريو->priv;
	حرف *buf = info->buf;
	ulong copy_سائيز = سائيز;

	طالما (info->image_read < وٿي) {
		res = xyz_Modem_وهڪرو_read(buf, BUF_سائيز, &err);
		إذا (res <= 0)
			اكسر;

		info->image_read += res;
	}

	إذا (info->image_read > وٿي) {
		res = info->image_read - وٿي;
		إذا (info->image_read % BUF_سائيز)
			buf_وٿي = (info->image_read % BUF_سائيز);
		وإلا
			buf_وٿي = BUF_سائيز;

		إذا (res > copy_سائيز) {
			يادگيري_نقل(addr, &buf[buf_وٿي - res], copy_سائيز);
			اذهب پورو;
		}
		يادگيري_نقل(addr, &buf[buf_وٿي - res], res);
		addr = addr + res;
		copy_سائيز -= res;
	}

	طالما (info->image_read < وٿي + سائيز) {
		res = xyz_Modem_وهڪرو_read(buf, BUF_سائيز, &err);
		إذا (res <= 0)
			اكسر;

		info->image_read += res;
		إذا (res > copy_سائيز) {
			يادگيري_نقل(addr, buf, copy_سائيز);
			اذهب پورو;
		}
		يادگيري_نقل(addr, buf, res);
		addr += res;
		copy_سائيز -= res;
	}

پورو:
	أرجع سائيز;
}

صحيح spl_ymodem_ڀريو_image(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev)
{
	ulong سائيز = 0;
	صحيح err;
	صحيح res;
	صحيح ret;
	connection_info_t info;
	حرف buf[BUF_سائيز];
	بنية legacy_img_hdr *ih = عدم;
	ulong addr = 0;

	info.گهڻائي = xyzModem_ymodem;
	ret = xyz_Modem_وهڪرو_open(&info, &err);
	إذا (ret) {
		لکو("spl: ymodem err - %s\n", xyzModem_error(err));
		أرجع ret;
	}

	res = xyz_Modem_وهڪرو_read(buf, BUF_سائيز, &err);
	إذا (res <= 0)
		اذهب پڇاڙي_وهڪرو;

	إذا (IS_ENABLED(CONFIG_SPL_ڀريو_FIT_ڀريل) &&
	    image_get_magic((بنية legacy_img_hdr *)buf) == FDT_MAGIC) {
		addr = CONFIG_SYS_ڀريو_ADDR;
		ih = (بنية legacy_img_hdr *)addr;

		يادگيري_نقل((فراغ *)addr, buf, res);
		سائيز += res;
		addr += res;

		طالما ((res = xyz_Modem_وهڪرو_read(buf, BUF_سائيز, &err)) > 0) {
			يادگيري_نقل((فراغ *)addr, buf, res);
			سائيز += res;
			addr += res;
		}

		ret = spl_parse_image_سرو(spl_image, bootdev, ih);
		إذا (ret)
			أرجع ret;
	} وإلا إذا (IS_ENABLED(CONFIG_SPL_ڀريو_FIT) &&
	    image_get_magic((بنية legacy_img_hdr *)buf) == FDT_MAGIC) {
		بنية spl_ڀريو_info ڀريو;
		بنية ymodem_fit_info info;

		debug("Found FIT\n");
		spl_ڀريو_init(&ڀريو, ymodem_read_fit, (فراغ *)&info, 1);
		info.buf = buf;
		info.image_read = BUF_سائيز;
		ret = spl_ڀريو_simple_fit(spl_image, &ڀريو, 0, (فراغ *)buf);
		سائيز = info.image_read;

		طالما ((res = xyz_Modem_وهڪرو_read(buf, BUF_سائيز, &err)) > 0)
			سائيز += res;
	} وإلا {
		ih = (بنية legacy_img_hdr *)buf;
		ret = spl_parse_image_سرو(spl_image, bootdev, ih);
		إذا (ret)
			اذهب پڇاڙي_وهڪرو;
#جيڪڏهن_وصف_ٿيل CONFIG_SPL_GZIP
		إذا (ih->ih_comp == IH_COMP_GZIP)
			addr = CONFIG_SYS_ڀريو_ADDR;
		وإلا
#جيڪڏهن_پورو
			addr = spl_image->ڀريو_addr;
		يادگيري_نقل((فراغ *)addr, buf, res);
		ih = (بنية legacy_img_hdr *)addr;
		سائيز += res;
		addr += res;

		طالما ((res = xyz_Modem_وهڪرو_read(buf, BUF_سائيز, &err)) > 0) {
			يادگيري_نقل((فراغ *)addr, buf, res);
			سائيز += res;
			addr += res;
		}
	}

پڇاڙي_وهڪرو:
	xyz_Modem_وهڪرو_close(&err);
	xyz_Modem_وهڪرو_terminate(false, &getcymodem);

	لکو("Loaded %lu bytes\n", سائيز);

#جيڪڏهن_وصف_ٿيل CONFIG_SPL_GZIP
	إذا (!(IS_ENABLED(CONFIG_SPL_ڀريو_FIT) &&
	      image_get_magic((بنية legacy_img_hdr *)buf) == FDT_MAGIC) &&
	    (ih->ih_comp == IH_COMP_GZIP)) {
		إذا (gunzip((فراغ *)(spl_image->ڀريو_addr + حجم(*ih)),
			   CONFIG_SYS_BOOTM_LEN,
			   (فراغ *)(CONFIG_SYS_ڀريو_ADDR + حجم(*ih)),
			   &سائيز)) {
			puts("Uncompressing error\n");
			أرجع -EIO;
		}
	}
#جيڪڏهن_پورو

	أرجع ret;
}
SPL_ڀريو_IMAGE_طريقو("UART", 0, BOOT_DEVICE_UART, spl_ymodem_ڀريو_image);
