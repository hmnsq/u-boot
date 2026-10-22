// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Limited
 */

#ضمن <fdt_support.h>
#ضمن <fdtdec.h>
#ضمن <memtop.h>

#ضمن <asm/types.h>

#وصف_ڪريو MEM_RGN_ڳڻپ	64

بنية region {
	phys_addr_t بنياد;
	phys_سائيز_t سائيز;
};

بنية mem_region {
	بنية region rgn[MEM_RGN_ڳڻپ];
	uint ڳڻپ;
};

ساكن فراغ جوڙيو_mem_region(بنية mem_region *mem_rgn, phys_addr_t بنياد,
			   phys_سائيز_t سائيز)
{
	ڊگهو i;

	لكل (i = mem_rgn->ڳڻپ; i >= 0; i--) {
		إذا (i && بنياد < mem_rgn->rgn[i - 1].بنياد) {
			mem_rgn->rgn[i] = mem_rgn->rgn[i - 1];
		} وإلا {
			mem_rgn->rgn[i].بنياد = بنياد;
			mem_rgn->rgn[i].سائيز = سائيز;
			اكسر;
		}
	}

	mem_rgn->ڳڻپ++;
}

ساكن فراغ mem_regions_init(بنية mem_region *mem)
{
	uint i;

	mem->ڳڻپ = 0;
	لكل (i = 0; i < MEM_RGN_ڳڻپ; i++) {
		mem->rgn[i].بنياد = 0;
		mem->rgn[i].سائيز = 0;
	}
}

ساكن صحيح fdt_جوڙيو_reserved_regions(بنية mem_region *free_mem,
				    بنية mem_region *reserved_mem,
				    فراغ *fdt_blob)
{
	u64 addr, سائيز;
	صحيح i, ڪل, ret;
	صحيح nodeoffset, subnode;
	بنية fdt_resource res;

	إذا (fdt_check_سرو(fdt_blob) != 0)
		أرجع -1;

	/* process memreserve sections */
	ڪل = fdt_انگ_mem_rsv(fdt_blob);
	assert_noisy(ڪل < MEM_RGN_ڳڻپ);
	لكل (i = 0; i < ڪل; i++) {
		إذا (fdt_get_mem_rsv(fdt_blob, i, &addr, &سائيز) != 0)
			استمر;
		جوڙيو_mem_region(reserved_mem, addr, سائيز);
	}

	i = 0;
	/* process reserved-memory */
	nodeoffset = fdt_subnode_وٿي(fdt_blob, 0, "reserved-memory");
	إذا (nodeoffset >= 0) {
		subnode = fdt_پهريون_subnode(fdt_blob, nodeoffset);
		طالما (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			إذا (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				addr = res.شروعات;
				سائيز = res.پڇاڙي_2 - res.شروعات + 1;
				assert_noisy(i < MEM_RGN_ڳڻپ);
				جوڙيو_mem_region(reserved_mem, addr, سائيز);
			}

			subnode = fdt_ايندڙ_subnode(fdt_blob, subnode);
			++i;
		}
	}

	أرجع 0;
}

ساكن ڊگهو addrs_overlap(phys_addr_t base1, phys_سائيز_t size1,
			  phys_addr_t base2, phys_سائيز_t size2)
{
	ثابت phys_addr_t base1_پڇاڙي = base1 + size1 - 1;
	ثابت phys_addr_t base2_پڇاڙي = base2 + size2 - 1;

	أرجع ((base1 <= base2_پڇاڙي) && (base2 <= base1_پڇاڙي));
}

ساكن ڊگهو region_overlap_check(بنية mem_region *mem_rgn, phys_addr_t بنياد,
				 phys_سائيز_t سائيز)
{
	بنانشان ڊگهو i;
	بنية region *rgn = mem_rgn->rgn;

	لكل (i = 0; i < mem_rgn->ڳڻپ; i++) {
		phys_addr_t rgnbase = rgn[i].بنياد;
		phys_سائيز_t rgnsize = rgn[i].سائيز;

		إذا (addrs_overlap(بنياد, سائيز, rgnbase, rgnsize))
			اكسر;
	}

	أرجع (i < mem_rgn->ڳڻپ) ? i : -1;
}

ساكن phys_addr_t ڳوليو_ram_چوٽي(بنية mem_region *free_mem,
				بنية mem_region *reserved_mem, phys_سائيز_t سائيز)
{
	ڊگهو i, rgn;
	phys_addr_t بنياد = 0;
	phys_addr_t res_بنياد;

	لكل (i = free_mem->ڳڻپ - 1; i >= 0; i--) {
		phys_addr_t rgnbase = free_mem->rgn[i].بنياد;
		phys_سائيز_t rgnsize = free_mem->rgn[i].سائيز;

		إذا (rgnsize < سائيز)
			استمر;

		بنياد = rgnbase + rgnsize - سائيز;
		طالما (بنياد && rgnbase <= بنياد) {
			rgn = region_overlap_check(reserved_mem, بنياد, سائيز);
			إذا (rgn < 0)
				أرجع بنياد;

			res_بنياد = reserved_mem->rgn[rgn].بنياد;
			إذا (res_بنياد < سائيز)
				اكسر;
			بنياد = res_بنياد - سائيز;
		}
	}

	أرجع 0;
}

phys_addr_t get_mem_چوٽي(phys_addr_t ram_شروعات, phys_سائيز_t ram_سائيز,
			phys_سائيز_t سائيز, فراغ *fdt)
{
	صحيح i;
	بنية mem_region free_mem;
	بنية mem_region reserved_mem;

	mem_regions_init(&free_mem);
	mem_regions_init(&reserved_mem);

	جوڙيو_mem_region(&free_mem, ram_شروعات, ram_سائيز);

	i = fdt_جوڙيو_reserved_regions(&free_mem, &reserved_mem, fdt);
	إذا (i < 0)
		أرجع 0;

	أرجع ڳوليو_ram_چوٽي(&free_mem, &reserved_mem, سائيز);
}
