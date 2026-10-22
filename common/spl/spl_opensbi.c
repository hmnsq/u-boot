// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Fraunhofer AISEC,
 * Lukas Auer <lukas.auer@aisec.fraunhofer.de>
 *
 * Based on common/spl/spl_atf.c
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <cpu_func.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <hang.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/smp.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <opensbi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/libfdt.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/printk.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mapmem.h>

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

ਬਣਤਰ fw_dynamic_info opensbi_info;

ਸਥਿਰ ਪੂਰਨ spl_opensbi_ਲੱਭੋ_os_ਗੰਢ(ਖਾਲੀ *blob, ਪੂਰਨ *uboot_ਗੰਢ, ਪੂਰਨ os_type)
{
	ਪੂਰਨ fit_images_ਗੰਢ, ਗੰਢ;
	ਅਚਲ ਅੱਖਰ *fit_os;

	fit_images_ਗੰਢ = fdt_ਰਸਤਾ_ਵਿੱਥ(blob, "/fit-images");
	ਜੇ (fit_images_ਗੰਢ < 0)
		ਵਾਪਸ -ENODEV;

	fdt_for_each_subnode(ਗੰਢ, blob, fit_images_ਗੰਢ) {
		fit_os = fdt_getprop(blob, ਗੰਢ, FIT_OS_PROP, ਨਿਰਾ);
		ਜੇ (!fit_os)
			ਜਾਰੀ;

		ਜੇ (genimg_get_os_id(fit_os) == os_type) {
			*uboot_ਗੰਢ = ਗੰਢ;
			ਵਾਪਸ 0;
		}
	}

	ਵਾਪਸ -ENODEV;
}

ਖਾਲੀ __noreturn spl_invoke_opensbi(ਬਣਤਰ spl_image_info *spl_image)
{
	ਪੂਰਨ ret, os_ਗੰਢ;
	ulong os_ਇੰਦਰਾਜ;
	ਪੂਰਨ os_type;
	ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ __noreturn (*opensbi_ਇੰਦਰਾਜ_t)(ulong hartid, ulong dtb, ulong info);
	opensbi_ਇੰਦਰਾਜ_t opensbi_ਇੰਦਰਾਜ;

	ਜੇ (!spl_image->fdt_addr) {
		pr_err("No device tree specified in SPL image\n");
		hang();
	}

	ਜੇ (!IS_ALIGNED((uintptr_t)spl_image->fdt_addr, 8)) {
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
#ਜੇ CONFIG_IS_ENABLED(ਲੱਦੋ_FIT_OPENSBI_OS_BOOT) && \
    CONFIG_VAL(PAYLOAD_ਦਲੀਲਾਂ_ADDR)
	ਸਮ੍ਰਿਤੀ_ਨਕਲ((ਖਾਲੀ *)CONFIG_SPL_PAYLOAD_ਦਲੀਲਾਂ_ADDR, spl_image->fdt_addr,
	       fdt_totalsize(spl_image->fdt_addr));
	spl_image->fdt_addr = map_sysmem(CONFIG_SPL_PAYLOAD_ਦਲੀਲਾਂ_ADDR, 0);
#ਜੇ_ਅੰਤ

	/*
	 * Find next os image in /fit-images
	 * The next os image default is u-boot proper, once enable
	 * OpenSBI OS boot mode, the OS image should be linux.
	 */
	ਜੇ (CONFIG_IS_ENABLED(ਲੱਦੋ_FIT_OPENSBI_OS_BOOT))
		os_type = IH_OS_LINUX;
	ਨਹੀਂਤਾਂ
		os_type = IH_OS_U_BOOT;

	ret = spl_opensbi_ਲੱਭੋ_os_ਗੰਢ(spl_image->fdt_addr, &os_ਗੰਢ, os_type);
	ਜੇ (ret) {
		pr_err("Can't find %s node for opensbi, %d\n",
		       genimg_get_os_name(os_type), ret);
		hang();
	}

	/* Get U-Boot entry point */
	ret = fit_image_get_ਇੰਦਰਾਜ(spl_image->fdt_addr, os_ਗੰਢ, &os_ਇੰਦਰਾਜ);
	ਜੇ (ret)
		ret = fit_image_get_ਲੱਦੋ(spl_image->fdt_addr, os_ਗੰਢ, &os_ਇੰਦਰਾਜ);

	/* Prepare opensbi_info object */
	opensbi_info.magic = FW_DYNAMIC_INFO_MAGIC_ਮੁੱਲ;
	opensbi_info.version = FW_DYNAMIC_INFO_ਸੰਸਕਰਣ;
	opensbi_info.ਅਗਲਾ_addr = os_ਇੰਦਰਾਜ;
	opensbi_info.ਅਗਲਾ_ਬਹੁਲਕ = FW_DYNAMIC_INFO_ਅਗਲਾ_ਬਹੁਲਕ_S;
	opensbi_info.options = CONFIG_SPL_OPENSBI_SCRATCH_OPTIONS;
	opensbi_info.boot_hart = gd->arch.boot_hart;

	opensbi_ਇੰਦਰਾਜ = (opensbi_ਇੰਦਰਾਜ_t)spl_image->ਇੰਦਰਾਜ_ਬਿੰਦੂ;
	invalidate_icache_all();

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPL_SMP
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
	ret = smp_call_function((ulong)spl_image->ਇੰਦਰਾਜ_ਬਿੰਦੂ,
				(ulong)spl_image->fdt_addr,
				(ulong)&opensbi_info, 1);
	ਜੇ (ret)
		hang();
#ਜੇ_ਅੰਤ
	opensbi_ਇੰਦਰਾਜ(gd->arch.boot_hart, (ulong)spl_image->fdt_addr,
		      (ulong)&opensbi_info);
}
