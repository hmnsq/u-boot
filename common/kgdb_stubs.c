/*
 * U-Boot - stub functions for common kgdb code,
 *          can be overridden in board specific files
 *
 * Copyright 2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#અન્તર્ભાવ <cpu_func.h>
#અન્તર્ભાવ <kgdb.h>
#અન્તર્ભાવ <serial.h>
#અન્તર્ભાવ <asm/ptrace.h>

પૂર્ણાંક (*debugger_exception_handler)(સંરચના pt_regs *);

__attribute__((weak))
શૂન્ય kgdb_serial_init(શૂન્ય)
{
	puts("[on serial] ");
}

__attribute__((weak))
શૂન્ય putDebugChar(પૂર્ણાંક c)
{
	serial_putc(c);
}

__attribute__((weak))
શૂન્ય putDebugStr(અચલ અક્ષર *str)
{
#ifdef DEBUG
	serial_puts(str);
#endif
}

__attribute__((weak))
પૂર્ણાંક getDebugChar(શૂન્ય)
{
	પાછા serial_getc();
}

__attribute__((weak))
શૂન્ય kgdb_interruptible(પૂર્ણાંક yes)
{
	પાછા;
}

__attribute__((weak))
શૂન્ય kgdb_flush_cache_range(શૂન્ય *from, શૂન્ય *to)
{
	flush_cache((unsigned long)from, (unsigned long)(to - from));
}

__attribute__((weak))
શૂન્ય kgdb_flush_cache_all(શૂન્ય)
{
	જો (dcache_status()) {
		dcache_disable();
		dcache_enable();
	}
	જો (icache_status()) {
		icache_disable();
		icache_enable();
	}
}
