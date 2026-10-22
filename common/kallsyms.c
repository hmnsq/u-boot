/*
 * Helper functions for working with the builtin symbol table
 *
 * Copyright (c) 2008-2009 Analog Devices Inc.
 * Licensed under the GPL-2 or later.
 */

/* We need the weak marking as this symbol is provided specially */
ബാഹ്യ സ്ഥിരം അക്ഷരം system_map[] __attribute__((weak));

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
സ്ഥിരം അക്ഷരം *ചിഹ്നം_lookup(unsigned long addr, unsigned long *caddr)
{
	സ്ഥിരം അക്ഷരം *sym, *csym;
	അക്ഷരം *esym;
	unsigned long sym_addr;

	sym = system_map;
	csym = ശൂന്യ;
	*caddr = 0;

	ആകുമ്പോൾ (*sym) {
		sym_addr = hextoul(sym, &esym);
		sym = esym;
		എങ്കിൽ (sym_addr > addr)
			നിർത്തുക;
		*caddr = sym_addr;
		csym = sym;
		sym += strlen(sym) + 1;
	}

	തിരികെ csym;
}
