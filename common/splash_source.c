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
स्थिर पूर्णांक splash_sf_read_raw(ulong bmp_load_addr, पूर्णांक offset, माप_प्रकार read_size)
{
	जर (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_SPEED,
				     CONFIG_SF_DEFAULT_MODE);
		जर (!sf)
			परत -ENODEV;
	}

	परत spi_flash_read(sf, offset, read_size, (शून्य *)(uintptr_t)bmp_load_addr);
}
#अन्यथा
स्थिर पूर्णांक splash_sf_read_raw(ulong bmp_load_addr, पूर्णांक offset, माप_प्रकार read_size)
{
	debug("%s: sf support not available\n", __func__);
	परत -ENOSYS;
}
#endif

#ifdef CONFIG_CMD_NAND
स्थिर पूर्णांक splash_nand_read_raw(ulong bmp_load_addr, पूर्णांक offset, माप_प्रकार read_size)
{
	संरचना mtd_info *mtd = get_nand_dev_by_index(nand_curr_device);
	परत nand_read_skip_bad(mtd, offset,
				  &read_size, रिक्त,
				  mtd->size,
				  (u_char *)bmp_load_addr);
}
#अन्यथा
स्थिर पूर्णांक splash_nand_read_raw(ulong bmp_load_addr, पूर्णांक offset, माप_प्रकार read_size)
{
	debug("%s: nand support not available\n", __func__);
	परत -ENOSYS;
}
#endif

