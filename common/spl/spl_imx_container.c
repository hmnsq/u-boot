// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2021 NXP
 */

#تعریف_کن LOG_CATEGORY LOGC_ARCH
#ضمن <stdlib.h>
#ضمن <errno.h>
#ضمن <imx_container.h>
#ضمن <log.h>
#ضمن <mapmem.h>
#ضمن <spl.h>
#ضمن <cpu_func.h>
#اگر_تعریف_شده CONFIG_AHAB_BOOT
#ضمن <asm/mach-imx/ahab.h>
#پایان_اگر

__weak bool arch_check_dst_درون_secure(فراغ *شروع, ulong اندازه)
{
	أرجع false;
}

__weak فراغ *arch_get_container_trampoline(فراغ)
{
	أرجع عدم;
}

ساكن بنية boot_img_t *read_auth_image(بنية spl_image_info *spl_image,
					  بنية spl_بارگیری_info *info,
					  بنية container_hdr *container,
					  صحيح image_شاخص,
					  ulong container_جابه_جایی)
{
	بنية boot_img_t *images;
	ulong جابه_جایی, اندازه;
	فراغ *buf, *trampoline;

	إذا (image_شاخص > container->رقم_images) {
		debug("Invalid image number\n");
		أرجع عدم;
	}

	images = (بنية boot_img_t *)((u8 *)container +
				       حجم(بنية container_hdr));

	إذا (!IS_ALIGNED(images[image_شاخص].جابه_جایی, spl_get_bl_len(info))) {
		چاپ_کن("%s: image%d offset not aligned to %u\n",
		       __func__, image_شاخص, spl_get_bl_len(info));
		أرجع عدم;
	}

	اندازه = ALIGN(images[image_شاخص].اندازه, spl_get_bl_len(info));
	جابه_جایی = images[image_شاخص].جابه_جایی + container_جابه_جایی;

	debug("%s: container: %p offset: %lu size: %lu\n", __func__,
	      container, جابه_جایی, اندازه);

	buf = map_sysmem(images[image_شاخص].dst, images[image_شاخص].اندازه);
	إذا (IS_ENABLED(CONFIG_SPL_IMX_CONTAINER_USE_TRAMPOLINE) &&
	    arch_check_dst_درون_secure(buf, اندازه)) {
		trampoline = arch_get_container_trampoline();
		إذا (!trampoline) {
			چاپ_کن("%s: trampoline size is zero\n", __func__);
			أرجع عدم;
		}

		إذا (info->اقرأ(info, جابه_جایی, اندازه, trampoline) < images[image_شاخص].اندازه) {
			چاپ_کن("%s: failed to load image to a trampoline buffer\n", __func__);
			أرجع عدم;
		}

		کپی_حافظه(buf, trampoline, images[image_شاخص].اندازه);
	} وإلا {
		إذا (info->اقرأ(info, جابه_جایی, اندازه, buf) < images[image_شاخص].اندازه) {
				چاپ_کن("%s: failed to load image to a non-secure region\n", __func__);
			أرجع عدم;
		}
	}

#اگر_تعریف_شده CONFIG_AHAB_BOOT
	flush_dcache_range((ulong)buf, (ulong)(buf + images[image_شاخص].اندازه - 1));
	إذا (ahab_verify_cntr_image(&images[image_شاخص], image_شاخص))
		أرجع عدم;
#پایان_اگر

	أرجع &images[image_شاخص];
}

ساكن صحيح read_auth_container(بنية spl_image_info *spl_image,
			       بنية spl_بارگیری_info *info, ulong جابه_جایی)
{
	بنية container_hdr *container = عدم;
	بنية container_hdr *authhdr;
	u16 درازی;
	صحيح i, اندازه, ret = 0;

	اندازه = ALIGN(CONTAINER_HDR_ALIGNMENT, spl_get_bl_len(info));

	/*
	 * It will not override the ATF code, so safe to use it here,
	 * no need malloc
	 */
	container = حافظه_بده(اندازه);
	إذا (!container)
		أرجع -ENOMEM;

	debug("%s: container: %p offset: %lu size: %u\n", __func__,
	      container, جابه_جایی, اندازه);
	إذا (info->اقرأ(info, جابه_جایی, اندازه, container) <
	    CONTAINER_HDR_ALIGNMENT) {
		ret = -EIO;
		اذهب انتها_2;
	}

	إذا (!valid_container_hdr(container)) {
		log_err("Wrong container header\n");
		ret = -ENOENT;
		اذهب انتها_2;
	}

	إذا (!container->رقم_images) {
		log_err("Wrong container, no image found\n");
		ret = -ENOENT;
		اذهب انتها_2;
	}

	درازی = container->درازی_lsb + (container->درازی_msb << 8);
	debug("Container length %u\n", درازی);

	إذا (درازی > CONTAINER_HDR_ALIGNMENT) {
		اندازه = ALIGN(درازی, spl_get_bl_len(info));

		حرر(container);
		container = حافظه_بده(اندازه);
		إذا (!container)
			أرجع -ENOMEM;

		debug("%s: container: %p offset: %lu size: %u\n",
		      __func__, container, جابه_جایی, اندازه);
		إذا (info->اقرأ(info, جابه_جایی, اندازه, container) < درازی) {
			ret = -EIO;
			اذهب انتها_2;
		}
	}

	authhdr = container;

#اگر_تعریف_شده CONFIG_AHAB_BOOT
	authhdr = ahab_auth_cntr_hdr(authhdr, درازی);
	إذا (!authhdr) {
		ret = -EINVAL;
		اذهب انتها_auth;
	}
#پایان_اگر

	لكل (i = 0; i < authhdr->رقم_images; i++) {
		بنية boot_img_t *image = read_auth_image(spl_image, info,
							   authhdr, i,
							   جابه_جایی);

		إذا (!image) {
			ret = -EINVAL;
			اذهب انتها_auth;
		}

		إذا (i == 0) {
			spl_image->بارگیری_addr = image->dst;
			spl_image->مدخل_نقطه = image->مدخل;
		}
	}

انتها_auth:
#اگر_تعریف_شده CONFIG_AHAB_BOOT
	ahab_auth_release();
#پایان_اگر

انتها_2:
	حرر(container);

	أرجع ret;
}

صحيح spl_بارگیری_imx_container(بنية spl_image_info *spl_image,
			   بنية spl_بارگیری_info *info, ulong جابه_جایی)
{
	أرجع read_auth_container(spl_image, info, جابه_جایی);
}
