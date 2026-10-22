// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright 2022 Google LLC
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <cli.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/errno.h>

/**
 * enum cli_esc_state_t - indicates what to do with an escape character
 *
 * @ESC_REJECT: Invalid escape sequence, so the esc_save[] characters are
 *	returned from each subsequent call to cli_ch_esc()
 * @ESC_SAVE: Character should be saved in esc_save until we have another one
 * @ESC_CONVERTED: Escape sequence has been completed and the resulting
 *	character is available
 */
ਗਿਣਤੀ cli_esc_state_t {
	ESC_REJECT,
	ESC_SAVE,
	ESC_CONVERTED
};

ਖਾਲੀ cli_ch_init(ਬਣਤਰ cli_ch_state *cch)
{
	memset(cch, '\0', ਆਕਾਰ(*cch));
}

/**
 * cli_ch_esc() - Process a character in an ongoing escape sequence
 *
 * @cch: State information
 * @ichar: Character to process
 * @actp: Returns the action to take
 * Returns: Output character if *actp is ESC_CONVERTED, else 0
 */
ਸਥਿਰ ਪੂਰਨ cli_ch_esc(ਬਣਤਰ cli_ch_state *cch, ਪੂਰਨ ichar,
		      ਗਿਣਤੀ cli_esc_state_t *actp)
{
	ਗਿਣਤੀ cli_esc_state_t act = ESC_REJECT;

	ਚੋਣ (cch->esc_len) {
	ਹਾਲਤ 1:
		ਜੇ (ichar == '[' || ichar == 'O')
			act = ESC_SAVE;
		ਨਹੀਂਤਾਂ
			act = ESC_CONVERTED;
		ਰੋਕੋ;
	ਹਾਲਤ 2:
		ਚੋਣ (ichar) {
		ਹਾਲਤ 'D':	/* <- key */
			ichar = CTL_CH('b');
			act = ESC_CONVERTED;
			ਰੋਕੋ;	/* pass off to ^B handler */
		ਹਾਲਤ 'C':	/* -> key */
			ichar = CTL_CH('f');
			act = ESC_CONVERTED;
			ਰੋਕੋ;	/* pass off to ^F handler */
		ਹਾਲਤ 'H':	/* Home key */
			ichar = CTL_CH('a');
			act = ESC_CONVERTED;
			ਰੋਕੋ;	/* pass off to ^A handler */
		ਹਾਲਤ 'F':	/* End key */
			ichar = CTL_CH('e');
			act = ESC_CONVERTED;
			ਰੋਕੋ;	/* pass off to ^E handler */
		ਹਾਲਤ 'A':	/* up arrow */
			ichar = CTL_CH('p');
			act = ESC_CONVERTED;
			ਰੋਕੋ;	/* pass off to ^P handler */
		ਹਾਲਤ 'B':	/* down arrow */
			ichar = CTL_CH('n');
			act = ESC_CONVERTED;
			ਰੋਕੋ;	/* pass off to ^N handler */
		ਹਾਲਤ '1':
		ਹਾਲਤ '2':
		ਹਾਲਤ '3':
		ਹਾਲਤ '4':
		ਹਾਲਤ '7':
		ਹਾਲਤ '8':
			ਜੇ (cch->esc_save[1] == '[') {
				/* see if next character is ~ */
				act = ESC_SAVE;
			}
			ਰੋਕੋ;
		}
		ਰੋਕੋ;
	ਹਾਲਤ 3:
		ਚੋਣ (ichar) {
		ਹਾਲਤ '~':
			ਚੋਣ (cch->esc_save[2]) {
			ਹਾਲਤ '3':	/* Delete key */
				ichar = CTL_CH('d');
				act = ESC_CONVERTED;
				ਰੋਕੋ;	/* pass to ^D handler */
			ਹਾਲਤ '1':	/* Home key */
			ਹਾਲਤ '7':
				ichar = CTL_CH('a');
				act = ESC_CONVERTED;
				ਰੋਕੋ;	/* pass to ^A handler */
			ਹਾਲਤ '4':	/* End key */
			ਹਾਲਤ '8':
				ichar = CTL_CH('e');
				act = ESC_CONVERTED;
				ਰੋਕੋ;	/* pass to ^E handler */
			}
			ਰੋਕੋ;
		ਹਾਲਤ '0':
			ਜੇ (cch->esc_save[2] == '2')
				act = ESC_SAVE;
			ਰੋਕੋ;
		}
		ਰੋਕੋ;
	ਹਾਲਤ 4:
		ਚੋਣ (ichar) {
		ਹਾਲਤ '0':
		ਹਾਲਤ '1':
			act = ESC_SAVE;
			ਰੋਕੋ;		/* bracketed paste */
		}
		ਰੋਕੋ;
	ਹਾਲਤ 5:
		ਜੇ (ichar == '~') {	/* bracketed paste */
			ichar = 0;
			act = ESC_CONVERTED;
		}
	}

	*actp = act;

	ਵਾਪਸ ichar;
}

ਪੂਰਨ cli_ch_process(ਬਣਤਰ cli_ch_state *cch, ਪੂਰਨ ichar)
{
	/*
	 * ichar=0x0 when error occurs in U-Boot getchar() or when the caller
	 * wants to check if there are more characters saved in the escape
	 * sequence
	 */
	ਜੇ (!ichar) {
		ਜੇ (cch->emitting) {
			ਜੇ (cch->emit_upto < cch->esc_len)
				ਵਾਪਸ cch->esc_save[cch->emit_upto++];
			cch->emit_upto = 0;
			cch->emitting = false;
			cch->esc_len = 0;
		}
		ਵਾਪਸ 0;
	} ਨਹੀਂਤਾਂ ਜੇ (ichar == -ETIMEDOUT) {
		/*
		 * If we are in an escape sequence but nothing has followed the
		 * Escape character, then the user probably just pressed the
		 * Escape key. Return it and clear the sequence.
		 */
		ਜੇ (cch->esc_len) {
			cch->esc_len = 0;
			ਵਾਪਸ '\e';
		}

		/* Otherwise there is nothing to return */
		ਵਾਪਸ 0;
	}

	ਜੇ (ichar == '\n' || ichar == '\r')
		ਵਾਪਸ '\n';

	/* handle standard linux xterm esc sequences for arrow key, etc. */
	ਜੇ (cch->esc_len != 0) {
		ਗਿਣਤੀ cli_esc_state_t act;

		ichar = cli_ch_esc(cch, ichar, &act);

		ਚੋਣ (act) {
		ਹਾਲਤ ESC_SAVE:
			/* save this character and return nothing */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = 0;
			ਰੋਕੋ;
		ਹਾਲਤ ESC_REJECT:
			/*
			 * invalid escape sequence, start returning the
			 * characters in it
			 */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = cch->esc_save[cch->emit_upto++];
			cch->emitting = true;
			ਵਾਪਸ ichar;
		ਹਾਲਤ ESC_CONVERTED:
			/* valid escape sequence, return the resulting char */
			cch->esc_len = 0;
			ਰੋਕੋ;
		}
	}

	ਜੇ (ichar == '\e') {
		ਜੇ (!cch->esc_len) {
			cch->esc_save[cch->esc_len] = ichar;
			cch->esc_len = 1;
		} ਨਹੀਂਤਾਂ {
			puts("impossible condition #876\n");
			cch->esc_len = 0;
		}
		ਵਾਪਸ 0;
	}

	ਵਾਪਸ ichar;
}
