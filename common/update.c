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

#якщо defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
/* env variable holding the location of the update file */
#define UPDATE_FILE_ENV		"updatefile"

extern ulong tftp_timeout_ms;
extern ціле tftp_timeout_count_max;
#ifdef CONFIG_MTD_NOR_FLASH
#include <flash.h>
#include <mtd/cfi_flash.h>
static uchar *saved_prot_info;
#endif
static ціле update_load(char *filename, ulong msec_max, ціле cnt_max, ulong addr)
{
	ціле rv, ret;
	ulong saved_timeout_msecs;
	ціле saved_timeout_count;
	char *saved_netretry, *saved_bootfile;

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
	copy_filename(net_boot_file_name, filename, sizeof(net_boot_file_name));
	ret = net_loop(TFTPGET);

	якщо (ret < 0)
		rv = 1;
	інакше
		flush_cache(addr, net_boot_file_size);

	/* restore changed globals and env variable */
	tftp_timeout_ms = saved_timeout_msecs;
	tftp_timeout_count_max = saved_timeout_count;

	env_set("netretry", saved_netretry);
	якщо (saved_netretry != NULL)
		free(saved_netretry);

	якщо (saved_bootfile != NULL) {
		copy_filename(net_boot_file_name, saved_bootfile,
			      sizeof(net_boot_file_name));
		free(saved_bootfile);
	}

	повернути rv;
}

#ifdef CONFIG_MTD_NOR_FLASH
static ціле update_flash_protect(ціле prot, ulong addr_first, ulong addr_last)
{
	uchar *sp_info_ptr;
	ulong s;
	ціле i, bank, cnt;
	flash_info_t *info;

	sp_info_ptr = NULL;

	якщо (prot == 0) {
		saved_prot_info =
			calloc(CFI_FLASH_BANKS * CONFIG_SYS_MAX_FLASH_SECT, 1);
		якщо (!saved_prot_info)
			повернути 1;
	}

	для (bank = 0; bank < CFI_FLASH_BANKS; ++bank) {
		cnt = 0;
		info = &flash_info[bank];

		/* Nothing to do if the bank doesn't exist */
		якщо (info->sector_count == 0)
			повернути 0;

		/* Point to current bank protection information */
		sp_info_ptr = saved_prot_info + (bank * CONFIG_SYS_MAX_FLASH_SECT);

		/*
		 * Adjust addr_first or addr_last if we are on bank boundary.
		 * Address space between banks must be continuous for other
		 * flash functions (like flash_sect_erase or flash_write) to
		 * succeed. Banks must also be numbered in correct order,
		 * according to increasing addresses.
		 */
		якщо (addr_last > info->start[0] + info->size - 1)
			addr_last = info->start[0] + info->size - 1;
		якщо (addr_first < info->start[0])
			addr_first = info->start[0];

		для (i = 0; i < info->sector_count; i++) {
			/* Save current information about protected sectors */
			якщо (prot == 0) {
				s = info->start[i];
				якщо ((s >= addr_first) && (s <= addr_last))
					sp_info_ptr[i] = info->protect[i];

			}

			/* Protect/unprotect sectors */
			якщо (sp_info_ptr[i] == 1) {
#якщо defined(CONFIG_SYS_FLASH_PROTECTION)
				якщо (flash_real_protect(info, i, prot))
					повернути 1;
#інакше
				info->protect[i] = prot;
#endif
				cnt++;
			}
		}

		якщо (cnt) {
			printf("%sProtected %d sectors\n",
						prot ? "": "Un-", cnt);
		}
	}

	якщо((prot == 1) && saved_prot_info)
		free(saved_prot_info);

	повернути 0;
}
#endif

static ціле update_flash(ulong addr_source, ulong addr_first, ulong size)
{
#ifdef CONFIG_MTD_NOR_FLASH
	ulong addr_last = addr_first + size - 1;

	/* round last address to the sector boundary */
	якщо (flash_sect_roundb(&addr_last) > 0)
		повернути 1;

	якщо (addr_first >= addr_last) {
		printf("Error: end address exceeds addressing space\n");
		повернути 1;
	}

	/* remove protection on processed sectors */
	якщо (update_flash_protect(0, addr_first, addr_last) > 0) {
		printf("Error: could not unprotect flash sectors\n");
		повернути 1;
	}

	printf("Erasing 0x%08lx - 0x%08lx", addr_first, addr_last);
	якщо (flash_sect_erase(addr_first, addr_last) > 0) {
		printf("Error: could not erase flash\n");
		повернути 1;
	}

	printf("Copying to flash...");
	якщо (flash_write((char *)addr_source, addr_first, size) > 0) {
		printf("Error: could not copy to flash\n");
		повернути 1;
	}
	printf("done\n");

	/* enable protection on processed sectors */
	якщо (update_flash_protect(1, addr_first, addr_last) > 0) {
		printf("Error: could not protect flash sectors\n");
		повернути 1;
	}
#endif
	повернути 0;
}
#endif /* CONFIG_DFU_TFTP || CONFIG_UPDATE_TFTP */

