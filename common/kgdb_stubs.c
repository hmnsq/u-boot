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

ಪೂರ್ಣಾಂಕ (*debugger_exception_handler)(ರಚನೆ pt_regs *);

__attribute__((weak))
ಶೂನ್ಯ kgdb_serial_init(ಶೂನ್ಯ)
{
	puts("[on serial] ");
}

__attribute__((weak))
ಶೂನ್ಯ putDebugChar(ಪೂರ್ಣಾಂಕ c)
{
	serial_putc(c);
}

__attribute__((weak))
ಶೂನ್ಯ putDebugStr(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DEBUG
	serial_puts(str);
#ಆದರೆ_ಅಂತ್ಯ
}

__attribute__((weak))
ಪೂರ್ಣಾಂಕ getDebugChar(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ serial_getc();
}

__attribute__((weak))
ಶೂನ್ಯ kgdb_interruptible(ಪೂರ್ಣಾಂಕ yes)
{
	ಹಿಂತಿರುಗಿ;
}

__attribute__((weak))
ಶೂನ್ಯ kgdb_flush_cache_range(ಶೂನ್ಯ *from, ಶೂನ್ಯ *ಗಮ್ಯ)
{
	flush_cache((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)from, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(ಗಮ್ಯ - from));
}

__attribute__((weak))
ಶೂನ್ಯ kgdb_flush_cache_all(ಶೂನ್ಯ)
{
	ಆದರೆ (dcache_ಸ್ಥಿತಿ()) {
		dcache_disable();
		dcache_enable();
	}
	ಆದರೆ (icache_ಸ್ಥಿತಿ()) {
		icache_disable();
		icache_enable();
	}
}
