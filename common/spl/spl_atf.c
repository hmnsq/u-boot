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

#समावेश <atf_common.h>
#समावेश <cpu_func.h>
#समावेश <errno.h>
#समावेश <image.h>
#समावेश <log.h>
#समावेश <spl.h>
#समावेश <asm/cache.h>

/* Holds all the structures we need for bl31 parameter passing */
काष्ठा bl2_to_bl31_params_mem {
	काष्ठा bl31_params bl31_params;
	काष्ठा atf_image_info bl31_image_info;
	काष्ठा atf_image_info bl32_image_info;
	काष्ठा atf_image_info bl33_image_info;
	काष्ठा entry_point_info bl33_ep_info;
	काष्ठा entry_point_info bl32_ep_info;
	काष्ठा entry_point_info bl31_ep_info;
};

काष्ठा bl2_to_bl31_params_mem_v2 {
	काष्ठा bl_params bl_params;
	काष्ठा bl_params_गाँठ bl31_params_गाँठ;
	काष्ठा bl_params_गाँठ bl32_params_गाँठ;
	काष्ठा bl_params_गाँठ bl33_params_गाँठ;
	काष्ठा atf_image_info bl31_image_info;
	काष्ठा atf_image_info bl32_image_info;
	काष्ठा atf_image_info bl33_image_info;
	काष्ठा entry_point_info bl33_ep_info;
	काष्ठा entry_point_info bl32_ep_info;
	काष्ठा entry_point_info bl31_ep_info;
};

काष्ठा bl31_params *bl2_plat_get_bl31_params_default(ulong bl32_entry,
						     ulong bl33_entry,
						     ulong fdt_addr)
{
	अटल काष्ठा bl2_to_bl31_params_mem bl31_params_mem;
	काष्ठा bl31_params *bl2_to_bl31_params;
	काष्ठा entry_point_info *bl32_ep_info;
	काष्ठा entry_point_info *bl33_ep_info;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	स_रखो(&bl31_params_mem, 0, माप(काष्ठा bl2_to_bl31_params_mem));

	/* Assign memory for TF related information */
	bl2_to_bl31_params = &bl31_params_mem.bl31_params;
	SET_PARAM_शीर्ष(bl2_to_bl31_params, ATF_PARAM_BL31, ATF_VERSION_1, 0);

	/* Fill BL31 related information */
	bl2_to_bl31_params->bl31_image_info = &bl31_params_mem.bl31_image_info;
	SET_PARAM_शीर्ष(bl2_to_bl31_params->bl31_image_info,
		       ATF_PARAM_IMAGE_द्विआधारी, ATF_VERSION_1, 0);

	/* Fill BL32 related information */
	bl2_to_bl31_params->bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	SET_PARAM_शीर्ष(bl32_ep_info, ATF_PARAM_EP, ATF_VERSION_1,
		       ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl32_ep_info->args.arg3 = fdt_addr;
	bl32_ep_info->pc = bl32_entry ? bl32_entry : 0;
	bl32_ep_info->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
				     DISABLE_ALL_EXCEPTIONS);

	bl2_to_bl31_params->bl32_image_info = &bl31_params_mem.bl32_image_info;
	SET_PARAM_शीर्ष(bl2_to_bl31_params->bl32_image_info,
		       ATF_PARAM_IMAGE_द्विआधारी, ATF_VERSION_1, 0);

	/* Fill BL33 related information */
	bl2_to_bl31_params->bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	SET_PARAM_शीर्ष(bl33_ep_info, ATF_PARAM_EP, ATF_VERSION_1,
		       ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl33_ep_info->args.arg0 = 0xffff & read_mpidr();
	bl33_ep_info->pc = bl33_entry;
	bl33_ep_info->spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
				     DISABLE_ALL_EXCEPTIONS);

	bl2_to_bl31_params->bl33_image_info = &bl31_params_mem.bl33_image_info;
	SET_PARAM_शीर्ष(bl2_to_bl31_params->bl33_image_info,
		       ATF_PARAM_IMAGE_द्विआधारी, ATF_VERSION_1, 0);

	वापस bl2_to_bl31_params;
}

__weak काष्ठा bl31_params *bl2_plat_get_bl31_params(ulong bl32_entry,
						    ulong bl33_entry,
						    ulong fdt_addr)
{
	वापस bl2_plat_get_bl31_params_default(bl32_entry, bl33_entry,
						fdt_addr);
}

