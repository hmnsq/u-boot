// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Limited
 */

#ضمن <fdt_support.h>
#ضمن <fdtdec.h>
#ضمن <memtop.h>

#ضمن <asm/types.h>

#تعریف_کرو MEM_RGN_گنتی	64

بنية region {
	phys_addr_t ادھار;
	phys_ناپ_t ناپ;
};

بنية mem_region {
	بنية region rgn[MEM_RGN_گنتی];
	uint گنتی;
};

ساكن فراغ جوڑو_mem_region(بنية mem_region *mem_rgn, phys_addr_t ادھار,
			   phys_ناپ_t ناپ)
{
	لما i;

	لكل (i = mem_rgn->گنتی; i >= 0; i--) {
		إذا (i && ادھار < mem_rgn->rgn[i - 1].ادھار) {
			mem_rgn->rgn[i] = mem_rgn->rgn[i - 1];
		} وإلا {
			mem_rgn->rgn[i].ادھار = ادھار;
			mem_rgn->rgn[i].ناپ = ناپ;
			اكسر;
		}
	}

	mem_rgn->گنتی++;
}

ساكن فراغ mem_regions_init(بنية mem_region *mem)
{
	uint i;

	mem->گنتی = 0;
	لكل (i = 0; i < MEM_RGN_گنتی; i++) {
		mem->rgn[i].ادھار = 0;
		mem->rgn[i].ناپ = 0;
	}
}

ساكن صحيح fdt_جوڑو_reserved_regions(بنية mem_region *free_mem,
				    بنية mem_region *reserved_mem,
				    فراغ *fdt_blob)
{
	u64 addr, ناپ;
	صحيح i, کل, ret;
	صحيح nodeoffset, subnode;
	بنية fdt_resource res;

	إذا (fdt_check_سرخی(fdt_blob) != 0)
		أرجع -1;

	/* process memreserve sections */
	کل = fdt_عدد_mem_rsv(fdt_blob);
	assert_noisy(کل < MEM_RGN_گنتی);
	لكل (i = 0; i < کل; i++) {
		إذا (fdt_get_mem_rsv(fdt_blob, i, &addr, &ناپ) != 0)
			استمر;
		جوڑو_mem_region(reserved_mem, addr, ناپ);
	}

	i = 0;
	/* process reserved-memory */
	nodeoffset = fdt_subnode_وتھ(fdt_blob, 0, "reserved-memory");
	إذا (nodeoffset >= 0) {
		subnode = fdt_پہلا_subnode(fdt_blob, nodeoffset);
		طالما (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			إذا (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				addr = res.شروع;
				ناپ = res.سرا_2 - res.شروع + 1;
				assert_noisy(i < MEM_RGN_گنتی);
				جوڑو_mem_region(reserved_mem, addr, ناپ);
			}

			subnode = fdt_اگلا_subnode(fdt_blob, subnode);
			++i;
		}
	}

	أرجع 0;
}

ساكن لما addrs_overlap(phys_addr_t base1, phys_ناپ_t size1,
			  phys_addr_t base2, phys_ناپ_t size2)
{
	ثابت phys_addr_t base1_سرا = base1 + size1 - 1;
	ثابت phys_addr_t base2_سرا = base2 + size2 - 1;

	أرجع ((base1 <= base2_سرا) && (base2 <= base1_سرا));
}

ساكن لما region_overlap_check(بنية mem_region *mem_rgn, phys_addr_t ادھار,
				 phys_ناپ_t ناپ)
{
	بنانشانی لما i;
	بنية region *rgn = mem_rgn->rgn;

	لكل (i = 0; i < mem_rgn->گنتی; i++) {
		phys_addr_t rgnbase = rgn[i].ادھار;
		phys_ناپ_t rgnsize = rgn[i].ناپ;

		إذا (addrs_overlap(ادھار, ناپ, rgnbase, rgnsize))
			اكسر;
	}

	أرجع (i < mem_rgn->گنتی) ? i : -1;
}

ساكن phys_addr_t لبھو_ram_چوٹی(بنية mem_region *free_mem,
				بنية mem_region *reserved_mem, phys_ناپ_t ناپ)
{
	لما i, rgn;
	phys_addr_t ادھار = 0;
	phys_addr_t res_ادھار;

	لكل (i = free_mem->گنتی - 1; i >= 0; i--) {
		phys_addr_t rgnbase = free_mem->rgn[i].ادھار;
		phys_ناپ_t rgnsize = free_mem->rgn[i].ناپ;

		إذا (rgnsize < ناپ)
			استمر;

		ادھار = rgnbase + rgnsize - ناپ;
		طالما (ادھار && rgnbase <= ادھار) {
			rgn = region_overlap_check(reserved_mem, ادھار, ناپ);
			إذا (rgn < 0)
				أرجع ادھار;

			res_ادھار = reserved_mem->rgn[rgn].ادھار;
			إذا (res_ادھار < ناپ)
				اكسر;
			ادھار = res_ادھار - ناپ;
		}
	}

	أرجع 0;
}

phys_addr_t get_mem_چوٹی(phys_addr_t ram_شروع, phys_ناپ_t ram_ناپ,
			phys_ناپ_t ناپ, فراغ *fdt)
{
	صحيح i;
	بنية mem_region free_mem;
	بنية mem_region reserved_mem;

	mem_regions_init(&free_mem);
	mem_regions_init(&reserved_mem);

	جوڑو_mem_region(&free_mem, ram_شروع, ram_ناپ);

	i = fdt_جوڑو_reserved_regions(&free_mem, &reserved_mem, fdt);
	إذا (i < 0)
		أرجع 0;

	أرجع لبھو_ram_چوٹی(&free_mem, &reserved_mem, ناپ);
}
