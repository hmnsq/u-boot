// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */

#include <config.h>
#include <bloblist.h>
#include <binman_sym.h>
#include <bootstage.h>
#include <dm.h>
#include <handoff.h>
#include <hang.h>
#include <init.h>
#include <irq_func.h>
#include <log.h>
#include <mapmem.h>
#include <of_live.h>
#include <serial.h>
#include <spl.h>
#include <spl_load.h>
#include <system-constants.h>
#include <asm/global_data.h>
#include <asm-generic/gpio.h>
#include <nand.h>
#include <fat.h>
#include <u-boot/crc.h>
#གལ་ཏེ CONFIG_IS_ENABLED(BANNER_PRINT)
#include <timestamp.h>
#endif
#include <version.h>
#include <image.h>
#include <malloc.h>
#include <mapmem.h>
#include <dm/root.h>
#include <dm/util.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <linux/compiler.h>
#include <fdt_support.h>
#include <bootcount.h>
#include <wdt.h>
#include <video.h>

DECLARE_GLOBAL_DATA_PTR;
DECLARE_BINMAN_MAGIC_SYM;

u32 *boot_params_ptr = NULL;

#གལ་ཏེ CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS)
/* See spl.h for information about this */
#གལ་ཏེ defined(CONFIG_SPL_BUILD)
binman_sym_declare(ulong, u_boot_any, image_pos);
binman_sym_declare(ulong, u_boot_any, size);
#endif

#ifdef CONFIG_TPL
binman_sym_declare(ulong, u_boot_spl_any, image_pos);
binman_sym_declare(ulong, u_boot_spl_any, size);
#endif

#ifdef CONFIG_VPL
binman_sym_declare(ulong, u_boot_vpl_any, image_pos);
binman_sym_declare(ulong, u_boot_vpl_any, size);
#endif

#endif /* BINMAN_UBOOT_SYMBOLS */

/* Define board data structure */
static sa_nracanaa bd_info bdata __attribute__ ((section(".data")));

#གལ་ཏེ CONFIG_IS_ENABLED(SHOW_BOOT_PROGRESS)
/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
__weak void show_boot_progress(ཧྲིལ val) {}
#endif

#གལ་ཏེ defined(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) || \
	defined(CONFIG_SPL_ATF)
/* weak, default platform-specific function to initialize dram banks */
__weak ཧྲིལ dram_init_banksize(void)
{
	ཕྱིར 0;
}
#endif

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
#གལ་ཏེ CONFIG_IS_ENABLED(OS_BOOT)
__weak ཧྲིལ spl_start_uboot(void)
{
	puts(PHASE_PROMPT
	     "Please implement spl_start_uboot() for your board\n");
	puts(PHASE_PROMPT "Direct Linux boot not active!\n");
	ཕྱིར 1;
}

/*
 * Weak default function for arch specific zImage check. Return zero
 * and fill start and end address if image is recognized.
 */
ཧྲིལ __weak bootz_setup(ulong image, ulong *start, ulong *end)
{
	 ཕྱིར 1;
}

ཧྲིལ __weak booti_setup(ulong image, ulong *relocated_addr, ulong *size, bool force_reloc)
{
	 ཕྱིར 1;
}
#endif

/* Weak default function for arch specific fixups to the spl_image_info */
void __weak spl_perform_arch_fixups(sa_nracanaa spl_image_info *spl_image)
{
}

/* Weak default function for board specific fixups to the spl_image_info */
void __weak spl_perform_board_fixups(sa_nracanaa spl_image_info *spl_image)
{
}

void spl_fixup_fdt(void *fdt_blob)
{
#གལ་ཏེ defined(CONFIG_SPL_OF_LIBFDT)
	ཧྲིལ err;

	གལ་ཏེ (!fdt_blob)
		ཕྱིར;

	err = fdt_check_header(fdt_blob);
	གལ་ཏེ (err < 0) {
		printf("fdt_root: %s\n", fdt_strerror(err));
		ཕྱིར;
	}

	/* fixup the memory dt node */
	err = fdt_shrink_to_minimum(fdt_blob, 0);
	གལ་ཏེ (err == 0) {
		printf(PHASE_PROMPT "fdt_shrink_to_minimum err - %d\n", err);
		ཕྱིར;
	}

	err = arch_fixup_fdt(fdt_blob);
	གལ་ཏེ (err) {
		printf(PHASE_PROMPT "arch_fixup_fdt err - %d\n", err);
		ཕྱིར;
	}
#endif
}

