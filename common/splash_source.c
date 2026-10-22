// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 */

#அந்தர்பாவ <bmp_layout.h>
#அந்தர்பாவ <command.h>
#அந்தர்பாவ <env.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <fs.h>
#அந்தர்பாவ <fdt_support.h>
#அந்தர்பாவ <image.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <nand.h>
#அந்தர்பாவ <sata.h>
#அந்தர்பாவ <spi.h>
#அந்தர்பாவ <spi_flash.h>
#அந்தர்பாவ <splash.h>
#அந்தர்பாவ <usb.h>
#அந்தர்பாவ <virtio.h>
#அந்தர்பாவ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SPI_FLASH
நிலையான கட்டமைப்பு spi_flash *sf;
நிலையான முழு splash_sf_read_raw(ulong bmp_load_addr, முழு offset, size_t read_size)
{
	எனில் (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_SPEED,
				     CONFIG_SF_DEFAULT_MODE);
		எனில் (!sf)
			திரும்பு -ENODEV;
	}

	திரும்பு spi_flash_read(sf, offset, read_size, (வெறுமை *)(uintptr_t)bmp_load_addr);
}
#இல்லையெனில்
நிலையான முழு splash_sf_read_raw(ulong bmp_load_addr, முழு offset, size_t read_size)
{
	debug("%s: sf support not available\n", __func__);
	திரும்பு -ENOSYS;
}
#endif

#ifdef CONFIG_CMD_NAND
நிலையான முழு splash_nand_read_raw(ulong bmp_load_addr, முழு offset, size_t read_size)
{
	கட்டமைப்பு mtd_info *mtd = get_nand_dev_by_index(nand_curr_device);
	திரும்பு nand_read_skip_bad(mtd, offset,
				  &read_size, வெற்று,
				  mtd->size,
				  (u_char *)bmp_load_addr);
}
#இல்லையெனில்
நிலையான முழு splash_nand_read_raw(ulong bmp_load_addr, முழு offset, size_t read_size)
{
	debug("%s: nand support not available\n", __func__);
	திரும்பு -ENOSYS;
}
#endif

