// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Limited
 */

#અન્તર્ભાવ <fdt_support.h>
#અન્તર્ભાવ <fdtdec.h>
#અન્તર્ભાવ <memtop.h>

#અન્તર્ભાવ <asm/types.h>

#define MEM_RGN_ગણતરી	64

સંરચના region {
	phys_addr_t base;
	phys_size_t size;
};

સંરચના mem_region {
	સંરચના region rgn[MEM_RGN_ગણતરી];
	uint ગણતરી;
};

સ્થિર શૂન્ય add_mem_region(સંરચના mem_region *mem_rgn, phys_addr_t base,
			   phys_size_t size)
{
	long i;

	માટે (i = mem_rgn->ગણતરી; i >= 0; i--) {
		જો (i && base < mem_rgn->rgn[i - 1].base) {
			mem_rgn->rgn[i] = mem_rgn->rgn[i - 1];
		} નહીંતર {
			mem_rgn->rgn[i].base = base;
			mem_rgn->rgn[i].size = size;
			તોડો;
		}
	}

	mem_rgn->ગણતરી++;
}

સ્થિર શૂન્ય mem_regions_init(સંરચના mem_region *mem)
{
	uint i;

	mem->ગણતરી = 0;
	માટે (i = 0; i < MEM_RGN_ગણતરી; i++) {
		mem->rgn[i].base = 0;
		mem->rgn[i].size = 0;
	}
}

સ્થિર પૂર્ણાંક fdt_add_reserved_regions(સંરચના mem_region *free_mem,
				    સંરચના mem_region *reserved_mem,
				    શૂન્ય *fdt_blob)
{
	u64 addr, size;
	પૂર્ણાંક i, total, ret;
	પૂર્ણાંક nodeoffset, subnode;
	સંરચના fdt_resource res;

	જો (fdt_check_header(fdt_blob) != 0)
		પાછા -1;

	/* process memreserve sections */
	total = fdt_અંક_mem_rsv(fdt_blob);
	assert_noisy(total < MEM_RGN_ગણતરી);
	માટે (i = 0; i < total; i++) {
		જો (fdt_get_mem_rsv(fdt_blob, i, &addr, &size) != 0)
			ચાલુ;
		add_mem_region(reserved_mem, addr, size);
	}

	i = 0;
	/* process reserved-memory */
	nodeoffset = fdt_subnode_offset(fdt_blob, 0, "reserved-memory");
	જો (nodeoffset >= 0) {
		subnode = fdt_first_subnode(fdt_blob, nodeoffset);
		જ્યારે (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			જો (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				addr = res.start;
				size = res.end - res.start + 1;
				assert_noisy(i < MEM_RGN_ગણતરી);
				add_mem_region(reserved_mem, addr, size);
			}

			subnode = fdt_next_subnode(fdt_blob, subnode);
			++i;
		}
	}

	પાછા 0;
}

સ્થિર long addrs_overlap(phys_addr_t base1, phys_size_t size1,
			  phys_addr_t base2, phys_size_t size2)
{
	અચલ phys_addr_t base1_end = base1 + size1 - 1;
	અચલ phys_addr_t base2_end = base2 + size2 - 1;

	પાછા ((base1 <= base2_end) && (base2 <= base1_end));
}

સ્થિર long region_overlap_check(સંરચના mem_region *mem_rgn, phys_addr_t base,
				 phys_size_t size)
{
	unsigned long i;
	સંરચના region *rgn = mem_rgn->rgn;

	માટે (i = 0; i < mem_rgn->ગણતરી; i++) {
		phys_addr_t rgnbase = rgn[i].base;
		phys_size_t rgnsize = rgn[i].size;

		જો (addrs_overlap(base, size, rgnbase, rgnsize))
			તોડો;
	}

	પાછા (i < mem_rgn->ગણતરી) ? i : -1;
}

સ્થિર phys_addr_t find_ram_ટોચ(સંરચના mem_region *free_mem,
				સંરચના mem_region *reserved_mem, phys_size_t size)
{
	long i, rgn;
	phys_addr_t base = 0;
	phys_addr_t res_base;

	માટે (i = free_mem->ગણતરી - 1; i >= 0; i--) {
		phys_addr_t rgnbase = free_mem->rgn[i].base;
		phys_size_t rgnsize = free_mem->rgn[i].size;

		જો (rgnsize < size)
			ચાલુ;

		base = rgnbase + rgnsize - size;
		જ્યારે (base && rgnbase <= base) {
			rgn = region_overlap_check(reserved_mem, base, size);
			જો (rgn < 0)
				પાછા base;

			res_base = reserved_mem->rgn[rgn].base;
			જો (res_base < size)
				તોડો;
			base = res_base - size;
		}
	}

	પાછા 0;
}

phys_addr_t get_mem_ટોચ(phys_addr_t ram_start, phys_size_t ram_size,
			phys_size_t size, શૂન્ય *fdt)
{
	પૂર્ણાંક i;
	સંરચના mem_region free_mem;
	સંરચના mem_region reserved_mem;

	mem_regions_init(&free_mem);
	mem_regions_init(&reserved_mem);

	add_mem_region(&free_mem, ram_start, ram_size);

	i = fdt_add_reserved_regions(&free_mem, &reserved_mem, fdt);
	જો (i < 0)
		પાછા 0;

	પાછા find_ram_ટોચ(&free_mem, &reserved_mem, size);
}
