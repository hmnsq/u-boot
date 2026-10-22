// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Semihalf
 *
 * Written by: Rafal Czubak <rcz@semihalf.com>
 *             Bartlomiej Sieka <tur@semihalf.com>
 */

#अन्तर्भाव <cpu_func.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <linux/printk.h>

#अन्तर्भाव <command.h>
#अन्तर्भाव <env.h>
#अन्तर्भाव <net.h>
#अन्तर्भाव <net/tftp.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <mapmem.h>
#अन्तर्भाव <dfu.h>
#अन्तर्भाव <errno.h>

#यदि defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
/* env variable holding the location of the update file */
#परिभाषय UPDATE_FILE_ENV		"updatefile"

बाह्य ulong tftp_timeout_ms;
बाह्य पूर्णाङ्क tftp_timeout_गणनम्_max;
#यदि_परिभाषितम् CONFIG_MTD_NOR_FLASH
#अन्तर्भाव <flash.h>
#अन्तर्भाव <mtd/cfi_flash.h>
स्थिर uchar *saved_prot_info;
#यदि_अन्तः
स्थिर पूर्णाङ्क update_आरोपय(वर्ण *filename, ulong msec_max, पूर्णाङ्क cnt_max, ulong addr)
{
	पूर्णाङ्क rv, ret;
	ulong saved_timeout_msecs;
	पूर्णाङ्क saved_timeout_गणनम्;
	वर्ण *saved_netretry, *saved_bootfile;

	rv = 0;
	/* save used globals and env variable */
	saved_timeout_msecs = tftp_timeout_ms;
	saved_timeout_गणनम् = tftp_timeout_गणनम्_max;
	saved_netretry = strdup(env_get("netretry"));
	saved_bootfile = strdup(net_boot_file_name);

	/* set timeouts for auto-update */
	tftp_timeout_ms = msec_max;
	tftp_timeout_गणनम्_max = cnt_max;

	/* we don't want to retry the connection if errors occur */
	env_समुच्चयः("netretry", "no");

	/* download the update file */
	image_आरोपय_addr = addr;
	copy_filename(net_boot_file_name, filename, परिमाण(net_boot_file_name));
	ret = net_loop(TFTPGET);

	यदि (ret < 0)
		rv = 1;
	अन्यथा
		flush_cache(addr, net_boot_file_आकार);

	/* restore changed globals and env variable */
	tftp_timeout_ms = saved_timeout_msecs;
	tftp_timeout_गणनम्_max = saved_timeout_गणनम्;

	env_समुच्चयः("netretry", saved_netretry);
	यदि (saved_netretry != रिक्त)
		मुक्त(saved_netretry);

	यदि (saved_bootfile != रिक्त) {
		copy_filename(net_boot_file_name, saved_bootfile,
			      परिमाण(net_boot_file_name));
		मुक्त(saved_bootfile);
	}

	प्रत्यागम rv;
}

#यदि_परिभाषितम् CONFIG_MTD_NOR_FLASH
स्थिर पूर्णाङ्क update_flash_protect(पूर्णाङ्क prot, ulong addr_first, ulong addr_चरमः)
{
	uchar *sp_info_ptr;
	ulong s;
	पूर्णाङ्क i, bank, cnt;
	flash_info_t *info;

	sp_info_ptr = रिक्त;

	यदि (prot == 0) {
		saved_prot_info =
			शून्यस्मृतिं_देहि(CFI_FLASH_BANKS * CONFIG_SYS_MAX_FLASH_SECT, 1);
		यदि (!saved_prot_info)
			प्रत्यागम 1;
	}

	प्रति (bank = 0; bank < CFI_FLASH_BANKS; ++bank) {
		cnt = 0;
		info = &flash_info[bank];

		/* Nothing to do if the bank doesn't exist */
		यदि (info->sector_गणनम् == 0)
			प्रत्यागम 0;

		/* Point to current bank protection information */
		sp_info_ptr = saved_prot_info + (bank * CONFIG_SYS_MAX_FLASH_SECT);

		/*
		 * Adjust addr_first or addr_last if we are on bank boundary.
		 * Address space between banks must be continuous for other
		 * flash functions (like flash_sect_erase or flash_write) to
		 * succeed. Banks must also be numbered in correct order,
		 * according to increasing addresses.
		 */
		यदि (addr_चरमः > info->start[0] + info->आकार - 1)
			addr_चरमः = info->start[0] + info->आकार - 1;
		यदि (addr_first < info->start[0])
			addr_first = info->start[0];

		प्रति (i = 0; i < info->sector_गणनम्; i++) {
			/* Save current information about protected sectors */
			यदि (prot == 0) {
				s = info->start[i];
				यदि ((s >= addr_first) && (s <= addr_चरमः))
					sp_info_ptr[i] = info->protect[i];

			}

			/* Protect/unprotect sectors */
			यदि (sp_info_ptr[i] == 1) {
#यदि defined(CONFIG_SYS_FLASH_PROTECTION)
				यदि (flash_real_protect(info, i, prot))
					प्रत्यागम 1;
#अन्यथा
				info->protect[i] = prot;
#यदि_अन्तः
				cnt++;
			}
		}

		यदि (cnt) {
			प्रारूप_लिख("%sProtected %d sectors\n",
						prot ? "": "Un-", cnt);
		}
	}

	यदि((prot == 1) && saved_prot_info)
		मुक्त(saved_prot_info);

	प्रत्यागम 0;
}
#यदि_अन्तः

