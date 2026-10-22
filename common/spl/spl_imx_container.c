// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2021 NXP
 */

#وصف_ڪريو LOG_CATEGORY LOGC_ARCH
#ضمن <stdlib.h>
#ضمن <errno.h>
#ضمن <imx_container.h>
#ضمن <log.h>
#ضمن <mapmem.h>
#ضمن <spl.h>
#ضمن <cpu_func.h>
#جيڪڏهن_وصف_ٿيل CONFIG_AHAB_BOOT
#ضمن <asm/mach-imx/ahab.h>
#جيڪڏهن_پورو

__weak bool arch_check_dst_اندريون_secure(فراغ *شروعات, ulong سائيز)
{
	أرجع false;
}

__weak فراغ *arch_get_container_trampoline(فراغ)
{
	أرجع عدم;
}

ساكن بنية boot_img_t *read_auth_image(بنية spl_image_info *spl_image,
					  بنية spl_ڀريو_info *info,
					  بنية container_hdr *container,
					  صحيح image_اشاريو,
					  ulong container_وٿي)
{
	بنية boot_img_t *images;
	ulong وٿي, سائيز;
	فراغ *buf, *trampoline;

	إذا (image_اشاريو > container->انگ_images) {
		debug("Invalid image number\n");
		أرجع عدم;
	}

	images = (بنية boot_img_t *)((u8 *)container +
				       حجم(بنية container_hdr));

	إذا (!IS_ALIGNED(images[image_اشاريو].وٿي, spl_get_bl_len(info))) {
		لکو("%s: image%d offset not aligned to %u\n",
		       __func__, image_اشاريو, spl_get_bl_len(info));
		أرجع عدم;
	}

	سائيز = ALIGN(images[image_اشاريو].سائيز, spl_get_bl_len(info));
	وٿي = images[image_اشاريو].وٿي + container_وٿي;

	debug("%s: container: %p offset: %lu size: %lu\n", __func__,
	      container, وٿي, سائيز);

	buf = map_sysmem(images[image_اشاريو].dst, images[image_اشاريو].سائيز);
	إذا (IS_ENABLED(CONFIG_SPL_IMX_CONTAINER_USE_TRAMPOLINE) &&
	    arch_check_dst_اندريون_secure(buf, سائيز)) {
		trampoline = arch_get_container_trampoline();
		إذا (!trampoline) {
			لکو("%s: trampoline size is zero\n", __func__);
			أرجع عدم;
		}

		إذا (info->اقرأ(info, وٿي, سائيز, trampoline) < images[image_اشاريو].سائيز) {
			لکو("%s: failed to load image to a trampoline buffer\n", __func__);
			أرجع عدم;
		}

		يادگيري_نقل(buf, trampoline, images[image_اشاريو].سائيز);
	} وإلا {
		إذا (info->اقرأ(info, وٿي, سائيز, buf) < images[image_اشاريو].سائيز) {
				لکو("%s: failed to load image to a non-secure region\n", __func__);
			أرجع عدم;
		}
	}

#جيڪڏهن_وصف_ٿيل CONFIG_AHAB_BOOT
	flush_dcache_range((ulong)buf, (ulong)(buf + images[image_اشاريو].سائيز - 1));
	إذا (ahab_verify_cntr_image(&images[image_اشاريو], image_اشاريو))
		أرجع عدم;
#جيڪڏهن_پورو

	أرجع &images[image_اشاريو];
}

ساكن صحيح read_auth_container(بنية spl_image_info *spl_image,
			       بنية spl_ڀريو_info *info, ulong وٿي)
{
	بنية container_hdr *container = عدم;
	بنية container_hdr *authhdr;
	u16 ڊيگهائي;
	صحيح i, سائيز, ret = 0;

	سائيز = ALIGN(CONTAINER_HDR_ALIGNMENT, spl_get_bl_len(info));

	/*
	 * It will not override the ATF code, so safe to use it here,
	 * no need malloc
	 */
	container = يادگيري_ڏيو(سائيز);
	إذا (!container)
		أرجع -ENOMEM;

	debug("%s: container: %p offset: %lu size: %u\n", __func__,
	      container, وٿي, سائيز);
	إذا (info->اقرأ(info, وٿي, سائيز, container) <
	    CONTAINER_HDR_ALIGNMENT) {
		ret = -EIO;
		اذهب پڇاڙي_2;
	}

	إذا (!valid_container_hdr(container)) {
		log_err("Wrong container header\n");
		ret = -ENOENT;
		اذهب پڇاڙي_2;
	}

	إذا (!container->انگ_images) {
		log_err("Wrong container, no image found\n");
		ret = -ENOENT;
		اذهب پڇاڙي_2;
	}

	ڊيگهائي = container->ڊيگهائي_lsb + (container->ڊيگهائي_msb << 8);
	debug("Container length %u\n", ڊيگهائي);

	إذا (ڊيگهائي > CONTAINER_HDR_ALIGNMENT) {
		سائيز = ALIGN(ڊيگهائي, spl_get_bl_len(info));

		حرر(container);
		container = يادگيري_ڏيو(سائيز);
		إذا (!container)
			أرجع -ENOMEM;

		debug("%s: container: %p offset: %lu size: %u\n",
		      __func__, container, وٿي, سائيز);
		إذا (info->اقرأ(info, وٿي, سائيز, container) < ڊيگهائي) {
			ret = -EIO;
			اذهب پڇاڙي_2;
		}
	}

	authhdr = container;

#جيڪڏهن_وصف_ٿيل CONFIG_AHAB_BOOT
	authhdr = ahab_auth_cntr_hdr(authhdr, ڊيگهائي);
	إذا (!authhdr) {
		ret = -EINVAL;
		اذهب پڇاڙي_auth;
	}
#جيڪڏهن_پورو

	لكل (i = 0; i < authhdr->انگ_images; i++) {
		بنية boot_img_t *image = read_auth_image(spl_image, info,
							   authhdr, i,
							   وٿي);

		إذا (!image) {
			ret = -EINVAL;
			اذهب پڇاڙي_auth;
		}

		إذا (i == 0) {
			spl_image->ڀريو_addr = image->dst;
			spl_image->داخلا_نقطو = image->داخلا_2;
		}
	}

پڇاڙي_auth:
#جيڪڏهن_وصف_ٿيل CONFIG_AHAB_BOOT
	ahab_auth_release();
#جيڪڏهن_پورو

پڇاڙي_2:
	حرر(container);

	أرجع ret;
}

صحيح spl_ڀريو_imx_container(بنية spl_image_info *spl_image,
			   بنية spl_ڀريو_info *info, ulong وٿي)
{
	أرجع read_auth_container(spl_image, info, وٿي);
}
