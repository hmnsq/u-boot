// SPDX-License-Identifier: GPL-2.0+
/*
 * UPL handoff parsing
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#تعریف_کړه LOG_CATEGORY UCLASS_BOOTSTD

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

DECLARE_GLOBAL_معلومات_PTR;

بنية upl s_upl;

فراغ upl_سېټ_fit_addr(ulong fit)
{
	بنية upl *upl = &s_upl;

	upl->fit = fit;
}

فراغ upl_سېټ_fit_info(ulong fit, صحيح conf_دننه_واټن, ulong ثبت_addr)
{
	بنية upl *upl = &s_upl;

	upl->fit = fit;
	upl->conf_دننه_واټن = conf_دننه_واټن;
	log_debug("upl: add fit %lx conf %x\n", fit, conf_دننه_واټن);
}

صحيح upl_جمع_کړه_image_2(صحيح غوټه, ulong بارول_addr, ulong اندازه, ثابت حرف *desc)
{
	بنية upl *upl = &s_upl;
	بنية upl_image img;

	img.بارول = بارول_addr;
	img.اندازه = اندازه;
	img.دننه_واټن = غوټه;
	img.description = desc;
	إذا (!alist_جمع_کړه(&upl->image, img))
		أرجع -ENOMEM;
	log_debug("upl: add image %s at %lx size %lx\n", desc, بارول_addr, اندازه);

	أرجع 0;
}

ساكن صحيح write_serial(بنية upl_serial *ser)
{
	بنية udevice *dev = gd->cur_serial_dev;
	بنية serial_device_info info;
	بنية memregion region;
	صحيح ret;

	إذا (!dev)
		أرجع log_پیغام_ret("ser", -ENOENT);
	ret = serial_getinfo(dev, &info);
	إذا (ret)
		أرجع log_پیغام_ret("inf", ret);

	ser->compatible = ofnode_read_string(dev_ofnode(dev), "compatible");
	ser->clock_frequency = info.clock;
	ser->current_چټکتیا = gd->baudrate;
	region.بنسټ = info.addr;
	region.اندازه = info.اندازه;
	alist_init_struct(&ser->reg, بنية memregion);
	إذا (!alist_جمع_کړه(&ser->reg, region))
		أرجع -ENOMEM;
	ser->reg_io_ښويدنه = info.reg_ښويدنه;
	ser->reg_دننه_واټن = info.reg_دننه_واټن;
	ser->reg_io_پلنوالی = info.reg_پلنوالی;
	ser->virtual_reg = 0;
	ser->access_type = info.addr_space;

	أرجع 0;
}

ساكن صحيح write_graphics(بنية upl_graphics *gra)
{
	بنية video_uc_plat *plat;
	بنية video_priv *priv;
	بنية memregion region;
	بنية udevice *dev;

	alist_init_struct(&gra->reg, بنية memregion);
	uclass_پیدا_کړه_لومړی_device(UCLASS_VIDEO, &dev);
	إذا (!dev || !device_فعال(dev))
		أرجع log_پیغام_ret("vid", -ENOENT);

	plat = dev_get_uclass_plat(dev);
	region.بنسټ = plat->بنسټ;
	region.اندازه = plat->اندازه;
	إذا (!alist_جمع_کړه(&gra->reg, region))
		أرجع log_پیغام_ret("reg", -ENOMEM);

	priv = dev_get_uclass_priv(dev);
	gra->پلنوالی = priv->xsize;
	gra->لوړوالی = priv->ysize;
	gra->stride = priv->کرښه_اوږدتیا;	/* private field */
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
		أرجع log_پیغام_ret("for", -EPROTO);
	حالة VIDEO_UNKNOWN:
		log_debug("device '%s': Unknown video format\n", dev->name);
		أرجع log_پیغام_ret("for", -EPROTO);
	}

	أرجع 0;
}

صحيح spl_write_upl_handoff(بنية spl_image_info *spl_image)
{
	بنية upl *upl = &s_upl;
	بنية abuf buf;
	ofnode ریښه;
	فراغ *ptr;
	صحيح ret;

	log_debug("UPL: Writing handoff - image_count=%d\n", upl->image.شمېر);
	upl->addr_cells = IS_ENABLED(CONFIG_PHYS_64_بټ) ? 2 : 1;
	upl->اندازه_cells = IS_ENABLED(CONFIG_PHYS_64_بټ) ? 2 : 1;
	upl->bootmode = UPLBM_DEFAULT;
	ret = write_serial(&upl->serial);
	إذا (ret)
		أرجع log_پیغام_ret("ser", ret);
	ret = write_graphics(&upl->graphics);
	إذا (ret && ret != -ENOENT)
		أرجع log_پیغام_ret("gra", ret);

	ریښه = ofnode_ریښه();
	ret = upl_write_handoff(upl, ریښه, true);
	إذا (ret)
		أرجع log_پیغام_ret("wr", ret);

	ret = oftree_منزل_fdt(oftree_default(), &buf);
	إذا (ret)
		أرجع log_پیغام_ret("fdt", ret);
	log_debug("FDT size %zx\n", abuf_اندازه(&buf));

	ptr = bloblist_جمع_کړه(BLOBLISTT_CONTROL_FDT, abuf_اندازه(&buf), 0);
	إذا (!ptr)
		أرجع log_پیغام_ret("blo", -ENOENT);
	حافظه_نقل(ptr, abuf_معلومات(&buf), abuf_اندازه(&buf));

	أرجع 0;
}

فراغ spl_upl_init(فراغ)
{
	upl_init(&s_upl);
}
