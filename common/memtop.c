// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Limited
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <fdt_support.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fdtdec.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <memtop.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <asm/types.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEM_RGN_ਗਿਣਤੀ	64

ਬਣਤਰ region {
	phys_addr_t ਅਧਾਰ;
	phys_ਮਾਪ_t ਮਾਪ;
};

ਬਣਤਰ mem_region {
	ਬਣਤਰ region rgn[MEM_RGN_ਗਿਣਤੀ];
	uint ਗਿਣਤੀ_2;
};

ਸਥਿਰ ਖਾਲੀ ਜੋੜੋ_mem_region(ਬਣਤਰ mem_region *mem_rgn, phys_addr_t ਅਧਾਰ,
			   phys_ਮਾਪ_t ਮਾਪ)
{
	ਲੰਮਾ i;

	ਲਈ (i = mem_rgn->ਗਿਣਤੀ_2; i >= 0; i--) {
		ਜੇ (i && ਅਧਾਰ < mem_rgn->rgn[i - 1].ਅਧਾਰ) {
			mem_rgn->rgn[i] = mem_rgn->rgn[i - 1];
		} ਨਹੀਂਤਾਂ {
			mem_rgn->rgn[i].ਅਧਾਰ = ਅਧਾਰ;
			mem_rgn->rgn[i].ਮਾਪ = ਮਾਪ;
			ਰੋਕੋ;
		}
	}

	mem_rgn->ਗਿਣਤੀ_2++;
}

ਸਥਿਰ ਖਾਲੀ mem_regions_init(ਬਣਤਰ mem_region *mem)
{
	uint i;

	mem->ਗਿਣਤੀ_2 = 0;
	ਲਈ (i = 0; i < MEM_RGN_ਗਿਣਤੀ; i++) {
		mem->rgn[i].ਅਧਾਰ = 0;
		mem->rgn[i].ਮਾਪ = 0;
	}
}

