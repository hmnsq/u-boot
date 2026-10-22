// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2021 NXP
 */

#define LOG_CATEGORY LOGC_ARCH
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <imx_container.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <mapmem.h>
#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <cpu_func.h>
#ifdef CONFIG_AHAB_BOOT
#அந்தர்பாவ <asm/mach-imx/ahab.h>
#endif

__weak bool arch_check_dst_in_secure(வெறுமை *start, ulong size)
{
	திரும்பு false;
}

__weak வெறுமை *arch_get_container_trampoline(வெறுமை)
{
	திரும்பு வெற்று;
}

நிலையான கட்டமைப்பு boot_img_t *read_auth_image(கட்டமைப்பு spl_image_info *spl_image,
					  கட்டமைப்பு spl_load_info *info,
					  கட்டமைப்பு container_hdr *container,
					  முழு image_index,
					  ulong container_offset)
{
	கட்டமைப்பு boot_img_t *images;
	ulong offset, size;
	வெறுமை *buf, *trampoline;

	எனில் (image_index > container->இலக்கம்_images) {
		debug("Invalid image number\n");
		திரும்பு வெற்று;
	}

	images = (கட்டமைப்பு boot_img_t *)((u8 *)container +
				       அளவு(கட்டமைப்பு container_hdr));

	எனில் (!IS_ALIGNED(images[image_index].offset, spl_get_bl_len(info))) {
		printf("%s: image%d offset not aligned to %u\n",
		       __func__, image_index, spl_get_bl_len(info));
		திரும்பு வெற்று;
	}

	size = ALIGN(images[image_index].size, spl_get_bl_len(info));
	offset = images[image_index].offset + container_offset;

	debug("%s: container: %p offset: %lu size: %lu\n", __func__,
	      container, offset, size);

	buf = map_sysmem(images[image_index].dst, images[image_index].size);
	எனில் (IS_ENABLED(CONFIG_SPL_IMX_CONTAINER_USE_TRAMPOLINE) &&
	    arch_check_dst_in_secure(buf, size)) {
		trampoline = arch_get_container_trampoline();
		எனில் (!trampoline) {
			printf("%s: trampoline size is zero\n", __func__);
			திரும்பு வெற்று;
		}

		எனில் (info->பட(info, offset, size, trampoline) < images[image_index].size) {
			printf("%s: failed to load image to a trampoline buffer\n", __func__);
			திரும்பு வெற்று;
		}

		memcpy(buf, trampoline, images[image_index].size);
	} இல்லையெனில் {
		எனில் (info->பட(info, offset, size, buf) < images[image_index].size) {
				printf("%s: failed to load image to a non-secure region\n", __func__);
			திரும்பு வெற்று;
		}
	}

#ifdef CONFIG_AHAB_BOOT
	flush_dcache_range((ulong)buf, (ulong)(buf + images[image_index].size - 1));
	எனில் (ahab_verify_cntr_image(&images[image_index], image_index))
		திரும்பு வெற்று;
#endif

	திரும்பு &images[image_index];
}

நிலையான முழு read_auth_container(கட்டமைப்பு spl_image_info *spl_image,
			       கட்டமைப்பு spl_load_info *info, ulong offset)
{
	கட்டமைப்பு container_hdr *container = வெற்று;
	கட்டமைப்பு container_hdr *authhdr;
	u16 length;
	முழு i, size, ret = 0;

	size = ALIGN(CONTAINER_HDR_ALIGNMENT, spl_get_bl_len(info));

	/*
	 * It will not override the ATF code, so safe to use it here,
	 * no need malloc
	 */
	container = malloc(size);
	எனில் (!container)
		திரும்பு -ENOMEM;

	debug("%s: container: %p offset: %lu size: %u\n", __func__,
	      container, offset, size);
	எனில் (info->பட(info, offset, size, container) <
	    CONTAINER_HDR_ALIGNMENT) {
		ret = -EIO;
		செல் end;
	}

	எனில் (!valid_container_hdr(container)) {
		log_err("Wrong container header\n");
		ret = -ENOENT;
		செல் end;
	}

	எனில் (!container->இலக்கம்_images) {
		log_err("Wrong container, no image found\n");
		ret = -ENOENT;
		செல் end;
	}

	length = container->length_lsb + (container->length_msb << 8);
	debug("Container length %u\n", length);

	எனில் (length > CONTAINER_HDR_ALIGNMENT) {
		size = ALIGN(length, spl_get_bl_len(info));

		விடுவி(container);
		container = malloc(size);
		எனில் (!container)
			திரும்பு -ENOMEM;

		debug("%s: container: %p offset: %lu size: %u\n",
		      __func__, container, offset, size);
		எனில் (info->பட(info, offset, size, container) < length) {
			ret = -EIO;
			செல் end;
		}
	}

	authhdr = container;

#ifdef CONFIG_AHAB_BOOT
	authhdr = ahab_auth_cntr_hdr(authhdr, length);
	எனில் (!authhdr) {
		ret = -EINVAL;
		செல் end_auth;
	}
#endif

	க்கு (i = 0; i < authhdr->இலக்கம்_images; i++) {
		கட்டமைப்பு boot_img_t *image = read_auth_image(spl_image, info,
							   authhdr, i,
							   offset);

		எனில் (!image) {
			ret = -EINVAL;
			செல் end_auth;
		}

		எனில் (i == 0) {
			spl_image->load_addr = image->dst;
			spl_image->entry_point = image->entry;
		}
	}

end_auth:
#ifdef CONFIG_AHAB_BOOT
	ahab_auth_release();
#endif

end:
	விடுவி(container);

	திரும்பு ret;
}

முழு spl_load_imx_container(கட்டமைப்பு spl_image_info *spl_image,
			   கட்டமைப்பு spl_load_info *info, ulong offset)
{
	திரும்பு read_auth_container(spl_image, info, offset);
}
