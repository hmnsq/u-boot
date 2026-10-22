// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 */

/*
 * This module records the progress of boot and arbitrary commands, and
 * permits accurate timestamping of each.
 */

#تعریف_کړه LOG_CATEGORY	LOGC_BOOT

#ضمن <bootstage.h>
#ضمن <hang.h>
#ضمن <log.h>
#ضمن <malloc.h>
#ضمن <sort.h>
#ضمن <spl.h>
#ضمن <asm/global_data.h>
#ضمن <linux/compiler.h>
#ضمن <linux/libfdt.h>

DECLARE_GLOBAL_معلومات_PTR;

تعداد {
	ریکارډ_شمېر = CONFIG_VAL(BOOTSTAGE_ریکارډ_شمېر),
};

بنية bootstage_ریکارډ {
	ulong وخت_us;
	uint32_t پیل_us;
	ثابت حرف *name;
	صحيح flags;		/* see enum bootstage_flags */
	تعداد bootstage_id id;
};

بنية bootstage_معلومات {
	uint rec_شمېر;
	uint بل_id;
	بنية bootstage_ریکارډ ریکارډ[ریکارډ_شمېر];
};

تعداد {
	BOOTSTAGE_نسخه	= 0,
	BOOTSTAGE_MAGIC		= 0xb00757a3,
	BOOTSTAGE_DIGITS	= 9,
};

بنية bootstage_hdr {
	u32 version;		/* BOOTSTAGE_VERSION */
	u32 شمېر;		/* Number of records */
	u32 اندازه;		/* Total data size (non-zero if valid) */
	u32 magic;		/* Magic number */
	u32 بل_id;		/* Next ID to use for bootstage */
};

صحيح bootstage_relocate(فراغ *منزل)
{
	بنية bootstage_معلومات *معلومات;
	صحيح i;
	حرف *ptr;

	debug("Copying bootstage from %p to %p\n", gd->bootstage, منزل);
	حافظه_نقل(منزل, gd->bootstage, حجم(بنية bootstage_معلومات));
	معلومات = gd->bootstage = منزل;

	/* Figure out where to relocate the strings to */
	ptr = (حرف *)(معلومات + 1);

	/*
	 * Duplicate all strings.  They may point to an old location in the
	 * program .text section that can eventually get trashed.
	 */
	debug("Relocating %d records\n", معلومات->rec_شمېر);
	لكل (i = 0; i < معلومات->rec_شمېر; i++) {
		ثابت حرف *from = معلومات->ریکارډ[i].name;

		إذا (!from)
			استمر;
		strcpy(ptr, from);
		معلومات->ریکارډ[i].name = ptr;
		ptr += مزي_اوږدوالی(ptr) + 1;
	}

	أرجع 0;
}

بنية bootstage_ریکارډ *پیدا_کړه_id(بنية bootstage_معلومات *معلومات,
				 تعداد bootstage_id id)
{
	بنية bootstage_ریکارډ *rec;
	بنية bootstage_ریکارډ *پای_ټکی_2;

	لكل (rec = معلومات->ریکارډ, پای_ټکی_2 = rec + معلومات->rec_شمېر; rec < پای_ټکی_2;
	     rec++) {
		إذا (rec->id == id)
			أرجع rec;
	}

	أرجع عدم;
}

بنية bootstage_ریکارډ *ensure_id(بنية bootstage_معلومات *معلومات,
				   تعداد bootstage_id id)
{
	بنية bootstage_ریکارډ *rec;

	rec = پیدا_کړه_id(معلومات, id);
	إذا (!rec && معلومات->rec_شمېر < ریکارډ_شمېر) {
		rec = &معلومات->ریکارډ[معلومات->rec_شمېر++];
		rec->id = id;
		أرجع rec;
	}

	أرجع rec;
}

ulong bootstage_جمع_کړه_ریکارډ(تعداد bootstage_id id, ثابت حرف *name,
			   صحيح flags, ulong mark)
{
	بنية bootstage_معلومات *معلومات = gd->bootstage;
	بنية bootstage_ریکارډ *rec;

	/*
	 * initf_bootstage() is called very early during boot but since hang()
	 * calls bootstage_error() we can be called before bootstage is set up.
	 * Add a check to avoid this.
	 */
	إذا (!معلومات)
		أرجع mark;
	إذا (flags & BOOTSTAGEF_ALLOC)
		id = معلومات->بل_id++;

	/* Only record the first event for each */
	rec = پیدا_کړه_id(معلومات, id);
	إذا (!rec) {
		إذا (معلومات->rec_شمېر < ریکارډ_شمېر) {
			rec = &معلومات->ریکارډ[معلومات->rec_شمېر++];
			rec->وخت_us = mark;
			rec->name = name;
			rec->flags = flags;
			rec->id = id;
		} وإلا {
			log_warning("Bootstage space exhausted\n");
		}
	}

	/* Tell the board about this progress */
	show_boot_پرمختګ(flags & BOOTSTAGEF_ERROR ? -id : id);

	أرجع mark;
}

