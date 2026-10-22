/*
 * U-Boot - stub functions for common kgdb code,
 *          can be overridden in board specific files
 *
 * Copyright 2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#شامل <cpu_func.h>
#شامل <kgdb.h>
#شامل <serial.h>
#شامل <asm/ptrace.h>

صحیح (*debugger_exception_handler)(ساختار pt_regs *);

__attribute__((weak))
تهی kgdb_serial_init(تهی)
{
	puts("[on serial] ");
}

__attribute__((weak))
تهی putDebugChar(صحیح c)
{
	serial_putc(c);
}

__attribute__((weak))
تهی putDebugStr(ثابت نویسه *str)
{
#ifdef DEBUG
	serial_puts(str);
#endif
}

__attribute__((weak))
صحیح getDebugChar(تهی)
{
	بازگشت serial_getc();
}

__attribute__((weak))
تهی kgdb_interruptible(صحیح yes)
{
	بازگشت;
}

__attribute__((weak))
تهی kgdb_flush_cache_range(تهی *from, تهی *to)
{
	flush_cache((unsigned long)from, (unsigned long)(to - from));
}

__attribute__((weak))
تهی kgdb_flush_cache_all(تهی)
{
	اگر (dcache_status()) {
		dcache_disable();
		dcache_enable();
	}
	اگر (icache_status()) {
		icache_disable();
		icache_enable();
	}
}
