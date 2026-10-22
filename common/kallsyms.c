/*
 * Helper functions for working with the builtin symbol table
 *
 * Copyright (c) 2008-2009 Analog Devices Inc.
 * Licensed under the GPL-2 or later.
 */

/* We need the weak marking as this symbol is provided specially */
బాహ్య స్థిరరాశి అక్షరం system_map[] __attribute__((weak));

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
స్థిరరాశి అక్షరం *చిహ్నం_lookup(గుర్తులేని దీర్ఘ addr, గుర్తులేని దీర్ఘ *caddr)
{
	స్థిరరాశి అక్షరం *sym, *csym;
	అక్షరం *esym;
	గుర్తులేని దీర్ఘ sym_addr;

	sym = system_map;
	csym = ఖాళీ;
	*caddr = 0;

	అయితే (*sym) {
		sym_addr = hextoul(sym, &esym);
		sym = esym;
		ఒకవేళ (sym_addr > addr)
			ఆపు;
		*caddr = sym_addr;
		csym = sym;
		sym += అక్షరశ్రేణి_పొడవు(sym) + 1;
	}

	తిరిగి csym;
}
