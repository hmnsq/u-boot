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
#وصف_ڪريو UPDATE_FILE_ENV		"updatefile"

خارجي ulong tftp_timeout_ms;
خارجي صحيح tftp_timeout_ڳڻپ_max;
#جيڪڏهن_وصف_ٿيل CONFIG_MTD_NOR_FLASH
#ضمن <flash.h>
#ضمن <mtd/cfi_flash.h>
ساكن uchar *saved_prot_info;
#جيڪڏهن_پورو
ساكن صحيح update_ڀريو(حرف *filename, ulong msec_max, صحيح cnt_max, ulong addr)
{
	صحيح rv, ret;
	ulong saved_timeout_msecs;
	صحيح saved_timeout_ڳڻپ;
	حرف *saved_netretry, *saved_bootfile;

	rv = 0;
	/* save used globals and env variable */
	saved_timeout_msecs = tftp_timeout_ms;
	saved_timeout_ڳڻپ = tftp_timeout_ڳڻپ_max;
	saved_netretry = strdup(env_get("netretry"));
	saved_bootfile = strdup(net_boot_file_name);

	/* set timeouts for auto-update */
	tftp_timeout_ms = msec_max;
	tftp_timeout_ڳڻپ_max = cnt_max;

	/* we don't want to retry the connection if errors occur */
	env_سيٽ("netretry", "no");

	/* download the update file */
	image_ڀريو_addr = addr;
	copy_filename(net_boot_file_name, filename, حجم(net_boot_file_name));
	ret = net_loop(TFTPGET);

	إذا (ret < 0)
		rv = 1;
	وإلا
		flush_cache(addr, net_boot_file_سائيز);

	/* restore changed globals and env variable */
	tftp_timeout_ms = saved_timeout_msecs;
	tftp_timeout_ڳڻپ_max = saved_timeout_ڳڻپ;

	env_سيٽ("netretry", saved_netretry);
	إذا (saved_netretry != عدم)
		حرر(saved_netretry);

	إذا (saved_bootfile != عدم) {
		copy_filename(net_boot_file_name, saved_bootfile,
			      حجم(net_boot_file_name));
		حرر(saved_bootfile);
	}

	أرجع rv;
}

#جيڪڏهن_وصف_ٿيل CONFIG_MTD_NOR_FLASH
ساكن صحيح update_flash_protect(صحيح prot, ulong addr_پهريون, ulong addr_آخري)
{
	uchar *sp_info_ptr;
	ulong s;
	صحيح i, bank, cnt;
	flash_info_t *info;

	sp_info_ptr = عدم;

	إذا (prot == 0) {
		saved_prot_info =
			صفر_يادگيري_ڏيو(CFI_FLASH_BANKS * CONFIG_SYS_MAX_FLASH_SECT, 1);
		إذا (!saved_prot_info)
			أرجع 1;
	}

	لكل (bank = 0; bank < CFI_FLASH_BANKS; ++bank) {
		cnt = 0;
		info = &flash_info[bank];

		/* Nothing to do if the bank doesn't exist */
		إذا (info->sector_ڳڻپ == 0)
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
		إذا (addr_آخري > info->شروعات[0] + info->سائيز - 1)
			addr_آخري = info->شروعات[0] + info->سائيز - 1;
		إذا (addr_پهريون < info->شروعات[0])
			addr_پهريون = info->شروعات[0];

		لكل (i = 0; i < info->sector_ڳڻپ; i++) {
			/* Save current information about protected sectors */
			إذا (prot == 0) {
				s = info->شروعات[i];
				إذا ((s >= addr_پهريون) && (s <= addr_آخري))
					sp_info_ptr[i] = info->protect[i];

			}

			/* Protect/unprotect sectors */
			إذا (sp_info_ptr[i] == 1) {
#إذا defined(CONFIG_SYS_FLASH_PROTECTION)
				إذا (flash_real_protect(info, i, prot))
					أرجع 1;
#وإلا
				info->protect[i] = prot;
#جيڪڏهن_پورو
				cnt++;
			}
		}

		إذا (cnt) {
			لکو("%sProtected %d sectors\n",
						prot ? "": "Un-", cnt);
		}
	}

	إذا((prot == 1) && saved_prot_info)
		حرر(saved_prot_info);

	أرجع 0;
}
#جيڪڏهن_پورو

