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

#وصف_ڪريو LZMA_LEN	(1 << 20)

ساكن فراغ spl_parse_legacy_validate(ulong شروعات, ulong سائيز)
{
	ulong spl_شروعات = (ulong)شروعات_2;
	ulong spl_پڇاڙي = (ulong)&image_ٻٽو_پڇاڙي;
	ulong پڇاڙي_2 = شروعات + سائيز;

	إذا ((شروعات >= spl_شروعات && شروعات < spl_پڇاڙي) ||
	    (پڇاڙي_2 > spl_شروعات && پڇاڙي_2 <= spl_پڇاڙي) ||
	    (شروعات < spl_شروعات && پڇاڙي_2 >= spl_پڇاڙي) ||
	    (شروعات > پڇاڙي_2 && پڇاڙي_2 > spl_شروعات))
		panic("SPL: Image overlaps SPL\n");

	إذا (سائيز > CONFIG_SYS_BOOTM_LEN)
		panic("SPL: Image too large\n");
}

صحيح spl_parse_legacy_سرو(بنية spl_image_info *spl_image,
			    ثابت بنية legacy_img_hdr *سرو)
{
	u32 سرو_سائيز = حجم(بنية legacy_img_hdr);

	/* check uImage header CRC */
	إذا (IS_ENABLED(CONFIG_SPL_LEGACY_IMAGE_چڪري_جاچ_CHECK) &&
	    !image_check_hcrc(سرو)) {
		puts("SPL: Image header CRC check failed!\n");
		أرجع -EINVAL;
	}

	إذا (spl_image->flags & SPL_COPY_PAYLOAD_ONLY) {
		/*
		 * On some system (e.g. powerpc), the load-address and
		 * entry-point is located at address 0. We can't load
		 * to 0-0x40. So skip header in this case.
		 */
		spl_image->ڀريو_addr = image_get_ڀريو(سرو);
		spl_image->داخلا_نقطو = image_get_ep(سرو);
		spl_image->سائيز = image_get_معلومات_سائيز(سرو);
	} وإلا {
		spl_image->داخلا_نقطو = image_get_ep(سرو);
		/* Load including the header */
		spl_image->ڀريو_addr = image_get_ڀريو(سرو) -
			سرو_سائيز;
		spl_image->سائيز = image_get_معلومات_سائيز(سرو) +
			سرو_سائيز;
	}

#جيڪڏهن_وصف_ٿيل CONFIG_SPL_LEGACY_IMAGE_چڪري_جاچ_CHECK
	/* store uImage data length and CRC to check later */
	spl_image->dcrc_معلومات = image_get_ڀريو(سرو);
	spl_image->dcrc_ڊيگهائي = image_get_معلومات_سائيز(سرو);
	spl_image->dcrc = image_get_dcrc(سرو);
#جيڪڏهن_پورو

	spl_image->os = image_get_os(سرو);
	spl_image->name = image_get_name(سرو);
	debug(PHASE_PROMPT
	      "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->ڀريو_addr, spl_image->سائيز);

	spl_parse_legacy_validate(spl_image->ڀريو_addr, spl_image->سائيز);
	spl_parse_legacy_validate(spl_image->داخلا_نقطو, 0);

	أرجع 0;
}

صحيح spl_ڀريو_legacy_lzma(بنية spl_image_info *spl_image,
			 بنية spl_ڀريو_info *ڀريو, ulong وٿي)
{
	سائيز_T lzma_len = LZMA_LEN;
	فراغ *src;
	ulong dataptr, overhead, سائيز;
	صحيح ret;

	/* dataptr points to compressed payload  */
	dataptr = ALIGN_هيٺ_ڏانهن(حجم(بنية legacy_img_hdr),
			     spl_get_bl_len(ڀريو));
	overhead = حجم(بنية legacy_img_hdr) - dataptr;
	سائيز = ALIGN(spl_image->سائيز + overhead, spl_get_bl_len(ڀريو));
	dataptr += وٿي;

	debug("LZMA: Decompressing %08lx to %08lx\n",
	      dataptr, spl_image->ڀريو_addr);
	src = يادگيري_ڏيو(سائيز);
	إذا (!src) {
		لکو("Unable to allocate %d bytes for LZMA\n",
		       spl_image->سائيز);
		أرجع -ENOMEM;
	}

	ڀريو->اقرأ(ڀريو, dataptr, سائيز, src);
	ret = lzma_Buff_منزل_Buff_Decompress(map_sysmem(spl_image->ڀريو_addr,
						  spl_image->سائيز), &lzma_len,
				       src + overhead, spl_image->سائيز);
	إذا (ret) {
		لکو("LZMA decompression error: %d\n", ret);
		أرجع ret;
	}

	spl_image->سائيز = lzma_len;
	أرجع 0;
}
