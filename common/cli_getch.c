// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright 2022 Google LLC
 */

#അന്തര്ഭാവ <cli.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <linux/errno.h>

/**
 * enum cli_esc_state_t - indicates what to do with an escape character
 *
 * @ESC_REJECT: Invalid escape sequence, so the esc_save[] characters are
 *	returned from each subsequent call to cli_ch_esc()
 * @ESC_SAVE: Character should be saved in esc_save until we have another one
 * @ESC_CONVERTED: Escape sequence has been completed and the resulting
 *	character is available
 */
എണ്ണൽ cli_esc_state_t {
	ESC_REJECT,
	ESC_SAVE,
	ESC_CONVERTED
};

ശൂന്യം cli_ch_init(ഘടന cli_ch_state *cch)
{
	memset(cch, '\0', വലുപ്പം(*cch));
}

/**
 * cli_ch_esc() - Process a character in an ongoing escape sequence
 *
 * @cch: State information
 * @ichar: Character to process
 * @actp: Returns the action to take
 * Returns: Output character if *actp is ESC_CONVERTED, else 0
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ cli_ch_esc(ഘടന cli_ch_state *cch, പൂർണ്ണസംഖ്യ ichar,
		      എണ്ണൽ cli_esc_state_t *actp)
{
	എണ്ണൽ cli_esc_state_t act = ESC_REJECT;

	തിരഞ്ഞെടുപ്പ് (cch->esc_len) {
	അവസ്ഥ 1:
		എങ്കിൽ (ichar == '[' || ichar == 'O')
			act = ESC_SAVE;
		അല്ലെങ്കിൽ
			act = ESC_CONVERTED;
		നിർത്തുക;
	അവസ്ഥ 2:
		തിരഞ്ഞെടുപ്പ് (ichar) {
		അവസ്ഥ 'D':	/* <- key */
			ichar = CTL_CH('b');
			act = ESC_CONVERTED;
			നിർത്തുക;	/* pass off to ^B handler */
		അവസ്ഥ 'C':	/* -> key */
			ichar = CTL_CH('f');
			act = ESC_CONVERTED;
			നിർത്തുക;	/* pass off to ^F handler */
		അവസ്ഥ 'H':	/* Home key */
			ichar = CTL_CH('a');
			act = ESC_CONVERTED;
			നിർത്തുക;	/* pass off to ^A handler */
		അവസ്ഥ 'F':	/* End key */
			ichar = CTL_CH('e');
			act = ESC_CONVERTED;
			നിർത്തുക;	/* pass off to ^E handler */
		അവസ്ഥ 'A':	/* up arrow */
			ichar = CTL_CH('p');
			act = ESC_CONVERTED;
			നിർത്തുക;	/* pass off to ^P handler */
		അവസ്ഥ 'B':	/* down arrow */
			ichar = CTL_CH('n');
			act = ESC_CONVERTED;
			നിർത്തുക;	/* pass off to ^N handler */
		അവസ്ഥ '1':
		അവസ്ഥ '2':
		അവസ്ഥ '3':
		അവസ്ഥ '4':
		അവസ്ഥ '7':
		അവസ്ഥ '8':
			എങ്കിൽ (cch->esc_save[1] == '[') {
				/* see if next character is ~ */
				act = ESC_SAVE;
			}
			നിർത്തുക;
		}
		നിർത്തുക;
	അവസ്ഥ 3:
		തിരഞ്ഞെടുപ്പ് (ichar) {
		അവസ്ഥ '~':
			തിരഞ്ഞെടുപ്പ് (cch->esc_save[2]) {
			അവസ്ഥ '3':	/* Delete key */
				ichar = CTL_CH('d');
				act = ESC_CONVERTED;
				നിർത്തുക;	/* pass to ^D handler */
			അവസ്ഥ '1':	/* Home key */
			അവസ്ഥ '7':
				ichar = CTL_CH('a');
				act = ESC_CONVERTED;
				നിർത്തുക;	/* pass to ^A handler */
			അവസ്ഥ '4':	/* End key */
			അവസ്ഥ '8':
				ichar = CTL_CH('e');
				act = ESC_CONVERTED;
				നിർത്തുക;	/* pass to ^E handler */
			}
			നിർത്തുക;
		അവസ്ഥ '0':
			എങ്കിൽ (cch->esc_save[2] == '2')
				act = ESC_SAVE;
			നിർത്തുക;
		}
		നിർത്തുക;
	അവസ്ഥ 4:
		തിരഞ്ഞെടുപ്പ് (ichar) {
		അവസ്ഥ '0':
		അവസ്ഥ '1':
			act = ESC_SAVE;
			നിർത്തുക;		/* bracketed paste */
		}
		നിർത്തുക;
	അവസ്ഥ 5:
		എങ്കിൽ (ichar == '~') {	/* bracketed paste */
			ichar = 0;
			act = ESC_CONVERTED;
		}
	}

	*actp = act;

	തിരികെ ichar;
}

പൂർണ്ണസംഖ്യ cli_ch_process(ഘടന cli_ch_state *cch, പൂർണ്ണസംഖ്യ ichar)
{
	/*
	 * ichar=0x0 when error occurs in U-Boot getchar() or when the caller
	 * wants to check if there are more characters saved in the escape
	 * sequence
	 */
	എങ്കിൽ (!ichar) {
		എങ്കിൽ (cch->emitting) {
			എങ്കിൽ (cch->emit_upto < cch->esc_len)
				തിരികെ cch->esc_save[cch->emit_upto++];
			cch->emit_upto = 0;
			cch->emitting = false;
			cch->esc_len = 0;
		}
		തിരികെ 0;
	} അല്ലെങ്കിൽ എങ്കിൽ (ichar == -ETIMEDOUT) {
		/*
		 * If we are in an escape sequence but nothing has followed the
		 * Escape character, then the user probably just pressed the
		 * Escape key. Return it and clear the sequence.
		 */
		എങ്കിൽ (cch->esc_len) {
			cch->esc_len = 0;
			തിരികെ '\e';
		}

		/* Otherwise there is nothing to return */
		തിരികെ 0;
	}

	എങ്കിൽ (ichar == '\n' || ichar == '\r')
		തിരികെ '\n';

	/* handle standard linux xterm esc sequences for arrow key, etc. */
	എങ്കിൽ (cch->esc_len != 0) {
		എണ്ണൽ cli_esc_state_t act;

		ichar = cli_ch_esc(cch, ichar, &act);

		തിരഞ്ഞെടുപ്പ് (act) {
		അവസ്ഥ ESC_SAVE:
			/* save this character and return nothing */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = 0;
			നിർത്തുക;
		അവസ്ഥ ESC_REJECT:
			/*
			 * invalid escape sequence, start returning the
			 * characters in it
			 */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = cch->esc_save[cch->emit_upto++];
			cch->emitting = true;
			തിരികെ ichar;
		അവസ്ഥ ESC_CONVERTED:
			/* valid escape sequence, return the resulting char */
			cch->esc_len = 0;
			നിർത്തുക;
		}
	}

	എങ്കിൽ (ichar == '\e') {
		എങ്കിൽ (!cch->esc_len) {
			cch->esc_save[cch->esc_len] = ichar;
			cch->esc_len = 1;
		} അല്ലെങ്കിൽ {
			puts("impossible condition #876\n");
			cch->esc_len = 0;
		}
		തിരികെ 0;
	}

	തിരികെ ichar;
}
