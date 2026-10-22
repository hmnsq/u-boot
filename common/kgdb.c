/* taken from arch/powerpc/kernel/ppc-stub.c */

/****************************************************************************

		THIS SOFTWARE IS NOT COPYRIGHTED

   HP offers the following for use in the public domain.  HP makes no
   warranty with regard to the software or its performance and the
   user accepts the software "AS IS" with all faults.

   HP DISCLAIMS ANY WARRANTIES, EXPRESS OR IMPLIED, WITH REGARD
   TO THIS SOFTWARE INCLUDING BUT NOT LIMITED TO THE WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

****************************************************************************/

/****************************************************************************
 *  Header: remcom.c,v 1.34 91/03/09 12:29:49 glenne Exp $
 *
 *  Module name: remcom.c $
 *  Revision: 1.34 $
 *  Date: 91/03/09 12:29:49 $
 *  Contributor:     Lake Stevens Instrument Division$
 *
 *  Description:     low level support for gdb debugger. $
 *
 *  Considerations:  only works on target hardware $
 *
 *  Written by:      Glenn Engel $
 *  ModuleState:     Experimental $
 *
 *  NOTES:           See Below $
 *
 *  Modified for SPARC by Stu Grossman, Cygnus Support.
 *
 *  This code has been extensively tested on the Fujitsu SPARClite demo board.
 *
 *  To enable debugger support, two things need to happen.  One, a
 *  call to set_debug_traps() is necessary in order to allow any breakpoints
 *  or error conditions to be properly intercepted and reported to gdb.
 *  Two, a breakpoint needs to be generated to begin communication.  This
 *  is most easily accomplished by a call to breakpoint().  Breakpoint()
 *  simulates a breakpoint by executing a trap #1.
 *
 *************
 *
 *    The following gdb commands are supported:
 *
 * command          function                               Return value
 *
 *    g             return the value of the CPU registers  hex data or ENN
 *    G             set the value of the CPU registers     OK or ENN
 *    qOffsets      Get section offsets.  Reply is Text=xxx;Data=yyy;Bss=zzz
 *
 *    mAA..AA,LLLL  Read LLLL bytes at address AA..AA      hex data or ENN
 *    MAA..AA,LLLL: Write LLLL bytes at address AA.AA      OK or ENN
 *
 *    c             Resume at current address              SNN   ( signal NN)
 *    cAA..AA       Continue at address AA..AA             SNN
 *
 *    s             Step one instruction                   SNN
 *    sAA..AA       Step one instruction from AA..AA       SNN
 *
 *    k             kill
 *
 *    ?             What was the last sigval ?             SNN   (signal NN)
 *
 *    bBB..BB	    Set baud rate to BB..BB		   OK or BNN, then sets
 *							   baud rate
 *
 * All commands and responses are sent with a packet which includes a
 * checksum.  A packet consists of
 *
 * $<packet info>#<checksum>.
 *
 * where
 * <packet info> :: <characters representing the command or response>
 * <checksum>    :: <two hex digits computed as modulo 256 sum of <packetinfo>>
 *
 * When a packet is received, it is first acknowledged with either '+' or '-'.
 * '+' indicates a successful transfer.  '-' indicates a failed transfer.
 *
 * Example:
 *
 * Host:                  Reply:
 * $m0,10#2a               +$00010203040506070809101112131415#42
 *
 ****************************************************************************/

#ಅನ್ತರ್ಭಾವ <asm/ptrace.h>

#ಅನ್ತರ್ಭಾವ <kgdb.h>
#ಅನ್ತರ್ಭಾವ <command.h>

#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು KGDB_DEBUG

/*
 * BUFMAX defines the maximum number of characters in inbound/outbound buffers
 */
