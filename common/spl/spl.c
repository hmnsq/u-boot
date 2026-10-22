// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */

#ಅನ್ತರ್ಭಾವ <config.h>
#ಅನ್ತರ್ಭಾವ <bloblist.h>
#ಅನ್ತರ್ಭಾವ <binman_sym.h>
#ಅನ್ತರ್ಭಾವ <bootstage.h>
#ಅನ್ತರ್ಭಾವ <dm.h>
#ಅನ್ತರ್ಭಾವ <handoff.h>
#ಅನ್ತರ್ಭಾವ <hang.h>
#ಅನ್ತರ್ಭಾವ <init.h>
#ಅನ್ತರ್ಭಾವ <irq_func.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <of_live.h>
#ಅನ್ತರ್ಭಾವ <serial.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <spl_load.h>
#ಅನ್ತರ್ಭಾವ <system-constants.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <asm-generic/gpio.h>
#ಅನ್ತರ್ಭಾವ <nand.h>
#ಅನ್ತರ್ಭಾವ <fat.h>
#ಅನ್ತರ್ಭಾವ <u-boot/crc.h>
#ಆದರೆ CONFIG_IS_ENABLED(BANNER_PRINT)
#ಅನ್ತರ್ಭಾವ <timestamp.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <version.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <dm/root.h>
#ಅನ್ತರ್ಭಾವ <dm/util.h>
#ಅನ್ತರ್ಭಾವ <dm/device-internal.h>
#ಅನ್ತರ್ಭಾವ <dm/uclass-internal.h>
#ಅನ್ತರ್ಭಾವ <linux/compiler.h>
#ಅನ್ತರ್ಭಾವ <fdt_support.h>
#ಅನ್ತರ್ಭಾವ <bootcount.h>
#ಅನ್ತರ್ಭಾವ <wdt.h>
#ಅನ್ತರ್ಭಾವ <video.h>

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;
DECLARE_BINMAN_MAGIC_SYM;

u32 *boot_params_ptr = ಖಾಲಿ;

#ಆದರೆ CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS)
/* See spl.h for information about this */
#ಆದರೆ defined(CONFIG_SPL_BUILD)
binman_sym_declare(ulong, u_boot_any, image_pos);
binman_sym_declare(ulong, u_boot_any, ಗಾತ್ರ_2);
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_TPL
binman_sym_declare(ulong, u_boot_spl_any, image_pos);
binman_sym_declare(ulong, u_boot_spl_any, ಗಾತ್ರ_2);
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_VPL
binman_sym_declare(ulong, u_boot_vpl_any, image_pos);
binman_sym_declare(ulong, u_boot_vpl_any, ಗಾತ್ರ_2);
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ /* BINMAN_UBOOT_SYMBOLS */

/* Define board data structure */
ಸ್ಥಿರ ರಚನೆ bd_info bdata __attribute__ ((section(".data")));

#ಆದರೆ CONFIG_IS_ENABLED(SHOW_BOOT_ಪ್ರಗತಿ)
/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
__weak ಶೂನ್ಯ show_boot_ಪ್ರಗತಿ(ಪೂರ್ಣಾಂಕ val) {}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) || \
	defined(CONFIG_SPL_ATF)
