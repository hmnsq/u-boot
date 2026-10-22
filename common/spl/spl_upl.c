// SPDX-License-Identifier: GPL-2.0+
/*
 * UPL handoff parsing
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#شامل <alist.h>
#شامل <bloblist.h>
#شامل <dm.h>
#شامل <image.h>
#شامل <mapmem.h>
#شامل <serial.h>
#شامل <spl.h>
#شامل <upl.h>
#شامل <video.h>
#شامل <asm/global_data.h>
#شامل <dm/read.h>
#شامل <dm/uclass-internal.h>

DECLARE_GLOBAL_DATA_PTR;

ساختار upl s_upl;

تهی upl_set_fit_addr(ulong fit)
{
	ساختار upl *upl = &s_upl;

	upl->fit = fit;
}

تهی upl_set_fit_info(ulong fit, صحیح conf_offset, ulong entry_addr)
{
	ساختار upl *upl = &s_upl;

	upl->fit = fit;
	upl->conf_offset = conf_offset;
	log_debug("upl: add fit %lx conf %x\n", fit, conf_offset);
}

صحیح _upl_add_image(صحیح گره, ulong load_addr, ulong size, ثابت نویسه *desc)
{
	ساختار upl *upl = &s_upl;
	ساختار upl_image img;

	img.load = load_addr;
	img.size = size;
	img.offset = گره;
	img.description = desc;
	اگر (!alist_add(&upl->image, img))
		بازگشت -ENOMEM;
	log_debug("upl: add image %s at %lx size %lx\n", desc, load_addr, size);

	بازگشت 0;
}

ایستا صحیح write_serial(ساختار upl_serial *ser)
{
	ساختار udevice *dev = gd->cur_serial_dev;
	ساختار serial_device_info info;
	ساختار memregion region;
	صحیح ret;

	اگر (!dev)
		بازگشت log_پیام_ret("ser", -ENOENT);
	ret = serial_getinfo(dev, &info);
	اگر (ret)
		بازگشت log_پیام_ret("inf", ret);

	ser->compatible = ofnode_read_string(dev_ofnode(dev), "compatible");
	ser->clock_frequency = info.clock;
	ser->current_speed = gd->baudrate;
	region.base = info.addr;
	region.size = info.size;
	alist_init_struct(&ser->reg, ساختار memregion);
	اگر (!alist_add(&ser->reg, region))
		بازگشت -ENOMEM;
	ser->reg_io_shift = info.reg_shift;
	ser->reg_offset = info.reg_offset;
	ser->reg_io_width = info.reg_width;
	ser->virtual_reg = 0;
	ser->access_type = info.addr_space;

	بازگشت 0;
}

ایستا صحیح write_graphics(ساختار upl_graphics *gra)
{
	ساختار video_uc_plat *plat;
	ساختار video_priv *priv;
	ساختار memregion region;
	ساختار udevice *dev;

	alist_init_struct(&gra->reg, ساختار memregion);
	uclass_find_first_device(UCLASS_VIDEO, &dev);
	اگر (!dev || !device_active(dev))
		بازگشت log_پیام_ret("vid", -ENOENT);

	plat = dev_get_uclass_plat(dev);
	region.base = plat->base;
	region.size = plat->size;
	اگر (!alist_add(&gra->reg, region))
		بازگشت log_پیام_ret("reg", -ENOMEM);

	priv = dev_get_uclass_priv(dev);
	gra->width = priv->xsize;
	gra->height = priv->ysize;
	gra->stride = priv->سطر_length;	/* private field */
	انتخاب (priv->format) {
	حالت VIDEO_RGBA8888:
	حالت VIDEO_X8R8G8B8:
		gra->format = UPLGF_ARGB32;
		بشکن;
	حالت VIDEO_X8B8G8R8:
		gra->format = UPLGF_ABGR32;
		بشکن;
	حالت VIDEO_X2R10G10B10:
		log_debug("device '%s': VIDEO_X2R10G10B10 not supported\n",
			  dev->name);
		بازگشت log_پیام_ret("for", -EPROTO);
	حالت VIDEO_UNKNOWN:
		log_debug("device '%s': Unknown video format\n", dev->name);
		بازگشت log_پیام_ret("for", -EPROTO);
	}

	بازگشت 0;
}

صحیح spl_write_upl_handoff(ساختار spl_image_info *spl_image)
{
	ساختار upl *upl = &s_upl;
	ساختار abuf buf;
	ofnode ریشه;
	تهی *ptr;
	صحیح ret;

	log_debug("UPL: Writing handoff - image_count=%d\n", upl->image.شمار);
	upl->addr_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->size_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->bootmode = UPLBM_DEFAULT;
	ret = write_serial(&upl->serial);
	اگر (ret)
		بازگشت log_پیام_ret("ser", ret);
	ret = write_graphics(&upl->graphics);
	اگر (ret && ret != -ENOENT)
		بازگشت log_پیام_ret("gra", ret);

	ریشه = ofnode_ریشه();
	ret = upl_write_handoff(upl, ریشه, true);
	اگر (ret)
		بازگشت log_پیام_ret("wr", ret);

	ret = oftree_to_fdt(oftree_default(), &buf);
	اگر (ret)
		بازگشت log_پیام_ret("fdt", ret);
	log_debug("FDT size %zx\n", abuf_size(&buf));

	ptr = bloblist_add(BLOBLISTT_CONTROL_FDT, abuf_size(&buf), 0);
	اگر (!ptr)
		بازگشت log_پیام_ret("blo", -ENOENT);
	memcpy(ptr, abuf_data(&buf), abuf_size(&buf));

	بازگشت 0;
}

تهی spl_upl_init(تهی)
{
	upl_init(&s_upl);
}
