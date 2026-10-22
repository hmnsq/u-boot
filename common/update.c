// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Semihalf
 *
 * Written by: Rafal Czubak <rcz@semihalf.com>
 *             Bartlomiej Sieka <tur@semihalf.com>
 */

#അന്തര്ഭാവ <cpu_func.h>
#അന്തര്ഭാവ <image.h>
#അന്തര്ഭാവ <linux/printk.h>

#അന്തര്ഭാവ <command.h>
#അന്തര്ഭാവ <env.h>
#അന്തര്ഭാവ <net.h>
#അന്തര്ഭാവ <net/tftp.h>
#അന്തര്ഭാവ <malloc.h>
#അന്തര്ഭാവ <mapmem.h>
#അന്തര്ഭാവ <dfu.h>
#അന്തര്ഭാവ <errno.h>

#എങ്കിൽ defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
/* env variable holding the location of the update file */
#define UPDATE_FILE_ENV		"updatefile"

ബാഹ്യ ulong tftp_timeout_ms;
ബാഹ്യ പൂർണ്ണസംഖ്യ tftp_timeout_എണ്ണം_max;
#ifdef CONFIG_MTD_NOR_FLASH
#അന്തര്ഭാവ <flash.h>
#അന്തര്ഭാവ <mtd/cfi_flash.h>
സ്ഥിതം uchar *saved_prot_info;
#endif
സ്ഥിതം പൂർണ്ണസംഖ്യ update_load(അക്ഷരം *filename, ulong msec_max, പൂർണ്ണസംഖ്യ cnt_max, ulong addr)
{
	പൂർണ്ണസംഖ്യ rv, ret;
	ulong saved_timeout_msecs;
	പൂർണ്ണസംഖ്യ saved_timeout_എണ്ണം;
	അക്ഷരം *saved_netretry, *saved_bootfile;

	rv = 0;
	/* save used globals and env variable */
	saved_timeout_msecs = tftp_timeout_ms;
	saved_timeout_എണ്ണം = tftp_timeout_എണ്ണം_max;
	saved_netretry = strdup(env_get("netretry"));
	saved_bootfile = strdup(net_boot_file_name);

	/* set timeouts for auto-update */
	tftp_timeout_ms = msec_max;
	tftp_timeout_എണ്ണം_max = cnt_max;

	/* we don't want to retry the connection if errors occur */
	env_set("netretry", "no");

	/* download the update file */
	image_load_addr = addr;
	copy_filename(net_boot_file_name, filename, വലുപ്പം(net_boot_file_name));
	ret = net_loop(TFTPGET);

	എങ്കിൽ (ret < 0)
		rv = 1;
	അല്ലെങ്കിൽ
		flush_cache(addr, net_boot_file_size);

	/* restore changed globals and env variable */
	tftp_timeout_ms = saved_timeout_msecs;
	tftp_timeout_എണ്ണം_max = saved_timeout_എണ്ണം;

	env_set("netretry", saved_netretry);
	എങ്കിൽ (saved_netretry != ശൂന്യ)
		സ്വതന്ത്രം(saved_netretry);

	എങ്കിൽ (saved_bootfile != ശൂന്യ) {
		copy_filename(net_boot_file_name, saved_bootfile,
			      വലുപ്പം(net_boot_file_name));
		സ്വതന്ത്രം(saved_bootfile);
	}

	തിരികെ rv;
}

