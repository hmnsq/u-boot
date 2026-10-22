// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#അന്തര്ഭാവ <display_options.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <malloc.h>
#അന്തര്ഭാവ <asm/global_data.h>
#അന്തര്ഭാവ <dm/uclass.h>

DECLARE_GLOBAL_DATA_PTR;

സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം log_cat_name[] = {
	"none",
	"arch",
	"board",
	"core",
	"driver-model",
	"device-tree",
	"efi",
	"alloc",
	"sandbox",
	"bloblist",
	"devres",
	"acpi",
	"boot",
	"event",
	"fs",
	"expo",
	"console",
	"test",
};

_Static_assert(ARRAY_SIZE(log_cat_name) == LOGC_എണ്ണം - LOGC_NONE,
	       "log_cat_name size");

സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം log_level_name[] = {
	"EMERG",
	"ALERT",
	"CRIT",
	"ERR",
	"WARNING",
	"NOTICE",
	"INFO",
	"DEBUG",
	"CONTENT",
	"IO",
};

_Static_assert(ARRAY_SIZE(log_level_name) == LOGL_എണ്ണം, "log_level_name size");

/* All error responses MUST begin with '<' */
സ്ഥിരം അക്ഷരം *log_get_cat_name(എണ്ണൽ log_category_t cat)
{
	സ്ഥിരം അക്ഷരം *name;

	എങ്കിൽ (cat < 0 || cat >= LOGC_എണ്ണം)
		തിരികെ "<invalid>";
	എങ്കിൽ (cat >= LOGC_NONE)
		തിരികെ log_cat_name[cat - LOGC_NONE];

#എങ്കിൽ CONFIG_IS_ENABLED(DM)
	name = uclass_get_name((എണ്ണൽ uclass_id)cat);
#അല്ലെങ്കിൽ
	name = ശൂന്യ;
#endif

	തിരികെ name ? name : "<missing>";
}

എണ്ണൽ log_category_t log_get_cat_by_name(സ്ഥിരം അക്ഷരം *name)
{
	എണ്ണൽ uclass_id id;
	പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = LOGC_NONE; i < LOGC_എണ്ണം; i++)
		എങ്കിൽ (!strcmp(name, log_cat_name[i - LOGC_NONE]))
			തിരികെ i;
	id = uclass_get_by_name(name);
	എങ്കിൽ (id != UCLASS_INVALID)
		തിരികെ (എണ്ണൽ log_category_t)id;

	തിരികെ LOGC_NONE;
}

സ്ഥിരം അക്ഷരം *log_get_level_name(എണ്ണൽ log_level_t level)
{
	എങ്കിൽ (level >= LOGL_എണ്ണം)
		തിരികെ "INVALID";
	തിരികെ log_level_name[level];
}

എണ്ണൽ log_level_t log_get_level_by_name(സ്ഥിരം അക്ഷരം *name)
{
	പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = 0; i < LOGL_എണ്ണം; i++) {
		എങ്കിൽ (!strcasecmp(log_level_name[i], name))
			തിരികെ i;
	}

	തിരികെ LOGL_NONE;
}

ഘടന log_device *log_device_find_by_name(സ്ഥിരം അക്ഷരം *drv_name)
{
	ഘടന log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_കണ്ണി) {
		എങ്കിൽ (!strcmp(drv_name, ldev->drv->name))
			തിരികെ ldev;
	}

	തിരികെ ശൂന്യ;
}

bool log_has_cat(എണ്ണൽ log_category_t cat_list[], എണ്ണൽ log_category_t cat)
{
	പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = 0; i < LOGF_MAX_CATEGORIES && cat_list[i] != LOGC_END; i++) {
		എങ്കിൽ (cat_list[i] == cat)
			തിരികെ true;
	}

	തിരികെ false;
}

/**
 * log_has_member() - check if a string is in a comma separated list
 *
 * @list:	Comma separated list of strings
 * @member:	String to find
 *
 * Return: ``true`` if @member is in @list, else ``false``
 */
സ്ഥിതം bool log_has_member(സ്ഥിരം അക്ഷരം *list, സ്ഥിരം അക്ഷരം *member)
{
	പൂർണ്ണസംഖ്യ member_len = strlen(member);
	സ്ഥിരം അക്ഷരം *s, *p;
	പൂർണ്ണസംഖ്യ substr_len;

	വേണ്ടി (s = list; *s; s = p + (*p != '\0')) {
		p = strchrnul(s, ',');
		substr_len = p - s;
		എങ്കിൽ (member_len >= substr_len &&
		    !strncmp(member + member_len - substr_len, s, substr_len))
			തിരികെ true;
	}

	തിരികെ false;
}

