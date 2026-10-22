/*
 * U-Boot - stub functions for common kgdb code,
 *          can be overridden in board specific files
 *
 * Copyright 2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#అన్తర్భావ <cpu_func.h>
#అన్తర్భావ <kgdb.h>
#అన్తర్భావ <serial.h>
#అన్తర్భావ <asm/ptrace.h>

పూర్ణాంకం (*debugger_exception_handler)(నిర్మాణం pt_regs *);

__attribute__((weak))
శూన్య kgdb_serial_init(శూన్య)
{
	puts("[on serial] ");
}

__attribute__((weak))
శూన్య putDebugChar(పూర్ణాంకం c)
{
	serial_putc(c);
}

__attribute__((weak))
శూన్య putDebugStr(స్థిరరాశి అక్షరం *str)
{
#నిర్వచితమైతే DEBUG
	serial_puts(str);
#అయితే_ముగింపు
}

__attribute__((weak))
పూర్ణాంకం getDebugChar(శూన్య)
{
	తిరిగి serial_getc();
}

__attribute__((weak))
శూన్య kgdb_interruptible(పూర్ణాంకం yes)
{
	తిరిగి;
}

__attribute__((weak))
శూన్య kgdb_flush_cache_range(శూన్య *from, శూన్య *గమ్యం)
{
	flush_cache((గుర్తులేని దీర్ఘ)from, (గుర్తులేని దీర్ఘ)(గమ్యం - from));
}

__attribute__((weak))
శూన్య kgdb_flush_cache_all(శూన్య)
{
	ఒకవేళ (dcache_స్థితి()) {
		dcache_disable();
		dcache_enable();
	}
	ఒకవేళ (icache_స్థితి()) {
		icache_disable();
		icache_enable();
	}
}
