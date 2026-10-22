// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */

#ضمن <config.h>
#ضمن <bloblist.h>
#ضمن <binman_sym.h>
#ضمن <bootstage.h>
#ضمن <dm.h>
#ضمن <handoff.h>
#ضمن <hang.h>
#ضمن <init.h>
#ضمن <irq_func.h>
#ضمن <log.h>
#ضمن <mapmem.h>
#ضمن <of_live.h>
#ضمن <serial.h>
#ضمن <spl.h>
#ضمن <spl_load.h>
#ضمن <system-constants.h>
#ضمن <asm/global_data.h>
#ضمن <asm-generic/gpio.h>
#ضمن <nand.h>
#ضمن <fat.h>
#ضمن <u-boot/crc.h>
#إذا CONFIG_IS_ENABLED(BANNER_PRINT)
#ضمن <timestamp.h>
#پایان_اگر
#ضمن <version.h>
#ضمن <image.h>
#ضمن <malloc.h>
#ضمن <mapmem.h>
#ضمن <dm/root.h>
#ضمن <dm/util.h>
#ضمن <dm/device-internal.h>
#ضمن <dm/uclass-internal.h>
#ضمن <linux/compiler.h>
#ضمن <fdt_support.h>
#ضمن <bootcount.h>
#ضمن <wdt.h>
#ضمن <video.h>

DECLARE_GLOBAL_داده_PTR;
DECLARE_BINMAN_MAGIC_SYM;

u32 *boot_params_ptr = عدم;

#إذا CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS)
/* See spl.h for information about this */
#إذا defined(CONFIG_SPL_BUILD)
binman_sym_declare(ulong, u_boot_any, image_pos);
binman_sym_declare(ulong, u_boot_any, اندازه);
#پایان_اگر

#اگر_تعریف_شده CONFIG_TPL
binman_sym_declare(ulong, u_boot_spl_any, image_pos);
binman_sym_declare(ulong, u_boot_spl_any, اندازه);
#پایان_اگر

#اگر_تعریف_شده CONFIG_VPL
binman_sym_declare(ulong, u_boot_vpl_any, image_pos);
binman_sym_declare(ulong, u_boot_vpl_any, اندازه);
#پایان_اگر

#پایان_اگر /* BINMAN_UBOOT_SYMBOLS */

/* Define board data structure */
ساكن بنية bd_info bdata __attribute__ ((section(".data")));

#إذا CONFIG_IS_ENABLED(SHOW_BOOT_پیشرفت)
/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
__weak فراغ show_boot_پیشرفت(صحيح val) {}
#پایان_اگر

#إذا defined(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) || \
	defined(CONFIG_SPL_ATF)
/* weak, default platform-specific function to initialize dram banks */
__weak صحيح dram_init_banksize(فراغ)
{
	أرجع 0;
}
#پایان_اگر

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
#إذا CONFIG_IS_ENABLED(OS_BOOT)
__weak صحيح spl_شروع_uboot(فراغ)
{
	puts(PHASE_PROMPT
	     "Please implement spl_start_uboot() for your board\n");
	puts(PHASE_PROMPT "Direct Linux boot not active!\n");
	أرجع 1;
}

/*
 * Weak default function for arch specific zImage check. Return zero
 * and fill start and end address if image is recognized.
 */
صحيح __weak bootz_راهاندازی(ulong image, ulong *شروع, ulong *انتها_2)
{
	 أرجع 1;
}

صحيح __weak booti_راهاندازی(ulong image, ulong *relocated_addr, ulong *اندازه, bool force_reloc)
{
	 أرجع 1;
}
#پایان_اگر

/* Weak default function for arch specific fixups to the spl_image_info */
فراغ __weak spl_perform_arch_fixups(بنية spl_image_info *spl_image)
{
}

/* Weak default function for board specific fixups to the spl_image_info */
فراغ __weak spl_perform_board_fixups(بنية spl_image_info *spl_image)
{
}

