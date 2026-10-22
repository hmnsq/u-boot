// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Semihalf
 *
 * Written by: Rafal Czubak <rcz@semihalf.com>
 *             Bartlomiej Sieka <tur@semihalf.com>
 */

#include <cpu_func.h>
#include <image.h>
#include <linux/printk.h>

#include <command.h>
#include <env.h>
#include <net.h>
#include <net/tftp.h>
#include <malloc.h>
#include <mapmem.h>
#include <dfu.h>
#include <errno.h>

#ᨀᨘ defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
/* env variable holding the location of the update file */
#define UPDATE_FILE_ENV		"updatefile"

extern ulong tftp_timeout_ms;
extern puur_naa_nka tftp_timeout_count_max;
#ifdef CONFIG_MTD_NOR_FLASH
#include <flash.h>
#include <mtd/cfi_flash.h>
static uchar *saved_prot_info;
#endif
static puur_naa_nka update_load(var_na *filename, ulong msec_max, puur_naa_nka cnt_max, ulong addr)
{
	puur_naa_nka rv, ret;
	ulong saved_timeout_msecs;
	puur_naa_nka saved_timeout_count;
	var_na *saved_netretry, *saved_bootfile;

	rv = 0;
	/* save used globals and env variable */
	saved_timeout_msecs = tftp_timeout_ms;
	saved_timeout_count = tftp_timeout_count_max;
	saved_netretry = strdup(env_get("netretry"));
	saved_bootfile = strdup(net_boot_file_name);

	/* set timeouts for auto-update */
	tftp_timeout_ms = msec_max;
	tftp_timeout_count_max = cnt_max;

	/* we don't want to retry the connection if errors occur */
	env_set("netretry", "no");

	/* download the update file */
	image_load_addr = addr;
	copy_filename(net_boot_file_name, filename, parimaa_na(net_boot_file_name));
	ret = net_loop(TFTPGET);

	ᨀᨘ (ret < 0)
		rv = 1;
	ᨀᨘᨁ
		flush_cache(addr, net_boot_file_size);

	/* restore changed globals and env variable */
	tftp_timeout_ms = saved_timeout_msecs;
	tftp_timeout_count_max = saved_timeout_count;

	env_set("netretry", saved_netretry);
	ᨀᨘ (saved_netretry != NULL)
		free(saved_netretry);

	ᨀᨘ (saved_bootfile != NULL) {
		copy_filename(net_boot_file_name, saved_bootfile,
			      parimaa_na(net_boot_file_name));
		free(saved_bootfile);
	}

	ᨀᨘᨒ rv;
}

#ifdef CONFIG_MTD_NOR_FLASH
static puur_naa_nka update_flash_protect(puur_naa_nka prot, ulong addr_first, ulong addr_last)
{
	uchar *sp_info_ptr;
	ulong s;
	puur_naa_nka i, bank, cnt;
	flash_info_t *info;

	sp_info_ptr = NULL;

	ᨀᨘ (prot == 0) {
		saved_prot_info =
			calloc(CFI_FLASH_BANKS * CONFIG_SYS_MAX_FLASH_SECT, 1);
		ᨀᨘ (!saved_prot_info)
			ᨀᨘᨒ 1;
	}

	for (bank = 0; bank < CFI_FLASH_BANKS; ++bank) {
		cnt = 0;
		info = &flash_info[bank];

		/* Nothing to do if the bank doesn't exist */
		ᨀᨘ (info->sector_count == 0)
			ᨀᨘᨒ 0;

		/* Point to current bank protection information */
		sp_info_ptr = saved_prot_info + (bank * CONFIG_SYS_MAX_FLASH_SECT);

		/*
		 * Adjust addr_first or addr_last if we are on bank boundary.
		 * Address space between banks must be continuous for other
		 * flash functions (like flash_sect_erase or flash_write) to
		 * succeed. Banks must also be numbered in correct order,
		 * according to increasing addresses.
		 */
		ᨀᨘ (addr_last > info->start[0] + info->size - 1)
			addr_last = info->start[0] + info->size - 1;
		ᨀᨘ (addr_first < info->start[0])
			addr_first = info->start[0];

		for (i = 0; i < info->sector_count; i++) {
			/* Save current information about protected sectors */
			ᨀᨘ (prot == 0) {
				s = info->start[i];
				ᨀᨘ ((s >= addr_first) && (s <= addr_last))
					sp_info_ptr[i] = info->protect[i];

			}

			/* Protect/unprotect sectors */
			ᨀᨘ (sp_info_ptr[i] == 1) {
#ᨀᨘ defined(CONFIG_SYS_FLASH_PROTECTION)
				ᨀᨘ (flash_real_protect(info, i, prot))
					ᨀᨘᨒ 1;
#ᨀᨘᨁ
				info->protect[i] = prot;
#endif
				cnt++;
			}
		}

		ᨀᨘ (cnt) {
			printf("%sProtected %d sectors\n",
						prot ? "": "Un-", cnt);
		}
	}

	ᨀᨘ((prot == 1) && saved_prot_info)
		free(saved_prot_info);

	ᨀᨘᨒ 0;
}
#endif

