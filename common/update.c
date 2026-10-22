// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Semihalf
 *
 * Written by: Rafal Czubak <rcz@semihalf.com>
 *             Bartlomiej Sieka <tur@semihalf.com>
 */

#شامل <cpu_func.h>
#شامل <image.h>
#شامل <linux/printk.h>

#شامل <command.h>
#شامل <env.h>
#شامل <net.h>
#شامل <net/tftp.h>
#شامل <malloc.h>
#شامل <mapmem.h>
#شامل <dfu.h>
#شامل <errno.h>

#اگر defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
/* env variable holding the location of the update file */
#تعریف_کرو UPDATE_FILE_ENV		"updatefile"

خارجی ulong tftp_timeout_ms;
خارجی صحیح tftp_timeout_گنتی_max;
#اگر_تعریف_شدہ CONFIG_MTD_NOR_FLASH
#شامل <flash.h>
#شامل <mtd/cfi_flash.h>
ساکن uchar *saved_prot_info;
#اگر_ختم
ساکن صحیح update_بھرو(حرف *filename, ulong msec_max, صحیح cnt_max, ulong addr)
{
	صحیح rv, ret;
	ulong saved_timeout_msecs;
	صحیح saved_timeout_گنتی;
	حرف *saved_netretry, *saved_bootfile;

	rv = 0;
	/* save used globals and env variable */
	saved_timeout_msecs = tftp_timeout_ms;
	saved_timeout_گنتی = tftp_timeout_گنتی_max;
	saved_netretry = strdup(env_get("netretry"));
	saved_bootfile = strdup(net_boot_file_name);

	/* set timeouts for auto-update */
	tftp_timeout_ms = msec_max;
	tftp_timeout_گنتی_max = cnt_max;

	/* we don't want to retry the connection if errors occur */
	env_سیٹ("netretry", "no");

	/* download the update file */
	image_بھرو_addr = addr;
	copy_filename(net_boot_file_name, filename, حجم(net_boot_file_name));
	ret = net_loop(TFTPGET);

	اگر (ret < 0)
		rv = 1;
	ورنہ
		flush_cache(addr, net_boot_file_سائز);

	/* restore changed globals and env variable */
	tftp_timeout_ms = saved_timeout_msecs;
	tftp_timeout_گنتی_max = saved_timeout_گنتی;

	env_سیٹ("netretry", saved_netretry);
	اگر (saved_netretry != تہی)
		آزاد(saved_netretry);

	اگر (saved_bootfile != تہی) {
		copy_filename(net_boot_file_name, saved_bootfile,
			      حجم(net_boot_file_name));
		آزاد(saved_bootfile);
	}

	واپس rv;
}

#اگر_تعریف_شدہ CONFIG_MTD_NOR_FLASH
ساکن صحیح update_flash_protect(صحیح prot, ulong addr_پہلا, ulong addr_آخری)
{
	uchar *sp_info_ptr;
	ulong s;
	صحیح i, bank, cnt;
	flash_info_t *info;

	sp_info_ptr = تہی;

	اگر (prot == 0) {
		saved_prot_info =
			صفر_حافظہ_دو(CFI_FLASH_BANKS * CONFIG_SYS_MAX_FLASH_SECT, 1);
		اگر (!saved_prot_info)
			واپس 1;
	}

	برائے (bank = 0; bank < CFI_FLASH_BANKS; ++bank) {
		cnt = 0;
		info = &flash_info[bank];

		/* Nothing to do if the bank doesn't exist */
		اگر (info->sector_گنتی == 0)
			واپس 0;

		/* Point to current bank protection information */
		sp_info_ptr = saved_prot_info + (bank * CONFIG_SYS_MAX_FLASH_SECT);

		/*
		 * Adjust addr_first or addr_last if we are on bank boundary.
		 * Address space between banks must be continuous for other
		 * flash functions (like flash_sect_erase or flash_write) to
		 * succeed. Banks must also be numbered in correct order,
		 * according to increasing addresses.
		 */
		اگر (addr_آخری > info->آغاز[0] + info->سائز - 1)
			addr_آخری = info->آغاز[0] + info->سائز - 1;
		اگر (addr_پہلا < info->آغاز[0])
			addr_پہلا = info->آغاز[0];

		برائے (i = 0; i < info->sector_گنتی; i++) {
			/* Save current information about protected sectors */
			اگر (prot == 0) {
				s = info->آغاز[i];
				اگر ((s >= addr_پہلا) && (s <= addr_آخری))
					sp_info_ptr[i] = info->protect[i];

			}

			/* Protect/unprotect sectors */
			اگر (sp_info_ptr[i] == 1) {
#اگر defined(CONFIG_SYS_FLASH_PROTECTION)
				اگر (flash_real_protect(info, i, prot))
					واپس 1;
#ورنہ
				info->protect[i] = prot;
#اگر_ختم
				cnt++;
			}
		}

		اگر (cnt) {
			لکھو("%sProtected %d sectors\n",
						prot ? "": "Un-", cnt);
		}
	}

	اگر((prot == 1) && saved_prot_info)
		آزاد(saved_prot_info);

	واپس 0;
}
#اگر_ختم

