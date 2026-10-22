// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Semihalf
 *
 * Written by: Rafal Czubak <rcz@semihalf.com>
 *             Bartlomiej Sieka <tur@semihalf.com>
 */

#అన్తర్భావ <cpu_func.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <linux/printk.h>

#అన్తర్భావ <command.h>
#అన్తర్భావ <env.h>
#అన్తర్భావ <net.h>
#అన్తర్భావ <net/tftp.h>
#అన్తర్భావ <malloc.h>
#అన్తర్భావ <mapmem.h>
#అన్తర్భావ <dfu.h>
#అన్తర్భావ <errno.h>

#ఒకవేళ defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
/* env variable holding the location of the update file */
#నిర్వచించు UPDATE_FILE_ENV		"updatefile"

బాహ్య ulong tftp_timeout_ms;
బాహ్య పూర్ణాంకం tftp_timeout_లెక్క_max;
#నిర్వచితమైతే CONFIG_MTD_NOR_FLASH
#అన్తర్భావ <flash.h>
#అన్తర్భావ <mtd/cfi_flash.h>
స్థిర uchar *saved_prot_info;
#అయితే_ముగింపు
స్థిర పూర్ణాంకం update_ఎక్కించు(అక్షరం *filename, ulong msec_max, పూర్ణాంకం cnt_max, ulong addr)
{
	పూర్ణాంకం rv, ret;
	ulong saved_timeout_msecs;
	పూర్ణాంకం saved_timeout_లెక్క;
	అక్షరం *saved_netretry, *saved_bootfile;

	rv = 0;
	/* save used globals and env variable */
	saved_timeout_msecs = tftp_timeout_ms;
	saved_timeout_లెక్క = tftp_timeout_లెక్క_max;
	saved_netretry = strdup(env_get("netretry"));
	saved_bootfile = strdup(net_boot_file_name);

	/* set timeouts for auto-update */
	tftp_timeout_ms = msec_max;
	tftp_timeout_లెక్క_max = cnt_max;

	/* we don't want to retry the connection if errors occur */
	env_సమితి("netretry", "no");

	/* download the update file */
	image_ఎక్కించు_addr = addr;
	copy_filename(net_boot_file_name, filename, పరిమాణం(net_boot_file_name));
	ret = net_loop(TFTPGET);

	ఒకవేళ (ret < 0)
		rv = 1;
	లేకపోతే
		flush_cache(addr, net_boot_file_కొలత);

	/* restore changed globals and env variable */
	tftp_timeout_ms = saved_timeout_msecs;
	tftp_timeout_లెక్క_max = saved_timeout_లెక్క;

	env_సమితి("netretry", saved_netretry);
	ఒకవేళ (saved_netretry != ఖాళీ)
		విడుదల(saved_netretry);

	ఒకవేళ (saved_bootfile != ఖాళీ) {
		copy_filename(net_boot_file_name, saved_bootfile,
			      పరిమాణం(net_boot_file_name));
		విడుదల(saved_bootfile);
	}

	తిరిగి rv;
}

#నిర్వచితమైతే CONFIG_MTD_NOR_FLASH
స్థిర పూర్ణాంకం update_flash_protect(పూర్ణాంకం prot, ulong addr_మొదటి, ulong addr_చివరి)
{
	uchar *sp_info_ptr;
	ulong s;
	పూర్ణాంకం i, bank, cnt;
	flash_info_t *info;

	sp_info_ptr = ఖాళీ;

	ఒకవేళ (prot == 0) {
		saved_prot_info =
			సున్నా_జ్ఞాపకం_ఇవ్వు(CFI_FLASH_BANKS * CONFIG_SYS_MAX_FLASH_SECT, 1);
		ఒకవేళ (!saved_prot_info)
			తిరిగి 1;
	}

	కొరకు (bank = 0; bank < CFI_FLASH_BANKS; ++bank) {
		cnt = 0;
		info = &flash_info[bank];

		/* Nothing to do if the bank doesn't exist */
		ఒకవేళ (info->sector_లెక్క == 0)
			తిరిగి 0;

		/* Point to current bank protection information */
		sp_info_ptr = saved_prot_info + (bank * CONFIG_SYS_MAX_FLASH_SECT);

		/*
		 * Adjust addr_first or addr_last if we are on bank boundary.
		 * Address space between banks must be continuous for other
		 * flash functions (like flash_sect_erase or flash_write) to
		 * succeed. Banks must also be numbered in correct order,
		 * according to increasing addresses.
		 */
		ఒకవేళ (addr_చివరి > info->ప్రారంభం[0] + info->కొలత - 1)
			addr_చివరి = info->ప్రారంభం[0] + info->కొలత - 1;
		ఒకవేళ (addr_మొదటి < info->ప్రారంభం[0])
			addr_మొదటి = info->ప్రారంభం[0];

		కొరకు (i = 0; i < info->sector_లెక్క; i++) {
			/* Save current information about protected sectors */
			ఒకవేళ (prot == 0) {
				s = info->ప్రారంభం[i];
				ఒకవేళ ((s >= addr_మొదటి) && (s <= addr_చివరి))
					sp_info_ptr[i] = info->protect[i];

			}

			/* Protect/unprotect sectors */
			ఒకవేళ (sp_info_ptr[i] == 1) {
#ఒకవేళ defined(CONFIG_SYS_FLASH_PROTECTION)
				ఒకవేళ (flash_real_protect(info, i, prot))
					తిరిగి 1;
#లేకపోతే
				info->protect[i] = prot;
#అయితే_ముగింపు
				cnt++;
			}
		}

		ఒకవేళ (cnt) {
			రాయి("%sProtected %d sectors\n",
						prot ? "": "Un-", cnt);
		}
	}

	ఒకవేళ((prot == 1) && saved_prot_info)
		విడుదల(saved_prot_info);

	తిరిగి 0;
}
#అయితే_ముగింపు

