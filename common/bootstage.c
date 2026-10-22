// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 */

/*
 * This module records the progress of boot and arbitrary commands, and
 * permits accurate timestamping of each.
 */

#define LOG_CATEGORY	LOGC_BOOT

#അന്തര്ഭാവ <bootstage.h>
#അന്തര്ഭാവ <hang.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <malloc.h>
#അന്തര്ഭാവ <sort.h>
#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <asm/global_data.h>
#അന്തര്ഭാവ <linux/compiler.h>
#അന്തര്ഭാവ <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

എണ്ണൽ {
	RECORD_എണ്ണം = CONFIG_VAL(BOOTSTAGE_RECORD_എണ്ണം),
};

ഘടന bootstage_record {
	ulong time_us;
	uint32_t start_us;
	സ്ഥിരം അക്ഷരം *name;
	പൂർണ്ണസംഖ്യ flags;		/* see enum bootstage_flags */
	എണ്ണൽ bootstage_id id;
};

ഘടന bootstage_data {
	uint rec_എണ്ണം;
	uint next_id;
	ഘടന bootstage_record record[RECORD_എണ്ണം];
};

എണ്ണൽ {
	BOOTSTAGE_VERSION	= 0,
	BOOTSTAGE_MAGIC		= 0xb00757a3,
	BOOTSTAGE_DIGITS	= 9,
};

ഘടന bootstage_hdr {
	u32 version;		/* BOOTSTAGE_VERSION */
	u32 എണ്ണം;		/* Number of records */
	u32 size;		/* Total data size (non-zero if valid) */
	u32 magic;		/* Magic number */
	u32 next_id;		/* Next ID to use for bootstage */
};

പൂർണ്ണസംഖ്യ bootstage_relocate(ശൂന്യം *to)
{
	ഘടന bootstage_data *data;
	പൂർണ്ണസംഖ്യ i;
	അക്ഷരം *ptr;

	debug("Copying bootstage from %p to %p\n", gd->bootstage, to);
	memcpy(to, gd->bootstage, വലുപ്പം(ഘടന bootstage_data));
	data = gd->bootstage = to;

	/* Figure out where to relocate the strings to */
	ptr = (അക്ഷരം *)(data + 1);

	/*
	 * Duplicate all strings.  They may point to an old location in the
	 * program .text section that can eventually get trashed.
	 */
	debug("Relocating %d records\n", data->rec_എണ്ണം);
	വേണ്ടി (i = 0; i < data->rec_എണ്ണം; i++) {
		സ്ഥിരം അക്ഷരം *from = data->record[i].name;

		എങ്കിൽ (!from)
			തുടരുക;
		strcpy(ptr, from);
		data->record[i].name = ptr;
		ptr += strlen(ptr) + 1;
	}

	തിരികെ 0;
}

ഘടന bootstage_record *find_id(ഘടന bootstage_data *data,
				 എണ്ണൽ bootstage_id id)
{
	ഘടന bootstage_record *rec;
	ഘടന bootstage_record *end;

	വേണ്ടി (rec = data->record, end = rec + data->rec_എണ്ണം; rec < end;
	     rec++) {
		എങ്കിൽ (rec->id == id)
			തിരികെ rec;
	}

	തിരികെ ശൂന്യ;
}

ഘടന bootstage_record *ensure_id(ഘടന bootstage_data *data,
				   എണ്ണൽ bootstage_id id)
{
	ഘടന bootstage_record *rec;

	rec = find_id(data, id);
	എങ്കിൽ (!rec && data->rec_എണ്ണം < RECORD_എണ്ണം) {
		rec = &data->record[data->rec_എണ്ണം++];
		rec->id = id;
		തിരികെ rec;
	}

	തിരികെ rec;
}

