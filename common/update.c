// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Semihalf
 *
 * Written by: Rafal Czubak <rcz@semihalf.com>
 *             Bartlomiej Sieka <tur@semihalf.com>
 */

#समावेश <cpu_func.h>
#समावेश <image.h>
#समावेश <linux/printk.h>

#समावेश <command.h>
#समावेश <env.h>
#समावेश <net.h>
#समावेश <net/tftp.h>
#समावेश <malloc.h>
#समावेश <mapmem.h>
#समावेश <dfu.h>
#समावेश <errno.h>

#अगर defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
/* env variable holding the location of the update file */
#घोषणा UPDATE_FILE_ENV		"updatefile"

बाह्य ulong tftp_timeout_ms;
बाह्य पूर्णांक tftp_timeout_गिनती_max;
#अगर_घोषित CONFIG_MTD_NOR_FLASH
#समावेश <flash.h>
#समावेश <mtd/cfi_flash.h>
अटल uchar *saved_prot_info;
#पूर्ण_अगर
अटल पूर्णांक update_load(अक्षर *filename, ulong msec_max, पूर्णांक cnt_max, ulong addr)
{
	पूर्णांक rv, ret;
	ulong saved_timeout_msecs;
	पूर्णांक saved_timeout_गिनती;
	अक्षर *saved_netretry, *saved_bootfile;

	rv = 0;
	/* save used globals and env variable */
	saved_timeout_msecs = tftp_timeout_ms;
	saved_timeout_गिनती = tftp_timeout_गिनती_max;
	saved_netretry = strdup(env_get("netretry"));
	saved_bootfile = strdup(net_boot_file_name);

	/* set timeouts for auto-update */
	tftp_timeout_ms = msec_max;
	tftp_timeout_गिनती_max = cnt_max;

	/* we don't want to retry the connection if errors occur */
	env_set("netretry", "no");

	/* download the update file */
	image_load_addr = addr;
	copy_filename(net_boot_file_name, filename, माप(net_boot_file_name));
	ret = net_loop(TFTPGET);

	अगर (ret < 0)
		rv = 1;
	अन्यथा
		flush_cache(addr, net_boot_file_आकार);

	/* restore changed globals and env variable */
	tftp_timeout_ms = saved_timeout_msecs;
	tftp_timeout_गिनती_max = saved_timeout_गिनती;

	env_set("netretry", saved_netretry);
	अगर (saved_netretry != शून्य)
		मुक्त(saved_netretry);

	अगर (saved_bootfile != शून्य) {
		copy_filename(net_boot_file_name, saved_bootfile,
			      माप(net_boot_file_name));
		मुक्त(saved_bootfile);
	}

	वापस rv;
}

#अगर_घोषित CONFIG_MTD_NOR_FLASH
अटल पूर्णांक update_flash_protect(पूर्णांक prot, ulong addr_first, ulong addr_last)
{
	uchar *sp_info_ptr;
	ulong s;
	पूर्णांक i, bank, cnt;
	flash_info_t *info;

	sp_info_ptr = शून्य;

	अगर (prot == 0) {
		saved_prot_info =
			सुस्मृति(CFI_FLASH_BANKS * CONFIG_SYS_MAX_FLASH_SECT, 1);
		अगर (!saved_prot_info)
			वापस 1;
	}

	क्रम (bank = 0; bank < CFI_FLASH_BANKS; ++bank) {
		cnt = 0;
		info = &flash_info[bank];

		/* Nothing to do if the bank doesn't exist */
		अगर (info->sector_गिनती == 0)
			वापस 0;

		/* Point to current bank protection information */
		sp_info_ptr = saved_prot_info + (bank * CONFIG_SYS_MAX_FLASH_SECT);

		/*
		 * Adjust addr_first or addr_last if we are on bank boundary.
		 * Address space between banks must be continuous for other
		 * flash functions (like flash_sect_erase or flash_write) to
		 * succeed. Banks must also be numbered in correct order,
		 * according to increasing addresses.
		 */
		अगर (addr_last > info->start[0] + info->आकार - 1)
			addr_last = info->start[0] + info->आकार - 1;
		अगर (addr_first < info->start[0])
			addr_first = info->start[0];

		क्रम (i = 0; i < info->sector_गिनती; i++) {
			/* Save current information about protected sectors */
			अगर (prot == 0) {
				s = info->start[i];
				अगर ((s >= addr_first) && (s <= addr_last))
					sp_info_ptr[i] = info->protect[i];

			}

			/* Protect/unprotect sectors */
			अगर (sp_info_ptr[i] == 1) {
#अगर defined(CONFIG_SYS_FLASH_PROTECTION)
				अगर (flash_real_protect(info, i, prot))
					वापस 1;
#अन्यथा
				info->protect[i] = prot;
#पूर्ण_अगर
				cnt++;
			}
		}

		अगर (cnt) {
			म_लिखो("%sProtected %d sectors\n",
						prot ? "": "Un-", cnt);
		}
	}

	अगर((prot == 1) && saved_prot_info)
		मुक्त(saved_prot_info);

	वापस 0;
}
#पूर्ण_अगर

