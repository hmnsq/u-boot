// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ଅନ୍ତର୍ଭାଵ <display_options.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <malloc.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>
#ଅନ୍ତର୍ଭାଵ <dm/uclass.h>

DECLARE_GLOBAL_DATA_PTR;

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ log_cat_name[] = {
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

_Static_assert(ARRAY_SIZE(log_cat_name) == LOGC_ଗଣନା - LOGC_NONE,
	       "log_cat_name size");

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ log_level_name[] = {
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

_Static_assert(ARRAY_SIZE(log_level_name) == LOGL_ଗଣନା, "log_level_name size");

/* All error responses MUST begin with '<' */
ଅଚଳ ଅକ୍ଷର *log_get_cat_name(ଗଣନା log_category_t cat)
{
	ଅଚଳ ଅକ୍ଷର *name;

	ଯଦି (cat < 0 || cat >= LOGC_ଗଣନା)
		ଫେରନ୍ତୁ "<invalid>";
	ଯଦି (cat >= LOGC_NONE)
		ଫେରନ୍ତୁ log_cat_name[cat - LOGC_NONE];

#ଯଦି CONFIG_IS_ENABLED(DM)
	name = uclass_get_name((ଗଣନା uclass_id)cat);
#ଅନ୍ୟଥା
	name = ଖାଲି;
#endif

	ଫେରନ୍ତୁ name ? name : "<missing>";
}

ଗଣନା log_category_t log_get_cat_by_name(ଅଚଳ ଅକ୍ଷର *name)
{
	ଗଣନା uclass_id id;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପାଇଁ (i = LOGC_NONE; i < LOGC_ଗଣନା; i++)
		ଯଦି (!strcmp(name, log_cat_name[i - LOGC_NONE]))
			ଫେରନ୍ତୁ i;
	id = uclass_get_by_name(name);
	ଯଦି (id != UCLASS_INVALID)
		ଫେରନ୍ତୁ (ଗଣନା log_category_t)id;

	ଫେରନ୍ତୁ LOGC_NONE;
}

ଅଚଳ ଅକ୍ଷର *log_get_level_name(ଗଣନା log_level_t level)
{
	ଯଦି (level >= LOGL_ଗଣନା)
		ଫେରନ୍ତୁ "INVALID";
	ଫେରନ୍ତୁ log_level_name[level];
}

ଗଣନା log_level_t log_get_level_by_name(ଅଚଳ ଅକ୍ଷର *name)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପାଇଁ (i = 0; i < LOGL_ଗଣନା; i++) {
		ଯଦି (!strcasecmp(log_level_name[i], name))
			ଫେରନ୍ତୁ i;
	}

	ଫେରନ୍ତୁ LOGL_NONE;
}

ସଂରଚନା log_device *log_device_find_by_name(ଅଚଳ ଅକ୍ଷର *drv_name)
{
	ସଂରଚନା log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_ଗଣ୍ଠି) {
		ଯଦି (!strcmp(drv_name, ldev->drv->name))
			ଫେରନ୍ତୁ ldev;
	}

	ଫେରନ୍ତୁ ଖାଲି;
}

bool log_has_cat(ଗଣନା log_category_t cat_list[], ଗଣନା log_category_t cat)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପାଇଁ (i = 0; i < LOGF_MAX_CATEGORIES && cat_list[i] != LOGC_END; i++) {
		ଯଦି (cat_list[i] == cat)
			ଫେରନ୍ତୁ true;
	}

	ଫେରନ୍ତୁ false;
}

/**
 * log_has_member() - check if a string is in a comma separated list
 *
 * @list:	Comma separated list of strings
 * @member:	String to find
 *
 * Return: ``true`` if @member is in @list, else ``false``
 */
ସ୍ଥିର bool log_has_member(ଅଚଳ ଅକ୍ଷର *list, ଅଚଳ ଅକ୍ଷର *member)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା member_len = strlen(member);
	ଅଚଳ ଅକ୍ଷର *s, *p;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା substr_len;

	ପାଇଁ (s = list; *s; s = p + (*p != '\0')) {
		p = strchrnul(s, ',');
		substr_len = p - s;
		ଯଦି (member_len >= substr_len &&
		    !strncmp(member + member_len - substr_len, s, substr_len))
			ଫେରନ୍ତୁ true;
	}

	ଫେରନ୍ତୁ false;
}

