// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Fraunhofer AISEC,
 * Lukas Auer <lukas.auer@aisec.fraunhofer.de>
 *
 * Based on common/spl/spl_atf.c
 */
#ضمن <cpu_func.h>
#ضمن <errno.h>
#ضمن <hang.h>
#ضمن <image.h>
#ضمن <spl.h>
#ضمن <asm/global_data.h>
#ضمن <asm/smp.h>
#ضمن <opensbi.h>
#ضمن <linux/libfdt.h>
#ضمن <linux/printk.h>
#ضمن <mapmem.h>

DECLARE_GLOBAL_معلومات_PTR;

بنية fw_dynamic_info opensbi_info;

ساكن صحيح spl_opensbi_ڳوليو_os_ڳنڍ(فراغ *blob, صحيح *uboot_ڳنڍ, صحيح os_type)
{
	صحيح fit_images_ڳنڍ, ڳنڍ;
	ثابت حرف *fit_os;

	fit_images_ڳنڍ = fdt_رستو_وٿي(blob, "/fit-images");
	إذا (fit_images_ڳنڍ < 0)
		أرجع -ENODEV;

	fdt_for_each_subnode(ڳنڍ, blob, fit_images_ڳنڍ) {
		fit_os = fdt_getprop(blob, ڳنڍ, FIT_OS_PROP, عدم);
		إذا (!fit_os)
			استمر;

		إذا (genimg_get_os_id(fit_os) == os_type) {
			*uboot_ڳنڍ = ڳنڍ;
			أرجع 0;
		}
	}

	أرجع -ENODEV;
}

فراغ __noreturn spl_invoke_opensbi(بنية spl_image_info *spl_image)
{
	صحيح ret, os_ڳنڍ;
	ulong os_داخلا;
	صحيح os_type;
	تعريفنوع فراغ __noreturn (*opensbi_داخلا_t)(ulong hartid, ulong dtb, ulong info);
	opensbi_داخلا_t opensbi_داخلا;

	إذا (!spl_image->fdt_addr) {
		pr_err("No device tree specified in SPL image\n");
		hang();
	}

	إذا (!IS_ALIGNED((uintptr_t)spl_image->fdt_addr, 8)) {
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
#إذا CONFIG_IS_ENABLED(ڀريو_FIT_OPENSBI_OS_BOOT) && \
    CONFIG_VAL(PAYLOAD_دليلون_ADDR)
	يادگيري_نقل((فراغ *)CONFIG_SPL_PAYLOAD_دليلون_ADDR, spl_image->fdt_addr,
	       fdt_totalsize(spl_image->fdt_addr));
	spl_image->fdt_addr = map_sysmem(CONFIG_SPL_PAYLOAD_دليلون_ADDR, 0);
#جيڪڏهن_پورو

	/*
	 * Find next os image in /fit-images
	 * The next os image default is u-boot proper, once enable
	 * OpenSBI OS boot mode, the OS image should be linux.
	 */
	إذا (CONFIG_IS_ENABLED(ڀريو_FIT_OPENSBI_OS_BOOT))
		os_type = IH_OS_LINUX;
	وإلا
		os_type = IH_OS_U_BOOT;

	ret = spl_opensbi_ڳوليو_os_ڳنڍ(spl_image->fdt_addr, &os_ڳنڍ, os_type);
	إذا (ret) {
		pr_err("Can't find %s node for opensbi, %d\n",
		       genimg_get_os_name(os_type), ret);
		hang();
	}

	/* Get U-Boot entry point */
	ret = fit_image_get_داخلا(spl_image->fdt_addr, os_ڳنڍ, &os_داخلا);
	إذا (ret)
		ret = fit_image_get_ڀريو(spl_image->fdt_addr, os_ڳنڍ, &os_داخلا);

	/* Prepare opensbi_info object */
	opensbi_info.magic = FW_DYNAMIC_INFO_MAGIC_قدر;
	opensbi_info.version = FW_DYNAMIC_INFO_نسخو;
	opensbi_info.ايندڙ_addr = os_داخلا;
	opensbi_info.ايندڙ_گهڻائي = FW_DYNAMIC_INFO_ايندڙ_گهڻائي_S;
	opensbi_info.options = CONFIG_SPL_OPENSBI_SCRATCH_OPTIONS;
	opensbi_info.boot_hart = gd->arch.boot_hart;

	opensbi_داخلا = (opensbi_داخلا_t)spl_image->داخلا_نقطو;
	invalidate_icache_all();

#جيڪڏهن_وصف_ٿيل CONFIG_SPL_SMP
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
	ret = smp_call_function((ulong)spl_image->داخلا_نقطو,
				(ulong)spl_image->fdt_addr,
				(ulong)&opensbi_info, 1);
	إذا (ret)
		hang();
#جيڪڏهن_پورو
	opensbi_داخلا(gd->arch.boot_hart, (ulong)spl_image->fdt_addr,
		      (ulong)&opensbi_info);
}
