// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 */

#ଅନ୍ତର୍ଭାଵ <bmp_layout.h>
#ଅନ୍ତର୍ଭାଵ <command.h>
#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <fs.h>
#ଅନ୍ତର୍ଭାଵ <fdt_support.h>
#ଅନ୍ତର୍ଭାଵ <image.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <nand.h>
#ଅନ୍ତର୍ଭାଵ <sata.h>
#ଅନ୍ତର୍ଭାଵ <spi.h>
#ଅନ୍ତର୍ଭାଵ <spi_flash.h>
#ଅନ୍ତର୍ଭାଵ <splash.h>
#ଅନ୍ତର୍ଭାଵ <usb.h>
#ଅନ୍ତର୍ଭାଵ <virtio.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SPI_FLASH
ସ୍ଥିର ସଂରଚନା spi_flash *sf;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା splash_sf_read_raw(ulong bmp_load_addr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା offset, size_t read_size)
{
	ଯଦି (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_SPEED,
				     CONFIG_SF_DEFAULT_MODE);
		ଯଦି (!sf)
			ଫେରନ୍ତୁ -ENODEV;
	}

	ଫେରନ୍ତୁ spi_flash_read(sf, offset, read_size, (ଶୂନ୍ୟ *)(uintptr_t)bmp_load_addr);
}
#ଅନ୍ୟଥା
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା splash_sf_read_raw(ulong bmp_load_addr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା offset, size_t read_size)
{
	debug("%s: sf support not available\n", __func__);
	ଫେରନ୍ତୁ -ENOSYS;
}
#endif