/* weak, default platform-specific function to initialize dram banks */
__weak ಪೂರ್ಣಾಂಕ dram_init_banksize(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
 * Default function to determine if u-boot or the OS should
 * be started. This implementation always returns 1.
 *
 * Please implement your own board specific funcion to do this.
 *
 * RETURN
 * 0 to not start u-boot
 * positive if u-boot should start
 */
#ಆದರೆ CONFIG_IS_ENABLED(OS_BOOT)
__weak ಪೂರ್ಣಾಂಕ spl_ಆರಂಭ_uboot(ಶೂನ್ಯ)
{
	puts(PHASE_PROMPT
	     "Please implement spl_start_uboot() for your board\n");
	puts(PHASE_PROMPT "Direct Linux boot not active!\n");
	ಹಿಂತಿರುಗಿ 1;
}

/*
 * Weak default function for arch specific zImage check. Return zero
 * and fill start and end address if image is recognized.
 */
ಪೂರ್ಣಾಂಕ __weak bootz_ಸಿದ್ಧತೆ(ulong image, ulong *ಆರಂಭ, ulong *ಕೊನೆ_2)
{
	 ಹಿಂತಿರುಗಿ 1;
}

ಪೂರ್ಣಾಂಕ __weak booti_ಸಿದ್ಧತೆ(ulong image, ulong *relocated_addr, ulong *ಗಾತ್ರ_2, bool force_reloc)
{
	 ಹಿಂತಿರುಗಿ 1;
}
#ಆದರೆ_ಅಂತ್ಯ

/* Weak default function for arch specific fixups to the spl_image_info */
ಶೂನ್ಯ __weak spl_perform_arch_fixups(ರಚನೆ spl_image_info *spl_image)
{
}

/* Weak default function for board specific fixups to the spl_image_info */
ಶೂನ್ಯ __weak spl_perform_board_fixups(ರಚನೆ spl_image_info *spl_image)
{
}

ಶೂನ್ಯ spl_fixup_fdt(ಶೂನ್ಯ *fdt_blob)
{
#ಆದರೆ defined(CONFIG_SPL_ದ_LIBFDT)
	ಪೂರ್ಣಾಂಕ err;

	ಆದರೆ (!fdt_blob)
		ಹಿಂತಿರುಗಿ;

	err = fdt_check_ಶೀರ್ಷಿಕೆ(fdt_blob);
	ಆದರೆ (err < 0) {
		ಬರೆ("fdt_root: %s\n", fdt_strerror(err));
		ಹಿಂತಿರುಗಿ;
	}

	/* fixup the memory dt node */
	err = fdt_shrink_ಗಮ್ಯ_minimum(fdt_blob, 0);
	ಆದರೆ (err == 0) {
		ಬರೆ(PHASE_PROMPT "fdt_shrink_to_minimum err - %d\n", err);
		ಹಿಂತಿರುಗಿ;
	}

	err = arch_fixup_fdt(fdt_blob);
	ಆದರೆ (err) {
		ಬರೆ(PHASE_PROMPT "arch_fixup_fdt err - %d\n", err);
		ಹಿಂತಿರುಗಿ;
	}
#ಆದರೆ_ಅಂತ್ಯ
}

ಪೂರ್ಣಾಂಕ spl_reserve_video_from_ram_ತುದಿ(ಶೂನ್ಯ)
{
	ಆದರೆ (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		ಪೂರ್ಣಾಂಕ ret;

		addr = gd->ram_ತುದಿ;
		ret = video_reserve(&addr);
		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	ಹಿಂತಿರುಗಿ 0;
}

ulong spl_get_image_pos(ಶೂನ್ಯ)
{
	ಆದರೆ (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		ಹಿಂತಿರುಗಿ BINMAN_SYM_MISSING;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_VPL
	ಆದರೆ (xpl_ಮುಂದಿನ_phase() == PHASE_VPL)
		ಹಿಂತಿರುಗಿ binman_sym(ulong, u_boot_vpl_any, image_pos);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(CONFIG_TPL) && !defined(CONFIG_VPL)
	ಆದರೆ (xpl_ಮುಂದಿನ_phase() == PHASE_SPL)
		ಹಿಂತಿರುಗಿ binman_sym(ulong, u_boot_spl_any, image_pos);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(CONFIG_SPL_BUILD)
	ಹಿಂತಿರುಗಿ binman_sym(ulong, u_boot_any, image_pos);
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ BINMAN_SYM_MISSING;
}

ulong spl_get_image_ಗಾತ್ರ(ಶೂನ್ಯ)
{
	ಆದರೆ (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		ಹಿಂತಿರುಗಿ BINMAN_SYM_MISSING;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_VPL
	ಆದರೆ (xpl_ಮುಂದಿನ_phase() == PHASE_VPL)
		ಹಿಂತಿರುಗಿ binman_sym(ulong, u_boot_vpl_any, ಗಾತ್ರ_2);
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ xpl_ಮುಂದಿನ_phase() == PHASE_SPL ?
		binman_sym(ulong, u_boot_spl_any, ಗಾತ್ರ_2) :
		binman_sym(ulong, u_boot_any, ಗಾತ್ರ_2);
}

ulong spl_get_image_ಪಠ್ಯ_ತಳ(ಶೂನ್ಯ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_VPL
	ಆದರೆ (xpl_ಮುಂದಿನ_phase() == PHASE_VPL)
		ಹಿಂತಿರುಗಿ CONFIG_VPL_ಪಠ್ಯ_ತಳ;
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ xpl_ಮುಂದಿನ_phase() == PHASE_SPL ? CONFIG_SPL_ಪಠ್ಯ_ತಳ :
		CONFIG_ಪಠ್ಯ_ತಳ;
}

/*
 * Weak default function for board specific cleanup/preparation before
 * Linux boot. Some boards/platforms might not need it, so just provide
 * an empty stub here.
 */
__weak ಶೂನ್ಯ spl_board_prepare_for_linux(ಶೂನ್ಯ)
{
	/* Nothing to do! */
}

__weak ಶೂನ್ಯ spl_board_prepare_for_optee(ಶೂನ್ಯ *fdt)
{
}

__weak ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *spl_board_loader_name(u32 boot_device)
{
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

#ಆದರೆ CONFIG_IS_ENABLED(OPTEE_IMAGE)
__weak ಶೂನ್ಯ __noreturn jump_ಗಮ್ಯ_image_optee(ರಚನೆ spl_image_info *spl_image)
{
	spl_optee_ನಮೂದು(ಖಾಲಿ, ಖಾಲಿ, spl_image->fdt_addr,
			(ಶೂನ್ಯ *)spl_image->ನಮೂದು_ಬಿಂದು);
}
#ಆದರೆ_ಅಂತ್ಯ

__weak ಶೂನ್ಯ spl_board_prepare_for_boot(ಶೂನ್ಯ)
{
	/* Nothing to do! */
}

__weak ರಚನೆ legacy_img_hdr *spl_get_ತುಂಬು_ಬಫರ್(ssize_t ಸ್ಥಾನಾಂತರ, size_t ಗಾತ್ರ_2)
{
	ಹಿಂತಿರುಗಿ map_sysmem(CONFIG_ಪಠ್ಯ_ತಳ + ಸ್ಥಾನಾಂತರ, 0);
}

ಶೂನ್ಯ spl_ಗಣ_ಶೀರ್ಷಿಕೆ_raw_uboot(ರಚನೆ spl_image_info *spl_image)
{
	ulong u_boot_pos = spl_get_image_pos();

#ಆದರೆ CONFIG_SYS_MONITOR_LEN != 0
	spl_image->ಗಾತ್ರ_2 = CONFIG_SYS_MONITOR_LEN;
#ಇಲ್ಲದಿದ್ದರೆ
	/* Unknown U-Boot size, let's assume it will not be more than 200 KB */
	spl_image->ಗಾತ್ರ_2 = 200 * 1024;
#ಆದರೆ_ಅಂತ್ಯ

	/*
	 * Binman error cases: address of the end of the previous region or the
	 * start of the image's entry area (usually 0) if there is no previous
	 * region.
	 */
	ಆದರೆ (u_boot_pos && u_boot_pos != BINMAN_SYM_MISSING) {
		/* Binman does not support separated entry addresses */
		spl_image->ನಮೂದು_ಬಿಂದು = spl_get_image_ಪಠ್ಯ_ತಳ();
		spl_image->ತುಂಬು_addr = spl_get_image_ಪಠ್ಯ_ತಳ();
		spl_image->ಗಾತ್ರ_2 = spl_get_image_ಗಾತ್ರ();
		log_debug("Next load addr %lx\n", spl_image->ತುಂಬು_addr);
	} ಇಲ್ಲದಿದ್ದರೆ {
		spl_image->ನಮೂದು_ಬಿಂದು = CONFIG_SYS_UBOOT_ಆರಂಭ;
		spl_image->ತುಂಬು_addr = CONFIG_ಪಠ್ಯ_ತಳ;
		log_debug("Default load addr %lx (u_boot_pos=%lx)\n",
			  spl_image->ತುಂಬು_addr, u_boot_pos);
	}
	spl_image->os = IH_OS_U_BOOT;
	spl_image->name = xpl_name(xpl_ಮುಂದಿನ_phase());
	log_debug("Next phase: %s at %lx size %lx\n", spl_image->name,
		  spl_image->ತುಂಬು_addr, (ulong)spl_image->ಗಾತ್ರ_2);
}

__weak ಪೂರ್ಣಾಂಕ spl_parse_board_ಶೀರ್ಷಿಕೆ(ರಚನೆ spl_image_info *spl_image,
				  ಸ್ಥಿರಾಂಕ ರಚನೆ spl_boot_device *bootdev,
				  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *image_ಶೀರ್ಷಿಕೆ, size_t ಗಾತ್ರ_2)
{
	ಹಿಂತಿರುಗಿ -EINVAL;
}

__weak ಪೂರ್ಣಾಂಕ spl_parse_legacy_ಶೀರ್ಷಿಕೆ(ರಚನೆ spl_image_info *spl_image,
				   ಸ್ಥಿರಾಂಕ ರಚನೆ legacy_img_hdr *ಶೀರ್ಷಿಕೆ)
{
	/* LEGACY image not supported */
	debug("Legacy boot image support not enabled, proceeding to other boot methods\n");
	ಹಿಂತಿರುಗಿ -EINVAL;
}

ಪೂರ್ಣಾಂಕ spl_parse_image_ಶೀರ್ಷಿಕೆ(ರಚನೆ spl_image_info *spl_image,
			   ಸ್ಥಿರಾಂಕ ರಚನೆ spl_boot_device *bootdev,
			   ಸ್ಥಿರಾಂಕ ರಚನೆ legacy_img_hdr *ಶೀರ್ಷಿಕೆ)
{
	ಪೂರ್ಣಾಂಕ ret;

	ಆದರೆ (CONFIG_IS_ENABLED(ತುಂಬು_FIT_ತುಂಬಿದ)) {
		ret = spl_ತುಂಬು_fit_image(spl_image, ಶೀರ್ಷಿಕೆ);

		ಆದರೆ (!ret)
			ಹಿಂತಿರುಗಿ ret;
	}
	ಆದರೆ (image_get_magic(ಶೀರ್ಷಿಕೆ) == IH_MAGIC) {
		ಪೂರ್ಣಾಂಕ ret;

		ret = spl_parse_legacy_ಶೀರ್ಷಿಕೆ(spl_image, ಶೀರ್ಷಿಕೆ);
		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ ret;
		ಹಿಂತಿರುಗಿ 0;
	}

	ಆದರೆ (IS_ENABLED(CONFIG_SPL_PANIC_ON_RAW_IMAGE)) {
		/*
		 * CONFIG_SPL_PANIC_ON_RAW_IMAGE is defined when the
		 * code which loads images in SPL cannot guarantee that
		 * absolutely all read errors will be reported.
		 * An example is the LPC32XX MLC NAND driver, which
		 * will consider that a completely unreadable NAND block
		 * is bad, and thus should be skipped silently.
		 */
		panic("** no mkimage signature but raw image not supported");
	}

	ಆದರೆ (IS_ENABLED(CONFIG_SPL_OS_BOOT) && IS_ENABLED(CONFIG_SPL_BOOTI)) {
		ulong ಆರಂಭ, ಗಾತ್ರ_2;

		ಆದರೆ (!booti_ಸಿದ್ಧತೆ((ulong)ಶೀರ್ಷಿಕೆ, &ಆರಂಭ, &ಗಾತ್ರ_2, 0)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->ತುಂಬು_addr = ಆರಂಭ;
			spl_image->ನಮೂದು_ಬಿಂದು = ಆರಂಭ;
			spl_image->ಗಾತ್ರ_2 = ಗಾತ್ರ_2;
			debug(PHASE_PROMPT
			      "payload Image, load addr: 0x%lx size: %d\n",
			      spl_image->ತುಂಬು_addr, spl_image->ಗಾತ್ರ_2);
			ಹಿಂತಿರುಗಿ 0;
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (IS_ENABLED(CONFIG_SPL_OS_BOOT) &&
		   IS_ENABLED(CONFIG_SPL_BOOTZ)) {
		ulong ಆರಂಭ, ಕೊನೆ_2;

		ಆದರೆ (!bootz_ಸಿದ್ಧತೆ((ulong)ಶೀರ್ಷಿಕೆ, &ಆರಂಭ, &ಕೊನೆ_2)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->ತುಂಬು_addr = CONFIG_SYS_ತುಂಬು_ADDR;
			spl_image->ನಮೂದು_ಬಿಂದು = CONFIG_SYS_ತುಂಬು_ADDR;
			spl_image->ಗಾತ್ರ_2 = ಕೊನೆ_2 - ಆರಂಭ;
			debug(PHASE_PROMPT
			      "payload zImage, load addr: 0x%lx size: %d\n",
			      spl_image->ತುಂಬು_addr, spl_image->ಗಾತ್ರ_2);
			ಹಿಂತಿರುಗಿ 0;
		}
	}

	ಆದರೆ (!spl_parse_board_ಶೀರ್ಷಿಕೆ(spl_image, bootdev, (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *)ಶೀರ್ಷಿಕೆ,
				    ಗಾತ್ರ(*ಶೀರ್ಷಿಕೆ)))
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (IS_ENABLED(CONFIG_SPL_RAW_IMAGE_SUPPORT)) {
		/* Signature not found - assume u-boot.bin */
		debug("mkimage signature not found - ih_magic = %x\n",
		      ಶೀರ್ಷಿಕೆ->ih_magic);
		spl_ಗಣ_ಶೀರ್ಷಿಕೆ_raw_uboot(spl_image);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* RAW image not supported, proceed to other boot methods. */
		debug("Raw boot image support not enabled, proceeding to other boot methods\n");
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	ಹಿಂತಿರುಗಿ 0;
}

#ಆದರೆ SPL_ತುಂಬು_USERS > 1
ಪೂರ್ಣಾಂಕ spl_ತುಂಬು(ರಚನೆ spl_image_info *spl_image,
	     ಸ್ಥಿರಾಂಕ ರಚನೆ spl_boot_device *bootdev, ರಚನೆ spl_ತುಂಬು_info *info,
	     size_t ಗಾತ್ರ_2, size_t ಸ್ಥಾನಾಂತರ)
{
	ಹಿಂತಿರುಗಿ spl_ತುಂಬು_2(spl_image, bootdev, info, ಗಾತ್ರ_2, ಸ್ಥಾನಾಂತರ);
}
#ಆದರೆ_ಅಂತ್ಯ

__weak ಶೂನ್ಯ __noreturn jump_ಗಮ್ಯ_image(ರಚನೆ spl_image_info *spl_image)
{
	ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ __noreturn (*image_ನಮೂದು_noargs_t)(ಶೂನ್ಯ);

	image_ನಮೂದು_noargs_t image_ನಮೂದು =
		(image_ನಮೂದು_noargs_t)spl_image->ನಮೂದು_ಬಿಂದು;

	debug("image entry point: 0x%lx\n", spl_image->ನಮೂದು_ಬಿಂದು);
	image_ನಮೂದು();
}

#ಆದರೆ CONFIG_IS_ENABLED(HANDOFF)
/**
 * Set up the SPL hand-off information
 *
 * This is initially empty (zero) but can be written by
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಿದ್ಧತೆ_spl_handoff(ಶೂನ್ಯ)
{
	ರಚನೆ spl_handoff *ho;

	ho = bloblist_ensure(BLOBLISTT_U_BOOT_SPL_HANDOFF, ಗಾತ್ರ(ರಚನೆ spl_handoff));
	ಆದರೆ (!ho)
		ಹಿಂತಿರುಗಿ -ENOENT;

	ಹಿಂತಿರುಗಿ 0;
}

__weak ಪೂರ್ಣಾಂಕ handoff_arch_save(ರಚನೆ spl_handoff *ho)
{
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ write_spl_handoff(ಶೂನ್ಯ)
{
	ರಚನೆ spl_handoff *ho;
	ಪೂರ್ಣಾಂಕ ret;

	ho = bloblist_ಕಂಡುಹಿಡಿ(BLOBLISTT_U_BOOT_SPL_HANDOFF, ಗಾತ್ರ(ರಚನೆ spl_handoff));
	ಆದರೆ (!ho)
		ಹಿಂತಿರುಗಿ -ENOENT;
	handoff_save_dram(ho);
	ret = handoff_arch_save(ho);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;
	debug(PHASE_PROMPT "Wrote SPL handoff\n");

	ಹಿಂತಿರುಗಿ 0;
}
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ ಸಿದ್ಧತೆ_spl_handoff(ಶೂನ್ಯ) { ಹಿಂತಿರುಗಿ 0; }
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ write_spl_handoff(ಶೂನ್ಯ) { ಹಿಂತಿರುಗಿ 0; }

#ಆದರೆ_ಅಂತ್ಯ /* HANDOFF */

/**
 * get_bootstage_id() - Get the bootstage ID to emit
 *
 * @start: true if this is for starting SPL, false for ending it
 * Return: bootstage ID to use
 */
ಸ್ಥಿರ ಎಣಿಕೆ bootstage_id get_bootstage_id(bool ಆರಂಭ)
{
	ಎಣಿಕೆ xpl_phase_t phase = xpl_phase();

	ಆದರೆ (IS_ENABLED(CONFIG_TPL_BUILD) && phase == PHASE_TPL)
		ಹಿಂತಿರುಗಿ ಆರಂಭ ? BOOTSTAGE_ID_ಆರಂಭ_TPL : BOOTSTAGE_ID_ಕೊನೆ_TPL;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (IS_ENABLED(CONFIG_VPL_BUILD) && phase == PHASE_VPL)
		ಹಿಂತಿರುಗಿ ಆರಂಭ ? BOOTSTAGE_ID_ಆರಂಭ_VPL : BOOTSTAGE_ID_ಕೊನೆ_VPL;
	ಇಲ್ಲದಿದ್ದರೆ
		ಹಿಂತಿರುಗಿ ಆರಂಭ ? BOOTSTAGE_ID_ಆರಂಭ_SPL : BOOTSTAGE_ID_ಕೊನೆ_SPL;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_common_init(bool ಸಿದ್ಧತೆ_malloc)
{
	ಪೂರ್ಣಾಂಕ ret;

#ಆದರೆ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ಆದರೆ (ಸಿದ್ಧತೆ_malloc) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CFG_MALLOC_F_ADDR
		gd->malloc_ತಳ = CFG_MALLOC_F_ADDR;
#ಆದರೆ_ಅಂತ್ಯ
		gd->malloc_ಎಲ್ಲೆ = CONFIG_VAL(SYS_MALLOC_F_LEN);
		gd->malloc_ptr = 0;
	}
#ಆದರೆ_ಅಂತ್ಯ
	ret = bootstage_init(xpl_is_ಮೊದಲ_phase());
	ಆದರೆ (ret) {
		debug("%s: Failed to set up bootstage: ret=%d\n", __func__,
		      ret);
		ಹಿಂತಿರುಗಿ ret;
	}
	ಆದರೆ (!xpl_is_ಮೊದಲ_phase()) {
		ret = bootstage_unstash_default();
		ಆದರೆ (ret)
			log_debug("Failed to unstash bootstage: ret=%d\n", ret);
	}
	bootstage_mark_name(get_bootstage_id(true), xpl_name(xpl_phase()));
#ಆದರೆ CONFIG_IS_ENABLED(LOG)
	ret = log_init();
	ಆದರೆ (ret) {
		debug("%s: Failed to set up logging\n", __func__);
		ಹಿಂತಿರುಗಿ ret;
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (CONFIG_IS_ENABLED(ದ_REAL)) {
		ret = fdtdec_ಸಿದ್ಧತೆ();
		ಆದರೆ (ret) {
			debug("fdtdec_setup() returned error %d\n", ret);
			ಹಿಂತಿರುಗಿ ret;
		}
	}
	ಆದರೆ (CONFIG_IS_ENABLED(ದ_ಜೀವಂತ)) {
		bootstage_ಆರಂಭ(BOOTSTAGE_ID_ACCUM_ದ_ಜೀವಂತ, "of_live");
		ret = ದ_ಜೀವಂತ_build(gd->fdt_blob,
				    (ರಚನೆ device_ಗಂಟು **)gd_ದ_ಬೇರು_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_ದ_ಜೀವಂತ);
		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ ret;
	}
	ಆದರೆ (CONFIG_IS_ENABLED(DM)) {
		bootstage_ಆರಂಭ(BOOTSTAGE_ID_ACCUM_DM_SPL,
				xpl_phase() == PHASE_TPL ? "dm tpl" : "dm_spl");
		/* With CONFIG_SPL_OF_PLATDATA, bring in all devices */
		ret = dm_init_and_ಪರಿಶೀಲನೆ(!CONFIG_IS_ENABLED(ದ_PLATDATA));
		bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_SPL);
		ಆದರೆ (ret) {
			debug("dm_init_and_scan() returned error %d\n", ret);
			ಹಿಂತಿರುಗಿ ret;
		}

		ret = dm_autoprobe();
		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ ret;
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ spl_ಗಣ_bd(ಶೂನ್ಯ)
{
	/*
	 * NOTE: On some platforms (e.g. x86) bdata may be in flash and not
	 * writeable.
	 */
	ಆದರೆ (!gd->bd)
		gd->bd = &bdata;
}

ಪೂರ್ಣಾಂಕ spl_early_init(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ ret;

	debug("%s\n", __func__);

	ret = spl_common_init(true);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;
	gd->flags |= GD_FLG_SPL_EARLY_INIT;

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ spl_init(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ ret;
	bool ಸಿದ್ಧತೆ_malloc = !(IS_ENABLED(CONFIG_SPL_ರಾಶಿ_R) &&
			IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIMPLE));

	debug("%s\n", __func__);

	ಆದರೆ (!(gd->flags & GD_FLG_SPL_EARLY_INIT)) {
		ret = spl_common_init(ಸಿದ್ಧತೆ_malloc);
		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ ret;
	}
	gd->flags |= GD_FLG_SPL_INIT;

	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ BOOT_DEVICE_NONE
#ವ್ಯಾಖ್ಯಾನಿಸು BOOT_DEVICE_NONE 0xdeadbeef
#ಆದರೆ_ಅಂತ್ಯ

__weak ಶೂನ್ಯ board_boot_ಕ್ರಮ(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();
}

__weak ಪೂರ್ಣಾಂಕ spl_check_board_image(ರಚನೆ spl_image_info *spl_image,
				 ಸ್ಥಿರಾಂಕ ರಚನೆ spl_boot_device *bootdev)
{
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_ತುಂಬು_image(ರಚನೆ spl_image_info *spl_image,
			  ರಚನೆ spl_image_loader *loader)
{
	ಪೂರ್ಣಾಂಕ ret;
	ರಚನೆ spl_boot_device bootdev;

	bootdev.boot_device = loader->boot_device;
	bootdev.boot_device_name = ಖಾಲಿ;

	ret = loader->ತುಂಬು_image(spl_image, &bootdev);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPL_LEGACY_IMAGE_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_CHECK
	ಆದರೆ (!ret && spl_image->dcrc_ಉದ್ದಳತೆ) {
		/* check data crc */
		ulong dcrc = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_wd(0, (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)spl_image->dcrc_ದತ್ತಾಂಶ,
				      spl_image->dcrc_ಉದ್ದಳತೆ, CHUNKSZ_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32);
		ಆದರೆ (dcrc != spl_image->dcrc) {
			puts("SPL: Image data CRC check failed!\n");
			ret = -EINVAL;
		}
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (!ret)
		ret = spl_check_board_image(spl_image, &bootdev);

	ಹಿಂತಿರುಗಿ ret;
}

/**
 * boot_from_devices() - Try loading a booting U-Boot from a list of devices
 *
 * @spl_image: Place to put the image details if successful
 * @spl_boot_list: List of boot devices to try
 * @count: Number of elements in spl_boot_list
 * Return: 0 if OK, -ENODEV if there were no boot devices
 *	if CONFIG_SHOW_ERRORS is enabled, returns -ENXIO if there were
 *	devices but none worked
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ boot_from_devices(ರಚನೆ spl_image_info *spl_image,
			     u32 spl_boot_list[], ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2)
{
	ರಚನೆ spl_image_loader *drv =
		ll_ನಮೂದು_ಆರಂಭ(ರಚನೆ spl_image_loader, spl_image_loader);
	ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ n_ents =
		ll_ನಮೂದು_ಎಣಿಕೆ(ರಚನೆ spl_image_loader, spl_image_loader);
	ಪೂರ್ಣಾಂಕ ret = -ENODEV;
	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = 0; i < ಎಣಿಕೆ_2 && spl_boot_list[i] != BOOT_DEVICE_NONE; i++) {
		ರಚನೆ spl_image_loader *loader;
		ಪೂರ್ಣಾಂಕ bootdev = spl_boot_list[i];

		ಆದರೆ (CONFIG_IS_ENABLED(SHOW_ERRORS))
			ret = -ENXIO;
		ಗಾಗಿ (loader = drv; loader != drv + n_ents; loader++) {
			ಆದರೆ (loader && bootdev != loader->boot_device)
				ಮುಂದುವರಿ;
			ಆದರೆ (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
				ಬರೆ("Trying to boot from %s\n",
				       spl_loader_name(loader));
			}

			ret = spl_ತುಂಬು_image(spl_image, loader);
			ಆದರೆ (!ret) {
				spl_image->boot_device = bootdev;
				ಹಿಂತಿರುಗಿ 0;
			}
			ಬರೆ("Error: %d\n", ret);
		}

		ಆದರೆ (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
			ಆದರೆ (CONFIG_IS_ENABLED(SHOW_ERRORS))
				ಬರೆ(PHASE_PROMPT
				       "Unsupported Boot Device %d\n",
				       bootdev);
			ಇಲ್ಲದಿದ್ದರೆ
				ಬರೆ(PHASE_PROMPT
				       "Unsupported Boot Device!\n");
		}
	}

	ಹಿಂತಿರುಗಿ ret;
}

ಶೂನ್ಯ board_init_r(gd_t *dummy1, ulong dummy2)
{
	u32 spl_boot_list[] = {
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
	};
	spl_jump_ಗಮ್ಯ_image_t jumper = &jump_ಗಮ್ಯ_image;
	ರಚನೆ spl_image_info spl_image;
	ಪೂರ್ಣಾಂಕ ret, os;
	ಶೂನ್ಯ *fdt;

	debug(">>" PHASE_PROMPT "board_init_r()\n");

	spl_ಗಣ_bd();

	ಆದರೆ (IS_ENABLED(CONFIG_SPL_SYS_MALLOC)) {
		mem_malloc_init(SPL_SYS_MALLOC_ಆರಂಭ, SPL_SYS_MALLOC_ಗಾತ್ರ);
		gd->flags |= GD_FLG_ತುಂಬಿದ_MALLOC_INIT;
	}
	ಆದರೆ (!(gd->flags & GD_FLG_SPL_INIT)) {
		ಆದರೆ (spl_init())
			hang();
	}
	timer_init();
	ಆದರೆ (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_init();
		ಆದರೆ (ret) {
			debug("%s: Failed to set up bloblist: ret=%d\n",
			      __func__, ret);
			puts(PHASE_PROMPT "Cannot set up bloblist\n");
			hang();
		}
	}
	ಆದರೆ (CONFIG_IS_ENABLED(HANDOFF)) {
		ಪೂರ್ಣಾಂಕ ret;

		ret = ಸಿದ್ಧತೆ_spl_handoff();
		ಆದರೆ (ret) {
			puts(PHASE_PROMPT "Cannot set up SPL handoff\n");
			hang();
		}
	}

	ಆದರೆ (CONFIG_IS_ENABLED(SOC_INIT))
		spl_soc_init();

	ಆದರೆ (IS_ENABLED(CONFIG_SPL_WATCHDOG) && CONFIG_IS_ENABLED(WDT))
		initr_watchdog();

	ಆದರೆ (IS_ENABLED(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) ||
	    IS_ENABLED(CONFIG_SPL_ATF) || IS_ENABLED(CONFIG_SPL_NET))
		dram_init_banksize();

	ಆದರೆ (IS_ENABLED(CONFIG_SPL_LMB))
		lmb_init();

	ಆದರೆ (CONFIG_IS_ENABLED(PCI) && !(gd->flags & GD_FLG_DM_DEAD)) {
		ret = pci_init();
		ಆದರೆ (ret)
			puts(PHASE_PROMPT "Cannot initialize PCI\n");
		/* Don't fail. We still can try other boot methods. */
	}

	ಆದರೆ (CONFIG_IS_ENABLED(BOARD_INIT))
		spl_board_init();

	bootcount_inc();

	/* Dump driver model states to aid analysis */
	ಆದರೆ (CONFIG_IS_ENABLED(DM_STATS)) {
		ರಚನೆ dm_stats mem;

		dm_get_mem(&mem);
		dm_dump_mem(&mem);
	}

	memset(&spl_image, '\0', ಗಾತ್ರ(spl_image));
	ಆದರೆ (IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image.arg = (ಶೂನ್ಯ *)SPL_PAYLOAD_ವಾದಗಳು_ADDR;
	spl_image.boot_device = BOOT_DEVICE_NONE;
	board_boot_ಕ್ರಮ(spl_boot_list);

	ret = boot_from_devices(&spl_image, spl_boot_list,
				ARRAY_ಗಾತ್ರ(spl_boot_list));
	ಆದರೆ (ret) {
		ಆದರೆ (CONFIG_IS_ENABLED(SHOW_ERRORS))
			ಬರೆ(PHASE_PROMPT "failed to boot from all boot devices (err=%d)\n",
			       ret);
		ಇಲ್ಲದಿದ್ದರೆ
			puts(PHASE_PROMPT "failed to boot from all boot devices\n");
		hang();
	}

	spl_perform_arch_fixups(&spl_image);
	spl_perform_board_fixups(&spl_image);

	os = spl_image.os;
	ಆದರೆ (os == IH_OS_U_BOOT) {
		debug("Jumping to %s...\n", xpl_name(xpl_ಮುಂದಿನ_phase()));
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (CONFIG_IS_ENABLED(ATF) && os == IH_OS_ARM_TRUSTED_FIRMWARE) {
		debug("Jumping to U-Boot via ARM Trusted Firmware\n");
		spl_fixup_fdt(spl_image_fdt_addr(&spl_image));
		jumper = &spl_invoke_atf;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (CONFIG_IS_ENABLED(OPTEE_IMAGE) && os == IH_OS_TEE) {
		debug("Jumping to U-Boot via OP-TEE\n");
		spl_board_prepare_for_optee(spl_image_fdt_addr(&spl_image));
		jumper = &jump_ಗಮ್ಯ_image_optee;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (CONFIG_IS_ENABLED(OPENSBI) && os == IH_OS_OPENSBI) {
		debug("Jumping to U-Boot via RISC-V OpenSBI\n");
		jumper = &spl_invoke_opensbi;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (CONFIG_IS_ENABLED(OS_BOOT) && os == IH_OS_LINUX) {
		debug("Jumping to Linux\n");
		ಆದರೆ (CONFIG_IS_ENABLED(OS_BOOT_ವಾದಗಳು))
			fdt = (ಶೂನ್ಯ *)SPL_PAYLOAD_ವಾದಗಳು_ADDR;
		ಇಲ್ಲದಿದ್ದರೆ
			fdt = spl_image_fdt_addr(&spl_image);
		spl_fixup_fdt(fdt);
		spl_board_prepare_for_linux();
		spl_image.arg = fdt;
		jumper = &jump_ಗಮ್ಯ_image_linux;
	} ಇಲ್ಲದಿದ್ದರೆ {
		debug("Unsupported OS image.. Jumping nevertheless..\n");
	}
	ಆದರೆ (CONFIG_IS_ENABLED(SYS_MALLOC_F) &&
	    !IS_ENABLED(CONFIG_SPL_SYS_MALLOC_ಗಾತ್ರ))
		debug("SPL malloc() used 0x%x bytes (%d KB)\n",
		      gd_malloc_ptr(), gd_malloc_ptr() / 1024);

	bootstage_mark_name(get_bootstage_id(false), "end phase");
	ret = bootstage_stash_default();
	ಆದರೆ (ret)
		debug("Failed to stash bootstage: err=%d\n", ret);

	ಆದರೆ (IS_ENABLED(CONFIG_SPL_VIDEO_REMOVE)) {
		ರಚನೆ udevice *dev;
		ಪೂರ್ಣಾಂಕ rc;

		rc = uclass_ಕಂಡುಹಿಡಿ_device(UCLASS_VIDEO, 0, &dev);
		ಆದರೆ (!rc && dev) {
			rc = device_remove(dev, DM_REMOVE_ಸಾಮಾನ್ಯ);
			ಆದರೆ (rc)
				ಬರೆ("Cannot remove video device '%s' (err=%d)\n",
				       dev->name, rc);
		}
	}
	ಆದರೆ (CONFIG_IS_ENABLED(HANDOFF)) {
		ret = write_spl_handoff();
		ಆದರೆ (ret)
			ಬರೆ(PHASE_PROMPT
			       "SPL hand-off write failed (err=%d)\n", ret);
	}
	ಆದರೆ (CONFIG_IS_ENABLED(UPL_ಹೊರಹೋಗು) && (gd->flags & GD_FLG_UPL)) {
		ret = spl_write_upl_handoff(&spl_image);
		ಆದರೆ (ret) {
			ಬರೆ(PHASE_PROMPT
			       "UPL hand-off write failed (err=%d)\n", ret);
			hang();
		}
	}
	ಆದರೆ (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_ಮುಕ್ತಾಯ();
		ಆದರೆ (ret)
			ಬರೆ("Warning: Failed to finish bloblist (ret=%d)\n",
			       ret);
	}

	spl_board_prepare_for_boot();

	ಆದರೆ (CONFIG_IS_ENABLED(RELOC_LOADER)) {
		ಪೂರ್ಣಾಂಕ ret;

		ret = spl_reloc_jump(&spl_image, jumper);
		ಆದರೆ (ret) {
			ಆದರೆ (xpl_phase() == PHASE_VPL)
				ಬರೆ("jump failed %d\n", ret);
			hang();
		}
	}

	jumper(&spl_image);
}

/*
 * This requires UART clocks to be enabled.  In order for this to work the
 * caller must ensure that the gd pointer is valid.
 */
ಶೂನ್ಯ preloader_console_init(ಶೂನ್ಯ)
{
#ಆದರೆ CONFIG_IS_ENABLED(SERIAL)
	gd->baudrate = CONFIG_BAUDRATE;

	serial_init();		/* serial communications setup */

	gd->flags |= GD_FLG_HAVE_CONSOLE;

#ಆದರೆ CONFIG_IS_ENABLED(BANNER_PRINT)
	puts("\nU-Boot " PHASE_NAME " " PLAIN_ಆವೃತ್ತಿ " (" U_BOOT_DATE " - "
	     U_BOOT_ಸಮಯ " " U_BOOT_TZ ")\n");
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPL_DISPLAY_PRINT
	spl_display_print();
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
}

/**
 * This function is called before the stack is changed from initial stack to
 * relocated stack. It tries to dump the stack size used
 */
__weak ಶೂನ್ಯ spl_relocate_ರಾಶಿ_check(ಶೂನ್ಯ)
{
#ಆದರೆ CONFIG_IS_ENABLED(SYS_REPORT_ರಾಶಿ_F_USAGE)
	ulong init_sp = gd->ಆರಂಭ_addr_sp;
	ulong ರಾಶಿ_bottom = init_sp - CONFIG_VAL(ಗಾತ್ರ_ಎಲ್ಲೆ_PROVIDE_ರಾಶಿ);
	u8 *ptr = (u8 *)ರಾಶಿ_bottom;
	ulong i;

	ಗಾಗಿ (i = 0; i < CONFIG_VAL(ಗಾತ್ರ_ಎಲ್ಲೆ_PROVIDE_ರಾಶಿ); i++) {
		ಆದರೆ (*ptr != CONFIG_VAL(SYS_ರಾಶಿ_F_CHECK_BYTE))
			ನಿಲ್ಲಿಸು;
		ptr++;
	}
	ಬರೆ("SPL initial stack usage: %lu bytes\n",
	       CONFIG_VAL(ಗಾತ್ರ_ಎಲ್ಲೆ_PROVIDE_ರಾಶಿ) - i);
#ಆದರೆ_ಅಂತ್ಯ
}

/**
 * spl_relocate_stack_gd() - Relocate stack ready for board_init_r() execution
 *
 * Sometimes board_init_f() runs with a stack in SRAM but we want to use SDRAM
 * for the main board_init_r() execution. This is typically because we need
 * more stack space for things like the MMC sub-system.
 *
 * This function calculates the stack position, copies the global_data into
 * place, sets the new gd (except for ARM, for which setting GD within a C
 * function may not always work) and returns the new stack position. The
 * caller is responsible for setting up the sp register and, in the case
 * of ARM, setting up gd.
 *
 * All of this is done using the same layout and alignments as done in
 * board_init_f_init_reserve() / board_init_f_alloc_reserve().
 *
 * Return: new stack location, or 0 to use the same stack
 */
ulong spl_relocate_ರಾಶಿ_gd(ಶೂನ್ಯ)
{
#ಆದರೆ CONFIG_IS_ENABLED(ರಾಶಿ_R)
	gd_t *new_gd;
	ulong ptr = CONFIG_SPL_ರಾಶಿ_R_ADDR;

	ಆದರೆ (CONFIG_IS_ENABLED(SYS_REPORT_ರಾಶಿ_F_USAGE))
		spl_relocate_ರಾಶಿ_check();

#ಆದರೆ defined(CONFIG_SPL_SYS_MALLOC_SIMPLE) && CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ಆದರೆ (CONFIG_SPL_ರಾಶಿ_R_MALLOC_SIMPLE_LEN) {
		debug("SPL malloc() before relocation used 0x%x bytes (%d KB)\n",
		      gd->malloc_ptr, gd->malloc_ptr / 1024);
		ptr -= CONFIG_SPL_ರಾಶಿ_R_MALLOC_SIMPLE_LEN;
		gd->malloc_ತಳ = ptr;
		gd->malloc_ಎಲ್ಲೆ = CONFIG_SPL_ರಾಶಿ_R_MALLOC_SIMPLE_LEN;
		gd->malloc_ptr = 0;
	}
#ಆದರೆ_ಅಂತ್ಯ
	/* Get stack position: use 8-byte alignment for ABI compliance */
	ptr -= roundup(ಗಾತ್ರ(gd_t), 16);
	gd->ಆರಂಭ_addr_sp = ptr;
	new_gd = (gd_t *)ptr;
	ಸ್ಮರಣೆ_ನಕಲು(new_gd, (ಶೂನ್ಯ *)gd, ಗಾತ್ರ(gd_t));
#ಆದರೆ CONFIG_IS_ENABLED(DM)
	dm_fixup_for_gd_move(new_gd);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(LOG)
	log_fixup_for_gd_move(new_gd);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(CONFIG_ARM) && !defined(CONFIG_RISCV)
	gd = new_gd;
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ ptr;
#ಇಲ್ಲದಿದ್ದರೆ
	ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
}

#ಆದರೆ defined(CONFIG_BOOTCOUNT_ಎಲ್ಲೆ) && \
	((!defined(CONFIG_TPL_BUILD) && !defined(CONFIG_SPL_BOOTCOUNT_ಎಲ್ಲೆ)) || \
	 (defined(CONFIG_TPL_BUILD) && !defined(CONFIG_TPL_BOOTCOUNT_ಎಲ್ಲೆ)))
ಶೂನ್ಯ bootcount_store(ulong a)
{
}

ulong bootcount_ತುಂಬು(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ
