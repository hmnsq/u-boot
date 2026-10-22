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

स्थिर अचर वर्ण *अचर log_cat_name[] = {
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

_Static_assert(ARRAY_SIZE(log_cat_name) == LOGC_गन्ती - LOGC_NONE,
	       "log_cat_name size");

स्थिर अचर वर्ण *अचर log_level_name[] = {
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

_Static_assert(ARRAY_SIZE(log_level_name) == LOGL_गन्ती, "log_level_name size");

/* All error responses MUST begin with '<' */
अचर वर्ण *log_get_cat_name(गणना log_category_t cat)
{
	अचर वर्ण *name;

	यदि (cat < 0 || cat >= LOGC_गन्ती)
		फर्काऊ "<invalid>";
	यदि (cat >= LOGC_NONE)
		फर्काऊ log_cat_name[cat - LOGC_NONE];

#यदि CONFIG_IS_ENABLED(DM)
	name = uclass_get_name((गणना uclass_id)cat);
#अन्यथा
	name = रिक्त;
#endif

	फर्काऊ name ? name : "<missing>";
}

गणना log_category_t log_get_cat_by_name(अचर वर्ण *name)
{
	गणना uclass_id id;
	पूर्णाङ्क i;

	लागि (i = LOGC_NONE; i < LOGC_गन्ती; i++)
		यदि (!म_भेद(name, log_cat_name[i - LOGC_NONE]))
			फर्काऊ i;
	id = uclass_get_by_name(name);
	यदि (id != UCLASS_INVALID)
		फर्काऊ (गणना log_category_t)id;

	फर्काऊ LOGC_NONE;
}

अचर वर्ण *log_get_level_name(गणना log_level_t level)
{
	यदि (level >= LOGL_गन्ती)
		फर्काऊ "INVALID";
	फर्काऊ log_level_name[level];
}

गणना log_level_t log_get_level_by_name(अचर वर्ण *name)
{
	पूर्णाङ्क i;

	लागि (i = 0; i < LOGL_गन्ती; i++) {
		यदि (!strcasecmp(log_level_name[i], name))
			फर्काऊ i;
	}

	फर्काऊ LOGL_NONE;
}

संरचना log_device *log_device_find_by_name(अचर वर्ण *drv_name)
{
	संरचना log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_गाँठो) {
		यदि (!म_भेद(drv_name, ldev->drv->name))
			फर्काऊ ldev;
	}

	फर्काऊ रिक्त;
}

bool log_has_cat(गणना log_category_t cat_list[], गणना log_category_t cat)
{
	पूर्णाङ्क i;

	लागि (i = 0; i < LOGF_MAX_CATEGORIES && cat_list[i] != LOGC_END; i++) {
		यदि (cat_list[i] == cat)
			फर्काऊ true;
	}

	फर्काऊ false;
}

/**
 * log_has_member() - check if a string is in a comma separated list
 *
 * @list:	Comma separated list of strings
 * @member:	String to find
 *
 * Return: ``true`` if @member is in @list, else ``false``
 */
स्थिर bool log_has_member(अचर वर्ण *list, अचर वर्ण *member)
{
	पूर्णाङ्क member_len = म_माप(member);
	अचर वर्ण *s, *p;
	पूर्णाङ्क substr_len;

	लागि (s = list; *s; s = p + (*p != '\0')) {
		p = strchrnul(s, ',');
		substr_len = p - s;
		यदि (member_len >= substr_len &&
		    !म_भेदन(member + member_len - substr_len, s, substr_len))
			फर्काऊ true;
	}

	फर्काऊ false;
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
		फर्काऊ true;

	/* If there are no filters, filter on the default log level */
	यदि (list_empty(&ldev->filter_head)) {
		यदि (rec->level > gd->default_log_level)
			फर्काऊ false;
		फर्काऊ true;
	}

	list_for_each_entry(filt, &ldev->filter_head, sibling_गाँठो) {
		यदि (filt->flags & LOGFF_LEVEL_MIN) {
			यदि (rec->level < filt->level)
				जारी;
		} अन्यथा यदि (rec->level > filt->level) {
			जारी;
		}

		यदि ((filt->flags & LOGFF_HAS_CAT) &&
		    !log_has_cat(filt->cat_list, rec->cat))
			जारी;

		यदि (filt->file_list &&
		    !log_has_member(filt->file_list, rec->file))
			जारी;

		यदि (filt->func_list &&
		    !log_has_member(filt->func_list, rec->func))
			जारी;

		यदि (filt->flags & LOGFF_DENY)
			फर्काऊ false;
		अन्यथा
			फर्काऊ true;
	}

	फर्काऊ false;
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
स्थिर पूर्णाङ्क log_dispatch(संरचना log_rec *rec, अचर वर्ण *fmt, बहु_सूची args)
{
	संरचना log_device *ldev;
	वर्ण buf[CONFIG_SYS_CBSIZE];

	/*
	 * When a log driver writes messages (e.g. via the network stack) this
	 * may result in further generated messages. We cannot process them here
	 * as this might result in infinite recursion.
	 */
	यदि (gd->processing_सन्देश)
		फर्काऊ 1;

	/* Emit message */
	gd->processing_सन्देश = true;
	list_for_each_entry(ldev, &gd->log_head, sibling_गाँठो) {
		यदि ((ldev->flags & LOGDF_ENABLE) &&
		    log_passes_filters(ldev, rec)) {
			यदि (!rec->सन्देश) {
				पूर्णाङ्क len;

				len = vsnprintf(buf, परिमाण(buf), fmt, args);
				rec->सन्देश = buf;
				gd->log_cont = len && buf[len - 1] != '\n';
			}
			ldev->drv->emit(ldev, rec);
		}
	}
	gd->processing_सन्देश = false;
	फर्काऊ 0;
}

पूर्णाङ्क _log(गणना log_category_t cat, गणना log_level_t level, अचर वर्ण *file,
	 पूर्णाङ्क पङ्क्ति, अचर वर्ण *func, अचर वर्ण *fmt, ...)
{
	संरचना log_rec rec;
	बहु_सूची args;

	यदि (!gd)
		फर्काऊ -ENOSYS;

	/* Check for message continuation */
	यदि (cat == LOGC_CONT)
		cat = gd->logc_prev;
	यदि (level == LOGL_CONT)
		level = gd->logl_prev;

	rec.cat = cat;
	rec.level = level & LOGL_LEVEL_MASK;
	rec.flags = 0;
	यदि (level & LOGL_FORCE_DEBUG)
		rec.flags |= LOGRECF_FORCE_DEBUG;
	यदि (gd->log_cont)
		rec.flags |= LOGRECF_CONT;
	rec.file = file;
	rec.पङ्क्ति = पङ्क्ति;
	rec.func = func;
	rec.सन्देश = रिक्त;

	यदि (!(gd->flags & GD_FLG_LOG_READY)) {
		gd->log_drop_गन्ती++;

		/* display dropped traces with console puts and DEBUG_UART */
		यदि (rec.level <= CONFIG_LOG_DEFAULT_LEVEL ||
		    rec.flags & LOGRECF_FORCE_DEBUG) {
			वर्ण buf[CONFIG_SYS_CBSIZE];

			बहु_शुरू(args, fmt);
			vsnprintf(buf, परिमाण(buf), fmt, args);
			माला_दो(buf);
			बहु_पूर्ण(args);
		}

		फर्काऊ -ENOSYS;
	}
	बहु_शुरू(args, fmt);
	यदि (!log_dispatch(&rec, fmt, args)) {
		gd->logc_prev = cat;
		gd->logl_prev = level;
	}
	बहु_पूर्ण(args);

	फर्काऊ 0;
}

#define MAX_पङ्क्ति_LENGTH_BYTES		64
#define DEFAULT_पङ्क्ति_LENGTH_BYTES	16

पूर्णाङ्क _log_buffer(गणना log_category_t cat, गणना log_level_t level,
		अचर वर्ण *file, पूर्णाङ्क पङ्क्ति, अचर वर्ण *func, ulong addr,
		अचर शून्य *data, uint width, uint गन्ती, uint linelen)
{
	यदि (linelen * width > MAX_पङ्क्ति_LENGTH_BYTES)
		linelen = MAX_पङ्क्ति_LENGTH_BYTES / width;
	यदि (linelen < 1)
		linelen = DEFAULT_पङ्क्ति_LENGTH_BYTES / width;

	जबसम्म (गन्ती) {
		uint thislinelen;
		वर्ण buf[HEXDUMP_MAX_BUF_LENGTH(width * linelen)];

		thislinelen = hexdump_पङ्क्ति(addr, data, width, गन्ती, linelen,
					   buf, परिमाण(buf));
		प्रतिज्ञा(thislinelen >= 0);
		_log(cat, level, file, पङ्क्ति, func, "%s\n", buf);

		/* update references */
		data += thislinelen * width;
		addr += thislinelen * width;
		गन्ती -= thislinelen;
	}

	फर्काऊ 0;
}

पूर्णाङ्क log_add_filter_flags(अचर वर्ण *drv_name, गणना log_category_t cat_list[],
			 गणना log_level_t level, अचर वर्ण *file_list,
			 अचर वर्ण *func_list, पूर्णाङ्क flags)
{
	संरचना log_filter *filt;
	संरचना log_device *ldev;
	पूर्णाङ्क ret;
	पूर्णाङ्क i;

	ldev = log_device_find_by_name(drv_name);
	यदि (!ldev)
		फर्काऊ -ENOENT;
	filt = सुस्मृति(1, परिमाण(*filt));
	यदि (!filt)
		फर्काऊ -ENOMEM;

	filt->flags = flags;
	यदि (cat_list) {
		filt->flags |= LOGFF_HAS_CAT;
		लागि (i = 0; ; i++) {
			यदि (i == ARRAY_SIZE(filt->cat_list)) {
				ret = -ENOSPC;
				गच्छ err;
			}
			filt->cat_list[i] = cat_list[i];
			यदि (cat_list[i] == LOGC_END)
				रोक;
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
	filt->filter_अङ्क = ldev->next_filter_अङ्क++;
	/* Add deny filters to the beginning of the list */
	यदि (flags & LOGFF_DENY)
		list_add(&filt->sibling_गाँठो, &ldev->filter_head);
	अन्यथा
		list_add_tail(&filt->sibling_गाँठो, &ldev->filter_head);

	फर्काऊ filt->filter_अङ्क;

err:
	मुक्त(filt);
	फर्काऊ ret;
}

पूर्णाङ्क log_remove_filter(अचर वर्ण *drv_name, पूर्णाङ्क filter_अङ्क)
{
	संरचना log_filter *filt;
	संरचना log_device *ldev;

	ldev = log_device_find_by_name(drv_name);
	यदि (!ldev)
		फर्काऊ -ENOENT;

	list_for_each_entry(filt, &ldev->filter_head, sibling_गाँठो) {
		यदि (filt->filter_अङ्क == filter_अङ्क) {
			list_del(&filt->sibling_गाँठो);
			मुक्त(filt);

			फर्काऊ 0;
		}
	}

	फर्काऊ -ENOENT;
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

	list_for_each_entry(ldev, &gd->log_head, sibling_गाँठो) {
		यदि (ldev->drv == drv)
			फर्काऊ ldev;
	}
	/*
	 * It is quite hard to pass an invalid driver since passing an unknown
	 * LOG_GET_DRIVER(xxx) would normally produce a compilation error. But
	 * it is possible to pass NULL, for example, so this
	 */

	फर्काऊ रिक्त;
}

पूर्णाङ्क log_device_set_enable(संरचना log_driver *drv, bool enable)
{
	संरचना log_device *ldev;

	ldev = log_find_device_by_drv(drv);
	यदि (!ldev)
		फर्काऊ -ENOENT;
	यदि (enable)
		ldev->flags |= LOGDF_ENABLE;
	अन्यथा
		ldev->flags &= ~LOGDF_ENABLE;

	फर्काऊ 0;
}

शून्य log_fixup_for_gd_move(संरचना global_data *new_gd)
{
	new_gd->log_head.prev->next = &new_gd->log_head;
}

पूर्णाङ्क log_init(शून्य)
{
	संरचना log_driver *drv = ll_entry_start(संरचना log_driver, log_driver);
	अचर पूर्णाङ्क गन्ती = ll_entry_गन्ती(संरचना log_driver, log_driver);
	संरचना log_driver *end = drv + गन्ती;

	/*
	 * We cannot add runtime data to the driver since it is likely stored
	 * in rodata. Instead, set up a 'device' corresponding to each driver.
	 * We only support having a single device for each driver.
	 */
	INIT_LIST_HEAD((संरचना list_head *)&gd->log_head);
	जबसम्म (drv < end) {
		संरचना log_device *ldev;

		ldev = सुस्मृति(1, परिमाण(*ldev));
		यदि (!ldev) {
			debug("%s: Cannot allocate memory\n", __func__);
			फर्काऊ -ENOMEM;
		}
		INIT_LIST_HEAD(&ldev->filter_head);
		ldev->drv = drv;
		ldev->flags = drv->flags;
		list_add_tail(&ldev->sibling_गाँठो,
			      (संरचना list_head *)&gd->log_head);
		drv++;
	}
	gd->flags |= GD_FLG_LOG_READY;
	यदि (!gd->default_log_level)
		gd->default_log_level = CONFIG_LOG_DEFAULT_LEVEL;
	gd->log_fmt = log_get_default_format();
	gd->logc_prev = LOGC_NONE;
	gd->logl_prev = LOGL_INFO;

	फर्काऊ 0;
}
