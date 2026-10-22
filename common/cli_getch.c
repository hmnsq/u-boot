// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright 2022 Google LLC
 */

#अन्तर्भाव <cli.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <linux/errno.h>

/**
 * enum cli_esc_state_t - indicates what to do with an escape character
 *
 * @ESC_REJECT: Invalid escape sequence, so the esc_save[] characters are
 *	returned from each subsequent call to cli_ch_esc()
 * @ESC_SAVE: Character should be saved in esc_save until we have another one
 * @ESC_CONVERTED: Escape sequence has been completed and the resulting
 *	character is available
 */
गणना cli_esc_state_t {
	ESC_REJECT,
	ESC_SAVE,
	ESC_CONVERTED
};

शून्य cli_ch_init(संरचना cli_ch_state *cch)
{
	स_रखो(cch, '\0', परिमाण(*cch));
}

/**
 * cli_ch_esc() - Process a character in an ongoing escape sequence
 *
 * @cch: State information
 * @ichar: Character to process
 * @actp: Returns the action to take
 * Returns: Output character if *actp is ESC_CONVERTED, else 0
 */
स्थिर पूर्णाङ्क cli_ch_esc(संरचना cli_ch_state *cch, पूर्णाङ्क ichar,
		      गणना cli_esc_state_t *actp)
{
	गणना cli_esc_state_t act = ESC_REJECT;

	विकल्प (cch->esc_len) {
	पक्ष 1:
		यदि (ichar == '[' || ichar == 'O')
			act = ESC_SAVE;
		अन्यथा
			act = ESC_CONVERTED;
		रोक;
	पक्ष 2:
		विकल्प (ichar) {
		पक्ष 'D':	/* <- key */
			ichar = CTL_CH('b');
			act = ESC_CONVERTED;
			रोक;	/* pass off to ^B handler */
		पक्ष 'C':	/* -> key */
			ichar = CTL_CH('f');
			act = ESC_CONVERTED;
			रोक;	/* pass off to ^F handler */
		पक्ष 'H':	/* Home key */
			ichar = CTL_CH('a');
			act = ESC_CONVERTED;
			रोक;	/* pass off to ^A handler */
		पक्ष 'F':	/* End key */
			ichar = CTL_CH('e');
			act = ESC_CONVERTED;
			रोक;	/* pass off to ^E handler */
		पक्ष 'A':	/* up arrow */
			ichar = CTL_CH('p');
			act = ESC_CONVERTED;
			रोक;	/* pass off to ^P handler */
		पक्ष 'B':	/* down arrow */
			ichar = CTL_CH('n');
			act = ESC_CONVERTED;
			रोक;	/* pass off to ^N handler */
		पक्ष '1':
		पक्ष '2':
		पक्ष '3':
		पक्ष '4':
		पक्ष '7':
		पक्ष '8':
			यदि (cch->esc_save[1] == '[') {
				/* see if next character is ~ */
				act = ESC_SAVE;
			}
			रोक;
		}
		रोक;
	पक्ष 3:
		विकल्प (ichar) {
		पक्ष '~':
			विकल्प (cch->esc_save[2]) {
			पक्ष '3':	/* Delete key */
				ichar = CTL_CH('d');
				act = ESC_CONVERTED;
				रोक;	/* pass to ^D handler */
			पक्ष '1':	/* Home key */
			पक्ष '7':
				ichar = CTL_CH('a');
				act = ESC_CONVERTED;
				रोक;	/* pass to ^A handler */
			पक्ष '4':	/* End key */
			पक्ष '8':
				ichar = CTL_CH('e');
				act = ESC_CONVERTED;
				रोक;	/* pass to ^E handler */
			}
			रोक;
		पक्ष '0':
			यदि (cch->esc_save[2] == '2')
				act = ESC_SAVE;
			रोक;
		}
		रोक;
	पक्ष 4:
		विकल्प (ichar) {
		पक्ष '0':
		पक्ष '1':
			act = ESC_SAVE;
			रोक;		/* bracketed paste */
		}
		रोक;
	पक्ष 5:
		यदि (ichar == '~') {	/* bracketed paste */
			ichar = 0;
			act = ESC_CONVERTED;
		}
	}

	*actp = act;

	फर्काऊ ichar;
}

पूर्णाङ्क cli_ch_process(संरचना cli_ch_state *cch, पूर्णाङ्क ichar)
{
	/*
	 * ichar=0x0 when error occurs in U-Boot getchar() or when the caller
	 * wants to check if there are more characters saved in the escape
	 * sequence
	 */
	यदि (!ichar) {
		यदि (cch->emitting) {
			यदि (cch->emit_upto < cch->esc_len)
				फर्काऊ cch->esc_save[cch->emit_upto++];
			cch->emit_upto = 0;
			cch->emitting = false;
			cch->esc_len = 0;
		}
		फर्काऊ 0;
	} अन्यथा यदि (ichar == -ETIMEDOUT) {
		/*
		 * If we are in an escape sequence but nothing has followed the
		 * Escape character, then the user probably just pressed the
		 * Escape key. Return it and clear the sequence.
		 */
		यदि (cch->esc_len) {
			cch->esc_len = 0;
			फर्काऊ '\e';
		}

		/* Otherwise there is nothing to return */
		फर्काऊ 0;
	}

	यदि (ichar == '\n' || ichar == '\r')
		फर्काऊ '\n';

	/* handle standard linux xterm esc sequences for arrow key, etc. */
	यदि (cch->esc_len != 0) {
		गणना cli_esc_state_t act;

		ichar = cli_ch_esc(cch, ichar, &act);

		विकल्प (act) {
		पक्ष ESC_SAVE:
			/* save this character and return nothing */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = 0;
			रोक;
		पक्ष ESC_REJECT:
			/*
			 * invalid escape sequence, start returning the
			 * characters in it
			 */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = cch->esc_save[cch->emit_upto++];
			cch->emitting = true;
			फर्काऊ ichar;
		पक्ष ESC_CONVERTED:
			/* valid escape sequence, return the resulting char */
			cch->esc_len = 0;
			रोक;
		}
	}

	यदि (ichar == '\e') {
		यदि (!cch->esc_len) {
			cch->esc_save[cch->esc_len] = ichar;
			cch->esc_len = 1;
		} अन्यथा {
			माला_दो("impossible condition #876\n");
			cch->esc_len = 0;
		}
		फर्काऊ 0;
	}

	फर्काऊ ichar;
}