स्थिर पूर्णाङ्क update_flash(ulong addr_source, ulong addr_first, ulong आकार)
{
#यदि_परिभाषितम् CONFIG_MTD_NOR_FLASH
	ulong addr_चरमः = addr_first + आकार - 1;

	/* round last address to the sector boundary */
	यदि (flash_sect_roundb(&addr_चरमः) > 0)
		प्रत्यागम 1;

	यदि (addr_first >= addr_चरमः) {
		प्रारूप_लिख("Error: end address exceeds addressing space\n");
		प्रत्यागम 1;
	}

	/* remove protection on processed sectors */
	यदि (update_flash_protect(0, addr_first, addr_चरमः) > 0) {
		प्रारूप_लिख("Error: could not unprotect flash sectors\n");
		प्रत्यागम 1;
	}

	प्रारूप_लिख("Erasing 0x%08lx - 0x%08lx", addr_first, addr_चरमः);
	यदि (flash_sect_erase(addr_first, addr_चरमः) > 0) {
		प्रारूप_लिख("Error: could not erase flash\n");
		प्रत्यागम 1;
	}

	प्रारूप_लिख("Copying to flash...");
	यदि (flash_write((वर्ण *)addr_source, addr_first, आकार) > 0) {
		प्रारूप_लिख("Error: could not copy to flash\n");
		प्रत्यागम 1;
	}
	प्रारूप_लिख("done\n");

	/* enable protection on processed sectors */
	यदि (update_flash_protect(1, addr_first, addr_चरमः) > 0) {
		प्रारूप_लिख("Error: could not protect flash sectors\n");
		प्रत्यागम 1;
	}
#यदि_अन्तः
	प्रत्यागम 0;
}
#यदि_अन्तः /* CONFIG_DFU_TFTP || CONFIG_UPDATE_TFTP */

स्थिर पूर्णाङ्क update_fit_getparams(ध्रुव शून्य *fit, पूर्णाङ्क noffset, ulong *addr,
						ulong *fladdr, ulong *आकार)
{
	ध्रुव शून्य *दत्त;

	यदि (fit_image_get_दत्त(fit, noffset, &दत्त, (size_t *)आकार))
		प्रत्यागम 1;

	यदि (fit_image_get_आरोपय(fit, noffset, (ulong *)fladdr))
		प्रत्यागम 1;

	*addr = (ulong)दत्त;

	प्रत्यागम 0;
}