فراغ spl_fixup_fdt(فراغ *fdt_blob)
{
#إذا defined(CONFIG_SPL_از_LIBFDT)
	صحيح err;

	إذا (!fdt_blob)
		أرجع;

	err = fdt_check_سرآیند(fdt_blob);
	إذا (err < 0) {
		چاپ_کن("fdt_root: %s\n", fdt_strerror(err));
		أرجع;
	}

	/* fixup the memory dt node */
	err = fdt_shrink_مقصد_minimum(fdt_blob, 0);
	إذا (err == 0) {
		چاپ_کن(PHASE_PROMPT "fdt_shrink_to_minimum err - %d\n", err);
		أرجع;
	}

	err = arch_fixup_fdt(fdt_blob);
	إذا (err) {
		چاپ_کن(PHASE_PROMPT "arch_fixup_fdt err - %d\n", err);
		أرجع;
	}
#پایان_اگر
}

صحيح spl_reserve_video_from_ram_بالاترین(فراغ)
{
	إذا (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		صحيح ret;

		addr = gd->ram_بالاترین;
		ret = video_reserve(&addr);
		إذا (ret)
			أرجع ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((بیعلامت دراز)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	أرجع 0;
}

ulong spl_get_image_pos(فراغ)
{
	إذا (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		أرجع BINMAN_SYM_MISSING;

#اگر_تعریف_شده CONFIG_VPL
	إذا (xpl_بعدی_phase() == PHASE_VPL)
		أرجع binman_sym(ulong, u_boot_vpl_any, image_pos);
#پایان_اگر
#إذا defined(CONFIG_TPL) && !defined(CONFIG_VPL)
	إذا (xpl_بعدی_phase() == PHASE_SPL)
		أرجع binman_sym(ulong, u_boot_spl_any, image_pos);
#پایان_اگر
#إذا defined(CONFIG_SPL_BUILD)
	أرجع binman_sym(ulong, u_boot_any, image_pos);
#پایان_اگر

	أرجع BINMAN_SYM_MISSING;
}

ulong spl_get_image_اندازه(فراغ)
{
	إذا (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		أرجع BINMAN_SYM_MISSING;

#اگر_تعریف_شده CONFIG_VPL
	إذا (xpl_بعدی_phase() == PHASE_VPL)
		أرجع binman_sym(ulong, u_boot_vpl_any, اندازه);
#پایان_اگر
	أرجع xpl_بعدی_phase() == PHASE_SPL ?
		binman_sym(ulong, u_boot_spl_any, اندازه) :
		binman_sym(ulong, u_boot_any, اندازه);
}

ulong spl_get_image_متن_پایه(فراغ)
{
#اگر_تعریف_شده CONFIG_VPL
	إذا (xpl_بعدی_phase() == PHASE_VPL)
		أرجع CONFIG_VPL_متن_پایه;
#پایان_اگر
	أرجع xpl_بعدی_phase() == PHASE_SPL ? CONFIG_SPL_متن_پایه :
		CONFIG_متن_پایه;
}

/*
 * Weak default function for board specific cleanup/preparation before
 * Linux boot. Some boards/platforms might not need it, so just provide
 * an empty stub here.
 */
__weak فراغ spl_board_prepare_for_linux(فراغ)
{
	/* Nothing to do! */
}

__weak فراغ spl_board_prepare_for_optee(فراغ *fdt)
{
}

__weak ثابت حرف *spl_board_loader_name(u32 boot_device)
{
	أرجع عدم;
}

#إذا CONFIG_IS_ENABLED(OPTEE_IMAGE)
__weak فراغ __noreturn jump_مقصد_image_optee(بنية spl_image_info *spl_image)
{
	spl_optee_مدخل(عدم, عدم, spl_image->fdt_addr,
			(فراغ *)spl_image->مدخل_نقطه);
}
#پایان_اگر

__weak فراغ spl_board_prepare_for_boot(فراغ)
{
	/* Nothing to do! */
}

__weak بنية legacy_img_hdr *spl_get_بارگیری_میانگیر(ssize_t جابه_جایی, size_t اندازه)
{
	أرجع map_sysmem(CONFIG_متن_پایه + جابه_جایی, 0);
}

فراغ spl_مجموعه_سرآیند_raw_uboot(بنية spl_image_info *spl_image)
{
	ulong u_boot_pos = spl_get_image_pos();

#إذا CONFIG_SYS_MONITOR_LEN != 0
	spl_image->اندازه = CONFIG_SYS_MONITOR_LEN;
#وإلا
	/* Unknown U-Boot size, let's assume it will not be more than 200 KB */
	spl_image->اندازه = 200 * 1024;
#پایان_اگر

	/*
	 * Binman error cases: address of the end of the previous region or the
	 * start of the image's entry area (usually 0) if there is no previous
	 * region.
	 */
	إذا (u_boot_pos && u_boot_pos != BINMAN_SYM_MISSING) {
		/* Binman does not support separated entry addresses */
		spl_image->مدخل_نقطه = spl_get_image_متن_پایه();
		spl_image->بارگیری_addr = spl_get_image_متن_پایه();
		spl_image->اندازه = spl_get_image_اندازه();
		log_debug("Next load addr %lx\n", spl_image->بارگیری_addr);
	} وإلا {
		spl_image->مدخل_نقطه = CONFIG_SYS_UBOOT_شروع;
		spl_image->بارگیری_addr = CONFIG_متن_پایه;
		log_debug("Default load addr %lx (u_boot_pos=%lx)\n",
			  spl_image->بارگیری_addr, u_boot_pos);
	}
	spl_image->os = IH_OS_U_BOOT;
	spl_image->name = xpl_name(xpl_بعدی_phase());
	log_debug("Next phase: %s at %lx size %lx\n", spl_image->name,
		  spl_image->بارگیری_addr, (ulong)spl_image->اندازه);
}

__weak صحيح spl_parse_board_سرآیند(بنية spl_image_info *spl_image,
				  ثابت بنية spl_boot_device *bootdev,
				  ثابت فراغ *image_سرآیند, size_t اندازه)
{
	أرجع -EINVAL;
}

__weak صحيح spl_parse_legacy_سرآیند(بنية spl_image_info *spl_image,
				   ثابت بنية legacy_img_hdr *سرآیند)
{
	/* LEGACY image not supported */
	debug("Legacy boot image support not enabled, proceeding to other boot methods\n");
	أرجع -EINVAL;
}

صحيح spl_parse_image_سرآیند(بنية spl_image_info *spl_image,
			   ثابت بنية spl_boot_device *bootdev,
			   ثابت بنية legacy_img_hdr *سرآیند)
{
	صحيح ret;

	إذا (CONFIG_IS_ENABLED(بارگیری_FIT_پر)) {
		ret = spl_بارگیری_fit_image(spl_image, سرآیند);

		إذا (!ret)
			أرجع ret;
	}
	إذا (image_get_magic(سرآیند) == IH_MAGIC) {
		صحيح ret;

		ret = spl_parse_legacy_سرآیند(spl_image, سرآیند);
		إذا (ret)
			أرجع ret;
		أرجع 0;
	}

	إذا (IS_ENABLED(CONFIG_SPL_PANIC_ON_RAW_IMAGE)) {
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

	إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT) && IS_ENABLED(CONFIG_SPL_BOOTI)) {
		ulong شروع, اندازه;

		إذا (!booti_راهاندازی((ulong)سرآیند, &شروع, &اندازه, 0)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->بارگیری_addr = شروع;
			spl_image->مدخل_نقطه = شروع;
			spl_image->اندازه = اندازه;
			debug(PHASE_PROMPT
			      "payload Image, load addr: 0x%lx size: %d\n",
			      spl_image->بارگیری_addr, spl_image->اندازه);
			أرجع 0;
		}
	} وإلا إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT) &&
		   IS_ENABLED(CONFIG_SPL_BOOTZ)) {
		ulong شروع, انتها_2;

		إذا (!bootz_راهاندازی((ulong)سرآیند, &شروع, &انتها_2)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->بارگیری_addr = CONFIG_SYS_بارگیری_ADDR;
			spl_image->مدخل_نقطه = CONFIG_SYS_بارگیری_ADDR;
			spl_image->اندازه = انتها_2 - شروع;
			debug(PHASE_PROMPT
			      "payload zImage, load addr: 0x%lx size: %d\n",
			      spl_image->بارگیری_addr, spl_image->اندازه);
			أرجع 0;
		}
	}

	إذا (!spl_parse_board_سرآیند(spl_image, bootdev, (ثابت فراغ *)سرآیند,
				    حجم(*سرآیند)))
		أرجع 0;

	إذا (IS_ENABLED(CONFIG_SPL_RAW_IMAGE_SUPPORT)) {
		/* Signature not found - assume u-boot.bin */
		debug("mkimage signature not found - ih_magic = %x\n",
		      سرآیند->ih_magic);
		spl_مجموعه_سرآیند_raw_uboot(spl_image);
	} وإلا {
		/* RAW image not supported, proceed to other boot methods. */
		debug("Raw boot image support not enabled, proceeding to other boot methods\n");
		أرجع -EINVAL;
	}

	أرجع 0;
}

#إذا SPL_بارگیری_USERS > 1
صحيح spl_بارگیری(بنية spl_image_info *spl_image,
	     ثابت بنية spl_boot_device *bootdev, بنية spl_بارگیری_info *info,
	     size_t اندازه, size_t جابه_جایی)
{
	أرجع spl_بارگیری_2(spl_image, bootdev, info, اندازه, جابه_جایی);
}
#پایان_اگر

__weak فراغ __noreturn jump_مقصد_image(بنية spl_image_info *spl_image)
{
	تعريفنوع فراغ __noreturn (*image_مدخل_noargs_t)(فراغ);

	image_مدخل_noargs_t image_مدخل =
		(image_مدخل_noargs_t)spl_image->مدخل_نقطه;

	debug("image entry point: 0x%lx\n", spl_image->مدخل_نقطه);
	image_مدخل();
}

#إذا CONFIG_IS_ENABLED(HANDOFF)
/**
 * Set up the SPL hand-off information
 *
 * This is initially empty (zero) but can be written by
 */
ساكن صحيح راهاندازی_spl_handoff(فراغ)
{
	بنية spl_handoff *ho;

	ho = bloblist_ensure(BLOBLISTT_U_BOOT_SPL_HANDOFF, حجم(بنية spl_handoff));
	إذا (!ho)
		أرجع -ENOENT;

	أرجع 0;
}

__weak صحيح handoff_arch_save(بنية spl_handoff *ho)
{
	أرجع 0;
}

ساكن صحيح write_spl_handoff(فراغ)
{
	بنية spl_handoff *ho;
	صحيح ret;

	ho = bloblist_پیدا_کن(BLOBLISTT_U_BOOT_SPL_HANDOFF, حجم(بنية spl_handoff));
	إذا (!ho)
		أرجع -ENOENT;
	handoff_save_dram(ho);
	ret = handoff_arch_save(ho);
	إذا (ret)
		أرجع ret;
	debug(PHASE_PROMPT "Wrote SPL handoff\n");

	أرجع 0;
}
#وإلا
ساكن مضمن صحيح راهاندازی_spl_handoff(فراغ) { أرجع 0; }
ساكن مضمن صحيح write_spl_handoff(فراغ) { أرجع 0; }

#پایان_اگر /* HANDOFF */

/**
 * get_bootstage_id() - Get the bootstage ID to emit
 *
 * @start: true if this is for starting SPL, false for ending it
 * Return: bootstage ID to use
 */
ساكن تعداد bootstage_id get_bootstage_id(bool شروع)
{
	تعداد xpl_phase_t phase = xpl_phase();

	إذا (IS_ENABLED(CONFIG_TPL_BUILD) && phase == PHASE_TPL)
		أرجع شروع ? BOOTSTAGE_ID_شروع_TPL : BOOTSTAGE_ID_انتها_TPL;
	وإلا إذا (IS_ENABLED(CONFIG_VPL_BUILD) && phase == PHASE_VPL)
		أرجع شروع ? BOOTSTAGE_ID_شروع_VPL : BOOTSTAGE_ID_انتها_VPL;
	وإلا
		أرجع شروع ? BOOTSTAGE_ID_شروع_SPL : BOOTSTAGE_ID_انتها_SPL;
}

ساكن صحيح spl_common_init(bool راهاندازی_malloc)
{
	صحيح ret;

#إذا CONFIG_IS_ENABLED(SYS_MALLOC_F)
	إذا (راهاندازی_malloc) {
#اگر_تعریف_شده CFG_MALLOC_F_ADDR
		gd->malloc_پایه = CFG_MALLOC_F_ADDR;
#پایان_اگر
		gd->malloc_حد = CONFIG_VAL(SYS_MALLOC_F_LEN);
		gd->malloc_ptr = 0;
	}
#پایان_اگر
	ret = bootstage_init(xpl_is_اولی_phase());
	إذا (ret) {
		debug("%s: Failed to set up bootstage: ret=%d\n", __func__,
		      ret);
		أرجع ret;
	}
	إذا (!xpl_is_اولی_phase()) {
		ret = bootstage_unstash_default();
		إذا (ret)
			log_debug("Failed to unstash bootstage: ret=%d\n", ret);
	}
	bootstage_mark_name(get_bootstage_id(true), xpl_name(xpl_phase()));
#إذا CONFIG_IS_ENABLED(LOG)
	ret = log_init();
	إذا (ret) {
		debug("%s: Failed to set up logging\n", __func__);
		أرجع ret;
	}
#پایان_اگر
	إذا (CONFIG_IS_ENABLED(از_REAL)) {
		ret = fdtdec_راهاندازی();
		إذا (ret) {
			debug("fdtdec_setup() returned error %d\n", ret);
			أرجع ret;
		}
	}
	إذا (CONFIG_IS_ENABLED(از_زنده)) {
		bootstage_شروع(BOOTSTAGE_ID_ACCUM_از_زنده, "of_live");
		ret = از_زنده_build(gd->fdt_blob,
				    (بنية device_گره **)gd_از_ریشه_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_از_زنده);
		إذا (ret)
			أرجع ret;
	}
	إذا (CONFIG_IS_ENABLED(DM)) {
		bootstage_شروع(BOOTSTAGE_ID_ACCUM_DM_SPL,
				xpl_phase() == PHASE_TPL ? "dm tpl" : "dm_spl");
		/* With CONFIG_SPL_OF_PLATDATA, bring in all devices */
		ret = dm_init_and_پیمایش(!CONFIG_IS_ENABLED(از_PLATDATA));
		bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_SPL);
		إذا (ret) {
			debug("dm_init_and_scan() returned error %d\n", ret);
			أرجع ret;
		}

		ret = dm_autoprobe();
		إذا (ret)
			أرجع ret;
	}

	أرجع 0;
}

