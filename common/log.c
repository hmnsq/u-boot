// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ضمن <display_options.h>
#ضمن <log.h>
#ضمن <malloc.h>
#ضمن <asm/global_data.h>
#ضمن <dm/uclass.h>

DECLARE_GLOBAL_DATA_PTR;

ساكن ثابت حرف *ثابت log_cat_name[] = {
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

_Static_assert(ARRAY_SIZE(log_cat_name) == LOGC_COUNT - LOGC_NONE,
	       "log_cat_name size");

ساكن ثابت حرف *ثابت log_level_name[] = {
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

_Static_assert(ARRAY_SIZE(log_level_name) == LOGL_COUNT, "log_level_name size");

/* All error responses MUST begin with '<' */
ثابت حرف *log_get_cat_name(تعداد log_category_t cat)
{
	ثابت حرف *name;

	ܐܢ (cat < 0 || cat >= LOGC_COUNT)
		ܦܢܐ "<invalid>";
	ܐܢ (cat >= LOGC_NONE)
		ܦܢܐ log_cat_name[cat - LOGC_NONE];

#ܐܢ CONFIG_IS_ENABLED(DM)
	name = uclass_get_name((تعداد uclass_id)cat);
#ܐܚܪܢܐ
	name = عدم;
#endif

	ܦܢܐ name ? name : "<missing>";
}

تعداد log_category_t log_get_cat_by_name(ثابت حرف *name)
{
	تعداد uclass_id id;
	صحيح i;

	لكل (i = LOGC_NONE; i < LOGC_COUNT; i++)
		ܐܢ (!strcmp(name, log_cat_name[i - LOGC_NONE]))
			ܦܢܐ i;
	id = uclass_get_by_name(name);
	ܐܢ (id != UCLASS_INVALID)
		ܦܢܐ (تعداد log_category_t)id;

	ܦܢܐ LOGC_NONE;
}

ثابت حرف *log_get_level_name(تعداد log_level_t level)
{
	ܐܢ (level >= LOGL_COUNT)
		ܦܢܐ "INVALID";
	ܦܢܐ log_level_name[level];
}

تعداد log_level_t log_get_level_by_name(ثابت حرف *name)
{
	صحيح i;

	لكل (i = 0; i < LOGL_COUNT; i++) {
		ܐܢ (!strcasecmp(log_level_name[i], name))
			ܦܢܐ i;
	}

	ܦܢܐ LOGL_NONE;
}

بنية log_device *log_device_find_by_name(ثابت حرف *drv_name)
{
	بنية log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_node) {
		ܐܢ (!strcmp(drv_name, ldev->drv->name))
			ܦܢܐ ldev;
	}

	ܦܢܐ عدم;
}

bool log_has_cat(تعداد log_category_t cat_list[], تعداد log_category_t cat)
{
	صحيح i;

	لكل (i = 0; i < LOGF_MAX_CATEGORIES && cat_list[i] != LOGC_END; i++) {
		ܐܢ (cat_list[i] == cat)
			ܦܢܐ true;
	}

	ܦܢܐ false;
}

/**
 * log_has_member() - check if a string is in a comma separated list
 *
 * @list:	Comma separated list of strings
 * @member:	String to find
 *
 * Return: ``true`` if @member is in @list, else ``false``
 */
ساكن bool log_has_member(ثابت حرف *list, ثابت حرف *member)
{
	صحيح member_len = strlen(member);
	ثابت حرف *s, *p;
	صحيح substr_len;

	لكل (s = list; *s; s = p + (*p != '\0')) {
		p = strchrnul(s, ',');
		substr_len = p - s;
		ܐܢ (member_len >= substr_len &&
		    !strncmp(member + member_len - substr_len, s, substr_len))
			ܦܢܐ true;
	}

	ܦܢܐ false;
}

/**
 * log_passes_filters() - check if a log record passes the filters for a device
 *
 * @ldev: Log device to check
 * @rec: Log record to check
 * Return: true if @rec is not blocked by the filters in @ldev, false if it is
 */
ساكن bool log_passes_filters(بنية log_device *ldev, بنية log_rec *rec)
{
	بنية log_filter *filt;

	ܐܢ (rec->flags & LOGRECF_FORCE_DEBUG)
		ܦܢܐ true;

	/* If there are no filters, filter on the default log level */
	ܐܢ (list_empty(&ldev->filter_head)) {
		ܐܢ (rec->level > gd->default_log_level)
			ܦܢܐ false;
		ܦܢܐ true;
	}

	list_for_each_entry(filt, &ldev->filter_head, sibling_node) {
		ܐܢ (filt->flags & LOGFF_LEVEL_MIN) {
			ܐܢ (rec->level < filt->level)
				استمر;
		} ܐܚܪܢܐ ܐܢ (rec->level > filt->level) {
			استمر;
		}

		ܐܢ ((filt->flags & LOGFF_HAS_CAT) &&
		    !log_has_cat(filt->cat_list, rec->cat))
			استمر;

		ܐܢ (filt->file_list &&
		    !log_has_member(filt->file_list, rec->file))
			استمر;

		ܐܢ (filt->func_list &&
		    !log_has_member(filt->func_list, rec->func))
			استمر;

		ܐܢ (filt->flags & LOGFF_DENY)
			ܦܢܐ false;
		ܐܚܪܢܐ
			ܦܢܐ true;
	}

	ܦܢܐ false;
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
ساكن صحيح log_dispatch(بنية log_rec *rec, ثابت حرف *fmt, va_list args)
{
	بنية log_device *ldev;
	حرف buf[CONFIG_SYS_CBSIZE];

	/*
	 * When a log driver writes messages (e.g. via the network stack) this
	 * may result in further generated messages. We cannot process them here
	 * as this might result in infinite recursion.
	 */
	ܐܢ (gd->processing_msg)
		ܦܢܐ 1;

	/* Emit message */
	gd->processing_msg = true;
	list_for_each_entry(ldev, &gd->log_head, sibling_node) {
		ܐܢ ((ldev->flags & LOGDF_ENABLE) &&
		    log_passes_filters(ldev, rec)) {
			ܐܢ (!rec->msg) {
				صحيح len;

				len = vsnprintf(buf, حجم(buf), fmt, args);
				rec->msg = buf;
				gd->log_cont = len && buf[len - 1] != '\n';
			}
			ldev->drv->emit(ldev, rec);
		}
	}
	gd->processing_msg = false;
	ܦܢܐ 0;
}

صحيح _log(تعداد log_category_t cat, تعداد log_level_t level, ثابت حرف *file,
	 صحيح line, ثابت حرف *func, ثابت حرف *fmt, ...)
{
	بنية log_rec rec;
	va_list args;

	ܐܢ (!gd)
		ܦܢܐ -ENOSYS;

	/* Check for message continuation */
	ܐܢ (cat == LOGC_CONT)
		cat = gd->logc_prev;
	ܐܢ (level == LOGL_CONT)
		level = gd->logl_prev;

	rec.cat = cat;
	rec.level = level & LOGL_LEVEL_MASK;
	rec.flags = 0;
	ܐܢ (level & LOGL_FORCE_DEBUG)
		rec.flags |= LOGRECF_FORCE_DEBUG;
	ܐܢ (gd->log_cont)
		rec.flags |= LOGRECF_CONT;
	rec.file = file;
	rec.line = line;
	rec.func = func;
	rec.msg = عدم;

	ܐܢ (!(gd->flags & GD_FLG_LOG_READY)) {
		gd->log_drop_count++;

		/* display dropped traces with console puts and DEBUG_UART */
		ܐܢ (rec.level <= CONFIG_LOG_DEFAULT_LEVEL ||
		    rec.flags & LOGRECF_FORCE_DEBUG) {
			حرف buf[CONFIG_SYS_CBSIZE];

			va_start(args, fmt);
			vsnprintf(buf, حجم(buf), fmt, args);
			puts(buf);
			va_end(args);
		}

		ܦܢܐ -ENOSYS;
	}
	va_start(args, fmt);
	ܐܢ (!log_dispatch(&rec, fmt, args)) {
		gd->logc_prev = cat;
		gd->logl_prev = level;
	}
	va_end(args);

	ܦܢܐ 0;
}

#define MAX_LINE_LENGTH_BYTES		64
#define DEFAULT_LINE_LENGTH_BYTES	16

صحيح _log_buffer(تعداد log_category_t cat, تعداد log_level_t level,
		ثابت حرف *file, صحيح line, ثابت حرف *func, ulong addr,
		ثابت فراغ *data, uint width, uint count, uint linelen)
{
	ܐܢ (linelen * width > MAX_LINE_LENGTH_BYTES)
		linelen = MAX_LINE_LENGTH_BYTES / width;
	ܐܢ (linelen < 1)
		linelen = DEFAULT_LINE_LENGTH_BYTES / width;

	ܟܡܐ (count) {
		uint thislinelen;
		حرف buf[HEXDUMP_MAX_BUF_LENGTH(width * linelen)];

		thislinelen = hexdump_line(addr, data, width, count, linelen,
					   buf, حجم(buf));
		أكد(thislinelen >= 0);
		_log(cat, level, file, line, func, "%s\n", buf);

		/* update references */
		data += thislinelen * width;
		addr += thislinelen * width;
		count -= thislinelen;
	}

	ܦܢܐ 0;
}

صحيح log_add_filter_flags(ثابت حرف *drv_name, تعداد log_category_t cat_list[],
			 تعداد log_level_t level, ثابت حرف *file_list,
			 ثابت حرف *func_list, صحيح flags)
{
	بنية log_filter *filt;
	بنية log_device *ldev;
	صحيح ret;
	صحيح i;

	ldev = log_device_find_by_name(drv_name);
	ܐܢ (!ldev)
		ܦܢܐ -ENOENT;
	filt = calloc(1, حجم(*filt));
	ܐܢ (!filt)
		ܦܢܐ -ENOMEM;

	filt->flags = flags;
	ܐܢ (cat_list) {
		filt->flags |= LOGFF_HAS_CAT;
		لكل (i = 0; ; i++) {
			ܐܢ (i == ARRAY_SIZE(filt->cat_list)) {
				ret = -ENOSPC;
				اذهب err;
			}
			filt->cat_list[i] = cat_list[i];
			ܐܢ (cat_list[i] == LOGC_END)
				اكسر;
		}
	}
	filt->level = level;
	ܐܢ (file_list) {
		filt->file_list = strdup(file_list);
		ܐܢ (!filt->file_list) {
			ret = -ENOMEM;
			اذهب err;
		}
	}
	ܐܢ (func_list) {
		filt->func_list = strdup(func_list);
		ܐܢ (!filt->func_list) {
			ret = -ENOMEM;
			اذهب err;
		}
	}
	filt->filter_num = ldev->next_filter_num++;
	/* Add deny filters to the beginning of the list */
	ܐܢ (flags & LOGFF_DENY)
		list_add(&filt->sibling_node, &ldev->filter_head);
	ܐܚܪܢܐ
		list_add_tail(&filt->sibling_node, &ldev->filter_head);

	ܦܢܐ filt->filter_num;

err:
	حرر(filt);
	ܦܢܐ ret;
}

صحيح log_remove_filter(ثابت حرف *drv_name, صحيح filter_num)
{
	بنية log_filter *filt;
	بنية log_device *ldev;

	ldev = log_device_find_by_name(drv_name);
	ܐܢ (!ldev)
		ܦܢܐ -ENOENT;

	list_for_each_entry(filt, &ldev->filter_head, sibling_node) {
		ܐܢ (filt->filter_num == filter_num) {
			list_del(&filt->sibling_node);
			حرر(filt);

			ܦܢܐ 0;
		}
	}

	ܦܢܐ -ENOENT;
}

/**
 * log_find_device_by_drv() - Find a device by its driver
 *
 * @drv: Log driver
 * Return: Device associated with that driver, or NULL if not found
 */
ساكن بنية log_device *log_find_device_by_drv(بنية log_driver *drv)
{
	بنية log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_node) {
		ܐܢ (ldev->drv == drv)
			ܦܢܐ ldev;
	}
	/*
	 * It is quite hard to pass an invalid driver since passing an unknown
	 * LOG_GET_DRIVER(xxx) would normally produce a compilation error. But
	 * it is possible to pass NULL, for example, so this
	 */

	ܦܢܐ عدم;
}

صحيح log_device_set_enable(بنية log_driver *drv, bool enable)
{
	بنية log_device *ldev;

	ldev = log_find_device_by_drv(drv);
	ܐܢ (!ldev)
		ܦܢܐ -ENOENT;
	ܐܢ (enable)
		ldev->flags |= LOGDF_ENABLE;
	ܐܚܪܢܐ
		ldev->flags &= ~LOGDF_ENABLE;

	ܦܢܐ 0;
}

فراغ log_fixup_for_gd_move(بنية global_data *new_gd)
{
	new_gd->log_head.prev->next = &new_gd->log_head;
}

صحيح log_init(فراغ)
{
	بنية log_driver *drv = ll_entry_start(بنية log_driver, log_driver);
	ثابت صحيح count = ll_entry_count(بنية log_driver, log_driver);
	بنية log_driver *end = drv + count;

	/*
	 * We cannot add runtime data to the driver since it is likely stored
	 * in rodata. Instead, set up a 'device' corresponding to each driver.
	 * We only support having a single device for each driver.
	 */
	INIT_LIST_HEAD((بنية list_head *)&gd->log_head);
	ܟܡܐ (drv < end) {
		بنية log_device *ldev;

		ldev = calloc(1, حجم(*ldev));
		ܐܢ (!ldev) {
			debug("%s: Cannot allocate memory\n", __func__);
			ܦܢܐ -ENOMEM;
		}
		INIT_LIST_HEAD(&ldev->filter_head);
		ldev->drv = drv;
		ldev->flags = drv->flags;
		list_add_tail(&ldev->sibling_node,
			      (بنية list_head *)&gd->log_head);
		drv++;
	}
	gd->flags |= GD_FLG_LOG_READY;
	ܐܢ (!gd->default_log_level)
		gd->default_log_level = CONFIG_LOG_DEFAULT_LEVEL;
	gd->log_fmt = log_get_default_format();
	gd->logc_prev = LOGC_NONE;
	gd->logl_prev = LOGL_INFO;

	ܦܢܐ 0;
}
