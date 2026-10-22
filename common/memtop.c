// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Limited
 */

#अन्तर्भाव <fdt_support.h>
#अन्तर्भाव <fdtdec.h>
#अन्तर्भाव <memtop.h>

#अन्तर्भाव <asm/types.h>

#define MEM_RGN_गणनम्	64

संरचना region {
	phys_addr_t base;
	phys_size_t size;
};

संरचना mem_region {
	संरचना region rgn[MEM_RGN_गणनम्];
	uint गणनम्;
};

स्थिर शून्य add_mem_region(संरचना mem_region *mem_rgn, phys_addr_t base,
			   phys_size_t size)
{
	long i;

	प्रति (i = mem_rgn->गणनम्; i >= 0; i--) {
		जइ (i && base < mem_rgn->rgn[i - 1].base) {
			mem_rgn->rgn[i] = mem_rgn->rgn[i - 1];
		} अण्णहा {
			mem_rgn->rgn[i].base = base;
			mem_rgn->rgn[i].size = size;
			भङ्ग;
		}
	}

	mem_rgn->गणनम्++;
}

स्थिर शून्य mem_regions_init(संरचना mem_region *mem)
{
	uint i;

	mem->गणनम् = 0;
	प्रति (i = 0; i < MEM_RGN_गणनम्; i++) {
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

	जइ (fdt_check_header(fdt_blob) != 0)
		प्रत्यागम -1;

	/* process memreserve sections */
	total = fdt_अङ्कः_mem_rsv(fdt_blob);
	assert_noisy(total < MEM_RGN_गणनम्);
	प्रति (i = 0; i < total; i++) {
		जइ (fdt_get_mem_rsv(fdt_blob, i, &addr, &size) != 0)
			अनुवर्त;
		add_mem_region(reserved_mem, addr, size);
	}

	i = 0;
	/* process reserved-memory */
	nodeoffset = fdt_subnode_offset(fdt_blob, 0, "reserved-memory");
	जइ (nodeoffset >= 0) {
		subnode = fdt_first_subnode(fdt_blob, nodeoffset);
		जाव (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			जइ (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				addr = res.start;
				size = res.end - res.start + 1;
				assert_noisy(i < MEM_RGN_गणनम्);
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

	प्रति (i = 0; i < mem_rgn->गणनम्; i++) {
		phys_addr_t rgnbase = rgn[i].base;
		phys_size_t rgnsize = rgn[i].size;

		जइ (addrs_overlap(base, size, rgnbase, rgnsize))
			भङ्ग;
	}

	प्रत्यागम (i < mem_rgn->गणनम्) ? i : -1;
}

स्थिर phys_addr_t find_ram_शिखरम्(संरचना mem_region *free_mem,
				संरचना mem_region *reserved_mem, phys_size_t size)
{
	long i, rgn;
	phys_addr_t base = 0;
	phys_addr_t res_base;

	प्रति (i = free_mem->गणनम् - 1; i >= 0; i--) {
		phys_addr_t rgnbase = free_mem->rgn[i].base;
		phys_size_t rgnsize = free_mem->rgn[i].size;

		जइ (rgnsize < size)
			अनुवर्त;

		base = rgnbase + rgnsize - size;
		जाव (base && rgnbase <= base) {
			rgn = region_overlap_check(reserved_mem, base, size);
			जइ (rgn < 0)
				प्रत्यागम base;

			res_base = reserved_mem->rgn[rgn].base;
			जइ (res_base < size)
				भङ्ग;
			base = res_base - size;
		}
	}

	प्रत्यागम 0;
}

phys_addr_t get_mem_शिखरम्(phys_addr_t ram_start, phys_size_t ram_size,
			phys_size_t size, शून्य *fdt)
{
	पूर्णाङ्क i;
	संरचना mem_region free_mem;
	संरचना mem_region reserved_mem;

	mem_regions_init(&free_mem);
	mem_regions_init(&reserved_mem);

	add_mem_region(&free_mem, ram_start, ram_size);

	i = fdt_add_reserved_regions(&free_mem, &reserved_mem, fdt);
	जइ (i < 0)
		प्रत्यागम 0;

	प्रत्यागम find_ram_शिखरम्(&free_mem, &reserved_mem, size);
}
