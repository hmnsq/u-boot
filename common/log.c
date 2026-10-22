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

DECLARE_GLOBAL_ڈیٹا_PTR;

ساکن ثابت حرف *ثابت log_cat_name[] = {
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

_Static_assert(ARRAY_سائز(log_cat_name) == LOGC_گنتی - LOGC_NONE,
	       "log_cat_name size");

ساکن ثابت حرف *ثابت log_level_name[] = {
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

_Static_assert(ARRAY_سائز(log_level_name) == LOGL_گنتی, "log_level_name size");

/* All error responses MUST begin with '<' */
ثابت حرف *log_get_cat_name(شمار log_category_t cat)
{
	ثابت حرف *name;

	اگر (cat < 0 || cat >= LOGC_گنتی)
		واپس "<invalid>";
	اگر (cat >= LOGC_NONE)
		واپس log_cat_name[cat - LOGC_NONE];

#اگر CONFIG_IS_ENABLED(DM)
	name = uclass_get_name((شمار uclass_id)cat);
#ورنہ
	name = تہی;
#اگر_ختم

	واپس name ? name : "<missing>";
}

شمار log_category_t log_get_cat_by_name(ثابت حرف *name)
{
	شمار uclass_id id;
	صحیح i;

	برائے (i = LOGC_NONE; i < LOGC_گنتی; i++)
		اگر (!strcmp(name, log_cat_name[i - LOGC_NONE]))
			واپس i;
	id = uclass_get_by_name(name);
	اگر (id != UCLASS_INVALID)
		واپس (شمار log_category_t)id;

	واپس LOGC_NONE;
}

ثابت حرف *log_get_level_name(شمار log_level_t level)
{
	اگر (level >= LOGL_گنتی)
		واپس "INVALID";
	واپس log_level_name[level];
}

شمار log_level_t log_get_level_by_name(ثابت حرف *name)
{
	صحیح i;

	برائے (i = 0; i < LOGL_گنتی; i++) {
		اگر (!strcasecmp(log_level_name[i], name))
			واپس i;
	}

	واپس LOGL_NONE;
}

ساخت log_device *log_device_ڈھونڈو_by_name(ثابت حرف *drv_name)
{
	ساخت log_device *ldev;

	list_for_each_اندراج(ldev, &gd->log_سر, sibling_گرہ) {
		اگر (!strcmp(drv_name, ldev->drv->name))
			واپس ldev;
	}

	واپس تہی;
}

bool log_has_cat(شمار log_category_t cat_list[], شمار log_category_t cat)
{
	صحیح i;

	برائے (i = 0; i < LOGF_MAX_CATEGORIES && cat_list[i] != LOGC_انتہا; i++) {
		اگر (cat_list[i] == cat)
			واپس true;
	}

	واپس false;
}

/**
 * log_has_member() - check if a string is in a comma separated list
 *
 * @list:	Comma separated list of strings
 * @member:	String to find
 *
 * Return: ``true`` if @member is in @list, else ``false``
 */
ساکن bool log_has_member(ثابت حرف *list, ثابت حرف *member)
{
	صحیح member_len = لڑی_لمبائی(member);
	ثابت حرف *s, *p;
	صحیح substr_len;

	برائے (s = list; *s; s = p + (*p != '\0')) {
		p = strchrnul(s, ',');
		substr_len = p - s;
		اگر (member_len >= substr_len &&
		    !لڑی_موازنہ(member + member_len - substr_len, s, substr_len))
			واپس true;
	}

	واپس false;
}

/**
 * log_passes_filters() - check if a log record passes the filters for a device
 *
 * @ldev: Log device to check
 * @rec: Log record to check
 * Return: true if @rec is not blocked by the filters in @ldev, false if it is
 */
ساکن bool log_passes_filters(ساخت log_device *ldev, ساخت log_rec *rec)
{
	ساخت log_filter *filt;

	اگر (rec->flags & LOGRECF_FORCE_DEBUG)
		واپس true;

	/* If there are no filters, filter on the default log level */
	اگر (list_خالی_خانے(&ldev->filter_سر)) {
		اگر (rec->level > gd->default_log_level)
			واپس false;
		واپس true;
	}

	list_for_each_اندراج(filt, &ldev->filter_سر, sibling_گرہ) {
		اگر (filt->flags & LOGFF_LEVEL_MIN) {
			اگر (rec->level < filt->level)
				جاری;
		} ورنہ اگر (rec->level > filt->level) {
			جاری;
		}

		اگر ((filt->flags & LOGFF_HAS_CAT) &&
		    !log_has_cat(filt->cat_list, rec->cat))
			جاری;

		اگر (filt->file_list &&
		    !log_has_member(filt->file_list, rec->file))
			جاری;

		اگر (filt->func_list &&
		    !log_has_member(filt->func_list, rec->func))
			جاری;

		اگر (filt->flags & LOGFF_DENY)
			واپس false;
		ورنہ
			واپس true;
	}

	واپس false;
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
ساکن صحیح log_dispatch(ساخت log_rec *rec, ثابت حرف *fmt, va_list دلائل)
{
	ساخت log_device *ldev;
	حرف buf[CONFIG_SYS_CBSIZE];

	/*
	 * When a log driver writes messages (e.g. via the network stack) this
	 * may result in further generated messages. We cannot process them here
	 * as this might result in infinite recursion.
	 */
	اگر (gd->processing_پیغام)
		واپس 1;

	/* Emit message */
	gd->processing_پیغام = true;
	list_for_each_اندراج(ldev, &gd->log_سر, sibling_گرہ) {
		اگر ((ldev->flags & LOGDF_ENABLE) &&
		    log_passes_filters(ldev, rec)) {
			اگر (!rec->پیغام) {
				صحیح len;

				len = vsnprintf(buf, حجم(buf), fmt, دلائل);
				rec->پیغام = buf;
				gd->log_cont = len && buf[len - 1] != '\n';
			}
			ldev->drv->emit(ldev, rec);
		}
	}
	gd->processing_پیغام = false;
	واپس 0;
}

صحیح _log(شمار log_category_t cat, شمار log_level_t level, ثابت حرف *file,
	 صحیح سطر, ثابت حرف *func, ثابت حرف *fmt, ...)
{
	ساخت log_rec rec;
	va_list دلائل;

	اگر (!gd)
		واپس -ENOSYS;

	/* Check for message continuation */
	اگر (cat == LOGC_CONT)
		cat = gd->logc_پچھلا;
	اگر (level == LOGL_CONT)
		level = gd->logl_پچھلا;

	rec.cat = cat;
	rec.level = level & LOGL_LEVEL_نقاب;
	rec.flags = 0;
	اگر (level & LOGL_FORCE_DEBUG)
		rec.flags |= LOGRECF_FORCE_DEBUG;
	اگر (gd->log_cont)
		rec.flags |= LOGRECF_CONT;
	rec.file = file;
	rec.سطر = سطر;
	rec.func = func;
	rec.پیغام = تہی;

	اگر (!(gd->flags & GD_FLG_LOG_READY)) {
		gd->log_drop_گنتی++;

		/* display dropped traces with console puts and DEBUG_UART */
		اگر (rec.level <= CONFIG_LOG_DEFAULT_LEVEL ||
		    rec.flags & LOGRECF_FORCE_DEBUG) {
			حرف buf[CONFIG_SYS_CBSIZE];

			va_آغاز(دلائل, fmt);
			vsnprintf(buf, حجم(buf), fmt, دلائل);
			puts(buf);
			va_انتہا(دلائل);
		}

		واپس -ENOSYS;
	}
	va_آغاز(دلائل, fmt);
	اگر (!log_dispatch(&rec, fmt, دلائل)) {
		gd->logc_پچھلا = cat;
		gd->logl_پچھلا = level;
	}
	va_انتہا(دلائل);

	واپس 0;
}

#تعریف_کرو MAX_سطر_لمبائی_بائٹیں		64
#تعریف_کرو DEFAULT_سطر_لمبائی_بائٹیں	16

صحیح log_بفر(شمار log_category_t cat, شمار log_level_t level,
		ثابت حرف *file, صحیح سطر, ثابت حرف *func, ulong addr,
		ثابت خالی *ڈیٹا, uint چوڑائی, uint گنتی, uint linelen)
{
	اگر (linelen * چوڑائی > MAX_سطر_لمبائی_بائٹیں)
		linelen = MAX_سطر_لمبائی_بائٹیں / چوڑائی;
	اگر (linelen < 1)
		linelen = DEFAULT_سطر_لمبائی_بائٹیں / چوڑائی;

	جب (گنتی) {
		uint thislinelen;
		حرف buf[HEXDUMP_MAX_BUF_لمبائی(چوڑائی * linelen)];

		thislinelen = hexdump_سطر(addr, ڈیٹا, چوڑائی, گنتی, linelen,
					   buf, حجم(buf));
		أكد(thislinelen >= 0);
		_log(cat, level, file, سطر, func, "%s\n", buf);

		/* update references */
		ڈیٹا += thislinelen * چوڑائی;
		addr += thislinelen * چوڑائی;
		گنتی -= thislinelen;
	}

	واپس 0;
}

صحیح log_جوڑو_filter_flags(ثابت حرف *drv_name, شمار log_category_t cat_list[],
			 شمار log_level_t level, ثابت حرف *file_list,
			 ثابت حرف *func_list, صحیح flags)
{
	ساخت log_filter *filt;
	ساخت log_device *ldev;
	صحیح ret;
	صحیح i;

	ldev = log_device_ڈھونڈو_by_name(drv_name);
	اگر (!ldev)
		واپس -ENOENT;
	filt = صفر_حافظہ_دو(1, حجم(*filt));
	اگر (!filt)
		واپس -ENOMEM;

	filt->flags = flags;
	اگر (cat_list) {
		filt->flags |= LOGFF_HAS_CAT;
		برائے (i = 0; ; i++) {
			اگر (i == ARRAY_سائز(filt->cat_list)) {
				ret = -ENOSPC;
				جاؤ err;
			}
			filt->cat_list[i] = cat_list[i];
			اگر (cat_list[i] == LOGC_انتہا)
				توڑو;
		}
	}
	filt->level = level;
	اگر (file_list) {
		filt->file_list = strdup(file_list);
		اگر (!filt->file_list) {
			ret = -ENOMEM;
			جاؤ err;
		}
	}
	اگر (func_list) {
		filt->func_list = strdup(func_list);
		اگر (!filt->func_list) {
			ret = -ENOMEM;
			جاؤ err;
		}
	}
	filt->filter_عدد = ldev->اگلا_filter_عدد++;
	/* Add deny filters to the beginning of the list */
	اگر (flags & LOGFF_DENY)
		list_جوڑو(&filt->sibling_گرہ, &ldev->filter_سر);
	ورنہ
		list_جوڑو_دم(&filt->sibling_گرہ, &ldev->filter_سر);

	واپس filt->filter_عدد;

err:
	آزاد(filt);
	واپس ret;
}

صحیح log_remove_filter(ثابت حرف *drv_name, صحیح filter_عدد)
{
	ساخت log_filter *filt;
	ساخت log_device *ldev;

	ldev = log_device_ڈھونڈو_by_name(drv_name);
	اگر (!ldev)
		واپس -ENOENT;

	list_for_each_اندراج(filt, &ldev->filter_سر, sibling_گرہ) {
		اگر (filt->filter_عدد == filter_عدد) {
			list_del(&filt->sibling_گرہ);
			آزاد(filt);

			واپس 0;
		}
	}

	واپس -ENOENT;
}

/**
 * log_find_device_by_drv() - Find a device by its driver
 *
 * @drv: Log driver
 * Return: Device associated with that driver, or NULL if not found
 */
ساکن ساخت log_device *log_ڈھونڈو_device_by_drv(ساخت log_driver *drv)
{
	ساخت log_device *ldev;

	list_for_each_اندراج(ldev, &gd->log_سر, sibling_گرہ) {
		اگر (ldev->drv == drv)
			واپس ldev;
	}
	/*
	 * It is quite hard to pass an invalid driver since passing an unknown
	 * LOG_GET_DRIVER(xxx) would normally produce a compilation error. But
	 * it is possible to pass NULL, for example, so this
	 */

	واپس تہی;
}

صحیح log_device_سیٹ_enable(ساخت log_driver *drv, bool enable)
{
	ساخت log_device *ldev;

	ldev = log_ڈھونڈو_device_by_drv(drv);
	اگر (!ldev)
		واپس -ENOENT;
	اگر (enable)
		ldev->flags |= LOGDF_ENABLE;
	ورنہ
		ldev->flags &= ~LOGDF_ENABLE;

	واپس 0;
}

خالی log_fixup_for_gd_move(ساخت global_ڈیٹا *new_gd)
{
	new_gd->log_سر.پچھلا->اگلا = &new_gd->log_سر;
}

صحیح log_init(خالی)
{
	ساخت log_driver *drv = ll_اندراج_آغاز(ساخت log_driver, log_driver);
	ثابت صحیح گنتی = ll_اندراج_گنتی(ساخت log_driver, log_driver);
	ساخت log_driver *انتہا_2 = drv + گنتی;

	/*
	 * We cannot add runtime data to the driver since it is likely stored
	 * in rodata. Instead, set up a 'device' corresponding to each driver.
	 * We only support having a single device for each driver.
	 */
	INIT_LIST_سر((ساخت list_سر *)&gd->log_سر);
	جب (drv < انتہا_2) {
		ساخت log_device *ldev;

		ldev = صفر_حافظہ_دو(1, حجم(*ldev));
		اگر (!ldev) {
			debug("%s: Cannot allocate memory\n", __func__);
			واپس -ENOMEM;
		}
		INIT_LIST_سر(&ldev->filter_سر);
		ldev->drv = drv;
		ldev->flags = drv->flags;
		list_جوڑو_دم(&ldev->sibling_گرہ,
			      (ساخت list_سر *)&gd->log_سر);
		drv++;
	}
	gd->flags |= GD_FLG_LOG_READY;
	اگر (!gd->default_log_level)
		gd->default_log_level = CONFIG_LOG_DEFAULT_LEVEL;
	gd->log_fmt = log_get_default_format();
	gd->logc_پچھلا = LOGC_NONE;
	gd->logl_پچھلا = LOGL_INFO;

	واپس 0;
}
