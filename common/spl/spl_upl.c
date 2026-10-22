// SPDX-License-Identifier: GPL-2.0+
/*
 * UPL handoff parsing
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#অন্তর্ভাৱ <alist.h>
#অন্তর্ভাৱ <bloblist.h>
#অন্তর্ভাৱ <dm.h>
#অন্তর্ভাৱ <image.h>
#অন্তর্ভাৱ <mapmem.h>
#অন্তর্ভাৱ <serial.h>
#অন্তর্ভাৱ <spl.h>
#অন্তর্ভাৱ <upl.h>
#অন্তর্ভাৱ <video.h>
#অন্তর্ভাৱ <asm/global_data.h>
#অন্তর্ভাৱ <dm/read.h>
#অন্তর্ভাৱ <dm/uclass-internal.h>

DECLARE_GLOBAL_DATA_PTR;

সংরচনা upl s_upl;

শূন্য upl_set_fit_addr(ulong fit)
{
	সংরচনা upl *upl = &s_upl;

	upl->fit = fit;
}

শূন্য upl_set_fit_info(ulong fit, পূর্ণাঙ্ক conf_offset, ulong entry_addr)
{
	সংরচনা upl *upl = &s_upl;

	upl->fit = fit;
	upl->conf_offset = conf_offset;
	log_debug("upl: add fit %lx conf %x\n", fit, conf_offset);
}

পূর্ণাঙ্ক _upl_add_image(পূর্ণাঙ্ক গ্ৰন্থি, ulong load_addr, ulong size, ধ্রুৱ ৱর্ণ *desc)
{
	সংরচনা upl *upl = &s_upl;
	সংরচনা upl_image img;

	img.load = load_addr;
	img.size = size;
	img.offset = গ্ৰন্থি;
	img.description = desc;
	যদি (!alist_add(&upl->image, img))
		ঘূৰাই -ENOMEM;
	log_debug("upl: add image %s at %lx size %lx\n", desc, load_addr, size);

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক write_serial(সংরচনা upl_serial *ser)
{
	সংরচনা udevice *dev = gd->cur_serial_dev;
	সংরচনা serial_device_info info;
	সংরচনা memregion region;
	পূর্ণাঙ্ক ret;

	যদি (!dev)
		ঘূৰাই log_বাৰ্তা_ret("ser", -ENOENT);
	ret = serial_getinfo(dev, &info);
	যদি (ret)
		ঘূৰাই log_বাৰ্তা_ret("inf", ret);

	ser->compatible = ofnode_read_string(dev_ofnode(dev), "compatible");
	ser->clock_frequency = info.clock;
	ser->current_speed = gd->baudrate;
	region.base = info.addr;
	region.size = info.size;
	alist_init_struct(&ser->reg, সংরচনা memregion);
	যদি (!alist_add(&ser->reg, region))
		ঘূৰাই -ENOMEM;
	ser->reg_io_shift = info.reg_shift;
	ser->reg_offset = info.reg_offset;
	ser->reg_io_width = info.reg_width;
	ser->virtual_reg = 0;
	ser->access_type = info.addr_space;

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক write_graphics(সংরচনা upl_graphics *gra)
{
	সংরচনা video_uc_plat *plat;
	সংরচনা video_priv *priv;
	সংরচনা memregion region;
	সংরচনা udevice *dev;

	alist_init_struct(&gra->reg, সংরচনা memregion);
	uclass_find_first_device(UCLASS_VIDEO, &dev);
	যদি (!dev || !device_active(dev))
		ঘূৰাই log_বাৰ্তা_ret("vid", -ENOENT);

	plat = dev_get_uclass_plat(dev);
	region.base = plat->base;
	region.size = plat->size;
	যদি (!alist_add(&gra->reg, region))
		ঘূৰাই log_বাৰ্তা_ret("reg", -ENOMEM);

	priv = dev_get_uclass_priv(dev);
	gra->width = priv->xsize;
	gra->height = priv->ysize;
	gra->stride = priv->সাৰি_length;	/* private field */
	ৱিকল্প (priv->format) {
	পক্ষ VIDEO_RGBA8888:
	পক্ষ VIDEO_X8R8G8B8:
		gra->format = UPLGF_ARGB32;
		ভঙ্গ;
	পক্ষ VIDEO_X8B8G8R8:
		gra->format = UPLGF_ABGR32;
		ভঙ্গ;
	পক্ষ VIDEO_X2R10G10B10:
		log_debug("device '%s': VIDEO_X2R10G10B10 not supported\n",
			  dev->name);
		ঘূৰাই log_বাৰ্তা_ret("for", -EPROTO);
	পক্ষ VIDEO_UNKNOWN:
		log_debug("device '%s': Unknown video format\n", dev->name);
		ঘূৰাই log_বাৰ্তা_ret("for", -EPROTO);
	}

	ঘূৰাই 0;
}

পূর্ণাঙ্ক spl_write_upl_handoff(সংরচনা spl_image_info *spl_image)
{
	সংরচনা upl *upl = &s_upl;
	সংরচনা abuf buf;
	ofnode মূল;
	শূন্য *ptr;
	পূর্ণাঙ্ক ret;

	log_debug("UPL: Writing handoff - image_count=%d\n", upl->image.গুনতি);
	upl->addr_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->size_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->bootmode = UPLBM_DEFAULT;
	ret = write_serial(&upl->serial);
	যদি (ret)
		ঘূৰাই log_বাৰ্তা_ret("ser", ret);
	ret = write_graphics(&upl->graphics);
	যদি (ret && ret != -ENOENT)
		ঘূৰাই log_বাৰ্তা_ret("gra", ret);

	মূল = ofnode_মূল();
	ret = upl_write_handoff(upl, মূল, true);
	যদি (ret)
		ঘূৰাই log_বাৰ্তা_ret("wr", ret);

	ret = oftree_to_fdt(oftree_default(), &buf);
	যদি (ret)
		ঘূৰাই log_বাৰ্তা_ret("fdt", ret);
	log_debug("FDT size %zx\n", abuf_size(&buf));

	ptr = bloblist_add(BLOBLISTT_CONTROL_FDT, abuf_size(&buf), 0);
	যদি (!ptr)
		ঘূৰাই log_বাৰ্তা_ret("blo", -ENOENT);
	memcpy(ptr, abuf_data(&buf), abuf_size(&buf));

	ঘূৰাই 0;
}

শূন্য spl_upl_init(শূন্য)
{
	upl_init(&s_upl);
}
