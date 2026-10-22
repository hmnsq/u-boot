// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc.
 */

#define IOTRACE_IMPL

#অন্তর্ভাব <mapmem.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <asm/global_data.h>
#অন্তর্ভাব <asm/io.h>
#অন্তর্ভাব <linux/bug.h>
#অন্তর্ভাব <u-boot/crc.h>

DECLARE_GLOBAL_তথ্য_PTR;

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
স্থির কাঠামো iotrace {
	ulong start;
	ulong আকার_2;
	ulong needed_আকার;
	ulong offset;
	ulong region_start;
	ulong region_আকার;
	u32 crc32;
	bool enabled;
} iotrace;

স্থির শূন্য add_record(পূর্ণসংখ্যা flags, ধ্রুবক শূন্য *ptr, ulong মান)
{
	কাঠামো iotrace_record srec, *rec = &srec;

	/*
	 * We don't support iotrace before relocation. Since the trace buffer
	 * is set up by a command, it can't be enabled at present. To change
	 * this we would need to set the iotrace buffer at build-time. See
	 * lib/trace.c for how this might be done if you are interested.
	 */
	যদি (!(gd->flags & GD_FLG_RELOC) || !iotrace.enabled)
		ফেরত;

	যদি (iotrace.region_আকার)
		যদি ((ulong)ptr < iotrace.region_start ||
		    (ulong)ptr > iotrace.region_start + iotrace.region_আকার)
			ফেরত;

	/* Store it if there is room */
	যদি (iotrace.offset + আকার(*rec) < iotrace.আকার_2) {
		rec = (কাঠামো iotrace_record *)map_sysmem(
					iotrace.start + iotrace.offset,
					আকার(মান));
	} অন্যথা {
		WARN_ONCE(1, "WARNING: iotrace buffer exhausted, please check needed length using \"iotrace stats\"\n");
		iotrace.needed_আকার += আকার(কাঠামো iotrace_record);
		ফেরত;
	}

	rec->timestamp = timer_get_us();
	rec->flags = flags;
	rec->addr = map_to_sysmem(ptr);
	rec->মান = মান;

	/* Update our checksum */
	iotrace.crc32 = crc32(iotrace.crc32, (unsigned অক্ষর *)rec,
			      আকার(*rec));

	iotrace.needed_আকার += আকার(কাঠামো iotrace_record);
	iotrace.offset += আকার(কাঠামো iotrace_record);
}

u32 iotrace_readl(ধ্রুবক শূন্য *ptr)
{
	u32 v;

	v = readl(ptr);
	add_record(IOT_32 | IOT_READ, ptr, v);

	ফেরত v;
}

শূন্য iotrace_writel(ulong মান, শূন্য *ptr)
{
	add_record(IOT_32 | IOT_WRITE, ptr, মান);
	writel(মান, ptr);
}

u16 iotrace_readw(ধ্রুবক শূন্য *ptr)
{
	u32 v;

	v = readw(ptr);
	add_record(IOT_16 | IOT_READ, ptr, v);

	ফেরত v;
}

শূন্য iotrace_writew(ulong মান, শূন্য *ptr)
{
	add_record(IOT_16 | IOT_WRITE, ptr, মান);
	writew(মান, ptr);
}

u8 iotrace_readb(ধ্রুবক শূন্য *ptr)
{
	u32 v;

	v = readb(ptr);
	add_record(IOT_8 | IOT_READ, ptr, v);

	ফেরত v;
}

শূন্য iotrace_writeb(ulong মান, শূন্য *ptr)
{
	add_record(IOT_8 | IOT_WRITE, ptr, মান);
	writeb(মান, ptr);
}

শূন্য iotrace_reset_checksum(শূন্য)
{
	iotrace.crc32 = 0;
}

u32 iotrace_get_checksum(শূন্য)
{
	ফেরত iotrace.crc32;
}

শূন্য iotrace_set_region(ulong start, ulong আকার_2)
{
	iotrace.region_start = start;
	iotrace.region_আকার = আকার_2;
}

শূন্য iotrace_reset_region(শূন্য)
{
	iotrace.region_start = 0;
	iotrace.region_আকার = 0;
}

শূন্য iotrace_get_region(ulong *start, ulong *আকার_2)
{
	*start = iotrace.region_start;
	*আকার_2 = iotrace.region_আকার;
}

শূন্য iotrace_set_enabled(পূর্ণসংখ্যা enable)
{
	iotrace.enabled = enable;
}

পূর্ণসংখ্যা iotrace_get_enabled(শূন্য)
{
	ফেরত iotrace.enabled;
}

শূন্য iotrace_set_বাফার(ulong start, ulong আকার_2)
{
	iotrace.start = start;
	iotrace.আকার_2 = আকার_2;
	iotrace.offset = 0;
	iotrace.crc32 = 0;
}

শূন্য iotrace_get_বাফার(ulong *start, ulong *আকার_2, ulong *needed_আকার, ulong *offset, ulong *গুনতি)
{
	*start = iotrace.start;
	*আকার_2 = iotrace.আকার_2;
	*needed_আকার = iotrace.needed_আকার;
	*offset = iotrace.offset;
	*গুনতি = iotrace.offset / আকার(কাঠামো iotrace_record);
}