static ціле update_fit_getparams(const void *fit, ціле noffset, ulong *addr,
						ulong *fladdr, ulong *size)
{
	const void *data;

	якщо (fit_image_get_data(fit, noffset, &data, (size_t *)size))
		повернути 1;

	якщо (fit_image_get_load(fit, noffset, (ulong *)fladdr))
		повернути 1;

	*addr = (ulong)data;

	повернути 0;
}

#якщо defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
ціле update_tftp(ulong addr, char *interface, char *devstring)
{
	char *filename, *env_addr, *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
	ціле images_noffset, ndepth, noffset;
	bool update_tftp_dfu;
	ціле ret = 0;
	void *fit;

	якщо (interface == NULL && devstring == NULL) {
		update_tftp_dfu = false;
	} інакше якщо (interface && devstring) {
		update_tftp_dfu = true;
	} інакше {
		pr_err("Interface: %s and devstring: %s not supported!\n",
		      interface, devstring);
		повернути -EINVAL;
	}

	/* use already present image */
	якщо (addr)
		goto got_update_file;

	printf("Auto-update from TFTP: ");

	/* get the file name of the update file */
	filename = env_get(UPDATE_FILE_ENV);
	якщо (filename == NULL) {
		printf("failed, env. variable '%s' not found\n",
							UPDATE_FILE_ENV);
		повернути 1;
	}

	printf("trying update file '%s'\n", filename);

	/* get load address of downloaded update file */
	env_addr = env_get("loadaddr");
	якщо (env_addr)
		addr = hextoul(env_addr, NULL);
	інакше
		addr = CONFIG_UPDATE_LOAD_ADDR;

	якщо (update_load(filename, CONFIG_UPDATE_TFTP_MSEC_MAX,
					CONFIG_UPDATE_TFTP_CNT_MAX, addr)) {
		printf("Can't load update file, aborting auto-update\n");
		повернути 1;
	}

got_update_file:
	fit = map_sysmem(addr, 0);

	якщо (fit_check_format((void *)fit, IMAGE_SIZE_INVAL)) {
		printf("Bad FIT format of the update file, aborting "
							"auto-update\n");
		повернути 1;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_node(fit, images_noffset, &ndepth);
	поки (noffset >= 0 && ndepth > 0) {
		якщо (ndepth != 1)
			goto next_node;

		fit_image_name = (char *)fit_get_name(fit, noffset, NULL);
		printf("Processing update '%s' :", fit_image_name);

		якщо (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			goto next_node;
		}

		printf("\n");
		якщо (update_fit_getparams(fit, noffset, &update_addr,
					&update_fladdr, &update_size)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			goto next_node;
		}

		якщо (!update_tftp_dfu) {
			якщо (update_flash(update_addr, update_fladdr,
					 update_size)) {
				printf("Error: can't flash update, aborting\n");
				ret = 1;
				goto next_node;
			}
		} інакше якщо (fit_image_check_type(fit, noffset,
						IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(void *)update_addr,
						update_size, interface,
						devstring);
			якщо (ret)
				повернути ret;
		}
next_node:
		noffset = fdt_next_node(fit, noffset, &ndepth);
	}

	повернути ret;
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
ціле fit_update(const void *fit)
{
	char *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
	ціле images_noffset, ndepth, noffset;
	ціле ret = 0;

	якщо (!fit)
		повернути -EINVAL;

	якщо (fit_check_format((void *)fit, IMAGE_SIZE_INVAL)) {
		printf("Bad FIT format of the update file, aborting auto-update\n");
		повернути -EINVAL;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_node(fit, images_noffset, &ndepth);
	поки (noffset >= 0 && ndepth > 0) {
		якщо (ndepth != 1)
			goto next_node;

		fit_image_name = (char *)fit_get_name(fit, noffset, NULL);
		printf("Processing update '%s' :", fit_image_name);

		якщо (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			goto next_node;
		}

		printf("\n");
		якщо (update_fit_getparams(fit, noffset, &update_addr,
					 &update_fladdr, &update_size)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			goto next_node;
		}

		якщо (fit_image_check_type(fit, noffset, IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(void *)update_addr,
						update_size, NULL, NULL);
			якщо (ret)
				повернути ret;
		}
next_node:
		noffset = fdt_next_node(fit, noffset, &ndepth);
	}

	повернути ret;
}
#endif /* CONFIG_UPDATE_FIT */
