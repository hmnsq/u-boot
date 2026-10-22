// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc.
 */

#define IOTRACE_IMPL

#અન્તર્ભાવ <mapmem.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <asm/global_data.h>
#અન્તર્ભાવ <asm/io.h>
#અન્તર્ભાવ <linux/bug.h>
#અન્તર્ભાવ <u-boot/crc.h>

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
સ્થિર સંરચના iotrace {
	ulong start;
	ulong size;
	ulong needed_size;
	ulong offset;
	ulong region_start;
	ulong region_size;
	u32 crc32;
	bool enabled;
} iotrace;

સ્થિર શૂન્ય add_record(પૂર્ણાંક flags, અચલ શૂન્ય *ptr, ulong કિંમત)
{
	સંરચના iotrace_record srec, *rec = &srec;

	/*
	 * We don't support iotrace before relocation. Since the trace buffer
	 * is set up by a command, it can't be enabled at present. To change
	 * this we would need to set the iotrace buffer at build-time. See
	 * lib/trace.c for how this might be done if you are interested.
	 */
	જો (!(gd->flags & GD_FLG_RELOC) || !iotrace.enabled)
		પાછા;

	જો (iotrace.region_size)
		જો ((ulong)ptr < iotrace.region_start ||
		    (ulong)ptr > iotrace.region_start + iotrace.region_size)
			પાછા;

	/* Store it if there is room */
	જો (iotrace.offset + કદ(*rec) < iotrace.size) {
		rec = (સંરચના iotrace_record *)map_sysmem(
					iotrace.start + iotrace.offset,
					કદ(કિંમત));
	} નહીંતર {
		WARN_ONCE(1, "WARNING: iotrace buffer exhausted, please check needed length using \"iotrace stats\"\n");
		iotrace.needed_size += કદ(સંરચના iotrace_record);
		પાછા;
	}

	rec->timestamp = timer_get_us();
	rec->flags = flags;
	rec->addr = map_to_sysmem(ptr);
	rec->કિંમત = કિંમત;

	/* Update our checksum */
	iotrace.crc32 = crc32(iotrace.crc32, (unsigned અક્ષર *)rec,
			      કદ(*rec));

	iotrace.needed_size += કદ(સંરચના iotrace_record);
	iotrace.offset += કદ(સંરચના iotrace_record);
}

u32 iotrace_readl(અચલ શૂન્ય *ptr)
{
	u32 v;

	v = readl(ptr);
	add_record(IOT_32 | IOT_READ, ptr, v);

	પાછા v;
}

શૂન્ય iotrace_writel(ulong કિંમત, શૂન્ય *ptr)
{
	add_record(IOT_32 | IOT_WRITE, ptr, કિંમત);
	writel(કિંમત, ptr);
}

u16 iotrace_readw(અચલ શૂન્ય *ptr)
{
	u32 v;

	v = readw(ptr);
	add_record(IOT_16 | IOT_READ, ptr, v);

	પાછા v;
}

શૂન્ય iotrace_writew(ulong કિંમત, શૂન્ય *ptr)
{
	add_record(IOT_16 | IOT_WRITE, ptr, કિંમત);
	writew(કિંમત, ptr);
}

u8 iotrace_readb(અચલ શૂન્ય *ptr)
{
	u32 v;

	v = readb(ptr);
	add_record(IOT_8 | IOT_READ, ptr, v);

	પાછા v;
}

શૂન્ય iotrace_writeb(ulong કિંમત, શૂન્ય *ptr)
{
	add_record(IOT_8 | IOT_WRITE, ptr, કિંમત);
	writeb(કિંમત, ptr);
}

શૂન્ય iotrace_reset_checksum(શૂન્ય)
{
	iotrace.crc32 = 0;
}

u32 iotrace_get_checksum(શૂન્ય)
{
	પાછા iotrace.crc32;
}

શૂન્ય iotrace_set_region(ulong start, ulong size)
{
	iotrace.region_start = start;
	iotrace.region_size = size;
}

શૂન્ય iotrace_reset_region(શૂન્ય)
{
	iotrace.region_start = 0;
	iotrace.region_size = 0;
}

શૂન્ય iotrace_get_region(ulong *start, ulong *size)
{
	*start = iotrace.region_start;
	*size = iotrace.region_size;
}

શૂન્ય iotrace_set_enabled(પૂર્ણાંક enable)
{
	iotrace.enabled = enable;
}

પૂર્ણાંક iotrace_get_enabled(શૂન્ય)
{
	પાછા iotrace.enabled;
}

શૂન્ય iotrace_set_buffer(ulong start, ulong size)
{
	iotrace.start = start;
	iotrace.size = size;
	iotrace.offset = 0;
	iotrace.crc32 = 0;
}

શૂન્ય iotrace_get_buffer(ulong *start, ulong *size, ulong *needed_size, ulong *offset, ulong *ગણતરી)
{
	*start = iotrace.start;
	*size = iotrace.size;
	*needed_size = iotrace.needed_size;
	*offset = iotrace.offset;
	*ગણતરી = iotrace.offset / કદ(સંરચના iotrace_record);
}
