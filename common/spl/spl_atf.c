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

#ضمن <atf_common.h>
#ضمن <cpu_func.h>
#ضمن <errno.h>
#ضمن <image.h>
#ضمن <log.h>
#ضمن <spl.h>
#ضمن <asm/cache.h>

/* Holds all the structures we need for bl31 parameter passing */
بنية bl2_to_bl31_params_mem {
	بنية bl31_params bl31_params;
	بنية atf_image_info bl31_image_info;
	بنية atf_image_info bl32_image_info;
	بنية atf_image_info bl33_image_info;
	بنية entry_point_info bl33_ep_info;
	بنية entry_point_info bl32_ep_info;
	بنية entry_point_info bl31_ep_info;
};

بنية bl2_to_bl31_params_mem_v2 {
	بنية bl_params bl_params;
	بنية bl_params_node bl31_params_node;
	بنية bl_params_node bl32_params_node;
	بنية bl_params_node bl33_params_node;
	بنية atf_image_info bl31_image_info;
	بنية atf_image_info bl32_image_info;
	بنية atf_image_info bl33_image_info;
	بنية entry_point_info bl33_ep_info;
	بنية entry_point_info bl32_ep_info;
	بنية entry_point_info bl31_ep_info;
};

بنية bl31_params *bl2_plat_get_bl31_params_default(ulong bl32_entry,
						     ulong bl33_entry,
						     ulong fdt_addr)
{
	ساكن بنية bl2_to_bl31_params_mem bl31_params_mem;
	بنية bl31_params *bl2_to_bl31_params;
	بنية entry_point_info *bl32_ep_info;
	بنية entry_point_info *bl33_ep_info;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, حجم(بنية bl2_to_bl31_params_mem));

	/* Assign memory for TF related information */
	bl2_to_bl31_params = &bl31_params_mem.bl31_params;
	SET_PARAM_HEAD(bl2_to_bl31_params, ATF_PARAM_BL31, ATF_VERSION_1, 0);

	/* Fill BL31 related information */
	bl2_to_bl31_params->bl31_image_info = &bl31_params_mem.bl31_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl31_image_info,
		       ATF_PARAM_IMAGE_BINARY, ATF_VERSION_1, 0);

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
		       ATF_PARAM_IMAGE_BINARY, ATF_VERSION_1, 0);

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
		       ATF_PARAM_IMAGE_BINARY, ATF_VERSION_1, 0);

	ܦܢܐ bl2_to_bl31_params;
}

__weak بنية bl31_params *bl2_plat_get_bl31_params(ulong bl32_entry,
						    ulong bl33_entry,
						    ulong fdt_addr)
{
	ܦܢܐ bl2_plat_get_bl31_params_default(bl32_entry, bl33_entry,
						fdt_addr);
}

