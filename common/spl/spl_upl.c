// SPDX-License-Identifier: GPL-2.0+
/*
 * UPL handoff parsing
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#घोषणा LOG_CATEGORY UCLASS_BOOTSTD

#समावेश <alist.h>
#समावेश <bloblist.h>
#समावेश <dm.h>
#समावेश <image.h>
#समावेश <mapmem.h>
#समावेश <serial.h>
#समावेश <spl.h>
#समावेश <upl.h>
#समावेश <video.h>
#समावेश <asm/global_data.h>
#समावेश <dm/read.h>
#समावेश <dm/uclass-internal.h>

DECLARE_GLOBAL_तथ्य_PTR;

काष्ठा upl s_upl;

व्योम upl_set_fit_addr(ulong fit)
{
	काष्ठा upl *upl = &s_upl;

	upl->fit = fit;
}

व्योम upl_set_fit_info(ulong fit, पूर्णांक conf_offset, ulong entry_addr)
{
	काष्ठा upl *upl = &s_upl;

	upl->fit = fit;
	upl->conf_offset = conf_offset;
	log_debug("upl: add fit %lx conf %x\n", fit, conf_offset);
}

पूर्णांक _upl_add_image(पूर्णांक गाँठ, ulong load_addr, ulong आकार, स्थिर अक्षर *desc)
{
	काष्ठा upl *upl = &s_upl;
	काष्ठा upl_image img;

	img.load = load_addr;
	img.आकार = आकार;
	img.offset = गाँठ;
	img.description = desc;
	अगर (!alist_add(&upl->image, img))
		वापस -ENOMEM;
	log_debug("upl: add image %s at %lx size %lx\n", desc, load_addr, आकार);

	वापस 0;
}

अटल पूर्णांक write_serial(काष्ठा upl_serial *ser)
{
	काष्ठा udevice *dev = gd->cur_serial_dev;
	काष्ठा serial_device_info info;
	काष्ठा memregion region;
	पूर्णांक ret;

	अगर (!dev)
		वापस log_संदेश_ret("ser", -ENOENT);
	ret = serial_getinfo(dev, &info);
	अगर (ret)
		वापस log_संदेश_ret("inf", ret);

	ser->compatible = ofnode_read_string(dev_ofnode(dev), "compatible");
	ser->clock_frequency = info.घड़ी;
	ser->current_speed = gd->baudrate;
	region.base = info.addr;
	region.आकार = info.आकार;
	alist_init_struct(&ser->reg, काष्ठा memregion);
	अगर (!alist_add(&ser->reg, region))
		वापस -ENOMEM;
	ser->reg_io_shift = info.reg_shift;
	ser->reg_offset = info.reg_offset;
	ser->reg_io_width = info.reg_width;
	ser->virtual_reg = 0;
	ser->access_type = info.addr_space;

	वापस 0;
}

अटल पूर्णांक write_graphics(काष्ठा upl_graphics *gra)
{
	काष्ठा video_uc_plat *plat;
	काष्ठा video_priv *priv;
	काष्ठा memregion region;
	काष्ठा udevice *dev;

	alist_init_struct(&gra->reg, काष्ठा memregion);
	uclass_find_first_device(UCLASS_VIDEO, &dev);
	अगर (!dev || !device_active(dev))
		वापस log_संदेश_ret("vid", -ENOENT);

	plat = dev_get_uclass_plat(dev);
	region.base = plat->base;
	region.आकार = plat->आकार;
	अगर (!alist_add(&gra->reg, region))
		वापस log_संदेश_ret("reg", -ENOMEM);

	priv = dev_get_uclass_priv(dev);
	gra->width = priv->xsize;
	gra->height = priv->ysize;
	gra->stride = priv->पंक्ति_लंबाई;	/* private field */
	चयन (priv->format) {
	हाल VIDEO_RGBA8888:
	हाल VIDEO_X8R8G8B8:
		gra->format = UPLGF_ARGB32;
		अवरोध;
	हाल VIDEO_X8B8G8R8:
		gra->format = UPLGF_ABGR32;
		अवरोध;
	हाल VIDEO_X2R10G10B10:
		log_debug("device '%s': VIDEO_X2R10G10B10 not supported\n",
			  dev->name);
		वापस log_संदेश_ret("for", -EPROTO);
	हाल VIDEO_UNKNOWN:
		log_debug("device '%s': Unknown video format\n", dev->name);
		वापस log_संदेश_ret("for", -EPROTO);
	}

	वापस 0;
}

पूर्णांक spl_write_upl_handoff(काष्ठा spl_image_info *spl_image)
{
	काष्ठा upl *upl = &s_upl;
	काष्ठा abuf buf;
	ofnode मूल;
	व्योम *ptr;
	पूर्णांक ret;

	log_debug("UPL: Writing handoff - image_count=%d\n", upl->image.गिनती);
	upl->addr_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->आकार_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->bootmode = UPLBM_DEFAULT;
	ret = write_serial(&upl->serial);
	अगर (ret)
		वापस log_संदेश_ret("ser", ret);
	ret = write_graphics(&upl->graphics);
	अगर (ret && ret != -ENOENT)
		वापस log_संदेश_ret("gra", ret);

	मूल = ofnode_मूल();
	ret = upl_write_handoff(upl, मूल, true);
	अगर (ret)
		वापस log_संदेश_ret("wr", ret);

	ret = oftree_to_fdt(oftree_default(), &buf);
	अगर (ret)
		वापस log_संदेश_ret("fdt", ret);
	log_debug("FDT size %zx\n", abuf_आकार(&buf));

	ptr = bloblist_add(BLOBLISTT_CONTROL_FDT, abuf_आकार(&buf), 0);
	अगर (!ptr)
		वापस log_संदेश_ret("blo", -ENOENT);
	स_नकल(ptr, abuf_तथ्य(&buf), abuf_आकार(&buf));

	वापस 0;
}

व्योम spl_upl_init(व्योम)
{
	upl_init(&s_upl);
}
