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

DECLARE_GLOBAL_معلومات_PTR;

#که_تعریف_شوی CONFIG_SYS_CACHELINE_اندازه
# define MEMSIZE_CACHELINE_اندازه CONFIG_SYS_CACHELINE_اندازه
#وإلا
/* Just use the greatest cache flush alignment requirement I'm aware of */
# define MEMSIZE_CACHELINE_اندازه 128
#د_که_پای

#که_تعریف_شوی __PPC__
/*
 * At least on G2 PowerPC cores, sequential accesses to non-existent
 * memory must be synchronized.
 */
# include <asm/io.h>	/* for sync() */
#وإلا
# define sync()		/* nothing */
#د_که_پای

ساكن فراغ dcache_flush_invalidate(متطاير اوږد *p)
{
	uintptr_t پیل, stop;
	پیل = ALIGN_ښکته((uintptr_t)p, MEMSIZE_CACHELINE_اندازه);
	stop = پیل + MEMSIZE_CACHELINE_اندازه;
	flush_dcache_range(پیل, stop);
	invalidate_dcache_range(پیل, stop);
}

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'.
 */
اوږد get_ram_اندازه(اوږد *بنسټ, اوږد maxsize)
{
	متطاير اوږد *addr;
	اوږد           save[بټونه_فی_LONG - 1];
	اوږد           save_بنسټ;
	اوږد           cnt;
	اوږد           val;
	اوږد           اندازه;
	صحيح            i = 0;
	صحيح            dcache_en = 0;

	إذا (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_حالت();

	لكل (cnt = (maxsize / حجم(اوږد)) >> 1; cnt > 0; cnt >>= 1) {
		addr = بنسټ + cnt;	/* pointer arith! */
		sync();
		save[i++] = *addr;
		sync();
		*addr = ~cnt;
		إذا (dcache_en)
			dcache_flush_invalidate(addr);
	}

	addr = بنسټ;
	sync();
	save_بنسټ = *addr;
	sync();
	*addr = 0;

	sync();
	إذا (dcache_en)
		dcache_flush_invalidate(addr);

	إذا ((val = *addr) != 0) {
		/* Restore the original data before leaving the function. */
		sync();
		*بنسټ = save_بنسټ;
		لكل (cnt = 1; cnt < maxsize / حجم(اوږد); cnt <<= 1) {
			addr  = بنسټ + cnt;
			sync();
			*addr = save[--i];
			إذا (dcache_en)
				dcache_flush_invalidate(addr);
		}
		أرجع (0);
	}

	لكل (cnt = 1; cnt < maxsize / حجم(اوږد); cnt <<= 1) {
		addr = بنسټ + cnt;	/* pointer arith! */
		val = *addr;
		*addr = save[--i];
		إذا (dcache_en)
			dcache_flush_invalidate(addr);
		إذا (val != ~cnt) {
			اندازه = cnt * حجم(اوږد);
			/*
			 * Restore the original data
			 * before leaving the function.
			 */
			لكل (cnt <<= 1;
			     cnt < maxsize / حجم(اوږد);
			     cnt <<= 1) {
				addr  = بنسټ + cnt;
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
			أرجع (اندازه);
		}
	}
	*بنسټ = save_بنسټ;
	إذا (dcache_en)
		dcache_flush_invalidate(بنسټ);

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
اوږد probe_ram_اندازه_by_alias(ثابت بنية ram_alias_check *checks)
{
	اوږد save[2];
	اوږد pat;
	صحيح dcache_en = 0;
	اوږد ret = 0;

	إذا (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_حالت();

	طالما (checks->probe_addr && !ret) {
		متطاير اوږد *d = checks->probe_addr;
		متطاير اوږد *s = checks->alias_addr;

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
			ret = checks->اندازه;

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

phys_اندازه_t __weak get_effective_memsize(فراغ)
{
	phys_اندازه_t ram_اندازه = gd->ram_اندازه;

#که_تعریف_شوی CONFIG_MPC85xx
	/*
	 * Check for overflow and limit ram size to some representable value.
	 * It is required that ram_base + ram_size must be representable by
	 * phys_size_t type and must be aligned by direct access, therefore
	 * calculate it from last 4kB sector which should work as alignment
	 * on any platform.
	 */
	إذا (gd->ram_بنسټ + ram_اندازه < gd->ram_بنسټ)
		ram_اندازه = ((phys_اندازه_t)~0xfffULL) - gd->ram_بنسټ;
#د_که_پای

#که_نه_تعریف_شوی CFG_MAX_MEM_MAPPED
	أرجع ram_اندازه;
#وإلا
	/* limit stack to what we can reasonable map */
	أرجع ((ram_اندازه > CFG_MAX_MEM_MAPPED) ?
		CFG_MAX_MEM_MAPPED : ram_اندازه);
#د_که_پای
}
