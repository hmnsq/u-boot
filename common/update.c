// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Semihalf
 *
 * Written by: Rafal Czubak <rcz@semihalf.com>
 *             Bartlomiej Sieka <tur@semihalf.com>
 */

#ଅନ୍ତର୍ଭାଵ <cpu_func.h>
#ଅନ୍ତର୍ଭାଵ <image.h>
#ଅନ୍ତର୍ଭାଵ <linux/printk.h>

#ଅନ୍ତର୍ଭାଵ <command.h>
#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <net.h>
#ଅନ୍ତର୍ଭାଵ <net/tftp.h>
#ଅନ୍ତର୍ଭାଵ <malloc.h>
#ଅନ୍ତର୍ଭାଵ <mapmem.h>
#ଅନ୍ତର୍ଭାଵ <dfu.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>

#ଯଦି defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
/* env variable holding the location of the update file */
#define UPDATE_FILE_ENV		"updatefile"

ବାହ୍ୟ ulong tftp_timeout_ms;
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା tftp_timeout_ଗଣନା_max;
#ifdef CONFIG_MTD_NOR_FLASH
#ଅନ୍ତର୍ଭାଵ <flash.h>
#ଅନ୍ତର୍ଭାଵ <mtd/cfi_flash.h>
ସ୍ଥିର uchar *saved_prot_info;
#endif
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା update_load(ଅକ୍ଷର *filename, ulong msec_max, ପୂର୍ଣ୍ଣସଂଖ୍ୟା cnt_max, ulong addr)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rv, ret;
	ulong saved_timeout_msecs;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା saved_timeout_ଗଣନା;
	ଅକ୍ଷର *saved_netretry, *saved_bootfile;

	rv = 0;
	/* save used globals and env variable */
	saved_timeout_msecs = tftp_timeout_ms;
	saved_timeout_ଗଣନା = tftp_timeout_ଗଣନା_max;
	saved_netretry = strdup(env_get("netretry"));
	saved_bootfile = strdup(net_boot_file_name);

	/* set timeouts for auto-update */
	tftp_timeout_ms = msec_max;
	tftp_timeout_ଗଣନା_max = cnt_max;

	/* we don't want to retry the connection if errors occur */
	env_set("netretry", "no");

	/* download the update file */
	image_load_addr = addr;
	copy_filename(net_boot_file_name, filename, ଆକାର(net_boot_file_name));
	ret = net_loop(TFTPGET);

	ଯଦି (ret < 0)
		rv = 1;
	ଅନ୍ୟଥା
		flush_cache(addr, net_boot_file_size);

	/* restore changed globals and env variable */
	tftp_timeout_ms = saved_timeout_msecs;
	tftp_timeout_ଗଣନା_max = saved_timeout_ଗଣନା;

	env_set("netretry", saved_netretry);
	ଯଦି (saved_netretry != ଖାଲି)
		ମୁକ୍ତ(saved_netretry);

	ଯଦି (saved_bootfile != ଖାଲି) {
		copy_filename(net_boot_file_name, saved_bootfile,
			      ଆକାର(net_boot_file_name));
		ମୁକ୍ତ(saved_bootfile);
	}

	ଫେରନ୍ତୁ rv;
}

