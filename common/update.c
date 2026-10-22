// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Semihalf
 *
 * Written by: Rafal Czubak <rcz@semihalf.com>
 *             Bartlomiej Sieka <tur@semihalf.com>
 */

#ಅನ್ತರ್ಭಾವ <cpu_func.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <linux/printk.h>

#ಅನ್ತರ್ಭಾವ <command.h>
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <net.h>
#ಅನ್ತರ್ಭಾವ <net/tftp.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <dfu.h>
#ಅನ್ತರ್ಭಾವ <errno.h>

#ಯದಿ defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
/* env variable holding the location of the update file */
#define UPDATE_FILE_ENV		"updatefile"

ಬಾಹ್ಯ ulong tftp_timeout_ms;
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ tftp_timeout_ಎಣಿಕೆ_max;
#ifdef CONFIG_MTD_NOR_FLASH
#ಅನ್ತರ್ಭಾವ <flash.h>
#ಅನ್ತರ್ಭಾವ <mtd/cfi_flash.h>
ಸ್ಥಿರ uchar *saved_prot_info;
#endif
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ update_load(ವರ್ಣ *filename, ulong msec_max, ಪೂರ್ಣಾಙ್ಕ cnt_max, ulong addr)
{
	ಪೂರ್ಣಾಙ್ಕ rv, ret;
	ulong saved_timeout_msecs;
	ಪೂರ್ಣಾಙ್ಕ saved_timeout_ಎಣಿಕೆ;
	ವರ್ಣ *saved_netretry, *saved_bootfile;

	rv = 0;
	/* save used globals and env variable */
	saved_timeout_msecs = tftp_timeout_ms;
	saved_timeout_ಎಣಿಕೆ = tftp_timeout_ಎಣಿಕೆ_max;
	saved_netretry = strdup(env_get("netretry"));
	saved_bootfile = strdup(net_boot_file_name);

	/* set timeouts for auto-update */
	tftp_timeout_ms = msec_max;
	tftp_timeout_ಎಣಿಕೆ_max = cnt_max;

	/* we don't want to retry the connection if errors occur */
	env_set("netretry", "no");

	/* download the update file */
	image_load_addr = addr;
	copy_filename(net_boot_file_name, filename, ಪರಿಮಾಣ(net_boot_file_name));
	ret = net_loop(TFTPGET);

	ಯದಿ (ret < 0)
		rv = 1;
	ಅನ್ಯಥಾ
		flush_cache(addr, net_boot_file_size);

	/* restore changed globals and env variable */
	tftp_timeout_ms = saved_timeout_msecs;
	tftp_timeout_ಎಣಿಕೆ_max = saved_timeout_ಎಣಿಕೆ;

	env_set("netretry", saved_netretry);
	ಯದಿ (saved_netretry != ರಿಕ್ತ)
		ಮುಕ್ತ(saved_netretry);

	ಯದಿ (saved_bootfile != ರಿಕ್ತ) {
		copy_filename(net_boot_file_name, saved_bootfile,
			      ಪರಿಮಾಣ(net_boot_file_name));
		ಮುಕ್ತ(saved_bootfile);
	}

	ಪ್ರತ್ಯಾಗಮ rv;
}

