// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2021 NXP
 */

#define LOG_CATEGORY LOGC_ARCH
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <imx_container.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <mapmem.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <cpu_func.h>
#ifdef CONFIG_AHAB_BOOT
#अन्तर्भाव <asm/mach-imx/ahab.h>
#endif

__weak bool arch_check_dst_in_secure(शून्य *start, ulong size)
{
	फर्काऊ false;
}

__weak शून्य *arch_get_container_trampoline(शून्य)
{
	फर्काऊ रिक्त;
}

स्थिर संरचना boot_img_t *read_auth_image(संरचना spl_image_info *spl_image,
					  संरचना spl_load_info *info,
					  संरचना container_hdr *container,
					  पूर्णाङ्क image_index,
					  ulong container_offset)
{
	संरचना boot_img_t *images;
	ulong offset, size;
	शून्य *buf, *trampoline;

	यदि (image_index > container->अङ्क_images) {
		debug("Invalid image number\n");
		फर्काऊ रिक्त;
	}

	images = (संरचना boot_img_t *)((u8 *)container +
				       परिमाण(संरचना container_hdr));

	यदि (!IS_ALIGNED(images[image_index].offset, spl_get_bl_len(info))) {
		म_लिखो("%s: image%d offset not aligned to %u\n",
		       __func__, image_index, spl_get_bl_len(info));
		फर्काऊ रिक्त;
	}

	size = ALIGN(images[image_index].size, spl_get_bl_len(info));
	offset = images[image_index].offset + container_offset;

	debug("%s: container: %p offset: %lu size: %lu\n", __func__,
	      container, offset, size);

	buf = map_sysmem(images[image_index].dst, images[image_index].size);
	यदि (IS_ENABLED(CONFIG_SPL_IMX_CONTAINER_USE_TRAMPOLINE) &&
	    arch_check_dst_in_secure(buf, size)) {
		trampoline = arch_get_container_trampoline();
		यदि (!trampoline) {
			म_लिखो("%s: trampoline size is zero\n", __func__);
			फर्काऊ रिक्त;
		}

		यदि (info->पठ(info, offset, size, trampoline) < images[image_index].size) {
			म_लिखो("%s: failed to load image to a trampoline buffer\n", __func__);
			फर्काऊ रिक्त;
		}

		स_नकल(buf, trampoline, images[image_index].size);
	} अन्यथा {
		यदि (info->पठ(info, offset, size, buf) < images[image_index].size) {
				म_लिखो("%s: failed to load image to a non-secure region\n", __func__);
			फर्काऊ रिक्त;
		}
	}

#ifdef CONFIG_AHAB_BOOT
	flush_dcache_range((ulong)buf, (ulong)(buf + images[image_index].size - 1));
	यदि (ahab_verify_cntr_image(&images[image_index], image_index))
		फर्काऊ रिक्त;
#endif

	फर्काऊ &images[image_index];
}

स्थिर पूर्णाङ्क read_auth_container(संरचना spl_image_info *spl_image,
			       संरचना spl_load_info *info, ulong offset)
{
	संरचना container_hdr *container = रिक्त;
	संरचना container_hdr *authhdr;
	u16 length;
	पूर्णाङ्क i, size, ret = 0;

	size = ALIGN(CONTAINER_HDR_ALIGNMENT, spl_get_bl_len(info));

	/*
	 * It will not override the ATF code, so safe to use it here,
	 * no need malloc
	 */
	container = दो_स्मृति(size);
	यदि (!container)
		फर्काऊ -ENOMEM;

	debug("%s: container: %p offset: %lu size: %u\n", __func__,
	      container, offset, size);
	यदि (info->पठ(info, offset, size, container) <
	    CONTAINER_HDR_ALIGNMENT) {
		ret = -EIO;
		गच्छ end;
	}

	यदि (!valid_container_hdr(container)) {
		log_err("Wrong container header\n");
		ret = -ENOENT;
		गच्छ end;
	}

	यदि (!container->अङ्क_images) {
		log_err("Wrong container, no image found\n");
		ret = -ENOENT;
		गच्छ end;
	}

	length = container->length_lsb + (container->length_msb << 8);
	debug("Container length %u\n", length);

	यदि (length > CONTAINER_HDR_ALIGNMENT) {
		size = ALIGN(length, spl_get_bl_len(info));

		मुक्त(container);
		container = दो_स्मृति(size);
		यदि (!container)
			फर्काऊ -ENOMEM;

		debug("%s: container: %p offset: %lu size: %u\n",
		      __func__, container, offset, size);
		यदि (info->पठ(info, offset, size, container) < length) {
			ret = -EIO;
			गच्छ end;
		}
	}

	authhdr = container;

#ifdef CONFIG_AHAB_BOOT
	authhdr = ahab_auth_cntr_hdr(authhdr, length);
	यदि (!authhdr) {
		ret = -EINVAL;
		गच्छ end_auth;
	}
#endif

	लागि (i = 0; i < authhdr->अङ्क_images; i++) {
		संरचना boot_img_t *image = read_auth_image(spl_image, info,
							   authhdr, i,
							   offset);

		यदि (!image) {
			ret = -EINVAL;
			गच्छ end_auth;
		}

		यदि (i == 0) {
			spl_image->load_addr = image->dst;
			spl_image->entry_point = image->entry;
		}
	}

end_auth:
#ifdef CONFIG_AHAB_BOOT
	ahab_auth_release();
#endif

end:
	मुक्त(container);

	फर्काऊ ret;
}

पूर्णाङ्क spl_load_imx_container(संरचना spl_image_info *spl_image,
			   संरचना spl_load_info *info, ulong offset)
{
	फर्काऊ read_auth_container(spl_image, info, offset);
}
