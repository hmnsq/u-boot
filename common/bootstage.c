// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 */

/*
 * This module records the progress of boot and arbitrary commands, and
 * permits accurate timestamping of each.
 */

#تعریف_کن LOG_CATEGORY	LOGC_BOOT

#ضمن <bootstage.h>
#ضمن <hang.h>
#ضمن <log.h>
#ضمن <malloc.h>
#ضمن <sort.h>
#ضمن <spl.h>
#ضمن <asm/global_data.h>
#ضمن <linux/compiler.h>
#ضمن <linux/libfdt.h>

DECLARE_GLOBAL_داده_PTR;

تعداد {
	رکورد_شمار = CONFIG_VAL(BOOTSTAGE_رکورد_شمار),
};

بنية bootstage_رکورد {
	ulong زمان_us;
	uint32_t شروع_us;
	ثابت حرف *name;
	صحيح flags;		/* see enum bootstage_flags */
	تعداد bootstage_id id;
};

بنية bootstage_داده {
	uint rec_شمار;
	uint بعدی_id;
	بنية bootstage_رکورد رکورد[رکورد_شمار];
};

تعداد {
	BOOTSTAGE_نسخه	= 0,
	BOOTSTAGE_MAGIC		= 0xb00757a3,
	BOOTSTAGE_DIGITS	= 9,
};

بنية bootstage_hdr {
	u32 version;		/* BOOTSTAGE_VERSION */
	u32 شمار;		/* Number of records */
	u32 اندازه;		/* Total data size (non-zero if valid) */
	u32 magic;		/* Magic number */
	u32 بعدی_id;		/* Next ID to use for bootstage */
};

صحيح bootstage_relocate(فراغ *مقصد)
{
	بنية bootstage_داده *داده;
	صحيح i;
	حرف *ptr;

	debug("Copying bootstage from %p to %p\n", gd->bootstage, مقصد);
	کپی_حافظه(مقصد, gd->bootstage, حجم(بنية bootstage_داده));
	داده = gd->bootstage = مقصد;

	/* Figure out where to relocate the strings to */
	ptr = (حرف *)(داده + 1);

	/*
	 * Duplicate all strings.  They may point to an old location in the
	 * program .text section that can eventually get trashed.
	 */
	debug("Relocating %d records\n", داده->rec_شمار);
	لكل (i = 0; i < داده->rec_شمار; i++) {
		ثابت حرف *from = داده->رکورد[i].name;

		إذا (!from)
			استمر;
		strcpy(ptr, from);
		داده->رکورد[i].name = ptr;
		ptr += طول_رشته(ptr) + 1;
	}

	أرجع 0;
}

بنية bootstage_رکورد *پیدا_کن_id(بنية bootstage_داده *داده,
				 تعداد bootstage_id id)
{
	بنية bootstage_رکورد *rec;
	بنية bootstage_رکورد *انتها_2;

	لكل (rec = داده->رکورد, انتها_2 = rec + داده->rec_شمار; rec < انتها_2;
	     rec++) {
		إذا (rec->id == id)
			أرجع rec;
	}

	أرجع عدم;
}

بنية bootstage_رکورد *ensure_id(بنية bootstage_داده *داده,
				   تعداد bootstage_id id)
{
	بنية bootstage_رکورد *rec;

	rec = پیدا_کن_id(داده, id);
	إذا (!rec && داده->rec_شمار < رکورد_شمار) {
		rec = &داده->رکورد[داده->rec_شمار++];
		rec->id = id;
		أرجع rec;
	}

	أرجع rec;
}

ulong bootstage_جمع_کن_رکورد(تعداد bootstage_id id, ثابت حرف *name,
			   صحيح flags, ulong mark)
{
	بنية bootstage_داده *داده = gd->bootstage;
	بنية bootstage_رکورد *rec;

	/*
	 * initf_bootstage() is called very early during boot but since hang()
	 * calls bootstage_error() we can be called before bootstage is set up.
	 * Add a check to avoid this.
	 */
	إذا (!داده)
		أرجع mark;
	إذا (flags & BOOTSTAGEF_ALLOC)
		id = داده->بعدی_id++;

	/* Only record the first event for each */
	rec = پیدا_کن_id(داده, id);
	إذا (!rec) {
		إذا (داده->rec_شمار < رکورد_شمار) {
			rec = &داده->رکورد[داده->rec_شمار++];
			rec->زمان_us = mark;
			rec->name = name;
			rec->flags = flags;
			rec->id = id;
		} وإلا {
			log_warning("Bootstage space exhausted\n");
		}
	}

	/* Tell the board about this progress */
	show_boot_پیشرفت(flags & BOOTSTAGEF_ERROR ? -id : id);

	أرجع mark;
}

