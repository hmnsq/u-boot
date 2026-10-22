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

#അന്തര്ഭാവ <atf_common.h>
#അന്തര്ഭാവ <cpu_func.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <image.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <asm/cache.h>

/* Holds all the structures we need for bl31 parameter passing */
ഘടന bl2_to_bl31_params_mem {
	ഘടന bl31_params bl31_params;
	ഘടന atf_image_info bl31_image_info;
	ഘടന atf_image_info bl32_image_info;
	ഘടന atf_image_info bl33_image_info;
	ഘടന entry_point_info bl33_ep_info;
	ഘടന entry_point_info bl32_ep_info;
	ഘടന entry_point_info bl31_ep_info;
};

ഘടന bl2_to_bl31_params_mem_v2 {
	ഘടന bl_params bl_params;
	ഘടന bl_params_കണ്ണി bl31_params_കണ്ണി;
	ഘടന bl_params_കണ്ണി bl32_params_കണ്ണി;
	ഘടന bl_params_കണ്ണി bl33_params_കണ്ണി;
	ഘടന atf_image_info bl31_image_info;
	ഘടന atf_image_info bl32_image_info;
	ഘടന atf_image_info bl33_image_info;
	ഘടന entry_point_info bl33_ep_info;
	ഘടന entry_point_info bl32_ep_info;
	ഘടന entry_point_info bl31_ep_info;
};

ഘടന bl31_params *bl2_plat_get_bl31_params_default(ulong bl32_entry,
						     ulong bl33_entry,
						     ulong fdt_addr)
{
	സ്ഥിതം ഘടന bl2_to_bl31_params_mem bl31_params_mem;
	ഘടന bl31_params *bl2_to_bl31_params;
	ഘടന entry_point_info *bl32_ep_info;
	ഘടന entry_point_info *bl33_ep_info;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, വലുപ്പം(ഘടന bl2_to_bl31_params_mem));

	/* Assign memory for TF related information */
	bl2_to_bl31_params = &bl31_params_mem.bl31_params;
	SET_PARAM_HEAD(bl2_to_bl31_params, ATF_PARAM_BL31, ATF_VERSION_1, 0);

	/* Fill BL31 related information */
	bl2_to_bl31_params->bl31_image_info = &bl31_params_mem.bl31_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl31_image_info,
		       ATF_PARAM_IMAGE_ദ്വിമാന, ATF_VERSION_1, 0);

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
		       ATF_PARAM_IMAGE_ദ്വിമാന, ATF_VERSION_1, 0);

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
		       ATF_PARAM_IMAGE_ദ്വിമാന, ATF_VERSION_1, 0);

	തിരികെ bl2_to_bl31_params;
}

__weak ഘടന bl31_params *bl2_plat_get_bl31_params(ulong bl32_entry,
						    ulong bl33_entry,
						    ulong fdt_addr)
{
	തിരികെ bl2_plat_get_bl31_params_default(bl32_entry, bl33_entry,
						fdt_addr);
}

