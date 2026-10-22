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

#అన్తర్భావ <edid.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <fdtdec.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <linux/ctype.h>
#అన్తర్భావ <linux/string.h>

#ఒకవేళ CONFIG_IS_ENABLED(I2C_EDID_STANDARD)
#నిర్వచించు TIMING(c, ha, hfp, hbp, hsl, va, vfp, vbp, vsl, f)	\
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

స్థిర స్థిరరాశి నిర్మాణం display_timing dmt_timings[] = {
	{ TIMING(31500000, 640, 32, 64, 96, 350, 32, 3, 60,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(31500000, 640, 32, 64, 96, 400, 1, 3, 41,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(35500000, 720, 36, 72, 108, 400, 1, 3, 42,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(25175000, 640, 16, 96, 48, 480, 10, 2, 33,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(31500000, 640, 24, 40, 128, 480, 9, 3, 28,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(31500000, 640, 16, 64, 120, 480, 1, 3, 16,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(36000000, 640, 56, 56, 80, 480, 1, 3, 25,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(36000000, 800, 24, 72, 128, 600, 1, 2, 22,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(40000000, 800, 40, 128, 88, 600, 1, 4, 23,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(50000000, 800, 56, 120, 64, 600, 37, 6, 23,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(49500000, 800, 16, 80, 160, 600, 1, 3, 21,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(56250000, 800, 32, 64, 152, 600, 1, 3, 27,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(73250000, 800, 48, 32, 80, 600, 3, 4, 29,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(33750000, 848, 16, 112, 112, 480, 6, 8, 23,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(44900000, 1024, 8, 176, 56, 768, 0, 8, 41,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(65000000, 1024, 24, 136, 160, 768, 3, 6, 29,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(75000000, 1024, 24, 136, 144, 768, 3, 6, 29,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(78750000, 1024, 16, 96, 176, 768, 1, 3, 28,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(94500000, 1024, 48, 96, 208, 768, 1, 3, 36,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(115500000, 1024, 48, 32, 80, 768, 3, 4, 38,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(108000000, 1152, 64, 128, 256, 864, 1, 3, 32,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(74250000, 1280, 110, 40, 220, 720, 5, 5, 20,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(68250000, 1280, 48, 32, 80, 768, 3, 7, 12,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(79500000, 1280, 64, 128, 192, 768, 3, 7, 20,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(102250000, 1280, 80, 128, 208, 768, 3, 7, 27,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(117500000, 1280, 80, 136, 216, 768, 3, 7, 31,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(140250000, 1280, 48, 32, 80, 768, 3, 7, 35,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(71000000, 1280, 48, 32, 80, 800, 3, 6, 14,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(83500000, 1280, 72, 128, 200, 800, 3, 6, 22,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(106500000, 1280, 80, 128, 208, 800, 3, 6, 29,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(122500000, 1280, 80, 136, 216, 800, 3, 6, 34,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(146250000, 1280, 48, 32, 80, 800, 3, 6, 38,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(108000000, 1280, 96, 112, 312, 960, 1, 3, 36,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(148500000, 1280, 64, 160, 224, 960, 1, 3, 47,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(175500000, 1280, 48, 32, 80, 960, 3, 4, 50,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(108000000, 1280, 48, 112, 248, 1024, 1, 3, 38,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(135000000, 1280, 16, 144, 248, 1024, 1, 3, 38,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(157500000, 1280, 64, 160, 224, 1024, 1, 3, 44,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(187250000, 1280, 48, 32, 80, 1024, 3, 7, 50,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(85500000, 1360, 64, 112, 256, 768, 3, 6, 18,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(148250000, 1360, 48, 32, 80, 768, 3, 5, 37,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(85500000, 1366, 70, 143, 213, 768, 3, 3, 24,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(72000000, 1366, 14, 56, 64, 768, 1, 3, 28,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(101000000, 1400, 48, 32, 80, 1050, 3, 4, 23,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(121750000, 1400, 88, 144, 232, 1050, 3, 4, 32,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(156000000, 1400, 104, 144, 248, 1050, 3, 4, 42,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(179500000, 1400, 104, 152, 256, 1050, 3, 4, 48,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(208000000, 1400, 48, 32, 80, 1050, 3, 4, 55,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(88750000, 1440, 48, 32, 80, 900, 3, 6, 17,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(106500000, 1440, 80, 152, 232, 900, 3, 6, 25,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(136750000, 1440, 96, 152, 248, 900, 3, 6, 33,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(157000000, 1440, 104, 152, 256, 900, 3, 6, 39,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(182750000, 1440, 48, 32, 80, 900, 3, 6, 44,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(108000000, 1600, 24, 80, 96, 900, 1, 3, 96,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(162000000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(175500000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(189000000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(202500000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(229500000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(268250000, 1600, 48, 32, 80, 1200, 3, 4, 64,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(119000000, 1680, 48, 32, 80, 1050, 3, 6, 21,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(146250000, 1680, 104, 176, 280, 1050, 3, 6, 30,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(187000000, 1680, 120, 176, 296, 1050, 3, 6, 40,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(214750000, 1680, 128, 176, 304, 1050, 3, 6, 46,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(245500000, 1680, 48, 32, 80, 1050, 3, 6, 53,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(204750000, 1792, 128, 200, 328, 1344, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(261000000, 1792, 96, 216, 352, 1344, 1, 3, 69,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(333250000, 1792, 48, 32, 80, 1344, 3, 4, 72,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(218250000, 1856, 96, 224, 352, 1392, 1, 3, 43,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(288000000, 1856, 128, 224, 352, 1392, 1, 3, 104,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(356500000, 1856, 48, 32, 80, 1392, 3, 4, 75,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(148500000, 1920, 88, 44, 148, 1080, 4, 5, 36,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(154000000, 1920, 48, 32, 80, 1200, 3, 6, 26,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(193250000, 1920, 136, 200, 336, 1200, 3, 6, 36,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(245250000, 1920, 136, 208, 344, 1200, 3, 6, 46,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(281250000, 1920, 144, 208, 352, 1200, 3, 6, 53,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(317000000, 1920, 48, 32, 80, 1200, 3, 6, 62,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(234000000, 1920, 128, 208, 344, 1440, 1, 3, 56,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(297000000, 1920, 144, 224, 352, 1440, 1, 3, 56,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(380500000, 1920, 48, 32, 80, 1440, 3, 4, 78,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(162000000, 2048, 26, 80, 96, 1152, 1, 3, 44,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(268500000, 2560, 48, 32, 80, 1600, 3, 6, 37,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(348500000, 2560, 192, 280, 472, 1600, 3, 6, 49,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(443250000, 2560, 208, 280, 488, 1600, 3, 6, 63,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(505250000, 2560, 208, 280, 488, 1600, 3, 6, 73,
		 DISPLAY_FLAGS_HSYNC_దిగువ | DISPLAY_FLAGS_VSYNC_ఎగువ) },
	{ TIMING(552750000, 2560, 48, 32, 80, 1600, 3, 6, 85,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(556744000, 4096, 8, 32, 40, 2160, 48, 8, 6,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
	{ TIMING(556188000, 4096, 8, 32, 40, 2160, 48, 8, 6,
		 DISPLAY_FLAGS_HSYNC_ఎగువ | DISPLAY_FLAGS_VSYNC_దిగువ) },
};
#అయితే_ముగింపు

పూర్ణాంకం edid_check_info(నిర్మాణం edid1_info *edid_info)
{
	ఒకవేళ ((edid_info == ఖాళీ) || (edid_info->version == 0))
		తిరిగి -1;

	ఒకవేళ (memcmp(edid_info->శీర్షిక, "\x0\xff\xff\xff\xff\xff\xff\x0", 8))
		తిరిగి -1;

	ఒకవేళ (edid_info->version == 0xff && edid_info->revision == 0xff)
		తిరిగి -1;

	తిరిగి 0;
}

పూర్ణాంకం edid_check_checksum(u8 *edid_ఖండం)
{
	u8 checksum = 0;
	పూర్ణాంకం i;

	కొరకు (i = 0; i < 128; i++)
		checksum += edid_ఖండం[i];

	తిరిగి (checksum == 0) ? 0 : -EINVAL;
}

పూర్ణాంకం edid_get_ranges(నిర్మాణం edid1_info *edid, గుర్తులేని పూర్ణాంకం *hmin,
		    గుర్తులేని పూర్ణాంకం *hmax, గుర్తులేని పూర్ణాంకం *vmin,
		    గుర్తులేని పూర్ణాంకం *vmax)
{
	పూర్ణాంకం i;
	నిర్మాణం edid_monitor_descriptor *monitor;

	*hmin = *hmax = *vmin = *vmax = 0;
	ఒకవేళ (edid_check_info(edid))
		తిరిగి -1;

	కొరకు (i = 0; i < ARRAY_కొలత(edid->monitor_details.descriptor); i++) {
		monitor = &edid->monitor_details.descriptor[i];
		ఒకవేళ (monitor->type == EDID_MONITOR_DESCRIPTOR_RANGE) {
			*hmin = monitor->దత్తాంశం.range_దత్తాంశం.horizontal_min;
			*hmax = monitor->దత్తాంశం.range_దత్తాంశం.horizontal_max;
			*vmin = monitor->దత్తాంశం.range_దత్తాంశం.vertical_min;
			*vmax = monitor->దత్తాంశం.range_దత్తాంశం.vertical_max;
			తిరిగి 0;
		}
	}
	తిరిగి -1;
}

/* Set all parts of a timing entry to the same value */
స్థిర శూన్య సమితి_నమోదు(నిర్మాణం timing_నమోదు *నమోదు_2, u32 విలువ)
{
	నమోదు_2->min = విలువ;
	నమోదు_2->typ = విలువ;
	నమోదు_2->max = విలువ;
}

/**
 * decode_timing() - Decoding an 18-byte detailed timing record
 *
 * @buf:	Pointer to EDID detailed timing record
 * @timing:	Place to put timing
 */
స్థిర శూన్య decode_timing(u8 *buf, నిర్మాణం display_timing *timing)
{
	uint x_mm, y_mm;
	గుర్తులేని పూర్ణాంకం ha, hbl, hso, hspw, hborder;
	గుర్తులేని పూర్ణాంకం va, vbl, vso, vspw, vborder;
	నిర్మాణం edid_detailed_timing *t = (నిర్మాణం edid_detailed_timing *)buf;

	/* Edid contains pixel clock in terms of 10KHz */
	సమితి_నమోదు(&timing->pixelclock, (buf[0] + (buf[1] << 8)) * 10000);
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

	సమితి_నమోదు(&timing->hactive, ha);
	సమితి_నమోదు(&timing->hfront_porch, hso);
	సమితి_నమోదు(&timing->hback_porch, hbl - hso - hspw);
	సమితి_నమోదు(&timing->hsync_len, hspw);

	సమితి_నమోదు(&timing->vactive, va);
	సమితి_నమోదు(&timing->vfront_porch, vso);
	సమితి_నమోదు(&timing->vback_porch, vbl - vso - vspw);
	సమితి_నమోదు(&timing->vsync_len, vspw);

	timing->flags = 0;
	ఒకవేళ (EDID_DETAILED_TIMING_FLAG_HSYNC_POLARITY(*t))
		timing->flags |= DISPLAY_FLAGS_HSYNC_ఎగువ;
	లేకపోతే
		timing->flags |= DISPLAY_FLAGS_HSYNC_దిగువ;
	ఒకవేళ (EDID_DETAILED_TIMING_FLAG_VSYNC_POLARITY(*t))
		timing->flags |= DISPLAY_FLAGS_VSYNC_ఎగువ;
	లేకపోతే
		timing->flags |= DISPLAY_FLAGS_VSYNC_దిగువ;

	ఒకవేళ (EDID_DETAILED_TIMING_FLAG_INTERLACED(*t))
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
స్థిర bool cea_is_hdmi_vsdb_present(నిర్మాణం edid_cea861_info *info)
{
	u8 చివర_2, i = 0;

	/* check for end of data block */
	చివర_2 = info->dtd_స్థానభ్రంశం;
	ఒకవేళ (చివర_2 == 0)
		చివర_2 = పరిమాణం(info->దత్తాంశం);
	ఒకవేళ (చివర_2 < 4 || చివర_2 > పరిమాణం(info->దత్తాంశం))
		తిరిగి false;
	చివర_2 -= 4;

	అయితే (i < చివర_2) {
		/* Look for vendor specific data block of appropriate size */
		ఒకవేళ ((EDID_CEA861_DB_TYPE(*info, i) == EDID_CEA861_DB_VENDOR) &&
		    (EDID_CEA861_DB_LEN(*info, i) >= 5)) {
			u8 *db = &info->దత్తాంశం[i + 1];
			u32 oui = db[0] | (db[1] << 8) | (db[2] << 16);

			ఒకవేళ (oui == HDMI_IEEE_OUI)
				తిరిగి true;
		}
		i += EDID_CEA861_DB_LEN(*info, i) + 1;
	}

	తిరిగి false;
}

స్థిర bool edid_కనుగొను_valid_detailed_timing(శూన్య *buf, పూర్ణాంకం లెక్క,
					    నిర్మాణం display_timing *timing,
					    bool (*బహుళకం_valid)(శూన్య *priv,
						స్థిరరాశి నిర్మాణం display_timing *timing),
					    శూన్య *బహుళకం_valid_priv)
{
	నిర్మాణం edid_detailed_timing *t = buf;
	bool దొరికింది = false;
	పూర్ణాంకం i;

	కొరకు (i = 0; i < లెక్క && !దొరికింది; i++, t++)
		ఒకవేళ (EDID_DETAILED_TIMING_PIXEL_CLOCK(*t) != 0) {
			decode_timing((u8 *)t, timing);
			ఒకవేళ (బహుళకం_valid)
				దొరికింది = బహుళకం_valid(బహుళకం_valid_priv,
						   timing);
			లేకపోతే
				దొరికింది = true;
		}

	తిరిగి దొరికింది;
}

స్థిర bool edid_get_standard_timing(నిర్మాణం edid1_info *edid, పూర్ణాంకం i, గుర్తులేని పూర్ణాంకం *x,
				     గుర్తులేని పూర్ణాంకం *y, గుర్తులేని పూర్ణాంకం *freq)
{
	గుర్తులేని పూర్ణాంకం aspect = 10000;
	గుర్తులేని అక్షరం xres, vfreq;

	xres = EDID1_INFO_STANDARD_TIMING_XRESOLUTION(*edid, i);
	vfreq = EDID1_INFO_STANDARD_TIMING_VFREQ(*edid, i);
	ఒకవేళ (xres != vfreq || (xres != 0 && xres != 1) ||
	    (vfreq != 0 && vfreq != 1)) {
		ఎంపిక (EDID1_INFO_STANDARD_TIMING_ASPECT(*edid, i)) {
		స్థితి ASPECT_625: // 16:10
			aspect = 6250;
			ఆపు;
		స్థితి ASPECT_75: // 4:3
			aspect = 7500;
			ఆపు;
		స్థితి ASPECT_8: // 5:4
			aspect = 8000;
			ఆపు;
		స్థితి ASPECT_5625: // 16:9
			aspect = 5625;
			ఆపు;
		}

		*x = (xres + 31) * 8;
		*y = *x * aspect / 10000;
		*freq = (vfreq & 0x3f) + 60;

		తిరిగి true;
	}

	తిరిగి false;
}

#ఒకవేళ CONFIG_IS_ENABLED(I2C_EDID_STANDARD)
స్థిర bool edid_కనుగొను_valid_standard_timing(నిర్మాణం edid1_info *buf,
					    నిర్మాణం display_timing *timing,
					    bool (*బహుళకం_valid)(శూన్య *priv,
						స్థిరరాశి నిర్మాణం display_timing *timing),
					    శూన్య *బహుళకం_valid_priv)
{
	గుర్తులేని పూర్ణాంకం x, y, freq;
	bool దొరికింది = false;
	పూర్ణాంకం i, k;

	కొరకు (i = 0; i < ARRAY_కొలత(buf->standard_timings); i++) {
		ఒకవేళ (!edid_get_standard_timing(buf, i, &x, &y, &freq))
			కొనసాగు;

		కొరకు (k = 0; k < ARRAY_కొలత(dmt_timings); k++) {
			స్థిరరాశి నిర్మాణం display_timing *dt = &dmt_timings[k];

			ఒకవేళ (dt->hactive.typ == x && dt->vactive.typ == y) {
				దొరికింది = బహుళకం_valid(బహుళకం_valid_priv, dt);
				ఒకవేళ (దొరికింది) {
					జ్ఞాపకం_నకలు(timing, dt, పరిమాణం(*timing));
					తిరిగి true;
				}
			}
		}
	}

	తిరిగి దొరికింది;
}
#అయితే_ముగింపు

పూర్ణాంకం edid_get_timing_validate(u8 *buf, పూర్ణాంకం buf_కొలత,
			     నిర్మాణం display_timing *timing,
			     పూర్ణాంకం *panel_బిట్లు_చొప్పున_colourp,
			     bool (*బహుళకం_valid)(శూన్య *priv,
					స్థిరరాశి నిర్మాణం display_timing *timing),
			     శూన్య *బహుళకం_valid_priv)
{
	నిర్మాణం edid1_info *edid = (నిర్మాణం edid1_info *)buf;
	bool దొరికింది;

	ఒకవేళ (buf_కొలత < పరిమాణం(*edid) || edid_check_info(edid)) {
		debug("%s: Invalid buffer\n", __func__);
		తిరిగి -EINVAL;
	}

	ఒకవేళ (!EDID1_INFO_VIDEO_ఆదానం_DIGITAL(*edid)) {
		debug("%s: Not a digital display\n", __func__);
		తిరిగి -ENOSYS;
	}

	ఒకవేళ (!EDID1_INFO_FEATURE_PREFERRED_TIMING_బహుళకం(*edid)) {
		debug("%s: No preferred timing\n", __func__);
		తిరిగి -ENOENT;
	}

	/* Look for detailed timing in base EDID */
	దొరికింది = edid_కనుగొను_valid_detailed_timing(edid->monitor_details.descriptor, 4,
						timing, బహుళకం_valid, బహుళకం_valid_priv);

	/* Look for detailed timing in CTA-861 Extension Block */
	ఒకవేళ (!దొరికింది && edid->extension_flag && buf_కొలత >= EDID_EXT_కొలత) {
		నిర్మాణం edid_cea861_info *info =
			(నిర్మాణం edid_cea861_info *)(buf + పరిమాణం(*edid));

		ఒకవేళ (info->extension_ట్యాగ్ == EDID_CEA861_EXTENSION_ట్యాగ్) {
			పూర్ణాంకం లెక్క = EDID_CEA861_DTD_లెక్క(*info);
			పూర్ణాంకం స్థానభ్రంశం = info->dtd_స్థానభ్రంశం;
			పూర్ణాంకం కొలత = లెక్క * పరిమాణం(నిర్మాణం edid_detailed_timing);

			ఒకవేళ (స్థానభ్రంశం >= 4 && స్థానభ్రంశం + కొలత < EDID_కొలత)
				దొరికింది = edid_కనుగొను_valid_detailed_timing(
					(u8 *)info + స్థానభ్రంశం, లెక్క, timing,
					బహుళకం_valid, బహుళకం_valid_priv);
		}
	}

#ఒకవేళ CONFIG_IS_ENABLED(I2C_EDID_STANDARD)
	/* Look for timing in Standard Timings */
	ఒకవేళ (!దొరికింది)
		దొరికింది = edid_కనుగొను_valid_standard_timing(edid, timing, బహుళకం_valid,
							బహుళకం_valid_priv);
#అయితే_ముగింపు

	ఒకవేళ (!దొరికింది)
		తిరిగి -EINVAL;

	ఒకవేళ (edid->version != 1 || edid->revision < 4) {
		debug("%s: EDID version %d.%d does not have required info\n",
		      __func__, edid->version, edid->revision);
		*panel_బిట్లు_చొప్పున_colourp = -1;
	} లేకపోతే  {
		*panel_బిట్లు_చొప్పున_colourp =
			((edid->video_ఆదానం_definition & 0x70) >> 3) + 4;
	}

	timing->hdmi_monitor = false;
	ఒకవేళ (edid->extension_flag && (buf_కొలత >= EDID_EXT_కొలత)) {
		నిర్మాణం edid_cea861_info *info =
			(నిర్మాణం edid_cea861_info *)(buf + పరిమాణం(*edid));

		ఒకవేళ (info->extension_ట్యాగ్ == EDID_CEA861_EXTENSION_ట్యాగ్)
			timing->hdmi_monitor = cea_is_hdmi_vsdb_present(info);
	}

	తిరిగి 0;
}

పూర్ణాంకం edid_get_timing(u8 *buf, పూర్ణాంకం buf_కొలత, నిర్మాణం display_timing *timing,
		    పూర్ణాంకం *panel_బిట్లు_చొప్పున_colourp)
{
	తిరిగి edid_get_timing_validate(buf, buf_కొలత, timing,
					panel_బిట్లు_చొప్పున_colourp, ఖాళీ, ఖాళీ);
}

/**
 * Snip the tailing whitespace/return of a string.
 *
 * @param string	The string to be snipped
 * Return: the snipped string
 */
స్థిర అక్షరం *snip(అక్షరం *string)
{
	అక్షరం *s;

	/*
	 * This is always a 13 character buffer
	 * and it's not always terminated.
	 */
	string[12] = '\0';
	s = &string[అక్షరశ్రేణి_పొడవు(string) - 1];

	అయితే (s >= string && (ఖాళీనా(*s) || *s == '\n' || *s == '\r' ||
			*s == '\0'))
		*(s--) = '\0';

	తిరిగి string;
}

/**
 * Print an EDID monitor descriptor block
 *
 * @param monitor	The EDID monitor descriptor block
 * @have_timing		Modifies to 1 if the desciptor contains timing info
 */
స్థిర శూన్య edid_print_dtd(నిర్మాణం edid_monitor_descriptor *monitor,
			   గుర్తులేని పూర్ణాంకం *have_timing)
{
	గుర్తులేని అక్షరం *బైట్లు = (గుర్తులేని అక్షరం *)monitor;
	నిర్మాణం edid_detailed_timing *timing =
			(నిర్మాణం edid_detailed_timing *)monitor;

	ఒకవేళ (బైట్లు[0] == 0 && బైట్లు[1] == 0) {
		ఒకవేళ (monitor->type == EDID_MONITOR_DESCRIPTOR_SERIAL)
			రాయి("Monitor serial number: %s\n",
			       snip(monitor->దత్తాంశం.string));
		లేకపోతే ఒకవేళ (monitor->type == EDID_MONITOR_DESCRIPTOR_ASCII)
			రాయి("Monitor ID: %s\n",
			       snip(monitor->దత్తాంశం.string));
		లేకపోతే ఒకవేళ (monitor->type == EDID_MONITOR_DESCRIPTOR_NAME)
			రాయి("Monitor name: %s\n",
			       snip(monitor->దత్తాంశం.string));
		లేకపోతే ఒకవేళ (monitor->type == EDID_MONITOR_DESCRIPTOR_RANGE)
			రాయి("Monitor range limits, horizontal sync: "
			       "%d-%d kHz, vertical refresh: "
			       "%d-%d Hz, max pixel clock: "
			       "%d MHz\n",
			       monitor->దత్తాంశం.range_దత్తాంశం.horizontal_min,
			       monitor->దత్తాంశం.range_దత్తాంశం.horizontal_max,
			       monitor->దత్తాంశం.range_దత్తాంశం.vertical_min,
			       monitor->దత్తాంశం.range_దత్తాంశం.vertical_max,
			       monitor->దత్తాంశం.range_దత్తాంశం.pixel_clock_max * 10);
	} లేకపోతే {
		uint32_t pixclock, h_చురుకైన, h_blanking, v_చురుకైన, v_blanking;
		uint32_t h_మొత్తం, v_మొత్తం, vfreq;

		pixclock = EDID_DETAILED_TIMING_PIXEL_CLOCK(*timing);
		h_చురుకైన = EDID_DETAILED_TIMING_HORIZONTAL_చురుకైన(*timing);
		h_blanking = EDID_DETAILED_TIMING_HORIZONTAL_BLANKING(*timing);
		v_చురుకైన = EDID_DETAILED_TIMING_VERTICAL_చురుకైన(*timing);
		v_blanking = EDID_DETAILED_TIMING_VERTICAL_BLANKING(*timing);

		h_మొత్తం = h_చురుకైన + h_blanking;
		v_మొత్తం = v_చురుకైన + v_blanking;
		ఒకవేళ (v_మొత్తం > 0 && h_మొత్తం > 0)
			vfreq = pixclock / (v_మొత్తం * h_మొత్తం);
		లేకపోతే
			vfreq = 1; /* Error case */
		రాయి("\t%dx%d\%c\t%d Hz (detailed)\n", h_చురుకైన,
		       v_చురుకైన, h_చురుకైన > 1000 ? ' ' : '\t', vfreq);
		*have_timing = 1;
	}
}

/**
 * Get the manufacturer name from an EDID info.
 *
 * @param edid_info     The EDID info to be printed
 * @param name		Returns the string of the manufacturer name
 */
స్థిర శూన్య edid_get_manufacturer_name(నిర్మాణం edid1_info *edid, అక్షరం *name)
{
	name[0] = EDID1_INFO_MANUFACTURER_NAME_CHAR1(*edid) + 'A' - 1;
	name[1] = EDID1_INFO_MANUFACTURER_NAME_CHAR2(*edid) + 'A' - 1;
	name[2] = EDID1_INFO_MANUFACTURER_NAME_CHAR3(*edid) + 'A' - 1;
	name[3] = '\0';
}

శూన్య edid_print_info(నిర్మాణం edid1_info *edid_info)
{
	పూర్ణాంకం i;
	అక్షరం manufacturer[4];
	గుర్తులేని పూర్ణాంకం have_timing = 0;
	uint32_t serial_సంఖ్య;

	ఒకవేళ (edid_check_info(edid_info)) {
		రాయి("Not a valid EDID\n");
		తిరిగి;
	}

	రాయి("EDID version: %d.%d\n",
	       edid_info->version, edid_info->revision);

	రాయి("Product ID code: %04x\n", EDID1_INFO_లబ్ధం_సంకేతం(*edid_info));

	edid_get_manufacturer_name(edid_info, manufacturer);
	రాయి("Manufacturer: %s\n", manufacturer);

	serial_సంఖ్య = EDID1_INFO_SERIAL_సంఖ్య(*edid_info);
	ఒకవేళ (serial_సంఖ్య != 0xffffffff) {
		ఒకవేళ (strcmp(manufacturer, "MAG") == 0)
			serial_సంఖ్య -= 0x7000000;
		ఒకవేళ (strcmp(manufacturer, "OQI") == 0)
			serial_సంఖ్య -= 456150000;
		ఒకవేళ (strcmp(manufacturer, "VSC") == 0)
			serial_సంఖ్య -= 640000000;
	}
	రాయి("Serial number: %08x\n", serial_సంఖ్య);
	రాయి("Manufactured in week: %d year: %d\n",
	       edid_info->week, edid_info->year + 1990);

	రాయి("Video input definition: %svoltage level %d%s%s%s%s%s\n",
	       EDID1_INFO_VIDEO_ఆదానం_DIGITAL(*edid_info) ?
	       "digital signal, " : "analog signal, ",
	       EDID1_INFO_VIDEO_ఆదానం_VOLTAGE_LEVEL(*edid_info),
	       EDID1_INFO_VIDEO_ఆదానం_BLANK_గమ్యం_BLACK(*edid_info) ?
	       ", blank to black" : "",
	       EDID1_INFO_VIDEO_ఆదానం_SEPARATE_SYNC(*edid_info) ?
	       ", separate sync" : "",
	       EDID1_INFO_VIDEO_ఆదానం_సంయుక్త_SYNC(*edid_info) ?
	       ", composite sync" : "",
	       EDID1_INFO_VIDEO_ఆదానం_SYNC_ON_GREEN(*edid_info) ?
	       ", sync on green" : "",
	       EDID1_INFO_VIDEO_ఆదానం_SERRATION_V(*edid_info) ?
	       ", serration v" : "");

	రాయి("Monitor is %s\n",
	       EDID1_INFO_FEATURE_RGB(*edid_info) ? "RGB" : "non-RGB");

	రాయి("Maximum visible display size: %d cm x %d cm\n",
	       edid_info->max_కొలత_horizontal,
	       edid_info->max_కొలత_vertical);

	రాయి("Power management features: %s%s, %s%s, %s%s\n",
	       EDID1_INFO_FEATURE_చురుకైన_OFF(*edid_info) ?
	       "" : "no ", "active off",
	       EDID1_INFO_FEATURE_SUSPEND(*edid_info) ? "" : "no ", "suspend",
	       EDID1_INFO_FEATURE_STANDBY(*edid_info) ? "" : "no ", "standby");

	రాయి("Estabilished timings:\n");
	ఒకవేళ (EDID1_INFO_ESTABLISHED_TIMING_720X400_70(*edid_info))
		రాయి("\t720x400\t\t70 Hz (VGA 640x400, IBM)\n");
	ఒకవేళ (EDID1_INFO_ESTABLISHED_TIMING_720X400_88(*edid_info))
		రాయి("\t720x400\t\t88 Hz (XGA2)\n");
	ఒకవేళ (EDID1_INFO_ESTABLISHED_TIMING_640X480_60(*edid_info))
		రాయి("\t640x480\t\t60 Hz (VGA)\n");
	ఒకవేళ (EDID1_INFO_ESTABLISHED_TIMING_640X480_67(*edid_info))
		రాయి("\t640x480\t\t67 Hz (Mac II, Apple)\n");
	ఒకవేళ (EDID1_INFO_ESTABLISHED_TIMING_640X480_72(*edid_info))
		రాయి("\t640x480\t\t72 Hz (VESA)\n");
	ఒకవేళ (EDID1_INFO_ESTABLISHED_TIMING_640X480_75(*edid_info))
		రాయి("\t640x480\t\t75 Hz (VESA)\n");
	ఒకవేళ (EDID1_INFO_ESTABLISHED_TIMING_800X600_56(*edid_info))
		రాయి("\t800x600\t\t56 Hz (VESA)\n");
	ఒకవేళ (EDID1_INFO_ESTABLISHED_TIMING_800X600_60(*edid_info))
		రాయి("\t800x600\t\t60 Hz (VESA)\n");
	ఒకవేళ (EDID1_INFO_ESTABLISHED_TIMING_800X600_72(*edid_info))
		రాయి("\t800x600\t\t72 Hz (VESA)\n");
	ఒకవేళ (EDID1_INFO_ESTABLISHED_TIMING_800X600_75(*edid_info))
		రాయి("\t800x600\t\t75 Hz (VESA)\n");
	ఒకవేళ (EDID1_INFO_ESTABLISHED_TIMING_832X624_75(*edid_info))
		రాయి("\t832x624\t\t75 Hz (Mac II)\n");
	ఒకవేళ (EDID1_INFO_ESTABLISHED_TIMING_1024X768_87I(*edid_info))
		రాయి("\t1024x768\t87 Hz Interlaced (8514A)\n");
	ఒకవేళ (EDID1_INFO_ESTABLISHED_TIMING_1024X768_60(*edid_info))
		రాయి("\t1024x768\t60 Hz (VESA)\n");
	ఒకవేళ (EDID1_INFO_ESTABLISHED_TIMING_1024X768_70(*edid_info))
		రాయి("\t1024x768\t70 Hz (VESA)\n");
	ఒకవేళ (EDID1_INFO_ESTABLISHED_TIMING_1024X768_75(*edid_info))
		రాయి("\t1024x768\t75 Hz (VESA)\n");
	ఒకవేళ (EDID1_INFO_ESTABLISHED_TIMING_1280X1024_75(*edid_info))
		రాయి("\t1280x1024\t75 (VESA)\n");
	ఒకవేళ (EDID1_INFO_ESTABLISHED_TIMING_1152X870_75(*edid_info))
		రాయి("\t1152x870\t75 (Mac II)\n");

	/* Standard timings. */
	రాయి("Standard timings:\n");
	కొరకు (i = 0; i < ARRAY_కొలత(edid_info->standard_timings); i++) {
		గుర్తులేని పూర్ణాంకం x, y, freq;

		ఒకవేళ (edid_get_standard_timing(edid_info, i, &x, &y, &freq)) {
			రాయి("\t%dx%d%c\t%d Hz\n", x, y,
			       x > 1000 ? ' ' : '\t', freq);
			have_timing = 1;
		}
	}

	/* Detailed timing information. */
	కొరకు (i = 0; i < ARRAY_కొలత(edid_info->monitor_details.descriptor);
			i++) {
		edid_print_dtd(&edid_info->monitor_details.descriptor[i],
			       &have_timing);
	}

	ఒకవేళ (!have_timing)
		రాయి("\tNone\n");
}
