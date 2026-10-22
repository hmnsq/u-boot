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

#अन्तर्भाव <display_options.h>
#अन्तर्भाव <env.h>
#अन्तर्भाव <splash.h>
#अन्तर्भाव <video.h>
#अन्तर्भाव <vsprintf.h>
#अन्तर्भाव <linux/kernel.h>

स्थिर संरचना splash_location default_splash_locations[] = {
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

#अन्तर्भाव <bmp_logo_data.h>

स्थिर पूर्णांक splash_video_logo_load(शून्य)
{
	अक्षर *splashimage;
	ulong bmp_load_addr;

	splashimage = env_get("splashimage");
	जर (!splashimage)
		परत -ENOENT;

	bmp_load_addr = hextoul(splashimage, 0);
	जर (!bmp_load_addr) {
		म_लिखो("Error: bad 'splashimage' address\n");
		परत -EFAULT;
	}

	स_नकल((शून्य *)bmp_load_addr, bmp_logo_bitmap,
	       ARRAY_SIZE(bmp_logo_bitmap));

	परत 0;
}
#अन्यथा
स्थिर ओळीत पूर्णांक splash_video_logo_load(शून्य) { परत -ENOSYS; }
#endif

__weak पूर्णांक splash_screen_prepare(शून्य)
{
	जर (CONFIG_IS_ENABLED(SPLASH_SOURCE))
		परत splash_source_load(default_splash_locations,
					  ARRAY_SIZE(default_splash_locations));

	परत splash_video_logo_load();
}

शून्य splash_get_pos(पूर्णांक *x, पूर्णांक *y)
{
	अक्षर *s = env_get("splashpos");

	जर (!CONFIG_IS_ENABLED(SPLASH_SCREEN_ALIGN) || !s)
		परत;

	जर (s[0] == 'm')
		*x = BMP_ALIGN_CENTER;
	अन्यथा
		*x = simple_strtol(s, रिक्त, 0);

	s = म_अक्षर(s + 1, ',');
	जर (s != रिक्त) {
		जर (s[1] == 'm')
			*y = BMP_ALIGN_CENTER;
		अन्यथा
			*y = simple_strtol(s + 1, रिक्त, 0);
	}
}

#जर CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)

#ifdef CONFIG_VIDEO_LOGO
#अन्तर्भाव <bmp_logo.h>
#endif
#अन्तर्भाव <dm.h>
#अन्तर्भाव <video_console.h>
#अन्तर्भाव <video_font.h>
#अन्तर्भाव <video_font_data.h>

शून्य splash_display_banner(शून्य)
{
	संरचना video_fontdata __maybe_unused *fontdata = fonts;
	संरचना udevice *dev;
	अक्षर buf[DISPLAY_OPTIONS_BANNER_LENGTH];
	पूर्णांक col, row, ret;

	ret = uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &dev);
	जर (ret)
		परत;

#जर IS_ENABLED(CONFIG_VIDEO_LOGO)
	col = BMP_LOGO_WIDTH / fontdata->width + 1;
	row = BMP_LOGO_HEIGHT / fontdata->height + 1;
#अन्यथा
	col = 0;
	row = 0;
#endif

	display_options_get_banner(false, buf, आकार(buf));
	vidconsole_position_cursor(dev, col, 1);
	vidconsole_put_string(dev, buf);
	vidconsole_position_cursor(dev, 0, row);
}
#endif /* CONFIG_VIDEO && !CONFIG_HIDE_LOGO_VERSION */

/*
 * Common function to show a splash image if env("splashimage") is set.
 * For additional details please refer to doc/README.splashprepare.
 */
पूर्णांक splash_display(शून्य)
{
	ulong addr;
	अक्षर *s;
	पूर्णांक x = 0, y = 0, ret;
	जर (!CONFIG_IS_ENABLED(SPLASH_SCREEN))
		परत -ENOSYS;
	s = env_get("splashimage");
	जर (!s)
		परत -EINVAL;

	addr = hextoul(s, रिक्त);
	ret = splash_screen_prepare();
	जर (ret)
		परत ret;

	splash_get_pos(&x, &y);

	जर (CONFIG_IS_ENABLED(BMP))
		ret = bmp_display(addr, x, y);
	अन्यथा
		परत -ENOSYS;

	/* Skip banner output on video console if the logo is not at 0,0 */
	जर (x || y)
		जा end;

#जर CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)
	splash_display_banner();
#endif
end:
	परत ret;
}
