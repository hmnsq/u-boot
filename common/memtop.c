// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Limited
 */

#شامل <fdt_support.h>
#شامل <fdtdec.h>
#شامل <memtop.h>

#شامل <asm/types.h>

#تعریف_کرو MEM_RGN_گنتی	64

ساخت region {
	phys_addr_t بنیاد;
	phys_سائز_t سائز;
};

ساخت mem_region {
	ساخت region rgn[MEM_RGN_گنتی];
	uint گنتی;
};

ساکن خالی جوڑو_mem_region(ساخت mem_region *mem_rgn, phys_addr_t بنیاد,
			   phys_سائز_t سائز)
{
	طویل i;

	برائے (i = mem_rgn->گنتی; i >= 0; i--) {
		اگر (i && بنیاد < mem_rgn->rgn[i - 1].بنیاد) {
			mem_rgn->rgn[i] = mem_rgn->rgn[i - 1];
		} ورنہ {
			mem_rgn->rgn[i].بنیاد = بنیاد;
			mem_rgn->rgn[i].سائز = سائز;
			توڑو;
		}
	}

	mem_rgn->گنتی++;
}

ساکن خالی mem_regions_init(ساخت mem_region *mem)
{
	uint i;

	mem->گنتی = 0;
	برائے (i = 0; i < MEM_RGN_گنتی; i++) {
		mem->rgn[i].بنیاد = 0;
		mem->rgn[i].سائز = 0;
	}
}

ساکن صحیح fdt_جوڑو_reserved_regions(ساخت mem_region *free_mem,
				    ساخت mem_region *reserved_mem,
				    خالی *fdt_blob)
{
	u64 addr, سائز;
	صحیح i, کل, ret;
	صحیح nodeoffset, subnode;
	ساخت fdt_resource res;

	اگر (fdt_check_سرخی(fdt_blob) != 0)
		واپس -1;

	/* process memreserve sections */
	کل = fdt_عدد_mem_rsv(fdt_blob);
	assert_noisy(کل < MEM_RGN_گنتی);
	برائے (i = 0; i < کل; i++) {
		اگر (fdt_get_mem_rsv(fdt_blob, i, &addr, &سائز) != 0)
			جاری;
		جوڑو_mem_region(reserved_mem, addr, سائز);
	}

	i = 0;
	/* process reserved-memory */
	nodeoffset = fdt_subnode_انحراف(fdt_blob, 0, "reserved-memory");
	اگر (nodeoffset >= 0) {
		subnode = fdt_پہلا_subnode(fdt_blob, nodeoffset);
		جب (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			اگر (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				addr = res.آغاز;
				سائز = res.انتہا_2 - res.آغاز + 1;
				assert_noisy(i < MEM_RGN_گنتی);
				جوڑو_mem_region(reserved_mem, addr, سائز);
			}

			subnode = fdt_اگلا_subnode(fdt_blob, subnode);
			++i;
		}
	}

	واپس 0;
}

ساکن طویل addrs_overlap(phys_addr_t base1, phys_سائز_t size1,
			  phys_addr_t base2, phys_سائز_t size2)
{
	ثابت phys_addr_t base1_انتہا = base1 + size1 - 1;
	ثابت phys_addr_t base2_انتہا = base2 + size2 - 1;

	واپس ((base1 <= base2_انتہا) && (base2 <= base1_انتہا));
}

ساکن طویل region_overlap_check(ساخت mem_region *mem_rgn, phys_addr_t بنیاد,
				 phys_سائز_t سائز)
{
	غیرعلامتی طویل i;
	ساخت region *rgn = mem_rgn->rgn;

	برائے (i = 0; i < mem_rgn->گنتی; i++) {
		phys_addr_t rgnbase = rgn[i].بنیاد;
		phys_سائز_t rgnsize = rgn[i].سائز;

		اگر (addrs_overlap(بنیاد, سائز, rgnbase, rgnsize))
			توڑو;
	}

	واپس (i < mem_rgn->گنتی) ? i : -1;
}

ساکن phys_addr_t ڈھونڈو_ram_چوٹی(ساخت mem_region *free_mem,
				ساخت mem_region *reserved_mem, phys_سائز_t سائز)
{
	طویل i, rgn;
	phys_addr_t بنیاد = 0;
	phys_addr_t res_بنیاد;

	برائے (i = free_mem->گنتی - 1; i >= 0; i--) {
		phys_addr_t rgnbase = free_mem->rgn[i].بنیاد;
		phys_سائز_t rgnsize = free_mem->rgn[i].سائز;

		اگر (rgnsize < سائز)
			جاری;

		بنیاد = rgnbase + rgnsize - سائز;
		جب (بنیاد && rgnbase <= بنیاد) {
			rgn = region_overlap_check(reserved_mem, بنیاد, سائز);
			اگر (rgn < 0)
				واپس بنیاد;

			res_بنیاد = reserved_mem->rgn[rgn].بنیاد;
			اگر (res_بنیاد < سائز)
				توڑو;
			بنیاد = res_بنیاد - سائز;
		}
	}

	واپس 0;
}

phys_addr_t get_mem_چوٹی(phys_addr_t ram_آغاز, phys_سائز_t ram_سائز,
			phys_سائز_t سائز, خالی *fdt)
{
	صحیح i;
	ساخت mem_region free_mem;
	ساخت mem_region reserved_mem;

	mem_regions_init(&free_mem);
	mem_regions_init(&reserved_mem);

	جوڑو_mem_region(&free_mem, ram_آغاز, ram_سائز);

	i = fdt_جوڑو_reserved_regions(&free_mem, &reserved_mem, fdt);
	اگر (i < 0)
		واپس 0;

	واپس ڈھونڈو_ram_چوٹی(&free_mem, &reserved_mem, سائز);
}
