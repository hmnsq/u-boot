// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 */

#ضمن <bmp_layout.h>
#ضمن <command.h>
#ضمن <env.h>
#ضمن <errno.h>
#ضمن <fs.h>
#ضمن <fdt_support.h>
#ضمن <image.h>
#ضمن <log.h>
#ضمن <nand.h>
#ضمن <sata.h>
#ضمن <spi.h>
#ضمن <spi_flash.h>
#ضمن <splash.h>
#ضمن <usb.h>
#ضمن <virtio.h>
#ضمن <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SPI_FLASH
ساكن بنية spi_flash *sf;
ساكن صحيح splash_sf_read_raw(ulong bmp_load_addr, صحيح offset, size_t read_size)
{
	אם (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_SPEED,
				     CONFIG_SF_DEFAULT_MODE);
		אם (!sf)
			החזר -ENODEV;
	}

	החזר spi_flash_read(sf, offset, read_size, (فراغ *)(uintptr_t)bmp_load_addr);
}
#אחרת
ساكن صحيح splash_sf_read_raw(ulong bmp_load_addr, صحيح offset, size_t read_size)
{
	debug("%s: sf support not available\n", __func__);
	החזר -ENOSYS;
}
#endif

#ifdef CONFIG_CMD_NAND
ساكن صحيح splash_nand_read_raw(ulong bmp_load_addr, صحيح offset, size_t read_size)
{
	بنية mtd_info *mtd = get_nand_dev_by_index(nand_curr_device);
	החזר nand_read_skip_bad(mtd, offset,
				  &read_size, عدم,
				  mtd->size,
				  (u_char *)bmp_load_addr);
}
#אחרת
ساكن صحيح splash_nand_read_raw(ulong bmp_load_addr, صحيح offset, size_t read_size)
{
	debug("%s: nand support not available\n", __func__);
	החזר -ENOSYS;
}
#endif

