// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Semihalf
 *
 * Written by: Rafal Czubak <rcz@semihalf.com>
 *             Bartlomiej Sieka <tur@semihalf.com>
 */

#ضمن <cpu_func.h>
#ضمن <image.h>
#ضمن <linux/printk.h>

#ضمن <command.h>
#ضمن <env.h>
#ضمن <net.h>
#ضمن <net/tftp.h>
#ضمن <malloc.h>
#ضمن <mapmem.h>
#ضمن <dfu.h>
#ضمن <errno.h>

#إذا defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
/* env variable holding the location of the update file */
#تعریف_کن UPDATE_FILE_ENV		"updatefile"

خارجي ulong tftp_timeout_ms;
خارجي صحيح tftp_timeout_شمار_max;
#اگر_تعریف_شده CONFIG_MTD_NOR_FLASH
#ضمن <flash.h>
#ضمن <mtd/cfi_flash.h>
ساكن uchar *saved_prot_info;
#پایان_اگر
ساكن صحيح update_بارگیری(حرف *filename, ulong msec_max, صحيح cnt_max, ulong addr)
{
	صحيح rv, ret;
	ulong saved_timeout_msecs;
	صحيح saved_timeout_شمار;
	حرف *saved_netretry, *saved_bootfile;

	rv = 0;
	/* save used globals and env variable */
	saved_timeout_msecs = tftp_timeout_ms;
	saved_timeout_شمار = tftp_timeout_شمار_max;
	saved_netretry = strdup(env_get("netretry"));
	saved_bootfile = strdup(net_boot_file_name);

	/* set timeouts for auto-update */
	tftp_timeout_ms = msec_max;
	tftp_timeout_شمار_max = cnt_max;

	/* we don't want to retry the connection if errors occur */
	env_مجموعه("netretry", "no");

	/* download the update file */
	image_بارگیری_addr = addr;
	copy_filename(net_boot_file_name, filename, حجم(net_boot_file_name));
	ret = net_loop(TFTPGET);

	إذا (ret < 0)
		rv = 1;
	وإلا
		flush_cache(addr, net_boot_file_اندازه);

	/* restore changed globals and env variable */
	tftp_timeout_ms = saved_timeout_msecs;
	tftp_timeout_شمار_max = saved_timeout_شمار;

	env_مجموعه("netretry", saved_netretry);
	إذا (saved_netretry != عدم)
		حرر(saved_netretry);

	إذا (saved_bootfile != عدم) {
		copy_filename(net_boot_file_name, saved_bootfile,
			      حجم(net_boot_file_name));
		حرر(saved_bootfile);
	}

	أرجع rv;
}

#اگر_تعریف_شده CONFIG_MTD_NOR_FLASH
ساكن صحيح update_flash_protect(صحيح prot, ulong addr_اولی, ulong addr_آخرین)
{
	uchar *sp_info_ptr;
	ulong s;
	صحيح i, bank, cnt;
	flash_info_t *info;

	sp_info_ptr = عدم;

	إذا (prot == 0) {
		saved_prot_info =
			حافظه_صفر_بده(CFI_FLASH_BANKS * CONFIG_SYS_MAX_FLASH_SECT, 1);
		إذا (!saved_prot_info)
			أرجع 1;
	}

	لكل (bank = 0; bank < CFI_FLASH_BANKS; ++bank) {
		cnt = 0;
		info = &flash_info[bank];

		/* Nothing to do if the bank doesn't exist */
		إذا (info->sector_شمار == 0)
			أرجع 0;

		/* Point to current bank protection information */
		sp_info_ptr = saved_prot_info + (bank * CONFIG_SYS_MAX_FLASH_SECT);

		/*
		 * Adjust addr_first or addr_last if we are on bank boundary.
		 * Address space between banks must be continuous for other
		 * flash functions (like flash_sect_erase or flash_write) to
		 * succeed. Banks must also be numbered in correct order,
		 * according to increasing addresses.
		 */
		إذا (addr_آخرین > info->شروع[0] + info->اندازه - 1)
			addr_آخرین = info->شروع[0] + info->اندازه - 1;
		إذا (addr_اولی < info->شروع[0])
			addr_اولی = info->شروع[0];

		لكل (i = 0; i < info->sector_شمار; i++) {
			/* Save current information about protected sectors */
			إذا (prot == 0) {
				s = info->شروع[i];
				إذا ((s >= addr_اولی) && (s <= addr_آخرین))
					sp_info_ptr[i] = info->protect[i];

			}

			/* Protect/unprotect sectors */
			إذا (sp_info_ptr[i] == 1) {
#إذا defined(CONFIG_SYS_FLASH_PROTECTION)
				إذا (flash_real_protect(info, i, prot))
					أرجع 1;
#وإلا
				info->protect[i] = prot;
#پایان_اگر
				cnt++;
			}
		}

		إذا (cnt) {
			چاپ_کن("%sProtected %d sectors\n",
						prot ? "": "Un-", cnt);
		}
	}

	إذا((prot == 1) && saved_prot_info)
		حرر(saved_prot_info);

	أرجع 0;
}
#پایان_اگر

