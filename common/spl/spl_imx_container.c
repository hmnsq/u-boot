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
	반환 false;
}

__weak void *arch_get_container_trampoline(void)
{
	반환 NULL;
}

static struct boot_img_t *read_auth_image(struct spl_image_info *spl_image,
					  struct spl_load_info *info,
					  struct container_hdr *container,
					  정수 image_index,
					  ulong container_offset)
{
	struct boot_img_t *images;
	ulong offset, size;
	void *buf, *trampoline;

	만약 (image_index > container->num_images) {
		debug("Invalid image number\n");
		반환 NULL;
	}

	images = (struct boot_img_t *)((u8 *)container +
				       sizeof(struct container_hdr));

	만약 (!IS_ALIGNED(images[image_index].offset, spl_get_bl_len(info))) {
		printf("%s: image%d offset not aligned to %u\n",
		       __func__, image_index, spl_get_bl_len(info));
		반환 NULL;
	}

	size = ALIGN(images[image_index].size, spl_get_bl_len(info));
	offset = images[image_index].offset + container_offset;

	debug("%s: container: %p offset: %lu size: %lu\n", __func__,
	      container, offset, size);

	buf = map_sysmem(images[image_index].dst, images[image_index].size);
	만약 (IS_ENABLED(CONFIG_SPL_IMX_CONTAINER_USE_TRAMPOLINE) &&
	    arch_check_dst_in_secure(buf, size)) {
		trampoline = arch_get_container_trampoline();
		만약 (!trampoline) {
			printf("%s: trampoline size is zero\n", __func__);
			반환 NULL;
		}

		만약 (info->read(info, offset, size, trampoline) < images[image_index].size) {
			printf("%s: failed to load image to a trampoline buffer\n", __func__);
			반환 NULL;
		}

		memcpy(buf, trampoline, images[image_index].size);
	} 아니면 {
		만약 (info->read(info, offset, size, buf) < images[image_index].size) {
				printf("%s: failed to load image to a non-secure region\n", __func__);
			반환 NULL;
		}
	}

#ifdef CONFIG_AHAB_BOOT
	flush_dcache_range((ulong)buf, (ulong)(buf + images[image_index].size - 1));
	만약 (ahab_verify_cntr_image(&images[image_index], image_index))
		반환 NULL;
#endif

	반환 &images[image_index];
}

static 정수 read_auth_container(struct spl_image_info *spl_image,
			       struct spl_load_info *info, ulong offset)
{
	struct container_hdr *container = NULL;
	struct container_hdr *authhdr;
	u16 length;
	정수 i, size, ret = 0;

	size = ALIGN(CONTAINER_HDR_ALIGNMENT, spl_get_bl_len(info));

	/*
	 * It will not override the ATF code, so safe to use it here,
	 * no need malloc
	 */
	container = malloc(size);
	만약 (!container)
		반환 -ENOMEM;

	debug("%s: container: %p offset: %lu size: %u\n", __func__,
	      container, offset, size);
	만약 (info->read(info, offset, size, container) <
	    CONTAINER_HDR_ALIGNMENT) {
		ret = -EIO;
		goto end;
	}

	만약 (!valid_container_hdr(container)) {
		log_err("Wrong container header\n");
		ret = -ENOENT;
		goto end;
	}

	만약 (!container->num_images) {
		log_err("Wrong container, no image found\n");
		ret = -ENOENT;
		goto end;
	}

	length = container->length_lsb + (container->length_msb << 8);
	debug("Container length %u\n", length);

	만약 (length > CONTAINER_HDR_ALIGNMENT) {
		size = ALIGN(length, spl_get_bl_len(info));

		free(container);
		container = malloc(size);
		만약 (!container)
			반환 -ENOMEM;

		debug("%s: container: %p offset: %lu size: %u\n",
		      __func__, container, offset, size);
		만약 (info->read(info, offset, size, container) < length) {
			ret = -EIO;
			goto end;
		}
	}

	authhdr = container;

#ifdef CONFIG_AHAB_BOOT
	authhdr = ahab_auth_cntr_hdr(authhdr, length);
	만약 (!authhdr) {
		ret = -EINVAL;
		goto end_auth;
	}
#endif

	위해 (i = 0; i < authhdr->num_images; i++) {
		struct boot_img_t *image = read_auth_image(spl_image, info,
							   authhdr, i,
							   offset);

		만약 (!image) {
			ret = -EINVAL;
			goto end_auth;
		}

		만약 (i == 0) {
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

	반환 ret;
}

정수 spl_load_imx_container(struct spl_image_info *spl_image,
			   struct spl_load_info *info, ulong offset)
{
	반환 read_auth_container(spl_image, info, offset);
}