ساكن صحيح splash_mmc_read_raw(ulong bmp_load_addr, بنية splash_location *location,
			       size_t read_size)
{
	بنية disk_partition partition;
	بنية blk_desc *desc;
	lbaint_t blkcnt;
	صحيح ret, n;

	אם (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		החזר -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_num("mmc", location->devpart, &desc,
						   &partition, 1);
	אם (ret < 0)
		החזר ret;

	blkcnt = DIV_ROUND_UP(read_size, partition.blksz);
	n = blk_dread(desc, partition.start, blkcnt, (فراغ *)(uintptr_t)bmp_load_addr);

	החזר (n == blkcnt) ? 0 : -EIO;
}

ساكن صحيح splash_storage_read_raw(بنية splash_location *location,
			       ulong bmp_load_addr, size_t read_size)
{
	u32 offset;

	אם (!location)
		החזר -EINVAL;

	offset = location->offset;
	بدل (location->storage) {
	حالة SPLASH_STORAGE_MMC:
		החזר splash_mmc_read_raw(bmp_load_addr, location, read_size);
	حالة SPLASH_STORAGE_NAND:
		החזר splash_nand_read_raw(bmp_load_addr, offset, read_size);
	حالة SPLASH_STORAGE_SF:
		החזר splash_sf_read_raw(bmp_load_addr, offset, read_size);
	افتراضي:
		printf("Unknown splash location\n");
	}

	החזר -EINVAL;
}

ساكن صحيح splash_load_raw(بنية splash_location *location, ulong bmp_load_addr)
{
	بنية bmp_header *bmp_hdr;
	صحيح res;
	size_t bmp_size, bmp_header_size = حجم(بنية bmp_header);

	אם (bmp_load_addr + bmp_header_size >= gd->start_addr_sp)
		اذهب splash_address_too_high;

	res = splash_storage_read_raw(location, bmp_load_addr, bmp_header_size);
	אם (res < 0)
		החזר res;

	bmp_hdr = (بنية bmp_header *)(uintptr_t)bmp_load_addr;
	bmp_size = le32_to_cpu(bmp_hdr->file_size);

	אם (bmp_load_addr + bmp_size >= gd->start_addr_sp)
		اذهب splash_address_too_high;

	החזר splash_storage_read_raw(location, bmp_load_addr, bmp_size);

splash_address_too_high:
	printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	החזר -EFAULT;
}

ساكن صحيح splash_select_fs_dev(بنية splash_location *location)
{
	صحيح res;

	بدل (location->storage) {
	حالة SPLASH_STORAGE_MMC:
		res = fs_set_blk_dev("mmc", location->devpart, FS_TYPE_ANY);
		اكسر;
	حالة SPLASH_STORAGE_USB:
		res = fs_set_blk_dev("usb", location->devpart, FS_TYPE_ANY);
		اكسر;
	حالة SPLASH_STORAGE_SATA:
		res = fs_set_blk_dev("sata", location->devpart, FS_TYPE_ANY);
		اكسر;
	حالة SPLASH_STORAGE_NAND:
		אם (location->ubivol != عدم)
			res = fs_set_blk_dev("ubi", عدم, FS_TYPE_UBIFS);
		אחרת
			res = -ENODEV;
		اكسر;
	افتراضي:
		printf("Error: %s: unsupported location storage.\n", __func__);
		החזר -ENODEV;
	}

	אם (res)
		printf("Error: %s: could not access storage.\n", __func__);

	החזר res;
}

#ifdef CONFIG_USB_STORAGE
ساكن صحيح splash_init_usb(فراغ)
{
	صحيح err;

	err = usb_init();
	אם (err)
		החזר err;

#ifndef CONFIG_DM_USB
	err = usb_stor_scan(1) < 0 ? -ENODEV : 0;
#endif

	החזר err;
}
#אחרת
ساكن مضمن صحيح splash_init_usb(فراغ)
{
	printf("Cannot load splash image: no USB support\n");
	החזר -ENOSYS;
}
#endif

#ifdef CONFIG_SATA
ساكن صحيح splash_init_sata(فراغ)
{
	החזר sata_probe(0);
}
#אחרת
ساكن مضمن صحيح splash_init_sata(فراغ)
{
	printf("Cannot load splash image: no SATA support\n");
	החזר -ENOSYS;
}
#endif

ساكن صحيح splash_init_virtio(فراغ)
{
	אם (!IS_ENABLED(CONFIG_VIRTIO)) {
		printf("Cannot load splash image: no virtio support\n");
		החזר -ENOSYS;
	} אחרת {
		החזר virtio_init();
	}
}

#אם defined(CONFIG_CMD_UBIFS) && !defined(CONFIG_XPL_BUILD)
ساكن صحيح splash_mount_ubifs(بنية splash_location *location)
{
	صحيح res;
	حرف cmd[32];

	sprintf(cmd, "ubi part %s", location->mtdpart);
	res = run_command(cmd, 0);
	אם (res)
		החזר res;

	sprintf(cmd, "ubifsmount %s", location->ubivol);
	res = run_command(cmd, 0);

	החזר res;
}

ساكن مضمن صحيح splash_umount_ubifs(فراغ)
{
	החזר run_command("ubifsumount", 0);
}
#אחרת
ساكن مضمن صحيح splash_mount_ubifs(بنية splash_location *location)
{
	printf("Cannot load splash image: no UBIFS support\n");
	החזר -ENOSYS;
}

ساكن مضمن صحيح splash_umount_ubifs(فراغ)
{
	printf("Cannot unmount UBIFS: no UBIFS support\n");
	החזר -ENOSYS;
}
#endif

#define SPLASH_SOURCE_DEFAULT_FILE_NAME		"splash.bmp"

ساكن صحيح splash_load_fs(بنية splash_location *location, ulong bmp_load_addr)
{
	صحيح res = 0;
	loff_t bmp_size;
	loff_t actread;
	حرف *splash_file;

	splash_file = env_get("splashfile");
	אם (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	אם (location->storage == SPLASH_STORAGE_USB)
		res = splash_init_usb();

	אם (location->storage == SPLASH_STORAGE_SATA)
		res = splash_init_sata();

	אם (location->storage == SPLASH_STORAGE_VIRTIO)
		res = splash_init_virtio();

	אם (location->ubivol != عدم)
		res = splash_mount_ubifs(location);

	אם (res)
		החזר res;

	res = splash_select_fs_dev(location);
	אם (res)
		اذهب out;

	res = fs_size(splash_file, &bmp_size);
	אם (res) {
		printf("Error: %s: cannot determine file size (%d)\n",
		       __func__, res);
		اذهب out;
	}

	אם (bmp_load_addr + bmp_size >= gd->start_addr_sp) {
		printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		اذهب out;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_load_addr, 0, 0, &actread);

out:
	אם (location->ubivol != عدم)
		splash_umount_ubifs();

	החזר res;
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
ساكن بنية splash_location *select_splash_location(
			    بنية splash_location *locations, uint size)
{
	صحيح i;
	حرف *env_splashsource;

	אם (!locations || size == 0)
		החזר عدم;

	env_splashsource = env_get("splashsource");
	אם (env_splashsource == عدم)
		החזר &locations[0];

	עבור (i = 0; i < size; i++) {
		אם (!strcmp(locations[i].name, env_splashsource))
			החזר &locations[i];
	}

	printf("splashsource env variable set to unsupported value\n");
	החזר عدم;
}

#ifdef CONFIG_FIT
ساكن صحيح splash_load_fit(بنية splash_location *location, ulong bmp_load_addr)
{
	صحيح res;
	صحيح node_offset;
	ثابت حرف *splash_file;
	ثابت فراغ *internal_splash_data;
	size_t internal_splash_size;
	صحيح external_splash_addr;
	u32 external_splash_size;
	bool is_splash_external = false;
	بنية legacy_img_hdr *img_header;
	ثابت u32 *fit_header;
	u32 fit_size;
	ثابت size_t header_size = حجم(بنية legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_load_addr, header_size);
	אם (res < 0)
		החזר res;

	img_header = (بنية legacy_img_hdr *)(uintptr_t)bmp_load_addr;
	אם (image_get_magic(img_header) != FDT_MAGIC) {
		printf("Could not find FDT magic\n");
		החזר -EINVAL;
	}

	fit_size = fdt_totalsize(img_header);

	/* Read in entire FIT */
	fit_header = (ثابت u32 *)(bmp_load_addr + header_size);
	res = splash_storage_read_raw(location, (uintptr_t)fit_header, fit_size);
	אם (res < 0)
		החזר res;

	res = fit_check_format(fit_header, IMAGE_SIZE_INVAL);
	אם (res) {
		debug("Could not find valid FIT image\n");
		החזר res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	אם (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	node_offset = fit_image_get_node(fit_header, splash_file);
	אם (node_offset < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		החזר -ENOENT;
	}

	/* Extract the splash data from FIT */
	אם (!fit_image_get_data(fit_header, node_offset, &internal_splash_data,
				&internal_splash_size)) {
		memmove((فراغ *)(uintptr_t)bmp_load_addr, internal_splash_data,
			internal_splash_size);
	} אחרת {
		printf("Failed to get splash image from FIT\n");
		החזר -ENODATA;
	}

	אם (is_splash_external) {
		res = fit_image_get_data_size(fit_header, node_offset, &external_splash_size);
		אם (res < 0) {
			printf("Failed to get size of splash image (err=%d)\n", res);
			החזר res;
		}

		/* Read in the splash data */
		location->offset = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_load_addr, external_splash_size);
		אם (res < 0)
			החזר res;
	}

	החזר 0;
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
صحيح splash_source_load(بنية splash_location *locations, uint size)
{
	بنية splash_location *splash_location;
	حرف *env_splashimage_value;
	حرف *devpart;
	ulong bmp_load_addr;

	env_splashimage_value = env_get("splashimage");
	אם (env_splashimage_value == عدم)
		החזר -ENOENT;

	bmp_load_addr = hextoul(env_splashimage_value, 0);
	אם (bmp_load_addr == 0) {
		printf("Error: bad splashimage address specified\n");
		החזר -EFAULT;
	}

	splash_location = select_splash_location(locations, size);
	אם (!splash_location)
		החזר -EINVAL;

	devpart = env_get("splashdevpart");
	אם (devpart)
		splash_location->devpart = devpart;

	אם (splash_location->flags == SPLASH_STORAGE_RAW)
		החזר splash_load_raw(splash_location, bmp_load_addr);
	אחרת אם (splash_location->flags == SPLASH_STORAGE_FS)
		החזר splash_load_fs(splash_location, bmp_load_addr);
#ifdef CONFIG_FIT
	אחרת אם (splash_location->flags == SPLASH_STORAGE_FIT)
		החזר splash_load_fit(splash_location, bmp_load_addr);
#endif
	החזר -EINVAL;
}
