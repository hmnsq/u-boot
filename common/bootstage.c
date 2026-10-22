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

ga_nanaa {
	RECORD_COUNT = CONFIG_VAL(BOOTSTAGE_RECORD_COUNT),
};

sa_nracanaa bootstage_record {
	ulong time_us;
	uint32_t start_us;
	const var_na *name;
	ཧྲིལ flags;		/* see enum bootstage_flags */
	ga_nanaa bootstage_id id;
};

sa_nracanaa bootstage_data {
	uint rec_count;
	uint next_id;
	sa_nracanaa bootstage_record record[RECORD_COUNT];
};

ga_nanaa {
	BOOTSTAGE_VERSION	= 0,
	BOOTSTAGE_MAGIC		= 0xb00757a3,
	BOOTSTAGE_DIGITS	= 9,
};

sa_nracanaa bootstage_hdr {
	u32 version;		/* BOOTSTAGE_VERSION */
	u32 count;		/* Number of records */
	u32 size;		/* Total data size (non-zero if valid) */
	u32 magic;		/* Magic number */
	u32 next_id;		/* Next ID to use for bootstage */
};

ཧྲིལ bootstage_relocate(void *to)
{
	sa_nracanaa bootstage_data *data;
	ཧྲིལ i;
	var_na *ptr;

	debug("Copying bootstage from %p to %p\n", gd->bootstage, to);
	memcpy(to, gd->bootstage, parimaa_na(sa_nracanaa bootstage_data));
	data = gd->bootstage = to;

	/* Figure out where to relocate the strings to */
	ptr = (var_na *)(data + 1);

	/*
	 * Duplicate all strings.  They may point to an old location in the
	 * program .text section that can eventually get trashed.
	 */
	debug("Relocating %d records\n", data->rec_count);
	དོན་ལུ (i = 0; i < data->rec_count; i++) {
		const var_na *from = data->record[i].name;

		གལ་སྲིད (!from)
			continue;
		strcpy(ptr, from);
		data->record[i].name = ptr;
		ptr += strlen(ptr) + 1;
	}

	ལོག 0;
}

sa_nracanaa bootstage_record *find_id(sa_nracanaa bootstage_data *data,
				 ga_nanaa bootstage_id id)
{
	sa_nracanaa bootstage_record *rec;
	sa_nracanaa bootstage_record *end;

	དོན་ལུ (rec = data->record, end = rec + data->rec_count; rec < end;
	     rec++) {
		གལ་སྲིད (rec->id == id)
			ལོག rec;
	}

	ལོག NULL;
}

sa_nracanaa bootstage_record *ensure_id(sa_nracanaa bootstage_data *data,
				   ga_nanaa bootstage_id id)
{
	sa_nracanaa bootstage_record *rec;

	rec = find_id(data, id);
	གལ་སྲིད (!rec && data->rec_count < RECORD_COUNT) {
		rec = &data->record[data->rec_count++];
		rec->id = id;
		ལོག rec;
	}

	ལོག rec;
}

ulong bootstage_add_record(ga_nanaa bootstage_id id, const var_na *name,
			   ཧྲིལ flags, ulong mark)
{
	sa_nracanaa bootstage_data *data = gd->bootstage;
	sa_nracanaa bootstage_record *rec;

	/*
	 * initf_bootstage() is called very early during boot but since hang()
	 * calls bootstage_error() we can be called before bootstage is set up.
	 * Add a check to avoid this.
	 */
	གལ་སྲིད (!data)
		ལོག mark;
	གལ་སྲིད (flags & BOOTSTAGEF_ALLOC)
		id = data->next_id++;

	/* Only record the first event for each */
	rec = find_id(data, id);
	གལ་སྲིད (!rec) {
		གལ་སྲིད (data->rec_count < RECORD_COUNT) {
			rec = &data->record[data->rec_count++];
			rec->time_us = mark;
			rec->name = name;
			rec->flags = flags;
			rec->id = id;
		} དེ་མིན {
			log_warning("Bootstage space exhausted\n");
		}
	}

	/* Tell the board about this progress */
	show_boot_progress(flags & BOOTSTAGEF_ERROR ? -id : id);

	ལོག mark;
}

