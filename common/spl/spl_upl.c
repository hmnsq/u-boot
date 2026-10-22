// SPDX-License-Identifier: GPL-2.0+
/*
 * UPL handoff parsing
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#அந்தர்பாவ <alist.h>
#அந்தர்பாவ <bloblist.h>
#அந்தர்பாவ <dm.h>
#அந்தர்பாவ <image.h>
#அந்தர்பாவ <mapmem.h>
#அந்தர்பாவ <serial.h>
#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <upl.h>
#அந்தர்பாவ <video.h>
#அந்தர்பாவ <asm/global_data.h>
#அந்தர்பாவ <dm/read.h>
#அந்தர்பாவ <dm/uclass-internal.h>

DECLARE_GLOBAL_DATA_PTR;

கட்டமைப்பு upl s_upl;

வெறுமை upl_set_fit_addr(ulong fit)
{
	கட்டமைப்பு upl *upl = &s_upl;

	upl->fit = fit;
}

வெறுமை upl_set_fit_info(ulong fit, முழு conf_offset, ulong entry_addr)
{
	கட்டமைப்பு upl *upl = &s_upl;

	upl->fit = fit;
	upl->conf_offset = conf_offset;
	log_debug("upl: add fit %lx conf %x\n", fit, conf_offset);
}

முழு _upl_add_image(முழு முனை, ulong load_addr, ulong size, மாறிலி எழுத்து *desc)
{
	கட்டமைப்பு upl *upl = &s_upl;
	கட்டமைப்பு upl_image img;

	img.load = load_addr;
	img.size = size;
	img.offset = முனை;
	img.description = desc;
	எனில் (!alist_add(&upl->image, img))
		திரும்பு -ENOMEM;
	log_debug("upl: add image %s at %lx size %lx\n", desc, load_addr, size);

	திரும்பு 0;
}

நிலையான முழு write_serial(கட்டமைப்பு upl_serial *ser)
{
	கட்டமைப்பு udevice *dev = gd->cur_serial_dev;
	கட்டமைப்பு serial_device_info info;
	கட்டமைப்பு memregion region;
	முழு ret;

	எனில் (!dev)
		திரும்பு log_செய்தி_ret("ser", -ENOENT);
	ret = serial_getinfo(dev, &info);
	எனில் (ret)
		திரும்பு log_செய்தி_ret("inf", ret);

	ser->compatible = ofnode_read_string(dev_ofnode(dev), "compatible");
	ser->clock_frequency = info.clock;
	ser->current_speed = gd->baudrate;
	region.base = info.addr;
	region.size = info.size;
	alist_init_struct(&ser->reg, கட்டமைப்பு memregion);
	எனில் (!alist_add(&ser->reg, region))
		திரும்பு -ENOMEM;
	ser->reg_io_shift = info.reg_shift;
	ser->reg_offset = info.reg_offset;
	ser->reg_io_width = info.reg_width;
	ser->virtual_reg = 0;
	ser->access_type = info.addr_space;

	திரும்பு 0;
}

நிலையான முழு write_graphics(கட்டமைப்பு upl_graphics *gra)
{
	கட்டமைப்பு video_uc_plat *plat;
	கட்டமைப்பு video_priv *priv;
	கட்டமைப்பு memregion region;
	கட்டமைப்பு udevice *dev;

	alist_init_struct(&gra->reg, கட்டமைப்பு memregion);
	uclass_find_first_device(UCLASS_VIDEO, &dev);
	எனில் (!dev || !device_active(dev))
		திரும்பு log_செய்தி_ret("vid", -ENOENT);

	plat = dev_get_uclass_plat(dev);
	region.base = plat->base;
	region.size = plat->size;
	எனில் (!alist_add(&gra->reg, region))
		திரும்பு log_செய்தி_ret("reg", -ENOMEM);

	priv = dev_get_uclass_priv(dev);
	gra->width = priv->xsize;
	gra->height = priv->ysize;
	gra->stride = priv->வரி_length;	/* private field */
	தேர்வு (priv->format) {
	நிலை VIDEO_RGBA8888:
	நிலை VIDEO_X8R8G8B8:
		gra->format = UPLGF_ARGB32;
		நிறுத்து;
	நிலை VIDEO_X8B8G8R8:
		gra->format = UPLGF_ABGR32;
		நிறுத்து;
	நிலை VIDEO_X2R10G10B10:
		log_debug("device '%s': VIDEO_X2R10G10B10 not supported\n",
			  dev->name);
		திரும்பு log_செய்தி_ret("for", -EPROTO);
	நிலை VIDEO_UNKNOWN:
		log_debug("device '%s': Unknown video format\n", dev->name);
		திரும்பு log_செய்தி_ret("for", -EPROTO);
	}

	திரும்பு 0;
}

முழு spl_write_upl_handoff(கட்டமைப்பு spl_image_info *spl_image)
{
	கட்டமைப்பு upl *upl = &s_upl;
	கட்டமைப்பு abuf buf;
	ofnode வேர்;
	வெறுமை *ptr;
	முழு ret;

	log_debug("UPL: Writing handoff - image_count=%d\n", upl->image.மொத்தம்);
	upl->addr_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->size_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->bootmode = UPLBM_DEFAULT;
	ret = write_serial(&upl->serial);
	எனில் (ret)
		திரும்பு log_செய்தி_ret("ser", ret);
	ret = write_graphics(&upl->graphics);
	எனில் (ret && ret != -ENOENT)
		திரும்பு log_செய்தி_ret("gra", ret);

	வேர் = ofnode_வேர்();
	ret = upl_write_handoff(upl, வேர், true);
	எனில் (ret)
		திரும்பு log_செய்தி_ret("wr", ret);

	ret = oftree_to_fdt(oftree_default(), &buf);
	எனில் (ret)
		திரும்பு log_செய்தி_ret("fdt", ret);
	log_debug("FDT size %zx\n", abuf_size(&buf));

	ptr = bloblist_add(BLOBLISTT_CONTROL_FDT, abuf_size(&buf), 0);
	எனில் (!ptr)
		திரும்பு log_செய்தி_ret("blo", -ENOENT);
	memcpy(ptr, abuf_data(&buf), abuf_size(&buf));

	திரும்பு 0;
}

வெறுமை spl_upl_init(வெறுமை)
{
	upl_init(&s_upl);
}
