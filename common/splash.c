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
		.جابه_جایی = 0x0,
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

#اگر_تعریف_شده CONFIG_VIDEO_LOGO

#ضمن <bmp_logo_data.h>

ساكن صحيح splash_video_logo_بارگیری(فراغ)
{
	حرف *splashimage;
	ulong bmp_بارگیری_addr;

	splashimage = env_get("splashimage");
	إذا (!splashimage)
		أرجع -ENOENT;

	bmp_بارگیری_addr = hextoul(splashimage, 0);
	إذا (!bmp_بارگیری_addr) {
		چاپ_کن("Error: bad 'splashimage' address\n");
		أرجع -EFAULT;
	}

	کپی_حافظه((فراغ *)bmp_بارگیری_addr, bmp_logo_bitmap,
	       ARRAY_اندازه(bmp_logo_bitmap));

	أرجع 0;
}
#وإلا
ساكن مضمن صحيح splash_video_logo_بارگیری(فراغ) { أرجع -ENOSYS; }
#پایان_اگر

__weak صحيح splash_screen_prepare(فراغ)
{
	إذا (CONFIG_IS_ENABLED(SPLASH_منبع))
		أرجع splash_منبع_بارگیری(default_splash_locations,
					  ARRAY_اندازه(default_splash_locations));

	أرجع splash_video_logo_بارگیری();
}

فراغ splash_get_pos(صحيح *x, صحيح *y)
{
	حرف *s = env_get("splashpos");

	إذا (!CONFIG_IS_ENABLED(SPLASH_SCREEN_ALIGN) || !s)
		أرجع;

	إذا (s[0] == 'm')
		*x = BMP_ALIGN_CENTER;
	وإلا
		*x = simple_strtol(s, عدم, 0);

	s = strchr(s + 1, ',');
	إذا (s != عدم) {
		إذا (s[1] == 'm')
			*y = BMP_ALIGN_CENTER;
		وإلا
			*y = simple_strtol(s + 1, عدم, 0);
	}
}

#إذا CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_نسخه)

#اگر_تعریف_شده CONFIG_VIDEO_LOGO
#ضمن <bmp_logo.h>
#پایان_اگر
#ضمن <dm.h>
#ضمن <video_console.h>
#ضمن <video_font.h>
#ضمن <video_font_data.h>

فراغ splash_display_banner(فراغ)
{
	بنية video_fontdata __maybe_unused *fontdata = fonts;
	بنية udevice *dev;
	حرف buf[DISPLAY_OPTIONS_BANNER_درازی];
	صحيح ستون, ردیف, ret;

	ret = uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &dev);
	إذا (ret)
		أرجع;

#إذا IS_ENABLED(CONFIG_VIDEO_LOGO)
	ستون = BMP_LOGO_عرض / fontdata->عرض + 1;
	ردیف = BMP_LOGO_ارتفاع / fontdata->ارتفاع + 1;
#وإلا
	ستون = 0;
	ردیف = 0;
#پایان_اگر

	display_options_get_banner(false, buf, حجم(buf));
	vidconsole_position_cursor(dev, ستون, 1);
	vidconsole_put_string(dev, buf);
	vidconsole_position_cursor(dev, 0, ردیف);
}
#پایان_اگر /* CONFIG_VIDEO && !CONFIG_HIDE_LOGO_VERSION */

/*
 * Common function to show a splash image if env("splashimage") is set.
 * For additional details please refer to doc/README.splashprepare.
 */
صحيح splash_display(فراغ)
{
	ulong addr;
	حرف *s;
	صحيح x = 0, y = 0, ret;
	إذا (!CONFIG_IS_ENABLED(SPLASH_SCREEN))
		أرجع -ENOSYS;
	s = env_get("splashimage");
	إذا (!s)
		أرجع -EINVAL;

	addr = hextoul(s, عدم);
	ret = splash_screen_prepare();
	إذا (ret)
		أرجع ret;

	splash_get_pos(&x, &y);

	إذا (CONFIG_IS_ENABLED(BMP))
		ret = bmp_display(addr, x, y);
	وإلا
		أرجع -ENOSYS;

	/* Skip banner output on video console if the logo is not at 0,0 */
	إذا (x || y)
		اذهب انتها_2;

#إذا CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_نسخه)
	splash_display_banner();
#پایان_اگر
انتها_2:
	أرجع ret;
}
