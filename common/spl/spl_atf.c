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

#ಅನ್ತರ್ಭಾವ <atf_common.h>
#ಅನ್ತರ್ಭಾವ <cpu_func.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <asm/cache.h>

/* Holds all the structures we need for bl31 parameter passing */
ರಚನೆ bl2_ಗಮ್ಯ_bl31_params_mem {
	ರಚನೆ bl31_params bl31_params;
	ರಚನೆ atf_image_info bl31_image_info;
	ರಚನೆ atf_image_info bl32_image_info;
	ರಚನೆ atf_image_info bl33_image_info;
	ರಚನೆ ನಮೂದು_ಬಿಂದು_info bl33_ep_info;
	ರಚನೆ ನಮೂದು_ಬಿಂದು_info bl32_ep_info;
	ರಚನೆ ನಮೂದು_ಬಿಂದು_info bl31_ep_info;
};

ರಚನೆ bl2_ಗಮ್ಯ_bl31_params_mem_v2 {
	ರಚನೆ bl_params bl_params;
	ರಚನೆ bl_params_ಗಂಟು bl31_params_ಗಂಟು;
	ರಚನೆ bl_params_ಗಂಟು bl32_params_ಗಂಟು;
	ರಚನೆ bl_params_ಗಂಟು bl33_params_ಗಂಟು;
	ರಚನೆ atf_image_info bl31_image_info;
	ರಚನೆ atf_image_info bl32_image_info;
	ರಚನೆ atf_image_info bl33_image_info;
	ರಚನೆ ನಮೂದು_ಬಿಂದು_info bl33_ep_info;
	ರಚನೆ ನಮೂದು_ಬಿಂದು_info bl32_ep_info;
	ರಚನೆ ನಮೂದು_ಬಿಂದು_info bl31_ep_info;
};

