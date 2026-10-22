// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 */

/*
 * This module records the progress of boot and arbitrary commands, and
 * permits accurate timestamping of each.
 */

#ವ್ಯಾಖ್ಯಾನಿಸು LOG_CATEGORY	LOGC_BOOT

#ಅನ್ತರ್ಭಾವ <bootstage.h>
#ಅನ್ತರ್ಭಾವ <hang.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <sort.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <linux/compiler.h>
#ಅನ್ತರ್ಭಾವ <linux/libfdt.h>

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

ಎಣಿಕೆ {
	ದಾಖಲೆ_ಎಣಿಕೆ = CONFIG_VAL(BOOTSTAGE_ದಾಖಲೆ_ಎಣಿಕೆ),
};

ರಚನೆ bootstage_ದಾಖಲೆ {
	ulong ಸಮಯ_us;
	uint32_t ಆರಂಭ_us;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
	ಪೂರ್ಣಾಂಕ flags;		/* see enum bootstage_flags */
	ಎಣಿಕೆ bootstage_id id;
};

ರಚನೆ bootstage_ದತ್ತಾಂಶ {
	uint rec_ಎಣಿಕೆ;
	uint ಮುಂದಿನ_id;
	ರಚನೆ bootstage_ದಾಖಲೆ ದಾಖಲೆ[ದಾಖಲೆ_ಎಣಿಕೆ];
};

ಎಣಿಕೆ {
	BOOTSTAGE_ಆವೃತ್ತಿ	= 0,
	BOOTSTAGE_MAGIC		= 0xb00757a3,
	BOOTSTAGE_DIGITS	= 9,
};

ರಚನೆ bootstage_hdr {
	u32 version;		/* BOOTSTAGE_VERSION */
	u32 ಎಣಿಕೆ_2;		/* Number of records */
	u32 ಗಾತ್ರ_2;		/* Total data size (non-zero if valid) */
	u32 magic;		/* Magic number */
	u32 ಮುಂದಿನ_id;		/* Next ID to use for bootstage */
};

ಪೂರ್ಣಾಂಕ bootstage_relocate(ಶೂನ್ಯ *ಗಮ್ಯ)
{
	ರಚನೆ bootstage_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ;
	ಪೂರ್ಣಾಂಕ i;
	ಅಕ್ಷರ *ptr;

	debug("Copying bootstage from %p to %p\n", gd->bootstage, ಗಮ್ಯ);
	ಸ್ಮರಣೆ_ನಕಲು(ಗಮ್ಯ, gd->bootstage, ಗಾತ್ರ(ರಚನೆ bootstage_ದತ್ತಾಂಶ));
	ದತ್ತಾಂಶ = gd->bootstage = ಗಮ್ಯ;

	/* Figure out where to relocate the strings to */
	ptr = (ಅಕ್ಷರ *)(ದತ್ತಾಂಶ + 1);

	/*
	 * Duplicate all strings.  They may point to an old location in the
	 * program .text section that can eventually get trashed.
	 */
	debug("Relocating %d records\n", ದತ್ತಾಂಶ->rec_ಎಣಿಕೆ);
	ಗಾಗಿ (i = 0; i < ದತ್ತಾಂಶ->rec_ಎಣಿಕೆ; i++) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *from = ದತ್ತಾಂಶ->ದಾಖಲೆ[i].name;

		ಆದರೆ (!from)
			ಮುಂದುವರಿ;
		strcpy(ptr, from);
		ದತ್ತಾಂಶ->ದಾಖಲೆ[i].name = ptr;
		ptr += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ptr) + 1;
	}

	ಹಿಂತಿರುಗಿ 0;
}

