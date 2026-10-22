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

DECLARE_GLOBAL_दत्त_PTR;

#यदि_परिभाषितम् CONFIG_SPI_FLASH
स्थिर संरचना spi_flash *sf;
स्थिर पूर्णाङ्क splash_sf_read_raw(ulong bmp_आरोपय_addr, पूर्णाङ्क विस्थापनम्, size_t read_आकार)
{
	यदि (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_जवः,
				     CONFIG_SF_DEFAULT_बहुलकम्);
		यदि (!sf)
			प्रत्यागम -ENODEV;
	}

	प्रत्यागम spi_flash_read(sf, विस्थापनम्, read_आकार, (शून्य *)(uintptr_t)bmp_आरोपय_addr);
}
#अन्यथा
स्थिर पूर्णाङ्क splash_sf_read_raw(ulong bmp_आरोपय_addr, पूर्णाङ्क विस्थापनम्, size_t read_आकार)
{
	debug("%s: sf support not available\n", __func__);
	प्रत्यागम -ENOSYS;
}
#यदि_अन्तः

#यदि_परिभाषितम् CONFIG_CMD_NAND
स्थिर पूर्णाङ्क splash_nand_read_raw(ulong bmp_आरोपय_addr, पूर्णाङ्क विस्थापनम्, size_t read_आकार)
{
	संरचना mtd_info *mtd = get_nand_dev_by_सूचकांक(nand_curr_device);
	प्रत्यागम nand_read_skip_bad(mtd, विस्थापनम्,
				  &read_आकार, रिक्त,
				  mtd->आकार,
				  (u_char *)bmp_आरोपय_addr);
}
#अन्यथा
स्थिर पूर्णाङ्क splash_nand_read_raw(ulong bmp_आरोपय_addr, पूर्णाङ्क विस्थापनम्, size_t read_आकार)
{
	debug("%s: nand support not available\n", __func__);
	प्रत्यागम -ENOSYS;
}
#यदि_अन्तः

