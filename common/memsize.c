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

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SYS_CACHELINE_SIZE
# define MEMSIZE_CACHELINE_SIZE CONFIG_SYS_CACHELINE_SIZE
#अन्यथा
/* Just use the greatest cache flush alignment requirement I'm aware of */
# define MEMSIZE_CACHELINE_SIZE 128
#endif

#ifdef __PPC__
/*
 * At least on G2 PowerPC cores, sequential accesses to non-existent
 * memory must be synchronized.
 */
# include <asm/io.h>	/* for sync() */
#अन्यथा
# define sync()		/* nothing */
#endif

स्थिर शून्य dcache_flush_invalidate(चञ्चल दीर्घ *p)
{
	uintptr_t start, stop;
	start = ALIGN_DOWN((uintptr_t)p, MEMSIZE_CACHELINE_SIZE);
	stop = start + MEMSIZE_CACHELINE_SIZE;
	flush_dcache_range(start, stop);
	invalidate_dcache_range(start, stop);
}

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'.
 */
दीर्घ get_ram_size(दीर्घ *base, दीर्घ maxsize)
{
	चञ्चल दीर्घ *addr;
	दीर्घ           save[BITS_PER_LONG - 1];
	दीर्घ           save_base;
	दीर्घ           cnt;
	दीर्घ           val;
	दीर्घ           size;
	पूर्णाङ्क            i = 0;
	पूर्णाङ्क            dcache_en = 0;

	यदि (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_status();

	लागि (cnt = (maxsize / परिमाण(दीर्घ)) >> 1; cnt > 0; cnt >>= 1) {
		addr = base + cnt;	/* pointer arith! */
		sync();
		save[i++] = *addr;
		sync();
		*addr = ~cnt;
		यदि (dcache_en)
			dcache_flush_invalidate(addr);
	}

	addr = base;
	sync();
	save_base = *addr;
	sync();
	*addr = 0;

	sync();
	यदि (dcache_en)
		dcache_flush_invalidate(addr);

	यदि ((val = *addr) != 0) {
		/* Restore the original data before leaving the function. */
		sync();
		*base = save_base;
		लागि (cnt = 1; cnt < maxsize / परिमाण(दीर्घ); cnt <<= 1) {
			addr  = base + cnt;
			sync();
			*addr = save[--i];
			यदि (dcache_en)
				dcache_flush_invalidate(addr);
		}
		फर्काऊ (0);
	}

	लागि (cnt = 1; cnt < maxsize / परिमाण(दीर्घ); cnt <<= 1) {
		addr = base + cnt;	/* pointer arith! */
		val = *addr;
		*addr = save[--i];
		यदि (dcache_en)
			dcache_flush_invalidate(addr);
		यदि (val != ~cnt) {
			size = cnt * परिमाण(दीर्घ);
			/*
			 * Restore the original data
			 * before leaving the function.
			 */
			लागि (cnt <<= 1;
			     cnt < maxsize / परिमाण(दीर्घ);
			     cnt <<= 1) {
				addr  = base + cnt;
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
			फर्काऊ (size);
		}
	}
	*base = save_base;
	यदि (dcache_en)
		dcache_flush_invalidate(base);

	फर्काऊ (maxsize);
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
दीर्घ probe_ram_size_by_alias(अचर संरचना ram_alias_check *checks)
{
	दीर्घ save[2];
	दीर्घ pat;
	पूर्णाङ्क dcache_en = 0;
	दीर्घ ret = 0;

	यदि (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_status();

	जबसम्म (checks->probe_addr && !ret) {
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
			जारी;
		}

		यदि (*s != pat)
			ret = checks->size;

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

	फर्काऊ ret;
}

phys_size_t __weak get_effective_memsize(शून्य)
{
	phys_size_t ram_size = gd->ram_size;

#ifdef CONFIG_MPC85xx
	/*
	 * Check for overflow and limit ram size to some representable value.
	 * It is required that ram_base + ram_size must be representable by
	 * phys_size_t type and must be aligned by direct access, therefore
	 * calculate it from last 4kB sector which should work as alignment
	 * on any platform.
	 */
	यदि (gd->ram_base + ram_size < gd->ram_base)
		ram_size = ((phys_size_t)~0xfffULL) - gd->ram_base;
#endif

#ifndef CFG_MAX_MEM_MAPPED
	फर्काऊ ram_size;
#अन्यथा
	/* limit stack to what we can reasonable map */
	फर्काऊ ((ram_size > CFG_MAX_MEM_MAPPED) ?
		CFG_MAX_MEM_MAPPED : ram_size);
#endif
}
