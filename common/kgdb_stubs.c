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

صحیح (*debugger_exception_handler)(ساخت pt_regs *);

__attribute__((weak))
خالی kgdb_serial_init(خالی)
{
	puts("[on serial] ");
}

__attribute__((weak))
خالی putDebugChar(صحیح c)
{
	serial_putc(c);
}

__attribute__((weak))
خالی putDebugStr(ثابت حرف *str)
{
#اگر_تعریف_شدہ DEBUG
	serial_puts(str);
#اگر_ختم
}

__attribute__((weak))
صحیح getDebugChar(خالی)
{
	واپس serial_getc();
}

__attribute__((weak))
خالی kgdb_interruptible(صحیح yes)
{
	واپس;
}

__attribute__((weak))
خالی kgdb_flush_cache_range(خالی *from, خالی *مقصد)
{
	flush_cache((غیرعلامتی طویل)from, (غیرعلامتی طویل)(مقصد - from));
}

__attribute__((weak))
خالی kgdb_flush_cache_all(خالی)
{
	اگر (dcache_حالت()) {
		dcache_disable();
		dcache_enable();
	}
	اگر (icache_حالت()) {
		icache_disable();
		icache_enable();
	}
}
