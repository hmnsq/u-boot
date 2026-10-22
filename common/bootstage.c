// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 */

/*
 * This module records the progress of boot and arbitrary commands, and
 * permits accurate timestamping of each.
 */

#define LOG_CATEGORY	LOGC_BOOT

#ଅନ୍ତର୍ଭାଵ <bootstage.h>
#ଅନ୍ତର୍ଭାଵ <hang.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <malloc.h>
#ଅନ୍ତର୍ଭାଵ <sort.h>
#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>
#ଅନ୍ତର୍ଭାଵ <linux/compiler.h>
#ଅନ୍ତର୍ଭାଵ <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

ଗଣନା {
	RECORD_ଗଣନା = CONFIG_VAL(BOOTSTAGE_RECORD_ଗଣନା),
};

ସଂରଚନା bootstage_record {
	ulong time_us;
	uint32_t start_us;
	ଅଚଳ ଅକ୍ଷର *name;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags;		/* see enum bootstage_flags */
	ଗଣନା bootstage_id id;
};

ସଂରଚନା bootstage_data {
	uint rec_ଗଣନା;
	uint next_id;
	ସଂରଚନା bootstage_record record[RECORD_ଗଣନା];
};

ଗଣନା {
	BOOTSTAGE_VERSION	= 0,
	BOOTSTAGE_MAGIC		= 0xb00757a3,
	BOOTSTAGE_DIGITS	= 9,
};

ସଂରଚନା bootstage_hdr {
	u32 version;		/* BOOTSTAGE_VERSION */
	u32 ଗଣନା_2;		/* Number of records */
	u32 size;		/* Total data size (non-zero if valid) */
	u32 magic;		/* Magic number */
	u32 next_id;		/* Next ID to use for bootstage */
};

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootstage_relocate(ଶୂନ୍ୟ *to)
{
	ସଂରଚନା bootstage_data *data;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ଅକ୍ଷର *ptr;

	debug("Copying bootstage from %p to %p\n", gd->bootstage, to);
	memcpy(to, gd->bootstage, ଆକାର(ସଂରଚନା bootstage_data));
	data = gd->bootstage = to;

	/* Figure out where to relocate the strings to */
	ptr = (ଅକ୍ଷର *)(data + 1);

	/*
	 * Duplicate all strings.  They may point to an old location in the
	 * program .text section that can eventually get trashed.
	 */
	debug("Relocating %d records\n", data->rec_ଗଣନା);
	ପାଇଁ (i = 0; i < data->rec_ଗଣନା; i++) {
		ଅଚଳ ଅକ୍ଷର *from = data->record[i].name;

		ଯଦି (!from)
			ଜାରି;
		strcpy(ptr, from);
		data->record[i].name = ptr;
		ptr += strlen(ptr) + 1;
	}

	ଫେରନ୍ତୁ 0;
}

ସଂରଚନା bootstage_record *find_id(ସଂରଚନା bootstage_data *data,
				 ଗଣନା bootstage_id id)
{
	ସଂରଚନା bootstage_record *rec;
	ସଂରଚନା bootstage_record *end;

	ପାଇଁ (rec = data->record, end = rec + data->rec_ଗଣନା; rec < end;
	     rec++) {
		ଯଦି (rec->id == id)
			ଫେରନ୍ତୁ rec;
	}

	ଫେରନ୍ତୁ ଖାଲି;
}

ସଂରଚନା bootstage_record *ensure_id(ସଂରଚନା bootstage_data *data,
				   ଗଣନା bootstage_id id)
{
	ସଂରଚନା bootstage_record *rec;

	rec = find_id(data, id);
	ଯଦି (!rec && data->rec_ଗଣନା < RECORD_ଗଣନା) {
		rec = &data->record[data->rec_ଗଣନା++];
		rec->id = id;
		ଫେରନ୍ତୁ rec;
	}

	ଫେରନ୍ତୁ rec;
}

ulong bootstage_add_record(ଗଣନା bootstage_id id, ଅଚଳ ଅକ୍ଷର *name,
			   ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ulong mark)
{
	ସଂରଚନା bootstage_data *data = gd->bootstage;
	ସଂରଚନା bootstage_record *rec;

	/*
	 * initf_bootstage() is called very early during boot but since hang()
	 * calls bootstage_error() we can be called before bootstage is set up.
	 * Add a check to avoid this.
	 */
	ଯଦି (!data)
		ଫେରନ୍ତୁ mark;
	ଯଦି (flags & BOOTSTAGEF_ALLOC)
		id = data->next_id++;

	/* Only record the first event for each */
	rec = find_id(data, id);
	ଯଦି (!rec) {
		ଯଦି (data->rec_ଗଣନା < RECORD_ଗଣନା) {
			rec = &data->record[data->rec_ଗଣନା++];
			rec->time_us = mark;
			rec->name = name;
			rec->flags = flags;
			rec->id = id;
		} ଅନ୍ୟଥା {
			log_warning("Bootstage space exhausted\n");
		}
	}

	/* Tell the board about this progress */
	show_boot_progress(flags & BOOTSTAGEF_ERROR ? -id : id);

	ଫେରନ୍ତୁ mark;
}

