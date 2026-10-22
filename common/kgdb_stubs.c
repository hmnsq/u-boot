/*
 * U-Boot - stub functions for common kgdb code,
 *          can be overridden in board specific files
 *
 * Copyright 2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#অন্তর্ভাৱ <cpu_func.h>
#অন্তর্ভাৱ <kgdb.h>
#অন্তর্ভাৱ <serial.h>
#অন্তর্ভাৱ <asm/ptrace.h>

পূর্ণাঙ্ক (*debugger_exception_handler)(সংরচনা pt_regs *);

__attribute__((weak))
শূন্য kgdb_serial_init(শূন্য)
{
	puts("[on serial] ");
}

__attribute__((weak))
শূন্য putDebugChar(পূর্ণাঙ্ক c)
{
	serial_putc(c);
}

__attribute__((weak))
শূন্য putDebugStr(ধ্রুৱ ৱর্ণ *str)
{
#ifdef DEBUG
	serial_puts(str);
#endif
}

__attribute__((weak))
পূর্ণাঙ্ক getDebugChar(শূন্য)
{
	ঘূৰাই serial_getc();
}

__attribute__((weak))
শূন্য kgdb_interruptible(পূর্ণাঙ্ক yes)
{
	ঘূৰাই;
}

__attribute__((weak))
শূন্য kgdb_flush_cache_range(শূন্য *from, শূন্য *to)
{
	flush_cache((unsigned long)from, (unsigned long)(to - from));
}

__attribute__((weak))
শূন্য kgdb_flush_cache_all(শূন্য)
{
	যদি (dcache_status()) {
		dcache_disable();
		dcache_enable();
	}
	যদি (icache_status()) {
		icache_disable();
		icache_enable();
	}
}
