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

	nếu (cat < 0 || cat >= LOGC_COUNT)
		trả_về "<invalid>";
	nếu (cat >= LOGC_NONE)
		trả_về log_cat_name[cat - LOGC_NONE];

#nếu CONFIG_IS_ENABLED(DM)
	name = uclass_get_name((enum uclass_id)cat);
#không_thì
	name = NULL;
#endif

	trả_về name ? name : "<missing>";
}

enum log_category_t log_get_cat_by_name(const char *name)
{
	enum uclass_id id;
	số_nguyên i;

	cho (i = LOGC_NONE; i < LOGC_COUNT; i++)
		nếu (!strcmp(name, log_cat_name[i - LOGC_NONE]))
			trả_về i;
	id = uclass_get_by_name(name);
	nếu (id != UCLASS_INVALID)
		trả_về (enum log_category_t)id;

	trả_về LOGC_NONE;
}

const char *log_get_level_name(enum log_level_t level)
{
	nếu (level >= LOGL_COUNT)
		trả_về "INVALID";
	trả_về log_level_name[level];
}

enum log_level_t log_get_level_by_name(const char *name)
{
	số_nguyên i;

	cho (i = 0; i < LOGL_COUNT; i++) {
		nếu (!strcasecmp(log_level_name[i], name))
			trả_về i;
	}

	trả_về LOGL_NONE;
}

struct log_device *log_device_find_by_name(const char *drv_name)
{
	struct log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_node) {
		nếu (!strcmp(drv_name, ldev->drv->name))
			trả_về ldev;
	}

	trả_về NULL;
}

