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

#અન્તર્ભાવ <atf_common.h>
#અન્તર્ભાવ <cpu_func.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <image.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <asm/cache.h>

/* Holds all the structures we need for bl31 parameter passing */
સંરચના bl2_to_bl31_params_mem {
	સંરચના bl31_params bl31_params;
	સંરચના atf_image_info bl31_image_info;
	સંરચના atf_image_info bl32_image_info;
	સંરચના atf_image_info bl33_image_info;
	સંરચના entry_point_info bl33_ep_info;
	સંરચના entry_point_info bl32_ep_info;
	સંરચના entry_point_info bl31_ep_info;
};

સંરચના bl2_to_bl31_params_mem_v2 {
	સંરચના bl_params bl_params;
	સંરચના bl_params_ગાંઠ bl31_params_ગાંઠ;
	સંરચના bl_params_ગાંઠ bl32_params_ગાંઠ;
	સંરચના bl_params_ગાંઠ bl33_params_ગાંઠ;
	સંરચના atf_image_info bl31_image_info;
	સંરચના atf_image_info bl32_image_info;
	સંરચના atf_image_info bl33_image_info;
	સંરચના entry_point_info bl33_ep_info;
	સંરચના entry_point_info bl32_ep_info;
	સંરચના entry_point_info bl31_ep_info;
};

સંરચના bl31_params *bl2_plat_get_bl31_params_default(ulong bl32_entry,
						     ulong bl33_entry,
						     ulong fdt_addr)
{
	સ્થિર સંરચના bl2_to_bl31_params_mem bl31_params_mem;
	સંરચના bl31_params *bl2_to_bl31_params;
	સંરચના entry_point_info *bl32_ep_info;
	સંરચના entry_point_info *bl33_ep_info;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, કદ(સંરચના bl2_to_bl31_params_mem));

	/* Assign memory for TF related information */
	bl2_to_bl31_params = &bl31_params_mem.bl31_params;
	SET_PARAM_HEAD(bl2_to_bl31_params, ATF_PARAM_BL31, ATF_VERSION_1, 0);

	/* Fill BL31 related information */
	bl2_to_bl31_params->bl31_image_info = &bl31_params_mem.bl31_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl31_image_info,
		       ATF_PARAM_IMAGE_દ્વિઅંકી, ATF_VERSION_1, 0);

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
		       ATF_PARAM_IMAGE_દ્વિઅંકી, ATF_VERSION_1, 0);

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
		       ATF_PARAM_IMAGE_દ્વિઅંકી, ATF_VERSION_1, 0);

	પાછા bl2_to_bl31_params;
}

__weak સંરચના bl31_params *bl2_plat_get_bl31_params(ulong bl32_entry,
						    ulong bl33_entry,
						    ulong fdt_addr)
{
	પાછા bl2_plat_get_bl31_params_default(bl32_entry, bl33_entry,
						fdt_addr);
}

