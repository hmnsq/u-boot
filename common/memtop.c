// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Limited
 */

#شامل <fdt_support.h>
#شامل <fdtdec.h>
#شامل <memtop.h>

#شامل <asm/types.h>

#define MEM_RGN_شمار	64

ساختار region {
	phys_addr_t base;
	phys_size_t size;
};

ساختار mem_region {
	ساختار region rgn[MEM_RGN_شمار];
	uint شمار;
};

ایستا تهی add_mem_region(ساختار mem_region *mem_rgn, phys_addr_t base,
			   phys_size_t size)
{
	long i;

	برای (i = mem_rgn->شمار; i >= 0; i--) {
		اگر (i && base < mem_rgn->rgn[i - 1].base) {
			mem_rgn->rgn[i] = mem_rgn->rgn[i - 1];
		} وگرنه {
			mem_rgn->rgn[i].base = base;
			mem_rgn->rgn[i].size = size;
			بشکن;
		}
	}

	mem_rgn->شمار++;
}

ایستا تهی mem_regions_init(ساختار mem_region *mem)
{
	uint i;

	mem->شمار = 0;
	برای (i = 0; i < MEM_RGN_شمار; i++) {
		mem->rgn[i].base = 0;
		mem->rgn[i].size = 0;
	}
}

ایستا صحیح fdt_add_reserved_regions(ساختار mem_region *free_mem,
				    ساختار mem_region *reserved_mem,
				    تهی *fdt_blob)
{
	u64 addr, size;
	صحیح i, total, ret;
	صحیح nodeoffset, subnode;
	ساختار fdt_resource res;

	اگر (fdt_check_header(fdt_blob) != 0)
		بازگشت -1;

	/* process memreserve sections */
	total = fdt_رقم_mem_rsv(fdt_blob);
	assert_noisy(total < MEM_RGN_شمار);
	برای (i = 0; i < total; i++) {
		اگر (fdt_get_mem_rsv(fdt_blob, i, &addr, &size) != 0)
			ادامه;
		add_mem_region(reserved_mem, addr, size);
	}

	i = 0;
	/* process reserved-memory */
	nodeoffset = fdt_subnode_offset(fdt_blob, 0, "reserved-memory");
	اگر (nodeoffset >= 0) {
		subnode = fdt_first_subnode(fdt_blob, nodeoffset);
		وقتی (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			اگر (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				addr = res.start;
				size = res.end - res.start + 1;
				assert_noisy(i < MEM_RGN_شمار);
				add_mem_region(reserved_mem, addr, size);
			}

			subnode = fdt_next_subnode(fdt_blob, subnode);
			++i;
		}
	}

	بازگشت 0;
}

ایستا long addrs_overlap(phys_addr_t base1, phys_size_t size1,
			  phys_addr_t base2, phys_size_t size2)
{
	ثابت phys_addr_t base1_end = base1 + size1 - 1;
	ثابت phys_addr_t base2_end = base2 + size2 - 1;

	بازگشت ((base1 <= base2_end) && (base2 <= base1_end));
}

ایستا long region_overlap_check(ساختار mem_region *mem_rgn, phys_addr_t base,
				 phys_size_t size)
{
	unsigned long i;
	ساختار region *rgn = mem_rgn->rgn;

	برای (i = 0; i < mem_rgn->شمار; i++) {
		phys_addr_t rgnbase = rgn[i].base;
		phys_size_t rgnsize = rgn[i].size;

		اگر (addrs_overlap(base, size, rgnbase, rgnsize))
			بشکن;
	}

	بازگشت (i < mem_rgn->شمار) ? i : -1;
}

ایستا phys_addr_t find_ram_بالاترین(ساختار mem_region *free_mem,
				ساختار mem_region *reserved_mem, phys_size_t size)
{
	long i, rgn;
	phys_addr_t base = 0;
	phys_addr_t res_base;

	برای (i = free_mem->شمار - 1; i >= 0; i--) {
		phys_addr_t rgnbase = free_mem->rgn[i].base;
		phys_size_t rgnsize = free_mem->rgn[i].size;

		اگر (rgnsize < size)
			ادامه;

		base = rgnbase + rgnsize - size;
		وقتی (base && rgnbase <= base) {
			rgn = region_overlap_check(reserved_mem, base, size);
			اگر (rgn < 0)
				بازگشت base;

			res_base = reserved_mem->rgn[rgn].base;
			اگر (res_base < size)
				بشکن;
			base = res_base - size;
		}
	}

	بازگشت 0;
}

phys_addr_t get_mem_بالاترین(phys_addr_t ram_start, phys_size_t ram_size,
			phys_size_t size, تهی *fdt)
{
	صحیح i;
	ساختار mem_region free_mem;
	ساختار mem_region reserved_mem;

	mem_regions_init(&free_mem);
	mem_regions_init(&reserved_mem);

	add_mem_region(&free_mem, ram_start, ram_size);

	i = fdt_add_reserved_regions(&free_mem, &reserved_mem, fdt);
	اگر (i < 0)
		بازگشت 0;

	بازگشت find_ram_بالاترین(&free_mem, &reserved_mem, size);
}
