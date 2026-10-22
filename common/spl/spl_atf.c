// SPDX-License-Identifier: BSD-3-Clause
/*
 * Reference to the ARM TF Project,
 * plat/arm/common/arm_bl2_setup.c
 * Portions copyright (c) 2013-2016, ARM Limited and Contributors. All rights
 * reserved.
 * Copyright (C) 2016 Rockchip Electronic Co.,Ltd
 * Written by Kever Yang <kever.yang@rock-chips.com>
 * Copyright (C) 2017 Theobroma Systems Design und Consulting GmbH
 */

#అన్తర్భావ <atf_common.h>
#అన్తర్భావ <cpu_func.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <spl.h>
#అన్తర్భావ <asm/cache.h>

/* Holds all the structures we need for bl31 parameter passing */
నిర్మాణం bl2_గమ్యం_bl31_params_mem {
	నిర్మాణం bl31_params bl31_params;
	నిర్మాణం atf_image_info bl31_image_info;
	నిర్మాణం atf_image_info bl32_image_info;
	నిర్మాణం atf_image_info bl33_image_info;
	నిర్మాణం నమోదు_బిందువు_info bl33_ep_info;
	నిర్మాణం నమోదు_బిందువు_info bl32_ep_info;
	నిర్మాణం నమోదు_బిందువు_info bl31_ep_info;
};

నిర్మాణం bl2_గమ్యం_bl31_params_mem_v2 {
	నిర్మాణం bl_params bl_params;
	నిర్మాణం bl_params_కణుపు bl31_params_కణుపు;
	నిర్మాణం bl_params_కణుపు bl32_params_కణుపు;
	నిర్మాణం bl_params_కణుపు bl33_params_కణుపు;
	నిర్మాణం atf_image_info bl31_image_info;
	నిర్మాణం atf_image_info bl32_image_info;
	నిర్మాణం atf_image_info bl33_image_info;
	నిర్మాణం నమోదు_బిందువు_info bl33_ep_info;
	నిర్మాణం నమోదు_బిందువు_info bl32_ep_info;
	నిర్మాణం నమోదు_బిందువు_info bl31_ep_info;
};