#यदि defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
पूर्णाङ्क update_tftp(ulong addr, वर्ण *interface, वर्ण *devstring)
{
	वर्ण *filename, *env_addr, *fit_image_name;
	ulong update_addr, update_fladdr, update_आकार;
	पूर्णाङ्क images_noffset, ndepth, noffset;
	bool update_tftp_dfu;
	पूर्णाङ्क ret = 0;
	शून्य *fit;

	यदि (interface == रिक्त && devstring == रिक्त) {
		update_tftp_dfu = false;
	} अन्यथा यदि (interface && devstring) {
		update_tftp_dfu = true;
	} अन्यथा {
		pr_err("Interface: %s and devstring: %s not supported!\n",
		      interface, devstring);
		प्रत्यागम -EINVAL;
	}

	/* use already present image */
	यदि (addr)
		गच्छ got_update_file;

	प्रारूप_लिख("Auto-update from TFTP: ");

	/* get the file name of the update file */
	filename = env_get(UPDATE_FILE_ENV);
	यदि (filename == रिक्त) {
		प्रारूप_लिख("failed, env. variable '%s' not found\n",
							UPDATE_FILE_ENV);
		प्रत्यागम 1;
	}

	प्रारूप_लिख("trying update file '%s'\n", filename);

	/* get load address of downloaded update file */
	env_addr = env_get("loadaddr");
	यदि (env_addr)
		addr = hextoul(env_addr, रिक्त);
	अन्यथा
		addr = CONFIG_UPDATE_आरोपय_ADDR;

	यदि (update_आरोपय(filename, CONFIG_UPDATE_TFTP_MSEC_MAX,
					CONFIG_UPDATE_TFTP_CNT_MAX, addr)) {
		प्रारूप_लिख("Can't load update file, aborting auto-update\n");
		प्रत्यागम 1;
	}

got_update_file:
	fit = map_sysmem(addr, 0);

	यदि (fit_check_format((शून्य *)fit, IMAGE_आकार_INVAL)) {
		प्रारूप_लिख("Bad FIT format of the update file, aborting "
							"auto-update\n");
		प्रत्यागम 1;
	}

	/* process updates */
	images_noffset = fdt_मार्गः_विस्थापनम्(fit, FIT_IMAGES_मार्गः);

	ndepth = 0;
	noffset = fdt_अग्र_ग्रन्थिः(fit, images_noffset, &ndepth);
	यावत् (noffset >= 0 && ndepth > 0) {
		यदि (ndepth != 1)
			गच्छ अग्र_ग्रन्थिः;

		fit_image_name = (वर्ण *)fit_get_name(fit, noffset, रिक्त);
		प्रारूप_लिख("Processing update '%s' :", fit_image_name);

		यदि (!fit_image_verify(fit, noffset)) {
			प्रारूप_लिख("Error: invalid update hash, aborting\n");
			ret = 1;
			गच्छ अग्र_ग्रन्थिः;
		}

		प्रारूप_लिख("\n");
		यदि (update_fit_getparams(fit, noffset, &update_addr,
					&update_fladdr, &update_आकार)) {
			प्रारूप_लिख("Error: can't get update parameters, aborting\n");
			ret = 1;
			गच्छ अग्र_ग्रन्थिः;
		}

		यदि (!update_tftp_dfu) {
			यदि (update_flash(update_addr, update_fladdr,
					 update_आकार)) {
				प्रारूप_लिख("Error: can't flash update, aborting\n");
				ret = 1;
				गच्छ अग्र_ग्रन्थिः;
			}
		} अन्यथा यदि (fit_image_check_type(fit, noffset,
						IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(शून्य *)update_addr,
						update_आकार, interface,
						devstring);
			यदि (ret)
				प्रत्यागम ret;
		}
अग्र_ग्रन्थिः:
		noffset = fdt_अग्र_ग्रन्थिः(fit, noffset, &ndepth);
	}

	प्रत्यागम ret;
}
#यदि_अन्तः /* CONFIG_DFU_UPDATE || CONFIG_UPDATE_TFTP */

#यदि_परिभाषितम् CONFIG_UPDATE_FIT
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
पूर्णाङ्क fit_update(ध्रुव शून्य *fit)
{
	वर्ण *fit_image_name;
	ulong update_addr, update_fladdr, update_आकार;
	पूर्णाङ्क images_noffset, ndepth, noffset;
	पूर्णाङ्क ret = 0;

	यदि (!fit)
		प्रत्यागम -EINVAL;

	यदि (fit_check_format((शून्य *)fit, IMAGE_आकार_INVAL)) {
		प्रारूप_लिख("Bad FIT format of the update file, aborting auto-update\n");
		प्रत्यागम -EINVAL;
	}

	/* process updates */
	images_noffset = fdt_मार्गः_विस्थापनम्(fit, FIT_IMAGES_मार्गः);

	ndepth = 0;
	noffset = fdt_अग्र_ग्रन्थिः(fit, images_noffset, &ndepth);
	यावत् (noffset >= 0 && ndepth > 0) {
		यदि (ndepth != 1)
			गच्छ अग्र_ग्रन्थिः;

		fit_image_name = (वर्ण *)fit_get_name(fit, noffset, रिक्त);
		प्रारूप_लिख("Processing update '%s' :", fit_image_name);

		यदि (!fit_image_verify(fit, noffset)) {
			प्रारूप_लिख("Error: invalid update hash, aborting\n");
			ret = 1;
			गच्छ अग्र_ग्रन्थिः;
		}

		प्रारूप_लिख("\n");
		यदि (update_fit_getparams(fit, noffset, &update_addr,
					 &update_fladdr, &update_आकार)) {
			प्रारूप_लिख("Error: can't get update parameters, aborting\n");
			ret = 1;
			गच्छ अग्र_ग्रन्थिः;
		}

		यदि (fit_image_check_type(fit, noffset, IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(शून्य *)update_addr,
						update_आकार, रिक्त, रिक्त);
			यदि (ret)
				प्रत्यागम ret;
		}
अग्र_ग्रन्थिः:
		noffset = fdt_अग्र_ग्रन्थिः(fit, noffset, &ndepth);
	}

	प्रत्यागम ret;
}
#यदि_अन्तः /* CONFIG_UPDATE_FIT */
