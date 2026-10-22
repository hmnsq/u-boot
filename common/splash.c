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

#समावेश <display_options.h>
#समावेश <env.h>
#समावेश <splash.h>
#समावेश <video.h>
#समावेश <vsprintf.h>
#समावेश <linux/kernel.h>

अटल काष्ठा splash_location default_splash_locations[] = {
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

#अगर_घोषित CONFIG_VIDEO_LOGO

#समावेश <bmp_logo_data.h>

अटल पूर्णांक splash_video_logo_load(व्योम)
{
	अक्षर *splashimage;
	ulong bmp_load_addr;

	splashimage = env_get("splashimage");
	अगर (!splashimage)
		वापस -ENOENT;

	bmp_load_addr = hextoul(splashimage, 0);
	अगर (!bmp_load_addr) {
		म_लिखो("Error: bad 'splashimage' address\n");
		वापस -EFAULT;
	}

	स_नकल((व्योम *)bmp_load_addr, bmp_logo_bitmap,
	       ARRAY_आकार(bmp_logo_bitmap));

	वापस 0;
}
#अन्यथा
अटल अंतरभूत पूर्णांक splash_video_logo_load(व्योम) { वापस -ENOSYS; }
#पूर्ण_अगर

__weak पूर्णांक splash_screen_prepare(व्योम)
{
	अगर (CONFIG_IS_ENABLED(SPLASH_SOURCE))
		वापस splash_source_load(default_splash_locations,
					  ARRAY_आकार(default_splash_locations));

	वापस splash_video_logo_load();
}

व्योम splash_get_pos(पूर्णांक *x, पूर्णांक *y)
{
	अक्षर *s = env_get("splashpos");

	अगर (!CONFIG_IS_ENABLED(SPLASH_SCREEN_ALIGN) || !s)
		वापस;

	अगर (s[0] == 'm')
		*x = BMP_ALIGN_CENTER;
	अन्यथा
		*x = simple_strtol(s, शून्य, 0);

	s = म_अक्षर(s + 1, ',');
	अगर (s != शून्य) {
		अगर (s[1] == 'm')
			*y = BMP_ALIGN_CENTER;
		अन्यथा
			*y = simple_strtol(s + 1, शून्य, 0);
	}
}

#अगर CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)

#अगर_घोषित CONFIG_VIDEO_LOGO
#समावेश <bmp_logo.h>
#पूर्ण_अगर
#समावेश <dm.h>
#समावेश <video_console.h>
#समावेश <video_font.h>
#समावेश <video_font_data.h>

व्योम splash_display_banner(व्योम)
{
	काष्ठा video_fontdata __maybe_unused *fontdata = fonts;
	काष्ठा udevice *dev;
	अक्षर buf[DISPLAY_OPTIONS_BANNER_लंबाई];
	पूर्णांक col, row, ret;

	ret = uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &dev);
	अगर (ret)
		वापस;

#अगर IS_ENABLED(CONFIG_VIDEO_LOGO)
	col = BMP_LOGO_WIDTH / fontdata->width + 1;
	row = BMP_LOGO_HEIGHT / fontdata->height + 1;
#अन्यथा
	col = 0;
	row = 0;
#पूर्ण_अगर

	display_options_get_banner(false, buf, माप(buf));
	vidconsole_position_cursor(dev, col, 1);
	vidconsole_put_string(dev, buf);
	vidconsole_position_cursor(dev, 0, row);
}
#पूर्ण_अगर /* CONFIG_VIDEO && !CONFIG_HIDE_LOGO_VERSION */

/*
 * Common function to show a splash image if env("splashimage") is set.
 * For additional details please refer to doc/README.splashprepare.
 */
पूर्णांक splash_display(व्योम)
{
	ulong addr;
	अक्षर *s;
	पूर्णांक x = 0, y = 0, ret;
	अगर (!CONFIG_IS_ENABLED(SPLASH_SCREEN))
		वापस -ENOSYS;
	s = env_get("splashimage");
	अगर (!s)
		वापस -EINVAL;

	addr = hextoul(s, शून्य);
	ret = splash_screen_prepare();
	अगर (ret)
		वापस ret;

	splash_get_pos(&x, &y);

	अगर (CONFIG_IS_ENABLED(BMP))
		ret = bmp_display(addr, x, y);
	अन्यथा
		वापस -ENOSYS;

	/* Skip banner output on video console if the logo is not at 0,0 */
	अगर (x || y)
		जाओ end;

#अगर CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)
	splash_display_banner();
#पूर्ण_अगर
end:
	वापस ret;
}