ഘടന bl_params *bl2_plat_get_bl31_params_v2_default(ulong bl32_entry,
						      ulong bl33_entry,
						      ulong fdt_addr)
{
	സ്ഥിതം ഘടന bl2_to_bl31_params_mem_v2 bl31_params_mem;
	ഘടന bl_params *bl_params;
	ഘടന bl_params_കണ്ണി *bl_params_കണ്ണി;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, വലുപ്പം(bl31_params_mem));

	/* Assign memory for TF related information */
	bl_params = &bl31_params_mem.bl_params;
	SET_PARAM_HEAD(bl_params, ATF_PARAM_BL_PARAMS, ATF_VERSION_2, 0);
	bl_params->head = &bl31_params_mem.bl31_params_കണ്ണി;

	/* Fill BL31 related information */
	bl_params_കണ്ണി = &bl31_params_mem.bl31_params_കണ്ണി;
	bl_params_കണ്ണി->image_id = ATF_BL31_IMAGE_ID;
	bl_params_കണ്ണി->image_info = &bl31_params_mem.bl31_image_info;
	bl_params_കണ്ണി->ep_info = &bl31_params_mem.bl31_ep_info;
	bl_params_കണ്ണി->next_params_info = &bl31_params_mem.bl32_params_കണ്ണി;
	SET_PARAM_HEAD(bl_params_കണ്ണി->image_info, ATF_PARAM_IMAGE_ദ്വിമാന,
		       ATF_VERSION_2, 0);

	/* Fill BL32 related information */
	bl_params_കണ്ണി = &bl31_params_mem.bl32_params_കണ്ണി;
	bl_params_കണ്ണി->image_id = ATF_BL32_IMAGE_ID;
	bl_params_കണ്ണി->image_info = &bl31_params_mem.bl32_image_info;
	bl_params_കണ്ണി->ep_info = &bl31_params_mem.bl32_ep_info;
	bl_params_കണ്ണി->next_params_info = &bl31_params_mem.bl33_params_കണ്ണി;
	SET_PARAM_HEAD(bl_params_കണ്ണി->ep_info, ATF_PARAM_EP,
		       ATF_VERSION_2, ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl_params_കണ്ണി->ep_info->args.arg3 = fdt_addr;
	bl_params_കണ്ണി->ep_info->pc = bl32_entry ? bl32_entry : 0;
	bl_params_കണ്ണി->ep_info->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	SET_PARAM_HEAD(bl_params_കണ്ണി->image_info, ATF_PARAM_IMAGE_ദ്വിമാന,
		       ATF_VERSION_2, 0);

	/* Fill BL33 related information */
	bl_params_കണ്ണി = &bl31_params_mem.bl33_params_കണ്ണി;
	bl_params_കണ്ണി->image_id = ATF_BL33_IMAGE_ID;
	bl_params_കണ്ണി->image_info = &bl31_params_mem.bl33_image_info;
	bl_params_കണ്ണി->ep_info = &bl31_params_mem.bl33_ep_info;
	bl_params_കണ്ണി->next_params_info = ശൂന്യ;
	SET_PARAM_HEAD(bl_params_കണ്ണി->ep_info, ATF_PARAM_EP,
		       ATF_VERSION_2, ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl_params_കണ്ണി->ep_info->args.arg0 = 0xffff & read_mpidr();
	bl_params_കണ്ണി->ep_info->pc = bl33_entry;
	bl_params_കണ്ണി->ep_info->spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	SET_PARAM_HEAD(bl_params_കണ്ണി->image_info, ATF_PARAM_IMAGE_ദ്വിമാന,
		       ATF_VERSION_2, 0);

	തിരികെ bl_params;
}

__weak ഘടന bl_params *bl2_plat_get_bl31_params_v2(ulong bl32_entry,
						     ulong bl33_entry,
						     ulong fdt_addr)
{
	തിരികെ bl2_plat_get_bl31_params_v2_default(bl32_entry, bl33_entry,
						   fdt_addr);
}

സ്ഥിതം വരിയിൽ ശൂന്യം raw_write_daif(unsigned പൂർണ്ണസംഖ്യ daif)
{
	__asm__ __volatile__("msr DAIF, %x0\n\t" : : "r" (daif) : "memory");
}

തരനിർവചനം ശൂന്യം __noreturn (*atf_entry_t)(ഘടന bl31_params *params, ശൂന്യം *plat_params);

സ്ഥിതം ശൂന്യം __noreturn bl31_entry(ulong bl31_entry, ulong bl32_entry,
				  ulong bl33_entry, ulong fdt_addr)
{
	atf_entry_t  atf_entry = (atf_entry_t)bl31_entry;
	ശൂന്യം *bl31_params;

	എങ്കിൽ (CONFIG_IS_ENABLED(ATF_LOAD_IMAGE_V2))
		bl31_params = bl2_plat_get_bl31_params_v2(bl32_entry,
							  bl33_entry,
							  fdt_addr);
	അല്ലെങ്കിൽ
		bl31_params = bl2_plat_get_bl31_params(bl32_entry, bl33_entry,
						       fdt_addr);

	raw_write_daif(SPSR_EXCEPTION_MASK);
	എങ്കിൽ (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_disable();

	atf_entry(bl31_params, (ശൂന്യം *)fdt_addr);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_fit_images_find(ശൂന്യം *blob, പൂർണ്ണസംഖ്യ os)
{
	പൂർണ്ണസംഖ്യ parent, കണ്ണി, ndepth = 0;
	പൂർണ്ണസംഖ്യ found = -FDT_ERR_NOTFOUND;
	സ്ഥിരം ശൂന്യം *data;
	ulong val;

	എങ്കിൽ (!blob)
		തിരികെ -FDT_ERR_BADMAGIC;

	parent = fdt_path_offset(blob, "/fit-images");
	എങ്കിൽ (parent < 0)
		തിരികെ -FDT_ERR_NOTFOUND;

	വേണ്ടി (കണ്ണി = fdt_next_കണ്ണി(blob, parent, &ndepth);
	     (കണ്ണി >= 0) && (ndepth > 0);
	     കണ്ണി = fdt_next_കണ്ണി(blob, കണ്ണി, &ndepth)) {
		എങ്കിൽ (ndepth != 1)
			തുടരുക;

		data = fdt_getprop(blob, കണ്ണി, FIT_OS_PROP, ശൂന്യ);
		എങ്കിൽ (!data)
			തുടരുക;

		എങ്കിൽ (genimg_get_os_id(data) != os)
			തുടരുക;

		/*
		 * A multi-segment image, e.g. an OP-TEE ELF split by
		 * binman, is recorded as one node per segment, all with
		 * the same os. Only the segment holding the ELF entry
		 * point carries an entry property, so prefer that node.
		 */
		എങ്കിൽ (!fit_image_get_entry(blob, കണ്ണി, &val))
			തിരികെ കണ്ണി;

		എങ്കിൽ (found < 0)
			found = കണ്ണി;
	};

	തിരികെ found;
}

ulong spl_fit_images_get_entry(ശൂന്യം *blob, പൂർണ്ണസംഖ്യ കണ്ണി)
{
	ulong  val;
	പൂർണ്ണസംഖ്യ ret;

	ret = fit_image_get_entry(blob, കണ്ണി, &val);
	എങ്കിൽ (ret)
		ret = fit_image_get_load(blob, കണ്ണി, &val);

	debug("%s: entry point 0x%lx\n", __func__, val);
	തിരികെ val;
}

ശൂന്യം __noreturn spl_invoke_atf(ഘടന spl_image_info *spl_image)
{
	ulong  bl32_entry = 0;
	ulong  bl33_entry = CONFIG_TEXT_BASE;
	ശൂന്യം *blob = spl_image->fdt_addr;
	ulong platform_param = (ulong)blob;
	പൂർണ്ണസംഖ്യ കണ്ണി;

	/*
	 * Find the OP-TEE binary (in /fit-images) load address or
	 * entry point (if different) and pass it as the BL3-2 entry
	 * point, this is optional.
	 */
	കണ്ണി = spl_fit_images_find(blob, IH_OS_TEE);
	എങ്കിൽ (കണ്ണി >= 0)
		bl32_entry = spl_fit_images_get_entry(blob, കണ്ണി);

	/*
	 * Find the U-Boot binary (in /fit-images) load addreess or
	 * entry point (if different) and pass it as the BL3-3 entry
	 * point.
	 * This will need to be extended to support Falcon mode.
	 */

	കണ്ണി = spl_fit_images_find(blob, IH_OS_U_BOOT);
	എങ്കിൽ (കണ്ണി >= 0)
		bl33_entry = spl_fit_images_get_entry(blob, കണ്ണി);

	/*
	 * If ATF_NO_PLATFORM_PARAM is set, we override the platform
	 * parameter and always pass 0.  This is a workaround for
	 * older ATF versions that have insufficiently robust (or
	 * overzealous) argument validation.
	 */
	എങ്കിൽ (CONFIG_IS_ENABLED(ATF_NO_PLATFORM_PARAM))
		platform_param = 0;

	/*
	 * We don't provide a BL3-2 entry yet, but this will be possible
	 * using similar logic.
	 */
	bl31_entry(spl_image->entry_point, bl32_entry,
		   bl33_entry, platform_param);
}
