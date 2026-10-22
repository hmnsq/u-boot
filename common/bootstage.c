// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 */

/*
 * This module records the progress of boot and arbitrary commands, and
 * permits accurate timestamping of each.
 */

#define LOG_CATEGORY	LOGC_BOOT

#ಅನ್ತರ್ಭಾವ <bootstage.h>
#ಅನ್ತರ್ಭಾವ <hang.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <sort.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <linux/compiler.h>
#ಅನ್ತರ್ಭಾವ <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

ಗಣನಾ {
	RECORD_ಎಣಿಕೆ = CONFIG_VAL(BOOTSTAGE_RECORD_ಎಣಿಕೆ),
};

ಸಂರಚನಾ bootstage_record {
	ulong time_us;
	uint32_t start_us;
	ಧ್ರುವ ವರ್ಣ *name;
	ಪೂರ್ಣಾಙ್ಕ flags;		/* see enum bootstage_flags */
	ಗಣನಾ bootstage_id id;
};

ಸಂರಚನಾ bootstage_data {
	uint rec_ಎಣಿಕೆ;
	uint next_id;
	ಸಂರಚನಾ bootstage_record record[RECORD_ಎಣಿಕೆ];
};

ಗಣನಾ {
	BOOTSTAGE_VERSION	= 0,
	BOOTSTAGE_MAGIC		= 0xb00757a3,
	BOOTSTAGE_DIGITS	= 9,
};

ಸಂರಚನಾ bootstage_hdr {
	u32 version;		/* BOOTSTAGE_VERSION */
	u32 ಎಣಿಕೆ;		/* Number of records */
	u32 size;		/* Total data size (non-zero if valid) */
	u32 magic;		/* Magic number */
	u32 next_id;		/* Next ID to use for bootstage */
};

