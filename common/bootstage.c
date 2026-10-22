// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 */

/*
 * This module records the progress of boot and arbitrary commands, and
 * permits accurate timestamping of each.
 */

#define LOG_CATEGORY	LOGC_BOOT

#অন্তর্ভাৱ <bootstage.h>
#অন্তর্ভাৱ <hang.h>
#অন্তর্ভাৱ <log.h>
#অন্তর্ভাৱ <malloc.h>
#অন্তর্ভাৱ <sort.h>
#অন্তর্ভাৱ <spl.h>
#অন্তর্ভাৱ <asm/global_data.h>
#অন্তর্ভাৱ <linux/compiler.h>
#অন্তর্ভাৱ <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

গণনা {
	RECORD_গুনতি = CONFIG_VAL(BOOTSTAGE_RECORD_গুনতি),
};

সংরচনা bootstage_record {
	ulong time_us;
	uint32_t start_us;
	ধ্রুৱ ৱর্ণ *name;
	পূর্ণাঙ্ক flags;		/* see enum bootstage_flags */
	গণনা bootstage_id id;
};

সংরচনা bootstage_data {
	uint rec_গুনতি;
	uint next_id;
	সংরচনা bootstage_record record[RECORD_গুনতি];
};

গণনা {
	BOOTSTAGE_VERSION	= 0,
	BOOTSTAGE_MAGIC		= 0xb00757a3,
	BOOTSTAGE_DIGITS	= 9,
};

সংরচনা bootstage_hdr {
	u32 version;		/* BOOTSTAGE_VERSION */
	u32 গুনতি;		/* Number of records */
	u32 size;		/* Total data size (non-zero if valid) */
	u32 magic;		/* Magic number */
	u32 next_id;		/* Next ID to use for bootstage */
};

পূর্ণাঙ্ক bootstage_relocate(শূন্য *to)
{
	সংরচনা bootstage_data *data;
	পূর্ণাঙ্ক i;
	ৱর্ণ *ptr;

	debug("Copying bootstage from %p to %p\n", gd->bootstage, to);
	memcpy(to, gd->bootstage, পরিমাণ(সংরচনা bootstage_data));
	data = gd->bootstage = to;

	/* Figure out where to relocate the strings to */
	ptr = (ৱর্ণ *)(data + 1);

	/*
	 * Duplicate all strings.  They may point to an old location in the
	 * program .text section that can eventually get trashed.
	 */
	debug("Relocating %d records\n", data->rec_গুনতি);
	বাবে (i = 0; i < data->rec_গুনতি; i++) {
		ধ্রুৱ ৱর্ণ *from = data->record[i].name;

		যদি (!from)
			অনুৱর্ত;
		strcpy(ptr, from);
		data->record[i].name = ptr;
		ptr += strlen(ptr) + 1;
	}

	ঘূৰাই 0;
}

সংরচনা bootstage_record *find_id(সংরচনা bootstage_data *data,
				 গণনা bootstage_id id)
{
	সংরচনা bootstage_record *rec;
	সংরচনা bootstage_record *end;

	বাবে (rec = data->record, end = rec + data->rec_গুনতি; rec < end;
	     rec++) {
		যদি (rec->id == id)
			ঘূৰাই rec;
	}

	ঘূৰাই রিক্ত;
}

সংরচনা bootstage_record *ensure_id(সংরচনা bootstage_data *data,
				   গণনা bootstage_id id)
{
	সংরচনা bootstage_record *rec;

	rec = find_id(data, id);
	যদি (!rec && data->rec_গুনতি < RECORD_গুনতি) {
		rec = &data->record[data->rec_গুনতি++];
		rec->id = id;
		ঘূৰাই rec;
	}

	ঘূৰাই rec;
}

ulong bootstage_add_record(গণনা bootstage_id id, ধ্রুৱ ৱর্ণ *name,
			   পূর্ণাঙ্ক flags, ulong mark)
{
	সংরচনা bootstage_data *data = gd->bootstage;
	সংরচনা bootstage_record *rec;

	/*
	 * initf_bootstage() is called very early during boot but since hang()
	 * calls bootstage_error() we can be called before bootstage is set up.
	 * Add a check to avoid this.
	 */
	যদি (!data)
		ঘূৰাই mark;
	যদি (flags & BOOTSTAGEF_ALLOC)
		id = data->next_id++;

	/* Only record the first event for each */
	rec = find_id(data, id);
	যদি (!rec) {
		যদি (data->rec_গুনতি < RECORD_গুনতি) {
			rec = &data->record[data->rec_গুনতি++];
			rec->time_us = mark;
			rec->name = name;
			rec->flags = flags;
			rec->id = id;
		} অন্যথা {
			log_warning("Bootstage space exhausted\n");
		}
	}

	/* Tell the board about this progress */
	show_boot_progress(flags & BOOTSTAGEF_ERROR ? -id : id);

	ঘূৰাই mark;
}

