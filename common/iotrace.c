// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc.
 */

#define IOTRACE_IMPL

#अन्तर्भाव <mapmem.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <asm/io.h>
#अन्तर्भाव <linux/bug.h>
#अन्तर्भाव <u-boot/crc.h>

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
स्थिर संरचना iotrace {
	ulong start;
	ulong size;
	ulong needed_size;
	ulong offset;
	ulong region_start;
	ulong region_size;
	u32 crc32;
	bool enabled;
} iotrace;

स्थिर शून्य add_record(पूर्णांक flags, स्थिरांक शून्य *ptr, ulong मूल्य)
{
	संरचना iotrace_record srec, *rec = &srec;

	/*
	 * We don't support iotrace before relocation. Since the trace buffer
	 * is set up by a command, it can't be enabled at present. To change
	 * this we would need to set the iotrace buffer at build-time. See
	 * lib/trace.c for how this might be done if you are interested.
	 */
	जर (!(gd->flags & GD_FLG_RELOC) || !iotrace.enabled)
		परत;

	जर (iotrace.region_size)
		जर ((ulong)ptr < iotrace.region_start ||
		    (ulong)ptr > iotrace.region_start + iotrace.region_size)
			परत;

	/* Store it if there is room */
	जर (iotrace.offset + आकार(*rec) < iotrace.size) {
		rec = (संरचना iotrace_record *)map_sysmem(
					iotrace.start + iotrace.offset,
					आकार(मूल्य));
	} अन्यथा {
		WARN_ONCE(1, "WARNING: iotrace buffer exhausted, please check needed length using \"iotrace stats\"\n");
		iotrace.needed_size += आकार(संरचना iotrace_record);
		परत;
	}

	rec->timestamp = timer_get_us();
	rec->flags = flags;
	rec->addr = map_to_sysmem(ptr);
	rec->मूल्य = मूल्य;

	/* Update our checksum */
	iotrace.crc32 = crc32(iotrace.crc32, (अचिन्हित अक्षर *)rec,
			      आकार(*rec));

	iotrace.needed_size += आकार(संरचना iotrace_record);
	iotrace.offset += आकार(संरचना iotrace_record);
}

u32 iotrace_readl(स्थिरांक शून्य *ptr)
{
	u32 v;

	v = readl(ptr);
	add_record(IOT_32 | IOT_READ, ptr, v);

	परत v;
}

शून्य iotrace_writel(ulong मूल्य, शून्य *ptr)
{
	add_record(IOT_32 | IOT_WRITE, ptr, मूल्य);
	writel(मूल्य, ptr);
}

u16 iotrace_readw(स्थिरांक शून्य *ptr)
{
	u32 v;

	v = readw(ptr);
	add_record(IOT_16 | IOT_READ, ptr, v);

	परत v;
}

शून्य iotrace_writew(ulong मूल्य, शून्य *ptr)
{
	add_record(IOT_16 | IOT_WRITE, ptr, मूल्य);
	writew(मूल्य, ptr);
}

u8 iotrace_readb(स्थिरांक शून्य *ptr)
{
	u32 v;

	v = readb(ptr);
	add_record(IOT_8 | IOT_READ, ptr, v);

	परत v;
}

शून्य iotrace_writeb(ulong मूल्य, शून्य *ptr)
{
	add_record(IOT_8 | IOT_WRITE, ptr, मूल्य);
	writeb(मूल्य, ptr);
}

शून्य iotrace_reset_checksum(शून्य)
{
	iotrace.crc32 = 0;
}

u32 iotrace_get_checksum(शून्य)
{
	परत iotrace.crc32;
}

शून्य iotrace_set_region(ulong start, ulong size)
{
	iotrace.region_start = start;
	iotrace.region_size = size;
}

शून्य iotrace_reset_region(शून्य)
{
	iotrace.region_start = 0;
	iotrace.region_size = 0;
}

शून्य iotrace_get_region(ulong *start, ulong *size)
{
	*start = iotrace.region_start;
	*size = iotrace.region_size;
}

शून्य iotrace_set_enabled(पूर्णांक enable)
{
	iotrace.enabled = enable;
}

पूर्णांक iotrace_get_enabled(शून्य)
{
	परत iotrace.enabled;
}

शून्य iotrace_set_buffer(ulong start, ulong size)
{
	iotrace.start = start;
	iotrace.size = size;
	iotrace.offset = 0;
	iotrace.crc32 = 0;
}

शून्य iotrace_get_buffer(ulong *start, ulong *size, ulong *needed_size, ulong *offset, ulong *मोजणी)
{
	*start = iotrace.start;
	*size = iotrace.size;
	*needed_size = iotrace.needed_size;
	*offset = iotrace.offset;
	*मोजणी = iotrace.offset / आकार(संरचना iotrace_record);
}