अटल पूर्णांक update_flash(ulong addr_source, ulong addr_first, ulong आकार)
{
#अगर_घोषित CONFIG_MTD_NOR_FLASH
	ulong addr_last = addr_first + आकार - 1;

	/* round last address to the sector boundary */
	अगर (flash_sect_roundb(&addr_last) > 0)
		वापस 1;

	अगर (addr_first >= addr_last) {
		म_लिखो("Error: end address exceeds addressing space\n");
		वापस 1;
	}

	/* remove protection on processed sectors */
	अगर (update_flash_protect(0, addr_first, addr_last) > 0) {
		म_लिखो("Error: could not unprotect flash sectors\n");
		वापस 1;
	}

	म_लिखो("Erasing 0x%08lx - 0x%08lx", addr_first, addr_last);
	अगर (flash_sect_erase(addr_first, addr_last) > 0) {
		म_लिखो("Error: could not erase flash\n");
		वापस 1;
	}

	म_लिखो("Copying to flash...");
	अगर (flash_write((अक्षर *)addr_source, addr_first, आकार) > 0) {
		म_लिखो("Error: could not copy to flash\n");
		वापस 1;
	}
	म_लिखो("done\n");

	/* enable protection on processed sectors */
	अगर (update_flash_protect(1, addr_first, addr_last) > 0) {
		म_लिखो("Error: could not protect flash sectors\n");
		वापस 1;
	}
#पूर्ण_अगर
	वापस 0;
}
#पूर्ण_अगर /* CONFIG_DFU_TFTP || CONFIG_UPDATE_TFTP */

अटल पूर्णांक update_fit_getparams(स्थिर व्योम *fit, पूर्णांक noffset, ulong *addr,
						ulong *fladdr, ulong *आकार)
{
	स्थिर व्योम *तथ्य;

	अगर (fit_image_get_तथ्य(fit, noffset, &तथ्य, (माप_प्रकार *)आकार))
		वापस 1;

	अगर (fit_image_get_load(fit, noffset, (ulong *)fladdr))
		वापस 1;

	*addr = (ulong)तथ्य;

	वापस 0;
}

