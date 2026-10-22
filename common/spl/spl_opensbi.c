// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Fraunhofer AISEC,
 * Lukas Auer <lukas.auer@aisec.fraunhofer.de>
 *
 * Based on common/spl/spl_atf.c
 */
#અન્તર્ભાવ <cpu_func.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <hang.h>
#અન્તર્ભાવ <image.h>
#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <asm/global_data.h>
#અન્તર્ભાવ <asm/smp.h>
#અન્તર્ભાવ <opensbi.h>
#અન્તર્ભાવ <linux/libfdt.h>
#અન્તર્ભાવ <linux/printk.h>
#અન્તર્ભાવ <mapmem.h>

DECLARE_GLOBAL_DATA_PTR;

સંરચના fw_dynamic_info opensbi_info;

સ્થિર પૂર્ણાંક spl_opensbi_find_os_ગાંઠ(શૂન્ય *blob, પૂર્ણાંક *uboot_ગાંઠ, પૂર્ણાંક os_type)
{
	પૂર્ણાંક fit_images_ગાંઠ, ગાંઠ;
	અચલ અક્ષર *fit_os;

	fit_images_ગાંઠ = fdt_path_offset(blob, "/fit-images");
	જો (fit_images_ગાંઠ < 0)
		પાછા -ENODEV;

	fdt_for_each_subnode(ગાંઠ, blob, fit_images_ગાંઠ) {
		fit_os = fdt_getprop(blob, ગાંઠ, FIT_OS_PROP, ખાલી);
		જો (!fit_os)
			ચાલુ;

		જો (genimg_get_os_id(fit_os) == os_type) {
			*uboot_ગાંઠ = ગાંઠ;
			પાછા 0;
		}
	}

	પાછા -ENODEV;
}

શૂન્ય __noreturn spl_invoke_opensbi(સંરચના spl_image_info *spl_image)
{
	પૂર્ણાંક ret, os_ગાંઠ;
	ulong os_entry;
	પૂર્ણાંક os_type;
	પ્રકારવ્યાખ્યા શૂન્ય __noreturn (*opensbi_entry_t)(ulong hartid, ulong dtb, ulong info);
	opensbi_entry_t opensbi_entry;

	જો (!spl_image->fdt_addr) {
		pr_err("No device tree specified in SPL image\n");
		hang();
	}

	જો (!IS_ALIGNED((uintptr_t)spl_image->fdt_addr, 8)) {
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
#જો CONFIG_IS_ENABLED(LOAD_FIT_OPENSBI_OS_BOOT) && \
    CONFIG_VAL(PAYLOAD_ARGS_ADDR)
	memcpy((શૂન્ય *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, spl_image->fdt_addr,
	       fdt_totalsize(spl_image->fdt_addr));
	spl_image->fdt_addr = map_sysmem(CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
#endif

	/*
	 * Find next os image in /fit-images
	 * The next os image default is u-boot proper, once enable
	 * OpenSBI OS boot mode, the OS image should be linux.
	 */
	જો (CONFIG_IS_ENABLED(LOAD_FIT_OPENSBI_OS_BOOT))
		os_type = IH_OS_LINUX;
	નહીંતર
		os_type = IH_OS_U_BOOT;

	ret = spl_opensbi_find_os_ગાંઠ(spl_image->fdt_addr, &os_ગાંઠ, os_type);
	જો (ret) {
		pr_err("Can't find %s node for opensbi, %d\n",
		       genimg_get_os_name(os_type), ret);
		hang();
	}

	/* Get U-Boot entry point */
	ret = fit_image_get_entry(spl_image->fdt_addr, os_ગાંઠ, &os_entry);
	જો (ret)
		ret = fit_image_get_load(spl_image->fdt_addr, os_ગાંઠ, &os_entry);

	/* Prepare opensbi_info object */
	opensbi_info.magic = FW_DYNAMIC_INFO_MAGIC_કિંમત;
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
	જો (ret)
		hang();
#endif
	opensbi_entry(gd->arch.boot_hart, (ulong)spl_image->fdt_addr,
		      (ulong)&opensbi_info);
}
