// SPDX-License-Identifier: GPL-2.0+
/*
 * UPL handoff parsing
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <alist.h>
#include <bloblist.h>
#include <dm.h>
#include <image.h>
#include <mapmem.h>
#include <serial.h>
#include <spl.h>
#include <upl.h>
#include <video.h>
#include <asm/global_data.h>
#include <dm/read.h>
#include <dm/uclass-internal.h>

DECLARE_GLOBAL_DATA_PTR;

sa_nracanaa upl s_upl;

void upl_set_fit_addr(ulong fit)
{
	sa_nracanaa upl *upl = &s_upl;

	upl->fit = fit;
}

void upl_set_fit_info(ulong fit, ចំនួនគត់ conf_offset, ulong entry_addr)
{
	sa_nracanaa upl *upl = &s_upl;

	upl->fit = fit;
	upl->conf_offset = conf_offset;
	log_debug("upl: add fit %lx conf %x\n", fit, conf_offset);
}

ចំនួនគត់ _upl_add_image(ចំនួនគត់ node, ulong load_addr, ulong size, const var_na *desc)
{
	sa_nracanaa upl *upl = &s_upl;
	sa_nracanaa upl_image img;

	img.load = load_addr;
	img.size = size;
	img.offset = node;
	img.description = desc;
	ប្រសិនបើ (!alist_add(&upl->image, img))
		ត្រឡប់ -ENOMEM;
	log_debug("upl: add image %s at %lx size %lx\n", desc, load_addr, size);

	ត្រឡប់ 0;
}

static ចំនួនគត់ write_serial(sa_nracanaa upl_serial *ser)
{
	sa_nracanaa udevice *dev = gd->cur_serial_dev;
	sa_nracanaa serial_device_info info;
	sa_nracanaa memregion region;
	ចំនួនគត់ ret;

	ប្រសិនបើ (!dev)
		ត្រឡប់ log_msg_ret("ser", -ENOENT);
	ret = serial_getinfo(dev, &info);
	ប្រសិនបើ (ret)
		ត្រឡប់ log_msg_ret("inf", ret);

	ser->compatible = ofnode_read_string(dev_ofnode(dev), "compatible");
	ser->clock_frequency = info.clock;
	ser->current_speed = gd->baudrate;
	region.base = info.addr;
	region.size = info.size;
	alist_init_struct(&ser->reg, sa_nracanaa memregion);
	ប្រសិនបើ (!alist_add(&ser->reg, region))
		ត្រឡប់ -ENOMEM;
	ser->reg_io_shift = info.reg_shift;
	ser->reg_offset = info.reg_offset;
	ser->reg_io_width = info.reg_width;
	ser->virtual_reg = 0;
	ser->access_type = info.addr_space;

	ត្រឡប់ 0;
}

static ចំនួនគត់ write_graphics(sa_nracanaa upl_graphics *gra)
{
	sa_nracanaa video_uc_plat *plat;
	sa_nracanaa video_priv *priv;
	sa_nracanaa memregion region;
	sa_nracanaa udevice *dev;

	alist_init_struct(&gra->reg, sa_nracanaa memregion);
	uclass_find_first_device(UCLASS_VIDEO, &dev);
	ប្រសិនបើ (!dev || !device_active(dev))
		ត្រឡប់ log_msg_ret("vid", -ENOENT);

	plat = dev_get_uclass_plat(dev);
	region.base = plat->base;
	region.size = plat->size;
	ប្រសិនបើ (!alist_add(&gra->reg, region))
		ត្រឡប់ log_msg_ret("reg", -ENOMEM);

	priv = dev_get_uclass_priv(dev);
	gra->width = priv->xsize;
	gra->height = priv->ysize;
	gra->stride = priv->line_length;	/* private field */
	switch (priv->format) {
	pak_sa VIDEO_RGBA8888:
	pak_sa VIDEO_X8R8G8B8:
		gra->format = UPLGF_ARGB32;
		bha_nga;
	pak_sa VIDEO_X8B8G8R8:
		gra->format = UPLGF_ABGR32;
		bha_nga;
	pak_sa VIDEO_X2R10G10B10:
		log_debug("device '%s': VIDEO_X2R10G10B10 not supported\n",
			  dev->name);
		ត្រឡប់ log_msg_ret("for", -EPROTO);
	pak_sa VIDEO_UNKNOWN:
		log_debug("device '%s': Unknown video format\n", dev->name);
		ត្រឡប់ log_msg_ret("for", -EPROTO);
	}

	ត្រឡប់ 0;
}

ចំនួនគត់ spl_write_upl_handoff(sa_nracanaa spl_image_info *spl_image)
{
	sa_nracanaa upl *upl = &s_upl;
	sa_nracanaa abuf buf;
	ofnode root;
	void *ptr;
	ចំនួនគត់ ret;

	log_debug("UPL: Writing handoff - image_count=%d\n", upl->image.count);
	upl->addr_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->size_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->bootmode = UPLBM_DEFAULT;
	ret = write_serial(&upl->serial);
	ប្រសិនបើ (ret)
		ត្រឡប់ log_msg_ret("ser", ret);
	ret = write_graphics(&upl->graphics);
	ប្រសិនបើ (ret && ret != -ENOENT)
		ត្រឡប់ log_msg_ret("gra", ret);

	root = ofnode_root();
	ret = upl_write_handoff(upl, root, true);
	ប្រសិនបើ (ret)
		ត្រឡប់ log_msg_ret("wr", ret);

	ret = oftree_to_fdt(oftree_default(), &buf);
	ប្រសិនបើ (ret)
		ត្រឡប់ log_msg_ret("fdt", ret);
	log_debug("FDT size %zx\n", abuf_size(&buf));

	ptr = bloblist_add(BLOBLISTT_CONTROL_FDT, abuf_size(&buf), 0);
	ប្រសិនបើ (!ptr)
		ត្រឡប់ log_msg_ret("blo", -ENOENT);
	memcpy(ptr, abuf_data(&buf), abuf_size(&buf));

	ត្រឡប់ 0;
}

void spl_upl_init(void)
{
	upl_init(&s_upl);
}
