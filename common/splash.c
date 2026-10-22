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

#അന്തര്ഭാവ <display_options.h>
#അന്തര്ഭാവ <env.h>
#അന്തര്ഭാവ <splash.h>
#അന്തര്ഭാവ <video.h>
#അന്തര്ഭാവ <vsprintf.h>
#അന്തര്ഭാവ <linux/kernel.h>

സ്ഥിതം ഘടന splash_location default_splash_locations[] = {
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

#അന്തര്ഭാവ <bmp_logo_data.h>

സ്ഥിതം പൂർണ്ണസംഖ്യ splash_video_logo_load(ശൂന്യം)
{
	അക്ഷരം *splashimage;
	ulong bmp_load_addr;

	splashimage = env_get("splashimage");
	എങ്കിൽ (!splashimage)
		തിരികെ -ENOENT;

	bmp_load_addr = hextoul(splashimage, 0);
	എങ്കിൽ (!bmp_load_addr) {
		printf("Error: bad 'splashimage' address\n");
		തിരികെ -EFAULT;
	}

	memcpy((ശൂന്യം *)bmp_load_addr, bmp_logo_bitmap,
	       ARRAY_SIZE(bmp_logo_bitmap));

	തിരികെ 0;
}
#അല്ലെങ്കിൽ
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ splash_video_logo_load(ശൂന്യം) { തിരികെ -ENOSYS; }
#endif

__weak പൂർണ്ണസംഖ്യ splash_screen_prepare(ശൂന്യം)
{
	എങ്കിൽ (CONFIG_IS_ENABLED(SPLASH_SOURCE))
		തിരികെ splash_source_load(default_splash_locations,
					  ARRAY_SIZE(default_splash_locations));

	തിരികെ splash_video_logo_load();
}

ശൂന്യം splash_get_pos(പൂർണ്ണസംഖ്യ *x, പൂർണ്ണസംഖ്യ *y)
{
	അക്ഷരം *s = env_get("splashpos");

	എങ്കിൽ (!CONFIG_IS_ENABLED(SPLASH_SCREEN_ALIGN) || !s)
		തിരികെ;

	എങ്കിൽ (s[0] == 'm')
		*x = BMP_ALIGN_CENTER;
	അല്ലെങ്കിൽ
		*x = simple_strtol(s, ശൂന്യ, 0);

	s = strchr(s + 1, ',');
	എങ്കിൽ (s != ശൂന്യ) {
		എങ്കിൽ (s[1] == 'm')
			*y = BMP_ALIGN_CENTER;
		അല്ലെങ്കിൽ
			*y = simple_strtol(s + 1, ശൂന്യ, 0);
	}
}

#എങ്കിൽ CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)

#ifdef CONFIG_VIDEO_LOGO
#അന്തര്ഭാവ <bmp_logo.h>
#endif
#അന്തര്ഭാവ <dm.h>
#അന്തര്ഭാവ <video_console.h>
#അന്തര്ഭാവ <video_font.h>
#അന്തര്ഭാവ <video_font_data.h>

ശൂന്യം splash_display_banner(ശൂന്യം)
{
	ഘടന video_fontdata __maybe_unused *fontdata = fonts;
	ഘടന udevice *dev;
	അക്ഷരം buf[DISPLAY_OPTIONS_BANNER_LENGTH];
	പൂർണ്ണസംഖ്യ col, row, ret;

	ret = uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &dev);
	എങ്കിൽ (ret)
		തിരികെ;

#എങ്കിൽ IS_ENABLED(CONFIG_VIDEO_LOGO)
	col = BMP_LOGO_WIDTH / fontdata->width + 1;
	row = BMP_LOGO_HEIGHT / fontdata->height + 1;
#അല്ലെങ്കിൽ
	col = 0;
	row = 0;
#endif

	display_options_get_banner(false, buf, വലുപ്പം(buf));
	vidconsole_position_cursor(dev, col, 1);
	vidconsole_put_string(dev, buf);
	vidconsole_position_cursor(dev, 0, row);
}
#endif /* CONFIG_VIDEO && !CONFIG_HIDE_LOGO_VERSION */

/*
 * Common function to show a splash image if env("splashimage") is set.
 * For additional details please refer to doc/README.splashprepare.
 */
പൂർണ്ണസംഖ്യ splash_display(ശൂന്യം)
{
	ulong addr;
	അക്ഷരം *s;
	പൂർണ്ണസംഖ്യ x = 0, y = 0, ret;
	എങ്കിൽ (!CONFIG_IS_ENABLED(SPLASH_SCREEN))
		തിരികെ -ENOSYS;
	s = env_get("splashimage");
	എങ്കിൽ (!s)
		തിരികെ -EINVAL;

	addr = hextoul(s, ശൂന്യ);
	ret = splash_screen_prepare();
	എങ്കിൽ (ret)
		തിരികെ ret;

	splash_get_pos(&x, &y);

	എങ്കിൽ (CONFIG_IS_ENABLED(BMP))
		ret = bmp_display(addr, x, y);
	അല്ലെങ്കിൽ
		തിരികെ -ENOSYS;

	/* Skip banner output on video console if the logo is not at 0,0 */
	എങ്കിൽ (x || y)
		പോകുക end;

#എങ്കിൽ CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)
	splash_display_banner();
#endif
end:
	തിരികെ ret;
}
