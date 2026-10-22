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

#nếu defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
/* env variable holding the location of the update file */
#define UPDATE_FILE_ENV		"updatefile"

extern ulong tftp_timeout_ms;
extern số_nguyên tftp_timeout_count_max;
#ifdef CONFIG_MTD_NOR_FLASH
#include <flash.h>
#include <mtd/cfi_flash.h>
static uchar *saved_prot_info;
#endif
static số_nguyên update_load(char *filename, ulong msec_max, số_nguyên cnt_max, ulong addr)
{
	số_nguyên rv, ret;
	ulong saved_timeout_msecs;
	số_nguyên saved_timeout_count;
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

	nếu (ret < 0)
		rv = 1;
	không_thì
		flush_cache(addr, net_boot_file_size);

	/* restore changed globals and env variable */
	tftp_timeout_ms = saved_timeout_msecs;
	tftp_timeout_count_max = saved_timeout_count;

	env_set("netretry", saved_netretry);
	nếu (saved_netretry != NULL)
		free(saved_netretry);

	nếu (saved_bootfile != NULL) {
		copy_filename(net_boot_file_name, saved_bootfile,
			      sizeof(net_boot_file_name));
		free(saved_bootfile);
	}

	trả_về rv;
}

#ifdef CONFIG_MTD_NOR_FLASH
static số_nguyên update_flash_protect(số_nguyên prot, ulong addr_first, ulong addr_last)
{
	uchar *sp_info_ptr;
	ulong s;
	số_nguyên i, bank, cnt;
	flash_info_t *info;

	sp_info_ptr = NULL;

	nếu (prot == 0) {
		saved_prot_info =
			calloc(CFI_FLASH_BANKS * CONFIG_SYS_MAX_FLASH_SECT, 1);
		nếu (!saved_prot_info)
			trả_về 1;
	}

	cho (bank = 0; bank < CFI_FLASH_BANKS; ++bank) {
		cnt = 0;
		info = &flash_info[bank];

		/* Nothing to do if the bank doesn't exist */
		nếu (info->sector_count == 0)
			trả_về 0;

		/* Point to current bank protection information */
		sp_info_ptr = saved_prot_info + (bank * CONFIG_SYS_MAX_FLASH_SECT);

		/*
		 * Adjust addr_first or addr_last if we are on bank boundary.
		 * Address space between banks must be continuous for other
		 * flash functions (like flash_sect_erase or flash_write) to
		 * succeed. Banks must also be numbered in correct order,
		 * according to increasing addresses.
		 */
		nếu (addr_last > info->start[0] + info->size - 1)
			addr_last = info->start[0] + info->size - 1;
		nếu (addr_first < info->start[0])
			addr_first = info->start[0];

		cho (i = 0; i < info->sector_count; i++) {
			/* Save current information about protected sectors */
			nếu (prot == 0) {
				s = info->start[i];
				nếu ((s >= addr_first) && (s <= addr_last))
					sp_info_ptr[i] = info->protect[i];

			}

			/* Protect/unprotect sectors */
			nếu (sp_info_ptr[i] == 1) {
#nếu defined(CONFIG_SYS_FLASH_PROTECTION)
				nếu (flash_real_protect(info, i, prot))
					trả_về 1;
#không_thì
				info->protect[i] = prot;
#endif
				cnt++;
			}
		}

		nếu (cnt) {
			printf("%sProtected %d sectors\n",
						prot ? "": "Un-", cnt);
		}
	}

	nếu((prot == 1) && saved_prot_info)
		free(saved_prot_info);

	trả_về 0;
}
#endif

static số_nguyên update_flash(ulong addr_source, ulong addr_first, ulong size)
{
#ifdef CONFIG_MTD_NOR_FLASH
	ulong addr_last = addr_first + size - 1;

	/* round last address to the sector boundary */
	nếu (flash_sect_roundb(&addr_last) > 0)
		trả_về 1;

	nếu (addr_first >= addr_last) {
		printf("Error: end address exceeds addressing space\n");
		trả_về 1;
	}

	/* remove protection on processed sectors */
	nếu (update_flash_protect(0, addr_first, addr_last) > 0) {
		printf("Error: could not unprotect flash sectors\n");
		trả_về 1;
	}

	printf("Erasing 0x%08lx - 0x%08lx", addr_first, addr_last);
	nếu (flash_sect_erase(addr_first, addr_last) > 0) {
		printf("Error: could not erase flash\n");
		trả_về 1;
	}

	printf("Copying to flash...");
	nếu (flash_write((char *)addr_source, addr_first, size) > 0) {
		printf("Error: could not copy to flash\n");
		trả_về 1;
	}
	printf("done\n");

	/* enable protection on processed sectors */
	nếu (update_flash_protect(1, addr_first, addr_last) > 0) {
		printf("Error: could not protect flash sectors\n");
		trả_về 1;
	}
#endif
	trả_về 0;
}
#endif /* CONFIG_DFU_TFTP || CONFIG_UPDATE_TFTP */

