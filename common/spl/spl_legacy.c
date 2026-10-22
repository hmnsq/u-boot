// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Stefan Roese <sr@denx.de>
 */

#شامل <image.h>
#شامل <log.h>
#شامل <malloc.h>
#شامل <mapmem.h>
#شامل <asm/sections.h>
#شامل <spl.h>

#شامل <lzma/LzmaTypes.h>
#شامل <lzma/LzmaDec.h>
#شامل <lzma/LzmaTools.h>

#تعریف_کرو LZMA_LEN	(1 << 20)

ساکن خالی spl_parse_legacy_validate(ulong آغاز, ulong سائز)
{
	ulong spl_آغاز = (ulong)آغاز_2;
	ulong spl_انتہا = (ulong)&image_ثنائی_انتہا;
	ulong انتہا_2 = آغاز + سائز;

	اگر ((آغاز >= spl_آغاز && آغاز < spl_انتہا) ||
	    (انتہا_2 > spl_آغاز && انتہا_2 <= spl_انتہا) ||
	    (آغاز < spl_آغاز && انتہا_2 >= spl_انتہا) ||
	    (آغاز > انتہا_2 && انتہا_2 > spl_آغاز))
		panic("SPL: Image overlaps SPL\n");

	اگر (سائز > CONFIG_SYS_BOOTM_LEN)
		panic("SPL: Image too large\n");
}

صحیح spl_parse_legacy_سرخی(ساخت spl_image_info *spl_image,
			    ثابت ساخت legacy_img_hdr *سرخی)
{
	u32 سرخی_سائز = حجم(ساخت legacy_img_hdr);

	/* check uImage header CRC */
	اگر (IS_ENABLED(CONFIG_SPL_LEGACY_IMAGE_چکری_جانچ_CHECK) &&
	    !image_check_hcrc(سرخی)) {
		puts("SPL: Image header CRC check failed!\n");
		واپس -EINVAL;
	}

	اگر (spl_image->flags & SPL_COPY_PAYLOAD_ONLY) {
		/*
		 * On some system (e.g. powerpc), the load-address and
		 * entry-point is located at address 0. We can't load
		 * to 0-0x40. So skip header in this case.
		 */
		spl_image->بھرو_addr = image_get_بھرو(سرخی);
		spl_image->اندراج_نقطہ = image_get_ep(سرخی);
		spl_image->سائز = image_get_ڈیٹا_سائز(سرخی);
	} ورنہ {
		spl_image->اندراج_نقطہ = image_get_ep(سرخی);
		/* Load including the header */
		spl_image->بھرو_addr = image_get_بھرو(سرخی) -
			سرخی_سائز;
		spl_image->سائز = image_get_ڈیٹا_سائز(سرخی) +
			سرخی_سائز;
	}

#اگر_تعریف_شدہ CONFIG_SPL_LEGACY_IMAGE_چکری_جانچ_CHECK
	/* store uImage data length and CRC to check later */
	spl_image->dcrc_ڈیٹا = image_get_بھرو(سرخی);
	spl_image->dcrc_لمبائی = image_get_ڈیٹا_سائز(سرخی);
	spl_image->dcrc = image_get_dcrc(سرخی);
#اگر_ختم

	spl_image->os = image_get_os(سرخی);
	spl_image->name = image_get_name(سرخی);
	debug(PHASE_PROMPT
	      "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->بھرو_addr, spl_image->سائز);

	spl_parse_legacy_validate(spl_image->بھرو_addr, spl_image->سائز);
	spl_parse_legacy_validate(spl_image->اندراج_نقطہ, 0);

	واپس 0;
}

صحیح spl_بھرو_legacy_lzma(ساخت spl_image_info *spl_image,
			 ساخت spl_بھرو_info *بھرو, ulong انحراف)
{
	سائز_T lzma_len = LZMA_LEN;
	خالی *src;
	ulong dataptr, overhead, سائز;
	صحیح ret;

	/* dataptr points to compressed payload  */
	dataptr = ALIGN_نیچے_کو(حجم(ساخت legacy_img_hdr),
			     spl_get_bl_len(بھرو));
	overhead = حجم(ساخت legacy_img_hdr) - dataptr;
	سائز = ALIGN(spl_image->سائز + overhead, spl_get_bl_len(بھرو));
	dataptr += انحراف;

	debug("LZMA: Decompressing %08lx to %08lx\n",
	      dataptr, spl_image->بھرو_addr);
	src = حافظہ_دو(سائز);
	اگر (!src) {
		لکھو("Unable to allocate %d bytes for LZMA\n",
		       spl_image->سائز);
		واپس -ENOMEM;
	}

	بھرو->اقرأ(بھرو, dataptr, سائز, src);
	ret = lzma_Buff_مقصد_Buff_Decompress(map_sysmem(spl_image->بھرو_addr,
						  spl_image->سائز), &lzma_len,
				       src + overhead, spl_image->سائز);
	اگر (ret) {
		لکھو("LZMA decompression error: %d\n", ret);
		واپس ret;
	}

	spl_image->سائز = lzma_len;
	واپس 0;
}
