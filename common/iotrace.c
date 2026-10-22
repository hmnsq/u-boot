// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc.
 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IOTRACE_IMPL

#ਅਨ੍ਤਰ੍ਭਾਵ <mapmem.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/io.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/bug.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <u-boot/crc.h>

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

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
ਸਥਿਰ ਬਣਤਰ iotrace {
	ulong ਸ਼ੁਰੂ;
	ulong ਮਾਪ;
	ulong needed_ਮਾਪ;
	ulong ਵਿੱਥ;
	ulong region_ਸ਼ੁਰੂ;
	ulong region_ਮਾਪ;
	u32 ਚੱਕਰੀ_ਜਾਂਚ_32;
	bool enabled;
} iotrace;

ਸਥਿਰ ਖਾਲੀ ਜੋੜੋ_ਰਿਕਾਰਡ(ਪੂਰਨ flags, ਅਚਲ ਖਾਲੀ *ptr, ulong ਮੁੱਲ)
{
	ਬਣਤਰ iotrace_ਰਿਕਾਰਡ srec, *rec = &srec;

	/*
	 * We don't support iotrace before relocation. Since the trace buffer
	 * is set up by a command, it can't be enabled at present. To change
	 * this we would need to set the iotrace buffer at build-time. See
	 * lib/trace.c for how this might be done if you are interested.
	 */
	ਜੇ (!(gd->flags & GD_FLG_RELOC) || !iotrace.enabled)
		ਵਾਪਸ;

	ਜੇ (iotrace.region_ਮਾਪ)
		ਜੇ ((ulong)ptr < iotrace.region_ਸ਼ੁਰੂ ||
		    (ulong)ptr > iotrace.region_ਸ਼ੁਰੂ + iotrace.region_ਮਾਪ)
			ਵਾਪਸ;

	/* Store it if there is room */
	ਜੇ (iotrace.ਵਿੱਥ + ਆਕਾਰ(*rec) < iotrace.ਮਾਪ) {
		rec = (ਬਣਤਰ iotrace_ਰਿਕਾਰਡ *)map_sysmem(
					iotrace.ਸ਼ੁਰੂ + iotrace.ਵਿੱਥ,
					ਆਕਾਰ(ਮੁੱਲ));
	} ਨਹੀਂਤਾਂ {
		WARN_ONCE(1, "WARNING: iotrace buffer exhausted, please check needed length using \"iotrace stats\"\n");
		iotrace.needed_ਮਾਪ += ਆਕਾਰ(ਬਣਤਰ iotrace_ਰਿਕਾਰਡ);
		ਵਾਪਸ;
	}

	rec->timestamp = timer_get_us();
	rec->flags = flags;
	rec->addr = map_ਮੰਜ਼ਿਲ_sysmem(ptr);
	rec->ਮੁੱਲ = ਮੁੱਲ;

	/* Update our checksum */
	iotrace.ਚੱਕਰੀ_ਜਾਂਚ_32 = ਚੱਕਰੀ_ਜਾਂਚ_32(iotrace.ਚੱਕਰੀ_ਜਾਂਚ_32, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)rec,
			      ਆਕਾਰ(*rec));

	iotrace.needed_ਮਾਪ += ਆਕਾਰ(ਬਣਤਰ iotrace_ਰਿਕਾਰਡ);
	iotrace.ਵਿੱਥ += ਆਕਾਰ(ਬਣਤਰ iotrace_ਰਿਕਾਰਡ);
}

u32 iotrace_readl(ਅਚਲ ਖਾਲੀ *ptr)
{
	u32 v;

	v = readl(ptr);
	ਜੋੜੋ_ਰਿਕਾਰਡ(IOT_32 | IOT_READ, ptr, v);

	ਵਾਪਸ v;
}

ਖਾਲੀ iotrace_writel(ulong ਮੁੱਲ, ਖਾਲੀ *ptr)
{
	ਜੋੜੋ_ਰਿਕਾਰਡ(IOT_32 | IOT_WRITE, ptr, ਮੁੱਲ);
	writel(ਮੁੱਲ, ptr);
}