#अगर defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
पूर्णांक update_tftp(ulong addr, अक्षर *interface, अक्षर *devstring)
{
	अक्षर *filename, *env_addr, *fit_image_name;
	ulong update_addr, update_fladdr, update_आकार;
	पूर्णांक images_noffset, ndepth, noffset;
	bool update_tftp_dfu;
	पूर्णांक ret = 0;
	व्योम *fit;

	अगर (interface == शून्य && devstring == शून्य) {
		update_tftp_dfu = false;
	} अन्यथा अगर (interface && devstring) {
		update_tftp_dfu = true;
	} अन्यथा {
		pr_err("Interface: %s and devstring: %s not supported!\n",
		      interface, devstring);
		वापस -EINVAL;
	}

	/* use already present image */
	अगर (addr)
		जाओ got_update_file;

	म_लिखो("Auto-update from TFTP: ");

	/* get the file name of the update file */
	filename = env_get(UPDATE_FILE_ENV);
	अगर (filename == शून्य) {
		म_लिखो("failed, env. variable '%s' not found\n",
							UPDATE_FILE_ENV);
		वापस 1;
	}

	म_लिखो("trying update file '%s'\n", filename);

	/* get load address of downloaded update file */
	env_addr = env_get("loadaddr");
	अगर (env_addr)
		addr = hextoul(env_addr, शून्य);
	अन्यथा
		addr = CONFIG_UPDATE_LOAD_ADDR;

	अगर (update_load(filename, CONFIG_UPDATE_TFTP_MSEC_MAX,
					CONFIG_UPDATE_TFTP_CNT_MAX, addr)) {
		म_लिखो("Can't load update file, aborting auto-update\n");
		वापस 1;
	}

got_update_file:
	fit = map_sysmem(addr, 0);

	अगर (fit_check_format((व्योम *)fit, IMAGE_आकार_INVAL)) {
		म_लिखो("Bad FIT format of the update file, aborting "
							"auto-update\n");
		वापस 1;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_अग्र_गाँठ(fit, images_noffset, &ndepth);
	जबतक (noffset >= 0 && ndepth > 0) {
		अगर (ndepth != 1)
			जाओ अग्र_गाँठ;

		fit_image_name = (अक्षर *)fit_get_name(fit, noffset, शून्य);
		म_लिखो("Processing update '%s' :", fit_image_name);

		अगर (!fit_image_verify(fit, noffset)) {
			म_लिखो("Error: invalid update hash, aborting\n");
			ret = 1;
			जाओ अग्र_गाँठ;
		}

		म_लिखो("\n");
		अगर (update_fit_getparams(fit, noffset, &update_addr,
					&update_fladdr, &update_आकार)) {
			म_लिखो("Error: can't get update parameters, aborting\n");
			ret = 1;
			जाओ अग्र_गाँठ;
		}

		अगर (!update_tftp_dfu) {
			अगर (update_flash(update_addr, update_fladdr,
					 update_आकार)) {
				म_लिखो("Error: can't flash update, aborting\n");
				ret = 1;
				जाओ अग्र_गाँठ;
			}
		} अन्यथा अगर (fit_image_check_type(fit, noffset,
						IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(व्योम *)update_addr,
						update_आकार, interface,
						devstring);
			अगर (ret)
				वापस ret;
		}
अग्र_गाँठ:
		noffset = fdt_अग्र_गाँठ(fit, noffset, &ndepth);
	}

	वापस ret;
}
#पूर्ण_अगर /* CONFIG_DFU_UPDATE || CONFIG_UPDATE_TFTP */

#अगर_घोषित CONFIG_UPDATE_FIT
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
पूर्णांक fit_update(स्थिर व्योम *fit)
{
	अक्षर *fit_image_name;
	ulong update_addr, update_fladdr, update_आकार;
	पूर्णांक images_noffset, ndepth, noffset;
	पूर्णांक ret = 0;

	अगर (!fit)
		वापस -EINVAL;

	अगर (fit_check_format((व्योम *)fit, IMAGE_आकार_INVAL)) {
		म_लिखो("Bad FIT format of the update file, aborting auto-update\n");
		वापस -EINVAL;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_अग्र_गाँठ(fit, images_noffset, &ndepth);
	जबतक (noffset >= 0 && ndepth > 0) {
		अगर (ndepth != 1)
			जाओ अग्र_गाँठ;

		fit_image_name = (अक्षर *)fit_get_name(fit, noffset, शून्य);
		म_लिखो("Processing update '%s' :", fit_image_name);

		अगर (!fit_image_verify(fit, noffset)) {
			म_लिखो("Error: invalid update hash, aborting\n");
			ret = 1;
			जाओ अग्र_गाँठ;
		}

		म_लिखो("\n");
		अगर (update_fit_getparams(fit, noffset, &update_addr,
					 &update_fladdr, &update_आकार)) {
			म_लिखो("Error: can't get update parameters, aborting\n");
			ret = 1;
			जाओ अग्र_गाँठ;
		}

		अगर (fit_image_check_type(fit, noffset, IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(व्योम *)update_addr,
						update_आकार, शून्य, शून्य);
			अगर (ret)
				वापस ret;
		}
अग्र_गाँठ:
		noffset = fdt_अग्र_गाँठ(fit, noffset, &ndepth);
	}

	वापस ret;
}
#पूर्ण_अगर /* CONFIG_UPDATE_FIT */
