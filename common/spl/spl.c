// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */

#شامل <config.h>
#شامل <bloblist.h>
#شامل <binman_sym.h>
#شامل <bootstage.h>
#شامل <dm.h>
#شامل <handoff.h>
#شامل <hang.h>
#شامل <init.h>
#شامل <irq_func.h>
#شامل <log.h>
#شامل <mapmem.h>
#شامل <of_live.h>
#شامل <serial.h>
#شامل <spl.h>
#شامل <spl_load.h>
#شامل <system-constants.h>
#شامل <asm/global_data.h>
#شامل <asm-generic/gpio.h>
#شامل <nand.h>
#شامل <fat.h>
#شامل <u-boot/crc.h>
#اگر CONFIG_IS_ENABLED(BANNER_PRINT)
#شامل <timestamp.h>
#اگر_ختم
#شامل <version.h>
#شامل <image.h>
#شامل <malloc.h>
#شامل <mapmem.h>
#شامل <dm/root.h>
#شامل <dm/util.h>
#شامل <dm/device-internal.h>
#شامل <dm/uclass-internal.h>
#شامل <linux/compiler.h>
#شامل <fdt_support.h>
#شامل <bootcount.h>
#شامل <wdt.h>
#شامل <video.h>

DECLARE_GLOBAL_ڈیٹا_PTR;
DECLARE_BINMAN_MAGIC_SYM;

u32 *boot_params_ptr = تہی;

#اگر CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS)
/* See spl.h for information about this */
#اگر defined(CONFIG_SPL_BUILD)
binman_sym_declare(ulong, u_boot_any, image_pos);
binman_sym_declare(ulong, u_boot_any, سائز);
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_TPL
binman_sym_declare(ulong, u_boot_spl_any, image_pos);
binman_sym_declare(ulong, u_boot_spl_any, سائز);
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_VPL
binman_sym_declare(ulong, u_boot_vpl_any, image_pos);
binman_sym_declare(ulong, u_boot_vpl_any, سائز);
#اگر_ختم

#اگر_ختم /* BINMAN_UBOOT_SYMBOLS */

/* Define board data structure */
ساکن ساخت bd_info bdata __attribute__ ((section(".data")));

#اگر CONFIG_IS_ENABLED(SHOW_BOOT_پیش_رفت)
/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
__weak خالی show_boot_پیش_رفت(صحیح val) {}
#اگر_ختم

#اگر defined(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) || \
	defined(CONFIG_SPL_ATF)
/* weak, default platform-specific function to initialize dram banks */
__weak صحیح dram_init_banksize(خالی)
{
	واپس 0;
}
#اگر_ختم

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
#اگر CONFIG_IS_ENABLED(OS_BOOT)
__weak صحیح spl_آغاز_uboot(خالی)
{
	puts(PHASE_PROMPT
	     "Please implement spl_start_uboot() for your board\n");
	puts(PHASE_PROMPT "Direct Linux boot not active!\n");
	واپس 1;
}

/*
 * Weak default function for arch specific zImage check. Return zero
 * and fill start and end address if image is recognized.
 */
صحیح __weak bootz_تیاری(ulong image, ulong *آغاز, ulong *انتہا_2)
{
	 واپس 1;
}

صحیح __weak booti_تیاری(ulong image, ulong *relocated_addr, ulong *سائز, bool force_reloc)
{
	 واپس 1;
}
#اگر_ختم

/* Weak default function for arch specific fixups to the spl_image_info */
خالی __weak spl_perform_arch_fixups(ساخت spl_image_info *spl_image)
{
}

/* Weak default function for board specific fixups to the spl_image_info */
خالی __weak spl_perform_board_fixups(ساخت spl_image_info *spl_image)
{
}

خالی spl_fixup_fdt(خالی *fdt_blob)
{
#اگر defined(CONFIG_SPL_کا_LIBFDT)
	صحیح err;

	اگر (!fdt_blob)
		واپس;

	err = fdt_check_سرخی(fdt_blob);
	اگر (err < 0) {
		لکھو("fdt_root: %s\n", fdt_strerror(err));
		واپس;
	}

	/* fixup the memory dt node */
	err = fdt_shrink_مقصد_minimum(fdt_blob, 0);
	اگر (err == 0) {
		لکھو(PHASE_PROMPT "fdt_shrink_to_minimum err - %d\n", err);
		واپس;
	}

	err = arch_fixup_fdt(fdt_blob);
	اگر (err) {
		لکھو(PHASE_PROMPT "arch_fixup_fdt err - %d\n", err);
		واپس;
	}
#اگر_ختم
}

