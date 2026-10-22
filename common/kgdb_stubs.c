/*
 * U-Boot - stub functions for common kgdb code,
 *          can be overridden in board specific files
 *
 * Copyright 2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#অন্তর্ভাব <cpu_func.h>
#অন্তর্ভাব <kgdb.h>
#অন্তর্ভাব <serial.h>
#অন্তর্ভাব <asm/ptrace.h>

পূর্ণসংখ্যা (*debugger_exception_handler)(কাঠামো pt_regs *);

__attribute__((weak))
শূন্য kgdb_serial_init(শূন্য)
{
	puts("[on serial] ");
}

__attribute__((weak))
শূন্য putDebugChar(পূর্ণসংখ্যা c)
{
	serial_putc(c);
}

__attribute__((weak))
শূন্য putDebugStr(ধ্রুবক অক্ষর *str)
{
#ifdef DEBUG
	serial_puts(str);
#endif
}

__attribute__((weak))
পূর্ণসংখ্যা getDebugChar(শূন্য)
{
	ফেরত serial_getc();
}

__attribute__((weak))
শূন্য kgdb_interruptible(পূর্ণসংখ্যা yes)
{
	ফেরত;
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
