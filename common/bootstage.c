// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 */

/*
 * This module records the progress of boot and arbitrary commands, and
 * permits accurate timestamping of each.
 */

#وصف_ڪريو LOG_CATEGORY	LOGC_BOOT

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
	رڪارڊ_ڳڻپ = CONFIG_VAL(BOOTSTAGE_رڪارڊ_ڳڻپ),
};

بنية bootstage_رڪارڊ {
	ulong وقت_us;
	uint32_t شروعات_us;
	ثابت حرف *name;
	صحيح flags;		/* see enum bootstage_flags */
	تعداد bootstage_id id;
};

بنية bootstage_معلومات {
	uint rec_ڳڻپ;
	uint ايندڙ_id;
	بنية bootstage_رڪارڊ رڪارڊ[رڪارڊ_ڳڻپ];
};

تعداد {
	BOOTSTAGE_نسخو	= 0,
	BOOTSTAGE_MAGIC		= 0xb00757a3,
	BOOTSTAGE_DIGITS	= 9,
};

بنية bootstage_hdr {
	u32 version;		/* BOOTSTAGE_VERSION */
	u32 ڳڻپ;		/* Number of records */
	u32 سائيز;		/* Total data size (non-zero if valid) */
	u32 magic;		/* Magic number */
	u32 ايندڙ_id;		/* Next ID to use for bootstage */
};

صحيح bootstage_relocate(فراغ *منزل)
{
	بنية bootstage_معلومات *معلومات;
	صحيح i;
	حرف *ptr;

	debug("Copying bootstage from %p to %p\n", gd->bootstage, منزل);
	يادگيري_نقل(منزل, gd->bootstage, حجم(بنية bootstage_معلومات));
	معلومات = gd->bootstage = منزل;

	/* Figure out where to relocate the strings to */
	ptr = (حرف *)(معلومات + 1);

	/*
	 * Duplicate all strings.  They may point to an old location in the
	 * program .text section that can eventually get trashed.
	 */
	debug("Relocating %d records\n", معلومات->rec_ڳڻپ);
	لكل (i = 0; i < معلومات->rec_ڳڻپ; i++) {
		ثابت حرف *from = معلومات->رڪارڊ[i].name;

		إذا (!from)
			استمر;
		strcpy(ptr, from);
		معلومات->رڪارڊ[i].name = ptr;
		ptr += لڙي_ڊيگهه(ptr) + 1;
	}

	أرجع 0;
}

بنية bootstage_رڪارڊ *ڳوليو_id(بنية bootstage_معلومات *معلومات,
				 تعداد bootstage_id id)
{
	بنية bootstage_رڪارڊ *rec;
	بنية bootstage_رڪارڊ *پڇاڙي_2;

	لكل (rec = معلومات->رڪارڊ, پڇاڙي_2 = rec + معلومات->rec_ڳڻپ; rec < پڇاڙي_2;
	     rec++) {
		إذا (rec->id == id)
			أرجع rec;
	}

	أرجع عدم;
}

بنية bootstage_رڪارڊ *ensure_id(بنية bootstage_معلومات *معلومات,
				   تعداد bootstage_id id)
{
	بنية bootstage_رڪارڊ *rec;

	rec = ڳوليو_id(معلومات, id);
	إذا (!rec && معلومات->rec_ڳڻپ < رڪارڊ_ڳڻپ) {
		rec = &معلومات->رڪارڊ[معلومات->rec_ڳڻپ++];
		rec->id = id;
		أرجع rec;
	}

	أرجع rec;
}

ulong bootstage_جوڙيو_رڪارڊ(تعداد bootstage_id id, ثابت حرف *name,
			   صحيح flags, ulong mark)
{
	بنية bootstage_معلومات *معلومات = gd->bootstage;
	بنية bootstage_رڪارڊ *rec;

	/*
	 * initf_bootstage() is called very early during boot but since hang()
	 * calls bootstage_error() we can be called before bootstage is set up.
	 * Add a check to avoid this.
	 */
	إذا (!معلومات)
		أرجع mark;
	إذا (flags & BOOTSTAGEF_ALLOC)
		id = معلومات->ايندڙ_id++;

	/* Only record the first event for each */
	rec = ڳوليو_id(معلومات, id);
	إذا (!rec) {
		إذا (معلومات->rec_ڳڻپ < رڪارڊ_ڳڻپ) {
			rec = &معلومات->رڪارڊ[معلومات->rec_ڳڻپ++];
			rec->وقت_us = mark;
			rec->name = name;
			rec->flags = flags;
			rec->id = id;
		} وإلا {
			log_warning("Bootstage space exhausted\n");
		}
	}

	/* Tell the board about this progress */
	show_boot_ترقي(flags & BOOTSTAGEF_ERROR ? -id : id);

	أرجع mark;
}

