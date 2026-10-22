// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 */

/*
 * This module records the progress of boot and arbitrary commands, and
 * permits accurate timestamping of each.
 */

#घोषणा LOG_CATEGORY	LOGC_BOOT

#समावेश <bootstage.h>
#समावेश <hang.h>
#समावेश <log.h>
#समावेश <malloc.h>
#समावेश <sort.h>
#समावेश <spl.h>
#समावेश <asm/global_data.h>
#समावेश <linux/compiler.h>
#समावेश <linux/libfdt.h>

DECLARE_GLOBAL_तथ्य_PTR;

क्रमागत {
	RECORD_गिनती = CONFIG_VAL(BOOTSTAGE_RECORD_गिनती),
};

काष्ठा bootstage_record {
	ulong time_us;
	uint32_t start_us;
	स्थिर अक्षर *name;
	पूर्णांक flags;		/* see enum bootstage_flags */
	क्रमागत bootstage_id id;
};

काष्ठा bootstage_तथ्य {
	uint rec_गिनती;
	uint अग्र_id;
	काष्ठा bootstage_record record[RECORD_गिनती];
};

क्रमागत {
	BOOTSTAGE_VERSION	= 0,
	BOOTSTAGE_MAGIC		= 0xb00757a3,
	BOOTSTAGE_DIGITS	= 9,
};

काष्ठा bootstage_hdr {
	u32 version;		/* BOOTSTAGE_VERSION */
	u32 गिनती;		/* Number of records */
	u32 आकार;		/* Total data size (non-zero if valid) */
	u32 magic;		/* Magic number */
	u32 अग्र_id;		/* Next ID to use for bootstage */
};

पूर्णांक bootstage_relocate(व्योम *to)
{
	काष्ठा bootstage_तथ्य *तथ्य;
	पूर्णांक i;
	अक्षर *ptr;

	debug("Copying bootstage from %p to %p\n", gd->bootstage, to);
	स_नकल(to, gd->bootstage, माप(काष्ठा bootstage_तथ्य));
	तथ्य = gd->bootstage = to;

	/* Figure out where to relocate the strings to */
	ptr = (अक्षर *)(तथ्य + 1);

	/*
	 * Duplicate all strings.  They may point to an old location in the
	 * program .text section that can eventually get trashed.
	 */
	debug("Relocating %d records\n", तथ्य->rec_गिनती);
	क्रम (i = 0; i < तथ्य->rec_गिनती; i++) {
		स्थिर अक्षर *from = तथ्य->record[i].name;

		अगर (!from)
			जारी;
		म_नकल(ptr, from);
		तथ्य->record[i].name = ptr;
		ptr += म_माप(ptr) + 1;
	}

	वापस 0;
}

काष्ठा bootstage_record *find_id(काष्ठा bootstage_तथ्य *तथ्य,
				 क्रमागत bootstage_id id)
{
	काष्ठा bootstage_record *rec;
	काष्ठा bootstage_record *end;

	क्रम (rec = तथ्य->record, end = rec + तथ्य->rec_गिनती; rec < end;
	     rec++) {
		अगर (rec->id == id)
			वापस rec;
	}

	वापस शून्य;
}

काष्ठा bootstage_record *ensure_id(काष्ठा bootstage_तथ्य *तथ्य,
				   क्रमागत bootstage_id id)
{
	काष्ठा bootstage_record *rec;

	rec = find_id(तथ्य, id);
	अगर (!rec && तथ्य->rec_गिनती < RECORD_गिनती) {
		rec = &तथ्य->record[तथ्य->rec_गिनती++];
		rec->id = id;
		वापस rec;
	}

	वापस rec;
}

