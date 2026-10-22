// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 */

#অন্তর্ভাব <bmp_layout.h>
#অন্তর্ভাব <command.h>
#অন্তর্ভাব <env.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <fs.h>
#অন্তর্ভাব <fdt_support.h>
#অন্তর্ভাব <image.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <nand.h>
#অন্তর্ভাব <sata.h>
#অন্তর্ভাব <spi.h>
#অন্তর্ভাব <spi_flash.h>
#অন্তর্ভাব <splash.h>
#অন্তর্ভাব <usb.h>
#অন্তর্ভাব <virtio.h>
#অন্তর্ভাব <asm/global_data.h>

DECLARE_GLOBAL_তথ্য_PTR;

#ifdef CONFIG_SPI_FLASH
স্থির কাঠামো spi_flash *sf;
স্থির পূর্ণসংখ্যা splash_sf_read_raw(ulong bmp_load_addr, পূর্ণসংখ্যা offset, size_t read_আকার)
{
	যদি (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_SPEED,
				     CONFIG_SF_DEFAULT_MODE);
		যদি (!sf)
			ফেরত -ENODEV;
	}

	ফেরত spi_flash_read(sf, offset, read_আকার, (শূন্য *)(uintptr_t)bmp_load_addr);
}
#অন্যথা
স্থির পূর্ণসংখ্যা splash_sf_read_raw(ulong bmp_load_addr, পূর্ণসংখ্যা offset, size_t read_আকার)
{
	debug("%s: sf support not available\n", __func__);
	ফেরত -ENOSYS;
}
#endif

#ifdef CONFIG_CMD_NAND
স্থির পূর্ণসংখ্যা splash_nand_read_raw(ulong bmp_load_addr, পূর্ণসংখ্যা offset, size_t read_আকার)
{
	কাঠামো mtd_info *mtd = get_nand_dev_by_সূচক(nand_curr_device);
	ফেরত nand_read_skip_bad(mtd, offset,
				  &read_আকার, ফাঁকা,
				  mtd->আকার_2,
				  (u_char *)bmp_load_addr);
}
#অন্যথা
স্থির পূর্ণসংখ্যা splash_nand_read_raw(ulong bmp_load_addr, পূর্ণসংখ্যা offset, size_t read_আকার)
{
	debug("%s: nand support not available\n", __func__);
	ফেরত -ENOSYS;
}
#endif

