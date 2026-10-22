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

DECLARE_GLOBAL_DATA_PTR;

بنية fw_dynamic_info opensbi_info;

ساكن صحيح spl_opensbi_find_os_كلي(فراغ *blob, صحيح *uboot_كلي, صحيح os_type)
{
	صحيح fit_images_كلي, كلي;
	ثابت حرف *fit_os;

	fit_images_كلي = fdt_path_offset(blob, "/fit-images");
	إذا (fit_images_كلي < 0)
		أرجع -ENODEV;

	fdt_for_each_subnode(كلي, blob, fit_images_كلي) {
		fit_os = fdt_getprop(blob, كلي, FIT_OS_PROP, عدم);
		إذا (!fit_os)
			استمر;

		إذا (genimg_get_os_id(fit_os) == os_type) {
			*uboot_كلي = كلي;
			أرجع 0;
		}
	}

	أرجع -ENODEV;
}

فراغ __noreturn spl_invoke_opensbi(بنية spl_image_info *spl_image)
{
	صحيح ret, os_كلي;
	ulong os_entry;
	صحيح os_type;
	تعريفنوع فراغ __noreturn (*opensbi_entry_t)(ulong hartid, ulong dtb, ulong info);
	opensbi_entry_t opensbi_entry;

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
#إذا CONFIG_IS_ENABLED(LOAD_FIT_OPENSBI_OS_BOOT) && \
    CONFIG_VAL(PAYLOAD_حجوجي_ADDR)
	memcpy((فراغ *)CONFIG_SPL_PAYLOAD_حجوجي_ADDR, spl_image->fdt_addr,
	       fdt_totalsize(spl_image->fdt_addr));
	spl_image->fdt_addr = map_sysmem(CONFIG_SPL_PAYLOAD_حجوجي_ADDR, 0);
#endif

	/*
	 * Find next os image in /fit-images
	 * The next os image default is u-boot proper, once enable
	 * OpenSBI OS boot mode, the OS image should be linux.
	 */
	إذا (CONFIG_IS_ENABLED(LOAD_FIT_OPENSBI_OS_BOOT))
		os_type = IH_OS_LINUX;
	وإلا
		os_type = IH_OS_U_BOOT;

	ret = spl_opensbi_find_os_كلي(spl_image->fdt_addr, &os_كلي, os_type);
	إذا (ret) {
		pr_err("Can't find %s node for opensbi, %d\n",
		       genimg_get_os_name(os_type), ret);
		hang();
	}

	/* Get U-Boot entry point */
	ret = fit_image_get_entry(spl_image->fdt_addr, os_كلي, &os_entry);
	إذا (ret)
		ret = fit_image_get_load(spl_image->fdt_addr, os_كلي, &os_entry);

	/* Prepare opensbi_info object */
	opensbi_info.magic = FW_DYNAMIC_INFO_MAGIC_درج;
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
	إذا (ret)
		hang();
#endif
	opensbi_entry(gd->arch.boot_hart, (ulong)spl_image->fdt_addr,
		      (ulong)&opensbi_info);
}
