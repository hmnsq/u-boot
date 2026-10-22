// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#অন্তর্ভাব <display_options.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <malloc.h>
#অন্তর্ভাব <asm/global_data.h>
#অন্তর্ভাব <dm/uclass.h>

DECLARE_GLOBAL_DATA_PTR;

স্থির ধ্রুব বর্ণ *ধ্রুব log_cat_name[] = {
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

_Static_assert(ARRAY_SIZE(log_cat_name) == LOGC_গুনতি - LOGC_NONE,
	       "log_cat_name size");

স্থির ধ্রুব বর্ণ *ধ্রুব log_level_name[] = {
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

_Static_assert(ARRAY_SIZE(log_level_name) == LOGL_গুনতি, "log_level_name size");

/* All error responses MUST begin with '<' */
ধ্রুব বর্ণ *log_get_cat_name(গণনা log_category_t cat)
{
	ধ্রুব বর্ণ *name;

	যদি (cat < 0 || cat >= LOGC_গুনতি)
		প্রত্যাগম "<invalid>";
	যদি (cat >= LOGC_NONE)
		প্রত্যাগম log_cat_name[cat - LOGC_NONE];

#যদি CONFIG_IS_ENABLED(DM)
	name = uclass_get_name((গণনা uclass_id)cat);
#অন্যথা
	name = রিক্ত;
#endif

	প্রত্যাগম name ? name : "<missing>";
}

গণনা log_category_t log_get_cat_by_name(ধ্রুব বর্ণ *name)
{
	গণনা uclass_id id;
	পূর্ণাঙ্ক i;

	প্রতি (i = LOGC_NONE; i < LOGC_গুনতি; i++)
		যদি (!strcmp(name, log_cat_name[i - LOGC_NONE]))
			প্রত্যাগম i;
	id = uclass_get_by_name(name);
	যদি (id != UCLASS_INVALID)
		প্রত্যাগম (গণনা log_category_t)id;

	প্রত্যাগম LOGC_NONE;
}

ধ্রুব বর্ণ *log_get_level_name(গণনা log_level_t level)
{
	যদি (level >= LOGL_গুনতি)
		প্রত্যাগম "INVALID";
	প্রত্যাগম log_level_name[level];
}

গণনা log_level_t log_get_level_by_name(ধ্রুব বর্ণ *name)
{
	পূর্ণাঙ্ক i;

	প্রতি (i = 0; i < LOGL_গুনতি; i++) {
		যদি (!strcasecmp(log_level_name[i], name))
			প্রত্যাগম i;
	}

	প্রত্যাগম LOGL_NONE;
}

সংরচনা log_device *log_device_find_by_name(ধ্রুব বর্ণ *drv_name)
{
	সংরচনা log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_গ্রন্থি) {
		যদি (!strcmp(drv_name, ldev->drv->name))
			প্রত্যাগম ldev;
	}

	প্রত্যাগম রিক্ত;
}

bool log_has_cat(গণনা log_category_t cat_list[], গণনা log_category_t cat)
{
	পূর্ণাঙ্ক i;

	প্রতি (i = 0; i < LOGF_MAX_CATEGORIES && cat_list[i] != LOGC_END; i++) {
		যদি (cat_list[i] == cat)
			প্রত্যাগম true;
	}

	প্রত্যাগম false;
}

/**
 * log_has_member() - check if a string is in a comma separated list
 *
 * @list:	Comma separated list of strings
 * @member:	String to find
 *
 * Return: ``true`` if @member is in @list, else ``false``
 */
স্থির bool log_has_member(ধ্রুব বর্ণ *list, ধ্রুব বর্ণ *member)
{
	পূর্ণাঙ্ক member_len = strlen(member);
	ধ্রুব বর্ণ *s, *p;
	পূর্ণাঙ্ক substr_len;

	প্রতি (s = list; *s; s = p + (*p != '\0')) {
		p = strchrnul(s, ',');
		substr_len = p - s;
		যদি (member_len >= substr_len &&
		    !strncmp(member + member_len - substr_len, s, substr_len))
			প্রত্যাগম true;
	}

	প্রত্যাগম false;
}

/**
 * log_passes_filters() - check if a log record passes the filters for a device
 *
 * @ldev: Log device to check
 * @rec: Log record to check
 * Return: true if @rec is not blocked by the filters in @ldev, false if it is
 */
স্থির bool log_passes_filters(সংরচনা log_device *ldev, সংরচনা log_rec *rec)
{
	সংরচনা log_filter *filt;

	যদি (rec->flags & LOGRECF_FORCE_DEBUG)
		প্রত্যাগম true;

	/* If there are no filters, filter on the default log level */
	যদি (list_empty(&ldev->filter_head)) {
		যদি (rec->level > gd->default_log_level)
			প্রত্যাগম false;
		প্রত্যাগম true;
	}

	list_for_each_entry(filt, &ldev->filter_head, sibling_গ্রন্থি) {
		যদি (filt->flags & LOGFF_LEVEL_MIN) {
			যদি (rec->level < filt->level)
				অনুবর্ত;
		} অন্যথা যদি (rec->level > filt->level) {
			অনুবর্ত;
		}

		যদি ((filt->flags & LOGFF_HAS_CAT) &&
		    !log_has_cat(filt->cat_list, rec->cat))
			অনুবর্ত;

		যদি (filt->file_list &&
		    !log_has_member(filt->file_list, rec->file))
			অনুবর্ত;

		যদি (filt->func_list &&
		    !log_has_member(filt->func_list, rec->func))
			অনুবর্ত;

		যদি (filt->flags & LOGFF_DENY)
			প্রত্যাগম false;
		অন্যথা
			প্রত্যাগম true;
	}

	প্রত্যাগম false;
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
স্থির পূর্ণাঙ্ক log_dispatch(সংরচনা log_rec *rec, ধ্রুব বর্ণ *fmt, va_list args)
{
	সংরচনা log_device *ldev;
	বর্ণ buf[CONFIG_SYS_CBSIZE];

	/*
	 * When a log driver writes messages (e.g. via the network stack) this
	 * may result in further generated messages. We cannot process them here
	 * as this might result in infinite recursion.
	 */
	যদি (gd->processing_বার্তা)
		প্রত্যাগম 1;

	/* Emit message */
	gd->processing_বার্তা = true;
	list_for_each_entry(ldev, &gd->log_head, sibling_গ্রন্থি) {
		যদি ((ldev->flags & LOGDF_ENABLE) &&
		    log_passes_filters(ldev, rec)) {
			যদি (!rec->বার্তা) {
				পূর্ণাঙ্ক len;

				len = vsnprintf(buf, পরিমাণ(buf), fmt, args);
				rec->বার্তা = buf;
				gd->log_cont = len && buf[len - 1] != '\n';
			}
			ldev->drv->emit(ldev, rec);
		}
	}
	gd->processing_বার্তা = false;
	প্রত্যাগম 0;
}

পূর্ণাঙ্ক _log(গণনা log_category_t cat, গণনা log_level_t level, ধ্রুব বর্ণ *file,
	 পূর্ণাঙ্ক সারি, ধ্রুব বর্ণ *func, ধ্রুব বর্ণ *fmt, ...)
{
	সংরচনা log_rec rec;
	va_list args;

	যদি (!gd)
		প্রত্যাগম -ENOSYS;

	/* Check for message continuation */
	যদি (cat == LOGC_CONT)
		cat = gd->logc_prev;
	যদি (level == LOGL_CONT)
		level = gd->logl_prev;

	rec.cat = cat;
	rec.level = level & LOGL_LEVEL_MASK;
	rec.flags = 0;
	যদি (level & LOGL_FORCE_DEBUG)
		rec.flags |= LOGRECF_FORCE_DEBUG;
	যদি (gd->log_cont)
		rec.flags |= LOGRECF_CONT;
	rec.file = file;
	rec.সারি = সারি;
	rec.func = func;
	rec.বার্তা = রিক্ত;

	যদি (!(gd->flags & GD_FLG_LOG_READY)) {
		gd->log_drop_গুনতি++;

		/* display dropped traces with console puts and DEBUG_UART */
		যদি (rec.level <= CONFIG_LOG_DEFAULT_LEVEL ||
		    rec.flags & LOGRECF_FORCE_DEBUG) {
			বর্ণ buf[CONFIG_SYS_CBSIZE];

			va_start(args, fmt);
			vsnprintf(buf, পরিমাণ(buf), fmt, args);
			puts(buf);
			va_end(args);
		}

		প্রত্যাগম -ENOSYS;
	}
	va_start(args, fmt);
	যদি (!log_dispatch(&rec, fmt, args)) {
		gd->logc_prev = cat;
		gd->logl_prev = level;
	}
	va_end(args);

	প্রত্যাগম 0;
}

#define MAX_সারি_LENGTH_BYTES		64
#define DEFAULT_সারি_LENGTH_BYTES	16

পূর্ণাঙ্ক _log_buffer(গণনা log_category_t cat, গণনা log_level_t level,
		ধ্রুব বর্ণ *file, পূর্ণাঙ্ক সারি, ধ্রুব বর্ণ *func, ulong addr,
		ধ্রুব শূন্য *data, uint width, uint গুনতি, uint linelen)
{
	যদি (linelen * width > MAX_সারি_LENGTH_BYTES)
		linelen = MAX_সারি_LENGTH_BYTES / width;
	যদি (linelen < 1)
		linelen = DEFAULT_সারি_LENGTH_BYTES / width;

	যাবত্ (গুনতি) {
		uint thislinelen;
		বর্ণ buf[HEXDUMP_MAX_BUF_LENGTH(width * linelen)];

		thislinelen = hexdump_সারি(addr, data, width, গুনতি, linelen,
					   buf, পরিমাণ(buf));
		প্রতিজ্ঞা(thislinelen >= 0);
		_log(cat, level, file, সারি, func, "%s\n", buf);

		/* update references */
		data += thislinelen * width;
		addr += thislinelen * width;
		গুনতি -= thislinelen;
	}

	প্রত্যাগম 0;
}

পূর্ণাঙ্ক log_add_filter_flags(ধ্রুব বর্ণ *drv_name, গণনা log_category_t cat_list[],
			 গণনা log_level_t level, ধ্রুব বর্ণ *file_list,
			 ধ্রুব বর্ণ *func_list, পূর্ণাঙ্ক flags)
{
	সংরচনা log_filter *filt;
	সংরচনা log_device *ldev;
	পূর্ণাঙ্ক ret;
	পূর্ণাঙ্ক i;

	ldev = log_device_find_by_name(drv_name);
	যদি (!ldev)
		প্রত্যাগম -ENOENT;
	filt = calloc(1, পরিমাণ(*filt));
	যদি (!filt)
		প্রত্যাগম -ENOMEM;

	filt->flags = flags;
	যদি (cat_list) {
		filt->flags |= LOGFF_HAS_CAT;
		প্রতি (i = 0; ; i++) {
			যদি (i == ARRAY_SIZE(filt->cat_list)) {
				ret = -ENOSPC;
				গচ্ছ err;
			}
			filt->cat_list[i] = cat_list[i];
			যদি (cat_list[i] == LOGC_END)
				ভঙ্গ;
		}
	}
	filt->level = level;
	যদি (file_list) {
		filt->file_list = strdup(file_list);
		যদি (!filt->file_list) {
			ret = -ENOMEM;
			গচ্ছ err;
		}
	}
	যদি (func_list) {
		filt->func_list = strdup(func_list);
		যদি (!filt->func_list) {
			ret = -ENOMEM;
			গচ্ছ err;
		}
	}
	filt->filter_অঙ্ক = ldev->next_filter_অঙ্ক++;
	/* Add deny filters to the beginning of the list */
	যদি (flags & LOGFF_DENY)
		list_add(&filt->sibling_গ্রন্থি, &ldev->filter_head);
	অন্যথা
		list_add_tail(&filt->sibling_গ্রন্থি, &ldev->filter_head);

	প্রত্যাগম filt->filter_অঙ্ক;

err:
	মুক্ত(filt);
	প্রত্যাগম ret;
}

পূর্ণাঙ্ক log_remove_filter(ধ্রুব বর্ণ *drv_name, পূর্ণাঙ্ক filter_অঙ্ক)
{
	সংরচনা log_filter *filt;
	সংরচনা log_device *ldev;

	ldev = log_device_find_by_name(drv_name);
	যদি (!ldev)
		প্রত্যাগম -ENOENT;

	list_for_each_entry(filt, &ldev->filter_head, sibling_গ্রন্থি) {
		যদি (filt->filter_অঙ্ক == filter_অঙ্ক) {
			list_del(&filt->sibling_গ্রন্থি);
			মুক্ত(filt);

			প্রত্যাগম 0;
		}
	}

	প্রত্যাগম -ENOENT;
}

/**
 * log_find_device_by_drv() - Find a device by its driver
 *
 * @drv: Log driver
 * Return: Device associated with that driver, or NULL if not found
 */
স্থির সংরচনা log_device *log_find_device_by_drv(সংরচনা log_driver *drv)
{
	সংরচনা log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_গ্রন্থি) {
		যদি (ldev->drv == drv)
			প্রত্যাগম ldev;
	}
	/*
	 * It is quite hard to pass an invalid driver since passing an unknown
	 * LOG_GET_DRIVER(xxx) would normally produce a compilation error. But
	 * it is possible to pass NULL, for example, so this
	 */

	প্রত্যাগম রিক্ত;
}

পূর্ণাঙ্ক log_device_set_enable(সংরচনা log_driver *drv, bool enable)
{
	সংরচনা log_device *ldev;

	ldev = log_find_device_by_drv(drv);
	যদি (!ldev)
		প্রত্যাগম -ENOENT;
	যদি (enable)
		ldev->flags |= LOGDF_ENABLE;
	অন্যথা
		ldev->flags &= ~LOGDF_ENABLE;

	প্রত্যাগম 0;
}

শূন্য log_fixup_for_gd_move(সংরচনা global_data *new_gd)
{
	new_gd->log_head.prev->next = &new_gd->log_head;
}

পূর্ণাঙ্ক log_init(শূন্য)
{
	সংরচনা log_driver *drv = ll_entry_start(সংরচনা log_driver, log_driver);
	ধ্রুব পূর্ণাঙ্ক গুনতি = ll_entry_গুনতি(সংরচনা log_driver, log_driver);
	সংরচনা log_driver *end = drv + গুনতি;

	/*
	 * We cannot add runtime data to the driver since it is likely stored
	 * in rodata. Instead, set up a 'device' corresponding to each driver.
	 * We only support having a single device for each driver.
	 */
	INIT_LIST_HEAD((সংরচনা list_head *)&gd->log_head);
	যাবত্ (drv < end) {
		সংরচনা log_device *ldev;

		ldev = calloc(1, পরিমাণ(*ldev));
		যদি (!ldev) {
			debug("%s: Cannot allocate memory\n", __func__);
			প্রত্যাগম -ENOMEM;
		}
		INIT_LIST_HEAD(&ldev->filter_head);
		ldev->drv = drv;
		ldev->flags = drv->flags;
		list_add_tail(&ldev->sibling_গ্রন্থি,
			      (সংরচনা list_head *)&gd->log_head);
		drv++;
	}
	gd->flags |= GD_FLG_LOG_READY;
	যদি (!gd->default_log_level)
		gd->default_log_level = CONFIG_LOG_DEFAULT_LEVEL;
	gd->log_fmt = log_get_default_format();
	gd->logc_prev = LOGC_NONE;
	gd->logl_prev = LOGL_INFO;

	প্রত্যাগম 0;
}