ulong bootstage_error_name(ଗଣନା bootstage_id id, ଅଚଳ ଅକ୍ଷର *name)
{
	ଫେରନ୍ତୁ bootstage_add_record(id, name, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(ଗଣନା bootstage_id id, ଅଚଳ ଅକ୍ଷର *name)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = 0;

	ଯଦି (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;

	ଫେରନ୍ତୁ bootstage_add_record(id, name, flags, timer_get_boot_us());
}

ulong bootstage_mark_code(ଅଚଳ ଅକ୍ଷର *file, ଅଚଳ ଅକ୍ଷର *func, ପୂର୍ଣ୍ଣସଂଖ୍ୟା linenum)
{
	ଅକ୍ଷର *str, *p;
	__maybe_unused ଅକ୍ଷର *end;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len = 0;

	/* First work out the length we need to allocate */
	ଯଦି (linenum != -1)
		len = 11;
	ଯଦି (func)
		len += strlen(func);
	ଯଦି (file)
		len += strlen(file);

	str = malloc(len + 1);
	p = str;
	end = p + len;
	ଯଦି (file)
		p += snprintf(p, end - p, "%s,", file);
	ଯଦି (linenum != -1)
		p += snprintf(p, end - p, "%d", linenum);
	ଯଦି (func)
		p += snprintf(p, end - p, ": %s", func);

	ଫେରନ୍ତୁ bootstage_mark_name(BOOTSTAGE_ID_ALLOC, str);
}

uint32_t bootstage_start(ଗଣନା bootstage_id id, ଅଚଳ ଅକ୍ଷର *name)
{
	ସଂରଚନା bootstage_data *data = gd->bootstage;
	ସଂରଚନା bootstage_record *rec = ensure_id(data, id);
	ulong start_us = timer_get_boot_us();

	ଯଦି (rec) {
		rec->start_us = start_us;
		rec->name = name;
	}

	ଫେରନ୍ତୁ start_us;
}

uint32_t bootstage_accum(ଗଣନା bootstage_id id)
{
	ସଂରଚନା bootstage_data *data = gd->bootstage;
	ସଂରଚନା bootstage_record *rec = ensure_id(data, id);
	uint32_t duration;

	ଯଦି (!rec)
		ଫେରନ୍ତୁ 0;
	duration = (uint32_t)timer_get_boot_us() - rec->start_us;
	rec->time_us += duration;

	ଫେରନ୍ତୁ duration;
}

/**
 * Get a record name as a printable string
 *
 * @param buf	Buffer to put name if needed
 * @param len	Length of buffer
 * @param rec	Boot stage record to get the name from
 * Return: pointer to name, either from the record or pointing to buf.
 */
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *get_record_name(ଅକ୍ଷର *buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା len,
				   ଅଚଳ ସଂରଚନା bootstage_record *rec)
{
	ଯଦି (rec->name)
		ଫେରନ୍ତୁ rec->name;
	ଅନ୍ୟଥା ଯଦି (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	ଅନ୍ୟଥା
		snprintf(buf, len, "id=%d", rec->id);

	ଫେରନ୍ତୁ buf;
}

ସ୍ଥିର uint32_t print_time_record(ସଂରଚନା bootstage_record *rec, uint32_t prev)
{
	ଅକ୍ଷର buf[20];

	ଯଦି (prev == -1U) {
		printf("%11s", "");
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
	} ଅନ୍ୟଥା {
		ଯଦି (prev > rec->time_us)
			prev = 0;
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
		print_grouped_ull(rec->time_us - prev, BOOTSTAGE_DIGITS);
	}
	printf("  %s\n", get_record_name(buf, ଆକାର(buf), rec));

	ଫେରନ୍ତୁ rec->time_us;
}

#ifdef CONFIG_OF_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * Return: 0 on success, != 0 on failure.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା add_bootstages_devicetree(ସଂରଚନା fdt_header *blob)
{
	ସଂରଚନା bootstage_data *data = gd->bootstage;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootstage;
	ଅକ୍ଷର buf[20];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା recnum;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ଯଦି (!blob)
		ଫେରନ୍ତୁ 0;

	/*
	 * Create the node for bootstage.
	 * The address of flat device tree is set up by the command bootm.
	 */
	bootstage = fdt_add_subnode(blob, 0, "bootstage");
	ଯଦି (bootstage < 0)
		ଫେରନ୍ତୁ -EINVAL;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	ପାଇଁ (recnum = data->rec_ଗଣନା - 1, i = 0; recnum >= 0; recnum--, i++) {
		ସଂରଚନା bootstage_record *rec = &data->record[recnum];
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣ୍ଠି;

		ଯଦି (rec->id != BOOTSTAGE_ID_AWAKE && rec->time_us == 0)
			ଜାରି;

		ଗଣ୍ଠି = fdt_add_subnode(blob, bootstage, simple_itoa(i));
		ଯଦି (ଗଣ୍ଠି < 0)
			ଭାଙ୍ଗ;

		/* add properties to the node. */
		ଯଦି (fdt_setprop_string(blob, ଗଣ୍ଠି, "name",
				       get_record_name(buf, ଆକାର(buf), rec)))
			ଫେରନ୍ତୁ -EINVAL;

		/* Check if this is a 'mark' or 'accum' record */
		ଯଦି (fdt_setprop_cell(blob, ଗଣ୍ଠି,
				rec->start_us ? "accum" : "mark",
				rec->time_us))
			ଫେରନ୍ତୁ -EINVAL;
	}

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootstage_fdt_add_report(ଶୂନ୍ୟ)
{
	ଯଦି (add_bootstages_devicetree(working_fdt))
		puts("bootstage: Failed to add to device tree\n");

	ଫେରନ୍ତୁ 0;
}
#endif

ଶୂନ୍ୟ bootstage_report(ଶୂନ୍ୟ)
{
	ସଂରଚନା bootstage_data *data = gd->bootstage;
	ସଂରଚନା bootstage_record *rec = data->record;
	uint32_t prev;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	printf("Timer summary in microseconds (%d records):\n",
	       data->rec_ଗଣନା);
	printf("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	prev = print_time_record(rec, 0);

	ପାଇଁ (i = 1, rec++; i < data->rec_ଗଣନା; i++, rec++) {
		ଯଦି (rec->id && !rec->start_us)
			prev = print_time_record(rec, prev);
	}
	ଯଦି (data->rec_ଗଣନା > RECORD_ଗଣନା)
		printf("Overflowed internal boot id table by %d entries\n"
		       "Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		       data->rec_ଗଣନା - RECORD_ଗଣନା);

	puts("\nAccumulated time:\n");
	ପାଇଁ (i = 0, rec = data->record; i < data->rec_ଗଣନା; i++, rec++) {
		ଯଦି (rec->start_us)
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
ସ୍ଥିର ଶୂନ୍ୟ append_data(ଅକ୍ଷର **ptrp, ଅକ୍ଷର *end, ଅଚଳ ଶୂନ୍ୟ *data, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	ଅକ୍ଷର *ptr = *ptrp;

	*ptrp += size;
	ଯଦି (*ptrp > end)
		ଫେରନ୍ତୁ;

	memcpy(ptr, data, size);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootstage_stash(ଶୂନ୍ୟ *base, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	ଅଚଳ ସଂରଚନା bootstage_data *data = gd->bootstage;
	ସଂରଚନା bootstage_hdr *hdr = (ସଂରଚନା bootstage_hdr *)base;
	ଅଚଳ ସଂରଚନା bootstage_record *rec;
	ଅକ୍ଷର buf[20];
	ଅକ୍ଷର *ptr = base, *end = ptr + size;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ଯଦି (hdr + 1 > (ସଂରଚନା bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		ଫେରନ୍ତୁ -ENOSPC;
	}

	/* Write an arbitrary version number */
	hdr->version = BOOTSTAGE_VERSION;

	hdr->ଗଣନା_2 = data->rec_ଗଣନା;
	hdr->size = 0;
	hdr->magic = BOOTSTAGE_MAGIC;
	hdr->next_id = data->next_id;
	ptr += ଆକାର(*hdr);

	/* Write the records, silently stopping when we run out of space */
	ପାଇଁ (rec = data->record, i = 0; i < data->rec_ଗଣନା; i++, rec++)
		append_data(&ptr, end, rec, ଆକାର(*rec));

	/* Write the name strings */
	ପାଇଁ (rec = data->record, i = 0; i < data->rec_ଗଣନା; i++, rec++) {
		ଅଚଳ ଅକ୍ଷର *name;

		name = get_record_name(buf, ଆକାର(buf), rec);
		append_data(&ptr, end, name, strlen(name) + 1);
	}

	/* Check for buffer overflow */
	ଯଦି (ptr > end) {
		debug("%s: Not enough space for bootstage stash\n", __func__);
		ଫେରନ୍ତୁ -ENOSPC;
	}

	/* Update total data size */
	hdr->size = ptr - (ଅକ୍ଷର *)base;
	debug("Stashed %d records\n", hdr->ଗଣନା_2);

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootstage_unstash(ଅଚଳ ଶୂନ୍ୟ *base, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	ଅଚଳ ସଂରଚନା bootstage_hdr *hdr = (ସଂରଚନା bootstage_hdr *)base;
	ସଂରଚନା bootstage_data *data = gd->bootstage;
	ଅଚଳ ଅକ୍ଷର *ptr = base, *end = ptr + size;
	ସଂରଚନା bootstage_record *rec;
	uint rec_size;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ଯଦି (size == -1)
		end = (ଅକ୍ଷର *)(~(uintptr_t)0);

	ଯଦି (hdr + 1 > (ସଂରଚନା bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		ଫେରନ୍ତୁ -EPERM;
	}

	ଯଦି (hdr->magic != BOOTSTAGE_MAGIC) {
		debug("%s: Invalid bootstage magic\n", __func__);
		ଫେରନ୍ତୁ -ENOENT;
	}

	ଯଦି (ptr + hdr->size > end) {
		debug("%s: Bootstage data runs past buffer end\n", __func__);
		ଫେରନ୍ତୁ -ENOSPC;
	}

	ଯଦି (hdr->ଗଣନା_2 * ଆକାର(*rec) > hdr->size) {
		debug("%s: Bootstage has %d records needing %lu bytes, but "
			"only %d bytes is available\n", __func__, hdr->ଗଣନା_2,
		      (ulong)hdr->ଗଣନା_2 * ଆକାର(*rec), hdr->size);
		ଫେରନ୍ତୁ -ENOSPC;
	}

	ଯଦି (hdr->version != BOOTSTAGE_VERSION) {
		debug("%s: Bootstage data version %#0x unrecognised\n",
		      __func__, hdr->version);
		ଫେରନ୍ତୁ -EINVAL;
	}

	ଯଦି (data->rec_ଗଣନା + hdr->ଗଣନା_2 > RECORD_ଗଣନା) {
		debug("%s: Bootstage has %d records, we have space for %d\n"
			"Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		      __func__, hdr->ଗଣନା_2, RECORD_ଗଣନା - data->rec_ଗଣନା);
		ଫେରନ୍ତୁ -ENOSPC;
	}

	ptr += ଆକାର(*hdr);

	/* Read the records */
	rec_size = hdr->ଗଣନା_2 * ଆକାର(*data->record);
	memcpy(data->record + data->rec_ଗଣନା, ptr, rec_size);

	/* Read the name strings */
	ptr += rec_size;
	ପାଇଁ (rec = data->record + data->rec_ଗଣନା, i = 0; i < hdr->ଗଣନା_2;
	     i++, rec++) {
		rec->name = ptr;
		ଯଦି (xpl_phase() == PHASE_SPL)
			rec->name = strdup(ptr);

		/* Assume no data corruption here */
		ptr += strlen(ptr) + 1;
	}

	/* Mark the records as read */
	data->rec_ଗଣନା += hdr->ଗଣନା_2;
	data->next_id = hdr->next_id;
	debug("Unstashed %d records\n", hdr->ଗଣନା_2);

	ଫେରନ୍ତୁ 0;
}

#ଯଦି IS_ENABLED(CONFIG_BOOTSTAGE_STASH)
ପୂର୍ଣ୍ଣସଂଖ୍ୟା _bootstage_stash_default(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ bootstage_stash(map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR, 0),
			       CONFIG_BOOTSTAGE_STASH_SIZE);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା _bootstage_unstash_default(ଶୂନ୍ୟ)
{
	ଅଚଳ ଶୂନ୍ୟ *stash = map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR,
				       CONFIG_BOOTSTAGE_STASH_SIZE);

	ଫେରନ୍ତୁ bootstage_unstash(stash, CONFIG_BOOTSTAGE_STASH_SIZE);
}
#endif

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootstage_get_size(bool add_strings)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା size;

	size = ଆକାର(ସଂରଚନା bootstage_data);
	ଯଦି (add_strings) {
		ସଂରଚନା bootstage_data *data = gd->bootstage;
		ସଂରଚନା bootstage_record *rec;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

		ପାଇଁ (rec = data->record, i = 0; i < data->rec_ଗଣନା; i++, rec++)
			ଯଦି (rec->name)
				size += strlen(rec->name) + 1;
	}

	ଫେରନ୍ତୁ size;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bootstage_init(bool first)
{
	ସଂରଚନା bootstage_data *data;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା size = ଆକାର(ସଂରଚନା bootstage_data);

	gd->bootstage = (ସଂରଚନା bootstage_data *)malloc(size);
	ଯଦି (!gd->bootstage)
		ଫେରନ୍ତୁ -ENOMEM;
	data = gd->bootstage;
	memset(data, '\0', size);
	ଯଦି (first) {
		data->next_id = BOOTSTAGE_ID_USER;
		bootstage_add_record(BOOTSTAGE_ID_AWAKE, "reset", 0, 0);
	}

	ଫେରନ୍ତୁ 0;
}