/**
 * log_passes_filters() - check if a log record passes the filters for a device
 *
 * @ldev: Log device to check
 * @rec: Log record to check
 * Return: true if @rec is not blocked by the filters in @ldev, false if it is
 */
സ്ഥിതം bool log_passes_filters(ഘടന log_device *ldev, ഘടന log_rec *rec)
{
	ഘടന log_filter *filt;

	എങ്കിൽ (rec->flags & LOGRECF_FORCE_DEBUG)
		തിരികെ true;

	/* If there are no filters, filter on the default log level */
	എങ്കിൽ (list_empty(&ldev->filter_head)) {
		എങ്കിൽ (rec->level > gd->default_log_level)
			തിരികെ false;
		തിരികെ true;
	}

	list_for_each_entry(filt, &ldev->filter_head, sibling_കണ്ണി) {
		എങ്കിൽ (filt->flags & LOGFF_LEVEL_MIN) {
			എങ്കിൽ (rec->level < filt->level)
				തുടരുക;
		} അല്ലെങ്കിൽ എങ്കിൽ (rec->level > filt->level) {
			തുടരുക;
		}

		എങ്കിൽ ((filt->flags & LOGFF_HAS_CAT) &&
		    !log_has_cat(filt->cat_list, rec->cat))
			തുടരുക;

		എങ്കിൽ (filt->file_list &&
		    !log_has_member(filt->file_list, rec->file))
			തുടരുക;

		എങ്കിൽ (filt->func_list &&
		    !log_has_member(filt->func_list, rec->func))
			തുടരുക;

		എങ്കിൽ (filt->flags & LOGFF_DENY)
			തിരികെ false;
		അല്ലെങ്കിൽ
			തിരികെ true;
	}

	തിരികെ false;
}

/**
 * log_dispatch() - Send a log record to all log devices for processing
 *
 * The log record is sent to each log device in turn, skipping those which have
 * filters which block the record.
 *
 * All log messages created while processing log record @rec are ignored.
 *
 * @rec:	log record to dispatch
 * Return:	0 msg sent, 1 msg not sent while already dispatching another msg
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ log_dispatch(ഘടന log_rec *rec, സ്ഥിരം അക്ഷരം *fmt, va_list args)
{
	ഘടന log_device *ldev;
	അക്ഷരം buf[CONFIG_SYS_CBSIZE];

	/*
	 * When a log driver writes messages (e.g. via the network stack) this
	 * may result in further generated messages. We cannot process them here
	 * as this might result in infinite recursion.
	 */
	എങ്കിൽ (gd->processing_സന്ദേശം)
		തിരികെ 1;

	/* Emit message */
	gd->processing_സന്ദേശം = true;
	list_for_each_entry(ldev, &gd->log_head, sibling_കണ്ണി) {
		എങ്കിൽ ((ldev->flags & LOGDF_ENABLE) &&
		    log_passes_filters(ldev, rec)) {
			എങ്കിൽ (!rec->സന്ദേശം) {
				പൂർണ്ണസംഖ്യ len;

				len = vsnprintf(buf, വലുപ്പം(buf), fmt, args);
				rec->സന്ദേശം = buf;
				gd->log_cont = len && buf[len - 1] != '\n';
			}
			ldev->drv->emit(ldev, rec);
		}
	}
	gd->processing_സന്ദേശം = false;
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ _log(എണ്ണൽ log_category_t cat, എണ്ണൽ log_level_t level, സ്ഥിരം അക്ഷരം *file,
	 പൂർണ്ണസംഖ്യ വരി, സ്ഥിരം അക്ഷരം *func, സ്ഥിരം അക്ഷരം *fmt, ...)
{
	ഘടന log_rec rec;
	va_list args;

	എങ്കിൽ (!gd)
		തിരികെ -ENOSYS;

	/* Check for message continuation */
	എങ്കിൽ (cat == LOGC_CONT)
		cat = gd->logc_prev;
	എങ്കിൽ (level == LOGL_CONT)
		level = gd->logl_prev;

	rec.cat = cat;
	rec.level = level & LOGL_LEVEL_MASK;
	rec.flags = 0;
	എങ്കിൽ (level & LOGL_FORCE_DEBUG)
		rec.flags |= LOGRECF_FORCE_DEBUG;
	എങ്കിൽ (gd->log_cont)
		rec.flags |= LOGRECF_CONT;
	rec.file = file;
	rec.വരി = വരി;
	rec.func = func;
	rec.സന്ദേശം = ശൂന്യ;

	എങ്കിൽ (!(gd->flags & GD_FLG_LOG_READY)) {
		gd->log_drop_എണ്ണം++;

		/* display dropped traces with console puts and DEBUG_UART */
		എങ്കിൽ (rec.level <= CONFIG_LOG_DEFAULT_LEVEL ||
		    rec.flags & LOGRECF_FORCE_DEBUG) {
			അക്ഷരം buf[CONFIG_SYS_CBSIZE];

			va_start(args, fmt);
			vsnprintf(buf, വലുപ്പം(buf), fmt, args);
			puts(buf);
			va_end(args);
		}

		തിരികെ -ENOSYS;
	}
	va_start(args, fmt);
	എങ്കിൽ (!log_dispatch(&rec, fmt, args)) {
		gd->logc_prev = cat;
		gd->logl_prev = level;
	}
	va_end(args);

	തിരികെ 0;
}

