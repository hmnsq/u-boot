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

#شامل <atf_common.h>
#شامل <cpu_func.h>
#شامل <errno.h>
#شامل <image.h>
#شامل <log.h>
#شامل <spl.h>
#شامل <asm/cache.h>

/* Holds all the structures we need for bl31 parameter passing */
ساختار bl2_to_bl31_params_mem {
	ساختار bl31_params bl31_params;
	ساختار atf_image_info bl31_image_info;
	ساختار atf_image_info bl32_image_info;
	ساختار atf_image_info bl33_image_info;
	ساختار entry_point_info bl33_ep_info;
	ساختار entry_point_info bl32_ep_info;
	ساختار entry_point_info bl31_ep_info;
};

ساختار bl2_to_bl31_params_mem_v2 {
	ساختار bl_params bl_params;
	ساختار bl_params_گره bl31_params_گره;
	ساختار bl_params_گره bl32_params_گره;
	ساختار bl_params_گره bl33_params_گره;
	ساختار atf_image_info bl31_image_info;
	ساختار atf_image_info bl32_image_info;
	ساختار atf_image_info bl33_image_info;
	ساختار entry_point_info bl33_ep_info;
	ساختار entry_point_info bl32_ep_info;
	ساختار entry_point_info bl31_ep_info;
};

ساختار bl31_params *bl2_plat_get_bl31_params_default(ulong bl32_entry,
						     ulong bl33_entry,
						     ulong fdt_addr)
{
	ایستا ساختار bl2_to_bl31_params_mem bl31_params_mem;
	ساختار bl31_params *bl2_to_bl31_params;
	ساختار entry_point_info *bl32_ep_info;
	ساختار entry_point_info *bl33_ep_info;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, اندازه(ساختار bl2_to_bl31_params_mem));

	/* Assign memory for TF related information */
	bl2_to_bl31_params = &bl31_params_mem.bl31_params;
	SET_PARAM_HEAD(bl2_to_bl31_params, ATF_PARAM_BL31, ATF_VERSION_1, 0);

	/* Fill BL31 related information */
	bl2_to_bl31_params->bl31_image_info = &bl31_params_mem.bl31_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl31_image_info,
		       ATF_PARAM_IMAGE_دودویی, ATF_VERSION_1, 0);

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
		       ATF_PARAM_IMAGE_دودویی, ATF_VERSION_1, 0);

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
		       ATF_PARAM_IMAGE_دودویی, ATF_VERSION_1, 0);

	بازگشت bl2_to_bl31_params;
}

__weak ساختار bl31_params *bl2_plat_get_bl31_params(ulong bl32_entry,
						    ulong bl33_entry,
						    ulong fdt_addr)
{
	بازگشت bl2_plat_get_bl31_params_default(bl32_entry, bl33_entry,
						fdt_addr);
}

