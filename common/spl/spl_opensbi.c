// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Fraunhofer AISEC,
 * Lukas Auer <lukas.auer@aisec.fraunhofer.de>
 *
 * Based on common/spl/spl_atf.c
 */
#অন্তর্ভাৱ <cpu_func.h>
#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <hang.h>
#অন্তর্ভাৱ <image.h>
#অন্তর্ভাৱ <spl.h>
#অন্তর্ভাৱ <asm/global_data.h>
#অন্তর্ভাৱ <asm/smp.h>
#অন্তর্ভাৱ <opensbi.h>
#অন্তর্ভাৱ <linux/libfdt.h>
#অন্তর্ভাৱ <linux/printk.h>
#অন্তর্ভাৱ <mapmem.h>

DECLARE_GLOBAL_DATA_PTR;

সংরচনা fw_dynamic_info opensbi_info;

স্থির পূর্ণাঙ্ক spl_opensbi_find_os_গ্ৰন্থি(শূন্য *blob, পূর্ণাঙ্ক *uboot_গ্ৰন্থি, পূর্ণাঙ্ক os_type)
{
	পূর্ণাঙ্ক fit_images_গ্ৰন্থি, গ্ৰন্থি;
	ধ্রুৱ ৱর্ণ *fit_os;

	fit_images_গ্ৰন্থি = fdt_path_offset(blob, "/fit-images");
	যদি (fit_images_গ্ৰন্থি < 0)
		ঘূৰাই -ENODEV;

	fdt_for_each_subnode(গ্ৰন্থি, blob, fit_images_গ্ৰন্থি) {
		fit_os = fdt_getprop(blob, গ্ৰন্থি, FIT_OS_PROP, রিক্ত);
		যদি (!fit_os)
			অনুৱর্ত;

		যদি (genimg_get_os_id(fit_os) == os_type) {
			*uboot_গ্ৰন্থি = গ্ৰন্থি;
			ঘূৰাই 0;
		}
	}

	ঘূৰাই -ENODEV;
}

শূন্য __noreturn spl_invoke_opensbi(সংরচনা spl_image_info *spl_image)
{
	পূর্ণাঙ্ক ret, os_গ্ৰন্থি;
	ulong os_entry;
	পূর্ণাঙ্ক os_type;
	প্রকারনির্দেশ শূন্য __noreturn (*opensbi_entry_t)(ulong hartid, ulong dtb, ulong info);
	opensbi_entry_t opensbi_entry;

	যদি (!spl_image->fdt_addr) {
		pr_err("No device tree specified in SPL image\n");
		hang();
	}

	যদি (!IS_ALIGNED((uintptr_t)spl_image->fdt_addr, 8)) {
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
#যদি CONFIG_IS_ENABLED(LOAD_FIT_OPENSBI_OS_BOOT) && \
    CONFIG_VAL(PAYLOAD_ARGS_ADDR)
	memcpy((শূন্য *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, spl_image->fdt_addr,
	       fdt_totalsize(spl_image->fdt_addr));
	spl_image->fdt_addr = map_sysmem(CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
#endif

	/*
	 * Find next os image in /fit-images
	 * The next os image default is u-boot proper, once enable
	 * OpenSBI OS boot mode, the OS image should be linux.
	 */
	যদি (CONFIG_IS_ENABLED(LOAD_FIT_OPENSBI_OS_BOOT))
		os_type = IH_OS_LINUX;
	অন্যথা
		os_type = IH_OS_U_BOOT;

	ret = spl_opensbi_find_os_গ্ৰন্থি(spl_image->fdt_addr, &os_গ্ৰন্থি, os_type);
	যদি (ret) {
		pr_err("Can't find %s node for opensbi, %d\n",
		       genimg_get_os_name(os_type), ret);
		hang();
	}

	/* Get U-Boot entry point */
	ret = fit_image_get_entry(spl_image->fdt_addr, os_গ্ৰন্থি, &os_entry);
	যদি (ret)
		ret = fit_image_get_load(spl_image->fdt_addr, os_গ্ৰন্থি, &os_entry);

	/* Prepare opensbi_info object */
	opensbi_info.magic = FW_DYNAMIC_INFO_MAGIC_মান;
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
	যদি (ret)
		hang();
#endif
	opensbi_entry(gd->arch.boot_hart, (ulong)spl_image->fdt_addr,
		      (ulong)&opensbi_info);
}
