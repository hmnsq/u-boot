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

#अन्तर्भाव <atf_common.h>
#अन्तर्भाव <cpu_func.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <asm/cache.h>

/* Holds all the structures we need for bl31 parameter passing */
संरचना bl2_गन्तव्यम्_bl31_params_mem {
	संरचना bl31_params bl31_params;
	संरचना atf_image_info bl31_image_info;
	संरचना atf_image_info bl32_image_info;
	संरचना atf_image_info bl33_image_info;
	संरचना प्रविष्टिः_point_info bl33_ep_info;
	संरचना प्रविष्टिः_point_info bl32_ep_info;
	संरचना प्रविष्टिः_point_info bl31_ep_info;
};

संरचना bl2_गन्तव्यम्_bl31_params_mem_v2 {
	संरचना bl_params bl_params;
	संरचना bl_params_ग्रन्थिः bl31_params_ग्रन्थिः;
	संरचना bl_params_ग्रन्थिः bl32_params_ग्रन्थिः;
	संरचना bl_params_ग्रन्थिः bl33_params_ग्रन्थिः;
	संरचना atf_image_info bl31_image_info;
	संरचना atf_image_info bl32_image_info;
	संरचना atf_image_info bl33_image_info;
	संरचना प्रविष्टिः_point_info bl33_ep_info;
	संरचना प्रविष्टिः_point_info bl32_ep_info;
	संरचना प्रविष्टिः_point_info bl31_ep_info;
};

