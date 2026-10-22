// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2021 NXP
 */

#تعریف_کرو LOG_CATEGORY LOGC_ARCH
#ضمن <stdlib.h>
#ضمن <errno.h>
#ضمن <imx_container.h>
#ضمن <log.h>
#ضمن <mapmem.h>
#ضمن <spl.h>
#ضمن <cpu_func.h>
#جے_تعریف_شدہ CONFIG_AHAB_BOOT
#ضمن <asm/mach-imx/ahab.h>
#جے_انت

__weak bool arch_check_dst_وچ_secure(فراغ *شروع, ulong ناپ)
{
	أرجع false;
}

__weak فراغ *arch_get_container_trampoline(فراغ)
{
	أرجع عدم;
}

ساكن بنية boot_img_t *read_auth_image(بنية spl_image_info *spl_image,
					  بنية spl_لدو_info *info,
					  بنية container_hdr *container,
					  صحيح image_اشاریہ,
					  ulong container_وتھ)
{
	بنية boot_img_t *images;
	ulong وتھ, ناپ;
	فراغ *buf, *trampoline;

	إذا (image_اشاریہ > container->عدد_images) {
		debug("Invalid image number\n");
		أرجع عدم;
	}

	images = (بنية boot_img_t *)((u8 *)container +
				       حجم(بنية container_hdr));

	إذا (!IS_ALIGNED(images[image_اشاریہ].وتھ, spl_get_bl_len(info))) {
		لکھو("%s: image%d offset not aligned to %u\n",
		       __func__, image_اشاریہ, spl_get_bl_len(info));
		أرجع عدم;
	}

	ناپ = ALIGN(images[image_اشاریہ].ناپ, spl_get_bl_len(info));
	وتھ = images[image_اشاریہ].وتھ + container_وتھ;

	debug("%s: container: %p offset: %lu size: %lu\n", __func__,
	      container, وتھ, ناپ);

	buf = map_sysmem(images[image_اشاریہ].dst, images[image_اشاریہ].ناپ);
	إذا (IS_ENABLED(CONFIG_SPL_IMX_CONTAINER_USE_TRAMPOLINE) &&
	    arch_check_dst_وچ_secure(buf, ناپ)) {
		trampoline = arch_get_container_trampoline();
		إذا (!trampoline) {
			لکھو("%s: trampoline size is zero\n", __func__);
			أرجع عدم;
		}

		إذا (info->اقرأ(info, وتھ, ناپ, trampoline) < images[image_اشاریہ].ناپ) {
			لکھو("%s: failed to load image to a trampoline buffer\n", __func__);
			أرجع عدم;
		}

		حافظہ_نقل(buf, trampoline, images[image_اشاریہ].ناپ);
	} وإلا {
		إذا (info->اقرأ(info, وتھ, ناپ, buf) < images[image_اشاریہ].ناپ) {
				لکھو("%s: failed to load image to a non-secure region\n", __func__);
			أرجع عدم;
		}
	}

#جے_تعریف_شدہ CONFIG_AHAB_BOOT
	flush_dcache_range((ulong)buf, (ulong)(buf + images[image_اشاریہ].ناپ - 1));
	إذا (ahab_verify_cntr_image(&images[image_اشاریہ], image_اشاریہ))
		أرجع عدم;
#جے_انت

	أرجع &images[image_اشاریہ];
}

ساكن صحيح read_auth_container(بنية spl_image_info *spl_image,
			       بنية spl_لدو_info *info, ulong وتھ)
{
	بنية container_hdr *container = عدم;
	بنية container_hdr *authhdr;
	u16 لمائی;
	صحيح i, ناپ, ret = 0;

	ناپ = ALIGN(CONTAINER_HDR_ALIGNMENT, spl_get_bl_len(info));

	/*
	 * It will not override the ATF code, so safe to use it here,
	 * no need malloc
	 */
	container = حافظہ_دیو(ناپ);
	إذا (!container)
		أرجع -ENOMEM;

	debug("%s: container: %p offset: %lu size: %u\n", __func__,
	      container, وتھ, ناپ);
	إذا (info->اقرأ(info, وتھ, ناپ, container) <
	    CONTAINER_HDR_ALIGNMENT) {
		ret = -EIO;
		اذهب سرا_2;
	}

	إذا (!valid_container_hdr(container)) {
		log_err("Wrong container header\n");
		ret = -ENOENT;
		اذهب سرا_2;
	}

	إذا (!container->عدد_images) {
		log_err("Wrong container, no image found\n");
		ret = -ENOENT;
		اذهب سرا_2;
	}

	لمائی = container->لمائی_lsb + (container->لمائی_msb << 8);
	debug("Container length %u\n", لمائی);

	إذا (لمائی > CONTAINER_HDR_ALIGNMENT) {
		ناپ = ALIGN(لمائی, spl_get_bl_len(info));

		حرر(container);
		container = حافظہ_دیو(ناپ);
		إذا (!container)
			أرجع -ENOMEM;

		debug("%s: container: %p offset: %lu size: %u\n",
		      __func__, container, وتھ, ناپ);
		إذا (info->اقرأ(info, وتھ, ناپ, container) < لمائی) {
			ret = -EIO;
			اذهب سرا_2;
		}
	}

	authhdr = container;

#جے_تعریف_شدہ CONFIG_AHAB_BOOT
	authhdr = ahab_auth_cntr_hdr(authhdr, لمائی);
	إذا (!authhdr) {
		ret = -EINVAL;
		اذهب سرا_auth;
	}
#جے_انت

	لكل (i = 0; i < authhdr->عدد_images; i++) {
		بنية boot_img_t *image = read_auth_image(spl_image, info,
							   authhdr, i,
							   وتھ);

		إذا (!image) {
			ret = -EINVAL;
			اذهب سرا_auth;
		}

		إذا (i == 0) {
			spl_image->لدو_addr = image->dst;
			spl_image->اندراج_نقطہ = image->اندراج;
		}
	}

سرا_auth:
#جے_تعریف_شدہ CONFIG_AHAB_BOOT
	ahab_auth_release();
#جے_انت

سرا_2:
	حرر(container);

	أرجع ret;
}

صحيح spl_لدو_imx_container(بنية spl_image_info *spl_image,
			   بنية spl_لدو_info *info, ulong وتھ)
{
	أرجع read_auth_container(spl_image, info, وتھ);
}
