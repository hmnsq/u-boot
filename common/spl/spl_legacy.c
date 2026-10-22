// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Stefan Roese <sr@denx.de>
 */

#अन्तर्भाव <image.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <mapmem.h>
#अन्तर्भाव <asm/sections.h>
#अन्तर्भाव <spl.h>

#अन्तर्भाव <lzma/LzmaTypes.h>
#अन्तर्भाव <lzma/LzmaDec.h>
#अन्तर्भाव <lzma/LzmaTools.h>

#define LZMA_LEN	(1 << 20)

स्थिर शून्य spl_parse_legacy_validate(ulong start, ulong size)
{
	ulong spl_start = (ulong)_start;
	ulong spl_end = (ulong)&image_द्विमान_end;
	ulong end = start + size;

	जइ ((start >= spl_start && start < spl_end) ||
	    (end > spl_start && end <= spl_end) ||
	    (start < spl_start && end >= spl_end) ||
	    (start > end && end > spl_start))
		panic("SPL: Image overlaps SPL\n");

	जइ (size > CONFIG_SYS_BOOTM_LEN)
		panic("SPL: Image too large\n");
}

पूर्णाङ्क spl_parse_legacy_header(संरचना spl_image_info *spl_image,
			    ध्रुव संरचना legacy_img_hdr *header)
{
	u32 header_size = परिमाण(संरचना legacy_img_hdr);

	/* check uImage header CRC */
	जइ (IS_ENABLED(CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK) &&
	    !image_check_hcrc(header)) {
		puts("SPL: Image header CRC check failed!\n");
		प्रत्यागम -EINVAL;
	}

	जइ (spl_image->flags & SPL_COPY_PAYLOAD_ONLY) {
		/*
		 * On some system (e.g. powerpc), the load-address and
		 * entry-point is located at address 0. We can't load
		 * to 0-0x40. So skip header in this case.
		 */
		spl_image->load_addr = image_get_load(header);
		spl_image->entry_point = image_get_ep(header);
		spl_image->size = image_get_data_size(header);
	} अण्णहा {
		spl_image->entry_point = image_get_ep(header);
		/* Load including the header */
		spl_image->load_addr = image_get_load(header) -
			header_size;
		spl_image->size = image_get_data_size(header) +
			header_size;
	}

#ifdef CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK
	/* store uImage data length and CRC to check later */
	spl_image->dcrc_data = image_get_load(header);
	spl_image->dcrc_length = image_get_data_size(header);
	spl_image->dcrc = image_get_dcrc(header);
#endif

	spl_image->os = image_get_os(header);
	spl_image->name = image_get_name(header);
	debug(PHASE_PROMPT
	      "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->load_addr, spl_image->size);

	spl_parse_legacy_validate(spl_image->load_addr, spl_image->size);
	spl_parse_legacy_validate(spl_image->entry_point, 0);

	प्रत्यागम 0;
}

पूर्णाङ्क spl_load_legacy_lzma(संरचना spl_image_info *spl_image,
			 संरचना spl_load_info *load, ulong offset)
{
	SizeT lzma_len = LZMA_LEN;
	शून्य *src;
	ulong dataptr, overhead, size;
	पूर्णाङ्क ret;

	/* dataptr points to compressed payload  */
	dataptr = ALIGN_DOWN(परिमाण(संरचना legacy_img_hdr),
			     spl_get_bl_len(load));
	overhead = परिमाण(संरचना legacy_img_hdr) - dataptr;
	size = ALIGN(spl_image->size + overhead, spl_get_bl_len(load));
	dataptr += offset;

	debug("LZMA: Decompressing %08lx to %08lx\n",
	      dataptr, spl_image->load_addr);
	src = malloc(size);
	जइ (!src) {
		printf("Unable to allocate %d bytes for LZMA\n",
		       spl_image->size);
		प्रत्यागम -ENOMEM;
	}

	load->पठ(load, dataptr, size, src);
	ret = lzmaBuffToBuffDecompress(map_sysmem(spl_image->load_addr,
						  spl_image->size), &lzma_len,
				       src + overhead, spl_image->size);
	जइ (ret) {
		printf("LZMA decompression error: %d\n", ret);
		प्रत्यागम ret;
	}

	spl_image->size = lzma_len;
	प्रत्यागम 0;
}
