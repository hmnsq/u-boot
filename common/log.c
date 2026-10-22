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

_Static_assert(ARRAY_SIZE(log_cat_name) == LOGC_تعداد - LOGC_NONE,
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

_Static_assert(ARRAY_SIZE(log_level_name) == LOGL_تعداد, "log_level_name size");

/* All error responses MUST begin with '<' */
ثابت حرف *log_get_cat_name(تعداد log_category_t cat)
{
	ثابت حرف *name;

	إذا (cat < 0 || cat >= LOGC_تعداد)
		إرجاع "<invalid>";
	إذا (cat >= LOGC_NONE)
		إرجاع log_cat_name[cat - LOGC_NONE];

#إذا CONFIG_IS_ENABLED(DM)
	name = uclass_get_name((تعداد uclass_id)cat);
#وإلا
	name = عدم;
#endif

	إرجاع name ? name : "<missing>";
}

تعداد log_category_t log_get_cat_by_name(ثابت حرف *name)
{
	تعداد uclass_id id;
	صحيح i;

	لكل (i = LOGC_NONE; i < LOGC_تعداد; i++)
		إذا (!strcmp(name, log_cat_name[i - LOGC_NONE]))
			إرجاع i;
	id = uclass_get_by_name(name);
	إذا (id != UCLASS_INVALID)
		إرجاع (تعداد log_category_t)id;

	إرجاع LOGC_NONE;
}

ثابت حرف *log_get_level_name(تعداد log_level_t level)
{
	إذا (level >= LOGL_تعداد)
		إرجاع "INVALID";
	إرجاع log_level_name[level];
}

تعداد log_level_t log_get_level_by_name(ثابت حرف *name)
{
	صحيح i;

	لكل (i = 0; i < LOGL_تعداد; i++) {
		إذا (!strcasecmp(log_level_name[i], name))
			إرجاع i;
	}

	إرجاع LOGL_NONE;
}

بنية log_device *log_device_find_by_name(ثابت حرف *drv_name)
{
	بنية log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_عقدة) {
		إذا (!strcmp(drv_name, ldev->drv->name))
			إرجاع ldev;
	}

	إرجاع عدم;
}

bool log_has_cat(تعداد log_category_t cat_list[], تعداد log_category_t cat)
{
	صحيح i;

	لكل (i = 0; i < LOGF_MAX_CATEGORIES && cat_list[i] != LOGC_END; i++) {
		إذا (cat_list[i] == cat)
			إرجاع true;
	}

	إرجاع false;
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
		إذا (member_len >= substr_len &&
		    !strncmp(member + member_len - substr_len, s, substr_len))
			إرجاع true;
	}

	إرجاع false;
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

	إذا (rec->flags & LOGRECF_FORCE_DEBUG)
		إرجاع true;

	/* If there are no filters, filter on the default log level */
	إذا (list_empty(&ldev->filter_head)) {
		إذا (rec->level > gd->default_log_level)
			إرجاع false;
		إرجاع true;
	}

	list_for_each_entry(filt, &ldev->filter_head, sibling_عقدة) {
		إذا (filt->flags & LOGFF_LEVEL_MIN) {
			إذا (rec->level < filt->level)
				استمر;
		} وإلا إذا (rec->level > filt->level) {
			استمر;
		}

		إذا ((filt->flags & LOGFF_HAS_CAT) &&
		    !log_has_cat(filt->cat_list, rec->cat))
			استمر;

		إذا (filt->file_list &&
		    !log_has_member(filt->file_list, rec->file))
			استمر;

		إذا (filt->func_list &&
		    !log_has_member(filt->func_list, rec->func))
			استمر;

		إذا (filt->flags & LOGFF_DENY)
			إرجاع false;
		وإلا
			إرجاع true;
	}

	إرجاع false;
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
	إذا (gd->processing_رسالة)
		إرجاع 1;

	/* Emit message */
	gd->processing_رسالة = true;
	list_for_each_entry(ldev, &gd->log_head, sibling_عقدة) {
		إذا ((ldev->flags & LOGDF_ENABLE) &&
		    log_passes_filters(ldev, rec)) {
			إذا (!rec->رسالة) {
				صحيح len;

				len = vsnprintf(buf, حجم(buf), fmt, args);
				rec->رسالة = buf;
				gd->log_cont = len && buf[len - 1] != '\n';
			}
			ldev->drv->emit(ldev, rec);
		}
	}
	gd->processing_رسالة = false;
	إرجاع 0;
}

