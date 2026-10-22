// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc.
 */

#تعریف_کن IOTRACE_IMPL

#ضمن <mapmem.h>
#ضمن <time.h>
#ضمن <asm/global_data.h>
#ضمن <asm/io.h>
#ضمن <linux/bug.h>
#ضمن <u-boot/crc.h>

DECLARE_GLOBAL_داده_PTR;

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
	ulong شروع;
	ulong اندازه;
	ulong needed_اندازه;
	ulong جابه_جایی;
	ulong region_شروع;
	ulong region_اندازه;
	u32 بررسی_چرخهای_32;
	bool enabled;
} iotrace;

ساكن فراغ جمع_کن_رکورد(صحيح flags, ثابت فراغ *ptr, ulong مقدار)
{
	بنية iotrace_رکورد srec, *rec = &srec;

	/*
	 * We don't support iotrace before relocation. Since the trace buffer
	 * is set up by a command, it can't be enabled at present. To change
	 * this we would need to set the iotrace buffer at build-time. See
	 * lib/trace.c for how this might be done if you are interested.
	 */
	إذا (!(gd->flags & GD_FLG_RELOC) || !iotrace.enabled)
		أرجع;

	إذا (iotrace.region_اندازه)
		إذا ((ulong)ptr < iotrace.region_شروع ||
		    (ulong)ptr > iotrace.region_شروع + iotrace.region_اندازه)
			أرجع;

	/* Store it if there is room */
	إذا (iotrace.جابه_جایی + حجم(*rec) < iotrace.اندازه) {
		rec = (بنية iotrace_رکورد *)map_sysmem(
					iotrace.شروع + iotrace.جابه_جایی,
					حجم(مقدار));
	} وإلا {
		WARN_ONCE(1, "WARNING: iotrace buffer exhausted, please check needed length using \"iotrace stats\"\n");
		iotrace.needed_اندازه += حجم(بنية iotrace_رکورد);
		أرجع;
	}

	rec->timestamp = timer_get_us();
	rec->flags = flags;
	rec->addr = map_مقصد_sysmem(ptr);
	rec->مقدار = مقدار;

	/* Update our checksum */
	iotrace.بررسی_چرخهای_32 = بررسی_چرخهای_32(iotrace.بررسی_چرخهای_32, (بیعلامت حرف *)rec,
			      حجم(*rec));

	iotrace.needed_اندازه += حجم(بنية iotrace_رکورد);
	iotrace.جابه_جایی += حجم(بنية iotrace_رکورد);
}

u32 iotrace_readl(ثابت فراغ *ptr)
{
	u32 v;

	v = readl(ptr);
	جمع_کن_رکورد(IOT_32 | IOT_READ, ptr, v);

	أرجع v;
}

فراغ iotrace_writel(ulong مقدار, فراغ *ptr)
{
	جمع_کن_رکورد(IOT_32 | IOT_WRITE, ptr, مقدار);
	writel(مقدار, ptr);
}

u16 iotrace_readw(ثابت فراغ *ptr)
{
	u32 v;

	v = readw(ptr);
	جمع_کن_رکورد(IOT_16 | IOT_READ, ptr, v);

	أرجع v;
}

فراغ iotrace_writew(ulong مقدار, فراغ *ptr)
{
	جمع_کن_رکورد(IOT_16 | IOT_WRITE, ptr, مقدار);
	writew(مقدار, ptr);
}

u8 iotrace_readb(ثابت فراغ *ptr)
{
	u32 v;

	v = readb(ptr);
	جمع_کن_رکورد(IOT_8 | IOT_READ, ptr, v);

	أرجع v;
}

فراغ iotrace_writeb(ulong مقدار, فراغ *ptr)
{
	جمع_کن_رکورد(IOT_8 | IOT_WRITE, ptr, مقدار);
	writeb(مقدار, ptr);
}

فراغ iotrace_reset_checksum(فراغ)
{
	iotrace.بررسی_چرخهای_32 = 0;
}

u32 iotrace_get_checksum(فراغ)
{
	أرجع iotrace.بررسی_چرخهای_32;
}

فراغ iotrace_مجموعه_region(ulong شروع, ulong اندازه)
{
	iotrace.region_شروع = شروع;
	iotrace.region_اندازه = اندازه;
}

فراغ iotrace_reset_region(فراغ)
{
	iotrace.region_شروع = 0;
	iotrace.region_اندازه = 0;
}

فراغ iotrace_get_region(ulong *شروع, ulong *اندازه)
{
	*شروع = iotrace.region_شروع;
	*اندازه = iotrace.region_اندازه;
}

فراغ iotrace_مجموعه_enabled(صحيح enable)
{
	iotrace.enabled = enable;
}

صحيح iotrace_get_enabled(فراغ)
{
	أرجع iotrace.enabled;
}

فراغ iotrace_مجموعه_میانگیر(ulong شروع, ulong اندازه)
{
	iotrace.شروع = شروع;
	iotrace.اندازه = اندازه;
	iotrace.جابه_جایی = 0;
	iotrace.بررسی_چرخهای_32 = 0;
}

فراغ iotrace_get_میانگیر(ulong *شروع, ulong *اندازه, ulong *needed_اندازه, ulong *جابه_جایی, ulong *شمار)
{
	*شروع = iotrace.شروع;
	*اندازه = iotrace.اندازه;
	*needed_اندازه = iotrace.needed_اندازه;
	*جابه_جایی = iotrace.جابه_جایی;
	*شمار = iotrace.جابه_جایی / حجم(بنية iotrace_رکورد);
}