ساكن صحيح update_flash(ulong addr_منبع, ulong addr_اولی, ulong اندازه)
{
#اگر_تعریف_شده CONFIG_MTD_NOR_FLASH
	ulong addr_آخرین = addr_اولی + اندازه - 1;

	/* round last address to the sector boundary */
	إذا (flash_sect_roundb(&addr_آخرین) > 0)
		أرجع 1;

	إذا (addr_اولی >= addr_آخرین) {
		چاپ_کن("Error: end address exceeds addressing space\n");
		أرجع 1;
	}

	/* remove protection on processed sectors */
	إذا (update_flash_protect(0, addr_اولی, addr_آخرین) > 0) {
		چاپ_کن("Error: could not unprotect flash sectors\n");
		أرجع 1;
	}

	چاپ_کن("Erasing 0x%08lx - 0x%08lx", addr_اولی, addr_آخرین);
	إذا (flash_sect_erase(addr_اولی, addr_آخرین) > 0) {
		چاپ_کن("Error: could not erase flash\n");
		أرجع 1;
	}

	چاپ_کن("Copying to flash...");
	إذا (flash_write((حرف *)addr_منبع, addr_اولی, اندازه) > 0) {
		چاپ_کن("Error: could not copy to flash\n");
		أرجع 1;
	}
	چاپ_کن("done\n");

	/* enable protection on processed sectors */
	إذا (update_flash_protect(1, addr_اولی, addr_آخرین) > 0) {
		چاپ_کن("Error: could not protect flash sectors\n");
		أرجع 1;
	}
#پایان_اگر
	أرجع 0;
}
#پایان_اگر /* CONFIG_DFU_TFTP || CONFIG_UPDATE_TFTP */

ساكن صحيح update_fit_getparams(ثابت فراغ *fit, صحيح noffset, ulong *addr,
						ulong *fladdr, ulong *اندازه)
{
	ثابت فراغ *داده;

	إذا (fit_image_get_داده(fit, noffset, &داده, (size_t *)اندازه))
		أرجع 1;

	إذا (fit_image_get_بارگیری(fit, noffset, (ulong *)fladdr))
		أرجع 1;

	*addr = (ulong)داده;

	أرجع 0;
}

