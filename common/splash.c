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

#అన్తర్భావ <display_options.h>
#అన్తర్భావ <env.h>
#అన్తర్భావ <splash.h>
#అన్తర్భావ <video.h>
#అన్తర్భావ <vsprintf.h>
#అన్తర్భావ <linux/kernel.h>

స్థిర నిర్మాణం splash_location default_splash_locations[] = {
	{
		.name = "sf",
		.storage = SPLASH_STORAGE_SF,
		.flags = SPLASH_STORAGE_RAW,
		.స్థానభ్రంశం = 0x0,
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

#నిర్వచితమైతే CONFIG_VIDEO_LOGO

#అన్తర్భావ <bmp_logo_data.h>

స్థిర పూర్ణాంకం splash_video_logo_ఎక్కించు(శూన్య)
{
	అక్షరం *splashimage;
	ulong bmp_ఎక్కించు_addr;

	splashimage = env_get("splashimage");
	ఒకవేళ (!splashimage)
		తిరిగి -ENOENT;

	bmp_ఎక్కించు_addr = hextoul(splashimage, 0);
	ఒకవేళ (!bmp_ఎక్కించు_addr) {
		రాయి("Error: bad 'splashimage' address\n");
		తిరిగి -EFAULT;
	}

	జ్ఞాపకం_నకలు((శూన్య *)bmp_ఎక్కించు_addr, bmp_logo_bitmap,
	       ARRAY_కొలత(bmp_logo_bitmap));

	తిరిగి 0;
}
#లేకపోతే
స్థిర అంతఃవరుస పూర్ణాంకం splash_video_logo_ఎక్కించు(శూన్య) { తిరిగి -ENOSYS; }
#అయితే_ముగింపు

__weak పూర్ణాంకం splash_screen_prepare(శూన్య)
{
	ఒకవేళ (CONFIG_IS_ENABLED(SPLASH_ఆధారం))
		తిరిగి splash_ఆధారం_ఎక్కించు(default_splash_locations,
					  ARRAY_కొలత(default_splash_locations));

	తిరిగి splash_video_logo_ఎక్కించు();
}

శూన్య splash_get_pos(పూర్ణాంకం *x, పూర్ణాంకం *y)
{
	అక్షరం *s = env_get("splashpos");

	ఒకవేళ (!CONFIG_IS_ENABLED(SPLASH_SCREEN_ALIGN) || !s)
		తిరిగి;

	ఒకవేళ (s[0] == 'm')
		*x = BMP_ALIGN_CENTER;
	లేకపోతే
		*x = simple_strtol(s, ఖాళీ, 0);

	s = strchr(s + 1, ',');
	ఒకవేళ (s != ఖాళీ) {
		ఒకవేళ (s[1] == 'm')
			*y = BMP_ALIGN_CENTER;
		లేకపోతే
			*y = simple_strtol(s + 1, ఖాళీ, 0);
	}
}

#ఒకవేళ CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_సంస్కరణ)

#నిర్వచితమైతే CONFIG_VIDEO_LOGO
#అన్తర్భావ <bmp_logo.h>
#అయితే_ముగింపు
#అన్తర్భావ <dm.h>
#అన్తర్భావ <video_console.h>
#అన్తర్భావ <video_font.h>
#అన్తర్భావ <video_font_data.h>

శూన్య splash_display_banner(శూన్య)
{
	నిర్మాణం video_fontdata __maybe_unused *fontdata = fonts;
	నిర్మాణం udevice *dev;
	అక్షరం buf[DISPLAY_OPTIONS_BANNER_నిడివి];
	పూర్ణాంకం నిలువు, అడ్డువరుస, ret;

	ret = uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &dev);
	ఒకవేళ (ret)
		తిరిగి;

#ఒకవేళ IS_ENABLED(CONFIG_VIDEO_LOGO)
	నిలువు = BMP_LOGO_వెడల్పు / fontdata->వెడల్పు + 1;
	అడ్డువరుస = BMP_LOGO_ఎత్తు / fontdata->ఎత్తు + 1;
#లేకపోతే
	నిలువు = 0;
	అడ్డువరుస = 0;
#అయితే_ముగింపు

	display_options_get_banner(false, buf, పరిమాణం(buf));
	vidconsole_position_cursor(dev, నిలువు, 1);
	vidconsole_put_string(dev, buf);
	vidconsole_position_cursor(dev, 0, అడ్డువరుస);
}
#అయితే_ముగింపు /* CONFIG_VIDEO && !CONFIG_HIDE_LOGO_VERSION */

/*
 * Common function to show a splash image if env("splashimage") is set.
 * For additional details please refer to doc/README.splashprepare.
 */
పూర్ణాంకం splash_display(శూన్య)
{
	ulong addr;
	అక్షరం *s;
	పూర్ణాంకం x = 0, y = 0, ret;
	ఒకవేళ (!CONFIG_IS_ENABLED(SPLASH_SCREEN))
		తిరిగి -ENOSYS;
	s = env_get("splashimage");
	ఒకవేళ (!s)
		తిరిగి -EINVAL;

	addr = hextoul(s, ఖాళీ);
	ret = splash_screen_prepare();
	ఒకవేళ (ret)
		తిరిగి ret;

	splash_get_pos(&x, &y);

	ఒకవేళ (CONFIG_IS_ENABLED(BMP))
		ret = bmp_display(addr, x, y);
	లేకపోతే
		తిరిగి -ENOSYS;

	/* Skip banner output on video console if the logo is not at 0,0 */
	ఒకవేళ (x || y)
		వెళ్ళు చివర_2;

#ఒకవేళ CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_సంస్కరణ)
	splash_display_banner();
#అయితే_ముగింపు
చివర_2:
	తిరిగి ret;
}