#ವ್ಯಾಖ್ಯಾನಿಸು BUFMAX 1024
ಸ್ಥಿರ ಅಕ್ಷರ remcom_ಒಳ_ಬಫರ್[BUFMAX];
ಸ್ಥಿರ ಅಕ್ಷರ remcom_ಹೊರಹೋಗು_ಬಫರ್[BUFMAX];
ಸ್ಥಿರ ಅಕ್ಷರ remcom_Reg_ಬಫರ್[BUFMAX];

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initialized = 0;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kgdb_ಸಕ್ರಿಯ;
ಸ್ಥಿರ ರಚನೆ pt_regs ನಮೂದು_regs;
ಸ್ಥಿರ ದೀರ್ಘ error_jmp_buf[BUFMAX/2];
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ longjmp_on_fault = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ KGDB_DEBUG
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kdebug = 1;
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ hexchars[]="0123456789abcdef";

/* Convert ch from a hex digit to an int */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
hex(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ch)
{
	ಆದರೆ (ch >= 'a' && ch <= 'f')
		ಹಿಂತಿರುಗಿ ch-'a'+10;
	ಆದರೆ (ch >= '0' && ch <= '9')
		ಹಿಂತಿರುಗಿ ch-'0';
	ಆದರೆ (ch >= 'A' && ch <= 'F')
		ಹಿಂತಿರುಗಿ ch-'A'+10;
	ಹಿಂತಿರುಗಿ -1;
}

/* Convert the memory pointed to by mem into hex, placing result in buf.
 * Return a pointer to the last char put in buf (null).
 */
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *
mem2hex(ಅಕ್ಷರ *mem, ಅಕ್ಷರ *buf, ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2)
{
	ಅಕ್ಷರ *tmp;
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ch;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory copy.  Hex conversion will work against this one.
	 */
	tmp = buf + ಎಣಿಕೆ_2;
	longjmp_on_fault = 1;

	ಸ್ಮರಣೆ_ನಕಲು(tmp, mem, ಎಣಿಕೆ_2);

	ಆಗ (ಎಣಿಕೆ_2-- > 0) {
		ch = *tmp++;
		*buf++ = hexchars[ch >> 4];
		*buf++ = hexchars[ch & 0xf];
	}
	*buf = 0;
	longjmp_on_fault = 0;
	ಹಿಂತಿರುಗಿ (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)buf;
}