ساختار bl_params *bl2_plat_get_bl31_params_v2_default(ulong bl32_entry,
						      ulong bl33_entry,
						      ulong fdt_addr)
{
	ایستا ساختار bl2_to_bl31_params_mem_v2 bl31_params_mem;
	ساختار bl_params *bl_params;
	ساختار bl_params_گره *bl_params_گره;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, اندازه(bl31_params_mem));

	/* Assign memory for TF related information */
	bl_params = &bl31_params_mem.bl_params;
	SET_PARAM_HEAD(bl_params, ATF_PARAM_BL_PARAMS, ATF_VERSION_2, 0);
	bl_params->head = &bl31_params_mem.bl31_params_گره;

	/* Fill BL31 related information */
	bl_params_گره = &bl31_params_mem.bl31_params_گره;
	bl_params_گره->image_id = ATF_BL31_IMAGE_ID;
	bl_params_گره->image_info = &bl31_params_mem.bl31_image_info;
	bl_params_گره->ep_info = &bl31_params_mem.bl31_ep_info;
	bl_params_گره->next_params_info = &bl31_params_mem.bl32_params_گره;
	SET_PARAM_HEAD(bl_params_گره->image_info, ATF_PARAM_IMAGE_دودویی,
		       ATF_VERSION_2, 0);

	/* Fill BL32 related information */
	bl_params_گره = &bl31_params_mem.bl32_params_گره;
	bl_params_گره->image_id = ATF_BL32_IMAGE_ID;
	bl_params_گره->image_info = &bl31_params_mem.bl32_image_info;
	bl_params_گره->ep_info = &bl31_params_mem.bl32_ep_info;
	bl_params_گره->next_params_info = &bl31_params_mem.bl33_params_گره;
	SET_PARAM_HEAD(bl_params_گره->ep_info, ATF_PARAM_EP,
		       ATF_VERSION_2, ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl_params_گره->ep_info->args.arg3 = fdt_addr;
	bl_params_گره->ep_info->pc = bl32_entry ? bl32_entry : 0;
	bl_params_گره->ep_info->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	SET_PARAM_HEAD(bl_params_گره->image_info, ATF_PARAM_IMAGE_دودویی,
		       ATF_VERSION_2, 0);

	/* Fill BL33 related information */
	bl_params_گره = &bl31_params_mem.bl33_params_گره;
	bl_params_گره->image_id = ATF_BL33_IMAGE_ID;
	bl_params_گره->image_info = &bl31_params_mem.bl33_image_info;
	bl_params_گره->ep_info = &bl31_params_mem.bl33_ep_info;
	bl_params_گره->next_params_info = پوچ;
	SET_PARAM_HEAD(bl_params_گره->ep_info, ATF_PARAM_EP,
		       ATF_VERSION_2, ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl_params_گره->ep_info->args.arg0 = 0xffff & read_mpidr();
	bl_params_گره->ep_info->pc = bl33_entry;
	bl_params_گره->ep_info->spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	SET_PARAM_HEAD(bl_params_گره->image_info, ATF_PARAM_IMAGE_دودویی,
		       ATF_VERSION_2, 0);

	بازگشت bl_params;
}

__weak ساختار bl_params *bl2_plat_get_bl31_params_v2(ulong bl32_entry,
						     ulong bl33_entry,
						     ulong fdt_addr)
{
	بازگشت bl2_plat_get_bl31_params_v2_default(bl32_entry, bl33_entry,
						   fdt_addr);
}

ایستا درونخطی تهی raw_write_daif(unsigned صحیح daif)
{
	__asm__ __volatile__("msr DAIF, %x0\n\t" : : "r" (daif) : "memory");
}

تعریفنوع تهی __noreturn (*atf_entry_t)(ساختار bl31_params *params, تهی *plat_params);

ایستا تهی __noreturn bl31_entry(ulong bl31_entry, ulong bl32_entry,
				  ulong bl33_entry, ulong fdt_addr)
{
	atf_entry_t  atf_entry = (atf_entry_t)bl31_entry;
	تهی *bl31_params;

	اگر (CONFIG_IS_ENABLED(ATF_LOAD_IMAGE_V2))
		bl31_params = bl2_plat_get_bl31_params_v2(bl32_entry,
							  bl33_entry,
							  fdt_addr);
	وگرنه
		bl31_params = bl2_plat_get_bl31_params(bl32_entry, bl33_entry,
						       fdt_addr);

	raw_write_daif(SPSR_EXCEPTION_MASK);
	اگر (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_disable();

	atf_entry(bl31_params, (تهی *)fdt_addr);
}

ایستا صحیح spl_fit_images_find(تهی *blob, صحیح os)
{
	صحیح parent, گره, ndepth = 0;
	صحیح found = -FDT_ERR_NOTFOUND;
	ثابت تهی *data;
	ulong val;

	اگر (!blob)
		بازگشت -FDT_ERR_BADMAGIC;

	parent = fdt_path_offset(blob, "/fit-images");
	اگر (parent < 0)
		بازگشت -FDT_ERR_NOTFOUND;

	برای (گره = fdt_next_گره(blob, parent, &ndepth);
	     (گره >= 0) && (ndepth > 0);
	     گره = fdt_next_گره(blob, گره, &ndepth)) {
		اگر (ndepth != 1)
			ادامه;

		data = fdt_getprop(blob, گره, FIT_OS_PROP, پوچ);
		اگر (!data)
			ادامه;

		اگر (genimg_get_os_id(data) != os)
			ادامه;

		/*
		 * A multi-segment image, e.g. an OP-TEE ELF split by
		 * binman, is recorded as one node per segment, all with
		 * the same os. Only the segment holding the ELF entry
		 * point carries an entry property, so prefer that node.
		 */
		اگر (!fit_image_get_entry(blob, گره, &val))
			بازگشت گره;

		اگر (found < 0)
			found = گره;
	};

	بازگشت found;
}

ulong spl_fit_images_get_entry(تهی *blob, صحیح گره)
{
	ulong  val;
	صحیح ret;

	ret = fit_image_get_entry(blob, گره, &val);
	اگر (ret)
		ret = fit_image_get_load(blob, گره, &val);

	debug("%s: entry point 0x%lx\n", __func__, val);
	بازگشت val;
}

تهی __noreturn spl_invoke_atf(ساختار spl_image_info *spl_image)
{
	ulong  bl32_entry = 0;
	ulong  bl33_entry = CONFIG_TEXT_BASE;
	تهی *blob = spl_image->fdt_addr;
	ulong platform_param = (ulong)blob;
	صحیح گره;

	/*
	 * Find the OP-TEE binary (in /fit-images) load address or
	 * entry point (if different) and pass it as the BL3-2 entry
	 * point, this is optional.
	 */
	گره = spl_fit_images_find(blob, IH_OS_TEE);
	اگر (گره >= 0)
		bl32_entry = spl_fit_images_get_entry(blob, گره);

	/*
	 * Find the U-Boot binary (in /fit-images) load addreess or
	 * entry point (if different) and pass it as the BL3-3 entry
	 * point.
	 * This will need to be extended to support Falcon mode.
	 */

	گره = spl_fit_images_find(blob, IH_OS_U_BOOT);
	اگر (گره >= 0)
		bl33_entry = spl_fit_images_get_entry(blob, گره);

	/*
	 * If ATF_NO_PLATFORM_PARAM is set, we override the platform
	 * parameter and always pass 0.  This is a workaround for
	 * older ATF versions that have insufficiently robust (or
	 * overzealous) argument validation.
	 */
	اگر (CONFIG_IS_ENABLED(ATF_NO_PLATFORM_PARAM))
		platform_param = 0;

	/*
	 * We don't provide a BL3-2 entry yet, but this will be possible
	 * using similar logic.
	 */
	bl31_entry(spl_image->entry_point, bl32_entry,
		   bl33_entry, platform_param);
}
