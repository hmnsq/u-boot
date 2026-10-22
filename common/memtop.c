// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Limited
 */

#অন্তর্ভাৱ <fdt_support.h>
#অন্তর্ভাৱ <fdtdec.h>
#অন্তর্ভাৱ <memtop.h>

#অন্তর্ভাৱ <asm/types.h>

#define MEM_RGN_গুনতি	64

সংরচনা region {
	phys_addr_t base;
	phys_size_t size;
};

সংরচনা mem_region {
	সংরচনা region rgn[MEM_RGN_গুনতি];
	uint গুনতি;
};

স্থির শূন্য add_mem_region(সংরচনা mem_region *mem_rgn, phys_addr_t base,
			   phys_size_t size)
{
	long i;

	বাবে (i = mem_rgn->গুনতি; i >= 0; i--) {
		যদি (i && base < mem_rgn->rgn[i - 1].base) {
			mem_rgn->rgn[i] = mem_rgn->rgn[i - 1];
		} অন্যথা {
			mem_rgn->rgn[i].base = base;
			mem_rgn->rgn[i].size = size;
			ভঙ্গ;
		}
	}

	mem_rgn->গুনতি++;
}

স্থির শূন্য mem_regions_init(সংরচনা mem_region *mem)
{
	uint i;

	mem->গুনতি = 0;
	বাবে (i = 0; i < MEM_RGN_গুনতি; i++) {
		mem->rgn[i].base = 0;
		mem->rgn[i].size = 0;
	}
}

স্থির পূর্ণাঙ্ক fdt_add_reserved_regions(সংরচনা mem_region *free_mem,
				    সংরচনা mem_region *reserved_mem,
				    শূন্য *fdt_blob)
{
	u64 addr, size;
	পূর্ণাঙ্ক i, total, ret;
	পূর্ণাঙ্ক nodeoffset, subnode;
	সংরচনা fdt_resource res;

	যদি (fdt_check_header(fdt_blob) != 0)
		ঘূৰাই -1;

	/* process memreserve sections */
	total = fdt_অঙ্ক_mem_rsv(fdt_blob);
	assert_noisy(total < MEM_RGN_গুনতি);
	বাবে (i = 0; i < total; i++) {
		যদি (fdt_get_mem_rsv(fdt_blob, i, &addr, &size) != 0)
			অনুৱর্ত;
		add_mem_region(reserved_mem, addr, size);
	}

	i = 0;
	/* process reserved-memory */
	nodeoffset = fdt_subnode_offset(fdt_blob, 0, "reserved-memory");
	যদি (nodeoffset >= 0) {
		subnode = fdt_first_subnode(fdt_blob, nodeoffset);
		যেতিয়ালৈ (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			যদি (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				addr = res.start;
				size = res.end - res.start + 1;
				assert_noisy(i < MEM_RGN_গুনতি);
				add_mem_region(reserved_mem, addr, size);
			}

			subnode = fdt_next_subnode(fdt_blob, subnode);
			++i;
		}
	}

	ঘূৰাই 0;
}

স্থির long addrs_overlap(phys_addr_t base1, phys_size_t size1,
			  phys_addr_t base2, phys_size_t size2)
{
	ধ্রুৱ phys_addr_t base1_end = base1 + size1 - 1;
	ধ্রুৱ phys_addr_t base2_end = base2 + size2 - 1;

	ঘূৰাই ((base1 <= base2_end) && (base2 <= base1_end));
}

স্থির long region_overlap_check(সংরচনা mem_region *mem_rgn, phys_addr_t base,
				 phys_size_t size)
{
	unsigned long i;
	সংরচনা region *rgn = mem_rgn->rgn;

	বাবে (i = 0; i < mem_rgn->গুনতি; i++) {
		phys_addr_t rgnbase = rgn[i].base;
		phys_size_t rgnsize = rgn[i].size;

		যদি (addrs_overlap(base, size, rgnbase, rgnsize))
			ভঙ্গ;
	}

	ঘূৰাই (i < mem_rgn->গুনতি) ? i : -1;
}

স্থির phys_addr_t find_ram_শীৰ্ষ(সংরচনা mem_region *free_mem,
				সংরচনা mem_region *reserved_mem, phys_size_t size)
{
	long i, rgn;
	phys_addr_t base = 0;
	phys_addr_t res_base;

	বাবে (i = free_mem->গুনতি - 1; i >= 0; i--) {
		phys_addr_t rgnbase = free_mem->rgn[i].base;
		phys_size_t rgnsize = free_mem->rgn[i].size;

		যদি (rgnsize < size)
			অনুৱর্ত;

		base = rgnbase + rgnsize - size;
		যেতিয়ালৈ (base && rgnbase <= base) {
			rgn = region_overlap_check(reserved_mem, base, size);
			যদি (rgn < 0)
				ঘূৰাই base;

			res_base = reserved_mem->rgn[rgn].base;
			যদি (res_base < size)
				ভঙ্গ;
			base = res_base - size;
		}
	}

	ঘূৰাই 0;
}

phys_addr_t get_mem_শীৰ্ষ(phys_addr_t ram_start, phys_size_t ram_size,
			phys_size_t size, শূন্য *fdt)
{
	পূর্ণাঙ্ক i;
	সংরচনা mem_region free_mem;
	সংরচনা mem_region reserved_mem;

	mem_regions_init(&free_mem);
	mem_regions_init(&reserved_mem);

	add_mem_region(&free_mem, ram_start, ram_size);

	i = fdt_add_reserved_regions(&free_mem, &reserved_mem, fdt);
	যদি (i < 0)
		ঘূৰাই 0;

	ঘূৰাই find_ram_শীৰ্ষ(&free_mem, &reserved_mem, size);
}
