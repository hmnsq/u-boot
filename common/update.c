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

#जर defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
/* env variable holding the location of the update file */
#define UPDATE_FILE_ENV		"updatefile"

बाह्य ulong tftp_timeout_ms;
बाह्य पूर्णांक tftp_timeout_मोजणी_max;
#ifdef CONFIG_MTD_NOR_FLASH
#अन्तर्भाव <flash.h>
#अन्तर्भाव <mtd/cfi_flash.h>
स्थिर uchar *saved_prot_info;
#endif
स्थिर पूर्णांक update_load(अक्षर *filename, ulong msec_max, पूर्णांक cnt_max, ulong addr)
{
	पूर्णांक rv, ret;
	ulong saved_timeout_msecs;
	पूर्णांक saved_timeout_मोजणी;
	अक्षर *saved_netretry, *saved_bootfile;

	rv = 0;
	/* save used globals and env variable */
	saved_timeout_msecs = tftp_timeout_ms;
	saved_timeout_मोजणी = tftp_timeout_मोजणी_max;
	saved_netretry = strdup(env_get("netretry"));
	saved_bootfile = strdup(net_boot_file_name);

	/* set timeouts for auto-update */
	tftp_timeout_ms = msec_max;
	tftp_timeout_मोजणी_max = cnt_max;

	/* we don't want to retry the connection if errors occur */
	env_set("netretry", "no");

	/* download the update file */
	image_load_addr = addr;
	copy_filename(net_boot_file_name, filename, आकार(net_boot_file_name));
	ret = net_loop(TFTPGET);

	जर (ret < 0)
		rv = 1;
	अन्यथा
		flush_cache(addr, net_boot_file_size);

	/* restore changed globals and env variable */
	tftp_timeout_ms = saved_timeout_msecs;
	tftp_timeout_मोजणी_max = saved_timeout_मोजणी;

	env_set("netretry", saved_netretry);
	जर (saved_netretry != रिक्त)
		मुक्त(saved_netretry);

	जर (saved_bootfile != रिक्त) {
		copy_filename(net_boot_file_name, saved_bootfile,
			      आकार(net_boot_file_name));
		मुक्त(saved_bootfile);
	}

	परत rv;
}

#ifdef CONFIG_MTD_NOR_FLASH
स्थिर पूर्णांक update_flash_protect(पूर्णांक prot, ulong addr_first, ulong addr_last)
{
	uchar *sp_info_ptr;
	ulong s;
	पूर्णांक i, bank, cnt;
	flash_info_t *info;

	sp_info_ptr = रिक्त;

	जर (prot == 0) {
		saved_prot_info =
			सुस्मृति(CFI_FLASH_BANKS * CONFIG_SYS_MAX_FLASH_SECT, 1);
		जर (!saved_prot_info)
			परत 1;
	}

	साठी (bank = 0; bank < CFI_FLASH_BANKS; ++bank) {
		cnt = 0;
		info = &flash_info[bank];

		/* Nothing to do if the bank doesn't exist */
		जर (info->sector_मोजणी == 0)
			परत 0;

		/* Point to current bank protection information */
		sp_info_ptr = saved_prot_info + (bank * CONFIG_SYS_MAX_FLASH_SECT);

		/*
		 * Adjust addr_first or addr_last if we are on bank boundary.
		 * Address space between banks must be continuous for other
		 * flash functions (like flash_sect_erase or flash_write) to
		 * succeed. Banks must also be numbered in correct order,
		 * according to increasing addresses.
		 */
		जर (addr_last > info->start[0] + info->size - 1)
			addr_last = info->start[0] + info->size - 1;
		जर (addr_first < info->start[0])
			addr_first = info->start[0];

		साठी (i = 0; i < info->sector_मोजणी; i++) {
			/* Save current information about protected sectors */
			जर (prot == 0) {
				s = info->start[i];
				जर ((s >= addr_first) && (s <= addr_last))
					sp_info_ptr[i] = info->protect[i];

			}

			/* Protect/unprotect sectors */
			जर (sp_info_ptr[i] == 1) {
#जर defined(CONFIG_SYS_FLASH_PROTECTION)
				जर (flash_real_protect(info, i, prot))
					परत 1;
#अन्यथा
				info->protect[i] = prot;
#endif
				cnt++;
			}
		}

		जर (cnt) {
			म_लिखो("%sProtected %d sectors\n",
						prot ? "": "Un-", cnt);
		}
	}

	जर((prot == 1) && saved_prot_info)
		मुक्त(saved_prot_info);

	परत 0;
}
#endif

स्थिर पूर्णांक update_flash(ulong addr_source, ulong addr_first, ulong size)
{
#ifdef CONFIG_MTD_NOR_FLASH
	ulong addr_last = addr_first + size - 1;

	/* round last address to the sector boundary */
	जर (flash_sect_roundb(&addr_last) > 0)
		परत 1;

	जर (addr_first >= addr_last) {
		म_लिखो("Error: end address exceeds addressing space\n");
		परत 1;
	}

	/* remove protection on processed sectors */
	जर (update_flash_protect(0, addr_first, addr_last) > 0) {
		म_लिखो("Error: could not unprotect flash sectors\n");
		परत 1;
	}

	म_लिखो("Erasing 0x%08lx - 0x%08lx", addr_first, addr_last);
	जर (flash_sect_erase(addr_first, addr_last) > 0) {
		म_लिखो("Error: could not erase flash\n");
		परत 1;
	}

	म_लिखो("Copying to flash...");
	जर (flash_write((अक्षर *)addr_source, addr_first, size) > 0) {
		म_लिखो("Error: could not copy to flash\n");
		परत 1;
	}
	म_लिखो("done\n");

	/* enable protection on processed sectors */
	जर (update_flash_protect(1, addr_first, addr_last) > 0) {
		म_लिखो("Error: could not protect flash sectors\n");
		परत 1;
	}
#endif
	परत 0;
}
#endif /* CONFIG_DFU_TFTP || CONFIG_UPDATE_TFTP */