فراغ spl_مجموعه_bd(فراغ)
{
	/*
	 * NOTE: On some platforms (e.g. x86) bdata may be in flash and not
	 * writeable.
	 */
	إذا (!gd->bd)
		gd->bd = &bdata;
}

صحيح spl_early_init(فراغ)
{
	صحيح ret;

	debug("%s\n", __func__);

	ret = spl_common_init(true);
	إذا (ret)
		أرجع ret;
	gd->flags |= GD_FLG_SPL_EARLY_INIT;

	أرجع 0;
}

صحيح spl_init(فراغ)
{
	صحيح ret;
	bool راهاندازی_malloc = !(IS_ENABLED(CONFIG_SPL_پشته_R) &&
			IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIMPLE));

	debug("%s\n", __func__);

	إذا (!(gd->flags & GD_FLG_SPL_EARLY_INIT)) {
		ret = spl_common_init(راهاندازی_malloc);
		إذا (ret)
			أرجع ret;
	}
	gd->flags |= GD_FLG_SPL_INIT;

	أرجع 0;
}

#اگر_تعریف_نشده BOOT_DEVICE_NONE
#تعریف_کن BOOT_DEVICE_NONE 0xdeadbeef
#پایان_اگر

__weak فراغ board_boot_ترتیب(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();
}

