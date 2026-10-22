// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright 2022 Google LLC
 */

#அந்தர்பாவ <cli.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <linux/errno.h>

/**
 * enum cli_esc_state_t - indicates what to do with an escape character
 *
 * @ESC_REJECT: Invalid escape sequence, so the esc_save[] characters are
 *	returned from each subsequent call to cli_ch_esc()
 * @ESC_SAVE: Character should be saved in esc_save until we have another one
 * @ESC_CONVERTED: Escape sequence has been completed and the resulting
 *	character is available
 */
எண்ணிக்கை cli_esc_state_t {
	ESC_REJECT,
	ESC_SAVE,
	ESC_CONVERTED
};

வெறுமை cli_ch_init(கட்டமைப்பு cli_ch_state *cch)
{
	memset(cch, '\0', அளவு(*cch));
}

/**
 * cli_ch_esc() - Process a character in an ongoing escape sequence
 *
 * @cch: State information
 * @ichar: Character to process
 * @actp: Returns the action to take
 * Returns: Output character if *actp is ESC_CONVERTED, else 0
 */
நிலையான முழு cli_ch_esc(கட்டமைப்பு cli_ch_state *cch, முழு ichar,
		      எண்ணிக்கை cli_esc_state_t *actp)
{
	எண்ணிக்கை cli_esc_state_t act = ESC_REJECT;

	தேர்வு (cch->esc_len) {
	நிலை 1:
		எனில் (ichar == '[' || ichar == 'O')
			act = ESC_SAVE;
		இல்லையெனில்
			act = ESC_CONVERTED;
		நிறுத்து;
	நிலை 2:
		தேர்வு (ichar) {
		நிலை 'D':	/* <- key */
			ichar = CTL_CH('b');
			act = ESC_CONVERTED;
			நிறுத்து;	/* pass off to ^B handler */
		நிலை 'C':	/* -> key */
			ichar = CTL_CH('f');
			act = ESC_CONVERTED;
			நிறுத்து;	/* pass off to ^F handler */
		நிலை 'H':	/* Home key */
			ichar = CTL_CH('a');
			act = ESC_CONVERTED;
			நிறுத்து;	/* pass off to ^A handler */
		நிலை 'F':	/* End key */
			ichar = CTL_CH('e');
			act = ESC_CONVERTED;
			நிறுத்து;	/* pass off to ^E handler */
		நிலை 'A':	/* up arrow */
			ichar = CTL_CH('p');
			act = ESC_CONVERTED;
			நிறுத்து;	/* pass off to ^P handler */
		நிலை 'B':	/* down arrow */
			ichar = CTL_CH('n');
			act = ESC_CONVERTED;
			நிறுத்து;	/* pass off to ^N handler */
		நிலை '1':
		நிலை '2':
		நிலை '3':
		நிலை '4':
		நிலை '7':
		நிலை '8':
			எனில் (cch->esc_save[1] == '[') {
				/* see if next character is ~ */
				act = ESC_SAVE;
			}
			நிறுத்து;
		}
		நிறுத்து;
	நிலை 3:
		தேர்வு (ichar) {
		நிலை '~':
			தேர்வு (cch->esc_save[2]) {
			நிலை '3':	/* Delete key */
				ichar = CTL_CH('d');
				act = ESC_CONVERTED;
				நிறுத்து;	/* pass to ^D handler */
			நிலை '1':	/* Home key */
			நிலை '7':
				ichar = CTL_CH('a');
				act = ESC_CONVERTED;
				நிறுத்து;	/* pass to ^A handler */
			நிலை '4':	/* End key */
			நிலை '8':
				ichar = CTL_CH('e');
				act = ESC_CONVERTED;
				நிறுத்து;	/* pass to ^E handler */
			}
			நிறுத்து;
		நிலை '0':
			எனில் (cch->esc_save[2] == '2')
				act = ESC_SAVE;
			நிறுத்து;
		}
		நிறுத்து;
	நிலை 4:
		தேர்வு (ichar) {
		நிலை '0':
		நிலை '1':
			act = ESC_SAVE;
			நிறுத்து;		/* bracketed paste */
		}
		நிறுத்து;
	நிலை 5:
		எனில் (ichar == '~') {	/* bracketed paste */
			ichar = 0;
			act = ESC_CONVERTED;
		}
	}

	*actp = act;

	திரும்பு ichar;
}

முழு cli_ch_process(கட்டமைப்பு cli_ch_state *cch, முழு ichar)
{
	/*
	 * ichar=0x0 when error occurs in U-Boot getchar() or when the caller
	 * wants to check if there are more characters saved in the escape
	 * sequence
	 */
	எனில் (!ichar) {
		எனில் (cch->emitting) {
			எனில் (cch->emit_upto < cch->esc_len)
				திரும்பு cch->esc_save[cch->emit_upto++];
			cch->emit_upto = 0;
			cch->emitting = false;
			cch->esc_len = 0;
		}
		திரும்பு 0;
	} இல்லையெனில் எனில் (ichar == -ETIMEDOUT) {
		/*
		 * If we are in an escape sequence but nothing has followed the
		 * Escape character, then the user probably just pressed the
		 * Escape key. Return it and clear the sequence.
		 */
		எனில் (cch->esc_len) {
			cch->esc_len = 0;
			திரும்பு '\e';
		}

		/* Otherwise there is nothing to return */
		திரும்பு 0;
	}

	எனில் (ichar == '\n' || ichar == '\r')
		திரும்பு '\n';

	/* handle standard linux xterm esc sequences for arrow key, etc. */
	எனில் (cch->esc_len != 0) {
		எண்ணிக்கை cli_esc_state_t act;

		ichar = cli_ch_esc(cch, ichar, &act);

		தேர்வு (act) {
		நிலை ESC_SAVE:
			/* save this character and return nothing */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = 0;
			நிறுத்து;
		நிலை ESC_REJECT:
			/*
			 * invalid escape sequence, start returning the
			 * characters in it
			 */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = cch->esc_save[cch->emit_upto++];
			cch->emitting = true;
			திரும்பு ichar;
		நிலை ESC_CONVERTED:
			/* valid escape sequence, return the resulting char */
			cch->esc_len = 0;
			நிறுத்து;
		}
	}

	எனில் (ichar == '\e') {
		எனில் (!cch->esc_len) {
			cch->esc_save[cch->esc_len] = ichar;
			cch->esc_len = 1;
		} இல்லையெனில் {
			puts("impossible condition #876\n");
			cch->esc_len = 0;
		}
		திரும்பு 0;
	}

	திரும்பு ichar;
}
