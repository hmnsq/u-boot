// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Limited
 */

#ಅನ್ತರ್ಭಾವ <fdt_support.h>
#ಅನ್ತರ್ಭಾವ <fdtdec.h>
#ಅನ್ತರ್ಭಾವ <memtop.h>

#ಅನ್ತರ್ಭಾವ <asm/types.h>

#ವ್ಯಾಖ್ಯಾನಿಸು MEM_RGN_ಎಣಿಕೆ	64

ರಚನೆ region {
	phys_addr_t ತಳ;
	phys_ಗಾತ್ರ_t ಗಾತ್ರ_2;
};

ರಚನೆ mem_region {
	ರಚನೆ region rgn[MEM_RGN_ಎಣಿಕೆ];
	uint ಎಣಿಕೆ_2;
};

ಸ್ಥಿರ ಶೂನ್ಯ ಕೂಡಿಸು_mem_region(ರಚನೆ mem_region *mem_rgn, phys_addr_t ತಳ,
			   phys_ಗಾತ್ರ_t ಗಾತ್ರ_2)
{
	ದೀರ್ಘ i;

	ಗಾಗಿ (i = mem_rgn->ಎಣಿಕೆ_2; i >= 0; i--) {
		ಆದರೆ (i && ತಳ < mem_rgn->rgn[i - 1].ತಳ) {
			mem_rgn->rgn[i] = mem_rgn->rgn[i - 1];
		} ಇಲ್ಲದಿದ್ದರೆ {
			mem_rgn->rgn[i].ತಳ = ತಳ;
			mem_rgn->rgn[i].ಗಾತ್ರ_2 = ಗಾತ್ರ_2;
			ನಿಲ್ಲಿಸು;
		}
	}

	mem_rgn->ಎಣಿಕೆ_2++;
}

