// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 */

/*
 * This module records the progress of boot and arbitrary commands, and
 * permits accurate timestamping of each.
 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LOG_CATEGORY	LOGC_BOOT

#ਅਨ੍ਤਰ੍ਭਾਵ <bootstage.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <hang.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sort.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/compiler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/libfdt.h>

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

ਗਿਣਤੀ {
	ਰਿਕਾਰਡ_ਗਿਣਤੀ = CONFIG_VAL(BOOTSTAGE_ਰਿਕਾਰਡ_ਗਿਣਤੀ),
};

ਬਣਤਰ bootstage_ਰਿਕਾਰਡ {
	ulong ਸਮਾਂ_us;
	uint32_t ਸ਼ੁਰੂ_us;
	ਅਚਲ ਅੱਖਰ *name;
	ਪੂਰਨ flags;		/* see enum bootstage_flags */
	ਗਿਣਤੀ bootstage_id id;
};

ਬਣਤਰ bootstage_ਅੰਕੜੇ {
	uint rec_ਗਿਣਤੀ;
	uint ਅਗਲਾ_id;
	ਬਣਤਰ bootstage_ਰਿਕਾਰਡ ਰਿਕਾਰਡ[ਰਿਕਾਰਡ_ਗਿਣਤੀ];
};

ਗਿਣਤੀ {
	BOOTSTAGE_ਸੰਸਕਰਣ	= 0,
	BOOTSTAGE_MAGIC		= 0xb00757a3,
	BOOTSTAGE_DIGITS	= 9,
};

ਬਣਤਰ bootstage_hdr {
	u32 version;		/* BOOTSTAGE_VERSION */
	u32 ਗਿਣਤੀ_2;		/* Number of records */
	u32 ਮਾਪ;		/* Total data size (non-zero if valid) */
	u32 magic;		/* Magic number */
	u32 ਅਗਲਾ_id;		/* Next ID to use for bootstage */
};

ਪੂਰਨ bootstage_relocate(ਖਾਲੀ *ਮੰਜ਼ਿਲ)
{
	ਬਣਤਰ bootstage_ਅੰਕੜੇ *ਅੰਕੜੇ;
	ਪੂਰਨ i;
	ਅੱਖਰ *ptr;

	debug("Copying bootstage from %p to %p\n", gd->bootstage, ਮੰਜ਼ਿਲ);
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਮੰਜ਼ਿਲ, gd->bootstage, ਆਕਾਰ(ਬਣਤਰ bootstage_ਅੰਕੜੇ));
	ਅੰਕੜੇ = gd->bootstage = ਮੰਜ਼ਿਲ;

	/* Figure out where to relocate the strings to */
	ptr = (ਅੱਖਰ *)(ਅੰਕੜੇ + 1);

	/*
	 * Duplicate all strings.  They may point to an old location in the
	 * program .text section that can eventually get trashed.
	 */
	debug("Relocating %d records\n", ਅੰਕੜੇ->rec_ਗਿਣਤੀ);
	ਲਈ (i = 0; i < ਅੰਕੜੇ->rec_ਗਿਣਤੀ; i++) {
		ਅਚਲ ਅੱਖਰ *from = ਅੰਕੜੇ->ਰਿਕਾਰਡ[i].name;

		ਜੇ (!from)
			ਜਾਰੀ;
		strcpy(ptr, from);
		ਅੰਕੜੇ->ਰਿਕਾਰਡ[i].name = ptr;
		ptr += ਲੜੀ_ਲੰਬਾਈ(ptr) + 1;
	}

	ਵਾਪਸ 0;
}

