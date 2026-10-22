// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc.
 */

#परिभाषय IOTRACE_IMPL

#अन्तर्भाव <mapmem.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <asm/io.h>
#अन्तर्भाव <linux/bug.h>
#अन्तर्भाव <u-boot/crc.h>

DECLARE_GLOBAL_दत्त_PTR;

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
	ulong आकार;
	ulong needed_आकार;
	ulong विस्थापनम्;
	ulong region_start;
	ulong region_आकार;
	u32 चक्रीयपरीक्षा_32;
	bool enabled;
} iotrace;

स्थिर शून्य सङ्कलय_लेखः(पूर्णाङ्क flags, ध्रुव शून्य *ptr, ulong मानम्)
{
	संरचना iotrace_लेखः srec, *rec = &srec;

	/*
	 * We don't support iotrace before relocation. Since the trace buffer
	 * is set up by a command, it can't be enabled at present. To change
	 * this we would need to set the iotrace buffer at build-time. See
	 * lib/trace.c for how this might be done if you are interested.
	 */
	यदि (!(gd->flags & GD_FLG_RELOC) || !iotrace.enabled)
		प्रत्यागम;

	यदि (iotrace.region_आकार)
		यदि ((ulong)ptr < iotrace.region_start ||
		    (ulong)ptr > iotrace.region_start + iotrace.region_आकार)
			प्रत्यागम;

	/* Store it if there is room */
	यदि (iotrace.विस्थापनम् + परिमाण(*rec) < iotrace.आकार) {
		rec = (संरचना iotrace_लेखः *)map_sysmem(
					iotrace.start + iotrace.विस्थापनम्,
					परिमाण(मानम्));
	} अन्यथा {
		WARN_ONCE(1, "WARNING: iotrace buffer exhausted, please check needed length using \"iotrace stats\"\n");
		iotrace.needed_आकार += परिमाण(संरचना iotrace_लेखः);
		प्रत्यागम;
	}

	rec->timestamp = timer_get_us();
	rec->flags = flags;
	rec->addr = map_गन्तव्यम्_sysmem(ptr);
	rec->मानम् = मानम्;

	/* Update our checksum */
	iotrace.चक्रीयपरीक्षा_32 = चक्रीयपरीक्षा_32(iotrace.चक्रीयपरीक्षा_32, (अचिह्नित वर्ण *)rec,
			      परिमाण(*rec));

	iotrace.needed_आकार += परिमाण(संरचना iotrace_लेखः);
	iotrace.विस्थापनम् += परिमाण(संरचना iotrace_लेखः);
}

u32 iotrace_readl(ध्रुव शून्य *ptr)
{
	u32 v;

	v = readl(ptr);
	सङ्कलय_लेखः(IOT_32 | IOT_READ, ptr, v);

	प्रत्यागम v;
}

शून्य iotrace_writel(ulong मानम्, शून्य *ptr)
{
	सङ्कलय_लेखः(IOT_32 | IOT_WRITE, ptr, मानम्);
	writel(मानम्, ptr);
}

u16 iotrace_readw(ध्रुव शून्य *ptr)
{
	u32 v;

	v = readw(ptr);
	सङ्कलय_लेखः(IOT_16 | IOT_READ, ptr, v);

	प्रत्यागम v;
}

शून्य iotrace_writew(ulong मानम्, शून्य *ptr)
{
	सङ्कलय_लेखः(IOT_16 | IOT_WRITE, ptr, मानम्);
	writew(मानम्, ptr);
}

u8 iotrace_readb(ध्रुव शून्य *ptr)
{
	u32 v;

	v = readb(ptr);
	सङ्कलय_लेखः(IOT_8 | IOT_READ, ptr, v);

	प्रत्यागम v;
}

शून्य iotrace_writeb(ulong मानम्, शून्य *ptr)
{
	सङ्कलय_लेखः(IOT_8 | IOT_WRITE, ptr, मानम्);
	writeb(मानम्, ptr);
}

शून्य iotrace_reset_checksum(शून्य)
{
	iotrace.चक्रीयपरीक्षा_32 = 0;
}

u32 iotrace_get_checksum(शून्य)
{
	प्रत्यागम iotrace.चक्रीयपरीक्षा_32;
}

शून्य iotrace_समुच्चयः_region(ulong start, ulong आकार)
{
	iotrace.region_start = start;
	iotrace.region_आकार = आकार;
}

शून्य iotrace_reset_region(शून्य)
{
	iotrace.region_start = 0;
	iotrace.region_आकार = 0;
}

शून्य iotrace_get_region(ulong *start, ulong *आकार)
{
	*start = iotrace.region_start;
	*आकार = iotrace.region_आकार;
}

शून्य iotrace_समुच्चयः_enabled(पूर्णाङ्क enable)
{
	iotrace.enabled = enable;
}

पूर्णाङ्क iotrace_get_enabled(शून्य)
{
	प्रत्यागम iotrace.enabled;
}

शून्य iotrace_समुच्चयः_बफर(ulong start, ulong आकार)
{
	iotrace.start = start;
	iotrace.आकार = आकार;
	iotrace.विस्थापनम् = 0;
	iotrace.चक्रीयपरीक्षा_32 = 0;
}

शून्य iotrace_get_बफर(ulong *start, ulong *आकार, ulong *needed_आकार, ulong *विस्थापनम्, ulong *गणनम्)
{
	*start = iotrace.start;
	*आकार = iotrace.आकार;
	*needed_आकार = iotrace.needed_आकार;
	*विस्थापनम् = iotrace.विस्थापनम्;
	*गणनम् = iotrace.विस्थापनम् / परिमाण(संरचना iotrace_लेखः);
}
