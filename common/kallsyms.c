/*
 * Helper functions for working with the builtin symbol table
 *
 * Copyright (c) 2008-2009 Analog Devices Inc.
 * Licensed under the GPL-2 or later.
 */

/* We need the weak marking as this symbol is provided specially */
बाह्य स्थिरांक अक्षर system_map[] __attribute__((weak));

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
स्थिरांक अक्षर *चिन्ह_lookup(अचिन्हित दीर्घ addr, अचिन्हित दीर्घ *caddr)
{
	स्थिरांक अक्षर *sym, *csym;
	अक्षर *esym;
	अचिन्हित दीर्घ sym_addr;

	sym = system_map;
	csym = रिक्त;
	*caddr = 0;

	पर्यंत (*sym) {
		sym_addr = hextoul(sym, &esym);
		sym = esym;
		जर (sym_addr > addr)
			थांबा;
		*caddr = sym_addr;
		csym = sym;
		sym += म_माप(sym) + 1;
	}

	परत csym;
}
