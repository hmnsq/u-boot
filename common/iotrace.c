// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc.
 */

#تعریف_کرو IOTRACE_IMPL

#شامل <mapmem.h>
#شامل <time.h>
#شامل <asm/global_data.h>
#شامل <asm/io.h>
#شامل <linux/bug.h>
#شامل <u-boot/crc.h>

DECLARE_GLOBAL_ڈیٹا_PTR;

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
ساکن ساخت iotrace {
	ulong آغاز;
	ulong سائز;
	ulong needed_سائز;
	ulong انحراف;
	ulong region_آغاز;
	ulong region_سائز;
	u32 چکری_جانچ_32;
	bool enabled;
} iotrace;

ساکن خالی جوڑو_ریکارڈ(صحیح flags, ثابت خالی *ptr, ulong قدر)
{
	ساخت iotrace_ریکارڈ srec, *rec = &srec;

	/*
	 * We don't support iotrace before relocation. Since the trace buffer
	 * is set up by a command, it can't be enabled at present. To change
	 * this we would need to set the iotrace buffer at build-time. See
	 * lib/trace.c for how this might be done if you are interested.
	 */
	اگر (!(gd->flags & GD_FLG_RELOC) || !iotrace.enabled)
		واپس;

	اگر (iotrace.region_سائز)
		اگر ((ulong)ptr < iotrace.region_آغاز ||
		    (ulong)ptr > iotrace.region_آغاز + iotrace.region_سائز)
			واپس;

	/* Store it if there is room */
	اگر (iotrace.انحراف + حجم(*rec) < iotrace.سائز) {
		rec = (ساخت iotrace_ریکارڈ *)map_sysmem(
					iotrace.آغاز + iotrace.انحراف,
					حجم(قدر));
	} ورنہ {
		WARN_ONCE(1, "WARNING: iotrace buffer exhausted, please check needed length using \"iotrace stats\"\n");
		iotrace.needed_سائز += حجم(ساخت iotrace_ریکارڈ);
		واپس;
	}

	rec->timestamp = timer_get_us();
	rec->flags = flags;
	rec->addr = map_مقصد_sysmem(ptr);
	rec->قدر = قدر;

	/* Update our checksum */
	iotrace.چکری_جانچ_32 = چکری_جانچ_32(iotrace.چکری_جانچ_32, (غیرعلامتی حرف *)rec,
			      حجم(*rec));

	iotrace.needed_سائز += حجم(ساخت iotrace_ریکارڈ);
	iotrace.انحراف += حجم(ساخت iotrace_ریکارڈ);
}

u32 iotrace_readl(ثابت خالی *ptr)
{
	u32 v;

	v = readl(ptr);
	جوڑو_ریکارڈ(IOT_32 | IOT_READ, ptr, v);

	واپس v;
}

خالی iotrace_writel(ulong قدر, خالی *ptr)
{
	جوڑو_ریکارڈ(IOT_32 | IOT_WRITE, ptr, قدر);
	writel(قدر, ptr);
}

u16 iotrace_readw(ثابت خالی *ptr)
{
	u32 v;

	v = readw(ptr);
	جوڑو_ریکارڈ(IOT_16 | IOT_READ, ptr, v);

	واپس v;
}

خالی iotrace_writew(ulong قدر, خالی *ptr)
{
	جوڑو_ریکارڈ(IOT_16 | IOT_WRITE, ptr, قدر);
	writew(قدر, ptr);
}

u8 iotrace_readb(ثابت خالی *ptr)
{
	u32 v;

	v = readb(ptr);
	جوڑو_ریکارڈ(IOT_8 | IOT_READ, ptr, v);

	واپس v;
}

خالی iotrace_writeb(ulong قدر, خالی *ptr)
{
	جوڑو_ریکارڈ(IOT_8 | IOT_WRITE, ptr, قدر);
	writeb(قدر, ptr);
}

خالی iotrace_reset_checksum(خالی)
{
	iotrace.چکری_جانچ_32 = 0;
}

u32 iotrace_get_checksum(خالی)
{
	واپس iotrace.چکری_جانچ_32;
}

خالی iotrace_سیٹ_region(ulong آغاز, ulong سائز)
{
	iotrace.region_آغاز = آغاز;
	iotrace.region_سائز = سائز;
}

خالی iotrace_reset_region(خالی)
{
	iotrace.region_آغاز = 0;
	iotrace.region_سائز = 0;
}

خالی iotrace_get_region(ulong *آغاز, ulong *سائز)
{
	*آغاز = iotrace.region_آغاز;
	*سائز = iotrace.region_سائز;
}

خالی iotrace_سیٹ_enabled(صحیح enable)
{
	iotrace.enabled = enable;
}

صحیح iotrace_get_enabled(خالی)
{
	واپس iotrace.enabled;
}

خالی iotrace_سیٹ_بفر(ulong آغاز, ulong سائز)
{
	iotrace.آغاز = آغاز;
	iotrace.سائز = سائز;
	iotrace.انحراف = 0;
	iotrace.چکری_جانچ_32 = 0;
}

خالی iotrace_get_بفر(ulong *آغاز, ulong *سائز, ulong *needed_سائز, ulong *انحراف, ulong *گنتی)
{
	*آغاز = iotrace.آغاز;
	*سائز = iotrace.سائز;
	*needed_سائز = iotrace.needed_سائز;
	*انحراف = iotrace.انحراف;
	*گنتی = iotrace.انحراف / حجم(ساخت iotrace_ریکارڈ);
}
