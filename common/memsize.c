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

#جيڪڏهن_وصف_ٿيل CONFIG_SYS_CACHELINE_سائيز
# define MEMSIZE_CACHELINE_سائيز CONFIG_SYS_CACHELINE_سائيز
#وإلا
/* Just use the greatest cache flush alignment requirement I'm aware of */
# define MEMSIZE_CACHELINE_سائيز 128
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل __PPC__
/*
 * At least on G2 PowerPC cores, sequential accesses to non-existent
 * memory must be synchronized.
 */
# include <asm/io.h>	/* for sync() */
#وإلا
# define sync()		/* nothing */
#جيڪڏهن_پورو

ساكن فراغ dcache_flush_invalidate(متطاير ڊگهو *p)
{
	uintptr_t شروعات, stop;
	شروعات = ALIGN_هيٺ_ڏانهن((uintptr_t)p, MEMSIZE_CACHELINE_سائيز);
	stop = شروعات + MEMSIZE_CACHELINE_سائيز;
	flush_dcache_range(شروعات, stop);
	invalidate_dcache_range(شروعات, stop);
}

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'.
 */
ڊگهو get_ram_سائيز(ڊگهو *بنياد, ڊگهو maxsize)
{
	متطاير ڊگهو *addr;
	ڊگهو           save[بٽ_في_LONG - 1];
	ڊگهو           save_بنياد;
	ڊگهو           cnt;
	ڊگهو           val;
	ڊگهو           سائيز;
	صحيح            i = 0;
	صحيح            dcache_en = 0;

	إذا (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_حالت();

	لكل (cnt = (maxsize / حجم(ڊگهو)) >> 1; cnt > 0; cnt >>= 1) {
		addr = بنياد + cnt;	/* pointer arith! */
		sync();
		save[i++] = *addr;
		sync();
		*addr = ~cnt;
		إذا (dcache_en)
			dcache_flush_invalidate(addr);
	}

	addr = بنياد;
	sync();
	save_بنياد = *addr;
	sync();
	*addr = 0;

	sync();
	إذا (dcache_en)
		dcache_flush_invalidate(addr);

	إذا ((val = *addr) != 0) {
		/* Restore the original data before leaving the function. */
		sync();
		*بنياد = save_بنياد;
		لكل (cnt = 1; cnt < maxsize / حجم(ڊگهو); cnt <<= 1) {
			addr  = بنياد + cnt;
			sync();
			*addr = save[--i];
			إذا (dcache_en)
				dcache_flush_invalidate(addr);
		}
		أرجع (0);
	}

	لكل (cnt = 1; cnt < maxsize / حجم(ڊگهو); cnt <<= 1) {
		addr = بنياد + cnt;	/* pointer arith! */
		val = *addr;
		*addr = save[--i];
		إذا (dcache_en)
			dcache_flush_invalidate(addr);
		إذا (val != ~cnt) {
			سائيز = cnt * حجم(ڊگهو);
			/*
			 * Restore the original data
			 * before leaving the function.
			 */
			لكل (cnt <<= 1;
			     cnt < maxsize / حجم(ڊگهو);
			     cnt <<= 1) {
				addr  = بنياد + cnt;
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
			أرجع (سائيز);
		}
	}
	*بنياد = save_بنياد;
	إذا (dcache_en)
		dcache_flush_invalidate(بنياد);

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
ڊگهو probe_ram_سائيز_by_alias(ثابت بنية ram_alias_check *checks)
{
	ڊگهو save[2];
	ڊگهو pat;
	صحيح dcache_en = 0;
	ڊگهو ret = 0;

	إذا (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_حالت();

	طالما (checks->probe_addr && !ret) {
		متطاير ڊگهو *d = checks->probe_addr;
		متطاير ڊگهو *s = checks->alias_addr;

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
			ret = checks->سائيز;

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

phys_سائيز_t __weak get_effective_memsize(فراغ)
{
	phys_سائيز_t ram_سائيز = gd->ram_سائيز;

#جيڪڏهن_وصف_ٿيل CONFIG_MPC85xx
	/*
	 * Check for overflow and limit ram size to some representable value.
	 * It is required that ram_base + ram_size must be representable by
	 * phys_size_t type and must be aligned by direct access, therefore
	 * calculate it from last 4kB sector which should work as alignment
	 * on any platform.
	 */
	إذا (gd->ram_بنياد + ram_سائيز < gd->ram_بنياد)
		ram_سائيز = ((phys_سائيز_t)~0xfffULL) - gd->ram_بنياد;
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل CFG_MAX_MEM_MAPPED
	أرجع ram_سائيز;
#وإلا
	/* limit stack to what we can reasonable map */
	أرجع ((ram_سائيز > CFG_MAX_MEM_MAPPED) ?
		CFG_MAX_MEM_MAPPED : ram_سائيز);
#جيڪڏهن_پورو
}
