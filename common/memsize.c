// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <config.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <init.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <cpu_func.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_CACHELINE_ਮਾਪ
# define MEMSIZE_CACHELINE_ਮਾਪ CONFIG_SYS_CACHELINE_ਮਾਪ
#ਨਹੀਂਤਾਂ
/* Just use the greatest cache flush alignment requirement I'm aware of */
# define MEMSIZE_CACHELINE_ਮਾਪ 128
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __PPC__
/*
 * At least on G2 PowerPC cores, sequential accesses to non-existent
 * memory must be synchronized.
 */
# include <asm/io.h>	/* for sync() */
#ਨਹੀਂਤਾਂ
# define sync()		/* nothing */
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਖਾਲੀ dcache_flush_invalidate(ਅਸਥਿਰ ਲੰਮਾ *p)
{
	uintptr_t ਸ਼ੁਰੂ, stop;
	ਸ਼ੁਰੂ = ALIGN_ਥੱਲੇ((uintptr_t)p, MEMSIZE_CACHELINE_ਮਾਪ);
	stop = ਸ਼ੁਰੂ + MEMSIZE_CACHELINE_ਮਾਪ;
	flush_dcache_range(ਸ਼ੁਰੂ, stop);
	invalidate_dcache_range(ਸ਼ੁਰੂ, stop);
}

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'.
 */
ਲੰਮਾ get_ram_ਮਾਪ(ਲੰਮਾ *ਅਧਾਰ, ਲੰਮਾ maxsize)
{
	ਅਸਥਿਰ ਲੰਮਾ *addr;
	ਲੰਮਾ           save[ਬਿੱਟ_ਪ੍ਰਤੀ_LONG - 1];
	ਲੰਮਾ           save_ਅਧਾਰ;
	ਲੰਮਾ           cnt;
	ਲੰਮਾ           val;
	ਲੰਮਾ           ਮਾਪ;
	ਪੂਰਨ            i = 0;
	ਪੂਰਨ            dcache_en = 0;

	ਜੇ (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_ਸਥਿਤੀ();

	ਲਈ (cnt = (maxsize / ਆਕਾਰ(ਲੰਮਾ)) >> 1; cnt > 0; cnt >>= 1) {
		addr = ਅਧਾਰ + cnt;	/* pointer arith! */
		sync();
		save[i++] = *addr;
		sync();
		*addr = ~cnt;
		ਜੇ (dcache_en)
			dcache_flush_invalidate(addr);
	}

	addr = ਅਧਾਰ;
	sync();
	save_ਅਧਾਰ = *addr;
	sync();
	*addr = 0;

	sync();
	ਜੇ (dcache_en)
		dcache_flush_invalidate(addr);

	ਜੇ ((val = *addr) != 0) {
		/* Restore the original data before leaving the function. */
		sync();
		*ਅਧਾਰ = save_ਅਧਾਰ;
		ਲਈ (cnt = 1; cnt < maxsize / ਆਕਾਰ(ਲੰਮਾ); cnt <<= 1) {
			addr  = ਅਧਾਰ + cnt;
			sync();
			*addr = save[--i];
			ਜੇ (dcache_en)
				dcache_flush_invalidate(addr);
		}
		ਵਾਪਸ (0);
	}

	ਲਈ (cnt = 1; cnt < maxsize / ਆਕਾਰ(ਲੰਮਾ); cnt <<= 1) {
		addr = ਅਧਾਰ + cnt;	/* pointer arith! */
		val = *addr;
		*addr = save[--i];
		ਜੇ (dcache_en)
			dcache_flush_invalidate(addr);
		ਜੇ (val != ~cnt) {
			ਮਾਪ = cnt * ਆਕਾਰ(ਲੰਮਾ);
			/*
			 * Restore the original data
			 * before leaving the function.
			 */
			ਲਈ (cnt <<= 1;
			     cnt < maxsize / ਆਕਾਰ(ਲੰਮਾ);
			     cnt <<= 1) {
				addr  = ਅਧਾਰ + cnt;
				*addr = save[--i];
				ਜੇ (dcache_en)
					dcache_flush_invalidate(addr);
			}
			/* warning: don't restore save_base in this case,
			 * it is already done in the loop because
			 * base and base+size share the same physical memory
			 * and *base is saved after *(base+size) modification
			 * in first loop
			 */
			ਵਾਪਸ (ਮਾਪ);
		}
	}
	*ਅਧਾਰ = save_ਅਧਾਰ;
	ਜੇ (dcache_en)
		dcache_flush_invalidate(ਅਧਾਰ);

	ਵਾਪਸ (maxsize);
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
ਲੰਮਾ probe_ram_ਮਾਪ_by_alias(ਅਚਲ ਬਣਤਰ ram_alias_check *checks)
{
	ਲੰਮਾ save[2];
	ਲੰਮਾ pat;
	ਪੂਰਨ dcache_en = 0;
	ਲੰਮਾ ret = 0;

	ਜੇ (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_ਸਥਿਤੀ();

	ਜਦੋਂ (checks->probe_addr && !ret) {
		ਅਸਥਿਰ ਲੰਮਾ *d = checks->probe_addr;
		ਅਸਥਿਰ ਲੰਮਾ *s = checks->alias_addr;

		save[0] = *s;
		save[1] = *d;
		/* Ensure s is written and not cached */
		ਜੇ (dcache_en)
			dcache_flush_invalidate(s);

		pat = ~save[0];
		*d = pat;
		sync();
		ਜੇ (dcache_en)
			dcache_flush_invalidate(d);

		/*
		 * Make sure the test pattern is observable at the probe
		 * address before checking whether it is also visible through
		 * the alias address.
		 */
		ਜੇ (*d != pat) {
			*d = save[1];
			sync();
			ਜੇ (dcache_en)
				dcache_flush_invalidate(d);
			checks++;
			ਜਾਰੀ;
		}

		ਜੇ (*s != pat)
			ret = checks->ਮਾਪ;

		/* Restore content */
		*d = save[1];
		sync();
		ਜੇ (dcache_en)
			dcache_flush_invalidate(d);

		*s = save[0];
		sync();
		ਜੇ (dcache_en)
			dcache_flush_invalidate(s);

		checks++;
	}

	ਵਾਪਸ ret;
}

phys_ਮਾਪ_t __weak get_effective_memsize(ਖਾਲੀ)
{
	phys_ਮਾਪ_t ram_ਮਾਪ = gd->ram_ਮਾਪ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MPC85xx
	/*
	 * Check for overflow and limit ram size to some representable value.
	 * It is required that ram_base + ram_size must be representable by
	 * phys_size_t type and must be aligned by direct access, therefore
	 * calculate it from last 4kB sector which should work as alignment
	 * on any platform.
	 */
	ਜੇ (gd->ram_ਅਧਾਰ + ram_ਮਾਪ < gd->ram_ਅਧਾਰ)
		ram_ਮਾਪ = ((phys_ਮਾਪ_t)~0xfffULL) - gd->ram_ਅਧਾਰ;
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CFG_MAX_MEM_MAPPED
	ਵਾਪਸ ram_ਮਾਪ;
#ਨਹੀਂਤਾਂ
	/* limit stack to what we can reasonable map */
	ਵਾਪਸ ((ram_ਮਾਪ > CFG_MAX_MEM_MAPPED) ?
		CFG_MAX_MEM_MAPPED : ram_ਮਾਪ);
#ਜੇ_ਅੰਤ
}
