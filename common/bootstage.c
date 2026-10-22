// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 */

/*
 * This module records the progress of boot and arbitrary commands, and
 * permits accurate timestamping of each.
 */

#परिभाषय LOG_CATEGORY	LOGC_BOOT

#अन्तर्भाव <bootstage.h>
#अन्तर्भाव <hang.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <sort.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <linux/compiler.h>
#अन्तर्भाव <linux/libfdt.h>

DECLARE_GLOBAL_दत्त_PTR;

गणना {
	लेखः_गणनम् = CONFIG_VAL(BOOTSTAGE_लेखः_गणनम्),
};

संरचना bootstage_लेखः {
	ulong time_us;
	uint32_t start_us;
	ध्रुव वर्ण *name;
	पूर्णाङ्क flags;		/* see enum bootstage_flags */
	गणना bootstage_id id;
};

संरचना bootstage_दत्त {
	uint rec_गणनम्;
	uint अग्र_id;
	संरचना bootstage_लेखः लेखः[लेखः_गणनम्];
};

गणना {
	BOOTSTAGE_संस्करणम्	= 0,
	BOOTSTAGE_MAGIC		= 0xb00757a3,
	BOOTSTAGE_DIGITS	= 9,
};

संरचना bootstage_hdr {
	u32 version;		/* BOOTSTAGE_VERSION */
	u32 गणनम्;		/* Number of records */
	u32 आकार;		/* Total data size (non-zero if valid) */
	u32 magic;		/* Magic number */
	u32 अग्र_id;		/* Next ID to use for bootstage */
};

पूर्णाङ्क bootstage_relocate(शून्य *गन्तव्यम्)
{
	संरचना bootstage_दत्त *दत्त;
	पूर्णाङ्क i;
	वर्ण *ptr;

	debug("Copying bootstage from %p to %p\n", gd->bootstage, गन्तव्यम्);
	स्मृतिं_प्रतिलिख(गन्तव्यम्, gd->bootstage, परिमाण(संरचना bootstage_दत्त));
	दत्त = gd->bootstage = गन्तव्यम्;

	/* Figure out where to relocate the strings to */
	ptr = (वर्ण *)(दत्त + 1);

	/*
	 * Duplicate all strings.  They may point to an old location in the
	 * program .text section that can eventually get trashed.
	 */
	debug("Relocating %d records\n", दत्त->rec_गणनम्);
	प्रति (i = 0; i < दत्त->rec_गणनम्; i++) {
		ध्रुव वर्ण *from = दत्त->लेखः[i].name;

		यदि (!from)
			अनुवर्त;
		वाक्यं_प्रतिलिख(ptr, from);
		दत्त->लेखः[i].name = ptr;
		ptr += वाक्यदैर्घ्यम्(ptr) + 1;
	}

	प्रत्यागम 0;
}

संरचना bootstage_लेखः *find_id(संरचना bootstage_दत्त *दत्त,
				 गणना bootstage_id id)
{
	संरचना bootstage_लेखः *rec;
	संरचना bootstage_लेखः *अन्तः_2;

	प्रति (rec = दत्त->लेखः, अन्तः_2 = rec + दत्त->rec_गणनम्; rec < अन्तः_2;
	     rec++) {
		यदि (rec->id == id)
			प्रत्यागम rec;
	}

	प्रत्यागम रिक्त;
}

संरचना bootstage_लेखः *ensure_id(संरचना bootstage_दत्त *दत्त,
				   गणना bootstage_id id)
{
	संरचना bootstage_लेखः *rec;

	rec = find_id(दत्त, id);
	यदि (!rec && दत्त->rec_गणनम् < लेखः_गणनम्) {
		rec = &दत्त->लेखः[दत्त->rec_गणनम्++];
		rec->id = id;
		प्रत्यागम rec;
	}

	प्रत्यागम rec;
}

ulong bootstage_सङ्कलय_लेखः(गणना bootstage_id id, ध्रुव वर्ण *name,
			   पूर्णाङ्क flags, ulong mark)
{
	संरचना bootstage_दत्त *दत्त = gd->bootstage;
	संरचना bootstage_लेखः *rec;

	/*
	 * initf_bootstage() is called very early during boot but since hang()
	 * calls bootstage_error() we can be called before bootstage is set up.
	 * Add a check to avoid this.
	 */
	यदि (!दत्त)
		प्रत्यागम mark;
	यदि (flags & BOOTSTAGEF_ALLOC)
		id = दत्त->अग्र_id++;

	/* Only record the first event for each */
	rec = find_id(दत्त, id);
	यदि (!rec) {
		यदि (दत्त->rec_गणनम् < लेखः_गणनम्) {
			rec = &दत्त->लेखः[दत्त->rec_गणनम्++];
			rec->time_us = mark;
			rec->name = name;
			rec->flags = flags;
			rec->id = id;
		} अन्यथा {
			log_warning("Bootstage space exhausted\n");
		}
	}

	/* Tell the board about this progress */
	show_boot_प्रगतिः(flags & BOOTSTAGEF_ERROR ? -id : id);

	प्रत्यागम mark;
}

