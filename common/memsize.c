// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#અન્તર્ભાવ <config.h>
#અન્તર્ભાવ <init.h>
#અન્તર્ભાવ <asm/global_data.h>
#અન્તર્ભાવ <cpu_func.h>
#અન્તર્ભાવ <stdint.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SYS_CACHELINE_SIZE
# define MEMSIZE_CACHELINE_SIZE CONFIG_SYS_CACHELINE_SIZE
#નહીંતર
/* Just use the greatest cache flush alignment requirement I'm aware of */
# define MEMSIZE_CACHELINE_SIZE 128
#endif

#ifdef __PPC__
/*
 * At least on G2 PowerPC cores, sequential accesses to non-existent
 * memory must be synchronized.
 */
# include <asm/io.h>	/* for sync() */
#નહીંતર
# define sync()		/* nothing */
#endif

સ્થિર શૂન્ય dcache_flush_invalidate(અસ્થિર long *p)
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
	અસ્થિર long *addr;
	long           save[BITS_PER_LONG - 1];
	long           save_base;
	long           cnt;
	long           val;
	long           size;
	પૂર્ણાંક            i = 0;
	પૂર્ણાંક            dcache_en = 0;

	જો (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_status();

	માટે (cnt = (maxsize / કદ(long)) >> 1; cnt > 0; cnt >>= 1) {
		addr = base + cnt;	/* pointer arith! */
		sync();
		save[i++] = *addr;
		sync();
		*addr = ~cnt;
		જો (dcache_en)
			dcache_flush_invalidate(addr);
	}

	addr = base;
	sync();
	save_base = *addr;
	sync();
	*addr = 0;

	sync();
	જો (dcache_en)
		dcache_flush_invalidate(addr);

	જો ((val = *addr) != 0) {
		/* Restore the original data before leaving the function. */
		sync();
		*base = save_base;
		માટે (cnt = 1; cnt < maxsize / કદ(long); cnt <<= 1) {
			addr  = base + cnt;
			sync();
			*addr = save[--i];
			જો (dcache_en)
				dcache_flush_invalidate(addr);
		}
		પાછા (0);
	}

	માટે (cnt = 1; cnt < maxsize / કદ(long); cnt <<= 1) {
		addr = base + cnt;	/* pointer arith! */
		val = *addr;
		*addr = save[--i];
		જો (dcache_en)
			dcache_flush_invalidate(addr);
		જો (val != ~cnt) {
			size = cnt * કદ(long);
			/*
			 * Restore the original data
			 * before leaving the function.
			 */
			માટે (cnt <<= 1;
			     cnt < maxsize / કદ(long);
			     cnt <<= 1) {
				addr  = base + cnt;
				*addr = save[--i];
				જો (dcache_en)
					dcache_flush_invalidate(addr);
			}
			/* warning: don't restore save_base in this case,
			 * it is already done in the loop because
			 * base and base+size share the same physical memory
			 * and *base is saved after *(base+size) modification
			 * in first loop
			 */
			પાછા (size);
		}
	}
	*base = save_base;
	જો (dcache_en)
		dcache_flush_invalidate(base);

	પાછા (maxsize);
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
long probe_ram_size_by_alias(અચલ સંરચના ram_alias_check *checks)
{
	long save[2];
	long pat;
	પૂર્ણાંક dcache_en = 0;
	long ret = 0;

	જો (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_status();

	જ્યારે (checks->probe_addr && !ret) {
		અસ્થિર long *d = checks->probe_addr;
		અસ્થિર long *s = checks->alias_addr;

		save[0] = *s;
		save[1] = *d;
		/* Ensure s is written and not cached */
		જો (dcache_en)
			dcache_flush_invalidate(s);

		pat = ~save[0];
		*d = pat;
		sync();
		જો (dcache_en)
			dcache_flush_invalidate(d);

		/*
		 * Make sure the test pattern is observable at the probe
		 * address before checking whether it is also visible through
		 * the alias address.
		 */
		જો (*d != pat) {
			*d = save[1];
			sync();
			જો (dcache_en)
				dcache_flush_invalidate(d);
			checks++;
			ચાલુ;
		}

		જો (*s != pat)
			ret = checks->size;

		/* Restore content */
		*d = save[1];
		sync();
		જો (dcache_en)
			dcache_flush_invalidate(d);

		*s = save[0];
		sync();
		જો (dcache_en)
			dcache_flush_invalidate(s);

		checks++;
	}

	પાછા ret;
}

phys_size_t __weak get_effective_memsize(શૂન્ય)
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
	જો (gd->ram_base + ram_size < gd->ram_base)
		ram_size = ((phys_size_t)~0xfffULL) - gd->ram_base;
#endif

#ifndef CFG_MAX_MEM_MAPPED
	પાછા ram_size;
#નહીંતર
	/* limit stack to what we can reasonable map */
	પાછા ((ram_size > CFG_MAX_MEM_MAPPED) ?
		CFG_MAX_MEM_MAPPED : ram_size);
#endif
}
