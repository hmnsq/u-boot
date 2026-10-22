// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#समावेश <config.h>
#समावेश <init.h>
#समावेश <asm/global_data.h>
#समावेश <cpu_func.h>
#समावेश <stdint.h>

DECLARE_GLOBAL_तथ्य_PTR;

#अगर_घोषित CONFIG_SYS_CACHELINE_आकार
# define MEMSIZE_CACHELINE_आकार CONFIG_SYS_CACHELINE_आकार
#अन्यथा
/* Just use the greatest cache flush alignment requirement I'm aware of */
# define MEMSIZE_CACHELINE_आकार 128
#पूर्ण_अगर

#अगर_घोषित __PPC__
/*
 * At least on G2 PowerPC cores, sequential accesses to non-existent
 * memory must be synchronized.
 */
# include <asm/io.h>	/* for sync() */
#अन्यथा
# define sync()		/* nothing */
#पूर्ण_अगर

अटल व्योम dcache_flush_invalidate(अस्थिर दीर्घ *p)
{
	uintptr_t start, stop;
	start = ALIGN_DOWN((uintptr_t)p, MEMSIZE_CACHELINE_आकार);
	stop = start + MEMSIZE_CACHELINE_आकार;
	flush_dcache_range(start, stop);
	invalidate_dcache_range(start, stop);
}

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'.
 */
दीर्घ get_ram_आकार(दीर्घ *base, दीर्घ maxsize)
{
	अस्थिर दीर्घ *addr;
	दीर्घ           save[BITS_PER_LONG - 1];
	दीर्घ           save_base;
	दीर्घ           cnt;
	दीर्घ           val;
	दीर्घ           आकार;
	पूर्णांक            i = 0;
	पूर्णांक            dcache_en = 0;

	अगर (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_status();

	क्रम (cnt = (maxsize / माप(दीर्घ)) >> 1; cnt > 0; cnt >>= 1) {
		addr = base + cnt;	/* pointer arith! */
		sync();
		save[i++] = *addr;
		sync();
		*addr = ~cnt;
		अगर (dcache_en)
			dcache_flush_invalidate(addr);
	}

	addr = base;
	sync();
	save_base = *addr;
	sync();
	*addr = 0;

	sync();
	अगर (dcache_en)
		dcache_flush_invalidate(addr);

	अगर ((val = *addr) != 0) {
		/* Restore the original data before leaving the function. */
		sync();
		*base = save_base;
		क्रम (cnt = 1; cnt < maxsize / माप(दीर्घ); cnt <<= 1) {
			addr  = base + cnt;
			sync();
			*addr = save[--i];
			अगर (dcache_en)
				dcache_flush_invalidate(addr);
		}
		वापस (0);
	}

	क्रम (cnt = 1; cnt < maxsize / माप(दीर्घ); cnt <<= 1) {
		addr = base + cnt;	/* pointer arith! */
		val = *addr;
		*addr = save[--i];
		अगर (dcache_en)
			dcache_flush_invalidate(addr);
		अगर (val != ~cnt) {
			आकार = cnt * माप(दीर्घ);
			/*
			 * Restore the original data
			 * before leaving the function.
			 */
			क्रम (cnt <<= 1;
			     cnt < maxsize / माप(दीर्घ);
			     cnt <<= 1) {
				addr  = base + cnt;
				*addr = save[--i];
				अगर (dcache_en)
					dcache_flush_invalidate(addr);
			}
			/* warning: don't restore save_base in this case,
			 * it is already done in the loop because
			 * base and base+size share the same physical memory
			 * and *base is saved after *(base+size) modification
			 * in first loop
			 */
			वापस (आकार);
		}
	}
	*base = save_base;
	अगर (dcache_en)
		dcache_flush_invalidate(base);

	वापस (maxsize);
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
दीर्घ probe_ram_आकार_by_alias(स्थिर काष्ठा ram_alias_check *checks)
{
	दीर्घ save[2];
	दीर्घ pat;
	पूर्णांक dcache_en = 0;
	दीर्घ ret = 0;

	अगर (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_status();

	जबतक (checks->probe_addr && !ret) {
		अस्थिर दीर्घ *d = checks->probe_addr;
		अस्थिर दीर्घ *s = checks->alias_addr;

		save[0] = *s;
		save[1] = *d;
		/* Ensure s is written and not cached */
		अगर (dcache_en)
			dcache_flush_invalidate(s);

		pat = ~save[0];
		*d = pat;
		sync();
		अगर (dcache_en)
			dcache_flush_invalidate(d);

		/*
		 * Make sure the test pattern is observable at the probe
		 * address before checking whether it is also visible through
		 * the alias address.
		 */
		अगर (*d != pat) {
			*d = save[1];
			sync();
			अगर (dcache_en)
				dcache_flush_invalidate(d);
			checks++;
			जारी;
		}

		अगर (*s != pat)
			ret = checks->आकार;

		/* Restore content */
		*d = save[1];
		sync();
		अगर (dcache_en)
			dcache_flush_invalidate(d);

		*s = save[0];
		sync();
		अगर (dcache_en)
			dcache_flush_invalidate(s);

		checks++;
	}

	वापस ret;
}

phys_आकार_t __weak get_effective_memsize(व्योम)
{
	phys_आकार_t ram_आकार = gd->ram_आकार;

#अगर_घोषित CONFIG_MPC85xx
	/*
	 * Check for overflow and limit ram size to some representable value.
	 * It is required that ram_base + ram_size must be representable by
	 * phys_size_t type and must be aligned by direct access, therefore
	 * calculate it from last 4kB sector which should work as alignment
	 * on any platform.
	 */
	अगर (gd->ram_base + ram_आकार < gd->ram_base)
		ram_आकार = ((phys_आकार_t)~0xfffULL) - gd->ram_base;
#पूर्ण_अगर

#अगर_अघोषित CFG_MAX_MEM_MAPPED
	वापस ram_आकार;
#अन्यथा
	/* limit stack to what we can reasonable map */
	वापस ((ram_आकार > CFG_MAX_MEM_MAPPED) ?
		CFG_MAX_MEM_MAPPED : ram_आकार);
#पूर्ण_अगर
}
