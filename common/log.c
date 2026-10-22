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

स्थिर स्थिरांक अक्षर *स्थिरांक log_cat_name[] = {
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

_Static_assert(ARRAY_SIZE(log_cat_name) == LOGC_मोजणी - LOGC_NONE,
	       "log_cat_name size");

स्थिर स्थिरांक अक्षर *स्थिरांक log_level_name[] = {
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

_Static_assert(ARRAY_SIZE(log_level_name) == LOGL_मोजणी, "log_level_name size");

/* All error responses MUST begin with '<' */
स्थिरांक अक्षर *log_get_cat_name(गणना log_category_t cat)
{
	स्थिरांक अक्षर *name;

	जर (cat < 0 || cat >= LOGC_मोजणी)
		परत "<invalid>";
	जर (cat >= LOGC_NONE)
		परत log_cat_name[cat - LOGC_NONE];

#जर CONFIG_IS_ENABLED(DM)
	name = uclass_get_name((गणना uclass_id)cat);
#अन्यथा
	name = रिक्त;
#endif

	परत name ? name : "<missing>";
}

गणना log_category_t log_get_cat_by_name(स्थिरांक अक्षर *name)
{
	गणना uclass_id id;
	पूर्णांक i;

	साठी (i = LOGC_NONE; i < LOGC_मोजणी; i++)
		जर (!म_भेद(name, log_cat_name[i - LOGC_NONE]))
			परत i;
	id = uclass_get_by_name(name);
	जर (id != UCLASS_INVALID)
		परत (गणना log_category_t)id;

	परत LOGC_NONE;
}

स्थिरांक अक्षर *log_get_level_name(गणना log_level_t level)
{
	जर (level >= LOGL_मोजणी)
		परत "INVALID";
	परत log_level_name[level];
}

गणना log_level_t log_get_level_by_name(स्थिरांक अक्षर *name)
{
	पूर्णांक i;

	साठी (i = 0; i < LOGL_मोजणी; i++) {
		जर (!strcasecmp(log_level_name[i], name))
			परत i;
	}

	परत LOGL_NONE;
}

संरचना log_device *log_device_find_by_name(स्थिरांक अक्षर *drv_name)
{
	संरचना log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_गाठ) {
		जर (!म_भेद(drv_name, ldev->drv->name))
			परत ldev;
	}

	परत रिक्त;
}

bool log_has_cat(गणना log_category_t cat_list[], गणना log_category_t cat)
{
	पूर्णांक i;

	साठी (i = 0; i < LOGF_MAX_CATEGORIES && cat_list[i] != LOGC_END; i++) {
		जर (cat_list[i] == cat)
			परत true;
	}

	परत false;
}

/**
 * log_has_member() - check if a string is in a comma separated list
 *
 * @list:	Comma separated list of strings
 * @member:	String to find
 *
 * Return: ``true`` if @member is in @list, else ``false``
 */
स्थिर bool log_has_member(स्थिरांक अक्षर *list, स्थिरांक अक्षर *member)
{
	पूर्णांक member_len = म_माप(member);
	स्थिरांक अक्षर *s, *p;
	पूर्णांक substr_len;

	साठी (s = list; *s; s = p + (*p != '\0')) {
		p = strchrnul(s, ',');
		substr_len = p - s;
		जर (member_len >= substr_len &&
		    !म_भेदन(member + member_len - substr_len, s, substr_len))
			परत true;
	}

	परत false;
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

	जर (rec->flags & LOGRECF_FORCE_DEBUG)
		परत true;

	/* If there are no filters, filter on the default log level */
	जर (list_empty(&ldev->filter_head)) {
		जर (rec->level > gd->default_log_level)
			परत false;
		परत true;
	}

	list_for_each_entry(filt, &ldev->filter_head, sibling_गाठ) {
		जर (filt->flags & LOGFF_LEVEL_MIN) {
			जर (rec->level < filt->level)
				सुरूठेवा;
		} अन्यथा जर (rec->level > filt->level) {
			सुरूठेवा;
		}

		जर ((filt->flags & LOGFF_HAS_CAT) &&
		    !log_has_cat(filt->cat_list, rec->cat))
			सुरूठेवा;

		जर (filt->file_list &&
		    !log_has_member(filt->file_list, rec->file))
			सुरूठेवा;

		जर (filt->func_list &&
		    !log_has_member(filt->func_list, rec->func))
			सुरूठेवा;

		जर (filt->flags & LOGFF_DENY)
			परत false;
		अन्यथा
			परत true;
	}

	परत false;
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
स्थिर पूर्णांक log_dispatch(संरचना log_rec *rec, स्थिरांक अक्षर *fmt, बहु_सूची args)
{
	संरचना log_device *ldev;
	अक्षर buf[CONFIG_SYS_CBSIZE];

	/*
	 * When a log driver writes messages (e.g. via the network stack) this
	 * may result in further generated messages. We cannot process them here
	 * as this might result in infinite recursion.
	 */
	जर (gd->processing_संदेश)
		परत 1;

	/* Emit message */
	gd->processing_संदेश = true;
	list_for_each_entry(ldev, &gd->log_head, sibling_गाठ) {
		जर ((ldev->flags & LOGDF_ENABLE) &&
		    log_passes_filters(ldev, rec)) {
			जर (!rec->संदेश) {
				पूर्णांक len;

				len = vsnprintf(buf, आकार(buf), fmt, args);
				rec->संदेश = buf;
				gd->log_cont = len && buf[len - 1] != '\n';
			}
			ldev->drv->emit(ldev, rec);
		}
	}
	gd->processing_संदेश = false;
	परत 0;
}

पूर्णांक _log(गणना log_category_t cat, गणना log_level_t level, स्थिरांक अक्षर *file,
	 पूर्णांक ओळ, स्थिरांक अक्षर *func, स्थिरांक अक्षर *fmt, ...)
{
	संरचना log_rec rec;
	बहु_सूची args;

	जर (!gd)
		परत -ENOSYS;

	/* Check for message continuation */
	जर (cat == LOGC_CONT)
		cat = gd->logc_prev;
	जर (level == LOGL_CONT)
		level = gd->logl_prev;

	rec.cat = cat;
	rec.level = level & LOGL_LEVEL_MASK;
	rec.flags = 0;
	जर (level & LOGL_FORCE_DEBUG)
		rec.flags |= LOGRECF_FORCE_DEBUG;
	जर (gd->log_cont)
		rec.flags |= LOGRECF_CONT;
	rec.file = file;
	rec.ओळ = ओळ;
	rec.func = func;
	rec.संदेश = रिक्त;

	जर (!(gd->flags & GD_FLG_LOG_READY)) {
		gd->log_drop_मोजणी++;

		/* display dropped traces with console puts and DEBUG_UART */
		जर (rec.level <= CONFIG_LOG_DEFAULT_LEVEL ||
		    rec.flags & LOGRECF_FORCE_DEBUG) {
			अक्षर buf[CONFIG_SYS_CBSIZE];

			बहु_शुरू(args, fmt);
			vsnprintf(buf, आकार(buf), fmt, args);
			माला_दो(buf);
			बहु_पूर्ण(args);
		}

		परत -ENOSYS;
	}
	बहु_शुरू(args, fmt);
	जर (!log_dispatch(&rec, fmt, args)) {
		gd->logc_prev = cat;
		gd->logl_prev = level;
	}
	बहु_पूर्ण(args);

	परत 0;
}

#define MAX_ओळ_LENGTH_BYTES		64
#define DEFAULT_ओळ_LENGTH_BYTES	16

पूर्णांक _log_buffer(गणना log_category_t cat, गणना log_level_t level,
		स्थिरांक अक्षर *file, पूर्णांक ओळ, स्थिरांक अक्षर *func, ulong addr,
		स्थिरांक शून्य *data, uint width, uint मोजणी, uint linelen)
{
	जर (linelen * width > MAX_ओळ_LENGTH_BYTES)
		linelen = MAX_ओळ_LENGTH_BYTES / width;
	जर (linelen < 1)
		linelen = DEFAULT_ओळ_LENGTH_BYTES / width;

	पर्यंत (मोजणी) {
		uint thislinelen;
		अक्षर buf[HEXDUMP_MAX_BUF_LENGTH(width * linelen)];

		thislinelen = hexdump_ओळ(addr, data, width, मोजणी, linelen,
					   buf, आकार(buf));
		प्रतिज्ञा(thislinelen >= 0);
		_log(cat, level, file, ओळ, func, "%s\n", buf);

		/* update references */
		data += thislinelen * width;
		addr += thislinelen * width;
		मोजणी -= thislinelen;
	}

	परत 0;
}

पूर्णांक log_add_filter_flags(स्थिरांक अक्षर *drv_name, गणना log_category_t cat_list[],
			 गणना log_level_t level, स्थिरांक अक्षर *file_list,
			 स्थिरांक अक्षर *func_list, पूर्णांक flags)
{
	संरचना log_filter *filt;
	संरचना log_device *ldev;
	पूर्णांक ret;
	पूर्णांक i;

	ldev = log_device_find_by_name(drv_name);
	जर (!ldev)
		परत -ENOENT;
	filt = सुस्मृति(1, आकार(*filt));
	जर (!filt)
		परत -ENOMEM;

	filt->flags = flags;
	जर (cat_list) {
		filt->flags |= LOGFF_HAS_CAT;
		साठी (i = 0; ; i++) {
			जर (i == ARRAY_SIZE(filt->cat_list)) {
				ret = -ENOSPC;
				जा err;
			}
			filt->cat_list[i] = cat_list[i];
			जर (cat_list[i] == LOGC_END)
				थांबा;
		}
	}
	filt->level = level;
	जर (file_list) {
		filt->file_list = strdup(file_list);
		जर (!filt->file_list) {
			ret = -ENOMEM;
			जा err;
		}
	}
	जर (func_list) {
		filt->func_list = strdup(func_list);
		जर (!filt->func_list) {
			ret = -ENOMEM;
			जा err;
		}
	}
	filt->filter_अंक = ldev->next_filter_अंक++;
	/* Add deny filters to the beginning of the list */
	जर (flags & LOGFF_DENY)
		list_add(&filt->sibling_गाठ, &ldev->filter_head);
	अन्यथा
		list_add_tail(&filt->sibling_गाठ, &ldev->filter_head);

	परत filt->filter_अंक;

err:
	मुक्त(filt);
	परत ret;
}

पूर्णांक log_remove_filter(स्थिरांक अक्षर *drv_name, पूर्णांक filter_अंक)
{
	संरचना log_filter *filt;
	संरचना log_device *ldev;

	ldev = log_device_find_by_name(drv_name);
	जर (!ldev)
		परत -ENOENT;

	list_for_each_entry(filt, &ldev->filter_head, sibling_गाठ) {
		जर (filt->filter_अंक == filter_अंक) {
			list_del(&filt->sibling_गाठ);
			मुक्त(filt);

			परत 0;
		}
	}

	परत -ENOENT;
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

	list_for_each_entry(ldev, &gd->log_head, sibling_गाठ) {
		जर (ldev->drv == drv)
			परत ldev;
	}
	/*
	 * It is quite hard to pass an invalid driver since passing an unknown
	 * LOG_GET_DRIVER(xxx) would normally produce a compilation error. But
	 * it is possible to pass NULL, for example, so this
	 */

	परत रिक्त;
}

पूर्णांक log_device_set_enable(संरचना log_driver *drv, bool enable)
{
	संरचना log_device *ldev;

	ldev = log_find_device_by_drv(drv);
	जर (!ldev)
		परत -ENOENT;
	जर (enable)
		ldev->flags |= LOGDF_ENABLE;
	अन्यथा
		ldev->flags &= ~LOGDF_ENABLE;

	परत 0;
}

शून्य log_fixup_for_gd_move(संरचना global_data *new_gd)
{
	new_gd->log_head.prev->next = &new_gd->log_head;
}

पूर्णांक log_init(शून्य)
{
	संरचना log_driver *drv = ll_entry_start(संरचना log_driver, log_driver);
	स्थिरांक पूर्णांक मोजणी = ll_entry_मोजणी(संरचना log_driver, log_driver);
	संरचना log_driver *end = drv + मोजणी;

	/*
	 * We cannot add runtime data to the driver since it is likely stored
	 * in rodata. Instead, set up a 'device' corresponding to each driver.
	 * We only support having a single device for each driver.
	 */
	INIT_LIST_HEAD((संरचना list_head *)&gd->log_head);
	पर्यंत (drv < end) {
		संरचना log_device *ldev;

		ldev = सुस्मृति(1, आकार(*ldev));
		जर (!ldev) {
			debug("%s: Cannot allocate memory\n", __func__);
			परत -ENOMEM;
		}
		INIT_LIST_HEAD(&ldev->filter_head);
		ldev->drv = drv;
		ldev->flags = drv->flags;
		list_add_tail(&ldev->sibling_गाठ,
			      (संरचना list_head *)&gd->log_head);
		drv++;
	}
	gd->flags |= GD_FLG_LOG_READY;
	जर (!gd->default_log_level)
		gd->default_log_level = CONFIG_LOG_DEFAULT_LEVEL;
	gd->log_fmt = log_get_default_format();
	gd->logc_prev = LOGC_NONE;
	gd->logl_prev = LOGL_INFO;

	परत 0;
}
