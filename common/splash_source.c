// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 */

#ಅನ್ತರ್ಭಾವ <bmp_layout.h>
#ಅನ್ತರ್ಭಾವ <command.h>
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fs.h>
#ಅನ್ತರ್ಭಾವ <fdt_support.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <nand.h>
#ಅನ್ತರ್ಭಾವ <sata.h>
#ಅನ್ತರ್ಭಾವ <spi.h>
#ಅನ್ತರ್ಭಾವ <spi_flash.h>
#ಅನ್ತರ್ಭಾವ <splash.h>
#ಅನ್ತರ್ಭಾವ <usb.h>
#ಅನ್ತರ್ಭಾವ <virtio.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SPI_FLASH
ಸ್ಥಿರ ಸಂರಚನಾ spi_flash *sf;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ splash_sf_read_raw(ulong bmp_load_addr, ಪೂರ್ಣಾಙ್ಕ offset, size_t read_size)
{
	ಯದಿ (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_SPEED,
				     CONFIG_SF_DEFAULT_MODE);
		ಯದಿ (!sf)
			ಪ್ರತ್ಯಾಗಮ -ENODEV;
	}

	ಪ್ರತ್ಯಾಗಮ spi_flash_read(sf, offset, read_size, (ಶೂನ್ಯ *)(uintptr_t)bmp_load_addr);
}
#ಅನ್ಯಥಾ
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ splash_sf_read_raw(ulong bmp_load_addr, ಪೂರ್ಣಾಙ್ಕ offset, size_t read_size)
{
	debug("%s: sf support not available\n", __func__);
	ಪ್ರತ್ಯಾಗಮ -ENOSYS;
}
#endif

