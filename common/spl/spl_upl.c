// SPDX-License-Identifier: GPL-2.0+
/*
 * UPL handoff parsing
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#അന്തര്ഭാവ <alist.h>
#അന്തര്ഭാവ <bloblist.h>
#അന്തര്ഭാവ <dm.h>
#അന്തര്ഭാവ <image.h>
#അന്തര്ഭാവ <mapmem.h>
#അന്തര്ഭാവ <serial.h>
#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <upl.h>
#അന്തര്ഭാവ <video.h>
#അന്തര്ഭാവ <asm/global_data.h>
#അന്തര്ഭാവ <dm/read.h>
#അന്തര്ഭാവ <dm/uclass-internal.h>

DECLARE_GLOBAL_DATA_PTR;

ഘടന upl s_upl;

ശൂന്യം upl_set_fit_addr(ulong fit)
{
	ഘടന upl *upl = &s_upl;

	upl->fit = fit;
}

ശൂന്യം upl_set_fit_info(ulong fit, പൂർണ്ണസംഖ്യ conf_offset, ulong entry_addr)
{
	ഘടന upl *upl = &s_upl;

	upl->fit = fit;
	upl->conf_offset = conf_offset;
	log_debug("upl: add fit %lx conf %x\n", fit, conf_offset);
}

പൂർണ്ണസംഖ്യ _upl_add_image(പൂർണ്ണസംഖ്യ കണ്ണി, ulong load_addr, ulong size, സ്ഥിരം അക്ഷരം *desc)
{
	ഘടന upl *upl = &s_upl;
	ഘടന upl_image img;

	img.load = load_addr;
	img.size = size;
	img.offset = കണ്ണി;
	img.description = desc;
	എങ്കിൽ (!alist_add(&upl->image, img))
		തിരികെ -ENOMEM;
	log_debug("upl: add image %s at %lx size %lx\n", desc, load_addr, size);

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ write_serial(ഘടന upl_serial *ser)
{
	ഘടന udevice *dev = gd->cur_serial_dev;
	ഘടന serial_device_info info;
	ഘടന memregion region;
	പൂർണ്ണസംഖ്യ ret;

	എങ്കിൽ (!dev)
		തിരികെ log_സന്ദേശം_ret("ser", -ENOENT);
	ret = serial_getinfo(dev, &info);
	എങ്കിൽ (ret)
		തിരികെ log_സന്ദേശം_ret("inf", ret);

	ser->compatible = ofnode_read_string(dev_ofnode(dev), "compatible");
	ser->clock_frequency = info.clock;
	ser->current_speed = gd->baudrate;
	region.base = info.addr;
	region.size = info.size;
	alist_init_struct(&ser->reg, ഘടന memregion);
	എങ്കിൽ (!alist_add(&ser->reg, region))
		തിരികെ -ENOMEM;
	ser->reg_io_shift = info.reg_shift;
	ser->reg_offset = info.reg_offset;
	ser->reg_io_width = info.reg_width;
	ser->virtual_reg = 0;
	ser->access_type = info.addr_space;

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ write_graphics(ഘടന upl_graphics *gra)
{
	ഘടന video_uc_plat *plat;
	ഘടന video_priv *priv;
	ഘടന memregion region;
	ഘടന udevice *dev;

	alist_init_struct(&gra->reg, ഘടന memregion);
	uclass_find_first_device(UCLASS_VIDEO, &dev);
	എങ്കിൽ (!dev || !device_active(dev))
		തിരികെ log_സന്ദേശം_ret("vid", -ENOENT);

	plat = dev_get_uclass_plat(dev);
	region.base = plat->base;
	region.size = plat->size;
	എങ്കിൽ (!alist_add(&gra->reg, region))
		തിരികെ log_സന്ദേശം_ret("reg", -ENOMEM);

	priv = dev_get_uclass_priv(dev);
	gra->width = priv->xsize;
	gra->height = priv->ysize;
	gra->stride = priv->വരി_length;	/* private field */
	തിരഞ്ഞെടുപ്പ് (priv->format) {
	അവസ്ഥ VIDEO_RGBA8888:
	അവസ്ഥ VIDEO_X8R8G8B8:
		gra->format = UPLGF_ARGB32;
		നിർത്തുക;
	അവസ്ഥ VIDEO_X8B8G8R8:
		gra->format = UPLGF_ABGR32;
		നിർത്തുക;
	അവസ്ഥ VIDEO_X2R10G10B10:
		log_debug("device '%s': VIDEO_X2R10G10B10 not supported\n",
			  dev->name);
		തിരികെ log_സന്ദേശം_ret("for", -EPROTO);
	അവസ്ഥ VIDEO_UNKNOWN:
		log_debug("device '%s': Unknown video format\n", dev->name);
		തിരികെ log_സന്ദേശം_ret("for", -EPROTO);
	}

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ spl_write_upl_handoff(ഘടന spl_image_info *spl_image)
{
	ഘടന upl *upl = &s_upl;
	ഘടന abuf buf;
	ofnode വേര്;
	ശൂന്യം *ptr;
	പൂർണ്ണസംഖ്യ ret;

	log_debug("UPL: Writing handoff - image_count=%d\n", upl->image.എണ്ണം);
	upl->addr_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->size_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->bootmode = UPLBM_DEFAULT;
	ret = write_serial(&upl->serial);
	എങ്കിൽ (ret)
		തിരികെ log_സന്ദേശം_ret("ser", ret);
	ret = write_graphics(&upl->graphics);
	എങ്കിൽ (ret && ret != -ENOENT)
		തിരികെ log_സന്ദേശം_ret("gra", ret);

	വേര് = ofnode_വേര്();
	ret = upl_write_handoff(upl, വേര്, true);
	എങ്കിൽ (ret)
		തിരികെ log_സന്ദേശം_ret("wr", ret);

	ret = oftree_to_fdt(oftree_default(), &buf);
	എങ്കിൽ (ret)
		തിരികെ log_സന്ദേശം_ret("fdt", ret);
	log_debug("FDT size %zx\n", abuf_size(&buf));

	ptr = bloblist_add(BLOBLISTT_CONTROL_FDT, abuf_size(&buf), 0);
	എങ്കിൽ (!ptr)
		തിരികെ log_സന്ദേശം_ret("blo", -ENOENT);
	memcpy(ptr, abuf_data(&buf), abuf_size(&buf));

	തിരികെ 0;
}

ശൂന്യം spl_upl_init(ശൂന്യം)
{
	upl_init(&s_upl);
}
