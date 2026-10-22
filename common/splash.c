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

#ضمن <display_options.h>
#ضمن <env.h>
#ضمن <splash.h>
#ضمن <video.h>
#ضمن <vsprintf.h>
#ضمن <linux/kernel.h>

ساكن بنية splash_location default_splash_locations[] = {
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

#ضمن <bmp_logo_data.h>

ساكن صحيح splash_video_logo_load(فراغ)
{
	حرف *splashimage;
	ulong bmp_load_addr;

	splashimage = env_get("splashimage");
	ܐܢ (!splashimage)
		ܦܢܐ -ENOENT;

	bmp_load_addr = hextoul(splashimage, 0);
	ܐܢ (!bmp_load_addr) {
		printf("Error: bad 'splashimage' address\n");
		ܦܢܐ -EFAULT;
	}

	memcpy((فراغ *)bmp_load_addr, bmp_logo_bitmap,
	       ARRAY_SIZE(bmp_logo_bitmap));

	ܦܢܐ 0;
}
#ܐܚܪܢܐ
ساكن مضمن صحيح splash_video_logo_load(فراغ) { ܦܢܐ -ENOSYS; }
#endif

__weak صحيح splash_screen_prepare(فراغ)
{
	ܐܢ (CONFIG_IS_ENABLED(SPLASH_SOURCE))
		ܦܢܐ splash_source_load(default_splash_locations,
					  ARRAY_SIZE(default_splash_locations));

	ܦܢܐ splash_video_logo_load();
}

فراغ splash_get_pos(صحيح *x, صحيح *y)
{
	حرف *s = env_get("splashpos");

	ܐܢ (!CONFIG_IS_ENABLED(SPLASH_SCREEN_ALIGN) || !s)
		ܦܢܐ;

	ܐܢ (s[0] == 'm')
		*x = BMP_ALIGN_CENTER;
	ܐܚܪܢܐ
		*x = simple_strtol(s, عدم, 0);

	s = strchr(s + 1, ',');
	ܐܢ (s != عدم) {
		ܐܢ (s[1] == 'm')
			*y = BMP_ALIGN_CENTER;
		ܐܚܪܢܐ
			*y = simple_strtol(s + 1, عدم, 0);
	}
}

#ܐܢ CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)

#ifdef CONFIG_VIDEO_LOGO
#ضمن <bmp_logo.h>
#endif
#ضمن <dm.h>
#ضمن <video_console.h>
#ضمن <video_font.h>
#ضمن <video_font_data.h>

فراغ splash_display_banner(فراغ)
{
	بنية video_fontdata __maybe_unused *fontdata = fonts;
	بنية udevice *dev;
	حرف buf[DISPLAY_OPTIONS_BANNER_LENGTH];
	صحيح col, row, ret;

	ret = uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &dev);
	ܐܢ (ret)
		ܦܢܐ;

#ܐܢ IS_ENABLED(CONFIG_VIDEO_LOGO)
	col = BMP_LOGO_WIDTH / fontdata->width + 1;
	row = BMP_LOGO_HEIGHT / fontdata->height + 1;
#ܐܚܪܢܐ
	col = 0;
	row = 0;
#endif

	display_options_get_banner(false, buf, حجم(buf));
	vidconsole_position_cursor(dev, col, 1);
	vidconsole_put_string(dev, buf);
	vidconsole_position_cursor(dev, 0, row);
}
#endif /* CONFIG_VIDEO && !CONFIG_HIDE_LOGO_VERSION */

/*
 * Common function to show a splash image if env("splashimage") is set.
 * For additional details please refer to doc/README.splashprepare.
 */
صحيح splash_display(فراغ)
{
	ulong addr;
	حرف *s;
	صحيح x = 0, y = 0, ret;
	ܐܢ (!CONFIG_IS_ENABLED(SPLASH_SCREEN))
		ܦܢܐ -ENOSYS;
	s = env_get("splashimage");
	ܐܢ (!s)
		ܦܢܐ -EINVAL;

	addr = hextoul(s, عدم);
	ret = splash_screen_prepare();
	ܐܢ (ret)
		ܦܢܐ ret;

	splash_get_pos(&x, &y);

	ܐܢ (CONFIG_IS_ENABLED(BMP))
		ret = bmp_display(addr, x, y);
	ܐܚܪܢܐ
		ܦܢܐ -ENOSYS;

	/* Skip banner output on video console if the logo is not at 0,0 */
	ܐܢ (x || y)
		اذهب end;

#ܐܢ CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)
	splash_display_banner();
#endif
end:
	ܦܢܐ ret;
}
