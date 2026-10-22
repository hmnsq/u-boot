// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Fraunhofer AISEC,
 * Lukas Auer <lukas.auer@aisec.fraunhofer.de>
 *
 * Based on common/spl/spl_atf.c
 */
#अन्तर्भाव <cpu_func.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <hang.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <asm/smp.h>
#अन्तर्भाव <opensbi.h>
#अन्तर्भाव <linux/libfdt.h>
#अन्तर्भाव <linux/printk.h>
#अन्तर्भाव <mapmem.h>

DECLARE_GLOBAL_DATA_PTR;

संरचना fw_dynamic_info opensbi_info;

स्थिर पूर्णाङ्क spl_opensbi_find_os_गाँठ(शून्य *blob, पूर्णाङ्क *uboot_गाँठ, पूर्णाङ्क os_type)
{
	पूर्णाङ्क fit_images_गाँठ, गाँठ;
	ध्रुव वर्ण *fit_os;

	fit_images_गाँठ = fdt_path_offset(blob, "/fit-images");
	यदि (fit_images_गाँठ < 0)
		प्रत्यागम -ENODEV;

	fdt_for_each_subnode(गाँठ, blob, fit_images_गाँठ) {
		fit_os = fdt_getprop(blob, गाँठ, FIT_OS_PROP, रिक्त);
		यदि (!fit_os)
			अनुवर्त;

		यदि (genimg_get_os_id(fit_os) == os_type) {
			*uboot_गाँठ = गाँठ;
			प्रत्यागम 0;
		}
	}

	प्रत्यागम -ENODEV;
}

शून्य __noreturn spl_invoke_opensbi(संरचना spl_image_info *spl_image)
{
	पूर्णाङ्क ret, os_गाँठ;
	ulong os_entry;
	पूर्णाङ्क os_type;
	प्रकारनिर्देश शून्य __noreturn (*opensbi_entry_t)(ulong hartid, ulong dtb, ulong info);
	opensbi_entry_t opensbi_entry;

	यदि (!spl_image->fdt_addr) {
		pr_err("No device tree specified in SPL image\n");
		hang();
	}

	यदि (!IS_ALIGNED((uintptr_t)spl_image->fdt_addr, 8)) {
		pr_err("SPL image loaded an improperly-aligned device tree\n");
		hang();
	}

	/*
	 * Originally, u-boot-spl will place DTB directly after the kernel,
	 * but the size of the kernel did not include the BSS section, which
	 * means u-boot-spl will place the DTB in the kernel BSS section
	 * causing the DTB to be cleared by kernel BSS initializtion.
	 * Moving DTB in front of the kernel can avoid the error.
	 */
#यदि CONFIG_IS_ENABLED(LOAD_FIT_OPENSBI_OS_BOOT) && \
    CONFIG_VAL(PAYLOAD_ARGS_ADDR)
	स_नकल((शून्य *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, spl_image->fdt_addr,
	       fdt_totalsize(spl_image->fdt_addr));
	spl_image->fdt_addr = map_sysmem(CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
#endif

	/*
	 * Find next os image in /fit-images
	 * The next os image default is u-boot proper, once enable
	 * OpenSBI OS boot mode, the OS image should be linux.
	 */
	यदि (CONFIG_IS_ENABLED(LOAD_FIT_OPENSBI_OS_BOOT))
		os_type = IH_OS_LINUX;
	अन्यथा
		os_type = IH_OS_U_BOOT;

	ret = spl_opensbi_find_os_गाँठ(spl_image->fdt_addr, &os_गाँठ, os_type);
	यदि (ret) {
		pr_err("Can't find %s node for opensbi, %d\n",
		       genimg_get_os_name(os_type), ret);
		hang();
	}

	/* Get U-Boot entry point */
	ret = fit_image_get_entry(spl_image->fdt_addr, os_गाँठ, &os_entry);
	यदि (ret)
		ret = fit_image_get_load(spl_image->fdt_addr, os_गाँठ, &os_entry);

	/* Prepare opensbi_info object */
	opensbi_info.magic = FW_DYNAMIC_INFO_MAGIC_मूल्य;
	opensbi_info.version = FW_DYNAMIC_INFO_VERSION;
	opensbi_info.next_addr = os_entry;
	opensbi_info.next_mode = FW_DYNAMIC_INFO_NEXT_MODE_S;
	opensbi_info.options = CONFIG_SPL_OPENSBI_SCRATCH_OPTIONS;
	opensbi_info.boot_hart = gd->arch.boot_hart;

	opensbi_entry = (opensbi_entry_t)spl_image->entry_point;
	invalidate_icache_all();

#ifdef CONFIG_SPL_SMP
	/*
	 * Start OpenSBI on all secondary harts and wait for acknowledgment.
	 *
	 * OpenSBI first relocates itself to its link address. This is done by
	 * the main hart. To make sure no hart is still running U-Boot SPL
	 * during relocation, we wait for all secondary harts to acknowledge
	 * the call-function request before entering OpenSBI on the main hart.
	 * Otherwise, code corruption can occur if the link address ranges of
	 * U-Boot SPL and OpenSBI overlap.
	 */
	ret = smp_call_function((ulong)spl_image->entry_point,
				(ulong)spl_image->fdt_addr,
				(ulong)&opensbi_info, 1);
	यदि (ret)
		hang();
#endif
	opensbi_entry(gd->arch.boot_hart, (ulong)spl_image->fdt_addr,
		      (ulong)&opensbi_info);
}
