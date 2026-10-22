// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 */

/*
 * This module records the progress of boot and arbitrary commands, and
 * permits accurate timestamping of each.
 */

#define LOG_CATEGORY	LOGC_BOOT

#অন্তর্ভাব <bootstage.h>
#অন্তর্ভাব <hang.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <malloc.h>
#অন্তর্ভাব <sort.h>
#অন্তর্ভাব <spl.h>
#অন্তর্ভাব <asm/global_data.h>
#অন্তর্ভাব <linux/compiler.h>
#অন্তর্ভাব <linux/libfdt.h>

DECLARE_GLOBAL_তথ্য_PTR;

গণনা {
	RECORD_গুনতি = CONFIG_VAL(BOOTSTAGE_RECORD_গুনতি),
};

কাঠামো bootstage_record {
	ulong time_us;
	uint32_t start_us;
	ধ্রুবক অক্ষর *name;
	পূর্ণসংখ্যা flags;		/* see enum bootstage_flags */
	গণনা bootstage_id id;
};

কাঠামো bootstage_তথ্য {
	uint rec_গুনতি;
	uint পরবর্তী_id;
	কাঠামো bootstage_record record[RECORD_গুনতি];
};

গণনা {
	BOOTSTAGE_VERSION	= 0,
	BOOTSTAGE_MAGIC		= 0xb00757a3,
	BOOTSTAGE_DIGITS	= 9,
};

কাঠামো bootstage_hdr {
	u32 version;		/* BOOTSTAGE_VERSION */
	u32 গুনতি;		/* Number of records */
	u32 আকার_2;		/* Total data size (non-zero if valid) */
	u32 magic;		/* Magic number */
	u32 পরবর্তী_id;		/* Next ID to use for bootstage */
};

পূর্ণসংখ্যা bootstage_relocate(শূন্য *to)
{
	কাঠামো bootstage_তথ্য *তথ্য;
	পূর্ণসংখ্যা i;
	অক্ষর *ptr;

	debug("Copying bootstage from %p to %p\n", gd->bootstage, to);
	memcpy(to, gd->bootstage, আকার(কাঠামো bootstage_তথ্য));
	তথ্য = gd->bootstage = to;

	/* Figure out where to relocate the strings to */
	ptr = (অক্ষর *)(তথ্য + 1);

	/*
	 * Duplicate all strings.  They may point to an old location in the
	 * program .text section that can eventually get trashed.
	 */
	debug("Relocating %d records\n", তথ্য->rec_গুনতি);
	জন্য (i = 0; i < তথ্য->rec_গুনতি; i++) {
		ধ্রুবক অক্ষর *from = তথ্য->record[i].name;

		যদি (!from)
			চালু;
		strcpy(ptr, from);
		তথ্য->record[i].name = ptr;
		ptr += strlen(ptr) + 1;
	}

	ফেরত 0;
}

কাঠামো bootstage_record *find_id(কাঠামো bootstage_তথ্য *তথ্য,
				 গণনা bootstage_id id)
{
	কাঠামো bootstage_record *rec;
	কাঠামো bootstage_record *end;

	জন্য (rec = তথ্য->record, end = rec + তথ্য->rec_গুনতি; rec < end;
	     rec++) {
		যদি (rec->id == id)
			ফেরত rec;
	}

	ফেরত ফাঁকা;
}

কাঠামো bootstage_record *ensure_id(কাঠামো bootstage_তথ্য *তথ্য,
				   গণনা bootstage_id id)
{
	কাঠামো bootstage_record *rec;

	rec = find_id(তথ্য, id);
	যদি (!rec && তথ্য->rec_গুনতি < RECORD_গুনতি) {
		rec = &তথ্য->record[তথ্য->rec_গুনতি++];
		rec->id = id;
		ফেরত rec;
	}

	ফেরত rec;
}