ulong bootstage_error_name(تعداد bootstage_id id, ثابت حرف *name)
{
	أرجع bootstage_جوڙيو_رڪارڊ(id, name, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(تعداد bootstage_id id, ثابت حرف *name)
{
	صحيح flags = 0;

	إذا (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;

	أرجع bootstage_جوڙيو_رڪارڊ(id, name, flags, timer_get_boot_us());
}

ulong bootstage_mark_ڪوڊ(ثابت حرف *file, ثابت حرف *func, صحيح linenum)
{
	حرف *str, *p;
	__maybe_unused حرف *پڇاڙي_2;
	صحيح len = 0;

	/* First work out the length we need to allocate */
	إذا (linenum != -1)
		len = 11;
	إذا (func)
		len += لڙي_ڊيگهه(func);
	إذا (file)
		len += لڙي_ڊيگهه(file);

	str = يادگيري_ڏيو(len + 1);
	p = str;
	پڇاڙي_2 = p + len;
	إذا (file)
		p += snprintf(p, پڇاڙي_2 - p, "%s,", file);
	إذا (linenum != -1)
		p += snprintf(p, پڇاڙي_2 - p, "%d", linenum);
	إذا (func)
		p += snprintf(p, پڇاڙي_2 - p, ": %s", func);

	أرجع bootstage_mark_name(BOOTSTAGE_ID_ALLOC, str);
}

uint32_t bootstage_شروعات(تعداد bootstage_id id, ثابت حرف *name)
{
	بنية bootstage_معلومات *معلومات = gd->bootstage;
	بنية bootstage_رڪارڊ *rec = ensure_id(معلومات, id);
	ulong شروعات_us = timer_get_boot_us();

	إذا (rec) {
		rec->شروعات_us = شروعات_us;
		rec->name = name;
	}

	أرجع شروعات_us;
}

uint32_t bootstage_accum(تعداد bootstage_id id)
{
	بنية bootstage_معلومات *معلومات = gd->bootstage;
	بنية bootstage_رڪارڊ *rec = ensure_id(معلومات, id);
	uint32_t duration;

	إذا (!rec)
		أرجع 0;
	duration = (uint32_t)timer_get_boot_us() - rec->شروعات_us;
	rec->وقت_us += duration;

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
ساكن ثابت حرف *get_رڪارڊ_name(حرف *buf, صحيح len,
				   ثابت بنية bootstage_رڪارڊ *rec)
{
	إذا (rec->name)
		أرجع rec->name;
	وإلا إذا (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	وإلا
		snprintf(buf, len, "id=%d", rec->id);

	أرجع buf;
}

ساكن uint32_t print_وقت_رڪارڊ(بنية bootstage_رڪارڊ *rec, uint32_t prev)
{
	حرف buf[20];

	إذا (prev == -1U) {
		لکو("%11s", "");
		print_grouped_ull(rec->وقت_us, BOOTSTAGE_DIGITS);
	} وإلا {
		إذا (prev > rec->وقت_us)
			prev = 0;
		print_grouped_ull(rec->وقت_us, BOOTSTAGE_DIGITS);
		print_grouped_ull(rec->وقت_us - prev, BOOTSTAGE_DIGITS);
	}
	لکو("  %s\n", get_رڪارڊ_name(buf, حجم(buf), rec));

	أرجع rec->وقت_us;
}

#جيڪڏهن_وصف_ٿيل CONFIG_جو_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * Return: 0 on success, != 0 on failure.
 */
ساكن صحيح جوڙيو_bootstages_devicetree(بنية fdt_سرو *blob)
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
	bootstage = fdt_جوڙيو_subnode(blob, 0, "bootstage");
	إذا (bootstage < 0)
		أرجع -EINVAL;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	لكل (recnum = معلومات->rec_ڳڻپ - 1, i = 0; recnum >= 0; recnum--, i++) {
		بنية bootstage_رڪارڊ *rec = &معلومات->رڪارڊ[recnum];
		صحيح ڳنڍ;

		إذا (rec->id != BOOTSTAGE_ID_AWAKE && rec->وقت_us == 0)
			استمر;

		ڳنڍ = fdt_جوڙيو_subnode(blob, bootstage, simple_itoa(i));
		إذا (ڳنڍ < 0)
			اكسر;

		/* add properties to the node. */
		إذا (fdt_setprop_string(blob, ڳنڍ, "name",
				       get_رڪارڊ_name(buf, حجم(buf), rec)))
			أرجع -EINVAL;

		/* Check if this is a 'mark' or 'accum' record */
		إذا (fdt_setprop_cell(blob, ڳنڍ,
				rec->شروعات_us ? "accum" : "mark",
				rec->وقت_us))
			أرجع -EINVAL;
	}

	أرجع 0;
}

صحيح bootstage_fdt_جوڙيو_report(فراغ)
{
	إذا (جوڙيو_bootstages_devicetree(working_fdt))
		puts("bootstage: Failed to add to device tree\n");

	أرجع 0;
}
#جيڪڏهن_پورو

فراغ bootstage_report(فراغ)
{
	بنية bootstage_معلومات *معلومات = gd->bootstage;
	بنية bootstage_رڪارڊ *rec = معلومات->رڪارڊ;
	uint32_t prev;
	صحيح i;

	لکو("Timer summary in microseconds (%d records):\n",
	       معلومات->rec_ڳڻپ);
	لکو("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	prev = print_وقت_رڪارڊ(rec, 0);

	لكل (i = 1, rec++; i < معلومات->rec_ڳڻپ; i++, rec++) {
		إذا (rec->id && !rec->شروعات_us)
			prev = print_وقت_رڪارڊ(rec, prev);
	}
	إذا (معلومات->rec_ڳڻپ > رڪارڊ_ڳڻپ)
		لکو("Overflowed internal boot id table by %d entries\n"
		       "Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		       معلومات->rec_ڳڻپ - رڪارڊ_ڳڻپ);

	puts("\nAccumulated time:\n");
	لكل (i = 0, rec = معلومات->رڪارڊ; i < معلومات->rec_ڳڻپ; i++, rec++) {
		إذا (rec->شروعات_us)
			prev = print_وقت_رڪارڊ(rec, -1);
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
ساكن فراغ append_معلومات(حرف **ptrp, حرف *پڇاڙي_2, ثابت فراغ *معلومات, صحيح سائيز)
{
	حرف *ptr = *ptrp;

	*ptrp += سائيز;
	إذا (*ptrp > پڇاڙي_2)
		أرجع;

	يادگيري_نقل(ptr, معلومات, سائيز);
}

صحيح bootstage_stash(فراغ *بنياد, صحيح سائيز)
{
	ثابت بنية bootstage_معلومات *معلومات = gd->bootstage;
	بنية bootstage_hdr *hdr = (بنية bootstage_hdr *)بنياد;
	ثابت بنية bootstage_رڪارڊ *rec;
	حرف buf[20];
	حرف *ptr = بنياد, *پڇاڙي_2 = ptr + سائيز;
	صحيح i;

	إذا (hdr + 1 > (بنية bootstage_hdr *)پڇاڙي_2) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		أرجع -ENOSPC;
	}

	/* Write an arbitrary version number */
	hdr->version = BOOTSTAGE_نسخو;

	hdr->ڳڻپ = معلومات->rec_ڳڻپ;
	hdr->سائيز = 0;
	hdr->magic = BOOTSTAGE_MAGIC;
	hdr->ايندڙ_id = معلومات->ايندڙ_id;
	ptr += حجم(*hdr);

	/* Write the records, silently stopping when we run out of space */
	لكل (rec = معلومات->رڪارڊ, i = 0; i < معلومات->rec_ڳڻپ; i++, rec++)
		append_معلومات(&ptr, پڇاڙي_2, rec, حجم(*rec));

	/* Write the name strings */
	لكل (rec = معلومات->رڪارڊ, i = 0; i < معلومات->rec_ڳڻپ; i++, rec++) {
		ثابت حرف *name;

		name = get_رڪارڊ_name(buf, حجم(buf), rec);
		append_معلومات(&ptr, پڇاڙي_2, name, لڙي_ڊيگهه(name) + 1);
	}

	/* Check for buffer overflow */
	إذا (ptr > پڇاڙي_2) {
		debug("%s: Not enough space for bootstage stash\n", __func__);
		أرجع -ENOSPC;
	}

	/* Update total data size */
	hdr->سائيز = ptr - (حرف *)بنياد;
	debug("Stashed %d records\n", hdr->ڳڻپ);

	أرجع 0;
}

صحيح bootstage_unstash(ثابت فراغ *بنياد, صحيح سائيز)
{
	ثابت بنية bootstage_hdr *hdr = (بنية bootstage_hdr *)بنياد;
	بنية bootstage_معلومات *معلومات = gd->bootstage;
	ثابت حرف *ptr = بنياد, *پڇاڙي_2 = ptr + سائيز;
	بنية bootstage_رڪارڊ *rec;
	uint rec_سائيز;
	صحيح i;

	إذا (سائيز == -1)
		پڇاڙي_2 = (حرف *)(~(uintptr_t)0);

	إذا (hdr + 1 > (بنية bootstage_hdr *)پڇاڙي_2) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		أرجع -EPERM;
	}

	إذا (hdr->magic != BOOTSTAGE_MAGIC) {
		debug("%s: Invalid bootstage magic\n", __func__);
		أرجع -ENOENT;
	}

	إذا (ptr + hdr->سائيز > پڇاڙي_2) {
		debug("%s: Bootstage data runs past buffer end\n", __func__);
		أرجع -ENOSPC;
	}

	إذا (hdr->ڳڻپ * حجم(*rec) > hdr->سائيز) {
		debug("%s: Bootstage has %d records needing %lu bytes, but "
			"only %d bytes is available\n", __func__, hdr->ڳڻپ,
		      (ulong)hdr->ڳڻپ * حجم(*rec), hdr->سائيز);
		أرجع -ENOSPC;
	}

	إذا (hdr->version != BOOTSTAGE_نسخو) {
		debug("%s: Bootstage data version %#0x unrecognised\n",
		      __func__, hdr->version);
		أرجع -EINVAL;
	}

	إذا (معلومات->rec_ڳڻپ + hdr->ڳڻپ > رڪارڊ_ڳڻپ) {
		debug("%s: Bootstage has %d records, we have space for %d\n"
			"Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		      __func__, hdr->ڳڻپ, رڪارڊ_ڳڻپ - معلومات->rec_ڳڻپ);
		أرجع -ENOSPC;
	}

	ptr += حجم(*hdr);

	/* Read the records */
	rec_سائيز = hdr->ڳڻپ * حجم(*معلومات->رڪارڊ);
	يادگيري_نقل(معلومات->رڪارڊ + معلومات->rec_ڳڻپ, ptr, rec_سائيز);

	/* Read the name strings */
	ptr += rec_سائيز;
	لكل (rec = معلومات->رڪارڊ + معلومات->rec_ڳڻپ, i = 0; i < hdr->ڳڻپ;
	     i++, rec++) {
		rec->name = ptr;
		إذا (xpl_phase() == PHASE_SPL)
			rec->name = strdup(ptr);

		/* Assume no data corruption here */
		ptr += لڙي_ڊيگهه(ptr) + 1;
	}

	/* Mark the records as read */
	معلومات->rec_ڳڻپ += hdr->ڳڻپ;
	معلومات->ايندڙ_id = hdr->ايندڙ_id;
	debug("Unstashed %d records\n", hdr->ڳڻپ);

	أرجع 0;
}

#إذا IS_ENABLED(CONFIG_BOOTSTAGE_STASH)
صحيح _bootstage_stash_default(فراغ)
{
	أرجع bootstage_stash(map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR, 0),
			       CONFIG_BOOTSTAGE_STASH_سائيز);
}

صحيح _bootstage_unstash_default(فراغ)
{
	ثابت فراغ *stash = map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR,
				       CONFIG_BOOTSTAGE_STASH_سائيز);

	أرجع bootstage_unstash(stash, CONFIG_BOOTSTAGE_STASH_سائيز);
}
#جيڪڏهن_پورو

صحيح bootstage_get_سائيز(bool جوڙيو_لڙيون)
{
	صحيح سائيز;

	سائيز = حجم(بنية bootstage_معلومات);
	إذا (جوڙيو_لڙيون) {
		بنية bootstage_معلومات *معلومات = gd->bootstage;
		بنية bootstage_رڪارڊ *rec;
		صحيح i;

		لكل (rec = معلومات->رڪارڊ, i = 0; i < معلومات->rec_ڳڻپ; i++, rec++)
			إذا (rec->name)
				سائيز += لڙي_ڊيگهه(rec->name) + 1;
	}

	أرجع سائيز;
}

صحيح bootstage_init(bool پهريون)
{
	بنية bootstage_معلومات *معلومات;
	صحيح سائيز = حجم(بنية bootstage_معلومات);

	gd->bootstage = (بنية bootstage_معلومات *)يادگيري_ڏيو(سائيز);
	إذا (!gd->bootstage)
		أرجع -ENOMEM;
	معلومات = gd->bootstage;
	memset(معلومات, '\0', سائيز);
	إذا (پهريون) {
		معلومات->ايندڙ_id = BOOTSTAGE_ID_USER;
		bootstage_جوڙيو_رڪارڊ(BOOTSTAGE_ID_AWAKE, "reset", 0, 0);
	}

	أرجع 0;
}
