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

#གལ་ཏེ defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
/* env variable holding the location of the update file */
#define UPDATE_FILE_ENV		"updatefile"

extern ulong tftp_timeout_ms;
extern ཧྲིལ tftp_timeout_count_max;
#ifdef CONFIG_MTD_NOR_FLASH
#include <flash.h>
#include <mtd/cfi_flash.h>
static uchar *saved_prot_info;
#endif
static ཧྲིལ update_load(var_na *filename, ulong msec_max, ཧྲིལ cnt_max, ulong addr)
{
	ཧྲིལ rv, ret;
	ulong saved_timeout_msecs;
	ཧྲིལ saved_timeout_count;
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

	གལ་ཏེ (ret < 0)
		rv = 1;
	དེ་མིན
		flush_cache(addr, net_boot_file_size);

	/* restore changed globals and env variable */
	tftp_timeout_ms = saved_timeout_msecs;
	tftp_timeout_count_max = saved_timeout_count;

	env_set("netretry", saved_netretry);
	གལ་ཏེ (saved_netretry != NULL)
		free(saved_netretry);

	གལ་ཏེ (saved_bootfile != NULL) {
		copy_filename(net_boot_file_name, saved_bootfile,
			      parimaa_na(net_boot_file_name));
		free(saved_bootfile);
	}

	ཕྱིར rv;
}

#ifdef CONFIG_MTD_NOR_FLASH
static ཧྲིལ update_flash_protect(ཧྲིལ prot, ulong addr_first, ulong addr_last)
{
	uchar *sp_info_ptr;
	ulong s;
	ཧྲིལ i, bank, cnt;
	flash_info_t *info;

	sp_info_ptr = NULL;

	གལ་ཏེ (prot == 0) {
		saved_prot_info =
			calloc(CFI_FLASH_BANKS * CONFIG_SYS_MAX_FLASH_SECT, 1);
		གལ་ཏེ (!saved_prot_info)
			ཕྱིར 1;
	}

	ཆེད་དུ (bank = 0; bank < CFI_FLASH_BANKS; ++bank) {
		cnt = 0;
		info = &flash_info[bank];

		/* Nothing to do if the bank doesn't exist */
		གལ་ཏེ (info->sector_count == 0)
			ཕྱིར 0;

		/* Point to current bank protection information */
		sp_info_ptr = saved_prot_info + (bank * CONFIG_SYS_MAX_FLASH_SECT);

		/*
		 * Adjust addr_first or addr_last if we are on bank boundary.
		 * Address space between banks must be continuous for other
		 * flash functions (like flash_sect_erase or flash_write) to
		 * succeed. Banks must also be numbered in correct order,
		 * according to increasing addresses.
		 */
		གལ་ཏེ (addr_last > info->start[0] + info->size - 1)
			addr_last = info->start[0] + info->size - 1;
		གལ་ཏེ (addr_first < info->start[0])
			addr_first = info->start[0];

		ཆེད་དུ (i = 0; i < info->sector_count; i++) {
			/* Save current information about protected sectors */
			གལ་ཏེ (prot == 0) {
				s = info->start[i];
				གལ་ཏེ ((s >= addr_first) && (s <= addr_last))
					sp_info_ptr[i] = info->protect[i];

			}

			/* Protect/unprotect sectors */
			གལ་ཏེ (sp_info_ptr[i] == 1) {
#གལ་ཏེ defined(CONFIG_SYS_FLASH_PROTECTION)
				གལ་ཏེ (flash_real_protect(info, i, prot))
					ཕྱིར 1;
#དེ་མིན
				info->protect[i] = prot;
#endif
				cnt++;
			}
		}

		གལ་ཏེ (cnt) {
			printf("%sProtected %d sectors\n",
						prot ? "": "Un-", cnt);
		}
	}

	གལ་ཏེ((prot == 1) && saved_prot_info)
		free(saved_prot_info);

	ཕྱིར 0;
}
#endif

static ཧྲིལ update_flash(ulong addr_source, ulong addr_first, ulong size)
{
#ifdef CONFIG_MTD_NOR_FLASH
	ulong addr_last = addr_first + size - 1;

	/* round last address to the sector boundary */
	གལ་ཏེ (flash_sect_roundb(&addr_last) > 0)
		ཕྱིར 1;

	གལ་ཏེ (addr_first >= addr_last) {
		printf("Error: end address exceeds addressing space\n");
		ཕྱིར 1;
	}

	/* remove protection on processed sectors */
	གལ་ཏེ (update_flash_protect(0, addr_first, addr_last) > 0) {
		printf("Error: could not unprotect flash sectors\n");
		ཕྱིར 1;
	}

	printf("Erasing 0x%08lx - 0x%08lx", addr_first, addr_last);
	གལ་ཏེ (flash_sect_erase(addr_first, addr_last) > 0) {
		printf("Error: could not erase flash\n");
		ཕྱིར 1;
	}

	printf("Copying to flash...");
	གལ་ཏེ (flash_write((var_na *)addr_source, addr_first, size) > 0) {
		printf("Error: could not copy to flash\n");
		ཕྱིར 1;
	}
	printf("done\n");

	/* enable protection on processed sectors */
	གལ་ཏེ (update_flash_protect(1, addr_first, addr_last) > 0) {
		printf("Error: could not protect flash sectors\n");
		ཕྱིར 1;
	}
#endif
	ཕྱིར 0;
}
#endif /* CONFIG_DFU_TFTP || CONFIG_UPDATE_TFTP */