స్థిర పూర్ణాంకం update_flash(ulong addr_ఆధారం, ulong addr_మొదటి, ulong కొలత)
{
#నిర్వచితమైతే CONFIG_MTD_NOR_FLASH
	ulong addr_చివరి = addr_మొదటి + కొలత - 1;

	/* round last address to the sector boundary */
	ఒకవేళ (flash_sect_roundb(&addr_చివరి) > 0)
		తిరిగి 1;

	ఒకవేళ (addr_మొదటి >= addr_చివరి) {
		రాయి("Error: end address exceeds addressing space\n");
		తిరిగి 1;
	}

	/* remove protection on processed sectors */
	ఒకవేళ (update_flash_protect(0, addr_మొదటి, addr_చివరి) > 0) {
		రాయి("Error: could not unprotect flash sectors\n");
		తిరిగి 1;
	}

	రాయి("Erasing 0x%08lx - 0x%08lx", addr_మొదటి, addr_చివరి);
	ఒకవేళ (flash_sect_erase(addr_మొదటి, addr_చివరి) > 0) {
		రాయి("Error: could not erase flash\n");
		తిరిగి 1;
	}

	రాయి("Copying to flash...");
	ఒకవేళ (flash_write((అక్షరం *)addr_ఆధారం, addr_మొదటి, కొలత) > 0) {
		రాయి("Error: could not copy to flash\n");
		తిరిగి 1;
	}
	రాయి("done\n");

	/* enable protection on processed sectors */
	ఒకవేళ (update_flash_protect(1, addr_మొదటి, addr_చివరి) > 0) {
		రాయి("Error: could not protect flash sectors\n");
		తిరిగి 1;
	}
#అయితే_ముగింపు
	తిరిగి 0;
}
#అయితే_ముగింపు /* CONFIG_DFU_TFTP || CONFIG_UPDATE_TFTP */

స్థిర పూర్ణాంకం update_fit_getparams(స్థిరరాశి శూన్య *fit, పూర్ణాంకం noffset, ulong *addr,
						ulong *fladdr, ulong *కొలత)
{
	స్థిరరాశి శూన్య *దత్తాంశం;

	ఒకవేళ (fit_image_get_దత్తాంశం(fit, noffset, &దత్తాంశం, (size_t *)కొలత))
		తిరిగి 1;

	ఒకవేళ (fit_image_get_ఎక్కించు(fit, noffset, (ulong *)fladdr))
		తిరిగి 1;

	*addr = (ulong)దత్తాంశం;

	తిరిగి 0;
}

