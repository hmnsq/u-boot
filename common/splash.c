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

static ကိန်းပြည့် splash_video_logo_load(void)
{
	var_na *splashimage;
	ulong bmp_load_addr;

	splashimage = env_get("splashimage");
	အကယ်၍ (!splashimage)
		ပြန်ပေး -ENOENT;

	bmp_load_addr = hextoul(splashimage, 0);
	အကယ်၍ (!bmp_load_addr) {
		printf("Error: bad 'splashimage' address\n");
		ပြန်ပေး -EFAULT;
	}

	memcpy((void *)bmp_load_addr, bmp_logo_bitmap,
	       ARRAY_SIZE(bmp_logo_bitmap));

	ပြန်ပေး 0;
}
#မဟုတ်ပါက
static anta_hstha ကိန်းပြည့် splash_video_logo_load(void) { ပြန်ပေး -ENOSYS; }
#endif

__weak ကိန်းပြည့် splash_screen_prepare(void)
{
	အကယ်၍ (CONFIG_IS_ENABLED(SPLASH_SOURCE))
		ပြန်ပေး splash_source_load(default_splash_locations,
					  ARRAY_SIZE(default_splash_locations));

	ပြန်ပေး splash_video_logo_load();
}

void splash_get_pos(ကိန်းပြည့် *x, ကိန်းပြည့် *y)
{
	var_na *s = env_get("splashpos");

	အကယ်၍ (!CONFIG_IS_ENABLED(SPLASH_SCREEN_ALIGN) || !s)
		ပြန်ပေး;

	အကယ်၍ (s[0] == 'm')
		*x = BMP_ALIGN_CENTER;
	မဟုတ်ပါက
		*x = simple_strtol(s, NULL, 0);

	s = strchr(s + 1, ',');
	အကယ်၍ (s != NULL) {
		အကယ်၍ (s[1] == 'm')
			*y = BMP_ALIGN_CENTER;
		မဟုတ်ပါက
			*y = simple_strtol(s + 1, NULL, 0);
	}
}

#အကယ်၍ CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)

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
	ကိန်းပြည့် col, row, ret;

	ret = uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &dev);
	အကယ်၍ (ret)
		ပြန်ပေး;

#အကယ်၍ IS_ENABLED(CONFIG_VIDEO_LOGO)
	col = BMP_LOGO_WIDTH / fontdata->width + 1;
	row = BMP_LOGO_HEIGHT / fontdata->height + 1;
#မဟုတ်ပါက
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
ကိန်းပြည့် splash_display(void)
{
	ulong addr;
	var_na *s;
	ကိန်းပြည့် x = 0, y = 0, ret;
	အကယ်၍ (!CONFIG_IS_ENABLED(SPLASH_SCREEN))
		ပြန်ပေး -ENOSYS;
	s = env_get("splashimage");
	အကယ်၍ (!s)
		ပြန်ပေး -EINVAL;

	addr = hextoul(s, NULL);
	ret = splash_screen_prepare();
	အကယ်၍ (ret)
		ပြန်ပေး ret;

	splash_get_pos(&x, &y);

	အကယ်၍ (CONFIG_IS_ENABLED(BMP))
		ret = bmp_display(addr, x, y);
	မဟုတ်ပါက
		ပြန်ပေး -ENOSYS;

	/* Skip banner output on video console if the logo is not at 0,0 */
	အကယ်၍ (x || y)
		goto end;

#အကယ်၍ CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)
	splash_display_banner();
#endif
end:
	ပြန်ပေး ret;
}
