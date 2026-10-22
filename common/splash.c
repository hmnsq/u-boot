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

#અન્તર્ભાવ <display_options.h>
#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <splash.h>
#અન્તર્ભાવ <video.h>
#અન્તર્ભાવ <vsprintf.h>
#અન્તર્ભાવ <linux/kernel.h>

સ્થિર સંરચના splash_location default_splash_locations[] = {
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

#અન્તર્ભાવ <bmp_logo_data.h>

સ્થિર પૂર્ણાંક splash_video_logo_load(શૂન્ય)
{
	અક્ષર *splashimage;
	ulong bmp_load_addr;

	splashimage = env_get("splashimage");
	જો (!splashimage)
		પાછા -ENOENT;

	bmp_load_addr = hextoul(splashimage, 0);
	જો (!bmp_load_addr) {
		printf("Error: bad 'splashimage' address\n");
		પાછા -EFAULT;
	}

	memcpy((શૂન્ય *)bmp_load_addr, bmp_logo_bitmap,
	       ARRAY_SIZE(bmp_logo_bitmap));

	પાછા 0;
}
#નહીંતર
સ્થિર અંતઃરેખ પૂર્ણાંક splash_video_logo_load(શૂન્ય) { પાછા -ENOSYS; }
#endif

__weak પૂર્ણાંક splash_screen_prepare(શૂન્ય)
{
	જો (CONFIG_IS_ENABLED(SPLASH_SOURCE))
		પાછા splash_source_load(default_splash_locations,
					  ARRAY_SIZE(default_splash_locations));

	પાછા splash_video_logo_load();
}

શૂન્ય splash_get_pos(પૂર્ણાંક *x, પૂર્ણાંક *y)
{
	અક્ષર *s = env_get("splashpos");

	જો (!CONFIG_IS_ENABLED(SPLASH_SCREEN_ALIGN) || !s)
		પાછા;

	જો (s[0] == 'm')
		*x = BMP_ALIGN_CENTER;
	નહીંતર
		*x = simple_strtol(s, ખાલી, 0);

	s = strchr(s + 1, ',');
	જો (s != ખાલી) {
		જો (s[1] == 'm')
			*y = BMP_ALIGN_CENTER;
		નહીંતર
			*y = simple_strtol(s + 1, ખાલી, 0);
	}
}

#જો CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)

#ifdef CONFIG_VIDEO_LOGO
#અન્તર્ભાવ <bmp_logo.h>
#endif
#અન્તર્ભાવ <dm.h>
#અન્તર્ભાવ <video_console.h>
#અન્તર્ભાવ <video_font.h>
#અન્તર્ભાવ <video_font_data.h>

શૂન્ય splash_display_banner(શૂન્ય)
{
	સંરચના video_fontdata __maybe_unused *fontdata = fonts;
	સંરચના udevice *dev;
	અક્ષર buf[DISPLAY_OPTIONS_BANNER_LENGTH];
	પૂર્ણાંક col, row, ret;

	ret = uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &dev);
	જો (ret)
		પાછા;

#જો IS_ENABLED(CONFIG_VIDEO_LOGO)
	col = BMP_LOGO_WIDTH / fontdata->width + 1;
	row = BMP_LOGO_HEIGHT / fontdata->height + 1;
#નહીંતર
	col = 0;
	row = 0;
#endif

	display_options_get_banner(false, buf, કદ(buf));
	vidconsole_position_cursor(dev, col, 1);
	vidconsole_put_string(dev, buf);
	vidconsole_position_cursor(dev, 0, row);
}
#endif /* CONFIG_VIDEO && !CONFIG_HIDE_LOGO_VERSION */

/*
 * Common function to show a splash image if env("splashimage") is set.
 * For additional details please refer to doc/README.splashprepare.
 */
પૂર્ણાંક splash_display(શૂન્ય)
{
	ulong addr;
	અક્ષર *s;
	પૂર્ણાંક x = 0, y = 0, ret;
	જો (!CONFIG_IS_ENABLED(SPLASH_SCREEN))
		પાછા -ENOSYS;
	s = env_get("splashimage");
	જો (!s)
		પાછા -EINVAL;

	addr = hextoul(s, ખાલી);
	ret = splash_screen_prepare();
	જો (ret)
		પાછા ret;

	splash_get_pos(&x, &y);

	જો (CONFIG_IS_ENABLED(BMP))
		ret = bmp_display(addr, x, y);
	નહીંતર
		પાછા -ENOSYS;

	/* Skip banner output on video console if the logo is not at 0,0 */
	જો (x || y)
		જાઓ end;

#જો CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)
	splash_display_banner();
#endif
end:
	પાછા ret;
}
