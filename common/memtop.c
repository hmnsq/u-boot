// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Limited
 */

#अन्तर्भाव <fdt_support.h>
#अन्तर्भाव <fdtdec.h>
#अन्तर्भाव <memtop.h>

#अन्तर्भाव <asm/types.h>

#define MEM_RGN_मोजणी	64

संरचना region {
	phys_addr_t base;
	phys_size_t size;
};

संरचना mem_region {
	संरचना region rgn[MEM_RGN_मोजणी];
	uint मोजणी;
};

स्थिर शून्य add_mem_region(संरचना mem_region *mem_rgn, phys_addr_t base,
			   phys_size_t size)
{
	दीर्घ i;

	साठी (i = mem_rgn->मोजणी; i >= 0; i--) {
		जर (i && base < mem_rgn->rgn[i - 1].base) {
			mem_rgn->rgn[i] = mem_rgn->rgn[i - 1];
		} अन्यथा {
			mem_rgn->rgn[i].base = base;
			mem_rgn->rgn[i].size = size;
			थांबा;
		}
	}

	mem_rgn->मोजणी++;
}

स्थिर शून्य mem_regions_init(संरचना mem_region *mem)
{
	uint i;

	mem->मोजणी = 0;
	साठी (i = 0; i < MEM_RGN_मोजणी; i++) {
		mem->rgn[i].base = 0;
		mem->rgn[i].size = 0;
	}
}

स्थिर पूर्णांक fdt_add_reserved_regions(संरचना mem_region *free_mem,
				    संरचना mem_region *reserved_mem,
				    शून्य *fdt_blob)
{
	u64 addr, size;
	पूर्णांक i, total, ret;
	पूर्णांक nodeoffset, subnode;
	संरचना fdt_resource res;

	जर (fdt_check_header(fdt_blob) != 0)
		परत -1;

	/* process memreserve sections */
	total = fdt_अंक_mem_rsv(fdt_blob);
	assert_noisy(total < MEM_RGN_मोजणी);
	साठी (i = 0; i < total; i++) {
		जर (fdt_get_mem_rsv(fdt_blob, i, &addr, &size) != 0)
			सुरूठेवा;
		add_mem_region(reserved_mem, addr, size);
	}

	i = 0;
	/* process reserved-memory */
	nodeoffset = fdt_subnode_offset(fdt_blob, 0, "reserved-memory");
	जर (nodeoffset >= 0) {
		subnode = fdt_first_subnode(fdt_blob, nodeoffset);
		पर्यंत (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			जर (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				addr = res.start;
				size = res.end - res.start + 1;
				assert_noisy(i < MEM_RGN_मोजणी);
				add_mem_region(reserved_mem, addr, size);
			}

			subnode = fdt_next_subnode(fdt_blob, subnode);
			++i;
		}
	}

	परत 0;
}

स्थिर दीर्घ addrs_overlap(phys_addr_t base1, phys_size_t size1,
			  phys_addr_t base2, phys_size_t size2)
{
	स्थिरांक phys_addr_t base1_end = base1 + size1 - 1;
	स्थिरांक phys_addr_t base2_end = base2 + size2 - 1;

	परत ((base1 <= base2_end) && (base2 <= base1_end));
}

स्थिर दीर्घ region_overlap_check(संरचना mem_region *mem_rgn, phys_addr_t base,
				 phys_size_t size)
{
	अचिन्हित दीर्घ i;
	संरचना region *rgn = mem_rgn->rgn;

	साठी (i = 0; i < mem_rgn->मोजणी; i++) {
		phys_addr_t rgnbase = rgn[i].base;
		phys_size_t rgnsize = rgn[i].size;

		जर (addrs_overlap(base, size, rgnbase, rgnsize))
			थांबा;
	}

	परत (i < mem_rgn->मोजणी) ? i : -1;
}

स्थिर phys_addr_t find_ram_शीर्ष(संरचना mem_region *free_mem,
				संरचना mem_region *reserved_mem, phys_size_t size)
{
	दीर्घ i, rgn;
	phys_addr_t base = 0;
	phys_addr_t res_base;

	साठी (i = free_mem->मोजणी - 1; i >= 0; i--) {
		phys_addr_t rgnbase = free_mem->rgn[i].base;
		phys_size_t rgnsize = free_mem->rgn[i].size;

		जर (rgnsize < size)
			सुरूठेवा;

		base = rgnbase + rgnsize - size;
		पर्यंत (base && rgnbase <= base) {
			rgn = region_overlap_check(reserved_mem, base, size);
			जर (rgn < 0)
				परत base;

			res_base = reserved_mem->rgn[rgn].base;
			जर (res_base < size)
				थांबा;
			base = res_base - size;
		}
	}

	परत 0;
}

phys_addr_t get_mem_शीर्ष(phys_addr_t ram_start, phys_size_t ram_size,
			phys_size_t size, शून्य *fdt)
{
	पूर्णांक i;
	संरचना mem_region free_mem;
	संरचना mem_region reserved_mem;

	mem_regions_init(&free_mem);
	mem_regions_init(&reserved_mem);

	add_mem_region(&free_mem, ram_start, ram_size);

	i = fdt_add_reserved_regions(&free_mem, &reserved_mem, fdt);
	जर (i < 0)
		परत 0;

	परत find_ram_शीर्ष(&free_mem, &reserved_mem, size);
}
