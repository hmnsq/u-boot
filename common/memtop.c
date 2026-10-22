// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Limited
 */

#include <fdt_support.h>
#include <fdtdec.h>
#include <memtop.h>

#include <asm/types.h>

#define MEM_RGN_COUNT	64

struct region {
	phys_addr_t base;
	phys_size_t size;
};

struct mem_region {
	struct region rgn[MEM_RGN_COUNT];
	uint count;
};

static void add_mem_region(struct mem_region *mem_rgn, phys_addr_t base,
			   phys_size_t size)
{
	long i;

	위해 (i = mem_rgn->count; i >= 0; i--) {
		만약 (i && base < mem_rgn->rgn[i - 1].base) {
			mem_rgn->rgn[i] = mem_rgn->rgn[i - 1];
		} 아니면 {
			mem_rgn->rgn[i].base = base;
			mem_rgn->rgn[i].size = size;
			break;
		}
	}

	mem_rgn->count++;
}

static void mem_regions_init(struct mem_region *mem)
{
	uint i;

	mem->count = 0;
	위해 (i = 0; i < MEM_RGN_COUNT; i++) {
		mem->rgn[i].base = 0;
		mem->rgn[i].size = 0;
	}
}

static 정수 fdt_add_reserved_regions(struct mem_region *free_mem,
				    struct mem_region *reserved_mem,
				    void *fdt_blob)
{
	u64 addr, size;
	정수 i, total, ret;
	정수 nodeoffset, subnode;
	struct fdt_resource res;

	만약 (fdt_check_header(fdt_blob) != 0)
		반환 -1;

	/* process memreserve sections */
	total = fdt_num_mem_rsv(fdt_blob);
	assert_noisy(total < MEM_RGN_COUNT);
	위해 (i = 0; i < total; i++) {
		만약 (fdt_get_mem_rsv(fdt_blob, i, &addr, &size) != 0)
			continue;
		add_mem_region(reserved_mem, addr, size);
	}

	i = 0;
	/* process reserved-memory */
	nodeoffset = fdt_subnode_offset(fdt_blob, 0, "reserved-memory");
	만약 (nodeoffset >= 0) {
		subnode = fdt_first_subnode(fdt_blob, nodeoffset);
		동안 (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			만약 (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				addr = res.start;
				size = res.end - res.start + 1;
				assert_noisy(i < MEM_RGN_COUNT);
				add_mem_region(reserved_mem, addr, size);
			}

			subnode = fdt_next_subnode(fdt_blob, subnode);
			++i;
		}
	}

	반환 0;
}

static long addrs_overlap(phys_addr_t base1, phys_size_t size1,
			  phys_addr_t base2, phys_size_t size2)
{
	const phys_addr_t base1_end = base1 + size1 - 1;
	const phys_addr_t base2_end = base2 + size2 - 1;

	반환 ((base1 <= base2_end) && (base2 <= base1_end));
}

static long region_overlap_check(struct mem_region *mem_rgn, phys_addr_t base,
				 phys_size_t size)
{
	unsigned long i;
	struct region *rgn = mem_rgn->rgn;

	위해 (i = 0; i < mem_rgn->count; i++) {
		phys_addr_t rgnbase = rgn[i].base;
		phys_size_t rgnsize = rgn[i].size;

		만약 (addrs_overlap(base, size, rgnbase, rgnsize))
			break;
	}

	반환 (i < mem_rgn->count) ? i : -1;
}

static phys_addr_t find_ram_top(struct mem_region *free_mem,
				struct mem_region *reserved_mem, phys_size_t size)
{
	long i, rgn;
	phys_addr_t base = 0;
	phys_addr_t res_base;

	위해 (i = free_mem->count - 1; i >= 0; i--) {
		phys_addr_t rgnbase = free_mem->rgn[i].base;
		phys_size_t rgnsize = free_mem->rgn[i].size;

		만약 (rgnsize < size)
			continue;

		base = rgnbase + rgnsize - size;
		동안 (base && rgnbase <= base) {
			rgn = region_overlap_check(reserved_mem, base, size);
			만약 (rgn < 0)
				반환 base;

			res_base = reserved_mem->rgn[rgn].base;
			만약 (res_base < size)
				break;
			base = res_base - size;
		}
	}

	반환 0;
}

phys_addr_t get_mem_top(phys_addr_t ram_start, phys_size_t ram_size,
			phys_size_t size, void *fdt)
{
	정수 i;
	struct mem_region free_mem;
	struct mem_region reserved_mem;

	mem_regions_init(&free_mem);
	mem_regions_init(&reserved_mem);

	add_mem_region(&free_mem, ram_start, ram_size);

	i = fdt_add_reserved_regions(&free_mem, &reserved_mem, fdt);
	만약 (i < 0)
		반환 0;

	반환 find_ram_top(&free_mem, &reserved_mem, size);
}
