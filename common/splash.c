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

#ਅਨ੍ਤਰ੍ਭਾਵ <display_options.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <splash.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <video.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <vsprintf.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/kernel.h>

ਸਥਿਰ ਬਣਤਰ splash_location default_splash_locations[] = {
	{
		.name = "sf",
		.storage = SPLASH_STORAGE_SF,
		.flags = SPLASH_STORAGE_RAW,
		.ਵਿੱਥ = 0x0,
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

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_VIDEO_LOGO

#ਅਨ੍ਤਰ੍ਭਾਵ <bmp_logo_data.h>

ਸਥਿਰ ਪੂਰਨ splash_video_logo_ਲੱਦੋ(ਖਾਲੀ)
{
	ਅੱਖਰ *splashimage;
	ulong bmp_ਲੱਦੋ_addr;

	splashimage = env_get("splashimage");
	ਜੇ (!splashimage)
		ਵਾਪਸ -ENOENT;

	bmp_ਲੱਦੋ_addr = hextoul(splashimage, 0);
	ਜੇ (!bmp_ਲੱਦੋ_addr) {
		ਲਿਖੋ("Error: bad 'splashimage' address\n");
		ਵਾਪਸ -EFAULT;
	}

	ਸਮ੍ਰਿਤੀ_ਨਕਲ((ਖਾਲੀ *)bmp_ਲੱਦੋ_addr, bmp_logo_bitmap,
	       ARRAY_ਮਾਪ(bmp_logo_bitmap));

	ਵਾਪਸ 0;
}
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ splash_video_logo_ਲੱਦੋ(ਖਾਲੀ) { ਵਾਪਸ -ENOSYS; }
#ਜੇ_ਅੰਤ

__weak ਪੂਰਨ splash_screen_prepare(ਖਾਲੀ)
{
	ਜੇ (CONFIG_IS_ENABLED(SPLASH_ਸਰੋਤ))
		ਵਾਪਸ splash_ਸਰੋਤ_ਲੱਦੋ(default_splash_locations,
					  ARRAY_ਮਾਪ(default_splash_locations));

	ਵਾਪਸ splash_video_logo_ਲੱਦੋ();
}

ਖਾਲੀ splash_get_pos(ਪੂਰਨ *x, ਪੂਰਨ *y)
{
	ਅੱਖਰ *s = env_get("splashpos");

	ਜੇ (!CONFIG_IS_ENABLED(SPLASH_SCREEN_ALIGN) || !s)
		ਵਾਪਸ;

	ਜੇ (s[0] == 'm')
		*x = BMP_ALIGN_CENTER;
	ਨਹੀਂਤਾਂ
		*x = simple_strtol(s, ਨਿਰਾ, 0);

	s = strchr(s + 1, ',');
	ਜੇ (s != ਨਿਰਾ) {
		ਜੇ (s[1] == 'm')
			*y = BMP_ALIGN_CENTER;
		ਨਹੀਂਤਾਂ
			*y = simple_strtol(s + 1, ਨਿਰਾ, 0);
	}
}

#ਜੇ CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_ਸੰਸਕਰਣ)

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_VIDEO_LOGO
#ਅਨ੍ਤਰ੍ਭਾਵ <bmp_logo.h>
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <dm.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <video_console.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <video_font.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <video_font_data.h>

ਖਾਲੀ splash_display_banner(ਖਾਲੀ)
{
	ਬਣਤਰ video_fontdata __maybe_unused *fontdata = fonts;
	ਬਣਤਰ udevice *dev;
	ਅੱਖਰ buf[DISPLAY_OPTIONS_BANNER_ਲੰਮਾਈ];
	ਪੂਰਨ ਖਾਨਾ, ਪੰਕਤੀ, ret;

	ret = uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &dev);
	ਜੇ (ret)
		ਵਾਪਸ;

#ਜੇ IS_ENABLED(CONFIG_VIDEO_LOGO)
	ਖਾਨਾ = BMP_LOGO_ਚੌੜਾਈ / fontdata->ਚੌੜਾਈ + 1;
	ਪੰਕਤੀ = BMP_LOGO_ਉਚਾਈ / fontdata->ਉਚਾਈ + 1;
#ਨਹੀਂਤਾਂ
	ਖਾਨਾ = 0;
	ਪੰਕਤੀ = 0;
#ਜੇ_ਅੰਤ

	display_options_get_banner(false, buf, ਆਕਾਰ(buf));
	vidconsole_position_cursor(dev, ਖਾਨਾ, 1);
	vidconsole_put_string(dev, buf);
	vidconsole_position_cursor(dev, 0, ਪੰਕਤੀ);
}
#ਜੇ_ਅੰਤ /* CONFIG_VIDEO && !CONFIG_HIDE_LOGO_VERSION */

/*
 * Common function to show a splash image if env("splashimage") is set.
 * For additional details please refer to doc/README.splashprepare.
 */
ਪੂਰਨ splash_display(ਖਾਲੀ)
{
	ulong addr;
	ਅੱਖਰ *s;
	ਪੂਰਨ x = 0, y = 0, ret;
	ਜੇ (!CONFIG_IS_ENABLED(SPLASH_SCREEN))
		ਵਾਪਸ -ENOSYS;
	s = env_get("splashimage");
	ਜੇ (!s)
		ਵਾਪਸ -EINVAL;

	addr = hextoul(s, ਨਿਰਾ);
	ret = splash_screen_prepare();
	ਜੇ (ret)
		ਵਾਪਸ ret;

	splash_get_pos(&x, &y);

	ਜੇ (CONFIG_IS_ENABLED(BMP))
		ret = bmp_display(addr, x, y);
	ਨਹੀਂਤਾਂ
		ਵਾਪਸ -ENOSYS;

	/* Skip banner output on video console if the logo is not at 0,0 */
	ਜੇ (x || y)
		ਜਾਓ ਸਿਰਾ_2;

#ਜੇ CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_ਸੰਸਕਰਣ)
	splash_display_banner();
#ਜੇ_ਅੰਤ
ਸਿਰਾ_2:
	ਵਾਪਸ ret;
}