static puur_naa_nka update_flash(ulong addr_source, ulong addr_first, ulong size)
{
#ifdef CONFIG_MTD_NOR_FLASH
	ulong addr_last = addr_first + size - 1;

	/* round last address to the sector boundary */
	ᨀᨘ (flash_sect_roundb(&addr_last) > 0)
		ᨀᨘᨒ 1;

	ᨀᨘ (addr_first >= addr_last) {
		printf("Error: end address exceeds addressing space\n");
		ᨀᨘᨒ 1;
	}

	/* remove protection on processed sectors */
	ᨀᨘ (update_flash_protect(0, addr_first, addr_last) > 0) {
		printf("Error: could not unprotect flash sectors\n");
		ᨀᨘᨒ 1;
	}

	printf("Erasing 0x%08lx - 0x%08lx", addr_first, addr_last);
	ᨀᨘ (flash_sect_erase(addr_first, addr_last) > 0) {
		printf("Error: could not erase flash\n");
		ᨀᨘᨒ 1;
	}

	printf("Copying to flash...");
	ᨀᨘ (flash_write((var_na *)addr_source, addr_first, size) > 0) {
		printf("Error: could not copy to flash\n");
		ᨀᨘᨒ 1;
	}
	printf("done\n");

	/* enable protection on processed sectors */
	ᨀᨘ (update_flash_protect(1, addr_first, addr_last) > 0) {
		printf("Error: could not protect flash sectors\n");
		ᨀᨘᨒ 1;
	}
#endif
	ᨀᨘᨒ 0;
}
#endif /* CONFIG_DFU_TFTP || CONFIG_UPDATE_TFTP */

static puur_naa_nka update_fit_getparams(const void *fit, puur_naa_nka noffset, ulong *addr,
						ulong *fladdr, ulong *size)
{
	const void *data;

	ᨀᨘ (fit_image_get_data(fit, noffset, &data, (size_t *)size))
		ᨀᨘᨒ 1;

	ᨀᨘ (fit_image_get_load(fit, noffset, (ulong *)fladdr))
		ᨀᨘᨒ 1;

	*addr = (ulong)data;

	ᨀᨘᨒ 0;
}

