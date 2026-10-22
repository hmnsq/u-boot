// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 */

/*
 * This module records the progress of boot and arbitrary commands, and
 * permits accurate timestamping of each.
 */

#define LOG_CATEGORY	LOGC_BOOT

#شامل <bootstage.h>
#شامل <hang.h>
#شامل <log.h>
#شامل <malloc.h>
#شامل <sort.h>
#شامل <spl.h>
#شامل <asm/global_data.h>
#شامل <linux/compiler.h>
#شامل <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

شمارش {
	RECORD_شمار = CONFIG_VAL(BOOTSTAGE_RECORD_شمار),
};

ساختار bootstage_record {
	ulong time_us;
	uint32_t start_us;
	ثابت نویسه *name;
	صحیح flags;		/* see enum bootstage_flags */
	شمارش bootstage_id id;
};

ساختار bootstage_data {
	uint rec_شمار;
	uint next_id;
	ساختار bootstage_record record[RECORD_شمار];
};

شمارش {
	BOOTSTAGE_VERSION	= 0,
	BOOTSTAGE_MAGIC		= 0xb00757a3,
	BOOTSTAGE_DIGITS	= 9,
};

ساختار bootstage_hdr {
	u32 version;		/* BOOTSTAGE_VERSION */
	u32 شمار;		/* Number of records */
	u32 size;		/* Total data size (non-zero if valid) */
	u32 magic;		/* Magic number */
	u32 next_id;		/* Next ID to use for bootstage */
};

صحیح bootstage_relocate(تهی *to)
{
	ساختار bootstage_data *data;
	صحیح i;
	نویسه *ptr;

	debug("Copying bootstage from %p to %p\n", gd->bootstage, to);
	memcpy(to, gd->bootstage, اندازه(ساختار bootstage_data));
	data = gd->bootstage = to;

	/* Figure out where to relocate the strings to */
	ptr = (نویسه *)(data + 1);

	/*
	 * Duplicate all strings.  They may point to an old location in the
	 * program .text section that can eventually get trashed.
	 */
	debug("Relocating %d records\n", data->rec_شمار);
	برای (i = 0; i < data->rec_شمار; i++) {
		ثابت نویسه *from = data->record[i].name;

		اگر (!from)
			ادامه;
		strcpy(ptr, from);
		data->record[i].name = ptr;
		ptr += strlen(ptr) + 1;
	}

	بازگشت 0;
}

ساختار bootstage_record *find_id(ساختار bootstage_data *data,
				 شمارش bootstage_id id)
{
	ساختار bootstage_record *rec;
	ساختار bootstage_record *end;

	برای (rec = data->record, end = rec + data->rec_شمار; rec < end;
	     rec++) {
		اگر (rec->id == id)
			بازگشت rec;
	}

	بازگشت پوچ;
}

ساختار bootstage_record *ensure_id(ساختار bootstage_data *data,
				   شمارش bootstage_id id)
{
	ساختار bootstage_record *rec;

	rec = find_id(data, id);
	اگر (!rec && data->rec_شمار < RECORD_شمار) {
		rec = &data->record[data->rec_شمار++];
		rec->id = id;
		بازگشت rec;
	}

	بازگشت rec;
}

ulong bootstage_add_record(شمارش bootstage_id id, ثابت نویسه *name,
			   صحیح flags, ulong mark)
{
	ساختار bootstage_data *data = gd->bootstage;
	ساختار bootstage_record *rec;

	/*
	 * initf_bootstage() is called very early during boot but since hang()
	 * calls bootstage_error() we can be called before bootstage is set up.
	 * Add a check to avoid this.
	 */
	اگر (!data)
		بازگشت mark;
	اگر (flags & BOOTSTAGEF_ALLOC)
		id = data->next_id++;

	/* Only record the first event for each */
	rec = find_id(data, id);
	اگر (!rec) {
		اگر (data->rec_شمار < RECORD_شمار) {
			rec = &data->record[data->rec_شمار++];
			rec->time_us = mark;
			rec->name = name;
			rec->flags = flags;
			rec->id = id;
		} وگرنه {
			log_warning("Bootstage space exhausted\n");
		}
	}

	/* Tell the board about this progress */
	show_boot_progress(flags & BOOTSTAGEF_ERROR ? -id : id);

	بازگشت mark;
}

