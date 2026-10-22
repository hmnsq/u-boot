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
#समावेश <gzip.h>
#समावेश <image.h>
#समावेश <log.h>
#समावेश <spl.h>
#समावेश <xyzModem.h>
#समावेश <linux/libfdt.h>

#घोषणा BUF_आकार 1024

/*
 * Information required to load image using ymodem.
 *
 * @image_read: Now of bytes read from the image.
 * @buf: pointer to the previous read block.
 */
काष्ठा ymodem_fit_info {
	पूर्णांक image_read;
	अक्षर *buf;
};

अटल पूर्णांक getcymodem(व्योम) {
	अगर (tstc())
		वापस (अक्षर_लो());
	वापस -1;
}

अटल ulong ymodem_read_fit(काष्ठा spl_load_info *load, ulong offset,
			     ulong आकार, व्योम *addr)
{
	पूर्णांक res, err, buf_offset;
	काष्ठा ymodem_fit_info *info = load->priv;
	अक्षर *buf = info->buf;
	ulong copy_आकार = आकार;

	जबतक (info->image_read < offset) {
		res = xyzModem_stream_read(buf, BUF_आकार, &err);
		अगर (res <= 0)
			अवरोध;

		info->image_read += res;
	}

	अगर (info->image_read > offset) {
		res = info->image_read - offset;
		अगर (info->image_read % BUF_आकार)
			buf_offset = (info->image_read % BUF_आकार);
		अन्यथा
			buf_offset = BUF_आकार;

		अगर (res > copy_आकार) {
			स_नकल(addr, &buf[buf_offset - res], copy_आकार);
			जाओ done;
		}
		स_नकल(addr, &buf[buf_offset - res], res);
		addr = addr + res;
		copy_आकार -= res;
	}

	जबतक (info->image_read < offset + आकार) {
		res = xyzModem_stream_read(buf, BUF_आकार, &err);
		अगर (res <= 0)
			अवरोध;

		info->image_read += res;
		अगर (res > copy_आकार) {
			स_नकल(addr, buf, copy_आकार);
			जाओ done;
		}
		स_नकल(addr, buf, res);
		addr += res;
		copy_आकार -= res;
	}

done:
	वापस आकार;
}

पूर्णांक spl_ymodem_load_image(काष्ठा spl_image_info *spl_image,
			  काष्ठा spl_boot_device *bootdev)
{
	ulong आकार = 0;
	पूर्णांक err;
	पूर्णांक res;
	पूर्णांक ret;
	connection_info_t info;
	अक्षर buf[BUF_आकार];
	काष्ठा legacy_img_hdr *ih = शून्य;
	ulong addr = 0;

	info.mode = xyzModem_ymodem;
	ret = xyzModem_stream_open(&info, &err);
	अगर (ret) {
		म_लिखो("spl: ymodem err - %s\n", xyzModem_error(err));
		वापस ret;
	}

	res = xyzModem_stream_read(buf, BUF_आकार, &err);
	अगर (res <= 0)
		जाओ end_stream;

	अगर (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL) &&
	    image_get_magic((काष्ठा legacy_img_hdr *)buf) == FDT_MAGIC) {
		addr = CONFIG_SYS_LOAD_ADDR;
		ih = (काष्ठा legacy_img_hdr *)addr;

		स_नकल((व्योम *)addr, buf, res);
		आकार += res;
		addr += res;

		जबतक ((res = xyzModem_stream_read(buf, BUF_आकार, &err)) > 0) {
			स_नकल((व्योम *)addr, buf, res);
			आकार += res;
			addr += res;
		}

		ret = spl_parse_image_header(spl_image, bootdev, ih);
		अगर (ret)
			वापस ret;
	} अन्यथा अगर (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	    image_get_magic((काष्ठा legacy_img_hdr *)buf) == FDT_MAGIC) {
		काष्ठा spl_load_info load;
		काष्ठा ymodem_fit_info info;

		debug("Found FIT\n");
		spl_load_init(&load, ymodem_read_fit, (व्योम *)&info, 1);
		info.buf = buf;
		info.image_read = BUF_आकार;
		ret = spl_load_simple_fit(spl_image, &load, 0, (व्योम *)buf);
		आकार = info.image_read;

		जबतक ((res = xyzModem_stream_read(buf, BUF_आकार, &err)) > 0)
			आकार += res;
	} अन्यथा {
		ih = (काष्ठा legacy_img_hdr *)buf;
		ret = spl_parse_image_header(spl_image, bootdev, ih);
		अगर (ret)
			जाओ end_stream;
#अगर_घोषित CONFIG_SPL_GZIP
		अगर (ih->ih_comp == IH_COMP_GZIP)
			addr = CONFIG_SYS_LOAD_ADDR;
		अन्यथा
#पूर्ण_अगर
			addr = spl_image->load_addr;
		स_नकल((व्योम *)addr, buf, res);
		ih = (काष्ठा legacy_img_hdr *)addr;
		आकार += res;
		addr += res;

		जबतक ((res = xyzModem_stream_read(buf, BUF_आकार, &err)) > 0) {
			स_नकल((व्योम *)addr, buf, res);
			आकार += res;
			addr += res;
		}
	}

end_stream:
	xyzModem_stream_close(&err);
	xyzModem_stream_terminate(false, &getcymodem);

	म_लिखो("Loaded %lu bytes\n", आकार);

#अगर_घोषित CONFIG_SPL_GZIP
	अगर (!(IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	      image_get_magic((काष्ठा legacy_img_hdr *)buf) == FDT_MAGIC) &&
	    (ih->ih_comp == IH_COMP_GZIP)) {
		अगर (gunzip((व्योम *)(spl_image->load_addr + माप(*ih)),
			   CONFIG_SYS_BOOTM_LEN,
			   (व्योम *)(CONFIG_SYS_LOAD_ADDR + माप(*ih)),
			   &आकार)) {
			माला_दो("Uncompressing error\n");
			वापस -EIO;
		}
	}
#पूर्ण_अगर

	वापस ret;
}
SPL_LOAD_IMAGE_METHOD("UART", 0, BOOT_DEVICE_UART, spl_ymodem_load_image);
