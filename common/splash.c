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

#شامل <display_options.h>
#شامل <env.h>
#شامل <splash.h>
#شامل <video.h>
#شامل <vsprintf.h>
#شامل <linux/kernel.h>

ایستا ساختار splash_location default_splash_locations[] = {
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

#شامل <bmp_logo_data.h>

ایستا صحیح splash_video_logo_load(تهی)
{
	نویسه *splashimage;
	ulong bmp_load_addr;

	splashimage = env_get("splashimage");
	اگر (!splashimage)
		بازگشت -ENOENT;

	bmp_load_addr = hextoul(splashimage, 0);
	اگر (!bmp_load_addr) {
		printf("Error: bad 'splashimage' address\n");
		بازگشت -EFAULT;
	}

	memcpy((تهی *)bmp_load_addr, bmp_logo_bitmap,
	       ARRAY_SIZE(bmp_logo_bitmap));

	بازگشت 0;
}
#وگرنه
ایستا درونخطی صحیح splash_video_logo_load(تهی) { بازگشت -ENOSYS; }
#endif

__weak صحیح splash_screen_prepare(تهی)
{
	اگر (CONFIG_IS_ENABLED(SPLASH_SOURCE))
		بازگشت splash_source_load(default_splash_locations,
					  ARRAY_SIZE(default_splash_locations));

	بازگشت splash_video_logo_load();
}

تهی splash_get_pos(صحیح *x, صحیح *y)
{
	نویسه *s = env_get("splashpos");

	اگر (!CONFIG_IS_ENABLED(SPLASH_SCREEN_ALIGN) || !s)
		بازگشت;

	اگر (s[0] == 'm')
		*x = BMP_ALIGN_CENTER;
	وگرنه
		*x = simple_strtol(s, پوچ, 0);

	s = strchr(s + 1, ',');
	اگر (s != پوچ) {
		اگر (s[1] == 'm')
			*y = BMP_ALIGN_CENTER;
		وگرنه
			*y = simple_strtol(s + 1, پوچ, 0);
	}
}

#اگر CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)

#ifdef CONFIG_VIDEO_LOGO
#شامل <bmp_logo.h>
#endif
#شامل <dm.h>
#شامل <video_console.h>
#شامل <video_font.h>
#شامل <video_font_data.h>

تهی splash_display_banner(تهی)
{
	ساختار video_fontdata __maybe_unused *fontdata = fonts;
	ساختار udevice *dev;
	نویسه buf[DISPLAY_OPTIONS_BANNER_LENGTH];
	صحیح col, row, ret;

	ret = uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &dev);
	اگر (ret)
		بازگشت;

#اگر IS_ENABLED(CONFIG_VIDEO_LOGO)
	col = BMP_LOGO_WIDTH / fontdata->width + 1;
	row = BMP_LOGO_HEIGHT / fontdata->height + 1;
#وگرنه
	col = 0;
	row = 0;
#endif

	display_options_get_banner(false, buf, اندازه(buf));
	vidconsole_position_cursor(dev, col, 1);
	vidconsole_put_string(dev, buf);
	vidconsole_position_cursor(dev, 0, row);
}
#endif /* CONFIG_VIDEO && !CONFIG_HIDE_LOGO_VERSION */

/*
 * Common function to show a splash image if env("splashimage") is set.
 * For additional details please refer to doc/README.splashprepare.
 */
صحیح splash_display(تهی)
{
	ulong addr;
	نویسه *s;
	صحیح x = 0, y = 0, ret;
	اگر (!CONFIG_IS_ENABLED(SPLASH_SCREEN))
		بازگشت -ENOSYS;
	s = env_get("splashimage");
	اگر (!s)
		بازگشت -EINVAL;

	addr = hextoul(s, پوچ);
	ret = splash_screen_prepare();
	اگر (ret)
		بازگشت ret;

	splash_get_pos(&x, &y);

	اگر (CONFIG_IS_ENABLED(BMP))
		ret = bmp_display(addr, x, y);
	وگرنه
		بازگشت -ENOSYS;

	/* Skip banner output on video console if the logo is not at 0,0 */
	اگر (x || y)
		برو end;

#اگر CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)
	splash_display_banner();
#endif
end:
	بازگشت ret;
}
