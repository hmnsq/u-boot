// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 */

#समावेश <bmp_layout.h>
#समावेश <command.h>
#समावेश <env.h>
#समावेश <errno.h>
#समावेश <fs.h>
#समावेश <fdt_support.h>
#समावेश <image.h>
#समावेश <log.h>
#समावेश <nand.h>
#समावेश <sata.h>
#समावेश <spi.h>
#समावेश <spi_flash.h>
#समावेश <splash.h>
#समावेश <usb.h>
#समावेश <virtio.h>
#समावेश <asm/global_data.h>

DECLARE_GLOBAL_तथ्य_PTR;

#अगर_घोषित CONFIG_SPI_FLASH
अटल काष्ठा spi_flash *sf;
अटल पूर्णांक splash_sf_read_raw(ulong bmp_load_addr, पूर्णांक offset, माप_प्रकार read_आकार)
{
	अगर (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_SPEED,
				     CONFIG_SF_DEFAULT_MODE);
		अगर (!sf)
			वापस -ENODEV;
	}

	वापस spi_flash_read(sf, offset, read_आकार, (व्योम *)(uintptr_t)bmp_load_addr);
}
#अन्यथा
अटल पूर्णांक splash_sf_read_raw(ulong bmp_load_addr, पूर्णांक offset, माप_प्रकार read_आकार)
{
	debug("%s: sf support not available\n", __func__);
	वापस -ENOSYS;
}
#पूर्ण_अगर

#अगर_घोषित CONFIG_CMD_NAND
अटल पूर्णांक splash_nand_read_raw(ulong bmp_load_addr, पूर्णांक offset, माप_प्रकार read_आकार)
{
	काष्ठा mtd_info *mtd = get_nand_dev_by_सूचकांक(nand_curr_device);
	वापस nand_read_skip_bad(mtd, offset,
				  &read_आकार, शून्य,
				  mtd->आकार,
				  (u_char *)bmp_load_addr);
}
#अन्यथा
अटल पूर्णांक splash_nand_read_raw(ulong bmp_load_addr, पूर्णांक offset, माप_प्रकार read_आकार)
{
	debug("%s: nand support not available\n", __func__);
	वापस -ENOSYS;
}
#पूर्ण_अगर

