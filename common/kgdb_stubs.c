/*
 * U-Boot - stub functions for common kgdb code,
 *          can be overridden in board specific files
 *
 * Copyright 2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#ଅନ୍ତର୍ଭାଵ <cpu_func.h>
#ଅନ୍ତର୍ଭାଵ <kgdb.h>
#ଅନ୍ତର୍ଭାଵ <serial.h>
#ଅନ୍ତର୍ଭାଵ <asm/ptrace.h>

ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*debugger_exception_handler)(ସଂରଚନା pt_regs *);

__attribute__((weak))
ଶୂନ୍ୟ kgdb_serial_init(ଶୂନ୍ୟ)
{
	puts("[on serial] ");
}

__attribute__((weak))
ଶୂନ୍ୟ putDebugChar(ପୂର୍ଣ୍ଣସଂଖ୍ୟା c)
{
	serial_putc(c);
}

__attribute__((weak))
ଶୂନ୍ୟ putDebugStr(ଅଚଳ ଅକ୍ଷର *str)
{
#ifdef DEBUG
	serial_puts(str);
#endif
}

__attribute__((weak))
ପୂର୍ଣ୍ଣସଂଖ୍ୟା getDebugChar(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ serial_getc();
}

__attribute__((weak))
ଶୂନ୍ୟ kgdb_interruptible(ପୂର୍ଣ୍ଣସଂଖ୍ୟା yes)
{
	ଫେରନ୍ତୁ;
}

__attribute__((weak))
ଶୂନ୍ୟ kgdb_flush_cache_range(ଶୂନ୍ୟ *from, ଶୂନ୍ୟ *to)
{
	flush_cache((unsigned long)from, (unsigned long)(to - from));
}

__attribute__((weak))
ଶୂନ୍ୟ kgdb_flush_cache_all(ଶୂନ୍ୟ)
{
	ଯଦି (dcache_status()) {
		dcache_disable();
		dcache_enable();
	}
	ଯଦି (icache_status()) {
		icache_disable();
		icache_enable();
	}
}
