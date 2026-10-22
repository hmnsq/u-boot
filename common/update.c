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
#define UPDATE_FILE_ENV		"updatefile"

خارجي ulong tftp_timeout_ms;
خارجي صحيح tftp_timeout_کيراءن_max;
#ifdef CONFIG_MTD_NOR_FLASH
#ضمن <flash.h>
#ضمن <mtd/cfi_flash.h>
ساكن uchar *saved_prot_info;
#endif
ساكن صحيح update_load(حرف *filename, ulong msec_max, صحيح cnt_max, ulong addr)
{
	صحيح rv, ret;
	ulong saved_timeout_msecs;
	صحيح saved_timeout_کيراءن;
	حرف *saved_netretry, *saved_bootfile;

	rv = 0;
	/* save used globals and env variable */
	saved_timeout_msecs = tftp_timeout_ms;
	saved_timeout_کيراءن = tftp_timeout_کيراءن_max;
	saved_netretry = strdup(env_get("netretry"));
	saved_bootfile = strdup(net_boot_file_name);

	/* set timeouts for auto-update */
	tftp_timeout_ms = msec_max;
	tftp_timeout_کيراءن_max = cnt_max;

	/* we don't want to retry the connection if errors occur */
	env_set("netretry", "no");

	/* download the update file */
	image_load_addr = addr;
	copy_filename(net_boot_file_name, filename, حجم(net_boot_file_name));
	ret = net_loop(TFTPGET);

	إذا (ret < 0)
		rv = 1;
	وإلا
		flush_cache(addr, net_boot_file_size);

	/* restore changed globals and env variable */
	tftp_timeout_ms = saved_timeout_msecs;
	tftp_timeout_کيراءن_max = saved_timeout_کيراءن;

	env_set("netretry", saved_netretry);
	إذا (saved_netretry != عدم)
		حرر(saved_netretry);

	إذا (saved_bootfile != عدم) {
		copy_filename(net_boot_file_name, saved_bootfile,
			      حجم(net_boot_file_name));
		حرر(saved_bootfile);
	}

	أرجع rv;
}

#ifdef CONFIG_MTD_NOR_FLASH
ساكن صحيح update_flash_protect(صحيح prot, ulong addr_first, ulong addr_last)
{
	uchar *sp_info_ptr;
	ulong s;
	صحيح i, bank, cnt;
	flash_info_t *info;

	sp_info_ptr = عدم;

	إذا (prot == 0) {
		saved_prot_info =
			calloc(CFI_FLASH_BANKS * CONFIG_SYS_MAX_FLASH_SECT, 1);
		إذا (!saved_prot_info)
			أرجع 1;
	}

	لكل (bank = 0; bank < CFI_FLASH_BANKS; ++bank) {
		cnt = 0;
		info = &flash_info[bank];

		/* Nothing to do if the bank doesn't exist */
		إذا (info->sector_کيراءن == 0)
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
		إذا (addr_last > info->start[0] + info->size - 1)
			addr_last = info->start[0] + info->size - 1;
		إذا (addr_first < info->start[0])
			addr_first = info->start[0];

		لكل (i = 0; i < info->sector_کيراءن; i++) {
			/* Save current information about protected sectors */
			إذا (prot == 0) {
				s = info->start[i];
				إذا ((s >= addr_first) && (s <= addr_last))
					sp_info_ptr[i] = info->protect[i];

			}

			/* Protect/unprotect sectors */
			إذا (sp_info_ptr[i] == 1) {
#إذا defined(CONFIG_SYS_FLASH_PROTECTION)
				إذا (flash_real_protect(info, i, prot))
					أرجع 1;
#وإلا
				info->protect[i] = prot;
#endif
				cnt++;
			}
		}

		إذا (cnt) {
			printf("%sProtected %d sectors\n",
						prot ? "": "Un-", cnt);
		}
	}

	إذا((prot == 1) && saved_prot_info)
		حرر(saved_prot_info);

	أرجع 0;
}
#endif

ساكن صحيح update_flash(ulong addr_source, ulong addr_first, ulong size)
{
#ifdef CONFIG_MTD_NOR_FLASH
	ulong addr_last = addr_first + size - 1;

	/* round last address to the sector boundary */
	إذا (flash_sect_roundb(&addr_last) > 0)
		أرجع 1;

	إذا (addr_first >= addr_last) {
		printf("Error: end address exceeds addressing space\n");
		أرجع 1;
	}

	/* remove protection on processed sectors */
	إذا (update_flash_protect(0, addr_first, addr_last) > 0) {
		printf("Error: could not unprotect flash sectors\n");
		أرجع 1;
	}

	printf("Erasing 0x%08lx - 0x%08lx", addr_first, addr_last);
	إذا (flash_sect_erase(addr_first, addr_last) > 0) {
		printf("Error: could not erase flash\n");
		أرجع 1;
	}

	printf("Copying to flash...");
	إذا (flash_write((حرف *)addr_source, addr_first, size) > 0) {
		printf("Error: could not copy to flash\n");
		أرجع 1;
	}
	printf("done\n");

	/* enable protection on processed sectors */
	إذا (update_flash_protect(1, addr_first, addr_last) > 0) {
		printf("Error: could not protect flash sectors\n");
		أرجع 1;
	}
#endif
	أرجع 0;
}
#endif /* CONFIG_DFU_TFTP || CONFIG_UPDATE_TFTP */

