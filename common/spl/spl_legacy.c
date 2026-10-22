// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Stefan Roese <sr@denx.de>
 */

#ضمن <image.h>
#ضمن <log.h>
#ضمن <malloc.h>
#ضمن <mapmem.h>
#ضمن <asm/sections.h>
#ضمن <spl.h>

#ضمن <lzma/LzmaTypes.h>
#ضمن <lzma/LzmaDec.h>
#ضمن <lzma/LzmaTools.h>

#تعریف_کرو LZMA_LEN	(1 << 20)

ساكن فراغ spl_parse_legacy_validate(ulong شروع, ulong ناپ)
{
	ulong spl_شروع = (ulong)شروع_2;
	ulong spl_سرا = (ulong)&image_ثنائی_سرا;
	ulong سرا_2 = شروع + ناپ;

	إذا ((شروع >= spl_شروع && شروع < spl_سرا) ||
	    (سرا_2 > spl_شروع && سرا_2 <= spl_سرا) ||
	    (شروع < spl_شروع && سرا_2 >= spl_سرا) ||
	    (شروع > سرا_2 && سرا_2 > spl_شروع))
		panic("SPL: Image overlaps SPL\n");

	إذا (ناپ > CONFIG_SYS_BOOTM_LEN)
		panic("SPL: Image too large\n");
}

صحيح spl_parse_legacy_سرخی(بنية spl_image_info *spl_image,
			    ثابت بنية legacy_img_hdr *سرخی)
{
	u32 سرخی_ناپ = حجم(بنية legacy_img_hdr);

	/* check uImage header CRC */
	إذا (IS_ENABLED(CONFIG_SPL_LEGACY_IMAGE_چکری_جانچ_CHECK) &&
	    !image_check_hcrc(سرخی)) {
		puts("SPL: Image header CRC check failed!\n");
		أرجع -EINVAL;
	}

	إذا (spl_image->flags & SPL_COPY_PAYLOAD_ONLY) {
		/*
		 * On some system (e.g. powerpc), the load-address and
		 * entry-point is located at address 0. We can't load
		 * to 0-0x40. So skip header in this case.
		 */
		spl_image->لدو_addr = image_get_لدو(سرخی);
		spl_image->اندراج_نقطہ = image_get_ep(سرخی);
		spl_image->ناپ = image_get_انکڑے_ناپ(سرخی);
	} وإلا {
		spl_image->اندراج_نقطہ = image_get_ep(سرخی);
		/* Load including the header */
		spl_image->لدو_addr = image_get_لدو(سرخی) -
			سرخی_ناپ;
		spl_image->ناپ = image_get_انکڑے_ناپ(سرخی) +
			سرخی_ناپ;
	}

#جے_تعریف_شدہ CONFIG_SPL_LEGACY_IMAGE_چکری_جانچ_CHECK
	/* store uImage data length and CRC to check later */
	spl_image->dcrc_انکڑے = image_get_لدو(سرخی);
	spl_image->dcrc_لمائی = image_get_انکڑے_ناپ(سرخی);
	spl_image->dcrc = image_get_dcrc(سرخی);
#جے_انت

	spl_image->os = image_get_os(سرخی);
	spl_image->name = image_get_name(سرخی);
	debug(PHASE_PROMPT
	      "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->لدو_addr, spl_image->ناپ);

	spl_parse_legacy_validate(spl_image->لدو_addr, spl_image->ناپ);
	spl_parse_legacy_validate(spl_image->اندراج_نقطہ, 0);

	أرجع 0;
}

صحيح spl_لدو_legacy_lzma(بنية spl_image_info *spl_image,
			 بنية spl_لدو_info *لدو, ulong وتھ)
{
	ناپ_T lzma_len = LZMA_LEN;
	فراغ *src;
	ulong dataptr, overhead, ناپ;
	صحيح ret;

	/* dataptr points to compressed payload  */
	dataptr = ALIGN_تھلے(حجم(بنية legacy_img_hdr),
			     spl_get_bl_len(لدو));
	overhead = حجم(بنية legacy_img_hdr) - dataptr;
	ناپ = ALIGN(spl_image->ناپ + overhead, spl_get_bl_len(لدو));
	dataptr += وتھ;

	debug("LZMA: Decompressing %08lx to %08lx\n",
	      dataptr, spl_image->لدو_addr);
	src = حافظہ_دیو(ناپ);
	إذا (!src) {
		لکھو("Unable to allocate %d bytes for LZMA\n",
		       spl_image->ناپ);
		أرجع -ENOMEM;
	}

	لدو->اقرأ(لدو, dataptr, ناپ, src);
	ret = lzma_Buff_منزل_Buff_Decompress(map_sysmem(spl_image->لدو_addr,
						  spl_image->ناپ), &lzma_len,
				       src + overhead, spl_image->ناپ);
	إذا (ret) {
		لکھو("LZMA decompression error: %d\n", ret);
		أرجع ret;
	}

	spl_image->ناپ = lzma_len;
	أرجع 0;
}
