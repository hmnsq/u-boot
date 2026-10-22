// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <display_options.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm/uclass.h>

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ log_cat_name[] = {
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

_Static_assert(ARRAY_ਮਾਪ(log_cat_name) == LOGC_ਗਿਣਤੀ - LOGC_NONE,
	       "log_cat_name size");

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ log_level_name[] = {
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

_Static_assert(ARRAY_ਮਾਪ(log_level_name) == LOGL_ਗਿਣਤੀ, "log_level_name size");

/* All error responses MUST begin with '<' */
ਅਚਲ ਅੱਖਰ *log_get_cat_name(ਗਿਣਤੀ log_category_t cat)
{
	ਅਚਲ ਅੱਖਰ *name;

	ਜੇ (cat < 0 || cat >= LOGC_ਗਿਣਤੀ)
		ਵਾਪਸ "<invalid>";
	ਜੇ (cat >= LOGC_NONE)
		ਵਾਪਸ log_cat_name[cat - LOGC_NONE];

#ਜੇ CONFIG_IS_ENABLED(DM)
	name = uclass_get_name((ਗਿਣਤੀ uclass_id)cat);
#ਨਹੀਂਤਾਂ
	name = ਨਿਰਾ;
#ਜੇ_ਅੰਤ

	ਵਾਪਸ name ? name : "<missing>";
}

ਗਿਣਤੀ log_category_t log_get_cat_by_name(ਅਚਲ ਅੱਖਰ *name)
{
	ਗਿਣਤੀ uclass_id id;
	ਪੂਰਨ i;

	ਲਈ (i = LOGC_NONE; i < LOGC_ਗਿਣਤੀ; i++)
		ਜੇ (!strcmp(name, log_cat_name[i - LOGC_NONE]))
			ਵਾਪਸ i;
	id = uclass_get_by_name(name);
	ਜੇ (id != UCLASS_INVALID)
		ਵਾਪਸ (ਗਿਣਤੀ log_category_t)id;

	ਵਾਪਸ LOGC_NONE;
}

ਅਚਲ ਅੱਖਰ *log_get_level_name(ਗਿਣਤੀ log_level_t level)
{
	ਜੇ (level >= LOGL_ਗਿਣਤੀ)
		ਵਾਪਸ "INVALID";
	ਵਾਪਸ log_level_name[level];
}

ਗਿਣਤੀ log_level_t log_get_level_by_name(ਅਚਲ ਅੱਖਰ *name)
{
	ਪੂਰਨ i;

	ਲਈ (i = 0; i < LOGL_ਗਿਣਤੀ; i++) {
		ਜੇ (!strcasecmp(log_level_name[i], name))
			ਵਾਪਸ i;
	}

	ਵਾਪਸ LOGL_NONE;
}

ਬਣਤਰ log_device *log_device_ਲੱਭੋ_by_name(ਅਚਲ ਅੱਖਰ *drv_name)
{
	ਬਣਤਰ log_device *ldev;

	list_for_each_ਇੰਦਰਾਜ(ldev, &gd->log_ਸਿਰ, sibling_ਗੰਢ) {
		ਜੇ (!strcmp(drv_name, ldev->drv->name))
			ਵਾਪਸ ldev;
	}

	ਵਾਪਸ ਨਿਰਾ;
}

bool log_has_cat(ਗਿਣਤੀ log_category_t cat_list[], ਗਿਣਤੀ log_category_t cat)
{
	ਪੂਰਨ i;

	ਲਈ (i = 0; i < LOGF_MAX_CATEGORIES && cat_list[i] != LOGC_ਸਿਰਾ; i++) {
		ਜੇ (cat_list[i] == cat)
			ਵਾਪਸ true;
	}

	ਵਾਪਸ false;
}

/**
 * log_has_member() - check if a string is in a comma separated list
 *
 * @list:	Comma separated list of strings
 * @member:	String to find
 *
 * Return: ``true`` if @member is in @list, else ``false``
 */
ਸਥਿਰ bool log_has_member(ਅਚਲ ਅੱਖਰ *list, ਅਚਲ ਅੱਖਰ *member)
{
	ਪੂਰਨ member_len = ਲੜੀ_ਲੰਬਾਈ(member);
	ਅਚਲ ਅੱਖਰ *s, *p;
	ਪੂਰਨ substr_len;

	ਲਈ (s = list; *s; s = p + (*p != '\0')) {
		p = strchrnul(s, ',');
		substr_len = p - s;
		ਜੇ (member_len >= substr_len &&
		    !ਲੜੀ_ਤੁਲਨਾ(member + member_len - substr_len, s, substr_len))
			ਵਾਪਸ true;
	}

	ਵਾਪਸ false;
}

/**
 * log_passes_filters() - check if a log record passes the filters for a device
 *
 * @ldev: Log device to check
 * @rec: Log record to check
 * Return: true if @rec is not blocked by the filters in @ldev, false if it is
 */
ਸਥਿਰ bool log_passes_filters(ਬਣਤਰ log_device *ldev, ਬਣਤਰ log_rec *rec)
{
	ਬਣਤਰ log_filter *filt;

	ਜੇ (rec->flags & LOGRECF_FORCE_DEBUG)
		ਵਾਪਸ true;

	/* If there are no filters, filter on the default log level */
	ਜੇ (list_ਖਾਲੀ_ਥਾਵਾਂ(&ldev->filter_ਸਿਰ)) {
		ਜੇ (rec->level > gd->default_log_level)
			ਵਾਪਸ false;
		ਵਾਪਸ true;
	}

	list_for_each_ਇੰਦਰਾਜ(filt, &ldev->filter_ਸਿਰ, sibling_ਗੰਢ) {
		ਜੇ (filt->flags & LOGFF_LEVEL_MIN) {
			ਜੇ (rec->level < filt->level)
				ਜਾਰੀ;
		} ਨਹੀਂਤਾਂ ਜੇ (rec->level > filt->level) {
			ਜਾਰੀ;
		}

		ਜੇ ((filt->flags & LOGFF_HAS_CAT) &&
		    !log_has_cat(filt->cat_list, rec->cat))
			ਜਾਰੀ;

		ਜੇ (filt->file_list &&
		    !log_has_member(filt->file_list, rec->file))
			ਜਾਰੀ;

		ਜੇ (filt->func_list &&
		    !log_has_member(filt->func_list, rec->func))
			ਜਾਰੀ;

		ਜੇ (filt->flags & LOGFF_DENY)
			ਵਾਪਸ false;
		ਨਹੀਂਤਾਂ
			ਵਾਪਸ true;
	}

	ਵਾਪਸ false;
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
ਸਥਿਰ ਪੂਰਨ log_dispatch(ਬਣਤਰ log_rec *rec, ਅਚਲ ਅੱਖਰ *fmt, va_list ਦਲੀਲਾਂ)
{
	ਬਣਤਰ log_device *ldev;
	ਅੱਖਰ buf[CONFIG_SYS_CBSIZE];

	/*
	 * When a log driver writes messages (e.g. via the network stack) this
	 * may result in further generated messages. We cannot process them here
	 * as this might result in infinite recursion.
	 */
	ਜੇ (gd->processing_ਸੁਨੇਹਾ)
		ਵਾਪਸ 1;

	/* Emit message */
	gd->processing_ਸੁਨੇਹਾ = true;
	list_for_each_ਇੰਦਰਾਜ(ldev, &gd->log_ਸਿਰ, sibling_ਗੰਢ) {
		ਜੇ ((ldev->flags & LOGDF_ENABLE) &&
		    log_passes_filters(ldev, rec)) {
			ਜੇ (!rec->ਸੁਨੇਹਾ) {
				ਪੂਰਨ len;

				len = vsnprintf(buf, ਆਕਾਰ(buf), fmt, ਦਲੀਲਾਂ);
				rec->ਸੁਨੇਹਾ = buf;
				gd->log_cont = len && buf[len - 1] != '\n';
			}
			ldev->drv->emit(ldev, rec);
		}
	}
	gd->processing_ਸੁਨੇਹਾ = false;
	ਵਾਪਸ 0;
}

ਪੂਰਨ _log(ਗਿਣਤੀ log_category_t cat, ਗਿਣਤੀ log_level_t level, ਅਚਲ ਅੱਖਰ *file,
	 ਪੂਰਨ ਸਤਰ, ਅਚਲ ਅੱਖਰ *func, ਅਚਲ ਅੱਖਰ *fmt, ...)
{
	ਬਣਤਰ log_rec rec;
	va_list ਦਲੀਲਾਂ;

	ਜੇ (!gd)
		ਵਾਪਸ -ENOSYS;

	/* Check for message continuation */
	ਜੇ (cat == LOGC_CONT)
		cat = gd->logc_prev;
	ਜੇ (level == LOGL_CONT)
		level = gd->logl_prev;

	rec.cat = cat;
	rec.level = level & LOGL_LEVEL_ਨਕਾਬ;
	rec.flags = 0;
	ਜੇ (level & LOGL_FORCE_DEBUG)
		rec.flags |= LOGRECF_FORCE_DEBUG;
	ਜੇ (gd->log_cont)
		rec.flags |= LOGRECF_CONT;
	rec.file = file;
	rec.ਸਤਰ = ਸਤਰ;
	rec.func = func;
	rec.ਸੁਨੇਹਾ = ਨਿਰਾ;

	ਜੇ (!(gd->flags & GD_FLG_LOG_READY)) {
		gd->log_drop_ਗਿਣਤੀ++;

		/* display dropped traces with console puts and DEBUG_UART */
		ਜੇ (rec.level <= CONFIG_LOG_DEFAULT_LEVEL ||
		    rec.flags & LOGRECF_FORCE_DEBUG) {
			ਅੱਖਰ buf[CONFIG_SYS_CBSIZE];

			va_ਸ਼ੁਰੂ(ਦਲੀਲਾਂ, fmt);
			vsnprintf(buf, ਆਕਾਰ(buf), fmt, ਦਲੀਲਾਂ);
			puts(buf);
			va_ਸਿਰਾ(ਦਲੀਲਾਂ);
		}

		ਵਾਪਸ -ENOSYS;
	}
	va_ਸ਼ੁਰੂ(ਦਲੀਲਾਂ, fmt);
	ਜੇ (!log_dispatch(&rec, fmt, ਦਲੀਲਾਂ)) {
		gd->logc_prev = cat;
		gd->logl_prev = level;
	}
	va_ਸਿਰਾ(ਦਲੀਲਾਂ);

	ਵਾਪਸ 0;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_ਸਤਰ_ਲੰਮਾਈ_ਬਾਈਟਾਂ		64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEFAULT_ਸਤਰ_ਲੰਮਾਈ_ਬਾਈਟਾਂ	16

ਪੂਰਨ log_ਬਫਰ(ਗਿਣਤੀ log_category_t cat, ਗਿਣਤੀ log_level_t level,
		ਅਚਲ ਅੱਖਰ *file, ਪੂਰਨ ਸਤਰ, ਅਚਲ ਅੱਖਰ *func, ulong addr,
		ਅਚਲ ਖਾਲੀ *ਅੰਕੜੇ, uint ਚੌੜਾਈ, uint ਗਿਣਤੀ_2, uint linelen)
{
	ਜੇ (linelen * ਚੌੜਾਈ > MAX_ਸਤਰ_ਲੰਮਾਈ_ਬਾਈਟਾਂ)
		linelen = MAX_ਸਤਰ_ਲੰਮਾਈ_ਬਾਈਟਾਂ / ਚੌੜਾਈ;
	ਜੇ (linelen < 1)
		linelen = DEFAULT_ਸਤਰ_ਲੰਮਾਈ_ਬਾਈਟਾਂ / ਚੌੜਾਈ;

	ਜਦੋਂ (ਗਿਣਤੀ_2) {
		uint thislinelen;
		ਅੱਖਰ buf[HEXDUMP_MAX_BUF_ਲੰਮਾਈ(ਚੌੜਾਈ * linelen)];

		thislinelen = hexdump_ਸਤਰ(addr, ਅੰਕੜੇ, ਚੌੜਾਈ, ਗਿਣਤੀ_2, linelen,
					   buf, ਆਕਾਰ(buf));
		ਪ੍ਰਤਿਜ੍ਞਾ(thislinelen >= 0);
		_log(cat, level, file, ਸਤਰ, func, "%s\n", buf);

		/* update references */
		ਅੰਕੜੇ += thislinelen * ਚੌੜਾਈ;
		addr += thislinelen * ਚੌੜਾਈ;
		ਗਿਣਤੀ_2 -= thislinelen;
	}

	ਵਾਪਸ 0;
}

ਪੂਰਨ log_ਜੋੜੋ_filter_flags(ਅਚਲ ਅੱਖਰ *drv_name, ਗਿਣਤੀ log_category_t cat_list[],
			 ਗਿਣਤੀ log_level_t level, ਅਚਲ ਅੱਖਰ *file_list,
			 ਅਚਲ ਅੱਖਰ *func_list, ਪੂਰਨ flags)
{
	ਬਣਤਰ log_filter *filt;
	ਬਣਤਰ log_device *ldev;
	ਪੂਰਨ ret;
	ਪੂਰਨ i;

	ldev = log_device_ਲੱਭੋ_by_name(drv_name);
	ਜੇ (!ldev)
		ਵਾਪਸ -ENOENT;
	filt = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(*filt));
	ਜੇ (!filt)
		ਵਾਪਸ -ENOMEM;

	filt->flags = flags;
	ਜੇ (cat_list) {
		filt->flags |= LOGFF_HAS_CAT;
		ਲਈ (i = 0; ; i++) {
			ਜੇ (i == ARRAY_ਮਾਪ(filt->cat_list)) {
				ret = -ENOSPC;
				ਜਾਓ err;
			}
			filt->cat_list[i] = cat_list[i];
			ਜੇ (cat_list[i] == LOGC_ਸਿਰਾ)
				ਰੋਕੋ;
		}
	}
	filt->level = level;
	ਜੇ (file_list) {
		filt->file_list = strdup(file_list);
		ਜੇ (!filt->file_list) {
			ret = -ENOMEM;
			ਜਾਓ err;
		}
	}
	ਜੇ (func_list) {
		filt->func_list = strdup(func_list);
		ਜੇ (!filt->func_list) {
			ret = -ENOMEM;
			ਜਾਓ err;
		}
	}
	filt->filter_ਅੰਕ = ldev->ਅਗਲਾ_filter_ਅੰਕ++;
	/* Add deny filters to the beginning of the list */
	ਜੇ (flags & LOGFF_DENY)
		list_ਜੋੜੋ(&filt->sibling_ਗੰਢ, &ldev->filter_ਸਿਰ);
	ਨਹੀਂਤਾਂ
		list_ਜੋੜੋ_ਪੂਛ(&filt->sibling_ਗੰਢ, &ldev->filter_ਸਿਰ);

	ਵਾਪਸ filt->filter_ਅੰਕ;

err:
	ਮੁਕਤ(filt);
	ਵਾਪਸ ret;
}

ਪੂਰਨ log_remove_filter(ਅਚਲ ਅੱਖਰ *drv_name, ਪੂਰਨ filter_ਅੰਕ)
{
	ਬਣਤਰ log_filter *filt;
	ਬਣਤਰ log_device *ldev;

	ldev = log_device_ਲੱਭੋ_by_name(drv_name);
	ਜੇ (!ldev)
		ਵਾਪਸ -ENOENT;

	list_for_each_ਇੰਦਰਾਜ(filt, &ldev->filter_ਸਿਰ, sibling_ਗੰਢ) {
		ਜੇ (filt->filter_ਅੰਕ == filter_ਅੰਕ) {
			list_del(&filt->sibling_ਗੰਢ);
			ਮੁਕਤ(filt);

			ਵਾਪਸ 0;
		}
	}

	ਵਾਪਸ -ENOENT;
}

/**
 * log_find_device_by_drv() - Find a device by its driver
 *
 * @drv: Log driver
 * Return: Device associated with that driver, or NULL if not found
 */
ਸਥਿਰ ਬਣਤਰ log_device *log_ਲੱਭੋ_device_by_drv(ਬਣਤਰ log_driver *drv)
{
	ਬਣਤਰ log_device *ldev;

	list_for_each_ਇੰਦਰਾਜ(ldev, &gd->log_ਸਿਰ, sibling_ਗੰਢ) {
		ਜੇ (ldev->drv == drv)
			ਵਾਪਸ ldev;
	}
	/*
	 * It is quite hard to pass an invalid driver since passing an unknown
	 * LOG_GET_DRIVER(xxx) would normally produce a compilation error. But
	 * it is possible to pass NULL, for example, so this
	 */

	ਵਾਪਸ ਨਿਰਾ;
}

ਪੂਰਨ log_device_ਸੈੱਟ_enable(ਬਣਤਰ log_driver *drv, bool enable)
{
	ਬਣਤਰ log_device *ldev;

	ldev = log_ਲੱਭੋ_device_by_drv(drv);
	ਜੇ (!ldev)
		ਵਾਪਸ -ENOENT;
	ਜੇ (enable)
		ldev->flags |= LOGDF_ENABLE;
	ਨਹੀਂਤਾਂ
		ldev->flags &= ~LOGDF_ENABLE;

	ਵਾਪਸ 0;
}

ਖਾਲੀ log_fixup_for_gd_move(ਬਣਤਰ global_ਅੰਕੜੇ *new_gd)
{
	new_gd->log_ਸਿਰ.prev->ਅਗਲਾ = &new_gd->log_ਸਿਰ;
}

ਪੂਰਨ log_init(ਖਾਲੀ)
{
	ਬਣਤਰ log_driver *drv = ll_ਇੰਦਰਾਜ_ਸ਼ੁਰੂ(ਬਣਤਰ log_driver, log_driver);
	ਅਚਲ ਪੂਰਨ ਗਿਣਤੀ_2 = ll_ਇੰਦਰਾਜ_ਗਿਣਤੀ(ਬਣਤਰ log_driver, log_driver);
	ਬਣਤਰ log_driver *ਸਿਰਾ_2 = drv + ਗਿਣਤੀ_2;

	/*
	 * We cannot add runtime data to the driver since it is likely stored
	 * in rodata. Instead, set up a 'device' corresponding to each driver.
	 * We only support having a single device for each driver.
	 */
	INIT_LIST_ਸਿਰ((ਬਣਤਰ list_ਸਿਰ *)&gd->log_ਸਿਰ);
	ਜਦੋਂ (drv < ਸਿਰਾ_2) {
		ਬਣਤਰ log_device *ldev;

		ldev = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(*ldev));
		ਜੇ (!ldev) {
			debug("%s: Cannot allocate memory\n", __func__);
			ਵਾਪਸ -ENOMEM;
		}
		INIT_LIST_ਸਿਰ(&ldev->filter_ਸਿਰ);
		ldev->drv = drv;
		ldev->flags = drv->flags;
		list_ਜੋੜੋ_ਪੂਛ(&ldev->sibling_ਗੰਢ,
			      (ਬਣਤਰ list_ਸਿਰ *)&gd->log_ਸਿਰ);
		drv++;
	}
	gd->flags |= GD_FLG_LOG_READY;
	ਜੇ (!gd->default_log_level)
		gd->default_log_level = CONFIG_LOG_DEFAULT_LEVEL;
	gd->log_fmt = log_get_default_format();
	gd->logc_prev = LOGC_NONE;
	gd->logl_prev = LOGL_INFO;

	ਵਾਪਸ 0;
}
