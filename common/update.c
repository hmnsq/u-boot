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

#නම් defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
/* env variable holding the location of the update file */
#define UPDATE_FILE_ENV		"updatefile"

extern ulong tftp_timeout_ms;
extern පූර්ණ_සංඛ්‍යා tftp_timeout_count_max;
#ifdef CONFIG_MTD_NOR_FLASH
#include <flash.h>
#include <mtd/cfi_flash.h>
static uchar *saved_prot_info;
#endif
static පූර්ණ_සංඛ්‍යා update_load(var_na *filename, ulong msec_max, පූර්ණ_සංඛ්‍යා cnt_max, ulong addr)
{
	පූර්ණ_සංඛ්‍යා rv, ret;
	ulong saved_timeout_msecs;
	පූර්ණ_සංඛ්‍යා saved_timeout_count;
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

	නම් (ret < 0)
		rv = 1;
	නැතහොත්
		flush_cache(addr, net_boot_file_size);

	/* restore changed globals and env variable */
	tftp_timeout_ms = saved_timeout_msecs;
	tftp_timeout_count_max = saved_timeout_count;

	env_set("netretry", saved_netretry);
	නම් (saved_netretry != NULL)
		free(saved_netretry);

	නම් (saved_bootfile != NULL) {
		copy_filename(net_boot_file_name, saved_bootfile,
			      parimaa_na(net_boot_file_name));
		free(saved_bootfile);
	}

	ආපසු rv;
}

#ifdef CONFIG_MTD_NOR_FLASH
static පූර්ණ_සංඛ්‍යා update_flash_protect(පූර්ණ_සංඛ්‍යා prot, ulong addr_first, ulong addr_last)
{
	uchar *sp_info_ptr;
	ulong s;
	පූර්ණ_සංඛ්‍යා i, bank, cnt;
	flash_info_t *info;

	sp_info_ptr = NULL;

	නම් (prot == 0) {
		saved_prot_info =
			calloc(CFI_FLASH_BANKS * CONFIG_SYS_MAX_FLASH_SECT, 1);
		නම් (!saved_prot_info)
			ආපසු 1;
	}

	සඳහා (bank = 0; bank < CFI_FLASH_BANKS; ++bank) {
		cnt = 0;
		info = &flash_info[bank];

		/* Nothing to do if the bank doesn't exist */
		නම් (info->sector_count == 0)
			ආපසු 0;

		/* Point to current bank protection information */
		sp_info_ptr = saved_prot_info + (bank * CONFIG_SYS_MAX_FLASH_SECT);

		/*
		 * Adjust addr_first or addr_last if we are on bank boundary.
		 * Address space between banks must be continuous for other
		 * flash functions (like flash_sect_erase or flash_write) to
		 * succeed. Banks must also be numbered in correct order,
		 * according to increasing addresses.
		 */
		නම් (addr_last > info->start[0] + info->size - 1)
			addr_last = info->start[0] + info->size - 1;
		නම් (addr_first < info->start[0])
			addr_first = info->start[0];

		සඳහා (i = 0; i < info->sector_count; i++) {
			/* Save current information about protected sectors */
			නම් (prot == 0) {
				s = info->start[i];
				නම් ((s >= addr_first) && (s <= addr_last))
					sp_info_ptr[i] = info->protect[i];

			}

			/* Protect/unprotect sectors */
			නම් (sp_info_ptr[i] == 1) {
#නම් defined(CONFIG_SYS_FLASH_PROTECTION)
				නම් (flash_real_protect(info, i, prot))
					ආපසු 1;
#නැතහොත්
				info->protect[i] = prot;
#endif
				cnt++;
			}
		}

		නම් (cnt) {
			printf("%sProtected %d sectors\n",
						prot ? "": "Un-", cnt);
		}
	}

	නම්((prot == 1) && saved_prot_info)
		free(saved_prot_info);

	ආපසු 0;
}
#endif

static පූර්ණ_සංඛ්‍යා update_flash(ulong addr_source, ulong addr_first, ulong size)
{
#ifdef CONFIG_MTD_NOR_FLASH
	ulong addr_last = addr_first + size - 1;

	/* round last address to the sector boundary */
	නම් (flash_sect_roundb(&addr_last) > 0)
		ආපසු 1;

	නම් (addr_first >= addr_last) {
		printf("Error: end address exceeds addressing space\n");
		ආපසු 1;
	}

	/* remove protection on processed sectors */
	නම් (update_flash_protect(0, addr_first, addr_last) > 0) {
		printf("Error: could not unprotect flash sectors\n");
		ආපසු 1;
	}

	printf("Erasing 0x%08lx - 0x%08lx", addr_first, addr_last);
	නම් (flash_sect_erase(addr_first, addr_last) > 0) {
		printf("Error: could not erase flash\n");
		ආපසු 1;
	}

	printf("Copying to flash...");
	නම් (flash_write((var_na *)addr_source, addr_first, size) > 0) {
		printf("Error: could not copy to flash\n");
		ආපසු 1;
	}
	printf("done\n");

	/* enable protection on processed sectors */
	නම් (update_flash_protect(1, addr_first, addr_last) > 0) {
		printf("Error: could not protect flash sectors\n");
		ආපසු 1;
	}
#endif
	ආපසු 0;
}
#endif /* CONFIG_DFU_TFTP || CONFIG_UPDATE_TFTP */