صحيح _log(تعداد log_category_t cat, تعداد log_level_t level, ثابت حرف *file,
	 صحيح سطر, ثابت حرف *func, ثابت حرف *fmt, ...)
{
	بنية log_rec rec;
	va_list args;

	إذا (!gd)
		إرجاع -ENOSYS;

	/* Check for message continuation */
	إذا (cat == LOGC_CONT)
		cat = gd->logc_prev;
	إذا (level == LOGL_CONT)
		level = gd->logl_prev;

	rec.cat = cat;
	rec.level = level & LOGL_LEVEL_MASK;
	rec.flags = 0;
	إذا (level & LOGL_FORCE_DEBUG)
		rec.flags |= LOGRECF_FORCE_DEBUG;
	إذا (gd->log_cont)
		rec.flags |= LOGRECF_CONT;
	rec.file = file;
	rec.سطر = سطر;
	rec.func = func;
	rec.رسالة = عدم;

	إذا (!(gd->flags & GD_FLG_LOG_READY)) {
		gd->log_drop_تعداد++;

		/* display dropped traces with console puts and DEBUG_UART */
		إذا (rec.level <= CONFIG_LOG_DEFAULT_LEVEL ||
		    rec.flags & LOGRECF_FORCE_DEBUG) {
			حرف buf[CONFIG_SYS_CBSIZE];

			va_start(args, fmt);
			vsnprintf(buf, حجم(buf), fmt, args);
			puts(buf);
			va_end(args);
		}

		إرجاع -ENOSYS;
	}
	va_start(args, fmt);
	إذا (!log_dispatch(&rec, fmt, args)) {
		gd->logc_prev = cat;
		gd->logl_prev = level;
	}
	va_end(args);

	إرجاع 0;
}

#define MAX_سطر_LENGTH_BYTES		64
#define DEFAULT_سطر_LENGTH_BYTES	16

صحيح _log_buffer(تعداد log_category_t cat, تعداد log_level_t level,
		ثابت حرف *file, صحيح سطر, ثابت حرف *func, ulong addr,
		ثابت فراغ *data, uint width, uint تعداد_2, uint linelen)
{
	إذا (linelen * width > MAX_سطر_LENGTH_BYTES)
		linelen = MAX_سطر_LENGTH_BYTES / width;
	إذا (linelen < 1)
		linelen = DEFAULT_سطر_LENGTH_BYTES / width;

	بينما (تعداد_2) {
		uint thislinelen;
		حرف buf[HEXDUMP_MAX_BUF_LENGTH(width * linelen)];

		thislinelen = hexdump_سطر(addr, data, width, تعداد_2, linelen,
					   buf, حجم(buf));
		أكد(thislinelen >= 0);
		_log(cat, level, file, سطر, func, "%s\n", buf);

		/* update references */
		data += thislinelen * width;
		addr += thislinelen * width;
		تعداد_2 -= thislinelen;
	}

	إرجاع 0;
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
	إذا (!ldev)
		إرجاع -ENOENT;
	filt = calloc(1, حجم(*filt));
	إذا (!filt)
		إرجاع -ENOMEM;

	filt->flags = flags;
	إذا (cat_list) {
		filt->flags |= LOGFF_HAS_CAT;
		لكل (i = 0; ; i++) {
			إذا (i == ARRAY_SIZE(filt->cat_list)) {
				ret = -ENOSPC;
				اذهب err;
			}
			filt->cat_list[i] = cat_list[i];
			إذا (cat_list[i] == LOGC_END)
				اكسر;
		}
	}
	filt->level = level;
	إذا (file_list) {
		filt->file_list = strdup(file_list);
		إذا (!filt->file_list) {
			ret = -ENOMEM;
			اذهب err;
		}
	}
	إذا (func_list) {
		filt->func_list = strdup(func_list);
		إذا (!filt->func_list) {
			ret = -ENOMEM;
			اذهب err;
		}
	}
	filt->filter_رقم = ldev->next_filter_رقم++;
	/* Add deny filters to the beginning of the list */
	إذا (flags & LOGFF_DENY)
		list_add(&filt->sibling_عقدة, &ldev->filter_head);
	وإلا
		list_add_tail(&filt->sibling_عقدة, &ldev->filter_head);

	إرجاع filt->filter_رقم;

err:
	حرر(filt);
	إرجاع ret;
}

