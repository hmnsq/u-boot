/*
 * Helper functions for working with the builtin symbol table
 *
 * Copyright (c) 2008-2009 Analog Devices Inc.
 * Licensed under the GPL-2 or later.
 */

/* We need the weak marking as this symbol is provided specially */
बाह्य ध्रुव वर्ण system_map[] __attribute__((weak));

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
ध्रुव वर्ण *प्रतीक_lookup(unsigned long addr, unsigned long *caddr)
{
	ध्रुव वर्ण *sym, *csym;
	वर्ण *esym;
	unsigned long sym_addr;

	sym = system_map;
	csym = रिक्त;
	*caddr = 0;

	यावत् (*sym) {
		sym_addr = hextoul(sym, &esym);
		sym = esym;
		यदि (sym_addr > addr)
			भङ्ग;
		*caddr = sym_addr;
		csym = sym;
		sym += strlen(sym) + 1;
	}

	प्रत्यागम csym;
}