ಸ್ಥಿರ ಶೂನ್ಯ mem_regions_init(ರಚನೆ mem_region *mem)
{
	uint i;

	mem->ಎಣಿಕೆ_2 = 0;
	ಗಾಗಿ (i = 0; i < MEM_RGN_ಎಣಿಕೆ; i++) {
		mem->rgn[i].ತಳ = 0;
		mem->rgn[i].ಗಾತ್ರ_2 = 0;
	}
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fdt_ಕೂಡಿಸು_reserved_regions(ರಚನೆ mem_region *free_mem,
				    ರಚನೆ mem_region *reserved_mem,
				    ಶೂನ್ಯ *fdt_blob)
{
	u64 addr, ಗಾತ್ರ_2;
	ಪೂರ್ಣಾಂಕ i, ಒಟ್ಟು, ret;
	ಪೂರ್ಣಾಂಕ nodeoffset, subnode;
	ರಚನೆ fdt_resource res;

	ಆದರೆ (fdt_check_ಶೀರ್ಷಿಕೆ(fdt_blob) != 0)
		ಹಿಂತಿರುಗಿ -1;

	/* process memreserve sections */
	ಒಟ್ಟು = fdt_ಅಂಕೆ_mem_rsv(fdt_blob);
	assert_noisy(ಒಟ್ಟು < MEM_RGN_ಎಣಿಕೆ);
	ಗಾಗಿ (i = 0; i < ಒಟ್ಟು; i++) {
		ಆದರೆ (fdt_get_mem_rsv(fdt_blob, i, &addr, &ಗಾತ್ರ_2) != 0)
			ಮುಂದುವರಿ;
		ಕೂಡಿಸು_mem_region(reserved_mem, addr, ಗಾತ್ರ_2);
	}

	i = 0;
	/* process reserved-memory */
	nodeoffset = fdt_subnode_ಸ್ಥಾನಾಂತರ(fdt_blob, 0, "reserved-memory");
	ಆದರೆ (nodeoffset >= 0) {
		subnode = fdt_ಮೊದಲ_subnode(fdt_blob, nodeoffset);
		ಆಗ (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			ಆದರೆ (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				addr = res.ಆರಂಭ;
				ಗಾತ್ರ_2 = res.ಕೊನೆ_2 - res.ಆರಂಭ + 1;
				assert_noisy(i < MEM_RGN_ಎಣಿಕೆ);
				ಕೂಡಿಸು_mem_region(reserved_mem, addr, ಗಾತ್ರ_2);
			}

			subnode = fdt_ಮುಂದಿನ_subnode(fdt_blob, subnode);
			++i;
		}
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ದೀರ್ಘ addrs_overlap(phys_addr_t base1, phys_ಗಾತ್ರ_t size1,
			  phys_addr_t base2, phys_ಗಾತ್ರ_t size2)
{
	ಸ್ಥಿರಾಂಕ phys_addr_t base1_ಕೊನೆ = base1 + size1 - 1;
	ಸ್ಥಿರಾಂಕ phys_addr_t base2_ಕೊನೆ = base2 + size2 - 1;

	ಹಿಂತಿರುಗಿ ((base1 <= base2_ಕೊನೆ) && (base2 <= base1_ಕೊನೆ));
}

ಸ್ಥಿರ ದೀರ್ಘ region_overlap_check(ರಚನೆ mem_region *mem_rgn, phys_addr_t ತಳ,
				 phys_ಗಾತ್ರ_t ಗಾತ್ರ_2)
{
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ i;
	ರಚನೆ region *rgn = mem_rgn->rgn;

	ಗಾಗಿ (i = 0; i < mem_rgn->ಎಣಿಕೆ_2; i++) {
		phys_addr_t rgnbase = rgn[i].ತಳ;
		phys_ಗಾತ್ರ_t rgnsize = rgn[i].ಗಾತ್ರ_2;

		ಆದರೆ (addrs_overlap(ತಳ, ಗಾತ್ರ_2, rgnbase, rgnsize))
			ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ (i < mem_rgn->ಎಣಿಕೆ_2) ? i : -1;
}

ಸ್ಥಿರ phys_addr_t ಕಂಡುಹಿಡಿ_ram_ತುದಿ(ರಚನೆ mem_region *free_mem,
				ರಚನೆ mem_region *reserved_mem, phys_ಗಾತ್ರ_t ಗಾತ್ರ_2)
{
	ದೀರ್ಘ i, rgn;
	phys_addr_t ತಳ = 0;
	phys_addr_t res_ತಳ;

	ಗಾಗಿ (i = free_mem->ಎಣಿಕೆ_2 - 1; i >= 0; i--) {
		phys_addr_t rgnbase = free_mem->rgn[i].ತಳ;
		phys_ಗಾತ್ರ_t rgnsize = free_mem->rgn[i].ಗಾತ್ರ_2;

		ಆದರೆ (rgnsize < ಗಾತ್ರ_2)
			ಮುಂದುವರಿ;

		ತಳ = rgnbase + rgnsize - ಗಾತ್ರ_2;
		ಆಗ (ತಳ && rgnbase <= ತಳ) {
			rgn = region_overlap_check(reserved_mem, ತಳ, ಗಾತ್ರ_2);
			ಆದರೆ (rgn < 0)
				ಹಿಂತಿರುಗಿ ತಳ;

			res_ತಳ = reserved_mem->rgn[rgn].ತಳ;
			ಆದರೆ (res_ತಳ < ಗಾತ್ರ_2)
				ನಿಲ್ಲಿಸು;
			ತಳ = res_ತಳ - ಗಾತ್ರ_2;
		}
	}

	ಹಿಂತಿರುಗಿ 0;
}

phys_addr_t get_mem_ತುದಿ(phys_addr_t ram_ಆರಂಭ, phys_ಗಾತ್ರ_t ram_ಗಾತ್ರ,
			phys_ಗಾತ್ರ_t ಗಾತ್ರ_2, ಶೂನ್ಯ *fdt)
{
	ಪೂರ್ಣಾಂಕ i;
	ರಚನೆ mem_region free_mem;
	ರಚನೆ mem_region reserved_mem;

	mem_regions_init(&free_mem);
	mem_regions_init(&reserved_mem);

	ಕೂಡಿಸು_mem_region(&free_mem, ram_ಆರಂಭ, ram_ಗಾತ್ರ);

	i = fdt_ಕೂಡಿಸು_reserved_regions(&free_mem, &reserved_mem, fdt);
	ಆದರೆ (i < 0)
		ಹಿಂತಿರುಗಿ 0;

	ಹಿಂತಿರುಗಿ ಕಂಡುಹಿಡಿ_ram_ತುದಿ(&free_mem, &reserved_mem, ಗಾತ್ರ_2);
}
