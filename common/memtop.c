// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Limited
 */

#अन्तर्भाव <fdt_support.h>
#अन्तर्भाव <fdtdec.h>
#अन्तर्भाव <memtop.h>

#अन्तर्भाव <asm/types.h>

#define MEM_RGN_गन्ती	64

संरचना region {
	phys_addr_t base;
	phys_size_t size;
};

संरचना mem_region {
	संरचना region rgn[MEM_RGN_गन्ती];
	uint गन्ती;
};

स्थिर शून्य add_mem_region(संरचना mem_region *mem_rgn, phys_addr_t base,
			   phys_size_t size)
{
	दीर्घ i;

	लागि (i = mem_rgn->गन्ती; i >= 0; i--) {
		यदि (i && base < mem_rgn->rgn[i - 1].base) {
			mem_rgn->rgn[i] = mem_rgn->rgn[i - 1];
		} अन्यथा {
			mem_rgn->rgn[i].base = base;
			mem_rgn->rgn[i].size = size;
			रोक;
		}
	}

	mem_rgn->गन्ती++;
}

स्थिर शून्य mem_regions_init(संरचना mem_region *mem)
{
	uint i;

	mem->गन्ती = 0;
	लागि (i = 0; i < MEM_RGN_गन्ती; i++) {
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
		फर्काऊ -1;

	/* process memreserve sections */
	total = fdt_अङ्क_mem_rsv(fdt_blob);
	assert_noisy(total < MEM_RGN_गन्ती);
	लागि (i = 0; i < total; i++) {
		यदि (fdt_get_mem_rsv(fdt_blob, i, &addr, &size) != 0)
			जारी;
		add_mem_region(reserved_mem, addr, size);
	}

	i = 0;
	/* process reserved-memory */
	nodeoffset = fdt_subnode_offset(fdt_blob, 0, "reserved-memory");
	यदि (nodeoffset >= 0) {
		subnode = fdt_first_subnode(fdt_blob, nodeoffset);
		जबसम्म (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			यदि (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				addr = res.start;
				size = res.end - res.start + 1;
				assert_noisy(i < MEM_RGN_गन्ती);
				add_mem_region(reserved_mem, addr, size);
			}

			subnode = fdt_next_subnode(fdt_blob, subnode);
			++i;
		}
	}

	फर्काऊ 0;
}

स्थिर दीर्घ addrs_overlap(phys_addr_t base1, phys_size_t size1,
			  phys_addr_t base2, phys_size_t size2)
{
	अचर phys_addr_t base1_end = base1 + size1 - 1;
	अचर phys_addr_t base2_end = base2 + size2 - 1;

	फर्काऊ ((base1 <= base2_end) && (base2 <= base1_end));
}

स्थिर दीर्घ region_overlap_check(संरचना mem_region *mem_rgn, phys_addr_t base,
				 phys_size_t size)
{
	अचिन्हित दीर्घ i;
	संरचना region *rgn = mem_rgn->rgn;

	लागि (i = 0; i < mem_rgn->गन्ती; i++) {
		phys_addr_t rgnbase = rgn[i].base;
		phys_size_t rgnsize = rgn[i].size;

		यदि (addrs_overlap(base, size, rgnbase, rgnsize))
			रोक;
	}

	फर्काऊ (i < mem_rgn->गन्ती) ? i : -1;
}

स्थिर phys_addr_t find_ram_शीर्ष(संरचना mem_region *free_mem,
				संरचना mem_region *reserved_mem, phys_size_t size)
{
	दीर्घ i, rgn;
	phys_addr_t base = 0;
	phys_addr_t res_base;

	लागि (i = free_mem->गन्ती - 1; i >= 0; i--) {
		phys_addr_t rgnbase = free_mem->rgn[i].base;
		phys_size_t rgnsize = free_mem->rgn[i].size;

		यदि (rgnsize < size)
			जारी;

		base = rgnbase + rgnsize - size;
		जबसम्म (base && rgnbase <= base) {
			rgn = region_overlap_check(reserved_mem, base, size);
			यदि (rgn < 0)
				फर्काऊ base;

			res_base = reserved_mem->rgn[rgn].base;
			यदि (res_base < size)
				रोक;
			base = res_base - size;
		}
	}

	फर्काऊ 0;
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
		फर्काऊ 0;

	फर्काऊ find_ram_शीर्ष(&free_mem, &reserved_mem, size);
}
