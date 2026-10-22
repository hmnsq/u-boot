// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 */

/*
 * This module records the progress of boot and arbitrary commands, and
 * permits accurate timestamping of each.
 */

#నిర్వచించు LOG_CATEGORY	LOGC_BOOT

#అన్తర్భావ <bootstage.h>
#అన్తర్భావ <hang.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <malloc.h>
#అన్తర్భావ <sort.h>
#అన్తర్భావ <spl.h>
#అన్తర్భావ <asm/global_data.h>
#అన్తర్భావ <linux/compiler.h>
#అన్తర్భావ <linux/libfdt.h>

DECLARE_GLOBAL_దత్తాంశం_PTR;

గణన {
	రికార్డు_లెక్క = CONFIG_VAL(BOOTSTAGE_రికార్డు_లెక్క),
};

నిర్మాణం bootstage_రికార్డు {
	ulong సమయం_us;
	uint32_t ప్రారంభం_us;
	స్థిరరాశి అక్షరం *name;
	పూర్ణాంకం flags;		/* see enum bootstage_flags */
	గణన bootstage_id id;
};

నిర్మాణం bootstage_దత్తాంశం {
	uint rec_లెక్క;
	uint తదుపరి_id;
	నిర్మాణం bootstage_రికార్డు రికార్డు[రికార్డు_లెక్క];
};

గణన {
	BOOTSTAGE_సంస్కరణ	= 0,
	BOOTSTAGE_MAGIC		= 0xb00757a3,
	BOOTSTAGE_DIGITS	= 9,
};

నిర్మాణం bootstage_hdr {
	u32 version;		/* BOOTSTAGE_VERSION */
	u32 లెక్క;		/* Number of records */
	u32 కొలత;		/* Total data size (non-zero if valid) */
	u32 magic;		/* Magic number */
	u32 తదుపరి_id;		/* Next ID to use for bootstage */
};

పూర్ణాంకం bootstage_relocate(శూన్య *గమ్యం)
{
	నిర్మాణం bootstage_దత్తాంశం *దత్తాంశం;
	పూర్ణాంకం i;
	అక్షరం *ptr;

	debug("Copying bootstage from %p to %p\n", gd->bootstage, గమ్యం);
	జ్ఞాపకం_నకలు(గమ్యం, gd->bootstage, పరిమాణం(నిర్మాణం bootstage_దత్తాంశం));
	దత్తాంశం = gd->bootstage = గమ్యం;

	/* Figure out where to relocate the strings to */
	ptr = (అక్షరం *)(దత్తాంశం + 1);

	/*
	 * Duplicate all strings.  They may point to an old location in the
	 * program .text section that can eventually get trashed.
	 */
	debug("Relocating %d records\n", దత్తాంశం->rec_లెక్క);
	కొరకు (i = 0; i < దత్తాంశం->rec_లెక్క; i++) {
		స్థిరరాశి అక్షరం *from = దత్తాంశం->రికార్డు[i].name;

		ఒకవేళ (!from)
			కొనసాగు;
		strcpy(ptr, from);
		దత్తాంశం->రికార్డు[i].name = ptr;
		ptr += అక్షరశ్రేణి_పొడవు(ptr) + 1;
	}

	తిరిగి 0;
}

నిర్మాణం bootstage_రికార్డు *కనుగొను_id(నిర్మాణం bootstage_దత్తాంశం *దత్తాంశం,
				 గణన bootstage_id id)
{
	నిర్మాణం bootstage_రికార్డు *rec;
	నిర్మాణం bootstage_రికార్డు *చివర_2;

	కొరకు (rec = దత్తాంశం->రికార్డు, చివర_2 = rec + దత్తాంశం->rec_లెక్క; rec < చివర_2;
	     rec++) {
		ఒకవేళ (rec->id == id)
			తిరిగి rec;
	}

	తిరిగి ఖాళీ;
}

నిర్మాణం bootstage_రికార్డు *ensure_id(నిర్మాణం bootstage_దత్తాంశం *దత్తాంశం,
				   గణన bootstage_id id)
{
	నిర్మాణం bootstage_రికార్డు *rec;

	rec = కనుగొను_id(దత్తాంశం, id);
	ఒకవేళ (!rec && దత్తాంశం->rec_లెక్క < రికార్డు_లెక్క) {
		rec = &దత్తాంశం->రికార్డు[దత్తాంశం->rec_లెక్క++];
		rec->id = id;
		తిరిగి rec;
	}

	తిరిగి rec;
}

