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

#ಆದರೆ defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
/* env variable holding the location of the update file */
#ವ್ಯಾಖ್ಯಾನಿಸು UPDATE_FILE_ENV		"updatefile"

ಬಾಹ್ಯ ulong tftp_timeout_ms;
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ tftp_timeout_ಎಣಿಕೆ_max;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_MTD_NOR_FLASH
#ಅನ್ತರ್ಭಾವ <flash.h>
#ಅನ್ತರ್ಭಾವ <mtd/cfi_flash.h>
ಸ್ಥಿರ uchar *saved_prot_info;
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ update_ತುಂಬು(ಅಕ್ಷರ *filename, ulong msec_max, ಪೂರ್ಣಾಂಕ cnt_max, ulong addr)
{
	ಪೂರ್ಣಾಂಕ rv, ret;
	ulong saved_timeout_msecs;
	ಪೂರ್ಣಾಂಕ saved_timeout_ಎಣಿಕೆ;
	ಅಕ್ಷರ *saved_netretry, *saved_bootfile;

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
	env_ಗಣ("netretry", "no");

	/* download the update file */
	image_ತುಂಬು_addr = addr;
	copy_filename(net_boot_file_name, filename, ಗಾತ್ರ(net_boot_file_name));
	ret = net_loop(TFTPGET);

	ಆದರೆ (ret < 0)
		rv = 1;
	ಇಲ್ಲದಿದ್ದರೆ
		flush_cache(addr, net_boot_file_ಗಾತ್ರ);

	/* restore changed globals and env variable */
	tftp_timeout_ms = saved_timeout_msecs;
	tftp_timeout_ಎಣಿಕೆ_max = saved_timeout_ಎಣಿಕೆ;

	env_ಗಣ("netretry", saved_netretry);
	ಆದರೆ (saved_netretry != ಖಾಲಿ)
		ಬಿಡುಗಡೆ(saved_netretry);

	ಆದರೆ (saved_bootfile != ಖಾಲಿ) {
		copy_filename(net_boot_file_name, saved_bootfile,
			      ಗಾತ್ರ(net_boot_file_name));
		ಬಿಡುಗಡೆ(saved_bootfile);
	}

	ಹಿಂತಿರುಗಿ rv;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_MTD_NOR_FLASH
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ update_flash_protect(ಪೂರ್ಣಾಂಕ prot, ulong addr_ಮೊದಲ, ulong addr_ಕೊನೆಯ)
{
	uchar *sp_info_ptr;
	ulong s;
	ಪೂರ್ಣಾಂಕ i, bank, cnt;
	flash_info_t *info;

	sp_info_ptr = ಖಾಲಿ;

	ಆದರೆ (prot == 0) {
		saved_prot_info =
			ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(CFI_FLASH_BANKS * CONFIG_SYS_MAX_FLASH_SECT, 1);
		ಆದರೆ (!saved_prot_info)
			ಹಿಂತಿರುಗಿ 1;
	}

	ಗಾಗಿ (bank = 0; bank < CFI_FLASH_BANKS; ++bank) {
		cnt = 0;
		info = &flash_info[bank];

		/* Nothing to do if the bank doesn't exist */
		ಆದರೆ (info->sector_ಎಣಿಕೆ == 0)
			ಹಿಂತಿರುಗಿ 0;

		/* Point to current bank protection information */
		sp_info_ptr = saved_prot_info + (bank * CONFIG_SYS_MAX_FLASH_SECT);

		/*
		 * Adjust addr_first or addr_last if we are on bank boundary.
		 * Address space between banks must be continuous for other
		 * flash functions (like flash_sect_erase or flash_write) to
		 * succeed. Banks must also be numbered in correct order,
		 * according to increasing addresses.
		 */
		ಆದರೆ (addr_ಕೊನೆಯ > info->ಆರಂಭ[0] + info->ಗಾತ್ರ_2 - 1)
			addr_ಕೊನೆಯ = info->ಆರಂಭ[0] + info->ಗಾತ್ರ_2 - 1;
		ಆದರೆ (addr_ಮೊದಲ < info->ಆರಂಭ[0])
			addr_ಮೊದಲ = info->ಆರಂಭ[0];

		ಗಾಗಿ (i = 0; i < info->sector_ಎಣಿಕೆ; i++) {
			/* Save current information about protected sectors */
			ಆದರೆ (prot == 0) {
				s = info->ಆರಂಭ[i];
				ಆದರೆ ((s >= addr_ಮೊದಲ) && (s <= addr_ಕೊನೆಯ))
					sp_info_ptr[i] = info->protect[i];

			}

			/* Protect/unprotect sectors */
			ಆದರೆ (sp_info_ptr[i] == 1) {
#ಆದರೆ defined(CONFIG_SYS_FLASH_PROTECTION)
				ಆದರೆ (flash_real_protect(info, i, prot))
					ಹಿಂತಿರುಗಿ 1;
#ಇಲ್ಲದಿದ್ದರೆ
				info->protect[i] = prot;
#ಆದರೆ_ಅಂತ್ಯ
				cnt++;
			}
		}

		ಆದರೆ (cnt) {
			ಬರೆ("%sProtected %d sectors\n",
						prot ? "": "Un-", cnt);
		}
	}

	ಆದರೆ((prot == 1) && saved_prot_info)
		ಬಿಡುಗಡೆ(saved_prot_info);

	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ update_flash(ulong addr_ಮೂಲ, ulong addr_ಮೊದಲ, ulong ಗಾತ್ರ_2)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_MTD_NOR_FLASH
	ulong addr_ಕೊನೆಯ = addr_ಮೊದಲ + ಗಾತ್ರ_2 - 1;

	/* round last address to the sector boundary */
	ಆದರೆ (flash_sect_roundb(&addr_ಕೊನೆಯ) > 0)
		ಹಿಂತಿರುಗಿ 1;

	ಆದರೆ (addr_ಮೊದಲ >= addr_ಕೊನೆಯ) {
		ಬರೆ("Error: end address exceeds addressing space\n");
		ಹಿಂತಿರುಗಿ 1;
	}

	/* remove protection on processed sectors */
	ಆದರೆ (update_flash_protect(0, addr_ಮೊದಲ, addr_ಕೊನೆಯ) > 0) {
		ಬರೆ("Error: could not unprotect flash sectors\n");
		ಹಿಂತಿರುಗಿ 1;
	}

	ಬರೆ("Erasing 0x%08lx - 0x%08lx", addr_ಮೊದಲ, addr_ಕೊನೆಯ);
	ಆದರೆ (flash_sect_erase(addr_ಮೊದಲ, addr_ಕೊನೆಯ) > 0) {
		ಬರೆ("Error: could not erase flash\n");
		ಹಿಂತಿರುಗಿ 1;
	}

	ಬರೆ("Copying to flash...");
	ಆದರೆ (flash_write((ಅಕ್ಷರ *)addr_ಮೂಲ, addr_ಮೊದಲ, ಗಾತ್ರ_2) > 0) {
		ಬರೆ("Error: could not copy to flash\n");
		ಹಿಂತಿರುಗಿ 1;
	}
	ಬರೆ("done\n");

	/* enable protection on processed sectors */
	ಆದರೆ (update_flash_protect(1, addr_ಮೊದಲ, addr_ಕೊನೆಯ) > 0) {
		ಬರೆ("Error: could not protect flash sectors\n");
		ಹಿಂತಿರುಗಿ 1;
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DFU_TFTP || CONFIG_UPDATE_TFTP */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ update_fit_getparams(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *fit, ಪೂರ್ಣಾಂಕ noffset, ulong *addr,
						ulong *fladdr, ulong *ಗಾತ್ರ_2)
{
	ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ದತ್ತಾಂಶ;

	ಆದರೆ (fit_image_get_ದತ್ತಾಂಶ(fit, noffset, &ದತ್ತಾಂಶ, (size_t *)ಗಾತ್ರ_2))
		ಹಿಂತಿರುಗಿ 1;

	ಆದರೆ (fit_image_get_ತುಂಬು(fit, noffset, (ulong *)fladdr))
		ಹಿಂತಿರುಗಿ 1;

	*addr = (ulong)ದತ್ತಾಂಶ;

	ಹಿಂತಿರುಗಿ 0;
}

#ಆದರೆ defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
ಪೂರ್ಣಾಂಕ update_tftp(ulong addr, ಅಕ್ಷರ *interface, ಅಕ್ಷರ *devstring)
{
	ಅಕ್ಷರ *filename, *env_addr, *fit_image_name;
	ulong update_addr, update_fladdr, update_ಗಾತ್ರ;
	ಪೂರ್ಣಾಂಕ images_noffset, ndepth, noffset;
	bool update_tftp_dfu;
	ಪೂರ್ಣಾಂಕ ret = 0;
	ಶೂನ್ಯ *fit;

	ಆದರೆ (interface == ಖಾಲಿ && devstring == ಖಾಲಿ) {
		update_tftp_dfu = false;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (interface && devstring) {
		update_tftp_dfu = true;
	} ಇಲ್ಲದಿದ್ದರೆ {
		pr_err("Interface: %s and devstring: %s not supported!\n",
		      interface, devstring);
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	/* use already present image */
	ಆದರೆ (addr)
		ಹೋಗು got_update_file;

	ಬರೆ("Auto-update from TFTP: ");

	/* get the file name of the update file */
	filename = env_get(UPDATE_FILE_ENV);
	ಆದರೆ (filename == ಖಾಲಿ) {
		ಬರೆ("failed, env. variable '%s' not found\n",
							UPDATE_FILE_ENV);
		ಹಿಂತಿರುಗಿ 1;
	}

	ಬರೆ("trying update file '%s'\n", filename);

	/* get load address of downloaded update file */
	env_addr = env_get("loadaddr");
	ಆದರೆ (env_addr)
		addr = hextoul(env_addr, ಖಾಲಿ);
	ಇಲ್ಲದಿದ್ದರೆ
		addr = CONFIG_UPDATE_ತುಂಬು_ADDR;

	ಆದರೆ (update_ತುಂಬು(filename, CONFIG_UPDATE_TFTP_MSEC_MAX,
					CONFIG_UPDATE_TFTP_CNT_MAX, addr)) {
		ಬರೆ("Can't load update file, aborting auto-update\n");
		ಹಿಂತಿರುಗಿ 1;
	}

got_update_file:
	fit = map_sysmem(addr, 0);

	ಆದರೆ (fit_check_format((ಶೂನ್ಯ *)fit, IMAGE_ಗಾತ್ರ_INVAL)) {
		ಬರೆ("Bad FIT format of the update file, aborting "
							"auto-update\n");
		ಹಿಂತಿರುಗಿ 1;
	}

	/* process updates */
	images_noffset = fdt_ಮಾರ್ಗ_ಸ್ಥಾನಾಂತರ(fit, FIT_IMAGES_ಮಾರ್ಗ);

	ndepth = 0;
	noffset = fdt_ಮುಂದಿನ_ಗಂಟು(fit, images_noffset, &ndepth);
	ಆಗ (noffset >= 0 && ndepth > 0) {
		ಆದರೆ (ndepth != 1)
			ಹೋಗು ಮುಂದಿನ_ಗಂಟು;

		fit_image_name = (ಅಕ್ಷರ *)fit_get_name(fit, noffset, ಖಾಲಿ);
		ಬರೆ("Processing update '%s' :", fit_image_name);

		ಆದರೆ (!fit_image_verify(fit, noffset)) {
			ಬರೆ("Error: invalid update hash, aborting\n");
			ret = 1;
			ಹೋಗು ಮುಂದಿನ_ಗಂಟು;
		}

		ಬರೆ("\n");
		ಆದರೆ (update_fit_getparams(fit, noffset, &update_addr,
					&update_fladdr, &update_ಗಾತ್ರ)) {
			ಬರೆ("Error: can't get update parameters, aborting\n");
			ret = 1;
			ಹೋಗು ಮುಂದಿನ_ಗಂಟು;
		}

		ಆದರೆ (!update_tftp_dfu) {
			ಆದರೆ (update_flash(update_addr, update_fladdr,
					 update_ಗಾತ್ರ)) {
				ಬರೆ("Error: can't flash update, aborting\n");
				ret = 1;
				ಹೋಗು ಮುಂದಿನ_ಗಂಟು;
			}
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (fit_image_check_type(fit, noffset,
						IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(ಶೂನ್ಯ *)update_addr,
						update_ಗಾತ್ರ, interface,
						devstring);
			ಆದರೆ (ret)
				ಹಿಂತಿರುಗಿ ret;
		}
ಮುಂದಿನ_ಗಂಟು:
		noffset = fdt_ಮುಂದಿನ_ಗಂಟು(fit, noffset, &ndepth);
	}

	ಹಿಂತಿರುಗಿ ret;
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DFU_UPDATE || CONFIG_UPDATE_TFTP */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_UPDATE_FIT
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
ಪೂರ್ಣಾಂಕ fit_update(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *fit)
{
	ಅಕ್ಷರ *fit_image_name;
	ulong update_addr, update_fladdr, update_ಗಾತ್ರ;
	ಪೂರ್ಣಾಂಕ images_noffset, ndepth, noffset;
	ಪೂರ್ಣಾಂಕ ret = 0;

	ಆದರೆ (!fit)
		ಹಿಂತಿರುಗಿ -EINVAL;

	ಆದರೆ (fit_check_format((ಶೂನ್ಯ *)fit, IMAGE_ಗಾತ್ರ_INVAL)) {
		ಬರೆ("Bad FIT format of the update file, aborting auto-update\n");
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	/* process updates */
	images_noffset = fdt_ಮಾರ್ಗ_ಸ್ಥಾನಾಂತರ(fit, FIT_IMAGES_ಮಾರ್ಗ);

	ndepth = 0;
	noffset = fdt_ಮುಂದಿನ_ಗಂಟು(fit, images_noffset, &ndepth);
	ಆಗ (noffset >= 0 && ndepth > 0) {
		ಆದರೆ (ndepth != 1)
			ಹೋಗು ಮುಂದಿನ_ಗಂಟು;

		fit_image_name = (ಅಕ್ಷರ *)fit_get_name(fit, noffset, ಖಾಲಿ);
		ಬರೆ("Processing update '%s' :", fit_image_name);

		ಆದರೆ (!fit_image_verify(fit, noffset)) {
			ಬರೆ("Error: invalid update hash, aborting\n");
			ret = 1;
			ಹೋಗು ಮುಂದಿನ_ಗಂಟು;
		}

		ಬರೆ("\n");
		ಆದರೆ (update_fit_getparams(fit, noffset, &update_addr,
					 &update_fladdr, &update_ಗಾತ್ರ)) {
			ಬರೆ("Error: can't get update parameters, aborting\n");
			ret = 1;
			ಹೋಗು ಮುಂದಿನ_ಗಂಟು;
		}

		ಆದರೆ (fit_image_check_type(fit, noffset, IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(ಶೂನ್ಯ *)update_addr,
						update_ಗಾತ್ರ, ಖಾಲಿ, ಖಾಲಿ);
			ಆದರೆ (ret)
				ಹಿಂತಿರುಗಿ ret;
		}
ಮುಂದಿನ_ಗಂಟು:
		noffset = fdt_ಮುಂದಿನ_ಗಂಟು(fit, noffset, &ndepth);
	}

	ಹಿಂತಿರುಗಿ ret;
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_UPDATE_FIT */
