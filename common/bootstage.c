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
	RECORD_मोजणी = CONFIG_VAL(BOOTSTAGE_RECORD_मोजणी),
};

संरचना bootstage_record {
	ulong time_us;
	uint32_t start_us;
	स्थिरांक अक्षर *name;
	पूर्णांक flags;		/* see enum bootstage_flags */
	गणना bootstage_id id;
};

संरचना bootstage_data {
	uint rec_मोजणी;
	uint next_id;
	संरचना bootstage_record record[RECORD_मोजणी];
};

गणना {
	BOOTSTAGE_VERSION	= 0,
	BOOTSTAGE_MAGIC		= 0xb00757a3,
	BOOTSTAGE_DIGITS	= 9,
};

संरचना bootstage_hdr {
	u32 version;		/* BOOTSTAGE_VERSION */
	u32 मोजणी;		/* Number of records */
	u32 size;		/* Total data size (non-zero if valid) */
	u32 magic;		/* Magic number */
	u32 next_id;		/* Next ID to use for bootstage */
};

पूर्णांक bootstage_relocate(शून्य *to)
{
	संरचना bootstage_data *data;
	पूर्णांक i;
	अक्षर *ptr;

	debug("Copying bootstage from %p to %p\n", gd->bootstage, to);
	स_नकल(to, gd->bootstage, आकार(संरचना bootstage_data));
	data = gd->bootstage = to;

	/* Figure out where to relocate the strings to */
	ptr = (अक्षर *)(data + 1);

	/*
	 * Duplicate all strings.  They may point to an old location in the
	 * program .text section that can eventually get trashed.
	 */
	debug("Relocating %d records\n", data->rec_मोजणी);
	साठी (i = 0; i < data->rec_मोजणी; i++) {
		स्थिरांक अक्षर *from = data->record[i].name;

		जर (!from)
			सुरूठेवा;
		म_नकल(ptr, from);
		data->record[i].name = ptr;
		ptr += म_माप(ptr) + 1;
	}

	परत 0;
}

संरचना bootstage_record *find_id(संरचना bootstage_data *data,
				 गणना bootstage_id id)
{
	संरचना bootstage_record *rec;
	संरचना bootstage_record *end;

	साठी (rec = data->record, end = rec + data->rec_मोजणी; rec < end;
	     rec++) {
		जर (rec->id == id)
			परत rec;
	}

	परत रिक्त;
}

संरचना bootstage_record *ensure_id(संरचना bootstage_data *data,
				   गणना bootstage_id id)
{
	संरचना bootstage_record *rec;

	rec = find_id(data, id);
	जर (!rec && data->rec_मोजणी < RECORD_मोजणी) {
		rec = &data->record[data->rec_मोजणी++];
		rec->id = id;
		परत rec;
	}

	परत rec;
}

