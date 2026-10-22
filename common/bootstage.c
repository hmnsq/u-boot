// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 */

/*
 * This module records the progress of boot and arbitrary commands, and
 * permits accurate timestamping of each.
 */

#تعریف_کرو LOG_CATEGORY	LOGC_BOOT

#ضمن <bootstage.h>
#ضمن <hang.h>
#ضمن <log.h>
#ضمن <malloc.h>
#ضمن <sort.h>
#ضمن <spl.h>
#ضمن <asm/global_data.h>
#ضمن <linux/compiler.h>
#ضمن <linux/libfdt.h>

DECLARE_GLOBAL_انکڑے_PTR;

تعداد {
	ریکارڈ_گنتی = CONFIG_VAL(BOOTSTAGE_ریکارڈ_گنتی),
};

بنية bootstage_ریکارڈ {
	ulong ویلا_us;
	uint32_t شروع_us;
	ثابت حرف *name;
	صحيح flags;		/* see enum bootstage_flags */
	تعداد bootstage_id id;
};

بنية bootstage_انکڑے {
	uint rec_گنتی;
	uint اگلا_id;
	بنية bootstage_ریکارڈ ریکارڈ[ریکارڈ_گنتی];
};

تعداد {
	BOOTSTAGE_نسخہ	= 0,
	BOOTSTAGE_MAGIC		= 0xb00757a3,
	BOOTSTAGE_DIGITS	= 9,
};

بنية bootstage_hdr {
	u32 version;		/* BOOTSTAGE_VERSION */
	u32 گنتی;		/* Number of records */
	u32 ناپ;		/* Total data size (non-zero if valid) */
	u32 magic;		/* Magic number */
	u32 اگلا_id;		/* Next ID to use for bootstage */
};

صحيح bootstage_relocate(فراغ *منزل)
{
	بنية bootstage_انکڑے *انکڑے;
	صحيح i;
	حرف *ptr;

	debug("Copying bootstage from %p to %p\n", gd->bootstage, منزل);
	حافظہ_نقل(منزل, gd->bootstage, حجم(بنية bootstage_انکڑے));
	انکڑے = gd->bootstage = منزل;

	/* Figure out where to relocate the strings to */
	ptr = (حرف *)(انکڑے + 1);

	/*
	 * Duplicate all strings.  They may point to an old location in the
	 * program .text section that can eventually get trashed.
	 */
	debug("Relocating %d records\n", انکڑے->rec_گنتی);
	لكل (i = 0; i < انکڑے->rec_گنتی; i++) {
		ثابت حرف *from = انکڑے->ریکارڈ[i].name;

		إذا (!from)
			استمر;
		strcpy(ptr, from);
		انکڑے->ریکارڈ[i].name = ptr;
		ptr += لڑی_لمبائی(ptr) + 1;
	}

	أرجع 0;
}

بنية bootstage_ریکارڈ *لبھو_id(بنية bootstage_انکڑے *انکڑے,
				 تعداد bootstage_id id)
{
	بنية bootstage_ریکارڈ *rec;
	بنية bootstage_ریکارڈ *سرا_2;

	لكل (rec = انکڑے->ریکارڈ, سرا_2 = rec + انکڑے->rec_گنتی; rec < سرا_2;
	     rec++) {
		إذا (rec->id == id)
			أرجع rec;
	}

	أرجع عدم;
}

بنية bootstage_ریکارڈ *ensure_id(بنية bootstage_انکڑے *انکڑے,
				   تعداد bootstage_id id)
{
	بنية bootstage_ریکارڈ *rec;

	rec = لبھو_id(انکڑے, id);
	إذا (!rec && انکڑے->rec_گنتی < ریکارڈ_گنتی) {
		rec = &انکڑے->ریکارڈ[انکڑے->rec_گنتی++];
		rec->id = id;
		أرجع rec;
	}

	أرجع rec;
}

ulong bootstage_جوڑو_ریکارڈ(تعداد bootstage_id id, ثابت حرف *name,
			   صحيح flags, ulong mark)
{
	بنية bootstage_انکڑے *انکڑے = gd->bootstage;
	بنية bootstage_ریکارڈ *rec;

	/*
	 * initf_bootstage() is called very early during boot but since hang()
	 * calls bootstage_error() we can be called before bootstage is set up.
	 * Add a check to avoid this.
	 */
	إذا (!انکڑے)
		أرجع mark;
	إذا (flags & BOOTSTAGEF_ALLOC)
		id = انکڑے->اگلا_id++;

	/* Only record the first event for each */
	rec = لبھو_id(انکڑے, id);
	إذا (!rec) {
		إذا (انکڑے->rec_گنتی < ریکارڈ_گنتی) {
			rec = &انکڑے->ریکارڈ[انکڑے->rec_گنتی++];
			rec->ویلا_us = mark;
			rec->name = name;
			rec->flags = flags;
			rec->id = id;
		} وإلا {
			log_warning("Bootstage space exhausted\n");
		}
	}

	/* Tell the board about this progress */
	show_boot_ترقی(flags & BOOTSTAGEF_ERROR ? -id : id);

	أرجع mark;
}

