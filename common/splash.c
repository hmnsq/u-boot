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

#அந்தர்பாவ <display_options.h>
#அந்தர்பாவ <env.h>
#அந்தர்பாவ <splash.h>
#அந்தர்பாவ <video.h>
#அந்தர்பாவ <vsprintf.h>
#அந்தர்பாவ <linux/kernel.h>

நிலையான கட்டமைப்பு splash_location default_splash_locations[] = {
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

#அந்தர்பாவ <bmp_logo_data.h>

நிலையான முழு splash_video_logo_load(வெறுமை)
{
	எழுத்து *splashimage;
	ulong bmp_load_addr;

	splashimage = env_get("splashimage");
	எனில் (!splashimage)
		திரும்பு -ENOENT;

	bmp_load_addr = hextoul(splashimage, 0);
	எனில் (!bmp_load_addr) {
		printf("Error: bad 'splashimage' address\n");
		திரும்பு -EFAULT;
	}

	memcpy((வெறுமை *)bmp_load_addr, bmp_logo_bitmap,
	       ARRAY_SIZE(bmp_logo_bitmap));

	திரும்பு 0;
}
#இல்லையெனில்
நிலையான உள்வரி முழு splash_video_logo_load(வெறுமை) { திரும்பு -ENOSYS; }
#endif

__weak முழு splash_screen_prepare(வெறுமை)
{
	எனில் (CONFIG_IS_ENABLED(SPLASH_SOURCE))
		திரும்பு splash_source_load(default_splash_locations,
					  ARRAY_SIZE(default_splash_locations));

	திரும்பு splash_video_logo_load();
}

வெறுமை splash_get_pos(முழு *x, முழு *y)
{
	எழுத்து *s = env_get("splashpos");

	எனில் (!CONFIG_IS_ENABLED(SPLASH_SCREEN_ALIGN) || !s)
		திரும்பு;

	எனில் (s[0] == 'm')
		*x = BMP_ALIGN_CENTER;
	இல்லையெனில்
		*x = simple_strtol(s, வெற்று, 0);

	s = strchr(s + 1, ',');
	எனில் (s != வெற்று) {
		எனில் (s[1] == 'm')
			*y = BMP_ALIGN_CENTER;
		இல்லையெனில்
			*y = simple_strtol(s + 1, வெற்று, 0);
	}
}

#எனில் CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)

#ifdef CONFIG_VIDEO_LOGO
#அந்தர்பாவ <bmp_logo.h>
#endif
#அந்தர்பாவ <dm.h>
#அந்தர்பாவ <video_console.h>
#அந்தர்பாவ <video_font.h>
#அந்தர்பாவ <video_font_data.h>

வெறுமை splash_display_banner(வெறுமை)
{
	கட்டமைப்பு video_fontdata __maybe_unused *fontdata = fonts;
	கட்டமைப்பு udevice *dev;
	எழுத்து buf[DISPLAY_OPTIONS_BANNER_LENGTH];
	முழு col, row, ret;

	ret = uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &dev);
	எனில் (ret)
		திரும்பு;

#எனில் IS_ENABLED(CONFIG_VIDEO_LOGO)
	col = BMP_LOGO_WIDTH / fontdata->width + 1;
	row = BMP_LOGO_HEIGHT / fontdata->height + 1;
#இல்லையெனில்
	col = 0;
	row = 0;
#endif

	display_options_get_banner(false, buf, அளவு(buf));
	vidconsole_position_cursor(dev, col, 1);
	vidconsole_put_string(dev, buf);
	vidconsole_position_cursor(dev, 0, row);
}
#endif /* CONFIG_VIDEO && !CONFIG_HIDE_LOGO_VERSION */

/*
 * Common function to show a splash image if env("splashimage") is set.
 * For additional details please refer to doc/README.splashprepare.
 */
முழு splash_display(வெறுமை)
{
	ulong addr;
	எழுத்து *s;
	முழு x = 0, y = 0, ret;
	எனில் (!CONFIG_IS_ENABLED(SPLASH_SCREEN))
		திரும்பு -ENOSYS;
	s = env_get("splashimage");
	எனில் (!s)
		திரும்பு -EINVAL;

	addr = hextoul(s, வெற்று);
	ret = splash_screen_prepare();
	எனில் (ret)
		திரும்பு ret;

	splash_get_pos(&x, &y);

	எனில் (CONFIG_IS_ENABLED(BMP))
		ret = bmp_display(addr, x, y);
	இல்லையெனில்
		திரும்பு -ENOSYS;

	/* Skip banner output on video console if the logo is not at 0,0 */
	எனில் (x || y)
		செல் end;

#எனில் CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)
	splash_display_banner();
#endif
end:
	திரும்பு ret;
}