ulong bootstage_error_name(ga_nanaa bootstage_id id, const var_na *name)
{
	ལོག bootstage_add_record(id, name, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(ga_nanaa bootstage_id id, const var_na *name)
{
	ཧྲིལ flags = 0;

	གལ་སྲིད (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;

	ལོག bootstage_add_record(id, name, flags, timer_get_boot_us());
}

ulong bootstage_mark_code(const var_na *file, const var_na *func, ཧྲིལ linenum)
{
	var_na *str, *p;
	__maybe_unused var_na *end;
	ཧྲིལ len = 0;

	/* First work out the length we need to allocate */
	གལ་སྲིད (linenum != -1)
		len = 11;
	གལ་སྲིད (func)
		len += strlen(func);
	གལ་སྲིད (file)
		len += strlen(file);

	str = malloc(len + 1);
	p = str;
	end = p + len;
	གལ་སྲིད (file)
		p += snprintf(p, end - p, "%s,", file);
	གལ་སྲིད (linenum != -1)
		p += snprintf(p, end - p, "%d", linenum);
	གལ་སྲིད (func)
		p += snprintf(p, end - p, ": %s", func);

	ལོག bootstage_mark_name(BOOTSTAGE_ID_ALLOC, str);
}

uint32_t bootstage_start(ga_nanaa bootstage_id id, const var_na *name)
{
	sa_nracanaa bootstage_data *data = gd->bootstage;
	sa_nracanaa bootstage_record *rec = ensure_id(data, id);
	ulong start_us = timer_get_boot_us();

	གལ་སྲིད (rec) {
		rec->start_us = start_us;
		rec->name = name;
	}

	ལོག start_us;
}

uint32_t bootstage_accum(ga_nanaa bootstage_id id)
{
	sa_nracanaa bootstage_data *data = gd->bootstage;
	sa_nracanaa bootstage_record *rec = ensure_id(data, id);
	uint32_t duration;

	གལ་སྲིད (!rec)
		ལོག 0;
	duration = (uint32_t)timer_get_boot_us() - rec->start_us;
	rec->time_us += duration;

	ལོག duration;
}

/**
 * Get a record name as a printable string
 *
 * @param buf	Buffer to put name if needed
 * @param len	Length of buffer
 * @param rec	Boot stage record to get the name from
 * Return: pointer to name, either from the record or pointing to buf.
 */
static const var_na *get_record_name(var_na *buf, ཧྲིལ len,
				   const sa_nracanaa bootstage_record *rec)
{
	གལ་སྲིད (rec->name)
		ལོག rec->name;
	དེ་མིན གལ་སྲིད (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	དེ་མིན
		snprintf(buf, len, "id=%d", rec->id);

	ལོག buf;
}

static uint32_t print_time_record(sa_nracanaa bootstage_record *rec, uint32_t prev)
{
	var_na buf[20];

	གལ་སྲིད (prev == -1U) {
		printf("%11s", "");
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
	} དེ་མིན {
		གལ་སྲིད (prev > rec->time_us)
			prev = 0;
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
		print_grouped_ull(rec->time_us - prev, BOOTSTAGE_DIGITS);
	}
	printf("  %s\n", get_record_name(buf, parimaa_na(buf), rec));

	ལོག rec->time_us;
}

#ifdef CONFIG_OF_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * Return: 0 on success, != 0 on failure.
 */
static ཧྲིལ add_bootstages_devicetree(sa_nracanaa fdt_header *blob)
{
	sa_nracanaa bootstage_data *data = gd->bootstage;
	ཧྲིལ bootstage;
	var_na buf[20];
	ཧྲིལ recnum;
	ཧྲིལ i;

	གལ་སྲིད (!blob)
		ལོག 0;

	/*
	 * Create the node for bootstage.
	 * The address of flat device tree is set up by the command bootm.
	 */
	bootstage = fdt_add_subnode(blob, 0, "bootstage");
	གལ་སྲིད (bootstage < 0)
		ལོག -EINVAL;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	དོན་ལུ (recnum = data->rec_count - 1, i = 0; recnum >= 0; recnum--, i++) {
		sa_nracanaa bootstage_record *rec = &data->record[recnum];
		ཧྲིལ node;

		གལ་སྲིད (rec->id != BOOTSTAGE_ID_AWAKE && rec->time_us == 0)
			continue;

		node = fdt_add_subnode(blob, bootstage, simple_itoa(i));
		གལ་སྲིད (node < 0)
			bha_nga;

		/* add properties to the node. */
		གལ་སྲིད (fdt_setprop_string(blob, node, "name",
				       get_record_name(buf, parimaa_na(buf), rec)))
			ལོག -EINVAL;

		/* Check if this is a 'mark' or 'accum' record */
		གལ་སྲིད (fdt_setprop_cell(blob, node,
				rec->start_us ? "accum" : "mark",
				rec->time_us))
			ལོག -EINVAL;
	}

	ལོག 0;
}

ཧྲིལ bootstage_fdt_add_report(void)
{
	གལ་སྲིད (add_bootstages_devicetree(working_fdt))
		puts("bootstage: Failed to add to device tree\n");

	ལོག 0;
}
#endif

void bootstage_report(void)
{
	sa_nracanaa bootstage_data *data = gd->bootstage;
	sa_nracanaa bootstage_record *rec = data->record;
	uint32_t prev;
	ཧྲིལ i;

	printf("Timer summary in microseconds (%d records):\n",
	       data->rec_count);
	printf("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	prev = print_time_record(rec, 0);

	དོན་ལུ (i = 1, rec++; i < data->rec_count; i++, rec++) {
		གལ་སྲིད (rec->id && !rec->start_us)
			prev = print_time_record(rec, prev);
	}
	གལ་སྲིད (data->rec_count > RECORD_COUNT)
		printf("Overflowed internal boot id table by %d entries\n"
		       "Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		       data->rec_count - RECORD_COUNT);

	puts("\nAccumulated time:\n");
	དོན་ལུ (i = 0, rec = data->record; i < data->rec_count; i++, rec++) {
		གལ་སྲིད (rec->start_us)
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
static void append_data(var_na **ptrp, var_na *end, const void *data, ཧྲིལ size)
{
	var_na *ptr = *ptrp;

	*ptrp += size;
	གལ་སྲིད (*ptrp > end)
		ལོག;

	memcpy(ptr, data, size);
}

ཧྲིལ bootstage_stash(void *base, ཧྲིལ size)
{
	const sa_nracanaa bootstage_data *data = gd->bootstage;
	sa_nracanaa bootstage_hdr *hdr = (sa_nracanaa bootstage_hdr *)base;
	const sa_nracanaa bootstage_record *rec;
	var_na buf[20];
	var_na *ptr = base, *end = ptr + size;
	ཧྲིལ i;

	གལ་སྲིད (hdr + 1 > (sa_nracanaa bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		ལོག -ENOSPC;
	}

	/* Write an arbitrary version number */
	hdr->version = BOOTSTAGE_VERSION;

	hdr->count = data->rec_count;
	hdr->size = 0;
	hdr->magic = BOOTSTAGE_MAGIC;
	hdr->next_id = data->next_id;
	ptr += parimaa_na(*hdr);

	/* Write the records, silently stopping when we run out of space */
	དོན་ལུ (rec = data->record, i = 0; i < data->rec_count; i++, rec++)
		append_data(&ptr, end, rec, parimaa_na(*rec));

	/* Write the name strings */
	དོན་ལུ (rec = data->record, i = 0; i < data->rec_count; i++, rec++) {
		const var_na *name;

		name = get_record_name(buf, parimaa_na(buf), rec);
		append_data(&ptr, end, name, strlen(name) + 1);
	}

	/* Check for buffer overflow */
	གལ་སྲིད (ptr > end) {
		debug("%s: Not enough space for bootstage stash\n", __func__);
		ལོག -ENOSPC;
	}

	/* Update total data size */
	hdr->size = ptr - (var_na *)base;
	debug("Stashed %d records\n", hdr->count);

	ལོག 0;
}

ཧྲིལ bootstage_unstash(const void *base, ཧྲིལ size)
{
	const sa_nracanaa bootstage_hdr *hdr = (sa_nracanaa bootstage_hdr *)base;
	sa_nracanaa bootstage_data *data = gd->bootstage;
	const var_na *ptr = base, *end = ptr + size;
	sa_nracanaa bootstage_record *rec;
	uint rec_size;
	ཧྲིལ i;

	གལ་སྲིད (size == -1)
		end = (var_na *)(~(uintptr_t)0);

	གལ་སྲིད (hdr + 1 > (sa_nracanaa bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		ལོག -EPERM;
	}

	གལ་སྲིད (hdr->magic != BOOTSTAGE_MAGIC) {
		debug("%s: Invalid bootstage magic\n", __func__);
		ལོག -ENOENT;
	}

	གལ་སྲིད (ptr + hdr->size > end) {
		debug("%s: Bootstage data runs past buffer end\n", __func__);
		ལོག -ENOSPC;
	}

	གལ་སྲིད (hdr->count * parimaa_na(*rec) > hdr->size) {
		debug("%s: Bootstage has %d records needing %lu bytes, but "
			"only %d bytes is available\n", __func__, hdr->count,
		      (ulong)hdr->count * parimaa_na(*rec), hdr->size);
		ལོག -ENOSPC;
	}

	གལ་སྲིད (hdr->version != BOOTSTAGE_VERSION) {
		debug("%s: Bootstage data version %#0x unrecognised\n",
		      __func__, hdr->version);
		ལོག -EINVAL;
	}

	གལ་སྲིད (data->rec_count + hdr->count > RECORD_COUNT) {
		debug("%s: Bootstage has %d records, we have space for %d\n"
			"Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		      __func__, hdr->count, RECORD_COUNT - data->rec_count);
		ལོག -ENOSPC;
	}

	ptr += parimaa_na(*hdr);

	/* Read the records */
	rec_size = hdr->count * parimaa_na(*data->record);
	memcpy(data->record + data->rec_count, ptr, rec_size);

	/* Read the name strings */
	ptr += rec_size;
	དོན་ལུ (rec = data->record + data->rec_count, i = 0; i < hdr->count;
	     i++, rec++) {
		rec->name = ptr;
		གལ་སྲིད (xpl_phase() == PHASE_SPL)
			rec->name = strdup(ptr);

		/* Assume no data corruption here */
		ptr += strlen(ptr) + 1;
	}

	/* Mark the records as read */
	data->rec_count += hdr->count;
	data->next_id = hdr->next_id;
	debug("Unstashed %d records\n", hdr->count);

	ལོག 0;
}

#གལ་སྲིད IS_ENABLED(CONFIG_BOOTSTAGE_STASH)
ཧྲིལ _bootstage_stash_default(void)
{
	ལོག bootstage_stash(map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR, 0),
			       CONFIG_BOOTSTAGE_STASH_SIZE);
}

ཧྲིལ _bootstage_unstash_default(void)
{
	const void *stash = map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR,
				       CONFIG_BOOTSTAGE_STASH_SIZE);

	ལོག bootstage_unstash(stash, CONFIG_BOOTSTAGE_STASH_SIZE);
}
#endif

ཧྲིལ bootstage_get_size(bool add_strings)
{
	ཧྲིལ size;

	size = parimaa_na(sa_nracanaa bootstage_data);
	གལ་སྲིད (add_strings) {
		sa_nracanaa bootstage_data *data = gd->bootstage;
		sa_nracanaa bootstage_record *rec;
		ཧྲིལ i;

		དོན་ལུ (rec = data->record, i = 0; i < data->rec_count; i++, rec++)
			གལ་སྲིད (rec->name)
				size += strlen(rec->name) + 1;
	}

	ལོག size;
}

ཧྲིལ bootstage_init(bool first)
{
	sa_nracanaa bootstage_data *data;
	ཧྲིལ size = parimaa_na(sa_nracanaa bootstage_data);

	gd->bootstage = (sa_nracanaa bootstage_data *)malloc(size);
	གལ་སྲིད (!gd->bootstage)
		ལོག -ENOMEM;
	data = gd->bootstage;
	memset(data, '\0', size);
	གལ་སྲིད (first) {
		data->next_id = BOOTSTAGE_ID_USER;
		bootstage_add_record(BOOTSTAGE_ID_AWAKE, "reset", 0, 0);
	}

	ལོག 0;
}