#ifdef CONFIG_MTD_NOR_FLASH
സ്ഥിതം പൂർണ്ണസംഖ്യ update_flash_protect(പൂർണ്ണസംഖ്യ prot, ulong addr_first, ulong addr_last)
{
	uchar *sp_info_ptr;
	ulong s;
	പൂർണ്ണസംഖ്യ i, bank, cnt;
	flash_info_t *info;

	sp_info_ptr = ശൂന്യ;

	എങ്കിൽ (prot == 0) {
		saved_prot_info =
			calloc(CFI_FLASH_BANKS * CONFIG_SYS_MAX_FLASH_SECT, 1);
		എങ്കിൽ (!saved_prot_info)
			തിരികെ 1;
	}

	വേണ്ടി (bank = 0; bank < CFI_FLASH_BANKS; ++bank) {
		cnt = 0;
		info = &flash_info[bank];

		/* Nothing to do if the bank doesn't exist */
		എങ്കിൽ (info->sector_എണ്ണം == 0)
			തിരികെ 0;

		/* Point to current bank protection information */
		sp_info_ptr = saved_prot_info + (bank * CONFIG_SYS_MAX_FLASH_SECT);

		/*
		 * Adjust addr_first or addr_last if we are on bank boundary.
		 * Address space between banks must be continuous for other
		 * flash functions (like flash_sect_erase or flash_write) to
		 * succeed. Banks must also be numbered in correct order,
		 * according to increasing addresses.
		 */
		എങ്കിൽ (addr_last > info->start[0] + info->size - 1)
			addr_last = info->start[0] + info->size - 1;
		എങ്കിൽ (addr_first < info->start[0])
			addr_first = info->start[0];

		വേണ്ടി (i = 0; i < info->sector_എണ്ണം; i++) {
			/* Save current information about protected sectors */
			എങ്കിൽ (prot == 0) {
				s = info->start[i];
				എങ്കിൽ ((s >= addr_first) && (s <= addr_last))
					sp_info_ptr[i] = info->protect[i];

			}

			/* Protect/unprotect sectors */
			എങ്കിൽ (sp_info_ptr[i] == 1) {
#എങ്കിൽ defined(CONFIG_SYS_FLASH_PROTECTION)
				എങ്കിൽ (flash_real_protect(info, i, prot))
					തിരികെ 1;
#അല്ലെങ്കിൽ
				info->protect[i] = prot;
#endif
				cnt++;
			}
		}

		എങ്കിൽ (cnt) {
			printf("%sProtected %d sectors\n",
						prot ? "": "Un-", cnt);
		}
	}

	എങ്കിൽ((prot == 1) && saved_prot_info)
		സ്വതന്ത്രം(saved_prot_info);

	തിരികെ 0;
}
#endif

സ്ഥിതം പൂർണ്ണസംഖ്യ update_flash(ulong addr_source, ulong addr_first, ulong size)
{
#ifdef CONFIG_MTD_NOR_FLASH
	ulong addr_last = addr_first + size - 1;

	/* round last address to the sector boundary */
	എങ്കിൽ (flash_sect_roundb(&addr_last) > 0)
		തിരികെ 1;

	എങ്കിൽ (addr_first >= addr_last) {
		printf("Error: end address exceeds addressing space\n");
		തിരികെ 1;
	}

	/* remove protection on processed sectors */
	എങ്കിൽ (update_flash_protect(0, addr_first, addr_last) > 0) {
		printf("Error: could not unprotect flash sectors\n");
		തിരികെ 1;
	}

	printf("Erasing 0x%08lx - 0x%08lx", addr_first, addr_last);
	എങ്കിൽ (flash_sect_erase(addr_first, addr_last) > 0) {
		printf("Error: could not erase flash\n");
		തിരികെ 1;
	}

	printf("Copying to flash...");
	എങ്കിൽ (flash_write((അക്ഷരം *)addr_source, addr_first, size) > 0) {
		printf("Error: could not copy to flash\n");
		തിരികെ 1;
	}
	printf("done\n");

	/* enable protection on processed sectors */
	എങ്കിൽ (update_flash_protect(1, addr_first, addr_last) > 0) {
		printf("Error: could not protect flash sectors\n");
		തിരികെ 1;
	}
#endif
	തിരികെ 0;
}
#endif /* CONFIG_DFU_TFTP || CONFIG_UPDATE_TFTP */

സ്ഥിതം പൂർണ്ണസംഖ്യ update_fit_getparams(സ്ഥിരം ശൂന്യം *fit, പൂർണ്ണസംഖ്യ noffset, ulong *addr,
						ulong *fladdr, ulong *size)
{
	സ്ഥിരം ശൂന്യം *data;

	എങ്കിൽ (fit_image_get_data(fit, noffset, &data, (size_t *)size))
		തിരികെ 1;

	എങ്കിൽ (fit_image_get_load(fit, noffset, (ulong *)fladdr))
		തിരികെ 1;

	*addr = (ulong)data;

	തിരികെ 0;
}

