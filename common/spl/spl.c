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
#د_که_پای
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

DECLARE_GLOBAL_معلومات_PTR;
DECLARE_BINMAN_MAGIC_SYM;

u32 *boot_params_ptr = عدم;

#إذا CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS)
/* See spl.h for information about this */
#إذا defined(CONFIG_SPL_BUILD)
binman_sym_declare(ulong, u_boot_any, image_pos);
binman_sym_declare(ulong, u_boot_any, اندازه);
#د_که_پای

#که_تعریف_شوی CONFIG_TPL
binman_sym_declare(ulong, u_boot_spl_any, image_pos);
binman_sym_declare(ulong, u_boot_spl_any, اندازه);
#د_که_پای

#که_تعریف_شوی CONFIG_VPL
binman_sym_declare(ulong, u_boot_vpl_any, image_pos);
binman_sym_declare(ulong, u_boot_vpl_any, اندازه);
#د_که_پای

#د_که_پای /* BINMAN_UBOOT_SYMBOLS */

/* Define board data structure */
ساكن بنية bd_info bdata __attribute__ ((section(".data")));

#إذا CONFIG_IS_ENABLED(SHOW_BOOT_پرمختګ)
/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
__weak فراغ show_boot_پرمختګ(صحيح val) {}
#د_که_پای

#إذا defined(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) || \
	defined(CONFIG_SPL_ATF)
/* weak, default platform-specific function to initialize dram banks */
__weak صحيح dram_init_banksize(فراغ)
{
	أرجع 0;
}
#د_که_پای

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
__weak صحيح spl_پیل_uboot(فراغ)
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
صحيح __weak bootz_تیاری(ulong image, ulong *پیل, ulong *پای_ټکی_2)
{
	 أرجع 1;
}

صحيح __weak booti_تیاری(ulong image, ulong *relocated_addr, ulong *اندازه, bool force_reloc)
{
	 أرجع 1;
}
#د_که_پای

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
#إذا defined(CONFIG_SPL_د_LIBFDT)
	صحيح err;

	إذا (!fdt_blob)
		أرجع;

	err = fdt_check_سرلیک(fdt_blob);
	إذا (err < 0) {
		چاپ_کړه("fdt_root: %s\n", fdt_strerror(err));
		أرجع;
	}

	/* fixup the memory dt node */
	err = fdt_shrink_منزل_minimum(fdt_blob, 0);
	إذا (err == 0) {
		چاپ_کړه(PHASE_PROMPT "fdt_shrink_to_minimum err - %d\n", err);
		أرجع;
	}

	err = arch_fixup_fdt(fdt_blob);
	إذا (err) {
		چاپ_کړه(PHASE_PROMPT "arch_fixup_fdt err - %d\n", err);
		أرجع;
	}
#د_که_پای
}