#ఒకవేళ defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
పూర్ణాంకం update_tftp(ulong addr, అక్షరం *interface, అక్షరం *devstring)
{
	అక్షరం *filename, *env_addr, *fit_image_name;
	ulong update_addr, update_fladdr, update_కొలత;
	పూర్ణాంకం images_noffset, ndepth, noffset;
	bool update_tftp_dfu;
	పూర్ణాంకం ret = 0;
	శూన్య *fit;

	ఒకవేళ (interface == ఖాళీ && devstring == ఖాళీ) {
		update_tftp_dfu = false;
	} లేకపోతే ఒకవేళ (interface && devstring) {
		update_tftp_dfu = true;
	} లేకపోతే {
		pr_err("Interface: %s and devstring: %s not supported!\n",
		      interface, devstring);
		తిరిగి -EINVAL;
	}

	/* use already present image */
	ఒకవేళ (addr)
		వెళ్ళు got_update_file;

	రాయి("Auto-update from TFTP: ");

	/* get the file name of the update file */
	filename = env_get(UPDATE_FILE_ENV);
	ఒకవేళ (filename == ఖాళీ) {
		రాయి("failed, env. variable '%s' not found\n",
							UPDATE_FILE_ENV);
		తిరిగి 1;
	}

	రాయి("trying update file '%s'\n", filename);

	/* get load address of downloaded update file */
	env_addr = env_get("loadaddr");
	ఒకవేళ (env_addr)
		addr = hextoul(env_addr, ఖాళీ);
	లేకపోతే
		addr = CONFIG_UPDATE_ఎక్కించు_ADDR;

	ఒకవేళ (update_ఎక్కించు(filename, CONFIG_UPDATE_TFTP_MSEC_MAX,
					CONFIG_UPDATE_TFTP_CNT_MAX, addr)) {
		రాయి("Can't load update file, aborting auto-update\n");
		తిరిగి 1;
	}

got_update_file:
	fit = map_sysmem(addr, 0);

	ఒకవేళ (fit_check_format((శూన్య *)fit, IMAGE_కొలత_INVAL)) {
		రాయి("Bad FIT format of the update file, aborting "
							"auto-update\n");
		తిరిగి 1;
	}

	/* process updates */
	images_noffset = fdt_మార్గం_స్థానభ్రంశం(fit, FIT_IMAGES_మార్గం);

	ndepth = 0;
	noffset = fdt_తదుపరి_కణుపు(fit, images_noffset, &ndepth);
	అయితే (noffset >= 0 && ndepth > 0) {
		ఒకవేళ (ndepth != 1)
			వెళ్ళు తదుపరి_కణుపు;

		fit_image_name = (అక్షరం *)fit_get_name(fit, noffset, ఖాళీ);
		రాయి("Processing update '%s' :", fit_image_name);

		ఒకవేళ (!fit_image_verify(fit, noffset)) {
			రాయి("Error: invalid update hash, aborting\n");
			ret = 1;
			వెళ్ళు తదుపరి_కణుపు;
		}

		రాయి("\n");
		ఒకవేళ (update_fit_getparams(fit, noffset, &update_addr,
					&update_fladdr, &update_కొలత)) {
			రాయి("Error: can't get update parameters, aborting\n");
			ret = 1;
			వెళ్ళు తదుపరి_కణుపు;
		}

		ఒకవేళ (!update_tftp_dfu) {
			ఒకవేళ (update_flash(update_addr, update_fladdr,
					 update_కొలత)) {
				రాయి("Error: can't flash update, aborting\n");
				ret = 1;
				వెళ్ళు తదుపరి_కణుపు;
			}
		} లేకపోతే ఒకవేళ (fit_image_check_type(fit, noffset,
						IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(శూన్య *)update_addr,
						update_కొలత, interface,
						devstring);
			ఒకవేళ (ret)
				తిరిగి ret;
		}
తదుపరి_కణుపు:
		noffset = fdt_తదుపరి_కణుపు(fit, noffset, &ndepth);
	}

	తిరిగి ret;
}
#అయితే_ముగింపు /* CONFIG_DFU_UPDATE || CONFIG_UPDATE_TFTP */

#నిర్వచితమైతే CONFIG_UPDATE_FIT
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
పూర్ణాంకం fit_update(స్థిరరాశి శూన్య *fit)
{
	అక్షరం *fit_image_name;
	ulong update_addr, update_fladdr, update_కొలత;
	పూర్ణాంకం images_noffset, ndepth, noffset;
	పూర్ణాంకం ret = 0;

	ఒకవేళ (!fit)
		తిరిగి -EINVAL;

	ఒకవేళ (fit_check_format((శూన్య *)fit, IMAGE_కొలత_INVAL)) {
		రాయి("Bad FIT format of the update file, aborting auto-update\n");
		తిరిగి -EINVAL;
	}

	/* process updates */
	images_noffset = fdt_మార్గం_స్థానభ్రంశం(fit, FIT_IMAGES_మార్గం);

	ndepth = 0;
	noffset = fdt_తదుపరి_కణుపు(fit, images_noffset, &ndepth);
	అయితే (noffset >= 0 && ndepth > 0) {
		ఒకవేళ (ndepth != 1)
			వెళ్ళు తదుపరి_కణుపు;

		fit_image_name = (అక్షరం *)fit_get_name(fit, noffset, ఖాళీ);
		రాయి("Processing update '%s' :", fit_image_name);

		ఒకవేళ (!fit_image_verify(fit, noffset)) {
			రాయి("Error: invalid update hash, aborting\n");
			ret = 1;
			వెళ్ళు తదుపరి_కణుపు;
		}

		రాయి("\n");
		ఒకవేళ (update_fit_getparams(fit, noffset, &update_addr,
					 &update_fladdr, &update_కొలత)) {
			రాయి("Error: can't get update parameters, aborting\n");
			ret = 1;
			వెళ్ళు తదుపరి_కణుపు;
		}

		ఒకవేళ (fit_image_check_type(fit, noffset, IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(శూన్య *)update_addr,
						update_కొలత, ఖాళీ, ఖాళీ);
			ఒకవేళ (ret)
				తిరిగి ret;
		}
తదుపరి_కణుపు:
		noffset = fdt_తదుపరి_కణుపు(fit, noffset, &ndepth);
	}

	తిరిగి ret;
}
#అయితే_ముగింపు /* CONFIG_UPDATE_FIT */