static පූර්ණ_සංඛ්‍යා update_fit_getparams(const void *fit, පූර්ණ_සංඛ්‍යා noffset, ulong *addr,
						ulong *fladdr, ulong *size)
{
	const void *data;

	නම් (fit_image_get_data(fit, noffset, &data, (size_t *)size))
		ආපසු 1;

	නම් (fit_image_get_load(fit, noffset, (ulong *)fladdr))
		ආපසු 1;

	*addr = (ulong)data;

	ආපසු 0;
}

#නම් defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
පූර්ණ_සංඛ්‍යා update_tftp(ulong addr, var_na *interface, var_na *devstring)
{
	var_na *filename, *env_addr, *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
	පූර්ණ_සංඛ්‍යා images_noffset, ndepth, noffset;
	bool update_tftp_dfu;
	පූර්ණ_සංඛ්‍යා ret = 0;
	void *fit;

	නම් (interface == NULL && devstring == NULL) {
		update_tftp_dfu = false;
	} නැතහොත් නම් (interface && devstring) {
		update_tftp_dfu = true;
	} නැතහොත් {
		pr_err("Interface: %s and devstring: %s not supported!\n",
		      interface, devstring);
		ආපසු -EINVAL;
	}

	/* use already present image */
	නම් (addr)
		goto got_update_file;

	printf("Auto-update from TFTP: ");

	/* get the file name of the update file */
	filename = env_get(UPDATE_FILE_ENV);
	නම් (filename == NULL) {
		printf("failed, env. variable '%s' not found\n",
							UPDATE_FILE_ENV);
		ආපසු 1;
	}

	printf("trying update file '%s'\n", filename);

	/* get load address of downloaded update file */
	env_addr = env_get("loadaddr");
	නම් (env_addr)
		addr = hextoul(env_addr, NULL);
	නැතහොත්
		addr = CONFIG_UPDATE_LOAD_ADDR;

	නම් (update_load(filename, CONFIG_UPDATE_TFTP_MSEC_MAX,
					CONFIG_UPDATE_TFTP_CNT_MAX, addr)) {
		printf("Can't load update file, aborting auto-update\n");
		ආපසු 1;
	}

got_update_file:
	fit = map_sysmem(addr, 0);

	නම් (fit_check_format((void *)fit, IMAGE_SIZE_INVAL)) {
		printf("Bad FIT format of the update file, aborting "
							"auto-update\n");
		ආපසු 1;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_node(fit, images_noffset, &ndepth);
	අතරතුර (noffset >= 0 && ndepth > 0) {
		නම් (ndepth != 1)
			goto next_node;

		fit_image_name = (var_na *)fit_get_name(fit, noffset, NULL);
		printf("Processing update '%s' :", fit_image_name);

		නම් (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			goto next_node;
		}

		printf("\n");
		නම් (update_fit_getparams(fit, noffset, &update_addr,
					&update_fladdr, &update_size)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			goto next_node;
		}

		නම් (!update_tftp_dfu) {
			නම් (update_flash(update_addr, update_fladdr,
					 update_size)) {
				printf("Error: can't flash update, aborting\n");
				ret = 1;
				goto next_node;
			}
		} නැතහොත් නම් (fit_image_check_type(fit, noffset,
						IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(void *)update_addr,
						update_size, interface,
						devstring);
			නම් (ret)
				ආපසු ret;
		}
next_node:
		noffset = fdt_next_node(fit, noffset, &ndepth);
	}

	ආපසු ret;
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
පූර්ණ_සංඛ්‍යා fit_update(const void *fit)
{
	var_na *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
	පූර්ණ_සංඛ්‍යා images_noffset, ndepth, noffset;
	පූර්ණ_සංඛ්‍යා ret = 0;

	නම් (!fit)
		ආපසු -EINVAL;

	නම් (fit_check_format((void *)fit, IMAGE_SIZE_INVAL)) {
		printf("Bad FIT format of the update file, aborting auto-update\n");
		ආපසු -EINVAL;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_node(fit, images_noffset, &ndepth);
	අතරතුර (noffset >= 0 && ndepth > 0) {
		නම් (ndepth != 1)
			goto next_node;

		fit_image_name = (var_na *)fit_get_name(fit, noffset, NULL);
		printf("Processing update '%s' :", fit_image_name);

		නම් (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			goto next_node;
		}

		printf("\n");
		නම් (update_fit_getparams(fit, noffset, &update_addr,
					 &update_fladdr, &update_size)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			goto next_node;
		}

		නම් (fit_image_check_type(fit, noffset, IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(void *)update_addr,
						update_size, NULL, NULL);
			නම් (ret)
				ආපසු ret;
		}
next_node:
		noffset = fdt_next_node(fit, noffset, &ndepth);
	}

	ආපසු ret;
}
#endif /* CONFIG_UPDATE_FIT */
