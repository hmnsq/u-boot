/*
 * U-Boot - stub functions for common kgdb code,
 *          can be overridden in board specific files
 *
 * Copyright 2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#அந்தர்பாவ <cpu_func.h>
#அந்தர்பாவ <kgdb.h>
#அந்தர்பாவ <serial.h>
#அந்தர்பாவ <asm/ptrace.h>

முழு (*debugger_exception_handler)(கட்டமைப்பு pt_regs *);

__attribute__((weak))
வெறுமை kgdb_serial_init(வெறுமை)
{
	puts("[on serial] ");
}

__attribute__((weak))
வெறுமை putDebugChar(முழு c)
{
	serial_putc(c);
}

__attribute__((weak))
வெறுமை putDebugStr(மாறிலி எழுத்து *str)
{
#ifdef DEBUG
	serial_puts(str);
#endif
}

__attribute__((weak))
முழு getDebugChar(வெறுமை)
{
	திரும்பு serial_getc();
}

__attribute__((weak))
வெறுமை kgdb_interruptible(முழு yes)
{
	திரும்பு;
}

__attribute__((weak))
வெறுமை kgdb_flush_cache_range(வெறுமை *from, வெறுமை *to)
{
	flush_cache((unsigned long)from, (unsigned long)(to - from));
}

__attribute__((weak))
வெறுமை kgdb_flush_cache_all(வெறுமை)
{
	எனில் (dcache_status()) {
		dcache_disable();
		dcache_enable();
	}
	எனில் (icache_status()) {
		icache_disable();
		icache_enable();
	}
}
