// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Semihalf
 *
 * Written by: Rafal Czubak <rcz@semihalf.com>
 *             Bartlomiej Sieka <tur@semihalf.com>
 */

#અન્તર્ભાવ <cpu_func.h>
#અન્તર્ભાવ <image.h>
#અન્તર્ભાવ <linux/printk.h>

#અન્તર્ભાવ <command.h>
#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <net.h>
#અન્તર્ભાવ <net/tftp.h>
#અન્તર્ભાવ <malloc.h>
#અન્તર્ભાવ <mapmem.h>
#અન્તર્ભાવ <dfu.h>
#અન્તર્ભાવ <errno.h>

#જો defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
/* env variable holding the location of the update file */
#define UPDATE_FILE_ENV		"updatefile"

બાહ્ય ulong tftp_timeout_ms;
બાહ્ય પૂર્ણાંક tftp_timeout_ગણતરી_max;
#ifdef CONFIG_MTD_NOR_FLASH
#અન્તર્ભાવ <flash.h>
#અન્તર્ભાવ <mtd/cfi_flash.h>
સ્થિર uchar *saved_prot_info;
#endif
સ્થિર પૂર્ણાંક update_load(અક્ષર *filename, ulong msec_max, પૂર્ણાંક cnt_max, ulong addr)
{
	પૂર્ણાંક rv, ret;
	ulong saved_timeout_msecs;
	પૂર્ણાંક saved_timeout_ગણતરી;
	અક્ષર *saved_netretry, *saved_bootfile;

	rv = 0;
	/* save used globals and env variable */
	saved_timeout_msecs = tftp_timeout_ms;
	saved_timeout_ગણતરી = tftp_timeout_ગણતરી_max;
	saved_netretry = strdup(env_get("netretry"));
	saved_bootfile = strdup(net_boot_file_name);

	/* set timeouts for auto-update */
	tftp_timeout_ms = msec_max;
	tftp_timeout_ગણતરી_max = cnt_max;

	/* we don't want to retry the connection if errors occur */
	env_set("netretry", "no");

	/* download the update file */
	image_load_addr = addr;
	copy_filename(net_boot_file_name, filename, કદ(net_boot_file_name));
	ret = net_loop(TFTPGET);

	જો (ret < 0)
		rv = 1;
	નહીંતર
		flush_cache(addr, net_boot_file_size);

	/* restore changed globals and env variable */
	tftp_timeout_ms = saved_timeout_msecs;
	tftp_timeout_ગણતરી_max = saved_timeout_ગણતરી;

	env_set("netretry", saved_netretry);
	જો (saved_netretry != ખાલી)
		મુક્ત(saved_netretry);

	જો (saved_bootfile != ખાલી) {
		copy_filename(net_boot_file_name, saved_bootfile,
			      કદ(net_boot_file_name));
		મુક્ત(saved_bootfile);
	}

	પાછા rv;
}

