// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ಅನ್ತರ್ಭಾವ <display_options.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <dm/uclass.h>

DECLARE_GLOBAL_DATA_PTR;

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ log_cat_name[] = {
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

_Static_assert(ARRAY_SIZE(log_cat_name) == LOGC_ಎಣಿಕೆ - LOGC_NONE,
	       "log_cat_name size");

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ log_level_name[] = {
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

_Static_assert(ARRAY_SIZE(log_level_name) == LOGL_ಎಣಿಕೆ, "log_level_name size");

/* All error responses MUST begin with '<' */
ಧ್ರುವ ವರ್ಣ *log_get_cat_name(ಗಣನಾ log_category_t cat)
{
	ಧ್ರುವ ವರ್ಣ *name;

	ಯದಿ (cat < 0 || cat >= LOGC_ಎಣಿಕೆ)
		ಪ್ರತ್ಯಾಗಮ "<invalid>";
	ಯದಿ (cat >= LOGC_NONE)
		ಪ್ರತ್ಯಾಗಮ log_cat_name[cat - LOGC_NONE];

#ಯದಿ CONFIG_IS_ENABLED(DM)
	name = uclass_get_name((ಗಣನಾ uclass_id)cat);
#ಅನ್ಯಥಾ
	name = ರಿಕ್ತ;
#endif

	ಪ್ರತ್ಯಾಗಮ name ? name : "<missing>";
}

ಗಣನಾ log_category_t log_get_cat_by_name(ಧ್ರುವ ವರ್ಣ *name)
{
	ಗಣನಾ uclass_id id;
	ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = LOGC_NONE; i < LOGC_ಎಣಿಕೆ; i++)
		ಯದಿ (!strcmp(name, log_cat_name[i - LOGC_NONE]))
			ಪ್ರತ್ಯಾಗಮ i;
	id = uclass_get_by_name(name);
	ಯದಿ (id != UCLASS_INVALID)
		ಪ್ರತ್ಯಾಗಮ (ಗಣನಾ log_category_t)id;

	ಪ್ರತ್ಯಾಗಮ LOGC_NONE;
}

ಧ್ರುವ ವರ್ಣ *log_get_level_name(ಗಣನಾ log_level_t level)
{
	ಯದಿ (level >= LOGL_ಎಣಿಕೆ)
		ಪ್ರತ್ಯಾಗಮ "INVALID";
	ಪ್ರತ್ಯಾಗಮ log_level_name[level];
}

ಗಣನಾ log_level_t log_get_level_by_name(ಧ್ರುವ ವರ್ಣ *name)
{
	ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = 0; i < LOGL_ಎಣಿಕೆ; i++) {
		ಯದಿ (!strcasecmp(log_level_name[i], name))
			ಪ್ರತ್ಯಾಗಮ i;
	}

	ಪ್ರತ್ಯಾಗಮ LOGL_NONE;
}

ಸಂರಚನಾ log_device *log_device_find_by_name(ಧ್ರುವ ವರ್ಣ *drv_name)
{
	ಸಂರಚನಾ log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_ಗಂಟು) {
		ಯದಿ (!strcmp(drv_name, ldev->drv->name))
			ಪ್ರತ್ಯಾಗಮ ldev;
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

bool log_has_cat(ಗಣನಾ log_category_t cat_list[], ಗಣನಾ log_category_t cat)
{
	ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = 0; i < LOGF_MAX_CATEGORIES && cat_list[i] != LOGC_END; i++) {
		ಯದಿ (cat_list[i] == cat)
			ಪ್ರತ್ಯಾಗಮ true;
	}

	ಪ್ರತ್ಯಾಗಮ false;
}

/**
 * log_has_member() - check if a string is in a comma separated list
 *
 * @list:	Comma separated list of strings
 * @member:	String to find
 *
 * Return: ``true`` if @member is in @list, else ``false``
 */
ಸ್ಥಿರ bool log_has_member(ಧ್ರುವ ವರ್ಣ *list, ಧ್ರುವ ವರ್ಣ *member)
{
	ಪೂರ್ಣಾಙ್ಕ member_len = strlen(member);
	ಧ್ರುವ ವರ್ಣ *s, *p;
	ಪೂರ್ಣಾಙ್ಕ substr_len;

	ಪ್ರತಿ (s = list; *s; s = p + (*p != '\0')) {
		p = strchrnul(s, ',');
		substr_len = p - s;
		ಯದಿ (member_len >= substr_len &&
		    !strncmp(member + member_len - substr_len, s, substr_len))
			ಪ್ರತ್ಯಾಗಮ true;
	}

	ಪ್ರತ್ಯಾಗಮ false;
}

