// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 */

#অন্তর্ভাৱ <bmp_layout.h>
#অন্তর্ভাৱ <command.h>
#অন্তর্ভাৱ <env.h>
#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <fs.h>
#অন্তর্ভাৱ <fdt_support.h>
#অন্তর্ভাৱ <image.h>
#অন্তর্ভাৱ <log.h>
#অন্তর্ভাৱ <nand.h>
#অন্তর্ভাৱ <sata.h>
#অন্তর্ভাৱ <spi.h>
#অন্তর্ভাৱ <spi_flash.h>
#অন্তর্ভাৱ <splash.h>
#অন্তর্ভাৱ <usb.h>
#অন্তর্ভাৱ <virtio.h>
#অন্তর্ভাৱ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SPI_FLASH
স্থির সংরচনা spi_flash *sf;
স্থির পূর্ণাঙ্ক splash_sf_read_raw(ulong bmp_load_addr, পূর্ণাঙ্ক offset, size_t read_size)
{
	যদি (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_SPEED,
				     CONFIG_SF_DEFAULT_MODE);
		যদি (!sf)
			ঘূৰাই -ENODEV;
	}

	ঘূৰাই spi_flash_read(sf, offset, read_size, (শূন্য *)(uintptr_t)bmp_load_addr);
}
#অন্যথা
স্থির পূর্ণাঙ্ক splash_sf_read_raw(ulong bmp_load_addr, পূর্ণাঙ্ক offset, size_t read_size)
{
	debug("%s: sf support not available\n", __func__);
	ঘূৰাই -ENOSYS;
}
#endif

#ifdef CONFIG_CMD_NAND
স্থির পূর্ণাঙ্ক splash_nand_read_raw(ulong bmp_load_addr, পূর্ণাঙ্ক offset, size_t read_size)
{
	সংরচনা mtd_info *mtd = get_nand_dev_by_index(nand_curr_device);
	ঘূৰাই nand_read_skip_bad(mtd, offset,
				  &read_size, রিক্ত,
				  mtd->size,
				  (u_char *)bmp_load_addr);
}
#অন্যথা
স্থির পূর্ণাঙ্ক splash_nand_read_raw(ulong bmp_load_addr, পূর্ণাঙ্ক offset, size_t read_size)
{
	debug("%s: nand support not available\n", __func__);
	ঘূৰাই -ENOSYS;
}
#endif

