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

#அந்தர்பாவ <atf_common.h>
#அந்தர்பாவ <cpu_func.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <image.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <asm/cache.h>

/* Holds all the structures we need for bl31 parameter passing */
கட்டமைப்பு bl2_to_bl31_params_mem {
	கட்டமைப்பு bl31_params bl31_params;
	கட்டமைப்பு atf_image_info bl31_image_info;
	கட்டமைப்பு atf_image_info bl32_image_info;
	கட்டமைப்பு atf_image_info bl33_image_info;
	கட்டமைப்பு entry_point_info bl33_ep_info;
	கட்டமைப்பு entry_point_info bl32_ep_info;
	கட்டமைப்பு entry_point_info bl31_ep_info;
};

கட்டமைப்பு bl2_to_bl31_params_mem_v2 {
	கட்டமைப்பு bl_params bl_params;
	கட்டமைப்பு bl_params_முனை bl31_params_முனை;
	கட்டமைப்பு bl_params_முனை bl32_params_முனை;
	கட்டமைப்பு bl_params_முனை bl33_params_முனை;
	கட்டமைப்பு atf_image_info bl31_image_info;
	கட்டமைப்பு atf_image_info bl32_image_info;
	கட்டமைப்பு atf_image_info bl33_image_info;
	கட்டமைப்பு entry_point_info bl33_ep_info;
	கட்டமைப்பு entry_point_info bl32_ep_info;
	கட்டமைப்பு entry_point_info bl31_ep_info;
};

கட்டமைப்பு bl31_params *bl2_plat_get_bl31_params_default(ulong bl32_entry,
						     ulong bl33_entry,
						     ulong fdt_addr)
{
	நிலையான கட்டமைப்பு bl2_to_bl31_params_mem bl31_params_mem;
	கட்டமைப்பு bl31_params *bl2_to_bl31_params;
	கட்டமைப்பு entry_point_info *bl32_ep_info;
	கட்டமைப்பு entry_point_info *bl33_ep_info;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, அளவு(கட்டமைப்பு bl2_to_bl31_params_mem));

	/* Assign memory for TF related information */
	bl2_to_bl31_params = &bl31_params_mem.bl31_params;
	SET_PARAM_HEAD(bl2_to_bl31_params, ATF_PARAM_BL31, ATF_VERSION_1, 0);

	/* Fill BL31 related information */
	bl2_to_bl31_params->bl31_image_info = &bl31_params_mem.bl31_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl31_image_info,
		       ATF_PARAM_IMAGE_இருமம், ATF_VERSION_1, 0);

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
		       ATF_PARAM_IMAGE_இருமம், ATF_VERSION_1, 0);

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
		       ATF_PARAM_IMAGE_இருமம், ATF_VERSION_1, 0);

	திரும்பு bl2_to_bl31_params;
}

__weak கட்டமைப்பு bl31_params *bl2_plat_get_bl31_params(ulong bl32_entry,
						    ulong bl33_entry,
						    ulong fdt_addr)
{
	திரும்பு bl2_plat_get_bl31_params_default(bl32_entry, bl33_entry,
						fdt_addr);
}

