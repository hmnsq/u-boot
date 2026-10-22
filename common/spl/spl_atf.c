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

#অন্তর্ভাব <atf_common.h>
#অন্তর্ভাব <cpu_func.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <image.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <spl.h>
#অন্তর্ভাব <asm/cache.h>

/* Holds all the structures we need for bl31 parameter passing */
সংরচনা bl2_to_bl31_params_mem {
	সংরচনা bl31_params bl31_params;
	সংরচনা atf_image_info bl31_image_info;
	সংরচনা atf_image_info bl32_image_info;
	সংরচনা atf_image_info bl33_image_info;
	সংরচনা entry_point_info bl33_ep_info;
	সংরচনা entry_point_info bl32_ep_info;
	সংরচনা entry_point_info bl31_ep_info;
};

সংরচনা bl2_to_bl31_params_mem_v2 {
	সংরচনা bl_params bl_params;
	সংরচনা bl_params_গ্রন্থি bl31_params_গ্রন্থি;
	সংরচনা bl_params_গ্রন্থি bl32_params_গ্রন্থি;
	সংরচনা bl_params_গ্রন্থি bl33_params_গ্রন্থি;
	সংরচনা atf_image_info bl31_image_info;
	সংরচনা atf_image_info bl32_image_info;
	সংরচনা atf_image_info bl33_image_info;
	সংরচনা entry_point_info bl33_ep_info;
	সংরচনা entry_point_info bl32_ep_info;
	সংরচনা entry_point_info bl31_ep_info;
};

সংরচনা bl31_params *bl2_plat_get_bl31_params_default(ulong bl32_entry,
						     ulong bl33_entry,
						     ulong fdt_addr)
{
	স্থির সংরচনা bl2_to_bl31_params_mem bl31_params_mem;
	সংরচনা bl31_params *bl2_to_bl31_params;
	সংরচনা entry_point_info *bl32_ep_info;
	সংরচনা entry_point_info *bl33_ep_info;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, পরিমাণ(সংরচনা bl2_to_bl31_params_mem));

	/* Assign memory for TF related information */
	bl2_to_bl31_params = &bl31_params_mem.bl31_params;
	SET_PARAM_HEAD(bl2_to_bl31_params, ATF_PARAM_BL31, ATF_VERSION_1, 0);

	/* Fill BL31 related information */
	bl2_to_bl31_params->bl31_image_info = &bl31_params_mem.bl31_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl31_image_info,
		       ATF_PARAM_IMAGE_দ্বিমিক, ATF_VERSION_1, 0);

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
		       ATF_PARAM_IMAGE_দ্বিমিক, ATF_VERSION_1, 0);

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
		       ATF_PARAM_IMAGE_দ্বিমিক, ATF_VERSION_1, 0);

	প্রত্যাগম bl2_to_bl31_params;
}

__weak সংরচনা bl31_params *bl2_plat_get_bl31_params(ulong bl32_entry,
						    ulong bl33_entry,
						    ulong fdt_addr)
{
	প্রত্যাগম bl2_plat_get_bl31_params_default(bl32_entry, bl33_entry,
						fdt_addr);
}

