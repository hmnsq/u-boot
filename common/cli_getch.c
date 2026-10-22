// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright 2022 Google LLC
 */

#ಅನ್ತರ್ಭಾವ <cli.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <linux/errno.h>

/**
 * enum cli_esc_state_t - indicates what to do with an escape character
 *
 * @ESC_REJECT: Invalid escape sequence, so the esc_save[] characters are
 *	returned from each subsequent call to cli_ch_esc()
 * @ESC_SAVE: Character should be saved in esc_save until we have another one
 * @ESC_CONVERTED: Escape sequence has been completed and the resulting
 *	character is available
 */
ಎಣಿಕೆ cli_esc_state_t {
	ESC_REJECT,
	ESC_SAVE,
	ESC_CONVERTED
};

ಶೂನ್ಯ cli_ch_init(ರಚನೆ cli_ch_state *cch)
{
	memset(cch, '\0', ಗಾತ್ರ(*cch));
}

/**
 * cli_ch_esc() - Process a character in an ongoing escape sequence
 *
 * @cch: State information
 * @ichar: Character to process
 * @actp: Returns the action to take
 * Returns: Output character if *actp is ESC_CONVERTED, else 0
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cli_ch_esc(ರಚನೆ cli_ch_state *cch, ಪೂರ್ಣಾಂಕ ichar,
		      ಎಣಿಕೆ cli_esc_state_t *actp)
{
	ಎಣಿಕೆ cli_esc_state_t act = ESC_REJECT;

	ಆಯ್ಕೆ (cch->esc_len) {
	ಸ್ಥಿತಿ 1:
		ಆದರೆ (ichar == '[' || ichar == 'O')
			act = ESC_SAVE;
		ಇಲ್ಲದಿದ್ದರೆ
			act = ESC_CONVERTED;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 2:
		ಆಯ್ಕೆ (ichar) {
		ಸ್ಥಿತಿ 'D':	/* <- key */
			ichar = CTL_CH('b');
			act = ESC_CONVERTED;
			ನಿಲ್ಲಿಸು;	/* pass off to ^B handler */
		ಸ್ಥಿತಿ 'C':	/* -> key */
			ichar = CTL_CH('f');
			act = ESC_CONVERTED;
			ನಿಲ್ಲಿಸು;	/* pass off to ^F handler */
		ಸ್ಥಿತಿ 'H':	/* Home key */
			ichar = CTL_CH('a');
			act = ESC_CONVERTED;
			ನಿಲ್ಲಿಸು;	/* pass off to ^A handler */
		ಸ್ಥಿತಿ 'F':	/* End key */
			ichar = CTL_CH('e');
			act = ESC_CONVERTED;
			ನಿಲ್ಲಿಸು;	/* pass off to ^E handler */
		ಸ್ಥಿತಿ 'A':	/* up arrow */
			ichar = CTL_CH('p');
			act = ESC_CONVERTED;
			ನಿಲ್ಲಿಸು;	/* pass off to ^P handler */
		ಸ್ಥಿತಿ 'B':	/* down arrow */
			ichar = CTL_CH('n');
			act = ESC_CONVERTED;
			ನಿಲ್ಲಿಸು;	/* pass off to ^N handler */
		ಸ್ಥಿತಿ '1':
		ಸ್ಥಿತಿ '2':
		ಸ್ಥಿತಿ '3':
		ಸ್ಥಿತಿ '4':
		ಸ್ಥಿತಿ '7':
		ಸ್ಥಿತಿ '8':
			ಆದರೆ (cch->esc_save[1] == '[') {
				/* see if next character is ~ */
				act = ESC_SAVE;
			}
			ನಿಲ್ಲಿಸು;
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 3:
		ಆಯ್ಕೆ (ichar) {
		ಸ್ಥಿತಿ '~':
			ಆಯ್ಕೆ (cch->esc_save[2]) {
			ಸ್ಥಿತಿ '3':	/* Delete key */
				ichar = CTL_CH('d');
				act = ESC_CONVERTED;
				ನಿಲ್ಲಿಸು;	/* pass to ^D handler */
			ಸ್ಥಿತಿ '1':	/* Home key */
			ಸ್ಥಿತಿ '7':
				ichar = CTL_CH('a');
				act = ESC_CONVERTED;
				ನಿಲ್ಲಿಸು;	/* pass to ^A handler */
			ಸ್ಥಿತಿ '4':	/* End key */
			ಸ್ಥಿತಿ '8':
				ichar = CTL_CH('e');
				act = ESC_CONVERTED;
				ನಿಲ್ಲಿಸು;	/* pass to ^E handler */
			}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '0':
			ಆದರೆ (cch->esc_save[2] == '2')
				act = ESC_SAVE;
			ನಿಲ್ಲಿಸು;
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 4:
		ಆಯ್ಕೆ (ichar) {
		ಸ್ಥಿತಿ '0':
		ಸ್ಥಿತಿ '1':
			act = ESC_SAVE;
			ನಿಲ್ಲಿಸು;		/* bracketed paste */
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 5:
		ಆದರೆ (ichar == '~') {	/* bracketed paste */
			ichar = 0;
			act = ESC_CONVERTED;
		}
	}

	*actp = act;

	ಹಿಂತಿರುಗಿ ichar;
}