صحيح spl_reserve_video_from_ram_سر(فراغ)
{
	إذا (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		صحيح ret;

		addr = gd->ram_سر;
		ret = video_reserve(&addr);
		إذا (ret)
			أرجع ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((بینښه اوږد)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	أرجع 0;
}

ulong spl_get_image_pos(فراغ)
{
	إذا (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		أرجع BINMAN_SYM_MISSING;

#که_تعریف_شوی CONFIG_VPL
	إذا (xpl_بل_phase() == PHASE_VPL)
		أرجع binman_sym(ulong, u_boot_vpl_any, image_pos);
#د_که_پای
#إذا defined(CONFIG_TPL) && !defined(CONFIG_VPL)
	إذا (xpl_بل_phase() == PHASE_SPL)
		أرجع binman_sym(ulong, u_boot_spl_any, image_pos);
#د_که_پای
#إذا defined(CONFIG_SPL_BUILD)
	أرجع binman_sym(ulong, u_boot_any, image_pos);
#د_که_پای

	أرجع BINMAN_SYM_MISSING;
}

ulong spl_get_image_اندازه(فراغ)
{
	إذا (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		أرجع BINMAN_SYM_MISSING;

#که_تعریف_شوی CONFIG_VPL
	إذا (xpl_بل_phase() == PHASE_VPL)
		أرجع binman_sym(ulong, u_boot_vpl_any, اندازه);
#د_که_پای
	أرجع xpl_بل_phase() == PHASE_SPL ?
		binman_sym(ulong, u_boot_spl_any, اندازه) :
		binman_sym(ulong, u_boot_any, اندازه);
}

ulong spl_get_image_متن_بنسټ(فراغ)
{
#که_تعریف_شوی CONFIG_VPL
	إذا (xpl_بل_phase() == PHASE_VPL)
		أرجع CONFIG_VPL_متن_بنسټ;
#د_که_پای
	أرجع xpl_بل_phase() == PHASE_SPL ? CONFIG_SPL_متن_بنسټ :
		CONFIG_متن_بنسټ;
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
__weak فراغ __noreturn jump_منزل_image_optee(بنية spl_image_info *spl_image)
{
	spl_optee_ثبت(عدم, عدم, spl_image->fdt_addr,
			(فراغ *)spl_image->ثبت_ټکی);
}
#د_که_پای

__weak فراغ spl_board_prepare_for_boot(فراغ)
{
	/* Nothing to do! */
}

__weak بنية legacy_img_hdr *spl_get_بارول_بفر(ssize_t دننه_واټن, size_t اندازه)
{
	أرجع map_sysmem(CONFIG_متن_بنسټ + دننه_واټن, 0);
}

فراغ spl_سېټ_سرلیک_raw_uboot(بنية spl_image_info *spl_image)
{
	ulong u_boot_pos = spl_get_image_pos();

#إذا CONFIG_SYS_MONITOR_LEN != 0
	spl_image->اندازه = CONFIG_SYS_MONITOR_LEN;
#وإلا
	/* Unknown U-Boot size, let's assume it will not be more than 200 KB */
	spl_image->اندازه = 200 * 1024;
#د_که_پای

	/*
	 * Binman error cases: address of the end of the previous region or the
	 * start of the image's entry area (usually 0) if there is no previous
	 * region.
	 */
	إذا (u_boot_pos && u_boot_pos != BINMAN_SYM_MISSING) {
		/* Binman does not support separated entry addresses */
		spl_image->ثبت_ټکی = spl_get_image_متن_بنسټ();
		spl_image->بارول_addr = spl_get_image_متن_بنسټ();
		spl_image->اندازه = spl_get_image_اندازه();
		log_debug("Next load addr %lx\n", spl_image->بارول_addr);
	} وإلا {
		spl_image->ثبت_ټکی = CONFIG_SYS_UBOOT_پیل;
		spl_image->بارول_addr = CONFIG_متن_بنسټ;
		log_debug("Default load addr %lx (u_boot_pos=%lx)\n",
			  spl_image->بارول_addr, u_boot_pos);
	}
	spl_image->os = IH_OS_U_BOOT;
	spl_image->name = xpl_name(xpl_بل_phase());
	log_debug("Next phase: %s at %lx size %lx\n", spl_image->name,
		  spl_image->بارول_addr, (ulong)spl_image->اندازه);
}

__weak صحيح spl_parse_board_سرلیک(بنية spl_image_info *spl_image,
				  ثابت بنية spl_boot_device *bootdev,
				  ثابت فراغ *image_سرلیک, size_t اندازه)
{
	أرجع -EINVAL;
}

__weak صحيح spl_parse_legacy_سرلیک(بنية spl_image_info *spl_image,
				   ثابت بنية legacy_img_hdr *سرلیک)
{
	/* LEGACY image not supported */
	debug("Legacy boot image support not enabled, proceeding to other boot methods\n");
	أرجع -EINVAL;
}

صحيح spl_parse_image_سرلیک(بنية spl_image_info *spl_image,
			   ثابت بنية spl_boot_device *bootdev,
			   ثابت بنية legacy_img_hdr *سرلیک)
{
	صحيح ret;

	إذا (CONFIG_IS_ENABLED(بارول_FIT_ډک)) {
		ret = spl_بارول_fit_image(spl_image, سرلیک);

		إذا (!ret)
			أرجع ret;
	}
	إذا (image_get_magic(سرلیک) == IH_MAGIC) {
		صحيح ret;

		ret = spl_parse_legacy_سرلیک(spl_image, سرلیک);
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
		ulong پیل, اندازه;

		إذا (!booti_تیاری((ulong)سرلیک, &پیل, &اندازه, 0)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->بارول_addr = پیل;
			spl_image->ثبت_ټکی = پیل;
			spl_image->اندازه = اندازه;
			debug(PHASE_PROMPT
			      "payload Image, load addr: 0x%lx size: %d\n",
			      spl_image->بارول_addr, spl_image->اندازه);
			أرجع 0;
		}
	} وإلا إذا (IS_ENABLED(CONFIG_SPL_OS_BOOT) &&
		   IS_ENABLED(CONFIG_SPL_BOOTZ)) {
		ulong پیل, پای_ټکی_2;

		إذا (!bootz_تیاری((ulong)سرلیک, &پیل, &پای_ټکی_2)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->بارول_addr = CONFIG_SYS_بارول_ADDR;
			spl_image->ثبت_ټکی = CONFIG_SYS_بارول_ADDR;
			spl_image->اندازه = پای_ټکی_2 - پیل;
			debug(PHASE_PROMPT
			      "payload zImage, load addr: 0x%lx size: %d\n",
			      spl_image->بارول_addr, spl_image->اندازه);
			أرجع 0;
		}
	}

	إذا (!spl_parse_board_سرلیک(spl_image, bootdev, (ثابت فراغ *)سرلیک,
				    حجم(*سرلیک)))
		أرجع 0;

	إذا (IS_ENABLED(CONFIG_SPL_RAW_IMAGE_SUPPORT)) {
		/* Signature not found - assume u-boot.bin */
		debug("mkimage signature not found - ih_magic = %x\n",
		      سرلیک->ih_magic);
		spl_سېټ_سرلیک_raw_uboot(spl_image);
	} وإلا {
		/* RAW image not supported, proceed to other boot methods. */
		debug("Raw boot image support not enabled, proceeding to other boot methods\n");
		أرجع -EINVAL;
	}

	أرجع 0;
}

#إذا SPL_بارول_USERS > 1
صحيح spl_بارول(بنية spl_image_info *spl_image,
	     ثابت بنية spl_boot_device *bootdev, بنية spl_بارول_info *info,
	     size_t اندازه, size_t دننه_واټن)
{
	أرجع spl_بارول_2(spl_image, bootdev, info, اندازه, دننه_واټن);
}
#د_که_پای

__weak فراغ __noreturn jump_منزل_image(بنية spl_image_info *spl_image)
{
	تعريفنوع فراغ __noreturn (*image_ثبت_noargs_t)(فراغ);

	image_ثبت_noargs_t image_ثبت =
		(image_ثبت_noargs_t)spl_image->ثبت_ټکی;

	debug("image entry point: 0x%lx\n", spl_image->ثبت_ټکی);
	image_ثبت();
}

#إذا CONFIG_IS_ENABLED(HANDOFF)
/**
 * Set up the SPL hand-off information
 *
 * This is initially empty (zero) but can be written by
 */
ساكن صحيح تیاری_spl_handoff(فراغ)
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

	ho = bloblist_پیدا_کړه(BLOBLISTT_U_BOOT_SPL_HANDOFF, حجم(بنية spl_handoff));
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
ساكن مضمن صحيح تیاری_spl_handoff(فراغ) { أرجع 0; }
ساكن مضمن صحيح write_spl_handoff(فراغ) { أرجع 0; }

#د_که_پای /* HANDOFF */

/**
 * get_bootstage_id() - Get the bootstage ID to emit
 *
 * @start: true if this is for starting SPL, false for ending it
 * Return: bootstage ID to use
 */
ساكن تعداد bootstage_id get_bootstage_id(bool پیل)
{
	تعداد xpl_phase_t phase = xpl_phase();

	إذا (IS_ENABLED(CONFIG_TPL_BUILD) && phase == PHASE_TPL)
		أرجع پیل ? BOOTSTAGE_ID_پیل_TPL : BOOTSTAGE_ID_پای_ټکی_TPL;
	وإلا إذا (IS_ENABLED(CONFIG_VPL_BUILD) && phase == PHASE_VPL)
		أرجع پیل ? BOOTSTAGE_ID_پیل_VPL : BOOTSTAGE_ID_پای_ټکی_VPL;
	وإلا
		أرجع پیل ? BOOTSTAGE_ID_پیل_SPL : BOOTSTAGE_ID_پای_ټکی_SPL;
}

ساكن صحيح spl_common_init(bool تیاری_malloc)
{
	صحيح ret;

#إذا CONFIG_IS_ENABLED(SYS_MALLOC_F)
	إذا (تیاری_malloc) {
#که_تعریف_شوی CFG_MALLOC_F_ADDR
		gd->malloc_بنسټ = CFG_MALLOC_F_ADDR;
#د_که_پای
		gd->malloc_حد = CONFIG_VAL(SYS_MALLOC_F_LEN);
		gd->malloc_ptr = 0;
	}
#د_که_پای
	ret = bootstage_init(xpl_is_لومړی_phase());
	إذا (ret) {
		debug("%s: Failed to set up bootstage: ret=%d\n", __func__,
		      ret);
		أرجع ret;
	}
	إذا (!xpl_is_لومړی_phase()) {
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
#د_که_پای
	إذا (CONFIG_IS_ENABLED(د_REAL)) {
		ret = fdtdec_تیاری();
		إذا (ret) {
			debug("fdtdec_setup() returned error %d\n", ret);
			أرجع ret;
		}
	}
	إذا (CONFIG_IS_ENABLED(د_ژوندی)) {
		bootstage_پیل(BOOTSTAGE_ID_ACCUM_د_ژوندی, "of_live");
		ret = د_ژوندی_build(gd->fdt_blob,
				    (بنية device_غوټه **)gd_د_ریښه_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_د_ژوندی);
		إذا (ret)
			أرجع ret;
	}
	إذا (CONFIG_IS_ENABLED(DM)) {
		bootstage_پیل(BOOTSTAGE_ID_ACCUM_DM_SPL,
				xpl_phase() == PHASE_TPL ? "dm tpl" : "dm_spl");
		/* With CONFIG_SPL_OF_PLATDATA, bring in all devices */
		ret = dm_init_and_لټون(!CONFIG_IS_ENABLED(د_PLATDATA));
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

فراغ spl_سېټ_bd(فراغ)
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
	bool تیاری_malloc = !(IS_ENABLED(CONFIG_SPL_ډېری_R) &&
			IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIMPLE));

	debug("%s\n", __func__);

	إذا (!(gd->flags & GD_FLG_SPL_EARLY_INIT)) {
		ret = spl_common_init(تیاری_malloc);
		إذا (ret)
			أرجع ret;
	}
	gd->flags |= GD_FLG_SPL_INIT;

	أرجع 0;
}

#که_نه_تعریف_شوی BOOT_DEVICE_NONE
#تعریف_کړه BOOT_DEVICE_NONE 0xdeadbeef
#د_که_پای

__weak فراغ board_boot_ترتیب(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();
}

__weak صحيح spl_check_board_image(بنية spl_image_info *spl_image,
				 ثابت بنية spl_boot_device *bootdev)
{
	أرجع 0;
}

ساكن صحيح spl_بارول_image(بنية spl_image_info *spl_image,
			  بنية spl_image_loader *loader)
{
	صحيح ret;
	بنية spl_boot_device bootdev;

	bootdev.boot_device = loader->boot_device;
	bootdev.boot_device_name = عدم;

	ret = loader->بارول_image(spl_image, &bootdev);
#که_تعریف_شوی CONFIG_SPL_LEGACY_IMAGE_دوراني_ازموینه_CHECK
	إذا (!ret && spl_image->dcrc_اوږدتیا) {
		/* check data crc */
		ulong dcrc = دوراني_ازموینه_32_wd(0, (بینښه حرف *)spl_image->dcrc_معلومات,
				      spl_image->dcrc_اوږدتیا, CHUNKSZ_دوراني_ازموینه_32);
		إذا (dcrc != spl_image->dcrc) {
			puts("SPL: Image data CRC check failed!\n");
			ret = -EINVAL;
		}
	}
#د_که_پای
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
			     u32 spl_boot_list[], صحيح شمېر)
{
	بنية spl_image_loader *drv =
		ll_ثبت_پیل(بنية spl_image_loader, spl_image_loader);
	ثابت صحيح n_ents =
		ll_ثبت_شمېر(بنية spl_image_loader, spl_image_loader);
	صحيح ret = -ENODEV;
	صحيح i;

	لكل (i = 0; i < شمېر && spl_boot_list[i] != BOOT_DEVICE_NONE; i++) {
		بنية spl_image_loader *loader;
		صحيح bootdev = spl_boot_list[i];

		إذا (CONFIG_IS_ENABLED(SHOW_ERRORS))
			ret = -ENXIO;
		لكل (loader = drv; loader != drv + n_ents; loader++) {
			إذا (loader && bootdev != loader->boot_device)
				استمر;
			إذا (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
				چاپ_کړه("Trying to boot from %s\n",
				       spl_loader_name(loader));
			}

			ret = spl_بارول_image(spl_image, loader);
			إذا (!ret) {
				spl_image->boot_device = bootdev;
				أرجع 0;
			}
			چاپ_کړه("Error: %d\n", ret);
		}

		إذا (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
			إذا (CONFIG_IS_ENABLED(SHOW_ERRORS))
				چاپ_کړه(PHASE_PROMPT
				       "Unsupported Boot Device %d\n",
				       bootdev);
			وإلا
				چاپ_کړه(PHASE_PROMPT
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
	spl_jump_منزل_image_t jumper = &jump_منزل_image;
	بنية spl_image_info spl_image;
	صحيح ret, os;
	فراغ *fdt;

	debug(">>" PHASE_PROMPT "board_init_r()\n");

	spl_سېټ_bd();

	إذا (IS_ENABLED(CONFIG_SPL_SYS_MALLOC)) {
		mem_malloc_init(SPL_SYS_MALLOC_پیل, SPL_SYS_MALLOC_اندازه);
		gd->flags |= GD_FLG_ډک_MALLOC_INIT;
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

		ret = تیاری_spl_handoff();
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
		spl_image.arg = (فراغ *)SPL_PAYLOAD_دلیلونه_ADDR;
	spl_image.boot_device = BOOT_DEVICE_NONE;
	board_boot_ترتیب(spl_boot_list);

	ret = boot_from_devices(&spl_image, spl_boot_list,
				ARRAY_اندازه(spl_boot_list));
	إذا (ret) {
		إذا (CONFIG_IS_ENABLED(SHOW_ERRORS))
			چاپ_کړه(PHASE_PROMPT "failed to boot from all boot devices (err=%d)\n",
			       ret);
		وإلا
			puts(PHASE_PROMPT "failed to boot from all boot devices\n");
		hang();
	}

	spl_perform_arch_fixups(&spl_image);
	spl_perform_board_fixups(&spl_image);

	os = spl_image.os;
	إذا (os == IH_OS_U_BOOT) {
		debug("Jumping to %s...\n", xpl_name(xpl_بل_phase()));
	} وإلا إذا (CONFIG_IS_ENABLED(ATF) && os == IH_OS_ARM_TRUSTED_FIRMWARE) {
		debug("Jumping to U-Boot via ARM Trusted Firmware\n");
		spl_fixup_fdt(spl_image_fdt_addr(&spl_image));
		jumper = &spl_invoke_atf;
	} وإلا إذا (CONFIG_IS_ENABLED(OPTEE_IMAGE) && os == IH_OS_TEE) {
		debug("Jumping to U-Boot via OP-TEE\n");
		spl_board_prepare_for_optee(spl_image_fdt_addr(&spl_image));
		jumper = &jump_منزل_image_optee;
	} وإلا إذا (CONFIG_IS_ENABLED(OPENSBI) && os == IH_OS_OPENSBI) {
		debug("Jumping to U-Boot via RISC-V OpenSBI\n");
		jumper = &spl_invoke_opensbi;
	} وإلا إذا (CONFIG_IS_ENABLED(OS_BOOT) && os == IH_OS_LINUX) {
		debug("Jumping to Linux\n");
		إذا (CONFIG_IS_ENABLED(OS_BOOT_دلیلونه))
			fdt = (فراغ *)SPL_PAYLOAD_دلیلونه_ADDR;
		وإلا
			fdt = spl_image_fdt_addr(&spl_image);
		spl_fixup_fdt(fdt);
		spl_board_prepare_for_linux();
		spl_image.arg = fdt;
		jumper = &jump_منزل_image_linux;
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

		rc = uclass_پیدا_کړه_device(UCLASS_VIDEO, 0, &dev);
		إذا (!rc && dev) {
			rc = device_remove(dev, DM_REMOVE_نورمال);
			إذا (rc)
				چاپ_کړه("Cannot remove video device '%s' (err=%d)\n",
				       dev->name, rc);
		}
	}
	إذا (CONFIG_IS_ENABLED(HANDOFF)) {
		ret = write_spl_handoff();
		إذا (ret)
			چاپ_کړه(PHASE_PROMPT
			       "SPL hand-off write failed (err=%d)\n", ret);
	}
	إذا (CONFIG_IS_ENABLED(UPL_بهر) && (gd->flags & GD_FLG_UPL)) {
		ret = spl_write_upl_handoff(&spl_image);
		إذا (ret) {
			چاپ_کړه(PHASE_PROMPT
			       "UPL hand-off write failed (err=%d)\n", ret);
			hang();
		}
	}
	إذا (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_پای();
		إذا (ret)
			چاپ_کړه("Warning: Failed to finish bloblist (ret=%d)\n",
			       ret);
	}

	spl_board_prepare_for_boot();

	إذا (CONFIG_IS_ENABLED(RELOC_LOADER)) {
		صحيح ret;

		ret = spl_reloc_jump(&spl_image, jumper);
		إذا (ret) {
			إذا (xpl_phase() == PHASE_VPL)
				چاپ_کړه("jump failed %d\n", ret);
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
	     U_BOOT_وخت " " U_BOOT_TZ ")\n");
#د_که_پای
#که_تعریف_شوی CONFIG_SPL_DISPLAY_PRINT
	spl_display_print();
#د_که_پای
#د_که_پای
}

/**
 * This function is called before the stack is changed from initial stack to
 * relocated stack. It tries to dump the stack size used
 */
__weak فراغ spl_relocate_ډېری_check(فراغ)
{
#إذا CONFIG_IS_ENABLED(SYS_REPORT_ډېری_F_USAGE)
	ulong init_sp = gd->پیل_addr_sp;
	ulong ډېری_bottom = init_sp - CONFIG_VAL(اندازه_حد_PROVIDE_ډېری);
	u8 *ptr = (u8 *)ډېری_bottom;
	ulong i;

	لكل (i = 0; i < CONFIG_VAL(اندازه_حد_PROVIDE_ډېری); i++) {
		إذا (*ptr != CONFIG_VAL(SYS_ډېری_F_CHECK_BYTE))
			اكسر;
		ptr++;
	}
	چاپ_کړه("SPL initial stack usage: %lu bytes\n",
	       CONFIG_VAL(اندازه_حد_PROVIDE_ډېری) - i);
#د_که_پای
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
ulong spl_relocate_ډېری_gd(فراغ)
{
#إذا CONFIG_IS_ENABLED(ډېری_R)
	gd_t *new_gd;
	ulong ptr = CONFIG_SPL_ډېری_R_ADDR;

	إذا (CONFIG_IS_ENABLED(SYS_REPORT_ډېری_F_USAGE))
		spl_relocate_ډېری_check();

#إذا defined(CONFIG_SPL_SYS_MALLOC_SIMPLE) && CONFIG_IS_ENABLED(SYS_MALLOC_F)
	إذا (CONFIG_SPL_ډېری_R_MALLOC_SIMPLE_LEN) {
		debug("SPL malloc() before relocation used 0x%x bytes (%d KB)\n",
		      gd->malloc_ptr, gd->malloc_ptr / 1024);
		ptr -= CONFIG_SPL_ډېری_R_MALLOC_SIMPLE_LEN;
		gd->malloc_بنسټ = ptr;
		gd->malloc_حد = CONFIG_SPL_ډېری_R_MALLOC_SIMPLE_LEN;
		gd->malloc_ptr = 0;
	}
#د_که_پای
	/* Get stack position: use 8-byte alignment for ABI compliance */
	ptr -= roundup(حجم(gd_t), 16);
	gd->پیل_addr_sp = ptr;
	new_gd = (gd_t *)ptr;
	حافظه_نقل(new_gd, (فراغ *)gd, حجم(gd_t));
#إذا CONFIG_IS_ENABLED(DM)
	dm_fixup_for_gd_move(new_gd);
#د_که_پای
#إذا CONFIG_IS_ENABLED(LOG)
	log_fixup_for_gd_move(new_gd);
#د_که_پای
#إذا !defined(CONFIG_ARM) && !defined(CONFIG_RISCV)
	gd = new_gd;
#د_که_پای
	أرجع ptr;
#وإلا
	أرجع 0;
#د_که_پای
}

#إذا defined(CONFIG_BOOTCOUNT_حد) && \
	((!defined(CONFIG_TPL_BUILD) && !defined(CONFIG_SPL_BOOTCOUNT_حد)) || \
	 (defined(CONFIG_TPL_BUILD) && !defined(CONFIG_TPL_BOOTCOUNT_حد)))
فراغ bootcount_store(ulong a)
{
}

ulong bootcount_بارول(فراغ)
{
	أرجع 0;
}
#د_که_پای