கட்டமைப்பு bl_params *bl2_plat_get_bl31_params_v2_default(ulong bl32_entry,
						      ulong bl33_entry,
						      ulong fdt_addr)
{
	நிலையான கட்டமைப்பு bl2_to_bl31_params_mem_v2 bl31_params_mem;
	கட்டமைப்பு bl_params *bl_params;
	கட்டமைப்பு bl_params_முனை *bl_params_முனை;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, அளவு(bl31_params_mem));

	/* Assign memory for TF related information */
	bl_params = &bl31_params_mem.bl_params;
	SET_PARAM_HEAD(bl_params, ATF_PARAM_BL_PARAMS, ATF_VERSION_2, 0);
	bl_params->head = &bl31_params_mem.bl31_params_முனை;

	/* Fill BL31 related information */
	bl_params_முனை = &bl31_params_mem.bl31_params_முனை;
	bl_params_முனை->image_id = ATF_BL31_IMAGE_ID;
	bl_params_முனை->image_info = &bl31_params_mem.bl31_image_info;
	bl_params_முனை->ep_info = &bl31_params_mem.bl31_ep_info;
	bl_params_முனை->next_params_info = &bl31_params_mem.bl32_params_முனை;
	SET_PARAM_HEAD(bl_params_முனை->image_info, ATF_PARAM_IMAGE_இருமம்,
		       ATF_VERSION_2, 0);

	/* Fill BL32 related information */
	bl_params_முனை = &bl31_params_mem.bl32_params_முனை;
	bl_params_முனை->image_id = ATF_BL32_IMAGE_ID;
	bl_params_முனை->image_info = &bl31_params_mem.bl32_image_info;
	bl_params_முனை->ep_info = &bl31_params_mem.bl32_ep_info;
	bl_params_முனை->next_params_info = &bl31_params_mem.bl33_params_முனை;
	SET_PARAM_HEAD(bl_params_முனை->ep_info, ATF_PARAM_EP,
		       ATF_VERSION_2, ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl_params_முனை->ep_info->args.arg3 = fdt_addr;
	bl_params_முனை->ep_info->pc = bl32_entry ? bl32_entry : 0;
	bl_params_முனை->ep_info->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	SET_PARAM_HEAD(bl_params_முனை->image_info, ATF_PARAM_IMAGE_இருமம்,
		       ATF_VERSION_2, 0);

	/* Fill BL33 related information */
	bl_params_முனை = &bl31_params_mem.bl33_params_முனை;
	bl_params_முனை->image_id = ATF_BL33_IMAGE_ID;
	bl_params_முனை->image_info = &bl31_params_mem.bl33_image_info;
	bl_params_முனை->ep_info = &bl31_params_mem.bl33_ep_info;
	bl_params_முனை->next_params_info = வெற்று;
	SET_PARAM_HEAD(bl_params_முனை->ep_info, ATF_PARAM_EP,
		       ATF_VERSION_2, ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl_params_முனை->ep_info->args.arg0 = 0xffff & read_mpidr();
	bl_params_முனை->ep_info->pc = bl33_entry;
	bl_params_முனை->ep_info->spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	SET_PARAM_HEAD(bl_params_முனை->image_info, ATF_PARAM_IMAGE_இருமம்,
		       ATF_VERSION_2, 0);

	திரும்பு bl_params;
}

__weak கட்டமைப்பு bl_params *bl2_plat_get_bl31_params_v2(ulong bl32_entry,
						     ulong bl33_entry,
						     ulong fdt_addr)
{
	திரும்பு bl2_plat_get_bl31_params_v2_default(bl32_entry, bl33_entry,
						   fdt_addr);
}

நிலையான உள்வரி வெறுமை raw_write_daif(unsigned முழு daif)
{
	__asm__ __volatile__("msr DAIF, %x0\n\t" : : "r" (daif) : "memory");
}

வகைவரை வெறுமை __noreturn (*atf_entry_t)(கட்டமைப்பு bl31_params *params, வெறுமை *plat_params);

நிலையான வெறுமை __noreturn bl31_entry(ulong bl31_entry, ulong bl32_entry,
				  ulong bl33_entry, ulong fdt_addr)
{
	atf_entry_t  atf_entry = (atf_entry_t)bl31_entry;
	வெறுமை *bl31_params;

	எனில் (CONFIG_IS_ENABLED(ATF_LOAD_IMAGE_V2))
		bl31_params = bl2_plat_get_bl31_params_v2(bl32_entry,
							  bl33_entry,
							  fdt_addr);
	இல்லையெனில்
		bl31_params = bl2_plat_get_bl31_params(bl32_entry, bl33_entry,
						       fdt_addr);

	raw_write_daif(SPSR_EXCEPTION_MASK);
	எனில் (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_disable();

	atf_entry(bl31_params, (வெறுமை *)fdt_addr);
}

நிலையான முழு spl_fit_images_find(வெறுமை *blob, முழு os)
{
	முழு parent, முனை, ndepth = 0;
	முழு found = -FDT_ERR_NOTFOUND;
	மாறிலி வெறுமை *data;
	ulong val;

	எனில் (!blob)
		திரும்பு -FDT_ERR_BADMAGIC;

	parent = fdt_path_offset(blob, "/fit-images");
	எனில் (parent < 0)
		திரும்பு -FDT_ERR_NOTFOUND;

	க்கு (முனை = fdt_next_முனை(blob, parent, &ndepth);
	     (முனை >= 0) && (ndepth > 0);
	     முனை = fdt_next_முனை(blob, முனை, &ndepth)) {
		எனில் (ndepth != 1)
			தொடர்;

		data = fdt_getprop(blob, முனை, FIT_OS_PROP, வெற்று);
		எனில் (!data)
			தொடர்;

		எனில் (genimg_get_os_id(data) != os)
			தொடர்;

		/*
		 * A multi-segment image, e.g. an OP-TEE ELF split by
		 * binman, is recorded as one node per segment, all with
		 * the same os. Only the segment holding the ELF entry
		 * point carries an entry property, so prefer that node.
		 */
		எனில் (!fit_image_get_entry(blob, முனை, &val))
			திரும்பு முனை;

		எனில் (found < 0)
			found = முனை;
	};

	திரும்பு found;
}

ulong spl_fit_images_get_entry(வெறுமை *blob, முழு முனை)
{
	ulong  val;
	முழு ret;

	ret = fit_image_get_entry(blob, முனை, &val);
	எனில் (ret)
		ret = fit_image_get_load(blob, முனை, &val);

	debug("%s: entry point 0x%lx\n", __func__, val);
	திரும்பு val;
}

வெறுமை __noreturn spl_invoke_atf(கட்டமைப்பு spl_image_info *spl_image)
{
	ulong  bl32_entry = 0;
	ulong  bl33_entry = CONFIG_TEXT_BASE;
	வெறுமை *blob = spl_image->fdt_addr;
	ulong platform_param = (ulong)blob;
	முழு முனை;

	/*
	 * Find the OP-TEE binary (in /fit-images) load address or
	 * entry point (if different) and pass it as the BL3-2 entry
	 * point, this is optional.
	 */
	முனை = spl_fit_images_find(blob, IH_OS_TEE);
	எனில் (முனை >= 0)
		bl32_entry = spl_fit_images_get_entry(blob, முனை);

	/*
	 * Find the U-Boot binary (in /fit-images) load addreess or
	 * entry point (if different) and pass it as the BL3-3 entry
	 * point.
	 * This will need to be extended to support Falcon mode.
	 */

	முனை = spl_fit_images_find(blob, IH_OS_U_BOOT);
	எனில் (முனை >= 0)
		bl33_entry = spl_fit_images_get_entry(blob, முனை);

	/*
	 * If ATF_NO_PLATFORM_PARAM is set, we override the platform
	 * parameter and always pass 0.  This is a workaround for
	 * older ATF versions that have insufficiently robust (or
	 * overzealous) argument validation.
	 */
	எனில் (CONFIG_IS_ENABLED(ATF_NO_PLATFORM_PARAM))
		platform_param = 0;

	/*
	 * We don't provide a BL3-2 entry yet, but this will be possible
	 * using similar logic.
	 */
	bl31_entry(spl_image->entry_point, bl32_entry,
		   bl33_entry, platform_param);
}
