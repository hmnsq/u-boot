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

static ຈຳນວນເຕັມ splash_video_logo_load(void)
{
	var_na *splashimage;
	ulong bmp_load_addr;

	splashimage = env_get("splashimage");
	ຖ້າ (!splashimage)
		ສົ່ງຄືນ -ENOENT;

	bmp_load_addr = hextoul(splashimage, 0);
	ຖ້າ (!bmp_load_addr) {
		printf("Error: bad 'splashimage' address\n");
		ສົ່ງຄືນ -EFAULT;
	}

	memcpy((void *)bmp_load_addr, bmp_logo_bitmap,
	       ARRAY_SIZE(bmp_logo_bitmap));

	ສົ່ງຄືນ 0;
}
#ບໍ່ດັ່ງນັ້ນ
static anta_hstha ຈຳນວນເຕັມ splash_video_logo_load(void) { ສົ່ງຄືນ -ENOSYS; }
#endif

__weak ຈຳນວນເຕັມ splash_screen_prepare(void)
{
	ຖ້າ (CONFIG_IS_ENABLED(SPLASH_SOURCE))
		ສົ່ງຄືນ splash_source_load(default_splash_locations,
					  ARRAY_SIZE(default_splash_locations));

	ສົ່ງຄືນ splash_video_logo_load();
}

void splash_get_pos(ຈຳນວນເຕັມ *x, ຈຳນວນເຕັມ *y)
{
	var_na *s = env_get("splashpos");

	ຖ້າ (!CONFIG_IS_ENABLED(SPLASH_SCREEN_ALIGN) || !s)
		ສົ່ງຄືນ;

	ຖ້າ (s[0] == 'm')
		*x = BMP_ALIGN_CENTER;
	ບໍ່ດັ່ງນັ້ນ
		*x = simple_strtol(s, NULL, 0);

	s = strchr(s + 1, ',');
	ຖ້າ (s != NULL) {
		ຖ້າ (s[1] == 'm')
			*y = BMP_ALIGN_CENTER;
		ບໍ່ດັ່ງນັ້ນ
			*y = simple_strtol(s + 1, NULL, 0);
	}
}

#ຖ້າ CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)

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
	ຈຳນວນເຕັມ col, row, ret;

	ret = uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &dev);
	ຖ້າ (ret)
		ສົ່ງຄືນ;

#ຖ້າ IS_ENABLED(CONFIG_VIDEO_LOGO)
	col = BMP_LOGO_WIDTH / fontdata->width + 1;
	row = BMP_LOGO_HEIGHT / fontdata->height + 1;
#ບໍ່ດັ່ງນັ້ນ
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
ຈຳນວນເຕັມ splash_display(void)
{
	ulong addr;
	var_na *s;
	ຈຳນວນເຕັມ x = 0, y = 0, ret;
	ຖ້າ (!CONFIG_IS_ENABLED(SPLASH_SCREEN))
		ສົ່ງຄືນ -ENOSYS;
	s = env_get("splashimage");
	ຖ້າ (!s)
		ສົ່ງຄືນ -EINVAL;

	addr = hextoul(s, NULL);
	ret = splash_screen_prepare();
	ຖ້າ (ret)
		ສົ່ງຄືນ ret;

	splash_get_pos(&x, &y);

	ຖ້າ (CONFIG_IS_ENABLED(BMP))
		ret = bmp_display(addr, x, y);
	ບໍ່ດັ່ງນັ້ນ
		ສົ່ງຄືນ -ENOSYS;

	/* Skip banner output on video console if the logo is not at 0,0 */
	ຖ້າ (x || y)
		goto end;

#ຖ້າ CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)
	splash_display_banner();
#endif
end:
	ສົ່ງຄືນ ret;
}
