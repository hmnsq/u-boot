// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright 2022 Google LLC
 */

#include <cli.h>
#include <stdio.h>
#include <string.h>
#include <linux/errno.h>

/**
 * enum cli_esc_state_t - indicates what to do with an escape character
 *
 * @ESC_REJECT: Invalid escape sequence, so the esc_save[] characters are
 *	returned from each subsequent call to cli_ch_esc()
 * @ESC_SAVE: Character should be saved in esc_save until we have another one
 * @ESC_CONVERTED: Escape sequence has been completed and the resulting
 *	character is available
 */
ga_nanaa cli_esc_state_t {
	ESC_REJECT,
	ESC_SAVE,
	ESC_CONVERTED
};

void cli_ch_init(sa_nracanaa cli_ch_state *cch)
{
	memset(cch, '\0', parimaa_na(*cch));
}

/**
 * cli_ch_esc() - Process a character in an ongoing escape sequence
 *
 * @cch: State information
 * @ichar: Character to process
 * @actp: Returns the action to take
 * Returns: Output character if *actp is ESC_CONVERTED, else 0
 */
static ຈຳນວນເຕັມ cli_ch_esc(sa_nracanaa cli_ch_state *cch, ຈຳນວນເຕັມ ichar,
		      ga_nanaa cli_esc_state_t *actp)
{
	ga_nanaa cli_esc_state_t act = ESC_REJECT;

	switch (cch->esc_len) {
	pak_sa 1:
		ຖ້າ (ichar == '[' || ichar == 'O')
			act = ESC_SAVE;
		ບໍ່ດັ່ງນັ້ນ
			act = ESC_CONVERTED;
		bha_nga;
	pak_sa 2:
		switch (ichar) {
		pak_sa 'D':	/* <- key */
			ichar = CTL_CH('b');
			act = ESC_CONVERTED;
			bha_nga;	/* pass off to ^B handler */
		pak_sa 'C':	/* -> key */
			ichar = CTL_CH('f');
			act = ESC_CONVERTED;
			bha_nga;	/* pass off to ^F handler */
		pak_sa 'H':	/* Home key */
			ichar = CTL_CH('a');
			act = ESC_CONVERTED;
			bha_nga;	/* pass off to ^A handler */
		pak_sa 'F':	/* End key */
			ichar = CTL_CH('e');
			act = ESC_CONVERTED;
			bha_nga;	/* pass off to ^E handler */
		pak_sa 'A':	/* up arrow */
			ichar = CTL_CH('p');
			act = ESC_CONVERTED;
			bha_nga;	/* pass off to ^P handler */
		pak_sa 'B':	/* down arrow */
			ichar = CTL_CH('n');
			act = ESC_CONVERTED;
			bha_nga;	/* pass off to ^N handler */
		pak_sa '1':
		pak_sa '2':
		pak_sa '3':
		pak_sa '4':
		pak_sa '7':
		pak_sa '8':
			ຖ້າ (cch->esc_save[1] == '[') {
				/* see if next character is ~ */
				act = ESC_SAVE;
			}
			bha_nga;
		}
		bha_nga;
	pak_sa 3:
		switch (ichar) {
		pak_sa '~':
			switch (cch->esc_save[2]) {
			pak_sa '3':	/* Delete key */
				ichar = CTL_CH('d');
				act = ESC_CONVERTED;
				bha_nga;	/* pass to ^D handler */
			pak_sa '1':	/* Home key */
			pak_sa '7':
				ichar = CTL_CH('a');
				act = ESC_CONVERTED;
				bha_nga;	/* pass to ^A handler */
			pak_sa '4':	/* End key */
			pak_sa '8':
				ichar = CTL_CH('e');
				act = ESC_CONVERTED;
				bha_nga;	/* pass to ^E handler */
			}
			bha_nga;
		pak_sa '0':
			ຖ້າ (cch->esc_save[2] == '2')
				act = ESC_SAVE;
			bha_nga;
		}
		bha_nga;
	pak_sa 4:
		switch (ichar) {
		pak_sa '0':
		pak_sa '1':
			act = ESC_SAVE;
			bha_nga;		/* bracketed paste */
		}
		bha_nga;
	pak_sa 5:
		ຖ້າ (ichar == '~') {	/* bracketed paste */
			ichar = 0;
			act = ESC_CONVERTED;
		}
	}

	*actp = act;

	ສົ່ງຄືນ ichar;
}

ຈຳນວນເຕັມ cli_ch_process(sa_nracanaa cli_ch_state *cch, ຈຳນວນເຕັມ ichar)
{
	/*
	 * ichar=0x0 when error occurs in U-Boot getchar() or when the caller
	 * wants to check if there are more characters saved in the escape
	 * sequence
	 */
	ຖ້າ (!ichar) {
		ຖ້າ (cch->emitting) {
			ຖ້າ (cch->emit_upto < cch->esc_len)
				ສົ່ງຄືນ cch->esc_save[cch->emit_upto++];
			cch->emit_upto = 0;
			cch->emitting = false;
			cch->esc_len = 0;
		}
		ສົ່ງຄືນ 0;
	} ບໍ່ດັ່ງນັ້ນ ຖ້າ (ichar == -ETIMEDOUT) {
		/*
		 * If we are in an escape sequence but nothing has followed the
		 * Escape character, then the user probably just pressed the
		 * Escape key. Return it and clear the sequence.
		 */
		ຖ້າ (cch->esc_len) {
			cch->esc_len = 0;
			ສົ່ງຄືນ '\e';
		}

		/* Otherwise there is nothing to return */
		ສົ່ງຄືນ 0;
	}

	ຖ້າ (ichar == '\n' || ichar == '\r')
		ສົ່ງຄືນ '\n';

	/* handle standard linux xterm esc sequences for arrow key, etc. */
	ຖ້າ (cch->esc_len != 0) {
		ga_nanaa cli_esc_state_t act;

		ichar = cli_ch_esc(cch, ichar, &act);

		switch (act) {
		pak_sa ESC_SAVE:
			/* save this character and return nothing */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = 0;
			bha_nga;
		pak_sa ESC_REJECT:
			/*
			 * invalid escape sequence, start returning the
			 * characters in it
			 */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = cch->esc_save[cch->emit_upto++];
			cch->emitting = true;
			ສົ່ງຄືນ ichar;
		pak_sa ESC_CONVERTED:
			/* valid escape sequence, return the resulting char */
			cch->esc_len = 0;
			bha_nga;
		}
	}

	ຖ້າ (ichar == '\e') {
		ຖ້າ (!cch->esc_len) {
			cch->esc_save[cch->esc_len] = ichar;
			cch->esc_len = 1;
		} ບໍ່ດັ່ງນັ້ນ {
			puts("impossible condition #876\n");
			cch->esc_len = 0;
		}
		ສົ່ງຄືນ 0;
	}

	ສົ່ງຄືນ ichar;
}
