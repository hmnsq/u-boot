// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Semihalf
 *
 * Written by: Rafal Czubak <rcz@semihalf.com>
 *             Bartlomiej Sieka <tur@semihalf.com>
 */

#অন্তর্ভাব <cpu_func.h>
#অন্তর্ভাব <image.h>
#অন্তর্ভাব <linux/printk.h>

#অন্তর্ভাব <command.h>
#অন্তর্ভাব <env.h>
#অন্তর্ভাব <net.h>
#অন্তর্ভাব <net/tftp.h>
#অন্তর্ভাব <malloc.h>
#অন্তর্ভাব <mapmem.h>
#অন্তর্ভাব <dfu.h>
#অন্তর্ভাব <errno.h>

#যদি defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
/* env variable holding the location of the update file */
#define UPDATE_FILE_ENV		"updatefile"

বাহ্যিক ulong tftp_timeout_ms;
বাহ্যিক পূর্ণসংখ্যা tftp_timeout_গুনতি_max;
#ifdef CONFIG_MTD_NOR_FLASH
#অন্তর্ভাব <flash.h>
#অন্তর্ভাব <mtd/cfi_flash.h>
স্থির uchar *saved_prot_info;
#endif
স্থির পূর্ণসংখ্যা update_load(অক্ষর *filename, ulong msec_max, পূর্ণসংখ্যা cnt_max, ulong addr)
{
	পূর্ণসংখ্যা rv, ret;
	ulong saved_timeout_msecs;
	পূর্ণসংখ্যা saved_timeout_গুনতি;
	অক্ষর *saved_netretry, *saved_bootfile;

	rv = 0;
	/* save used globals and env variable */
	saved_timeout_msecs = tftp_timeout_ms;
	saved_timeout_গুনতি = tftp_timeout_গুনতি_max;
	saved_netretry = strdup(env_get("netretry"));
	saved_bootfile = strdup(net_boot_file_name);

	/* set timeouts for auto-update */
	tftp_timeout_ms = msec_max;
	tftp_timeout_গুনতি_max = cnt_max;

	/* we don't want to retry the connection if errors occur */
	env_set("netretry", "no");

	/* download the update file */
	image_load_addr = addr;
	copy_filename(net_boot_file_name, filename, আকার(net_boot_file_name));
	ret = net_loop(TFTPGET);

	যদি (ret < 0)
		rv = 1;
	অন্যথা
		flush_cache(addr, net_boot_file_আকার);

	/* restore changed globals and env variable */
	tftp_timeout_ms = saved_timeout_msecs;
	tftp_timeout_গুনতি_max = saved_timeout_গুনতি;

	env_set("netretry", saved_netretry);
	যদি (saved_netretry != ফাঁকা)
		মুক্ত(saved_netretry);

	যদি (saved_bootfile != ফাঁকা) {
		copy_filename(net_boot_file_name, saved_bootfile,
			      আকার(net_boot_file_name));
		মুক্ত(saved_bootfile);
	}

	ফেরত rv;
}

