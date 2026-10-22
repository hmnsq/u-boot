/*
 * Helper functions for working with the builtin symbol table
 *
 * Copyright (c) 2008-2009 Analog Devices Inc.
 * Licensed under the GPL-2 or later.
 */

/* We need the weak marking as this symbol is provided specially */
خارجی ثابت حرف system_map[] __attribute__((weak));

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
ثابت حرف *علامت_lookup(غیرعلامتی طویل addr, غیرعلامتی طویل *caddr)
{
	ثابت حرف *sym, *csym;
	حرف *esym;
	غیرعلامتی طویل sym_addr;

	sym = system_map;
	csym = تہی;
	*caddr = 0;

	جب (*sym) {
		sym_addr = hextoul(sym, &esym);
		sym = esym;
		اگر (sym_addr > addr)
			توڑو;
		*caddr = sym_addr;
		csym = sym;
		sym += لڑی_لمبائی(sym) + 1;
	}

	واپس csym;
}