ulong bootstage_error_name(تعداد bootstage_id id, ثابت حرف *name)
{
	أرجع bootstage_جمع_کړه_ریکارډ(id, name, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(تعداد bootstage_id id, ثابت حرف *name)
{
	صحيح flags = 0;

	إذا (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;

	أرجع bootstage_جمع_کړه_ریکارډ(id, name, flags, timer_get_boot_us());
}

ulong bootstage_mark_کوډ(ثابت حرف *file, ثابت حرف *func, صحيح linenum)
{
	حرف *str, *p;
	__maybe_unused حرف *پای_ټکی_2;
	صحيح len = 0;

	/* First work out the length we need to allocate */
	إذا (linenum != -1)
		len = 11;
	إذا (func)
		len += مزي_اوږدوالی(func);
	إذا (file)
		len += مزي_اوږدوالی(file);

	str = حافظه_ورکړه(len + 1);
	p = str;
	پای_ټکی_2 = p + len;
	إذا (file)
		p += snprintf(p, پای_ټکی_2 - p, "%s,", file);
	إذا (linenum != -1)
		p += snprintf(p, پای_ټکی_2 - p, "%d", linenum);
	إذا (func)
		p += snprintf(p, پای_ټکی_2 - p, ": %s", func);

	أرجع bootstage_mark_name(BOOTSTAGE_ID_ALLOC, str);
}

uint32_t bootstage_پیل(تعداد bootstage_id id, ثابت حرف *name)
{
	بنية bootstage_معلومات *معلومات = gd->bootstage;
	بنية bootstage_ریکارډ *rec = ensure_id(معلومات, id);
	ulong پیل_us = timer_get_boot_us();

	إذا (rec) {
		rec->پیل_us = پیل_us;
		rec->name = name;
	}

	أرجع پیل_us;
}

uint32_t bootstage_accum(تعداد bootstage_id id)
{
	بنية bootstage_معلومات *معلومات = gd->bootstage;
	بنية bootstage_ریکارډ *rec = ensure_id(معلومات, id);
	uint32_t duration;

	إذا (!rec)
		أرجع 0;
	duration = (uint32_t)timer_get_boot_us() - rec->پیل_us;
	rec->وخت_us += duration;

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
ساكن ثابت حرف *get_ریکارډ_name(حرف *buf, صحيح len,
				   ثابت بنية bootstage_ریکارډ *rec)
{
	إذا (rec->name)
		أرجع rec->name;
	وإلا إذا (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	وإلا
		snprintf(buf, len, "id=%d", rec->id);

	أرجع buf;
}

ساكن uint32_t print_وخت_ریکارډ(بنية bootstage_ریکارډ *rec, uint32_t prev)
{
	حرف buf[20];

	إذا (prev == -1U) {
		چاپ_کړه("%11s", "");
		print_grouped_ull(rec->وخت_us, BOOTSTAGE_DIGITS);
	} وإلا {
		إذا (prev > rec->وخت_us)
			prev = 0;
		print_grouped_ull(rec->وخت_us, BOOTSTAGE_DIGITS);
		print_grouped_ull(rec->وخت_us - prev, BOOTSTAGE_DIGITS);
	}
	چاپ_کړه("  %s\n", get_ریکارډ_name(buf, حجم(buf), rec));

	أرجع rec->وخت_us;
}

#که_تعریف_شوی CONFIG_د_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * Return: 0 on success, != 0 on failure.
 */
ساكن صحيح جمع_کړه_bootstages_devicetree(بنية fdt_سرلیک *blob)
{
	بنية bootstage_معلومات *معلومات = gd->bootstage;
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
	bootstage = fdt_جمع_کړه_subnode(blob, 0, "bootstage");
	إذا (bootstage < 0)
		أرجع -EINVAL;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	لكل (recnum = معلومات->rec_شمېر - 1, i = 0; recnum >= 0; recnum--, i++) {
		بنية bootstage_ریکارډ *rec = &معلومات->ریکارډ[recnum];
		صحيح غوټه;

		إذا (rec->id != BOOTSTAGE_ID_AWAKE && rec->وخت_us == 0)
			استمر;

		غوټه = fdt_جمع_کړه_subnode(blob, bootstage, simple_itoa(i));
		إذا (غوټه < 0)
			اكسر;

		/* add properties to the node. */
		إذا (fdt_setprop_string(blob, غوټه, "name",
				       get_ریکارډ_name(buf, حجم(buf), rec)))
			أرجع -EINVAL;

		/* Check if this is a 'mark' or 'accum' record */
		إذا (fdt_setprop_cell(blob, غوټه,
				rec->پیل_us ? "accum" : "mark",
				rec->وخت_us))
			أرجع -EINVAL;
	}

	أرجع 0;
}

صحيح bootstage_fdt_جمع_کړه_report(فراغ)
{
	إذا (جمع_کړه_bootstages_devicetree(working_fdt))
		puts("bootstage: Failed to add to device tree\n");

	أرجع 0;
}
#د_که_پای

فراغ bootstage_report(فراغ)
{
	بنية bootstage_معلومات *معلومات = gd->bootstage;
	بنية bootstage_ریکارډ *rec = معلومات->ریکارډ;
	uint32_t prev;
	صحيح i;

	چاپ_کړه("Timer summary in microseconds (%d records):\n",
	       معلومات->rec_شمېر);
	چاپ_کړه("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	prev = print_وخت_ریکارډ(rec, 0);

	لكل (i = 1, rec++; i < معلومات->rec_شمېر; i++, rec++) {
		إذا (rec->id && !rec->پیل_us)
			prev = print_وخت_ریکارډ(rec, prev);
	}
	إذا (معلومات->rec_شمېر > ریکارډ_شمېر)
		چاپ_کړه("Overflowed internal boot id table by %d entries\n"
		       "Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		       معلومات->rec_شمېر - ریکارډ_شمېر);

	puts("\nAccumulated time:\n");
	لكل (i = 0, rec = معلومات->ریکارډ; i < معلومات->rec_شمېر; i++, rec++) {
		إذا (rec->پیل_us)
			prev = print_وخت_ریکارډ(rec, -1);
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
ساكن فراغ append_معلومات(حرف **ptrp, حرف *پای_ټکی_2, ثابت فراغ *معلومات, صحيح اندازه)
{
	حرف *ptr = *ptrp;

	*ptrp += اندازه;
	إذا (*ptrp > پای_ټکی_2)
		أرجع;

	حافظه_نقل(ptr, معلومات, اندازه);
}

صحيح bootstage_stash(فراغ *بنسټ, صحيح اندازه)
{
	ثابت بنية bootstage_معلومات *معلومات = gd->bootstage;
	بنية bootstage_hdr *hdr = (بنية bootstage_hdr *)بنسټ;
	ثابت بنية bootstage_ریکارډ *rec;
	حرف buf[20];
	حرف *ptr = بنسټ, *پای_ټکی_2 = ptr + اندازه;
	صحيح i;

	إذا (hdr + 1 > (بنية bootstage_hdr *)پای_ټکی_2) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		أرجع -ENOSPC;
	}

	/* Write an arbitrary version number */
	hdr->version = BOOTSTAGE_نسخه;

	hdr->شمېر = معلومات->rec_شمېر;
	hdr->اندازه = 0;
	hdr->magic = BOOTSTAGE_MAGIC;
	hdr->بل_id = معلومات->بل_id;
	ptr += حجم(*hdr);

	/* Write the records, silently stopping when we run out of space */
	لكل (rec = معلومات->ریکارډ, i = 0; i < معلومات->rec_شمېر; i++, rec++)
		append_معلومات(&ptr, پای_ټکی_2, rec, حجم(*rec));

	/* Write the name strings */
	لكل (rec = معلومات->ریکارډ, i = 0; i < معلومات->rec_شمېر; i++, rec++) {
		ثابت حرف *name;

		name = get_ریکارډ_name(buf, حجم(buf), rec);
		append_معلومات(&ptr, پای_ټکی_2, name, مزي_اوږدوالی(name) + 1);
	}

	/* Check for buffer overflow */
	إذا (ptr > پای_ټکی_2) {
		debug("%s: Not enough space for bootstage stash\n", __func__);
		أرجع -ENOSPC;
	}

	/* Update total data size */
	hdr->اندازه = ptr - (حرف *)بنسټ;
	debug("Stashed %d records\n", hdr->شمېر);

	أرجع 0;
}

صحيح bootstage_unstash(ثابت فراغ *بنسټ, صحيح اندازه)
{
	ثابت بنية bootstage_hdr *hdr = (بنية bootstage_hdr *)بنسټ;
	بنية bootstage_معلومات *معلومات = gd->bootstage;
	ثابت حرف *ptr = بنسټ, *پای_ټکی_2 = ptr + اندازه;
	بنية bootstage_ریکارډ *rec;
	uint rec_اندازه;
	صحيح i;

	إذا (اندازه == -1)
		پای_ټکی_2 = (حرف *)(~(uintptr_t)0);

	إذا (hdr + 1 > (بنية bootstage_hdr *)پای_ټکی_2) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		أرجع -EPERM;
	}

	إذا (hdr->magic != BOOTSTAGE_MAGIC) {
		debug("%s: Invalid bootstage magic\n", __func__);
		أرجع -ENOENT;
	}

	إذا (ptr + hdr->اندازه > پای_ټکی_2) {
		debug("%s: Bootstage data runs past buffer end\n", __func__);
		أرجع -ENOSPC;
	}

	إذا (hdr->شمېر * حجم(*rec) > hdr->اندازه) {
		debug("%s: Bootstage has %d records needing %lu bytes, but "
			"only %d bytes is available\n", __func__, hdr->شمېر,
		      (ulong)hdr->شمېر * حجم(*rec), hdr->اندازه);
		أرجع -ENOSPC;
	}

	إذا (hdr->version != BOOTSTAGE_نسخه) {
		debug("%s: Bootstage data version %#0x unrecognised\n",
		      __func__, hdr->version);
		أرجع -EINVAL;
	}

	إذا (معلومات->rec_شمېر + hdr->شمېر > ریکارډ_شمېر) {
		debug("%s: Bootstage has %d records, we have space for %d\n"
			"Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		      __func__, hdr->شمېر, ریکارډ_شمېر - معلومات->rec_شمېر);
		أرجع -ENOSPC;
	}

	ptr += حجم(*hdr);

	/* Read the records */
	rec_اندازه = hdr->شمېر * حجم(*معلومات->ریکارډ);
	حافظه_نقل(معلومات->ریکارډ + معلومات->rec_شمېر, ptr, rec_اندازه);

	/* Read the name strings */
	ptr += rec_اندازه;
	لكل (rec = معلومات->ریکارډ + معلومات->rec_شمېر, i = 0; i < hdr->شمېر;
	     i++, rec++) {
		rec->name = ptr;
		إذا (xpl_phase() == PHASE_SPL)
			rec->name = strdup(ptr);

		/* Assume no data corruption here */
		ptr += مزي_اوږدوالی(ptr) + 1;
	}

	/* Mark the records as read */
	معلومات->rec_شمېر += hdr->شمېر;
	معلومات->بل_id = hdr->بل_id;
	debug("Unstashed %d records\n", hdr->شمېر);

	أرجع 0;
}

#إذا IS_ENABLED(CONFIG_BOOTSTAGE_STASH)
صحيح _bootstage_stash_default(فراغ)
{
	أرجع bootstage_stash(map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR, 0),
			       CONFIG_BOOTSTAGE_STASH_اندازه);
}

صحيح _bootstage_unstash_default(فراغ)
{
	ثابت فراغ *stash = map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR,
				       CONFIG_BOOTSTAGE_STASH_اندازه);

	أرجع bootstage_unstash(stash, CONFIG_BOOTSTAGE_STASH_اندازه);
}
#د_که_پای

صحيح bootstage_get_اندازه(bool جمع_کړه_مزي_ګانې)
{
	صحيح اندازه;

	اندازه = حجم(بنية bootstage_معلومات);
	إذا (جمع_کړه_مزي_ګانې) {
		بنية bootstage_معلومات *معلومات = gd->bootstage;
		بنية bootstage_ریکارډ *rec;
		صحيح i;

		لكل (rec = معلومات->ریکارډ, i = 0; i < معلومات->rec_شمېر; i++, rec++)
			إذا (rec->name)
				اندازه += مزي_اوږدوالی(rec->name) + 1;
	}

	أرجع اندازه;
}

صحيح bootstage_init(bool لومړی)
{
	بنية bootstage_معلومات *معلومات;
	صحيح اندازه = حجم(بنية bootstage_معلومات);

	gd->bootstage = (بنية bootstage_معلومات *)حافظه_ورکړه(اندازه);
	إذا (!gd->bootstage)
		أرجع -ENOMEM;
	معلومات = gd->bootstage;
	memset(معلومات, '\0', اندازه);
	إذا (لومړی) {
		معلومات->بل_id = BOOTSTAGE_ID_USER;
		bootstage_جمع_کړه_ریکارډ(BOOTSTAGE_ID_AWAKE, "reset", 0, 0);
	}

	أرجع 0;
}
