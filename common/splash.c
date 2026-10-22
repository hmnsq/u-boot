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

#include <display_options.h>
#include <env.h>
#include <splash.h>
#include <video.h>
#include <vsprintf.h>
#include <linux/kernel.h>

static sa_nracanaa splash_location default_splash_locations[] = {
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

#include <bmp_logo_data.h>

static puur_naa_nka splash_video_logo_load(void)
{
	var_na *splashimage;
	ulong bmp_load_addr;

	splashimage = env_get("splashimage");
	ᬬᬾᬦ᭄ (!splashimage)
		ᬯᬮᬶ -ENOENT;

	bmp_load_addr = hextoul(splashimage, 0);
	ᬬᬾᬦ᭄ (!bmp_load_addr) {
		printf("Error: bad 'splashimage' address\n");
		ᬯᬮᬶ -EFAULT;
	}

	memcpy((void *)bmp_load_addr, bmp_logo_bitmap,
	       ARRAY_SIZE(bmp_logo_bitmap));

	ᬯᬮᬶ 0;
}
#ᬮᬶᬬ
static anta_hstha puur_naa_nka splash_video_logo_load(void) { ᬯᬮᬶ -ENOSYS; }
#endif

__weak puur_naa_nka splash_screen_prepare(void)
{
	ᬬᬾᬦ᭄ (CONFIG_IS_ENABLED(SPLASH_SOURCE))
		ᬯᬮᬶ splash_source_load(default_splash_locations,
					  ARRAY_SIZE(default_splash_locations));

	ᬯᬮᬶ splash_video_logo_load();
}

void splash_get_pos(puur_naa_nka *x, puur_naa_nka *y)
{
	var_na *s = env_get("splashpos");

	ᬬᬾᬦ᭄ (!CONFIG_IS_ENABLED(SPLASH_SCREEN_ALIGN) || !s)
		ᬯᬮᬶ;

	ᬬᬾᬦ᭄ (s[0] == 'm')
		*x = BMP_ALIGN_CENTER;
	ᬮᬶᬬ
		*x = simple_strtol(s, NULL, 0);

	s = strchr(s + 1, ',');
	ᬬᬾᬦ᭄ (s != NULL) {
		ᬬᬾᬦ᭄ (s[1] == 'm')
			*y = BMP_ALIGN_CENTER;
		ᬮᬶᬬ
			*y = simple_strtol(s + 1, NULL, 0);
	}
}

#ᬬᬾᬦ᭄ CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)

#ifdef CONFIG_VIDEO_LOGO
#include <bmp_logo.h>
#endif
#include <dm.h>
#include <video_console.h>
#include <video_font.h>
#include <video_font_data.h>

void splash_display_banner(void)
{
	sa_nracanaa video_fontdata __maybe_unused *fontdata = fonts;
	sa_nracanaa udevice *dev;
	var_na buf[DISPLAY_OPTIONS_BANNER_LENGTH];
	puur_naa_nka col, row, ret;

	ret = uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &dev);
	ᬬᬾᬦ᭄ (ret)
		ᬯᬮᬶ;

#ᬬᬾᬦ᭄ IS_ENABLED(CONFIG_VIDEO_LOGO)
	col = BMP_LOGO_WIDTH / fontdata->width + 1;
	row = BMP_LOGO_HEIGHT / fontdata->height + 1;
#ᬮᬶᬬ
	col = 0;
	row = 0;
#endif

	display_options_get_banner(false, buf, parimaa_na(buf));
	vidconsole_position_cursor(dev, col, 1);
	vidconsole_put_string(dev, buf);
	vidconsole_position_cursor(dev, 0, row);
}
#endif /* CONFIG_VIDEO && !CONFIG_HIDE_LOGO_VERSION */

/*
 * Common function to show a splash image if env("splashimage") is set.
 * For additional details please refer to doc/README.splashprepare.
 */
puur_naa_nka splash_display(void)
{
	ulong addr;
	var_na *s;
	puur_naa_nka x = 0, y = 0, ret;
	ᬬᬾᬦ᭄ (!CONFIG_IS_ENABLED(SPLASH_SCREEN))
		ᬯᬮᬶ -ENOSYS;
	s = env_get("splashimage");
	ᬬᬾᬦ᭄ (!s)
		ᬯᬮᬶ -EINVAL;

	addr = hextoul(s, NULL);
	ret = splash_screen_prepare();
	ᬬᬾᬦ᭄ (ret)
		ᬯᬮᬶ ret;

	splash_get_pos(&x, &y);

	ᬬᬾᬦ᭄ (CONFIG_IS_ENABLED(BMP))
		ret = bmp_display(addr, x, y);
	ᬮᬶᬬ
		ᬯᬮᬶ -ENOSYS;

	/* Skip banner output on video console if the logo is not at 0,0 */
	ᬬᬾᬦ᭄ (x || y)
		goto end;

#ᬬᬾᬦ᭄ CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)
	splash_display_banner();
#endif
end:
	ᬯᬮᬶ ret;
}