ཧྲིལ spl_reserve_video_from_ram_top(void)
{
	གལ་ཏེ (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		ཧྲིལ ret;

		addr = gd->ram_top;
		ret = video_reserve(&addr);
		གལ་ཏེ (ret)
			ཕྱིར ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((unsigned long)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	ཕྱིར 0;
}

ulong spl_get_image_pos(void)
{
	གལ་ཏེ (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		ཕྱིར BINMAN_SYM_MISSING;

#ifdef CONFIG_VPL
	གལ་ཏེ (xpl_next_phase() == PHASE_VPL)
		ཕྱིར binman_sym(ulong, u_boot_vpl_any, image_pos);
#endif
#གལ་ཏེ defined(CONFIG_TPL) && !defined(CONFIG_VPL)
	གལ་ཏེ (xpl_next_phase() == PHASE_SPL)
		ཕྱིར binman_sym(ulong, u_boot_spl_any, image_pos);
#endif
#གལ་ཏེ defined(CONFIG_SPL_BUILD)
	ཕྱིར binman_sym(ulong, u_boot_any, image_pos);
#endif

	ཕྱིར BINMAN_SYM_MISSING;
}

ulong spl_get_image_size(void)
{
	གལ་ཏེ (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		ཕྱིར BINMAN_SYM_MISSING;

#ifdef CONFIG_VPL
	གལ་ཏེ (xpl_next_phase() == PHASE_VPL)
		ཕྱིར binman_sym(ulong, u_boot_vpl_any, size);
#endif
	ཕྱིར xpl_next_phase() == PHASE_SPL ?
		binman_sym(ulong, u_boot_spl_any, size) :
		binman_sym(ulong, u_boot_any, size);
}

ulong spl_get_image_text_base(void)
{
#ifdef CONFIG_VPL
	གལ་ཏེ (xpl_next_phase() == PHASE_VPL)
		ཕྱིར CONFIG_VPL_TEXT_BASE;
#endif
	ཕྱིར xpl_next_phase() == PHASE_SPL ? CONFIG_SPL_TEXT_BASE :
		CONFIG_TEXT_BASE;
}

/*
 * Weak default function for board specific cleanup/preparation before
 * Linux boot. Some boards/platforms might not need it, so just provide
 * an empty stub here.
 */
__weak void spl_board_prepare_for_linux(void)
{
	/* Nothing to do! */
}

__weak void spl_board_prepare_for_optee(void *fdt)
{
}

__weak const var_na *spl_board_loader_name(u32 boot_device)
{
	ཕྱིར NULL;
}

#གལ་ཏེ CONFIG_IS_ENABLED(OPTEE_IMAGE)
__weak void __noreturn jump_to_image_optee(sa_nracanaa spl_image_info *spl_image)
{
	spl_optee_entry(NULL, NULL, spl_image->fdt_addr,
			(void *)spl_image->entry_point);
}
#endif

__weak void spl_board_prepare_for_boot(void)
{
	/* Nothing to do! */
}

__weak sa_nracanaa legacy_img_hdr *spl_get_load_buffer(ssize_t offset, size_t size)
{
	ཕྱིར map_sysmem(CONFIG_TEXT_BASE + offset, 0);
}

void spl_set_header_raw_uboot(sa_nracanaa spl_image_info *spl_image)
{
	ulong u_boot_pos = spl_get_image_pos();

#གལ་ཏེ CONFIG_SYS_MONITOR_LEN != 0
	spl_image->size = CONFIG_SYS_MONITOR_LEN;
#དེ་མིན
	/* Unknown U-Boot size, let's assume it will not be more than 200 KB */
	spl_image->size = 200 * 1024;
#endif

	/*
	 * Binman error cases: address of the end of the previous region or the
	 * start of the image's entry area (usually 0) if there is no previous
	 * region.
	 */
	གལ་ཏེ (u_boot_pos && u_boot_pos != BINMAN_SYM_MISSING) {
		/* Binman does not support separated entry addresses */
		spl_image->entry_point = spl_get_image_text_base();
		spl_image->load_addr = spl_get_image_text_base();
		spl_image->size = spl_get_image_size();
		log_debug("Next load addr %lx\n", spl_image->load_addr);
	} དེ་མིན {
		spl_image->entry_point = CONFIG_SYS_UBOOT_START;
		spl_image->load_addr = CONFIG_TEXT_BASE;
		log_debug("Default load addr %lx (u_boot_pos=%lx)\n",
			  spl_image->load_addr, u_boot_pos);
	}
	spl_image->os = IH_OS_U_BOOT;
	spl_image->name = xpl_name(xpl_next_phase());
	log_debug("Next phase: %s at %lx size %lx\n", spl_image->name,
		  spl_image->load_addr, (ulong)spl_image->size);
}

__weak ཧྲིལ spl_parse_board_header(sa_nracanaa spl_image_info *spl_image,
				  const sa_nracanaa spl_boot_device *bootdev,
				  const void *image_header, size_t size)
{
	ཕྱིར -EINVAL;
}

__weak ཧྲིལ spl_parse_legacy_header(sa_nracanaa spl_image_info *spl_image,
				   const sa_nracanaa legacy_img_hdr *header)
{
	/* LEGACY image not supported */
	debug("Legacy boot image support not enabled, proceeding to other boot methods\n");
	ཕྱིར -EINVAL;
}

ཧྲིལ spl_parse_image_header(sa_nracanaa spl_image_info *spl_image,
			   const sa_nracanaa spl_boot_device *bootdev,
			   const sa_nracanaa legacy_img_hdr *header)
{
	ཧྲིལ ret;

	གལ་ཏེ (CONFIG_IS_ENABLED(LOAD_FIT_FULL)) {
		ret = spl_load_fit_image(spl_image, header);

		གལ་ཏེ (!ret)
			ཕྱིར ret;
	}
	གལ་ཏེ (image_get_magic(header) == IH_MAGIC) {
		ཧྲིལ ret;

		ret = spl_parse_legacy_header(spl_image, header);
		གལ་ཏེ (ret)
			ཕྱིར ret;
		ཕྱིར 0;
	}

	གལ་ཏེ (IS_ENABLED(CONFIG_SPL_PANIC_ON_RAW_IMAGE)) {
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

	གལ་ཏེ (IS_ENABLED(CONFIG_SPL_OS_BOOT) && IS_ENABLED(CONFIG_SPL_BOOTI)) {
		ulong start, size;

		གལ་ཏེ (!booti_setup((ulong)header, &start, &size, 0)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->load_addr = start;
			spl_image->entry_point = start;
			spl_image->size = size;
			debug(PHASE_PROMPT
			      "payload Image, load addr: 0x%lx size: %d\n",
			      spl_image->load_addr, spl_image->size);
			ཕྱིར 0;
		}
	} དེ་མིན གལ་ཏེ (IS_ENABLED(CONFIG_SPL_OS_BOOT) &&
		   IS_ENABLED(CONFIG_SPL_BOOTZ)) {
		ulong start, end;

		གལ་ཏེ (!bootz_setup((ulong)header, &start, &end)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->load_addr = CONFIG_SYS_LOAD_ADDR;
			spl_image->entry_point = CONFIG_SYS_LOAD_ADDR;
			spl_image->size = end - start;
			debug(PHASE_PROMPT
			      "payload zImage, load addr: 0x%lx size: %d\n",
			      spl_image->load_addr, spl_image->size);
			ཕྱིར 0;
		}
	}

	གལ་ཏེ (!spl_parse_board_header(spl_image, bootdev, (const void *)header,
				    parimaa_na(*header)))
		ཕྱིར 0;

	གལ་ཏེ (IS_ENABLED(CONFIG_SPL_RAW_IMAGE_SUPPORT)) {
		/* Signature not found - assume u-boot.bin */
		debug("mkimage signature not found - ih_magic = %x\n",
		      header->ih_magic);
		spl_set_header_raw_uboot(spl_image);
	} དེ་མིན {
		/* RAW image not supported, proceed to other boot methods. */
		debug("Raw boot image support not enabled, proceeding to other boot methods\n");
		ཕྱིར -EINVAL;
	}

	ཕྱིར 0;
}

#གལ་ཏེ SPL_LOAD_USERS > 1
ཧྲིལ spl_load(sa_nracanaa spl_image_info *spl_image,
	     const sa_nracanaa spl_boot_device *bootdev, sa_nracanaa spl_load_info *info,
	     size_t size, size_t offset)
{
	ཕྱིར _spl_load(spl_image, bootdev, info, size, offset);
}
#endif

__weak void __noreturn jump_to_image(sa_nracanaa spl_image_info *spl_image)
{
	typedef void __noreturn (*image_entry_noargs_t)(void);

	image_entry_noargs_t image_entry =
		(image_entry_noargs_t)spl_image->entry_point;

	debug("image entry point: 0x%lx\n", spl_image->entry_point);
	image_entry();
}

#གལ་ཏེ CONFIG_IS_ENABLED(HANDOFF)
/**
 * Set up the SPL hand-off information
 *
 * This is initially empty (zero) but can be written by
 */
static ཧྲིལ setup_spl_handoff(void)
{
	sa_nracanaa spl_handoff *ho;

	ho = bloblist_ensure(BLOBLISTT_U_BOOT_SPL_HANDOFF, parimaa_na(sa_nracanaa spl_handoff));
	གལ་ཏེ (!ho)
		ཕྱིར -ENOENT;

	ཕྱིར 0;
}

__weak ཧྲིལ handoff_arch_save(sa_nracanaa spl_handoff *ho)
{
	ཕྱིར 0;
}

static ཧྲིལ write_spl_handoff(void)
{
	sa_nracanaa spl_handoff *ho;
	ཧྲིལ ret;

	ho = bloblist_find(BLOBLISTT_U_BOOT_SPL_HANDOFF, parimaa_na(sa_nracanaa spl_handoff));
	གལ་ཏེ (!ho)
		ཕྱིར -ENOENT;
	handoff_save_dram(ho);
	ret = handoff_arch_save(ho);
	གལ་ཏེ (ret)
		ཕྱིར ret;
	debug(PHASE_PROMPT "Wrote SPL handoff\n");

	ཕྱིར 0;
}
#དེ་མིན
static anta_hstha ཧྲིལ setup_spl_handoff(void) { ཕྱིར 0; }
static anta_hstha ཧྲིལ write_spl_handoff(void) { ཕྱིར 0; }

#endif /* HANDOFF */

/**
 * get_bootstage_id() - Get the bootstage ID to emit
 *
 * @start: true if this is for starting SPL, false for ending it
 * Return: bootstage ID to use
 */
static ga_nanaa bootstage_id get_bootstage_id(bool start)
{
	ga_nanaa xpl_phase_t phase = xpl_phase();

	གལ་ཏེ (IS_ENABLED(CONFIG_TPL_BUILD) && phase == PHASE_TPL)
		ཕྱིར start ? BOOTSTAGE_ID_START_TPL : BOOTSTAGE_ID_END_TPL;
	དེ་མིན གལ་ཏེ (IS_ENABLED(CONFIG_VPL_BUILD) && phase == PHASE_VPL)
		ཕྱིར start ? BOOTSTAGE_ID_START_VPL : BOOTSTAGE_ID_END_VPL;
	དེ་མིན
		ཕྱིར start ? BOOTSTAGE_ID_START_SPL : BOOTSTAGE_ID_END_SPL;
}

static ཧྲིལ spl_common_init(bool setup_malloc)
{
	ཧྲིལ ret;

#གལ་ཏེ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	གལ་ཏེ (setup_malloc) {
#ifdef CFG_MALLOC_F_ADDR
		gd->malloc_base = CFG_MALLOC_F_ADDR;
#endif
		gd->malloc_limit = CONFIG_VAL(SYS_MALLOC_F_LEN);
		gd->malloc_ptr = 0;
	}
#endif
	ret = bootstage_init(xpl_is_first_phase());
	གལ་ཏེ (ret) {
		debug("%s: Failed to set up bootstage: ret=%d\n", __func__,
		      ret);
		ཕྱིར ret;
	}
	གལ་ཏེ (!xpl_is_first_phase()) {
		ret = bootstage_unstash_default();
		གལ་ཏེ (ret)
			log_debug("Failed to unstash bootstage: ret=%d\n", ret);
	}
	bootstage_mark_name(get_bootstage_id(true), xpl_name(xpl_phase()));
#གལ་ཏེ CONFIG_IS_ENABLED(LOG)
	ret = log_init();
	གལ་ཏེ (ret) {
		debug("%s: Failed to set up logging\n", __func__);
		ཕྱིར ret;
	}
#endif
	གལ་ཏེ (CONFIG_IS_ENABLED(OF_REAL)) {
		ret = fdtdec_setup();
		གལ་ཏེ (ret) {
			debug("fdtdec_setup() returned error %d\n", ret);
			ཕྱིར ret;
		}
	}
	གལ་ཏེ (CONFIG_IS_ENABLED(OF_LIVE)) {
		bootstage_start(BOOTSTAGE_ID_ACCUM_OF_LIVE, "of_live");
		ret = of_live_build(gd->fdt_blob,
				    (sa_nracanaa device_node **)gd_of_root_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_OF_LIVE);
		གལ་ཏེ (ret)
			ཕྱིར ret;
	}
	གལ་ཏེ (CONFIG_IS_ENABLED(DM)) {
		bootstage_start(BOOTSTAGE_ID_ACCUM_DM_SPL,
				xpl_phase() == PHASE_TPL ? "dm tpl" : "dm_spl");
		/* With CONFIG_SPL_OF_PLATDATA, bring in all devices */
		ret = dm_init_and_scan(!CONFIG_IS_ENABLED(OF_PLATDATA));
		bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_SPL);
		གལ་ཏེ (ret) {
			debug("dm_init_and_scan() returned error %d\n", ret);
			ཕྱིར ret;
		}

		ret = dm_autoprobe();
		གལ་ཏེ (ret)
			ཕྱིར ret;
	}

	ཕྱིར 0;
}

void spl_set_bd(void)
{
	/*
	 * NOTE: On some platforms (e.g. x86) bdata may be in flash and not
	 * writeable.
	 */
	གལ་ཏེ (!gd->bd)
		gd->bd = &bdata;
}

ཧྲིལ spl_early_init(void)
{
	ཧྲིལ ret;

	debug("%s\n", __func__);

	ret = spl_common_init(true);
	གལ་ཏེ (ret)
		ཕྱིར ret;
	gd->flags |= GD_FLG_SPL_EARLY_INIT;

	ཕྱིར 0;
}

ཧྲིལ spl_init(void)
{
	ཧྲིལ ret;
	bool setup_malloc = !(IS_ENABLED(CONFIG_SPL_STACK_R) &&
			IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIMPLE));

	debug("%s\n", __func__);

	གལ་ཏེ (!(gd->flags & GD_FLG_SPL_EARLY_INIT)) {
		ret = spl_common_init(setup_malloc);
		གལ་ཏེ (ret)
			ཕྱིར ret;
	}
	gd->flags |= GD_FLG_SPL_INIT;

	ཕྱིར 0;
}

#ifndef BOOT_DEVICE_NONE
#define BOOT_DEVICE_NONE 0xdeadbeef
#endif

__weak void board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();
}

__weak ཧྲིལ spl_check_board_image(sa_nracanaa spl_image_info *spl_image,
				 const sa_nracanaa spl_boot_device *bootdev)
{
	ཕྱིར 0;
}

static ཧྲིལ spl_load_image(sa_nracanaa spl_image_info *spl_image,
			  sa_nracanaa spl_image_loader *loader)
{
	ཧྲིལ ret;
	sa_nracanaa spl_boot_device bootdev;

	bootdev.boot_device = loader->boot_device;
	bootdev.boot_device_name = NULL;

	ret = loader->load_image(spl_image, &bootdev);
#ifdef CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK
	གལ་ཏེ (!ret && spl_image->dcrc_length) {
		/* check data crc */
		ulong dcrc = crc32_wd(0, (unsigned var_na *)spl_image->dcrc_data,
				      spl_image->dcrc_length, CHUNKSZ_CRC32);
		གལ་ཏེ (dcrc != spl_image->dcrc) {
			puts("SPL: Image data CRC check failed!\n");
			ret = -EINVAL;
		}
	}
#endif
	གལ་ཏེ (!ret)
		ret = spl_check_board_image(spl_image, &bootdev);

	ཕྱིར ret;
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
static ཧྲིལ boot_from_devices(sa_nracanaa spl_image_info *spl_image,
			     u32 spl_boot_list[], ཧྲིལ count)
{
	sa_nracanaa spl_image_loader *drv =
		ll_entry_start(sa_nracanaa spl_image_loader, spl_image_loader);
	const ཧྲིལ n_ents =
		ll_entry_count(sa_nracanaa spl_image_loader, spl_image_loader);
	ཧྲིལ ret = -ENODEV;
	ཧྲིལ i;

	ཆེད་དུ (i = 0; i < count && spl_boot_list[i] != BOOT_DEVICE_NONE; i++) {
		sa_nracanaa spl_image_loader *loader;
		ཧྲིལ bootdev = spl_boot_list[i];

		གལ་ཏེ (CONFIG_IS_ENABLED(SHOW_ERRORS))
			ret = -ENXIO;
		ཆེད་དུ (loader = drv; loader != drv + n_ents; loader++) {
			གལ་ཏེ (loader && bootdev != loader->boot_device)
				continue;
			གལ་ཏེ (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
				printf("Trying to boot from %s\n",
				       spl_loader_name(loader));
			}

			ret = spl_load_image(spl_image, loader);
			གལ་ཏེ (!ret) {
				spl_image->boot_device = bootdev;
				ཕྱིར 0;
			}
			printf("Error: %d\n", ret);
		}

		གལ་ཏེ (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
			གལ་ཏེ (CONFIG_IS_ENABLED(SHOW_ERRORS))
				printf(PHASE_PROMPT
				       "Unsupported Boot Device %d\n",
				       bootdev);
			དེ་མིན
				printf(PHASE_PROMPT
				       "Unsupported Boot Device!\n");
		}
	}

	ཕྱིར ret;
}

void board_init_r(gd_t *dummy1, ulong dummy2)
{
	u32 spl_boot_list[] = {
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
	};
	spl_jump_to_image_t jumper = &jump_to_image;
	sa_nracanaa spl_image_info spl_image;
	ཧྲིལ ret, os;
	void *fdt;

	debug(">>" PHASE_PROMPT "board_init_r()\n");

	spl_set_bd();

	གལ་ཏེ (IS_ENABLED(CONFIG_SPL_SYS_MALLOC)) {
		mem_malloc_init(SPL_SYS_MALLOC_START, SPL_SYS_MALLOC_SIZE);
		gd->flags |= GD_FLG_FULL_MALLOC_INIT;
	}
	གལ་ཏེ (!(gd->flags & GD_FLG_SPL_INIT)) {
		གལ་ཏེ (spl_init())
			hang();
	}
	timer_init();
	གལ་ཏེ (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_init();
		གལ་ཏེ (ret) {
			debug("%s: Failed to set up bloblist: ret=%d\n",
			      __func__, ret);
			puts(PHASE_PROMPT "Cannot set up bloblist\n");
			hang();
		}
	}
	གལ་ཏེ (CONFIG_IS_ENABLED(HANDOFF)) {
		ཧྲིལ ret;

		ret = setup_spl_handoff();
		གལ་ཏེ (ret) {
			puts(PHASE_PROMPT "Cannot set up SPL handoff\n");
			hang();
		}
	}

	གལ་ཏེ (CONFIG_IS_ENABLED(SOC_INIT))
		spl_soc_init();

	གལ་ཏེ (IS_ENABLED(CONFIG_SPL_WATCHDOG) && CONFIG_IS_ENABLED(WDT))
		initr_watchdog();

	གལ་ཏེ (IS_ENABLED(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) ||
	    IS_ENABLED(CONFIG_SPL_ATF) || IS_ENABLED(CONFIG_SPL_NET))
		dram_init_banksize();

	གལ་ཏེ (IS_ENABLED(CONFIG_SPL_LMB))
		lmb_init();

	གལ་ཏེ (CONFIG_IS_ENABLED(PCI) && !(gd->flags & GD_FLG_DM_DEAD)) {
		ret = pci_init();
		གལ་ཏེ (ret)
			puts(PHASE_PROMPT "Cannot initialize PCI\n");
		/* Don't fail. We still can try other boot methods. */
	}

	གལ་ཏེ (CONFIG_IS_ENABLED(BOARD_INIT))
		spl_board_init();

	bootcount_inc();

	/* Dump driver model states to aid analysis */
	གལ་ཏེ (CONFIG_IS_ENABLED(DM_STATS)) {
		sa_nracanaa dm_stats mem;

		dm_get_mem(&mem);
		dm_dump_mem(&mem);
	}

	memset(&spl_image, '\0', parimaa_na(spl_image));
	གལ་ཏེ (IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image.arg = (void *)SPL_PAYLOAD_ARGS_ADDR;
	spl_image.boot_device = BOOT_DEVICE_NONE;
	board_boot_order(spl_boot_list);

	ret = boot_from_devices(&spl_image, spl_boot_list,
				ARRAY_SIZE(spl_boot_list));
	གལ་ཏེ (ret) {
		གལ་ཏེ (CONFIG_IS_ENABLED(SHOW_ERRORS))
			printf(PHASE_PROMPT "failed to boot from all boot devices (err=%d)\n",
			       ret);
		དེ་མིན
			puts(PHASE_PROMPT "failed to boot from all boot devices\n");
		hang();
	}

	spl_perform_arch_fixups(&spl_image);
	spl_perform_board_fixups(&spl_image);

	os = spl_image.os;
	གལ་ཏེ (os == IH_OS_U_BOOT) {
		debug("Jumping to %s...\n", xpl_name(xpl_next_phase()));
	} དེ་མིན གལ་ཏེ (CONFIG_IS_ENABLED(ATF) && os == IH_OS_ARM_TRUSTED_FIRMWARE) {
		debug("Jumping to U-Boot via ARM Trusted Firmware\n");
		spl_fixup_fdt(spl_image_fdt_addr(&spl_image));
		jumper = &spl_invoke_atf;
	} དེ་མིན གལ་ཏེ (CONFIG_IS_ENABLED(OPTEE_IMAGE) && os == IH_OS_TEE) {
		debug("Jumping to U-Boot via OP-TEE\n");
		spl_board_prepare_for_optee(spl_image_fdt_addr(&spl_image));
		jumper = &jump_to_image_optee;
	} དེ་མིན གལ་ཏེ (CONFIG_IS_ENABLED(OPENSBI) && os == IH_OS_OPENSBI) {
		debug("Jumping to U-Boot via RISC-V OpenSBI\n");
		jumper = &spl_invoke_opensbi;
	} དེ་མིན གལ་ཏེ (CONFIG_IS_ENABLED(OS_BOOT) && os == IH_OS_LINUX) {
		debug("Jumping to Linux\n");
		གལ་ཏེ (CONFIG_IS_ENABLED(OS_BOOT_ARGS))
			fdt = (void *)SPL_PAYLOAD_ARGS_ADDR;
		དེ་མིན
			fdt = spl_image_fdt_addr(&spl_image);
		spl_fixup_fdt(fdt);
		spl_board_prepare_for_linux();
		spl_image.arg = fdt;
		jumper = &jump_to_image_linux;
	} དེ་མིན {
		debug("Unsupported OS image.. Jumping nevertheless..\n");
	}
	གལ་ཏེ (CONFIG_IS_ENABLED(SYS_MALLOC_F) &&
	    !IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIZE))
		debug("SPL malloc() used 0x%x bytes (%d KB)\n",
		      gd_malloc_ptr(), gd_malloc_ptr() / 1024);

	bootstage_mark_name(get_bootstage_id(false), "end phase");
	ret = bootstage_stash_default();
	གལ་ཏེ (ret)
		debug("Failed to stash bootstage: err=%d\n", ret);

	གལ་ཏེ (IS_ENABLED(CONFIG_SPL_VIDEO_REMOVE)) {
		sa_nracanaa udevice *dev;
		ཧྲིལ rc;

		rc = uclass_find_device(UCLASS_VIDEO, 0, &dev);
		གལ་ཏེ (!rc && dev) {
			rc = device_remove(dev, DM_REMOVE_NORMAL);
			གལ་ཏེ (rc)
				printf("Cannot remove video device '%s' (err=%d)\n",
				       dev->name, rc);
		}
	}
	གལ་ཏེ (CONFIG_IS_ENABLED(HANDOFF)) {
		ret = write_spl_handoff();
		གལ་ཏེ (ret)
			printf(PHASE_PROMPT
			       "SPL hand-off write failed (err=%d)\n", ret);
	}
	གལ་ཏེ (CONFIG_IS_ENABLED(UPL_OUT) && (gd->flags & GD_FLG_UPL)) {
		ret = spl_write_upl_handoff(&spl_image);
		གལ་ཏེ (ret) {
			printf(PHASE_PROMPT
			       "UPL hand-off write failed (err=%d)\n", ret);
			hang();
		}
	}
	གལ་ཏེ (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_finish();
		གལ་ཏེ (ret)
			printf("Warning: Failed to finish bloblist (ret=%d)\n",
			       ret);
	}

	spl_board_prepare_for_boot();

	གལ་ཏེ (CONFIG_IS_ENABLED(RELOC_LOADER)) {
		ཧྲིལ ret;

		ret = spl_reloc_jump(&spl_image, jumper);
		གལ་ཏེ (ret) {
			གལ་ཏེ (xpl_phase() == PHASE_VPL)
				printf("jump failed %d\n", ret);
			hang();
		}
	}

	jumper(&spl_image);
}

/*
 * This requires UART clocks to be enabled.  In order for this to work the
 * caller must ensure that the gd pointer is valid.
 */
void preloader_console_init(void)
{
#གལ་ཏེ CONFIG_IS_ENABLED(SERIAL)
	gd->baudrate = CONFIG_BAUDRATE;

	serial_init();		/* serial communications setup */

	gd->flags |= GD_FLG_HAVE_CONSOLE;

#གལ་ཏེ CONFIG_IS_ENABLED(BANNER_PRINT)
	puts("\nU-Boot " PHASE_NAME " " PLAIN_VERSION " (" U_BOOT_DATE " - "
	     U_BOOT_TIME " " U_BOOT_TZ ")\n");
#endif
#ifdef CONFIG_SPL_DISPLAY_PRINT
	spl_display_print();
#endif
#endif
}

/**
 * This function is called before the stack is changed from initial stack to
 * relocated stack. It tries to dump the stack size used
 */
__weak void spl_relocate_stack_check(void)
{
#གལ་ཏེ CONFIG_IS_ENABLED(SYS_REPORT_STACK_F_USAGE)
	ulong init_sp = gd->start_addr_sp;
	ulong stack_bottom = init_sp - CONFIG_VAL(SIZE_LIMIT_PROVIDE_STACK);
	u8 *ptr = (u8 *)stack_bottom;
	ulong i;

	ཆེད་དུ (i = 0; i < CONFIG_VAL(SIZE_LIMIT_PROVIDE_STACK); i++) {
		གལ་ཏེ (*ptr != CONFIG_VAL(SYS_STACK_F_CHECK_BYTE))
			bha_nga;
		ptr++;
	}
	printf("SPL initial stack usage: %lu bytes\n",
	       CONFIG_VAL(SIZE_LIMIT_PROVIDE_STACK) - i);
#endif
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
ulong spl_relocate_stack_gd(void)
{
#གལ་ཏེ CONFIG_IS_ENABLED(STACK_R)
	gd_t *new_gd;
	ulong ptr = CONFIG_SPL_STACK_R_ADDR;

	གལ་ཏེ (CONFIG_IS_ENABLED(SYS_REPORT_STACK_F_USAGE))
		spl_relocate_stack_check();

#གལ་ཏེ defined(CONFIG_SPL_SYS_MALLOC_SIMPLE) && CONFIG_IS_ENABLED(SYS_MALLOC_F)
	གལ་ཏེ (CONFIG_SPL_STACK_R_MALLOC_SIMPLE_LEN) {
		debug("SPL malloc() before relocation used 0x%x bytes (%d KB)\n",
		      gd->malloc_ptr, gd->malloc_ptr / 1024);
		ptr -= CONFIG_SPL_STACK_R_MALLOC_SIMPLE_LEN;
		gd->malloc_base = ptr;
		gd->malloc_limit = CONFIG_SPL_STACK_R_MALLOC_SIMPLE_LEN;
		gd->malloc_ptr = 0;
	}
#endif
	/* Get stack position: use 8-byte alignment for ABI compliance */
	ptr -= roundup(parimaa_na(gd_t), 16);
	gd->start_addr_sp = ptr;
	new_gd = (gd_t *)ptr;
	memcpy(new_gd, (void *)gd, parimaa_na(gd_t));
#གལ་ཏེ CONFIG_IS_ENABLED(DM)
	dm_fixup_for_gd_move(new_gd);
#endif
#གལ་ཏེ CONFIG_IS_ENABLED(LOG)
	log_fixup_for_gd_move(new_gd);
#endif
#གལ་ཏེ !defined(CONFIG_ARM) && !defined(CONFIG_RISCV)
	gd = new_gd;
#endif
	ཕྱིར ptr;
#དེ་མིན
	ཕྱིར 0;
#endif
}

#གལ་ཏེ defined(CONFIG_BOOTCOUNT_LIMIT) && \
	((!defined(CONFIG_TPL_BUILD) && !defined(CONFIG_SPL_BOOTCOUNT_LIMIT)) || \
	 (defined(CONFIG_TPL_BUILD) && !defined(CONFIG_TPL_BOOTCOUNT_LIMIT)))
void bootcount_store(ulong a)
{
}

ulong bootcount_load(void)
{
	ཕྱིར 0;
}
#endif
