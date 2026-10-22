// SPDX-License-Identifier: GPL-2.0+
/*
 * UPL handoff parsing
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#ಅನ್ತರ್ಭಾವ <alist.h>
#ಅನ್ತರ್ಭಾವ <bloblist.h>
#ಅನ್ತರ್ಭಾವ <dm.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <serial.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <upl.h>
#ಅನ್ತರ್ಭಾವ <video.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <dm/read.h>
#ಅನ್ತರ್ಭಾವ <dm/uclass-internal.h>

DECLARE_GLOBAL_DATA_PTR;

ಸಂರಚನಾ upl s_upl;

ಶೂನ್ಯ upl_set_fit_addr(ulong fit)
{
	ಸಂರಚನಾ upl *upl = &s_upl;

	upl->fit = fit;
}

ಶೂನ್ಯ upl_set_fit_info(ulong fit, ಪೂರ್ಣಾಙ್ಕ conf_offset, ulong entry_addr)
{
	ಸಂರಚನಾ upl *upl = &s_upl;

	upl->fit = fit;
	upl->conf_offset = conf_offset;
	log_debug("upl: add fit %lx conf %x\n", fit, conf_offset);
}

ಪೂರ್ಣಾಙ್ಕ _upl_add_image(ಪೂರ್ಣಾಙ್ಕ ಗಂಟು, ulong load_addr, ulong size, ಧ್ರುವ ವರ್ಣ *desc)
{
	ಸಂರಚನಾ upl *upl = &s_upl;
	ಸಂರಚನಾ upl_image img;

	img.load = load_addr;
	img.size = size;
	img.offset = ಗಂಟು;
	img.description = desc;
	ಯದಿ (!alist_add(&upl->image, img))
		ಪ್ರತ್ಯಾಗಮ -ENOMEM;
	log_debug("upl: add image %s at %lx size %lx\n", desc, load_addr, size);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ write_serial(ಸಂರಚನಾ upl_serial *ser)
{
	ಸಂರಚನಾ udevice *dev = gd->cur_serial_dev;
	ಸಂರಚನಾ serial_device_info info;
	ಸಂರಚನಾ memregion region;
	ಪೂರ್ಣಾಙ್ಕ ret;

	ಯದಿ (!dev)
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("ser", -ENOENT);
	ret = serial_getinfo(dev, &info);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("inf", ret);

	ser->compatible = ofnode_read_string(dev_ofnode(dev), "compatible");
	ser->clock_frequency = info.clock;
	ser->current_speed = gd->baudrate;
	region.base = info.addr;
	region.size = info.size;
	alist_init_struct(&ser->reg, ಸಂರಚನಾ memregion);
	ಯದಿ (!alist_add(&ser->reg, region))
		ಪ್ರತ್ಯಾಗಮ -ENOMEM;
	ser->reg_io_shift = info.reg_shift;
	ser->reg_offset = info.reg_offset;
	ser->reg_io_width = info.reg_width;
	ser->virtual_reg = 0;
	ser->access_type = info.addr_space;

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ write_graphics(ಸಂರಚನಾ upl_graphics *gra)
{
	ಸಂರಚನಾ video_uc_plat *plat;
	ಸಂರಚನಾ video_priv *priv;
	ಸಂರಚನಾ memregion region;
	ಸಂರಚನಾ udevice *dev;

	alist_init_struct(&gra->reg, ಸಂರಚನಾ memregion);
	uclass_find_first_device(UCLASS_VIDEO, &dev);
	ಯದಿ (!dev || !device_active(dev))
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("vid", -ENOENT);

	plat = dev_get_uclass_plat(dev);
	region.base = plat->base;
	region.size = plat->size;
	ಯದಿ (!alist_add(&gra->reg, region))
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("reg", -ENOMEM);

	priv = dev_get_uclass_priv(dev);
	gra->width = priv->xsize;
	gra->height = priv->ysize;
	gra->stride = priv->ಸಾಲು_length;	/* private field */
	ವಿಕಲ್ಪ (priv->format) {
	ಪಕ್ಷ VIDEO_RGBA8888:
	ಪಕ್ಷ VIDEO_X8R8G8B8:
		gra->format = UPLGF_ARGB32;
		ಭಙ್ಗ;
	ಪಕ್ಷ VIDEO_X8B8G8R8:
		gra->format = UPLGF_ABGR32;
		ಭಙ್ಗ;
	ಪಕ್ಷ VIDEO_X2R10G10B10:
		log_debug("device '%s': VIDEO_X2R10G10B10 not supported\n",
			  dev->name);
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("for", -EPROTO);
	ಪಕ್ಷ VIDEO_UNKNOWN:
		log_debug("device '%s': Unknown video format\n", dev->name);
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("for", -EPROTO);
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ spl_write_upl_handoff(ಸಂರಚನಾ spl_image_info *spl_image)
{
	ಸಂರಚನಾ upl *upl = &s_upl;
	ಸಂರಚನಾ abuf buf;
	ofnode ಬೇರು;
	ಶೂನ್ಯ *ptr;
	ಪೂರ್ಣಾಙ್ಕ ret;

	log_debug("UPL: Writing handoff - image_count=%d\n", upl->image.ಎಣಿಕೆ);
	upl->addr_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->size_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->bootmode = UPLBM_DEFAULT;
	ret = write_serial(&upl->serial);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("ser", ret);
	ret = write_graphics(&upl->graphics);
	ಯದಿ (ret && ret != -ENOENT)
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("gra", ret);

	ಬೇರು = ofnode_ಬೇರು();
	ret = upl_write_handoff(upl, ಬೇರು, true);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("wr", ret);

	ret = oftree_to_fdt(oftree_default(), &buf);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("fdt", ret);
	log_debug("FDT size %zx\n", abuf_size(&buf));

	ptr = bloblist_add(BLOBLISTT_CONTROL_FDT, abuf_size(&buf), 0);
	ಯದಿ (!ptr)
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("blo", -ENOENT);
	memcpy(ptr, abuf_data(&buf), abuf_size(&buf));

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಶೂನ್ಯ spl_upl_init(ಶೂನ್ಯ)
{
	upl_init(&s_upl);
}