ساکن صحیح update_flash(ulong addr_ماخذ, ulong addr_پہلا, ulong سائز)
{
#اگر_تعریف_شدہ CONFIG_MTD_NOR_FLASH
	ulong addr_آخری = addr_پہلا + سائز - 1;

	/* round last address to the sector boundary */
	اگر (flash_sect_roundb(&addr_آخری) > 0)
		واپس 1;

	اگر (addr_پہلا >= addr_آخری) {
		لکھو("Error: end address exceeds addressing space\n");
		واپس 1;
	}

	/* remove protection on processed sectors */
	اگر (update_flash_protect(0, addr_پہلا, addr_آخری) > 0) {
		لکھو("Error: could not unprotect flash sectors\n");
		واپس 1;
	}

	لکھو("Erasing 0x%08lx - 0x%08lx", addr_پہلا, addr_آخری);
	اگر (flash_sect_erase(addr_پہلا, addr_آخری) > 0) {
		لکھو("Error: could not erase flash\n");
		واپس 1;
	}

	لکھو("Copying to flash...");
	اگر (flash_write((حرف *)addr_ماخذ, addr_پہلا, سائز) > 0) {
		لکھو("Error: could not copy to flash\n");
		واپس 1;
	}
	لکھو("done\n");

	/* enable protection on processed sectors */
	اگر (update_flash_protect(1, addr_پہلا, addr_آخری) > 0) {
		لکھو("Error: could not protect flash sectors\n");
		واپس 1;
	}
#اگر_ختم
	واپس 0;
}
#اگر_ختم /* CONFIG_DFU_TFTP || CONFIG_UPDATE_TFTP */

ساکن صحیح update_fit_getparams(ثابت خالی *fit, صحیح noffset, ulong *addr,
						ulong *fladdr, ulong *سائز)
{
	ثابت خالی *ڈیٹا;

	اگر (fit_image_get_ڈیٹا(fit, noffset, &ڈیٹا, (size_t *)سائز))
		واپس 1;

	اگر (fit_image_get_بھرو(fit, noffset, (ulong *)fladdr))
		واپس 1;

	*addr = (ulong)ڈیٹا;

	واپس 0;
}

