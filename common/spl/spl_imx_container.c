// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2021 NXP
 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LOG_CATEGORY LOGC_ARCH
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <imx_container.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mapmem.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <cpu_func.h>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_AHAB_BOOT
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/mach-imx/ahab.h>
#ਜੇ_ਅੰਤ

__weak bool arch_check_dst_ਵਿੱਚ_secure(ਖਾਲੀ *ਸ਼ੁਰੂ, ulong ਮਾਪ)
{
	ਵਾਪਸ false;
}

__weak ਖਾਲੀ *arch_get_container_trampoline(ਖਾਲੀ)
{
	ਵਾਪਸ ਨਿਰਾ;
}

ਸਥਿਰ ਬਣਤਰ boot_img_t *read_auth_image(ਬਣਤਰ spl_image_info *spl_image,
					  ਬਣਤਰ spl_ਲੱਦੋ_info *info,
					  ਬਣਤਰ container_hdr *container,
					  ਪੂਰਨ image_ਸੂਚਕਾਂਕ,
					  ulong container_ਵਿੱਥ)
{
	ਬਣਤਰ boot_img_t *images;
	ulong ਵਿੱਥ, ਮਾਪ;
	ਖਾਲੀ *buf, *trampoline;

	ਜੇ (image_ਸੂਚਕਾਂਕ > container->ਅੰਕ_images) {
		debug("Invalid image number\n");
		ਵਾਪਸ ਨਿਰਾ;
	}

	images = (ਬਣਤਰ boot_img_t *)((u8 *)container +
				       ਆਕਾਰ(ਬਣਤਰ container_hdr));

	ਜੇ (!IS_ALIGNED(images[image_ਸੂਚਕਾਂਕ].ਵਿੱਥ, spl_get_bl_len(info))) {
		ਲਿਖੋ("%s: image%d offset not aligned to %u\n",
		       __func__, image_ਸੂਚਕਾਂਕ, spl_get_bl_len(info));
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਮਾਪ = ALIGN(images[image_ਸੂਚਕਾਂਕ].ਮਾਪ, spl_get_bl_len(info));
	ਵਿੱਥ = images[image_ਸੂਚਕਾਂਕ].ਵਿੱਥ + container_ਵਿੱਥ;

	debug("%s: container: %p offset: %lu size: %lu\n", __func__,
	      container, ਵਿੱਥ, ਮਾਪ);

	buf = map_sysmem(images[image_ਸੂਚਕਾਂਕ].dst, images[image_ਸੂਚਕਾਂਕ].ਮਾਪ);
	ਜੇ (IS_ENABLED(CONFIG_SPL_IMX_CONTAINER_USE_TRAMPOLINE) &&
	    arch_check_dst_ਵਿੱਚ_secure(buf, ਮਾਪ)) {
		trampoline = arch_get_container_trampoline();
		ਜੇ (!trampoline) {
			ਲਿਖੋ("%s: trampoline size is zero\n", __func__);
			ਵਾਪਸ ਨਿਰਾ;
		}

		ਜੇ (info->ਪਠ(info, ਵਿੱਥ, ਮਾਪ, trampoline) < images[image_ਸੂਚਕਾਂਕ].ਮਾਪ) {
			ਲਿਖੋ("%s: failed to load image to a trampoline buffer\n", __func__);
			ਵਾਪਸ ਨਿਰਾ;
		}

		ਸਮ੍ਰਿਤੀ_ਨਕਲ(buf, trampoline, images[image_ਸੂਚਕਾਂਕ].ਮਾਪ);
	} ਨਹੀਂਤਾਂ {
		ਜੇ (info->ਪਠ(info, ਵਿੱਥ, ਮਾਪ, buf) < images[image_ਸੂਚਕਾਂਕ].ਮਾਪ) {
				ਲਿਖੋ("%s: failed to load image to a non-secure region\n", __func__);
			ਵਾਪਸ ਨਿਰਾ;
		}
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_AHAB_BOOT
	flush_dcache_range((ulong)buf, (ulong)(buf + images[image_ਸੂਚਕਾਂਕ].ਮਾਪ - 1));
	ਜੇ (ahab_verify_cntr_image(&images[image_ਸੂਚਕਾਂਕ], image_ਸੂਚਕਾਂਕ))
		ਵਾਪਸ ਨਿਰਾ;
#ਜੇ_ਅੰਤ

	ਵਾਪਸ &images[image_ਸੂਚਕਾਂਕ];
}

ਸਥਿਰ ਪੂਰਨ read_auth_container(ਬਣਤਰ spl_image_info *spl_image,
			       ਬਣਤਰ spl_ਲੱਦੋ_info *info, ulong ਵਿੱਥ)
{
	ਬਣਤਰ container_hdr *container = ਨਿਰਾ;
	ਬਣਤਰ container_hdr *authhdr;
	u16 ਲੰਮਾਈ;
	ਪੂਰਨ i, ਮਾਪ, ret = 0;

	ਮਾਪ = ALIGN(CONTAINER_HDR_ALIGNMENT, spl_get_bl_len(info));

	/*
	 * It will not override the ATF code, so safe to use it here,
	 * no need malloc
	 */
	container = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਮਾਪ);
	ਜੇ (!container)
		ਵਾਪਸ -ENOMEM;

	debug("%s: container: %p offset: %lu size: %u\n", __func__,
	      container, ਵਿੱਥ, ਮਾਪ);
	ਜੇ (info->ਪਠ(info, ਵਿੱਥ, ਮਾਪ, container) <
	    CONTAINER_HDR_ALIGNMENT) {
		ret = -EIO;
		ਜਾਓ ਸਿਰਾ_2;
	}

	ਜੇ (!valid_container_hdr(container)) {
		log_err("Wrong container header\n");
		ret = -ENOENT;
		ਜਾਓ ਸਿਰਾ_2;
	}

	ਜੇ (!container->ਅੰਕ_images) {
		log_err("Wrong container, no image found\n");
		ret = -ENOENT;
		ਜਾਓ ਸਿਰਾ_2;
	}

	ਲੰਮਾਈ = container->ਲੰਮਾਈ_lsb + (container->ਲੰਮਾਈ_msb << 8);
	debug("Container length %u\n", ਲੰਮਾਈ);

	ਜੇ (ਲੰਮਾਈ > CONTAINER_HDR_ALIGNMENT) {
		ਮਾਪ = ALIGN(ਲੰਮਾਈ, spl_get_bl_len(info));

		ਮੁਕਤ(container);
		container = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਮਾਪ);
		ਜੇ (!container)
			ਵਾਪਸ -ENOMEM;

		debug("%s: container: %p offset: %lu size: %u\n",
		      __func__, container, ਵਿੱਥ, ਮਾਪ);
		ਜੇ (info->ਪਠ(info, ਵਿੱਥ, ਮਾਪ, container) < ਲੰਮਾਈ) {
			ret = -EIO;
			ਜਾਓ ਸਿਰਾ_2;
		}
	}

	authhdr = container;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_AHAB_BOOT
	authhdr = ahab_auth_cntr_hdr(authhdr, ਲੰਮਾਈ);
	ਜੇ (!authhdr) {
		ret = -EINVAL;
		ਜਾਓ ਸਿਰਾ_auth;
	}
#ਜੇ_ਅੰਤ

	ਲਈ (i = 0; i < authhdr->ਅੰਕ_images; i++) {
		ਬਣਤਰ boot_img_t *image = read_auth_image(spl_image, info,
							   authhdr, i,
							   ਵਿੱਥ);

		ਜੇ (!image) {
			ret = -EINVAL;
			ਜਾਓ ਸਿਰਾ_auth;
		}

		ਜੇ (i == 0) {
			spl_image->ਲੱਦੋ_addr = image->dst;
			spl_image->ਇੰਦਰਾਜ_ਬਿੰਦੂ = image->ਇੰਦਰਾਜ;
		}
	}

ਸਿਰਾ_auth:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_AHAB_BOOT
	ahab_auth_release();
#ਜੇ_ਅੰਤ

ਸਿਰਾ_2:
	ਮੁਕਤ(container);

	ਵਾਪਸ ret;
}

ਪੂਰਨ spl_ਲੱਦੋ_imx_container(ਬਣਤਰ spl_image_info *spl_image,
			   ਬਣਤਰ spl_ਲੱਦੋ_info *info, ulong ਵਿੱਥ)
{
	ਵਾਪਸ read_auth_container(spl_image, info, ਵਿੱਥ);
}