#إذا defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
صحيح update_tftp(ulong addr, حرف *interface, حرف *devstring)
{
	حرف *filename, *env_addr, *fit_image_name;
	ulong update_addr, update_fladdr, update_اندازه;
	صحيح images_noffset, ndepth, noffset;
	bool update_tftp_dfu;
	صحيح ret = 0;
	فراغ *fit;

	إذا (interface == عدم && devstring == عدم) {
		update_tftp_dfu = false;
	} وإلا إذا (interface && devstring) {
		update_tftp_dfu = true;
	} وإلا {
		pr_err("Interface: %s and devstring: %s not supported!\n",
		      interface, devstring);
		أرجع -EINVAL;
	}

	/* use already present image */
	إذا (addr)
		اذهب got_update_file;

	چاپ_کن("Auto-update from TFTP: ");

	/* get the file name of the update file */
	filename = env_get(UPDATE_FILE_ENV);
	إذا (filename == عدم) {
		چاپ_کن("failed, env. variable '%s' not found\n",
							UPDATE_FILE_ENV);
		أرجع 1;
	}

	چاپ_کن("trying update file '%s'\n", filename);

	/* get load address of downloaded update file */
	env_addr = env_get("loadaddr");
	إذا (env_addr)
		addr = hextoul(env_addr, عدم);
	وإلا
		addr = CONFIG_UPDATE_بارگیری_ADDR;

	إذا (update_بارگیری(filename, CONFIG_UPDATE_TFTP_MSEC_MAX,
					CONFIG_UPDATE_TFTP_CNT_MAX, addr)) {
		چاپ_کن("Can't load update file, aborting auto-update\n");
		أرجع 1;
	}

got_update_file:
	fit = map_sysmem(addr, 0);

	إذا (fit_check_format((فراغ *)fit, IMAGE_اندازه_INVAL)) {
		چاپ_کن("Bad FIT format of the update file, aborting "
							"auto-update\n");
		أرجع 1;
	}

	/* process updates */
	images_noffset = fdt_مسیر_جابه_جایی(fit, FIT_IMAGES_مسیر);

	ndepth = 0;
	noffset = fdt_بعدی_گره(fit, images_noffset, &ndepth);
	طالما (noffset >= 0 && ndepth > 0) {
		إذا (ndepth != 1)
			اذهب بعدی_گره;

		fit_image_name = (حرف *)fit_get_name(fit, noffset, عدم);
		چاپ_کن("Processing update '%s' :", fit_image_name);

		إذا (!fit_image_verify(fit, noffset)) {
			چاپ_کن("Error: invalid update hash, aborting\n");
			ret = 1;
			اذهب بعدی_گره;
		}

		چاپ_کن("\n");
		إذا (update_fit_getparams(fit, noffset, &update_addr,
					&update_fladdr, &update_اندازه)) {
			چاپ_کن("Error: can't get update parameters, aborting\n");
			ret = 1;
			اذهب بعدی_گره;
		}

		إذا (!update_tftp_dfu) {
			إذا (update_flash(update_addr, update_fladdr,
					 update_اندازه)) {
				چاپ_کن("Error: can't flash update, aborting\n");
				ret = 1;
				اذهب بعدی_گره;
			}
		} وإلا إذا (fit_image_check_type(fit, noffset,
						IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(فراغ *)update_addr,
						update_اندازه, interface,
						devstring);
			إذا (ret)
				أرجع ret;
		}
بعدی_گره:
		noffset = fdt_بعدی_گره(fit, noffset, &ndepth);
	}

	أرجع ret;
}
#پایان_اگر /* CONFIG_DFU_UPDATE || CONFIG_UPDATE_TFTP */

#اگر_تعریف_شده CONFIG_UPDATE_FIT
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
صحيح fit_update(ثابت فراغ *fit)
{
	حرف *fit_image_name;
	ulong update_addr, update_fladdr, update_اندازه;
	صحيح images_noffset, ndepth, noffset;
	صحيح ret = 0;

	إذا (!fit)
		أرجع -EINVAL;

	إذا (fit_check_format((فراغ *)fit, IMAGE_اندازه_INVAL)) {
		چاپ_کن("Bad FIT format of the update file, aborting auto-update\n");
		أرجع -EINVAL;
	}

	/* process updates */
	images_noffset = fdt_مسیر_جابه_جایی(fit, FIT_IMAGES_مسیر);

	ndepth = 0;
	noffset = fdt_بعدی_گره(fit, images_noffset, &ndepth);
	طالما (noffset >= 0 && ndepth > 0) {
		إذا (ndepth != 1)
			اذهب بعدی_گره;

		fit_image_name = (حرف *)fit_get_name(fit, noffset, عدم);
		چاپ_کن("Processing update '%s' :", fit_image_name);

		إذا (!fit_image_verify(fit, noffset)) {
			چاپ_کن("Error: invalid update hash, aborting\n");
			ret = 1;
			اذهب بعدی_گره;
		}

		چاپ_کن("\n");
		إذا (update_fit_getparams(fit, noffset, &update_addr,
					 &update_fladdr, &update_اندازه)) {
			چاپ_کن("Error: can't get update parameters, aborting\n");
			ret = 1;
			اذهب بعدی_گره;
		}

		إذا (fit_image_check_type(fit, noffset, IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(فراغ *)update_addr,
						update_اندازه, عدم, عدم);
			إذا (ret)
				أرجع ret;
		}
بعدی_گره:
		noffset = fdt_بعدی_گره(fit, noffset, &ndepth);
	}

	أرجع ret;
}
#پایان_اگر /* CONFIG_UPDATE_FIT */