সংরচনা bl_params *bl2_plat_get_bl31_params_v2_default(ulong bl32_entry,
						      ulong bl33_entry,
						      ulong fdt_addr)
{
	স্থির সংরচনা bl2_to_bl31_params_mem_v2 bl31_params_mem;
	সংরচনা bl_params *bl_params;
	সংরচনা bl_params_গ্রন্থি *bl_params_গ্রন্থি;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, পরিমাণ(bl31_params_mem));

	/* Assign memory for TF related information */
	bl_params = &bl31_params_mem.bl_params;
	SET_PARAM_HEAD(bl_params, ATF_PARAM_BL_PARAMS, ATF_VERSION_2, 0);
	bl_params->head = &bl31_params_mem.bl31_params_গ্রন্থি;

	/* Fill BL31 related information */
	bl_params_গ্রন্থি = &bl31_params_mem.bl31_params_গ্রন্থি;
	bl_params_গ্রন্থি->image_id = ATF_BL31_IMAGE_ID;
	bl_params_গ্রন্থি->image_info = &bl31_params_mem.bl31_image_info;
	bl_params_গ্রন্থি->ep_info = &bl31_params_mem.bl31_ep_info;
	bl_params_গ্রন্থি->next_params_info = &bl31_params_mem.bl32_params_গ্রন্থি;
	SET_PARAM_HEAD(bl_params_গ্রন্থি->image_info, ATF_PARAM_IMAGE_দ্বিমিক,
		       ATF_VERSION_2, 0);

	/* Fill BL32 related information */
	bl_params_গ্রন্থি = &bl31_params_mem.bl32_params_গ্রন্থি;
	bl_params_গ্রন্থি->image_id = ATF_BL32_IMAGE_ID;
	bl_params_গ্রন্থি->image_info = &bl31_params_mem.bl32_image_info;
	bl_params_গ্রন্থি->ep_info = &bl31_params_mem.bl32_ep_info;
	bl_params_গ্রন্থি->next_params_info = &bl31_params_mem.bl33_params_গ্রন্থি;
	SET_PARAM_HEAD(bl_params_গ্রন্থি->ep_info, ATF_PARAM_EP,
		       ATF_VERSION_2, ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl_params_গ্রন্থি->ep_info->args.arg3 = fdt_addr;
	bl_params_গ্রন্থি->ep_info->pc = bl32_entry ? bl32_entry : 0;
	bl_params_গ্রন্থি->ep_info->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	SET_PARAM_HEAD(bl_params_গ্রন্থি->image_info, ATF_PARAM_IMAGE_দ্বিমিক,
		       ATF_VERSION_2, 0);

	/* Fill BL33 related information */
	bl_params_গ্রন্থি = &bl31_params_mem.bl33_params_গ্রন্থি;
	bl_params_গ্রন্থি->image_id = ATF_BL33_IMAGE_ID;
	bl_params_গ্রন্থি->image_info = &bl31_params_mem.bl33_image_info;
	bl_params_গ্রন্থি->ep_info = &bl31_params_mem.bl33_ep_info;
	bl_params_গ্রন্থি->next_params_info = রিক্ত;
	SET_PARAM_HEAD(bl_params_গ্রন্থি->ep_info, ATF_PARAM_EP,
		       ATF_VERSION_2, ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl_params_গ্রন্থি->ep_info->args.arg0 = 0xffff & read_mpidr();
	bl_params_গ্রন্থি->ep_info->pc = bl33_entry;
	bl_params_গ্রন্থি->ep_info->spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	SET_PARAM_HEAD(bl_params_গ্রন্থি->image_info, ATF_PARAM_IMAGE_দ্বিমিক,
		       ATF_VERSION_2, 0);

	প্রত্যাগম bl_params;
}

__weak সংরচনা bl_params *bl2_plat_get_bl31_params_v2(ulong bl32_entry,
						     ulong bl33_entry,
						     ulong fdt_addr)
{
	প্রত্যাগম bl2_plat_get_bl31_params_v2_default(bl32_entry, bl33_entry,
						   fdt_addr);
}

স্থির অন্তঃস্থ শূন্য raw_write_daif(unsigned পূর্ণাঙ্ক daif)
{
	__asm__ __volatile__("msr DAIF, %x0\n\t" : : "r" (daif) : "memory");
}

প্রকারনির্দেশ শূন্য __noreturn (*atf_entry_t)(সংরচনা bl31_params *params, শূন্য *plat_params);

স্থির শূন্য __noreturn bl31_entry(ulong bl31_entry, ulong bl32_entry,
				  ulong bl33_entry, ulong fdt_addr)
{
	atf_entry_t  atf_entry = (atf_entry_t)bl31_entry;
	শূন্য *bl31_params;

	যদি (CONFIG_IS_ENABLED(ATF_LOAD_IMAGE_V2))
		bl31_params = bl2_plat_get_bl31_params_v2(bl32_entry,
							  bl33_entry,
							  fdt_addr);
	অন্যথা
		bl31_params = bl2_plat_get_bl31_params(bl32_entry, bl33_entry,
						       fdt_addr);

	raw_write_daif(SPSR_EXCEPTION_MASK);
	যদি (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_disable();

	atf_entry(bl31_params, (শূন্য *)fdt_addr);
}

স্থির পূর্ণাঙ্ক spl_fit_images_find(শূন্য *blob, পূর্ণাঙ্ক os)
{
	পূর্ণাঙ্ক parent, গ্রন্থি, ndepth = 0;
	পূর্ণাঙ্ক found = -FDT_ERR_NOTFOUND;
	ধ্রুব শূন্য *data;
	ulong val;

	যদি (!blob)
		প্রত্যাগম -FDT_ERR_BADMAGIC;

	parent = fdt_path_offset(blob, "/fit-images");
	যদি (parent < 0)
		প্রত্যাগম -FDT_ERR_NOTFOUND;

	প্রতি (গ্রন্থি = fdt_next_গ্রন্থি(blob, parent, &ndepth);
	     (গ্রন্থি >= 0) && (ndepth > 0);
	     গ্রন্থি = fdt_next_গ্রন্থি(blob, গ্রন্থি, &ndepth)) {
		যদি (ndepth != 1)
			অনুবর্ত;

		data = fdt_getprop(blob, গ্রন্থি, FIT_OS_PROP, রিক্ত);
		যদি (!data)
			অনুবর্ত;

		যদি (genimg_get_os_id(data) != os)
			অনুবর্ত;

		/*
		 * A multi-segment image, e.g. an OP-TEE ELF split by
		 * binman, is recorded as one node per segment, all with
		 * the same os. Only the segment holding the ELF entry
		 * point carries an entry property, so prefer that node.
		 */
		যদি (!fit_image_get_entry(blob, গ্রন্থি, &val))
			প্রত্যাগম গ্রন্থি;

		যদি (found < 0)
			found = গ্রন্থি;
	};

	প্রত্যাগম found;
}

ulong spl_fit_images_get_entry(শূন্য *blob, পূর্ণাঙ্ক গ্রন্থি)
{
	ulong  val;
	পূর্ণাঙ্ক ret;

	ret = fit_image_get_entry(blob, গ্রন্থি, &val);
	যদি (ret)
		ret = fit_image_get_load(blob, গ্রন্থি, &val);

	debug("%s: entry point 0x%lx\n", __func__, val);
	প্রত্যাগম val;
}

শূন্য __noreturn spl_invoke_atf(সংরচনা spl_image_info *spl_image)
{
	ulong  bl32_entry = 0;
	ulong  bl33_entry = CONFIG_TEXT_BASE;
	শূন্য *blob = spl_image->fdt_addr;
	ulong platform_param = (ulong)blob;
	পূর্ণাঙ্ক গ্রন্থি;

	/*
	 * Find the OP-TEE binary (in /fit-images) load address or
	 * entry point (if different) and pass it as the BL3-2 entry
	 * point, this is optional.
	 */
	গ্রন্থি = spl_fit_images_find(blob, IH_OS_TEE);
	যদি (গ্রন্থি >= 0)
		bl32_entry = spl_fit_images_get_entry(blob, গ্রন্থি);

	/*
	 * Find the U-Boot binary (in /fit-images) load addreess or
	 * entry point (if different) and pass it as the BL3-3 entry
	 * point.
	 * This will need to be extended to support Falcon mode.
	 */

	গ্রন্থি = spl_fit_images_find(blob, IH_OS_U_BOOT);
	যদি (গ্রন্থি >= 0)
		bl33_entry = spl_fit_images_get_entry(blob, গ্রন্থি);

	/*
	 * If ATF_NO_PLATFORM_PARAM is set, we override the platform
	 * parameter and always pass 0.  This is a workaround for
	 * older ATF versions that have insufficiently robust (or
	 * overzealous) argument validation.
	 */
	যদি (CONFIG_IS_ENABLED(ATF_NO_PLATFORM_PARAM))
		platform_param = 0;

	/*
	 * We don't provide a BL3-2 entry yet, but this will be possible
	 * using similar logic.
	 */
	bl31_entry(spl_image->entry_point, bl32_entry,
		   bl33_entry, platform_param);
}
