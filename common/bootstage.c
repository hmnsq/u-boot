// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 */

/*
 * This module records the progress of boot and arbitrary commands, and
 * permits accurate timestamping of each.
 */

#تعریف_کرو LOG_CATEGORY	LOGC_BOOT

#شامل <bootstage.h>
#شامل <hang.h>
#شامل <log.h>
#شامل <malloc.h>
#شامل <sort.h>
#شامل <spl.h>
#شامل <asm/global_data.h>
#شامل <linux/compiler.h>
#شامل <linux/libfdt.h>

DECLARE_GLOBAL_ڈیٹا_PTR;

شمار {
	ریکارڈ_گنتی = CONFIG_VAL(BOOTSTAGE_ریکارڈ_گنتی),
};

ساخت bootstage_ریکارڈ {
	ulong وقت_us;
	uint32_t آغاز_us;
	ثابت حرف *name;
	صحیح flags;		/* see enum bootstage_flags */
	شمار bootstage_id id;
};

ساخت bootstage_ڈیٹا {
	uint rec_گنتی;
	uint اگلا_id;
	ساخت bootstage_ریکارڈ ریکارڈ[ریکارڈ_گنتی];
};

شمار {
	BOOTSTAGE_نسخہ	= 0,
	BOOTSTAGE_MAGIC		= 0xb00757a3,
	BOOTSTAGE_DIGITS	= 9,
};

ساخت bootstage_hdr {
	u32 version;		/* BOOTSTAGE_VERSION */
	u32 گنتی;		/* Number of records */
	u32 سائز;		/* Total data size (non-zero if valid) */
	u32 magic;		/* Magic number */
	u32 اگلا_id;		/* Next ID to use for bootstage */
};

صحیح bootstage_relocate(خالی *مقصد)
{
	ساخت bootstage_ڈیٹا *ڈیٹا;
	صحیح i;
	حرف *ptr;

	debug("Copying bootstage from %p to %p\n", gd->bootstage, مقصد);
	حافظہ_نقل(مقصد, gd->bootstage, حجم(ساخت bootstage_ڈیٹا));
	ڈیٹا = gd->bootstage = مقصد;

	/* Figure out where to relocate the strings to */
	ptr = (حرف *)(ڈیٹا + 1);

	/*
	 * Duplicate all strings.  They may point to an old location in the
	 * program .text section that can eventually get trashed.
	 */
	debug("Relocating %d records\n", ڈیٹا->rec_گنتی);
	برائے (i = 0; i < ڈیٹا->rec_گنتی; i++) {
		ثابت حرف *from = ڈیٹا->ریکارڈ[i].name;

		اگر (!from)
			جاری;
		strcpy(ptr, from);
		ڈیٹا->ریکارڈ[i].name = ptr;
		ptr += لڑی_لمبائی(ptr) + 1;
	}

	واپس 0;
}

ساخت bootstage_ریکارڈ *ڈھونڈو_id(ساخت bootstage_ڈیٹا *ڈیٹا,
				 شمار bootstage_id id)
{
	ساخت bootstage_ریکارڈ *rec;
	ساخت bootstage_ریکارڈ *انتہا_2;

	برائے (rec = ڈیٹا->ریکارڈ, انتہا_2 = rec + ڈیٹا->rec_گنتی; rec < انتہا_2;
	     rec++) {
		اگر (rec->id == id)
			واپس rec;
	}

	واپس تہی;
}

ساخت bootstage_ریکارڈ *ensure_id(ساخت bootstage_ڈیٹا *ڈیٹا,
				   شمار bootstage_id id)
{
	ساخت bootstage_ریکارڈ *rec;

	rec = ڈھونڈو_id(ڈیٹا, id);
	اگر (!rec && ڈیٹا->rec_گنتی < ریکارڈ_گنتی) {
		rec = &ڈیٹا->ریکارڈ[ڈیٹا->rec_گنتی++];
		rec->id = id;
		واپس rec;
	}

	واپس rec;
}

