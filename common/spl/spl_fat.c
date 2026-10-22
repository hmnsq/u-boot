// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * FAT Image Functions copied from spl_mmc.c
 */

#समावेश <env.h>
#समावेश <log.h>
#समावेश <spl.h>
#समावेश <spl_load.h>
#समावेश <fat.h>
#समावेश <errno.h>
#समावेश <image.h>
#समावेश <linux/libfdt.h>
#समावेश <asm/cache.h>

अटल पूर्णांक fat_registered;

व्योम spl_fat_force_reregister(व्योम)
{
	fat_registered = 0;
}

अटल पूर्णांक spl_register_fat_device(काष्ठा blk_desc *block_dev, पूर्णांक partition)
{
	पूर्णांक err = 0;

	अगर (fat_registered)
		वापस err;

	err = fat_register_device(block_dev, partition);
	अगर (err) {
		म_लिखो("%s: fat register err - %d\n", __func__, err);
		वापस err;
	}

	fat_registered = 1;

	वापस err;
}

अटल ulong spl_fit_read(काष्ठा spl_load_info *load, ulong file_offset,
			  ulong आकार, व्योम *buf)
{
	काष्ठा legacy_img_hdr *header;
	loff_t actread;
	पूर्णांक ret;
	अक्षर *filename = load->priv;

	ret = fat_read_file(filename, buf, file_offset, आकार, &actread);
	अगर (ret)
		वापस ret;

	अगर (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (काष्ठा legacy_img_hdr *)buf;
		अगर (image_get_magic(header) != FDT_MAGIC)
			वापस आकार;
	}

	वापस actread;
}

पूर्णांक spl_load_image_fat(काष्ठा spl_image_info *spl_image,
		       काष्ठा spl_boot_device *bootdev,
		       काष्ठा blk_desc *block_dev, पूर्णांक partition,
		       स्थिर अक्षर *filename)
{
	पूर्णांक err;
	loff_t आकार;
	काष्ठा spl_load_info load;

	err = spl_register_fat_device(block_dev, partition);
	अगर (err)
		जाओ end;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	अगर (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = fat_आकार(filename, &आकार);
		अगर (err)
			जाओ end;
	} अन्यथा {
		आकार = 0;
	}

	spl_load_init(&load, spl_fit_read, (व्योम *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_load(spl_image, bootdev, &load, आकार, 0);

end:
	अगर (err < 0) {
		spl_fat_force_reregister();
		म_लिखो("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	वापस err;
}

#अगर CONFIG_IS_ENABLED(OS_BOOT)
पूर्णांक spl_load_image_fat_os(काष्ठा spl_image_info *spl_image,
			  काष्ठा spl_boot_device *bootdev,
			  काष्ठा blk_desc *block_dev, पूर्णांक partition)
{
	पूर्णांक err;
	__maybe_unused अक्षर *file;

	err = spl_register_fat_device(block_dev, partition);
	अगर (err)
		वापस err;

	अगर (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		जाओ defaults;

	file = env_get("falcon_image_file");
	अगर (file) {
		err = spl_load_image_fat(spl_image, bootdev, block_dev,
					 partition, file);
		अगर (err != 0) {
			माला_दो("spl: falling back to default\n");
			जाओ defaults;
		}

#अगर IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		file = env_get("falcon_args_file");
		अगर (file) {
			err = file_fat_read(
				file, (व्योम *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
			अगर (err <= 0) {
				म_लिखो("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				जाओ defaults;
			}
			वापस 0;
		} अन्यथा
			माला_दो("spl: falcon_args_file not set in environment, falling back to default\n");
#पूर्ण_अगर

	} अन्यथा
		माला_दो("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_load_image_fat(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	अगर (err)
		वापस err;

#अगर IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	err = file_fat_read(CONFIG_SPL_FS_LOAD_ARGS_NAME,
			    (व्योम *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
	अगर (err <= 0) {
		म_लिखो("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		वापस -1;
	}
#पूर्ण_अगर

	वापस 0;
}
#अन्यथा
पूर्णांक spl_load_image_fat_os(काष्ठा spl_image_info *spl_image,
			  काष्ठा spl_boot_device *bootdev,
			  काष्ठा blk_desc *block_dev, पूर्णांक partition)
{
	वापस -ENOSYS;
}
#पूर्ण_अगर
