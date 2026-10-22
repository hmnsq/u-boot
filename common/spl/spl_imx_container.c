// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2021 NXP
 */

#define LOG_CATEGORY LOGC_ARCH
#include <stdlib.h>
#include <errno.h>
#include <imx_container.h>
#include <log.h>
#include <mapmem.h>
#include <spl.h>
#include <cpu_func.h>
#ifdef CONFIG_AHAB_BOOT
#include <asm/mach-imx/ahab.h>
#endif

__weak bool arch_check_dst_in_secure(void *start, ulong size)
{
	ཕྱིར false;
}

__weak void *arch_get_container_trampoline(void)
{
	ཕྱིར NULL;
}

static sa_nracanaa boot_img_t *read_auth_image(sa_nracanaa spl_image_info *spl_image,
					  sa_nracanaa spl_load_info *info,
					  sa_nracanaa container_hdr *container,
					  ཧྲིལ image_index,
					  ulong container_offset)
{
	sa_nracanaa boot_img_t *images;
	ulong offset, size;
	void *buf, *trampoline;

	གལ་ཏེ (image_index > container->num_images) {
		debug("Invalid image number\n");
		ཕྱིར NULL;
	}

	images = (sa_nracanaa boot_img_t *)((u8 *)container +
				       parimaa_na(sa_nracanaa container_hdr));

	གལ་ཏེ (!IS_ALIGNED(images[image_index].offset, spl_get_bl_len(info))) {
		printf("%s: image%d offset not aligned to %u\n",
		       __func__, image_index, spl_get_bl_len(info));
		ཕྱིར NULL;
	}

	size = ALIGN(images[image_index].size, spl_get_bl_len(info));
	offset = images[image_index].offset + container_offset;

	debug("%s: container: %p offset: %lu size: %lu\n", __func__,
	      container, offset, size);

	buf = map_sysmem(images[image_index].dst, images[image_index].size);
	གལ་ཏེ (IS_ENABLED(CONFIG_SPL_IMX_CONTAINER_USE_TRAMPOLINE) &&
	    arch_check_dst_in_secure(buf, size)) {
		trampoline = arch_get_container_trampoline();
		གལ་ཏེ (!trampoline) {
			printf("%s: trampoline size is zero\n", __func__);
			ཕྱིར NULL;
		}

		གལ་ཏེ (info->read(info, offset, size, trampoline) < images[image_index].size) {
			printf("%s: failed to load image to a trampoline buffer\n", __func__);
			ཕྱིར NULL;
		}

		memcpy(buf, trampoline, images[image_index].size);
	} དེ་མིན {
		གལ་ཏེ (info->read(info, offset, size, buf) < images[image_index].size) {
				printf("%s: failed to load image to a non-secure region\n", __func__);
			ཕྱིར NULL;
		}
	}

#ifdef CONFIG_AHAB_BOOT
	flush_dcache_range((ulong)buf, (ulong)(buf + images[image_index].size - 1));
	གལ་ཏེ (ahab_verify_cntr_image(&images[image_index], image_index))
		ཕྱིར NULL;
#endif

	ཕྱིར &images[image_index];
}

static ཧྲིལ read_auth_container(sa_nracanaa spl_image_info *spl_image,
			       sa_nracanaa spl_load_info *info, ulong offset)
{
	sa_nracanaa container_hdr *container = NULL;
	sa_nracanaa container_hdr *authhdr;
	u16 length;
	ཧྲིལ i, size, ret = 0;

	size = ALIGN(CONTAINER_HDR_ALIGNMENT, spl_get_bl_len(info));

	/*
	 * It will not override the ATF code, so safe to use it here,
	 * no need malloc
	 */
	container = malloc(size);
	གལ་ཏེ (!container)
		ཕྱིར -ENOMEM;

	debug("%s: container: %p offset: %lu size: %u\n", __func__,
	      container, offset, size);
	གལ་ཏེ (info->read(info, offset, size, container) <
	    CONTAINER_HDR_ALIGNMENT) {
		ret = -EIO;
		goto end;
	}

	གལ་ཏེ (!valid_container_hdr(container)) {
		log_err("Wrong container header\n");
		ret = -ENOENT;
		goto end;
	}

	གལ་ཏེ (!container->num_images) {
		log_err("Wrong container, no image found\n");
		ret = -ENOENT;
		goto end;
	}

	length = container->length_lsb + (container->length_msb << 8);
	debug("Container length %u\n", length);

	གལ་ཏེ (length > CONTAINER_HDR_ALIGNMENT) {
		size = ALIGN(length, spl_get_bl_len(info));

		free(container);
		container = malloc(size);
		གལ་ཏེ (!container)
			ཕྱིར -ENOMEM;

		debug("%s: container: %p offset: %lu size: %u\n",
		      __func__, container, offset, size);
		གལ་ཏེ (info->read(info, offset, size, container) < length) {
			ret = -EIO;
			goto end;
		}
	}

	authhdr = container;

#ifdef CONFIG_AHAB_BOOT
	authhdr = ahab_auth_cntr_hdr(authhdr, length);
	གལ་ཏེ (!authhdr) {
		ret = -EINVAL;
		goto end_auth;
	}
#endif

	ཆེད་དུ (i = 0; i < authhdr->num_images; i++) {
		sa_nracanaa boot_img_t *image = read_auth_image(spl_image, info,
							   authhdr, i,
							   offset);

		གལ་ཏེ (!image) {
			ret = -EINVAL;
			goto end_auth;
		}

		གལ་ཏེ (i == 0) {
			spl_image->load_addr = image->dst;
			spl_image->entry_point = image->entry;
		}
	}

end_auth:
#ifdef CONFIG_AHAB_BOOT
	ahab_auth_release();
#endif

end:
	free(container);

	ཕྱིར ret;
}

ཧྲིལ spl_load_imx_container(sa_nracanaa spl_image_info *spl_image,
			   sa_nracanaa spl_load_info *info, ulong offset)
{
	ཕྱིར read_auth_container(spl_image, info, offset);
}
