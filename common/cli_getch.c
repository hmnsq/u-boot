// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright 2022 Google LLC
 */

#شامل <cli.h>
#شامل <stdio.h>
#شامل <string.h>
#شامل <linux/errno.h>

/**
 * enum cli_esc_state_t - indicates what to do with an escape character
 *
 * @ESC_REJECT: Invalid escape sequence, so the esc_save[] characters are
 *	returned from each subsequent call to cli_ch_esc()
 * @ESC_SAVE: Character should be saved in esc_save until we have another one
 * @ESC_CONVERTED: Escape sequence has been completed and the resulting
 *	character is available
 */
شمار cli_esc_state_t {
	ESC_REJECT,
	ESC_SAVE,
	ESC_CONVERTED
};

خالی cli_ch_init(ساخت cli_ch_state *cch)
{
	memset(cch, '\0', حجم(*cch));
}

/**
 * cli_ch_esc() - Process a character in an ongoing escape sequence
 *
 * @cch: State information
 * @ichar: Character to process
 * @actp: Returns the action to take
 * Returns: Output character if *actp is ESC_CONVERTED, else 0
 */
ساکن صحیح cli_ch_esc(ساخت cli_ch_state *cch, صحیح ichar,
		      شمار cli_esc_state_t *actp)
{
	شمار cli_esc_state_t act = ESC_REJECT;

	انتخاب (cch->esc_len) {
	حالت 1:
		اگر (ichar == '[' || ichar == 'O')
			act = ESC_SAVE;
		ورنہ
			act = ESC_CONVERTED;
		توڑو;
	حالت 2:
		انتخاب (ichar) {
		حالت 'D':	/* <- key */
			ichar = CTL_CH('b');
			act = ESC_CONVERTED;
			توڑو;	/* pass off to ^B handler */
		حالت 'C':	/* -> key */
			ichar = CTL_CH('f');
			act = ESC_CONVERTED;
			توڑو;	/* pass off to ^F handler */
		حالت 'H':	/* Home key */
			ichar = CTL_CH('a');
			act = ESC_CONVERTED;
			توڑو;	/* pass off to ^A handler */
		حالت 'F':	/* End key */
			ichar = CTL_CH('e');
			act = ESC_CONVERTED;
			توڑو;	/* pass off to ^E handler */
		حالت 'A':	/* up arrow */
			ichar = CTL_CH('p');
			act = ESC_CONVERTED;
			توڑو;	/* pass off to ^P handler */
		حالت 'B':	/* down arrow */
			ichar = CTL_CH('n');
			act = ESC_CONVERTED;
			توڑو;	/* pass off to ^N handler */
		حالت '1':
		حالت '2':
		حالت '3':
		حالت '4':
		حالت '7':
		حالت '8':
			اگر (cch->esc_save[1] == '[') {
				/* see if next character is ~ */
				act = ESC_SAVE;
			}
			توڑو;
		}
		توڑو;
	حالت 3:
		انتخاب (ichar) {
		حالت '~':
			انتخاب (cch->esc_save[2]) {
			حالت '3':	/* Delete key */
				ichar = CTL_CH('d');
				act = ESC_CONVERTED;
				توڑو;	/* pass to ^D handler */
			حالت '1':	/* Home key */
			حالت '7':
				ichar = CTL_CH('a');
				act = ESC_CONVERTED;
				توڑو;	/* pass to ^A handler */
			حالت '4':	/* End key */
			حالت '8':
				ichar = CTL_CH('e');
				act = ESC_CONVERTED;
				توڑو;	/* pass to ^E handler */
			}
			توڑو;
		حالت '0':
			اگر (cch->esc_save[2] == '2')
				act = ESC_SAVE;
			توڑو;
		}
		توڑو;
	حالت 4:
		انتخاب (ichar) {
		حالت '0':
		حالت '1':
			act = ESC_SAVE;
			توڑو;		/* bracketed paste */
		}
		توڑو;
	حالت 5:
		اگر (ichar == '~') {	/* bracketed paste */
			ichar = 0;
			act = ESC_CONVERTED;
		}
	}

	*actp = act;

	واپس ichar;
}

صحیح cli_ch_process(ساخت cli_ch_state *cch, صحیح ichar)
{
	/*
	 * ichar=0x0 when error occurs in U-Boot getchar() or when the caller
	 * wants to check if there are more characters saved in the escape
	 * sequence
	 */
	اگر (!ichar) {
		اگر (cch->emitting) {
			اگر (cch->emit_upto < cch->esc_len)
				واپس cch->esc_save[cch->emit_upto++];
			cch->emit_upto = 0;
			cch->emitting = false;
			cch->esc_len = 0;
		}
		واپس 0;
	} ورنہ اگر (ichar == -ETIMEDOUT) {
		/*
		 * If we are in an escape sequence but nothing has followed the
		 * Escape character, then the user probably just pressed the
		 * Escape key. Return it and clear the sequence.
		 */
		اگر (cch->esc_len) {
			cch->esc_len = 0;
			واپس '\e';
		}

		/* Otherwise there is nothing to return */
		واپس 0;
	}

	اگر (ichar == '\n' || ichar == '\r')
		واپس '\n';

	/* handle standard linux xterm esc sequences for arrow key, etc. */
	اگر (cch->esc_len != 0) {
		شمار cli_esc_state_t act;

		ichar = cli_ch_esc(cch, ichar, &act);

		انتخاب (act) {
		حالت ESC_SAVE:
			/* save this character and return nothing */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = 0;
			توڑو;
		حالت ESC_REJECT:
			/*
			 * invalid escape sequence, start returning the
			 * characters in it
			 */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = cch->esc_save[cch->emit_upto++];
			cch->emitting = true;
			واپس ichar;
		حالت ESC_CONVERTED:
			/* valid escape sequence, return the resulting char */
			cch->esc_len = 0;
			توڑو;
		}
	}

	اگر (ichar == '\e') {
		اگر (!cch->esc_len) {
			cch->esc_save[cch->esc_len] = ichar;
			cch->esc_len = 1;
		} ورنہ {
			puts("impossible condition #876\n");
			cch->esc_len = 0;
		}
		واپس 0;
	}

	واپس ichar;
}