স্থির পূর্ণাঙ্ক splash_mmc_read_raw(ulong bmp_load_addr, সংরচনা splash_location *location,
			       size_t read_size)
{
	সংরচনা disk_partition partition;
	সংরচনা blk_desc *desc;
	lbaint_t blkcnt;
	পূর্ণাঙ্ক ret, n;

	যদি (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		ঘূৰাই -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_অঙ্ক("mmc", location->devpart, &desc,
						   &partition, 1);
	যদি (ret < 0)
		ঘূৰাই ret;

	blkcnt = DIV_ROUND_UP(read_size, partition.blksz);
	n = blk_dread(desc, partition.start, blkcnt, (শূন্য *)(uintptr_t)bmp_load_addr);

	ঘূৰাই (n == blkcnt) ? 0 : -EIO;
}

স্থির পূর্ণাঙ্ক splash_storage_read_raw(সংরচনা splash_location *location,
			       ulong bmp_load_addr, size_t read_size)
{
	u32 offset;

	যদি (!location)
		ঘূৰাই -EINVAL;

	offset = location->offset;
	ৱিকল্প (location->storage) {
	পক্ষ SPLASH_STORAGE_MMC:
		ঘূৰাই splash_mmc_read_raw(bmp_load_addr, location, read_size);
	পক্ষ SPLASH_STORAGE_NAND:
		ঘূৰাই splash_nand_read_raw(bmp_load_addr, offset, read_size);
	পক্ষ SPLASH_STORAGE_SF:
		ঘূৰাই splash_sf_read_raw(bmp_load_addr, offset, read_size);
	স্ৱভাৱ:
		printf("Unknown splash location\n");
	}

	ঘূৰাই -EINVAL;
}

স্থির পূর্ণাঙ্ক splash_load_raw(সংরচনা splash_location *location, ulong bmp_load_addr)
{
	সংরচনা bmp_header *bmp_hdr;
	পূর্ণাঙ্ক res;
	size_t bmp_size, bmp_header_size = পরিমাণ(সংরচনা bmp_header);

	যদি (bmp_load_addr + bmp_header_size >= gd->start_addr_sp)
		গচ্ছ splash_address_too_উপৰেৰ;

	res = splash_storage_read_raw(location, bmp_load_addr, bmp_header_size);
	যদি (res < 0)
		ঘূৰাই res;

	bmp_hdr = (সংরচনা bmp_header *)(uintptr_t)bmp_load_addr;
	bmp_size = le32_to_cpu(bmp_hdr->file_size);

	যদি (bmp_load_addr + bmp_size >= gd->start_addr_sp)
		গচ্ছ splash_address_too_উপৰেৰ;

	ঘূৰাই splash_storage_read_raw(location, bmp_load_addr, bmp_size);

splash_address_too_উপৰেৰ:
	printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	ঘূৰাই -EFAULT;
}

স্থির পূর্ণাঙ্ক splash_select_fs_dev(সংরচনা splash_location *location)
{
	পূর্ণাঙ্ক res;

	ৱিকল্প (location->storage) {
	পক্ষ SPLASH_STORAGE_MMC:
		res = fs_set_blk_dev("mmc", location->devpart, FS_TYPE_ANY);
		ভঙ্গ;
	পক্ষ SPLASH_STORAGE_USB:
		res = fs_set_blk_dev("usb", location->devpart, FS_TYPE_ANY);
		ভঙ্গ;
	পক্ষ SPLASH_STORAGE_SATA:
		res = fs_set_blk_dev("sata", location->devpart, FS_TYPE_ANY);
		ভঙ্গ;
	পক্ষ SPLASH_STORAGE_NAND:
		যদি (location->ubivol != রিক্ত)
			res = fs_set_blk_dev("ubi", রিক্ত, FS_TYPE_UBIFS);
		অন্যথা
			res = -ENODEV;
		ভঙ্গ;
	স্ৱভাৱ:
		printf("Error: %s: unsupported location storage.\n", __func__);
		ঘূৰাই -ENODEV;
	}

	যদি (res)
		printf("Error: %s: could not access storage.\n", __func__);

	ঘূৰাই res;
}

#ifdef CONFIG_USB_STORAGE
স্থির পূর্ণাঙ্ক splash_init_usb(শূন্য)
{
	পূর্ণাঙ্ক err;

	err = usb_init();
	যদি (err)
		ঘূৰাই err;

#ifndef CONFIG_DM_USB
	err = usb_stor_scan(1) < 0 ? -ENODEV : 0;
#endif

	ঘূৰাই err;
}
#অন্যথা
স্থির অন্তঃস্থ পূর্ণাঙ্ক splash_init_usb(শূন্য)
{
	printf("Cannot load splash image: no USB support\n");
	ঘূৰাই -ENOSYS;
}
#endif

#ifdef CONFIG_SATA
স্থির পূর্ণাঙ্ক splash_init_sata(শূন্য)
{
	ঘূৰাই sata_probe(0);
}
#অন্যথা
স্থির অন্তঃস্থ পূর্ণাঙ্ক splash_init_sata(শূন্য)
{
	printf("Cannot load splash image: no SATA support\n");
	ঘূৰাই -ENOSYS;
}
#endif

স্থির পূর্ণাঙ্ক splash_init_virtio(শূন্য)
{
	যদি (!IS_ENABLED(CONFIG_VIRTIO)) {
		printf("Cannot load splash image: no virtio support\n");
		ঘূৰাই -ENOSYS;
	} অন্যথা {
		ঘূৰাই virtio_init();
	}
}

#যদি defined(CONFIG_CMD_UBIFS) && !defined(CONFIG_XPL_BUILD)
স্থির পূর্ণাঙ্ক splash_mount_ubifs(সংরচনা splash_location *location)
{
	পূর্ণাঙ্ক res;
	ৱর্ণ cmd[32];

	sprintf(cmd, "ubi part %s", location->mtdpart);
	res = run_command(cmd, 0);
	যদি (res)
		ঘূৰাই res;

	sprintf(cmd, "ubifsmount %s", location->ubivol);
	res = run_command(cmd, 0);

	ঘূৰাই res;
}

স্থির অন্তঃস্থ পূর্ণাঙ্ক splash_umount_ubifs(শূন্য)
{
	ঘূৰাই run_command("ubifsumount", 0);
}
#অন্যথা
স্থির অন্তঃস্থ পূর্ণাঙ্ক splash_mount_ubifs(সংরচনা splash_location *location)
{
	printf("Cannot load splash image: no UBIFS support\n");
	ঘূৰাই -ENOSYS;
}

স্থির অন্তঃস্থ পূর্ণাঙ্ক splash_umount_ubifs(শূন্য)
{
	printf("Cannot unmount UBIFS: no UBIFS support\n");
	ঘূৰাই -ENOSYS;
}
#endif

#define SPLASH_SOURCE_DEFAULT_FILE_NAME		"splash.bmp"

স্থির পূর্ণাঙ্ক splash_load_fs(সংরচনা splash_location *location, ulong bmp_load_addr)
{
	পূর্ণাঙ্ক res = 0;
	loff_t bmp_size;
	loff_t actread;
	ৱর্ণ *splash_file;

	splash_file = env_get("splashfile");
	যদি (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	যদি (location->storage == SPLASH_STORAGE_USB)
		res = splash_init_usb();

	যদি (location->storage == SPLASH_STORAGE_SATA)
		res = splash_init_sata();

	যদি (location->storage == SPLASH_STORAGE_VIRTIO)
		res = splash_init_virtio();

	যদি (location->ubivol != রিক্ত)
		res = splash_mount_ubifs(location);

	যদি (res)
		ঘূৰাই res;

	res = splash_select_fs_dev(location);
	যদি (res)
		গচ্ছ out;

	res = fs_size(splash_file, &bmp_size);
	যদি (res) {
		printf("Error: %s: cannot determine file size (%d)\n",
		       __func__, res);
		গচ্ছ out;
	}

	যদি (bmp_load_addr + bmp_size >= gd->start_addr_sp) {
		printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		গচ্ছ out;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_load_addr, 0, 0, &actread);

out:
	যদি (location->ubivol != রিক্ত)
		splash_umount_ubifs();

	ঘূৰাই res;
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
স্থির সংরচনা splash_location *select_splash_location(
			    সংরচনা splash_location *locations, uint size)
{
	পূর্ণাঙ্ক i;
	ৱর্ণ *env_splashsource;

	যদি (!locations || size == 0)
		ঘূৰাই রিক্ত;

	env_splashsource = env_get("splashsource");
	যদি (env_splashsource == রিক্ত)
		ঘূৰাই &locations[0];

	বাবে (i = 0; i < size; i++) {
		যদি (!strcmp(locations[i].name, env_splashsource))
			ঘূৰাই &locations[i];
	}

	printf("splashsource env variable set to unsupported value\n");
	ঘূৰাই রিক্ত;
}

#ifdef CONFIG_FIT
স্থির পূর্ণাঙ্ক splash_load_fit(সংরচনা splash_location *location, ulong bmp_load_addr)
{
	পূর্ণাঙ্ক res;
	পূর্ণাঙ্ক গ্ৰন্থি_offset;
	ধ্রুৱ ৱর্ণ *splash_file;
	ধ্রুৱ শূন্য *internal_splash_data;
	size_t internal_splash_size;
	পূর্ণাঙ্ক external_splash_addr;
	u32 external_splash_size;
	bool is_splash_external = false;
	সংরচনা legacy_img_hdr *img_header;
	ধ্রুৱ u32 *fit_header;
	u32 fit_size;
	ধ্রুৱ size_t header_size = পরিমাণ(সংরচনা legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_load_addr, header_size);
	যদি (res < 0)
		ঘূৰাই res;

	img_header = (সংরচনা legacy_img_hdr *)(uintptr_t)bmp_load_addr;
	যদি (image_get_magic(img_header) != FDT_MAGIC) {
		printf("Could not find FDT magic\n");
		ঘূৰাই -EINVAL;
	}

	fit_size = fdt_totalsize(img_header);

	/* Read in entire FIT */
	fit_header = (ধ্রুৱ u32 *)(bmp_load_addr + header_size);
	res = splash_storage_read_raw(location, (uintptr_t)fit_header, fit_size);
	যদি (res < 0)
		ঘূৰাই res;

	res = fit_check_format(fit_header, IMAGE_SIZE_INVAL);
	যদি (res) {
		debug("Could not find valid FIT image\n");
		ঘূৰাই res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	যদি (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	গ্ৰন্থি_offset = fit_image_get_গ্ৰন্থি(fit_header, splash_file);
	যদি (গ্ৰন্থি_offset < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		ঘূৰাই -ENOENT;
	}

	/* Extract the splash data from FIT */
	যদি (!fit_image_get_data(fit_header, গ্ৰন্থি_offset, &internal_splash_data,
				&internal_splash_size)) {
		memmove((শূন্য *)(uintptr_t)bmp_load_addr, internal_splash_data,
			internal_splash_size);
	} অন্যথা {
		printf("Failed to get splash image from FIT\n");
		ঘূৰাই -ENODATA;
	}

	যদি (is_splash_external) {
		res = fit_image_get_data_size(fit_header, গ্ৰন্থি_offset, &external_splash_size);
		যদি (res < 0) {
			printf("Failed to get size of splash image (err=%d)\n", res);
			ঘূৰাই res;
		}

		/* Read in the splash data */
		location->offset = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_load_addr, external_splash_size);
		যদি (res < 0)
			ঘূৰাই res;
	}

	ঘূৰাই 0;
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
পূর্ণাঙ্ক splash_source_load(সংরচনা splash_location *locations, uint size)
{
	সংরচনা splash_location *splash_location;
	ৱর্ণ *env_splashimage_মান;
	ৱর্ণ *devpart;
	ulong bmp_load_addr;

	env_splashimage_মান = env_get("splashimage");
	যদি (env_splashimage_মান == রিক্ত)
		ঘূৰাই -ENOENT;

	bmp_load_addr = hextoul(env_splashimage_মান, 0);
	যদি (bmp_load_addr == 0) {
		printf("Error: bad splashimage address specified\n");
		ঘূৰাই -EFAULT;
	}

	splash_location = select_splash_location(locations, size);
	যদি (!splash_location)
		ঘূৰাই -EINVAL;

	devpart = env_get("splashdevpart");
	যদি (devpart)
		splash_location->devpart = devpart;

	যদি (splash_location->flags == SPLASH_STORAGE_RAW)
		ঘূৰাই splash_load_raw(splash_location, bmp_load_addr);
	অন্যথা যদি (splash_location->flags == SPLASH_STORAGE_FS)
		ঘূৰাই splash_load_fs(splash_location, bmp_load_addr);
#ifdef CONFIG_FIT
	অন্যথা যদি (splash_location->flags == SPLASH_STORAGE_FIT)
		ঘূৰাই splash_load_fit(splash_location, bmp_load_addr);
#endif
	ঘূৰাই -EINVAL;
}
