// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#अन्तर्भाव <config.h>
#अन्तर्भाव <init.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <cpu_func.h>
#अन्तर्भाव <stdint.h>

DECLARE_GLOBAL_दत्त_PTR;

#यदि_परिभाषितम् CONFIG_SYS_CACHELINE_आकार
# define MEMSIZE_CACHELINE_आकार CONFIG_SYS_CACHELINE_आकार
#अन्यथा
/* Just use the greatest cache flush alignment requirement I'm aware of */
# define MEMSIZE_CACHELINE_आकार 128
#यदि_अन्तः

#यदि_परिभाषितम् __PPC__
/*
 * At least on G2 PowerPC cores, sequential accesses to non-existent
 * memory must be synchronized.
 */
# include <asm/io.h>	/* for sync() */
#अन्यथा
# define sync()		/* nothing */
#यदि_अन्तः

स्थिर शून्य dcache_flush_invalidate(चञ्चल दीर्घ *p)
{
	uintptr_t start, stop;
	start = ALIGN_अधस्तात्((uintptr_t)p, MEMSIZE_CACHELINE_आकार);
	stop = start + MEMSIZE_CACHELINE_आकार;
	flush_dcache_range(start, stop);
	invalidate_dcache_range(start, stop);
}

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'.
 */
दीर्घ get_ram_आकार(दीर्घ *आधारः, दीर्घ maxsize)
{
	चञ्चल दीर्घ *addr;
	दीर्घ           save[द्वयङ्काः_प्रत्येकम्_LONG - 1];
	दीर्घ           save_आधारः;
	दीर्घ           cnt;
	दीर्घ           val;
	दीर्घ           आकार;
	पूर्णाङ्क            i = 0;
	पूर्णाङ्क            dcache_en = 0;

	यदि (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_स्थितिः();

	प्रति (cnt = (maxsize / परिमाण(दीर्घ)) >> 1; cnt > 0; cnt >>= 1) {
		addr = आधारः + cnt;	/* pointer arith! */
		sync();
		save[i++] = *addr;
		sync();
		*addr = ~cnt;
		यदि (dcache_en)
			dcache_flush_invalidate(addr);
	}

	addr = आधारः;
	sync();
	save_आधारः = *addr;
	sync();
	*addr = 0;

	sync();
	यदि (dcache_en)
		dcache_flush_invalidate(addr);

	यदि ((val = *addr) != 0) {
		/* Restore the original data before leaving the function. */
		sync();
		*आधारः = save_आधारः;
		प्रति (cnt = 1; cnt < maxsize / परिमाण(दीर्घ); cnt <<= 1) {
			addr  = आधारः + cnt;
			sync();
			*addr = save[--i];
			यदि (dcache_en)
				dcache_flush_invalidate(addr);
		}
		प्रत्यागम (0);
	}

	प्रति (cnt = 1; cnt < maxsize / परिमाण(दीर्घ); cnt <<= 1) {
		addr = आधारः + cnt;	/* pointer arith! */
		val = *addr;
		*addr = save[--i];
		यदि (dcache_en)
			dcache_flush_invalidate(addr);
		यदि (val != ~cnt) {
			आकार = cnt * परिमाण(दीर्घ);
			/*
			 * Restore the original data
			 * before leaving the function.
			 */
			प्रति (cnt <<= 1;
			     cnt < maxsize / परिमाण(दीर्घ);
			     cnt <<= 1) {
				addr  = आधारः + cnt;
				*addr = save[--i];
				यदि (dcache_en)
					dcache_flush_invalidate(addr);
			}
			/* warning: don't restore save_base in this case,
			 * it is already done in the loop because
			 * base and base+size share the same physical memory
			 * and *base is saved after *(base+size) modification
			 * in first loop
			 */
			प्रत्यागम (आकार);
		}
	}
	*आधारः = save_आधारः;
	यदि (dcache_en)
		dcache_flush_invalidate(आधारः);

	प्रत्यागम (maxsize);
}

/**
 * probe_ram_size_by_alias() - Detect RAM size using known alias addresses
 * @checks: Array of RAM alias probe descriptors, terminated by a NULL
 *	    @probe_addr entry
 *
 * Probe RAM size by writing a test pattern to each @probe_addr and checking
 * whether the same pattern does not appear at the corresponding @alias_addr.
 * This is useful on systems where address wrap-around does not alias to the
 * base of memory in a linear way, so get_ram_size() cannot be used directly.
 * It is also useful on systems where the base of the physical memory cannot
 * be safely accessed, so get_ram_size() cannot be used at all.
 *
 * Return: The size associated with the first matching entry, or 0 if no match
 * is found.
 */
दीर्घ probe_ram_आकार_by_alias(ध्रुव संरचना ram_alias_check *checks)
{
	दीर्घ save[2];
	दीर्घ pat;
	पूर्णाङ्क dcache_en = 0;
	दीर्घ ret = 0;

	यदि (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_स्थितिः();

	यावत् (checks->probe_addr && !ret) {
		चञ्चल दीर्घ *d = checks->probe_addr;
		चञ्चल दीर्घ *s = checks->alias_addr;

		save[0] = *s;
		save[1] = *d;
		/* Ensure s is written and not cached */
		यदि (dcache_en)
			dcache_flush_invalidate(s);

		pat = ~save[0];
		*d = pat;
		sync();
		यदि (dcache_en)
			dcache_flush_invalidate(d);

		/*
		 * Make sure the test pattern is observable at the probe
		 * address before checking whether it is also visible through
		 * the alias address.
		 */
		यदि (*d != pat) {
			*d = save[1];
			sync();
			यदि (dcache_en)
				dcache_flush_invalidate(d);
			checks++;
			अनुवर्त;
		}

		यदि (*s != pat)
			ret = checks->आकार;

		/* Restore content */
		*d = save[1];
		sync();
		यदि (dcache_en)
			dcache_flush_invalidate(d);

		*s = save[0];
		sync();
		यदि (dcache_en)
			dcache_flush_invalidate(s);

		checks++;
	}

	प्रत्यागम ret;
}

phys_आकार_t __weak get_effective_memsize(शून्य)
{
	phys_आकार_t ram_आकार = gd->ram_आकार;

#यदि_परिभाषितम् CONFIG_MPC85xx
	/*
	 * Check for overflow and limit ram size to some representable value.
	 * It is required that ram_base + ram_size must be representable by
	 * phys_size_t type and must be aligned by direct access, therefore
	 * calculate it from last 4kB sector which should work as alignment
	 * on any platform.
	 */
	यदि (gd->ram_आधारः + ram_आकार < gd->ram_आधारः)
		ram_आकार = ((phys_आकार_t)~0xfffULL) - gd->ram_आधारः;
#यदि_अन्तः

#यदि_अपरिभाषितम् CFG_MAX_MEM_MAPPED
	प्रत्यागम ram_आकार;
#अन्यथा
	/* limit stack to what we can reasonable map */
	प्रत्यागम ((ram_आकार > CFG_MAX_MEM_MAPPED) ?
		CFG_MAX_MEM_MAPPED : ram_आकार);
#यदि_अन्तः
}
