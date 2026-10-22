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
struct ymodem_fit_info {
	整数 image_read;
	char *buf;
};

static 整数 getcymodem(void) {
	如果 (tstc())
		返回 (getchar());
	返回 -1;
}

static ulong ymodem_read_fit(struct spl_load_info *load, ulong offset,
			     ulong size, void *addr)
{
	整数 res, err, buf_offset;
	struct ymodem_fit_info *info = load->priv;
	char *buf = info->buf;
	ulong copy_size = size;

	当 (info->image_read < offset) {
		res = xyzModem_stream_read(buf, BUF_SIZE, &err);
		如果 (res <= 0)
			break;

		info->image_read += res;
	}

	如果 (info->image_read > offset) {
		res = info->image_read - offset;
		如果 (info->image_read % BUF_SIZE)
			buf_offset = (info->image_read % BUF_SIZE);
		否则
			buf_offset = BUF_SIZE;

		如果 (res > copy_size) {
			memcpy(addr, &buf[buf_offset - res], copy_size);
			goto done;
		}
		memcpy(addr, &buf[buf_offset - res], res);
		addr = addr + res;
		copy_size -= res;
	}

	当 (info->image_read < offset + size) {
		res = xyzModem_stream_read(buf, BUF_SIZE, &err);
		如果 (res <= 0)
			break;

		info->image_read += res;
		如果 (res > copy_size) {
			memcpy(addr, buf, copy_size);
			goto done;
		}
		memcpy(addr, buf, res);
		addr += res;
		copy_size -= res;
	}

done:
	返回 size;
}

整数 spl_ymodem_load_image(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev)
{
	ulong size = 0;
	整数 err;
	整数 res;
	整数 ret;
	connection_info_t info;
	char buf[BUF_SIZE];
	struct legacy_img_hdr *ih = NULL;
	ulong addr = 0;

	info.mode = xyzModem_ymodem;
	ret = xyzModem_stream_open(&info, &err);
	如果 (ret) {
		printf("spl: ymodem err - %s\n", xyzModem_error(err));
		返回 ret;
	}

	res = xyzModem_stream_read(buf, BUF_SIZE, &err);
	如果 (res <= 0)
		goto end_stream;

	如果 (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL) &&
	    image_get_magic((struct legacy_img_hdr *)buf) == FDT_MAGIC) {
		addr = CONFIG_SYS_LOAD_ADDR;
		ih = (struct legacy_img_hdr *)addr;

		memcpy((void *)addr, buf, res);
		size += res;
		addr += res;

		当 ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
			memcpy((void *)addr, buf, res);
			size += res;
			addr += res;
		}

		ret = spl_parse_image_header(spl_image, bootdev, ih);
		如果 (ret)
			返回 ret;
	} 否则 如果 (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	    image_get_magic((struct legacy_img_hdr *)buf) == FDT_MAGIC) {
		struct spl_load_info load;
		struct ymodem_fit_info info;

		debug("Found FIT\n");
		spl_load_init(&load, ymodem_read_fit, (void *)&info, 1);
		info.buf = buf;
		info.image_read = BUF_SIZE;
		ret = spl_load_simple_fit(spl_image, &load, 0, (void *)buf);
		size = info.image_read;

		当 ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0)
			size += res;
	} 否则 {
		ih = (struct legacy_img_hdr *)buf;
		ret = spl_parse_image_header(spl_image, bootdev, ih);
		如果 (ret)
			goto end_stream;
#ifdef CONFIG_SPL_GZIP
		如果 (ih->ih_comp == IH_COMP_GZIP)
			addr = CONFIG_SYS_LOAD_ADDR;
		否则
#endif
			addr = spl_image->load_addr;
		memcpy((void *)addr, buf, res);
		ih = (struct legacy_img_hdr *)addr;
		size += res;
		addr += res;

		当 ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
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
	如果 (!(IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	      image_get_magic((struct legacy_img_hdr *)buf) == FDT_MAGIC) &&
	    (ih->ih_comp == IH_COMP_GZIP)) {
		如果 (gunzip((void *)(spl_image->load_addr + sizeof(*ih)),
			   CONFIG_SYS_BOOTM_LEN,
			   (void *)(CONFIG_SYS_LOAD_ADDR + sizeof(*ih)),
			   &size)) {
			puts("Uncompressing error\n");
			返回 -EIO;
		}
	}
#endif

	返回 ret;
}
SPL_LOAD_IMAGE_METHOD("UART", 0, BOOT_DEVICE_UART, spl_ymodem_load_image);