بنية bl_params *bl2_plat_get_bl31_params_v2_default(ulong bl32_entry,
						      ulong bl33_entry,
						      ulong fdt_addr)
{
	ساكن بنية bl2_to_bl31_params_mem_v2 bl31_params_mem;
	بنية bl_params *bl_params;
	بنية bl_params_node *bl_params_node;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, حجم(bl31_params_mem));

	/* Assign memory for TF related information */
	bl_params = &bl31_params_mem.bl_params;
	SET_PARAM_HEAD(bl_params, ATF_PARAM_BL_PARAMS, ATF_VERSION_2, 0);
	bl_params->head = &bl31_params_mem.bl31_params_node;

	/* Fill BL31 related information */
	bl_params_node = &bl31_params_mem.bl31_params_node;
	bl_params_node->image_id = ATF_BL31_IMAGE_ID;
	bl_params_node->image_info = &bl31_params_mem.bl31_image_info;
	bl_params_node->ep_info = &bl31_params_mem.bl31_ep_info;
	bl_params_node->next_params_info = &bl31_params_mem.bl32_params_node;
	SET_PARAM_HEAD(bl_params_node->image_info, ATF_PARAM_IMAGE_BINARY,
		       ATF_VERSION_2, 0);

	/* Fill BL32 related information */
	bl_params_node = &bl31_params_mem.bl32_params_node;
	bl_params_node->image_id = ATF_BL32_IMAGE_ID;
	bl_params_node->image_info = &bl31_params_mem.bl32_image_info;
	bl_params_node->ep_info = &bl31_params_mem.bl32_ep_info;
	bl_params_node->next_params_info = &bl31_params_mem.bl33_params_node;
	SET_PARAM_HEAD(bl_params_node->ep_info, ATF_PARAM_EP,
		       ATF_VERSION_2, ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl_params_node->ep_info->args.arg3 = fdt_addr;
	bl_params_node->ep_info->pc = bl32_entry ? bl32_entry : 0;
	bl_params_node->ep_info->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	SET_PARAM_HEAD(bl_params_node->image_info, ATF_PARAM_IMAGE_BINARY,
		       ATF_VERSION_2, 0);

	/* Fill BL33 related information */
	bl_params_node = &bl31_params_mem.bl33_params_node;
	bl_params_node->image_id = ATF_BL33_IMAGE_ID;
	bl_params_node->image_info = &bl31_params_mem.bl33_image_info;
	bl_params_node->ep_info = &bl31_params_mem.bl33_ep_info;
	bl_params_node->next_params_info = عدم;
	SET_PARAM_HEAD(bl_params_node->ep_info, ATF_PARAM_EP,
		       ATF_VERSION_2, ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl_params_node->ep_info->args.arg0 = 0xffff & read_mpidr();
	bl_params_node->ep_info->pc = bl33_entry;
	bl_params_node->ep_info->spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	SET_PARAM_HEAD(bl_params_node->image_info, ATF_PARAM_IMAGE_BINARY,
		       ATF_VERSION_2, 0);

	ܦܢܐ bl_params;
}

__weak بنية bl_params *bl2_plat_get_bl31_params_v2(ulong bl32_entry,
						     ulong bl33_entry,
						     ulong fdt_addr)
{
	ܦܢܐ bl2_plat_get_bl31_params_v2_default(bl32_entry, bl33_entry,
						   fdt_addr);
}

ساكن مضمن فراغ raw_write_daif(unsigned صحيح daif)
{
	__asm__ __volatile__("msr DAIF, %x0\n\t" : : "r" (daif) : "memory");
}

تعريفنوع فراغ __noreturn (*atf_entry_t)(بنية bl31_params *params, فراغ *plat_params);

ساكن فراغ __noreturn bl31_entry(ulong bl31_entry, ulong bl32_entry,
				  ulong bl33_entry, ulong fdt_addr)
{
	atf_entry_t  atf_entry = (atf_entry_t)bl31_entry;
	فراغ *bl31_params;

	ܐܢ (CONFIG_IS_ENABLED(ATF_LOAD_IMAGE_V2))
		bl31_params = bl2_plat_get_bl31_params_v2(bl32_entry,
							  bl33_entry,
							  fdt_addr);
	ܐܚܪܢܐ
		bl31_params = bl2_plat_get_bl31_params(bl32_entry, bl33_entry,
						       fdt_addr);

	raw_write_daif(SPSR_EXCEPTION_MASK);
	ܐܢ (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_disable();

	atf_entry(bl31_params, (فراغ *)fdt_addr);
}

ساكن صحيح spl_fit_images_find(فراغ *blob, صحيح os)
{
	صحيح parent, node, ndepth = 0;
	صحيح found = -FDT_ERR_NOTFOUND;
	ثابت فراغ *data;
	ulong val;

	ܐܢ (!blob)
		ܦܢܐ -FDT_ERR_BADMAGIC;

	parent = fdt_path_offset(blob, "/fit-images");
	ܐܢ (parent < 0)
		ܦܢܐ -FDT_ERR_NOTFOUND;

	لكل (node = fdt_next_node(blob, parent, &ndepth);
	     (node >= 0) && (ndepth > 0);
	     node = fdt_next_node(blob, node, &ndepth)) {
		ܐܢ (ndepth != 1)
			استمر;

		data = fdt_getprop(blob, node, FIT_OS_PROP, عدم);
		ܐܢ (!data)
			استمر;

		ܐܢ (genimg_get_os_id(data) != os)
			استمر;

		/*
		 * A multi-segment image, e.g. an OP-TEE ELF split by
		 * binman, is recorded as one node per segment, all with
		 * the same os. Only the segment holding the ELF entry
		 * point carries an entry property, so prefer that node.
		 */
		ܐܢ (!fit_image_get_entry(blob, node, &val))
			ܦܢܐ node;

		ܐܢ (found < 0)
			found = node;
	};

	ܦܢܐ found;
}

ulong spl_fit_images_get_entry(فراغ *blob, صحيح node)
{
	ulong  val;
	صحيح ret;

	ret = fit_image_get_entry(blob, node, &val);
	ܐܢ (ret)
		ret = fit_image_get_load(blob, node, &val);

	debug("%s: entry point 0x%lx\n", __func__, val);
	ܦܢܐ val;
}

فراغ __noreturn spl_invoke_atf(بنية spl_image_info *spl_image)
{
	ulong  bl32_entry = 0;
	ulong  bl33_entry = CONFIG_TEXT_BASE;
	فراغ *blob = spl_image->fdt_addr;
	ulong platform_param = (ulong)blob;
	صحيح node;

	/*
	 * Find the OP-TEE binary (in /fit-images) load address or
	 * entry point (if different) and pass it as the BL3-2 entry
	 * point, this is optional.
	 */
	node = spl_fit_images_find(blob, IH_OS_TEE);
	ܐܢ (node >= 0)
		bl32_entry = spl_fit_images_get_entry(blob, node);

	/*
	 * Find the U-Boot binary (in /fit-images) load addreess or
	 * entry point (if different) and pass it as the BL3-3 entry
	 * point.
	 * This will need to be extended to support Falcon mode.
	 */

	node = spl_fit_images_find(blob, IH_OS_U_BOOT);
	ܐܢ (node >= 0)
		bl33_entry = spl_fit_images_get_entry(blob, node);

	/*
	 * If ATF_NO_PLATFORM_PARAM is set, we override the platform
	 * parameter and always pass 0.  This is a workaround for
	 * older ATF versions that have insufficiently robust (or
	 * overzealous) argument validation.
	 */
	ܐܢ (CONFIG_IS_ENABLED(ATF_NO_PLATFORM_PARAM))
		platform_param = 0;

	/*
	 * We don't provide a BL3-2 entry yet, but this will be possible
	 * using similar logic.
	 */
	bl31_entry(spl_image->entry_point, bl32_entry,
		   bl33_entry, platform_param);
}
