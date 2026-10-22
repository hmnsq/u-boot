// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2021 NXP
 */

#ವ್ಯಾಖ್ಯಾನಿಸು LOG_CATEGORY LOGC_ARCH
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <imx_container.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <cpu_func.h>
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_AHAB_BOOT
#ಅನ್ತರ್ಭಾವ <asm/mach-imx/ahab.h>
#ಆದರೆ_ಅಂತ್ಯ

__weak bool arch_check_dst_ಒಳ_secure(ಶೂನ್ಯ *ಆರಂಭ, ulong ಗಾತ್ರ_2)
{
	ಹಿಂತಿರುಗಿ false;
}

__weak ಶೂನ್ಯ *arch_get_container_trampoline(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಸ್ಥಿರ ರಚನೆ boot_img_t *read_auth_image(ರಚನೆ spl_image_info *spl_image,
					  ರಚನೆ spl_ತುಂಬು_info *info,
					  ರಚನೆ container_hdr *container,
					  ಪೂರ್ಣಾಂಕ image_ಸೂಚ್ಯಂಕ,
					  ulong container_ಸ್ಥಾನಾಂತರ)
{
	ರಚನೆ boot_img_t *images;
	ulong ಸ್ಥಾನಾಂತರ, ಗಾತ್ರ_2;
	ಶೂನ್ಯ *buf, *trampoline;

	ಆದರೆ (image_ಸೂಚ್ಯಂಕ > container->ಅಂಕೆ_images) {
		debug("Invalid image number\n");
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	images = (ರಚನೆ boot_img_t *)((u8 *)container +
				       ಗಾತ್ರ(ರಚನೆ container_hdr));

	ಆದರೆ (!IS_ALIGNED(images[image_ಸೂಚ್ಯಂಕ].ಸ್ಥಾನಾಂತರ, spl_get_bl_len(info))) {
		ಬರೆ("%s: image%d offset not aligned to %u\n",
		       __func__, image_ಸೂಚ್ಯಂಕ, spl_get_bl_len(info));
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಗಾತ್ರ_2 = ALIGN(images[image_ಸೂಚ್ಯಂಕ].ಗಾತ್ರ_2, spl_get_bl_len(info));
	ಸ್ಥಾನಾಂತರ = images[image_ಸೂಚ್ಯಂಕ].ಸ್ಥಾನಾಂತರ + container_ಸ್ಥಾನಾಂತರ;

	debug("%s: container: %p offset: %lu size: %lu\n", __func__,
	      container, ಸ್ಥಾನಾಂತರ, ಗಾತ್ರ_2);

	buf = map_sysmem(images[image_ಸೂಚ್ಯಂಕ].dst, images[image_ಸೂಚ್ಯಂಕ].ಗಾತ್ರ_2);
	ಆದರೆ (IS_ENABLED(CONFIG_SPL_IMX_CONTAINER_USE_TRAMPOLINE) &&
	    arch_check_dst_ಒಳ_secure(buf, ಗಾತ್ರ_2)) {
		trampoline = arch_get_container_trampoline();
		ಆದರೆ (!trampoline) {
			ಬರೆ("%s: trampoline size is zero\n", __func__);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}

		ಆದರೆ (info->ಪಠ(info, ಸ್ಥಾನಾಂತರ, ಗಾತ್ರ_2, trampoline) < images[image_ಸೂಚ್ಯಂಕ].ಗಾತ್ರ_2) {
			ಬರೆ("%s: failed to load image to a trampoline buffer\n", __func__);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}

		ಸ್ಮರಣೆ_ನಕಲು(buf, trampoline, images[image_ಸೂಚ್ಯಂಕ].ಗಾತ್ರ_2);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (info->ಪಠ(info, ಸ್ಥಾನಾಂತರ, ಗಾತ್ರ_2, buf) < images[image_ಸೂಚ್ಯಂಕ].ಗಾತ್ರ_2) {
				ಬರೆ("%s: failed to load image to a non-secure region\n", __func__);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_AHAB_BOOT
	flush_dcache_range((ulong)buf, (ulong)(buf + images[image_ಸೂಚ್ಯಂಕ].ಗಾತ್ರ_2 - 1));
	ಆದರೆ (ahab_verify_cntr_image(&images[image_ಸೂಚ್ಯಂಕ], image_ಸೂಚ್ಯಂಕ))
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ &images[image_ಸೂಚ್ಯಂಕ];
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ read_auth_container(ರಚನೆ spl_image_info *spl_image,
			       ರಚನೆ spl_ತುಂಬು_info *info, ulong ಸ್ಥಾನಾಂತರ)
{
	ರಚನೆ container_hdr *container = ಖಾಲಿ;
	ರಚನೆ container_hdr *authhdr;
	u16 ಉದ್ದಳತೆ;
	ಪೂರ್ಣಾಂಕ i, ಗಾತ್ರ_2, ret = 0;

	ಗಾತ್ರ_2 = ALIGN(CONTAINER_HDR_ALIGNMENT, spl_get_bl_len(info));

	/*
	 * It will not override the ATF code, so safe to use it here,
	 * no need malloc
	 */
	container = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ_2);
	ಆದರೆ (!container)
		ಹಿಂತಿರುಗಿ -ENOMEM;

	debug("%s: container: %p offset: %lu size: %u\n", __func__,
	      container, ಸ್ಥಾನಾಂತರ, ಗಾತ್ರ_2);
	ಆದರೆ (info->ಪಠ(info, ಸ್ಥಾನಾಂತರ, ಗಾತ್ರ_2, container) <
	    CONTAINER_HDR_ALIGNMENT) {
		ret = -EIO;
		ಹೋಗು ಕೊನೆ_2;
	}

	ಆದರೆ (!valid_container_hdr(container)) {
		log_err("Wrong container header\n");
		ret = -ENOENT;
		ಹೋಗು ಕೊನೆ_2;
	}

	ಆದರೆ (!container->ಅಂಕೆ_images) {
		log_err("Wrong container, no image found\n");
		ret = -ENOENT;
		ಹೋಗು ಕೊನೆ_2;
	}

	ಉದ್ದಳತೆ = container->ಉದ್ದಳತೆ_lsb + (container->ಉದ್ದಳತೆ_msb << 8);
	debug("Container length %u\n", ಉದ್ದಳತೆ);

	ಆದರೆ (ಉದ್ದಳತೆ > CONTAINER_HDR_ALIGNMENT) {
		ಗಾತ್ರ_2 = ALIGN(ಉದ್ದಳತೆ, spl_get_bl_len(info));

		ಬಿಡುಗಡೆ(container);
		container = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ_2);
		ಆದರೆ (!container)
			ಹಿಂತಿರುಗಿ -ENOMEM;

		debug("%s: container: %p offset: %lu size: %u\n",
		      __func__, container, ಸ್ಥಾನಾಂತರ, ಗಾತ್ರ_2);
		ಆದರೆ (info->ಪಠ(info, ಸ್ಥಾನಾಂತರ, ಗಾತ್ರ_2, container) < ಉದ್ದಳತೆ) {
			ret = -EIO;
			ಹೋಗು ಕೊನೆ_2;
		}
	}

	authhdr = container;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_AHAB_BOOT
	authhdr = ahab_auth_cntr_hdr(authhdr, ಉದ್ದಳತೆ);
	ಆದರೆ (!authhdr) {
		ret = -EINVAL;
		ಹೋಗು ಕೊನೆ_auth;
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಗಾಗಿ (i = 0; i < authhdr->ಅಂಕೆ_images; i++) {
		ರಚನೆ boot_img_t *image = read_auth_image(spl_image, info,
							   authhdr, i,
							   ಸ್ಥಾನಾಂತರ);

		ಆದರೆ (!image) {
			ret = -EINVAL;
			ಹೋಗು ಕೊನೆ_auth;
		}

		ಆದರೆ (i == 0) {
			spl_image->ತುಂಬು_addr = image->dst;
			spl_image->ನಮೂದು_ಬಿಂದು = image->ನಮೂದು;
		}
	}

ಕೊನೆ_auth:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_AHAB_BOOT
	ahab_auth_release();
#ಆದರೆ_ಅಂತ್ಯ

ಕೊನೆ_2:
	ಬಿಡುಗಡೆ(container);

	ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ spl_ತುಂಬು_imx_container(ರಚನೆ spl_image_info *spl_image,
			   ರಚನೆ spl_ತುಂಬು_info *info, ulong ಸ್ಥಾನಾಂತರ)
{
	ಹಿಂತಿರುಗಿ read_auth_container(spl_image, info, ಸ್ಥಾನಾಂತರ);
}