#ifdef CONFIG_MTD_NOR_FLASH
স্থির পূর্ণসংখ্যা update_flash_protect(পূর্ণসংখ্যা prot, ulong addr_first, ulong addr_last)
{
	uchar *sp_info_ptr;
	ulong s;
	পূর্ণসংখ্যা i, bank, cnt;
	flash_info_t *info;

	sp_info_ptr = ফাঁকা;

	যদি (prot == 0) {
		saved_prot_info =
			calloc(CFI_FLASH_BANKS * CONFIG_SYS_MAX_FLASH_SECT, 1);
		যদি (!saved_prot_info)
			ফেরত 1;
	}

	জন্য (bank = 0; bank < CFI_FLASH_BANKS; ++bank) {
		cnt = 0;
		info = &flash_info[bank];

		/* Nothing to do if the bank doesn't exist */
		যদি (info->sector_গুনতি == 0)
			ফেরত 0;

		/* Point to current bank protection information */
		sp_info_ptr = saved_prot_info + (bank * CONFIG_SYS_MAX_FLASH_SECT);

		/*
		 * Adjust addr_first or addr_last if we are on bank boundary.
		 * Address space between banks must be continuous for other
		 * flash functions (like flash_sect_erase or flash_write) to
		 * succeed. Banks must also be numbered in correct order,
		 * according to increasing addresses.
		 */
		যদি (addr_last > info->start[0] + info->আকার_2 - 1)
			addr_last = info->start[0] + info->আকার_2 - 1;
		যদি (addr_first < info->start[0])
			addr_first = info->start[0];

		জন্য (i = 0; i < info->sector_গুনতি; i++) {
			/* Save current information about protected sectors */
			যদি (prot == 0) {
				s = info->start[i];
				যদি ((s >= addr_first) && (s <= addr_last))
					sp_info_ptr[i] = info->protect[i];

			}

			/* Protect/unprotect sectors */
			যদি (sp_info_ptr[i] == 1) {
#যদি defined(CONFIG_SYS_FLASH_PROTECTION)
				যদি (flash_real_protect(info, i, prot))
					ফেরত 1;
#অন্যথা
				info->protect[i] = prot;
#endif
				cnt++;
			}
		}

		যদি (cnt) {
			printf("%sProtected %d sectors\n",
						prot ? "": "Un-", cnt);
		}
	}

	যদি((prot == 1) && saved_prot_info)
		মুক্ত(saved_prot_info);

	ফেরত 0;
}
#endif

স্থির পূর্ণসংখ্যা update_flash(ulong addr_source, ulong addr_first, ulong আকার_2)
{
#ifdef CONFIG_MTD_NOR_FLASH
	ulong addr_last = addr_first + আকার_2 - 1;

	/* round last address to the sector boundary */
	যদি (flash_sect_roundb(&addr_last) > 0)
		ফেরত 1;

	যদি (addr_first >= addr_last) {
		printf("Error: end address exceeds addressing space\n");
		ফেরত 1;
	}

	/* remove protection on processed sectors */
	যদি (update_flash_protect(0, addr_first, addr_last) > 0) {
		printf("Error: could not unprotect flash sectors\n");
		ফেরত 1;
	}

	printf("Erasing 0x%08lx - 0x%08lx", addr_first, addr_last);
	যদি (flash_sect_erase(addr_first, addr_last) > 0) {
		printf("Error: could not erase flash\n");
		ফেরত 1;
	}

	printf("Copying to flash...");
	যদি (flash_write((অক্ষর *)addr_source, addr_first, আকার_2) > 0) {
		printf("Error: could not copy to flash\n");
		ফেরত 1;
	}
	printf("done\n");

	/* enable protection on processed sectors */
	যদি (update_flash_protect(1, addr_first, addr_last) > 0) {
		printf("Error: could not protect flash sectors\n");
		ফেরত 1;
	}
#endif
	ফেরত 0;
}
#endif /* CONFIG_DFU_TFTP || CONFIG_UPDATE_TFTP */

স্থির পূর্ণসংখ্যা update_fit_getparams(ধ্রুবক শূন্য *fit, পূর্ণসংখ্যা noffset, ulong *addr,
						ulong *fladdr, ulong *আকার_2)
{
	ধ্রুবক শূন্য *তথ্য;

	যদি (fit_image_get_তথ্য(fit, noffset, &তথ্য, (size_t *)আকার_2))
		ফেরত 1;

	যদি (fit_image_get_load(fit, noffset, (ulong *)fladdr))
		ফেরত 1;

	*addr = (ulong)তথ্য;

	ফেরত 0;
}