काष्ठा bl_params *bl2_plat_get_bl31_params_v2_default(ulong bl32_entry,
						      ulong bl33_entry,
						      ulong fdt_addr)
{
	अटल काष्ठा bl2_to_bl31_params_mem_v2 bl31_params_mem;
	काष्ठा bl_params *bl_params;
	काष्ठा bl_params_गाँठ *bl_params_गाँठ;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	स_रखो(&bl31_params_mem, 0, माप(bl31_params_mem));

	/* Assign memory for TF related information */
	bl_params = &bl31_params_mem.bl_params;
	SET_PARAM_शीर्ष(bl_params, ATF_PARAM_BL_PARAMS, ATF_VERSION_2, 0);
	bl_params->शीर्ष_2 = &bl31_params_mem.bl31_params_गाँठ;

	/* Fill BL31 related information */
	bl_params_गाँठ = &bl31_params_mem.bl31_params_गाँठ;
	bl_params_गाँठ->image_id = ATF_BL31_IMAGE_ID;
	bl_params_गाँठ->image_info = &bl31_params_mem.bl31_image_info;
	bl_params_गाँठ->ep_info = &bl31_params_mem.bl31_ep_info;
	bl_params_गाँठ->अग्र_params_info = &bl31_params_mem.bl32_params_गाँठ;
	SET_PARAM_शीर्ष(bl_params_गाँठ->image_info, ATF_PARAM_IMAGE_द्विआधारी,
		       ATF_VERSION_2, 0);

	/* Fill BL32 related information */
	bl_params_गाँठ = &bl31_params_mem.bl32_params_गाँठ;
	bl_params_गाँठ->image_id = ATF_BL32_IMAGE_ID;
	bl_params_गाँठ->image_info = &bl31_params_mem.bl32_image_info;
	bl_params_गाँठ->ep_info = &bl31_params_mem.bl32_ep_info;
	bl_params_गाँठ->अग्र_params_info = &bl31_params_mem.bl33_params_गाँठ;
	SET_PARAM_शीर्ष(bl_params_गाँठ->ep_info, ATF_PARAM_EP,
		       ATF_VERSION_2, ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl_params_गाँठ->ep_info->args.arg3 = fdt_addr;
	bl_params_गाँठ->ep_info->pc = bl32_entry ? bl32_entry : 0;
	bl_params_गाँठ->ep_info->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	SET_PARAM_शीर्ष(bl_params_गाँठ->image_info, ATF_PARAM_IMAGE_द्विआधारी,
		       ATF_VERSION_2, 0);

	/* Fill BL33 related information */
	bl_params_गाँठ = &bl31_params_mem.bl33_params_गाँठ;
	bl_params_गाँठ->image_id = ATF_BL33_IMAGE_ID;
	bl_params_गाँठ->image_info = &bl31_params_mem.bl33_image_info;
	bl_params_गाँठ->ep_info = &bl31_params_mem.bl33_ep_info;
	bl_params_गाँठ->अग्र_params_info = शून्य;
	SET_PARAM_शीर्ष(bl_params_गाँठ->ep_info, ATF_PARAM_EP,
		       ATF_VERSION_2, ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl_params_गाँठ->ep_info->args.arg0 = 0xffff & read_mpidr();
	bl_params_गाँठ->ep_info->pc = bl33_entry;
	bl_params_गाँठ->ep_info->spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	SET_PARAM_शीर्ष(bl_params_गाँठ->image_info, ATF_PARAM_IMAGE_द्विआधारी,
		       ATF_VERSION_2, 0);

	वापस bl_params;
}

__weak काष्ठा bl_params *bl2_plat_get_bl31_params_v2(ulong bl32_entry,
						     ulong bl33_entry,
						     ulong fdt_addr)
{
	वापस bl2_plat_get_bl31_params_v2_default(bl32_entry, bl33_entry,
						   fdt_addr);
}

अटल अंतरभूत व्योम raw_write_daif(अचिन्हित पूर्णांक daif)
{
	__asm__ __volatile__("msr DAIF, %x0\n\t" : : "r" (daif) : "memory");
}

प्रकार व्योम __noreturn (*atf_entry_t)(काष्ठा bl31_params *params, व्योम *plat_params);

अटल व्योम __noreturn bl31_entry(ulong bl31_entry, ulong bl32_entry,
				  ulong bl33_entry, ulong fdt_addr)
{
	atf_entry_t  atf_entry = (atf_entry_t)bl31_entry;
	व्योम *bl31_params;

	अगर (CONFIG_IS_ENABLED(ATF_LOAD_IMAGE_V2))
		bl31_params = bl2_plat_get_bl31_params_v2(bl32_entry,
							  bl33_entry,
							  fdt_addr);
	अन्यथा
		bl31_params = bl2_plat_get_bl31_params(bl32_entry, bl33_entry,
						       fdt_addr);

	raw_write_daif(SPSR_EXCEPTION_MASK);
	अगर (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_disable();

	atf_entry(bl31_params, (व्योम *)fdt_addr);
}

अटल पूर्णांक spl_fit_images_find(व्योम *blob, पूर्णांक os)
{
	पूर्णांक parent, गाँठ, ndepth = 0;
	पूर्णांक found = -FDT_ERR_NOTFOUND;
	स्थिर व्योम *तथ्य;
	ulong val;

	अगर (!blob)
		वापस -FDT_ERR_BADMAGIC;

	parent = fdt_path_offset(blob, "/fit-images");
	अगर (parent < 0)
		वापस -FDT_ERR_NOTFOUND;

	क्रम (गाँठ = fdt_अग्र_गाँठ(blob, parent, &ndepth);
	     (गाँठ >= 0) && (ndepth > 0);
	     गाँठ = fdt_अग्र_गाँठ(blob, गाँठ, &ndepth)) {
		अगर (ndepth != 1)
			जारी;

		तथ्य = fdt_getprop(blob, गाँठ, FIT_OS_PROP, शून्य);
		अगर (!तथ्य)
			जारी;

		अगर (genimg_get_os_id(तथ्य) != os)
			जारी;

		/*
		 * A multi-segment image, e.g. an OP-TEE ELF split by
		 * binman, is recorded as one node per segment, all with
		 * the same os. Only the segment holding the ELF entry
		 * point carries an entry property, so prefer that node.
		 */
		अगर (!fit_image_get_entry(blob, गाँठ, &val))
			वापस गाँठ;

		अगर (found < 0)
			found = गाँठ;
	};

	वापस found;
}

ulong spl_fit_images_get_entry(व्योम *blob, पूर्णांक गाँठ)
{
	ulong  val;
	पूर्णांक ret;

	ret = fit_image_get_entry(blob, गाँठ, &val);
	अगर (ret)
		ret = fit_image_get_load(blob, गाँठ, &val);

	debug("%s: entry point 0x%lx\n", __func__, val);
	वापस val;
}

व्योम __noreturn spl_invoke_atf(काष्ठा spl_image_info *spl_image)
{
	ulong  bl32_entry = 0;
	ulong  bl33_entry = CONFIG_TEXT_BASE;
	व्योम *blob = spl_image->fdt_addr;
	ulong platform_param = (ulong)blob;
	पूर्णांक गाँठ;

	/*
	 * Find the OP-TEE binary (in /fit-images) load address or
	 * entry point (if different) and pass it as the BL3-2 entry
	 * point, this is optional.
	 */
	गाँठ = spl_fit_images_find(blob, IH_OS_TEE);
	अगर (गाँठ >= 0)
		bl32_entry = spl_fit_images_get_entry(blob, गाँठ);

	/*
	 * Find the U-Boot binary (in /fit-images) load addreess or
	 * entry point (if different) and pass it as the BL3-3 entry
	 * point.
	 * This will need to be extended to support Falcon mode.
	 */

	गाँठ = spl_fit_images_find(blob, IH_OS_U_BOOT);
	अगर (गाँठ >= 0)
		bl33_entry = spl_fit_images_get_entry(blob, गाँठ);

	/*
	 * If ATF_NO_PLATFORM_PARAM is set, we override the platform
	 * parameter and always pass 0.  This is a workaround for
	 * older ATF versions that have insufficiently robust (or
	 * overzealous) argument validation.
	 */
	अगर (CONFIG_IS_ENABLED(ATF_NO_PLATFORM_PARAM))
		platform_param = 0;

	/*
	 * We don't provide a BL3-2 entry yet, but this will be possible
	 * using similar logic.
	 */
	bl31_entry(spl_image->entry_point, bl32_entry,
		   bl33_entry, platform_param);
}