#ifdef CONFIG_MTD_NOR_FLASH
સ્થિર પૂર્ણાંક update_flash_protect(પૂર્ણાંક prot, ulong addr_first, ulong addr_last)
{
	uchar *sp_info_ptr;
	ulong s;
	પૂર્ણાંક i, bank, cnt;
	flash_info_t *info;

	sp_info_ptr = ખાલી;

	જો (prot == 0) {
		saved_prot_info =
			calloc(CFI_FLASH_BANKS * CONFIG_SYS_MAX_FLASH_SECT, 1);
		જો (!saved_prot_info)
			પાછા 1;
	}

	માટે (bank = 0; bank < CFI_FLASH_BANKS; ++bank) {
		cnt = 0;
		info = &flash_info[bank];

		/* Nothing to do if the bank doesn't exist */
		જો (info->sector_ગણતરી == 0)
			પાછા 0;

		/* Point to current bank protection information */
		sp_info_ptr = saved_prot_info + (bank * CONFIG_SYS_MAX_FLASH_SECT);

		/*
		 * Adjust addr_first or addr_last if we are on bank boundary.
		 * Address space between banks must be continuous for other
		 * flash functions (like flash_sect_erase or flash_write) to
		 * succeed. Banks must also be numbered in correct order,
		 * according to increasing addresses.
		 */
		જો (addr_last > info->start[0] + info->size - 1)
			addr_last = info->start[0] + info->size - 1;
		જો (addr_first < info->start[0])
			addr_first = info->start[0];

		માટે (i = 0; i < info->sector_ગણતરી; i++) {
			/* Save current information about protected sectors */
			જો (prot == 0) {
				s = info->start[i];
				જો ((s >= addr_first) && (s <= addr_last))
					sp_info_ptr[i] = info->protect[i];

			}

			/* Protect/unprotect sectors */
			જો (sp_info_ptr[i] == 1) {
#જો defined(CONFIG_SYS_FLASH_PROTECTION)
				જો (flash_real_protect(info, i, prot))
					પાછા 1;
#નહીંતર
				info->protect[i] = prot;
#endif
				cnt++;
			}
		}

		જો (cnt) {
			printf("%sProtected %d sectors\n",
						prot ? "": "Un-", cnt);
		}
	}

	જો((prot == 1) && saved_prot_info)
		મુક્ત(saved_prot_info);

	પાછા 0;
}
#endif

સ્થિર પૂર્ણાંક update_flash(ulong addr_source, ulong addr_first, ulong size)
{
#ifdef CONFIG_MTD_NOR_FLASH
	ulong addr_last = addr_first + size - 1;

	/* round last address to the sector boundary */
	જો (flash_sect_roundb(&addr_last) > 0)
		પાછા 1;

	જો (addr_first >= addr_last) {
		printf("Error: end address exceeds addressing space\n");
		પાછા 1;
	}

	/* remove protection on processed sectors */
	જો (update_flash_protect(0, addr_first, addr_last) > 0) {
		printf("Error: could not unprotect flash sectors\n");
		પાછા 1;
	}

	printf("Erasing 0x%08lx - 0x%08lx", addr_first, addr_last);
	જો (flash_sect_erase(addr_first, addr_last) > 0) {
		printf("Error: could not erase flash\n");
		પાછા 1;
	}

	printf("Copying to flash...");
	જો (flash_write((અક્ષર *)addr_source, addr_first, size) > 0) {
		printf("Error: could not copy to flash\n");
		પાછા 1;
	}
	printf("done\n");

	/* enable protection on processed sectors */
	જો (update_flash_protect(1, addr_first, addr_last) > 0) {
		printf("Error: could not protect flash sectors\n");
		પાછા 1;
	}
#endif
	પાછા 0;
}
#endif /* CONFIG_DFU_TFTP || CONFIG_UPDATE_TFTP */

સ્થિર પૂર્ણાંક update_fit_getparams(અચલ શૂન્ય *fit, પૂર્ણાંક noffset, ulong *addr,
						ulong *fladdr, ulong *size)
{
	અચલ શૂન્ય *data;

	જો (fit_image_get_data(fit, noffset, &data, (size_t *)size))
		પાછા 1;

	જો (fit_image_get_load(fit, noffset, (ulong *)fladdr))
		પાછા 1;

	*addr = (ulong)data;

	પાછા 0;
}

