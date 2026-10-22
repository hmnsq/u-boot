// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ضمن <config.h>
#ضمن <init.h>
#ضمن <asm/global_data.h>
#ضمن <cpu_func.h>
#ضمن <stdint.h>

DECLARE_GLOBAL_انکڑے_PTR;

#جے_تعریف_شدہ CONFIG_SYS_CACHELINE_ناپ
# define MEMSIZE_CACHELINE_ناپ CONFIG_SYS_CACHELINE_ناپ
#وإلا
/* Just use the greatest cache flush alignment requirement I'm aware of */
# define MEMSIZE_CACHELINE_ناپ 128
#جے_انت

#جے_تعریف_شدہ __PPC__
/*
 * At least on G2 PowerPC cores, sequential accesses to non-existent
 * memory must be synchronized.
 */
# include <asm/io.h>	/* for sync() */
#وإلا
# define sync()		/* nothing */
#جے_انت

ساكن فراغ dcache_flush_invalidate(متطاير لما *p)
{
	uintptr_t شروع, stop;
	شروع = ALIGN_تھلے((uintptr_t)p, MEMSIZE_CACHELINE_ناپ);
	stop = شروع + MEMSIZE_CACHELINE_ناپ;
	flush_dcache_range(شروع, stop);
	invalidate_dcache_range(شروع, stop);
}

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'.
 */
لما get_ram_ناپ(لما *ادھار, لما maxsize)
{
	متطاير لما *addr;
	لما           save[بٹ_فی_LONG - 1];
	لما           save_ادھار;
	لما           cnt;
	لما           val;
	لما           ناپ;
	صحيح            i = 0;
	صحيح            dcache_en = 0;

	إذا (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_حالت();

	لكل (cnt = (maxsize / حجم(لما)) >> 1; cnt > 0; cnt >>= 1) {
		addr = ادھار + cnt;	/* pointer arith! */
		sync();
		save[i++] = *addr;
		sync();
		*addr = ~cnt;
		إذا (dcache_en)
			dcache_flush_invalidate(addr);
	}

	addr = ادھار;
	sync();
	save_ادھار = *addr;
	sync();
	*addr = 0;

	sync();
	إذا (dcache_en)
		dcache_flush_invalidate(addr);

	إذا ((val = *addr) != 0) {
		/* Restore the original data before leaving the function. */
		sync();
		*ادھار = save_ادھار;
		لكل (cnt = 1; cnt < maxsize / حجم(لما); cnt <<= 1) {
			addr  = ادھار + cnt;
			sync();
			*addr = save[--i];
			إذا (dcache_en)
				dcache_flush_invalidate(addr);
		}
		أرجع (0);
	}

	لكل (cnt = 1; cnt < maxsize / حجم(لما); cnt <<= 1) {
		addr = ادھار + cnt;	/* pointer arith! */
		val = *addr;
		*addr = save[--i];
		إذا (dcache_en)
			dcache_flush_invalidate(addr);
		إذا (val != ~cnt) {
			ناپ = cnt * حجم(لما);
			/*
			 * Restore the original data
			 * before leaving the function.
			 */
			لكل (cnt <<= 1;
			     cnt < maxsize / حجم(لما);
			     cnt <<= 1) {
				addr  = ادھار + cnt;
				*addr = save[--i];
				إذا (dcache_en)
					dcache_flush_invalidate(addr);
			}
			/* warning: don't restore save_base in this case,
			 * it is already done in the loop because
			 * base and base+size share the same physical memory
			 * and *base is saved after *(base+size) modification
			 * in first loop
			 */
			أرجع (ناپ);
		}
	}
	*ادھار = save_ادھار;
	إذا (dcache_en)
		dcache_flush_invalidate(ادھار);

	أرجع (maxsize);
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
لما probe_ram_ناپ_by_alias(ثابت بنية ram_alias_check *checks)
{
	لما save[2];
	لما pat;
	صحيح dcache_en = 0;
	لما ret = 0;

	إذا (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_حالت();

	طالما (checks->probe_addr && !ret) {
		متطاير لما *d = checks->probe_addr;
		متطاير لما *s = checks->alias_addr;

		save[0] = *s;
		save[1] = *d;
		/* Ensure s is written and not cached */
		إذا (dcache_en)
			dcache_flush_invalidate(s);

		pat = ~save[0];
		*d = pat;
		sync();
		إذا (dcache_en)
			dcache_flush_invalidate(d);

		/*
		 * Make sure the test pattern is observable at the probe
		 * address before checking whether it is also visible through
		 * the alias address.
		 */
		إذا (*d != pat) {
			*d = save[1];
			sync();
			إذا (dcache_en)
				dcache_flush_invalidate(d);
			checks++;
			استمر;
		}

		إذا (*s != pat)
			ret = checks->ناپ;

		/* Restore content */
		*d = save[1];
		sync();
		إذا (dcache_en)
			dcache_flush_invalidate(d);

		*s = save[0];
		sync();
		إذا (dcache_en)
			dcache_flush_invalidate(s);

		checks++;
	}

	أرجع ret;
}

phys_ناپ_t __weak get_effective_memsize(فراغ)
{
	phys_ناپ_t ram_ناپ = gd->ram_ناپ;

#جے_تعریف_شدہ CONFIG_MPC85xx
	/*
	 * Check for overflow and limit ram size to some representable value.
	 * It is required that ram_base + ram_size must be representable by
	 * phys_size_t type and must be aligned by direct access, therefore
	 * calculate it from last 4kB sector which should work as alignment
	 * on any platform.
	 */
	إذا (gd->ram_ادھار + ram_ناپ < gd->ram_ادھار)
		ram_ناپ = ((phys_ناپ_t)~0xfffULL) - gd->ram_ادھار;
#جے_انت

#جے_بنا_تعریف CFG_MAX_MEM_MAPPED
	أرجع ram_ناپ;
#وإلا
	/* limit stack to what we can reasonable map */
	أرجع ((ram_ناپ > CFG_MAX_MEM_MAPPED) ?
		CFG_MAX_MEM_MAPPED : ram_ناپ);
#جے_انت
}