स्थिर पूर्णाङ्क splash_mmc_read_raw(ulong bmp_आरोपय_addr, संरचना splash_location *location,
			       size_t read_आकार)
{
	संरचना disk_partition partition;
	संरचना blk_desc *desc;
	lbaint_t blkcnt;
	पूर्णाङ्क ret, n;

	यदि (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		प्रत्यागम -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_अङ्कः("mmc", location->devpart, &desc,
						   &partition, 1);
	यदि (ret < 0)
		प्रत्यागम ret;

	blkcnt = DIV_आवर्तनम्_ऊर्ध्वम्(read_आकार, partition.blksz);
	n = blk_dread(desc, partition.start, blkcnt, (शून्य *)(uintptr_t)bmp_आरोपय_addr);

	प्रत्यागम (n == blkcnt) ? 0 : -EIO;
}

स्थिर पूर्णाङ्क splash_storage_read_raw(संरचना splash_location *location,
			       ulong bmp_आरोपय_addr, size_t read_आकार)
{
	u32 विस्थापनम्;

	यदि (!location)
		प्रत्यागम -EINVAL;

	विस्थापनम् = location->विस्थापनम्;
	विकल्प (location->storage) {
	पक्ष SPLASH_STORAGE_MMC:
		प्रत्यागम splash_mmc_read_raw(bmp_आरोपय_addr, location, read_आकार);
	पक्ष SPLASH_STORAGE_NAND:
		प्रत्यागम splash_nand_read_raw(bmp_आरोपय_addr, विस्थापनम्, read_आकार);
	पक्ष SPLASH_STORAGE_SF:
		प्रत्यागम splash_sf_read_raw(bmp_आरोपय_addr, विस्थापनम्, read_आकार);
	स्वभाव:
		प्रारूप_लिख("Unknown splash location\n");
	}

	प्रत्यागम -EINVAL;
}

स्थिर पूर्णाङ्क splash_आरोपय_raw(संरचना splash_location *location, ulong bmp_आरोपय_addr)
{
	संरचना bmp_शीर्षकम् *bmp_hdr;
	पूर्णाङ्क res;
	size_t bmp_आकार, bmp_शीर्षकम्_आकार = परिमाण(संरचना bmp_शीर्षकम्);

	यदि (bmp_आरोपय_addr + bmp_शीर्षकम्_आकार >= gd->start_addr_sp)
		गच्छ splash_स्थानाङ्कः_too_उच्चः;

	res = splash_storage_read_raw(location, bmp_आरोपय_addr, bmp_शीर्षकम्_आकार);
	यदि (res < 0)
		प्रत्यागम res;

	bmp_hdr = (संरचना bmp_शीर्षकम् *)(uintptr_t)bmp_आरोपय_addr;
	bmp_आकार = le32_गन्तव्यम्_cpu(bmp_hdr->file_आकार);

	यदि (bmp_आरोपय_addr + bmp_आकार >= gd->start_addr_sp)
		गच्छ splash_स्थानाङ्कः_too_उच्चः;

	प्रत्यागम splash_storage_read_raw(location, bmp_आरोपय_addr, bmp_आकार);

splash_स्थानाङ्कः_too_उच्चः:
	प्रारूप_लिख("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	प्रत्यागम -EFAULT;
}

स्थिर पूर्णाङ्क splash_select_fs_dev(संरचना splash_location *location)
{
	पूर्णाङ्क res;

	विकल्प (location->storage) {
	पक्ष SPLASH_STORAGE_MMC:
		res = fs_समुच्चयः_blk_dev("mmc", location->devpart, FS_TYPE_ANY);
		भङ्ग;
	पक्ष SPLASH_STORAGE_USB:
		res = fs_समुच्चयः_blk_dev("usb", location->devpart, FS_TYPE_ANY);
		भङ्ग;
	पक्ष SPLASH_STORAGE_SATA:
		res = fs_समुच्चयः_blk_dev("sata", location->devpart, FS_TYPE_ANY);
		भङ्ग;
	पक्ष SPLASH_STORAGE_NAND:
		यदि (location->ubivol != रिक्त)
			res = fs_समुच्चयः_blk_dev("ubi", रिक्त, FS_TYPE_UBIFS);
		अन्यथा
			res = -ENODEV;
		भङ्ग;
	स्वभाव:
		प्रारूप_लिख("Error: %s: unsupported location storage.\n", __func__);
		प्रत्यागम -ENODEV;
	}

	यदि (res)
		प्रारूप_लिख("Error: %s: could not access storage.\n", __func__);

	प्रत्यागम res;
}

#यदि_परिभाषितम् CONFIG_USB_STORAGE
स्थिर पूर्णाङ्क splash_init_usb(शून्य)
{
	पूर्णाङ्क err;

	err = usb_init();
	यदि (err)
		प्रत्यागम err;

#यदि_अपरिभाषितम् CONFIG_DM_USB
	err = usb_stor_अवलोकनम्(1) < 0 ? -ENODEV : 0;
#यदि_अन्तः

	प्रत्यागम err;
}
#अन्यथा
स्थिर अन्तःस्थ पूर्णाङ्क splash_init_usb(शून्य)
{
	प्रारूप_लिख("Cannot load splash image: no USB support\n");
	प्रत्यागम -ENOSYS;
}
#यदि_अन्तः

#यदि_परिभाषितम् CONFIG_SATA
स्थिर पूर्णाङ्क splash_init_sata(शून्य)
{
	प्रत्यागम sata_probe(0);
}
#अन्यथा
स्थिर अन्तःस्थ पूर्णाङ्क splash_init_sata(शून्य)
{
	प्रारूप_लिख("Cannot load splash image: no SATA support\n");
	प्रत्यागम -ENOSYS;
}
#यदि_अन्तः

स्थिर पूर्णाङ्क splash_init_virtio(शून्य)
{
	यदि (!IS_ENABLED(CONFIG_VIRTIO)) {
		प्रारूप_लिख("Cannot load splash image: no virtio support\n");
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
	res = सततम्_command(cmd, 0);
	यदि (res)
		प्रत्यागम res;

	sprintf(cmd, "ubifsmount %s", location->ubivol);
	res = सततम्_command(cmd, 0);

	प्रत्यागम res;
}

स्थिर अन्तःस्थ पूर्णाङ्क splash_umount_ubifs(शून्य)
{
	प्रत्यागम सततम्_command("ubifsumount", 0);
}
#अन्यथा
स्थिर अन्तःस्थ पूर्णाङ्क splash_mount_ubifs(संरचना splash_location *location)
{
	प्रारूप_लिख("Cannot load splash image: no UBIFS support\n");
	प्रत्यागम -ENOSYS;
}

स्थिर अन्तःस्थ पूर्णाङ्क splash_umount_ubifs(शून्य)
{
	प्रारूप_लिख("Cannot unmount UBIFS: no UBIFS support\n");
	प्रत्यागम -ENOSYS;
}
#यदि_अन्तः

#परिभाषय SPLASH_SOURCE_DEFAULT_FILE_NAME		"splash.bmp"

स्थिर पूर्णाङ्क splash_आरोपय_fs(संरचना splash_location *location, ulong bmp_आरोपय_addr)
{
	पूर्णाङ्क res = 0;
	loff_t bmp_आकार;
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
		गच्छ बहिर्गमनम्;

	res = fs_आकार(splash_file, &bmp_आकार);
	यदि (res) {
		प्रारूप_लिख("Error: %s: cannot determine file size (%d)\n",
		       __func__, res);
		गच्छ बहिर्गमनम्;
	}

	यदि (bmp_आरोपय_addr + bmp_आकार >= gd->start_addr_sp) {
		प्रारूप_लिख("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		गच्छ बहिर्गमनम्;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_आरोपय_addr, 0, 0, &actread);

बहिर्गमनम्:
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
			    संरचना splash_location *locations, uint आकार)
{
	पूर्णाङ्क i;
	वर्ण *env_splashsource;

	यदि (!locations || आकार == 0)
		प्रत्यागम रिक्त;

	env_splashsource = env_get("splashsource");
	यदि (env_splashsource == रिक्त)
		प्रत्यागम &locations[0];

	प्रति (i = 0; i < आकार; i++) {
		यदि (!वाक्यं_तुलय(locations[i].name, env_splashsource))
			प्रत्यागम &locations[i];
	}

	प्रारूप_लिख("splashsource env variable set to unsupported value\n");
	प्रत्यागम रिक्त;
}

#यदि_परिभाषितम् CONFIG_FIT
स्थिर पूर्णाङ्क splash_आरोपय_fit(संरचना splash_location *location, ulong bmp_आरोपय_addr)
{
	पूर्णाङ्क res;
	पूर्णाङ्क ग्रन्थिः_विस्थापनम्;
	ध्रुव वर्ण *splash_file;
	ध्रुव शून्य *आन्तरिकम्_splash_दत्त;
	size_t आन्तरिकम्_splash_आकार;
	पूर्णाङ्क external_splash_addr;
	u32 external_splash_आकार;
	bool is_splash_external = false;
	संरचना legacy_img_hdr *img_शीर्षकम्;
	ध्रुव u32 *fit_शीर्षकम्;
	u32 fit_आकार;
	ध्रुव size_t शीर्षकम्_आकार = परिमाण(संरचना legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_आरोपय_addr, शीर्षकम्_आकार);
	यदि (res < 0)
		प्रत्यागम res;

	img_शीर्षकम् = (संरचना legacy_img_hdr *)(uintptr_t)bmp_आरोपय_addr;
	यदि (image_get_magic(img_शीर्षकम्) != FDT_MAGIC) {
		प्रारूप_लिख("Could not find FDT magic\n");
		प्रत्यागम -EINVAL;
	}

	fit_आकार = fdt_totalsize(img_शीर्षकम्);

	/* Read in entire FIT */
	fit_शीर्षकम् = (ध्रुव u32 *)(bmp_आरोपय_addr + शीर्षकम्_आकार);
	res = splash_storage_read_raw(location, (uintptr_t)fit_शीर्षकम्, fit_आकार);
	यदि (res < 0)
		प्रत्यागम res;

	res = fit_check_format(fit_शीर्षकम्, IMAGE_आकार_INVAL);
	यदि (res) {
		debug("Could not find valid FIT image\n");
		प्रत्यागम res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	यदि (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	ग्रन्थिः_विस्थापनम् = fit_image_get_ग्रन्थिः(fit_शीर्षकम्, splash_file);
	यदि (ग्रन्थिः_विस्थापनम् < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		प्रत्यागम -ENOENT;
	}

	/* Extract the splash data from FIT */
	यदि (!fit_image_get_दत्त(fit_शीर्षकम्, ग्रन्थिः_विस्थापनम्, &आन्तरिकम्_splash_दत्त,
				&आन्तरिकम्_splash_आकार)) {
		memmove((शून्य *)(uintptr_t)bmp_आरोपय_addr, आन्तरिकम्_splash_दत्त,
			आन्तरिकम्_splash_आकार);
	} अन्यथा {
		प्रारूप_लिख("Failed to get splash image from FIT\n");
		प्रत्यागम -ENODATA;
	}

	यदि (is_splash_external) {
		res = fit_image_get_दत्त_आकार(fit_शीर्षकम्, ग्रन्थिः_विस्थापनम्, &external_splash_आकार);
		यदि (res < 0) {
			प्रारूप_लिख("Failed to get size of splash image (err=%d)\n", res);
			प्रत्यागम res;
		}

		/* Read in the splash data */
		location->विस्थापनम् = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_आरोपय_addr, external_splash_आकार);
		यदि (res < 0)
			प्रत्यागम res;
	}

	प्रत्यागम 0;
}
#यदि_अन्तः /* CONFIG_FIT */

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
पूर्णाङ्क splash_source_आरोपय(संरचना splash_location *locations, uint आकार)
{
	संरचना splash_location *splash_location;
	वर्ण *env_splashimage_मानम्;
	वर्ण *devpart;
	ulong bmp_आरोपय_addr;

	env_splashimage_मानम् = env_get("splashimage");
	यदि (env_splashimage_मानम् == रिक्त)
		प्रत्यागम -ENOENT;

	bmp_आरोपय_addr = hextoul(env_splashimage_मानम्, 0);
	यदि (bmp_आरोपय_addr == 0) {
		प्रारूप_लिख("Error: bad splashimage address specified\n");
		प्रत्यागम -EFAULT;
	}

	splash_location = select_splash_location(locations, आकार);
	यदि (!splash_location)
		प्रत्यागम -EINVAL;

	devpart = env_get("splashdevpart");
	यदि (devpart)
		splash_location->devpart = devpart;

	यदि (splash_location->flags == SPLASH_STORAGE_RAW)
		प्रत्यागम splash_आरोपय_raw(splash_location, bmp_आरोपय_addr);
	अन्यथा यदि (splash_location->flags == SPLASH_STORAGE_FS)
		प्रत्यागम splash_आरोपय_fs(splash_location, bmp_आरोपय_addr);
#यदि_परिभाषितम् CONFIG_FIT
	अन्यथा यदि (splash_location->flags == SPLASH_STORAGE_FIT)
		प्रत्यागम splash_आरोपय_fit(splash_location, bmp_आरोपय_addr);
#यदि_अन्तः
	प्रत्यागम -EINVAL;
}
