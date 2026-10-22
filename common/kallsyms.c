/*
 * Helper functions for working with the builtin symbol table
 *
 * Copyright (c) 2008-2009 Analog Devices Inc.
 * Licensed under the GPL-2 or later.
 */

/* We need the weak marking as this symbol is provided specially */
ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର system_map[] __attribute__((weak));

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
ଅଚଳ ଅକ୍ଷର *ପ୍ରତୀକ_lookup(unsigned long addr, unsigned long *caddr)
{
	ଅଚଳ ଅକ୍ଷର *sym, *csym;
	ଅକ୍ଷର *esym;
	unsigned long sym_addr;

	sym = system_map;
	csym = ଖାଲି;
	*caddr = 0;

	ଯେତେବେଳେ (*sym) {
		sym_addr = hextoul(sym, &esym);
		sym = esym;
		ଯଦି (sym_addr > addr)
			ଭାଙ୍ଗ;
		*caddr = sym_addr;
		csym = sym;
		sym += strlen(sym) + 1;
	}

	ଫେରନ୍ତୁ csym;
}
