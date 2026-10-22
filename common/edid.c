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

#ਅਨ੍ਤਰ੍ਭਾਵ <edid.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fdtdec.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/string.h>

#ਜੇ CONFIG_IS_ENABLED(I2C_EDID_STANDARD)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIMING(c, ha, hfp, hbp, hsl, va, vfp, vbp, vsl, f)	\
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

ਸਥਿਰ ਅਚਲ ਬਣਤਰ display_timing dmt_timings[] = {
	{ TIMING(31500000, 640, 32, 64, 96, 350, 32, 3, 60,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(31500000, 640, 32, 64, 96, 400, 1, 3, 41,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(35500000, 720, 36, 72, 108, 400, 1, 3, 42,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(25175000, 640, 16, 96, 48, 480, 10, 2, 33,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(31500000, 640, 24, 40, 128, 480, 9, 3, 28,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(31500000, 640, 16, 64, 120, 480, 1, 3, 16,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(36000000, 640, 56, 56, 80, 480, 1, 3, 25,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(36000000, 800, 24, 72, 128, 600, 1, 2, 22,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(40000000, 800, 40, 128, 88, 600, 1, 4, 23,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(50000000, 800, 56, 120, 64, 600, 37, 6, 23,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(49500000, 800, 16, 80, 160, 600, 1, 3, 21,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(56250000, 800, 32, 64, 152, 600, 1, 3, 27,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(73250000, 800, 48, 32, 80, 600, 3, 4, 29,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(33750000, 848, 16, 112, 112, 480, 6, 8, 23,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(44900000, 1024, 8, 176, 56, 768, 0, 8, 41,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(65000000, 1024, 24, 136, 160, 768, 3, 6, 29,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(75000000, 1024, 24, 136, 144, 768, 3, 6, 29,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(78750000, 1024, 16, 96, 176, 768, 1, 3, 28,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(94500000, 1024, 48, 96, 208, 768, 1, 3, 36,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(115500000, 1024, 48, 32, 80, 768, 3, 4, 38,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(108000000, 1152, 64, 128, 256, 864, 1, 3, 32,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(74250000, 1280, 110, 40, 220, 720, 5, 5, 20,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(68250000, 1280, 48, 32, 80, 768, 3, 7, 12,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(79500000, 1280, 64, 128, 192, 768, 3, 7, 20,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(102250000, 1280, 80, 128, 208, 768, 3, 7, 27,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(117500000, 1280, 80, 136, 216, 768, 3, 7, 31,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(140250000, 1280, 48, 32, 80, 768, 3, 7, 35,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(71000000, 1280, 48, 32, 80, 800, 3, 6, 14,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(83500000, 1280, 72, 128, 200, 800, 3, 6, 22,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(106500000, 1280, 80, 128, 208, 800, 3, 6, 29,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(122500000, 1280, 80, 136, 216, 800, 3, 6, 34,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(146250000, 1280, 48, 32, 80, 800, 3, 6, 38,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(108000000, 1280, 96, 112, 312, 960, 1, 3, 36,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(148500000, 1280, 64, 160, 224, 960, 1, 3, 47,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(175500000, 1280, 48, 32, 80, 960, 3, 4, 50,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(108000000, 1280, 48, 112, 248, 1024, 1, 3, 38,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(135000000, 1280, 16, 144, 248, 1024, 1, 3, 38,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(157500000, 1280, 64, 160, 224, 1024, 1, 3, 44,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(187250000, 1280, 48, 32, 80, 1024, 3, 7, 50,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(85500000, 1360, 64, 112, 256, 768, 3, 6, 18,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(148250000, 1360, 48, 32, 80, 768, 3, 5, 37,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(85500000, 1366, 70, 143, 213, 768, 3, 3, 24,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(72000000, 1366, 14, 56, 64, 768, 1, 3, 28,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(101000000, 1400, 48, 32, 80, 1050, 3, 4, 23,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(121750000, 1400, 88, 144, 232, 1050, 3, 4, 32,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(156000000, 1400, 104, 144, 248, 1050, 3, 4, 42,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(179500000, 1400, 104, 152, 256, 1050, 3, 4, 48,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(208000000, 1400, 48, 32, 80, 1050, 3, 4, 55,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(88750000, 1440, 48, 32, 80, 900, 3, 6, 17,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(106500000, 1440, 80, 152, 232, 900, 3, 6, 25,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(136750000, 1440, 96, 152, 248, 900, 3, 6, 33,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(157000000, 1440, 104, 152, 256, 900, 3, 6, 39,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(182750000, 1440, 48, 32, 80, 900, 3, 6, 44,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(108000000, 1600, 24, 80, 96, 900, 1, 3, 96,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(162000000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(175500000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(189000000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(202500000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(229500000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(268250000, 1600, 48, 32, 80, 1200, 3, 4, 64,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(119000000, 1680, 48, 32, 80, 1050, 3, 6, 21,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(146250000, 1680, 104, 176, 280, 1050, 3, 6, 30,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(187000000, 1680, 120, 176, 296, 1050, 3, 6, 40,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(214750000, 1680, 128, 176, 304, 1050, 3, 6, 46,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(245500000, 1680, 48, 32, 80, 1050, 3, 6, 53,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(204750000, 1792, 128, 200, 328, 1344, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(261000000, 1792, 96, 216, 352, 1344, 1, 3, 69,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(333250000, 1792, 48, 32, 80, 1344, 3, 4, 72,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(218250000, 1856, 96, 224, 352, 1392, 1, 3, 43,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(288000000, 1856, 128, 224, 352, 1392, 1, 3, 104,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(356500000, 1856, 48, 32, 80, 1392, 3, 4, 75,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(148500000, 1920, 88, 44, 148, 1080, 4, 5, 36,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(154000000, 1920, 48, 32, 80, 1200, 3, 6, 26,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(193250000, 1920, 136, 200, 336, 1200, 3, 6, 36,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(245250000, 1920, 136, 208, 344, 1200, 3, 6, 46,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(281250000, 1920, 144, 208, 352, 1200, 3, 6, 53,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(317000000, 1920, 48, 32, 80, 1200, 3, 6, 62,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(234000000, 1920, 128, 208, 344, 1440, 1, 3, 56,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(297000000, 1920, 144, 224, 352, 1440, 1, 3, 56,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(380500000, 1920, 48, 32, 80, 1440, 3, 4, 78,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(162000000, 2048, 26, 80, 96, 1152, 1, 3, 44,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(268500000, 2560, 48, 32, 80, 1600, 3, 6, 37,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(348500000, 2560, 192, 280, 472, 1600, 3, 6, 49,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(443250000, 2560, 208, 280, 488, 1600, 3, 6, 63,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(505250000, 2560, 208, 280, 488, 1600, 3, 6, 73,
		 DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ | DISPLAY_FLAGS_VSYNC_ਉੱਚਾ) },
	{ TIMING(552750000, 2560, 48, 32, 80, 1600, 3, 6, 85,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(556744000, 4096, 8, 32, 40, 2160, 48, 8, 6,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
	{ TIMING(556188000, 4096, 8, 32, 40, 2160, 48, 8, 6,
		 DISPLAY_FLAGS_HSYNC_ਉੱਚਾ | DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ) },
};
#ਜੇ_ਅੰਤ

ਪੂਰਨ edid_check_info(ਬਣਤਰ edid1_info *edid_info)
{
	ਜੇ ((edid_info == ਨਿਰਾ) || (edid_info->version == 0))
		ਵਾਪਸ -1;

	ਜੇ (memcmp(edid_info->ਸਿਰਲੇਖ, "\x0\xff\xff\xff\xff\xff\xff\x0", 8))
		ਵਾਪਸ -1;

	ਜੇ (edid_info->version == 0xff && edid_info->revision == 0xff)
		ਵਾਪਸ -1;

	ਵਾਪਸ 0;
}

ਪੂਰਨ edid_check_checksum(u8 *edid_ਖੰਡ)
{
	u8 checksum = 0;
	ਪੂਰਨ i;

	ਲਈ (i = 0; i < 128; i++)
		checksum += edid_ਖੰਡ[i];

	ਵਾਪਸ (checksum == 0) ? 0 : -EINVAL;
}

ਪੂਰਨ edid_get_ranges(ਬਣਤਰ edid1_info *edid, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *hmin,
		    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *hmax, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *vmin,
		    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *vmax)
{
	ਪੂਰਨ i;
	ਬਣਤਰ edid_monitor_descriptor *monitor;

	*hmin = *hmax = *vmin = *vmax = 0;
	ਜੇ (edid_check_info(edid))
		ਵਾਪਸ -1;

	ਲਈ (i = 0; i < ARRAY_ਮਾਪ(edid->monitor_details.descriptor); i++) {
		monitor = &edid->monitor_details.descriptor[i];
		ਜੇ (monitor->type == EDID_MONITOR_DESCRIPTOR_RANGE) {
			*hmin = monitor->ਅੰਕੜੇ.range_ਅੰਕੜੇ.horizontal_min;
			*hmax = monitor->ਅੰਕੜੇ.range_ਅੰਕੜੇ.horizontal_max;
			*vmin = monitor->ਅੰਕੜੇ.range_ਅੰਕੜੇ.vertical_min;
			*vmax = monitor->ਅੰਕੜੇ.range_ਅੰਕੜੇ.vertical_max;
			ਵਾਪਸ 0;
		}
	}
	ਵਾਪਸ -1;
}

/* Set all parts of a timing entry to the same value */
ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_ਇੰਦਰਾਜ(ਬਣਤਰ timing_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ, u32 ਮੁੱਲ)
{
	ਇੰਦਰਾਜ->min = ਮੁੱਲ;
	ਇੰਦਰਾਜ->typ = ਮੁੱਲ;
	ਇੰਦਰਾਜ->max = ਮੁੱਲ;
}

/**
 * decode_timing() - Decoding an 18-byte detailed timing record
 *
 * @buf:	Pointer to EDID detailed timing record
 * @timing:	Place to put timing
 */
ਸਥਿਰ ਖਾਲੀ decode_timing(u8 *buf, ਬਣਤਰ display_timing *timing)
{
	uint x_mm, y_mm;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ha, hbl, hso, hspw, hborder;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ va, vbl, vso, vspw, vborder;
	ਬਣਤਰ edid_detailed_timing *t = (ਬਣਤਰ edid_detailed_timing *)buf;

	/* Edid contains pixel clock in terms of 10KHz */
	ਸੈੱਟ_ਇੰਦਰਾਜ(&timing->pixelclock, (buf[0] + (buf[1] << 8)) * 10000);
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

	ਸੈੱਟ_ਇੰਦਰਾਜ(&timing->hactive, ha);
	ਸੈੱਟ_ਇੰਦਰਾਜ(&timing->hfront_porch, hso);
	ਸੈੱਟ_ਇੰਦਰਾਜ(&timing->hback_porch, hbl - hso - hspw);
	ਸੈੱਟ_ਇੰਦਰਾਜ(&timing->hsync_len, hspw);

	ਸੈੱਟ_ਇੰਦਰਾਜ(&timing->vactive, va);
	ਸੈੱਟ_ਇੰਦਰਾਜ(&timing->vfront_porch, vso);
	ਸੈੱਟ_ਇੰਦਰਾਜ(&timing->vback_porch, vbl - vso - vspw);
	ਸੈੱਟ_ਇੰਦਰਾਜ(&timing->vsync_len, vspw);

	timing->flags = 0;
	ਜੇ (EDID_DETAILED_TIMING_FLAG_HSYNC_POLARITY(*t))
		timing->flags |= DISPLAY_FLAGS_HSYNC_ਉੱਚਾ;
	ਨਹੀਂਤਾਂ
		timing->flags |= DISPLAY_FLAGS_HSYNC_ਨੀਵਾਂ;
	ਜੇ (EDID_DETAILED_TIMING_FLAG_VSYNC_POLARITY(*t))
		timing->flags |= DISPLAY_FLAGS_VSYNC_ਉੱਚਾ;
	ਨਹੀਂਤਾਂ
		timing->flags |= DISPLAY_FLAGS_VSYNC_ਨੀਵਾਂ;

	ਜੇ (EDID_DETAILED_TIMING_FLAG_INTERLACED(*t))
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
ਸਥਿਰ bool cea_is_hdmi_vsdb_present(ਬਣਤਰ edid_cea861_info *info)
{
	u8 ਸਿਰਾ_2, i = 0;

	/* check for end of data block */
	ਸਿਰਾ_2 = info->dtd_ਵਿੱਥ;
	ਜੇ (ਸਿਰਾ_2 == 0)
		ਸਿਰਾ_2 = ਆਕਾਰ(info->ਅੰਕੜੇ);
	ਜੇ (ਸਿਰਾ_2 < 4 || ਸਿਰਾ_2 > ਆਕਾਰ(info->ਅੰਕੜੇ))
		ਵਾਪਸ false;
	ਸਿਰਾ_2 -= 4;

	ਜਦੋਂ (i < ਸਿਰਾ_2) {
		/* Look for vendor specific data block of appropriate size */
		ਜੇ ((EDID_CEA861_DB_TYPE(*info, i) == EDID_CEA861_DB_VENDOR) &&
		    (EDID_CEA861_DB_LEN(*info, i) >= 5)) {
			u8 *db = &info->ਅੰਕੜੇ[i + 1];
			u32 oui = db[0] | (db[1] << 8) | (db[2] << 16);

			ਜੇ (oui == HDMI_IEEE_OUI)
				ਵਾਪਸ true;
		}
		i += EDID_CEA861_DB_LEN(*info, i) + 1;
	}

	ਵਾਪਸ false;
}

ਸਥਿਰ bool edid_ਲੱਭੋ_valid_detailed_timing(ਖਾਲੀ *buf, ਪੂਰਨ ਗਿਣਤੀ_2,
					    ਬਣਤਰ display_timing *timing,
					    bool (*ਬਹੁਲਕ_valid)(ਖਾਲੀ *priv,
						ਅਚਲ ਬਣਤਰ display_timing *timing),
					    ਖਾਲੀ *ਬਹੁਲਕ_valid_priv)
{
	ਬਣਤਰ edid_detailed_timing *t = buf;
	bool ਮਿਲਿਆ = false;
	ਪੂਰਨ i;

	ਲਈ (i = 0; i < ਗਿਣਤੀ_2 && !ਮਿਲਿਆ; i++, t++)
		ਜੇ (EDID_DETAILED_TIMING_PIXEL_CLOCK(*t) != 0) {
			decode_timing((u8 *)t, timing);
			ਜੇ (ਬਹੁਲਕ_valid)
				ਮਿਲਿਆ = ਬਹੁਲਕ_valid(ਬਹੁਲਕ_valid_priv,
						   timing);
			ਨਹੀਂਤਾਂ
				ਮਿਲਿਆ = true;
		}

	ਵਾਪਸ ਮਿਲਿਆ;
}

ਸਥਿਰ bool edid_get_standard_timing(ਬਣਤਰ edid1_info *edid, ਪੂਰਨ i, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *x,
				     ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *y, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *freq)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ aspect = 10000;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ xres, vfreq;

	xres = EDID1_INFO_STANDARD_TIMING_XRESOLUTION(*edid, i);
	vfreq = EDID1_INFO_STANDARD_TIMING_VFREQ(*edid, i);
	ਜੇ (xres != vfreq || (xres != 0 && xres != 1) ||
	    (vfreq != 0 && vfreq != 1)) {
		ਚੋਣ (EDID1_INFO_STANDARD_TIMING_ASPECT(*edid, i)) {
		ਹਾਲਤ ASPECT_625: // 16:10
			aspect = 6250;
			ਰੋਕੋ;
		ਹਾਲਤ ASPECT_75: // 4:3
			aspect = 7500;
			ਰੋਕੋ;
		ਹਾਲਤ ASPECT_8: // 5:4
			aspect = 8000;
			ਰੋਕੋ;
		ਹਾਲਤ ASPECT_5625: // 16:9
			aspect = 5625;
			ਰੋਕੋ;
		}

		*x = (xres + 31) * 8;
		*y = *x * aspect / 10000;
		*freq = (vfreq & 0x3f) + 60;

		ਵਾਪਸ true;
	}

	ਵਾਪਸ false;
}

#ਜੇ CONFIG_IS_ENABLED(I2C_EDID_STANDARD)
ਸਥਿਰ bool edid_ਲੱਭੋ_valid_standard_timing(ਬਣਤਰ edid1_info *buf,
					    ਬਣਤਰ display_timing *timing,
					    bool (*ਬਹੁਲਕ_valid)(ਖਾਲੀ *priv,
						ਅਚਲ ਬਣਤਰ display_timing *timing),
					    ਖਾਲੀ *ਬਹੁਲਕ_valid_priv)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ x, y, freq;
	bool ਮਿਲਿਆ = false;
	ਪੂਰਨ i, k;

	ਲਈ (i = 0; i < ARRAY_ਮਾਪ(buf->standard_timings); i++) {
		ਜੇ (!edid_get_standard_timing(buf, i, &x, &y, &freq))
			ਜਾਰੀ;

		ਲਈ (k = 0; k < ARRAY_ਮਾਪ(dmt_timings); k++) {
			ਅਚਲ ਬਣਤਰ display_timing *dt = &dmt_timings[k];

			ਜੇ (dt->hactive.typ == x && dt->vactive.typ == y) {
				ਮਿਲਿਆ = ਬਹੁਲਕ_valid(ਬਹੁਲਕ_valid_priv, dt);
				ਜੇ (ਮਿਲਿਆ) {
					ਸਮ੍ਰਿਤੀ_ਨਕਲ(timing, dt, ਆਕਾਰ(*timing));
					ਵਾਪਸ true;
				}
			}
		}
	}

	ਵਾਪਸ ਮਿਲਿਆ;
}
#ਜੇ_ਅੰਤ

ਪੂਰਨ edid_get_timing_validate(u8 *buf, ਪੂਰਨ buf_ਮਾਪ,
			     ਬਣਤਰ display_timing *timing,
			     ਪੂਰਨ *panel_ਬਿੱਟ_ਪ੍ਰਤੀ_colourp,
			     bool (*ਬਹੁਲਕ_valid)(ਖਾਲੀ *priv,
					ਅਚਲ ਬਣਤਰ display_timing *timing),
			     ਖਾਲੀ *ਬਹੁਲਕ_valid_priv)
{
	ਬਣਤਰ edid1_info *edid = (ਬਣਤਰ edid1_info *)buf;
	bool ਮਿਲਿਆ;

	ਜੇ (buf_ਮਾਪ < ਆਕਾਰ(*edid) || edid_check_info(edid)) {
		debug("%s: Invalid buffer\n", __func__);
		ਵਾਪਸ -EINVAL;
	}

	ਜੇ (!EDID1_INFO_VIDEO_ਨਿਵੇਸ਼_DIGITAL(*edid)) {
		debug("%s: Not a digital display\n", __func__);
		ਵਾਪਸ -ENOSYS;
	}

	ਜੇ (!EDID1_INFO_FEATURE_PREFERRED_TIMING_ਬਹੁਲਕ(*edid)) {
		debug("%s: No preferred timing\n", __func__);
		ਵਾਪਸ -ENOENT;
	}

	/* Look for detailed timing in base EDID */
	ਮਿਲਿਆ = edid_ਲੱਭੋ_valid_detailed_timing(edid->monitor_details.descriptor, 4,
						timing, ਬਹੁਲਕ_valid, ਬਹੁਲਕ_valid_priv);

	/* Look for detailed timing in CTA-861 Extension Block */
	ਜੇ (!ਮਿਲਿਆ && edid->extension_flag && buf_ਮਾਪ >= EDID_EXT_ਮਾਪ) {
		ਬਣਤਰ edid_cea861_info *info =
			(ਬਣਤਰ edid_cea861_info *)(buf + ਆਕਾਰ(*edid));

		ਜੇ (info->extension_ਟੈਗ == EDID_CEA861_EXTENSION_ਟੈਗ) {
			ਪੂਰਨ ਗਿਣਤੀ_2 = EDID_CEA861_DTD_ਗਿਣਤੀ(*info);
			ਪੂਰਨ ਵਿੱਥ = info->dtd_ਵਿੱਥ;
			ਪੂਰਨ ਮਾਪ = ਗਿਣਤੀ_2 * ਆਕਾਰ(ਬਣਤਰ edid_detailed_timing);

			ਜੇ (ਵਿੱਥ >= 4 && ਵਿੱਥ + ਮਾਪ < EDID_ਮਾਪ)
				ਮਿਲਿਆ = edid_ਲੱਭੋ_valid_detailed_timing(
					(u8 *)info + ਵਿੱਥ, ਗਿਣਤੀ_2, timing,
					ਬਹੁਲਕ_valid, ਬਹੁਲਕ_valid_priv);
		}
	}

#ਜੇ CONFIG_IS_ENABLED(I2C_EDID_STANDARD)
	/* Look for timing in Standard Timings */
	ਜੇ (!ਮਿਲਿਆ)
		ਮਿਲਿਆ = edid_ਲੱਭੋ_valid_standard_timing(edid, timing, ਬਹੁਲਕ_valid,
							ਬਹੁਲਕ_valid_priv);
#ਜੇ_ਅੰਤ

	ਜੇ (!ਮਿਲਿਆ)
		ਵਾਪਸ -EINVAL;

	ਜੇ (edid->version != 1 || edid->revision < 4) {
		debug("%s: EDID version %d.%d does not have required info\n",
		      __func__, edid->version, edid->revision);
		*panel_ਬਿੱਟ_ਪ੍ਰਤੀ_colourp = -1;
	} ਨਹੀਂਤਾਂ  {
		*panel_ਬਿੱਟ_ਪ੍ਰਤੀ_colourp =
			((edid->video_ਨਿਵੇਸ਼_definition & 0x70) >> 3) + 4;
	}

	timing->hdmi_monitor = false;
	ਜੇ (edid->extension_flag && (buf_ਮਾਪ >= EDID_EXT_ਮਾਪ)) {
		ਬਣਤਰ edid_cea861_info *info =
			(ਬਣਤਰ edid_cea861_info *)(buf + ਆਕਾਰ(*edid));

		ਜੇ (info->extension_ਟੈਗ == EDID_CEA861_EXTENSION_ਟੈਗ)
			timing->hdmi_monitor = cea_is_hdmi_vsdb_present(info);
	}

	ਵਾਪਸ 0;
}

ਪੂਰਨ edid_get_timing(u8 *buf, ਪੂਰਨ buf_ਮਾਪ, ਬਣਤਰ display_timing *timing,
		    ਪੂਰਨ *panel_ਬਿੱਟ_ਪ੍ਰਤੀ_colourp)
{
	ਵਾਪਸ edid_get_timing_validate(buf, buf_ਮਾਪ, timing,
					panel_ਬਿੱਟ_ਪ੍ਰਤੀ_colourp, ਨਿਰਾ, ਨਿਰਾ);
}

/**
 * Snip the tailing whitespace/return of a string.
 *
 * @param string	The string to be snipped
 * Return: the snipped string
 */
ਸਥਿਰ ਅੱਖਰ *snip(ਅੱਖਰ *string)
{
	ਅੱਖਰ *s;

	/*
	 * This is always a 13 character buffer
	 * and it's not always terminated.
	 */
	string[12] = '\0';
	s = &string[ਲੜੀ_ਲੰਬਾਈ(string) - 1];

	ਜਦੋਂ (s >= string && (ਖਾਲੀ_ਥਾਂ_ਹੈ(*s) || *s == '\n' || *s == '\r' ||
			*s == '\0'))
		*(s--) = '\0';

	ਵਾਪਸ string;
}

/**
 * Print an EDID monitor descriptor block
 *
 * @param monitor	The EDID monitor descriptor block
 * @have_timing		Modifies to 1 if the desciptor contains timing info
 */
ਸਥਿਰ ਖਾਲੀ edid_print_dtd(ਬਣਤਰ edid_monitor_descriptor *monitor,
			   ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *have_timing)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਬਾਈਟਾਂ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)monitor;
	ਬਣਤਰ edid_detailed_timing *timing =
			(ਬਣਤਰ edid_detailed_timing *)monitor;

	ਜੇ (ਬਾਈਟਾਂ[0] == 0 && ਬਾਈਟਾਂ[1] == 0) {
		ਜੇ (monitor->type == EDID_MONITOR_DESCRIPTOR_SERIAL)
			ਲਿਖੋ("Monitor serial number: %s\n",
			       snip(monitor->ਅੰਕੜੇ.string));
		ਨਹੀਂਤਾਂ ਜੇ (monitor->type == EDID_MONITOR_DESCRIPTOR_ASCII)
			ਲਿਖੋ("Monitor ID: %s\n",
			       snip(monitor->ਅੰਕੜੇ.string));
		ਨਹੀਂਤਾਂ ਜੇ (monitor->type == EDID_MONITOR_DESCRIPTOR_NAME)
			ਲਿਖੋ("Monitor name: %s\n",
			       snip(monitor->ਅੰਕੜੇ.string));
		ਨਹੀਂਤਾਂ ਜੇ (monitor->type == EDID_MONITOR_DESCRIPTOR_RANGE)
			ਲਿਖੋ("Monitor range limits, horizontal sync: "
			       "%d-%d kHz, vertical refresh: "
			       "%d-%d Hz, max pixel clock: "
			       "%d MHz\n",
			       monitor->ਅੰਕੜੇ.range_ਅੰਕੜੇ.horizontal_min,
			       monitor->ਅੰਕੜੇ.range_ਅੰਕੜੇ.horizontal_max,
			       monitor->ਅੰਕੜੇ.range_ਅੰਕੜੇ.vertical_min,
			       monitor->ਅੰਕੜੇ.range_ਅੰਕੜੇ.vertical_max,
			       monitor->ਅੰਕੜੇ.range_ਅੰਕੜੇ.pixel_clock_max * 10);
	} ਨਹੀਂਤਾਂ {
		uint32_t pixclock, h_ਸਰਗਰਮ, h_blanking, v_ਸਰਗਰਮ, v_blanking;
		uint32_t h_ਕੁੱਲ, v_ਕੁੱਲ, vfreq;

		pixclock = EDID_DETAILED_TIMING_PIXEL_CLOCK(*timing);
		h_ਸਰਗਰਮ = EDID_DETAILED_TIMING_HORIZONTAL_ਸਰਗਰਮ(*timing);
		h_blanking = EDID_DETAILED_TIMING_HORIZONTAL_BLANKING(*timing);
		v_ਸਰਗਰਮ = EDID_DETAILED_TIMING_VERTICAL_ਸਰਗਰਮ(*timing);
		v_blanking = EDID_DETAILED_TIMING_VERTICAL_BLANKING(*timing);

		h_ਕੁੱਲ = h_ਸਰਗਰਮ + h_blanking;
		v_ਕੁੱਲ = v_ਸਰਗਰਮ + v_blanking;
		ਜੇ (v_ਕੁੱਲ > 0 && h_ਕੁੱਲ > 0)
			vfreq = pixclock / (v_ਕੁੱਲ * h_ਕੁੱਲ);
		ਨਹੀਂਤਾਂ
			vfreq = 1; /* Error case */
		ਲਿਖੋ("\t%dx%d\%c\t%d Hz (detailed)\n", h_ਸਰਗਰਮ,
		       v_ਸਰਗਰਮ, h_ਸਰਗਰਮ > 1000 ? ' ' : '\t', vfreq);
		*have_timing = 1;
	}
}

/**
 * Get the manufacturer name from an EDID info.
 *
 * @param edid_info     The EDID info to be printed
 * @param name		Returns the string of the manufacturer name
 */
ਸਥਿਰ ਖਾਲੀ edid_get_manufacturer_name(ਬਣਤਰ edid1_info *edid, ਅੱਖਰ *name)
{
	name[0] = EDID1_INFO_MANUFACTURER_NAME_CHAR1(*edid) + 'A' - 1;
	name[1] = EDID1_INFO_MANUFACTURER_NAME_CHAR2(*edid) + 'A' - 1;
	name[2] = EDID1_INFO_MANUFACTURER_NAME_CHAR3(*edid) + 'A' - 1;
	name[3] = '\0';
}

ਖਾਲੀ edid_print_info(ਬਣਤਰ edid1_info *edid_info)
{
	ਪੂਰਨ i;
	ਅੱਖਰ manufacturer[4];
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ have_timing = 0;
	uint32_t serial_ਸੰਖਿਆ;

	ਜੇ (edid_check_info(edid_info)) {
		ਲਿਖੋ("Not a valid EDID\n");
		ਵਾਪਸ;
	}

	ਲਿਖੋ("EDID version: %d.%d\n",
	       edid_info->version, edid_info->revision);

	ਲਿਖੋ("Product ID code: %04x\n", EDID1_INFO_ਗੁਣਨਫਲ_ਕੋਡ(*edid_info));

	edid_get_manufacturer_name(edid_info, manufacturer);
	ਲਿਖੋ("Manufacturer: %s\n", manufacturer);

	serial_ਸੰਖਿਆ = EDID1_INFO_SERIAL_ਸੰਖਿਆ(*edid_info);
	ਜੇ (serial_ਸੰਖਿਆ != 0xffffffff) {
		ਜੇ (strcmp(manufacturer, "MAG") == 0)
			serial_ਸੰਖਿਆ -= 0x7000000;
		ਜੇ (strcmp(manufacturer, "OQI") == 0)
			serial_ਸੰਖਿਆ -= 456150000;
		ਜੇ (strcmp(manufacturer, "VSC") == 0)
			serial_ਸੰਖਿਆ -= 640000000;
	}
	ਲਿਖੋ("Serial number: %08x\n", serial_ਸੰਖਿਆ);
	ਲਿਖੋ("Manufactured in week: %d year: %d\n",
	       edid_info->week, edid_info->year + 1990);

	ਲਿਖੋ("Video input definition: %svoltage level %d%s%s%s%s%s\n",
	       EDID1_INFO_VIDEO_ਨਿਵੇਸ਼_DIGITAL(*edid_info) ?
	       "digital signal, " : "analog signal, ",
	       EDID1_INFO_VIDEO_ਨਿਵੇਸ਼_VOLTAGE_LEVEL(*edid_info),
	       EDID1_INFO_VIDEO_ਨਿਵੇਸ਼_BLANK_ਮੰਜ਼ਿਲ_BLACK(*edid_info) ?
	       ", blank to black" : "",
	       EDID1_INFO_VIDEO_ਨਿਵੇਸ਼_SEPARATE_SYNC(*edid_info) ?
	       ", separate sync" : "",
	       EDID1_INFO_VIDEO_ਨਿਵੇਸ਼_ਸੰਯੁਕਤ_SYNC(*edid_info) ?
	       ", composite sync" : "",
	       EDID1_INFO_VIDEO_ਨਿਵੇਸ਼_SYNC_ON_GREEN(*edid_info) ?
	       ", sync on green" : "",
	       EDID1_INFO_VIDEO_ਨਿਵੇਸ਼_SERRATION_V(*edid_info) ?
	       ", serration v" : "");

	ਲਿਖੋ("Monitor is %s\n",
	       EDID1_INFO_FEATURE_RGB(*edid_info) ? "RGB" : "non-RGB");

	ਲਿਖੋ("Maximum visible display size: %d cm x %d cm\n",
	       edid_info->max_ਮਾਪ_horizontal,
	       edid_info->max_ਮਾਪ_vertical);

	ਲਿਖੋ("Power management features: %s%s, %s%s, %s%s\n",
	       EDID1_INFO_FEATURE_ਸਰਗਰਮ_OFF(*edid_info) ?
	       "" : "no ", "active off",
	       EDID1_INFO_FEATURE_SUSPEND(*edid_info) ? "" : "no ", "suspend",
	       EDID1_INFO_FEATURE_STANDBY(*edid_info) ? "" : "no ", "standby");

	ਲਿਖੋ("Estabilished timings:\n");
	ਜੇ (EDID1_INFO_ESTABLISHED_TIMING_720X400_70(*edid_info))
		ਲਿਖੋ("\t720x400\t\t70 Hz (VGA 640x400, IBM)\n");
	ਜੇ (EDID1_INFO_ESTABLISHED_TIMING_720X400_88(*edid_info))
		ਲਿਖੋ("\t720x400\t\t88 Hz (XGA2)\n");
	ਜੇ (EDID1_INFO_ESTABLISHED_TIMING_640X480_60(*edid_info))
		ਲਿਖੋ("\t640x480\t\t60 Hz (VGA)\n");
	ਜੇ (EDID1_INFO_ESTABLISHED_TIMING_640X480_67(*edid_info))
		ਲਿਖੋ("\t640x480\t\t67 Hz (Mac II, Apple)\n");
	ਜੇ (EDID1_INFO_ESTABLISHED_TIMING_640X480_72(*edid_info))
		ਲਿਖੋ("\t640x480\t\t72 Hz (VESA)\n");
	ਜੇ (EDID1_INFO_ESTABLISHED_TIMING_640X480_75(*edid_info))
		ਲਿਖੋ("\t640x480\t\t75 Hz (VESA)\n");
	ਜੇ (EDID1_INFO_ESTABLISHED_TIMING_800X600_56(*edid_info))
		ਲਿਖੋ("\t800x600\t\t56 Hz (VESA)\n");
	ਜੇ (EDID1_INFO_ESTABLISHED_TIMING_800X600_60(*edid_info))
		ਲਿਖੋ("\t800x600\t\t60 Hz (VESA)\n");
	ਜੇ (EDID1_INFO_ESTABLISHED_TIMING_800X600_72(*edid_info))
		ਲਿਖੋ("\t800x600\t\t72 Hz (VESA)\n");
	ਜੇ (EDID1_INFO_ESTABLISHED_TIMING_800X600_75(*edid_info))
		ਲਿਖੋ("\t800x600\t\t75 Hz (VESA)\n");
	ਜੇ (EDID1_INFO_ESTABLISHED_TIMING_832X624_75(*edid_info))
		ਲਿਖੋ("\t832x624\t\t75 Hz (Mac II)\n");
	ਜੇ (EDID1_INFO_ESTABLISHED_TIMING_1024X768_87I(*edid_info))
		ਲਿਖੋ("\t1024x768\t87 Hz Interlaced (8514A)\n");
	ਜੇ (EDID1_INFO_ESTABLISHED_TIMING_1024X768_60(*edid_info))
		ਲਿਖੋ("\t1024x768\t60 Hz (VESA)\n");
	ਜੇ (EDID1_INFO_ESTABLISHED_TIMING_1024X768_70(*edid_info))
		ਲਿਖੋ("\t1024x768\t70 Hz (VESA)\n");
	ਜੇ (EDID1_INFO_ESTABLISHED_TIMING_1024X768_75(*edid_info))
		ਲਿਖੋ("\t1024x768\t75 Hz (VESA)\n");
	ਜੇ (EDID1_INFO_ESTABLISHED_TIMING_1280X1024_75(*edid_info))
		ਲਿਖੋ("\t1280x1024\t75 (VESA)\n");
	ਜੇ (EDID1_INFO_ESTABLISHED_TIMING_1152X870_75(*edid_info))
		ਲਿਖੋ("\t1152x870\t75 (Mac II)\n");

	/* Standard timings. */
	ਲਿਖੋ("Standard timings:\n");
	ਲਈ (i = 0; i < ARRAY_ਮਾਪ(edid_info->standard_timings); i++) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ x, y, freq;

		ਜੇ (edid_get_standard_timing(edid_info, i, &x, &y, &freq)) {
			ਲਿਖੋ("\t%dx%d%c\t%d Hz\n", x, y,
			       x > 1000 ? ' ' : '\t', freq);
			have_timing = 1;
		}
	}

	/* Detailed timing information. */
	ਲਈ (i = 0; i < ARRAY_ਮਾਪ(edid_info->monitor_details.descriptor);
			i++) {
		edid_print_dtd(&edid_info->monitor_details.descriptor[i],
			       &have_timing);
	}

	ਜੇ (!have_timing)
		ਲਿਖੋ("\tNone\n");
}
