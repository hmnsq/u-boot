// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc.
 */

#define IOTRACE_IMPL

#அந்தர்பாவ <mapmem.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <asm/global_data.h>
#அந்தர்பாவ <asm/io.h>
#அந்தர்பாவ <linux/bug.h>
#அந்தர்பாவ <u-boot/crc.h>

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
நிலையான கட்டமைப்பு iotrace {
	ulong start;
	ulong size;
	ulong needed_size;
	ulong offset;
	ulong region_start;
	ulong region_size;
	u32 crc32;
	bool enabled;
} iotrace;

நிலையான வெறுமை add_record(முழு flags, மாறிலி வெறுமை *ptr, ulong மதிப்பு)
{
	கட்டமைப்பு iotrace_record srec, *rec = &srec;

	/*
	 * We don't support iotrace before relocation. Since the trace buffer
	 * is set up by a command, it can't be enabled at present. To change
	 * this we would need to set the iotrace buffer at build-time. See
	 * lib/trace.c for how this might be done if you are interested.
	 */
	எனில் (!(gd->flags & GD_FLG_RELOC) || !iotrace.enabled)
		திரும்பு;

	எனில் (iotrace.region_size)
		எனில் ((ulong)ptr < iotrace.region_start ||
		    (ulong)ptr > iotrace.region_start + iotrace.region_size)
			திரும்பு;

	/* Store it if there is room */
	எனில் (iotrace.offset + அளவு(*rec) < iotrace.size) {
		rec = (கட்டமைப்பு iotrace_record *)map_sysmem(
					iotrace.start + iotrace.offset,
					அளவு(மதிப்பு));
	} இல்லையெனில் {
		WARN_ONCE(1, "WARNING: iotrace buffer exhausted, please check needed length using \"iotrace stats\"\n");
		iotrace.needed_size += அளவு(கட்டமைப்பு iotrace_record);
		திரும்பு;
	}

	rec->timestamp = timer_get_us();
	rec->flags = flags;
	rec->addr = map_to_sysmem(ptr);
	rec->மதிப்பு = மதிப்பு;

	/* Update our checksum */
	iotrace.crc32 = crc32(iotrace.crc32, (unsigned எழுத்து *)rec,
			      அளவு(*rec));

	iotrace.needed_size += அளவு(கட்டமைப்பு iotrace_record);
	iotrace.offset += அளவு(கட்டமைப்பு iotrace_record);
}

u32 iotrace_readl(மாறிலி வெறுமை *ptr)
{
	u32 v;

	v = readl(ptr);
	add_record(IOT_32 | IOT_READ, ptr, v);

	திரும்பு v;
}

வெறுமை iotrace_writel(ulong மதிப்பு, வெறுமை *ptr)
{
	add_record(IOT_32 | IOT_WRITE, ptr, மதிப்பு);
	writel(மதிப்பு, ptr);
}

u16 iotrace_readw(மாறிலி வெறுமை *ptr)
{
	u32 v;

	v = readw(ptr);
	add_record(IOT_16 | IOT_READ, ptr, v);

	திரும்பு v;
}

வெறுமை iotrace_writew(ulong மதிப்பு, வெறுமை *ptr)
{
	add_record(IOT_16 | IOT_WRITE, ptr, மதிப்பு);
	writew(மதிப்பு, ptr);
}

u8 iotrace_readb(மாறிலி வெறுமை *ptr)
{
	u32 v;

	v = readb(ptr);
	add_record(IOT_8 | IOT_READ, ptr, v);

	திரும்பு v;
}

வெறுமை iotrace_writeb(ulong மதிப்பு, வெறுமை *ptr)
{
	add_record(IOT_8 | IOT_WRITE, ptr, மதிப்பு);
	writeb(மதிப்பு, ptr);
}

வெறுமை iotrace_reset_checksum(வெறுமை)
{
	iotrace.crc32 = 0;
}

u32 iotrace_get_checksum(வெறுமை)
{
	திரும்பு iotrace.crc32;
}

வெறுமை iotrace_set_region(ulong start, ulong size)
{
	iotrace.region_start = start;
	iotrace.region_size = size;
}

வெறுமை iotrace_reset_region(வெறுமை)
{
	iotrace.region_start = 0;
	iotrace.region_size = 0;
}

வெறுமை iotrace_get_region(ulong *start, ulong *size)
{
	*start = iotrace.region_start;
	*size = iotrace.region_size;
}

வெறுமை iotrace_set_enabled(முழு enable)
{
	iotrace.enabled = enable;
}

முழு iotrace_get_enabled(வெறுமை)
{
	திரும்பு iotrace.enabled;
}

வெறுமை iotrace_set_buffer(ulong start, ulong size)
{
	iotrace.start = start;
	iotrace.size = size;
	iotrace.offset = 0;
	iotrace.crc32 = 0;
}

வெறுமை iotrace_get_buffer(ulong *start, ulong *size, ulong *needed_size, ulong *offset, ulong *மொத்தம்)
{
	*start = iotrace.start;
	*size = iotrace.size;
	*needed_size = iotrace.needed_size;
	*offset = iotrace.offset;
	*மொத்தம் = iotrace.offset / அளவு(கட்டமைப்பு iotrace_record);
}
