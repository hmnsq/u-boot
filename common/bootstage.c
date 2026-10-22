// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 */

/*
 * This module records the progress of boot and arbitrary commands, and
 * permits accurate timestamping of each.
 */

#define LOG_CATEGORY	LOGC_BOOT

#அந்தர்பாவ <bootstage.h>
#அந்தர்பாவ <hang.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <malloc.h>
#அந்தர்பாவ <sort.h>
#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <asm/global_data.h>
#அந்தர்பாவ <linux/compiler.h>
#அந்தர்பாவ <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

எண்ணிக்கை {
	RECORD_மொத்தம் = CONFIG_VAL(BOOTSTAGE_RECORD_மொத்தம்),
};

கட்டமைப்பு bootstage_record {
	ulong time_us;
	uint32_t start_us;
	மாறிலி எழுத்து *name;
	முழு flags;		/* see enum bootstage_flags */
	எண்ணிக்கை bootstage_id id;
};

கட்டமைப்பு bootstage_data {
	uint rec_மொத்தம்;
	uint next_id;
	கட்டமைப்பு bootstage_record record[RECORD_மொத்தம்];
};

எண்ணிக்கை {
	BOOTSTAGE_VERSION	= 0,
	BOOTSTAGE_MAGIC		= 0xb00757a3,
	BOOTSTAGE_DIGITS	= 9,
};

கட்டமைப்பு bootstage_hdr {
	u32 version;		/* BOOTSTAGE_VERSION */
	u32 மொத்தம்;		/* Number of records */
	u32 size;		/* Total data size (non-zero if valid) */
	u32 magic;		/* Magic number */
	u32 next_id;		/* Next ID to use for bootstage */
};

முழு bootstage_relocate(வெறுமை *to)
{
	கட்டமைப்பு bootstage_data *data;
	முழு i;
	எழுத்து *ptr;

	debug("Copying bootstage from %p to %p\n", gd->bootstage, to);
	memcpy(to, gd->bootstage, அளவு(கட்டமைப்பு bootstage_data));
	data = gd->bootstage = to;

	/* Figure out where to relocate the strings to */
	ptr = (எழுத்து *)(data + 1);

	/*
	 * Duplicate all strings.  They may point to an old location in the
	 * program .text section that can eventually get trashed.
	 */
	debug("Relocating %d records\n", data->rec_மொத்தம்);
	க்கு (i = 0; i < data->rec_மொத்தம்; i++) {
		மாறிலி எழுத்து *from = data->record[i].name;

		எனில் (!from)
			தொடர்;
		strcpy(ptr, from);
		data->record[i].name = ptr;
		ptr += strlen(ptr) + 1;
	}

	திரும்பு 0;
}

கட்டமைப்பு bootstage_record *find_id(கட்டமைப்பு bootstage_data *data,
				 எண்ணிக்கை bootstage_id id)
{
	கட்டமைப்பு bootstage_record *rec;
	கட்டமைப்பு bootstage_record *end;

	க்கு (rec = data->record, end = rec + data->rec_மொத்தம்; rec < end;
	     rec++) {
		எனில் (rec->id == id)
			திரும்பு rec;
	}

	திரும்பு வெற்று;
}

கட்டமைப்பு bootstage_record *ensure_id(கட்டமைப்பு bootstage_data *data,
				   எண்ணிக்கை bootstage_id id)
{
	கட்டமைப்பு bootstage_record *rec;

	rec = find_id(data, id);
	எனில் (!rec && data->rec_மொத்தம் < RECORD_மொத்தம்) {
		rec = &data->record[data->rec_மொத்தம்++];
		rec->id = id;
		திரும்பு rec;
	}

	திரும்பு rec;
}

ulong bootstage_add_record(எண்ணிக்கை bootstage_id id, மாறிலி எழுத்து *name,
			   முழு flags, ulong mark)
{
	கட்டமைப்பு bootstage_data *data = gd->bootstage;
	கட்டமைப்பு bootstage_record *rec;

	/*
	 * initf_bootstage() is called very early during boot but since hang()
	 * calls bootstage_error() we can be called before bootstage is set up.
	 * Add a check to avoid this.
	 */
	எனில் (!data)
		திரும்பு mark;
	எனில் (flags & BOOTSTAGEF_ALLOC)
		id = data->next_id++;

	/* Only record the first event for each */
	rec = find_id(data, id);
	எனில் (!rec) {
		எனில் (data->rec_மொத்தம் < RECORD_மொத்தம்) {
			rec = &data->record[data->rec_மொத்தம்++];
			rec->time_us = mark;
			rec->name = name;
			rec->flags = flags;
			rec->id = id;
		} இல்லையெனில் {
			log_warning("Bootstage space exhausted\n");
		}
	}

	/* Tell the board about this progress */
	show_boot_progress(flags & BOOTSTAGEF_ERROR ? -id : id);

	திரும்பு mark;
}

