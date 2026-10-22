// SPDX-License-Identifier: GPL-2.0+
/*
 * UPL handoff parsing
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#अन्तर्भाव <alist.h>
#अन्तर्भाव <bloblist.h>
#अन्तर्भाव <dm.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <mapmem.h>
#अन्तर्भाव <serial.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <upl.h>
#अन्तर्भाव <video.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <dm/read.h>
#अन्तर्भाव <dm/uclass-internal.h>

DECLARE_GLOBAL_DATA_PTR;

संरचना upl s_upl;

शून्य upl_set_fit_addr(ulong fit)
{
	संरचना upl *upl = &s_upl;

	upl->fit = fit;
}

शून्य upl_set_fit_info(ulong fit, पूर्णाङ्क conf_offset, ulong entry_addr)
{
	संरचना upl *upl = &s_upl;

	upl->fit = fit;
	upl->conf_offset = conf_offset;
	log_debug("upl: add fit %lx conf %x\n", fit, conf_offset);
}

पूर्णाङ्क _upl_add_image(पूर्णाङ्क गाठ, ulong load_addr, ulong size, ध्रुव वर्ण *desc)
{
	संरचना upl *upl = &s_upl;
	संरचना upl_image img;

	img.load = load_addr;
	img.size = size;
	img.offset = गाठ;
	img.description = desc;
	यदि (!alist_add(&upl->image, img))
		प्रत्यागम -ENOMEM;
	log_debug("upl: add image %s at %lx size %lx\n", desc, load_addr, size);

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क write_serial(संरचना upl_serial *ser)
{
	संरचना udevice *dev = gd->cur_serial_dev;
	संरचना serial_device_info info;
	संरचना memregion region;
	पूर्णाङ्क ret;

	यदि (!dev)
		प्रत्यागम log_संदेश_ret("ser", -ENOENT);
	ret = serial_getinfo(dev, &info);
	यदि (ret)
		प्रत्यागम log_संदेश_ret("inf", ret);

	ser->compatible = ofnode_read_string(dev_ofnode(dev), "compatible");
	ser->clock_frequency = info.clock;
	ser->current_speed = gd->baudrate;
	region.base = info.addr;
	region.size = info.size;
	alist_init_struct(&ser->reg, संरचना memregion);
	यदि (!alist_add(&ser->reg, region))
		प्रत्यागम -ENOMEM;
	ser->reg_io_shift = info.reg_shift;
	ser->reg_offset = info.reg_offset;
	ser->reg_io_width = info.reg_width;
	ser->virtual_reg = 0;
	ser->access_type = info.addr_space;

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क write_graphics(संरचना upl_graphics *gra)
{
	संरचना video_uc_plat *plat;
	संरचना video_priv *priv;
	संरचना memregion region;
	संरचना udevice *dev;

	alist_init_struct(&gra->reg, संरचना memregion);
	uclass_find_first_device(UCLASS_VIDEO, &dev);
	यदि (!dev || !device_active(dev))
		प्रत्यागम log_संदेश_ret("vid", -ENOENT);

	plat = dev_get_uclass_plat(dev);
	region.base = plat->base;
	region.size = plat->size;
	यदि (!alist_add(&gra->reg, region))
		प्रत्यागम log_संदेश_ret("reg", -ENOMEM);

	priv = dev_get_uclass_priv(dev);
	gra->width = priv->xsize;
	gra->height = priv->ysize;
	gra->stride = priv->ओळ_length;	/* private field */
	विकल्प (priv->format) {
	पक्ष VIDEO_RGBA8888:
	पक्ष VIDEO_X8R8G8B8:
		gra->format = UPLGF_ARGB32;
		भङ्ग;
	पक्ष VIDEO_X8B8G8R8:
		gra->format = UPLGF_ABGR32;
		भङ्ग;
	पक्ष VIDEO_X2R10G10B10:
		log_debug("device '%s': VIDEO_X2R10G10B10 not supported\n",
			  dev->name);
		प्रत्यागम log_संदेश_ret("for", -EPROTO);
	पक्ष VIDEO_UNKNOWN:
		log_debug("device '%s': Unknown video format\n", dev->name);
		प्रत्यागम log_संदेश_ret("for", -EPROTO);
	}

	प्रत्यागम 0;
}

पूर्णाङ्क spl_write_upl_handoff(संरचना spl_image_info *spl_image)
{
	संरचना upl *upl = &s_upl;
	संरचना abuf buf;
	ofnode मूळ;
	शून्य *ptr;
	पूर्णाङ्क ret;

	log_debug("UPL: Writing handoff - image_count=%d\n", upl->image.मोजणी);
	upl->addr_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->size_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->bootmode = UPLBM_DEFAULT;
	ret = write_serial(&upl->serial);
	यदि (ret)
		प्रत्यागम log_संदेश_ret("ser", ret);
	ret = write_graphics(&upl->graphics);
	यदि (ret && ret != -ENOENT)
		प्रत्यागम log_संदेश_ret("gra", ret);

	मूळ = ofnode_मूळ();
	ret = upl_write_handoff(upl, मूळ, true);
	यदि (ret)
		प्रत्यागम log_संदेश_ret("wr", ret);

	ret = oftree_to_fdt(oftree_default(), &buf);
	यदि (ret)
		प्रत्यागम log_संदेश_ret("fdt", ret);
	log_debug("FDT size %zx\n", abuf_size(&buf));

	ptr = bloblist_add(BLOBLISTT_CONTROL_FDT, abuf_size(&buf), 0);
	यदि (!ptr)
		प्रत्यागम log_संदेश_ret("blo", -ENOENT);
	memcpy(ptr, abuf_data(&buf), abuf_size(&buf));

	प्रत्यागम 0;
}

शून्य spl_upl_init(शून्य)
{
	upl_init(&s_upl);
}
