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
কাঠামো bl2_to_bl31_params_mem {
	কাঠামো bl31_params bl31_params;
	কাঠামো atf_image_info bl31_image_info;
	কাঠামো atf_image_info bl32_image_info;
	কাঠামো atf_image_info bl33_image_info;
	কাঠামো entry_point_info bl33_ep_info;
	কাঠামো entry_point_info bl32_ep_info;
	কাঠামো entry_point_info bl31_ep_info;
};

কাঠামো bl2_to_bl31_params_mem_v2 {
	কাঠামো bl_params bl_params;
	কাঠামো bl_params_গ্রন্থি bl31_params_গ্রন্থি;
	কাঠামো bl_params_গ্রন্থি bl32_params_গ্রন্থি;
	কাঠামো bl_params_গ্রন্থি bl33_params_গ্রন্থি;
	কাঠামো atf_image_info bl31_image_info;
	কাঠামো atf_image_info bl32_image_info;
	কাঠামো atf_image_info bl33_image_info;
	কাঠামো entry_point_info bl33_ep_info;
	কাঠামো entry_point_info bl32_ep_info;
	কাঠামো entry_point_info bl31_ep_info;
};

কাঠামো bl31_params *bl2_plat_get_bl31_params_default(ulong bl32_entry,
						     ulong bl33_entry,
						     ulong fdt_addr)
{
	স্থির কাঠামো bl2_to_bl31_params_mem bl31_params_mem;
	কাঠামো bl31_params *bl2_to_bl31_params;
	কাঠামো entry_point_info *bl32_ep_info;
	কাঠামো entry_point_info *bl33_ep_info;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, আকার(কাঠামো bl2_to_bl31_params_mem));

	/* Assign memory for TF related information */
	bl2_to_bl31_params = &bl31_params_mem.bl31_params;
	SET_PARAM_শীর্ষ(bl2_to_bl31_params, ATF_PARAM_BL31, ATF_VERSION_1, 0);

	/* Fill BL31 related information */
	bl2_to_bl31_params->bl31_image_info = &bl31_params_mem.bl31_image_info;
	SET_PARAM_শীর্ষ(bl2_to_bl31_params->bl31_image_info,
		       ATF_PARAM_IMAGE_দ্বিমিক, ATF_VERSION_1, 0);

	/* Fill BL32 related information */
	bl2_to_bl31_params->bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	SET_PARAM_শীর্ষ(bl32_ep_info, ATF_PARAM_EP, ATF_VERSION_1,
		       ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl32_ep_info->args.arg3 = fdt_addr;
	bl32_ep_info->pc = bl32_entry ? bl32_entry : 0;
	bl32_ep_info->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
				     DISABLE_ALL_EXCEPTIONS);

	bl2_to_bl31_params->bl32_image_info = &bl31_params_mem.bl32_image_info;
	SET_PARAM_শীর্ষ(bl2_to_bl31_params->bl32_image_info,
		       ATF_PARAM_IMAGE_দ্বিমিক, ATF_VERSION_1, 0);

	/* Fill BL33 related information */
	bl2_to_bl31_params->bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	SET_PARAM_শীর্ষ(bl33_ep_info, ATF_PARAM_EP, ATF_VERSION_1,
		       ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl33_ep_info->args.arg0 = 0xffff & read_mpidr();
	bl33_ep_info->pc = bl33_entry;
	bl33_ep_info->spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
				     DISABLE_ALL_EXCEPTIONS);

	bl2_to_bl31_params->bl33_image_info = &bl31_params_mem.bl33_image_info;
	SET_PARAM_শীর্ষ(bl2_to_bl31_params->bl33_image_info,
		       ATF_PARAM_IMAGE_দ্বিমিক, ATF_VERSION_1, 0);

	ফেরত bl2_to_bl31_params;
}

__weak কাঠামো bl31_params *bl2_plat_get_bl31_params(ulong bl32_entry,
						    ulong bl33_entry,
						    ulong fdt_addr)
{
	ফেরত bl2_plat_get_bl31_params_default(bl32_entry, bl33_entry,
						fdt_addr);
}

