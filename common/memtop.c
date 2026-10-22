// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Limited
 */

#अन्तर्भाव <fdt_support.h>
#अन्तर्भाव <fdtdec.h>
#अन्तर्भाव <memtop.h>

#अन्तर्भाव <asm/types.h>

#define MEM_RGN_गिनती	64

संरचना region {
	phys_addr_t base;
	phys_size_t size;
};

संरचना mem_region {
	संरचना region rgn[MEM_RGN_गिनती];
	uint गिनती;
};

स्थिर शून्य add_mem_region(संरचना mem_region *mem_rgn, phys_addr_t base,
			   phys_size_t size)
{
	long i;

	प्रति (i = mem_rgn->गिनती; i >= 0; i--) {
		यदि (i && base < mem_rgn->rgn[i - 1].base) {
			mem_rgn->rgn[i] = mem_rgn->rgn[i - 1];
		} अन्यथा {
			mem_rgn->rgn[i].base = base;
			mem_rgn->rgn[i].size = size;
			भङ्ग;
		}
	}

	mem_rgn->गिनती++;
}

स्थिर शून्य mem_regions_init(संरचना mem_region *mem)
{
	uint i;

	mem->गिनती = 0;
	प्रति (i = 0; i < MEM_RGN_गिनती; i++) {
		mem->rgn[i].base = 0;
		mem->rgn[i].size = 0;
	}
}

स्थिर पूर्णाङ्क fdt_add_reserved_regions(संरचना mem_region *free_mem,
				    संरचना mem_region *reserved_mem,
				    शून्य *fdt_blob)
{
	u64 addr, size;
	पूर्णाङ्क i, total, ret;
	पूर्णाङ्क nodeoffset, subnode;
	संरचना fdt_resource res;

	यदि (fdt_check_header(fdt_blob) != 0)
		प्रत्यागम -1;

	/* process memreserve sections */
	total = fdt_अंक_mem_rsv(fdt_blob);
	assert_noisy(total < MEM_RGN_गिनती);
	प्रति (i = 0; i < total; i++) {
		यदि (fdt_get_mem_rsv(fdt_blob, i, &addr, &size) != 0)
			अनुवर्त;
		add_mem_region(reserved_mem, addr, size);
	}

	i = 0;
	/* process reserved-memory */
	nodeoffset = fdt_subnode_offset(fdt_blob, 0, "reserved-memory");
	यदि (nodeoffset >= 0) {
		subnode = fdt_first_subnode(fdt_blob, nodeoffset);
		यावत् (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			यदि (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				addr = res.start;
				size = res.end - res.start + 1;
				assert_noisy(i < MEM_RGN_गिनती);
				add_mem_region(reserved_mem, addr, size);
			}

			subnode = fdt_next_subnode(fdt_blob, subnode);
			++i;
		}
	}

	प्रत्यागम 0;
}

स्थिर long addrs_overlap(phys_addr_t base1, phys_size_t size1,
			  phys_addr_t base2, phys_size_t size2)
{
	ध्रुव phys_addr_t base1_end = base1 + size1 - 1;
	ध्रुव phys_addr_t base2_end = base2 + size2 - 1;

	प्रत्यागम ((base1 <= base2_end) && (base2 <= base1_end));
}

स्थिर long region_overlap_check(संरचना mem_region *mem_rgn, phys_addr_t base,
				 phys_size_t size)
{
	unsigned long i;
	संरचना region *rgn = mem_rgn->rgn;

	प्रति (i = 0; i < mem_rgn->गिनती; i++) {
		phys_addr_t rgnbase = rgn[i].base;
		phys_size_t rgnsize = rgn[i].size;

		यदि (addrs_overlap(base, size, rgnbase, rgnsize))
			भङ्ग;
	}

	प्रत्यागम (i < mem_rgn->गिनती) ? i : -1;
}

स्थिर phys_addr_t find_ram_शीर्ष(संरचना mem_region *free_mem,
				संरचना mem_region *reserved_mem, phys_size_t size)
{
	long i, rgn;
	phys_addr_t base = 0;
	phys_addr_t res_base;

	प्रति (i = free_mem->गिनती - 1; i >= 0; i--) {
		phys_addr_t rgnbase = free_mem->rgn[i].base;
		phys_size_t rgnsize = free_mem->rgn[i].size;

		यदि (rgnsize < size)
			अनुवर्त;

		base = rgnbase + rgnsize - size;
		यावत् (base && rgnbase <= base) {
			rgn = region_overlap_check(reserved_mem, base, size);
			यदि (rgn < 0)
				प्रत्यागम base;

			res_base = reserved_mem->rgn[rgn].base;
			यदि (res_base < size)
				भङ्ग;
			base = res_base - size;
		}
	}

	प्रत्यागम 0;
}

phys_addr_t get_mem_शीर्ष(phys_addr_t ram_start, phys_size_t ram_size,
			phys_size_t size, शून्य *fdt)
{
	पूर्णाङ्क i;
	संरचना mem_region free_mem;
	संरचना mem_region reserved_mem;

	mem_regions_init(&free_mem);
	mem_regions_init(&reserved_mem);

	add_mem_region(&free_mem, ram_start, ram_size);

	i = fdt_add_reserved_regions(&free_mem, &reserved_mem, fdt);
	यदि (i < 0)
		प्रत्यागम 0;

	प्रत्यागम find_ram_शीर्ष(&free_mem, &reserved_mem, size);
}
