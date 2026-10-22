// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Stefan Roese <sr@denx.de>
 */

#అన్తర్భావ <image.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <malloc.h>
#అన్తర్భావ <mapmem.h>
#అన్తర్భావ <asm/sections.h>
#అన్తర్భావ <spl.h>

#అన్తర్భావ <lzma/LzmaTypes.h>
#అన్తర్భావ <lzma/LzmaDec.h>
#అన్తర్భావ <lzma/LzmaTools.h>

#నిర్వచించు LZMA_LEN	(1 << 20)

స్థిర శూన్య spl_parse_legacy_validate(ulong ప్రారంభం, ulong కొలత)
{
	ulong spl_ప్రారంభం = (ulong)ప్రారంభం_2;
	ulong spl_చివర = (ulong)&image_ద్విమాన_చివర;
	ulong చివర_2 = ప్రారంభం + కొలత;

	ఒకవేళ ((ప్రారంభం >= spl_ప్రారంభం && ప్రారంభం < spl_చివర) ||
	    (చివర_2 > spl_ప్రారంభం && చివర_2 <= spl_చివర) ||
	    (ప్రారంభం < spl_ప్రారంభం && చివర_2 >= spl_చివర) ||
	    (ప్రారంభం > చివర_2 && చివర_2 > spl_ప్రారంభం))
		panic("SPL: Image overlaps SPL\n");

	ఒకవేళ (కొలత > CONFIG_SYS_BOOTM_LEN)
		panic("SPL: Image too large\n");
}

పూర్ణాంకం spl_parse_legacy_శీర్షిక(నిర్మాణం spl_image_info *spl_image,
			    స్థిరరాశి నిర్మాణం legacy_img_hdr *శీర్షిక)
{
	u32 శీర్షిక_కొలత = పరిమాణం(నిర్మాణం legacy_img_hdr);

	/* check uImage header CRC */
	ఒకవేళ (IS_ENABLED(CONFIG_SPL_LEGACY_IMAGE_చక్రీయ_తనిఖీ_CHECK) &&
	    !image_check_hcrc(శీర్షిక)) {
		puts("SPL: Image header CRC check failed!\n");
		తిరిగి -EINVAL;
	}

	ఒకవేళ (spl_image->flags & SPL_COPY_PAYLOAD_ONLY) {
		/*
		 * On some system (e.g. powerpc), the load-address and
		 * entry-point is located at address 0. We can't load
		 * to 0-0x40. So skip header in this case.
		 */
		spl_image->ఎక్కించు_addr = image_get_ఎక్కించు(శీర్షిక);
		spl_image->నమోదు_బిందువు = image_get_ep(శీర్షిక);
		spl_image->కొలత = image_get_దత్తాంశం_కొలత(శీర్షిక);
	} లేకపోతే {
		spl_image->నమోదు_బిందువు = image_get_ep(శీర్షిక);
		/* Load including the header */
		spl_image->ఎక్కించు_addr = image_get_ఎక్కించు(శీర్షిక) -
			శీర్షిక_కొలత;
		spl_image->కొలత = image_get_దత్తాంశం_కొలత(శీర్షిక) +
			శీర్షిక_కొలత;
	}

#నిర్వచితమైతే CONFIG_SPL_LEGACY_IMAGE_చక్రీయ_తనిఖీ_CHECK
	/* store uImage data length and CRC to check later */
	spl_image->dcrc_దత్తాంశం = image_get_ఎక్కించు(శీర్షిక);
	spl_image->dcrc_నిడివి = image_get_దత్తాంశం_కొలత(శీర్షిక);
	spl_image->dcrc = image_get_dcrc(శీర్షిక);
#అయితే_ముగింపు

	spl_image->os = image_get_os(శీర్షిక);
	spl_image->name = image_get_name(శీర్షిక);
	debug(PHASE_PROMPT
	      "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->ఎక్కించు_addr, spl_image->కొలత);

	spl_parse_legacy_validate(spl_image->ఎక్కించు_addr, spl_image->కొలత);
	spl_parse_legacy_validate(spl_image->నమోదు_బిందువు, 0);

	తిరిగి 0;
}

పూర్ణాంకం spl_ఎక్కించు_legacy_lzma(నిర్మాణం spl_image_info *spl_image,
			 నిర్మాణం spl_ఎక్కించు_info *ఎక్కించు, ulong స్థానభ్రంశం)
{
	కొలత_T lzma_len = LZMA_LEN;
	శూన్య *src;
	ulong dataptr, overhead, కొలత;
	పూర్ణాంకం ret;

	/* dataptr points to compressed payload  */
	dataptr = ALIGN_కిందికి(పరిమాణం(నిర్మాణం legacy_img_hdr),
			     spl_get_bl_len(ఎక్కించు));
	overhead = పరిమాణం(నిర్మాణం legacy_img_hdr) - dataptr;
	కొలత = ALIGN(spl_image->కొలత + overhead, spl_get_bl_len(ఎక్కించు));
	dataptr += స్థానభ్రంశం;

	debug("LZMA: Decompressing %08lx to %08lx\n",
	      dataptr, spl_image->ఎక్కించు_addr);
	src = జ్ఞాపకం_ఇవ్వు(కొలత);
	ఒకవేళ (!src) {
		రాయి("Unable to allocate %d bytes for LZMA\n",
		       spl_image->కొలత);
		తిరిగి -ENOMEM;
	}

	ఎక్కించు->పఠ(ఎక్కించు, dataptr, కొలత, src);
	ret = lzma_Buff_గమ్యం_Buff_Decompress(map_sysmem(spl_image->ఎక్కించు_addr,
						  spl_image->కొలత), &lzma_len,
				       src + overhead, spl_image->కొలత);
	ఒకవేళ (ret) {
		రాయి("LZMA decompression error: %d\n", ret);
		తిరిగి ret;
	}

	spl_image->కొలత = lzma_len;
	తిరిగి 0;
}