#എങ്കിൽ defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
പൂർണ്ണസംഖ്യ update_tftp(ulong addr, അക്ഷരം *interface, അക്ഷരം *devstring)
{
	അക്ഷരം *filename, *env_addr, *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
	പൂർണ്ണസംഖ്യ images_noffset, ndepth, noffset;
	bool update_tftp_dfu;
	പൂർണ്ണസംഖ്യ ret = 0;
	ശൂന്യം *fit;

	എങ്കിൽ (interface == ശൂന്യ && devstring == ശൂന്യ) {
		update_tftp_dfu = false;
	} അല്ലെങ്കിൽ എങ്കിൽ (interface && devstring) {
		update_tftp_dfu = true;
	} അല്ലെങ്കിൽ {
		pr_err("Interface: %s and devstring: %s not supported!\n",
		      interface, devstring);
		തിരികെ -EINVAL;
	}

	/* use already present image */
	എങ്കിൽ (addr)
		പോകുക got_update_file;

	printf("Auto-update from TFTP: ");

	/* get the file name of the update file */
	filename = env_get(UPDATE_FILE_ENV);
	എങ്കിൽ (filename == ശൂന്യ) {
		printf("failed, env. variable '%s' not found\n",
							UPDATE_FILE_ENV);
		തിരികെ 1;
	}

	printf("trying update file '%s'\n", filename);

	/* get load address of downloaded update file */
	env_addr = env_get("loadaddr");
	എങ്കിൽ (env_addr)
		addr = hextoul(env_addr, ശൂന്യ);
	അല്ലെങ്കിൽ
		addr = CONFIG_UPDATE_LOAD_ADDR;

	എങ്കിൽ (update_load(filename, CONFIG_UPDATE_TFTP_MSEC_MAX,
					CONFIG_UPDATE_TFTP_CNT_MAX, addr)) {
		printf("Can't load update file, aborting auto-update\n");
		തിരികെ 1;
	}

got_update_file:
	fit = map_sysmem(addr, 0);

	എങ്കിൽ (fit_check_format((ശൂന്യം *)fit, IMAGE_SIZE_INVAL)) {
		printf("Bad FIT format of the update file, aborting "
							"auto-update\n");
		തിരികെ 1;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_കണ്ണി(fit, images_noffset, &ndepth);
	ആകുമ്പോൾ (noffset >= 0 && ndepth > 0) {
		എങ്കിൽ (ndepth != 1)
			പോകുക next_കണ്ണി;

		fit_image_name = (അക്ഷരം *)fit_get_name(fit, noffset, ശൂന്യ);
		printf("Processing update '%s' :", fit_image_name);

		എങ്കിൽ (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			പോകുക next_കണ്ണി;
		}

		printf("\n");
		എങ്കിൽ (update_fit_getparams(fit, noffset, &update_addr,
					&update_fladdr, &update_size)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			പോകുക next_കണ്ണി;
		}

		എങ്കിൽ (!update_tftp_dfu) {
			എങ്കിൽ (update_flash(update_addr, update_fladdr,
					 update_size)) {
				printf("Error: can't flash update, aborting\n");
				ret = 1;
				പോകുക next_കണ്ണി;
			}
		} അല്ലെങ്കിൽ എങ്കിൽ (fit_image_check_type(fit, noffset,
						IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(ശൂന്യം *)update_addr,
						update_size, interface,
						devstring);
			എങ്കിൽ (ret)
				തിരികെ ret;
		}
next_കണ്ണി:
		noffset = fdt_next_കണ്ണി(fit, noffset, &ndepth);
	}

	തിരികെ ret;
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
പൂർണ്ണസംഖ്യ fit_update(സ്ഥിരം ശൂന്യം *fit)
{
	അക്ഷരം *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
	പൂർണ്ണസംഖ്യ images_noffset, ndepth, noffset;
	പൂർണ്ണസംഖ്യ ret = 0;

	എങ്കിൽ (!fit)
		തിരികെ -EINVAL;

	എങ്കിൽ (fit_check_format((ശൂന്യം *)fit, IMAGE_SIZE_INVAL)) {
		printf("Bad FIT format of the update file, aborting auto-update\n");
		തിരികെ -EINVAL;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_കണ്ണി(fit, images_noffset, &ndepth);
	ആകുമ്പോൾ (noffset >= 0 && ndepth > 0) {
		എങ്കിൽ (ndepth != 1)
			പോകുക next_കണ്ണി;

		fit_image_name = (അക്ഷരം *)fit_get_name(fit, noffset, ശൂന്യ);
		printf("Processing update '%s' :", fit_image_name);

		എങ്കിൽ (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			പോകുക next_കണ്ണി;
		}

		printf("\n");
		എങ്കിൽ (update_fit_getparams(fit, noffset, &update_addr,
					 &update_fladdr, &update_size)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			പോകുക next_കണ്ണി;
		}

		എങ്കിൽ (fit_image_check_type(fit, noffset, IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(ശൂന്യം *)update_addr,
						update_size, ശൂന്യ, ശൂന്യ);
			എങ്കിൽ (ret)
				തിരികെ ret;
		}
next_കണ്ണി:
		noffset = fdt_next_കണ്ണി(fit, noffset, &ndepth);
	}

	തിരികെ ret;
}
#endif /* CONFIG_UPDATE_FIT */
