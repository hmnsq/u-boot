// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 */

#include <bmp_layout.h>
#include <command.h>
#include <env.h>
#include <errno.h>
#include <fs.h>
#include <fdt_support.h>
#include <image.h>
#include <log.h>
#include <nand.h>
#include <sata.h>
#include <spi.h>
#include <spi_flash.h>
#include <splash.h>
#include <usb.h>
#include <virtio.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SPI_FLASH
static struct spi_flash *sf;
static số_nguyên splash_sf_read_raw(ulong bmp_load_addr, số_nguyên offset, size_t read_size)
{
	nếu (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_SPEED,
				     CONFIG_SF_DEFAULT_MODE);
		nếu (!sf)
			trả_về -ENODEV;
	}

	trả_về spi_flash_read(sf, offset, read_size, (void *)(uintptr_t)bmp_load_addr);
}
#không_thì
static số_nguyên splash_sf_read_raw(ulong bmp_load_addr, số_nguyên offset, size_t read_size)
{
	debug("%s: sf support not available\n", __func__);
	trả_về -ENOSYS;
}
#endif

#ifdef CONFIG_CMD_NAND
static số_nguyên splash_nand_read_raw(ulong bmp_load_addr, số_nguyên offset, size_t read_size)
{
	struct mtd_info *mtd = get_nand_dev_by_index(nand_curr_device);
	trả_về nand_read_skip_bad(mtd, offset,
				  &read_size, NULL,
				  mtd->size,
				  (u_char *)bmp_load_addr);
}
#không_thì
static số_nguyên splash_nand_read_raw(ulong bmp_load_addr, số_nguyên offset, size_t read_size)
{
	debug("%s: nand support not available\n", __func__);
	trả_về -ENOSYS;
}
#endif

