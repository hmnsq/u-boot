/*
 * Helper functions for working with the builtin symbol table
 *
 * Copyright (c) 2008-2009 Analog Devices Inc.
 * Licensed under the GPL-2 or later.
 */

/* We need the weak marking as this symbol is provided specially */
خارجي ثابت حرف system_map[] __attribute__((weak));

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
ثابت حرف *symbol_lookup(unsigned long addr, unsigned long *caddr)
{
	ثابت حرف *sym, *csym;
	حرف *esym;
	unsigned long sym_addr;

	sym = system_map;
	csym = عدم;
	*caddr = 0;

	طالما (*sym) {
		sym_addr = hextoul(sym, &esym);
		sym = esym;
		𐤀𐤌 (sym_addr > addr)
			اكسر;
		*caddr = sym_addr;
		csym = sym;
		sym += strlen(sym) + 1;
	}

	أرجع csym;
}
