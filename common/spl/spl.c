// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */

#అన్తర్భావ <config.h>
#అన్తర్భావ <bloblist.h>
#అన్తర్భావ <binman_sym.h>
#అన్తర్భావ <bootstage.h>
#అన్తర్భావ <dm.h>
#అన్తర్భావ <handoff.h>
#అన్తర్భావ <hang.h>
#అన్తర్భావ <init.h>
#అన్తర్భావ <irq_func.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <mapmem.h>
#అన్తర్భావ <of_live.h>
#అన్తర్భావ <serial.h>
#అన్తర్భావ <spl.h>
#అన్తర్భావ <spl_load.h>
#అన్తర్భావ <system-constants.h>
#అన్తర్భావ <asm/global_data.h>
#అన్తర్భావ <asm-generic/gpio.h>
#అన్తర్భావ <nand.h>
#అన్తర్భావ <fat.h>
#అన్తర్భావ <u-boot/crc.h>
#ఒకవేళ CONFIG_IS_ENABLED(BANNER_PRINT)
#అన్తర్భావ <timestamp.h>
#అయితే_ముగింపు
#అన్తర్భావ <version.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <malloc.h>
#అన్తర్భావ <mapmem.h>
#అన్తర్భావ <dm/root.h>
#అన్తర్భావ <dm/util.h>
#అన్తర్భావ <dm/device-internal.h>
#అన్తర్భావ <dm/uclass-internal.h>
#అన్తర్భావ <linux/compiler.h>
#అన్తర్భావ <fdt_support.h>
#అన్తర్భావ <bootcount.h>
#అన్తర్భావ <wdt.h>
#అన్తర్భావ <video.h>

DECLARE_GLOBAL_దత్తాంశం_PTR;
DECLARE_BINMAN_MAGIC_SYM;

u32 *boot_params_ptr = ఖాళీ;

#ఒకవేళ CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS)
/* See spl.h for information about this */
#ఒకవేళ defined(CONFIG_SPL_BUILD)
binman_sym_declare(ulong, u_boot_any, image_pos);
binman_sym_declare(ulong, u_boot_any, కొలత);
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_TPL
binman_sym_declare(ulong, u_boot_spl_any, image_pos);
binman_sym_declare(ulong, u_boot_spl_any, కొలత);
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_VPL
binman_sym_declare(ulong, u_boot_vpl_any, image_pos);
binman_sym_declare(ulong, u_boot_vpl_any, కొలత);
#అయితే_ముగింపు

#అయితే_ముగింపు /* BINMAN_UBOOT_SYMBOLS */

/* Define board data structure */
స్థిర నిర్మాణం bd_info bdata __attribute__ ((section(".data")));

#ఒకవేళ CONFIG_IS_ENABLED(SHOW_BOOT_పురోగతి)
/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
__weak శూన్య show_boot_పురోగతి(పూర్ణాంకం val) {}
#అయితే_ముగింపు

#ఒకవేళ defined(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) || \
	defined(CONFIG_SPL_ATF)
/* weak, default platform-specific function to initialize dram banks */
__weak పూర్ణాంకం dram_init_banksize(శూన్య)
{
	తిరిగి 0;
}
#అయితే_ముగింపు

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
#ఒకవేళ CONFIG_IS_ENABLED(OS_BOOT)
__weak పూర్ణాంకం spl_ప్రారంభం_uboot(శూన్య)
{
	puts(PHASE_PROMPT
	     "Please implement spl_start_uboot() for your board\n");
	puts(PHASE_PROMPT "Direct Linux boot not active!\n");
	తిరిగి 1;
}

/*
 * Weak default function for arch specific zImage check. Return zero
 * and fill start and end address if image is recognized.
 */
పూర్ణాంకం __weak bootz_అమరిక(ulong image, ulong *ప్రారంభం, ulong *చివర_2)
{
	 తిరిగి 1;
}

పూర్ణాంకం __weak booti_అమరిక(ulong image, ulong *relocated_addr, ulong *కొలత, bool force_reloc)
{
	 తిరిగి 1;
}
#అయితే_ముగింపు

/* Weak default function for arch specific fixups to the spl_image_info */
శూన్య __weak spl_perform_arch_fixups(నిర్మాణం spl_image_info *spl_image)
{
}

/* Weak default function for board specific fixups to the spl_image_info */
శూన్య __weak spl_perform_board_fixups(నిర్మాణం spl_image_info *spl_image)
{
}

శూన్య spl_fixup_fdt(శూన్య *fdt_blob)
{
#ఒకవేళ defined(CONFIG_SPL_యొక్క_LIBFDT)
	పూర్ణాంకం err;

	ఒకవేళ (!fdt_blob)
		తిరిగి;

	err = fdt_check_శీర్షిక(fdt_blob);
	ఒకవేళ (err < 0) {
		రాయి("fdt_root: %s\n", fdt_strerror(err));
		తిరిగి;
	}

	/* fixup the memory dt node */
	err = fdt_shrink_గమ్యం_minimum(fdt_blob, 0);
	ఒకవేళ (err == 0) {
		రాయి(PHASE_PROMPT "fdt_shrink_to_minimum err - %d\n", err);
		తిరిగి;
	}

	err = arch_fixup_fdt(fdt_blob);
	ఒకవేళ (err) {
		రాయి(PHASE_PROMPT "arch_fixup_fdt err - %d\n", err);
		తిరిగి;
	}
#అయితే_ముగింపు
}

