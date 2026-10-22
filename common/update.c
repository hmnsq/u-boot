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

#အကယ်၍ defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
/* env variable holding the location of the update file */
#define UPDATE_FILE_ENV		"updatefile"

extern ulong tftp_timeout_ms;
extern ကိန်းပြည့် tftp_timeout_count_max;
#ifdef CONFIG_MTD_NOR_FLASH
#include <flash.h>
#include <mtd/cfi_flash.h>
static uchar *saved_prot_info;
#endif
static ကိန်းပြည့် update_load(var_na *filename, ulong msec_max, ကိန်းပြည့် cnt_max, ulong addr)
{
	ကိန်းပြည့် rv, ret;
	ulong saved_timeout_msecs;
	ကိန်းပြည့် saved_timeout_count;
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

	အကယ်၍ (ret < 0)
		rv = 1;
	မဟုတ်ပါက
		flush_cache(addr, net_boot_file_size);

	/* restore changed globals and env variable */
	tftp_timeout_ms = saved_timeout_msecs;
	tftp_timeout_count_max = saved_timeout_count;

	env_set("netretry", saved_netretry);
	အကယ်၍ (saved_netretry != NULL)
		free(saved_netretry);

	အကယ်၍ (saved_bootfile != NULL) {
		copy_filename(net_boot_file_name, saved_bootfile,
			      parimaa_na(net_boot_file_name));
		free(saved_bootfile);
	}

	ပြန်ပေး rv;
}

#ifdef CONFIG_MTD_NOR_FLASH
static ကိန်းပြည့် update_flash_protect(ကိန်းပြည့် prot, ulong addr_first, ulong addr_last)
{
	uchar *sp_info_ptr;
	ulong s;
	ကိန်းပြည့် i, bank, cnt;
	flash_info_t *info;

	sp_info_ptr = NULL;

	အကယ်၍ (prot == 0) {
		saved_prot_info =
			calloc(CFI_FLASH_BANKS * CONFIG_SYS_MAX_FLASH_SECT, 1);
		အကယ်၍ (!saved_prot_info)
			ပြန်ပေး 1;
	}

	အတွက် (bank = 0; bank < CFI_FLASH_BANKS; ++bank) {
		cnt = 0;
		info = &flash_info[bank];

		/* Nothing to do if the bank doesn't exist */
		အကယ်၍ (info->sector_count == 0)
			ပြန်ပေး 0;

		/* Point to current bank protection information */
		sp_info_ptr = saved_prot_info + (bank * CONFIG_SYS_MAX_FLASH_SECT);

		/*
		 * Adjust addr_first or addr_last if we are on bank boundary.
		 * Address space between banks must be continuous for other
		 * flash functions (like flash_sect_erase or flash_write) to
		 * succeed. Banks must also be numbered in correct order,
		 * according to increasing addresses.
		 */
		အကယ်၍ (addr_last > info->start[0] + info->size - 1)
			addr_last = info->start[0] + info->size - 1;
		အကယ်၍ (addr_first < info->start[0])
			addr_first = info->start[0];

		အတွက် (i = 0; i < info->sector_count; i++) {
			/* Save current information about protected sectors */
			အကယ်၍ (prot == 0) {
				s = info->start[i];
				အကယ်၍ ((s >= addr_first) && (s <= addr_last))
					sp_info_ptr[i] = info->protect[i];

			}

			/* Protect/unprotect sectors */
			အကယ်၍ (sp_info_ptr[i] == 1) {
#အကယ်၍ defined(CONFIG_SYS_FLASH_PROTECTION)
				အကယ်၍ (flash_real_protect(info, i, prot))
					ပြန်ပေး 1;
#မဟုတ်ပါက
				info->protect[i] = prot;
#endif
				cnt++;
			}
		}

		အကယ်၍ (cnt) {
			printf("%sProtected %d sectors\n",
						prot ? "": "Un-", cnt);
		}
	}

	အကယ်၍((prot == 1) && saved_prot_info)
		free(saved_prot_info);

	ပြန်ပေး 0;
}
#endif

static ကိန်းပြည့် update_flash(ulong addr_source, ulong addr_first, ulong size)
{
#ifdef CONFIG_MTD_NOR_FLASH
	ulong addr_last = addr_first + size - 1;

	/* round last address to the sector boundary */
	အကယ်၍ (flash_sect_roundb(&addr_last) > 0)
		ပြန်ပေး 1;

	အကယ်၍ (addr_first >= addr_last) {
		printf("Error: end address exceeds addressing space\n");
		ပြန်ပေး 1;
	}

	/* remove protection on processed sectors */
	အကယ်၍ (update_flash_protect(0, addr_first, addr_last) > 0) {
		printf("Error: could not unprotect flash sectors\n");
		ပြန်ပေး 1;
	}

	printf("Erasing 0x%08lx - 0x%08lx", addr_first, addr_last);
	အကယ်၍ (flash_sect_erase(addr_first, addr_last) > 0) {
		printf("Error: could not erase flash\n");
		ပြန်ပေး 1;
	}

	printf("Copying to flash...");
	အကယ်၍ (flash_write((var_na *)addr_source, addr_first, size) > 0) {
		printf("Error: could not copy to flash\n");
		ပြန်ပေး 1;
	}
	printf("done\n");

	/* enable protection on processed sectors */
	အကယ်၍ (update_flash_protect(1, addr_first, addr_last) > 0) {
		printf("Error: could not protect flash sectors\n");
		ပြန်ပေး 1;
	}
#endif
	ပြန်ပေး 0;
}
#endif /* CONFIG_DFU_TFTP || CONFIG_UPDATE_TFTP */

