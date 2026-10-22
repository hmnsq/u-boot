// SPDX-License-Identifier: GPL-2.0+
/*
 * UPL handoff parsing
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#تعریف_کرو LOG_CATEGORY UCLASS_BOOTSTD

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

DECLARE_GLOBAL_انکڑے_PTR;

بنية upl s_upl;

فراغ upl_سیٹ_fit_addr(ulong fit)
{
	بنية upl *upl = &s_upl;

	upl->fit = fit;
}

فراغ upl_سیٹ_fit_info(ulong fit, صحيح conf_وتھ, ulong اندراج_addr)
{
	بنية upl *upl = &s_upl;

	upl->fit = fit;
	upl->conf_وتھ = conf_وتھ;
	log_debug("upl: add fit %lx conf %x\n", fit, conf_وتھ);
}

صحيح upl_جوڑو_image_2(صحيح گرہ, ulong لدو_addr, ulong ناپ, ثابت حرف *desc)
{
	بنية upl *upl = &s_upl;
	بنية upl_image img;

	img.لدو = لدو_addr;
	img.ناپ = ناپ;
	img.وتھ = گرہ;
	img.description = desc;
	إذا (!alist_جوڑو(&upl->image, img))
		أرجع -ENOMEM;
	log_debug("upl: add image %s at %lx size %lx\n", desc, لدو_addr, ناپ);

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
	ser->current_رفتار = gd->baudrate;
	region.ادھار = info.addr;
	region.ناپ = info.ناپ;
	alist_init_struct(&ser->reg, بنية memregion);
	إذا (!alist_جوڑو(&ser->reg, region))
		أرجع -ENOMEM;
	ser->reg_io_کھسکاؤ = info.reg_کھسکاؤ;
	ser->reg_وتھ = info.reg_وتھ;
	ser->reg_io_چوڑائی = info.reg_چوڑائی;
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
	uclass_لبھو_پہلا_device(UCLASS_VIDEO, &dev);
	إذا (!dev || !device_سرگرم(dev))
		أرجع log_پیغام_ret("vid", -ENOENT);

	plat = dev_get_uclass_plat(dev);
	region.ادھار = plat->ادھار;
	region.ناپ = plat->ناپ;
	إذا (!alist_جوڑو(&gra->reg, region))
		أرجع log_پیغام_ret("reg", -ENOMEM);

	priv = dev_get_uclass_priv(dev);
	gra->چوڑائی = priv->xsize;
	gra->اچائی = priv->ysize;
	gra->stride = priv->سطر_لمائی;	/* private field */
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
	ofnode جڑ;
	فراغ *ptr;
	صحيح ret;

	log_debug("UPL: Writing handoff - image_count=%d\n", upl->image.گنتی);
	upl->addr_cells = IS_ENABLED(CONFIG_PHYS_64_ثنائی_ہندسہ) ? 2 : 1;
	upl->ناپ_cells = IS_ENABLED(CONFIG_PHYS_64_ثنائی_ہندسہ) ? 2 : 1;
	upl->bootmode = UPLBM_DEFAULT;
	ret = write_serial(&upl->serial);
	إذا (ret)
		أرجع log_پیغام_ret("ser", ret);
	ret = write_graphics(&upl->graphics);
	إذا (ret && ret != -ENOENT)
		أرجع log_پیغام_ret("gra", ret);

	جڑ = ofnode_جڑ();
	ret = upl_write_handoff(upl, جڑ, true);
	إذا (ret)
		أرجع log_پیغام_ret("wr", ret);

	ret = oftree_منزل_fdt(oftree_default(), &buf);
	إذا (ret)
		أرجع log_پیغام_ret("fdt", ret);
	log_debug("FDT size %zx\n", abuf_ناپ(&buf));

	ptr = bloblist_جوڑو(BLOBLISTT_CONTROL_FDT, abuf_ناپ(&buf), 0);
	إذا (!ptr)
		أرجع log_پیغام_ret("blo", -ENOENT);
	حافظہ_نقل(ptr, abuf_انکڑے(&buf), abuf_ناپ(&buf));

	أرجع 0;
}

فراغ spl_upl_init(فراغ)
{
	upl_init(&s_upl);
}
