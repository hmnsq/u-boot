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
#شامل <gzip.h>
#شامل <image.h>
#شامل <log.h>
#شامل <spl.h>
#شامل <xyzModem.h>
#شامل <linux/libfdt.h>

#تعریف_کرو BUF_سائز 1024

/*
 * Information required to load image using ymodem.
 *
 * @image_read: Now of bytes read from the image.
 * @buf: pointer to the previous read block.
 */
ساخت ymodem_fit_info {
	صحیح image_read;
	حرف *buf;
};

ساکن صحیح getcymodem(خالی) {
	اگر (tstc())
		واپس (getchar());
	واپس -1;
}

ساکن ulong ymodem_read_fit(ساخت spl_بھرو_info *بھرو, ulong انحراف,
			     ulong سائز, خالی *addr)
{
	صحیح res, err, buf_انحراف;
	ساخت ymodem_fit_info *info = بھرو->priv;
	حرف *buf = info->buf;
	ulong copy_سائز = سائز;

	جب (info->image_read < انحراف) {
		res = xyz_Modem_دھارا_read(buf, BUF_سائز, &err);
		اگر (res <= 0)
			توڑو;

		info->image_read += res;
	}

	اگر (info->image_read > انحراف) {
		res = info->image_read - انحراف;
		اگر (info->image_read % BUF_سائز)
			buf_انحراف = (info->image_read % BUF_سائز);
		ورنہ
			buf_انحراف = BUF_سائز;

		اگر (res > copy_سائز) {
			حافظہ_نقل(addr, &buf[buf_انحراف - res], copy_سائز);
			جاؤ مکمل;
		}
		حافظہ_نقل(addr, &buf[buf_انحراف - res], res);
		addr = addr + res;
		copy_سائز -= res;
	}

	جب (info->image_read < انحراف + سائز) {
		res = xyz_Modem_دھارا_read(buf, BUF_سائز, &err);
		اگر (res <= 0)
			توڑو;

		info->image_read += res;
		اگر (res > copy_سائز) {
			حافظہ_نقل(addr, buf, copy_سائز);
			جاؤ مکمل;
		}
		حافظہ_نقل(addr, buf, res);
		addr += res;
		copy_سائز -= res;
	}

مکمل:
	واپس سائز;
}

صحیح spl_ymodem_بھرو_image(ساخت spl_image_info *spl_image,
			  ساخت spl_boot_device *bootdev)
{
	ulong سائز = 0;
	صحیح err;
	صحیح res;
	صحیح ret;
	connection_info_t info;
	حرف buf[BUF_سائز];
	ساخت legacy_img_hdr *ih = تہی;
	ulong addr = 0;

	info.منوال = xyzModem_ymodem;
	ret = xyz_Modem_دھارا_open(&info, &err);
	اگر (ret) {
		لکھو("spl: ymodem err - %s\n", xyzModem_error(err));
		واپس ret;
	}

	res = xyz_Modem_دھارا_read(buf, BUF_سائز, &err);
	اگر (res <= 0)
		جاؤ انتہا_دھارا;

	اگر (IS_ENABLED(CONFIG_SPL_بھرو_FIT_بھرا) &&
	    image_get_magic((ساخت legacy_img_hdr *)buf) == FDT_MAGIC) {
		addr = CONFIG_SYS_بھرو_ADDR;
		ih = (ساخت legacy_img_hdr *)addr;

		حافظہ_نقل((خالی *)addr, buf, res);
		سائز += res;
		addr += res;

		جب ((res = xyz_Modem_دھارا_read(buf, BUF_سائز, &err)) > 0) {
			حافظہ_نقل((خالی *)addr, buf, res);
			سائز += res;
			addr += res;
		}

		ret = spl_parse_image_سرخی(spl_image, bootdev, ih);
		اگر (ret)
			واپس ret;
	} ورنہ اگر (IS_ENABLED(CONFIG_SPL_بھرو_FIT) &&
	    image_get_magic((ساخت legacy_img_hdr *)buf) == FDT_MAGIC) {
		ساخت spl_بھرو_info بھرو;
		ساخت ymodem_fit_info info;

		debug("Found FIT\n");
		spl_بھرو_init(&بھرو, ymodem_read_fit, (خالی *)&info, 1);
		info.buf = buf;
		info.image_read = BUF_سائز;
		ret = spl_بھرو_simple_fit(spl_image, &بھرو, 0, (خالی *)buf);
		سائز = info.image_read;

		جب ((res = xyz_Modem_دھارا_read(buf, BUF_سائز, &err)) > 0)
			سائز += res;
	} ورنہ {
		ih = (ساخت legacy_img_hdr *)buf;
		ret = spl_parse_image_سرخی(spl_image, bootdev, ih);
		اگر (ret)
			جاؤ انتہا_دھارا;
#اگر_تعریف_شدہ CONFIG_SPL_GZIP
		اگر (ih->ih_comp == IH_COMP_GZIP)
			addr = CONFIG_SYS_بھرو_ADDR;
		ورنہ
#اگر_ختم
			addr = spl_image->بھرو_addr;
		حافظہ_نقل((خالی *)addr, buf, res);
		ih = (ساخت legacy_img_hdr *)addr;
		سائز += res;
		addr += res;

		جب ((res = xyz_Modem_دھارا_read(buf, BUF_سائز, &err)) > 0) {
			حافظہ_نقل((خالی *)addr, buf, res);
			سائز += res;
			addr += res;
		}
	}

انتہا_دھارا:
	xyz_Modem_دھارا_close(&err);
	xyz_Modem_دھارا_terminate(false, &getcymodem);

	لکھو("Loaded %lu bytes\n", سائز);

#اگر_تعریف_شدہ CONFIG_SPL_GZIP
	اگر (!(IS_ENABLED(CONFIG_SPL_بھرو_FIT) &&
	      image_get_magic((ساخت legacy_img_hdr *)buf) == FDT_MAGIC) &&
	    (ih->ih_comp == IH_COMP_GZIP)) {
		اگر (gunzip((خالی *)(spl_image->بھرو_addr + حجم(*ih)),
			   CONFIG_SYS_BOOTM_LEN,
			   (خالی *)(CONFIG_SYS_بھرو_ADDR + حجم(*ih)),
			   &سائز)) {
			puts("Uncompressing error\n");
			واپس -EIO;
		}
	}
#اگر_ختم

	واپس ret;
}
SPL_بھرو_IMAGE_طریقہ("UART", 0, BOOT_DEVICE_UART, spl_ymodem_بھرو_image);
