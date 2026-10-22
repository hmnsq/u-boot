// SPDX-License-Identifier: GPL-2.0+
/*
 * UPL handoff parsing
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#నిర్వచించు LOG_CATEGORY UCLASS_BOOTSTD

#అన్తర్భావ <alist.h>
#అన్తర్భావ <bloblist.h>
#అన్తర్భావ <dm.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <mapmem.h>
#అన్తర్భావ <serial.h>
#అన్తర్భావ <spl.h>
#అన్తర్భావ <upl.h>
#అన్తర్భావ <video.h>
#అన్తర్భావ <asm/global_data.h>
#అన్తర్భావ <dm/read.h>
#అన్తర్భావ <dm/uclass-internal.h>

DECLARE_GLOBAL_దత్తాంశం_PTR;

నిర్మాణం upl s_upl;

శూన్య upl_సమితి_fit_addr(ulong fit)
{
	నిర్మాణం upl *upl = &s_upl;

	upl->fit = fit;
}

శూన్య upl_సమితి_fit_info(ulong fit, పూర్ణాంకం conf_స్థానభ్రంశం, ulong నమోదు_addr)
{
	నిర్మాణం upl *upl = &s_upl;

	upl->fit = fit;
	upl->conf_స్థానభ్రంశం = conf_స్థానభ్రంశం;
	log_debug("upl: add fit %lx conf %x\n", fit, conf_స్థానభ్రంశం);
}

పూర్ణాంకం upl_కూడు_image_2(పూర్ణాంకం కణుపు, ulong ఎక్కించు_addr, ulong కొలత, స్థిరరాశి అక్షరం *desc)
{
	నిర్మాణం upl *upl = &s_upl;
	నిర్మాణం upl_image img;

	img.ఎక్కించు = ఎక్కించు_addr;
	img.కొలత = కొలత;
	img.స్థానభ్రంశం = కణుపు;
	img.description = desc;
	ఒకవేళ (!alist_కూడు(&upl->image, img))
		తిరిగి -ENOMEM;
	log_debug("upl: add image %s at %lx size %lx\n", desc, ఎక్కించు_addr, కొలత);

	తిరిగి 0;
}

స్థిర పూర్ణాంకం write_serial(నిర్మాణం upl_serial *ser)
{
	నిర్మాణం udevice *dev = gd->cur_serial_dev;
	నిర్మాణం serial_device_info info;
	నిర్మాణం memregion region;
	పూర్ణాంకం ret;

	ఒకవేళ (!dev)
		తిరిగి log_సందేశం_ret("ser", -ENOENT);
	ret = serial_getinfo(dev, &info);
	ఒకవేళ (ret)
		తిరిగి log_సందేశం_ret("inf", ret);

	ser->compatible = ofnode_read_string(dev_ofnode(dev), "compatible");
	ser->clock_frequency = info.clock;
	ser->current_వేగం = gd->baudrate;
	region.పునాది = info.addr;
	region.కొలత = info.కొలత;
	alist_init_struct(&ser->reg, నిర్మాణం memregion);
	ఒకవేళ (!alist_కూడు(&ser->reg, region))
		తిరిగి -ENOMEM;
	ser->reg_io_జరుపు = info.reg_జరుపు;
	ser->reg_స్థానభ్రంశం = info.reg_స్థానభ్రంశం;
	ser->reg_io_వెడల్పు = info.reg_వెడల్పు;
	ser->virtual_reg = 0;
	ser->access_type = info.addr_space;

	తిరిగి 0;
}

స్థిర పూర్ణాంకం write_graphics(నిర్మాణం upl_graphics *gra)
{
	నిర్మాణం video_uc_plat *plat;
	నిర్మాణం video_priv *priv;
	నిర్మాణం memregion region;
	నిర్మాణం udevice *dev;

	alist_init_struct(&gra->reg, నిర్మాణం memregion);
	uclass_కనుగొను_మొదటి_device(UCLASS_VIDEO, &dev);
	ఒకవేళ (!dev || !device_చురుకైన(dev))
		తిరిగి log_సందేశం_ret("vid", -ENOENT);

	plat = dev_get_uclass_plat(dev);
	region.పునాది = plat->పునాది;
	region.కొలత = plat->కొలత;
	ఒకవేళ (!alist_కూడు(&gra->reg, region))
		తిరిగి log_సందేశం_ret("reg", -ENOMEM);

	priv = dev_get_uclass_priv(dev);
	gra->వెడల్పు = priv->xsize;
	gra->ఎత్తు = priv->ysize;
	gra->stride = priv->పంక్తి_నిడివి;	/* private field */
	ఎంపిక (priv->format) {
	స్థితి VIDEO_RGBA8888:
	స్థితి VIDEO_X8R8G8B8:
		gra->format = UPLGF_ARGB32;
		ఆపు;
	స్థితి VIDEO_X8B8G8R8:
		gra->format = UPLGF_ABGR32;
		ఆపు;
	స్థితి VIDEO_X2R10G10B10:
		log_debug("device '%s': VIDEO_X2R10G10B10 not supported\n",
			  dev->name);
		తిరిగి log_సందేశం_ret("for", -EPROTO);
	స్థితి VIDEO_UNKNOWN:
		log_debug("device '%s': Unknown video format\n", dev->name);
		తిరిగి log_సందేశం_ret("for", -EPROTO);
	}

	తిరిగి 0;
}

పూర్ణాంకం spl_write_upl_handoff(నిర్మాణం spl_image_info *spl_image)
{
	నిర్మాణం upl *upl = &s_upl;
	నిర్మాణం abuf buf;
	ofnode మూలం;
	శూన్య *ptr;
	పూర్ణాంకం ret;

	log_debug("UPL: Writing handoff - image_count=%d\n", upl->image.లెక్క);
	upl->addr_cells = IS_ENABLED(CONFIG_PHYS_64_బిట్టు) ? 2 : 1;
	upl->కొలత_cells = IS_ENABLED(CONFIG_PHYS_64_బిట్టు) ? 2 : 1;
	upl->bootmode = UPLBM_DEFAULT;
	ret = write_serial(&upl->serial);
	ఒకవేళ (ret)
		తిరిగి log_సందేశం_ret("ser", ret);
	ret = write_graphics(&upl->graphics);
	ఒకవేళ (ret && ret != -ENOENT)
		తిరిగి log_సందేశం_ret("gra", ret);

	మూలం = ofnode_మూలం();
	ret = upl_write_handoff(upl, మూలం, true);
	ఒకవేళ (ret)
		తిరిగి log_సందేశం_ret("wr", ret);

	ret = oftree_గమ్యం_fdt(oftree_default(), &buf);
	ఒకవేళ (ret)
		తిరిగి log_సందేశం_ret("fdt", ret);
	log_debug("FDT size %zx\n", abuf_కొలత(&buf));

	ptr = bloblist_కూడు(BLOBLISTT_CONTROL_FDT, abuf_కొలత(&buf), 0);
	ఒకవేళ (!ptr)
		తిరిగి log_సందేశం_ret("blo", -ENOENT);
	జ్ఞాపకం_నకలు(ptr, abuf_దత్తాంశం(&buf), abuf_కొలత(&buf));

	తిరిగి 0;
}

శూన్య spl_upl_init(శూన్య)
{
	upl_init(&s_upl);
}