स्थिर पूर्णांक update_fit_getparams(स्थिरांक शून्य *fit, पूर्णांक noffset, ulong *addr,
						ulong *fladdr, ulong *size)
{
	स्थिरांक शून्य *data;

	जर (fit_image_get_data(fit, noffset, &data, (माप_प्रकार *)size))
		परत 1;

	जर (fit_image_get_load(fit, noffset, (ulong *)fladdr))
		परत 1;

	*addr = (ulong)data;

	परत 0;
}

#जर defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
पूर्णांक update_tftp(ulong addr, अक्षर *interface, अक्षर *devstring)
{
	अक्षर *filename, *env_addr, *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
	पूर्णांक images_noffset, ndepth, noffset;
	bool update_tftp_dfu;
	पूर्णांक ret = 0;
	शून्य *fit;

	जर (interface == रिक्त && devstring == रिक्त) {
		update_tftp_dfu = false;
	} अन्यथा जर (interface && devstring) {
		update_tftp_dfu = true;
	} अन्यथा {
		pr_err("Interface: %s and devstring: %s not supported!\n",
		      interface, devstring);
		परत -EINVAL;
	}

	/* use already present image */
	जर (addr)
		जा got_update_file;

	म_लिखो("Auto-update from TFTP: ");

	/* get the file name of the update file */
	filename = env_get(UPDATE_FILE_ENV);
	जर (filename == रिक्त) {
		म_लिखो("failed, env. variable '%s' not found\n",
							UPDATE_FILE_ENV);
		परत 1;
	}

	म_लिखो("trying update file '%s'\n", filename);

	/* get load address of downloaded update file */
	env_addr = env_get("loadaddr");
	जर (env_addr)
		addr = hextoul(env_addr, रिक्त);
	अन्यथा
		addr = CONFIG_UPDATE_LOAD_ADDR;

	जर (update_load(filename, CONFIG_UPDATE_TFTP_MSEC_MAX,
					CONFIG_UPDATE_TFTP_CNT_MAX, addr)) {
		म_लिखो("Can't load update file, aborting auto-update\n");
		परत 1;
	}

got_update_file:
	fit = map_sysmem(addr, 0);

	जर (fit_check_format((शून्य *)fit, IMAGE_SIZE_INVAL)) {
		म_लिखो("Bad FIT format of the update file, aborting "
							"auto-update\n");
		परत 1;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_गाठ(fit, images_noffset, &ndepth);
	पर्यंत (noffset >= 0 && ndepth > 0) {
		जर (ndepth != 1)
			जा next_गाठ;

		fit_image_name = (अक्षर *)fit_get_name(fit, noffset, रिक्त);
		म_लिखो("Processing update '%s' :", fit_image_name);

		जर (!fit_image_verify(fit, noffset)) {
			म_लिखो("Error: invalid update hash, aborting\n");
			ret = 1;
			जा next_गाठ;
		}

		म_लिखो("\n");
		जर (update_fit_getparams(fit, noffset, &update_addr,
					&update_fladdr, &update_size)) {
			म_लिखो("Error: can't get update parameters, aborting\n");
			ret = 1;
			जा next_गाठ;
		}

		जर (!update_tftp_dfu) {
			जर (update_flash(update_addr, update_fladdr,
					 update_size)) {
				म_लिखो("Error: can't flash update, aborting\n");
				ret = 1;
				जा next_गाठ;
			}
		} अन्यथा जर (fit_image_check_type(fit, noffset,
						IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(शून्य *)update_addr,
						update_size, interface,
						devstring);
			जर (ret)
				परत ret;
		}
next_गाठ:
		noffset = fdt_next_गाठ(fit, noffset, &ndepth);
	}

	परत ret;
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
पूर्णांक fit_update(स्थिरांक शून्य *fit)
{
	अक्षर *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
	पूर्णांक images_noffset, ndepth, noffset;
	पूर्णांक ret = 0;

	जर (!fit)
		परत -EINVAL;

	जर (fit_check_format((शून्य *)fit, IMAGE_SIZE_INVAL)) {
		म_लिखो("Bad FIT format of the update file, aborting auto-update\n");
		परत -EINVAL;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_गाठ(fit, images_noffset, &ndepth);
	पर्यंत (noffset >= 0 && ndepth > 0) {
		जर (ndepth != 1)
			जा next_गाठ;

		fit_image_name = (अक्षर *)fit_get_name(fit, noffset, रिक्त);
		म_लिखो("Processing update '%s' :", fit_image_name);

		जर (!fit_image_verify(fit, noffset)) {
			म_लिखो("Error: invalid update hash, aborting\n");
			ret = 1;
			जा next_गाठ;
		}

		म_लिखो("\n");
		जर (update_fit_getparams(fit, noffset, &update_addr,
					 &update_fladdr, &update_size)) {
			म_लिखो("Error: can't get update parameters, aborting\n");
			ret = 1;
			जा next_गाठ;
		}

		जर (fit_image_check_type(fit, noffset, IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(शून्य *)update_addr,
						update_size, रिक्त, रिक्त);
			जर (ret)
				परत ret;
		}
next_गाठ:
		noffset = fdt_next_गाठ(fit, noffset, &ndepth);
	}

	परत ret;
}
#endif /* CONFIG_UPDATE_FIT */
