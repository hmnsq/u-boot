// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2021 NXP
 */

#define LOG_CATEGORY LOGC_ARCH
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <imx_container.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <mapmem.h>
#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <cpu_func.h>
#ifdef CONFIG_AHAB_BOOT
#അന്തര്ഭാവ <asm/mach-imx/ahab.h>
#endif

__weak bool arch_check_dst_in_secure(ശൂന്യം *start, ulong size)
{
	തിരികെ false;
}

__weak ശൂന്യം *arch_get_container_trampoline(ശൂന്യം)
{
	തിരികെ ശൂന്യ;
}

സ്ഥിതം ഘടന boot_img_t *read_auth_image(ഘടന spl_image_info *spl_image,
					  ഘടന spl_load_info *info,
					  ഘടന container_hdr *container,
					  പൂർണ്ണസംഖ്യ image_index,
					  ulong container_offset)
{
	ഘടന boot_img_t *images;
	ulong offset, size;
	ശൂന്യം *buf, *trampoline;

	എങ്കിൽ (image_index > container->അക്കം_images) {
		debug("Invalid image number\n");
		തിരികെ ശൂന്യ;
	}

	images = (ഘടന boot_img_t *)((u8 *)container +
				       വലുപ്പം(ഘടന container_hdr));

	എങ്കിൽ (!IS_ALIGNED(images[image_index].offset, spl_get_bl_len(info))) {
		printf("%s: image%d offset not aligned to %u\n",
		       __func__, image_index, spl_get_bl_len(info));
		തിരികെ ശൂന്യ;
	}

	size = ALIGN(images[image_index].size, spl_get_bl_len(info));
	offset = images[image_index].offset + container_offset;

	debug("%s: container: %p offset: %lu size: %lu\n", __func__,
	      container, offset, size);

	buf = map_sysmem(images[image_index].dst, images[image_index].size);
	എങ്കിൽ (IS_ENABLED(CONFIG_SPL_IMX_CONTAINER_USE_TRAMPOLINE) &&
	    arch_check_dst_in_secure(buf, size)) {
		trampoline = arch_get_container_trampoline();
		എങ്കിൽ (!trampoline) {
			printf("%s: trampoline size is zero\n", __func__);
			തിരികെ ശൂന്യ;
		}

		എങ്കിൽ (info->പഠ(info, offset, size, trampoline) < images[image_index].size) {
			printf("%s: failed to load image to a trampoline buffer\n", __func__);
			തിരികെ ശൂന്യ;
		}

		memcpy(buf, trampoline, images[image_index].size);
	} അല്ലെങ്കിൽ {
		എങ്കിൽ (info->പഠ(info, offset, size, buf) < images[image_index].size) {
				printf("%s: failed to load image to a non-secure region\n", __func__);
			തിരികെ ശൂന്യ;
		}
	}

#ifdef CONFIG_AHAB_BOOT
	flush_dcache_range((ulong)buf, (ulong)(buf + images[image_index].size - 1));
	എങ്കിൽ (ahab_verify_cntr_image(&images[image_index], image_index))
		തിരികെ ശൂന്യ;
#endif

	തിരികെ &images[image_index];
}

സ്ഥിതം പൂർണ്ണസംഖ്യ read_auth_container(ഘടന spl_image_info *spl_image,
			       ഘടന spl_load_info *info, ulong offset)
{
	ഘടന container_hdr *container = ശൂന്യ;
	ഘടന container_hdr *authhdr;
	u16 length;
	പൂർണ്ണസംഖ്യ i, size, ret = 0;

	size = ALIGN(CONTAINER_HDR_ALIGNMENT, spl_get_bl_len(info));

	/*
	 * It will not override the ATF code, so safe to use it here,
	 * no need malloc
	 */
	container = malloc(size);
	എങ്കിൽ (!container)
		തിരികെ -ENOMEM;

	debug("%s: container: %p offset: %lu size: %u\n", __func__,
	      container, offset, size);
	എങ്കിൽ (info->പഠ(info, offset, size, container) <
	    CONTAINER_HDR_ALIGNMENT) {
		ret = -EIO;
		പോകുക end;
	}

	എങ്കിൽ (!valid_container_hdr(container)) {
		log_err("Wrong container header\n");
		ret = -ENOENT;
		പോകുക end;
	}

	എങ്കിൽ (!container->അക്കം_images) {
		log_err("Wrong container, no image found\n");
		ret = -ENOENT;
		പോകുക end;
	}

	length = container->length_lsb + (container->length_msb << 8);
	debug("Container length %u\n", length);

	എങ്കിൽ (length > CONTAINER_HDR_ALIGNMENT) {
		size = ALIGN(length, spl_get_bl_len(info));

		സ്വതന്ത്രം(container);
		container = malloc(size);
		എങ്കിൽ (!container)
			തിരികെ -ENOMEM;

		debug("%s: container: %p offset: %lu size: %u\n",
		      __func__, container, offset, size);
		എങ്കിൽ (info->പഠ(info, offset, size, container) < length) {
			ret = -EIO;
			പോകുക end;
		}
	}

	authhdr = container;

#ifdef CONFIG_AHAB_BOOT
	authhdr = ahab_auth_cntr_hdr(authhdr, length);
	എങ്കിൽ (!authhdr) {
		ret = -EINVAL;
		പോകുക end_auth;
	}
#endif

	വേണ്ടി (i = 0; i < authhdr->അക്കം_images; i++) {
		ഘടന boot_img_t *image = read_auth_image(spl_image, info,
							   authhdr, i,
							   offset);

		എങ്കിൽ (!image) {
			ret = -EINVAL;
			പോകുക end_auth;
		}

		എങ്കിൽ (i == 0) {
			spl_image->load_addr = image->dst;
			spl_image->entry_point = image->entry;
		}
	}

end_auth:
#ifdef CONFIG_AHAB_BOOT
	ahab_auth_release();
#endif

end:
	സ്വതന്ത്രം(container);

	തിരികെ ret;
}

പൂർണ്ണസംഖ്യ spl_load_imx_container(ഘടന spl_image_info *spl_image,
			   ഘടന spl_load_info *info, ulong offset)
{
	തിരികെ read_auth_container(spl_image, info, offset);
}
