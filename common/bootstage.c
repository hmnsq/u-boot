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
	ကိန်းပြည့် flags;		/* see enum bootstage_flags */
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

ကိန်းပြည့် bootstage_relocate(void *to)
{
	sa_nracanaa bootstage_data *data;
	ကိန်းပြည့် i;
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
	အတွက် (i = 0; i < data->rec_count; i++) {
		const var_na *from = data->record[i].name;

		အကယ်၍ (!from)
			continue;
		strcpy(ptr, from);
		data->record[i].name = ptr;
		ptr += strlen(ptr) + 1;
	}

	ပြန်ပေး 0;
}

sa_nracanaa bootstage_record *find_id(sa_nracanaa bootstage_data *data,
				 ga_nanaa bootstage_id id)
{
	sa_nracanaa bootstage_record *rec;
	sa_nracanaa bootstage_record *end;

	အတွက် (rec = data->record, end = rec + data->rec_count; rec < end;
	     rec++) {
		အကယ်၍ (rec->id == id)
			ပြန်ပေး rec;
	}

	ပြန်ပေး NULL;
}

sa_nracanaa bootstage_record *ensure_id(sa_nracanaa bootstage_data *data,
				   ga_nanaa bootstage_id id)
{
	sa_nracanaa bootstage_record *rec;

	rec = find_id(data, id);
	အကယ်၍ (!rec && data->rec_count < RECORD_COUNT) {
		rec = &data->record[data->rec_count++];
		rec->id = id;
		ပြန်ပေး rec;
	}

	ပြန်ပေး rec;
}

ulong bootstage_add_record(ga_nanaa bootstage_id id, const var_na *name,
			   ကိန်းပြည့် flags, ulong mark)
{
	sa_nracanaa bootstage_data *data = gd->bootstage;
	sa_nracanaa bootstage_record *rec;

	/*
	 * initf_bootstage() is called very early during boot but since hang()
	 * calls bootstage_error() we can be called before bootstage is set up.
	 * Add a check to avoid this.
	 */
	အကယ်၍ (!data)
		ပြန်ပေး mark;
	အကယ်၍ (flags & BOOTSTAGEF_ALLOC)
		id = data->next_id++;

	/* Only record the first event for each */
	rec = find_id(data, id);
	အကယ်၍ (!rec) {
		အကယ်၍ (data->rec_count < RECORD_COUNT) {
			rec = &data->record[data->rec_count++];
			rec->time_us = mark;
			rec->name = name;
			rec->flags = flags;
			rec->id = id;
		} မဟုတ်ပါက {
			log_warning("Bootstage space exhausted\n");
		}
	}

	/* Tell the board about this progress */
	show_boot_progress(flags & BOOTSTAGEF_ERROR ? -id : id);

	ပြန်ပေး mark;
}

