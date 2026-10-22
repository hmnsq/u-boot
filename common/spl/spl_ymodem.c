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
#అన్తర్భావ <gzip.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <spl.h>
#అన్తర్భావ <xyzModem.h>
#అన్తర్భావ <linux/libfdt.h>

#నిర్వచించు BUF_కొలత 1024

/*
 * Information required to load image using ymodem.
 *
 * @image_read: Now of bytes read from the image.
 * @buf: pointer to the previous read block.
 */
నిర్మాణం ymodem_fit_info {
	పూర్ణాంకం image_read;
	అక్షరం *buf;
};

స్థిర పూర్ణాంకం getcymodem(శూన్య) {
	ఒకవేళ (tstc())
		తిరిగి (getchar());
	తిరిగి -1;
}

స్థిర ulong ymodem_read_fit(నిర్మాణం spl_ఎక్కించు_info *ఎక్కించు, ulong స్థానభ్రంశం,
			     ulong కొలత, శూన్య *addr)
{
	పూర్ణాంకం res, err, buf_స్థానభ్రంశం;
	నిర్మాణం ymodem_fit_info *info = ఎక్కించు->priv;
	అక్షరం *buf = info->buf;
	ulong copy_కొలత = కొలత;

	అయితే (info->image_read < స్థానభ్రంశం) {
		res = xyz_Modem_ప్రవాహం_read(buf, BUF_కొలత, &err);
		ఒకవేళ (res <= 0)
			ఆపు;

		info->image_read += res;
	}

	ఒకవేళ (info->image_read > స్థానభ్రంశం) {
		res = info->image_read - స్థానభ్రంశం;
		ఒకవేళ (info->image_read % BUF_కొలత)
			buf_స్థానభ్రంశం = (info->image_read % BUF_కొలత);
		లేకపోతే
			buf_స్థానభ్రంశం = BUF_కొలత;

		ఒకవేళ (res > copy_కొలత) {
			జ్ఞాపకం_నకలు(addr, &buf[buf_స్థానభ్రంశం - res], copy_కొలత);
			వెళ్ళు పూర్తి;
		}
		జ్ఞాపకం_నకలు(addr, &buf[buf_స్థానభ్రంశం - res], res);
		addr = addr + res;
		copy_కొలత -= res;
	}

	అయితే (info->image_read < స్థానభ్రంశం + కొలత) {
		res = xyz_Modem_ప్రవాహం_read(buf, BUF_కొలత, &err);
		ఒకవేళ (res <= 0)
			ఆపు;

		info->image_read += res;
		ఒకవేళ (res > copy_కొలత) {
			జ్ఞాపకం_నకలు(addr, buf, copy_కొలత);
			వెళ్ళు పూర్తి;
		}
		జ్ఞాపకం_నకలు(addr, buf, res);
		addr += res;
		copy_కొలత -= res;
	}

పూర్తి:
	తిరిగి కొలత;
}

