// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 */

/*
 * This module records the progress of boot and arbitrary commands, and
 * permits accurate timestamping of each.
 */

#define LOG_CATEGORY	LOGC_BOOT

#ضمن <bootstage.h>
#ضمن <hang.h>
#ضمن <log.h>
#ضمن <malloc.h>
#ضمن <sort.h>
#ضمن <spl.h>
#ضمن <asm/global_data.h>
#ضمن <linux/compiler.h>
#ضمن <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

تعداد {
	RECORD_تعداد = CONFIG_VAL(BOOTSTAGE_RECORD_تعداد),
};

بنية bootstage_record {
	ulong time_us;
	uint32_t start_us;
	ثابت حرف *name;
	صحيح flags;		/* see enum bootstage_flags */
	تعداد bootstage_id id;
};

بنية bootstage_data {
	uint rec_تعداد;
	uint next_id;
	بنية bootstage_record record[RECORD_تعداد];
};

تعداد {
	BOOTSTAGE_VERSION	= 0,
	BOOTSTAGE_MAGIC		= 0xb00757a3,
	BOOTSTAGE_DIGITS	= 9,
};

بنية bootstage_hdr {
	u32 version;		/* BOOTSTAGE_VERSION */
	u32 تعداد_2;		/* Number of records */
	u32 size;		/* Total data size (non-zero if valid) */
	u32 magic;		/* Magic number */
	u32 next_id;		/* Next ID to use for bootstage */
};

صحيح bootstage_relocate(فراغ *to)
{
	بنية bootstage_data *data;
	صحيح i;
	حرف *ptr;

	debug("Copying bootstage from %p to %p\n", gd->bootstage, to);
	memcpy(to, gd->bootstage, حجم(بنية bootstage_data));
	data = gd->bootstage = to;

	/* Figure out where to relocate the strings to */
	ptr = (حرف *)(data + 1);

	/*
	 * Duplicate all strings.  They may point to an old location in the
	 * program .text section that can eventually get trashed.
	 */
	debug("Relocating %d records\n", data->rec_تعداد);
	لكل (i = 0; i < data->rec_تعداد; i++) {
		ثابت حرف *from = data->record[i].name;

		إذا (!from)
			استمر;
		strcpy(ptr, from);
		data->record[i].name = ptr;
		ptr += strlen(ptr) + 1;
	}

	إرجاع 0;
}

بنية bootstage_record *find_id(بنية bootstage_data *data,
				 تعداد bootstage_id id)
{
	بنية bootstage_record *rec;
	بنية bootstage_record *end;

	لكل (rec = data->record, end = rec + data->rec_تعداد; rec < end;
	     rec++) {
		إذا (rec->id == id)
			إرجاع rec;
	}

	إرجاع عدم;
}

بنية bootstage_record *ensure_id(بنية bootstage_data *data,
				   تعداد bootstage_id id)
{
	بنية bootstage_record *rec;

	rec = find_id(data, id);
	إذا (!rec && data->rec_تعداد < RECORD_تعداد) {
		rec = &data->record[data->rec_تعداد++];
		rec->id = id;
		إرجاع rec;
	}

	إرجاع rec;
}

ulong bootstage_add_record(تعداد bootstage_id id, ثابت حرف *name,
			   صحيح flags, ulong mark)
{
	بنية bootstage_data *data = gd->bootstage;
	بنية bootstage_record *rec;

	/*
	 * initf_bootstage() is called very early during boot but since hang()
	 * calls bootstage_error() we can be called before bootstage is set up.
	 * Add a check to avoid this.
	 */
	إذا (!data)
		إرجاع mark;
	إذا (flags & BOOTSTAGEF_ALLOC)
		id = data->next_id++;

	/* Only record the first event for each */
	rec = find_id(data, id);
	إذا (!rec) {
		إذا (data->rec_تعداد < RECORD_تعداد) {
			rec = &data->record[data->rec_تعداد++];
			rec->time_us = mark;
			rec->name = name;
			rec->flags = flags;
			rec->id = id;
		} وإلا {
			log_warning("Bootstage space exhausted\n");
		}
	}

	/* Tell the board about this progress */
	show_boot_progress(flags & BOOTSTAGEF_ERROR ? -id : id);

	إرجاع mark;
}

