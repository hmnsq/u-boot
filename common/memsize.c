// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#অন্তর্ভাব <config.h>
#অন্তর্ভাব <init.h>
#অন্তর্ভাব <asm/global_data.h>
#অন্তর্ভাব <cpu_func.h>
#অন্তর্ভাব <stdint.h>

DECLARE_GLOBAL_তথ্য_PTR;

#ifdef CONFIG_SYS_CACHELINE_আকার
# define MEMSIZE_CACHELINE_আকার CONFIG_SYS_CACHELINE_আকার
#অন্যথা
/* Just use the greatest cache flush alignment requirement I'm aware of */
# define MEMSIZE_CACHELINE_আকার 128
#endif

#ifdef __PPC__
/*
 * At least on G2 PowerPC cores, sequential accesses to non-existent
 * memory must be synchronized.
 */
# include <asm/io.h>	/* for sync() */
#অন্যথা
# define sync()		/* nothing */
#endif

স্থির শূন্য dcache_flush_invalidate(অস্থির long *p)
{
	uintptr_t start, stop;
	start = ALIGN_DOWN((uintptr_t)p, MEMSIZE_CACHELINE_আকার);
	stop = start + MEMSIZE_CACHELINE_আকার;
	flush_dcache_range(start, stop);
	invalidate_dcache_range(start, stop);
}

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'.
 */
long get_ram_আকার(long *base, long maxsize)
{
	অস্থির long *addr;
	long           save[BITS_PER_LONG - 1];
	long           save_base;
	long           cnt;
	long           val;
	long           আকার_2;
	পূর্ণসংখ্যা            i = 0;
	পূর্ণসংখ্যা            dcache_en = 0;

	যদি (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_status();

	জন্য (cnt = (maxsize / আকার(long)) >> 1; cnt > 0; cnt >>= 1) {
		addr = base + cnt;	/* pointer arith! */
		sync();
		save[i++] = *addr;
		sync();
		*addr = ~cnt;
		যদি (dcache_en)
			dcache_flush_invalidate(addr);
	}

	addr = base;
	sync();
	save_base = *addr;
	sync();
	*addr = 0;

	sync();
	যদি (dcache_en)
		dcache_flush_invalidate(addr);

	যদি ((val = *addr) != 0) {
		/* Restore the original data before leaving the function. */
		sync();
		*base = save_base;
		জন্য (cnt = 1; cnt < maxsize / আকার(long); cnt <<= 1) {
			addr  = base + cnt;
			sync();
			*addr = save[--i];
			যদি (dcache_en)
				dcache_flush_invalidate(addr);
		}
		ফেরত (0);
	}

	জন্য (cnt = 1; cnt < maxsize / আকার(long); cnt <<= 1) {
		addr = base + cnt;	/* pointer arith! */
		val = *addr;
		*addr = save[--i];
		যদি (dcache_en)
			dcache_flush_invalidate(addr);
		যদি (val != ~cnt) {
			আকার_2 = cnt * আকার(long);
			/*
			 * Restore the original data
			 * before leaving the function.
			 */
			জন্য (cnt <<= 1;
			     cnt < maxsize / আকার(long);
			     cnt <<= 1) {
				addr  = base + cnt;
				*addr = save[--i];
				যদি (dcache_en)
					dcache_flush_invalidate(addr);
			}
			/* warning: don't restore save_base in this case,
			 * it is already done in the loop because
			 * base and base+size share the same physical memory
			 * and *base is saved after *(base+size) modification
			 * in first loop
			 */
			ফেরত (আকার_2);
		}
	}
	*base = save_base;
	যদি (dcache_en)
		dcache_flush_invalidate(base);

	ফেরত (maxsize);
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
long probe_ram_আকার_by_alias(ধ্রুবক কাঠামো ram_alias_check *checks)
{
	long save[2];
	long pat;
	পূর্ণসংখ্যা dcache_en = 0;
	long ret = 0;

	যদি (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_status();

	যখন (checks->probe_addr && !ret) {
		অস্থির long *d = checks->probe_addr;
		অস্থির long *s = checks->alias_addr;

		save[0] = *s;
		save[1] = *d;
		/* Ensure s is written and not cached */
		যদি (dcache_en)
			dcache_flush_invalidate(s);

		pat = ~save[0];
		*d = pat;
		sync();
		যদি (dcache_en)
			dcache_flush_invalidate(d);

		/*
		 * Make sure the test pattern is observable at the probe
		 * address before checking whether it is also visible through
		 * the alias address.
		 */
		যদি (*d != pat) {
			*d = save[1];
			sync();
			যদি (dcache_en)
				dcache_flush_invalidate(d);
			checks++;
			চালু;
		}

		যদি (*s != pat)
			ret = checks->আকার_2;

		/* Restore content */
		*d = save[1];
		sync();
		যদি (dcache_en)
			dcache_flush_invalidate(d);

		*s = save[0];
		sync();
		যদি (dcache_en)
			dcache_flush_invalidate(s);

		checks++;
	}

	ফেরত ret;
}

phys_আকার_t __weak get_effective_memsize(শূন্য)
{
	phys_আকার_t ram_আকার = gd->ram_আকার;

#ifdef CONFIG_MPC85xx
	/*
	 * Check for overflow and limit ram size to some representable value.
	 * It is required that ram_base + ram_size must be representable by
	 * phys_size_t type and must be aligned by direct access, therefore
	 * calculate it from last 4kB sector which should work as alignment
	 * on any platform.
	 */
	যদি (gd->ram_base + ram_আকার < gd->ram_base)
		ram_আকার = ((phys_আকার_t)~0xfffULL) - gd->ram_base;
#endif

#ifndef CFG_MAX_MEM_MAPPED
	ফেরত ram_আকার;
#অন্যথা
	/* limit stack to what we can reasonable map */
	ফেরত ((ram_আকার > CFG_MAX_MEM_MAPPED) ?
		CFG_MAX_MEM_MAPPED : ram_আকার);
#endif
}