நிலையான முழு splash_mmc_read_raw(ulong bmp_load_addr, கட்டமைப்பு splash_location *location,
			       size_t read_size)
{
	கட்டமைப்பு disk_partition partition;
	கட்டமைப்பு blk_desc *desc;
	lbaint_t blkcnt;
	முழு ret, n;

	எனில் (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		திரும்பு -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_இலக்கம்("mmc", location->devpart, &desc,
						   &partition, 1);
	எனில் (ret < 0)
		திரும்பு ret;

	blkcnt = DIV_ROUND_UP(read_size, partition.blksz);
	n = blk_dread(desc, partition.start, blkcnt, (வெறுமை *)(uintptr_t)bmp_load_addr);

	திரும்பு (n == blkcnt) ? 0 : -EIO;
}

நிலையான முழு splash_storage_read_raw(கட்டமைப்பு splash_location *location,
			       ulong bmp_load_addr, size_t read_size)
{
	u32 offset;

	எனில் (!location)
		திரும்பு -EINVAL;

	offset = location->offset;
	தேர்வு (location->storage) {
	நிலை SPLASH_STORAGE_MMC:
		திரும்பு splash_mmc_read_raw(bmp_load_addr, location, read_size);
	நிலை SPLASH_STORAGE_NAND:
		திரும்பு splash_nand_read_raw(bmp_load_addr, offset, read_size);
	நிலை SPLASH_STORAGE_SF:
		திரும்பு splash_sf_read_raw(bmp_load_addr, offset, read_size);
	இயல்பு:
		printf("Unknown splash location\n");
	}

	திரும்பு -EINVAL;
}

நிலையான முழு splash_load_raw(கட்டமைப்பு splash_location *location, ulong bmp_load_addr)
{
	கட்டமைப்பு bmp_header *bmp_hdr;
	முழு res;
	size_t bmp_size, bmp_header_size = அளவு(கட்டமைப்பு bmp_header);

	எனில் (bmp_load_addr + bmp_header_size >= gd->start_addr_sp)
		செல் splash_address_too_உயர்வு;

	res = splash_storage_read_raw(location, bmp_load_addr, bmp_header_size);
	எனில் (res < 0)
		திரும்பு res;

	bmp_hdr = (கட்டமைப்பு bmp_header *)(uintptr_t)bmp_load_addr;
	bmp_size = le32_to_cpu(bmp_hdr->file_size);

	எனில் (bmp_load_addr + bmp_size >= gd->start_addr_sp)
		செல் splash_address_too_உயர்வு;

	திரும்பு splash_storage_read_raw(location, bmp_load_addr, bmp_size);

splash_address_too_உயர்வு:
	printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	திரும்பு -EFAULT;
}

நிலையான முழு splash_select_fs_dev(கட்டமைப்பு splash_location *location)
{
	முழு res;

	தேர்வு (location->storage) {
	நிலை SPLASH_STORAGE_MMC:
		res = fs_set_blk_dev("mmc", location->devpart, FS_TYPE_ANY);
		நிறுத்து;
	நிலை SPLASH_STORAGE_USB:
		res = fs_set_blk_dev("usb", location->devpart, FS_TYPE_ANY);
		நிறுத்து;
	நிலை SPLASH_STORAGE_SATA:
		res = fs_set_blk_dev("sata", location->devpart, FS_TYPE_ANY);
		நிறுத்து;
	நிலை SPLASH_STORAGE_NAND:
		எனில் (location->ubivol != வெற்று)
			res = fs_set_blk_dev("ubi", வெற்று, FS_TYPE_UBIFS);
		இல்லையெனில்
			res = -ENODEV;
		நிறுத்து;
	இயல்பு:
		printf("Error: %s: unsupported location storage.\n", __func__);
		திரும்பு -ENODEV;
	}

	எனில் (res)
		printf("Error: %s: could not access storage.\n", __func__);

	திரும்பு res;
}

#ifdef CONFIG_USB_STORAGE
நிலையான முழு splash_init_usb(வெறுமை)
{
	முழு err;

	err = usb_init();
	எனில் (err)
		திரும்பு err;

#ifndef CONFIG_DM_USB
	err = usb_stor_scan(1) < 0 ? -ENODEV : 0;
#endif

	திரும்பு err;
}
#இல்லையெனில்
நிலையான உள்வரி முழு splash_init_usb(வெறுமை)
{
	printf("Cannot load splash image: no USB support\n");
	திரும்பு -ENOSYS;
}
#endif

#ifdef CONFIG_SATA
நிலையான முழு splash_init_sata(வெறுமை)
{
	திரும்பு sata_probe(0);
}
#இல்லையெனில்
நிலையான உள்வரி முழு splash_init_sata(வெறுமை)
{
	printf("Cannot load splash image: no SATA support\n");
	திரும்பு -ENOSYS;
}
#endif

நிலையான முழு splash_init_virtio(வெறுமை)
{
	எனில் (!IS_ENABLED(CONFIG_VIRTIO)) {
		printf("Cannot load splash image: no virtio support\n");
		திரும்பு -ENOSYS;
	} இல்லையெனில் {
		திரும்பு virtio_init();
	}
}

#எனில் defined(CONFIG_CMD_UBIFS) && !defined(CONFIG_XPL_BUILD)
நிலையான முழு splash_mount_ubifs(கட்டமைப்பு splash_location *location)
{
	முழு res;
	எழுத்து cmd[32];

	sprintf(cmd, "ubi part %s", location->mtdpart);
	res = run_command(cmd, 0);
	எனில் (res)
		திரும்பு res;

	sprintf(cmd, "ubifsmount %s", location->ubivol);
	res = run_command(cmd, 0);

	திரும்பு res;
}

நிலையான உள்வரி முழு splash_umount_ubifs(வெறுமை)
{
	திரும்பு run_command("ubifsumount", 0);
}
#இல்லையெனில்
நிலையான உள்வரி முழு splash_mount_ubifs(கட்டமைப்பு splash_location *location)
{
	printf("Cannot load splash image: no UBIFS support\n");
	திரும்பு -ENOSYS;
}

நிலையான உள்வரி முழு splash_umount_ubifs(வெறுமை)
{
	printf("Cannot unmount UBIFS: no UBIFS support\n");
	திரும்பு -ENOSYS;
}
#endif

#define SPLASH_SOURCE_DEFAULT_FILE_NAME		"splash.bmp"

நிலையான முழு splash_load_fs(கட்டமைப்பு splash_location *location, ulong bmp_load_addr)
{
	முழு res = 0;
	loff_t bmp_size;
	loff_t actread;
	எழுத்து *splash_file;

	splash_file = env_get("splashfile");
	எனில் (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	எனில் (location->storage == SPLASH_STORAGE_USB)
		res = splash_init_usb();

	எனில் (location->storage == SPLASH_STORAGE_SATA)
		res = splash_init_sata();

	எனில் (location->storage == SPLASH_STORAGE_VIRTIO)
		res = splash_init_virtio();

	எனில் (location->ubivol != வெற்று)
		res = splash_mount_ubifs(location);

	எனில் (res)
		திரும்பு res;

	res = splash_select_fs_dev(location);
	எனில் (res)
		செல் out;

	res = fs_size(splash_file, &bmp_size);
	எனில் (res) {
		printf("Error: %s: cannot determine file size (%d)\n",
		       __func__, res);
		செல் out;
	}

	எனில் (bmp_load_addr + bmp_size >= gd->start_addr_sp) {
		printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		செல் out;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_load_addr, 0, 0, &actread);

out:
	எனில் (location->ubivol != வெற்று)
		splash_umount_ubifs();

	திரும்பு res;
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
நிலையான கட்டமைப்பு splash_location *select_splash_location(
			    கட்டமைப்பு splash_location *locations, uint size)
{
	முழு i;
	எழுத்து *env_splashsource;

	எனில் (!locations || size == 0)
		திரும்பு வெற்று;

	env_splashsource = env_get("splashsource");
	எனில் (env_splashsource == வெற்று)
		திரும்பு &locations[0];

	க்கு (i = 0; i < size; i++) {
		எனில் (!strcmp(locations[i].name, env_splashsource))
			திரும்பு &locations[i];
	}

	printf("splashsource env variable set to unsupported value\n");
	திரும்பு வெற்று;
}

#ifdef CONFIG_FIT
நிலையான முழு splash_load_fit(கட்டமைப்பு splash_location *location, ulong bmp_load_addr)
{
	முழு res;
	முழு முனை_offset;
	மாறிலி எழுத்து *splash_file;
	மாறிலி வெறுமை *internal_splash_data;
	size_t internal_splash_size;
	முழு external_splash_addr;
	u32 external_splash_size;
	bool is_splash_external = false;
	கட்டமைப்பு legacy_img_hdr *img_header;
	மாறிலி u32 *fit_header;
	u32 fit_size;
	மாறிலி size_t header_size = அளவு(கட்டமைப்பு legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_load_addr, header_size);
	எனில் (res < 0)
		திரும்பு res;

	img_header = (கட்டமைப்பு legacy_img_hdr *)(uintptr_t)bmp_load_addr;
	எனில் (image_get_magic(img_header) != FDT_MAGIC) {
		printf("Could not find FDT magic\n");
		திரும்பு -EINVAL;
	}

	fit_size = fdt_totalsize(img_header);

	/* Read in entire FIT */
	fit_header = (மாறிலி u32 *)(bmp_load_addr + header_size);
	res = splash_storage_read_raw(location, (uintptr_t)fit_header, fit_size);
	எனில் (res < 0)
		திரும்பு res;

	res = fit_check_format(fit_header, IMAGE_SIZE_INVAL);
	எனில் (res) {
		debug("Could not find valid FIT image\n");
		திரும்பு res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	எனில் (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	முனை_offset = fit_image_get_முனை(fit_header, splash_file);
	எனில் (முனை_offset < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		திரும்பு -ENOENT;
	}

	/* Extract the splash data from FIT */
	எனில் (!fit_image_get_data(fit_header, முனை_offset, &internal_splash_data,
				&internal_splash_size)) {
		memmove((வெறுமை *)(uintptr_t)bmp_load_addr, internal_splash_data,
			internal_splash_size);
	} இல்லையெனில் {
		printf("Failed to get splash image from FIT\n");
		திரும்பு -ENODATA;
	}

	எனில் (is_splash_external) {
		res = fit_image_get_data_size(fit_header, முனை_offset, &external_splash_size);
		எனில் (res < 0) {
			printf("Failed to get size of splash image (err=%d)\n", res);
			திரும்பு res;
		}

		/* Read in the splash data */
		location->offset = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_load_addr, external_splash_size);
		எனில் (res < 0)
			திரும்பு res;
	}

	திரும்பு 0;
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
முழு splash_source_load(கட்டமைப்பு splash_location *locations, uint size)
{
	கட்டமைப்பு splash_location *splash_location;
	எழுத்து *env_splashimage_மதிப்பு;
	எழுத்து *devpart;
	ulong bmp_load_addr;

	env_splashimage_மதிப்பு = env_get("splashimage");
	எனில் (env_splashimage_மதிப்பு == வெற்று)
		திரும்பு -ENOENT;

	bmp_load_addr = hextoul(env_splashimage_மதிப்பு, 0);
	எனில் (bmp_load_addr == 0) {
		printf("Error: bad splashimage address specified\n");
		திரும்பு -EFAULT;
	}

	splash_location = select_splash_location(locations, size);
	எனில் (!splash_location)
		திரும்பு -EINVAL;

	devpart = env_get("splashdevpart");
	எனில் (devpart)
		splash_location->devpart = devpart;

	எனில் (splash_location->flags == SPLASH_STORAGE_RAW)
		திரும்பு splash_load_raw(splash_location, bmp_load_addr);
	இல்லையெனில் எனில் (splash_location->flags == SPLASH_STORAGE_FS)
		திரும்பு splash_load_fs(splash_location, bmp_load_addr);
#ifdef CONFIG_FIT
	இல்லையெனில் எனில் (splash_location->flags == SPLASH_STORAGE_FIT)
		திரும்பு splash_load_fit(splash_location, bmp_load_addr);
#endif
	திரும்பு -EINVAL;
}