/**
 * log_passes_filters() - check if a log record passes the filters for a device
 *
 * @ldev: Log device to check
 * @rec: Log record to check
 * Return: true if @rec is not blocked by the filters in @ldev, false if it is
 */
ସ୍ଥିର bool log_passes_filters(ସଂରଚନା log_device *ldev, ସଂରଚନା log_rec *rec)
{
	ସଂରଚନା log_filter *filt;

	ଯଦି (rec->flags & LOGRECF_FORCE_DEBUG)
		ଫେରନ୍ତୁ true;

	/* If there are no filters, filter on the default log level */
	ଯଦି (list_empty(&ldev->filter_head)) {
		ଯଦି (rec->level > gd->default_log_level)
			ଫେରନ୍ତୁ false;
		ଫେରନ୍ତୁ true;
	}

	list_for_each_entry(filt, &ldev->filter_head, sibling_ଗଣ୍ଠି) {
		ଯଦି (filt->flags & LOGFF_LEVEL_MIN) {
			ଯଦି (rec->level < filt->level)
				ଜାରି;
		} ଅନ୍ୟଥା ଯଦି (rec->level > filt->level) {
			ଜାରି;
		}

		ଯଦି ((filt->flags & LOGFF_HAS_CAT) &&
		    !log_has_cat(filt->cat_list, rec->cat))
			ଜାରି;

		ଯଦି (filt->file_list &&
		    !log_has_member(filt->file_list, rec->file))
			ଜାରି;

		ଯଦି (filt->func_list &&
		    !log_has_member(filt->func_list, rec->func))
			ଜାରି;

		ଯଦି (filt->flags & LOGFF_DENY)
			ଫେରନ୍ତୁ false;
		ଅନ୍ୟଥା
			ଫେରନ୍ତୁ true;
	}

	ଫେରନ୍ତୁ false;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା log_dispatch(ସଂରଚନା log_rec *rec, ଅଚଳ ଅକ୍ଷର *fmt, va_list args)
{
	ସଂରଚନା log_device *ldev;
	ଅକ୍ଷର buf[CONFIG_SYS_CBSIZE];

	/*
	 * When a log driver writes messages (e.g. via the network stack) this
	 * may result in further generated messages. We cannot process them here
	 * as this might result in infinite recursion.
	 */
	ଯଦି (gd->processing_ସନ୍ଦେଶ)
		ଫେରନ୍ତୁ 1;

	/* Emit message */
	gd->processing_ସନ୍ଦେଶ = true;
	list_for_each_entry(ldev, &gd->log_head, sibling_ଗଣ୍ଠି) {
		ଯଦି ((ldev->flags & LOGDF_ENABLE) &&
		    log_passes_filters(ldev, rec)) {
			ଯଦି (!rec->ସନ୍ଦେଶ) {
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;

				len = vsnprintf(buf, ଆକାର(buf), fmt, args);
				rec->ସନ୍ଦେଶ = buf;
				gd->log_cont = len && buf[len - 1] != '\n';
			}
			ldev->drv->emit(ldev, rec);
		}
	}
	gd->processing_ସନ୍ଦେଶ = false;
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା _log(ଗଣନା log_category_t cat, ଗଣନା log_level_t level, ଅଚଳ ଅକ୍ଷର *file,
	 ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି, ଅଚଳ ଅକ୍ଷର *func, ଅଚଳ ଅକ୍ଷର *fmt, ...)
{
	ସଂରଚନା log_rec rec;
	va_list args;

	ଯଦି (!gd)
		ଫେରନ୍ତୁ -ENOSYS;

	/* Check for message continuation */
	ଯଦି (cat == LOGC_CONT)
		cat = gd->logc_prev;
	ଯଦି (level == LOGL_CONT)
		level = gd->logl_prev;

	rec.cat = cat;
	rec.level = level & LOGL_LEVEL_MASK;
	rec.flags = 0;
	ଯଦି (level & LOGL_FORCE_DEBUG)
		rec.flags |= LOGRECF_FORCE_DEBUG;
	ଯଦି (gd->log_cont)
		rec.flags |= LOGRECF_CONT;
	rec.file = file;
	rec.ଧାଡ଼ି = ଧାଡ଼ି;
	rec.func = func;
	rec.ସନ୍ଦେଶ = ଖାଲି;

	ଯଦି (!(gd->flags & GD_FLG_LOG_READY)) {
		gd->log_drop_ଗଣନା++;

		/* display dropped traces with console puts and DEBUG_UART */
		ଯଦି (rec.level <= CONFIG_LOG_DEFAULT_LEVEL ||
		    rec.flags & LOGRECF_FORCE_DEBUG) {
			ଅକ୍ଷର buf[CONFIG_SYS_CBSIZE];

			va_start(args, fmt);
			vsnprintf(buf, ଆକାର(buf), fmt, args);
			puts(buf);
			va_end(args);
		}

		ଫେରନ୍ତୁ -ENOSYS;
	}
	va_start(args, fmt);
	ଯଦି (!log_dispatch(&rec, fmt, args)) {
		gd->logc_prev = cat;
		gd->logl_prev = level;
	}
	va_end(args);

	ଫେରନ୍ତୁ 0;
}

#define MAX_ଧାଡ଼ି_LENGTH_BYTES		64
#define DEFAULT_ଧାଡ଼ି_LENGTH_BYTES	16

ପୂର୍ଣ୍ଣସଂଖ୍ୟା _log_buffer(ଗଣନା log_category_t cat, ଗଣନା log_level_t level,
		ଅଚଳ ଅକ୍ଷର *file, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି, ଅଚଳ ଅକ୍ଷର *func, ulong addr,
		ଅଚଳ ଶୂନ୍ୟ *data, uint width, uint ଗଣନା_2, uint linelen)
{
	ଯଦି (linelen * width > MAX_ଧାଡ଼ି_LENGTH_BYTES)
		linelen = MAX_ଧାଡ଼ି_LENGTH_BYTES / width;
	ଯଦି (linelen < 1)
		linelen = DEFAULT_ଧାଡ଼ି_LENGTH_BYTES / width;

	ଯେତେବେଳେ (ଗଣନା_2) {
		uint thislinelen;
		ଅକ୍ଷର buf[HEXDUMP_MAX_BUF_LENGTH(width * linelen)];

		thislinelen = hexdump_ଧାଡ଼ି(addr, data, width, ଗଣନା_2, linelen,
					   buf, ଆକାର(buf));
		ପ୍ରତିଜ୍ଞା(thislinelen >= 0);
		_log(cat, level, file, ଧାଡ଼ି, func, "%s\n", buf);

		/* update references */
		data += thislinelen * width;
		addr += thislinelen * width;
		ଗଣନା_2 -= thislinelen;
	}

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା log_add_filter_flags(ଅଚଳ ଅକ୍ଷର *drv_name, ଗଣନା log_category_t cat_list[],
			 ଗଣନା log_level_t level, ଅଚଳ ଅକ୍ଷର *file_list,
			 ଅଚଳ ଅକ୍ଷର *func_list, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
	ସଂରଚନା log_filter *filt;
	ସଂରଚନା log_device *ldev;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ldev = log_device_find_by_name(drv_name);
	ଯଦି (!ldev)
		ଫେରନ୍ତୁ -ENOENT;
	filt = calloc(1, ଆକାର(*filt));
	ଯଦି (!filt)
		ଫେରନ୍ତୁ -ENOMEM;

	filt->flags = flags;
	ଯଦି (cat_list) {
		filt->flags |= LOGFF_HAS_CAT;
		ପାଇଁ (i = 0; ; i++) {
			ଯଦି (i == ARRAY_SIZE(filt->cat_list)) {
				ret = -ENOSPC;
				ଯାଅ err;
			}
			filt->cat_list[i] = cat_list[i];
			ଯଦି (cat_list[i] == LOGC_END)
				ଭାଙ୍ଗ;
		}
	}
	filt->level = level;
	ଯଦି (file_list) {
		filt->file_list = strdup(file_list);
		ଯଦି (!filt->file_list) {
			ret = -ENOMEM;
			ଯାଅ err;
		}
	}
	ଯଦି (func_list) {
		filt->func_list = strdup(func_list);
		ଯଦି (!filt->func_list) {
			ret = -ENOMEM;
			ଯାଅ err;
		}
	}
	filt->filter_ଅଙ୍କ = ldev->next_filter_ଅଙ୍କ++;
	/* Add deny filters to the beginning of the list */
	ଯଦି (flags & LOGFF_DENY)
		list_add(&filt->sibling_ଗଣ୍ଠି, &ldev->filter_head);
	ଅନ୍ୟଥା
		list_add_tail(&filt->sibling_ଗଣ୍ଠି, &ldev->filter_head);

	ଫେରନ୍ତୁ filt->filter_ଅଙ୍କ;

err:
	ମୁକ୍ତ(filt);
	ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା log_remove_filter(ଅଚଳ ଅକ୍ଷର *drv_name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା filter_ଅଙ୍କ)
{
	ସଂରଚନା log_filter *filt;
	ସଂରଚନା log_device *ldev;

	ldev = log_device_find_by_name(drv_name);
	ଯଦି (!ldev)
		ଫେରନ୍ତୁ -ENOENT;

	list_for_each_entry(filt, &ldev->filter_head, sibling_ଗଣ୍ଠି) {
		ଯଦି (filt->filter_ଅଙ୍କ == filter_ଅଙ୍କ) {
			list_del(&filt->sibling_ଗଣ୍ଠି);
			ମୁକ୍ତ(filt);

			ଫେରନ୍ତୁ 0;
		}
	}

	ଫେରନ୍ତୁ -ENOENT;
}

/**
 * log_find_device_by_drv() - Find a device by its driver
 *
 * @drv: Log driver
 * Return: Device associated with that driver, or NULL if not found
 */
ସ୍ଥିର ସଂରଚନା log_device *log_find_device_by_drv(ସଂରଚନା log_driver *drv)
{
	ସଂରଚନା log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_ଗଣ୍ଠି) {
		ଯଦି (ldev->drv == drv)
			ଫେରନ୍ତୁ ldev;
	}
	/*
	 * It is quite hard to pass an invalid driver since passing an unknown
	 * LOG_GET_DRIVER(xxx) would normally produce a compilation error. But
	 * it is possible to pass NULL, for example, so this
	 */

	ଫେରନ୍ତୁ ଖାଲି;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା log_device_set_enable(ସଂରଚନା log_driver *drv, bool enable)
{
	ସଂରଚନା log_device *ldev;

	ldev = log_find_device_by_drv(drv);
	ଯଦି (!ldev)
		ଫେରନ୍ତୁ -ENOENT;
	ଯଦି (enable)
		ldev->flags |= LOGDF_ENABLE;
	ଅନ୍ୟଥା
		ldev->flags &= ~LOGDF_ENABLE;

	ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ log_fixup_for_gd_move(ସଂରଚନା global_data *new_gd)
{
	new_gd->log_head.prev->next = &new_gd->log_head;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା log_init(ଶୂନ୍ୟ)
{
	ସଂରଚନା log_driver *drv = ll_entry_start(ସଂରଚନା log_driver, log_driver);
	ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = ll_entry_ଗଣନା(ସଂରଚନା log_driver, log_driver);
	ସଂରଚନା log_driver *end = drv + ଗଣନା_2;

	/*
	 * We cannot add runtime data to the driver since it is likely stored
	 * in rodata. Instead, set up a 'device' corresponding to each driver.
	 * We only support having a single device for each driver.
	 */
	INIT_LIST_HEAD((ସଂରଚନା list_head *)&gd->log_head);
	ଯେତେବେଳେ (drv < end) {
		ସଂରଚନା log_device *ldev;

		ldev = calloc(1, ଆକାର(*ldev));
		ଯଦି (!ldev) {
			debug("%s: Cannot allocate memory\n", __func__);
			ଫେରନ୍ତୁ -ENOMEM;
		}
		INIT_LIST_HEAD(&ldev->filter_head);
		ldev->drv = drv;
		ldev->flags = drv->flags;
		list_add_tail(&ldev->sibling_ଗଣ୍ଠି,
			      (ସଂରଚନା list_head *)&gd->log_head);
		drv++;
	}
	gd->flags |= GD_FLG_LOG_READY;
	ଯଦି (!gd->default_log_level)
		gd->default_log_level = CONFIG_LOG_DEFAULT_LEVEL;
	gd->log_fmt = log_get_default_format();
	gd->logc_prev = LOGC_NONE;
	gd->logl_prev = LOGL_INFO;

	ଫେରନ୍ତୁ 0;
}
