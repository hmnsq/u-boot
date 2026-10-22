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

#تعریف_کړه LZMA_LEN	(1 << 20)

ساكن فراغ spl_parse_legacy_validate(ulong پیل, ulong اندازه)
{
	ulong spl_پیل = (ulong)پیل_2;
	ulong spl_پای_ټکی = (ulong)&image_دوګونی_پای_ټکی;
	ulong پای_ټکی_2 = پیل + اندازه;

	إذا ((پیل >= spl_پیل && پیل < spl_پای_ټکی) ||
	    (پای_ټکی_2 > spl_پیل && پای_ټکی_2 <= spl_پای_ټکی) ||
	    (پیل < spl_پیل && پای_ټکی_2 >= spl_پای_ټکی) ||
	    (پیل > پای_ټکی_2 && پای_ټکی_2 > spl_پیل))
		panic("SPL: Image overlaps SPL\n");

	إذا (اندازه > CONFIG_SYS_BOOTM_LEN)
		panic("SPL: Image too large\n");
}

صحيح spl_parse_legacy_سرلیک(بنية spl_image_info *spl_image,
			    ثابت بنية legacy_img_hdr *سرلیک)
{
	u32 سرلیک_اندازه = حجم(بنية legacy_img_hdr);

	/* check uImage header CRC */
	إذا (IS_ENABLED(CONFIG_SPL_LEGACY_IMAGE_دوراني_ازموینه_CHECK) &&
	    !image_check_hcrc(سرلیک)) {
		puts("SPL: Image header CRC check failed!\n");
		أرجع -EINVAL;
	}

	إذا (spl_image->flags & SPL_COPY_PAYLOAD_ONLY) {
		/*
		 * On some system (e.g. powerpc), the load-address and
		 * entry-point is located at address 0. We can't load
		 * to 0-0x40. So skip header in this case.
		 */
		spl_image->بارول_addr = image_get_بارول(سرلیک);
		spl_image->ثبت_ټکی = image_get_ep(سرلیک);
		spl_image->اندازه = image_get_معلومات_اندازه(سرلیک);
	} وإلا {
		spl_image->ثبت_ټکی = image_get_ep(سرلیک);
		/* Load including the header */
		spl_image->بارول_addr = image_get_بارول(سرلیک) -
			سرلیک_اندازه;
		spl_image->اندازه = image_get_معلومات_اندازه(سرلیک) +
			سرلیک_اندازه;
	}

#که_تعریف_شوی CONFIG_SPL_LEGACY_IMAGE_دوراني_ازموینه_CHECK
	/* store uImage data length and CRC to check later */
	spl_image->dcrc_معلومات = image_get_بارول(سرلیک);
	spl_image->dcrc_اوږدتیا = image_get_معلومات_اندازه(سرلیک);
	spl_image->dcrc = image_get_dcrc(سرلیک);
#د_که_پای

	spl_image->os = image_get_os(سرلیک);
	spl_image->name = image_get_name(سرلیک);
	debug(PHASE_PROMPT
	      "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->بارول_addr, spl_image->اندازه);

	spl_parse_legacy_validate(spl_image->بارول_addr, spl_image->اندازه);
	spl_parse_legacy_validate(spl_image->ثبت_ټکی, 0);

	أرجع 0;
}

صحيح spl_بارول_legacy_lzma(بنية spl_image_info *spl_image,
			 بنية spl_بارول_info *بارول, ulong دننه_واټن)
{
	اندازه_T lzma_len = LZMA_LEN;
	فراغ *src;
	ulong dataptr, overhead, اندازه;
	صحيح ret;

	/* dataptr points to compressed payload  */
	dataptr = ALIGN_ښکته(حجم(بنية legacy_img_hdr),
			     spl_get_bl_len(بارول));
	overhead = حجم(بنية legacy_img_hdr) - dataptr;
	اندازه = ALIGN(spl_image->اندازه + overhead, spl_get_bl_len(بارول));
	dataptr += دننه_واټن;

	debug("LZMA: Decompressing %08lx to %08lx\n",
	      dataptr, spl_image->بارول_addr);
	src = حافظه_ورکړه(اندازه);
	إذا (!src) {
		چاپ_کړه("Unable to allocate %d bytes for LZMA\n",
		       spl_image->اندازه);
		أرجع -ENOMEM;
	}

	بارول->اقرأ(بارول, dataptr, اندازه, src);
	ret = lzma_Buff_منزل_Buff_Decompress(map_sysmem(spl_image->بارول_addr,
						  spl_image->اندازه), &lzma_len,
				       src + overhead, spl_image->اندازه);
	إذا (ret) {
		چاپ_کړه("LZMA decompression error: %d\n", ret);
		أرجع ret;
	}

	spl_image->اندازه = lzma_len;
	أرجع 0;
}
