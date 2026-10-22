// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Stefan Roese <sr@denx.de>
 */

#समावेश <image.h>
#समावेश <log.h>
#समावेश <malloc.h>
#समावेश <mapmem.h>
#समावेश <asm/sections.h>
#समावेश <spl.h>

#समावेश <lzma/LzmaTypes.h>
#समावेश <lzma/LzmaDec.h>
#समावेश <lzma/LzmaTools.h>

#घोषणा LZMA_LEN	(1 << 20)

अटल व्योम spl_parse_legacy_validate(ulong start, ulong आकार)
{
	ulong spl_start = (ulong)_start;
	ulong spl_end = (ulong)&image_द्विआधारी_end;
	ulong end = start + आकार;

	अगर ((start >= spl_start && start < spl_end) ||
	    (end > spl_start && end <= spl_end) ||
	    (start < spl_start && end >= spl_end) ||
	    (start > end && end > spl_start))
		panic("SPL: Image overlaps SPL\n");

	अगर (आकार > CONFIG_SYS_BOOTM_LEN)
		panic("SPL: Image too large\n");
}

पूर्णांक spl_parse_legacy_header(काष्ठा spl_image_info *spl_image,
			    स्थिर काष्ठा legacy_img_hdr *header)
{
	u32 header_आकार = माप(काष्ठा legacy_img_hdr);

	/* check uImage header CRC */
	अगर (IS_ENABLED(CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK) &&
	    !image_check_hcrc(header)) {
		माला_दो("SPL: Image header CRC check failed!\n");
		वापस -EINVAL;
	}

	अगर (spl_image->flags & SPL_COPY_PAYLOAD_ONLY) {
		/*
		 * On some system (e.g. powerpc), the load-address and
		 * entry-point is located at address 0. We can't load
		 * to 0-0x40. So skip header in this case.
		 */
		spl_image->load_addr = image_get_load(header);
		spl_image->entry_point = image_get_ep(header);
		spl_image->आकार = image_get_तथ्य_आकार(header);
	} अन्यथा {
		spl_image->entry_point = image_get_ep(header);
		/* Load including the header */
		spl_image->load_addr = image_get_load(header) -
			header_आकार;
		spl_image->आकार = image_get_तथ्य_आकार(header) +
			header_आकार;
	}

#अगर_घोषित CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK
	/* store uImage data length and CRC to check later */
	spl_image->dcrc_तथ्य = image_get_load(header);
	spl_image->dcrc_लंबाई = image_get_तथ्य_आकार(header);
	spl_image->dcrc = image_get_dcrc(header);
#पूर्ण_अगर

	spl_image->os = image_get_os(header);
	spl_image->name = image_get_name(header);
	debug(PHASE_PROMPT
	      "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->load_addr, spl_image->आकार);

	spl_parse_legacy_validate(spl_image->load_addr, spl_image->आकार);
	spl_parse_legacy_validate(spl_image->entry_point, 0);

	वापस 0;
}

पूर्णांक spl_load_legacy_lzma(काष्ठा spl_image_info *spl_image,
			 काष्ठा spl_load_info *load, ulong offset)
{
	आकार_T lzma_len = LZMA_LEN;
	व्योम *src;
	ulong dataptr, overhead, आकार;
	पूर्णांक ret;

	/* dataptr points to compressed payload  */
	dataptr = ALIGN_DOWN(माप(काष्ठा legacy_img_hdr),
			     spl_get_bl_len(load));
	overhead = माप(काष्ठा legacy_img_hdr) - dataptr;
	आकार = ALIGN(spl_image->आकार + overhead, spl_get_bl_len(load));
	dataptr += offset;

	debug("LZMA: Decompressing %08lx to %08lx\n",
	      dataptr, spl_image->load_addr);
	src = दो_स्मृति(आकार);
	अगर (!src) {
		म_लिखो("Unable to allocate %d bytes for LZMA\n",
		       spl_image->आकार);
		वापस -ENOMEM;
	}

	load->पढ़ो(load, dataptr, आकार, src);
	ret = lzmaBuffToBuffDecompress(map_sysmem(spl_image->load_addr,
						  spl_image->आकार), &lzma_len,
				       src + overhead, spl_image->आकार);
	अगर (ret) {
		म_लिखो("LZMA decompression error: %d\n", ret);
		वापस ret;
	}

	spl_image->आकार = lzma_len;
	वापस 0;
}
