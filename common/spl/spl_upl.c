// SPDX-License-Identifier: GPL-2.0+
/*
 * UPL handoff parsing
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#وصف_ڪريو LOG_CATEGORY UCLASS_BOOTSTD

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

فراغ upl_سيٽ_fit_addr(ulong fit)
{
	بنية upl *upl = &s_upl;

	upl->fit = fit;
}

فراغ upl_سيٽ_fit_info(ulong fit, صحيح conf_وٿي, ulong داخلا_addr)
{
	بنية upl *upl = &s_upl;

	upl->fit = fit;
	upl->conf_وٿي = conf_وٿي;
	log_debug("upl: add fit %lx conf %x\n", fit, conf_وٿي);
}

صحيح upl_جوڙيو_image_2(صحيح ڳنڍ, ulong ڀريو_addr, ulong سائيز, ثابت حرف *desc)
{
	بنية upl *upl = &s_upl;
	بنية upl_image img;

	img.ڀريو = ڀريو_addr;
	img.سائيز = سائيز;
	img.وٿي = ڳنڍ;
	img.description = desc;
	إذا (!alist_جوڙيو(&upl->image, img))
		أرجع -ENOMEM;
	log_debug("upl: add image %s at %lx size %lx\n", desc, ڀريو_addr, سائيز);

	أرجع 0;
}

ساكن صحيح write_serial(بنية upl_serial *ser)
{
	بنية udevice *dev = gd->cur_serial_dev;
	بنية serial_device_info info;
	بنية memregion region;
	صحيح ret;

	إذا (!dev)
		أرجع log_نياپو_ret("ser", -ENOENT);
	ret = serial_getinfo(dev, &info);
	إذا (ret)
		أرجع log_نياپو_ret("inf", ret);

	ser->compatible = ofnode_read_string(dev_ofnode(dev), "compatible");
	ser->clock_frequency = info.clock;
	ser->current_رفتار = gd->baudrate;
	region.بنياد = info.addr;
	region.سائيز = info.سائيز;
	alist_init_struct(&ser->reg, بنية memregion);
	إذا (!alist_جوڙيو(&ser->reg, region))
		أرجع -ENOMEM;
	ser->reg_io_سرڪايو = info.reg_سرڪايو;
	ser->reg_وٿي = info.reg_وٿي;
	ser->reg_io_ويڪر = info.reg_ويڪر;
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
	uclass_ڳوليو_پهريون_device(UCLASS_VIDEO, &dev);
	إذا (!dev || !device_سرگرم(dev))
		أرجع log_نياپو_ret("vid", -ENOENT);

	plat = dev_get_uclass_plat(dev);
	region.بنياد = plat->بنياد;
	region.سائيز = plat->سائيز;
	إذا (!alist_جوڙيو(&gra->reg, region))
		أرجع log_نياپو_ret("reg", -ENOMEM);

	priv = dev_get_uclass_priv(dev);
	gra->ويڪر = priv->xsize;
	gra->اوچائي = priv->ysize;
	gra->stride = priv->سٽ_ڊيگهائي;	/* private field */
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
		أرجع log_نياپو_ret("for", -EPROTO);
	حالة VIDEO_UNKNOWN:
		log_debug("device '%s': Unknown video format\n", dev->name);
		أرجع log_نياپو_ret("for", -EPROTO);
	}

	أرجع 0;
}

صحيح spl_write_upl_handoff(بنية spl_image_info *spl_image)
{
	بنية upl *upl = &s_upl;
	بنية abuf buf;
	ofnode پاڙ;
	فراغ *ptr;
	صحيح ret;

	log_debug("UPL: Writing handoff - image_count=%d\n", upl->image.ڳڻپ);
	upl->addr_cells = IS_ENABLED(CONFIG_PHYS_64_ٻٽو_انگ) ? 2 : 1;
	upl->سائيز_cells = IS_ENABLED(CONFIG_PHYS_64_ٻٽو_انگ) ? 2 : 1;
	upl->bootmode = UPLBM_DEFAULT;
	ret = write_serial(&upl->serial);
	إذا (ret)
		أرجع log_نياپو_ret("ser", ret);
	ret = write_graphics(&upl->graphics);
	إذا (ret && ret != -ENOENT)
		أرجع log_نياپو_ret("gra", ret);

	پاڙ = ofnode_پاڙ();
	ret = upl_write_handoff(upl, پاڙ, true);
	إذا (ret)
		أرجع log_نياپو_ret("wr", ret);

	ret = oftree_منزل_fdt(oftree_default(), &buf);
	إذا (ret)
		أرجع log_نياپو_ret("fdt", ret);
	log_debug("FDT size %zx\n", abuf_سائيز(&buf));

	ptr = bloblist_جوڙيو(BLOBLISTT_CONTROL_FDT, abuf_سائيز(&buf), 0);
	إذا (!ptr)
		أرجع log_نياپو_ret("blo", -ENOENT);
	يادگيري_نقل(ptr, abuf_معلومات(&buf), abuf_سائيز(&buf));

	أرجع 0;
}

فراغ spl_upl_init(فراغ)
{
	upl_init(&s_upl);
}