ulong bootstage_కూడు_రికార్డు(గణన bootstage_id id, స్థిరరాశి అక్షరం *name,
			   పూర్ణాంకం flags, ulong mark)
{
	నిర్మాణం bootstage_దత్తాంశం *దత్తాంశం = gd->bootstage;
	నిర్మాణం bootstage_రికార్డు *rec;

	/*
	 * initf_bootstage() is called very early during boot but since hang()
	 * calls bootstage_error() we can be called before bootstage is set up.
	 * Add a check to avoid this.
	 */
	ఒకవేళ (!దత్తాంశం)
		తిరిగి mark;
	ఒకవేళ (flags & BOOTSTAGEF_ALLOC)
		id = దత్తాంశం->తదుపరి_id++;

	/* Only record the first event for each */
	rec = కనుగొను_id(దత్తాంశం, id);
	ఒకవేళ (!rec) {
		ఒకవేళ (దత్తాంశం->rec_లెక్క < రికార్డు_లెక్క) {
			rec = &దత్తాంశం->రికార్డు[దత్తాంశం->rec_లెక్క++];
			rec->సమయం_us = mark;
			rec->name = name;
			rec->flags = flags;
			rec->id = id;
		} లేకపోతే {
			log_warning("Bootstage space exhausted\n");
		}
	}

	/* Tell the board about this progress */
	show_boot_పురోగతి(flags & BOOTSTAGEF_ERROR ? -id : id);

	తిరిగి mark;
}

