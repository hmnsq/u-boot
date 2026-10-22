// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Limited
 */

#ଅନ୍ତର୍ଭାଵ <fdt_support.h>
#ଅନ୍ତର୍ଭାଵ <fdtdec.h>
#ଅନ୍ତର୍ଭାଵ <memtop.h>

#ଅନ୍ତର୍ଭାଵ <asm/types.h>

#define MEM_RGN_ଗଣନା	64

ସଂରଚନା region {
	phys_addr_t base;
	phys_size_t size;
};

ସଂରଚନା mem_region {
	ସଂରଚନା region rgn[MEM_RGN_ଗଣନା];
	uint ଗଣନା_2;
};

ସ୍ଥିର ଶୂନ୍ୟ add_mem_region(ସଂରଚନା mem_region *mem_rgn, phys_addr_t base,
			   phys_size_t size)
{
	long i;

	ପାଇଁ (i = mem_rgn->ଗଣନା_2; i >= 0; i--) {
		ଯଦି (i && base < mem_rgn->rgn[i - 1].base) {
			mem_rgn->rgn[i] = mem_rgn->rgn[i - 1];
		} ଅନ୍ୟଥା {
			mem_rgn->rgn[i].base = base;
			mem_rgn->rgn[i].size = size;
			ଭାଙ୍ଗ;
		}
	}

	mem_rgn->ଗଣନା_2++;
}

ସ୍ଥିର ଶୂନ୍ୟ mem_regions_init(ସଂରଚନା mem_region *mem)
{
	uint i;

	mem->ଗଣନା_2 = 0;
	ପାଇଁ (i = 0; i < MEM_RGN_ଗଣନା; i++) {
		mem->rgn[i].base = 0;
		mem->rgn[i].size = 0;
	}
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fdt_add_reserved_regions(ସଂରଚନା mem_region *free_mem,
				    ସଂରଚନା mem_region *reserved_mem,
				    ଶୂନ୍ୟ *fdt_blob)
{
	u64 addr, size;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, total, ret;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା nodeoffset, subnode;
	ସଂରଚନା fdt_resource res;

	ଯଦି (fdt_check_header(fdt_blob) != 0)
		ଫେରନ୍ତୁ -1;

	/* process memreserve sections */
	total = fdt_ଅଙ୍କ_mem_rsv(fdt_blob);
	assert_noisy(total < MEM_RGN_ଗଣନା);
	ପାଇଁ (i = 0; i < total; i++) {
		ଯଦି (fdt_get_mem_rsv(fdt_blob, i, &addr, &size) != 0)
			ଜାରି;
		add_mem_region(reserved_mem, addr, size);
	}

	i = 0;
	/* process reserved-memory */
	nodeoffset = fdt_subnode_offset(fdt_blob, 0, "reserved-memory");
	ଯଦି (nodeoffset >= 0) {
		subnode = fdt_first_subnode(fdt_blob, nodeoffset);
		ଯେତେବେଳେ (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			ଯଦି (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				addr = res.start;
				size = res.end - res.start + 1;
				assert_noisy(i < MEM_RGN_ଗଣନା);
				add_mem_region(reserved_mem, addr, size);
			}

			subnode = fdt_next_subnode(fdt_blob, subnode);
			++i;
		}
	}

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର long addrs_overlap(phys_addr_t base1, phys_size_t size1,
			  phys_addr_t base2, phys_size_t size2)
{
	ଅଚଳ phys_addr_t base1_end = base1 + size1 - 1;
	ଅଚଳ phys_addr_t base2_end = base2 + size2 - 1;

	ଫେରନ୍ତୁ ((base1 <= base2_end) && (base2 <= base1_end));
}

ସ୍ଥିର long region_overlap_check(ସଂରଚନା mem_region *mem_rgn, phys_addr_t base,
				 phys_size_t size)
{
	unsigned long i;
	ସଂରଚନା region *rgn = mem_rgn->rgn;

	ପାଇଁ (i = 0; i < mem_rgn->ଗଣନା_2; i++) {
		phys_addr_t rgnbase = rgn[i].base;
		phys_size_t rgnsize = rgn[i].size;

		ଯଦି (addrs_overlap(base, size, rgnbase, rgnsize))
			ଭାଙ୍ଗ;
	}

	ଫେରନ୍ତୁ (i < mem_rgn->ଗଣନା_2) ? i : -1;
}

ସ୍ଥିର phys_addr_t find_ram_ଶୀର୍ଷ(ସଂରଚନା mem_region *free_mem,
				ସଂରଚନା mem_region *reserved_mem, phys_size_t size)
{
	long i, rgn;
	phys_addr_t base = 0;
	phys_addr_t res_base;

	ପାଇଁ (i = free_mem->ଗଣନା_2 - 1; i >= 0; i--) {
		phys_addr_t rgnbase = free_mem->rgn[i].base;
		phys_size_t rgnsize = free_mem->rgn[i].size;

		ଯଦି (rgnsize < size)
			ଜାରି;

		base = rgnbase + rgnsize - size;
		ଯେତେବେଳେ (base && rgnbase <= base) {
			rgn = region_overlap_check(reserved_mem, base, size);
			ଯଦି (rgn < 0)
				ଫେରନ୍ତୁ base;

			res_base = reserved_mem->rgn[rgn].base;
			ଯଦି (res_base < size)
				ଭାଙ୍ଗ;
			base = res_base - size;
		}
	}

	ଫେରନ୍ତୁ 0;
}

phys_addr_t get_mem_ଶୀର୍ଷ(phys_addr_t ram_start, phys_size_t ram_size,
			phys_size_t size, ଶୂନ୍ୟ *fdt)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ସଂରଚନା mem_region free_mem;
	ସଂରଚନା mem_region reserved_mem;

	mem_regions_init(&free_mem);
	mem_regions_init(&reserved_mem);

	add_mem_region(&free_mem, ram_start, ram_size);

	i = fdt_add_reserved_regions(&free_mem, &reserved_mem, fdt);
	ଯଦି (i < 0)
		ଫେରନ୍ତୁ 0;

	ଫେରନ୍ତୁ find_ram_ଶୀର୍ଷ(&free_mem, &reserved_mem, size);
}
