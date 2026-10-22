// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 */

#अन्तर्भाव <bmp_layout.h>
#अन्तर्भाव <command.h>
#अन्तर्भाव <env.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <fs.h>
#अन्तर्भाव <fdt_support.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <nand.h>
#अन्तर्भाव <sata.h>
#अन्तर्भाव <spi.h>
#अन्तर्भाव <spi_flash.h>
#अन्तर्भाव <splash.h>
#अन्तर्भाव <usb.h>
#अन्तर्भाव <virtio.h>
#अन्तर्भाव <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SPI_FLASH
स्थिर संरचना spi_flash *sf;
स्थिर पूर्णाङ्क splash_sf_read_raw(ulong bmp_load_addr, पूर्णाङ्क offset, size_t read_size)
{
	यदि (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_SPEED,
				     CONFIG_SF_DEFAULT_MODE);
		यदि (!sf)
			प्रत्यागम -ENODEV;
	}

	प्रत्यागम spi_flash_read(sf, offset, read_size, (शून्य *)(uintptr_t)bmp_load_addr);
}
#अन्यथा
स्थिर पूर्णाङ्क splash_sf_read_raw(ulong bmp_load_addr, पूर्णाङ्क offset, size_t read_size)
{
	debug("%s: sf support not available\n", __func__);
	प्रत्यागम -ENOSYS;
}
#endif

#ifdef CONFIG_CMD_NAND
स्थिर पूर्णाङ्क splash_nand_read_raw(ulong bmp_load_addr, पूर्णाङ्क offset, size_t read_size)
{
	संरचना mtd_info *mtd = get_nand_dev_by_index(nand_curr_device);
	प्रत्यागम nand_read_skip_bad(mtd, offset,
				  &read_size, रिक्त,
				  mtd->size,
				  (u_char *)bmp_load_addr);
}
#अन्यथा
स्थिर पूर्णाङ्क splash_nand_read_raw(ulong bmp_load_addr, पूर्णाङ्क offset, size_t read_size)
{
	debug("%s: nand support not available\n", __func__);
	प्रत्यागम -ENOSYS;
}
#endif