పూర్ణాంకం spl_reserve_video_from_ram_శిఖరం(శూన్య)
{
	ఒకవేళ (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		పూర్ణాంకం ret;

		addr = gd->ram_శిఖరం;
		ret = video_reserve(&addr);
		ఒకవేళ (ret)
			తిరిగి ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((గుర్తులేని దీర్ఘ)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	తిరిగి 0;
}

ulong spl_get_image_pos(శూన్య)
{
	ఒకవేళ (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		తిరిగి BINMAN_SYM_MISSING;

#నిర్వచితమైతే CONFIG_VPL
	ఒకవేళ (xpl_తదుపరి_phase() == PHASE_VPL)
		తిరిగి binman_sym(ulong, u_boot_vpl_any, image_pos);
#అయితే_ముగింపు
#ఒకవేళ defined(CONFIG_TPL) && !defined(CONFIG_VPL)
	ఒకవేళ (xpl_తదుపరి_phase() == PHASE_SPL)
		తిరిగి binman_sym(ulong, u_boot_spl_any, image_pos);
#అయితే_ముగింపు
#ఒకవేళ defined(CONFIG_SPL_BUILD)
	తిరిగి binman_sym(ulong, u_boot_any, image_pos);
#అయితే_ముగింపు

	తిరిగి BINMAN_SYM_MISSING;
}

ulong spl_get_image_కొలత(శూన్య)
{
	ఒకవేళ (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		తిరిగి BINMAN_SYM_MISSING;

#నిర్వచితమైతే CONFIG_VPL
	ఒకవేళ (xpl_తదుపరి_phase() == PHASE_VPL)
		తిరిగి binman_sym(ulong, u_boot_vpl_any, కొలత);
#అయితే_ముగింపు
	తిరిగి xpl_తదుపరి_phase() == PHASE_SPL ?
		binman_sym(ulong, u_boot_spl_any, కొలత) :
		binman_sym(ulong, u_boot_any, కొలత);
}

ulong spl_get_image_పాఠ్యం_పునాది(శూన్య)
{
#నిర్వచితమైతే CONFIG_VPL
	ఒకవేళ (xpl_తదుపరి_phase() == PHASE_VPL)
		తిరిగి CONFIG_VPL_పాఠ్యం_పునాది;
#అయితే_ముగింపు
	తిరిగి xpl_తదుపరి_phase() == PHASE_SPL ? CONFIG_SPL_పాఠ్యం_పునాది :
		CONFIG_పాఠ్యం_పునాది;
}

/*
 * Weak default function for board specific cleanup/preparation before
 * Linux boot. Some boards/platforms might not need it, so just provide
 * an empty stub here.
 */
__weak శూన్య spl_board_prepare_for_linux(శూన్య)
{
	/* Nothing to do! */
}

__weak శూన్య spl_board_prepare_for_optee(శూన్య *fdt)
{
}

__weak స్థిరరాశి అక్షరం *spl_board_loader_name(u32 boot_device)
{
	తిరిగి ఖాళీ;
}

#ఒకవేళ CONFIG_IS_ENABLED(OPTEE_IMAGE)
__weak శూన్య __noreturn jump_గమ్యం_image_optee(నిర్మాణం spl_image_info *spl_image)
{
	spl_optee_నమోదు(ఖాళీ, ఖాళీ, spl_image->fdt_addr,
			(శూన్య *)spl_image->నమోదు_బిందువు);
}
#అయితే_ముగింపు

__weak శూన్య spl_board_prepare_for_boot(శూన్య)
{
	/* Nothing to do! */
}

__weak నిర్మాణం legacy_img_hdr *spl_get_ఎక్కించు_బఫరు(ssize_t స్థానభ్రంశం, size_t కొలత)
{
	తిరిగి map_sysmem(CONFIG_పాఠ్యం_పునాది + స్థానభ్రంశం, 0);
}

శూన్య spl_సమితి_శీర్షిక_raw_uboot(నిర్మాణం spl_image_info *spl_image)
{
	ulong u_boot_pos = spl_get_image_pos();

#ఒకవేళ CONFIG_SYS_MONITOR_LEN != 0
	spl_image->కొలత = CONFIG_SYS_MONITOR_LEN;
#లేకపోతే
	/* Unknown U-Boot size, let's assume it will not be more than 200 KB */
	spl_image->కొలత = 200 * 1024;
#అయితే_ముగింపు

	/*
	 * Binman error cases: address of the end of the previous region or the
	 * start of the image's entry area (usually 0) if there is no previous
	 * region.
	 */
	ఒకవేళ (u_boot_pos && u_boot_pos != BINMAN_SYM_MISSING) {
		/* Binman does not support separated entry addresses */
		spl_image->నమోదు_బిందువు = spl_get_image_పాఠ్యం_పునాది();
		spl_image->ఎక్కించు_addr = spl_get_image_పాఠ్యం_పునాది();
		spl_image->కొలత = spl_get_image_కొలత();
		log_debug("Next load addr %lx\n", spl_image->ఎక్కించు_addr);
	} లేకపోతే {
		spl_image->నమోదు_బిందువు = CONFIG_SYS_UBOOT_ప్రారంభం;
		spl_image->ఎక్కించు_addr = CONFIG_పాఠ్యం_పునాది;
		log_debug("Default load addr %lx (u_boot_pos=%lx)\n",
			  spl_image->ఎక్కించు_addr, u_boot_pos);
	}
	spl_image->os = IH_OS_U_BOOT;
	spl_image->name = xpl_name(xpl_తదుపరి_phase());
	log_debug("Next phase: %s at %lx size %lx\n", spl_image->name,
		  spl_image->ఎక్కించు_addr, (ulong)spl_image->కొలత);
}

__weak పూర్ణాంకం spl_parse_board_శీర్షిక(నిర్మాణం spl_image_info *spl_image,
				  స్థిరరాశి నిర్మాణం spl_boot_device *bootdev,
				  స్థిరరాశి శూన్య *image_శీర్షిక, size_t కొలత)
{
	తిరిగి -EINVAL;
}

__weak పూర్ణాంకం spl_parse_legacy_శీర్షిక(నిర్మాణం spl_image_info *spl_image,
				   స్థిరరాశి నిర్మాణం legacy_img_hdr *శీర్షిక)
{
	/* LEGACY image not supported */
	debug("Legacy boot image support not enabled, proceeding to other boot methods\n");
	తిరిగి -EINVAL;
}

పూర్ణాంకం spl_parse_image_శీర్షిక(నిర్మాణం spl_image_info *spl_image,
			   స్థిరరాశి నిర్మాణం spl_boot_device *bootdev,
			   స్థిరరాశి నిర్మాణం legacy_img_hdr *శీర్షిక)
{
	పూర్ణాంకం ret;

	ఒకవేళ (CONFIG_IS_ENABLED(ఎక్కించు_FIT_నిండిన)) {
		ret = spl_ఎక్కించు_fit_image(spl_image, శీర్షిక);

		ఒకవేళ (!ret)
			తిరిగి ret;
	}
	ఒకవేళ (image_get_magic(శీర్షిక) == IH_MAGIC) {
		పూర్ణాంకం ret;

		ret = spl_parse_legacy_శీర్షిక(spl_image, శీర్షిక);
		ఒకవేళ (ret)
			తిరిగి ret;
		తిరిగి 0;
	}

	ఒకవేళ (IS_ENABLED(CONFIG_SPL_PANIC_ON_RAW_IMAGE)) {
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

	ఒకవేళ (IS_ENABLED(CONFIG_SPL_OS_BOOT) && IS_ENABLED(CONFIG_SPL_BOOTI)) {
		ulong ప్రారంభం, కొలత;

		ఒకవేళ (!booti_అమరిక((ulong)శీర్షిక, &ప్రారంభం, &కొలత, 0)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->ఎక్కించు_addr = ప్రారంభం;
			spl_image->నమోదు_బిందువు = ప్రారంభం;
			spl_image->కొలత = కొలత;
			debug(PHASE_PROMPT
			      "payload Image, load addr: 0x%lx size: %d\n",
			      spl_image->ఎక్కించు_addr, spl_image->కొలత);
			తిరిగి 0;
		}
	} లేకపోతే ఒకవేళ (IS_ENABLED(CONFIG_SPL_OS_BOOT) &&
		   IS_ENABLED(CONFIG_SPL_BOOTZ)) {
		ulong ప్రారంభం, చివర_2;

		ఒకవేళ (!bootz_అమరిక((ulong)శీర్షిక, &ప్రారంభం, &చివర_2)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->ఎక్కించు_addr = CONFIG_SYS_ఎక్కించు_ADDR;
			spl_image->నమోదు_బిందువు = CONFIG_SYS_ఎక్కించు_ADDR;
			spl_image->కొలత = చివర_2 - ప్రారంభం;
			debug(PHASE_PROMPT
			      "payload zImage, load addr: 0x%lx size: %d\n",
			      spl_image->ఎక్కించు_addr, spl_image->కొలత);
			తిరిగి 0;
		}
	}

	ఒకవేళ (!spl_parse_board_శీర్షిక(spl_image, bootdev, (స్థిరరాశి శూన్య *)శీర్షిక,
				    పరిమాణం(*శీర్షిక)))
		తిరిగి 0;

	ఒకవేళ (IS_ENABLED(CONFIG_SPL_RAW_IMAGE_SUPPORT)) {
		/* Signature not found - assume u-boot.bin */
		debug("mkimage signature not found - ih_magic = %x\n",
		      శీర్షిక->ih_magic);
		spl_సమితి_శీర్షిక_raw_uboot(spl_image);
	} లేకపోతే {
		/* RAW image not supported, proceed to other boot methods. */
		debug("Raw boot image support not enabled, proceeding to other boot methods\n");
		తిరిగి -EINVAL;
	}

	తిరిగి 0;
}

#ఒకవేళ SPL_ఎక్కించు_USERS > 1
పూర్ణాంకం spl_ఎక్కించు(నిర్మాణం spl_image_info *spl_image,
	     స్థిరరాశి నిర్మాణం spl_boot_device *bootdev, నిర్మాణం spl_ఎక్కించు_info *info,
	     size_t కొలత, size_t స్థానభ్రంశం)
{
	తిరిగి spl_ఎక్కించు_2(spl_image, bootdev, info, కొలత, స్థానభ్రంశం);
}
#అయితే_ముగింపు

__weak శూన్య __noreturn jump_గమ్యం_image(నిర్మాణం spl_image_info *spl_image)
{
	రకనిర్వచనం శూన్య __noreturn (*image_నమోదు_noargs_t)(శూన్య);

	image_నమోదు_noargs_t image_నమోదు =
		(image_నమోదు_noargs_t)spl_image->నమోదు_బిందువు;

	debug("image entry point: 0x%lx\n", spl_image->నమోదు_బిందువు);
	image_నమోదు();
}

#ఒకవేళ CONFIG_IS_ENABLED(HANDOFF)
/**
 * Set up the SPL hand-off information
 *
 * This is initially empty (zero) but can be written by
 */
స్థిర పూర్ణాంకం అమరిక_spl_handoff(శూన్య)
{
	నిర్మాణం spl_handoff *ho;

	ho = bloblist_ensure(BLOBLISTT_U_BOOT_SPL_HANDOFF, పరిమాణం(నిర్మాణం spl_handoff));
	ఒకవేళ (!ho)
		తిరిగి -ENOENT;

	తిరిగి 0;
}

__weak పూర్ణాంకం handoff_arch_save(నిర్మాణం spl_handoff *ho)
{
	తిరిగి 0;
}

స్థిర పూర్ణాంకం write_spl_handoff(శూన్య)
{
	నిర్మాణం spl_handoff *ho;
	పూర్ణాంకం ret;

	ho = bloblist_కనుగొను(BLOBLISTT_U_BOOT_SPL_HANDOFF, పరిమాణం(నిర్మాణం spl_handoff));
	ఒకవేళ (!ho)
		తిరిగి -ENOENT;
	handoff_save_dram(ho);
	ret = handoff_arch_save(ho);
	ఒకవేళ (ret)
		తిరిగి ret;
	debug(PHASE_PROMPT "Wrote SPL handoff\n");

	తిరిగి 0;
}
#లేకపోతే
స్థిర అంతఃవరుస పూర్ణాంకం అమరిక_spl_handoff(శూన్య) { తిరిగి 0; }
స్థిర అంతఃవరుస పూర్ణాంకం write_spl_handoff(శూన్య) { తిరిగి 0; }

#అయితే_ముగింపు /* HANDOFF */

/**
 * get_bootstage_id() - Get the bootstage ID to emit
 *
 * @start: true if this is for starting SPL, false for ending it
 * Return: bootstage ID to use
 */
స్థిర గణన bootstage_id get_bootstage_id(bool ప్రారంభం)
{
	గణన xpl_phase_t phase = xpl_phase();

	ఒకవేళ (IS_ENABLED(CONFIG_TPL_BUILD) && phase == PHASE_TPL)
		తిరిగి ప్రారంభం ? BOOTSTAGE_ID_ప్రారంభం_TPL : BOOTSTAGE_ID_చివర_TPL;
	లేకపోతే ఒకవేళ (IS_ENABLED(CONFIG_VPL_BUILD) && phase == PHASE_VPL)
		తిరిగి ప్రారంభం ? BOOTSTAGE_ID_ప్రారంభం_VPL : BOOTSTAGE_ID_చివర_VPL;
	లేకపోతే
		తిరిగి ప్రారంభం ? BOOTSTAGE_ID_ప్రారంభం_SPL : BOOTSTAGE_ID_చివర_SPL;
}

స్థిర పూర్ణాంకం spl_common_init(bool అమరిక_malloc)
{
	పూర్ణాంకం ret;

#ఒకవేళ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ఒకవేళ (అమరిక_malloc) {
#నిర్వచితమైతే CFG_MALLOC_F_ADDR
		gd->malloc_పునాది = CFG_MALLOC_F_ADDR;
#అయితే_ముగింపు
		gd->malloc_హద్దు = CONFIG_VAL(SYS_MALLOC_F_LEN);
		gd->malloc_ptr = 0;
	}
#అయితే_ముగింపు
	ret = bootstage_init(xpl_is_మొదటి_phase());
	ఒకవేళ (ret) {
		debug("%s: Failed to set up bootstage: ret=%d\n", __func__,
		      ret);
		తిరిగి ret;
	}
	ఒకవేళ (!xpl_is_మొదటి_phase()) {
		ret = bootstage_unstash_default();
		ఒకవేళ (ret)
			log_debug("Failed to unstash bootstage: ret=%d\n", ret);
	}
	bootstage_mark_name(get_bootstage_id(true), xpl_name(xpl_phase()));
#ఒకవేళ CONFIG_IS_ENABLED(LOG)
	ret = log_init();
	ఒకవేళ (ret) {
		debug("%s: Failed to set up logging\n", __func__);
		తిరిగి ret;
	}
#అయితే_ముగింపు
	ఒకవేళ (CONFIG_IS_ENABLED(యొక్క_REAL)) {
		ret = fdtdec_అమరిక();
		ఒకవేళ (ret) {
			debug("fdtdec_setup() returned error %d\n", ret);
			తిరిగి ret;
		}
	}
	ఒకవేళ (CONFIG_IS_ENABLED(యొక్క_సజీవ)) {
		bootstage_ప్రారంభం(BOOTSTAGE_ID_ACCUM_యొక్క_సజీవ, "of_live");
		ret = యొక్క_సజీవ_build(gd->fdt_blob,
				    (నిర్మాణం device_కణుపు **)gd_యొక్క_మూలం_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_యొక్క_సజీవ);
		ఒకవేళ (ret)
			తిరిగి ret;
	}
	ఒకవేళ (CONFIG_IS_ENABLED(DM)) {
		bootstage_ప్రారంభం(BOOTSTAGE_ID_ACCUM_DM_SPL,
				xpl_phase() == PHASE_TPL ? "dm tpl" : "dm_spl");
		/* With CONFIG_SPL_OF_PLATDATA, bring in all devices */
		ret = dm_init_and_పరిశీలన(!CONFIG_IS_ENABLED(యొక్క_PLATDATA));
		bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_SPL);
		ఒకవేళ (ret) {
			debug("dm_init_and_scan() returned error %d\n", ret);
			తిరిగి ret;
		}

		ret = dm_autoprobe();
		ఒకవేళ (ret)
			తిరిగి ret;
	}

	తిరిగి 0;
}

శూన్య spl_సమితి_bd(శూన్య)
{
	/*
	 * NOTE: On some platforms (e.g. x86) bdata may be in flash and not
	 * writeable.
	 */
	ఒకవేళ (!gd->bd)
		gd->bd = &bdata;
}

పూర్ణాంకం spl_early_init(శూన్య)
{
	పూర్ణాంకం ret;

	debug("%s\n", __func__);

	ret = spl_common_init(true);
	ఒకవేళ (ret)
		తిరిగి ret;
	gd->flags |= GD_FLG_SPL_EARLY_INIT;

	తిరిగి 0;
}

పూర్ణాంకం spl_init(శూన్య)
{
	పూర్ణాంకం ret;
	bool అమరిక_malloc = !(IS_ENABLED(CONFIG_SPL_దొంతర_R) &&
			IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIMPLE));

	debug("%s\n", __func__);

	ఒకవేళ (!(gd->flags & GD_FLG_SPL_EARLY_INIT)) {
		ret = spl_common_init(అమరిక_malloc);
		ఒకవేళ (ret)
			తిరిగి ret;
	}
	gd->flags |= GD_FLG_SPL_INIT;

	తిరిగి 0;
}

#నిర్వచితం_కాకపోతే BOOT_DEVICE_NONE
#నిర్వచించు BOOT_DEVICE_NONE 0xdeadbeef
#అయితే_ముగింపు

__weak శూన్య board_boot_క్రమం(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();
}

__weak పూర్ణాంకం spl_check_board_image(నిర్మాణం spl_image_info *spl_image,
				 స్థిరరాశి నిర్మాణం spl_boot_device *bootdev)
{
	తిరిగి 0;
}

స్థిర పూర్ణాంకం spl_ఎక్కించు_image(నిర్మాణం spl_image_info *spl_image,
			  నిర్మాణం spl_image_loader *loader)
{
	పూర్ణాంకం ret;
	నిర్మాణం spl_boot_device bootdev;

	bootdev.boot_device = loader->boot_device;
	bootdev.boot_device_name = ఖాళీ;

	ret = loader->ఎక్కించు_image(spl_image, &bootdev);
#నిర్వచితమైతే CONFIG_SPL_LEGACY_IMAGE_చక్రీయ_తనిఖీ_CHECK
	ఒకవేళ (!ret && spl_image->dcrc_నిడివి) {
		/* check data crc */
		ulong dcrc = చక్రీయ_తనిఖీ_32_wd(0, (గుర్తులేని అక్షరం *)spl_image->dcrc_దత్తాంశం,
				      spl_image->dcrc_నిడివి, CHUNKSZ_చక్రీయ_తనిఖీ_32);
		ఒకవేళ (dcrc != spl_image->dcrc) {
			puts("SPL: Image data CRC check failed!\n");
			ret = -EINVAL;
		}
	}
#అయితే_ముగింపు
	ఒకవేళ (!ret)
		ret = spl_check_board_image(spl_image, &bootdev);

	తిరిగి ret;
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
స్థిర పూర్ణాంకం boot_from_devices(నిర్మాణం spl_image_info *spl_image,
			     u32 spl_boot_list[], పూర్ణాంకం లెక్క)
{
	నిర్మాణం spl_image_loader *drv =
		ll_నమోదు_ప్రారంభం(నిర్మాణం spl_image_loader, spl_image_loader);
	స్థిరరాశి పూర్ణాంకం n_ents =
		ll_నమోదు_లెక్క(నిర్మాణం spl_image_loader, spl_image_loader);
	పూర్ణాంకం ret = -ENODEV;
	పూర్ణాంకం i;

	కొరకు (i = 0; i < లెక్క && spl_boot_list[i] != BOOT_DEVICE_NONE; i++) {
		నిర్మాణం spl_image_loader *loader;
		పూర్ణాంకం bootdev = spl_boot_list[i];

		ఒకవేళ (CONFIG_IS_ENABLED(SHOW_ERRORS))
			ret = -ENXIO;
		కొరకు (loader = drv; loader != drv + n_ents; loader++) {
			ఒకవేళ (loader && bootdev != loader->boot_device)
				కొనసాగు;
			ఒకవేళ (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
				రాయి("Trying to boot from %s\n",
				       spl_loader_name(loader));
			}

			ret = spl_ఎక్కించు_image(spl_image, loader);
			ఒకవేళ (!ret) {
				spl_image->boot_device = bootdev;
				తిరిగి 0;
			}
			రాయి("Error: %d\n", ret);
		}

		ఒకవేళ (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
			ఒకవేళ (CONFIG_IS_ENABLED(SHOW_ERRORS))
				రాయి(PHASE_PROMPT
				       "Unsupported Boot Device %d\n",
				       bootdev);
			లేకపోతే
				రాయి(PHASE_PROMPT
				       "Unsupported Boot Device!\n");
		}
	}

	తిరిగి ret;
}

శూన్య board_init_r(gd_t *dummy1, ulong dummy2)
{
	u32 spl_boot_list[] = {
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
	};
	spl_jump_గమ్యం_image_t jumper = &jump_గమ్యం_image;
	నిర్మాణం spl_image_info spl_image;
	పూర్ణాంకం ret, os;
	శూన్య *fdt;

	debug(">>" PHASE_PROMPT "board_init_r()\n");

	spl_సమితి_bd();

	ఒకవేళ (IS_ENABLED(CONFIG_SPL_SYS_MALLOC)) {
		mem_malloc_init(SPL_SYS_MALLOC_ప్రారంభం, SPL_SYS_MALLOC_కొలత);
		gd->flags |= GD_FLG_నిండిన_MALLOC_INIT;
	}
	ఒకవేళ (!(gd->flags & GD_FLG_SPL_INIT)) {
		ఒకవేళ (spl_init())
			hang();
	}
	timer_init();
	ఒకవేళ (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_init();
		ఒకవేళ (ret) {
			debug("%s: Failed to set up bloblist: ret=%d\n",
			      __func__, ret);
			puts(PHASE_PROMPT "Cannot set up bloblist\n");
			hang();
		}
	}
	ఒకవేళ (CONFIG_IS_ENABLED(HANDOFF)) {
		పూర్ణాంకం ret;

		ret = అమరిక_spl_handoff();
		ఒకవేళ (ret) {
			puts(PHASE_PROMPT "Cannot set up SPL handoff\n");
			hang();
		}
	}

	ఒకవేళ (CONFIG_IS_ENABLED(SOC_INIT))
		spl_soc_init();

	ఒకవేళ (IS_ENABLED(CONFIG_SPL_WATCHDOG) && CONFIG_IS_ENABLED(WDT))
		initr_watchdog();

	ఒకవేళ (IS_ENABLED(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) ||
	    IS_ENABLED(CONFIG_SPL_ATF) || IS_ENABLED(CONFIG_SPL_NET))
		dram_init_banksize();

	ఒకవేళ (IS_ENABLED(CONFIG_SPL_LMB))
		lmb_init();

	ఒకవేళ (CONFIG_IS_ENABLED(PCI) && !(gd->flags & GD_FLG_DM_DEAD)) {
		ret = pci_init();
		ఒకవేళ (ret)
			puts(PHASE_PROMPT "Cannot initialize PCI\n");
		/* Don't fail. We still can try other boot methods. */
	}

	ఒకవేళ (CONFIG_IS_ENABLED(BOARD_INIT))
		spl_board_init();

	bootcount_inc();

	/* Dump driver model states to aid analysis */
	ఒకవేళ (CONFIG_IS_ENABLED(DM_STATS)) {
		నిర్మాణం dm_stats mem;

		dm_get_mem(&mem);
		dm_dump_mem(&mem);
	}

	memset(&spl_image, '\0', పరిమాణం(spl_image));
	ఒకవేళ (IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image.arg = (శూన్య *)SPL_PAYLOAD_వాదనలు_ADDR;
	spl_image.boot_device = BOOT_DEVICE_NONE;
	board_boot_క్రమం(spl_boot_list);

	ret = boot_from_devices(&spl_image, spl_boot_list,
				ARRAY_కొలత(spl_boot_list));
	ఒకవేళ (ret) {
		ఒకవేళ (CONFIG_IS_ENABLED(SHOW_ERRORS))
			రాయి(PHASE_PROMPT "failed to boot from all boot devices (err=%d)\n",
			       ret);
		లేకపోతే
			puts(PHASE_PROMPT "failed to boot from all boot devices\n");
		hang();
	}

	spl_perform_arch_fixups(&spl_image);
	spl_perform_board_fixups(&spl_image);

	os = spl_image.os;
	ఒకవేళ (os == IH_OS_U_BOOT) {
		debug("Jumping to %s...\n", xpl_name(xpl_తదుపరి_phase()));
	} లేకపోతే ఒకవేళ (CONFIG_IS_ENABLED(ATF) && os == IH_OS_ARM_TRUSTED_FIRMWARE) {
		debug("Jumping to U-Boot via ARM Trusted Firmware\n");
		spl_fixup_fdt(spl_image_fdt_addr(&spl_image));
		jumper = &spl_invoke_atf;
	} లేకపోతే ఒకవేళ (CONFIG_IS_ENABLED(OPTEE_IMAGE) && os == IH_OS_TEE) {
		debug("Jumping to U-Boot via OP-TEE\n");
		spl_board_prepare_for_optee(spl_image_fdt_addr(&spl_image));
		jumper = &jump_గమ్యం_image_optee;
	} లేకపోతే ఒకవేళ (CONFIG_IS_ENABLED(OPENSBI) && os == IH_OS_OPENSBI) {
		debug("Jumping to U-Boot via RISC-V OpenSBI\n");
		jumper = &spl_invoke_opensbi;
	} లేకపోతే ఒకవేళ (CONFIG_IS_ENABLED(OS_BOOT) && os == IH_OS_LINUX) {
		debug("Jumping to Linux\n");
		ఒకవేళ (CONFIG_IS_ENABLED(OS_BOOT_వాదనలు))
			fdt = (శూన్య *)SPL_PAYLOAD_వాదనలు_ADDR;
		లేకపోతే
			fdt = spl_image_fdt_addr(&spl_image);
		spl_fixup_fdt(fdt);
		spl_board_prepare_for_linux();
		spl_image.arg = fdt;
		jumper = &jump_గమ్యం_image_linux;
	} లేకపోతే {
		debug("Unsupported OS image.. Jumping nevertheless..\n");
	}
	ఒకవేళ (CONFIG_IS_ENABLED(SYS_MALLOC_F) &&
	    !IS_ENABLED(CONFIG_SPL_SYS_MALLOC_కొలత))
		debug("SPL malloc() used 0x%x bytes (%d KB)\n",
		      gd_malloc_ptr(), gd_malloc_ptr() / 1024);

	bootstage_mark_name(get_bootstage_id(false), "end phase");
	ret = bootstage_stash_default();
	ఒకవేళ (ret)
		debug("Failed to stash bootstage: err=%d\n", ret);

	ఒకవేళ (IS_ENABLED(CONFIG_SPL_VIDEO_REMOVE)) {
		నిర్మాణం udevice *dev;
		పూర్ణాంకం rc;

		rc = uclass_కనుగొను_device(UCLASS_VIDEO, 0, &dev);
		ఒకవేళ (!rc && dev) {
			rc = device_remove(dev, DM_REMOVE_సాధారణ);
			ఒకవేళ (rc)
				రాయి("Cannot remove video device '%s' (err=%d)\n",
				       dev->name, rc);
		}
	}
	ఒకవేళ (CONFIG_IS_ENABLED(HANDOFF)) {
		ret = write_spl_handoff();
		ఒకవేళ (ret)
			రాయి(PHASE_PROMPT
			       "SPL hand-off write failed (err=%d)\n", ret);
	}
	ఒకవేళ (CONFIG_IS_ENABLED(UPL_బహిర్గతం) && (gd->flags & GD_FLG_UPL)) {
		ret = spl_write_upl_handoff(&spl_image);
		ఒకవేళ (ret) {
			రాయి(PHASE_PROMPT
			       "UPL hand-off write failed (err=%d)\n", ret);
			hang();
		}
	}
	ఒకవేళ (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_ముగింపు();
		ఒకవేళ (ret)
			రాయి("Warning: Failed to finish bloblist (ret=%d)\n",
			       ret);
	}

	spl_board_prepare_for_boot();

	ఒకవేళ (CONFIG_IS_ENABLED(RELOC_LOADER)) {
		పూర్ణాంకం ret;

		ret = spl_reloc_jump(&spl_image, jumper);
		ఒకవేళ (ret) {
			ఒకవేళ (xpl_phase() == PHASE_VPL)
				రాయి("jump failed %d\n", ret);
			hang();
		}
	}

	jumper(&spl_image);
}

/*
 * This requires UART clocks to be enabled.  In order for this to work the
 * caller must ensure that the gd pointer is valid.
 */
శూన్య preloader_console_init(శూన్య)
{
#ఒకవేళ CONFIG_IS_ENABLED(SERIAL)
	gd->baudrate = CONFIG_BAUDRATE;

	serial_init();		/* serial communications setup */

	gd->flags |= GD_FLG_HAVE_CONSOLE;

#ఒకవేళ CONFIG_IS_ENABLED(BANNER_PRINT)
	puts("\nU-Boot " PHASE_NAME " " PLAIN_సంస్కరణ " (" U_BOOT_DATE " - "
	     U_BOOT_సమయం " " U_BOOT_TZ ")\n");
#అయితే_ముగింపు
#నిర్వచితమైతే CONFIG_SPL_DISPLAY_PRINT
	spl_display_print();
#అయితే_ముగింపు
#అయితే_ముగింపు
}

/**
 * This function is called before the stack is changed from initial stack to
 * relocated stack. It tries to dump the stack size used
 */
__weak శూన్య spl_relocate_దొంతర_check(శూన్య)
{
#ఒకవేళ CONFIG_IS_ENABLED(SYS_REPORT_దొంతర_F_USAGE)
	ulong init_sp = gd->ప్రారంభం_addr_sp;
	ulong దొంతర_bottom = init_sp - CONFIG_VAL(కొలత_హద్దు_PROVIDE_దొంతర);
	u8 *ptr = (u8 *)దొంతర_bottom;
	ulong i;

	కొరకు (i = 0; i < CONFIG_VAL(కొలత_హద్దు_PROVIDE_దొంతర); i++) {
		ఒకవేళ (*ptr != CONFIG_VAL(SYS_దొంతర_F_CHECK_BYTE))
			ఆపు;
		ptr++;
	}
	రాయి("SPL initial stack usage: %lu bytes\n",
	       CONFIG_VAL(కొలత_హద్దు_PROVIDE_దొంతర) - i);
#అయితే_ముగింపు
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
ulong spl_relocate_దొంతర_gd(శూన్య)
{
#ఒకవేళ CONFIG_IS_ENABLED(దొంతర_R)
	gd_t *new_gd;
	ulong ptr = CONFIG_SPL_దొంతర_R_ADDR;

	ఒకవేళ (CONFIG_IS_ENABLED(SYS_REPORT_దొంతర_F_USAGE))
		spl_relocate_దొంతర_check();

#ఒకవేళ defined(CONFIG_SPL_SYS_MALLOC_SIMPLE) && CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ఒకవేళ (CONFIG_SPL_దొంతర_R_MALLOC_SIMPLE_LEN) {
		debug("SPL malloc() before relocation used 0x%x bytes (%d KB)\n",
		      gd->malloc_ptr, gd->malloc_ptr / 1024);
		ptr -= CONFIG_SPL_దొంతర_R_MALLOC_SIMPLE_LEN;
		gd->malloc_పునాది = ptr;
		gd->malloc_హద్దు = CONFIG_SPL_దొంతర_R_MALLOC_SIMPLE_LEN;
		gd->malloc_ptr = 0;
	}
#అయితే_ముగింపు
	/* Get stack position: use 8-byte alignment for ABI compliance */
	ptr -= roundup(పరిమాణం(gd_t), 16);
	gd->ప్రారంభం_addr_sp = ptr;
	new_gd = (gd_t *)ptr;
	జ్ఞాపకం_నకలు(new_gd, (శూన్య *)gd, పరిమాణం(gd_t));
#ఒకవేళ CONFIG_IS_ENABLED(DM)
	dm_fixup_for_gd_move(new_gd);
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(LOG)
	log_fixup_for_gd_move(new_gd);
#అయితే_ముగింపు
#ఒకవేళ !defined(CONFIG_ARM) && !defined(CONFIG_RISCV)
	gd = new_gd;
#అయితే_ముగింపు
	తిరిగి ptr;
#లేకపోతే
	తిరిగి 0;
#అయితే_ముగింపు
}

#ఒకవేళ defined(CONFIG_BOOTCOUNT_హద్దు) && \
	((!defined(CONFIG_TPL_BUILD) && !defined(CONFIG_SPL_BOOTCOUNT_హద్దు)) || \
	 (defined(CONFIG_TPL_BUILD) && !defined(CONFIG_TPL_BOOTCOUNT_హద్దు)))
శూన్య bootcount_store(ulong a)
{
}

ulong bootcount_ఎక్కించు(శూన్య)
{
	తిరిగి 0;
}
#అయితే_ముగింపు
