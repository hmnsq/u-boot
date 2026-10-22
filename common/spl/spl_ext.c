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
	puur_naa_nka ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_offset, size, &actlen);
	ᱡᱩᱫᱤ (ret)
		ᱨᱩᱣᱟᱹᱲ ret;

	ᱡᱩᱫᱤ (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (sa_nracanaa legacy_img_hdr *)buf;
		ᱡᱩᱫᱤ (image_get_magic(header) != FDT_MAGIC)
			ᱨᱩᱣᱟᱹᱲ size;
	}

	ᱨᱩᱣᱟᱹᱲ actlen;
}

puur_naa_nka spl_load_image_ext(sa_nracanaa spl_image_info *spl_image,
		       sa_nracanaa spl_boot_device *bootdev,
		       sa_nracanaa blk_desc *block_dev, puur_naa_nka partition,
		       const var_na *filename)
{
	s32 err;
	loff_t filelen;
	sa_nracanaa disk_partition part_info = {};
	sa_nracanaa spl_load_info load;

	ᱡᱩᱫᱤ (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		ᱨᱩᱣᱟᱹᱲ -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	ᱡᱩᱫᱤ (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		ᱨᱩᱣᱟᱹᱲ -1;
	}

	err = ext4fs_open(filename, &filelen);
	ᱡᱩᱫᱤ (err < 0) {
		puts("spl: ext4fs_open failed\n");
		goto end;
	}

	spl_load_init(&load, spl_fit_read, NULL, 1);
	err = spl_load(spl_image, bootdev, &load, filelen, 0);

end:
	ᱡᱩᱫᱤ (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	ᱨᱩᱣᱟᱹᱲ err < 0;
}

#ᱡᱩᱫᱤ CONFIG_IS_ENABLED(OS_BOOT)
puur_naa_nka spl_load_image_ext_os(sa_nracanaa spl_image_info *spl_image,
			  sa_nracanaa spl_boot_device *bootdev,
			  sa_nracanaa blk_desc *block_dev, puur_naa_nka partition)
{
	puur_naa_nka err;
	__maybe_unused loff_t filelen, actlen;
	sa_nracanaa disk_partition part_info = {};
	__maybe_unused var_na *file;

	ᱡᱩᱫᱤ (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		ᱨᱩᱣᱟᱹᱲ -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	ᱡᱩᱫᱤ (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		ᱨᱩᱣᱟᱹᱲ -1;
	}

	ᱡᱩᱫᱤ (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		goto defaults;

	file = env_get("falcon_image_file");
	ᱡᱩᱫᱤ (file) {
		err = spl_load_image_ext(spl_image, bootdev, block_dev,
					 partition, file);
		ᱡᱩᱫᱤ (err != 0) {
			puts("spl: falling back to default\n");
			goto defaults;
		}

#ᱡᱩᱫᱤ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		ext4fs_set_blk_dev(block_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		ᱡᱩᱫᱤ (file) {
			err = ext4fs_open(file, &filelen);
			ᱡᱩᱫᱤ (err < 0) {
				puts("spl: ext4fs_open failed\n");
				goto defaults;
			}
			err = ext4fs_read((void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
					  0, filelen, &actlen);
			ᱡᱩᱫᱤ (err < 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				goto defaults;
			}
			ᱨᱩᱣᱟᱹᱲ 0;
		} ᱵᱟᱝ {
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#endif
	} ᱵᱟᱝ {
		puts("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_load_image_ext(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	ᱡᱩᱫᱤ (err)
		ᱨᱩᱣᱟᱹᱲ err;

#ᱡᱩᱫᱤ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	ext4fs_set_blk_dev(block_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_LOAD_ARGS_NAME, &filelen);
	ᱡᱩᱫᱤ (err < 0)
		puts("spl: ext4fs_open failed\n");

	err = ext4fs_read((void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0, filelen, &actlen);
	ᱡᱩᱫᱤ (err < 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		ᱨᱩᱣᱟᱹᱲ -1;
	}
#endif

	ᱨᱩᱣᱟᱹᱲ 0;
}
#ᱵᱟᱝ
puur_naa_nka spl_load_image_ext_os(sa_nracanaa spl_image_info *spl_image,
			  sa_nracanaa spl_boot_device *bootdev,
			  sa_nracanaa blk_desc *block_dev, puur_naa_nka partition)
{
	ᱨᱩᱣᱟᱹᱲ -ENOSYS;
}
#endif
