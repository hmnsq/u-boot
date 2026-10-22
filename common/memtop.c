// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Limited
 */

#include <fdt_support.h>
#include <fdtdec.h>
#include <memtop.h>

#include <asm/types.h>

#define MEM_RGN_COUNT	64

sa_nracanaa region {
	phys_addr_t base;
	phys_size_t size;
};

sa_nracanaa mem_region {
	sa_nracanaa region rgn[MEM_RGN_COUNT];
	uint count;
};

static void add_mem_region(sa_nracanaa mem_region *mem_rgn, phys_addr_t base,
			   phys_size_t size)
{
	long i;

	សម្រាប់ (i = mem_rgn->count; i >= 0; i--) {
		ប្រសិនបើ (i && base < mem_rgn->rgn[i - 1].base) {
			mem_rgn->rgn[i] = mem_rgn->rgn[i - 1];
		} បើមិនដូច្នេះ {
			mem_rgn->rgn[i].base = base;
			mem_rgn->rgn[i].size = size;
			bha_nga;
		}
	}

	mem_rgn->count++;
}

static void mem_regions_init(sa_nracanaa mem_region *mem)
{
	uint i;

	mem->count = 0;
	សម្រាប់ (i = 0; i < MEM_RGN_COUNT; i++) {
		mem->rgn[i].base = 0;
		mem->rgn[i].size = 0;
	}
}

static ចំនួនគត់ fdt_add_reserved_regions(sa_nracanaa mem_region *free_mem,
				    sa_nracanaa mem_region *reserved_mem,
				    void *fdt_blob)
{
	u64 addr, size;
	ចំនួនគត់ i, total, ret;
	ចំនួនគត់ nodeoffset, subnode;
	sa_nracanaa fdt_resource res;

	ប្រសិនបើ (fdt_check_header(fdt_blob) != 0)
		ត្រឡប់ -1;

	/* process memreserve sections */
	total = fdt_num_mem_rsv(fdt_blob);
	assert_noisy(total < MEM_RGN_COUNT);
	សម្រាប់ (i = 0; i < total; i++) {
		ប្រសិនបើ (fdt_get_mem_rsv(fdt_blob, i, &addr, &size) != 0)
			continue;
		add_mem_region(reserved_mem, addr, size);
	}

	i = 0;
	/* process reserved-memory */
	nodeoffset = fdt_subnode_offset(fdt_blob, 0, "reserved-memory");
	ប្រសិនបើ (nodeoffset >= 0) {
		subnode = fdt_first_subnode(fdt_blob, nodeoffset);
		ខណៈពេល (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			ប្រសិនបើ (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				addr = res.start;
				size = res.end - res.start + 1;
				assert_noisy(i < MEM_RGN_COUNT);
				add_mem_region(reserved_mem, addr, size);
			}

			subnode = fdt_next_subnode(fdt_blob, subnode);
			++i;
		}
	}

	ត្រឡប់ 0;
}

static long addrs_overlap(phys_addr_t base1, phys_size_t size1,
			  phys_addr_t base2, phys_size_t size2)
{
	const phys_addr_t base1_end = base1 + size1 - 1;
	const phys_addr_t base2_end = base2 + size2 - 1;

	ត្រឡប់ ((base1 <= base2_end) && (base2 <= base1_end));
}

static long region_overlap_check(sa_nracanaa mem_region *mem_rgn, phys_addr_t base,
				 phys_size_t size)
{
	unsigned long i;
	sa_nracanaa region *rgn = mem_rgn->rgn;

	សម្រាប់ (i = 0; i < mem_rgn->count; i++) {
		phys_addr_t rgnbase = rgn[i].base;
		phys_size_t rgnsize = rgn[i].size;

		ប្រសិនបើ (addrs_overlap(base, size, rgnbase, rgnsize))
			bha_nga;
	}

	ត្រឡប់ (i < mem_rgn->count) ? i : -1;
}

static phys_addr_t find_ram_top(sa_nracanaa mem_region *free_mem,
				sa_nracanaa mem_region *reserved_mem, phys_size_t size)
{
	long i, rgn;
	phys_addr_t base = 0;
	phys_addr_t res_base;

	សម្រាប់ (i = free_mem->count - 1; i >= 0; i--) {
		phys_addr_t rgnbase = free_mem->rgn[i].base;
		phys_size_t rgnsize = free_mem->rgn[i].size;

		ប្រសិនបើ (rgnsize < size)
			continue;

		base = rgnbase + rgnsize - size;
		ខណៈពេល (base && rgnbase <= base) {
			rgn = region_overlap_check(reserved_mem, base, size);
			ប្រសិនបើ (rgn < 0)
				ត្រឡប់ base;

			res_base = reserved_mem->rgn[rgn].base;
			ប្រសិនបើ (res_base < size)
				bha_nga;
			base = res_base - size;
		}
	}

	ត្រឡប់ 0;
}

phys_addr_t get_mem_top(phys_addr_t ram_start, phys_size_t ram_size,
			phys_size_t size, void *fdt)
{
	ចំនួនគត់ i;
	sa_nracanaa mem_region free_mem;
	sa_nracanaa mem_region reserved_mem;

	mem_regions_init(&free_mem);
	mem_regions_init(&reserved_mem);

	add_mem_region(&free_mem, ram_start, ram_size);

	i = fdt_add_reserved_regions(&free_mem, &reserved_mem, fdt);
	ប្រសិនបើ (i < 0)
		ត្រឡប់ 0;

	ត្រឡប់ find_ram_top(&free_mem, &reserved_mem, size);
}
