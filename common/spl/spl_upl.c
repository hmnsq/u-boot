// SPDX-License-Identifier: GPL-2.0+
/*
 * UPL handoff parsing
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#تعریف_کرو LOG_CATEGORY UCLASS_BOOTSTD

#شامل <alist.h>
#شامل <bloblist.h>
#شامل <dm.h>
#شامل <image.h>
#شامل <mapmem.h>
#شامل <serial.h>
#شامل <spl.h>
#شامل <upl.h>
#شامل <video.h>
#شامل <asm/global_data.h>
#شامل <dm/read.h>
#شامل <dm/uclass-internal.h>

DECLARE_GLOBAL_ڈیٹا_PTR;

ساخت upl s_upl;

خالی upl_سیٹ_fit_addr(ulong fit)
{
	ساخت upl *upl = &s_upl;

	upl->fit = fit;
}

خالی upl_سیٹ_fit_info(ulong fit, صحیح conf_انحراف, ulong اندراج_addr)
{
	ساخت upl *upl = &s_upl;

	upl->fit = fit;
	upl->conf_انحراف = conf_انحراف;
	log_debug("upl: add fit %lx conf %x\n", fit, conf_انحراف);
}

صحیح upl_جوڑو_image_2(صحیح گرہ, ulong بھرو_addr, ulong سائز, ثابت حرف *desc)
{
	ساخت upl *upl = &s_upl;
	ساخت upl_image img;

	img.بھرو = بھرو_addr;
	img.سائز = سائز;
	img.انحراف = گرہ;
	img.description = desc;
	اگر (!alist_جوڑو(&upl->image, img))
		واپس -ENOMEM;
	log_debug("upl: add image %s at %lx size %lx\n", desc, بھرو_addr, سائز);

	واپس 0;
}

ساکن صحیح write_serial(ساخت upl_serial *ser)
{
	ساخت udevice *dev = gd->cur_serial_dev;
	ساخت serial_device_info info;
	ساخت memregion region;
	صحیح ret;

	اگر (!dev)
		واپس log_پیغام_ret("ser", -ENOENT);
	ret = serial_getinfo(dev, &info);
	اگر (ret)
		واپس log_پیغام_ret("inf", ret);

	ser->compatible = ofnode_read_string(dev_ofnode(dev), "compatible");
	ser->clock_frequency = info.clock;
	ser->current_رفتار = gd->baudrate;
	region.بنیاد = info.addr;
	region.سائز = info.سائز;
	alist_init_struct(&ser->reg, ساخت memregion);
	اگر (!alist_جوڑو(&ser->reg, region))
		واپس -ENOMEM;
	ser->reg_io_سرکاؤ = info.reg_سرکاؤ;
	ser->reg_انحراف = info.reg_انحراف;
	ser->reg_io_چوڑائی = info.reg_چوڑائی;
	ser->virtual_reg = 0;
	ser->access_type = info.addr_space;

	واپس 0;
}

ساکن صحیح write_graphics(ساخت upl_graphics *gra)
{
	ساخت video_uc_plat *plat;
	ساخت video_priv *priv;
	ساخت memregion region;
	ساخت udevice *dev;

	alist_init_struct(&gra->reg, ساخت memregion);
	uclass_ڈھونڈو_پہلا_device(UCLASS_VIDEO, &dev);
	اگر (!dev || !device_فعال(dev))
		واپس log_پیغام_ret("vid", -ENOENT);

	plat = dev_get_uclass_plat(dev);
	region.بنیاد = plat->بنیاد;
	region.سائز = plat->سائز;
	اگر (!alist_جوڑو(&gra->reg, region))
		واپس log_پیغام_ret("reg", -ENOMEM);

	priv = dev_get_uclass_priv(dev);
	gra->چوڑائی = priv->xsize;
	gra->اونچائی = priv->ysize;
	gra->stride = priv->سطر_لمبائی;	/* private field */
	انتخاب (priv->format) {
	حالت VIDEO_RGBA8888:
	حالت VIDEO_X8R8G8B8:
		gra->format = UPLGF_ARGB32;
		توڑو;
	حالت VIDEO_X8B8G8R8:
		gra->format = UPLGF_ABGR32;
		توڑو;
	حالت VIDEO_X2R10G10B10:
		log_debug("device '%s': VIDEO_X2R10G10B10 not supported\n",
			  dev->name);
		واپس log_پیغام_ret("for", -EPROTO);
	حالت VIDEO_UNKNOWN:
		log_debug("device '%s': Unknown video format\n", dev->name);
		واپس log_پیغام_ret("for", -EPROTO);
	}

	واپس 0;
}

صحیح spl_write_upl_handoff(ساخت spl_image_info *spl_image)
{
	ساخت upl *upl = &s_upl;
	ساخت abuf buf;
	ofnode جڑ;
	خالی *ptr;
	صحیح ret;

	log_debug("UPL: Writing handoff - image_count=%d\n", upl->image.گنتی);
	upl->addr_cells = IS_ENABLED(CONFIG_PHYS_64_ثنائی_ہندسہ) ? 2 : 1;
	upl->سائز_cells = IS_ENABLED(CONFIG_PHYS_64_ثنائی_ہندسہ) ? 2 : 1;
	upl->bootmode = UPLBM_DEFAULT;
	ret = write_serial(&upl->serial);
	اگر (ret)
		واپس log_پیغام_ret("ser", ret);
	ret = write_graphics(&upl->graphics);
	اگر (ret && ret != -ENOENT)
		واپس log_پیغام_ret("gra", ret);

	جڑ = ofnode_جڑ();
	ret = upl_write_handoff(upl, جڑ, true);
	اگر (ret)
		واپس log_پیغام_ret("wr", ret);

	ret = oftree_مقصد_fdt(oftree_default(), &buf);
	اگر (ret)
		واپس log_پیغام_ret("fdt", ret);
	log_debug("FDT size %zx\n", abuf_سائز(&buf));

	ptr = bloblist_جوڑو(BLOBLISTT_CONTROL_FDT, abuf_سائز(&buf), 0);
	اگر (!ptr)
		واپس log_پیغام_ret("blo", -ENOENT);
	حافظہ_نقل(ptr, abuf_ڈیٹا(&buf), abuf_سائز(&buf));

	واپس 0;
}

خالی spl_upl_init(خالی)
{
	upl_init(&s_upl);
}
