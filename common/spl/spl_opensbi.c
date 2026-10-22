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

स्थिर पूर्णाङ्क spl_opensbi_find_os_ग्रन्थिः(शून्य *blob, पूर्णाङ्क *uboot_ग्रन्थिः, पूर्णाङ्क os_type)
{
	पूर्णाङ्क fit_images_ग्रन्थिः, ग्रन्थिः;
	ध्रुव वर्ण *fit_os;

	fit_images_ग्रन्थिः = fdt_path_offset(blob, "/fit-images");
	जइ (fit_images_ग्रन्थिः < 0)
		प्रत्यागम -ENODEV;

	fdt_for_each_subnode(ग्रन्थिः, blob, fit_images_ग्रन्थिः) {
		fit_os = fdt_getprop(blob, ग्रन्थिः, FIT_OS_PROP, रिक्त);
		जइ (!fit_os)
			अनुवर्त;

		जइ (genimg_get_os_id(fit_os) == os_type) {
			*uboot_ग्रन्थिः = ग्रन्थिः;
			प्रत्यागम 0;
		}
	}

	प्रत्यागम -ENODEV;
}

शून्य __noreturn spl_invoke_opensbi(संरचना spl_image_info *spl_image)
{
	पूर्णाङ्क ret, os_ग्रन्थिः;
	ulong os_entry;
	पूर्णाङ्क os_type;
	प्रकारनिर्देश शून्य __noreturn (*opensbi_entry_t)(ulong hartid, ulong dtb, ulong info);
	opensbi_entry_t opensbi_entry;

	जइ (!spl_image->fdt_addr) {
		pr_err("No device tree specified in SPL image\n");
		hang();
	}

	जइ (!IS_ALIGNED((uintptr_t)spl_image->fdt_addr, 8)) {
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
#जइ CONFIG_IS_ENABLED(LOAD_FIT_OPENSBI_OS_BOOT) && \
    CONFIG_VAL(PAYLOAD_ARGS_ADDR)
	memcpy((शून्य *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, spl_image->fdt_addr,
	       fdt_totalsize(spl_image->fdt_addr));
	spl_image->fdt_addr = map_sysmem(CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
#endif

	/*
	 * Find next os image in /fit-images
	 * The next os image default is u-boot proper, once enable
	 * OpenSBI OS boot mode, the OS image should be linux.
	 */
	जइ (CONFIG_IS_ENABLED(LOAD_FIT_OPENSBI_OS_BOOT))
		os_type = IH_OS_LINUX;
	अण्णहा
		os_type = IH_OS_U_BOOT;

	ret = spl_opensbi_find_os_ग्रन्थिः(spl_image->fdt_addr, &os_ग्रन्थिः, os_type);
	जइ (ret) {
		pr_err("Can't find %s node for opensbi, %d\n",
		       genimg_get_os_name(os_type), ret);
		hang();
	}

	/* Get U-Boot entry point */
	ret = fit_image_get_entry(spl_image->fdt_addr, os_ग्रन्थिः, &os_entry);
	जइ (ret)
		ret = fit_image_get_load(spl_image->fdt_addr, os_ग्रन्थिः, &os_entry);

	/* Prepare opensbi_info object */
	opensbi_info.magic = FW_DYNAMIC_INFO_MAGIC_मानम्;
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
	जइ (ret)
		hang();
#endif
	opensbi_entry(gd->arch.boot_hart, (ulong)spl_image->fdt_addr,
		      (ulong)&opensbi_info);
}
