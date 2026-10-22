// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc.
 */

#ವ್ಯಾಖ್ಯಾನಿಸು IOTRACE_IMPL

#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <asm/io.h>
#ಅನ್ತರ್ಭಾವ <linux/bug.h>
#ಅನ್ತರ್ಭಾವ <u-boot/crc.h>

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

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
ಸ್ಥಿರ ರಚನೆ iotrace {
	ulong ಆರಂಭ;
	ulong ಗಾತ್ರ_2;
	ulong needed_ಗಾತ್ರ;
	ulong ಸ್ಥಾನಾಂತರ;
	ulong region_ಆರಂಭ;
	ulong region_ಗಾತ್ರ;
	u32 ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32;
	bool enabled;
} iotrace;

ಸ್ಥಿರ ಶೂನ್ಯ ಕೂಡಿಸು_ದಾಖಲೆ(ಪೂರ್ಣಾಂಕ flags, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ptr, ulong ಮೌಲ್ಯ)
{
	ರಚನೆ iotrace_ದಾಖಲೆ srec, *rec = &srec;

	/*
	 * We don't support iotrace before relocation. Since the trace buffer
	 * is set up by a command, it can't be enabled at present. To change
	 * this we would need to set the iotrace buffer at build-time. See
	 * lib/trace.c for how this might be done if you are interested.
	 */
	ಆದರೆ (!(gd->flags & GD_FLG_RELOC) || !iotrace.enabled)
		ಹಿಂತಿರುಗಿ;

	ಆದರೆ (iotrace.region_ಗಾತ್ರ)
		ಆದರೆ ((ulong)ptr < iotrace.region_ಆರಂಭ ||
		    (ulong)ptr > iotrace.region_ಆರಂಭ + iotrace.region_ಗಾತ್ರ)
			ಹಿಂತಿರುಗಿ;

	/* Store it if there is room */
	ಆದರೆ (iotrace.ಸ್ಥಾನಾಂತರ + ಗಾತ್ರ(*rec) < iotrace.ಗಾತ್ರ_2) {
		rec = (ರಚನೆ iotrace_ದಾಖಲೆ *)map_sysmem(
					iotrace.ಆರಂಭ + iotrace.ಸ್ಥಾನಾಂತರ,
					ಗಾತ್ರ(ಮೌಲ್ಯ));
	} ಇಲ್ಲದಿದ್ದರೆ {
		WARN_ONCE(1, "WARNING: iotrace buffer exhausted, please check needed length using \"iotrace stats\"\n");
		iotrace.needed_ಗಾತ್ರ += ಗಾತ್ರ(ರಚನೆ iotrace_ದಾಖಲೆ);
		ಹಿಂತಿರುಗಿ;
	}

	rec->timestamp = timer_get_us();
	rec->flags = flags;
	rec->addr = map_ಗಮ್ಯ_sysmem(ptr);
	rec->ಮೌಲ್ಯ = ಮೌಲ್ಯ;

	/* Update our checksum */
	iotrace.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32 = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32(iotrace.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32, (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)rec,
			      ಗಾತ್ರ(*rec));

	iotrace.needed_ಗಾತ್ರ += ಗಾತ್ರ(ರಚನೆ iotrace_ದಾಖಲೆ);
	iotrace.ಸ್ಥಾನಾಂತರ += ಗಾತ್ರ(ರಚನೆ iotrace_ದಾಖಲೆ);
}

u32 iotrace_readl(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ptr)
{
	u32 v;

	v = readl(ptr);
	ಕೂಡಿಸು_ದಾಖಲೆ(IOT_32 | IOT_READ, ptr, v);

	ಹಿಂತಿರುಗಿ v;
}

ಶೂನ್ಯ iotrace_writel(ulong ಮೌಲ್ಯ, ಶೂನ್ಯ *ptr)
{
	ಕೂಡಿಸು_ದಾಖಲೆ(IOT_32 | IOT_WRITE, ptr, ಮೌಲ್ಯ);
	writel(ಮೌಲ್ಯ, ptr);
}

