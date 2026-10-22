// SPDX-License-Identifier: GPL-2.0+
/*
 * UPL handoff parsing
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#অন্তর্ভাব <alist.h>
#অন্তর্ভাব <bloblist.h>
#অন্তর্ভাব <dm.h>
#অন্তর্ভাব <image.h>
#অন্তর্ভাব <mapmem.h>
#অন্তর্ভাব <serial.h>
#অন্তর্ভাব <spl.h>
#অন্তর্ভাব <upl.h>
#অন্তর্ভাব <video.h>
#অন্তর্ভাব <asm/global_data.h>
#অন্তর্ভাব <dm/read.h>
#অন্তর্ভাব <dm/uclass-internal.h>

DECLARE_GLOBAL_তথ্য_PTR;

কাঠামো upl s_upl;

শূন্য upl_set_fit_addr(ulong fit)
{
	কাঠামো upl *upl = &s_upl;

	upl->fit = fit;
}

শূন্য upl_set_fit_info(ulong fit, পূর্ণসংখ্যা conf_offset, ulong entry_addr)
{
	কাঠামো upl *upl = &s_upl;

	upl->fit = fit;
	upl->conf_offset = conf_offset;
	log_debug("upl: add fit %lx conf %x\n", fit, conf_offset);
}

পূর্ণসংখ্যা _upl_add_image(পূর্ণসংখ্যা গ্রন্থি, ulong load_addr, ulong আকার_2, ধ্রুবক অক্ষর *desc)
{
	কাঠামো upl *upl = &s_upl;
	কাঠামো upl_image img;

	img.load = load_addr;
	img.আকার_2 = আকার_2;
	img.offset = গ্রন্থি;
	img.description = desc;
	যদি (!alist_add(&upl->image, img))
		ফেরত -ENOMEM;
	log_debug("upl: add image %s at %lx size %lx\n", desc, load_addr, আকার_2);

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা write_serial(কাঠামো upl_serial *ser)
{
	কাঠামো udevice *dev = gd->cur_serial_dev;
	কাঠামো serial_device_info info;
	কাঠামো memregion region;
	পূর্ণসংখ্যা ret;

	যদি (!dev)
		ফেরত log_বার্তা_ret("ser", -ENOENT);
	ret = serial_getinfo(dev, &info);
	যদি (ret)
		ফেরত log_বার্তা_ret("inf", ret);

	ser->compatible = ofnode_read_string(dev_ofnode(dev), "compatible");
	ser->clock_frequency = info.clock;
	ser->current_speed = gd->baudrate;
	region.base = info.addr;
	region.আকার_2 = info.আকার_2;
	alist_init_struct(&ser->reg, কাঠামো memregion);
	যদি (!alist_add(&ser->reg, region))
		ফেরত -ENOMEM;
	ser->reg_io_shift = info.reg_shift;
	ser->reg_offset = info.reg_offset;
	ser->reg_io_width = info.reg_width;
	ser->virtual_reg = 0;
	ser->access_type = info.addr_space;

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা write_graphics(কাঠামো upl_graphics *gra)
{
	কাঠামো video_uc_plat *plat;
	কাঠামো video_priv *priv;
	কাঠামো memregion region;
	কাঠামো udevice *dev;

	alist_init_struct(&gra->reg, কাঠামো memregion);
	uclass_find_first_device(UCLASS_VIDEO, &dev);
	যদি (!dev || !device_active(dev))
		ফেরত log_বার্তা_ret("vid", -ENOENT);

	plat = dev_get_uclass_plat(dev);
	region.base = plat->base;
	region.আকার_2 = plat->আকার_2;
	যদি (!alist_add(&gra->reg, region))
		ফেরত log_বার্তা_ret("reg", -ENOMEM);

	priv = dev_get_uclass_priv(dev);
	gra->width = priv->xsize;
	gra->height = priv->ysize;
	gra->stride = priv->সারি_দৈর্ঘ্য;	/* private field */
	নির্বাচন (priv->format) {
	ক্ষেত্র VIDEO_RGBA8888:
	ক্ষেত্র VIDEO_X8R8G8B8:
		gra->format = UPLGF_ARGB32;
		বিরতি;
	ক্ষেত্র VIDEO_X8B8G8R8:
		gra->format = UPLGF_ABGR32;
		বিরতি;
	ক্ষেত্র VIDEO_X2R10G10B10:
		log_debug("device '%s': VIDEO_X2R10G10B10 not supported\n",
			  dev->name);
		ফেরত log_বার্তা_ret("for", -EPROTO);
	ক্ষেত্র VIDEO_UNKNOWN:
		log_debug("device '%s': Unknown video format\n", dev->name);
		ফেরত log_বার্তা_ret("for", -EPROTO);
	}

	ফেরত 0;
}

পূর্ণসংখ্যা spl_write_upl_handoff(কাঠামো spl_image_info *spl_image)
{
	কাঠামো upl *upl = &s_upl;
	কাঠামো abuf buf;
	ofnode মূল;
	শূন্য *ptr;
	পূর্ণসংখ্যা ret;

	log_debug("UPL: Writing handoff - image_count=%d\n", upl->image.গুনতি);
	upl->addr_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->আকার_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->bootmode = UPLBM_DEFAULT;
	ret = write_serial(&upl->serial);
	যদি (ret)
		ফেরত log_বার্তা_ret("ser", ret);
	ret = write_graphics(&upl->graphics);
	যদি (ret && ret != -ENOENT)
		ফেরত log_বার্তা_ret("gra", ret);

	মূল = ofnode_মূল();
	ret = upl_write_handoff(upl, মূল, true);
	যদি (ret)
		ফেরত log_বার্তা_ret("wr", ret);

	ret = oftree_to_fdt(oftree_default(), &buf);
	যদি (ret)
		ফেরত log_বার্তা_ret("fdt", ret);
	log_debug("FDT size %zx\n", abuf_আকার(&buf));

	ptr = bloblist_add(BLOBLISTT_CONTROL_FDT, abuf_আকার(&buf), 0);
	যদি (!ptr)
		ফেরত log_বার্তা_ret("blo", -ENOENT);
	memcpy(ptr, abuf_তথ্য(&buf), abuf_আকার(&buf));

	ফেরত 0;
}

শূন্য spl_upl_init(শূন্য)
{
	upl_init(&s_upl);
}