ulong bootstage_جوڑو_ریکارڈ(شمار bootstage_id id, ثابت حرف *name,
			   صحیح flags, ulong mark)
{
	ساخت bootstage_ڈیٹا *ڈیٹا = gd->bootstage;
	ساخت bootstage_ریکارڈ *rec;

	/*
	 * initf_bootstage() is called very early during boot but since hang()
	 * calls bootstage_error() we can be called before bootstage is set up.
	 * Add a check to avoid this.
	 */
	اگر (!ڈیٹا)
		واپس mark;
	اگر (flags & BOOTSTAGEF_ALLOC)
		id = ڈیٹا->اگلا_id++;

	/* Only record the first event for each */
	rec = ڈھونڈو_id(ڈیٹا, id);
	اگر (!rec) {
		اگر (ڈیٹا->rec_گنتی < ریکارڈ_گنتی) {
			rec = &ڈیٹا->ریکارڈ[ڈیٹا->rec_گنتی++];
			rec->وقت_us = mark;
			rec->name = name;
			rec->flags = flags;
			rec->id = id;
		} ورنہ {
			log_warning("Bootstage space exhausted\n");
		}
	}

	/* Tell the board about this progress */
	show_boot_پیش_رفت(flags & BOOTSTAGEF_ERROR ? -id : id);

	واپس mark;
}

