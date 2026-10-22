// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2021 NXP
 */

#define LOG_CATEGORY LOGC_ARCH
#شامل <stdlib.h>
#شامل <errno.h>
#شامل <imx_container.h>
#شامل <log.h>
#شامل <mapmem.h>
#شامل <spl.h>
#شامل <cpu_func.h>
#ifdef CONFIG_AHAB_BOOT
#شامل <asm/mach-imx/ahab.h>
#endif

__weak bool arch_check_dst_in_secure(تهی *start, ulong size)
{
	بازگشت false;
}

__weak تهی *arch_get_container_trampoline(تهی)
{
	بازگشت پوچ;
}

ایستا ساختار boot_img_t *read_auth_image(ساختار spl_image_info *spl_image,
					  ساختار spl_load_info *info,
					  ساختار container_hdr *container,
					  صحیح image_index,
					  ulong container_offset)
{
	ساختار boot_img_t *images;
	ulong offset, size;
	تهی *buf, *trampoline;

	اگر (image_index > container->رقم_images) {
		debug("Invalid image number\n");
		بازگشت پوچ;
	}

	images = (ساختار boot_img_t *)((u8 *)container +
				       اندازه(ساختار container_hdr));

	اگر (!IS_ALIGNED(images[image_index].offset, spl_get_bl_len(info))) {
		printf("%s: image%d offset not aligned to %u\n",
		       __func__, image_index, spl_get_bl_len(info));
		بازگشت پوچ;
	}

	size = ALIGN(images[image_index].size, spl_get_bl_len(info));
	offset = images[image_index].offset + container_offset;

	debug("%s: container: %p offset: %lu size: %lu\n", __func__,
	      container, offset, size);

	buf = map_sysmem(images[image_index].dst, images[image_index].size);
	اگر (IS_ENABLED(CONFIG_SPL_IMX_CONTAINER_USE_TRAMPOLINE) &&
	    arch_check_dst_in_secure(buf, size)) {
		trampoline = arch_get_container_trampoline();
		اگر (!trampoline) {
			printf("%s: trampoline size is zero\n", __func__);
			بازگشت پوچ;
		}

		اگر (info->اقرأ(info, offset, size, trampoline) < images[image_index].size) {
			printf("%s: failed to load image to a trampoline buffer\n", __func__);
			بازگشت پوچ;
		}

		memcpy(buf, trampoline, images[image_index].size);
	} وگرنه {
		اگر (info->اقرأ(info, offset, size, buf) < images[image_index].size) {
				printf("%s: failed to load image to a non-secure region\n", __func__);
			بازگشت پوچ;
		}
	}

#ifdef CONFIG_AHAB_BOOT
	flush_dcache_range((ulong)buf, (ulong)(buf + images[image_index].size - 1));
	اگر (ahab_verify_cntr_image(&images[image_index], image_index))
		بازگشت پوچ;
#endif

	بازگشت &images[image_index];
}

ایستا صحیح read_auth_container(ساختار spl_image_info *spl_image,
			       ساختار spl_load_info *info, ulong offset)
{
	ساختار container_hdr *container = پوچ;
	ساختار container_hdr *authhdr;
	u16 length;
	صحیح i, size, ret = 0;

	size = ALIGN(CONTAINER_HDR_ALIGNMENT, spl_get_bl_len(info));

	/*
	 * It will not override the ATF code, so safe to use it here,
	 * no need malloc
	 */
	container = malloc(size);
	اگر (!container)
		بازگشت -ENOMEM;

	debug("%s: container: %p offset: %lu size: %u\n", __func__,
	      container, offset, size);
	اگر (info->اقرأ(info, offset, size, container) <
	    CONTAINER_HDR_ALIGNMENT) {
		ret = -EIO;
		برو end;
	}

	اگر (!valid_container_hdr(container)) {
		log_err("Wrong container header\n");
		ret = -ENOENT;
		برو end;
	}

	اگر (!container->رقم_images) {
		log_err("Wrong container, no image found\n");
		ret = -ENOENT;
		برو end;
	}

	length = container->length_lsb + (container->length_msb << 8);
	debug("Container length %u\n", length);

	اگر (length > CONTAINER_HDR_ALIGNMENT) {
		size = ALIGN(length, spl_get_bl_len(info));

		آزاد(container);
		container = malloc(size);
		اگر (!container)
			بازگشت -ENOMEM;

		debug("%s: container: %p offset: %lu size: %u\n",
		      __func__, container, offset, size);
		اگر (info->اقرأ(info, offset, size, container) < length) {
			ret = -EIO;
			برو end;
		}
	}

	authhdr = container;

#ifdef CONFIG_AHAB_BOOT
	authhdr = ahab_auth_cntr_hdr(authhdr, length);
	اگر (!authhdr) {
		ret = -EINVAL;
		برو end_auth;
	}
#endif

	برای (i = 0; i < authhdr->رقم_images; i++) {
		ساختار boot_img_t *image = read_auth_image(spl_image, info,
							   authhdr, i,
							   offset);

		اگر (!image) {
			ret = -EINVAL;
			برو end_auth;
		}

		اگر (i == 0) {
			spl_image->load_addr = image->dst;
			spl_image->entry_point = image->entry;
		}
	}

end_auth:
#ifdef CONFIG_AHAB_BOOT
	ahab_auth_release();
#endif

end:
	آزاد(container);

	بازگشت ret;
}

صحیح spl_load_imx_container(ساختار spl_image_info *spl_image,
			   ساختار spl_load_info *info, ulong offset)
{
	بازگشت read_auth_container(spl_image, info, offset);
}
