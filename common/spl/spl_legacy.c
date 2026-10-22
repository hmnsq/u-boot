// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Stefan Roese <sr@denx.de>
 */

#അന്തര്ഭാവ <image.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <malloc.h>
#അന്തര്ഭാവ <mapmem.h>
#അന്തര്ഭാവ <asm/sections.h>
#അന്തര്ഭാവ <spl.h>

#അന്തര്ഭാവ <lzma/LzmaTypes.h>
#അന്തര്ഭാവ <lzma/LzmaDec.h>
#അന്തര്ഭാവ <lzma/LzmaTools.h>

#define LZMA_LEN	(1 << 20)

സ്ഥിതം ശൂന്യം spl_parse_legacy_validate(ulong start, ulong size)
{
	ulong spl_start = (ulong)_start;
	ulong spl_end = (ulong)&image_ദ്വിമാന_end;
	ulong end = start + size;

	എങ്കിൽ ((start >= spl_start && start < spl_end) ||
	    (end > spl_start && end <= spl_end) ||
	    (start < spl_start && end >= spl_end) ||
	    (start > end && end > spl_start))
		panic("SPL: Image overlaps SPL\n");

	എങ്കിൽ (size > CONFIG_SYS_BOOTM_LEN)
		panic("SPL: Image too large\n");
}

പൂർണ്ണസംഖ്യ spl_parse_legacy_header(ഘടന spl_image_info *spl_image,
			    സ്ഥിരം ഘടന legacy_img_hdr *header)
{
	u32 header_size = വലുപ്പം(ഘടന legacy_img_hdr);

	/* check uImage header CRC */
	എങ്കിൽ (IS_ENABLED(CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK) &&
	    !image_check_hcrc(header)) {
		puts("SPL: Image header CRC check failed!\n");
		തിരികെ -EINVAL;
	}

	എങ്കിൽ (spl_image->flags & SPL_COPY_PAYLOAD_ONLY) {
		/*
		 * On some system (e.g. powerpc), the load-address and
		 * entry-point is located at address 0. We can't load
		 * to 0-0x40. So skip header in this case.
		 */
		spl_image->load_addr = image_get_load(header);
		spl_image->entry_point = image_get_ep(header);
		spl_image->size = image_get_data_size(header);
	} അല്ലെങ്കിൽ {
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

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ spl_load_legacy_lzma(ഘടന spl_image_info *spl_image,
			 ഘടന spl_load_info *load, ulong offset)
{
	SizeT lzma_len = LZMA_LEN;
	ശൂന്യം *src;
	ulong dataptr, overhead, size;
	പൂർണ്ണസംഖ്യ ret;

	/* dataptr points to compressed payload  */
	dataptr = ALIGN_DOWN(വലുപ്പം(ഘടന legacy_img_hdr),
			     spl_get_bl_len(load));
	overhead = വലുപ്പം(ഘടന legacy_img_hdr) - dataptr;
	size = ALIGN(spl_image->size + overhead, spl_get_bl_len(load));
	dataptr += offset;

	debug("LZMA: Decompressing %08lx to %08lx\n",
	      dataptr, spl_image->load_addr);
	src = malloc(size);
	എങ്കിൽ (!src) {
		printf("Unable to allocate %d bytes for LZMA\n",
		       spl_image->size);
		തിരികെ -ENOMEM;
	}

	load->പഠ(load, dataptr, size, src);
	ret = lzmaBuffToBuffDecompress(map_sysmem(spl_image->load_addr,
						  spl_image->size), &lzma_len,
				       src + overhead, spl_image->size);
	എങ്കിൽ (ret) {
		printf("LZMA decompression error: %d\n", ret);
		തിരികെ ret;
	}

	spl_image->size = lzma_len;
	തിരികെ 0;
}