ساكن صحيح update_fit_getparams(ثابت فراغ *fit, صحيح noffset, ulong *addr,
						ulong *fladdr, ulong *size)
{
	ثابت فراغ *data;

	إذا (fit_image_get_data(fit, noffset, &data, (size_t *)size))
		أرجع 1;

	إذا (fit_image_get_load(fit, noffset, (ulong *)fladdr))
		أرجع 1;

	*addr = (ulong)data;

	أرجع 0;
}

#إذا defined(CONFIG_DFU_TFTP) || defined(CONFIG_UPDATE_TFTP)
صحيح update_tftp(ulong addr, حرف *interface, حرف *devstring)
{
	حرف *filename, *env_addr, *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
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

	printf("Auto-update from TFTP: ");

	/* get the file name of the update file */
	filename = env_get(UPDATE_FILE_ENV);
	إذا (filename == عدم) {
		printf("failed, env. variable '%s' not found\n",
							UPDATE_FILE_ENV);
		أرجع 1;
	}

	printf("trying update file '%s'\n", filename);

	/* get load address of downloaded update file */
	env_addr = env_get("loadaddr");
	إذا (env_addr)
		addr = hextoul(env_addr, عدم);
	وإلا
		addr = CONFIG_UPDATE_LOAD_ADDR;

	إذا (update_load(filename, CONFIG_UPDATE_TFTP_MSEC_MAX,
					CONFIG_UPDATE_TFTP_CNT_MAX, addr)) {
		printf("Can't load update file, aborting auto-update\n");
		أرجع 1;
	}

got_update_file:
	fit = map_sysmem(addr, 0);

	إذا (fit_check_format((فراغ *)fit, IMAGE_SIZE_INVAL)) {
		printf("Bad FIT format of the update file, aborting "
							"auto-update\n");
		أرجع 1;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_نود(fit, images_noffset, &ndepth);
	طالما (noffset >= 0 && ndepth > 0) {
		إذا (ndepth != 1)
			اذهب next_نود;

		fit_image_name = (حرف *)fit_get_name(fit, noffset, عدم);
		printf("Processing update '%s' :", fit_image_name);

		إذا (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			اذهب next_نود;
		}

		printf("\n");
		إذا (update_fit_getparams(fit, noffset, &update_addr,
					&update_fladdr, &update_size)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			اذهب next_نود;
		}

		إذا (!update_tftp_dfu) {
			إذا (update_flash(update_addr, update_fladdr,
					 update_size)) {
				printf("Error: can't flash update, aborting\n");
				ret = 1;
				اذهب next_نود;
			}
		} وإلا إذا (fit_image_check_type(fit, noffset,
						IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(فراغ *)update_addr,
						update_size, interface,
						devstring);
			إذا (ret)
				أرجع ret;
		}
next_نود:
		noffset = fdt_next_نود(fit, noffset, &ndepth);
	}

	أرجع ret;
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
صحيح fit_update(ثابت فراغ *fit)
{
	حرف *fit_image_name;
	ulong update_addr, update_fladdr, update_size;
	صحيح images_noffset, ndepth, noffset;
	صحيح ret = 0;

	إذا (!fit)
		أرجع -EINVAL;

	إذا (fit_check_format((فراغ *)fit, IMAGE_SIZE_INVAL)) {
		printf("Bad FIT format of the update file, aborting auto-update\n");
		أرجع -EINVAL;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_نود(fit, images_noffset, &ndepth);
	طالما (noffset >= 0 && ndepth > 0) {
		إذا (ndepth != 1)
			اذهب next_نود;

		fit_image_name = (حرف *)fit_get_name(fit, noffset, عدم);
		printf("Processing update '%s' :", fit_image_name);

		إذا (!fit_image_verify(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			اذهب next_نود;
		}

		printf("\n");
		إذا (update_fit_getparams(fit, noffset, &update_addr,
					 &update_fladdr, &update_size)) {
			printf("Error: can't get update parameters, aborting\n");
			ret = 1;
			اذهب next_نود;
		}

		إذا (fit_image_check_type(fit, noffset, IH_TYPE_FIRMWARE)) {
			ret = dfu_write_by_name(fit_image_name,
						(فراغ *)update_addr,
						update_size, عدم, عدم);
			إذا (ret)
				أرجع ret;
		}
next_نود:
		noffset = fdt_next_نود(fit, noffset, &ndepth);
	}

	أرجع ret;
}
#endif /* CONFIG_UPDATE_FIT */
