/*
 * Helper functions for working with the builtin symbol table
 *
 * Copyright (c) 2008-2009 Analog Devices Inc.
 * Licensed under the GPL-2 or later.
 */

/* We need the weak marking as this symbol is provided specially */
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ system_map[] __attribute__((weak));

/* Given an address, return a pointer to the symbol name and store
 * the base address in caddr.  So if the symbol map had an entry:
 *		03fb9b7c_spi_cs_deactivate
 * Then the following call:
 *		unsigned long base;
 *		const char *sym = symbol_lookup(0x03fb9b80, &base);
 * Would end up setting the variables like so:
 *		base = 0x03fb9b7c;
 *		sym = "_spi_cs_deactivate";
 */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಚಿಹ್ನೆ_lookup(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ addr, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *caddr)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sym, *csym;
	ಅಕ್ಷರ *esym;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ sym_addr;

	sym = system_map;
	csym = ಖಾಲಿ;
	*caddr = 0;

	ಆಗ (*sym) {
		sym_addr = hextoul(sym, &esym);
		sym = esym;
		ಆದರೆ (sym_addr > addr)
			ನಿಲ್ಲಿಸು;
		*caddr = sym_addr;
		csym = sym;
		sym += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(sym) + 1;
	}

	ಹಿಂತಿರುಗಿ csym;
}
