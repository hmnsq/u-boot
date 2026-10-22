// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2021 NXP
 */

#define LOG_CATEGORY LOGC_ARCH
#ضمن <stdlib.h>
#ضمن <errno.h>
#ضمن <imx_container.h>
#ضمن <log.h>
#ضمن <mapmem.h>
#ضمن <spl.h>
#ضمن <cpu_func.h>
#ifdef CONFIG_AHAB_BOOT
#ضمن <asm/mach-imx/ahab.h>
#endif

__weak bool arch_check_dst_in_secure(فراغ *start, ulong size)
{
	أرجع false;
}

__weak فراغ *arch_get_container_trampoline(فراغ)
{
	أرجع عدم;
}

ساكن بنية boot_img_t *read_auth_image(بنية spl_image_info *spl_image,
					  بنية spl_load_info *info,
					  بنية container_hdr *container,
					  صحيح image_index,
					  ulong container_offset)
{
	بنية boot_img_t *images;
	ulong offset, size;
	فراغ *buf, *trampoline;

	إذا (image_index > container->لمب_images) {
		debug("Invalid image number\n");
		أرجع عدم;
	}

	images = (بنية boot_img_t *)((u8 *)container +
				       حجم(بنية container_hdr));

	إذا (!IS_ALIGNED(images[image_index].offset, spl_get_bl_len(info))) {
		printf("%s: image%d offset not aligned to %u\n",
		       __func__, image_index, spl_get_bl_len(info));
		أرجع عدم;
	}

	size = ALIGN(images[image_index].size, spl_get_bl_len(info));
	offset = images[image_index].offset + container_offset;

	debug("%s: container: %p offset: %lu size: %lu\n", __func__,
	      container, offset, size);

	buf = map_sysmem(images[image_index].dst, images[image_index].size);
	إذا (IS_ENABLED(CONFIG_SPL_IMX_CONTAINER_USE_TRAMPOLINE) &&
	    arch_check_dst_in_secure(buf, size)) {
		trampoline = arch_get_container_trampoline();
		إذا (!trampoline) {
			printf("%s: trampoline size is zero\n", __func__);
			أرجع عدم;
		}

		إذا (info->اقرأ(info, offset, size, trampoline) < images[image_index].size) {
			printf("%s: failed to load image to a trampoline buffer\n", __func__);
			أرجع عدم;
		}

		memcpy(buf, trampoline, images[image_index].size);
	} وإلا {
		إذا (info->اقرأ(info, offset, size, buf) < images[image_index].size) {
				printf("%s: failed to load image to a non-secure region\n", __func__);
			أرجع عدم;
		}
	}

#ifdef CONFIG_AHAB_BOOT
	flush_dcache_range((ulong)buf, (ulong)(buf + images[image_index].size - 1));
	إذا (ahab_verify_cntr_image(&images[image_index], image_index))
		أرجع عدم;
#endif

	أرجع &images[image_index];
}

ساكن صحيح read_auth_container(بنية spl_image_info *spl_image,
			       بنية spl_load_info *info, ulong offset)
{
	بنية container_hdr *container = عدم;
	بنية container_hdr *authhdr;
	u16 length;
	صحيح i, size, ret = 0;

	size = ALIGN(CONTAINER_HDR_ALIGNMENT, spl_get_bl_len(info));

	/*
	 * It will not override the ATF code, so safe to use it here,
	 * no need malloc
	 */
	container = malloc(size);
	إذا (!container)
		أرجع -ENOMEM;

	debug("%s: container: %p offset: %lu size: %u\n", __func__,
	      container, offset, size);
	إذا (info->اقرأ(info, offset, size, container) <
	    CONTAINER_HDR_ALIGNMENT) {
		ret = -EIO;
		اذهب end;
	}

	إذا (!valid_container_hdr(container)) {
		log_err("Wrong container header\n");
		ret = -ENOENT;
		اذهب end;
	}

	إذا (!container->لمب_images) {
		log_err("Wrong container, no image found\n");
		ret = -ENOENT;
		اذهب end;
	}

	length = container->length_lsb + (container->length_msb << 8);
	debug("Container length %u\n", length);

	إذا (length > CONTAINER_HDR_ALIGNMENT) {
		size = ALIGN(length, spl_get_bl_len(info));

		حرر(container);
		container = malloc(size);
		إذا (!container)
			أرجع -ENOMEM;

		debug("%s: container: %p offset: %lu size: %u\n",
		      __func__, container, offset, size);
		إذا (info->اقرأ(info, offset, size, container) < length) {
			ret = -EIO;
			اذهب end;
		}
	}

	authhdr = container;

#ifdef CONFIG_AHAB_BOOT
	authhdr = ahab_auth_cntr_hdr(authhdr, length);
	إذا (!authhdr) {
		ret = -EINVAL;
		اذهب end_auth;
	}
#endif

	لكل (i = 0; i < authhdr->لمب_images; i++) {
		بنية boot_img_t *image = read_auth_image(spl_image, info,
							   authhdr, i,
							   offset);

		إذا (!image) {
			ret = -EINVAL;
			اذهب end_auth;
		}

		إذا (i == 0) {
			spl_image->load_addr = image->dst;
			spl_image->entry_point = image->entry;
		}
	}

end_auth:
#ifdef CONFIG_AHAB_BOOT
	ahab_auth_release();
#endif

end:
	حرر(container);

	أرجع ret;
}

صحيح spl_load_imx_container(بنية spl_image_info *spl_image,
			   بنية spl_load_info *info, ulong offset)
{
	أرجع read_auth_container(spl_image, info, offset);
}
