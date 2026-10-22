// SPDX-License-Identifier: GPL-2.0+

#ضمن <env.h>
#ضمن <part.h>
#ضمن <spl.h>
#ضمن <spl_load.h>
#ضمن <ext4fs.h>
#ضمن <errno.h>
#ضمن <image.h>

ساكن ulong spl_fit_read(بنية spl_بارگیری_info *بارگیری, ulong file_جابه_جایی,
			  ulong اندازه, فراغ *buf)
{
	بنية legacy_img_hdr *سرآیند;
	صحيح ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_جابه_جایی, اندازه, &actlen);
	إذا (ret)
		أرجع ret;

	إذا (CONFIG_IS_ENABLED(OS_BOOT)) {
		سرآیند = (بنية legacy_img_hdr *)buf;
		إذا (image_get_magic(سرآیند) != FDT_MAGIC)
			أرجع اندازه;
	}

	أرجع actlen;
}

صحيح spl_بارگیری_image_ext(بنية spl_image_info *spl_image,
		       بنية spl_boot_device *bootdev,
		       بنية blk_desc *بلوک_dev, صحيح افراز,
		       ثابت حرف *filename)
{
	s32 err;
	loff_t filelen;
	بنية disk_افراز part_info = {};
	بنية spl_بارگیری_info بارگیری;

	إذا (part_get_info(بلوک_dev, افراز, &part_info)) {
		چاپ_کن("spl: no partition table found\n");
		أرجع -1;
	}

	ext4fs_مجموعه_blk_dev(بلوک_dev, &part_info);

	err = ext4fs_mount();
	إذا (!err) {
		چاپ_کن("%s: ext4fs mount err - %d\n", __func__, err);
		أرجع -1;
	}

	err = ext4fs_open(filename, &filelen);
	إذا (err < 0) {
		puts("spl: ext4fs_open failed\n");
		اذهب انتها_2;
	}

	spl_بارگیری_init(&بارگیری, spl_fit_read, عدم, 1);
	err = spl_بارگیری(spl_image, bootdev, &بارگیری, filelen, 0);

انتها_2:
	إذا (err < 0)
		چاپ_کن("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	أرجع err < 0;
}

#إذا CONFIG_IS_ENABLED(OS_BOOT)
صحيح spl_بارگیری_image_ext_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية blk_desc *بلوک_dev, صحيح افراز)
{
	صحيح err;
	__maybe_unused loff_t filelen, actlen;
	بنية disk_افراز part_info = {};
	__maybe_unused حرف *file;

	إذا (part_get_info(بلوک_dev, افراز, &part_info)) {
		چاپ_کن("spl: no partition table found\n");
		أرجع -1;
	}

	ext4fs_مجموعه_blk_dev(بلوک_dev, &part_info);

	err = ext4fs_mount();
	إذا (!err) {
		چاپ_کن("%s: ext4fs mount err - %d\n", __func__, err);
		أرجع -1;
	}

	إذا (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		اذهب defaults;

	file = env_get("falcon_image_file");
	إذا (file) {
		err = spl_بارگیری_image_ext(spl_image, bootdev, بلوک_dev,
					 افراز, file);
		إذا (err != 0) {
			puts("spl: falling back to default\n");
			اذهب defaults;
		}

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_آرگومانها)
		ext4fs_مجموعه_blk_dev(بلوک_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		إذا (file) {
			err = ext4fs_open(file, &filelen);
			إذا (err < 0) {
				puts("spl: ext4fs_open failed\n");
				اذهب defaults;
			}
			err = ext4fs_read((فراغ *)CONFIG_SPL_PAYLOAD_آرگومانها_ADDR,
					  0, filelen, &actlen);
			إذا (err < 0) {
				چاپ_کن("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				اذهب defaults;
			}
			أرجع 0;
		} وإلا {
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#پایان_اگر
	} وإلا {
		puts("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_بارگیری_image_ext(spl_image, bootdev, بلوک_dev, افراز,
				 CONFIG_SPL_FS_بارگیری_KERNEL_NAME);

	إذا (err)
		أرجع err;

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_آرگومانها)
	ext4fs_مجموعه_blk_dev(بلوک_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_بارگیری_آرگومانها_NAME, &filelen);
	إذا (err < 0)
		puts("spl: ext4fs_open failed\n");

	err = ext4fs_read((فراغ *)CONFIG_SPL_PAYLOAD_آرگومانها_ADDR, 0, filelen, &actlen);
	إذا (err < 0) {
		چاپ_کن("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_بارگیری_آرگومانها_NAME, err);
		أرجع -1;
	}
#پایان_اگر

	أرجع 0;
}
#وإلا
صحيح spl_بارگیری_image_ext_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية blk_desc *بلوک_dev, صحيح افراز)
{
	أرجع -ENOSYS;
}
#پایان_اگر
