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
	ᱡᱩᱫᱤ (!splashimage)
		ᱨᱩᱣᱟᱹᱲ -ENOENT;

	bmp_load_addr = hextoul(splashimage, 0);
	ᱡᱩᱫᱤ (!bmp_load_addr) {
		printf("Error: bad 'splashimage' address\n");
		ᱨᱩᱣᱟᱹᱲ -EFAULT;
	}

	memcpy((void *)bmp_load_addr, bmp_logo_bitmap,
	       ARRAY_SIZE(bmp_logo_bitmap));

	ᱨᱩᱣᱟᱹᱲ 0;
}
#ᱵᱟᱝ
static anta_hstha puur_naa_nka splash_video_logo_load(void) { ᱨᱩᱣᱟᱹᱲ -ENOSYS; }
#endif

__weak puur_naa_nka splash_screen_prepare(void)
{
	ᱡᱩᱫᱤ (CONFIG_IS_ENABLED(SPLASH_SOURCE))
		ᱨᱩᱣᱟᱹᱲ splash_source_load(default_splash_locations,
					  ARRAY_SIZE(default_splash_locations));

	ᱨᱩᱣᱟᱹᱲ splash_video_logo_load();
}

void splash_get_pos(puur_naa_nka *x, puur_naa_nka *y)
{
	var_na *s = env_get("splashpos");

	ᱡᱩᱫᱤ (!CONFIG_IS_ENABLED(SPLASH_SCREEN_ALIGN) || !s)
		ᱨᱩᱣᱟᱹᱲ;

	ᱡᱩᱫᱤ (s[0] == 'm')
		*x = BMP_ALIGN_CENTER;
	ᱵᱟᱝ
		*x = simple_strtol(s, NULL, 0);

	s = strchr(s + 1, ',');
	ᱡᱩᱫᱤ (s != NULL) {
		ᱡᱩᱫᱤ (s[1] == 'm')
			*y = BMP_ALIGN_CENTER;
		ᱵᱟᱝ
			*y = simple_strtol(s + 1, NULL, 0);
	}
}

#ᱡᱩᱫᱤ CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)

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
	ᱡᱩᱫᱤ (ret)
		ᱨᱩᱣᱟᱹᱲ;

#ᱡᱩᱫᱤ IS_ENABLED(CONFIG_VIDEO_LOGO)
	col = BMP_LOGO_WIDTH / fontdata->width + 1;
	row = BMP_LOGO_HEIGHT / fontdata->height + 1;
#ᱵᱟᱝ
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
	ᱡᱩᱫᱤ (!CONFIG_IS_ENABLED(SPLASH_SCREEN))
		ᱨᱩᱣᱟᱹᱲ -ENOSYS;
	s = env_get("splashimage");
	ᱡᱩᱫᱤ (!s)
		ᱨᱩᱣᱟᱹᱲ -EINVAL;

	addr = hextoul(s, NULL);
	ret = splash_screen_prepare();
	ᱡᱩᱫᱤ (ret)
		ᱨᱩᱣᱟᱹᱲ ret;

	splash_get_pos(&x, &y);

	ᱡᱩᱫᱤ (CONFIG_IS_ENABLED(BMP))
		ret = bmp_display(addr, x, y);
	ᱵᱟᱝ
		ᱨᱩᱣᱟᱹᱲ -ENOSYS;

	/* Skip banner output on video console if the logo is not at 0,0 */
	ᱡᱩᱫᱤ (x || y)
		goto end;

#ᱡᱩᱫᱤ CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)
	splash_display_banner();
#endif
end:
	ᱨᱩᱣᱟᱹᱲ ret;
}
