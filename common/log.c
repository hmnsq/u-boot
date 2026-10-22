// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#अन्तर्भाव <display_options.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <dm/uclass.h>

DECLARE_GLOBAL_DATA_PTR;

स्थिर ध्रुव वर्ण *ध्रुव log_cat_name[] = {
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

_Static_assert(ARRAY_SIZE(log_cat_name) == LOGC_गणनम् - LOGC_NONE,
	       "log_cat_name size");

स्थिर ध्रुव वर्ण *ध्रुव log_level_name[] = {
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

_Static_assert(ARRAY_SIZE(log_level_name) == LOGL_गणनम्, "log_level_name size");

/* All error responses MUST begin with '<' */
ध्रुव वर्ण *log_get_cat_name(गणना log_category_t cat)
{
	ध्रुव वर्ण *name;

	जइ (cat < 0 || cat >= LOGC_गणनम्)
		प्रत्यागम "<invalid>";
	जइ (cat >= LOGC_NONE)
		प्रत्यागम log_cat_name[cat - LOGC_NONE];

#जइ CONFIG_IS_ENABLED(DM)
	name = uclass_get_name((गणना uclass_id)cat);
#अण्णहा
	name = रिक्त;
#endif

	प्रत्यागम name ? name : "<missing>";
}

गणना log_category_t log_get_cat_by_name(ध्रुव वर्ण *name)
{
	गणना uclass_id id;
	पूर्णाङ्क i;

	प्रति (i = LOGC_NONE; i < LOGC_गणनम्; i++)
		जइ (!strcmp(name, log_cat_name[i - LOGC_NONE]))
			प्रत्यागम i;
	id = uclass_get_by_name(name);
	जइ (id != UCLASS_INVALID)
		प्रत्यागम (गणना log_category_t)id;

	प्रत्यागम LOGC_NONE;
}

ध्रुव वर्ण *log_get_level_name(गणना log_level_t level)
{
	जइ (level >= LOGL_गणनम्)
		प्रत्यागम "INVALID";
	प्रत्यागम log_level_name[level];
}

गणना log_level_t log_get_level_by_name(ध्रुव वर्ण *name)
{
	पूर्णाङ्क i;

	प्रति (i = 0; i < LOGL_गणनम्; i++) {
		जइ (!strcasecmp(log_level_name[i], name))
			प्रत्यागम i;
	}

	प्रत्यागम LOGL_NONE;
}

संरचना log_device *log_device_find_by_name(ध्रुव वर्ण *drv_name)
{
	संरचना log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_ग्रन्थिः) {
		जइ (!strcmp(drv_name, ldev->drv->name))
			प्रत्यागम ldev;
	}

	प्रत्यागम रिक्त;
}

bool log_has_cat(गणना log_category_t cat_list[], गणना log_category_t cat)
{
	पूर्णाङ्क i;

	प्रति (i = 0; i < LOGF_MAX_CATEGORIES && cat_list[i] != LOGC_END; i++) {
		जइ (cat_list[i] == cat)
			प्रत्यागम true;
	}

	प्रत्यागम false;
}

/**
 * log_has_member() - check if a string is in a comma separated list
 *
 * @list:	Comma separated list of strings
 * @member:	String to find
 *
 * Return: ``true`` if @member is in @list, else ``false``
 */
स्थिर bool log_has_member(ध्रुव वर्ण *list, ध्रुव वर्ण *member)
{
	पूर्णाङ्क member_len = strlen(member);
	ध्रुव वर्ण *s, *p;
	पूर्णाङ्क substr_len;

	प्रति (s = list; *s; s = p + (*p != '\0')) {
		p = strchrnul(s, ',');
		substr_len = p - s;
		जइ (member_len >= substr_len &&
		    !strncmp(member + member_len - substr_len, s, substr_len))
			प्रत्यागम true;
	}

	प्रत्यागम false;
}

/**
 * log_passes_filters() - check if a log record passes the filters for a device
 *
 * @ldev: Log device to check
 * @rec: Log record to check
 * Return: true if @rec is not blocked by the filters in @ldev, false if it is
 */
स्थिर bool log_passes_filters(संरचना log_device *ldev, संरचना log_rec *rec)
{
	संरचना log_filter *filt;

	जइ (rec->flags & LOGRECF_FORCE_DEBUG)
		प्रत्यागम true;

	/* If there are no filters, filter on the default log level */
	जइ (list_empty(&ldev->filter_head)) {
		जइ (rec->level > gd->default_log_level)
			प्रत्यागम false;
		प्रत्यागम true;
	}

	list_for_each_entry(filt, &ldev->filter_head, sibling_ग्रन्थिः) {
		जइ (filt->flags & LOGFF_LEVEL_MIN) {
			जइ (rec->level < filt->level)
				अनुवर्त;
		} अण्णहा जइ (rec->level > filt->level) {
			अनुवर्त;
		}

		जइ ((filt->flags & LOGFF_HAS_CAT) &&
		    !log_has_cat(filt->cat_list, rec->cat))
			अनुवर्त;

		जइ (filt->file_list &&
		    !log_has_member(filt->file_list, rec->file))
			अनुवर्त;

		जइ (filt->func_list &&
		    !log_has_member(filt->func_list, rec->func))
			अनुवर्त;

		जइ (filt->flags & LOGFF_DENY)
			प्रत्यागम false;
		अण्णहा
			प्रत्यागम true;
	}

	प्रत्यागम false;
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
स्थिर पूर्णाङ्क log_dispatch(संरचना log_rec *rec, ध्रुव वर्ण *fmt, va_list args)
{
	संरचना log_device *ldev;
	वर्ण buf[CONFIG_SYS_CBSIZE];

	/*
	 * When a log driver writes messages (e.g. via the network stack) this
	 * may result in further generated messages. We cannot process them here
	 * as this might result in infinite recursion.
	 */
	जइ (gd->processing_सन्देशः)
		प्रत्यागम 1;

	/* Emit message */
	gd->processing_सन्देशः = true;
	list_for_each_entry(ldev, &gd->log_head, sibling_ग्रन्थिः) {
		जइ ((ldev->flags & LOGDF_ENABLE) &&
		    log_passes_filters(ldev, rec)) {
			जइ (!rec->सन्देशः) {
				पूर्णाङ्क len;

				len = vsnprintf(buf, परिमाण(buf), fmt, args);
				rec->सन्देशः = buf;
				gd->log_cont = len && buf[len - 1] != '\n';
			}
			ldev->drv->emit(ldev, rec);
		}
	}
	gd->processing_सन्देशः = false;
	प्रत्यागम 0;
}

पूर्णाङ्क _log(गणना log_category_t cat, गणना log_level_t level, ध्रुव वर्ण *file,
	 पूर्णाङ्क पङ्क्तिः, ध्रुव वर्ण *func, ध्रुव वर्ण *fmt, ...)
{
	संरचना log_rec rec;
	va_list args;

	जइ (!gd)
		प्रत्यागम -ENOSYS;

	/* Check for message continuation */
	जइ (cat == LOGC_CONT)
		cat = gd->logc_prev;
	जइ (level == LOGL_CONT)
		level = gd->logl_prev;

	rec.cat = cat;
	rec.level = level & LOGL_LEVEL_MASK;
	rec.flags = 0;
	जइ (level & LOGL_FORCE_DEBUG)
		rec.flags |= LOGRECF_FORCE_DEBUG;
	जइ (gd->log_cont)
		rec.flags |= LOGRECF_CONT;
	rec.file = file;
	rec.पङ्क्तिः = पङ्क्तिः;
	rec.func = func;
	rec.सन्देशः = रिक्त;

	जइ (!(gd->flags & GD_FLG_LOG_READY)) {
		gd->log_drop_गणनम्++;

		/* display dropped traces with console puts and DEBUG_UART */
		जइ (rec.level <= CONFIG_LOG_DEFAULT_LEVEL ||
		    rec.flags & LOGRECF_FORCE_DEBUG) {
			वर्ण buf[CONFIG_SYS_CBSIZE];

			va_start(args, fmt);
			vsnprintf(buf, परिमाण(buf), fmt, args);
			puts(buf);
			va_end(args);
		}

		प्रत्यागम -ENOSYS;
	}
	va_start(args, fmt);
	जइ (!log_dispatch(&rec, fmt, args)) {
		gd->logc_prev = cat;
		gd->logl_prev = level;
	}
	va_end(args);

	प्रत्यागम 0;
}

#define MAX_पङ्क्तिः_LENGTH_BYTES		64
#define DEFAULT_पङ्क्तिः_LENGTH_BYTES	16

पूर्णाङ्क _log_buffer(गणना log_category_t cat, गणना log_level_t level,
		ध्रुव वर्ण *file, पूर्णाङ्क पङ्क्तिः, ध्रुव वर्ण *func, ulong addr,
		ध्रुव शून्य *data, uint width, uint गणनम्, uint linelen)
{
	जइ (linelen * width > MAX_पङ्क्तिः_LENGTH_BYTES)
		linelen = MAX_पङ्क्तिः_LENGTH_BYTES / width;
	जइ (linelen < 1)
		linelen = DEFAULT_पङ्क्तिः_LENGTH_BYTES / width;

	जाव (गणनम्) {
		uint thislinelen;
		वर्ण buf[HEXDUMP_MAX_BUF_LENGTH(width * linelen)];

		thislinelen = hexdump_पङ्क्तिः(addr, data, width, गणनम्, linelen,
					   buf, परिमाण(buf));
		प्रतिज्ञा(thislinelen >= 0);
		_log(cat, level, file, पङ्क्तिः, func, "%s\n", buf);

		/* update references */
		data += thislinelen * width;
		addr += thislinelen * width;
		गणनम् -= thislinelen;
	}

	प्रत्यागम 0;
}

पूर्णाङ्क log_add_filter_flags(ध्रुव वर्ण *drv_name, गणना log_category_t cat_list[],
			 गणना log_level_t level, ध्रुव वर्ण *file_list,
			 ध्रुव वर्ण *func_list, पूर्णाङ्क flags)
{
	संरचना log_filter *filt;
	संरचना log_device *ldev;
	पूर्णाङ्क ret;
	पूर्णाङ्क i;

	ldev = log_device_find_by_name(drv_name);
	जइ (!ldev)
		प्रत्यागम -ENOENT;
	filt = calloc(1, परिमाण(*filt));
	जइ (!filt)
		प्रत्यागम -ENOMEM;

	filt->flags = flags;
	जइ (cat_list) {
		filt->flags |= LOGFF_HAS_CAT;
		प्रति (i = 0; ; i++) {
			जइ (i == ARRAY_SIZE(filt->cat_list)) {
				ret = -ENOSPC;
				गच्छ err;
			}
			filt->cat_list[i] = cat_list[i];
			जइ (cat_list[i] == LOGC_END)
				भङ्ग;
		}
	}
	filt->level = level;
	जइ (file_list) {
		filt->file_list = strdup(file_list);
		जइ (!filt->file_list) {
			ret = -ENOMEM;
			गच्छ err;
		}
	}
	जइ (func_list) {
		filt->func_list = strdup(func_list);
		जइ (!filt->func_list) {
			ret = -ENOMEM;
			गच्छ err;
		}
	}
	filt->filter_अङ्कः = ldev->next_filter_अङ्कः++;
	/* Add deny filters to the beginning of the list */
	जइ (flags & LOGFF_DENY)
		list_add(&filt->sibling_ग्रन्थिः, &ldev->filter_head);
	अण्णहा
		list_add_tail(&filt->sibling_ग्रन्थिः, &ldev->filter_head);

	प्रत्यागम filt->filter_अङ्कः;

err:
	मुक्त(filt);
	प्रत्यागम ret;
}

पूर्णाङ्क log_remove_filter(ध्रुव वर्ण *drv_name, पूर्णाङ्क filter_अङ्कः)
{
	संरचना log_filter *filt;
	संरचना log_device *ldev;

	ldev = log_device_find_by_name(drv_name);
	जइ (!ldev)
		प्रत्यागम -ENOENT;

	list_for_each_entry(filt, &ldev->filter_head, sibling_ग्रन्थिः) {
		जइ (filt->filter_अङ्कः == filter_अङ्कः) {
			list_del(&filt->sibling_ग्रन्थिः);
			मुक्त(filt);

			प्रत्यागम 0;
		}
	}

	प्रत्यागम -ENOENT;
}

/**
 * log_find_device_by_drv() - Find a device by its driver
 *
 * @drv: Log driver
 * Return: Device associated with that driver, or NULL if not found
 */
स्थिर संरचना log_device *log_find_device_by_drv(संरचना log_driver *drv)
{
	संरचना log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_ग्रन्थिः) {
		जइ (ldev->drv == drv)
			प्रत्यागम ldev;
	}
	/*
	 * It is quite hard to pass an invalid driver since passing an unknown
	 * LOG_GET_DRIVER(xxx) would normally produce a compilation error. But
	 * it is possible to pass NULL, for example, so this
	 */

	प्रत्यागम रिक्त;
}

पूर्णाङ्क log_device_set_enable(संरचना log_driver *drv, bool enable)
{
	संरचना log_device *ldev;

	ldev = log_find_device_by_drv(drv);
	जइ (!ldev)
		प्रत्यागम -ENOENT;
	जइ (enable)
		ldev->flags |= LOGDF_ENABLE;
	अण्णहा
		ldev->flags &= ~LOGDF_ENABLE;

	प्रत्यागम 0;
}

शून्य log_fixup_for_gd_move(संरचना global_data *new_gd)
{
	new_gd->log_head.prev->next = &new_gd->log_head;
}

पूर्णाङ्क log_init(शून्य)
{
	संरचना log_driver *drv = ll_entry_start(संरचना log_driver, log_driver);
	ध्रुव पूर्णाङ्क गणनम् = ll_entry_गणनम्(संरचना log_driver, log_driver);
	संरचना log_driver *end = drv + गणनम्;

	/*
	 * We cannot add runtime data to the driver since it is likely stored
	 * in rodata. Instead, set up a 'device' corresponding to each driver.
	 * We only support having a single device for each driver.
	 */
	INIT_LIST_HEAD((संरचना list_head *)&gd->log_head);
	जाव (drv < end) {
		संरचना log_device *ldev;

		ldev = calloc(1, परिमाण(*ldev));
		जइ (!ldev) {
			debug("%s: Cannot allocate memory\n", __func__);
			प्रत्यागम -ENOMEM;
		}
		INIT_LIST_HEAD(&ldev->filter_head);
		ldev->drv = drv;
		ldev->flags = drv->flags;
		list_add_tail(&ldev->sibling_ग्रन्थिः,
			      (संरचना list_head *)&gd->log_head);
		drv++;
	}
	gd->flags |= GD_FLG_LOG_READY;
	जइ (!gd->default_log_level)
		gd->default_log_level = CONFIG_LOG_DEFAULT_LEVEL;
	gd->log_fmt = log_get_default_format();
	gd->logc_prev = LOGC_NONE;
	gd->logl_prev = LOGL_INFO;

	प्रत्यागम 0;
}
