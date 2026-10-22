// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <config.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <bloblist.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <binman_sym.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <bootstage.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <handoff.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <hang.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <init.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <irq_func.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mapmem.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <of_live.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <serial.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl_load.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <system-constants.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm-generic/gpio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <nand.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <u-boot/crc.h>
#ਜੇ CONFIG_IS_ENABLED(BANNER_PRINT)
#ਅਨ੍ਤਰ੍ਭਾਵ <timestamp.h>
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <version.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mapmem.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm/root.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm/util.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm/device-internal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm/uclass-internal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/compiler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fdt_support.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <bootcount.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <wdt.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <video.h>

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;
DECLARE_BINMAN_MAGIC_SYM;

u32 *boot_params_ptr = ਨਿਰਾ;

#ਜੇ CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS)
/* See spl.h for information about this */
#ਜੇ defined(CONFIG_SPL_BUILD)
binman_sym_declare(ulong, u_boot_any, image_pos);
binman_sym_declare(ulong, u_boot_any, ਮਾਪ);
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_TPL
binman_sym_declare(ulong, u_boot_spl_any, image_pos);
binman_sym_declare(ulong, u_boot_spl_any, ਮਾਪ);
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_VPL
binman_sym_declare(ulong, u_boot_vpl_any, image_pos);
binman_sym_declare(ulong, u_boot_vpl_any, ਮਾਪ);
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* BINMAN_UBOOT_SYMBOLS */

/* Define board data structure */
ਸਥਿਰ ਬਣਤਰ bd_info bdata __attribute__ ((section(".data")));

#ਜੇ CONFIG_IS_ENABLED(SHOW_BOOT_ਤਰੱਕੀ)
/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
__weak ਖਾਲੀ show_boot_ਤਰੱਕੀ(ਪੂਰਨ val) {}
#ਜੇ_ਅੰਤ

#ਜੇ defined(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) || \
	defined(CONFIG_SPL_ATF)
