// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 */

/*
 * This module records the progress of boot and arbitrary commands, and
 * permits accurate timestamping of each.
 */

#define LOG_CATEGORY	LOGC_BOOT

#અન્તર્ભાવ <bootstage.h>
#અન્તર્ભાવ <hang.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <malloc.h>
#અન્તર્ભાવ <sort.h>
#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <asm/global_data.h>
#અન્તર્ભાવ <linux/compiler.h>
#અન્તર્ભાવ <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

ગણના {
	RECORD_ગણતરી = CONFIG_VAL(BOOTSTAGE_RECORD_ગણતરી),
};

સંરચના bootstage_record {
	ulong time_us;
	uint32_t start_us;
	અચલ અક્ષર *name;
	પૂર્ણાંક flags;		/* see enum bootstage_flags */
	ગણના bootstage_id id;
};

સંરચના bootstage_data {
	uint rec_ગણતરી;
	uint next_id;
	સંરચના bootstage_record record[RECORD_ગણતરી];
};

ગણના {
	BOOTSTAGE_VERSION	= 0,
	BOOTSTAGE_MAGIC		= 0xb00757a3,
	BOOTSTAGE_DIGITS	= 9,
};

સંરચના bootstage_hdr {
	u32 version;		/* BOOTSTAGE_VERSION */
	u32 ગણતરી;		/* Number of records */
	u32 size;		/* Total data size (non-zero if valid) */
	u32 magic;		/* Magic number */
	u32 next_id;		/* Next ID to use for bootstage */
};

પૂર્ણાંક bootstage_relocate(શૂન્ય *to)
{
	સંરચના bootstage_data *data;
	પૂર્ણાંક i;
	અક્ષર *ptr;

	debug("Copying bootstage from %p to %p\n", gd->bootstage, to);
	memcpy(to, gd->bootstage, કદ(સંરચના bootstage_data));
	data = gd->bootstage = to;

	/* Figure out where to relocate the strings to */
	ptr = (અક્ષર *)(data + 1);

	/*
	 * Duplicate all strings.  They may point to an old location in the
	 * program .text section that can eventually get trashed.
	 */
	debug("Relocating %d records\n", data->rec_ગણતરી);
	માટે (i = 0; i < data->rec_ગણતરી; i++) {
		અચલ અક્ષર *from = data->record[i].name;

		જો (!from)
			ચાલુ;
		strcpy(ptr, from);
		data->record[i].name = ptr;
		ptr += strlen(ptr) + 1;
	}

	પાછા 0;
}

સંરચના bootstage_record *find_id(સંરચના bootstage_data *data,
				 ગણના bootstage_id id)
{
	સંરચના bootstage_record *rec;
	સંરચના bootstage_record *end;

	માટે (rec = data->record, end = rec + data->rec_ગણતરી; rec < end;
	     rec++) {
		જો (rec->id == id)
			પાછા rec;
	}

	પાછા ખાલી;
}

સંરચના bootstage_record *ensure_id(સંરચના bootstage_data *data,
				   ગણના bootstage_id id)
{
	સંરચના bootstage_record *rec;

	rec = find_id(data, id);
	જો (!rec && data->rec_ગણતરી < RECORD_ગણતરી) {
		rec = &data->record[data->rec_ગણતરી++];
		rec->id = id;
		પાછા rec;
	}

	પાછા rec;
}

ulong bootstage_add_record(ગણના bootstage_id id, અચલ અક્ષર *name,
			   પૂર્ણાંક flags, ulong mark)
{
	સંરચના bootstage_data *data = gd->bootstage;
	સંરચના bootstage_record *rec;

	/*
	 * initf_bootstage() is called very early during boot but since hang()
	 * calls bootstage_error() we can be called before bootstage is set up.
	 * Add a check to avoid this.
	 */
	જો (!data)
		પાછા mark;
	જો (flags & BOOTSTAGEF_ALLOC)
		id = data->next_id++;

	/* Only record the first event for each */
	rec = find_id(data, id);
	જો (!rec) {
		જો (data->rec_ગણતરી < RECORD_ગણતરી) {
			rec = &data->record[data->rec_ગણતરી++];
			rec->time_us = mark;
			rec->name = name;
			rec->flags = flags;
			rec->id = id;
		} નહીંતર {
			log_warning("Bootstage space exhausted\n");
		}
	}

	/* Tell the board about this progress */
	show_boot_progress(flags & BOOTSTAGEF_ERROR ? -id : id);

	પાછા mark;
}