अटल पूर्णांक splash_mmc_read_raw(ulong bmp_load_addr, काष्ठा splash_location *location,
			       माप_प्रकार read_आकार)
{
	काष्ठा disk_partition partition;
	काष्ठा blk_desc *desc;
	lbaint_t blkcnt;
	पूर्णांक ret, n;

	अगर (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		वापस -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_अंक("mmc", location->devpart, &desc,
						   &partition, 1);
	अगर (ret < 0)
		वापस ret;

	blkcnt = DIV_ROUND_UP(read_आकार, partition.blksz);
	n = blk_dread(desc, partition.start, blkcnt, (व्योम *)(uintptr_t)bmp_load_addr);

	वापस (n == blkcnt) ? 0 : -EIO;
}

अटल पूर्णांक splash_storage_read_raw(काष्ठा splash_location *location,
			       ulong bmp_load_addr, माप_प्रकार read_आकार)
{
	u32 offset;

	अगर (!location)
		वापस -EINVAL;

	offset = location->offset;
	चयन (location->storage) {
	हाल SPLASH_STORAGE_MMC:
		वापस splash_mmc_read_raw(bmp_load_addr, location, read_आकार);
	हाल SPLASH_STORAGE_NAND:
		वापस splash_nand_read_raw(bmp_load_addr, offset, read_आकार);
	हाल SPLASH_STORAGE_SF:
		वापस splash_sf_read_raw(bmp_load_addr, offset, read_आकार);
	शेष:
		म_लिखो("Unknown splash location\n");
	}

	वापस -EINVAL;
}

अटल पूर्णांक splash_load_raw(काष्ठा splash_location *location, ulong bmp_load_addr)
{
	काष्ठा bmp_header *bmp_hdr;
	पूर्णांक res;
	माप_प्रकार bmp_आकार, bmp_header_आकार = माप(काष्ठा bmp_header);

	अगर (bmp_load_addr + bmp_header_आकार >= gd->start_addr_sp)
		जाओ splash_address_too_ऊपरी;

	res = splash_storage_read_raw(location, bmp_load_addr, bmp_header_आकार);
	अगर (res < 0)
		वापस res;

	bmp_hdr = (काष्ठा bmp_header *)(uintptr_t)bmp_load_addr;
	bmp_आकार = le32_to_cpu(bmp_hdr->file_आकार);

	अगर (bmp_load_addr + bmp_आकार >= gd->start_addr_sp)
		जाओ splash_address_too_ऊपरी;

	वापस splash_storage_read_raw(location, bmp_load_addr, bmp_आकार);

splash_address_too_ऊपरी:
	म_लिखो("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	वापस -EFAULT;
}

अटल पूर्णांक splash_select_fs_dev(काष्ठा splash_location *location)
{
	पूर्णांक res;

	चयन (location->storage) {
	हाल SPLASH_STORAGE_MMC:
		res = fs_set_blk_dev("mmc", location->devpart, FS_TYPE_ANY);
		अवरोध;
	हाल SPLASH_STORAGE_USB:
		res = fs_set_blk_dev("usb", location->devpart, FS_TYPE_ANY);
		अवरोध;
	हाल SPLASH_STORAGE_SATA:
		res = fs_set_blk_dev("sata", location->devpart, FS_TYPE_ANY);
		अवरोध;
	हाल SPLASH_STORAGE_NAND:
		अगर (location->ubivol != शून्य)
			res = fs_set_blk_dev("ubi", शून्य, FS_TYPE_UBIFS);
		अन्यथा
			res = -ENODEV;
		अवरोध;
	शेष:
		म_लिखो("Error: %s: unsupported location storage.\n", __func__);
		वापस -ENODEV;
	}

	अगर (res)
		म_लिखो("Error: %s: could not access storage.\n", __func__);

	वापस res;
}

#अगर_घोषित CONFIG_USB_STORAGE
अटल पूर्णांक splash_init_usb(व्योम)
{
	पूर्णांक err;

	err = usb_init();
	अगर (err)
		वापस err;

#अगर_अघोषित CONFIG_DM_USB
	err = usb_stor_scan(1) < 0 ? -ENODEV : 0;
#पूर्ण_अगर

	वापस err;
}
#अन्यथा
अटल अंतरभूत पूर्णांक splash_init_usb(व्योम)
{
	म_लिखो("Cannot load splash image: no USB support\n");
	वापस -ENOSYS;
}
#पूर्ण_अगर

#अगर_घोषित CONFIG_SATA
अटल पूर्णांक splash_init_sata(व्योम)
{
	वापस sata_probe(0);
}
#अन्यथा
अटल अंतरभूत पूर्णांक splash_init_sata(व्योम)
{
	म_लिखो("Cannot load splash image: no SATA support\n");
	वापस -ENOSYS;
}
#पूर्ण_अगर

अटल पूर्णांक splash_init_virtio(व्योम)
{
	अगर (!IS_ENABLED(CONFIG_VIRTIO)) {
		म_लिखो("Cannot load splash image: no virtio support\n");
		वापस -ENOSYS;
	} अन्यथा {
		वापस virtio_init();
	}
}

#अगर defined(CONFIG_CMD_UBIFS) && !defined(CONFIG_XPL_BUILD)
अटल पूर्णांक splash_mount_ubifs(काष्ठा splash_location *location)
{
	पूर्णांक res;
	अक्षर cmd[32];

	प्र_लिखो(cmd, "ubi part %s", location->mtdpart);
	res = run_command(cmd, 0);
	अगर (res)
		वापस res;

	प्र_लिखो(cmd, "ubifsmount %s", location->ubivol);
	res = run_command(cmd, 0);

	वापस res;
}

अटल अंतरभूत पूर्णांक splash_umount_ubifs(व्योम)
{
	वापस run_command("ubifsumount", 0);
}
#अन्यथा
अटल अंतरभूत पूर्णांक splash_mount_ubifs(काष्ठा splash_location *location)
{
	म_लिखो("Cannot load splash image: no UBIFS support\n");
	वापस -ENOSYS;
}

अटल अंतरभूत पूर्णांक splash_umount_ubifs(व्योम)
{
	म_लिखो("Cannot unmount UBIFS: no UBIFS support\n");
	वापस -ENOSYS;
}
#पूर्ण_अगर

#घोषणा SPLASH_SOURCE_DEFAULT_FILE_NAME		"splash.bmp"

अटल पूर्णांक splash_load_fs(काष्ठा splash_location *location, ulong bmp_load_addr)
{
	पूर्णांक res = 0;
	loff_t bmp_आकार;
	loff_t actread;
	अक्षर *splash_file;

	splash_file = env_get("splashfile");
	अगर (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	अगर (location->storage == SPLASH_STORAGE_USB)
		res = splash_init_usb();

	अगर (location->storage == SPLASH_STORAGE_SATA)
		res = splash_init_sata();

	अगर (location->storage == SPLASH_STORAGE_VIRTIO)
		res = splash_init_virtio();

	अगर (location->ubivol != शून्य)
		res = splash_mount_ubifs(location);

	अगर (res)
		वापस res;

	res = splash_select_fs_dev(location);
	अगर (res)
		जाओ out;

	res = fs_आकार(splash_file, &bmp_आकार);
	अगर (res) {
		म_लिखो("Error: %s: cannot determine file size (%d)\n",
		       __func__, res);
		जाओ out;
	}

	अगर (bmp_load_addr + bmp_आकार >= gd->start_addr_sp) {
		म_लिखो("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		जाओ out;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_load_addr, 0, 0, &actread);

out:
	अगर (location->ubivol != शून्य)
		splash_umount_ubifs();

	वापस res;
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
अटल काष्ठा splash_location *select_splash_location(
			    काष्ठा splash_location *locations, uint आकार)
{
	पूर्णांक i;
	अक्षर *env_splashsource;

	अगर (!locations || आकार == 0)
		वापस शून्य;

	env_splashsource = env_get("splashsource");
	अगर (env_splashsource == शून्य)
		वापस &locations[0];

	क्रम (i = 0; i < आकार; i++) {
		अगर (!म_भेद(locations[i].name, env_splashsource))
			वापस &locations[i];
	}

	म_लिखो("splashsource env variable set to unsupported value\n");
	वापस शून्य;
}

#अगर_घोषित CONFIG_FIT
अटल पूर्णांक splash_load_fit(काष्ठा splash_location *location, ulong bmp_load_addr)
{
	पूर्णांक res;
	पूर्णांक गाँठ_offset;
	स्थिर अक्षर *splash_file;
	स्थिर व्योम *internal_splash_तथ्य;
	माप_प्रकार internal_splash_आकार;
	पूर्णांक external_splash_addr;
	u32 external_splash_आकार;
	bool is_splash_external = false;
	काष्ठा legacy_img_hdr *img_header;
	स्थिर u32 *fit_header;
	u32 fit_आकार;
	स्थिर माप_प्रकार header_आकार = माप(काष्ठा legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_load_addr, header_आकार);
	अगर (res < 0)
		वापस res;

	img_header = (काष्ठा legacy_img_hdr *)(uintptr_t)bmp_load_addr;
	अगर (image_get_magic(img_header) != FDT_MAGIC) {
		म_लिखो("Could not find FDT magic\n");
		वापस -EINVAL;
	}

	fit_आकार = fdt_totalsize(img_header);

	/* Read in entire FIT */
	fit_header = (स्थिर u32 *)(bmp_load_addr + header_आकार);
	res = splash_storage_read_raw(location, (uintptr_t)fit_header, fit_आकार);
	अगर (res < 0)
		वापस res;

	res = fit_check_format(fit_header, IMAGE_आकार_INVAL);
	अगर (res) {
		debug("Could not find valid FIT image\n");
		वापस res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	अगर (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	गाँठ_offset = fit_image_get_गाँठ(fit_header, splash_file);
	अगर (गाँठ_offset < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		वापस -ENOENT;
	}

	/* Extract the splash data from FIT */
	अगर (!fit_image_get_तथ्य(fit_header, गाँठ_offset, &internal_splash_तथ्य,
				&internal_splash_आकार)) {
		स_हटाओ((व्योम *)(uintptr_t)bmp_load_addr, internal_splash_तथ्य,
			internal_splash_आकार);
	} अन्यथा {
		म_लिखो("Failed to get splash image from FIT\n");
		वापस -ENODATA;
	}

	अगर (is_splash_external) {
		res = fit_image_get_तथ्य_आकार(fit_header, गाँठ_offset, &external_splash_आकार);
		अगर (res < 0) {
			म_लिखो("Failed to get size of splash image (err=%d)\n", res);
			वापस res;
		}

		/* Read in the splash data */
		location->offset = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_load_addr, external_splash_आकार);
		अगर (res < 0)
			वापस res;
	}

	वापस 0;
}
#पूर्ण_अगर /* CONFIG_FIT */

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
पूर्णांक splash_source_load(काष्ठा splash_location *locations, uint आकार)
{
	काष्ठा splash_location *splash_location;
	अक्षर *env_splashimage_मूल्य;
	अक्षर *devpart;
	ulong bmp_load_addr;

	env_splashimage_मूल्य = env_get("splashimage");
	अगर (env_splashimage_मूल्य == शून्य)
		वापस -ENOENT;

	bmp_load_addr = hextoul(env_splashimage_मूल्य, 0);
	अगर (bmp_load_addr == 0) {
		म_लिखो("Error: bad splashimage address specified\n");
		वापस -EFAULT;
	}

	splash_location = select_splash_location(locations, आकार);
	अगर (!splash_location)
		वापस -EINVAL;

	devpart = env_get("splashdevpart");
	अगर (devpart)
		splash_location->devpart = devpart;

	अगर (splash_location->flags == SPLASH_STORAGE_RAW)
		वापस splash_load_raw(splash_location, bmp_load_addr);
	अन्यथा अगर (splash_location->flags == SPLASH_STORAGE_FS)
		वापस splash_load_fs(splash_location, bmp_load_addr);
#अगर_घोषित CONFIG_FIT
	अन्यथा अगर (splash_location->flags == SPLASH_STORAGE_FIT)
		वापस splash_load_fit(splash_location, bmp_load_addr);
#पूर्ण_अगर
	वापस -EINVAL;
}
