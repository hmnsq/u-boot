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
بنية bl2_منزل_bl31_params_mem {
	بنية bl31_params bl31_params;
	بنية atf_image_info bl31_image_info;
	بنية atf_image_info bl32_image_info;
	بنية atf_image_info bl33_image_info;
	بنية ثبت_ټکی_info bl33_ep_info;
	بنية ثبت_ټکی_info bl32_ep_info;
	بنية ثبت_ټکی_info bl31_ep_info;
};

بنية bl2_منزل_bl31_params_mem_v2 {
	بنية bl_params bl_params;
	بنية bl_params_غوټه bl31_params_غوټه;
	بنية bl_params_غوټه bl32_params_غوټه;
	بنية bl_params_غوټه bl33_params_غوټه;
	بنية atf_image_info bl31_image_info;
	بنية atf_image_info bl32_image_info;
	بنية atf_image_info bl33_image_info;
	بنية ثبت_ټکی_info bl33_ep_info;
	بنية ثبت_ټکی_info bl32_ep_info;
	بنية ثبت_ټکی_info bl31_ep_info;
};

بنية bl31_params *bl2_plat_get_bl31_params_default(ulong bl32_ثبت,
						     ulong bl33_ثبت,
						     ulong fdt_addr)
{
	ساكن بنية bl2_منزل_bl31_params_mem bl31_params_mem;
	بنية bl31_params *bl2_منزل_bl31_params;
	بنية ثبت_ټکی_info *bl32_ep_info;
	بنية ثبت_ټکی_info *bl33_ep_info;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, حجم(بنية bl2_منزل_bl31_params_mem));

	/* Assign memory for TF related information */
	bl2_منزل_bl31_params = &bl31_params_mem.bl31_params;
	سېټ_PARAM_سر(bl2_منزل_bl31_params, ATF_PARAM_BL31, ATF_نسخه_1, 0);

	/* Fill BL31 related information */
	bl2_منزل_bl31_params->bl31_image_info = &bl31_params_mem.bl31_image_info;
	سېټ_PARAM_سر(bl2_منزل_bl31_params->bl31_image_info,
		       ATF_PARAM_IMAGE_دوګونی, ATF_نسخه_1, 0);

	/* Fill BL32 related information */
	bl2_منزل_bl31_params->bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	سېټ_PARAM_سر(bl32_ep_info, ATF_PARAM_EP, ATF_نسخه_1,
		       ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl32_ep_info->دلیلونه.arg3 = fdt_addr;
	bl32_ep_info->pc = bl32_ثبت ? bl32_ثبت : 0;
	bl32_ep_info->spsr = SPSR_64(ډېرپېښ_EL1, ډېرپېښ_SP_ELX,
				     DISABLE_ALL_EXCEPTIONS);

	bl2_منزل_bl31_params->bl32_image_info = &bl31_params_mem.bl32_image_info;
	سېټ_PARAM_سر(bl2_منزل_bl31_params->bl32_image_info,
		       ATF_PARAM_IMAGE_دوګونی, ATF_نسخه_1, 0);

	/* Fill BL33 related information */
	bl2_منزل_bl31_params->bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	سېټ_PARAM_سر(bl33_ep_info, ATF_PARAM_EP, ATF_نسخه_1,
		       ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl33_ep_info->دلیلونه.arg0 = 0xffff & read_mpidr();
	bl33_ep_info->pc = bl33_ثبت;
	bl33_ep_info->spsr = SPSR_64(ډېرپېښ_EL2, ډېرپېښ_SP_ELX,
				     DISABLE_ALL_EXCEPTIONS);

	bl2_منزل_bl31_params->bl33_image_info = &bl31_params_mem.bl33_image_info;
	سېټ_PARAM_سر(bl2_منزل_bl31_params->bl33_image_info,
		       ATF_PARAM_IMAGE_دوګونی, ATF_نسخه_1, 0);

	أرجع bl2_منزل_bl31_params;
}

__weak بنية bl31_params *bl2_plat_get_bl31_params(ulong bl32_ثبت,
						    ulong bl33_ثبت,
						    ulong fdt_addr)
{
	أرجع bl2_plat_get_bl31_params_default(bl32_ثبت, bl33_ثبت,
						fdt_addr);
}

بنية bl_params *bl2_plat_get_bl31_params_v2_default(ulong bl32_ثبت,
						      ulong bl33_ثبت,
						      ulong fdt_addr)
{
	ساكن بنية bl2_منزل_bl31_params_mem_v2 bl31_params_mem;
	بنية bl_params *bl_params;
	بنية bl_params_غوټه *bl_params_غوټه;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, حجم(bl31_params_mem));

	/* Assign memory for TF related information */
	bl_params = &bl31_params_mem.bl_params;
	سېټ_PARAM_سر(bl_params, ATF_PARAM_BL_PARAMS, ATF_نسخه_2, 0);
	bl_params->سر_2 = &bl31_params_mem.bl31_params_غوټه;

	/* Fill BL31 related information */
	bl_params_غوټه = &bl31_params_mem.bl31_params_غوټه;
	bl_params_غوټه->image_id = ATF_BL31_IMAGE_ID;
	bl_params_غوټه->image_info = &bl31_params_mem.bl31_image_info;
	bl_params_غوټه->ep_info = &bl31_params_mem.bl31_ep_info;
	bl_params_غوټه->بل_params_info = &bl31_params_mem.bl32_params_غوټه;
	سېټ_PARAM_سر(bl_params_غوټه->image_info, ATF_PARAM_IMAGE_دوګونی,
		       ATF_نسخه_2, 0);

	/* Fill BL32 related information */
	bl_params_غوټه = &bl31_params_mem.bl32_params_غوټه;
	bl_params_غوټه->image_id = ATF_BL32_IMAGE_ID;
	bl_params_غوټه->image_info = &bl31_params_mem.bl32_image_info;
	bl_params_غوټه->ep_info = &bl31_params_mem.bl32_ep_info;
	bl_params_غوټه->بل_params_info = &bl31_params_mem.bl33_params_غوټه;
	سېټ_PARAM_سر(bl_params_غوټه->ep_info, ATF_PARAM_EP,
		       ATF_نسخه_2, ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl_params_غوټه->ep_info->دلیلونه.arg3 = fdt_addr;
	bl_params_غوټه->ep_info->pc = bl32_ثبت ? bl32_ثبت : 0;
	bl_params_غوټه->ep_info->spsr = SPSR_64(ډېرپېښ_EL1, ډېرپېښ_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	سېټ_PARAM_سر(bl_params_غوټه->image_info, ATF_PARAM_IMAGE_دوګونی,
		       ATF_نسخه_2, 0);

	/* Fill BL33 related information */
	bl_params_غوټه = &bl31_params_mem.bl33_params_غوټه;
	bl_params_غوټه->image_id = ATF_BL33_IMAGE_ID;
	bl_params_غوټه->image_info = &bl31_params_mem.bl33_image_info;
	bl_params_غوټه->ep_info = &bl31_params_mem.bl33_ep_info;
	bl_params_غوټه->بل_params_info = عدم;
	سېټ_PARAM_سر(bl_params_غوټه->ep_info, ATF_PARAM_EP,
		       ATF_نسخه_2, ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl_params_غوټه->ep_info->دلیلونه.arg0 = 0xffff & read_mpidr();
	bl_params_غوټه->ep_info->pc = bl33_ثبت;
	bl_params_غوټه->ep_info->spsr = SPSR_64(ډېرپېښ_EL2, ډېرپېښ_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	سېټ_PARAM_سر(bl_params_غوټه->image_info, ATF_PARAM_IMAGE_دوګونی,
		       ATF_نسخه_2, 0);

	أرجع bl_params;
}

__weak بنية bl_params *bl2_plat_get_bl31_params_v2(ulong bl32_ثبت,
						     ulong bl33_ثبت,
						     ulong fdt_addr)
{
	أرجع bl2_plat_get_bl31_params_v2_default(bl32_ثبت, bl33_ثبت,
						   fdt_addr);
}

ساكن مضمن فراغ raw_write_daif(بینښه صحيح daif)
{
	__asm__ __volatile__("msr DAIF, %x0\n\t" : : "r" (daif) : "memory");
}

تعريفنوع فراغ __noreturn (*atf_ثبت_t)(بنية bl31_params *params, فراغ *plat_params);

ساكن فراغ __noreturn bl31_ثبت(ulong bl31_ثبت, ulong bl32_ثبت,
				  ulong bl33_ثبت, ulong fdt_addr)
{
	atf_ثبت_t  atf_ثبت = (atf_ثبت_t)bl31_ثبت;
	فراغ *bl31_params;

	إذا (CONFIG_IS_ENABLED(ATF_بارول_IMAGE_V2))
		bl31_params = bl2_plat_get_bl31_params_v2(bl32_ثبت,
							  bl33_ثبت,
							  fdt_addr);
	وإلا
		bl31_params = bl2_plat_get_bl31_params(bl32_ثبت, bl33_ثبت,
						       fdt_addr);

	raw_write_daif(SPSR_EXCEPTION_ماسک);
	إذا (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_disable();

	atf_ثبت(bl31_params, (فراغ *)fdt_addr);
}

ساكن صحيح spl_fit_images_پیدا_کړه(فراغ *blob, صحيح os)
{
	صحيح پلار, غوټه, ndepth = 0;
	صحيح وموندل = -FDT_ERR_NOTFOUND;
	ثابت فراغ *معلومات;
	ulong val;

	إذا (!blob)
		أرجع -FDT_ERR_BADMAGIC;

	پلار = fdt_لار_دننه_واټن(blob, "/fit-images");
	إذا (پلار < 0)
		أرجع -FDT_ERR_NOTFOUND;

	لكل (غوټه = fdt_بل_غوټه(blob, پلار, &ndepth);
	     (غوټه >= 0) && (ndepth > 0);
	     غوټه = fdt_بل_غوټه(blob, غوټه, &ndepth)) {
		إذا (ndepth != 1)
			استمر;

		معلومات = fdt_getprop(blob, غوټه, FIT_OS_PROP, عدم);
		إذا (!معلومات)
			استمر;

		إذا (genimg_get_os_id(معلومات) != os)
			استمر;

		/*
		 * A multi-segment image, e.g. an OP-TEE ELF split by
		 * binman, is recorded as one node per segment, all with
		 * the same os. Only the segment holding the ELF entry
		 * point carries an entry property, so prefer that node.
		 */
		إذا (!fit_image_get_ثبت(blob, غوټه, &val))
			أرجع غوټه;

		إذا (وموندل < 0)
			وموندل = غوټه;
	};

	أرجع وموندل;
}

ulong spl_fit_images_get_ثبت(فراغ *blob, صحيح غوټه)
{
	ulong  val;
	صحيح ret;

	ret = fit_image_get_ثبت(blob, غوټه, &val);
	إذا (ret)
		ret = fit_image_get_بارول(blob, غوټه, &val);

	debug("%s: entry point 0x%lx\n", __func__, val);
	أرجع val;
}

فراغ __noreturn spl_invoke_atf(بنية spl_image_info *spl_image)
{
	ulong  bl32_ثبت = 0;
	ulong  bl33_ثبت = CONFIG_متن_بنسټ;
	فراغ *blob = spl_image->fdt_addr;
	ulong platform_param = (ulong)blob;
	صحيح غوټه;

	/*
	 * Find the OP-TEE binary (in /fit-images) load address or
	 * entry point (if different) and pass it as the BL3-2 entry
	 * point, this is optional.
	 */
	غوټه = spl_fit_images_پیدا_کړه(blob, IH_OS_TEE);
	إذا (غوټه >= 0)
		bl32_ثبت = spl_fit_images_get_ثبت(blob, غوټه);

	/*
	 * Find the U-Boot binary (in /fit-images) load addreess or
	 * entry point (if different) and pass it as the BL3-3 entry
	 * point.
	 * This will need to be extended to support Falcon mode.
	 */

	غوټه = spl_fit_images_پیدا_کړه(blob, IH_OS_U_BOOT);
	إذا (غوټه >= 0)
		bl33_ثبت = spl_fit_images_get_ثبت(blob, غوټه);

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
	bl31_ثبت(spl_image->ثبت_ټکی, bl32_ثبت,
		   bl33_ثبت, platform_param);
}