ulong bootstage_error_name(شمارش bootstage_id id, ثابت نویسه *name)
{
	بازگشت bootstage_add_record(id, name, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(شمارش bootstage_id id, ثابت نویسه *name)
{
	صحیح flags = 0;

	اگر (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;

	بازگشت bootstage_add_record(id, name, flags, timer_get_boot_us());
}

ulong bootstage_mark_code(ثابت نویسه *file, ثابت نویسه *func, صحیح linenum)
{
	نویسه *str, *p;
	__maybe_unused نویسه *end;
	صحیح len = 0;

	/* First work out the length we need to allocate */
	اگر (linenum != -1)
		len = 11;
	اگر (func)
		len += strlen(func);
	اگر (file)
		len += strlen(file);

	str = malloc(len + 1);
	p = str;
	end = p + len;
	اگر (file)
		p += snprintf(p, end - p, "%s,", file);
	اگر (linenum != -1)
		p += snprintf(p, end - p, "%d", linenum);
	اگر (func)
		p += snprintf(p, end - p, ": %s", func);

	بازگشت bootstage_mark_name(BOOTSTAGE_ID_ALLOC, str);
}

uint32_t bootstage_start(شمارش bootstage_id id, ثابت نویسه *name)
{
	ساختار bootstage_data *data = gd->bootstage;
	ساختار bootstage_record *rec = ensure_id(data, id);
	ulong start_us = timer_get_boot_us();

	اگر (rec) {
		rec->start_us = start_us;
		rec->name = name;
	}

	بازگشت start_us;
}

uint32_t bootstage_accum(شمارش bootstage_id id)
{
	ساختار bootstage_data *data = gd->bootstage;
	ساختار bootstage_record *rec = ensure_id(data, id);
	uint32_t duration;

	اگر (!rec)
		بازگشت 0;
	duration = (uint32_t)timer_get_boot_us() - rec->start_us;
	rec->time_us += duration;

	بازگشت duration;
}

/**
 * Get a record name as a printable string
 *
 * @param buf	Buffer to put name if needed
 * @param len	Length of buffer
 * @param rec	Boot stage record to get the name from
 * Return: pointer to name, either from the record or pointing to buf.
 */
ایستا ثابت نویسه *get_record_name(نویسه *buf, صحیح len,
				   ثابت ساختار bootstage_record *rec)
{
	اگر (rec->name)
		بازگشت rec->name;
	وگرنه اگر (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	وگرنه
		snprintf(buf, len, "id=%d", rec->id);

	بازگشت buf;
}

ایستا uint32_t print_time_record(ساختار bootstage_record *rec, uint32_t prev)
{
	نویسه buf[20];

	اگر (prev == -1U) {
		printf("%11s", "");
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
	} وگرنه {
		اگر (prev > rec->time_us)
			prev = 0;
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
		print_grouped_ull(rec->time_us - prev, BOOTSTAGE_DIGITS);
	}
	printf("  %s\n", get_record_name(buf, اندازه(buf), rec));

	بازگشت rec->time_us;
}

#ifdef CONFIG_OF_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * Return: 0 on success, != 0 on failure.
 */
ایستا صحیح add_bootstages_devicetree(ساختار fdt_header *blob)
{
	ساختار bootstage_data *data = gd->bootstage;
	صحیح bootstage;
	نویسه buf[20];
	صحیح recnum;
	صحیح i;

	اگر (!blob)
		بازگشت 0;

	/*
	 * Create the node for bootstage.
	 * The address of flat device tree is set up by the command bootm.
	 */
	bootstage = fdt_add_subnode(blob, 0, "bootstage");
	اگر (bootstage < 0)
		بازگشت -EINVAL;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	برای (recnum = data->rec_شمار - 1, i = 0; recnum >= 0; recnum--, i++) {
		ساختار bootstage_record *rec = &data->record[recnum];
		صحیح گره;

		اگر (rec->id != BOOTSTAGE_ID_AWAKE && rec->time_us == 0)
			ادامه;

		گره = fdt_add_subnode(blob, bootstage, simple_itoa(i));
		اگر (گره < 0)
			بشکن;

		/* add properties to the node. */
		اگر (fdt_setprop_string(blob, گره, "name",
				       get_record_name(buf, اندازه(buf), rec)))
			بازگشت -EINVAL;

		/* Check if this is a 'mark' or 'accum' record */
		اگر (fdt_setprop_cell(blob, گره,
				rec->start_us ? "accum" : "mark",
				rec->time_us))
			بازگشت -EINVAL;
	}

	بازگشت 0;
}

صحیح bootstage_fdt_add_report(تهی)
{
	اگر (add_bootstages_devicetree(working_fdt))
		puts("bootstage: Failed to add to device tree\n");

	بازگشت 0;
}
#endif

تهی bootstage_report(تهی)
{
	ساختار bootstage_data *data = gd->bootstage;
	ساختار bootstage_record *rec = data->record;
	uint32_t prev;
	صحیح i;

	printf("Timer summary in microseconds (%d records):\n",
	       data->rec_شمار);
	printf("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	prev = print_time_record(rec, 0);

	برای (i = 1, rec++; i < data->rec_شمار; i++, rec++) {
		اگر (rec->id && !rec->start_us)
			prev = print_time_record(rec, prev);
	}
	اگر (data->rec_شمار > RECORD_شمار)
		printf("Overflowed internal boot id table by %d entries\n"
		       "Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		       data->rec_شمار - RECORD_شمار);

	puts("\nAccumulated time:\n");
	برای (i = 0, rec = data->record; i < data->rec_شمار; i++, rec++) {
		اگر (rec->start_us)
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
ایستا تهی append_data(نویسه **ptrp, نویسه *end, ثابت تهی *data, صحیح size)
{
	نویسه *ptr = *ptrp;

	*ptrp += size;
	اگر (*ptrp > end)
		بازگشت;

	memcpy(ptr, data, size);
}

صحیح bootstage_stash(تهی *base, صحیح size)
{
	ثابت ساختار bootstage_data *data = gd->bootstage;
	ساختار bootstage_hdr *hdr = (ساختار bootstage_hdr *)base;
	ثابت ساختار bootstage_record *rec;
	نویسه buf[20];
	نویسه *ptr = base, *end = ptr + size;
	صحیح i;

	اگر (hdr + 1 > (ساختار bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		بازگشت -ENOSPC;
	}

	/* Write an arbitrary version number */
	hdr->version = BOOTSTAGE_VERSION;

	hdr->شمار = data->rec_شمار;
	hdr->size = 0;
	hdr->magic = BOOTSTAGE_MAGIC;
	hdr->next_id = data->next_id;
	ptr += اندازه(*hdr);

	/* Write the records, silently stopping when we run out of space */
	برای (rec = data->record, i = 0; i < data->rec_شمار; i++, rec++)
		append_data(&ptr, end, rec, اندازه(*rec));

	/* Write the name strings */
	برای (rec = data->record, i = 0; i < data->rec_شمار; i++, rec++) {
		ثابت نویسه *name;

		name = get_record_name(buf, اندازه(buf), rec);
		append_data(&ptr, end, name, strlen(name) + 1);
	}

	/* Check for buffer overflow */
	اگر (ptr > end) {
		debug("%s: Not enough space for bootstage stash\n", __func__);
		بازگشت -ENOSPC;
	}

	/* Update total data size */
	hdr->size = ptr - (نویسه *)base;
	debug("Stashed %d records\n", hdr->شمار);

	بازگشت 0;
}

صحیح bootstage_unstash(ثابت تهی *base, صحیح size)
{
	ثابت ساختار bootstage_hdr *hdr = (ساختار bootstage_hdr *)base;
	ساختار bootstage_data *data = gd->bootstage;
	ثابت نویسه *ptr = base, *end = ptr + size;
	ساختار bootstage_record *rec;
	uint rec_size;
	صحیح i;

	اگر (size == -1)
		end = (نویسه *)(~(uintptr_t)0);

	اگر (hdr + 1 > (ساختار bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		بازگشت -EPERM;
	}

	اگر (hdr->magic != BOOTSTAGE_MAGIC) {
		debug("%s: Invalid bootstage magic\n", __func__);
		بازگشت -ENOENT;
	}

	اگر (ptr + hdr->size > end) {
		debug("%s: Bootstage data runs past buffer end\n", __func__);
		بازگشت -ENOSPC;
	}

	اگر (hdr->شمار * اندازه(*rec) > hdr->size) {
		debug("%s: Bootstage has %d records needing %lu bytes, but "
			"only %d bytes is available\n", __func__, hdr->شمار,
		      (ulong)hdr->شمار * اندازه(*rec), hdr->size);
		بازگشت -ENOSPC;
	}

	اگر (hdr->version != BOOTSTAGE_VERSION) {
		debug("%s: Bootstage data version %#0x unrecognised\n",
		      __func__, hdr->version);
		بازگشت -EINVAL;
	}

	اگر (data->rec_شمار + hdr->شمار > RECORD_شمار) {
		debug("%s: Bootstage has %d records, we have space for %d\n"
			"Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		      __func__, hdr->شمار, RECORD_شمار - data->rec_شمار);
		بازگشت -ENOSPC;
	}

	ptr += اندازه(*hdr);

	/* Read the records */
	rec_size = hdr->شمار * اندازه(*data->record);
	memcpy(data->record + data->rec_شمار, ptr, rec_size);

	/* Read the name strings */
	ptr += rec_size;
	برای (rec = data->record + data->rec_شمار, i = 0; i < hdr->شمار;
	     i++, rec++) {
		rec->name = ptr;
		اگر (xpl_phase() == PHASE_SPL)
			rec->name = strdup(ptr);

		/* Assume no data corruption here */
		ptr += strlen(ptr) + 1;
	}

	/* Mark the records as read */
	data->rec_شمار += hdr->شمار;
	data->next_id = hdr->next_id;
	debug("Unstashed %d records\n", hdr->شمار);

	بازگشت 0;
}

#اگر IS_ENABLED(CONFIG_BOOTSTAGE_STASH)
صحیح _bootstage_stash_default(تهی)
{
	بازگشت bootstage_stash(map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR, 0),
			       CONFIG_BOOTSTAGE_STASH_SIZE);
}

صحیح _bootstage_unstash_default(تهی)
{
	ثابت تهی *stash = map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR,
				       CONFIG_BOOTSTAGE_STASH_SIZE);

	بازگشت bootstage_unstash(stash, CONFIG_BOOTSTAGE_STASH_SIZE);
}
#endif

صحیح bootstage_get_size(bool add_strings)
{
	صحیح size;

	size = اندازه(ساختار bootstage_data);
	اگر (add_strings) {
		ساختار bootstage_data *data = gd->bootstage;
		ساختار bootstage_record *rec;
		صحیح i;

		برای (rec = data->record, i = 0; i < data->rec_شمار; i++, rec++)
			اگر (rec->name)
				size += strlen(rec->name) + 1;
	}

	بازگشت size;
}

صحیح bootstage_init(bool first)
{
	ساختار bootstage_data *data;
	صحیح size = اندازه(ساختار bootstage_data);

	gd->bootstage = (ساختار bootstage_data *)malloc(size);
	اگر (!gd->bootstage)
		بازگشت -ENOMEM;
	data = gd->bootstage;
	memset(data, '\0', size);
	اگر (first) {
		data->next_id = BOOTSTAGE_ID_USER;
		bootstage_add_record(BOOTSTAGE_ID_AWAKE, "reset", 0, 0);
	}

	بازگشت 0;
}
