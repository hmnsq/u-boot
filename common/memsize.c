// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ಅನ್ತರ್ಭಾವ <config.h>
#ಅನ್ತರ್ಭಾವ <init.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <cpu_func.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_CACHELINE_ಗಾತ್ರ
# define MEMSIZE_CACHELINE_ಗಾತ್ರ CONFIG_SYS_CACHELINE_ಗಾತ್ರ
#ಇಲ್ಲದಿದ್ದರೆ
/* Just use the greatest cache flush alignment requirement I'm aware of */
# define MEMSIZE_CACHELINE_ಗಾತ್ರ 128
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __PPC__
/*
 * At least on G2 PowerPC cores, sequential accesses to non-existent
 * memory must be synchronized.
 */
# include <asm/io.h>	/* for sync() */
#ಇಲ್ಲದಿದ್ದರೆ
# define sync()		/* nothing */
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಶೂನ್ಯ dcache_flush_invalidate(ಅಸ್ಥಿರ ದೀರ್ಘ *p)
{
	uintptr_t ಆರಂಭ, stop;
	ಆರಂಭ = ALIGN_ಕೆಳಕ್ಕೆ((uintptr_t)p, MEMSIZE_CACHELINE_ಗಾತ್ರ);
	stop = ಆರಂಭ + MEMSIZE_CACHELINE_ಗಾತ್ರ;
	flush_dcache_range(ಆರಂಭ, stop);
	invalidate_dcache_range(ಆರಂಭ, stop);
}

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'.
 */
