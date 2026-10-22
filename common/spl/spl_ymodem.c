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

#ವ್ಯಾಖ್ಯಾನಿಸು BUF_ಗಾತ್ರ 1024

/*
 * Information required to load image using ymodem.
 *
 * @image_read: Now of bytes read from the image.
 * @buf: pointer to the previous read block.
 */
ರಚನೆ ymodem_fit_info {
	ಪೂರ್ಣಾಂಕ image_read;
	ಅಕ್ಷರ *buf;
};

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ getcymodem(ಶೂನ್ಯ) {
	ಆದರೆ (tstc())
		ಹಿಂತಿರುಗಿ (getchar());
	ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ulong ymodem_read_fit(ರಚನೆ spl_ತುಂಬು_info *ತುಂಬು, ulong ಸ್ಥಾನಾಂತರ,
			     ulong ಗಾತ್ರ_2, ಶೂನ್ಯ *addr)
{
	ಪೂರ್ಣಾಂಕ res, err, buf_ಸ್ಥಾನಾಂತರ;
	ರಚನೆ ymodem_fit_info *info = ತುಂಬು->priv;
	ಅಕ್ಷರ *buf = info->buf;
	ulong copy_ಗಾತ್ರ = ಗಾತ್ರ_2;

	ಆಗ (info->image_read < ಸ್ಥಾನಾಂತರ) {
		res = xyz_Modem_ಹರಿವು_read(buf, BUF_ಗಾತ್ರ, &err);
		ಆದರೆ (res <= 0)
			ನಿಲ್ಲಿಸು;

		info->image_read += res;
	}

	ಆದರೆ (info->image_read > ಸ್ಥಾನಾಂತರ) {
		res = info->image_read - ಸ್ಥಾನಾಂತರ;
		ಆದರೆ (info->image_read % BUF_ಗಾತ್ರ)
			buf_ಸ್ಥಾನಾಂತರ = (info->image_read % BUF_ಗಾತ್ರ);
		ಇಲ್ಲದಿದ್ದರೆ
			buf_ಸ್ಥಾನಾಂತರ = BUF_ಗಾತ್ರ;

		ಆದರೆ (res > copy_ಗಾತ್ರ) {
			ಸ್ಮರಣೆ_ನಕಲು(addr, &buf[buf_ಸ್ಥಾನಾಂತರ - res], copy_ಗಾತ್ರ);
			ಹೋಗು ಮುಗಿದ;
		}
		ಸ್ಮರಣೆ_ನಕಲು(addr, &buf[buf_ಸ್ಥಾನಾಂತರ - res], res);
		addr = addr + res;
		copy_ಗಾತ್ರ -= res;
	}

	ಆಗ (info->image_read < ಸ್ಥಾನಾಂತರ + ಗಾತ್ರ_2) {
		res = xyz_Modem_ಹರಿವು_read(buf, BUF_ಗಾತ್ರ, &err);
		ಆದರೆ (res <= 0)
			ನಿಲ್ಲಿಸು;

		info->image_read += res;
		ಆದರೆ (res > copy_ಗಾತ್ರ) {
			ಸ್ಮರಣೆ_ನಕಲು(addr, buf, copy_ಗಾತ್ರ);
			ಹೋಗು ಮುಗಿದ;
		}
		ಸ್ಮರಣೆ_ನಕಲು(addr, buf, res);
		addr += res;
		copy_ಗಾತ್ರ -= res;
	}

ಮುಗಿದ:
	ಹಿಂತಿರುಗಿ ಗಾತ್ರ_2;
}

ಪೂರ್ಣಾಂಕ spl_ymodem_ತುಂಬು_image(ರಚನೆ spl_image_info *spl_image,
			  ರಚನೆ spl_boot_device *bootdev)
{
	ulong ಗಾತ್ರ_2 = 0;
	ಪೂರ್ಣಾಂಕ err;
	ಪೂರ್ಣಾಂಕ res;
	ಪೂರ್ಣಾಂಕ ret;
	connection_info_t info;
	ಅಕ್ಷರ buf[BUF_ಗಾತ್ರ];
	ರಚನೆ legacy_img_hdr *ih = ಖಾಲಿ;
	ulong addr = 0;

	info.ಬಹುಲಕ = xyzModem_ymodem;
	ret = xyz_Modem_ಹರಿವು_open(&info, &err);
	ಆದರೆ (ret) {
		ಬರೆ("spl: ymodem err - %s\n", xyzModem_error(err));
		ಹಿಂತಿರುಗಿ ret;
	}

	res = xyz_Modem_ಹರಿವು_read(buf, BUF_ಗಾತ್ರ, &err);
	ಆದರೆ (res <= 0)
		ಹೋಗು ಕೊನೆ_ಹರಿವು;

	ಆದರೆ (IS_ENABLED(CONFIG_SPL_ತುಂಬು_FIT_ತುಂಬಿದ) &&
	    image_get_magic((ರಚನೆ legacy_img_hdr *)buf) == FDT_MAGIC) {
		addr = CONFIG_SYS_ತುಂಬು_ADDR;
		ih = (ರಚನೆ legacy_img_hdr *)addr;

		ಸ್ಮರಣೆ_ನಕಲು((ಶೂನ್ಯ *)addr, buf, res);
		ಗಾತ್ರ_2 += res;
		addr += res;

		ಆಗ ((res = xyz_Modem_ಹರಿವು_read(buf, BUF_ಗಾತ್ರ, &err)) > 0) {
			ಸ್ಮರಣೆ_ನಕಲು((ಶೂನ್ಯ *)addr, buf, res);
			ಗಾತ್ರ_2 += res;
			addr += res;
		}

		ret = spl_parse_image_ಶೀರ್ಷಿಕೆ(spl_image, bootdev, ih);
		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ ret;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (IS_ENABLED(CONFIG_SPL_ತುಂಬು_FIT) &&
	    image_get_magic((ರಚನೆ legacy_img_hdr *)buf) == FDT_MAGIC) {
		ರಚನೆ spl_ತುಂಬು_info ತುಂಬು;
		ರಚನೆ ymodem_fit_info info;

		debug("Found FIT\n");
		spl_ತುಂಬು_init(&ತುಂಬು, ymodem_read_fit, (ಶೂನ್ಯ *)&info, 1);
		info.buf = buf;
		info.image_read = BUF_ಗಾತ್ರ;
		ret = spl_ತುಂಬು_simple_fit(spl_image, &ತುಂಬು, 0, (ಶೂನ್ಯ *)buf);
		ಗಾತ್ರ_2 = info.image_read;

		ಆಗ ((res = xyz_Modem_ಹರಿವು_read(buf, BUF_ಗಾತ್ರ, &err)) > 0)
			ಗಾತ್ರ_2 += res;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ih = (ರಚನೆ legacy_img_hdr *)buf;
		ret = spl_parse_image_ಶೀರ್ಷಿಕೆ(spl_image, bootdev, ih);
		ಆದರೆ (ret)
			ಹೋಗು ಕೊನೆ_ಹರಿವು;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPL_GZIP
		ಆದರೆ (ih->ih_comp == IH_COMP_GZIP)
			addr = CONFIG_SYS_ತುಂಬು_ADDR;
		ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
			addr = spl_image->ತುಂಬು_addr;
		ಸ್ಮರಣೆ_ನಕಲು((ಶೂನ್ಯ *)addr, buf, res);
		ih = (ರಚನೆ legacy_img_hdr *)addr;
		ಗಾತ್ರ_2 += res;
		addr += res;

		ಆಗ ((res = xyz_Modem_ಹರಿವು_read(buf, BUF_ಗಾತ್ರ, &err)) > 0) {
			ಸ್ಮರಣೆ_ನಕಲು((ಶೂನ್ಯ *)addr, buf, res);
			ಗಾತ್ರ_2 += res;
			addr += res;
		}
	}

ಕೊನೆ_ಹರಿವು:
	xyz_Modem_ಹರಿವು_close(&err);
	xyz_Modem_ಹರಿವು_terminate(false, &getcymodem);

	ಬರೆ("Loaded %lu bytes\n", ಗಾತ್ರ_2);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPL_GZIP
	ಆದರೆ (!(IS_ENABLED(CONFIG_SPL_ತುಂಬು_FIT) &&
	      image_get_magic((ರಚನೆ legacy_img_hdr *)buf) == FDT_MAGIC) &&
	    (ih->ih_comp == IH_COMP_GZIP)) {
		ಆದರೆ (gunzip((ಶೂನ್ಯ *)(spl_image->ತುಂಬು_addr + ಗಾತ್ರ(*ih)),
			   CONFIG_SYS_BOOTM_LEN,
			   (ಶೂನ್ಯ *)(CONFIG_SYS_ತುಂಬು_ADDR + ಗಾತ್ರ(*ih)),
			   &ಗಾತ್ರ_2)) {
			puts("Uncompressing error\n");
			ಹಿಂತಿರುಗಿ -EIO;
		}
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ ret;
}
SPL_ತುಂಬು_IMAGE_ವಿಧಾನ("UART", 0, BOOT_DEVICE_UART, spl_ymodem_ತುಂಬು_image);