__weak صحيح spl_check_board_image(بنية spl_image_info *spl_image,
				 ثابت بنية spl_boot_device *bootdev)
{
	أرجع 0;
}

ساكن صحيح spl_بارگیری_image(بنية spl_image_info *spl_image,
			  بنية spl_image_loader *loader)
{
	صحيح ret;
	بنية spl_boot_device bootdev;

	bootdev.boot_device = loader->boot_device;
	bootdev.boot_device_name = عدم;

	ret = loader->بارگیری_image(spl_image, &bootdev);
#اگر_تعریف_شده CONFIG_SPL_LEGACY_IMAGE_بررسی_چرخهای_CHECK
	إذا (!ret && spl_image->dcrc_درازی) {
		/* check data crc */
		ulong dcrc = بررسی_چرخهای_32_wd(0, (بیعلامت حرف *)spl_image->dcrc_داده,
				      spl_image->dcrc_درازی, CHUNKSZ_بررسی_چرخهای_32);
		إذا (dcrc != spl_image->dcrc) {
			puts("SPL: Image data CRC check failed!\n");
			ret = -EINVAL;
		}
	}
#پایان_اگر
	إذا (!ret)
		ret = spl_check_board_image(spl_image, &bootdev);

	أرجع ret;
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
ساكن صحيح boot_from_devices(بنية spl_image_info *spl_image,
			     u32 spl_boot_list[], صحيح شمار)
{
	بنية spl_image_loader *drv =
		ll_مدخل_شروع(بنية spl_image_loader, spl_image_loader);
	ثابت صحيح n_ents =
		ll_مدخل_شمار(بنية spl_image_loader, spl_image_loader);
	صحيح ret = -ENODEV;
	صحيح i;

	لكل (i = 0; i < شمار && spl_boot_list[i] != BOOT_DEVICE_NONE; i++) {
		بنية spl_image_loader *loader;
		صحيح bootdev = spl_boot_list[i];

		إذا (CONFIG_IS_ENABLED(SHOW_ERRORS))
			ret = -ENXIO;
		لكل (loader = drv; loader != drv + n_ents; loader++) {
			إذا (loader && bootdev != loader->boot_device)
				استمر;
			إذا (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
				چاپ_کن("Trying to boot from %s\n",
				       spl_loader_name(loader));
			}

			ret = spl_بارگیری_image(spl_image, loader);
			إذا (!ret) {
				spl_image->boot_device = bootdev;
				أرجع 0;
			}
			چاپ_کن("Error: %d\n", ret);
		}

		إذا (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
			إذا (CONFIG_IS_ENABLED(SHOW_ERRORS))
				چاپ_کن(PHASE_PROMPT
				       "Unsupported Boot Device %d\n",
				       bootdev);
			وإلا
				چاپ_کن(PHASE_PROMPT
				       "Unsupported Boot Device!\n");
		}
	}

	أرجع ret;
}

فراغ board_init_r(gd_t *dummy1, ulong dummy2)
{
	u32 spl_boot_list[] = {
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
	};
	spl_jump_مقصد_image_t jumper = &jump_مقصد_image;
	بنية spl_image_info spl_image;
	صحيح ret, os;
	فراغ *fdt;

	debug(">>" PHASE_PROMPT "board_init_r()\n");

	spl_مجموعه_bd();

	إذا (IS_ENABLED(CONFIG_SPL_SYS_MALLOC)) {
		mem_malloc_init(SPL_SYS_MALLOC_شروع, SPL_SYS_MALLOC_اندازه);
		gd->flags |= GD_FLG_پر_MALLOC_INIT;
	}
	إذا (!(gd->flags & GD_FLG_SPL_INIT)) {
		إذا (spl_init())
			hang();
	}
	timer_init();
	إذا (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_init();
		إذا (ret) {
			debug("%s: Failed to set up bloblist: ret=%d\n",
			      __func__, ret);
			puts(PHASE_PROMPT "Cannot set up bloblist\n");
			hang();
		}
	}
	إذا (CONFIG_IS_ENABLED(HANDOFF)) {
		صحيح ret;

		ret = راهاندازی_spl_handoff();
		إذا (ret) {
			puts(PHASE_PROMPT "Cannot set up SPL handoff\n");
			hang();
		}
	}

	إذا (CONFIG_IS_ENABLED(SOC_INIT))
		spl_soc_init();

	إذا (IS_ENABLED(CONFIG_SPL_WATCHDOG) && CONFIG_IS_ENABLED(WDT))
		initr_watchdog();

	إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) ||
	    IS_ENABLED(CONFIG_SPL_ATF) || IS_ENABLED(CONFIG_SPL_NET))
		dram_init_banksize();

	إذا (IS_ENABLED(CONFIG_SPL_LMB))
		lmb_init();

	إذا (CONFIG_IS_ENABLED(PCI) && !(gd->flags & GD_FLG_DM_DEAD)) {
		ret = pci_init();
		إذا (ret)
			puts(PHASE_PROMPT "Cannot initialize PCI\n");
		/* Don't fail. We still can try other boot methods. */
	}

	إذا (CONFIG_IS_ENABLED(BOARD_INIT))
		spl_board_init();

	bootcount_inc();

	/* Dump driver model states to aid analysis */
	إذا (CONFIG_IS_ENABLED(DM_STATS)) {
		بنية dm_stats mem;

		dm_get_mem(&mem);
		dm_dump_mem(&mem);
	}

	memset(&spl_image, '\0', حجم(spl_image));
	إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image.arg = (فراغ *)SPL_PAYLOAD_آرگومانها_ADDR;
	spl_image.boot_device = BOOT_DEVICE_NONE;
	board_boot_ترتیب(spl_boot_list);

	ret = boot_from_devices(&spl_image, spl_boot_list,
				ARRAY_اندازه(spl_boot_list));
	إذا (ret) {
		إذا (CONFIG_IS_ENABLED(SHOW_ERRORS))
			چاپ_کن(PHASE_PROMPT "failed to boot from all boot devices (err=%d)\n",
			       ret);
		وإلا
			puts(PHASE_PROMPT "failed to boot from all boot devices\n");
		hang();
	}

	spl_perform_arch_fixups(&spl_image);
	spl_perform_board_fixups(&spl_image);

	os = spl_image.os;
	إذا (os == IH_OS_U_BOOT) {
		debug("Jumping to %s...\n", xpl_name(xpl_بعدی_phase()));
	} وإلا إذا (CONFIG_IS_ENABLED(ATF) && os == IH_OS_ARM_TRUSTED_FIRMWARE) {
		debug("Jumping to U-Boot via ARM Trusted Firmware\n");
		spl_fixup_fdt(spl_image_fdt_addr(&spl_image));
		jumper = &spl_invoke_atf;
	} وإلا إذا (CONFIG_IS_ENABLED(OPTEE_IMAGE) && os == IH_OS_TEE) {
		debug("Jumping to U-Boot via OP-TEE\n");
		spl_board_prepare_for_optee(spl_image_fdt_addr(&spl_image));
		jumper = &jump_مقصد_image_optee;
	} وإلا إذا (CONFIG_IS_ENABLED(OPENSBI) && os == IH_OS_OPENSBI) {
		debug("Jumping to U-Boot via RISC-V OpenSBI\n");
		jumper = &spl_invoke_opensbi;
	} وإلا إذا (CONFIG_IS_ENABLED(OS_BOOT) && os == IH_OS_LINUX) {
		debug("Jumping to Linux\n");
		إذا (CONFIG_IS_ENABLED(OS_BOOT_آرگومانها))
			fdt = (فراغ *)SPL_PAYLOAD_آرگومانها_ADDR;
		وإلا
			fdt = spl_image_fdt_addr(&spl_image);
		spl_fixup_fdt(fdt);
		spl_board_prepare_for_linux();
		spl_image.arg = fdt;
		jumper = &jump_مقصد_image_linux;
	} وإلا {
		debug("Unsupported OS image.. Jumping nevertheless..\n");
	}
	إذا (CONFIG_IS_ENABLED(SYS_MALLOC_F) &&
	    !IS_ENABLED(CONFIG_SPL_SYS_MALLOC_اندازه))
		debug("SPL malloc() used 0x%x bytes (%d KB)\n",
		      gd_malloc_ptr(), gd_malloc_ptr() / 1024);

	bootstage_mark_name(get_bootstage_id(false), "end phase");
	ret = bootstage_stash_default();
	إذا (ret)
		debug("Failed to stash bootstage: err=%d\n", ret);

	إذا (IS_ENABLED(CONFIG_SPL_VIDEO_REMOVE)) {
		بنية udevice *dev;
		صحيح rc;

		rc = uclass_پیدا_کن_device(UCLASS_VIDEO, 0, &dev);
		إذا (!rc && dev) {
			rc = device_remove(dev, DM_REMOVE_نرمال);
			إذا (rc)
				چاپ_کن("Cannot remove video device '%s' (err=%d)\n",
				       dev->name, rc);
		}
	}
	إذا (CONFIG_IS_ENABLED(HANDOFF)) {
		ret = write_spl_handoff();
		إذا (ret)
			چاپ_کن(PHASE_PROMPT
			       "SPL hand-off write failed (err=%d)\n", ret);
	}
	إذا (CONFIG_IS_ENABLED(UPL_بیرون) && (gd->flags & GD_FLG_UPL)) {
		ret = spl_write_upl_handoff(&spl_image);
		إذا (ret) {
			چاپ_کن(PHASE_PROMPT
			       "UPL hand-off write failed (err=%d)\n", ret);
			hang();
		}
	}
	إذا (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_پایان();
		إذا (ret)
			چاپ_کن("Warning: Failed to finish bloblist (ret=%d)\n",
			       ret);
	}

	spl_board_prepare_for_boot();

	إذا (CONFIG_IS_ENABLED(RELOC_LOADER)) {
		صحيح ret;

		ret = spl_reloc_jump(&spl_image, jumper);
		إذا (ret) {
			إذا (xpl_phase() == PHASE_VPL)
				چاپ_کن("jump failed %d\n", ret);
			hang();
		}
	}

	jumper(&spl_image);
}

