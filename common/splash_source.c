// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 */

#అన్తర్భావ <bmp_layout.h>
#అన్తర్భావ <command.h>
#అన్తర్భావ <env.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <fs.h>
#అన్తర్భావ <fdt_support.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <nand.h>
#అన్తర్భావ <sata.h>
#అన్తర్భావ <spi.h>
#అన్తర్భావ <spi_flash.h>
#అన్తర్భావ <splash.h>
#అన్తర్భావ <usb.h>
#అన్తర్భావ <virtio.h>
#అన్తర్భావ <asm/global_data.h>

DECLARE_GLOBAL_దత్తాంశం_PTR;

#నిర్వచితమైతే CONFIG_SPI_FLASH
స్థిర నిర్మాణం spi_flash *sf;
స్థిర పూర్ణాంకం splash_sf_read_raw(ulong bmp_ఎక్కించు_addr, పూర్ణాంకం స్థానభ్రంశం, size_t read_కొలత)
{
	ఒకవేళ (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_వేగం,
				     CONFIG_SF_DEFAULT_బహుళకం);
		ఒకవేళ (!sf)
			తిరిగి -ENODEV;
	}

	తిరిగి spi_flash_read(sf, స్థానభ్రంశం, read_కొలత, (శూన్య *)(uintptr_t)bmp_ఎక్కించు_addr);
}
#లేకపోతే
స్థిర పూర్ణాంకం splash_sf_read_raw(ulong bmp_ఎక్కించు_addr, పూర్ణాంకం స్థానభ్రంశం, size_t read_కొలత)
{
	debug("%s: sf support not available\n", __func__);
	తిరిగి -ENOSYS;
}
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_CMD_NAND
స్థిర పూర్ణాంకం splash_nand_read_raw(ulong bmp_ఎక్కించు_addr, పూర్ణాంకం స్థానభ్రంశం, size_t read_కొలత)
{
	నిర్మాణం mtd_info *mtd = get_nand_dev_by_సూచీ(nand_curr_device);
	తిరిగి nand_read_skip_bad(mtd, స్థానభ్రంశం,
				  &read_కొలత, ఖాళీ,
				  mtd->కొలత,
				  (u_char *)bmp_ఎక్కించు_addr);
}
#లేకపోతే
స్థిర పూర్ణాంకం splash_nand_read_raw(ulong bmp_ఎక్కించు_addr, పూర్ణాంకం స్థానభ్రంశం, size_t read_కొలత)
{
	debug("%s: nand support not available\n", __func__);
	తిరిగి -ENOSYS;
}
#అయితే_ముగింపు

