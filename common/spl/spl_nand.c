// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#include <config.h>
#include <fdt_support.h>
#include <image.h>
#include <imx_container.h>
#include <log.h>
#include <spl.h>
#include <spl_load.h>
#include <asm/io.h>
#include <mapmem.h>
#include <nand.h>
#include <linux/libfdt_env.h>
#include <fdt.h>

uint32_t __weak spl_nand_get_uboot_raw_page(void)
{
	ပြန်ပေး CONFIG_SYS_NAND_U_BOOT_OFFS;
}

#အကယ်၍ defined(CONFIG_SPL_NAND_RAW_ONLY)
static ကိန်းပြည့် spl_nand_load_image(sa_nracanaa spl_image_info *spl_image,
			sa_nracanaa spl_boot_device *bootdev)
{
	nand_init();

	printf("Loading U-Boot from 0x%08x (size 0x%08x) to 0x%08x\n",
	       CONFIG_SYS_NAND_U_BOOT_OFFS, CFG_SYS_NAND_U_BOOT_SIZE,
	       CFG_SYS_NAND_U_BOOT_DST);

	nand_spl_load_image(spl_nand_get_uboot_raw_page(),
			    CFG_SYS_NAND_U_BOOT_SIZE,
			    map_sysmem(CFG_SYS_NAND_U_BOOT_DST,
				       CFG_SYS_NAND_U_BOOT_SIZE));
	spl_set_header_raw_uboot(spl_image);
	nand_deselect();

	ပြန်ပေး 0;
}
#မဟုတ်ပါက

__weak u32 nand_spl_adjust_offset(u32 sector, u32 offs)
{
	ပြန်ပေး offs;
}

static ulong spl_nand_read(sa_nracanaa spl_load_info *load, ulong offs, ulong size,
			   void *dst)
{
	ကိန်းပြည့် err;
	ulong sector;

	debug("%s: offs %lx, size %lx, dst %p\n",
	      __func__, offs, size, dst);

	sector = *(ကိန်းပြည့် *)load->priv;
	offs = sector + nand_spl_adjust_offset(sector, offs - sector);
	err = nand_spl_load_image(offs, size, dst);
	spl_set_bl_len(load, nand_page_size());
	အကယ်၍ (err)
		ပြန်ပေး 0;

	ပြန်ပေး size;
}

static ကိန်းပြည့် spl_nand_load_element(sa_nracanaa spl_image_info *spl_image,
				 sa_nracanaa spl_boot_device *bootdev, ကိန်းပြည့် offset)
{
	sa_nracanaa spl_load_info load;

	spl_load_init(&load, spl_nand_read, &offset, 1);
	ပြန်ပေး spl_load(spl_image, bootdev, &load, 0, offset);
}

#အကယ်၍ IS_ENABLED(CONFIG_SPL_OS_BOOT)
static ကိန်းပြည့် spl_nand_load_image_os(sa_nracanaa spl_image_info *spl_image,
				  sa_nracanaa spl_boot_device *bootdev)
{
	ကိန်းပြည့် err;
	sa_nracanaa legacy_img_hdr *header = spl_get_load_buffer(0, parimaa_na(*header));

	/* load linux */
	nand_spl_load_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS, parimaa_na(*header),
			    (void *)header);
	err = spl_parse_image_header(spl_image, bootdev, header);
	အကယ်၍ (err)
		ပြန်ပေး err;

	အကယ်၍ (header->ih_os != IH_OS_LINUX)
		ပြန်ပေး -EINVAL;

	/* happy - was a linux */
	err = nand_spl_load_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS,
				  spl_image->size,
				  (void *)spl_image->load_addr);
	nand_deselect();

	အကယ်၍ (err)
		ပြန်ပေး err;

#အကယ်၍ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	/*
	 * load parameter image load to temp position since nand_spl_load_image
	 * reads a whole block which is typically larger than
	 * CONFIG_CMD_SPL_WRITE_SIZE therefore may overwrite following sections
	 * like BSS
	 */
	ကိန်းပြည့် *src, *dst;
	nand_spl_load_image(CONFIG_CMD_SPL_NAND_OFS, CONFIG_CMD_SPL_WRITE_SIZE,
			    (void *)CONFIG_TEXT_BASE);
	/* copy to destination */
	အတွက် (dst = (ကိန်းပြည့် *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
	    src = (ကိန်းပြည့် *)CONFIG_TEXT_BASE;
	     src < (ကိန်းပြည့် *)(CONFIG_TEXT_BASE + CONFIG_CMD_SPL_WRITE_SIZE);
	     src++, dst++) {
		writel(readl(src), dst);
	}
#endif

	ပြန်ပေး 0;
}
#endif

static ကိန်းပြည့် spl_nand_load_image(sa_nracanaa spl_image_info *spl_image,
			       sa_nracanaa spl_boot_device *bootdev)
{
	ကိန်းပြည့် err;

#ifdef CONFIG_SPL_NAND_SOFTECC
	debug("spl: nand - using sw ecc\n");
#မဟုတ်ပါက
	debug("spl: nand - using hw ecc\n");
#endif
	nand_init();

#အကယ်၍ CONFIG_IS_ENABLED(OS_BOOT)
	အကယ်၍ (!spl_start_uboot()) {
		err = spl_nand_load_image_os(spl_image, bootdev);
		အကယ်၍ (!err)
			ပြန်ပေး 0;
		printf("%s: Failed in falcon boot: %d", __func__, err);
		အကယ်၍ (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ပြန်ပေး err;
		printf("Fallback to U-Boot\n");
	}
#endif

#ifdef CONFIG_NAND_ENV_DST
	spl_nand_load_element(spl_image, bootdev, CONFIG_ENV_OFFSET);
#ifdef CONFIG_ENV_OFFSET_REDUND
	spl_nand_load_element(spl_image, bootdev, CONFIG_ENV_OFFSET_REDUND);
#endif
#endif
	/* Load u-boot */
	err = spl_nand_load_element(spl_image, bootdev, spl_nand_get_uboot_raw_page());
#ifdef CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
#အကယ်၍ CONFIG_SYS_NAND_U_BOOT_OFFS != CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
	အကယ်၍ (err)
		err = spl_nand_load_element(spl_image, bootdev,
					    CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND);
#endif
#endif
	nand_deselect();
	ပြန်ပေး err;
}
#endif
/* Use priorty 1 so that Ubi can override this */
SPL_LOAD_IMAGE_METHOD("NAND", 1, BOOT_DEVICE_NAND, spl_nand_load_image);
