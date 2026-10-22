// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Stefan Roese <sr@denx.de>
 */

#অন্তর্ভাৱ <image.h>
#অন্তর্ভাৱ <log.h>
#অন্তর্ভাৱ <malloc.h>
#অন্তর্ভাৱ <mapmem.h>
#অন্তর্ভাৱ <asm/sections.h>
#অন্তর্ভাৱ <spl.h>

#অন্তর্ভাৱ <lzma/LzmaTypes.h>
#অন্তর্ভাৱ <lzma/LzmaDec.h>
#অন্তর্ভাৱ <lzma/LzmaTools.h>

#define LZMA_LEN	(1 << 20)

স্থির শূন্য spl_parse_legacy_validate(ulong start, ulong size)
{
	ulong spl_start = (ulong)_start;
	ulong spl_end = (ulong)&image_দ্বিমিক_end;
	ulong end = start + size;

	যদি ((start >= spl_start && start < spl_end) ||
	    (end > spl_start && end <= spl_end) ||
	    (start < spl_start && end >= spl_end) ||
	    (start > end && end > spl_start))
		panic("SPL: Image overlaps SPL\n");

	যদি (size > CONFIG_SYS_BOOTM_LEN)
		panic("SPL: Image too large\n");
}

পূর্ণাঙ্ক spl_parse_legacy_header(সংরচনা spl_image_info *spl_image,
			    ধ্রুৱ সংরচনা legacy_img_hdr *header)
{
	u32 header_size = পরিমাণ(সংরচনা legacy_img_hdr);

	/* check uImage header CRC */
	যদি (IS_ENABLED(CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK) &&
	    !image_check_hcrc(header)) {
		puts("SPL: Image header CRC check failed!\n");
		ঘূৰাই -EINVAL;
	}

	যদি (spl_image->flags & SPL_COPY_PAYLOAD_ONLY) {
		/*
		 * On some system (e.g. powerpc), the load-address and
		 * entry-point is located at address 0. We can't load
		 * to 0-0x40. So skip header in this case.
		 */
		spl_image->load_addr = image_get_load(header);
		spl_image->entry_point = image_get_ep(header);
		spl_image->size = image_get_data_size(header);
	} অন্যথা {
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

	ঘূৰাই 0;
}

পূর্ণাঙ্ক spl_load_legacy_lzma(সংরচনা spl_image_info *spl_image,
			 সংরচনা spl_load_info *load, ulong offset)
{
	SizeT lzma_len = LZMA_LEN;
	শূন্য *src;
	ulong dataptr, overhead, size;
	পূর্ণাঙ্ক ret;

	/* dataptr points to compressed payload  */
	dataptr = ALIGN_DOWN(পরিমাণ(সংরচনা legacy_img_hdr),
			     spl_get_bl_len(load));
	overhead = পরিমাণ(সংরচনা legacy_img_hdr) - dataptr;
	size = ALIGN(spl_image->size + overhead, spl_get_bl_len(load));
	dataptr += offset;

	debug("LZMA: Decompressing %08lx to %08lx\n",
	      dataptr, spl_image->load_addr);
	src = malloc(size);
	যদি (!src) {
		printf("Unable to allocate %d bytes for LZMA\n",
		       spl_image->size);
		ঘূৰাই -ENOMEM;
	}

	load->পঠ(load, dataptr, size, src);
	ret = lzmaBuffToBuffDecompress(map_sysmem(spl_image->load_addr,
						  spl_image->size), &lzma_len,
				       src + overhead, spl_image->size);
	যদি (ret) {
		printf("LZMA decompression error: %d\n", ret);
		ঘূৰাই ret;
	}

	spl_image->size = lzma_len;
	ঘূৰাই 0;
}