ਬਣਤਰ bootstage_ਰਿਕਾਰਡ *ਲੱਭੋ_id(ਬਣਤਰ bootstage_ਅੰਕੜੇ *ਅੰਕੜੇ,
				 ਗਿਣਤੀ bootstage_id id)
{
	ਬਣਤਰ bootstage_ਰਿਕਾਰਡ *rec;
	ਬਣਤਰ bootstage_ਰਿਕਾਰਡ *ਸਿਰਾ_2;

	ਲਈ (rec = ਅੰਕੜੇ->ਰਿਕਾਰਡ, ਸਿਰਾ_2 = rec + ਅੰਕੜੇ->rec_ਗਿਣਤੀ; rec < ਸਿਰਾ_2;
	     rec++) {
		ਜੇ (rec->id == id)
			ਵਾਪਸ rec;
	}

	ਵਾਪਸ ਨਿਰਾ;
}

ਬਣਤਰ bootstage_ਰਿਕਾਰਡ *ensure_id(ਬਣਤਰ bootstage_ਅੰਕੜੇ *ਅੰਕੜੇ,
				   ਗਿਣਤੀ bootstage_id id)
{
	ਬਣਤਰ bootstage_ਰਿਕਾਰਡ *rec;

	rec = ਲੱਭੋ_id(ਅੰਕੜੇ, id);
	ਜੇ (!rec && ਅੰਕੜੇ->rec_ਗਿਣਤੀ < ਰਿਕਾਰਡ_ਗਿਣਤੀ) {
		rec = &ਅੰਕੜੇ->ਰਿਕਾਰਡ[ਅੰਕੜੇ->rec_ਗਿਣਤੀ++];
		rec->id = id;
		ਵਾਪਸ rec;
	}

	ਵਾਪਸ rec;
}

ulong bootstage_ਜੋੜੋ_ਰਿਕਾਰਡ(ਗਿਣਤੀ bootstage_id id, ਅਚਲ ਅੱਖਰ *name,
			   ਪੂਰਨ flags, ulong mark)
{
	ਬਣਤਰ bootstage_ਅੰਕੜੇ *ਅੰਕੜੇ = gd->bootstage;
	ਬਣਤਰ bootstage_ਰਿਕਾਰਡ *rec;

	/*
	 * initf_bootstage() is called very early during boot but since hang()
	 * calls bootstage_error() we can be called before bootstage is set up.
	 * Add a check to avoid this.
	 */
	ਜੇ (!ਅੰਕੜੇ)
		ਵਾਪਸ mark;
	ਜੇ (flags & BOOTSTAGEF_ALLOC)
		id = ਅੰਕੜੇ->ਅਗਲਾ_id++;

	/* Only record the first event for each */
	rec = ਲੱਭੋ_id(ਅੰਕੜੇ, id);
	ਜੇ (!rec) {
		ਜੇ (ਅੰਕੜੇ->rec_ਗਿਣਤੀ < ਰਿਕਾਰਡ_ਗਿਣਤੀ) {
			rec = &ਅੰਕੜੇ->ਰਿਕਾਰਡ[ਅੰਕੜੇ->rec_ਗਿਣਤੀ++];
			rec->ਸਮਾਂ_us = mark;
			rec->name = name;
			rec->flags = flags;
			rec->id = id;
		} ਨਹੀਂਤਾਂ {
			log_warning("Bootstage space exhausted\n");
		}
	}

	/* Tell the board about this progress */
	show_boot_ਤਰੱਕੀ(flags & BOOTSTAGEF_ERROR ? -id : id);

	ਵਾਪਸ mark;
}

