// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 */

#شامل <bmp_layout.h>
#شامل <command.h>
#شامل <env.h>
#شامل <errno.h>
#شامل <fs.h>
#شامل <fdt_support.h>
#شامل <image.h>
#شامل <log.h>
#شامل <nand.h>
#شامل <sata.h>
#شامل <spi.h>
#شامل <spi_flash.h>
#شامل <splash.h>
#شامل <usb.h>
#شامل <virtio.h>
#شامل <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SPI_FLASH
ایستا ساختار spi_flash *sf;
ایستا صحیح splash_sf_read_raw(ulong bmp_load_addr, صحیح offset, size_t read_size)
{
	اگر (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_SPEED,
				     CONFIG_SF_DEFAULT_MODE);
		اگر (!sf)
			بازگشت -ENODEV;
	}

	بازگشت spi_flash_read(sf, offset, read_size, (تهی *)(uintptr_t)bmp_load_addr);
}
#وگرنه
ایستا صحیح splash_sf_read_raw(ulong bmp_load_addr, صحیح offset, size_t read_size)
{
	debug("%s: sf support not available\n", __func__);
	بازگشت -ENOSYS;
}
#endif

#ifdef CONFIG_CMD_NAND
ایستا صحیح splash_nand_read_raw(ulong bmp_load_addr, صحیح offset, size_t read_size)
{
	ساختار mtd_info *mtd = get_nand_dev_by_index(nand_curr_device);
	بازگشت nand_read_skip_bad(mtd, offset,
				  &read_size, پوچ,
				  mtd->size,
				  (u_char *)bmp_load_addr);
}
#وگرنه
ایستا صحیح splash_nand_read_raw(ulong bmp_load_addr, صحیح offset, size_t read_size)
{
	debug("%s: nand support not available\n", __func__);
	بازگشت -ENOSYS;
}
#endif

