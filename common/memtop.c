// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Limited
 */

#ضمن <fdt_support.h>
#ضمن <fdtdec.h>
#ضمن <memtop.h>

#ضمن <asm/types.h>

#تعریف_کړه MEM_RGN_شمېر	64

بنية region {
	phys_addr_t بنسټ;
	phys_اندازه_t اندازه;
};

بنية mem_region {
	بنية region rgn[MEM_RGN_شمېر];
	uint شمېر;
};

ساكن فراغ جمع_کړه_mem_region(بنية mem_region *mem_rgn, phys_addr_t بنسټ,
			   phys_اندازه_t اندازه)
{
	اوږد i;

	لكل (i = mem_rgn->شمېر; i >= 0; i--) {
		إذا (i && بنسټ < mem_rgn->rgn[i - 1].بنسټ) {
			mem_rgn->rgn[i] = mem_rgn->rgn[i - 1];
		} وإلا {
			mem_rgn->rgn[i].بنسټ = بنسټ;
			mem_rgn->rgn[i].اندازه = اندازه;
			اكسر;
		}
	}

	mem_rgn->شمېر++;
}

ساكن فراغ mem_regions_init(بنية mem_region *mem)
{
	uint i;

	mem->شمېر = 0;
	لكل (i = 0; i < MEM_RGN_شمېر; i++) {
		mem->rgn[i].بنسټ = 0;
		mem->rgn[i].اندازه = 0;
	}
}

ساكن صحيح fdt_جمع_کړه_reserved_regions(بنية mem_region *free_mem,
				    بنية mem_region *reserved_mem,
				    فراغ *fdt_blob)
{
	u64 addr, اندازه;
	صحيح i, ټول, ret;
	صحيح nodeoffset, subnode;
	بنية fdt_resource res;

	إذا (fdt_check_سرلیک(fdt_blob) != 0)
		أرجع -1;

	/* process memreserve sections */
	ټول = fdt_عدد_mem_rsv(fdt_blob);
	assert_noisy(ټول < MEM_RGN_شمېر);
	لكل (i = 0; i < ټول; i++) {
		إذا (fdt_get_mem_rsv(fdt_blob, i, &addr, &اندازه) != 0)
			استمر;
		جمع_کړه_mem_region(reserved_mem, addr, اندازه);
	}

	i = 0;
	/* process reserved-memory */
	nodeoffset = fdt_subnode_دننه_واټن(fdt_blob, 0, "reserved-memory");
	إذا (nodeoffset >= 0) {
		subnode = fdt_لومړی_subnode(fdt_blob, nodeoffset);
		طالما (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			إذا (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				addr = res.پیل;
				اندازه = res.پای_ټکی_2 - res.پیل + 1;
				assert_noisy(i < MEM_RGN_شمېر);
				جمع_کړه_mem_region(reserved_mem, addr, اندازه);
			}

			subnode = fdt_بل_subnode(fdt_blob, subnode);
			++i;
		}
	}

	أرجع 0;
}

ساكن اوږد addrs_overlap(phys_addr_t base1, phys_اندازه_t size1,
			  phys_addr_t base2, phys_اندازه_t size2)
{
	ثابت phys_addr_t base1_پای_ټکی = base1 + size1 - 1;
	ثابت phys_addr_t base2_پای_ټکی = base2 + size2 - 1;

	أرجع ((base1 <= base2_پای_ټکی) && (base2 <= base1_پای_ټکی));
}

ساكن اوږد region_overlap_check(بنية mem_region *mem_rgn, phys_addr_t بنسټ,
				 phys_اندازه_t اندازه)
{
	بینښه اوږد i;
	بنية region *rgn = mem_rgn->rgn;

	لكل (i = 0; i < mem_rgn->شمېر; i++) {
		phys_addr_t rgnbase = rgn[i].بنسټ;
		phys_اندازه_t rgnsize = rgn[i].اندازه;

		إذا (addrs_overlap(بنسټ, اندازه, rgnbase, rgnsize))
			اكسر;
	}

	أرجع (i < mem_rgn->شمېر) ? i : -1;
}

ساكن phys_addr_t پیدا_کړه_ram_سر(بنية mem_region *free_mem,
				بنية mem_region *reserved_mem, phys_اندازه_t اندازه)
{
	اوږد i, rgn;
	phys_addr_t بنسټ = 0;
	phys_addr_t res_بنسټ;

	لكل (i = free_mem->شمېر - 1; i >= 0; i--) {
		phys_addr_t rgnbase = free_mem->rgn[i].بنسټ;
		phys_اندازه_t rgnsize = free_mem->rgn[i].اندازه;

		إذا (rgnsize < اندازه)
			استمر;

		بنسټ = rgnbase + rgnsize - اندازه;
		طالما (بنسټ && rgnbase <= بنسټ) {
			rgn = region_overlap_check(reserved_mem, بنسټ, اندازه);
			إذا (rgn < 0)
				أرجع بنسټ;

			res_بنسټ = reserved_mem->rgn[rgn].بنسټ;
			إذا (res_بنسټ < اندازه)
				اكسر;
			بنسټ = res_بنسټ - اندازه;
		}
	}

	أرجع 0;
}

phys_addr_t get_mem_سر(phys_addr_t ram_پیل, phys_اندازه_t ram_اندازه,
			phys_اندازه_t اندازه, فراغ *fdt)
{
	صحيح i;
	بنية mem_region free_mem;
	بنية mem_region reserved_mem;

	mem_regions_init(&free_mem);
	mem_regions_init(&reserved_mem);

	جمع_کړه_mem_region(&free_mem, ram_پیل, ram_اندازه);

	i = fdt_جمع_کړه_reserved_regions(&free_mem, &reserved_mem, fdt);
	إذا (i < 0)
		أرجع 0;

	أرجع پیدا_کړه_ram_سر(&free_mem, &reserved_mem, اندازه);
}