ulong bootstage_add_record(गणना bootstage_id id, स्थिरांक अक्षर *name,
			   पूर्णांक flags, ulong mark)
{
	संरचना bootstage_data *data = gd->bootstage;
	संरचना bootstage_record *rec;

	/*
	 * initf_bootstage() is called very early during boot but since hang()
	 * calls bootstage_error() we can be called before bootstage is set up.
	 * Add a check to avoid this.
	 */
	जर (!data)
		परत mark;
	जर (flags & BOOTSTAGEF_ALLOC)
		id = data->next_id++;

	/* Only record the first event for each */
	rec = find_id(data, id);
	जर (!rec) {
		जर (data->rec_मोजणी < RECORD_मोजणी) {
			rec = &data->record[data->rec_मोजणी++];
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

	परत mark;
}

ulong bootstage_error_name(गणना bootstage_id id, स्थिरांक अक्षर *name)
{
	परत bootstage_add_record(id, name, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(गणना bootstage_id id, स्थिरांक अक्षर *name)
{
	पूर्णांक flags = 0;

	जर (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;

	परत bootstage_add_record(id, name, flags, timer_get_boot_us());
}

ulong bootstage_mark_code(स्थिरांक अक्षर *file, स्थिरांक अक्षर *func, पूर्णांक linenum)
{
	अक्षर *str, *p;
	__maybe_unused अक्षर *end;
	पूर्णांक len = 0;

	/* First work out the length we need to allocate */
	जर (linenum != -1)
		len = 11;
	जर (func)
		len += म_माप(func);
	जर (file)
		len += म_माप(file);

	str = दो_स्मृति(len + 1);
	p = str;
	end = p + len;
	जर (file)
		p += snprintf(p, end - p, "%s,", file);
	जर (linenum != -1)
		p += snprintf(p, end - p, "%d", linenum);
	जर (func)
		p += snprintf(p, end - p, ": %s", func);

	परत bootstage_mark_name(BOOTSTAGE_ID_ALLOC, str);
}

uint32_t bootstage_start(गणना bootstage_id id, स्थिरांक अक्षर *name)
{
	संरचना bootstage_data *data = gd->bootstage;
	संरचना bootstage_record *rec = ensure_id(data, id);
	ulong start_us = timer_get_boot_us();

	जर (rec) {
		rec->start_us = start_us;
		rec->name = name;
	}

	परत start_us;
}

uint32_t bootstage_accum(गणना bootstage_id id)
{
	संरचना bootstage_data *data = gd->bootstage;
	संरचना bootstage_record *rec = ensure_id(data, id);
	uint32_t duration;

	जर (!rec)
		परत 0;
	duration = (uint32_t)timer_get_boot_us() - rec->start_us;
	rec->time_us += duration;

	परत duration;
}

/**
 * Get a record name as a printable string
 *
 * @param buf	Buffer to put name if needed
 * @param len	Length of buffer
 * @param rec	Boot stage record to get the name from
 * Return: pointer to name, either from the record or pointing to buf.
 */
स्थिर स्थिरांक अक्षर *get_record_name(अक्षर *buf, पूर्णांक len,
				   स्थिरांक संरचना bootstage_record *rec)
{
	जर (rec->name)
		परत rec->name;
	अन्यथा जर (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	अन्यथा
		snprintf(buf, len, "id=%d", rec->id);

	परत buf;
}

स्थिर uint32_t print_time_record(संरचना bootstage_record *rec, uint32_t prev)
{
	अक्षर buf[20];

	जर (prev == -1U) {
		म_लिखो("%11s", "");
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
	} अन्यथा {
		जर (prev > rec->time_us)
			prev = 0;
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
		print_grouped_ull(rec->time_us - prev, BOOTSTAGE_DIGITS);
	}
	म_लिखो("  %s\n", get_record_name(buf, आकार(buf), rec));

	परत rec->time_us;
}

#ifdef CONFIG_OF_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * Return: 0 on success, != 0 on failure.
 */
स्थिर पूर्णांक add_bootstages_devicetree(संरचना fdt_header *blob)
{
	संरचना bootstage_data *data = gd->bootstage;
	पूर्णांक bootstage;
	अक्षर buf[20];
	पूर्णांक recnum;
	पूर्णांक i;

	जर (!blob)
		परत 0;

	/*
	 * Create the node for bootstage.
	 * The address of flat device tree is set up by the command bootm.
	 */
	bootstage = fdt_add_subnode(blob, 0, "bootstage");
	जर (bootstage < 0)
		परत -EINVAL;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	साठी (recnum = data->rec_मोजणी - 1, i = 0; recnum >= 0; recnum--, i++) {
		संरचना bootstage_record *rec = &data->record[recnum];
		पूर्णांक गाठ;

		जर (rec->id != BOOTSTAGE_ID_AWAKE && rec->time_us == 0)
			सुरूठेवा;

		गाठ = fdt_add_subnode(blob, bootstage, simple_itoa(i));
		जर (गाठ < 0)
			थांबा;

		/* add properties to the node. */
		जर (fdt_setprop_string(blob, गाठ, "name",
				       get_record_name(buf, आकार(buf), rec)))
			परत -EINVAL;

		/* Check if this is a 'mark' or 'accum' record */
		जर (fdt_setprop_cell(blob, गाठ,
				rec->start_us ? "accum" : "mark",
				rec->time_us))
			परत -EINVAL;
	}

	परत 0;
}

पूर्णांक bootstage_fdt_add_report(शून्य)
{
	जर (add_bootstages_devicetree(working_fdt))
		माला_दो("bootstage: Failed to add to device tree\n");

	परत 0;
}
#endif

शून्य bootstage_report(शून्य)
{
	संरचना bootstage_data *data = gd->bootstage;
	संरचना bootstage_record *rec = data->record;
	uint32_t prev;
	पूर्णांक i;

	म_लिखो("Timer summary in microseconds (%d records):\n",
	       data->rec_मोजणी);
	म_लिखो("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	prev = print_time_record(rec, 0);

	साठी (i = 1, rec++; i < data->rec_मोजणी; i++, rec++) {
		जर (rec->id && !rec->start_us)
			prev = print_time_record(rec, prev);
	}
	जर (data->rec_मोजणी > RECORD_मोजणी)
		म_लिखो("Overflowed internal boot id table by %d entries\n"
		       "Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		       data->rec_मोजणी - RECORD_मोजणी);

	माला_दो("\nAccumulated time:\n");
	साठी (i = 0, rec = data->record; i < data->rec_मोजणी; i++, rec++) {
		जर (rec->start_us)
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
स्थिर शून्य append_data(अक्षर **ptrp, अक्षर *end, स्थिरांक शून्य *data, पूर्णांक size)
{
	अक्षर *ptr = *ptrp;

	*ptrp += size;
	जर (*ptrp > end)
		परत;

	स_नकल(ptr, data, size);
}

पूर्णांक bootstage_stash(शून्य *base, पूर्णांक size)
{
	स्थिरांक संरचना bootstage_data *data = gd->bootstage;
	संरचना bootstage_hdr *hdr = (संरचना bootstage_hdr *)base;
	स्थिरांक संरचना bootstage_record *rec;
	अक्षर buf[20];
	अक्षर *ptr = base, *end = ptr + size;
	पूर्णांक i;

	जर (hdr + 1 > (संरचना bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		परत -ENOSPC;
	}

	/* Write an arbitrary version number */
	hdr->version = BOOTSTAGE_VERSION;

	hdr->मोजणी = data->rec_मोजणी;
	hdr->size = 0;
	hdr->magic = BOOTSTAGE_MAGIC;
	hdr->next_id = data->next_id;
	ptr += आकार(*hdr);

	/* Write the records, silently stopping when we run out of space */
	साठी (rec = data->record, i = 0; i < data->rec_मोजणी; i++, rec++)
		append_data(&ptr, end, rec, आकार(*rec));

	/* Write the name strings */
	साठी (rec = data->record, i = 0; i < data->rec_मोजणी; i++, rec++) {
		स्थिरांक अक्षर *name;

		name = get_record_name(buf, आकार(buf), rec);
		append_data(&ptr, end, name, म_माप(name) + 1);
	}

	/* Check for buffer overflow */
	जर (ptr > end) {
		debug("%s: Not enough space for bootstage stash\n", __func__);
		परत -ENOSPC;
	}

	/* Update total data size */
	hdr->size = ptr - (अक्षर *)base;
	debug("Stashed %d records\n", hdr->मोजणी);

	परत 0;
}

पूर्णांक bootstage_unstash(स्थिरांक शून्य *base, पूर्णांक size)
{
	स्थिरांक संरचना bootstage_hdr *hdr = (संरचना bootstage_hdr *)base;
	संरचना bootstage_data *data = gd->bootstage;
	स्थिरांक अक्षर *ptr = base, *end = ptr + size;
	संरचना bootstage_record *rec;
	uint rec_size;
	पूर्णांक i;

	जर (size == -1)
		end = (अक्षर *)(~(uintptr_t)0);

	जर (hdr + 1 > (संरचना bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		परत -EPERM;
	}

	जर (hdr->magic != BOOTSTAGE_MAGIC) {
		debug("%s: Invalid bootstage magic\n", __func__);
		परत -ENOENT;
	}

	जर (ptr + hdr->size > end) {
		debug("%s: Bootstage data runs past buffer end\n", __func__);
		परत -ENOSPC;
	}

	जर (hdr->मोजणी * आकार(*rec) > hdr->size) {
		debug("%s: Bootstage has %d records needing %lu bytes, but "
			"only %d bytes is available\n", __func__, hdr->मोजणी,
		      (ulong)hdr->मोजणी * आकार(*rec), hdr->size);
		परत -ENOSPC;
	}

	जर (hdr->version != BOOTSTAGE_VERSION) {
		debug("%s: Bootstage data version %#0x unrecognised\n",
		      __func__, hdr->version);
		परत -EINVAL;
	}

	जर (data->rec_मोजणी + hdr->मोजणी > RECORD_मोजणी) {
		debug("%s: Bootstage has %d records, we have space for %d\n"
			"Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		      __func__, hdr->मोजणी, RECORD_मोजणी - data->rec_मोजणी);
		परत -ENOSPC;
	}

	ptr += आकार(*hdr);

	/* Read the records */
	rec_size = hdr->मोजणी * आकार(*data->record);
	स_नकल(data->record + data->rec_मोजणी, ptr, rec_size);

	/* Read the name strings */
	ptr += rec_size;
	साठी (rec = data->record + data->rec_मोजणी, i = 0; i < hdr->मोजणी;
	     i++, rec++) {
		rec->name = ptr;
		जर (xpl_phase() == PHASE_SPL)
			rec->name = strdup(ptr);

		/* Assume no data corruption here */
		ptr += म_माप(ptr) + 1;
	}

	/* Mark the records as read */
	data->rec_मोजणी += hdr->मोजणी;
	data->next_id = hdr->next_id;
	debug("Unstashed %d records\n", hdr->मोजणी);

	परत 0;
}

#जर IS_ENABLED(CONFIG_BOOTSTAGE_STASH)
पूर्णांक _bootstage_stash_default(शून्य)
{
	परत bootstage_stash(map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR, 0),
			       CONFIG_BOOTSTAGE_STASH_SIZE);
}

पूर्णांक _bootstage_unstash_default(शून्य)
{
	स्थिरांक शून्य *stash = map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR,
				       CONFIG_BOOTSTAGE_STASH_SIZE);

	परत bootstage_unstash(stash, CONFIG_BOOTSTAGE_STASH_SIZE);
}
#endif

पूर्णांक bootstage_get_size(bool add_strings)
{
	पूर्णांक size;

	size = आकार(संरचना bootstage_data);
	जर (add_strings) {
		संरचना bootstage_data *data = gd->bootstage;
		संरचना bootstage_record *rec;
		पूर्णांक i;

		साठी (rec = data->record, i = 0; i < data->rec_मोजणी; i++, rec++)
			जर (rec->name)
				size += म_माप(rec->name) + 1;
	}

	परत size;
}

पूर्णांक bootstage_init(bool first)
{
	संरचना bootstage_data *data;
	पूर्णांक size = आकार(संरचना bootstage_data);

	gd->bootstage = (संरचना bootstage_data *)दो_स्मृति(size);
	जर (!gd->bootstage)
		परत -ENOMEM;
	data = gd->bootstage;
	स_रखो(data, '\0', size);
	जर (first) {
		data->next_id = BOOTSTAGE_ID_USER;
		bootstage_add_record(BOOTSTAGE_ID_AWAKE, "reset", 0, 0);
	}

	परत 0;
}
