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

static පූර්ණ_සංඛ්‍යා splash_video_logo_load(void)
{
	var_na *splashimage;
	ulong bmp_load_addr;

	splashimage = env_get("splashimage");
	නම් (!splashimage)
		ආපසු -ENOENT;

	bmp_load_addr = hextoul(splashimage, 0);
	නම් (!bmp_load_addr) {
		printf("Error: bad 'splashimage' address\n");
		ආපසු -EFAULT;
	}

	memcpy((void *)bmp_load_addr, bmp_logo_bitmap,
	       ARRAY_SIZE(bmp_logo_bitmap));

	ආපසු 0;
}
#නැතහොත්
static anta_hstha පූර්ණ_සංඛ්‍යා splash_video_logo_load(void) { ආපසු -ENOSYS; }
#endif

__weak පූර්ණ_සංඛ්‍යා splash_screen_prepare(void)
{
	නම් (CONFIG_IS_ENABLED(SPLASH_SOURCE))
		ආපසු splash_source_load(default_splash_locations,
					  ARRAY_SIZE(default_splash_locations));

	ආපසු splash_video_logo_load();
}

void splash_get_pos(පූර්ණ_සංඛ්‍යා *x, පූර්ණ_සංඛ්‍යා *y)
{
	var_na *s = env_get("splashpos");

	නම් (!CONFIG_IS_ENABLED(SPLASH_SCREEN_ALIGN) || !s)
		ආපසු;

	නම් (s[0] == 'm')
		*x = BMP_ALIGN_CENTER;
	නැතහොත්
		*x = simple_strtol(s, NULL, 0);

	s = strchr(s + 1, ',');
	නම් (s != NULL) {
		නම් (s[1] == 'm')
			*y = BMP_ALIGN_CENTER;
		නැතහොත්
			*y = simple_strtol(s + 1, NULL, 0);
	}
}

#නම් CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)

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
	පූර්ණ_සංඛ්‍යා col, row, ret;

	ret = uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &dev);
	නම් (ret)
		ආපසු;

#නම් IS_ENABLED(CONFIG_VIDEO_LOGO)
	col = BMP_LOGO_WIDTH / fontdata->width + 1;
	row = BMP_LOGO_HEIGHT / fontdata->height + 1;
#නැතහොත්
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
පූර්ණ_සංඛ්‍යා splash_display(void)
{
	ulong addr;
	var_na *s;
	පූර්ණ_සංඛ්‍යා x = 0, y = 0, ret;
	නම් (!CONFIG_IS_ENABLED(SPLASH_SCREEN))
		ආපසු -ENOSYS;
	s = env_get("splashimage");
	නම් (!s)
		ආපසු -EINVAL;

	addr = hextoul(s, NULL);
	ret = splash_screen_prepare();
	නම් (ret)
		ආපසු ret;

	splash_get_pos(&x, &y);

	නම් (CONFIG_IS_ENABLED(BMP))
		ret = bmp_display(addr, x, y);
	නැතහොත්
		ආපසු -ENOSYS;

	/* Skip banner output on video console if the logo is not at 0,0 */
	නම් (x || y)
		goto end;

#නම් CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)
	splash_display_banner();
#endif
end:
	ආපසු ret;
}
