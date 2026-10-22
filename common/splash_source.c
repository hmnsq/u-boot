// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <bmp_layout.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <command.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fs.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fdt_support.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <nand.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sata.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spi_flash.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <splash.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <usb.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <virtio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPI_FLASH
ਸਥਿਰ ਬਣਤਰ spi_flash *sf;
ਸਥਿਰ ਪੂਰਨ splash_sf_read_raw(ulong bmp_ਲੱਦੋ_addr, ਪੂਰਨ ਵਿੱਥ, size_t read_ਮਾਪ)
{
	ਜੇ (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_ਰਫਤਾਰ,
				     CONFIG_SF_DEFAULT_ਬਹੁਲਕ);
		ਜੇ (!sf)
			ਵਾਪਸ -ENODEV;
	}

	ਵਾਪਸ spi_flash_read(sf, ਵਿੱਥ, read_ਮਾਪ, (ਖਾਲੀ *)(uintptr_t)bmp_ਲੱਦੋ_addr);
}
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਪੂਰਨ splash_sf_read_raw(ulong bmp_ਲੱਦੋ_addr, ਪੂਰਨ ਵਿੱਥ, size_t read_ਮਾਪ)
{
	debug("%s: sf support not available\n", __func__);
	ਵਾਪਸ -ENOSYS;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_CMD_NAND
ਸਥਿਰ ਪੂਰਨ splash_nand_read_raw(ulong bmp_ਲੱਦੋ_addr, ਪੂਰਨ ਵਿੱਥ, size_t read_ਮਾਪ)
{
	ਬਣਤਰ mtd_info *mtd = get_nand_dev_by_ਸੂਚਕਾਂਕ(nand_curr_device);
	ਵਾਪਸ nand_read_skip_bad(mtd, ਵਿੱਥ,
				  &read_ਮਾਪ, ਨਿਰਾ,
				  mtd->ਮਾਪ,
				  (u_char *)bmp_ਲੱਦੋ_addr);
}
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਪੂਰਨ splash_nand_read_raw(ulong bmp_ਲੱਦੋ_addr, ਪੂਰਨ ਵਿੱਥ, size_t read_ਮਾਪ)
{
	debug("%s: nand support not available\n", __func__);
	ਵਾਪਸ -ENOSYS;
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ splash_mmc_read_raw(ulong bmp_ਲੱਦੋ_addr, ਬਣਤਰ splash_location *location,
			       size_t read_ਮਾਪ)
{
	ਬਣਤਰ disk_ਹਿੱਸੇ_ਕਰੋ ਹਿੱਸੇ_ਕਰੋ;
	ਬਣਤਰ blk_desc *desc;
	lbaint_t blkcnt;
	ਪੂਰਨ ret, n;

	ਜੇ (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		ਵਾਪਸ -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_ਅੰਕ("mmc", location->devpart, &desc,
						   &ਹਿੱਸੇ_ਕਰੋ, 1);
	ਜੇ (ret < 0)
		ਵਾਪਸ ret;

	blkcnt = DIV_ਗੇੜਾ_ਉੱਪਰ(read_ਮਾਪ, ਹਿੱਸੇ_ਕਰੋ.blksz);
	n = blk_dread(desc, ਹਿੱਸੇ_ਕਰੋ.ਸ਼ੁਰੂ, blkcnt, (ਖਾਲੀ *)(uintptr_t)bmp_ਲੱਦੋ_addr);

	ਵਾਪਸ (n == blkcnt) ? 0 : -EIO;
}

ਸਥਿਰ ਪੂਰਨ splash_storage_read_raw(ਬਣਤਰ splash_location *location,
			       ulong bmp_ਲੱਦੋ_addr, size_t read_ਮਾਪ)
{
	u32 ਵਿੱਥ;

	ਜੇ (!location)
		ਵਾਪਸ -EINVAL;

	ਵਿੱਥ = location->ਵਿੱਥ;
	ਚੋਣ (location->storage) {
	ਹਾਲਤ SPLASH_STORAGE_MMC:
		ਵਾਪਸ splash_mmc_read_raw(bmp_ਲੱਦੋ_addr, location, read_ਮਾਪ);
	ਹਾਲਤ SPLASH_STORAGE_NAND:
		ਵਾਪਸ splash_nand_read_raw(bmp_ਲੱਦੋ_addr, ਵਿੱਥ, read_ਮਾਪ);
	ਹਾਲਤ SPLASH_STORAGE_SF:
		ਵਾਪਸ splash_sf_read_raw(bmp_ਲੱਦੋ_addr, ਵਿੱਥ, read_ਮਾਪ);
	ਮੂਲ:
		ਲਿਖੋ("Unknown splash location\n");
	}

	ਵਾਪਸ -EINVAL;
}

ਸਥਿਰ ਪੂਰਨ splash_ਲੱਦੋ_raw(ਬਣਤਰ splash_location *location, ulong bmp_ਲੱਦੋ_addr)
{
	ਬਣਤਰ bmp_ਸਿਰਲੇਖ *bmp_hdr;
	ਪੂਰਨ res;
	size_t bmp_ਮਾਪ, bmp_ਸਿਰਲੇਖ_ਮਾਪ = ਆਕਾਰ(ਬਣਤਰ bmp_ਸਿਰਲੇਖ);

	ਜੇ (bmp_ਲੱਦੋ_addr + bmp_ਸਿਰਲੇਖ_ਮਾਪ >= gd->ਸ਼ੁਰੂ_addr_sp)
		ਜਾਓ splash_ਪਤਾ_too_ਉੱਚਾ;

	res = splash_storage_read_raw(location, bmp_ਲੱਦੋ_addr, bmp_ਸਿਰਲੇਖ_ਮਾਪ);
	ਜੇ (res < 0)
		ਵਾਪਸ res;

	bmp_hdr = (ਬਣਤਰ bmp_ਸਿਰਲੇਖ *)(uintptr_t)bmp_ਲੱਦੋ_addr;
	bmp_ਮਾਪ = le32_ਮੰਜ਼ਿਲ_cpu(bmp_hdr->file_ਮਾਪ);

	ਜੇ (bmp_ਲੱਦੋ_addr + bmp_ਮਾਪ >= gd->ਸ਼ੁਰੂ_addr_sp)
		ਜਾਓ splash_ਪਤਾ_too_ਉੱਚਾ;

	ਵਾਪਸ splash_storage_read_raw(location, bmp_ਲੱਦੋ_addr, bmp_ਮਾਪ);

splash_ਪਤਾ_too_ਉੱਚਾ:
	ਲਿਖੋ("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	ਵਾਪਸ -EFAULT;
}

ਸਥਿਰ ਪੂਰਨ splash_select_fs_dev(ਬਣਤਰ splash_location *location)
{
	ਪੂਰਨ res;

	ਚੋਣ (location->storage) {
	ਹਾਲਤ SPLASH_STORAGE_MMC:
		res = fs_ਸੈੱਟ_blk_dev("mmc", location->devpart, FS_TYPE_ANY);
		ਰੋਕੋ;
	ਹਾਲਤ SPLASH_STORAGE_USB:
		res = fs_ਸੈੱਟ_blk_dev("usb", location->devpart, FS_TYPE_ANY);
		ਰੋਕੋ;
	ਹਾਲਤ SPLASH_STORAGE_SATA:
		res = fs_ਸੈੱਟ_blk_dev("sata", location->devpart, FS_TYPE_ANY);
		ਰੋਕੋ;
	ਹਾਲਤ SPLASH_STORAGE_NAND:
		ਜੇ (location->ubivol != ਨਿਰਾ)
			res = fs_ਸੈੱਟ_blk_dev("ubi", ਨਿਰਾ, FS_TYPE_UBIFS);
		ਨਹੀਂਤਾਂ
			res = -ENODEV;
		ਰੋਕੋ;
	ਮੂਲ:
		ਲਿਖੋ("Error: %s: unsupported location storage.\n", __func__);
		ਵਾਪਸ -ENODEV;
	}

	ਜੇ (res)
		ਲਿਖੋ("Error: %s: could not access storage.\n", __func__);

	ਵਾਪਸ res;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USB_STORAGE
ਸਥਿਰ ਪੂਰਨ splash_init_usb(ਖਾਲੀ)
{
	ਪੂਰਨ err;

	err = usb_init();
	ਜੇ (err)
		ਵਾਪਸ err;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_DM_USB
	err = usb_stor_ਜਾਂਚ(1) < 0 ? -ENODEV : 0;
#ਜੇ_ਅੰਤ

	ਵਾਪਸ err;
}
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ splash_init_usb(ਖਾਲੀ)
{
	ਲਿਖੋ("Cannot load splash image: no USB support\n");
	ਵਾਪਸ -ENOSYS;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SATA
ਸਥਿਰ ਪੂਰਨ splash_init_sata(ਖਾਲੀ)
{
	ਵਾਪਸ sata_probe(0);
}
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ splash_init_sata(ਖਾਲੀ)
{
	ਲਿਖੋ("Cannot load splash image: no SATA support\n");
	ਵਾਪਸ -ENOSYS;
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ splash_init_virtio(ਖਾਲੀ)
{
	ਜੇ (!IS_ENABLED(CONFIG_VIRTIO)) {
		ਲਿਖੋ("Cannot load splash image: no virtio support\n");
		ਵਾਪਸ -ENOSYS;
	} ਨਹੀਂਤਾਂ {
		ਵਾਪਸ virtio_init();
	}
}

#ਜੇ defined(CONFIG_CMD_UBIFS) && !defined(CONFIG_XPL_BUILD)
ਸਥਿਰ ਪੂਰਨ splash_mount_ubifs(ਬਣਤਰ splash_location *location)
{
	ਪੂਰਨ res;
	ਅੱਖਰ cmd[32];

	sprintf(cmd, "ubi part %s", location->mtdpart);
	res = ਲੜੀਵਾਰ_command(cmd, 0);
	ਜੇ (res)
		ਵਾਪਸ res;

	sprintf(cmd, "ubifsmount %s", location->ubivol);
	res = ਲੜੀਵਾਰ_command(cmd, 0);

	ਵਾਪਸ res;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ splash_umount_ubifs(ਖਾਲੀ)
{
	ਵਾਪਸ ਲੜੀਵਾਰ_command("ubifsumount", 0);
}
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ splash_mount_ubifs(ਬਣਤਰ splash_location *location)
{
	ਲਿਖੋ("Cannot load splash image: no UBIFS support\n");
	ਵਾਪਸ -ENOSYS;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ splash_umount_ubifs(ਖਾਲੀ)
{
	ਲਿਖੋ("Cannot unmount UBIFS: no UBIFS support\n");
	ਵਾਪਸ -ENOSYS;
}
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SPLASH_ਸਰੋਤ_DEFAULT_FILE_NAME		"splash.bmp"

ਸਥਿਰ ਪੂਰਨ splash_ਲੱਦੋ_fs(ਬਣਤਰ splash_location *location, ulong bmp_ਲੱਦੋ_addr)
{
	ਪੂਰਨ res = 0;
	loff_t bmp_ਮਾਪ;
	loff_t actread;
	ਅੱਖਰ *splash_file;

	splash_file = env_get("splashfile");
	ਜੇ (!splash_file)
		splash_file = SPLASH_ਸਰੋਤ_DEFAULT_FILE_NAME;

	ਜੇ (location->storage == SPLASH_STORAGE_USB)
		res = splash_init_usb();

	ਜੇ (location->storage == SPLASH_STORAGE_SATA)
		res = splash_init_sata();

	ਜੇ (location->storage == SPLASH_STORAGE_VIRTIO)
		res = splash_init_virtio();

	ਜੇ (location->ubivol != ਨਿਰਾ)
		res = splash_mount_ubifs(location);

	ਜੇ (res)
		ਵਾਪਸ res;

	res = splash_select_fs_dev(location);
	ਜੇ (res)
		ਜਾਓ ਬਾਹਰਲਾ;

	res = fs_ਮਾਪ(splash_file, &bmp_ਮਾਪ);
	ਜੇ (res) {
		ਲਿਖੋ("Error: %s: cannot determine file size (%d)\n",
		       __func__, res);
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਜੇ (bmp_ਲੱਦੋ_addr + bmp_ਮਾਪ >= gd->ਸ਼ੁਰੂ_addr_sp) {
		ਲਿਖੋ("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_ਲੱਦੋ_addr, 0, 0, &actread);

ਬਾਹਰਲਾ:
	ਜੇ (location->ubivol != ਨਿਰਾ)
		splash_umount_ubifs();

	ਵਾਪਸ res;
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
ਸਥਿਰ ਬਣਤਰ splash_location *select_splash_location(
			    ਬਣਤਰ splash_location *locations, uint ਮਾਪ)
{
	ਪੂਰਨ i;
	ਅੱਖਰ *env_splashsource;

	ਜੇ (!locations || ਮਾਪ == 0)
		ਵਾਪਸ ਨਿਰਾ;

	env_splashsource = env_get("splashsource");
	ਜੇ (env_splashsource == ਨਿਰਾ)
		ਵਾਪਸ &locations[0];

	ਲਈ (i = 0; i < ਮਾਪ; i++) {
		ਜੇ (!strcmp(locations[i].name, env_splashsource))
			ਵਾਪਸ &locations[i];
	}

	ਲਿਖੋ("splashsource env variable set to unsupported value\n");
	ਵਾਪਸ ਨਿਰਾ;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_FIT
ਸਥਿਰ ਪੂਰਨ splash_ਲੱਦੋ_fit(ਬਣਤਰ splash_location *location, ulong bmp_ਲੱਦੋ_addr)
{
	ਪੂਰਨ res;
	ਪੂਰਨ ਗੰਢ_ਵਿੱਥ;
	ਅਚਲ ਅੱਖਰ *splash_file;
	ਅਚਲ ਖਾਲੀ *ਅੰਦਰੂਨੀ_splash_ਅੰਕੜੇ;
	size_t ਅੰਦਰੂਨੀ_splash_ਮਾਪ;
	ਪੂਰਨ external_splash_addr;
	u32 external_splash_ਮਾਪ;
	bool is_splash_external = false;
	ਬਣਤਰ legacy_img_hdr *img_ਸਿਰਲੇਖ;
	ਅਚਲ u32 *fit_ਸਿਰਲੇਖ;
	u32 fit_ਮਾਪ;
	ਅਚਲ size_t ਸਿਰਲੇਖ_ਮਾਪ = ਆਕਾਰ(ਬਣਤਰ legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_ਲੱਦੋ_addr, ਸਿਰਲੇਖ_ਮਾਪ);
	ਜੇ (res < 0)
		ਵਾਪਸ res;

	img_ਸਿਰਲੇਖ = (ਬਣਤਰ legacy_img_hdr *)(uintptr_t)bmp_ਲੱਦੋ_addr;
	ਜੇ (image_get_magic(img_ਸਿਰਲੇਖ) != FDT_MAGIC) {
		ਲਿਖੋ("Could not find FDT magic\n");
		ਵਾਪਸ -EINVAL;
	}

	fit_ਮਾਪ = fdt_totalsize(img_ਸਿਰਲੇਖ);

	/* Read in entire FIT */
	fit_ਸਿਰਲੇਖ = (ਅਚਲ u32 *)(bmp_ਲੱਦੋ_addr + ਸਿਰਲੇਖ_ਮਾਪ);
	res = splash_storage_read_raw(location, (uintptr_t)fit_ਸਿਰਲੇਖ, fit_ਮਾਪ);
	ਜੇ (res < 0)
		ਵਾਪਸ res;

	res = fit_check_format(fit_ਸਿਰਲੇਖ, IMAGE_ਮਾਪ_INVAL);
	ਜੇ (res) {
		debug("Could not find valid FIT image\n");
		ਵਾਪਸ res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	ਜੇ (!splash_file)
		splash_file = SPLASH_ਸਰੋਤ_DEFAULT_FILE_NAME;

	ਗੰਢ_ਵਿੱਥ = fit_image_get_ਗੰਢ(fit_ਸਿਰਲੇਖ, splash_file);
	ਜੇ (ਗੰਢ_ਵਿੱਥ < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		ਵਾਪਸ -ENOENT;
	}

	/* Extract the splash data from FIT */
	ਜੇ (!fit_image_get_ਅੰਕੜੇ(fit_ਸਿਰਲੇਖ, ਗੰਢ_ਵਿੱਥ, &ਅੰਦਰੂਨੀ_splash_ਅੰਕੜੇ,
				&ਅੰਦਰੂਨੀ_splash_ਮਾਪ)) {
		memmove((ਖਾਲੀ *)(uintptr_t)bmp_ਲੱਦੋ_addr, ਅੰਦਰੂਨੀ_splash_ਅੰਕੜੇ,
			ਅੰਦਰੂਨੀ_splash_ਮਾਪ);
	} ਨਹੀਂਤਾਂ {
		ਲਿਖੋ("Failed to get splash image from FIT\n");
		ਵਾਪਸ -ENODATA;
	}

	ਜੇ (is_splash_external) {
		res = fit_image_get_ਅੰਕੜੇ_ਮਾਪ(fit_ਸਿਰਲੇਖ, ਗੰਢ_ਵਿੱਥ, &external_splash_ਮਾਪ);
		ਜੇ (res < 0) {
			ਲਿਖੋ("Failed to get size of splash image (err=%d)\n", res);
			ਵਾਪਸ res;
		}

		/* Read in the splash data */
		location->ਵਿੱਥ = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_ਲੱਦੋ_addr, external_splash_ਮਾਪ);
		ਜੇ (res < 0)
			ਵਾਪਸ res;
	}

	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ /* CONFIG_FIT */

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
ਪੂਰਨ splash_ਸਰੋਤ_ਲੱਦੋ(ਬਣਤਰ splash_location *locations, uint ਮਾਪ)
{
	ਬਣਤਰ splash_location *splash_location;
	ਅੱਖਰ *env_splashimage_ਮੁੱਲ;
	ਅੱਖਰ *devpart;
	ulong bmp_ਲੱਦੋ_addr;

	env_splashimage_ਮੁੱਲ = env_get("splashimage");
	ਜੇ (env_splashimage_ਮੁੱਲ == ਨਿਰਾ)
		ਵਾਪਸ -ENOENT;

	bmp_ਲੱਦੋ_addr = hextoul(env_splashimage_ਮੁੱਲ, 0);
	ਜੇ (bmp_ਲੱਦੋ_addr == 0) {
		ਲਿਖੋ("Error: bad splashimage address specified\n");
		ਵਾਪਸ -EFAULT;
	}

	splash_location = select_splash_location(locations, ਮਾਪ);
	ਜੇ (!splash_location)
		ਵਾਪਸ -EINVAL;

	devpart = env_get("splashdevpart");
	ਜੇ (devpart)
		splash_location->devpart = devpart;

	ਜੇ (splash_location->flags == SPLASH_STORAGE_RAW)
		ਵਾਪਸ splash_ਲੱਦੋ_raw(splash_location, bmp_ਲੱਦੋ_addr);
	ਨਹੀਂਤਾਂ ਜੇ (splash_location->flags == SPLASH_STORAGE_FS)
		ਵਾਪਸ splash_ਲੱਦੋ_fs(splash_location, bmp_ਲੱਦੋ_addr);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_FIT
	ਨਹੀਂਤਾਂ ਜੇ (splash_location->flags == SPLASH_STORAGE_FIT)
		ਵਾਪਸ splash_ਲੱਦੋ_fit(splash_location, bmp_ਲੱਦੋ_addr);
#ਜੇ_ਅੰਤ
	ਵਾਪਸ -EINVAL;
}
