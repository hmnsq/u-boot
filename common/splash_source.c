// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 */

#അന്തര്ഭാവ <bmp_layout.h>
#അന്തര്ഭാവ <command.h>
#അന്തര്ഭാവ <env.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <fs.h>
#അന്തര്ഭാവ <fdt_support.h>
#അന്തര്ഭാവ <image.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <nand.h>
#അന്തര്ഭാവ <sata.h>
#അന്തര്ഭാവ <spi.h>
#അന്തര്ഭാവ <spi_flash.h>
#അന്തര്ഭാവ <splash.h>
#അന്തര്ഭാവ <usb.h>
#അന്തര്ഭാവ <virtio.h>
#അന്തര്ഭാവ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SPI_FLASH
സ്ഥിതം ഘടന spi_flash *sf;
സ്ഥിതം പൂർണ്ണസംഖ്യ splash_sf_read_raw(ulong bmp_load_addr, പൂർണ്ണസംഖ്യ offset, size_t read_size)
{
	എങ്കിൽ (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_SPEED,
				     CONFIG_SF_DEFAULT_MODE);
		എങ്കിൽ (!sf)
			തിരികെ -ENODEV;
	}

	തിരികെ spi_flash_read(sf, offset, read_size, (ശൂന്യം *)(uintptr_t)bmp_load_addr);
}
#അല്ലെങ്കിൽ
സ്ഥിതം പൂർണ്ണസംഖ്യ splash_sf_read_raw(ulong bmp_load_addr, പൂർണ്ണസംഖ്യ offset, size_t read_size)
{
	debug("%s: sf support not available\n", __func__);
	തിരികെ -ENOSYS;
}
#endif

#ifdef CONFIG_CMD_NAND
സ്ഥിതം പൂർണ്ണസംഖ്യ splash_nand_read_raw(ulong bmp_load_addr, പൂർണ്ണസംഖ്യ offset, size_t read_size)
{
	ഘടന mtd_info *mtd = get_nand_dev_by_index(nand_curr_device);
	തിരികെ nand_read_skip_bad(mtd, offset,
				  &read_size, ശൂന്യ,
				  mtd->size,
				  (u_char *)bmp_load_addr);
}
#അല്ലെങ്കിൽ
സ്ഥിതം പൂർണ്ണസംഖ്യ splash_nand_read_raw(ulong bmp_load_addr, പൂർണ്ണസംഖ്യ offset, size_t read_size)
{
	debug("%s: nand support not available\n", __func__);
	തിരികെ -ENOSYS;
}
#endif

