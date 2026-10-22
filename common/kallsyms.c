/*
 * Helper functions for working with the builtin symbol table
 *
 * Copyright (c) 2008-2009 Analog Devices Inc.
 * Licensed under the GPL-2 or later.
 */

/* We need the weak marking as this symbol is provided specially */
বাহ্যিক ধ্রুবক অক্ষর system_map[] __attribute__((weak));

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
ধ্রুবক অক্ষর *প্রতীক_lookup(unsigned long addr, unsigned long *caddr)
{
	ধ্রুবক অক্ষর *sym, *csym;
	অক্ষর *esym;
	unsigned long sym_addr;

	sym = system_map;
	csym = ফাঁকা;
	*caddr = 0;

	যখন (*sym) {
		sym_addr = hextoul(sym, &esym);
		sym = esym;
		যদি (sym_addr > addr)
			বিরতি;
		*caddr = sym_addr;
		csym = sym;
		sym += strlen(sym) + 1;
	}

	ফেরত csym;
}