ಪೂರ್ಣಾಂಕ cli_ch_process(ರಚನೆ cli_ch_state *cch, ಪೂರ್ಣಾಂಕ ichar)
{
	/*
	 * ichar=0x0 when error occurs in U-Boot getchar() or when the caller
	 * wants to check if there are more characters saved in the escape
	 * sequence
	 */
	ಆದರೆ (!ichar) {
		ಆದರೆ (cch->emitting) {
			ಆದರೆ (cch->emit_upto < cch->esc_len)
				ಹಿಂತಿರುಗಿ cch->esc_save[cch->emit_upto++];
			cch->emit_upto = 0;
			cch->emitting = false;
			cch->esc_len = 0;
		}
		ಹಿಂತಿರುಗಿ 0;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ichar == -ETIMEDOUT) {
		/*
		 * If we are in an escape sequence but nothing has followed the
		 * Escape character, then the user probably just pressed the
		 * Escape key. Return it and clear the sequence.
		 */
		ಆದರೆ (cch->esc_len) {
			cch->esc_len = 0;
			ಹಿಂತಿರುಗಿ '\e';
		}

		/* Otherwise there is nothing to return */
		ಹಿಂತಿರುಗಿ 0;
	}

	ಆದರೆ (ichar == '\n' || ichar == '\r')
		ಹಿಂತಿರುಗಿ '\n';

	/* handle standard linux xterm esc sequences for arrow key, etc. */
	ಆದರೆ (cch->esc_len != 0) {
		ಎಣಿಕೆ cli_esc_state_t act;

		ichar = cli_ch_esc(cch, ichar, &act);

		ಆಯ್ಕೆ (act) {
		ಸ್ಥಿತಿ ESC_SAVE:
			/* save this character and return nothing */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = 0;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ ESC_REJECT:
			/*
			 * invalid escape sequence, start returning the
			 * characters in it
			 */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = cch->esc_save[cch->emit_upto++];
			cch->emitting = true;
			ಹಿಂತಿರುಗಿ ichar;
		ಸ್ಥಿತಿ ESC_CONVERTED:
			/* valid escape sequence, return the resulting char */
			cch->esc_len = 0;
			ನಿಲ್ಲಿಸು;
		}
	}

	ಆದರೆ (ichar == '\e') {
		ಆದರೆ (!cch->esc_len) {
			cch->esc_save[cch->esc_len] = ichar;
			cch->esc_len = 1;
		} ಇಲ್ಲದಿದ್ದರೆ {
			puts("impossible condition #876\n");
			cch->esc_len = 0;
		}
		ಹಿಂತಿರುಗಿ 0;
	}

	ಹಿಂತಿರುಗಿ ichar;
}
