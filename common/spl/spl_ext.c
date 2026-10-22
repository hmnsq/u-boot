// SPDX-License-Identifier: GPL-2.0+

#شامل <env.h>
#شامل <part.h>
#شامل <spl.h>
#شامل <spl_load.h>
#شامل <ext4fs.h>
#شامل <errno.h>
#شامل <image.h>

ساکن ulong spl_fit_read(ساخت spl_بھرو_info *بھرو, ulong file_انحراف,
			  ulong سائز, خالی *buf)
{
	ساخت legacy_img_hdr *سرخی;
	صحیح ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_انحراف, سائز, &actlen);
	اگر (ret)
		واپس ret;

	اگر (CONFIG_IS_ENABLED(OS_BOOT)) {
		سرخی = (ساخت legacy_img_hdr *)buf;
		اگر (image_get_magic(سرخی) != FDT_MAGIC)
			واپس سائز;
	}

	واپس actlen;
}

صحیح spl_بھرو_image_ext(ساخت spl_image_info *spl_image,
		       ساخت spl_boot_device *bootdev,
		       ساخت blk_desc *بلاک_dev, صحیح تقسیم,
		       ثابت حرف *filename)
{
	s32 err;
	loff_t filelen;
	ساخت disk_تقسیم part_info = {};
	ساخت spl_بھرو_info بھرو;

	اگر (part_get_info(بلاک_dev, تقسیم, &part_info)) {
		لکھو("spl: no partition table found\n");
		واپس -1;
	}

	ext4fs_سیٹ_blk_dev(بلاک_dev, &part_info);

	err = ext4fs_mount();
	اگر (!err) {
		لکھو("%s: ext4fs mount err - %d\n", __func__, err);
		واپس -1;
	}

	err = ext4fs_open(filename, &filelen);
	اگر (err < 0) {
		puts("spl: ext4fs_open failed\n");
		جاؤ انتہا_2;
	}

	spl_بھرو_init(&بھرو, spl_fit_read, تہی, 1);
	err = spl_بھرو(spl_image, bootdev, &بھرو, filelen, 0);

انتہا_2:
	اگر (err < 0)
		لکھو("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	واپس err < 0;
}

#اگر CONFIG_IS_ENABLED(OS_BOOT)
صحیح spl_بھرو_image_ext_os(ساخت spl_image_info *spl_image,
			  ساخت spl_boot_device *bootdev,
			  ساخت blk_desc *بلاک_dev, صحیح تقسیم)
{
	صحیح err;
	__maybe_unused loff_t filelen, actlen;
	ساخت disk_تقسیم part_info = {};
	__maybe_unused حرف *file;

	اگر (part_get_info(بلاک_dev, تقسیم, &part_info)) {
		لکھو("spl: no partition table found\n");
		واپس -1;
	}

	ext4fs_سیٹ_blk_dev(بلاک_dev, &part_info);

	err = ext4fs_mount();
	اگر (!err) {
		لکھو("%s: ext4fs mount err - %d\n", __func__, err);
		واپس -1;
	}

	اگر (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		جاؤ defaults;

	file = env_get("falcon_image_file");
	اگر (file) {
		err = spl_بھرو_image_ext(spl_image, bootdev, بلاک_dev,
					 تقسیم, file);
		اگر (err != 0) {
			puts("spl: falling back to default\n");
			جاؤ defaults;
		}

#اگر IS_ENABLED(CONFIG_SPL_OS_BOOT_دلائل)
		ext4fs_سیٹ_blk_dev(بلاک_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		اگر (file) {
			err = ext4fs_open(file, &filelen);
			اگر (err < 0) {
				puts("spl: ext4fs_open failed\n");
				جاؤ defaults;
			}
			err = ext4fs_read((خالی *)CONFIG_SPL_PAYLOAD_دلائل_ADDR,
					  0, filelen, &actlen);
			اگر (err < 0) {
				لکھو("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				جاؤ defaults;
			}
			واپس 0;
		} ورنہ {
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#اگر_ختم
	} ورنہ {
		puts("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_بھرو_image_ext(spl_image, bootdev, بلاک_dev, تقسیم,
				 CONFIG_SPL_FS_بھرو_KERNEL_NAME);

	اگر (err)
		واپس err;

#اگر IS_ENABLED(CONFIG_SPL_OS_BOOT_دلائل)
	ext4fs_سیٹ_blk_dev(بلاک_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_بھرو_دلائل_NAME, &filelen);
	اگر (err < 0)
		puts("spl: ext4fs_open failed\n");

	err = ext4fs_read((خالی *)CONFIG_SPL_PAYLOAD_دلائل_ADDR, 0, filelen, &actlen);
	اگر (err < 0) {
		لکھو("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_بھرو_دلائل_NAME, err);
		واپس -1;
	}
#اگر_ختم

	واپس 0;
}
#ورنہ
صحیح spl_بھرو_image_ext_os(ساخت spl_image_info *spl_image,
			  ساخت spl_boot_device *bootdev,
			  ساخت blk_desc *بلاک_dev, صحیح تقسیم)
{
	واپس -ENOSYS;
}
#اگر_ختم
