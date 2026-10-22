// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Limited
 */

#ضمن <fdt_support.h>
#ضمن <fdtdec.h>
#ضمن <memtop.h>

#ضمن <asm/types.h>

#define MEM_RGN_COUNT	64

بنية region {
	phys_addr_t base;
	phys_size_t size;
};

بنية mem_region {
	بنية region rgn[MEM_RGN_COUNT];
	uint count;
};

ساكن فراغ add_mem_region(بنية mem_region *mem_rgn, phys_addr_t base,
			   phys_size_t size)
{
	long i;

	עבור (i = mem_rgn->count; i >= 0; i--) {
		אם (i && base < mem_rgn->rgn[i - 1].base) {
			mem_rgn->rgn[i] = mem_rgn->rgn[i - 1];
		} אחרת {
			mem_rgn->rgn[i].base = base;
			mem_rgn->rgn[i].size = size;
			اكسر;
		}
	}

	mem_rgn->count++;
}

ساكن فراغ mem_regions_init(بنية mem_region *mem)
{
	uint i;

	mem->count = 0;
	עבור (i = 0; i < MEM_RGN_COUNT; i++) {
		mem->rgn[i].base = 0;
		mem->rgn[i].size = 0;
	}
}

ساكن صحيح fdt_add_reserved_regions(بنية mem_region *free_mem,
				    بنية mem_region *reserved_mem,
				    فراغ *fdt_blob)
{
	u64 addr, size;
	صحيح i, total, ret;
	صحيح nodeoffset, subnode;
	بنية fdt_resource res;

	אם (fdt_check_header(fdt_blob) != 0)
		החזר -1;

	/* process memreserve sections */
	total = fdt_num_mem_rsv(fdt_blob);
	assert_noisy(total < MEM_RGN_COUNT);
	עבור (i = 0; i < total; i++) {
		אם (fdt_get_mem_rsv(fdt_blob, i, &addr, &size) != 0)
			استمر;
		add_mem_region(reserved_mem, addr, size);
	}

	i = 0;
	/* process reserved-memory */
	nodeoffset = fdt_subnode_offset(fdt_blob, 0, "reserved-memory");
	אם (nodeoffset >= 0) {
		subnode = fdt_first_subnode(fdt_blob, nodeoffset);
		כלעוד (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			אם (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				addr = res.start;
				size = res.end - res.start + 1;
				assert_noisy(i < MEM_RGN_COUNT);
				add_mem_region(reserved_mem, addr, size);
			}

			subnode = fdt_next_subnode(fdt_blob, subnode);
			++i;
		}
	}

	החזר 0;
}

ساكن long addrs_overlap(phys_addr_t base1, phys_size_t size1,
			  phys_addr_t base2, phys_size_t size2)
{
	ثابت phys_addr_t base1_end = base1 + size1 - 1;
	ثابت phys_addr_t base2_end = base2 + size2 - 1;

	החזר ((base1 <= base2_end) && (base2 <= base1_end));
}

ساكن long region_overlap_check(بنية mem_region *mem_rgn, phys_addr_t base,
				 phys_size_t size)
{
	unsigned long i;
	بنية region *rgn = mem_rgn->rgn;

	עבור (i = 0; i < mem_rgn->count; i++) {
		phys_addr_t rgnbase = rgn[i].base;
		phys_size_t rgnsize = rgn[i].size;

		אם (addrs_overlap(base, size, rgnbase, rgnsize))
			اكسر;
	}

	החזר (i < mem_rgn->count) ? i : -1;
}

ساكن phys_addr_t find_ram_top(بنية mem_region *free_mem,
				بنية mem_region *reserved_mem, phys_size_t size)
{
	long i, rgn;
	phys_addr_t base = 0;
	phys_addr_t res_base;

	עבור (i = free_mem->count - 1; i >= 0; i--) {
		phys_addr_t rgnbase = free_mem->rgn[i].base;
		phys_size_t rgnsize = free_mem->rgn[i].size;

		אם (rgnsize < size)
			استمر;

		base = rgnbase + rgnsize - size;
		כלעוד (base && rgnbase <= base) {
			rgn = region_overlap_check(reserved_mem, base, size);
			אם (rgn < 0)
				החזר base;

			res_base = reserved_mem->rgn[rgn].base;
			אם (res_base < size)
				اكسر;
			base = res_base - size;
		}
	}

	החזר 0;
}

phys_addr_t get_mem_top(phys_addr_t ram_start, phys_size_t ram_size,
			phys_size_t size, فراغ *fdt)
{
	صحيح i;
	بنية mem_region free_mem;
	بنية mem_region reserved_mem;

	mem_regions_init(&free_mem);
	mem_regions_init(&reserved_mem);

	add_mem_region(&free_mem, ram_start, ram_size);

	i = fdt_add_reserved_regions(&free_mem, &reserved_mem, fdt);
	אם (i < 0)
		החזר 0;

	החזר find_ram_top(&free_mem, &reserved_mem, size);
}
