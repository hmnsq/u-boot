// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 */

#અન્તર્ભાવ <bmp_layout.h>
#અન્તર્ભાવ <command.h>
#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <fs.h>
#અન્તર્ભાવ <fdt_support.h>
#અન્તર્ભાવ <image.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <nand.h>
#અન્તર્ભાવ <sata.h>
#અન્તર્ભાવ <spi.h>
#અન્તર્ભાવ <spi_flash.h>
#અન્તર્ભાવ <splash.h>
#અન્તર્ભાવ <usb.h>
#અન્તર્ભાવ <virtio.h>
#અન્તર્ભાવ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SPI_FLASH
સ્થિર સંરચના spi_flash *sf;
સ્થિર પૂર્ણાંક splash_sf_read_raw(ulong bmp_load_addr, પૂર્ણાંક offset, size_t read_size)
{
	જો (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_SPEED,
				     CONFIG_SF_DEFAULT_MODE);
		જો (!sf)
			પાછા -ENODEV;
	}

	પાછા spi_flash_read(sf, offset, read_size, (શૂન્ય *)(uintptr_t)bmp_load_addr);
}
#નહીંતર
સ્થિર પૂર્ણાંક splash_sf_read_raw(ulong bmp_load_addr, પૂર્ણાંક offset, size_t read_size)
{
	debug("%s: sf support not available\n", __func__);
	પાછા -ENOSYS;
}
#endif

#ifdef CONFIG_CMD_NAND
સ્થિર પૂર્ણાંક splash_nand_read_raw(ulong bmp_load_addr, પૂર્ણાંક offset, size_t read_size)
{
	સંરચના mtd_info *mtd = get_nand_dev_by_index(nand_curr_device);
	પાછા nand_read_skip_bad(mtd, offset,
				  &read_size, ખાલી,
				  mtd->size,
				  (u_char *)bmp_load_addr);
}
#નહીંતર
સ્થિર પૂર્ણાંક splash_nand_read_raw(ulong bmp_load_addr, પૂર્ણાંક offset, size_t read_size)
{
	debug("%s: nand support not available\n", __func__);
	પાછા -ENOSYS;
}
#endif