స్థిర పూర్ణాంకం splash_mmc_read_raw(ulong bmp_ఎక్కించు_addr, నిర్మాణం splash_location *location,
			       size_t read_కొలత)
{
	నిర్మాణం disk_విభజన విభజన;
	నిర్మాణం blk_desc *desc;
	lbaint_t blkcnt;
	పూర్ణాంకం ret, n;

	ఒకవేళ (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		తిరిగి -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_అంకె("mmc", location->devpart, &desc,
						   &విభజన, 1);
	ఒకవేళ (ret < 0)
		తిరిగి ret;

	blkcnt = DIV_విడత_పైకి(read_కొలత, విభజన.blksz);
	n = blk_dread(desc, విభజన.ప్రారంభం, blkcnt, (శూన్య *)(uintptr_t)bmp_ఎక్కించు_addr);

	తిరిగి (n == blkcnt) ? 0 : -EIO;
}

స్థిర పూర్ణాంకం splash_storage_read_raw(నిర్మాణం splash_location *location,
			       ulong bmp_ఎక్కించు_addr, size_t read_కొలత)
{
	u32 స్థానభ్రంశం;

	ఒకవేళ (!location)
		తిరిగి -EINVAL;

	స్థానభ్రంశం = location->స్థానభ్రంశం;
	ఎంపిక (location->storage) {
	స్థితి SPLASH_STORAGE_MMC:
		తిరిగి splash_mmc_read_raw(bmp_ఎక్కించు_addr, location, read_కొలత);
	స్థితి SPLASH_STORAGE_NAND:
		తిరిగి splash_nand_read_raw(bmp_ఎక్కించు_addr, స్థానభ్రంశం, read_కొలత);
	స్థితి SPLASH_STORAGE_SF:
		తిరిగి splash_sf_read_raw(bmp_ఎక్కించు_addr, స్థానభ్రంశం, read_కొలత);
	అప్రమేయ:
		రాయి("Unknown splash location\n");
	}

	తిరిగి -EINVAL;
}

స్థిర పూర్ణాంకం splash_ఎక్కించు_raw(నిర్మాణం splash_location *location, ulong bmp_ఎక్కించు_addr)
{
	నిర్మాణం bmp_శీర్షిక *bmp_hdr;
	పూర్ణాంకం res;
	size_t bmp_కొలత, bmp_శీర్షిక_కొలత = పరిమాణం(నిర్మాణం bmp_శీర్షిక);

	ఒకవేళ (bmp_ఎక్కించు_addr + bmp_శీర్షిక_కొలత >= gd->ప్రారంభం_addr_sp)
		వెళ్ళు splash_చిరునామా_too_ఎగువ;

	res = splash_storage_read_raw(location, bmp_ఎక్కించు_addr, bmp_శీర్షిక_కొలత);
	ఒకవేళ (res < 0)
		తిరిగి res;

	bmp_hdr = (నిర్మాణం bmp_శీర్షిక *)(uintptr_t)bmp_ఎక్కించు_addr;
	bmp_కొలత = le32_గమ్యం_cpu(bmp_hdr->file_కొలత);

	ఒకవేళ (bmp_ఎక్కించు_addr + bmp_కొలత >= gd->ప్రారంభం_addr_sp)
		వెళ్ళు splash_చిరునామా_too_ఎగువ;

	తిరిగి splash_storage_read_raw(location, bmp_ఎక్కించు_addr, bmp_కొలత);

splash_చిరునామా_too_ఎగువ:
	రాయి("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	తిరిగి -EFAULT;
}

స్థిర పూర్ణాంకం splash_select_fs_dev(నిర్మాణం splash_location *location)
{
	పూర్ణాంకం res;

	ఎంపిక (location->storage) {
	స్థితి SPLASH_STORAGE_MMC:
		res = fs_సమితి_blk_dev("mmc", location->devpart, FS_TYPE_ANY);
		ఆపు;
	స్థితి SPLASH_STORAGE_USB:
		res = fs_సమితి_blk_dev("usb", location->devpart, FS_TYPE_ANY);
		ఆపు;
	స్థితి SPLASH_STORAGE_SATA:
		res = fs_సమితి_blk_dev("sata", location->devpart, FS_TYPE_ANY);
		ఆపు;
	స్థితి SPLASH_STORAGE_NAND:
		ఒకవేళ (location->ubivol != ఖాళీ)
			res = fs_సమితి_blk_dev("ubi", ఖాళీ, FS_TYPE_UBIFS);
		లేకపోతే
			res = -ENODEV;
		ఆపు;
	అప్రమేయ:
		రాయి("Error: %s: unsupported location storage.\n", __func__);
		తిరిగి -ENODEV;
	}

	ఒకవేళ (res)
		రాయి("Error: %s: could not access storage.\n", __func__);

	తిరిగి res;
}

#నిర్వచితమైతే CONFIG_USB_STORAGE
స్థిర పూర్ణాంకం splash_init_usb(శూన్య)
{
	పూర్ణాంకం err;

	err = usb_init();
	ఒకవేళ (err)
		తిరిగి err;

#నిర్వచితం_కాకపోతే CONFIG_DM_USB
	err = usb_stor_పరిశీలన(1) < 0 ? -ENODEV : 0;
#అయితే_ముగింపు

	తిరిగి err;
}
#లేకపోతే
స్థిర అంతఃవరుస పూర్ణాంకం splash_init_usb(శూన్య)
{
	రాయి("Cannot load splash image: no USB support\n");
	తిరిగి -ENOSYS;
}
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_SATA
స్థిర పూర్ణాంకం splash_init_sata(శూన్య)
{
	తిరిగి sata_probe(0);
}
#లేకపోతే
స్థిర అంతఃవరుస పూర్ణాంకం splash_init_sata(శూన్య)
{
	రాయి("Cannot load splash image: no SATA support\n");
	తిరిగి -ENOSYS;
}
#అయితే_ముగింపు

స్థిర పూర్ణాంకం splash_init_virtio(శూన్య)
{
	ఒకవేళ (!IS_ENABLED(CONFIG_VIRTIO)) {
		రాయి("Cannot load splash image: no virtio support\n");
		తిరిగి -ENOSYS;
	} లేకపోతే {
		తిరిగి virtio_init();
	}
}

#ఒకవేళ defined(CONFIG_CMD_UBIFS) && !defined(CONFIG_XPL_BUILD)
స్థిర పూర్ణాంకం splash_mount_ubifs(నిర్మాణం splash_location *location)
{
	పూర్ణాంకం res;
	అక్షరం cmd[32];

	sprintf(cmd, "ubi part %s", location->mtdpart);
	res = నిరంతరం_command(cmd, 0);
	ఒకవేళ (res)
		తిరిగి res;

	sprintf(cmd, "ubifsmount %s", location->ubivol);
	res = నిరంతరం_command(cmd, 0);

	తిరిగి res;
}

స్థిర అంతఃవరుస పూర్ణాంకం splash_umount_ubifs(శూన్య)
{
	తిరిగి నిరంతరం_command("ubifsumount", 0);
}
#లేకపోతే
స్థిర అంతఃవరుస పూర్ణాంకం splash_mount_ubifs(నిర్మాణం splash_location *location)
{
	రాయి("Cannot load splash image: no UBIFS support\n");
	తిరిగి -ENOSYS;
}

స్థిర అంతఃవరుస పూర్ణాంకం splash_umount_ubifs(శూన్య)
{
	రాయి("Cannot unmount UBIFS: no UBIFS support\n");
	తిరిగి -ENOSYS;
}
#అయితే_ముగింపు

#నిర్వచించు SPLASH_ఆధారం_DEFAULT_FILE_NAME		"splash.bmp"

స్థిర పూర్ణాంకం splash_ఎక్కించు_fs(నిర్మాణం splash_location *location, ulong bmp_ఎక్కించు_addr)
{
	పూర్ణాంకం res = 0;
	loff_t bmp_కొలత;
	loff_t actread;
	అక్షరం *splash_file;

	splash_file = env_get("splashfile");
	ఒకవేళ (!splash_file)
		splash_file = SPLASH_ఆధారం_DEFAULT_FILE_NAME;

	ఒకవేళ (location->storage == SPLASH_STORAGE_USB)
		res = splash_init_usb();

	ఒకవేళ (location->storage == SPLASH_STORAGE_SATA)
		res = splash_init_sata();

	ఒకవేళ (location->storage == SPLASH_STORAGE_VIRTIO)
		res = splash_init_virtio();

	ఒకవేళ (location->ubivol != ఖాళీ)
		res = splash_mount_ubifs(location);

	ఒకవేళ (res)
		తిరిగి res;

	res = splash_select_fs_dev(location);
	ఒకవేళ (res)
		వెళ్ళు బహిర్గతం;

	res = fs_కొలత(splash_file, &bmp_కొలత);
	ఒకవేళ (res) {
		రాయి("Error: %s: cannot determine file size (%d)\n",
		       __func__, res);
		వెళ్ళు బహిర్గతం;
	}

	ఒకవేళ (bmp_ఎక్కించు_addr + bmp_కొలత >= gd->ప్రారంభం_addr_sp) {
		రాయి("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		వెళ్ళు బహిర్గతం;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_ఎక్కించు_addr, 0, 0, &actread);

బహిర్గతం:
	ఒకవేళ (location->ubivol != ఖాళీ)
		splash_umount_ubifs();

	తిరిగి res;
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
స్థిర నిర్మాణం splash_location *select_splash_location(
			    నిర్మాణం splash_location *locations, uint కొలత)
{
	పూర్ణాంకం i;
	అక్షరం *env_splashsource;

	ఒకవేళ (!locations || కొలత == 0)
		తిరిగి ఖాళీ;

	env_splashsource = env_get("splashsource");
	ఒకవేళ (env_splashsource == ఖాళీ)
		తిరిగి &locations[0];

	కొరకు (i = 0; i < కొలత; i++) {
		ఒకవేళ (!strcmp(locations[i].name, env_splashsource))
			తిరిగి &locations[i];
	}

	రాయి("splashsource env variable set to unsupported value\n");
	తిరిగి ఖాళీ;
}

#నిర్వచితమైతే CONFIG_FIT
స్థిర పూర్ణాంకం splash_ఎక్కించు_fit(నిర్మాణం splash_location *location, ulong bmp_ఎక్కించు_addr)
{
	పూర్ణాంకం res;
	పూర్ణాంకం కణుపు_స్థానభ్రంశం;
	స్థిరరాశి అక్షరం *splash_file;
	స్థిరరాశి శూన్య *అంతర్గత_splash_దత్తాంశం;
	size_t అంతర్గత_splash_కొలత;
	పూర్ణాంకం external_splash_addr;
	u32 external_splash_కొలత;
	bool is_splash_external = false;
	నిర్మాణం legacy_img_hdr *img_శీర్షిక;
	స్థిరరాశి u32 *fit_శీర్షిక;
	u32 fit_కొలత;
	స్థిరరాశి size_t శీర్షిక_కొలత = పరిమాణం(నిర్మాణం legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_ఎక్కించు_addr, శీర్షిక_కొలత);
	ఒకవేళ (res < 0)
		తిరిగి res;

	img_శీర్షిక = (నిర్మాణం legacy_img_hdr *)(uintptr_t)bmp_ఎక్కించు_addr;
	ఒకవేళ (image_get_magic(img_శీర్షిక) != FDT_MAGIC) {
		రాయి("Could not find FDT magic\n");
		తిరిగి -EINVAL;
	}

	fit_కొలత = fdt_totalsize(img_శీర్షిక);

	/* Read in entire FIT */
	fit_శీర్షిక = (స్థిరరాశి u32 *)(bmp_ఎక్కించు_addr + శీర్షిక_కొలత);
	res = splash_storage_read_raw(location, (uintptr_t)fit_శీర్షిక, fit_కొలత);
	ఒకవేళ (res < 0)
		తిరిగి res;

	res = fit_check_format(fit_శీర్షిక, IMAGE_కొలత_INVAL);
	ఒకవేళ (res) {
		debug("Could not find valid FIT image\n");
		తిరిగి res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	ఒకవేళ (!splash_file)
		splash_file = SPLASH_ఆధారం_DEFAULT_FILE_NAME;

	కణుపు_స్థానభ్రంశం = fit_image_get_కణుపు(fit_శీర్షిక, splash_file);
	ఒకవేళ (కణుపు_స్థానభ్రంశం < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		తిరిగి -ENOENT;
	}

	/* Extract the splash data from FIT */
	ఒకవేళ (!fit_image_get_దత్తాంశం(fit_శీర్షిక, కణుపు_స్థానభ్రంశం, &అంతర్గత_splash_దత్తాంశం,
				&అంతర్గత_splash_కొలత)) {
		memmove((శూన్య *)(uintptr_t)bmp_ఎక్కించు_addr, అంతర్గత_splash_దత్తాంశం,
			అంతర్గత_splash_కొలత);
	} లేకపోతే {
		రాయి("Failed to get splash image from FIT\n");
		తిరిగి -ENODATA;
	}

	ఒకవేళ (is_splash_external) {
		res = fit_image_get_దత్తాంశం_కొలత(fit_శీర్షిక, కణుపు_స్థానభ్రంశం, &external_splash_కొలత);
		ఒకవేళ (res < 0) {
			రాయి("Failed to get size of splash image (err=%d)\n", res);
			తిరిగి res;
		}

		/* Read in the splash data */
		location->స్థానభ్రంశం = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_ఎక్కించు_addr, external_splash_కొలత);
		ఒకవేళ (res < 0)
			తిరిగి res;
	}

	తిరిగి 0;
}
#అయితే_ముగింపు /* CONFIG_FIT */

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
పూర్ణాంకం splash_ఆధారం_ఎక్కించు(నిర్మాణం splash_location *locations, uint కొలత)
{
	నిర్మాణం splash_location *splash_location;
	అక్షరం *env_splashimage_విలువ;
	అక్షరం *devpart;
	ulong bmp_ఎక్కించు_addr;

	env_splashimage_విలువ = env_get("splashimage");
	ఒకవేళ (env_splashimage_విలువ == ఖాళీ)
		తిరిగి -ENOENT;

	bmp_ఎక్కించు_addr = hextoul(env_splashimage_విలువ, 0);
	ఒకవేళ (bmp_ఎక్కించు_addr == 0) {
		రాయి("Error: bad splashimage address specified\n");
		తిరిగి -EFAULT;
	}

	splash_location = select_splash_location(locations, కొలత);
	ఒకవేళ (!splash_location)
		తిరిగి -EINVAL;

	devpart = env_get("splashdevpart");
	ఒకవేళ (devpart)
		splash_location->devpart = devpart;

	ఒకవేళ (splash_location->flags == SPLASH_STORAGE_RAW)
		తిరిగి splash_ఎక్కించు_raw(splash_location, bmp_ఎక్కించు_addr);
	లేకపోతే ఒకవేళ (splash_location->flags == SPLASH_STORAGE_FS)
		తిరిగి splash_ఎక్కించు_fs(splash_location, bmp_ఎక్కించు_addr);
#నిర్వచితమైతే CONFIG_FIT
	లేకపోతే ఒకవేళ (splash_location->flags == SPLASH_STORAGE_FIT)
		తిరిగి splash_ఎక్కించు_fit(splash_location, bmp_ఎక్కించు_addr);
#అయితే_ముగింపు
	తిరిగి -EINVAL;
}