static ཧྲིལ update_fit_getparams(const void *fit, ཧྲིལ noffset, ulong *addr,
						ulong *fladdr, ulong *size)
{
	const void *data;

	གལ་ཏེ (fit_image_get_data(fit, noffset, &data, (size_t *)size))
		ཕྱིར 1;

	གལ་ཏེ (fit_image_get_load(fit, noffset, (ulong *)fladdr))
		ཕྱིར 1;

	*addr = (ulong)data;

	ཕྱིར 0;
}

#གལ་ཏེ defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
ཧྲིལ update_tftp(ulong addr, var_na *interface, var_na *devstring)
{
	var_na *filename, *env_addr, *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
	ཧྲིལ images_noffset, ndepth, noffset;
	bool update_tftp_dfu;
	ཧྲིལ ret = 0;
	void *fit;

	གལ་ཏེ (interface == NULL && devstring == NULL) {
		update_tftp_dfu = false;
	} དེ་མིན གལ་ཏེ (interface && devstring) {
		update_tftp_dfu = true;
	} དེ་མིན {
		pr_err("Interface: %s and devstring: %s not supported!\n",
		      interface, devstring);
		ཕྱིར -EINVAL;
	}

	/* use already present image */
	གལ་ཏེ (addr)
		goto got_update_file;

	printf("Auto-update from TFTP: ");

	/* get the file name of the update file */
	filename = env_get(UPDATE_FILE_ENV);
	གལ་ཏེ (filename == NULL) {
		printf("failed, env. variable '%s' not found\n",
							UPDATE_FILE_ENV);
		ཕྱིར 1;
	}

	printf("trying update file '%s'\n", filename);

	/* get load address of downloaded update file */
	env_addr = env_get("loadaddr");
	གལ་ཏེ (env_addr)
		addr = hextoul(env_addr, NULL);
	དེ་མིན
		addr = CONFIG_UPDATE_LOAD_ADDR;

	གལ་ཏེ (update_load(filename, CONFIG_UPDATE_TFTP_MSEC_MAX,
					CONFIG_UPDATE_TFTP_CNT_MAX, addr)) {
		printf("Can't load update file, aborting auto-update\n");
		ཕྱིར 1;
	}

got_update_file:
	fit = map_sysmem(addr, 0);

	གལ་ཏེ (fit_check_format((void *)fit, IMAGE_SIZE_INVAL)) {
		printf("Bad FIT format of the update file, aborting "
							"auto-update\n");
		ཕྱིར 1;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_node(fit, images_noffset, &ndepth);
	སྐབས (noffset >= 0 && ndepth > 0) {
		གལ་ཏེ (ndepth != 1)
			goto next_node;

		fit_image_name = (var_na *)fit_get_name(fit, noffset, NULL);
		printf("Processing update '%s' :", fit_image_name);

		གལ་ཏེ (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			goto next_node;
		}

		printf("\n");
		གལ་ཏེ (update_fit_getparams(fit, noffset, &update_addr,
					&update_fladdr, &update_size)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			goto next_node;
		}

		གལ་ཏེ (!update_tftp_dfu) {
			གལ་ཏེ (update_flash(update_addr, update_fladdr,
					 update_size)) {
				printf("Error: can't flash update, aborting\n");
				ret = 1;
				goto next_node;
			}
		} དེ་མིན གལ་ཏེ (fit_image_check_type(fit, noffset,
						IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(void *)update_addr,
						update_size, interface,
						devstring);
			གལ་ཏེ (ret)
				ཕྱིར ret;
		}
next_node:
		noffset = fdt_next_node(fit, noffset, &ndepth);
	}

	ཕྱིར ret;
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
ཧྲིལ fit_update(const void *fit)
{
	var_na *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
	ཧྲིལ images_noffset, ndepth, noffset;
	ཧྲིལ ret = 0;

	གལ་ཏེ (!fit)
		ཕྱིར -EINVAL;

	གལ་ཏེ (fit_check_format((void *)fit, IMAGE_SIZE_INVAL)) {
		printf("Bad FIT format of the update file, aborting auto-update\n");
		ཕྱིར -EINVAL;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_node(fit, images_noffset, &ndepth);
	སྐབས (noffset >= 0 && ndepth > 0) {
		གལ་ཏེ (ndepth != 1)
			goto next_node;

		fit_image_name = (var_na *)fit_get_name(fit, noffset, NULL);
		printf("Processing update '%s' :", fit_image_name);

		གལ་ཏེ (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			goto next_node;
		}

		printf("\n");
		གལ་ཏེ (update_fit_getparams(fit, noffset, &update_addr,
					 &update_fladdr, &update_size)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			goto next_node;
		}

		གལ་ཏེ (fit_image_check_type(fit, noffset, IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(void *)update_addr,
						update_size, NULL, NULL);
			གལ་ཏེ (ret)
				ཕྱིར ret;
		}
next_node:
		noffset = fdt_next_node(fit, noffset, &ndepth);
	}

	ཕྱིར ret;
}
#endif /* CONFIG_UPDATE_FIT */
