// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2021 NXP
 */

#define LOG_CATEGORY LOGC_ARCH
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <imx_container.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <mapmem.h>
#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <cpu_func.h>
#ifdef CONFIG_AHAB_BOOT
#અન્તર્ભાવ <asm/mach-imx/ahab.h>
#endif

__weak bool arch_check_dst_in_secure(શૂન્ય *start, ulong size)
{
	પાછા false;
}

__weak શૂન્ય *arch_get_container_trampoline(શૂન્ય)
{
	પાછા ખાલી;
}

સ્થિર સંરચના boot_img_t *read_auth_image(સંરચના spl_image_info *spl_image,
					  સંરચના spl_load_info *info,
					  સંરચના container_hdr *container,
					  પૂર્ણાંક image_index,
					  ulong container_offset)
{
	સંરચના boot_img_t *images;
	ulong offset, size;
	શૂન્ય *buf, *trampoline;

	જો (image_index > container->અંક_images) {
		debug("Invalid image number\n");
		પાછા ખાલી;
	}

	images = (સંરચના boot_img_t *)((u8 *)container +
				       કદ(સંરચના container_hdr));

	જો (!IS_ALIGNED(images[image_index].offset, spl_get_bl_len(info))) {
		printf("%s: image%d offset not aligned to %u\n",
		       __func__, image_index, spl_get_bl_len(info));
		પાછા ખાલી;
	}

	size = ALIGN(images[image_index].size, spl_get_bl_len(info));
	offset = images[image_index].offset + container_offset;

	debug("%s: container: %p offset: %lu size: %lu\n", __func__,
	      container, offset, size);

	buf = map_sysmem(images[image_index].dst, images[image_index].size);
	જો (IS_ENABLED(CONFIG_SPL_IMX_CONTAINER_USE_TRAMPOLINE) &&
	    arch_check_dst_in_secure(buf, size)) {
		trampoline = arch_get_container_trampoline();
		જો (!trampoline) {
			printf("%s: trampoline size is zero\n", __func__);
			પાછા ખાલી;
		}

		જો (info->પઠ(info, offset, size, trampoline) < images[image_index].size) {
			printf("%s: failed to load image to a trampoline buffer\n", __func__);
			પાછા ખાલી;
		}

		memcpy(buf, trampoline, images[image_index].size);
	} નહીંતર {
		જો (info->પઠ(info, offset, size, buf) < images[image_index].size) {
				printf("%s: failed to load image to a non-secure region\n", __func__);
			પાછા ખાલી;
		}
	}

#ifdef CONFIG_AHAB_BOOT
	flush_dcache_range((ulong)buf, (ulong)(buf + images[image_index].size - 1));
	જો (ahab_verify_cntr_image(&images[image_index], image_index))
		પાછા ખાલી;
#endif

	પાછા &images[image_index];
}

સ્થિર પૂર્ણાંક read_auth_container(સંરચના spl_image_info *spl_image,
			       સંરચના spl_load_info *info, ulong offset)
{
	સંરચના container_hdr *container = ખાલી;
	સંરચના container_hdr *authhdr;
	u16 length;
	પૂર્ણાંક i, size, ret = 0;

	size = ALIGN(CONTAINER_HDR_ALIGNMENT, spl_get_bl_len(info));

	/*
	 * It will not override the ATF code, so safe to use it here,
	 * no need malloc
	 */
	container = malloc(size);
	જો (!container)
		પાછા -ENOMEM;

	debug("%s: container: %p offset: %lu size: %u\n", __func__,
	      container, offset, size);
	જો (info->પઠ(info, offset, size, container) <
	    CONTAINER_HDR_ALIGNMENT) {
		ret = -EIO;
		જાઓ end;
	}

	જો (!valid_container_hdr(container)) {
		log_err("Wrong container header\n");
		ret = -ENOENT;
		જાઓ end;
	}

	જો (!container->અંક_images) {
		log_err("Wrong container, no image found\n");
		ret = -ENOENT;
		જાઓ end;
	}

	length = container->length_lsb + (container->length_msb << 8);
	debug("Container length %u\n", length);

	જો (length > CONTAINER_HDR_ALIGNMENT) {
		size = ALIGN(length, spl_get_bl_len(info));

		મુક્ત(container);
		container = malloc(size);
		જો (!container)
			પાછા -ENOMEM;

		debug("%s: container: %p offset: %lu size: %u\n",
		      __func__, container, offset, size);
		જો (info->પઠ(info, offset, size, container) < length) {
			ret = -EIO;
			જાઓ end;
		}
	}

	authhdr = container;

#ifdef CONFIG_AHAB_BOOT
	authhdr = ahab_auth_cntr_hdr(authhdr, length);
	જો (!authhdr) {
		ret = -EINVAL;
		જાઓ end_auth;
	}
#endif

	માટે (i = 0; i < authhdr->અંક_images; i++) {
		સંરચના boot_img_t *image = read_auth_image(spl_image, info,
							   authhdr, i,
							   offset);

		જો (!image) {
			ret = -EINVAL;
			જાઓ end_auth;
		}

		જો (i == 0) {
			spl_image->load_addr = image->dst;
			spl_image->entry_point = image->entry;
		}
	}

end_auth:
#ifdef CONFIG_AHAB_BOOT
	ahab_auth_release();
#endif

end:
	મુક્ત(container);

	પાછા ret;
}

પૂર્ણાંક spl_load_imx_container(સંરચના spl_image_info *spl_image,
			   સંરચના spl_load_info *info, ulong offset)
{
	પાછા read_auth_container(spl_image, info, offset);
}
