// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 */

/*
 * This module records the progress of boot and arbitrary commands, and
 * permits accurate timestamping of each.
 */

#define LOG_CATEGORY	LOGC_BOOT

#अन्तर्भाव <bootstage.h>
#अन्तर्भाव <hang.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <sort.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <linux/compiler.h>
#अन्तर्भाव <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

गणना {
	RECORD_गिनती = CONFIG_VAL(BOOTSTAGE_RECORD_गिनती),
};

संरचना bootstage_record {
	ulong time_us;
	uint32_t start_us;
	ध्रुव वर्ण *name;
	पूर्णाङ्क flags;		/* see enum bootstage_flags */
	गणना bootstage_id id;
};

संरचना bootstage_data {
	uint rec_गिनती;
	uint next_id;
	संरचना bootstage_record record[RECORD_गिनती];
};

गणना {
	BOOTSTAGE_VERSION	= 0,
	BOOTSTAGE_MAGIC		= 0xb00757a3,
	BOOTSTAGE_DIGITS	= 9,
};

संरचना bootstage_hdr {
	u32 version;		/* BOOTSTAGE_VERSION */
	u32 गिनती;		/* Number of records */
	u32 size;		/* Total data size (non-zero if valid) */
	u32 magic;		/* Magic number */
	u32 next_id;		/* Next ID to use for bootstage */
};

पूर्णाङ्क bootstage_relocate(शून्य *to)
{
	संरचना bootstage_data *data;
	पूर्णाङ्क i;
	वर्ण *ptr;

	debug("Copying bootstage from %p to %p\n", gd->bootstage, to);
	स_नकल(to, gd->bootstage, परिमाण(संरचना bootstage_data));
	data = gd->bootstage = to;

	/* Figure out where to relocate the strings to */
	ptr = (वर्ण *)(data + 1);

	/*
	 * Duplicate all strings.  They may point to an old location in the
	 * program .text section that can eventually get trashed.
	 */
	debug("Relocating %d records\n", data->rec_गिनती);
	प्रति (i = 0; i < data->rec_गिनती; i++) {
		ध्रुव वर्ण *from = data->record[i].name;

		यदि (!from)
			अनुवर्त;
		म_नकल(ptr, from);
		data->record[i].name = ptr;
		ptr += म_माप(ptr) + 1;
	}

	प्रत्यागम 0;
}

संरचना bootstage_record *find_id(संरचना bootstage_data *data,
				 गणना bootstage_id id)
{
	संरचना bootstage_record *rec;
	संरचना bootstage_record *end;

	प्रति (rec = data->record, end = rec + data->rec_गिनती; rec < end;
	     rec++) {
		यदि (rec->id == id)
			प्रत्यागम rec;
	}

	प्रत्यागम रिक्त;
}

संरचना bootstage_record *ensure_id(संरचना bootstage_data *data,
				   गणना bootstage_id id)
{
	संरचना bootstage_record *rec;

	rec = find_id(data, id);
	यदि (!rec && data->rec_गिनती < RECORD_गिनती) {
		rec = &data->record[data->rec_गिनती++];
		rec->id = id;
		प्रत्यागम rec;
	}

	प्रत्यागम rec;
}

ulong bootstage_add_record(गणना bootstage_id id, ध्रुव वर्ण *name,
			   पूर्णाङ्क flags, ulong mark)
{
	संरचना bootstage_data *data = gd->bootstage;
	संरचना bootstage_record *rec;

	/*
	 * initf_bootstage() is called very early during boot but since hang()
	 * calls bootstage_error() we can be called before bootstage is set up.
	 * Add a check to avoid this.
	 */
	यदि (!data)
		प्रत्यागम mark;
	यदि (flags & BOOTSTAGEF_ALLOC)
		id = data->next_id++;

	/* Only record the first event for each */
	rec = find_id(data, id);
	यदि (!rec) {
		यदि (data->rec_गिनती < RECORD_गिनती) {
			rec = &data->record[data->rec_गिनती++];
			rec->time_us = mark;
			rec->name = name;
			rec->flags = flags;
			rec->id = id;
		} अन्यथा {
			log_warning("Bootstage space exhausted\n");
		}
	}

	/* Tell the board about this progress */
	show_boot_progress(flags & BOOTSTAGEF_ERROR ? -id : id);

	प्रत्यागम mark;
}