u16 iotrace_readw(ਅਚਲ ਖਾਲੀ *ptr)
{
	u32 v;

	v = readw(ptr);
	ਜੋੜੋ_ਰਿਕਾਰਡ(IOT_16 | IOT_READ, ptr, v);

	ਵਾਪਸ v;
}

ਖਾਲੀ iotrace_writew(ulong ਮੁੱਲ, ਖਾਲੀ *ptr)
{
	ਜੋੜੋ_ਰਿਕਾਰਡ(IOT_16 | IOT_WRITE, ptr, ਮੁੱਲ);
	writew(ਮੁੱਲ, ptr);
}

u8 iotrace_readb(ਅਚਲ ਖਾਲੀ *ptr)
{
	u32 v;

	v = readb(ptr);
	ਜੋੜੋ_ਰਿਕਾਰਡ(IOT_8 | IOT_READ, ptr, v);

	ਵਾਪਸ v;
}

ਖਾਲੀ iotrace_writeb(ulong ਮੁੱਲ, ਖਾਲੀ *ptr)
{
	ਜੋੜੋ_ਰਿਕਾਰਡ(IOT_8 | IOT_WRITE, ptr, ਮੁੱਲ);
	writeb(ਮੁੱਲ, ptr);
}

ਖਾਲੀ iotrace_reset_checksum(ਖਾਲੀ)
{
	iotrace.ਚੱਕਰੀ_ਜਾਂਚ_32 = 0;
}

u32 iotrace_get_checksum(ਖਾਲੀ)
{
	ਵਾਪਸ iotrace.ਚੱਕਰੀ_ਜਾਂਚ_32;
}

ਖਾਲੀ iotrace_ਸੈੱਟ_region(ulong ਸ਼ੁਰੂ, ulong ਮਾਪ)
{
	iotrace.region_ਸ਼ੁਰੂ = ਸ਼ੁਰੂ;
	iotrace.region_ਮਾਪ = ਮਾਪ;
}

ਖਾਲੀ iotrace_reset_region(ਖਾਲੀ)
{
	iotrace.region_ਸ਼ੁਰੂ = 0;
	iotrace.region_ਮਾਪ = 0;
}

ਖਾਲੀ iotrace_get_region(ulong *ਸ਼ੁਰੂ, ulong *ਮਾਪ)
{
	*ਸ਼ੁਰੂ = iotrace.region_ਸ਼ੁਰੂ;
	*ਮਾਪ = iotrace.region_ਮਾਪ;
}

ਖਾਲੀ iotrace_ਸੈੱਟ_enabled(ਪੂਰਨ enable)
{
	iotrace.enabled = enable;
}

ਪੂਰਨ iotrace_get_enabled(ਖਾਲੀ)
{
	ਵਾਪਸ iotrace.enabled;
}

ਖਾਲੀ iotrace_ਸੈੱਟ_ਬਫਰ(ulong ਸ਼ੁਰੂ, ulong ਮਾਪ)
{
	iotrace.ਸ਼ੁਰੂ = ਸ਼ੁਰੂ;
	iotrace.ਮਾਪ = ਮਾਪ;
	iotrace.ਵਿੱਥ = 0;
	iotrace.ਚੱਕਰੀ_ਜਾਂਚ_32 = 0;
}

ਖਾਲੀ iotrace_get_ਬਫਰ(ulong *ਸ਼ੁਰੂ, ulong *ਮਾਪ, ulong *needed_ਮਾਪ, ulong *ਵਿੱਥ, ulong *ਗਿਣਤੀ_2)
{
	*ਸ਼ੁਰੂ = iotrace.ਸ਼ੁਰੂ;
	*ਮਾਪ = iotrace.ਮਾਪ;
	*needed_ਮਾਪ = iotrace.needed_ਮਾਪ;
	*ਵਿੱਥ = iotrace.ਵਿੱਥ;
	*ਗਿਣਤੀ_2 = iotrace.ਵਿੱਥ / ਆਕਾਰ(ਬਣਤਰ iotrace_ਰਿਕਾਰਡ);
}
