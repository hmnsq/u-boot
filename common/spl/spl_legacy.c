// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Stefan Roese <sr@denx.de>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mapmem.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/sections.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <lzma/LzmaTypes.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <lzma/LzmaDec.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <lzma/LzmaTools.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LZMA_LEN	(1 << 20)

ਸਥਿਰ ਖਾਲੀ spl_parse_legacy_validate(ulong ਸ਼ੁਰੂ, ulong ਮਾਪ)
{
	ulong spl_ਸ਼ੁਰੂ = (ulong)ਸ਼ੁਰੂ_2;
	ulong spl_ਸਿਰਾ = (ulong)&image_ਦੁਆਧਾਰੀ_ਸਿਰਾ;
	ulong ਸਿਰਾ_2 = ਸ਼ੁਰੂ + ਮਾਪ;

	ਜੇ ((ਸ਼ੁਰੂ >= spl_ਸ਼ੁਰੂ && ਸ਼ੁਰੂ < spl_ਸਿਰਾ) ||
	    (ਸਿਰਾ_2 > spl_ਸ਼ੁਰੂ && ਸਿਰਾ_2 <= spl_ਸਿਰਾ) ||
	    (ਸ਼ੁਰੂ < spl_ਸ਼ੁਰੂ && ਸਿਰਾ_2 >= spl_ਸਿਰਾ) ||
	    (ਸ਼ੁਰੂ > ਸਿਰਾ_2 && ਸਿਰਾ_2 > spl_ਸ਼ੁਰੂ))
		panic("SPL: Image overlaps SPL\n");

	ਜੇ (ਮਾਪ > CONFIG_SYS_BOOTM_LEN)
		panic("SPL: Image too large\n");
}

ਪੂਰਨ spl_parse_legacy_ਸਿਰਲੇਖ(ਬਣਤਰ spl_image_info *spl_image,
			    ਅਚਲ ਬਣਤਰ legacy_img_hdr *ਸਿਰਲੇਖ)
{
	u32 ਸਿਰਲੇਖ_ਮਾਪ = ਆਕਾਰ(ਬਣਤਰ legacy_img_hdr);

	/* check uImage header CRC */
	ਜੇ (IS_ENABLED(CONFIG_SPL_LEGACY_IMAGE_ਚੱਕਰੀ_ਜਾਂਚ_CHECK) &&
	    !image_check_hcrc(ਸਿਰਲੇਖ)) {
		puts("SPL: Image header CRC check failed!\n");
		ਵਾਪਸ -EINVAL;
	}

	ਜੇ (spl_image->flags & SPL_COPY_PAYLOAD_ONLY) {
		/*
		 * On some system (e.g. powerpc), the load-address and
		 * entry-point is located at address 0. We can't load
		 * to 0-0x40. So skip header in this case.
		 */
		spl_image->ਲੱਦੋ_addr = image_get_ਲੱਦੋ(ਸਿਰਲੇਖ);
		spl_image->ਇੰਦਰਾਜ_ਬਿੰਦੂ = image_get_ep(ਸਿਰਲੇਖ);
		spl_image->ਮਾਪ = image_get_ਅੰਕੜੇ_ਮਾਪ(ਸਿਰਲੇਖ);
	} ਨਹੀਂਤਾਂ {
		spl_image->ਇੰਦਰਾਜ_ਬਿੰਦੂ = image_get_ep(ਸਿਰਲੇਖ);
		/* Load including the header */
		spl_image->ਲੱਦੋ_addr = image_get_ਲੱਦੋ(ਸਿਰਲੇਖ) -
			ਸਿਰਲੇਖ_ਮਾਪ;
		spl_image->ਮਾਪ = image_get_ਅੰਕੜੇ_ਮਾਪ(ਸਿਰਲੇਖ) +
			ਸਿਰਲੇਖ_ਮਾਪ;
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPL_LEGACY_IMAGE_ਚੱਕਰੀ_ਜਾਂਚ_CHECK
	/* store uImage data length and CRC to check later */
	spl_image->dcrc_ਅੰਕੜੇ = image_get_ਲੱਦੋ(ਸਿਰਲੇਖ);
	spl_image->dcrc_ਲੰਮਾਈ = image_get_ਅੰਕੜੇ_ਮਾਪ(ਸਿਰਲੇਖ);
	spl_image->dcrc = image_get_dcrc(ਸਿਰਲੇਖ);
#ਜੇ_ਅੰਤ

	spl_image->os = image_get_os(ਸਿਰਲੇਖ);
	spl_image->name = image_get_name(ਸਿਰਲੇਖ);
	debug(PHASE_PROMPT
	      "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->ਲੱਦੋ_addr, spl_image->ਮਾਪ);

	spl_parse_legacy_validate(spl_image->ਲੱਦੋ_addr, spl_image->ਮਾਪ);
	spl_parse_legacy_validate(spl_image->ਇੰਦਰਾਜ_ਬਿੰਦੂ, 0);

	ਵਾਪਸ 0;
}

ਪੂਰਨ spl_ਲੱਦੋ_legacy_lzma(ਬਣਤਰ spl_image_info *spl_image,
			 ਬਣਤਰ spl_ਲੱਦੋ_info *ਲੱਦੋ, ulong ਵਿੱਥ)
{
	ਮਾਪ_T lzma_len = LZMA_LEN;
	ਖਾਲੀ *src;
	ulong dataptr, overhead, ਮਾਪ;
	ਪੂਰਨ ret;

	/* dataptr points to compressed payload  */
	dataptr = ALIGN_ਥੱਲੇ(ਆਕਾਰ(ਬਣਤਰ legacy_img_hdr),
			     spl_get_bl_len(ਲੱਦੋ));
	overhead = ਆਕਾਰ(ਬਣਤਰ legacy_img_hdr) - dataptr;
	ਮਾਪ = ALIGN(spl_image->ਮਾਪ + overhead, spl_get_bl_len(ਲੱਦੋ));
	dataptr += ਵਿੱਥ;

	debug("LZMA: Decompressing %08lx to %08lx\n",
	      dataptr, spl_image->ਲੱਦੋ_addr);
	src = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਮਾਪ);
	ਜੇ (!src) {
		ਲਿਖੋ("Unable to allocate %d bytes for LZMA\n",
		       spl_image->ਮਾਪ);
		ਵਾਪਸ -ENOMEM;
	}

	ਲੱਦੋ->ਪਠ(ਲੱਦੋ, dataptr, ਮਾਪ, src);
	ret = lzma_Buff_ਮੰਜ਼ਿਲ_Buff_Decompress(map_sysmem(spl_image->ਲੱਦੋ_addr,
						  spl_image->ਮਾਪ), &lzma_len,
				       src + overhead, spl_image->ਮਾਪ);
	ਜੇ (ret) {
		ਲਿਖੋ("LZMA decompression error: %d\n", ret);
		ਵਾਪਸ ret;
	}

	spl_image->ਮਾਪ = lzma_len;
	ਵਾਪਸ 0;
}