ulong bootstage_error_name(गणना bootstage_id id, ध्रुव वर्ण *name)
{
	प्रत्यागम bootstage_add_record(id, name, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(गणना bootstage_id id, ध्रुव वर्ण *name)
{
	पूर्णाङ्क flags = 0;

	यदि (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;

	प्रत्यागम bootstage_add_record(id, name, flags, timer_get_boot_us());
}

ulong bootstage_mark_code(ध्रुव वर्ण *file, ध्रुव वर्ण *func, पूर्णाङ्क linenum)
{
	वर्ण *str, *p;
	__maybe_unused वर्ण *end;
	पूर्णाङ्क len = 0;

	/* First work out the length we need to allocate */
	यदि (linenum != -1)
		len = 11;
	यदि (func)
		len += म_माप(func);
	यदि (file)
		len += म_माप(file);

	str = दो_स्मृति(len + 1);
	p = str;
	end = p + len;
	यदि (file)
		p += snprintf(p, end - p, "%s,", file);
	यदि (linenum != -1)
		p += snprintf(p, end - p, "%d", linenum);
	यदि (func)
		p += snprintf(p, end - p, ": %s", func);

	प्रत्यागम bootstage_mark_name(BOOTSTAGE_ID_ALLOC, str);
}

uint32_t bootstage_start(गणना bootstage_id id, ध्रुव वर्ण *name)
{
	संरचना bootstage_data *data = gd->bootstage;
	संरचना bootstage_record *rec = ensure_id(data, id);
	ulong start_us = timer_get_boot_us();

	यदि (rec) {
		rec->start_us = start_us;
		rec->name = name;
	}

	प्रत्यागम start_us;
}

uint32_t bootstage_accum(गणना bootstage_id id)
{
	संरचना bootstage_data *data = gd->bootstage;
	संरचना bootstage_record *rec = ensure_id(data, id);
	uint32_t duration;

	यदि (!rec)
		प्रत्यागम 0;
	duration = (uint32_t)timer_get_boot_us() - rec->start_us;
	rec->time_us += duration;

	प्रत्यागम duration;
}

/**
 * Get a record name as a printable string
 *
 * @param buf	Buffer to put name if needed
 * @param len	Length of buffer
 * @param rec	Boot stage record to get the name from
 * Return: pointer to name, either from the record or pointing to buf.
 */
स्थिर ध्रुव वर्ण *get_record_name(वर्ण *buf, पूर्णाङ्क len,
				   ध्रुव संरचना bootstage_record *rec)
{
	यदि (rec->name)
		प्रत्यागम rec->name;
	अन्यथा यदि (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	अन्यथा
		snprintf(buf, len, "id=%d", rec->id);

	प्रत्यागम buf;
}

स्थिर uint32_t print_time_record(संरचना bootstage_record *rec, uint32_t prev)
{
	वर्ण buf[20];

	यदि (prev == -1U) {
		म_लिखो("%11s", "");
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
	} अन्यथा {
		यदि (prev > rec->time_us)
			prev = 0;
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
		print_grouped_ull(rec->time_us - prev, BOOTSTAGE_DIGITS);
	}
	म_लिखो("  %s\n", get_record_name(buf, परिमाण(buf), rec));

	प्रत्यागम rec->time_us;
}

#ifdef CONFIG_OF_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * Return: 0 on success, != 0 on failure.
 */
स्थिर पूर्णाङ्क add_bootstages_devicetree(संरचना fdt_header *blob)
{
	संरचना bootstage_data *data = gd->bootstage;
	पूर्णाङ्क bootstage;
	वर्ण buf[20];
	पूर्णाङ्क recnum;
	पूर्णाङ्क i;

	यदि (!blob)
		प्रत्यागम 0;

	/*
	 * Create the node for bootstage.
	 * The address of flat device tree is set up by the command bootm.
	 */
	bootstage = fdt_add_subnode(blob, 0, "bootstage");
	यदि (bootstage < 0)
		प्रत्यागम -EINVAL;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	प्रति (recnum = data->rec_गिनती - 1, i = 0; recnum >= 0; recnum--, i++) {
		संरचना bootstage_record *rec = &data->record[recnum];
		पूर्णाङ्क गाँठ;

		यदि (rec->id != BOOTSTAGE_ID_AWAKE && rec->time_us == 0)
			अनुवर्त;

		गाँठ = fdt_add_subnode(blob, bootstage, simple_itoa(i));
		यदि (गाँठ < 0)
			भङ्ग;

		/* add properties to the node. */
		यदि (fdt_setprop_string(blob, गाँठ, "name",
				       get_record_name(buf, परिमाण(buf), rec)))
			प्रत्यागम -EINVAL;

		/* Check if this is a 'mark' or 'accum' record */
		यदि (fdt_setprop_cell(blob, गाँठ,
				rec->start_us ? "accum" : "mark",
				rec->time_us))
			प्रत्यागम -EINVAL;
	}

	प्रत्यागम 0;
}

पूर्णाङ्क bootstage_fdt_add_report(शून्य)
{
	यदि (add_bootstages_devicetree(working_fdt))
		माला_दो("bootstage: Failed to add to device tree\n");

	प्रत्यागम 0;
}
#endif

शून्य bootstage_report(शून्य)
{
	संरचना bootstage_data *data = gd->bootstage;
	संरचना bootstage_record *rec = data->record;
	uint32_t prev;
	पूर्णाङ्क i;

	म_लिखो("Timer summary in microseconds (%d records):\n",
	       data->rec_गिनती);
	म_लिखो("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	prev = print_time_record(rec, 0);

	प्रति (i = 1, rec++; i < data->rec_गिनती; i++, rec++) {
		यदि (rec->id && !rec->start_us)
			prev = print_time_record(rec, prev);
	}
	यदि (data->rec_गिनती > RECORD_गिनती)
		म_लिखो("Overflowed internal boot id table by %d entries\n"
		       "Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		       data->rec_गिनती - RECORD_गिनती);

	माला_दो("\nAccumulated time:\n");
	प्रति (i = 0, rec = data->record; i < data->rec_गिनती; i++, rec++) {
		यदि (rec->start_us)
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
स्थिर शून्य append_data(वर्ण **ptrp, वर्ण *end, ध्रुव शून्य *data, पूर्णाङ्क size)
{
	वर्ण *ptr = *ptrp;

	*ptrp += size;
	यदि (*ptrp > end)
		प्रत्यागम;

	स_नकल(ptr, data, size);
}

पूर्णाङ्क bootstage_stash(शून्य *base, पूर्णाङ्क size)
{
	ध्रुव संरचना bootstage_data *data = gd->bootstage;
	संरचना bootstage_hdr *hdr = (संरचना bootstage_hdr *)base;
	ध्रुव संरचना bootstage_record *rec;
	वर्ण buf[20];
	वर्ण *ptr = base, *end = ptr + size;
	पूर्णाङ्क i;

	यदि (hdr + 1 > (संरचना bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		प्रत्यागम -ENOSPC;
	}

	/* Write an arbitrary version number */
	hdr->version = BOOTSTAGE_VERSION;

	hdr->गिनती = data->rec_गिनती;
	hdr->size = 0;
	hdr->magic = BOOTSTAGE_MAGIC;
	hdr->next_id = data->next_id;
	ptr += परिमाण(*hdr);

	/* Write the records, silently stopping when we run out of space */
	प्रति (rec = data->record, i = 0; i < data->rec_गिनती; i++, rec++)
		append_data(&ptr, end, rec, परिमाण(*rec));

	/* Write the name strings */
	प्रति (rec = data->record, i = 0; i < data->rec_गिनती; i++, rec++) {
		ध्रुव वर्ण *name;

		name = get_record_name(buf, परिमाण(buf), rec);
		append_data(&ptr, end, name, म_माप(name) + 1);
	}

	/* Check for buffer overflow */
	यदि (ptr > end) {
		debug("%s: Not enough space for bootstage stash\n", __func__);
		प्रत्यागम -ENOSPC;
	}

	/* Update total data size */
	hdr->size = ptr - (वर्ण *)base;
	debug("Stashed %d records\n", hdr->गिनती);

	प्रत्यागम 0;
}

पूर्णाङ्क bootstage_unstash(ध्रुव शून्य *base, पूर्णाङ्क size)
{
	ध्रुव संरचना bootstage_hdr *hdr = (संरचना bootstage_hdr *)base;
	संरचना bootstage_data *data = gd->bootstage;
	ध्रुव वर्ण *ptr = base, *end = ptr + size;
	संरचना bootstage_record *rec;
	uint rec_size;
	पूर्णाङ्क i;

	यदि (size == -1)
		end = (वर्ण *)(~(uintptr_t)0);

	यदि (hdr + 1 > (संरचना bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		प्रत्यागम -EPERM;
	}

	यदि (hdr->magic != BOOTSTAGE_MAGIC) {
		debug("%s: Invalid bootstage magic\n", __func__);
		प्रत्यागम -ENOENT;
	}

	यदि (ptr + hdr->size > end) {
		debug("%s: Bootstage data runs past buffer end\n", __func__);
		प्रत्यागम -ENOSPC;
	}

	यदि (hdr->गिनती * परिमाण(*rec) > hdr->size) {
		debug("%s: Bootstage has %d records needing %lu bytes, but "
			"only %d bytes is available\n", __func__, hdr->गिनती,
		      (ulong)hdr->गिनती * परिमाण(*rec), hdr->size);
		प्रत्यागम -ENOSPC;
	}

	यदि (hdr->version != BOOTSTAGE_VERSION) {
		debug("%s: Bootstage data version %#0x unrecognised\n",
		      __func__, hdr->version);
		प्रत्यागम -EINVAL;
	}

	यदि (data->rec_गिनती + hdr->गिनती > RECORD_गिनती) {
		debug("%s: Bootstage has %d records, we have space for %d\n"
			"Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		      __func__, hdr->गिनती, RECORD_गिनती - data->rec_गिनती);
		प्रत्यागम -ENOSPC;
	}

	ptr += परिमाण(*hdr);

	/* Read the records */
	rec_size = hdr->गिनती * परिमाण(*data->record);
	स_नकल(data->record + data->rec_गिनती, ptr, rec_size);

	/* Read the name strings */
	ptr += rec_size;
	प्रति (rec = data->record + data->rec_गिनती, i = 0; i < hdr->गिनती;
	     i++, rec++) {
		rec->name = ptr;
		यदि (xpl_phase() == PHASE_SPL)
			rec->name = strdup(ptr);

		/* Assume no data corruption here */
		ptr += म_माप(ptr) + 1;
	}

	/* Mark the records as read */
	data->rec_गिनती += hdr->गिनती;
	data->next_id = hdr->next_id;
	debug("Unstashed %d records\n", hdr->गिनती);

	प्रत्यागम 0;
}

#यदि IS_ENABLED(CONFIG_BOOTSTAGE_STASH)
पूर्णाङ्क _bootstage_stash_default(शून्य)
{
	प्रत्यागम bootstage_stash(map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR, 0),
			       CONFIG_BOOTSTAGE_STASH_SIZE);
}

पूर्णाङ्क _bootstage_unstash_default(शून्य)
{
	ध्रुव शून्य *stash = map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR,
				       CONFIG_BOOTSTAGE_STASH_SIZE);

	प्रत्यागम bootstage_unstash(stash, CONFIG_BOOTSTAGE_STASH_SIZE);
}
#endif

पूर्णाङ्क bootstage_get_size(bool add_strings)
{
	पूर्णाङ्क size;

	size = परिमाण(संरचना bootstage_data);
	यदि (add_strings) {
		संरचना bootstage_data *data = gd->bootstage;
		संरचना bootstage_record *rec;
		पूर्णाङ्क i;

		प्रति (rec = data->record, i = 0; i < data->rec_गिनती; i++, rec++)
			यदि (rec->name)
				size += म_माप(rec->name) + 1;
	}

	प्रत्यागम size;
}

पूर्णाङ्क bootstage_init(bool first)
{
	संरचना bootstage_data *data;
	पूर्णाङ्क size = परिमाण(संरचना bootstage_data);

	gd->bootstage = (संरचना bootstage_data *)दो_स्मृति(size);
	यदि (!gd->bootstage)
		प्रत्यागम -ENOMEM;
	data = gd->bootstage;
	स_रखो(data, '\0', size);
	यदि (first) {
		data->next_id = BOOTSTAGE_ID_USER;
		bootstage_add_record(BOOTSTAGE_ID_AWAKE, "reset", 0, 0);
	}

	प्रत्यागम 0;
}