/**
 * log_passes_filters() - check if a log record passes the filters for a device
 *
 * @ldev: Log device to check
 * @rec: Log record to check
 * Return: true if @rec is not blocked by the filters in @ldev, false if it is
 */
ಸ್ಥಿರ bool log_passes_filters(ಸಂರಚನಾ log_device *ldev, ಸಂರಚನಾ log_rec *rec)
{
	ಸಂರಚನಾ log_filter *filt;

	ಯದಿ (rec->flags & LOGRECF_FORCE_DEBUG)
		ಪ್ರತ್ಯಾಗಮ true;

	/* If there are no filters, filter on the default log level */
	ಯದಿ (list_empty(&ldev->filter_head)) {
		ಯದಿ (rec->level > gd->default_log_level)
			ಪ್ರತ್ಯಾಗಮ false;
		ಪ್ರತ್ಯಾಗಮ true;
	}

	list_for_each_entry(filt, &ldev->filter_head, sibling_ಗಂಟು) {
		ಯದಿ (filt->flags & LOGFF_LEVEL_MIN) {
			ಯದಿ (rec->level < filt->level)
				ಅನುವರ್ತ;
		} ಅನ್ಯಥಾ ಯದಿ (rec->level > filt->level) {
			ಅನುವರ್ತ;
		}

		ಯದಿ ((filt->flags & LOGFF_HAS_CAT) &&
		    !log_has_cat(filt->cat_list, rec->cat))
			ಅನುವರ್ತ;

		ಯದಿ (filt->file_list &&
		    !log_has_member(filt->file_list, rec->file))
			ಅನುವರ್ತ;

		ಯದಿ (filt->func_list &&
		    !log_has_member(filt->func_list, rec->func))
			ಅನುವರ್ತ;

		ಯದಿ (filt->flags & LOGFF_DENY)
			ಪ್ರತ್ಯಾಗಮ false;
		ಅನ್ಯಥಾ
			ಪ್ರತ್ಯಾಗಮ true;
	}

	ಪ್ರತ್ಯಾಗಮ false;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ log_dispatch(ಸಂರಚನಾ log_rec *rec, ಧ್ರುವ ವರ್ಣ *fmt, va_list args)
{
	ಸಂರಚನಾ log_device *ldev;
	ವರ್ಣ buf[CONFIG_SYS_CBSIZE];

	/*
	 * When a log driver writes messages (e.g. via the network stack) this
	 * may result in further generated messages. We cannot process them here
	 * as this might result in infinite recursion.
	 */
	ಯದಿ (gd->processing_ಸಂದೇಶ)
		ಪ್ರತ್ಯಾಗಮ 1;

	/* Emit message */
	gd->processing_ಸಂದೇಶ = true;
	list_for_each_entry(ldev, &gd->log_head, sibling_ಗಂಟು) {
		ಯದಿ ((ldev->flags & LOGDF_ENABLE) &&
		    log_passes_filters(ldev, rec)) {
			ಯದಿ (!rec->ಸಂದೇಶ) {
				ಪೂರ್ಣಾಙ್ಕ len;

				len = vsnprintf(buf, ಪರಿಮಾಣ(buf), fmt, args);
				rec->ಸಂದೇಶ = buf;
				gd->log_cont = len && buf[len - 1] != '\n';
			}
			ldev->drv->emit(ldev, rec);
		}
	}
	gd->processing_ಸಂದೇಶ = false;
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ _log(ಗಣನಾ log_category_t cat, ಗಣನಾ log_level_t level, ಧ್ರುವ ವರ್ಣ *file,
	 ಪೂರ್ಣಾಙ್ಕ ಸಾಲು, ಧ್ರುವ ವರ್ಣ *func, ಧ್ರುವ ವರ್ಣ *fmt, ...)
{
	ಸಂರಚನಾ log_rec rec;
	va_list args;

	ಯದಿ (!gd)
		ಪ್ರತ್ಯಾಗಮ -ENOSYS;

	/* Check for message continuation */
	ಯದಿ (cat == LOGC_CONT)
		cat = gd->logc_prev;
	ಯದಿ (level == LOGL_CONT)
		level = gd->logl_prev;

	rec.cat = cat;
	rec.level = level & LOGL_LEVEL_MASK;
	rec.flags = 0;
	ಯದಿ (level & LOGL_FORCE_DEBUG)
		rec.flags |= LOGRECF_FORCE_DEBUG;
	ಯದಿ (gd->log_cont)
		rec.flags |= LOGRECF_CONT;
	rec.file = file;
	rec.ಸಾಲು = ಸಾಲು;
	rec.func = func;
	rec.ಸಂದೇಶ = ರಿಕ್ತ;

	ಯದಿ (!(gd->flags & GD_FLG_LOG_READY)) {
		gd->log_drop_ಎಣಿಕೆ++;

		/* display dropped traces with console puts and DEBUG_UART */
		ಯದಿ (rec.level <= CONFIG_LOG_DEFAULT_LEVEL ||
		    rec.flags & LOGRECF_FORCE_DEBUG) {
			ವರ್ಣ buf[CONFIG_SYS_CBSIZE];

			va_start(args, fmt);
			vsnprintf(buf, ಪರಿಮಾಣ(buf), fmt, args);
			puts(buf);
			va_end(args);
		}

		ಪ್ರತ್ಯಾಗಮ -ENOSYS;
	}
	va_start(args, fmt);
	ಯದಿ (!log_dispatch(&rec, fmt, args)) {
		gd->logc_prev = cat;
		gd->logl_prev = level;
	}
	va_end(args);

	ಪ್ರತ್ಯಾಗಮ 0;
}

#define MAX_ಸಾಲು_LENGTH_BYTES		64
#define DEFAULT_ಸಾಲು_LENGTH_BYTES	16

ಪೂರ್ಣಾಙ್ಕ _log_buffer(ಗಣನಾ log_category_t cat, ಗಣನಾ log_level_t level,
		ಧ್ರುವ ವರ್ಣ *file, ಪೂರ್ಣಾಙ್ಕ ಸಾಲು, ಧ್ರುವ ವರ್ಣ *func, ulong addr,
		ಧ್ರುವ ಶೂನ್ಯ *data, uint width, uint ಎಣಿಕೆ, uint linelen)
{
	ಯದಿ (linelen * width > MAX_ಸಾಲು_LENGTH_BYTES)
		linelen = MAX_ಸಾಲು_LENGTH_BYTES / width;
	ಯದಿ (linelen < 1)
		linelen = DEFAULT_ಸಾಲು_LENGTH_BYTES / width;

	ಯಾವತ್ (ಎಣಿಕೆ) {
		uint thislinelen;
		ವರ್ಣ buf[HEXDUMP_MAX_BUF_LENGTH(width * linelen)];

		thislinelen = hexdump_ಸಾಲು(addr, data, width, ಎಣಿಕೆ, linelen,
					   buf, ಪರಿಮಾಣ(buf));
		ಪ್ರತಿಜ್ಞಾ(thislinelen >= 0);
		_log(cat, level, file, ಸಾಲು, func, "%s\n", buf);

		/* update references */
		data += thislinelen * width;
		addr += thislinelen * width;
		ಎಣಿಕೆ -= thislinelen;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ log_add_filter_flags(ಧ್ರುವ ವರ್ಣ *drv_name, ಗಣನಾ log_category_t cat_list[],
			 ಗಣನಾ log_level_t level, ಧ್ರುವ ವರ್ಣ *file_list,
			 ಧ್ರುವ ವರ್ಣ *func_list, ಪೂರ್ಣಾಙ್ಕ flags)
{
	ಸಂರಚನಾ log_filter *filt;
	ಸಂರಚನಾ log_device *ldev;
	ಪೂರ್ಣಾಙ್ಕ ret;
	ಪೂರ್ಣಾಙ್ಕ i;

	ldev = log_device_find_by_name(drv_name);
	ಯದಿ (!ldev)
		ಪ್ರತ್ಯಾಗಮ -ENOENT;
	filt = calloc(1, ಪರಿಮಾಣ(*filt));
	ಯದಿ (!filt)
		ಪ್ರತ್ಯಾಗಮ -ENOMEM;

	filt->flags = flags;
	ಯದಿ (cat_list) {
		filt->flags |= LOGFF_HAS_CAT;
		ಪ್ರತಿ (i = 0; ; i++) {
			ಯದಿ (i == ARRAY_SIZE(filt->cat_list)) {
				ret = -ENOSPC;
				ಗಚ್ಛ err;
			}
			filt->cat_list[i] = cat_list[i];
			ಯದಿ (cat_list[i] == LOGC_END)
				ಭಙ್ಗ;
		}
	}
	filt->level = level;
	ಯದಿ (file_list) {
		filt->file_list = strdup(file_list);
		ಯದಿ (!filt->file_list) {
			ret = -ENOMEM;
			ಗಚ್ಛ err;
		}
	}
	ಯದಿ (func_list) {
		filt->func_list = strdup(func_list);
		ಯದಿ (!filt->func_list) {
			ret = -ENOMEM;
			ಗಚ್ಛ err;
		}
	}
	filt->filter_ಅಂಕೆ = ldev->next_filter_ಅಂಕೆ++;
	/* Add deny filters to the beginning of the list */
	ಯದಿ (flags & LOGFF_DENY)
		list_add(&filt->sibling_ಗಂಟು, &ldev->filter_head);
	ಅನ್ಯಥಾ
		list_add_tail(&filt->sibling_ಗಂಟು, &ldev->filter_head);

	ಪ್ರತ್ಯಾಗಮ filt->filter_ಅಂಕೆ;

err:
	ಮುಕ್ತ(filt);
	ಪ್ರತ್ಯಾಗಮ ret;
}

ಪೂರ್ಣಾಙ್ಕ log_remove_filter(ಧ್ರುವ ವರ್ಣ *drv_name, ಪೂರ್ಣಾಙ್ಕ filter_ಅಂಕೆ)
{
	ಸಂರಚನಾ log_filter *filt;
	ಸಂರಚನಾ log_device *ldev;

	ldev = log_device_find_by_name(drv_name);
	ಯದಿ (!ldev)
		ಪ್ರತ್ಯಾಗಮ -ENOENT;

	list_for_each_entry(filt, &ldev->filter_head, sibling_ಗಂಟು) {
		ಯದಿ (filt->filter_ಅಂಕೆ == filter_ಅಂಕೆ) {
			list_del(&filt->sibling_ಗಂಟು);
			ಮುಕ್ತ(filt);

			ಪ್ರತ್ಯಾಗಮ 0;
		}
	}

	ಪ್ರತ್ಯಾಗಮ -ENOENT;
}

/**
 * log_find_device_by_drv() - Find a device by its driver
 *
 * @drv: Log driver
 * Return: Device associated with that driver, or NULL if not found
 */
ಸ್ಥಿರ ಸಂರಚನಾ log_device *log_find_device_by_drv(ಸಂರಚನಾ log_driver *drv)
{
	ಸಂರಚನಾ log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_ಗಂಟು) {
		ಯದಿ (ldev->drv == drv)
			ಪ್ರತ್ಯಾಗಮ ldev;
	}
	/*
	 * It is quite hard to pass an invalid driver since passing an unknown
	 * LOG_GET_DRIVER(xxx) would normally produce a compilation error. But
	 * it is possible to pass NULL, for example, so this
	 */

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಪೂರ್ಣಾಙ್ಕ log_device_set_enable(ಸಂರಚನಾ log_driver *drv, bool enable)
{
	ಸಂರಚನಾ log_device *ldev;

	ldev = log_find_device_by_drv(drv);
	ಯದಿ (!ldev)
		ಪ್ರತ್ಯಾಗಮ -ENOENT;
	ಯದಿ (enable)
		ldev->flags |= LOGDF_ENABLE;
	ಅನ್ಯಥಾ
		ldev->flags &= ~LOGDF_ENABLE;

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಶೂನ್ಯ log_fixup_for_gd_move(ಸಂರಚನಾ global_data *new_gd)
{
	new_gd->log_head.prev->next = &new_gd->log_head;
}

ಪೂರ್ಣಾಙ್ಕ log_init(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ log_driver *drv = ll_entry_start(ಸಂರಚನಾ log_driver, log_driver);
	ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = ll_entry_ಎಣಿಕೆ(ಸಂರಚನಾ log_driver, log_driver);
	ಸಂರಚನಾ log_driver *end = drv + ಎಣಿಕೆ;

	/*
	 * We cannot add runtime data to the driver since it is likely stored
	 * in rodata. Instead, set up a 'device' corresponding to each driver.
	 * We only support having a single device for each driver.
	 */
	INIT_LIST_HEAD((ಸಂರಚನಾ list_head *)&gd->log_head);
	ಯಾವತ್ (drv < end) {
		ಸಂರಚನಾ log_device *ldev;

		ldev = calloc(1, ಪರಿಮಾಣ(*ldev));
		ಯದಿ (!ldev) {
			debug("%s: Cannot allocate memory\n", __func__);
			ಪ್ರತ್ಯಾಗಮ -ENOMEM;
		}
		INIT_LIST_HEAD(&ldev->filter_head);
		ldev->drv = drv;
		ldev->flags = drv->flags;
		list_add_tail(&ldev->sibling_ಗಂಟು,
			      (ಸಂರಚನಾ list_head *)&gd->log_head);
		drv++;
	}
	gd->flags |= GD_FLG_LOG_READY;
	ಯದಿ (!gd->default_log_level)
		gd->default_log_level = CONFIG_LOG_DEFAULT_LEVEL;
	gd->log_fmt = log_get_default_format();
	gd->logc_prev = LOGC_NONE;
	gd->logl_prev = LOGL_INFO;

	ಪ್ರತ್ಯಾಗಮ 0;
}
