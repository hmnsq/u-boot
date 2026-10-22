// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2021 NXP
 */

#تعریف_کرو LOG_CATEGORY LOGC_ARCH
#شامل <stdlib.h>
#شامل <errno.h>
#شامل <imx_container.h>
#شامل <log.h>
#شامل <mapmem.h>
#شامل <spl.h>
#شامل <cpu_func.h>
#اگر_تعریف_شدہ CONFIG_AHAB_BOOT
#شامل <asm/mach-imx/ahab.h>
#اگر_ختم

__weak bool arch_check_dst_اندر_secure(خالی *آغاز, ulong سائز)
{
	واپس false;
}

__weak خالی *arch_get_container_trampoline(خالی)
{
	واپس تہی;
}

ساکن ساخت boot_img_t *read_auth_image(ساخت spl_image_info *spl_image,
					  ساخت spl_بھرو_info *info,
					  ساخت container_hdr *container,
					  صحیح image_اشاریہ,
					  ulong container_انحراف)
{
	ساخت boot_img_t *images;
	ulong انحراف, سائز;
	خالی *buf, *trampoline;

	اگر (image_اشاریہ > container->عدد_images) {
		debug("Invalid image number\n");
		واپس تہی;
	}

	images = (ساخت boot_img_t *)((u8 *)container +
				       حجم(ساخت container_hdr));

	اگر (!IS_ALIGNED(images[image_اشاریہ].انحراف, spl_get_bl_len(info))) {
		لکھو("%s: image%d offset not aligned to %u\n",
		       __func__, image_اشاریہ, spl_get_bl_len(info));
		واپس تہی;
	}

	سائز = ALIGN(images[image_اشاریہ].سائز, spl_get_bl_len(info));
	انحراف = images[image_اشاریہ].انحراف + container_انحراف;

	debug("%s: container: %p offset: %lu size: %lu\n", __func__,
	      container, انحراف, سائز);

	buf = map_sysmem(images[image_اشاریہ].dst, images[image_اشاریہ].سائز);
	اگر (IS_ENABLED(CONFIG_SPL_IMX_CONTAINER_USE_TRAMPOLINE) &&
	    arch_check_dst_اندر_secure(buf, سائز)) {
		trampoline = arch_get_container_trampoline();
		اگر (!trampoline) {
			لکھو("%s: trampoline size is zero\n", __func__);
			واپس تہی;
		}

		اگر (info->اقرأ(info, انحراف, سائز, trampoline) < images[image_اشاریہ].سائز) {
			لکھو("%s: failed to load image to a trampoline buffer\n", __func__);
			واپس تہی;
		}

		حافظہ_نقل(buf, trampoline, images[image_اشاریہ].سائز);
	} ورنہ {
		اگر (info->اقرأ(info, انحراف, سائز, buf) < images[image_اشاریہ].سائز) {
				لکھو("%s: failed to load image to a non-secure region\n", __func__);
			واپس تہی;
		}
	}

#اگر_تعریف_شدہ CONFIG_AHAB_BOOT
	flush_dcache_range((ulong)buf, (ulong)(buf + images[image_اشاریہ].سائز - 1));
	اگر (ahab_verify_cntr_image(&images[image_اشاریہ], image_اشاریہ))
		واپس تہی;
#اگر_ختم

	واپس &images[image_اشاریہ];
}

ساکن صحیح read_auth_container(ساخت spl_image_info *spl_image,
			       ساخت spl_بھرو_info *info, ulong انحراف)
{
	ساخت container_hdr *container = تہی;
	ساخت container_hdr *authhdr;
	u16 لمبائی;
	صحیح i, سائز, ret = 0;

	سائز = ALIGN(CONTAINER_HDR_ALIGNMENT, spl_get_bl_len(info));

	/*
	 * It will not override the ATF code, so safe to use it here,
	 * no need malloc
	 */
	container = حافظہ_دو(سائز);
	اگر (!container)
		واپس -ENOMEM;

	debug("%s: container: %p offset: %lu size: %u\n", __func__,
	      container, انحراف, سائز);
	اگر (info->اقرأ(info, انحراف, سائز, container) <
	    CONTAINER_HDR_ALIGNMENT) {
		ret = -EIO;
		جاؤ انتہا_2;
	}

	اگر (!valid_container_hdr(container)) {
		log_err("Wrong container header\n");
		ret = -ENOENT;
		جاؤ انتہا_2;
	}

	اگر (!container->عدد_images) {
		log_err("Wrong container, no image found\n");
		ret = -ENOENT;
		جاؤ انتہا_2;
	}

	لمبائی = container->لمبائی_lsb + (container->لمبائی_msb << 8);
	debug("Container length %u\n", لمبائی);

	اگر (لمبائی > CONTAINER_HDR_ALIGNMENT) {
		سائز = ALIGN(لمبائی, spl_get_bl_len(info));

		آزاد(container);
		container = حافظہ_دو(سائز);
		اگر (!container)
			واپس -ENOMEM;

		debug("%s: container: %p offset: %lu size: %u\n",
		      __func__, container, انحراف, سائز);
		اگر (info->اقرأ(info, انحراف, سائز, container) < لمبائی) {
			ret = -EIO;
			جاؤ انتہا_2;
		}
	}

	authhdr = container;

#اگر_تعریف_شدہ CONFIG_AHAB_BOOT
	authhdr = ahab_auth_cntr_hdr(authhdr, لمبائی);
	اگر (!authhdr) {
		ret = -EINVAL;
		جاؤ انتہا_auth;
	}
#اگر_ختم

	برائے (i = 0; i < authhdr->عدد_images; i++) {
		ساخت boot_img_t *image = read_auth_image(spl_image, info,
							   authhdr, i,
							   انحراف);

		اگر (!image) {
			ret = -EINVAL;
			جاؤ انتہا_auth;
		}

		اگر (i == 0) {
			spl_image->بھرو_addr = image->dst;
			spl_image->اندراج_نقطہ = image->اندراج_2;
		}
	}

انتہا_auth:
#اگر_تعریف_شدہ CONFIG_AHAB_BOOT
	ahab_auth_release();
#اگر_ختم

انتہا_2:
	آزاد(container);

	واپس ret;
}

صحیح spl_بھرو_imx_container(ساخت spl_image_info *spl_image,
			   ساخت spl_بھرو_info *info, ulong انحراف)
{
	واپس read_auth_container(spl_image, info, انحراف);
}
