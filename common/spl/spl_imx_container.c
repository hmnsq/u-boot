// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2021 NXP
 */

#define LOG_CATEGORY LOGC_ARCH
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <imx_container.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <cpu_func.h>
#ifdef CONFIG_AHAB_BOOT
#ಅನ್ತರ್ಭಾವ <asm/mach-imx/ahab.h>
#endif

__weak bool arch_check_dst_in_secure(ಶೂನ್ಯ *start, ulong size)
{
	ಪ್ರತ್ಯಾಗಮ false;
}

__weak ಶೂನ್ಯ *arch_get_container_trampoline(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸ್ಥಿರ ಸಂರಚನಾ boot_img_t *read_auth_image(ಸಂರಚನಾ spl_image_info *spl_image,
					  ಸಂರಚನಾ spl_load_info *info,
					  ಸಂರಚನಾ container_hdr *container,
					  ಪೂರ್ಣಾಙ್ಕ image_index,
					  ulong container_offset)
{
	ಸಂರಚನಾ boot_img_t *images;
	ulong offset, size;
	ಶೂನ್ಯ *buf, *trampoline;

	ಯದಿ (image_index > container->ಅಂಕೆ_images) {
		debug("Invalid image number\n");
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	images = (ಸಂರಚನಾ boot_img_t *)((u8 *)container +
				       ಪರಿಮಾಣ(ಸಂರಚನಾ container_hdr));

	ಯದಿ (!IS_ALIGNED(images[image_index].offset, spl_get_bl_len(info))) {
		printf("%s: image%d offset not aligned to %u\n",
		       __func__, image_index, spl_get_bl_len(info));
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	size = ALIGN(images[image_index].size, spl_get_bl_len(info));
	offset = images[image_index].offset + container_offset;

	debug("%s: container: %p offset: %lu size: %lu\n", __func__,
	      container, offset, size);

	buf = map_sysmem(images[image_index].dst, images[image_index].size);
	ಯದಿ (IS_ENABLED(CONFIG_SPL_IMX_CONTAINER_USE_TRAMPOLINE) &&
	    arch_check_dst_in_secure(buf, size)) {
		trampoline = arch_get_container_trampoline();
		ಯದಿ (!trampoline) {
			printf("%s: trampoline size is zero\n", __func__);
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}

		ಯದಿ (info->ಪಠ(info, offset, size, trampoline) < images[image_index].size) {
			printf("%s: failed to load image to a trampoline buffer\n", __func__);
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}

		memcpy(buf, trampoline, images[image_index].size);
	} ಅನ್ಯಥಾ {
		ಯದಿ (info->ಪಠ(info, offset, size, buf) < images[image_index].size) {
				printf("%s: failed to load image to a non-secure region\n", __func__);
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}
	}

#ifdef CONFIG_AHAB_BOOT
	flush_dcache_range((ulong)buf, (ulong)(buf + images[image_index].size - 1));
	ಯದಿ (ahab_verify_cntr_image(&images[image_index], image_index))
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
#endif

	ಪ್ರತ್ಯಾಗಮ &images[image_index];
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ read_auth_container(ಸಂರಚನಾ spl_image_info *spl_image,
			       ಸಂರಚನಾ spl_load_info *info, ulong offset)
{
	ಸಂರಚನಾ container_hdr *container = ರಿಕ್ತ;
	ಸಂರಚನಾ container_hdr *authhdr;
	u16 length;
	ಪೂರ್ಣಾಙ್ಕ i, size, ret = 0;

	size = ALIGN(CONTAINER_HDR_ALIGNMENT, spl_get_bl_len(info));

	/*
	 * It will not override the ATF code, so safe to use it here,
	 * no need malloc
	 */
	container = malloc(size);
	ಯದಿ (!container)
		ಪ್ರತ್ಯಾಗಮ -ENOMEM;

	debug("%s: container: %p offset: %lu size: %u\n", __func__,
	      container, offset, size);
	ಯದಿ (info->ಪಠ(info, offset, size, container) <
	    CONTAINER_HDR_ALIGNMENT) {
		ret = -EIO;
		ಗಚ್ಛ end;
	}

	ಯದಿ (!valid_container_hdr(container)) {
		log_err("Wrong container header\n");
		ret = -ENOENT;
		ಗಚ್ಛ end;
	}

	ಯದಿ (!container->ಅಂಕೆ_images) {
		log_err("Wrong container, no image found\n");
		ret = -ENOENT;
		ಗಚ್ಛ end;
	}

	length = container->length_lsb + (container->length_msb << 8);
	debug("Container length %u\n", length);

	ಯದಿ (length > CONTAINER_HDR_ALIGNMENT) {
		size = ALIGN(length, spl_get_bl_len(info));

		ಮುಕ್ತ(container);
		container = malloc(size);
		ಯದಿ (!container)
			ಪ್ರತ್ಯಾಗಮ -ENOMEM;

		debug("%s: container: %p offset: %lu size: %u\n",
		      __func__, container, offset, size);
		ಯದಿ (info->ಪಠ(info, offset, size, container) < length) {
			ret = -EIO;
			ಗಚ್ಛ end;
		}
	}

	authhdr = container;

#ifdef CONFIG_AHAB_BOOT
	authhdr = ahab_auth_cntr_hdr(authhdr, length);
	ಯದಿ (!authhdr) {
		ret = -EINVAL;
		ಗಚ್ಛ end_auth;
	}
#endif

	ಪ್ರತಿ (i = 0; i < authhdr->ಅಂಕೆ_images; i++) {
		ಸಂರಚನಾ boot_img_t *image = read_auth_image(spl_image, info,
							   authhdr, i,
							   offset);

		ಯದಿ (!image) {
			ret = -EINVAL;
			ಗಚ್ಛ end_auth;
		}

		ಯದಿ (i == 0) {
			spl_image->load_addr = image->dst;
			spl_image->entry_point = image->entry;
		}
	}

end_auth:
#ifdef CONFIG_AHAB_BOOT
	ahab_auth_release();
#endif

end:
	ಮುಕ್ತ(container);

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಪೂರ್ಣಾಙ್ಕ spl_load_imx_container(ಸಂರಚನಾ spl_image_info *spl_image,
			   ಸಂರಚನಾ spl_load_info *info, ulong offset)
{
	ಪ್ರತ್ಯಾಗಮ read_auth_container(spl_image, info, offset);
}