#যদি defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
পূর্ণসংখ্যা update_tftp(ulong addr, অক্ষর *interface, অক্ষর *devstring)
{
	অক্ষর *filename, *env_addr, *fit_image_name;
	ulong update_addr, update_fladdr, update_আকার;
	পূর্ণসংখ্যা images_noffset, ndepth, noffset;
	bool update_tftp_dfu;
	পূর্ণসংখ্যা ret = 0;
	শূন্য *fit;

	যদি (interface == ফাঁকা && devstring == ফাঁকা) {
		update_tftp_dfu = false;
	} অন্যথা যদি (interface && devstring) {
		update_tftp_dfu = true;
	} অন্যথা {
		pr_err("Interface: %s and devstring: %s not supported!\n",
		      interface, devstring);
		ফেরত -EINVAL;
	}

	/* use already present image */
	যদি (addr)
		যাও got_update_file;

	printf("Auto-update from TFTP: ");

	/* get the file name of the update file */
	filename = env_get(UPDATE_FILE_ENV);
	যদি (filename == ফাঁকা) {
		printf("failed, env. variable '%s' not found\n",
							UPDATE_FILE_ENV);
		ফেরত 1;
	}

	printf("trying update file '%s'\n", filename);

	/* get load address of downloaded update file */
	env_addr = env_get("loadaddr");
	যদি (env_addr)
		addr = hextoul(env_addr, ফাঁকা);
	অন্যথা
		addr = CONFIG_UPDATE_LOAD_ADDR;

	যদি (update_load(filename, CONFIG_UPDATE_TFTP_MSEC_MAX,
					CONFIG_UPDATE_TFTP_CNT_MAX, addr)) {
		printf("Can't load update file, aborting auto-update\n");
		ফেরত 1;
	}

got_update_file:
	fit = map_sysmem(addr, 0);

	যদি (fit_check_format((শূন্য *)fit, IMAGE_আকার_INVAL)) {
		printf("Bad FIT format of the update file, aborting "
							"auto-update\n");
		ফেরত 1;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_পরবর্তী_গ্রন্থি(fit, images_noffset, &ndepth);
	যখন (noffset >= 0 && ndepth > 0) {
		যদি (ndepth != 1)
			যাও পরবর্তী_গ্রন্থি;

		fit_image_name = (অক্ষর *)fit_get_name(fit, noffset, ফাঁকা);
		printf("Processing update '%s' :", fit_image_name);

		যদি (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			যাও পরবর্তী_গ্রন্থি;
		}

		printf("\n");
		যদি (update_fit_getparams(fit, noffset, &update_addr,
					&update_fladdr, &update_আকার)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			যাও পরবর্তী_গ্রন্থি;
		}

		যদি (!update_tftp_dfu) {
			যদি (update_flash(update_addr, update_fladdr,
					 update_আকার)) {
				printf("Error: can't flash update, aborting\n");
				ret = 1;
				যাও পরবর্তী_গ্রন্থি;
			}
		} অন্যথা যদি (fit_image_check_type(fit, noffset,
						IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(শূন্য *)update_addr,
						update_আকার, interface,
						devstring);
			যদি (ret)
				ফেরত ret;
		}
পরবর্তী_গ্রন্থি:
		noffset = fdt_পরবর্তী_গ্রন্থি(fit, noffset, &ndepth);
	}

	ফেরত ret;
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
পূর্ণসংখ্যা fit_update(ধ্রুবক শূন্য *fit)
{
	অক্ষর *fit_image_name;
	ulong update_addr, update_fladdr, update_আকার;
	পূর্ণসংখ্যা images_noffset, ndepth, noffset;
	পূর্ণসংখ্যা ret = 0;

	যদি (!fit)
		ফেরত -EINVAL;

	যদি (fit_check_format((শূন্য *)fit, IMAGE_আকার_INVAL)) {
		printf("Bad FIT format of the update file, aborting auto-update\n");
		ফেরত -EINVAL;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_পরবর্তী_গ্রন্থি(fit, images_noffset, &ndepth);
	যখন (noffset >= 0 && ndepth > 0) {
		যদি (ndepth != 1)
			যাও পরবর্তী_গ্রন্থি;

		fit_image_name = (অক্ষর *)fit_get_name(fit, noffset, ফাঁকা);
		printf("Processing update '%s' :", fit_image_name);

		যদি (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			যাও পরবর্তী_গ্রন্থি;
		}

		printf("\n");
		যদি (update_fit_getparams(fit, noffset, &update_addr,
					 &update_fladdr, &update_আকার)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			যাও পরবর্তী_গ্রন্থি;
		}

		যদি (fit_image_check_type(fit, noffset, IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(শূন্য *)update_addr,
						update_আকার, ফাঁকা, ফাঁকা);
			যদি (ret)
				ফেরত ret;
		}
পরবর্তী_গ্রন্থি:
		noffset = fdt_পরবর্তী_গ্রন্থি(fit, noffset, &ndepth);
	}

	ফেরত ret;
}
#endif /* CONFIG_UPDATE_FIT */