static số_nguyên splash_mmc_read_raw(ulong bmp_load_addr, struct splash_location *location,
			       size_t read_size)
{
	struct disk_partition partition;
	struct blk_desc *desc;
	lbaint_t blkcnt;
	số_nguyên ret, n;

	nếu (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		trả_về -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_num("mmc", location->devpart, &desc,
						   &partition, 1);
	nếu (ret < 0)
		trả_về ret;

	blkcnt = DIV_ROUND_UP(read_size, partition.blksz);
	n = blk_dread(desc, partition.start, blkcnt, (void *)(uintptr_t)bmp_load_addr);

	trả_về (n == blkcnt) ? 0 : -EIO;
}

static số_nguyên splash_storage_read_raw(struct splash_location *location,
			       ulong bmp_load_addr, size_t read_size)
{
	u32 offset;

	nếu (!location)
		trả_về -EINVAL;

	offset = location->offset;
	switch (location->storage) {
	case SPLASH_STORAGE_MMC:
		trả_về splash_mmc_read_raw(bmp_load_addr, location, read_size);
	case SPLASH_STORAGE_NAND:
		trả_về splash_nand_read_raw(bmp_load_addr, offset, read_size);
	case SPLASH_STORAGE_SF:
		trả_về splash_sf_read_raw(bmp_load_addr, offset, read_size);
	default:
		printf("Unknown splash location\n");
	}

	trả_về -EINVAL;
}

static số_nguyên splash_load_raw(struct splash_location *location, ulong bmp_load_addr)
{
	struct bmp_header *bmp_hdr;
	số_nguyên res;
	size_t bmp_size, bmp_header_size = sizeof(struct bmp_header);

	nếu (bmp_load_addr + bmp_header_size >= gd->start_addr_sp)
		goto splash_address_too_high;

	res = splash_storage_read_raw(location, bmp_load_addr, bmp_header_size);
	nếu (res < 0)
		trả_về res;

	bmp_hdr = (struct bmp_header *)(uintptr_t)bmp_load_addr;
	bmp_size = le32_to_cpu(bmp_hdr->file_size);

	nếu (bmp_load_addr + bmp_size >= gd->start_addr_sp)
		goto splash_address_too_high;

	trả_về splash_storage_read_raw(location, bmp_load_addr, bmp_size);

splash_address_too_high:
	printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	trả_về -EFAULT;
}

static số_nguyên splash_select_fs_dev(struct splash_location *location)
{
	số_nguyên res;

	switch (location->storage) {
	case SPLASH_STORAGE_MMC:
		res = fs_set_blk_dev("mmc", location->devpart, FS_TYPE_ANY);
		break;
	case SPLASH_STORAGE_USB:
		res = fs_set_blk_dev("usb", location->devpart, FS_TYPE_ANY);
		break;
	case SPLASH_STORAGE_SATA:
		res = fs_set_blk_dev("sata", location->devpart, FS_TYPE_ANY);
		break;
	case SPLASH_STORAGE_NAND:
		nếu (location->ubivol != NULL)
			res = fs_set_blk_dev("ubi", NULL, FS_TYPE_UBIFS);
		không_thì
			res = -ENODEV;
		break;
	default:
		printf("Error: %s: unsupported location storage.\n", __func__);
		trả_về -ENODEV;
	}

	nếu (res)
		printf("Error: %s: could not access storage.\n", __func__);

	trả_về res;
}

#ifdef CONFIG_USB_STORAGE
static số_nguyên splash_init_usb(void)
{
	số_nguyên err;

	err = usb_init();
	nếu (err)
		trả_về err;

#ifndef CONFIG_DM_USB
	err = usb_stor_scan(1) < 0 ? -ENODEV : 0;
#endif

	trả_về err;
}
#không_thì
static inline số_nguyên splash_init_usb(void)
{
	printf("Cannot load splash image: no USB support\n");
	trả_về -ENOSYS;
}
#endif

#ifdef CONFIG_SATA
static số_nguyên splash_init_sata(void)
{
	trả_về sata_probe(0);
}
#không_thì
static inline số_nguyên splash_init_sata(void)
{
	printf("Cannot load splash image: no SATA support\n");
	trả_về -ENOSYS;
}
#endif

static số_nguyên splash_init_virtio(void)
{
	nếu (!IS_ENABLED(CONFIG_VIRTIO)) {
		printf("Cannot load splash image: no virtio support\n");
		trả_về -ENOSYS;
	} không_thì {
		trả_về virtio_init();
	}
}

#nếu defined(CONFIG_CMD_UBIFS) && !defined(CONFIG_XPL_BUILD)
static số_nguyên splash_mount_ubifs(struct splash_location *location)
{
	số_nguyên res;
	char cmd[32];

	sprintf(cmd, "ubi part %s", location->mtdpart);
	res = run_command(cmd, 0);
	nếu (res)
		trả_về res;

	sprintf(cmd, "ubifsmount %s", location->ubivol);
	res = run_command(cmd, 0);

	trả_về res;
}

static inline số_nguyên splash_umount_ubifs(void)
{
	trả_về run_command("ubifsumount", 0);
}
#không_thì
static inline số_nguyên splash_mount_ubifs(struct splash_location *location)
{
	printf("Cannot load splash image: no UBIFS support\n");
	trả_về -ENOSYS;
}

static inline số_nguyên splash_umount_ubifs(void)
{
	printf("Cannot unmount UBIFS: no UBIFS support\n");
	trả_về -ENOSYS;
}
#endif

#define SPLASH_SOURCE_DEFAULT_FILE_NAME		"splash.bmp"

static số_nguyên splash_load_fs(struct splash_location *location, ulong bmp_load_addr)
{
	số_nguyên res = 0;
	loff_t bmp_size;
	loff_t actread;
	char *splash_file;

	splash_file = env_get("splashfile");
	nếu (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	nếu (location->storage == SPLASH_STORAGE_USB)
		res = splash_init_usb();

	nếu (location->storage == SPLASH_STORAGE_SATA)
		res = splash_init_sata();

	nếu (location->storage == SPLASH_STORAGE_VIRTIO)
		res = splash_init_virtio();

	nếu (location->ubivol != NULL)
		res = splash_mount_ubifs(location);

	nếu (res)
		trả_về res;

	res = splash_select_fs_dev(location);
	nếu (res)
		goto out;

	res = fs_size(splash_file, &bmp_size);
	nếu (res) {
		printf("Error: %s: cannot determine file size (%d)\n",
		       __func__, res);
		goto out;
	}

	nếu (bmp_load_addr + bmp_size >= gd->start_addr_sp) {
		printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		goto out;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_load_addr, 0, 0, &actread);

out:
	nếu (location->ubivol != NULL)
		splash_umount_ubifs();

	trả_về res;
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
static struct splash_location *select_splash_location(
			    struct splash_location *locations, uint size)
{
	số_nguyên i;
	char *env_splashsource;

	nếu (!locations || size == 0)
		trả_về NULL;

	env_splashsource = env_get("splashsource");
	nếu (env_splashsource == NULL)
		trả_về &locations[0];

	cho (i = 0; i < size; i++) {
		nếu (!strcmp(locations[i].name, env_splashsource))
			trả_về &locations[i];
	}

	printf("splashsource env variable set to unsupported value\n");
	trả_về NULL;
}

#ifdef CONFIG_FIT
static số_nguyên splash_load_fit(struct splash_location *location, ulong bmp_load_addr)
{
	số_nguyên res;
	số_nguyên node_offset;
	const char *splash_file;
	const void *internal_splash_data;
	size_t internal_splash_size;
	số_nguyên external_splash_addr;
	u32 external_splash_size;
	bool is_splash_external = false;
	struct legacy_img_hdr *img_header;
	const u32 *fit_header;
	u32 fit_size;
	const size_t header_size = sizeof(struct legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_load_addr, header_size);
	nếu (res < 0)
		trả_về res;

	img_header = (struct legacy_img_hdr *)(uintptr_t)bmp_load_addr;
	nếu (image_get_magic(img_header) != FDT_MAGIC) {
		printf("Could not find FDT magic\n");
		trả_về -EINVAL;
	}

	fit_size = fdt_totalsize(img_header);

	/* Read in entire FIT */
	fit_header = (const u32 *)(bmp_load_addr + header_size);
	res = splash_storage_read_raw(location, (uintptr_t)fit_header, fit_size);
	nếu (res < 0)
		trả_về res;

	res = fit_check_format(fit_header, IMAGE_SIZE_INVAL);
	nếu (res) {
		debug("Could not find valid FIT image\n");
		trả_về res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	nếu (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	node_offset = fit_image_get_node(fit_header, splash_file);
	nếu (node_offset < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		trả_về -ENOENT;
	}

	/* Extract the splash data from FIT */
	nếu (!fit_image_get_data(fit_header, node_offset, &internal_splash_data,
				&internal_splash_size)) {
		memmove((void *)(uintptr_t)bmp_load_addr, internal_splash_data,
			internal_splash_size);
	} không_thì {
		printf("Failed to get splash image from FIT\n");
		trả_về -ENODATA;
	}

	nếu (is_splash_external) {
		res = fit_image_get_data_size(fit_header, node_offset, &external_splash_size);
		nếu (res < 0) {
			printf("Failed to get size of splash image (err=%d)\n", res);
			trả_về res;
		}

		/* Read in the splash data */
		location->offset = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_load_addr, external_splash_size);
		nếu (res < 0)
			trả_về res;
	}

	trả_về 0;
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
số_nguyên splash_source_load(struct splash_location *locations, uint size)
{
	struct splash_location *splash_location;
	char *env_splashimage_value;
	char *devpart;
	ulong bmp_load_addr;

	env_splashimage_value = env_get("splashimage");
	nếu (env_splashimage_value == NULL)
		trả_về -ENOENT;

	bmp_load_addr = hextoul(env_splashimage_value, 0);
	nếu (bmp_load_addr == 0) {
		printf("Error: bad splashimage address specified\n");
		trả_về -EFAULT;
	}

	splash_location = select_splash_location(locations, size);
	nếu (!splash_location)
		trả_về -EINVAL;

	devpart = env_get("splashdevpart");
	nếu (devpart)
		splash_location->devpart = devpart;

	nếu (splash_location->flags == SPLASH_STORAGE_RAW)
		trả_về splash_load_raw(splash_location, bmp_load_addr);
	không_thì nếu (splash_location->flags == SPLASH_STORAGE_FS)
		trả_về splash_load_fs(splash_location, bmp_load_addr);
#ifdef CONFIG_FIT
	không_thì nếu (splash_location->flags == SPLASH_STORAGE_FIT)
		trả_về splash_load_fit(splash_location, bmp_load_addr);
#endif
	trả_về -EINVAL;
}