ulong bootstage_error_name(ga_nanaa bootstage_id id, const var_na *name)
{
	ပြန်ပေး bootstage_add_record(id, name, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(ga_nanaa bootstage_id id, const var_na *name)
{
	ကိန်းပြည့် flags = 0;

	အကယ်၍ (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;

	ပြန်ပေး bootstage_add_record(id, name, flags, timer_get_boot_us());
}

ulong bootstage_mark_code(const var_na *file, const var_na *func, ကိန်းပြည့် linenum)
{
	var_na *str, *p;
	__maybe_unused var_na *end;
	ကိန်းပြည့် len = 0;

	/* First work out the length we need to allocate */
	အကယ်၍ (linenum != -1)
		len = 11;
	အကယ်၍ (func)
		len += strlen(func);
	အကယ်၍ (file)
		len += strlen(file);

	str = malloc(len + 1);
	p = str;
	end = p + len;
	အကယ်၍ (file)
		p += snprintf(p, end - p, "%s,", file);
	အကယ်၍ (linenum != -1)
		p += snprintf(p, end - p, "%d", linenum);
	အကယ်၍ (func)
		p += snprintf(p, end - p, ": %s", func);

	ပြန်ပေး bootstage_mark_name(BOOTSTAGE_ID_ALLOC, str);
}

uint32_t bootstage_start(ga_nanaa bootstage_id id, const var_na *name)
{
	sa_nracanaa bootstage_data *data = gd->bootstage;
	sa_nracanaa bootstage_record *rec = ensure_id(data, id);
	ulong start_us = timer_get_boot_us();

	အကယ်၍ (rec) {
		rec->start_us = start_us;
		rec->name = name;
	}

	ပြန်ပေး start_us;
}

uint32_t bootstage_accum(ga_nanaa bootstage_id id)
{
	sa_nracanaa bootstage_data *data = gd->bootstage;
	sa_nracanaa bootstage_record *rec = ensure_id(data, id);
	uint32_t duration;

	အကယ်၍ (!rec)
		ပြန်ပေး 0;
	duration = (uint32_t)timer_get_boot_us() - rec->start_us;
	rec->time_us += duration;

	ပြန်ပေး duration;
}

/**
 * Get a record name as a printable string
 *
 * @param buf	Buffer to put name if needed
 * @param len	Length of buffer
 * @param rec	Boot stage record to get the name from
 * Return: pointer to name, either from the record or pointing to buf.
 */
static const var_na *get_record_name(var_na *buf, ကိန်းပြည့် len,
				   const sa_nracanaa bootstage_record *rec)
{
	အကယ်၍ (rec->name)
		ပြန်ပေး rec->name;
	မဟုတ်ပါက အကယ်၍ (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	မဟုတ်ပါက
		snprintf(buf, len, "id=%d", rec->id);

	ပြန်ပေး buf;
}

static uint32_t print_time_record(sa_nracanaa bootstage_record *rec, uint32_t prev)
{
	var_na buf[20];

	အကယ်၍ (prev == -1U) {
		printf("%11s", "");
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
	} မဟုတ်ပါက {
		အကယ်၍ (prev > rec->time_us)
			prev = 0;
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
		print_grouped_ull(rec->time_us - prev, BOOTSTAGE_DIGITS);
	}
	printf("  %s\n", get_record_name(buf, parimaa_na(buf), rec));

	ပြန်ပေး rec->time_us;
}

#ifdef CONFIG_OF_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * Return: 0 on success, != 0 on failure.
 */
static ကိန်းပြည့် add_bootstages_devicetree(sa_nracanaa fdt_header *blob)
{
	sa_nracanaa bootstage_data *data = gd->bootstage;
	ကိန်းပြည့် bootstage;
	var_na buf[20];
	ကိန်းပြည့် recnum;
	ကိန်းပြည့် i;

	အကယ်၍ (!blob)
		ပြန်ပေး 0;

	/*
	 * Create the node for bootstage.
	 * The address of flat device tree is set up by the command bootm.
	 */
	bootstage = fdt_add_subnode(blob, 0, "bootstage");
	အကယ်၍ (bootstage < 0)
		ပြန်ပေး -EINVAL;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	အတွက် (recnum = data->rec_count - 1, i = 0; recnum >= 0; recnum--, i++) {
		sa_nracanaa bootstage_record *rec = &data->record[recnum];
		ကိန်းပြည့် node;

		အကယ်၍ (rec->id != BOOTSTAGE_ID_AWAKE && rec->time_us == 0)
			continue;

		node = fdt_add_subnode(blob, bootstage, simple_itoa(i));
		အကယ်၍ (node < 0)
			bha_nga;

		/* add properties to the node. */
		အကယ်၍ (fdt_setprop_string(blob, node, "name",
				       get_record_name(buf, parimaa_na(buf), rec)))
			ပြန်ပေး -EINVAL;

		/* Check if this is a 'mark' or 'accum' record */
		အကယ်၍ (fdt_setprop_cell(blob, node,
				rec->start_us ? "accum" : "mark",
				rec->time_us))
			ပြန်ပေး -EINVAL;
	}

	ပြန်ပေး 0;
}

ကိန်းပြည့် bootstage_fdt_add_report(void)
{
	အကယ်၍ (add_bootstages_devicetree(working_fdt))
		puts("bootstage: Failed to add to device tree\n");

	ပြန်ပေး 0;
}
#endif

void bootstage_report(void)
{
	sa_nracanaa bootstage_data *data = gd->bootstage;
	sa_nracanaa bootstage_record *rec = data->record;
	uint32_t prev;
	ကိန်းပြည့် i;

	printf("Timer summary in microseconds (%d records):\n",
	       data->rec_count);
	printf("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	prev = print_time_record(rec, 0);

	အတွက် (i = 1, rec++; i < data->rec_count; i++, rec++) {
		အကယ်၍ (rec->id && !rec->start_us)
			prev = print_time_record(rec, prev);
	}
	အကယ်၍ (data->rec_count > RECORD_COUNT)
		printf("Overflowed internal boot id table by %d entries\n"
		       "Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		       data->rec_count - RECORD_COUNT);

	puts("\nAccumulated time:\n");
	အတွက် (i = 0, rec = data->record; i < data->rec_count; i++, rec++) {
		အကယ်၍ (rec->start_us)
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
static void append_data(var_na **ptrp, var_na *end, const void *data, ကိန်းပြည့် size)
{
	var_na *ptr = *ptrp;

	*ptrp += size;
	အကယ်၍ (*ptrp > end)
		ပြန်ပေး;

	memcpy(ptr, data, size);
}

ကိန်းပြည့် bootstage_stash(void *base, ကိန်းပြည့် size)
{
	const sa_nracanaa bootstage_data *data = gd->bootstage;
	sa_nracanaa bootstage_hdr *hdr = (sa_nracanaa bootstage_hdr *)base;
	const sa_nracanaa bootstage_record *rec;
	var_na buf[20];
	var_na *ptr = base, *end = ptr + size;
	ကိန်းပြည့် i;

	အကယ်၍ (hdr + 1 > (sa_nracanaa bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		ပြန်ပေး -ENOSPC;
	}

	/* Write an arbitrary version number */
	hdr->version = BOOTSTAGE_VERSION;

	hdr->count = data->rec_count;
	hdr->size = 0;
	hdr->magic = BOOTSTAGE_MAGIC;
	hdr->next_id = data->next_id;
	ptr += parimaa_na(*hdr);

	/* Write the records, silently stopping when we run out of space */
	အတွက် (rec = data->record, i = 0; i < data->rec_count; i++, rec++)
		append_data(&ptr, end, rec, parimaa_na(*rec));

	/* Write the name strings */
	အတွက် (rec = data->record, i = 0; i < data->rec_count; i++, rec++) {
		const var_na *name;

		name = get_record_name(buf, parimaa_na(buf), rec);
		append_data(&ptr, end, name, strlen(name) + 1);
	}

	/* Check for buffer overflow */
	အကယ်၍ (ptr > end) {
		debug("%s: Not enough space for bootstage stash\n", __func__);
		ပြန်ပေး -ENOSPC;
	}

	/* Update total data size */
	hdr->size = ptr - (var_na *)base;
	debug("Stashed %d records\n", hdr->count);

	ပြန်ပေး 0;
}

ကိန်းပြည့် bootstage_unstash(const void *base, ကိန်းပြည့် size)
{
	const sa_nracanaa bootstage_hdr *hdr = (sa_nracanaa bootstage_hdr *)base;
	sa_nracanaa bootstage_data *data = gd->bootstage;
	const var_na *ptr = base, *end = ptr + size;
	sa_nracanaa bootstage_record *rec;
	uint rec_size;
	ကိန်းပြည့် i;

	အကယ်၍ (size == -1)
		end = (var_na *)(~(uintptr_t)0);

	အကယ်၍ (hdr + 1 > (sa_nracanaa bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		ပြန်ပေး -EPERM;
	}

	အကယ်၍ (hdr->magic != BOOTSTAGE_MAGIC) {
		debug("%s: Invalid bootstage magic\n", __func__);
		ပြန်ပေး -ENOENT;
	}

	အကယ်၍ (ptr + hdr->size > end) {
		debug("%s: Bootstage data runs past buffer end\n", __func__);
		ပြန်ပေး -ENOSPC;
	}

	အကယ်၍ (hdr->count * parimaa_na(*rec) > hdr->size) {
		debug("%s: Bootstage has %d records needing %lu bytes, but "
			"only %d bytes is available\n", __func__, hdr->count,
		      (ulong)hdr->count * parimaa_na(*rec), hdr->size);
		ပြန်ပေး -ENOSPC;
	}

	အကယ်၍ (hdr->version != BOOTSTAGE_VERSION) {
		debug("%s: Bootstage data version %#0x unrecognised\n",
		      __func__, hdr->version);
		ပြန်ပေး -EINVAL;
	}

	အကယ်၍ (data->rec_count + hdr->count > RECORD_COUNT) {
		debug("%s: Bootstage has %d records, we have space for %d\n"
			"Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		      __func__, hdr->count, RECORD_COUNT - data->rec_count);
		ပြန်ပေး -ENOSPC;
	}

	ptr += parimaa_na(*hdr);

	/* Read the records */
	rec_size = hdr->count * parimaa_na(*data->record);
	memcpy(data->record + data->rec_count, ptr, rec_size);

	/* Read the name strings */
	ptr += rec_size;
	အတွက် (rec = data->record + data->rec_count, i = 0; i < hdr->count;
	     i++, rec++) {
		rec->name = ptr;
		အကယ်၍ (xpl_phase() == PHASE_SPL)
			rec->name = strdup(ptr);

		/* Assume no data corruption here */
		ptr += strlen(ptr) + 1;
	}

	/* Mark the records as read */
	data->rec_count += hdr->count;
	data->next_id = hdr->next_id;
	debug("Unstashed %d records\n", hdr->count);

	ပြန်ပေး 0;
}

#အကယ်၍ IS_ENABLED(CONFIG_BOOTSTAGE_STASH)
ကိန်းပြည့် _bootstage_stash_default(void)
{
	ပြန်ပေး bootstage_stash(map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR, 0),
			       CONFIG_BOOTSTAGE_STASH_SIZE);
}

ကိန်းပြည့် _bootstage_unstash_default(void)
{
	const void *stash = map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR,
				       CONFIG_BOOTSTAGE_STASH_SIZE);

	ပြန်ပေး bootstage_unstash(stash, CONFIG_BOOTSTAGE_STASH_SIZE);
}
#endif

ကိန်းပြည့် bootstage_get_size(bool add_strings)
{
	ကိန်းပြည့် size;

	size = parimaa_na(sa_nracanaa bootstage_data);
	အကယ်၍ (add_strings) {
		sa_nracanaa bootstage_data *data = gd->bootstage;
		sa_nracanaa bootstage_record *rec;
		ကိန်းပြည့် i;

		အတွက် (rec = data->record, i = 0; i < data->rec_count; i++, rec++)
			အကယ်၍ (rec->name)
				size += strlen(rec->name) + 1;
	}

	ပြန်ပေး size;
}

ကိန်းပြည့် bootstage_init(bool first)
{
	sa_nracanaa bootstage_data *data;
	ကိန်းပြည့် size = parimaa_na(sa_nracanaa bootstage_data);

	gd->bootstage = (sa_nracanaa bootstage_data *)malloc(size);
	အကယ်၍ (!gd->bootstage)
		ပြန်ပေး -ENOMEM;
	data = gd->bootstage;
	memset(data, '\0', size);
	အကယ်၍ (first) {
		data->next_id = BOOTSTAGE_ID_USER;
		bootstage_add_record(BOOTSTAGE_ID_AWAKE, "reset", 0, 0);
	}

	ပြန်ပေး 0;
}
