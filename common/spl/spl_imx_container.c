// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2021 NXP
 */

#నిర్వచించు LOG_CATEGORY LOGC_ARCH
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <imx_container.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <mapmem.h>
#అన్తర్భావ <spl.h>
#అన్తర్భావ <cpu_func.h>
#నిర్వచితమైతే CONFIG_AHAB_BOOT
#అన్తర్భావ <asm/mach-imx/ahab.h>
#అయితే_ముగింపు

__weak bool arch_check_dst_లోపల_secure(శూన్య *ప్రారంభం, ulong కొలత)
{
	తిరిగి false;
}

__weak శూన్య *arch_get_container_trampoline(శూన్య)
{
	తిరిగి ఖాళీ;
}

స్థిర నిర్మాణం boot_img_t *read_auth_image(నిర్మాణం spl_image_info *spl_image,
					  నిర్మాణం spl_ఎక్కించు_info *info,
					  నిర్మాణం container_hdr *container,
					  పూర్ణాంకం image_సూచీ,
					  ulong container_స్థానభ్రంశం)
{
	నిర్మాణం boot_img_t *images;
	ulong స్థానభ్రంశం, కొలత;
	శూన్య *buf, *trampoline;

	ఒకవేళ (image_సూచీ > container->అంకె_images) {
		debug("Invalid image number\n");
		తిరిగి ఖాళీ;
	}

	images = (నిర్మాణం boot_img_t *)((u8 *)container +
				       పరిమాణం(నిర్మాణం container_hdr));

	ఒకవేళ (!IS_ALIGNED(images[image_సూచీ].స్థానభ్రంశం, spl_get_bl_len(info))) {
		రాయి("%s: image%d offset not aligned to %u\n",
		       __func__, image_సూచీ, spl_get_bl_len(info));
		తిరిగి ఖాళీ;
	}

	కొలత = ALIGN(images[image_సూచీ].కొలత, spl_get_bl_len(info));
	స్థానభ్రంశం = images[image_సూచీ].స్థానభ్రంశం + container_స్థానభ్రంశం;

	debug("%s: container: %p offset: %lu size: %lu\n", __func__,
	      container, స్థానభ్రంశం, కొలత);

	buf = map_sysmem(images[image_సూచీ].dst, images[image_సూచీ].కొలత);
	ఒకవేళ (IS_ENABLED(CONFIG_SPL_IMX_CONTAINER_USE_TRAMPOLINE) &&
	    arch_check_dst_లోపల_secure(buf, కొలత)) {
		trampoline = arch_get_container_trampoline();
		ఒకవేళ (!trampoline) {
			రాయి("%s: trampoline size is zero\n", __func__);
			తిరిగి ఖాళీ;
		}

		ఒకవేళ (info->పఠ(info, స్థానభ్రంశం, కొలత, trampoline) < images[image_సూచీ].కొలత) {
			రాయి("%s: failed to load image to a trampoline buffer\n", __func__);
			తిరిగి ఖాళీ;
		}

		జ్ఞాపకం_నకలు(buf, trampoline, images[image_సూచీ].కొలత);
	} లేకపోతే {
		ఒకవేళ (info->పఠ(info, స్థానభ్రంశం, కొలత, buf) < images[image_సూచీ].కొలత) {
				రాయి("%s: failed to load image to a non-secure region\n", __func__);
			తిరిగి ఖాళీ;
		}
	}

#నిర్వచితమైతే CONFIG_AHAB_BOOT
	flush_dcache_range((ulong)buf, (ulong)(buf + images[image_సూచీ].కొలత - 1));
	ఒకవేళ (ahab_verify_cntr_image(&images[image_సూచీ], image_సూచీ))
		తిరిగి ఖాళీ;
#అయితే_ముగింపు

	తిరిగి &images[image_సూచీ];
}

స్థిర పూర్ణాంకం read_auth_container(నిర్మాణం spl_image_info *spl_image,
			       నిర్మాణం spl_ఎక్కించు_info *info, ulong స్థానభ్రంశం)
{
	నిర్మాణం container_hdr *container = ఖాళీ;
	నిర్మాణం container_hdr *authhdr;
	u16 నిడివి;
	పూర్ణాంకం i, కొలత, ret = 0;

	కొలత = ALIGN(CONTAINER_HDR_ALIGNMENT, spl_get_bl_len(info));

	/*
	 * It will not override the ATF code, so safe to use it here,
	 * no need malloc
	 */
	container = జ్ఞాపకం_ఇవ్వు(కొలత);
	ఒకవేళ (!container)
		తిరిగి -ENOMEM;

	debug("%s: container: %p offset: %lu size: %u\n", __func__,
	      container, స్థానభ్రంశం, కొలత);
	ఒకవేళ (info->పఠ(info, స్థానభ్రంశం, కొలత, container) <
	    CONTAINER_HDR_ALIGNMENT) {
		ret = -EIO;
		వెళ్ళు చివర_2;
	}

	ఒకవేళ (!valid_container_hdr(container)) {
		log_err("Wrong container header\n");
		ret = -ENOENT;
		వెళ్ళు చివర_2;
	}

	ఒకవేళ (!container->అంకె_images) {
		log_err("Wrong container, no image found\n");
		ret = -ENOENT;
		వెళ్ళు చివర_2;
	}

	నిడివి = container->నిడివి_lsb + (container->నిడివి_msb << 8);
	debug("Container length %u\n", నిడివి);

	ఒకవేళ (నిడివి > CONTAINER_HDR_ALIGNMENT) {
		కొలత = ALIGN(నిడివి, spl_get_bl_len(info));

		విడుదల(container);
		container = జ్ఞాపకం_ఇవ్వు(కొలత);
		ఒకవేళ (!container)
			తిరిగి -ENOMEM;

		debug("%s: container: %p offset: %lu size: %u\n",
		      __func__, container, స్థానభ్రంశం, కొలత);
		ఒకవేళ (info->పఠ(info, స్థానభ్రంశం, కొలత, container) < నిడివి) {
			ret = -EIO;
			వెళ్ళు చివర_2;
		}
	}

	authhdr = container;

#నిర్వచితమైతే CONFIG_AHAB_BOOT
	authhdr = ahab_auth_cntr_hdr(authhdr, నిడివి);
	ఒకవేళ (!authhdr) {
		ret = -EINVAL;
		వెళ్ళు చివర_auth;
	}
#అయితే_ముగింపు

	కొరకు (i = 0; i < authhdr->అంకె_images; i++) {
		నిర్మాణం boot_img_t *image = read_auth_image(spl_image, info,
							   authhdr, i,
							   స్థానభ్రంశం);

		ఒకవేళ (!image) {
			ret = -EINVAL;
			వెళ్ళు చివర_auth;
		}

		ఒకవేళ (i == 0) {
			spl_image->ఎక్కించు_addr = image->dst;
			spl_image->నమోదు_బిందువు = image->నమోదు_2;
		}
	}

చివర_auth:
#నిర్వచితమైతే CONFIG_AHAB_BOOT
	ahab_auth_release();
#అయితే_ముగింపు

చివర_2:
	విడుదల(container);

	తిరిగి ret;
}

పూర్ణాంకం spl_ఎక్కించు_imx_container(నిర్మాణం spl_image_info *spl_image,
			   నిర్మాణం spl_ఎక్కించు_info *info, ulong స్థానభ్రంశం)
{
	తిరిగి read_auth_container(spl_image, info, స్థానభ్రంశం);
}