/* convert the hex array pointed to by buf into binary to be placed in mem
 * return a pointer to the character AFTER the last byte fetched from buf.
*/
ಸ್ಥಿರ ಅಕ್ಷರ *
hex2mem(ಅಕ್ಷರ *buf, ಅಕ್ಷರ *mem, ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2)
{
	ಪೂರ್ಣಾಂಕ hex_ಮೌಲ್ಯ;
	ಅಕ್ಷರ *tmp_raw, *tmp_hex;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory that is converted from hex.
	 */
	tmp_raw = buf + ಎಣಿಕೆ_2 * 2;
	tmp_hex = tmp_raw - 1;

	longjmp_on_fault = 1;
	ಆಗ (tmp_hex >= buf) {
		tmp_raw--;
		hex_ಮೌಲ್ಯ = hex(*tmp_hex--);
		ಆದರೆ (hex_ಮೌಲ್ಯ < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw = hex_ಮೌಲ್ಯ;
		hex_ಮೌಲ್ಯ = hex(*tmp_hex--);
		ಆದರೆ (hex_ಮೌಲ್ಯ < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw |= hex_ಮೌಲ್ಯ << 4;

	}

	ಸ್ಮರಣೆ_ನಕಲು(mem, tmp_raw, ಎಣಿಕೆ_2);

	kgdb_flush_cache_range((ಶೂನ್ಯ *)mem, (ಶೂನ್ಯ *)(mem+ಎಣಿಕೆ_2));
	longjmp_on_fault = 0;

	ಹಿಂತಿರುಗಿ buf;
}

/*
 * While we find nice hex chars, build an int.
 * Return number of chars processed.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
hex_ಗಮ್ಯ_Int(ಅಕ್ಷರ **ptr, ಪೂರ್ಣಾಂಕ *int_ಮೌಲ್ಯ)
{
	ಪೂರ್ಣಾಂಕ ಅಂಕೆ_Chars = 0;
	ಪೂರ್ಣಾಂಕ hex_ಮೌಲ್ಯ;

	*int_ಮೌಲ್ಯ = 0;

	longjmp_on_fault = 1;
	ಆಗ (**ptr) {
		hex_ಮೌಲ್ಯ = hex(**ptr);
		ಆದರೆ (hex_ಮೌಲ್ಯ < 0)
			ನಿಲ್ಲಿಸು;

		*int_ಮೌಲ್ಯ = (*int_ಮೌಲ್ಯ << 4) | hex_ಮೌಲ್ಯ;
		ಅಂಕೆ_Chars ++;

		(*ptr)++;
	}
	longjmp_on_fault = 0;

	ಹಿಂತಿರುಗಿ (ಅಂಕೆ_Chars);
}

/* scan for the sequence $<data>#<checksum>     */
ಸ್ಥಿರ ಶೂನ್ಯ
getpacket(ಅಕ್ಷರ *ಬಫರ್)
{
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ checksum;
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ xmitcsum;
	ಪೂರ್ಣಾಂಕ i;
	ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2;
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ch;

	ಮಾಡು {
		/* wait around for the start character, ignore all other
		 * characters */
		ಆಗ ((ch = (getDebugChar() & 0x7f)) != '$') {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ KGDB_DEBUG
			ಆದರೆ (kdebug)
				putc(ch);
#ಆದರೆ_ಅಂತ್ಯ
			;
		}

		checksum = 0;
		xmitcsum = -1;

		ಎಣಿಕೆ_2 = 0;

		/* now, read until a # or end of buffer is found */
		ಆಗ (ಎಣಿಕೆ_2 < BUFMAX) {
			ch = getDebugChar() & 0x7f;
			ಆದರೆ (ch == '#')
				ನಿಲ್ಲಿಸು;
			checksum = checksum + ch;
			ಬಫರ್[ಎಣಿಕೆ_2] = ch;
			ಎಣಿಕೆ_2 = ಎಣಿಕೆ_2 + 1;
		}

		ಆದರೆ (ಎಣಿಕೆ_2 >= BUFMAX)
			ಮುಂದುವರಿ;

		ಬಫರ್[ಎಣಿಕೆ_2] = 0;

		ಆದರೆ (ch == '#') {
			xmitcsum = hex(getDebugChar() & 0x7f) << 4;
			xmitcsum |= hex(getDebugChar() & 0x7f);
			ಆದರೆ (checksum != xmitcsum)
				putDebugChar('-');	/* failed checksum */
			ಇಲ್ಲದಿದ್ದರೆ {
				putDebugChar('+'); /* successful transfer */
				/* if a sequence char is present, reply the ID */
				ಆದರೆ (ಬಫರ್[2] == ':') {
					putDebugChar(ಬಫರ್[0]);
					putDebugChar(ಬಫರ್[1]);
					/* remove sequence chars from buffer */
					ಎಣಿಕೆ_2 = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಬಫರ್);
					ಗಾಗಿ (i=3; i <= ಎಣಿಕೆ_2; i++)
						ಬಫರ್[i-3] = ಬಫರ್[i];
				}
			}
		}
	} ಆಗ (checksum != xmitcsum);
}

/* send the packet in buffer.  */
ಸ್ಥಿರ ಶೂನ್ಯ
putpacket(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಬಫರ್)
{
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ checksum;
	ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2;
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ch, recv;

	/*  $<packet info>#<checksum>. */
	ಮಾಡು {
		putDebugChar('$');
		checksum = 0;
		ಎಣಿಕೆ_2 = 0;

		ಆಗ ((ch = ಬಫರ್[ಎಣಿಕೆ_2])) {
			putDebugChar(ch);
			checksum += ch;
			ಎಣಿಕೆ_2 += 1;
		}

		putDebugChar('#');
		putDebugChar(hexchars[checksum >> 4]);
		putDebugChar(hexchars[checksum & 0xf]);
		recv = getDebugChar();
	} ಆಗ ((recv & 0x7f) != '+');
}

/*
 * This function does all command processing for interfacing to gdb.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
handle_exception (ರಚನೆ pt_regs *regs)
{
	ಪೂರ್ಣಾಂಕ addr;
	ಪೂರ್ಣಾಂಕ ಉದ್ದಳತೆ;
	ಅಕ್ಷರ *ptr;
	kgdb_ದತ್ತಾಂಶ kd;
	ಪೂರ್ಣಾಂಕ i;

	ಆದರೆ (!initialized) {
		ಬರೆ("kgdb: exception before kgdb is initialized! huh?\n");
		ಹಿಂತಿರುಗಿ (0);
	}

	/* probably should check which exception occurred as well */
	ಆದರೆ (longjmp_on_fault) {
		longjmp_on_fault = 0;
		kgdb_longjmp(error_jmp_buf, KGDBERR_MEMFAULT);
		panic("kgdb longjump failed!\n");
	}

	ಆದರೆ (kgdb_ಸಕ್ರಿಯ) {
		ಬರೆ("kgdb: unexpected exception from within kgdb\n");
		ಹಿಂತಿರುಗಿ (0);
	}
	kgdb_ಸಕ್ರಿಯ = 1;

	kgdb_interruptible(0);

	ಬರೆ("kgdb: handle_exception; trap [0x%x]\n", kgdb_trap(regs));

	ಆದರೆ (kgdb_setjmp(error_jmp_buf) != 0)
		panic("kgdb: error or fault in entry init!\n");

	kgdb_enter(regs, &kd);

	ನಮೂದು_regs = *regs;

	ptr = remcom_ಹೊರಹೋಗು_ಬಫರ್;

	*ptr++ = 'T';

	*ptr++ = hexchars[kd.sigval >> 4];
	*ptr++ = hexchars[kd.sigval & 0xf];

	ಗಾಗಿ (i = 0; i < kd.nregs; i++) {
		kgdb_reg *rp = &kd.regs[i];

		*ptr++ = hexchars[rp->ಅಂಕೆ >> 4];
		*ptr++ = hexchars[rp->ಅಂಕೆ & 0xf];
		*ptr++ = ':';
		ptr = (ಅಕ್ಷರ *)mem2hex((ಅಕ್ಷರ *)&rp->val, ptr, 4);
		*ptr++ = ';';
	}

	*ptr = 0;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ KGDB_DEBUG
	ಆದರೆ (kdebug)
		ಬರೆ("kgdb: remcomOutBuffer: %s\n", remcom_ಹೊರಹೋಗು_ಬಫರ್);
#ಆದರೆ_ಅಂತ್ಯ

	putpacket((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)&remcom_ಹೊರಹೋಗು_ಬಫರ್);

	ಆಗ (1) {
		ಅಸ್ಥಿರ ಪೂರ್ಣಾಂಕ errnum;

		remcom_ಹೊರಹೋಗು_ಬಫರ್[0] = 0;

		getpacket(remcom_ಒಳ_ಬಫರ್);
		ptr = &remcom_ಒಳ_ಬಫರ್[1];

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ KGDB_DEBUG
		ಆದರೆ (kdebug)
			ಬರೆ("kgdb:  remcomInBuffer: %s\n", remcom_ಒಳ_ಬಫರ್);
#ಆದರೆ_ಅಂತ್ಯ

		errnum = kgdb_setjmp(error_jmp_buf);

		ಆದರೆ (errnum == 0) ಆಯ್ಕೆ (remcom_ಒಳ_ಬಫರ್[0]) {

		ಸ್ಥಿತಿ '?':               /* report most recent signal */
			remcom_ಹೊರಹೋಗು_ಬಫರ್[0] = 'S';
			remcom_ಹೊರಹೋಗು_ಬಫರ್[1] = hexchars[kd.sigval >> 4];
			remcom_ಹೊರಹೋಗು_ಬಫರ್[2] = hexchars[kd.sigval & 0xf];
			remcom_ಹೊರಹೋಗು_ಬಫರ್[3] = 0;
			ನಿಲ್ಲಿಸು;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ KGDB_DEBUG
		ಸ್ಥಿತಿ 'd':
			/* toggle debug flag */
			kdebug ^= 1;
			ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

		ಸ್ಥಿತಿ 'g':	/* return the value of the CPU registers. */
			ಉದ್ದಳತೆ = kgdb_getregs(regs, remcom_Reg_ಬಫರ್, BUFMAX);
			mem2hex(remcom_Reg_ಬಫರ್, remcom_ಹೊರಹೋಗು_ಬಫರ್, ಉದ್ದಳತೆ);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'G':   /* set the value of the CPU registers */
			ಉದ್ದಳತೆ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ptr);
			ಆದರೆ ((ಉದ್ದಳತೆ & 1) != 0) kgdb_error(KGDBERR_BADPARAMS);
			hex2mem(ptr, remcom_Reg_ಬಫರ್, ಉದ್ದಳತೆ/2);
			kgdb_putregs(regs, remcom_Reg_ಬಫರ್, ಉದ್ದಳತೆ/2);
			strcpy(remcom_ಹೊರಹೋಗು_ಬಫರ್,"OK");
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'm':	/* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
				/* Try to read %x,%x.  */

			ಆದರೆ (hex_ಗಮ್ಯ_Int(&ptr, &addr)
			    && *ptr++ == ','
			    && hex_ಗಮ್ಯ_Int(&ptr, &ಉದ್ದಳತೆ))	{
				mem2hex((ಅಕ್ಷರ *)addr, remcom_ಹೊರಹೋಗು_ಬಫರ್, ಉದ್ದಳತೆ);
			} ಇಲ್ಲದಿದ್ದರೆ {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'M': /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
			/* Try to read '%x,%x:'.  */

			ಆದರೆ (hex_ಗಮ್ಯ_Int(&ptr, &addr)
			    && *ptr++ == ','
			    && hex_ಗಮ್ಯ_Int(&ptr, &ಉದ್ದಳತೆ)
			    && *ptr++ == ':') {
				hex2mem(ptr, (ಅಕ್ಷರ *)addr, ಉದ್ದಳತೆ);
				strcpy(remcom_ಹೊರಹೋಗು_ಬಫರ್, "OK");
			} ಇಲ್ಲದಿದ್ದರೆ {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'k':    /* kill the program, actually return to monitor */
			kd.extype = KGDBEXIT_KILL;
			*regs = ನಮೂದು_regs;
			ಹೋಗು doexit;

		ಸ್ಥಿತಿ 'C':    /* CSS  continue with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		ಸ್ಥಿತಿ 'c':    /* cAA..AA  Continue; address AA..AA optional */
			/* try to read optional parameter, pc unchanged if no parm */
			kd.extype = KGDBEXIT_CONTINUE;

			ಆದರೆ (hex_ಗಮ್ಯ_Int(&ptr, &addr)) {
				kd.exaddr = addr;
				kd.extype |= KGDBEXIT_WITHADDR;
			}

			ಹೋಗು doexit;

		ಸ್ಥಿತಿ 'S':    /* SSS  single step with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		ಸ್ಥಿತಿ 's':
			kd.extype = KGDBEXIT_SINGLE;

			ಆದರೆ (hex_ಗಮ್ಯ_Int(&ptr, &addr)) {
				kd.exaddr = addr;
				kd.extype |= KGDBEXIT_WITHADDR;
			}

		doexit:
/* Need to flush the instruction cache here, as we may have deposited a
 * breakpoint, and the icache probably has no way of knowing that a data ref to
 * some location may have changed something that is in the instruction cache.
 */
			kgdb_flush_cache_all();
			kgdb_exit(regs, &kd);
			kgdb_ಸಕ್ರಿಯ = 0;
			kgdb_interruptible(1);
			ಹಿಂತಿರುಗಿ (1);

		ಸ್ಥಿತಿ 'r':		/* Reset (if user process..exit ???)*/
			panic("kgdb reset.");
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'P':    /* Pr=v  set reg r to value v (r and v are hex) */
			ಆದರೆ (hex_ಗಮ್ಯ_Int(&ptr, &addr)
			    && *ptr++ == '='
			    && ((ಉದ್ದಳತೆ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ptr)) & 1) == 0) {
				hex2mem(ptr, remcom_Reg_ಬಫರ್, ಉದ್ದಳತೆ/2);
				kgdb_putreg(regs, addr,
					remcom_Reg_ಬಫರ್, ಉದ್ದಳತೆ/2);
				strcpy(remcom_ಹೊರಹೋಗು_ಬಫರ್,"OK");
			} ಇಲ್ಲದಿದ್ದರೆ {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			ನಿಲ್ಲಿಸು;
		}			/* switch */

		ಆದರೆ (errnum != 0)
			sprintf(remcom_ಹೊರಹೋಗು_ಬಫರ್, "E%02d", errnum);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ KGDB_DEBUG
		ಆದರೆ (kdebug)
			ಬರೆ("kgdb: remcomOutBuffer: %s\n", remcom_ಹೊರಹೋಗು_ಬಫರ್);
#ಆದರೆ_ಅಂತ್ಯ

		/* reply to the request */
		putpacket((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)&remcom_ಹೊರಹೋಗು_ಬಫರ್);

	} /* while(1) */
}

/*
 * kgdb_init must be called *after* the
 * monitor is relocated into ram
 */
ಪೂರ್ಣಾಂಕ kgdb_init(ಶೂನ್ಯ)
{
	puts("KGDB:  ");

	kgdb_serial_init();
	debugger_exception_handler = handle_exception;
	initialized = 1;

	putDebugStr("kgdb ready\n");
	puts("ready\n");

	ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ
kgdb_error(ಪೂರ್ಣಾಂಕ errnum)
{
	longjmp_on_fault = 0;
	kgdb_longjmp(error_jmp_buf, errnum);
	panic("kgdb_error: longjmp failed!\n");
}

/* Output string in GDB O-packet format if GDB has connected. If nothing
   output, returns 0 (caller must then handle output). */
ಪೂರ್ಣಾಂಕ
kgdb_output_string (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* s, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2)
{
	ಅಕ್ಷರ ಬಫರ್[512];

	ಎಣಿಕೆ_2 = (ಎಣಿಕೆ_2 <= (ಗಾತ್ರ(ಬಫರ್) / 2 - 2))
		? ಎಣಿಕೆ_2 : (ಗಾತ್ರ(ಬಫರ್) / 2 - 2);

	ಬಫರ್[0] = 'O';
	mem2hex ((ಅಕ್ಷರ *)s, &ಬಫರ್[1], ಎಣಿಕೆ_2);
	putpacket((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)&ಬಫರ್);

	ಹಿಂತಿರುಗಿ 1;
}

ಶೂನ್ಯ
breakpoint(ಶೂನ್ಯ)
{
	ಆದರೆ (!initialized) {
		ಬರೆ("breakpoint() called b4 kgdb init\n");
		ಹಿಂತಿರುಗಿ;
	}

	kgdb_breakpoint(0, 0);
}

ಪೂರ್ಣಾಂಕ
do_kgdb(ರಚನೆ cmd_tbl *cmdtp, ಪೂರ್ಣಾಂಕ flag, ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ argv[])
{
    ಬರೆ("Entering KGDB mode via exception handler...\n\n");
    kgdb_breakpoint(argc - 1, argv + 1);
    ಬರೆ("\nReturned from KGDB mode\n");
    ಹಿಂತಿರುಗಿ 0;
}

U_BOOT_CMD(
	kgdb, CONFIG_SYS_MAXARGS, 1,	do_kgdb,
	"enter gdb remote debug mode",
	"[arg0 arg1 .. argN]\n"
	"    - executes a breakpoint so that kgdb mode is\n"
	"      entered via the exception handler. To return\n"
	"      to the monitor, the remote gdb debugger must\n"
	"      execute a \"continue\" or \"quit\" command.\n"
	"\n"
	"      if a program is loaded by the remote gdb, any args\n"
	"      passed to the kgdb command are given to the loaded\n"
	"      program if it is executed (see the \"hello_world\"\n"
	"      example program in the U-Boot examples directory)."
);
