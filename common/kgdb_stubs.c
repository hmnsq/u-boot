/*
 * U-Boot - stub functions for common kgdb code,
 *          can be overridden in board specific files
 *
 * Copyright 2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#ಅನ್ತರ್ಭಾವ <cpu_func.h>
#ಅನ್ತರ್ಭಾವ <kgdb.h>
#ಅನ್ತರ್ಭಾವ <serial.h>
#ಅನ್ತರ್ಭಾವ <asm/ptrace.h>

ಪೂರ್ಣಾಙ್ಕ (*debugger_exception_handler)(ಸಂರಚನಾ pt_regs *);

__attribute__((weak))
ಶೂನ್ಯ kgdb_serial_init(ಶೂನ್ಯ)
{
	puts("[on serial] ");
}

__attribute__((weak))
ಶೂನ್ಯ putDebugChar(ಪೂರ್ಣಾಙ್ಕ c)
{
	serial_putc(c);
}

__attribute__((weak))
ಶೂನ್ಯ putDebugStr(ಧ್ರುವ ವರ್ಣ *str)
{
#ifdef DEBUG
	serial_puts(str);
#endif
}

__attribute__((weak))
ಪೂರ್ಣಾಙ್ಕ getDebugChar(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ serial_getc();
}

__attribute__((weak))
ಶೂನ್ಯ kgdb_interruptible(ಪೂರ್ಣಾಙ್ಕ yes)
{
	ಪ್ರತ್ಯಾಗಮ;
}

__attribute__((weak))
ಶೂನ್ಯ kgdb_flush_cache_range(ಶೂನ್ಯ *from, ಶೂನ್ಯ *to)
{
	flush_cache((unsigned long)from, (unsigned long)(to - from));
}

__attribute__((weak))
ಶೂನ್ಯ kgdb_flush_cache_all(ಶೂನ್ಯ)
{
	ಯದಿ (dcache_status()) {
		dcache_disable();
		dcache_enable();
	}
	ಯದಿ (icache_status()) {
		icache_disable();
		icache_enable();
	}
}
