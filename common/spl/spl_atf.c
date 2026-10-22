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
ಸಂರಚನಾ bl2_to_bl31_params_mem {
	ಸಂರಚನಾ bl31_params bl31_params;
	ಸಂರಚನಾ atf_image_info bl31_image_info;
	ಸಂರಚನಾ atf_image_info bl32_image_info;
	ಸಂರಚನಾ atf_image_info bl33_image_info;
	ಸಂರಚನಾ entry_point_info bl33_ep_info;
	ಸಂರಚನಾ entry_point_info bl32_ep_info;
	ಸಂರಚನಾ entry_point_info bl31_ep_info;
};

ಸಂರಚನಾ bl2_to_bl31_params_mem_v2 {
	ಸಂರಚನಾ bl_params bl_params;
	ಸಂರಚನಾ bl_params_ಗಂಟು bl31_params_ಗಂಟು;
	ಸಂರಚನಾ bl_params_ಗಂಟು bl32_params_ಗಂಟು;
	ಸಂರಚನಾ bl_params_ಗಂಟು bl33_params_ಗಂಟು;
	ಸಂರಚನಾ atf_image_info bl31_image_info;
	ಸಂರಚನಾ atf_image_info bl32_image_info;
	ಸಂರಚನಾ atf_image_info bl33_image_info;
	ಸಂರಚನಾ entry_point_info bl33_ep_info;
	ಸಂರಚನಾ entry_point_info bl32_ep_info;
	ಸಂರಚನಾ entry_point_info bl31_ep_info;
};

ಸಂರಚನಾ bl31_params *bl2_plat_get_bl31_params_default(ulong bl32_entry,
						     ulong bl33_entry,
						     ulong fdt_addr)
{
	ಸ್ಥಿರ ಸಂರಚನಾ bl2_to_bl31_params_mem bl31_params_mem;
	ಸಂರಚನಾ bl31_params *bl2_to_bl31_params;
	ಸಂರಚನಾ entry_point_info *bl32_ep_info;
	ಸಂರಚನಾ entry_point_info *bl33_ep_info;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, ಪರಿಮಾಣ(ಸಂರಚನಾ bl2_to_bl31_params_mem));

	/* Assign memory for TF related information */
	bl2_to_bl31_params = &bl31_params_mem.bl31_params;
	SET_PARAM_HEAD(bl2_to_bl31_params, ATF_PARAM_BL31, ATF_VERSION_1, 0);

	/* Fill BL31 related information */
	bl2_to_bl31_params->bl31_image_info = &bl31_params_mem.bl31_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl31_image_info,
		       ATF_PARAM_IMAGE_ದ್ವಿಮಾನ, ATF_VERSION_1, 0);

	/* Fill BL32 related information */
	bl2_to_bl31_params->bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	SET_PARAM_HEAD(bl32_ep_info, ATF_PARAM_EP, ATF_VERSION_1,
		       ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl32_ep_info->args.arg3 = fdt_addr;
	bl32_ep_info->pc = bl32_entry ? bl32_entry : 0;
	bl32_ep_info->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
				     DISABLE_ALL_EXCEPTIONS);

	bl2_to_bl31_params->bl32_image_info = &bl31_params_mem.bl32_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl32_image_info,
		       ATF_PARAM_IMAGE_ದ್ವಿಮಾನ, ATF_VERSION_1, 0);

	/* Fill BL33 related information */
	bl2_to_bl31_params->bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	SET_PARAM_HEAD(bl33_ep_info, ATF_PARAM_EP, ATF_VERSION_1,
		       ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl33_ep_info->args.arg0 = 0xffff & read_mpidr();
	bl33_ep_info->pc = bl33_entry;
	bl33_ep_info->spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
				     DISABLE_ALL_EXCEPTIONS);

	bl2_to_bl31_params->bl33_image_info = &bl31_params_mem.bl33_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl33_image_info,
		       ATF_PARAM_IMAGE_ದ್ವಿಮಾನ, ATF_VERSION_1, 0);

	ಪ್ರತ್ಯಾಗಮ bl2_to_bl31_params;
}

__weak ಸಂರಚನಾ bl31_params *bl2_plat_get_bl31_params(ulong bl32_entry,
						    ulong bl33_entry,
						    ulong fdt_addr)
{
	ಪ್ರತ್ಯಾಗಮ bl2_plat_get_bl31_params_default(bl32_entry, bl33_entry,
						fdt_addr);
}

