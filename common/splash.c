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

#ଅନ୍ତର୍ଭାଵ <display_options.h>
#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <splash.h>
#ଅନ୍ତର୍ଭାଵ <video.h>
#ଅନ୍ତର୍ଭାଵ <vsprintf.h>
#ଅନ୍ତର୍ଭାଵ <linux/kernel.h>

ସ୍ଥିର ସଂରଚନା splash_location default_splash_locations[] = {
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

#ଅନ୍ତର୍ଭାଵ <bmp_logo_data.h>

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା splash_video_logo_load(ଶୂନ୍ୟ)
{
	ଅକ୍ଷର *splashimage;
	ulong bmp_load_addr;

	splashimage = env_get("splashimage");
	ଯଦି (!splashimage)
		ଫେରନ୍ତୁ -ENOENT;

	bmp_load_addr = hextoul(splashimage, 0);
	ଯଦି (!bmp_load_addr) {
		printf("Error: bad 'splashimage' address\n");
		ଫେରନ୍ତୁ -EFAULT;
	}

	memcpy((ଶୂନ୍ୟ *)bmp_load_addr, bmp_logo_bitmap,
	       ARRAY_SIZE(bmp_logo_bitmap));

	ଫେରନ୍ତୁ 0;
}
#ଅନ୍ୟଥା
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା splash_video_logo_load(ଶୂନ୍ୟ) { ଫେରନ୍ତୁ -ENOSYS; }
#endif

__weak ପୂର୍ଣ୍ଣସଂଖ୍ୟା splash_screen_prepare(ଶୂନ୍ୟ)
{
	ଯଦି (CONFIG_IS_ENABLED(SPLASH_SOURCE))
		ଫେରନ୍ତୁ splash_source_load(default_splash_locations,
					  ARRAY_SIZE(default_splash_locations));

	ଫେରନ୍ତୁ splash_video_logo_load();
}

ଶୂନ୍ୟ splash_get_pos(ପୂର୍ଣ୍ଣସଂଖ୍ୟା *x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *y)
{
	ଅକ୍ଷର *s = env_get("splashpos");

	ଯଦି (!CONFIG_IS_ENABLED(SPLASH_SCREEN_ALIGN) || !s)
		ଫେରନ୍ତୁ;

	ଯଦି (s[0] == 'm')
		*x = BMP_ALIGN_CENTER;
	ଅନ୍ୟଥା
		*x = simple_strtol(s, ଖାଲି, 0);

	s = strchr(s + 1, ',');
	ଯଦି (s != ଖାଲି) {
		ଯଦି (s[1] == 'm')
			*y = BMP_ALIGN_CENTER;
		ଅନ୍ୟଥା
			*y = simple_strtol(s + 1, ଖାଲି, 0);
	}
}

#ଯଦି CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)

#ifdef CONFIG_VIDEO_LOGO
#ଅନ୍ତର୍ଭାଵ <bmp_logo.h>
#endif
#ଅନ୍ତର୍ଭାଵ <dm.h>
#ଅନ୍ତର୍ଭାଵ <video_console.h>
#ଅନ୍ତର୍ଭାଵ <video_font.h>
#ଅନ୍ତର୍ଭାଵ <video_font_data.h>

ଶୂନ୍ୟ splash_display_banner(ଶୂନ୍ୟ)
{
	ସଂରଚନା video_fontdata __maybe_unused *fontdata = fonts;
	ସଂରଚନା udevice *dev;
	ଅକ୍ଷର buf[DISPLAY_OPTIONS_BANNER_LENGTH];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା col, row, ret;

	ret = uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &dev);
	ଯଦି (ret)
		ଫେରନ୍ତୁ;

#ଯଦି IS_ENABLED(CONFIG_VIDEO_LOGO)
	col = BMP_LOGO_WIDTH / fontdata->width + 1;
	row = BMP_LOGO_HEIGHT / fontdata->height + 1;
#ଅନ୍ୟଥା
	col = 0;
	row = 0;
#endif

	display_options_get_banner(false, buf, ଆକାର(buf));
	vidconsole_position_cursor(dev, col, 1);
	vidconsole_put_string(dev, buf);
	vidconsole_position_cursor(dev, 0, row);
}
#endif /* CONFIG_VIDEO && !CONFIG_HIDE_LOGO_VERSION */

/*
 * Common function to show a splash image if env("splashimage") is set.
 * For additional details please refer to doc/README.splashprepare.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା splash_display(ଶୂନ୍ୟ)
{
	ulong addr;
	ଅକ୍ଷର *s;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x = 0, y = 0, ret;
	ଯଦି (!CONFIG_IS_ENABLED(SPLASH_SCREEN))
		ଫେରନ୍ତୁ -ENOSYS;
	s = env_get("splashimage");
	ଯଦି (!s)
		ଫେରନ୍ତୁ -EINVAL;

	addr = hextoul(s, ଖାଲି);
	ret = splash_screen_prepare();
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;

	splash_get_pos(&x, &y);

	ଯଦି (CONFIG_IS_ENABLED(BMP))
		ret = bmp_display(addr, x, y);
	ଅନ୍ୟଥା
		ଫେରନ୍ତୁ -ENOSYS;

	/* Skip banner output on video console if the logo is not at 0,0 */
	ଯଦି (x || y)
		ଯାଅ end;

#ଯଦି CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)
	splash_display_banner();
#endif
end:
	ଫେରନ୍ତୁ ret;
}
