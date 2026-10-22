/*
 * Copyright (C) 2013, Boundary Devices <info@boundarydevices.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., http://www.fsf.org/about/contact/
 *
 */

#অন্তর্ভাব <display_options.h>
#অন্তর্ভাব <env.h>
#অন্তর্ভাব <splash.h>
#অন্তর্ভাব <video.h>
#অন্তর্ভাব <vsprintf.h>
#অন্তর্ভাব <linux/kernel.h>

স্থির কাঠামো splash_location default_splash_locations[] = {
	{
		.name = "sf",
		.storage = SPLASH_STORAGE_SF,
		.flags = SPLASH_STORAGE_RAW,
		.offset = 0x0,
	},
	{
		.name = "mmc_fs",
		.storage = SPLASH_STORAGE_MMC,
		.flags = SPLASH_STORAGE_FS,
		.devpart = "0:1",
	},
	{
		.name = "mmc_raw",
		.storage = SPLASH_STORAGE_MMC,
		.flags = SPLASH_STORAGE_RAW,
		.devpart = "0:1",
	},
	{
		.name = "usb_fs",
		.storage = SPLASH_STORAGE_USB,
		.flags = SPLASH_STORAGE_FS,
		.devpart = "0:1",
	},
	{
		.name = "sata_fs",
		.storage = SPLASH_STORAGE_SATA,
		.flags = SPLASH_STORAGE_FS,
		.devpart = "0:1",
	},
};

#ifdef CONFIG_VIDEO_LOGO

#অন্তর্ভাব <bmp_logo_data.h>

স্থির পূর্ণসংখ্যা splash_video_logo_load(শূন্য)
{
	অক্ষর *splashimage;
	ulong bmp_load_addr;

	splashimage = env_get("splashimage");
	যদি (!splashimage)
		ফেরত -ENOENT;

	bmp_load_addr = hextoul(splashimage, 0);
	যদি (!bmp_load_addr) {
		printf("Error: bad 'splashimage' address\n");
		ফেরত -EFAULT;
	}

	memcpy((শূন্য *)bmp_load_addr, bmp_logo_bitmap,
	       ARRAY_আকার(bmp_logo_bitmap));

	ফেরত 0;
}
#অন্যথা
স্থির অন্তঃরেখ পূর্ণসংখ্যা splash_video_logo_load(শূন্য) { ফেরত -ENOSYS; }
#endif

__weak পূর্ণসংখ্যা splash_screen_prepare(শূন্য)
{
	যদি (CONFIG_IS_ENABLED(SPLASH_SOURCE))
		ফেরত splash_source_load(default_splash_locations,
					  ARRAY_আকার(default_splash_locations));

	ফেরত splash_video_logo_load();
}

শূন্য splash_get_pos(পূর্ণসংখ্যা *x, পূর্ণসংখ্যা *y)
{
	অক্ষর *s = env_get("splashpos");

	যদি (!CONFIG_IS_ENABLED(SPLASH_SCREEN_ALIGN) || !s)
		ফেরত;

	যদি (s[0] == 'm')
		*x = BMP_ALIGN_CENTER;
	অন্যথা
		*x = simple_strtol(s, ফাঁকা, 0);

	s = strchr(s + 1, ',');
	যদি (s != ফাঁকা) {
		যদি (s[1] == 'm')
			*y = BMP_ALIGN_CENTER;
		অন্যথা
			*y = simple_strtol(s + 1, ফাঁকা, 0);
	}
}

#যদি CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)

#ifdef CONFIG_VIDEO_LOGO
#অন্তর্ভাব <bmp_logo.h>
#endif
#অন্তর্ভাব <dm.h>
#অন্তর্ভাব <video_console.h>
#অন্তর্ভাব <video_font.h>
#অন্তর্ভাব <video_font_data.h>

শূন্য splash_display_banner(শূন্য)
{
	কাঠামো video_fontdata __maybe_unused *fontdata = fonts;
	কাঠামো udevice *dev;
	অক্ষর buf[DISPLAY_OPTIONS_BANNER_দৈর্ঘ্য];
	পূর্ণসংখ্যা col, row, ret;

	ret = uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &dev);
	যদি (ret)
		ফেরত;

#যদি IS_ENABLED(CONFIG_VIDEO_LOGO)
	col = BMP_LOGO_WIDTH / fontdata->width + 1;
	row = BMP_LOGO_HEIGHT / fontdata->height + 1;
#অন্যথা
	col = 0;
	row = 0;
#endif

	display_options_get_banner(false, buf, আকার(buf));
	vidconsole_position_cursor(dev, col, 1);
	vidconsole_put_string(dev, buf);
	vidconsole_position_cursor(dev, 0, row);
}
#endif /* CONFIG_VIDEO && !CONFIG_HIDE_LOGO_VERSION */

/*
 * Common function to show a splash image if env("splashimage") is set.
 * For additional details please refer to doc/README.splashprepare.
 */
পূর্ণসংখ্যা splash_display(শূন্য)
{
	ulong addr;
	অক্ষর *s;
	পূর্ণসংখ্যা x = 0, y = 0, ret;
	যদি (!CONFIG_IS_ENABLED(SPLASH_SCREEN))
		ফেরত -ENOSYS;
	s = env_get("splashimage");
	যদি (!s)
		ফেরত -EINVAL;

	addr = hextoul(s, ফাঁকা);
	ret = splash_screen_prepare();
	যদি (ret)
		ফেরত ret;

	splash_get_pos(&x, &y);

	যদি (CONFIG_IS_ENABLED(BMP))
		ret = bmp_display(addr, x, y);
	অন্যথা
		ফেরত -ENOSYS;

	/* Skip banner output on video console if the logo is not at 0,0 */
	যদি (x || y)
		যাও end;

#যদি CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)
	splash_display_banner();
#endif
end:
	ফেরত ret;
}