#ifdef CONFIG_MTD_NOR_FLASH
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ update_flash_protect(ಪೂರ್ಣಾಙ್ಕ prot, ulong addr_first, ulong addr_last)
{
	uchar *sp_info_ptr;
	ulong s;
	ಪೂರ್ಣಾಙ್ಕ i, bank, cnt;
	flash_info_t *info;

	sp_info_ptr = ರಿಕ್ತ;

	ಯದಿ (prot == 0) {
		saved_prot_info =
			calloc(CFI_FLASH_BANKS * CONFIG_SYS_MAX_FLASH_SECT, 1);
		ಯದಿ (!saved_prot_info)
			ಪ್ರತ್ಯಾಗಮ 1;
	}

	ಪ್ರತಿ (bank = 0; bank < CFI_FLASH_BANKS; ++bank) {
		cnt = 0;
		info = &flash_info[bank];

		/* Nothing to do if the bank doesn't exist */
		ಯದಿ (info->sector_ಎಣಿಕೆ == 0)
			ಪ್ರತ್ಯಾಗಮ 0;

		/* Point to current bank protection information */
		sp_info_ptr = saved_prot_info + (bank * CONFIG_SYS_MAX_FLASH_SECT);

		/*
		 * Adjust addr_first or addr_last if we are on bank boundary.
		 * Address space between banks must be continuous for other
		 * flash functions (like flash_sect_erase or flash_write) to
		 * succeed. Banks must also be numbered in correct order,
		 * according to increasing addresses.
		 */
		ಯದಿ (addr_last > info->start[0] + info->size - 1)
			addr_last = info->start[0] + info->size - 1;
		ಯದಿ (addr_first < info->start[0])
			addr_first = info->start[0];

		ಪ್ರತಿ (i = 0; i < info->sector_ಎಣಿಕೆ; i++) {
			/* Save current information about protected sectors */
			ಯದಿ (prot == 0) {
				s = info->start[i];
				ಯದಿ ((s >= addr_first) && (s <= addr_last))
					sp_info_ptr[i] = info->protect[i];

			}

			/* Protect/unprotect sectors */
			ಯದಿ (sp_info_ptr[i] == 1) {
#ಯದಿ defined(CONFIG_SYS_FLASH_PROTECTION)
				ಯದಿ (flash_real_protect(info, i, prot))
					ಪ್ರತ್ಯಾಗಮ 1;
#ಅನ್ಯಥಾ
				info->protect[i] = prot;
#endif
				cnt++;
			}
		}

		ಯದಿ (cnt) {
			printf("%sProtected %d sectors\n",
						prot ? "": "Un-", cnt);
		}
	}

	ಯದಿ((prot == 1) && saved_prot_info)
		ಮುಕ್ತ(saved_prot_info);

	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ update_flash(ulong addr_source, ulong addr_first, ulong size)
{
#ifdef CONFIG_MTD_NOR_FLASH
	ulong addr_last = addr_first + size - 1;

	/* round last address to the sector boundary */
	ಯದಿ (flash_sect_roundb(&addr_last) > 0)
		ಪ್ರತ್ಯಾಗಮ 1;

	ಯದಿ (addr_first >= addr_last) {
		printf("Error: end address exceeds addressing space\n");
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	/* remove protection on processed sectors */
	ಯದಿ (update_flash_protect(0, addr_first, addr_last) > 0) {
		printf("Error: could not unprotect flash sectors\n");
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	printf("Erasing 0x%08lx - 0x%08lx", addr_first, addr_last);
	ಯದಿ (flash_sect_erase(addr_first, addr_last) > 0) {
		printf("Error: could not erase flash\n");
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	printf("Copying to flash...");
	ಯದಿ (flash_write((ವರ್ಣ *)addr_source, addr_first, size) > 0) {
		printf("Error: could not copy to flash\n");
		ಪ್ರತ್ಯಾಗಮ 1;
	}
	printf("done\n");

	/* enable protection on processed sectors */
	ಯದಿ (update_flash_protect(1, addr_first, addr_last) > 0) {
		printf("Error: could not protect flash sectors\n");
		ಪ್ರತ್ಯಾಗಮ 1;
	}
#endif
	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif /* CONFIG_DFU_TFTP || CONFIG_UPDATE_TFTP */

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ update_fit_getparams(ಧ್ರುವ ಶೂನ್ಯ *fit, ಪೂರ್ಣಾಙ್ಕ noffset, ulong *addr,
						ulong *fladdr, ulong *size)
{
	ಧ್ರುವ ಶೂನ್ಯ *data;

	ಯದಿ (fit_image_get_data(fit, noffset, &data, (size_t *)size))
		ಪ್ರತ್ಯಾಗಮ 1;

	ಯದಿ (fit_image_get_load(fit, noffset, (ulong *)fladdr))
		ಪ್ರತ್ಯಾಗಮ 1;

	*addr = (ulong)data;

	ಪ್ರತ್ಯಾಗಮ 0;
}

#ಯದಿ defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
ಪೂರ್ಣಾಙ್ಕ update_tftp(ulong addr, ವರ್ಣ *interface, ವರ್ಣ *devstring)
{
	ವರ್ಣ *filename, *env_addr, *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
	ಪೂರ್ಣಾಙ್ಕ images_noffset, ndepth, noffset;
	bool update_tftp_dfu;
	ಪೂರ್ಣಾಙ್ಕ ret = 0;
	ಶೂನ್ಯ *fit;

	ಯದಿ (interface == ರಿಕ್ತ && devstring == ರಿಕ್ತ) {
		update_tftp_dfu = false;
	} ಅನ್ಯಥಾ ಯದಿ (interface && devstring) {
		update_tftp_dfu = true;
	} ಅನ್ಯಥಾ {
		pr_err("Interface: %s and devstring: %s not supported!\n",
		      interface, devstring);
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	/* use already present image */
	ಯದಿ (addr)
		ಗಚ್ಛ got_update_file;

	printf("Auto-update from TFTP: ");

	/* get the file name of the update file */
	filename = env_get(UPDATE_FILE_ENV);
	ಯದಿ (filename == ರಿಕ್ತ) {
		printf("failed, env. variable '%s' not found\n",
							UPDATE_FILE_ENV);
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	printf("trying update file '%s'\n", filename);

	/* get load address of downloaded update file */
	env_addr = env_get("loadaddr");
	ಯದಿ (env_addr)
		addr = hextoul(env_addr, ರಿಕ್ತ);
	ಅನ್ಯಥಾ
		addr = CONFIG_UPDATE_LOAD_ADDR;

	ಯದಿ (update_load(filename, CONFIG_UPDATE_TFTP_MSEC_MAX,
					CONFIG_UPDATE_TFTP_CNT_MAX, addr)) {
		printf("Can't load update file, aborting auto-update\n");
		ಪ್ರತ್ಯಾಗಮ 1;
	}

got_update_file:
	fit = map_sysmem(addr, 0);

	ಯದಿ (fit_check_format((ಶೂನ್ಯ *)fit, IMAGE_SIZE_INVAL)) {
		printf("Bad FIT format of the update file, aborting "
							"auto-update\n");
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_ಗಂಟು(fit, images_noffset, &ndepth);
	ಯಾವತ್ (noffset >= 0 && ndepth > 0) {
		ಯದಿ (ndepth != 1)
			ಗಚ್ಛ next_ಗಂಟು;

		fit_image_name = (ವರ್ಣ *)fit_get_name(fit, noffset, ರಿಕ್ತ);
		printf("Processing update '%s' :", fit_image_name);

		ಯದಿ (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			ಗಚ್ಛ next_ಗಂಟು;
		}

		printf("\n");
		ಯದಿ (update_fit_getparams(fit, noffset, &update_addr,
					&update_fladdr, &update_size)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			ಗಚ್ಛ next_ಗಂಟು;
		}

		ಯದಿ (!update_tftp_dfu) {
			ಯದಿ (update_flash(update_addr, update_fladdr,
					 update_size)) {
				printf("Error: can't flash update, aborting\n");
				ret = 1;
				ಗಚ್ಛ next_ಗಂಟು;
			}
		} ಅನ್ಯಥಾ ಯದಿ (fit_image_check_type(fit, noffset,
						IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(ಶೂನ್ಯ *)update_addr,
						update_size, interface,
						devstring);
			ಯದಿ (ret)
				ಪ್ರತ್ಯಾಗಮ ret;
		}
next_ಗಂಟು:
		noffset = fdt_next_ಗಂಟು(fit, noffset, &ndepth);
	}

	ಪ್ರತ್ಯಾಗಮ ret;
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
ಪೂರ್ಣಾಙ್ಕ fit_update(ಧ್ರುವ ಶೂನ್ಯ *fit)
{
	ವರ್ಣ *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
	ಪೂರ್ಣಾಙ್ಕ images_noffset, ndepth, noffset;
	ಪೂರ್ಣಾಙ್ಕ ret = 0;

	ಯದಿ (!fit)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	ಯದಿ (fit_check_format((ಶೂನ್ಯ *)fit, IMAGE_SIZE_INVAL)) {
		printf("Bad FIT format of the update file, aborting auto-update\n");
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_ಗಂಟು(fit, images_noffset, &ndepth);
	ಯಾವತ್ (noffset >= 0 && ndepth > 0) {
		ಯದಿ (ndepth != 1)
			ಗಚ್ಛ next_ಗಂಟು;

		fit_image_name = (ವರ್ಣ *)fit_get_name(fit, noffset, ರಿಕ್ತ);
		printf("Processing update '%s' :", fit_image_name);

		ಯದಿ (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			ಗಚ್ಛ next_ಗಂಟು;
		}

		printf("\n");
		ಯದಿ (update_fit_getparams(fit, noffset, &update_addr,
					 &update_fladdr, &update_size)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			ಗಚ್ಛ next_ಗಂಟು;
		}

		ಯದಿ (fit_image_check_type(fit, noffset, IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(ಶೂನ್ಯ *)update_addr,
						update_size, ರಿಕ್ತ, ರಿಕ್ತ);
			ಯದಿ (ret)
				ಪ್ರತ್ಯಾಗಮ ret;
		}
next_ಗಂಟು:
		noffset = fdt_next_ಗಂಟು(fit, noffset, &ndepth);
	}

	ಪ್ರತ್ಯಾಗಮ ret;
}
#endif /* CONFIG_UPDATE_FIT */