#define MAX_വരി_LENGTH_BYTES		64
#define DEFAULT_വരി_LENGTH_BYTES	16

പൂർണ്ണസംഖ്യ _log_buffer(എണ്ണൽ log_category_t cat, എണ്ണൽ log_level_t level,
		സ്ഥിരം അക്ഷരം *file, പൂർണ്ണസംഖ്യ വരി, സ്ഥിരം അക്ഷരം *func, ulong addr,
		സ്ഥിരം ശൂന്യം *data, uint width, uint എണ്ണം, uint linelen)
{
	എങ്കിൽ (linelen * width > MAX_വരി_LENGTH_BYTES)
		linelen = MAX_വരി_LENGTH_BYTES / width;
	എങ്കിൽ (linelen < 1)
		linelen = DEFAULT_വരി_LENGTH_BYTES / width;

	ആകുമ്പോൾ (എണ്ണം) {
		uint thislinelen;
		അക്ഷരം buf[HEXDUMP_MAX_BUF_LENGTH(width * linelen)];

		thislinelen = hexdump_വരി(addr, data, width, എണ്ണം, linelen,
					   buf, വലുപ്പം(buf));
		പ്രതിജ്ഞാ(thislinelen >= 0);
		_log(cat, level, file, വരി, func, "%s\n", buf);

		/* update references */
		data += thislinelen * width;
		addr += thislinelen * width;
		എണ്ണം -= thislinelen;
	}

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ log_add_filter_flags(സ്ഥിരം അക്ഷരം *drv_name, എണ്ണൽ log_category_t cat_list[],
			 എണ്ണൽ log_level_t level, സ്ഥിരം അക്ഷരം *file_list,
			 സ്ഥിരം അക്ഷരം *func_list, പൂർണ്ണസംഖ്യ flags)
{
	ഘടന log_filter *filt;
	ഘടന log_device *ldev;
	പൂർണ്ണസംഖ്യ ret;
	പൂർണ്ണസംഖ്യ i;

	ldev = log_device_find_by_name(drv_name);
	എങ്കിൽ (!ldev)
		തിരികെ -ENOENT;
	filt = calloc(1, വലുപ്പം(*filt));
	എങ്കിൽ (!filt)
		തിരികെ -ENOMEM;

	filt->flags = flags;
	എങ്കിൽ (cat_list) {
		filt->flags |= LOGFF_HAS_CAT;
		വേണ്ടി (i = 0; ; i++) {
			എങ്കിൽ (i == ARRAY_SIZE(filt->cat_list)) {
				ret = -ENOSPC;
				പോകുക err;
			}
			filt->cat_list[i] = cat_list[i];
			എങ്കിൽ (cat_list[i] == LOGC_END)
				നിർത്തുക;
		}
	}
	filt->level = level;
	എങ്കിൽ (file_list) {
		filt->file_list = strdup(file_list);
		എങ്കിൽ (!filt->file_list) {
			ret = -ENOMEM;
			പോകുക err;
		}
	}
	എങ്കിൽ (func_list) {
		filt->func_list = strdup(func_list);
		എങ്കിൽ (!filt->func_list) {
			ret = -ENOMEM;
			പോകുക err;
		}
	}
	filt->filter_അക്കം = ldev->next_filter_അക്കം++;
	/* Add deny filters to the beginning of the list */
	എങ്കിൽ (flags & LOGFF_DENY)
		list_add(&filt->sibling_കണ്ണി, &ldev->filter_head);
	അല്ലെങ്കിൽ
		list_add_tail(&filt->sibling_കണ്ണി, &ldev->filter_head);

	തിരികെ filt->filter_അക്കം;

err:
	സ്വതന്ത്രം(filt);
	തിരികെ ret;
}

പൂർണ്ണസംഖ്യ log_remove_filter(സ്ഥിരം അക്ഷരം *drv_name, പൂർണ്ണസംഖ്യ filter_അക്കം)
{
	ഘടന log_filter *filt;
	ഘടന log_device *ldev;

	ldev = log_device_find_by_name(drv_name);
	എങ്കിൽ (!ldev)
		തിരികെ -ENOENT;

	list_for_each_entry(filt, &ldev->filter_head, sibling_കണ്ണി) {
		എങ്കിൽ (filt->filter_അക്കം == filter_അക്കം) {
			list_del(&filt->sibling_കണ്ണി);
			സ്വതന്ത്രം(filt);

			തിരികെ 0;
		}
	}

	തിരികെ -ENOENT;
}

/**
 * log_find_device_by_drv() - Find a device by its driver
 *
 * @drv: Log driver
 * Return: Device associated with that driver, or NULL if not found
 */
സ്ഥിതം ഘടന log_device *log_find_device_by_drv(ഘടന log_driver *drv)
{
	ഘടന log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_കണ്ണി) {
		എങ്കിൽ (ldev->drv == drv)
			തിരികെ ldev;
	}
	/*
	 * It is quite hard to pass an invalid driver since passing an unknown
	 * LOG_GET_DRIVER(xxx) would normally produce a compilation error. But
	 * it is possible to pass NULL, for example, so this
	 */

	തിരികെ ശൂന്യ;
}

