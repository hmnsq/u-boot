// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Limited
 */

#അന്തര്ഭാവ <fdt_support.h>
#അന്തര്ഭാവ <fdtdec.h>
#അന്തര്ഭാവ <memtop.h>

#അന്തര്ഭാവ <asm/types.h>

#define MEM_RGN_എണ്ണം	64

ഘടന region {
	phys_addr_t base;
	phys_size_t size;
};

ഘടന mem_region {
	ഘടന region rgn[MEM_RGN_എണ്ണം];
	uint എണ്ണം;
};

സ്ഥിതം ശൂന്യം add_mem_region(ഘടന mem_region *mem_rgn, phys_addr_t base,
			   phys_size_t size)
{
	long i;

	വേണ്ടി (i = mem_rgn->എണ്ണം; i >= 0; i--) {
		എങ്കിൽ (i && base < mem_rgn->rgn[i - 1].base) {
			mem_rgn->rgn[i] = mem_rgn->rgn[i - 1];
		} അല്ലെങ്കിൽ {
			mem_rgn->rgn[i].base = base;
			mem_rgn->rgn[i].size = size;
			നിർത്തുക;
		}
	}

	mem_rgn->എണ്ണം++;
}

സ്ഥിതം ശൂന്യം mem_regions_init(ഘടന mem_region *mem)
{
	uint i;

	mem->എണ്ണം = 0;
	വേണ്ടി (i = 0; i < MEM_RGN_എണ്ണം; i++) {
		mem->rgn[i].base = 0;
		mem->rgn[i].size = 0;
	}
}

സ്ഥിതം പൂർണ്ണസംഖ്യ fdt_add_reserved_regions(ഘടന mem_region *free_mem,
				    ഘടന mem_region *reserved_mem,
				    ശൂന്യം *fdt_blob)
{
	u64 addr, size;
	പൂർണ്ണസംഖ്യ i, total, ret;
	പൂർണ്ണസംഖ്യ nodeoffset, subnode;
	ഘടന fdt_resource res;

	എങ്കിൽ (fdt_check_header(fdt_blob) != 0)
		തിരികെ -1;

	/* process memreserve sections */
	total = fdt_അക്കം_mem_rsv(fdt_blob);
	assert_noisy(total < MEM_RGN_എണ്ണം);
	വേണ്ടി (i = 0; i < total; i++) {
		എങ്കിൽ (fdt_get_mem_rsv(fdt_blob, i, &addr, &size) != 0)
			തുടരുക;
		add_mem_region(reserved_mem, addr, size);
	}

	i = 0;
	/* process reserved-memory */
	nodeoffset = fdt_subnode_offset(fdt_blob, 0, "reserved-memory");
	എങ്കിൽ (nodeoffset >= 0) {
		subnode = fdt_first_subnode(fdt_blob, nodeoffset);
		ആകുമ്പോൾ (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			എങ്കിൽ (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				addr = res.start;
				size = res.end - res.start + 1;
				assert_noisy(i < MEM_RGN_എണ്ണം);
				add_mem_region(reserved_mem, addr, size);
			}

			subnode = fdt_next_subnode(fdt_blob, subnode);
			++i;
		}
	}

	തിരികെ 0;
}

സ്ഥിതം long addrs_overlap(phys_addr_t base1, phys_size_t size1,
			  phys_addr_t base2, phys_size_t size2)
{
	സ്ഥിരം phys_addr_t base1_end = base1 + size1 - 1;
	സ്ഥിരം phys_addr_t base2_end = base2 + size2 - 1;

	തിരികെ ((base1 <= base2_end) && (base2 <= base1_end));
}

സ്ഥിതം long region_overlap_check(ഘടന mem_region *mem_rgn, phys_addr_t base,
				 phys_size_t size)
{
	unsigned long i;
	ഘടന region *rgn = mem_rgn->rgn;

	വേണ്ടി (i = 0; i < mem_rgn->എണ്ണം; i++) {
		phys_addr_t rgnbase = rgn[i].base;
		phys_size_t rgnsize = rgn[i].size;

		എങ്കിൽ (addrs_overlap(base, size, rgnbase, rgnsize))
			നിർത്തുക;
	}

	തിരികെ (i < mem_rgn->എണ്ണം) ? i : -1;
}

സ്ഥിതം phys_addr_t find_ram_മുകൾ(ഘടന mem_region *free_mem,
				ഘടന mem_region *reserved_mem, phys_size_t size)
{
	long i, rgn;
	phys_addr_t base = 0;
	phys_addr_t res_base;

	വേണ്ടി (i = free_mem->എണ്ണം - 1; i >= 0; i--) {
		phys_addr_t rgnbase = free_mem->rgn[i].base;
		phys_size_t rgnsize = free_mem->rgn[i].size;

		എങ്കിൽ (rgnsize < size)
			തുടരുക;

		base = rgnbase + rgnsize - size;
		ആകുമ്പോൾ (base && rgnbase <= base) {
			rgn = region_overlap_check(reserved_mem, base, size);
			എങ്കിൽ (rgn < 0)
				തിരികെ base;

			res_base = reserved_mem->rgn[rgn].base;
			എങ്കിൽ (res_base < size)
				നിർത്തുക;
			base = res_base - size;
		}
	}

	തിരികെ 0;
}

phys_addr_t get_mem_മുകൾ(phys_addr_t ram_start, phys_size_t ram_size,
			phys_size_t size, ശൂന്യം *fdt)
{
	പൂർണ്ണസംഖ്യ i;
	ഘടന mem_region free_mem;
	ഘടന mem_region reserved_mem;

	mem_regions_init(&free_mem);
	mem_regions_init(&reserved_mem);

	add_mem_region(&free_mem, ram_start, ram_size);

	i = fdt_add_reserved_regions(&free_mem, &reserved_mem, fdt);
	എങ്കിൽ (i < 0)
		തിരികെ 0;

	തിരികെ find_ram_മുകൾ(&free_mem, &reserved_mem, size);
}
