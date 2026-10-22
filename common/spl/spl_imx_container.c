// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2021 NXP
 */

#تعریف_کړه LOG_CATEGORY LOGC_ARCH
#ضمن <stdlib.h>
#ضمن <errno.h>
#ضمن <imx_container.h>
#ضمن <log.h>
#ضمن <mapmem.h>
#ضمن <spl.h>
#ضمن <cpu_func.h>
#که_تعریف_شوی CONFIG_AHAB_BOOT
#ضمن <asm/mach-imx/ahab.h>
#د_که_پای

__weak bool arch_check_dst_دننه_secure(فراغ *پیل, ulong اندازه)
{
	أرجع false;
}

__weak فراغ *arch_get_container_trampoline(فراغ)
{
	أرجع عدم;
}

ساكن بنية boot_img_t *read_auth_image(بنية spl_image_info *spl_image,
					  بنية spl_بارول_info *info,
					  بنية container_hdr *container,
					  صحيح image_شاخص,
					  ulong container_دننه_واټن)
{
	بنية boot_img_t *images;
	ulong دننه_واټن, اندازه;
	فراغ *buf, *trampoline;

	إذا (image_شاخص > container->عدد_images) {
		debug("Invalid image number\n");
		أرجع عدم;
	}

	images = (بنية boot_img_t *)((u8 *)container +
				       حجم(بنية container_hdr));

	إذا (!IS_ALIGNED(images[image_شاخص].دننه_واټن, spl_get_bl_len(info))) {
		چاپ_کړه("%s: image%d offset not aligned to %u\n",
		       __func__, image_شاخص, spl_get_bl_len(info));
		أرجع عدم;
	}

	اندازه = ALIGN(images[image_شاخص].اندازه, spl_get_bl_len(info));
	دننه_واټن = images[image_شاخص].دننه_واټن + container_دننه_واټن;

	debug("%s: container: %p offset: %lu size: %lu\n", __func__,
	      container, دننه_واټن, اندازه);

	buf = map_sysmem(images[image_شاخص].dst, images[image_شاخص].اندازه);
	إذا (IS_ENABLED(CONFIG_SPL_IMX_CONTAINER_USE_TRAMPOLINE) &&
	    arch_check_dst_دننه_secure(buf, اندازه)) {
		trampoline = arch_get_container_trampoline();
		إذا (!trampoline) {
			چاپ_کړه("%s: trampoline size is zero\n", __func__);
			أرجع عدم;
		}

		إذا (info->اقرأ(info, دننه_واټن, اندازه, trampoline) < images[image_شاخص].اندازه) {
			چاپ_کړه("%s: failed to load image to a trampoline buffer\n", __func__);
			أرجع عدم;
		}

		حافظه_نقل(buf, trampoline, images[image_شاخص].اندازه);
	} وإلا {
		إذا (info->اقرأ(info, دننه_واټن, اندازه, buf) < images[image_شاخص].اندازه) {
				چاپ_کړه("%s: failed to load image to a non-secure region\n", __func__);
			أرجع عدم;
		}
	}

#که_تعریف_شوی CONFIG_AHAB_BOOT
	flush_dcache_range((ulong)buf, (ulong)(buf + images[image_شاخص].اندازه - 1));
	إذا (ahab_verify_cntr_image(&images[image_شاخص], image_شاخص))
		أرجع عدم;
#د_که_پای

	أرجع &images[image_شاخص];
}

ساكن صحيح read_auth_container(بنية spl_image_info *spl_image,
			       بنية spl_بارول_info *info, ulong دننه_واټن)
{
	بنية container_hdr *container = عدم;
	بنية container_hdr *authhdr;
	u16 اوږدتیا;
	صحيح i, اندازه, ret = 0;

	اندازه = ALIGN(CONTAINER_HDR_ALIGNMENT, spl_get_bl_len(info));

	/*
	 * It will not override the ATF code, so safe to use it here,
	 * no need malloc
	 */
	container = حافظه_ورکړه(اندازه);
	إذا (!container)
		أرجع -ENOMEM;

	debug("%s: container: %p offset: %lu size: %u\n", __func__,
	      container, دننه_واټن, اندازه);
	إذا (info->اقرأ(info, دننه_واټن, اندازه, container) <
	    CONTAINER_HDR_ALIGNMENT) {
		ret = -EIO;
		اذهب پای_ټکی_2;
	}

	إذا (!valid_container_hdr(container)) {
		log_err("Wrong container header\n");
		ret = -ENOENT;
		اذهب پای_ټکی_2;
	}

	إذا (!container->عدد_images) {
		log_err("Wrong container, no image found\n");
		ret = -ENOENT;
		اذهب پای_ټکی_2;
	}

	اوږدتیا = container->اوږدتیا_lsb + (container->اوږدتیا_msb << 8);
	debug("Container length %u\n", اوږدتیا);

	إذا (اوږدتیا > CONTAINER_HDR_ALIGNMENT) {
		اندازه = ALIGN(اوږدتیا, spl_get_bl_len(info));

		حرر(container);
		container = حافظه_ورکړه(اندازه);
		إذا (!container)
			أرجع -ENOMEM;

		debug("%s: container: %p offset: %lu size: %u\n",
		      __func__, container, دننه_واټن, اندازه);
		إذا (info->اقرأ(info, دننه_واټن, اندازه, container) < اوږدتیا) {
			ret = -EIO;
			اذهب پای_ټکی_2;
		}
	}

	authhdr = container;

#که_تعریف_شوی CONFIG_AHAB_BOOT
	authhdr = ahab_auth_cntr_hdr(authhdr, اوږدتیا);
	إذا (!authhdr) {
		ret = -EINVAL;
		اذهب پای_ټکی_auth;
	}
#د_که_پای

	لكل (i = 0; i < authhdr->عدد_images; i++) {
		بنية boot_img_t *image = read_auth_image(spl_image, info,
							   authhdr, i,
							   دننه_واټن);

		إذا (!image) {
			ret = -EINVAL;
			اذهب پای_ټکی_auth;
		}

		إذا (i == 0) {
			spl_image->بارول_addr = image->dst;
			spl_image->ثبت_ټکی = image->ثبت;
		}
	}

پای_ټکی_auth:
#که_تعریف_شوی CONFIG_AHAB_BOOT
	ahab_auth_release();
#د_که_پای

پای_ټکی_2:
	حرر(container);

	أرجع ret;
}

صحيح spl_بارول_imx_container(بنية spl_image_info *spl_image,
			   بنية spl_بارول_info *info, ulong دننه_واټن)
{
	أرجع read_auth_container(spl_image, info, دننه_واټن);
}
