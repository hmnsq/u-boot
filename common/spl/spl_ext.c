// SPDX-License-Identifier: GPL-2.0+

#include <env.h>
#include <part.h>
#include <spl.h>
#include <spl_load.h>
#include <ext4fs.h>
#include <errno.h>
#include <image.h>

static ulong spl_fit_read(struct spl_load_info *load, ulong file_offset,
			  ulong size, void *buf)
{
	struct legacy_img_hdr *header;
	ціле ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_offset, size, &actlen);
	якщо (ret)
		повернути ret;

	якщо (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (struct legacy_img_hdr *)buf;
		якщо (image_get_magic(header) != FDT_MAGIC)
			повернути size;
	}

	повернути actlen;
}

ціле spl_load_image_ext(struct spl_image_info *spl_image,
		       struct spl_boot_device *bootdev,
		       struct blk_desc *block_dev, ціле partition,
		       const char *filename)
{
	s32 err;
	loff_t filelen;
	struct disk_partition part_info = {};
	struct spl_load_info load;

	якщо (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		повернути -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	якщо (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		повернути -1;
	}

	err = ext4fs_open(filename, &filelen);
	якщо (err < 0) {
		puts("spl: ext4fs_open failed\n");
		goto end;
	}

	spl_load_init(&load, spl_fit_read, NULL, 1);
	err = spl_load(spl_image, bootdev, &load, filelen, 0);

end:
	якщо (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	повернути err < 0;
}

#якщо CONFIG_IS_ENABLED(OS_BOOT)
ціле spl_load_image_ext_os(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev,
			  struct blk_desc *block_dev, ціле partition)
{
	ціле err;
	__maybe_unused loff_t filelen, actlen;
	struct disk_partition part_info = {};
	__maybe_unused char *file;

	якщо (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		повернути -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	якщо (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		повернути -1;
	}

	якщо (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		goto defaults;

	file = env_get("falcon_image_file");
	якщо (file) {
		err = spl_load_image_ext(spl_image, bootdev, block_dev,
					 partition, file);
		якщо (err != 0) {
			puts("spl: falling back to default\n");
			goto defaults;
		}

#якщо IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		ext4fs_set_blk_dev(block_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		якщо (file) {
			err = ext4fs_open(file, &filelen);
			якщо (err < 0) {
				puts("spl: ext4fs_open failed\n");
				goto defaults;
			}
			err = ext4fs_read((void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
					  0, filelen, &actlen);
			якщо (err < 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				goto defaults;
			}
			повернути 0;
		} інакше {
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#endif
	} інакше {
		puts("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_load_image_ext(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	якщо (err)
		повернути err;

#якщо IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	ext4fs_set_blk_dev(block_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_LOAD_ARGS_NAME, &filelen);
	якщо (err < 0)
		puts("spl: ext4fs_open failed\n");

	err = ext4fs_read((void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0, filelen, &actlen);
	якщо (err < 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		повернути -1;
	}
#endif

	повернути 0;
}
#інакше
ціле spl_load_image_ext_os(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev,
			  struct blk_desc *block_dev, ціле partition)
{
	повернути -ENOSYS;
}
#endif