ulong bootstage_error_name(ਗਿਣਤੀ bootstage_id id, ਅਚਲ ਅੱਖਰ *name)
{
	ਵਾਪਸ bootstage_ਜੋੜੋ_ਰਿਕਾਰਡ(id, name, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(ਗਿਣਤੀ bootstage_id id, ਅਚਲ ਅੱਖਰ *name)
{
	ਪੂਰਨ flags = 0;

	ਜੇ (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;

	ਵਾਪਸ bootstage_ਜੋੜੋ_ਰਿਕਾਰਡ(id, name, flags, timer_get_boot_us());
}

ulong bootstage_mark_ਕੋਡ(ਅਚਲ ਅੱਖਰ *file, ਅਚਲ ਅੱਖਰ *func, ਪੂਰਨ linenum)
{
	ਅੱਖਰ *str, *p;
	__maybe_unused ਅੱਖਰ *ਸਿਰਾ_2;
	ਪੂਰਨ len = 0;

	/* First work out the length we need to allocate */
	ਜੇ (linenum != -1)
		len = 11;
	ਜੇ (func)
		len += ਲੜੀ_ਲੰਬਾਈ(func);
	ਜੇ (file)
		len += ਲੜੀ_ਲੰਬਾਈ(file);

	str = ਸਮ੍ਰਿਤੀ_ਦਿਓ(len + 1);
	p = str;
	ਸਿਰਾ_2 = p + len;
	ਜੇ (file)
		p += snprintf(p, ਸਿਰਾ_2 - p, "%s,", file);
	ਜੇ (linenum != -1)
		p += snprintf(p, ਸਿਰਾ_2 - p, "%d", linenum);
	ਜੇ (func)
		p += snprintf(p, ਸਿਰਾ_2 - p, ": %s", func);

	ਵਾਪਸ bootstage_mark_name(BOOTSTAGE_ID_ALLOC, str);
}

uint32_t bootstage_ਸ਼ੁਰੂ(ਗਿਣਤੀ bootstage_id id, ਅਚਲ ਅੱਖਰ *name)
{
	ਬਣਤਰ bootstage_ਅੰਕੜੇ *ਅੰਕੜੇ = gd->bootstage;
	ਬਣਤਰ bootstage_ਰਿਕਾਰਡ *rec = ensure_id(ਅੰਕੜੇ, id);
	ulong ਸ਼ੁਰੂ_us = timer_get_boot_us();

	ਜੇ (rec) {
		rec->ਸ਼ੁਰੂ_us = ਸ਼ੁਰੂ_us;
		rec->name = name;
	}

	ਵਾਪਸ ਸ਼ੁਰੂ_us;
}

uint32_t bootstage_accum(ਗਿਣਤੀ bootstage_id id)
{
	ਬਣਤਰ bootstage_ਅੰਕੜੇ *ਅੰਕੜੇ = gd->bootstage;
	ਬਣਤਰ bootstage_ਰਿਕਾਰਡ *rec = ensure_id(ਅੰਕੜੇ, id);
	uint32_t duration;

	ਜੇ (!rec)
		ਵਾਪਸ 0;
	duration = (uint32_t)timer_get_boot_us() - rec->ਸ਼ੁਰੂ_us;
	rec->ਸਮਾਂ_us += duration;

	ਵਾਪਸ duration;
}

/**
 * Get a record name as a printable string
 *
 * @param buf	Buffer to put name if needed
 * @param len	Length of buffer
 * @param rec	Boot stage record to get the name from
 * Return: pointer to name, either from the record or pointing to buf.
 */
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *get_ਰਿਕਾਰਡ_name(ਅੱਖਰ *buf, ਪੂਰਨ len,
				   ਅਚਲ ਬਣਤਰ bootstage_ਰਿਕਾਰਡ *rec)
{
	ਜੇ (rec->name)
		ਵਾਪਸ rec->name;
	ਨਹੀਂਤਾਂ ਜੇ (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	ਨਹੀਂਤਾਂ
		snprintf(buf, len, "id=%d", rec->id);

	ਵਾਪਸ buf;
}

ਸਥਿਰ uint32_t print_ਸਮਾਂ_ਰਿਕਾਰਡ(ਬਣਤਰ bootstage_ਰਿਕਾਰਡ *rec, uint32_t prev)
{
	ਅੱਖਰ buf[20];

	ਜੇ (prev == -1U) {
		ਲਿਖੋ("%11s", "");
		print_grouped_ull(rec->ਸਮਾਂ_us, BOOTSTAGE_DIGITS);
	} ਨਹੀਂਤਾਂ {
		ਜੇ (prev > rec->ਸਮਾਂ_us)
			prev = 0;
		print_grouped_ull(rec->ਸਮਾਂ_us, BOOTSTAGE_DIGITS);
		print_grouped_ull(rec->ਸਮਾਂ_us - prev, BOOTSTAGE_DIGITS);
	}
	ਲਿਖੋ("  %s\n", get_ਰਿਕਾਰਡ_name(buf, ਆਕਾਰ(buf), rec));

	ਵਾਪਸ rec->ਸਮਾਂ_us;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ਦਾ_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * Return: 0 on success, != 0 on failure.
 */
ਸਥਿਰ ਪੂਰਨ ਜੋੜੋ_bootstages_devicetree(ਬਣਤਰ fdt_ਸਿਰਲੇਖ *blob)
{
	ਬਣਤਰ bootstage_ਅੰਕੜੇ *ਅੰਕੜੇ = gd->bootstage;
	ਪੂਰਨ bootstage;
	ਅੱਖਰ buf[20];
	ਪੂਰਨ recnum;
	ਪੂਰਨ i;

	ਜੇ (!blob)
		ਵਾਪਸ 0;

	/*
	 * Create the node for bootstage.
	 * The address of flat device tree is set up by the command bootm.
	 */
	bootstage = fdt_ਜੋੜੋ_subnode(blob, 0, "bootstage");
	ਜੇ (bootstage < 0)
		ਵਾਪਸ -EINVAL;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	ਲਈ (recnum = ਅੰਕੜੇ->rec_ਗਿਣਤੀ - 1, i = 0; recnum >= 0; recnum--, i++) {
		ਬਣਤਰ bootstage_ਰਿਕਾਰਡ *rec = &ਅੰਕੜੇ->ਰਿਕਾਰਡ[recnum];
		ਪੂਰਨ ਗੰਢ;

		ਜੇ (rec->id != BOOTSTAGE_ID_AWAKE && rec->ਸਮਾਂ_us == 0)
			ਜਾਰੀ;

		ਗੰਢ = fdt_ਜੋੜੋ_subnode(blob, bootstage, simple_itoa(i));
		ਜੇ (ਗੰਢ < 0)
			ਰੋਕੋ;

		/* add properties to the node. */
		ਜੇ (fdt_setprop_string(blob, ਗੰਢ, "name",
				       get_ਰਿਕਾਰਡ_name(buf, ਆਕਾਰ(buf), rec)))
			ਵਾਪਸ -EINVAL;

		/* Check if this is a 'mark' or 'accum' record */
		ਜੇ (fdt_setprop_cell(blob, ਗੰਢ,
				rec->ਸ਼ੁਰੂ_us ? "accum" : "mark",
				rec->ਸਮਾਂ_us))
			ਵਾਪਸ -EINVAL;
	}

	ਵਾਪਸ 0;
}

ਪੂਰਨ bootstage_fdt_ਜੋੜੋ_report(ਖਾਲੀ)
{
	ਜੇ (ਜੋੜੋ_bootstages_devicetree(working_fdt))
		puts("bootstage: Failed to add to device tree\n");

	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

ਖਾਲੀ bootstage_report(ਖਾਲੀ)
{
	ਬਣਤਰ bootstage_ਅੰਕੜੇ *ਅੰਕੜੇ = gd->bootstage;
	ਬਣਤਰ bootstage_ਰਿਕਾਰਡ *rec = ਅੰਕੜੇ->ਰਿਕਾਰਡ;
	uint32_t prev;
	ਪੂਰਨ i;

	ਲਿਖੋ("Timer summary in microseconds (%d records):\n",
	       ਅੰਕੜੇ->rec_ਗਿਣਤੀ);
	ਲਿਖੋ("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	prev = print_ਸਮਾਂ_ਰਿਕਾਰਡ(rec, 0);

	ਲਈ (i = 1, rec++; i < ਅੰਕੜੇ->rec_ਗਿਣਤੀ; i++, rec++) {
		ਜੇ (rec->id && !rec->ਸ਼ੁਰੂ_us)
			prev = print_ਸਮਾਂ_ਰਿਕਾਰਡ(rec, prev);
	}
	ਜੇ (ਅੰਕੜੇ->rec_ਗਿਣਤੀ > ਰਿਕਾਰਡ_ਗਿਣਤੀ)
		ਲਿਖੋ("Overflowed internal boot id table by %d entries\n"
		       "Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		       ਅੰਕੜੇ->rec_ਗਿਣਤੀ - ਰਿਕਾਰਡ_ਗਿਣਤੀ);

	puts("\nAccumulated time:\n");
	ਲਈ (i = 0, rec = ਅੰਕੜੇ->ਰਿਕਾਰਡ; i < ਅੰਕੜੇ->rec_ਗਿਣਤੀ; i++, rec++) {
		ਜੇ (rec->ਸ਼ੁਰੂ_us)
			prev = print_ਸਮਾਂ_ਰਿਕਾਰਡ(rec, -1);
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
ਸਥਿਰ ਖਾਲੀ append_ਅੰਕੜੇ(ਅੱਖਰ **ptrp, ਅੱਖਰ *ਸਿਰਾ_2, ਅਚਲ ਖਾਲੀ *ਅੰਕੜੇ, ਪੂਰਨ ਮਾਪ)
{
	ਅੱਖਰ *ptr = *ptrp;

	*ptrp += ਮਾਪ;
	ਜੇ (*ptrp > ਸਿਰਾ_2)
		ਵਾਪਸ;

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(ptr, ਅੰਕੜੇ, ਮਾਪ);
}

ਪੂਰਨ bootstage_stash(ਖਾਲੀ *ਅਧਾਰ, ਪੂਰਨ ਮਾਪ)
{
	ਅਚਲ ਬਣਤਰ bootstage_ਅੰਕੜੇ *ਅੰਕੜੇ = gd->bootstage;
	ਬਣਤਰ bootstage_hdr *hdr = (ਬਣਤਰ bootstage_hdr *)ਅਧਾਰ;
	ਅਚਲ ਬਣਤਰ bootstage_ਰਿਕਾਰਡ *rec;
	ਅੱਖਰ buf[20];
	ਅੱਖਰ *ptr = ਅਧਾਰ, *ਸਿਰਾ_2 = ptr + ਮਾਪ;
	ਪੂਰਨ i;

	ਜੇ (hdr + 1 > (ਬਣਤਰ bootstage_hdr *)ਸਿਰਾ_2) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		ਵਾਪਸ -ENOSPC;
	}

	/* Write an arbitrary version number */
	hdr->version = BOOTSTAGE_ਸੰਸਕਰਣ;

	hdr->ਗਿਣਤੀ_2 = ਅੰਕੜੇ->rec_ਗਿਣਤੀ;
	hdr->ਮਾਪ = 0;
	hdr->magic = BOOTSTAGE_MAGIC;
	hdr->ਅਗਲਾ_id = ਅੰਕੜੇ->ਅਗਲਾ_id;
	ptr += ਆਕਾਰ(*hdr);

	/* Write the records, silently stopping when we run out of space */
	ਲਈ (rec = ਅੰਕੜੇ->ਰਿਕਾਰਡ, i = 0; i < ਅੰਕੜੇ->rec_ਗਿਣਤੀ; i++, rec++)
		append_ਅੰਕੜੇ(&ptr, ਸਿਰਾ_2, rec, ਆਕਾਰ(*rec));

	/* Write the name strings */
	ਲਈ (rec = ਅੰਕੜੇ->ਰਿਕਾਰਡ, i = 0; i < ਅੰਕੜੇ->rec_ਗਿਣਤੀ; i++, rec++) {
		ਅਚਲ ਅੱਖਰ *name;

		name = get_ਰਿਕਾਰਡ_name(buf, ਆਕਾਰ(buf), rec);
		append_ਅੰਕੜੇ(&ptr, ਸਿਰਾ_2, name, ਲੜੀ_ਲੰਬਾਈ(name) + 1);
	}

	/* Check for buffer overflow */
	ਜੇ (ptr > ਸਿਰਾ_2) {
		debug("%s: Not enough space for bootstage stash\n", __func__);
		ਵਾਪਸ -ENOSPC;
	}

	/* Update total data size */
	hdr->ਮਾਪ = ptr - (ਅੱਖਰ *)ਅਧਾਰ;
	debug("Stashed %d records\n", hdr->ਗਿਣਤੀ_2);

	ਵਾਪਸ 0;
}

ਪੂਰਨ bootstage_unstash(ਅਚਲ ਖਾਲੀ *ਅਧਾਰ, ਪੂਰਨ ਮਾਪ)
{
	ਅਚਲ ਬਣਤਰ bootstage_hdr *hdr = (ਬਣਤਰ bootstage_hdr *)ਅਧਾਰ;
	ਬਣਤਰ bootstage_ਅੰਕੜੇ *ਅੰਕੜੇ = gd->bootstage;
	ਅਚਲ ਅੱਖਰ *ptr = ਅਧਾਰ, *ਸਿਰਾ_2 = ptr + ਮਾਪ;
	ਬਣਤਰ bootstage_ਰਿਕਾਰਡ *rec;
	uint rec_ਮਾਪ;
	ਪੂਰਨ i;

	ਜੇ (ਮਾਪ == -1)
		ਸਿਰਾ_2 = (ਅੱਖਰ *)(~(uintptr_t)0);

	ਜੇ (hdr + 1 > (ਬਣਤਰ bootstage_hdr *)ਸਿਰਾ_2) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		ਵਾਪਸ -EPERM;
	}

	ਜੇ (hdr->magic != BOOTSTAGE_MAGIC) {
		debug("%s: Invalid bootstage magic\n", __func__);
		ਵਾਪਸ -ENOENT;
	}

	ਜੇ (ptr + hdr->ਮਾਪ > ਸਿਰਾ_2) {
		debug("%s: Bootstage data runs past buffer end\n", __func__);
		ਵਾਪਸ -ENOSPC;
	}

	ਜੇ (hdr->ਗਿਣਤੀ_2 * ਆਕਾਰ(*rec) > hdr->ਮਾਪ) {
		debug("%s: Bootstage has %d records needing %lu bytes, but "
			"only %d bytes is available\n", __func__, hdr->ਗਿਣਤੀ_2,
		      (ulong)hdr->ਗਿਣਤੀ_2 * ਆਕਾਰ(*rec), hdr->ਮਾਪ);
		ਵਾਪਸ -ENOSPC;
	}

	ਜੇ (hdr->version != BOOTSTAGE_ਸੰਸਕਰਣ) {
		debug("%s: Bootstage data version %#0x unrecognised\n",
		      __func__, hdr->version);
		ਵਾਪਸ -EINVAL;
	}

	ਜੇ (ਅੰਕੜੇ->rec_ਗਿਣਤੀ + hdr->ਗਿਣਤੀ_2 > ਰਿਕਾਰਡ_ਗਿਣਤੀ) {
		debug("%s: Bootstage has %d records, we have space for %d\n"
			"Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		      __func__, hdr->ਗਿਣਤੀ_2, ਰਿਕਾਰਡ_ਗਿਣਤੀ - ਅੰਕੜੇ->rec_ਗਿਣਤੀ);
		ਵਾਪਸ -ENOSPC;
	}

	ptr += ਆਕਾਰ(*hdr);

	/* Read the records */
	rec_ਮਾਪ = hdr->ਗਿਣਤੀ_2 * ਆਕਾਰ(*ਅੰਕੜੇ->ਰਿਕਾਰਡ);
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਅੰਕੜੇ->ਰਿਕਾਰਡ + ਅੰਕੜੇ->rec_ਗਿਣਤੀ, ptr, rec_ਮਾਪ);

	/* Read the name strings */
	ptr += rec_ਮਾਪ;
	ਲਈ (rec = ਅੰਕੜੇ->ਰਿਕਾਰਡ + ਅੰਕੜੇ->rec_ਗਿਣਤੀ, i = 0; i < hdr->ਗਿਣਤੀ_2;
	     i++, rec++) {
		rec->name = ptr;
		ਜੇ (xpl_phase() == PHASE_SPL)
			rec->name = strdup(ptr);

		/* Assume no data corruption here */
		ptr += ਲੜੀ_ਲੰਬਾਈ(ptr) + 1;
	}

	/* Mark the records as read */
	ਅੰਕੜੇ->rec_ਗਿਣਤੀ += hdr->ਗਿਣਤੀ_2;
	ਅੰਕੜੇ->ਅਗਲਾ_id = hdr->ਅਗਲਾ_id;
	debug("Unstashed %d records\n", hdr->ਗਿਣਤੀ_2);

	ਵਾਪਸ 0;
}

#ਜੇ IS_ENABLED(CONFIG_BOOTSTAGE_STASH)
ਪੂਰਨ _bootstage_stash_default(ਖਾਲੀ)
{
	ਵਾਪਸ bootstage_stash(map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR, 0),
			       CONFIG_BOOTSTAGE_STASH_ਮਾਪ);
}

ਪੂਰਨ _bootstage_unstash_default(ਖਾਲੀ)
{
	ਅਚਲ ਖਾਲੀ *stash = map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR,
				       CONFIG_BOOTSTAGE_STASH_ਮਾਪ);

	ਵਾਪਸ bootstage_unstash(stash, CONFIG_BOOTSTAGE_STASH_ਮਾਪ);
}
#ਜੇ_ਅੰਤ

ਪੂਰਨ bootstage_get_ਮਾਪ(bool ਜੋੜੋ_ਲੜੀਆਂ)
{
	ਪੂਰਨ ਮਾਪ;

	ਮਾਪ = ਆਕਾਰ(ਬਣਤਰ bootstage_ਅੰਕੜੇ);
	ਜੇ (ਜੋੜੋ_ਲੜੀਆਂ) {
		ਬਣਤਰ bootstage_ਅੰਕੜੇ *ਅੰਕੜੇ = gd->bootstage;
		ਬਣਤਰ bootstage_ਰਿਕਾਰਡ *rec;
		ਪੂਰਨ i;

		ਲਈ (rec = ਅੰਕੜੇ->ਰਿਕਾਰਡ, i = 0; i < ਅੰਕੜੇ->rec_ਗਿਣਤੀ; i++, rec++)
			ਜੇ (rec->name)
				ਮਾਪ += ਲੜੀ_ਲੰਬਾਈ(rec->name) + 1;
	}

	ਵਾਪਸ ਮਾਪ;
}

ਪੂਰਨ bootstage_init(bool ਪਹਿਲਾ)
{
	ਬਣਤਰ bootstage_ਅੰਕੜੇ *ਅੰਕੜੇ;
	ਪੂਰਨ ਮਾਪ = ਆਕਾਰ(ਬਣਤਰ bootstage_ਅੰਕੜੇ);

	gd->bootstage = (ਬਣਤਰ bootstage_ਅੰਕੜੇ *)ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਮਾਪ);
	ਜੇ (!gd->bootstage)
		ਵਾਪਸ -ENOMEM;
	ਅੰਕੜੇ = gd->bootstage;
	memset(ਅੰਕੜੇ, '\0', ਮਾਪ);
	ਜੇ (ਪਹਿਲਾ) {
		ਅੰਕੜੇ->ਅਗਲਾ_id = BOOTSTAGE_ID_USER;
		bootstage_ਜੋੜੋ_ਰਿਕਾਰਡ(BOOTSTAGE_ID_AWAKE, "reset", 0, 0);
	}

	ਵਾਪਸ 0;
}