#ifdef CONFIG_CMD_NAND
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ splash_nand_read_raw(ulong bmp_load_addr, ಪೂರ್ಣಾಙ್ಕ offset, size_t read_size)
{
	ಸಂರಚನಾ mtd_info *mtd = get_nand_dev_by_index(nand_curr_device);
	ಪ್ರತ್ಯಾಗಮ nand_read_skip_bad(mtd, offset,
				  &read_size, ರಿಕ್ತ,
				  mtd->size,
				  (u_char *)bmp_load_addr);
}
#ಅನ್ಯಥಾ
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ splash_nand_read_raw(ulong bmp_load_addr, ಪೂರ್ಣಾಙ್ಕ offset, size_t read_size)
{
	debug("%s: nand support not available\n", __func__);
	ಪ್ರತ್ಯಾಗಮ -ENOSYS;
}
#endif

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ splash_mmc_read_raw(ulong bmp_load_addr, ಸಂರಚನಾ splash_location *location,
			       size_t read_size)
{
	ಸಂರಚನಾ disk_partition partition;
	ಸಂರಚನಾ blk_desc *desc;
	lbaint_t blkcnt;
	ಪೂರ್ಣಾಙ್ಕ ret, n;

	ಯದಿ (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		ಪ್ರತ್ಯಾಗಮ -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_ಅಂಕೆ("mmc", location->devpart, &desc,
						   &partition, 1);
	ಯದಿ (ret < 0)
		ಪ್ರತ್ಯಾಗಮ ret;

	blkcnt = DIV_ROUND_UP(read_size, partition.blksz);
	n = blk_dread(desc, partition.start, blkcnt, (ಶೂನ್ಯ *)(uintptr_t)bmp_load_addr);

	ಪ್ರತ್ಯಾಗಮ (n == blkcnt) ? 0 : -EIO;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ splash_storage_read_raw(ಸಂರಚನಾ splash_location *location,
			       ulong bmp_load_addr, size_t read_size)
{
	u32 offset;

	ಯದಿ (!location)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	offset = location->offset;
	ವಿಕಲ್ಪ (location->storage) {
	ಪಕ್ಷ SPLASH_STORAGE_MMC:
		ಪ್ರತ್ಯಾಗಮ splash_mmc_read_raw(bmp_load_addr, location, read_size);
	ಪಕ್ಷ SPLASH_STORAGE_NAND:
		ಪ್ರತ್ಯಾಗಮ splash_nand_read_raw(bmp_load_addr, offset, read_size);
	ಪಕ್ಷ SPLASH_STORAGE_SF:
		ಪ್ರತ್ಯಾಗಮ splash_sf_read_raw(bmp_load_addr, offset, read_size);
	ಸ್ವಭಾವ:
		printf("Unknown splash location\n");
	}

	ಪ್ರತ್ಯಾಗಮ -EINVAL;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ splash_load_raw(ಸಂರಚನಾ splash_location *location, ulong bmp_load_addr)
{
	ಸಂರಚನಾ bmp_header *bmp_hdr;
	ಪೂರ್ಣಾಙ್ಕ res;
	size_t bmp_size, bmp_header_size = ಪರಿಮಾಣ(ಸಂರಚನಾ bmp_header);

	ಯದಿ (bmp_load_addr + bmp_header_size >= gd->start_addr_sp)
		ಗಚ್ಛ splash_address_too_ಮೇಲಿನ;

	res = splash_storage_read_raw(location, bmp_load_addr, bmp_header_size);
	ಯದಿ (res < 0)
		ಪ್ರತ್ಯಾಗಮ res;

	bmp_hdr = (ಸಂರಚನಾ bmp_header *)(uintptr_t)bmp_load_addr;
	bmp_size = le32_to_cpu(bmp_hdr->file_size);

	ಯದಿ (bmp_load_addr + bmp_size >= gd->start_addr_sp)
		ಗಚ್ಛ splash_address_too_ಮೇಲಿನ;

	ಪ್ರತ್ಯಾಗಮ splash_storage_read_raw(location, bmp_load_addr, bmp_size);

splash_address_too_ಮೇಲಿನ:
	printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	ಪ್ರತ್ಯಾಗಮ -EFAULT;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ splash_select_fs_dev(ಸಂರಚನಾ splash_location *location)
{
	ಪೂರ್ಣಾಙ್ಕ res;

	ವಿಕಲ್ಪ (location->storage) {
	ಪಕ್ಷ SPLASH_STORAGE_MMC:
		res = fs_set_blk_dev("mmc", location->devpart, FS_TYPE_ANY);
		ಭಙ್ಗ;
	ಪಕ್ಷ SPLASH_STORAGE_USB:
		res = fs_set_blk_dev("usb", location->devpart, FS_TYPE_ANY);
		ಭಙ್ಗ;
	ಪಕ್ಷ SPLASH_STORAGE_SATA:
		res = fs_set_blk_dev("sata", location->devpart, FS_TYPE_ANY);
		ಭಙ್ಗ;
	ಪಕ್ಷ SPLASH_STORAGE_NAND:
		ಯದಿ (location->ubivol != ರಿಕ್ತ)
			res = fs_set_blk_dev("ubi", ರಿಕ್ತ, FS_TYPE_UBIFS);
		ಅನ್ಯಥಾ
			res = -ENODEV;
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		printf("Error: %s: unsupported location storage.\n", __func__);
		ಪ್ರತ್ಯಾಗಮ -ENODEV;
	}

	ಯದಿ (res)
		printf("Error: %s: could not access storage.\n", __func__);

	ಪ್ರತ್ಯಾಗಮ res;
}

#ifdef CONFIG_USB_STORAGE
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ splash_init_usb(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ err;

	err = usb_init();
	ಯದಿ (err)
		ಪ್ರತ್ಯಾಗಮ err;

#ifndef CONFIG_DM_USB
	err = usb_stor_scan(1) < 0 ? -ENODEV : 0;
#endif

	ಪ್ರತ್ಯಾಗಮ err;
}
#ಅನ್ಯಥಾ
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ splash_init_usb(ಶೂನ್ಯ)
{
	printf("Cannot load splash image: no USB support\n");
	ಪ್ರತ್ಯಾಗಮ -ENOSYS;
}
#endif

#ifdef CONFIG_SATA
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ splash_init_sata(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ sata_probe(0);
}
#ಅನ್ಯಥಾ
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ splash_init_sata(ಶೂನ್ಯ)
{
	printf("Cannot load splash image: no SATA support\n");
	ಪ್ರತ್ಯಾಗಮ -ENOSYS;
}
#endif

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ splash_init_virtio(ಶೂನ್ಯ)
{
	ಯದಿ (!IS_ENABLED(CONFIG_VIRTIO)) {
		printf("Cannot load splash image: no virtio support\n");
		ಪ್ರತ್ಯಾಗಮ -ENOSYS;
	} ಅನ್ಯಥಾ {
		ಪ್ರತ್ಯಾಗಮ virtio_init();
	}
}

#ಯದಿ defined(CONFIG_CMD_UBIFS) && !defined(CONFIG_XPL_BUILD)
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ splash_mount_ubifs(ಸಂರಚನಾ splash_location *location)
{
	ಪೂರ್ಣಾಙ್ಕ res;
	ವರ್ಣ cmd[32];

	sprintf(cmd, "ubi part %s", location->mtdpart);
	res = run_command(cmd, 0);
	ಯದಿ (res)
		ಪ್ರತ್ಯಾಗಮ res;

	sprintf(cmd, "ubifsmount %s", location->ubivol);
	res = run_command(cmd, 0);

	ಪ್ರತ್ಯಾಗಮ res;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ splash_umount_ubifs(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ run_command("ubifsumount", 0);
}
#ಅನ್ಯಥಾ
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ splash_mount_ubifs(ಸಂರಚನಾ splash_location *location)
{
	printf("Cannot load splash image: no UBIFS support\n");
	ಪ್ರತ್ಯಾಗಮ -ENOSYS;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ splash_umount_ubifs(ಶೂನ್ಯ)
{
	printf("Cannot unmount UBIFS: no UBIFS support\n");
	ಪ್ರತ್ಯಾಗಮ -ENOSYS;
}
#endif

#define SPLASH_SOURCE_DEFAULT_FILE_NAME		"splash.bmp"

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ splash_load_fs(ಸಂರಚನಾ splash_location *location, ulong bmp_load_addr)
{
	ಪೂರ್ಣಾಙ್ಕ res = 0;
	loff_t bmp_size;
	loff_t actread;
	ವರ್ಣ *splash_file;

	splash_file = env_get("splashfile");
	ಯದಿ (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	ಯದಿ (location->storage == SPLASH_STORAGE_USB)
		res = splash_init_usb();

	ಯದಿ (location->storage == SPLASH_STORAGE_SATA)
		res = splash_init_sata();

	ಯದಿ (location->storage == SPLASH_STORAGE_VIRTIO)
		res = splash_init_virtio();

	ಯದಿ (location->ubivol != ರಿಕ್ತ)
		res = splash_mount_ubifs(location);

	ಯದಿ (res)
		ಪ್ರತ್ಯಾಗಮ res;

	res = splash_select_fs_dev(location);
	ಯದಿ (res)
		ಗಚ್ಛ out;

	res = fs_size(splash_file, &bmp_size);
	ಯದಿ (res) {
		printf("Error: %s: cannot determine file size (%d)\n",
		       __func__, res);
		ಗಚ್ಛ out;
	}

	ಯದಿ (bmp_load_addr + bmp_size >= gd->start_addr_sp) {
		printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		ಗಚ್ಛ out;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_load_addr, 0, 0, &actread);

out:
	ಯದಿ (location->ubivol != ರಿಕ್ತ)
		splash_umount_ubifs();

	ಪ್ರತ್ಯಾಗಮ res;
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
ಸ್ಥಿರ ಸಂರಚನಾ splash_location *select_splash_location(
			    ಸಂರಚನಾ splash_location *locations, uint size)
{
	ಪೂರ್ಣಾಙ್ಕ i;
	ವರ್ಣ *env_splashsource;

	ಯದಿ (!locations || size == 0)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	env_splashsource = env_get("splashsource");
	ಯದಿ (env_splashsource == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ &locations[0];

	ಪ್ರತಿ (i = 0; i < size; i++) {
		ಯದಿ (!strcmp(locations[i].name, env_splashsource))
			ಪ್ರತ್ಯಾಗಮ &locations[i];
	}

	printf("splashsource env variable set to unsupported value\n");
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

#ifdef CONFIG_FIT
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ splash_load_fit(ಸಂರಚನಾ splash_location *location, ulong bmp_load_addr)
{
	ಪೂರ್ಣಾಙ್ಕ res;
	ಪೂರ್ಣಾಙ್ಕ ಗಂಟು_offset;
	ಧ್ರುವ ವರ್ಣ *splash_file;
	ಧ್ರುವ ಶೂನ್ಯ *internal_splash_data;
	size_t internal_splash_size;
	ಪೂರ್ಣಾಙ್ಕ external_splash_addr;
	u32 external_splash_size;
	bool is_splash_external = false;
	ಸಂರಚನಾ legacy_img_hdr *img_header;
	ಧ್ರುವ u32 *fit_header;
	u32 fit_size;
	ಧ್ರುವ size_t header_size = ಪರಿಮಾಣ(ಸಂರಚನಾ legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_load_addr, header_size);
	ಯದಿ (res < 0)
		ಪ್ರತ್ಯಾಗಮ res;

	img_header = (ಸಂರಚನಾ legacy_img_hdr *)(uintptr_t)bmp_load_addr;
	ಯದಿ (image_get_magic(img_header) != FDT_MAGIC) {
		printf("Could not find FDT magic\n");
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	fit_size = fdt_totalsize(img_header);

	/* Read in entire FIT */
	fit_header = (ಧ್ರುವ u32 *)(bmp_load_addr + header_size);
	res = splash_storage_read_raw(location, (uintptr_t)fit_header, fit_size);
	ಯದಿ (res < 0)
		ಪ್ರತ್ಯಾಗಮ res;

	res = fit_check_format(fit_header, IMAGE_SIZE_INVAL);
	ಯದಿ (res) {
		debug("Could not find valid FIT image\n");
		ಪ್ರತ್ಯಾಗಮ res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	ಯದಿ (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	ಗಂಟು_offset = fit_image_get_ಗಂಟು(fit_header, splash_file);
	ಯದಿ (ಗಂಟು_offset < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		ಪ್ರತ್ಯಾಗಮ -ENOENT;
	}

	/* Extract the splash data from FIT */
	ಯದಿ (!fit_image_get_data(fit_header, ಗಂಟು_offset, &internal_splash_data,
				&internal_splash_size)) {
		memmove((ಶೂನ್ಯ *)(uintptr_t)bmp_load_addr, internal_splash_data,
			internal_splash_size);
	} ಅನ್ಯಥಾ {
		printf("Failed to get splash image from FIT\n");
		ಪ್ರತ್ಯಾಗಮ -ENODATA;
	}

	ಯದಿ (is_splash_external) {
		res = fit_image_get_data_size(fit_header, ಗಂಟು_offset, &external_splash_size);
		ಯದಿ (res < 0) {
			printf("Failed to get size of splash image (err=%d)\n", res);
			ಪ್ರತ್ಯಾಗಮ res;
		}

		/* Read in the splash data */
		location->offset = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_load_addr, external_splash_size);
		ಯದಿ (res < 0)
			ಪ್ರತ್ಯಾಗಮ res;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
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
ಪೂರ್ಣಾಙ್ಕ splash_source_load(ಸಂರಚನಾ splash_location *locations, uint size)
{
	ಸಂರಚನಾ splash_location *splash_location;
	ವರ್ಣ *env_splashimage_ಮೌಲ್ಯ;
	ವರ್ಣ *devpart;
	ulong bmp_load_addr;

	env_splashimage_ಮೌಲ್ಯ = env_get("splashimage");
	ಯದಿ (env_splashimage_ಮೌಲ್ಯ == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ -ENOENT;

	bmp_load_addr = hextoul(env_splashimage_ಮೌಲ್ಯ, 0);
	ಯದಿ (bmp_load_addr == 0) {
		printf("Error: bad splashimage address specified\n");
		ಪ್ರತ್ಯಾಗಮ -EFAULT;
	}

	splash_location = select_splash_location(locations, size);
	ಯದಿ (!splash_location)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	devpart = env_get("splashdevpart");
	ಯದಿ (devpart)
		splash_location->devpart = devpart;

	ಯದಿ (splash_location->flags == SPLASH_STORAGE_RAW)
		ಪ್ರತ್ಯಾಗಮ splash_load_raw(splash_location, bmp_load_addr);
	ಅನ್ಯಥಾ ಯದಿ (splash_location->flags == SPLASH_STORAGE_FS)
		ಪ್ರತ್ಯಾಗಮ splash_load_fs(splash_location, bmp_load_addr);
#ifdef CONFIG_FIT
	ಅನ್ಯಥಾ ಯದಿ (splash_location->flags == SPLASH_STORAGE_FIT)
		ಪ್ರತ್ಯಾಗಮ splash_load_fit(splash_location, bmp_load_addr);
#endif
	ಪ್ರತ್ಯಾಗಮ -EINVAL;
}
