// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#అన్తర్భావ <display_options.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <malloc.h>
#అన్తర్భావ <asm/global_data.h>
#అన్తర్భావ <dm/uclass.h>

DECLARE_GLOBAL_దత్తాంశం_PTR;

స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి log_cat_name[] = {
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

_Static_assert(ARRAY_కొలత(log_cat_name) == LOGC_లెక్క - LOGC_NONE,
	       "log_cat_name size");

స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి log_level_name[] = {
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

_Static_assert(ARRAY_కొలత(log_level_name) == LOGL_లెక్క, "log_level_name size");

/* All error responses MUST begin with '<' */
స్థిరరాశి అక్షరం *log_get_cat_name(గణన log_category_t cat)
{
	స్థిరరాశి అక్షరం *name;

	ఒకవేళ (cat < 0 || cat >= LOGC_లెక్క)
		తిరిగి "<invalid>";
	ఒకవేళ (cat >= LOGC_NONE)
		తిరిగి log_cat_name[cat - LOGC_NONE];

#ఒకవేళ CONFIG_IS_ENABLED(DM)
	name = uclass_get_name((గణన uclass_id)cat);
#లేకపోతే
	name = ఖాళీ;
#అయితే_ముగింపు

	తిరిగి name ? name : "<missing>";
}

గణన log_category_t log_get_cat_by_name(స్థిరరాశి అక్షరం *name)
{
	గణన uclass_id id;
	పూర్ణాంకం i;

	కొరకు (i = LOGC_NONE; i < LOGC_లెక్క; i++)
		ఒకవేళ (!strcmp(name, log_cat_name[i - LOGC_NONE]))
			తిరిగి i;
	id = uclass_get_by_name(name);
	ఒకవేళ (id != UCLASS_INVALID)
		తిరిగి (గణన log_category_t)id;

	తిరిగి LOGC_NONE;
}

స్థిరరాశి అక్షరం *log_get_level_name(గణన log_level_t level)
{
	ఒకవేళ (level >= LOGL_లెక్క)
		తిరిగి "INVALID";
	తిరిగి log_level_name[level];
}

గణన log_level_t log_get_level_by_name(స్థిరరాశి అక్షరం *name)
{
	పూర్ణాంకం i;

	కొరకు (i = 0; i < LOGL_లెక్క; i++) {
		ఒకవేళ (!strcasecmp(log_level_name[i], name))
			తిరిగి i;
	}

	తిరిగి LOGL_NONE;
}

నిర్మాణం log_device *log_device_కనుగొను_by_name(స్థిరరాశి అక్షరం *drv_name)
{
	నిర్మాణం log_device *ldev;

	list_for_each_నమోదు(ldev, &gd->log_తల, sibling_కణుపు) {
		ఒకవేళ (!strcmp(drv_name, ldev->drv->name))
			తిరిగి ldev;
	}

	తిరిగి ఖాళీ;
}

bool log_has_cat(గణన log_category_t cat_list[], గణన log_category_t cat)
{
	పూర్ణాంకం i;

	కొరకు (i = 0; i < LOGF_MAX_CATEGORIES && cat_list[i] != LOGC_చివర; i++) {
		ఒకవేళ (cat_list[i] == cat)
			తిరిగి true;
	}

	తిరిగి false;
}

/**
 * log_has_member() - check if a string is in a comma separated list
 *
 * @list:	Comma separated list of strings
 * @member:	String to find
 *
 * Return: ``true`` if @member is in @list, else ``false``
 */
స్థిర bool log_has_member(స్థిరరాశి అక్షరం *list, స్థిరరాశి అక్షరం *member)
{
	పూర్ణాంకం member_len = అక్షరశ్రేణి_పొడవు(member);
	స్థిరరాశి అక్షరం *s, *p;
	పూర్ణాంకం substr_len;

	కొరకు (s = list; *s; s = p + (*p != '\0')) {
		p = strchrnul(s, ',');
		substr_len = p - s;
		ఒకవేళ (member_len >= substr_len &&
		    !అక్షరశ్రేణి_పోలిక(member + member_len - substr_len, s, substr_len))
			తిరిగి true;
	}

	తిరిగి false;
}

/**
 * log_passes_filters() - check if a log record passes the filters for a device
 *
 * @ldev: Log device to check
 * @rec: Log record to check
 * Return: true if @rec is not blocked by the filters in @ldev, false if it is
 */
స్థిర bool log_passes_filters(నిర్మాణం log_device *ldev, నిర్మాణం log_rec *rec)
{
	నిర్మాణం log_filter *filt;

	ఒకవేళ (rec->flags & LOGRECF_FORCE_DEBUG)
		తిరిగి true;

	/* If there are no filters, filter on the default log level */
	ఒకవేళ (list_ఖాళీలు(&ldev->filter_తల)) {
		ఒకవేళ (rec->level > gd->default_log_level)
			తిరిగి false;
		తిరిగి true;
	}

	list_for_each_నమోదు(filt, &ldev->filter_తల, sibling_కణుపు) {
		ఒకవేళ (filt->flags & LOGFF_LEVEL_MIN) {
			ఒకవేళ (rec->level < filt->level)
				కొనసాగు;
		} లేకపోతే ఒకవేళ (rec->level > filt->level) {
			కొనసాగు;
		}

		ఒకవేళ ((filt->flags & LOGFF_HAS_CAT) &&
		    !log_has_cat(filt->cat_list, rec->cat))
			కొనసాగు;

		ఒకవేళ (filt->file_list &&
		    !log_has_member(filt->file_list, rec->file))
			కొనసాగు;

		ఒకవేళ (filt->func_list &&
		    !log_has_member(filt->func_list, rec->func))
			కొనసాగు;

		ఒకవేళ (filt->flags & LOGFF_DENY)
			తిరిగి false;
		లేకపోతే
			తిరిగి true;
	}

	తిరిగి false;
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
స్థిర పూర్ణాంకం log_dispatch(నిర్మాణం log_rec *rec, స్థిరరాశి అక్షరం *fmt, va_list వాదనలు)
{
	నిర్మాణం log_device *ldev;
	అక్షరం buf[CONFIG_SYS_CBSIZE];

	/*
	 * When a log driver writes messages (e.g. via the network stack) this
	 * may result in further generated messages. We cannot process them here
	 * as this might result in infinite recursion.
	 */
	ఒకవేళ (gd->processing_సందేశం)
		తిరిగి 1;

	/* Emit message */
	gd->processing_సందేశం = true;
	list_for_each_నమోదు(ldev, &gd->log_తల, sibling_కణుపు) {
		ఒకవేళ ((ldev->flags & LOGDF_ENABLE) &&
		    log_passes_filters(ldev, rec)) {
			ఒకవేళ (!rec->సందేశం) {
				పూర్ణాంకం len;

				len = vsnprintf(buf, పరిమాణం(buf), fmt, వాదనలు);
				rec->సందేశం = buf;
				gd->log_cont = len && buf[len - 1] != '\n';
			}
			ldev->drv->emit(ldev, rec);
		}
	}
	gd->processing_సందేశం = false;
	తిరిగి 0;
}

పూర్ణాంకం _log(గణన log_category_t cat, గణన log_level_t level, స్థిరరాశి అక్షరం *file,
	 పూర్ణాంకం పంక్తి, స్థిరరాశి అక్షరం *func, స్థిరరాశి అక్షరం *fmt, ...)
{
	నిర్మాణం log_rec rec;
	va_list వాదనలు;

	ఒకవేళ (!gd)
		తిరిగి -ENOSYS;

	/* Check for message continuation */
	ఒకవేళ (cat == LOGC_CONT)
		cat = gd->logc_prev;
	ఒకవేళ (level == LOGL_CONT)
		level = gd->logl_prev;

	rec.cat = cat;
	rec.level = level & LOGL_LEVEL_ముసుగు;
	rec.flags = 0;
	ఒకవేళ (level & LOGL_FORCE_DEBUG)
		rec.flags |= LOGRECF_FORCE_DEBUG;
	ఒకవేళ (gd->log_cont)
		rec.flags |= LOGRECF_CONT;
	rec.file = file;
	rec.పంక్తి = పంక్తి;
	rec.func = func;
	rec.సందేశం = ఖాళీ;

	ఒకవేళ (!(gd->flags & GD_FLG_LOG_READY)) {
		gd->log_drop_లెక్క++;

		/* display dropped traces with console puts and DEBUG_UART */
		ఒకవేళ (rec.level <= CONFIG_LOG_DEFAULT_LEVEL ||
		    rec.flags & LOGRECF_FORCE_DEBUG) {
			అక్షరం buf[CONFIG_SYS_CBSIZE];

			va_ప్రారంభం(వాదనలు, fmt);
			vsnprintf(buf, పరిమాణం(buf), fmt, వాదనలు);
			puts(buf);
			va_చివర(వాదనలు);
		}

		తిరిగి -ENOSYS;
	}
	va_ప్రారంభం(వాదనలు, fmt);
	ఒకవేళ (!log_dispatch(&rec, fmt, వాదనలు)) {
		gd->logc_prev = cat;
		gd->logl_prev = level;
	}
	va_చివర(వాదనలు);

	తిరిగి 0;
}

#నిర్వచించు MAX_పంక్తి_నిడివి_బైట్లు		64
#నిర్వచించు DEFAULT_పంక్తి_నిడివి_బైట్లు	16

పూర్ణాంకం log_బఫరు(గణన log_category_t cat, గణన log_level_t level,
		స్థిరరాశి అక్షరం *file, పూర్ణాంకం పంక్తి, స్థిరరాశి అక్షరం *func, ulong addr,
		స్థిరరాశి శూన్య *దత్తాంశం, uint వెడల్పు, uint లెక్క, uint linelen)
{
	ఒకవేళ (linelen * వెడల్పు > MAX_పంక్తి_నిడివి_బైట్లు)
		linelen = MAX_పంక్తి_నిడివి_బైట్లు / వెడల్పు;
	ఒకవేళ (linelen < 1)
		linelen = DEFAULT_పంక్తి_నిడివి_బైట్లు / వెడల్పు;

	అయితే (లెక్క) {
		uint thislinelen;
		అక్షరం buf[HEXDUMP_MAX_BUF_నిడివి(వెడల్పు * linelen)];

		thislinelen = hexdump_పంక్తి(addr, దత్తాంశం, వెడల్పు, లెక్క, linelen,
					   buf, పరిమాణం(buf));
		ప్రతిజ్ఞా(thislinelen >= 0);
		_log(cat, level, file, పంక్తి, func, "%s\n", buf);

		/* update references */
		దత్తాంశం += thislinelen * వెడల్పు;
		addr += thislinelen * వెడల్పు;
		లెక్క -= thislinelen;
	}

	తిరిగి 0;
}

పూర్ణాంకం log_కూడు_filter_flags(స్థిరరాశి అక్షరం *drv_name, గణన log_category_t cat_list[],
			 గణన log_level_t level, స్థిరరాశి అక్షరం *file_list,
			 స్థిరరాశి అక్షరం *func_list, పూర్ణాంకం flags)
{
	నిర్మాణం log_filter *filt;
	నిర్మాణం log_device *ldev;
	పూర్ణాంకం ret;
	పూర్ణాంకం i;

	ldev = log_device_కనుగొను_by_name(drv_name);
	ఒకవేళ (!ldev)
		తిరిగి -ENOENT;
	filt = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(*filt));
	ఒకవేళ (!filt)
		తిరిగి -ENOMEM;

	filt->flags = flags;
	ఒకవేళ (cat_list) {
		filt->flags |= LOGFF_HAS_CAT;
		కొరకు (i = 0; ; i++) {
			ఒకవేళ (i == ARRAY_కొలత(filt->cat_list)) {
				ret = -ENOSPC;
				వెళ్ళు err;
			}
			filt->cat_list[i] = cat_list[i];
			ఒకవేళ (cat_list[i] == LOGC_చివర)
				ఆపు;
		}
	}
	filt->level = level;
	ఒకవేళ (file_list) {
		filt->file_list = strdup(file_list);
		ఒకవేళ (!filt->file_list) {
			ret = -ENOMEM;
			వెళ్ళు err;
		}
	}
	ఒకవేళ (func_list) {
		filt->func_list = strdup(func_list);
		ఒకవేళ (!filt->func_list) {
			ret = -ENOMEM;
			వెళ్ళు err;
		}
	}
	filt->filter_అంకె = ldev->తదుపరి_filter_అంకె++;
	/* Add deny filters to the beginning of the list */
	ఒకవేళ (flags & LOGFF_DENY)
		list_కూడు(&filt->sibling_కణుపు, &ldev->filter_తల);
	లేకపోతే
		list_కూడు_తోక(&filt->sibling_కణుపు, &ldev->filter_తల);

	తిరిగి filt->filter_అంకె;

err:
	విడుదల(filt);
	తిరిగి ret;
}

పూర్ణాంకం log_remove_filter(స్థిరరాశి అక్షరం *drv_name, పూర్ణాంకం filter_అంకె)
{
	నిర్మాణం log_filter *filt;
	నిర్మాణం log_device *ldev;

	ldev = log_device_కనుగొను_by_name(drv_name);
	ఒకవేళ (!ldev)
		తిరిగి -ENOENT;

	list_for_each_నమోదు(filt, &ldev->filter_తల, sibling_కణుపు) {
		ఒకవేళ (filt->filter_అంకె == filter_అంకె) {
			list_del(&filt->sibling_కణుపు);
			విడుదల(filt);

			తిరిగి 0;
		}
	}

	తిరిగి -ENOENT;
}

/**
 * log_find_device_by_drv() - Find a device by its driver
 *
 * @drv: Log driver
 * Return: Device associated with that driver, or NULL if not found
 */
స్థిర నిర్మాణం log_device *log_కనుగొను_device_by_drv(నిర్మాణం log_driver *drv)
{
	నిర్మాణం log_device *ldev;

	list_for_each_నమోదు(ldev, &gd->log_తల, sibling_కణుపు) {
		ఒకవేళ (ldev->drv == drv)
			తిరిగి ldev;
	}
	/*
	 * It is quite hard to pass an invalid driver since passing an unknown
	 * LOG_GET_DRIVER(xxx) would normally produce a compilation error. But
	 * it is possible to pass NULL, for example, so this
	 */

	తిరిగి ఖాళీ;
}

పూర్ణాంకం log_device_సమితి_enable(నిర్మాణం log_driver *drv, bool enable)
{
	నిర్మాణం log_device *ldev;

	ldev = log_కనుగొను_device_by_drv(drv);
	ఒకవేళ (!ldev)
		తిరిగి -ENOENT;
	ఒకవేళ (enable)
		ldev->flags |= LOGDF_ENABLE;
	లేకపోతే
		ldev->flags &= ~LOGDF_ENABLE;

	తిరిగి 0;
}

శూన్య log_fixup_for_gd_move(నిర్మాణం global_దత్తాంశం *new_gd)
{
	new_gd->log_తల.prev->తదుపరి = &new_gd->log_తల;
}

పూర్ణాంకం log_init(శూన్య)
{
	నిర్మాణం log_driver *drv = ll_నమోదు_ప్రారంభం(నిర్మాణం log_driver, log_driver);
	స్థిరరాశి పూర్ణాంకం లెక్క = ll_నమోదు_లెక్క(నిర్మాణం log_driver, log_driver);
	నిర్మాణం log_driver *చివర_2 = drv + లెక్క;

	/*
	 * We cannot add runtime data to the driver since it is likely stored
	 * in rodata. Instead, set up a 'device' corresponding to each driver.
	 * We only support having a single device for each driver.
	 */
	INIT_LIST_తల((నిర్మాణం list_తల *)&gd->log_తల);
	అయితే (drv < చివర_2) {
		నిర్మాణం log_device *ldev;

		ldev = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(*ldev));
		ఒకవేళ (!ldev) {
			debug("%s: Cannot allocate memory\n", __func__);
			తిరిగి -ENOMEM;
		}
		INIT_LIST_తల(&ldev->filter_తల);
		ldev->drv = drv;
		ldev->flags = drv->flags;
		list_కూడు_తోక(&ldev->sibling_కణుపు,
			      (నిర్మాణం list_తల *)&gd->log_తల);
		drv++;
	}
	gd->flags |= GD_FLG_LOG_READY;
	ఒకవేళ (!gd->default_log_level)
		gd->default_log_level = CONFIG_LOG_DEFAULT_LEVEL;
	gd->log_fmt = log_get_default_format();
	gd->logc_prev = LOGC_NONE;
	gd->logl_prev = LOGL_INFO;

	తిరిగి 0;
}
