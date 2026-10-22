// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc.
 */

#घोषणा IOTRACE_IMPL

#समावेश <mapmem.h>
#समावेश <time.h>
#समावेश <asm/global_data.h>
#समावेश <asm/io.h>
#समावेश <linux/bug.h>
#समावेश <u-boot/crc.h>

DECLARE_GLOBAL_तथ्य_PTR;

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
अटल काष्ठा iotrace {
	ulong start;
	ulong आकार;
	ulong needed_आकार;
	ulong offset;
	ulong region_start;
	ulong region_आकार;
	u32 crc32;
	bool enabled;
} iotrace;

अटल व्योम add_record(पूर्णांक flags, स्थिर व्योम *ptr, ulong मूल्य)
{
	काष्ठा iotrace_record srec, *rec = &srec;

	/*
	 * We don't support iotrace before relocation. Since the trace buffer
	 * is set up by a command, it can't be enabled at present. To change
	 * this we would need to set the iotrace buffer at build-time. See
	 * lib/trace.c for how this might be done if you are interested.
	 */
	अगर (!(gd->flags & GD_FLG_RELOC) || !iotrace.enabled)
		वापस;

	अगर (iotrace.region_आकार)
		अगर ((ulong)ptr < iotrace.region_start ||
		    (ulong)ptr > iotrace.region_start + iotrace.region_आकार)
			वापस;

	/* Store it if there is room */
	अगर (iotrace.offset + माप(*rec) < iotrace.आकार) {
		rec = (काष्ठा iotrace_record *)map_sysmem(
					iotrace.start + iotrace.offset,
					माप(मूल्य));
	} अन्यथा {
		WARN_ONCE(1, "WARNING: iotrace buffer exhausted, please check needed length using \"iotrace stats\"\n");
		iotrace.needed_आकार += माप(काष्ठा iotrace_record);
		वापस;
	}

	rec->timestamp = timer_get_us();
	rec->flags = flags;
	rec->addr = map_to_sysmem(ptr);
	rec->मूल्य = मूल्य;

	/* Update our checksum */
	iotrace.crc32 = crc32(iotrace.crc32, (अचिन्हित अक्षर *)rec,
			      माप(*rec));

	iotrace.needed_आकार += माप(काष्ठा iotrace_record);
	iotrace.offset += माप(काष्ठा iotrace_record);
}

u32 iotrace_readl(स्थिर व्योम *ptr)
{
	u32 v;

	v = readl(ptr);
	add_record(IOT_32 | IOT_READ, ptr, v);

	वापस v;
}

व्योम iotrace_writel(ulong मूल्य, व्योम *ptr)
{
	add_record(IOT_32 | IOT_WRITE, ptr, मूल्य);
	writel(मूल्य, ptr);
}

u16 iotrace_readw(स्थिर व्योम *ptr)
{
	u32 v;

	v = readw(ptr);
	add_record(IOT_16 | IOT_READ, ptr, v);

	वापस v;
}

व्योम iotrace_writew(ulong मूल्य, व्योम *ptr)
{
	add_record(IOT_16 | IOT_WRITE, ptr, मूल्य);
	writew(मूल्य, ptr);
}

u8 iotrace_readb(स्थिर व्योम *ptr)
{
	u32 v;

	v = readb(ptr);
	add_record(IOT_8 | IOT_READ, ptr, v);

	वापस v;
}

व्योम iotrace_writeb(ulong मूल्य, व्योम *ptr)
{
	add_record(IOT_8 | IOT_WRITE, ptr, मूल्य);
	writeb(मूल्य, ptr);
}

व्योम iotrace_reset_checksum(व्योम)
{
	iotrace.crc32 = 0;
}

u32 iotrace_get_checksum(व्योम)
{
	वापस iotrace.crc32;
}

व्योम iotrace_set_region(ulong start, ulong आकार)
{
	iotrace.region_start = start;
	iotrace.region_आकार = आकार;
}

व्योम iotrace_reset_region(व्योम)
{
	iotrace.region_start = 0;
	iotrace.region_आकार = 0;
}

व्योम iotrace_get_region(ulong *start, ulong *आकार)
{
	*start = iotrace.region_start;
	*आकार = iotrace.region_आकार;
}

व्योम iotrace_set_enabled(पूर्णांक enable)
{
	iotrace.enabled = enable;
}

पूर्णांक iotrace_get_enabled(व्योम)
{
	वापस iotrace.enabled;
}

व्योम iotrace_set_बफर(ulong start, ulong आकार)
{
	iotrace.start = start;
	iotrace.आकार = आकार;
	iotrace.offset = 0;
	iotrace.crc32 = 0;
}

व्योम iotrace_get_बफर(ulong *start, ulong *आकार, ulong *needed_आकार, ulong *offset, ulong *गिनती)
{
	*start = iotrace.start;
	*आकार = iotrace.आकार;
	*needed_आकार = iotrace.needed_आकार;
	*offset = iotrace.offset;
	*गिनती = iotrace.offset / माप(काष्ठा iotrace_record);
}
