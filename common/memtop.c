// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Limited
 */

#अन्तर्भाव <fdt_support.h>
#अन्तर्भाव <fdtdec.h>
#अन्तर्भाव <memtop.h>

#अन्तर्भाव <asm/types.h>

#परिभाषय MEM_RGN_गणनम्	64

संरचना region {
	phys_addr_t आधारः;
	phys_आकार_t आकार;
};

संरचना mem_region {
	संरचना region rgn[MEM_RGN_गणनम्];
	uint गणनम्;
};

स्थिर शून्य सङ्कलय_mem_region(संरचना mem_region *mem_rgn, phys_addr_t आधारः,
			   phys_आकार_t आकार)
{
	दीर्घ i;

	प्रति (i = mem_rgn->गणनम्; i >= 0; i--) {
		यदि (i && आधारः < mem_rgn->rgn[i - 1].आधारः) {
			mem_rgn->rgn[i] = mem_rgn->rgn[i - 1];
		} अन्यथा {
			mem_rgn->rgn[i].आधारः = आधारः;
			mem_rgn->rgn[i].आकार = आकार;
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
		mem->rgn[i].आधारः = 0;
		mem->rgn[i].आकार = 0;
	}
}

स्थिर पूर्णाङ्क fdt_सङ्कलय_reserved_regions(संरचना mem_region *free_mem,
				    संरचना mem_region *reserved_mem,
				    शून्य *fdt_blob)
{
	u64 addr, आकार;
	पूर्णाङ्क i, योग_2, ret;
	पूर्णाङ्क nodeoffset, subnode;
	संरचना fdt_resource res;

	यदि (fdt_check_शीर्षकम्(fdt_blob) != 0)
		प्रत्यागम -1;

	/* process memreserve sections */
	योग_2 = fdt_अङ्कः_mem_rsv(fdt_blob);
	assert_noisy(योग_2 < MEM_RGN_गणनम्);
	प्रति (i = 0; i < योग_2; i++) {
		यदि (fdt_get_mem_rsv(fdt_blob, i, &addr, &आकार) != 0)
			अनुवर्त;
		सङ्कलय_mem_region(reserved_mem, addr, आकार);
	}

	i = 0;
	/* process reserved-memory */
	nodeoffset = fdt_subnode_विस्थापनम्(fdt_blob, 0, "reserved-memory");
	यदि (nodeoffset >= 0) {
		subnode = fdt_first_subnode(fdt_blob, nodeoffset);
		यावत् (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			यदि (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				addr = res.start;
				आकार = res.अन्तः_2 - res.start + 1;
				assert_noisy(i < MEM_RGN_गणनम्);
				सङ्कलय_mem_region(reserved_mem, addr, आकार);
			}

			subnode = fdt_अग्र_subnode(fdt_blob, subnode);
			++i;
		}
	}

	प्रत्यागम 0;
}

स्थिर दीर्घ addrs_overlap(phys_addr_t base1, phys_आकार_t size1,
			  phys_addr_t base2, phys_आकार_t size2)
{
	ध्रुव phys_addr_t base1_अन्तः = base1 + size1 - 1;
	ध्रुव phys_addr_t base2_अन्तः = base2 + size2 - 1;

	प्रत्यागम ((base1 <= base2_अन्तः) && (base2 <= base1_अन्तः));
}

स्थिर दीर्घ region_overlap_check(संरचना mem_region *mem_rgn, phys_addr_t आधारः,
				 phys_आकार_t आकार)
{
	अचिह्नित दीर्घ i;
	संरचना region *rgn = mem_rgn->rgn;

	प्रति (i = 0; i < mem_rgn->गणनम्; i++) {
		phys_addr_t rgnbase = rgn[i].आधारः;
		phys_आकार_t rgnsize = rgn[i].आकार;

		यदि (addrs_overlap(आधारः, आकार, rgnbase, rgnsize))
			भङ्ग;
	}

	प्रत्यागम (i < mem_rgn->गणनम्) ? i : -1;
}

स्थिर phys_addr_t find_ram_शिखरम्(संरचना mem_region *free_mem,
				संरचना mem_region *reserved_mem, phys_आकार_t आकार)
{
	दीर्घ i, rgn;
	phys_addr_t आधारः = 0;
	phys_addr_t res_आधारः;

	प्रति (i = free_mem->गणनम् - 1; i >= 0; i--) {
		phys_addr_t rgnbase = free_mem->rgn[i].आधारः;
		phys_आकार_t rgnsize = free_mem->rgn[i].आकार;

		यदि (rgnsize < आकार)
			अनुवर्त;

		आधारः = rgnbase + rgnsize - आकार;
		यावत् (आधारः && rgnbase <= आधारः) {
			rgn = region_overlap_check(reserved_mem, आधारः, आकार);
			यदि (rgn < 0)
				प्रत्यागम आधारः;

			res_आधारः = reserved_mem->rgn[rgn].आधारः;
			यदि (res_आधारः < आकार)
				भङ्ग;
			आधारः = res_आधारः - आकार;
		}
	}

	प्रत्यागम 0;
}

phys_addr_t get_mem_शिखरम्(phys_addr_t ram_start, phys_आकार_t ram_आकार,
			phys_आकार_t आकार, शून्य *fdt)
{
	पूर्णाङ्क i;
	संरचना mem_region free_mem;
	संरचना mem_region reserved_mem;

	mem_regions_init(&free_mem);
	mem_regions_init(&reserved_mem);

	सङ्कलय_mem_region(&free_mem, ram_start, ram_आकार);

	i = fdt_सङ्कलय_reserved_regions(&free_mem, &reserved_mem, fdt);
	यदि (i < 0)
		प्रत्यागम 0;

	प्रत्यागम find_ram_शिखरम्(&free_mem, &reserved_mem, आकार);
}
