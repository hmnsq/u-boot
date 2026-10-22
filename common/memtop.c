// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Limited
 */

#ضمن <fdt_support.h>
#ضمن <fdtdec.h>
#ضمن <memtop.h>

#ضمن <asm/types.h>

#define MEM_RGN_كدي	64

بنية region {
	phys_addr_t base;
	phys_size_t size;
};

بنية mem_region {
	بنية region rgn[MEM_RGN_كدي];
	uint كدي;
};

ساكن فراغ add_mem_region(بنية mem_region *mem_rgn, phys_addr_t base,
			   phys_size_t size)
{
	long i;

	لكل (i = mem_rgn->كدي; i >= 0; i--) {
		إذا (i && base < mem_rgn->rgn[i - 1].base) {
			mem_rgn->rgn[i] = mem_rgn->rgn[i - 1];
		} وإلا {
			mem_rgn->rgn[i].base = base;
			mem_rgn->rgn[i].size = size;
			اكسر;
		}
	}

	mem_rgn->كدي++;
}

ساكن فراغ mem_regions_init(بنية mem_region *mem)
{
	uint i;

	mem->كدي = 0;
	لكل (i = 0; i < MEM_RGN_كدي; i++) {
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

	إذا (fdt_check_header(fdt_blob) != 0)
		أرجع -1;

	/* process memreserve sections */
	total = fdt_لمب_mem_rsv(fdt_blob);
	assert_noisy(total < MEM_RGN_كدي);
	لكل (i = 0; i < total; i++) {
		إذا (fdt_get_mem_rsv(fdt_blob, i, &addr, &size) != 0)
			استمر;
		add_mem_region(reserved_mem, addr, size);
	}

	i = 0;
	/* process reserved-memory */
	nodeoffset = fdt_subnode_offset(fdt_blob, 0, "reserved-memory");
	إذا (nodeoffset >= 0) {
		subnode = fdt_first_subnode(fdt_blob, nodeoffset);
		طالما (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			إذا (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				addr = res.start;
				size = res.end - res.start + 1;
				assert_noisy(i < MEM_RGN_كدي);
				add_mem_region(reserved_mem, addr, size);
			}

			subnode = fdt_next_subnode(fdt_blob, subnode);
			++i;
		}
	}

	أرجع 0;
}

ساكن long addrs_overlap(phys_addr_t base1, phys_size_t size1,
			  phys_addr_t base2, phys_size_t size2)
{
	ثابت phys_addr_t base1_end = base1 + size1 - 1;
	ثابت phys_addr_t base2_end = base2 + size2 - 1;

	أرجع ((base1 <= base2_end) && (base2 <= base1_end));
}

ساكن long region_overlap_check(بنية mem_region *mem_rgn, phys_addr_t base,
				 phys_size_t size)
{
	unsigned long i;
	بنية region *rgn = mem_rgn->rgn;

	لكل (i = 0; i < mem_rgn->كدي; i++) {
		phys_addr_t rgnbase = rgn[i].base;
		phys_size_t rgnsize = rgn[i].size;

		إذا (addrs_overlap(base, size, rgnbase, rgnsize))
			اكسر;
	}

	أرجع (i < mem_rgn->كدي) ? i : -1;
}

ساكن phys_addr_t find_ram_كولي(بنية mem_region *free_mem,
				بنية mem_region *reserved_mem, phys_size_t size)
{
	long i, rgn;
	phys_addr_t base = 0;
	phys_addr_t res_base;

	لكل (i = free_mem->كدي - 1; i >= 0; i--) {
		phys_addr_t rgnbase = free_mem->rgn[i].base;
		phys_size_t rgnsize = free_mem->rgn[i].size;

		إذا (rgnsize < size)
			استمر;

		base = rgnbase + rgnsize - size;
		طالما (base && rgnbase <= base) {
			rgn = region_overlap_check(reserved_mem, base, size);
			إذا (rgn < 0)
				أرجع base;

			res_base = reserved_mem->rgn[rgn].base;
			إذا (res_base < size)
				اكسر;
			base = res_base - size;
		}
	}

	أرجع 0;
}

phys_addr_t get_mem_كولي(phys_addr_t ram_start, phys_size_t ram_size,
			phys_size_t size, فراغ *fdt)
{
	صحيح i;
	بنية mem_region free_mem;
	بنية mem_region reserved_mem;

	mem_regions_init(&free_mem);
	mem_regions_init(&reserved_mem);

	add_mem_region(&free_mem, ram_start, ram_size);

	i = fdt_add_reserved_regions(&free_mem, &reserved_mem, fdt);
	إذا (i < 0)
		أرجع 0;

	أرجع find_ram_كولي(&free_mem, &reserved_mem, size);
}