কাঠামো bl_params *bl2_plat_get_bl31_params_v2_default(ulong bl32_entry,
						      ulong bl33_entry,
						      ulong fdt_addr)
{
	স্থির কাঠামো bl2_to_bl31_params_mem_v2 bl31_params_mem;
	কাঠামো bl_params *bl_params;
	কাঠামো bl_params_গ্রন্থি *bl_params_গ্রন্থি;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, আকার(bl31_params_mem));

	/* Assign memory for TF related information */
	bl_params = &bl31_params_mem.bl_params;
	SET_PARAM_শীর্ষ(bl_params, ATF_PARAM_BL_PARAMS, ATF_VERSION_2, 0);
	bl_params->শীর্ষ_2 = &bl31_params_mem.bl31_params_গ্রন্থি;

	/* Fill BL31 related information */
	bl_params_গ্রন্থি = &bl31_params_mem.bl31_params_গ্রন্থি;
	bl_params_গ্রন্থি->image_id = ATF_BL31_IMAGE_ID;
	bl_params_গ্রন্থি->image_info = &bl31_params_mem.bl31_image_info;
	bl_params_গ্রন্থি->ep_info = &bl31_params_mem.bl31_ep_info;
	bl_params_গ্রন্থি->পরবর্তী_params_info = &bl31_params_mem.bl32_params_গ্রন্থি;
	SET_PARAM_শীর্ষ(bl_params_গ্রন্থি->image_info, ATF_PARAM_IMAGE_দ্বিমিক,
		       ATF_VERSION_2, 0);

	/* Fill BL32 related information */
	bl_params_গ্রন্থি = &bl31_params_mem.bl32_params_গ্রন্থি;
	bl_params_গ্রন্থি->image_id = ATF_BL32_IMAGE_ID;
	bl_params_গ্রন্থি->image_info = &bl31_params_mem.bl32_image_info;
	bl_params_গ্রন্থি->ep_info = &bl31_params_mem.bl32_ep_info;
	bl_params_গ্রন্থি->পরবর্তী_params_info = &bl31_params_mem.bl33_params_গ্রন্থি;
	SET_PARAM_শীর্ষ(bl_params_গ্রন্থি->ep_info, ATF_PARAM_EP,
		       ATF_VERSION_2, ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl_params_গ্রন্থি->ep_info->args.arg3 = fdt_addr;
	bl_params_গ্রন্থি->ep_info->pc = bl32_entry ? bl32_entry : 0;
	bl_params_গ্রন্থি->ep_info->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	SET_PARAM_শীর্ষ(bl_params_গ্রন্থি->image_info, ATF_PARAM_IMAGE_দ্বিমিক,
		       ATF_VERSION_2, 0);

	/* Fill BL33 related information */
	bl_params_গ্রন্থি = &bl31_params_mem.bl33_params_গ্রন্থি;
	bl_params_গ্রন্থি->image_id = ATF_BL33_IMAGE_ID;
	bl_params_গ্রন্থি->image_info = &bl31_params_mem.bl33_image_info;
	bl_params_গ্রন্থি->ep_info = &bl31_params_mem.bl33_ep_info;
	bl_params_গ্রন্থি->পরবর্তী_params_info = ফাঁকা;
	SET_PARAM_শীর্ষ(bl_params_গ্রন্থি->ep_info, ATF_PARAM_EP,
		       ATF_VERSION_2, ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl_params_গ্রন্থি->ep_info->args.arg0 = 0xffff & read_mpidr();
	bl_params_গ্রন্থি->ep_info->pc = bl33_entry;
	bl_params_গ্রন্থি->ep_info->spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	SET_PARAM_শীর্ষ(bl_params_গ্রন্থি->image_info, ATF_PARAM_IMAGE_দ্বিমিক,
		       ATF_VERSION_2, 0);

	ফেরত bl_params;
}

__weak কাঠামো bl_params *bl2_plat_get_bl31_params_v2(ulong bl32_entry,
						     ulong bl33_entry,
						     ulong fdt_addr)
{
	ফেরত bl2_plat_get_bl31_params_v2_default(bl32_entry, bl33_entry,
						   fdt_addr);
}

স্থির অন্তঃরেখ শূন্য raw_write_daif(unsigned পূর্ণসংখ্যা daif)
{
	__asm__ __volatile__("msr DAIF, %x0\n\t" : : "r" (daif) : "memory");
}

প্রকারলেখ শূন্য __noreturn (*atf_entry_t)(কাঠামো bl31_params *params, শূন্য *plat_params);

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

স্থির পূর্ণসংখ্যা spl_fit_images_find(শূন্য *blob, পূর্ণসংখ্যা os)
{
	পূর্ণসংখ্যা parent, গ্রন্থি, ndepth = 0;
	পূর্ণসংখ্যা found = -FDT_ERR_NOTFOUND;
	ধ্রুবক শূন্য *তথ্য;
	ulong val;

	যদি (!blob)
		ফেরত -FDT_ERR_BADMAGIC;

	parent = fdt_path_offset(blob, "/fit-images");
	যদি (parent < 0)
		ফেরত -FDT_ERR_NOTFOUND;

	জন্য (গ্রন্থি = fdt_পরবর্তী_গ্রন্থি(blob, parent, &ndepth);
	     (গ্রন্থি >= 0) && (ndepth > 0);
	     গ্রন্থি = fdt_পরবর্তী_গ্রন্থি(blob, গ্রন্থি, &ndepth)) {
		যদি (ndepth != 1)
			চালু;

		তথ্য = fdt_getprop(blob, গ্রন্থি, FIT_OS_PROP, ফাঁকা);
		যদি (!তথ্য)
			চালু;

		যদি (genimg_get_os_id(তথ্য) != os)
			চালু;

		/*
		 * A multi-segment image, e.g. an OP-TEE ELF split by
		 * binman, is recorded as one node per segment, all with
		 * the same os. Only the segment holding the ELF entry
		 * point carries an entry property, so prefer that node.
		 */
		যদি (!fit_image_get_entry(blob, গ্রন্থি, &val))
			ফেরত গ্রন্থি;

		যদি (found < 0)
			found = গ্রন্থি;
	};

	ফেরত found;
}

ulong spl_fit_images_get_entry(শূন্য *blob, পূর্ণসংখ্যা গ্রন্থি)
{
	ulong  val;
	পূর্ণসংখ্যা ret;

	ret = fit_image_get_entry(blob, গ্রন্থি, &val);
	যদি (ret)
		ret = fit_image_get_load(blob, গ্রন্থি, &val);

	debug("%s: entry point 0x%lx\n", __func__, val);
	ফেরত val;
}

শূন্য __noreturn spl_invoke_atf(কাঠামো spl_image_info *spl_image)
{
	ulong  bl32_entry = 0;
	ulong  bl33_entry = CONFIG_TEXT_BASE;
	শূন্য *blob = spl_image->fdt_addr;
	ulong platform_param = (ulong)blob;
	পূর্ণসংখ্যা গ্রন্থি;

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