ایستا صحیح splash_mmc_read_raw(ulong bmp_load_addr, ساختار splash_location *location,
			       size_t read_size)
{
	ساختار disk_partition partition;
	ساختار blk_desc *desc;
	lbaint_t blkcnt;
	صحیح ret, n;

	اگر (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		بازگشت -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_رقم("mmc", location->devpart, &desc,
						   &partition, 1);
	اگر (ret < 0)
		بازگشت ret;

	blkcnt = DIV_ROUND_UP(read_size, partition.blksz);
	n = blk_dread(desc, partition.start, blkcnt, (تهی *)(uintptr_t)bmp_load_addr);

	بازگشت (n == blkcnt) ? 0 : -EIO;
}

ایستا صحیح splash_storage_read_raw(ساختار splash_location *location,
			       ulong bmp_load_addr, size_t read_size)
{
	u32 offset;

	اگر (!location)
		بازگشت -EINVAL;

	offset = location->offset;
	انتخاب (location->storage) {
	حالت SPLASH_STORAGE_MMC:
		بازگشت splash_mmc_read_raw(bmp_load_addr, location, read_size);
	حالت SPLASH_STORAGE_NAND:
		بازگشت splash_nand_read_raw(bmp_load_addr, offset, read_size);
	حالت SPLASH_STORAGE_SF:
		بازگشت splash_sf_read_raw(bmp_load_addr, offset, read_size);
	پیشفرض:
		printf("Unknown splash location\n");
	}

	بازگشت -EINVAL;
}

ایستا صحیح splash_load_raw(ساختار splash_location *location, ulong bmp_load_addr)
{
	ساختار bmp_header *bmp_hdr;
	صحیح res;
	size_t bmp_size, bmp_header_size = اندازه(ساختار bmp_header);

	اگر (bmp_load_addr + bmp_header_size >= gd->start_addr_sp)
		برو splash_address_too_بالا;

	res = splash_storage_read_raw(location, bmp_load_addr, bmp_header_size);
	اگر (res < 0)
		بازگشت res;

	bmp_hdr = (ساختار bmp_header *)(uintptr_t)bmp_load_addr;
	bmp_size = le32_to_cpu(bmp_hdr->file_size);

	اگر (bmp_load_addr + bmp_size >= gd->start_addr_sp)
		برو splash_address_too_بالا;

	بازگشت splash_storage_read_raw(location, bmp_load_addr, bmp_size);

splash_address_too_بالا:
	printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	بازگشت -EFAULT;
}

ایستا صحیح splash_select_fs_dev(ساختار splash_location *location)
{
	صحیح res;

	انتخاب (location->storage) {
	حالت SPLASH_STORAGE_MMC:
		res = fs_set_blk_dev("mmc", location->devpart, FS_TYPE_ANY);
		بشکن;
	حالت SPLASH_STORAGE_USB:
		res = fs_set_blk_dev("usb", location->devpart, FS_TYPE_ANY);
		بشکن;
	حالت SPLASH_STORAGE_SATA:
		res = fs_set_blk_dev("sata", location->devpart, FS_TYPE_ANY);
		بشکن;
	حالت SPLASH_STORAGE_NAND:
		اگر (location->ubivol != پوچ)
			res = fs_set_blk_dev("ubi", پوچ, FS_TYPE_UBIFS);
		وگرنه
			res = -ENODEV;
		بشکن;
	پیشفرض:
		printf("Error: %s: unsupported location storage.\n", __func__);
		بازگشت -ENODEV;
	}

	اگر (res)
		printf("Error: %s: could not access storage.\n", __func__);

	بازگشت res;
}

#ifdef CONFIG_USB_STORAGE
ایستا صحیح splash_init_usb(تهی)
{
	صحیح err;

	err = usb_init();
	اگر (err)
		بازگشت err;

#ifndef CONFIG_DM_USB
	err = usb_stor_scan(1) < 0 ? -ENODEV : 0;
#endif

	بازگشت err;
}
#وگرنه
ایستا درونخطی صحیح splash_init_usb(تهی)
{
	printf("Cannot load splash image: no USB support\n");
	بازگشت -ENOSYS;
}
#endif

#ifdef CONFIG_SATA
ایستا صحیح splash_init_sata(تهی)
{
	بازگشت sata_probe(0);
}
#وگرنه
ایستا درونخطی صحیح splash_init_sata(تهی)
{
	printf("Cannot load splash image: no SATA support\n");
	بازگشت -ENOSYS;
}
#endif

ایستا صحیح splash_init_virtio(تهی)
{
	اگر (!IS_ENABLED(CONFIG_VIRTIO)) {
		printf("Cannot load splash image: no virtio support\n");
		بازگشت -ENOSYS;
	} وگرنه {
		بازگشت virtio_init();
	}
}

#اگر defined(CONFIG_CMD_UBIFS) && !defined(CONFIG_XPL_BUILD)
ایستا صحیح splash_mount_ubifs(ساختار splash_location *location)
{
	صحیح res;
	نویسه cmd[32];

	sprintf(cmd, "ubi part %s", location->mtdpart);
	res = run_command(cmd, 0);
	اگر (res)
		بازگشت res;

	sprintf(cmd, "ubifsmount %s", location->ubivol);
	res = run_command(cmd, 0);

	بازگشت res;
}

ایستا درونخطی صحیح splash_umount_ubifs(تهی)
{
	بازگشت run_command("ubifsumount", 0);
}
#وگرنه
ایستا درونخطی صحیح splash_mount_ubifs(ساختار splash_location *location)
{
	printf("Cannot load splash image: no UBIFS support\n");
	بازگشت -ENOSYS;
}

ایستا درونخطی صحیح splash_umount_ubifs(تهی)
{
	printf("Cannot unmount UBIFS: no UBIFS support\n");
	بازگشت -ENOSYS;
}
#endif

#define SPLASH_SOURCE_DEFAULT_FILE_NAME		"splash.bmp"

ایستا صحیح splash_load_fs(ساختار splash_location *location, ulong bmp_load_addr)
{
	صحیح res = 0;
	loff_t bmp_size;
	loff_t actread;
	نویسه *splash_file;

	splash_file = env_get("splashfile");
	اگر (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	اگر (location->storage == SPLASH_STORAGE_USB)
		res = splash_init_usb();

	اگر (location->storage == SPLASH_STORAGE_SATA)
		res = splash_init_sata();

	اگر (location->storage == SPLASH_STORAGE_VIRTIO)
		res = splash_init_virtio();

	اگر (location->ubivol != پوچ)
		res = splash_mount_ubifs(location);

	اگر (res)
		بازگشت res;

	res = splash_select_fs_dev(location);
	اگر (res)
		برو out;

	res = fs_size(splash_file, &bmp_size);
	اگر (res) {
		printf("Error: %s: cannot determine file size (%d)\n",
		       __func__, res);
		برو out;
	}

	اگر (bmp_load_addr + bmp_size >= gd->start_addr_sp) {
		printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		برو out;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_load_addr, 0, 0, &actread);

out:
	اگر (location->ubivol != پوچ)
		splash_umount_ubifs();

	بازگشت res;
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
ایستا ساختار splash_location *select_splash_location(
			    ساختار splash_location *locations, uint size)
{
	صحیح i;
	نویسه *env_splashsource;

	اگر (!locations || size == 0)
		بازگشت پوچ;

	env_splashsource = env_get("splashsource");
	اگر (env_splashsource == پوچ)
		بازگشت &locations[0];

	برای (i = 0; i < size; i++) {
		اگر (!strcmp(locations[i].name, env_splashsource))
			بازگشت &locations[i];
	}

	printf("splashsource env variable set to unsupported value\n");
	بازگشت پوچ;
}

#ifdef CONFIG_FIT
ایستا صحیح splash_load_fit(ساختار splash_location *location, ulong bmp_load_addr)
{
	صحیح res;
	صحیح گره_offset;
	ثابت نویسه *splash_file;
	ثابت تهی *internal_splash_data;
	size_t internal_splash_size;
	صحیح external_splash_addr;
	u32 external_splash_size;
	bool is_splash_external = false;
	ساختار legacy_img_hdr *img_header;
	ثابت u32 *fit_header;
	u32 fit_size;
	ثابت size_t header_size = اندازه(ساختار legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_load_addr, header_size);
	اگر (res < 0)
		بازگشت res;

	img_header = (ساختار legacy_img_hdr *)(uintptr_t)bmp_load_addr;
	اگر (image_get_magic(img_header) != FDT_MAGIC) {
		printf("Could not find FDT magic\n");
		بازگشت -EINVAL;
	}

	fit_size = fdt_totalsize(img_header);

	/* Read in entire FIT */
	fit_header = (ثابت u32 *)(bmp_load_addr + header_size);
	res = splash_storage_read_raw(location, (uintptr_t)fit_header, fit_size);
	اگر (res < 0)
		بازگشت res;

	res = fit_check_format(fit_header, IMAGE_SIZE_INVAL);
	اگر (res) {
		debug("Could not find valid FIT image\n");
		بازگشت res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	اگر (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	گره_offset = fit_image_get_گره(fit_header, splash_file);
	اگر (گره_offset < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		بازگشت -ENOENT;
	}

	/* Extract the splash data from FIT */
	اگر (!fit_image_get_data(fit_header, گره_offset, &internal_splash_data,
				&internal_splash_size)) {
		memmove((تهی *)(uintptr_t)bmp_load_addr, internal_splash_data,
			internal_splash_size);
	} وگرنه {
		printf("Failed to get splash image from FIT\n");
		بازگشت -ENODATA;
	}

	اگر (is_splash_external) {
		res = fit_image_get_data_size(fit_header, گره_offset, &external_splash_size);
		اگر (res < 0) {
			printf("Failed to get size of splash image (err=%d)\n", res);
			بازگشت res;
		}

		/* Read in the splash data */
		location->offset = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_load_addr, external_splash_size);
		اگر (res < 0)
			بازگشت res;
	}

	بازگشت 0;
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
صحیح splash_source_load(ساختار splash_location *locations, uint size)
{
	ساختار splash_location *splash_location;
	نویسه *env_splashimage_مقدار;
	نویسه *devpart;
	ulong bmp_load_addr;

	env_splashimage_مقدار = env_get("splashimage");
	اگر (env_splashimage_مقدار == پوچ)
		بازگشت -ENOENT;

	bmp_load_addr = hextoul(env_splashimage_مقدار, 0);
	اگر (bmp_load_addr == 0) {
		printf("Error: bad splashimage address specified\n");
		بازگشت -EFAULT;
	}

	splash_location = select_splash_location(locations, size);
	اگر (!splash_location)
		بازگشت -EINVAL;

	devpart = env_get("splashdevpart");
	اگر (devpart)
		splash_location->devpart = devpart;

	اگر (splash_location->flags == SPLASH_STORAGE_RAW)
		بازگشت splash_load_raw(splash_location, bmp_load_addr);
	وگرنه اگر (splash_location->flags == SPLASH_STORAGE_FS)
		بازگشت splash_load_fs(splash_location, bmp_load_addr);
#ifdef CONFIG_FIT
	وگرنه اگر (splash_location->flags == SPLASH_STORAGE_FIT)
		بازگشت splash_load_fit(splash_location, bmp_load_addr);
#endif
	بازگشت -EINVAL;
}
