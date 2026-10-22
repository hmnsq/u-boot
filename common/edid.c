// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012 The Chromium OS Authors.
 *
 * (C) Copyright 2010
 * Petr Stetiar <ynezz@true.cz>
 *
 * Contains stolen code from ddcprobe project which is:
 * Copyright (C) Nalin Dahyabhai <bigfun@pobox.com>
 */

#ضمن <edid.h>
#ضمن <errno.h>
#ضمن <fdtdec.h>
#ضمن <log.h>
#ضمن <linux/ctype.h>
#ضمن <linux/string.h>

#إذا CONFIG_IS_ENABLED(I2C_EDID_STANDARD)
#وصف_ڪريو TIMING(c, ha, hfp, hbp, hsl, va, vfp, vbp, vsl, f)	\
	.pixelclock = { (c), (c), (c) },			\
	.hactive = { (ha), (ha), (ha) },			\
	.hfront_porch = { (hfp), (hfp), (hfp) },		\
	.hback_porch = { (hbp), (hbp), (hbp) },			\
	.hsync_len = { (hsl), (hsl), (hsl) },			\
	.vactive = { (va), (va), (va) },			\
	.vfront_porch = { (vfp), (vfp), (vfp) },		\
	.vback_porch = { (vbp), (vbp), (vbp) },			\
	.vsync_len = { (vsl), (vsl), (vsl) },			\
	.flags = (f)

