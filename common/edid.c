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

#ಅನ್ತರ್ಭಾವ <edid.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fdtdec.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <linux/ctype.h>
#ಅನ್ತರ್ಭಾವ <linux/string.h>

#ಆದರೆ CONFIG_IS_ENABLED(I2C_EDID_STANDARD)
#ವ್ಯಾಖ್ಯಾನಿಸು TIMING(c, ha, hfp, hbp, hsl, va, vfp, vbp, vsl, f)	\
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

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ display_timing dmt_timings[] = {
	{ TIMING(31500000, 640, 32, 64, 96, 350, 32, 3, 60,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(31500000, 640, 32, 64, 96, 400, 1, 3, 41,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(35500000, 720, 36, 72, 108, 400, 1, 3, 42,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(25175000, 640, 16, 96, 48, 480, 10, 2, 33,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(31500000, 640, 24, 40, 128, 480, 9, 3, 28,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(31500000, 640, 16, 64, 120, 480, 1, 3, 16,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(36000000, 640, 56, 56, 80, 480, 1, 3, 25,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(36000000, 800, 24, 72, 128, 600, 1, 2, 22,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(40000000, 800, 40, 128, 88, 600, 1, 4, 23,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(50000000, 800, 56, 120, 64, 600, 37, 6, 23,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(49500000, 800, 16, 80, 160, 600, 1, 3, 21,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(56250000, 800, 32, 64, 152, 600, 1, 3, 27,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(73250000, 800, 48, 32, 80, 600, 3, 4, 29,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(33750000, 848, 16, 112, 112, 480, 6, 8, 23,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(44900000, 1024, 8, 176, 56, 768, 0, 8, 41,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(65000000, 1024, 24, 136, 160, 768, 3, 6, 29,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(75000000, 1024, 24, 136, 144, 768, 3, 6, 29,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(78750000, 1024, 16, 96, 176, 768, 1, 3, 28,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(94500000, 1024, 48, 96, 208, 768, 1, 3, 36,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(115500000, 1024, 48, 32, 80, 768, 3, 4, 38,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(108000000, 1152, 64, 128, 256, 864, 1, 3, 32,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(74250000, 1280, 110, 40, 220, 720, 5, 5, 20,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(68250000, 1280, 48, 32, 80, 768, 3, 7, 12,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(79500000, 1280, 64, 128, 192, 768, 3, 7, 20,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(102250000, 1280, 80, 128, 208, 768, 3, 7, 27,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(117500000, 1280, 80, 136, 216, 768, 3, 7, 31,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(140250000, 1280, 48, 32, 80, 768, 3, 7, 35,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(71000000, 1280, 48, 32, 80, 800, 3, 6, 14,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(83500000, 1280, 72, 128, 200, 800, 3, 6, 22,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(106500000, 1280, 80, 128, 208, 800, 3, 6, 29,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(122500000, 1280, 80, 136, 216, 800, 3, 6, 34,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(146250000, 1280, 48, 32, 80, 800, 3, 6, 38,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(108000000, 1280, 96, 112, 312, 960, 1, 3, 36,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(148500000, 1280, 64, 160, 224, 960, 1, 3, 47,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(175500000, 1280, 48, 32, 80, 960, 3, 4, 50,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(108000000, 1280, 48, 112, 248, 1024, 1, 3, 38,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(135000000, 1280, 16, 144, 248, 1024, 1, 3, 38,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(157500000, 1280, 64, 160, 224, 1024, 1, 3, 44,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(187250000, 1280, 48, 32, 80, 1024, 3, 7, 50,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(85500000, 1360, 64, 112, 256, 768, 3, 6, 18,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(148250000, 1360, 48, 32, 80, 768, 3, 5, 37,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(85500000, 1366, 70, 143, 213, 768, 3, 3, 24,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(72000000, 1366, 14, 56, 64, 768, 1, 3, 28,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(101000000, 1400, 48, 32, 80, 1050, 3, 4, 23,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(121750000, 1400, 88, 144, 232, 1050, 3, 4, 32,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(156000000, 1400, 104, 144, 248, 1050, 3, 4, 42,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(179500000, 1400, 104, 152, 256, 1050, 3, 4, 48,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(208000000, 1400, 48, 32, 80, 1050, 3, 4, 55,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(88750000, 1440, 48, 32, 80, 900, 3, 6, 17,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(106500000, 1440, 80, 152, 232, 900, 3, 6, 25,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(136750000, 1440, 96, 152, 248, 900, 3, 6, 33,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(157000000, 1440, 104, 152, 256, 900, 3, 6, 39,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(182750000, 1440, 48, 32, 80, 900, 3, 6, 44,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(108000000, 1600, 24, 80, 96, 900, 1, 3, 96,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(162000000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(175500000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(189000000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(202500000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(229500000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(268250000, 1600, 48, 32, 80, 1200, 3, 4, 64,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(119000000, 1680, 48, 32, 80, 1050, 3, 6, 21,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(146250000, 1680, 104, 176, 280, 1050, 3, 6, 30,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(187000000, 1680, 120, 176, 296, 1050, 3, 6, 40,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(214750000, 1680, 128, 176, 304, 1050, 3, 6, 46,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(245500000, 1680, 48, 32, 80, 1050, 3, 6, 53,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(204750000, 1792, 128, 200, 328, 1344, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(261000000, 1792, 96, 216, 352, 1344, 1, 3, 69,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(333250000, 1792, 48, 32, 80, 1344, 3, 4, 72,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(218250000, 1856, 96, 224, 352, 1392, 1, 3, 43,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(288000000, 1856, 128, 224, 352, 1392, 1, 3, 104,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(356500000, 1856, 48, 32, 80, 1392, 3, 4, 75,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(148500000, 1920, 88, 44, 148, 1080, 4, 5, 36,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(154000000, 1920, 48, 32, 80, 1200, 3, 6, 26,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(193250000, 1920, 136, 200, 336, 1200, 3, 6, 36,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(245250000, 1920, 136, 208, 344, 1200, 3, 6, 46,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(281250000, 1920, 144, 208, 352, 1200, 3, 6, 53,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(317000000, 1920, 48, 32, 80, 1200, 3, 6, 62,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(234000000, 1920, 128, 208, 344, 1440, 1, 3, 56,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(297000000, 1920, 144, 224, 352, 1440, 1, 3, 56,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(380500000, 1920, 48, 32, 80, 1440, 3, 4, 78,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(162000000, 2048, 26, 80, 96, 1152, 1, 3, 44,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(268500000, 2560, 48, 32, 80, 1600, 3, 6, 37,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(348500000, 2560, 192, 280, 472, 1600, 3, 6, 49,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(443250000, 2560, 208, 280, 488, 1600, 3, 6, 63,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(505250000, 2560, 208, 280, 488, 1600, 3, 6, 73,
		 DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ | DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ) },
	{ TIMING(552750000, 2560, 48, 32, 80, 1600, 3, 6, 85,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(556744000, 4096, 8, 32, 40, 2160, 48, 8, 6,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
	{ TIMING(556188000, 4096, 8, 32, 40, 2160, 48, 8, 6,
		 DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ | DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ) },
};
#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ edid_check_info(ರಚನೆ edid1_info *edid_info)
{
	ಆದರೆ ((edid_info == ಖಾಲಿ) || (edid_info->version == 0))
		ಹಿಂತಿರುಗಿ -1;

	ಆದರೆ (memcmp(edid_info->ಶೀರ್ಷಿಕೆ, "\x0\xff\xff\xff\xff\xff\xff\x0", 8))
		ಹಿಂತಿರುಗಿ -1;

	ಆದರೆ (edid_info->version == 0xff && edid_info->revision == 0xff)
		ಹಿಂತಿರುಗಿ -1;

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ edid_check_checksum(u8 *edid_ಖಂಡ)
{
	u8 checksum = 0;
	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = 0; i < 128; i++)
		checksum += edid_ಖಂಡ[i];

	ಹಿಂತಿರುಗಿ (checksum == 0) ? 0 : -EINVAL;
}

ಪೂರ್ಣಾಂಕ edid_get_ranges(ರಚನೆ edid1_info *edid, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *hmin,
		    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *hmax, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *vmin,
		    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *vmax)
{
	ಪೂರ್ಣಾಂಕ i;
	ರಚನೆ edid_monitor_descriptor *monitor;

	*hmin = *hmax = *vmin = *vmax = 0;
	ಆದರೆ (edid_check_info(edid))
		ಹಿಂತಿರುಗಿ -1;

	ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(edid->monitor_details.descriptor); i++) {
		monitor = &edid->monitor_details.descriptor[i];
		ಆದರೆ (monitor->type == EDID_MONITOR_DESCRIPTOR_RANGE) {
			*hmin = monitor->ದತ್ತಾಂಶ.range_ದತ್ತಾಂಶ.horizontal_min;
			*hmax = monitor->ದತ್ತಾಂಶ.range_ದತ್ತಾಂಶ.horizontal_max;
			*vmin = monitor->ದತ್ತಾಂಶ.range_ದತ್ತಾಂಶ.vertical_min;
			*vmax = monitor->ದತ್ತಾಂಶ.range_ದತ್ತಾಂಶ.vertical_max;
			ಹಿಂತಿರುಗಿ 0;
		}
	}
	ಹಿಂತಿರುಗಿ -1;
}

/* Set all parts of a timing entry to the same value */
ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_ನಮೂದು(ರಚನೆ timing_ನಮೂದು *ನಮೂದು, u32 ಮೌಲ್ಯ)
{
	ನಮೂದು->min = ಮೌಲ್ಯ;
	ನಮೂದು->typ = ಮೌಲ್ಯ;
	ನಮೂದು->max = ಮೌಲ್ಯ;
}

/**
 * decode_timing() - Decoding an 18-byte detailed timing record
 *
 * @buf:	Pointer to EDID detailed timing record
 * @timing:	Place to put timing
 */
ಸ್ಥಿರ ಶೂನ್ಯ decode_timing(u8 *buf, ರಚನೆ display_timing *timing)
{
	uint x_mm, y_mm;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ha, hbl, hso, hspw, hborder;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ va, vbl, vso, vspw, vborder;
	ರಚನೆ edid_detailed_timing *t = (ರಚನೆ edid_detailed_timing *)buf;

	/* Edid contains pixel clock in terms of 10KHz */
	ಗಣ_ನಮೂದು(&timing->pixelclock, (buf[0] + (buf[1] << 8)) * 10000);
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

	ಗಣ_ನಮೂದು(&timing->hactive, ha);
	ಗಣ_ನಮೂದು(&timing->hfront_porch, hso);
	ಗಣ_ನಮೂದು(&timing->hback_porch, hbl - hso - hspw);
	ಗಣ_ನಮೂದು(&timing->hsync_len, hspw);

	ಗಣ_ನಮೂದು(&timing->vactive, va);
	ಗಣ_ನಮೂದು(&timing->vfront_porch, vso);
	ಗಣ_ನಮೂದು(&timing->vback_porch, vbl - vso - vspw);
	ಗಣ_ನಮೂದು(&timing->vsync_len, vspw);

	timing->flags = 0;
	ಆದರೆ (EDID_DETAILED_TIMING_FLAG_HSYNC_POLARITY(*t))
		timing->flags |= DISPLAY_FLAGS_HSYNC_ಮೇಲಿನ;
	ಇಲ್ಲದಿದ್ದರೆ
		timing->flags |= DISPLAY_FLAGS_HSYNC_ಕೆಳಗಿನ;
	ಆದರೆ (EDID_DETAILED_TIMING_FLAG_VSYNC_POLARITY(*t))
		timing->flags |= DISPLAY_FLAGS_VSYNC_ಮೇಲಿನ;
	ಇಲ್ಲದಿದ್ದರೆ
		timing->flags |= DISPLAY_FLAGS_VSYNC_ಕೆಳಗಿನ;

	ಆದರೆ (EDID_DETAILED_TIMING_FLAG_INTERLACED(*t))
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
ಸ್ಥಿರ bool cea_is_hdmi_vsdb_present(ರಚನೆ edid_cea861_info *info)
{
	u8 ಕೊನೆ_2, i = 0;

	/* check for end of data block */
	ಕೊನೆ_2 = info->dtd_ಸ್ಥಾನಾಂತರ;
	ಆದರೆ (ಕೊನೆ_2 == 0)
		ಕೊನೆ_2 = ಗಾತ್ರ(info->ದತ್ತಾಂಶ);
	ಆದರೆ (ಕೊನೆ_2 < 4 || ಕೊನೆ_2 > ಗಾತ್ರ(info->ದತ್ತಾಂಶ))
		ಹಿಂತಿರುಗಿ false;
	ಕೊನೆ_2 -= 4;

	ಆಗ (i < ಕೊನೆ_2) {
		/* Look for vendor specific data block of appropriate size */
		ಆದರೆ ((EDID_CEA861_DB_TYPE(*info, i) == EDID_CEA861_DB_VENDOR) &&
		    (EDID_CEA861_DB_LEN(*info, i) >= 5)) {
			u8 *db = &info->ದತ್ತಾಂಶ[i + 1];
			u32 oui = db[0] | (db[1] << 8) | (db[2] << 16);

			ಆದರೆ (oui == HDMI_IEEE_OUI)
				ಹಿಂತಿರುಗಿ true;
		}
		i += EDID_CEA861_DB_LEN(*info, i) + 1;
	}

	ಹಿಂತಿರುಗಿ false;
}

ಸ್ಥಿರ bool edid_ಕಂಡುಹಿಡಿ_valid_detailed_timing(ಶೂನ್ಯ *buf, ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2,
					    ರಚನೆ display_timing *timing,
					    bool (*ಬಹುಲಕ_valid)(ಶೂನ್ಯ *priv,
						ಸ್ಥಿರಾಂಕ ರಚನೆ display_timing *timing),
					    ಶೂನ್ಯ *ಬಹುಲಕ_valid_priv)
{
	ರಚನೆ edid_detailed_timing *t = buf;
	bool ಸಿಕ್ಕಿತು = false;
	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = 0; i < ಎಣಿಕೆ_2 && !ಸಿಕ್ಕಿತು; i++, t++)
		ಆದರೆ (EDID_DETAILED_TIMING_PIXEL_CLOCK(*t) != 0) {
			decode_timing((u8 *)t, timing);
			ಆದರೆ (ಬಹುಲಕ_valid)
				ಸಿಕ್ಕಿತು = ಬಹುಲಕ_valid(ಬಹುಲಕ_valid_priv,
						   timing);
			ಇಲ್ಲದಿದ್ದರೆ
				ಸಿಕ್ಕಿತು = true;
		}

	ಹಿಂತಿರುಗಿ ಸಿಕ್ಕಿತು;
}

ಸ್ಥಿರ bool edid_get_standard_timing(ರಚನೆ edid1_info *edid, ಪೂರ್ಣಾಂಕ i, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *x,
				     ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *y, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *freq)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ aspect = 10000;
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ xres, vfreq;

	xres = EDID1_INFO_STANDARD_TIMING_XRESOLUTION(*edid, i);
	vfreq = EDID1_INFO_STANDARD_TIMING_VFREQ(*edid, i);
	ಆದರೆ (xres != vfreq || (xres != 0 && xres != 1) ||
	    (vfreq != 0 && vfreq != 1)) {
		ಆಯ್ಕೆ (EDID1_INFO_STANDARD_TIMING_ASPECT(*edid, i)) {
		ಸ್ಥಿತಿ ASPECT_625: // 16:10
			aspect = 6250;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ ASPECT_75: // 4:3
			aspect = 7500;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ ASPECT_8: // 5:4
			aspect = 8000;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ ASPECT_5625: // 16:9
			aspect = 5625;
			ನಿಲ್ಲಿಸು;
		}

		*x = (xres + 31) * 8;
		*y = *x * aspect / 10000;
		*freq = (vfreq & 0x3f) + 60;

		ಹಿಂತಿರುಗಿ true;
	}

	ಹಿಂತಿರುಗಿ false;
}

#ಆದರೆ CONFIG_IS_ENABLED(I2C_EDID_STANDARD)
ಸ್ಥಿರ bool edid_ಕಂಡುಹಿಡಿ_valid_standard_timing(ರಚನೆ edid1_info *buf,
					    ರಚನೆ display_timing *timing,
					    bool (*ಬಹುಲಕ_valid)(ಶೂನ್ಯ *priv,
						ಸ್ಥಿರಾಂಕ ರಚನೆ display_timing *timing),
					    ಶೂನ್ಯ *ಬಹುಲಕ_valid_priv)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ x, y, freq;
	bool ಸಿಕ್ಕಿತು = false;
	ಪೂರ್ಣಾಂಕ i, k;

	ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(buf->standard_timings); i++) {
		ಆದರೆ (!edid_get_standard_timing(buf, i, &x, &y, &freq))
			ಮುಂದುವರಿ;

		ಗಾಗಿ (k = 0; k < ARRAY_ಗಾತ್ರ(dmt_timings); k++) {
			ಸ್ಥಿರಾಂಕ ರಚನೆ display_timing *dt = &dmt_timings[k];

			ಆದರೆ (dt->hactive.typ == x && dt->vactive.typ == y) {
				ಸಿಕ್ಕಿತು = ಬಹುಲಕ_valid(ಬಹುಲಕ_valid_priv, dt);
				ಆದರೆ (ಸಿಕ್ಕಿತು) {
					ಸ್ಮರಣೆ_ನಕಲು(timing, dt, ಗಾತ್ರ(*timing));
					ಹಿಂತಿರುಗಿ true;
				}
			}
		}
	}

	ಹಿಂತಿರುಗಿ ಸಿಕ್ಕಿತು;
}
#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ edid_get_timing_validate(u8 *buf, ಪೂರ್ಣಾಂಕ buf_ಗಾತ್ರ,
			     ರಚನೆ display_timing *timing,
			     ಪೂರ್ಣಾಂಕ *panel_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_colourp,
			     bool (*ಬಹುಲಕ_valid)(ಶೂನ್ಯ *priv,
					ಸ್ಥಿರಾಂಕ ರಚನೆ display_timing *timing),
			     ಶೂನ್ಯ *ಬಹುಲಕ_valid_priv)
{
	ರಚನೆ edid1_info *edid = (ರಚನೆ edid1_info *)buf;
	bool ಸಿಕ್ಕಿತು;

	ಆದರೆ (buf_ಗಾತ್ರ < ಗಾತ್ರ(*edid) || edid_check_info(edid)) {
		debug("%s: Invalid buffer\n", __func__);
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	ಆದರೆ (!EDID1_INFO_VIDEO_ಒಳಹರಿವು_DIGITAL(*edid)) {
		debug("%s: Not a digital display\n", __func__);
		ಹಿಂತಿರುಗಿ -ENOSYS;
	}

	ಆದರೆ (!EDID1_INFO_FEATURE_PREFERRED_TIMING_ಬಹುಲಕ(*edid)) {
		debug("%s: No preferred timing\n", __func__);
		ಹಿಂತಿರುಗಿ -ENOENT;
	}

	/* Look for detailed timing in base EDID */
	ಸಿಕ್ಕಿತು = edid_ಕಂಡುಹಿಡಿ_valid_detailed_timing(edid->monitor_details.descriptor, 4,
						timing, ಬಹುಲಕ_valid, ಬಹುಲಕ_valid_priv);

	/* Look for detailed timing in CTA-861 Extension Block */
	ಆದರೆ (!ಸಿಕ್ಕಿತು && edid->extension_flag && buf_ಗಾತ್ರ >= EDID_EXT_ಗಾತ್ರ) {
		ರಚನೆ edid_cea861_info *info =
			(ರಚನೆ edid_cea861_info *)(buf + ಗಾತ್ರ(*edid));

		ಆದರೆ (info->extension_ಟ್ಯಾಗ್ == EDID_CEA861_EXTENSION_ಟ್ಯಾಗ್) {
			ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = EDID_CEA861_DTD_ಎಣಿಕೆ(*info);
			ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ = info->dtd_ಸ್ಥಾನಾಂತರ;
			ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = ಎಣಿಕೆ_2 * ಗಾತ್ರ(ರಚನೆ edid_detailed_timing);

			ಆದರೆ (ಸ್ಥಾನಾಂತರ >= 4 && ಸ್ಥಾನಾಂತರ + ಗಾತ್ರ_2 < EDID_ಗಾತ್ರ)
				ಸಿಕ್ಕಿತು = edid_ಕಂಡುಹಿಡಿ_valid_detailed_timing(
					(u8 *)info + ಸ್ಥಾನಾಂತರ, ಎಣಿಕೆ_2, timing,
					ಬಹುಲಕ_valid, ಬಹುಲಕ_valid_priv);
		}
	}

#ಆದರೆ CONFIG_IS_ENABLED(I2C_EDID_STANDARD)
	/* Look for timing in Standard Timings */
	ಆದರೆ (!ಸಿಕ್ಕಿತು)
		ಸಿಕ್ಕಿತು = edid_ಕಂಡುಹಿಡಿ_valid_standard_timing(edid, timing, ಬಹುಲಕ_valid,
							ಬಹುಲಕ_valid_priv);
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (!ಸಿಕ್ಕಿತು)
		ಹಿಂತಿರುಗಿ -EINVAL;

	ಆದರೆ (edid->version != 1 || edid->revision < 4) {
		debug("%s: EDID version %d.%d does not have required info\n",
		      __func__, edid->version, edid->revision);
		*panel_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_colourp = -1;
	} ಇಲ್ಲದಿದ್ದರೆ  {
		*panel_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_colourp =
			((edid->video_ಒಳಹರಿವು_definition & 0x70) >> 3) + 4;
	}

	timing->hdmi_monitor = false;
	ಆದರೆ (edid->extension_flag && (buf_ಗಾತ್ರ >= EDID_EXT_ಗಾತ್ರ)) {
		ರಚನೆ edid_cea861_info *info =
			(ರಚನೆ edid_cea861_info *)(buf + ಗಾತ್ರ(*edid));

		ಆದರೆ (info->extension_ಟ್ಯಾಗ್ == EDID_CEA861_EXTENSION_ಟ್ಯಾಗ್)
			timing->hdmi_monitor = cea_is_hdmi_vsdb_present(info);
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ edid_get_timing(u8 *buf, ಪೂರ್ಣಾಂಕ buf_ಗಾತ್ರ, ರಚನೆ display_timing *timing,
		    ಪೂರ್ಣಾಂಕ *panel_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_colourp)
{
	ಹಿಂತಿರುಗಿ edid_get_timing_validate(buf, buf_ಗಾತ್ರ, timing,
					panel_ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_colourp, ಖಾಲಿ, ಖಾಲಿ);
}

/**
 * Snip the tailing whitespace/return of a string.
 *
 * @param string	The string to be snipped
 * Return: the snipped string
 */
ಸ್ಥಿರ ಅಕ್ಷರ *snip(ಅಕ್ಷರ *string)
{
	ಅಕ್ಷರ *s;

	/*
	 * This is always a 13 character buffer
	 * and it's not always terminated.
	 */
	string[12] = '\0';
	s = &string[ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(string) - 1];

	ಆಗ (s >= string && (ಖಾಲಿಯೇ(*s) || *s == '\n' || *s == '\r' ||
			*s == '\0'))
		*(s--) = '\0';

	ಹಿಂತಿರುಗಿ string;
}

/**
 * Print an EDID monitor descriptor block
 *
 * @param monitor	The EDID monitor descriptor block
 * @have_timing		Modifies to 1 if the desciptor contains timing info
 */
ಸ್ಥಿರ ಶೂನ್ಯ edid_print_dtd(ರಚನೆ edid_monitor_descriptor *monitor,
			   ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *have_timing)
{
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಬೈಟುಗಳು = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)monitor;
	ರಚನೆ edid_detailed_timing *timing =
			(ರಚನೆ edid_detailed_timing *)monitor;

	ಆದರೆ (ಬೈಟುಗಳು[0] == 0 && ಬೈಟುಗಳು[1] == 0) {
		ಆದರೆ (monitor->type == EDID_MONITOR_DESCRIPTOR_SERIAL)
			ಬರೆ("Monitor serial number: %s\n",
			       snip(monitor->ದತ್ತಾಂಶ.string));
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (monitor->type == EDID_MONITOR_DESCRIPTOR_ASCII)
			ಬರೆ("Monitor ID: %s\n",
			       snip(monitor->ದತ್ತಾಂಶ.string));
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (monitor->type == EDID_MONITOR_DESCRIPTOR_NAME)
			ಬರೆ("Monitor name: %s\n",
			       snip(monitor->ದತ್ತಾಂಶ.string));
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (monitor->type == EDID_MONITOR_DESCRIPTOR_RANGE)
			ಬರೆ("Monitor range limits, horizontal sync: "
			       "%d-%d kHz, vertical refresh: "
			       "%d-%d Hz, max pixel clock: "
			       "%d MHz\n",
			       monitor->ದತ್ತಾಂಶ.range_ದತ್ತಾಂಶ.horizontal_min,
			       monitor->ದತ್ತಾಂಶ.range_ದತ್ತಾಂಶ.horizontal_max,
			       monitor->ದತ್ತಾಂಶ.range_ದತ್ತಾಂಶ.vertical_min,
			       monitor->ದತ್ತಾಂಶ.range_ದತ್ತಾಂಶ.vertical_max,
			       monitor->ದತ್ತಾಂಶ.range_ದತ್ತಾಂಶ.pixel_clock_max * 10);
	} ಇಲ್ಲದಿದ್ದರೆ {
		uint32_t pixclock, h_ಸಕ್ರಿಯ, h_blanking, v_ಸಕ್ರಿಯ, v_blanking;
		uint32_t h_ಒಟ್ಟು, v_ಒಟ್ಟು, vfreq;

		pixclock = EDID_DETAILED_TIMING_PIXEL_CLOCK(*timing);
		h_ಸಕ್ರಿಯ = EDID_DETAILED_TIMING_HORIZONTAL_ಸಕ್ರಿಯ(*timing);
		h_blanking = EDID_DETAILED_TIMING_HORIZONTAL_BLANKING(*timing);
		v_ಸಕ್ರಿಯ = EDID_DETAILED_TIMING_VERTICAL_ಸಕ್ರಿಯ(*timing);
		v_blanking = EDID_DETAILED_TIMING_VERTICAL_BLANKING(*timing);

		h_ಒಟ್ಟು = h_ಸಕ್ರಿಯ + h_blanking;
		v_ಒಟ್ಟು = v_ಸಕ್ರಿಯ + v_blanking;
		ಆದರೆ (v_ಒಟ್ಟು > 0 && h_ಒಟ್ಟು > 0)
			vfreq = pixclock / (v_ಒಟ್ಟು * h_ಒಟ್ಟು);
		ಇಲ್ಲದಿದ್ದರೆ
			vfreq = 1; /* Error case */
		ಬರೆ("\t%dx%d\%c\t%d Hz (detailed)\n", h_ಸಕ್ರಿಯ,
		       v_ಸಕ್ರಿಯ, h_ಸಕ್ರಿಯ > 1000 ? ' ' : '\t', vfreq);
		*have_timing = 1;
	}
}

/**
 * Get the manufacturer name from an EDID info.
 *
 * @param edid_info     The EDID info to be printed
 * @param name		Returns the string of the manufacturer name
 */
ಸ್ಥಿರ ಶೂನ್ಯ edid_get_manufacturer_name(ರಚನೆ edid1_info *edid, ಅಕ್ಷರ *name)
{
	name[0] = EDID1_INFO_MANUFACTURER_NAME_CHAR1(*edid) + 'A' - 1;
	name[1] = EDID1_INFO_MANUFACTURER_NAME_CHAR2(*edid) + 'A' - 1;
	name[2] = EDID1_INFO_MANUFACTURER_NAME_CHAR3(*edid) + 'A' - 1;
	name[3] = '\0';
}

ಶೂನ್ಯ edid_print_info(ರಚನೆ edid1_info *edid_info)
{
	ಪೂರ್ಣಾಂಕ i;
	ಅಕ್ಷರ manufacturer[4];
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ have_timing = 0;
	uint32_t serial_ಸಂಖ್ಯೆ;

	ಆದರೆ (edid_check_info(edid_info)) {
		ಬರೆ("Not a valid EDID\n");
		ಹಿಂತಿರುಗಿ;
	}

	ಬರೆ("EDID version: %d.%d\n",
	       edid_info->version, edid_info->revision);

	ಬರೆ("Product ID code: %04x\n", EDID1_INFO_ಗುಣಲಬ್ಧ_ಸಂಕೇತ(*edid_info));

	edid_get_manufacturer_name(edid_info, manufacturer);
	ಬರೆ("Manufacturer: %s\n", manufacturer);

	serial_ಸಂಖ್ಯೆ = EDID1_INFO_SERIAL_ಸಂಖ್ಯೆ(*edid_info);
	ಆದರೆ (serial_ಸಂಖ್ಯೆ != 0xffffffff) {
		ಆದರೆ (strcmp(manufacturer, "MAG") == 0)
			serial_ಸಂಖ್ಯೆ -= 0x7000000;
		ಆದರೆ (strcmp(manufacturer, "OQI") == 0)
			serial_ಸಂಖ್ಯೆ -= 456150000;
		ಆದರೆ (strcmp(manufacturer, "VSC") == 0)
			serial_ಸಂಖ್ಯೆ -= 640000000;
	}
	ಬರೆ("Serial number: %08x\n", serial_ಸಂಖ್ಯೆ);
	ಬರೆ("Manufactured in week: %d year: %d\n",
	       edid_info->week, edid_info->year + 1990);

	ಬರೆ("Video input definition: %svoltage level %d%s%s%s%s%s\n",
	       EDID1_INFO_VIDEO_ಒಳಹರಿವು_DIGITAL(*edid_info) ?
	       "digital signal, " : "analog signal, ",
	       EDID1_INFO_VIDEO_ಒಳಹರಿವು_VOLTAGE_LEVEL(*edid_info),
	       EDID1_INFO_VIDEO_ಒಳಹರಿವು_BLANK_ಗಮ್ಯ_BLACK(*edid_info) ?
	       ", blank to black" : "",
	       EDID1_INFO_VIDEO_ಒಳಹರಿವು_SEPARATE_SYNC(*edid_info) ?
	       ", separate sync" : "",
	       EDID1_INFO_VIDEO_ಒಳಹರಿವು_ಸಂಯುಕ್ತ_SYNC(*edid_info) ?
	       ", composite sync" : "",
	       EDID1_INFO_VIDEO_ಒಳಹರಿವು_SYNC_ON_GREEN(*edid_info) ?
	       ", sync on green" : "",
	       EDID1_INFO_VIDEO_ಒಳಹರಿವು_SERRATION_V(*edid_info) ?
	       ", serration v" : "");

	ಬರೆ("Monitor is %s\n",
	       EDID1_INFO_FEATURE_RGB(*edid_info) ? "RGB" : "non-RGB");

	ಬರೆ("Maximum visible display size: %d cm x %d cm\n",
	       edid_info->max_ಗಾತ್ರ_horizontal,
	       edid_info->max_ಗಾತ್ರ_vertical);

	ಬರೆ("Power management features: %s%s, %s%s, %s%s\n",
	       EDID1_INFO_FEATURE_ಸಕ್ರಿಯ_OFF(*edid_info) ?
	       "" : "no ", "active off",
	       EDID1_INFO_FEATURE_SUSPEND(*edid_info) ? "" : "no ", "suspend",
	       EDID1_INFO_FEATURE_STANDBY(*edid_info) ? "" : "no ", "standby");

	ಬರೆ("Estabilished timings:\n");
	ಆದರೆ (EDID1_INFO_ESTABLISHED_TIMING_720X400_70(*edid_info))
		ಬರೆ("\t720x400\t\t70 Hz (VGA 640x400, IBM)\n");
	ಆದರೆ (EDID1_INFO_ESTABLISHED_TIMING_720X400_88(*edid_info))
		ಬರೆ("\t720x400\t\t88 Hz (XGA2)\n");
	ಆದರೆ (EDID1_INFO_ESTABLISHED_TIMING_640X480_60(*edid_info))
		ಬರೆ("\t640x480\t\t60 Hz (VGA)\n");
	ಆದರೆ (EDID1_INFO_ESTABLISHED_TIMING_640X480_67(*edid_info))
		ಬರೆ("\t640x480\t\t67 Hz (Mac II, Apple)\n");
	ಆದರೆ (EDID1_INFO_ESTABLISHED_TIMING_640X480_72(*edid_info))
		ಬರೆ("\t640x480\t\t72 Hz (VESA)\n");
	ಆದರೆ (EDID1_INFO_ESTABLISHED_TIMING_640X480_75(*edid_info))
		ಬರೆ("\t640x480\t\t75 Hz (VESA)\n");
	ಆದರೆ (EDID1_INFO_ESTABLISHED_TIMING_800X600_56(*edid_info))
		ಬರೆ("\t800x600\t\t56 Hz (VESA)\n");
	ಆದರೆ (EDID1_INFO_ESTABLISHED_TIMING_800X600_60(*edid_info))
		ಬರೆ("\t800x600\t\t60 Hz (VESA)\n");
	ಆದರೆ (EDID1_INFO_ESTABLISHED_TIMING_800X600_72(*edid_info))
		ಬರೆ("\t800x600\t\t72 Hz (VESA)\n");
	ಆದರೆ (EDID1_INFO_ESTABLISHED_TIMING_800X600_75(*edid_info))
		ಬರೆ("\t800x600\t\t75 Hz (VESA)\n");
	ಆದರೆ (EDID1_INFO_ESTABLISHED_TIMING_832X624_75(*edid_info))
		ಬರೆ("\t832x624\t\t75 Hz (Mac II)\n");
	ಆದರೆ (EDID1_INFO_ESTABLISHED_TIMING_1024X768_87I(*edid_info))
		ಬರೆ("\t1024x768\t87 Hz Interlaced (8514A)\n");
	ಆದರೆ (EDID1_INFO_ESTABLISHED_TIMING_1024X768_60(*edid_info))
		ಬರೆ("\t1024x768\t60 Hz (VESA)\n");
	ಆದರೆ (EDID1_INFO_ESTABLISHED_TIMING_1024X768_70(*edid_info))
		ಬರೆ("\t1024x768\t70 Hz (VESA)\n");
	ಆದರೆ (EDID1_INFO_ESTABLISHED_TIMING_1024X768_75(*edid_info))
		ಬರೆ("\t1024x768\t75 Hz (VESA)\n");
	ಆದರೆ (EDID1_INFO_ESTABLISHED_TIMING_1280X1024_75(*edid_info))
		ಬರೆ("\t1280x1024\t75 (VESA)\n");
	ಆದರೆ (EDID1_INFO_ESTABLISHED_TIMING_1152X870_75(*edid_info))
		ಬರೆ("\t1152x870\t75 (Mac II)\n");

	/* Standard timings. */
	ಬರೆ("Standard timings:\n");
	ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(edid_info->standard_timings); i++) {
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ x, y, freq;

		ಆದರೆ (edid_get_standard_timing(edid_info, i, &x, &y, &freq)) {
			ಬರೆ("\t%dx%d%c\t%d Hz\n", x, y,
			       x > 1000 ? ' ' : '\t', freq);
			have_timing = 1;
		}
	}

	/* Detailed timing information. */
	ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(edid_info->monitor_details.descriptor);
			i++) {
		edid_print_dtd(&edid_info->monitor_details.descriptor[i],
			       &have_timing);
	}

	ಆದರೆ (!have_timing)
		ಬರೆ("\tNone\n");
}
