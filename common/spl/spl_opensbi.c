// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Fraunhofer AISEC,
 * Lukas Auer <lukas.auer@aisec.fraunhofer.de>
 *
 * Based on common/spl/spl_atf.c
 */
#அந்தர்பாவ <cpu_func.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <hang.h>
#அந்தர்பாவ <image.h>
#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <asm/global_data.h>
#அந்தர்பாவ <asm/smp.h>
#அந்தர்பாவ <opensbi.h>
#அந்தர்பாவ <linux/libfdt.h>
#அந்தர்பாவ <linux/printk.h>
#அந்தர்பாவ <mapmem.h>

DECLARE_GLOBAL_DATA_PTR;

கட்டமைப்பு fw_dynamic_info opensbi_info;

நிலையான முழு spl_opensbi_find_os_முனை(வெறுமை *blob, முழு *uboot_முனை, முழு os_type)
{
	முழு fit_images_முனை, முனை;
	மாறிலி எழுத்து *fit_os;

	fit_images_முனை = fdt_path_offset(blob, "/fit-images");
	எனில் (fit_images_முனை < 0)
		திரும்பு -ENODEV;

	fdt_for_each_subnode(முனை, blob, fit_images_முனை) {
		fit_os = fdt_getprop(blob, முனை, FIT_OS_PROP, வெற்று);
		எனில் (!fit_os)
			தொடர்;

		எனில் (genimg_get_os_id(fit_os) == os_type) {
			*uboot_முனை = முனை;
			திரும்பு 0;
		}
	}

	திரும்பு -ENODEV;
}

வெறுமை __noreturn spl_invoke_opensbi(கட்டமைப்பு spl_image_info *spl_image)
{
	முழு ret, os_முனை;
	ulong os_entry;
	முழு os_type;
	வகைவரை வெறுமை __noreturn (*opensbi_entry_t)(ulong hartid, ulong dtb, ulong info);
	opensbi_entry_t opensbi_entry;

	எனில் (!spl_image->fdt_addr) {
		pr_err("No device tree specified in SPL image\n");
		hang();
	}

	எனில் (!IS_ALIGNED((uintptr_t)spl_image->fdt_addr, 8)) {
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
#எனில் CONFIG_IS_ENABLED(LOAD_FIT_OPENSBI_OS_BOOT) && \
    CONFIG_VAL(PAYLOAD_ARGS_ADDR)
	memcpy((வெறுமை *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, spl_image->fdt_addr,
	       fdt_totalsize(spl_image->fdt_addr));
	spl_image->fdt_addr = map_sysmem(CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
#endif

	/*
	 * Find next os image in /fit-images
	 * The next os image default is u-boot proper, once enable
	 * OpenSBI OS boot mode, the OS image should be linux.
	 */
	எனில் (CONFIG_IS_ENABLED(LOAD_FIT_OPENSBI_OS_BOOT))
		os_type = IH_OS_LINUX;
	இல்லையெனில்
		os_type = IH_OS_U_BOOT;

	ret = spl_opensbi_find_os_முனை(spl_image->fdt_addr, &os_முனை, os_type);
	எனில் (ret) {
		pr_err("Can't find %s node for opensbi, %d\n",
		       genimg_get_os_name(os_type), ret);
		hang();
	}

	/* Get U-Boot entry point */
	ret = fit_image_get_entry(spl_image->fdt_addr, os_முனை, &os_entry);
	எனில் (ret)
		ret = fit_image_get_load(spl_image->fdt_addr, os_முனை, &os_entry);

	/* Prepare opensbi_info object */
	opensbi_info.magic = FW_DYNAMIC_INFO_MAGIC_மதிப்பு;
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
	எனில் (ret)
		hang();
#endif
	opensbi_entry(gd->arch.boot_hart, (ulong)spl_image->fdt_addr,
		      (ulong)&opensbi_info);
}