ulong bootstage_add_record(क्रमागत bootstage_id id, स्थिर अक्षर *name,
			   पूर्णांक flags, ulong mark)
{
	काष्ठा bootstage_तथ्य *तथ्य = gd->bootstage;
	काष्ठा bootstage_record *rec;

	/*
	 * initf_bootstage() is called very early during boot but since hang()
	 * calls bootstage_error() we can be called before bootstage is set up.
	 * Add a check to avoid this.
	 */
	अगर (!तथ्य)
		वापस mark;
	अगर (flags & BOOTSTAGEF_ALLOC)
		id = तथ्य->अग्र_id++;

	/* Only record the first event for each */
	rec = find_id(तथ्य, id);
	अगर (!rec) {
		अगर (तथ्य->rec_गिनती < RECORD_गिनती) {
			rec = &तथ्य->record[तथ्य->rec_गिनती++];
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

	वापस mark;
}

ulong bootstage_error_name(क्रमागत bootstage_id id, स्थिर अक्षर *name)
{
	वापस bootstage_add_record(id, name, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(क्रमागत bootstage_id id, स्थिर अक्षर *name)
{
	पूर्णांक flags = 0;

	अगर (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;

	वापस bootstage_add_record(id, name, flags, timer_get_boot_us());
}

ulong bootstage_mark_code(स्थिर अक्षर *file, स्थिर अक्षर *func, पूर्णांक linenum)
{
	अक्षर *str, *p;
	__maybe_unused अक्षर *end;
	पूर्णांक len = 0;

	/* First work out the length we need to allocate */
	अगर (linenum != -1)
		len = 11;
	अगर (func)
		len += म_माप(func);
	अगर (file)
		len += म_माप(file);

	str = दो_स्मृति(len + 1);
	p = str;
	end = p + len;
	अगर (file)
		p += snprintf(p, end - p, "%s,", file);
	अगर (linenum != -1)
		p += snprintf(p, end - p, "%d", linenum);
	अगर (func)
		p += snprintf(p, end - p, ": %s", func);

	वापस bootstage_mark_name(BOOTSTAGE_ID_ALLOC, str);
}

uint32_t bootstage_start(क्रमागत bootstage_id id, स्थिर अक्षर *name)
{
	काष्ठा bootstage_तथ्य *तथ्य = gd->bootstage;
	काष्ठा bootstage_record *rec = ensure_id(तथ्य, id);
	ulong start_us = timer_get_boot_us();

	अगर (rec) {
		rec->start_us = start_us;
		rec->name = name;
	}

	वापस start_us;
}

uint32_t bootstage_accum(क्रमागत bootstage_id id)
{
	काष्ठा bootstage_तथ्य *तथ्य = gd->bootstage;
	काष्ठा bootstage_record *rec = ensure_id(तथ्य, id);
	uint32_t duration;

	अगर (!rec)
		वापस 0;
	duration = (uint32_t)timer_get_boot_us() - rec->start_us;
	rec->time_us += duration;

	वापस duration;
}

/**
 * Get a record name as a printable string
 *
 * @param buf	Buffer to put name if needed
 * @param len	Length of buffer
 * @param rec	Boot stage record to get the name from
 * Return: pointer to name, either from the record or pointing to buf.
 */
अटल स्थिर अक्षर *get_record_name(अक्षर *buf, पूर्णांक len,
				   स्थिर काष्ठा bootstage_record *rec)
{
	अगर (rec->name)
		वापस rec->name;
	अन्यथा अगर (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	अन्यथा
		snprintf(buf, len, "id=%d", rec->id);

	वापस buf;
}

अटल uint32_t print_time_record(काष्ठा bootstage_record *rec, uint32_t पूर्व)
{
	अक्षर buf[20];

	अगर (पूर्व == -1U) {
		म_लिखो("%11s", "");
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
	} अन्यथा {
		अगर (पूर्व > rec->time_us)
			पूर्व = 0;
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
		print_grouped_ull(rec->time_us - पूर्व, BOOTSTAGE_DIGITS);
	}
	म_लिखो("  %s\n", get_record_name(buf, माप(buf), rec));

	वापस rec->time_us;
}

#अगर_घोषित CONFIG_OF_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * Return: 0 on success, != 0 on failure.
 */
अटल पूर्णांक add_bootstages_devicetree(काष्ठा fdt_header *blob)
{
	काष्ठा bootstage_तथ्य *तथ्य = gd->bootstage;
	पूर्णांक bootstage;
	अक्षर buf[20];
	पूर्णांक recnum;
	पूर्णांक i;

	अगर (!blob)
		वापस 0;

	/*
	 * Create the node for bootstage.
	 * The address of flat device tree is set up by the command bootm.
	 */
	bootstage = fdt_add_subnode(blob, 0, "bootstage");
	अगर (bootstage < 0)
		वापस -EINVAL;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	क्रम (recnum = तथ्य->rec_गिनती - 1, i = 0; recnum >= 0; recnum--, i++) {
		काष्ठा bootstage_record *rec = &तथ्य->record[recnum];
		पूर्णांक गाँठ;

		अगर (rec->id != BOOTSTAGE_ID_AWAKE && rec->time_us == 0)
			जारी;

		गाँठ = fdt_add_subnode(blob, bootstage, simple_itoa(i));
		अगर (गाँठ < 0)
			अवरोध;

		/* add properties to the node. */
		अगर (fdt_setprop_string(blob, गाँठ, "name",
				       get_record_name(buf, माप(buf), rec)))
			वापस -EINVAL;

		/* Check if this is a 'mark' or 'accum' record */
		अगर (fdt_setprop_cell(blob, गाँठ,
				rec->start_us ? "accum" : "mark",
				rec->time_us))
			वापस -EINVAL;
	}

	वापस 0;
}

पूर्णांक bootstage_fdt_add_report(व्योम)
{
	अगर (add_bootstages_devicetree(working_fdt))
		माला_दो("bootstage: Failed to add to device tree\n");

	वापस 0;
}
#पूर्ण_अगर

व्योम bootstage_report(व्योम)
{
	काष्ठा bootstage_तथ्य *तथ्य = gd->bootstage;
	काष्ठा bootstage_record *rec = तथ्य->record;
	uint32_t पूर्व;
	पूर्णांक i;

	म_लिखो("Timer summary in microseconds (%d records):\n",
	       तथ्य->rec_गिनती);
	म_लिखो("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	पूर्व = print_time_record(rec, 0);

	क्रम (i = 1, rec++; i < तथ्य->rec_गिनती; i++, rec++) {
		अगर (rec->id && !rec->start_us)
			पूर्व = print_time_record(rec, पूर्व);
	}
	अगर (तथ्य->rec_गिनती > RECORD_गिनती)
		म_लिखो("Overflowed internal boot id table by %d entries\n"
		       "Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		       तथ्य->rec_गिनती - RECORD_गिनती);

	माला_दो("\nAccumulated time:\n");
	क्रम (i = 0, rec = तथ्य->record; i < तथ्य->rec_गिनती; i++, rec++) {
		अगर (rec->start_us)
			पूर्व = print_time_record(rec, -1);
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
अटल व्योम append_तथ्य(अक्षर **ptrp, अक्षर *end, स्थिर व्योम *तथ्य, पूर्णांक आकार)
{
	अक्षर *ptr = *ptrp;

	*ptrp += आकार;
	अगर (*ptrp > end)
		वापस;

	स_नकल(ptr, तथ्य, आकार);
}

पूर्णांक bootstage_stash(व्योम *base, पूर्णांक आकार)
{
	स्थिर काष्ठा bootstage_तथ्य *तथ्य = gd->bootstage;
	काष्ठा bootstage_hdr *hdr = (काष्ठा bootstage_hdr *)base;
	स्थिर काष्ठा bootstage_record *rec;
	अक्षर buf[20];
	अक्षर *ptr = base, *end = ptr + आकार;
	पूर्णांक i;

	अगर (hdr + 1 > (काष्ठा bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		वापस -ENOSPC;
	}

	/* Write an arbitrary version number */
	hdr->version = BOOTSTAGE_VERSION;

	hdr->गिनती = तथ्य->rec_गिनती;
	hdr->आकार = 0;
	hdr->magic = BOOTSTAGE_MAGIC;
	hdr->अग्र_id = तथ्य->अग्र_id;
	ptr += माप(*hdr);

	/* Write the records, silently stopping when we run out of space */
	क्रम (rec = तथ्य->record, i = 0; i < तथ्य->rec_गिनती; i++, rec++)
		append_तथ्य(&ptr, end, rec, माप(*rec));

	/* Write the name strings */
	क्रम (rec = तथ्य->record, i = 0; i < तथ्य->rec_गिनती; i++, rec++) {
		स्थिर अक्षर *name;

		name = get_record_name(buf, माप(buf), rec);
		append_तथ्य(&ptr, end, name, म_माप(name) + 1);
	}

	/* Check for buffer overflow */
	अगर (ptr > end) {
		debug("%s: Not enough space for bootstage stash\n", __func__);
		वापस -ENOSPC;
	}

	/* Update total data size */
	hdr->आकार = ptr - (अक्षर *)base;
	debug("Stashed %d records\n", hdr->गिनती);

	वापस 0;
}

पूर्णांक bootstage_unstash(स्थिर व्योम *base, पूर्णांक आकार)
{
	स्थिर काष्ठा bootstage_hdr *hdr = (काष्ठा bootstage_hdr *)base;
	काष्ठा bootstage_तथ्य *तथ्य = gd->bootstage;
	स्थिर अक्षर *ptr = base, *end = ptr + आकार;
	काष्ठा bootstage_record *rec;
	uint rec_आकार;
	पूर्णांक i;

	अगर (आकार == -1)
		end = (अक्षर *)(~(uintptr_t)0);

	अगर (hdr + 1 > (काष्ठा bootstage_hdr *)end) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		वापस -EPERM;
	}

	अगर (hdr->magic != BOOTSTAGE_MAGIC) {
		debug("%s: Invalid bootstage magic\n", __func__);
		वापस -ENOENT;
	}

	अगर (ptr + hdr->आकार > end) {
		debug("%s: Bootstage data runs past buffer end\n", __func__);
		वापस -ENOSPC;
	}

	अगर (hdr->गिनती * माप(*rec) > hdr->आकार) {
		debug("%s: Bootstage has %d records needing %lu bytes, but "
			"only %d bytes is available\n", __func__, hdr->गिनती,
		      (ulong)hdr->गिनती * माप(*rec), hdr->आकार);
		वापस -ENOSPC;
	}

	अगर (hdr->version != BOOTSTAGE_VERSION) {
		debug("%s: Bootstage data version %#0x unrecognised\n",
		      __func__, hdr->version);
		वापस -EINVAL;
	}

	अगर (तथ्य->rec_गिनती + hdr->गिनती > RECORD_गिनती) {
		debug("%s: Bootstage has %d records, we have space for %d\n"
			"Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		      __func__, hdr->गिनती, RECORD_गिनती - तथ्य->rec_गिनती);
		वापस -ENOSPC;
	}

	ptr += माप(*hdr);

	/* Read the records */
	rec_आकार = hdr->गिनती * माप(*तथ्य->record);
	स_नकल(तथ्य->record + तथ्य->rec_गिनती, ptr, rec_आकार);

	/* Read the name strings */
	ptr += rec_आकार;
	क्रम (rec = तथ्य->record + तथ्य->rec_गिनती, i = 0; i < hdr->गिनती;
	     i++, rec++) {
		rec->name = ptr;
		अगर (xpl_phase() == PHASE_SPL)
			rec->name = strdup(ptr);

		/* Assume no data corruption here */
		ptr += म_माप(ptr) + 1;
	}

	/* Mark the records as read */
	तथ्य->rec_गिनती += hdr->गिनती;
	तथ्य->अग्र_id = hdr->अग्र_id;
	debug("Unstashed %d records\n", hdr->गिनती);

	वापस 0;
}

#अगर IS_ENABLED(CONFIG_BOOTSTAGE_STASH)
पूर्णांक _bootstage_stash_default(व्योम)
{
	वापस bootstage_stash(map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR, 0),
			       CONFIG_BOOTSTAGE_STASH_आकार);
}

पूर्णांक _bootstage_unstash_default(व्योम)
{
	स्थिर व्योम *stash = map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR,
				       CONFIG_BOOTSTAGE_STASH_आकार);

	वापस bootstage_unstash(stash, CONFIG_BOOTSTAGE_STASH_आकार);
}
#पूर्ण_अगर

पूर्णांक bootstage_get_आकार(bool add_strings)
{
	पूर्णांक आकार;

	आकार = माप(काष्ठा bootstage_तथ्य);
	अगर (add_strings) {
		काष्ठा bootstage_तथ्य *तथ्य = gd->bootstage;
		काष्ठा bootstage_record *rec;
		पूर्णांक i;

		क्रम (rec = तथ्य->record, i = 0; i < तथ्य->rec_गिनती; i++, rec++)
			अगर (rec->name)
				आकार += म_माप(rec->name) + 1;
	}

	वापस आकार;
}

पूर्णांक bootstage_init(bool first)
{
	काष्ठा bootstage_तथ्य *तथ्य;
	पूर्णांक आकार = माप(काष्ठा bootstage_तथ्य);

	gd->bootstage = (काष्ठा bootstage_तथ्य *)दो_स्मृति(आकार);
	अगर (!gd->bootstage)
		वापस -ENOMEM;
	तथ्य = gd->bootstage;
	स_रखो(तथ्य, '\0', आकार);
	अगर (first) {
		तथ्य->अग्र_id = BOOTSTAGE_ID_USER;
		bootstage_add_record(BOOTSTAGE_ID_AWAKE, "reset", 0, 0);
	}

	वापस 0;
}
