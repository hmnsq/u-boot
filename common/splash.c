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

ساکن ساخت splash_location default_splash_locations[] = {
	{
		.name = "sf",
		.storage = SPLASH_STORAGE_SF,
		.flags = SPLASH_STORAGE_RAW,
		.انحراف = 0x0,
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

#اگر_تعریف_شدہ CONFIG_VIDEO_LOGO

#شامل <bmp_logo_data.h>

ساکن صحیح splash_video_logo_بھرو(خالی)
{
	حرف *splashimage;
	ulong bmp_بھرو_addr;

	splashimage = env_get("splashimage");
	اگر (!splashimage)
		واپس -ENOENT;

	bmp_بھرو_addr = hextoul(splashimage, 0);
	اگر (!bmp_بھرو_addr) {
		لکھو("Error: bad 'splashimage' address\n");
		واپس -EFAULT;
	}

	حافظہ_نقل((خالی *)bmp_بھرو_addr, bmp_logo_bitmap,
	       ARRAY_سائز(bmp_logo_bitmap));

	واپس 0;
}
#ورنہ
ساکن اندرونی صحیح splash_video_logo_بھرو(خالی) { واپس -ENOSYS; }
#اگر_ختم

__weak صحیح splash_screen_prepare(خالی)
{
	اگر (CONFIG_IS_ENABLED(SPLASH_ماخذ))
		واپس splash_ماخذ_بھرو(default_splash_locations,
					  ARRAY_سائز(default_splash_locations));

	واپس splash_video_logo_بھرو();
}

خالی splash_get_pos(صحیح *x, صحیح *y)
{
	حرف *s = env_get("splashpos");

	اگر (!CONFIG_IS_ENABLED(SPLASH_SCREEN_ALIGN) || !s)
		واپس;

	اگر (s[0] == 'm')
		*x = BMP_ALIGN_CENTER;
	ورنہ
		*x = simple_strtol(s, تہی, 0);

	s = strchr(s + 1, ',');
	اگر (s != تہی) {
		اگر (s[1] == 'm')
			*y = BMP_ALIGN_CENTER;
		ورنہ
			*y = simple_strtol(s + 1, تہی, 0);
	}
}

#اگر CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_نسخہ)

#اگر_تعریف_شدہ CONFIG_VIDEO_LOGO
#شامل <bmp_logo.h>
#اگر_ختم
#شامل <dm.h>
#شامل <video_console.h>
#شامل <video_font.h>
#شامل <video_font_data.h>

خالی splash_display_banner(خالی)
{
	ساخت video_fontdata __maybe_unused *fontdata = fonts;
	ساخت udevice *dev;
	حرف buf[DISPLAY_OPTIONS_BANNER_لمبائی];
	صحیح ستون, صف, ret;

	ret = uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &dev);
	اگر (ret)
		واپس;

#اگر IS_ENABLED(CONFIG_VIDEO_LOGO)
	ستون = BMP_LOGO_چوڑائی / fontdata->چوڑائی + 1;
	صف = BMP_LOGO_اونچائی / fontdata->اونچائی + 1;
#ورنہ
	ستون = 0;
	صف = 0;
#اگر_ختم

	display_options_get_banner(false, buf, حجم(buf));
	vidconsole_position_cursor(dev, ستون, 1);
	vidconsole_put_string(dev, buf);
	vidconsole_position_cursor(dev, 0, صف);
}
#اگر_ختم /* CONFIG_VIDEO && !CONFIG_HIDE_LOGO_VERSION */

/*
 * Common function to show a splash image if env("splashimage") is set.
 * For additional details please refer to doc/README.splashprepare.
 */
صحیح splash_display(خالی)
{
	ulong addr;
	حرف *s;
	صحیح x = 0, y = 0, ret;
	اگر (!CONFIG_IS_ENABLED(SPLASH_SCREEN))
		واپس -ENOSYS;
	s = env_get("splashimage");
	اگر (!s)
		واپس -EINVAL;

	addr = hextoul(s, تہی);
	ret = splash_screen_prepare();
	اگر (ret)
		واپس ret;

	splash_get_pos(&x, &y);

	اگر (CONFIG_IS_ENABLED(BMP))
		ret = bmp_display(addr, x, y);
	ورنہ
		واپس -ENOSYS;

	/* Skip banner output on video console if the logo is not at 0,0 */
	اگر (x || y)
		جاؤ انتہا_2;

#اگر CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_نسخہ)
	splash_display_banner();
#اگر_ختم
انتہا_2:
	واپس ret;
}
