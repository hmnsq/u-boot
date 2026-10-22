// SPDX-License-Identifier: GPL-2.0+
/*
 * UPL handoff parsing
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#ଅନ୍ତର୍ଭାଵ <alist.h>
#ଅନ୍ତର୍ଭାଵ <bloblist.h>
#ଅନ୍ତର୍ଭାଵ <dm.h>
#ଅନ୍ତର୍ଭାଵ <image.h>
#ଅନ୍ତର୍ଭାଵ <mapmem.h>
#ଅନ୍ତର୍ଭାଵ <serial.h>
#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <upl.h>
#ଅନ୍ତର୍ଭାଵ <video.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>
#ଅନ୍ତର୍ଭାଵ <dm/read.h>
#ଅନ୍ତର୍ଭାଵ <dm/uclass-internal.h>

DECLARE_GLOBAL_DATA_PTR;

ସଂରଚନା upl s_upl;

ଶୂନ୍ୟ upl_set_fit_addr(ulong fit)
{
	ସଂରଚନା upl *upl = &s_upl;

	upl->fit = fit;
}

ଶୂନ୍ୟ upl_set_fit_info(ulong fit, ପୂର୍ଣ୍ଣସଂଖ୍ୟା conf_offset, ulong entry_addr)
{
	ସଂରଚନା upl *upl = &s_upl;

	upl->fit = fit;
	upl->conf_offset = conf_offset;
	log_debug("upl: add fit %lx conf %x\n", fit, conf_offset);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା _upl_add_image(ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣ୍ଠି, ulong load_addr, ulong size, ଅଚଳ ଅକ୍ଷର *desc)
{
	ସଂରଚନା upl *upl = &s_upl;
	ସଂରଚନା upl_image img;

	img.load = load_addr;
	img.size = size;
	img.offset = ଗଣ୍ଠି;
	img.description = desc;
	ଯଦି (!alist_add(&upl->image, img))
		ଫେରନ୍ତୁ -ENOMEM;
	log_debug("upl: add image %s at %lx size %lx\n", desc, load_addr, size);

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା write_serial(ସଂରଚନା upl_serial *ser)
{
	ସଂରଚନା udevice *dev = gd->cur_serial_dev;
	ସଂରଚନା serial_device_info info;
	ସଂରଚନା memregion region;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ଯଦି (!dev)
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("ser", -ENOENT);
	ret = serial_getinfo(dev, &info);
	ଯଦି (ret)
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("inf", ret);

	ser->compatible = ofnode_read_string(dev_ofnode(dev), "compatible");
	ser->clock_frequency = info.clock;
	ser->current_speed = gd->baudrate;
	region.base = info.addr;
	region.size = info.size;
	alist_init_struct(&ser->reg, ସଂରଚନା memregion);
	ଯଦି (!alist_add(&ser->reg, region))
		ଫେରନ୍ତୁ -ENOMEM;
	ser->reg_io_shift = info.reg_shift;
	ser->reg_offset = info.reg_offset;
	ser->reg_io_width = info.reg_width;
	ser->virtual_reg = 0;
	ser->access_type = info.addr_space;

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା write_graphics(ସଂରଚନା upl_graphics *gra)
{
	ସଂରଚନା video_uc_plat *plat;
	ସଂରଚନା video_priv *priv;
	ସଂରଚନା memregion region;
	ସଂରଚନା udevice *dev;

	alist_init_struct(&gra->reg, ସଂରଚନା memregion);
	uclass_find_first_device(UCLASS_VIDEO, &dev);
	ଯଦି (!dev || !device_active(dev))
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("vid", -ENOENT);

	plat = dev_get_uclass_plat(dev);
	region.base = plat->base;
	region.size = plat->size;
	ଯଦି (!alist_add(&gra->reg, region))
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("reg", -ENOMEM);

	priv = dev_get_uclass_priv(dev);
	gra->width = priv->xsize;
	gra->height = priv->ysize;
	gra->stride = priv->ଧାଡ଼ି_length;	/* private field */
	ବାଛ (priv->format) {
	ସ୍ଥିତି VIDEO_RGBA8888:
	ସ୍ଥିତି VIDEO_X8R8G8B8:
		gra->format = UPLGF_ARGB32;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି VIDEO_X8B8G8R8:
		gra->format = UPLGF_ABGR32;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି VIDEO_X2R10G10B10:
		log_debug("device '%s': VIDEO_X2R10G10B10 not supported\n",
			  dev->name);
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("for", -EPROTO);
	ସ୍ଥିତି VIDEO_UNKNOWN:
		log_debug("device '%s': Unknown video format\n", dev->name);
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("for", -EPROTO);
	}

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_write_upl_handoff(ସଂରଚନା spl_image_info *spl_image)
{
	ସଂରଚନା upl *upl = &s_upl;
	ସଂରଚନା abuf buf;
	ofnode ମୂଳ;
	ଶୂନ୍ୟ *ptr;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	log_debug("UPL: Writing handoff - image_count=%d\n", upl->image.ଗଣନା_2);
	upl->addr_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->size_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->bootmode = UPLBM_DEFAULT;
	ret = write_serial(&upl->serial);
	ଯଦି (ret)
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("ser", ret);
	ret = write_graphics(&upl->graphics);
	ଯଦି (ret && ret != -ENOENT)
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("gra", ret);

	ମୂଳ = ofnode_ମୂଳ();
	ret = upl_write_handoff(upl, ମୂଳ, true);
	ଯଦି (ret)
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("wr", ret);

	ret = oftree_to_fdt(oftree_default(), &buf);
	ଯଦି (ret)
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("fdt", ret);
	log_debug("FDT size %zx\n", abuf_size(&buf));

	ptr = bloblist_add(BLOBLISTT_CONTROL_FDT, abuf_size(&buf), 0);
	ଯଦି (!ptr)
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("blo", -ENOENT);
	memcpy(ptr, abuf_data(&buf), abuf_size(&buf));

	ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ spl_upl_init(ଶୂନ୍ୟ)
{
	upl_init(&s_upl);
}
