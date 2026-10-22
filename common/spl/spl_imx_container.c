// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2021 NXP
 */

#define LOG_CATEGORY LOGC_ARCH
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <imx_container.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <mapmem.h>
#অন্তর্ভাব <spl.h>
#অন্তর্ভাব <cpu_func.h>
#ifdef CONFIG_AHAB_BOOT
#অন্তর্ভাব <asm/mach-imx/ahab.h>
#endif

__weak bool arch_check_dst_in_secure(শূন্য *start, ulong size)
{
	প্রত্যাগম false;
}

__weak শূন্য *arch_get_container_trampoline(শূন্য)
{
	প্রত্যাগম রিক্ত;
}

স্থির সংরচনা boot_img_t *read_auth_image(সংরচনা spl_image_info *spl_image,
					  সংরচনা spl_load_info *info,
					  সংরচনা container_hdr *container,
					  পূর্ণাঙ্ক image_index,
					  ulong container_offset)
{
	সংরচনা boot_img_t *images;
	ulong offset, size;
	শূন্য *buf, *trampoline;

	যদি (image_index > container->অঙ্ক_images) {
		debug("Invalid image number\n");
		প্রত্যাগম রিক্ত;
	}

	images = (সংরচনা boot_img_t *)((u8 *)container +
				       পরিমাণ(সংরচনা container_hdr));

	যদি (!IS_ALIGNED(images[image_index].offset, spl_get_bl_len(info))) {
		printf("%s: image%d offset not aligned to %u\n",
		       __func__, image_index, spl_get_bl_len(info));
		প্রত্যাগম রিক্ত;
	}

	size = ALIGN(images[image_index].size, spl_get_bl_len(info));
	offset = images[image_index].offset + container_offset;

	debug("%s: container: %p offset: %lu size: %lu\n", __func__,
	      container, offset, size);

	buf = map_sysmem(images[image_index].dst, images[image_index].size);
	যদি (IS_ENABLED(CONFIG_SPL_IMX_CONTAINER_USE_TRAMPOLINE) &&
	    arch_check_dst_in_secure(buf, size)) {
		trampoline = arch_get_container_trampoline();
		যদি (!trampoline) {
			printf("%s: trampoline size is zero\n", __func__);
			প্রত্যাগম রিক্ত;
		}

		যদি (info->পঠ(info, offset, size, trampoline) < images[image_index].size) {
			printf("%s: failed to load image to a trampoline buffer\n", __func__);
			প্রত্যাগম রিক্ত;
		}

		memcpy(buf, trampoline, images[image_index].size);
	} অন্যথা {
		যদি (info->পঠ(info, offset, size, buf) < images[image_index].size) {
				printf("%s: failed to load image to a non-secure region\n", __func__);
			প্রত্যাগম রিক্ত;
		}
	}

#ifdef CONFIG_AHAB_BOOT
	flush_dcache_range((ulong)buf, (ulong)(buf + images[image_index].size - 1));
	যদি (ahab_verify_cntr_image(&images[image_index], image_index))
		প্রত্যাগম রিক্ত;
#endif

	প্রত্যাগম &images[image_index];
}

স্থির পূর্ণাঙ্ক read_auth_container(সংরচনা spl_image_info *spl_image,
			       সংরচনা spl_load_info *info, ulong offset)
{
	সংরচনা container_hdr *container = রিক্ত;
	সংরচনা container_hdr *authhdr;
	u16 length;
	পূর্ণাঙ্ক i, size, ret = 0;

	size = ALIGN(CONTAINER_HDR_ALIGNMENT, spl_get_bl_len(info));

	/*
	 * It will not override the ATF code, so safe to use it here,
	 * no need malloc
	 */
	container = malloc(size);
	যদি (!container)
		প্রত্যাগম -ENOMEM;

	debug("%s: container: %p offset: %lu size: %u\n", __func__,
	      container, offset, size);
	যদি (info->পঠ(info, offset, size, container) <
	    CONTAINER_HDR_ALIGNMENT) {
		ret = -EIO;
		গচ্ছ end;
	}

	যদি (!valid_container_hdr(container)) {
		log_err("Wrong container header\n");
		ret = -ENOENT;
		গচ্ছ end;
	}

	যদি (!container->অঙ্ক_images) {
		log_err("Wrong container, no image found\n");
		ret = -ENOENT;
		গচ্ছ end;
	}

	length = container->length_lsb + (container->length_msb << 8);
	debug("Container length %u\n", length);

	যদি (length > CONTAINER_HDR_ALIGNMENT) {
		size = ALIGN(length, spl_get_bl_len(info));

		মুক্ত(container);
		container = malloc(size);
		যদি (!container)
			প্রত্যাগম -ENOMEM;

		debug("%s: container: %p offset: %lu size: %u\n",
		      __func__, container, offset, size);
		যদি (info->পঠ(info, offset, size, container) < length) {
			ret = -EIO;
			গচ্ছ end;
		}
	}

	authhdr = container;

#ifdef CONFIG_AHAB_BOOT
	authhdr = ahab_auth_cntr_hdr(authhdr, length);
	যদি (!authhdr) {
		ret = -EINVAL;
		গচ্ছ end_auth;
	}
#endif

	প্রতি (i = 0; i < authhdr->অঙ্ক_images; i++) {
		সংরচনা boot_img_t *image = read_auth_image(spl_image, info,
							   authhdr, i,
							   offset);

		যদি (!image) {
			ret = -EINVAL;
			গচ্ছ end_auth;
		}

		যদি (i == 0) {
			spl_image->load_addr = image->dst;
			spl_image->entry_point = image->entry;
		}
	}

end_auth:
#ifdef CONFIG_AHAB_BOOT
	ahab_auth_release();
#endif

end:
	মুক্ত(container);

	প্রত্যাগম ret;
}

পূর্ণাঙ্ক spl_load_imx_container(সংরচনা spl_image_info *spl_image,
			   সংরচনা spl_load_info *info, ulong offset)
{
	প্রত্যাগম read_auth_container(spl_image, info, offset);
}
