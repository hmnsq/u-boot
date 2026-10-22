// SPDX-License-Identifier: GPL-2.0+

#include <env.h>
#include <part.h>
#include <spl.h>
#include <spl_load.h>
#include <ext4fs.h>
#include <errno.h>
#include <image.h>

static ulong spl_fit_read(sa_nracanaa spl_load_info *load, ulong file_offset,
			  ulong size, void *buf)
{
	sa_nracanaa legacy_img_hdr *header;
	ကိန်းပြည့် ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_offset, size, &actlen);
	အကယ်၍ (ret)
		ပြန်ပေး ret;

	အကယ်၍ (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (sa_nracanaa legacy_img_hdr *)buf;
		အကယ်၍ (image_get_magic(header) != FDT_MAGIC)
			ပြန်ပေး size;
	}

	ပြန်ပေး actlen;
}

ကိန်းပြည့် spl_load_image_ext(sa_nracanaa spl_image_info *spl_image,
		       sa_nracanaa spl_boot_device *bootdev,
		       sa_nracanaa blk_desc *block_dev, ကိန်းပြည့် partition,
		       const var_na *filename)
{
	s32 err;
	loff_t filelen;
	sa_nracanaa disk_partition part_info = {};
	sa_nracanaa spl_load_info load;

	အကယ်၍ (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		ပြန်ပေး -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	အကယ်၍ (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		ပြန်ပေး -1;
	}

	err = ext4fs_open(filename, &filelen);
	အကယ်၍ (err < 0) {
		puts("spl: ext4fs_open failed\n");
		goto end;
	}

	spl_load_init(&load, spl_fit_read, NULL, 1);
	err = spl_load(spl_image, bootdev, &load, filelen, 0);

end:
	အကယ်၍ (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	ပြန်ပေး err < 0;
}

#အကယ်၍ CONFIG_IS_ENABLED(OS_BOOT)
ကိန်းပြည့် spl_load_image_ext_os(sa_nracanaa spl_image_info *spl_image,
			  sa_nracanaa spl_boot_device *bootdev,
			  sa_nracanaa blk_desc *block_dev, ကိန်းပြည့် partition)
{
	ကိန်းပြည့် err;
	__maybe_unused loff_t filelen, actlen;
	sa_nracanaa disk_partition part_info = {};
	__maybe_unused var_na *file;

	အကယ်၍ (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		ပြန်ပေး -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	အကယ်၍ (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		ပြန်ပေး -1;
	}

	အကယ်၍ (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		goto defaults;

	file = env_get("falcon_image_file");
	အကယ်၍ (file) {
		err = spl_load_image_ext(spl_image, bootdev, block_dev,
					 partition, file);
		အကယ်၍ (err != 0) {
			puts("spl: falling back to default\n");
			goto defaults;
		}

#အကယ်၍ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		ext4fs_set_blk_dev(block_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		အကယ်၍ (file) {
			err = ext4fs_open(file, &filelen);
			အကယ်၍ (err < 0) {
				puts("spl: ext4fs_open failed\n");
				goto defaults;
			}
			err = ext4fs_read((void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
					  0, filelen, &actlen);
			အကယ်၍ (err < 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				goto defaults;
			}
			ပြန်ပေး 0;
		} မဟုတ်ပါက {
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#endif
	} မဟုတ်ပါက {
		puts("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_load_image_ext(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	အကယ်၍ (err)
		ပြန်ပေး err;

#အကယ်၍ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	ext4fs_set_blk_dev(block_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_LOAD_ARGS_NAME, &filelen);
	အကယ်၍ (err < 0)
		puts("spl: ext4fs_open failed\n");

	err = ext4fs_read((void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0, filelen, &actlen);
	အကယ်၍ (err < 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		ပြန်ပေး -1;
	}
#endif

	ပြန်ပေး 0;
}
#မဟုတ်ပါက
ကိန်းပြည့် spl_load_image_ext_os(sa_nracanaa spl_image_info *spl_image,
			  sa_nracanaa spl_boot_device *bootdev,
			  sa_nracanaa blk_desc *block_dev, ကိန်းပြည့် partition)
{
	ပြန်ပေး -ENOSYS;
}
#endif