ulong bootstage_error_name(ગણના bootstage_id id, અચલ અક્ષર *name)
{
	પાછા bootstage_add_record(id, name, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(ગણના bootstage_id id, અચલ અક્ષર *name)
{
	પૂર્ણાંક flags = 0;

	જો (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;

	પાછા bootstage_add_record(id, name, flags, timer_get_boot_us());
}

ulong bootstage_mark_code(અચલ અક્ષર *file, અચલ અક્ષર *func, પૂર્ણાંક linenum)
{
	અક્ષર *str, *p;
	__maybe_unused અક્ષર *end;
	પૂર્ણાંક len = 0;

	/* First work out the length we need to allocate */
	જો (linenum != -1)
		len = 11;
	જો (func)
		len += strlen(func);
	જો (file)
		len += strlen(file);

	str = malloc(len + 1);
	p = str;
	end = p + len;
	જો (file)
		p += snprintf(p, end - p, "%s,", file);
	જો (linenum != -1)
		p += snprintf(p, end - p, "%d", linenum);
	જો (func)
		p += snprintf(p, end - p, ": %s", func);

	પાછા bootstage_mark_name(BOOTSTAGE_ID_ALLOC, str);
}

uint32_t bootstage_start(ગણના bootstage_id id, અચલ અક્ષર *name)
{
	સંરચના bootstage_data *data = gd->bootstage;
	સંરચના bootstage_record *rec = ensure_id(data, id);
	ulong start_us = timer_get_boot_us();

	જો (rec) {
		rec->start_us = start_us;
		rec->name = name;
	}

	પાછા start_us;
}

uint32_t bootstage_accum(ગણના bootstage_id id)
{
	સંરચના bootstage_data *data = gd->bootstage;
	સંરચના bootstage_record *rec = ensure_id(data, id);
	uint32_t duration;

	જો (!rec)
		પાછા 0;
	duration = (uint32_t)timer_get_boot_us() - rec->start_us;
	rec->time_us += duration;

	પાછા duration;
}

/**
 * Get a record name as a printable string
 *
 * @param buf	Buffer to put name if needed
 * @param len	Length of buffer
 * @param rec	Boot stage record to get the name from
 * Return: pointer to name, either from the record or pointing to buf.
 */
સ્થિર અચલ અક્ષર *get_record_name(અક્ષર *buf, પૂર્ણાંક len,
				   અચલ સંરચના bootstage_record *rec)
{
	જો (rec->name)
		પાછા rec->name;
	નહીંતર જો (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	નહીંતર
		snprintf(buf, len, "id=%d", rec->id);

	પાછા buf;
}

સ્થિર uint32_t print_time_record(સંરચના bootstage_record *rec, uint32_t prev)
{
	અક્ષર buf[20];

	જો (prev == -1U) {
		printf("%11s", "");
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
	} નહીંતર {
		જો (prev > rec->time_us)
			prev = 0;
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
		print_grouped_ull(rec->time_us - prev, BOOTSTAGE_DIGITS);
	}
	printf("  %s\n", get_record_name(buf, કદ(buf), rec));

	પાછા rec->time_us;
}

#ifdef CONFIG_OF_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * Return: 0 on success, != 0 on failure.
 */
સ્થિર પૂર્ણાંક add_bootstages_devicetree(સંરચના fdt_header *blob)
{
	સંરચના bootstage_data *data = gd->bootstage;
	પૂર્ણાંક bootstage;
	અક્ષર buf[20];
	પૂર્ણાંક recnum;
	પૂર્ણાંક i;

	જો (!blob)
		પાછા 0;

	/*
	 * Create the node for bootstage.
	 * The address of flat device tree is set up by the command bootm.
	 */
	bootstage = fdt_add_subnode(blob, 0, "bootstage");
	જો (bootstage < 0)
		પાછા -EINVAL;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	માટે (recnum = data->rec_ગણતરી - 1, i = 0; recnum >= 0; recnum--, i++) {
		સંરચના bootstage_record *rec = &data->record[recnum];
		પૂર્ણાંક ગાંઠ;

		જો (rec->id != BOOTSTAGE_ID_AWAKE && rec->time_us == 0)
			ચાલુ;

		ગાંઠ = fdt_add_subnode(blob, bootstage, simple_itoa(i));
		જો (ગાંઠ < 0)
			તોડો;

		/* add properties to the node. */
		જો (fdt_setprop_string(blob, ગાંઠ, "name",
				       get_record_name(buf, કદ(buf), rec)))
			પાછા -EINVAL;

		/* Check if this is a 'mark' or 'accum' record */
		જો (fdt_setprop_cell(blob, ગાંઠ,
				rec->start_us ? "accum" : "mark",
				rec->time_us))
			પાછા -EINVAL;
	}

	પાછા 0;
}

પૂર્ણાંક bootstage_fdt_add_report(શૂન્ય)
{
	જો (add_bootstages_devicetree(working_fdt))
		puts("bootstage: Failed to add to device tree\n");

	પાછા 0;
}
#endif

શૂન્ય bootstage_report(શૂન્ય)
{
	સંરચના bootstage_data *data = gd->bootstage;
	સંરચના bootstage_record *rec = data->record;
	uint32_t prev;
	પૂર્ણાંક i;

	printf("Timer summary in microseconds (%d records):\n",
	       data->rec_ગણતરી);
	printf("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	prev = print_time_record(rec, 0);

	માટે (i = 1, rec++; i < data->rec_ગણતરી; i++, rec++) {
		જો (rec->id && !rec->start_us)
			prev = print_time_record(rec, prev);
	}
	જો (data->rec_ગણતરી > RECORD_ગણતરી)
		printf("Overflowed internal boot id table by %d entries\n"
		       "Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		       data->rec_ગણતરી - RECORD_ગણતરી);

	puts("\nAccumulated time:\n");
	માટે (i = 0, rec = data->record; i < data->rec_ગણતરી; i++, rec++) {
		જો (rec->start_us)
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
સ્થિર શૂન્ય append_data(અક્ષર **ptrp, અક્ષર *end, અચલ શૂન્ય *data, પૂર્ણાંક size)
{
	અક્ષર *ptr = *ptrp;

	*ptrp += size;
	જો (*ptrp > end)
		પાછા;

	memcpy(ptr, data, size);
}

પૂર્ણાંક bootstage_stash(શૂન્ય *base, પૂર્ણાંક size)
{
	અચલ સંરચના bootstage_data *data = gd->bootstage;
	સંરચના bootstage_hdr *hdr = (સંરચના bootstage_hdr *)base;
	અચલ સંરચના bootstage_record *rec;
	અક્ષર buf[20];
	અક્ષર *ptr = base, *end = ptr + size;
	પૂર્ણાંક i;

	જો (hdr + 1 > (સંરચના bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		પાછા -ENOSPC;
	}

	/* Write an arbitrary version number */
	hdr->version = BOOTSTAGE_VERSION;

	hdr->ગણતરી = data->rec_ગણતરી;
	hdr->size = 0;
	hdr->magic = BOOTSTAGE_MAGIC;
	hdr->next_id = data->next_id;
	ptr += કદ(*hdr);

	/* Write the records, silently stopping when we run out of space */
	માટે (rec = data->record, i = 0; i < data->rec_ગણતરી; i++, rec++)
		append_data(&ptr, end, rec, કદ(*rec));

	/* Write the name strings */
	માટે (rec = data->record, i = 0; i < data->rec_ગણતરી; i++, rec++) {
		અચલ અક્ષર *name;

		name = get_record_name(buf, કદ(buf), rec);
		append_data(&ptr, end, name, strlen(name) + 1);
	}

	/* Check for buffer overflow */
	જો (ptr > end) {
		debug("%s: Not enough space for bootstage stash\n", __func__);
		પાછા -ENOSPC;
	}

	/* Update total data size */
	hdr->size = ptr - (અક્ષર *)base;
	debug("Stashed %d records\n", hdr->ગણતરી);

	પાછા 0;
}

પૂર્ણાંક bootstage_unstash(અચલ શૂન્ય *base, પૂર્ણાંક size)
{
	અચલ સંરચના bootstage_hdr *hdr = (સંરચના bootstage_hdr *)base;
	સંરચના bootstage_data *data = gd->bootstage;
	અચલ અક્ષર *ptr = base, *end = ptr + size;
	સંરચના bootstage_record *rec;
	uint rec_size;
	પૂર્ણાંક i;

	જો (size == -1)
		end = (અક્ષર *)(~(uintptr_t)0);

	જો (hdr + 1 > (સંરચના bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		પાછા -EPERM;
	}

	જો (hdr->magic != BOOTSTAGE_MAGIC) {
		debug("%s: Invalid bootstage magic\n", __func__);
		પાછા -ENOENT;
	}

	જો (ptr + hdr->size > end) {
		debug("%s: Bootstage data runs past buffer end\n", __func__);
		પાછા -ENOSPC;
	}

	જો (hdr->ગણતરી * કદ(*rec) > hdr->size) {
		debug("%s: Bootstage has %d records needing %lu bytes, but "
			"only %d bytes is available\n", __func__, hdr->ગણતરી,
		      (ulong)hdr->ગણતરી * કદ(*rec), hdr->size);
		પાછા -ENOSPC;
	}

	જો (hdr->version != BOOTSTAGE_VERSION) {
		debug("%s: Bootstage data version %#0x unrecognised\n",
		      __func__, hdr->version);
		પાછા -EINVAL;
	}

	જો (data->rec_ગણતરી + hdr->ગણતરી > RECORD_ગણતરી) {
		debug("%s: Bootstage has %d records, we have space for %d\n"
			"Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		      __func__, hdr->ગણતરી, RECORD_ગણતરી - data->rec_ગણતરી);
		પાછા -ENOSPC;
	}

	ptr += કદ(*hdr);

	/* Read the records */
	rec_size = hdr->ગણતરી * કદ(*data->record);
	memcpy(data->record + data->rec_ગણતરી, ptr, rec_size);

	/* Read the name strings */
	ptr += rec_size;
	માટે (rec = data->record + data->rec_ગણતરી, i = 0; i < hdr->ગણતરી;
	     i++, rec++) {
		rec->name = ptr;
		જો (xpl_phase() == PHASE_SPL)
			rec->name = strdup(ptr);

		/* Assume no data corruption here */
		ptr += strlen(ptr) + 1;
	}

	/* Mark the records as read */
	data->rec_ગણતરી += hdr->ગણતરી;
	data->next_id = hdr->next_id;
	debug("Unstashed %d records\n", hdr->ગણતરી);

	પાછા 0;
}

#જો IS_ENABLED(CONFIG_BOOTSTAGE_STASH)
પૂર્ણાંક _bootstage_stash_default(શૂન્ય)
{
	પાછા bootstage_stash(map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR, 0),
			       CONFIG_BOOTSTAGE_STASH_SIZE);
}

પૂર્ણાંક _bootstage_unstash_default(શૂન્ય)
{
	અચલ શૂન્ય *stash = map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR,
				       CONFIG_BOOTSTAGE_STASH_SIZE);

	પાછા bootstage_unstash(stash, CONFIG_BOOTSTAGE_STASH_SIZE);
}
#endif

પૂર્ણાંક bootstage_get_size(bool add_strings)
{
	પૂર્ણાંક size;

	size = કદ(સંરચના bootstage_data);
	જો (add_strings) {
		સંરચના bootstage_data *data = gd->bootstage;
		સંરચના bootstage_record *rec;
		પૂર્ણાંક i;

		માટે (rec = data->record, i = 0; i < data->rec_ગણતરી; i++, rec++)
			જો (rec->name)
				size += strlen(rec->name) + 1;
	}

	પાછા size;
}

પૂર્ણાંક bootstage_init(bool first)
{
	સંરચના bootstage_data *data;
	પૂર્ણાંક size = કદ(સંરચના bootstage_data);

	gd->bootstage = (સંરચના bootstage_data *)malloc(size);
	જો (!gd->bootstage)
		પાછા -ENOMEM;
	data = gd->bootstage;
	memset(data, '\0', size);
	જો (first) {
		data->next_id = BOOTSTAGE_ID_USER;
		bootstage_add_record(BOOTSTAGE_ID_AWAKE, "reset", 0, 0);
	}

	પાછા 0;
}
