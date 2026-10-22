// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Semihalf
 *
 * Written by: Rafal Czubak <rcz@semihalf.com>
 *             Bartlomiej Sieka <tur@semihalf.com>
 */

#شامل <cpu_func.h>
#شامل <image.h>
#شامل <linux/printk.h>

#شامل <command.h>
#شامل <env.h>
#شامل <net.h>
#شامل <net/tftp.h>
#شامل <malloc.h>
#شامل <mapmem.h>
#شامل <dfu.h>
#شامل <errno.h>

#اگر defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
/* env variable holding the location of the update file */
#define UPDATE_FILE_ENV		"updatefile"

بیرونی ulong tftp_timeout_ms;
بیرونی صحیح tftp_timeout_شمار_max;
#ifdef CONFIG_MTD_NOR_FLASH
#شامل <flash.h>
#شامل <mtd/cfi_flash.h>
ایستا uchar *saved_prot_info;
#endif
ایستا صحیح update_load(نویسه *filename, ulong msec_max, صحیح cnt_max, ulong addr)
{
	صحیح rv, ret;
	ulong saved_timeout_msecs;
	صحیح saved_timeout_شمار;
	نویسه *saved_netretry, *saved_bootfile;

	rv = 0;
	/* save used globals and env variable */
	saved_timeout_msecs = tftp_timeout_ms;
	saved_timeout_شمار = tftp_timeout_شمار_max;
	saved_netretry = strdup(env_get("netretry"));
	saved_bootfile = strdup(net_boot_file_name);

	/* set timeouts for auto-update */
	tftp_timeout_ms = msec_max;
	tftp_timeout_شمار_max = cnt_max;

	/* we don't want to retry the connection if errors occur */
	env_set("netretry", "no");

	/* download the update file */
	image_load_addr = addr;
	copy_filename(net_boot_file_name, filename, اندازه(net_boot_file_name));
	ret = net_loop(TFTPGET);

	اگر (ret < 0)
		rv = 1;
	وگرنه
		flush_cache(addr, net_boot_file_size);

	/* restore changed globals and env variable */
	tftp_timeout_ms = saved_timeout_msecs;
	tftp_timeout_شمار_max = saved_timeout_شمار;

	env_set("netretry", saved_netretry);
	اگر (saved_netretry != پوچ)
		آزاد(saved_netretry);

	اگر (saved_bootfile != پوچ) {
		copy_filename(net_boot_file_name, saved_bootfile,
			      اندازه(net_boot_file_name));
		آزاد(saved_bootfile);
	}

	بازگشت rv;
}

#ifdef CONFIG_MTD_NOR_FLASH
ایستا صحیح update_flash_protect(صحیح prot, ulong addr_first, ulong addr_last)
{
	uchar *sp_info_ptr;
	ulong s;
	صحیح i, bank, cnt;
	flash_info_t *info;

	sp_info_ptr = پوچ;

	اگر (prot == 0) {
		saved_prot_info =
			calloc(CFI_FLASH_BANKS * CONFIG_SYS_MAX_FLASH_SECT, 1);
		اگر (!saved_prot_info)
			بازگشت 1;
	}

	برای (bank = 0; bank < CFI_FLASH_BANKS; ++bank) {
		cnt = 0;
		info = &flash_info[bank];

		/* Nothing to do if the bank doesn't exist */
		اگر (info->sector_شمار == 0)
			بازگشت 0;

		/* Point to current bank protection information */
		sp_info_ptr = saved_prot_info + (bank * CONFIG_SYS_MAX_FLASH_SECT);

		/*
		 * Adjust addr_first or addr_last if we are on bank boundary.
		 * Address space between banks must be continuous for other
		 * flash functions (like flash_sect_erase or flash_write) to
		 * succeed. Banks must also be numbered in correct order,
		 * according to increasing addresses.
		 */
		اگر (addr_last > info->start[0] + info->size - 1)
			addr_last = info->start[0] + info->size - 1;
		اگر (addr_first < info->start[0])
			addr_first = info->start[0];

		برای (i = 0; i < info->sector_شمار; i++) {
			/* Save current information about protected sectors */
			اگر (prot == 0) {
				s = info->start[i];
				اگر ((s >= addr_first) && (s <= addr_last))
					sp_info_ptr[i] = info->protect[i];

			}

			/* Protect/unprotect sectors */
			اگر (sp_info_ptr[i] == 1) {
#اگر defined(CONFIG_SYS_FLASH_PROTECTION)
				اگر (flash_real_protect(info, i, prot))
					بازگشت 1;
#وگرنه
				info->protect[i] = prot;
#endif
				cnt++;
			}
		}

		اگر (cnt) {
			printf("%sProtected %d sectors\n",
						prot ? "": "Un-", cnt);
		}
	}

	اگر((prot == 1) && saved_prot_info)
		آزاد(saved_prot_info);

	بازگشت 0;
}
#endif