#ifdef CONFIG_MTD_NOR_FLASH
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା update_flash_protect(ପୂର୍ଣ୍ଣସଂଖ୍ୟା prot, ulong addr_first, ulong addr_last)
{
	uchar *sp_info_ptr;
	ulong s;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, bank, cnt;
	flash_info_t *info;

	sp_info_ptr = ଖାଲି;

	ଯଦି (prot == 0) {
		saved_prot_info =
			calloc(CFI_FLASH_BANKS * CONFIG_SYS_MAX_FLASH_SECT, 1);
		ଯଦି (!saved_prot_info)
			ଫେରନ୍ତୁ 1;
	}

	ପାଇଁ (bank = 0; bank < CFI_FLASH_BANKS; ++bank) {
		cnt = 0;
		info = &flash_info[bank];

		/* Nothing to do if the bank doesn't exist */
		ଯଦି (info->sector_ଗଣନା == 0)
			ଫେରନ୍ତୁ 0;

		/* Point to current bank protection information */
		sp_info_ptr = saved_prot_info + (bank * CONFIG_SYS_MAX_FLASH_SECT);

		/*
		 * Adjust addr_first or addr_last if we are on bank boundary.
		 * Address space between banks must be continuous for other
		 * flash functions (like flash_sect_erase or flash_write) to
		 * succeed. Banks must also be numbered in correct order,
		 * according to increasing addresses.
		 */
		ଯଦି (addr_last > info->start[0] + info->size - 1)
			addr_last = info->start[0] + info->size - 1;
		ଯଦି (addr_first < info->start[0])
			addr_first = info->start[0];

		ପାଇଁ (i = 0; i < info->sector_ଗଣନା; i++) {
			/* Save current information about protected sectors */
			ଯଦି (prot == 0) {
				s = info->start[i];
				ଯଦି ((s >= addr_first) && (s <= addr_last))
					sp_info_ptr[i] = info->protect[i];

			}

			/* Protect/unprotect sectors */
			ଯଦି (sp_info_ptr[i] == 1) {
#ଯଦି defined(CONFIG_SYS_FLASH_PROTECTION)
				ଯଦି (flash_real_protect(info, i, prot))
					ଫେରନ୍ତୁ 1;
#ଅନ୍ୟଥା
				info->protect[i] = prot;
#endif
				cnt++;
			}
		}

		ଯଦି (cnt) {
			printf("%sProtected %d sectors\n",
						prot ? "": "Un-", cnt);
		}
	}

	ଯଦି((prot == 1) && saved_prot_info)
		ମୁକ୍ତ(saved_prot_info);

	ଫେରନ୍ତୁ 0;
}
#endif

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା update_flash(ulong addr_source, ulong addr_first, ulong size)
{
#ifdef CONFIG_MTD_NOR_FLASH
	ulong addr_last = addr_first + size - 1;

	/* round last address to the sector boundary */
	ଯଦି (flash_sect_roundb(&addr_last) > 0)
		ଫେରନ୍ତୁ 1;

	ଯଦି (addr_first >= addr_last) {
		printf("Error: end address exceeds addressing space\n");
		ଫେରନ୍ତୁ 1;
	}

	/* remove protection on processed sectors */
	ଯଦି (update_flash_protect(0, addr_first, addr_last) > 0) {
		printf("Error: could not unprotect flash sectors\n");
		ଫେରନ୍ତୁ 1;
	}

	printf("Erasing 0x%08lx - 0x%08lx", addr_first, addr_last);
	ଯଦି (flash_sect_erase(addr_first, addr_last) > 0) {
		printf("Error: could not erase flash\n");
		ଫେରନ୍ତୁ 1;
	}

	printf("Copying to flash...");
	ଯଦି (flash_write((ଅକ୍ଷର *)addr_source, addr_first, size) > 0) {
		printf("Error: could not copy to flash\n");
		ଫେରନ୍ତୁ 1;
	}
	printf("done\n");

	/* enable protection on processed sectors */
	ଯଦି (update_flash_protect(1, addr_first, addr_last) > 0) {
		printf("Error: could not protect flash sectors\n");
		ଫେରନ୍ତୁ 1;
	}
#endif
	ଫେରନ୍ତୁ 0;
}
#endif /* CONFIG_DFU_TFTP || CONFIG_UPDATE_TFTP */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା update_fit_getparams(ଅଚଳ ଶୂନ୍ୟ *fit, ପୂର୍ଣ୍ଣସଂଖ୍ୟା noffset, ulong *addr,
						ulong *fladdr, ulong *size)
{
	ଅଚଳ ଶୂନ୍ୟ *data;

	ଯଦି (fit_image_get_data(fit, noffset, &data, (size_t *)size))
		ଫେରନ୍ତୁ 1;

	ଯଦି (fit_image_get_load(fit, noffset, (ulong *)fladdr))
		ଫେରନ୍ତୁ 1;

	*addr = (ulong)data;

	ଫେରନ୍ତୁ 0;
}

