// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc.
 */

#define IOTRACE_IMPL

#شامل <mapmem.h>
#شامل <time.h>
#شامل <asm/global_data.h>
#شامل <asm/io.h>
#شامل <linux/bug.h>
#شامل <u-boot/crc.h>

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
ایستا ساختار iotrace {
	ulong start;
	ulong size;
	ulong needed_size;
	ulong offset;
	ulong region_start;
	ulong region_size;
	u32 crc32;
	bool enabled;
} iotrace;

ایستا تهی add_record(صحیح flags, ثابت تهی *ptr, ulong مقدار)
{
	ساختار iotrace_record srec, *rec = &srec;

	/*
	 * We don't support iotrace before relocation. Since the trace buffer
	 * is set up by a command, it can't be enabled at present. To change
	 * this we would need to set the iotrace buffer at build-time. See
	 * lib/trace.c for how this might be done if you are interested.
	 */
	اگر (!(gd->flags & GD_FLG_RELOC) || !iotrace.enabled)
		بازگشت;

	اگر (iotrace.region_size)
		اگر ((ulong)ptr < iotrace.region_start ||
		    (ulong)ptr > iotrace.region_start + iotrace.region_size)
			بازگشت;

	/* Store it if there is room */
	اگر (iotrace.offset + اندازه(*rec) < iotrace.size) {
		rec = (ساختار iotrace_record *)map_sysmem(
					iotrace.start + iotrace.offset,
					اندازه(مقدار));
	} وگرنه {
		WARN_ONCE(1, "WARNING: iotrace buffer exhausted, please check needed length using \"iotrace stats\"\n");
		iotrace.needed_size += اندازه(ساختار iotrace_record);
		بازگشت;
	}

	rec->timestamp = timer_get_us();
	rec->flags = flags;
	rec->addr = map_to_sysmem(ptr);
	rec->مقدار = مقدار;

	/* Update our checksum */
	iotrace.crc32 = crc32(iotrace.crc32, (unsigned نویسه *)rec,
			      اندازه(*rec));

	iotrace.needed_size += اندازه(ساختار iotrace_record);
	iotrace.offset += اندازه(ساختار iotrace_record);
}

u32 iotrace_readl(ثابت تهی *ptr)
{
	u32 v;

	v = readl(ptr);
	add_record(IOT_32 | IOT_READ, ptr, v);

	بازگشت v;
}

تهی iotrace_writel(ulong مقدار, تهی *ptr)
{
	add_record(IOT_32 | IOT_WRITE, ptr, مقدار);
	writel(مقدار, ptr);
}

u16 iotrace_readw(ثابت تهی *ptr)
{
	u32 v;

	v = readw(ptr);
	add_record(IOT_16 | IOT_READ, ptr, v);

	بازگشت v;
}

تهی iotrace_writew(ulong مقدار, تهی *ptr)
{
	add_record(IOT_16 | IOT_WRITE, ptr, مقدار);
	writew(مقدار, ptr);
}

u8 iotrace_readb(ثابت تهی *ptr)
{
	u32 v;

	v = readb(ptr);
	add_record(IOT_8 | IOT_READ, ptr, v);

	بازگشت v;
}

تهی iotrace_writeb(ulong مقدار, تهی *ptr)
{
	add_record(IOT_8 | IOT_WRITE, ptr, مقدار);
	writeb(مقدار, ptr);
}

تهی iotrace_reset_checksum(تهی)
{
	iotrace.crc32 = 0;
}

u32 iotrace_get_checksum(تهی)
{
	بازگشت iotrace.crc32;
}

تهی iotrace_set_region(ulong start, ulong size)
{
	iotrace.region_start = start;
	iotrace.region_size = size;
}

تهی iotrace_reset_region(تهی)
{
	iotrace.region_start = 0;
	iotrace.region_size = 0;
}

تهی iotrace_get_region(ulong *start, ulong *size)
{
	*start = iotrace.region_start;
	*size = iotrace.region_size;
}

تهی iotrace_set_enabled(صحیح enable)
{
	iotrace.enabled = enable;
}

صحیح iotrace_get_enabled(تهی)
{
	بازگشت iotrace.enabled;
}

تهی iotrace_set_buffer(ulong start, ulong size)
{
	iotrace.start = start;
	iotrace.size = size;
	iotrace.offset = 0;
	iotrace.crc32 = 0;
}

تهی iotrace_get_buffer(ulong *start, ulong *size, ulong *needed_size, ulong *offset, ulong *شمار)
{
	*start = iotrace.start;
	*size = iotrace.size;
	*needed_size = iotrace.needed_size;
	*offset = iotrace.offset;
	*شمار = iotrace.offset / اندازه(ساختار iotrace_record);
}
