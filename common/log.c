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

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ log_cat_name[] = {
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

_Static_assert(ARRAY_ಗಾತ್ರ(log_cat_name) == LOGC_ಎಣಿಕೆ - LOGC_NONE,
	       "log_cat_name size");

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ log_level_name[] = {
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

_Static_assert(ARRAY_ಗಾತ್ರ(log_level_name) == LOGL_ಎಣಿಕೆ, "log_level_name size");

/* All error responses MUST begin with '<' */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *log_get_cat_name(ಎಣಿಕೆ log_category_t cat)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;

	ಆದರೆ (cat < 0 || cat >= LOGC_ಎಣಿಕೆ)
		ಹಿಂತಿರುಗಿ "<invalid>";
	ಆದರೆ (cat >= LOGC_NONE)
		ಹಿಂತಿರುಗಿ log_cat_name[cat - LOGC_NONE];

#ಆದರೆ CONFIG_IS_ENABLED(DM)
	name = uclass_get_name((ಎಣಿಕೆ uclass_id)cat);
#ಇಲ್ಲದಿದ್ದರೆ
	name = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ name ? name : "<missing>";
}

ಎಣಿಕೆ log_category_t log_get_cat_by_name(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಎಣಿಕೆ uclass_id id;
	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = LOGC_NONE; i < LOGC_ಎಣಿಕೆ; i++)
		ಆದರೆ (!strcmp(name, log_cat_name[i - LOGC_NONE]))
			ಹಿಂತಿರುಗಿ i;
	id = uclass_get_by_name(name);
	ಆದರೆ (id != UCLASS_INVALID)
		ಹಿಂತಿರುಗಿ (ಎಣಿಕೆ log_category_t)id;

	ಹಿಂತಿರುಗಿ LOGC_NONE;
}

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *log_get_level_name(ಎಣಿಕೆ log_level_t level)
{
	ಆದರೆ (level >= LOGL_ಎಣಿಕೆ)
		ಹಿಂತಿರುಗಿ "INVALID";
	ಹಿಂತಿರುಗಿ log_level_name[level];
}

ಎಣಿಕೆ log_level_t log_get_level_by_name(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = 0; i < LOGL_ಎಣಿಕೆ; i++) {
		ಆದರೆ (!strcasecmp(log_level_name[i], name))
			ಹಿಂತಿರುಗಿ i;
	}

	ಹಿಂತಿರುಗಿ LOGL_NONE;
}

