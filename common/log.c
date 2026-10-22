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

DECLARE_GLOBAL_दत्त_PTR;

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

_Static_assert(ARRAY_आकार(log_cat_name) == LOGC_गणनम् - LOGC_NONE,
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

_Static_assert(ARRAY_आकार(log_level_name) == LOGL_गणनम्, "log_level_name size");

/* All error responses MUST begin with '<' */
ध्रुव वर्ण *log_get_cat_name(गणना log_category_t cat)
{
	ध्रुव वर्ण *name;

	यदि (cat < 0 || cat >= LOGC_गणनम्)
		प्रत्यागम "<invalid>";
	यदि (cat >= LOGC_NONE)
		प्रत्यागम log_cat_name[cat - LOGC_NONE];

#यदि CONFIG_IS_ENABLED(DM)
	name = uclass_get_name((गणना uclass_id)cat);
#अन्यथा
	name = रिक्त;
#यदि_अन्तः

	प्रत्यागम name ? name : "<missing>";
}

गणना log_category_t log_get_cat_by_name(ध्रुव वर्ण *name)
{
	गणना uclass_id id;
	पूर्णाङ्क i;

	प्रति (i = LOGC_NONE; i < LOGC_गणनम्; i++)
		यदि (!वाक्यं_तुलय(name, log_cat_name[i - LOGC_NONE]))
			प्रत्यागम i;
	id = uclass_get_by_name(name);
	यदि (id != UCLASS_INVALID)
		प्रत्यागम (गणना log_category_t)id;

	प्रत्यागम LOGC_NONE;
}

ध्रुव वर्ण *log_get_level_name(गणना log_level_t level)
{
	यदि (level >= LOGL_गणनम्)
		प्रत्यागम "INVALID";
	प्रत्यागम log_level_name[level];
}

गणना log_level_t log_get_level_by_name(ध्रुव वर्ण *name)
{
	पूर्णाङ्क i;

	प्रति (i = 0; i < LOGL_गणनम्; i++) {
		यदि (!strcasecmp(log_level_name[i], name))
			प्रत्यागम i;
	}

	प्रत्यागम LOGL_NONE;
}

संरचना log_device *log_device_find_by_name(ध्रुव वर्ण *drv_name)
{
	संरचना log_device *ldev;

	list_for_each_प्रविष्टिः(ldev, &gd->log_शीर्ष, sibling_ग्रन्थिः) {
		यदि (!वाक्यं_तुलय(drv_name, ldev->drv->name))
			प्रत्यागम ldev;
	}

	प्रत्यागम रिक्त;
}

bool log_has_cat(गणना log_category_t cat_list[], गणना log_category_t cat)
{
	पूर्णाङ्क i;

	प्रति (i = 0; i < LOGF_MAX_CATEGORIES && cat_list[i] != LOGC_अन्तः; i++) {
		यदि (cat_list[i] == cat)
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
	पूर्णाङ्क member_len = वाक्यदैर्घ्यम्(member);
	ध्रुव वर्ण *s, *p;
	पूर्णाङ्क substr_len;

	प्रति (s = list; *s; s = p + (*p != '\0')) {
		p = strchrnul(s, ',');
		substr_len = p - s;
		यदि (member_len >= substr_len &&
		    !वाक्ये_तुलय(member + member_len - substr_len, s, substr_len))
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

	यदि (rec->flags & LOGRECF_FORCE_DEBUG)
		प्रत्यागम true;

	/* If there are no filters, filter on the default log level */
	यदि (list_रिक्तानि(&ldev->filter_शीर्ष)) {
		यदि (rec->level > gd->default_log_level)
			प्रत्यागम false;
		प्रत्यागम true;
	}

	list_for_each_प्रविष्टिः(filt, &ldev->filter_शीर्ष, sibling_ग्रन्थिः) {
		यदि (filt->flags & LOGFF_LEVEL_MIN) {
			यदि (rec->level < filt->level)
				अनुवर्त;
		} अन्यथा यदि (rec->level > filt->level) {
			अनुवर्त;
		}

		यदि ((filt->flags & LOGFF_HAS_CAT) &&
		    !log_has_cat(filt->cat_list, rec->cat))
			अनुवर्त;

		यदि (filt->file_list &&
		    !log_has_member(filt->file_list, rec->file))
			अनुवर्त;

		यदि (filt->func_list &&
		    !log_has_member(filt->func_list, rec->func))
			अनुवर्त;

		यदि (filt->flags & LOGFF_DENY)
			प्रत्यागम false;
		अन्यथा
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
स्थिर पूर्णाङ्क log_dispatch(संरचना log_rec *rec, ध्रुव वर्ण *fmt, va_list तर्काः)
{
	संरचना log_device *ldev;
	वर्ण buf[CONFIG_SYS_CBSIZE];

	/*
	 * When a log driver writes messages (e.g. via the network stack) this
	 * may result in further generated messages. We cannot process them here
	 * as this might result in infinite recursion.
	 */
	यदि (gd->processing_सन्देशः)
		प्रत्यागम 1;

	/* Emit message */
	gd->processing_सन्देशः = true;
	list_for_each_प्रविष्टिः(ldev, &gd->log_शीर्ष, sibling_ग्रन्थिः) {
		यदि ((ldev->flags & LOGDF_ENABLE) &&
		    log_passes_filters(ldev, rec)) {
			यदि (!rec->सन्देशः) {
				पूर्णाङ्क len;

				len = vsnprintf(buf, परिमाण(buf), fmt, तर्काः);
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
	va_list तर्काः;

	यदि (!gd)
		प्रत्यागम -ENOSYS;

	/* Check for message continuation */
	यदि (cat == LOGC_CONT)
		cat = gd->logc_पूर्व;
	यदि (level == LOGL_CONT)
		level = gd->logl_पूर्व;

	rec.cat = cat;
	rec.level = level & LOGL_LEVEL_आवरणम्;
	rec.flags = 0;
	यदि (level & LOGL_FORCE_DEBUG)
		rec.flags |= LOGRECF_FORCE_DEBUG;
	यदि (gd->log_cont)
		rec.flags |= LOGRECF_CONT;
	rec.file = file;
	rec.पङ्क्तिः = पङ्क्तिः;
	rec.func = func;
	rec.सन्देशः = रिक्त;

	यदि (!(gd->flags & GD_FLG_LOG_READY)) {
		gd->log_drop_गणनम्++;

		/* display dropped traces with console puts and DEBUG_UART */
		यदि (rec.level <= CONFIG_LOG_DEFAULT_LEVEL ||
		    rec.flags & LOGRECF_FORCE_DEBUG) {
			वर्ण buf[CONFIG_SYS_CBSIZE];

			va_start(तर्काः, fmt);
			vsnprintf(buf, परिमाण(buf), fmt, तर्काः);
			पङ्क्तिं_लिख(buf);
			va_अन्तः(तर्काः);
		}

		प्रत्यागम -ENOSYS;
	}
	va_start(तर्काः, fmt);
	यदि (!log_dispatch(&rec, fmt, तर्काः)) {
		gd->logc_पूर्व = cat;
		gd->logl_पूर्व = level;
	}
	va_अन्तः(तर्काः);

	प्रत्यागम 0;
}

#परिभाषय MAX_पङ्क्तिः_दीर्घता_अष्टकानि		64
#परिभाषय DEFAULT_पङ्क्तिः_दीर्घता_अष्टकानि	16

पूर्णाङ्क log_बफर(गणना log_category_t cat, गणना log_level_t level,
		ध्रुव वर्ण *file, पूर्णाङ्क पङ्क्तिः, ध्रुव वर्ण *func, ulong addr,
		ध्रुव शून्य *दत्त, uint विस्तारः, uint गणनम्, uint linelen)
{
	यदि (linelen * विस्तारः > MAX_पङ्क्तिः_दीर्घता_अष्टकानि)
		linelen = MAX_पङ्क्तिः_दीर्घता_अष्टकानि / विस्तारः;
	यदि (linelen < 1)
		linelen = DEFAULT_पङ्क्तिः_दीर्घता_अष्टकानि / विस्तारः;

	यावत् (गणनम्) {
		uint thislinelen;
		वर्ण buf[HEXDUMP_MAX_BUF_दीर्घता(विस्तारः * linelen)];

		thislinelen = hexdump_पङ्क्तिः(addr, दत्त, विस्तारः, गणनम्, linelen,
					   buf, परिमाण(buf));
		प्रतिज्ञा(thislinelen >= 0);
		_log(cat, level, file, पङ्क्तिः, func, "%s\n", buf);

		/* update references */
		दत्त += thislinelen * विस्तारः;
		addr += thislinelen * विस्तारः;
		गणनम् -= thislinelen;
	}

	प्रत्यागम 0;
}

पूर्णाङ्क log_सङ्कलय_filter_flags(ध्रुव वर्ण *drv_name, गणना log_category_t cat_list[],
			 गणना log_level_t level, ध्रुव वर्ण *file_list,
			 ध्रुव वर्ण *func_list, पूर्णाङ्क flags)
{
	संरचना log_filter *filt;
	संरचना log_device *ldev;
	पूर्णाङ्क ret;
	पूर्णाङ्क i;

	ldev = log_device_find_by_name(drv_name);
	यदि (!ldev)
		प्रत्यागम -ENOENT;
	filt = शून्यस्मृतिं_देहि(1, परिमाण(*filt));
	यदि (!filt)
		प्रत्यागम -ENOMEM;

	filt->flags = flags;
	यदि (cat_list) {
		filt->flags |= LOGFF_HAS_CAT;
		प्रति (i = 0; ; i++) {
			यदि (i == ARRAY_आकार(filt->cat_list)) {
				ret = -ENOSPC;
				गच्छ err;
			}
			filt->cat_list[i] = cat_list[i];
			यदि (cat_list[i] == LOGC_अन्तः)
				भङ्ग;
		}
	}
	filt->level = level;
	यदि (file_list) {
		filt->file_list = strdup(file_list);
		यदि (!filt->file_list) {
			ret = -ENOMEM;
			गच्छ err;
		}
	}
	यदि (func_list) {
		filt->func_list = strdup(func_list);
		यदि (!filt->func_list) {
			ret = -ENOMEM;
			गच्छ err;
		}
	}
	filt->filter_अङ्कः = ldev->अग्र_filter_अङ्कः++;
	/* Add deny filters to the beginning of the list */
	यदि (flags & LOGFF_DENY)
		list_सङ्कलय(&filt->sibling_ग्रन्थिः, &ldev->filter_शीर्ष);
	अन्यथा
		list_सङ्कलय_पुच्छ(&filt->sibling_ग्रन्थिः, &ldev->filter_शीर्ष);

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
	यदि (!ldev)
		प्रत्यागम -ENOENT;

	list_for_each_प्रविष्टिः(filt, &ldev->filter_शीर्ष, sibling_ग्रन्थिः) {
		यदि (filt->filter_अङ्कः == filter_अङ्कः) {
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

	list_for_each_प्रविष्टिः(ldev, &gd->log_शीर्ष, sibling_ग्रन्थिः) {
		यदि (ldev->drv == drv)
			प्रत्यागम ldev;
	}
	/*
	 * It is quite hard to pass an invalid driver since passing an unknown
	 * LOG_GET_DRIVER(xxx) would normally produce a compilation error. But
	 * it is possible to pass NULL, for example, so this
	 */

	प्रत्यागम रिक्त;
}

पूर्णाङ्क log_device_समुच्चयः_enable(संरचना log_driver *drv, bool enable)
{
	संरचना log_device *ldev;

	ldev = log_find_device_by_drv(drv);
	यदि (!ldev)
		प्रत्यागम -ENOENT;
	यदि (enable)
		ldev->flags |= LOGDF_ENABLE;
	अन्यथा
		ldev->flags &= ~LOGDF_ENABLE;

	प्रत्यागम 0;
}

शून्य log_fixup_for_gd_move(संरचना global_दत्त *new_gd)
{
	new_gd->log_शीर्ष.पूर्व->अग्र = &new_gd->log_शीर्ष;
}

पूर्णाङ्क log_init(शून्य)
{
	संरचना log_driver *drv = ll_प्रविष्टिः_start(संरचना log_driver, log_driver);
	ध्रुव पूर्णाङ्क गणनम् = ll_प्रविष्टिः_गणनम्(संरचना log_driver, log_driver);
	संरचना log_driver *अन्तः_2 = drv + गणनम्;

	/*
	 * We cannot add runtime data to the driver since it is likely stored
	 * in rodata. Instead, set up a 'device' corresponding to each driver.
	 * We only support having a single device for each driver.
	 */
	INIT_LIST_शीर्ष((संरचना list_शीर्ष *)&gd->log_शीर्ष);
	यावत् (drv < अन्तः_2) {
		संरचना log_device *ldev;

		ldev = शून्यस्मृतिं_देहि(1, परिमाण(*ldev));
		यदि (!ldev) {
			debug("%s: Cannot allocate memory\n", __func__);
			प्रत्यागम -ENOMEM;
		}
		INIT_LIST_शीर्ष(&ldev->filter_शीर्ष);
		ldev->drv = drv;
		ldev->flags = drv->flags;
		list_सङ्कलय_पुच्छ(&ldev->sibling_ग्रन्थिः,
			      (संरचना list_शीर्ष *)&gd->log_शीर्ष);
		drv++;
	}
	gd->flags |= GD_FLG_LOG_READY;
	यदि (!gd->default_log_level)
		gd->default_log_level = CONFIG_LOG_DEFAULT_LEVEL;
	gd->log_fmt = log_get_default_format();
	gd->logc_पूर्व = LOGC_NONE;
	gd->logl_पूर्व = LOGL_INFO;

	प्रत्यागम 0;
}
