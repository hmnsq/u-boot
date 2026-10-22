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
স্থির সংরচনা iotrace {
	ulong start;
	ulong size;
	ulong needed_size;
	ulong offset;
	ulong region_start;
	ulong region_size;
	u32 crc32;
	bool enabled;
} iotrace;

স্থির শূন্য add_record(পূর্ণাঙ্ক flags, ধ্রুব শূন্য *ptr, ulong মান)
{
	সংরচনা iotrace_record srec, *rec = &srec;

	/*
	 * We don't support iotrace before relocation. Since the trace buffer
	 * is set up by a command, it can't be enabled at present. To change
	 * this we would need to set the iotrace buffer at build-time. See
	 * lib/trace.c for how this might be done if you are interested.
	 */
	যদি (!(gd->flags & GD_FLG_RELOC) || !iotrace.enabled)
		প্রত্যাগম;

	যদি (iotrace.region_size)
		যদি ((ulong)ptr < iotrace.region_start ||
		    (ulong)ptr > iotrace.region_start + iotrace.region_size)
			প্রত্যাগম;

	/* Store it if there is room */
	যদি (iotrace.offset + পরিমাণ(*rec) < iotrace.size) {
		rec = (সংরচনা iotrace_record *)map_sysmem(
					iotrace.start + iotrace.offset,
					পরিমাণ(মান));
	} অন্যথা {
		WARN_ONCE(1, "WARNING: iotrace buffer exhausted, please check needed length using \"iotrace stats\"\n");
		iotrace.needed_size += পরিমাণ(সংরচনা iotrace_record);
		প্রত্যাগম;
	}

	rec->timestamp = timer_get_us();
	rec->flags = flags;
	rec->addr = map_to_sysmem(ptr);
	rec->মান = মান;

	/* Update our checksum */
	iotrace.crc32 = crc32(iotrace.crc32, (unsigned বর্ণ *)rec,
			      পরিমাণ(*rec));

	iotrace.needed_size += পরিমাণ(সংরচনা iotrace_record);
	iotrace.offset += পরিমাণ(সংরচনা iotrace_record);
}

u32 iotrace_readl(ধ্রুব শূন্য *ptr)
{
	u32 v;

	v = readl(ptr);
	add_record(IOT_32 | IOT_READ, ptr, v);

	প্রত্যাগম v;
}

শূন্য iotrace_writel(ulong মান, শূন্য *ptr)
{
	add_record(IOT_32 | IOT_WRITE, ptr, মান);
	writel(মান, ptr);
}

u16 iotrace_readw(ধ্রুব শূন্য *ptr)
{
	u32 v;

	v = readw(ptr);
	add_record(IOT_16 | IOT_READ, ptr, v);

	প্রত্যাগম v;
}

শূন্য iotrace_writew(ulong মান, শূন্য *ptr)
{
	add_record(IOT_16 | IOT_WRITE, ptr, মান);
	writew(মান, ptr);
}

u8 iotrace_readb(ধ্রুব শূন্য *ptr)
{
	u32 v;

	v = readb(ptr);
	add_record(IOT_8 | IOT_READ, ptr, v);

	প্রত্যাগম v;
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
	প্রত্যাগম iotrace.crc32;
}

শূন্য iotrace_set_region(ulong start, ulong size)
{
	iotrace.region_start = start;
	iotrace.region_size = size;
}

শূন্য iotrace_reset_region(শূন্য)
{
	iotrace.region_start = 0;
	iotrace.region_size = 0;
}

শূন্য iotrace_get_region(ulong *start, ulong *size)
{
	*start = iotrace.region_start;
	*size = iotrace.region_size;
}

শূন্য iotrace_set_enabled(পূর্ণাঙ্ক enable)
{
	iotrace.enabled = enable;
}

পূর্ণাঙ্ক iotrace_get_enabled(শূন্য)
{
	প্রত্যাগম iotrace.enabled;
}

শূন্য iotrace_set_buffer(ulong start, ulong size)
{
	iotrace.start = start;
	iotrace.size = size;
	iotrace.offset = 0;
	iotrace.crc32 = 0;
}

শূন্য iotrace_get_buffer(ulong *start, ulong *size, ulong *needed_size, ulong *offset, ulong *গুনতি)
{
	*start = iotrace.start;
	*size = iotrace.size;
	*needed_size = iotrace.needed_size;
	*offset = iotrace.offset;
	*গুনতি = iotrace.offset / পরিমাণ(সংরচনা iotrace_record);
}
