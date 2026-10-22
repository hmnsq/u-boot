// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc.
 */

#وصف_ڪريو IOTRACE_IMPL

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
	ulong شروعات;
	ulong سائيز;
	ulong needed_سائيز;
	ulong وٿي;
	ulong region_شروعات;
	ulong region_سائيز;
	u32 چڪري_جاچ_32;
	bool enabled;
} iotrace;

ساكن فراغ جوڙيو_رڪارڊ(صحيح flags, ثابت فراغ *ptr, ulong قدر)
{
	بنية iotrace_رڪارڊ srec, *rec = &srec;

	/*
	 * We don't support iotrace before relocation. Since the trace buffer
	 * is set up by a command, it can't be enabled at present. To change
	 * this we would need to set the iotrace buffer at build-time. See
	 * lib/trace.c for how this might be done if you are interested.
	 */
	إذا (!(gd->flags & GD_FLG_RELOC) || !iotrace.enabled)
		أرجع;

	إذا (iotrace.region_سائيز)
		إذا ((ulong)ptr < iotrace.region_شروعات ||
		    (ulong)ptr > iotrace.region_شروعات + iotrace.region_سائيز)
			أرجع;

	/* Store it if there is room */
	إذا (iotrace.وٿي + حجم(*rec) < iotrace.سائيز) {
		rec = (بنية iotrace_رڪارڊ *)map_sysmem(
					iotrace.شروعات + iotrace.وٿي,
					حجم(قدر));
	} وإلا {
		WARN_ONCE(1, "WARNING: iotrace buffer exhausted, please check needed length using \"iotrace stats\"\n");
		iotrace.needed_سائيز += حجم(بنية iotrace_رڪارڊ);
		أرجع;
	}

	rec->timestamp = timer_get_us();
	rec->flags = flags;
	rec->addr = map_منزل_sysmem(ptr);
	rec->قدر = قدر;

	/* Update our checksum */
	iotrace.چڪري_جاچ_32 = چڪري_جاچ_32(iotrace.چڪري_جاچ_32, (بنانشان حرف *)rec,
			      حجم(*rec));

	iotrace.needed_سائيز += حجم(بنية iotrace_رڪارڊ);
	iotrace.وٿي += حجم(بنية iotrace_رڪارڊ);
}

u32 iotrace_readl(ثابت فراغ *ptr)
{
	u32 v;

	v = readl(ptr);
	جوڙيو_رڪارڊ(IOT_32 | IOT_READ, ptr, v);

	أرجع v;
}

فراغ iotrace_writel(ulong قدر, فراغ *ptr)
{
	جوڙيو_رڪارڊ(IOT_32 | IOT_WRITE, ptr, قدر);
	writel(قدر, ptr);
}

u16 iotrace_readw(ثابت فراغ *ptr)
{
	u32 v;

	v = readw(ptr);
	جوڙيو_رڪارڊ(IOT_16 | IOT_READ, ptr, v);

	أرجع v;
}

فراغ iotrace_writew(ulong قدر, فراغ *ptr)
{
	جوڙيو_رڪارڊ(IOT_16 | IOT_WRITE, ptr, قدر);
	writew(قدر, ptr);
}

u8 iotrace_readb(ثابت فراغ *ptr)
{
	u32 v;

	v = readb(ptr);
	جوڙيو_رڪارڊ(IOT_8 | IOT_READ, ptr, v);

	أرجع v;
}

فراغ iotrace_writeb(ulong قدر, فراغ *ptr)
{
	جوڙيو_رڪارڊ(IOT_8 | IOT_WRITE, ptr, قدر);
	writeb(قدر, ptr);
}

فراغ iotrace_reset_checksum(فراغ)
{
	iotrace.چڪري_جاچ_32 = 0;
}

u32 iotrace_get_checksum(فراغ)
{
	أرجع iotrace.چڪري_جاچ_32;
}

فراغ iotrace_سيٽ_region(ulong شروعات, ulong سائيز)
{
	iotrace.region_شروعات = شروعات;
	iotrace.region_سائيز = سائيز;
}

فراغ iotrace_reset_region(فراغ)
{
	iotrace.region_شروعات = 0;
	iotrace.region_سائيز = 0;
}

فراغ iotrace_get_region(ulong *شروعات, ulong *سائيز)
{
	*شروعات = iotrace.region_شروعات;
	*سائيز = iotrace.region_سائيز;
}

فراغ iotrace_سيٽ_enabled(صحيح enable)
{
	iotrace.enabled = enable;
}

صحيح iotrace_get_enabled(فراغ)
{
	أرجع iotrace.enabled;
}

فراغ iotrace_سيٽ_بفر(ulong شروعات, ulong سائيز)
{
	iotrace.شروعات = شروعات;
	iotrace.سائيز = سائيز;
	iotrace.وٿي = 0;
	iotrace.چڪري_جاچ_32 = 0;
}

فراغ iotrace_get_بفر(ulong *شروعات, ulong *سائيز, ulong *needed_سائيز, ulong *وٿي, ulong *ڳڻپ)
{
	*شروعات = iotrace.شروعات;
	*سائيز = iotrace.سائيز;
	*needed_سائيز = iotrace.needed_سائيز;
	*وٿي = iotrace.وٿي;
	*ڳڻپ = iotrace.وٿي / حجم(بنية iotrace_رڪارڊ);
}