u16 iotrace_readw(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ptr)
{
	u32 v;

	v = readw(ptr);
	ಕೂಡಿಸು_ದಾಖಲೆ(IOT_16 | IOT_READ, ptr, v);

	ಹಿಂತಿರುಗಿ v;
}

ಶೂನ್ಯ iotrace_writew(ulong ಮೌಲ್ಯ, ಶೂನ್ಯ *ptr)
{
	ಕೂಡಿಸು_ದಾಖಲೆ(IOT_16 | IOT_WRITE, ptr, ಮೌಲ್ಯ);
	writew(ಮೌಲ್ಯ, ptr);
}

u8 iotrace_readb(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ptr)
{
	u32 v;

	v = readb(ptr);
	ಕೂಡಿಸು_ದಾಖಲೆ(IOT_8 | IOT_READ, ptr, v);

	ಹಿಂತಿರುಗಿ v;
}

ಶೂನ್ಯ iotrace_writeb(ulong ಮೌಲ್ಯ, ಶೂನ್ಯ *ptr)
{
	ಕೂಡಿಸು_ದಾಖಲೆ(IOT_8 | IOT_WRITE, ptr, ಮೌಲ್ಯ);
	writeb(ಮೌಲ್ಯ, ptr);
}

ಶೂನ್ಯ iotrace_reset_checksum(ಶೂನ್ಯ)
{
	iotrace.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32 = 0;
}

u32 iotrace_get_checksum(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ iotrace.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32;
}

ಶೂನ್ಯ iotrace_ಗಣ_region(ulong ಆರಂಭ, ulong ಗಾತ್ರ_2)
{
	iotrace.region_ಆರಂಭ = ಆರಂಭ;
	iotrace.region_ಗಾತ್ರ = ಗಾತ್ರ_2;
}

ಶೂನ್ಯ iotrace_reset_region(ಶೂನ್ಯ)
{
	iotrace.region_ಆರಂಭ = 0;
	iotrace.region_ಗಾತ್ರ = 0;
}

ಶೂನ್ಯ iotrace_get_region(ulong *ಆರಂಭ, ulong *ಗಾತ್ರ_2)
{
	*ಆರಂಭ = iotrace.region_ಆರಂಭ;
	*ಗಾತ್ರ_2 = iotrace.region_ಗಾತ್ರ;
}

ಶೂನ್ಯ iotrace_ಗಣ_enabled(ಪೂರ್ಣಾಂಕ enable)
{
	iotrace.enabled = enable;
}

ಪೂರ್ಣಾಂಕ iotrace_get_enabled(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ iotrace.enabled;
}

ಶೂನ್ಯ iotrace_ಗಣ_ಬಫರ್(ulong ಆರಂಭ, ulong ಗಾತ್ರ_2)
{
	iotrace.ಆರಂಭ = ಆರಂಭ;
	iotrace.ಗಾತ್ರ_2 = ಗಾತ್ರ_2;
	iotrace.ಸ್ಥಾನಾಂತರ = 0;
	iotrace.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32 = 0;
}

ಶೂನ್ಯ iotrace_get_ಬಫರ್(ulong *ಆರಂಭ, ulong *ಗಾತ್ರ_2, ulong *needed_ಗಾತ್ರ, ulong *ಸ್ಥಾನಾಂತರ, ulong *ಎಣಿಕೆ_2)
{
	*ಆರಂಭ = iotrace.ಆರಂಭ;
	*ಗಾತ್ರ_2 = iotrace.ಗಾತ್ರ_2;
	*needed_ಗಾತ್ರ = iotrace.needed_ಗಾತ್ರ;
	*ಸ್ಥಾನಾಂತರ = iotrace.ಸ್ಥಾನಾಂತರ;
	*ಎಣಿಕೆ_2 = iotrace.ಸ್ಥಾನಾಂತರ / ಗಾತ್ರ(ರಚನೆ iotrace_ದಾಖಲೆ);
}
