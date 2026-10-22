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
	पङ्क्तिं_लिख("[on serial] ");
}

__attribute__((weak))
शून्य putDebugChar(पूर्णाङ्क c)
{
	serial_putc(c);
}

__attribute__((weak))
शून्य putDebugStr(ध्रुव वर्ण *str)
{
#यदि_परिभाषितम् DEBUG
	serial_puts(str);
#यदि_अन्तः
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
शून्य kgdb_flush_cache_range(शून्य *from, शून्य *गन्तव्यम्)
{
	flush_cache((अचिह्नित दीर्घ)from, (अचिह्नित दीर्घ)(गन्तव्यम् - from));
}

__attribute__((weak))
शून्य kgdb_flush_cache_all(शून्य)
{
	यदि (dcache_स्थितिः()) {
		dcache_disable();
		dcache_enable();
	}
	यदि (icache_स्थितिः()) {
		icache_disable();
		icache_enable();
	}
}
