/*
 * U-Boot - stub functions for common kgdb code,
 *          can be overridden in board specific files
 *
 * Copyright 2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#ضمن <cpu_func.h>
#ضمن <kgdb.h>
#ضمن <serial.h>
#ضمن <asm/ptrace.h>

صحيح (*debugger_exception_handler)(بنية pt_regs *);

__attribute__((weak))
فراغ kgdb_serial_init(فراغ)
{
	puts("[on serial] ");
}

__attribute__((weak))
فراغ putDebugChar(صحيح c)
{
	serial_putc(c);
}

__attribute__((weak))
فراغ putDebugStr(ثابت حرف *str)
{
#جيڪڏهن_وصف_ٿيل DEBUG
	serial_puts(str);
#جيڪڏهن_پورو
}

__attribute__((weak))
صحيح getDebugChar(فراغ)
{
	أرجع serial_getc();
}

__attribute__((weak))
فراغ kgdb_interruptible(صحيح yes)
{
	أرجع;
}

__attribute__((weak))
فراغ kgdb_flush_cache_range(فراغ *from, فراغ *منزل)
{
	flush_cache((بنانشان ڊگهو)from, (بنانشان ڊگهو)(منزل - from));
}

__attribute__((weak))
فراغ kgdb_flush_cache_all(فراغ)
{
	إذا (dcache_حالت()) {
		dcache_disable();
		dcache_enable();
	}
	إذا (icache_حالت()) {
		icache_disable();
		icache_enable();
	}
}