ulong bootstage_error_name(গণনা bootstage_id id, ধ্রুৱ ৱর্ণ *name)
{
	ঘূৰাই bootstage_add_record(id, name, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(গণনা bootstage_id id, ধ্রুৱ ৱর্ণ *name)
{
	পূর্ণাঙ্ক flags = 0;

	যদি (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;

	ঘূৰাই bootstage_add_record(id, name, flags, timer_get_boot_us());
}

ulong bootstage_mark_code(ধ্রুৱ ৱর্ণ *file, ধ্রুৱ ৱর্ণ *func, পূর্ণাঙ্ক linenum)
{
	ৱর্ণ *str, *p;
	__maybe_unused ৱর্ণ *end;
	পূর্ণাঙ্ক len = 0;

	/* First work out the length we need to allocate */
	যদি (linenum != -1)
		len = 11;
	যদি (func)
		len += strlen(func);
	যদি (file)
		len += strlen(file);

	str = malloc(len + 1);
	p = str;
	end = p + len;
	যদি (file)
		p += snprintf(p, end - p, "%s,", file);
	যদি (linenum != -1)
		p += snprintf(p, end - p, "%d", linenum);
	যদি (func)
		p += snprintf(p, end - p, ": %s", func);

	ঘূৰাই bootstage_mark_name(BOOTSTAGE_ID_ALLOC, str);
}

uint32_t bootstage_start(গণনা bootstage_id id, ধ্রুৱ ৱর্ণ *name)
{
	সংরচনা bootstage_data *data = gd->bootstage;
	সংরচনা bootstage_record *rec = ensure_id(data, id);
	ulong start_us = timer_get_boot_us();

	যদি (rec) {
		rec->start_us = start_us;
		rec->name = name;
	}

	ঘূৰাই start_us;
}

uint32_t bootstage_accum(গণনা bootstage_id id)
{
	সংরচনা bootstage_data *data = gd->bootstage;
	সংরচনা bootstage_record *rec = ensure_id(data, id);
	uint32_t duration;

	যদি (!rec)
		ঘূৰাই 0;
	duration = (uint32_t)timer_get_boot_us() - rec->start_us;
	rec->time_us += duration;

	ঘূৰাই duration;
}

/**
 * Get a record name as a printable string
 *
 * @param buf	Buffer to put name if needed
 * @param len	Length of buffer
 * @param rec	Boot stage record to get the name from
 * Return: pointer to name, either from the record or pointing to buf.
 */
স্থির ধ্রুৱ ৱর্ণ *get_record_name(ৱর্ণ *buf, পূর্ণাঙ্ক len,
				   ধ্রুৱ সংরচনা bootstage_record *rec)
{
	যদি (rec->name)
		ঘূৰাই rec->name;
	অন্যথা যদি (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	অন্যথা
		snprintf(buf, len, "id=%d", rec->id);

	ঘূৰাই buf;
}

স্থির uint32_t print_time_record(সংরচনা bootstage_record *rec, uint32_t prev)
{
	ৱর্ণ buf[20];

	যদি (prev == -1U) {
		printf("%11s", "");
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
	} অন্যথা {
		যদি (prev > rec->time_us)
			prev = 0;
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
		print_grouped_ull(rec->time_us - prev, BOOTSTAGE_DIGITS);
	}
	printf("  %s\n", get_record_name(buf, পরিমাণ(buf), rec));

	ঘূৰাই rec->time_us;
}

#ifdef CONFIG_OF_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * Return: 0 on success, != 0 on failure.
 */
স্থির পূর্ণাঙ্ক add_bootstages_devicetree(সংরচনা fdt_header *blob)
{
	সংরচনা bootstage_data *data = gd->bootstage;
	পূর্ণাঙ্ক bootstage;
	ৱর্ণ buf[20];
	পূর্ণাঙ্ক recnum;
	পূর্ণাঙ্ক i;

	যদি (!blob)
		ঘূৰাই 0;

	/*
	 * Create the node for bootstage.
	 * The address of flat device tree is set up by the command bootm.
	 */
	bootstage = fdt_add_subnode(blob, 0, "bootstage");
	যদি (bootstage < 0)
		ঘূৰাই -EINVAL;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	বাবে (recnum = data->rec_গুনতি - 1, i = 0; recnum >= 0; recnum--, i++) {
		সংরচনা bootstage_record *rec = &data->record[recnum];
		পূর্ণাঙ্ক গ্ৰন্থি;

		যদি (rec->id != BOOTSTAGE_ID_AWAKE && rec->time_us == 0)
			অনুৱর্ত;

		গ্ৰন্থি = fdt_add_subnode(blob, bootstage, simple_itoa(i));
		যদি (গ্ৰন্থি < 0)
			ভঙ্গ;

		/* add properties to the node. */
		যদি (fdt_setprop_string(blob, গ্ৰন্থি, "name",
				       get_record_name(buf, পরিমাণ(buf), rec)))
			ঘূৰাই -EINVAL;

		/* Check if this is a 'mark' or 'accum' record */
		যদি (fdt_setprop_cell(blob, গ্ৰন্থি,
				rec->start_us ? "accum" : "mark",
				rec->time_us))
			ঘূৰাই -EINVAL;
	}

	ঘূৰাই 0;
}

পূর্ণাঙ্ক bootstage_fdt_add_report(শূন্য)
{
	যদি (add_bootstages_devicetree(working_fdt))
		puts("bootstage: Failed to add to device tree\n");

	ঘূৰাই 0;
}
#endif

শূন্য bootstage_report(শূন্য)
{
	সংরচনা bootstage_data *data = gd->bootstage;
	সংরচনা bootstage_record *rec = data->record;
	uint32_t prev;
	পূর্ণাঙ্ক i;

	printf("Timer summary in microseconds (%d records):\n",
	       data->rec_গুনতি);
	printf("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	prev = print_time_record(rec, 0);

	বাবে (i = 1, rec++; i < data->rec_গুনতি; i++, rec++) {
		যদি (rec->id && !rec->start_us)
			prev = print_time_record(rec, prev);
	}
	যদি (data->rec_গুনতি > RECORD_গুনতি)
		printf("Overflowed internal boot id table by %d entries\n"
		       "Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		       data->rec_গুনতি - RECORD_গুনতি);

	puts("\nAccumulated time:\n");
	বাবে (i = 0, rec = data->record; i < data->rec_গুনতি; i++, rec++) {
		যদি (rec->start_us)
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
স্থির শূন্য append_data(ৱর্ণ **ptrp, ৱর্ণ *end, ধ্রুৱ শূন্য *data, পূর্ণাঙ্ক size)
{
	ৱর্ণ *ptr = *ptrp;

	*ptrp += size;
	যদি (*ptrp > end)
		ঘূৰাই;

	memcpy(ptr, data, size);
}

পূর্ণাঙ্ক bootstage_stash(শূন্য *base, পূর্ণাঙ্ক size)
{
	ধ্রুৱ সংরচনা bootstage_data *data = gd->bootstage;
	সংরচনা bootstage_hdr *hdr = (সংরচনা bootstage_hdr *)base;
	ধ্রুৱ সংরচনা bootstage_record *rec;
	ৱর্ণ buf[20];
	ৱর্ণ *ptr = base, *end = ptr + size;
	পূর্ণাঙ্ক i;

	যদি (hdr + 1 > (সংরচনা bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		ঘূৰাই -ENOSPC;
	}

	/* Write an arbitrary version number */
	hdr->version = BOOTSTAGE_VERSION;

	hdr->গুনতি = data->rec_গুনতি;
	hdr->size = 0;
	hdr->magic = BOOTSTAGE_MAGIC;
	hdr->next_id = data->next_id;
	ptr += পরিমাণ(*hdr);

	/* Write the records, silently stopping when we run out of space */
	বাবে (rec = data->record, i = 0; i < data->rec_গুনতি; i++, rec++)
		append_data(&ptr, end, rec, পরিমাণ(*rec));

	/* Write the name strings */
	বাবে (rec = data->record, i = 0; i < data->rec_গুনতি; i++, rec++) {
		ধ্রুৱ ৱর্ণ *name;

		name = get_record_name(buf, পরিমাণ(buf), rec);
		append_data(&ptr, end, name, strlen(name) + 1);
	}

	/* Check for buffer overflow */
	যদি (ptr > end) {
		debug("%s: Not enough space for bootstage stash\n", __func__);
		ঘূৰাই -ENOSPC;
	}

	/* Update total data size */
	hdr->size = ptr - (ৱর্ণ *)base;
	debug("Stashed %d records\n", hdr->গুনতি);

	ঘূৰাই 0;
}

পূর্ণাঙ্ক bootstage_unstash(ধ্রুৱ শূন্য *base, পূর্ণাঙ্ক size)
{
	ধ্রুৱ সংরচনা bootstage_hdr *hdr = (সংরচনা bootstage_hdr *)base;
	সংরচনা bootstage_data *data = gd->bootstage;
	ধ্রুৱ ৱর্ণ *ptr = base, *end = ptr + size;
	সংরচনা bootstage_record *rec;
	uint rec_size;
	পূর্ণাঙ্ক i;

	যদি (size == -1)
		end = (ৱর্ণ *)(~(uintptr_t)0);

	যদি (hdr + 1 > (সংরচনা bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		ঘূৰাই -EPERM;
	}

	যদি (hdr->magic != BOOTSTAGE_MAGIC) {
		debug("%s: Invalid bootstage magic\n", __func__);
		ঘূৰাই -ENOENT;
	}

	যদি (ptr + hdr->size > end) {
		debug("%s: Bootstage data runs past buffer end\n", __func__);
		ঘূৰাই -ENOSPC;
	}

	যদি (hdr->গুনতি * পরিমাণ(*rec) > hdr->size) {
		debug("%s: Bootstage has %d records needing %lu bytes, but "
			"only %d bytes is available\n", __func__, hdr->গুনতি,
		      (ulong)hdr->গুনতি * পরিমাণ(*rec), hdr->size);
		ঘূৰাই -ENOSPC;
	}

	যদি (hdr->version != BOOTSTAGE_VERSION) {
		debug("%s: Bootstage data version %#0x unrecognised\n",
		      __func__, hdr->version);
		ঘূৰাই -EINVAL;
	}

	যদি (data->rec_গুনতি + hdr->গুনতি > RECORD_গুনতি) {
		debug("%s: Bootstage has %d records, we have space for %d\n"
			"Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		      __func__, hdr->গুনতি, RECORD_গুনতি - data->rec_গুনতি);
		ঘূৰাই -ENOSPC;
	}

	ptr += পরিমাণ(*hdr);

	/* Read the records */
	rec_size = hdr->গুনতি * পরিমাণ(*data->record);
	memcpy(data->record + data->rec_গুনতি, ptr, rec_size);

	/* Read the name strings */
	ptr += rec_size;
	বাবে (rec = data->record + data->rec_গুনতি, i = 0; i < hdr->গুনতি;
	     i++, rec++) {
		rec->name = ptr;
		যদি (xpl_phase() == PHASE_SPL)
			rec->name = strdup(ptr);

		/* Assume no data corruption here */
		ptr += strlen(ptr) + 1;
	}

	/* Mark the records as read */
	data->rec_গুনতি += hdr->গুনতি;
	data->next_id = hdr->next_id;
	debug("Unstashed %d records\n", hdr->গুনতি);

	ঘূৰাই 0;
}

#যদি IS_ENABLED(CONFIG_BOOTSTAGE_STASH)
পূর্ণাঙ্ক _bootstage_stash_default(শূন্য)
{
	ঘূৰাই bootstage_stash(map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR, 0),
			       CONFIG_BOOTSTAGE_STASH_SIZE);
}

পূর্ণাঙ্ক _bootstage_unstash_default(শূন্য)
{
	ধ্রুৱ শূন্য *stash = map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR,
				       CONFIG_BOOTSTAGE_STASH_SIZE);

	ঘূৰাই bootstage_unstash(stash, CONFIG_BOOTSTAGE_STASH_SIZE);
}
#endif

পূর্ণাঙ্ক bootstage_get_size(bool add_strings)
{
	পূর্ণাঙ্ক size;

	size = পরিমাণ(সংরচনা bootstage_data);
	যদি (add_strings) {
		সংরচনা bootstage_data *data = gd->bootstage;
		সংরচনা bootstage_record *rec;
		পূর্ণাঙ্ক i;

		বাবে (rec = data->record, i = 0; i < data->rec_গুনতি; i++, rec++)
			যদি (rec->name)
				size += strlen(rec->name) + 1;
	}

	ঘূৰাই size;
}

পূর্ণাঙ্ক bootstage_init(bool first)
{
	সংরচনা bootstage_data *data;
	পূর্ণাঙ্ক size = পরিমাণ(সংরচনা bootstage_data);

	gd->bootstage = (সংরচনা bootstage_data *)malloc(size);
	যদি (!gd->bootstage)
		ঘূৰাই -ENOMEM;
	data = gd->bootstage;
	memset(data, '\0', size);
	যদি (first) {
		data->next_id = BOOTSTAGE_ID_USER;
		bootstage_add_record(BOOTSTAGE_ID_AWAKE, "reset", 0, 0);
	}

	ঘূৰাই 0;
}