സ്ഥിതം പൂർണ്ണസംഖ്യ splash_mmc_read_raw(ulong bmp_load_addr, ഘടന splash_location *location,
			       size_t read_size)
{
	ഘടന disk_partition partition;
	ഘടന blk_desc *desc;
	lbaint_t blkcnt;
	പൂർണ്ണസംഖ്യ ret, n;

	എങ്കിൽ (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		തിരികെ -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_അക്കം("mmc", location->devpart, &desc,
						   &partition, 1);
	എങ്കിൽ (ret < 0)
		തിരികെ ret;

	blkcnt = DIV_ROUND_UP(read_size, partition.blksz);
	n = blk_dread(desc, partition.start, blkcnt, (ശൂന്യം *)(uintptr_t)bmp_load_addr);

	തിരികെ (n == blkcnt) ? 0 : -EIO;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ splash_storage_read_raw(ഘടന splash_location *location,
			       ulong bmp_load_addr, size_t read_size)
{
	u32 offset;

	എങ്കിൽ (!location)
		തിരികെ -EINVAL;

	offset = location->offset;
	തിരഞ്ഞെടുപ്പ് (location->storage) {
	അവസ്ഥ SPLASH_STORAGE_MMC:
		തിരികെ splash_mmc_read_raw(bmp_load_addr, location, read_size);
	അവസ്ഥ SPLASH_STORAGE_NAND:
		തിരികെ splash_nand_read_raw(bmp_load_addr, offset, read_size);
	അവസ്ഥ SPLASH_STORAGE_SF:
		തിരികെ splash_sf_read_raw(bmp_load_addr, offset, read_size);
	സ്ഥിരസ്ഥിതി:
		printf("Unknown splash location\n");
	}

	തിരികെ -EINVAL;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ splash_load_raw(ഘടന splash_location *location, ulong bmp_load_addr)
{
	ഘടന bmp_header *bmp_hdr;
	പൂർണ്ണസംഖ്യ res;
	size_t bmp_size, bmp_header_size = വലുപ്പം(ഘടന bmp_header);

	എങ്കിൽ (bmp_load_addr + bmp_header_size >= gd->start_addr_sp)
		പോകുക splash_address_too_മുകളിലെ;

	res = splash_storage_read_raw(location, bmp_load_addr, bmp_header_size);
	എങ്കിൽ (res < 0)
		തിരികെ res;

	bmp_hdr = (ഘടന bmp_header *)(uintptr_t)bmp_load_addr;
	bmp_size = le32_to_cpu(bmp_hdr->file_size);

	എങ്കിൽ (bmp_load_addr + bmp_size >= gd->start_addr_sp)
		പോകുക splash_address_too_മുകളിലെ;

	തിരികെ splash_storage_read_raw(location, bmp_load_addr, bmp_size);

splash_address_too_മുകളിലെ:
	printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	തിരികെ -EFAULT;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ splash_select_fs_dev(ഘടന splash_location *location)
{
	പൂർണ്ണസംഖ്യ res;

	തിരഞ്ഞെടുപ്പ് (location->storage) {
	അവസ്ഥ SPLASH_STORAGE_MMC:
		res = fs_set_blk_dev("mmc", location->devpart, FS_TYPE_ANY);
		നിർത്തുക;
	അവസ്ഥ SPLASH_STORAGE_USB:
		res = fs_set_blk_dev("usb", location->devpart, FS_TYPE_ANY);
		നിർത്തുക;
	അവസ്ഥ SPLASH_STORAGE_SATA:
		res = fs_set_blk_dev("sata", location->devpart, FS_TYPE_ANY);
		നിർത്തുക;
	അവസ്ഥ SPLASH_STORAGE_NAND:
		എങ്കിൽ (location->ubivol != ശൂന്യ)
			res = fs_set_blk_dev("ubi", ശൂന്യ, FS_TYPE_UBIFS);
		അല്ലെങ്കിൽ
			res = -ENODEV;
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		printf("Error: %s: unsupported location storage.\n", __func__);
		തിരികെ -ENODEV;
	}

	എങ്കിൽ (res)
		printf("Error: %s: could not access storage.\n", __func__);

	തിരികെ res;
}

#ifdef CONFIG_USB_STORAGE
സ്ഥിതം പൂർണ്ണസംഖ്യ splash_init_usb(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ err;

	err = usb_init();
	എങ്കിൽ (err)
		തിരികെ err;

#ifndef CONFIG_DM_USB
	err = usb_stor_scan(1) < 0 ? -ENODEV : 0;
#endif

	തിരികെ err;
}
#അല്ലെങ്കിൽ
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ splash_init_usb(ശൂന്യം)
{
	printf("Cannot load splash image: no USB support\n");
	തിരികെ -ENOSYS;
}
#endif

#ifdef CONFIG_SATA
സ്ഥിതം പൂർണ്ണസംഖ്യ splash_init_sata(ശൂന്യം)
{
	തിരികെ sata_probe(0);
}
#അല്ലെങ്കിൽ
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ splash_init_sata(ശൂന്യം)
{
	printf("Cannot load splash image: no SATA support\n");
	തിരികെ -ENOSYS;
}
#endif

സ്ഥിതം പൂർണ്ണസംഖ്യ splash_init_virtio(ശൂന്യം)
{
	എങ്കിൽ (!IS_ENABLED(CONFIG_VIRTIO)) {
		printf("Cannot load splash image: no virtio support\n");
		തിരികെ -ENOSYS;
	} അല്ലെങ്കിൽ {
		തിരികെ virtio_init();
	}
}

#എങ്കിൽ defined(CONFIG_CMD_UBIFS) && !defined(CONFIG_XPL_BUILD)
സ്ഥിതം പൂർണ്ണസംഖ്യ splash_mount_ubifs(ഘടന splash_location *location)
{
	പൂർണ്ണസംഖ്യ res;
	അക്ഷരം cmd[32];

	sprintf(cmd, "ubi part %s", location->mtdpart);
	res = run_command(cmd, 0);
	എങ്കിൽ (res)
		തിരികെ res;

	sprintf(cmd, "ubifsmount %s", location->ubivol);
	res = run_command(cmd, 0);

	തിരികെ res;
}

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ splash_umount_ubifs(ശൂന്യം)
{
	തിരികെ run_command("ubifsumount", 0);
}
#അല്ലെങ്കിൽ
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ splash_mount_ubifs(ഘടന splash_location *location)
{
	printf("Cannot load splash image: no UBIFS support\n");
	തിരികെ -ENOSYS;
}

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ splash_umount_ubifs(ശൂന്യം)
{
	printf("Cannot unmount UBIFS: no UBIFS support\n");
	തിരികെ -ENOSYS;
}
#endif

#define SPLASH_SOURCE_DEFAULT_FILE_NAME		"splash.bmp"

സ്ഥിതം പൂർണ്ണസംഖ്യ splash_load_fs(ഘടന splash_location *location, ulong bmp_load_addr)
{
	പൂർണ്ണസംഖ്യ res = 0;
	loff_t bmp_size;
	loff_t actread;
	അക്ഷരം *splash_file;

	splash_file = env_get("splashfile");
	എങ്കിൽ (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	എങ്കിൽ (location->storage == SPLASH_STORAGE_USB)
		res = splash_init_usb();

	എങ്കിൽ (location->storage == SPLASH_STORAGE_SATA)
		res = splash_init_sata();

	എങ്കിൽ (location->storage == SPLASH_STORAGE_VIRTIO)
		res = splash_init_virtio();

	എങ്കിൽ (location->ubivol != ശൂന്യ)
		res = splash_mount_ubifs(location);

	എങ്കിൽ (res)
		തിരികെ res;

	res = splash_select_fs_dev(location);
	എങ്കിൽ (res)
		പോകുക out;

	res = fs_size(splash_file, &bmp_size);
	എങ്കിൽ (res) {
		printf("Error: %s: cannot determine file size (%d)\n",
		       __func__, res);
		പോകുക out;
	}

	എങ്കിൽ (bmp_load_addr + bmp_size >= gd->start_addr_sp) {
		printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		പോകുക out;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_load_addr, 0, 0, &actread);

out:
	എങ്കിൽ (location->ubivol != ശൂന്യ)
		splash_umount_ubifs();

	തിരികെ res;
}

/**
 * select_splash_location - return the splash location based on board support
 *			    and env variable "splashsource".
 *
 * @locations:		An array of supported splash locations.
 * @size:		Size of splash_locations array.
 *
 * @return: If a null set of splash locations is given, or
 *	    splashsource env variable is set to unsupported value
 *			return NULL.
 *	    If splashsource env variable is not defined
 *			return the first entry in splash_locations as default.
 *	    If splashsource env variable contains a supported value
 *			return the location selected by splashsource.
 */
സ്ഥിതം ഘടന splash_location *select_splash_location(
			    ഘടന splash_location *locations, uint size)
{
	പൂർണ്ണസംഖ്യ i;
	അക്ഷരം *env_splashsource;

	എങ്കിൽ (!locations || size == 0)
		തിരികെ ശൂന്യ;

	env_splashsource = env_get("splashsource");
	എങ്കിൽ (env_splashsource == ശൂന്യ)
		തിരികെ &locations[0];

	വേണ്ടി (i = 0; i < size; i++) {
		എങ്കിൽ (!strcmp(locations[i].name, env_splashsource))
			തിരികെ &locations[i];
	}

	printf("splashsource env variable set to unsupported value\n");
	തിരികെ ശൂന്യ;
}

#ifdef CONFIG_FIT
സ്ഥിതം പൂർണ്ണസംഖ്യ splash_load_fit(ഘടന splash_location *location, ulong bmp_load_addr)
{
	പൂർണ്ണസംഖ്യ res;
	പൂർണ്ണസംഖ്യ കണ്ണി_offset;
	സ്ഥിരം അക്ഷരം *splash_file;
	സ്ഥിരം ശൂന്യം *internal_splash_data;
	size_t internal_splash_size;
	പൂർണ്ണസംഖ്യ external_splash_addr;
	u32 external_splash_size;
	bool is_splash_external = false;
	ഘടന legacy_img_hdr *img_header;
	സ്ഥിരം u32 *fit_header;
	u32 fit_size;
	സ്ഥിരം size_t header_size = വലുപ്പം(ഘടന legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_load_addr, header_size);
	എങ്കിൽ (res < 0)
		തിരികെ res;

	img_header = (ഘടന legacy_img_hdr *)(uintptr_t)bmp_load_addr;
	എങ്കിൽ (image_get_magic(img_header) != FDT_MAGIC) {
		printf("Could not find FDT magic\n");
		തിരികെ -EINVAL;
	}

	fit_size = fdt_totalsize(img_header);

	/* Read in entire FIT */
	fit_header = (സ്ഥിരം u32 *)(bmp_load_addr + header_size);
	res = splash_storage_read_raw(location, (uintptr_t)fit_header, fit_size);
	എങ്കിൽ (res < 0)
		തിരികെ res;

	res = fit_check_format(fit_header, IMAGE_SIZE_INVAL);
	എങ്കിൽ (res) {
		debug("Could not find valid FIT image\n");
		തിരികെ res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	എങ്കിൽ (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	കണ്ണി_offset = fit_image_get_കണ്ണി(fit_header, splash_file);
	എങ്കിൽ (കണ്ണി_offset < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		തിരികെ -ENOENT;
	}

	/* Extract the splash data from FIT */
	എങ്കിൽ (!fit_image_get_data(fit_header, കണ്ണി_offset, &internal_splash_data,
				&internal_splash_size)) {
		memmove((ശൂന്യം *)(uintptr_t)bmp_load_addr, internal_splash_data,
			internal_splash_size);
	} അല്ലെങ്കിൽ {
		printf("Failed to get splash image from FIT\n");
		തിരികെ -ENODATA;
	}

	എങ്കിൽ (is_splash_external) {
		res = fit_image_get_data_size(fit_header, കണ്ണി_offset, &external_splash_size);
		എങ്കിൽ (res < 0) {
			printf("Failed to get size of splash image (err=%d)\n", res);
			തിരികെ res;
		}

		/* Read in the splash data */
		location->offset = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_load_addr, external_splash_size);
		എങ്കിൽ (res < 0)
			തിരികെ res;
	}

	തിരികെ 0;
}
#endif /* CONFIG_FIT */

/**
 * splash_source_load - load splash image from a supported location.
 *
 * Select a splash image location based on the value of splashsource environment
 * variable and the board supported splash source locations, and load a
 * splashimage to the address pointed to by splashimage environment variable.
 *
 * @locations:		An array of supported splash locations.
 * @size:		Size of splash_locations array.
 *
 * @return: 0 on success, negative value on failure.
 */
പൂർണ്ണസംഖ്യ splash_source_load(ഘടന splash_location *locations, uint size)
{
	ഘടന splash_location *splash_location;
	അക്ഷരം *env_splashimage_മൂല്യം;
	അക്ഷരം *devpart;
	ulong bmp_load_addr;

	env_splashimage_മൂല്യം = env_get("splashimage");
	എങ്കിൽ (env_splashimage_മൂല്യം == ശൂന്യ)
		തിരികെ -ENOENT;

	bmp_load_addr = hextoul(env_splashimage_മൂല്യം, 0);
	എങ്കിൽ (bmp_load_addr == 0) {
		printf("Error: bad splashimage address specified\n");
		തിരികെ -EFAULT;
	}

	splash_location = select_splash_location(locations, size);
	എങ്കിൽ (!splash_location)
		തിരികെ -EINVAL;

	devpart = env_get("splashdevpart");
	എങ്കിൽ (devpart)
		splash_location->devpart = devpart;

	എങ്കിൽ (splash_location->flags == SPLASH_STORAGE_RAW)
		തിരികെ splash_load_raw(splash_location, bmp_load_addr);
	അല്ലെങ്കിൽ എങ്കിൽ (splash_location->flags == SPLASH_STORAGE_FS)
		തിരികെ splash_load_fs(splash_location, bmp_load_addr);
#ifdef CONFIG_FIT
	അല്ലെങ്കിൽ എങ്കിൽ (splash_location->flags == SPLASH_STORAGE_FIT)
		തിരികെ splash_load_fit(splash_location, bmp_load_addr);
#endif
	തിരികെ -EINVAL;
}
