// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#அந்தர்பாவ <display_options.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <malloc.h>
#அந்தர்பாவ <asm/global_data.h>
#அந்தர்பாவ <dm/uclass.h>

DECLARE_GLOBAL_DATA_PTR;

நிலையான மாறிலி எழுத்து *மாறிலி log_cat_name[] = {
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

_Static_assert(ARRAY_SIZE(log_cat_name) == LOGC_மொத்தம் - LOGC_NONE,
	       "log_cat_name size");

நிலையான மாறிலி எழுத்து *மாறிலி log_level_name[] = {
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

_Static_assert(ARRAY_SIZE(log_level_name) == LOGL_மொத்தம், "log_level_name size");

/* All error responses MUST begin with '<' */
மாறிலி எழுத்து *log_get_cat_name(எண்ணிக்கை log_category_t cat)
{
	மாறிலி எழுத்து *name;

	எனில் (cat < 0 || cat >= LOGC_மொத்தம்)
		திரும்பு "<invalid>";
	எனில் (cat >= LOGC_NONE)
		திரும்பு log_cat_name[cat - LOGC_NONE];

#எனில் CONFIG_IS_ENABLED(DM)
	name = uclass_get_name((எண்ணிக்கை uclass_id)cat);
#இல்லையெனில்
	name = வெற்று;
#endif

	திரும்பு name ? name : "<missing>";
}

எண்ணிக்கை log_category_t log_get_cat_by_name(மாறிலி எழுத்து *name)
{
	எண்ணிக்கை uclass_id id;
	முழு i;

	க்கு (i = LOGC_NONE; i < LOGC_மொத்தம்; i++)
		எனில் (!strcmp(name, log_cat_name[i - LOGC_NONE]))
			திரும்பு i;
	id = uclass_get_by_name(name);
	எனில் (id != UCLASS_INVALID)
		திரும்பு (எண்ணிக்கை log_category_t)id;

	திரும்பு LOGC_NONE;
}

மாறிலி எழுத்து *log_get_level_name(எண்ணிக்கை log_level_t level)
{
	எனில் (level >= LOGL_மொத்தம்)
		திரும்பு "INVALID";
	திரும்பு log_level_name[level];
}

எண்ணிக்கை log_level_t log_get_level_by_name(மாறிலி எழுத்து *name)
{
	முழு i;

	க்கு (i = 0; i < LOGL_மொத்தம்; i++) {
		எனில் (!strcasecmp(log_level_name[i], name))
			திரும்பு i;
	}

	திரும்பு LOGL_NONE;
}

கட்டமைப்பு log_device *log_device_find_by_name(மாறிலி எழுத்து *drv_name)
{
	கட்டமைப்பு log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_முனை) {
		எனில் (!strcmp(drv_name, ldev->drv->name))
			திரும்பு ldev;
	}

	திரும்பு வெற்று;
}

bool log_has_cat(எண்ணிக்கை log_category_t cat_list[], எண்ணிக்கை log_category_t cat)
{
	முழு i;

	க்கு (i = 0; i < LOGF_MAX_CATEGORIES && cat_list[i] != LOGC_END; i++) {
		எனில் (cat_list[i] == cat)
			திரும்பு true;
	}

	திரும்பு false;
}

/**
 * log_has_member() - check if a string is in a comma separated list
 *
 * @list:	Comma separated list of strings
 * @member:	String to find
 *
 * Return: ``true`` if @member is in @list, else ``false``
 */
நிலையான bool log_has_member(மாறிலி எழுத்து *list, மாறிலி எழுத்து *member)
{
	முழு member_len = strlen(member);
	மாறிலி எழுத்து *s, *p;
	முழு substr_len;

	க்கு (s = list; *s; s = p + (*p != '\0')) {
		p = strchrnul(s, ',');
		substr_len = p - s;
		எனில் (member_len >= substr_len &&
		    !strncmp(member + member_len - substr_len, s, substr_len))
			திரும்பு true;
	}

	திரும்பு false;
}

/**
 * log_passes_filters() - check if a log record passes the filters for a device
 *
 * @ldev: Log device to check
 * @rec: Log record to check
 * Return: true if @rec is not blocked by the filters in @ldev, false if it is
 */
நிலையான bool log_passes_filters(கட்டமைப்பு log_device *ldev, கட்டமைப்பு log_rec *rec)
{
	கட்டமைப்பு log_filter *filt;

	எனில் (rec->flags & LOGRECF_FORCE_DEBUG)
		திரும்பு true;

	/* If there are no filters, filter on the default log level */
	எனில் (list_empty(&ldev->filter_head)) {
		எனில் (rec->level > gd->default_log_level)
			திரும்பு false;
		திரும்பு true;
	}

	list_for_each_entry(filt, &ldev->filter_head, sibling_முனை) {
		எனில் (filt->flags & LOGFF_LEVEL_MIN) {
			எனில் (rec->level < filt->level)
				தொடர்;
		} இல்லையெனில் எனில் (rec->level > filt->level) {
			தொடர்;
		}

		எனில் ((filt->flags & LOGFF_HAS_CAT) &&
		    !log_has_cat(filt->cat_list, rec->cat))
			தொடர்;

		எனில் (filt->file_list &&
		    !log_has_member(filt->file_list, rec->file))
			தொடர்;

		எனில் (filt->func_list &&
		    !log_has_member(filt->func_list, rec->func))
			தொடர்;

		எனில் (filt->flags & LOGFF_DENY)
			திரும்பு false;
		இல்லையெனில்
			திரும்பு true;
	}

	திரும்பு false;
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
நிலையான முழு log_dispatch(கட்டமைப்பு log_rec *rec, மாறிலி எழுத்து *fmt, va_list args)
{
	கட்டமைப்பு log_device *ldev;
	எழுத்து buf[CONFIG_SYS_CBSIZE];

	/*
	 * When a log driver writes messages (e.g. via the network stack) this
	 * may result in further generated messages. We cannot process them here
	 * as this might result in infinite recursion.
	 */
	எனில் (gd->processing_செய்தி)
		திரும்பு 1;

	/* Emit message */
	gd->processing_செய்தி = true;
	list_for_each_entry(ldev, &gd->log_head, sibling_முனை) {
		எனில் ((ldev->flags & LOGDF_ENABLE) &&
		    log_passes_filters(ldev, rec)) {
			எனில் (!rec->செய்தி) {
				முழு len;

				len = vsnprintf(buf, அளவு(buf), fmt, args);
				rec->செய்தி = buf;
				gd->log_cont = len && buf[len - 1] != '\n';
			}
			ldev->drv->emit(ldev, rec);
		}
	}
	gd->processing_செய்தி = false;
	திரும்பு 0;
}

முழு _log(எண்ணிக்கை log_category_t cat, எண்ணிக்கை log_level_t level, மாறிலி எழுத்து *file,
	 முழு வரி, மாறிலி எழுத்து *func, மாறிலி எழுத்து *fmt, ...)
{
	கட்டமைப்பு log_rec rec;
	va_list args;

	எனில் (!gd)
		திரும்பு -ENOSYS;

	/* Check for message continuation */
	எனில் (cat == LOGC_CONT)
		cat = gd->logc_prev;
	எனில் (level == LOGL_CONT)
		level = gd->logl_prev;

	rec.cat = cat;
	rec.level = level & LOGL_LEVEL_MASK;
	rec.flags = 0;
	எனில் (level & LOGL_FORCE_DEBUG)
		rec.flags |= LOGRECF_FORCE_DEBUG;
	எனில் (gd->log_cont)
		rec.flags |= LOGRECF_CONT;
	rec.file = file;
	rec.வரி = வரி;
	rec.func = func;
	rec.செய்தி = வெற்று;

	எனில் (!(gd->flags & GD_FLG_LOG_READY)) {
		gd->log_drop_மொத்தம்++;

		/* display dropped traces with console puts and DEBUG_UART */
		எனில் (rec.level <= CONFIG_LOG_DEFAULT_LEVEL ||
		    rec.flags & LOGRECF_FORCE_DEBUG) {
			எழுத்து buf[CONFIG_SYS_CBSIZE];

			va_start(args, fmt);
			vsnprintf(buf, அளவு(buf), fmt, args);
			puts(buf);
			va_end(args);
		}

		திரும்பு -ENOSYS;
	}
	va_start(args, fmt);
	எனில் (!log_dispatch(&rec, fmt, args)) {
		gd->logc_prev = cat;
		gd->logl_prev = level;
	}
	va_end(args);

	திரும்பு 0;
}

#define MAX_வரி_LENGTH_BYTES		64
#define DEFAULT_வரி_LENGTH_BYTES	16

முழு _log_buffer(எண்ணிக்கை log_category_t cat, எண்ணிக்கை log_level_t level,
		மாறிலி எழுத்து *file, முழு வரி, மாறிலி எழுத்து *func, ulong addr,
		மாறிலி வெறுமை *data, uint width, uint மொத்தம், uint linelen)
{
	எனில் (linelen * width > MAX_வரி_LENGTH_BYTES)
		linelen = MAX_வரி_LENGTH_BYTES / width;
	எனில் (linelen < 1)
		linelen = DEFAULT_வரி_LENGTH_BYTES / width;

	போது (மொத்தம்) {
		uint thislinelen;
		எழுத்து buf[HEXDUMP_MAX_BUF_LENGTH(width * linelen)];

		thislinelen = hexdump_வரி(addr, data, width, மொத்தம், linelen,
					   buf, அளவு(buf));
		ப்ரதிஜ்ஞா(thislinelen >= 0);
		_log(cat, level, file, வரி, func, "%s\n", buf);

		/* update references */
		data += thislinelen * width;
		addr += thislinelen * width;
		மொத்தம் -= thislinelen;
	}

	திரும்பு 0;
}

முழு log_add_filter_flags(மாறிலி எழுத்து *drv_name, எண்ணிக்கை log_category_t cat_list[],
			 எண்ணிக்கை log_level_t level, மாறிலி எழுத்து *file_list,
			 மாறிலி எழுத்து *func_list, முழு flags)
{
	கட்டமைப்பு log_filter *filt;
	கட்டமைப்பு log_device *ldev;
	முழு ret;
	முழு i;

	ldev = log_device_find_by_name(drv_name);
	எனில் (!ldev)
		திரும்பு -ENOENT;
	filt = calloc(1, அளவு(*filt));
	எனில் (!filt)
		திரும்பு -ENOMEM;

	filt->flags = flags;
	எனில் (cat_list) {
		filt->flags |= LOGFF_HAS_CAT;
		க்கு (i = 0; ; i++) {
			எனில் (i == ARRAY_SIZE(filt->cat_list)) {
				ret = -ENOSPC;
				செல் err;
			}
			filt->cat_list[i] = cat_list[i];
			எனில் (cat_list[i] == LOGC_END)
				நிறுத்து;
		}
	}
	filt->level = level;
	எனில் (file_list) {
		filt->file_list = strdup(file_list);
		எனில் (!filt->file_list) {
			ret = -ENOMEM;
			செல் err;
		}
	}
	எனில் (func_list) {
		filt->func_list = strdup(func_list);
		எனில் (!filt->func_list) {
			ret = -ENOMEM;
			செல் err;
		}
	}
	filt->filter_இலக்கம் = ldev->next_filter_இலக்கம்++;
	/* Add deny filters to the beginning of the list */
	எனில் (flags & LOGFF_DENY)
		list_add(&filt->sibling_முனை, &ldev->filter_head);
	இல்லையெனில்
		list_add_tail(&filt->sibling_முனை, &ldev->filter_head);

	திரும்பு filt->filter_இலக்கம்;

err:
	விடுவி(filt);
	திரும்பு ret;
}

முழு log_remove_filter(மாறிலி எழுத்து *drv_name, முழு filter_இலக்கம்)
{
	கட்டமைப்பு log_filter *filt;
	கட்டமைப்பு log_device *ldev;

	ldev = log_device_find_by_name(drv_name);
	எனில் (!ldev)
		திரும்பு -ENOENT;

	list_for_each_entry(filt, &ldev->filter_head, sibling_முனை) {
		எனில் (filt->filter_இலக்கம் == filter_இலக்கம்) {
			list_del(&filt->sibling_முனை);
			விடுவி(filt);

			திரும்பு 0;
		}
	}

	திரும்பு -ENOENT;
}

/**
 * log_find_device_by_drv() - Find a device by its driver
 *
 * @drv: Log driver
 * Return: Device associated with that driver, or NULL if not found
 */
நிலையான கட்டமைப்பு log_device *log_find_device_by_drv(கட்டமைப்பு log_driver *drv)
{
	கட்டமைப்பு log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_முனை) {
		எனில் (ldev->drv == drv)
			திரும்பு ldev;
	}
	/*
	 * It is quite hard to pass an invalid driver since passing an unknown
	 * LOG_GET_DRIVER(xxx) would normally produce a compilation error. But
	 * it is possible to pass NULL, for example, so this
	 */

	திரும்பு வெற்று;
}

முழு log_device_set_enable(கட்டமைப்பு log_driver *drv, bool enable)
{
	கட்டமைப்பு log_device *ldev;

	ldev = log_find_device_by_drv(drv);
	எனில் (!ldev)
		திரும்பு -ENOENT;
	எனில் (enable)
		ldev->flags |= LOGDF_ENABLE;
	இல்லையெனில்
		ldev->flags &= ~LOGDF_ENABLE;

	திரும்பு 0;
}

வெறுமை log_fixup_for_gd_move(கட்டமைப்பு global_data *new_gd)
{
	new_gd->log_head.prev->next = &new_gd->log_head;
}

முழு log_init(வெறுமை)
{
	கட்டமைப்பு log_driver *drv = ll_entry_start(கட்டமைப்பு log_driver, log_driver);
	மாறிலி முழு மொத்தம் = ll_entry_மொத்தம்(கட்டமைப்பு log_driver, log_driver);
	கட்டமைப்பு log_driver *end = drv + மொத்தம்;

	/*
	 * We cannot add runtime data to the driver since it is likely stored
	 * in rodata. Instead, set up a 'device' corresponding to each driver.
	 * We only support having a single device for each driver.
	 */
	INIT_LIST_HEAD((கட்டமைப்பு list_head *)&gd->log_head);
	போது (drv < end) {
		கட்டமைப்பு log_device *ldev;

		ldev = calloc(1, அளவு(*ldev));
		எனில் (!ldev) {
			debug("%s: Cannot allocate memory\n", __func__);
			திரும்பு -ENOMEM;
		}
		INIT_LIST_HEAD(&ldev->filter_head);
		ldev->drv = drv;
		ldev->flags = drv->flags;
		list_add_tail(&ldev->sibling_முனை,
			      (கட்டமைப்பு list_head *)&gd->log_head);
		drv++;
	}
	gd->flags |= GD_FLG_LOG_READY;
	எனில் (!gd->default_log_level)
		gd->default_log_level = CONFIG_LOG_DEFAULT_LEVEL;
	gd->log_fmt = log_get_default_format();
	gd->logc_prev = LOGC_NONE;
	gd->logl_prev = LOGL_INFO;

	திரும்பு 0;
}
