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
بنية bl2_مقصد_bl31_params_mem {
	بنية bl31_params bl31_params;
	بنية atf_image_info bl31_image_info;
	بنية atf_image_info bl32_image_info;
	بنية atf_image_info bl33_image_info;
	بنية مدخل_نقطه_info bl33_ep_info;
	بنية مدخل_نقطه_info bl32_ep_info;
	بنية مدخل_نقطه_info bl31_ep_info;
};

بنية bl2_مقصد_bl31_params_mem_v2 {
	بنية bl_params bl_params;
	بنية bl_params_گره bl31_params_گره;
	بنية bl_params_گره bl32_params_گره;
	بنية bl_params_گره bl33_params_گره;
	بنية atf_image_info bl31_image_info;
	بنية atf_image_info bl32_image_info;
	بنية atf_image_info bl33_image_info;
	بنية مدخل_نقطه_info bl33_ep_info;
	بنية مدخل_نقطه_info bl32_ep_info;
	بنية مدخل_نقطه_info bl31_ep_info;
};

بنية bl31_params *bl2_plat_get_bl31_params_default(ulong bl32_مدخل,
						     ulong bl33_مدخل,
						     ulong fdt_addr)
{
	ساكن بنية bl2_مقصد_bl31_params_mem bl31_params_mem;
	بنية bl31_params *bl2_مقصد_bl31_params;
	بنية مدخل_نقطه_info *bl32_ep_info;
	بنية مدخل_نقطه_info *bl33_ep_info;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, حجم(بنية bl2_مقصد_bl31_params_mem));

	/* Assign memory for TF related information */
	bl2_مقصد_bl31_params = &bl31_params_mem.bl31_params;
	مجموعه_PARAM_سر(bl2_مقصد_bl31_params, ATF_PARAM_BL31, ATF_نسخه_1, 0);

	/* Fill BL31 related information */
	bl2_مقصد_bl31_params->bl31_image_info = &bl31_params_mem.bl31_image_info;
	مجموعه_PARAM_سر(bl2_مقصد_bl31_params->bl31_image_info,
		       ATF_PARAM_IMAGE_دودویی, ATF_نسخه_1, 0);

	/* Fill BL32 related information */
	bl2_مقصد_bl31_params->bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	مجموعه_PARAM_سر(bl32_ep_info, ATF_PARAM_EP, ATF_نسخه_1,
		       ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl32_ep_info->آرگومانها.arg3 = fdt_addr;
	bl32_ep_info->pc = bl32_مدخل ? bl32_مدخل : 0;
	bl32_ep_info->spsr = SPSR_64(نما_EL1, نما_SP_ELX,
				     DISABLE_ALL_EXCEPTIONS);

	bl2_مقصد_bl31_params->bl32_image_info = &bl31_params_mem.bl32_image_info;
	مجموعه_PARAM_سر(bl2_مقصد_bl31_params->bl32_image_info,
		       ATF_PARAM_IMAGE_دودویی, ATF_نسخه_1, 0);

	/* Fill BL33 related information */
	bl2_مقصد_bl31_params->bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	مجموعه_PARAM_سر(bl33_ep_info, ATF_PARAM_EP, ATF_نسخه_1,
		       ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl33_ep_info->آرگومانها.arg0 = 0xffff & read_mpidr();
	bl33_ep_info->pc = bl33_مدخل;
	bl33_ep_info->spsr = SPSR_64(نما_EL2, نما_SP_ELX,
				     DISABLE_ALL_EXCEPTIONS);

	bl2_مقصد_bl31_params->bl33_image_info = &bl31_params_mem.bl33_image_info;
	مجموعه_PARAM_سر(bl2_مقصد_bl31_params->bl33_image_info,
		       ATF_PARAM_IMAGE_دودویی, ATF_نسخه_1, 0);

	أرجع bl2_مقصد_bl31_params;
}

__weak بنية bl31_params *bl2_plat_get_bl31_params(ulong bl32_مدخل,
						    ulong bl33_مدخل,
						    ulong fdt_addr)
{
	أرجع bl2_plat_get_bl31_params_default(bl32_مدخل, bl33_مدخل,
						fdt_addr);
}

بنية bl_params *bl2_plat_get_bl31_params_v2_default(ulong bl32_مدخل,
						      ulong bl33_مدخل,
						      ulong fdt_addr)
{
	ساكن بنية bl2_مقصد_bl31_params_mem_v2 bl31_params_mem;
	بنية bl_params *bl_params;
	بنية bl_params_گره *bl_params_گره;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, حجم(bl31_params_mem));

	/* Assign memory for TF related information */
	bl_params = &bl31_params_mem.bl_params;
	مجموعه_PARAM_سر(bl_params, ATF_PARAM_BL_PARAMS, ATF_نسخه_2, 0);
	bl_params->سر = &bl31_params_mem.bl31_params_گره;

	/* Fill BL31 related information */
	bl_params_گره = &bl31_params_mem.bl31_params_گره;
	bl_params_گره->image_id = ATF_BL31_IMAGE_ID;
	bl_params_گره->image_info = &bl31_params_mem.bl31_image_info;
	bl_params_گره->ep_info = &bl31_params_mem.bl31_ep_info;
	bl_params_گره->بعدی_params_info = &bl31_params_mem.bl32_params_گره;
	مجموعه_PARAM_سر(bl_params_گره->image_info, ATF_PARAM_IMAGE_دودویی,
		       ATF_نسخه_2, 0);

	/* Fill BL32 related information */
	bl_params_گره = &bl31_params_mem.bl32_params_گره;
	bl_params_گره->image_id = ATF_BL32_IMAGE_ID;
	bl_params_گره->image_info = &bl31_params_mem.bl32_image_info;
	bl_params_گره->ep_info = &bl31_params_mem.bl32_ep_info;
	bl_params_گره->بعدی_params_info = &bl31_params_mem.bl33_params_گره;
	مجموعه_PARAM_سر(bl_params_گره->ep_info, ATF_PARAM_EP,
		       ATF_نسخه_2, ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl_params_گره->ep_info->آرگومانها.arg3 = fdt_addr;
	bl_params_گره->ep_info->pc = bl32_مدخل ? bl32_مدخل : 0;
	bl_params_گره->ep_info->spsr = SPSR_64(نما_EL1, نما_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	مجموعه_PARAM_سر(bl_params_گره->image_info, ATF_PARAM_IMAGE_دودویی,
		       ATF_نسخه_2, 0);

	/* Fill BL33 related information */
	bl_params_گره = &bl31_params_mem.bl33_params_گره;
	bl_params_گره->image_id = ATF_BL33_IMAGE_ID;
	bl_params_گره->image_info = &bl31_params_mem.bl33_image_info;
	bl_params_گره->ep_info = &bl31_params_mem.bl33_ep_info;
	bl_params_گره->بعدی_params_info = عدم;
	مجموعه_PARAM_سر(bl_params_گره->ep_info, ATF_PARAM_EP,
		       ATF_نسخه_2, ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl_params_گره->ep_info->آرگومانها.arg0 = 0xffff & read_mpidr();
	bl_params_گره->ep_info->pc = bl33_مدخل;
	bl_params_گره->ep_info->spsr = SPSR_64(نما_EL2, نما_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	مجموعه_PARAM_سر(bl_params_گره->image_info, ATF_PARAM_IMAGE_دودویی,
		       ATF_نسخه_2, 0);

	أرجع bl_params;
}

__weak بنية bl_params *bl2_plat_get_bl31_params_v2(ulong bl32_مدخل,
						     ulong bl33_مدخل,
						     ulong fdt_addr)
{
	أرجع bl2_plat_get_bl31_params_v2_default(bl32_مدخل, bl33_مدخل,
						   fdt_addr);
}

ساكن مضمن فراغ raw_write_daif(بیعلامت صحيح daif)
{
	__asm__ __volatile__("msr DAIF, %x0\n\t" : : "r" (daif) : "memory");
}

تعريفنوع فراغ __noreturn (*atf_مدخل_t)(بنية bl31_params *params, فراغ *plat_params);

ساكن فراغ __noreturn bl31_مدخل(ulong bl31_مدخل, ulong bl32_مدخل,
				  ulong bl33_مدخل, ulong fdt_addr)
{
	atf_مدخل_t  atf_مدخل = (atf_مدخل_t)bl31_مدخل;
	فراغ *bl31_params;

	إذا (CONFIG_IS_ENABLED(ATF_بارگیری_IMAGE_V2))
		bl31_params = bl2_plat_get_bl31_params_v2(bl32_مدخل,
							  bl33_مدخل,
							  fdt_addr);
	وإلا
		bl31_params = bl2_plat_get_bl31_params(bl32_مدخل, bl33_مدخل,
						       fdt_addr);

	raw_write_daif(SPSR_EXCEPTION_ماسک);
	إذا (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_disable();

	atf_مدخل(bl31_params, (فراغ *)fdt_addr);
}

ساكن صحيح spl_fit_images_پیدا_کن(فراغ *blob, صحيح os)
{
	صحيح والد, گره, ndepth = 0;
	صحيح یافت = -FDT_ERR_NOTFOUND;
	ثابت فراغ *داده;
	ulong val;

	إذا (!blob)
		أرجع -FDT_ERR_BADMAGIC;

	والد = fdt_مسیر_جابه_جایی(blob, "/fit-images");
	إذا (والد < 0)
		أرجع -FDT_ERR_NOTFOUND;

	لكل (گره = fdt_بعدی_گره(blob, والد, &ndepth);
	     (گره >= 0) && (ndepth > 0);
	     گره = fdt_بعدی_گره(blob, گره, &ndepth)) {
		إذا (ndepth != 1)
			استمر;

		داده = fdt_getprop(blob, گره, FIT_OS_PROP, عدم);
		إذا (!داده)
			استمر;

		إذا (genimg_get_os_id(داده) != os)
			استمر;

		/*
		 * A multi-segment image, e.g. an OP-TEE ELF split by
		 * binman, is recorded as one node per segment, all with
		 * the same os. Only the segment holding the ELF entry
		 * point carries an entry property, so prefer that node.
		 */
		إذا (!fit_image_get_مدخل(blob, گره, &val))
			أرجع گره;

		إذا (یافت < 0)
			یافت = گره;
	};

	أرجع یافت;
}

ulong spl_fit_images_get_مدخل(فراغ *blob, صحيح گره)
{
	ulong  val;
	صحيح ret;

	ret = fit_image_get_مدخل(blob, گره, &val);
	إذا (ret)
		ret = fit_image_get_بارگیری(blob, گره, &val);

	debug("%s: entry point 0x%lx\n", __func__, val);
	أرجع val;
}

فراغ __noreturn spl_invoke_atf(بنية spl_image_info *spl_image)
{
	ulong  bl32_مدخل = 0;
	ulong  bl33_مدخل = CONFIG_متن_پایه;
	فراغ *blob = spl_image->fdt_addr;
	ulong platform_param = (ulong)blob;
	صحيح گره;

	/*
	 * Find the OP-TEE binary (in /fit-images) load address or
	 * entry point (if different) and pass it as the BL3-2 entry
	 * point, this is optional.
	 */
	گره = spl_fit_images_پیدا_کن(blob, IH_OS_TEE);
	إذا (گره >= 0)
		bl32_مدخل = spl_fit_images_get_مدخل(blob, گره);

	/*
	 * Find the U-Boot binary (in /fit-images) load addreess or
	 * entry point (if different) and pass it as the BL3-3 entry
	 * point.
	 * This will need to be extended to support Falcon mode.
	 */

	گره = spl_fit_images_پیدا_کن(blob, IH_OS_U_BOOT);
	إذا (گره >= 0)
		bl33_مدخل = spl_fit_images_get_مدخل(blob, گره);

	/*
	 * If ATF_NO_PLATFORM_PARAM is set, we override the platform
	 * parameter and always pass 0.  This is a workaround for
	 * older ATF versions that have insufficiently robust (or
	 * overzealous) argument validation.
	 */
	إذا (CONFIG_IS_ENABLED(ATF_NO_PLATFORM_PARAM))
		platform_param = 0;

	/*
	 * We don't provide a BL3-2 entry yet, but this will be possible
	 * using similar logic.
	 */
	bl31_مدخل(spl_image->مدخل_نقطه, bl32_مدخل,
		   bl33_مدخل, platform_param);
}
