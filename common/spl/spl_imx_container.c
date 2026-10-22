// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2021 NXP
 */

#परिभाषय LOG_CATEGORY LOGC_ARCH
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <imx_container.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <mapmem.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <cpu_func.h>
#यदि_परिभाषितम् CONFIG_AHAB_BOOT
#अन्तर्भाव <asm/mach-imx/ahab.h>
#यदि_अन्तः

__weak bool arch_check_dst_अन्तर्_secure(शून्य *start, ulong आकार)
{
	प्रत्यागम false;
}

__weak शून्य *arch_get_container_trampoline(शून्य)
{
	प्रत्यागम रिक्त;
}

स्थिर संरचना boot_img_t *read_auth_image(संरचना spl_image_info *spl_image,
					  संरचना spl_आरोपय_info *info,
					  संरचना container_hdr *container,
					  पूर्णाङ्क image_सूचकांक,
					  ulong container_विस्थापनम्)
{
	संरचना boot_img_t *images;
	ulong विस्थापनम्, आकार;
	शून्य *buf, *trampoline;

	यदि (image_सूचकांक > container->अङ्कः_images) {
		debug("Invalid image number\n");
		प्रत्यागम रिक्त;
	}

	images = (संरचना boot_img_t *)((u8 *)container +
				       परिमाण(संरचना container_hdr));

	यदि (!IS_ALIGNED(images[image_सूचकांक].विस्थापनम्, spl_get_bl_len(info))) {
		प्रारूप_लिख("%s: image%d offset not aligned to %u\n",
		       __func__, image_सूचकांक, spl_get_bl_len(info));
		प्रत्यागम रिक्त;
	}

	आकार = ALIGN(images[image_सूचकांक].आकार, spl_get_bl_len(info));
	विस्थापनम् = images[image_सूचकांक].विस्थापनम् + container_विस्थापनम्;

	debug("%s: container: %p offset: %lu size: %lu\n", __func__,
	      container, विस्थापनम्, आकार);

	buf = map_sysmem(images[image_सूचकांक].dst, images[image_सूचकांक].आकार);
	यदि (IS_ENABLED(CONFIG_SPL_IMX_CONTAINER_USE_TRAMPOLINE) &&
	    arch_check_dst_अन्तर्_secure(buf, आकार)) {
		trampoline = arch_get_container_trampoline();
		यदि (!trampoline) {
			प्रारूप_लिख("%s: trampoline size is zero\n", __func__);
			प्रत्यागम रिक्त;
		}

		यदि (info->पठ(info, विस्थापनम्, आकार, trampoline) < images[image_सूचकांक].आकार) {
			प्रारूप_लिख("%s: failed to load image to a trampoline buffer\n", __func__);
			प्रत्यागम रिक्त;
		}

		स्मृतिं_प्रतिलिख(buf, trampoline, images[image_सूचकांक].आकार);
	} अन्यथा {
		यदि (info->पठ(info, विस्थापनम्, आकार, buf) < images[image_सूचकांक].आकार) {
				प्रारूप_लिख("%s: failed to load image to a non-secure region\n", __func__);
			प्रत्यागम रिक्त;
		}
	}

#यदि_परिभाषितम् CONFIG_AHAB_BOOT
	flush_dcache_range((ulong)buf, (ulong)(buf + images[image_सूचकांक].आकार - 1));
	यदि (ahab_verify_cntr_image(&images[image_सूचकांक], image_सूचकांक))
		प्रत्यागम रिक्त;
#यदि_अन्तः

	प्रत्यागम &images[image_सूचकांक];
}

स्थिर पूर्णाङ्क read_auth_container(संरचना spl_image_info *spl_image,
			       संरचना spl_आरोपय_info *info, ulong विस्थापनम्)
{
	संरचना container_hdr *container = रिक्त;
	संरचना container_hdr *authhdr;
	u16 दीर्घता;
	पूर्णाङ्क i, आकार, ret = 0;

	आकार = ALIGN(CONTAINER_HDR_ALIGNMENT, spl_get_bl_len(info));

	/*
	 * It will not override the ATF code, so safe to use it here,
	 * no need malloc
	 */
	container = स्मृतिं_देहि(आकार);
	यदि (!container)
		प्रत्यागम -ENOMEM;

	debug("%s: container: %p offset: %lu size: %u\n", __func__,
	      container, विस्थापनम्, आकार);
	यदि (info->पठ(info, विस्थापनम्, आकार, container) <
	    CONTAINER_HDR_ALIGNMENT) {
		ret = -EIO;
		गच्छ अन्तः_2;
	}

	यदि (!valid_container_hdr(container)) {
		log_err("Wrong container header\n");
		ret = -ENOENT;
		गच्छ अन्तः_2;
	}

	यदि (!container->अङ्कः_images) {
		log_err("Wrong container, no image found\n");
		ret = -ENOENT;
		गच्छ अन्तः_2;
	}

	दीर्घता = container->दीर्घता_lsb + (container->दीर्घता_msb << 8);
	debug("Container length %u\n", दीर्घता);

	यदि (दीर्घता > CONTAINER_HDR_ALIGNMENT) {
		आकार = ALIGN(दीर्घता, spl_get_bl_len(info));

		मुक्त(container);
		container = स्मृतिं_देहि(आकार);
		यदि (!container)
			प्रत्यागम -ENOMEM;

		debug("%s: container: %p offset: %lu size: %u\n",
		      __func__, container, विस्थापनम्, आकार);
		यदि (info->पठ(info, विस्थापनम्, आकार, container) < दीर्घता) {
			ret = -EIO;
			गच्छ अन्तः_2;
		}
	}

	authhdr = container;

#यदि_परिभाषितम् CONFIG_AHAB_BOOT
	authhdr = ahab_auth_cntr_hdr(authhdr, दीर्घता);
	यदि (!authhdr) {
		ret = -EINVAL;
		गच्छ अन्तः_auth;
	}
#यदि_अन्तः

	प्रति (i = 0; i < authhdr->अङ्कः_images; i++) {
		संरचना boot_img_t *image = read_auth_image(spl_image, info,
							   authhdr, i,
							   विस्थापनम्);

		यदि (!image) {
			ret = -EINVAL;
			गच्छ अन्तः_auth;
		}

		यदि (i == 0) {
			spl_image->आरोपय_addr = image->dst;
			spl_image->प्रविष्टिः_point = image->प्रविष्टिः;
		}
	}

अन्तः_auth:
#यदि_परिभाषितम् CONFIG_AHAB_BOOT
	ahab_auth_release();
#यदि_अन्तः

अन्तः_2:
	मुक्त(container);

	प्रत्यागम ret;
}

पूर्णाङ्क spl_आरोपय_imx_container(संरचना spl_image_info *spl_image,
			   संरचना spl_आरोपय_info *info, ulong विस्थापनम्)
{
	प्रत्यागम read_auth_container(spl_image, info, विस्थापनम्);
}
