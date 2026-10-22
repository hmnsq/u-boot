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

#ଅନ୍ତର୍ଭାଵ <atf_common.h>
#ଅନ୍ତର୍ଭାଵ <cpu_func.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <image.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <asm/cache.h>

/* Holds all the structures we need for bl31 parameter passing */
ସଂରଚନା bl2_to_bl31_params_mem {
	ସଂରଚନା bl31_params bl31_params;
	ସଂରଚନା atf_image_info bl31_image_info;
	ସଂରଚନା atf_image_info bl32_image_info;
	ସଂରଚନା atf_image_info bl33_image_info;
	ସଂରଚନା entry_point_info bl33_ep_info;
	ସଂରଚନା entry_point_info bl32_ep_info;
	ସଂରଚନା entry_point_info bl31_ep_info;
};

ସଂରଚନା bl2_to_bl31_params_mem_v2 {
	ସଂରଚନା bl_params bl_params;
	ସଂରଚନା bl_params_ଗଣ୍ଠି bl31_params_ଗଣ୍ଠି;
	ସଂରଚନା bl_params_ଗଣ୍ଠି bl32_params_ଗଣ୍ଠି;
	ସଂରଚନା bl_params_ଗଣ୍ଠି bl33_params_ଗଣ୍ଠି;
	ସଂରଚନା atf_image_info bl31_image_info;
	ସଂରଚନା atf_image_info bl32_image_info;
	ସଂରଚନା atf_image_info bl33_image_info;
	ସଂରଚନା entry_point_info bl33_ep_info;
	ସଂରଚନା entry_point_info bl32_ep_info;
	ସଂରଚନା entry_point_info bl31_ep_info;
};

ସଂରଚନା bl31_params *bl2_plat_get_bl31_params_default(ulong bl32_entry,
						     ulong bl33_entry,
						     ulong fdt_addr)
{
	ସ୍ଥିର ସଂରଚନା bl2_to_bl31_params_mem bl31_params_mem;
	ସଂରଚନା bl31_params *bl2_to_bl31_params;
	ସଂରଚନା entry_point_info *bl32_ep_info;
	ସଂରଚନା entry_point_info *bl33_ep_info;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, ଆକାର(ସଂରଚନା bl2_to_bl31_params_mem));

	/* Assign memory for TF related information */
	bl2_to_bl31_params = &bl31_params_mem.bl31_params;
	SET_PARAM_HEAD(bl2_to_bl31_params, ATF_PARAM_BL31, ATF_VERSION_1, 0);

	/* Fill BL31 related information */
	bl2_to_bl31_params->bl31_image_info = &bl31_params_mem.bl31_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl31_image_info,
		       ATF_PARAM_IMAGE_ଦ୍ୱିମିକ, ATF_VERSION_1, 0);

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
		       ATF_PARAM_IMAGE_ଦ୍ୱିମିକ, ATF_VERSION_1, 0);

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
		       ATF_PARAM_IMAGE_ଦ୍ୱିମିକ, ATF_VERSION_1, 0);

	ଫେରନ୍ତୁ bl2_to_bl31_params;
}

__weak ସଂରଚନା bl31_params *bl2_plat_get_bl31_params(ulong bl32_entry,
						    ulong bl33_entry,
						    ulong fdt_addr)
{
	ଫେରନ୍ତୁ bl2_plat_get_bl31_params_default(bl32_entry, bl33_entry,
						fdt_addr);
}

