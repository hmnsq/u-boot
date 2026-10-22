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
	स_रखो(cch, '\0', आकार(*cch));
}

/**
 * cli_ch_esc() - Process a character in an ongoing escape sequence
 *
 * @cch: State information
 * @ichar: Character to process
 * @actp: Returns the action to take
 * Returns: Output character if *actp is ESC_CONVERTED, else 0
 */
स्थिर पूर्णांक cli_ch_esc(संरचना cli_ch_state *cch, पूर्णांक ichar,
		      गणना cli_esc_state_t *actp)
{
	गणना cli_esc_state_t act = ESC_REJECT;

	निवड (cch->esc_len) {
	स्थिती 1:
		जर (ichar == '[' || ichar == 'O')
			act = ESC_SAVE;
		अन्यथा
			act = ESC_CONVERTED;
		थांबा;
	स्थिती 2:
		निवड (ichar) {
		स्थिती 'D':	/* <- key */
			ichar = CTL_CH('b');
			act = ESC_CONVERTED;
			थांबा;	/* pass off to ^B handler */
		स्थिती 'C':	/* -> key */
			ichar = CTL_CH('f');
			act = ESC_CONVERTED;
			थांबा;	/* pass off to ^F handler */
		स्थिती 'H':	/* Home key */
			ichar = CTL_CH('a');
			act = ESC_CONVERTED;
			थांबा;	/* pass off to ^A handler */
		स्थिती 'F':	/* End key */
			ichar = CTL_CH('e');
			act = ESC_CONVERTED;
			थांबा;	/* pass off to ^E handler */
		स्थिती 'A':	/* up arrow */
			ichar = CTL_CH('p');
			act = ESC_CONVERTED;
			थांबा;	/* pass off to ^P handler */
		स्थिती 'B':	/* down arrow */
			ichar = CTL_CH('n');
			act = ESC_CONVERTED;
			थांबा;	/* pass off to ^N handler */
		स्थिती '1':
		स्थिती '2':
		स्थिती '3':
		स्थिती '4':
		स्थिती '7':
		स्थिती '8':
			जर (cch->esc_save[1] == '[') {
				/* see if next character is ~ */
				act = ESC_SAVE;
			}
			थांबा;
		}
		थांबा;
	स्थिती 3:
		निवड (ichar) {
		स्थिती '~':
			निवड (cch->esc_save[2]) {
			स्थिती '3':	/* Delete key */
				ichar = CTL_CH('d');
				act = ESC_CONVERTED;
				थांबा;	/* pass to ^D handler */
			स्थिती '1':	/* Home key */
			स्थिती '7':
				ichar = CTL_CH('a');
				act = ESC_CONVERTED;
				थांबा;	/* pass to ^A handler */
			स्थिती '4':	/* End key */
			स्थिती '8':
				ichar = CTL_CH('e');
				act = ESC_CONVERTED;
				थांबा;	/* pass to ^E handler */
			}
			थांबा;
		स्थिती '0':
			जर (cch->esc_save[2] == '2')
				act = ESC_SAVE;
			थांबा;
		}
		थांबा;
	स्थिती 4:
		निवड (ichar) {
		स्थिती '0':
		स्थिती '1':
			act = ESC_SAVE;
			थांबा;		/* bracketed paste */
		}
		थांबा;
	स्थिती 5:
		जर (ichar == '~') {	/* bracketed paste */
			ichar = 0;
			act = ESC_CONVERTED;
		}
	}

	*actp = act;

	परत ichar;
}

पूर्णांक cli_ch_process(संरचना cli_ch_state *cch, पूर्णांक ichar)
{
	/*
	 * ichar=0x0 when error occurs in U-Boot getchar() or when the caller
	 * wants to check if there are more characters saved in the escape
	 * sequence
	 */
	जर (!ichar) {
		जर (cch->emitting) {
			जर (cch->emit_upto < cch->esc_len)
				परत cch->esc_save[cch->emit_upto++];
			cch->emit_upto = 0;
			cch->emitting = false;
			cch->esc_len = 0;
		}
		परत 0;
	} अन्यथा जर (ichar == -ETIMEDOUT) {
		/*
		 * If we are in an escape sequence but nothing has followed the
		 * Escape character, then the user probably just pressed the
		 * Escape key. Return it and clear the sequence.
		 */
		जर (cch->esc_len) {
			cch->esc_len = 0;
			परत '\e';
		}

		/* Otherwise there is nothing to return */
		परत 0;
	}

	जर (ichar == '\n' || ichar == '\r')
		परत '\n';

	/* handle standard linux xterm esc sequences for arrow key, etc. */
	जर (cch->esc_len != 0) {
		गणना cli_esc_state_t act;

		ichar = cli_ch_esc(cch, ichar, &act);

		निवड (act) {
		स्थिती ESC_SAVE:
			/* save this character and return nothing */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = 0;
			थांबा;
		स्थिती ESC_REJECT:
			/*
			 * invalid escape sequence, start returning the
			 * characters in it
			 */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = cch->esc_save[cch->emit_upto++];
			cch->emitting = true;
			परत ichar;
		स्थिती ESC_CONVERTED:
			/* valid escape sequence, return the resulting char */
			cch->esc_len = 0;
			थांबा;
		}
	}

	जर (ichar == '\e') {
		जर (!cch->esc_len) {
			cch->esc_save[cch->esc_len] = ichar;
			cch->esc_len = 1;
		} अन्यथा {
			माला_दो("impossible condition #876\n");
			cch->esc_len = 0;
		}
		परत 0;
	}

	परत ichar;
}
