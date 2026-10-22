// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Fraunhofer AISEC,
 * Lukas Auer <lukas.auer@aisec.fraunhofer.de>
 *
 * Based on common/spl/spl_atf.c
 */
#شامل <cpu_func.h>
#شامل <errno.h>
#شامل <hang.h>
#شامل <image.h>
#شامل <spl.h>
#شامل <asm/global_data.h>
#شامل <asm/smp.h>
#شامل <opensbi.h>
#شامل <linux/libfdt.h>
#شامل <linux/printk.h>
#شامل <mapmem.h>

DECLARE_GLOBAL_ڈیٹا_PTR;

ساخت fw_dynamic_info opensbi_info;

ساکن صحیح spl_opensbi_ڈھونڈو_os_گرہ(خالی *blob, صحیح *uboot_گرہ, صحیح os_type)
{
	صحیح fit_images_گرہ, گرہ;
	ثابت حرف *fit_os;

	fit_images_گرہ = fdt_راستہ_انحراف(blob, "/fit-images");
	اگر (fit_images_گرہ < 0)
		واپس -ENODEV;

	fdt_for_each_subnode(گرہ, blob, fit_images_گرہ) {
		fit_os = fdt_getprop(blob, گرہ, FIT_OS_PROP, تہی);
		اگر (!fit_os)
			جاری;

		اگر (genimg_get_os_id(fit_os) == os_type) {
			*uboot_گرہ = گرہ;
			واپس 0;
		}
	}

	واپس -ENODEV;
}

خالی __noreturn spl_invoke_opensbi(ساخت spl_image_info *spl_image)
{
	صحیح ret, os_گرہ;
	ulong os_اندراج;
	صحیح os_type;
	قسمتعریف خالی __noreturn (*opensbi_اندراج_t)(ulong hartid, ulong dtb, ulong info);
	opensbi_اندراج_t opensbi_اندراج;

	اگر (!spl_image->fdt_addr) {
		pr_err("No device tree specified in SPL image\n");
		hang();
	}

	اگر (!IS_ALIGNED((uintptr_t)spl_image->fdt_addr, 8)) {
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
#اگر CONFIG_IS_ENABLED(بھرو_FIT_OPENSBI_OS_BOOT) && \
    CONFIG_VAL(PAYLOAD_دلائل_ADDR)
	حافظہ_نقل((خالی *)CONFIG_SPL_PAYLOAD_دلائل_ADDR, spl_image->fdt_addr,
	       fdt_totalsize(spl_image->fdt_addr));
	spl_image->fdt_addr = map_sysmem(CONFIG_SPL_PAYLOAD_دلائل_ADDR, 0);
#اگر_ختم

	/*
	 * Find next os image in /fit-images
	 * The next os image default is u-boot proper, once enable
	 * OpenSBI OS boot mode, the OS image should be linux.
	 */
	اگر (CONFIG_IS_ENABLED(بھرو_FIT_OPENSBI_OS_BOOT))
		os_type = IH_OS_LINUX;
	ورنہ
		os_type = IH_OS_U_BOOT;

	ret = spl_opensbi_ڈھونڈو_os_گرہ(spl_image->fdt_addr, &os_گرہ, os_type);
	اگر (ret) {
		pr_err("Can't find %s node for opensbi, %d\n",
		       genimg_get_os_name(os_type), ret);
		hang();
	}

	/* Get U-Boot entry point */
	ret = fit_image_get_اندراج(spl_image->fdt_addr, os_گرہ, &os_اندراج);
	اگر (ret)
		ret = fit_image_get_بھرو(spl_image->fdt_addr, os_گرہ, &os_اندراج);

	/* Prepare opensbi_info object */
	opensbi_info.magic = FW_DYNAMIC_INFO_MAGIC_قدر;
	opensbi_info.version = FW_DYNAMIC_INFO_نسخہ;
	opensbi_info.اگلا_addr = os_اندراج;
	opensbi_info.اگلا_منوال = FW_DYNAMIC_INFO_اگلا_منوال_S;
	opensbi_info.options = CONFIG_SPL_OPENSBI_SCRATCH_OPTIONS;
	opensbi_info.boot_hart = gd->arch.boot_hart;

	opensbi_اندراج = (opensbi_اندراج_t)spl_image->اندراج_نقطہ;
	invalidate_icache_all();

#اگر_تعریف_شدہ CONFIG_SPL_SMP
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
	ret = smp_call_function((ulong)spl_image->اندراج_نقطہ,
				(ulong)spl_image->fdt_addr,
				(ulong)&opensbi_info, 1);
	اگر (ret)
		hang();
#اگر_ختم
	opensbi_اندراج(gd->arch.boot_hart, (ulong)spl_image->fdt_addr,
		      (ulong)&opensbi_info);
}
