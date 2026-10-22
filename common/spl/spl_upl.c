// SPDX-License-Identifier: GPL-2.0+
/*
 * UPL handoff parsing
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#અન્તર્ભાવ <alist.h>
#અન્તર્ભાવ <bloblist.h>
#અન્તર્ભાવ <dm.h>
#અન્તર્ભાવ <image.h>
#અન્તર્ભાવ <mapmem.h>
#અન્તર્ભાવ <serial.h>
#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <upl.h>
#અન્તર્ભાવ <video.h>
#અન્તર્ભાવ <asm/global_data.h>
#અન્તર્ભાવ <dm/read.h>
#અન્તર્ભાવ <dm/uclass-internal.h>

DECLARE_GLOBAL_DATA_PTR;

સંરચના upl s_upl;

શૂન્ય upl_set_fit_addr(ulong fit)
{
	સંરચના upl *upl = &s_upl;

	upl->fit = fit;
}

શૂન્ય upl_set_fit_info(ulong fit, પૂર્ણાંક conf_offset, ulong entry_addr)
{
	સંરચના upl *upl = &s_upl;

	upl->fit = fit;
	upl->conf_offset = conf_offset;
	log_debug("upl: add fit %lx conf %x\n", fit, conf_offset);
}

પૂર્ણાંક _upl_add_image(પૂર્ણાંક ગાંઠ, ulong load_addr, ulong size, અચલ અક્ષર *desc)
{
	સંરચના upl *upl = &s_upl;
	સંરચના upl_image img;

	img.load = load_addr;
	img.size = size;
	img.offset = ગાંઠ;
	img.description = desc;
	જો (!alist_add(&upl->image, img))
		પાછા -ENOMEM;
	log_debug("upl: add image %s at %lx size %lx\n", desc, load_addr, size);

	પાછા 0;
}

સ્થિર પૂર્ણાંક write_serial(સંરચના upl_serial *ser)
{
	સંરચના udevice *dev = gd->cur_serial_dev;
	સંરચના serial_device_info info;
	સંરચના memregion region;
	પૂર્ણાંક ret;

	જો (!dev)
		પાછા log_સંદેશ_ret("ser", -ENOENT);
	ret = serial_getinfo(dev, &info);
	જો (ret)
		પાછા log_સંદેશ_ret("inf", ret);

	ser->compatible = ofnode_read_string(dev_ofnode(dev), "compatible");
	ser->clock_frequency = info.clock;
	ser->current_speed = gd->baudrate;
	region.base = info.addr;
	region.size = info.size;
	alist_init_struct(&ser->reg, સંરચના memregion);
	જો (!alist_add(&ser->reg, region))
		પાછા -ENOMEM;
	ser->reg_io_shift = info.reg_shift;
	ser->reg_offset = info.reg_offset;
	ser->reg_io_width = info.reg_width;
	ser->virtual_reg = 0;
	ser->access_type = info.addr_space;

	પાછા 0;
}

સ્થિર પૂર્ણાંક write_graphics(સંરચના upl_graphics *gra)
{
	સંરચના video_uc_plat *plat;
	સંરચના video_priv *priv;
	સંરચના memregion region;
	સંરચના udevice *dev;

	alist_init_struct(&gra->reg, સંરચના memregion);
	uclass_find_first_device(UCLASS_VIDEO, &dev);
	જો (!dev || !device_active(dev))
		પાછા log_સંદેશ_ret("vid", -ENOENT);

	plat = dev_get_uclass_plat(dev);
	region.base = plat->base;
	region.size = plat->size;
	જો (!alist_add(&gra->reg, region))
		પાછા log_સંદેશ_ret("reg", -ENOMEM);

	priv = dev_get_uclass_priv(dev);
	gra->width = priv->xsize;
	gra->height = priv->ysize;
	gra->stride = priv->લીટી_length;	/* private field */
	પસંદગી (priv->format) {
	સ્થિતિ VIDEO_RGBA8888:
	સ્થિતિ VIDEO_X8R8G8B8:
		gra->format = UPLGF_ARGB32;
		તોડો;
	સ્થિતિ VIDEO_X8B8G8R8:
		gra->format = UPLGF_ABGR32;
		તોડો;
	સ્થિતિ VIDEO_X2R10G10B10:
		log_debug("device '%s': VIDEO_X2R10G10B10 not supported\n",
			  dev->name);
		પાછા log_સંદેશ_ret("for", -EPROTO);
	સ્થિતિ VIDEO_UNKNOWN:
		log_debug("device '%s': Unknown video format\n", dev->name);
		પાછા log_સંદેશ_ret("for", -EPROTO);
	}

	પાછા 0;
}

પૂર્ણાંક spl_write_upl_handoff(સંરચના spl_image_info *spl_image)
{
	સંરચના upl *upl = &s_upl;
	સંરચના abuf buf;
	ofnode મૂળ;
	શૂન્ય *ptr;
	પૂર્ણાંક ret;

	log_debug("UPL: Writing handoff - image_count=%d\n", upl->image.ગણતરી);
	upl->addr_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->size_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->bootmode = UPLBM_DEFAULT;
	ret = write_serial(&upl->serial);
	જો (ret)
		પાછા log_સંદેશ_ret("ser", ret);
	ret = write_graphics(&upl->graphics);
	જો (ret && ret != -ENOENT)
		પાછા log_સંદેશ_ret("gra", ret);

	મૂળ = ofnode_મૂળ();
	ret = upl_write_handoff(upl, મૂળ, true);
	જો (ret)
		પાછા log_સંદેશ_ret("wr", ret);

	ret = oftree_to_fdt(oftree_default(), &buf);
	જો (ret)
		પાછા log_સંદેશ_ret("fdt", ret);
	log_debug("FDT size %zx\n", abuf_size(&buf));

	ptr = bloblist_add(BLOBLISTT_CONTROL_FDT, abuf_size(&buf), 0);
	જો (!ptr)
		પાછા log_સંદેશ_ret("blo", -ENOENT);
	memcpy(ptr, abuf_data(&buf), abuf_size(&buf));

	પાછા 0;
}

શૂન્ય spl_upl_init(શૂન્ય)
{
	upl_init(&s_upl);
}
