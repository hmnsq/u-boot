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

#परिभाषय BUF_आकार 1024

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
	यदि (tstc())
		प्रत्यागम (अक्षरं_पठ());
	प्रत्यागम -1;
}

स्थिर ulong ymodem_read_fit(संरचना spl_आरोपय_info *आरोपय, ulong विस्थापनम्,
			     ulong आकार, शून्य *addr)
{
	पूर्णाङ्क res, err, buf_विस्थापनम्;
	संरचना ymodem_fit_info *info = आरोपय->priv;
	वर्ण *buf = info->buf;
	ulong copy_आकार = आकार;

	यावत् (info->image_read < विस्थापनम्) {
		res = xyz_Modem_प्रवाहः_read(buf, BUF_आकार, &err);
		यदि (res <= 0)
			भङ्ग;

		info->image_read += res;
	}

	यदि (info->image_read > विस्थापनम्) {
		res = info->image_read - विस्थापनम्;
		यदि (info->image_read % BUF_आकार)
			buf_विस्थापनम् = (info->image_read % BUF_आकार);
		अन्यथा
			buf_विस्थापनम् = BUF_आकार;

		यदि (res > copy_आकार) {
			स्मृतिं_प्रतिलिख(addr, &buf[buf_विस्थापनम् - res], copy_आकार);
			गच्छ समाप्तम्;
		}
		स्मृतिं_प्रतिलिख(addr, &buf[buf_विस्थापनम् - res], res);
		addr = addr + res;
		copy_आकार -= res;
	}

	यावत् (info->image_read < विस्थापनम् + आकार) {
		res = xyz_Modem_प्रवाहः_read(buf, BUF_आकार, &err);
		यदि (res <= 0)
			भङ्ग;

		info->image_read += res;
		यदि (res > copy_आकार) {
			स्मृतिं_प्रतिलिख(addr, buf, copy_आकार);
			गच्छ समाप्तम्;
		}
		स्मृतिं_प्रतिलिख(addr, buf, res);
		addr += res;
		copy_आकार -= res;
	}

समाप्तम्:
	प्रत्यागम आकार;
}

पूर्णाङ्क spl_ymodem_आरोपय_image(संरचना spl_image_info *spl_image,
			  संरचना spl_boot_device *bootdev)
{
	ulong आकार = 0;
	पूर्णाङ्क err;
	पूर्णाङ्क res;
	पूर्णाङ्क ret;
	connection_info_t info;
	वर्ण buf[BUF_आकार];
	संरचना legacy_img_hdr *ih = रिक्त;
	ulong addr = 0;

	info.बहुलकम् = xyzModem_ymodem;
	ret = xyz_Modem_प्रवाहः_open(&info, &err);
	यदि (ret) {
		प्रारूप_लिख("spl: ymodem err - %s\n", xyzModem_error(err));
		प्रत्यागम ret;
	}

	res = xyz_Modem_प्रवाहः_read(buf, BUF_आकार, &err);
	यदि (res <= 0)
		गच्छ अन्तः_प्रवाहः;

	यदि (IS_ENABLED(CONFIG_SPL_आरोपय_FIT_पूर्णम्) &&
	    image_get_magic((संरचना legacy_img_hdr *)buf) == FDT_MAGIC) {
		addr = CONFIG_SYS_आरोपय_ADDR;
		ih = (संरचना legacy_img_hdr *)addr;

		स्मृतिं_प्रतिलिख((शून्य *)addr, buf, res);
		आकार += res;
		addr += res;

		यावत् ((res = xyz_Modem_प्रवाहः_read(buf, BUF_आकार, &err)) > 0) {
			स्मृतिं_प्रतिलिख((शून्य *)addr, buf, res);
			आकार += res;
			addr += res;
		}

		ret = spl_parse_image_शीर्षकम्(spl_image, bootdev, ih);
		यदि (ret)
			प्रत्यागम ret;
	} अन्यथा यदि (IS_ENABLED(CONFIG_SPL_आरोपय_FIT) &&
	    image_get_magic((संरचना legacy_img_hdr *)buf) == FDT_MAGIC) {
		संरचना spl_आरोपय_info आरोपय;
		संरचना ymodem_fit_info info;

		debug("Found FIT\n");
		spl_आरोपय_init(&आरोपय, ymodem_read_fit, (शून्य *)&info, 1);
		info.buf = buf;
		info.image_read = BUF_आकार;
		ret = spl_आरोपय_simple_fit(spl_image, &आरोपय, 0, (शून्य *)buf);
		आकार = info.image_read;

		यावत् ((res = xyz_Modem_प्रवाहः_read(buf, BUF_आकार, &err)) > 0)
			आकार += res;
	} अन्यथा {
		ih = (संरचना legacy_img_hdr *)buf;
		ret = spl_parse_image_शीर्षकम्(spl_image, bootdev, ih);
		यदि (ret)
			गच्छ अन्तः_प्रवाहः;
#यदि_परिभाषितम् CONFIG_SPL_GZIP
		यदि (ih->ih_comp == IH_COMP_GZIP)
			addr = CONFIG_SYS_आरोपय_ADDR;
		अन्यथा
#यदि_अन्तः
			addr = spl_image->आरोपय_addr;
		स्मृतिं_प्रतिलिख((शून्य *)addr, buf, res);
		ih = (संरचना legacy_img_hdr *)addr;
		आकार += res;
		addr += res;

		यावत् ((res = xyz_Modem_प्रवाहः_read(buf, BUF_आकार, &err)) > 0) {
			स्मृतिं_प्रतिलिख((शून्य *)addr, buf, res);
			आकार += res;
			addr += res;
		}
	}

अन्तः_प्रवाहः:
	xyz_Modem_प्रवाहः_close(&err);
	xyz_Modem_प्रवाहः_terminate(false, &getcymodem);

	प्रारूप_लिख("Loaded %lu bytes\n", आकार);

#यदि_परिभाषितम् CONFIG_SPL_GZIP
	यदि (!(IS_ENABLED(CONFIG_SPL_आरोपय_FIT) &&
	      image_get_magic((संरचना legacy_img_hdr *)buf) == FDT_MAGIC) &&
	    (ih->ih_comp == IH_COMP_GZIP)) {
		यदि (gunzip((शून्य *)(spl_image->आरोपय_addr + परिमाण(*ih)),
			   CONFIG_SYS_BOOTM_LEN,
			   (शून्य *)(CONFIG_SYS_आरोपय_ADDR + परिमाण(*ih)),
			   &आकार)) {
			पङ्क्तिं_लिख("Uncompressing error\n");
			प्रत्यागम -EIO;
		}
	}
#यदि_अन्तः

	प्रत्यागम ret;
}
SPL_आरोपय_IMAGE_विधानम्("UART", 0, BOOT_DEVICE_UART, spl_ymodem_आरोपय_image);
