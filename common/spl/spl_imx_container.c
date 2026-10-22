// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2021 NXP
 */

#define LOG_CATEGORY LOGC_ARCH
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <imx_container.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <mapmem.h>
#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <cpu_func.h>
#ifdef CONFIG_AHAB_BOOT
#ଅନ୍ତର୍ଭାଵ <asm/mach-imx/ahab.h>
#endif

__weak bool arch_check_dst_in_secure(ଶୂନ୍ୟ *start, ulong size)
{
	ଫେରନ୍ତୁ false;
}

__weak ଶୂନ୍ୟ *arch_get_container_trampoline(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ ଖାଲି;
}

ସ୍ଥିର ସଂରଚନା boot_img_t *read_auth_image(ସଂରଚନା spl_image_info *spl_image,
					  ସଂରଚନା spl_load_info *info,
					  ସଂରଚନା container_hdr *container,
					  ପୂର୍ଣ୍ଣସଂଖ୍ୟା image_index,
					  ulong container_offset)
{
	ସଂରଚନା boot_img_t *images;
	ulong offset, size;
	ଶୂନ୍ୟ *buf, *trampoline;

	ଯଦି (image_index > container->ଅଙ୍କ_images) {
		debug("Invalid image number\n");
		ଫେରନ୍ତୁ ଖାଲି;
	}

	images = (ସଂରଚନା boot_img_t *)((u8 *)container +
				       ଆକାର(ସଂରଚନା container_hdr));

	ଯଦି (!IS_ALIGNED(images[image_index].offset, spl_get_bl_len(info))) {
		printf("%s: image%d offset not aligned to %u\n",
		       __func__, image_index, spl_get_bl_len(info));
		ଫେରନ୍ତୁ ଖାଲି;
	}

	size = ALIGN(images[image_index].size, spl_get_bl_len(info));
	offset = images[image_index].offset + container_offset;

	debug("%s: container: %p offset: %lu size: %lu\n", __func__,
	      container, offset, size);

	buf = map_sysmem(images[image_index].dst, images[image_index].size);
	ଯଦି (IS_ENABLED(CONFIG_SPL_IMX_CONTAINER_USE_TRAMPOLINE) &&
	    arch_check_dst_in_secure(buf, size)) {
		trampoline = arch_get_container_trampoline();
		ଯଦି (!trampoline) {
			printf("%s: trampoline size is zero\n", __func__);
			ଫେରନ୍ତୁ ଖାଲି;
		}

		ଯଦି (info->ପଠ(info, offset, size, trampoline) < images[image_index].size) {
			printf("%s: failed to load image to a trampoline buffer\n", __func__);
			ଫେରନ୍ତୁ ଖାଲି;
		}

		memcpy(buf, trampoline, images[image_index].size);
	} ଅନ୍ୟଥା {
		ଯଦି (info->ପଠ(info, offset, size, buf) < images[image_index].size) {
				printf("%s: failed to load image to a non-secure region\n", __func__);
			ଫେରନ୍ତୁ ଖାଲି;
		}
	}

#ifdef CONFIG_AHAB_BOOT
	flush_dcache_range((ulong)buf, (ulong)(buf + images[image_index].size - 1));
	ଯଦି (ahab_verify_cntr_image(&images[image_index], image_index))
		ଫେରନ୍ତୁ ଖାଲି;
#endif

	ଫେରନ୍ତୁ &images[image_index];
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା read_auth_container(ସଂରଚନା spl_image_info *spl_image,
			       ସଂରଚନା spl_load_info *info, ulong offset)
{
	ସଂରଚନା container_hdr *container = ଖାଲି;
	ସଂରଚନା container_hdr *authhdr;
	u16 length;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, size, ret = 0;

	size = ALIGN(CONTAINER_HDR_ALIGNMENT, spl_get_bl_len(info));

	/*
	 * It will not override the ATF code, so safe to use it here,
	 * no need malloc
	 */
	container = malloc(size);
	ଯଦି (!container)
		ଫେରନ୍ତୁ -ENOMEM;

	debug("%s: container: %p offset: %lu size: %u\n", __func__,
	      container, offset, size);
	ଯଦି (info->ପଠ(info, offset, size, container) <
	    CONTAINER_HDR_ALIGNMENT) {
		ret = -EIO;
		ଯାଅ end;
	}

	ଯଦି (!valid_container_hdr(container)) {
		log_err("Wrong container header\n");
		ret = -ENOENT;
		ଯାଅ end;
	}

	ଯଦି (!container->ଅଙ୍କ_images) {
		log_err("Wrong container, no image found\n");
		ret = -ENOENT;
		ଯାଅ end;
	}

	length = container->length_lsb + (container->length_msb << 8);
	debug("Container length %u\n", length);

	ଯଦି (length > CONTAINER_HDR_ALIGNMENT) {
		size = ALIGN(length, spl_get_bl_len(info));

		ମୁକ୍ତ(container);
		container = malloc(size);
		ଯଦି (!container)
			ଫେରନ୍ତୁ -ENOMEM;

		debug("%s: container: %p offset: %lu size: %u\n",
		      __func__, container, offset, size);
		ଯଦି (info->ପଠ(info, offset, size, container) < length) {
			ret = -EIO;
			ଯାଅ end;
		}
	}

	authhdr = container;

#ifdef CONFIG_AHAB_BOOT
	authhdr = ahab_auth_cntr_hdr(authhdr, length);
	ଯଦି (!authhdr) {
		ret = -EINVAL;
		ଯାଅ end_auth;
	}
#endif

	ପାଇଁ (i = 0; i < authhdr->ଅଙ୍କ_images; i++) {
		ସଂରଚନା boot_img_t *image = read_auth_image(spl_image, info,
							   authhdr, i,
							   offset);

		ଯଦି (!image) {
			ret = -EINVAL;
			ଯାଅ end_auth;
		}

		ଯଦି (i == 0) {
			spl_image->load_addr = image->dst;
			spl_image->entry_point = image->entry;
		}
	}

end_auth:
#ifdef CONFIG_AHAB_BOOT
	ahab_auth_release();
#endif

end:
	ମୁକ୍ତ(container);

	ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_load_imx_container(ସଂରଚନା spl_image_info *spl_image,
			   ସଂରଚନା spl_load_info *info, ulong offset)
{
	ଫେରନ୍ତୁ read_auth_container(spl_image, info, offset);
}
