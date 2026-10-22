// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Fraunhofer AISEC,
 * Lukas Auer <lukas.auer@aisec.fraunhofer.de>
 *
 * Based on common/spl/spl_atf.c
 */
#അന്തര്ഭാവ <cpu_func.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <hang.h>
#അന്തര്ഭാവ <image.h>
#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <asm/global_data.h>
#അന്തര്ഭാവ <asm/smp.h>
#അന്തര്ഭാവ <opensbi.h>
#അന്തര്ഭാവ <linux/libfdt.h>
#അന്തര്ഭാവ <linux/printk.h>
#അന്തര്ഭാവ <mapmem.h>

DECLARE_GLOBAL_DATA_PTR;

ഘടന fw_dynamic_info opensbi_info;

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_opensbi_find_os_കണ്ണി(ശൂന്യം *blob, പൂർണ്ണസംഖ്യ *uboot_കണ്ണി, പൂർണ്ണസംഖ്യ os_type)
{
	പൂർണ്ണസംഖ്യ fit_images_കണ്ണി, കണ്ണി;
	സ്ഥിരം അക്ഷരം *fit_os;

	fit_images_കണ്ണി = fdt_path_offset(blob, "/fit-images");
	എങ്കിൽ (fit_images_കണ്ണി < 0)
		തിരികെ -ENODEV;

	fdt_for_each_subnode(കണ്ണി, blob, fit_images_കണ്ണി) {
		fit_os = fdt_getprop(blob, കണ്ണി, FIT_OS_PROP, ശൂന്യ);
		എങ്കിൽ (!fit_os)
			തുടരുക;

		എങ്കിൽ (genimg_get_os_id(fit_os) == os_type) {
			*uboot_കണ്ണി = കണ്ണി;
			തിരികെ 0;
		}
	}

	തിരികെ -ENODEV;
}

ശൂന്യം __noreturn spl_invoke_opensbi(ഘടന spl_image_info *spl_image)
{
	പൂർണ്ണസംഖ്യ ret, os_കണ്ണി;
	ulong os_entry;
	പൂർണ്ണസംഖ്യ os_type;
	തരനിർവചനം ശൂന്യം __noreturn (*opensbi_entry_t)(ulong hartid, ulong dtb, ulong info);
	opensbi_entry_t opensbi_entry;

	എങ്കിൽ (!spl_image->fdt_addr) {
		pr_err("No device tree specified in SPL image\n");
		hang();
	}

	എങ്കിൽ (!IS_ALIGNED((uintptr_t)spl_image->fdt_addr, 8)) {
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
#എങ്കിൽ CONFIG_IS_ENABLED(LOAD_FIT_OPENSBI_OS_BOOT) && \
    CONFIG_VAL(PAYLOAD_ARGS_ADDR)
	memcpy((ശൂന്യം *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, spl_image->fdt_addr,
	       fdt_totalsize(spl_image->fdt_addr));
	spl_image->fdt_addr = map_sysmem(CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
#endif

	/*
	 * Find next os image in /fit-images
	 * The next os image default is u-boot proper, once enable
	 * OpenSBI OS boot mode, the OS image should be linux.
	 */
	എങ്കിൽ (CONFIG_IS_ENABLED(LOAD_FIT_OPENSBI_OS_BOOT))
		os_type = IH_OS_LINUX;
	അല്ലെങ്കിൽ
		os_type = IH_OS_U_BOOT;

	ret = spl_opensbi_find_os_കണ്ണി(spl_image->fdt_addr, &os_കണ്ണി, os_type);
	എങ്കിൽ (ret) {
		pr_err("Can't find %s node for opensbi, %d\n",
		       genimg_get_os_name(os_type), ret);
		hang();
	}

	/* Get U-Boot entry point */
	ret = fit_image_get_entry(spl_image->fdt_addr, os_കണ്ണി, &os_entry);
	എങ്കിൽ (ret)
		ret = fit_image_get_load(spl_image->fdt_addr, os_കണ്ണി, &os_entry);

	/* Prepare opensbi_info object */
	opensbi_info.magic = FW_DYNAMIC_INFO_MAGIC_മൂല്യം;
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
	എങ്കിൽ (ret)
		hang();
#endif
	opensbi_entry(gd->arch.boot_hart, (ulong)spl_image->fdt_addr,
		      (ulong)&opensbi_info);
}