నిర్మాణం bl31_params *bl2_plat_get_bl31_params_default(ulong bl32_నమోదు,
						     ulong bl33_నమోదు,
						     ulong fdt_addr)
{
	స్థిర నిర్మాణం bl2_గమ్యం_bl31_params_mem bl31_params_mem;
	నిర్మాణం bl31_params *bl2_గమ్యం_bl31_params;
	నిర్మాణం నమోదు_బిందువు_info *bl32_ep_info;
	నిర్మాణం నమోదు_బిందువు_info *bl33_ep_info;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, పరిమాణం(నిర్మాణం bl2_గమ్యం_bl31_params_mem));

	/* Assign memory for TF related information */
	bl2_గమ్యం_bl31_params = &bl31_params_mem.bl31_params;
	సమితి_PARAM_తల(bl2_గమ్యం_bl31_params, ATF_PARAM_BL31, ATF_సంస్కరణ_1, 0);

	/* Fill BL31 related information */
	bl2_గమ్యం_bl31_params->bl31_image_info = &bl31_params_mem.bl31_image_info;
	సమితి_PARAM_తల(bl2_గమ్యం_bl31_params->bl31_image_info,
		       ATF_PARAM_IMAGE_ద్విమాన, ATF_సంస్కరణ_1, 0);

	/* Fill BL32 related information */
	bl2_గమ్యం_bl31_params->bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	సమితి_PARAM_తల(bl32_ep_info, ATF_PARAM_EP, ATF_సంస్కరణ_1,
		       ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl32_ep_info->వాదనలు.arg3 = fdt_addr;
	bl32_ep_info->pc = bl32_నమోదు ? bl32_నమోదు : 0;
	bl32_ep_info->spsr = SPSR_64(బహుళకం_EL1, బహుళకం_SP_ELX,
				     DISABLE_ALL_EXCEPTIONS);

	bl2_గమ్యం_bl31_params->bl32_image_info = &bl31_params_mem.bl32_image_info;
	సమితి_PARAM_తల(bl2_గమ్యం_bl31_params->bl32_image_info,
		       ATF_PARAM_IMAGE_ద్విమాన, ATF_సంస్కరణ_1, 0);

	/* Fill BL33 related information */
	bl2_గమ్యం_bl31_params->bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	సమితి_PARAM_తల(bl33_ep_info, ATF_PARAM_EP, ATF_సంస్కరణ_1,
		       ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl33_ep_info->వాదనలు.arg0 = 0xffff & read_mpidr();
	bl33_ep_info->pc = bl33_నమోదు;
	bl33_ep_info->spsr = SPSR_64(బహుళకం_EL2, బహుళకం_SP_ELX,
				     DISABLE_ALL_EXCEPTIONS);

	bl2_గమ్యం_bl31_params->bl33_image_info = &bl31_params_mem.bl33_image_info;
	సమితి_PARAM_తల(bl2_గమ్యం_bl31_params->bl33_image_info,
		       ATF_PARAM_IMAGE_ద్విమాన, ATF_సంస్కరణ_1, 0);

	తిరిగి bl2_గమ్యం_bl31_params;
}

__weak నిర్మాణం bl31_params *bl2_plat_get_bl31_params(ulong bl32_నమోదు,
						    ulong bl33_నమోదు,
						    ulong fdt_addr)
{
	తిరిగి bl2_plat_get_bl31_params_default(bl32_నమోదు, bl33_నమోదు,
						fdt_addr);
}

నిర్మాణం bl_params *bl2_plat_get_bl31_params_v2_default(ulong bl32_నమోదు,
						      ulong bl33_నమోదు,
						      ulong fdt_addr)
{
	స్థిర నిర్మాణం bl2_గమ్యం_bl31_params_mem_v2 bl31_params_mem;
	నిర్మాణం bl_params *bl_params;
	నిర్మాణం bl_params_కణుపు *bl_params_కణుపు;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, పరిమాణం(bl31_params_mem));

	/* Assign memory for TF related information */
	bl_params = &bl31_params_mem.bl_params;
	సమితి_PARAM_తల(bl_params, ATF_PARAM_BL_PARAMS, ATF_సంస్కరణ_2, 0);
	bl_params->తల = &bl31_params_mem.bl31_params_కణుపు;

	/* Fill BL31 related information */
	bl_params_కణుపు = &bl31_params_mem.bl31_params_కణుపు;
	bl_params_కణుపు->image_id = ATF_BL31_IMAGE_ID;
	bl_params_కణుపు->image_info = &bl31_params_mem.bl31_image_info;
	bl_params_కణుపు->ep_info = &bl31_params_mem.bl31_ep_info;
	bl_params_కణుపు->తదుపరి_params_info = &bl31_params_mem.bl32_params_కణుపు;
	సమితి_PARAM_తల(bl_params_కణుపు->image_info, ATF_PARAM_IMAGE_ద్విమాన,
		       ATF_సంస్కరణ_2, 0);

	/* Fill BL32 related information */
	bl_params_కణుపు = &bl31_params_mem.bl32_params_కణుపు;
	bl_params_కణుపు->image_id = ATF_BL32_IMAGE_ID;
	bl_params_కణుపు->image_info = &bl31_params_mem.bl32_image_info;
	bl_params_కణుపు->ep_info = &bl31_params_mem.bl32_ep_info;
	bl_params_కణుపు->తదుపరి_params_info = &bl31_params_mem.bl33_params_కణుపు;
	సమితి_PARAM_తల(bl_params_కణుపు->ep_info, ATF_PARAM_EP,
		       ATF_సంస్కరణ_2, ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl_params_కణుపు->ep_info->వాదనలు.arg3 = fdt_addr;
	bl_params_కణుపు->ep_info->pc = bl32_నమోదు ? bl32_నమోదు : 0;
	bl_params_కణుపు->ep_info->spsr = SPSR_64(బహుళకం_EL1, బహుళకం_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	సమితి_PARAM_తల(bl_params_కణుపు->image_info, ATF_PARAM_IMAGE_ద్విమాన,
		       ATF_సంస్కరణ_2, 0);

	/* Fill BL33 related information */
	bl_params_కణుపు = &bl31_params_mem.bl33_params_కణుపు;
	bl_params_కణుపు->image_id = ATF_BL33_IMAGE_ID;
	bl_params_కణుపు->image_info = &bl31_params_mem.bl33_image_info;
	bl_params_కణుపు->ep_info = &bl31_params_mem.bl33_ep_info;
	bl_params_కణుపు->తదుపరి_params_info = ఖాళీ;
	సమితి_PARAM_తల(bl_params_కణుపు->ep_info, ATF_PARAM_EP,
		       ATF_సంస్కరణ_2, ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl_params_కణుపు->ep_info->వాదనలు.arg0 = 0xffff & read_mpidr();
	bl_params_కణుపు->ep_info->pc = bl33_నమోదు;
	bl_params_కణుపు->ep_info->spsr = SPSR_64(బహుళకం_EL2, బహుళకం_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	సమితి_PARAM_తల(bl_params_కణుపు->image_info, ATF_PARAM_IMAGE_ద్విమాన,
		       ATF_సంస్కరణ_2, 0);

	తిరిగి bl_params;
}

__weak నిర్మాణం bl_params *bl2_plat_get_bl31_params_v2(ulong bl32_నమోదు,
						     ulong bl33_నమోదు,
						     ulong fdt_addr)
{
	తిరిగి bl2_plat_get_bl31_params_v2_default(bl32_నమోదు, bl33_నమోదు,
						   fdt_addr);
}

స్థిర అంతఃవరుస శూన్య raw_write_daif(గుర్తులేని పూర్ణాంకం daif)
{
	__asm__ __volatile__("msr DAIF, %x0\n\t" : : "r" (daif) : "memory");
}

రకనిర్వచనం శూన్య __noreturn (*atf_నమోదు_t)(నిర్మాణం bl31_params *params, శూన్య *plat_params);

స్థిర శూన్య __noreturn bl31_నమోదు(ulong bl31_నమోదు, ulong bl32_నమోదు,
				  ulong bl33_నమోదు, ulong fdt_addr)
{
	atf_నమోదు_t  atf_నమోదు = (atf_నమోదు_t)bl31_నమోదు;
	శూన్య *bl31_params;

	ఒకవేళ (CONFIG_IS_ENABLED(ATF_ఎక్కించు_IMAGE_V2))
		bl31_params = bl2_plat_get_bl31_params_v2(bl32_నమోదు,
							  bl33_నమోదు,
							  fdt_addr);
	లేకపోతే
		bl31_params = bl2_plat_get_bl31_params(bl32_నమోదు, bl33_నమోదు,
						       fdt_addr);

	raw_write_daif(SPSR_EXCEPTION_ముసుగు);
	ఒకవేళ (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_disable();

	atf_నమోదు(bl31_params, (శూన్య *)fdt_addr);
}

స్థిర పూర్ణాంకం spl_fit_images_కనుగొను(శూన్య *blob, పూర్ణాంకం os)
{
	పూర్ణాంకం మాతృ, కణుపు, ndepth = 0;
	పూర్ణాంకం దొరికింది = -FDT_ERR_NOTFOUND;
	స్థిరరాశి శూన్య *దత్తాంశం;
	ulong val;

	ఒకవేళ (!blob)
		తిరిగి -FDT_ERR_BADMAGIC;

	మాతృ = fdt_మార్గం_స్థానభ్రంశం(blob, "/fit-images");
	ఒకవేళ (మాతృ < 0)
		తిరిగి -FDT_ERR_NOTFOUND;

	కొరకు (కణుపు = fdt_తదుపరి_కణుపు(blob, మాతృ, &ndepth);
	     (కణుపు >= 0) && (ndepth > 0);
	     కణుపు = fdt_తదుపరి_కణుపు(blob, కణుపు, &ndepth)) {
		ఒకవేళ (ndepth != 1)
			కొనసాగు;

		దత్తాంశం = fdt_getprop(blob, కణుపు, FIT_OS_PROP, ఖాళీ);
		ఒకవేళ (!దత్తాంశం)
			కొనసాగు;

		ఒకవేళ (genimg_get_os_id(దత్తాంశం) != os)
			కొనసాగు;

		/*
		 * A multi-segment image, e.g. an OP-TEE ELF split by
		 * binman, is recorded as one node per segment, all with
		 * the same os. Only the segment holding the ELF entry
		 * point carries an entry property, so prefer that node.
		 */
		ఒకవేళ (!fit_image_get_నమోదు(blob, కణుపు, &val))
			తిరిగి కణుపు;

		ఒకవేళ (దొరికింది < 0)
			దొరికింది = కణుపు;
	};

	తిరిగి దొరికింది;
}

ulong spl_fit_images_get_నమోదు(శూన్య *blob, పూర్ణాంకం కణుపు)
{
	ulong  val;
	పూర్ణాంకం ret;

	ret = fit_image_get_నమోదు(blob, కణుపు, &val);
	ఒకవేళ (ret)
		ret = fit_image_get_ఎక్కించు(blob, కణుపు, &val);

	debug("%s: entry point 0x%lx\n", __func__, val);
	తిరిగి val;
}

శూన్య __noreturn spl_invoke_atf(నిర్మాణం spl_image_info *spl_image)
{
	ulong  bl32_నమోదు = 0;
	ulong  bl33_నమోదు = CONFIG_పాఠ్యం_పునాది;
	శూన్య *blob = spl_image->fdt_addr;
	ulong platform_param = (ulong)blob;
	పూర్ణాంకం కణుపు;

	/*
	 * Find the OP-TEE binary (in /fit-images) load address or
	 * entry point (if different) and pass it as the BL3-2 entry
	 * point, this is optional.
	 */
	కణుపు = spl_fit_images_కనుగొను(blob, IH_OS_TEE);
	ఒకవేళ (కణుపు >= 0)
		bl32_నమోదు = spl_fit_images_get_నమోదు(blob, కణుపు);

	/*
	 * Find the U-Boot binary (in /fit-images) load addreess or
	 * entry point (if different) and pass it as the BL3-3 entry
	 * point.
	 * This will need to be extended to support Falcon mode.
	 */

	కణుపు = spl_fit_images_కనుగొను(blob, IH_OS_U_BOOT);
	ఒకవేళ (కణుపు >= 0)
		bl33_నమోదు = spl_fit_images_get_నమోదు(blob, కణుపు);

	/*
	 * If ATF_NO_PLATFORM_PARAM is set, we override the platform
	 * parameter and always pass 0.  This is a workaround for
	 * older ATF versions that have insufficiently robust (or
	 * overzealous) argument validation.
	 */
	ఒకవేళ (CONFIG_IS_ENABLED(ATF_NO_PLATFORM_PARAM))
		platform_param = 0;

	/*
	 * We don't provide a BL3-2 entry yet, but this will be possible
	 * using similar logic.
	 */
	bl31_నమోదు(spl_image->నమోదు_బిందువు, bl32_నమోదు,
		   bl33_నమోదు, platform_param);
}
