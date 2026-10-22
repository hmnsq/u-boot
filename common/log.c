// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#شامل <display_options.h>
#شامل <log.h>
#شامل <malloc.h>
#شامل <asm/global_data.h>
#شامل <dm/uclass.h>

DECLARE_GLOBAL_DATA_PTR;

ایستا ثابت نویسه *ثابت log_cat_name[] = {
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

_Static_assert(ARRAY_SIZE(log_cat_name) == LOGC_شمار - LOGC_NONE,
	       "log_cat_name size");

ایستا ثابت نویسه *ثابت log_level_name[] = {
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

_Static_assert(ARRAY_SIZE(log_level_name) == LOGL_شمار, "log_level_name size");

/* All error responses MUST begin with '<' */
ثابت نویسه *log_get_cat_name(شمارش log_category_t cat)
{
	ثابت نویسه *name;

	اگر (cat < 0 || cat >= LOGC_شمار)
		بازگشت "<invalid>";
	اگر (cat >= LOGC_NONE)
		بازگشت log_cat_name[cat - LOGC_NONE];

#اگر CONFIG_IS_ENABLED(DM)
	name = uclass_get_name((شمارش uclass_id)cat);
#وگرنه
	name = پوچ;
#endif

	بازگشت name ? name : "<missing>";
}

شمارش log_category_t log_get_cat_by_name(ثابت نویسه *name)
{
	شمارش uclass_id id;
	صحیح i;

	برای (i = LOGC_NONE; i < LOGC_شمار; i++)
		اگر (!strcmp(name, log_cat_name[i - LOGC_NONE]))
			بازگشت i;
	id = uclass_get_by_name(name);
	اگر (id != UCLASS_INVALID)
		بازگشت (شمارش log_category_t)id;

	بازگشت LOGC_NONE;
}

ثابت نویسه *log_get_level_name(شمارش log_level_t level)
{
	اگر (level >= LOGL_شمار)
		بازگشت "INVALID";
	بازگشت log_level_name[level];
}

شمارش log_level_t log_get_level_by_name(ثابت نویسه *name)
{
	صحیح i;

	برای (i = 0; i < LOGL_شمار; i++) {
		اگر (!strcasecmp(log_level_name[i], name))
			بازگشت i;
	}

	بازگشت LOGL_NONE;
}

ساختار log_device *log_device_find_by_name(ثابت نویسه *drv_name)
{
	ساختار log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_گره) {
		اگر (!strcmp(drv_name, ldev->drv->name))
			بازگشت ldev;
	}

	بازگشت پوچ;
}

bool log_has_cat(شمارش log_category_t cat_list[], شمارش log_category_t cat)
{
	صحیح i;

	برای (i = 0; i < LOGF_MAX_CATEGORIES && cat_list[i] != LOGC_END; i++) {
		اگر (cat_list[i] == cat)
			بازگشت true;
	}

	بازگشت false;
}

/**
 * log_has_member() - check if a string is in a comma separated list
 *
 * @list:	Comma separated list of strings
 * @member:	String to find
 *
 * Return: ``true`` if @member is in @list, else ``false``
 */
ایستا bool log_has_member(ثابت نویسه *list, ثابت نویسه *member)
{
	صحیح member_len = strlen(member);
	ثابت نویسه *s, *p;
	صحیح substr_len;

	برای (s = list; *s; s = p + (*p != '\0')) {
		p = strchrnul(s, ',');
		substr_len = p - s;
		اگر (member_len >= substr_len &&
		    !strncmp(member + member_len - substr_len, s, substr_len))
			بازگشت true;
	}

	بازگشت false;
}

/**
 * log_passes_filters() - check if a log record passes the filters for a device
 *
 * @ldev: Log device to check
 * @rec: Log record to check
 * Return: true if @rec is not blocked by the filters in @ldev, false if it is
 */
ایستا bool log_passes_filters(ساختار log_device *ldev, ساختار log_rec *rec)
{
	ساختار log_filter *filt;

	اگر (rec->flags & LOGRECF_FORCE_DEBUG)
		بازگشت true;

	/* If there are no filters, filter on the default log level */
	اگر (list_empty(&ldev->filter_head)) {
		اگر (rec->level > gd->default_log_level)
			بازگشت false;
		بازگشت true;
	}

	list_for_each_entry(filt, &ldev->filter_head, sibling_گره) {
		اگر (filt->flags & LOGFF_LEVEL_MIN) {
			اگر (rec->level < filt->level)
				ادامه;
		} وگرنه اگر (rec->level > filt->level) {
			ادامه;
		}

		اگر ((filt->flags & LOGFF_HAS_CAT) &&
		    !log_has_cat(filt->cat_list, rec->cat))
			ادامه;

		اگر (filt->file_list &&
		    !log_has_member(filt->file_list, rec->file))
			ادامه;

		اگر (filt->func_list &&
		    !log_has_member(filt->func_list, rec->func))
			ادامه;

		اگر (filt->flags & LOGFF_DENY)
			بازگشت false;
		وگرنه
			بازگشت true;
	}

	بازگشت false;
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
ایستا صحیح log_dispatch(ساختار log_rec *rec, ثابت نویسه *fmt, va_list args)
{
	ساختار log_device *ldev;
	نویسه buf[CONFIG_SYS_CBSIZE];

	/*
	 * When a log driver writes messages (e.g. via the network stack) this
	 * may result in further generated messages. We cannot process them here
	 * as this might result in infinite recursion.
	 */
	اگر (gd->processing_پیام)
		بازگشت 1;

	/* Emit message */
	gd->processing_پیام = true;
	list_for_each_entry(ldev, &gd->log_head, sibling_گره) {
		اگر ((ldev->flags & LOGDF_ENABLE) &&
		    log_passes_filters(ldev, rec)) {
			اگر (!rec->پیام) {
				صحیح len;

				len = vsnprintf(buf, اندازه(buf), fmt, args);
				rec->پیام = buf;
				gd->log_cont = len && buf[len - 1] != '\n';
			}
			ldev->drv->emit(ldev, rec);
		}
	}
	gd->processing_پیام = false;
	بازگشت 0;
}

صحیح _log(شمارش log_category_t cat, شمارش log_level_t level, ثابت نویسه *file,
	 صحیح سطر, ثابت نویسه *func, ثابت نویسه *fmt, ...)
{
	ساختار log_rec rec;
	va_list args;

	اگر (!gd)
		بازگشت -ENOSYS;

	/* Check for message continuation */
	اگر (cat == LOGC_CONT)
		cat = gd->logc_prev;
	اگر (level == LOGL_CONT)
		level = gd->logl_prev;

	rec.cat = cat;
	rec.level = level & LOGL_LEVEL_MASK;
	rec.flags = 0;
	اگر (level & LOGL_FORCE_DEBUG)
		rec.flags |= LOGRECF_FORCE_DEBUG;
	اگر (gd->log_cont)
		rec.flags |= LOGRECF_CONT;
	rec.file = file;
	rec.سطر = سطر;
	rec.func = func;
	rec.پیام = پوچ;

	اگر (!(gd->flags & GD_FLG_LOG_READY)) {
		gd->log_drop_شمار++;

		/* display dropped traces with console puts and DEBUG_UART */
		اگر (rec.level <= CONFIG_LOG_DEFAULT_LEVEL ||
		    rec.flags & LOGRECF_FORCE_DEBUG) {
			نویسه buf[CONFIG_SYS_CBSIZE];

			va_start(args, fmt);
			vsnprintf(buf, اندازه(buf), fmt, args);
			puts(buf);
			va_end(args);
		}

		بازگشت -ENOSYS;
	}
	va_start(args, fmt);
	اگر (!log_dispatch(&rec, fmt, args)) {
		gd->logc_prev = cat;
		gd->logl_prev = level;
	}
	va_end(args);

	بازگشت 0;
}

#define MAX_سطر_LENGTH_BYTES		64
#define DEFAULT_سطر_LENGTH_BYTES	16

صحیح _log_buffer(شمارش log_category_t cat, شمارش log_level_t level,
		ثابت نویسه *file, صحیح سطر, ثابت نویسه *func, ulong addr,
		ثابت تهی *data, uint width, uint شمار, uint linelen)
{
	اگر (linelen * width > MAX_سطر_LENGTH_BYTES)
		linelen = MAX_سطر_LENGTH_BYTES / width;
	اگر (linelen < 1)
		linelen = DEFAULT_سطر_LENGTH_BYTES / width;

	وقتی (شمار) {
		uint thislinelen;
		نویسه buf[HEXDUMP_MAX_BUF_LENGTH(width * linelen)];

		thislinelen = hexdump_سطر(addr, data, width, شمار, linelen,
					   buf, اندازه(buf));
		أكد(thislinelen >= 0);
		_log(cat, level, file, سطر, func, "%s\n", buf);

		/* update references */
		data += thislinelen * width;
		addr += thislinelen * width;
		شمار -= thislinelen;
	}

	بازگشت 0;
}

صحیح log_add_filter_flags(ثابت نویسه *drv_name, شمارش log_category_t cat_list[],
			 شمارش log_level_t level, ثابت نویسه *file_list,
			 ثابت نویسه *func_list, صحیح flags)
{
	ساختار log_filter *filt;
	ساختار log_device *ldev;
	صحیح ret;
	صحیح i;

	ldev = log_device_find_by_name(drv_name);
	اگر (!ldev)
		بازگشت -ENOENT;
	filt = calloc(1, اندازه(*filt));
	اگر (!filt)
		بازگشت -ENOMEM;

	filt->flags = flags;
	اگر (cat_list) {
		filt->flags |= LOGFF_HAS_CAT;
		برای (i = 0; ; i++) {
			اگر (i == ARRAY_SIZE(filt->cat_list)) {
				ret = -ENOSPC;
				برو err;
			}
			filt->cat_list[i] = cat_list[i];
			اگر (cat_list[i] == LOGC_END)
				بشکن;
		}
	}
	filt->level = level;
	اگر (file_list) {
		filt->file_list = strdup(file_list);
		اگر (!filt->file_list) {
			ret = -ENOMEM;
			برو err;
		}
	}
	اگر (func_list) {
		filt->func_list = strdup(func_list);
		اگر (!filt->func_list) {
			ret = -ENOMEM;
			برو err;
		}
	}
	filt->filter_رقم = ldev->next_filter_رقم++;
	/* Add deny filters to the beginning of the list */
	اگر (flags & LOGFF_DENY)
		list_add(&filt->sibling_گره, &ldev->filter_head);
	وگرنه
		list_add_tail(&filt->sibling_گره, &ldev->filter_head);

	بازگشت filt->filter_رقم;

err:
	آزاد(filt);
	بازگشت ret;
}

صحیح log_remove_filter(ثابت نویسه *drv_name, صحیح filter_رقم)
{
	ساختار log_filter *filt;
	ساختار log_device *ldev;

	ldev = log_device_find_by_name(drv_name);
	اگر (!ldev)
		بازگشت -ENOENT;

	list_for_each_entry(filt, &ldev->filter_head, sibling_گره) {
		اگر (filt->filter_رقم == filter_رقم) {
			list_del(&filt->sibling_گره);
			آزاد(filt);

			بازگشت 0;
		}
	}

	بازگشت -ENOENT;
}

/**
 * log_find_device_by_drv() - Find a device by its driver
 *
 * @drv: Log driver
 * Return: Device associated with that driver, or NULL if not found
 */
ایستا ساختار log_device *log_find_device_by_drv(ساختار log_driver *drv)
{
	ساختار log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_گره) {
		اگر (ldev->drv == drv)
			بازگشت ldev;
	}
	/*
	 * It is quite hard to pass an invalid driver since passing an unknown
	 * LOG_GET_DRIVER(xxx) would normally produce a compilation error. But
	 * it is possible to pass NULL, for example, so this
	 */

	بازگشت پوچ;
}

صحیح log_device_set_enable(ساختار log_driver *drv, bool enable)
{
	ساختار log_device *ldev;

	ldev = log_find_device_by_drv(drv);
	اگر (!ldev)
		بازگشت -ENOENT;
	اگر (enable)
		ldev->flags |= LOGDF_ENABLE;
	وگرنه
		ldev->flags &= ~LOGDF_ENABLE;

	بازگشت 0;
}

تهی log_fixup_for_gd_move(ساختار global_data *new_gd)
{
	new_gd->log_head.prev->next = &new_gd->log_head;
}

صحیح log_init(تهی)
{
	ساختار log_driver *drv = ll_entry_start(ساختار log_driver, log_driver);
	ثابت صحیح شمار = ll_entry_شمار(ساختار log_driver, log_driver);
	ساختار log_driver *end = drv + شمار;

	/*
	 * We cannot add runtime data to the driver since it is likely stored
	 * in rodata. Instead, set up a 'device' corresponding to each driver.
	 * We only support having a single device for each driver.
	 */
	INIT_LIST_HEAD((ساختار list_head *)&gd->log_head);
	وقتی (drv < end) {
		ساختار log_device *ldev;

		ldev = calloc(1, اندازه(*ldev));
		اگر (!ldev) {
			debug("%s: Cannot allocate memory\n", __func__);
			بازگشت -ENOMEM;
		}
		INIT_LIST_HEAD(&ldev->filter_head);
		ldev->drv = drv;
		ldev->flags = drv->flags;
		list_add_tail(&ldev->sibling_گره,
			      (ساختار list_head *)&gd->log_head);
		drv++;
	}
	gd->flags |= GD_FLG_LOG_READY;
	اگر (!gd->default_log_level)
		gd->default_log_level = CONFIG_LOG_DEFAULT_LEVEL;
	gd->log_fmt = log_get_default_format();
	gd->logc_prev = LOGC_NONE;
	gd->logl_prev = LOGL_INFO;

	بازگشت 0;
}