ulong bootstage_error_name(شمار bootstage_id id, ثابت حرف *name)
{
	واپس bootstage_جوڑو_ریکارڈ(id, name, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(شمار bootstage_id id, ثابت حرف *name)
{
	صحیح flags = 0;

	اگر (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;

	واپس bootstage_جوڑو_ریکارڈ(id, name, flags, timer_get_boot_us());
}

ulong bootstage_mark_رمز(ثابت حرف *file, ثابت حرف *func, صحیح linenum)
{
	حرف *str, *p;
	__maybe_unused حرف *انتہا_2;
	صحیح len = 0;

	/* First work out the length we need to allocate */
	اگر (linenum != -1)
		len = 11;
	اگر (func)
		len += لڑی_لمبائی(func);
	اگر (file)
		len += لڑی_لمبائی(file);

	str = حافظہ_دو(len + 1);
	p = str;
	انتہا_2 = p + len;
	اگر (file)
		p += snprintf(p, انتہا_2 - p, "%s,", file);
	اگر (linenum != -1)
		p += snprintf(p, انتہا_2 - p, "%d", linenum);
	اگر (func)
		p += snprintf(p, انتہا_2 - p, ": %s", func);

	واپس bootstage_mark_name(BOOTSTAGE_ID_ALLOC, str);
}

uint32_t bootstage_آغاز(شمار bootstage_id id, ثابت حرف *name)
{
	ساخت bootstage_ڈیٹا *ڈیٹا = gd->bootstage;
	ساخت bootstage_ریکارڈ *rec = ensure_id(ڈیٹا, id);
	ulong آغاز_us = timer_get_boot_us();

	اگر (rec) {
		rec->آغاز_us = آغاز_us;
		rec->name = name;
	}

	واپس آغاز_us;
}

uint32_t bootstage_accum(شمار bootstage_id id)
{
	ساخت bootstage_ڈیٹا *ڈیٹا = gd->bootstage;
	ساخت bootstage_ریکارڈ *rec = ensure_id(ڈیٹا, id);
	uint32_t duration;

	اگر (!rec)
		واپس 0;
	duration = (uint32_t)timer_get_boot_us() - rec->آغاز_us;
	rec->وقت_us += duration;

	واپس duration;
}

/**
 * Get a record name as a printable string
 *
 * @param buf	Buffer to put name if needed
 * @param len	Length of buffer
 * @param rec	Boot stage record to get the name from
 * Return: pointer to name, either from the record or pointing to buf.
 */
ساکن ثابت حرف *get_ریکارڈ_name(حرف *buf, صحیح len,
				   ثابت ساخت bootstage_ریکارڈ *rec)
{
	اگر (rec->name)
		واپس rec->name;
	ورنہ اگر (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	ورنہ
		snprintf(buf, len, "id=%d", rec->id);

	واپس buf;
}

ساکن uint32_t print_وقت_ریکارڈ(ساخت bootstage_ریکارڈ *rec, uint32_t پچھلا)
{
	حرف buf[20];

	اگر (پچھلا == -1U) {
		لکھو("%11s", "");
		print_grouped_ull(rec->وقت_us, BOOTSTAGE_DIGITS);
	} ورنہ {
		اگر (پچھلا > rec->وقت_us)
			پچھلا = 0;
		print_grouped_ull(rec->وقت_us, BOOTSTAGE_DIGITS);
		print_grouped_ull(rec->وقت_us - پچھلا, BOOTSTAGE_DIGITS);
	}
	لکھو("  %s\n", get_ریکارڈ_name(buf, حجم(buf), rec));

	واپس rec->وقت_us;
}

#اگر_تعریف_شدہ CONFIG_کا_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * Return: 0 on success, != 0 on failure.
 */
ساکن صحیح جوڑو_bootstages_devicetree(ساخت fdt_سرخی *blob)
{
	ساخت bootstage_ڈیٹا *ڈیٹا = gd->bootstage;
	صحیح bootstage;
	حرف buf[20];
	صحیح recnum;
	صحیح i;

	اگر (!blob)
		واپس 0;

	/*
	 * Create the node for bootstage.
	 * The address of flat device tree is set up by the command bootm.
	 */
	bootstage = fdt_جوڑو_subnode(blob, 0, "bootstage");
	اگر (bootstage < 0)
		واپس -EINVAL;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	برائے (recnum = ڈیٹا->rec_گنتی - 1, i = 0; recnum >= 0; recnum--, i++) {
		ساخت bootstage_ریکارڈ *rec = &ڈیٹا->ریکارڈ[recnum];
		صحیح گرہ;

		اگر (rec->id != BOOTSTAGE_ID_AWAKE && rec->وقت_us == 0)
			جاری;

		گرہ = fdt_جوڑو_subnode(blob, bootstage, simple_itoa(i));
		اگر (گرہ < 0)
			توڑو;

		/* add properties to the node. */
		اگر (fdt_setprop_string(blob, گرہ, "name",
				       get_ریکارڈ_name(buf, حجم(buf), rec)))
			واپس -EINVAL;

		/* Check if this is a 'mark' or 'accum' record */
		اگر (fdt_setprop_cell(blob, گرہ,
				rec->آغاز_us ? "accum" : "mark",
				rec->وقت_us))
			واپس -EINVAL;
	}

	واپس 0;
}

صحیح bootstage_fdt_جوڑو_report(خالی)
{
	اگر (جوڑو_bootstages_devicetree(working_fdt))
		puts("bootstage: Failed to add to device tree\n");

	واپس 0;
}
#اگر_ختم

خالی bootstage_report(خالی)
{
	ساخت bootstage_ڈیٹا *ڈیٹا = gd->bootstage;
	ساخت bootstage_ریکارڈ *rec = ڈیٹا->ریکارڈ;
	uint32_t پچھلا;
	صحیح i;

	لکھو("Timer summary in microseconds (%d records):\n",
	       ڈیٹا->rec_گنتی);
	لکھو("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	پچھلا = print_وقت_ریکارڈ(rec, 0);

	برائے (i = 1, rec++; i < ڈیٹا->rec_گنتی; i++, rec++) {
		اگر (rec->id && !rec->آغاز_us)
			پچھلا = print_وقت_ریکارڈ(rec, پچھلا);
	}
	اگر (ڈیٹا->rec_گنتی > ریکارڈ_گنتی)
		لکھو("Overflowed internal boot id table by %d entries\n"
		       "Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		       ڈیٹا->rec_گنتی - ریکارڈ_گنتی);

	puts("\nAccumulated time:\n");
	برائے (i = 0, rec = ڈیٹا->ریکارڈ; i < ڈیٹا->rec_گنتی; i++, rec++) {
		اگر (rec->آغاز_us)
			پچھلا = print_وقت_ریکارڈ(rec, -1);
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
ساکن خالی append_ڈیٹا(حرف **ptrp, حرف *انتہا_2, ثابت خالی *ڈیٹا, صحیح سائز)
{
	حرف *ptr = *ptrp;

	*ptrp += سائز;
	اگر (*ptrp > انتہا_2)
		واپس;

	حافظہ_نقل(ptr, ڈیٹا, سائز);
}

صحیح bootstage_stash(خالی *بنیاد, صحیح سائز)
{
	ثابت ساخت bootstage_ڈیٹا *ڈیٹا = gd->bootstage;
	ساخت bootstage_hdr *hdr = (ساخت bootstage_hdr *)بنیاد;
	ثابت ساخت bootstage_ریکارڈ *rec;
	حرف buf[20];
	حرف *ptr = بنیاد, *انتہا_2 = ptr + سائز;
	صحیح i;

	اگر (hdr + 1 > (ساخت bootstage_hdr *)انتہا_2) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		واپس -ENOSPC;
	}

	/* Write an arbitrary version number */
	hdr->version = BOOTSTAGE_نسخہ;

	hdr->گنتی = ڈیٹا->rec_گنتی;
	hdr->سائز = 0;
	hdr->magic = BOOTSTAGE_MAGIC;
	hdr->اگلا_id = ڈیٹا->اگلا_id;
	ptr += حجم(*hdr);

	/* Write the records, silently stopping when we run out of space */
	برائے (rec = ڈیٹا->ریکارڈ, i = 0; i < ڈیٹا->rec_گنتی; i++, rec++)
		append_ڈیٹا(&ptr, انتہا_2, rec, حجم(*rec));

	/* Write the name strings */
	برائے (rec = ڈیٹا->ریکارڈ, i = 0; i < ڈیٹا->rec_گنتی; i++, rec++) {
		ثابت حرف *name;

		name = get_ریکارڈ_name(buf, حجم(buf), rec);
		append_ڈیٹا(&ptr, انتہا_2, name, لڑی_لمبائی(name) + 1);
	}

	/* Check for buffer overflow */
	اگر (ptr > انتہا_2) {
		debug("%s: Not enough space for bootstage stash\n", __func__);
		واپس -ENOSPC;
	}

	/* Update total data size */
	hdr->سائز = ptr - (حرف *)بنیاد;
	debug("Stashed %d records\n", hdr->گنتی);

	واپس 0;
}

صحیح bootstage_unstash(ثابت خالی *بنیاد, صحیح سائز)
{
	ثابت ساخت bootstage_hdr *hdr = (ساخت bootstage_hdr *)بنیاد;
	ساخت bootstage_ڈیٹا *ڈیٹا = gd->bootstage;
	ثابت حرف *ptr = بنیاد, *انتہا_2 = ptr + سائز;
	ساخت bootstage_ریکارڈ *rec;
	uint rec_سائز;
	صحیح i;

	اگر (سائز == -1)
		انتہا_2 = (حرف *)(~(uintptr_t)0);

	اگر (hdr + 1 > (ساخت bootstage_hdr *)انتہا_2) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		واپس -EPERM;
	}

	اگر (hdr->magic != BOOTSTAGE_MAGIC) {
		debug("%s: Invalid bootstage magic\n", __func__);
		واپس -ENOENT;
	}

	اگر (ptr + hdr->سائز > انتہا_2) {
		debug("%s: Bootstage data runs past buffer end\n", __func__);
		واپس -ENOSPC;
	}

	اگر (hdr->گنتی * حجم(*rec) > hdr->سائز) {
		debug("%s: Bootstage has %d records needing %lu bytes, but "
			"only %d bytes is available\n", __func__, hdr->گنتی,
		      (ulong)hdr->گنتی * حجم(*rec), hdr->سائز);
		واپس -ENOSPC;
	}

	اگر (hdr->version != BOOTSTAGE_نسخہ) {
		debug("%s: Bootstage data version %#0x unrecognised\n",
		      __func__, hdr->version);
		واپس -EINVAL;
	}

	اگر (ڈیٹا->rec_گنتی + hdr->گنتی > ریکارڈ_گنتی) {
		debug("%s: Bootstage has %d records, we have space for %d\n"
			"Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		      __func__, hdr->گنتی, ریکارڈ_گنتی - ڈیٹا->rec_گنتی);
		واپس -ENOSPC;
	}

	ptr += حجم(*hdr);

	/* Read the records */
	rec_سائز = hdr->گنتی * حجم(*ڈیٹا->ریکارڈ);
	حافظہ_نقل(ڈیٹا->ریکارڈ + ڈیٹا->rec_گنتی, ptr, rec_سائز);

	/* Read the name strings */
	ptr += rec_سائز;
	برائے (rec = ڈیٹا->ریکارڈ + ڈیٹا->rec_گنتی, i = 0; i < hdr->گنتی;
	     i++, rec++) {
		rec->name = ptr;
		اگر (xpl_phase() == PHASE_SPL)
			rec->name = strdup(ptr);

		/* Assume no data corruption here */
		ptr += لڑی_لمبائی(ptr) + 1;
	}

	/* Mark the records as read */
	ڈیٹا->rec_گنتی += hdr->گنتی;
	ڈیٹا->اگلا_id = hdr->اگلا_id;
	debug("Unstashed %d records\n", hdr->گنتی);

	واپس 0;
}

#اگر IS_ENABLED(CONFIG_BOOTSTAGE_STASH)
صحیح _bootstage_stash_default(خالی)
{
	واپس bootstage_stash(map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR, 0),
			       CONFIG_BOOTSTAGE_STASH_سائز);
}

صحیح _bootstage_unstash_default(خالی)
{
	ثابت خالی *stash = map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR,
				       CONFIG_BOOTSTAGE_STASH_سائز);

	واپس bootstage_unstash(stash, CONFIG_BOOTSTAGE_STASH_سائز);
}
#اگر_ختم

صحیح bootstage_get_سائز(bool جوڑو_لڑیاں)
{
	صحیح سائز;

	سائز = حجم(ساخت bootstage_ڈیٹا);
	اگر (جوڑو_لڑیاں) {
		ساخت bootstage_ڈیٹا *ڈیٹا = gd->bootstage;
		ساخت bootstage_ریکارڈ *rec;
		صحیح i;

		برائے (rec = ڈیٹا->ریکارڈ, i = 0; i < ڈیٹا->rec_گنتی; i++, rec++)
			اگر (rec->name)
				سائز += لڑی_لمبائی(rec->name) + 1;
	}

	واپس سائز;
}

صحیح bootstage_init(bool پہلا)
{
	ساخت bootstage_ڈیٹا *ڈیٹا;
	صحیح سائز = حجم(ساخت bootstage_ڈیٹا);

	gd->bootstage = (ساخت bootstage_ڈیٹا *)حافظہ_دو(سائز);
	اگر (!gd->bootstage)
		واپس -ENOMEM;
	ڈیٹا = gd->bootstage;
	memset(ڈیٹا, '\0', سائز);
	اگر (پہلا) {
		ڈیٹا->اگلا_id = BOOTSTAGE_ID_USER;
		bootstage_جوڑو_ریکارڈ(BOOTSTAGE_ID_AWAKE, "reset", 0, 0);
	}

	واپس 0;
}
