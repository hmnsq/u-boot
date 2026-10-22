// SPDX-License-Identifier: GPL-2.0+
/*
 * UPL handoff parsing
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#تعریف_کن LOG_CATEGORY UCLASS_BOOTSTD

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

DECLARE_GLOBAL_داده_PTR;

بنية upl s_upl;

فراغ upl_مجموعه_fit_addr(ulong fit)
{
	بنية upl *upl = &s_upl;

	upl->fit = fit;
}

فراغ upl_مجموعه_fit_info(ulong fit, صحيح conf_جابه_جایی, ulong مدخل_addr)
{
	بنية upl *upl = &s_upl;

	upl->fit = fit;
	upl->conf_جابه_جایی = conf_جابه_جایی;
	log_debug("upl: add fit %lx conf %x\n", fit, conf_جابه_جایی);
}

صحيح upl_جمع_کن_image_2(صحيح گره, ulong بارگیری_addr, ulong اندازه, ثابت حرف *desc)
{
	بنية upl *upl = &s_upl;
	بنية upl_image img;

	img.بارگیری = بارگیری_addr;
	img.اندازه = اندازه;
	img.جابه_جایی = گره;
	img.description = desc;
	إذا (!alist_جمع_کن(&upl->image, img))
		أرجع -ENOMEM;
	log_debug("upl: add image %s at %lx size %lx\n", desc, بارگیری_addr, اندازه);

	أرجع 0;
}

ساكن صحيح write_serial(بنية upl_serial *ser)
{
	بنية udevice *dev = gd->cur_serial_dev;
	بنية serial_device_info info;
	بنية memregion region;
	صحيح ret;

	إذا (!dev)
		أرجع log_پیام_ret("ser", -ENOENT);
	ret = serial_getinfo(dev, &info);
	إذا (ret)
		أرجع log_پیام_ret("inf", ret);

	ser->compatible = ofnode_read_string(dev_ofnode(dev), "compatible");
	ser->clock_frequency = info.clock;
	ser->current_سرعت = gd->baudrate;
	region.پایه = info.addr;
	region.اندازه = info.اندازه;
	alist_init_struct(&ser->reg, بنية memregion);
	إذا (!alist_جمع_کن(&ser->reg, region))
		أرجع -ENOMEM;
	ser->reg_io_لغزش = info.reg_لغزش;
	ser->reg_جابه_جایی = info.reg_جابه_جایی;
	ser->reg_io_عرض = info.reg_عرض;
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
	uclass_پیدا_کن_اولی_device(UCLASS_VIDEO, &dev);
	إذا (!dev || !device_فعال(dev))
		أرجع log_پیام_ret("vid", -ENOENT);

	plat = dev_get_uclass_plat(dev);
	region.پایه = plat->پایه;
	region.اندازه = plat->اندازه;
	إذا (!alist_جمع_کن(&gra->reg, region))
		أرجع log_پیام_ret("reg", -ENOMEM);

	priv = dev_get_uclass_priv(dev);
	gra->عرض = priv->xsize;
	gra->ارتفاع = priv->ysize;
	gra->stride = priv->سطر_درازی;	/* private field */
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
		أرجع log_پیام_ret("for", -EPROTO);
	حالة VIDEO_UNKNOWN:
		log_debug("device '%s': Unknown video format\n", dev->name);
		أرجع log_پیام_ret("for", -EPROTO);
	}

	أرجع 0;
}

صحيح spl_write_upl_handoff(بنية spl_image_info *spl_image)
{
	بنية upl *upl = &s_upl;
	بنية abuf buf;
	ofnode ریشه;
	فراغ *ptr;
	صحيح ret;

	log_debug("UPL: Writing handoff - image_count=%d\n", upl->image.شمار);
	upl->addr_cells = IS_ENABLED(CONFIG_PHYS_64_بیت) ? 2 : 1;
	upl->اندازه_cells = IS_ENABLED(CONFIG_PHYS_64_بیت) ? 2 : 1;
	upl->bootmode = UPLBM_DEFAULT;
	ret = write_serial(&upl->serial);
	إذا (ret)
		أرجع log_پیام_ret("ser", ret);
	ret = write_graphics(&upl->graphics);
	إذا (ret && ret != -ENOENT)
		أرجع log_پیام_ret("gra", ret);

	ریشه = ofnode_ریشه();
	ret = upl_write_handoff(upl, ریشه, true);
	إذا (ret)
		أرجع log_پیام_ret("wr", ret);

	ret = oftree_مقصد_fdt(oftree_default(), &buf);
	إذا (ret)
		أرجع log_پیام_ret("fdt", ret);
	log_debug("FDT size %zx\n", abuf_اندازه(&buf));

	ptr = bloblist_جمع_کن(BLOBLISTT_CONTROL_FDT, abuf_اندازه(&buf), 0);
	إذا (!ptr)
		أرجع log_پیام_ret("blo", -ENOENT);
	کپی_حافظه(ptr, abuf_داده(&buf), abuf_اندازه(&buf));

	أرجع 0;
}

فراغ spl_upl_init(فراغ)
{
	upl_init(&s_upl);
}