संरचना bl31_params *bl2_plat_get_bl31_params_default(ulong bl32_प्रविष्टिः,
						     ulong bl33_प्रविष्टिः,
						     ulong fdt_addr)
{
	स्थिर संरचना bl2_गन्तव्यम्_bl31_params_mem bl31_params_mem;
	संरचना bl31_params *bl2_गन्तव्यम्_bl31_params;
	संरचना प्रविष्टिः_point_info *bl32_ep_info;
	संरचना प्रविष्टिः_point_info *bl33_ep_info;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	स्मृतिं_पूरय(&bl31_params_mem, 0, परिमाण(संरचना bl2_गन्तव्यम्_bl31_params_mem));

	/* Assign memory for TF related information */
	bl2_गन्तव्यम्_bl31_params = &bl31_params_mem.bl31_params;
	समुच्चयः_PARAM_शीर्ष(bl2_गन्तव्यम्_bl31_params, ATF_PARAM_BL31, ATF_संस्करणम्_1, 0);

	/* Fill BL31 related information */
	bl2_गन्तव्यम्_bl31_params->bl31_image_info = &bl31_params_mem.bl31_image_info;
	समुच्चयः_PARAM_शीर्ष(bl2_गन्तव्यम्_bl31_params->bl31_image_info,
		       ATF_PARAM_IMAGE_द्विमान, ATF_संस्करणम्_1, 0);

	/* Fill BL32 related information */
	bl2_गन्तव्यम्_bl31_params->bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	समुच्चयः_PARAM_शीर्ष(bl32_ep_info, ATF_PARAM_EP, ATF_संस्करणम्_1,
		       ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl32_ep_info->तर्काः.arg3 = fdt_addr;
	bl32_ep_info->pc = bl32_प्रविष्टिः ? bl32_प्रविष्टिः : 0;
	bl32_ep_info->spsr = SPSR_64(बहुलकम्_EL1, बहुलकम्_SP_ELX,
				     DISABLE_ALL_EXCEPTIONS);

	bl2_गन्तव्यम्_bl31_params->bl32_image_info = &bl31_params_mem.bl32_image_info;
	समुच्चयः_PARAM_शीर्ष(bl2_गन्तव्यम्_bl31_params->bl32_image_info,
		       ATF_PARAM_IMAGE_द्विमान, ATF_संस्करणम्_1, 0);

	/* Fill BL33 related information */
	bl2_गन्तव्यम्_bl31_params->bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	समुच्चयः_PARAM_शीर्ष(bl33_ep_info, ATF_PARAM_EP, ATF_संस्करणम्_1,
		       ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl33_ep_info->तर्काः.arg0 = 0xffff & read_mpidr();
	bl33_ep_info->pc = bl33_प्रविष्टिः;
	bl33_ep_info->spsr = SPSR_64(बहुलकम्_EL2, बहुलकम्_SP_ELX,
				     DISABLE_ALL_EXCEPTIONS);

	bl2_गन्तव्यम्_bl31_params->bl33_image_info = &bl31_params_mem.bl33_image_info;
	समुच्चयः_PARAM_शीर्ष(bl2_गन्तव्यम्_bl31_params->bl33_image_info,
		       ATF_PARAM_IMAGE_द्विमान, ATF_संस्करणम्_1, 0);

	प्रत्यागम bl2_गन्तव्यम्_bl31_params;
}

__weak संरचना bl31_params *bl2_plat_get_bl31_params(ulong bl32_प्रविष्टिः,
						    ulong bl33_प्रविष्टिः,
						    ulong fdt_addr)
{
	प्रत्यागम bl2_plat_get_bl31_params_default(bl32_प्रविष्टिः, bl33_प्रविष्टिः,
						fdt_addr);
}

संरचना bl_params *bl2_plat_get_bl31_params_v2_default(ulong bl32_प्रविष्टिः,
						      ulong bl33_प्रविष्टिः,
						      ulong fdt_addr)
{
	स्थिर संरचना bl2_गन्तव्यम्_bl31_params_mem_v2 bl31_params_mem;
	संरचना bl_params *bl_params;
	संरचना bl_params_ग्रन्थिः *bl_params_ग्रन्थिः;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	स्मृतिं_पूरय(&bl31_params_mem, 0, परिमाण(bl31_params_mem));

	/* Assign memory for TF related information */
	bl_params = &bl31_params_mem.bl_params;
	समुच्चयः_PARAM_शीर्ष(bl_params, ATF_PARAM_BL_PARAMS, ATF_संस्करणम्_2, 0);
	bl_params->शीर्ष = &bl31_params_mem.bl31_params_ग्रन्थिः;

	/* Fill BL31 related information */
	bl_params_ग्रन्थिः = &bl31_params_mem.bl31_params_ग्रन्थिः;
	bl_params_ग्रन्थिः->image_id = ATF_BL31_IMAGE_ID;
	bl_params_ग्रन्थिः->image_info = &bl31_params_mem.bl31_image_info;
	bl_params_ग्रन्थिः->ep_info = &bl31_params_mem.bl31_ep_info;
	bl_params_ग्रन्थिः->अग्र_params_info = &bl31_params_mem.bl32_params_ग्रन्थिः;
	समुच्चयः_PARAM_शीर्ष(bl_params_ग्रन्थिः->image_info, ATF_PARAM_IMAGE_द्विमान,
		       ATF_संस्करणम्_2, 0);

	/* Fill BL32 related information */
	bl_params_ग्रन्थिः = &bl31_params_mem.bl32_params_ग्रन्थिः;
	bl_params_ग्रन्थिः->image_id = ATF_BL32_IMAGE_ID;
	bl_params_ग्रन्थिः->image_info = &bl31_params_mem.bl32_image_info;
	bl_params_ग्रन्थिः->ep_info = &bl31_params_mem.bl32_ep_info;
	bl_params_ग्रन्थिः->अग्र_params_info = &bl31_params_mem.bl33_params_ग्रन्थिः;
	समुच्चयः_PARAM_शीर्ष(bl_params_ग्रन्थिः->ep_info, ATF_PARAM_EP,
		       ATF_संस्करणम्_2, ATF_EP_SECURE);

	/* secure payload is optional, so set pc to 0 if absent */
	bl_params_ग्रन्थिः->ep_info->तर्काः.arg3 = fdt_addr;
	bl_params_ग्रन्थिः->ep_info->pc = bl32_प्रविष्टिः ? bl32_प्रविष्टिः : 0;
	bl_params_ग्रन्थिः->ep_info->spsr = SPSR_64(बहुलकम्_EL1, बहुलकम्_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	समुच्चयः_PARAM_शीर्ष(bl_params_ग्रन्थिः->image_info, ATF_PARAM_IMAGE_द्विमान,
		       ATF_संस्करणम्_2, 0);

	/* Fill BL33 related information */
	bl_params_ग्रन्थिः = &bl31_params_mem.bl33_params_ग्रन्थिः;
	bl_params_ग्रन्थिः->image_id = ATF_BL33_IMAGE_ID;
	bl_params_ग्रन्थिः->image_info = &bl31_params_mem.bl33_image_info;
	bl_params_ग्रन्थिः->ep_info = &bl31_params_mem.bl33_ep_info;
	bl_params_ग्रन्थिः->अग्र_params_info = रिक्त;
	समुच्चयः_PARAM_शीर्ष(bl_params_ग्रन्थिः->ep_info, ATF_PARAM_EP,
		       ATF_संस्करणम्_2, ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl_params_ग्रन्थिः->ep_info->तर्काः.arg0 = 0xffff & read_mpidr();
	bl_params_ग्रन्थिः->ep_info->pc = bl33_प्रविष्टिः;
	bl_params_ग्रन्थिः->ep_info->spsr = SPSR_64(बहुलकम्_EL2, बहुलकम्_SP_ELX,
						DISABLE_ALL_EXCEPTIONS);
	समुच्चयः_PARAM_शीर्ष(bl_params_ग्रन्थिः->image_info, ATF_PARAM_IMAGE_द्विमान,
		       ATF_संस्करणम्_2, 0);

	प्रत्यागम bl_params;
}

__weak संरचना bl_params *bl2_plat_get_bl31_params_v2(ulong bl32_प्रविष्टिः,
						     ulong bl33_प्रविष्टिः,
						     ulong fdt_addr)
{
	प्रत्यागम bl2_plat_get_bl31_params_v2_default(bl32_प्रविष्टिः, bl33_प्रविष्टिः,
						   fdt_addr);
}

स्थिर अन्तःस्थ शून्य raw_write_daif(अचिह्नित पूर्णाङ्क daif)
{
	__asm__ __volatile__("msr DAIF, %x0\n\t" : : "r" (daif) : "memory");
}

प्रकारनिर्देश शून्य __noreturn (*atf_प्रविष्टिः_t)(संरचना bl31_params *params, शून्य *plat_params);

स्थिर शून्य __noreturn bl31_प्रविष्टिः(ulong bl31_प्रविष्टिः, ulong bl32_प्रविष्टिः,
				  ulong bl33_प्रविष्टिः, ulong fdt_addr)
{
	atf_प्रविष्टिः_t  atf_प्रविष्टिः = (atf_प्रविष्टिः_t)bl31_प्रविष्टिः;
	शून्य *bl31_params;

	यदि (CONFIG_IS_ENABLED(ATF_आरोपय_IMAGE_V2))
		bl31_params = bl2_plat_get_bl31_params_v2(bl32_प्रविष्टिः,
							  bl33_प्रविष्टिः,
							  fdt_addr);
	अन्यथा
		bl31_params = bl2_plat_get_bl31_params(bl32_प्रविष्टिः, bl33_प्रविष्टिः,
						       fdt_addr);

	raw_write_daif(SPSR_EXCEPTION_आवरणम्);
	यदि (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_disable();

	atf_प्रविष्टिः(bl31_params, (शून्य *)fdt_addr);
}

स्थिर पूर्णाङ्क spl_fit_images_find(शून्य *blob, पूर्णाङ्क os)
{
	पूर्णाङ्क parent, ग्रन्थिः, ndepth = 0;
	पूर्णाङ्क प्राप्तम् = -FDT_ERR_NOTFOUND;
	ध्रुव शून्य *दत्त;
	ulong val;

	यदि (!blob)
		प्रत्यागम -FDT_ERR_BADMAGIC;

	parent = fdt_मार्गः_विस्थापनम्(blob, "/fit-images");
	यदि (parent < 0)
		प्रत्यागम -FDT_ERR_NOTFOUND;

	प्रति (ग्रन्थिः = fdt_अग्र_ग्रन्थिः(blob, parent, &ndepth);
	     (ग्रन्थिः >= 0) && (ndepth > 0);
	     ग्रन्थिः = fdt_अग्र_ग्रन्थिः(blob, ग्रन्थिः, &ndepth)) {
		यदि (ndepth != 1)
			अनुवर्त;

		दत्त = fdt_getprop(blob, ग्रन्थिः, FIT_OS_PROP, रिक्त);
		यदि (!दत्त)
			अनुवर्त;

		यदि (genimg_get_os_id(दत्त) != os)
			अनुवर्त;

		/*
		 * A multi-segment image, e.g. an OP-TEE ELF split by
		 * binman, is recorded as one node per segment, all with
		 * the same os. Only the segment holding the ELF entry
		 * point carries an entry property, so prefer that node.
		 */
		यदि (!fit_image_get_प्रविष्टिः(blob, ग्रन्थिः, &val))
			प्रत्यागम ग्रन्थिः;

		यदि (प्राप्तम् < 0)
			प्राप्तम् = ग्रन्थिः;
	};

	प्रत्यागम प्राप्तम्;
}

ulong spl_fit_images_get_प्रविष्टिः(शून्य *blob, पूर्णाङ्क ग्रन्थिः)
{
	ulong  val;
	पूर्णाङ्क ret;

	ret = fit_image_get_प्रविष्टिः(blob, ग्रन्थिः, &val);
	यदि (ret)
		ret = fit_image_get_आरोपय(blob, ग्रन्थिः, &val);

	debug("%s: entry point 0x%lx\n", __func__, val);
	प्रत्यागम val;
}

शून्य __noreturn spl_invoke_atf(संरचना spl_image_info *spl_image)
{
	ulong  bl32_प्रविष्टिः = 0;
	ulong  bl33_प्रविष्टिः = CONFIG_TEXT_आधारः;
	शून्य *blob = spl_image->fdt_addr;
	ulong platform_param = (ulong)blob;
	पूर्णाङ्क ग्रन्थिः;

	/*
	 * Find the OP-TEE binary (in /fit-images) load address or
	 * entry point (if different) and pass it as the BL3-2 entry
	 * point, this is optional.
	 */
	ग्रन्थिः = spl_fit_images_find(blob, IH_OS_TEE);
	यदि (ग्रन्थिः >= 0)
		bl32_प्रविष्टिः = spl_fit_images_get_प्रविष्टिः(blob, ग्रन्थिः);

	/*
	 * Find the U-Boot binary (in /fit-images) load addreess or
	 * entry point (if different) and pass it as the BL3-3 entry
	 * point.
	 * This will need to be extended to support Falcon mode.
	 */

	ग्रन्थिः = spl_fit_images_find(blob, IH_OS_U_BOOT);
	यदि (ग्रन्थिः >= 0)
		bl33_प्रविष्टिः = spl_fit_images_get_प्रविष्टिः(blob, ग्रन्थिः);

	/*
	 * If ATF_NO_PLATFORM_PARAM is set, we override the platform
	 * parameter and always pass 0.  This is a workaround for
	 * older ATF versions that have insufficiently robust (or
	 * overzealous) argument validation.
	 */
	यदि (CONFIG_IS_ENABLED(ATF_NO_PLATFORM_PARAM))
		platform_param = 0;

	/*
	 * We don't provide a BL3-2 entry yet, but this will be possible
	 * using similar logic.
	 */
	bl31_प्रविष्टिः(spl_image->प्रविष्टिः_point, bl32_प्रविष्टिः,
		   bl33_प्रविष्टिः, platform_param);
}
