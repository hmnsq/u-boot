// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Stefan Roese <sr@denx.de>
 */

#অন্তর্ভাব <image.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <malloc.h>
#অন্তর্ভাব <mapmem.h>
#অন্তর্ভাব <asm/sections.h>
#অন্তর্ভাব <spl.h>

#অন্তর্ভাব <lzma/LzmaTypes.h>
#অন্তর্ভাব <lzma/LzmaDec.h>
#অন্তর্ভাব <lzma/LzmaTools.h>

#define LZMA_LEN	(1 << 20)

স্থির শূন্য spl_parse_legacy_validate(ulong start, ulong আকার_2)
{
	ulong spl_start = (ulong)_start;
	ulong spl_end = (ulong)&image_দ্বিমিক_end;
	ulong end = start + আকার_2;

	যদি ((start >= spl_start && start < spl_end) ||
	    (end > spl_start && end <= spl_end) ||
	    (start < spl_start && end >= spl_end) ||
	    (start > end && end > spl_start))
		panic("SPL: Image overlaps SPL\n");

	যদি (আকার_2 > CONFIG_SYS_BOOTM_LEN)
		panic("SPL: Image too large\n");
}

পূর্ণসংখ্যা spl_parse_legacy_header(কাঠামো spl_image_info *spl_image,
			    ধ্রুবক কাঠামো legacy_img_hdr *header)
{
	u32 header_আকার = আকার(কাঠামো legacy_img_hdr);

	/* check uImage header CRC */
	যদি (IS_ENABLED(CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK) &&
	    !image_check_hcrc(header)) {
		puts("SPL: Image header CRC check failed!\n");
		ফেরত -EINVAL;
	}

	যদি (spl_image->flags & SPL_COPY_PAYLOAD_ONLY) {
		/*
		 * On some system (e.g. powerpc), the load-address and
		 * entry-point is located at address 0. We can't load
		 * to 0-0x40. So skip header in this case.
		 */
		spl_image->load_addr = image_get_load(header);
		spl_image->entry_point = image_get_ep(header);
		spl_image->আকার_2 = image_get_তথ্য_আকার(header);
	} অন্যথা {
		spl_image->entry_point = image_get_ep(header);
		/* Load including the header */
		spl_image->load_addr = image_get_load(header) -
			header_আকার;
		spl_image->আকার_2 = image_get_তথ্য_আকার(header) +
			header_আকার;
	}

#ifdef CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK
	/* store uImage data length and CRC to check later */
	spl_image->dcrc_তথ্য = image_get_load(header);
	spl_image->dcrc_দৈর্ঘ্য = image_get_তথ্য_আকার(header);
	spl_image->dcrc = image_get_dcrc(header);
#endif

	spl_image->os = image_get_os(header);
	spl_image->name = image_get_name(header);
	debug(PHASE_PROMPT
	      "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->load_addr, spl_image->আকার_2);

	spl_parse_legacy_validate(spl_image->load_addr, spl_image->আকার_2);
	spl_parse_legacy_validate(spl_image->entry_point, 0);

	ফেরত 0;
}

পূর্ণসংখ্যা spl_load_legacy_lzma(কাঠামো spl_image_info *spl_image,
			 কাঠামো spl_load_info *load, ulong offset)
{
	আকার_T lzma_len = LZMA_LEN;
	শূন্য *src;
	ulong dataptr, overhead, আকার_2;
	পূর্ণসংখ্যা ret;

	/* dataptr points to compressed payload  */
	dataptr = ALIGN_DOWN(আকার(কাঠামো legacy_img_hdr),
			     spl_get_bl_len(load));
	overhead = আকার(কাঠামো legacy_img_hdr) - dataptr;
	আকার_2 = ALIGN(spl_image->আকার_2 + overhead, spl_get_bl_len(load));
	dataptr += offset;

	debug("LZMA: Decompressing %08lx to %08lx\n",
	      dataptr, spl_image->load_addr);
	src = malloc(আকার_2);
	যদি (!src) {
		printf("Unable to allocate %d bytes for LZMA\n",
		       spl_image->আকার_2);
		ফেরত -ENOMEM;
	}

	load->পঠ(load, dataptr, আকার_2, src);
	ret = lzmaBuffToBuffDecompress(map_sysmem(spl_image->load_addr,
						  spl_image->আকার_2), &lzma_len,
				       src + overhead, spl_image->আকার_2);
	যদি (ret) {
		printf("LZMA decompression error: %d\n", ret);
		ফেরত ret;
	}

	spl_image->আকার_2 = lzma_len;
	ফেরত 0;
}
