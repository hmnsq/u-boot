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

#ਅਨ੍ਤਰ੍ਭਾਵ <atf_common.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <cpu_func.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/cache.h>

/* Holds all the structures we need for bl31 parameter passing */
ਬਣਤਰ bl2_ਮੰਜ਼ਿਲ_bl31_params_mem {
	ਬਣਤਰ bl31_params bl31_params;
	ਬਣਤਰ atf_image_info bl31_image_info;
	ਬਣਤਰ atf_image_info bl32_image_info;
	ਬਣਤਰ atf_image_info bl33_image_info;
	ਬਣਤਰ ਇੰਦਰਾਜ_ਬਿੰਦੂ_info bl33_ep_info;
	ਬਣਤਰ ਇੰਦਰਾਜ_ਬਿੰਦੂ_info bl32_ep_info;
	ਬਣਤਰ ਇੰਦਰਾਜ_ਬਿੰਦੂ_info bl31_ep_info;
};

ਬਣਤਰ bl2_ਮੰਜ਼ਿਲ_bl31_params_mem_v2 {
	ਬਣਤਰ bl_params bl_params;
	ਬਣਤਰ bl_params_ਗੰਢ bl31_params_ਗੰਢ;
	ਬਣਤਰ bl_params_ਗੰਢ bl32_params_ਗੰਢ;
	ਬਣਤਰ bl_params_ਗੰਢ bl33_params_ਗੰਢ;
	ਬਣਤਰ atf_image_info bl31_image_info;
	ਬਣਤਰ atf_image_info bl32_image_info;
	ਬਣਤਰ atf_image_info bl33_image_info;
	ਬਣਤਰ ਇੰਦਰਾਜ_ਬਿੰਦੂ_info bl33_ep_info;
	ਬਣਤਰ ਇੰਦਰਾਜ_ਬਿੰਦੂ_info bl32_ep_info;
	ਬਣਤਰ ਇੰਦਰਾਜ_ਬਿੰਦੂ_info bl31_ep_info;
};

