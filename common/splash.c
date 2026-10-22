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

ಸ್ಥಿರ ರಚನೆ splash_location default_splash_locations[] = {
	{
		.name = "sf",
		.storage = SPLASH_STORAGE_SF,
		.flags = SPLASH_STORAGE_RAW,
		.ಸ್ಥಾನಾಂತರ = 0x0,
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

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_VIDEO_LOGO

#ಅನ್ತರ್ಭಾವ <bmp_logo_data.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ splash_video_logo_ತುಂಬು(ಶೂನ್ಯ)
{
	ಅಕ್ಷರ *splashimage;
	ulong bmp_ತುಂಬು_addr;

	splashimage = env_get("splashimage");
	ಆದರೆ (!splashimage)
		ಹಿಂತಿರುಗಿ -ENOENT;

	bmp_ತುಂಬು_addr = hextoul(splashimage, 0);
	ಆದರೆ (!bmp_ತುಂಬು_addr) {
		ಬರೆ("Error: bad 'splashimage' address\n");
		ಹಿಂತಿರುಗಿ -EFAULT;
	}

	ಸ್ಮರಣೆ_ನಕಲು((ಶೂನ್ಯ *)bmp_ತುಂಬು_addr, bmp_logo_bitmap,
	       ARRAY_ಗಾತ್ರ(bmp_logo_bitmap));

	ಹಿಂತಿರುಗಿ 0;
}
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ splash_video_logo_ತುಂಬು(ಶೂನ್ಯ) { ಹಿಂತಿರುಗಿ -ENOSYS; }
#ಆದರೆ_ಅಂತ್ಯ

__weak ಪೂರ್ಣಾಂಕ splash_screen_prepare(ಶೂನ್ಯ)
{
	ಆದರೆ (CONFIG_IS_ENABLED(SPLASH_ಮೂಲ))
		ಹಿಂತಿರುಗಿ splash_ಮೂಲ_ತುಂಬು(default_splash_locations,
					  ARRAY_ಗಾತ್ರ(default_splash_locations));

	ಹಿಂತಿರುಗಿ splash_video_logo_ತುಂಬು();
}

ಶೂನ್ಯ splash_get_pos(ಪೂರ್ಣಾಂಕ *x, ಪೂರ್ಣಾಂಕ *y)
{
	ಅಕ್ಷರ *s = env_get("splashpos");

	ಆದರೆ (!CONFIG_IS_ENABLED(SPLASH_SCREEN_ALIGN) || !s)
		ಹಿಂತಿರುಗಿ;

	ಆದರೆ (s[0] == 'm')
		*x = BMP_ALIGN_CENTER;
	ಇಲ್ಲದಿದ್ದರೆ
		*x = simple_strtol(s, ಖಾಲಿ, 0);

	s = strchr(s + 1, ',');
	ಆದರೆ (s != ಖಾಲಿ) {
		ಆದರೆ (s[1] == 'm')
			*y = BMP_ALIGN_CENTER;
		ಇಲ್ಲದಿದ್ದರೆ
			*y = simple_strtol(s + 1, ಖಾಲಿ, 0);
	}
}

#ಆದರೆ CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_ಆವೃತ್ತಿ)

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_VIDEO_LOGO
#ಅನ್ತರ್ಭಾವ <bmp_logo.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <dm.h>
#ಅನ್ತರ್ಭಾವ <video_console.h>
#ಅನ್ತರ್ಭಾವ <video_font.h>
#ಅನ್ತರ್ಭಾವ <video_font_data.h>

ಶೂನ್ಯ splash_display_banner(ಶೂನ್ಯ)
{
	ರಚನೆ video_fontdata __maybe_unused *fontdata = fonts;
	ರಚನೆ udevice *dev;
	ಅಕ್ಷರ buf[DISPLAY_OPTIONS_BANNER_ಉದ್ದಳತೆ];
	ಪೂರ್ಣಾಂಕ ಲಂಬಸಾಲು, ಅಡ್ಡಸಾಲು, ret;

	ret = uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &dev);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ;

#ಆದರೆ IS_ENABLED(CONFIG_VIDEO_LOGO)
	ಲಂಬಸಾಲು = BMP_LOGO_ಅಗಲ / fontdata->ಅಗಲ + 1;
	ಅಡ್ಡಸಾಲು = BMP_LOGO_ಎತ್ತರ / fontdata->ಎತ್ತರ + 1;
#ಇಲ್ಲದಿದ್ದರೆ
	ಲಂಬಸಾಲು = 0;
	ಅಡ್ಡಸಾಲು = 0;
#ಆದರೆ_ಅಂತ್ಯ

	display_options_get_banner(false, buf, ಗಾತ್ರ(buf));
	vidconsole_position_cursor(dev, ಲಂಬಸಾಲು, 1);
	vidconsole_put_string(dev, buf);
	vidconsole_position_cursor(dev, 0, ಅಡ್ಡಸಾಲು);
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_VIDEO && !CONFIG_HIDE_LOGO_VERSION */

/*
 * Common function to show a splash image if env("splashimage") is set.
 * For additional details please refer to doc/README.splashprepare.
 */
ಪೂರ್ಣಾಂಕ splash_display(ಶೂನ್ಯ)
{
	ulong addr;
	ಅಕ್ಷರ *s;
	ಪೂರ್ಣಾಂಕ x = 0, y = 0, ret;
	ಆದರೆ (!CONFIG_IS_ENABLED(SPLASH_SCREEN))
		ಹಿಂತಿರುಗಿ -ENOSYS;
	s = env_get("splashimage");
	ಆದರೆ (!s)
		ಹಿಂತಿರುಗಿ -EINVAL;

	addr = hextoul(s, ಖಾಲಿ);
	ret = splash_screen_prepare();
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;

	splash_get_pos(&x, &y);

	ಆದರೆ (CONFIG_IS_ENABLED(BMP))
		ret = bmp_display(addr, x, y);
	ಇಲ್ಲದಿದ್ದರೆ
		ಹಿಂತಿರುಗಿ -ENOSYS;

	/* Skip banner output on video console if the logo is not at 0,0 */
	ಆದರೆ (x || y)
		ಹೋಗು ಕೊನೆ_2;

#ಆದರೆ CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_ಆವೃತ್ತಿ)
	splash_display_banner();
#ಆದರೆ_ಅಂತ್ಯ
ಕೊನೆ_2:
	ಹಿಂತಿರುಗಿ ret;
}