#જો defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
પૂર્ણાંક update_tftp(ulong addr, અક્ષર *interface, અક્ષર *devstring)
{
	અક્ષર *filename, *env_addr, *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
	પૂર્ણાંક images_noffset, ndepth, noffset;
	bool update_tftp_dfu;
	પૂર્ણાંક ret = 0;
	શૂન્ય *fit;

	જો (interface == ખાલી && devstring == ખાલી) {
		update_tftp_dfu = false;
	} નહીંતર જો (interface && devstring) {
		update_tftp_dfu = true;
	} નહીંતર {
		pr_err("Interface: %s and devstring: %s not supported!\n",
		      interface, devstring);
		પાછા -EINVAL;
	}

	/* use already present image */
	જો (addr)
		જાઓ got_update_file;

	printf("Auto-update from TFTP: ");

	/* get the file name of the update file */
	filename = env_get(UPDATE_FILE_ENV);
	જો (filename == ખાલી) {
		printf("failed, env. variable '%s' not found\n",
							UPDATE_FILE_ENV);
		પાછા 1;
	}

	printf("trying update file '%s'\n", filename);

	/* get load address of downloaded update file */
	env_addr = env_get("loadaddr");
	જો (env_addr)
		addr = hextoul(env_addr, ખાલી);
	નહીંતર
		addr = CONFIG_UPDATE_LOAD_ADDR;

	જો (update_load(filename, CONFIG_UPDATE_TFTP_MSEC_MAX,
					CONFIG_UPDATE_TFTP_CNT_MAX, addr)) {
		printf("Can't load update file, aborting auto-update\n");
		પાછા 1;
	}

got_update_file:
	fit = map_sysmem(addr, 0);

	જો (fit_check_format((શૂન્ય *)fit, IMAGE_SIZE_INVAL)) {
		printf("Bad FIT format of the update file, aborting "
							"auto-update\n");
		પાછા 1;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_ગાંઠ(fit, images_noffset, &ndepth);
	જ્યારે (noffset >= 0 && ndepth > 0) {
		જો (ndepth != 1)
			જાઓ next_ગાંઠ;

		fit_image_name = (અક્ષર *)fit_get_name(fit, noffset, ખાલી);
		printf("Processing update '%s' :", fit_image_name);

		જો (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			જાઓ next_ગાંઠ;
		}

		printf("\n");
		જો (update_fit_getparams(fit, noffset, &update_addr,
					&update_fladdr, &update_size)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			જાઓ next_ગાંઠ;
		}

		જો (!update_tftp_dfu) {
			જો (update_flash(update_addr, update_fladdr,
					 update_size)) {
				printf("Error: can't flash update, aborting\n");
				ret = 1;
				જાઓ next_ગાંઠ;
			}
		} નહીંતર જો (fit_image_check_type(fit, noffset,
						IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(શૂન્ય *)update_addr,
						update_size, interface,
						devstring);
			જો (ret)
				પાછા ret;
		}
next_ગાંઠ:
		noffset = fdt_next_ગાંઠ(fit, noffset, &ndepth);
	}

	પાછા ret;
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
પૂર્ણાંક fit_update(અચલ શૂન્ય *fit)
{
	અક્ષર *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
	પૂર્ણાંક images_noffset, ndepth, noffset;
	પૂર્ણાંક ret = 0;

	જો (!fit)
		પાછા -EINVAL;

	જો (fit_check_format((શૂન્ય *)fit, IMAGE_SIZE_INVAL)) {
		printf("Bad FIT format of the update file, aborting auto-update\n");
		પાછા -EINVAL;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_ગાંઠ(fit, images_noffset, &ndepth);
	જ્યારે (noffset >= 0 && ndepth > 0) {
		જો (ndepth != 1)
			જાઓ next_ગાંઠ;

		fit_image_name = (અક્ષર *)fit_get_name(fit, noffset, ખાલી);
		printf("Processing update '%s' :", fit_image_name);

		જો (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			જાઓ next_ગાંઠ;
		}

		printf("\n");
		જો (update_fit_getparams(fit, noffset, &update_addr,
					 &update_fladdr, &update_size)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			જાઓ next_ગાંઠ;
		}

		જો (fit_image_check_type(fit, noffset, IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(શૂન્ય *)update_addr,
						update_size, ખાલી, ખાલી);
			જો (ret)
				પાછા ret;
		}
next_ગાંઠ:
		noffset = fdt_next_ગાંઠ(fit, noffset, &ndepth);
	}

	પાછા ret;
}
#endif /* CONFIG_UPDATE_FIT */