ulong bootstage_add_record(എണ്ണൽ bootstage_id id, സ്ഥിരം അക്ഷരം *name,
			   പൂർണ്ണസംഖ്യ flags, ulong mark)
{
	ഘടന bootstage_data *data = gd->bootstage;
	ഘടന bootstage_record *rec;

	/*
	 * initf_bootstage() is called very early during boot but since hang()
	 * calls bootstage_error() we can be called before bootstage is set up.
	 * Add a check to avoid this.
	 */
	എങ്കിൽ (!data)
		തിരികെ mark;
	എങ്കിൽ (flags & BOOTSTAGEF_ALLOC)
		id = data->next_id++;

	/* Only record the first event for each */
	rec = find_id(data, id);
	എങ്കിൽ (!rec) {
		എങ്കിൽ (data->rec_എണ്ണം < RECORD_എണ്ണം) {
			rec = &data->record[data->rec_എണ്ണം++];
			rec->time_us = mark;
			rec->name = name;
			rec->flags = flags;
			rec->id = id;
		} അല്ലെങ്കിൽ {
			log_warning("Bootstage space exhausted\n");
		}
	}

	/* Tell the board about this progress */
	show_boot_progress(flags & BOOTSTAGEF_ERROR ? -id : id);

	തിരികെ mark;
}