ସଂରଚନା bl_params *bl2_plat_get_bl31_params_v2_default(ulong bl32_entry,
						      ulong bl33_entry,
						      ulong fdt_addr)
{
	ସ୍ଥିର ସଂରଚନା bl2_to_bl31_params_mem_v2 bl31_params_mem;
	ସଂରଚନା bl_params *bl_params;
	ସଂରଚନା bl_params_ଗଣ୍ଠି *bl_params_ଗଣ୍ଠି;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, ଆକାର(bl31_params_mem));

	/* Assign memory for TF related information */
	bl_params = &bl31_params_mem.bl_params;
	SET_PARAM_HEAD(bl_params, ATF_PARAM_BL_PARAMS, ATF_VERSION_2, 0);
	bl_params->head = &bl31_params_mem.bl31_params_ଗଣ୍ଠି;

	/* Fill BL31 related information */
	bl_params_ଗଣ୍ଠି = &bl31_params_mem.bl31_params_ଗଣ୍ଠି;
	bl_params_ଗଣ୍ଠି->image_id = ATF_BL31_IMAGE_ID;
	bl_params_ଗଣ୍ଠି->image_info = &bl31_params_mem.bl31_image_info;
	bl_params_ଗଣ୍ଠି->ep_info = &bl31_params_mem.bl31_ep_info;
	bl_params_ଗଣ୍ଠି->next_params_info = &bl31_params_mem.bl32_params_ଗଣ୍ଠି;
	SET_PARAM_HEAD(bl_params_ଗଣ୍ଠି->image_info, ATF_PARAM_IMAGE_ଦ୍ୱିମିକ,
		       ATF_VERSION_2, 0);

	/* Fill BL32 related information */
	bl_params_ଗଣ୍ଠି = &bl31_params_mem.bl32_params_ଗଣ୍ଠି;
	bl_params_ଗଣ୍ଠି->image_id = ATF_BL32_IMAGE_ID;
	bl_params_ଗଣ୍ଠି->image_info = &bl31_params_mem.bl32_image_info;
	bl_params_ଗଣ୍ଠି->ep_info = &bl31_params_mem.bl32_ep_info;
	bl_params_ଗଣ୍ଠି->next_params_info = &bl31_params_mem.bl33_params_ଗଣ୍ଠି;
	SET_PARAM_HEAD(bl_params_ଗଣ୍ଠି->ep_info, ATF_PARAM_EP,
		       ATF_VERSION_2, ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl_params_ଗଣ୍ଠି->ep_info->args.arg3 = fdt_addr;
	bl_params_ଗଣ୍ଠି->ep_info->pc = bl32_entry ? bl32_entry : 0;
	bl_params_ଗଣ୍ଠି->ep_info->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	SET_PARAM_HEAD(bl_params_ଗଣ୍ଠି->image_info, ATF_PARAM_IMAGE_ଦ୍ୱିମିକ,
		       ATF_VERSION_2, 0);

	/* Fill BL33 related information */
	bl_params_ଗଣ୍ଠି = &bl31_params_mem.bl33_params_ଗଣ୍ଠି;
	bl_params_ଗଣ୍ଠି->image_id = ATF_BL33_IMAGE_ID;
	bl_params_ଗଣ୍ଠି->image_info = &bl31_params_mem.bl33_image_info;
	bl_params_ଗଣ୍ଠି->ep_info = &bl31_params_mem.bl33_ep_info;
	bl_params_ଗଣ୍ଠି->next_params_info = ଖାଲି;
	SET_PARAM_HEAD(bl_params_ଗଣ୍ଠି->ep_info, ATF_PARAM_EP,
		       ATF_VERSION_2, ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl_params_ଗଣ୍ଠି->ep_info->args.arg0 = 0xffff & read_mpidr();
	bl_params_ଗଣ୍ଠି->ep_info->pc = bl33_entry;
	bl_params_ଗଣ୍ଠି->ep_info->spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	SET_PARAM_HEAD(bl_params_ଗଣ୍ଠି->image_info, ATF_PARAM_IMAGE_ଦ୍ୱିମିକ,
		       ATF_VERSION_2, 0);

	ଫେରନ୍ତୁ bl_params;
}

__weak ସଂରଚନା bl_params *bl2_plat_get_bl31_params_v2(ulong bl32_entry,
						     ulong bl33_entry,
						     ulong fdt_addr)
{
	ଫେରନ୍ତୁ bl2_plat_get_bl31_params_v2_default(bl32_entry, bl33_entry,
						   fdt_addr);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ raw_write_daif(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା daif)
{
	__asm__ __volatile__("msr DAIF, %x0\n\t" : : "r" (daif) : "memory");
}

ପ୍ରକାରବ୍ୟାଖ୍ୟା ଶୂନ୍ୟ __noreturn (*atf_entry_t)(ସଂରଚନା bl31_params *params, ଶୂନ୍ୟ *plat_params);

ସ୍ଥିର ଶୂନ୍ୟ __noreturn bl31_entry(ulong bl31_entry, ulong bl32_entry,
				  ulong bl33_entry, ulong fdt_addr)
{
	atf_entry_t  atf_entry = (atf_entry_t)bl31_entry;
	ଶୂନ୍ୟ *bl31_params;

	ଯଦି (CONFIG_IS_ENABLED(ATF_LOAD_IMAGE_V2))
		bl31_params = bl2_plat_get_bl31_params_v2(bl32_entry,
							  bl33_entry,
							  fdt_addr);
	ଅନ୍ୟଥା
		bl31_params = bl2_plat_get_bl31_params(bl32_entry, bl33_entry,
						       fdt_addr);

	raw_write_daif(SPSR_EXCEPTION_MASK);
	ଯଦି (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_disable();

	atf_entry(bl31_params, (ଶୂନ୍ୟ *)fdt_addr);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_fit_images_find(ଶୂନ୍ୟ *blob, ପୂର୍ଣ୍ଣସଂଖ୍ୟା os)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା parent, ଗଣ୍ଠି, ndepth = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା found = -FDT_ERR_NOTFOUND;
	ଅଚଳ ଶୂନ୍ୟ *data;
	ulong val;

	ଯଦି (!blob)
		ଫେରନ୍ତୁ -FDT_ERR_BADMAGIC;

	parent = fdt_path_offset(blob, "/fit-images");
	ଯଦି (parent < 0)
		ଫେରନ୍ତୁ -FDT_ERR_NOTFOUND;

	ପାଇଁ (ଗଣ୍ଠି = fdt_next_ଗଣ୍ଠି(blob, parent, &ndepth);
	     (ଗଣ୍ଠି >= 0) && (ndepth > 0);
	     ଗଣ୍ଠି = fdt_next_ଗଣ୍ଠି(blob, ଗଣ୍ଠି, &ndepth)) {
		ଯଦି (ndepth != 1)
			ଜାରି;

		data = fdt_getprop(blob, ଗଣ୍ଠି, FIT_OS_PROP, ଖାଲି);
		ଯଦି (!data)
			ଜାରି;

		ଯଦି (genimg_get_os_id(data) != os)
			ଜାରି;

		/*
		 * A multi-segment image, e.g. an OP-TEE ELF split by
		 * binman, is recorded as one node per segment, all with
		 * the same os. Only the segment holding the ELF entry
		 * point carries an entry property, so prefer that node.
		 */
		ଯଦି (!fit_image_get_entry(blob, ଗଣ୍ଠି, &val))
			ଫେରନ୍ତୁ ଗଣ୍ଠି;

		ଯଦି (found < 0)
			found = ଗଣ୍ଠି;
	};

	ଫେରନ୍ତୁ found;
}

ulong spl_fit_images_get_entry(ଶୂନ୍ୟ *blob, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣ୍ଠି)
{
	ulong  val;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ret = fit_image_get_entry(blob, ଗଣ୍ଠି, &val);
	ଯଦି (ret)
		ret = fit_image_get_load(blob, ଗଣ୍ଠି, &val);

	debug("%s: entry point 0x%lx\n", __func__, val);
	ଫେରନ୍ତୁ val;
}

ଶୂନ୍ୟ __noreturn spl_invoke_atf(ସଂରଚନା spl_image_info *spl_image)
{
	ulong  bl32_entry = 0;
	ulong  bl33_entry = CONFIG_TEXT_BASE;
	ଶୂନ୍ୟ *blob = spl_image->fdt_addr;
	ulong platform_param = (ulong)blob;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣ୍ଠି;

	/*
	 * Find the OP-TEE binary (in /fit-images) load address or
	 * entry point (if different) and pass it as the BL3-2 entry
	 * point, this is optional.
	 */
	ଗଣ୍ଠି = spl_fit_images_find(blob, IH_OS_TEE);
	ଯଦି (ଗଣ୍ଠି >= 0)
		bl32_entry = spl_fit_images_get_entry(blob, ଗଣ୍ଠି);

	/*
	 * Find the U-Boot binary (in /fit-images) load addreess or
	 * entry point (if different) and pass it as the BL3-3 entry
	 * point.
	 * This will need to be extended to support Falcon mode.
	 */

	ଗଣ୍ଠି = spl_fit_images_find(blob, IH_OS_U_BOOT);
	ଯଦି (ଗଣ୍ଠି >= 0)
		bl33_entry = spl_fit_images_get_entry(blob, ଗଣ୍ଠି);

	/*
	 * If ATF_NO_PLATFORM_PARAM is set, we override the platform
	 * parameter and always pass 0.  This is a workaround for
	 * older ATF versions that have insufficiently robust (or
	 * overzealous) argument validation.
	 */
	ଯଦି (CONFIG_IS_ENABLED(ATF_NO_PLATFORM_PARAM))
		platform_param = 0;

	/*
	 * We don't provide a BL3-2 entry yet, but this will be possible
	 * using similar logic.
	 */
	bl31_entry(spl_image->entry_point, bl32_entry,
		   bl33_entry, platform_param);
}
