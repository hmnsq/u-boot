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

DECLARE_GLOBAL_انکڑے_PTR;

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

_Static_assert(ARRAY_ناپ(log_cat_name) == LOGC_گنتی - LOGC_NONE,
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

_Static_assert(ARRAY_ناپ(log_level_name) == LOGL_گنتی, "log_level_name size");

/* All error responses MUST begin with '<' */
ثابت حرف *log_get_cat_name(تعداد log_category_t cat)
{
	ثابت حرف *name;

	إذا (cat < 0 || cat >= LOGC_گنتی)
		أرجع "<invalid>";
	إذا (cat >= LOGC_NONE)
		أرجع log_cat_name[cat - LOGC_NONE];

#إذا CONFIG_IS_ENABLED(DM)
	name = uclass_get_name((تعداد uclass_id)cat);
#وإلا
	name = عدم;
#جے_انت

	أرجع name ? name : "<missing>";
}

تعداد log_category_t log_get_cat_by_name(ثابت حرف *name)
{
	تعداد uclass_id id;
	صحيح i;

	لكل (i = LOGC_NONE; i < LOGC_گنتی; i++)
		إذا (!strcmp(name, log_cat_name[i - LOGC_NONE]))
			أرجع i;
	id = uclass_get_by_name(name);
	إذا (id != UCLASS_INVALID)
		أرجع (تعداد log_category_t)id;

	أرجع LOGC_NONE;
}

ثابت حرف *log_get_level_name(تعداد log_level_t level)
{
	إذا (level >= LOGL_گنتی)
		أرجع "INVALID";
	أرجع log_level_name[level];
}

تعداد log_level_t log_get_level_by_name(ثابت حرف *name)
{
	صحيح i;

	لكل (i = 0; i < LOGL_گنتی; i++) {
		إذا (!strcasecmp(log_level_name[i], name))
			أرجع i;
	}

	أرجع LOGL_NONE;
}

بنية log_device *log_device_لبھو_by_name(ثابت حرف *drv_name)
{
	بنية log_device *ldev;

	list_for_each_اندراج(ldev, &gd->log_سر, sibling_گرہ) {
		إذا (!strcmp(drv_name, ldev->drv->name))
			أرجع ldev;
	}

	أرجع عدم;
}

bool log_has_cat(تعداد log_category_t cat_list[], تعداد log_category_t cat)
{
	صحيح i;

	لكل (i = 0; i < LOGF_MAX_CATEGORIES && cat_list[i] != LOGC_سرا; i++) {
		إذا (cat_list[i] == cat)
			أرجع true;
	}

	أرجع false;
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
	صحيح member_len = لڑی_لمبائی(member);
	ثابت حرف *s, *p;
	صحيح substr_len;

	لكل (s = list; *s; s = p + (*p != '\0')) {
		p = strchrnul(s, ',');
		substr_len = p - s;
		إذا (member_len >= substr_len &&
		    !لڑی_مقابلہ(member + member_len - substr_len, s, substr_len))
			أرجع true;
	}

	أرجع false;
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
		أرجع true;

	/* If there are no filters, filter on the default log level */
	إذا (list_خالی_تھاواں(&ldev->filter_سر)) {
		إذا (rec->level > gd->default_log_level)
			أرجع false;
		أرجع true;
	}

	list_for_each_اندراج(filt, &ldev->filter_سر, sibling_گرہ) {
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
			أرجع false;
		وإلا
			أرجع true;
	}

	أرجع false;
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
ساكن صحيح log_dispatch(بنية log_rec *rec, ثابت حرف *fmt, va_list دلیلاں)
{
	بنية log_device *ldev;
	حرف buf[CONFIG_SYS_CBSIZE];

	/*
	 * When a log driver writes messages (e.g. via the network stack) this
	 * may result in further generated messages. We cannot process them here
	 * as this might result in infinite recursion.
	 */
	إذا (gd->processing_پیغام)
		أرجع 1;

	/* Emit message */
	gd->processing_پیغام = true;
	list_for_each_اندراج(ldev, &gd->log_سر, sibling_گرہ) {
		إذا ((ldev->flags & LOGDF_ENABLE) &&
		    log_passes_filters(ldev, rec)) {
			إذا (!rec->پیغام) {
				صحيح len;

				len = vsnprintf(buf, حجم(buf), fmt, دلیلاں);
				rec->پیغام = buf;
				gd->log_cont = len && buf[len - 1] != '\n';
			}
			ldev->drv->emit(ldev, rec);
		}
	}
	gd->processing_پیغام = false;
	أرجع 0;
}

صحيح _log(تعداد log_category_t cat, تعداد log_level_t level, ثابت حرف *file,
	 صحيح سطر, ثابت حرف *func, ثابت حرف *fmt, ...)
{
	بنية log_rec rec;
	va_list دلیلاں;

	إذا (!gd)
		أرجع -ENOSYS;

	/* Check for message continuation */
	إذا (cat == LOGC_CONT)
		cat = gd->logc_prev;
	إذا (level == LOGL_CONT)
		level = gd->logl_prev;

	rec.cat = cat;
	rec.level = level & LOGL_LEVEL_نقاب;
	rec.flags = 0;
	إذا (level & LOGL_FORCE_DEBUG)
		rec.flags |= LOGRECF_FORCE_DEBUG;
	إذا (gd->log_cont)
		rec.flags |= LOGRECF_CONT;
	rec.file = file;
	rec.سطر = سطر;
	rec.func = func;
	rec.پیغام = عدم;

	إذا (!(gd->flags & GD_FLG_LOG_READY)) {
		gd->log_drop_گنتی++;

		/* display dropped traces with console puts and DEBUG_UART */
		إذا (rec.level <= CONFIG_LOG_DEFAULT_LEVEL ||
		    rec.flags & LOGRECF_FORCE_DEBUG) {
			حرف buf[CONFIG_SYS_CBSIZE];

			va_شروع(دلیلاں, fmt);
			vsnprintf(buf, حجم(buf), fmt, دلیلاں);
			puts(buf);
			va_سرا(دلیلاں);
		}

		أرجع -ENOSYS;
	}
	va_شروع(دلیلاں, fmt);
	إذا (!log_dispatch(&rec, fmt, دلیلاں)) {
		gd->logc_prev = cat;
		gd->logl_prev = level;
	}
	va_سرا(دلیلاں);

	أرجع 0;
}

#تعریف_کرو MAX_سطر_لمائی_بائٹاں		64
#تعریف_کرو DEFAULT_سطر_لمائی_بائٹاں	16

صحيح log_بفر(تعداد log_category_t cat, تعداد log_level_t level,
		ثابت حرف *file, صحيح سطر, ثابت حرف *func, ulong addr,
		ثابت فراغ *انکڑے, uint چوڑائی, uint گنتی, uint linelen)
{
	إذا (linelen * چوڑائی > MAX_سطر_لمائی_بائٹاں)
		linelen = MAX_سطر_لمائی_بائٹاں / چوڑائی;
	إذا (linelen < 1)
		linelen = DEFAULT_سطر_لمائی_بائٹاں / چوڑائی;

	طالما (گنتی) {
		uint thislinelen;
		حرف buf[HEXDUMP_MAX_BUF_لمائی(چوڑائی * linelen)];

		thislinelen = hexdump_سطر(addr, انکڑے, چوڑائی, گنتی, linelen,
					   buf, حجم(buf));
		أكد(thislinelen >= 0);
		_log(cat, level, file, سطر, func, "%s\n", buf);

		/* update references */
		انکڑے += thislinelen * چوڑائی;
		addr += thislinelen * چوڑائی;
		گنتی -= thislinelen;
	}

	أرجع 0;
}

صحيح log_جوڑو_filter_flags(ثابت حرف *drv_name, تعداد log_category_t cat_list[],
			 تعداد log_level_t level, ثابت حرف *file_list,
			 ثابت حرف *func_list, صحيح flags)
{
	بنية log_filter *filt;
	بنية log_device *ldev;
	صحيح ret;
	صحيح i;

	ldev = log_device_لبھو_by_name(drv_name);
	إذا (!ldev)
		أرجع -ENOENT;
	filt = صفر_حافظہ_دیو(1, حجم(*filt));
	إذا (!filt)
		أرجع -ENOMEM;

	filt->flags = flags;
	إذا (cat_list) {
		filt->flags |= LOGFF_HAS_CAT;
		لكل (i = 0; ; i++) {
			إذا (i == ARRAY_ناپ(filt->cat_list)) {
				ret = -ENOSPC;
				اذهب err;
			}
			filt->cat_list[i] = cat_list[i];
			إذا (cat_list[i] == LOGC_سرا)
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
	filt->filter_عدد = ldev->اگلا_filter_عدد++;
	/* Add deny filters to the beginning of the list */
	إذا (flags & LOGFF_DENY)
		list_جوڑو(&filt->sibling_گرہ, &ldev->filter_سر);
	وإلا
		list_جوڑو_پوچھ(&filt->sibling_گرہ, &ldev->filter_سر);

	أرجع filt->filter_عدد;

err:
	حرر(filt);
	أرجع ret;
}

صحيح log_remove_filter(ثابت حرف *drv_name, صحيح filter_عدد)
{
	بنية log_filter *filt;
	بنية log_device *ldev;

	ldev = log_device_لبھو_by_name(drv_name);
	إذا (!ldev)
		أرجع -ENOENT;

	list_for_each_اندراج(filt, &ldev->filter_سر, sibling_گرہ) {
		إذا (filt->filter_عدد == filter_عدد) {
			list_del(&filt->sibling_گرہ);
			حرر(filt);

			أرجع 0;
		}
	}

	أرجع -ENOENT;
}

/**
 * log_find_device_by_drv() - Find a device by its driver
 *
 * @drv: Log driver
 * Return: Device associated with that driver, or NULL if not found
 */
ساكن بنية log_device *log_لبھو_device_by_drv(بنية log_driver *drv)
{
	بنية log_device *ldev;

	list_for_each_اندراج(ldev, &gd->log_سر, sibling_گرہ) {
		إذا (ldev->drv == drv)
			أرجع ldev;
	}
	/*
	 * It is quite hard to pass an invalid driver since passing an unknown
	 * LOG_GET_DRIVER(xxx) would normally produce a compilation error. But
	 * it is possible to pass NULL, for example, so this
	 */

	أرجع عدم;
}

صحيح log_device_سیٹ_enable(بنية log_driver *drv, bool enable)
{
	بنية log_device *ldev;

	ldev = log_لبھو_device_by_drv(drv);
	إذا (!ldev)
		أرجع -ENOENT;
	إذا (enable)
		ldev->flags |= LOGDF_ENABLE;
	وإلا
		ldev->flags &= ~LOGDF_ENABLE;

	أرجع 0;
}

فراغ log_fixup_for_gd_move(بنية global_انکڑے *new_gd)
{
	new_gd->log_سر.prev->اگلا = &new_gd->log_سر;
}

صحيح log_init(فراغ)
{
	بنية log_driver *drv = ll_اندراج_شروع(بنية log_driver, log_driver);
	ثابت صحيح گنتی = ll_اندراج_گنتی(بنية log_driver, log_driver);
	بنية log_driver *سرا_2 = drv + گنتی;

	/*
	 * We cannot add runtime data to the driver since it is likely stored
	 * in rodata. Instead, set up a 'device' corresponding to each driver.
	 * We only support having a single device for each driver.
	 */
	INIT_LIST_سر((بنية list_سر *)&gd->log_سر);
	طالما (drv < سرا_2) {
		بنية log_device *ldev;

		ldev = صفر_حافظہ_دیو(1, حجم(*ldev));
		إذا (!ldev) {
			debug("%s: Cannot allocate memory\n", __func__);
			أرجع -ENOMEM;
		}
		INIT_LIST_سر(&ldev->filter_سر);
		ldev->drv = drv;
		ldev->flags = drv->flags;
		list_جوڑو_پوچھ(&ldev->sibling_گرہ,
			      (بنية list_سر *)&gd->log_سر);
		drv++;
	}
	gd->flags |= GD_FLG_LOG_READY;
	إذا (!gd->default_log_level)
		gd->default_log_level = CONFIG_LOG_DEFAULT_LEVEL;
	gd->log_fmt = log_get_default_format();
	gd->logc_prev = LOGC_NONE;
	gd->logl_prev = LOGL_INFO;

	أرجع 0;
}
