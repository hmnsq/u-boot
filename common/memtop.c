// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Limited
 */

#அந்தர்பாவ <fdt_support.h>
#அந்தர்பாவ <fdtdec.h>
#அந்தர்பாவ <memtop.h>

#அந்தர்பாவ <asm/types.h>

#define MEM_RGN_மொத்தம்	64

கட்டமைப்பு region {
	phys_addr_t base;
	phys_size_t size;
};

கட்டமைப்பு mem_region {
	கட்டமைப்பு region rgn[MEM_RGN_மொத்தம்];
	uint மொத்தம்;
};

நிலையான வெறுமை add_mem_region(கட்டமைப்பு mem_region *mem_rgn, phys_addr_t base,
			   phys_size_t size)
{
	long i;

	க்கு (i = mem_rgn->மொத்தம்; i >= 0; i--) {
		எனில் (i && base < mem_rgn->rgn[i - 1].base) {
			mem_rgn->rgn[i] = mem_rgn->rgn[i - 1];
		} இல்லையெனில் {
			mem_rgn->rgn[i].base = base;
			mem_rgn->rgn[i].size = size;
			நிறுத்து;
		}
	}

	mem_rgn->மொத்தம்++;
}

நிலையான வெறுமை mem_regions_init(கட்டமைப்பு mem_region *mem)
{
	uint i;

	mem->மொத்தம் = 0;
	க்கு (i = 0; i < MEM_RGN_மொத்தம்; i++) {
		mem->rgn[i].base = 0;
		mem->rgn[i].size = 0;
	}
}

நிலையான முழு fdt_add_reserved_regions(கட்டமைப்பு mem_region *free_mem,
				    கட்டமைப்பு mem_region *reserved_mem,
				    வெறுமை *fdt_blob)
{
	u64 addr, size;
	முழு i, total, ret;
	முழு nodeoffset, subnode;
	கட்டமைப்பு fdt_resource res;

	எனில் (fdt_check_header(fdt_blob) != 0)
		திரும்பு -1;

	/* process memreserve sections */
	total = fdt_இலக்கம்_mem_rsv(fdt_blob);
	assert_noisy(total < MEM_RGN_மொத்தம்);
	க்கு (i = 0; i < total; i++) {
		எனில் (fdt_get_mem_rsv(fdt_blob, i, &addr, &size) != 0)
			தொடர்;
		add_mem_region(reserved_mem, addr, size);
	}

	i = 0;
	/* process reserved-memory */
	nodeoffset = fdt_subnode_offset(fdt_blob, 0, "reserved-memory");
	எனில் (nodeoffset >= 0) {
		subnode = fdt_first_subnode(fdt_blob, nodeoffset);
		போது (subnode >= 0) {
			/* check if this subnode has a reg property */
			ret = fdt_get_resource(fdt_blob, subnode, "reg", 0,
					       &res);
			எனில் (!ret && fdtdec_get_is_enabled(fdt_blob, subnode)) {
				addr = res.start;
				size = res.end - res.start + 1;
				assert_noisy(i < MEM_RGN_மொத்தம்);
				add_mem_region(reserved_mem, addr, size);
			}

			subnode = fdt_next_subnode(fdt_blob, subnode);
			++i;
		}
	}

	திரும்பு 0;
}

நிலையான long addrs_overlap(phys_addr_t base1, phys_size_t size1,
			  phys_addr_t base2, phys_size_t size2)
{
	மாறிலி phys_addr_t base1_end = base1 + size1 - 1;
	மாறிலி phys_addr_t base2_end = base2 + size2 - 1;

	திரும்பு ((base1 <= base2_end) && (base2 <= base1_end));
}

நிலையான long region_overlap_check(கட்டமைப்பு mem_region *mem_rgn, phys_addr_t base,
				 phys_size_t size)
{
	unsigned long i;
	கட்டமைப்பு region *rgn = mem_rgn->rgn;

	க்கு (i = 0; i < mem_rgn->மொத்தம்; i++) {
		phys_addr_t rgnbase = rgn[i].base;
		phys_size_t rgnsize = rgn[i].size;

		எனில் (addrs_overlap(base, size, rgnbase, rgnsize))
			நிறுத்து;
	}

	திரும்பு (i < mem_rgn->மொத்தம்) ? i : -1;
}

நிலையான phys_addr_t find_ram_உச்சி(கட்டமைப்பு mem_region *free_mem,
				கட்டமைப்பு mem_region *reserved_mem, phys_size_t size)
{
	long i, rgn;
	phys_addr_t base = 0;
	phys_addr_t res_base;

	க்கு (i = free_mem->மொத்தம் - 1; i >= 0; i--) {
		phys_addr_t rgnbase = free_mem->rgn[i].base;
		phys_size_t rgnsize = free_mem->rgn[i].size;

		எனில் (rgnsize < size)
			தொடர்;

		base = rgnbase + rgnsize - size;
		போது (base && rgnbase <= base) {
			rgn = region_overlap_check(reserved_mem, base, size);
			எனில் (rgn < 0)
				திரும்பு base;

			res_base = reserved_mem->rgn[rgn].base;
			எனில் (res_base < size)
				நிறுத்து;
			base = res_base - size;
		}
	}

	திரும்பு 0;
}

phys_addr_t get_mem_உச்சி(phys_addr_t ram_start, phys_size_t ram_size,
			phys_size_t size, வெறுமை *fdt)
{
	முழு i;
	கட்டமைப்பு mem_region free_mem;
	கட்டமைப்பு mem_region reserved_mem;

	mem_regions_init(&free_mem);
	mem_regions_init(&reserved_mem);

	add_mem_region(&free_mem, ram_start, ram_size);

	i = fdt_add_reserved_regions(&free_mem, &reserved_mem, fdt);
	எனில் (i < 0)
		திரும்பு 0;

	திரும்பு find_ram_உச்சி(&free_mem, &reserved_mem, size);
}