ulong bootstage_error_name(गणना bootstage_id id, ध्रुव वर्ण *name)
{
	प्रत्यागम bootstage_सङ्कलय_लेखः(id, name, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(गणना bootstage_id id, ध्रुव वर्ण *name)
{
	पूर्णाङ्क flags = 0;

	यदि (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;

	प्रत्यागम bootstage_सङ्कलय_लेखः(id, name, flags, timer_get_boot_us());
}

ulong bootstage_mark_संहिता(ध्रुव वर्ण *file, ध्रुव वर्ण *func, पूर्णाङ्क linenum)
{
	वर्ण *str, *p;
	__maybe_unused वर्ण *अन्तः_2;
	पूर्णाङ्क len = 0;

	/* First work out the length we need to allocate */
	यदि (linenum != -1)
		len = 11;
	यदि (func)
		len += वाक्यदैर्घ्यम्(func);
	यदि (file)
		len += वाक्यदैर्घ्यम्(file);

	str = स्मृतिं_देहि(len + 1);
	p = str;
	अन्तः_2 = p + len;
	यदि (file)
		p += snprintf(p, अन्तः_2 - p, "%s,", file);
	यदि (linenum != -1)
		p += snprintf(p, अन्तः_2 - p, "%d", linenum);
	यदि (func)
		p += snprintf(p, अन्तः_2 - p, ": %s", func);

	प्रत्यागम bootstage_mark_name(BOOTSTAGE_ID_ALLOC, str);
}

uint32_t bootstage_start(गणना bootstage_id id, ध्रुव वर्ण *name)
{
	संरचना bootstage_दत्त *दत्त = gd->bootstage;
	संरचना bootstage_लेखः *rec = ensure_id(दत्त, id);
	ulong start_us = timer_get_boot_us();

	यदि (rec) {
		rec->start_us = start_us;
		rec->name = name;
	}

	प्रत्यागम start_us;
}

uint32_t bootstage_accum(गणना bootstage_id id)
{
	संरचना bootstage_दत्त *दत्त = gd->bootstage;
	संरचना bootstage_लेखः *rec = ensure_id(दत्त, id);
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
स्थिर ध्रुव वर्ण *get_लेखः_name(वर्ण *buf, पूर्णाङ्क len,
				   ध्रुव संरचना bootstage_लेखः *rec)
{
	यदि (rec->name)
		प्रत्यागम rec->name;
	अन्यथा यदि (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	अन्यथा
		snprintf(buf, len, "id=%d", rec->id);

	प्रत्यागम buf;
}

स्थिर uint32_t print_time_लेखः(संरचना bootstage_लेखः *rec, uint32_t पूर्व)
{
	वर्ण buf[20];

	यदि (पूर्व == -1U) {
		प्रारूप_लिख("%11s", "");
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
	} अन्यथा {
		यदि (पूर्व > rec->time_us)
			पूर्व = 0;
		print_grouped_ull(rec->time_us, BOOTSTAGE_DIGITS);
		print_grouped_ull(rec->time_us - पूर्व, BOOTSTAGE_DIGITS);
	}
	प्रारूप_लिख("  %s\n", get_लेखः_name(buf, परिमाण(buf), rec));

	प्रत्यागम rec->time_us;
}

#यदि_परिभाषितम् CONFIG_सम्बन्धि_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * Return: 0 on success, != 0 on failure.
 */
स्थिर पूर्णाङ्क सङ्कलय_bootstages_devicetree(संरचना fdt_शीर्षकम् *blob)
{
	संरचना bootstage_दत्त *दत्त = gd->bootstage;
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
	bootstage = fdt_सङ्कलय_subnode(blob, 0, "bootstage");
	यदि (bootstage < 0)
		प्रत्यागम -EINVAL;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	प्रति (recnum = दत्त->rec_गणनम् - 1, i = 0; recnum >= 0; recnum--, i++) {
		संरचना bootstage_लेखः *rec = &दत्त->लेखः[recnum];
		पूर्णाङ्क ग्रन्थिः;

		यदि (rec->id != BOOTSTAGE_ID_AWAKE && rec->time_us == 0)
			अनुवर्त;

		ग्रन्थिः = fdt_सङ्कलय_subnode(blob, bootstage, simple_itoa(i));
		यदि (ग्रन्थिः < 0)
			भङ्ग;

		/* add properties to the node. */
		यदि (fdt_setprop_string(blob, ग्रन्थिः, "name",
				       get_लेखः_name(buf, परिमाण(buf), rec)))
			प्रत्यागम -EINVAL;

		/* Check if this is a 'mark' or 'accum' record */
		यदि (fdt_setprop_cell(blob, ग्रन्थिः,
				rec->start_us ? "accum" : "mark",
				rec->time_us))
			प्रत्यागम -EINVAL;
	}

	प्रत्यागम 0;
}

पूर्णाङ्क bootstage_fdt_सङ्कलय_report(शून्य)
{
	यदि (सङ्कलय_bootstages_devicetree(working_fdt))
		पङ्क्तिं_लिख("bootstage: Failed to add to device tree\n");

	प्रत्यागम 0;
}
#यदि_अन्तः

शून्य bootstage_report(शून्य)
{
	संरचना bootstage_दत्त *दत्त = gd->bootstage;
	संरचना bootstage_लेखः *rec = दत्त->लेखः;
	uint32_t पूर्व;
	पूर्णाङ्क i;

	प्रारूप_लिख("Timer summary in microseconds (%d records):\n",
	       दत्त->rec_गणनम्);
	प्रारूप_लिख("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	पूर्व = print_time_लेखः(rec, 0);

	प्रति (i = 1, rec++; i < दत्त->rec_गणनम्; i++, rec++) {
		यदि (rec->id && !rec->start_us)
			पूर्व = print_time_लेखः(rec, पूर्व);
	}
	यदि (दत्त->rec_गणनम् > लेखः_गणनम्)
		प्रारूप_लिख("Overflowed internal boot id table by %d entries\n"
		       "Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		       दत्त->rec_गणनम् - लेखः_गणनम्);

	पङ्क्तिं_लिख("\nAccumulated time:\n");
	प्रति (i = 0, rec = दत्त->लेखः; i < दत्त->rec_गणनम्; i++, rec++) {
		यदि (rec->start_us)
			पूर्व = print_time_लेखः(rec, -1);
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
स्थिर शून्य append_दत्त(वर्ण **ptrp, वर्ण *अन्तः_2, ध्रुव शून्य *दत्त, पूर्णाङ्क आकार)
{
	वर्ण *ptr = *ptrp;

	*ptrp += आकार;
	यदि (*ptrp > अन्तः_2)
		प्रत्यागम;

	स्मृतिं_प्रतिलिख(ptr, दत्त, आकार);
}

पूर्णाङ्क bootstage_stash(शून्य *आधारः, पूर्णाङ्क आकार)
{
	ध्रुव संरचना bootstage_दत्त *दत्त = gd->bootstage;
	संरचना bootstage_hdr *hdr = (संरचना bootstage_hdr *)आधारः;
	ध्रुव संरचना bootstage_लेखः *rec;
	वर्ण buf[20];
	वर्ण *ptr = आधारः, *अन्तः_2 = ptr + आकार;
	पूर्णाङ्क i;

	यदि (hdr + 1 > (संरचना bootstage_hdr *)अन्तः_2) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		प्रत्यागम -ENOSPC;
	}

	/* Write an arbitrary version number */
	hdr->version = BOOTSTAGE_संस्करणम्;

	hdr->गणनम् = दत्त->rec_गणनम्;
	hdr->आकार = 0;
	hdr->magic = BOOTSTAGE_MAGIC;
	hdr->अग्र_id = दत्त->अग्र_id;
	ptr += परिमाण(*hdr);

	/* Write the records, silently stopping when we run out of space */
	प्रति (rec = दत्त->लेखः, i = 0; i < दत्त->rec_गणनम्; i++, rec++)
		append_दत्त(&ptr, अन्तः_2, rec, परिमाण(*rec));

	/* Write the name strings */
	प्रति (rec = दत्त->लेखः, i = 0; i < दत्त->rec_गणनम्; i++, rec++) {
		ध्रुव वर्ण *name;

		name = get_लेखः_name(buf, परिमाण(buf), rec);
		append_दत्त(&ptr, अन्तः_2, name, वाक्यदैर्घ्यम्(name) + 1);
	}

	/* Check for buffer overflow */
	यदि (ptr > अन्तः_2) {
		debug("%s: Not enough space for bootstage stash\n", __func__);
		प्रत्यागम -ENOSPC;
	}

	/* Update total data size */
	hdr->आकार = ptr - (वर्ण *)आधारः;
	debug("Stashed %d records\n", hdr->गणनम्);

	प्रत्यागम 0;
}

पूर्णाङ्क bootstage_unstash(ध्रुव शून्य *आधारः, पूर्णाङ्क आकार)
{
	ध्रुव संरचना bootstage_hdr *hdr = (संरचना bootstage_hdr *)आधारः;
	संरचना bootstage_दत्त *दत्त = gd->bootstage;
	ध्रुव वर्ण *ptr = आधारः, *अन्तः_2 = ptr + आकार;
	संरचना bootstage_लेखः *rec;
	uint rec_आकार;
	पूर्णाङ्क i;

	यदि (आकार == -1)
		अन्तः_2 = (वर्ण *)(~(uintptr_t)0);

	यदि (hdr + 1 > (संरचना bootstage_hdr *)अन्तः_2) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		प्रत्यागम -EPERM;
	}

	यदि (hdr->magic != BOOTSTAGE_MAGIC) {
		debug("%s: Invalid bootstage magic\n", __func__);
		प्रत्यागम -ENOENT;
	}

	यदि (ptr + hdr->आकार > अन्तः_2) {
		debug("%s: Bootstage data runs past buffer end\n", __func__);
		प्रत्यागम -ENOSPC;
	}

	यदि (hdr->गणनम् * परिमाण(*rec) > hdr->आकार) {
		debug("%s: Bootstage has %d records needing %lu bytes, but "
			"only %d bytes is available\n", __func__, hdr->गणनम्,
		      (ulong)hdr->गणनम् * परिमाण(*rec), hdr->आकार);
		प्रत्यागम -ENOSPC;
	}

	यदि (hdr->version != BOOTSTAGE_संस्करणम्) {
		debug("%s: Bootstage data version %#0x unrecognised\n",
		      __func__, hdr->version);
		प्रत्यागम -EINVAL;
	}

	यदि (दत्त->rec_गणनम् + hdr->गणनम् > लेखः_गणनम्) {
		debug("%s: Bootstage has %d records, we have space for %d\n"
			"Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		      __func__, hdr->गणनम्, लेखः_गणनम् - दत्त->rec_गणनम्);
		प्रत्यागम -ENOSPC;
	}

	ptr += परिमाण(*hdr);

	/* Read the records */
	rec_आकार = hdr->गणनम् * परिमाण(*दत्त->लेखः);
	स्मृतिं_प्रतिलिख(दत्त->लेखः + दत्त->rec_गणनम्, ptr, rec_आकार);

	/* Read the name strings */
	ptr += rec_आकार;
	प्रति (rec = दत्त->लेखः + दत्त->rec_गणनम्, i = 0; i < hdr->गणनम्;
	     i++, rec++) {
		rec->name = ptr;
		यदि (xpl_phase() == PHASE_SPL)
			rec->name = strdup(ptr);

		/* Assume no data corruption here */
		ptr += वाक्यदैर्घ्यम्(ptr) + 1;
	}

	/* Mark the records as read */
	दत्त->rec_गणनम् += hdr->गणनम्;
	दत्त->अग्र_id = hdr->अग्र_id;
	debug("Unstashed %d records\n", hdr->गणनम्);

	प्रत्यागम 0;
}

#यदि IS_ENABLED(CONFIG_BOOTSTAGE_STASH)
पूर्णाङ्क _bootstage_stash_default(शून्य)
{
	प्रत्यागम bootstage_stash(map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR, 0),
			       CONFIG_BOOTSTAGE_STASH_आकार);
}

पूर्णाङ्क _bootstage_unstash_default(शून्य)
{
	ध्रुव शून्य *stash = map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR,
				       CONFIG_BOOTSTAGE_STASH_आकार);

	प्रत्यागम bootstage_unstash(stash, CONFIG_BOOTSTAGE_STASH_आकार);
}
#यदि_अन्तः

पूर्णाङ्क bootstage_get_आकार(bool सङ्कलय_वाक्यानि)
{
	पूर्णाङ्क आकार;

	आकार = परिमाण(संरचना bootstage_दत्त);
	यदि (सङ्कलय_वाक्यानि) {
		संरचना bootstage_दत्त *दत्त = gd->bootstage;
		संरचना bootstage_लेखः *rec;
		पूर्णाङ्क i;

		प्रति (rec = दत्त->लेखः, i = 0; i < दत्त->rec_गणनम्; i++, rec++)
			यदि (rec->name)
				आकार += वाक्यदैर्घ्यम्(rec->name) + 1;
	}

	प्रत्यागम आकार;
}

पूर्णाङ्क bootstage_init(bool first)
{
	संरचना bootstage_दत्त *दत्त;
	पूर्णाङ्क आकार = परिमाण(संरचना bootstage_दत्त);

	gd->bootstage = (संरचना bootstage_दत्त *)स्मृतिं_देहि(आकार);
	यदि (!gd->bootstage)
		प्रत्यागम -ENOMEM;
	दत्त = gd->bootstage;
	स्मृतिं_पूरय(दत्त, '\0', आकार);
	यदि (first) {
		दत्त->अग्र_id = BOOTSTAGE_ID_USER;
		bootstage_सङ्कलय_लेखः(BOOTSTAGE_ID_AWAKE, "reset", 0, 0);
	}

	प्रत्यागम 0;
}