સ્થિર પૂર્ણાંક splash_mmc_read_raw(ulong bmp_load_addr, સંરચના splash_location *location,
			       size_t read_size)
{
	સંરચના disk_partition partition;
	સંરચના blk_desc *desc;
	lbaint_t blkcnt;
	પૂર્ણાંક ret, n;

	જો (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		પાછા -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_અંક("mmc", location->devpart, &desc,
						   &partition, 1);
	જો (ret < 0)
		પાછા ret;

	blkcnt = DIV_ROUND_UP(read_size, partition.blksz);
	n = blk_dread(desc, partition.start, blkcnt, (શૂન્ય *)(uintptr_t)bmp_load_addr);

	પાછા (n == blkcnt) ? 0 : -EIO;
}

સ્થિર પૂર્ણાંક splash_storage_read_raw(સંરચના splash_location *location,
			       ulong bmp_load_addr, size_t read_size)
{
	u32 offset;

	જો (!location)
		પાછા -EINVAL;

	offset = location->offset;
	પસંદગી (location->storage) {
	સ્થિતિ SPLASH_STORAGE_MMC:
		પાછા splash_mmc_read_raw(bmp_load_addr, location, read_size);
	સ્થિતિ SPLASH_STORAGE_NAND:
		પાછા splash_nand_read_raw(bmp_load_addr, offset, read_size);
	સ્થિતિ SPLASH_STORAGE_SF:
		પાછા splash_sf_read_raw(bmp_load_addr, offset, read_size);
	મૂળભૂત:
		printf("Unknown splash location\n");
	}

	પાછા -EINVAL;
}

સ્થિર પૂર્ણાંક splash_load_raw(સંરચના splash_location *location, ulong bmp_load_addr)
{
	સંરચના bmp_header *bmp_hdr;
	પૂર્ણાંક res;
	size_t bmp_size, bmp_header_size = કદ(સંરચના bmp_header);

	જો (bmp_load_addr + bmp_header_size >= gd->start_addr_sp)
		જાઓ splash_address_too_ઉપલું;

	res = splash_storage_read_raw(location, bmp_load_addr, bmp_header_size);
	જો (res < 0)
		પાછા res;

	bmp_hdr = (સંરચના bmp_header *)(uintptr_t)bmp_load_addr;
	bmp_size = le32_to_cpu(bmp_hdr->file_size);

	જો (bmp_load_addr + bmp_size >= gd->start_addr_sp)
		જાઓ splash_address_too_ઉપલું;

	પાછા splash_storage_read_raw(location, bmp_load_addr, bmp_size);

splash_address_too_ઉપલું:
	printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	પાછા -EFAULT;
}

સ્થિર પૂર્ણાંક splash_select_fs_dev(સંરચના splash_location *location)
{
	પૂર્ણાંક res;

	પસંદગી (location->storage) {
	સ્થિતિ SPLASH_STORAGE_MMC:
		res = fs_set_blk_dev("mmc", location->devpart, FS_TYPE_ANY);
		તોડો;
	સ્થિતિ SPLASH_STORAGE_USB:
		res = fs_set_blk_dev("usb", location->devpart, FS_TYPE_ANY);
		તોડો;
	સ્થિતિ SPLASH_STORAGE_SATA:
		res = fs_set_blk_dev("sata", location->devpart, FS_TYPE_ANY);
		તોડો;
	સ્થિતિ SPLASH_STORAGE_NAND:
		જો (location->ubivol != ખાલી)
			res = fs_set_blk_dev("ubi", ખાલી, FS_TYPE_UBIFS);
		નહીંતર
			res = -ENODEV;
		તોડો;
	મૂળભૂત:
		printf("Error: %s: unsupported location storage.\n", __func__);
		પાછા -ENODEV;
	}

	જો (res)
		printf("Error: %s: could not access storage.\n", __func__);

	પાછા res;
}

#ifdef CONFIG_USB_STORAGE
સ્થિર પૂર્ણાંક splash_init_usb(શૂન્ય)
{
	પૂર્ણાંક err;

	err = usb_init();
	જો (err)
		પાછા err;

#ifndef CONFIG_DM_USB
	err = usb_stor_scan(1) < 0 ? -ENODEV : 0;
#endif

	પાછા err;
}
#નહીંતર
સ્થિર અંતઃરેખ પૂર્ણાંક splash_init_usb(શૂન્ય)
{
	printf("Cannot load splash image: no USB support\n");
	પાછા -ENOSYS;
}
#endif

#ifdef CONFIG_SATA
સ્થિર પૂર્ણાંક splash_init_sata(શૂન્ય)
{
	પાછા sata_probe(0);
}
#નહીંતર
સ્થિર અંતઃરેખ પૂર્ણાંક splash_init_sata(શૂન્ય)
{
	printf("Cannot load splash image: no SATA support\n");
	પાછા -ENOSYS;
}
#endif

સ્થિર પૂર્ણાંક splash_init_virtio(શૂન્ય)
{
	જો (!IS_ENABLED(CONFIG_VIRTIO)) {
		printf("Cannot load splash image: no virtio support\n");
		પાછા -ENOSYS;
	} નહીંતર {
		પાછા virtio_init();
	}
}

#જો defined(CONFIG_CMD_UBIFS) && !defined(CONFIG_XPL_BUILD)
સ્થિર પૂર્ણાંક splash_mount_ubifs(સંરચના splash_location *location)
{
	પૂર્ણાંક res;
	અક્ષર cmd[32];

	sprintf(cmd, "ubi part %s", location->mtdpart);
	res = run_command(cmd, 0);
	જો (res)
		પાછા res;

	sprintf(cmd, "ubifsmount %s", location->ubivol);
	res = run_command(cmd, 0);

	પાછા res;
}

સ્થિર અંતઃરેખ પૂર્ણાંક splash_umount_ubifs(શૂન્ય)
{
	પાછા run_command("ubifsumount", 0);
}
#નહીંતર
સ્થિર અંતઃરેખ પૂર્ણાંક splash_mount_ubifs(સંરચના splash_location *location)
{
	printf("Cannot load splash image: no UBIFS support\n");
	પાછા -ENOSYS;
}

સ્થિર અંતઃરેખ પૂર્ણાંક splash_umount_ubifs(શૂન્ય)
{
	printf("Cannot unmount UBIFS: no UBIFS support\n");
	પાછા -ENOSYS;
}
#endif

#define SPLASH_SOURCE_DEFAULT_FILE_NAME		"splash.bmp"

સ્થિર પૂર્ણાંક splash_load_fs(સંરચના splash_location *location, ulong bmp_load_addr)
{
	પૂર્ણાંક res = 0;
	loff_t bmp_size;
	loff_t actread;
	અક્ષર *splash_file;

	splash_file = env_get("splashfile");
	જો (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	જો (location->storage == SPLASH_STORAGE_USB)
		res = splash_init_usb();

	જો (location->storage == SPLASH_STORAGE_SATA)
		res = splash_init_sata();

	જો (location->storage == SPLASH_STORAGE_VIRTIO)
		res = splash_init_virtio();

	જો (location->ubivol != ખાલી)
		res = splash_mount_ubifs(location);

	જો (res)
		પાછા res;

	res = splash_select_fs_dev(location);
	જો (res)
		જાઓ out;

	res = fs_size(splash_file, &bmp_size);
	જો (res) {
		printf("Error: %s: cannot determine file size (%d)\n",
		       __func__, res);
		જાઓ out;
	}

	જો (bmp_load_addr + bmp_size >= gd->start_addr_sp) {
		printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		જાઓ out;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_load_addr, 0, 0, &actread);

out:
	જો (location->ubivol != ખાલી)
		splash_umount_ubifs();

	પાછા res;
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
સ્થિર સંરચના splash_location *select_splash_location(
			    સંરચના splash_location *locations, uint size)
{
	પૂર્ણાંક i;
	અક્ષર *env_splashsource;

	જો (!locations || size == 0)
		પાછા ખાલી;

	env_splashsource = env_get("splashsource");
	જો (env_splashsource == ખાલી)
		પાછા &locations[0];

	માટે (i = 0; i < size; i++) {
		જો (!strcmp(locations[i].name, env_splashsource))
			પાછા &locations[i];
	}

	printf("splashsource env variable set to unsupported value\n");
	પાછા ખાલી;
}

#ifdef CONFIG_FIT
સ્થિર પૂર્ણાંક splash_load_fit(સંરચના splash_location *location, ulong bmp_load_addr)
{
	પૂર્ણાંક res;
	પૂર્ણાંક ગાંઠ_offset;
	અચલ અક્ષર *splash_file;
	અચલ શૂન્ય *internal_splash_data;
	size_t internal_splash_size;
	પૂર્ણાંક external_splash_addr;
	u32 external_splash_size;
	bool is_splash_external = false;
	સંરચના legacy_img_hdr *img_header;
	અચલ u32 *fit_header;
	u32 fit_size;
	અચલ size_t header_size = કદ(સંરચના legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_load_addr, header_size);
	જો (res < 0)
		પાછા res;

	img_header = (સંરચના legacy_img_hdr *)(uintptr_t)bmp_load_addr;
	જો (image_get_magic(img_header) != FDT_MAGIC) {
		printf("Could not find FDT magic\n");
		પાછા -EINVAL;
	}

	fit_size = fdt_totalsize(img_header);

	/* Read in entire FIT */
	fit_header = (અચલ u32 *)(bmp_load_addr + header_size);
	res = splash_storage_read_raw(location, (uintptr_t)fit_header, fit_size);
	જો (res < 0)
		પાછા res;

	res = fit_check_format(fit_header, IMAGE_SIZE_INVAL);
	જો (res) {
		debug("Could not find valid FIT image\n");
		પાછા res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	જો (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	ગાંઠ_offset = fit_image_get_ગાંઠ(fit_header, splash_file);
	જો (ગાંઠ_offset < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		પાછા -ENOENT;
	}

	/* Extract the splash data from FIT */
	જો (!fit_image_get_data(fit_header, ગાંઠ_offset, &internal_splash_data,
				&internal_splash_size)) {
		memmove((શૂન્ય *)(uintptr_t)bmp_load_addr, internal_splash_data,
			internal_splash_size);
	} નહીંતર {
		printf("Failed to get splash image from FIT\n");
		પાછા -ENODATA;
	}

	જો (is_splash_external) {
		res = fit_image_get_data_size(fit_header, ગાંઠ_offset, &external_splash_size);
		જો (res < 0) {
			printf("Failed to get size of splash image (err=%d)\n", res);
			પાછા res;
		}

		/* Read in the splash data */
		location->offset = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_load_addr, external_splash_size);
		જો (res < 0)
			પાછા res;
	}

	પાછા 0;
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
પૂર્ણાંક splash_source_load(સંરચના splash_location *locations, uint size)
{
	સંરચના splash_location *splash_location;
	અક્ષર *env_splashimage_કિંમત;
	અક્ષર *devpart;
	ulong bmp_load_addr;

	env_splashimage_કિંમત = env_get("splashimage");
	જો (env_splashimage_કિંમત == ખાલી)
		પાછા -ENOENT;

	bmp_load_addr = hextoul(env_splashimage_કિંમત, 0);
	જો (bmp_load_addr == 0) {
		printf("Error: bad splashimage address specified\n");
		પાછા -EFAULT;
	}

	splash_location = select_splash_location(locations, size);
	જો (!splash_location)
		પાછા -EINVAL;

	devpart = env_get("splashdevpart");
	જો (devpart)
		splash_location->devpart = devpart;

	જો (splash_location->flags == SPLASH_STORAGE_RAW)
		પાછા splash_load_raw(splash_location, bmp_load_addr);
	નહીંતર જો (splash_location->flags == SPLASH_STORAGE_FS)
		પાછા splash_load_fs(splash_location, bmp_load_addr);
#ifdef CONFIG_FIT
	નહીંતર જો (splash_location->flags == SPLASH_STORAGE_FIT)
		પાછા splash_load_fit(splash_location, bmp_load_addr);
#endif
	પાછા -EINVAL;
}