పూర్ణాంకం spl_ymodem_ఎక్కించు_image(నిర్మాణం spl_image_info *spl_image,
			  నిర్మాణం spl_boot_device *bootdev)
{
	ulong కొలత = 0;
	పూర్ణాంకం err;
	పూర్ణాంకం res;
	పూర్ణాంకం ret;
	connection_info_t info;
	అక్షరం buf[BUF_కొలత];
	నిర్మాణం legacy_img_hdr *ih = ఖాళీ;
	ulong addr = 0;

	info.బహుళకం = xyzModem_ymodem;
	ret = xyz_Modem_ప్రవాహం_open(&info, &err);
	ఒకవేళ (ret) {
		రాయి("spl: ymodem err - %s\n", xyzModem_error(err));
		తిరిగి ret;
	}

	res = xyz_Modem_ప్రవాహం_read(buf, BUF_కొలత, &err);
	ఒకవేళ (res <= 0)
		వెళ్ళు చివర_ప్రవాహం;

	ఒకవేళ (IS_ENABLED(CONFIG_SPL_ఎక్కించు_FIT_నిండిన) &&
	    image_get_magic((నిర్మాణం legacy_img_hdr *)buf) == FDT_MAGIC) {
		addr = CONFIG_SYS_ఎక్కించు_ADDR;
		ih = (నిర్మాణం legacy_img_hdr *)addr;

		జ్ఞాపకం_నకలు((శూన్య *)addr, buf, res);
		కొలత += res;
		addr += res;

		అయితే ((res = xyz_Modem_ప్రవాహం_read(buf, BUF_కొలత, &err)) > 0) {
			జ్ఞాపకం_నకలు((శూన్య *)addr, buf, res);
			కొలత += res;
			addr += res;
		}

		ret = spl_parse_image_శీర్షిక(spl_image, bootdev, ih);
		ఒకవేళ (ret)
			తిరిగి ret;
	} లేకపోతే ఒకవేళ (IS_ENABLED(CONFIG_SPL_ఎక్కించు_FIT) &&
	    image_get_magic((నిర్మాణం legacy_img_hdr *)buf) == FDT_MAGIC) {
		నిర్మాణం spl_ఎక్కించు_info ఎక్కించు;
		నిర్మాణం ymodem_fit_info info;

		debug("Found FIT\n");
		spl_ఎక్కించు_init(&ఎక్కించు, ymodem_read_fit, (శూన్య *)&info, 1);
		info.buf = buf;
		info.image_read = BUF_కొలత;
		ret = spl_ఎక్కించు_simple_fit(spl_image, &ఎక్కించు, 0, (శూన్య *)buf);
		కొలత = info.image_read;

		అయితే ((res = xyz_Modem_ప్రవాహం_read(buf, BUF_కొలత, &err)) > 0)
			కొలత += res;
	} లేకపోతే {
		ih = (నిర్మాణం legacy_img_hdr *)buf;
		ret = spl_parse_image_శీర్షిక(spl_image, bootdev, ih);
		ఒకవేళ (ret)
			వెళ్ళు చివర_ప్రవాహం;
#నిర్వచితమైతే CONFIG_SPL_GZIP
		ఒకవేళ (ih->ih_comp == IH_COMP_GZIP)
			addr = CONFIG_SYS_ఎక్కించు_ADDR;
		లేకపోతే
#అయితే_ముగింపు
			addr = spl_image->ఎక్కించు_addr;
		జ్ఞాపకం_నకలు((శూన్య *)addr, buf, res);
		ih = (నిర్మాణం legacy_img_hdr *)addr;
		కొలత += res;
		addr += res;

		అయితే ((res = xyz_Modem_ప్రవాహం_read(buf, BUF_కొలత, &err)) > 0) {
			జ్ఞాపకం_నకలు((శూన్య *)addr, buf, res);
			కొలత += res;
			addr += res;
		}
	}

చివర_ప్రవాహం:
	xyz_Modem_ప్రవాహం_close(&err);
	xyz_Modem_ప్రవాహం_terminate(false, &getcymodem);

	రాయి("Loaded %lu bytes\n", కొలత);

#నిర్వచితమైతే CONFIG_SPL_GZIP
	ఒకవేళ (!(IS_ENABLED(CONFIG_SPL_ఎక్కించు_FIT) &&
	      image_get_magic((నిర్మాణం legacy_img_hdr *)buf) == FDT_MAGIC) &&
	    (ih->ih_comp == IH_COMP_GZIP)) {
		ఒకవేళ (gunzip((శూన్య *)(spl_image->ఎక్కించు_addr + పరిమాణం(*ih)),
			   CONFIG_SYS_BOOTM_LEN,
			   (శూన్య *)(CONFIG_SYS_ఎక్కించు_ADDR + పరిమాణం(*ih)),
			   &కొలత)) {
			puts("Uncompressing error\n");
			తిరిగి -EIO;
		}
	}
#అయితే_ముగింపు

	తిరిగి ret;
}
SPL_ఎక్కించు_IMAGE_పద్ధతి("UART", 0, BOOT_DEVICE_UART, spl_ymodem_ఎక్కించు_image);