പൂർണ്ണസംഖ്യ log_device_set_enable(ഘടന log_driver *drv, bool enable)
{
	ഘടന log_device *ldev;

	ldev = log_find_device_by_drv(drv);
	എങ്കിൽ (!ldev)
		തിരികെ -ENOENT;
	എങ്കിൽ (enable)
		ldev->flags |= LOGDF_ENABLE;
	അല്ലെങ്കിൽ
		ldev->flags &= ~LOGDF_ENABLE;

	തിരികെ 0;
}

ശൂന്യം log_fixup_for_gd_move(ഘടന global_data *new_gd)
{
	new_gd->log_head.prev->next = &new_gd->log_head;
}

പൂർണ്ണസംഖ്യ log_init(ശൂന്യം)
{
	ഘടന log_driver *drv = ll_entry_start(ഘടന log_driver, log_driver);
	സ്ഥിരം പൂർണ്ണസംഖ്യ എണ്ണം = ll_entry_എണ്ണം(ഘടന log_driver, log_driver);
	ഘടന log_driver *end = drv + എണ്ണം;

	/*
	 * We cannot add runtime data to the driver since it is likely stored
	 * in rodata. Instead, set up a 'device' corresponding to each driver.
	 * We only support having a single device for each driver.
	 */
	INIT_LIST_HEAD((ഘടന list_head *)&gd->log_head);
	ആകുമ്പോൾ (drv < end) {
		ഘടന log_device *ldev;

		ldev = calloc(1, വലുപ്പം(*ldev));
		എങ്കിൽ (!ldev) {
			debug("%s: Cannot allocate memory\n", __func__);
			തിരികെ -ENOMEM;
		}
		INIT_LIST_HEAD(&ldev->filter_head);
		ldev->drv = drv;
		ldev->flags = drv->flags;
		list_add_tail(&ldev->sibling_കണ്ണി,
			      (ഘടന list_head *)&gd->log_head);
		drv++;
	}
	gd->flags |= GD_FLG_LOG_READY;
	എങ്കിൽ (!gd->default_log_level)
		gd->default_log_level = CONFIG_LOG_DEFAULT_LEVEL;
	gd->log_fmt = log_get_default_format();
	gd->logc_prev = LOGC_NONE;
	gd->logl_prev = LOGL_INFO;

	തിരികെ 0;
}
