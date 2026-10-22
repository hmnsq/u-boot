/*
 * U-Boot - stub functions for common kgdb code,
 *          can be overridden in board specific files
 *
 * Copyright 2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#अन्तर्भाव <cpu_func.h>
#अन्तर्भाव <kgdb.h>
#अन्तर्भाव <serial.h>
#अन्तर्भाव <asm/ptrace.h>

पूर्णाङ्क (*debugger_exception_handler)(संरचना pt_regs *);

__attribute__((weak))
शून्य kgdb_serial_init(शून्य)
{
	puts("[on serial] ");
}

__attribute__((weak))
शून्य putDebugChar(पूर्णाङ्क c)
{
	serial_putc(c);
}

__attribute__((weak))
शून्य putDebugStr(ध्रुव वर्ण *str)
{
#ifdef DEBUG
	serial_puts(str);
#endif
}

__attribute__((weak))
पूर्णाङ्क getDebugChar(शून्य)
{
	प्रत्यागम serial_getc();
}

__attribute__((weak))
शून्य kgdb_interruptible(पूर्णाङ्क yes)
{
	प्रत्यागम;
}

__attribute__((weak))
शून्य kgdb_flush_cache_range(शून्य *from, शून्य *to)
{
	flush_cache((unsigned long)from, (unsigned long)(to - from));
}

__attribute__((weak))
शून्य kgdb_flush_cache_all(शून्य)
{
	यदि (dcache_status()) {
		dcache_disable();
		dcache_enable();
	}
	यदि (icache_status()) {
		icache_disable();
		icache_enable();
	}
}
