/*
 * U-Boot - stub functions for common kgdb code,
 *          can be overridden in board specific files
 *
 * Copyright 2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <cpu_func.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kgdb.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <serial.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/ptrace.h>

ਪੂਰਨ (*debugger_exception_handler)(ਬਣਤਰ pt_regs *);

__attribute__((weak))
ਖਾਲੀ kgdb_serial_init(ਖਾਲੀ)
{
	puts("[on serial] ");
}

__attribute__((weak))
ਖਾਲੀ putDebugChar(ਪੂਰਨ c)
{
	serial_putc(c);
}

__attribute__((weak))
ਖਾਲੀ putDebugStr(ਅਚਲ ਅੱਖਰ *str)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DEBUG
	serial_puts(str);
#ਜੇ_ਅੰਤ
}

__attribute__((weak))
ਪੂਰਨ getDebugChar(ਖਾਲੀ)
{
	ਵਾਪਸ serial_getc();
}

__attribute__((weak))
ਖਾਲੀ kgdb_interruptible(ਪੂਰਨ yes)
{
	ਵਾਪਸ;
}

__attribute__((weak))
ਖਾਲੀ kgdb_flush_cache_range(ਖਾਲੀ *from, ਖਾਲੀ *ਮੰਜ਼ਿਲ)
{
	flush_cache((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)from, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(ਮੰਜ਼ਿਲ - from));
}

__attribute__((weak))
ਖਾਲੀ kgdb_flush_cache_all(ਖਾਲੀ)
{
	ਜੇ (dcache_ਸਥਿਤੀ()) {
		dcache_disable();
		dcache_enable();
	}
	ਜੇ (icache_ਸਥਿਤੀ()) {
		icache_disable();
		icache_enable();
	}
}