/*
 * This requires UART clocks to be enabled.  In order for this to work the
 * caller must ensure that the gd pointer is valid.
 */
فراغ preloader_console_init(فراغ)
{
#إذا CONFIG_IS_ENABLED(SERIAL)
	gd->baudrate = CONFIG_BAUDRATE;

	serial_init();		/* serial communications setup */

	gd->flags |= GD_FLG_HAVE_CONSOLE;

#إذا CONFIG_IS_ENABLED(BANNER_PRINT)
	puts("\nU-Boot " PHASE_NAME " " PLAIN_نسخه " (" U_BOOT_DATE " - "
	     U_BOOT_زمان " " U_BOOT_TZ ")\n");
#پایان_اگر
#اگر_تعریف_شده CONFIG_SPL_DISPLAY_PRINT
	spl_display_print();
#پایان_اگر
#پایان_اگر
}

/**
 * This function is called before the stack is changed from initial stack to
 * relocated stack. It tries to dump the stack size used
 */
__weak فراغ spl_relocate_پشته_check(فراغ)
{
#إذا CONFIG_IS_ENABLED(SYS_REPORT_پشته_F_USAGE)
	ulong init_sp = gd->شروع_addr_sp;
	ulong پشته_bottom = init_sp - CONFIG_VAL(اندازه_حد_PROVIDE_پشته);
	u8 *ptr = (u8 *)پشته_bottom;
	ulong i;

	لكل (i = 0; i < CONFIG_VAL(اندازه_حد_PROVIDE_پشته); i++) {
		إذا (*ptr != CONFIG_VAL(SYS_پشته_F_CHECK_BYTE))
			اكسر;
		ptr++;
	}
	چاپ_کن("SPL initial stack usage: %lu bytes\n",
	       CONFIG_VAL(اندازه_حد_PROVIDE_پشته) - i);
#پایان_اگر
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
ulong spl_relocate_پشته_gd(فراغ)
{
#إذا CONFIG_IS_ENABLED(پشته_R)
	gd_t *new_gd;
	ulong ptr = CONFIG_SPL_پشته_R_ADDR;

	إذا (CONFIG_IS_ENABLED(SYS_REPORT_پشته_F_USAGE))
		spl_relocate_پشته_check();

#إذا defined(CONFIG_SPL_SYS_MALLOC_SIMPLE) && CONFIG_IS_ENABLED(SYS_MALLOC_F)
	إذا (CONFIG_SPL_پشته_R_MALLOC_SIMPLE_LEN) {
		debug("SPL malloc() before relocation used 0x%x bytes (%d KB)\n",
		      gd->malloc_ptr, gd->malloc_ptr / 1024);
		ptr -= CONFIG_SPL_پشته_R_MALLOC_SIMPLE_LEN;
		gd->malloc_پایه = ptr;
		gd->malloc_حد = CONFIG_SPL_پشته_R_MALLOC_SIMPLE_LEN;
		gd->malloc_ptr = 0;
	}
#پایان_اگر
	/* Get stack position: use 8-byte alignment for ABI compliance */
	ptr -= roundup(حجم(gd_t), 16);
	gd->شروع_addr_sp = ptr;
	new_gd = (gd_t *)ptr;
	کپی_حافظه(new_gd, (فراغ *)gd, حجم(gd_t));
#إذا CONFIG_IS_ENABLED(DM)
	dm_fixup_for_gd_move(new_gd);
#پایان_اگر
#إذا CONFIG_IS_ENABLED(LOG)
	log_fixup_for_gd_move(new_gd);
#پایان_اگر
#إذا !defined(CONFIG_ARM) && !defined(CONFIG_RISCV)
	gd = new_gd;
#پایان_اگر
	أرجع ptr;
#وإلا
	أرجع 0;
#پایان_اگر
}

#إذا defined(CONFIG_BOOTCOUNT_حد) && \
	((!defined(CONFIG_TPL_BUILD) && !defined(CONFIG_SPL_BOOTCOUNT_حد)) || \
	 (defined(CONFIG_TPL_BUILD) && !defined(CONFIG_TPL_BOOTCOUNT_حد)))
فراغ bootcount_store(ulong a)
{
}

ulong bootcount_بارگیری(فراغ)
{
	أرجع 0;
}
#پایان_اگر