#ଯଦି defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
ପୂର୍ଣ୍ଣସଂଖ୍ୟା update_tftp(ulong addr, ଅକ୍ଷର *interface, ଅକ୍ଷର *devstring)
{
	ଅକ୍ଷର *filename, *env_addr, *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା images_noffset, ndepth, noffset;
	bool update_tftp_dfu;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;
	ଶୂନ୍ୟ *fit;

	ଯଦି (interface == ଖାଲି && devstring == ଖାଲି) {
		update_tftp_dfu = false;
	} ଅନ୍ୟଥା ଯଦି (interface && devstring) {
		update_tftp_dfu = true;
	} ଅନ୍ୟଥା {
		pr_err("Interface: %s and devstring: %s not supported!\n",
		      interface, devstring);
		ଫେରନ୍ତୁ -EINVAL;
	}

	/* use already present image */
	ଯଦି (addr)
		ଯାଅ got_update_file;

	printf("Auto-update from TFTP: ");

	/* get the file name of the update file */
	filename = env_get(UPDATE_FILE_ENV);
	ଯଦି (filename == ଖାଲି) {
		printf("failed, env. variable '%s' not found\n",
							UPDATE_FILE_ENV);
		ଫେରନ୍ତୁ 1;
	}

	printf("trying update file '%s'\n", filename);

	/* get load address of downloaded update file */
	env_addr = env_get("loadaddr");
	ଯଦି (env_addr)
		addr = hextoul(env_addr, ଖାଲି);
	ଅନ୍ୟଥା
		addr = CONFIG_UPDATE_LOAD_ADDR;

	ଯଦି (update_load(filename, CONFIG_UPDATE_TFTP_MSEC_MAX,
					CONFIG_UPDATE_TFTP_CNT_MAX, addr)) {
		printf("Can't load update file, aborting auto-update\n");
		ଫେରନ୍ତୁ 1;
	}

got_update_file:
	fit = map_sysmem(addr, 0);

	ଯଦି (fit_check_format((ଶୂନ୍ୟ *)fit, IMAGE_SIZE_INVAL)) {
		printf("Bad FIT format of the update file, aborting "
							"auto-update\n");
		ଫେରନ୍ତୁ 1;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_ଗଣ୍ଠି(fit, images_noffset, &ndepth);
	ଯେତେବେଳେ (noffset >= 0 && ndepth > 0) {
		ଯଦି (ndepth != 1)
			ଯାଅ next_ଗଣ୍ଠି;

		fit_image_name = (ଅକ୍ଷର *)fit_get_name(fit, noffset, ଖାଲି);
		printf("Processing update '%s' :", fit_image_name);

		ଯଦି (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			ଯାଅ next_ଗଣ୍ଠି;
		}

		printf("\n");
		ଯଦି (update_fit_getparams(fit, noffset, &update_addr,
					&update_fladdr, &update_size)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			ଯାଅ next_ଗଣ୍ଠି;
		}

		ଯଦି (!update_tftp_dfu) {
			ଯଦି (update_flash(update_addr, update_fladdr,
					 update_size)) {
				printf("Error: can't flash update, aborting\n");
				ret = 1;
				ଯାଅ next_ଗଣ୍ଠି;
			}
		} ଅନ୍ୟଥା ଯଦି (fit_image_check_type(fit, noffset,
						IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(ଶୂନ୍ୟ *)update_addr,
						update_size, interface,
						devstring);
			ଯଦି (ret)
				ଫେରନ୍ତୁ ret;
		}
next_ଗଣ୍ଠି:
		noffset = fdt_next_ଗଣ୍ଠି(fit, noffset, &ndepth);
	}

	ଫେରନ୍ତୁ ret;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା fit_update(ଅଚଳ ଶୂନ୍ୟ *fit)
{
	ଅକ୍ଷର *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା images_noffset, ndepth, noffset;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;

	ଯଦି (!fit)
		ଫେରନ୍ତୁ -EINVAL;

	ଯଦି (fit_check_format((ଶୂନ୍ୟ *)fit, IMAGE_SIZE_INVAL)) {
		printf("Bad FIT format of the update file, aborting auto-update\n");
		ଫେରନ୍ତୁ -EINVAL;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_ଗଣ୍ଠି(fit, images_noffset, &ndepth);
	ଯେତେବେଳେ (noffset >= 0 && ndepth > 0) {
		ଯଦି (ndepth != 1)
			ଯାଅ next_ଗଣ୍ଠି;

		fit_image_name = (ଅକ୍ଷର *)fit_get_name(fit, noffset, ଖାଲି);
		printf("Processing update '%s' :", fit_image_name);

		ଯଦି (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			ଯାଅ next_ଗଣ୍ଠି;
		}

		printf("\n");
		ଯଦି (update_fit_getparams(fit, noffset, &update_addr,
					 &update_fladdr, &update_size)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			ଯାଅ next_ଗଣ୍ଠି;
		}

		ଯଦି (fit_image_check_type(fit, noffset, IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(ଶୂନ୍ୟ *)update_addr,
						update_size, ଖାଲି, ଖାଲି);
			ଯଦି (ret)
				ଫେରନ୍ତୁ ret;
		}
next_ଗଣ୍ଠି:
		noffset = fdt_next_ଗଣ୍ଠି(fit, noffset, &ndepth);
	}

	ଫେରନ୍ତୁ ret;
}
#endif /* CONFIG_UPDATE_FIT */
