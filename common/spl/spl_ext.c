// SPDX-License-Identifier: GPL-2.0+

#ضمن <env.h>
#ضمن <part.h>
#ضمن <spl.h>
#ضمن <spl_load.h>
#ضمن <ext4fs.h>
#ضمن <errno.h>
#ضمن <image.h>

ساكن ulong spl_fit_read(بنية spl_لدو_info *لدو, ulong file_وتھ,
			  ulong ناپ, فراغ *buf)
{
	بنية legacy_img_hdr *سرخی;
	صحيح ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_وتھ, ناپ, &actlen);
	إذا (ret)
		أرجع ret;

	إذا (CONFIG_IS_ENABLED(OS_BOOT)) {
		سرخی = (بنية legacy_img_hdr *)buf;
		إذا (image_get_magic(سرخی) != FDT_MAGIC)
			أرجع ناپ;
	}

	أرجع actlen;
}

صحيح spl_لدو_image_ext(بنية spl_image_info *spl_image,
		       بنية spl_boot_device *bootdev,
		       بنية blk_desc *بلاک_dev, صحيح حصے_کرو,
		       ثابت حرف *filename)
{
	s32 err;
	loff_t filelen;
	بنية disk_حصے_کرو part_info = {};
	بنية spl_لدو_info لدو;

	إذا (part_get_info(بلاک_dev, حصے_کرو, &part_info)) {
		لکھو("spl: no partition table found\n");
		أرجع -1;
	}

	ext4fs_سیٹ_blk_dev(بلاک_dev, &part_info);

	err = ext4fs_mount();
	إذا (!err) {
		لکھو("%s: ext4fs mount err - %d\n", __func__, err);
		أرجع -1;
	}

	err = ext4fs_open(filename, &filelen);
	إذا (err < 0) {
		puts("spl: ext4fs_open failed\n");
		اذهب سرا_2;
	}

	spl_لدو_init(&لدو, spl_fit_read, عدم, 1);
	err = spl_لدو(spl_image, bootdev, &لدو, filelen, 0);

سرا_2:
	إذا (err < 0)
		لکھو("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	أرجع err < 0;
}

#إذا CONFIG_IS_ENABLED(OS_BOOT)
صحيح spl_لدو_image_ext_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية blk_desc *بلاک_dev, صحيح حصے_کرو)
{
	صحيح err;
	__maybe_unused loff_t filelen, actlen;
	بنية disk_حصے_کرو part_info = {};
	__maybe_unused حرف *file;

	إذا (part_get_info(بلاک_dev, حصے_کرو, &part_info)) {
		لکھو("spl: no partition table found\n");
		أرجع -1;
	}

	ext4fs_سیٹ_blk_dev(بلاک_dev, &part_info);

	err = ext4fs_mount();
	إذا (!err) {
		لکھو("%s: ext4fs mount err - %d\n", __func__, err);
		أرجع -1;
	}

	إذا (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		اذهب defaults;

	file = env_get("falcon_image_file");
	إذا (file) {
		err = spl_لدو_image_ext(spl_image, bootdev, بلاک_dev,
					 حصے_کرو, file);
		إذا (err != 0) {
			puts("spl: falling back to default\n");
			اذهب defaults;
		}

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_دلیلاں)
		ext4fs_سیٹ_blk_dev(بلاک_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		إذا (file) {
			err = ext4fs_open(file, &filelen);
			إذا (err < 0) {
				puts("spl: ext4fs_open failed\n");
				اذهب defaults;
			}
			err = ext4fs_read((فراغ *)CONFIG_SPL_PAYLOAD_دلیلاں_ADDR,
					  0, filelen, &actlen);
			إذا (err < 0) {
				لکھو("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				اذهب defaults;
			}
			أرجع 0;
		} وإلا {
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#جے_انت
	} وإلا {
		puts("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_لدو_image_ext(spl_image, bootdev, بلاک_dev, حصے_کرو,
				 CONFIG_SPL_FS_لدو_KERNEL_NAME);

	إذا (err)
		أرجع err;

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_دلیلاں)
	ext4fs_سیٹ_blk_dev(بلاک_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_لدو_دلیلاں_NAME, &filelen);
	إذا (err < 0)
		puts("spl: ext4fs_open failed\n");

	err = ext4fs_read((فراغ *)CONFIG_SPL_PAYLOAD_دلیلاں_ADDR, 0, filelen, &actlen);
	إذا (err < 0) {
		لکھو("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_لدو_دلیلاں_NAME, err);
		أرجع -1;
	}
#جے_انت

	أرجع 0;
}
#وإلا
صحيح spl_لدو_image_ext_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية blk_desc *بلاک_dev, صحيح حصے_کرو)
{
	أرجع -ENOSYS;
}
#جے_انت