/* weak, default platform-specific function to initialize dram banks */
__weak ਪੂਰਨ dram_init_banksize(ਖਾਲੀ)
{
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

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
#ਜੇ CONFIG_IS_ENABLED(OS_BOOT)
__weak ਪੂਰਨ spl_ਸ਼ੁਰੂ_uboot(ਖਾਲੀ)
{
	puts(PHASE_PROMPT
	     "Please implement spl_start_uboot() for your board\n");
	puts(PHASE_PROMPT "Direct Linux boot not active!\n");
	ਵਾਪਸ 1;
}

/*
 * Weak default function for arch specific zImage check. Return zero
 * and fill start and end address if image is recognized.
 */
ਪੂਰਨ __weak bootz_ਤਿਆਰੀ(ulong image, ulong *ਸ਼ੁਰੂ, ulong *ਸਿਰਾ_2)
{
	 ਵਾਪਸ 1;
}

ਪੂਰਨ __weak booti_ਤਿਆਰੀ(ulong image, ulong *relocated_addr, ulong *ਮਾਪ, bool force_reloc)
{
	 ਵਾਪਸ 1;
}
#ਜੇ_ਅੰਤ

/* Weak default function for arch specific fixups to the spl_image_info */
ਖਾਲੀ __weak spl_perform_arch_fixups(ਬਣਤਰ spl_image_info *spl_image)
{
}

/* Weak default function for board specific fixups to the spl_image_info */
ਖਾਲੀ __weak spl_perform_board_fixups(ਬਣਤਰ spl_image_info *spl_image)
{
}

ਖਾਲੀ spl_fixup_fdt(ਖਾਲੀ *fdt_blob)
{
#ਜੇ defined(CONFIG_SPL_ਦਾ_LIBFDT)
	ਪੂਰਨ err;

	ਜੇ (!fdt_blob)
		ਵਾਪਸ;

	err = fdt_check_ਸਿਰਲੇਖ(fdt_blob);
	ਜੇ (err < 0) {
		ਲਿਖੋ("fdt_root: %s\n", fdt_strerror(err));
		ਵਾਪਸ;
	}

	/* fixup the memory dt node */
	err = fdt_shrink_ਮੰਜ਼ਿਲ_minimum(fdt_blob, 0);
	ਜੇ (err == 0) {
		ਲਿਖੋ(PHASE_PROMPT "fdt_shrink_to_minimum err - %d\n", err);
		ਵਾਪਸ;
	}

	err = arch_fixup_fdt(fdt_blob);
	ਜੇ (err) {
		ਲਿਖੋ(PHASE_PROMPT "arch_fixup_fdt err - %d\n", err);
		ਵਾਪਸ;
	}
#ਜੇ_ਅੰਤ
}

ਪੂਰਨ spl_reserve_video_from_ram_ਸਿਖਰ(ਖਾਲੀ)
{
	ਜੇ (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		ਪੂਰਨ ret;

		addr = gd->ram_ਸਿਖਰ;
		ret = video_reserve(&addr);
		ਜੇ (ret)
			ਵਾਪਸ ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	ਵਾਪਸ 0;
}

ulong spl_get_image_pos(ਖਾਲੀ)
{
	ਜੇ (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		ਵਾਪਸ BINMAN_SYM_MISSING;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_VPL
	ਜੇ (xpl_ਅਗਲਾ_phase() == PHASE_VPL)
		ਵਾਪਸ binman_sym(ulong, u_boot_vpl_any, image_pos);
#ਜੇ_ਅੰਤ
#ਜੇ defined(CONFIG_TPL) && !defined(CONFIG_VPL)
	ਜੇ (xpl_ਅਗਲਾ_phase() == PHASE_SPL)
		ਵਾਪਸ binman_sym(ulong, u_boot_spl_any, image_pos);
#ਜੇ_ਅੰਤ
#ਜੇ defined(CONFIG_SPL_BUILD)
	ਵਾਪਸ binman_sym(ulong, u_boot_any, image_pos);
#ਜੇ_ਅੰਤ

	ਵਾਪਸ BINMAN_SYM_MISSING;
}

ulong spl_get_image_ਮਾਪ(ਖਾਲੀ)
{
	ਜੇ (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		ਵਾਪਸ BINMAN_SYM_MISSING;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_VPL
	ਜੇ (xpl_ਅਗਲਾ_phase() == PHASE_VPL)
		ਵਾਪਸ binman_sym(ulong, u_boot_vpl_any, ਮਾਪ);
#ਜੇ_ਅੰਤ
	ਵਾਪਸ xpl_ਅਗਲਾ_phase() == PHASE_SPL ?
		binman_sym(ulong, u_boot_spl_any, ਮਾਪ) :
		binman_sym(ulong, u_boot_any, ਮਾਪ);
}

ulong spl_get_image_ਪਾਠ_ਅਧਾਰ(ਖਾਲੀ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_VPL
	ਜੇ (xpl_ਅਗਲਾ_phase() == PHASE_VPL)
		ਵਾਪਸ CONFIG_VPL_ਪਾਠ_ਅਧਾਰ;
#ਜੇ_ਅੰਤ
	ਵਾਪਸ xpl_ਅਗਲਾ_phase() == PHASE_SPL ? CONFIG_SPL_ਪਾਠ_ਅਧਾਰ :
		CONFIG_ਪਾਠ_ਅਧਾਰ;
}

/*
 * Weak default function for board specific cleanup/preparation before
 * Linux boot. Some boards/platforms might not need it, so just provide
 * an empty stub here.
 */
__weak ਖਾਲੀ spl_board_prepare_for_linux(ਖਾਲੀ)
{
	/* Nothing to do! */
}

__weak ਖਾਲੀ spl_board_prepare_for_optee(ਖਾਲੀ *fdt)
{
}

__weak ਅਚਲ ਅੱਖਰ *spl_board_loader_name(u32 boot_device)
{
	ਵਾਪਸ ਨਿਰਾ;
}

#ਜੇ CONFIG_IS_ENABLED(OPTEE_IMAGE)
__weak ਖਾਲੀ __noreturn jump_ਮੰਜ਼ਿਲ_image_optee(ਬਣਤਰ spl_image_info *spl_image)
{
	spl_optee_ਇੰਦਰਾਜ(ਨਿਰਾ, ਨਿਰਾ, spl_image->fdt_addr,
			(ਖਾਲੀ *)spl_image->ਇੰਦਰਾਜ_ਬਿੰਦੂ);
}
#ਜੇ_ਅੰਤ

__weak ਖਾਲੀ spl_board_prepare_for_boot(ਖਾਲੀ)
{
	/* Nothing to do! */
}

__weak ਬਣਤਰ legacy_img_hdr *spl_get_ਲੱਦੋ_ਬਫਰ(ssize_t ਵਿੱਥ, size_t ਮਾਪ)
{
	ਵਾਪਸ map_sysmem(CONFIG_ਪਾਠ_ਅਧਾਰ + ਵਿੱਥ, 0);
}

ਖਾਲੀ spl_ਸੈੱਟ_ਸਿਰਲੇਖ_raw_uboot(ਬਣਤਰ spl_image_info *spl_image)
{
	ulong u_boot_pos = spl_get_image_pos();

#ਜੇ CONFIG_SYS_MONITOR_LEN != 0
	spl_image->ਮਾਪ = CONFIG_SYS_MONITOR_LEN;
#ਨਹੀਂਤਾਂ
	/* Unknown U-Boot size, let's assume it will not be more than 200 KB */
	spl_image->ਮਾਪ = 200 * 1024;
#ਜੇ_ਅੰਤ

	/*
	 * Binman error cases: address of the end of the previous region or the
	 * start of the image's entry area (usually 0) if there is no previous
	 * region.
	 */
	ਜੇ (u_boot_pos && u_boot_pos != BINMAN_SYM_MISSING) {
		/* Binman does not support separated entry addresses */
		spl_image->ਇੰਦਰਾਜ_ਬਿੰਦੂ = spl_get_image_ਪਾਠ_ਅਧਾਰ();
		spl_image->ਲੱਦੋ_addr = spl_get_image_ਪਾਠ_ਅਧਾਰ();
		spl_image->ਮਾਪ = spl_get_image_ਮਾਪ();
		log_debug("Next load addr %lx\n", spl_image->ਲੱਦੋ_addr);
	} ਨਹੀਂਤਾਂ {
		spl_image->ਇੰਦਰਾਜ_ਬਿੰਦੂ = CONFIG_SYS_UBOOT_ਸ਼ੁਰੂ;
		spl_image->ਲੱਦੋ_addr = CONFIG_ਪਾਠ_ਅਧਾਰ;
		log_debug("Default load addr %lx (u_boot_pos=%lx)\n",
			  spl_image->ਲੱਦੋ_addr, u_boot_pos);
	}
	spl_image->os = IH_OS_U_BOOT;
	spl_image->name = xpl_name(xpl_ਅਗਲਾ_phase());
	log_debug("Next phase: %s at %lx size %lx\n", spl_image->name,
		  spl_image->ਲੱਦੋ_addr, (ulong)spl_image->ਮਾਪ);
}

__weak ਪੂਰਨ spl_parse_board_ਸਿਰਲੇਖ(ਬਣਤਰ spl_image_info *spl_image,
				  ਅਚਲ ਬਣਤਰ spl_boot_device *bootdev,
				  ਅਚਲ ਖਾਲੀ *image_ਸਿਰਲੇਖ, size_t ਮਾਪ)
{
	ਵਾਪਸ -EINVAL;
}

__weak ਪੂਰਨ spl_parse_legacy_ਸਿਰਲੇਖ(ਬਣਤਰ spl_image_info *spl_image,
				   ਅਚਲ ਬਣਤਰ legacy_img_hdr *ਸਿਰਲੇਖ)
{
	/* LEGACY image not supported */
	debug("Legacy boot image support not enabled, proceeding to other boot methods\n");
	ਵਾਪਸ -EINVAL;
}

ਪੂਰਨ spl_parse_image_ਸਿਰਲੇਖ(ਬਣਤਰ spl_image_info *spl_image,
			   ਅਚਲ ਬਣਤਰ spl_boot_device *bootdev,
			   ਅਚਲ ਬਣਤਰ legacy_img_hdr *ਸਿਰਲੇਖ)
{
	ਪੂਰਨ ret;

	ਜੇ (CONFIG_IS_ENABLED(ਲੱਦੋ_FIT_ਭਰਿਆ)) {
		ret = spl_ਲੱਦੋ_fit_image(spl_image, ਸਿਰਲੇਖ);

		ਜੇ (!ret)
			ਵਾਪਸ ret;
	}
	ਜੇ (image_get_magic(ਸਿਰਲੇਖ) == IH_MAGIC) {
		ਪੂਰਨ ret;

		ret = spl_parse_legacy_ਸਿਰਲੇਖ(spl_image, ਸਿਰਲੇਖ);
		ਜੇ (ret)
			ਵਾਪਸ ret;
		ਵਾਪਸ 0;
	}

	ਜੇ (IS_ENABLED(CONFIG_SPL_PANIC_ON_RAW_IMAGE)) {
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

	ਜੇ (IS_ENABLED(CONFIG_SPL_OS_BOOT) && IS_ENABLED(CONFIG_SPL_BOOTI)) {
		ulong ਸ਼ੁਰੂ, ਮਾਪ;

		ਜੇ (!booti_ਤਿਆਰੀ((ulong)ਸਿਰਲੇਖ, &ਸ਼ੁਰੂ, &ਮਾਪ, 0)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->ਲੱਦੋ_addr = ਸ਼ੁਰੂ;
			spl_image->ਇੰਦਰਾਜ_ਬਿੰਦੂ = ਸ਼ੁਰੂ;
			spl_image->ਮਾਪ = ਮਾਪ;
			debug(PHASE_PROMPT
			      "payload Image, load addr: 0x%lx size: %d\n",
			      spl_image->ਲੱਦੋ_addr, spl_image->ਮਾਪ);
			ਵਾਪਸ 0;
		}
	} ਨਹੀਂਤਾਂ ਜੇ (IS_ENABLED(CONFIG_SPL_OS_BOOT) &&
		   IS_ENABLED(CONFIG_SPL_BOOTZ)) {
		ulong ਸ਼ੁਰੂ, ਸਿਰਾ_2;

		ਜੇ (!bootz_ਤਿਆਰੀ((ulong)ਸਿਰਲੇਖ, &ਸ਼ੁਰੂ, &ਸਿਰਾ_2)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->ਲੱਦੋ_addr = CONFIG_SYS_ਲੱਦੋ_ADDR;
			spl_image->ਇੰਦਰਾਜ_ਬਿੰਦੂ = CONFIG_SYS_ਲੱਦੋ_ADDR;
			spl_image->ਮਾਪ = ਸਿਰਾ_2 - ਸ਼ੁਰੂ;
			debug(PHASE_PROMPT
			      "payload zImage, load addr: 0x%lx size: %d\n",
			      spl_image->ਲੱਦੋ_addr, spl_image->ਮਾਪ);
			ਵਾਪਸ 0;
		}
	}

	ਜੇ (!spl_parse_board_ਸਿਰਲੇਖ(spl_image, bootdev, (ਅਚਲ ਖਾਲੀ *)ਸਿਰਲੇਖ,
				    ਆਕਾਰ(*ਸਿਰਲੇਖ)))
		ਵਾਪਸ 0;

	ਜੇ (IS_ENABLED(CONFIG_SPL_RAW_IMAGE_SUPPORT)) {
		/* Signature not found - assume u-boot.bin */
		debug("mkimage signature not found - ih_magic = %x\n",
		      ਸਿਰਲੇਖ->ih_magic);
		spl_ਸੈੱਟ_ਸਿਰਲੇਖ_raw_uboot(spl_image);
	} ਨਹੀਂਤਾਂ {
		/* RAW image not supported, proceed to other boot methods. */
		debug("Raw boot image support not enabled, proceeding to other boot methods\n");
		ਵਾਪਸ -EINVAL;
	}

	ਵਾਪਸ 0;
}

#ਜੇ SPL_ਲੱਦੋ_USERS > 1
ਪੂਰਨ spl_ਲੱਦੋ(ਬਣਤਰ spl_image_info *spl_image,
	     ਅਚਲ ਬਣਤਰ spl_boot_device *bootdev, ਬਣਤਰ spl_ਲੱਦੋ_info *info,
	     size_t ਮਾਪ, size_t ਵਿੱਥ)
{
	ਵਾਪਸ spl_ਲੱਦੋ_2(spl_image, bootdev, info, ਮਾਪ, ਵਿੱਥ);
}
#ਜੇ_ਅੰਤ

__weak ਖਾਲੀ __noreturn jump_ਮੰਜ਼ਿਲ_image(ਬਣਤਰ spl_image_info *spl_image)
{
	ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ __noreturn (*image_ਇੰਦਰਾਜ_noargs_t)(ਖਾਲੀ);

	image_ਇੰਦਰਾਜ_noargs_t image_ਇੰਦਰਾਜ =
		(image_ਇੰਦਰਾਜ_noargs_t)spl_image->ਇੰਦਰਾਜ_ਬਿੰਦੂ;

	debug("image entry point: 0x%lx\n", spl_image->ਇੰਦਰਾਜ_ਬਿੰਦੂ);
	image_ਇੰਦਰਾਜ();
}

#ਜੇ CONFIG_IS_ENABLED(HANDOFF)
/**
 * Set up the SPL hand-off information
 *
 * This is initially empty (zero) but can be written by
 */
ਸਥਿਰ ਪੂਰਨ ਤਿਆਰੀ_spl_handoff(ਖਾਲੀ)
{
	ਬਣਤਰ spl_handoff *ho;

	ho = bloblist_ensure(BLOBLISTT_U_BOOT_SPL_HANDOFF, ਆਕਾਰ(ਬਣਤਰ spl_handoff));
	ਜੇ (!ho)
		ਵਾਪਸ -ENOENT;

	ਵਾਪਸ 0;
}

__weak ਪੂਰਨ handoff_arch_save(ਬਣਤਰ spl_handoff *ho)
{
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ write_spl_handoff(ਖਾਲੀ)
{
	ਬਣਤਰ spl_handoff *ho;
	ਪੂਰਨ ret;

	ho = bloblist_ਲੱਭੋ(BLOBLISTT_U_BOOT_SPL_HANDOFF, ਆਕਾਰ(ਬਣਤਰ spl_handoff));
	ਜੇ (!ho)
		ਵਾਪਸ -ENOENT;
	handoff_save_dram(ho);
	ret = handoff_arch_save(ho);
	ਜੇ (ret)
		ਵਾਪਸ ret;
	debug(PHASE_PROMPT "Wrote SPL handoff\n");

	ਵਾਪਸ 0;
}
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ ਤਿਆਰੀ_spl_handoff(ਖਾਲੀ) { ਵਾਪਸ 0; }
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ write_spl_handoff(ਖਾਲੀ) { ਵਾਪਸ 0; }

#ਜੇ_ਅੰਤ /* HANDOFF */

/**
 * get_bootstage_id() - Get the bootstage ID to emit
 *
 * @start: true if this is for starting SPL, false for ending it
 * Return: bootstage ID to use
 */
ਸਥਿਰ ਗਿਣਤੀ bootstage_id get_bootstage_id(bool ਸ਼ੁਰੂ)
{
	ਗਿਣਤੀ xpl_phase_t phase = xpl_phase();

	ਜੇ (IS_ENABLED(CONFIG_TPL_BUILD) && phase == PHASE_TPL)
		ਵਾਪਸ ਸ਼ੁਰੂ ? BOOTSTAGE_ID_ਸ਼ੁਰੂ_TPL : BOOTSTAGE_ID_ਸਿਰਾ_TPL;
	ਨਹੀਂਤਾਂ ਜੇ (IS_ENABLED(CONFIG_VPL_BUILD) && phase == PHASE_VPL)
		ਵਾਪਸ ਸ਼ੁਰੂ ? BOOTSTAGE_ID_ਸ਼ੁਰੂ_VPL : BOOTSTAGE_ID_ਸਿਰਾ_VPL;
	ਨਹੀਂਤਾਂ
		ਵਾਪਸ ਸ਼ੁਰੂ ? BOOTSTAGE_ID_ਸ਼ੁਰੂ_SPL : BOOTSTAGE_ID_ਸਿਰਾ_SPL;
}

ਸਥਿਰ ਪੂਰਨ spl_common_init(bool ਤਿਆਰੀ_malloc)
{
	ਪੂਰਨ ret;

#ਜੇ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ਜੇ (ਤਿਆਰੀ_malloc) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CFG_MALLOC_F_ADDR
		gd->malloc_ਅਧਾਰ = CFG_MALLOC_F_ADDR;
#ਜੇ_ਅੰਤ
		gd->malloc_ਹੱਦਬੰਦੀ = CONFIG_VAL(SYS_MALLOC_F_LEN);
		gd->malloc_ptr = 0;
	}
#ਜੇ_ਅੰਤ
	ret = bootstage_init(xpl_is_ਪਹਿਲਾ_phase());
	ਜੇ (ret) {
		debug("%s: Failed to set up bootstage: ret=%d\n", __func__,
		      ret);
		ਵਾਪਸ ret;
	}
	ਜੇ (!xpl_is_ਪਹਿਲਾ_phase()) {
		ret = bootstage_unstash_default();
		ਜੇ (ret)
			log_debug("Failed to unstash bootstage: ret=%d\n", ret);
	}
	bootstage_mark_name(get_bootstage_id(true), xpl_name(xpl_phase()));
#ਜੇ CONFIG_IS_ENABLED(LOG)
	ret = log_init();
	ਜੇ (ret) {
		debug("%s: Failed to set up logging\n", __func__);
		ਵਾਪਸ ret;
	}
#ਜੇ_ਅੰਤ
	ਜੇ (CONFIG_IS_ENABLED(ਦਾ_REAL)) {
		ret = fdtdec_ਤਿਆਰੀ();
		ਜੇ (ret) {
			debug("fdtdec_setup() returned error %d\n", ret);
			ਵਾਪਸ ret;
		}
	}
	ਜੇ (CONFIG_IS_ENABLED(ਦਾ_ਜੀਵਤ)) {
		bootstage_ਸ਼ੁਰੂ(BOOTSTAGE_ID_ACCUM_ਦਾ_ਜੀਵਤ, "of_live");
		ret = ਦਾ_ਜੀਵਤ_build(gd->fdt_blob,
				    (ਬਣਤਰ device_ਗੰਢ **)gd_ਦਾ_ਜੜ੍ਹ_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_ਦਾ_ਜੀਵਤ);
		ਜੇ (ret)
			ਵਾਪਸ ret;
	}
	ਜੇ (CONFIG_IS_ENABLED(DM)) {
		bootstage_ਸ਼ੁਰੂ(BOOTSTAGE_ID_ACCUM_DM_SPL,
				xpl_phase() == PHASE_TPL ? "dm tpl" : "dm_spl");
		/* With CONFIG_SPL_OF_PLATDATA, bring in all devices */
		ret = dm_init_and_ਜਾਂਚ(!CONFIG_IS_ENABLED(ਦਾ_PLATDATA));
		bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_SPL);
		ਜੇ (ret) {
			debug("dm_init_and_scan() returned error %d\n", ret);
			ਵਾਪਸ ret;
		}

		ret = dm_autoprobe();
		ਜੇ (ret)
			ਵਾਪਸ ret;
	}

	ਵਾਪਸ 0;
}

ਖਾਲੀ spl_ਸੈੱਟ_bd(ਖਾਲੀ)
{
	/*
	 * NOTE: On some platforms (e.g. x86) bdata may be in flash and not
	 * writeable.
	 */
	ਜੇ (!gd->bd)
		gd->bd = &bdata;
}

ਪੂਰਨ spl_early_init(ਖਾਲੀ)
{
	ਪੂਰਨ ret;

	debug("%s\n", __func__);

	ret = spl_common_init(true);
	ਜੇ (ret)
		ਵਾਪਸ ret;
	gd->flags |= GD_FLG_SPL_EARLY_INIT;

	ਵਾਪਸ 0;
}

ਪੂਰਨ spl_init(ਖਾਲੀ)
{
	ਪੂਰਨ ret;
	bool ਤਿਆਰੀ_malloc = !(IS_ENABLED(CONFIG_SPL_ਢੇਰ_R) &&
			IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIMPLE));

	debug("%s\n", __func__);

	ਜੇ (!(gd->flags & GD_FLG_SPL_EARLY_INIT)) {
		ret = spl_common_init(ਤਿਆਰੀ_malloc);
		ਜੇ (ret)
			ਵਾਪਸ ret;
	}
	gd->flags |= GD_FLG_SPL_INIT;

	ਵਾਪਸ 0;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ BOOT_DEVICE_NONE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BOOT_DEVICE_NONE 0xdeadbeef
#ਜੇ_ਅੰਤ

__weak ਖਾਲੀ board_boot_ਕ੍ਰਮ(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();
}

__weak ਪੂਰਨ spl_check_board_image(ਬਣਤਰ spl_image_info *spl_image,
				 ਅਚਲ ਬਣਤਰ spl_boot_device *bootdev)
{
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ spl_ਲੱਦੋ_image(ਬਣਤਰ spl_image_info *spl_image,
			  ਬਣਤਰ spl_image_loader *loader)
{
	ਪੂਰਨ ret;
	ਬਣਤਰ spl_boot_device bootdev;

	bootdev.boot_device = loader->boot_device;
	bootdev.boot_device_name = ਨਿਰਾ;

	ret = loader->ਲੱਦੋ_image(spl_image, &bootdev);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPL_LEGACY_IMAGE_ਚੱਕਰੀ_ਜਾਂਚ_CHECK
	ਜੇ (!ret && spl_image->dcrc_ਲੰਮਾਈ) {
		/* check data crc */
		ulong dcrc = ਚੱਕਰੀ_ਜਾਂਚ_32_wd(0, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)spl_image->dcrc_ਅੰਕੜੇ,
				      spl_image->dcrc_ਲੰਮਾਈ, CHUNKSZ_ਚੱਕਰੀ_ਜਾਂਚ_32);
		ਜੇ (dcrc != spl_image->dcrc) {
			puts("SPL: Image data CRC check failed!\n");
			ret = -EINVAL;
		}
	}
#ਜੇ_ਅੰਤ
	ਜੇ (!ret)
		ret = spl_check_board_image(spl_image, &bootdev);

	ਵਾਪਸ ret;
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
ਸਥਿਰ ਪੂਰਨ boot_from_devices(ਬਣਤਰ spl_image_info *spl_image,
			     u32 spl_boot_list[], ਪੂਰਨ ਗਿਣਤੀ_2)
{
	ਬਣਤਰ spl_image_loader *drv =
		ll_ਇੰਦਰਾਜ_ਸ਼ੁਰੂ(ਬਣਤਰ spl_image_loader, spl_image_loader);
	ਅਚਲ ਪੂਰਨ n_ents =
		ll_ਇੰਦਰਾਜ_ਗਿਣਤੀ(ਬਣਤਰ spl_image_loader, spl_image_loader);
	ਪੂਰਨ ret = -ENODEV;
	ਪੂਰਨ i;

	ਲਈ (i = 0; i < ਗਿਣਤੀ_2 && spl_boot_list[i] != BOOT_DEVICE_NONE; i++) {
		ਬਣਤਰ spl_image_loader *loader;
		ਪੂਰਨ bootdev = spl_boot_list[i];

		ਜੇ (CONFIG_IS_ENABLED(SHOW_ERRORS))
			ret = -ENXIO;
		ਲਈ (loader = drv; loader != drv + n_ents; loader++) {
			ਜੇ (loader && bootdev != loader->boot_device)
				ਜਾਰੀ;
			ਜੇ (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
				ਲਿਖੋ("Trying to boot from %s\n",
				       spl_loader_name(loader));
			}

			ret = spl_ਲੱਦੋ_image(spl_image, loader);
			ਜੇ (!ret) {
				spl_image->boot_device = bootdev;
				ਵਾਪਸ 0;
			}
			ਲਿਖੋ("Error: %d\n", ret);
		}

		ਜੇ (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
			ਜੇ (CONFIG_IS_ENABLED(SHOW_ERRORS))
				ਲਿਖੋ(PHASE_PROMPT
				       "Unsupported Boot Device %d\n",
				       bootdev);
			ਨਹੀਂਤਾਂ
				ਲਿਖੋ(PHASE_PROMPT
				       "Unsupported Boot Device!\n");
		}
	}

	ਵਾਪਸ ret;
}

ਖਾਲੀ board_init_r(gd_t *dummy1, ulong dummy2)
{
	u32 spl_boot_list[] = {
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
	};
	spl_jump_ਮੰਜ਼ਿਲ_image_t jumper = &jump_ਮੰਜ਼ਿਲ_image;
	ਬਣਤਰ spl_image_info spl_image;
	ਪੂਰਨ ret, os;
	ਖਾਲੀ *fdt;

	debug(">>" PHASE_PROMPT "board_init_r()\n");

	spl_ਸੈੱਟ_bd();

	ਜੇ (IS_ENABLED(CONFIG_SPL_SYS_MALLOC)) {
		mem_malloc_init(SPL_SYS_MALLOC_ਸ਼ੁਰੂ, SPL_SYS_MALLOC_ਮਾਪ);
		gd->flags |= GD_FLG_ਭਰਿਆ_MALLOC_INIT;
	}
	ਜੇ (!(gd->flags & GD_FLG_SPL_INIT)) {
		ਜੇ (spl_init())
			hang();
	}
	timer_init();
	ਜੇ (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_init();
		ਜੇ (ret) {
			debug("%s: Failed to set up bloblist: ret=%d\n",
			      __func__, ret);
			puts(PHASE_PROMPT "Cannot set up bloblist\n");
			hang();
		}
	}
	ਜੇ (CONFIG_IS_ENABLED(HANDOFF)) {
		ਪੂਰਨ ret;

		ret = ਤਿਆਰੀ_spl_handoff();
		ਜੇ (ret) {
			puts(PHASE_PROMPT "Cannot set up SPL handoff\n");
			hang();
		}
	}

	ਜੇ (CONFIG_IS_ENABLED(SOC_INIT))
		spl_soc_init();

	ਜੇ (IS_ENABLED(CONFIG_SPL_WATCHDOG) && CONFIG_IS_ENABLED(WDT))
		initr_watchdog();

	ਜੇ (IS_ENABLED(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) ||
	    IS_ENABLED(CONFIG_SPL_ATF) || IS_ENABLED(CONFIG_SPL_NET))
		dram_init_banksize();

	ਜੇ (IS_ENABLED(CONFIG_SPL_LMB))
		lmb_init();

	ਜੇ (CONFIG_IS_ENABLED(PCI) && !(gd->flags & GD_FLG_DM_DEAD)) {
		ret = pci_init();
		ਜੇ (ret)
			puts(PHASE_PROMPT "Cannot initialize PCI\n");
		/* Don't fail. We still can try other boot methods. */
	}

	ਜੇ (CONFIG_IS_ENABLED(BOARD_INIT))
		spl_board_init();

	bootcount_inc();

	/* Dump driver model states to aid analysis */
	ਜੇ (CONFIG_IS_ENABLED(DM_STATS)) {
		ਬਣਤਰ dm_stats mem;

		dm_get_mem(&mem);
		dm_dump_mem(&mem);
	}

	memset(&spl_image, '\0', ਆਕਾਰ(spl_image));
	ਜੇ (IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image.arg = (ਖਾਲੀ *)SPL_PAYLOAD_ਦਲੀਲਾਂ_ADDR;
	spl_image.boot_device = BOOT_DEVICE_NONE;
	board_boot_ਕ੍ਰਮ(spl_boot_list);

	ret = boot_from_devices(&spl_image, spl_boot_list,
				ARRAY_ਮਾਪ(spl_boot_list));
	ਜੇ (ret) {
		ਜੇ (CONFIG_IS_ENABLED(SHOW_ERRORS))
			ਲਿਖੋ(PHASE_PROMPT "failed to boot from all boot devices (err=%d)\n",
			       ret);
		ਨਹੀਂਤਾਂ
			puts(PHASE_PROMPT "failed to boot from all boot devices\n");
		hang();
	}

	spl_perform_arch_fixups(&spl_image);
	spl_perform_board_fixups(&spl_image);

	os = spl_image.os;
	ਜੇ (os == IH_OS_U_BOOT) {
		debug("Jumping to %s...\n", xpl_name(xpl_ਅਗਲਾ_phase()));
	} ਨਹੀਂਤਾਂ ਜੇ (CONFIG_IS_ENABLED(ATF) && os == IH_OS_ARM_TRUSTED_FIRMWARE) {
		debug("Jumping to U-Boot via ARM Trusted Firmware\n");
		spl_fixup_fdt(spl_image_fdt_addr(&spl_image));
		jumper = &spl_invoke_atf;
	} ਨਹੀਂਤਾਂ ਜੇ (CONFIG_IS_ENABLED(OPTEE_IMAGE) && os == IH_OS_TEE) {
		debug("Jumping to U-Boot via OP-TEE\n");
		spl_board_prepare_for_optee(spl_image_fdt_addr(&spl_image));
		jumper = &jump_ਮੰਜ਼ਿਲ_image_optee;
	} ਨਹੀਂਤਾਂ ਜੇ (CONFIG_IS_ENABLED(OPENSBI) && os == IH_OS_OPENSBI) {
		debug("Jumping to U-Boot via RISC-V OpenSBI\n");
		jumper = &spl_invoke_opensbi;
	} ਨਹੀਂਤਾਂ ਜੇ (CONFIG_IS_ENABLED(OS_BOOT) && os == IH_OS_LINUX) {
		debug("Jumping to Linux\n");
		ਜੇ (CONFIG_IS_ENABLED(OS_BOOT_ਦਲੀਲਾਂ))
			fdt = (ਖਾਲੀ *)SPL_PAYLOAD_ਦਲੀਲਾਂ_ADDR;
		ਨਹੀਂਤਾਂ
			fdt = spl_image_fdt_addr(&spl_image);
		spl_fixup_fdt(fdt);
		spl_board_prepare_for_linux();
		spl_image.arg = fdt;
		jumper = &jump_ਮੰਜ਼ਿਲ_image_linux;
	} ਨਹੀਂਤਾਂ {
		debug("Unsupported OS image.. Jumping nevertheless..\n");
	}
	ਜੇ (CONFIG_IS_ENABLED(SYS_MALLOC_F) &&
	    !IS_ENABLED(CONFIG_SPL_SYS_MALLOC_ਮਾਪ))
		debug("SPL malloc() used 0x%x bytes (%d KB)\n",
		      gd_malloc_ptr(), gd_malloc_ptr() / 1024);

	bootstage_mark_name(get_bootstage_id(false), "end phase");
	ret = bootstage_stash_default();
	ਜੇ (ret)
		debug("Failed to stash bootstage: err=%d\n", ret);

	ਜੇ (IS_ENABLED(CONFIG_SPL_VIDEO_REMOVE)) {
		ਬਣਤਰ udevice *dev;
		ਪੂਰਨ rc;

		rc = uclass_ਲੱਭੋ_device(UCLASS_VIDEO, 0, &dev);
		ਜੇ (!rc && dev) {
			rc = device_remove(dev, DM_REMOVE_ਸਾਧਾਰਨ);
			ਜੇ (rc)
				ਲਿਖੋ("Cannot remove video device '%s' (err=%d)\n",
				       dev->name, rc);
		}
	}
	ਜੇ (CONFIG_IS_ENABLED(HANDOFF)) {
		ret = write_spl_handoff();
		ਜੇ (ret)
			ਲਿਖੋ(PHASE_PROMPT
			       "SPL hand-off write failed (err=%d)\n", ret);
	}
	ਜੇ (CONFIG_IS_ENABLED(UPL_ਬਾਹਰਲਾ) && (gd->flags & GD_FLG_UPL)) {
		ret = spl_write_upl_handoff(&spl_image);
		ਜੇ (ret) {
			ਲਿਖੋ(PHASE_PROMPT
			       "UPL hand-off write failed (err=%d)\n", ret);
			hang();
		}
	}
	ਜੇ (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_ਅੰਤ();
		ਜੇ (ret)
			ਲਿਖੋ("Warning: Failed to finish bloblist (ret=%d)\n",
			       ret);
	}

	spl_board_prepare_for_boot();

	ਜੇ (CONFIG_IS_ENABLED(RELOC_LOADER)) {
		ਪੂਰਨ ret;

		ret = spl_reloc_jump(&spl_image, jumper);
		ਜੇ (ret) {
			ਜੇ (xpl_phase() == PHASE_VPL)
				ਲਿਖੋ("jump failed %d\n", ret);
			hang();
		}
	}

	jumper(&spl_image);
}

/*
 * This requires UART clocks to be enabled.  In order for this to work the
 * caller must ensure that the gd pointer is valid.
 */
ਖਾਲੀ preloader_console_init(ਖਾਲੀ)
{
#ਜੇ CONFIG_IS_ENABLED(SERIAL)
	gd->baudrate = CONFIG_BAUDRATE;

	serial_init();		/* serial communications setup */

	gd->flags |= GD_FLG_HAVE_CONSOLE;

#ਜੇ CONFIG_IS_ENABLED(BANNER_PRINT)
	puts("\nU-Boot " PHASE_NAME " " PLAIN_ਸੰਸਕਰਣ " (" U_BOOT_DATE " - "
	     U_BOOT_ਸਮਾਂ " " U_BOOT_TZ ")\n");
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPL_DISPLAY_PRINT
	spl_display_print();
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
}

/**
 * This function is called before the stack is changed from initial stack to
 * relocated stack. It tries to dump the stack size used
 */
__weak ਖਾਲੀ spl_relocate_ਢੇਰ_check(ਖਾਲੀ)
{
#ਜੇ CONFIG_IS_ENABLED(SYS_REPORT_ਢੇਰ_F_USAGE)
	ulong init_sp = gd->ਸ਼ੁਰੂ_addr_sp;
	ulong ਢੇਰ_bottom = init_sp - CONFIG_VAL(ਮਾਪ_ਹੱਦਬੰਦੀ_PROVIDE_ਢੇਰ);
	u8 *ptr = (u8 *)ਢੇਰ_bottom;
	ulong i;

	ਲਈ (i = 0; i < CONFIG_VAL(ਮਾਪ_ਹੱਦਬੰਦੀ_PROVIDE_ਢੇਰ); i++) {
		ਜੇ (*ptr != CONFIG_VAL(SYS_ਢੇਰ_F_CHECK_BYTE))
			ਰੋਕੋ;
		ptr++;
	}
	ਲਿਖੋ("SPL initial stack usage: %lu bytes\n",
	       CONFIG_VAL(ਮਾਪ_ਹੱਦਬੰਦੀ_PROVIDE_ਢੇਰ) - i);
#ਜੇ_ਅੰਤ
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
ulong spl_relocate_ਢੇਰ_gd(ਖਾਲੀ)
{
#ਜੇ CONFIG_IS_ENABLED(ਢੇਰ_R)
	gd_t *new_gd;
	ulong ptr = CONFIG_SPL_ਢੇਰ_R_ADDR;

	ਜੇ (CONFIG_IS_ENABLED(SYS_REPORT_ਢੇਰ_F_USAGE))
		spl_relocate_ਢੇਰ_check();

#ਜੇ defined(CONFIG_SPL_SYS_MALLOC_SIMPLE) && CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ਜੇ (CONFIG_SPL_ਢੇਰ_R_MALLOC_SIMPLE_LEN) {
		debug("SPL malloc() before relocation used 0x%x bytes (%d KB)\n",
		      gd->malloc_ptr, gd->malloc_ptr / 1024);
		ptr -= CONFIG_SPL_ਢੇਰ_R_MALLOC_SIMPLE_LEN;
		gd->malloc_ਅਧਾਰ = ptr;
		gd->malloc_ਹੱਦਬੰਦੀ = CONFIG_SPL_ਢੇਰ_R_MALLOC_SIMPLE_LEN;
		gd->malloc_ptr = 0;
	}
#ਜੇ_ਅੰਤ
	/* Get stack position: use 8-byte alignment for ABI compliance */
	ptr -= roundup(ਆਕਾਰ(gd_t), 16);
	gd->ਸ਼ੁਰੂ_addr_sp = ptr;
	new_gd = (gd_t *)ptr;
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(new_gd, (ਖਾਲੀ *)gd, ਆਕਾਰ(gd_t));
#ਜੇ CONFIG_IS_ENABLED(DM)
	dm_fixup_for_gd_move(new_gd);
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(LOG)
	log_fixup_for_gd_move(new_gd);
#ਜੇ_ਅੰਤ
#ਜੇ !defined(CONFIG_ARM) && !defined(CONFIG_RISCV)
	gd = new_gd;
#ਜੇ_ਅੰਤ
	ਵਾਪਸ ptr;
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
}

#ਜੇ defined(CONFIG_BOOTCOUNT_ਹੱਦਬੰਦੀ) && \
	((!defined(CONFIG_TPL_BUILD) && !defined(CONFIG_SPL_BOOTCOUNT_ਹੱਦਬੰਦੀ)) || \
	 (defined(CONFIG_TPL_BUILD) && !defined(CONFIG_TPL_BOOTCOUNT_ਹੱਦਬੰਦੀ)))
ਖਾਲੀ bootcount_store(ulong a)
{
}

ulong bootcount_ਲੱਦੋ(ਖਾਲੀ)
{
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ
