// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright 2022 Google LLC
 */

#ضمن <cli.h>
#ضمن <stdio.h>
#ضمن <string.h>
#ضمن <linux/errno.h>

/**
 * enum cli_esc_state_t - indicates what to do with an escape character
 *
 * @ESC_REJECT: Invalid escape sequence, so the esc_save[] characters are
 *	returned from each subsequent call to cli_ch_esc()
 * @ESC_SAVE: Character should be saved in esc_save until we have another one
 * @ESC_CONVERTED: Escape sequence has been completed and the resulting
 *	character is available
 */
تعداد cli_esc_state_t {
	ESC_REJECT,
	ESC_SAVE,
	ESC_CONVERTED
};

فراغ cli_ch_init(بنية cli_ch_state *cch)
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
ساكن صحيح cli_ch_esc(بنية cli_ch_state *cch, صحيح ichar,
		      تعداد cli_esc_state_t *actp)
{
	تعداد cli_esc_state_t act = ESC_REJECT;

	بدل (cch->esc_len) {
	حالة 1:
		إذا (ichar == '[' || ichar == 'O')
			act = ESC_SAVE;
		وإلا
			act = ESC_CONVERTED;
		اكسر;
	حالة 2:
		بدل (ichar) {
		حالة 'D':	/* <- key */
			ichar = CTL_CH('b');
			act = ESC_CONVERTED;
			اكسر;	/* pass off to ^B handler */
		حالة 'C':	/* -> key */
			ichar = CTL_CH('f');
			act = ESC_CONVERTED;
			اكسر;	/* pass off to ^F handler */
		حالة 'H':	/* Home key */
			ichar = CTL_CH('a');
			act = ESC_CONVERTED;
			اكسر;	/* pass off to ^A handler */
		حالة 'F':	/* End key */
			ichar = CTL_CH('e');
			act = ESC_CONVERTED;
			اكسر;	/* pass off to ^E handler */
		حالة 'A':	/* up arrow */
			ichar = CTL_CH('p');
			act = ESC_CONVERTED;
			اكسر;	/* pass off to ^P handler */
		حالة 'B':	/* down arrow */
			ichar = CTL_CH('n');
			act = ESC_CONVERTED;
			اكسر;	/* pass off to ^N handler */
		حالة '1':
		حالة '2':
		حالة '3':
		حالة '4':
		حالة '7':
		حالة '8':
			إذا (cch->esc_save[1] == '[') {
				/* see if next character is ~ */
				act = ESC_SAVE;
			}
			اكسر;
		}
		اكسر;
	حالة 3:
		بدل (ichar) {
		حالة '~':
			بدل (cch->esc_save[2]) {
			حالة '3':	/* Delete key */
				ichar = CTL_CH('d');
				act = ESC_CONVERTED;
				اكسر;	/* pass to ^D handler */
			حالة '1':	/* Home key */
			حالة '7':
				ichar = CTL_CH('a');
				act = ESC_CONVERTED;
				اكسر;	/* pass to ^A handler */
			حالة '4':	/* End key */
			حالة '8':
				ichar = CTL_CH('e');
				act = ESC_CONVERTED;
				اكسر;	/* pass to ^E handler */
			}
			اكسر;
		حالة '0':
			إذا (cch->esc_save[2] == '2')
				act = ESC_SAVE;
			اكسر;
		}
		اكسر;
	حالة 4:
		بدل (ichar) {
		حالة '0':
		حالة '1':
			act = ESC_SAVE;
			اكسر;		/* bracketed paste */
		}
		اكسر;
	حالة 5:
		إذا (ichar == '~') {	/* bracketed paste */
			ichar = 0;
			act = ESC_CONVERTED;
		}
	}

	*actp = act;

	أرجع ichar;
}

صحيح cli_ch_process(بنية cli_ch_state *cch, صحيح ichar)
{
	/*
	 * ichar=0x0 when error occurs in U-Boot getchar() or when the caller
	 * wants to check if there are more characters saved in the escape
	 * sequence
	 */
	إذا (!ichar) {
		إذا (cch->emitting) {
			إذا (cch->emit_upto < cch->esc_len)
				أرجع cch->esc_save[cch->emit_upto++];
			cch->emit_upto = 0;
			cch->emitting = false;
			cch->esc_len = 0;
		}
		أرجع 0;
	} وإلا إذا (ichar == -ETIMEDOUT) {
		/*
		 * If we are in an escape sequence but nothing has followed the
		 * Escape character, then the user probably just pressed the
		 * Escape key. Return it and clear the sequence.
		 */
		إذا (cch->esc_len) {
			cch->esc_len = 0;
			أرجع '\e';
		}

		/* Otherwise there is nothing to return */
		أرجع 0;
	}

	إذا (ichar == '\n' || ichar == '\r')
		أرجع '\n';

	/* handle standard linux xterm esc sequences for arrow key, etc. */
	إذا (cch->esc_len != 0) {
		تعداد cli_esc_state_t act;

		ichar = cli_ch_esc(cch, ichar, &act);

		بدل (act) {
		حالة ESC_SAVE:
			/* save this character and return nothing */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = 0;
			اكسر;
		حالة ESC_REJECT:
			/*
			 * invalid escape sequence, start returning the
			 * characters in it
			 */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = cch->esc_save[cch->emit_upto++];
			cch->emitting = true;
			أرجع ichar;
		حالة ESC_CONVERTED:
			/* valid escape sequence, return the resulting char */
			cch->esc_len = 0;
			اكسر;
		}
	}

	إذا (ichar == '\e') {
		إذا (!cch->esc_len) {
			cch->esc_save[cch->esc_len] = ichar;
			cch->esc_len = 1;
		} وإلا {
			puts("impossible condition #876\n");
			cch->esc_len = 0;
		}
		أرجع 0;
	}

	أرجع ichar;
}
