// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#شامل <config.h>
#شامل <init.h>
#شامل <asm/global_data.h>
#شامل <cpu_func.h>
#شامل <stdint.h>

DECLARE_GLOBAL_ڈیٹا_PTR;

#اگر_تعریف_شدہ CONFIG_SYS_CACHELINE_سائز
# define MEMSIZE_CACHELINE_سائز CONFIG_SYS_CACHELINE_سائز
#ورنہ
/* Just use the greatest cache flush alignment requirement I'm aware of */
# define MEMSIZE_CACHELINE_سائز 128
#اگر_ختم

#اگر_تعریف_شدہ __PPC__
/*
 * At least on G2 PowerPC cores, sequential accesses to non-existent
 * memory must be synchronized.
 */
# include <asm/io.h>	/* for sync() */
#ورنہ
# define sync()		/* nothing */
#اگر_ختم

ساکن خالی dcache_flush_invalidate(متلون طویل *p)
{
	uintptr_t آغاز, stop;
	آغاز = ALIGN_نیچے_کو((uintptr_t)p, MEMSIZE_CACHELINE_سائز);
	stop = آغاز + MEMSIZE_CACHELINE_سائز;
	flush_dcache_range(آغاز, stop);
	invalidate_dcache_range(آغاز, stop);
}

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'.
 */
طویل get_ram_سائز(طویل *بنیاد, طویل maxsize)
{
	متلون طویل *addr;
	طویل           save[بٹ_فی_LONG - 1];
	طویل           save_بنیاد;
	طویل           cnt;
	طویل           val;
	طویل           سائز;
	صحیح            i = 0;
	صحیح            dcache_en = 0;

	اگر (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_حالت();

	برائے (cnt = (maxsize / حجم(طویل)) >> 1; cnt > 0; cnt >>= 1) {
		addr = بنیاد + cnt;	/* pointer arith! */
		sync();
		save[i++] = *addr;
		sync();
		*addr = ~cnt;
		اگر (dcache_en)
			dcache_flush_invalidate(addr);
	}

	addr = بنیاد;
	sync();
	save_بنیاد = *addr;
	sync();
	*addr = 0;

	sync();
	اگر (dcache_en)
		dcache_flush_invalidate(addr);

	اگر ((val = *addr) != 0) {
		/* Restore the original data before leaving the function. */
		sync();
		*بنیاد = save_بنیاد;
		برائے (cnt = 1; cnt < maxsize / حجم(طویل); cnt <<= 1) {
			addr  = بنیاد + cnt;
			sync();
			*addr = save[--i];
			اگر (dcache_en)
				dcache_flush_invalidate(addr);
		}
		واپس (0);
	}

	برائے (cnt = 1; cnt < maxsize / حجم(طویل); cnt <<= 1) {
		addr = بنیاد + cnt;	/* pointer arith! */
		val = *addr;
		*addr = save[--i];
		اگر (dcache_en)
			dcache_flush_invalidate(addr);
		اگر (val != ~cnt) {
			سائز = cnt * حجم(طویل);
			/*
			 * Restore the original data
			 * before leaving the function.
			 */
			برائے (cnt <<= 1;
			     cnt < maxsize / حجم(طویل);
			     cnt <<= 1) {
				addr  = بنیاد + cnt;
				*addr = save[--i];
				اگر (dcache_en)
					dcache_flush_invalidate(addr);
			}
			/* warning: don't restore save_base in this case,
			 * it is already done in the loop because
			 * base and base+size share the same physical memory
			 * and *base is saved after *(base+size) modification
			 * in first loop
			 */
			واپس (سائز);
		}
	}
	*بنیاد = save_بنیاد;
	اگر (dcache_en)
		dcache_flush_invalidate(بنیاد);

	واپس (maxsize);
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
طویل probe_ram_سائز_by_alias(ثابت ساخت ram_alias_check *checks)
{
	طویل save[2];
	طویل pat;
	صحیح dcache_en = 0;
	طویل ret = 0;

	اگر (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		dcache_en = dcache_حالت();

	جب (checks->probe_addr && !ret) {
		متلون طویل *d = checks->probe_addr;
		متلون طویل *s = checks->alias_addr;

		save[0] = *s;
		save[1] = *d;
		/* Ensure s is written and not cached */
		اگر (dcache_en)
			dcache_flush_invalidate(s);

		pat = ~save[0];
		*d = pat;
		sync();
		اگر (dcache_en)
			dcache_flush_invalidate(d);

		/*
		 * Make sure the test pattern is observable at the probe
		 * address before checking whether it is also visible through
		 * the alias address.
		 */
		اگر (*d != pat) {
			*d = save[1];
			sync();
			اگر (dcache_en)
				dcache_flush_invalidate(d);
			checks++;
			جاری;
		}

		اگر (*s != pat)
			ret = checks->سائز;

		/* Restore content */
		*d = save[1];
		sync();
		اگر (dcache_en)
			dcache_flush_invalidate(d);

		*s = save[0];
		sync();
		اگر (dcache_en)
			dcache_flush_invalidate(s);

		checks++;
	}

	واپس ret;
}

phys_سائز_t __weak get_effective_memsize(خالی)
{
	phys_سائز_t ram_سائز = gd->ram_سائز;

#اگر_تعریف_شدہ CONFIG_MPC85xx
	/*
	 * Check for overflow and limit ram size to some representable value.
	 * It is required that ram_base + ram_size must be representable by
	 * phys_size_t type and must be aligned by direct access, therefore
	 * calculate it from last 4kB sector which should work as alignment
	 * on any platform.
	 */
	اگر (gd->ram_بنیاد + ram_سائز < gd->ram_بنیاد)
		ram_سائز = ((phys_سائز_t)~0xfffULL) - gd->ram_بنیاد;
#اگر_ختم

#اگر_غیر_تعریف_شدہ CFG_MAX_MEM_MAPPED
	واپس ram_سائز;
#ورنہ
	/* limit stack to what we can reasonable map */
	واپس ((ram_سائز > CFG_MAX_MEM_MAPPED) ?
		CFG_MAX_MEM_MAPPED : ram_سائز);
#اگر_ختم
}
