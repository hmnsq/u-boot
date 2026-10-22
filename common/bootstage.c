// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 */

/*
 * This module records the progress of boot and arbitrary commands, and
 * permits accurate timestamping of each.
 */

#define LOG_CATEGORY	LOGC_BOOT

#include <bootstage.h>
#include <hang.h>
#include <log.h>
#include <malloc.h>
#include <sort.h>
#include <spl.h>
#include <asm/global_data.h>
#include <linux/compiler.h>
#include <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	RECORD_COUNT = CONFIG_VAL(BOOTSTAGE_RECORD_COUNT),
};

struct bootstage_record {
	ulong time_us;
	uint32_t start_us;
	const char *name;
	ціле flags;		/* see enum bootstage_flags */
	enum bootstage_id id;
};

struct bootstage_data {
	uint rec_count;
	uint next_id;
	struct bootstage_record record[RECORD_COUNT];
};

enum {
	BOOTSTAGE_VERSION	= 0,
	BOOTSTAGE_MAGIC		= 0xb00757a3,
	BOOTSTAGE_DIGITS	= 9,
};

struct bootstage_hdr {
	u32 version;		/* BOOTSTAGE_VERSION */
	u32 count;		/* Number of records */
	u32 size;		/* Total data size (non-zero if valid) */
	u32 magic;		/* Magic number */
	u32 next_id;		/* Next ID to use for bootstage */
};

ціле bootstage_relocate(void *to)
{
	struct bootstage_data *data;
	ціле i;
	char *ptr;

	debug("Copying bootstage from %p to %p\n", gd->bootstage, to);
	memcpy(to, gd->bootstage, sizeof(struct bootstage_data));
	data = gd->bootstage = to;

	/* Figure out where to relocate the strings to */
	ptr = (char *)(data + 1);

	/*
	 * Duplicate all strings.  They may point to an old location in the
	 * program .text section that can eventually get trashed.
	 */
	debug("Relocating %d records\n", data->rec_count);
	для (i = 0; i < data->rec_count; i++) {
		const char *from = data->record[i].name;

		якщо (!from)
			continue;
		strcpy(ptr, from);
		data->record[i].name = ptr;
		ptr += strlen(ptr) + 1;
	}

	повернути 0;
}

struct bootstage_record *find_id(struct bootstage_data *data,
				 enum bootstage_id id)
{
	struct bootstage_record *rec;
	struct bootstage_record *end;

	для (rec = data->record, end = rec + data->rec_count; rec < end;
	     rec++) {
		якщо (rec->id == id)
			повернути rec;
	}

	повернути NULL;
}

struct bootstage_record *ensure_id(struct bootstage_data *data,
				   enum bootstage_id id)
{
	struct bootstage_record *rec;

	rec = find_id(data, id);
	якщо (!rec && data->rec_count < RECORD_COUNT) {
		rec = &data->record[data->rec_count++];
		rec->id = id;
		повернути rec;
	}

	повернути rec;
}

ulong bootstage_add_record(enum bootstage_id id, const char *name,
			   ціле flags, ulong mark)
{
	struct bootstage_data *data = gd->bootstage;
	struct bootstage_record *rec;

	/*
	 * initf_bootstage() is called very early during boot but since hang()
	 * calls bootstage_error() we can be called before bootstage is set up.
	 * Add a check to avoid this.
	 */
	якщо (!data)
		повернути mark;
	якщо (flags & BOOTSTAGEF_ALLOC)
		id = data->next_id++;

	/* Only record the first event for each */
	rec = find_id(data, id);
	якщо (!rec) {
		якщо (data->rec_count < RECORD_COUNT) {
			rec = &data->record[data->rec_count++];
			rec->time_us = mark;
			rec->name = name;
			rec->flags = flags;
			rec->id = id;
		} інакше {
			log_warning("Bootstage space exhausted\n");
		}
	}

	/* Tell the board about this progress */
	show_boot_progress(flags & BOOTSTAGEF_ERROR ? -id : id);

	повернути mark;
}

