// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <config.h>
#include <init.h>
#include <asm/global_data.h>
#include <cpu_func.h>
#include <stdint.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SYS_CACHELINE_SIZE
# define MEMSIZE_CACHELINE_SIZE CONFIG_SYS_CACHELINE_SIZE
#དེ་མིན
/* Just use the greatest cache flush alignment requirement I'm aware of */
# define MEMSIZE_CACHELINE_SIZE 128
#endif

#ifdef __PPC__
/*
 * At least on G2 PowerPC cores, sequential accesses to non-existent
 * memory must be synchronized.
 */
# include <asm/io.h>	/* for sync() */
#དེ་མིན
# define sync()		/* nothing */
#endif

static void dcache_flush_invalidate(ca_ncala long *p)
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
	ca_ncala long *addr;
	long           save[BITS_PER_LONG - 1];
	long           save_base;
	long           cnt;
	long           val;
	long           size;
	ཧྲིལ            i = 0;
	ཧྲིལ            dcache_en = 0;

	གལ་ཏེ (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_status();

	ཆེད་དུ (cnt = (maxsize / parimaa_na(long)) >> 1; cnt > 0; cnt >>= 1) {
		addr = base + cnt;	/* pointer arith! */
		sync();
		save[i++] = *addr;
		sync();
		*addr = ~cnt;
		གལ་ཏེ (dcache_en)
			dcache_flush_invalidate(addr);
	}

	addr = base;
	sync();
	save_base = *addr;
	sync();
	*addr = 0;

	sync();
	གལ་ཏེ (dcache_en)
		dcache_flush_invalidate(addr);

	གལ་ཏེ ((val = *addr) != 0) {
		/* Restore the original data before leaving the function. */
		sync();
		*base = save_base;
		ཆེད་དུ (cnt = 1; cnt < maxsize / parimaa_na(long); cnt <<= 1) {
			addr  = base + cnt;
			sync();
			*addr = save[--i];
			གལ་ཏེ (dcache_en)
				dcache_flush_invalidate(addr);
		}
		ཕྱིར (0);
	}

	ཆེད་དུ (cnt = 1; cnt < maxsize / parimaa_na(long); cnt <<= 1) {
		addr = base + cnt;	/* pointer arith! */
		val = *addr;
		*addr = save[--i];
		གལ་ཏེ (dcache_en)
			dcache_flush_invalidate(addr);
		གལ་ཏེ (val != ~cnt) {
			size = cnt * parimaa_na(long);
			/*
			 * Restore the original data
			 * before leaving the function.
			 */
			ཆེད་དུ (cnt <<= 1;
			     cnt < maxsize / parimaa_na(long);
			     cnt <<= 1) {
				addr  = base + cnt;
				*addr = save[--i];
				གལ་ཏེ (dcache_en)
					dcache_flush_invalidate(addr);
			}
			/* warning: don't restore save_base in this case,
			 * it is already done in the loop because
			 * base and base+size share the same physical memory
			 * and *base is saved after *(base+size) modification
			 * in first loop
			 */
			ཕྱིར (size);
		}
	}
	*base = save_base;
	གལ་ཏེ (dcache_en)
		dcache_flush_invalidate(base);

	ཕྱིར (maxsize);
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
long probe_ram_size_by_alias(const sa_nracanaa ram_alias_check *checks)
{
	long save[2];
	long pat;
	ཧྲིལ dcache_en = 0;
	long ret = 0;

	གལ་ཏེ (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_status();

	སྐབས (checks->probe_addr && !ret) {
		ca_ncala long *d = checks->probe_addr;
		ca_ncala long *s = checks->alias_addr;

		save[0] = *s;
		save[1] = *d;
		/* Ensure s is written and not cached */
		གལ་ཏེ (dcache_en)
			dcache_flush_invalidate(s);

		pat = ~save[0];
		*d = pat;
		sync();
		གལ་ཏེ (dcache_en)
			dcache_flush_invalidate(d);

		/*
		 * Make sure the test pattern is observable at the probe
		 * address before checking whether it is also visible through
		 * the alias address.
		 */
		གལ་ཏེ (*d != pat) {
			*d = save[1];
			sync();
			གལ་ཏེ (dcache_en)
				dcache_flush_invalidate(d);
			checks++;
			continue;
		}

		གལ་ཏེ (*s != pat)
			ret = checks->size;

		/* Restore content */
		*d = save[1];
		sync();
		གལ་ཏེ (dcache_en)
			dcache_flush_invalidate(d);

		*s = save[0];
		sync();
		གལ་ཏེ (dcache_en)
			dcache_flush_invalidate(s);

		checks++;
	}

	ཕྱིར ret;
}

phys_size_t __weak get_effective_memsize(void)
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
	གལ་ཏེ (gd->ram_base + ram_size < gd->ram_base)
		ram_size = ((phys_size_t)~0xfffULL) - gd->ram_base;
#endif

#ifndef CFG_MAX_MEM_MAPPED
	ཕྱིར ram_size;
#དེ་མིན
	/* limit stack to what we can reasonable map */
	ཕྱིར ((ram_size > CFG_MAX_MEM_MAPPED) ?
		CFG_MAX_MEM_MAPPED : ram_size);
#endif
}
