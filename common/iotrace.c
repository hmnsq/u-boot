// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc.
 */

#define IOTRACE_IMPL

#അന്തര്ഭാവ <mapmem.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <asm/global_data.h>
#അന്തര്ഭാവ <asm/io.h>
#അന്തര്ഭാവ <linux/bug.h>
#അന്തര്ഭാവ <u-boot/crc.h>

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
സ്ഥിതം ഘടന iotrace {
	ulong start;
	ulong size;
	ulong needed_size;
	ulong offset;
	ulong region_start;
	ulong region_size;
	u32 crc32;
	bool enabled;
} iotrace;

സ്ഥിതം ശൂന്യം add_record(പൂർണ്ണസംഖ്യ flags, സ്ഥിരം ശൂന്യം *ptr, ulong മൂല്യം)
{
	ഘടന iotrace_record srec, *rec = &srec;

	/*
	 * We don't support iotrace before relocation. Since the trace buffer
	 * is set up by a command, it can't be enabled at present. To change
	 * this we would need to set the iotrace buffer at build-time. See
	 * lib/trace.c for how this might be done if you are interested.
	 */
	എങ്കിൽ (!(gd->flags & GD_FLG_RELOC) || !iotrace.enabled)
		തിരികെ;

	എങ്കിൽ (iotrace.region_size)
		എങ്കിൽ ((ulong)ptr < iotrace.region_start ||
		    (ulong)ptr > iotrace.region_start + iotrace.region_size)
			തിരികെ;

	/* Store it if there is room */
	എങ്കിൽ (iotrace.offset + വലുപ്പം(*rec) < iotrace.size) {
		rec = (ഘടന iotrace_record *)map_sysmem(
					iotrace.start + iotrace.offset,
					വലുപ്പം(മൂല്യം));
	} അല്ലെങ്കിൽ {
		WARN_ONCE(1, "WARNING: iotrace buffer exhausted, please check needed length using \"iotrace stats\"\n");
		iotrace.needed_size += വലുപ്പം(ഘടന iotrace_record);
		തിരികെ;
	}

	rec->timestamp = timer_get_us();
	rec->flags = flags;
	rec->addr = map_to_sysmem(ptr);
	rec->മൂല്യം = മൂല്യം;

	/* Update our checksum */
	iotrace.crc32 = crc32(iotrace.crc32, (unsigned അക്ഷരം *)rec,
			      വലുപ്പം(*rec));

	iotrace.needed_size += വലുപ്പം(ഘടന iotrace_record);
	iotrace.offset += വലുപ്പം(ഘടന iotrace_record);
}

u32 iotrace_readl(സ്ഥിരം ശൂന്യം *ptr)
{
	u32 v;

	v = readl(ptr);
	add_record(IOT_32 | IOT_READ, ptr, v);

	തിരികെ v;
}

ശൂന്യം iotrace_writel(ulong മൂല്യം, ശൂന്യം *ptr)
{
	add_record(IOT_32 | IOT_WRITE, ptr, മൂല്യം);
	writel(മൂല്യം, ptr);
}

u16 iotrace_readw(സ്ഥിരം ശൂന്യം *ptr)
{
	u32 v;

	v = readw(ptr);
	add_record(IOT_16 | IOT_READ, ptr, v);

	തിരികെ v;
}

ശൂന്യം iotrace_writew(ulong മൂല്യം, ശൂന്യം *ptr)
{
	add_record(IOT_16 | IOT_WRITE, ptr, മൂല്യം);
	writew(മൂല്യം, ptr);
}

u8 iotrace_readb(സ്ഥിരം ശൂന്യം *ptr)
{
	u32 v;

	v = readb(ptr);
	add_record(IOT_8 | IOT_READ, ptr, v);

	തിരികെ v;
}

ശൂന്യം iotrace_writeb(ulong മൂല്യം, ശൂന്യം *ptr)
{
	add_record(IOT_8 | IOT_WRITE, ptr, മൂല്യം);
	writeb(മൂല്യം, ptr);
}

ശൂന്യം iotrace_reset_checksum(ശൂന്യം)
{
	iotrace.crc32 = 0;
}

u32 iotrace_get_checksum(ശൂന്യം)
{
	തിരികെ iotrace.crc32;
}

ശൂന്യം iotrace_set_region(ulong start, ulong size)
{
	iotrace.region_start = start;
	iotrace.region_size = size;
}

ശൂന്യം iotrace_reset_region(ശൂന്യം)
{
	iotrace.region_start = 0;
	iotrace.region_size = 0;
}

ശൂന്യം iotrace_get_region(ulong *start, ulong *size)
{
	*start = iotrace.region_start;
	*size = iotrace.region_size;
}

ശൂന്യം iotrace_set_enabled(പൂർണ്ണസംഖ്യ enable)
{
	iotrace.enabled = enable;
}

പൂർണ്ണസംഖ്യ iotrace_get_enabled(ശൂന്യം)
{
	തിരികെ iotrace.enabled;
}

ശൂന്യം iotrace_set_buffer(ulong start, ulong size)
{
	iotrace.start = start;
	iotrace.size = size;
	iotrace.offset = 0;
	iotrace.crc32 = 0;
}

ശൂന്യം iotrace_get_buffer(ulong *start, ulong *size, ulong *needed_size, ulong *offset, ulong *എണ്ണം)
{
	*start = iotrace.start;
	*size = iotrace.size;
	*needed_size = iotrace.needed_size;
	*offset = iotrace.offset;
	*എണ്ണം = iotrace.offset / വലുപ്പം(ഘടന iotrace_record);
}