#اگر defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
صحیح update_tftp(ulong addr, حرف *interface, حرف *devstring)
{
	حرف *filename, *env_addr, *fit_image_name;
	ulong update_addr, update_fladdr, update_سائز;
	صحیح images_noffset, ndepth, noffset;
	bool update_tftp_dfu;
	صحیح ret = 0;
	خالی *fit;

	اگر (interface == تہی && devstring == تہی) {
		update_tftp_dfu = false;
	} ورنہ اگر (interface && devstring) {
		update_tftp_dfu = true;
	} ورنہ {
		pr_err("Interface: %s and devstring: %s not supported!\n",
		      interface, devstring);
		واپس -EINVAL;
	}

	/* use already present image */
	اگر (addr)
		جاؤ got_update_file;

	لکھو("Auto-update from TFTP: ");

	/* get the file name of the update file */
	filename = env_get(UPDATE_FILE_ENV);
	اگر (filename == تہی) {
		لکھو("failed, env. variable '%s' not found\n",
							UPDATE_FILE_ENV);
		واپس 1;
	}

	لکھو("trying update file '%s'\n", filename);

	/* get load address of downloaded update file */
	env_addr = env_get("loadaddr");
	اگر (env_addr)
		addr = hextoul(env_addr, تہی);
	ورنہ
		addr = CONFIG_UPDATE_بھرو_ADDR;

	اگر (update_بھرو(filename, CONFIG_UPDATE_TFTP_MSEC_MAX,
					CONFIG_UPDATE_TFTP_CNT_MAX, addr)) {
		لکھو("Can't load update file, aborting auto-update\n");
		واپس 1;
	}

got_update_file:
	fit = map_sysmem(addr, 0);

	اگر (fit_check_format((خالی *)fit, IMAGE_سائز_INVAL)) {
		لکھو("Bad FIT format of the update file, aborting "
							"auto-update\n");
		واپس 1;
	}

	/* process updates */
	images_noffset = fdt_راستہ_انحراف(fit, FIT_IMAGES_راستہ);

	ndepth = 0;
	noffset = fdt_اگلا_گرہ(fit, images_noffset, &ndepth);
	جب (noffset >= 0 && ndepth > 0) {
		اگر (ndepth != 1)
			جاؤ اگلا_گرہ;

		fit_image_name = (حرف *)fit_get_name(fit, noffset, تہی);
		لکھو("Processing update '%s' :", fit_image_name);

		اگر (!fit_image_verify(fit, noffset)) {
			لکھو("Error: invalid update hash, aborting\n");
			ret = 1;
			جاؤ اگلا_گرہ;
		}

		لکھو("\n");
		اگر (update_fit_getparams(fit, noffset, &update_addr,
					&update_fladdr, &update_سائز)) {
			لکھو("Error: can't get update parameters, aborting\n");
			ret = 1;
			جاؤ اگلا_گرہ;
		}

		اگر (!update_tftp_dfu) {
			اگر (update_flash(update_addr, update_fladdr,
					 update_سائز)) {
				لکھو("Error: can't flash update, aborting\n");
				ret = 1;
				جاؤ اگلا_گرہ;
			}
		} ورنہ اگر (fit_image_check_type(fit, noffset,
						IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(خالی *)update_addr,
						update_سائز, interface,
						devstring);
			اگر (ret)
				واپس ret;
		}
اگلا_گرہ:
		noffset = fdt_اگلا_گرہ(fit, noffset, &ndepth);
	}

	واپس ret;
}
#اگر_ختم /* CONFIG_DFU_UPDATE || CONFIG_UPDATE_TFTP */

#اگر_تعریف_شدہ CONFIG_UPDATE_FIT
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
صحیح fit_update(ثابت خالی *fit)
{
	حرف *fit_image_name;
	ulong update_addr, update_fladdr, update_سائز;
	صحیح images_noffset, ndepth, noffset;
	صحیح ret = 0;

	اگر (!fit)
		واپس -EINVAL;

	اگر (fit_check_format((خالی *)fit, IMAGE_سائز_INVAL)) {
		لکھو("Bad FIT format of the update file, aborting auto-update\n");
		واپس -EINVAL;
	}

	/* process updates */
	images_noffset = fdt_راستہ_انحراف(fit, FIT_IMAGES_راستہ);

	ndepth = 0;
	noffset = fdt_اگلا_گرہ(fit, images_noffset, &ndepth);
	جب (noffset >= 0 && ndepth > 0) {
		اگر (ndepth != 1)
			جاؤ اگلا_گرہ;

		fit_image_name = (حرف *)fit_get_name(fit, noffset, تہی);
		لکھو("Processing update '%s' :", fit_image_name);

		اگر (!fit_image_verify(fit, noffset)) {
			لکھو("Error: invalid update hash, aborting\n");
			ret = 1;
			جاؤ اگلا_گرہ;
		}

		لکھو("\n");
		اگر (update_fit_getparams(fit, noffset, &update_addr,
					 &update_fladdr, &update_سائز)) {
			لکھو("Error: can't get update parameters, aborting\n");
			ret = 1;
			جاؤ اگلا_گرہ;
		}

		اگر (fit_image_check_type(fit, noffset, IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(خالی *)update_addr,
						update_سائز, تہی, تہی);
			اگر (ret)
				واپس ret;
		}
اگلا_گرہ:
		noffset = fdt_اگلا_گرہ(fit, noffset, &ndepth);
	}

	واپس ret;
}
#اگر_ختم /* CONFIG_UPDATE_FIT */
