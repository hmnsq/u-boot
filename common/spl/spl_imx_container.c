// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2021 NXP
 */

#घोषणा LOG_CATEGORY LOGC_ARCH
#समावेश <stdlib.h>
#समावेश <errno.h>
#समावेश <imx_container.h>
#समावेश <log.h>
#समावेश <mapmem.h>
#समावेश <spl.h>
#समावेश <cpu_func.h>
#अगर_घोषित CONFIG_AHAB_BOOT
#समावेश <asm/mach-imx/ahab.h>
#पूर्ण_अगर

__weak bool arch_check_dst_in_secure(व्योम *start, ulong आकार)
{
	वापस false;
}

__weak व्योम *arch_get_container_trampoline(व्योम)
{
	वापस शून्य;
}

अटल काष्ठा boot_img_t *read_auth_image(काष्ठा spl_image_info *spl_image,
					  काष्ठा spl_load_info *info,
					  काष्ठा container_hdr *container,
					  पूर्णांक image_सूचकांक,
					  ulong container_offset)
{
	काष्ठा boot_img_t *images;
	ulong offset, आकार;
	व्योम *buf, *trampoline;

	अगर (image_सूचकांक > container->अंक_images) {
		debug("Invalid image number\n");
		वापस शून्य;
	}

	images = (काष्ठा boot_img_t *)((u8 *)container +
				       माप(काष्ठा container_hdr));

	अगर (!IS_ALIGNED(images[image_सूचकांक].offset, spl_get_bl_len(info))) {
		म_लिखो("%s: image%d offset not aligned to %u\n",
		       __func__, image_सूचकांक, spl_get_bl_len(info));
		वापस शून्य;
	}

	आकार = ALIGN(images[image_सूचकांक].आकार, spl_get_bl_len(info));
	offset = images[image_सूचकांक].offset + container_offset;

	debug("%s: container: %p offset: %lu size: %lu\n", __func__,
	      container, offset, आकार);

	buf = map_sysmem(images[image_सूचकांक].dst, images[image_सूचकांक].आकार);
	अगर (IS_ENABLED(CONFIG_SPL_IMX_CONTAINER_USE_TRAMPOLINE) &&
	    arch_check_dst_in_secure(buf, आकार)) {
		trampoline = arch_get_container_trampoline();
		अगर (!trampoline) {
			म_लिखो("%s: trampoline size is zero\n", __func__);
			वापस शून्य;
		}

		अगर (info->पढ़ो(info, offset, आकार, trampoline) < images[image_सूचकांक].आकार) {
			म_लिखो("%s: failed to load image to a trampoline buffer\n", __func__);
			वापस शून्य;
		}

		स_नकल(buf, trampoline, images[image_सूचकांक].आकार);
	} अन्यथा {
		अगर (info->पढ़ो(info, offset, आकार, buf) < images[image_सूचकांक].आकार) {
				म_लिखो("%s: failed to load image to a non-secure region\n", __func__);
			वापस शून्य;
		}
	}

#अगर_घोषित CONFIG_AHAB_BOOT
	flush_dcache_range((ulong)buf, (ulong)(buf + images[image_सूचकांक].आकार - 1));
	अगर (ahab_verify_cntr_image(&images[image_सूचकांक], image_सूचकांक))
		वापस शून्य;
#पूर्ण_अगर

	वापस &images[image_सूचकांक];
}

अटल पूर्णांक read_auth_container(काष्ठा spl_image_info *spl_image,
			       काष्ठा spl_load_info *info, ulong offset)
{
	काष्ठा container_hdr *container = शून्य;
	काष्ठा container_hdr *authhdr;
	u16 लंबाई;
	पूर्णांक i, आकार, ret = 0;

	आकार = ALIGN(CONTAINER_HDR_ALIGNMENT, spl_get_bl_len(info));

	/*
	 * It will not override the ATF code, so safe to use it here,
	 * no need malloc
	 */
	container = दो_स्मृति(आकार);
	अगर (!container)
		वापस -ENOMEM;

	debug("%s: container: %p offset: %lu size: %u\n", __func__,
	      container, offset, आकार);
	अगर (info->पढ़ो(info, offset, आकार, container) <
	    CONTAINER_HDR_ALIGNMENT) {
		ret = -EIO;
		जाओ end;
	}

	अगर (!valid_container_hdr(container)) {
		log_err("Wrong container header\n");
		ret = -ENOENT;
		जाओ end;
	}

	अगर (!container->अंक_images) {
		log_err("Wrong container, no image found\n");
		ret = -ENOENT;
		जाओ end;
	}

	लंबाई = container->लंबाई_lsb + (container->लंबाई_msb << 8);
	debug("Container length %u\n", लंबाई);

	अगर (लंबाई > CONTAINER_HDR_ALIGNMENT) {
		आकार = ALIGN(लंबाई, spl_get_bl_len(info));

		मुक्त(container);
		container = दो_स्मृति(आकार);
		अगर (!container)
			वापस -ENOMEM;

		debug("%s: container: %p offset: %lu size: %u\n",
		      __func__, container, offset, आकार);
		अगर (info->पढ़ो(info, offset, आकार, container) < लंबाई) {
			ret = -EIO;
			जाओ end;
		}
	}

	authhdr = container;

#अगर_घोषित CONFIG_AHAB_BOOT
	authhdr = ahab_auth_cntr_hdr(authhdr, लंबाई);
	अगर (!authhdr) {
		ret = -EINVAL;
		जाओ end_auth;
	}
#पूर्ण_अगर

	क्रम (i = 0; i < authhdr->अंक_images; i++) {
		काष्ठा boot_img_t *image = read_auth_image(spl_image, info,
							   authhdr, i,
							   offset);

		अगर (!image) {
			ret = -EINVAL;
			जाओ end_auth;
		}

		अगर (i == 0) {
			spl_image->load_addr = image->dst;
			spl_image->entry_point = image->entry;
		}
	}

end_auth:
#अगर_घोषित CONFIG_AHAB_BOOT
	ahab_auth_release();
#पूर्ण_अगर

end:
	मुक्त(container);

	वापस ret;
}

पूर्णांक spl_load_imx_container(काष्ठा spl_image_info *spl_image,
			   काष्ठा spl_load_info *info, ulong offset)
{
	वापस read_auth_container(spl_image, info, offset);
}
