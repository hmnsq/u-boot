// SPDX-License-Identifier: GPL-2.0+
/*
 * UPL handoff parsing
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ವ್ಯಾಖ್ಯಾನಿಸು LOG_CATEGORY UCLASS_BOOTSTD

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

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

ರಚನೆ upl s_upl;

ಶೂನ್ಯ upl_ಗಣ_fit_addr(ulong fit)
{
	ರಚನೆ upl *upl = &s_upl;

	upl->fit = fit;
}

ಶೂನ್ಯ upl_ಗಣ_fit_info(ulong fit, ಪೂರ್ಣಾಂಕ conf_ಸ್ಥಾನಾಂತರ, ulong ನಮೂದು_addr)
{
	ರಚನೆ upl *upl = &s_upl;

	upl->fit = fit;
	upl->conf_ಸ್ಥಾನಾಂತರ = conf_ಸ್ಥಾನಾಂತರ;
	log_debug("upl: add fit %lx conf %x\n", fit, conf_ಸ್ಥಾನಾಂತರ);
}

ಪೂರ್ಣಾಂಕ upl_ಕೂಡಿಸು_image_2(ಪೂರ್ಣಾಂಕ ಗಂಟು, ulong ತುಂಬು_addr, ulong ಗಾತ್ರ_2, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *desc)
{
	ರಚನೆ upl *upl = &s_upl;
	ರಚನೆ upl_image img;

	img.ತುಂಬು = ತುಂಬು_addr;
	img.ಗಾತ್ರ_2 = ಗಾತ್ರ_2;
	img.ಸ್ಥಾನಾಂತರ = ಗಂಟು;
	img.description = desc;
	ಆದರೆ (!alist_ಕೂಡಿಸು(&upl->image, img))
		ಹಿಂತಿರುಗಿ -ENOMEM;
	log_debug("upl: add image %s at %lx size %lx\n", desc, ತುಂಬು_addr, ಗಾತ್ರ_2);

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ write_serial(ರಚನೆ upl_serial *ser)
{
	ರಚನೆ udevice *dev = gd->cur_serial_dev;
	ರಚನೆ serial_device_info info;
	ರಚನೆ memregion region;
	ಪೂರ್ಣಾಂಕ ret;

	ಆದರೆ (!dev)
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("ser", -ENOENT);
	ret = serial_getinfo(dev, &info);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("inf", ret);

	ser->compatible = ofnode_read_string(dev_ofnode(dev), "compatible");
	ser->clock_frequency = info.clock;
	ser->current_ವೇಗ = gd->baudrate;
	region.ತಳ = info.addr;
	region.ಗಾತ್ರ_2 = info.ಗಾತ್ರ_2;
	alist_init_struct(&ser->reg, ರಚನೆ memregion);
	ಆದರೆ (!alist_ಕೂಡಿಸು(&ser->reg, region))
		ಹಿಂತಿರುಗಿ -ENOMEM;
	ser->reg_io_ಸರಿಸು = info.reg_ಸರಿಸು;
	ser->reg_ಸ್ಥಾನಾಂತರ = info.reg_ಸ್ಥಾನಾಂತರ;
	ser->reg_io_ಅಗಲ = info.reg_ಅಗಲ;
	ser->virtual_reg = 0;
	ser->access_type = info.addr_space;

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ write_graphics(ರಚನೆ upl_graphics *gra)
{
	ರಚನೆ video_uc_plat *plat;
	ರಚನೆ video_priv *priv;
	ರಚನೆ memregion region;
	ರಚನೆ udevice *dev;

	alist_init_struct(&gra->reg, ರಚನೆ memregion);
	uclass_ಕಂಡುಹಿಡಿ_ಮೊದಲ_device(UCLASS_VIDEO, &dev);
	ಆದರೆ (!dev || !device_ಸಕ್ರಿಯ(dev))
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("vid", -ENOENT);

	plat = dev_get_uclass_plat(dev);
	region.ತಳ = plat->ತಳ;
	region.ಗಾತ್ರ_2 = plat->ಗಾತ್ರ_2;
	ಆದರೆ (!alist_ಕೂಡಿಸು(&gra->reg, region))
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("reg", -ENOMEM);

	priv = dev_get_uclass_priv(dev);
	gra->ಅಗಲ = priv->xsize;
	gra->ಎತ್ತರ = priv->ysize;
	gra->stride = priv->ಸಾಲು_ಉದ್ದಳತೆ;	/* private field */
	ಆಯ್ಕೆ (priv->format) {
	ಸ್ಥಿತಿ VIDEO_RGBA8888:
	ಸ್ಥಿತಿ VIDEO_X8R8G8B8:
		gra->format = UPLGF_ARGB32;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ VIDEO_X8B8G8R8:
		gra->format = UPLGF_ABGR32;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ VIDEO_X2R10G10B10:
		log_debug("device '%s': VIDEO_X2R10G10B10 not supported\n",
			  dev->name);
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("for", -EPROTO);
	ಸ್ಥಿತಿ VIDEO_UNKNOWN:
		log_debug("device '%s': Unknown video format\n", dev->name);
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("for", -EPROTO);
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ spl_write_upl_handoff(ರಚನೆ spl_image_info *spl_image)
{
	ರಚನೆ upl *upl = &s_upl;
	ರಚನೆ abuf buf;
	ofnode ಬೇರು;
	ಶೂನ್ಯ *ptr;
	ಪೂರ್ಣಾಂಕ ret;

	log_debug("UPL: Writing handoff - image_count=%d\n", upl->image.ಎಣಿಕೆ_2);
	upl->addr_cells = IS_ENABLED(CONFIG_PHYS_64_ಬಿಟ್) ? 2 : 1;
	upl->ಗಾತ್ರ_cells = IS_ENABLED(CONFIG_PHYS_64_ಬಿಟ್) ? 2 : 1;
	upl->bootmode = UPLBM_DEFAULT;
	ret = write_serial(&upl->serial);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("ser", ret);
	ret = write_graphics(&upl->graphics);
	ಆದರೆ (ret && ret != -ENOENT)
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("gra", ret);

	ಬೇರು = ofnode_ಬೇರು();
	ret = upl_write_handoff(upl, ಬೇರು, true);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("wr", ret);

	ret = oftree_ಗಮ್ಯ_fdt(oftree_default(), &buf);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("fdt", ret);
	log_debug("FDT size %zx\n", abuf_ಗಾತ್ರ(&buf));

	ptr = bloblist_ಕೂಡಿಸು(BLOBLISTT_CONTROL_FDT, abuf_ಗಾತ್ರ(&buf), 0);
	ಆದರೆ (!ptr)
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("blo", -ENOENT);
	ಸ್ಮರಣೆ_ನಕಲು(ptr, abuf_ದತ್ತಾಂಶ(&buf), abuf_ಗಾತ್ರ(&buf));

	ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ spl_upl_init(ಶೂನ್ಯ)
{
	upl_init(&s_upl);
}
