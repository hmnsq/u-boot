// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc.
 */

#تعریف_کرو IOTRACE_IMPL

#ضمن <mapmem.h>
#ضمن <time.h>
#ضمن <asm/global_data.h>
#ضمن <asm/io.h>
#ضمن <linux/bug.h>
#ضمن <u-boot/crc.h>

DECLARE_GLOBAL_انکڑے_PTR;

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
	ulong ناپ;
	ulong needed_ناپ;
	ulong وتھ;
	ulong region_شروع;
	ulong region_ناپ;
	u32 چکری_جانچ_32;
	bool enabled;
} iotrace;

ساكن فراغ جوڑو_ریکارڈ(صحيح flags, ثابت فراغ *ptr, ulong قدر)
{
	بنية iotrace_ریکارڈ srec, *rec = &srec;

	/*
	 * We don't support iotrace before relocation. Since the trace buffer
	 * is set up by a command, it can't be enabled at present. To change
	 * this we would need to set the iotrace buffer at build-time. See
	 * lib/trace.c for how this might be done if you are interested.
	 */
	إذا (!(gd->flags & GD_FLG_RELOC) || !iotrace.enabled)
		أرجع;

	إذا (iotrace.region_ناپ)
		إذا ((ulong)ptr < iotrace.region_شروع ||
		    (ulong)ptr > iotrace.region_شروع + iotrace.region_ناپ)
			أرجع;

	/* Store it if there is room */
	إذا (iotrace.وتھ + حجم(*rec) < iotrace.ناپ) {
		rec = (بنية iotrace_ریکارڈ *)map_sysmem(
					iotrace.شروع + iotrace.وتھ,
					حجم(قدر));
	} وإلا {
		WARN_ONCE(1, "WARNING: iotrace buffer exhausted, please check needed length using \"iotrace stats\"\n");
		iotrace.needed_ناپ += حجم(بنية iotrace_ریکارڈ);
		أرجع;
	}

	rec->timestamp = timer_get_us();
	rec->flags = flags;
	rec->addr = map_منزل_sysmem(ptr);
	rec->قدر = قدر;

	/* Update our checksum */
	iotrace.چکری_جانچ_32 = چکری_جانچ_32(iotrace.چکری_جانچ_32, (بنانشانی حرف *)rec,
			      حجم(*rec));

	iotrace.needed_ناپ += حجم(بنية iotrace_ریکارڈ);
	iotrace.وتھ += حجم(بنية iotrace_ریکارڈ);
}

u32 iotrace_readl(ثابت فراغ *ptr)
{
	u32 v;

	v = readl(ptr);
	جوڑو_ریکارڈ(IOT_32 | IOT_READ, ptr, v);

	أرجع v;
}

فراغ iotrace_writel(ulong قدر, فراغ *ptr)
{
	جوڑو_ریکارڈ(IOT_32 | IOT_WRITE, ptr, قدر);
	writel(قدر, ptr);
}

u16 iotrace_readw(ثابت فراغ *ptr)
{
	u32 v;

	v = readw(ptr);
	جوڑو_ریکارڈ(IOT_16 | IOT_READ, ptr, v);

	أرجع v;
}

فراغ iotrace_writew(ulong قدر, فراغ *ptr)
{
	جوڑو_ریکارڈ(IOT_16 | IOT_WRITE, ptr, قدر);
	writew(قدر, ptr);
}

u8 iotrace_readb(ثابت فراغ *ptr)
{
	u32 v;

	v = readb(ptr);
	جوڑو_ریکارڈ(IOT_8 | IOT_READ, ptr, v);

	أرجع v;
}

فراغ iotrace_writeb(ulong قدر, فراغ *ptr)
{
	جوڑو_ریکارڈ(IOT_8 | IOT_WRITE, ptr, قدر);
	writeb(قدر, ptr);
}

فراغ iotrace_reset_checksum(فراغ)
{
	iotrace.چکری_جانچ_32 = 0;
}

u32 iotrace_get_checksum(فراغ)
{
	أرجع iotrace.چکری_جانچ_32;
}

فراغ iotrace_سیٹ_region(ulong شروع, ulong ناپ)
{
	iotrace.region_شروع = شروع;
	iotrace.region_ناپ = ناپ;
}

فراغ iotrace_reset_region(فراغ)
{
	iotrace.region_شروع = 0;
	iotrace.region_ناپ = 0;
}

فراغ iotrace_get_region(ulong *شروع, ulong *ناپ)
{
	*شروع = iotrace.region_شروع;
	*ناپ = iotrace.region_ناپ;
}

فراغ iotrace_سیٹ_enabled(صحيح enable)
{
	iotrace.enabled = enable;
}

صحيح iotrace_get_enabled(فراغ)
{
	أرجع iotrace.enabled;
}

فراغ iotrace_سیٹ_بفر(ulong شروع, ulong ناپ)
{
	iotrace.شروع = شروع;
	iotrace.ناپ = ناپ;
	iotrace.وتھ = 0;
	iotrace.چکری_جانچ_32 = 0;
}

فراغ iotrace_get_بفر(ulong *شروع, ulong *ناپ, ulong *needed_ناپ, ulong *وتھ, ulong *گنتی)
{
	*شروع = iotrace.شروع;
	*ناپ = iotrace.ناپ;
	*needed_ناپ = iotrace.needed_ناپ;
	*وتھ = iotrace.وتھ;
	*گنتی = iotrace.وتھ / حجم(بنية iotrace_ریکارڈ);
}
