// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#समावेश <display_options.h>
#समावेश <log.h>
#समावेश <malloc.h>
#समावेश <asm/global_data.h>
#समावेश <dm/uclass.h>

DECLARE_GLOBAL_तथ्य_PTR;

अटल स्थिर अक्षर *स्थिर log_cat_name[] = {
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

_Static_assert(ARRAY_आकार(log_cat_name) == LOGC_गिनती - LOGC_NONE,
	       "log_cat_name size");

अटल स्थिर अक्षर *स्थिर log_level_name[] = {
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

_Static_assert(ARRAY_आकार(log_level_name) == LOGL_गिनती, "log_level_name size");

/* All error responses MUST begin with '<' */
स्थिर अक्षर *log_get_cat_name(क्रमागत log_category_t cat)
{
	स्थिर अक्षर *name;

	अगर (cat < 0 || cat >= LOGC_गिनती)
		वापस "<invalid>";
	अगर (cat >= LOGC_NONE)
		वापस log_cat_name[cat - LOGC_NONE];

#अगर CONFIG_IS_ENABLED(DM)
	name = uclass_get_name((क्रमागत uclass_id)cat);
#अन्यथा
	name = शून्य;
#पूर्ण_अगर

	वापस name ? name : "<missing>";
}

क्रमागत log_category_t log_get_cat_by_name(स्थिर अक्षर *name)
{
	क्रमागत uclass_id id;
	पूर्णांक i;

	क्रम (i = LOGC_NONE; i < LOGC_गिनती; i++)
		अगर (!म_भेद(name, log_cat_name[i - LOGC_NONE]))
			वापस i;
	id = uclass_get_by_name(name);
	अगर (id != UCLASS_INVALID)
		वापस (क्रमागत log_category_t)id;

	वापस LOGC_NONE;
}

स्थिर अक्षर *log_get_level_name(क्रमागत log_level_t level)
{
	अगर (level >= LOGL_गिनती)
		वापस "INVALID";
	वापस log_level_name[level];
}

क्रमागत log_level_t log_get_level_by_name(स्थिर अक्षर *name)
{
	पूर्णांक i;

	क्रम (i = 0; i < LOGL_गिनती; i++) {
		अगर (!strcasecmp(log_level_name[i], name))
			वापस i;
	}

	वापस LOGL_NONE;
}

काष्ठा log_device *log_device_find_by_name(स्थिर अक्षर *drv_name)
{
	काष्ठा log_device *ldev;

	list_for_each_entry(ldev, &gd->log_शीर्ष, sibling_गाँठ) {
		अगर (!म_भेद(drv_name, ldev->drv->name))
			वापस ldev;
	}

	वापस शून्य;
}

bool log_has_cat(क्रमागत log_category_t cat_list[], क्रमागत log_category_t cat)
{
	पूर्णांक i;

	क्रम (i = 0; i < LOGF_MAX_CATEGORIES && cat_list[i] != LOGC_END; i++) {
		अगर (cat_list[i] == cat)
			वापस true;
	}

	वापस false;
}

/**
 * log_has_member() - check if a string is in a comma separated list
 *
 * @list:	Comma separated list of strings
 * @member:	String to find
 *
 * Return: ``true`` if @member is in @list, else ``false``
 */
अटल bool log_has_member(स्थिर अक्षर *list, स्थिर अक्षर *member)
{
	पूर्णांक member_len = म_माप(member);
	स्थिर अक्षर *s, *p;
	पूर्णांक substr_len;

	क्रम (s = list; *s; s = p + (*p != '\0')) {
		p = strchrnul(s, ',');
		substr_len = p - s;
		अगर (member_len >= substr_len &&
		    !म_भेदन(member + member_len - substr_len, s, substr_len))
			वापस true;
	}

	वापस false;
}

/**
 * log_passes_filters() - check if a log record passes the filters for a device
 *
 * @ldev: Log device to check
 * @rec: Log record to check
 * Return: true if @rec is not blocked by the filters in @ldev, false if it is
 */
अटल bool log_passes_filters(काष्ठा log_device *ldev, काष्ठा log_rec *rec)
{
	काष्ठा log_filter *filt;

	अगर (rec->flags & LOGRECF_FORCE_DEBUG)
		वापस true;

	/* If there are no filters, filter on the default log level */
	अगर (list_empty(&ldev->filter_शीर्ष)) {
		अगर (rec->level > gd->default_log_level)
			वापस false;
		वापस true;
	}

	list_for_each_entry(filt, &ldev->filter_शीर्ष, sibling_गाँठ) {
		अगर (filt->flags & LOGFF_LEVEL_MIN) {
			अगर (rec->level < filt->level)
				जारी;
		} अन्यथा अगर (rec->level > filt->level) {
			जारी;
		}

		अगर ((filt->flags & LOGFF_HAS_CAT) &&
		    !log_has_cat(filt->cat_list, rec->cat))
			जारी;

		अगर (filt->file_list &&
		    !log_has_member(filt->file_list, rec->file))
			जारी;

		अगर (filt->func_list &&
		    !log_has_member(filt->func_list, rec->func))
			जारी;

		अगर (filt->flags & LOGFF_DENY)
			वापस false;
		अन्यथा
			वापस true;
	}

	वापस false;
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
अटल पूर्णांक log_dispatch(काष्ठा log_rec *rec, स्थिर अक्षर *fmt, बहु_सूची args)
{
	काष्ठा log_device *ldev;
	अक्षर buf[CONFIG_SYS_CBSIZE];

	/*
	 * When a log driver writes messages (e.g. via the network stack) this
	 * may result in further generated messages. We cannot process them here
	 * as this might result in infinite recursion.
	 */
	अगर (gd->processing_संदेश)
		वापस 1;

	/* Emit message */
	gd->processing_संदेश = true;
	list_for_each_entry(ldev, &gd->log_शीर्ष, sibling_गाँठ) {
		अगर ((ldev->flags & LOGDF_ENABLE) &&
		    log_passes_filters(ldev, rec)) {
			अगर (!rec->संदेश) {
				पूर्णांक len;

				len = vsnprintf(buf, माप(buf), fmt, args);
				rec->संदेश = buf;
				gd->log_cont = len && buf[len - 1] != '\n';
			}
			ldev->drv->emit(ldev, rec);
		}
	}
	gd->processing_संदेश = false;
	वापस 0;
}

पूर्णांक _log(क्रमागत log_category_t cat, क्रमागत log_level_t level, स्थिर अक्षर *file,
	 पूर्णांक पंक्ति, स्थिर अक्षर *func, स्थिर अक्षर *fmt, ...)
{
	काष्ठा log_rec rec;
	बहु_सूची args;

	अगर (!gd)
		वापस -ENOSYS;

	/* Check for message continuation */
	अगर (cat == LOGC_CONT)
		cat = gd->logc_पूर्व;
	अगर (level == LOGL_CONT)
		level = gd->logl_पूर्व;

	rec.cat = cat;
	rec.level = level & LOGL_LEVEL_MASK;
	rec.flags = 0;
	अगर (level & LOGL_FORCE_DEBUG)
		rec.flags |= LOGRECF_FORCE_DEBUG;
	अगर (gd->log_cont)
		rec.flags |= LOGRECF_CONT;
	rec.file = file;
	rec.पंक्ति = पंक्ति;
	rec.func = func;
	rec.संदेश = शून्य;

	अगर (!(gd->flags & GD_FLG_LOG_READY)) {
		gd->log_drop_गिनती++;

		/* display dropped traces with console puts and DEBUG_UART */
		अगर (rec.level <= CONFIG_LOG_DEFAULT_LEVEL ||
		    rec.flags & LOGRECF_FORCE_DEBUG) {
			अक्षर buf[CONFIG_SYS_CBSIZE];

			बहु_शुरू(args, fmt);
			vsnprintf(buf, माप(buf), fmt, args);
			माला_दो(buf);
			बहु_पूर्ण(args);
		}

		वापस -ENOSYS;
	}
	बहु_शुरू(args, fmt);
	अगर (!log_dispatch(&rec, fmt, args)) {
		gd->logc_पूर्व = cat;
		gd->logl_पूर्व = level;
	}
	बहु_पूर्ण(args);

	वापस 0;
}

#घोषणा MAX_पंक्ति_लंबाई_BYTES		64
#घोषणा DEFAULT_पंक्ति_लंबाई_BYTES	16

पूर्णांक log_बफर(क्रमागत log_category_t cat, क्रमागत log_level_t level,
		स्थिर अक्षर *file, पूर्णांक पंक्ति, स्थिर अक्षर *func, ulong addr,
		स्थिर व्योम *तथ्य, uint width, uint गिनती, uint linelen)
{
	अगर (linelen * width > MAX_पंक्ति_लंबाई_BYTES)
		linelen = MAX_पंक्ति_लंबाई_BYTES / width;
	अगर (linelen < 1)
		linelen = DEFAULT_पंक्ति_लंबाई_BYTES / width;

	जबतक (गिनती) {
		uint thislinelen;
		अक्षर buf[HEXDUMP_MAX_BUF_लंबाई(width * linelen)];

		thislinelen = hexdump_पंक्ति(addr, तथ्य, width, गिनती, linelen,
					   buf, माप(buf));
		निश्चित(thislinelen >= 0);
		_log(cat, level, file, पंक्ति, func, "%s\n", buf);

		/* update references */
		तथ्य += thislinelen * width;
		addr += thislinelen * width;
		गिनती -= thislinelen;
	}

	वापस 0;
}

पूर्णांक log_add_filter_flags(स्थिर अक्षर *drv_name, क्रमागत log_category_t cat_list[],
			 क्रमागत log_level_t level, स्थिर अक्षर *file_list,
			 स्थिर अक्षर *func_list, पूर्णांक flags)
{
	काष्ठा log_filter *filt;
	काष्ठा log_device *ldev;
	पूर्णांक ret;
	पूर्णांक i;

	ldev = log_device_find_by_name(drv_name);
	अगर (!ldev)
		वापस -ENOENT;
	filt = सुस्मृति(1, माप(*filt));
	अगर (!filt)
		वापस -ENOMEM;

	filt->flags = flags;
	अगर (cat_list) {
		filt->flags |= LOGFF_HAS_CAT;
		क्रम (i = 0; ; i++) {
			अगर (i == ARRAY_आकार(filt->cat_list)) {
				ret = -ENOSPC;
				जाओ err;
			}
			filt->cat_list[i] = cat_list[i];
			अगर (cat_list[i] == LOGC_END)
				अवरोध;
		}
	}
	filt->level = level;
	अगर (file_list) {
		filt->file_list = strdup(file_list);
		अगर (!filt->file_list) {
			ret = -ENOMEM;
			जाओ err;
		}
	}
	अगर (func_list) {
		filt->func_list = strdup(func_list);
		अगर (!filt->func_list) {
			ret = -ENOMEM;
			जाओ err;
		}
	}
	filt->filter_अंक = ldev->अग्र_filter_अंक++;
	/* Add deny filters to the beginning of the list */
	अगर (flags & LOGFF_DENY)
		list_add(&filt->sibling_गाँठ, &ldev->filter_शीर्ष);
	अन्यथा
		list_add_पुच्छ(&filt->sibling_गाँठ, &ldev->filter_शीर्ष);

	वापस filt->filter_अंक;

err:
	मुक्त(filt);
	वापस ret;
}

पूर्णांक log_remove_filter(स्थिर अक्षर *drv_name, पूर्णांक filter_अंक)
{
	काष्ठा log_filter *filt;
	काष्ठा log_device *ldev;

	ldev = log_device_find_by_name(drv_name);
	अगर (!ldev)
		वापस -ENOENT;

	list_for_each_entry(filt, &ldev->filter_शीर्ष, sibling_गाँठ) {
		अगर (filt->filter_अंक == filter_अंक) {
			list_del(&filt->sibling_गाँठ);
			मुक्त(filt);

			वापस 0;
		}
	}

	वापस -ENOENT;
}

/**
 * log_find_device_by_drv() - Find a device by its driver
 *
 * @drv: Log driver
 * Return: Device associated with that driver, or NULL if not found
 */
अटल काष्ठा log_device *log_find_device_by_drv(काष्ठा log_driver *drv)
{
	काष्ठा log_device *ldev;

	list_for_each_entry(ldev, &gd->log_शीर्ष, sibling_गाँठ) {
		अगर (ldev->drv == drv)
			वापस ldev;
	}
	/*
	 * It is quite hard to pass an invalid driver since passing an unknown
	 * LOG_GET_DRIVER(xxx) would normally produce a compilation error. But
	 * it is possible to pass NULL, for example, so this
	 */

	वापस शून्य;
}

पूर्णांक log_device_set_enable(काष्ठा log_driver *drv, bool enable)
{
	काष्ठा log_device *ldev;

	ldev = log_find_device_by_drv(drv);
	अगर (!ldev)
		वापस -ENOENT;
	अगर (enable)
		ldev->flags |= LOGDF_ENABLE;
	अन्यथा
		ldev->flags &= ~LOGDF_ENABLE;

	वापस 0;
}

व्योम log_fixup_for_gd_move(काष्ठा global_तथ्य *new_gd)
{
	new_gd->log_शीर्ष.पूर्व->अग्र = &new_gd->log_शीर्ष;
}

पूर्णांक log_init(व्योम)
{
	काष्ठा log_driver *drv = ll_entry_start(काष्ठा log_driver, log_driver);
	स्थिर पूर्णांक गिनती = ll_entry_गिनती(काष्ठा log_driver, log_driver);
	काष्ठा log_driver *end = drv + गिनती;

	/*
	 * We cannot add runtime data to the driver since it is likely stored
	 * in rodata. Instead, set up a 'device' corresponding to each driver.
	 * We only support having a single device for each driver.
	 */
	INIT_LIST_शीर्ष((काष्ठा list_शीर्ष *)&gd->log_शीर्ष);
	जबतक (drv < end) {
		काष्ठा log_device *ldev;

		ldev = सुस्मृति(1, माप(*ldev));
		अगर (!ldev) {
			debug("%s: Cannot allocate memory\n", __func__);
			वापस -ENOMEM;
		}
		INIT_LIST_शीर्ष(&ldev->filter_शीर्ष);
		ldev->drv = drv;
		ldev->flags = drv->flags;
		list_add_पुच्छ(&ldev->sibling_गाँठ,
			      (काष्ठा list_शीर्ष *)&gd->log_शीर्ष);
		drv++;
	}
	gd->flags |= GD_FLG_LOG_READY;
	अगर (!gd->default_log_level)
		gd->default_log_level = CONFIG_LOG_DEFAULT_LEVEL;
	gd->log_fmt = log_get_default_format();
	gd->logc_पूर्व = LOGC_NONE;
	gd->logl_पूर्व = LOGL_INFO;

	वापस 0;
}
