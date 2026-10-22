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

	ถ้า (cat < 0 || cat >= LOGC_COUNT)
		คืน "<invalid>";
	ถ้า (cat >= LOGC_NONE)
		คืน log_cat_name[cat - LOGC_NONE];

#ถ้า CONFIG_IS_ENABLED(DM)
	name = uclass_get_name((ga_nanaa uclass_id)cat);
#มิฉะนั้น
	name = NULL;
#endif

	คืน name ? name : "<missing>";
}

ga_nanaa log_category_t log_get_cat_by_name(const var_na *name)
{
	ga_nanaa uclass_id id;
	จำนวนเต็ม i;

	สำหรับ (i = LOGC_NONE; i < LOGC_COUNT; i++)
		ถ้า (!strcmp(name, log_cat_name[i - LOGC_NONE]))
			คืน i;
	id = uclass_get_by_name(name);
	ถ้า (id != UCLASS_INVALID)
		คืน (ga_nanaa log_category_t)id;

	คืน LOGC_NONE;
}

const var_na *log_get_level_name(ga_nanaa log_level_t level)
{
	ถ้า (level >= LOGL_COUNT)
		คืน "INVALID";
	คืน log_level_name[level];
}

ga_nanaa log_level_t log_get_level_by_name(const var_na *name)
{
	จำนวนเต็ม i;

	สำหรับ (i = 0; i < LOGL_COUNT; i++) {
		ถ้า (!strcasecmp(log_level_name[i], name))
			คืน i;
	}

	คืน LOGL_NONE;
}

sa_nracanaa log_device *log_device_find_by_name(const var_na *drv_name)
{
	sa_nracanaa log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_node) {
		ถ้า (!strcmp(drv_name, ldev->drv->name))
			คืน ldev;
	}

	คืน NULL;
}

