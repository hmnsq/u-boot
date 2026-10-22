/*
 * U-Boot - stub functions for common kgdb code,
 *          can be overridden in board specific files
 *
 * Copyright 2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#അന്തര്ഭാവ <cpu_func.h>
#അന്തര്ഭാവ <kgdb.h>
#അന്തര്ഭാവ <serial.h>
#അന്തര്ഭാവ <asm/ptrace.h>

പൂർണ്ണസംഖ്യ (*debugger_exception_handler)(ഘടന pt_regs *);

__attribute__((weak))
ശൂന്യം kgdb_serial_init(ശൂന്യം)
{
	puts("[on serial] ");
}

__attribute__((weak))
ശൂന്യം putDebugChar(പൂർണ്ണസംഖ്യ c)
{
	serial_putc(c);
}

__attribute__((weak))
ശൂന്യം putDebugStr(സ്ഥിരം അക്ഷരം *str)
{
#ifdef DEBUG
	serial_puts(str);
#endif
}

__attribute__((weak))
പൂർണ്ണസംഖ്യ getDebugChar(ശൂന്യം)
{
	തിരികെ serial_getc();
}

__attribute__((weak))
ശൂന്യം kgdb_interruptible(പൂർണ്ണസംഖ്യ yes)
{
	തിരികെ;
}

__attribute__((weak))
ശൂന്യം kgdb_flush_cache_range(ശൂന്യം *from, ശൂന്യം *to)
{
	flush_cache((unsigned long)from, (unsigned long)(to - from));
}

__attribute__((weak))
ശൂന്യം kgdb_flush_cache_all(ശൂന്യം)
{
	എങ്കിൽ (dcache_status()) {
		dcache_disable();
		dcache_enable();
	}
	എങ്കിൽ (icache_status()) {
		icache_disable();
		icache_enable();
	}
}
