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

#परिभाषय LZMA_LEN	(1 << 20)

स्थिर शून्य spl_parse_legacy_validate(ulong start, ulong आकार)
{
	ulong spl_start = (ulong)_start;
	ulong spl_अन्तः = (ulong)&image_द्विमान_अन्तः;
	ulong अन्तः_2 = start + आकार;

	यदि ((start >= spl_start && start < spl_अन्तः) ||
	    (अन्तः_2 > spl_start && अन्तः_2 <= spl_अन्तः) ||
	    (start < spl_start && अन्तः_2 >= spl_अन्तः) ||
	    (start > अन्तः_2 && अन्तः_2 > spl_start))
		panic("SPL: Image overlaps SPL\n");

	यदि (आकार > CONFIG_SYS_BOOTM_LEN)
		panic("SPL: Image too large\n");
}

पूर्णाङ्क spl_parse_legacy_शीर्षकम्(संरचना spl_image_info *spl_image,
			    ध्रुव संरचना legacy_img_hdr *शीर्षकम्)
{
	u32 शीर्षकम्_आकार = परिमाण(संरचना legacy_img_hdr);

	/* check uImage header CRC */
	यदि (IS_ENABLED(CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK) &&
	    !image_check_hcrc(शीर्षकम्)) {
		पङ्क्तिं_लिख("SPL: Image header CRC check failed!\n");
		प्रत्यागम -EINVAL;
	}

	यदि (spl_image->flags & SPL_COPY_PAYLOAD_ONLY) {
		/*
		 * On some system (e.g. powerpc), the load-address and
		 * entry-point is located at address 0. We can't load
		 * to 0-0x40. So skip header in this case.
		 */
		spl_image->आरोपय_addr = image_get_आरोपय(शीर्षकम्);
		spl_image->प्रविष्टिः_point = image_get_ep(शीर्षकम्);
		spl_image->आकार = image_get_दत्त_आकार(शीर्षकम्);
	} अन्यथा {
		spl_image->प्रविष्टिः_point = image_get_ep(शीर्षकम्);
		/* Load including the header */
		spl_image->आरोपय_addr = image_get_आरोपय(शीर्षकम्) -
			शीर्षकम्_आकार;
		spl_image->आकार = image_get_दत्त_आकार(शीर्षकम्) +
			शीर्षकम्_आकार;
	}

#यदि_परिभाषितम् CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK
	/* store uImage data length and CRC to check later */
	spl_image->dcrc_दत्त = image_get_आरोपय(शीर्षकम्);
	spl_image->dcrc_दीर्घता = image_get_दत्त_आकार(शीर्षकम्);
	spl_image->dcrc = image_get_dcrc(शीर्षकम्);
#यदि_अन्तः

	spl_image->os = image_get_os(शीर्षकम्);
	spl_image->name = image_get_name(शीर्षकम्);
	debug(PHASE_PROMPT
	      "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->आरोपय_addr, spl_image->आकार);

	spl_parse_legacy_validate(spl_image->आरोपय_addr, spl_image->आकार);
	spl_parse_legacy_validate(spl_image->प्रविष्टिः_point, 0);

	प्रत्यागम 0;
}

पूर्णाङ्क spl_आरोपय_legacy_lzma(संरचना spl_image_info *spl_image,
			 संरचना spl_आरोपय_info *आरोपय, ulong विस्थापनम्)
{
	आकार_T lzma_len = LZMA_LEN;
	शून्य *src;
	ulong dataptr, overhead, आकार;
	पूर्णाङ्क ret;

	/* dataptr points to compressed payload  */
	dataptr = ALIGN_अधस्तात्(परिमाण(संरचना legacy_img_hdr),
			     spl_get_bl_len(आरोपय));
	overhead = परिमाण(संरचना legacy_img_hdr) - dataptr;
	आकार = ALIGN(spl_image->आकार + overhead, spl_get_bl_len(आरोपय));
	dataptr += विस्थापनम्;

	debug("LZMA: Decompressing %08lx to %08lx\n",
	      dataptr, spl_image->आरोपय_addr);
	src = स्मृतिं_देहि(आकार);
	यदि (!src) {
		प्रारूप_लिख("Unable to allocate %d bytes for LZMA\n",
		       spl_image->आकार);
		प्रत्यागम -ENOMEM;
	}

	आरोपय->पठ(आरोपय, dataptr, आकार, src);
	ret = lzma_Buff_गन्तव्यम्_Buff_Decompress(map_sysmem(spl_image->आरोपय_addr,
						  spl_image->आकार), &lzma_len,
				       src + overhead, spl_image->आकार);
	यदि (ret) {
		प्रारूप_लिख("LZMA decompression error: %d\n", ret);
		प्रत्यागम ret;
	}

	spl_image->आकार = lzma_len;
	प्रत्यागम 0;
}