صحيح log_remove_filter(ثابت حرف *drv_name, صحيح filter_رقم)
{
	بنية log_filter *filt;
	بنية log_device *ldev;

	ldev = log_device_find_by_name(drv_name);
	إذا (!ldev)
		إرجاع -ENOENT;

	list_for_each_entry(filt, &ldev->filter_head, sibling_عقدة) {
		إذا (filt->filter_رقم == filter_رقم) {
			list_del(&filt->sibling_عقدة);
			حرر(filt);

			إرجاع 0;
		}
	}

	إرجاع -ENOENT;
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

	list_for_each_entry(ldev, &gd->log_head, sibling_عقدة) {
		إذا (ldev->drv == drv)
			إرجاع ldev;
	}
	/*
	 * It is quite hard to pass an invalid driver since passing an unknown
	 * LOG_GET_DRIVER(xxx) would normally produce a compilation error. But
	 * it is possible to pass NULL, for example, so this
	 */

	إرجاع عدم;
}

صحيح log_device_set_enable(بنية log_driver *drv, bool enable)
{
	بنية log_device *ldev;

	ldev = log_find_device_by_drv(drv);
	إذا (!ldev)
		إرجاع -ENOENT;
	إذا (enable)
		ldev->flags |= LOGDF_ENABLE;
	وإلا
		ldev->flags &= ~LOGDF_ENABLE;

	إرجاع 0;
}

فراغ log_fixup_for_gd_move(بنية global_data *new_gd)
{
	new_gd->log_head.prev->next = &new_gd->log_head;
}

صحيح log_init(فراغ)
{
	بنية log_driver *drv = ll_entry_start(بنية log_driver, log_driver);
	ثابت صحيح تعداد_2 = ll_entry_تعداد(بنية log_driver, log_driver);
	بنية log_driver *end = drv + تعداد_2;

	/*
	 * We cannot add runtime data to the driver since it is likely stored
	 * in rodata. Instead, set up a 'device' corresponding to each driver.
	 * We only support having a single device for each driver.
	 */
	INIT_LIST_HEAD((بنية list_head *)&gd->log_head);
	بينما (drv < end) {
		بنية log_device *ldev;

		ldev = calloc(1, حجم(*ldev));
		إذا (!ldev) {
			debug("%s: Cannot allocate memory\n", __func__);
			إرجاع -ENOMEM;
		}
		INIT_LIST_HEAD(&ldev->filter_head);
		ldev->drv = drv;
		ldev->flags = drv->flags;
		list_add_tail(&ldev->sibling_عقدة,
			      (بنية list_head *)&gd->log_head);
		drv++;
	}
	gd->flags |= GD_FLG_LOG_READY;
	إذا (!gd->default_log_level)
		gd->default_log_level = CONFIG_LOG_DEFAULT_LEVEL;
	gd->log_fmt = log_get_default_format();
	gd->logc_prev = LOGC_NONE;
	gd->logl_prev = LOGL_INFO;

	إرجاع 0;
}
