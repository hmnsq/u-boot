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

#شامل <edid.h>
#شامل <errno.h>
#شامل <fdtdec.h>
#شامل <log.h>
#شامل <linux/ctype.h>
#شامل <linux/string.h>

#اگر CONFIG_IS_ENABLED(I2C_EDID_STANDARD)
#تعریف_کرو TIMING(c, ha, hfp, hbp, hsl, va, vfp, vbp, vsl, f)	\
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

ساکن ثابت ساخت display_timing dmt_timings[] = {
	{ TIMING(31500000, 640, 32, 64, 96, 350, 32, 3, 60,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(31500000, 640, 32, 64, 96, 400, 1, 3, 41,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(35500000, 720, 36, 72, 108, 400, 1, 3, 42,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(25175000, 640, 16, 96, 48, 480, 10, 2, 33,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(31500000, 640, 24, 40, 128, 480, 9, 3, 28,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(31500000, 640, 16, 64, 120, 480, 1, 3, 16,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(36000000, 640, 56, 56, 80, 480, 1, 3, 25,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(36000000, 800, 24, 72, 128, 600, 1, 2, 22,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(40000000, 800, 40, 128, 88, 600, 1, 4, 23,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(50000000, 800, 56, 120, 64, 600, 37, 6, 23,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(49500000, 800, 16, 80, 160, 600, 1, 3, 21,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(56250000, 800, 32, 64, 152, 600, 1, 3, 27,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(73250000, 800, 48, 32, 80, 600, 3, 4, 29,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(33750000, 848, 16, 112, 112, 480, 6, 8, 23,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(44900000, 1024, 8, 176, 56, 768, 0, 8, 41,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(65000000, 1024, 24, 136, 160, 768, 3, 6, 29,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(75000000, 1024, 24, 136, 144, 768, 3, 6, 29,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(78750000, 1024, 16, 96, 176, 768, 1, 3, 28,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(94500000, 1024, 48, 96, 208, 768, 1, 3, 36,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(115500000, 1024, 48, 32, 80, 768, 3, 4, 38,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(108000000, 1152, 64, 128, 256, 864, 1, 3, 32,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(74250000, 1280, 110, 40, 220, 720, 5, 5, 20,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(68250000, 1280, 48, 32, 80, 768, 3, 7, 12,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(79500000, 1280, 64, 128, 192, 768, 3, 7, 20,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(102250000, 1280, 80, 128, 208, 768, 3, 7, 27,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(117500000, 1280, 80, 136, 216, 768, 3, 7, 31,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(140250000, 1280, 48, 32, 80, 768, 3, 7, 35,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(71000000, 1280, 48, 32, 80, 800, 3, 6, 14,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(83500000, 1280, 72, 128, 200, 800, 3, 6, 22,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(106500000, 1280, 80, 128, 208, 800, 3, 6, 29,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(122500000, 1280, 80, 136, 216, 800, 3, 6, 34,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(146250000, 1280, 48, 32, 80, 800, 3, 6, 38,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(108000000, 1280, 96, 112, 312, 960, 1, 3, 36,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(148500000, 1280, 64, 160, 224, 960, 1, 3, 47,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(175500000, 1280, 48, 32, 80, 960, 3, 4, 50,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(108000000, 1280, 48, 112, 248, 1024, 1, 3, 38,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(135000000, 1280, 16, 144, 248, 1024, 1, 3, 38,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(157500000, 1280, 64, 160, 224, 1024, 1, 3, 44,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(187250000, 1280, 48, 32, 80, 1024, 3, 7, 50,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(85500000, 1360, 64, 112, 256, 768, 3, 6, 18,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(148250000, 1360, 48, 32, 80, 768, 3, 5, 37,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(85500000, 1366, 70, 143, 213, 768, 3, 3, 24,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(72000000, 1366, 14, 56, 64, 768, 1, 3, 28,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(101000000, 1400, 48, 32, 80, 1050, 3, 4, 23,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(121750000, 1400, 88, 144, 232, 1050, 3, 4, 32,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(156000000, 1400, 104, 144, 248, 1050, 3, 4, 42,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(179500000, 1400, 104, 152, 256, 1050, 3, 4, 48,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(208000000, 1400, 48, 32, 80, 1050, 3, 4, 55,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(88750000, 1440, 48, 32, 80, 900, 3, 6, 17,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(106500000, 1440, 80, 152, 232, 900, 3, 6, 25,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(136750000, 1440, 96, 152, 248, 900, 3, 6, 33,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(157000000, 1440, 104, 152, 256, 900, 3, 6, 39,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(182750000, 1440, 48, 32, 80, 900, 3, 6, 44,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(108000000, 1600, 24, 80, 96, 900, 1, 3, 96,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(162000000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(175500000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(189000000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(202500000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(229500000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(268250000, 1600, 48, 32, 80, 1200, 3, 4, 64,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(119000000, 1680, 48, 32, 80, 1050, 3, 6, 21,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(146250000, 1680, 104, 176, 280, 1050, 3, 6, 30,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(187000000, 1680, 120, 176, 296, 1050, 3, 6, 40,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(214750000, 1680, 128, 176, 304, 1050, 3, 6, 46,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(245500000, 1680, 48, 32, 80, 1050, 3, 6, 53,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(204750000, 1792, 128, 200, 328, 1344, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(261000000, 1792, 96, 216, 352, 1344, 1, 3, 69,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(333250000, 1792, 48, 32, 80, 1344, 3, 4, 72,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(218250000, 1856, 96, 224, 352, 1392, 1, 3, 43,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(288000000, 1856, 128, 224, 352, 1392, 1, 3, 104,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(356500000, 1856, 48, 32, 80, 1392, 3, 4, 75,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(148500000, 1920, 88, 44, 148, 1080, 4, 5, 36,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(154000000, 1920, 48, 32, 80, 1200, 3, 6, 26,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(193250000, 1920, 136, 200, 336, 1200, 3, 6, 36,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(245250000, 1920, 136, 208, 344, 1200, 3, 6, 46,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(281250000, 1920, 144, 208, 352, 1200, 3, 6, 53,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(317000000, 1920, 48, 32, 80, 1200, 3, 6, 62,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(234000000, 1920, 128, 208, 344, 1440, 1, 3, 56,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(297000000, 1920, 144, 224, 352, 1440, 1, 3, 56,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(380500000, 1920, 48, 32, 80, 1440, 3, 4, 78,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(162000000, 2048, 26, 80, 96, 1152, 1, 3, 44,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(268500000, 2560, 48, 32, 80, 1600, 3, 6, 37,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(348500000, 2560, 192, 280, 472, 1600, 3, 6, 49,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(443250000, 2560, 208, 280, 488, 1600, 3, 6, 63,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(505250000, 2560, 208, 280, 488, 1600, 3, 6, 73,
		 DISPLAY_FLAGS_HSYNC_کم | DISPLAY_FLAGS_VSYNC_بلند) },
	{ TIMING(552750000, 2560, 48, 32, 80, 1600, 3, 6, 85,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(556744000, 4096, 8, 32, 40, 2160, 48, 8, 6,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
	{ TIMING(556188000, 4096, 8, 32, 40, 2160, 48, 8, 6,
		 DISPLAY_FLAGS_HSYNC_بلند | DISPLAY_FLAGS_VSYNC_کم) },
};
#اگر_ختم

صحیح edid_check_info(ساخت edid1_info *edid_info)
{
	اگر ((edid_info == تہی) || (edid_info->version == 0))
		واپس -1;

	اگر (memcmp(edid_info->سرخی, "\x0\xff\xff\xff\xff\xff\xff\x0", 8))
		واپس -1;

	اگر (edid_info->version == 0xff && edid_info->revision == 0xff)
		واپس -1;

	واپس 0;
}

صحیح edid_check_checksum(u8 *edid_بلاک)
{
	u8 checksum = 0;
	صحیح i;

	برائے (i = 0; i < 128; i++)
		checksum += edid_بلاک[i];

	واپس (checksum == 0) ? 0 : -EINVAL;
}

صحیح edid_get_ranges(ساخت edid1_info *edid, غیرعلامتی صحیح *hmin,
		    غیرعلامتی صحیح *hmax, غیرعلامتی صحیح *vmin,
		    غیرعلامتی صحیح *vmax)
{
	صحیح i;
	ساخت edid_monitor_descriptor *monitor;

	*hmin = *hmax = *vmin = *vmax = 0;
	اگر (edid_check_info(edid))
		واپس -1;

	برائے (i = 0; i < ARRAY_سائز(edid->monitor_details.descriptor); i++) {
		monitor = &edid->monitor_details.descriptor[i];
		اگر (monitor->type == EDID_MONITOR_DESCRIPTOR_RANGE) {
			*hmin = monitor->ڈیٹا.range_ڈیٹا.horizontal_min;
			*hmax = monitor->ڈیٹا.range_ڈیٹا.horizontal_max;
			*vmin = monitor->ڈیٹا.range_ڈیٹا.vertical_min;
			*vmax = monitor->ڈیٹا.range_ڈیٹا.vertical_max;
			واپس 0;
		}
	}
	واپس -1;
}

/* Set all parts of a timing entry to the same value */
ساکن خالی سیٹ_اندراج(ساخت timing_اندراج *اندراج_2, u32 قدر)
{
	اندراج_2->min = قدر;
	اندراج_2->typ = قدر;
	اندراج_2->max = قدر;
}

/**
 * decode_timing() - Decoding an 18-byte detailed timing record
 *
 * @buf:	Pointer to EDID detailed timing record
 * @timing:	Place to put timing
 */
ساکن خالی decode_timing(u8 *buf, ساخت display_timing *timing)
{
	uint x_mm, y_mm;
	غیرعلامتی صحیح ha, hbl, hso, hspw, hborder;
	غیرعلامتی صحیح va, vbl, vso, vspw, vborder;
	ساخت edid_detailed_timing *t = (ساخت edid_detailed_timing *)buf;

	/* Edid contains pixel clock in terms of 10KHz */
	سیٹ_اندراج(&timing->pixelclock, (buf[0] + (buf[1] << 8)) * 10000);
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

	سیٹ_اندراج(&timing->hactive, ha);
	سیٹ_اندراج(&timing->hfront_porch, hso);
	سیٹ_اندراج(&timing->hback_porch, hbl - hso - hspw);
	سیٹ_اندراج(&timing->hsync_len, hspw);

	سیٹ_اندراج(&timing->vactive, va);
	سیٹ_اندراج(&timing->vfront_porch, vso);
	سیٹ_اندراج(&timing->vback_porch, vbl - vso - vspw);
	سیٹ_اندراج(&timing->vsync_len, vspw);

	timing->flags = 0;
	اگر (EDID_DETAILED_TIMING_FLAG_HSYNC_POLARITY(*t))
		timing->flags |= DISPLAY_FLAGS_HSYNC_بلند;
	ورنہ
		timing->flags |= DISPLAY_FLAGS_HSYNC_کم;
	اگر (EDID_DETAILED_TIMING_FLAG_VSYNC_POLARITY(*t))
		timing->flags |= DISPLAY_FLAGS_VSYNC_بلند;
	ورنہ
		timing->flags |= DISPLAY_FLAGS_VSYNC_کم;

	اگر (EDID_DETAILED_TIMING_FLAG_INTERLACED(*t))
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
ساکن bool cea_is_hdmi_vsdb_present(ساخت edid_cea861_info *info)
{
	u8 انتہا_2, i = 0;

	/* check for end of data block */
	انتہا_2 = info->dtd_انحراف;
	اگر (انتہا_2 == 0)
		انتہا_2 = حجم(info->ڈیٹا);
	اگر (انتہا_2 < 4 || انتہا_2 > حجم(info->ڈیٹا))
		واپس false;
	انتہا_2 -= 4;

	جب (i < انتہا_2) {
		/* Look for vendor specific data block of appropriate size */
		اگر ((EDID_CEA861_DB_TYPE(*info, i) == EDID_CEA861_DB_VENDOR) &&
		    (EDID_CEA861_DB_LEN(*info, i) >= 5)) {
			u8 *db = &info->ڈیٹا[i + 1];
			u32 oui = db[0] | (db[1] << 8) | (db[2] << 16);

			اگر (oui == HDMI_IEEE_OUI)
				واپس true;
		}
		i += EDID_CEA861_DB_LEN(*info, i) + 1;
	}

	واپس false;
}

ساکن bool edid_ڈھونڈو_valid_detailed_timing(خالی *buf, صحیح گنتی,
					    ساخت display_timing *timing,
					    bool (*منوال_valid)(خالی *priv,
						ثابت ساخت display_timing *timing),
					    خالی *منوال_valid_priv)
{
	ساخت edid_detailed_timing *t = buf;
	bool ملا = false;
	صحیح i;

	برائے (i = 0; i < گنتی && !ملا; i++, t++)
		اگر (EDID_DETAILED_TIMING_PIXEL_CLOCK(*t) != 0) {
			decode_timing((u8 *)t, timing);
			اگر (منوال_valid)
				ملا = منوال_valid(منوال_valid_priv,
						   timing);
			ورنہ
				ملا = true;
		}

	واپس ملا;
}

ساکن bool edid_get_standard_timing(ساخت edid1_info *edid, صحیح i, غیرعلامتی صحیح *x,
				     غیرعلامتی صحیح *y, غیرعلامتی صحیح *freq)
{
	غیرعلامتی صحیح aspect = 10000;
	غیرعلامتی حرف xres, vfreq;

	xres = EDID1_INFO_STANDARD_TIMING_XRESOLUTION(*edid, i);
	vfreq = EDID1_INFO_STANDARD_TIMING_VFREQ(*edid, i);
	اگر (xres != vfreq || (xres != 0 && xres != 1) ||
	    (vfreq != 0 && vfreq != 1)) {
		انتخاب (EDID1_INFO_STANDARD_TIMING_ASPECT(*edid, i)) {
		حالت ASPECT_625: // 16:10
			aspect = 6250;
			توڑو;
		حالت ASPECT_75: // 4:3
			aspect = 7500;
			توڑو;
		حالت ASPECT_8: // 5:4
			aspect = 8000;
			توڑو;
		حالت ASPECT_5625: // 16:9
			aspect = 5625;
			توڑو;
		}

		*x = (xres + 31) * 8;
		*y = *x * aspect / 10000;
		*freq = (vfreq & 0x3f) + 60;

		واپس true;
	}

	واپس false;
}

#اگر CONFIG_IS_ENABLED(I2C_EDID_STANDARD)
ساکن bool edid_ڈھونڈو_valid_standard_timing(ساخت edid1_info *buf,
					    ساخت display_timing *timing,
					    bool (*منوال_valid)(خالی *priv,
						ثابت ساخت display_timing *timing),
					    خالی *منوال_valid_priv)
{
	غیرعلامتی صحیح x, y, freq;
	bool ملا = false;
	صحیح i, k;

	برائے (i = 0; i < ARRAY_سائز(buf->standard_timings); i++) {
		اگر (!edid_get_standard_timing(buf, i, &x, &y, &freq))
			جاری;

		برائے (k = 0; k < ARRAY_سائز(dmt_timings); k++) {
			ثابت ساخت display_timing *dt = &dmt_timings[k];

			اگر (dt->hactive.typ == x && dt->vactive.typ == y) {
				ملا = منوال_valid(منوال_valid_priv, dt);
				اگر (ملا) {
					حافظہ_نقل(timing, dt, حجم(*timing));
					واپس true;
				}
			}
		}
	}

	واپس ملا;
}
#اگر_ختم

صحیح edid_get_timing_validate(u8 *buf, صحیح buf_سائز,
			     ساخت display_timing *timing,
			     صحیح *panel_بٹ_فی_colourp,
			     bool (*منوال_valid)(خالی *priv,
					ثابت ساخت display_timing *timing),
			     خالی *منوال_valid_priv)
{
	ساخت edid1_info *edid = (ساخت edid1_info *)buf;
	bool ملا;

	اگر (buf_سائز < حجم(*edid) || edid_check_info(edid)) {
		debug("%s: Invalid buffer\n", __func__);
		واپس -EINVAL;
	}

	اگر (!EDID1_INFO_VIDEO_داخلہ_DIGITAL(*edid)) {
		debug("%s: Not a digital display\n", __func__);
		واپس -ENOSYS;
	}

	اگر (!EDID1_INFO_FEATURE_PREFERRED_TIMING_منوال(*edid)) {
		debug("%s: No preferred timing\n", __func__);
		واپس -ENOENT;
	}

	/* Look for detailed timing in base EDID */
	ملا = edid_ڈھونڈو_valid_detailed_timing(edid->monitor_details.descriptor, 4,
						timing, منوال_valid, منوال_valid_priv);

	/* Look for detailed timing in CTA-861 Extension Block */
	اگر (!ملا && edid->extension_flag && buf_سائز >= EDID_EXT_سائز) {
		ساخت edid_cea861_info *info =
			(ساخت edid_cea861_info *)(buf + حجم(*edid));

		اگر (info->extension_لیبل == EDID_CEA861_EXTENSION_لیبل) {
			صحیح گنتی = EDID_CEA861_DTD_گنتی(*info);
			صحیح انحراف = info->dtd_انحراف;
			صحیح سائز = گنتی * حجم(ساخت edid_detailed_timing);

			اگر (انحراف >= 4 && انحراف + سائز < EDID_سائز)
				ملا = edid_ڈھونڈو_valid_detailed_timing(
					(u8 *)info + انحراف, گنتی, timing,
					منوال_valid, منوال_valid_priv);
		}
	}

#اگر CONFIG_IS_ENABLED(I2C_EDID_STANDARD)
	/* Look for timing in Standard Timings */
	اگر (!ملا)
		ملا = edid_ڈھونڈو_valid_standard_timing(edid, timing, منوال_valid,
							منوال_valid_priv);
#اگر_ختم

	اگر (!ملا)
		واپس -EINVAL;

	اگر (edid->version != 1 || edid->revision < 4) {
		debug("%s: EDID version %d.%d does not have required info\n",
		      __func__, edid->version, edid->revision);
		*panel_بٹ_فی_colourp = -1;
	} ورنہ  {
		*panel_بٹ_فی_colourp =
			((edid->video_داخلہ_definition & 0x70) >> 3) + 4;
	}

	timing->hdmi_monitor = false;
	اگر (edid->extension_flag && (buf_سائز >= EDID_EXT_سائز)) {
		ساخت edid_cea861_info *info =
			(ساخت edid_cea861_info *)(buf + حجم(*edid));

		اگر (info->extension_لیبل == EDID_CEA861_EXTENSION_لیبل)
			timing->hdmi_monitor = cea_is_hdmi_vsdb_present(info);
	}

	واپس 0;
}

صحیح edid_get_timing(u8 *buf, صحیح buf_سائز, ساخت display_timing *timing,
		    صحیح *panel_بٹ_فی_colourp)
{
	واپس edid_get_timing_validate(buf, buf_سائز, timing,
					panel_بٹ_فی_colourp, تہی, تہی);
}

/**
 * Snip the tailing whitespace/return of a string.
 *
 * @param string	The string to be snipped
 * Return: the snipped string
 */
ساکن حرف *snip(حرف *string)
{
	حرف *s;

	/*
	 * This is always a 13 character buffer
	 * and it's not always terminated.
	 */
	string[12] = '\0';
	s = &string[لڑی_لمبائی(string) - 1];

	جب (s >= string && (خلا_ہے(*s) || *s == '\n' || *s == '\r' ||
			*s == '\0'))
		*(s--) = '\0';

	واپس string;
}

/**
 * Print an EDID monitor descriptor block
 *
 * @param monitor	The EDID monitor descriptor block
 * @have_timing		Modifies to 1 if the desciptor contains timing info
 */
ساکن خالی edid_print_dtd(ساخت edid_monitor_descriptor *monitor,
			   غیرعلامتی صحیح *have_timing)
{
	غیرعلامتی حرف *بائٹیں = (غیرعلامتی حرف *)monitor;
	ساخت edid_detailed_timing *timing =
			(ساخت edid_detailed_timing *)monitor;

	اگر (بائٹیں[0] == 0 && بائٹیں[1] == 0) {
		اگر (monitor->type == EDID_MONITOR_DESCRIPTOR_SERIAL)
			لکھو("Monitor serial number: %s\n",
			       snip(monitor->ڈیٹا.string));
		ورنہ اگر (monitor->type == EDID_MONITOR_DESCRIPTOR_ASCII)
			لکھو("Monitor ID: %s\n",
			       snip(monitor->ڈیٹا.string));
		ورنہ اگر (monitor->type == EDID_MONITOR_DESCRIPTOR_NAME)
			لکھو("Monitor name: %s\n",
			       snip(monitor->ڈیٹا.string));
		ورنہ اگر (monitor->type == EDID_MONITOR_DESCRIPTOR_RANGE)
			لکھو("Monitor range limits, horizontal sync: "
			       "%d-%d kHz, vertical refresh: "
			       "%d-%d Hz, max pixel clock: "
			       "%d MHz\n",
			       monitor->ڈیٹا.range_ڈیٹا.horizontal_min,
			       monitor->ڈیٹا.range_ڈیٹا.horizontal_max,
			       monitor->ڈیٹا.range_ڈیٹا.vertical_min,
			       monitor->ڈیٹا.range_ڈیٹا.vertical_max,
			       monitor->ڈیٹا.range_ڈیٹا.pixel_clock_max * 10);
	} ورنہ {
		uint32_t pixclock, h_فعال, h_blanking, v_فعال, v_blanking;
		uint32_t h_کل, v_کل, vfreq;

		pixclock = EDID_DETAILED_TIMING_PIXEL_CLOCK(*timing);
		h_فعال = EDID_DETAILED_TIMING_HORIZONTAL_فعال(*timing);
		h_blanking = EDID_DETAILED_TIMING_HORIZONTAL_BLANKING(*timing);
		v_فعال = EDID_DETAILED_TIMING_VERTICAL_فعال(*timing);
		v_blanking = EDID_DETAILED_TIMING_VERTICAL_BLANKING(*timing);

		h_کل = h_فعال + h_blanking;
		v_کل = v_فعال + v_blanking;
		اگر (v_کل > 0 && h_کل > 0)
			vfreq = pixclock / (v_کل * h_کل);
		ورنہ
			vfreq = 1; /* Error case */
		لکھو("\t%dx%d\%c\t%d Hz (detailed)\n", h_فعال,
		       v_فعال, h_فعال > 1000 ? ' ' : '\t', vfreq);
		*have_timing = 1;
	}
}

/**
 * Get the manufacturer name from an EDID info.
 *
 * @param edid_info     The EDID info to be printed
 * @param name		Returns the string of the manufacturer name
 */
ساکن خالی edid_get_manufacturer_name(ساخت edid1_info *edid, حرف *name)
{
	name[0] = EDID1_INFO_MANUFACTURER_NAME_CHAR1(*edid) + 'A' - 1;
	name[1] = EDID1_INFO_MANUFACTURER_NAME_CHAR2(*edid) + 'A' - 1;
	name[2] = EDID1_INFO_MANUFACTURER_NAME_CHAR3(*edid) + 'A' - 1;
	name[3] = '\0';
}

خالی edid_print_info(ساخت edid1_info *edid_info)
{
	صحیح i;
	حرف manufacturer[4];
	غیرعلامتی صحیح have_timing = 0;
	uint32_t serial_نمبر;

	اگر (edid_check_info(edid_info)) {
		لکھو("Not a valid EDID\n");
		واپس;
	}

	لکھو("EDID version: %d.%d\n",
	       edid_info->version, edid_info->revision);

	لکھو("Product ID code: %04x\n", EDID1_INFO_حاصل_ضرب_رمز(*edid_info));

	edid_get_manufacturer_name(edid_info, manufacturer);
	لکھو("Manufacturer: %s\n", manufacturer);

	serial_نمبر = EDID1_INFO_SERIAL_نمبر(*edid_info);
	اگر (serial_نمبر != 0xffffffff) {
		اگر (strcmp(manufacturer, "MAG") == 0)
			serial_نمبر -= 0x7000000;
		اگر (strcmp(manufacturer, "OQI") == 0)
			serial_نمبر -= 456150000;
		اگر (strcmp(manufacturer, "VSC") == 0)
			serial_نمبر -= 640000000;
	}
	لکھو("Serial number: %08x\n", serial_نمبر);
	لکھو("Manufactured in week: %d year: %d\n",
	       edid_info->week, edid_info->year + 1990);

	لکھو("Video input definition: %svoltage level %d%s%s%s%s%s\n",
	       EDID1_INFO_VIDEO_داخلہ_DIGITAL(*edid_info) ?
	       "digital signal, " : "analog signal, ",
	       EDID1_INFO_VIDEO_داخلہ_VOLTAGE_LEVEL(*edid_info),
	       EDID1_INFO_VIDEO_داخلہ_BLANK_مقصد_BLACK(*edid_info) ?
	       ", blank to black" : "",
	       EDID1_INFO_VIDEO_داخلہ_SEPARATE_SYNC(*edid_info) ?
	       ", separate sync" : "",
	       EDID1_INFO_VIDEO_داخلہ_مرکب_SYNC(*edid_info) ?
	       ", composite sync" : "",
	       EDID1_INFO_VIDEO_داخلہ_SYNC_ON_GREEN(*edid_info) ?
	       ", sync on green" : "",
	       EDID1_INFO_VIDEO_داخلہ_SERRATION_V(*edid_info) ?
	       ", serration v" : "");

	لکھو("Monitor is %s\n",
	       EDID1_INFO_FEATURE_RGB(*edid_info) ? "RGB" : "non-RGB");

	لکھو("Maximum visible display size: %d cm x %d cm\n",
	       edid_info->max_سائز_horizontal,
	       edid_info->max_سائز_vertical);

	لکھو("Power management features: %s%s, %s%s, %s%s\n",
	       EDID1_INFO_FEATURE_فعال_OFF(*edid_info) ?
	       "" : "no ", "active off",
	       EDID1_INFO_FEATURE_SUSPEND(*edid_info) ? "" : "no ", "suspend",
	       EDID1_INFO_FEATURE_STANDBY(*edid_info) ? "" : "no ", "standby");

	لکھو("Estabilished timings:\n");
	اگر (EDID1_INFO_ESTABLISHED_TIMING_720X400_70(*edid_info))
		لکھو("\t720x400\t\t70 Hz (VGA 640x400, IBM)\n");
	اگر (EDID1_INFO_ESTABLISHED_TIMING_720X400_88(*edid_info))
		لکھو("\t720x400\t\t88 Hz (XGA2)\n");
	اگر (EDID1_INFO_ESTABLISHED_TIMING_640X480_60(*edid_info))
		لکھو("\t640x480\t\t60 Hz (VGA)\n");
	اگر (EDID1_INFO_ESTABLISHED_TIMING_640X480_67(*edid_info))
		لکھو("\t640x480\t\t67 Hz (Mac II, Apple)\n");
	اگر (EDID1_INFO_ESTABLISHED_TIMING_640X480_72(*edid_info))
		لکھو("\t640x480\t\t72 Hz (VESA)\n");
	اگر (EDID1_INFO_ESTABLISHED_TIMING_640X480_75(*edid_info))
		لکھو("\t640x480\t\t75 Hz (VESA)\n");
	اگر (EDID1_INFO_ESTABLISHED_TIMING_800X600_56(*edid_info))
		لکھو("\t800x600\t\t56 Hz (VESA)\n");
	اگر (EDID1_INFO_ESTABLISHED_TIMING_800X600_60(*edid_info))
		لکھو("\t800x600\t\t60 Hz (VESA)\n");
	اگر (EDID1_INFO_ESTABLISHED_TIMING_800X600_72(*edid_info))
		لکھو("\t800x600\t\t72 Hz (VESA)\n");
	اگر (EDID1_INFO_ESTABLISHED_TIMING_800X600_75(*edid_info))
		لکھو("\t800x600\t\t75 Hz (VESA)\n");
	اگر (EDID1_INFO_ESTABLISHED_TIMING_832X624_75(*edid_info))
		لکھو("\t832x624\t\t75 Hz (Mac II)\n");
	اگر (EDID1_INFO_ESTABLISHED_TIMING_1024X768_87I(*edid_info))
		لکھو("\t1024x768\t87 Hz Interlaced (8514A)\n");
	اگر (EDID1_INFO_ESTABLISHED_TIMING_1024X768_60(*edid_info))
		لکھو("\t1024x768\t60 Hz (VESA)\n");
	اگر (EDID1_INFO_ESTABLISHED_TIMING_1024X768_70(*edid_info))
		لکھو("\t1024x768\t70 Hz (VESA)\n");
	اگر (EDID1_INFO_ESTABLISHED_TIMING_1024X768_75(*edid_info))
		لکھو("\t1024x768\t75 Hz (VESA)\n");
	اگر (EDID1_INFO_ESTABLISHED_TIMING_1280X1024_75(*edid_info))
		لکھو("\t1280x1024\t75 (VESA)\n");
	اگر (EDID1_INFO_ESTABLISHED_TIMING_1152X870_75(*edid_info))
		لکھو("\t1152x870\t75 (Mac II)\n");

	/* Standard timings. */
	لکھو("Standard timings:\n");
	برائے (i = 0; i < ARRAY_سائز(edid_info->standard_timings); i++) {
		غیرعلامتی صحیح x, y, freq;

		اگر (edid_get_standard_timing(edid_info, i, &x, &y, &freq)) {
			لکھو("\t%dx%d%c\t%d Hz\n", x, y,
			       x > 1000 ? ' ' : '\t', freq);
			have_timing = 1;
		}
	}

	/* Detailed timing information. */
	برائے (i = 0; i < ARRAY_سائز(edid_info->monitor_details.descriptor);
			i++) {
		edid_print_dtd(&edid_info->monitor_details.descriptor[i],
			       &have_timing);
	}

	اگر (!have_timing)
		لکھو("\tNone\n");
}
