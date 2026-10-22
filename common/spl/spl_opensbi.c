// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Fraunhofer AISEC,
 * Lukas Auer <lukas.auer@aisec.fraunhofer.de>
 *
 * Based on common/spl/spl_atf.c
 */
#అన్తర్భావ <cpu_func.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <hang.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <spl.h>
#అన్తర్భావ <asm/global_data.h>
#అన్తర్భావ <asm/smp.h>
#అన్తర్భావ <opensbi.h>
#అన్తర్భావ <linux/libfdt.h>
#అన్తర్భావ <linux/printk.h>
#అన్తర్భావ <mapmem.h>

DECLARE_GLOBAL_దత్తాంశం_PTR;

నిర్మాణం fw_dynamic_info opensbi_info;

స్థిర పూర్ణాంకం spl_opensbi_కనుగొను_os_కణుపు(శూన్య *blob, పూర్ణాంకం *uboot_కణుపు, పూర్ణాంకం os_type)
{
	పూర్ణాంకం fit_images_కణుపు, కణుపు;
	స్థిరరాశి అక్షరం *fit_os;

	fit_images_కణుపు = fdt_మార్గం_స్థానభ్రంశం(blob, "/fit-images");
	ఒకవేళ (fit_images_కణుపు < 0)
		తిరిగి -ENODEV;

	fdt_for_each_subnode(కణుపు, blob, fit_images_కణుపు) {
		fit_os = fdt_getprop(blob, కణుపు, FIT_OS_PROP, ఖాళీ);
		ఒకవేళ (!fit_os)
			కొనసాగు;

		ఒకవేళ (genimg_get_os_id(fit_os) == os_type) {
			*uboot_కణుపు = కణుపు;
			తిరిగి 0;
		}
	}

	తిరిగి -ENODEV;
}

శూన్య __noreturn spl_invoke_opensbi(నిర్మాణం spl_image_info *spl_image)
{
	పూర్ణాంకం ret, os_కణుపు;
	ulong os_నమోదు;
	పూర్ణాంకం os_type;
	రకనిర్వచనం శూన్య __noreturn (*opensbi_నమోదు_t)(ulong hartid, ulong dtb, ulong info);
	opensbi_నమోదు_t opensbi_నమోదు;

	ఒకవేళ (!spl_image->fdt_addr) {
		pr_err("No device tree specified in SPL image\n");
		hang();
	}

	ఒకవేళ (!IS_ALIGNED((uintptr_t)spl_image->fdt_addr, 8)) {
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
#ఒకవేళ CONFIG_IS_ENABLED(ఎక్కించు_FIT_OPENSBI_OS_BOOT) && \
    CONFIG_VAL(PAYLOAD_వాదనలు_ADDR)
	జ్ఞాపకం_నకలు((శూన్య *)CONFIG_SPL_PAYLOAD_వాదనలు_ADDR, spl_image->fdt_addr,
	       fdt_totalsize(spl_image->fdt_addr));
	spl_image->fdt_addr = map_sysmem(CONFIG_SPL_PAYLOAD_వాదనలు_ADDR, 0);
#అయితే_ముగింపు

	/*
	 * Find next os image in /fit-images
	 * The next os image default is u-boot proper, once enable
	 * OpenSBI OS boot mode, the OS image should be linux.
	 */
	ఒకవేళ (CONFIG_IS_ENABLED(ఎక్కించు_FIT_OPENSBI_OS_BOOT))
		os_type = IH_OS_LINUX;
	లేకపోతే
		os_type = IH_OS_U_BOOT;

	ret = spl_opensbi_కనుగొను_os_కణుపు(spl_image->fdt_addr, &os_కణుపు, os_type);
	ఒకవేళ (ret) {
		pr_err("Can't find %s node for opensbi, %d\n",
		       genimg_get_os_name(os_type), ret);
		hang();
	}

	/* Get U-Boot entry point */
	ret = fit_image_get_నమోదు(spl_image->fdt_addr, os_కణుపు, &os_నమోదు);
	ఒకవేళ (ret)
		ret = fit_image_get_ఎక్కించు(spl_image->fdt_addr, os_కణుపు, &os_నమోదు);

	/* Prepare opensbi_info object */
	opensbi_info.magic = FW_DYNAMIC_INFO_MAGIC_విలువ;
	opensbi_info.version = FW_DYNAMIC_INFO_సంస్కరణ;
	opensbi_info.తదుపరి_addr = os_నమోదు;
	opensbi_info.తదుపరి_బహుళకం = FW_DYNAMIC_INFO_తదుపరి_బహుళకం_S;
	opensbi_info.options = CONFIG_SPL_OPENSBI_SCRATCH_OPTIONS;
	opensbi_info.boot_hart = gd->arch.boot_hart;

	opensbi_నమోదు = (opensbi_నమోదు_t)spl_image->నమోదు_బిందువు;
	invalidate_icache_all();

#నిర్వచితమైతే CONFIG_SPL_SMP
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
	ret = smp_call_function((ulong)spl_image->నమోదు_బిందువు,
				(ulong)spl_image->fdt_addr,
				(ulong)&opensbi_info, 1);
	ఒకవేళ (ret)
		hang();
#అయితే_ముగింపు
	opensbi_నమోదు(gd->arch.boot_hart, (ulong)spl_image->fdt_addr,
		      (ulong)&opensbi_info);
}