स्थिर पूर्णांक splash_mmc_read_raw(ulong bmp_load_addr, संरचना splash_location *location,
			       माप_प्रकार read_size)
{
	संरचना disk_partition partition;
	संरचना blk_desc *desc;
	lbaint_t blkcnt;
	पूर्णांक ret, n;

	जर (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		परत -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_अंक("mmc", location->devpart, &desc,
						   &partition, 1);
	जर (ret < 0)
		परत ret;

	blkcnt = DIV_ROUND_UP(read_size, partition.blksz);
	n = blk_dread(desc, partition.start, blkcnt, (शून्य *)(uintptr_t)bmp_load_addr);

	परत (n == blkcnt) ? 0 : -EIO;
}

स्थिर पूर्णांक splash_storage_read_raw(संरचना splash_location *location,
			       ulong bmp_load_addr, माप_प्रकार read_size)
{
	u32 offset;

	जर (!location)
		परत -EINVAL;

	offset = location->offset;
	निवड (location->storage) {
	स्थिती SPLASH_STORAGE_MMC:
		परत splash_mmc_read_raw(bmp_load_addr, location, read_size);
	स्थिती SPLASH_STORAGE_NAND:
		परत splash_nand_read_raw(bmp_load_addr, offset, read_size);
	स्थिती SPLASH_STORAGE_SF:
		परत splash_sf_read_raw(bmp_load_addr, offset, read_size);
	पूर्वनिर्धारित:
		म_लिखो("Unknown splash location\n");
	}

	परत -EINVAL;
}

स्थिर पूर्णांक splash_load_raw(संरचना splash_location *location, ulong bmp_load_addr)
{
	संरचना bmp_header *bmp_hdr;
	पूर्णांक res;
	माप_प्रकार bmp_size, bmp_header_size = आकार(संरचना bmp_header);

	जर (bmp_load_addr + bmp_header_size >= gd->start_addr_sp)
		जा splash_address_too_वरचा;

	res = splash_storage_read_raw(location, bmp_load_addr, bmp_header_size);
	जर (res < 0)
		परत res;

	bmp_hdr = (संरचना bmp_header *)(uintptr_t)bmp_load_addr;
	bmp_size = le32_to_cpu(bmp_hdr->file_size);

	जर (bmp_load_addr + bmp_size >= gd->start_addr_sp)
		जा splash_address_too_वरचा;

	परत splash_storage_read_raw(location, bmp_load_addr, bmp_size);

splash_address_too_वरचा:
	म_लिखो("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	परत -EFAULT;
}

स्थिर पूर्णांक splash_select_fs_dev(संरचना splash_location *location)
{
	पूर्णांक res;

	निवड (location->storage) {
	स्थिती SPLASH_STORAGE_MMC:
		res = fs_set_blk_dev("mmc", location->devpart, FS_TYPE_ANY);
		थांबा;
	स्थिती SPLASH_STORAGE_USB:
		res = fs_set_blk_dev("usb", location->devpart, FS_TYPE_ANY);
		थांबा;
	स्थिती SPLASH_STORAGE_SATA:
		res = fs_set_blk_dev("sata", location->devpart, FS_TYPE_ANY);
		थांबा;
	स्थिती SPLASH_STORAGE_NAND:
		जर (location->ubivol != रिक्त)
			res = fs_set_blk_dev("ubi", रिक्त, FS_TYPE_UBIFS);
		अन्यथा
			res = -ENODEV;
		थांबा;
	पूर्वनिर्धारित:
		म_लिखो("Error: %s: unsupported location storage.\n", __func__);
		परत -ENODEV;
	}

	जर (res)
		म_लिखो("Error: %s: could not access storage.\n", __func__);

	परत res;
}

#ifdef CONFIG_USB_STORAGE
स्थिर पूर्णांक splash_init_usb(शून्य)
{
	पूर्णांक err;

	err = usb_init();
	जर (err)
		परत err;

#ifndef CONFIG_DM_USB
	err = usb_stor_scan(1) < 0 ? -ENODEV : 0;
#endif

	परत err;
}
#अन्यथा
स्थिर ओळीत पूर्णांक splash_init_usb(शून्य)
{
	म_लिखो("Cannot load splash image: no USB support\n");
	परत -ENOSYS;
}
#endif

#ifdef CONFIG_SATA
स्थिर पूर्णांक splash_init_sata(शून्य)
{
	परत sata_probe(0);
}
#अन्यथा
स्थिर ओळीत पूर्णांक splash_init_sata(शून्य)
{
	म_लिखो("Cannot load splash image: no SATA support\n");
	परत -ENOSYS;
}
#endif

स्थिर पूर्णांक splash_init_virtio(शून्य)
{
	जर (!IS_ENABLED(CONFIG_VIRTIO)) {
		म_लिखो("Cannot load splash image: no virtio support\n");
		परत -ENOSYS;
	} अन्यथा {
		परत virtio_init();
	}
}

#जर defined(CONFIG_CMD_UBIFS) && !defined(CONFIG_XPL_BUILD)
स्थिर पूर्णांक splash_mount_ubifs(संरचना splash_location *location)
{
	पूर्णांक res;
	अक्षर cmd[32];

	प्र_लिखो(cmd, "ubi part %s", location->mtdpart);
	res = run_command(cmd, 0);
	जर (res)
		परत res;

	प्र_लिखो(cmd, "ubifsmount %s", location->ubivol);
	res = run_command(cmd, 0);

	परत res;
}

स्थिर ओळीत पूर्णांक splash_umount_ubifs(शून्य)
{
	परत run_command("ubifsumount", 0);
}
#अन्यथा
स्थिर ओळीत पूर्णांक splash_mount_ubifs(संरचना splash_location *location)
{
	म_लिखो("Cannot load splash image: no UBIFS support\n");
	परत -ENOSYS;
}

स्थिर ओळीत पूर्णांक splash_umount_ubifs(शून्य)
{
	म_लिखो("Cannot unmount UBIFS: no UBIFS support\n");
	परत -ENOSYS;
}
#endif

#define SPLASH_SOURCE_DEFAULT_FILE_NAME		"splash.bmp"

स्थिर पूर्णांक splash_load_fs(संरचना splash_location *location, ulong bmp_load_addr)
{
	पूर्णांक res = 0;
	loff_t bmp_size;
	loff_t actread;
	अक्षर *splash_file;

	splash_file = env_get("splashfile");
	जर (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	जर (location->storage == SPLASH_STORAGE_USB)
		res = splash_init_usb();

	जर (location->storage == SPLASH_STORAGE_SATA)
		res = splash_init_sata();

	जर (location->storage == SPLASH_STORAGE_VIRTIO)
		res = splash_init_virtio();

	जर (location->ubivol != रिक्त)
		res = splash_mount_ubifs(location);

	जर (res)
		परत res;

	res = splash_select_fs_dev(location);
	जर (res)
		जा out;

	res = fs_size(splash_file, &bmp_size);
	जर (res) {
		म_लिखो("Error: %s: cannot determine file size (%d)\n",
		       __func__, res);
		जा out;
	}

	जर (bmp_load_addr + bmp_size >= gd->start_addr_sp) {
		म_लिखो("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		जा out;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_load_addr, 0, 0, &actread);

out:
	जर (location->ubivol != रिक्त)
		splash_umount_ubifs();

	परत res;
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
	पूर्णांक i;
	अक्षर *env_splashsource;

	जर (!locations || size == 0)
		परत रिक्त;

	env_splashsource = env_get("splashsource");
	जर (env_splashsource == रिक्त)
		परत &locations[0];

	साठी (i = 0; i < size; i++) {
		जर (!म_भेद(locations[i].name, env_splashsource))
			परत &locations[i];
	}

	म_लिखो("splashsource env variable set to unsupported value\n");
	परत रिक्त;
}

#ifdef CONFIG_FIT
स्थिर पूर्णांक splash_load_fit(संरचना splash_location *location, ulong bmp_load_addr)
{
	पूर्णांक res;
	पूर्णांक गाठ_offset;
	स्थिरांक अक्षर *splash_file;
	स्थिरांक शून्य *internal_splash_data;
	माप_प्रकार internal_splash_size;
	पूर्णांक external_splash_addr;
	u32 external_splash_size;
	bool is_splash_external = false;
	संरचना legacy_img_hdr *img_header;
	स्थिरांक u32 *fit_header;
	u32 fit_size;
	स्थिरांक माप_प्रकार header_size = आकार(संरचना legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_load_addr, header_size);
	जर (res < 0)
		परत res;

	img_header = (संरचना legacy_img_hdr *)(uintptr_t)bmp_load_addr;
	जर (image_get_magic(img_header) != FDT_MAGIC) {
		म_लिखो("Could not find FDT magic\n");
		परत -EINVAL;
	}

	fit_size = fdt_totalsize(img_header);

	/* Read in entire FIT */
	fit_header = (स्थिरांक u32 *)(bmp_load_addr + header_size);
	res = splash_storage_read_raw(location, (uintptr_t)fit_header, fit_size);
	जर (res < 0)
		परत res;

	res = fit_check_format(fit_header, IMAGE_SIZE_INVAL);
	जर (res) {
		debug("Could not find valid FIT image\n");
		परत res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	जर (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	गाठ_offset = fit_image_get_गाठ(fit_header, splash_file);
	जर (गाठ_offset < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		परत -ENOENT;
	}

	/* Extract the splash data from FIT */
	जर (!fit_image_get_data(fit_header, गाठ_offset, &internal_splash_data,
				&internal_splash_size)) {
		स_हटाओ((शून्य *)(uintptr_t)bmp_load_addr, internal_splash_data,
			internal_splash_size);
	} अन्यथा {
		म_लिखो("Failed to get splash image from FIT\n");
		परत -ENODATA;
	}

	जर (is_splash_external) {
		res = fit_image_get_data_size(fit_header, गाठ_offset, &external_splash_size);
		जर (res < 0) {
			म_लिखो("Failed to get size of splash image (err=%d)\n", res);
			परत res;
		}

		/* Read in the splash data */
		location->offset = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_load_addr, external_splash_size);
		जर (res < 0)
			परत res;
	}

	परत 0;
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
पूर्णांक splash_source_load(संरचना splash_location *locations, uint size)
{
	संरचना splash_location *splash_location;
	अक्षर *env_splashimage_मूल्य;
	अक्षर *devpart;
	ulong bmp_load_addr;

	env_splashimage_मूल्य = env_get("splashimage");
	जर (env_splashimage_मूल्य == रिक्त)
		परत -ENOENT;

	bmp_load_addr = hextoul(env_splashimage_मूल्य, 0);
	जर (bmp_load_addr == 0) {
		म_लिखो("Error: bad splashimage address specified\n");
		परत -EFAULT;
	}

	splash_location = select_splash_location(locations, size);
	जर (!splash_location)
		परत -EINVAL;

	devpart = env_get("splashdevpart");
	जर (devpart)
		splash_location->devpart = devpart;

	जर (splash_location->flags == SPLASH_STORAGE_RAW)
		परत splash_load_raw(splash_location, bmp_load_addr);
	अन्यथा जर (splash_location->flags == SPLASH_STORAGE_FS)
		परत splash_load_fs(splash_location, bmp_load_addr);
#ifdef CONFIG_FIT
	अन्यथा जर (splash_location->flags == SPLASH_STORAGE_FIT)
		परत splash_load_fit(splash_location, bmp_load_addr);
#endif
	परत -EINVAL;
}
