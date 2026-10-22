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
	整数 ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_offset, size, &actlen);
	如果 (ret)
		返回 ret;

	如果 (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (struct legacy_img_hdr *)buf;
		如果 (image_get_magic(header) != FDT_MAGIC)
			返回 size;
	}

	返回 actlen;
}

整数 spl_load_image_ext(struct spl_image_info *spl_image,
		       struct spl_boot_device *bootdev,
		       struct blk_desc *block_dev, 整数 partition,
		       const char *filename)
{
	s32 err;
	loff_t filelen;
	struct disk_partition part_info = {};
	struct spl_load_info load;

	如果 (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		返回 -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	如果 (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		返回 -1;
	}

	err = ext4fs_open(filename, &filelen);
	如果 (err < 0) {
		puts("spl: ext4fs_open failed\n");
		goto end;
	}

	spl_load_init(&load, spl_fit_read, NULL, 1);
	err = spl_load(spl_image, bootdev, &load, filelen, 0);

end:
	如果 (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	返回 err < 0;
}

#如果 CONFIG_IS_ENABLED(OS_BOOT)
整数 spl_load_image_ext_os(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev,
			  struct blk_desc *block_dev, 整数 partition)
{
	整数 err;
	__maybe_unused loff_t filelen, actlen;
	struct disk_partition part_info = {};
	__maybe_unused char *file;

	如果 (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		返回 -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	如果 (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		返回 -1;
	}

	如果 (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		goto defaults;

	file = env_get("falcon_image_file");
	如果 (file) {
		err = spl_load_image_ext(spl_image, bootdev, block_dev,
					 partition, file);
		如果 (err != 0) {
			puts("spl: falling back to default\n");
			goto defaults;
		}

#如果 IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		ext4fs_set_blk_dev(block_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		如果 (file) {
			err = ext4fs_open(file, &filelen);
			如果 (err < 0) {
				puts("spl: ext4fs_open failed\n");
				goto defaults;
			}
			err = ext4fs_read((void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
					  0, filelen, &actlen);
			如果 (err < 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				goto defaults;
			}
			返回 0;
		} 否则 {
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#endif
	} 否则 {
		puts("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_load_image_ext(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	如果 (err)
		返回 err;

#如果 IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	ext4fs_set_blk_dev(block_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_LOAD_ARGS_NAME, &filelen);
	如果 (err < 0)
		puts("spl: ext4fs_open failed\n");

	err = ext4fs_read((void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0, filelen, &actlen);
	如果 (err < 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		返回 -1;
	}
#endif

	返回 0;
}
#否则
整数 spl_load_image_ext_os(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev,
			  struct blk_desc *block_dev, 整数 partition)
{
	返回 -ENOSYS;
}
#endif
