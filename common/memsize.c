// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ଅନ୍ତର୍ଭାଵ <config.h>
#ଅନ୍ତର୍ଭାଵ <init.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>
#ଅନ୍ତର୍ଭାଵ <cpu_func.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SYS_CACHELINE_SIZE
# define MEMSIZE_CACHELINE_SIZE CONFIG_SYS_CACHELINE_SIZE
#ଅନ୍ୟଥା
/* Just use the greatest cache flush alignment requirement I'm aware of */
# define MEMSIZE_CACHELINE_SIZE 128
#endif

#ifdef __PPC__
/*
 * At least on G2 PowerPC cores, sequential accesses to non-existent
 * memory must be synchronized.
 */
# include <asm/io.h>	/* for sync() */
#ଅନ୍ୟଥା
# define sync()		/* nothing */
#endif

ସ୍ଥିର ଶୂନ୍ୟ dcache_flush_invalidate(ଅସ୍ଥିର long *p)
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
long get_ram_size(long *base, long maxsize)
{
	ଅସ୍ଥିର long *addr;
	long           save[BITS_PER_LONG - 1];
	long           save_base;
	long           cnt;
	long           val;
	long           size;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା            i = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା            dcache_en = 0;

	ଯଦି (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_status();

	ପାଇଁ (cnt = (maxsize / ଆକାର(long)) >> 1; cnt > 0; cnt >>= 1) {
		addr = base + cnt;	/* pointer arith! */
		sync();
		save[i++] = *addr;
		sync();
		*addr = ~cnt;
		ଯଦି (dcache_en)
			dcache_flush_invalidate(addr);
	}

	addr = base;
	sync();
	save_base = *addr;
	sync();
	*addr = 0;

	sync();
	ଯଦି (dcache_en)
		dcache_flush_invalidate(addr);

	ଯଦି ((val = *addr) != 0) {
		/* Restore the original data before leaving the function. */
		sync();
		*base = save_base;
		ପାଇଁ (cnt = 1; cnt < maxsize / ଆକାର(long); cnt <<= 1) {
			addr  = base + cnt;
			sync();
			*addr = save[--i];
			ଯଦି (dcache_en)
				dcache_flush_invalidate(addr);
		}
		ଫେରନ୍ତୁ (0);
	}

	ପାଇଁ (cnt = 1; cnt < maxsize / ଆକାର(long); cnt <<= 1) {
		addr = base + cnt;	/* pointer arith! */
		val = *addr;
		*addr = save[--i];
		ଯଦି (dcache_en)
			dcache_flush_invalidate(addr);
		ଯଦି (val != ~cnt) {
			size = cnt * ଆକାର(long);
			/*
			 * Restore the original data
			 * before leaving the function.
			 */
			ପାଇଁ (cnt <<= 1;
			     cnt < maxsize / ଆକାର(long);
			     cnt <<= 1) {
				addr  = base + cnt;
				*addr = save[--i];
				ଯଦି (dcache_en)
					dcache_flush_invalidate(addr);
			}
			/* warning: don't restore save_base in this case,
			 * it is already done in the loop because
			 * base and base+size share the same physical memory
			 * and *base is saved after *(base+size) modification
			 * in first loop
			 */
			ଫେରନ୍ତୁ (size);
		}
	}
	*base = save_base;
	ଯଦି (dcache_en)
		dcache_flush_invalidate(base);

	ଫେରନ୍ତୁ (maxsize);
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
long probe_ram_size_by_alias(ଅଚଳ ସଂରଚନା ram_alias_check *checks)
{
	long save[2];
	long pat;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା dcache_en = 0;
	long ret = 0;

	ଯଦି (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_status();

	ଯେତେବେଳେ (checks->probe_addr && !ret) {
		ଅସ୍ଥିର long *d = checks->probe_addr;
		ଅସ୍ଥିର long *s = checks->alias_addr;

		save[0] = *s;
		save[1] = *d;
		/* Ensure s is written and not cached */
		ଯଦି (dcache_en)
			dcache_flush_invalidate(s);

		pat = ~save[0];
		*d = pat;
		sync();
		ଯଦି (dcache_en)
			dcache_flush_invalidate(d);

		/*
		 * Make sure the test pattern is observable at the probe
		 * address before checking whether it is also visible through
		 * the alias address.
		 */
		ଯଦି (*d != pat) {
			*d = save[1];
			sync();
			ଯଦି (dcache_en)
				dcache_flush_invalidate(d);
			checks++;
			ଜାରି;
		}

		ଯଦି (*s != pat)
			ret = checks->size;

		/* Restore content */
		*d = save[1];
		sync();
		ଯଦି (dcache_en)
			dcache_flush_invalidate(d);

		*s = save[0];
		sync();
		ଯଦି (dcache_en)
			dcache_flush_invalidate(s);

		checks++;
	}

	ଫେରନ୍ତୁ ret;
}

phys_size_t __weak get_effective_memsize(ଶୂନ୍ୟ)
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
	ଯଦି (gd->ram_base + ram_size < gd->ram_base)
		ram_size = ((phys_size_t)~0xfffULL) - gd->ram_base;
#endif

#ifndef CFG_MAX_MEM_MAPPED
	ଫେରନ୍ତୁ ram_size;
#ଅନ୍ୟଥା
	/* limit stack to what we can reasonable map */
	ଫେରନ୍ତୁ ((ram_size > CFG_MAX_MEM_MAPPED) ?
		CFG_MAX_MEM_MAPPED : ram_size);
#endif
}