ساكن صحيح update_flash(ulong addr_ذريعو, ulong addr_پهريون, ulong سائيز)
{
#جيڪڏهن_وصف_ٿيل CONFIG_MTD_NOR_FLASH
	ulong addr_آخري = addr_پهريون + سائيز - 1;

	/* round last address to the sector boundary */
	إذا (flash_sect_roundb(&addr_آخري) > 0)
		أرجع 1;

	إذا (addr_پهريون >= addr_آخري) {
		لکو("Error: end address exceeds addressing space\n");
		أرجع 1;
	}

	/* remove protection on processed sectors */
	إذا (update_flash_protect(0, addr_پهريون, addr_آخري) > 0) {
		لکو("Error: could not unprotect flash sectors\n");
		أرجع 1;
	}

	لکو("Erasing 0x%08lx - 0x%08lx", addr_پهريون, addr_آخري);
	إذا (flash_sect_erase(addr_پهريون, addr_آخري) > 0) {
		لکو("Error: could not erase flash\n");
		أرجع 1;
	}

	لکو("Copying to flash...");
	إذا (flash_write((حرف *)addr_ذريعو, addr_پهريون, سائيز) > 0) {
		لکو("Error: could not copy to flash\n");
		أرجع 1;
	}
	لکو("done\n");

	/* enable protection on processed sectors */
	إذا (update_flash_protect(1, addr_پهريون, addr_آخري) > 0) {
		لکو("Error: could not protect flash sectors\n");
		أرجع 1;
	}
#جيڪڏهن_پورو
	أرجع 0;
}
#جيڪڏهن_پورو /* CONFIG_DFU_TFTP || CONFIG_UPDATE_TFTP */

ساكن صحيح update_fit_getparams(ثابت فراغ *fit, صحيح noffset, ulong *addr,
						ulong *fladdr, ulong *سائيز)
{
	ثابت فراغ *معلومات;

	إذا (fit_image_get_معلومات(fit, noffset, &معلومات, (size_t *)سائيز))
		أرجع 1;

	إذا (fit_image_get_ڀريو(fit, noffset, (ulong *)fladdr))
		أرجع 1;

	*addr = (ulong)معلومات;

	أرجع 0;
}