ਬਣਤਰ bl31_params *bl2_plat_get_bl31_params_default(ulong bl32_ਇੰਦਰਾਜ,
						     ulong bl33_ਇੰਦਰਾਜ,
						     ulong fdt_addr)
{
	ਸਥਿਰ ਬਣਤਰ bl2_ਮੰਜ਼ਿਲ_bl31_params_mem bl31_params_mem;
	ਬਣਤਰ bl31_params *bl2_ਮੰਜ਼ਿਲ_bl31_params;
	ਬਣਤਰ ਇੰਦਰਾਜ_ਬਿੰਦੂ_info *bl32_ep_info;
	ਬਣਤਰ ਇੰਦਰਾਜ_ਬਿੰਦੂ_info *bl33_ep_info;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, ਆਕਾਰ(ਬਣਤਰ bl2_ਮੰਜ਼ਿਲ_bl31_params_mem));

	/* Assign memory for TF related information */
	bl2_ਮੰਜ਼ਿਲ_bl31_params = &bl31_params_mem.bl31_params;
	ਸੈੱਟ_PARAM_ਸਿਰ(bl2_ਮੰਜ਼ਿਲ_bl31_params, ATF_PARAM_BL31, ATF_ਸੰਸਕਰਣ_1, 0);

	/* Fill BL31 related information */
	bl2_ਮੰਜ਼ਿਲ_bl31_params->bl31_image_info = &bl31_params_mem.bl31_image_info;
	ਸੈੱਟ_PARAM_ਸਿਰ(bl2_ਮੰਜ਼ਿਲ_bl31_params->bl31_image_info,
		       ATF_PARAM_IMAGE_ਦੁਆਧਾਰੀ, ATF_ਸੰਸਕਰਣ_1, 0);

	/* Fill BL32 related information */
	bl2_ਮੰਜ਼ਿਲ_bl31_params->bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	ਸੈੱਟ_PARAM_ਸਿਰ(bl32_ep_info, ATF_PARAM_EP, ATF_ਸੰਸਕਰਣ_1,
		       ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl32_ep_info->ਦਲੀਲਾਂ.arg3 = fdt_addr;
	bl32_ep_info->pc = bl32_ਇੰਦਰਾਜ ? bl32_ਇੰਦਰਾਜ : 0;
	bl32_ep_info->spsr = SPSR_64(ਬਹੁਲਕ_EL1, ਬਹੁਲਕ_SP_ELX,
				     DISABLE_ALL_EXCEPTIONS);

	bl2_ਮੰਜ਼ਿਲ_bl31_params->bl32_image_info = &bl31_params_mem.bl32_image_info;
	ਸੈੱਟ_PARAM_ਸਿਰ(bl2_ਮੰਜ਼ਿਲ_bl31_params->bl32_image_info,
		       ATF_PARAM_IMAGE_ਦੁਆਧਾਰੀ, ATF_ਸੰਸਕਰਣ_1, 0);

	/* Fill BL33 related information */
	bl2_ਮੰਜ਼ਿਲ_bl31_params->bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	ਸੈੱਟ_PARAM_ਸਿਰ(bl33_ep_info, ATF_PARAM_EP, ATF_ਸੰਸਕਰਣ_1,
		       ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl33_ep_info->ਦਲੀਲਾਂ.arg0 = 0xffff & read_mpidr();
	bl33_ep_info->pc = bl33_ਇੰਦਰਾਜ;
	bl33_ep_info->spsr = SPSR_64(ਬਹੁਲਕ_EL2, ਬਹੁਲਕ_SP_ELX,
				     DISABLE_ALL_EXCEPTIONS);

	bl2_ਮੰਜ਼ਿਲ_bl31_params->bl33_image_info = &bl31_params_mem.bl33_image_info;
	ਸੈੱਟ_PARAM_ਸਿਰ(bl2_ਮੰਜ਼ਿਲ_bl31_params->bl33_image_info,
		       ATF_PARAM_IMAGE_ਦੁਆਧਾਰੀ, ATF_ਸੰਸਕਰਣ_1, 0);

	ਵਾਪਸ bl2_ਮੰਜ਼ਿਲ_bl31_params;
}

__weak ਬਣਤਰ bl31_params *bl2_plat_get_bl31_params(ulong bl32_ਇੰਦਰਾਜ,
						    ulong bl33_ਇੰਦਰਾਜ,
						    ulong fdt_addr)
{
	ਵਾਪਸ bl2_plat_get_bl31_params_default(bl32_ਇੰਦਰਾਜ, bl33_ਇੰਦਰਾਜ,
						fdt_addr);
}

ਬਣਤਰ bl_params *bl2_plat_get_bl31_params_v2_default(ulong bl32_ਇੰਦਰਾਜ,
						      ulong bl33_ਇੰਦਰਾਜ,
						      ulong fdt_addr)
{
	ਸਥਿਰ ਬਣਤਰ bl2_ਮੰਜ਼ਿਲ_bl31_params_mem_v2 bl31_params_mem;
	ਬਣਤਰ bl_params *bl_params;
	ਬਣਤਰ bl_params_ਗੰਢ *bl_params_ਗੰਢ;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, ਆਕਾਰ(bl31_params_mem));

	/* Assign memory for TF related information */
	bl_params = &bl31_params_mem.bl_params;
	ਸੈੱਟ_PARAM_ਸਿਰ(bl_params, ATF_PARAM_BL_PARAMS, ATF_ਸੰਸਕਰਣ_2, 0);
	bl_params->ਸਿਰ = &bl31_params_mem.bl31_params_ਗੰਢ;

	/* Fill BL31 related information */
	bl_params_ਗੰਢ = &bl31_params_mem.bl31_params_ਗੰਢ;
	bl_params_ਗੰਢ->image_id = ATF_BL31_IMAGE_ID;
	bl_params_ਗੰਢ->image_info = &bl31_params_mem.bl31_image_info;
	bl_params_ਗੰਢ->ep_info = &bl31_params_mem.bl31_ep_info;
	bl_params_ਗੰਢ->ਅਗਲਾ_params_info = &bl31_params_mem.bl32_params_ਗੰਢ;
	ਸੈੱਟ_PARAM_ਸਿਰ(bl_params_ਗੰਢ->image_info, ATF_PARAM_IMAGE_ਦੁਆਧਾਰੀ,
		       ATF_ਸੰਸਕਰਣ_2, 0);

	/* Fill BL32 related information */
	bl_params_ਗੰਢ = &bl31_params_mem.bl32_params_ਗੰਢ;
	bl_params_ਗੰਢ->image_id = ATF_BL32_IMAGE_ID;
	bl_params_ਗੰਢ->image_info = &bl31_params_mem.bl32_image_info;
	bl_params_ਗੰਢ->ep_info = &bl31_params_mem.bl32_ep_info;
	bl_params_ਗੰਢ->ਅਗਲਾ_params_info = &bl31_params_mem.bl33_params_ਗੰਢ;
	ਸੈੱਟ_PARAM_ਸਿਰ(bl_params_ਗੰਢ->ep_info, ATF_PARAM_EP,
		       ATF_ਸੰਸਕਰਣ_2, ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl_params_ਗੰਢ->ep_info->ਦਲੀਲਾਂ.arg3 = fdt_addr;
	bl_params_ਗੰਢ->ep_info->pc = bl32_ਇੰਦਰਾਜ ? bl32_ਇੰਦਰਾਜ : 0;
	bl_params_ਗੰਢ->ep_info->spsr = SPSR_64(ਬਹੁਲਕ_EL1, ਬਹੁਲਕ_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	ਸੈੱਟ_PARAM_ਸਿਰ(bl_params_ਗੰਢ->image_info, ATF_PARAM_IMAGE_ਦੁਆਧਾਰੀ,
		       ATF_ਸੰਸਕਰਣ_2, 0);

	/* Fill BL33 related information */
	bl_params_ਗੰਢ = &bl31_params_mem.bl33_params_ਗੰਢ;
	bl_params_ਗੰਢ->image_id = ATF_BL33_IMAGE_ID;
	bl_params_ਗੰਢ->image_info = &bl31_params_mem.bl33_image_info;
	bl_params_ਗੰਢ->ep_info = &bl31_params_mem.bl33_ep_info;
	bl_params_ਗੰਢ->ਅਗਲਾ_params_info = ਨਿਰਾ;
	ਸੈੱਟ_PARAM_ਸਿਰ(bl_params_ਗੰਢ->ep_info, ATF_PARAM_EP,
		       ATF_ਸੰਸਕਰਣ_2, ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl_params_ਗੰਢ->ep_info->ਦਲੀਲਾਂ.arg0 = 0xffff & read_mpidr();
	bl_params_ਗੰਢ->ep_info->pc = bl33_ਇੰਦਰਾਜ;
	bl_params_ਗੰਢ->ep_info->spsr = SPSR_64(ਬਹੁਲਕ_EL2, ਬਹੁਲਕ_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	ਸੈੱਟ_PARAM_ਸਿਰ(bl_params_ਗੰਢ->image_info, ATF_PARAM_IMAGE_ਦੁਆਧਾਰੀ,
		       ATF_ਸੰਸਕਰਣ_2, 0);

	ਵਾਪਸ bl_params;
}

__weak ਬਣਤਰ bl_params *bl2_plat_get_bl31_params_v2(ulong bl32_ਇੰਦਰਾਜ,
						     ulong bl33_ਇੰਦਰਾਜ,
						     ulong fdt_addr)
{
	ਵਾਪਸ bl2_plat_get_bl31_params_v2_default(bl32_ਇੰਦਰਾਜ, bl33_ਇੰਦਰਾਜ,
						   fdt_addr);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ raw_write_daif(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ daif)
{
	__asm__ __volatile__("msr DAIF, %x0\n\t" : : "r" (daif) : "memory");
}

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ __noreturn (*atf_ਇੰਦਰਾਜ_t)(ਬਣਤਰ bl31_params *params, ਖਾਲੀ *plat_params);

ਸਥਿਰ ਖਾਲੀ __noreturn bl31_ਇੰਦਰਾਜ(ulong bl31_ਇੰਦਰਾਜ, ulong bl32_ਇੰਦਰਾਜ,
				  ulong bl33_ਇੰਦਰਾਜ, ulong fdt_addr)
{
	atf_ਇੰਦਰਾਜ_t  atf_ਇੰਦਰਾਜ = (atf_ਇੰਦਰਾਜ_t)bl31_ਇੰਦਰਾਜ;
	ਖਾਲੀ *bl31_params;

	ਜੇ (CONFIG_IS_ENABLED(ATF_ਲੱਦੋ_IMAGE_V2))
		bl31_params = bl2_plat_get_bl31_params_v2(bl32_ਇੰਦਰਾਜ,
							  bl33_ਇੰਦਰਾਜ,
							  fdt_addr);
	ਨਹੀਂਤਾਂ
		bl31_params = bl2_plat_get_bl31_params(bl32_ਇੰਦਰਾਜ, bl33_ਇੰਦਰਾਜ,
						       fdt_addr);

	raw_write_daif(SPSR_EXCEPTION_ਨਕਾਬ);
	ਜੇ (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_disable();

	atf_ਇੰਦਰਾਜ(bl31_params, (ਖਾਲੀ *)fdt_addr);
}

ਸਥਿਰ ਪੂਰਨ spl_fit_images_ਲੱਭੋ(ਖਾਲੀ *blob, ਪੂਰਨ os)
{
	ਪੂਰਨ ਮਾਪਾ, ਗੰਢ, ndepth = 0;
	ਪੂਰਨ ਮਿਲਿਆ = -FDT_ERR_NOTFOUND;
	ਅਚਲ ਖਾਲੀ *ਅੰਕੜੇ;
	ulong val;

	ਜੇ (!blob)
		ਵਾਪਸ -FDT_ERR_BADMAGIC;

	ਮਾਪਾ = fdt_ਰਸਤਾ_ਵਿੱਥ(blob, "/fit-images");
	ਜੇ (ਮਾਪਾ < 0)
		ਵਾਪਸ -FDT_ERR_NOTFOUND;

	ਲਈ (ਗੰਢ = fdt_ਅਗਲਾ_ਗੰਢ(blob, ਮਾਪਾ, &ndepth);
	     (ਗੰਢ >= 0) && (ndepth > 0);
	     ਗੰਢ = fdt_ਅਗਲਾ_ਗੰਢ(blob, ਗੰਢ, &ndepth)) {
		ਜੇ (ndepth != 1)
			ਜਾਰੀ;

		ਅੰਕੜੇ = fdt_getprop(blob, ਗੰਢ, FIT_OS_PROP, ਨਿਰਾ);
		ਜੇ (!ਅੰਕੜੇ)
			ਜਾਰੀ;

		ਜੇ (genimg_get_os_id(ਅੰਕੜੇ) != os)
			ਜਾਰੀ;

		/*
		 * A multi-segment image, e.g. an OP-TEE ELF split by
		 * binman, is recorded as one node per segment, all with
		 * the same os. Only the segment holding the ELF entry
		 * point carries an entry property, so prefer that node.
		 */
		ਜੇ (!fit_image_get_ਇੰਦਰਾਜ(blob, ਗੰਢ, &val))
			ਵਾਪਸ ਗੰਢ;

		ਜੇ (ਮਿਲਿਆ < 0)
			ਮਿਲਿਆ = ਗੰਢ;
	};

	ਵਾਪਸ ਮਿਲਿਆ;
}

ulong spl_fit_images_get_ਇੰਦਰਾਜ(ਖਾਲੀ *blob, ਪੂਰਨ ਗੰਢ)
{
	ulong  val;
	ਪੂਰਨ ret;

	ret = fit_image_get_ਇੰਦਰਾਜ(blob, ਗੰਢ, &val);
	ਜੇ (ret)
		ret = fit_image_get_ਲੱਦੋ(blob, ਗੰਢ, &val);

	debug("%s: entry point 0x%lx\n", __func__, val);
	ਵਾਪਸ val;
}

ਖਾਲੀ __noreturn spl_invoke_atf(ਬਣਤਰ spl_image_info *spl_image)
{
	ulong  bl32_ਇੰਦਰਾਜ = 0;
	ulong  bl33_ਇੰਦਰਾਜ = CONFIG_ਪਾਠ_ਅਧਾਰ;
	ਖਾਲੀ *blob = spl_image->fdt_addr;
	ulong platform_param = (ulong)blob;
	ਪੂਰਨ ਗੰਢ;

	/*
	 * Find the OP-TEE binary (in /fit-images) load address or
	 * entry point (if different) and pass it as the BL3-2 entry
	 * point, this is optional.
	 */
	ਗੰਢ = spl_fit_images_ਲੱਭੋ(blob, IH_OS_TEE);
	ਜੇ (ਗੰਢ >= 0)
		bl32_ਇੰਦਰਾਜ = spl_fit_images_get_ਇੰਦਰਾਜ(blob, ਗੰਢ);

	/*
	 * Find the U-Boot binary (in /fit-images) load addreess or
	 * entry point (if different) and pass it as the BL3-3 entry
	 * point.
	 * This will need to be extended to support Falcon mode.
	 */

	ਗੰਢ = spl_fit_images_ਲੱਭੋ(blob, IH_OS_U_BOOT);
	ਜੇ (ਗੰਢ >= 0)
		bl33_ਇੰਦਰਾਜ = spl_fit_images_get_ਇੰਦਰਾਜ(blob, ਗੰਢ);

	/*
	 * If ATF_NO_PLATFORM_PARAM is set, we override the platform
	 * parameter and always pass 0.  This is a workaround for
	 * older ATF versions that have insufficiently robust (or
	 * overzealous) argument validation.
	 */
	ਜੇ (CONFIG_IS_ENABLED(ATF_NO_PLATFORM_PARAM))
		platform_param = 0;

	/*
	 * We don't provide a BL3-2 entry yet, but this will be possible
	 * using similar logic.
	 */
	bl31_ਇੰਦਰਾਜ(spl_image->ਇੰਦਰਾਜ_ਬਿੰਦੂ, bl32_ਇੰਦਰਾਜ,
		   bl33_ਇੰਦਰਾਜ, platform_param);
}