ساكن ثابت بنية display_timing dmt_timings[] = {
	{ TIMING(31500000, 640, 32, 64, 96, 350, 32, 3, 60,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(31500000, 640, 32, 64, 96, 400, 1, 3, 41,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(35500000, 720, 36, 72, 108, 400, 1, 3, 42,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(25175000, 640, 16, 96, 48, 480, 10, 2, 33,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(31500000, 640, 24, 40, 128, 480, 9, 3, 28,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(31500000, 640, 16, 64, 120, 480, 1, 3, 16,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(36000000, 640, 56, 56, 80, 480, 1, 3, 25,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(36000000, 800, 24, 72, 128, 600, 1, 2, 22,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(40000000, 800, 40, 128, 88, 600, 1, 4, 23,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(50000000, 800, 56, 120, 64, 600, 37, 6, 23,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(49500000, 800, 16, 80, 160, 600, 1, 3, 21,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(56250000, 800, 32, 64, 152, 600, 1, 3, 27,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(73250000, 800, 48, 32, 80, 600, 3, 4, 29,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(33750000, 848, 16, 112, 112, 480, 6, 8, 23,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(44900000, 1024, 8, 176, 56, 768, 0, 8, 41,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(65000000, 1024, 24, 136, 160, 768, 3, 6, 29,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(75000000, 1024, 24, 136, 144, 768, 3, 6, 29,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(78750000, 1024, 16, 96, 176, 768, 1, 3, 28,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(94500000, 1024, 48, 96, 208, 768, 1, 3, 36,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(115500000, 1024, 48, 32, 80, 768, 3, 4, 38,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(108000000, 1152, 64, 128, 256, 864, 1, 3, 32,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(74250000, 1280, 110, 40, 220, 720, 5, 5, 20,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(68250000, 1280, 48, 32, 80, 768, 3, 7, 12,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(79500000, 1280, 64, 128, 192, 768, 3, 7, 20,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(102250000, 1280, 80, 128, 208, 768, 3, 7, 27,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(117500000, 1280, 80, 136, 216, 768, 3, 7, 31,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(140250000, 1280, 48, 32, 80, 768, 3, 7, 35,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(71000000, 1280, 48, 32, 80, 800, 3, 6, 14,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(83500000, 1280, 72, 128, 200, 800, 3, 6, 22,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(106500000, 1280, 80, 128, 208, 800, 3, 6, 29,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(122500000, 1280, 80, 136, 216, 800, 3, 6, 34,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(146250000, 1280, 48, 32, 80, 800, 3, 6, 38,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(108000000, 1280, 96, 112, 312, 960, 1, 3, 36,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(148500000, 1280, 64, 160, 224, 960, 1, 3, 47,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(175500000, 1280, 48, 32, 80, 960, 3, 4, 50,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(108000000, 1280, 48, 112, 248, 1024, 1, 3, 38,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(135000000, 1280, 16, 144, 248, 1024, 1, 3, 38,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(157500000, 1280, 64, 160, 224, 1024, 1, 3, 44,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(187250000, 1280, 48, 32, 80, 1024, 3, 7, 50,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(85500000, 1360, 64, 112, 256, 768, 3, 6, 18,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(148250000, 1360, 48, 32, 80, 768, 3, 5, 37,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(85500000, 1366, 70, 143, 213, 768, 3, 3, 24,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(72000000, 1366, 14, 56, 64, 768, 1, 3, 28,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(101000000, 1400, 48, 32, 80, 1050, 3, 4, 23,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(121750000, 1400, 88, 144, 232, 1050, 3, 4, 32,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(156000000, 1400, 104, 144, 248, 1050, 3, 4, 42,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(179500000, 1400, 104, 152, 256, 1050, 3, 4, 48,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(208000000, 1400, 48, 32, 80, 1050, 3, 4, 55,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(88750000, 1440, 48, 32, 80, 900, 3, 6, 17,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(106500000, 1440, 80, 152, 232, 900, 3, 6, 25,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(136750000, 1440, 96, 152, 248, 900, 3, 6, 33,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(157000000, 1440, 104, 152, 256, 900, 3, 6, 39,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(182750000, 1440, 48, 32, 80, 900, 3, 6, 44,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(108000000, 1600, 24, 80, 96, 900, 1, 3, 96,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(162000000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(175500000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(189000000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(202500000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(229500000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(268250000, 1600, 48, 32, 80, 1200, 3, 4, 64,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(119000000, 1680, 48, 32, 80, 1050, 3, 6, 21,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(146250000, 1680, 104, 176, 280, 1050, 3, 6, 30,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(187000000, 1680, 120, 176, 296, 1050, 3, 6, 40,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(214750000, 1680, 128, 176, 304, 1050, 3, 6, 46,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(245500000, 1680, 48, 32, 80, 1050, 3, 6, 53,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(204750000, 1792, 128, 200, 328, 1344, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(261000000, 1792, 96, 216, 352, 1344, 1, 3, 69,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(333250000, 1792, 48, 32, 80, 1344, 3, 4, 72,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(218250000, 1856, 96, 224, 352, 1392, 1, 3, 43,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(288000000, 1856, 128, 224, 352, 1392, 1, 3, 104,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(356500000, 1856, 48, 32, 80, 1392, 3, 4, 75,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(148500000, 1920, 88, 44, 148, 1080, 4, 5, 36,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(154000000, 1920, 48, 32, 80, 1200, 3, 6, 26,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(193250000, 1920, 136, 200, 336, 1200, 3, 6, 36,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(245250000, 1920, 136, 208, 344, 1200, 3, 6, 46,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(281250000, 1920, 144, 208, 352, 1200, 3, 6, 53,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(317000000, 1920, 48, 32, 80, 1200, 3, 6, 62,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(234000000, 1920, 128, 208, 344, 1440, 1, 3, 56,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(297000000, 1920, 144, 224, 352, 1440, 1, 3, 56,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(380500000, 1920, 48, 32, 80, 1440, 3, 4, 78,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(162000000, 2048, 26, 80, 96, 1152, 1, 3, 44,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(268500000, 2560, 48, 32, 80, 1600, 3, 6, 37,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(348500000, 2560, 192, 280, 472, 1600, 3, 6, 49,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(443250000, 2560, 208, 280, 488, 1600, 3, 6, 63,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(505250000, 2560, 208, 280, 488, 1600, 3, 6, 73,
		 DISPLAY_FLAGS_HSYNC_هيٺيون | DISPLAY_FLAGS_VSYNC_مٿاهون) },
	{ TIMING(552750000, 2560, 48, 32, 80, 1600, 3, 6, 85,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(556744000, 4096, 8, 32, 40, 2160, 48, 8, 6,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
	{ TIMING(556188000, 4096, 8, 32, 40, 2160, 48, 8, 6,
		 DISPLAY_FLAGS_HSYNC_مٿاهون | DISPLAY_FLAGS_VSYNC_هيٺيون) },
};
#جيڪڏهن_پورو

صحيح edid_check_info(بنية edid1_info *edid_info)
{
	إذا ((edid_info == عدم) || (edid_info->version == 0))
		أرجع -1;

	إذا (memcmp(edid_info->سرو, "\x0\xff\xff\xff\xff\xff\xff\x0", 8))
		أرجع -1;

	إذا (edid_info->version == 0xff && edid_info->revision == 0xff)
		أرجع -1;

	أرجع 0;
}

صحيح edid_check_checksum(u8 *edid_بلاڪ)
{
	u8 checksum = 0;
	صحيح i;

	لكل (i = 0; i < 128; i++)
		checksum += edid_بلاڪ[i];

	أرجع (checksum == 0) ? 0 : -EINVAL;
}

صحيح edid_get_ranges(بنية edid1_info *edid, بنانشان صحيح *hmin,
		    بنانشان صحيح *hmax, بنانشان صحيح *vmin,
		    بنانشان صحيح *vmax)
{
	صحيح i;
	بنية edid_monitor_descriptor *monitor;

	*hmin = *hmax = *vmin = *vmax = 0;
	إذا (edid_check_info(edid))
		أرجع -1;

	لكل (i = 0; i < ARRAY_سائيز(edid->monitor_details.descriptor); i++) {
		monitor = &edid->monitor_details.descriptor[i];
		إذا (monitor->type == EDID_MONITOR_DESCRIPTOR_RANGE) {
			*hmin = monitor->معلومات.range_معلومات.horizontal_min;
			*hmax = monitor->معلومات.range_معلومات.horizontal_max;
			*vmin = monitor->معلومات.range_معلومات.vertical_min;
			*vmax = monitor->معلومات.range_معلومات.vertical_max;
			أرجع 0;
		}
	}
	أرجع -1;
}

/* Set all parts of a timing entry to the same value */
ساكن فراغ سيٽ_داخلا(بنية timing_داخلا *داخلا_2, u32 قدر)
{
	داخلا_2->min = قدر;
	داخلا_2->typ = قدر;
	داخلا_2->max = قدر;
}

/**
 * decode_timing() - Decoding an 18-byte detailed timing record
 *
 * @buf:	Pointer to EDID detailed timing record
 * @timing:	Place to put timing
 */
ساكن فراغ decode_timing(u8 *buf, بنية display_timing *timing)
{
	uint x_mm, y_mm;
	بنانشان صحيح ha, hbl, hso, hspw, hborder;
	بنانشان صحيح va, vbl, vso, vspw, vborder;
	بنية edid_detailed_timing *t = (بنية edid_detailed_timing *)buf;

	/* Edid contains pixel clock in terms of 10KHz */
	سيٽ_داخلا(&timing->pixelclock, (buf[0] + (buf[1] << 8)) * 10000);
	x_mm = (buf[12] + ((buf[14] & 0xf0) << 4));
	y_mm = (buf[13] + ((buf[14] & 0x0f) << 8));
	ha = (buf[2] + ((buf[4] & 0xf0) << 4));
	hbl = (buf[3] + ((buf[4] & 0x0f) << 8));
	hso = (buf[8] + ((buf[11] & 0xc0) << 2));
	hspw = (buf[9] + ((buf[11] & 0x30) << 4));
	hborder = buf[15];
	va = (buf[5] + ((buf[7] & 0xf0) << 4));
	vbl = (buf[6] + ((buf[7] & 0x0f) << 8));
	vso = ((buf[10] >> 4) + ((buf[11] & 0x0c) << 2));
	vspw = ((buf[10] & 0x0f) + ((buf[11] & 0x03) << 4));
	vborder = buf[16];

	سيٽ_داخلا(&timing->hactive, ha);
	سيٽ_داخلا(&timing->hfront_porch, hso);
	سيٽ_داخلا(&timing->hback_porch, hbl - hso - hspw);
	سيٽ_داخلا(&timing->hsync_len, hspw);

	سيٽ_داخلا(&timing->vactive, va);
	سيٽ_داخلا(&timing->vfront_porch, vso);
	سيٽ_داخلا(&timing->vback_porch, vbl - vso - vspw);
	سيٽ_داخلا(&timing->vsync_len, vspw);

	timing->flags = 0;
	إذا (EDID_DETAILED_TIMING_FLAG_HSYNC_POLARITY(*t))
		timing->flags |= DISPLAY_FLAGS_HSYNC_مٿاهون;
	وإلا
		timing->flags |= DISPLAY_FLAGS_HSYNC_هيٺيون;
	إذا (EDID_DETAILED_TIMING_FLAG_VSYNC_POLARITY(*t))
		timing->flags |= DISPLAY_FLAGS_VSYNC_مٿاهون;
	وإلا
		timing->flags |= DISPLAY_FLAGS_VSYNC_هيٺيون;

	إذا (EDID_DETAILED_TIMING_FLAG_INTERLACED(*t))
		timing->flags = DISPLAY_FLAGS_INTERLACED;

	debug("Detailed mode clock %u Hz, %d mm x %d mm\n"
	      "               %04x %04x %04x %04x hborder %x\n"
	      "               %04x %04x %04x %04x vborder %x\n",
	      timing->pixelclock.typ,
	      x_mm, y_mm,
	      ha, ha + hso, ha + hso + hspw,
	      ha + hbl, hborder,
	      va, va + vso, va + vso + vspw,
	      va + vbl, vborder);
}

/**
 * Check if HDMI vendor specific data block is present in CEA block
 * @param info	CEA extension block
 * Return: true if block is found
 */
ساكن bool cea_is_hdmi_vsdb_present(بنية edid_cea861_info *info)
{
	u8 پڇاڙي_2, i = 0;

	/* check for end of data block */
	پڇاڙي_2 = info->dtd_وٿي;
	إذا (پڇاڙي_2 == 0)
		پڇاڙي_2 = حجم(info->معلومات);
	إذا (پڇاڙي_2 < 4 || پڇاڙي_2 > حجم(info->معلومات))
		أرجع false;
	پڇاڙي_2 -= 4;

	طالما (i < پڇاڙي_2) {
		/* Look for vendor specific data block of appropriate size */
		إذا ((EDID_CEA861_DB_TYPE(*info, i) == EDID_CEA861_DB_VENDOR) &&
		    (EDID_CEA861_DB_LEN(*info, i) >= 5)) {
			u8 *db = &info->معلومات[i + 1];
			u32 oui = db[0] | (db[1] << 8) | (db[2] << 16);

			إذا (oui == HDMI_IEEE_OUI)
				أرجع true;
		}
		i += EDID_CEA861_DB_LEN(*info, i) + 1;
	}

	أرجع false;
}

ساكن bool edid_ڳوليو_valid_detailed_timing(فراغ *buf, صحيح ڳڻپ,
					    بنية display_timing *timing,
					    bool (*گهڻائي_valid)(فراغ *priv,
						ثابت بنية display_timing *timing),
					    فراغ *گهڻائي_valid_priv)
{
	بنية edid_detailed_timing *t = buf;
	bool مليو = false;
	صحيح i;

	لكل (i = 0; i < ڳڻپ && !مليو; i++, t++)
		إذا (EDID_DETAILED_TIMING_PIXEL_CLOCK(*t) != 0) {
			decode_timing((u8 *)t, timing);
			إذا (گهڻائي_valid)
				مليو = گهڻائي_valid(گهڻائي_valid_priv,
						   timing);
			وإلا
				مليو = true;
		}

	أرجع مليو;
}

ساكن bool edid_get_standard_timing(بنية edid1_info *edid, صحيح i, بنانشان صحيح *x,
				     بنانشان صحيح *y, بنانشان صحيح *freq)
{
	بنانشان صحيح aspect = 10000;
	بنانشان حرف xres, vfreq;

	xres = EDID1_INFO_STANDARD_TIMING_XRESOLUTION(*edid, i);
	vfreq = EDID1_INFO_STANDARD_TIMING_VFREQ(*edid, i);
	إذا (xres != vfreq || (xres != 0 && xres != 1) ||
	    (vfreq != 0 && vfreq != 1)) {
		بدل (EDID1_INFO_STANDARD_TIMING_ASPECT(*edid, i)) {
		حالة ASPECT_625: // 16:10
			aspect = 6250;
			اكسر;
		حالة ASPECT_75: // 4:3
			aspect = 7500;
			اكسر;
		حالة ASPECT_8: // 5:4
			aspect = 8000;
			اكسر;
		حالة ASPECT_5625: // 16:9
			aspect = 5625;
			اكسر;
		}

		*x = (xres + 31) * 8;
		*y = *x * aspect / 10000;
		*freq = (vfreq & 0x3f) + 60;

		أرجع true;
	}

	أرجع false;
}

#إذا CONFIG_IS_ENABLED(I2C_EDID_STANDARD)
ساكن bool edid_ڳوليو_valid_standard_timing(بنية edid1_info *buf,
					    بنية display_timing *timing,
					    bool (*گهڻائي_valid)(فراغ *priv,
						ثابت بنية display_timing *timing),
					    فراغ *گهڻائي_valid_priv)
{
	بنانشان صحيح x, y, freq;
	bool مليو = false;
	صحيح i, k;

	لكل (i = 0; i < ARRAY_سائيز(buf->standard_timings); i++) {
		إذا (!edid_get_standard_timing(buf, i, &x, &y, &freq))
			استمر;

		لكل (k = 0; k < ARRAY_سائيز(dmt_timings); k++) {
			ثابت بنية display_timing *dt = &dmt_timings[k];

			إذا (dt->hactive.typ == x && dt->vactive.typ == y) {
				مليو = گهڻائي_valid(گهڻائي_valid_priv, dt);
				إذا (مليو) {
					يادگيري_نقل(timing, dt, حجم(*timing));
					أرجع true;
				}
			}
		}
	}

	أرجع مليو;
}
#جيڪڏهن_پورو

صحيح edid_get_timing_validate(u8 *buf, صحيح buf_سائيز,
			     بنية display_timing *timing,
			     صحيح *panel_بٽ_في_colourp,
			     bool (*گهڻائي_valid)(فراغ *priv,
					ثابت بنية display_timing *timing),
			     فراغ *گهڻائي_valid_priv)
{
	بنية edid1_info *edid = (بنية edid1_info *)buf;
	bool مليو;

	إذا (buf_سائيز < حجم(*edid) || edid_check_info(edid)) {
		debug("%s: Invalid buffer\n", __func__);
		أرجع -EINVAL;
	}

	إذا (!EDID1_INFO_VIDEO_داخلا_DIGITAL(*edid)) {
		debug("%s: Not a digital display\n", __func__);
		أرجع -ENOSYS;
	}

	إذا (!EDID1_INFO_FEATURE_PREFERRED_TIMING_گهڻائي(*edid)) {
		debug("%s: No preferred timing\n", __func__);
		أرجع -ENOENT;
	}

	/* Look for detailed timing in base EDID */
	مليو = edid_ڳوليو_valid_detailed_timing(edid->monitor_details.descriptor, 4,
						timing, گهڻائي_valid, گهڻائي_valid_priv);

	/* Look for detailed timing in CTA-861 Extension Block */
	إذا (!مليو && edid->extension_flag && buf_سائيز >= EDID_EXT_سائيز) {
		بنية edid_cea861_info *info =
			(بنية edid_cea861_info *)(buf + حجم(*edid));

		إذا (info->extension_ٽيگ == EDID_CEA861_EXTENSION_ٽيگ) {
			صحيح ڳڻپ = EDID_CEA861_DTD_ڳڻپ(*info);
			صحيح وٿي = info->dtd_وٿي;
			صحيح سائيز = ڳڻپ * حجم(بنية edid_detailed_timing);

			إذا (وٿي >= 4 && وٿي + سائيز < EDID_سائيز)
				مليو = edid_ڳوليو_valid_detailed_timing(
					(u8 *)info + وٿي, ڳڻپ, timing,
					گهڻائي_valid, گهڻائي_valid_priv);
		}
	}

#إذا CONFIG_IS_ENABLED(I2C_EDID_STANDARD)
	/* Look for timing in Standard Timings */
	إذا (!مليو)
		مليو = edid_ڳوليو_valid_standard_timing(edid, timing, گهڻائي_valid,
							گهڻائي_valid_priv);
#جيڪڏهن_پورو

	إذا (!مليو)
		أرجع -EINVAL;

	إذا (edid->version != 1 || edid->revision < 4) {
		debug("%s: EDID version %d.%d does not have required info\n",
		      __func__, edid->version, edid->revision);
		*panel_بٽ_في_colourp = -1;
	} وإلا  {
		*panel_بٽ_في_colourp =
			((edid->video_داخلا_definition & 0x70) >> 3) + 4;
	}

	timing->hdmi_monitor = false;
	إذا (edid->extension_flag && (buf_سائيز >= EDID_EXT_سائيز)) {
		بنية edid_cea861_info *info =
			(بنية edid_cea861_info *)(buf + حجم(*edid));

		إذا (info->extension_ٽيگ == EDID_CEA861_EXTENSION_ٽيگ)
			timing->hdmi_monitor = cea_is_hdmi_vsdb_present(info);
	}

	أرجع 0;
}

صحيح edid_get_timing(u8 *buf, صحيح buf_سائيز, بنية display_timing *timing,
		    صحيح *panel_بٽ_في_colourp)
{
	أرجع edid_get_timing_validate(buf, buf_سائيز, timing,
					panel_بٽ_في_colourp, عدم, عدم);
}

/**
 * Snip the tailing whitespace/return of a string.
 *
 * @param string	The string to be snipped
 * Return: the snipped string
 */
ساكن حرف *snip(حرف *string)
{
	حرف *s;

	/*
	 * This is always a 13 character buffer
	 * and it's not always terminated.
	 */
	string[12] = '\0';
	s = &string[لڙي_ڊيگهه(string) - 1];

	طالما (s >= string && (خال_آهي(*s) || *s == '\n' || *s == '\r' ||
			*s == '\0'))
		*(s--) = '\0';

	أرجع string;
}

/**
 * Print an EDID monitor descriptor block
 *
 * @param monitor	The EDID monitor descriptor block
 * @have_timing		Modifies to 1 if the desciptor contains timing info
 */
ساكن فراغ edid_print_dtd(بنية edid_monitor_descriptor *monitor,
			   بنانشان صحيح *have_timing)
{
	بنانشان حرف *بائيٽون = (بنانشان حرف *)monitor;
	بنية edid_detailed_timing *timing =
			(بنية edid_detailed_timing *)monitor;

	إذا (بائيٽون[0] == 0 && بائيٽون[1] == 0) {
		إذا (monitor->type == EDID_MONITOR_DESCRIPTOR_SERIAL)
			لکو("Monitor serial number: %s\n",
			       snip(monitor->معلومات.string));
		وإلا إذا (monitor->type == EDID_MONITOR_DESCRIPTOR_ASCII)
			لکو("Monitor ID: %s\n",
			       snip(monitor->معلومات.string));
		وإلا إذا (monitor->type == EDID_MONITOR_DESCRIPTOR_NAME)
			لکو("Monitor name: %s\n",
			       snip(monitor->معلومات.string));
		وإلا إذا (monitor->type == EDID_MONITOR_DESCRIPTOR_RANGE)
			لکو("Monitor range limits, horizontal sync: "
			       "%d-%d kHz, vertical refresh: "
			       "%d-%d Hz, max pixel clock: "
			       "%d MHz\n",
			       monitor->معلومات.range_معلومات.horizontal_min,
			       monitor->معلومات.range_معلومات.horizontal_max,
			       monitor->معلومات.range_معلومات.vertical_min,
			       monitor->معلومات.range_معلومات.vertical_max,
			       monitor->معلومات.range_معلومات.pixel_clock_max * 10);
	} وإلا {
		uint32_t pixclock, h_سرگرم, h_blanking, v_سرگرم, v_blanking;
		uint32_t h_ڪل, v_ڪل, vfreq;

		pixclock = EDID_DETAILED_TIMING_PIXEL_CLOCK(*timing);
		h_سرگرم = EDID_DETAILED_TIMING_HORIZONTAL_سرگرم(*timing);
		h_blanking = EDID_DETAILED_TIMING_HORIZONTAL_BLANKING(*timing);
		v_سرگرم = EDID_DETAILED_TIMING_VERTICAL_سرگرم(*timing);
		v_blanking = EDID_DETAILED_TIMING_VERTICAL_BLANKING(*timing);

		h_ڪل = h_سرگرم + h_blanking;
		v_ڪل = v_سرگرم + v_blanking;
		إذا (v_ڪل > 0 && h_ڪل > 0)
			vfreq = pixclock / (v_ڪل * h_ڪل);
		وإلا
			vfreq = 1; /* Error case */
		لکو("\t%dx%d\%c\t%d Hz (detailed)\n", h_سرگرم,
		       v_سرگرم, h_سرگرم > 1000 ? ' ' : '\t', vfreq);
		*have_timing = 1;
	}
}

/**
 * Get the manufacturer name from an EDID info.
 *
 * @param edid_info     The EDID info to be printed
 * @param name		Returns the string of the manufacturer name
 */
ساكن فراغ edid_get_manufacturer_name(بنية edid1_info *edid, حرف *name)
{
	name[0] = EDID1_INFO_MANUFACTURER_NAME_CHAR1(*edid) + 'A' - 1;
	name[1] = EDID1_INFO_MANUFACTURER_NAME_CHAR2(*edid) + 'A' - 1;
	name[2] = EDID1_INFO_MANUFACTURER_NAME_CHAR3(*edid) + 'A' - 1;
	name[3] = '\0';
}

فراغ edid_print_info(بنية edid1_info *edid_info)
{
	صحيح i;
	حرف manufacturer[4];
	بنانشان صحيح have_timing = 0;
	uint32_t serial_عدد;

	إذا (edid_check_info(edid_info)) {
		لکو("Not a valid EDID\n");
		أرجع;
	}

	لکو("EDID version: %d.%d\n",
	       edid_info->version, edid_info->revision);

	لکو("Product ID code: %04x\n", EDID1_INFO_حاصل_ضرب_ڪوڊ(*edid_info));

	edid_get_manufacturer_name(edid_info, manufacturer);
	لکو("Manufacturer: %s\n", manufacturer);

	serial_عدد = EDID1_INFO_SERIAL_عدد(*edid_info);
	إذا (serial_عدد != 0xffffffff) {
		إذا (strcmp(manufacturer, "MAG") == 0)
			serial_عدد -= 0x7000000;
		إذا (strcmp(manufacturer, "OQI") == 0)
			serial_عدد -= 456150000;
		إذا (strcmp(manufacturer, "VSC") == 0)
			serial_عدد -= 640000000;
	}
	لکو("Serial number: %08x\n", serial_عدد);
	لکو("Manufactured in week: %d year: %d\n",
	       edid_info->week, edid_info->year + 1990);

	لکو("Video input definition: %svoltage level %d%s%s%s%s%s\n",
	       EDID1_INFO_VIDEO_داخلا_DIGITAL(*edid_info) ?
	       "digital signal, " : "analog signal, ",
	       EDID1_INFO_VIDEO_داخلا_VOLTAGE_LEVEL(*edid_info),
	       EDID1_INFO_VIDEO_داخلا_BLANK_منزل_BLACK(*edid_info) ?
	       ", blank to black" : "",
	       EDID1_INFO_VIDEO_داخلا_SEPARATE_SYNC(*edid_info) ?
	       ", separate sync" : "",
	       EDID1_INFO_VIDEO_داخلا_مرکب_SYNC(*edid_info) ?
	       ", composite sync" : "",
	       EDID1_INFO_VIDEO_داخلا_SYNC_ON_GREEN(*edid_info) ?
	       ", sync on green" : "",
	       EDID1_INFO_VIDEO_داخلا_SERRATION_V(*edid_info) ?
	       ", serration v" : "");

	لکو("Monitor is %s\n",
	       EDID1_INFO_FEATURE_RGB(*edid_info) ? "RGB" : "non-RGB");

	لکو("Maximum visible display size: %d cm x %d cm\n",
	       edid_info->max_سائيز_horizontal,
	       edid_info->max_سائيز_vertical);

	لکو("Power management features: %s%s, %s%s, %s%s\n",
	       EDID1_INFO_FEATURE_سرگرم_OFF(*edid_info) ?
	       "" : "no ", "active off",
	       EDID1_INFO_FEATURE_SUSPEND(*edid_info) ? "" : "no ", "suspend",
	       EDID1_INFO_FEATURE_STANDBY(*edid_info) ? "" : "no ", "standby");

	لکو("Estabilished timings:\n");
	إذا (EDID1_INFO_ESTABLISHED_TIMING_720X400_70(*edid_info))
		لکو("\t720x400\t\t70 Hz (VGA 640x400, IBM)\n");
	إذا (EDID1_INFO_ESTABLISHED_TIMING_720X400_88(*edid_info))
		لکو("\t720x400\t\t88 Hz (XGA2)\n");
	إذا (EDID1_INFO_ESTABLISHED_TIMING_640X480_60(*edid_info))
		لکو("\t640x480\t\t60 Hz (VGA)\n");
	إذا (EDID1_INFO_ESTABLISHED_TIMING_640X480_67(*edid_info))
		لکو("\t640x480\t\t67 Hz (Mac II, Apple)\n");
	إذا (EDID1_INFO_ESTABLISHED_TIMING_640X480_72(*edid_info))
		لکو("\t640x480\t\t72 Hz (VESA)\n");
	إذا (EDID1_INFO_ESTABLISHED_TIMING_640X480_75(*edid_info))
		لکو("\t640x480\t\t75 Hz (VESA)\n");
	إذا (EDID1_INFO_ESTABLISHED_TIMING_800X600_56(*edid_info))
		لکو("\t800x600\t\t56 Hz (VESA)\n");
	إذا (EDID1_INFO_ESTABLISHED_TIMING_800X600_60(*edid_info))
		لکو("\t800x600\t\t60 Hz (VESA)\n");
	إذا (EDID1_INFO_ESTABLISHED_TIMING_800X600_72(*edid_info))
		لکو("\t800x600\t\t72 Hz (VESA)\n");
	إذا (EDID1_INFO_ESTABLISHED_TIMING_800X600_75(*edid_info))
		لکو("\t800x600\t\t75 Hz (VESA)\n");
	إذا (EDID1_INFO_ESTABLISHED_TIMING_832X624_75(*edid_info))
		لکو("\t832x624\t\t75 Hz (Mac II)\n");
	إذا (EDID1_INFO_ESTABLISHED_TIMING_1024X768_87I(*edid_info))
		لکو("\t1024x768\t87 Hz Interlaced (8514A)\n");
	إذا (EDID1_INFO_ESTABLISHED_TIMING_1024X768_60(*edid_info))
		لکو("\t1024x768\t60 Hz (VESA)\n");
	إذا (EDID1_INFO_ESTABLISHED_TIMING_1024X768_70(*edid_info))
		لکو("\t1024x768\t70 Hz (VESA)\n");
	إذا (EDID1_INFO_ESTABLISHED_TIMING_1024X768_75(*edid_info))
		لکو("\t1024x768\t75 Hz (VESA)\n");
	إذا (EDID1_INFO_ESTABLISHED_TIMING_1280X1024_75(*edid_info))
		لکو("\t1280x1024\t75 (VESA)\n");
	إذا (EDID1_INFO_ESTABLISHED_TIMING_1152X870_75(*edid_info))
		لکو("\t1152x870\t75 (Mac II)\n");

	/* Standard timings. */
	لکو("Standard timings:\n");
	لكل (i = 0; i < ARRAY_سائيز(edid_info->standard_timings); i++) {
		بنانشان صحيح x, y, freq;

		إذا (edid_get_standard_timing(edid_info, i, &x, &y, &freq)) {
			لکو("\t%dx%d%c\t%d Hz\n", x, y,
			       x > 1000 ? ' ' : '\t', freq);
			have_timing = 1;
		}
	}

	/* Detailed timing information. */
	لكل (i = 0; i < ARRAY_سائيز(edid_info->monitor_details.descriptor);
			i++) {
		edid_print_dtd(&edid_info->monitor_details.descriptor[i],
			       &have_timing);
	}

	إذا (!have_timing)
		لکو("\tNone\n");
}