ulong bootstage_error_name(تعداد bootstage_id id, ثابت حرف *name)
{
	أرجع bootstage_جوڑو_ریکارڈ(id, name, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(تعداد bootstage_id id, ثابت حرف *name)
{
	صحيح flags = 0;

	إذا (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;

	أرجع bootstage_جوڑو_ریکارڈ(id, name, flags, timer_get_boot_us());
}

ulong bootstage_mark_کوڈ(ثابت حرف *file, ثابت حرف *func, صحيح linenum)
{
	حرف *str, *p;
	__maybe_unused حرف *سرا_2;
	صحيح len = 0;

	/* First work out the length we need to allocate */
	إذا (linenum != -1)
		len = 11;
	إذا (func)
		len += لڑی_لمبائی(func);
	إذا (file)
		len += لڑی_لمبائی(file);

	str = حافظہ_دیو(len + 1);
	p = str;
	سرا_2 = p + len;
	إذا (file)
		p += snprintf(p, سرا_2 - p, "%s,", file);
	إذا (linenum != -1)
		p += snprintf(p, سرا_2 - p, "%d", linenum);
	إذا (func)
		p += snprintf(p, سرا_2 - p, ": %s", func);

	أرجع bootstage_mark_name(BOOTSTAGE_ID_ALLOC, str);
}

uint32_t bootstage_شروع(تعداد bootstage_id id, ثابت حرف *name)
{
	بنية bootstage_انکڑے *انکڑے = gd->bootstage;
	بنية bootstage_ریکارڈ *rec = ensure_id(انکڑے, id);
	ulong شروع_us = timer_get_boot_us();

	إذا (rec) {
		rec->شروع_us = شروع_us;
		rec->name = name;
	}

	أرجع شروع_us;
}

uint32_t bootstage_accum(تعداد bootstage_id id)
{
	بنية bootstage_انکڑے *انکڑے = gd->bootstage;
	بنية bootstage_ریکارڈ *rec = ensure_id(انکڑے, id);
	uint32_t duration;

	إذا (!rec)
		أرجع 0;
	duration = (uint32_t)timer_get_boot_us() - rec->شروع_us;
	rec->ویلا_us += duration;

	أرجع duration;
}

/**
 * Get a record name as a printable string
 *
 * @param buf	Buffer to put name if needed
 * @param len	Length of buffer
 * @param rec	Boot stage record to get the name from
 * Return: pointer to name, either from the record or pointing to buf.
 */
ساكن ثابت حرف *get_ریکارڈ_name(حرف *buf, صحيح len,
				   ثابت بنية bootstage_ریکارڈ *rec)
{
	إذا (rec->name)
		أرجع rec->name;
	وإلا إذا (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	وإلا
		snprintf(buf, len, "id=%d", rec->id);

	أرجع buf;
}

ساكن uint32_t print_ویلا_ریکارڈ(بنية bootstage_ریکارڈ *rec, uint32_t prev)
{
	حرف buf[20];

	إذا (prev == -1U) {
		لکھو("%11s", "");
		print_grouped_ull(rec->ویلا_us, BOOTSTAGE_DIGITS);
	} وإلا {
		إذا (prev > rec->ویلا_us)
			prev = 0;
		print_grouped_ull(rec->ویلا_us, BOOTSTAGE_DIGITS);
		print_grouped_ull(rec->ویلا_us - prev, BOOTSTAGE_DIGITS);
	}
	لکھو("  %s\n", get_ریکارڈ_name(buf, حجم(buf), rec));

	أرجع rec->ویلا_us;
}

#جے_تعریف_شدہ CONFIG_دا_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * Return: 0 on success, != 0 on failure.
 */
ساكن صحيح جوڑو_bootstages_devicetree(بنية fdt_سرخی *blob)
{
	بنية bootstage_انکڑے *انکڑے = gd->bootstage;
	صحيح bootstage;
	حرف buf[20];
	صحيح recnum;
	صحيح i;

	إذا (!blob)
		أرجع 0;

	/*
	 * Create the node for bootstage.
	 * The address of flat device tree is set up by the command bootm.
	 */
	bootstage = fdt_جوڑو_subnode(blob, 0, "bootstage");
	إذا (bootstage < 0)
		أرجع -EINVAL;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	لكل (recnum = انکڑے->rec_گنتی - 1, i = 0; recnum >= 0; recnum--, i++) {
		بنية bootstage_ریکارڈ *rec = &انکڑے->ریکارڈ[recnum];
		صحيح گرہ;

		إذا (rec->id != BOOTSTAGE_ID_AWAKE && rec->ویلا_us == 0)
			استمر;

		گرہ = fdt_جوڑو_subnode(blob, bootstage, simple_itoa(i));
		إذا (گرہ < 0)
			اكسر;

		/* add properties to the node. */
		إذا (fdt_setprop_string(blob, گرہ, "name",
				       get_ریکارڈ_name(buf, حجم(buf), rec)))
			أرجع -EINVAL;

		/* Check if this is a 'mark' or 'accum' record */
		إذا (fdt_setprop_cell(blob, گرہ,
				rec->شروع_us ? "accum" : "mark",
				rec->ویلا_us))
			أرجع -EINVAL;
	}

	أرجع 0;
}

صحيح bootstage_fdt_جوڑو_report(فراغ)
{
	إذا (جوڑو_bootstages_devicetree(working_fdt))
		puts("bootstage: Failed to add to device tree\n");

	أرجع 0;
}
#جے_انت

فراغ bootstage_report(فراغ)
{
	بنية bootstage_انکڑے *انکڑے = gd->bootstage;
	بنية bootstage_ریکارڈ *rec = انکڑے->ریکارڈ;
	uint32_t prev;
	صحيح i;

	لکھو("Timer summary in microseconds (%d records):\n",
	       انکڑے->rec_گنتی);
	لکھو("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	prev = print_ویلا_ریکارڈ(rec, 0);

	لكل (i = 1, rec++; i < انکڑے->rec_گنتی; i++, rec++) {
		إذا (rec->id && !rec->شروع_us)
			prev = print_ویلا_ریکارڈ(rec, prev);
	}
	إذا (انکڑے->rec_گنتی > ریکارڈ_گنتی)
		لکھو("Overflowed internal boot id table by %d entries\n"
		       "Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		       انکڑے->rec_گنتی - ریکارڈ_گنتی);

	puts("\nAccumulated time:\n");
	لكل (i = 0, rec = انکڑے->ریکارڈ; i < انکڑے->rec_گنتی; i++, rec++) {
		إذا (rec->شروع_us)
			prev = print_ویلا_ریکارڈ(rec, -1);
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
ساكن فراغ append_انکڑے(حرف **ptrp, حرف *سرا_2, ثابت فراغ *انکڑے, صحيح ناپ)
{
	حرف *ptr = *ptrp;

	*ptrp += ناپ;
	إذا (*ptrp > سرا_2)
		أرجع;

	حافظہ_نقل(ptr, انکڑے, ناپ);
}

صحيح bootstage_stash(فراغ *ادھار, صحيح ناپ)
{
	ثابت بنية bootstage_انکڑے *انکڑے = gd->bootstage;
	بنية bootstage_hdr *hdr = (بنية bootstage_hdr *)ادھار;
	ثابت بنية bootstage_ریکارڈ *rec;
	حرف buf[20];
	حرف *ptr = ادھار, *سرا_2 = ptr + ناپ;
	صحيح i;

	إذا (hdr + 1 > (بنية bootstage_hdr *)سرا_2) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		أرجع -ENOSPC;
	}

	/* Write an arbitrary version number */
	hdr->version = BOOTSTAGE_نسخہ;

	hdr->گنتی = انکڑے->rec_گنتی;
	hdr->ناپ = 0;
	hdr->magic = BOOTSTAGE_MAGIC;
	hdr->اگلا_id = انکڑے->اگلا_id;
	ptr += حجم(*hdr);

	/* Write the records, silently stopping when we run out of space */
	لكل (rec = انکڑے->ریکارڈ, i = 0; i < انکڑے->rec_گنتی; i++, rec++)
		append_انکڑے(&ptr, سرا_2, rec, حجم(*rec));

	/* Write the name strings */
	لكل (rec = انکڑے->ریکارڈ, i = 0; i < انکڑے->rec_گنتی; i++, rec++) {
		ثابت حرف *name;

		name = get_ریکارڈ_name(buf, حجم(buf), rec);
		append_انکڑے(&ptr, سرا_2, name, لڑی_لمبائی(name) + 1);
	}

	/* Check for buffer overflow */
	إذا (ptr > سرا_2) {
		debug("%s: Not enough space for bootstage stash\n", __func__);
		أرجع -ENOSPC;
	}

	/* Update total data size */
	hdr->ناپ = ptr - (حرف *)ادھار;
	debug("Stashed %d records\n", hdr->گنتی);

	أرجع 0;
}

صحيح bootstage_unstash(ثابت فراغ *ادھار, صحيح ناپ)
{
	ثابت بنية bootstage_hdr *hdr = (بنية bootstage_hdr *)ادھار;
	بنية bootstage_انکڑے *انکڑے = gd->bootstage;
	ثابت حرف *ptr = ادھار, *سرا_2 = ptr + ناپ;
	بنية bootstage_ریکارڈ *rec;
	uint rec_ناپ;
	صحيح i;

	إذا (ناپ == -1)
		سرا_2 = (حرف *)(~(uintptr_t)0);

	إذا (hdr + 1 > (بنية bootstage_hdr *)سرا_2) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		أرجع -EPERM;
	}

	إذا (hdr->magic != BOOTSTAGE_MAGIC) {
		debug("%s: Invalid bootstage magic\n", __func__);
		أرجع -ENOENT;
	}

	إذا (ptr + hdr->ناپ > سرا_2) {
		debug("%s: Bootstage data runs past buffer end\n", __func__);
		أرجع -ENOSPC;
	}

	إذا (hdr->گنتی * حجم(*rec) > hdr->ناپ) {
		debug("%s: Bootstage has %d records needing %lu bytes, but "
			"only %d bytes is available\n", __func__, hdr->گنتی,
		      (ulong)hdr->گنتی * حجم(*rec), hdr->ناپ);
		أرجع -ENOSPC;
	}

	إذا (hdr->version != BOOTSTAGE_نسخہ) {
		debug("%s: Bootstage data version %#0x unrecognised\n",
		      __func__, hdr->version);
		أرجع -EINVAL;
	}

	إذا (انکڑے->rec_گنتی + hdr->گنتی > ریکارڈ_گنتی) {
		debug("%s: Bootstage has %d records, we have space for %d\n"
			"Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		      __func__, hdr->گنتی, ریکارڈ_گنتی - انکڑے->rec_گنتی);
		أرجع -ENOSPC;
	}

	ptr += حجم(*hdr);

	/* Read the records */
	rec_ناپ = hdr->گنتی * حجم(*انکڑے->ریکارڈ);
	حافظہ_نقل(انکڑے->ریکارڈ + انکڑے->rec_گنتی, ptr, rec_ناپ);

	/* Read the name strings */
	ptr += rec_ناپ;
	لكل (rec = انکڑے->ریکارڈ + انکڑے->rec_گنتی, i = 0; i < hdr->گنتی;
	     i++, rec++) {
		rec->name = ptr;
		إذا (xpl_phase() == PHASE_SPL)
			rec->name = strdup(ptr);

		/* Assume no data corruption here */
		ptr += لڑی_لمبائی(ptr) + 1;
	}

	/* Mark the records as read */
	انکڑے->rec_گنتی += hdr->گنتی;
	انکڑے->اگلا_id = hdr->اگلا_id;
	debug("Unstashed %d records\n", hdr->گنتی);

	أرجع 0;
}

#إذا IS_ENABLED(CONFIG_BOOTSTAGE_STASH)
صحيح _bootstage_stash_default(فراغ)
{
	أرجع bootstage_stash(map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR, 0),
			       CONFIG_BOOTSTAGE_STASH_ناپ);
}

صحيح _bootstage_unstash_default(فراغ)
{
	ثابت فراغ *stash = map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR,
				       CONFIG_BOOTSTAGE_STASH_ناپ);

	أرجع bootstage_unstash(stash, CONFIG_BOOTSTAGE_STASH_ناپ);
}
#جے_انت

صحيح bootstage_get_ناپ(bool جوڑو_لڑیاں)
{
	صحيح ناپ;

	ناپ = حجم(بنية bootstage_انکڑے);
	إذا (جوڑو_لڑیاں) {
		بنية bootstage_انکڑے *انکڑے = gd->bootstage;
		بنية bootstage_ریکارڈ *rec;
		صحيح i;

		لكل (rec = انکڑے->ریکارڈ, i = 0; i < انکڑے->rec_گنتی; i++, rec++)
			إذا (rec->name)
				ناپ += لڑی_لمبائی(rec->name) + 1;
	}

	أرجع ناپ;
}

صحيح bootstage_init(bool پہلا)
{
	بنية bootstage_انکڑے *انکڑے;
	صحيح ناپ = حجم(بنية bootstage_انکڑے);

	gd->bootstage = (بنية bootstage_انکڑے *)حافظہ_دیو(ناپ);
	إذا (!gd->bootstage)
		أرجع -ENOMEM;
	انکڑے = gd->bootstage;
	memset(انکڑے, '\0', ناپ);
	إذا (پہلا) {
		انکڑے->اگلا_id = BOOTSTAGE_ID_USER;
		bootstage_جوڑو_ریکارڈ(BOOTSTAGE_ID_AWAKE, "reset", 0, 0);
	}

	أرجع 0;
}