ulong bootstage_error_name(எண்ணிக்கை bootstage_id id, மாறிலி எழுத்து *name)
{
	திரும்பு bootstage_add_record(id, name, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(எண்ணிக்கை bootstage_id id, மாறிலி எழுத்து *name)
{
	முழு flags = 0;

	எனில் (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;

	திரும்பு bootstage_add_record(id, name, flags, timer_get_boot_us());
}

ulong bootstage_mark_code(மாறிலி எழுத்து *file, மாறிலி எழுத்து *func, முழு linenum)
{
	எழுத்து *str, *p;
	__maybe_unused எழுத்து *end;
	முழு len = 0;

	/* First work out the length we need to allocate */
	எனில் (linenum != -1)
		len = 11;
	எனில் (func)
		len += strlen(func);
	எனில் (file)
		len += strlen(file);

	str = malloc(len + 1);
	p = str;
	end = p + len;
	எனில் (file)
		p += snprintf(p, end - p, "%s,", file);
	எனில் (linenum != -1)
		p += snprintf(p, end - p, "%d", linenum);
	எனில் (func)
		p += snprintf(p, end - p, ": %s", func);

	திரும்பு bootstage_mark_name(BOOTSTAGE_ID_ALLOC, str);
}

uint32_t bootstage_start(எண்ணிக்கை bootstage_id id, மாறிலி எழுத்து *name)
{
	கட்டமைப்பு bootstage_data *data = gd->bootstage;
	கட்டமைப்பு bootstage_record *rec = ensure_id(data, id);
	ulong start_us = timer_get_boot_us();

	எனில் (rec) {
		rec->start_us = start_us;
		rec->name = name;
	}

	திரும்பு start_us;
}

uint32_t bootstage_accum(எண்ணிக்கை bootstage_id id)
{
	கட்டமைப்பு bootstage_data *data = gd->bootstage;
	கட்டமைப்பு bootstage_record *rec = ensure_id(data, id);
	uint32_t duration;

	எனில் (!rec)
		திரும்பு 0;
	duration = (uint32_t)timer_get_boot_us() - rec->start_us;
	rec->time_us += duration;

	திரும்பு duration;
}

/**
 * Get a record name as a printable string
 *
 * @param buf	Buffer to put name if needed
 * @param len	Length of buffer
 * @param rec	Boot stage record to get the name from
 * Return: pointer to name, either from the record or pointing to buf.
 */
நிலையான மாறிலி எழுத்து *get_record_name(எழுத்து *buf, முழு len,
				   மாறிலி கட்டமைப்பு bootstage_record *rec)
{
	எனில் (rec->name)
		திரும்பு rec->name;
	இல்லையெனில் எனில் (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	இல்லையெனில்
		snprintf(buf, len, "id=%d", rec->id);

	திரும்பு buf;
}

நிலையான uint32_t print_time_record(கட்டமைப்பு bootstage_record *rec, uint32_t prev)
{
	எழுத்து buf[20];

	எனில் (prev == -1U) {
		printf("%11s", "");
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
	} இல்லையெனில் {
		எனில் (prev > rec->time_us)
			prev = 0;
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
		print_grouped_ull(rec->time_us - prev, BOOTSTAGE_DIGITS);
	}
	printf("  %s\n", get_record_name(buf, அளவு(buf), rec));

	திரும்பு rec->time_us;
}

#ifdef CONFIG_OF_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * Return: 0 on success, != 0 on failure.
 */
நிலையான முழு add_bootstages_devicetree(கட்டமைப்பு fdt_header *blob)
{
	கட்டமைப்பு bootstage_data *data = gd->bootstage;
	முழு bootstage;
	எழுத்து buf[20];
	முழு recnum;
	முழு i;

	எனில் (!blob)
		திரும்பு 0;

	/*
	 * Create the node for bootstage.
	 * The address of flat device tree is set up by the command bootm.
	 */
	bootstage = fdt_add_subnode(blob, 0, "bootstage");
	எனில் (bootstage < 0)
		திரும்பு -EINVAL;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	க்கு (recnum = data->rec_மொத்தம் - 1, i = 0; recnum >= 0; recnum--, i++) {
		கட்டமைப்பு bootstage_record *rec = &data->record[recnum];
		முழு முனை;

		எனில் (rec->id != BOOTSTAGE_ID_AWAKE && rec->time_us == 0)
			தொடர்;

		முனை = fdt_add_subnode(blob, bootstage, simple_itoa(i));
		எனில் (முனை < 0)
			நிறுத்து;

		/* add properties to the node. */
		எனில் (fdt_setprop_string(blob, முனை, "name",
				       get_record_name(buf, அளவு(buf), rec)))
			திரும்பு -EINVAL;

		/* Check if this is a 'mark' or 'accum' record */
		எனில் (fdt_setprop_cell(blob, முனை,
				rec->start_us ? "accum" : "mark",
				rec->time_us))
			திரும்பு -EINVAL;
	}

	திரும்பு 0;
}

முழு bootstage_fdt_add_report(வெறுமை)
{
	எனில் (add_bootstages_devicetree(working_fdt))
		puts("bootstage: Failed to add to device tree\n");

	திரும்பு 0;
}
#endif

வெறுமை bootstage_report(வெறுமை)
{
	கட்டமைப்பு bootstage_data *data = gd->bootstage;
	கட்டமைப்பு bootstage_record *rec = data->record;
	uint32_t prev;
	முழு i;

	printf("Timer summary in microseconds (%d records):\n",
	       data->rec_மொத்தம்);
	printf("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	prev = print_time_record(rec, 0);

	க்கு (i = 1, rec++; i < data->rec_மொத்தம்; i++, rec++) {
		எனில் (rec->id && !rec->start_us)
			prev = print_time_record(rec, prev);
	}
	எனில் (data->rec_மொத்தம் > RECORD_மொத்தம்)
		printf("Overflowed internal boot id table by %d entries\n"
		       "Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		       data->rec_மொத்தம் - RECORD_மொத்தம்);

	puts("\nAccumulated time:\n");
	க்கு (i = 0, rec = data->record; i < data->rec_மொத்தம்; i++, rec++) {
		எனில் (rec->start_us)
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
நிலையான வெறுமை append_data(எழுத்து **ptrp, எழுத்து *end, மாறிலி வெறுமை *data, முழு size)
{
	எழுத்து *ptr = *ptrp;

	*ptrp += size;
	எனில் (*ptrp > end)
		திரும்பு;

	memcpy(ptr, data, size);
}

முழு bootstage_stash(வெறுமை *base, முழு size)
{
	மாறிலி கட்டமைப்பு bootstage_data *data = gd->bootstage;
	கட்டமைப்பு bootstage_hdr *hdr = (கட்டமைப்பு bootstage_hdr *)base;
	மாறிலி கட்டமைப்பு bootstage_record *rec;
	எழுத்து buf[20];
	எழுத்து *ptr = base, *end = ptr + size;
	முழு i;

	எனில் (hdr + 1 > (கட்டமைப்பு bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		திரும்பு -ENOSPC;
	}

	/* Write an arbitrary version number */
	hdr->version = BOOTSTAGE_VERSION;

	hdr->மொத்தம் = data->rec_மொத்தம்;
	hdr->size = 0;
	hdr->magic = BOOTSTAGE_MAGIC;
	hdr->next_id = data->next_id;
	ptr += அளவு(*hdr);

	/* Write the records, silently stopping when we run out of space */
	க்கு (rec = data->record, i = 0; i < data->rec_மொத்தம்; i++, rec++)
		append_data(&ptr, end, rec, அளவு(*rec));

	/* Write the name strings */
	க்கு (rec = data->record, i = 0; i < data->rec_மொத்தம்; i++, rec++) {
		மாறிலி எழுத்து *name;

		name = get_record_name(buf, அளவு(buf), rec);
		append_data(&ptr, end, name, strlen(name) + 1);
	}

	/* Check for buffer overflow */
	எனில் (ptr > end) {
		debug("%s: Not enough space for bootstage stash\n", __func__);
		திரும்பு -ENOSPC;
	}

	/* Update total data size */
	hdr->size = ptr - (எழுத்து *)base;
	debug("Stashed %d records\n", hdr->மொத்தம்);

	திரும்பு 0;
}

முழு bootstage_unstash(மாறிலி வெறுமை *base, முழு size)
{
	மாறிலி கட்டமைப்பு bootstage_hdr *hdr = (கட்டமைப்பு bootstage_hdr *)base;
	கட்டமைப்பு bootstage_data *data = gd->bootstage;
	மாறிலி எழுத்து *ptr = base, *end = ptr + size;
	கட்டமைப்பு bootstage_record *rec;
	uint rec_size;
	முழு i;

	எனில் (size == -1)
		end = (எழுத்து *)(~(uintptr_t)0);

	எனில் (hdr + 1 > (கட்டமைப்பு bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		திரும்பு -EPERM;
	}

	எனில் (hdr->magic != BOOTSTAGE_MAGIC) {
		debug("%s: Invalid bootstage magic\n", __func__);
		திரும்பு -ENOENT;
	}

	எனில் (ptr + hdr->size > end) {
		debug("%s: Bootstage data runs past buffer end\n", __func__);
		திரும்பு -ENOSPC;
	}

	எனில் (hdr->மொத்தம் * அளவு(*rec) > hdr->size) {
		debug("%s: Bootstage has %d records needing %lu bytes, but "
			"only %d bytes is available\n", __func__, hdr->மொத்தம்,
		      (ulong)hdr->மொத்தம் * அளவு(*rec), hdr->size);
		திரும்பு -ENOSPC;
	}

	எனில் (hdr->version != BOOTSTAGE_VERSION) {
		debug("%s: Bootstage data version %#0x unrecognised\n",
		      __func__, hdr->version);
		திரும்பு -EINVAL;
	}

	எனில் (data->rec_மொத்தம் + hdr->மொத்தம் > RECORD_மொத்தம்) {
		debug("%s: Bootstage has %d records, we have space for %d\n"
			"Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		      __func__, hdr->மொத்தம், RECORD_மொத்தம் - data->rec_மொத்தம்);
		திரும்பு -ENOSPC;
	}

	ptr += அளவு(*hdr);

	/* Read the records */
	rec_size = hdr->மொத்தம் * அளவு(*data->record);
	memcpy(data->record + data->rec_மொத்தம், ptr, rec_size);

	/* Read the name strings */
	ptr += rec_size;
	க்கு (rec = data->record + data->rec_மொத்தம், i = 0; i < hdr->மொத்தம்;
	     i++, rec++) {
		rec->name = ptr;
		எனில் (xpl_phase() == PHASE_SPL)
			rec->name = strdup(ptr);

		/* Assume no data corruption here */
		ptr += strlen(ptr) + 1;
	}

	/* Mark the records as read */
	data->rec_மொத்தம் += hdr->மொத்தம்;
	data->next_id = hdr->next_id;
	debug("Unstashed %d records\n", hdr->மொத்தம்);

	திரும்பு 0;
}

#எனில் IS_ENABLED(CONFIG_BOOTSTAGE_STASH)
முழு _bootstage_stash_default(வெறுமை)
{
	திரும்பு bootstage_stash(map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR, 0),
			       CONFIG_BOOTSTAGE_STASH_SIZE);
}

முழு _bootstage_unstash_default(வெறுமை)
{
	மாறிலி வெறுமை *stash = map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR,
				       CONFIG_BOOTSTAGE_STASH_SIZE);

	திரும்பு bootstage_unstash(stash, CONFIG_BOOTSTAGE_STASH_SIZE);
}
#endif

முழு bootstage_get_size(bool add_strings)
{
	முழு size;

	size = அளவு(கட்டமைப்பு bootstage_data);
	எனில் (add_strings) {
		கட்டமைப்பு bootstage_data *data = gd->bootstage;
		கட்டமைப்பு bootstage_record *rec;
		முழு i;

		க்கு (rec = data->record, i = 0; i < data->rec_மொத்தம்; i++, rec++)
			எனில் (rec->name)
				size += strlen(rec->name) + 1;
	}

	திரும்பு size;
}

முழு bootstage_init(bool first)
{
	கட்டமைப்பு bootstage_data *data;
	முழு size = அளவு(கட்டமைப்பு bootstage_data);

	gd->bootstage = (கட்டமைப்பு bootstage_data *)malloc(size);
	எனில் (!gd->bootstage)
		திரும்பு -ENOMEM;
	data = gd->bootstage;
	memset(data, '\0', size);
	எனில் (first) {
		data->next_id = BOOTSTAGE_ID_USER;
		bootstage_add_record(BOOTSTAGE_ID_AWAKE, "reset", 0, 0);
	}

	திரும்பு 0;
}