bool log_has_cat(ga_nanaa log_category_t cat_list[], ga_nanaa log_category_t cat)
{
	จำนวนเต็ม i;

	สำหรับ (i = 0; i < LOGF_MAX_CATEGORIES && cat_list[i] != LOGC_END; i++) {
		ถ้า (cat_list[i] == cat)
			คืน true;
	}

	คืน false;
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
	จำนวนเต็ม member_len = strlen(member);
	const var_na *s, *p;
	จำนวนเต็ม substr_len;

	สำหรับ (s = list; *s; s = p + (*p != '\0')) {
		p = strchrnul(s, ',');
		substr_len = p - s;
		ถ้า (member_len >= substr_len &&
		    !strncmp(member + member_len - substr_len, s, substr_len))
			คืน true;
	}

	คืน false;
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

	ถ้า (rec->flags & LOGRECF_FORCE_DEBUG)
		คืน true;

	/* If there are no filters, filter on the default log level */
	ถ้า (list_empty(&ldev->filter_head)) {
		ถ้า (rec->level > gd->default_log_level)
			คืน false;
		คืน true;
	}

	list_for_each_entry(filt, &ldev->filter_head, sibling_node) {
		ถ้า (filt->flags & LOGFF_LEVEL_MIN) {
			ถ้า (rec->level < filt->level)
				continue;
		} มิฉะนั้น ถ้า (rec->level > filt->level) {
			continue;
		}

		ถ้า ((filt->flags & LOGFF_HAS_CAT) &&
		    !log_has_cat(filt->cat_list, rec->cat))
			continue;

		ถ้า (filt->file_list &&
		    !log_has_member(filt->file_list, rec->file))
			continue;

		ถ้า (filt->func_list &&
		    !log_has_member(filt->func_list, rec->func))
			continue;

		ถ้า (filt->flags & LOGFF_DENY)
			คืน false;
		มิฉะนั้น
			คืน true;
	}

	คืน false;
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
static จำนวนเต็ม log_dispatch(sa_nracanaa log_rec *rec, const var_na *fmt, va_list args)
{
	sa_nracanaa log_device *ldev;
	var_na buf[CONFIG_SYS_CBSIZE];

	/*
	 * When a log driver writes messages (e.g. via the network stack) this
	 * may result in further generated messages. We cannot process them here
	 * as this might result in infinite recursion.
	 */
	ถ้า (gd->processing_msg)
		คืน 1;

	/* Emit message */
	gd->processing_msg = true;
	list_for_each_entry(ldev, &gd->log_head, sibling_node) {
		ถ้า ((ldev->flags & LOGDF_ENABLE) &&
		    log_passes_filters(ldev, rec)) {
			ถ้า (!rec->msg) {
				จำนวนเต็ม len;

				len = vsnprintf(buf, parimaa_na(buf), fmt, args);
				rec->msg = buf;
				gd->log_cont = len && buf[len - 1] != '\n';
			}
			ldev->drv->emit(ldev, rec);
		}
	}
	gd->processing_msg = false;
	คืน 0;
}

จำนวนเต็ม _log(ga_nanaa log_category_t cat, ga_nanaa log_level_t level, const var_na *file,
	 จำนวนเต็ม line, const var_na *func, const var_na *fmt, ...)
{
	sa_nracanaa log_rec rec;
	va_list args;

	ถ้า (!gd)
		คืน -ENOSYS;

	/* Check for message continuation */
	ถ้า (cat == LOGC_CONT)
		cat = gd->logc_prev;
	ถ้า (level == LOGL_CONT)
		level = gd->logl_prev;

	rec.cat = cat;
	rec.level = level & LOGL_LEVEL_MASK;
	rec.flags = 0;
	ถ้า (level & LOGL_FORCE_DEBUG)
		rec.flags |= LOGRECF_FORCE_DEBUG;
	ถ้า (gd->log_cont)
		rec.flags |= LOGRECF_CONT;
	rec.file = file;
	rec.line = line;
	rec.func = func;
	rec.msg = NULL;

	ถ้า (!(gd->flags & GD_FLG_LOG_READY)) {
		gd->log_drop_count++;

		/* display dropped traces with console puts and DEBUG_UART */
		ถ้า (rec.level <= CONFIG_LOG_DEFAULT_LEVEL ||
		    rec.flags & LOGRECF_FORCE_DEBUG) {
			var_na buf[CONFIG_SYS_CBSIZE];

			va_start(args, fmt);
			vsnprintf(buf, parimaa_na(buf), fmt, args);
			puts(buf);
			va_end(args);
		}

		คืน -ENOSYS;
	}
	va_start(args, fmt);
	ถ้า (!log_dispatch(&rec, fmt, args)) {
		gd->logc_prev = cat;
		gd->logl_prev = level;
	}
	va_end(args);

	คืน 0;
}

#define MAX_LINE_LENGTH_BYTES		64
#define DEFAULT_LINE_LENGTH_BYTES	16

จำนวนเต็ม _log_buffer(ga_nanaa log_category_t cat, ga_nanaa log_level_t level,
		const var_na *file, จำนวนเต็ม line, const var_na *func, ulong addr,
		const void *data, uint width, uint count, uint linelen)
{
	ถ้า (linelen * width > MAX_LINE_LENGTH_BYTES)
		linelen = MAX_LINE_LENGTH_BYTES / width;
	ถ้า (linelen < 1)
		linelen = DEFAULT_LINE_LENGTH_BYTES / width;

	ขณะ (count) {
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

	คืน 0;
}

จำนวนเต็ม log_add_filter_flags(const var_na *drv_name, ga_nanaa log_category_t cat_list[],
			 ga_nanaa log_level_t level, const var_na *file_list,
			 const var_na *func_list, จำนวนเต็ม flags)
{
	sa_nracanaa log_filter *filt;
	sa_nracanaa log_device *ldev;
	จำนวนเต็ม ret;
	จำนวนเต็ม i;

	ldev = log_device_find_by_name(drv_name);
	ถ้า (!ldev)
		คืน -ENOENT;
	filt = calloc(1, parimaa_na(*filt));
	ถ้า (!filt)
		คืน -ENOMEM;

	filt->flags = flags;
	ถ้า (cat_list) {
		filt->flags |= LOGFF_HAS_CAT;
		สำหรับ (i = 0; ; i++) {
			ถ้า (i == ARRAY_SIZE(filt->cat_list)) {
				ret = -ENOSPC;
				goto err;
			}
			filt->cat_list[i] = cat_list[i];
			ถ้า (cat_list[i] == LOGC_END)
				bha_nga;
		}
	}
	filt->level = level;
	ถ้า (file_list) {
		filt->file_list = strdup(file_list);
		ถ้า (!filt->file_list) {
			ret = -ENOMEM;
			goto err;
		}
	}
	ถ้า (func_list) {
		filt->func_list = strdup(func_list);
		ถ้า (!filt->func_list) {
			ret = -ENOMEM;
			goto err;
		}
	}
	filt->filter_num = ldev->next_filter_num++;
	/* Add deny filters to the beginning of the list */
	ถ้า (flags & LOGFF_DENY)
		list_add(&filt->sibling_node, &ldev->filter_head);
	มิฉะนั้น
		list_add_tail(&filt->sibling_node, &ldev->filter_head);

	คืน filt->filter_num;

err:
	free(filt);
	คืน ret;
}

จำนวนเต็ม log_remove_filter(const var_na *drv_name, จำนวนเต็ม filter_num)
{
	sa_nracanaa log_filter *filt;
	sa_nracanaa log_device *ldev;

	ldev = log_device_find_by_name(drv_name);
	ถ้า (!ldev)
		คืน -ENOENT;

	list_for_each_entry(filt, &ldev->filter_head, sibling_node) {
		ถ้า (filt->filter_num == filter_num) {
			list_del(&filt->sibling_node);
			free(filt);

			คืน 0;
		}
	}

	คืน -ENOENT;
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
		ถ้า (ldev->drv == drv)
			คืน ldev;
	}
	/*
	 * It is quite hard to pass an invalid driver since passing an unknown
	 * LOG_GET_DRIVER(xxx) would normally produce a compilation error. But
	 * it is possible to pass NULL, for example, so this
	 */

	คืน NULL;
}

จำนวนเต็ม log_device_set_enable(sa_nracanaa log_driver *drv, bool enable)
{
	sa_nracanaa log_device *ldev;

	ldev = log_find_device_by_drv(drv);
	ถ้า (!ldev)
		คืน -ENOENT;
	ถ้า (enable)
		ldev->flags |= LOGDF_ENABLE;
	มิฉะนั้น
		ldev->flags &= ~LOGDF_ENABLE;

	คืน 0;
}

void log_fixup_for_gd_move(sa_nracanaa global_data *new_gd)
{
	new_gd->log_head.prev->next = &new_gd->log_head;
}

จำนวนเต็ม log_init(void)
{
	sa_nracanaa log_driver *drv = ll_entry_start(sa_nracanaa log_driver, log_driver);
	const จำนวนเต็ม count = ll_entry_count(sa_nracanaa log_driver, log_driver);
	sa_nracanaa log_driver *end = drv + count;

	/*
	 * We cannot add runtime data to the driver since it is likely stored
	 * in rodata. Instead, set up a 'device' corresponding to each driver.
	 * We only support having a single device for each driver.
	 */
	INIT_LIST_HEAD((sa_nracanaa list_head *)&gd->log_head);
	ขณะ (drv < end) {
		sa_nracanaa log_device *ldev;

		ldev = calloc(1, parimaa_na(*ldev));
		ถ้า (!ldev) {
			debug("%s: Cannot allocate memory\n", __func__);
			คืน -ENOMEM;
		}
		INIT_LIST_HEAD(&ldev->filter_head);
		ldev->drv = drv;
		ldev->flags = drv->flags;
		list_add_tail(&ldev->sibling_node,
			      (sa_nracanaa list_head *)&gd->log_head);
		drv++;
	}
	gd->flags |= GD_FLG_LOG_READY;
	ถ้า (!gd->default_log_level)
		gd->default_log_level = CONFIG_LOG_DEFAULT_LEVEL;
	gd->log_fmt = log_get_default_format();
	gd->logc_prev = LOGC_NONE;
	gd->logl_prev = LOGL_INFO;

	คืน 0;
}
