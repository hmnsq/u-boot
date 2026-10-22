// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Stefan Roese <sr@denx.de>
 */

#include <image.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <asm/sections.h>
#include <spl.h>

#include <lzma/LzmaTypes.h>
#include <lzma/LzmaDec.h>
#include <lzma/LzmaTools.h>

#define LZMA_LEN	(1 << 20)

static void spl_parse_legacy_validate(ulong start, ulong size)
{
	ulong spl_start = (ulong)_start;
	ulong spl_end = (ulong)&_image_binary_end;
	ulong end = start + size;

	ᬬᬾᬦ᭄ ((start >= spl_start && start < spl_end) ||
	    (end > spl_start && end <= spl_end) ||
	    (start < spl_start && end >= spl_end) ||
	    (start > end && end > spl_start))
		panic("SPL: Image overlaps SPL\n");

	ᬬᬾᬦ᭄ (size > CONFIG_SYS_BOOTM_LEN)
		panic("SPL: Image too large\n");
}

puur_naa_nka spl_parse_legacy_header(sa_nracanaa spl_image_info *spl_image,
			    const sa_nracanaa legacy_img_hdr *header)
{
	u32 header_size = parimaa_na(sa_nracanaa legacy_img_hdr);

	/* check uImage header CRC */
	ᬬᬾᬦ᭄ (IS_ENABLED(CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK) &&
	    !image_check_hcrc(header)) {
		puts("SPL: Image header CRC check failed!\n");
		ᬯᬮᬶ -EINVAL;
	}

	ᬬᬾᬦ᭄ (spl_image->flags & SPL_COPY_PAYLOAD_ONLY) {
		/*
		 * On some system (e.g. powerpc), the load-address and
		 * entry-point is located at address 0. We can't load
		 * to 0-0x40. So skip header in this case.
		 */
		spl_image->load_addr = image_get_load(header);
		spl_image->entry_point = image_get_ep(header);
		spl_image->size = image_get_data_size(header);
	} ᬮᬶᬬ {
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

	ᬯᬮᬶ 0;
}

puur_naa_nka spl_load_legacy_lzma(sa_nracanaa spl_image_info *spl_image,
			 sa_nracanaa spl_load_info *load, ulong offset)
{
	SizeT lzma_len = LZMA_LEN;
	void *src;
	ulong dataptr, overhead, size;
	puur_naa_nka ret;

	/* dataptr points to compressed payload  */
	dataptr = ALIGN_DOWN(parimaa_na(sa_nracanaa legacy_img_hdr),
			     spl_get_bl_len(load));
	overhead = parimaa_na(sa_nracanaa legacy_img_hdr) - dataptr;
	size = ALIGN(spl_image->size + overhead, spl_get_bl_len(load));
	dataptr += offset;

	debug("LZMA: Decompressing %08lx to %08lx\n",
	      dataptr, spl_image->load_addr);
	src = malloc(size);
	ᬬᬾᬦ᭄ (!src) {
		printf("Unable to allocate %d bytes for LZMA\n",
		       spl_image->size);
		ᬯᬮᬶ -ENOMEM;
	}

	load->read(load, dataptr, size, src);
	ret = lzmaBuffToBuffDecompress(map_sysmem(spl_image->load_addr,
						  spl_image->size), &lzma_len,
				       src + overhead, spl_image->size);
	ᬬᬾᬦ᭄ (ret) {
		printf("LZMA decompression error: %d\n", ret);
		ᬯᬮᬶ ret;
	}

	spl_image->size = lzma_len;
	ᬯᬮᬶ 0;
}