صحیح spl_reserve_video_from_ram_چوٹی(خالی)
{
	اگر (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		صحیح ret;

		addr = gd->ram_چوٹی;
		ret = video_reserve(&addr);
		اگر (ret)
			واپس ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((غیرعلامتی طویل)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	واپس 0;
}

ulong spl_get_image_pos(خالی)
{
	اگر (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		واپس BINMAN_SYM_MISSING;

#اگر_تعریف_شدہ CONFIG_VPL
	اگر (xpl_اگلا_phase() == PHASE_VPL)
		واپس binman_sym(ulong, u_boot_vpl_any, image_pos);
#اگر_ختم
#اگر defined(CONFIG_TPL) && !defined(CONFIG_VPL)
	اگر (xpl_اگلا_phase() == PHASE_SPL)
		واپس binman_sym(ulong, u_boot_spl_any, image_pos);
#اگر_ختم
#اگر defined(CONFIG_SPL_BUILD)
	واپس binman_sym(ulong, u_boot_any, image_pos);
#اگر_ختم

	واپس BINMAN_SYM_MISSING;
}

ulong spl_get_image_سائز(خالی)
{
	اگر (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		واپس BINMAN_SYM_MISSING;

#اگر_تعریف_شدہ CONFIG_VPL
	اگر (xpl_اگلا_phase() == PHASE_VPL)
		واپس binman_sym(ulong, u_boot_vpl_any, سائز);
#اگر_ختم
	واپس xpl_اگلا_phase() == PHASE_SPL ?
		binman_sym(ulong, u_boot_spl_any, سائز) :
		binman_sym(ulong, u_boot_any, سائز);
}

ulong spl_get_image_متن_بنیاد(خالی)
{
#اگر_تعریف_شدہ CONFIG_VPL
	اگر (xpl_اگلا_phase() == PHASE_VPL)
		واپس CONFIG_VPL_متن_بنیاد;
#اگر_ختم
	واپس xpl_اگلا_phase() == PHASE_SPL ? CONFIG_SPL_متن_بنیاد :
		CONFIG_متن_بنیاد;
}

/*
 * Weak default function for board specific cleanup/preparation before
 * Linux boot. Some boards/platforms might not need it, so just provide
 * an empty stub here.
 */
__weak خالی spl_board_prepare_for_linux(خالی)
{
	/* Nothing to do! */
}

__weak خالی spl_board_prepare_for_optee(خالی *fdt)
{
}

__weak ثابت حرف *spl_board_loader_name(u32 boot_device)
{
	واپس تہی;
}

#اگر CONFIG_IS_ENABLED(OPTEE_IMAGE)
__weak خالی __noreturn jump_مقصد_image_optee(ساخت spl_image_info *spl_image)
{
	spl_optee_اندراج(تہی, تہی, spl_image->fdt_addr,
			(خالی *)spl_image->اندراج_نقطہ);
}
#اگر_ختم

__weak خالی spl_board_prepare_for_boot(خالی)
{
	/* Nothing to do! */
}

__weak ساخت legacy_img_hdr *spl_get_بھرو_بفر(ssize_t انحراف, size_t سائز)
{
	واپس map_sysmem(CONFIG_متن_بنیاد + انحراف, 0);
}

خالی spl_سیٹ_سرخی_raw_uboot(ساخت spl_image_info *spl_image)
{
	ulong u_boot_pos = spl_get_image_pos();

#اگر CONFIG_SYS_MONITOR_LEN != 0
	spl_image->سائز = CONFIG_SYS_MONITOR_LEN;
#ورنہ
	/* Unknown U-Boot size, let's assume it will not be more than 200 KB */
	spl_image->سائز = 200 * 1024;
#اگر_ختم

	/*
	 * Binman error cases: address of the end of the previous region or the
	 * start of the image's entry area (usually 0) if there is no previous
	 * region.
	 */
	اگر (u_boot_pos && u_boot_pos != BINMAN_SYM_MISSING) {
		/* Binman does not support separated entry addresses */
		spl_image->اندراج_نقطہ = spl_get_image_متن_بنیاد();
		spl_image->بھرو_addr = spl_get_image_متن_بنیاد();
		spl_image->سائز = spl_get_image_سائز();
		log_debug("Next load addr %lx\n", spl_image->بھرو_addr);
	} ورنہ {
		spl_image->اندراج_نقطہ = CONFIG_SYS_UBOOT_آغاز;
		spl_image->بھرو_addr = CONFIG_متن_بنیاد;
		log_debug("Default load addr %lx (u_boot_pos=%lx)\n",
			  spl_image->بھرو_addr, u_boot_pos);
	}
	spl_image->os = IH_OS_U_BOOT;
	spl_image->name = xpl_name(xpl_اگلا_phase());
	log_debug("Next phase: %s at %lx size %lx\n", spl_image->name,
		  spl_image->بھرو_addr, (ulong)spl_image->سائز);
}

__weak صحیح spl_parse_board_سرخی(ساخت spl_image_info *spl_image,
				  ثابت ساخت spl_boot_device *bootdev,
				  ثابت خالی *image_سرخی, size_t سائز)
{
	واپس -EINVAL;
}

__weak صحیح spl_parse_legacy_سرخی(ساخت spl_image_info *spl_image,
				   ثابت ساخت legacy_img_hdr *سرخی)
{
	/* LEGACY image not supported */
	debug("Legacy boot image support not enabled, proceeding to other boot methods\n");
	واپس -EINVAL;
}

صحیح spl_parse_image_سرخی(ساخت spl_image_info *spl_image,
			   ثابت ساخت spl_boot_device *bootdev,
			   ثابت ساخت legacy_img_hdr *سرخی)
{
	صحیح ret;

	اگر (CONFIG_IS_ENABLED(بھرو_FIT_بھرا)) {
		ret = spl_بھرو_fit_image(spl_image, سرخی);

		اگر (!ret)
			واپس ret;
	}
	اگر (image_get_magic(سرخی) == IH_MAGIC) {
		صحیح ret;

		ret = spl_parse_legacy_سرخی(spl_image, سرخی);
		اگر (ret)
			واپس ret;
		واپس 0;
	}

	اگر (IS_ENABLED(CONFIG_SPL_PANIC_ON_RAW_IMAGE)) {
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

	اگر (IS_ENABLED(CONFIG_SPL_OS_BOOT) && IS_ENABLED(CONFIG_SPL_BOOTI)) {
		ulong آغاز, سائز;

		اگر (!booti_تیاری((ulong)سرخی, &آغاز, &سائز, 0)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->بھرو_addr = آغاز;
			spl_image->اندراج_نقطہ = آغاز;
			spl_image->سائز = سائز;
			debug(PHASE_PROMPT
			      "payload Image, load addr: 0x%lx size: %d\n",
			      spl_image->بھرو_addr, spl_image->سائز);
			واپس 0;
		}
	} ورنہ اگر (IS_ENABLED(CONFIG_SPL_OS_BOOT) &&
		   IS_ENABLED(CONFIG_SPL_BOOTZ)) {
		ulong آغاز, انتہا_2;

		اگر (!bootz_تیاری((ulong)سرخی, &آغاز, &انتہا_2)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->بھرو_addr = CONFIG_SYS_بھرو_ADDR;
			spl_image->اندراج_نقطہ = CONFIG_SYS_بھرو_ADDR;
			spl_image->سائز = انتہا_2 - آغاز;
			debug(PHASE_PROMPT
			      "payload zImage, load addr: 0x%lx size: %d\n",
			      spl_image->بھرو_addr, spl_image->سائز);
			واپس 0;
		}
	}

	اگر (!spl_parse_board_سرخی(spl_image, bootdev, (ثابت خالی *)سرخی,
				    حجم(*سرخی)))
		واپس 0;

	اگر (IS_ENABLED(CONFIG_SPL_RAW_IMAGE_SUPPORT)) {
		/* Signature not found - assume u-boot.bin */
		debug("mkimage signature not found - ih_magic = %x\n",
		      سرخی->ih_magic);
		spl_سیٹ_سرخی_raw_uboot(spl_image);
	} ورنہ {
		/* RAW image not supported, proceed to other boot methods. */
		debug("Raw boot image support not enabled, proceeding to other boot methods\n");
		واپس -EINVAL;
	}

	واپس 0;
}

#اگر SPL_بھرو_USERS > 1
صحیح spl_بھرو(ساخت spl_image_info *spl_image,
	     ثابت ساخت spl_boot_device *bootdev, ساخت spl_بھرو_info *info,
	     size_t سائز, size_t انحراف)
{
	واپس spl_بھرو_2(spl_image, bootdev, info, سائز, انحراف);
}
#اگر_ختم

__weak خالی __noreturn jump_مقصد_image(ساخت spl_image_info *spl_image)
{
	قسمتعریف خالی __noreturn (*image_اندراج_noargs_t)(خالی);

	image_اندراج_noargs_t image_اندراج =
		(image_اندراج_noargs_t)spl_image->اندراج_نقطہ;

	debug("image entry point: 0x%lx\n", spl_image->اندراج_نقطہ);
	image_اندراج();
}

#اگر CONFIG_IS_ENABLED(HANDOFF)
/**
 * Set up the SPL hand-off information
 *
 * This is initially empty (zero) but can be written by
 */
ساکن صحیح تیاری_spl_handoff(خالی)
{
	ساخت spl_handoff *ho;

	ho = bloblist_ensure(BLOBLISTT_U_BOOT_SPL_HANDOFF, حجم(ساخت spl_handoff));
	اگر (!ho)
		واپس -ENOENT;

	واپس 0;
}

__weak صحیح handoff_arch_save(ساخت spl_handoff *ho)
{
	واپس 0;
}

ساکن صحیح write_spl_handoff(خالی)
{
	ساخت spl_handoff *ho;
	صحیح ret;

	ho = bloblist_ڈھونڈو(BLOBLISTT_U_BOOT_SPL_HANDOFF, حجم(ساخت spl_handoff));
	اگر (!ho)
		واپس -ENOENT;
	handoff_save_dram(ho);
	ret = handoff_arch_save(ho);
	اگر (ret)
		واپس ret;
	debug(PHASE_PROMPT "Wrote SPL handoff\n");

	واپس 0;
}
#ورنہ
ساکن اندرونی صحیح تیاری_spl_handoff(خالی) { واپس 0; }
ساکن اندرونی صحیح write_spl_handoff(خالی) { واپس 0; }

#اگر_ختم /* HANDOFF */

/**
 * get_bootstage_id() - Get the bootstage ID to emit
 *
 * @start: true if this is for starting SPL, false for ending it
 * Return: bootstage ID to use
 */
ساکن شمار bootstage_id get_bootstage_id(bool آغاز)
{
	شمار xpl_phase_t phase = xpl_phase();

	اگر (IS_ENABLED(CONFIG_TPL_BUILD) && phase == PHASE_TPL)
		واپس آغاز ? BOOTSTAGE_ID_آغاز_TPL : BOOTSTAGE_ID_انتہا_TPL;
	ورنہ اگر (IS_ENABLED(CONFIG_VPL_BUILD) && phase == PHASE_VPL)
		واپس آغاز ? BOOTSTAGE_ID_آغاز_VPL : BOOTSTAGE_ID_انتہا_VPL;
	ورنہ
		واپس آغاز ? BOOTSTAGE_ID_آغاز_SPL : BOOTSTAGE_ID_انتہا_SPL;
}

ساکن صحیح spl_common_init(bool تیاری_malloc)
{
	صحیح ret;

#اگر CONFIG_IS_ENABLED(SYS_MALLOC_F)
	اگر (تیاری_malloc) {
#اگر_تعریف_شدہ CFG_MALLOC_F_ADDR
		gd->malloc_بنیاد = CFG_MALLOC_F_ADDR;
#اگر_ختم
		gd->malloc_حد_بندی = CONFIG_VAL(SYS_MALLOC_F_LEN);
		gd->malloc_ptr = 0;
	}
#اگر_ختم
	ret = bootstage_init(xpl_is_پہلا_phase());
	اگر (ret) {
		debug("%s: Failed to set up bootstage: ret=%d\n", __func__,
		      ret);
		واپس ret;
	}
	اگر (!xpl_is_پہلا_phase()) {
		ret = bootstage_unstash_default();
		اگر (ret)
			log_debug("Failed to unstash bootstage: ret=%d\n", ret);
	}
	bootstage_mark_name(get_bootstage_id(true), xpl_name(xpl_phase()));
#اگر CONFIG_IS_ENABLED(LOG)
	ret = log_init();
	اگر (ret) {
		debug("%s: Failed to set up logging\n", __func__);
		واپس ret;
	}
#اگر_ختم
	اگر (CONFIG_IS_ENABLED(کا_REAL)) {
		ret = fdtdec_تیاری();
		اگر (ret) {
			debug("fdtdec_setup() returned error %d\n", ret);
			واپس ret;
		}
	}
	اگر (CONFIG_IS_ENABLED(کا_زندہ)) {
		bootstage_آغاز(BOOTSTAGE_ID_ACCUM_کا_زندہ, "of_live");
		ret = کا_زندہ_build(gd->fdt_blob,
				    (ساخت device_گرہ **)gd_کا_جڑ_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_کا_زندہ);
		اگر (ret)
			واپس ret;
	}
	اگر (CONFIG_IS_ENABLED(DM)) {
		bootstage_آغاز(BOOTSTAGE_ID_ACCUM_DM_SPL,
				xpl_phase() == PHASE_TPL ? "dm tpl" : "dm_spl");
		/* With CONFIG_SPL_OF_PLATDATA, bring in all devices */
		ret = dm_init_and_جائزہ(!CONFIG_IS_ENABLED(کا_PLATDATA));
		bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_SPL);
		اگر (ret) {
			debug("dm_init_and_scan() returned error %d\n", ret);
			واپس ret;
		}

		ret = dm_autoprobe();
		اگر (ret)
			واپس ret;
	}

	واپس 0;
}

خالی spl_سیٹ_bd(خالی)
{
	/*
	 * NOTE: On some platforms (e.g. x86) bdata may be in flash and not
	 * writeable.
	 */
	اگر (!gd->bd)
		gd->bd = &bdata;
}

صحیح spl_early_init(خالی)
{
	صحیح ret;

	debug("%s\n", __func__);

	ret = spl_common_init(true);
	اگر (ret)
		واپس ret;
	gd->flags |= GD_FLG_SPL_EARLY_INIT;

	واپس 0;
}

صحیح spl_init(خالی)
{
	صحیح ret;
	bool تیاری_malloc = !(IS_ENABLED(CONFIG_SPL_ڈھیر_R) &&
			IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIMPLE));

	debug("%s\n", __func__);

	اگر (!(gd->flags & GD_FLG_SPL_EARLY_INIT)) {
		ret = spl_common_init(تیاری_malloc);
		اگر (ret)
			واپس ret;
	}
	gd->flags |= GD_FLG_SPL_INIT;

	واپس 0;
}

#اگر_غیر_تعریف_شدہ BOOT_DEVICE_NONE
#تعریف_کرو BOOT_DEVICE_NONE 0xdeadbeef
#اگر_ختم

__weak خالی board_boot_ترتیب(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();
}

__weak صحیح spl_check_board_image(ساخت spl_image_info *spl_image,
				 ثابت ساخت spl_boot_device *bootdev)
{
	واپس 0;
}

ساکن صحیح spl_بھرو_image(ساخت spl_image_info *spl_image,
			  ساخت spl_image_loader *loader)
{
	صحیح ret;
	ساخت spl_boot_device bootdev;

	bootdev.boot_device = loader->boot_device;
	bootdev.boot_device_name = تہی;

	ret = loader->بھرو_image(spl_image, &bootdev);
#اگر_تعریف_شدہ CONFIG_SPL_LEGACY_IMAGE_چکری_جانچ_CHECK
	اگر (!ret && spl_image->dcrc_لمبائی) {
		/* check data crc */
		ulong dcrc = چکری_جانچ_32_wd(0, (غیرعلامتی حرف *)spl_image->dcrc_ڈیٹا,
				      spl_image->dcrc_لمبائی, CHUNKSZ_چکری_جانچ_32);
		اگر (dcrc != spl_image->dcrc) {
			puts("SPL: Image data CRC check failed!\n");
			ret = -EINVAL;
		}
	}
#اگر_ختم
	اگر (!ret)
		ret = spl_check_board_image(spl_image, &bootdev);

	واپس ret;
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
ساکن صحیح boot_from_devices(ساخت spl_image_info *spl_image,
			     u32 spl_boot_list[], صحیح گنتی)
{
	ساخت spl_image_loader *drv =
		ll_اندراج_آغاز(ساخت spl_image_loader, spl_image_loader);
	ثابت صحیح n_ents =
		ll_اندراج_گنتی(ساخت spl_image_loader, spl_image_loader);
	صحیح ret = -ENODEV;
	صحیح i;

	برائے (i = 0; i < گنتی && spl_boot_list[i] != BOOT_DEVICE_NONE; i++) {
		ساخت spl_image_loader *loader;
		صحیح bootdev = spl_boot_list[i];

		اگر (CONFIG_IS_ENABLED(SHOW_ERRORS))
			ret = -ENXIO;
		برائے (loader = drv; loader != drv + n_ents; loader++) {
			اگر (loader && bootdev != loader->boot_device)
				جاری;
			اگر (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
				لکھو("Trying to boot from %s\n",
				       spl_loader_name(loader));
			}

			ret = spl_بھرو_image(spl_image, loader);
			اگر (!ret) {
				spl_image->boot_device = bootdev;
				واپس 0;
			}
			لکھو("Error: %d\n", ret);
		}

		اگر (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
			اگر (CONFIG_IS_ENABLED(SHOW_ERRORS))
				لکھو(PHASE_PROMPT
				       "Unsupported Boot Device %d\n",
				       bootdev);
			ورنہ
				لکھو(PHASE_PROMPT
				       "Unsupported Boot Device!\n");
		}
	}

	واپس ret;
}

خالی board_init_r(gd_t *dummy1, ulong dummy2)
{
	u32 spl_boot_list[] = {
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
	};
	spl_jump_مقصد_image_t jumper = &jump_مقصد_image;
	ساخت spl_image_info spl_image;
	صحیح ret, os;
	خالی *fdt;

	debug(">>" PHASE_PROMPT "board_init_r()\n");

	spl_سیٹ_bd();

	اگر (IS_ENABLED(CONFIG_SPL_SYS_MALLOC)) {
		mem_malloc_init(SPL_SYS_MALLOC_آغاز, SPL_SYS_MALLOC_سائز);
		gd->flags |= GD_FLG_بھرا_MALLOC_INIT;
	}
	اگر (!(gd->flags & GD_FLG_SPL_INIT)) {
		اگر (spl_init())
			hang();
	}
	timer_init();
	اگر (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_init();
		اگر (ret) {
			debug("%s: Failed to set up bloblist: ret=%d\n",
			      __func__, ret);
			puts(PHASE_PROMPT "Cannot set up bloblist\n");
			hang();
		}
	}
	اگر (CONFIG_IS_ENABLED(HANDOFF)) {
		صحیح ret;

		ret = تیاری_spl_handoff();
		اگر (ret) {
			puts(PHASE_PROMPT "Cannot set up SPL handoff\n");
			hang();
		}
	}

	اگر (CONFIG_IS_ENABLED(SOC_INIT))
		spl_soc_init();

	اگر (IS_ENABLED(CONFIG_SPL_WATCHDOG) && CONFIG_IS_ENABLED(WDT))
		initr_watchdog();

	اگر (IS_ENABLED(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) ||
	    IS_ENABLED(CONFIG_SPL_ATF) || IS_ENABLED(CONFIG_SPL_NET))
		dram_init_banksize();

	اگر (IS_ENABLED(CONFIG_SPL_LMB))
		lmb_init();

	اگر (CONFIG_IS_ENABLED(PCI) && !(gd->flags & GD_FLG_DM_DEAD)) {
		ret = pci_init();
		اگر (ret)
			puts(PHASE_PROMPT "Cannot initialize PCI\n");
		/* Don't fail. We still can try other boot methods. */
	}

	اگر (CONFIG_IS_ENABLED(BOARD_INIT))
		spl_board_init();

	bootcount_inc();

	/* Dump driver model states to aid analysis */
	اگر (CONFIG_IS_ENABLED(DM_STATS)) {
		ساخت dm_stats mem;

		dm_get_mem(&mem);
		dm_dump_mem(&mem);
	}

	memset(&spl_image, '\0', حجم(spl_image));
	اگر (IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image.arg = (خالی *)SPL_PAYLOAD_دلائل_ADDR;
	spl_image.boot_device = BOOT_DEVICE_NONE;
	board_boot_ترتیب(spl_boot_list);

	ret = boot_from_devices(&spl_image, spl_boot_list,
				ARRAY_سائز(spl_boot_list));
	اگر (ret) {
		اگر (CONFIG_IS_ENABLED(SHOW_ERRORS))
			لکھو(PHASE_PROMPT "failed to boot from all boot devices (err=%d)\n",
			       ret);
		ورنہ
			puts(PHASE_PROMPT "failed to boot from all boot devices\n");
		hang();
	}

	spl_perform_arch_fixups(&spl_image);
	spl_perform_board_fixups(&spl_image);

	os = spl_image.os;
	اگر (os == IH_OS_U_BOOT) {
		debug("Jumping to %s...\n", xpl_name(xpl_اگلا_phase()));
	} ورنہ اگر (CONFIG_IS_ENABLED(ATF) && os == IH_OS_ARM_TRUSTED_FIRMWARE) {
		debug("Jumping to U-Boot via ARM Trusted Firmware\n");
		spl_fixup_fdt(spl_image_fdt_addr(&spl_image));
		jumper = &spl_invoke_atf;
	} ورنہ اگر (CONFIG_IS_ENABLED(OPTEE_IMAGE) && os == IH_OS_TEE) {
		debug("Jumping to U-Boot via OP-TEE\n");
		spl_board_prepare_for_optee(spl_image_fdt_addr(&spl_image));
		jumper = &jump_مقصد_image_optee;
	} ورنہ اگر (CONFIG_IS_ENABLED(OPENSBI) && os == IH_OS_OPENSBI) {
		debug("Jumping to U-Boot via RISC-V OpenSBI\n");
		jumper = &spl_invoke_opensbi;
	} ورنہ اگر (CONFIG_IS_ENABLED(OS_BOOT) && os == IH_OS_LINUX) {
		debug("Jumping to Linux\n");
		اگر (CONFIG_IS_ENABLED(OS_BOOT_دلائل))
			fdt = (خالی *)SPL_PAYLOAD_دلائل_ADDR;
		ورنہ
			fdt = spl_image_fdt_addr(&spl_image);
		spl_fixup_fdt(fdt);
		spl_board_prepare_for_linux();
		spl_image.arg = fdt;
		jumper = &jump_مقصد_image_linux;
	} ورنہ {
		debug("Unsupported OS image.. Jumping nevertheless..\n");
	}
	اگر (CONFIG_IS_ENABLED(SYS_MALLOC_F) &&
	    !IS_ENABLED(CONFIG_SPL_SYS_MALLOC_سائز))
		debug("SPL malloc() used 0x%x bytes (%d KB)\n",
		      gd_malloc_ptr(), gd_malloc_ptr() / 1024);

	bootstage_mark_name(get_bootstage_id(false), "end phase");
	ret = bootstage_stash_default();
	اگر (ret)
		debug("Failed to stash bootstage: err=%d\n", ret);

	اگر (IS_ENABLED(CONFIG_SPL_VIDEO_REMOVE)) {
		ساخت udevice *dev;
		صحیح rc;

		rc = uclass_ڈھونڈو_device(UCLASS_VIDEO, 0, &dev);
		اگر (!rc && dev) {
			rc = device_remove(dev, DM_REMOVE_معمول);
			اگر (rc)
				لکھو("Cannot remove video device '%s' (err=%d)\n",
				       dev->name, rc);
		}
	}
	اگر (CONFIG_IS_ENABLED(HANDOFF)) {
		ret = write_spl_handoff();
		اگر (ret)
			لکھو(PHASE_PROMPT
			       "SPL hand-off write failed (err=%d)\n", ret);
	}
	اگر (CONFIG_IS_ENABLED(UPL_برآمد) && (gd->flags & GD_FLG_UPL)) {
		ret = spl_write_upl_handoff(&spl_image);
		اگر (ret) {
			لکھو(PHASE_PROMPT
			       "UPL hand-off write failed (err=%d)\n", ret);
			hang();
		}
	}
	اگر (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_اختتام();
		اگر (ret)
			لکھو("Warning: Failed to finish bloblist (ret=%d)\n",
			       ret);
	}

	spl_board_prepare_for_boot();

	اگر (CONFIG_IS_ENABLED(RELOC_LOADER)) {
		صحیح ret;

		ret = spl_reloc_jump(&spl_image, jumper);
		اگر (ret) {
			اگر (xpl_phase() == PHASE_VPL)
				لکھو("jump failed %d\n", ret);
			hang();
		}
	}

	jumper(&spl_image);
}

/*
 * This requires UART clocks to be enabled.  In order for this to work the
 * caller must ensure that the gd pointer is valid.
 */
خالی preloader_console_init(خالی)
{
#اگر CONFIG_IS_ENABLED(SERIAL)
	gd->baudrate = CONFIG_BAUDRATE;

	serial_init();		/* serial communications setup */

	gd->flags |= GD_FLG_HAVE_CONSOLE;

#اگر CONFIG_IS_ENABLED(BANNER_PRINT)
	puts("\nU-Boot " PHASE_NAME " " PLAIN_نسخہ " (" U_BOOT_DATE " - "
	     U_BOOT_وقت " " U_BOOT_TZ ")\n");
#اگر_ختم
#اگر_تعریف_شدہ CONFIG_SPL_DISPLAY_PRINT
	spl_display_print();
#اگر_ختم
#اگر_ختم
}

/**
 * This function is called before the stack is changed from initial stack to
 * relocated stack. It tries to dump the stack size used
 */
__weak خالی spl_relocate_ڈھیر_check(خالی)
{
#اگر CONFIG_IS_ENABLED(SYS_REPORT_ڈھیر_F_USAGE)
	ulong init_sp = gd->آغاز_addr_sp;
	ulong ڈھیر_bottom = init_sp - CONFIG_VAL(سائز_حد_بندی_PROVIDE_ڈھیر);
	u8 *ptr = (u8 *)ڈھیر_bottom;
	ulong i;

	برائے (i = 0; i < CONFIG_VAL(سائز_حد_بندی_PROVIDE_ڈھیر); i++) {
		اگر (*ptr != CONFIG_VAL(SYS_ڈھیر_F_CHECK_BYTE))
			توڑو;
		ptr++;
	}
	لکھو("SPL initial stack usage: %lu bytes\n",
	       CONFIG_VAL(سائز_حد_بندی_PROVIDE_ڈھیر) - i);
#اگر_ختم
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
ulong spl_relocate_ڈھیر_gd(خالی)
{
#اگر CONFIG_IS_ENABLED(ڈھیر_R)
	gd_t *new_gd;
	ulong ptr = CONFIG_SPL_ڈھیر_R_ADDR;

	اگر (CONFIG_IS_ENABLED(SYS_REPORT_ڈھیر_F_USAGE))
		spl_relocate_ڈھیر_check();

#اگر defined(CONFIG_SPL_SYS_MALLOC_SIMPLE) && CONFIG_IS_ENABLED(SYS_MALLOC_F)
	اگر (CONFIG_SPL_ڈھیر_R_MALLOC_SIMPLE_LEN) {
		debug("SPL malloc() before relocation used 0x%x bytes (%d KB)\n",
		      gd->malloc_ptr, gd->malloc_ptr / 1024);
		ptr -= CONFIG_SPL_ڈھیر_R_MALLOC_SIMPLE_LEN;
		gd->malloc_بنیاد = ptr;
		gd->malloc_حد_بندی = CONFIG_SPL_ڈھیر_R_MALLOC_SIMPLE_LEN;
		gd->malloc_ptr = 0;
	}
#اگر_ختم
	/* Get stack position: use 8-byte alignment for ABI compliance */
	ptr -= roundup(حجم(gd_t), 16);
	gd->آغاز_addr_sp = ptr;
	new_gd = (gd_t *)ptr;
	حافظہ_نقل(new_gd, (خالی *)gd, حجم(gd_t));
#اگر CONFIG_IS_ENABLED(DM)
	dm_fixup_for_gd_move(new_gd);
#اگر_ختم
#اگر CONFIG_IS_ENABLED(LOG)
	log_fixup_for_gd_move(new_gd);
#اگر_ختم
#اگر !defined(CONFIG_ARM) && !defined(CONFIG_RISCV)
	gd = new_gd;
#اگر_ختم
	واپس ptr;
#ورنہ
	واپس 0;
#اگر_ختم
}

#اگر defined(CONFIG_BOOTCOUNT_حد_بندی) && \
	((!defined(CONFIG_TPL_BUILD) && !defined(CONFIG_SPL_BOOTCOUNT_حد_بندی)) || \
	 (defined(CONFIG_TPL_BUILD) && !defined(CONFIG_TPL_BOOTCOUNT_حد_بندی)))
خالی bootcount_store(ulong a)
{
}

ulong bootcount_بھرو(خالی)
{
	واپس 0;
}
#اگر_ختم
