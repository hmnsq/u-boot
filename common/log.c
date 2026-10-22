// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <display_options.h>
#include <log.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <dm/uclass.h>

DECLARE_GLOBAL_DATA_PTR;

static const var_na *const log_cat_name[] = {
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

_Static_assert(ARRAY_SIZE(log_cat_name) == LOGC_COUNT - LOGC_NONE,
	       "log_cat_name size");

static const var_na *const log_level_name[] = {
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

_Static_assert(ARRAY_SIZE(log_level_name) == LOGL_COUNT, "log_level_name size");

/* All error responses MUST begin with '<' */
const var_na *log_get_cat_name(ga_nanaa log_category_t cat)
{
	const var_na *name;

	အကယ်၍ (cat < 0 || cat >= LOGC_COUNT)
		ပြန်ပေး "<invalid>";
	အကယ်၍ (cat >= LOGC_NONE)
		ပြန်ပေး log_cat_name[cat - LOGC_NONE];

#အကယ်၍ CONFIG_IS_ENABLED(DM)
	name = uclass_get_name((ga_nanaa uclass_id)cat);
#မဟုတ်ပါက
	name = NULL;
#endif

	ပြန်ပေး name ? name : "<missing>";
}

ga_nanaa log_category_t log_get_cat_by_name(const var_na *name)
{
	ga_nanaa uclass_id id;
	ကိန်းပြည့် i;

	အတွက် (i = LOGC_NONE; i < LOGC_COUNT; i++)
		အကယ်၍ (!strcmp(name, log_cat_name[i - LOGC_NONE]))
			ပြန်ပေး i;
	id = uclass_get_by_name(name);
	အကယ်၍ (id != UCLASS_INVALID)
		ပြန်ပေး (ga_nanaa log_category_t)id;

	ပြန်ပေး LOGC_NONE;
}

const var_na *log_get_level_name(ga_nanaa log_level_t level)
{
	အကယ်၍ (level >= LOGL_COUNT)
		ပြန်ပေး "INVALID";
	ပြန်ပေး log_level_name[level];
}

ga_nanaa log_level_t log_get_level_by_name(const var_na *name)
{
	ကိန်းပြည့် i;

	အတွက် (i = 0; i < LOGL_COUNT; i++) {
		အကယ်၍ (!strcasecmp(log_level_name[i], name))
			ပြန်ပေး i;
	}

	ပြန်ပေး LOGL_NONE;
}

sa_nracanaa log_device *log_device_find_by_name(const var_na *drv_name)
{
	sa_nracanaa log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_node) {
		အကယ်၍ (!strcmp(drv_name, ldev->drv->name))
			ပြန်ပေး ldev;
	}

	ပြန်ပေး NULL;
}

bool log_has_cat(ga_nanaa log_category_t cat_list[], ga_nanaa log_category_t cat)
{
	ကိန်းပြည့် i;

	အတွက် (i = 0; i < LOGF_MAX_CATEGORIES && cat_list[i] != LOGC_END; i++) {
		အကယ်၍ (cat_list[i] == cat)
			ပြန်ပေး true;
	}

	ပြန်ပေး false;
}

/**
 * log_has_member() - check if a string is in a comma separated list
 *
 * @list:	Comma separated list of strings
 * @member:	String to find
 *
 * Return: ``true`` if @member is in @list, else ``false``
 */
static bool log_has_member(const var_na *list, const var_na *member)
{
	ကိန်းပြည့် member_len = strlen(member);
	const var_na *s, *p;
	ကိန်းပြည့် substr_len;

	အတွက် (s = list; *s; s = p + (*p != '\0')) {
		p = strchrnul(s, ',');
		substr_len = p - s;
		အကယ်၍ (member_len >= substr_len &&
		    !strncmp(member + member_len - substr_len, s, substr_len))
			ပြန်ပေး true;
	}

	ပြန်ပေး false;
}

/**
 * log_passes_filters() - check if a log record passes the filters for a device
 *
 * @ldev: Log device to check
 * @rec: Log record to check
 * Return: true if @rec is not blocked by the filters in @ldev, false if it is
 */
static bool log_passes_filters(sa_nracanaa log_device *ldev, sa_nracanaa log_rec *rec)
{
	sa_nracanaa log_filter *filt;

	အကယ်၍ (rec->flags & LOGRECF_FORCE_DEBUG)
		ပြန်ပေး true;

	/* If there are no filters, filter on the default log level */
	အကယ်၍ (list_empty(&ldev->filter_head)) {
		အကယ်၍ (rec->level > gd->default_log_level)
			ပြန်ပေး false;
		ပြန်ပေး true;
	}

	list_for_each_entry(filt, &ldev->filter_head, sibling_node) {
		အကယ်၍ (filt->flags & LOGFF_LEVEL_MIN) {
			အကယ်၍ (rec->level < filt->level)
				continue;
		} မဟုတ်ပါက အကယ်၍ (rec->level > filt->level) {
			continue;
		}

		အကယ်၍ ((filt->flags & LOGFF_HAS_CAT) &&
		    !log_has_cat(filt->cat_list, rec->cat))
			continue;

		အကယ်၍ (filt->file_list &&
		    !log_has_member(filt->file_list, rec->file))
			continue;

		အကယ်၍ (filt->func_list &&
		    !log_has_member(filt->func_list, rec->func))
			continue;

		အကယ်၍ (filt->flags & LOGFF_DENY)
			ပြန်ပေး false;
		မဟုတ်ပါက
			ပြန်ပေး true;
	}

	ပြန်ပေး false;
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
static ကိန်းပြည့် log_dispatch(sa_nracanaa log_rec *rec, const var_na *fmt, va_list args)
{
	sa_nracanaa log_device *ldev;
	var_na buf[CONFIG_SYS_CBSIZE];

	/*
	 * When a log driver writes messages (e.g. via the network stack) this
	 * may result in further generated messages. We cannot process them here
	 * as this might result in infinite recursion.
	 */
	အကယ်၍ (gd->processing_msg)
		ပြန်ပေး 1;

	/* Emit message */
	gd->processing_msg = true;
	list_for_each_entry(ldev, &gd->log_head, sibling_node) {
		အကယ်၍ ((ldev->flags & LOGDF_ENABLE) &&
		    log_passes_filters(ldev, rec)) {
			အကယ်၍ (!rec->msg) {
				ကိန်းပြည့် len;

				len = vsnprintf(buf, parimaa_na(buf), fmt, args);
				rec->msg = buf;
				gd->log_cont = len && buf[len - 1] != '\n';
			}
			ldev->drv->emit(ldev, rec);
		}
	}
	gd->processing_msg = false;
	ပြန်ပေး 0;
}

ကိန်းပြည့် _log(ga_nanaa log_category_t cat, ga_nanaa log_level_t level, const var_na *file,
	 ကိန်းပြည့် line, const var_na *func, const var_na *fmt, ...)
{
	sa_nracanaa log_rec rec;
	va_list args;

	အကယ်၍ (!gd)
		ပြန်ပေး -ENOSYS;

	/* Check for message continuation */
	အကယ်၍ (cat == LOGC_CONT)
		cat = gd->logc_prev;
	အကယ်၍ (level == LOGL_CONT)
		level = gd->logl_prev;

	rec.cat = cat;
	rec.level = level & LOGL_LEVEL_MASK;
	rec.flags = 0;
	အကယ်၍ (level & LOGL_FORCE_DEBUG)
		rec.flags |= LOGRECF_FORCE_DEBUG;
	အကယ်၍ (gd->log_cont)
		rec.flags |= LOGRECF_CONT;
	rec.file = file;
	rec.line = line;
	rec.func = func;
	rec.msg = NULL;

	အကယ်၍ (!(gd->flags & GD_FLG_LOG_READY)) {
		gd->log_drop_count++;

		/* display dropped traces with console puts and DEBUG_UART */
		အကယ်၍ (rec.level <= CONFIG_LOG_DEFAULT_LEVEL ||
		    rec.flags & LOGRECF_FORCE_DEBUG) {
			var_na buf[CONFIG_SYS_CBSIZE];

			va_start(args, fmt);
			vsnprintf(buf, parimaa_na(buf), fmt, args);
			puts(buf);
			va_end(args);
		}

		ပြန်ပေး -ENOSYS;
	}
	va_start(args, fmt);
	အကယ်၍ (!log_dispatch(&rec, fmt, args)) {
		gd->logc_prev = cat;
		gd->logl_prev = level;
	}
	va_end(args);

	ပြန်ပေး 0;
}

#define MAX_LINE_LENGTH_BYTES		64
#define DEFAULT_LINE_LENGTH_BYTES	16

ကိန်းပြည့် _log_buffer(ga_nanaa log_category_t cat, ga_nanaa log_level_t level,
		const var_na *file, ကိန်းပြည့် line, const var_na *func, ulong addr,
		const void *data, uint width, uint count, uint linelen)
{
	အကယ်၍ (linelen * width > MAX_LINE_LENGTH_BYTES)
		linelen = MAX_LINE_LENGTH_BYTES / width;
	အကယ်၍ (linelen < 1)
		linelen = DEFAULT_LINE_LENGTH_BYTES / width;

	နေစဉ် (count) {
		uint thislinelen;
		var_na buf[HEXDUMP_MAX_BUF_LENGTH(width * linelen)];

		thislinelen = hexdump_line(addr, data, width, count, linelen,
					   buf, parimaa_na(buf));
		assert(thislinelen >= 0);
		_log(cat, level, file, line, func, "%s\n", buf);

		/* update references */
		data += thislinelen * width;
		addr += thislinelen * width;
		count -= thislinelen;
	}

	ပြန်ပေး 0;
}

ကိန်းပြည့် log_add_filter_flags(const var_na *drv_name, ga_nanaa log_category_t cat_list[],
			 ga_nanaa log_level_t level, const var_na *file_list,
			 const var_na *func_list, ကိန်းပြည့် flags)
{
	sa_nracanaa log_filter *filt;
	sa_nracanaa log_device *ldev;
	ကိန်းပြည့် ret;
	ကိန်းပြည့် i;

	ldev = log_device_find_by_name(drv_name);
	အကယ်၍ (!ldev)
		ပြန်ပေး -ENOENT;
	filt = calloc(1, parimaa_na(*filt));
	အကယ်၍ (!filt)
		ပြန်ပေး -ENOMEM;

	filt->flags = flags;
	အကယ်၍ (cat_list) {
		filt->flags |= LOGFF_HAS_CAT;
		အတွက် (i = 0; ; i++) {
			အကယ်၍ (i == ARRAY_SIZE(filt->cat_list)) {
				ret = -ENOSPC;
				goto err;
			}
			filt->cat_list[i] = cat_list[i];
			အကယ်၍ (cat_list[i] == LOGC_END)
				bha_nga;
		}
	}
	filt->level = level;
	အကယ်၍ (file_list) {
		filt->file_list = strdup(file_list);
		အကယ်၍ (!filt->file_list) {
			ret = -ENOMEM;
			goto err;
		}
	}
	အကယ်၍ (func_list) {
		filt->func_list = strdup(func_list);
		အကယ်၍ (!filt->func_list) {
			ret = -ENOMEM;
			goto err;
		}
	}
	filt->filter_num = ldev->next_filter_num++;
	/* Add deny filters to the beginning of the list */
	အကယ်၍ (flags & LOGFF_DENY)
		list_add(&filt->sibling_node, &ldev->filter_head);
	မဟုတ်ပါက
		list_add_tail(&filt->sibling_node, &ldev->filter_head);

	ပြန်ပေး filt->filter_num;

err:
	free(filt);
	ပြန်ပေး ret;
}

ကိန်းပြည့် log_remove_filter(const var_na *drv_name, ကိန်းပြည့် filter_num)
{
	sa_nracanaa log_filter *filt;
	sa_nracanaa log_device *ldev;

	ldev = log_device_find_by_name(drv_name);
	အကယ်၍ (!ldev)
		ပြန်ပေး -ENOENT;

	list_for_each_entry(filt, &ldev->filter_head, sibling_node) {
		အကယ်၍ (filt->filter_num == filter_num) {
			list_del(&filt->sibling_node);
			free(filt);

			ပြန်ပေး 0;
		}
	}

	ပြန်ပေး -ENOENT;
}

/**
 * log_find_device_by_drv() - Find a device by its driver
 *
 * @drv: Log driver
 * Return: Device associated with that driver, or NULL if not found
 */
static sa_nracanaa log_device *log_find_device_by_drv(sa_nracanaa log_driver *drv)
{
	sa_nracanaa log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_node) {
		အကယ်၍ (ldev->drv == drv)
			ပြန်ပေး ldev;
	}
	/*
	 * It is quite hard to pass an invalid driver since passing an unknown
	 * LOG_GET_DRIVER(xxx) would normally produce a compilation error. But
	 * it is possible to pass NULL, for example, so this
	 */

	ပြန်ပေး NULL;
}

ကိန်းပြည့် log_device_set_enable(sa_nracanaa log_driver *drv, bool enable)
{
	sa_nracanaa log_device *ldev;

	ldev = log_find_device_by_drv(drv);
	အကယ်၍ (!ldev)
		ပြန်ပေး -ENOENT;
	အကယ်၍ (enable)
		ldev->flags |= LOGDF_ENABLE;
	မဟုတ်ပါက
		ldev->flags &= ~LOGDF_ENABLE;

	ပြန်ပေး 0;
}

void log_fixup_for_gd_move(sa_nracanaa global_data *new_gd)
{
	new_gd->log_head.prev->next = &new_gd->log_head;
}

ကိန်းပြည့် log_init(void)
{
	sa_nracanaa log_driver *drv = ll_entry_start(sa_nracanaa log_driver, log_driver);
	const ကိန်းပြည့် count = ll_entry_count(sa_nracanaa log_driver, log_driver);
	sa_nracanaa log_driver *end = drv + count;

	/*
	 * We cannot add runtime data to the driver since it is likely stored
	 * in rodata. Instead, set up a 'device' corresponding to each driver.
	 * We only support having a single device for each driver.
	 */
	INIT_LIST_HEAD((sa_nracanaa list_head *)&gd->log_head);
	နေစဉ် (drv < end) {
		sa_nracanaa log_device *ldev;

		ldev = calloc(1, parimaa_na(*ldev));
		အကယ်၍ (!ldev) {
			debug("%s: Cannot allocate memory\n", __func__);
			ပြန်ပေး -ENOMEM;
		}
		INIT_LIST_HEAD(&ldev->filter_head);
		ldev->drv = drv;
		ldev->flags = drv->flags;
		list_add_tail(&ldev->sibling_node,
			      (sa_nracanaa list_head *)&gd->log_head);
		drv++;
	}
	gd->flags |= GD_FLG_LOG_READY;
	အကယ်၍ (!gd->default_log_level)
		gd->default_log_level = CONFIG_LOG_DEFAULT_LEVEL;
	gd->log_fmt = log_get_default_format();
	gd->logc_prev = LOGC_NONE;
	gd->logl_prev = LOGL_INFO;

	ပြန်ပေး 0;
}