ulong bootstage_add_record(গণনা bootstage_id id, ধ্রুবক অক্ষর *name,
			   পূর্ণসংখ্যা flags, ulong mark)
{
	কাঠামো bootstage_তথ্য *তথ্য = gd->bootstage;
	কাঠামো bootstage_record *rec;

	/*
	 * initf_bootstage() is called very early during boot but since hang()
	 * calls bootstage_error() we can be called before bootstage is set up.
	 * Add a check to avoid this.
	 */
	যদি (!তথ্য)
		ফেরত mark;
	যদি (flags & BOOTSTAGEF_ALLOC)
		id = তথ্য->পরবর্তী_id++;

	/* Only record the first event for each */
	rec = find_id(তথ্য, id);
	যদি (!rec) {
		যদি (তথ্য->rec_গুনতি < RECORD_গুনতি) {
			rec = &তথ্য->record[তথ্য->rec_গুনতি++];
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

	ফেরত mark;
}

ulong bootstage_error_name(গণনা bootstage_id id, ধ্রুবক অক্ষর *name)
{
	ফেরত bootstage_add_record(id, name, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(গণনা bootstage_id id, ধ্রুবক অক্ষর *name)
{
	পূর্ণসংখ্যা flags = 0;

	যদি (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;

	ফেরত bootstage_add_record(id, name, flags, timer_get_boot_us());
}

ulong bootstage_mark_code(ধ্রুবক অক্ষর *file, ধ্রুবক অক্ষর *func, পূর্ণসংখ্যা linenum)
{
	অক্ষর *str, *p;
	__maybe_unused অক্ষর *end;
	পূর্ণসংখ্যা len = 0;

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

	ফেরত bootstage_mark_name(BOOTSTAGE_ID_ALLOC, str);
}

uint32_t bootstage_start(গণনা bootstage_id id, ধ্রুবক অক্ষর *name)
{
	কাঠামো bootstage_তথ্য *তথ্য = gd->bootstage;
	কাঠামো bootstage_record *rec = ensure_id(তথ্য, id);
	ulong start_us = timer_get_boot_us();

	যদি (rec) {
		rec->start_us = start_us;
		rec->name = name;
	}

	ফেরত start_us;
}

uint32_t bootstage_accum(গণনা bootstage_id id)
{
	কাঠামো bootstage_তথ্য *তথ্য = gd->bootstage;
	কাঠামো bootstage_record *rec = ensure_id(তথ্য, id);
	uint32_t duration;

	যদি (!rec)
		ফেরত 0;
	duration = (uint32_t)timer_get_boot_us() - rec->start_us;
	rec->time_us += duration;

	ফেরত duration;
}

/**
 * Get a record name as a printable string
 *
 * @param buf	Buffer to put name if needed
 * @param len	Length of buffer
 * @param rec	Boot stage record to get the name from
 * Return: pointer to name, either from the record or pointing to buf.
 */
স্থির ধ্রুবক অক্ষর *get_record_name(অক্ষর *buf, পূর্ণসংখ্যা len,
				   ধ্রুবক কাঠামো bootstage_record *rec)
{
	যদি (rec->name)
		ফেরত rec->name;
	অন্যথা যদি (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	অন্যথা
		snprintf(buf, len, "id=%d", rec->id);

	ফেরত buf;
}

স্থির uint32_t print_time_record(কাঠামো bootstage_record *rec, uint32_t পূর্ব)
{
	অক্ষর buf[20];

	যদি (পূর্ব == -1U) {
		printf("%11s", "");
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
	} অন্যথা {
		যদি (পূর্ব > rec->time_us)
			পূর্ব = 0;
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
		print_grouped_ull(rec->time_us - পূর্ব, BOOTSTAGE_DIGITS);
	}
	printf("  %s\n", get_record_name(buf, আকার(buf), rec));

	ফেরত rec->time_us;
}

#ifdef CONFIG_OF_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * Return: 0 on success, != 0 on failure.
 */
স্থির পূর্ণসংখ্যা add_bootstages_devicetree(কাঠামো fdt_header *blob)
{
	কাঠামো bootstage_তথ্য *তথ্য = gd->bootstage;
	পূর্ণসংখ্যা bootstage;
	অক্ষর buf[20];
	পূর্ণসংখ্যা recnum;
	পূর্ণসংখ্যা i;

	যদি (!blob)
		ফেরত 0;

	/*
	 * Create the node for bootstage.
	 * The address of flat device tree is set up by the command bootm.
	 */
	bootstage = fdt_add_subnode(blob, 0, "bootstage");
	যদি (bootstage < 0)
		ফেরত -EINVAL;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	জন্য (recnum = তথ্য->rec_গুনতি - 1, i = 0; recnum >= 0; recnum--, i++) {
		কাঠামো bootstage_record *rec = &তথ্য->record[recnum];
		পূর্ণসংখ্যা গ্রন্থি;

		যদি (rec->id != BOOTSTAGE_ID_AWAKE && rec->time_us == 0)
			চালু;

		গ্রন্থি = fdt_add_subnode(blob, bootstage, simple_itoa(i));
		যদি (গ্রন্থি < 0)
			বিরতি;

		/* add properties to the node. */
		যদি (fdt_setprop_string(blob, গ্রন্থি, "name",
				       get_record_name(buf, আকার(buf), rec)))
			ফেরত -EINVAL;

		/* Check if this is a 'mark' or 'accum' record */
		যদি (fdt_setprop_cell(blob, গ্রন্থি,
				rec->start_us ? "accum" : "mark",
				rec->time_us))
			ফেরত -EINVAL;
	}

	ফেরত 0;
}

পূর্ণসংখ্যা bootstage_fdt_add_report(শূন্য)
{
	যদি (add_bootstages_devicetree(working_fdt))
		puts("bootstage: Failed to add to device tree\n");

	ফেরত 0;
}
#endif

শূন্য bootstage_report(শূন্য)
{
	কাঠামো bootstage_তথ্য *তথ্য = gd->bootstage;
	কাঠামো bootstage_record *rec = তথ্য->record;
	uint32_t পূর্ব;
	পূর্ণসংখ্যা i;

	printf("Timer summary in microseconds (%d records):\n",
	       তথ্য->rec_গুনতি);
	printf("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	পূর্ব = print_time_record(rec, 0);

	জন্য (i = 1, rec++; i < তথ্য->rec_গুনতি; i++, rec++) {
		যদি (rec->id && !rec->start_us)
			পূর্ব = print_time_record(rec, পূর্ব);
	}
	যদি (তথ্য->rec_গুনতি > RECORD_গুনতি)
		printf("Overflowed internal boot id table by %d entries\n"
		       "Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		       তথ্য->rec_গুনতি - RECORD_গুনতি);

	puts("\nAccumulated time:\n");
	জন্য (i = 0, rec = তথ্য->record; i < তথ্য->rec_গুনতি; i++, rec++) {
		যদি (rec->start_us)
			পূর্ব = print_time_record(rec, -1);
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
স্থির শূন্য append_তথ্য(অক্ষর **ptrp, অক্ষর *end, ধ্রুবক শূন্য *তথ্য, পূর্ণসংখ্যা আকার_2)
{
	অক্ষর *ptr = *ptrp;

	*ptrp += আকার_2;
	যদি (*ptrp > end)
		ফেরত;

	memcpy(ptr, তথ্য, আকার_2);
}

পূর্ণসংখ্যা bootstage_stash(শূন্য *base, পূর্ণসংখ্যা আকার_2)
{
	ধ্রুবক কাঠামো bootstage_তথ্য *তথ্য = gd->bootstage;
	কাঠামো bootstage_hdr *hdr = (কাঠামো bootstage_hdr *)base;
	ধ্রুবক কাঠামো bootstage_record *rec;
	অক্ষর buf[20];
	অক্ষর *ptr = base, *end = ptr + আকার_2;
	পূর্ণসংখ্যা i;

	যদি (hdr + 1 > (কাঠামো bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		ফেরত -ENOSPC;
	}

	/* Write an arbitrary version number */
	hdr->version = BOOTSTAGE_VERSION;

	hdr->গুনতি = তথ্য->rec_গুনতি;
	hdr->আকার_2 = 0;
	hdr->magic = BOOTSTAGE_MAGIC;
	hdr->পরবর্তী_id = তথ্য->পরবর্তী_id;
	ptr += আকার(*hdr);

	/* Write the records, silently stopping when we run out of space */
	জন্য (rec = তথ্য->record, i = 0; i < তথ্য->rec_গুনতি; i++, rec++)
		append_তথ্য(&ptr, end, rec, আকার(*rec));

	/* Write the name strings */
	জন্য (rec = তথ্য->record, i = 0; i < তথ্য->rec_গুনতি; i++, rec++) {
		ধ্রুবক অক্ষর *name;

		name = get_record_name(buf, আকার(buf), rec);
		append_তথ্য(&ptr, end, name, strlen(name) + 1);
	}

	/* Check for buffer overflow */
	যদি (ptr > end) {
		debug("%s: Not enough space for bootstage stash\n", __func__);
		ফেরত -ENOSPC;
	}

	/* Update total data size */
	hdr->আকার_2 = ptr - (অক্ষর *)base;
	debug("Stashed %d records\n", hdr->গুনতি);

	ফেরত 0;
}

পূর্ণসংখ্যা bootstage_unstash(ধ্রুবক শূন্য *base, পূর্ণসংখ্যা আকার_2)
{
	ধ্রুবক কাঠামো bootstage_hdr *hdr = (কাঠামো bootstage_hdr *)base;
	কাঠামো bootstage_তথ্য *তথ্য = gd->bootstage;
	ধ্রুবক অক্ষর *ptr = base, *end = ptr + আকার_2;
	কাঠামো bootstage_record *rec;
	uint rec_আকার;
	পূর্ণসংখ্যা i;

	যদি (আকার_2 == -1)
		end = (অক্ষর *)(~(uintptr_t)0);

	যদি (hdr + 1 > (কাঠামো bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		ফেরত -EPERM;
	}

	যদি (hdr->magic != BOOTSTAGE_MAGIC) {
		debug("%s: Invalid bootstage magic\n", __func__);
		ফেরত -ENOENT;
	}

	যদি (ptr + hdr->আকার_2 > end) {
		debug("%s: Bootstage data runs past buffer end\n", __func__);
		ফেরত -ENOSPC;
	}

	যদি (hdr->গুনতি * আকার(*rec) > hdr->আকার_2) {
		debug("%s: Bootstage has %d records needing %lu bytes, but "
			"only %d bytes is available\n", __func__, hdr->গুনতি,
		      (ulong)hdr->গুনতি * আকার(*rec), hdr->আকার_2);
		ফেরত -ENOSPC;
	}

	যদি (hdr->version != BOOTSTAGE_VERSION) {
		debug("%s: Bootstage data version %#0x unrecognised\n",
		      __func__, hdr->version);
		ফেরত -EINVAL;
	}

	যদি (তথ্য->rec_গুনতি + hdr->গুনতি > RECORD_গুনতি) {
		debug("%s: Bootstage has %d records, we have space for %d\n"
			"Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		      __func__, hdr->গুনতি, RECORD_গুনতি - তথ্য->rec_গুনতি);
		ফেরত -ENOSPC;
	}

	ptr += আকার(*hdr);

	/* Read the records */
	rec_আকার = hdr->গুনতি * আকার(*তথ্য->record);
	memcpy(তথ্য->record + তথ্য->rec_গুনতি, ptr, rec_আকার);

	/* Read the name strings */
	ptr += rec_আকার;
	জন্য (rec = তথ্য->record + তথ্য->rec_গুনতি, i = 0; i < hdr->গুনতি;
	     i++, rec++) {
		rec->name = ptr;
		যদি (xpl_phase() == PHASE_SPL)
			rec->name = strdup(ptr);

		/* Assume no data corruption here */
		ptr += strlen(ptr) + 1;
	}

	/* Mark the records as read */
	তথ্য->rec_গুনতি += hdr->গুনতি;
	তথ্য->পরবর্তী_id = hdr->পরবর্তী_id;
	debug("Unstashed %d records\n", hdr->গুনতি);

	ফেরত 0;
}

#যদি IS_ENABLED(CONFIG_BOOTSTAGE_STASH)
পূর্ণসংখ্যা _bootstage_stash_default(শূন্য)
{
	ফেরত bootstage_stash(map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR, 0),
			       CONFIG_BOOTSTAGE_STASH_আকার);
}

পূর্ণসংখ্যা _bootstage_unstash_default(শূন্য)
{
	ধ্রুবক শূন্য *stash = map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR,
				       CONFIG_BOOTSTAGE_STASH_আকার);

	ফেরত bootstage_unstash(stash, CONFIG_BOOTSTAGE_STASH_আকার);
}
#endif

পূর্ণসংখ্যা bootstage_get_আকার(bool add_strings)
{
	পূর্ণসংখ্যা আকার_2;

	আকার_2 = আকার(কাঠামো bootstage_তথ্য);
	যদি (add_strings) {
		কাঠামো bootstage_তথ্য *তথ্য = gd->bootstage;
		কাঠামো bootstage_record *rec;
		পূর্ণসংখ্যা i;

		জন্য (rec = তথ্য->record, i = 0; i < তথ্য->rec_গুনতি; i++, rec++)
			যদি (rec->name)
				আকার_2 += strlen(rec->name) + 1;
	}

	ফেরত আকার_2;
}

পূর্ণসংখ্যা bootstage_init(bool first)
{
	কাঠামো bootstage_তথ্য *তথ্য;
	পূর্ণসংখ্যা আকার_2 = আকার(কাঠামো bootstage_তথ্য);

	gd->bootstage = (কাঠামো bootstage_তথ্য *)malloc(আকার_2);
	যদি (!gd->bootstage)
		ফেরত -ENOMEM;
	তথ্য = gd->bootstage;
	memset(তথ্য, '\0', আকার_2);
	যদি (first) {
		তথ্য->পরবর্তী_id = BOOTSTAGE_ID_USER;
		bootstage_add_record(BOOTSTAGE_ID_AWAKE, "reset", 0, 0);
	}

	ফেরত 0;
}
