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
	بنية اندراج_نقطہ_info bl33_ep_info;
	بنية اندراج_نقطہ_info bl32_ep_info;
	بنية اندراج_نقطہ_info bl31_ep_info;
};

بنية bl2_منزل_bl31_params_mem_v2 {
	بنية bl_params bl_params;
	بنية bl_params_گرہ bl31_params_گرہ;
	بنية bl_params_گرہ bl32_params_گرہ;
	بنية bl_params_گرہ bl33_params_گرہ;
	بنية atf_image_info bl31_image_info;
	بنية atf_image_info bl32_image_info;
	بنية atf_image_info bl33_image_info;
	بنية اندراج_نقطہ_info bl33_ep_info;
	بنية اندراج_نقطہ_info bl32_ep_info;
	بنية اندراج_نقطہ_info bl31_ep_info;
};

بنية bl31_params *bl2_plat_get_bl31_params_default(ulong bl32_اندراج,
						     ulong bl33_اندراج,
						     ulong fdt_addr)
{
	ساكن بنية bl2_منزل_bl31_params_mem bl31_params_mem;
	بنية bl31_params *bl2_منزل_bl31_params;
	بنية اندراج_نقطہ_info *bl32_ep_info;
	بنية اندراج_نقطہ_info *bl33_ep_info;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, حجم(بنية bl2_منزل_bl31_params_mem));

	/* Assign memory for TF related information */
	bl2_منزل_bl31_params = &bl31_params_mem.bl31_params;
	سیٹ_PARAM_سر(bl2_منزل_bl31_params, ATF_PARAM_BL31, ATF_نسخہ_1, 0);

	/* Fill BL31 related information */
	bl2_منزل_bl31_params->bl31_image_info = &bl31_params_mem.bl31_image_info;
	سیٹ_PARAM_سر(bl2_منزل_bl31_params->bl31_image_info,
		       ATF_PARAM_IMAGE_ثنائی, ATF_نسخہ_1, 0);

	/* Fill BL32 related information */
	bl2_منزل_bl31_params->bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	سیٹ_PARAM_سر(bl32_ep_info, ATF_PARAM_EP, ATF_نسخہ_1,
		       ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl32_ep_info->دلیلاں.arg3 = fdt_addr;
	bl32_ep_info->pc = bl32_اندراج ? bl32_اندراج : 0;
	bl32_ep_info->spsr = SPSR_64(منوال_EL1, منوال_SP_ELX,
				     DISABLE_ALL_EXCEPTIONS);

	bl2_منزل_bl31_params->bl32_image_info = &bl31_params_mem.bl32_image_info;
	سیٹ_PARAM_سر(bl2_منزل_bl31_params->bl32_image_info,
		       ATF_PARAM_IMAGE_ثنائی, ATF_نسخہ_1, 0);

	/* Fill BL33 related information */
	bl2_منزل_bl31_params->bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	سیٹ_PARAM_سر(bl33_ep_info, ATF_PARAM_EP, ATF_نسخہ_1,
		       ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl33_ep_info->دلیلاں.arg0 = 0xffff & read_mpidr();
	bl33_ep_info->pc = bl33_اندراج;
	bl33_ep_info->spsr = SPSR_64(منوال_EL2, منوال_SP_ELX,
				     DISABLE_ALL_EXCEPTIONS);

	bl2_منزل_bl31_params->bl33_image_info = &bl31_params_mem.bl33_image_info;
	سیٹ_PARAM_سر(bl2_منزل_bl31_params->bl33_image_info,
		       ATF_PARAM_IMAGE_ثنائی, ATF_نسخہ_1, 0);

	أرجع bl2_منزل_bl31_params;
}

__weak بنية bl31_params *bl2_plat_get_bl31_params(ulong bl32_اندراج,
						    ulong bl33_اندراج,
						    ulong fdt_addr)
{
	أرجع bl2_plat_get_bl31_params_default(bl32_اندراج, bl33_اندراج,
						fdt_addr);
}

بنية bl_params *bl2_plat_get_bl31_params_v2_default(ulong bl32_اندراج,
						      ulong bl33_اندراج,
						      ulong fdt_addr)
{
	ساكن بنية bl2_منزل_bl31_params_mem_v2 bl31_params_mem;
	بنية bl_params *bl_params;
	بنية bl_params_گرہ *bl_params_گرہ;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, حجم(bl31_params_mem));

	/* Assign memory for TF related information */
	bl_params = &bl31_params_mem.bl_params;
	سیٹ_PARAM_سر(bl_params, ATF_PARAM_BL_PARAMS, ATF_نسخہ_2, 0);
	bl_params->سر = &bl31_params_mem.bl31_params_گرہ;

	/* Fill BL31 related information */
	bl_params_گرہ = &bl31_params_mem.bl31_params_گرہ;
	bl_params_گرہ->image_id = ATF_BL31_IMAGE_ID;
	bl_params_گرہ->image_info = &bl31_params_mem.bl31_image_info;
	bl_params_گرہ->ep_info = &bl31_params_mem.bl31_ep_info;
	bl_params_گرہ->اگلا_params_info = &bl31_params_mem.bl32_params_گرہ;
	سیٹ_PARAM_سر(bl_params_گرہ->image_info, ATF_PARAM_IMAGE_ثنائی,
		       ATF_نسخہ_2, 0);

	/* Fill BL32 related information */
	bl_params_گرہ = &bl31_params_mem.bl32_params_گرہ;
	bl_params_گرہ->image_id = ATF_BL32_IMAGE_ID;
	bl_params_گرہ->image_info = &bl31_params_mem.bl32_image_info;
	bl_params_گرہ->ep_info = &bl31_params_mem.bl32_ep_info;
	bl_params_گرہ->اگلا_params_info = &bl31_params_mem.bl33_params_گرہ;
	سیٹ_PARAM_سر(bl_params_گرہ->ep_info, ATF_PARAM_EP,
		       ATF_نسخہ_2, ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl_params_گرہ->ep_info->دلیلاں.arg3 = fdt_addr;
	bl_params_گرہ->ep_info->pc = bl32_اندراج ? bl32_اندراج : 0;
	bl_params_گرہ->ep_info->spsr = SPSR_64(منوال_EL1, منوال_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	سیٹ_PARAM_سر(bl_params_گرہ->image_info, ATF_PARAM_IMAGE_ثنائی,
		       ATF_نسخہ_2, 0);

	/* Fill BL33 related information */
	bl_params_گرہ = &bl31_params_mem.bl33_params_گرہ;
	bl_params_گرہ->image_id = ATF_BL33_IMAGE_ID;
	bl_params_گرہ->image_info = &bl31_params_mem.bl33_image_info;
	bl_params_گرہ->ep_info = &bl31_params_mem.bl33_ep_info;
	bl_params_گرہ->اگلا_params_info = عدم;
	سیٹ_PARAM_سر(bl_params_گرہ->ep_info, ATF_PARAM_EP,
		       ATF_نسخہ_2, ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl_params_گرہ->ep_info->دلیلاں.arg0 = 0xffff & read_mpidr();
	bl_params_گرہ->ep_info->pc = bl33_اندراج;
	bl_params_گرہ->ep_info->spsr = SPSR_64(منوال_EL2, منوال_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	سیٹ_PARAM_سر(bl_params_گرہ->image_info, ATF_PARAM_IMAGE_ثنائی,
		       ATF_نسخہ_2, 0);

	أرجع bl_params;
}

__weak بنية bl_params *bl2_plat_get_bl31_params_v2(ulong bl32_اندراج,
						     ulong bl33_اندراج,
						     ulong fdt_addr)
{
	أرجع bl2_plat_get_bl31_params_v2_default(bl32_اندراج, bl33_اندراج,
						   fdt_addr);
}

ساكن مضمن فراغ raw_write_daif(بنانشانی صحيح daif)
{
	__asm__ __volatile__("msr DAIF, %x0\n\t" : : "r" (daif) : "memory");
}

تعريفنوع فراغ __noreturn (*atf_اندراج_t)(بنية bl31_params *params, فراغ *plat_params);

ساكن فراغ __noreturn bl31_اندراج(ulong bl31_اندراج, ulong bl32_اندراج,
				  ulong bl33_اندراج, ulong fdt_addr)
{
	atf_اندراج_t  atf_اندراج = (atf_اندراج_t)bl31_اندراج;
	فراغ *bl31_params;

	إذا (CONFIG_IS_ENABLED(ATF_لدو_IMAGE_V2))
		bl31_params = bl2_plat_get_bl31_params_v2(bl32_اندراج,
							  bl33_اندراج,
							  fdt_addr);
	وإلا
		bl31_params = bl2_plat_get_bl31_params(bl32_اندراج, bl33_اندراج,
						       fdt_addr);

	raw_write_daif(SPSR_EXCEPTION_نقاب);
	إذا (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_disable();

	atf_اندراج(bl31_params, (فراغ *)fdt_addr);
}

ساكن صحيح spl_fit_images_لبھو(فراغ *blob, صحيح os)
{
	صحيح ماپا, گرہ, ndepth = 0;
	صحيح لبھیا = -FDT_ERR_NOTFOUND;
	ثابت فراغ *انکڑے;
	ulong val;

	إذا (!blob)
		أرجع -FDT_ERR_BADMAGIC;

	ماپا = fdt_رستہ_وتھ(blob, "/fit-images");
	إذا (ماپا < 0)
		أرجع -FDT_ERR_NOTFOUND;

	لكل (گرہ = fdt_اگلا_گرہ(blob, ماپا, &ndepth);
	     (گرہ >= 0) && (ndepth > 0);
	     گرہ = fdt_اگلا_گرہ(blob, گرہ, &ndepth)) {
		إذا (ndepth != 1)
			استمر;

		انکڑے = fdt_getprop(blob, گرہ, FIT_OS_PROP, عدم);
		إذا (!انکڑے)
			استمر;

		إذا (genimg_get_os_id(انکڑے) != os)
			استمر;

		/*
		 * A multi-segment image, e.g. an OP-TEE ELF split by
		 * binman, is recorded as one node per segment, all with
		 * the same os. Only the segment holding the ELF entry
		 * point carries an entry property, so prefer that node.
		 */
		إذا (!fit_image_get_اندراج(blob, گرہ, &val))
			أرجع گرہ;

		إذا (لبھیا < 0)
			لبھیا = گرہ;
	};

	أرجع لبھیا;
}

ulong spl_fit_images_get_اندراج(فراغ *blob, صحيح گرہ)
{
	ulong  val;
	صحيح ret;

	ret = fit_image_get_اندراج(blob, گرہ, &val);
	إذا (ret)
		ret = fit_image_get_لدو(blob, گرہ, &val);

	debug("%s: entry point 0x%lx\n", __func__, val);
	أرجع val;
}

فراغ __noreturn spl_invoke_atf(بنية spl_image_info *spl_image)
{
	ulong  bl32_اندراج = 0;
	ulong  bl33_اندراج = CONFIG_متن_ادھار;
	فراغ *blob = spl_image->fdt_addr;
	ulong platform_param = (ulong)blob;
	صحيح گرہ;

	/*
	 * Find the OP-TEE binary (in /fit-images) load address or
	 * entry point (if different) and pass it as the BL3-2 entry
	 * point, this is optional.
	 */
	گرہ = spl_fit_images_لبھو(blob, IH_OS_TEE);
	إذا (گرہ >= 0)
		bl32_اندراج = spl_fit_images_get_اندراج(blob, گرہ);

	/*
	 * Find the U-Boot binary (in /fit-images) load addreess or
	 * entry point (if different) and pass it as the BL3-3 entry
	 * point.
	 * This will need to be extended to support Falcon mode.
	 */

	گرہ = spl_fit_images_لبھو(blob, IH_OS_U_BOOT);
	إذا (گرہ >= 0)
		bl33_اندراج = spl_fit_images_get_اندراج(blob, گرہ);

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
	bl31_اندراج(spl_image->اندراج_نقطہ, bl32_اندراج,
		   bl33_اندراج, platform_param);
}
