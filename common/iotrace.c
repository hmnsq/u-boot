// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc.
 */

#నిర్వచించు IOTRACE_IMPL

#అన్తర్భావ <mapmem.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <asm/global_data.h>
#అన్తర్భావ <asm/io.h>
#అన్తర్భావ <linux/bug.h>
#అన్తర్భావ <u-boot/crc.h>

DECLARE_GLOBAL_దత్తాంశం_PTR;

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
స్థిర నిర్మాణం iotrace {
	ulong ప్రారంభం;
	ulong కొలత;
	ulong needed_కొలత;
	ulong స్థానభ్రంశం;
	ulong region_ప్రారంభం;
	ulong region_కొలత;
	u32 చక్రీయ_తనిఖీ_32;
	bool enabled;
} iotrace;

స్థిర శూన్య కూడు_రికార్డు(పూర్ణాంకం flags, స్థిరరాశి శూన్య *ptr, ulong విలువ)
{
	నిర్మాణం iotrace_రికార్డు srec, *rec = &srec;

	/*
	 * We don't support iotrace before relocation. Since the trace buffer
	 * is set up by a command, it can't be enabled at present. To change
	 * this we would need to set the iotrace buffer at build-time. See
	 * lib/trace.c for how this might be done if you are interested.
	 */
	ఒకవేళ (!(gd->flags & GD_FLG_RELOC) || !iotrace.enabled)
		తిరిగి;

	ఒకవేళ (iotrace.region_కొలత)
		ఒకవేళ ((ulong)ptr < iotrace.region_ప్రారంభం ||
		    (ulong)ptr > iotrace.region_ప్రారంభం + iotrace.region_కొలత)
			తిరిగి;

	/* Store it if there is room */
	ఒకవేళ (iotrace.స్థానభ్రంశం + పరిమాణం(*rec) < iotrace.కొలత) {
		rec = (నిర్మాణం iotrace_రికార్డు *)map_sysmem(
					iotrace.ప్రారంభం + iotrace.స్థానభ్రంశం,
					పరిమాణం(విలువ));
	} లేకపోతే {
		WARN_ONCE(1, "WARNING: iotrace buffer exhausted, please check needed length using \"iotrace stats\"\n");
		iotrace.needed_కొలత += పరిమాణం(నిర్మాణం iotrace_రికార్డు);
		తిరిగి;
	}

	rec->timestamp = timer_get_us();
	rec->flags = flags;
	rec->addr = map_గమ్యం_sysmem(ptr);
	rec->విలువ = విలువ;

	/* Update our checksum */
	iotrace.చక్రీయ_తనిఖీ_32 = చక్రీయ_తనిఖీ_32(iotrace.చక్రీయ_తనిఖీ_32, (గుర్తులేని అక్షరం *)rec,
			      పరిమాణం(*rec));

	iotrace.needed_కొలత += పరిమాణం(నిర్మాణం iotrace_రికార్డు);
	iotrace.స్థానభ్రంశం += పరిమాణం(నిర్మాణం iotrace_రికార్డు);
}

u32 iotrace_readl(స్థిరరాశి శూన్య *ptr)
{
	u32 v;

	v = readl(ptr);
	కూడు_రికార్డు(IOT_32 | IOT_READ, ptr, v);

	తిరిగి v;
}

శూన్య iotrace_writel(ulong విలువ, శూన్య *ptr)
{
	కూడు_రికార్డు(IOT_32 | IOT_WRITE, ptr, విలువ);
	writel(విలువ, ptr);
}

u16 iotrace_readw(స్థిరరాశి శూన్య *ptr)
{
	u32 v;

	v = readw(ptr);
	కూడు_రికార్డు(IOT_16 | IOT_READ, ptr, v);

	తిరిగి v;
}

శూన్య iotrace_writew(ulong విలువ, శూన్య *ptr)
{
	కూడు_రికార్డు(IOT_16 | IOT_WRITE, ptr, విలువ);
	writew(విలువ, ptr);
}

u8 iotrace_readb(స్థిరరాశి శూన్య *ptr)
{
	u32 v;

	v = readb(ptr);
	కూడు_రికార్డు(IOT_8 | IOT_READ, ptr, v);

	తిరిగి v;
}

శూన్య iotrace_writeb(ulong విలువ, శూన్య *ptr)
{
	కూడు_రికార్డు(IOT_8 | IOT_WRITE, ptr, విలువ);
	writeb(విలువ, ptr);
}

శూన్య iotrace_reset_checksum(శూన్య)
{
	iotrace.చక్రీయ_తనిఖీ_32 = 0;
}

u32 iotrace_get_checksum(శూన్య)
{
	తిరిగి iotrace.చక్రీయ_తనిఖీ_32;
}

శూన్య iotrace_సమితి_region(ulong ప్రారంభం, ulong కొలత)
{
	iotrace.region_ప్రారంభం = ప్రారంభం;
	iotrace.region_కొలత = కొలత;
}

శూన్య iotrace_reset_region(శూన్య)
{
	iotrace.region_ప్రారంభం = 0;
	iotrace.region_కొలత = 0;
}

శూన్య iotrace_get_region(ulong *ప్రారంభం, ulong *కొలత)
{
	*ప్రారంభం = iotrace.region_ప్రారంభం;
	*కొలత = iotrace.region_కొలత;
}

శూన్య iotrace_సమితి_enabled(పూర్ణాంకం enable)
{
	iotrace.enabled = enable;
}

పూర్ణాంకం iotrace_get_enabled(శూన్య)
{
	తిరిగి iotrace.enabled;
}

శూన్య iotrace_సమితి_బఫరు(ulong ప్రారంభం, ulong కొలత)
{
	iotrace.ప్రారంభం = ప్రారంభం;
	iotrace.కొలత = కొలత;
	iotrace.స్థానభ్రంశం = 0;
	iotrace.చక్రీయ_తనిఖీ_32 = 0;
}

శూన్య iotrace_get_బఫరు(ulong *ప్రారంభం, ulong *కొలత, ulong *needed_కొలత, ulong *స్థానభ్రంశం, ulong *లెక్క)
{
	*ప్రారంభం = iotrace.ప్రారంభం;
	*కొలత = iotrace.కొలత;
	*needed_కొలత = iotrace.needed_కొలత;
	*స్థానభ్రంశం = iotrace.స్థానభ్రంశం;
	*లెక్క = iotrace.స్థానభ్రంశం / పరిమాణం(నిర్మాణం iotrace_రికార్డు);
}
