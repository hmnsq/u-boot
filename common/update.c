// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Semihalf
 *
 * Written by: Rafal Czubak <rcz@semihalf.com>
 *             Bartlomiej Sieka <tur@semihalf.com>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <cpu_func.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/printk.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <command.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <net.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <net/tftp.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mapmem.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dfu.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>

#ਜੇ defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
/* env variable holding the location of the update file */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UPDATE_FILE_ENV		"updatefile"

ਬਾਹਰੀ ulong tftp_timeout_ms;
ਬਾਹਰੀ ਪੂਰਨ tftp_timeout_ਗਿਣਤੀ_max;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MTD_NOR_FLASH
#ਅਨ੍ਤਰ੍ਭਾਵ <flash.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mtd/cfi_flash.h>
ਸਥਿਰ uchar *saved_prot_info;
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਪੂਰਨ update_ਲੱਦੋ(ਅੱਖਰ *filename, ulong msec_max, ਪੂਰਨ cnt_max, ulong addr)
{
	ਪੂਰਨ rv, ret;
	ulong saved_timeout_msecs;
	ਪੂਰਨ saved_timeout_ਗਿਣਤੀ;
	ਅੱਖਰ *saved_netretry, *saved_bootfile;

	rv = 0;
	/* save used globals and env variable */
	saved_timeout_msecs = tftp_timeout_ms;
	saved_timeout_ਗਿਣਤੀ = tftp_timeout_ਗਿਣਤੀ_max;
	saved_netretry = strdup(env_get("netretry"));
	saved_bootfile = strdup(net_boot_file_name);

	/* set timeouts for auto-update */
	tftp_timeout_ms = msec_max;
	tftp_timeout_ਗਿਣਤੀ_max = cnt_max;

	/* we don't want to retry the connection if errors occur */
	env_ਸੈੱਟ("netretry", "no");

	/* download the update file */
	image_ਲੱਦੋ_addr = addr;
	copy_filename(net_boot_file_name, filename, ਆਕਾਰ(net_boot_file_name));
	ret = net_loop(TFTPGET);

	ਜੇ (ret < 0)
		rv = 1;
	ਨਹੀਂਤਾਂ
		flush_cache(addr, net_boot_file_ਮਾਪ);

	/* restore changed globals and env variable */
	tftp_timeout_ms = saved_timeout_msecs;
	tftp_timeout_ਗਿਣਤੀ_max = saved_timeout_ਗਿਣਤੀ;

	env_ਸੈੱਟ("netretry", saved_netretry);
	ਜੇ (saved_netretry != ਨਿਰਾ)
		ਮੁਕਤ(saved_netretry);

	ਜੇ (saved_bootfile != ਨਿਰਾ) {
		copy_filename(net_boot_file_name, saved_bootfile,
			      ਆਕਾਰ(net_boot_file_name));
		ਮੁਕਤ(saved_bootfile);
	}

	ਵਾਪਸ rv;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MTD_NOR_FLASH
ਸਥਿਰ ਪੂਰਨ update_flash_protect(ਪੂਰਨ prot, ulong addr_ਪਹਿਲਾ, ulong addr_ਆਖਰੀ)
{
	uchar *sp_info_ptr;
	ulong s;
	ਪੂਰਨ i, bank, cnt;
	flash_info_t *info;

	sp_info_ptr = ਨਿਰਾ;

	ਜੇ (prot == 0) {
		saved_prot_info =
			ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(CFI_FLASH_BANKS * CONFIG_SYS_MAX_FLASH_SECT, 1);
		ਜੇ (!saved_prot_info)
			ਵਾਪਸ 1;
	}

	ਲਈ (bank = 0; bank < CFI_FLASH_BANKS; ++bank) {
		cnt = 0;
		info = &flash_info[bank];

		/* Nothing to do if the bank doesn't exist */
		ਜੇ (info->sector_ਗਿਣਤੀ == 0)
			ਵਾਪਸ 0;

		/* Point to current bank protection information */
		sp_info_ptr = saved_prot_info + (bank * CONFIG_SYS_MAX_FLASH_SECT);

		/*
		 * Adjust addr_first or addr_last if we are on bank boundary.
		 * Address space between banks must be continuous for other
		 * flash functions (like flash_sect_erase or flash_write) to
		 * succeed. Banks must also be numbered in correct order,
		 * according to increasing addresses.
		 */
		ਜੇ (addr_ਆਖਰੀ > info->ਸ਼ੁਰੂ[0] + info->ਮਾਪ - 1)
			addr_ਆਖਰੀ = info->ਸ਼ੁਰੂ[0] + info->ਮਾਪ - 1;
		ਜੇ (addr_ਪਹਿਲਾ < info->ਸ਼ੁਰੂ[0])
			addr_ਪਹਿਲਾ = info->ਸ਼ੁਰੂ[0];

		ਲਈ (i = 0; i < info->sector_ਗਿਣਤੀ; i++) {
			/* Save current information about protected sectors */
			ਜੇ (prot == 0) {
				s = info->ਸ਼ੁਰੂ[i];
				ਜੇ ((s >= addr_ਪਹਿਲਾ) && (s <= addr_ਆਖਰੀ))
					sp_info_ptr[i] = info->protect[i];

			}

			/* Protect/unprotect sectors */
			ਜੇ (sp_info_ptr[i] == 1) {
#ਜੇ defined(CONFIG_SYS_FLASH_PROTECTION)
				ਜੇ (flash_real_protect(info, i, prot))
					ਵਾਪਸ 1;
#ਨਹੀਂਤਾਂ
				info->protect[i] = prot;
#ਜੇ_ਅੰਤ
				cnt++;
			}
		}

		ਜੇ (cnt) {
			ਲਿਖੋ("%sProtected %d sectors\n",
						prot ? "": "Un-", cnt);
		}
	}

	ਜੇ((prot == 1) && saved_prot_info)
		ਮੁਕਤ(saved_prot_info);

	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ update_flash(ulong addr_ਸਰੋਤ, ulong addr_ਪਹਿਲਾ, ulong ਮਾਪ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MTD_NOR_FLASH
	ulong addr_ਆਖਰੀ = addr_ਪਹਿਲਾ + ਮਾਪ - 1;

	/* round last address to the sector boundary */
	ਜੇ (flash_sect_roundb(&addr_ਆਖਰੀ) > 0)
		ਵਾਪਸ 1;

	ਜੇ (addr_ਪਹਿਲਾ >= addr_ਆਖਰੀ) {
		ਲਿਖੋ("Error: end address exceeds addressing space\n");
		ਵਾਪਸ 1;
	}

	/* remove protection on processed sectors */
	ਜੇ (update_flash_protect(0, addr_ਪਹਿਲਾ, addr_ਆਖਰੀ) > 0) {
		ਲਿਖੋ("Error: could not unprotect flash sectors\n");
		ਵਾਪਸ 1;
	}

	ਲਿਖੋ("Erasing 0x%08lx - 0x%08lx", addr_ਪਹਿਲਾ, addr_ਆਖਰੀ);
	ਜੇ (flash_sect_erase(addr_ਪਹਿਲਾ, addr_ਆਖਰੀ) > 0) {
		ਲਿਖੋ("Error: could not erase flash\n");
		ਵਾਪਸ 1;
	}

	ਲਿਖੋ("Copying to flash...");
	ਜੇ (flash_write((ਅੱਖਰ *)addr_ਸਰੋਤ, addr_ਪਹਿਲਾ, ਮਾਪ) > 0) {
		ਲਿਖੋ("Error: could not copy to flash\n");
		ਵਾਪਸ 1;
	}
	ਲਿਖੋ("done\n");

	/* enable protection on processed sectors */
	ਜੇ (update_flash_protect(1, addr_ਪਹਿਲਾ, addr_ਆਖਰੀ) > 0) {
		ਲਿਖੋ("Error: could not protect flash sectors\n");
		ਵਾਪਸ 1;
	}
#ਜੇ_ਅੰਤ
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ /* CONFIG_DFU_TFTP || CONFIG_UPDATE_TFTP */

ਸਥਿਰ ਪੂਰਨ update_fit_getparams(ਅਚਲ ਖਾਲੀ *fit, ਪੂਰਨ noffset, ulong *addr,
						ulong *fladdr, ulong *ਮਾਪ)
{
	ਅਚਲ ਖਾਲੀ *ਅੰਕੜੇ;

	ਜੇ (fit_image_get_ਅੰਕੜੇ(fit, noffset, &ਅੰਕੜੇ, (size_t *)ਮਾਪ))
		ਵਾਪਸ 1;

	ਜੇ (fit_image_get_ਲੱਦੋ(fit, noffset, (ulong *)fladdr))
		ਵਾਪਸ 1;

	*addr = (ulong)ਅੰਕੜੇ;

	ਵਾਪਸ 0;
}

#ਜੇ defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
ਪੂਰਨ update_tftp(ulong addr, ਅੱਖਰ *interface, ਅੱਖਰ *devstring)
{
	ਅੱਖਰ *filename, *env_addr, *fit_image_name;
	ulong update_addr, update_fladdr, update_ਮਾਪ;
	ਪੂਰਨ images_noffset, ndepth, noffset;
	bool update_tftp_dfu;
	ਪੂਰਨ ret = 0;
	ਖਾਲੀ *fit;

	ਜੇ (interface == ਨਿਰਾ && devstring == ਨਿਰਾ) {
		update_tftp_dfu = false;
	} ਨਹੀਂਤਾਂ ਜੇ (interface && devstring) {
		update_tftp_dfu = true;
	} ਨਹੀਂਤਾਂ {
		pr_err("Interface: %s and devstring: %s not supported!\n",
		      interface, devstring);
		ਵਾਪਸ -EINVAL;
	}

	/* use already present image */
	ਜੇ (addr)
		ਜਾਓ got_update_file;

	ਲਿਖੋ("Auto-update from TFTP: ");

	/* get the file name of the update file */
	filename = env_get(UPDATE_FILE_ENV);
	ਜੇ (filename == ਨਿਰਾ) {
		ਲਿਖੋ("failed, env. variable '%s' not found\n",
							UPDATE_FILE_ENV);
		ਵਾਪਸ 1;
	}

	ਲਿਖੋ("trying update file '%s'\n", filename);

	/* get load address of downloaded update file */
	env_addr = env_get("loadaddr");
	ਜੇ (env_addr)
		addr = hextoul(env_addr, ਨਿਰਾ);
	ਨਹੀਂਤਾਂ
		addr = CONFIG_UPDATE_ਲੱਦੋ_ADDR;

	ਜੇ (update_ਲੱਦੋ(filename, CONFIG_UPDATE_TFTP_MSEC_MAX,
					CONFIG_UPDATE_TFTP_CNT_MAX, addr)) {
		ਲਿਖੋ("Can't load update file, aborting auto-update\n");
		ਵਾਪਸ 1;
	}

got_update_file:
	fit = map_sysmem(addr, 0);

	ਜੇ (fit_check_format((ਖਾਲੀ *)fit, IMAGE_ਮਾਪ_INVAL)) {
		ਲਿਖੋ("Bad FIT format of the update file, aborting "
							"auto-update\n");
		ਵਾਪਸ 1;
	}

	/* process updates */
	images_noffset = fdt_ਰਸਤਾ_ਵਿੱਥ(fit, FIT_IMAGES_ਰਸਤਾ);

	ndepth = 0;
	noffset = fdt_ਅਗਲਾ_ਗੰਢ(fit, images_noffset, &ndepth);
	ਜਦੋਂ (noffset >= 0 && ndepth > 0) {
		ਜੇ (ndepth != 1)
			ਜਾਓ ਅਗਲਾ_ਗੰਢ;

		fit_image_name = (ਅੱਖਰ *)fit_get_name(fit, noffset, ਨਿਰਾ);
		ਲਿਖੋ("Processing update '%s' :", fit_image_name);

		ਜੇ (!fit_image_verify(fit, noffset)) {
			ਲਿਖੋ("Error: invalid update hash, aborting\n");
			ret = 1;
			ਜਾਓ ਅਗਲਾ_ਗੰਢ;
		}

		ਲਿਖੋ("\n");
		ਜੇ (update_fit_getparams(fit, noffset, &update_addr,
					&update_fladdr, &update_ਮਾਪ)) {
			ਲਿਖੋ("Error: can't get update parameters, aborting\n");
			ret = 1;
			ਜਾਓ ਅਗਲਾ_ਗੰਢ;
		}

		ਜੇ (!update_tftp_dfu) {
			ਜੇ (update_flash(update_addr, update_fladdr,
					 update_ਮਾਪ)) {
				ਲਿਖੋ("Error: can't flash update, aborting\n");
				ret = 1;
				ਜਾਓ ਅਗਲਾ_ਗੰਢ;
			}
		} ਨਹੀਂਤਾਂ ਜੇ (fit_image_check_type(fit, noffset,
						IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(ਖਾਲੀ *)update_addr,
						update_ਮਾਪ, interface,
						devstring);
			ਜੇ (ret)
				ਵਾਪਸ ret;
		}
ਅਗਲਾ_ਗੰਢ:
		noffset = fdt_ਅਗਲਾ_ਗੰਢ(fit, noffset, &ndepth);
	}

	ਵਾਪਸ ret;
}
#ਜੇ_ਅੰਤ /* CONFIG_DFU_UPDATE || CONFIG_UPDATE_TFTP */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_UPDATE_FIT
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
ਪੂਰਨ fit_update(ਅਚਲ ਖਾਲੀ *fit)
{
	ਅੱਖਰ *fit_image_name;
	ulong update_addr, update_fladdr, update_ਮਾਪ;
	ਪੂਰਨ images_noffset, ndepth, noffset;
	ਪੂਰਨ ret = 0;

	ਜੇ (!fit)
		ਵਾਪਸ -EINVAL;

	ਜੇ (fit_check_format((ਖਾਲੀ *)fit, IMAGE_ਮਾਪ_INVAL)) {
		ਲਿਖੋ("Bad FIT format of the update file, aborting auto-update\n");
		ਵਾਪਸ -EINVAL;
	}

	/* process updates */
	images_noffset = fdt_ਰਸਤਾ_ਵਿੱਥ(fit, FIT_IMAGES_ਰਸਤਾ);

	ndepth = 0;
	noffset = fdt_ਅਗਲਾ_ਗੰਢ(fit, images_noffset, &ndepth);
	ਜਦੋਂ (noffset >= 0 && ndepth > 0) {
		ਜੇ (ndepth != 1)
			ਜਾਓ ਅਗਲਾ_ਗੰਢ;

		fit_image_name = (ਅੱਖਰ *)fit_get_name(fit, noffset, ਨਿਰਾ);
		ਲਿਖੋ("Processing update '%s' :", fit_image_name);

		ਜੇ (!fit_image_verify(fit, noffset)) {
			ਲਿਖੋ("Error: invalid update hash, aborting\n");
			ret = 1;
			ਜਾਓ ਅਗਲਾ_ਗੰਢ;
		}

		ਲਿਖੋ("\n");
		ਜੇ (update_fit_getparams(fit, noffset, &update_addr,
					 &update_fladdr, &update_ਮਾਪ)) {
			ਲਿਖੋ("Error: can't get update parameters, aborting\n");
			ret = 1;
			ਜਾਓ ਅਗਲਾ_ਗੰਢ;
		}

		ਜੇ (fit_image_check_type(fit, noffset, IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(ਖਾਲੀ *)update_addr,
						update_ਮਾਪ, ਨਿਰਾ, ਨਿਰਾ);
			ਜੇ (ret)
				ਵਾਪਸ ret;
		}
ਅਗਲਾ_ਗੰਢ:
		noffset = fdt_ਅਗਲਾ_ਗੰਢ(fit, noffset, &ndepth);
	}

	ਵਾਪਸ ret;
}
#ਜੇ_ਅੰਤ /* CONFIG_UPDATE_FIT */
