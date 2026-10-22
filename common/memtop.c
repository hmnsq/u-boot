// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Limited
 */

#অন্তর্ভাব <fdt_support.h>
#অন্তর্ভাব <fdtdec.h>
#অন্তর্ভাব <memtop.h>

#অন্তর্ভাব <asm/types.h>

#define MEM_RGN_গুনতি	64

কাঠামো region {
	phys_addr_t base;
	phys_আকার_t আকার_2;
};

কাঠামো mem_region {
	কাঠামো region rgn[MEM_RGN_গুনতি];
	uint গুনতি;
};

স্থির শূন্য add_mem_region(কাঠামো mem_region *mem_rgn, phys_addr_t base,
			   phys_আকার_t আকার_2)
{
	long i;

	জন্য (i = mem_rgn->গুনতি; i >= 0; i--) {
		যদি (i && base < mem_rgn->rgn[i - 1].base) {
			mem_rgn->rgn[i] = mem_rgn->rgn[i - 1];
		} অন্যথা {
			mem_rgn->rgn[i].base = base;
			mem_rgn->rgn[i].আকার_2 = আকার_2;
			বিরতি;
		}
	}

	mem_rgn->গুনতি++;
}

স্থির শূন্য mem_regions_init(কাঠামো mem_region *mem)
{
	uint i;

	mem->গুনতি = 0;
	জন্য (i = 0; i < MEM_RGN_গুনতি; i++) {
		mem->rgn[i].base = 0;
		mem->rgn[i].আকার_2 = 0;
	}
}

স্থির পূর্ণসংখ্যা fdt_add_reserved_regions(কাঠামো mem_region *free_mem,
				    কাঠামো mem_region *reserved_mem,
				    শূন্য *fdt_blob)
{
	u64 addr, আকার_2;
	পূর্ণসংখ্যা i, যোগ_2, ret;
	পূর্ণসংখ্যা nodeoffset, subnode;
	কাঠামো fdt_resource res;

	যদি (fdt_check_header(fdt_blob) != 0)
		ফেরত -1;

	/* process memreserve sections */
	যোগ_2 = fdt_অঙ্ক_mem_rsv(fdt_blob);
	assert_noisy(যোগ_2 < MEM_RGN_গুনতি);
	জন্য (i = 0; i < যোগ_2; i++) {
		যদি (fdt_get_mem_rsv(fdt_blob, i, &addr, &আকার_2) != 0)
			চালু;
		add_mem_region(reserved_mem, addr, আকার_2);
	}

	i = 0;
	/* process reserved-memory */
	nodeoffset = fdt_subnode_offset(fdt_blob, 0, "reserved-memory");
	যদি (nodeoffset >= 0) {
		subnode = fdt_first_subnode(fdt_blob, nodeoffset);
		যখন (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			যদি (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				addr = res.start;
				আকার_2 = res.end - res.start + 1;
				assert_noisy(i < MEM_RGN_গুনতি);
				add_mem_region(reserved_mem, addr, আকার_2);
			}

			subnode = fdt_পরবর্তী_subnode(fdt_blob, subnode);
			++i;
		}
	}

	ফেরত 0;
}

স্থির long addrs_overlap(phys_addr_t base1, phys_আকার_t size1,
			  phys_addr_t base2, phys_আকার_t size2)
{
	ধ্রুবক phys_addr_t base1_end = base1 + size1 - 1;
	ধ্রুবক phys_addr_t base2_end = base2 + size2 - 1;

	ফেরত ((base1 <= base2_end) && (base2 <= base1_end));
}

স্থির long region_overlap_check(কাঠামো mem_region *mem_rgn, phys_addr_t base,
				 phys_আকার_t আকার_2)
{
	unsigned long i;
	কাঠামো region *rgn = mem_rgn->rgn;

	জন্য (i = 0; i < mem_rgn->গুনতি; i++) {
		phys_addr_t rgnbase = rgn[i].base;
		phys_আকার_t rgnsize = rgn[i].আকার_2;

		যদি (addrs_overlap(base, আকার_2, rgnbase, rgnsize))
			বিরতি;
	}

	ফেরত (i < mem_rgn->গুনতি) ? i : -1;
}

স্থির phys_addr_t find_ram_শীর্ষ(কাঠামো mem_region *free_mem,
				কাঠামো mem_region *reserved_mem, phys_আকার_t আকার_2)
{
	long i, rgn;
	phys_addr_t base = 0;
	phys_addr_t res_base;

	জন্য (i = free_mem->গুনতি - 1; i >= 0; i--) {
		phys_addr_t rgnbase = free_mem->rgn[i].base;
		phys_আকার_t rgnsize = free_mem->rgn[i].আকার_2;

		যদি (rgnsize < আকার_2)
			চালু;

		base = rgnbase + rgnsize - আকার_2;
		যখন (base && rgnbase <= base) {
			rgn = region_overlap_check(reserved_mem, base, আকার_2);
			যদি (rgn < 0)
				ফেরত base;

			res_base = reserved_mem->rgn[rgn].base;
			যদি (res_base < আকার_2)
				বিরতি;
			base = res_base - আকার_2;
		}
	}

	ফেরত 0;
}

phys_addr_t get_mem_শীর্ষ(phys_addr_t ram_start, phys_আকার_t ram_আকার,
			phys_আকার_t আকার_2, শূন্য *fdt)
{
	পূর্ণসংখ্যা i;
	কাঠামো mem_region free_mem;
	কাঠামো mem_region reserved_mem;

	mem_regions_init(&free_mem);
	mem_regions_init(&reserved_mem);

	add_mem_region(&free_mem, ram_start, ram_আকার);

	i = fdt_add_reserved_regions(&free_mem, &reserved_mem, fdt);
	যদি (i < 0)
		ফেরত 0;

	ফেরত find_ram_শীর্ষ(&free_mem, &reserved_mem, আকার_2);
}