স্থির পূর্ণসংখ্যা splash_mmc_read_raw(ulong bmp_load_addr, কাঠামো splash_location *location,
			       size_t read_আকার)
{
	কাঠামো disk_partition partition;
	কাঠামো blk_desc *desc;
	lbaint_t blkcnt;
	পূর্ণসংখ্যা ret, n;

	যদি (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		ফেরত -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_অঙ্ক("mmc", location->devpart, &desc,
						   &partition, 1);
	যদি (ret < 0)
		ফেরত ret;

	blkcnt = DIV_ROUND_UP(read_আকার, partition.blksz);
	n = blk_dread(desc, partition.start, blkcnt, (শূন্য *)(uintptr_t)bmp_load_addr);

	ফেরত (n == blkcnt) ? 0 : -EIO;
}

স্থির পূর্ণসংখ্যা splash_storage_read_raw(কাঠামো splash_location *location,
			       ulong bmp_load_addr, size_t read_আকার)
{
	u32 offset;

	যদি (!location)
		ফেরত -EINVAL;

	offset = location->offset;
	নির্বাচন (location->storage) {
	ক্ষেত্র SPLASH_STORAGE_MMC:
		ফেরত splash_mmc_read_raw(bmp_load_addr, location, read_আকার);
	ক্ষেত্র SPLASH_STORAGE_NAND:
		ফেরত splash_nand_read_raw(bmp_load_addr, offset, read_আকার);
	ক্ষেত্র SPLASH_STORAGE_SF:
		ফেরত splash_sf_read_raw(bmp_load_addr, offset, read_আকার);
	পূর্বনির্ধারিত:
		printf("Unknown splash location\n");
	}

	ফেরত -EINVAL;
}

স্থির পূর্ণসংখ্যা splash_load_raw(কাঠামো splash_location *location, ulong bmp_load_addr)
{
	কাঠামো bmp_header *bmp_hdr;
	পূর্ণসংখ্যা res;
	size_t bmp_আকার, bmp_header_আকার = আকার(কাঠামো bmp_header);

	যদি (bmp_load_addr + bmp_header_আকার >= gd->start_addr_sp)
		যাও splash_address_too_উপরের;

	res = splash_storage_read_raw(location, bmp_load_addr, bmp_header_আকার);
	যদি (res < 0)
		ফেরত res;

	bmp_hdr = (কাঠামো bmp_header *)(uintptr_t)bmp_load_addr;
	bmp_আকার = le32_to_cpu(bmp_hdr->file_আকার);

	যদি (bmp_load_addr + bmp_আকার >= gd->start_addr_sp)
		যাও splash_address_too_উপরের;

	ফেরত splash_storage_read_raw(location, bmp_load_addr, bmp_আকার);

splash_address_too_উপরের:
	printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	ফেরত -EFAULT;
}

স্থির পূর্ণসংখ্যা splash_select_fs_dev(কাঠামো splash_location *location)
{
	পূর্ণসংখ্যা res;

	নির্বাচন (location->storage) {
	ক্ষেত্র SPLASH_STORAGE_MMC:
		res = fs_set_blk_dev("mmc", location->devpart, FS_TYPE_ANY);
		বিরতি;
	ক্ষেত্র SPLASH_STORAGE_USB:
		res = fs_set_blk_dev("usb", location->devpart, FS_TYPE_ANY);
		বিরতি;
	ক্ষেত্র SPLASH_STORAGE_SATA:
		res = fs_set_blk_dev("sata", location->devpart, FS_TYPE_ANY);
		বিরতি;
	ক্ষেত্র SPLASH_STORAGE_NAND:
		যদি (location->ubivol != ফাঁকা)
			res = fs_set_blk_dev("ubi", ফাঁকা, FS_TYPE_UBIFS);
		অন্যথা
			res = -ENODEV;
		বিরতি;
	পূর্বনির্ধারিত:
		printf("Error: %s: unsupported location storage.\n", __func__);
		ফেরত -ENODEV;
	}

	যদি (res)
		printf("Error: %s: could not access storage.\n", __func__);

	ফেরত res;
}

#ifdef CONFIG_USB_STORAGE
স্থির পূর্ণসংখ্যা splash_init_usb(শূন্য)
{
	পূর্ণসংখ্যা err;

	err = usb_init();
	যদি (err)
		ফেরত err;

#ifndef CONFIG_DM_USB
	err = usb_stor_scan(1) < 0 ? -ENODEV : 0;
#endif

	ফেরত err;
}
#অন্যথা
স্থির অন্তঃরেখ পূর্ণসংখ্যা splash_init_usb(শূন্য)
{
	printf("Cannot load splash image: no USB support\n");
	ফেরত -ENOSYS;
}
#endif

#ifdef CONFIG_SATA
স্থির পূর্ণসংখ্যা splash_init_sata(শূন্য)
{
	ফেরত sata_probe(0);
}
#অন্যথা
স্থির অন্তঃরেখ পূর্ণসংখ্যা splash_init_sata(শূন্য)
{
	printf("Cannot load splash image: no SATA support\n");
	ফেরত -ENOSYS;
}
#endif

স্থির পূর্ণসংখ্যা splash_init_virtio(শূন্য)
{
	যদি (!IS_ENABLED(CONFIG_VIRTIO)) {
		printf("Cannot load splash image: no virtio support\n");
		ফেরত -ENOSYS;
	} অন্যথা {
		ফেরত virtio_init();
	}
}

#যদি defined(CONFIG_CMD_UBIFS) && !defined(CONFIG_XPL_BUILD)
স্থির পূর্ণসংখ্যা splash_mount_ubifs(কাঠামো splash_location *location)
{
	পূর্ণসংখ্যা res;
	অক্ষর cmd[32];

	sprintf(cmd, "ubi part %s", location->mtdpart);
	res = run_command(cmd, 0);
	যদি (res)
		ফেরত res;

	sprintf(cmd, "ubifsmount %s", location->ubivol);
	res = run_command(cmd, 0);

	ফেরত res;
}

স্থির অন্তঃরেখ পূর্ণসংখ্যা splash_umount_ubifs(শূন্য)
{
	ফেরত run_command("ubifsumount", 0);
}
#অন্যথা
স্থির অন্তঃরেখ পূর্ণসংখ্যা splash_mount_ubifs(কাঠামো splash_location *location)
{
	printf("Cannot load splash image: no UBIFS support\n");
	ফেরত -ENOSYS;
}

স্থির অন্তঃরেখ পূর্ণসংখ্যা splash_umount_ubifs(শূন্য)
{
	printf("Cannot unmount UBIFS: no UBIFS support\n");
	ফেরত -ENOSYS;
}
#endif

#define SPLASH_SOURCE_DEFAULT_FILE_NAME		"splash.bmp"

স্থির পূর্ণসংখ্যা splash_load_fs(কাঠামো splash_location *location, ulong bmp_load_addr)
{
	পূর্ণসংখ্যা res = 0;
	loff_t bmp_আকার;
	loff_t actread;
	অক্ষর *splash_file;

	splash_file = env_get("splashfile");
	যদি (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	যদি (location->storage == SPLASH_STORAGE_USB)
		res = splash_init_usb();

	যদি (location->storage == SPLASH_STORAGE_SATA)
		res = splash_init_sata();

	যদি (location->storage == SPLASH_STORAGE_VIRTIO)
		res = splash_init_virtio();

	যদি (location->ubivol != ফাঁকা)
		res = splash_mount_ubifs(location);

	যদি (res)
		ফেরত res;

	res = splash_select_fs_dev(location);
	যদি (res)
		যাও out;

	res = fs_আকার(splash_file, &bmp_আকার);
	যদি (res) {
		printf("Error: %s: cannot determine file size (%d)\n",
		       __func__, res);
		যাও out;
	}

	যদি (bmp_load_addr + bmp_আকার >= gd->start_addr_sp) {
		printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		যাও out;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_load_addr, 0, 0, &actread);

out:
	যদি (location->ubivol != ফাঁকা)
		splash_umount_ubifs();

	ফেরত res;
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
স্থির কাঠামো splash_location *select_splash_location(
			    কাঠামো splash_location *locations, uint আকার_2)
{
	পূর্ণসংখ্যা i;
	অক্ষর *env_splashsource;

	যদি (!locations || আকার_2 == 0)
		ফেরত ফাঁকা;

	env_splashsource = env_get("splashsource");
	যদি (env_splashsource == ফাঁকা)
		ফেরত &locations[0];

	জন্য (i = 0; i < আকার_2; i++) {
		যদি (!strcmp(locations[i].name, env_splashsource))
			ফেরত &locations[i];
	}

	printf("splashsource env variable set to unsupported value\n");
	ফেরত ফাঁকা;
}

#ifdef CONFIG_FIT
স্থির পূর্ণসংখ্যা splash_load_fit(কাঠামো splash_location *location, ulong bmp_load_addr)
{
	পূর্ণসংখ্যা res;
	পূর্ণসংখ্যা গ্রন্থি_offset;
	ধ্রুবক অক্ষর *splash_file;
	ধ্রুবক শূন্য *internal_splash_তথ্য;
	size_t internal_splash_আকার;
	পূর্ণসংখ্যা external_splash_addr;
	u32 external_splash_আকার;
	bool is_splash_external = false;
	কাঠামো legacy_img_hdr *img_header;
	ধ্রুবক u32 *fit_header;
	u32 fit_আকার;
	ধ্রুবক size_t header_আকার = আকার(কাঠামো legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_load_addr, header_আকার);
	যদি (res < 0)
		ফেরত res;

	img_header = (কাঠামো legacy_img_hdr *)(uintptr_t)bmp_load_addr;
	যদি (image_get_magic(img_header) != FDT_MAGIC) {
		printf("Could not find FDT magic\n");
		ফেরত -EINVAL;
	}

	fit_আকার = fdt_totalsize(img_header);

	/* Read in entire FIT */
	fit_header = (ধ্রুবক u32 *)(bmp_load_addr + header_আকার);
	res = splash_storage_read_raw(location, (uintptr_t)fit_header, fit_আকার);
	যদি (res < 0)
		ফেরত res;

	res = fit_check_format(fit_header, IMAGE_আকার_INVAL);
	যদি (res) {
		debug("Could not find valid FIT image\n");
		ফেরত res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	যদি (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	গ্রন্থি_offset = fit_image_get_গ্রন্থি(fit_header, splash_file);
	যদি (গ্রন্থি_offset < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		ফেরত -ENOENT;
	}

	/* Extract the splash data from FIT */
	যদি (!fit_image_get_তথ্য(fit_header, গ্রন্থি_offset, &internal_splash_তথ্য,
				&internal_splash_আকার)) {
		memmove((শূন্য *)(uintptr_t)bmp_load_addr, internal_splash_তথ্য,
			internal_splash_আকার);
	} অন্যথা {
		printf("Failed to get splash image from FIT\n");
		ফেরত -ENODATA;
	}

	যদি (is_splash_external) {
		res = fit_image_get_তথ্য_আকার(fit_header, গ্রন্থি_offset, &external_splash_আকার);
		যদি (res < 0) {
			printf("Failed to get size of splash image (err=%d)\n", res);
			ফেরত res;
		}

		/* Read in the splash data */
		location->offset = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_load_addr, external_splash_আকার);
		যদি (res < 0)
			ফেরত res;
	}

	ফেরত 0;
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
পূর্ণসংখ্যা splash_source_load(কাঠামো splash_location *locations, uint আকার_2)
{
	কাঠামো splash_location *splash_location;
	অক্ষর *env_splashimage_মান;
	অক্ষর *devpart;
	ulong bmp_load_addr;

	env_splashimage_মান = env_get("splashimage");
	যদি (env_splashimage_মান == ফাঁকা)
		ফেরত -ENOENT;

	bmp_load_addr = hextoul(env_splashimage_মান, 0);
	যদি (bmp_load_addr == 0) {
		printf("Error: bad splashimage address specified\n");
		ফেরত -EFAULT;
	}

	splash_location = select_splash_location(locations, আকার_2);
	যদি (!splash_location)
		ফেরত -EINVAL;

	devpart = env_get("splashdevpart");
	যদি (devpart)
		splash_location->devpart = devpart;

	যদি (splash_location->flags == SPLASH_STORAGE_RAW)
		ফেরত splash_load_raw(splash_location, bmp_load_addr);
	অন্যথা যদি (splash_location->flags == SPLASH_STORAGE_FS)
		ফেরত splash_load_fs(splash_location, bmp_load_addr);
#ifdef CONFIG_FIT
	অন্যথা যদি (splash_location->flags == SPLASH_STORAGE_FIT)
		ফেরত splash_load_fit(splash_location, bmp_load_addr);
#endif
	ফেরত -EINVAL;
}
