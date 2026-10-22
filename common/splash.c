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

স্থির সংরচনা splash_location default_splash_locations[] = {
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

স্থির পূর্ণাঙ্ক splash_video_logo_load(শূন্য)
{
	বর্ণ *splashimage;
	ulong bmp_load_addr;

	splashimage = env_get("splashimage");
	যদি (!splashimage)
		প্রত্যাগম -ENOENT;

	bmp_load_addr = hextoul(splashimage, 0);
	যদি (!bmp_load_addr) {
		printf("Error: bad 'splashimage' address\n");
		প্রত্যাগম -EFAULT;
	}

	memcpy((শূন্য *)bmp_load_addr, bmp_logo_bitmap,
	       ARRAY_SIZE(bmp_logo_bitmap));

	প্রত্যাগম 0;
}
#অন্যথা
স্থির অন্তঃস্থ পূর্ণাঙ্ক splash_video_logo_load(শূন্য) { প্রত্যাগম -ENOSYS; }
#endif

__weak পূর্ণাঙ্ক splash_screen_prepare(শূন্য)
{
	যদি (CONFIG_IS_ENABLED(SPLASH_SOURCE))
		প্রত্যাগম splash_source_load(default_splash_locations,
					  ARRAY_SIZE(default_splash_locations));

	প্রত্যাগম splash_video_logo_load();
}

শূন্য splash_get_pos(পূর্ণাঙ্ক *x, পূর্ণাঙ্ক *y)
{
	বর্ণ *s = env_get("splashpos");

	যদি (!CONFIG_IS_ENABLED(SPLASH_SCREEN_ALIGN) || !s)
		প্রত্যাগম;

	যদি (s[0] == 'm')
		*x = BMP_ALIGN_CENTER;
	অন্যথা
		*x = simple_strtol(s, রিক্ত, 0);

	s = strchr(s + 1, ',');
	যদি (s != রিক্ত) {
		যদি (s[1] == 'm')
			*y = BMP_ALIGN_CENTER;
		অন্যথা
			*y = simple_strtol(s + 1, রিক্ত, 0);
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
	সংরচনা video_fontdata __maybe_unused *fontdata = fonts;
	সংরচনা udevice *dev;
	বর্ণ buf[DISPLAY_OPTIONS_BANNER_LENGTH];
	পূর্ণাঙ্ক col, row, ret;

	ret = uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &dev);
	যদি (ret)
		প্রত্যাগম;

#যদি IS_ENABLED(CONFIG_VIDEO_LOGO)
	col = BMP_LOGO_WIDTH / fontdata->width + 1;
	row = BMP_LOGO_HEIGHT / fontdata->height + 1;
#অন্যথা
	col = 0;
	row = 0;
#endif

	display_options_get_banner(false, buf, পরিমাণ(buf));
	vidconsole_position_cursor(dev, col, 1);
	vidconsole_put_string(dev, buf);
	vidconsole_position_cursor(dev, 0, row);
}
#endif /* CONFIG_VIDEO && !CONFIG_HIDE_LOGO_VERSION */

/*
 * Common function to show a splash image if env("splashimage") is set.
 * For additional details please refer to doc/README.splashprepare.
 */
পূর্ণাঙ্ক splash_display(শূন্য)
{
	ulong addr;
	বর্ণ *s;
	পূর্ণাঙ্ক x = 0, y = 0, ret;
	যদি (!CONFIG_IS_ENABLED(SPLASH_SCREEN))
		প্রত্যাগম -ENOSYS;
	s = env_get("splashimage");
	যদি (!s)
		প্রত্যাগম -EINVAL;

	addr = hextoul(s, রিক্ত);
	ret = splash_screen_prepare();
	যদি (ret)
		প্রত্যাগম ret;

	splash_get_pos(&x, &y);

	যদি (CONFIG_IS_ENABLED(BMP))
		ret = bmp_display(addr, x, y);
	অন্যথা
		প্রত্যাগম -ENOSYS;

	/* Skip banner output on video console if the logo is not at 0,0 */
	যদি (x || y)
		গচ্ছ end;

#যদি CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)
	splash_display_banner();
#endif
end:
	প্রত্যাগম ret;
}