ulong bootstage_error_name(enum bootstage_id id, const char *name)
{
	повернути bootstage_add_record(id, name, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(enum bootstage_id id, const char *name)
{
	ціле flags = 0;

	якщо (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;

	повернути bootstage_add_record(id, name, flags, timer_get_boot_us());
}

ulong bootstage_mark_code(const char *file, const char *func, ціле linenum)
{
	char *str, *p;
	__maybe_unused char *end;
	ціле len = 0;

	/* First work out the length we need to allocate */
	якщо (linenum != -1)
		len = 11;
	якщо (func)
		len += strlen(func);
	якщо (file)
		len += strlen(file);

	str = malloc(len + 1);
	p = str;
	end = p + len;
	якщо (file)
		p += snprintf(p, end - p, "%s,", file);
	якщо (linenum != -1)
		p += snprintf(p, end - p, "%d", linenum);
	якщо (func)
		p += snprintf(p, end - p, ": %s", func);

	повернути bootstage_mark_name(BOOTSTAGE_ID_ALLOC, str);
}

uint32_t bootstage_start(enum bootstage_id id, const char *name)
{
	struct bootstage_data *data = gd->bootstage;
	struct bootstage_record *rec = ensure_id(data, id);
	ulong start_us = timer_get_boot_us();

	якщо (rec) {
		rec->start_us = start_us;
		rec->name = name;
	}

	повернути start_us;
}

uint32_t bootstage_accum(enum bootstage_id id)
{
	struct bootstage_data *data = gd->bootstage;
	struct bootstage_record *rec = ensure_id(data, id);
	uint32_t duration;

	якщо (!rec)
		повернути 0;
	duration = (uint32_t)timer_get_boot_us() - rec->start_us;
	rec->time_us += duration;

	повернути duration;
}

/**
 * Get a record name as a printable string
 *
 * @param buf	Buffer to put name if needed
 * @param len	Length of buffer
 * @param rec	Boot stage record to get the name from
 * Return: pointer to name, either from the record or pointing to buf.
 */
static const char *get_record_name(char *buf, ціле len,
				   const struct bootstage_record *rec)
{
	якщо (rec->name)
		повернути rec->name;
	інакше якщо (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	інакше
		snprintf(buf, len, "id=%d", rec->id);

	повернути buf;
}

static uint32_t print_time_record(struct bootstage_record *rec, uint32_t prev)
{
	char buf[20];

	якщо (prev == -1U) {
		printf("%11s", "");
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
	} інакше {
		якщо (prev > rec->time_us)
			prev = 0;
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
		print_grouped_ull(rec->time_us - prev, BOOTSTAGE_DIGITS);
	}
	printf("  %s\n", get_record_name(buf, sizeof(buf), rec));

	повернути rec->time_us;
}

#ifdef CONFIG_OF_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * Return: 0 on success, != 0 on failure.
 */
static ціле add_bootstages_devicetree(struct fdt_header *blob)
{
	struct bootstage_data *data = gd->bootstage;
	ціле bootstage;
	char buf[20];
	ціле recnum;
	ціле i;

	якщо (!blob)
		повернути 0;

	/*
	 * Create the node for bootstage.
	 * The address of flat device tree is set up by the command bootm.
	 */
	bootstage = fdt_add_subnode(blob, 0, "bootstage");
	якщо (bootstage < 0)
		повернути -EINVAL;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	для (recnum = data->rec_count - 1, i = 0; recnum >= 0; recnum--, i++) {
		struct bootstage_record *rec = &data->record[recnum];
		ціле node;

		якщо (rec->id != BOOTSTAGE_ID_AWAKE && rec->time_us == 0)
			continue;

		node = fdt_add_subnode(blob, bootstage, simple_itoa(i));
		якщо (node < 0)
			break;

		/* add properties to the node. */
		якщо (fdt_setprop_string(blob, node, "name",
				       get_record_name(buf, sizeof(buf), rec)))
			повернути -EINVAL;

		/* Check if this is a 'mark' or 'accum' record */
		якщо (fdt_setprop_cell(blob, node,
				rec->start_us ? "accum" : "mark",
				rec->time_us))
			повернути -EINVAL;
	}

	повернути 0;
}

ціле bootstage_fdt_add_report(void)
{
	якщо (add_bootstages_devicetree(working_fdt))
		puts("bootstage: Failed to add to device tree\n");

	повернути 0;
}
#endif

void bootstage_report(void)
{
	struct bootstage_data *data = gd->bootstage;
	struct bootstage_record *rec = data->record;
	uint32_t prev;
	ціле i;

	printf("Timer summary in microseconds (%d records):\n",
	       data->rec_count);
	printf("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	prev = print_time_record(rec, 0);

	для (i = 1, rec++; i < data->rec_count; i++, rec++) {
		якщо (rec->id && !rec->start_us)
			prev = print_time_record(rec, prev);
	}
	якщо (data->rec_count > RECORD_COUNT)
		printf("Overflowed internal boot id table by %d entries\n"
		       "Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		       data->rec_count - RECORD_COUNT);

	puts("\nAccumulated time:\n");
	для (i = 0, rec = data->record; i < data->rec_count; i++, rec++) {
		якщо (rec->start_us)
			prev = print_time_record(rec, -1);
	}
}

/**
 * Append data to a memory buffer
 *
 * Write data to the buffer if there is space. Whether there is space or not,
 * the buffer pointer is incremented.
 *
 * @param ptrp	Pointer to buffer, updated by this function
 * @param end	Pointer to end of buffer
 * @param data	Data to write to buffer
 * @param size	Size of data
 */
static void append_data(char **ptrp, char *end, const void *data, ціле size)
{
	char *ptr = *ptrp;

	*ptrp += size;
	якщо (*ptrp > end)
		повернути;

	memcpy(ptr, data, size);
}

ціле bootstage_stash(void *base, ціле size)
{
	const struct bootstage_data *data = gd->bootstage;
	struct bootstage_hdr *hdr = (struct bootstage_hdr *)base;
	const struct bootstage_record *rec;
	char buf[20];
	char *ptr = base, *end = ptr + size;
	ціле i;

	якщо (hdr + 1 > (struct bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		повернути -ENOSPC;
	}

	/* Write an arbitrary version number */
	hdr->version = BOOTSTAGE_VERSION;

	hdr->count = data->rec_count;
	hdr->size = 0;
	hdr->magic = BOOTSTAGE_MAGIC;
	hdr->next_id = data->next_id;
	ptr += sizeof(*hdr);

	/* Write the records, silently stopping when we run out of space */
	для (rec = data->record, i = 0; i < data->rec_count; i++, rec++)
		append_data(&ptr, end, rec, sizeof(*rec));

	/* Write the name strings */
	для (rec = data->record, i = 0; i < data->rec_count; i++, rec++) {
		const char *name;

		name = get_record_name(buf, sizeof(buf), rec);
		append_data(&ptr, end, name, strlen(name) + 1);
	}

	/* Check for buffer overflow */
	якщо (ptr > end) {
		debug("%s: Not enough space for bootstage stash\n", __func__);
		повернути -ENOSPC;
	}

	/* Update total data size */
	hdr->size = ptr - (char *)base;
	debug("Stashed %d records\n", hdr->count);

	повернути 0;
}

ціле bootstage_unstash(const void *base, ціле size)
{
	const struct bootstage_hdr *hdr = (struct bootstage_hdr *)base;
	struct bootstage_data *data = gd->bootstage;
	const char *ptr = base, *end = ptr + size;
	struct bootstage_record *rec;
	uint rec_size;
	ціле i;

	якщо (size == -1)
		end = (char *)(~(uintptr_t)0);

	якщо (hdr + 1 > (struct bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		повернути -EPERM;
	}

	якщо (hdr->magic != BOOTSTAGE_MAGIC) {
		debug("%s: Invalid bootstage magic\n", __func__);
		повернути -ENOENT;
	}

	якщо (ptr + hdr->size > end) {
		debug("%s: Bootstage data runs past buffer end\n", __func__);
		повернути -ENOSPC;
	}

	якщо (hdr->count * sizeof(*rec) > hdr->size) {
		debug("%s: Bootstage has %d records needing %lu bytes, but "
			"only %d bytes is available\n", __func__, hdr->count,
		      (ulong)hdr->count * sizeof(*rec), hdr->size);
		повернути -ENOSPC;
	}

	якщо (hdr->version != BOOTSTAGE_VERSION) {
		debug("%s: Bootstage data version %#0x unrecognised\n",
		      __func__, hdr->version);
		повернути -EINVAL;
	}

	якщо (data->rec_count + hdr->count > RECORD_COUNT) {
		debug("%s: Bootstage has %d records, we have space for %d\n"
			"Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		      __func__, hdr->count, RECORD_COUNT - data->rec_count);
		повернути -ENOSPC;
	}

	ptr += sizeof(*hdr);

	/* Read the records */
	rec_size = hdr->count * sizeof(*data->record);
	memcpy(data->record + data->rec_count, ptr, rec_size);

	/* Read the name strings */
	ptr += rec_size;
	для (rec = data->record + data->rec_count, i = 0; i < hdr->count;
	     i++, rec++) {
		rec->name = ptr;
		якщо (xpl_phase() == PHASE_SPL)
			rec->name = strdup(ptr);

		/* Assume no data corruption here */
		ptr += strlen(ptr) + 1;
	}

	/* Mark the records as read */
	data->rec_count += hdr->count;
	data->next_id = hdr->next_id;
	debug("Unstashed %d records\n", hdr->count);

	повернути 0;
}

#якщо IS_ENABLED(CONFIG_BOOTSTAGE_STASH)
ціле _bootstage_stash_default(void)
{
	повернути bootstage_stash(map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR, 0),
			       CONFIG_BOOTSTAGE_STASH_SIZE);
}

ціле _bootstage_unstash_default(void)
{
	const void *stash = map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR,
				       CONFIG_BOOTSTAGE_STASH_SIZE);

	повернути bootstage_unstash(stash, CONFIG_BOOTSTAGE_STASH_SIZE);
}
#endif

ціле bootstage_get_size(bool add_strings)
{
	ціле size;

	size = sizeof(struct bootstage_data);
	якщо (add_strings) {
		struct bootstage_data *data = gd->bootstage;
		struct bootstage_record *rec;
		ціле i;

		для (rec = data->record, i = 0; i < data->rec_count; i++, rec++)
			якщо (rec->name)
				size += strlen(rec->name) + 1;
	}

	повернути size;
}

ціле bootstage_init(bool first)
{
	struct bootstage_data *data;
	ціле size = sizeof(struct bootstage_data);

	gd->bootstage = (struct bootstage_data *)malloc(size);
	якщо (!gd->bootstage)
		повернути -ENOMEM;
	data = gd->bootstage;
	memset(data, '\0', size);
	якщо (first) {
		data->next_id = BOOTSTAGE_ID_USER;
		bootstage_add_record(BOOTSTAGE_ID_AWAKE, "reset", 0, 0);
	}

	повернути 0;
}