ایستا صحیح update_flash(ulong addr_source, ulong addr_first, ulong size)
{
#ifdef CONFIG_MTD_NOR_FLASH
	ulong addr_last = addr_first + size - 1;

	/* round last address to the sector boundary */
	اگر (flash_sect_roundb(&addr_last) > 0)
		بازگشت 1;

	اگر (addr_first >= addr_last) {
		printf("Error: end address exceeds addressing space\n");
		بازگشت 1;
	}

	/* remove protection on processed sectors */
	اگر (update_flash_protect(0, addr_first, addr_last) > 0) {
		printf("Error: could not unprotect flash sectors\n");
		بازگشت 1;
	}

	printf("Erasing 0x%08lx - 0x%08lx", addr_first, addr_last);
	اگر (flash_sect_erase(addr_first, addr_last) > 0) {
		printf("Error: could not erase flash\n");
		بازگشت 1;
	}

	printf("Copying to flash...");
	اگر (flash_write((نویسه *)addr_source, addr_first, size) > 0) {
		printf("Error: could not copy to flash\n");
		بازگشت 1;
	}
	printf("done\n");

	/* enable protection on processed sectors */
	اگر (update_flash_protect(1, addr_first, addr_last) > 0) {
		printf("Error: could not protect flash sectors\n");
		بازگشت 1;
	}
#endif
	بازگشت 0;
}
#endif /* CONFIG_DFU_TFTP || CONFIG_UPDATE_TFTP */

ایستا صحیح update_fit_getparams(ثابت تهی *fit, صحیح noffset, ulong *addr,
						ulong *fladdr, ulong *size)
{
	ثابت تهی *data;

	اگر (fit_image_get_data(fit, noffset, &data, (size_t *)size))
		بازگشت 1;

	اگر (fit_image_get_load(fit, noffset, (ulong *)fladdr))
		بازگشت 1;

	*addr = (ulong)data;

	بازگشت 0;
}

#اگر defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
صحیح update_tftp(ulong addr, نویسه *interface, نویسه *devstring)
{
	نویسه *filename, *env_addr, *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
	صحیح images_noffset, ndepth, noffset;
	bool update_tftp_dfu;
	صحیح ret = 0;
	تهی *fit;

	اگر (interface == پوچ && devstring == پوچ) {
		update_tftp_dfu = false;
	} وگرنه اگر (interface && devstring) {
		update_tftp_dfu = true;
	} وگرنه {
		pr_err("Interface: %s and devstring: %s not supported!\n",
		      interface, devstring);
		بازگشت -EINVAL;
	}

	/* use already present image */
	اگر (addr)
		برو got_update_file;

	printf("Auto-update from TFTP: ");

	/* get the file name of the update file */
	filename = env_get(UPDATE_FILE_ENV);
	اگر (filename == پوچ) {
		printf("failed, env. variable '%s' not found\n",
							UPDATE_FILE_ENV);
		بازگشت 1;
	}

	printf("trying update file '%s'\n", filename);

	/* get load address of downloaded update file */
	env_addr = env_get("loadaddr");
	اگر (env_addr)
		addr = hextoul(env_addr, پوچ);
	وگرنه
		addr = CONFIG_UPDATE_LOAD_ADDR;

	اگر (update_load(filename, CONFIG_UPDATE_TFTP_MSEC_MAX,
					CONFIG_UPDATE_TFTP_CNT_MAX, addr)) {
		printf("Can't load update file, aborting auto-update\n");
		بازگشت 1;
	}

got_update_file:
	fit = map_sysmem(addr, 0);

	اگر (fit_check_format((تهی *)fit, IMAGE_SIZE_INVAL)) {
		printf("Bad FIT format of the update file, aborting "
							"auto-update\n");
		بازگشت 1;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_گره(fit, images_noffset, &ndepth);
	وقتی (noffset >= 0 && ndepth > 0) {
		اگر (ndepth != 1)
			برو next_گره;

		fit_image_name = (نویسه *)fit_get_name(fit, noffset, پوچ);
		printf("Processing update '%s' :", fit_image_name);

		اگر (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			برو next_گره;
		}

		printf("\n");
		اگر (update_fit_getparams(fit, noffset, &update_addr,
					&update_fladdr, &update_size)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			برو next_گره;
		}

		اگر (!update_tftp_dfu) {
			اگر (update_flash(update_addr, update_fladdr,
					 update_size)) {
				printf("Error: can't flash update, aborting\n");
				ret = 1;
				برو next_گره;
			}
		} وگرنه اگر (fit_image_check_type(fit, noffset,
						IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(تهی *)update_addr,
						update_size, interface,
						devstring);
			اگر (ret)
				بازگشت ret;
		}
next_گره:
		noffset = fdt_next_گره(fit, noffset, &ndepth);
	}

	بازگشت ret;
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
صحیح fit_update(ثابت تهی *fit)
{
	نویسه *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
	صحیح images_noffset, ndepth, noffset;
	صحیح ret = 0;

	اگر (!fit)
		بازگشت -EINVAL;

	اگر (fit_check_format((تهی *)fit, IMAGE_SIZE_INVAL)) {
		printf("Bad FIT format of the update file, aborting auto-update\n");
		بازگشت -EINVAL;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_گره(fit, images_noffset, &ndepth);
	وقتی (noffset >= 0 && ndepth > 0) {
		اگر (ndepth != 1)
			برو next_گره;

		fit_image_name = (نویسه *)fit_get_name(fit, noffset, پوچ);
		printf("Processing update '%s' :", fit_image_name);

		اگر (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			برو next_گره;
		}

		printf("\n");
		اگر (update_fit_getparams(fit, noffset, &update_addr,
					 &update_fladdr, &update_size)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			برو next_گره;
		}

		اگر (fit_image_check_type(fit, noffset, IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(تهی *)update_addr,
						update_size, پوچ, پوچ);
			اگر (ret)
				بازگشت ret;
		}
next_گره:
		noffset = fdt_next_گره(fit, noffset, &ndepth);
	}

	بازگشت ret;
}
#endif /* CONFIG_UPDATE_FIT */
