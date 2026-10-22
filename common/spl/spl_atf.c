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
	بنية داخلا_نقطو_info bl33_ep_info;
	بنية داخلا_نقطو_info bl32_ep_info;
	بنية داخلا_نقطو_info bl31_ep_info;
};

بنية bl2_منزل_bl31_params_mem_v2 {
	بنية bl_params bl_params;
	بنية bl_params_ڳنڍ bl31_params_ڳنڍ;
	بنية bl_params_ڳنڍ bl32_params_ڳنڍ;
	بنية bl_params_ڳنڍ bl33_params_ڳنڍ;
	بنية atf_image_info bl31_image_info;
	بنية atf_image_info bl32_image_info;
	بنية atf_image_info bl33_image_info;
	بنية داخلا_نقطو_info bl33_ep_info;
	بنية داخلا_نقطو_info bl32_ep_info;
	بنية داخلا_نقطو_info bl31_ep_info;
};

بنية bl31_params *bl2_plat_get_bl31_params_default(ulong bl32_داخلا,
						     ulong bl33_داخلا,
						     ulong fdt_addr)
{
	ساكن بنية bl2_منزل_bl31_params_mem bl31_params_mem;
	بنية bl31_params *bl2_منزل_bl31_params;
	بنية داخلا_نقطو_info *bl32_ep_info;
	بنية داخلا_نقطو_info *bl33_ep_info;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, حجم(بنية bl2_منزل_bl31_params_mem));

	/* Assign memory for TF related information */
	bl2_منزل_bl31_params = &bl31_params_mem.bl31_params;
	سيٽ_PARAM_سر(bl2_منزل_bl31_params, ATF_PARAM_BL31, ATF_نسخو_1, 0);

	/* Fill BL31 related information */
	bl2_منزل_bl31_params->bl31_image_info = &bl31_params_mem.bl31_image_info;
	سيٽ_PARAM_سر(bl2_منزل_bl31_params->bl31_image_info,
		       ATF_PARAM_IMAGE_ٻٽو, ATF_نسخو_1, 0);

	/* Fill BL32 related information */
	bl2_منزل_bl31_params->bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	سيٽ_PARAM_سر(bl32_ep_info, ATF_PARAM_EP, ATF_نسخو_1,
		       ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl32_ep_info->دليلون.arg3 = fdt_addr;
	bl32_ep_info->pc = bl32_داخلا ? bl32_داخلا : 0;
	bl32_ep_info->spsr = SPSR_64(گهڻائي_EL1, گهڻائي_SP_ELX,
				     DISABLE_ALL_EXCEPTIONS);

	bl2_منزل_bl31_params->bl32_image_info = &bl31_params_mem.bl32_image_info;
	سيٽ_PARAM_سر(bl2_منزل_bl31_params->bl32_image_info,
		       ATF_PARAM_IMAGE_ٻٽو, ATF_نسخو_1, 0);

	/* Fill BL33 related information */
	bl2_منزل_bl31_params->bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	سيٽ_PARAM_سر(bl33_ep_info, ATF_PARAM_EP, ATF_نسخو_1,
		       ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl33_ep_info->دليلون.arg0 = 0xffff & read_mpidr();
	bl33_ep_info->pc = bl33_داخلا;
	bl33_ep_info->spsr = SPSR_64(گهڻائي_EL2, گهڻائي_SP_ELX,
				     DISABLE_ALL_EXCEPTIONS);

	bl2_منزل_bl31_params->bl33_image_info = &bl31_params_mem.bl33_image_info;
	سيٽ_PARAM_سر(bl2_منزل_bl31_params->bl33_image_info,
		       ATF_PARAM_IMAGE_ٻٽو, ATF_نسخو_1, 0);

	أرجع bl2_منزل_bl31_params;
}

__weak بنية bl31_params *bl2_plat_get_bl31_params(ulong bl32_داخلا,
						    ulong bl33_داخلا,
						    ulong fdt_addr)
{
	أرجع bl2_plat_get_bl31_params_default(bl32_داخلا, bl33_داخلا,
						fdt_addr);
}

بنية bl_params *bl2_plat_get_bl31_params_v2_default(ulong bl32_داخلا,
						      ulong bl33_داخلا,
						      ulong fdt_addr)
{
	ساكن بنية bl2_منزل_bl31_params_mem_v2 bl31_params_mem;
	بنية bl_params *bl_params;
	بنية bl_params_ڳنڍ *bl_params_ڳنڍ;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, حجم(bl31_params_mem));

	/* Assign memory for TF related information */
	bl_params = &bl31_params_mem.bl_params;
	سيٽ_PARAM_سر(bl_params, ATF_PARAM_BL_PARAMS, ATF_نسخو_2, 0);
	bl_params->سر = &bl31_params_mem.bl31_params_ڳنڍ;

	/* Fill BL31 related information */
	bl_params_ڳنڍ = &bl31_params_mem.bl31_params_ڳنڍ;
	bl_params_ڳنڍ->image_id = ATF_BL31_IMAGE_ID;
	bl_params_ڳنڍ->image_info = &bl31_params_mem.bl31_image_info;
	bl_params_ڳنڍ->ep_info = &bl31_params_mem.bl31_ep_info;
	bl_params_ڳنڍ->ايندڙ_params_info = &bl31_params_mem.bl32_params_ڳنڍ;
	سيٽ_PARAM_سر(bl_params_ڳنڍ->image_info, ATF_PARAM_IMAGE_ٻٽو,
		       ATF_نسخو_2, 0);

	/* Fill BL32 related information */
	bl_params_ڳنڍ = &bl31_params_mem.bl32_params_ڳنڍ;
	bl_params_ڳنڍ->image_id = ATF_BL32_IMAGE_ID;
	bl_params_ڳنڍ->image_info = &bl31_params_mem.bl32_image_info;
	bl_params_ڳنڍ->ep_info = &bl31_params_mem.bl32_ep_info;
	bl_params_ڳنڍ->ايندڙ_params_info = &bl31_params_mem.bl33_params_ڳنڍ;
	سيٽ_PARAM_سر(bl_params_ڳنڍ->ep_info, ATF_PARAM_EP,
		       ATF_نسخو_2, ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl_params_ڳنڍ->ep_info->دليلون.arg3 = fdt_addr;
	bl_params_ڳنڍ->ep_info->pc = bl32_داخلا ? bl32_داخلا : 0;
	bl_params_ڳنڍ->ep_info->spsr = SPSR_64(گهڻائي_EL1, گهڻائي_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	سيٽ_PARAM_سر(bl_params_ڳنڍ->image_info, ATF_PARAM_IMAGE_ٻٽو,
		       ATF_نسخو_2, 0);

	/* Fill BL33 related information */
	bl_params_ڳنڍ = &bl31_params_mem.bl33_params_ڳنڍ;
	bl_params_ڳنڍ->image_id = ATF_BL33_IMAGE_ID;
	bl_params_ڳنڍ->image_info = &bl31_params_mem.bl33_image_info;
	bl_params_ڳنڍ->ep_info = &bl31_params_mem.bl33_ep_info;
	bl_params_ڳنڍ->ايندڙ_params_info = عدم;
	سيٽ_PARAM_سر(bl_params_ڳنڍ->ep_info, ATF_PARAM_EP,
		       ATF_نسخو_2, ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl_params_ڳنڍ->ep_info->دليلون.arg0 = 0xffff & read_mpidr();
	bl_params_ڳنڍ->ep_info->pc = bl33_داخلا;
	bl_params_ڳنڍ->ep_info->spsr = SPSR_64(گهڻائي_EL2, گهڻائي_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	سيٽ_PARAM_سر(bl_params_ڳنڍ->image_info, ATF_PARAM_IMAGE_ٻٽو,
		       ATF_نسخو_2, 0);

	أرجع bl_params;
}

__weak بنية bl_params *bl2_plat_get_bl31_params_v2(ulong bl32_داخلا,
						     ulong bl33_داخلا,
						     ulong fdt_addr)
{
	أرجع bl2_plat_get_bl31_params_v2_default(bl32_داخلا, bl33_داخلا,
						   fdt_addr);
}

ساكن مضمن فراغ raw_write_daif(بنانشان صحيح daif)
{
	__asm__ __volatile__("msr DAIF, %x0\n\t" : : "r" (daif) : "memory");
}

تعريفنوع فراغ __noreturn (*atf_داخلا_t)(بنية bl31_params *params, فراغ *plat_params);

ساكن فراغ __noreturn bl31_داخلا(ulong bl31_داخلا, ulong bl32_داخلا,
				  ulong bl33_داخلا, ulong fdt_addr)
{
	atf_داخلا_t  atf_داخلا = (atf_داخلا_t)bl31_داخلا;
	فراغ *bl31_params;

	إذا (CONFIG_IS_ENABLED(ATF_ڀريو_IMAGE_V2))
		bl31_params = bl2_plat_get_bl31_params_v2(bl32_داخلا,
							  bl33_داخلا,
							  fdt_addr);
	وإلا
		bl31_params = bl2_plat_get_bl31_params(bl32_داخلا, bl33_داخلا,
						       fdt_addr);

	raw_write_daif(SPSR_EXCEPTION_نقاب);
	إذا (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_disable();

	atf_داخلا(bl31_params, (فراغ *)fdt_addr);
}

ساكن صحيح spl_fit_images_ڳوليو(فراغ *blob, صحيح os)
{
	صحيح والد, ڳنڍ, ndepth = 0;
	صحيح مليو = -FDT_ERR_NOTFOUND;
	ثابت فراغ *معلومات;
	ulong val;

	إذا (!blob)
		أرجع -FDT_ERR_BADMAGIC;

	والد = fdt_رستو_وٿي(blob, "/fit-images");
	إذا (والد < 0)
		أرجع -FDT_ERR_NOTFOUND;

	لكل (ڳنڍ = fdt_ايندڙ_ڳنڍ(blob, والد, &ndepth);
	     (ڳنڍ >= 0) && (ndepth > 0);
	     ڳنڍ = fdt_ايندڙ_ڳنڍ(blob, ڳنڍ, &ndepth)) {
		إذا (ndepth != 1)
			استمر;

		معلومات = fdt_getprop(blob, ڳنڍ, FIT_OS_PROP, عدم);
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
		إذا (!fit_image_get_داخلا(blob, ڳنڍ, &val))
			أرجع ڳنڍ;

		إذا (مليو < 0)
			مليو = ڳنڍ;
	};

	أرجع مليو;
}

ulong spl_fit_images_get_داخلا(فراغ *blob, صحيح ڳنڍ)
{
	ulong  val;
	صحيح ret;

	ret = fit_image_get_داخلا(blob, ڳنڍ, &val);
	إذا (ret)
		ret = fit_image_get_ڀريو(blob, ڳنڍ, &val);

	debug("%s: entry point 0x%lx\n", __func__, val);
	أرجع val;
}

فراغ __noreturn spl_invoke_atf(بنية spl_image_info *spl_image)
{
	ulong  bl32_داخلا = 0;
	ulong  bl33_داخلا = CONFIG_متن_بنياد;
	فراغ *blob = spl_image->fdt_addr;
	ulong platform_param = (ulong)blob;
	صحيح ڳنڍ;

	/*
	 * Find the OP-TEE binary (in /fit-images) load address or
	 * entry point (if different) and pass it as the BL3-2 entry
	 * point, this is optional.
	 */
	ڳنڍ = spl_fit_images_ڳوليو(blob, IH_OS_TEE);
	إذا (ڳنڍ >= 0)
		bl32_داخلا = spl_fit_images_get_داخلا(blob, ڳنڍ);

	/*
	 * Find the U-Boot binary (in /fit-images) load addreess or
	 * entry point (if different) and pass it as the BL3-3 entry
	 * point.
	 * This will need to be extended to support Falcon mode.
	 */

	ڳنڍ = spl_fit_images_ڳوليو(blob, IH_OS_U_BOOT);
	إذا (ڳنڍ >= 0)
		bl33_داخلا = spl_fit_images_get_داخلا(blob, ڳنڍ);

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
	bl31_داخلا(spl_image->داخلا_نقطو, bl32_داخلا,
		   bl33_داخلا, platform_param);
}