ਸਥਿਰ ਪੂਰਨ fdt_ਜੋੜੋ_reserved_regions(ਬਣਤਰ mem_region *free_mem,
				    ਬਣਤਰ mem_region *reserved_mem,
				    ਖਾਲੀ *fdt_blob)
{
	u64 addr, ਮਾਪ;
	ਪੂਰਨ i, ਕੁੱਲ, ret;
	ਪੂਰਨ nodeoffset, subnode;
	ਬਣਤਰ fdt_resource res;

	ਜੇ (fdt_check_ਸਿਰਲੇਖ(fdt_blob) != 0)
		ਵਾਪਸ -1;

	/* process memreserve sections */
	ਕੁੱਲ = fdt_ਅੰਕ_mem_rsv(fdt_blob);
	assert_noisy(ਕੁੱਲ < MEM_RGN_ਗਿਣਤੀ);
	ਲਈ (i = 0; i < ਕੁੱਲ; i++) {
		ਜੇ (fdt_get_mem_rsv(fdt_blob, i, &addr, &ਮਾਪ) != 0)
			ਜਾਰੀ;
		ਜੋੜੋ_mem_region(reserved_mem, addr, ਮਾਪ);
	}

	i = 0;
	/* process reserved-memory */
	nodeoffset = fdt_subnode_ਵਿੱਥ(fdt_blob, 0, "reserved-memory");
	ਜੇ (nodeoffset >= 0) {
		subnode = fdt_ਪਹਿਲਾ_subnode(fdt_blob, nodeoffset);
		ਜਦੋਂ (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			ਜੇ (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				addr = res.ਸ਼ੁਰੂ;
				ਮਾਪ = res.ਸਿਰਾ_2 - res.ਸ਼ੁਰੂ + 1;
				assert_noisy(i < MEM_RGN_ਗਿਣਤੀ);
				ਜੋੜੋ_mem_region(reserved_mem, addr, ਮਾਪ);
			}

			subnode = fdt_ਅਗਲਾ_subnode(fdt_blob, subnode);
			++i;
		}
	}

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਲੰਮਾ addrs_overlap(phys_addr_t base1, phys_ਮਾਪ_t size1,
			  phys_addr_t base2, phys_ਮਾਪ_t size2)
{
	ਅਚਲ phys_addr_t base1_ਸਿਰਾ = base1 + size1 - 1;
	ਅਚਲ phys_addr_t base2_ਸਿਰਾ = base2 + size2 - 1;

	ਵਾਪਸ ((base1 <= base2_ਸਿਰਾ) && (base2 <= base1_ਸਿਰਾ));
}

ਸਥਿਰ ਲੰਮਾ region_overlap_check(ਬਣਤਰ mem_region *mem_rgn, phys_addr_t ਅਧਾਰ,
				 phys_ਮਾਪ_t ਮਾਪ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ i;
	ਬਣਤਰ region *rgn = mem_rgn->rgn;

	ਲਈ (i = 0; i < mem_rgn->ਗਿਣਤੀ_2; i++) {
		phys_addr_t rgnbase = rgn[i].ਅਧਾਰ;
		phys_ਮਾਪ_t rgnsize = rgn[i].ਮਾਪ;

		ਜੇ (addrs_overlap(ਅਧਾਰ, ਮਾਪ, rgnbase, rgnsize))
			ਰੋਕੋ;
	}

	ਵਾਪਸ (i < mem_rgn->ਗਿਣਤੀ_2) ? i : -1;
}

ਸਥਿਰ phys_addr_t ਲੱਭੋ_ram_ਸਿਖਰ(ਬਣਤਰ mem_region *free_mem,
				ਬਣਤਰ mem_region *reserved_mem, phys_ਮਾਪ_t ਮਾਪ)
{
	ਲੰਮਾ i, rgn;
	phys_addr_t ਅਧਾਰ = 0;
	phys_addr_t res_ਅਧਾਰ;

	ਲਈ (i = free_mem->ਗਿਣਤੀ_2 - 1; i >= 0; i--) {
		phys_addr_t rgnbase = free_mem->rgn[i].ਅਧਾਰ;
		phys_ਮਾਪ_t rgnsize = free_mem->rgn[i].ਮਾਪ;

		ਜੇ (rgnsize < ਮਾਪ)
			ਜਾਰੀ;

		ਅਧਾਰ = rgnbase + rgnsize - ਮਾਪ;
		ਜਦੋਂ (ਅਧਾਰ && rgnbase <= ਅਧਾਰ) {
			rgn = region_overlap_check(reserved_mem, ਅਧਾਰ, ਮਾਪ);
			ਜੇ (rgn < 0)
				ਵਾਪਸ ਅਧਾਰ;

			res_ਅਧਾਰ = reserved_mem->rgn[rgn].ਅਧਾਰ;
			ਜੇ (res_ਅਧਾਰ < ਮਾਪ)
				ਰੋਕੋ;
			ਅਧਾਰ = res_ਅਧਾਰ - ਮਾਪ;
		}
	}

	ਵਾਪਸ 0;
}

phys_addr_t get_mem_ਸਿਖਰ(phys_addr_t ram_ਸ਼ੁਰੂ, phys_ਮਾਪ_t ram_ਮਾਪ,
			phys_ਮਾਪ_t ਮਾਪ, ਖਾਲੀ *fdt)
{
	ਪੂਰਨ i;
	ਬਣਤਰ mem_region free_mem;
	ਬਣਤਰ mem_region reserved_mem;

	mem_regions_init(&free_mem);
	mem_regions_init(&reserved_mem);

	ਜੋੜੋ_mem_region(&free_mem, ram_ਸ਼ੁਰੂ, ram_ਮਾਪ);

	i = fdt_ਜੋੜੋ_reserved_regions(&free_mem, &reserved_mem, fdt);
	ਜੇ (i < 0)
		ਵਾਪਸ 0;

	ਵਾਪਸ ਲੱਭੋ_ram_ਸਿਖਰ(&free_mem, &reserved_mem, ਮਾਪ);
}
