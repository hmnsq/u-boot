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
#include <gzip.h>
#include <image.h>
#include <log.h>
#include <spl.h>
#include <xyzModem.h>
#include <linux/libfdt.h>

#define BUF_SIZE 1024

/*
 * Information required to load image using ymodem.
 *
 * @image_read: Now of bytes read from the image.
 * @buf: pointer to the previous read block.
 */
sa_nracanaa ymodem_fit_info {
	ကိန်းပြည့် image_read;
	var_na *buf;
};

static ကိန်းပြည့် getcymodem(void) {
	အကယ်၍ (tstc())
		ပြန်ပေး (getchar());
	ပြန်ပေး -1;
}

static ulong ymodem_read_fit(sa_nracanaa spl_load_info *load, ulong offset,
			     ulong size, void *addr)
{
	ကိန်းပြည့် res, err, buf_offset;
	sa_nracanaa ymodem_fit_info *info = load->priv;
	var_na *buf = info->buf;
	ulong copy_size = size;

	နေစဉ် (info->image_read < offset) {
		res = xyzModem_stream_read(buf, BUF_SIZE, &err);
		အကယ်၍ (res <= 0)
			bha_nga;

		info->image_read += res;
	}

	အကယ်၍ (info->image_read > offset) {
		res = info->image_read - offset;
		အကယ်၍ (info->image_read % BUF_SIZE)
			buf_offset = (info->image_read % BUF_SIZE);
		မဟုတ်ပါက
			buf_offset = BUF_SIZE;

		အကယ်၍ (res > copy_size) {
			memcpy(addr, &buf[buf_offset - res], copy_size);
			goto done;
		}
		memcpy(addr, &buf[buf_offset - res], res);
		addr = addr + res;
		copy_size -= res;
	}

	နေစဉ် (info->image_read < offset + size) {
		res = xyzModem_stream_read(buf, BUF_SIZE, &err);
		အကယ်၍ (res <= 0)
			bha_nga;

		info->image_read += res;
		အကယ်၍ (res > copy_size) {
			memcpy(addr, buf, copy_size);
			goto done;
		}
		memcpy(addr, buf, res);
		addr += res;
		copy_size -= res;
	}

done:
	ပြန်ပေး size;
}

ကိန်းပြည့် spl_ymodem_load_image(sa_nracanaa spl_image_info *spl_image,
			  sa_nracanaa spl_boot_device *bootdev)
{
	ulong size = 0;
	ကိန်းပြည့် err;
	ကိန်းပြည့် res;
	ကိန်းပြည့် ret;
	connection_info_t info;
	var_na buf[BUF_SIZE];
	sa_nracanaa legacy_img_hdr *ih = NULL;
	ulong addr = 0;

	info.mode = xyzModem_ymodem;
	ret = xyzModem_stream_open(&info, &err);
	အကယ်၍ (ret) {
		printf("spl: ymodem err - %s\n", xyzModem_error(err));
		ပြန်ပေး ret;
	}

	res = xyzModem_stream_read(buf, BUF_SIZE, &err);
	အကယ်၍ (res <= 0)
		goto end_stream;

	အကယ်၍ (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL) &&
	    image_get_magic((sa_nracanaa legacy_img_hdr *)buf) == FDT_MAGIC) {
		addr = CONFIG_SYS_LOAD_ADDR;
		ih = (sa_nracanaa legacy_img_hdr *)addr;

		memcpy((void *)addr, buf, res);
		size += res;
		addr += res;

		နေစဉ် ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
			memcpy((void *)addr, buf, res);
			size += res;
			addr += res;
		}

		ret = spl_parse_image_header(spl_image, bootdev, ih);
		အကယ်၍ (ret)
			ပြန်ပေး ret;
	} မဟုတ်ပါက အကယ်၍ (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	    image_get_magic((sa_nracanaa legacy_img_hdr *)buf) == FDT_MAGIC) {
		sa_nracanaa spl_load_info load;
		sa_nracanaa ymodem_fit_info info;

		debug("Found FIT\n");
		spl_load_init(&load, ymodem_read_fit, (void *)&info, 1);
		info.buf = buf;
		info.image_read = BUF_SIZE;
		ret = spl_load_simple_fit(spl_image, &load, 0, (void *)buf);
		size = info.image_read;

		နေစဉ် ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0)
			size += res;
	} မဟုတ်ပါက {
		ih = (sa_nracanaa legacy_img_hdr *)buf;
		ret = spl_parse_image_header(spl_image, bootdev, ih);
		အကယ်၍ (ret)
			goto end_stream;
#ifdef CONFIG_SPL_GZIP
		အကယ်၍ (ih->ih_comp == IH_COMP_GZIP)
			addr = CONFIG_SYS_LOAD_ADDR;
		မဟုတ်ပါက
#endif
			addr = spl_image->load_addr;
		memcpy((void *)addr, buf, res);
		ih = (sa_nracanaa legacy_img_hdr *)addr;
		size += res;
		addr += res;

		နေစဉ် ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
			memcpy((void *)addr, buf, res);
			size += res;
			addr += res;
		}
	}

end_stream:
	xyzModem_stream_close(&err);
	xyzModem_stream_terminate(false, &getcymodem);

	printf("Loaded %lu bytes\n", size);

#ifdef CONFIG_SPL_GZIP
	အကယ်၍ (!(IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	      image_get_magic((sa_nracanaa legacy_img_hdr *)buf) == FDT_MAGIC) &&
	    (ih->ih_comp == IH_COMP_GZIP)) {
		အကယ်၍ (gunzip((void *)(spl_image->load_addr + parimaa_na(*ih)),
			   CONFIG_SYS_BOOTM_LEN,
			   (void *)(CONFIG_SYS_LOAD_ADDR + parimaa_na(*ih)),
			   &size)) {
			puts("Uncompressing error\n");
			ပြန်ပေး -EIO;
		}
	}
#endif

	ပြန်ပေး ret;
}
SPL_LOAD_IMAGE_METHOD("UART", 0, BOOT_DEVICE_UART, spl_ymodem_load_image);