ರಚನೆ bootstage_ದಾಖಲೆ *ಕಂಡುಹಿಡಿ_id(ರಚನೆ bootstage_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ,
				 ಎಣಿಕೆ bootstage_id id)
{
	ರಚನೆ bootstage_ದಾಖಲೆ *rec;
	ರಚನೆ bootstage_ದಾಖಲೆ *ಕೊನೆ_2;

	ಗಾಗಿ (rec = ದತ್ತಾಂಶ->ದಾಖಲೆ, ಕೊನೆ_2 = rec + ದತ್ತಾಂಶ->rec_ಎಣಿಕೆ; rec < ಕೊನೆ_2;
	     rec++) {
		ಆದರೆ (rec->id == id)
			ಹಿಂತಿರುಗಿ rec;
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ರಚನೆ bootstage_ದಾಖಲೆ *ensure_id(ರಚನೆ bootstage_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ,
				   ಎಣಿಕೆ bootstage_id id)
{
	ರಚನೆ bootstage_ದಾಖಲೆ *rec;

	rec = ಕಂಡುಹಿಡಿ_id(ದತ್ತಾಂಶ, id);
	ಆದರೆ (!rec && ದತ್ತಾಂಶ->rec_ಎಣಿಕೆ < ದಾಖಲೆ_ಎಣಿಕೆ) {
		rec = &ದತ್ತಾಂಶ->ದಾಖಲೆ[ದತ್ತಾಂಶ->rec_ಎಣಿಕೆ++];
		rec->id = id;
		ಹಿಂತಿರುಗಿ rec;
	}

	ಹಿಂತಿರುಗಿ rec;
}

ulong bootstage_ಕೂಡಿಸು_ದಾಖಲೆ(ಎಣಿಕೆ bootstage_id id, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name,
			   ಪೂರ್ಣಾಂಕ flags, ulong mark)
{
	ರಚನೆ bootstage_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ = gd->bootstage;
	ರಚನೆ bootstage_ದಾಖಲೆ *rec;

	/*
	 * initf_bootstage() is called very early during boot but since hang()
	 * calls bootstage_error() we can be called before bootstage is set up.
	 * Add a check to avoid this.
	 */
	ಆದರೆ (!ದತ್ತಾಂಶ)
		ಹಿಂತಿರುಗಿ mark;
	ಆದರೆ (flags & BOOTSTAGEF_ALLOC)
		id = ದತ್ತಾಂಶ->ಮುಂದಿನ_id++;

	/* Only record the first event for each */
	rec = ಕಂಡುಹಿಡಿ_id(ದತ್ತಾಂಶ, id);
	ಆದರೆ (!rec) {
		ಆದರೆ (ದತ್ತಾಂಶ->rec_ಎಣಿಕೆ < ದಾಖಲೆ_ಎಣಿಕೆ) {
			rec = &ದತ್ತಾಂಶ->ದಾಖಲೆ[ದತ್ತಾಂಶ->rec_ಎಣಿಕೆ++];
			rec->ಸಮಯ_us = mark;
			rec->name = name;
			rec->flags = flags;
			rec->id = id;
		} ಇಲ್ಲದಿದ್ದರೆ {
			log_warning("Bootstage space exhausted\n");
		}
	}

	/* Tell the board about this progress */
	show_boot_ಪ್ರಗತಿ(flags & BOOTSTAGEF_ERROR ? -id : id);

	ಹಿಂತಿರುಗಿ mark;
}

ulong bootstage_error_name(ಎಣಿಕೆ bootstage_id id, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಹಿಂತಿರುಗಿ bootstage_ಕೂಡಿಸು_ದಾಖಲೆ(id, name, BOOTSTAGEF_ERROR,
				    timer_get_boot_us());
}

ulong bootstage_mark_name(ಎಣಿಕೆ bootstage_id id, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಪೂರ್ಣಾಂಕ flags = 0;

	ಆದರೆ (id == BOOTSTAGE_ID_ALLOC)
		flags = BOOTSTAGEF_ALLOC;

	ಹಿಂತಿರುಗಿ bootstage_ಕೂಡಿಸು_ದಾಖಲೆ(id, name, flags, timer_get_boot_us());
}

ulong bootstage_mark_ಸಂಕೇತ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *func, ಪೂರ್ಣಾಂಕ linenum)
{
	ಅಕ್ಷರ *str, *p;
	__maybe_unused ಅಕ್ಷರ *ಕೊನೆ_2;
	ಪೂರ್ಣಾಂಕ len = 0;

	/* First work out the length we need to allocate */
	ಆದರೆ (linenum != -1)
		len = 11;
	ಆದರೆ (func)
		len += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(func);
	ಆದರೆ (file)
		len += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(file);

	str = ಸ್ಮರಣೆ_ಕೊಡು(len + 1);
	p = str;
	ಕೊನೆ_2 = p + len;
	ಆದರೆ (file)
		p += snprintf(p, ಕೊನೆ_2 - p, "%s,", file);
	ಆದರೆ (linenum != -1)
		p += snprintf(p, ಕೊನೆ_2 - p, "%d", linenum);
	ಆದರೆ (func)
		p += snprintf(p, ಕೊನೆ_2 - p, ": %s", func);

	ಹಿಂತಿರುಗಿ bootstage_mark_name(BOOTSTAGE_ID_ALLOC, str);
}

uint32_t bootstage_ಆರಂಭ(ಎಣಿಕೆ bootstage_id id, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ರಚನೆ bootstage_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ = gd->bootstage;
	ರಚನೆ bootstage_ದಾಖಲೆ *rec = ensure_id(ದತ್ತಾಂಶ, id);
	ulong ಆರಂಭ_us = timer_get_boot_us();

	ಆದರೆ (rec) {
		rec->ಆರಂಭ_us = ಆರಂಭ_us;
		rec->name = name;
	}

	ಹಿಂತಿರುಗಿ ಆರಂಭ_us;
}

uint32_t bootstage_accum(ಎಣಿಕೆ bootstage_id id)
{
	ರಚನೆ bootstage_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ = gd->bootstage;
	ರಚನೆ bootstage_ದಾಖಲೆ *rec = ensure_id(ದತ್ತಾಂಶ, id);
	uint32_t duration;

	ಆದರೆ (!rec)
		ಹಿಂತಿರುಗಿ 0;
	duration = (uint32_t)timer_get_boot_us() - rec->ಆರಂಭ_us;
	rec->ಸಮಯ_us += duration;

	ಹಿಂತಿರುಗಿ duration;
}

/**
 * Get a record name as a printable string
 *
 * @param buf	Buffer to put name if needed
 * @param len	Length of buffer
 * @param rec	Boot stage record to get the name from
 * Return: pointer to name, either from the record or pointing to buf.
 */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *get_ದಾಖಲೆ_name(ಅಕ್ಷರ *buf, ಪೂರ್ಣಾಂಕ len,
				   ಸ್ಥಿರಾಂಕ ರಚನೆ bootstage_ದಾಖಲೆ *rec)
{
	ಆದರೆ (rec->name)
		ಹಿಂತಿರುಗಿ rec->name;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (rec->id >= BOOTSTAGE_ID_USER)
		snprintf(buf, len, "user_%d", rec->id - BOOTSTAGE_ID_USER);
	ಇಲ್ಲದಿದ್ದರೆ
		snprintf(buf, len, "id=%d", rec->id);

	ಹಿಂತಿರುಗಿ buf;
}

ಸ್ಥಿರ uint32_t print_ಸಮಯ_ದಾಖಲೆ(ರಚನೆ bootstage_ದಾಖಲೆ *rec, uint32_t prev)
{
	ಅಕ್ಷರ buf[20];

	ಆದರೆ (prev == -1U) {
		ಬರೆ("%11s", "");
		print_grouped_ull(rec->ಸಮಯ_us, BOOTSTAGE_DIGITS);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (prev > rec->ಸಮಯ_us)
			prev = 0;
		print_grouped_ull(rec->ಸಮಯ_us, BOOTSTAGE_DIGITS);
		print_grouped_ull(rec->ಸಮಯ_us - prev, BOOTSTAGE_DIGITS);
	}
	ಬರೆ("  %s\n", get_ದಾಖಲೆ_name(buf, ಗಾತ್ರ(buf), rec));

	ಹಿಂತಿರುಗಿ rec->ಸಮಯ_us;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ದ_LIBFDT
/**
 * Add all bootstage timings to a device tree.
 *
 * @param blob	Device tree blob
 * Return: 0 on success, != 0 on failure.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಕೂಡಿಸು_bootstages_devicetree(ರಚನೆ fdt_ಶೀರ್ಷಿಕೆ *blob)
{
	ರಚನೆ bootstage_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ = gd->bootstage;
	ಪೂರ್ಣಾಂಕ bootstage;
	ಅಕ್ಷರ buf[20];
	ಪೂರ್ಣಾಂಕ recnum;
	ಪೂರ್ಣಾಂಕ i;

	ಆದರೆ (!blob)
		ಹಿಂತಿರುಗಿ 0;

	/*
	 * Create the node for bootstage.
	 * The address of flat device tree is set up by the command bootm.
	 */
	bootstage = fdt_ಕೂಡಿಸು_subnode(blob, 0, "bootstage");
	ಆದರೆ (bootstage < 0)
		ಹಿಂತಿರುಗಿ -EINVAL;

	/*
	 * Insert the timings to the device tree in the reverse order so
	 * that they can be printed in the Linux kernel in the right order.
	 */
	ಗಾಗಿ (recnum = ದತ್ತಾಂಶ->rec_ಎಣಿಕೆ - 1, i = 0; recnum >= 0; recnum--, i++) {
		ರಚನೆ bootstage_ದಾಖಲೆ *rec = &ದತ್ತಾಂಶ->ದಾಖಲೆ[recnum];
		ಪೂರ್ಣಾಂಕ ಗಂಟು;

		ಆದರೆ (rec->id != BOOTSTAGE_ID_AWAKE && rec->ಸಮಯ_us == 0)
			ಮುಂದುವರಿ;

		ಗಂಟು = fdt_ಕೂಡಿಸು_subnode(blob, bootstage, simple_itoa(i));
		ಆದರೆ (ಗಂಟು < 0)
			ನಿಲ್ಲಿಸು;

		/* add properties to the node. */
		ಆದರೆ (fdt_setprop_string(blob, ಗಂಟು, "name",
				       get_ದಾಖಲೆ_name(buf, ಗಾತ್ರ(buf), rec)))
			ಹಿಂತಿರುಗಿ -EINVAL;

		/* Check if this is a 'mark' or 'accum' record */
		ಆದರೆ (fdt_setprop_cell(blob, ಗಂಟು,
				rec->ಆರಂಭ_us ? "accum" : "mark",
				rec->ಸಮಯ_us))
			ಹಿಂತಿರುಗಿ -EINVAL;
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ bootstage_fdt_ಕೂಡಿಸು_report(ಶೂನ್ಯ)
{
	ಆದರೆ (ಕೂಡಿಸು_bootstages_devicetree(working_fdt))
		puts("bootstage: Failed to add to device tree\n");

	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ bootstage_report(ಶೂನ್ಯ)
{
	ರಚನೆ bootstage_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ = gd->bootstage;
	ರಚನೆ bootstage_ದಾಖಲೆ *rec = ದತ್ತಾಂಶ->ದಾಖಲೆ;
	uint32_t prev;
	ಪೂರ್ಣಾಂಕ i;

	ಬರೆ("Timer summary in microseconds (%d records):\n",
	       ದತ್ತಾಂಶ->rec_ಎಣಿಕೆ);
	ಬರೆ("%11s%11s  %s\n", "Mark", "Elapsed", "Stage");

	prev = print_ಸಮಯ_ದಾಖಲೆ(rec, 0);

	ಗಾಗಿ (i = 1, rec++; i < ದತ್ತಾಂಶ->rec_ಎಣಿಕೆ; i++, rec++) {
		ಆದರೆ (rec->id && !rec->ಆರಂಭ_us)
			prev = print_ಸಮಯ_ದಾಖಲೆ(rec, prev);
	}
	ಆದರೆ (ದತ್ತಾಂಶ->rec_ಎಣಿಕೆ > ದಾಖಲೆ_ಎಣಿಕೆ)
		ಬರೆ("Overflowed internal boot id table by %d entries\n"
		       "Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		       ದತ್ತಾಂಶ->rec_ಎಣಿಕೆ - ದಾಖಲೆ_ಎಣಿಕೆ);

	puts("\nAccumulated time:\n");
	ಗಾಗಿ (i = 0, rec = ದತ್ತಾಂಶ->ದಾಖಲೆ; i < ದತ್ತಾಂಶ->rec_ಎಣಿಕೆ; i++, rec++) {
		ಆದರೆ (rec->ಆರಂಭ_us)
			prev = print_ಸಮಯ_ದಾಖಲೆ(rec, -1);
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
ಸ್ಥಿರ ಶೂನ್ಯ append_ದತ್ತಾಂಶ(ಅಕ್ಷರ **ptrp, ಅಕ್ಷರ *ಕೊನೆ_2, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	ಅಕ್ಷರ *ptr = *ptrp;

	*ptrp += ಗಾತ್ರ_2;
	ಆದರೆ (*ptrp > ಕೊನೆ_2)
		ಹಿಂತಿರುಗಿ;

	ಸ್ಮರಣೆ_ನಕಲು(ptr, ದತ್ತಾಂಶ, ಗಾತ್ರ_2);
}

ಪೂರ್ಣಾಂಕ bootstage_stash(ಶೂನ್ಯ *ತಳ, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ bootstage_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ = gd->bootstage;
	ರಚನೆ bootstage_hdr *hdr = (ರಚನೆ bootstage_hdr *)ತಳ;
	ಸ್ಥಿರಾಂಕ ರಚನೆ bootstage_ದಾಖಲೆ *rec;
	ಅಕ್ಷರ buf[20];
	ಅಕ್ಷರ *ptr = ತಳ, *ಕೊನೆ_2 = ptr + ಗಾತ್ರ_2;
	ಪೂರ್ಣಾಂಕ i;

	ಆದರೆ (hdr + 1 > (ರಚನೆ bootstage_hdr *)ಕೊನೆ_2) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		ಹಿಂತಿರುಗಿ -ENOSPC;
	}

	/* Write an arbitrary version number */
	hdr->version = BOOTSTAGE_ಆವೃತ್ತಿ;

	hdr->ಎಣಿಕೆ_2 = ದತ್ತಾಂಶ->rec_ಎಣಿಕೆ;
	hdr->ಗಾತ್ರ_2 = 0;
	hdr->magic = BOOTSTAGE_MAGIC;
	hdr->ಮುಂದಿನ_id = ದತ್ತಾಂಶ->ಮುಂದಿನ_id;
	ptr += ಗಾತ್ರ(*hdr);

	/* Write the records, silently stopping when we run out of space */
	ಗಾಗಿ (rec = ದತ್ತಾಂಶ->ದಾಖಲೆ, i = 0; i < ದತ್ತಾಂಶ->rec_ಎಣಿಕೆ; i++, rec++)
		append_ದತ್ತಾಂಶ(&ptr, ಕೊನೆ_2, rec, ಗಾತ್ರ(*rec));

	/* Write the name strings */
	ಗಾಗಿ (rec = ದತ್ತಾಂಶ->ದಾಖಲೆ, i = 0; i < ದತ್ತಾಂಶ->rec_ಎಣಿಕೆ; i++, rec++) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;

		name = get_ದಾಖಲೆ_name(buf, ಗಾತ್ರ(buf), rec);
		append_ದತ್ತಾಂಶ(&ptr, ಕೊನೆ_2, name, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name) + 1);
	}

	/* Check for buffer overflow */
	ಆದರೆ (ptr > ಕೊನೆ_2) {
		debug("%s: Not enough space for bootstage stash\n", __func__);
		ಹಿಂತಿರುಗಿ -ENOSPC;
	}

	/* Update total data size */
	hdr->ಗಾತ್ರ_2 = ptr - (ಅಕ್ಷರ *)ತಳ;
	debug("Stashed %d records\n", hdr->ಎಣಿಕೆ_2);

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ bootstage_unstash(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ತಳ, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ bootstage_hdr *hdr = (ರಚನೆ bootstage_hdr *)ತಳ;
	ರಚನೆ bootstage_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ = gd->bootstage;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ptr = ತಳ, *ಕೊನೆ_2 = ptr + ಗಾತ್ರ_2;
	ರಚನೆ bootstage_ದಾಖಲೆ *rec;
	uint rec_ಗಾತ್ರ;
	ಪೂರ್ಣಾಂಕ i;

	ಆದರೆ (ಗಾತ್ರ_2 == -1)
		ಕೊನೆ_2 = (ಅಕ್ಷರ *)(~(uintptr_t)0);

	ಆದರೆ (hdr + 1 > (ರಚನೆ bootstage_hdr *)ಕೊನೆ_2) {
		debug("%s: Not enough space for bootstage hdr\n", __func__);
		ಹಿಂತಿರುಗಿ -EPERM;
	}

	ಆದರೆ (hdr->magic != BOOTSTAGE_MAGIC) {
		debug("%s: Invalid bootstage magic\n", __func__);
		ಹಿಂತಿರುಗಿ -ENOENT;
	}

	ಆದರೆ (ptr + hdr->ಗಾತ್ರ_2 > ಕೊನೆ_2) {
		debug("%s: Bootstage data runs past buffer end\n", __func__);
		ಹಿಂತಿರುಗಿ -ENOSPC;
	}

	ಆದರೆ (hdr->ಎಣಿಕೆ_2 * ಗಾತ್ರ(*rec) > hdr->ಗಾತ್ರ_2) {
		debug("%s: Bootstage has %d records needing %lu bytes, but "
			"only %d bytes is available\n", __func__, hdr->ಎಣಿಕೆ_2,
		      (ulong)hdr->ಎಣಿಕೆ_2 * ಗಾತ್ರ(*rec), hdr->ಗಾತ್ರ_2);
		ಹಿಂತಿರುಗಿ -ENOSPC;
	}

	ಆದರೆ (hdr->version != BOOTSTAGE_ಆವೃತ್ತಿ) {
		debug("%s: Bootstage data version %#0x unrecognised\n",
		      __func__, hdr->version);
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	ಆದರೆ (ದತ್ತಾಂಶ->rec_ಎಣಿಕೆ + hdr->ಎಣಿಕೆ_2 > ದಾಖಲೆ_ಎಣಿಕೆ) {
		debug("%s: Bootstage has %d records, we have space for %d\n"
			"Please increase CONFIG_(PHASE_)BOOTSTAGE_RECORD_COUNT\n",
		      __func__, hdr->ಎಣಿಕೆ_2, ದಾಖಲೆ_ಎಣಿಕೆ - ದತ್ತಾಂಶ->rec_ಎಣಿಕೆ);
		ಹಿಂತಿರುಗಿ -ENOSPC;
	}

	ptr += ಗಾತ್ರ(*hdr);

	/* Read the records */
	rec_ಗಾತ್ರ = hdr->ಎಣಿಕೆ_2 * ಗಾತ್ರ(*ದತ್ತಾಂಶ->ದಾಖಲೆ);
	ಸ್ಮರಣೆ_ನಕಲು(ದತ್ತಾಂಶ->ದಾಖಲೆ + ದತ್ತಾಂಶ->rec_ಎಣಿಕೆ, ptr, rec_ಗಾತ್ರ);

	/* Read the name strings */
	ptr += rec_ಗಾತ್ರ;
	ಗಾಗಿ (rec = ದತ್ತಾಂಶ->ದಾಖಲೆ + ದತ್ತಾಂಶ->rec_ಎಣಿಕೆ, i = 0; i < hdr->ಎಣಿಕೆ_2;
	     i++, rec++) {
		rec->name = ptr;
		ಆದರೆ (xpl_phase() == PHASE_SPL)
			rec->name = strdup(ptr);

		/* Assume no data corruption here */
		ptr += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ptr) + 1;
	}

	/* Mark the records as read */
	ದತ್ತಾಂಶ->rec_ಎಣಿಕೆ += hdr->ಎಣಿಕೆ_2;
	ದತ್ತಾಂಶ->ಮುಂದಿನ_id = hdr->ಮುಂದಿನ_id;
	debug("Unstashed %d records\n", hdr->ಎಣಿಕೆ_2);

	ಹಿಂತಿರುಗಿ 0;
}

#ಆದರೆ IS_ENABLED(CONFIG_BOOTSTAGE_STASH)
ಪೂರ್ಣಾಂಕ _bootstage_stash_default(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ bootstage_stash(map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR, 0),
			       CONFIG_BOOTSTAGE_STASH_ಗಾತ್ರ);
}

ಪೂರ್ಣಾಂಕ _bootstage_unstash_default(ಶೂನ್ಯ)
{
	ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *stash = map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR,
				       CONFIG_BOOTSTAGE_STASH_ಗಾತ್ರ);

	ಹಿಂತಿರುಗಿ bootstage_unstash(stash, CONFIG_BOOTSTAGE_STASH_ಗಾತ್ರ);
}
#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ bootstage_get_ಗಾತ್ರ(bool ಕೂಡಿಸು_ಅಕ್ಷರಸರಣಿಗಳು)
{
	ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2;

	ಗಾತ್ರ_2 = ಗಾತ್ರ(ರಚನೆ bootstage_ದತ್ತಾಂಶ);
	ಆದರೆ (ಕೂಡಿಸು_ಅಕ್ಷರಸರಣಿಗಳು) {
		ರಚನೆ bootstage_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ = gd->bootstage;
		ರಚನೆ bootstage_ದಾಖಲೆ *rec;
		ಪೂರ್ಣಾಂಕ i;

		ಗಾಗಿ (rec = ದತ್ತಾಂಶ->ದಾಖಲೆ, i = 0; i < ದತ್ತಾಂಶ->rec_ಎಣಿಕೆ; i++, rec++)
			ಆದರೆ (rec->name)
				ಗಾತ್ರ_2 += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(rec->name) + 1;
	}

	ಹಿಂತಿರುಗಿ ಗಾತ್ರ_2;
}

ಪೂರ್ಣಾಂಕ bootstage_init(bool ಮೊದಲ)
{
	ರಚನೆ bootstage_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ;
	ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = ಗಾತ್ರ(ರಚನೆ bootstage_ದತ್ತಾಂಶ);

	gd->bootstage = (ರಚನೆ bootstage_ದತ್ತಾಂಶ *)ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ_2);
	ಆದರೆ (!gd->bootstage)
		ಹಿಂತಿರುಗಿ -ENOMEM;
	ದತ್ತಾಂಶ = gd->bootstage;
	memset(ದತ್ತಾಂಶ, '\0', ಗಾತ್ರ_2);
	ಆದರೆ (ಮೊದಲ) {
		ದತ್ತಾಂಶ->ಮುಂದಿನ_id = BOOTSTAGE_ID_USER;
		bootstage_ಕೂಡಿಸು_ದಾಖಲೆ(BOOTSTAGE_ID_AWAKE, "reset", 0, 0);
	}

	ಹಿಂತಿರುಗಿ 0;
}
