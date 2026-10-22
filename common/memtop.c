// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Limited
 */

#समावेश <fdt_support.h>
#समावेश <fdtdec.h>
#समावेश <memtop.h>

#समावेश <asm/types.h>

#घोषणा MEM_RGN_गिनती	64

काष्ठा region {
	phys_addr_t base;
	phys_आकार_t आकार;
};

काष्ठा mem_region {
	काष्ठा region rgn[MEM_RGN_गिनती];
	uint गिनती;
};

अटल व्योम add_mem_region(काष्ठा mem_region *mem_rgn, phys_addr_t base,
			   phys_आकार_t आकार)
{
	दीर्घ i;

	क्रम (i = mem_rgn->गिनती; i >= 0; i--) {
		अगर (i && base < mem_rgn->rgn[i - 1].base) {
			mem_rgn->rgn[i] = mem_rgn->rgn[i - 1];
		} अन्यथा {
			mem_rgn->rgn[i].base = base;
			mem_rgn->rgn[i].आकार = आकार;
			अवरोध;
		}
	}

	mem_rgn->गिनती++;
}

अटल व्योम mem_regions_init(काष्ठा mem_region *mem)
{
	uint i;

	mem->गिनती = 0;
	क्रम (i = 0; i < MEM_RGN_गिनती; i++) {
		mem->rgn[i].base = 0;
		mem->rgn[i].आकार = 0;
	}
}

अटल पूर्णांक fdt_add_reserved_regions(काष्ठा mem_region *free_mem,
				    काष्ठा mem_region *reserved_mem,
				    व्योम *fdt_blob)
{
	u64 addr, आकार;
	पूर्णांक i, योग_2, ret;
	पूर्णांक nodeoffset, subnode;
	काष्ठा fdt_resource res;

	अगर (fdt_check_header(fdt_blob) != 0)
		वापस -1;

	/* process memreserve sections */
	योग_2 = fdt_अंक_mem_rsv(fdt_blob);
	assert_noisy(योग_2 < MEM_RGN_गिनती);
	क्रम (i = 0; i < योग_2; i++) {
		अगर (fdt_get_mem_rsv(fdt_blob, i, &addr, &आकार) != 0)
			जारी;
		add_mem_region(reserved_mem, addr, आकार);
	}

	i = 0;
	/* process reserved-memory */
	nodeoffset = fdt_subnode_offset(fdt_blob, 0, "reserved-memory");
	अगर (nodeoffset >= 0) {
		subnode = fdt_first_subnode(fdt_blob, nodeoffset);
		जबतक (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			अगर (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				addr = res.start;
				आकार = res.end - res.start + 1;
				assert_noisy(i < MEM_RGN_गिनती);
				add_mem_region(reserved_mem, addr, आकार);
			}

			subnode = fdt_अग्र_subnode(fdt_blob, subnode);
			++i;
		}
	}

	वापस 0;
}

अटल दीर्घ addrs_overlap(phys_addr_t base1, phys_आकार_t size1,
			  phys_addr_t base2, phys_आकार_t size2)
{
	स्थिर phys_addr_t base1_end = base1 + size1 - 1;
	स्थिर phys_addr_t base2_end = base2 + size2 - 1;

	वापस ((base1 <= base2_end) && (base2 <= base1_end));
}

अटल दीर्घ region_overlap_check(काष्ठा mem_region *mem_rgn, phys_addr_t base,
				 phys_आकार_t आकार)
{
	अचिन्हित दीर्घ i;
	काष्ठा region *rgn = mem_rgn->rgn;

	क्रम (i = 0; i < mem_rgn->गिनती; i++) {
		phys_addr_t rgnbase = rgn[i].base;
		phys_आकार_t rgnsize = rgn[i].आकार;

		अगर (addrs_overlap(base, आकार, rgnbase, rgnsize))
			अवरोध;
	}

	वापस (i < mem_rgn->गिनती) ? i : -1;
}

अटल phys_addr_t find_ram_शीर्ष(काष्ठा mem_region *free_mem,
				काष्ठा mem_region *reserved_mem, phys_आकार_t आकार)
{
	दीर्घ i, rgn;
	phys_addr_t base = 0;
	phys_addr_t res_base;

	क्रम (i = free_mem->गिनती - 1; i >= 0; i--) {
		phys_addr_t rgnbase = free_mem->rgn[i].base;
		phys_आकार_t rgnsize = free_mem->rgn[i].आकार;

		अगर (rgnsize < आकार)
			जारी;

		base = rgnbase + rgnsize - आकार;
		जबतक (base && rgnbase <= base) {
			rgn = region_overlap_check(reserved_mem, base, आकार);
			अगर (rgn < 0)
				वापस base;

			res_base = reserved_mem->rgn[rgn].base;
			अगर (res_base < आकार)
				अवरोध;
			base = res_base - आकार;
		}
	}

	वापस 0;
}

phys_addr_t get_mem_शीर्ष(phys_addr_t ram_start, phys_आकार_t ram_आकार,
			phys_आकार_t आकार, व्योम *fdt)
{
	पूर्णांक i;
	काष्ठा mem_region free_mem;
	काष्ठा mem_region reserved_mem;

	mem_regions_init(&free_mem);
	mem_regions_init(&reserved_mem);

	add_mem_region(&free_mem, ram_start, ram_आकार);

	i = fdt_add_reserved_regions(&free_mem, &reserved_mem, fdt);
	अगर (i < 0)
		वापस 0;

	वापस find_ram_शीर्ष(&free_mem, &reserved_mem, आकार);
}