સંરચના bl_params *bl2_plat_get_bl31_params_v2_default(ulong bl32_entry,
						      ulong bl33_entry,
						      ulong fdt_addr)
{
	સ્થિર સંરચના bl2_to_bl31_params_mem_v2 bl31_params_mem;
	સંરચના bl_params *bl_params;
	સંરચના bl_params_ગાંઠ *bl_params_ગાંઠ;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, કદ(bl31_params_mem));

	/* Assign memory for TF related information */
	bl_params = &bl31_params_mem.bl_params;
	SET_PARAM_HEAD(bl_params, ATF_PARAM_BL_PARAMS, ATF_VERSION_2, 0);
	bl_params->head = &bl31_params_mem.bl31_params_ગાંઠ;

	/* Fill BL31 related information */
	bl_params_ગાંઠ = &bl31_params_mem.bl31_params_ગાંઠ;
	bl_params_ગાંઠ->image_id = ATF_BL31_IMAGE_ID;
	bl_params_ગાંઠ->image_info = &bl31_params_mem.bl31_image_info;
	bl_params_ગાંઠ->ep_info = &bl31_params_mem.bl31_ep_info;
	bl_params_ગાંઠ->next_params_info = &bl31_params_mem.bl32_params_ગાંઠ;
	SET_PARAM_HEAD(bl_params_ગાંઠ->image_info, ATF_PARAM_IMAGE_દ્વિઅંકી,
		       ATF_VERSION_2, 0);

	/* Fill BL32 related information */
	bl_params_ગાંઠ = &bl31_params_mem.bl32_params_ગાંઠ;
	bl_params_ગાંઠ->image_id = ATF_BL32_IMAGE_ID;
	bl_params_ગાંઠ->image_info = &bl31_params_mem.bl32_image_info;
	bl_params_ગાંઠ->ep_info = &bl31_params_mem.bl32_ep_info;
	bl_params_ગાંઠ->next_params_info = &bl31_params_mem.bl33_params_ગાંઠ;
	SET_PARAM_HEAD(bl_params_ગાંઠ->ep_info, ATF_PARAM_EP,
		       ATF_VERSION_2, ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl_params_ગાંઠ->ep_info->args.arg3 = fdt_addr;
	bl_params_ગાંઠ->ep_info->pc = bl32_entry ? bl32_entry : 0;
	bl_params_ગાંઠ->ep_info->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	SET_PARAM_HEAD(bl_params_ગાંઠ->image_info, ATF_PARAM_IMAGE_દ્વિઅંકી,
		       ATF_VERSION_2, 0);

	/* Fill BL33 related information */
	bl_params_ગાંઠ = &bl31_params_mem.bl33_params_ગાંઠ;
	bl_params_ગાંઠ->image_id = ATF_BL33_IMAGE_ID;
	bl_params_ગાંઠ->image_info = &bl31_params_mem.bl33_image_info;
	bl_params_ગાંઠ->ep_info = &bl31_params_mem.bl33_ep_info;
	bl_params_ગાંઠ->next_params_info = ખાલી;
	SET_PARAM_HEAD(bl_params_ગાંઠ->ep_info, ATF_PARAM_EP,
		       ATF_VERSION_2, ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl_params_ગાંઠ->ep_info->args.arg0 = 0xffff & read_mpidr();
	bl_params_ગાંઠ->ep_info->pc = bl33_entry;
	bl_params_ગાંઠ->ep_info->spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	SET_PARAM_HEAD(bl_params_ગાંઠ->image_info, ATF_PARAM_IMAGE_દ્વિઅંકી,
		       ATF_VERSION_2, 0);

	પાછા bl_params;
}

__weak સંરચના bl_params *bl2_plat_get_bl31_params_v2(ulong bl32_entry,
						     ulong bl33_entry,
						     ulong fdt_addr)
{
	પાછા bl2_plat_get_bl31_params_v2_default(bl32_entry, bl33_entry,
						   fdt_addr);
}

સ્થિર અંતઃરેખ શૂન્ય raw_write_daif(unsigned પૂર્ણાંક daif)
{
	__asm__ __volatile__("msr DAIF, %x0\n\t" : : "r" (daif) : "memory");
}

પ્રકારવ્યાખ્યા શૂન્ય __noreturn (*atf_entry_t)(સંરચના bl31_params *params, શૂન્ય *plat_params);

સ્થિર શૂન્ય __noreturn bl31_entry(ulong bl31_entry, ulong bl32_entry,
				  ulong bl33_entry, ulong fdt_addr)
{
	atf_entry_t  atf_entry = (atf_entry_t)bl31_entry;
	શૂન્ય *bl31_params;

	જો (CONFIG_IS_ENABLED(ATF_LOAD_IMAGE_V2))
		bl31_params = bl2_plat_get_bl31_params_v2(bl32_entry,
							  bl33_entry,
							  fdt_addr);
	નહીંતર
		bl31_params = bl2_plat_get_bl31_params(bl32_entry, bl33_entry,
						       fdt_addr);

	raw_write_daif(SPSR_EXCEPTION_MASK);
	જો (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_disable();

	atf_entry(bl31_params, (શૂન્ય *)fdt_addr);
}

સ્થિર પૂર્ણાંક spl_fit_images_find(શૂન્ય *blob, પૂર્ણાંક os)
{
	પૂર્ણાંક parent, ગાંઠ, ndepth = 0;
	પૂર્ણાંક found = -FDT_ERR_NOTFOUND;
	અચલ શૂન્ય *data;
	ulong val;

	જો (!blob)
		પાછા -FDT_ERR_BADMAGIC;

	parent = fdt_path_offset(blob, "/fit-images");
	જો (parent < 0)
		પાછા -FDT_ERR_NOTFOUND;

	માટે (ગાંઠ = fdt_next_ગાંઠ(blob, parent, &ndepth);
	     (ગાંઠ >= 0) && (ndepth > 0);
	     ગાંઠ = fdt_next_ગાંઠ(blob, ગાંઠ, &ndepth)) {
		જો (ndepth != 1)
			ચાલુ;

		data = fdt_getprop(blob, ગાંઠ, FIT_OS_PROP, ખાલી);
		જો (!data)
			ચાલુ;

		જો (genimg_get_os_id(data) != os)
			ચાલુ;

		/*
		 * A multi-segment image, e.g. an OP-TEE ELF split by
		 * binman, is recorded as one node per segment, all with
		 * the same os. Only the segment holding the ELF entry
		 * point carries an entry property, so prefer that node.
		 */
		જો (!fit_image_get_entry(blob, ગાંઠ, &val))
			પાછા ગાંઠ;

		જો (found < 0)
			found = ગાંઠ;
	};

	પાછા found;
}

ulong spl_fit_images_get_entry(શૂન્ય *blob, પૂર્ણાંક ગાંઠ)
{
	ulong  val;
	પૂર્ણાંક ret;

	ret = fit_image_get_entry(blob, ગાંઠ, &val);
	જો (ret)
		ret = fit_image_get_load(blob, ગાંઠ, &val);

	debug("%s: entry point 0x%lx\n", __func__, val);
	પાછા val;
}

શૂન્ય __noreturn spl_invoke_atf(સંરચના spl_image_info *spl_image)
{
	ulong  bl32_entry = 0;
	ulong  bl33_entry = CONFIG_TEXT_BASE;
	શૂન્ય *blob = spl_image->fdt_addr;
	ulong platform_param = (ulong)blob;
	પૂર્ણાંક ગાંઠ;

	/*
	 * Find the OP-TEE binary (in /fit-images) load address or
	 * entry point (if different) and pass it as the BL3-2 entry
	 * point, this is optional.
	 */
	ગાંઠ = spl_fit_images_find(blob, IH_OS_TEE);
	જો (ગાંઠ >= 0)
		bl32_entry = spl_fit_images_get_entry(blob, ગાંઠ);

	/*
	 * Find the U-Boot binary (in /fit-images) load addreess or
	 * entry point (if different) and pass it as the BL3-3 entry
	 * point.
	 * This will need to be extended to support Falcon mode.
	 */

	ગાંઠ = spl_fit_images_find(blob, IH_OS_U_BOOT);
	જો (ગાંઠ >= 0)
		bl33_entry = spl_fit_images_get_entry(blob, ગાંઠ);

	/*
	 * If ATF_NO_PLATFORM_PARAM is set, we override the platform
	 * parameter and always pass 0.  This is a workaround for
	 * older ATF versions that have insufficiently robust (or
	 * overzealous) argument validation.
	 */
	જો (CONFIG_IS_ENABLED(ATF_NO_PLATFORM_PARAM))
		platform_param = 0;

	/*
	 * We don't provide a BL3-2 entry yet, but this will be possible
	 * using similar logic.
	 */
	bl31_entry(spl_image->entry_point, bl32_entry,
		   bl33_entry, platform_param);
}