ಸಂರಚನಾ bl_params *bl2_plat_get_bl31_params_v2_default(ulong bl32_entry,
						      ulong bl33_entry,
						      ulong fdt_addr)
{
	ಸ್ಥಿರ ಸಂರಚನಾ bl2_to_bl31_params_mem_v2 bl31_params_mem;
	ಸಂರಚನಾ bl_params *bl_params;
	ಸಂರಚನಾ bl_params_ಗಂಟು *bl_params_ಗಂಟು;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, ಪರಿಮಾಣ(bl31_params_mem));

	/* Assign memory for TF related information */
	bl_params = &bl31_params_mem.bl_params;
	SET_PARAM_HEAD(bl_params, ATF_PARAM_BL_PARAMS, ATF_VERSION_2, 0);
	bl_params->head = &bl31_params_mem.bl31_params_ಗಂಟು;

	/* Fill BL31 related information */
	bl_params_ಗಂಟು = &bl31_params_mem.bl31_params_ಗಂಟು;
	bl_params_ಗಂಟು->image_id = ATF_BL31_IMAGE_ID;
	bl_params_ಗಂಟು->image_info = &bl31_params_mem.bl31_image_info;
	bl_params_ಗಂಟು->ep_info = &bl31_params_mem.bl31_ep_info;
	bl_params_ಗಂಟು->next_params_info = &bl31_params_mem.bl32_params_ಗಂಟು;
	SET_PARAM_HEAD(bl_params_ಗಂಟು->image_info, ATF_PARAM_IMAGE_ದ್ವಿಮಾನ,
		       ATF_VERSION_2, 0);

	/* Fill BL32 related information */
	bl_params_ಗಂಟು = &bl31_params_mem.bl32_params_ಗಂಟು;
	bl_params_ಗಂಟು->image_id = ATF_BL32_IMAGE_ID;
	bl_params_ಗಂಟು->image_info = &bl31_params_mem.bl32_image_info;
	bl_params_ಗಂಟು->ep_info = &bl31_params_mem.bl32_ep_info;
	bl_params_ಗಂಟು->next_params_info = &bl31_params_mem.bl33_params_ಗಂಟು;
	SET_PARAM_HEAD(bl_params_ಗಂಟು->ep_info, ATF_PARAM_EP,
		       ATF_VERSION_2, ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl_params_ಗಂಟು->ep_info->args.arg3 = fdt_addr;
	bl_params_ಗಂಟು->ep_info->pc = bl32_entry ? bl32_entry : 0;
	bl_params_ಗಂಟು->ep_info->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	SET_PARAM_HEAD(bl_params_ಗಂಟು->image_info, ATF_PARAM_IMAGE_ದ್ವಿಮಾನ,
		       ATF_VERSION_2, 0);

	/* Fill BL33 related information */
	bl_params_ಗಂಟು = &bl31_params_mem.bl33_params_ಗಂಟು;
	bl_params_ಗಂಟು->image_id = ATF_BL33_IMAGE_ID;
	bl_params_ಗಂಟು->image_info = &bl31_params_mem.bl33_image_info;
	bl_params_ಗಂಟು->ep_info = &bl31_params_mem.bl33_ep_info;
	bl_params_ಗಂಟು->next_params_info = ರಿಕ್ತ;
	SET_PARAM_HEAD(bl_params_ಗಂಟು->ep_info, ATF_PARAM_EP,
		       ATF_VERSION_2, ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl_params_ಗಂಟು->ep_info->args.arg0 = 0xffff & read_mpidr();
	bl_params_ಗಂಟು->ep_info->pc = bl33_entry;
	bl_params_ಗಂಟು->ep_info->spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	SET_PARAM_HEAD(bl_params_ಗಂಟು->image_info, ATF_PARAM_IMAGE_ದ್ವಿಮಾನ,
		       ATF_VERSION_2, 0);

	ಪ್ರತ್ಯಾಗಮ bl_params;
}

__weak ಸಂರಚನಾ bl_params *bl2_plat_get_bl31_params_v2(ulong bl32_entry,
						     ulong bl33_entry,
						     ulong fdt_addr)
{
	ಪ್ರತ್ಯಾಗಮ bl2_plat_get_bl31_params_v2_default(bl32_entry, bl33_entry,
						   fdt_addr);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ raw_write_daif(unsigned ಪೂರ್ಣಾಙ್ಕ daif)
{
	__asm__ __volatile__("msr DAIF, %x0\n\t" : : "r" (daif) : "memory");
}

ಪ್ರಕಾರನಿರ್ದೇಶ ಶೂನ್ಯ __noreturn (*atf_entry_t)(ಸಂರಚನಾ bl31_params *params, ಶೂನ್ಯ *plat_params);

ಸ್ಥಿರ ಶೂನ್ಯ __noreturn bl31_entry(ulong bl31_entry, ulong bl32_entry,
				  ulong bl33_entry, ulong fdt_addr)
{
	atf_entry_t  atf_entry = (atf_entry_t)bl31_entry;
	ಶೂನ್ಯ *bl31_params;

	ಯದಿ (CONFIG_IS_ENABLED(ATF_LOAD_IMAGE_V2))
		bl31_params = bl2_plat_get_bl31_params_v2(bl32_entry,
							  bl33_entry,
							  fdt_addr);
	ಅನ್ಯಥಾ
		bl31_params = bl2_plat_get_bl31_params(bl32_entry, bl33_entry,
						       fdt_addr);

	raw_write_daif(SPSR_EXCEPTION_MASK);
	ಯದಿ (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_disable();

	atf_entry(bl31_params, (ಶೂನ್ಯ *)fdt_addr);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_fit_images_find(ಶೂನ್ಯ *blob, ಪೂರ್ಣಾಙ್ಕ os)
{
	ಪೂರ್ಣಾಙ್ಕ parent, ಗಂಟು, ndepth = 0;
	ಪೂರ್ಣಾಙ್ಕ found = -FDT_ERR_NOTFOUND;
	ಧ್ರುವ ಶೂನ್ಯ *data;
	ulong val;

	ಯದಿ (!blob)
		ಪ್ರತ್ಯಾಗಮ -FDT_ERR_BADMAGIC;

	parent = fdt_path_offset(blob, "/fit-images");
	ಯದಿ (parent < 0)
		ಪ್ರತ್ಯಾಗಮ -FDT_ERR_NOTFOUND;

	ಪ್ರತಿ (ಗಂಟು = fdt_next_ಗಂಟು(blob, parent, &ndepth);
	     (ಗಂಟು >= 0) && (ndepth > 0);
	     ಗಂಟು = fdt_next_ಗಂಟು(blob, ಗಂಟು, &ndepth)) {
		ಯದಿ (ndepth != 1)
			ಅನುವರ್ತ;

		data = fdt_getprop(blob, ಗಂಟು, FIT_OS_PROP, ರಿಕ್ತ);
		ಯದಿ (!data)
			ಅನುವರ್ತ;

		ಯದಿ (genimg_get_os_id(data) != os)
			ಅನುವರ್ತ;

		/*
		 * A multi-segment image, e.g. an OP-TEE ELF split by
		 * binman, is recorded as one node per segment, all with
		 * the same os. Only the segment holding the ELF entry
		 * point carries an entry property, so prefer that node.
		 */
		ಯದಿ (!fit_image_get_entry(blob, ಗಂಟು, &val))
			ಪ್ರತ್ಯಾಗಮ ಗಂಟು;

		ಯದಿ (found < 0)
			found = ಗಂಟು;
	};

	ಪ್ರತ್ಯಾಗಮ found;
}

ulong spl_fit_images_get_entry(ಶೂನ್ಯ *blob, ಪೂರ್ಣಾಙ್ಕ ಗಂಟು)
{
	ulong  val;
	ಪೂರ್ಣಾಙ್ಕ ret;

	ret = fit_image_get_entry(blob, ಗಂಟು, &val);
	ಯದಿ (ret)
		ret = fit_image_get_load(blob, ಗಂಟು, &val);

	debug("%s: entry point 0x%lx\n", __func__, val);
	ಪ್ರತ್ಯಾಗಮ val;
}

ಶೂನ್ಯ __noreturn spl_invoke_atf(ಸಂರಚನಾ spl_image_info *spl_image)
{
	ulong  bl32_entry = 0;
	ulong  bl33_entry = CONFIG_TEXT_BASE;
	ಶೂನ್ಯ *blob = spl_image->fdt_addr;
	ulong platform_param = (ulong)blob;
	ಪೂರ್ಣಾಙ್ಕ ಗಂಟು;

	/*
	 * Find the OP-TEE binary (in /fit-images) load address or
	 * entry point (if different) and pass it as the BL3-2 entry
	 * point, this is optional.
	 */
	ಗಂಟು = spl_fit_images_find(blob, IH_OS_TEE);
	ಯದಿ (ಗಂಟು >= 0)
		bl32_entry = spl_fit_images_get_entry(blob, ಗಂಟು);

	/*
	 * Find the U-Boot binary (in /fit-images) load addreess or
	 * entry point (if different) and pass it as the BL3-3 entry
	 * point.
	 * This will need to be extended to support Falcon mode.
	 */

	ಗಂಟು = spl_fit_images_find(blob, IH_OS_U_BOOT);
	ಯದಿ (ಗಂಟು >= 0)
		bl33_entry = spl_fit_images_get_entry(blob, ಗಂಟು);

	/*
	 * If ATF_NO_PLATFORM_PARAM is set, we override the platform
	 * parameter and always pass 0.  This is a workaround for
	 * older ATF versions that have insufficiently robust (or
	 * overzealous) argument validation.
	 */
	ಯದಿ (CONFIG_IS_ENABLED(ATF_NO_PLATFORM_PARAM))
		platform_param = 0;

	/*
	 * We don't provide a BL3-2 entry yet, but this will be possible
	 * using similar logic.
	 */
	bl31_entry(spl_image->entry_point, bl32_entry,
		   bl33_entry, platform_param);
}
