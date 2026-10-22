// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 */

#ಅನ್ತರ್ಭಾವ <bmp_layout.h>
#ಅನ್ತರ್ಭಾವ <command.h>
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fs.h>
#ಅನ್ತರ್ಭಾವ <fdt_support.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <nand.h>
#ಅನ್ತರ್ಭಾವ <sata.h>
#ಅನ್ತರ್ಭಾವ <spi.h>
#ಅನ್ತರ್ಭಾವ <spi_flash.h>
#ಅನ್ತರ್ಭಾವ <splash.h>
#ಅನ್ತರ್ಭಾವ <usb.h>
#ಅನ್ತರ್ಭಾವ <virtio.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPI_FLASH
ಸ್ಥಿರ ರಚನೆ spi_flash *sf;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ splash_sf_read_raw(ulong bmp_ತುಂಬು_addr, ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ, size_t read_ಗಾತ್ರ)
{
	ಆದರೆ (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_ವೇಗ,
				     CONFIG_SF_DEFAULT_ಬಹುಲಕ);
		ಆದರೆ (!sf)
			ಹಿಂತಿರುಗಿ -ENODEV;
	}

	ಹಿಂತಿರುಗಿ spi_flash_read(sf, ಸ್ಥಾನಾಂತರ, read_ಗಾತ್ರ, (ಶೂನ್ಯ *)(uintptr_t)bmp_ತುಂಬು_addr);
}
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ splash_sf_read_raw(ulong bmp_ತುಂಬು_addr, ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ, size_t read_ಗಾತ್ರ)
{
	debug("%s: sf support not available\n", __func__);
	ಹಿಂತಿರುಗಿ -ENOSYS;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_CMD_NAND
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ splash_nand_read_raw(ulong bmp_ತುಂಬು_addr, ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ, size_t read_ಗಾತ್ರ)
{
	ರಚನೆ mtd_info *mtd = get_nand_dev_by_ಸೂಚ್ಯಂಕ(nand_curr_device);
	ಹಿಂತಿರುಗಿ nand_read_skip_bad(mtd, ಸ್ಥಾನಾಂತರ,
				  &read_ಗಾತ್ರ, ಖಾಲಿ,
				  mtd->ಗಾತ್ರ_2,
				  (u_char *)bmp_ತುಂಬು_addr);
}
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ splash_nand_read_raw(ulong bmp_ತುಂಬು_addr, ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ, size_t read_ಗಾತ್ರ)
{
	debug("%s: nand support not available\n", __func__);
	ಹಿಂತಿರುಗಿ -ENOSYS;
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ splash_mmc_read_raw(ulong bmp_ತುಂಬು_addr, ರಚನೆ splash_location *location,
			       size_t read_ಗಾತ್ರ)
{
	ರಚನೆ disk_ವಿಭಜನೆ ವಿಭಜನೆ;
	ರಚನೆ blk_desc *desc;
	lbaint_t blkcnt;
	ಪೂರ್ಣಾಂಕ ret, n;

	ಆದರೆ (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		ಹಿಂತಿರುಗಿ -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_ಅಂಕೆ("mmc", location->devpart, &desc,
						   &ವಿಭಜನೆ, 1);
	ಆದರೆ (ret < 0)
		ಹಿಂತಿರುಗಿ ret;

	blkcnt = DIV_ಸುತ್ತು_ಮೇಲೆ(read_ಗಾತ್ರ, ವಿಭಜನೆ.blksz);
	n = blk_dread(desc, ವಿಭಜನೆ.ಆರಂಭ, blkcnt, (ಶೂನ್ಯ *)(uintptr_t)bmp_ತುಂಬು_addr);

	ಹಿಂತಿರುಗಿ (n == blkcnt) ? 0 : -EIO;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ splash_storage_read_raw(ರಚನೆ splash_location *location,
			       ulong bmp_ತುಂಬು_addr, size_t read_ಗಾತ್ರ)
{
	u32 ಸ್ಥಾನಾಂತರ;

	ಆದರೆ (!location)
		ಹಿಂತಿರುಗಿ -EINVAL;

	ಸ್ಥಾನಾಂತರ = location->ಸ್ಥಾನಾಂತರ;
	ಆಯ್ಕೆ (location->storage) {
	ಸ್ಥಿತಿ SPLASH_STORAGE_MMC:
		ಹಿಂತಿರುಗಿ splash_mmc_read_raw(bmp_ತುಂಬು_addr, location, read_ಗಾತ್ರ);
	ಸ್ಥಿತಿ SPLASH_STORAGE_NAND:
		ಹಿಂತಿರುಗಿ splash_nand_read_raw(bmp_ತುಂಬು_addr, ಸ್ಥಾನಾಂತರ, read_ಗಾತ್ರ);
	ಸ್ಥಿತಿ SPLASH_STORAGE_SF:
		ಹಿಂತಿರುಗಿ splash_sf_read_raw(bmp_ತುಂಬು_addr, ಸ್ಥಾನಾಂತರ, read_ಗಾತ್ರ);
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಬರೆ("Unknown splash location\n");
	}

	ಹಿಂತಿರುಗಿ -EINVAL;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ splash_ತುಂಬು_raw(ರಚನೆ splash_location *location, ulong bmp_ತುಂಬು_addr)
{
	ರಚನೆ bmp_ಶೀರ್ಷಿಕೆ *bmp_hdr;
	ಪೂರ್ಣಾಂಕ res;
	size_t bmp_ಗಾತ್ರ, bmp_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ = ಗಾತ್ರ(ರಚನೆ bmp_ಶೀರ್ಷಿಕೆ);

	ಆದರೆ (bmp_ತುಂಬು_addr + bmp_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ >= gd->ಆರಂಭ_addr_sp)
		ಹೋಗು splash_ವಿಳಾಸ_too_ಮೇಲಿನ;

	res = splash_storage_read_raw(location, bmp_ತುಂಬು_addr, bmp_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ);
	ಆದರೆ (res < 0)
		ಹಿಂತಿರುಗಿ res;

	bmp_hdr = (ರಚನೆ bmp_ಶೀರ್ಷಿಕೆ *)(uintptr_t)bmp_ತುಂಬು_addr;
	bmp_ಗಾತ್ರ = le32_ಗಮ್ಯ_cpu(bmp_hdr->file_ಗಾತ್ರ);

	ಆದರೆ (bmp_ತುಂಬು_addr + bmp_ಗಾತ್ರ >= gd->ಆರಂಭ_addr_sp)
		ಹೋಗು splash_ವಿಳಾಸ_too_ಮೇಲಿನ;

	ಹಿಂತಿರುಗಿ splash_storage_read_raw(location, bmp_ತುಂಬು_addr, bmp_ಗಾತ್ರ);

splash_ವಿಳಾಸ_too_ಮೇಲಿನ:
	ಬರೆ("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	ಹಿಂತಿರುಗಿ -EFAULT;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ splash_select_fs_dev(ರಚನೆ splash_location *location)
{
	ಪೂರ್ಣಾಂಕ res;

	ಆಯ್ಕೆ (location->storage) {
	ಸ್ಥಿತಿ SPLASH_STORAGE_MMC:
		res = fs_ಗಣ_blk_dev("mmc", location->devpart, FS_TYPE_ANY);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ SPLASH_STORAGE_USB:
		res = fs_ಗಣ_blk_dev("usb", location->devpart, FS_TYPE_ANY);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ SPLASH_STORAGE_SATA:
		res = fs_ಗಣ_blk_dev("sata", location->devpart, FS_TYPE_ANY);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ SPLASH_STORAGE_NAND:
		ಆದರೆ (location->ubivol != ಖಾಲಿ)
			res = fs_ಗಣ_blk_dev("ubi", ಖಾಲಿ, FS_TYPE_UBIFS);
		ಇಲ್ಲದಿದ್ದರೆ
			res = -ENODEV;
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಬರೆ("Error: %s: unsupported location storage.\n", __func__);
		ಹಿಂತಿರುಗಿ -ENODEV;
	}

	ಆದರೆ (res)
		ಬರೆ("Error: %s: could not access storage.\n", __func__);

	ಹಿಂತಿರುಗಿ res;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USB_STORAGE
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ splash_init_usb(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ err;

	err = usb_init();
	ಆದರೆ (err)
		ಹಿಂತಿರುಗಿ err;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_DM_USB
	err = usb_stor_ಪರಿಶೀಲನೆ(1) < 0 ? -ENODEV : 0;
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ err;
}
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ splash_init_usb(ಶೂನ್ಯ)
{
	ಬರೆ("Cannot load splash image: no USB support\n");
	ಹಿಂತಿರುಗಿ -ENOSYS;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SATA
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ splash_init_sata(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ sata_probe(0);
}
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ splash_init_sata(ಶೂನ್ಯ)
{
	ಬರೆ("Cannot load splash image: no SATA support\n");
	ಹಿಂತಿರುಗಿ -ENOSYS;
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ splash_init_virtio(ಶೂನ್ಯ)
{
	ಆದರೆ (!IS_ENABLED(CONFIG_VIRTIO)) {
		ಬರೆ("Cannot load splash image: no virtio support\n");
		ಹಿಂತಿರುಗಿ -ENOSYS;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಹಿಂತಿರುಗಿ virtio_init();
	}
}

#ಆದರೆ defined(CONFIG_CMD_UBIFS) && !defined(CONFIG_XPL_BUILD)
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ splash_mount_ubifs(ರಚನೆ splash_location *location)
{
	ಪೂರ್ಣಾಂಕ res;
	ಅಕ್ಷರ cmd[32];

	sprintf(cmd, "ubi part %s", location->mtdpart);
	res = ಸತತ_command(cmd, 0);
	ಆದರೆ (res)
		ಹಿಂತಿರುಗಿ res;

	sprintf(cmd, "ubifsmount %s", location->ubivol);
	res = ಸತತ_command(cmd, 0);

	ಹಿಂತಿರುಗಿ res;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ splash_umount_ubifs(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ ಸತತ_command("ubifsumount", 0);
}
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ splash_mount_ubifs(ರಚನೆ splash_location *location)
{
	ಬರೆ("Cannot load splash image: no UBIFS support\n");
	ಹಿಂತಿರುಗಿ -ENOSYS;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ splash_umount_ubifs(ಶೂನ್ಯ)
{
	ಬರೆ("Cannot unmount UBIFS: no UBIFS support\n");
	ಹಿಂತಿರುಗಿ -ENOSYS;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು SPLASH_ಮೂಲ_DEFAULT_FILE_NAME		"splash.bmp"

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ splash_ತುಂಬು_fs(ರಚನೆ splash_location *location, ulong bmp_ತುಂಬು_addr)
{
	ಪೂರ್ಣಾಂಕ res = 0;
	loff_t bmp_ಗಾತ್ರ;
	loff_t actread;
	ಅಕ್ಷರ *splash_file;

	splash_file = env_get("splashfile");
	ಆದರೆ (!splash_file)
		splash_file = SPLASH_ಮೂಲ_DEFAULT_FILE_NAME;

	ಆದರೆ (location->storage == SPLASH_STORAGE_USB)
		res = splash_init_usb();

	ಆದರೆ (location->storage == SPLASH_STORAGE_SATA)
		res = splash_init_sata();

	ಆದರೆ (location->storage == SPLASH_STORAGE_VIRTIO)
		res = splash_init_virtio();

	ಆದರೆ (location->ubivol != ಖಾಲಿ)
		res = splash_mount_ubifs(location);

	ಆದರೆ (res)
		ಹಿಂತಿರುಗಿ res;

	res = splash_select_fs_dev(location);
	ಆದರೆ (res)
		ಹೋಗು ಹೊರಹೋಗು;

	res = fs_ಗಾತ್ರ(splash_file, &bmp_ಗಾತ್ರ);
	ಆದರೆ (res) {
		ಬರೆ("Error: %s: cannot determine file size (%d)\n",
		       __func__, res);
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಆದರೆ (bmp_ತುಂಬು_addr + bmp_ಗಾತ್ರ >= gd->ಆರಂಭ_addr_sp) {
		ಬರೆ("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_ತುಂಬು_addr, 0, 0, &actread);

ಹೊರಹೋಗು:
	ಆದರೆ (location->ubivol != ಖಾಲಿ)
		splash_umount_ubifs();

	ಹಿಂತಿರುಗಿ res;
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
ಸ್ಥಿರ ರಚನೆ splash_location *select_splash_location(
			    ರಚನೆ splash_location *locations, uint ಗಾತ್ರ_2)
{
	ಪೂರ್ಣಾಂಕ i;
	ಅಕ್ಷರ *env_splashsource;

	ಆದರೆ (!locations || ಗಾತ್ರ_2 == 0)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	env_splashsource = env_get("splashsource");
	ಆದರೆ (env_splashsource == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ &locations[0];

	ಗಾಗಿ (i = 0; i < ಗಾತ್ರ_2; i++) {
		ಆದರೆ (!strcmp(locations[i].name, env_splashsource))
			ಹಿಂತಿರುಗಿ &locations[i];
	}

	ಬರೆ("splashsource env variable set to unsupported value\n");
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_FIT
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ splash_ತುಂಬು_fit(ರಚನೆ splash_location *location, ulong bmp_ತುಂಬು_addr)
{
	ಪೂರ್ಣಾಂಕ res;
	ಪೂರ್ಣಾಂಕ ಗಂಟು_ಸ್ಥಾನಾಂತರ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *splash_file;
	ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಆಂತರಿಕ_splash_ದತ್ತಾಂಶ;
	size_t ಆಂತರಿಕ_splash_ಗಾತ್ರ;
	ಪೂರ್ಣಾಂಕ external_splash_addr;
	u32 external_splash_ಗಾತ್ರ;
	bool is_splash_external = false;
	ರಚನೆ legacy_img_hdr *img_ಶೀರ್ಷಿಕೆ;
	ಸ್ಥಿರಾಂಕ u32 *fit_ಶೀರ್ಷಿಕೆ;
	u32 fit_ಗಾತ್ರ;
	ಸ್ಥಿರಾಂಕ size_t ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ = ಗಾತ್ರ(ರಚನೆ legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_ತುಂಬು_addr, ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ);
	ಆದರೆ (res < 0)
		ಹಿಂತಿರುಗಿ res;

	img_ಶೀರ್ಷಿಕೆ = (ರಚನೆ legacy_img_hdr *)(uintptr_t)bmp_ತುಂಬು_addr;
	ಆದರೆ (image_get_magic(img_ಶೀರ್ಷಿಕೆ) != FDT_MAGIC) {
		ಬರೆ("Could not find FDT magic\n");
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	fit_ಗಾತ್ರ = fdt_totalsize(img_ಶೀರ್ಷಿಕೆ);

	/* Read in entire FIT */
	fit_ಶೀರ್ಷಿಕೆ = (ಸ್ಥಿರಾಂಕ u32 *)(bmp_ತುಂಬು_addr + ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ);
	res = splash_storage_read_raw(location, (uintptr_t)fit_ಶೀರ್ಷಿಕೆ, fit_ಗಾತ್ರ);
	ಆದರೆ (res < 0)
		ಹಿಂತಿರುಗಿ res;

	res = fit_check_format(fit_ಶೀರ್ಷಿಕೆ, IMAGE_ಗಾತ್ರ_INVAL);
	ಆದರೆ (res) {
		debug("Could not find valid FIT image\n");
		ಹಿಂತಿರುಗಿ res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	ಆದರೆ (!splash_file)
		splash_file = SPLASH_ಮೂಲ_DEFAULT_FILE_NAME;

	ಗಂಟು_ಸ್ಥಾನಾಂತರ = fit_image_get_ಗಂಟು(fit_ಶೀರ್ಷಿಕೆ, splash_file);
	ಆದರೆ (ಗಂಟು_ಸ್ಥಾನಾಂತರ < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		ಹಿಂತಿರುಗಿ -ENOENT;
	}

	/* Extract the splash data from FIT */
	ಆದರೆ (!fit_image_get_ದತ್ತಾಂಶ(fit_ಶೀರ್ಷಿಕೆ, ಗಂಟು_ಸ್ಥಾನಾಂತರ, &ಆಂತರಿಕ_splash_ದತ್ತಾಂಶ,
				&ಆಂತರಿಕ_splash_ಗಾತ್ರ)) {
		memmove((ಶೂನ್ಯ *)(uintptr_t)bmp_ತುಂಬು_addr, ಆಂತರಿಕ_splash_ದತ್ತಾಂಶ,
			ಆಂತರಿಕ_splash_ಗಾತ್ರ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಬರೆ("Failed to get splash image from FIT\n");
		ಹಿಂತಿರುಗಿ -ENODATA;
	}

	ಆದರೆ (is_splash_external) {
		res = fit_image_get_ದತ್ತಾಂಶ_ಗಾತ್ರ(fit_ಶೀರ್ಷಿಕೆ, ಗಂಟು_ಸ್ಥಾನಾಂತರ, &external_splash_ಗಾತ್ರ);
		ಆದರೆ (res < 0) {
			ಬರೆ("Failed to get size of splash image (err=%d)\n", res);
			ಹಿಂತಿರುಗಿ res;
		}

		/* Read in the splash data */
		location->ಸ್ಥಾನಾಂತರ = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_ತುಂಬು_addr, external_splash_ಗಾತ್ರ);
		ಆದರೆ (res < 0)
			ಹಿಂತಿರುಗಿ res;
	}

	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_FIT */

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
ಪೂರ್ಣಾಂಕ splash_ಮೂಲ_ತುಂಬು(ರಚನೆ splash_location *locations, uint ಗಾತ್ರ_2)
{
	ರಚನೆ splash_location *splash_location;
	ಅಕ್ಷರ *env_splashimage_ಮೌಲ್ಯ;
	ಅಕ್ಷರ *devpart;
	ulong bmp_ತುಂಬು_addr;

	env_splashimage_ಮೌಲ್ಯ = env_get("splashimage");
	ಆದರೆ (env_splashimage_ಮೌಲ್ಯ == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ -ENOENT;

	bmp_ತುಂಬು_addr = hextoul(env_splashimage_ಮೌಲ್ಯ, 0);
	ಆದರೆ (bmp_ತುಂಬು_addr == 0) {
		ಬರೆ("Error: bad splashimage address specified\n");
		ಹಿಂತಿರುಗಿ -EFAULT;
	}

	splash_location = select_splash_location(locations, ಗಾತ್ರ_2);
	ಆದರೆ (!splash_location)
		ಹಿಂತಿರುಗಿ -EINVAL;

	devpart = env_get("splashdevpart");
	ಆದರೆ (devpart)
		splash_location->devpart = devpart;

	ಆದರೆ (splash_location->flags == SPLASH_STORAGE_RAW)
		ಹಿಂತಿರುಗಿ splash_ತುಂಬು_raw(splash_location, bmp_ತುಂಬು_addr);
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (splash_location->flags == SPLASH_STORAGE_FS)
		ಹಿಂತಿರುಗಿ splash_ತುಂಬು_fs(splash_location, bmp_ತುಂಬು_addr);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_FIT
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (splash_location->flags == SPLASH_STORAGE_FIT)
		ಹಿಂತಿರುಗಿ splash_ತುಂಬು_fit(splash_location, bmp_ತುಂಬು_addr);
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ -EINVAL;
}
