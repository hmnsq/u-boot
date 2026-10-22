// SPDX-License-Identifier: GPL-2.0+
/*
 * UPL handoff parsing
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#ضمن <alist.h>
#ضمن <bloblist.h>
#ضمن <dm.h>
#ضمن <image.h>
#ضمن <mapmem.h>
#ضمن <serial.h>
#ضمن <spl.h>
#ضمن <upl.h>
#ضمن <video.h>
#ضمن <asm/global_data.h>
#ضمن <dm/read.h>
#ضمن <dm/uclass-internal.h>

DECLARE_GLOBAL_DATA_PTR;

بنية upl s_upl;

فراغ upl_set_fit_addr(ulong fit)
{
	بنية upl *upl = &s_upl;

	upl->fit = fit;
}

فراغ upl_set_fit_info(ulong fit, صحيح conf_offset, ulong entry_addr)
{
	بنية upl *upl = &s_upl;

	upl->fit = fit;
	upl->conf_offset = conf_offset;
	log_debug("upl: add fit %lx conf %x\n", fit, conf_offset);
}

صحيح _upl_add_image(صحيح عقدة, ulong load_addr, ulong size, ثابت حرف *desc)
{
	بنية upl *upl = &s_upl;
	بنية upl_image img;

	img.load = load_addr;
	img.size = size;
	img.offset = عقدة;
	img.description = desc;
	إذا (!alist_add(&upl->image, img))
		إرجاع -ENOMEM;
	log_debug("upl: add image %s at %lx size %lx\n", desc, load_addr, size);

	إرجاع 0;
}

ساكن صحيح write_serial(بنية upl_serial *ser)
{
	بنية udevice *dev = gd->cur_serial_dev;
	بنية serial_device_info info;
	بنية memregion region;
	صحيح ret;

	إذا (!dev)
		إرجاع log_رسالة_ret("ser", -ENOENT);
	ret = serial_getinfo(dev, &info);
	إذا (ret)
		إرجاع log_رسالة_ret("inf", ret);

	ser->compatible = ofnode_read_string(dev_ofnode(dev), "compatible");
	ser->clock_frequency = info.clock;
	ser->current_speed = gd->baudrate;
	region.base = info.addr;
	region.size = info.size;
	alist_init_struct(&ser->reg, بنية memregion);
	إذا (!alist_add(&ser->reg, region))
		إرجاع -ENOMEM;
	ser->reg_io_shift = info.reg_shift;
	ser->reg_offset = info.reg_offset;
	ser->reg_io_width = info.reg_width;
	ser->virtual_reg = 0;
	ser->access_type = info.addr_space;

	إرجاع 0;
}

ساكن صحيح write_graphics(بنية upl_graphics *gra)
{
	بنية video_uc_plat *plat;
	بنية video_priv *priv;
	بنية memregion region;
	بنية udevice *dev;

	alist_init_struct(&gra->reg, بنية memregion);
	uclass_find_first_device(UCLASS_VIDEO, &dev);
	إذا (!dev || !device_active(dev))
		إرجاع log_رسالة_ret("vid", -ENOENT);

	plat = dev_get_uclass_plat(dev);
	region.base = plat->base;
	region.size = plat->size;
	إذا (!alist_add(&gra->reg, region))
		إرجاع log_رسالة_ret("reg", -ENOMEM);

	priv = dev_get_uclass_priv(dev);
	gra->width = priv->xsize;
	gra->height = priv->ysize;
	gra->stride = priv->سطر_length;	/* private field */
	بدل (priv->format) {
	حالة VIDEO_RGBA8888:
	حالة VIDEO_X8R8G8B8:
		gra->format = UPLGF_ARGB32;
		اكسر;
	حالة VIDEO_X8B8G8R8:
		gra->format = UPLGF_ABGR32;
		اكسر;
	حالة VIDEO_X2R10G10B10:
		log_debug("device '%s': VIDEO_X2R10G10B10 not supported\n",
			  dev->name);
		إرجاع log_رسالة_ret("for", -EPROTO);
	حالة VIDEO_UNKNOWN:
		log_debug("device '%s': Unknown video format\n", dev->name);
		إرجاع log_رسالة_ret("for", -EPROTO);
	}

	إرجاع 0;
}

صحيح spl_write_upl_handoff(بنية spl_image_info *spl_image)
{
	بنية upl *upl = &s_upl;
	بنية abuf buf;
	ofnode جذر;
	فراغ *ptr;
	صحيح ret;

	log_debug("UPL: Writing handoff - image_count=%d\n", upl->image.تعداد_2);
	upl->addr_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->size_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->bootmode = UPLBM_DEFAULT;
	ret = write_serial(&upl->serial);
	إذا (ret)
		إرجاع log_رسالة_ret("ser", ret);
	ret = write_graphics(&upl->graphics);
	إذا (ret && ret != -ENOENT)
		إرجاع log_رسالة_ret("gra", ret);

	جذر = ofnode_جذر();
	ret = upl_write_handoff(upl, جذر, true);
	إذا (ret)
		إرجاع log_رسالة_ret("wr", ret);

	ret = oftree_to_fdt(oftree_default(), &buf);
	إذا (ret)
		إرجاع log_رسالة_ret("fdt", ret);
	log_debug("FDT size %zx\n", abuf_size(&buf));

	ptr = bloblist_add(BLOBLISTT_CONTROL_FDT, abuf_size(&buf), 0);
	إذا (!ptr)
		إرجاع log_رسالة_ret("blo", -ENOENT);
	memcpy(ptr, abuf_data(&buf), abuf_size(&buf));

	إرجاع 0;
}

فراغ spl_upl_init(فراغ)
{
	upl_init(&s_upl);
}