ರಚನೆ bl31_params *bl2_plat_get_bl31_params_default(ulong bl32_ನಮೂದು,
						     ulong bl33_ನಮೂದು,
						     ulong fdt_addr)
{
	ಸ್ಥಿರ ರಚನೆ bl2_ಗಮ್ಯ_bl31_params_mem bl31_params_mem;
	ರಚನೆ bl31_params *bl2_ಗಮ್ಯ_bl31_params;
	ರಚನೆ ನಮೂದು_ಬಿಂದು_info *bl32_ep_info;
	ರಚನೆ ನಮೂದು_ಬಿಂದು_info *bl33_ep_info;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, ಗಾತ್ರ(ರಚನೆ bl2_ಗಮ್ಯ_bl31_params_mem));

	/* Assign memory for TF related information */
	bl2_ಗಮ್ಯ_bl31_params = &bl31_params_mem.bl31_params;
	ಗಣ_PARAM_ತಲೆ(bl2_ಗಮ್ಯ_bl31_params, ATF_PARAM_BL31, ATF_ಆವೃತ್ತಿ_1, 0);

	/* Fill BL31 related information */
	bl2_ಗಮ್ಯ_bl31_params->bl31_image_info = &bl31_params_mem.bl31_image_info;
	ಗಣ_PARAM_ತಲೆ(bl2_ಗಮ್ಯ_bl31_params->bl31_image_info,
		       ATF_PARAM_IMAGE_ದ್ವಿಮಾನ, ATF_ಆವೃತ್ತಿ_1, 0);

	/* Fill BL32 related information */
	bl2_ಗಮ್ಯ_bl31_params->bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	ಗಣ_PARAM_ತಲೆ(bl32_ep_info, ATF_PARAM_EP, ATF_ಆವೃತ್ತಿ_1,
		       ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl32_ep_info->ವಾದಗಳು.arg3 = fdt_addr;
	bl32_ep_info->pc = bl32_ನಮೂದು ? bl32_ನಮೂದು : 0;
	bl32_ep_info->spsr = SPSR_64(ಬಹುಲಕ_EL1, ಬಹುಲಕ_SP_ELX,
				     DISABLE_ALL_EXCEPTIONS);

	bl2_ಗಮ್ಯ_bl31_params->bl32_image_info = &bl31_params_mem.bl32_image_info;
	ಗಣ_PARAM_ತಲೆ(bl2_ಗಮ್ಯ_bl31_params->bl32_image_info,
		       ATF_PARAM_IMAGE_ದ್ವಿಮಾನ, ATF_ಆವೃತ್ತಿ_1, 0);

	/* Fill BL33 related information */
	bl2_ಗಮ್ಯ_bl31_params->bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	ಗಣ_PARAM_ತಲೆ(bl33_ep_info, ATF_PARAM_EP, ATF_ಆವೃತ್ತಿ_1,
		       ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl33_ep_info->ವಾದಗಳು.arg0 = 0xffff & read_mpidr();
	bl33_ep_info->pc = bl33_ನಮೂದು;
	bl33_ep_info->spsr = SPSR_64(ಬಹುಲಕ_EL2, ಬಹುಲಕ_SP_ELX,
				     DISABLE_ALL_EXCEPTIONS);

	bl2_ಗಮ್ಯ_bl31_params->bl33_image_info = &bl31_params_mem.bl33_image_info;
	ಗಣ_PARAM_ತಲೆ(bl2_ಗಮ್ಯ_bl31_params->bl33_image_info,
		       ATF_PARAM_IMAGE_ದ್ವಿಮಾನ, ATF_ಆವೃತ್ತಿ_1, 0);

	ಹಿಂತಿರುಗಿ bl2_ಗಮ್ಯ_bl31_params;
}

__weak ರಚನೆ bl31_params *bl2_plat_get_bl31_params(ulong bl32_ನಮೂದು,
						    ulong bl33_ನಮೂದು,
						    ulong fdt_addr)
{
	ಹಿಂತಿರುಗಿ bl2_plat_get_bl31_params_default(bl32_ನಮೂದು, bl33_ನಮೂದು,
						fdt_addr);
}

ರಚನೆ bl_params *bl2_plat_get_bl31_params_v2_default(ulong bl32_ನಮೂದು,
						      ulong bl33_ನಮೂದು,
						      ulong fdt_addr)
{
	ಸ್ಥಿರ ರಚನೆ bl2_ಗಮ್ಯ_bl31_params_mem_v2 bl31_params_mem;
	ರಚನೆ bl_params *bl_params;
	ರಚನೆ bl_params_ಗಂಟು *bl_params_ಗಂಟು;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, ಗಾತ್ರ(bl31_params_mem));

	/* Assign memory for TF related information */
	bl_params = &bl31_params_mem.bl_params;
	ಗಣ_PARAM_ತಲೆ(bl_params, ATF_PARAM_BL_PARAMS, ATF_ಆವೃತ್ತಿ_2, 0);
	bl_params->ತಲೆ = &bl31_params_mem.bl31_params_ಗಂಟು;

	/* Fill BL31 related information */
	bl_params_ಗಂಟು = &bl31_params_mem.bl31_params_ಗಂಟು;
	bl_params_ಗಂಟು->image_id = ATF_BL31_IMAGE_ID;
	bl_params_ಗಂಟು->image_info = &bl31_params_mem.bl31_image_info;
	bl_params_ಗಂಟು->ep_info = &bl31_params_mem.bl31_ep_info;
	bl_params_ಗಂಟು->ಮುಂದಿನ_params_info = &bl31_params_mem.bl32_params_ಗಂಟು;
	ಗಣ_PARAM_ತಲೆ(bl_params_ಗಂಟು->image_info, ATF_PARAM_IMAGE_ದ್ವಿಮಾನ,
		       ATF_ಆವೃತ್ತಿ_2, 0);

	/* Fill BL32 related information */
	bl_params_ಗಂಟು = &bl31_params_mem.bl32_params_ಗಂಟು;
	bl_params_ಗಂಟು->image_id = ATF_BL32_IMAGE_ID;
	bl_params_ಗಂಟು->image_info = &bl31_params_mem.bl32_image_info;
	bl_params_ಗಂಟು->ep_info = &bl31_params_mem.bl32_ep_info;
	bl_params_ಗಂಟು->ಮುಂದಿನ_params_info = &bl31_params_mem.bl33_params_ಗಂಟು;
	ಗಣ_PARAM_ತಲೆ(bl_params_ಗಂಟು->ep_info, ATF_PARAM_EP,
		       ATF_ಆವೃತ್ತಿ_2, ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl_params_ಗಂಟು->ep_info->ವಾದಗಳು.arg3 = fdt_addr;
	bl_params_ಗಂಟು->ep_info->pc = bl32_ನಮೂದು ? bl32_ನಮೂದು : 0;
	bl_params_ಗಂಟು->ep_info->spsr = SPSR_64(ಬಹುಲಕ_EL1, ಬಹುಲಕ_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	ಗಣ_PARAM_ತಲೆ(bl_params_ಗಂಟು->image_info, ATF_PARAM_IMAGE_ದ್ವಿಮಾನ,
		       ATF_ಆವೃತ್ತಿ_2, 0);

	/* Fill BL33 related information */
	bl_params_ಗಂಟು = &bl31_params_mem.bl33_params_ಗಂಟು;
	bl_params_ಗಂಟು->image_id = ATF_BL33_IMAGE_ID;
	bl_params_ಗಂಟು->image_info = &bl31_params_mem.bl33_image_info;
	bl_params_ಗಂಟು->ep_info = &bl31_params_mem.bl33_ep_info;
	bl_params_ಗಂಟು->ಮುಂದಿನ_params_info = ಖಾಲಿ;
	ಗಣ_PARAM_ತಲೆ(bl_params_ಗಂಟು->ep_info, ATF_PARAM_EP,
		       ATF_ಆವೃತ್ತಿ_2, ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl_params_ಗಂಟು->ep_info->ವಾದಗಳು.arg0 = 0xffff & read_mpidr();
	bl_params_ಗಂಟು->ep_info->pc = bl33_ನಮೂದು;
	bl_params_ಗಂಟು->ep_info->spsr = SPSR_64(ಬಹುಲಕ_EL2, ಬಹುಲಕ_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	ಗಣ_PARAM_ತಲೆ(bl_params_ಗಂಟು->image_info, ATF_PARAM_IMAGE_ದ್ವಿಮಾನ,
		       ATF_ಆವೃತ್ತಿ_2, 0);

	ಹಿಂತಿರುಗಿ bl_params;
}

__weak ರಚನೆ bl_params *bl2_plat_get_bl31_params_v2(ulong bl32_ನಮೂದು,
						     ulong bl33_ನಮೂದು,
						     ulong fdt_addr)
{
	ಹಿಂತಿರುಗಿ bl2_plat_get_bl31_params_v2_default(bl32_ನಮೂದು, bl33_ನಮೂದು,
						   fdt_addr);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ raw_write_daif(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ daif)
{
	__asm__ __volatile__("msr DAIF, %x0\n\t" : : "r" (daif) : "memory");
}

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ __noreturn (*atf_ನಮೂದು_t)(ರಚನೆ bl31_params *params, ಶೂನ್ಯ *plat_params);

ಸ್ಥಿರ ಶೂನ್ಯ __noreturn bl31_ನಮೂದು(ulong bl31_ನಮೂದು, ulong bl32_ನಮೂದು,
				  ulong bl33_ನಮೂದು, ulong fdt_addr)
{
	atf_ನಮೂದು_t  atf_ನಮೂದು = (atf_ನಮೂದು_t)bl31_ನಮೂದು;
	ಶೂನ್ಯ *bl31_params;

	ಆದರೆ (CONFIG_IS_ENABLED(ATF_ತುಂಬು_IMAGE_V2))
		bl31_params = bl2_plat_get_bl31_params_v2(bl32_ನಮೂದು,
							  bl33_ನಮೂದು,
							  fdt_addr);
	ಇಲ್ಲದಿದ್ದರೆ
		bl31_params = bl2_plat_get_bl31_params(bl32_ನಮೂದು, bl33_ನಮೂದು,
						       fdt_addr);

	raw_write_daif(SPSR_EXCEPTION_ಮುಖವಾಡ);
	ಆದರೆ (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_disable();

	atf_ನಮೂದು(bl31_params, (ಶೂನ್ಯ *)fdt_addr);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_fit_images_ಕಂಡುಹಿಡಿ(ಶೂನ್ಯ *blob, ಪೂರ್ಣಾಂಕ os)
{
	ಪೂರ್ಣಾಂಕ ಪೋಷಕ, ಗಂಟು, ndepth = 0;
	ಪೂರ್ಣಾಂಕ ಸಿಕ್ಕಿತು = -FDT_ERR_NOTFOUND;
	ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ದತ್ತಾಂಶ;
	ulong val;

	ಆದರೆ (!blob)
		ಹಿಂತಿರುಗಿ -FDT_ERR_BADMAGIC;

	ಪೋಷಕ = fdt_ಮಾರ್ಗ_ಸ್ಥಾನಾಂತರ(blob, "/fit-images");
	ಆದರೆ (ಪೋಷಕ < 0)
		ಹಿಂತಿರುಗಿ -FDT_ERR_NOTFOUND;

	ಗಾಗಿ (ಗಂಟು = fdt_ಮುಂದಿನ_ಗಂಟು(blob, ಪೋಷಕ, &ndepth);
	     (ಗಂಟು >= 0) && (ndepth > 0);
	     ಗಂಟು = fdt_ಮುಂದಿನ_ಗಂಟು(blob, ಗಂಟು, &ndepth)) {
		ಆದರೆ (ndepth != 1)
			ಮುಂದುವರಿ;

		ದತ್ತಾಂಶ = fdt_getprop(blob, ಗಂಟು, FIT_OS_PROP, ಖಾಲಿ);
		ಆದರೆ (!ದತ್ತಾಂಶ)
			ಮುಂದುವರಿ;

		ಆದರೆ (genimg_get_os_id(ದತ್ತಾಂಶ) != os)
			ಮುಂದುವರಿ;

		/*
		 * A multi-segment image, e.g. an OP-TEE ELF split by
		 * binman, is recorded as one node per segment, all with
		 * the same os. Only the segment holding the ELF entry
		 * point carries an entry property, so prefer that node.
		 */
		ಆದರೆ (!fit_image_get_ನಮೂದು(blob, ಗಂಟು, &val))
			ಹಿಂತಿರುಗಿ ಗಂಟು;

		ಆದರೆ (ಸಿಕ್ಕಿತು < 0)
			ಸಿಕ್ಕಿತು = ಗಂಟು;
	};

	ಹಿಂತಿರುಗಿ ಸಿಕ್ಕಿತು;
}

ulong spl_fit_images_get_ನಮೂದು(ಶೂನ್ಯ *blob, ಪೂರ್ಣಾಂಕ ಗಂಟು)
{
	ulong  val;
	ಪೂರ್ಣಾಂಕ ret;

	ret = fit_image_get_ನಮೂದು(blob, ಗಂಟು, &val);
	ಆದರೆ (ret)
		ret = fit_image_get_ತುಂಬು(blob, ಗಂಟು, &val);

	debug("%s: entry point 0x%lx\n", __func__, val);
	ಹಿಂತಿರುಗಿ val;
}

ಶೂನ್ಯ __noreturn spl_invoke_atf(ರಚನೆ spl_image_info *spl_image)
{
	ulong  bl32_ನಮೂದು = 0;
	ulong  bl33_ನಮೂದು = CONFIG_ಪಠ್ಯ_ತಳ;
	ಶೂನ್ಯ *blob = spl_image->fdt_addr;
	ulong platform_param = (ulong)blob;
	ಪೂರ್ಣಾಂಕ ಗಂಟು;

	/*
	 * Find the OP-TEE binary (in /fit-images) load address or
	 * entry point (if different) and pass it as the BL3-2 entry
	 * point, this is optional.
	 */
	ಗಂಟು = spl_fit_images_ಕಂಡುಹಿಡಿ(blob, IH_OS_TEE);
	ಆದರೆ (ಗಂಟು >= 0)
		bl32_ನಮೂದು = spl_fit_images_get_ನಮೂದು(blob, ಗಂಟು);

	/*
	 * Find the U-Boot binary (in /fit-images) load addreess or
	 * entry point (if different) and pass it as the BL3-3 entry
	 * point.
	 * This will need to be extended to support Falcon mode.
	 */

	ಗಂಟು = spl_fit_images_ಕಂಡುಹಿಡಿ(blob, IH_OS_U_BOOT);
	ಆದರೆ (ಗಂಟು >= 0)
		bl33_ನಮೂದು = spl_fit_images_get_ನಮೂದು(blob, ಗಂಟು);

	/*
	 * If ATF_NO_PLATFORM_PARAM is set, we override the platform
	 * parameter and always pass 0.  This is a workaround for
	 * older ATF versions that have insufficiently robust (or
	 * overzealous) argument validation.
	 */
	ಆದರೆ (CONFIG_IS_ENABLED(ATF_NO_PLATFORM_PARAM))
		platform_param = 0;

	/*
	 * We don't provide a BL3-2 entry yet, but this will be possible
	 * using similar logic.
	 */
	bl31_ನಮೂದು(spl_image->ನಮೂದು_ಬಿಂದು, bl32_ನಮೂದು,
		   bl33_ನಮೂದು, platform_param);
}
