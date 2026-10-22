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

DECLARE_GLOBAL_তথ্য_PTR;

স্থির ধ্রুবক অক্ষর *ধ্রুবক log_cat_name[] = {
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

_Static_assert(ARRAY_আকার(log_cat_name) == LOGC_গুনতি - LOGC_NONE,
	       "log_cat_name size");

স্থির ধ্রুবক অক্ষর *ধ্রুবক log_level_name[] = {
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

_Static_assert(ARRAY_আকার(log_level_name) == LOGL_গুনতি, "log_level_name size");

/* All error responses MUST begin with '<' */
ধ্রুবক অক্ষর *log_get_cat_name(গণনা log_category_t cat)
{
	ধ্রুবক অক্ষর *name;

	যদি (cat < 0 || cat >= LOGC_গুনতি)
		ফেরত "<invalid>";
	যদি (cat >= LOGC_NONE)
		ফেরত log_cat_name[cat - LOGC_NONE];

#যদি CONFIG_IS_ENABLED(DM)
	name = uclass_get_name((গণনা uclass_id)cat);
#অন্যথা
	name = ফাঁকা;
#endif

	ফেরত name ? name : "<missing>";
}

গণনা log_category_t log_get_cat_by_name(ধ্রুবক অক্ষর *name)
{
	গণনা uclass_id id;
	পূর্ণসংখ্যা i;

	জন্য (i = LOGC_NONE; i < LOGC_গুনতি; i++)
		যদি (!strcmp(name, log_cat_name[i - LOGC_NONE]))
			ফেরত i;
	id = uclass_get_by_name(name);
	যদি (id != UCLASS_INVALID)
		ফেরত (গণনা log_category_t)id;

	ফেরত LOGC_NONE;
}

ধ্রুবক অক্ষর *log_get_level_name(গণনা log_level_t level)
{
	যদি (level >= LOGL_গুনতি)
		ফেরত "INVALID";
	ফেরত log_level_name[level];
}

গণনা log_level_t log_get_level_by_name(ধ্রুবক অক্ষর *name)
{
	পূর্ণসংখ্যা i;

	জন্য (i = 0; i < LOGL_গুনতি; i++) {
		যদি (!strcasecmp(log_level_name[i], name))
			ফেরত i;
	}

	ফেরত LOGL_NONE;
}

কাঠামো log_device *log_device_find_by_name(ধ্রুবক অক্ষর *drv_name)
{
	কাঠামো log_device *ldev;

	list_for_each_entry(ldev, &gd->log_শীর্ষ, sibling_গ্রন্থি) {
		যদি (!strcmp(drv_name, ldev->drv->name))
			ফেরত ldev;
	}

	ফেরত ফাঁকা;
}

bool log_has_cat(গণনা log_category_t cat_list[], গণনা log_category_t cat)
{
	পূর্ণসংখ্যা i;

	জন্য (i = 0; i < LOGF_MAX_CATEGORIES && cat_list[i] != LOGC_END; i++) {
		যদি (cat_list[i] == cat)
			ফেরত true;
	}

	ফেরত false;
}

/**
 * log_has_member() - check if a string is in a comma separated list
 *
 * @list:	Comma separated list of strings
 * @member:	String to find
 *
 * Return: ``true`` if @member is in @list, else ``false``
 */
স্থির bool log_has_member(ধ্রুবক অক্ষর *list, ধ্রুবক অক্ষর *member)
{
	পূর্ণসংখ্যা member_len = strlen(member);
	ধ্রুবক অক্ষর *s, *p;
	পূর্ণসংখ্যা substr_len;

	জন্য (s = list; *s; s = p + (*p != '\0')) {
		p = strchrnul(s, ',');
		substr_len = p - s;
		যদি (member_len >= substr_len &&
		    !strncmp(member + member_len - substr_len, s, substr_len))
			ফেরত true;
	}

	ফেরত false;
}

/**
 * log_passes_filters() - check if a log record passes the filters for a device
 *
 * @ldev: Log device to check
 * @rec: Log record to check
 * Return: true if @rec is not blocked by the filters in @ldev, false if it is
 */
স্থির bool log_passes_filters(কাঠামো log_device *ldev, কাঠামো log_rec *rec)
{
	কাঠামো log_filter *filt;

	যদি (rec->flags & LOGRECF_FORCE_DEBUG)
		ফেরত true;

	/* If there are no filters, filter on the default log level */
	যদি (list_empty(&ldev->filter_শীর্ষ)) {
		যদি (rec->level > gd->default_log_level)
			ফেরত false;
		ফেরত true;
	}

	list_for_each_entry(filt, &ldev->filter_শীর্ষ, sibling_গ্রন্থি) {
		যদি (filt->flags & LOGFF_LEVEL_MIN) {
			যদি (rec->level < filt->level)
				চালু;
		} অন্যথা যদি (rec->level > filt->level) {
			চালু;
		}

		যদি ((filt->flags & LOGFF_HAS_CAT) &&
		    !log_has_cat(filt->cat_list, rec->cat))
			চালু;

		যদি (filt->file_list &&
		    !log_has_member(filt->file_list, rec->file))
			চালু;

		যদি (filt->func_list &&
		    !log_has_member(filt->func_list, rec->func))
			চালু;

		যদি (filt->flags & LOGFF_DENY)
			ফেরত false;
		অন্যথা
			ফেরত true;
	}

	ফেরত false;
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
স্থির পূর্ণসংখ্যা log_dispatch(কাঠামো log_rec *rec, ধ্রুবক অক্ষর *fmt, va_list args)
{
	কাঠামো log_device *ldev;
	অক্ষর buf[CONFIG_SYS_CBSIZE];

	/*
	 * When a log driver writes messages (e.g. via the network stack) this
	 * may result in further generated messages. We cannot process them here
	 * as this might result in infinite recursion.
	 */
	যদি (gd->processing_বার্তা)
		ফেরত 1;

	/* Emit message */
	gd->processing_বার্তা = true;
	list_for_each_entry(ldev, &gd->log_শীর্ষ, sibling_গ্রন্থি) {
		যদি ((ldev->flags & LOGDF_ENABLE) &&
		    log_passes_filters(ldev, rec)) {
			যদি (!rec->বার্তা) {
				পূর্ণসংখ্যা len;

				len = vsnprintf(buf, আকার(buf), fmt, args);
				rec->বার্তা = buf;
				gd->log_cont = len && buf[len - 1] != '\n';
			}
			ldev->drv->emit(ldev, rec);
		}
	}
	gd->processing_বার্তা = false;
	ফেরত 0;
}

পূর্ণসংখ্যা _log(গণনা log_category_t cat, গণনা log_level_t level, ধ্রুবক অক্ষর *file,
	 পূর্ণসংখ্যা সারি, ধ্রুবক অক্ষর *func, ধ্রুবক অক্ষর *fmt, ...)
{
	কাঠামো log_rec rec;
	va_list args;

	যদি (!gd)
		ফেরত -ENOSYS;

	/* Check for message continuation */
	যদি (cat == LOGC_CONT)
		cat = gd->logc_পূর্ব;
	যদি (level == LOGL_CONT)
		level = gd->logl_পূর্ব;

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
	rec.বার্তা = ফাঁকা;

	যদি (!(gd->flags & GD_FLG_LOG_READY)) {
		gd->log_drop_গুনতি++;

		/* display dropped traces with console puts and DEBUG_UART */
		যদি (rec.level <= CONFIG_LOG_DEFAULT_LEVEL ||
		    rec.flags & LOGRECF_FORCE_DEBUG) {
			অক্ষর buf[CONFIG_SYS_CBSIZE];

			va_start(args, fmt);
			vsnprintf(buf, আকার(buf), fmt, args);
			puts(buf);
			va_end(args);
		}

		ফেরত -ENOSYS;
	}
	va_start(args, fmt);
	যদি (!log_dispatch(&rec, fmt, args)) {
		gd->logc_পূর্ব = cat;
		gd->logl_পূর্ব = level;
	}
	va_end(args);

	ফেরত 0;
}

#define MAX_সারি_দৈর্ঘ্য_BYTES		64
#define DEFAULT_সারি_দৈর্ঘ্য_BYTES	16

পূর্ণসংখ্যা log_বাফার(গণনা log_category_t cat, গণনা log_level_t level,
		ধ্রুবক অক্ষর *file, পূর্ণসংখ্যা সারি, ধ্রুবক অক্ষর *func, ulong addr,
		ধ্রুবক শূন্য *তথ্য, uint width, uint গুনতি, uint linelen)
{
	যদি (linelen * width > MAX_সারি_দৈর্ঘ্য_BYTES)
		linelen = MAX_সারি_দৈর্ঘ্য_BYTES / width;
	যদি (linelen < 1)
		linelen = DEFAULT_সারি_দৈর্ঘ্য_BYTES / width;

	যখন (গুনতি) {
		uint thislinelen;
		অক্ষর buf[HEXDUMP_MAX_BUF_দৈর্ঘ্য(width * linelen)];

		thislinelen = hexdump_সারি(addr, তথ্য, width, গুনতি, linelen,
					   buf, আকার(buf));
		প্রতিজ্ঞা(thislinelen >= 0);
		_log(cat, level, file, সারি, func, "%s\n", buf);

		/* update references */
		তথ্য += thislinelen * width;
		addr += thislinelen * width;
		গুনতি -= thislinelen;
	}

	ফেরত 0;
}

পূর্ণসংখ্যা log_add_filter_flags(ধ্রুবক অক্ষর *drv_name, গণনা log_category_t cat_list[],
			 গণনা log_level_t level, ধ্রুবক অক্ষর *file_list,
			 ধ্রুবক অক্ষর *func_list, পূর্ণসংখ্যা flags)
{
	কাঠামো log_filter *filt;
	কাঠামো log_device *ldev;
	পূর্ণসংখ্যা ret;
	পূর্ণসংখ্যা i;

	ldev = log_device_find_by_name(drv_name);
	যদি (!ldev)
		ফেরত -ENOENT;
	filt = calloc(1, আকার(*filt));
	যদি (!filt)
		ফেরত -ENOMEM;

	filt->flags = flags;
	যদি (cat_list) {
		filt->flags |= LOGFF_HAS_CAT;
		জন্য (i = 0; ; i++) {
			যদি (i == ARRAY_আকার(filt->cat_list)) {
				ret = -ENOSPC;
				যাও err;
			}
			filt->cat_list[i] = cat_list[i];
			যদি (cat_list[i] == LOGC_END)
				বিরতি;
		}
	}
	filt->level = level;
	যদি (file_list) {
		filt->file_list = strdup(file_list);
		যদি (!filt->file_list) {
			ret = -ENOMEM;
			যাও err;
		}
	}
	যদি (func_list) {
		filt->func_list = strdup(func_list);
		যদি (!filt->func_list) {
			ret = -ENOMEM;
			যাও err;
		}
	}
	filt->filter_অঙ্ক = ldev->পরবর্তী_filter_অঙ্ক++;
	/* Add deny filters to the beginning of the list */
	যদি (flags & LOGFF_DENY)
		list_add(&filt->sibling_গ্রন্থি, &ldev->filter_শীর্ষ);
	অন্যথা
		list_add_লেজ(&filt->sibling_গ্রন্থি, &ldev->filter_শীর্ষ);

	ফেরত filt->filter_অঙ্ক;

err:
	মুক্ত(filt);
	ফেরত ret;
}

পূর্ণসংখ্যা log_remove_filter(ধ্রুবক অক্ষর *drv_name, পূর্ণসংখ্যা filter_অঙ্ক)
{
	কাঠামো log_filter *filt;
	কাঠামো log_device *ldev;

	ldev = log_device_find_by_name(drv_name);
	যদি (!ldev)
		ফেরত -ENOENT;

	list_for_each_entry(filt, &ldev->filter_শীর্ষ, sibling_গ্রন্থি) {
		যদি (filt->filter_অঙ্ক == filter_অঙ্ক) {
			list_del(&filt->sibling_গ্রন্থি);
			মুক্ত(filt);

			ফেরত 0;
		}
	}

	ফেরত -ENOENT;
}

/**
 * log_find_device_by_drv() - Find a device by its driver
 *
 * @drv: Log driver
 * Return: Device associated with that driver, or NULL if not found
 */
স্থির কাঠামো log_device *log_find_device_by_drv(কাঠামো log_driver *drv)
{
	কাঠামো log_device *ldev;

	list_for_each_entry(ldev, &gd->log_শীর্ষ, sibling_গ্রন্থি) {
		যদি (ldev->drv == drv)
			ফেরত ldev;
	}
	/*
	 * It is quite hard to pass an invalid driver since passing an unknown
	 * LOG_GET_DRIVER(xxx) would normally produce a compilation error. But
	 * it is possible to pass NULL, for example, so this
	 */

	ফেরত ফাঁকা;
}

পূর্ণসংখ্যা log_device_set_enable(কাঠামো log_driver *drv, bool enable)
{
	কাঠামো log_device *ldev;

	ldev = log_find_device_by_drv(drv);
	যদি (!ldev)
		ফেরত -ENOENT;
	যদি (enable)
		ldev->flags |= LOGDF_ENABLE;
	অন্যথা
		ldev->flags &= ~LOGDF_ENABLE;

	ফেরত 0;
}

শূন্য log_fixup_for_gd_move(কাঠামো global_তথ্য *new_gd)
{
	new_gd->log_শীর্ষ.পূর্ব->পরবর্তী = &new_gd->log_শীর্ষ;
}

পূর্ণসংখ্যা log_init(শূন্য)
{
	কাঠামো log_driver *drv = ll_entry_start(কাঠামো log_driver, log_driver);
	ধ্রুবক পূর্ণসংখ্যা গুনতি = ll_entry_গুনতি(কাঠামো log_driver, log_driver);
	কাঠামো log_driver *end = drv + গুনতি;

	/*
	 * We cannot add runtime data to the driver since it is likely stored
	 * in rodata. Instead, set up a 'device' corresponding to each driver.
	 * We only support having a single device for each driver.
	 */
	INIT_LIST_শীর্ষ((কাঠামো list_শীর্ষ *)&gd->log_শীর্ষ);
	যখন (drv < end) {
		কাঠামো log_device *ldev;

		ldev = calloc(1, আকার(*ldev));
		যদি (!ldev) {
			debug("%s: Cannot allocate memory\n", __func__);
			ফেরত -ENOMEM;
		}
		INIT_LIST_শীর্ষ(&ldev->filter_শীর্ষ);
		ldev->drv = drv;
		ldev->flags = drv->flags;
		list_add_লেজ(&ldev->sibling_গ্রন্থি,
			      (কাঠামো list_শীর্ষ *)&gd->log_শীর্ষ);
		drv++;
	}
	gd->flags |= GD_FLG_LOG_READY;
	যদি (!gd->default_log_level)
		gd->default_log_level = CONFIG_LOG_DEFAULT_LEVEL;
	gd->log_fmt = log_get_default_format();
	gd->logc_পূর্ব = LOGC_NONE;
	gd->logl_পূর্ব = LOGL_INFO;

	ফেরত 0;
}
