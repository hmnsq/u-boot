// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Limited
 */

#అన్తర్భావ <fdt_support.h>
#అన్తర్భావ <fdtdec.h>
#అన్తర్భావ <memtop.h>

#అన్తర్భావ <asm/types.h>

#నిర్వచించు MEM_RGN_లెక్క	64

నిర్మాణం region {
	phys_addr_t పునాది;
	phys_కొలత_t కొలత;
};

నిర్మాణం mem_region {
	నిర్మాణం region rgn[MEM_RGN_లెక్క];
	uint లెక్క;
};

స్థిర శూన్య కూడు_mem_region(నిర్మాణం mem_region *mem_rgn, phys_addr_t పునాది,
			   phys_కొలత_t కొలత)
{
	దీర్ఘ i;

	కొరకు (i = mem_rgn->లెక్క; i >= 0; i--) {
		ఒకవేళ (i && పునాది < mem_rgn->rgn[i - 1].పునాది) {
			mem_rgn->rgn[i] = mem_rgn->rgn[i - 1];
		} లేకపోతే {
			mem_rgn->rgn[i].పునాది = పునాది;
			mem_rgn->rgn[i].కొలత = కొలత;
			ఆపు;
		}
	}

	mem_rgn->లెక్క++;
}

స్థిర శూన్య mem_regions_init(నిర్మాణం mem_region *mem)
{
	uint i;

	mem->లెక్క = 0;
	కొరకు (i = 0; i < MEM_RGN_లెక్క; i++) {
		mem->rgn[i].పునాది = 0;
		mem->rgn[i].కొలత = 0;
	}
}

స్థిర పూర్ణాంకం fdt_కూడు_reserved_regions(నిర్మాణం mem_region *free_mem,
				    నిర్మాణం mem_region *reserved_mem,
				    శూన్య *fdt_blob)
{
	u64 addr, కొలత;
	పూర్ణాంకం i, మొత్తం, ret;
	పూర్ణాంకం nodeoffset, subnode;
	నిర్మాణం fdt_resource res;

	ఒకవేళ (fdt_check_శీర్షిక(fdt_blob) != 0)
		తిరిగి -1;

	/* process memreserve sections */
	మొత్తం = fdt_అంకె_mem_rsv(fdt_blob);
	assert_noisy(మొత్తం < MEM_RGN_లెక్క);
	కొరకు (i = 0; i < మొత్తం; i++) {
		ఒకవేళ (fdt_get_mem_rsv(fdt_blob, i, &addr, &కొలత) != 0)
			కొనసాగు;
		కూడు_mem_region(reserved_mem, addr, కొలత);
	}

	i = 0;
	/* process reserved-memory */
	nodeoffset = fdt_subnode_స్థానభ్రంశం(fdt_blob, 0, "reserved-memory");
	ఒకవేళ (nodeoffset >= 0) {
		subnode = fdt_మొదటి_subnode(fdt_blob, nodeoffset);
		అయితే (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			ఒకవేళ (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				addr = res.ప్రారంభం;
				కొలత = res.చివర_2 - res.ప్రారంభం + 1;
				assert_noisy(i < MEM_RGN_లెక్క);
				కూడు_mem_region(reserved_mem, addr, కొలత);
			}

			subnode = fdt_తదుపరి_subnode(fdt_blob, subnode);
			++i;
		}
	}

	తిరిగి 0;
}

స్థిర దీర్ఘ addrs_overlap(phys_addr_t base1, phys_కొలత_t size1,
			  phys_addr_t base2, phys_కొలత_t size2)
{
	స్థిరరాశి phys_addr_t base1_చివర = base1 + size1 - 1;
	స్థిరరాశి phys_addr_t base2_చివర = base2 + size2 - 1;

	తిరిగి ((base1 <= base2_చివర) && (base2 <= base1_చివర));
}

స్థిర దీర్ఘ region_overlap_check(నిర్మాణం mem_region *mem_rgn, phys_addr_t పునాది,
				 phys_కొలత_t కొలత)
{
	గుర్తులేని దీర్ఘ i;
	నిర్మాణం region *rgn = mem_rgn->rgn;

	కొరకు (i = 0; i < mem_rgn->లెక్క; i++) {
		phys_addr_t rgnbase = rgn[i].పునాది;
		phys_కొలత_t rgnsize = rgn[i].కొలత;

		ఒకవేళ (addrs_overlap(పునాది, కొలత, rgnbase, rgnsize))
			ఆపు;
	}

	తిరిగి (i < mem_rgn->లెక్క) ? i : -1;
}

స్థిర phys_addr_t కనుగొను_ram_శిఖరం(నిర్మాణం mem_region *free_mem,
				నిర్మాణం mem_region *reserved_mem, phys_కొలత_t కొలత)
{
	దీర్ఘ i, rgn;
	phys_addr_t పునాది = 0;
	phys_addr_t res_పునాది;

	కొరకు (i = free_mem->లెక్క - 1; i >= 0; i--) {
		phys_addr_t rgnbase = free_mem->rgn[i].పునాది;
		phys_కొలత_t rgnsize = free_mem->rgn[i].కొలత;

		ఒకవేళ (rgnsize < కొలత)
			కొనసాగు;

		పునాది = rgnbase + rgnsize - కొలత;
		అయితే (పునాది && rgnbase <= పునాది) {
			rgn = region_overlap_check(reserved_mem, పునాది, కొలత);
			ఒకవేళ (rgn < 0)
				తిరిగి పునాది;

			res_పునాది = reserved_mem->rgn[rgn].పునాది;
			ఒకవేళ (res_పునాది < కొలత)
				ఆపు;
			పునాది = res_పునాది - కొలత;
		}
	}

	తిరిగి 0;
}

phys_addr_t get_mem_శిఖరం(phys_addr_t ram_ప్రారంభం, phys_కొలత_t ram_కొలత,
			phys_కొలత_t కొలత, శూన్య *fdt)
{
	పూర్ణాంకం i;
	నిర్మాణం mem_region free_mem;
	నిర్మాణం mem_region reserved_mem;

	mem_regions_init(&free_mem);
	mem_regions_init(&reserved_mem);

	కూడు_mem_region(&free_mem, ram_ప్రారంభం, ram_కొలత);

	i = fdt_కూడు_reserved_regions(&free_mem, &reserved_mem, fdt);
	ఒకవేళ (i < 0)
		తిరిగి 0;

	తిరిగి కనుగొను_ram_శిఖరం(&free_mem, &reserved_mem, కొలత);
}