ulong bootstage_error_name(గణన bootstage_id id, స్థిరరాశి అక్షరం *name)
{
	తిరిగి bootstage_కూడు_రికార్డు(id, name, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(గణన bootstage_id id, స్థిరరాశి అక్షరం *name)
{
	పూర్ణాంకం flags = 0;

	ఒకవేళ (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;

	తిరిగి bootstage_కూడు_రికార్డు(id, name, flags, timer_get_boot_us());
}

ulong bootstage_mark_సంకేతం(స్థిరరాశి అక్షరం *file, స్థిరరాశి అక్షరం *func, పూర్ణాంకం linenum)
{
	అక్షరం *str, *p;
	__maybe_unused అక్షరం *చివర_2;
	పూర్ణాంకం len = 0;

	/* First work out the length we need to allocate */
	ఒకవేళ (linenum != -1)
		len = 11;
	ఒకవేళ (func)
		len += అక్షరశ్రేణి_పొడవు(func);
	ఒకవేళ (file)
		len += అక్షరశ్రేణి_పొడవు(file);

	str = జ్ఞాపకం_ఇవ్వు(len + 1);
	p = str;
	చివర_2 = p + len;
	ఒకవేళ (file)
		p += snprintf(p, చివర_2 - p, "%s,", file);
	ఒకవేళ (linenum != -1)
		p += snprintf(p, చివర_2 - p, "%d", linenum);
	ఒకవేళ (func)
		p += snprintf(p, చివర_2 - p, ": %s", func);

	తిరిగి bootstage_mark_name(BOOTSTAGE_ID_ALLOC, str);
}

uint32_t bootstage_ప్రారంభం(గణన bootstage_id id, స్థిరరాశి అక్షరం *name)
{
	నిర్మాణం bootstage_దత్తాంశం *దత్తాంశం = gd->bootstage;
	నిర్మాణం bootstage_రికార్డు *rec = ensure_id(దత్తాంశం, id);
	ulong ప్రారంభం_us = timer_get_boot_us();

	ఒకవేళ (rec) {
		rec->ప్రారంభం_us = ప్రారంభం_us;
		rec->name = name;
	}

	తిరిగి ప్రారంభం_us;
}

uint32_t bootstage_accum(గణన bootstage_id id)
{
	నిర్మాణం bootstage_దత్తాంశం *దత్తాంశం = gd->bootstage;
	నిర్మాణం bootstage_రికార్డు *rec = ensure_id(దత్తాంశం, id);
	uint32_t duration;

	ఒకవేళ (!rec)
		తిరిగి 0;
	duration = (uint32_t)timer_get_boot_us() - rec->ప్రారంభం_us;
	rec->సమయం_us += duration;

	తిరిగి duration;
}

/**
 * Get a record name as a printable string
 *
 * @param buf	Buffer to put name if needed
 * @param len	Length of buffer
 * @param rec	Boot stage record to get the name from
 * Return: pointer to name, either from the record or pointing to buf.
 */
స్థిర స్థిరరాశి అక్షరం *get_రికార్డు_name(అక్షరం *buf, పూర్ణాంకం len,
				   స్థిరరాశి నిర్మాణం bootstage_రికార్డు *rec)
{
	ఒకవేళ (rec->name)
		తిరిగి rec->name;
	లేకపోతే ఒకవేళ (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	లేకపోతే
		snprintf(buf, len, "id=%d", rec->id);

	తిరిగి buf;
}

స్థిర uint32_t print_సమయం_రికార్డు(నిర్మాణం bootstage_రికార్డు *rec, uint32_t prev)
{
	అక్షరం buf[20];

	ఒకవేళ (prev == -1U) {
		రాయి("%11s", "");
		print_grouped_ull(rec->సమయం_us, BOOTSTAGE_DIGITS);
	} లేకపోతే {
		ఒకవేళ (prev > rec->సమయం_us)
			prev = 0;
		print_grouped_ull(rec->సమయం_us, BOOTSTAGE_DIGITS);
		print_grouped_ull(rec->సమయం_us - prev, BOOTSTAGE_DIGITS);
	}
	రాయి("  %s\n", get_రికార్డు_name(buf, పరిమాణం(buf), rec));

	తిరిగి rec->సమయం_us;
}

#నిర్వచితమైతే CONFIG_యొక్క_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * Return: 0 on success, != 0 on failure.
 */
స్థిర పూర్ణాంకం కూడు_bootstages_devicetree(నిర్మాణం fdt_శీర్షిక *blob)
{
	నిర్మాణం bootstage_దత్తాంశం *దత్తాంశం = gd->bootstage;
	పూర్ణాంకం bootstage;
	అక్షరం buf[20];
	పూర్ణాంకం recnum;
	పూర్ణాంకం i;

	ఒకవేళ (!blob)
		తిరిగి 0;

	/*
	 * Create the node for bootstage.
	 * The address of flat device tree is set up by the command bootm.
	 */
	bootstage = fdt_కూడు_subnode(blob, 0, "bootstage");
	ఒకవేళ (bootstage < 0)
		తిరిగి -EINVAL;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	కొరకు (recnum = దత్తాంశం->rec_లెక్క - 1, i = 0; recnum >= 0; recnum--, i++) {
		నిర్మాణం bootstage_రికార్డు *rec = &దత్తాంశం->రికార్డు[recnum];
		పూర్ణాంకం కణుపు;

		ఒకవేళ (rec->id != BOOTSTAGE_ID_AWAKE && rec->సమయం_us == 0)
			కొనసాగు;

		కణుపు = fdt_కూడు_subnode(blob, bootstage, simple_itoa(i));
		ఒకవేళ (కణుపు < 0)
			ఆపు;

		/* add properties to the node. */
		ఒకవేళ (fdt_setprop_string(blob, కణుపు, "name",
				       get_రికార్డు_name(buf, పరిమాణం(buf), rec)))
			తిరిగి -EINVAL;

		/* Check if this is a 'mark' or 'accum' record */
		ఒకవేళ (fdt_setprop_cell(blob, కణుపు,
				rec->ప్రారంభం_us ? "accum" : "mark",
				rec->సమయం_us))
			తిరిగి -EINVAL;
	}

	తిరిగి 0;
}

పూర్ణాంకం bootstage_fdt_కూడు_report(శూన్య)
{
	ఒకవేళ (కూడు_bootstages_devicetree(working_fdt))
		puts("bootstage: Failed to add to device tree\n");

	తిరిగి 0;
}
#అయితే_ముగింపు

శూన్య bootstage_report(శూన్య)
{
	నిర్మాణం bootstage_దత్తాంశం *దత్తాంశం = gd->bootstage;
	నిర్మాణం bootstage_రికార్డు *rec = దత్తాంశం->రికార్డు;
	uint32_t prev;
	పూర్ణాంకం i;

	రాయి("Timer summary in microseconds (%d records):\n",
	       దత్తాంశం->rec_లెక్క);
	రాయి("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	prev = print_సమయం_రికార్డు(rec, 0);

	కొరకు (i = 1, rec++; i < దత్తాంశం->rec_లెక్క; i++, rec++) {
		ఒకవేళ (rec->id && !rec->ప్రారంభం_us)
			prev = print_సమయం_రికార్డు(rec, prev);
	}
	ఒకవేళ (దత్తాంశం->rec_లెక్క > రికార్డు_లెక్క)
		రాయి("Overflowed internal boot id table by %d entries\n"
		       "Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		       దత్తాంశం->rec_లెక్క - రికార్డు_లెక్క);

	puts("\nAccumulated time:\n");
	కొరకు (i = 0, rec = దత్తాంశం->రికార్డు; i < దత్తాంశం->rec_లెక్క; i++, rec++) {
		ఒకవేళ (rec->ప్రారంభం_us)
			prev = print_సమయం_రికార్డు(rec, -1);
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
స్థిర శూన్య append_దత్తాంశం(అక్షరం **ptrp, అక్షరం *చివర_2, స్థిరరాశి శూన్య *దత్తాంశం, పూర్ణాంకం కొలత)
{
	అక్షరం *ptr = *ptrp;

	*ptrp += కొలత;
	ఒకవేళ (*ptrp > చివర_2)
		తిరిగి;

	జ్ఞాపకం_నకలు(ptr, దత్తాంశం, కొలత);
}

పూర్ణాంకం bootstage_stash(శూన్య *పునాది, పూర్ణాంకం కొలత)
{
	స్థిరరాశి నిర్మాణం bootstage_దత్తాంశం *దత్తాంశం = gd->bootstage;
	నిర్మాణం bootstage_hdr *hdr = (నిర్మాణం bootstage_hdr *)పునాది;
	స్థిరరాశి నిర్మాణం bootstage_రికార్డు *rec;
	అక్షరం buf[20];
	అక్షరం *ptr = పునాది, *చివర_2 = ptr + కొలత;
	పూర్ణాంకం i;

	ఒకవేళ (hdr + 1 > (నిర్మాణం bootstage_hdr *)చివర_2) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		తిరిగి -ENOSPC;
	}

	/* Write an arbitrary version number */
	hdr->version = BOOTSTAGE_సంస్కరణ;

	hdr->లెక్క = దత్తాంశం->rec_లెక్క;
	hdr->కొలత = 0;
	hdr->magic = BOOTSTAGE_MAGIC;
	hdr->తదుపరి_id = దత్తాంశం->తదుపరి_id;
	ptr += పరిమాణం(*hdr);

	/* Write the records, silently stopping when we run out of space */
	కొరకు (rec = దత్తాంశం->రికార్డు, i = 0; i < దత్తాంశం->rec_లెక్క; i++, rec++)
		append_దత్తాంశం(&ptr, చివర_2, rec, పరిమాణం(*rec));

	/* Write the name strings */
	కొరకు (rec = దత్తాంశం->రికార్డు, i = 0; i < దత్తాంశం->rec_లెక్క; i++, rec++) {
		స్థిరరాశి అక్షరం *name;

		name = get_రికార్డు_name(buf, పరిమాణం(buf), rec);
		append_దత్తాంశం(&ptr, చివర_2, name, అక్షరశ్రేణి_పొడవు(name) + 1);
	}

	/* Check for buffer overflow */
	ఒకవేళ (ptr > చివర_2) {
		debug("%s: Not enough space for bootstage stash\n", __func__);
		తిరిగి -ENOSPC;
	}

	/* Update total data size */
	hdr->కొలత = ptr - (అక్షరం *)పునాది;
	debug("Stashed %d records\n", hdr->లెక్క);

	తిరిగి 0;
}

పూర్ణాంకం bootstage_unstash(స్థిరరాశి శూన్య *పునాది, పూర్ణాంకం కొలత)
{
	స్థిరరాశి నిర్మాణం bootstage_hdr *hdr = (నిర్మాణం bootstage_hdr *)పునాది;
	నిర్మాణం bootstage_దత్తాంశం *దత్తాంశం = gd->bootstage;
	స్థిరరాశి అక్షరం *ptr = పునాది, *చివర_2 = ptr + కొలత;
	నిర్మాణం bootstage_రికార్డు *rec;
	uint rec_కొలత;
	పూర్ణాంకం i;

	ఒకవేళ (కొలత == -1)
		చివర_2 = (అక్షరం *)(~(uintptr_t)0);

	ఒకవేళ (hdr + 1 > (నిర్మాణం bootstage_hdr *)చివర_2) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		తిరిగి -EPERM;
	}

	ఒకవేళ (hdr->magic != BOOTSTAGE_MAGIC) {
		debug("%s: Invalid bootstage magic\n", __func__);
		తిరిగి -ENOENT;
	}

	ఒకవేళ (ptr + hdr->కొలత > చివర_2) {
		debug("%s: Bootstage data runs past buffer end\n", __func__);
		తిరిగి -ENOSPC;
	}

	ఒకవేళ (hdr->లెక్క * పరిమాణం(*rec) > hdr->కొలత) {
		debug("%s: Bootstage has %d records needing %lu bytes, but "
			"only %d bytes is available\n", __func__, hdr->లెక్క,
		      (ulong)hdr->లెక్క * పరిమాణం(*rec), hdr->కొలత);
		తిరిగి -ENOSPC;
	}

	ఒకవేళ (hdr->version != BOOTSTAGE_సంస్కరణ) {
		debug("%s: Bootstage data version %#0x unrecognised\n",
		      __func__, hdr->version);
		తిరిగి -EINVAL;
	}

	ఒకవేళ (దత్తాంశం->rec_లెక్క + hdr->లెక్క > రికార్డు_లెక్క) {
		debug("%s: Bootstage has %d records, we have space for %d\n"
			"Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		      __func__, hdr->లెక్క, రికార్డు_లెక్క - దత్తాంశం->rec_లెక్క);
		తిరిగి -ENOSPC;
	}

	ptr += పరిమాణం(*hdr);

	/* Read the records */
	rec_కొలత = hdr->లెక్క * పరిమాణం(*దత్తాంశం->రికార్డు);
	జ్ఞాపకం_నకలు(దత్తాంశం->రికార్డు + దత్తాంశం->rec_లెక్క, ptr, rec_కొలత);

	/* Read the name strings */
	ptr += rec_కొలత;
	కొరకు (rec = దత్తాంశం->రికార్డు + దత్తాంశం->rec_లెక్క, i = 0; i < hdr->లెక్క;
	     i++, rec++) {
		rec->name = ptr;
		ఒకవేళ (xpl_phase() == PHASE_SPL)
			rec->name = strdup(ptr);

		/* Assume no data corruption here */
		ptr += అక్షరశ్రేణి_పొడవు(ptr) + 1;
	}

	/* Mark the records as read */
	దత్తాంశం->rec_లెక్క += hdr->లెక్క;
	దత్తాంశం->తదుపరి_id = hdr->తదుపరి_id;
	debug("Unstashed %d records\n", hdr->లెక్క);

	తిరిగి 0;
}

#ఒకవేళ IS_ENABLED(CONFIG_BOOTSTAGE_STASH)
పూర్ణాంకం _bootstage_stash_default(శూన్య)
{
	తిరిగి bootstage_stash(map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR, 0),
			       CONFIG_BOOTSTAGE_STASH_కొలత);
}

పూర్ణాంకం _bootstage_unstash_default(శూన్య)
{
	స్థిరరాశి శూన్య *stash = map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR,
				       CONFIG_BOOTSTAGE_STASH_కొలత);

	తిరిగి bootstage_unstash(stash, CONFIG_BOOTSTAGE_STASH_కొలత);
}
#అయితే_ముగింపు

పూర్ణాంకం bootstage_get_కొలత(bool కూడు_అక్షరశ్రేణులు)
{
	పూర్ణాంకం కొలత;

	కొలత = పరిమాణం(నిర్మాణం bootstage_దత్తాంశం);
	ఒకవేళ (కూడు_అక్షరశ్రేణులు) {
		నిర్మాణం bootstage_దత్తాంశం *దత్తాంశం = gd->bootstage;
		నిర్మాణం bootstage_రికార్డు *rec;
		పూర్ణాంకం i;

		కొరకు (rec = దత్తాంశం->రికార్డు, i = 0; i < దత్తాంశం->rec_లెక్క; i++, rec++)
			ఒకవేళ (rec->name)
				కొలత += అక్షరశ్రేణి_పొడవు(rec->name) + 1;
	}

	తిరిగి కొలత;
}

పూర్ణాంకం bootstage_init(bool మొదటి)
{
	నిర్మాణం bootstage_దత్తాంశం *దత్తాంశం;
	పూర్ణాంకం కొలత = పరిమాణం(నిర్మాణం bootstage_దత్తాంశం);

	gd->bootstage = (నిర్మాణం bootstage_దత్తాంశం *)జ్ఞాపకం_ఇవ్వు(కొలత);
	ఒకవేళ (!gd->bootstage)
		తిరిగి -ENOMEM;
	దత్తాంశం = gd->bootstage;
	memset(దత్తాంశం, '\0', కొలత);
	ఒకవేళ (మొదటి) {
		దత్తాంశం->తదుపరి_id = BOOTSTAGE_ID_USER;
		bootstage_కూడు_రికార్డు(BOOTSTAGE_ID_AWAKE, "reset", 0, 0);
	}

	తిరిగి 0;
}