#إذا defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
صحيح update_tftp(ulong addr, حرف *interface, حرف *devstring)
{
	حرف *filename, *env_addr, *fit_image_name;
	ulong update_addr, update_fladdr, update_سائيز;
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

	لکو("Auto-update from TFTP: ");

	/* get the file name of the update file */
	filename = env_get(UPDATE_FILE_ENV);
	إذا (filename == عدم) {
		لکو("failed, env. variable '%s' not found\n",
							UPDATE_FILE_ENV);
		أرجع 1;
	}

	لکو("trying update file '%s'\n", filename);

	/* get load address of downloaded update file */
	env_addr = env_get("loadaddr");
	إذا (env_addr)
		addr = hextoul(env_addr, عدم);
	وإلا
		addr = CONFIG_UPDATE_ڀريو_ADDR;

	إذا (update_ڀريو(filename, CONFIG_UPDATE_TFTP_MSEC_MAX,
					CONFIG_UPDATE_TFTP_CNT_MAX, addr)) {
		لکو("Can't load update file, aborting auto-update\n");
		أرجع 1;
	}

got_update_file:
	fit = map_sysmem(addr, 0);

	إذا (fit_check_format((فراغ *)fit, IMAGE_سائيز_INVAL)) {
		لکو("Bad FIT format of the update file, aborting "
							"auto-update\n");
		أرجع 1;
	}

	/* process updates */
	images_noffset = fdt_رستو_وٿي(fit, FIT_IMAGES_رستو);

	ndepth = 0;
	noffset = fdt_ايندڙ_ڳنڍ(fit, images_noffset, &ndepth);
	طالما (noffset >= 0 && ndepth > 0) {
		إذا (ndepth != 1)
			اذهب ايندڙ_ڳنڍ;

		fit_image_name = (حرف *)fit_get_name(fit, noffset, عدم);
		لکو("Processing update '%s' :", fit_image_name);

		إذا (!fit_image_verify(fit, noffset)) {
			لکو("Error: invalid update hash, aborting\n");
			ret = 1;
			اذهب ايندڙ_ڳنڍ;
		}

		لکو("\n");
		إذا (update_fit_getparams(fit, noffset, &update_addr,
					&update_fladdr, &update_سائيز)) {
			لکو("Error: can't get update parameters, aborting\n");
			ret = 1;
			اذهب ايندڙ_ڳنڍ;
		}

		إذا (!update_tftp_dfu) {
			إذا (update_flash(update_addr, update_fladdr,
					 update_سائيز)) {
				لکو("Error: can't flash update, aborting\n");
				ret = 1;
				اذهب ايندڙ_ڳنڍ;
			}
		} وإلا إذا (fit_image_check_type(fit, noffset,
						IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(فراغ *)update_addr,
						update_سائيز, interface,
						devstring);
			إذا (ret)
				أرجع ret;
		}
ايندڙ_ڳنڍ:
		noffset = fdt_ايندڙ_ڳنڍ(fit, noffset, &ndepth);
	}

	أرجع ret;
}
#جيڪڏهن_پورو /* CONFIG_DFU_UPDATE || CONFIG_UPDATE_TFTP */

#جيڪڏهن_وصف_ٿيل CONFIG_UPDATE_FIT
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
	ulong update_addr, update_fladdr, update_سائيز;
	صحيح images_noffset, ndepth, noffset;
	صحيح ret = 0;

	إذا (!fit)
		أرجع -EINVAL;

	إذا (fit_check_format((فراغ *)fit, IMAGE_سائيز_INVAL)) {
		لکو("Bad FIT format of the update file, aborting auto-update\n");
		أرجع -EINVAL;
	}

	/* process updates */
	images_noffset = fdt_رستو_وٿي(fit, FIT_IMAGES_رستو);

	ndepth = 0;
	noffset = fdt_ايندڙ_ڳنڍ(fit, images_noffset, &ndepth);
	طالما (noffset >= 0 && ndepth > 0) {
		إذا (ndepth != 1)
			اذهب ايندڙ_ڳنڍ;

		fit_image_name = (حرف *)fit_get_name(fit, noffset, عدم);
		لکو("Processing update '%s' :", fit_image_name);

		إذا (!fit_image_verify(fit, noffset)) {
			لکو("Error: invalid update hash, aborting\n");
			ret = 1;
			اذهب ايندڙ_ڳنڍ;
		}

		لکو("\n");
		إذا (update_fit_getparams(fit, noffset, &update_addr,
					 &update_fladdr, &update_سائيز)) {
			لکو("Error: can't get update parameters, aborting\n");
			ret = 1;
			اذهب ايندڙ_ڳنڍ;
		}

		إذا (fit_image_check_type(fit, noffset, IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(فراغ *)update_addr,
						update_سائيز, عدم, عدم);
			إذا (ret)
				أرجع ret;
		}
ايندڙ_ڳنڍ:
		noffset = fdt_ايندڙ_ڳنڍ(fit, noffset, &ndepth);
	}

	أرجع ret;
}
#جيڪڏهن_پورو /* CONFIG_UPDATE_FIT */
