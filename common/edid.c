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

#अन्तर्भाव <edid.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <fdtdec.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <linux/ctype.h>
#अन्तर्भाव <linux/string.h>

#जर CONFIG_IS_ENABLED(I2C_EDID_STANDARD)
#define TIMING(c, ha, hfp, hbp, hsl, va, vfp, vbp, vsl, f)	\
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

स्थिर स्थिरांक संरचना display_timing dmt_timings[] = {
	{ TIMING(31500000, 640, 32, 64, 96, 350, 32, 3, 60,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(31500000, 640, 32, 64, 96, 400, 1, 3, 41,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(35500000, 720, 36, 72, 108, 400, 1, 3, 42,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(25175000, 640, 16, 96, 48, 480, 10, 2, 33,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(31500000, 640, 24, 40, 128, 480, 9, 3, 28,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(31500000, 640, 16, 64, 120, 480, 1, 3, 16,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(36000000, 640, 56, 56, 80, 480, 1, 3, 25,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(36000000, 800, 24, 72, 128, 600, 1, 2, 22,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(40000000, 800, 40, 128, 88, 600, 1, 4, 23,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(50000000, 800, 56, 120, 64, 600, 37, 6, 23,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(49500000, 800, 16, 80, 160, 600, 1, 3, 21,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(56250000, 800, 32, 64, 152, 600, 1, 3, 27,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(73250000, 800, 48, 32, 80, 600, 3, 4, 29,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(33750000, 848, 16, 112, 112, 480, 6, 8, 23,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(44900000, 1024, 8, 176, 56, 768, 0, 8, 41,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(65000000, 1024, 24, 136, 160, 768, 3, 6, 29,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(75000000, 1024, 24, 136, 144, 768, 3, 6, 29,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(78750000, 1024, 16, 96, 176, 768, 1, 3, 28,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(94500000, 1024, 48, 96, 208, 768, 1, 3, 36,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(115500000, 1024, 48, 32, 80, 768, 3, 4, 38,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(108000000, 1152, 64, 128, 256, 864, 1, 3, 32,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(74250000, 1280, 110, 40, 220, 720, 5, 5, 20,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(68250000, 1280, 48, 32, 80, 768, 3, 7, 12,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(79500000, 1280, 64, 128, 192, 768, 3, 7, 20,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(102250000, 1280, 80, 128, 208, 768, 3, 7, 27,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(117500000, 1280, 80, 136, 216, 768, 3, 7, 31,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(140250000, 1280, 48, 32, 80, 768, 3, 7, 35,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(71000000, 1280, 48, 32, 80, 800, 3, 6, 14,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(83500000, 1280, 72, 128, 200, 800, 3, 6, 22,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(106500000, 1280, 80, 128, 208, 800, 3, 6, 29,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(122500000, 1280, 80, 136, 216, 800, 3, 6, 34,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(146250000, 1280, 48, 32, 80, 800, 3, 6, 38,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(108000000, 1280, 96, 112, 312, 960, 1, 3, 36,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(148500000, 1280, 64, 160, 224, 960, 1, 3, 47,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(175500000, 1280, 48, 32, 80, 960, 3, 4, 50,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(108000000, 1280, 48, 112, 248, 1024, 1, 3, 38,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(135000000, 1280, 16, 144, 248, 1024, 1, 3, 38,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(157500000, 1280, 64, 160, 224, 1024, 1, 3, 44,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(187250000, 1280, 48, 32, 80, 1024, 3, 7, 50,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(85500000, 1360, 64, 112, 256, 768, 3, 6, 18,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(148250000, 1360, 48, 32, 80, 768, 3, 5, 37,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(85500000, 1366, 70, 143, 213, 768, 3, 3, 24,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(72000000, 1366, 14, 56, 64, 768, 1, 3, 28,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(101000000, 1400, 48, 32, 80, 1050, 3, 4, 23,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(121750000, 1400, 88, 144, 232, 1050, 3, 4, 32,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(156000000, 1400, 104, 144, 248, 1050, 3, 4, 42,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(179500000, 1400, 104, 152, 256, 1050, 3, 4, 48,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(208000000, 1400, 48, 32, 80, 1050, 3, 4, 55,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(88750000, 1440, 48, 32, 80, 900, 3, 6, 17,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(106500000, 1440, 80, 152, 232, 900, 3, 6, 25,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(136750000, 1440, 96, 152, 248, 900, 3, 6, 33,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(157000000, 1440, 104, 152, 256, 900, 3, 6, 39,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(182750000, 1440, 48, 32, 80, 900, 3, 6, 44,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(108000000, 1600, 24, 80, 96, 900, 1, 3, 96,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(162000000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(175500000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(189000000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(202500000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(229500000, 1600, 64, 192, 304, 1200, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(268250000, 1600, 48, 32, 80, 1200, 3, 4, 64,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(119000000, 1680, 48, 32, 80, 1050, 3, 6, 21,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(146250000, 1680, 104, 176, 280, 1050, 3, 6, 30,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(187000000, 1680, 120, 176, 296, 1050, 3, 6, 40,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(214750000, 1680, 128, 176, 304, 1050, 3, 6, 46,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(245500000, 1680, 48, 32, 80, 1050, 3, 6, 53,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(204750000, 1792, 128, 200, 328, 1344, 1, 3, 46,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(261000000, 1792, 96, 216, 352, 1344, 1, 3, 69,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(333250000, 1792, 48, 32, 80, 1344, 3, 4, 72,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(218250000, 1856, 96, 224, 352, 1392, 1, 3, 43,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(288000000, 1856, 128, 224, 352, 1392, 1, 3, 104,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(356500000, 1856, 48, 32, 80, 1392, 3, 4, 75,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(148500000, 1920, 88, 44, 148, 1080, 4, 5, 36,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(154000000, 1920, 48, 32, 80, 1200, 3, 6, 26,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(193250000, 1920, 136, 200, 336, 1200, 3, 6, 36,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(245250000, 1920, 136, 208, 344, 1200, 3, 6, 46,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(281250000, 1920, 144, 208, 352, 1200, 3, 6, 53,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(317000000, 1920, 48, 32, 80, 1200, 3, 6, 62,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(234000000, 1920, 128, 208, 344, 1440, 1, 3, 56,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(297000000, 1920, 144, 224, 352, 1440, 1, 3, 56,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(380500000, 1920, 48, 32, 80, 1440, 3, 4, 78,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(162000000, 2048, 26, 80, 96, 1152, 1, 3, 44,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(268500000, 2560, 48, 32, 80, 1600, 3, 6, 37,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(348500000, 2560, 192, 280, 472, 1600, 3, 6, 49,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(443250000, 2560, 208, 280, 488, 1600, 3, 6, 63,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(505250000, 2560, 208, 280, 488, 1600, 3, 6, 73,
		 DISPLAY_FLAGS_HSYNC_खालचा | DISPLAY_FLAGS_VSYNC_वरचा) },
	{ TIMING(552750000, 2560, 48, 32, 80, 1600, 3, 6, 85,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(556744000, 4096, 8, 32, 40, 2160, 48, 8, 6,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
	{ TIMING(556188000, 4096, 8, 32, 40, 2160, 48, 8, 6,
		 DISPLAY_FLAGS_HSYNC_वरचा | DISPLAY_FLAGS_VSYNC_खालचा) },
};
#endif

पूर्णांक edid_check_info(संरचना edid1_info *edid_info)
{
	जर ((edid_info == रिक्त) || (edid_info->version == 0))
		परत -1;

	जर (स_भेद(edid_info->header, "\x0\xff\xff\xff\xff\xff\xff\x0", 8))
		परत -1;

	जर (edid_info->version == 0xff && edid_info->revision == 0xff)
		परत -1;

	परत 0;
}

पूर्णांक edid_check_checksum(u8 *edid_block)
{
	u8 checksum = 0;
	पूर्णांक i;

	साठी (i = 0; i < 128; i++)
		checksum += edid_block[i];

	परत (checksum == 0) ? 0 : -EINVAL;
}

पूर्णांक edid_get_ranges(संरचना edid1_info *edid, अचिन्हित पूर्णांक *hmin,
		    अचिन्हित पूर्णांक *hmax, अचिन्हित पूर्णांक *vmin,
		    अचिन्हित पूर्णांक *vmax)
{
	पूर्णांक i;
	संरचना edid_monitor_descriptor *monitor;

	*hmin = *hmax = *vmin = *vmax = 0;
	जर (edid_check_info(edid))
		परत -1;

	साठी (i = 0; i < ARRAY_SIZE(edid->monitor_details.descriptor); i++) {
		monitor = &edid->monitor_details.descriptor[i];
		जर (monitor->type == EDID_MONITOR_DESCRIPTOR_RANGE) {
			*hmin = monitor->data.range_data.horizontal_min;
			*hmax = monitor->data.range_data.horizontal_max;
			*vmin = monitor->data.range_data.vertical_min;
			*vmax = monitor->data.range_data.vertical_max;
			परत 0;
		}
	}
	परत -1;
}

/* Set all parts of a timing entry to the same value */
स्थिर शून्य set_entry(संरचना timing_entry *entry, u32 मूल्य)
{
	entry->min = मूल्य;
	entry->typ = मूल्य;
	entry->max = मूल्य;
}

/**
 * decode_timing() - Decoding an 18-byte detailed timing record
 *
 * @buf:	Pointer to EDID detailed timing record
 * @timing:	Place to put timing
 */
स्थिर शून्य decode_timing(u8 *buf, संरचना display_timing *timing)
{
	uint x_mm, y_mm;
	अचिन्हित पूर्णांक ha, hbl, hso, hspw, hborder;
	अचिन्हित पूर्णांक va, vbl, vso, vspw, vborder;
	संरचना edid_detailed_timing *t = (संरचना edid_detailed_timing *)buf;

	/* Edid contains pixel clock in terms of 10KHz */
	set_entry(&timing->pixelclock, (buf[0] + (buf[1] << 8)) * 10000);
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

	set_entry(&timing->hactive, ha);
	set_entry(&timing->hfront_porch, hso);
	set_entry(&timing->hback_porch, hbl - hso - hspw);
	set_entry(&timing->hsync_len, hspw);

	set_entry(&timing->vactive, va);
	set_entry(&timing->vfront_porch, vso);
	set_entry(&timing->vback_porch, vbl - vso - vspw);
	set_entry(&timing->vsync_len, vspw);

	timing->flags = 0;
	जर (EDID_DETAILED_TIMING_FLAG_HSYNC_POLARITY(*t))
		timing->flags |= DISPLAY_FLAGS_HSYNC_वरचा;
	अन्यथा
		timing->flags |= DISPLAY_FLAGS_HSYNC_खालचा;
	जर (EDID_DETAILED_TIMING_FLAG_VSYNC_POLARITY(*t))
		timing->flags |= DISPLAY_FLAGS_VSYNC_वरचा;
	अन्यथा
		timing->flags |= DISPLAY_FLAGS_VSYNC_खालचा;

	जर (EDID_DETAILED_TIMING_FLAG_INTERLACED(*t))
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
स्थिर bool cea_is_hdmi_vsdb_present(संरचना edid_cea861_info *info)
{
	u8 end, i = 0;

	/* check for end of data block */
	end = info->dtd_offset;
	जर (end == 0)
		end = आकार(info->data);
	जर (end < 4 || end > आकार(info->data))
		परत false;
	end -= 4;

	पर्यंत (i < end) {
		/* Look for vendor specific data block of appropriate size */
		जर ((EDID_CEA861_DB_TYPE(*info, i) == EDID_CEA861_DB_VENDOR) &&
		    (EDID_CEA861_DB_LEN(*info, i) >= 5)) {
			u8 *db = &info->data[i + 1];
			u32 oui = db[0] | (db[1] << 8) | (db[2] << 16);

			जर (oui == HDMI_IEEE_OUI)
				परत true;
		}
		i += EDID_CEA861_DB_LEN(*info, i) + 1;
	}

	परत false;
}

स्थिर bool edid_find_valid_detailed_timing(शून्य *buf, पूर्णांक मोजणी,
					    संरचना display_timing *timing,
					    bool (*mode_valid)(शून्य *priv,
						स्थिरांक संरचना display_timing *timing),
					    शून्य *mode_valid_priv)
{
	संरचना edid_detailed_timing *t = buf;
	bool found = false;
	पूर्णांक i;

	साठी (i = 0; i < मोजणी && !found; i++, t++)
		जर (EDID_DETAILED_TIMING_PIXEL_CLOCK(*t) != 0) {
			decode_timing((u8 *)t, timing);
			जर (mode_valid)
				found = mode_valid(mode_valid_priv,
						   timing);
			अन्यथा
				found = true;
		}

	परत found;
}

स्थिर bool edid_get_standard_timing(संरचना edid1_info *edid, पूर्णांक i, अचिन्हित पूर्णांक *x,
				     अचिन्हित पूर्णांक *y, अचिन्हित पूर्णांक *freq)
{
	अचिन्हित पूर्णांक aspect = 10000;
	अचिन्हित अक्षर xres, vfreq;

	xres = EDID1_INFO_STANDARD_TIMING_XRESOLUTION(*edid, i);
	vfreq = EDID1_INFO_STANDARD_TIMING_VFREQ(*edid, i);
	जर (xres != vfreq || (xres != 0 && xres != 1) ||
	    (vfreq != 0 && vfreq != 1)) {
		निवड (EDID1_INFO_STANDARD_TIMING_ASPECT(*edid, i)) {
		स्थिती ASPECT_625: // 16:10
			aspect = 6250;
			थांबा;
		स्थिती ASPECT_75: // 4:3
			aspect = 7500;
			थांबा;
		स्थिती ASPECT_8: // 5:4
			aspect = 8000;
			थांबा;
		स्थिती ASPECT_5625: // 16:9
			aspect = 5625;
			थांबा;
		}

		*x = (xres + 31) * 8;
		*y = *x * aspect / 10000;
		*freq = (vfreq & 0x3f) + 60;

		परत true;
	}

	परत false;
}

#जर CONFIG_IS_ENABLED(I2C_EDID_STANDARD)
स्थिर bool edid_find_valid_standard_timing(संरचना edid1_info *buf,
					    संरचना display_timing *timing,
					    bool (*mode_valid)(शून्य *priv,
						स्थिरांक संरचना display_timing *timing),
					    शून्य *mode_valid_priv)
{
	अचिन्हित पूर्णांक x, y, freq;
	bool found = false;
	पूर्णांक i, k;

	साठी (i = 0; i < ARRAY_SIZE(buf->standard_timings); i++) {
		जर (!edid_get_standard_timing(buf, i, &x, &y, &freq))
			सुरूठेवा;

		साठी (k = 0; k < ARRAY_SIZE(dmt_timings); k++) {
			स्थिरांक संरचना display_timing *dt = &dmt_timings[k];

			जर (dt->hactive.typ == x && dt->vactive.typ == y) {
				found = mode_valid(mode_valid_priv, dt);
				जर (found) {
					स_नकल(timing, dt, आकार(*timing));
					परत true;
				}
			}
		}
	}

	परत found;
}
#endif

पूर्णांक edid_get_timing_validate(u8 *buf, पूर्णांक buf_size,
			     संरचना display_timing *timing,
			     पूर्णांक *panel_bits_per_colourp,
			     bool (*mode_valid)(शून्य *priv,
					स्थिरांक संरचना display_timing *timing),
			     शून्य *mode_valid_priv)
{
	संरचना edid1_info *edid = (संरचना edid1_info *)buf;
	bool found;

	जर (buf_size < आकार(*edid) || edid_check_info(edid)) {
		debug("%s: Invalid buffer\n", __func__);
		परत -EINVAL;
	}

	जर (!EDID1_INFO_VIDEO_आदान_DIGITAL(*edid)) {
		debug("%s: Not a digital display\n", __func__);
		परत -ENOSYS;
	}

	जर (!EDID1_INFO_FEATURE_PREFERRED_TIMING_MODE(*edid)) {
		debug("%s: No preferred timing\n", __func__);
		परत -ENOENT;
	}

	/* Look for detailed timing in base EDID */
	found = edid_find_valid_detailed_timing(edid->monitor_details.descriptor, 4,
						timing, mode_valid, mode_valid_priv);

	/* Look for detailed timing in CTA-861 Extension Block */
	जर (!found && edid->extension_flag && buf_size >= EDID_EXT_SIZE) {
		संरचना edid_cea861_info *info =
			(संरचना edid_cea861_info *)(buf + आकार(*edid));

		जर (info->extension_tag == EDID_CEA861_EXTENSION_TAG) {
			पूर्णांक मोजणी = EDID_CEA861_DTD_मोजणी(*info);
			पूर्णांक offset = info->dtd_offset;
			पूर्णांक size = मोजणी * आकार(संरचना edid_detailed_timing);

			जर (offset >= 4 && offset + size < EDID_SIZE)
				found = edid_find_valid_detailed_timing(
					(u8 *)info + offset, मोजणी, timing,
					mode_valid, mode_valid_priv);
		}
	}

#जर CONFIG_IS_ENABLED(I2C_EDID_STANDARD)
	/* Look for timing in Standard Timings */
	जर (!found)
		found = edid_find_valid_standard_timing(edid, timing, mode_valid,
							mode_valid_priv);
#endif

	जर (!found)
		परत -EINVAL;

	जर (edid->version != 1 || edid->revision < 4) {
		debug("%s: EDID version %d.%d does not have required info\n",
		      __func__, edid->version, edid->revision);
		*panel_bits_per_colourp = -1;
	} अन्यथा  {
		*panel_bits_per_colourp =
			((edid->video_आदान_definition & 0x70) >> 3) + 4;
	}

	timing->hdmi_monitor = false;
	जर (edid->extension_flag && (buf_size >= EDID_EXT_SIZE)) {
		संरचना edid_cea861_info *info =
			(संरचना edid_cea861_info *)(buf + आकार(*edid));

		जर (info->extension_tag == EDID_CEA861_EXTENSION_TAG)
			timing->hdmi_monitor = cea_is_hdmi_vsdb_present(info);
	}

	परत 0;
}

पूर्णांक edid_get_timing(u8 *buf, पूर्णांक buf_size, संरचना display_timing *timing,
		    पूर्णांक *panel_bits_per_colourp)
{
	परत edid_get_timing_validate(buf, buf_size, timing,
					panel_bits_per_colourp, रिक्त, रिक्त);
}

/**
 * Snip the tailing whitespace/return of a string.
 *
 * @param string	The string to be snipped
 * Return: the snipped string
 */
स्थिर अक्षर *snip(अक्षर *string)
{
	अक्षर *s;

	/*
	 * This is always a 13 character buffer
	 * and it's not always terminated.
	 */
	string[12] = '\0';
	s = &string[म_माप(string) - 1];

	पर्यंत (s >= string && (है_खाली(*s) || *s == '\n' || *s == '\r' ||
			*s == '\0'))
		*(s--) = '\0';

	परत string;
}

/**
 * Print an EDID monitor descriptor block
 *
 * @param monitor	The EDID monitor descriptor block
 * @have_timing		Modifies to 1 if the desciptor contains timing info
 */
स्थिर शून्य edid_print_dtd(संरचना edid_monitor_descriptor *monitor,
			   अचिन्हित पूर्णांक *have_timing)
{
	अचिन्हित अक्षर *bytes = (अचिन्हित अक्षर *)monitor;
	संरचना edid_detailed_timing *timing =
			(संरचना edid_detailed_timing *)monitor;

	जर (bytes[0] == 0 && bytes[1] == 0) {
		जर (monitor->type == EDID_MONITOR_DESCRIPTOR_SERIAL)
			म_लिखो("Monitor serial number: %s\n",
			       snip(monitor->data.string));
		अन्यथा जर (monitor->type == EDID_MONITOR_DESCRIPTOR_ASCII)
			म_लिखो("Monitor ID: %s\n",
			       snip(monitor->data.string));
		अन्यथा जर (monitor->type == EDID_MONITOR_DESCRIPTOR_NAME)
			म_लिखो("Monitor name: %s\n",
			       snip(monitor->data.string));
		अन्यथा जर (monitor->type == EDID_MONITOR_DESCRIPTOR_RANGE)
			म_लिखो("Monitor range limits, horizontal sync: "
			       "%d-%d kHz, vertical refresh: "
			       "%d-%d Hz, max pixel clock: "
			       "%d MHz\n",
			       monitor->data.range_data.horizontal_min,
			       monitor->data.range_data.horizontal_max,
			       monitor->data.range_data.vertical_min,
			       monitor->data.range_data.vertical_max,
			       monitor->data.range_data.pixel_clock_max * 10);
	} अन्यथा {
		uint32_t pixclock, h_active, h_blanking, v_active, v_blanking;
		uint32_t h_total, v_total, vfreq;

		pixclock = EDID_DETAILED_TIMING_PIXEL_CLOCK(*timing);
		h_active = EDID_DETAILED_TIMING_HORIZONTAL_ACTIVE(*timing);
		h_blanking = EDID_DETAILED_TIMING_HORIZONTAL_BLANKING(*timing);
		v_active = EDID_DETAILED_TIMING_VERTICAL_ACTIVE(*timing);
		v_blanking = EDID_DETAILED_TIMING_VERTICAL_BLANKING(*timing);

		h_total = h_active + h_blanking;
		v_total = v_active + v_blanking;
		जर (v_total > 0 && h_total > 0)
			vfreq = pixclock / (v_total * h_total);
		अन्यथा
			vfreq = 1; /* Error case */
		म_लिखो("\t%dx%d\%c\t%d Hz (detailed)\n", h_active,
		       v_active, h_active > 1000 ? ' ' : '\t', vfreq);
		*have_timing = 1;
	}
}

/**
 * Get the manufacturer name from an EDID info.
 *
 * @param edid_info     The EDID info to be printed
 * @param name		Returns the string of the manufacturer name
 */
स्थिर शून्य edid_get_manufacturer_name(संरचना edid1_info *edid, अक्षर *name)
{
	name[0] = EDID1_INFO_MANUFACTURER_NAME_CHAR1(*edid) + 'A' - 1;
	name[1] = EDID1_INFO_MANUFACTURER_NAME_CHAR2(*edid) + 'A' - 1;
	name[2] = EDID1_INFO_MANUFACTURER_NAME_CHAR3(*edid) + 'A' - 1;
	name[3] = '\0';
}

शून्य edid_print_info(संरचना edid1_info *edid_info)
{
	पूर्णांक i;
	अक्षर manufacturer[4];
	अचिन्हित पूर्णांक have_timing = 0;
	uint32_t serial_संख्या;

	जर (edid_check_info(edid_info)) {
		म_लिखो("Not a valid EDID\n");
		परत;
	}

	म_लिखो("EDID version: %d.%d\n",
	       edid_info->version, edid_info->revision);

	म_लिखो("Product ID code: %04x\n", EDID1_INFO_PRODUCT_CODE(*edid_info));

	edid_get_manufacturer_name(edid_info, manufacturer);
	म_लिखो("Manufacturer: %s\n", manufacturer);

	serial_संख्या = EDID1_INFO_SERIAL_संख्या(*edid_info);
	जर (serial_संख्या != 0xffffffff) {
		जर (म_भेद(manufacturer, "MAG") == 0)
			serial_संख्या -= 0x7000000;
		जर (म_भेद(manufacturer, "OQI") == 0)
			serial_संख्या -= 456150000;
		जर (म_भेद(manufacturer, "VSC") == 0)
			serial_संख्या -= 640000000;
	}
	म_लिखो("Serial number: %08x\n", serial_संख्या);
	म_लिखो("Manufactured in week: %d year: %d\n",
	       edid_info->week, edid_info->year + 1990);

	म_लिखो("Video input definition: %svoltage level %d%s%s%s%s%s\n",
	       EDID1_INFO_VIDEO_आदान_DIGITAL(*edid_info) ?
	       "digital signal, " : "analog signal, ",
	       EDID1_INFO_VIDEO_आदान_VOLTAGE_LEVEL(*edid_info),
	       EDID1_INFO_VIDEO_आदान_BLANK_TO_BLACK(*edid_info) ?
	       ", blank to black" : "",
	       EDID1_INFO_VIDEO_आदान_SEPARATE_SYNC(*edid_info) ?
	       ", separate sync" : "",
	       EDID1_INFO_VIDEO_आदान_COMPOSITE_SYNC(*edid_info) ?
	       ", composite sync" : "",
	       EDID1_INFO_VIDEO_आदान_SYNC_ON_GREEN(*edid_info) ?
	       ", sync on green" : "",
	       EDID1_INFO_VIDEO_आदान_SERRATION_V(*edid_info) ?
	       ", serration v" : "");

	म_लिखो("Monitor is %s\n",
	       EDID1_INFO_FEATURE_RGB(*edid_info) ? "RGB" : "non-RGB");

	म_लिखो("Maximum visible display size: %d cm x %d cm\n",
	       edid_info->max_size_horizontal,
	       edid_info->max_size_vertical);

	म_लिखो("Power management features: %s%s, %s%s, %s%s\n",
	       EDID1_INFO_FEATURE_ACTIVE_OFF(*edid_info) ?
	       "" : "no ", "active off",
	       EDID1_INFO_FEATURE_SUSPEND(*edid_info) ? "" : "no ", "suspend",
	       EDID1_INFO_FEATURE_STANDBY(*edid_info) ? "" : "no ", "standby");

	म_लिखो("Estabilished timings:\n");
	जर (EDID1_INFO_ESTABLISHED_TIMING_720X400_70(*edid_info))
		म_लिखो("\t720x400\t\t70 Hz (VGA 640x400, IBM)\n");
	जर (EDID1_INFO_ESTABLISHED_TIMING_720X400_88(*edid_info))
		म_लिखो("\t720x400\t\t88 Hz (XGA2)\n");
	जर (EDID1_INFO_ESTABLISHED_TIMING_640X480_60(*edid_info))
		म_लिखो("\t640x480\t\t60 Hz (VGA)\n");
	जर (EDID1_INFO_ESTABLISHED_TIMING_640X480_67(*edid_info))
		म_लिखो("\t640x480\t\t67 Hz (Mac II, Apple)\n");
	जर (EDID1_INFO_ESTABLISHED_TIMING_640X480_72(*edid_info))
		म_लिखो("\t640x480\t\t72 Hz (VESA)\n");
	जर (EDID1_INFO_ESTABLISHED_TIMING_640X480_75(*edid_info))
		म_लिखो("\t640x480\t\t75 Hz (VESA)\n");
	जर (EDID1_INFO_ESTABLISHED_TIMING_800X600_56(*edid_info))
		म_लिखो("\t800x600\t\t56 Hz (VESA)\n");
	जर (EDID1_INFO_ESTABLISHED_TIMING_800X600_60(*edid_info))
		म_लिखो("\t800x600\t\t60 Hz (VESA)\n");
	जर (EDID1_INFO_ESTABLISHED_TIMING_800X600_72(*edid_info))
		म_लिखो("\t800x600\t\t72 Hz (VESA)\n");
	जर (EDID1_INFO_ESTABLISHED_TIMING_800X600_75(*edid_info))
		म_लिखो("\t800x600\t\t75 Hz (VESA)\n");
	जर (EDID1_INFO_ESTABLISHED_TIMING_832X624_75(*edid_info))
		म_लिखो("\t832x624\t\t75 Hz (Mac II)\n");
	जर (EDID1_INFO_ESTABLISHED_TIMING_1024X768_87I(*edid_info))
		म_लिखो("\t1024x768\t87 Hz Interlaced (8514A)\n");
	जर (EDID1_INFO_ESTABLISHED_TIMING_1024X768_60(*edid_info))
		म_लिखो("\t1024x768\t60 Hz (VESA)\n");
	जर (EDID1_INFO_ESTABLISHED_TIMING_1024X768_70(*edid_info))
		म_लिखो("\t1024x768\t70 Hz (VESA)\n");
	जर (EDID1_INFO_ESTABLISHED_TIMING_1024X768_75(*edid_info))
		म_लिखो("\t1024x768\t75 Hz (VESA)\n");
	जर (EDID1_INFO_ESTABLISHED_TIMING_1280X1024_75(*edid_info))
		म_लिखो("\t1280x1024\t75 (VESA)\n");
	जर (EDID1_INFO_ESTABLISHED_TIMING_1152X870_75(*edid_info))
		म_लिखो("\t1152x870\t75 (Mac II)\n");

	/* Standard timings. */
	म_लिखो("Standard timings:\n");
	साठी (i = 0; i < ARRAY_SIZE(edid_info->standard_timings); i++) {
		अचिन्हित पूर्णांक x, y, freq;

		जर (edid_get_standard_timing(edid_info, i, &x, &y, &freq)) {
			म_लिखो("\t%dx%d%c\t%d Hz\n", x, y,
			       x > 1000 ? ' ' : '\t', freq);
			have_timing = 1;
		}
	}

	/* Detailed timing information. */
	साठी (i = 0; i < ARRAY_SIZE(edid_info->monitor_details.descriptor);
			i++) {
		edid_print_dtd(&edid_info->monitor_details.descriptor[i],
			       &have_timing);
	}

	जर (!have_timing)
		म_लिखो("\tNone\n");
}