ulong bootstage_error_name(എണ്ണൽ bootstage_id id, സ്ഥിരം അക്ഷരം *name)
{
	തിരികെ bootstage_add_record(id, name, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(എണ്ണൽ bootstage_id id, സ്ഥിരം അക്ഷരം *name)
{
	പൂർണ്ണസംഖ്യ flags = 0;

	എങ്കിൽ (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;

	തിരികെ bootstage_add_record(id, name, flags, timer_get_boot_us());
}

ulong bootstage_mark_code(സ്ഥിരം അക്ഷരം *file, സ്ഥിരം അക്ഷരം *func, പൂർണ്ണസംഖ്യ linenum)
{
	അക്ഷരം *str, *p;
	__maybe_unused അക്ഷരം *end;
	പൂർണ്ണസംഖ്യ len = 0;

	/* First work out the length we need to allocate */
	എങ്കിൽ (linenum != -1)
		len = 11;
	എങ്കിൽ (func)
		len += strlen(func);
	എങ്കിൽ (file)
		len += strlen(file);

	str = malloc(len + 1);
	p = str;
	end = p + len;
	എങ്കിൽ (file)
		p += snprintf(p, end - p, "%s,", file);
	എങ്കിൽ (linenum != -1)
		p += snprintf(p, end - p, "%d", linenum);
	എങ്കിൽ (func)
		p += snprintf(p, end - p, ": %s", func);

	തിരികെ bootstage_mark_name(BOOTSTAGE_ID_ALLOC, str);
}

uint32_t bootstage_start(എണ്ണൽ bootstage_id id, സ്ഥിരം അക്ഷരം *name)
{
	ഘടന bootstage_data *data = gd->bootstage;
	ഘടന bootstage_record *rec = ensure_id(data, id);
	ulong start_us = timer_get_boot_us();

	എങ്കിൽ (rec) {
		rec->start_us = start_us;
		rec->name = name;
	}

	തിരികെ start_us;
}

uint32_t bootstage_accum(എണ്ണൽ bootstage_id id)
{
	ഘടന bootstage_data *data = gd->bootstage;
	ഘടന bootstage_record *rec = ensure_id(data, id);
	uint32_t duration;

	എങ്കിൽ (!rec)
		തിരികെ 0;
	duration = (uint32_t)timer_get_boot_us() - rec->start_us;
	rec->time_us += duration;

	തിരികെ duration;
}

/**
 * Get a record name as a printable string
 *
 * @param buf	Buffer to put name if needed
 * @param len	Length of buffer
 * @param rec	Boot stage record to get the name from
 * Return: pointer to name, either from the record or pointing to buf.
 */
സ്ഥിതം സ്ഥിരം അക്ഷരം *get_record_name(അക്ഷരം *buf, പൂർണ്ണസംഖ്യ len,
				   സ്ഥിരം ഘടന bootstage_record *rec)
{
	എങ്കിൽ (rec->name)
		തിരികെ rec->name;
	അല്ലെങ്കിൽ എങ്കിൽ (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	അല്ലെങ്കിൽ
		snprintf(buf, len, "id=%d", rec->id);

	തിരികെ buf;
}

സ്ഥിതം uint32_t print_time_record(ഘടന bootstage_record *rec, uint32_t prev)
{
	അക്ഷരം buf[20];

	എങ്കിൽ (prev == -1U) {
		printf("%11s", "");
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
	} അല്ലെങ്കിൽ {
		എങ്കിൽ (prev > rec->time_us)
			prev = 0;
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
		print_grouped_ull(rec->time_us - prev, BOOTSTAGE_DIGITS);
	}
	printf("  %s\n", get_record_name(buf, വലുപ്പം(buf), rec));

	തിരികെ rec->time_us;
}

#ifdef CONFIG_OF_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * Return: 0 on success, != 0 on failure.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ add_bootstages_devicetree(ഘടന fdt_header *blob)
{
	ഘടന bootstage_data *data = gd->bootstage;
	പൂർണ്ണസംഖ്യ bootstage;
	അക്ഷരം buf[20];
	പൂർണ്ണസംഖ്യ recnum;
	പൂർണ്ണസംഖ്യ i;

	എങ്കിൽ (!blob)
		തിരികെ 0;

	/*
	 * Create the node for bootstage.
	 * The address of flat device tree is set up by the command bootm.
	 */
	bootstage = fdt_add_subnode(blob, 0, "bootstage");
	എങ്കിൽ (bootstage < 0)
		തിരികെ -EINVAL;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	വേണ്ടി (recnum = data->rec_എണ്ണം - 1, i = 0; recnum >= 0; recnum--, i++) {
		ഘടന bootstage_record *rec = &data->record[recnum];
		പൂർണ്ണസംഖ്യ കണ്ണി;

		എങ്കിൽ (rec->id != BOOTSTAGE_ID_AWAKE && rec->time_us == 0)
			തുടരുക;

		കണ്ണി = fdt_add_subnode(blob, bootstage, simple_itoa(i));
		എങ്കിൽ (കണ്ണി < 0)
			നിർത്തുക;

		/* add properties to the node. */
		എങ്കിൽ (fdt_setprop_string(blob, കണ്ണി, "name",
				       get_record_name(buf, വലുപ്പം(buf), rec)))
			തിരികെ -EINVAL;

		/* Check if this is a 'mark' or 'accum' record */
		എങ്കിൽ (fdt_setprop_cell(blob, കണ്ണി,
				rec->start_us ? "accum" : "mark",
				rec->time_us))
			തിരികെ -EINVAL;
	}

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ bootstage_fdt_add_report(ശൂന്യം)
{
	എങ്കിൽ (add_bootstages_devicetree(working_fdt))
		puts("bootstage: Failed to add to device tree\n");

	തിരികെ 0;
}
#endif

ശൂന്യം bootstage_report(ശൂന്യം)
{
	ഘടന bootstage_data *data = gd->bootstage;
	ഘടന bootstage_record *rec = data->record;
	uint32_t prev;
	പൂർണ്ണസംഖ്യ i;

	printf("Timer summary in microseconds (%d records):\n",
	       data->rec_എണ്ണം);
	printf("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	prev = print_time_record(rec, 0);

	വേണ്ടി (i = 1, rec++; i < data->rec_എണ്ണം; i++, rec++) {
		എങ്കിൽ (rec->id && !rec->start_us)
			prev = print_time_record(rec, prev);
	}
	എങ്കിൽ (data->rec_എണ്ണം > RECORD_എണ്ണം)
		printf("Overflowed internal boot id table by %d entries\n"
		       "Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		       data->rec_എണ്ണം - RECORD_എണ്ണം);

	puts("\nAccumulated time:\n");
	വേണ്ടി (i = 0, rec = data->record; i < data->rec_എണ്ണം; i++, rec++) {
		എങ്കിൽ (rec->start_us)
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
സ്ഥിതം ശൂന്യം append_data(അക്ഷരം **ptrp, അക്ഷരം *end, സ്ഥിരം ശൂന്യം *data, പൂർണ്ണസംഖ്യ size)
{
	അക്ഷരം *ptr = *ptrp;

	*ptrp += size;
	എങ്കിൽ (*ptrp > end)
		തിരികെ;

	memcpy(ptr, data, size);
}

പൂർണ്ണസംഖ്യ bootstage_stash(ശൂന്യം *base, പൂർണ്ണസംഖ്യ size)
{
	സ്ഥിരം ഘടന bootstage_data *data = gd->bootstage;
	ഘടന bootstage_hdr *hdr = (ഘടന bootstage_hdr *)base;
	സ്ഥിരം ഘടന bootstage_record *rec;
	അക്ഷരം buf[20];
	അക്ഷരം *ptr = base, *end = ptr + size;
	പൂർണ്ണസംഖ്യ i;

	എങ്കിൽ (hdr + 1 > (ഘടന bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		തിരികെ -ENOSPC;
	}

	/* Write an arbitrary version number */
	hdr->version = BOOTSTAGE_VERSION;

	hdr->എണ്ണം = data->rec_എണ്ണം;
	hdr->size = 0;
	hdr->magic = BOOTSTAGE_MAGIC;
	hdr->next_id = data->next_id;
	ptr += വലുപ്പം(*hdr);

	/* Write the records, silently stopping when we run out of space */
	വേണ്ടി (rec = data->record, i = 0; i < data->rec_എണ്ണം; i++, rec++)
		append_data(&ptr, end, rec, വലുപ്പം(*rec));

	/* Write the name strings */
	വേണ്ടി (rec = data->record, i = 0; i < data->rec_എണ്ണം; i++, rec++) {
		സ്ഥിരം അക്ഷരം *name;

		name = get_record_name(buf, വലുപ്പം(buf), rec);
		append_data(&ptr, end, name, strlen(name) + 1);
	}

	/* Check for buffer overflow */
	എങ്കിൽ (ptr > end) {
		debug("%s: Not enough space for bootstage stash\n", __func__);
		തിരികെ -ENOSPC;
	}

	/* Update total data size */
	hdr->size = ptr - (അക്ഷരം *)base;
	debug("Stashed %d records\n", hdr->എണ്ണം);

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ bootstage_unstash(സ്ഥിരം ശൂന്യം *base, പൂർണ്ണസംഖ്യ size)
{
	സ്ഥിരം ഘടന bootstage_hdr *hdr = (ഘടന bootstage_hdr *)base;
	ഘടന bootstage_data *data = gd->bootstage;
	സ്ഥിരം അക്ഷരം *ptr = base, *end = ptr + size;
	ഘടന bootstage_record *rec;
	uint rec_size;
	പൂർണ്ണസംഖ്യ i;

	എങ്കിൽ (size == -1)
		end = (അക്ഷരം *)(~(uintptr_t)0);

	എങ്കിൽ (hdr + 1 > (ഘടന bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		തിരികെ -EPERM;
	}

	എങ്കിൽ (hdr->magic != BOOTSTAGE_MAGIC) {
		debug("%s: Invalid bootstage magic\n", __func__);
		തിരികെ -ENOENT;
	}

	എങ്കിൽ (ptr + hdr->size > end) {
		debug("%s: Bootstage data runs past buffer end\n", __func__);
		തിരികെ -ENOSPC;
	}

	എങ്കിൽ (hdr->എണ്ണം * വലുപ്പം(*rec) > hdr->size) {
		debug("%s: Bootstage has %d records needing %lu bytes, but "
			"only %d bytes is available\n", __func__, hdr->എണ്ണം,
		      (ulong)hdr->എണ്ണം * വലുപ്പം(*rec), hdr->size);
		തിരികെ -ENOSPC;
	}

	എങ്കിൽ (hdr->version != BOOTSTAGE_VERSION) {
		debug("%s: Bootstage data version %#0x unrecognised\n",
		      __func__, hdr->version);
		തിരികെ -EINVAL;
	}

	എങ്കിൽ (data->rec_എണ്ണം + hdr->എണ്ണം > RECORD_എണ്ണം) {
		debug("%s: Bootstage has %d records, we have space for %d\n"
			"Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		      __func__, hdr->എണ്ണം, RECORD_എണ്ണം - data->rec_എണ്ണം);
		തിരികെ -ENOSPC;
	}

	ptr += വലുപ്പം(*hdr);

	/* Read the records */
	rec_size = hdr->എണ്ണം * വലുപ്പം(*data->record);
	memcpy(data->record + data->rec_എണ്ണം, ptr, rec_size);

	/* Read the name strings */
	ptr += rec_size;
	വേണ്ടി (rec = data->record + data->rec_എണ്ണം, i = 0; i < hdr->എണ്ണം;
	     i++, rec++) {
		rec->name = ptr;
		എങ്കിൽ (xpl_phase() == PHASE_SPL)
			rec->name = strdup(ptr);

		/* Assume no data corruption here */
		ptr += strlen(ptr) + 1;
	}

	/* Mark the records as read */
	data->rec_എണ്ണം += hdr->എണ്ണം;
	data->next_id = hdr->next_id;
	debug("Unstashed %d records\n", hdr->എണ്ണം);

	തിരികെ 0;
}

#എങ്കിൽ IS_ENABLED(CONFIG_BOOTSTAGE_STASH)
പൂർണ്ണസംഖ്യ _bootstage_stash_default(ശൂന്യം)
{
	തിരികെ bootstage_stash(map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR, 0),
			       CONFIG_BOOTSTAGE_STASH_SIZE);
}

പൂർണ്ണസംഖ്യ _bootstage_unstash_default(ശൂന്യം)
{
	സ്ഥിരം ശൂന്യം *stash = map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR,
				       CONFIG_BOOTSTAGE_STASH_SIZE);

	തിരികെ bootstage_unstash(stash, CONFIG_BOOTSTAGE_STASH_SIZE);
}
#endif

പൂർണ്ണസംഖ്യ bootstage_get_size(bool add_strings)
{
	പൂർണ്ണസംഖ്യ size;

	size = വലുപ്പം(ഘടന bootstage_data);
	എങ്കിൽ (add_strings) {
		ഘടന bootstage_data *data = gd->bootstage;
		ഘടന bootstage_record *rec;
		പൂർണ്ണസംഖ്യ i;

		വേണ്ടി (rec = data->record, i = 0; i < data->rec_എണ്ണം; i++, rec++)
			എങ്കിൽ (rec->name)
				size += strlen(rec->name) + 1;
	}

	തിരികെ size;
}

പൂർണ്ണസംഖ്യ bootstage_init(bool first)
{
	ഘടന bootstage_data *data;
	പൂർണ്ണസംഖ്യ size = വലുപ്പം(ഘടന bootstage_data);

	gd->bootstage = (ഘടന bootstage_data *)malloc(size);
	എങ്കിൽ (!gd->bootstage)
		തിരികെ -ENOMEM;
	data = gd->bootstage;
	memset(data, '\0', size);
	എങ്കിൽ (first) {
		data->next_id = BOOTSTAGE_ID_USER;
		bootstage_add_record(BOOTSTAGE_ID_AWAKE, "reset", 0, 0);
	}

	തിരികെ 0;
}