ರಚನೆ log_device *log_device_ಕಂಡುಹಿಡಿ_by_name(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *drv_name)
{
	ರಚನೆ log_device *ldev;

	list_for_each_ನಮೂದು(ldev, &gd->log_ತಲೆ, sibling_ಗಂಟು) {
		ಆದರೆ (!strcmp(drv_name, ldev->drv->name))
			ಹಿಂತಿರುಗಿ ldev;
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

bool log_has_cat(ಎಣಿಕೆ log_category_t cat_list[], ಎಣಿಕೆ log_category_t cat)
{
	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = 0; i < LOGF_MAX_CATEGORIES && cat_list[i] != LOGC_ಕೊನೆ; i++) {
		ಆದರೆ (cat_list[i] == cat)
			ಹಿಂತಿರುಗಿ true;
	}

	ಹಿಂತಿರುಗಿ false;
}

/**
 * log_has_member() - check if a string is in a comma separated list
 *
 * @list:	Comma separated list of strings
 * @member:	String to find
 *
 * Return: ``true`` if @member is in @list, else ``false``
 */
ಸ್ಥಿರ bool log_has_member(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *list, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *member)
{
	ಪೂರ್ಣಾಂಕ member_len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(member);
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, *p;
	ಪೂರ್ಣಾಂಕ substr_len;

	ಗಾಗಿ (s = list; *s; s = p + (*p != '\0')) {
		p = strchrnul(s, ',');
		substr_len = p - s;
		ಆದರೆ (member_len >= substr_len &&
		    !ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(member + member_len - substr_len, s, substr_len))
			ಹಿಂತಿರುಗಿ true;
	}

	ಹಿಂತಿರುಗಿ false;
}

/**
 * log_passes_filters() - check if a log record passes the filters for a device
 *
 * @ldev: Log device to check
 * @rec: Log record to check
 * Return: true if @rec is not blocked by the filters in @ldev, false if it is
 */
ಸ್ಥಿರ bool log_passes_filters(ರಚನೆ log_device *ldev, ರಚನೆ log_rec *rec)
{
	ರಚನೆ log_filter *filt;

	ಆದರೆ (rec->flags & LOGRECF_FORCE_DEBUG)
		ಹಿಂತಿರುಗಿ true;

	/* If there are no filters, filter on the default log level */
	ಆದರೆ (list_ಖಾಲಿಗಳು(&ldev->filter_ತಲೆ)) {
		ಆದರೆ (rec->level > gd->default_log_level)
			ಹಿಂತಿರುಗಿ false;
		ಹಿಂತಿರುಗಿ true;
	}

	list_for_each_ನಮೂದು(filt, &ldev->filter_ತಲೆ, sibling_ಗಂಟು) {
		ಆದರೆ (filt->flags & LOGFF_LEVEL_MIN) {
			ಆದರೆ (rec->level < filt->level)
				ಮುಂದುವರಿ;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (rec->level > filt->level) {
			ಮುಂದುವರಿ;
		}

		ಆದರೆ ((filt->flags & LOGFF_HAS_CAT) &&
		    !log_has_cat(filt->cat_list, rec->cat))
			ಮುಂದುವರಿ;

		ಆದರೆ (filt->file_list &&
		    !log_has_member(filt->file_list, rec->file))
			ಮುಂದುವರಿ;

		ಆದರೆ (filt->func_list &&
		    !log_has_member(filt->func_list, rec->func))
			ಮುಂದುವರಿ;

		ಆದರೆ (filt->flags & LOGFF_DENY)
			ಹಿಂತಿರುಗಿ false;
		ಇಲ್ಲದಿದ್ದರೆ
			ಹಿಂತಿರುಗಿ true;
	}

	ಹಿಂತಿರುಗಿ false;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ log_dispatch(ರಚನೆ log_rec *rec, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, va_list ವಾದಗಳು)
{
	ರಚನೆ log_device *ldev;
	ಅಕ್ಷರ buf[CONFIG_SYS_CBSIZE];

	/*
	 * When a log driver writes messages (e.g. via the network stack) this
	 * may result in further generated messages. We cannot process them here
	 * as this might result in infinite recursion.
	 */
	ಆದರೆ (gd->processing_ಸಂದೇಶ)
		ಹಿಂತಿರುಗಿ 1;

	/* Emit message */
	gd->processing_ಸಂದೇಶ = true;
	list_for_each_ನಮೂದು(ldev, &gd->log_ತಲೆ, sibling_ಗಂಟು) {
		ಆದರೆ ((ldev->flags & LOGDF_ENABLE) &&
		    log_passes_filters(ldev, rec)) {
			ಆದರೆ (!rec->ಸಂದೇಶ) {
				ಪೂರ್ಣಾಂಕ len;

				len = vsnprintf(buf, ಗಾತ್ರ(buf), fmt, ವಾದಗಳು);
				rec->ಸಂದೇಶ = buf;
				gd->log_cont = len && buf[len - 1] != '\n';
			}
			ldev->drv->emit(ldev, rec);
		}
	}
	gd->processing_ಸಂದೇಶ = false;
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ _log(ಎಣಿಕೆ log_category_t cat, ಎಣಿಕೆ log_level_t level, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *file,
	 ಪೂರ್ಣಾಂಕ ಸಾಲು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *func, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
{
	ರಚನೆ log_rec rec;
	va_list ವಾದಗಳು;

	ಆದರೆ (!gd)
		ಹಿಂತಿರುಗಿ -ENOSYS;

	/* Check for message continuation */
	ಆದರೆ (cat == LOGC_CONT)
		cat = gd->logc_prev;
	ಆದರೆ (level == LOGL_CONT)
		level = gd->logl_prev;

	rec.cat = cat;
	rec.level = level & LOGL_LEVEL_ಮುಖವಾಡ;
	rec.flags = 0;
	ಆದರೆ (level & LOGL_FORCE_DEBUG)
		rec.flags |= LOGRECF_FORCE_DEBUG;
	ಆದರೆ (gd->log_cont)
		rec.flags |= LOGRECF_CONT;
	rec.file = file;
	rec.ಸಾಲು = ಸಾಲು;
	rec.func = func;
	rec.ಸಂದೇಶ = ಖಾಲಿ;

	ಆದರೆ (!(gd->flags & GD_FLG_LOG_READY)) {
		gd->log_drop_ಎಣಿಕೆ++;

		/* display dropped traces with console puts and DEBUG_UART */
		ಆದರೆ (rec.level <= CONFIG_LOG_DEFAULT_LEVEL ||
		    rec.flags & LOGRECF_FORCE_DEBUG) {
			ಅಕ್ಷರ buf[CONFIG_SYS_CBSIZE];

			va_ಆರಂಭ(ವಾದಗಳು, fmt);
			vsnprintf(buf, ಗಾತ್ರ(buf), fmt, ವಾದಗಳು);
			puts(buf);
			va_ಕೊನೆ(ವಾದಗಳು);
		}

		ಹಿಂತಿರುಗಿ -ENOSYS;
	}
	va_ಆರಂಭ(ವಾದಗಳು, fmt);
	ಆದರೆ (!log_dispatch(&rec, fmt, ವಾದಗಳು)) {
		gd->logc_prev = cat;
		gd->logl_prev = level;
	}
	va_ಕೊನೆ(ವಾದಗಳು);

	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿಸು MAX_ಸಾಲು_ಉದ್ದಳತೆ_ಬೈಟುಗಳು		64
#ವ್ಯಾಖ್ಯಾನಿಸು DEFAULT_ಸಾಲು_ಉದ್ದಳತೆ_ಬೈಟುಗಳು	16

ಪೂರ್ಣಾಂಕ log_ಬಫರ್(ಎಣಿಕೆ log_category_t cat, ಎಣಿಕೆ log_level_t level,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *file, ಪೂರ್ಣಾಂಕ ಸಾಲು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *func, ulong addr,
		ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ದತ್ತಾಂಶ, uint ಅಗಲ, uint ಎಣಿಕೆ_2, uint linelen)
{
	ಆದರೆ (linelen * ಅಗಲ > MAX_ಸಾಲು_ಉದ್ದಳತೆ_ಬೈಟುಗಳು)
		linelen = MAX_ಸಾಲು_ಉದ್ದಳತೆ_ಬೈಟುಗಳು / ಅಗಲ;
	ಆದರೆ (linelen < 1)
		linelen = DEFAULT_ಸಾಲು_ಉದ್ದಳತೆ_ಬೈಟುಗಳು / ಅಗಲ;

	ಆಗ (ಎಣಿಕೆ_2) {
		uint thislinelen;
		ಅಕ್ಷರ buf[HEXDUMP_MAX_BUF_ಉದ್ದಳತೆ(ಅಗಲ * linelen)];

		thislinelen = hexdump_ಸಾಲು(addr, ದತ್ತಾಂಶ, ಅಗಲ, ಎಣಿಕೆ_2, linelen,
					   buf, ಗಾತ್ರ(buf));
		ಪ್ರತಿಜ್ಞಾ(thislinelen >= 0);
		_log(cat, level, file, ಸಾಲು, func, "%s\n", buf);

		/* update references */
		ದತ್ತಾಂಶ += thislinelen * ಅಗಲ;
		addr += thislinelen * ಅಗಲ;
		ಎಣಿಕೆ_2 -= thislinelen;
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ log_ಕೂಡಿಸು_filter_flags(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *drv_name, ಎಣಿಕೆ log_category_t cat_list[],
			 ಎಣಿಕೆ log_level_t level, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *file_list,
			 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *func_list, ಪೂರ್ಣಾಂಕ flags)
{
	ರಚನೆ log_filter *filt;
	ರಚನೆ log_device *ldev;
	ಪೂರ್ಣಾಂಕ ret;
	ಪೂರ್ಣಾಂಕ i;

	ldev = log_device_ಕಂಡುಹಿಡಿ_by_name(drv_name);
	ಆದರೆ (!ldev)
		ಹಿಂತಿರುಗಿ -ENOENT;
	filt = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(*filt));
	ಆದರೆ (!filt)
		ಹಿಂತಿರುಗಿ -ENOMEM;

	filt->flags = flags;
	ಆದರೆ (cat_list) {
		filt->flags |= LOGFF_HAS_CAT;
		ಗಾಗಿ (i = 0; ; i++) {
			ಆದರೆ (i == ARRAY_ಗಾತ್ರ(filt->cat_list)) {
				ret = -ENOSPC;
				ಹೋಗು err;
			}
			filt->cat_list[i] = cat_list[i];
			ಆದರೆ (cat_list[i] == LOGC_ಕೊನೆ)
				ನಿಲ್ಲಿಸು;
		}
	}
	filt->level = level;
	ಆದರೆ (file_list) {
		filt->file_list = strdup(file_list);
		ಆದರೆ (!filt->file_list) {
			ret = -ENOMEM;
			ಹೋಗು err;
		}
	}
	ಆದರೆ (func_list) {
		filt->func_list = strdup(func_list);
		ಆದರೆ (!filt->func_list) {
			ret = -ENOMEM;
			ಹೋಗು err;
		}
	}
	filt->filter_ಅಂಕೆ = ldev->ಮುಂದಿನ_filter_ಅಂಕೆ++;
	/* Add deny filters to the beginning of the list */
	ಆದರೆ (flags & LOGFF_DENY)
		list_ಕೂಡಿಸು(&filt->sibling_ಗಂಟು, &ldev->filter_ತಲೆ);
	ಇಲ್ಲದಿದ್ದರೆ
		list_ಕೂಡಿಸು_ಬಾಲ(&filt->sibling_ಗಂಟು, &ldev->filter_ತಲೆ);

	ಹಿಂತಿರುಗಿ filt->filter_ಅಂಕೆ;

err:
	ಬಿಡುಗಡೆ(filt);
	ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ log_remove_filter(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *drv_name, ಪೂರ್ಣಾಂಕ filter_ಅಂಕೆ)
{
	ರಚನೆ log_filter *filt;
	ರಚನೆ log_device *ldev;

	ldev = log_device_ಕಂಡುಹಿಡಿ_by_name(drv_name);
	ಆದರೆ (!ldev)
		ಹಿಂತಿರುಗಿ -ENOENT;

	list_for_each_ನಮೂದು(filt, &ldev->filter_ತಲೆ, sibling_ಗಂಟು) {
		ಆದರೆ (filt->filter_ಅಂಕೆ == filter_ಅಂಕೆ) {
			list_del(&filt->sibling_ಗಂಟು);
			ಬಿಡುಗಡೆ(filt);

			ಹಿಂತಿರುಗಿ 0;
		}
	}

	ಹಿಂತಿರುಗಿ -ENOENT;
}

/**
 * log_find_device_by_drv() - Find a device by its driver
 *
 * @drv: Log driver
 * Return: Device associated with that driver, or NULL if not found
 */
ಸ್ಥಿರ ರಚನೆ log_device *log_ಕಂಡುಹಿಡಿ_device_by_drv(ರಚನೆ log_driver *drv)
{
	ರಚನೆ log_device *ldev;

	list_for_each_ನಮೂದು(ldev, &gd->log_ತಲೆ, sibling_ಗಂಟು) {
		ಆದರೆ (ldev->drv == drv)
			ಹಿಂತಿರುಗಿ ldev;
	}
	/*
	 * It is quite hard to pass an invalid driver since passing an unknown
	 * LOG_GET_DRIVER(xxx) would normally produce a compilation error. But
	 * it is possible to pass NULL, for example, so this
	 */

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಪೂರ್ಣಾಂಕ log_device_ಗಣ_enable(ರಚನೆ log_driver *drv, bool enable)
{
	ರಚನೆ log_device *ldev;

	ldev = log_ಕಂಡುಹಿಡಿ_device_by_drv(drv);
	ಆದರೆ (!ldev)
		ಹಿಂತಿರುಗಿ -ENOENT;
	ಆದರೆ (enable)
		ldev->flags |= LOGDF_ENABLE;
	ಇಲ್ಲದಿದ್ದರೆ
		ldev->flags &= ~LOGDF_ENABLE;

	ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ log_fixup_for_gd_move(ರಚನೆ global_ದತ್ತಾಂಶ *new_gd)
{
	new_gd->log_ತಲೆ.prev->ಮುಂದಿನ = &new_gd->log_ತಲೆ;
}

ಪೂರ್ಣಾಂಕ log_init(ಶೂನ್ಯ)
{
	ರಚನೆ log_driver *drv = ll_ನಮೂದು_ಆರಂಭ(ರಚನೆ log_driver, log_driver);
	ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = ll_ನಮೂದು_ಎಣಿಕೆ(ರಚನೆ log_driver, log_driver);
	ರಚನೆ log_driver *ಕೊನೆ_2 = drv + ಎಣಿಕೆ_2;

	/*
	 * We cannot add runtime data to the driver since it is likely stored
	 * in rodata. Instead, set up a 'device' corresponding to each driver.
	 * We only support having a single device for each driver.
	 */
	INIT_LIST_ತಲೆ((ರಚನೆ list_ತಲೆ *)&gd->log_ತಲೆ);
	ಆಗ (drv < ಕೊನೆ_2) {
		ರಚನೆ log_device *ldev;

		ldev = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(*ldev));
		ಆದರೆ (!ldev) {
			debug("%s: Cannot allocate memory\n", __func__);
			ಹಿಂತಿರುಗಿ -ENOMEM;
		}
		INIT_LIST_ತಲೆ(&ldev->filter_ತಲೆ);
		ldev->drv = drv;
		ldev->flags = drv->flags;
		list_ಕೂಡಿಸು_ಬಾಲ(&ldev->sibling_ಗಂಟು,
			      (ರಚನೆ list_ತಲೆ *)&gd->log_ತಲೆ);
		drv++;
	}
	gd->flags |= GD_FLG_LOG_READY;
	ಆದರೆ (!gd->default_log_level)
		gd->default_log_level = CONFIG_LOG_DEFAULT_LEVEL;
	gd->log_fmt = log_get_default_format();
	gd->logc_prev = LOGC_NONE;
	gd->logl_prev = LOGL_INFO;

	ಹಿಂತಿರುಗಿ 0;
}