स्थिर पूर्णाङ्क splash_mmc_read_raw(ulong bmp_load_addr, संरचना splash_location *location,
			       size_t read_size)
{
	संरचना disk_partition partition;
	संरचना blk_desc *desc;
	lbaint_t blkcnt;
	पूर्णाङ्क ret, n;

	यदि (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		प्रत्यागम -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_अंक("mmc", location->devpart, &desc,
						   &partition, 1);
	यदि (ret < 0)
		प्रत्यागम ret;

	blkcnt = DIV_ROUND_UP(read_size, partition.blksz);
	n = blk_dread(desc, partition.start, blkcnt, (शून्य *)(uintptr_t)bmp_load_addr);

	प्रत्यागम (n == blkcnt) ? 0 : -EIO;
}

स्थिर पूर्णाङ्क splash_storage_read_raw(संरचना splash_location *location,
			       ulong bmp_load_addr, size_t read_size)
{
	u32 offset;

	यदि (!location)
		प्रत्यागम -EINVAL;

	offset = location->offset;
	विकल्प (location->storage) {
	पक्ष SPLASH_STORAGE_MMC:
		प्रत्यागम splash_mmc_read_raw(bmp_load_addr, location, read_size);
	पक्ष SPLASH_STORAGE_NAND:
		प्रत्यागम splash_nand_read_raw(bmp_load_addr, offset, read_size);
	पक्ष SPLASH_STORAGE_SF:
		प्रत्यागम splash_sf_read_raw(bmp_load_addr, offset, read_size);
	स्वभाव:
		printf("Unknown splash location\n");
	}

	प्रत्यागम -EINVAL;
}

स्थिर पूर्णाङ्क splash_load_raw(संरचना splash_location *location, ulong bmp_load_addr)
{
	संरचना bmp_header *bmp_hdr;
	पूर्णाङ्क res;
	size_t bmp_size, bmp_header_size = परिमाण(संरचना bmp_header);

	यदि (bmp_load_addr + bmp_header_size >= gd->start_addr_sp)
		गच्छ splash_address_too_ऊपरी;

	res = splash_storage_read_raw(location, bmp_load_addr, bmp_header_size);
	यदि (res < 0)
		प्रत्यागम res;

	bmp_hdr = (संरचना bmp_header *)(uintptr_t)bmp_load_addr;
	bmp_size = le32_to_cpu(bmp_hdr->file_size);

	यदि (bmp_load_addr + bmp_size >= gd->start_addr_sp)
		गच्छ splash_address_too_ऊपरी;

	प्रत्यागम splash_storage_read_raw(location, bmp_load_addr, bmp_size);

splash_address_too_ऊपरी:
	printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	प्रत्यागम -EFAULT;
}

स्थिर पूर्णाङ्क splash_select_fs_dev(संरचना splash_location *location)
{
	पूर्णाङ्क res;

	विकल्प (location->storage) {
	पक्ष SPLASH_STORAGE_MMC:
		res = fs_set_blk_dev("mmc", location->devpart, FS_TYPE_ANY);
		भङ्ग;
	पक्ष SPLASH_STORAGE_USB:
		res = fs_set_blk_dev("usb", location->devpart, FS_TYPE_ANY);
		भङ्ग;
	पक्ष SPLASH_STORAGE_SATA:
		res = fs_set_blk_dev("sata", location->devpart, FS_TYPE_ANY);
		भङ्ग;
	पक्ष SPLASH_STORAGE_NAND:
		यदि (location->ubivol != रिक्त)
			res = fs_set_blk_dev("ubi", रिक्त, FS_TYPE_UBIFS);
		अन्यथा
			res = -ENODEV;
		भङ्ग;
	स्वभाव:
		printf("Error: %s: unsupported location storage.\n", __func__);
		प्रत्यागम -ENODEV;
	}

	यदि (res)
		printf("Error: %s: could not access storage.\n", __func__);

	प्रत्यागम res;
}

#ifdef CONFIG_USB_STORAGE
स्थिर पूर्णाङ्क splash_init_usb(शून्य)
{
	पूर्णाङ्क err;

	err = usb_init();
	यदि (err)
		प्रत्यागम err;

#ifndef CONFIG_DM_USB
	err = usb_stor_scan(1) < 0 ? -ENODEV : 0;
#endif

	प्रत्यागम err;
}
#अन्यथा
स्थिर अन्तःस्थ पूर्णाङ्क splash_init_usb(शून्य)
{
	printf("Cannot load splash image: no USB support\n");
	प्रत्यागम -ENOSYS;
}
#endif

#ifdef CONFIG_SATA
स्थिर पूर्णाङ्क splash_init_sata(शून्य)
{
	प्रत्यागम sata_probe(0);
}
#अन्यथा
स्थिर अन्तःस्थ पूर्णाङ्क splash_init_sata(शून्य)
{
	printf("Cannot load splash image: no SATA support\n");
	प्रत्यागम -ENOSYS;
}
#endif

स्थिर पूर्णाङ्क splash_init_virtio(शून्य)
{
	यदि (!IS_ENABLED(CONFIG_VIRTIO)) {
		printf("Cannot load splash image: no virtio support\n");
		प्रत्यागम -ENOSYS;
	} अन्यथा {
		प्रत्यागम virtio_init();
	}
}

#यदि defined(CONFIG_CMD_UBIFS) && !defined(CONFIG_XPL_BUILD)
स्थिर पूर्णाङ्क splash_mount_ubifs(संरचना splash_location *location)
{
	पूर्णाङ्क res;
	वर्ण cmd[32];

	sprintf(cmd, "ubi part %s", location->mtdpart);
	res = run_command(cmd, 0);
	यदि (res)
		प्रत्यागम res;

	sprintf(cmd, "ubifsmount %s", location->ubivol);
	res = run_command(cmd, 0);

	प्रत्यागम res;
}

स्थिर अन्तःस्थ पूर्णाङ्क splash_umount_ubifs(शून्य)
{
	प्रत्यागम run_command("ubifsumount", 0);
}
#अन्यथा
स्थिर अन्तःस्थ पूर्णाङ्क splash_mount_ubifs(संरचना splash_location *location)
{
	printf("Cannot load splash image: no UBIFS support\n");
	प्रत्यागम -ENOSYS;
}

स्थिर अन्तःस्थ पूर्णाङ्क splash_umount_ubifs(शून्य)
{
	printf("Cannot unmount UBIFS: no UBIFS support\n");
	प्रत्यागम -ENOSYS;
}
#endif

#define SPLASH_SOURCE_DEFAULT_FILE_NAME		"splash.bmp"

स्थिर पूर्णाङ्क splash_load_fs(संरचना splash_location *location, ulong bmp_load_addr)
{
	पूर्णाङ्क res = 0;
	loff_t bmp_size;
	loff_t actread;
	वर्ण *splash_file;

	splash_file = env_get("splashfile");
	यदि (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	यदि (location->storage == SPLASH_STORAGE_USB)
		res = splash_init_usb();

	यदि (location->storage == SPLASH_STORAGE_SATA)
		res = splash_init_sata();

	यदि (location->storage == SPLASH_STORAGE_VIRTIO)
		res = splash_init_virtio();

	यदि (location->ubivol != रिक्त)
		res = splash_mount_ubifs(location);

	यदि (res)
		प्रत्यागम res;

	res = splash_select_fs_dev(location);
	यदि (res)
		गच्छ out;

	res = fs_size(splash_file, &bmp_size);
	यदि (res) {
		printf("Error: %s: cannot determine file size (%d)\n",
		       __func__, res);
		गच्छ out;
	}

	यदि (bmp_load_addr + bmp_size >= gd->start_addr_sp) {
		printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		गच्छ out;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_load_addr, 0, 0, &actread);

out:
	यदि (location->ubivol != रिक्त)
		splash_umount_ubifs();

	प्रत्यागम res;
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
स्थिर संरचना splash_location *select_splash_location(
			    संरचना splash_location *locations, uint size)
{
	पूर्णाङ्क i;
	वर्ण *env_splashsource;

	यदि (!locations || size == 0)
		प्रत्यागम रिक्त;

	env_splashsource = env_get("splashsource");
	यदि (env_splashsource == रिक्त)
		प्रत्यागम &locations[0];

	प्रति (i = 0; i < size; i++) {
		यदि (!strcmp(locations[i].name, env_splashsource))
			प्रत्यागम &locations[i];
	}

	printf("splashsource env variable set to unsupported value\n");
	प्रत्यागम रिक्त;
}

#ifdef CONFIG_FIT
स्थिर पूर्णाङ्क splash_load_fit(संरचना splash_location *location, ulong bmp_load_addr)
{
	पूर्णाङ्क res;
	पूर्णाङ्क गाँठ_offset;
	ध्रुव वर्ण *splash_file;
	ध्रुव शून्य *internal_splash_data;
	size_t internal_splash_size;
	पूर्णाङ्क external_splash_addr;
	u32 external_splash_size;
	bool is_splash_external = false;
	संरचना legacy_img_hdr *img_header;
	ध्रुव u32 *fit_header;
	u32 fit_size;
	ध्रुव size_t header_size = परिमाण(संरचना legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_load_addr, header_size);
	यदि (res < 0)
		प्रत्यागम res;

	img_header = (संरचना legacy_img_hdr *)(uintptr_t)bmp_load_addr;
	यदि (image_get_magic(img_header) != FDT_MAGIC) {
		printf("Could not find FDT magic\n");
		प्रत्यागम -EINVAL;
	}

	fit_size = fdt_totalsize(img_header);

	/* Read in entire FIT */
	fit_header = (ध्रुव u32 *)(bmp_load_addr + header_size);
	res = splash_storage_read_raw(location, (uintptr_t)fit_header, fit_size);
	यदि (res < 0)
		प्रत्यागम res;

	res = fit_check_format(fit_header, IMAGE_SIZE_INVAL);
	यदि (res) {
		debug("Could not find valid FIT image\n");
		प्रत्यागम res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	यदि (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	गाँठ_offset = fit_image_get_गाँठ(fit_header, splash_file);
	यदि (गाँठ_offset < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		प्रत्यागम -ENOENT;
	}

	/* Extract the splash data from FIT */
	यदि (!fit_image_get_data(fit_header, गाँठ_offset, &internal_splash_data,
				&internal_splash_size)) {
		memmove((शून्य *)(uintptr_t)bmp_load_addr, internal_splash_data,
			internal_splash_size);
	} अन्यथा {
		printf("Failed to get splash image from FIT\n");
		प्रत्यागम -ENODATA;
	}

	यदि (is_splash_external) {
		res = fit_image_get_data_size(fit_header, गाँठ_offset, &external_splash_size);
		यदि (res < 0) {
			printf("Failed to get size of splash image (err=%d)\n", res);
			प्रत्यागम res;
		}

		/* Read in the splash data */
		location->offset = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_load_addr, external_splash_size);
		यदि (res < 0)
			प्रत्यागम res;
	}

	प्रत्यागम 0;
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
पूर्णाङ्क splash_source_load(संरचना splash_location *locations, uint size)
{
	संरचना splash_location *splash_location;
	वर्ण *env_splashimage_मूल्य;
	वर्ण *devpart;
	ulong bmp_load_addr;

	env_splashimage_मूल्य = env_get("splashimage");
	यदि (env_splashimage_मूल्य == रिक्त)
		प्रत्यागम -ENOENT;

	bmp_load_addr = hextoul(env_splashimage_मूल्य, 0);
	यदि (bmp_load_addr == 0) {
		printf("Error: bad splashimage address specified\n");
		प्रत्यागम -EFAULT;
	}

	splash_location = select_splash_location(locations, size);
	यदि (!splash_location)
		प्रत्यागम -EINVAL;

	devpart = env_get("splashdevpart");
	यदि (devpart)
		splash_location->devpart = devpart;

	यदि (splash_location->flags == SPLASH_STORAGE_RAW)
		प्रत्यागम splash_load_raw(splash_location, bmp_load_addr);
	अन्यथा यदि (splash_location->flags == SPLASH_STORAGE_FS)
		प्रत्यागम splash_load_fs(splash_location, bmp_load_addr);
#ifdef CONFIG_FIT
	अन्यथा यदि (splash_location->flags == SPLASH_STORAGE_FIT)
		प्रत्यागम splash_load_fit(splash_location, bmp_load_addr);
#endif
	प्रत्यागम -EINVAL;
}