ಪೂರ್ಣಾಙ್ಕ bootstage_relocate(ಶೂನ್ಯ *to)
{
	ಸಂರಚನಾ bootstage_data *data;
	ಪೂರ್ಣಾಙ್ಕ i;
	ವರ್ಣ *ptr;

	debug("Copying bootstage from %p to %p\n", gd->bootstage, to);
	memcpy(to, gd->bootstage, ಪರಿಮಾಣ(ಸಂರಚನಾ bootstage_data));
	data = gd->bootstage = to;

	/* Figure out where to relocate the strings to */
	ptr = (ವರ್ಣ *)(data + 1);

	/*
	 * Duplicate all strings.  They may point to an old location in the
	 * program .text section that can eventually get trashed.
	 */
	debug("Relocating %d records\n", data->rec_ಎಣಿಕೆ);
	ಪ್ರತಿ (i = 0; i < data->rec_ಎಣಿಕೆ; i++) {
		ಧ್ರುವ ವರ್ಣ *from = data->record[i].name;

		ಯದಿ (!from)
			ಅನುವರ್ತ;
		strcpy(ptr, from);
		data->record[i].name = ptr;
		ptr += strlen(ptr) + 1;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸಂರಚನಾ bootstage_record *find_id(ಸಂರಚನಾ bootstage_data *data,
				 ಗಣನಾ bootstage_id id)
{
	ಸಂರಚನಾ bootstage_record *rec;
	ಸಂರಚನಾ bootstage_record *end;

	ಪ್ರತಿ (rec = data->record, end = rec + data->rec_ಎಣಿಕೆ; rec < end;
	     rec++) {
		ಯದಿ (rec->id == id)
			ಪ್ರತ್ಯಾಗಮ rec;
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸಂರಚನಾ bootstage_record *ensure_id(ಸಂರಚನಾ bootstage_data *data,
				   ಗಣನಾ bootstage_id id)
{
	ಸಂರಚನಾ bootstage_record *rec;

	rec = find_id(data, id);
	ಯದಿ (!rec && data->rec_ಎಣಿಕೆ < RECORD_ಎಣಿಕೆ) {
		rec = &data->record[data->rec_ಎಣಿಕೆ++];
		rec->id = id;
		ಪ್ರತ್ಯಾಗಮ rec;
	}

	ಪ್ರತ್ಯಾಗಮ rec;
}

ulong bootstage_add_record(ಗಣನಾ bootstage_id id, ಧ್ರುವ ವರ್ಣ *name,
			   ಪೂರ್ಣಾಙ್ಕ flags, ulong mark)
{
	ಸಂರಚನಾ bootstage_data *data = gd->bootstage;
	ಸಂರಚನಾ bootstage_record *rec;

	/*
	 * initf_bootstage() is called very early during boot but since hang()
	 * calls bootstage_error() we can be called before bootstage is set up.
	 * Add a check to avoid this.
	 */
	ಯದಿ (!data)
		ಪ್ರತ್ಯಾಗಮ mark;
	ಯದಿ (flags & BOOTSTAGEF_ALLOC)
		id = data->next_id++;

	/* Only record the first event for each */
	rec = find_id(data, id);
	ಯದಿ (!rec) {
		ಯದಿ (data->rec_ಎಣಿಕೆ < RECORD_ಎಣಿಕೆ) {
			rec = &data->record[data->rec_ಎಣಿಕೆ++];
			rec->time_us = mark;
			rec->name = name;
			rec->flags = flags;
			rec->id = id;
		} ಅನ್ಯಥಾ {
			log_warning("Bootstage space exhausted\n");
		}
	}

	/* Tell the board about this progress */
	show_boot_progress(flags & BOOTSTAGEF_ERROR ? -id : id);

	ಪ್ರತ್ಯಾಗಮ mark;
}

ulong bootstage_error_name(ಗಣನಾ bootstage_id id, ಧ್ರುವ ವರ್ಣ *name)
{
	ಪ್ರತ್ಯಾಗಮ bootstage_add_record(id, name, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(ಗಣನಾ bootstage_id id, ಧ್ರುವ ವರ್ಣ *name)
{
	ಪೂರ್ಣಾಙ್ಕ flags = 0;

	ಯದಿ (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;

	ಪ್ರತ್ಯಾಗಮ bootstage_add_record(id, name, flags, timer_get_boot_us());
}

ulong bootstage_mark_code(ಧ್ರುವ ವರ್ಣ *file, ಧ್ರುವ ವರ್ಣ *func, ಪೂರ್ಣಾಙ್ಕ linenum)
{
	ವರ್ಣ *str, *p;
	__maybe_unused ವರ್ಣ *end;
	ಪೂರ್ಣಾಙ್ಕ len = 0;

	/* First work out the length we need to allocate */
	ಯದಿ (linenum != -1)
		len = 11;
	ಯದಿ (func)
		len += strlen(func);
	ಯದಿ (file)
		len += strlen(file);

	str = malloc(len + 1);
	p = str;
	end = p + len;
	ಯದಿ (file)
		p += snprintf(p, end - p, "%s,", file);
	ಯದಿ (linenum != -1)
		p += snprintf(p, end - p, "%d", linenum);
	ಯದಿ (func)
		p += snprintf(p, end - p, ": %s", func);

	ಪ್ರತ್ಯಾಗಮ bootstage_mark_name(BOOTSTAGE_ID_ALLOC, str);
}

uint32_t bootstage_start(ಗಣನಾ bootstage_id id, ಧ್ರುವ ವರ್ಣ *name)
{
	ಸಂರಚನಾ bootstage_data *data = gd->bootstage;
	ಸಂರಚನಾ bootstage_record *rec = ensure_id(data, id);
	ulong start_us = timer_get_boot_us();

	ಯದಿ (rec) {
		rec->start_us = start_us;
		rec->name = name;
	}

	ಪ್ರತ್ಯಾಗಮ start_us;
}

uint32_t bootstage_accum(ಗಣನಾ bootstage_id id)
{
	ಸಂರಚನಾ bootstage_data *data = gd->bootstage;
	ಸಂರಚನಾ bootstage_record *rec = ensure_id(data, id);
	uint32_t duration;

	ಯದಿ (!rec)
		ಪ್ರತ್ಯಾಗಮ 0;
	duration = (uint32_t)timer_get_boot_us() - rec->start_us;
	rec->time_us += duration;

	ಪ್ರತ್ಯಾಗಮ duration;
}

/**
 * Get a record name as a printable string
 *
 * @param buf	Buffer to put name if needed
 * @param len	Length of buffer
 * @param rec	Boot stage record to get the name from
 * Return: pointer to name, either from the record or pointing to buf.
 */
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *get_record_name(ವರ್ಣ *buf, ಪೂರ್ಣಾಙ್ಕ len,
				   ಧ್ರುವ ಸಂರಚನಾ bootstage_record *rec)
{
	ಯದಿ (rec->name)
		ಪ್ರತ್ಯಾಗಮ rec->name;
	ಅನ್ಯಥಾ ಯದಿ (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	ಅನ್ಯಥಾ
		snprintf(buf, len, "id=%d", rec->id);

	ಪ್ರತ್ಯಾಗಮ buf;
}

ಸ್ಥಿರ uint32_t print_time_record(ಸಂರಚನಾ bootstage_record *rec, uint32_t prev)
{
	ವರ್ಣ buf[20];

	ಯದಿ (prev == -1U) {
		printf("%11s", "");
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
	} ಅನ್ಯಥಾ {
		ಯದಿ (prev > rec->time_us)
			prev = 0;
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
		print_grouped_ull(rec->time_us - prev, BOOTSTAGE_DIGITS);
	}
	printf("  %s\n", get_record_name(buf, ಪರಿಮಾಣ(buf), rec));

	ಪ್ರತ್ಯಾಗಮ rec->time_us;
}

#ifdef CONFIG_OF_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * Return: 0 on success, != 0 on failure.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ add_bootstages_devicetree(ಸಂರಚನಾ fdt_header *blob)
{
	ಸಂರಚನಾ bootstage_data *data = gd->bootstage;
	ಪೂರ್ಣಾಙ್ಕ bootstage;
	ವರ್ಣ buf[20];
	ಪೂರ್ಣಾಙ್ಕ recnum;
	ಪೂರ್ಣಾಙ್ಕ i;

	ಯದಿ (!blob)
		ಪ್ರತ್ಯಾಗಮ 0;

	/*
	 * Create the node for bootstage.
	 * The address of flat device tree is set up by the command bootm.
	 */
	bootstage = fdt_add_subnode(blob, 0, "bootstage");
	ಯದಿ (bootstage < 0)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	ಪ್ರತಿ (recnum = data->rec_ಎಣಿಕೆ - 1, i = 0; recnum >= 0; recnum--, i++) {
		ಸಂರಚನಾ bootstage_record *rec = &data->record[recnum];
		ಪೂರ್ಣಾಙ್ಕ ಗಂಟು;

		ಯದಿ (rec->id != BOOTSTAGE_ID_AWAKE && rec->time_us == 0)
			ಅನುವರ್ತ;

		ಗಂಟು = fdt_add_subnode(blob, bootstage, simple_itoa(i));
		ಯದಿ (ಗಂಟು < 0)
			ಭಙ್ಗ;

		/* add properties to the node. */
		ಯದಿ (fdt_setprop_string(blob, ಗಂಟು, "name",
				       get_record_name(buf, ಪರಿಮಾಣ(buf), rec)))
			ಪ್ರತ್ಯಾಗಮ -EINVAL;

		/* Check if this is a 'mark' or 'accum' record */
		ಯದಿ (fdt_setprop_cell(blob, ಗಂಟು,
				rec->start_us ? "accum" : "mark",
				rec->time_us))
			ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ bootstage_fdt_add_report(ಶೂನ್ಯ)
{
	ಯದಿ (add_bootstages_devicetree(working_fdt))
		puts("bootstage: Failed to add to device tree\n");

	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif

ಶೂನ್ಯ bootstage_report(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ bootstage_data *data = gd->bootstage;
	ಸಂರಚನಾ bootstage_record *rec = data->record;
	uint32_t prev;
	ಪೂರ್ಣಾಙ್ಕ i;

	printf("Timer summary in microseconds (%d records):\n",
	       data->rec_ಎಣಿಕೆ);
	printf("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	prev = print_time_record(rec, 0);

	ಪ್ರತಿ (i = 1, rec++; i < data->rec_ಎಣಿಕೆ; i++, rec++) {
		ಯದಿ (rec->id && !rec->start_us)
			prev = print_time_record(rec, prev);
	}
	ಯದಿ (data->rec_ಎಣಿಕೆ > RECORD_ಎಣಿಕೆ)
		printf("Overflowed internal boot id table by %d entries\n"
		       "Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		       data->rec_ಎಣಿಕೆ - RECORD_ಎಣಿಕೆ);

	puts("\nAccumulated time:\n");
	ಪ್ರತಿ (i = 0, rec = data->record; i < data->rec_ಎಣಿಕೆ; i++, rec++) {
		ಯದಿ (rec->start_us)
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
ಸ್ಥಿರ ಶೂನ್ಯ append_data(ವರ್ಣ **ptrp, ವರ್ಣ *end, ಧ್ರುವ ಶೂನ್ಯ *data, ಪೂರ್ಣಾಙ್ಕ size)
{
	ವರ್ಣ *ptr = *ptrp;

	*ptrp += size;
	ಯದಿ (*ptrp > end)
		ಪ್ರತ್ಯಾಗಮ;

	memcpy(ptr, data, size);
}

ಪೂರ್ಣಾಙ್ಕ bootstage_stash(ಶೂನ್ಯ *base, ಪೂರ್ಣಾಙ್ಕ size)
{
	ಧ್ರುವ ಸಂರಚನಾ bootstage_data *data = gd->bootstage;
	ಸಂರಚನಾ bootstage_hdr *hdr = (ಸಂರಚನಾ bootstage_hdr *)base;
	ಧ್ರುವ ಸಂರಚನಾ bootstage_record *rec;
	ವರ್ಣ buf[20];
	ವರ್ಣ *ptr = base, *end = ptr + size;
	ಪೂರ್ಣಾಙ್ಕ i;

	ಯದಿ (hdr + 1 > (ಸಂರಚನಾ bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		ಪ್ರತ್ಯಾಗಮ -ENOSPC;
	}

	/* Write an arbitrary version number */
	hdr->version = BOOTSTAGE_VERSION;

	hdr->ಎಣಿಕೆ = data->rec_ಎಣಿಕೆ;
	hdr->size = 0;
	hdr->magic = BOOTSTAGE_MAGIC;
	hdr->next_id = data->next_id;
	ptr += ಪರಿಮಾಣ(*hdr);

	/* Write the records, silently stopping when we run out of space */
	ಪ್ರತಿ (rec = data->record, i = 0; i < data->rec_ಎಣಿಕೆ; i++, rec++)
		append_data(&ptr, end, rec, ಪರಿಮಾಣ(*rec));

	/* Write the name strings */
	ಪ್ರತಿ (rec = data->record, i = 0; i < data->rec_ಎಣಿಕೆ; i++, rec++) {
		ಧ್ರುವ ವರ್ಣ *name;

		name = get_record_name(buf, ಪರಿಮಾಣ(buf), rec);
		append_data(&ptr, end, name, strlen(name) + 1);
	}

	/* Check for buffer overflow */
	ಯದಿ (ptr > end) {
		debug("%s: Not enough space for bootstage stash\n", __func__);
		ಪ್ರತ್ಯಾಗಮ -ENOSPC;
	}

	/* Update total data size */
	hdr->size = ptr - (ವರ್ಣ *)base;
	debug("Stashed %d records\n", hdr->ಎಣಿಕೆ);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ bootstage_unstash(ಧ್ರುವ ಶೂನ್ಯ *base, ಪೂರ್ಣಾಙ್ಕ size)
{
	ಧ್ರುವ ಸಂರಚನಾ bootstage_hdr *hdr = (ಸಂರಚನಾ bootstage_hdr *)base;
	ಸಂರಚನಾ bootstage_data *data = gd->bootstage;
	ಧ್ರುವ ವರ್ಣ *ptr = base, *end = ptr + size;
	ಸಂರಚನಾ bootstage_record *rec;
	uint rec_size;
	ಪೂರ್ಣಾಙ್ಕ i;

	ಯದಿ (size == -1)
		end = (ವರ್ಣ *)(~(uintptr_t)0);

	ಯದಿ (hdr + 1 > (ಸಂರಚನಾ bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		ಪ್ರತ್ಯಾಗಮ -EPERM;
	}

	ಯದಿ (hdr->magic != BOOTSTAGE_MAGIC) {
		debug("%s: Invalid bootstage magic\n", __func__);
		ಪ್ರತ್ಯಾಗಮ -ENOENT;
	}

	ಯದಿ (ptr + hdr->size > end) {
		debug("%s: Bootstage data runs past buffer end\n", __func__);
		ಪ್ರತ್ಯಾಗಮ -ENOSPC;
	}

	ಯದಿ (hdr->ಎಣಿಕೆ * ಪರಿಮಾಣ(*rec) > hdr->size) {
		debug("%s: Bootstage has %d records needing %lu bytes, but "
			"only %d bytes is available\n", __func__, hdr->ಎಣಿಕೆ,
		      (ulong)hdr->ಎಣಿಕೆ * ಪರಿಮಾಣ(*rec), hdr->size);
		ಪ್ರತ್ಯಾಗಮ -ENOSPC;
	}

	ಯದಿ (hdr->version != BOOTSTAGE_VERSION) {
		debug("%s: Bootstage data version %#0x unrecognised\n",
		      __func__, hdr->version);
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	ಯದಿ (data->rec_ಎಣಿಕೆ + hdr->ಎಣಿಕೆ > RECORD_ಎಣಿಕೆ) {
		debug("%s: Bootstage has %d records, we have space for %d\n"
			"Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		      __func__, hdr->ಎಣಿಕೆ, RECORD_ಎಣಿಕೆ - data->rec_ಎಣಿಕೆ);
		ಪ್ರತ್ಯಾಗಮ -ENOSPC;
	}

	ptr += ಪರಿಮಾಣ(*hdr);

	/* Read the records */
	rec_size = hdr->ಎಣಿಕೆ * ಪರಿಮಾಣ(*data->record);
	memcpy(data->record + data->rec_ಎಣಿಕೆ, ptr, rec_size);

	/* Read the name strings */
	ptr += rec_size;
	ಪ್ರತಿ (rec = data->record + data->rec_ಎಣಿಕೆ, i = 0; i < hdr->ಎಣಿಕೆ;
	     i++, rec++) {
		rec->name = ptr;
		ಯದಿ (xpl_phase() == PHASE_SPL)
			rec->name = strdup(ptr);

		/* Assume no data corruption here */
		ptr += strlen(ptr) + 1;
	}

	/* Mark the records as read */
	data->rec_ಎಣಿಕೆ += hdr->ಎಣಿಕೆ;
	data->next_id = hdr->next_id;
	debug("Unstashed %d records\n", hdr->ಎಣಿಕೆ);

	ಪ್ರತ್ಯಾಗಮ 0;
}

#ಯದಿ IS_ENABLED(CONFIG_BOOTSTAGE_STASH)
ಪೂರ್ಣಾಙ್ಕ _bootstage_stash_default(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ bootstage_stash(map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR, 0),
			       CONFIG_BOOTSTAGE_STASH_SIZE);
}

ಪೂರ್ಣಾಙ್ಕ _bootstage_unstash_default(ಶೂನ್ಯ)
{
	ಧ್ರುವ ಶೂನ್ಯ *stash = map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR,
				       CONFIG_BOOTSTAGE_STASH_SIZE);

	ಪ್ರತ್ಯಾಗಮ bootstage_unstash(stash, CONFIG_BOOTSTAGE_STASH_SIZE);
}
#endif

ಪೂರ್ಣಾಙ್ಕ bootstage_get_size(bool add_strings)
{
	ಪೂರ್ಣಾಙ್ಕ size;

	size = ಪರಿಮಾಣ(ಸಂರಚನಾ bootstage_data);
	ಯದಿ (add_strings) {
		ಸಂರಚನಾ bootstage_data *data = gd->bootstage;
		ಸಂರಚನಾ bootstage_record *rec;
		ಪೂರ್ಣಾಙ್ಕ i;

		ಪ್ರತಿ (rec = data->record, i = 0; i < data->rec_ಎಣಿಕೆ; i++, rec++)
			ಯದಿ (rec->name)
				size += strlen(rec->name) + 1;
	}

	ಪ್ರತ್ಯಾಗಮ size;
}

ಪೂರ್ಣಾಙ್ಕ bootstage_init(bool first)
{
	ಸಂರಚನಾ bootstage_data *data;
	ಪೂರ್ಣಾಙ್ಕ size = ಪರಿಮಾಣ(ಸಂರಚನಾ bootstage_data);

	gd->bootstage = (ಸಂರಚನಾ bootstage_data *)malloc(size);
	ಯದಿ (!gd->bootstage)
		ಪ್ರತ್ಯಾಗಮ -ENOMEM;
	data = gd->bootstage;
	memset(data, '\0', size);
	ಯದಿ (first) {
		data->next_id = BOOTSTAGE_ID_USER;
		bootstage_add_record(BOOTSTAGE_ID_AWAKE, "reset", 0, 0);
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}
