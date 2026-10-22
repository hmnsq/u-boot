// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc.
 */

#define IOTRACE_IMPL

#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <asm/io.h>
#ಅನ್ತರ್ಭಾವ <linux/bug.h>
#ಅನ್ತರ್ಭಾವ <u-boot/crc.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * struct iotrace - current trace status and checksum
 *
 * @start:	Start address of iotrace buffer
 * @size:	Actual size of iotrace buffer in bytes
 * @needed_size: Needed of iotrace buffer in bytes
 * @offset:	Current write offset into iotrace buffer
 * @region_start: Address of IO region to trace
 * @region_size: Size of region to trace. if 0 will trace all address space
 * @crc32:	Current value of CRC chceksum of trace records
 * @enabled:	true if enabled, false if disabled
 */
ಸ್ಥಿರ ಸಂರಚನಾ iotrace {
	ulong start;
	ulong size;
	ulong needed_size;
	ulong offset;
	ulong region_start;
	ulong region_size;
	u32 crc32;
	bool enabled;
} iotrace;

ಸ್ಥಿರ ಶೂನ್ಯ add_record(ಪೂರ್ಣಾಙ್ಕ flags, ಧ್ರುವ ಶೂನ್ಯ *ptr, ulong ಮೌಲ್ಯ)
{
	ಸಂರಚನಾ iotrace_record srec, *rec = &srec;

	/*
	 * We don't support iotrace before relocation. Since the trace buffer
	 * is set up by a command, it can't be enabled at present. To change
	 * this we would need to set the iotrace buffer at build-time. See
	 * lib/trace.c for how this might be done if you are interested.
	 */
	ಯದಿ (!(gd->flags & GD_FLG_RELOC) || !iotrace.enabled)
		ಪ್ರತ್ಯಾಗಮ;

	ಯದಿ (iotrace.region_size)
		ಯದಿ ((ulong)ptr < iotrace.region_start ||
		    (ulong)ptr > iotrace.region_start + iotrace.region_size)
			ಪ್ರತ್ಯಾಗಮ;

	/* Store it if there is room */
	ಯದಿ (iotrace.offset + ಪರಿಮಾಣ(*rec) < iotrace.size) {
		rec = (ಸಂರಚನಾ iotrace_record *)map_sysmem(
					iotrace.start + iotrace.offset,
					ಪರಿಮಾಣ(ಮೌಲ್ಯ));
	} ಅನ್ಯಥಾ {
		WARN_ONCE(1, "WARNING: iotrace buffer exhausted, please check needed length using \"iotrace stats\"\n");
		iotrace.needed_size += ಪರಿಮಾಣ(ಸಂರಚನಾ iotrace_record);
		ಪ್ರತ್ಯಾಗಮ;
	}

	rec->timestamp = timer_get_us();
	rec->flags = flags;
	rec->addr = map_to_sysmem(ptr);
	rec->ಮೌಲ್ಯ = ಮೌಲ್ಯ;

	/* Update our checksum */
	iotrace.crc32 = crc32(iotrace.crc32, (unsigned ವರ್ಣ *)rec,
			      ಪರಿಮಾಣ(*rec));

	iotrace.needed_size += ಪರಿಮಾಣ(ಸಂರಚನಾ iotrace_record);
	iotrace.offset += ಪರಿಮಾಣ(ಸಂರಚನಾ iotrace_record);
}

u32 iotrace_readl(ಧ್ರುವ ಶೂನ್ಯ *ptr)
{
	u32 v;

	v = readl(ptr);
	add_record(IOT_32 | IOT_READ, ptr, v);

	ಪ್ರತ್ಯಾಗಮ v;
}

ಶೂನ್ಯ iotrace_writel(ulong ಮೌಲ್ಯ, ಶೂನ್ಯ *ptr)
{
	add_record(IOT_32 | IOT_WRITE, ptr, ಮೌಲ್ಯ);
	writel(ಮೌಲ್ಯ, ptr);
}

u16 iotrace_readw(ಧ್ರುವ ಶೂನ್ಯ *ptr)
{
	u32 v;

	v = readw(ptr);
	add_record(IOT_16 | IOT_READ, ptr, v);

	ಪ್ರತ್ಯಾಗಮ v;
}

ಶೂನ್ಯ iotrace_writew(ulong ಮೌಲ್ಯ, ಶೂನ್ಯ *ptr)
{
	add_record(IOT_16 | IOT_WRITE, ptr, ಮೌಲ್ಯ);
	writew(ಮೌಲ್ಯ, ptr);
}

u8 iotrace_readb(ಧ್ರುವ ಶೂನ್ಯ *ptr)
{
	u32 v;

	v = readb(ptr);
	add_record(IOT_8 | IOT_READ, ptr, v);

	ಪ್ರತ್ಯಾಗಮ v;
}

ಶೂನ್ಯ iotrace_writeb(ulong ಮೌಲ್ಯ, ಶೂನ್ಯ *ptr)
{
	add_record(IOT_8 | IOT_WRITE, ptr, ಮೌಲ್ಯ);
	writeb(ಮೌಲ್ಯ, ptr);
}

ಶೂನ್ಯ iotrace_reset_checksum(ಶೂನ್ಯ)
{
	iotrace.crc32 = 0;
}

u32 iotrace_get_checksum(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ iotrace.crc32;
}

ಶೂನ್ಯ iotrace_set_region(ulong start, ulong size)
{
	iotrace.region_start = start;
	iotrace.region_size = size;
}

ಶೂನ್ಯ iotrace_reset_region(ಶೂನ್ಯ)
{
	iotrace.region_start = 0;
	iotrace.region_size = 0;
}

ಶೂನ್ಯ iotrace_get_region(ulong *start, ulong *size)
{
	*start = iotrace.region_start;
	*size = iotrace.region_size;
}

ಶೂನ್ಯ iotrace_set_enabled(ಪೂರ್ಣಾಙ್ಕ enable)
{
	iotrace.enabled = enable;
}

ಪೂರ್ಣಾಙ್ಕ iotrace_get_enabled(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ iotrace.enabled;
}

ಶೂನ್ಯ iotrace_set_buffer(ulong start, ulong size)
{
	iotrace.start = start;
	iotrace.size = size;
	iotrace.offset = 0;
	iotrace.crc32 = 0;
}

ಶೂನ್ಯ iotrace_get_buffer(ulong *start, ulong *size, ulong *needed_size, ulong *offset, ulong *ಎಣಿಕೆ)
{
	*start = iotrace.start;
	*size = iotrace.size;
	*needed_size = iotrace.needed_size;
	*offset = iotrace.offset;
	*ಎಣಿಕೆ = iotrace.offset / ಪರಿಮಾಣ(ಸಂರಚನಾ iotrace_record);
}
