// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Stefan Roese <sr@denx.de>
 */

#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <asm/sections.h>
#ಅನ್ತರ್ಭಾವ <spl.h>

#ಅನ್ತರ್ಭಾವ <lzma/LzmaTypes.h>
#ಅನ್ತರ್ಭಾವ <lzma/LzmaDec.h>
#ಅನ್ತರ್ಭಾವ <lzma/LzmaTools.h>

#ವ್ಯಾಖ್ಯಾನಿಸು LZMA_LEN	(1 << 20)

ಸ್ಥಿರ ಶೂನ್ಯ spl_parse_legacy_validate(ulong ಆರಂಭ, ulong ಗಾತ್ರ_2)
{
	ulong spl_ಆರಂಭ = (ulong)ಆರಂಭ_2;
	ulong spl_ಕೊನೆ = (ulong)&image_ದ್ವಿಮಾನ_ಕೊನೆ;
	ulong ಕೊನೆ_2 = ಆರಂಭ + ಗಾತ್ರ_2;

	ಆದರೆ ((ಆರಂಭ >= spl_ಆರಂಭ && ಆರಂಭ < spl_ಕೊನೆ) ||
	    (ಕೊನೆ_2 > spl_ಆರಂಭ && ಕೊನೆ_2 <= spl_ಕೊನೆ) ||
	    (ಆರಂಭ < spl_ಆರಂಭ && ಕೊನೆ_2 >= spl_ಕೊನೆ) ||
	    (ಆರಂಭ > ಕೊನೆ_2 && ಕೊನೆ_2 > spl_ಆರಂಭ))
		panic("SPL: Image overlaps SPL\n");

	ಆದರೆ (ಗಾತ್ರ_2 > CONFIG_SYS_BOOTM_LEN)
		panic("SPL: Image too large\n");
}

ಪೂರ್ಣಾಂಕ spl_parse_legacy_ಶೀರ್ಷಿಕೆ(ರಚನೆ spl_image_info *spl_image,
			    ಸ್ಥಿರಾಂಕ ರಚನೆ legacy_img_hdr *ಶೀರ್ಷಿಕೆ)
{
	u32 ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ = ಗಾತ್ರ(ರಚನೆ legacy_img_hdr);

	/* check uImage header CRC */
	ಆದರೆ (IS_ENABLED(CONFIG_SPL_LEGACY_IMAGE_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_CHECK) &&
	    !image_check_hcrc(ಶೀರ್ಷಿಕೆ)) {
		puts("SPL: Image header CRC check failed!\n");
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	ಆದರೆ (spl_image->flags & SPL_COPY_PAYLOAD_ONLY) {
		/*
		 * On some system (e.g. powerpc), the load-address and
		 * entry-point is located at address 0. We can't load
		 * to 0-0x40. So skip header in this case.
		 */
		spl_image->ತುಂಬು_addr = image_get_ತುಂಬು(ಶೀರ್ಷಿಕೆ);
		spl_image->ನಮೂದು_ಬಿಂದು = image_get_ep(ಶೀರ್ಷಿಕೆ);
		spl_image->ಗಾತ್ರ_2 = image_get_ದತ್ತಾಂಶ_ಗಾತ್ರ(ಶೀರ್ಷಿಕೆ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		spl_image->ನಮೂದು_ಬಿಂದು = image_get_ep(ಶೀರ್ಷಿಕೆ);
		/* Load including the header */
		spl_image->ತುಂಬು_addr = image_get_ತುಂಬು(ಶೀರ್ಷಿಕೆ) -
			ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ;
		spl_image->ಗಾತ್ರ_2 = image_get_ದತ್ತಾಂಶ_ಗಾತ್ರ(ಶೀರ್ಷಿಕೆ) +
			ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ;
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPL_LEGACY_IMAGE_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_CHECK
	/* store uImage data length and CRC to check later */
	spl_image->dcrc_ದತ್ತಾಂಶ = image_get_ತುಂಬು(ಶೀರ್ಷಿಕೆ);
	spl_image->dcrc_ಉದ್ದಳತೆ = image_get_ದತ್ತಾಂಶ_ಗಾತ್ರ(ಶೀರ್ಷಿಕೆ);
	spl_image->dcrc = image_get_dcrc(ಶೀರ್ಷಿಕೆ);
#ಆದರೆ_ಅಂತ್ಯ

	spl_image->os = image_get_os(ಶೀರ್ಷಿಕೆ);
	spl_image->name = image_get_name(ಶೀರ್ಷಿಕೆ);
	debug(PHASE_PROMPT
	      "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->ತುಂಬು_addr, spl_image->ಗಾತ್ರ_2);

	spl_parse_legacy_validate(spl_image->ತುಂಬು_addr, spl_image->ಗಾತ್ರ_2);
	spl_parse_legacy_validate(spl_image->ನಮೂದು_ಬಿಂದು, 0);

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ spl_ತುಂಬು_legacy_lzma(ರಚನೆ spl_image_info *spl_image,
			 ರಚನೆ spl_ತುಂಬು_info *ತುಂಬು, ulong ಸ್ಥಾನಾಂತರ)
{
	ಗಾತ್ರ_T lzma_len = LZMA_LEN;
	ಶೂನ್ಯ *src;
	ulong dataptr, overhead, ಗಾತ್ರ_2;
	ಪೂರ್ಣಾಂಕ ret;

	/* dataptr points to compressed payload  */
	dataptr = ALIGN_ಕೆಳಕ್ಕೆ(ಗಾತ್ರ(ರಚನೆ legacy_img_hdr),
			     spl_get_bl_len(ತುಂಬು));
	overhead = ಗಾತ್ರ(ರಚನೆ legacy_img_hdr) - dataptr;
	ಗಾತ್ರ_2 = ALIGN(spl_image->ಗಾತ್ರ_2 + overhead, spl_get_bl_len(ತುಂಬು));
	dataptr += ಸ್ಥಾನಾಂತರ;

	debug("LZMA: Decompressing %08lx to %08lx\n",
	      dataptr, spl_image->ತುಂಬು_addr);
	src = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ_2);
	ಆದರೆ (!src) {
		ಬರೆ("Unable to allocate %d bytes for LZMA\n",
		       spl_image->ಗಾತ್ರ_2);
		ಹಿಂತಿರುಗಿ -ENOMEM;
	}

	ತುಂಬು->ಪಠ(ತುಂಬು, dataptr, ಗಾತ್ರ_2, src);
	ret = lzma_Buff_ಗಮ್ಯ_Buff_Decompress(map_sysmem(spl_image->ತುಂಬು_addr,
						  spl_image->ಗಾತ್ರ_2), &lzma_len,
				       src + overhead, spl_image->ಗಾತ್ರ_2);
	ಆದರೆ (ret) {
		ಬರೆ("LZMA decompression error: %d\n", ret);
		ಹಿಂತಿರುಗಿ ret;
	}

	spl_image->ಗಾತ್ರ_2 = lzma_len;
	ಹಿಂತಿರುಗಿ 0;
}