ulong bootstage_error_name(تعداد bootstage_id id, ثابت حرف *name)
{
	أرجع bootstage_جمع_کن_رکورد(id, name, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(تعداد bootstage_id id, ثابت حرف *name)
{
	صحيح flags = 0;

	إذا (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;

	أرجع bootstage_جمع_کن_رکورد(id, name, flags, timer_get_boot_us());
}

ulong bootstage_mark_کد(ثابت حرف *file, ثابت حرف *func, صحيح linenum)
{
	حرف *str, *p;
	__maybe_unused حرف *انتها_2;
	صحيح len = 0;

	/* First work out the length we need to allocate */
	إذا (linenum != -1)
		len = 11;
	إذا (func)
		len += طول_رشته(func);
	إذا (file)
		len += طول_رشته(file);

	str = حافظه_بده(len + 1);
	p = str;
	انتها_2 = p + len;
	إذا (file)
		p += snprintf(p, انتها_2 - p, "%s,", file);
	إذا (linenum != -1)
		p += snprintf(p, انتها_2 - p, "%d", linenum);
	إذا (func)
		p += snprintf(p, انتها_2 - p, ": %s", func);

	أرجع bootstage_mark_name(BOOTSTAGE_ID_ALLOC, str);
}

uint32_t bootstage_شروع(تعداد bootstage_id id, ثابت حرف *name)
{
	بنية bootstage_داده *داده = gd->bootstage;
	بنية bootstage_رکورد *rec = ensure_id(داده, id);
	ulong شروع_us = timer_get_boot_us();

	إذا (rec) {
		rec->شروع_us = شروع_us;
		rec->name = name;
	}

	أرجع شروع_us;
}

uint32_t bootstage_accum(تعداد bootstage_id id)
{
	بنية bootstage_داده *داده = gd->bootstage;
	بنية bootstage_رکورد *rec = ensure_id(داده, id);
	uint32_t duration;

	إذا (!rec)
		أرجع 0;
	duration = (uint32_t)timer_get_boot_us() - rec->شروع_us;
	rec->زمان_us += duration;

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
ساكن ثابت حرف *get_رکورد_name(حرف *buf, صحيح len,
				   ثابت بنية bootstage_رکورد *rec)
{
	إذا (rec->name)
		أرجع rec->name;
	وإلا إذا (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	وإلا
		snprintf(buf, len, "id=%d", rec->id);

	أرجع buf;
}

ساكن uint32_t print_زمان_رکورد(بنية bootstage_رکورد *rec, uint32_t prev)
{
	حرف buf[20];

	إذا (prev == -1U) {
		چاپ_کن("%11s", "");
		print_grouped_ull(rec->زمان_us, BOOTSTAGE_DIGITS);
	} وإلا {
		إذا (prev > rec->زمان_us)
			prev = 0;
		print_grouped_ull(rec->زمان_us, BOOTSTAGE_DIGITS);
		print_grouped_ull(rec->زمان_us - prev, BOOTSTAGE_DIGITS);
	}
	چاپ_کن("  %s\n", get_رکورد_name(buf, حجم(buf), rec));

	أرجع rec->زمان_us;
}

#اگر_تعریف_شده CONFIG_از_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * Return: 0 on success, != 0 on failure.
 */
ساكن صحيح جمع_کن_bootstages_devicetree(بنية fdt_سرآیند *blob)
{
	بنية bootstage_داده *داده = gd->bootstage;
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
	bootstage = fdt_جمع_کن_subnode(blob, 0, "bootstage");
	إذا (bootstage < 0)
		أرجع -EINVAL;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	لكل (recnum = داده->rec_شمار - 1, i = 0; recnum >= 0; recnum--, i++) {
		بنية bootstage_رکورد *rec = &داده->رکورد[recnum];
		صحيح گره;

		إذا (rec->id != BOOTSTAGE_ID_AWAKE && rec->زمان_us == 0)
			استمر;

		گره = fdt_جمع_کن_subnode(blob, bootstage, simple_itoa(i));
		إذا (گره < 0)
			اكسر;

		/* add properties to the node. */
		إذا (fdt_setprop_string(blob, گره, "name",
				       get_رکورد_name(buf, حجم(buf), rec)))
			أرجع -EINVAL;

		/* Check if this is a 'mark' or 'accum' record */
		إذا (fdt_setprop_cell(blob, گره,
				rec->شروع_us ? "accum" : "mark",
				rec->زمان_us))
			أرجع -EINVAL;
	}

	أرجع 0;
}

صحيح bootstage_fdt_جمع_کن_report(فراغ)
{
	إذا (جمع_کن_bootstages_devicetree(working_fdt))
		puts("bootstage: Failed to add to device tree\n");

	أرجع 0;
}
#پایان_اگر

فراغ bootstage_report(فراغ)
{
	بنية bootstage_داده *داده = gd->bootstage;
	بنية bootstage_رکورد *rec = داده->رکورد;
	uint32_t prev;
	صحيح i;

	چاپ_کن("Timer summary in microseconds (%d records):\n",
	       داده->rec_شمار);
	چاپ_کن("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	prev = print_زمان_رکورد(rec, 0);

	لكل (i = 1, rec++; i < داده->rec_شمار; i++, rec++) {
		إذا (rec->id && !rec->شروع_us)
			prev = print_زمان_رکورد(rec, prev);
	}
	إذا (داده->rec_شمار > رکورد_شمار)
		چاپ_کن("Overflowed internal boot id table by %d entries\n"
		       "Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		       داده->rec_شمار - رکورد_شمار);

	puts("\nAccumulated time:\n");
	لكل (i = 0, rec = داده->رکورد; i < داده->rec_شمار; i++, rec++) {
		إذا (rec->شروع_us)
			prev = print_زمان_رکورد(rec, -1);
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
ساكن فراغ append_داده(حرف **ptrp, حرف *انتها_2, ثابت فراغ *داده, صحيح اندازه)
{
	حرف *ptr = *ptrp;

	*ptrp += اندازه;
	إذا (*ptrp > انتها_2)
		أرجع;

	کپی_حافظه(ptr, داده, اندازه);
}

صحيح bootstage_stash(فراغ *پایه, صحيح اندازه)
{
	ثابت بنية bootstage_داده *داده = gd->bootstage;
	بنية bootstage_hdr *hdr = (بنية bootstage_hdr *)پایه;
	ثابت بنية bootstage_رکورد *rec;
	حرف buf[20];
	حرف *ptr = پایه, *انتها_2 = ptr + اندازه;
	صحيح i;

	إذا (hdr + 1 > (بنية bootstage_hdr *)انتها_2) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		أرجع -ENOSPC;
	}

	/* Write an arbitrary version number */
	hdr->version = BOOTSTAGE_نسخه;

	hdr->شمار = داده->rec_شمار;
	hdr->اندازه = 0;
	hdr->magic = BOOTSTAGE_MAGIC;
	hdr->بعدی_id = داده->بعدی_id;
	ptr += حجم(*hdr);

	/* Write the records, silently stopping when we run out of space */
	لكل (rec = داده->رکورد, i = 0; i < داده->rec_شمار; i++, rec++)
		append_داده(&ptr, انتها_2, rec, حجم(*rec));

	/* Write the name strings */
	لكل (rec = داده->رکورد, i = 0; i < داده->rec_شمار; i++, rec++) {
		ثابت حرف *name;

		name = get_رکورد_name(buf, حجم(buf), rec);
		append_داده(&ptr, انتها_2, name, طول_رشته(name) + 1);
	}

	/* Check for buffer overflow */
	إذا (ptr > انتها_2) {
		debug("%s: Not enough space for bootstage stash\n", __func__);
		أرجع -ENOSPC;
	}

	/* Update total data size */
	hdr->اندازه = ptr - (حرف *)پایه;
	debug("Stashed %d records\n", hdr->شمار);

	أرجع 0;
}

صحيح bootstage_unstash(ثابت فراغ *پایه, صحيح اندازه)
{
	ثابت بنية bootstage_hdr *hdr = (بنية bootstage_hdr *)پایه;
	بنية bootstage_داده *داده = gd->bootstage;
	ثابت حرف *ptr = پایه, *انتها_2 = ptr + اندازه;
	بنية bootstage_رکورد *rec;
	uint rec_اندازه;
	صحيح i;

	إذا (اندازه == -1)
		انتها_2 = (حرف *)(~(uintptr_t)0);

	إذا (hdr + 1 > (بنية bootstage_hdr *)انتها_2) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		أرجع -EPERM;
	}

	إذا (hdr->magic != BOOTSTAGE_MAGIC) {
		debug("%s: Invalid bootstage magic\n", __func__);
		أرجع -ENOENT;
	}

	إذا (ptr + hdr->اندازه > انتها_2) {
		debug("%s: Bootstage data runs past buffer end\n", __func__);
		أرجع -ENOSPC;
	}

	إذا (hdr->شمار * حجم(*rec) > hdr->اندازه) {
		debug("%s: Bootstage has %d records needing %lu bytes, but "
			"only %d bytes is available\n", __func__, hdr->شمار,
		      (ulong)hdr->شمار * حجم(*rec), hdr->اندازه);
		أرجع -ENOSPC;
	}

	إذا (hdr->version != BOOTSTAGE_نسخه) {
		debug("%s: Bootstage data version %#0x unrecognised\n",
		      __func__, hdr->version);
		أرجع -EINVAL;
	}

	إذا (داده->rec_شمار + hdr->شمار > رکورد_شمار) {
		debug("%s: Bootstage has %d records, we have space for %d\n"
			"Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		      __func__, hdr->شمار, رکورد_شمار - داده->rec_شمار);
		أرجع -ENOSPC;
	}

	ptr += حجم(*hdr);

	/* Read the records */
	rec_اندازه = hdr->شمار * حجم(*داده->رکورد);
	کپی_حافظه(داده->رکورد + داده->rec_شمار, ptr, rec_اندازه);

	/* Read the name strings */
	ptr += rec_اندازه;
	لكل (rec = داده->رکورد + داده->rec_شمار, i = 0; i < hdr->شمار;
	     i++, rec++) {
		rec->name = ptr;
		إذا (xpl_phase() == PHASE_SPL)
			rec->name = strdup(ptr);

		/* Assume no data corruption here */
		ptr += طول_رشته(ptr) + 1;
	}

	/* Mark the records as read */
	داده->rec_شمار += hdr->شمار;
	داده->بعدی_id = hdr->بعدی_id;
	debug("Unstashed %d records\n", hdr->شمار);

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
#پایان_اگر

صحيح bootstage_get_اندازه(bool جمع_کن_رشتهها)
{
	صحيح اندازه;

	اندازه = حجم(بنية bootstage_داده);
	إذا (جمع_کن_رشتهها) {
		بنية bootstage_داده *داده = gd->bootstage;
		بنية bootstage_رکورد *rec;
		صحيح i;

		لكل (rec = داده->رکورد, i = 0; i < داده->rec_شمار; i++, rec++)
			إذا (rec->name)
				اندازه += طول_رشته(rec->name) + 1;
	}

	أرجع اندازه;
}

صحيح bootstage_init(bool اولی)
{
	بنية bootstage_داده *داده;
	صحيح اندازه = حجم(بنية bootstage_داده);

	gd->bootstage = (بنية bootstage_داده *)حافظه_بده(اندازه);
	إذا (!gd->bootstage)
		أرجع -ENOMEM;
	داده = gd->bootstage;
	memset(داده, '\0', اندازه);
	إذا (اولی) {
		داده->بعدی_id = BOOTSTAGE_ID_USER;
		bootstage_جمع_کن_رکورد(BOOTSTAGE_ID_AWAKE, "reset", 0, 0);
	}

	أرجع 0;
}
