/*
 * U-Boot - stub functions for common kgdb code,
 *          can be overridden in board specific files
 *
 * Copyright 2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#समावेश <cpu_func.h>
#समावेश <kgdb.h>
#समावेश <serial.h>
#समावेश <asm/ptrace.h>

पूर्णांक (*debugger_exception_handler)(काष्ठा pt_regs *);

__attribute__((weak))
व्योम kgdb_serial_init(व्योम)
{
	माला_दो("[on serial] ");
}

__attribute__((weak))
व्योम putDebugChar(पूर्णांक c)
{
	serial_putc(c);
}

__attribute__((weak))
व्योम putDebugStr(स्थिर अक्षर *str)
{
#अगर_घोषित DEBUG
	serial_puts(str);
#पूर्ण_अगर
}

__attribute__((weak))
पूर्णांक getDebugChar(व्योम)
{
	वापस serial_getc();
}

__attribute__((weak))
व्योम kgdb_interruptible(पूर्णांक yes)
{
	वापस;
}

__attribute__((weak))
व्योम kgdb_flush_cache_range(व्योम *from, व्योम *to)
{
	flush_cache((अचिन्हित दीर्घ)from, (अचिन्हित दीर्घ)(to - from));
}

__attribute__((weak))
व्योम kgdb_flush_cache_all(व्योम)
{
	अगर (dcache_status()) {
		dcache_disable();
		dcache_enable();
	}
	अगर (icache_status()) {
		icache_disable();
		icache_enable();
	}
}
