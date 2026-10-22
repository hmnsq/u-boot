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

#ಅನ್ತರ್ಭಾವ <display_options.h>
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <splash.h>
#ಅನ್ತರ್ಭಾವ <video.h>
#ಅನ್ತರ್ಭಾವ <vsprintf.h>
#ಅನ್ತರ್ಭಾವ <linux/kernel.h>

ಸ್ಥಿರ ಸಂರಚನಾ splash_location default_splash_locations[] = {
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

#ಅನ್ತರ್ಭಾವ <bmp_logo_data.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ splash_video_logo_load(ಶೂನ್ಯ)
{
	ವರ್ಣ *splashimage;
	ulong bmp_load_addr;

	splashimage = env_get("splashimage");
	ಯದಿ (!splashimage)
		ಪ್ರತ್ಯಾಗಮ -ENOENT;

	bmp_load_addr = hextoul(splashimage, 0);
	ಯದಿ (!bmp_load_addr) {
		printf("Error: bad 'splashimage' address\n");
		ಪ್ರತ್ಯಾಗಮ -EFAULT;
	}

	memcpy((ಶೂನ್ಯ *)bmp_load_addr, bmp_logo_bitmap,
	       ARRAY_SIZE(bmp_logo_bitmap));

	ಪ್ರತ್ಯಾಗಮ 0;
}
#ಅನ್ಯಥಾ
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ splash_video_logo_load(ಶೂನ್ಯ) { ಪ್ರತ್ಯಾಗಮ -ENOSYS; }
#endif

__weak ಪೂರ್ಣಾಙ್ಕ splash_screen_prepare(ಶೂನ್ಯ)
{
	ಯದಿ (CONFIG_IS_ENABLED(SPLASH_SOURCE))
		ಪ್ರತ್ಯಾಗಮ splash_source_load(default_splash_locations,
					  ARRAY_SIZE(default_splash_locations));

	ಪ್ರತ್ಯಾಗಮ splash_video_logo_load();
}

ಶೂನ್ಯ splash_get_pos(ಪೂರ್ಣಾಙ್ಕ *x, ಪೂರ್ಣಾಙ್ಕ *y)
{
	ವರ್ಣ *s = env_get("splashpos");

	ಯದಿ (!CONFIG_IS_ENABLED(SPLASH_SCREEN_ALIGN) || !s)
		ಪ್ರತ್ಯಾಗಮ;

	ಯದಿ (s[0] == 'm')
		*x = BMP_ALIGN_CENTER;
	ಅನ್ಯಥಾ
		*x = simple_strtol(s, ರಿಕ್ತ, 0);

	s = strchr(s + 1, ',');
	ಯದಿ (s != ರಿಕ್ತ) {
		ಯದಿ (s[1] == 'm')
			*y = BMP_ALIGN_CENTER;
		ಅನ್ಯಥಾ
			*y = simple_strtol(s + 1, ರಿಕ್ತ, 0);
	}
}

#ಯದಿ CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)

#ifdef CONFIG_VIDEO_LOGO
#ಅನ್ತರ್ಭಾವ <bmp_logo.h>
#endif
#ಅನ್ತರ್ಭಾವ <dm.h>
#ಅನ್ತರ್ಭಾವ <video_console.h>
#ಅನ್ತರ್ಭಾವ <video_font.h>
#ಅನ್ತರ್ಭಾವ <video_font_data.h>

ಶೂನ್ಯ splash_display_banner(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ video_fontdata __maybe_unused *fontdata = fonts;
	ಸಂರಚನಾ udevice *dev;
	ವರ್ಣ buf[DISPLAY_OPTIONS_BANNER_LENGTH];
	ಪೂರ್ಣಾಙ್ಕ col, row, ret;

	ret = uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &dev);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ;

#ಯದಿ IS_ENABLED(CONFIG_VIDEO_LOGO)
	col = BMP_LOGO_WIDTH / fontdata->width + 1;
	row = BMP_LOGO_HEIGHT / fontdata->height + 1;
#ಅನ್ಯಥಾ
	col = 0;
	row = 0;
#endif

	display_options_get_banner(false, buf, ಪರಿಮಾಣ(buf));
	vidconsole_position_cursor(dev, col, 1);
	vidconsole_put_string(dev, buf);
	vidconsole_position_cursor(dev, 0, row);
}
#endif /* CONFIG_VIDEO && !CONFIG_HIDE_LOGO_VERSION */

/*
 * Common function to show a splash image if env("splashimage") is set.
 * For additional details please refer to doc/README.splashprepare.
 */
ಪೂರ್ಣಾಙ್ಕ splash_display(ಶೂನ್ಯ)
{
	ulong addr;
	ವರ್ಣ *s;
	ಪೂರ್ಣಾಙ್ಕ x = 0, y = 0, ret;
	ಯದಿ (!CONFIG_IS_ENABLED(SPLASH_SCREEN))
		ಪ್ರತ್ಯಾಗಮ -ENOSYS;
	s = env_get("splashimage");
	ಯದಿ (!s)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	addr = hextoul(s, ರಿಕ್ತ);
	ret = splash_screen_prepare();
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;

	splash_get_pos(&x, &y);

	ಯದಿ (CONFIG_IS_ENABLED(BMP))
		ret = bmp_display(addr, x, y);
	ಅನ್ಯಥಾ
		ಪ್ರತ್ಯಾಗಮ -ENOSYS;

	/* Skip banner output on video console if the logo is not at 0,0 */
	ಯದಿ (x || y)
		ಗಚ್ಛ end;

#ಯದಿ CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)
	splash_display_banner();
#endif
end:
	ಪ್ರತ್ಯಾಗಮ ret;
}