static số_nguyên update_fit_getparams(const void *fit, số_nguyên noffset, ulong *addr,
						ulong *fladdr, ulong *size)
{
	const void *data;

	nếu (fit_image_get_data(fit, noffset, &data, (size_t *)size))
		trả_về 1;

	nếu (fit_image_get_load(fit, noffset, (ulong *)fladdr))
		trả_về 1;

	*addr = (ulong)data;

	trả_về 0;
}

#nếu defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
số_nguyên update_tftp(ulong addr, char *interface, char *devstring)
{
	char *filename, *env_addr, *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
	số_nguyên images_noffset, ndepth, noffset;
	bool update_tftp_dfu;
	số_nguyên ret = 0;
	void *fit;

	nếu (interface == NULL && devstring == NULL) {
		update_tftp_dfu = false;
	} không_thì nếu (interface && devstring) {
		update_tftp_dfu = true;
	} không_thì {
		pr_err("Interface: %s and devstring: %s not supported!\n",
		      interface, devstring);
		trả_về -EINVAL;
	}

	/* use already present image */
	nếu (addr)
		goto got_update_file;

	printf("Auto-update from TFTP: ");

	/* get the file name of the update file */
	filename = env_get(UPDATE_FILE_ENV);
	nếu (filename == NULL) {
		printf("failed, env. variable '%s' not found\n",
							UPDATE_FILE_ENV);
		trả_về 1;
	}

	printf("trying update file '%s'\n", filename);

	/* get load address of downloaded update file */
	env_addr = env_get("loadaddr");
	nếu (env_addr)
		addr = hextoul(env_addr, NULL);
	không_thì
		addr = CONFIG_UPDATE_LOAD_ADDR;

	nếu (update_load(filename, CONFIG_UPDATE_TFTP_MSEC_MAX,
					CONFIG_UPDATE_TFTP_CNT_MAX, addr)) {
		printf("Can't load update file, aborting auto-update\n");
		trả_về 1;
	}

got_update_file:
	fit = map_sysmem(addr, 0);

	nếu (fit_check_format((void *)fit, IMAGE_SIZE_INVAL)) {
		printf("Bad FIT format of the update file, aborting "
							"auto-update\n");
		trả_về 1;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_node(fit, images_noffset, &ndepth);
	trong_khi (noffset >= 0 && ndepth > 0) {
		nếu (ndepth != 1)
			goto next_node;

		fit_image_name = (char *)fit_get_name(fit, noffset, NULL);
		printf("Processing update '%s' :", fit_image_name);

		nếu (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			goto next_node;
		}

		printf("\n");
		nếu (update_fit_getparams(fit, noffset, &update_addr,
					&update_fladdr, &update_size)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			goto next_node;
		}

		nếu (!update_tftp_dfu) {
			nếu (update_flash(update_addr, update_fladdr,
					 update_size)) {
				printf("Error: can't flash update, aborting\n");
				ret = 1;
				goto next_node;
			}
		} không_thì nếu (fit_image_check_type(fit, noffset,
						IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(void *)update_addr,
						update_size, interface,
						devstring);
			nếu (ret)
				trả_về ret;
		}
next_node:
		noffset = fdt_next_node(fit, noffset, &ndepth);
	}

	trả_về ret;
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
số_nguyên fit_update(const void *fit)
{
	char *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
	số_nguyên images_noffset, ndepth, noffset;
	số_nguyên ret = 0;

	nếu (!fit)
		trả_về -EINVAL;

	nếu (fit_check_format((void *)fit, IMAGE_SIZE_INVAL)) {
		printf("Bad FIT format of the update file, aborting auto-update\n");
		trả_về -EINVAL;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_node(fit, images_noffset, &ndepth);
	trong_khi (noffset >= 0 && ndepth > 0) {
		nếu (ndepth != 1)
			goto next_node;

		fit_image_name = (char *)fit_get_name(fit, noffset, NULL);
		printf("Processing update '%s' :", fit_image_name);

		nếu (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			goto next_node;
		}

		printf("\n");
		nếu (update_fit_getparams(fit, noffset, &update_addr,
					 &update_fladdr, &update_size)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			goto next_node;
		}

		nếu (fit_image_check_type(fit, noffset, IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(void *)update_addr,
						update_size, NULL, NULL);
			nếu (ret)
				trả_về ret;
		}
next_node:
		noffset = fdt_next_node(fit, noffset, &ndepth);
	}

	trả_về ret;
}
#endif /* CONFIG_UPDATE_FIT */
