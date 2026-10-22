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

#تعریف_کن LZMA_LEN	(1 << 20)

ساكن فراغ spl_parse_legacy_validate(ulong شروع, ulong اندازه)
{
	ulong spl_شروع = (ulong)شروع_2;
	ulong spl_انتها = (ulong)&image_دودویی_انتها;
	ulong انتها_2 = شروع + اندازه;

	إذا ((شروع >= spl_شروع && شروع < spl_انتها) ||
	    (انتها_2 > spl_شروع && انتها_2 <= spl_انتها) ||
	    (شروع < spl_شروع && انتها_2 >= spl_انتها) ||
	    (شروع > انتها_2 && انتها_2 > spl_شروع))
		panic("SPL: Image overlaps SPL\n");

	إذا (اندازه > CONFIG_SYS_BOOTM_LEN)
		panic("SPL: Image too large\n");
}

صحيح spl_parse_legacy_سرآیند(بنية spl_image_info *spl_image,
			    ثابت بنية legacy_img_hdr *سرآیند)
{
	u32 سرآیند_اندازه = حجم(بنية legacy_img_hdr);

	/* check uImage header CRC */
	إذا (IS_ENABLED(CONFIG_SPL_LEGACY_IMAGE_بررسی_چرخهای_CHECK) &&
	    !image_check_hcrc(سرآیند)) {
		puts("SPL: Image header CRC check failed!\n");
		أرجع -EINVAL;
	}

	إذا (spl_image->flags & SPL_COPY_PAYLOAD_ONLY) {
		/*
		 * On some system (e.g. powerpc), the load-address and
		 * entry-point is located at address 0. We can't load
		 * to 0-0x40. So skip header in this case.
		 */
		spl_image->بارگیری_addr = image_get_بارگیری(سرآیند);
		spl_image->مدخل_نقطه = image_get_ep(سرآیند);
		spl_image->اندازه = image_get_داده_اندازه(سرآیند);
	} وإلا {
		spl_image->مدخل_نقطه = image_get_ep(سرآیند);
		/* Load including the header */
		spl_image->بارگیری_addr = image_get_بارگیری(سرآیند) -
			سرآیند_اندازه;
		spl_image->اندازه = image_get_داده_اندازه(سرآیند) +
			سرآیند_اندازه;
	}

#اگر_تعریف_شده CONFIG_SPL_LEGACY_IMAGE_بررسی_چرخهای_CHECK
	/* store uImage data length and CRC to check later */
	spl_image->dcrc_داده = image_get_بارگیری(سرآیند);
	spl_image->dcrc_درازی = image_get_داده_اندازه(سرآیند);
	spl_image->dcrc = image_get_dcrc(سرآیند);
#پایان_اگر

	spl_image->os = image_get_os(سرآیند);
	spl_image->name = image_get_name(سرآیند);
	debug(PHASE_PROMPT
	      "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->بارگیری_addr, spl_image->اندازه);

	spl_parse_legacy_validate(spl_image->بارگیری_addr, spl_image->اندازه);
	spl_parse_legacy_validate(spl_image->مدخل_نقطه, 0);

	أرجع 0;
}

صحيح spl_بارگیری_legacy_lzma(بنية spl_image_info *spl_image,
			 بنية spl_بارگیری_info *بارگیری, ulong جابه_جایی)
{
	اندازه_T lzma_len = LZMA_LEN;
	فراغ *src;
	ulong dataptr, overhead, اندازه;
	صحيح ret;

	/* dataptr points to compressed payload  */
	dataptr = ALIGN_پایین(حجم(بنية legacy_img_hdr),
			     spl_get_bl_len(بارگیری));
	overhead = حجم(بنية legacy_img_hdr) - dataptr;
	اندازه = ALIGN(spl_image->اندازه + overhead, spl_get_bl_len(بارگیری));
	dataptr += جابه_جایی;

	debug("LZMA: Decompressing %08lx to %08lx\n",
	      dataptr, spl_image->بارگیری_addr);
	src = حافظه_بده(اندازه);
	إذا (!src) {
		چاپ_کن("Unable to allocate %d bytes for LZMA\n",
		       spl_image->اندازه);
		أرجع -ENOMEM;
	}

	بارگیری->اقرأ(بارگیری, dataptr, اندازه, src);
	ret = lzma_Buff_مقصد_Buff_Decompress(map_sysmem(spl_image->بارگیری_addr,
						  spl_image->اندازه), &lzma_len,
				       src + overhead, spl_image->اندازه);
	إذا (ret) {
		چاپ_کن("LZMA decompression error: %d\n", ret);
		أرجع ret;
	}

	spl_image->اندازه = lzma_len;
	أرجع 0;
}