ದೀರ್ಘ get_ram_ಗಾತ್ರ(ದೀರ್ಘ *ತಳ, ದೀರ್ಘ maxsize)
{
	ಅಸ್ಥಿರ ದೀರ್ಘ *addr;
	ದೀರ್ಘ           save[ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_LONG - 1];
	ದೀರ್ಘ           save_ತಳ;
	ದೀರ್ಘ           cnt;
	ದೀರ್ಘ           val;
	ದೀರ್ಘ           ಗಾತ್ರ_2;
	ಪೂರ್ಣಾಂಕ            i = 0;
	ಪೂರ್ಣಾಂಕ            dcache_en = 0;

	ಆದರೆ (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_ಸ್ಥಿತಿ();

	ಗಾಗಿ (cnt = (maxsize / ಗಾತ್ರ(ದೀರ್ಘ)) >> 1; cnt > 0; cnt >>= 1) {
		addr = ತಳ + cnt;	/* pointer arith! */
		sync();
		save[i++] = *addr;
		sync();
		*addr = ~cnt;
		ಆದರೆ (dcache_en)
			dcache_flush_invalidate(addr);
	}

	addr = ತಳ;
	sync();
	save_ತಳ = *addr;
	sync();
	*addr = 0;

	sync();
	ಆದರೆ (dcache_en)
		dcache_flush_invalidate(addr);

	ಆದರೆ ((val = *addr) != 0) {
		/* Restore the original data before leaving the function. */
		sync();
		*ತಳ = save_ತಳ;
		ಗಾಗಿ (cnt = 1; cnt < maxsize / ಗಾತ್ರ(ದೀರ್ಘ); cnt <<= 1) {
			addr  = ತಳ + cnt;
			sync();
			*addr = save[--i];
			ಆದರೆ (dcache_en)
				dcache_flush_invalidate(addr);
		}
		ಹಿಂತಿರುಗಿ (0);
	}

	ಗಾಗಿ (cnt = 1; cnt < maxsize / ಗಾತ್ರ(ದೀರ್ಘ); cnt <<= 1) {
		addr = ತಳ + cnt;	/* pointer arith! */
		val = *addr;
		*addr = save[--i];
		ಆದರೆ (dcache_en)
			dcache_flush_invalidate(addr);
		ಆದರೆ (val != ~cnt) {
			ಗಾತ್ರ_2 = cnt * ಗಾತ್ರ(ದೀರ್ಘ);
			/*
			 * Restore the original data
			 * before leaving the function.
			 */
			ಗಾಗಿ (cnt <<= 1;
			     cnt < maxsize / ಗಾತ್ರ(ದೀರ್ಘ);
			     cnt <<= 1) {
				addr  = ತಳ + cnt;
				*addr = save[--i];
				ಆದರೆ (dcache_en)
					dcache_flush_invalidate(addr);
			}
			/* warning: don't restore save_base in this case,
			 * it is already done in the loop because
			 * base and base+size share the same physical memory
			 * and *base is saved after *(base+size) modification
			 * in first loop
			 */
			ಹಿಂತಿರುಗಿ (ಗಾತ್ರ_2);
		}
	}
	*ತಳ = save_ತಳ;
	ಆದರೆ (dcache_en)
		dcache_flush_invalidate(ತಳ);

	ಹಿಂತಿರುಗಿ (maxsize);
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
ದೀರ್ಘ probe_ram_ಗಾತ್ರ_by_alias(ಸ್ಥಿರಾಂಕ ರಚನೆ ram_alias_check *checks)
{
	ದೀರ್ಘ save[2];
	ದೀರ್ಘ pat;
	ಪೂರ್ಣಾಂಕ dcache_en = 0;
	ದೀರ್ಘ ret = 0;

	ಆದರೆ (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_ಸ್ಥಿತಿ();

	ಆಗ (checks->probe_addr && !ret) {
		ಅಸ್ಥಿರ ದೀರ್ಘ *d = checks->probe_addr;
		ಅಸ್ಥಿರ ದೀರ್ಘ *s = checks->alias_addr;

		save[0] = *s;
		save[1] = *d;
		/* Ensure s is written and not cached */
		ಆದರೆ (dcache_en)
			dcache_flush_invalidate(s);

		pat = ~save[0];
		*d = pat;
		sync();
		ಆದರೆ (dcache_en)
			dcache_flush_invalidate(d);

		/*
		 * Make sure the test pattern is observable at the probe
		 * address before checking whether it is also visible through
		 * the alias address.
		 */
		ಆದರೆ (*d != pat) {
			*d = save[1];
			sync();
			ಆದರೆ (dcache_en)
				dcache_flush_invalidate(d);
			checks++;
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (*s != pat)
			ret = checks->ಗಾತ್ರ_2;

		/* Restore content */
		*d = save[1];
		sync();
		ಆದರೆ (dcache_en)
			dcache_flush_invalidate(d);

		*s = save[0];
		sync();
		ಆದರೆ (dcache_en)
			dcache_flush_invalidate(s);

		checks++;
	}

	ಹಿಂತಿರುಗಿ ret;
}

phys_ಗಾತ್ರ_t __weak get_effective_memsize(ಶೂನ್ಯ)
{
	phys_ಗಾತ್ರ_t ram_ಗಾತ್ರ = gd->ram_ಗಾತ್ರ;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_MPC85xx
	/*
	 * Check for overflow and limit ram size to some representable value.
	 * It is required that ram_base + ram_size must be representable by
	 * phys_size_t type and must be aligned by direct access, therefore
	 * calculate it from last 4kB sector which should work as alignment
	 * on any platform.
	 */
	ಆದರೆ (gd->ram_ತಳ + ram_ಗಾತ್ರ < gd->ram_ತಳ)
		ram_ಗಾತ್ರ = ((phys_ಗಾತ್ರ_t)~0xfffULL) - gd->ram_ತಳ;
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CFG_MAX_MEM_MAPPED
	ಹಿಂತಿರುಗಿ ram_ಗಾತ್ರ;
#ಇಲ್ಲದಿದ್ದರೆ
	/* limit stack to what we can reasonable map */
	ಹಿಂತಿರುಗಿ ((ram_ಗಾತ್ರ > CFG_MAX_MEM_MAPPED) ?
		CFG_MAX_MEM_MAPPED : ram_ಗಾತ್ರ);
#ಆದರೆ_ಅಂತ್ಯ
}
