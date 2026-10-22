// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Limited
 */

#ضمن <fdt_support.h>
#ضمن <fdtdec.h>
#ضمن <memtop.h>

#ضمن <asm/types.h>

#تعریف_کن MEM_RGN_شمار	64

بنية region {
	phys_addr_t پایه;
	phys_اندازه_t اندازه;
};

بنية mem_region {
	بنية region rgn[MEM_RGN_شمار];
	uint شمار;
};

ساكن فراغ جمع_کن_mem_region(بنية mem_region *mem_rgn, phys_addr_t پایه,
			   phys_اندازه_t اندازه)
{
	دراز i;

	لكل (i = mem_rgn->شمار; i >= 0; i--) {
		إذا (i && پایه < mem_rgn->rgn[i - 1].پایه) {
			mem_rgn->rgn[i] = mem_rgn->rgn[i - 1];
		} وإلا {
			mem_rgn->rgn[i].پایه = پایه;
			mem_rgn->rgn[i].اندازه = اندازه;
			اكسر;
		}
	}

	mem_rgn->شمار++;
}

ساكن فراغ mem_regions_init(بنية mem_region *mem)
{
	uint i;

	mem->شمار = 0;
	لكل (i = 0; i < MEM_RGN_شمار; i++) {
		mem->rgn[i].پایه = 0;
		mem->rgn[i].اندازه = 0;
	}
}

ساكن صحيح fdt_جمع_کن_reserved_regions(بنية mem_region *free_mem,
				    بنية mem_region *reserved_mem,
				    فراغ *fdt_blob)
{
	u64 addr, اندازه;
	صحيح i, مجموع, ret;
	صحيح nodeoffset, subnode;
	بنية fdt_resource res;

	إذا (fdt_check_سرآیند(fdt_blob) != 0)
		أرجع -1;

	/* process memreserve sections */
	مجموع = fdt_رقم_mem_rsv(fdt_blob);
	assert_noisy(مجموع < MEM_RGN_شمار);
	لكل (i = 0; i < مجموع; i++) {
		إذا (fdt_get_mem_rsv(fdt_blob, i, &addr, &اندازه) != 0)
			استمر;
		جمع_کن_mem_region(reserved_mem, addr, اندازه);
	}

	i = 0;
	/* process reserved-memory */
	nodeoffset = fdt_subnode_جابه_جایی(fdt_blob, 0, "reserved-memory");
	إذا (nodeoffset >= 0) {
		subnode = fdt_اولی_subnode(fdt_blob, nodeoffset);
		طالما (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			إذا (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				addr = res.شروع;
				اندازه = res.انتها_2 - res.شروع + 1;
				assert_noisy(i < MEM_RGN_شمار);
				جمع_کن_mem_region(reserved_mem, addr, اندازه);
			}

			subnode = fdt_بعدی_subnode(fdt_blob, subnode);
			++i;
		}
	}

	أرجع 0;
}

ساكن دراز addrs_overlap(phys_addr_t base1, phys_اندازه_t size1,
			  phys_addr_t base2, phys_اندازه_t size2)
{
	ثابت phys_addr_t base1_انتها = base1 + size1 - 1;
	ثابت phys_addr_t base2_انتها = base2 + size2 - 1;

	أرجع ((base1 <= base2_انتها) && (base2 <= base1_انتها));
}

ساكن دراز region_overlap_check(بنية mem_region *mem_rgn, phys_addr_t پایه,
				 phys_اندازه_t اندازه)
{
	بیعلامت دراز i;
	بنية region *rgn = mem_rgn->rgn;

	لكل (i = 0; i < mem_rgn->شمار; i++) {
		phys_addr_t rgnbase = rgn[i].پایه;
		phys_اندازه_t rgnsize = rgn[i].اندازه;

		إذا (addrs_overlap(پایه, اندازه, rgnbase, rgnsize))
			اكسر;
	}

	أرجع (i < mem_rgn->شمار) ? i : -1;
}

ساكن phys_addr_t پیدا_کن_ram_بالاترین(بنية mem_region *free_mem,
				بنية mem_region *reserved_mem, phys_اندازه_t اندازه)
{
	دراز i, rgn;
	phys_addr_t پایه = 0;
	phys_addr_t res_پایه;

	لكل (i = free_mem->شمار - 1; i >= 0; i--) {
		phys_addr_t rgnbase = free_mem->rgn[i].پایه;
		phys_اندازه_t rgnsize = free_mem->rgn[i].اندازه;

		إذا (rgnsize < اندازه)
			استمر;

		پایه = rgnbase + rgnsize - اندازه;
		طالما (پایه && rgnbase <= پایه) {
			rgn = region_overlap_check(reserved_mem, پایه, اندازه);
			إذا (rgn < 0)
				أرجع پایه;

			res_پایه = reserved_mem->rgn[rgn].پایه;
			إذا (res_پایه < اندازه)
				اكسر;
			پایه = res_پایه - اندازه;
		}
	}

	أرجع 0;
}

phys_addr_t get_mem_بالاترین(phys_addr_t ram_شروع, phys_اندازه_t ram_اندازه,
			phys_اندازه_t اندازه, فراغ *fdt)
{
	صحيح i;
	بنية mem_region free_mem;
	بنية mem_region reserved_mem;

	mem_regions_init(&free_mem);
	mem_regions_init(&reserved_mem);

	جمع_کن_mem_region(&free_mem, ram_شروع, ram_اندازه);

	i = fdt_جمع_کن_reserved_regions(&free_mem, &reserved_mem, fdt);
	إذا (i < 0)
		أرجع 0;

	أرجع پیدا_کن_ram_بالاترین(&free_mem, &reserved_mem, اندازه);
}