static ကိန်းပြည့် update_fit_getparams(const void *fit, ကိန်းပြည့် noffset, ulong *addr,
						ulong *fladdr, ulong *size)
{
	const void *data;

	အကယ်၍ (fit_image_get_data(fit, noffset, &data, (size_t *)size))
		ပြန်ပေး 1;

	အကယ်၍ (fit_image_get_load(fit, noffset, (ulong *)fladdr))
		ပြန်ပေး 1;

	*addr = (ulong)data;

	ပြန်ပေး 0;
}

#အကယ်၍ defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
ကိန်းပြည့် update_tftp(ulong addr, var_na *interface, var_na *devstring)
{
	var_na *filename, *env_addr, *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
	ကိန်းပြည့် images_noffset, ndepth, noffset;
	bool update_tftp_dfu;
	ကိန်းပြည့် ret = 0;
	void *fit;

	အကယ်၍ (interface == NULL && devstring == NULL) {
		update_tftp_dfu = false;
	} မဟုတ်ပါက အကယ်၍ (interface && devstring) {
		update_tftp_dfu = true;
	} မဟုတ်ပါက {
		pr_err("Interface: %s and devstring: %s not supported!\n",
		      interface, devstring);
		ပြန်ပေး -EINVAL;
	}

	/* use already present image */
	အကယ်၍ (addr)
		goto got_update_file;

	printf("Auto-update from TFTP: ");

	/* get the file name of the update file */
	filename = env_get(UPDATE_FILE_ENV);
	အကယ်၍ (filename == NULL) {
		printf("failed, env. variable '%s' not found\n",
							UPDATE_FILE_ENV);
		ပြန်ပေး 1;
	}

	printf("trying update file '%s'\n", filename);

	/* get load address of downloaded update file */
	env_addr = env_get("loadaddr");
	အကယ်၍ (env_addr)
		addr = hextoul(env_addr, NULL);
	မဟုတ်ပါက
		addr = CONFIG_UPDATE_LOAD_ADDR;

	အကယ်၍ (update_load(filename, CONFIG_UPDATE_TFTP_MSEC_MAX,
					CONFIG_UPDATE_TFTP_CNT_MAX, addr)) {
		printf("Can't load update file, aborting auto-update\n");
		ပြန်ပေး 1;
	}

got_update_file:
	fit = map_sysmem(addr, 0);

	အကယ်၍ (fit_check_format((void *)fit, IMAGE_SIZE_INVAL)) {
		printf("Bad FIT format of the update file, aborting "
							"auto-update\n");
		ပြန်ပေး 1;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_node(fit, images_noffset, &ndepth);
	နေစဉ် (noffset >= 0 && ndepth > 0) {
		အကယ်၍ (ndepth != 1)
			goto next_node;

		fit_image_name = (var_na *)fit_get_name(fit, noffset, NULL);
		printf("Processing update '%s' :", fit_image_name);

		အကယ်၍ (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			goto next_node;
		}

		printf("\n");
		အကယ်၍ (update_fit_getparams(fit, noffset, &update_addr,
					&update_fladdr, &update_size)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			goto next_node;
		}

		အကယ်၍ (!update_tftp_dfu) {
			အကယ်၍ (update_flash(update_addr, update_fladdr,
					 update_size)) {
				printf("Error: can't flash update, aborting\n");
				ret = 1;
				goto next_node;
			}
		} မဟုတ်ပါက အကယ်၍ (fit_image_check_type(fit, noffset,
						IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(void *)update_addr,
						update_size, interface,
						devstring);
			အကယ်၍ (ret)
				ပြန်ပေး ret;
		}
next_node:
		noffset = fdt_next_node(fit, noffset, &ndepth);
	}

	ပြန်ပေး ret;
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
ကိန်းပြည့် fit_update(const void *fit)
{
	var_na *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
	ကိန်းပြည့် images_noffset, ndepth, noffset;
	ကိန်းပြည့် ret = 0;

	အကယ်၍ (!fit)
		ပြန်ပေး -EINVAL;

	အကယ်၍ (fit_check_format((void *)fit, IMAGE_SIZE_INVAL)) {
		printf("Bad FIT format of the update file, aborting auto-update\n");
		ပြန်ပေး -EINVAL;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_node(fit, images_noffset, &ndepth);
	နေစဉ် (noffset >= 0 && ndepth > 0) {
		အကယ်၍ (ndepth != 1)
			goto next_node;

		fit_image_name = (var_na *)fit_get_name(fit, noffset, NULL);
		printf("Processing update '%s' :", fit_image_name);

		အကယ်၍ (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			goto next_node;
		}

		printf("\n");
		အကယ်၍ (update_fit_getparams(fit, noffset, &update_addr,
					 &update_fladdr, &update_size)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			goto next_node;
		}

		အကယ်၍ (fit_image_check_type(fit, noffset, IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(void *)update_addr,
						update_size, NULL, NULL);
			အကယ်၍ (ret)
				ပြန်ပေး ret;
		}
next_node:
		noffset = fdt_next_node(fit, noffset, &ndepth);
	}

	ပြန်ပေး ret;
}
#endif /* CONFIG_UPDATE_FIT */