#ifdef CONFIG_CMD_NAND
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା splash_nand_read_raw(ulong bmp_load_addr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା offset, size_t read_size)
{
	ସଂରଚନା mtd_info *mtd = get_nand_dev_by_index(nand_curr_device);
	ଫେରନ୍ତୁ nand_read_skip_bad(mtd, offset,
				  &read_size, ଖାଲି,
				  mtd->size,
				  (u_char *)bmp_load_addr);
}
#ଅନ୍ୟଥା
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା splash_nand_read_raw(ulong bmp_load_addr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା offset, size_t read_size)
{
	debug("%s: nand support not available\n", __func__);
	ଫେରନ୍ତୁ -ENOSYS;
}
#endif

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା splash_mmc_read_raw(ulong bmp_load_addr, ସଂରଚନା splash_location *location,
			       size_t read_size)
{
	ସଂରଚନା disk_partition partition;
	ସଂରଚନା blk_desc *desc;
	lbaint_t blkcnt;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret, n;

	ଯଦି (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		ଫେରନ୍ତୁ -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_ଅଙ୍କ("mmc", location->devpart, &desc,
						   &partition, 1);
	ଯଦି (ret < 0)
		ଫେରନ୍ତୁ ret;

	blkcnt = DIV_ROUND_UP(read_size, partition.blksz);
	n = blk_dread(desc, partition.start, blkcnt, (ଶୂନ୍ୟ *)(uintptr_t)bmp_load_addr);

	ଫେରନ୍ତୁ (n == blkcnt) ? 0 : -EIO;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା splash_storage_read_raw(ସଂରଚନା splash_location *location,
			       ulong bmp_load_addr, size_t read_size)
{
	u32 offset;

	ଯଦି (!location)
		ଫେରନ୍ତୁ -EINVAL;

	offset = location->offset;
	ବାଛ (location->storage) {
	ସ୍ଥିତି SPLASH_STORAGE_MMC:
		ଫେରନ୍ତୁ splash_mmc_read_raw(bmp_load_addr, location, read_size);
	ସ୍ଥିତି SPLASH_STORAGE_NAND:
		ଫେରନ୍ତୁ splash_nand_read_raw(bmp_load_addr, offset, read_size);
	ସ୍ଥିତି SPLASH_STORAGE_SF:
		ଫେରନ୍ତୁ splash_sf_read_raw(bmp_load_addr, offset, read_size);
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		printf("Unknown splash location\n");
	}

	ଫେରନ୍ତୁ -EINVAL;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା splash_load_raw(ସଂରଚନା splash_location *location, ulong bmp_load_addr)
{
	ସଂରଚନା bmp_header *bmp_hdr;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;
	size_t bmp_size, bmp_header_size = ଆକାର(ସଂରଚନା bmp_header);

	ଯଦି (bmp_load_addr + bmp_header_size >= gd->start_addr_sp)
		ଯାଅ splash_address_too_ଉପର;

	res = splash_storage_read_raw(location, bmp_load_addr, bmp_header_size);
	ଯଦି (res < 0)
		ଫେରନ୍ତୁ res;

	bmp_hdr = (ସଂରଚନା bmp_header *)(uintptr_t)bmp_load_addr;
	bmp_size = le32_to_cpu(bmp_hdr->file_size);

	ଯଦି (bmp_load_addr + bmp_size >= gd->start_addr_sp)
		ଯାଅ splash_address_too_ଉପର;

	ଫେରନ୍ତୁ splash_storage_read_raw(location, bmp_load_addr, bmp_size);

splash_address_too_ଉପର:
	printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	ଫେରନ୍ତୁ -EFAULT;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା splash_select_fs_dev(ସଂରଚନା splash_location *location)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;

	ବାଛ (location->storage) {
	ସ୍ଥିତି SPLASH_STORAGE_MMC:
		res = fs_set_blk_dev("mmc", location->devpart, FS_TYPE_ANY);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି SPLASH_STORAGE_USB:
		res = fs_set_blk_dev("usb", location->devpart, FS_TYPE_ANY);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି SPLASH_STORAGE_SATA:
		res = fs_set_blk_dev("sata", location->devpart, FS_TYPE_ANY);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି SPLASH_STORAGE_NAND:
		ଯଦି (location->ubivol != ଖାଲି)
			res = fs_set_blk_dev("ubi", ଖାଲି, FS_TYPE_UBIFS);
		ଅନ୍ୟଥା
			res = -ENODEV;
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		printf("Error: %s: unsupported location storage.\n", __func__);
		ଫେରନ୍ତୁ -ENODEV;
	}

	ଯଦି (res)
		printf("Error: %s: could not access storage.\n", __func__);

	ଫେରନ୍ତୁ res;
}

#ifdef CONFIG_USB_STORAGE
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା splash_init_usb(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;

	err = usb_init();
	ଯଦି (err)
		ଫେରନ୍ତୁ err;

#ifndef CONFIG_DM_USB
	err = usb_stor_scan(1) < 0 ? -ENODEV : 0;
#endif

	ଫେରନ୍ତୁ err;
}
#ଅନ୍ୟଥା
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା splash_init_usb(ଶୂନ୍ୟ)
{
	printf("Cannot load splash image: no USB support\n");
	ଫେରନ୍ତୁ -ENOSYS;
}
#endif

#ifdef CONFIG_SATA
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା splash_init_sata(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ sata_probe(0);
}
#ଅନ୍ୟଥା
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା splash_init_sata(ଶୂନ୍ୟ)
{
	printf("Cannot load splash image: no SATA support\n");
	ଫେରନ୍ତୁ -ENOSYS;
}
#endif

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା splash_init_virtio(ଶୂନ୍ୟ)
{
	ଯଦି (!IS_ENABLED(CONFIG_VIRTIO)) {
		printf("Cannot load splash image: no virtio support\n");
		ଫେରନ୍ତୁ -ENOSYS;
	} ଅନ୍ୟଥା {
		ଫେରନ୍ତୁ virtio_init();
	}
}

#ଯଦି defined(CONFIG_CMD_UBIFS) && !defined(CONFIG_XPL_BUILD)
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା splash_mount_ubifs(ସଂରଚନା splash_location *location)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;
	ଅକ୍ଷର cmd[32];

	sprintf(cmd, "ubi part %s", location->mtdpart);
	res = run_command(cmd, 0);
	ଯଦି (res)
		ଫେରନ୍ତୁ res;

	sprintf(cmd, "ubifsmount %s", location->ubivol);
	res = run_command(cmd, 0);

	ଫେରନ୍ତୁ res;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା splash_umount_ubifs(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ run_command("ubifsumount", 0);
}
#ଅନ୍ୟଥା
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା splash_mount_ubifs(ସଂରଚନା splash_location *location)
{
	printf("Cannot load splash image: no UBIFS support\n");
	ଫେରନ୍ତୁ -ENOSYS;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା splash_umount_ubifs(ଶୂନ୍ୟ)
{
	printf("Cannot unmount UBIFS: no UBIFS support\n");
	ଫେରନ୍ତୁ -ENOSYS;
}
#endif

#define SPLASH_SOURCE_DEFAULT_FILE_NAME		"splash.bmp"

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା splash_load_fs(ସଂରଚନା splash_location *location, ulong bmp_load_addr)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା res = 0;
	loff_t bmp_size;
	loff_t actread;
	ଅକ୍ଷର *splash_file;

	splash_file = env_get("splashfile");
	ଯଦି (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	ଯଦି (location->storage == SPLASH_STORAGE_USB)
		res = splash_init_usb();

	ଯଦି (location->storage == SPLASH_STORAGE_SATA)
		res = splash_init_sata();

	ଯଦି (location->storage == SPLASH_STORAGE_VIRTIO)
		res = splash_init_virtio();

	ଯଦି (location->ubivol != ଖାଲି)
		res = splash_mount_ubifs(location);

	ଯଦି (res)
		ଫେରନ୍ତୁ res;

	res = splash_select_fs_dev(location);
	ଯଦି (res)
		ଯାଅ out;

	res = fs_size(splash_file, &bmp_size);
	ଯଦି (res) {
		printf("Error: %s: cannot determine file size (%d)\n",
		       __func__, res);
		ଯାଅ out;
	}

	ଯଦି (bmp_load_addr + bmp_size >= gd->start_addr_sp) {
		printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		ଯାଅ out;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_load_addr, 0, 0, &actread);

out:
	ଯଦି (location->ubivol != ଖାଲି)
		splash_umount_ubifs();

	ଫେରନ୍ତୁ res;
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
ସ୍ଥିର ସଂରଚନା splash_location *select_splash_location(
			    ସଂରଚନା splash_location *locations, uint size)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ଅକ୍ଷର *env_splashsource;

	ଯଦି (!locations || size == 0)
		ଫେରନ୍ତୁ ଖାଲି;

	env_splashsource = env_get("splashsource");
	ଯଦି (env_splashsource == ଖାଲି)
		ଫେରନ୍ତୁ &locations[0];

	ପାଇଁ (i = 0; i < size; i++) {
		ଯଦି (!strcmp(locations[i].name, env_splashsource))
			ଫେରନ୍ତୁ &locations[i];
	}

	printf("splashsource env variable set to unsupported value\n");
	ଫେରନ୍ତୁ ଖାଲି;
}

#ifdef CONFIG_FIT
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା splash_load_fit(ସଂରଚନା splash_location *location, ulong bmp_load_addr)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣ୍ଠି_offset;
	ଅଚଳ ଅକ୍ଷର *splash_file;
	ଅଚଳ ଶୂନ୍ୟ *internal_splash_data;
	size_t internal_splash_size;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା external_splash_addr;
	u32 external_splash_size;
	bool is_splash_external = false;
	ସଂରଚନା legacy_img_hdr *img_header;
	ଅଚଳ u32 *fit_header;
	u32 fit_size;
	ଅଚଳ size_t header_size = ଆକାର(ସଂରଚନା legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_load_addr, header_size);
	ଯଦି (res < 0)
		ଫେରନ୍ତୁ res;

	img_header = (ସଂରଚନା legacy_img_hdr *)(uintptr_t)bmp_load_addr;
	ଯଦି (image_get_magic(img_header) != FDT_MAGIC) {
		printf("Could not find FDT magic\n");
		ଫେରନ୍ତୁ -EINVAL;
	}

	fit_size = fdt_totalsize(img_header);

	/* Read in entire FIT */
	fit_header = (ଅଚଳ u32 *)(bmp_load_addr + header_size);
	res = splash_storage_read_raw(location, (uintptr_t)fit_header, fit_size);
	ଯଦି (res < 0)
		ଫେରନ୍ତୁ res;

	res = fit_check_format(fit_header, IMAGE_SIZE_INVAL);
	ଯଦି (res) {
		debug("Could not find valid FIT image\n");
		ଫେରନ୍ତୁ res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	ଯଦି (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	ଗଣ୍ଠି_offset = fit_image_get_ଗଣ୍ଠି(fit_header, splash_file);
	ଯଦି (ଗଣ୍ଠି_offset < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		ଫେରନ୍ତୁ -ENOENT;
	}

	/* Extract the splash data from FIT */
	ଯଦି (!fit_image_get_data(fit_header, ଗଣ୍ଠି_offset, &internal_splash_data,
				&internal_splash_size)) {
		memmove((ଶୂନ୍ୟ *)(uintptr_t)bmp_load_addr, internal_splash_data,
			internal_splash_size);
	} ଅନ୍ୟଥା {
		printf("Failed to get splash image from FIT\n");
		ଫେରନ୍ତୁ -ENODATA;
	}

	ଯଦି (is_splash_external) {
		res = fit_image_get_data_size(fit_header, ଗଣ୍ଠି_offset, &external_splash_size);
		ଯଦି (res < 0) {
			printf("Failed to get size of splash image (err=%d)\n", res);
			ଫେରନ୍ତୁ res;
		}

		/* Read in the splash data */
		location->offset = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_load_addr, external_splash_size);
		ଯଦି (res < 0)
			ଫେରନ୍ତୁ res;
	}

	ଫେରନ୍ତୁ 0;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା splash_source_load(ସଂରଚନା splash_location *locations, uint size)
{
	ସଂରଚନା splash_location *splash_location;
	ଅକ୍ଷର *env_splashimage_ମୂଲ୍ୟ;
	ଅକ୍ଷର *devpart;
	ulong bmp_load_addr;

	env_splashimage_ମୂଲ୍ୟ = env_get("splashimage");
	ଯଦି (env_splashimage_ମୂଲ୍ୟ == ଖାଲି)
		ଫେରନ୍ତୁ -ENOENT;

	bmp_load_addr = hextoul(env_splashimage_ମୂଲ୍ୟ, 0);
	ଯଦି (bmp_load_addr == 0) {
		printf("Error: bad splashimage address specified\n");
		ଫେରନ୍ତୁ -EFAULT;
	}

	splash_location = select_splash_location(locations, size);
	ଯଦି (!splash_location)
		ଫେରନ୍ତୁ -EINVAL;

	devpart = env_get("splashdevpart");
	ଯଦି (devpart)
		splash_location->devpart = devpart;

	ଯଦି (splash_location->flags == SPLASH_STORAGE_RAW)
		ଫେରନ୍ତୁ splash_load_raw(splash_location, bmp_load_addr);
	ଅନ୍ୟଥା ଯଦି (splash_location->flags == SPLASH_STORAGE_FS)
		ଫେରନ୍ତୁ splash_load_fs(splash_location, bmp_load_addr);
#ifdef CONFIG_FIT
	ଅନ୍ୟଥା ଯଦି (splash_location->flags == SPLASH_STORAGE_FIT)
		ଫେରନ୍ତୁ splash_load_fit(splash_location, bmp_load_addr);
#endif
	ଫେରନ୍ତୁ -EINVAL;
}
