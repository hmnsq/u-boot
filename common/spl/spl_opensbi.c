// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Fraunhofer AISEC,
 * Lukas Auer <lukas.auer@aisec.fraunhofer.de>
 *
 * Based on common/spl/spl_atf.c
 */
#ಅನ್ತರ್ಭಾವ <cpu_func.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <hang.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <asm/smp.h>
#ಅನ್ತರ್ಭಾವ <opensbi.h>
#ಅನ್ತರ್ಭಾವ <linux/libfdt.h>
#ಅನ್ತರ್ಭಾವ <linux/printk.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

ರಚನೆ fw_dynamic_info opensbi_info;

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_opensbi_ಕಂಡುಹಿಡಿ_os_ಗಂಟು(ಶೂನ್ಯ *blob, ಪೂರ್ಣಾಂಕ *uboot_ಗಂಟು, ಪೂರ್ಣಾಂಕ os_type)
{
	ಪೂರ್ಣಾಂಕ fit_images_ಗಂಟು, ಗಂಟು;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fit_os;

	fit_images_ಗಂಟು = fdt_ಮಾರ್ಗ_ಸ್ಥಾನಾಂತರ(blob, "/fit-images");
	ಆದರೆ (fit_images_ಗಂಟು < 0)
		ಹಿಂತಿರುಗಿ -ENODEV;

	fdt_for_each_subnode(ಗಂಟು, blob, fit_images_ಗಂಟು) {
		fit_os = fdt_getprop(blob, ಗಂಟು, FIT_OS_PROP, ಖಾಲಿ);
		ಆದರೆ (!fit_os)
			ಮುಂದುವರಿ;

		ಆದರೆ (genimg_get_os_id(fit_os) == os_type) {
			*uboot_ಗಂಟು = ಗಂಟು;
			ಹಿಂತಿರುಗಿ 0;
		}
	}

	ಹಿಂತಿರುಗಿ -ENODEV;
}

ಶೂನ್ಯ __noreturn spl_invoke_opensbi(ರಚನೆ spl_image_info *spl_image)
{
	ಪೂರ್ಣಾಂಕ ret, os_ಗಂಟು;
	ulong os_ನಮೂದು;
	ಪೂರ್ಣಾಂಕ os_type;
	ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ __noreturn (*opensbi_ನಮೂದು_t)(ulong hartid, ulong dtb, ulong info);
	opensbi_ನಮೂದು_t opensbi_ನಮೂದು;

	ಆದರೆ (!spl_image->fdt_addr) {
		pr_err("No device tree specified in SPL image\n");
		hang();
	}

	ಆದರೆ (!IS_ALIGNED((uintptr_t)spl_image->fdt_addr, 8)) {
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
#ಆದರೆ CONFIG_IS_ENABLED(ತುಂಬು_FIT_OPENSBI_OS_BOOT) && \
    CONFIG_VAL(PAYLOAD_ವಾದಗಳು_ADDR)
	ಸ್ಮರಣೆ_ನಕಲು((ಶೂನ್ಯ *)CONFIG_SPL_PAYLOAD_ವಾದಗಳು_ADDR, spl_image->fdt_addr,
	       fdt_totalsize(spl_image->fdt_addr));
	spl_image->fdt_addr = map_sysmem(CONFIG_SPL_PAYLOAD_ವಾದಗಳು_ADDR, 0);
#ಆದರೆ_ಅಂತ್ಯ

	/*
	 * Find next os image in /fit-images
	 * The next os image default is u-boot proper, once enable
	 * OpenSBI OS boot mode, the OS image should be linux.
	 */
	ಆದರೆ (CONFIG_IS_ENABLED(ತುಂಬು_FIT_OPENSBI_OS_BOOT))
		os_type = IH_OS_LINUX;
	ಇಲ್ಲದಿದ್ದರೆ
		os_type = IH_OS_U_BOOT;

	ret = spl_opensbi_ಕಂಡುಹಿಡಿ_os_ಗಂಟು(spl_image->fdt_addr, &os_ಗಂಟು, os_type);
	ಆದರೆ (ret) {
		pr_err("Can't find %s node for opensbi, %d\n",
		       genimg_get_os_name(os_type), ret);
		hang();
	}

	/* Get U-Boot entry point */
	ret = fit_image_get_ನಮೂದು(spl_image->fdt_addr, os_ಗಂಟು, &os_ನಮೂದು);
	ಆದರೆ (ret)
		ret = fit_image_get_ತುಂಬು(spl_image->fdt_addr, os_ಗಂಟು, &os_ನಮೂದು);

	/* Prepare opensbi_info object */
	opensbi_info.magic = FW_DYNAMIC_INFO_MAGIC_ಮೌಲ್ಯ;
	opensbi_info.version = FW_DYNAMIC_INFO_ಆವೃತ್ತಿ;
	opensbi_info.ಮುಂದಿನ_addr = os_ನಮೂದು;
	opensbi_info.ಮುಂದಿನ_ಬಹುಲಕ = FW_DYNAMIC_INFO_ಮುಂದಿನ_ಬಹುಲಕ_S;
	opensbi_info.options = CONFIG_SPL_OPENSBI_SCRATCH_OPTIONS;
	opensbi_info.boot_hart = gd->arch.boot_hart;

	opensbi_ನಮೂದು = (opensbi_ನಮೂದು_t)spl_image->ನಮೂದು_ಬಿಂದು;
	invalidate_icache_all();

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPL_SMP
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
	ret = smp_call_function((ulong)spl_image->ನಮೂದು_ಬಿಂದು,
				(ulong)spl_image->fdt_addr,
				(ulong)&opensbi_info, 1);
	ಆದರೆ (ret)
		hang();
#ಆದರೆ_ಅಂತ್ಯ
	opensbi_ನಮೂದು(gd->arch.boot_hart, (ulong)spl_image->fdt_addr,
		      (ulong)&opensbi_info);
}
