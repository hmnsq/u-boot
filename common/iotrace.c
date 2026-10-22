// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc.
 */

#define IOTRACE_IMPL

#ଅନ୍ତର୍ଭାଵ <mapmem.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>
#ଅନ୍ତର୍ଭାଵ <asm/io.h>
#ଅନ୍ତର୍ଭାଵ <linux/bug.h>
#ଅନ୍ତର୍ଭାଵ <u-boot/crc.h>

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
ସ୍ଥିର ସଂରଚନା iotrace {
	ulong start;
	ulong size;
	ulong needed_size;
	ulong offset;
	ulong region_start;
	ulong region_size;
	u32 crc32;
	bool enabled;
} iotrace;

ସ୍ଥିର ଶୂନ୍ୟ add_record(ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ଅଚଳ ଶୂନ୍ୟ *ptr, ulong ମୂଲ୍ୟ)
{
	ସଂରଚନା iotrace_record srec, *rec = &srec;

	/*
	 * We don't support iotrace before relocation. Since the trace buffer
	 * is set up by a command, it can't be enabled at present. To change
	 * this we would need to set the iotrace buffer at build-time. See
	 * lib/trace.c for how this might be done if you are interested.
	 */
	ଯଦି (!(gd->flags & GD_FLG_RELOC) || !iotrace.enabled)
		ଫେରନ୍ତୁ;

	ଯଦି (iotrace.region_size)
		ଯଦି ((ulong)ptr < iotrace.region_start ||
		    (ulong)ptr > iotrace.region_start + iotrace.region_size)
			ଫେରନ୍ତୁ;

	/* Store it if there is room */
	ଯଦି (iotrace.offset + ଆକାର(*rec) < iotrace.size) {
		rec = (ସଂରଚନା iotrace_record *)map_sysmem(
					iotrace.start + iotrace.offset,
					ଆକାର(ମୂଲ୍ୟ));
	} ଅନ୍ୟଥା {
		WARN_ONCE(1, "WARNING: iotrace buffer exhausted, please check needed length using \"iotrace stats\"\n");
		iotrace.needed_size += ଆକାର(ସଂରଚନା iotrace_record);
		ଫେରନ୍ତୁ;
	}

	rec->timestamp = timer_get_us();
	rec->flags = flags;
	rec->addr = map_to_sysmem(ptr);
	rec->ମୂଲ୍ୟ = ମୂଲ୍ୟ;

	/* Update our checksum */
	iotrace.crc32 = crc32(iotrace.crc32, (unsigned ଅକ୍ଷର *)rec,
			      ଆକାର(*rec));

	iotrace.needed_size += ଆକାର(ସଂରଚନା iotrace_record);
	iotrace.offset += ଆକାର(ସଂରଚନା iotrace_record);
}

u32 iotrace_readl(ଅଚଳ ଶୂନ୍ୟ *ptr)
{
	u32 v;

	v = readl(ptr);
	add_record(IOT_32 | IOT_READ, ptr, v);

	ଫେରନ୍ତୁ v;
}

ଶୂନ୍ୟ iotrace_writel(ulong ମୂଲ୍ୟ, ଶୂନ୍ୟ *ptr)
{
	add_record(IOT_32 | IOT_WRITE, ptr, ମୂଲ୍ୟ);
	writel(ମୂଲ୍ୟ, ptr);
}

u16 iotrace_readw(ଅଚଳ ଶୂନ୍ୟ *ptr)
{
	u32 v;

	v = readw(ptr);
	add_record(IOT_16 | IOT_READ, ptr, v);

	ଫେରନ୍ତୁ v;
}

ଶୂନ୍ୟ iotrace_writew(ulong ମୂଲ୍ୟ, ଶୂନ୍ୟ *ptr)
{
	add_record(IOT_16 | IOT_WRITE, ptr, ମୂଲ୍ୟ);
	writew(ମୂଲ୍ୟ, ptr);
}

u8 iotrace_readb(ଅଚଳ ଶୂନ୍ୟ *ptr)
{
	u32 v;

	v = readb(ptr);
	add_record(IOT_8 | IOT_READ, ptr, v);

	ଫେରନ୍ତୁ v;
}

ଶୂନ୍ୟ iotrace_writeb(ulong ମୂଲ୍ୟ, ଶୂନ୍ୟ *ptr)
{
	add_record(IOT_8 | IOT_WRITE, ptr, ମୂଲ୍ୟ);
	writeb(ମୂଲ୍ୟ, ptr);
}

ଶୂନ୍ୟ iotrace_reset_checksum(ଶୂନ୍ୟ)
{
	iotrace.crc32 = 0;
}

u32 iotrace_get_checksum(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ iotrace.crc32;
}

ଶୂନ୍ୟ iotrace_set_region(ulong start, ulong size)
{
	iotrace.region_start = start;
	iotrace.region_size = size;
}

ଶୂନ୍ୟ iotrace_reset_region(ଶୂନ୍ୟ)
{
	iotrace.region_start = 0;
	iotrace.region_size = 0;
}

ଶୂନ୍ୟ iotrace_get_region(ulong *start, ulong *size)
{
	*start = iotrace.region_start;
	*size = iotrace.region_size;
}

ଶୂନ୍ୟ iotrace_set_enabled(ପୂର୍ଣ୍ଣସଂଖ୍ୟା enable)
{
	iotrace.enabled = enable;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା iotrace_get_enabled(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ iotrace.enabled;
}

ଶୂନ୍ୟ iotrace_set_buffer(ulong start, ulong size)
{
	iotrace.start = start;
	iotrace.size = size;
	iotrace.offset = 0;
	iotrace.crc32 = 0;
}

ଶୂନ୍ୟ iotrace_get_buffer(ulong *start, ulong *size, ulong *needed_size, ulong *offset, ulong *ଗଣନା_2)
{
	*start = iotrace.start;
	*size = iotrace.size;
	*needed_size = iotrace.needed_size;
	*offset = iotrace.offset;
	*ଗଣନା_2 = iotrace.offset / ଆକାର(ସଂରଚନା iotrace_record);
}
