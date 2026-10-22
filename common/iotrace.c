// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc.
 */

#تعریف_کړه IOTRACE_IMPL

#ضمن <mapmem.h>
#ضمن <time.h>
#ضمن <asm/global_data.h>
#ضمن <asm/io.h>
#ضمن <linux/bug.h>
#ضمن <u-boot/crc.h>

DECLARE_GLOBAL_معلومات_PTR;

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
	ulong پیل;
	ulong اندازه;
	ulong needed_اندازه;
	ulong دننه_واټن;
	ulong region_پیل;
	ulong region_اندازه;
	u32 دوراني_ازموینه_32;
	bool enabled;
} iotrace;

ساكن فراغ جمع_کړه_ریکارډ(صحيح flags, ثابت فراغ *ptr, ulong ارزښت)
{
	بنية iotrace_ریکارډ srec, *rec = &srec;

	/*
	 * We don't support iotrace before relocation. Since the trace buffer
	 * is set up by a command, it can't be enabled at present. To change
	 * this we would need to set the iotrace buffer at build-time. See
	 * lib/trace.c for how this might be done if you are interested.
	 */
	إذا (!(gd->flags & GD_FLG_RELOC) || !iotrace.enabled)
		أرجع;

	إذا (iotrace.region_اندازه)
		إذا ((ulong)ptr < iotrace.region_پیل ||
		    (ulong)ptr > iotrace.region_پیل + iotrace.region_اندازه)
			أرجع;

	/* Store it if there is room */
	إذا (iotrace.دننه_واټن + حجم(*rec) < iotrace.اندازه) {
		rec = (بنية iotrace_ریکارډ *)map_sysmem(
					iotrace.پیل + iotrace.دننه_واټن,
					حجم(ارزښت));
	} وإلا {
		WARN_ONCE(1, "WARNING: iotrace buffer exhausted, please check needed length using \"iotrace stats\"\n");
		iotrace.needed_اندازه += حجم(بنية iotrace_ریکارډ);
		أرجع;
	}

	rec->timestamp = timer_get_us();
	rec->flags = flags;
	rec->addr = map_منزل_sysmem(ptr);
	rec->ارزښت = ارزښت;

	/* Update our checksum */
	iotrace.دوراني_ازموینه_32 = دوراني_ازموینه_32(iotrace.دوراني_ازموینه_32, (بینښه حرف *)rec,
			      حجم(*rec));

	iotrace.needed_اندازه += حجم(بنية iotrace_ریکارډ);
	iotrace.دننه_واټن += حجم(بنية iotrace_ریکارډ);
}

u32 iotrace_readl(ثابت فراغ *ptr)
{
	u32 v;

	v = readl(ptr);
	جمع_کړه_ریکارډ(IOT_32 | IOT_READ, ptr, v);

	أرجع v;
}

فراغ iotrace_writel(ulong ارزښت, فراغ *ptr)
{
	جمع_کړه_ریکارډ(IOT_32 | IOT_WRITE, ptr, ارزښت);
	writel(ارزښت, ptr);
}

u16 iotrace_readw(ثابت فراغ *ptr)
{
	u32 v;

	v = readw(ptr);
	جمع_کړه_ریکارډ(IOT_16 | IOT_READ, ptr, v);

	أرجع v;
}

فراغ iotrace_writew(ulong ارزښت, فراغ *ptr)
{
	جمع_کړه_ریکارډ(IOT_16 | IOT_WRITE, ptr, ارزښت);
	writew(ارزښت, ptr);
}

u8 iotrace_readb(ثابت فراغ *ptr)
{
	u32 v;

	v = readb(ptr);
	جمع_کړه_ریکارډ(IOT_8 | IOT_READ, ptr, v);

	أرجع v;
}

فراغ iotrace_writeb(ulong ارزښت, فراغ *ptr)
{
	جمع_کړه_ریکارډ(IOT_8 | IOT_WRITE, ptr, ارزښت);
	writeb(ارزښت, ptr);
}

فراغ iotrace_reset_checksum(فراغ)
{
	iotrace.دوراني_ازموینه_32 = 0;
}

u32 iotrace_get_checksum(فراغ)
{
	أرجع iotrace.دوراني_ازموینه_32;
}

فراغ iotrace_سېټ_region(ulong پیل, ulong اندازه)
{
	iotrace.region_پیل = پیل;
	iotrace.region_اندازه = اندازه;
}

فراغ iotrace_reset_region(فراغ)
{
	iotrace.region_پیل = 0;
	iotrace.region_اندازه = 0;
}

فراغ iotrace_get_region(ulong *پیل, ulong *اندازه)
{
	*پیل = iotrace.region_پیل;
	*اندازه = iotrace.region_اندازه;
}

فراغ iotrace_سېټ_enabled(صحيح enable)
{
	iotrace.enabled = enable;
}

صحيح iotrace_get_enabled(فراغ)
{
	أرجع iotrace.enabled;
}

فراغ iotrace_سېټ_بفر(ulong پیل, ulong اندازه)
{
	iotrace.پیل = پیل;
	iotrace.اندازه = اندازه;
	iotrace.دننه_واټن = 0;
	iotrace.دوراني_ازموینه_32 = 0;
}

فراغ iotrace_get_بفر(ulong *پیل, ulong *اندازه, ulong *needed_اندازه, ulong *دننه_واټن, ulong *شمېر)
{
	*پیل = iotrace.پیل;
	*اندازه = iotrace.اندازه;
	*needed_اندازه = iotrace.needed_اندازه;
	*دننه_واټن = iotrace.دننه_واټن;
	*شمېر = iotrace.دننه_واټن / حجم(بنية iotrace_ریکارډ);
}
