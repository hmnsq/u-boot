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

static const char *const log_cat_name[] = {
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

static const char *const log_level_name[] = {
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
const char *log_get_cat_name(enum log_category_t cat)
{
	const char *name;

	만약 (cat < 0 || cat >= LOGC_COUNT)
		반환 "<invalid>";
	만약 (cat >= LOGC_NONE)
		반환 log_cat_name[cat - LOGC_NONE];

#만약 CONFIG_IS_ENABLED(DM)
	name = uclass_get_name((enum uclass_id)cat);
#아니면
	name = NULL;
#endif

	반환 name ? name : "<missing>";
}

enum log_category_t log_get_cat_by_name(const char *name)
{
	enum uclass_id id;
	정수 i;

	위해 (i = LOGC_NONE; i < LOGC_COUNT; i++)
		만약 (!strcmp(name, log_cat_name[i - LOGC_NONE]))
			반환 i;
	id = uclass_get_by_name(name);
	만약 (id != UCLASS_INVALID)
		반환 (enum log_category_t)id;

	반환 LOGC_NONE;
}

const char *log_get_level_name(enum log_level_t level)
{
	만약 (level >= LOGL_COUNT)
		반환 "INVALID";
	반환 log_level_name[level];
}

enum log_level_t log_get_level_by_name(const char *name)
{
	정수 i;

	위해 (i = 0; i < LOGL_COUNT; i++) {
		만약 (!strcasecmp(log_level_name[i], name))
			반환 i;
	}

	반환 LOGL_NONE;
}

struct log_device *log_device_find_by_name(const char *drv_name)
{
	struct log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_node) {
		만약 (!strcmp(drv_name, ldev->drv->name))
			반환 ldev;
	}

	반환 NULL;
}

bool log_has_cat(enum log_category_t cat_list[], enum log_category_t cat)
{
	정수 i;

	위해 (i = 0; i < LOGF_MAX_CATEGORIES && cat_list[i] != LOGC_END; i++) {
		만약 (cat_list[i] == cat)
			반환 true;
	}

	반환 false;
}

/**
 * log_has_member() - check if a string is in a comma separated list
 *
 * @list:	Comma separated list of strings
 * @member:	String to find
 *
 * Return: ``true`` if @member is in @list, else ``false``
 */
static bool log_has_member(const char *list, const char *member)
{
	정수 member_len = strlen(member);
	const char *s, *p;
	정수 substr_len;

	위해 (s = list; *s; s = p + (*p != '\0')) {
		p = strchrnul(s, ',');
		substr_len = p - s;
		만약 (member_len >= substr_len &&
		    !strncmp(member + member_len - substr_len, s, substr_len))
			반환 true;
	}

	반환 false;
}

/**
 * log_passes_filters() - check if a log record passes the filters for a device
 *
 * @ldev: Log device to check
 * @rec: Log record to check
 * Return: true if @rec is not blocked by the filters in @ldev, false if it is
 */
static bool log_passes_filters(struct log_device *ldev, struct log_rec *rec)
{
	struct log_filter *filt;

	만약 (rec->flags & LOGRECF_FORCE_DEBUG)
		반환 true;

	/* If there are no filters, filter on the default log level */
	만약 (list_empty(&ldev->filter_head)) {
		만약 (rec->level > gd->default_log_level)
			반환 false;
		반환 true;
	}

	list_for_each_entry(filt, &ldev->filter_head, sibling_node) {
		만약 (filt->flags & LOGFF_LEVEL_MIN) {
			만약 (rec->level < filt->level)
				continue;
		} 아니면 만약 (rec->level > filt->level) {
			continue;
		}

		만약 ((filt->flags & LOGFF_HAS_CAT) &&
		    !log_has_cat(filt->cat_list, rec->cat))
			continue;

		만약 (filt->file_list &&
		    !log_has_member(filt->file_list, rec->file))
			continue;

		만약 (filt->func_list &&
		    !log_has_member(filt->func_list, rec->func))
			continue;

		만약 (filt->flags & LOGFF_DENY)
			반환 false;
		아니면
			반환 true;
	}

	반환 false;
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
static 정수 log_dispatch(struct log_rec *rec, const char *fmt, va_list args)
{
	struct log_device *ldev;
	char buf[CONFIG_SYS_CBSIZE];

	/*
	 * When a log driver writes messages (e.g. via the network stack) this
	 * may result in further generated messages. We cannot process them here
	 * as this might result in infinite recursion.
	 */
	만약 (gd->processing_msg)
		반환 1;

	/* Emit message */
	gd->processing_msg = true;
	list_for_each_entry(ldev, &gd->log_head, sibling_node) {
		만약 ((ldev->flags & LOGDF_ENABLE) &&
		    log_passes_filters(ldev, rec)) {
			만약 (!rec->msg) {
				정수 len;

				len = vsnprintf(buf, sizeof(buf), fmt, args);
				rec->msg = buf;
				gd->log_cont = len && buf[len - 1] != '\n';
			}
			ldev->drv->emit(ldev, rec);
		}
	}
	gd->processing_msg = false;
	반환 0;
}

정수 _log(enum log_category_t cat, enum log_level_t level, const char *file,
	 정수 line, const char *func, const char *fmt, ...)
{
	struct log_rec rec;
	va_list args;

	만약 (!gd)
		반환 -ENOSYS;

	/* Check for message continuation */
	만약 (cat == LOGC_CONT)
		cat = gd->logc_prev;
	만약 (level == LOGL_CONT)
		level = gd->logl_prev;

	rec.cat = cat;
	rec.level = level & LOGL_LEVEL_MASK;
	rec.flags = 0;
	만약 (level & LOGL_FORCE_DEBUG)
		rec.flags |= LOGRECF_FORCE_DEBUG;
	만약 (gd->log_cont)
		rec.flags |= LOGRECF_CONT;
	rec.file = file;
	rec.line = line;
	rec.func = func;
	rec.msg = NULL;

	만약 (!(gd->flags & GD_FLG_LOG_READY)) {
		gd->log_drop_count++;

		/* display dropped traces with console puts and DEBUG_UART */
		만약 (rec.level <= CONFIG_LOG_DEFAULT_LEVEL ||
		    rec.flags & LOGRECF_FORCE_DEBUG) {
			char buf[CONFIG_SYS_CBSIZE];

			va_start(args, fmt);
			vsnprintf(buf, sizeof(buf), fmt, args);
			puts(buf);
			va_end(args);
		}

		반환 -ENOSYS;
	}
	va_start(args, fmt);
	만약 (!log_dispatch(&rec, fmt, args)) {
		gd->logc_prev = cat;
		gd->logl_prev = level;
	}
	va_end(args);

	반환 0;
}

#define MAX_LINE_LENGTH_BYTES		64
#define DEFAULT_LINE_LENGTH_BYTES	16

정수 _log_buffer(enum log_category_t cat, enum log_level_t level,
		const char *file, 정수 line, const char *func, ulong addr,
		const void *data, uint width, uint count, uint linelen)
{
	만약 (linelen * width > MAX_LINE_LENGTH_BYTES)
		linelen = MAX_LINE_LENGTH_BYTES / width;
	만약 (linelen < 1)
		linelen = DEFAULT_LINE_LENGTH_BYTES / width;

	동안 (count) {
		uint thislinelen;
		char buf[HEXDUMP_MAX_BUF_LENGTH(width * linelen)];

		thislinelen = hexdump_line(addr, data, width, count, linelen,
					   buf, sizeof(buf));
		assert(thislinelen >= 0);
		_log(cat, level, file, line, func, "%s\n", buf);

		/* update references */
		data += thislinelen * width;
		addr += thislinelen * width;
		count -= thislinelen;
	}

	반환 0;
}

정수 log_add_filter_flags(const char *drv_name, enum log_category_t cat_list[],
			 enum log_level_t level, const char *file_list,
			 const char *func_list, 정수 flags)
{
	struct log_filter *filt;
	struct log_device *ldev;
	정수 ret;
	정수 i;

	ldev = log_device_find_by_name(drv_name);
	만약 (!ldev)
		반환 -ENOENT;
	filt = calloc(1, sizeof(*filt));
	만약 (!filt)
		반환 -ENOMEM;

	filt->flags = flags;
	만약 (cat_list) {
		filt->flags |= LOGFF_HAS_CAT;
		위해 (i = 0; ; i++) {
			만약 (i == ARRAY_SIZE(filt->cat_list)) {
				ret = -ENOSPC;
				goto err;
			}
			filt->cat_list[i] = cat_list[i];
			만약 (cat_list[i] == LOGC_END)
				break;
		}
	}
	filt->level = level;
	만약 (file_list) {
		filt->file_list = strdup(file_list);
		만약 (!filt->file_list) {
			ret = -ENOMEM;
			goto err;
		}
	}
	만약 (func_list) {
		filt->func_list = strdup(func_list);
		만약 (!filt->func_list) {
			ret = -ENOMEM;
			goto err;
		}
	}
	filt->filter_num = ldev->next_filter_num++;
	/* Add deny filters to the beginning of the list */
	만약 (flags & LOGFF_DENY)
		list_add(&filt->sibling_node, &ldev->filter_head);
	아니면
		list_add_tail(&filt->sibling_node, &ldev->filter_head);

	반환 filt->filter_num;

err:
	free(filt);
	반환 ret;
}

정수 log_remove_filter(const char *drv_name, 정수 filter_num)
{
	struct log_filter *filt;
	struct log_device *ldev;

	ldev = log_device_find_by_name(drv_name);
	만약 (!ldev)
		반환 -ENOENT;

	list_for_each_entry(filt, &ldev->filter_head, sibling_node) {
		만약 (filt->filter_num == filter_num) {
			list_del(&filt->sibling_node);
			free(filt);

			반환 0;
		}
	}

	반환 -ENOENT;
}

/**
 * log_find_device_by_drv() - Find a device by its driver
 *
 * @drv: Log driver
 * Return: Device associated with that driver, or NULL if not found
 */
static struct log_device *log_find_device_by_drv(struct log_driver *drv)
{
	struct log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_node) {
		만약 (ldev->drv == drv)
			반환 ldev;
	}
	/*
	 * It is quite hard to pass an invalid driver since passing an unknown
	 * LOG_GET_DRIVER(xxx) would normally produce a compilation error. But
	 * it is possible to pass NULL, for example, so this
	 */

	반환 NULL;
}

정수 log_device_set_enable(struct log_driver *drv, bool enable)
{
	struct log_device *ldev;

	ldev = log_find_device_by_drv(drv);
	만약 (!ldev)
		반환 -ENOENT;
	만약 (enable)
		ldev->flags |= LOGDF_ENABLE;
	아니면
		ldev->flags &= ~LOGDF_ENABLE;

	반환 0;
}

void log_fixup_for_gd_move(struct global_data *new_gd)
{
	new_gd->log_head.prev->next = &new_gd->log_head;
}

정수 log_init(void)
{
	struct log_driver *drv = ll_entry_start(struct log_driver, log_driver);
	const 정수 count = ll_entry_count(struct log_driver, log_driver);
	struct log_driver *end = drv + count;

	/*
	 * We cannot add runtime data to the driver since it is likely stored
	 * in rodata. Instead, set up a 'device' corresponding to each driver.
	 * We only support having a single device for each driver.
	 */
	INIT_LIST_HEAD((struct list_head *)&gd->log_head);
	동안 (drv < end) {
		struct log_device *ldev;

		ldev = calloc(1, sizeof(*ldev));
		만약 (!ldev) {
			debug("%s: Cannot allocate memory\n", __func__);
			반환 -ENOMEM;
		}
		INIT_LIST_HEAD(&ldev->filter_head);
		ldev->drv = drv;
		ldev->flags = drv->flags;
		list_add_tail(&ldev->sibling_node,
			      (struct list_head *)&gd->log_head);
		drv++;
	}
	gd->flags |= GD_FLG_LOG_READY;
	만약 (!gd->default_log_level)
		gd->default_log_level = CONFIG_LOG_DEFAULT_LEVEL;
	gd->log_fmt = log_get_default_format();
	gd->logc_prev = LOGC_NONE;
	gd->logl_prev = LOGL_INFO;

	반환 0;
}