bool log_has_cat(enum log_category_t cat_list[], enum log_category_t cat)
{
	số_nguyên i;

	cho (i = 0; i < LOGF_MAX_CATEGORIES && cat_list[i] != LOGC_END; i++) {
		nếu (cat_list[i] == cat)
			trả_về true;
	}

	trả_về false;
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
	số_nguyên member_len = strlen(member);
	const char *s, *p;
	số_nguyên substr_len;

	cho (s = list; *s; s = p + (*p != '\0')) {
		p = strchrnul(s, ',');
		substr_len = p - s;
		nếu (member_len >= substr_len &&
		    !strncmp(member + member_len - substr_len, s, substr_len))
			trả_về true;
	}

	trả_về false;
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

	nếu (rec->flags & LOGRECF_FORCE_DEBUG)
		trả_về true;

	/* If there are no filters, filter on the default log level */
	nếu (list_empty(&ldev->filter_head)) {
		nếu (rec->level > gd->default_log_level)
			trả_về false;
		trả_về true;
	}

	list_for_each_entry(filt, &ldev->filter_head, sibling_node) {
		nếu (filt->flags & LOGFF_LEVEL_MIN) {
			nếu (rec->level < filt->level)
				continue;
		} không_thì nếu (rec->level > filt->level) {
			continue;
		}

		nếu ((filt->flags & LOGFF_HAS_CAT) &&
		    !log_has_cat(filt->cat_list, rec->cat))
			continue;

		nếu (filt->file_list &&
		    !log_has_member(filt->file_list, rec->file))
			continue;

		nếu (filt->func_list &&
		    !log_has_member(filt->func_list, rec->func))
			continue;

		nếu (filt->flags & LOGFF_DENY)
			trả_về false;
		không_thì
			trả_về true;
	}

	trả_về false;
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
static số_nguyên log_dispatch(struct log_rec *rec, const char *fmt, va_list args)
{
	struct log_device *ldev;
	char buf[CONFIG_SYS_CBSIZE];

	/*
	 * When a log driver writes messages (e.g. via the network stack) this
	 * may result in further generated messages. We cannot process them here
	 * as this might result in infinite recursion.
	 */
	nếu (gd->processing_msg)
		trả_về 1;

	/* Emit message */
	gd->processing_msg = true;
	list_for_each_entry(ldev, &gd->log_head, sibling_node) {
		nếu ((ldev->flags & LOGDF_ENABLE) &&
		    log_passes_filters(ldev, rec)) {
			nếu (!rec->msg) {
				số_nguyên len;

				len = vsnprintf(buf, sizeof(buf), fmt, args);
				rec->msg = buf;
				gd->log_cont = len && buf[len - 1] != '\n';
			}
			ldev->drv->emit(ldev, rec);
		}
	}
	gd->processing_msg = false;
	trả_về 0;
}

số_nguyên _log(enum log_category_t cat, enum log_level_t level, const char *file,
	 số_nguyên line, const char *func, const char *fmt, ...)
{
	struct log_rec rec;
	va_list args;

	nếu (!gd)
		trả_về -ENOSYS;

	/* Check for message continuation */
	nếu (cat == LOGC_CONT)
		cat = gd->logc_prev;
	nếu (level == LOGL_CONT)
		level = gd->logl_prev;

	rec.cat = cat;
	rec.level = level & LOGL_LEVEL_MASK;
	rec.flags = 0;
	nếu (level & LOGL_FORCE_DEBUG)
		rec.flags |= LOGRECF_FORCE_DEBUG;
	nếu (gd->log_cont)
		rec.flags |= LOGRECF_CONT;
	rec.file = file;
	rec.line = line;
	rec.func = func;
	rec.msg = NULL;

	nếu (!(gd->flags & GD_FLG_LOG_READY)) {
		gd->log_drop_count++;

		/* display dropped traces with console puts and DEBUG_UART */
		nếu (rec.level <= CONFIG_LOG_DEFAULT_LEVEL ||
		    rec.flags & LOGRECF_FORCE_DEBUG) {
			char buf[CONFIG_SYS_CBSIZE];

			va_start(args, fmt);
			vsnprintf(buf, sizeof(buf), fmt, args);
			puts(buf);
			va_end(args);
		}

		trả_về -ENOSYS;
	}
	va_start(args, fmt);
	nếu (!log_dispatch(&rec, fmt, args)) {
		gd->logc_prev = cat;
		gd->logl_prev = level;
	}
	va_end(args);

	trả_về 0;
}

#define MAX_LINE_LENGTH_BYTES		64
#define DEFAULT_LINE_LENGTH_BYTES	16

số_nguyên _log_buffer(enum log_category_t cat, enum log_level_t level,
		const char *file, số_nguyên line, const char *func, ulong addr,
		const void *data, uint width, uint count, uint linelen)
{
	nếu (linelen * width > MAX_LINE_LENGTH_BYTES)
		linelen = MAX_LINE_LENGTH_BYTES / width;
	nếu (linelen < 1)
		linelen = DEFAULT_LINE_LENGTH_BYTES / width;

	trong_khi (count) {
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

	trả_về 0;
}

số_nguyên log_add_filter_flags(const char *drv_name, enum log_category_t cat_list[],
			 enum log_level_t level, const char *file_list,
			 const char *func_list, số_nguyên flags)
{
	struct log_filter *filt;
	struct log_device *ldev;
	số_nguyên ret;
	số_nguyên i;

	ldev = log_device_find_by_name(drv_name);
	nếu (!ldev)
		trả_về -ENOENT;
	filt = calloc(1, sizeof(*filt));
	nếu (!filt)
		trả_về -ENOMEM;

	filt->flags = flags;
	nếu (cat_list) {
		filt->flags |= LOGFF_HAS_CAT;
		cho (i = 0; ; i++) {
			nếu (i == ARRAY_SIZE(filt->cat_list)) {
				ret = -ENOSPC;
				goto err;
			}
			filt->cat_list[i] = cat_list[i];
			nếu (cat_list[i] == LOGC_END)
				break;
		}
	}
	filt->level = level;
	nếu (file_list) {
		filt->file_list = strdup(file_list);
		nếu (!filt->file_list) {
			ret = -ENOMEM;
			goto err;
		}
	}
	nếu (func_list) {
		filt->func_list = strdup(func_list);
		nếu (!filt->func_list) {
			ret = -ENOMEM;
			goto err;
		}
	}
	filt->filter_num = ldev->next_filter_num++;
	/* Add deny filters to the beginning of the list */
	nếu (flags & LOGFF_DENY)
		list_add(&filt->sibling_node, &ldev->filter_head);
	không_thì
		list_add_tail(&filt->sibling_node, &ldev->filter_head);

	trả_về filt->filter_num;

err:
	free(filt);
	trả_về ret;
}

số_nguyên log_remove_filter(const char *drv_name, số_nguyên filter_num)
{
	struct log_filter *filt;
	struct log_device *ldev;

	ldev = log_device_find_by_name(drv_name);
	nếu (!ldev)
		trả_về -ENOENT;

	list_for_each_entry(filt, &ldev->filter_head, sibling_node) {
		nếu (filt->filter_num == filter_num) {
			list_del(&filt->sibling_node);
			free(filt);

			trả_về 0;
		}
	}

	trả_về -ENOENT;
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
		nếu (ldev->drv == drv)
			trả_về ldev;
	}
	/*
	 * It is quite hard to pass an invalid driver since passing an unknown
	 * LOG_GET_DRIVER(xxx) would normally produce a compilation error. But
	 * it is possible to pass NULL, for example, so this
	 */

	trả_về NULL;
}

số_nguyên log_device_set_enable(struct log_driver *drv, bool enable)
{
	struct log_device *ldev;

	ldev = log_find_device_by_drv(drv);
	nếu (!ldev)
		trả_về -ENOENT;
	nếu (enable)
		ldev->flags |= LOGDF_ENABLE;
	không_thì
		ldev->flags &= ~LOGDF_ENABLE;

	trả_về 0;
}

void log_fixup_for_gd_move(struct global_data *new_gd)
{
	new_gd->log_head.prev->next = &new_gd->log_head;
}

số_nguyên log_init(void)
{
	struct log_driver *drv = ll_entry_start(struct log_driver, log_driver);
	const số_nguyên count = ll_entry_count(struct log_driver, log_driver);
	struct log_driver *end = drv + count;

	/*
	 * We cannot add runtime data to the driver since it is likely stored
	 * in rodata. Instead, set up a 'device' corresponding to each driver.
	 * We only support having a single device for each driver.
	 */
	INIT_LIST_HEAD((struct list_head *)&gd->log_head);
	trong_khi (drv < end) {
		struct log_device *ldev;

		ldev = calloc(1, sizeof(*ldev));
		nếu (!ldev) {
			debug("%s: Cannot allocate memory\n", __func__);
			trả_về -ENOMEM;
		}
		INIT_LIST_HEAD(&ldev->filter_head);
		ldev->drv = drv;
		ldev->flags = drv->flags;
		list_add_tail(&ldev->sibling_node,
			      (struct list_head *)&gd->log_head);
		drv++;
	}
	gd->flags |= GD_FLG_LOG_READY;
	nếu (!gd->default_log_level)
		gd->default_log_level = CONFIG_LOG_DEFAULT_LEVEL;
	gd->log_fmt = log_get_default_format();
	gd->logc_prev = LOGC_NONE;
	gd->logl_prev = LOGL_INFO;

	trả_về 0;
}
