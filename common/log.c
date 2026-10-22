// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#અન્તર્ભાવ <display_options.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <malloc.h>
#અન્તર્ભાવ <asm/global_data.h>
#અન્તર્ભાવ <dm/uclass.h>

DECLARE_GLOBAL_DATA_PTR;

સ્થિર અચલ અક્ષર *અચલ log_cat_name[] = {
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

_Static_assert(ARRAY_SIZE(log_cat_name) == LOGC_ગણતરી - LOGC_NONE,
	       "log_cat_name size");

સ્થિર અચલ અક્ષર *અચલ log_level_name[] = {
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

_Static_assert(ARRAY_SIZE(log_level_name) == LOGL_ગણતરી, "log_level_name size");

/* All error responses MUST begin with '<' */
અચલ અક્ષર *log_get_cat_name(ગણના log_category_t cat)
{
	અચલ અક્ષર *name;

	જો (cat < 0 || cat >= LOGC_ગણતરી)
		પાછા "<invalid>";
	જો (cat >= LOGC_NONE)
		પાછા log_cat_name[cat - LOGC_NONE];

#જો CONFIG_IS_ENABLED(DM)
	name = uclass_get_name((ગણના uclass_id)cat);
#નહીંતર
	name = ખાલી;
#endif

	પાછા name ? name : "<missing>";
}

ગણના log_category_t log_get_cat_by_name(અચલ અક્ષર *name)
{
	ગણના uclass_id id;
	પૂર્ણાંક i;

	માટે (i = LOGC_NONE; i < LOGC_ગણતરી; i++)
		જો (!strcmp(name, log_cat_name[i - LOGC_NONE]))
			પાછા i;
	id = uclass_get_by_name(name);
	જો (id != UCLASS_INVALID)
		પાછા (ગણના log_category_t)id;

	પાછા LOGC_NONE;
}

અચલ અક્ષર *log_get_level_name(ગણના log_level_t level)
{
	જો (level >= LOGL_ગણતરી)
		પાછા "INVALID";
	પાછા log_level_name[level];
}

ગણના log_level_t log_get_level_by_name(અચલ અક્ષર *name)
{
	પૂર્ણાંક i;

	માટે (i = 0; i < LOGL_ગણતરી; i++) {
		જો (!strcasecmp(log_level_name[i], name))
			પાછા i;
	}

	પાછા LOGL_NONE;
}

સંરચના log_device *log_device_find_by_name(અચલ અક્ષર *drv_name)
{
	સંરચના log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_ગાંઠ) {
		જો (!strcmp(drv_name, ldev->drv->name))
			પાછા ldev;
	}

	પાછા ખાલી;
}

bool log_has_cat(ગણના log_category_t cat_list[], ગણના log_category_t cat)
{
	પૂર્ણાંક i;

	માટે (i = 0; i < LOGF_MAX_CATEGORIES && cat_list[i] != LOGC_END; i++) {
		જો (cat_list[i] == cat)
			પાછા true;
	}

	પાછા false;
}

/**
 * log_has_member() - check if a string is in a comma separated list
 *
 * @list:	Comma separated list of strings
 * @member:	String to find
 *
 * Return: ``true`` if @member is in @list, else ``false``
 */
સ્થિર bool log_has_member(અચલ અક્ષર *list, અચલ અક્ષર *member)
{
	પૂર્ણાંક member_len = strlen(member);
	અચલ અક્ષર *s, *p;
	પૂર્ણાંક substr_len;

	માટે (s = list; *s; s = p + (*p != '\0')) {
		p = strchrnul(s, ',');
		substr_len = p - s;
		જો (member_len >= substr_len &&
		    !strncmp(member + member_len - substr_len, s, substr_len))
			પાછા true;
	}

	પાછા false;
}

/**
 * log_passes_filters() - check if a log record passes the filters for a device
 *
 * @ldev: Log device to check
 * @rec: Log record to check
 * Return: true if @rec is not blocked by the filters in @ldev, false if it is
 */
સ્થિર bool log_passes_filters(સંરચના log_device *ldev, સંરચના log_rec *rec)
{
	સંરચના log_filter *filt;

	જો (rec->flags & LOGRECF_FORCE_DEBUG)
		પાછા true;

	/* If there are no filters, filter on the default log level */
	જો (list_empty(&ldev->filter_head)) {
		જો (rec->level > gd->default_log_level)
			પાછા false;
		પાછા true;
	}

	list_for_each_entry(filt, &ldev->filter_head, sibling_ગાંઠ) {
		જો (filt->flags & LOGFF_LEVEL_MIN) {
			જો (rec->level < filt->level)
				ચાલુ;
		} નહીંતર જો (rec->level > filt->level) {
			ચાલુ;
		}

		જો ((filt->flags & LOGFF_HAS_CAT) &&
		    !log_has_cat(filt->cat_list, rec->cat))
			ચાલુ;

		જો (filt->file_list &&
		    !log_has_member(filt->file_list, rec->file))
			ચાલુ;

		જો (filt->func_list &&
		    !log_has_member(filt->func_list, rec->func))
			ચાલુ;

		જો (filt->flags & LOGFF_DENY)
			પાછા false;
		નહીંતર
			પાછા true;
	}

	પાછા false;
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
સ્થિર પૂર્ણાંક log_dispatch(સંરચના log_rec *rec, અચલ અક્ષર *fmt, va_list args)
{
	સંરચના log_device *ldev;
	અક્ષર buf[CONFIG_SYS_CBSIZE];

	/*
	 * When a log driver writes messages (e.g. via the network stack) this
	 * may result in further generated messages. We cannot process them here
	 * as this might result in infinite recursion.
	 */
	જો (gd->processing_સંદેશ)
		પાછા 1;

	/* Emit message */
	gd->processing_સંદેશ = true;
	list_for_each_entry(ldev, &gd->log_head, sibling_ગાંઠ) {
		જો ((ldev->flags & LOGDF_ENABLE) &&
		    log_passes_filters(ldev, rec)) {
			જો (!rec->સંદેશ) {
				પૂર્ણાંક len;

				len = vsnprintf(buf, કદ(buf), fmt, args);
				rec->સંદેશ = buf;
				gd->log_cont = len && buf[len - 1] != '\n';
			}
			ldev->drv->emit(ldev, rec);
		}
	}
	gd->processing_સંદેશ = false;
	પાછા 0;
}

પૂર્ણાંક _log(ગણના log_category_t cat, ગણના log_level_t level, અચલ અક્ષર *file,
	 પૂર્ણાંક લીટી, અચલ અક્ષર *func, અચલ અક્ષર *fmt, ...)
{
	સંરચના log_rec rec;
	va_list args;

	જો (!gd)
		પાછા -ENOSYS;

	/* Check for message continuation */
	જો (cat == LOGC_CONT)
		cat = gd->logc_prev;
	જો (level == LOGL_CONT)
		level = gd->logl_prev;

	rec.cat = cat;
	rec.level = level & LOGL_LEVEL_MASK;
	rec.flags = 0;
	જો (level & LOGL_FORCE_DEBUG)
		rec.flags |= LOGRECF_FORCE_DEBUG;
	જો (gd->log_cont)
		rec.flags |= LOGRECF_CONT;
	rec.file = file;
	rec.લીટી = લીટી;
	rec.func = func;
	rec.સંદેશ = ખાલી;

	જો (!(gd->flags & GD_FLG_LOG_READY)) {
		gd->log_drop_ગણતરી++;

		/* display dropped traces with console puts and DEBUG_UART */
		જો (rec.level <= CONFIG_LOG_DEFAULT_LEVEL ||
		    rec.flags & LOGRECF_FORCE_DEBUG) {
			અક્ષર buf[CONFIG_SYS_CBSIZE];

			va_start(args, fmt);
			vsnprintf(buf, કદ(buf), fmt, args);
			puts(buf);
			va_end(args);
		}

		પાછા -ENOSYS;
	}
	va_start(args, fmt);
	જો (!log_dispatch(&rec, fmt, args)) {
		gd->logc_prev = cat;
		gd->logl_prev = level;
	}
	va_end(args);

	પાછા 0;
}

#define MAX_લીટી_LENGTH_BYTES		64
#define DEFAULT_લીટી_LENGTH_BYTES	16

પૂર્ણાંક _log_buffer(ગણના log_category_t cat, ગણના log_level_t level,
		અચલ અક્ષર *file, પૂર્ણાંક લીટી, અચલ અક્ષર *func, ulong addr,
		અચલ શૂન્ય *data, uint width, uint ગણતરી, uint linelen)
{
	જો (linelen * width > MAX_લીટી_LENGTH_BYTES)
		linelen = MAX_લીટી_LENGTH_BYTES / width;
	જો (linelen < 1)
		linelen = DEFAULT_લીટી_LENGTH_BYTES / width;

	જ્યારે (ગણતરી) {
		uint thislinelen;
		અક્ષર buf[HEXDUMP_MAX_BUF_LENGTH(width * linelen)];

		thislinelen = hexdump_લીટી(addr, data, width, ગણતરી, linelen,
					   buf, કદ(buf));
		પ્રતિજ્ઞા(thislinelen >= 0);
		_log(cat, level, file, લીટી, func, "%s\n", buf);

		/* update references */
		data += thislinelen * width;
		addr += thislinelen * width;
		ગણતરી -= thislinelen;
	}

	પાછા 0;
}

પૂર્ણાંક log_add_filter_flags(અચલ અક્ષર *drv_name, ગણના log_category_t cat_list[],
			 ગણના log_level_t level, અચલ અક્ષર *file_list,
			 અચલ અક્ષર *func_list, પૂર્ણાંક flags)
{
	સંરચના log_filter *filt;
	સંરચના log_device *ldev;
	પૂર્ણાંક ret;
	પૂર્ણાંક i;

	ldev = log_device_find_by_name(drv_name);
	જો (!ldev)
		પાછા -ENOENT;
	filt = calloc(1, કદ(*filt));
	જો (!filt)
		પાછા -ENOMEM;

	filt->flags = flags;
	જો (cat_list) {
		filt->flags |= LOGFF_HAS_CAT;
		માટે (i = 0; ; i++) {
			જો (i == ARRAY_SIZE(filt->cat_list)) {
				ret = -ENOSPC;
				જાઓ err;
			}
			filt->cat_list[i] = cat_list[i];
			જો (cat_list[i] == LOGC_END)
				તોડો;
		}
	}
	filt->level = level;
	જો (file_list) {
		filt->file_list = strdup(file_list);
		જો (!filt->file_list) {
			ret = -ENOMEM;
			જાઓ err;
		}
	}
	જો (func_list) {
		filt->func_list = strdup(func_list);
		જો (!filt->func_list) {
			ret = -ENOMEM;
			જાઓ err;
		}
	}
	filt->filter_અંક = ldev->next_filter_અંક++;
	/* Add deny filters to the beginning of the list */
	જો (flags & LOGFF_DENY)
		list_add(&filt->sibling_ગાંઠ, &ldev->filter_head);
	નહીંતર
		list_add_tail(&filt->sibling_ગાંઠ, &ldev->filter_head);

	પાછા filt->filter_અંક;

err:
	મુક્ત(filt);
	પાછા ret;
}

પૂર્ણાંક log_remove_filter(અચલ અક્ષર *drv_name, પૂર્ણાંક filter_અંક)
{
	સંરચના log_filter *filt;
	સંરચના log_device *ldev;

	ldev = log_device_find_by_name(drv_name);
	જો (!ldev)
		પાછા -ENOENT;

	list_for_each_entry(filt, &ldev->filter_head, sibling_ગાંઠ) {
		જો (filt->filter_અંક == filter_અંક) {
			list_del(&filt->sibling_ગાંઠ);
			મુક્ત(filt);

			પાછા 0;
		}
	}

	પાછા -ENOENT;
}

/**
 * log_find_device_by_drv() - Find a device by its driver
 *
 * @drv: Log driver
 * Return: Device associated with that driver, or NULL if not found
 */
સ્થિર સંરચના log_device *log_find_device_by_drv(સંરચના log_driver *drv)
{
	સંરચના log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_ગાંઠ) {
		જો (ldev->drv == drv)
			પાછા ldev;
	}
	/*
	 * It is quite hard to pass an invalid driver since passing an unknown
	 * LOG_GET_DRIVER(xxx) would normally produce a compilation error. But
	 * it is possible to pass NULL, for example, so this
	 */

	પાછા ખાલી;
}

પૂર્ણાંક log_device_set_enable(સંરચના log_driver *drv, bool enable)
{
	સંરચના log_device *ldev;

	ldev = log_find_device_by_drv(drv);
	જો (!ldev)
		પાછા -ENOENT;
	જો (enable)
		ldev->flags |= LOGDF_ENABLE;
	નહીંતર
		ldev->flags &= ~LOGDF_ENABLE;

	પાછા 0;
}

શૂન્ય log_fixup_for_gd_move(સંરચના global_data *new_gd)
{
	new_gd->log_head.prev->next = &new_gd->log_head;
}

પૂર્ણાંક log_init(શૂન્ય)
{
	સંરચના log_driver *drv = ll_entry_start(સંરચના log_driver, log_driver);
	અચલ પૂર્ણાંક ગણતરી = ll_entry_ગણતરી(સંરચના log_driver, log_driver);
	સંરચના log_driver *end = drv + ગણતરી;

	/*
	 * We cannot add runtime data to the driver since it is likely stored
	 * in rodata. Instead, set up a 'device' corresponding to each driver.
	 * We only support having a single device for each driver.
	 */
	INIT_LIST_HEAD((સંરચના list_head *)&gd->log_head);
	જ્યારે (drv < end) {
		સંરચના log_device *ldev;

		ldev = calloc(1, કદ(*ldev));
		જો (!ldev) {
			debug("%s: Cannot allocate memory\n", __func__);
			પાછા -ENOMEM;
		}
		INIT_LIST_HEAD(&ldev->filter_head);
		ldev->drv = drv;
		ldev->flags = drv->flags;
		list_add_tail(&ldev->sibling_ગાંઠ,
			      (સંરચના list_head *)&gd->log_head);
		drv++;
	}
	gd->flags |= GD_FLG_LOG_READY;
	જો (!gd->default_log_level)
		gd->default_log_level = CONFIG_LOG_DEFAULT_LEVEL;
	gd->log_fmt = log_get_default_format();
	gd->logc_prev = LOGC_NONE;
	gd->logl_prev = LOGL_INFO;

	પાછા 0;
}
