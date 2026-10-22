// SPDX-License-Identifier: GPL-2.0+
/*
 * UPL handoff parsing
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LOG_CATEGORY UCLASS_BOOTSTD

#ਅਨ੍ਤਰ੍ਭਾਵ <alist.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <bloblist.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mapmem.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <serial.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <upl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <video.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm/read.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm/uclass-internal.h>

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

ਬਣਤਰ upl s_upl;

ਖਾਲੀ upl_ਸੈੱਟ_fit_addr(ulong fit)
{
	ਬਣਤਰ upl *upl = &s_upl;

	upl->fit = fit;
}

ਖਾਲੀ upl_ਸੈੱਟ_fit_info(ulong fit, ਪੂਰਨ conf_ਵਿੱਥ, ulong ਇੰਦਰਾਜ_addr)
{
	ਬਣਤਰ upl *upl = &s_upl;

	upl->fit = fit;
	upl->conf_ਵਿੱਥ = conf_ਵਿੱਥ;
	log_debug("upl: add fit %lx conf %x\n", fit, conf_ਵਿੱਥ);
}

ਪੂਰਨ upl_ਜੋੜੋ_image_2(ਪੂਰਨ ਗੰਢ, ulong ਲੱਦੋ_addr, ulong ਮਾਪ, ਅਚਲ ਅੱਖਰ *desc)
{
	ਬਣਤਰ upl *upl = &s_upl;
	ਬਣਤਰ upl_image img;

	img.ਲੱਦੋ = ਲੱਦੋ_addr;
	img.ਮਾਪ = ਮਾਪ;
	img.ਵਿੱਥ = ਗੰਢ;
	img.description = desc;
	ਜੇ (!alist_ਜੋੜੋ(&upl->image, img))
		ਵਾਪਸ -ENOMEM;
	log_debug("upl: add image %s at %lx size %lx\n", desc, ਲੱਦੋ_addr, ਮਾਪ);

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ write_serial(ਬਣਤਰ upl_serial *ser)
{
	ਬਣਤਰ udevice *dev = gd->cur_serial_dev;
	ਬਣਤਰ serial_device_info info;
	ਬਣਤਰ memregion region;
	ਪੂਰਨ ret;

	ਜੇ (!dev)
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("ser", -ENOENT);
	ret = serial_getinfo(dev, &info);
	ਜੇ (ret)
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("inf", ret);

	ser->compatible = ofnode_read_string(dev_ofnode(dev), "compatible");
	ser->clock_frequency = info.clock;
	ser->current_ਰਫਤਾਰ = gd->baudrate;
	region.ਅਧਾਰ = info.addr;
	region.ਮਾਪ = info.ਮਾਪ;
	alist_init_struct(&ser->reg, ਬਣਤਰ memregion);
	ਜੇ (!alist_ਜੋੜੋ(&ser->reg, region))
		ਵਾਪਸ -ENOMEM;
	ser->reg_io_ਖਿਸਕਾਓ = info.reg_ਖਿਸਕਾਓ;
	ser->reg_ਵਿੱਥ = info.reg_ਵਿੱਥ;
	ser->reg_io_ਚੌੜਾਈ = info.reg_ਚੌੜਾਈ;
	ser->virtual_reg = 0;
	ser->access_type = info.addr_space;

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ write_graphics(ਬਣਤਰ upl_graphics *gra)
{
	ਬਣਤਰ video_uc_plat *plat;
	ਬਣਤਰ video_priv *priv;
	ਬਣਤਰ memregion region;
	ਬਣਤਰ udevice *dev;

	alist_init_struct(&gra->reg, ਬਣਤਰ memregion);
	uclass_ਲੱਭੋ_ਪਹਿਲਾ_device(UCLASS_VIDEO, &dev);
	ਜੇ (!dev || !device_ਸਰਗਰਮ(dev))
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("vid", -ENOENT);

	plat = dev_get_uclass_plat(dev);
	region.ਅਧਾਰ = plat->ਅਧਾਰ;
	region.ਮਾਪ = plat->ਮਾਪ;
	ਜੇ (!alist_ਜੋੜੋ(&gra->reg, region))
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("reg", -ENOMEM);

	priv = dev_get_uclass_priv(dev);
	gra->ਚੌੜਾਈ = priv->xsize;
	gra->ਉਚਾਈ = priv->ysize;
	gra->stride = priv->ਸਤਰ_ਲੰਮਾਈ;	/* private field */
	ਚੋਣ (priv->format) {
	ਹਾਲਤ VIDEO_RGBA8888:
	ਹਾਲਤ VIDEO_X8R8G8B8:
		gra->format = UPLGF_ARGB32;
		ਰੋਕੋ;
	ਹਾਲਤ VIDEO_X8B8G8R8:
		gra->format = UPLGF_ABGR32;
		ਰੋਕੋ;
	ਹਾਲਤ VIDEO_X2R10G10B10:
		log_debug("device '%s': VIDEO_X2R10G10B10 not supported\n",
			  dev->name);
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("for", -EPROTO);
	ਹਾਲਤ VIDEO_UNKNOWN:
		log_debug("device '%s': Unknown video format\n", dev->name);
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("for", -EPROTO);
	}

	ਵਾਪਸ 0;
}

ਪੂਰਨ spl_write_upl_handoff(ਬਣਤਰ spl_image_info *spl_image)
{
	ਬਣਤਰ upl *upl = &s_upl;
	ਬਣਤਰ abuf buf;
	ofnode ਜੜ੍ਹ;
	ਖਾਲੀ *ptr;
	ਪੂਰਨ ret;

	log_debug("UPL: Writing handoff - image_count=%d\n", upl->image.ਗਿਣਤੀ_2);
	upl->addr_cells = IS_ENABLED(CONFIG_PHYS_64_ਦੋਆਧਾਰੀ_ਅੰਕ) ? 2 : 1;
	upl->ਮਾਪ_cells = IS_ENABLED(CONFIG_PHYS_64_ਦੋਆਧਾਰੀ_ਅੰਕ) ? 2 : 1;
	upl->bootmode = UPLBM_DEFAULT;
	ret = write_serial(&upl->serial);
	ਜੇ (ret)
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("ser", ret);
	ret = write_graphics(&upl->graphics);
	ਜੇ (ret && ret != -ENOENT)
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("gra", ret);

	ਜੜ੍ਹ = ofnode_ਜੜ੍ਹ();
	ret = upl_write_handoff(upl, ਜੜ੍ਹ, true);
	ਜੇ (ret)
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("wr", ret);

	ret = oftree_ਮੰਜ਼ਿਲ_fdt(oftree_default(), &buf);
	ਜੇ (ret)
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("fdt", ret);
	log_debug("FDT size %zx\n", abuf_ਮਾਪ(&buf));

	ptr = bloblist_ਜੋੜੋ(BLOBLISTT_CONTROL_FDT, abuf_ਮਾਪ(&buf), 0);
	ਜੇ (!ptr)
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("blo", -ENOENT);
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(ptr, abuf_ਅੰਕੜੇ(&buf), abuf_ਮਾਪ(&buf));

	ਵਾਪਸ 0;
}

ਖਾਲੀ spl_upl_init(ਖਾਲੀ)
{
	upl_init(&s_upl);
}
