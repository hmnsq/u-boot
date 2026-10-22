/*
 * Helper functions for working with the builtin symbol table
 *
 * Copyright (c) 2008-2009 Analog Devices Inc.
 * Licensed under the GPL-2 or later.
 */

/* We need the weak marking as this symbol is provided specially */
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ system_map[] __attribute__((weak));

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
ਅਚਲ ਅੱਖਰ *ਚਿੰਨ੍ਹ_lookup(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ addr, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *caddr)
{
	ਅਚਲ ਅੱਖਰ *sym, *csym;
	ਅੱਖਰ *esym;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ sym_addr;

	sym = system_map;
	csym = ਨਿਰਾ;
	*caddr = 0;

	ਜਦੋਂ (*sym) {
		sym_addr = hextoul(sym, &esym);
		sym = esym;
		ਜੇ (sym_addr > addr)
			ਰੋਕੋ;
		*caddr = sym_addr;
		csym = sym;
		sym += ਲੜੀ_ਲੰਬਾਈ(sym) + 1;
	}

	ਵਾਪਸ csym;
}
