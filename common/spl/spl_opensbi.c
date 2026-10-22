// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Fraunhofer AISEC,
 * Lukas Auer <lukas.auer@aisec.fraunhofer.de>
 *
 * Based on common/spl/spl_atf.c
 */
#ଅନ୍ତର୍ଭାଵ <cpu_func.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <hang.h>
#ଅନ୍ତର୍ଭାଵ <image.h>
#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>
#ଅନ୍ତର୍ଭାଵ <asm/smp.h>
#ଅନ୍ତର୍ଭାଵ <opensbi.h>
#ଅନ୍ତର୍ଭାଵ <linux/libfdt.h>
#ଅନ୍ତର୍ଭାଵ <linux/printk.h>
#ଅନ୍ତର୍ଭାଵ <mapmem.h>

DECLARE_GLOBAL_DATA_PTR;

ସଂରଚନା fw_dynamic_info opensbi_info;

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_opensbi_find_os_ଗଣ୍ଠି(ଶୂନ୍ୟ *blob, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *uboot_ଗଣ୍ଠି, ପୂର୍ଣ୍ଣସଂଖ୍ୟା os_type)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fit_images_ଗଣ୍ଠି, ଗଣ୍ଠି;
	ଅଚଳ ଅକ୍ଷର *fit_os;

	fit_images_ଗଣ୍ଠି = fdt_path_offset(blob, "/fit-images");
	ଯଦି (fit_images_ଗଣ୍ଠି < 0)
		ଫେରନ୍ତୁ -ENODEV;

	fdt_for_each_subnode(ଗଣ୍ଠି, blob, fit_images_ଗଣ୍ଠି) {
		fit_os = fdt_getprop(blob, ଗଣ୍ଠି, FIT_OS_PROP, ଖାଲି);
		ଯଦି (!fit_os)
			ଜାରି;

		ଯଦି (genimg_get_os_id(fit_os) == os_type) {
			*uboot_ଗଣ୍ଠି = ଗଣ୍ଠି;
			ଫେରନ୍ତୁ 0;
		}
	}

	ଫେରନ୍ତୁ -ENODEV;
}

ଶୂନ୍ୟ __noreturn spl_invoke_opensbi(ସଂରଚନା spl_image_info *spl_image)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret, os_ଗଣ୍ଠି;
	ulong os_entry;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା os_type;
	ପ୍ରକାରବ୍ୟାଖ୍ୟା ଶୂନ୍ୟ __noreturn (*opensbi_entry_t)(ulong hartid, ulong dtb, ulong info);
	opensbi_entry_t opensbi_entry;

	ଯଦି (!spl_image->fdt_addr) {
		pr_err("No device tree specified in SPL image\n");
		hang();
	}

	ଯଦି (!IS_ALIGNED((uintptr_t)spl_image->fdt_addr, 8)) {
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
#ଯଦି CONFIG_IS_ENABLED(LOAD_FIT_OPENSBI_OS_BOOT) && \
    CONFIG_VAL(PAYLOAD_ARGS_ADDR)
	memcpy((ଶୂନ୍ୟ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, spl_image->fdt_addr,
	       fdt_totalsize(spl_image->fdt_addr));
	spl_image->fdt_addr = map_sysmem(CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
#endif

	/*
	 * Find next os image in /fit-images
	 * The next os image default is u-boot proper, once enable
	 * OpenSBI OS boot mode, the OS image should be linux.
	 */
	ଯଦି (CONFIG_IS_ENABLED(LOAD_FIT_OPENSBI_OS_BOOT))
		os_type = IH_OS_LINUX;
	ଅନ୍ୟଥା
		os_type = IH_OS_U_BOOT;

	ret = spl_opensbi_find_os_ଗଣ୍ଠି(spl_image->fdt_addr, &os_ଗଣ୍ଠି, os_type);
	ଯଦି (ret) {
		pr_err("Can't find %s node for opensbi, %d\n",
		       genimg_get_os_name(os_type), ret);
		hang();
	}

	/* Get U-Boot entry point */
	ret = fit_image_get_entry(spl_image->fdt_addr, os_ଗଣ୍ଠି, &os_entry);
	ଯଦି (ret)
		ret = fit_image_get_load(spl_image->fdt_addr, os_ଗଣ୍ଠି, &os_entry);

	/* Prepare opensbi_info object */
	opensbi_info.magic = FW_DYNAMIC_INFO_MAGIC_ମୂଲ୍ୟ;
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
	ଯଦି (ret)
		hang();
#endif
	opensbi_entry(gd->arch.boot_hart, (ulong)spl_image->fdt_addr,
		      (ulong)&opensbi_info);
}
