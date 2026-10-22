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

__weak bool arch_check_dst_in_secure(শূন্য *start, ulong আকার_2)
{
	ফেরত false;
}

__weak শূন্য *arch_get_container_trampoline(শূন্য)
{
	ফেরত ফাঁকা;
}

স্থির কাঠামো boot_img_t *read_auth_image(কাঠামো spl_image_info *spl_image,
					  কাঠামো spl_load_info *info,
					  কাঠামো container_hdr *container,
					  পূর্ণসংখ্যা image_সূচক,
					  ulong container_offset)
{
	কাঠামো boot_img_t *images;
	ulong offset, আকার_2;
	শূন্য *buf, *trampoline;

	যদি (image_সূচক > container->অঙ্ক_images) {
		debug("Invalid image number\n");
		ফেরত ফাঁকা;
	}

	images = (কাঠামো boot_img_t *)((u8 *)container +
				       আকার(কাঠামো container_hdr));

	যদি (!IS_ALIGNED(images[image_সূচক].offset, spl_get_bl_len(info))) {
		printf("%s: image%d offset not aligned to %u\n",
		       __func__, image_সূচক, spl_get_bl_len(info));
		ফেরত ফাঁকা;
	}

	আকার_2 = ALIGN(images[image_সূচক].আকার_2, spl_get_bl_len(info));
	offset = images[image_সূচক].offset + container_offset;

	debug("%s: container: %p offset: %lu size: %lu\n", __func__,
	      container, offset, আকার_2);

	buf = map_sysmem(images[image_সূচক].dst, images[image_সূচক].আকার_2);
	যদি (IS_ENABLED(CONFIG_SPL_IMX_CONTAINER_USE_TRAMPOLINE) &&
	    arch_check_dst_in_secure(buf, আকার_2)) {
		trampoline = arch_get_container_trampoline();
		যদি (!trampoline) {
			printf("%s: trampoline size is zero\n", __func__);
			ফেরত ফাঁকা;
		}

		যদি (info->পঠ(info, offset, আকার_2, trampoline) < images[image_সূচক].আকার_2) {
			printf("%s: failed to load image to a trampoline buffer\n", __func__);
			ফেরত ফাঁকা;
		}

		memcpy(buf, trampoline, images[image_সূচক].আকার_2);
	} অন্যথা {
		যদি (info->পঠ(info, offset, আকার_2, buf) < images[image_সূচক].আকার_2) {
				printf("%s: failed to load image to a non-secure region\n", __func__);
			ফেরত ফাঁকা;
		}
	}

#ifdef CONFIG_AHAB_BOOT
	flush_dcache_range((ulong)buf, (ulong)(buf + images[image_সূচক].আকার_2 - 1));
	যদি (ahab_verify_cntr_image(&images[image_সূচক], image_সূচক))
		ফেরত ফাঁকা;
#endif

	ফেরত &images[image_সূচক];
}

স্থির পূর্ণসংখ্যা read_auth_container(কাঠামো spl_image_info *spl_image,
			       কাঠামো spl_load_info *info, ulong offset)
{
	কাঠামো container_hdr *container = ফাঁকা;
	কাঠামো container_hdr *authhdr;
	u16 দৈর্ঘ্য;
	পূর্ণসংখ্যা i, আকার_2, ret = 0;

	আকার_2 = ALIGN(CONTAINER_HDR_ALIGNMENT, spl_get_bl_len(info));

	/*
	 * It will not override the ATF code, so safe to use it here,
	 * no need malloc
	 */
	container = malloc(আকার_2);
	যদি (!container)
		ফেরত -ENOMEM;

	debug("%s: container: %p offset: %lu size: %u\n", __func__,
	      container, offset, আকার_2);
	যদি (info->পঠ(info, offset, আকার_2, container) <
	    CONTAINER_HDR_ALIGNMENT) {
		ret = -EIO;
		যাও end;
	}

	যদি (!valid_container_hdr(container)) {
		log_err("Wrong container header\n");
		ret = -ENOENT;
		যাও end;
	}

	যদি (!container->অঙ্ক_images) {
		log_err("Wrong container, no image found\n");
		ret = -ENOENT;
		যাও end;
	}

	দৈর্ঘ্য = container->দৈর্ঘ্য_lsb + (container->দৈর্ঘ্য_msb << 8);
	debug("Container length %u\n", দৈর্ঘ্য);

	যদি (দৈর্ঘ্য > CONTAINER_HDR_ALIGNMENT) {
		আকার_2 = ALIGN(দৈর্ঘ্য, spl_get_bl_len(info));

		মুক্ত(container);
		container = malloc(আকার_2);
		যদি (!container)
			ফেরত -ENOMEM;

		debug("%s: container: %p offset: %lu size: %u\n",
		      __func__, container, offset, আকার_2);
		যদি (info->পঠ(info, offset, আকার_2, container) < দৈর্ঘ্য) {
			ret = -EIO;
			যাও end;
		}
	}

	authhdr = container;

#ifdef CONFIG_AHAB_BOOT
	authhdr = ahab_auth_cntr_hdr(authhdr, দৈর্ঘ্য);
	যদি (!authhdr) {
		ret = -EINVAL;
		যাও end_auth;
	}
#endif

	জন্য (i = 0; i < authhdr->অঙ্ক_images; i++) {
		কাঠামো boot_img_t *image = read_auth_image(spl_image, info,
							   authhdr, i,
							   offset);

		যদি (!image) {
			ret = -EINVAL;
			যাও end_auth;
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

	ফেরত ret;
}

পূর্ণসংখ্যা spl_load_imx_container(কাঠামো spl_image_info *spl_image,
			   কাঠামো spl_load_info *info, ulong offset)
{
	ফেরত read_auth_container(spl_image, info, offset);
}
