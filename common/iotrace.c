// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc.
 */

#define IOTRACE_IMPL

#ضمن <mapmem.h>
#ضمن <time.h>
#ضمن <asm/global_data.h>
#ضمن <asm/io.h>
#ضمن <linux/bug.h>
#ضمن <u-boot/crc.h>

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
ساكن بنية iotrace {
	ulong start;
	ulong size;
	ulong needed_size;
	ulong offset;
	ulong region_start;
	ulong region_size;
	u32 crc32;
	bool enabled;
} iotrace;

ساكن فراغ add_record(صحيح flags, ثابت فراغ *ptr, ulong value)
{
	بنية iotrace_record srec, *rec = &srec;

	/*
	 * We don't support iotrace before relocation. Since the trace buffer
	 * is set up by a command, it can't be enabled at present. To change
	 * this we would need to set the iotrace buffer at build-time. See
	 * lib/trace.c for how this might be done if you are interested.
	 */
	𐡄𐡍 (!(gd->flags & GD_FLG_RELOC) || !iotrace.enabled)
		أرجع;

	𐡄𐡍 (iotrace.region_size)
		𐡄𐡍 ((ulong)ptr < iotrace.region_start ||
		    (ulong)ptr > iotrace.region_start + iotrace.region_size)
			أرجع;

	/* Store it if there is room */
	𐡄𐡍 (iotrace.offset + حجم(*rec) < iotrace.size) {
		rec = (بنية iotrace_record *)map_sysmem(
					iotrace.start + iotrace.offset,
					حجم(value));
	} وإلا {
		WARN_ONCE(1, "WARNING: iotrace buffer exhausted, please check needed length using \"iotrace stats\"\n");
		iotrace.needed_size += حجم(بنية iotrace_record);
		أرجع;
	}

	rec->timestamp = timer_get_us();
	rec->flags = flags;
	rec->addr = map_to_sysmem(ptr);
	rec->value = value;

	/* Update our checksum */
	iotrace.crc32 = crc32(iotrace.crc32, (unsigned حرف *)rec,
			      حجم(*rec));

	iotrace.needed_size += حجم(بنية iotrace_record);
	iotrace.offset += حجم(بنية iotrace_record);
}

u32 iotrace_readl(ثابت فراغ *ptr)
{
	u32 v;

	v = readl(ptr);
	add_record(IOT_32 | IOT_READ, ptr, v);

	أرجع v;
}

فراغ iotrace_writel(ulong value, فراغ *ptr)
{
	add_record(IOT_32 | IOT_WRITE, ptr, value);
	writel(value, ptr);
}

u16 iotrace_readw(ثابت فراغ *ptr)
{
	u32 v;

	v = readw(ptr);
	add_record(IOT_16 | IOT_READ, ptr, v);

	أرجع v;
}

فراغ iotrace_writew(ulong value, فراغ *ptr)
{
	add_record(IOT_16 | IOT_WRITE, ptr, value);
	writew(value, ptr);
}

u8 iotrace_readb(ثابت فراغ *ptr)
{
	u32 v;

	v = readb(ptr);
	add_record(IOT_8 | IOT_READ, ptr, v);

	أرجع v;
}

فراغ iotrace_writeb(ulong value, فراغ *ptr)
{
	add_record(IOT_8 | IOT_WRITE, ptr, value);
	writeb(value, ptr);
}

فراغ iotrace_reset_checksum(فراغ)
{
	iotrace.crc32 = 0;
}

u32 iotrace_get_checksum(فراغ)
{
	أرجع iotrace.crc32;
}

فراغ iotrace_set_region(ulong start, ulong size)
{
	iotrace.region_start = start;
	iotrace.region_size = size;
}

فراغ iotrace_reset_region(فراغ)
{
	iotrace.region_start = 0;
	iotrace.region_size = 0;
}

فراغ iotrace_get_region(ulong *start, ulong *size)
{
	*start = iotrace.region_start;
	*size = iotrace.region_size;
}

فراغ iotrace_set_enabled(صحيح enable)
{
	iotrace.enabled = enable;
}

صحيح iotrace_get_enabled(فراغ)
{
	أرجع iotrace.enabled;
}

فراغ iotrace_set_buffer(ulong start, ulong size)
{
	iotrace.start = start;
	iotrace.size = size;
	iotrace.offset = 0;
	iotrace.crc32 = 0;
}

فراغ iotrace_get_buffer(ulong *start, ulong *size, ulong *needed_size, ulong *offset, ulong *count)
{
	*start = iotrace.start;
	*size = iotrace.size;
	*needed_size = iotrace.needed_size;
	*offset = iotrace.offset;
	*count = iotrace.offset / حجم(بنية iotrace_record);
}
