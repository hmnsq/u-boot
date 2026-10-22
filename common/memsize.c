// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#అన్తర్భావ <config.h>
#అన్తర్భావ <init.h>
#అన్తర్భావ <asm/global_data.h>
#అన్తర్భావ <cpu_func.h>
#అన్తర్భావ <stdint.h>

DECLARE_GLOBAL_దత్తాంశం_PTR;

#నిర్వచితమైతే CONFIG_SYS_CACHELINE_కొలత
# define MEMSIZE_CACHELINE_కొలత CONFIG_SYS_CACHELINE_కొలత
#లేకపోతే
/* Just use the greatest cache flush alignment requirement I'm aware of */
# define MEMSIZE_CACHELINE_కొలత 128
#అయితే_ముగింపు

#నిర్వచితమైతే __PPC__
/*
 * At least on G2 PowerPC cores, sequential accesses to non-existent
 * memory must be synchronized.
 */
# include <asm/io.h>	/* for sync() */
#లేకపోతే
# define sync()		/* nothing */
#అయితే_ముగింపు

స్థిర శూన్య dcache_flush_invalidate(అస్థిర దీర్ఘ *p)
{
	uintptr_t ప్రారంభం, stop;
	ప్రారంభం = ALIGN_కిందికి((uintptr_t)p, MEMSIZE_CACHELINE_కొలత);
	stop = ప్రారంభం + MEMSIZE_CACHELINE_కొలత;
	flush_dcache_range(ప్రారంభం, stop);
	invalidate_dcache_range(ప్రారంభం, stop);
}

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'.
 */
దీర్ఘ get_ram_కొలత(దీర్ఘ *పునాది, దీర్ఘ maxsize)
{
	అస్థిర దీర్ఘ *addr;
	దీర్ఘ           save[బిట్లు_చొప్పున_LONG - 1];
	దీర్ఘ           save_పునాది;
	దీర్ఘ           cnt;
	దీర్ఘ           val;
	దీర్ఘ           కొలత;
	పూర్ణాంకం            i = 0;
	పూర్ణాంకం            dcache_en = 0;

	ఒకవేళ (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_స్థితి();

	కొరకు (cnt = (maxsize / పరిమాణం(దీర్ఘ)) >> 1; cnt > 0; cnt >>= 1) {
		addr = పునాది + cnt;	/* pointer arith! */
		sync();
		save[i++] = *addr;
		sync();
		*addr = ~cnt;
		ఒకవేళ (dcache_en)
			dcache_flush_invalidate(addr);
	}

	addr = పునాది;
	sync();
	save_పునాది = *addr;
	sync();
	*addr = 0;

	sync();
	ఒకవేళ (dcache_en)
		dcache_flush_invalidate(addr);

	ఒకవేళ ((val = *addr) != 0) {
		/* Restore the original data before leaving the function. */
		sync();
		*పునాది = save_పునాది;
		కొరకు (cnt = 1; cnt < maxsize / పరిమాణం(దీర్ఘ); cnt <<= 1) {
			addr  = పునాది + cnt;
			sync();
			*addr = save[--i];
			ఒకవేళ (dcache_en)
				dcache_flush_invalidate(addr);
		}
		తిరిగి (0);
	}

	కొరకు (cnt = 1; cnt < maxsize / పరిమాణం(దీర్ఘ); cnt <<= 1) {
		addr = పునాది + cnt;	/* pointer arith! */
		val = *addr;
		*addr = save[--i];
		ఒకవేళ (dcache_en)
			dcache_flush_invalidate(addr);
		ఒకవేళ (val != ~cnt) {
			కొలత = cnt * పరిమాణం(దీర్ఘ);
			/*
			 * Restore the original data
			 * before leaving the function.
			 */
			కొరకు (cnt <<= 1;
			     cnt < maxsize / పరిమాణం(దీర్ఘ);
			     cnt <<= 1) {
				addr  = పునాది + cnt;
				*addr = save[--i];
				ఒకవేళ (dcache_en)
					dcache_flush_invalidate(addr);
			}
			/* warning: don't restore save_base in this case,
			 * it is already done in the loop because
			 * base and base+size share the same physical memory
			 * and *base is saved after *(base+size) modification
			 * in first loop
			 */
			తిరిగి (కొలత);
		}
	}
	*పునాది = save_పునాది;
	ఒకవేళ (dcache_en)
		dcache_flush_invalidate(పునాది);

	తిరిగి (maxsize);
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
దీర్ఘ probe_ram_కొలత_by_alias(స్థిరరాశి నిర్మాణం ram_alias_check *checks)
{
	దీర్ఘ save[2];
	దీర్ఘ pat;
	పూర్ణాంకం dcache_en = 0;
	దీర్ఘ ret = 0;

	ఒకవేళ (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_స్థితి();

	అయితే (checks->probe_addr && !ret) {
		అస్థిర దీర్ఘ *d = checks->probe_addr;
		అస్థిర దీర్ఘ *s = checks->alias_addr;

		save[0] = *s;
		save[1] = *d;
		/* Ensure s is written and not cached */
		ఒకవేళ (dcache_en)
			dcache_flush_invalidate(s);

		pat = ~save[0];
		*d = pat;
		sync();
		ఒకవేళ (dcache_en)
			dcache_flush_invalidate(d);

		/*
		 * Make sure the test pattern is observable at the probe
		 * address before checking whether it is also visible through
		 * the alias address.
		 */
		ఒకవేళ (*d != pat) {
			*d = save[1];
			sync();
			ఒకవేళ (dcache_en)
				dcache_flush_invalidate(d);
			checks++;
			కొనసాగు;
		}

		ఒకవేళ (*s != pat)
			ret = checks->కొలత;

		/* Restore content */
		*d = save[1];
		sync();
		ఒకవేళ (dcache_en)
			dcache_flush_invalidate(d);

		*s = save[0];
		sync();
		ఒకవేళ (dcache_en)
			dcache_flush_invalidate(s);

		checks++;
	}

	తిరిగి ret;
}

phys_కొలత_t __weak get_effective_memsize(శూన్య)
{
	phys_కొలత_t ram_కొలత = gd->ram_కొలత;

#నిర్వచితమైతే CONFIG_MPC85xx
	/*
	 * Check for overflow and limit ram size to some representable value.
	 * It is required that ram_base + ram_size must be representable by
	 * phys_size_t type and must be aligned by direct access, therefore
	 * calculate it from last 4kB sector which should work as alignment
	 * on any platform.
	 */
	ఒకవేళ (gd->ram_పునాది + ram_కొలత < gd->ram_పునాది)
		ram_కొలత = ((phys_కొలత_t)~0xfffULL) - gd->ram_పునాది;
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే CFG_MAX_MEM_MAPPED
	తిరిగి ram_కొలత;
#లేకపోతే
	/* limit stack to what we can reasonable map */
	తిరిగి ((ram_కొలత > CFG_MAX_MEM_MAPPED) ?
		CFG_MAX_MEM_MAPPED : ram_కొలత);
#అయితే_ముగింపు
}
