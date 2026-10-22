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
ساخت bl2_مقصد_bl31_params_mem {
	ساخت bl31_params bl31_params;
	ساخت atf_image_info bl31_image_info;
	ساخت atf_image_info bl32_image_info;
	ساخت atf_image_info bl33_image_info;
	ساخت اندراج_نقطہ_info bl33_ep_info;
	ساخت اندراج_نقطہ_info bl32_ep_info;
	ساخت اندراج_نقطہ_info bl31_ep_info;
};

ساخت bl2_مقصد_bl31_params_mem_v2 {
	ساخت bl_params bl_params;
	ساخت bl_params_گرہ bl31_params_گرہ;
	ساخت bl_params_گرہ bl32_params_گرہ;
	ساخت bl_params_گرہ bl33_params_گرہ;
	ساخت atf_image_info bl31_image_info;
	ساخت atf_image_info bl32_image_info;
	ساخت atf_image_info bl33_image_info;
	ساخت اندراج_نقطہ_info bl33_ep_info;
	ساخت اندراج_نقطہ_info bl32_ep_info;
	ساخت اندراج_نقطہ_info bl31_ep_info;
};

ساخت bl31_params *bl2_plat_get_bl31_params_default(ulong bl32_اندراج,
						     ulong bl33_اندراج,
						     ulong fdt_addr)
{
	ساکن ساخت bl2_مقصد_bl31_params_mem bl31_params_mem;
	ساخت bl31_params *bl2_مقصد_bl31_params;
	ساخت اندراج_نقطہ_info *bl32_ep_info;
	ساخت اندراج_نقطہ_info *bl33_ep_info;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, حجم(ساخت bl2_مقصد_bl31_params_mem));

	/* Assign memory for TF related information */
	bl2_مقصد_bl31_params = &bl31_params_mem.bl31_params;
	سیٹ_PARAM_سر(bl2_مقصد_bl31_params, ATF_PARAM_BL31, ATF_نسخہ_1, 0);

	/* Fill BL31 related information */
	bl2_مقصد_bl31_params->bl31_image_info = &bl31_params_mem.bl31_image_info;
	سیٹ_PARAM_سر(bl2_مقصد_bl31_params->bl31_image_info,
		       ATF_PARAM_IMAGE_ثنائی, ATF_نسخہ_1, 0);

	/* Fill BL32 related information */
	bl2_مقصد_bl31_params->bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	سیٹ_PARAM_سر(bl32_ep_info, ATF_PARAM_EP, ATF_نسخہ_1,
		       ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl32_ep_info->دلائل.arg3 = fdt_addr;
	bl32_ep_info->pc = bl32_اندراج ? bl32_اندراج : 0;
	bl32_ep_info->spsr = SPSR_64(منوال_EL1, منوال_SP_ELX,
				     DISABLE_ALL_EXCEPTIONS);

	bl2_مقصد_bl31_params->bl32_image_info = &bl31_params_mem.bl32_image_info;
	سیٹ_PARAM_سر(bl2_مقصد_bl31_params->bl32_image_info,
		       ATF_PARAM_IMAGE_ثنائی, ATF_نسخہ_1, 0);

	/* Fill BL33 related information */
	bl2_مقصد_bl31_params->bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	سیٹ_PARAM_سر(bl33_ep_info, ATF_PARAM_EP, ATF_نسخہ_1,
		       ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl33_ep_info->دلائل.arg0 = 0xffff & read_mpidr();
	bl33_ep_info->pc = bl33_اندراج;
	bl33_ep_info->spsr = SPSR_64(منوال_EL2, منوال_SP_ELX,
				     DISABLE_ALL_EXCEPTIONS);

	bl2_مقصد_bl31_params->bl33_image_info = &bl31_params_mem.bl33_image_info;
	سیٹ_PARAM_سر(bl2_مقصد_bl31_params->bl33_image_info,
		       ATF_PARAM_IMAGE_ثنائی, ATF_نسخہ_1, 0);

	واپس bl2_مقصد_bl31_params;
}

__weak ساخت bl31_params *bl2_plat_get_bl31_params(ulong bl32_اندراج,
						    ulong bl33_اندراج,
						    ulong fdt_addr)
{
	واپس bl2_plat_get_bl31_params_default(bl32_اندراج, bl33_اندراج,
						fdt_addr);
}

ساخت bl_params *bl2_plat_get_bl31_params_v2_default(ulong bl32_اندراج,
						      ulong bl33_اندراج,
						      ulong fdt_addr)
{
	ساکن ساخت bl2_مقصد_bl31_params_mem_v2 bl31_params_mem;
	ساخت bl_params *bl_params;
	ساخت bl_params_گرہ *bl_params_گرہ;

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
	bl_params_گرہ->ep_info->دلائل.arg3 = fdt_addr;
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
	bl_params_گرہ->اگلا_params_info = تہی;
	سیٹ_PARAM_سر(bl_params_گرہ->ep_info, ATF_PARAM_EP,
		       ATF_نسخہ_2, ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl_params_گرہ->ep_info->دلائل.arg0 = 0xffff & read_mpidr();
	bl_params_گرہ->ep_info->pc = bl33_اندراج;
	bl_params_گرہ->ep_info->spsr = SPSR_64(منوال_EL2, منوال_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	سیٹ_PARAM_سر(bl_params_گرہ->image_info, ATF_PARAM_IMAGE_ثنائی,
		       ATF_نسخہ_2, 0);

	واپس bl_params;
}

__weak ساخت bl_params *bl2_plat_get_bl31_params_v2(ulong bl32_اندراج,
						     ulong bl33_اندراج,
						     ulong fdt_addr)
{
	واپس bl2_plat_get_bl31_params_v2_default(bl32_اندراج, bl33_اندراج,
						   fdt_addr);
}

ساکن اندرونی خالی raw_write_daif(غیرعلامتی صحیح daif)
{
	__asm__ __volatile__("msr DAIF, %x0\n\t" : : "r" (daif) : "memory");
}

قسمتعریف خالی __noreturn (*atf_اندراج_t)(ساخت bl31_params *params, خالی *plat_params);

ساکن خالی __noreturn bl31_اندراج(ulong bl31_اندراج, ulong bl32_اندراج,
				  ulong bl33_اندراج, ulong fdt_addr)
{
	atf_اندراج_t  atf_اندراج = (atf_اندراج_t)bl31_اندراج;
	خالی *bl31_params;

	اگر (CONFIG_IS_ENABLED(ATF_بھرو_IMAGE_V2))
		bl31_params = bl2_plat_get_bl31_params_v2(bl32_اندراج,
							  bl33_اندراج,
							  fdt_addr);
	ورنہ
		bl31_params = bl2_plat_get_bl31_params(bl32_اندراج, bl33_اندراج,
						       fdt_addr);

	raw_write_daif(SPSR_EXCEPTION_نقاب);
	اگر (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_disable();

	atf_اندراج(bl31_params, (خالی *)fdt_addr);
}

ساکن صحیح spl_fit_images_ڈھونڈو(خالی *blob, صحیح os)
{
	صحیح والد, گرہ, ndepth = 0;
	صحیح ملا = -FDT_ERR_NOTFOUND;
	ثابت خالی *ڈیٹا;
	ulong val;

	اگر (!blob)
		واپس -FDT_ERR_BADMAGIC;

	والد = fdt_راستہ_انحراف(blob, "/fit-images");
	اگر (والد < 0)
		واپس -FDT_ERR_NOTFOUND;

	برائے (گرہ = fdt_اگلا_گرہ(blob, والد, &ndepth);
	     (گرہ >= 0) && (ndepth > 0);
	     گرہ = fdt_اگلا_گرہ(blob, گرہ, &ndepth)) {
		اگر (ndepth != 1)
			جاری;

		ڈیٹا = fdt_getprop(blob, گرہ, FIT_OS_PROP, تہی);
		اگر (!ڈیٹا)
			جاری;

		اگر (genimg_get_os_id(ڈیٹا) != os)
			جاری;

		/*
		 * A multi-segment image, e.g. an OP-TEE ELF split by
		 * binman, is recorded as one node per segment, all with
		 * the same os. Only the segment holding the ELF entry
		 * point carries an entry property, so prefer that node.
		 */
		اگر (!fit_image_get_اندراج(blob, گرہ, &val))
			واپس گرہ;

		اگر (ملا < 0)
			ملا = گرہ;
	};

	واپس ملا;
}

ulong spl_fit_images_get_اندراج(خالی *blob, صحیح گرہ)
{
	ulong  val;
	صحیح ret;

	ret = fit_image_get_اندراج(blob, گرہ, &val);
	اگر (ret)
		ret = fit_image_get_بھرو(blob, گرہ, &val);

	debug("%s: entry point 0x%lx\n", __func__, val);
	واپس val;
}

خالی __noreturn spl_invoke_atf(ساخت spl_image_info *spl_image)
{
	ulong  bl32_اندراج = 0;
	ulong  bl33_اندراج = CONFIG_متن_بنیاد;
	خالی *blob = spl_image->fdt_addr;
	ulong platform_param = (ulong)blob;
	صحیح گرہ;

	/*
	 * Find the OP-TEE binary (in /fit-images) load address or
	 * entry point (if different) and pass it as the BL3-2 entry
	 * point, this is optional.
	 */
	گرہ = spl_fit_images_ڈھونڈو(blob, IH_OS_TEE);
	اگر (گرہ >= 0)
		bl32_اندراج = spl_fit_images_get_اندراج(blob, گرہ);

	/*
	 * Find the U-Boot binary (in /fit-images) load addreess or
	 * entry point (if different) and pass it as the BL3-3 entry
	 * point.
	 * This will need to be extended to support Falcon mode.
	 */

	گرہ = spl_fit_images_ڈھونڈو(blob, IH_OS_U_BOOT);
	اگر (گرہ >= 0)
		bl33_اندراج = spl_fit_images_get_اندراج(blob, گرہ);

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
	bl31_اندراج(spl_image->اندراج_نقطہ, bl32_اندراج,
		   bl33_اندراج, platform_param);
}