ulong bootstage_error_name(تعداد bootstage_id id, ثابت حرف *name)
{
	إرجاع bootstage_add_record(id, name, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(تعداد bootstage_id id, ثابت حرف *name)
{
	صحيح flags = 0;

	إذا (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;

	إرجاع bootstage_add_record(id, name, flags, timer_get_boot_us());
}

ulong bootstage_mark_code(ثابت حرف *file, ثابت حرف *func, صحيح linenum)
{
	حرف *str, *p;
	__maybe_unused حرف *end;
	صحيح len = 0;

	/* First work out the length we need to allocate */
	إذا (linenum != -1)
		len = 11;
	إذا (func)
		len += strlen(func);
	إذا (file)
		len += strlen(file);

	str = malloc(len + 1);
	p = str;
	end = p + len;
	إذا (file)
		p += snprintf(p, end - p, "%s,", file);
	إذا (linenum != -1)
		p += snprintf(p, end - p, "%d", linenum);
	إذا (func)
		p += snprintf(p, end - p, ": %s", func);

	إرجاع bootstage_mark_name(BOOTSTAGE_ID_ALLOC, str);
}

uint32_t bootstage_start(تعداد bootstage_id id, ثابت حرف *name)
{
	بنية bootstage_data *data = gd->bootstage;
	بنية bootstage_record *rec = ensure_id(data, id);
	ulong start_us = timer_get_boot_us();

	إذا (rec) {
		rec->start_us = start_us;
		rec->name = name;
	}

	إرجاع start_us;
}

uint32_t bootstage_accum(تعداد bootstage_id id)
{
	بنية bootstage_data *data = gd->bootstage;
	بنية bootstage_record *rec = ensure_id(data, id);
	uint32_t duration;

	إذا (!rec)
		إرجاع 0;
	duration = (uint32_t)timer_get_boot_us() - rec->start_us;
	rec->time_us += duration;

	إرجاع duration;
}

/**
 * Get a record name as a printable string
 *
 * @param buf	Buffer to put name if needed
 * @param len	Length of buffer
 * @param rec	Boot stage record to get the name from
 * Return: pointer to name, either from the record or pointing to buf.
 */
ساكن ثابت حرف *get_record_name(حرف *buf, صحيح len,
				   ثابت بنية bootstage_record *rec)
{
	إذا (rec->name)
		إرجاع rec->name;
	وإلا إذا (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	وإلا
		snprintf(buf, len, "id=%d", rec->id);

	إرجاع buf;
}

ساكن uint32_t print_time_record(بنية bootstage_record *rec, uint32_t prev)
{
	حرف buf[20];

	إذا (prev == -1U) {
		printf("%11s", "");
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
	} وإلا {
		إذا (prev > rec->time_us)
			prev = 0;
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
		print_grouped_ull(rec->time_us - prev, BOOTSTAGE_DIGITS);
	}
	printf("  %s\n", get_record_name(buf, حجم(buf), rec));

	إرجاع rec->time_us;
}

#ifdef CONFIG_OF_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * Return: 0 on success, != 0 on failure.
 */
ساكن صحيح add_bootstages_devicetree(بنية fdt_header *blob)
{
	بنية bootstage_data *data = gd->bootstage;
	صحيح bootstage;
	حرف buf[20];
	صحيح recnum;
	صحيح i;

	إذا (!blob)
		إرجاع 0;

	/*
	 * Create the node for bootstage.
	 * The address of flat device tree is set up by the command bootm.
	 */
	bootstage = fdt_add_subnode(blob, 0, "bootstage");
	إذا (bootstage < 0)
		إرجاع -EINVAL;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	لكل (recnum = data->rec_تعداد - 1, i = 0; recnum >= 0; recnum--, i++) {
		بنية bootstage_record *rec = &data->record[recnum];
		صحيح عقدة;

		إذا (rec->id != BOOTSTAGE_ID_AWAKE && rec->time_us == 0)
			استمر;

		عقدة = fdt_add_subnode(blob, bootstage, simple_itoa(i));
		إذا (عقدة < 0)
			اكسر;

		/* add properties to the node. */
		إذا (fdt_setprop_string(blob, عقدة, "name",
				       get_record_name(buf, حجم(buf), rec)))
			إرجاع -EINVAL;

		/* Check if this is a 'mark' or 'accum' record */
		إذا (fdt_setprop_cell(blob, عقدة,
				rec->start_us ? "accum" : "mark",
				rec->time_us))
			إرجاع -EINVAL;
	}

	إرجاع 0;
}

صحيح bootstage_fdt_add_report(فراغ)
{
	إذا (add_bootstages_devicetree(working_fdt))
		puts("bootstage: Failed to add to device tree\n");

	إرجاع 0;
}
#endif

فراغ bootstage_report(فراغ)
{
	بنية bootstage_data *data = gd->bootstage;
	بنية bootstage_record *rec = data->record;
	uint32_t prev;
	صحيح i;

	printf("Timer summary in microseconds (%d records):\n",
	       data->rec_تعداد);
	printf("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	prev = print_time_record(rec, 0);

	لكل (i = 1, rec++; i < data->rec_تعداد; i++, rec++) {
		إذا (rec->id && !rec->start_us)
			prev = print_time_record(rec, prev);
	}
	إذا (data->rec_تعداد > RECORD_تعداد)
		printf("Overflowed internal boot id table by %d entries\n"
		       "Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		       data->rec_تعداد - RECORD_تعداد);

	puts("\nAccumulated time:\n");
	لكل (i = 0, rec = data->record; i < data->rec_تعداد; i++, rec++) {
		إذا (rec->start_us)
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
ساكن فراغ append_data(حرف **ptrp, حرف *end, ثابت فراغ *data, صحيح size)
{
	حرف *ptr = *ptrp;

	*ptrp += size;
	إذا (*ptrp > end)
		إرجاع;

	memcpy(ptr, data, size);
}

صحيح bootstage_stash(فراغ *base, صحيح size)
{
	ثابت بنية bootstage_data *data = gd->bootstage;
	بنية bootstage_hdr *hdr = (بنية bootstage_hdr *)base;
	ثابت بنية bootstage_record *rec;
	حرف buf[20];
	حرف *ptr = base, *end = ptr + size;
	صحيح i;

	إذا (hdr + 1 > (بنية bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		إرجاع -ENOSPC;
	}

	/* Write an arbitrary version number */
	hdr->version = BOOTSTAGE_VERSION;

	hdr->تعداد_2 = data->rec_تعداد;
	hdr->size = 0;
	hdr->magic = BOOTSTAGE_MAGIC;
	hdr->next_id = data->next_id;
	ptr += حجم(*hdr);

	/* Write the records, silently stopping when we run out of space */
	لكل (rec = data->record, i = 0; i < data->rec_تعداد; i++, rec++)
		append_data(&ptr, end, rec, حجم(*rec));

	/* Write the name strings */
	لكل (rec = data->record, i = 0; i < data->rec_تعداد; i++, rec++) {
		ثابت حرف *name;

		name = get_record_name(buf, حجم(buf), rec);
		append_data(&ptr, end, name, strlen(name) + 1);
	}

	/* Check for buffer overflow */
	إذا (ptr > end) {
		debug("%s: Not enough space for bootstage stash\n", __func__);
		إرجاع -ENOSPC;
	}

	/* Update total data size */
	hdr->size = ptr - (حرف *)base;
	debug("Stashed %d records\n", hdr->تعداد_2);

	إرجاع 0;
}

صحيح bootstage_unstash(ثابت فراغ *base, صحيح size)
{
	ثابت بنية bootstage_hdr *hdr = (بنية bootstage_hdr *)base;
	بنية bootstage_data *data = gd->bootstage;
	ثابت حرف *ptr = base, *end = ptr + size;
	بنية bootstage_record *rec;
	uint rec_size;
	صحيح i;

	إذا (size == -1)
		end = (حرف *)(~(uintptr_t)0);

	إذا (hdr + 1 > (بنية bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		إرجاع -EPERM;
	}

	إذا (hdr->magic != BOOTSTAGE_MAGIC) {
		debug("%s: Invalid bootstage magic\n", __func__);
		إرجاع -ENOENT;
	}

	إذا (ptr + hdr->size > end) {
		debug("%s: Bootstage data runs past buffer end\n", __func__);
		إرجاع -ENOSPC;
	}

	إذا (hdr->تعداد_2 * حجم(*rec) > hdr->size) {
		debug("%s: Bootstage has %d records needing %lu bytes, but "
			"only %d bytes is available\n", __func__, hdr->تعداد_2,
		      (ulong)hdr->تعداد_2 * حجم(*rec), hdr->size);
		إرجاع -ENOSPC;
	}

	إذا (hdr->version != BOOTSTAGE_VERSION) {
		debug("%s: Bootstage data version %#0x unrecognised\n",
		      __func__, hdr->version);
		إرجاع -EINVAL;
	}

	إذا (data->rec_تعداد + hdr->تعداد_2 > RECORD_تعداد) {
		debug("%s: Bootstage has %d records, we have space for %d\n"
			"Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		      __func__, hdr->تعداد_2, RECORD_تعداد - data->rec_تعداد);
		إرجاع -ENOSPC;
	}

	ptr += حجم(*hdr);

	/* Read the records */
	rec_size = hdr->تعداد_2 * حجم(*data->record);
	memcpy(data->record + data->rec_تعداد, ptr, rec_size);

	/* Read the name strings */
	ptr += rec_size;
	لكل (rec = data->record + data->rec_تعداد, i = 0; i < hdr->تعداد_2;
	     i++, rec++) {
		rec->name = ptr;
		إذا (xpl_phase() == PHASE_SPL)
			rec->name = strdup(ptr);

		/* Assume no data corruption here */
		ptr += strlen(ptr) + 1;
	}

	/* Mark the records as read */
	data->rec_تعداد += hdr->تعداد_2;
	data->next_id = hdr->next_id;
	debug("Unstashed %d records\n", hdr->تعداد_2);

	إرجاع 0;
}

#إذا IS_ENABLED(CONFIG_BOOTSTAGE_STASH)
صحيح _bootstage_stash_default(فراغ)
{
	إرجاع bootstage_stash(map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR, 0),
			       CONFIG_BOOTSTAGE_STASH_SIZE);
}

صحيح _bootstage_unstash_default(فراغ)
{
	ثابت فراغ *stash = map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR,
				       CONFIG_BOOTSTAGE_STASH_SIZE);

	إرجاع bootstage_unstash(stash, CONFIG_BOOTSTAGE_STASH_SIZE);
}
#endif

صحيح bootstage_get_size(bool add_strings)
{
	صحيح size;

	size = حجم(بنية bootstage_data);
	إذا (add_strings) {
		بنية bootstage_data *data = gd->bootstage;
		بنية bootstage_record *rec;
		صحيح i;

		لكل (rec = data->record, i = 0; i < data->rec_تعداد; i++, rec++)
			إذا (rec->name)
				size += strlen(rec->name) + 1;
	}

	إرجاع size;
}

صحيح bootstage_init(bool first)
{
	بنية bootstage_data *data;
	صحيح size = حجم(بنية bootstage_data);

	gd->bootstage = (بنية bootstage_data *)malloc(size);
	إذا (!gd->bootstage)
		إرجاع -ENOMEM;
	data = gd->bootstage;
	memset(data, '\0', size);
	إذا (first) {
		data->next_id = BOOTSTAGE_ID_USER;
		bootstage_add_record(BOOTSTAGE_ID_AWAKE, "reset", 0, 0);
	}

	إرجاع 0;
}
