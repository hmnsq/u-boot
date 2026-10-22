/*
 * U-Boot - stub functions for common kgdb code,
 *          can be overridden in board specific files
 *
 * Copyright 2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <cpu_func.h>
#include <kgdb.h>
#include <serial.h>
#include <asm/ptrace.h>

පූර්ණ_සංඛ්‍යා (*debugger_exception_handler)(sa_nracanaa pt_regs *);

__attribute__((weak))
void kgdb_serial_init(void)
{
	puts("[on serial] ");
}

__attribute__((weak))
void putDebugChar(පූර්ණ_සංඛ්‍යා c)
{
	serial_putc(c);
}

__attribute__((weak))
void putDebugStr(const var_na *str)
{
#ifdef DEBUG
	serial_puts(str);
#endif
}

__attribute__((weak))
පූර්ණ_සංඛ්‍යා getDebugChar(void)
{
	ආපසු serial_getc();
}

__attribute__((weak))
void kgdb_interruptible(පූර්ණ_සංඛ්‍යා yes)
{
	ආපසු;
}

__attribute__((weak))
void kgdb_flush_cache_range(void *from, void *to)
{
	flush_cache((unsigned long)from, (unsigned long)(to - from));
}

__attribute__((weak))
void kgdb_flush_cache_all(void)
{
	නම් (dcache_status()) {
		dcache_disable();
		dcache_enable();
	}
	නම් (icache_status()) {
		icache_disable();
		icache_enable();
	}
}