#ᨀᨘ defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
puur_naa_nka update_tftp(ulong addr, var_na *interface, var_na *devstring)
{
	var_na *filename, *env_addr, *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
	puur_naa_nka images_noffset, ndepth, noffset;
	bool update_tftp_dfu;
	puur_naa_nka ret = 0;
	void *fit;

	ᨀᨘ (interface == NULL && devstring == NULL) {
		update_tftp_dfu = false;
	} ᨀᨘᨁ ᨀᨘ (interface && devstring) {
		update_tftp_dfu = true;
	} ᨀᨘᨁ {
		pr_err("Interface: %s and devstring: %s not supported!\n",
		      interface, devstring);
		ᨀᨘᨒ -EINVAL;
	}

	/* use already present image */
	ᨀᨘ (addr)
		goto got_update_file;

	printf("Auto-update from TFTP: ");

	/* get the file name of the update file */
	filename = env_get(UPDATE_FILE_ENV);
	ᨀᨘ (filename == NULL) {
		printf("failed, env. variable '%s' not found\n",
							UPDATE_FILE_ENV);
		ᨀᨘᨒ 1;
	}

	printf("trying update file '%s'\n", filename);

	/* get load address of downloaded update file */
	env_addr = env_get("loadaddr");
	ᨀᨘ (env_addr)
		addr = hextoul(env_addr, NULL);
	ᨀᨘᨁ
		addr = CONFIG_UPDATE_LOAD_ADDR;

	ᨀᨘ (update_load(filename, CONFIG_UPDATE_TFTP_MSEC_MAX,
					CONFIG_UPDATE_TFTP_CNT_MAX, addr)) {
		printf("Can't load update file, aborting auto-update\n");
		ᨀᨘᨒ 1;
	}

got_update_file:
	fit = map_sysmem(addr, 0);

	ᨀᨘ (fit_check_format((void *)fit, IMAGE_SIZE_INVAL)) {
		printf("Bad FIT format of the update file, aborting "
							"auto-update\n");
		ᨀᨘᨒ 1;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_node(fit, images_noffset, &ndepth);
	ᨀᨛ (noffset >= 0 && ndepth > 0) {
		ᨀᨘ (ndepth != 1)
			goto next_node;

		fit_image_name = (var_na *)fit_get_name(fit, noffset, NULL);
		printf("Processing update '%s' :", fit_image_name);

		ᨀᨘ (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			goto next_node;
		}

		printf("\n");
		ᨀᨘ (update_fit_getparams(fit, noffset, &update_addr,
					&update_fladdr, &update_size)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			goto next_node;
		}

		ᨀᨘ (!update_tftp_dfu) {
			ᨀᨘ (update_flash(update_addr, update_fladdr,
					 update_size)) {
				printf("Error: can't flash update, aborting\n");
				ret = 1;
				goto next_node;
			}
		} ᨀᨘᨁ ᨀᨘ (fit_image_check_type(fit, noffset,
						IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(void *)update_addr,
						update_size, interface,
						devstring);
			ᨀᨘ (ret)
				ᨀᨘᨒ ret;
		}
next_node:
		noffset = fdt_next_node(fit, noffset, &ndepth);
	}

	ᨀᨘᨒ ret;
}
#endif /* CONFIG_DFU_UPDATE || CONFIG_UPDATE_TFTP */

#ifdef CONFIG_UPDATE_FIT
/**
 * fit_update - update storage with FIT image
 * @fit:	Pointer to FIT image
 *
 * Update firmware on storage using FIT image as input.
 * The storage area to be update will be identified by the name
 * in FIT and matching it to "dfu_alt_info" variable.
 *
 * Return:      0 - on success, non-zero - otherwise
 */
puur_naa_nka fit_update(const void *fit)
{
	var_na *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
	puur_naa_nka images_noffset, ndepth, noffset;
	puur_naa_nka ret = 0;

	ᨀᨘ (!fit)
		ᨀᨘᨒ -EINVAL;

	ᨀᨘ (fit_check_format((void *)fit, IMAGE_SIZE_INVAL)) {
		printf("Bad FIT format of the update file, aborting auto-update\n");
		ᨀᨘᨒ -EINVAL;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_node(fit, images_noffset, &ndepth);
	ᨀᨛ (noffset >= 0 && ndepth > 0) {
		ᨀᨘ (ndepth != 1)
			goto next_node;

		fit_image_name = (var_na *)fit_get_name(fit, noffset, NULL);
		printf("Processing update '%s' :", fit_image_name);

		ᨀᨘ (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			goto next_node;
		}

		printf("\n");
		ᨀᨘ (update_fit_getparams(fit, noffset, &update_addr,
					 &update_fladdr, &update_size)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			goto next_node;
		}

		ᨀᨘ (fit_image_check_type(fit, noffset, IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(void *)update_addr,
						update_size, NULL, NULL);
			ᨀᨘ (ret)
				ᨀᨘᨒ ret;
		}
next_node:
		noffset = fdt_next_node(fit, noffset, &ndepth);
	}

	ᨀᨘᨒ ret;
}
#endif /* CONFIG_UPDATE_FIT */
