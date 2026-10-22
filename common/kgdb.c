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

#ਅਨ੍ਤਰ੍ਭਾਵ <asm/ptrace.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <kgdb.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <command.h>

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ KGDB_DEBUG

/*
 * BUFMAX defines the maximum number of characters in inbound/outbound buffers
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUFMAX 1024
ਸਥਿਰ ਅੱਖਰ remcom_ਵਿੱਚ_ਬਫਰ[BUFMAX];
ਸਥਿਰ ਅੱਖਰ remcom_ਬਾਹਰਲਾ_ਬਫਰ[BUFMAX];
ਸਥਿਰ ਅੱਖਰ remcom_Reg_ਬਫਰ[BUFMAX];

ਸਥਿਰ ਪੂਰਨ initialized = 0;
ਸਥਿਰ ਪੂਰਨ kgdb_ਸਰਗਰਮ;
ਸਥਿਰ ਬਣਤਰ pt_regs ਇੰਦਰਾਜ_regs;
ਸਥਿਰ ਲੰਮਾ error_jmp_buf[BUFMAX/2];
ਸਥਿਰ ਪੂਰਨ longjmp_on_fault = 0;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ KGDB_DEBUG
ਸਥਿਰ ਪੂਰਨ kdebug = 1;
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਅਚਲ ਅੱਖਰ hexchars[]="0123456789abcdef";

/* Convert ch from a hex digit to an int */
ਸਥਿਰ ਪੂਰਨ
hex(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ch)
{
	ਜੇ (ch >= 'a' && ch <= 'f')
		ਵਾਪਸ ch-'a'+10;
	ਜੇ (ch >= '0' && ch <= '9')
		ਵਾਪਸ ch-'0';
	ਜੇ (ch >= 'A' && ch <= 'F')
		ਵਾਪਸ ch-'A'+10;
	ਵਾਪਸ -1;
}

/* Convert the memory pointed to by mem into hex, placing result in buf.
 * Return a pointer to the last char put in buf (null).
 */
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *
mem2hex(ਅੱਖਰ *mem, ਅੱਖਰ *buf, ਪੂਰਨ ਗਿਣਤੀ_2)
{
	ਅੱਖਰ *tmp;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ch;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory copy.  Hex conversion will work against this one.
	 */
	tmp = buf + ਗਿਣਤੀ_2;
	longjmp_on_fault = 1;

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(tmp, mem, ਗਿਣਤੀ_2);

	ਜਦੋਂ (ਗਿਣਤੀ_2-- > 0) {
		ch = *tmp++;
		*buf++ = hexchars[ch >> 4];
		*buf++ = hexchars[ch & 0xf];
	}
	*buf = 0;
	longjmp_on_fault = 0;
	ਵਾਪਸ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)buf;
}

/* convert the hex array pointed to by buf into binary to be placed in mem
 * return a pointer to the character AFTER the last byte fetched from buf.
*/
ਸਥਿਰ ਅੱਖਰ *
hex2mem(ਅੱਖਰ *buf, ਅੱਖਰ *mem, ਪੂਰਨ ਗਿਣਤੀ_2)
{
	ਪੂਰਨ hex_ਮੁੱਲ;
	ਅੱਖਰ *tmp_raw, *tmp_hex;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory that is converted from hex.
	 */
	tmp_raw = buf + ਗਿਣਤੀ_2 * 2;
	tmp_hex = tmp_raw - 1;

	longjmp_on_fault = 1;
	ਜਦੋਂ (tmp_hex >= buf) {
		tmp_raw--;
		hex_ਮੁੱਲ = hex(*tmp_hex--);
		ਜੇ (hex_ਮੁੱਲ < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw = hex_ਮੁੱਲ;
		hex_ਮੁੱਲ = hex(*tmp_hex--);
		ਜੇ (hex_ਮੁੱਲ < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw |= hex_ਮੁੱਲ << 4;

	}

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(mem, tmp_raw, ਗਿਣਤੀ_2);

	kgdb_flush_cache_range((ਖਾਲੀ *)mem, (ਖਾਲੀ *)(mem+ਗਿਣਤੀ_2));
	longjmp_on_fault = 0;

	ਵਾਪਸ buf;
}

/*
 * While we find nice hex chars, build an int.
 * Return number of chars processed.
 */
ਸਥਿਰ ਪੂਰਨ
hex_ਮੰਜ਼ਿਲ_Int(ਅੱਖਰ **ptr, ਪੂਰਨ *int_ਮੁੱਲ)
{
	ਪੂਰਨ ਅੰਕ_Chars = 0;
	ਪੂਰਨ hex_ਮੁੱਲ;

	*int_ਮੁੱਲ = 0;

	longjmp_on_fault = 1;
	ਜਦੋਂ (**ptr) {
		hex_ਮੁੱਲ = hex(**ptr);
		ਜੇ (hex_ਮੁੱਲ < 0)
			ਰੋਕੋ;

		*int_ਮੁੱਲ = (*int_ਮੁੱਲ << 4) | hex_ਮੁੱਲ;
		ਅੰਕ_Chars ++;

		(*ptr)++;
	}
	longjmp_on_fault = 0;

	ਵਾਪਸ (ਅੰਕ_Chars);
}

/* scan for the sequence $<data>#<checksum>     */
ਸਥਿਰ ਖਾਲੀ
getpacket(ਅੱਖਰ *ਬਫਰ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ checksum;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ xmitcsum;
	ਪੂਰਨ i;
	ਪੂਰਨ ਗਿਣਤੀ_2;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ch;

	ਕਰੋ {
		/* wait around for the start character, ignore all other
		 * characters */
		ਜਦੋਂ ((ch = (getDebugChar() & 0x7f)) != '$') {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ KGDB_DEBUG
			ਜੇ (kdebug)
				putc(ch);
#ਜੇ_ਅੰਤ
			;
		}

		checksum = 0;
		xmitcsum = -1;

		ਗਿਣਤੀ_2 = 0;

		/* now, read until a # or end of buffer is found */
		ਜਦੋਂ (ਗਿਣਤੀ_2 < BUFMAX) {
			ch = getDebugChar() & 0x7f;
			ਜੇ (ch == '#')
				ਰੋਕੋ;
			checksum = checksum + ch;
			ਬਫਰ[ਗਿਣਤੀ_2] = ch;
			ਗਿਣਤੀ_2 = ਗਿਣਤੀ_2 + 1;
		}

		ਜੇ (ਗਿਣਤੀ_2 >= BUFMAX)
			ਜਾਰੀ;

		ਬਫਰ[ਗਿਣਤੀ_2] = 0;

		ਜੇ (ch == '#') {
			xmitcsum = hex(getDebugChar() & 0x7f) << 4;
			xmitcsum |= hex(getDebugChar() & 0x7f);
			ਜੇ (checksum != xmitcsum)
				putDebugChar('-');	/* failed checksum */
			ਨਹੀਂਤਾਂ {
				putDebugChar('+'); /* successful transfer */
				/* if a sequence char is present, reply the ID */
				ਜੇ (ਬਫਰ[2] == ':') {
					putDebugChar(ਬਫਰ[0]);
					putDebugChar(ਬਫਰ[1]);
					/* remove sequence chars from buffer */
					ਗਿਣਤੀ_2 = ਲੜੀ_ਲੰਬਾਈ(ਬਫਰ);
					ਲਈ (i=3; i <= ਗਿਣਤੀ_2; i++)
						ਬਫਰ[i-3] = ਬਫਰ[i];
				}
			}
		}
	} ਜਦੋਂ (checksum != xmitcsum);
}

/* send the packet in buffer.  */
ਸਥਿਰ ਖਾਲੀ
putpacket(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਬਫਰ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ checksum;
	ਪੂਰਨ ਗਿਣਤੀ_2;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ch, recv;

	/*  $<packet info>#<checksum>. */
	ਕਰੋ {
		putDebugChar('$');
		checksum = 0;
		ਗਿਣਤੀ_2 = 0;

		ਜਦੋਂ ((ch = ਬਫਰ[ਗਿਣਤੀ_2])) {
			putDebugChar(ch);
			checksum += ch;
			ਗਿਣਤੀ_2 += 1;
		}

		putDebugChar('#');
		putDebugChar(hexchars[checksum >> 4]);
		putDebugChar(hexchars[checksum & 0xf]);
		recv = getDebugChar();
	} ਜਦੋਂ ((recv & 0x7f) != '+');
}

/*
 * This function does all command processing for interfacing to gdb.
 */
ਸਥਿਰ ਪੂਰਨ
handle_exception (ਬਣਤਰ pt_regs *regs)
{
	ਪੂਰਨ addr;
	ਪੂਰਨ ਲੰਮਾਈ;
	ਅੱਖਰ *ptr;
	kgdb_ਅੰਕੜੇ kd;
	ਪੂਰਨ i;

	ਜੇ (!initialized) {
		ਲਿਖੋ("kgdb: exception before kgdb is initialized! huh?\n");
		ਵਾਪਸ (0);
	}

	/* probably should check which exception occurred as well */
	ਜੇ (longjmp_on_fault) {
		longjmp_on_fault = 0;
		kgdb_longjmp(error_jmp_buf, KGDBERR_MEMFAULT);
		panic("kgdb longjump failed!\n");
	}

	ਜੇ (kgdb_ਸਰਗਰਮ) {
		ਲਿਖੋ("kgdb: unexpected exception from within kgdb\n");
		ਵਾਪਸ (0);
	}
	kgdb_ਸਰਗਰਮ = 1;

	kgdb_interruptible(0);

	ਲਿਖੋ("kgdb: handle_exception; trap [0x%x]\n", kgdb_trap(regs));

	ਜੇ (kgdb_setjmp(error_jmp_buf) != 0)
		panic("kgdb: error or fault in entry init!\n");

	kgdb_enter(regs, &kd);

	ਇੰਦਰਾਜ_regs = *regs;

	ptr = remcom_ਬਾਹਰਲਾ_ਬਫਰ;

	*ptr++ = 'T';

	*ptr++ = hexchars[kd.sigval >> 4];
	*ptr++ = hexchars[kd.sigval & 0xf];

	ਲਈ (i = 0; i < kd.nregs; i++) {
		kgdb_reg *rp = &kd.regs[i];

		*ptr++ = hexchars[rp->ਅੰਕ >> 4];
		*ptr++ = hexchars[rp->ਅੰਕ & 0xf];
		*ptr++ = ':';
		ptr = (ਅੱਖਰ *)mem2hex((ਅੱਖਰ *)&rp->val, ptr, 4);
		*ptr++ = ';';
	}

	*ptr = 0;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ KGDB_DEBUG
	ਜੇ (kdebug)
		ਲਿਖੋ("kgdb: remcomOutBuffer: %s\n", remcom_ਬਾਹਰਲਾ_ਬਫਰ);
#ਜੇ_ਅੰਤ

	putpacket((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)&remcom_ਬਾਹਰਲਾ_ਬਫਰ);

	ਜਦੋਂ (1) {
		ਅਸਥਿਰ ਪੂਰਨ errnum;

		remcom_ਬਾਹਰਲਾ_ਬਫਰ[0] = 0;

		getpacket(remcom_ਵਿੱਚ_ਬਫਰ);
		ptr = &remcom_ਵਿੱਚ_ਬਫਰ[1];

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ KGDB_DEBUG
		ਜੇ (kdebug)
			ਲਿਖੋ("kgdb:  remcomInBuffer: %s\n", remcom_ਵਿੱਚ_ਬਫਰ);
#ਜੇ_ਅੰਤ

		errnum = kgdb_setjmp(error_jmp_buf);

		ਜੇ (errnum == 0) ਚੋਣ (remcom_ਵਿੱਚ_ਬਫਰ[0]) {

		ਹਾਲਤ '?':               /* report most recent signal */
			remcom_ਬਾਹਰਲਾ_ਬਫਰ[0] = 'S';
			remcom_ਬਾਹਰਲਾ_ਬਫਰ[1] = hexchars[kd.sigval >> 4];
			remcom_ਬਾਹਰਲਾ_ਬਫਰ[2] = hexchars[kd.sigval & 0xf];
			remcom_ਬਾਹਰਲਾ_ਬਫਰ[3] = 0;
			ਰੋਕੋ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ KGDB_DEBUG
		ਹਾਲਤ 'd':
			/* toggle debug flag */
			kdebug ^= 1;
			ਰੋਕੋ;
#ਜੇ_ਅੰਤ

		ਹਾਲਤ 'g':	/* return the value of the CPU registers. */
			ਲੰਮਾਈ = kgdb_getregs(regs, remcom_Reg_ਬਫਰ, BUFMAX);
			mem2hex(remcom_Reg_ਬਫਰ, remcom_ਬਾਹਰਲਾ_ਬਫਰ, ਲੰਮਾਈ);
			ਰੋਕੋ;

		ਹਾਲਤ 'G':   /* set the value of the CPU registers */
			ਲੰਮਾਈ = ਲੜੀ_ਲੰਬਾਈ(ptr);
			ਜੇ ((ਲੰਮਾਈ & 1) != 0) kgdb_error(KGDBERR_BADPARAMS);
			hex2mem(ptr, remcom_Reg_ਬਫਰ, ਲੰਮਾਈ/2);
			kgdb_putregs(regs, remcom_Reg_ਬਫਰ, ਲੰਮਾਈ/2);
			strcpy(remcom_ਬਾਹਰਲਾ_ਬਫਰ,"OK");
			ਰੋਕੋ;

		ਹਾਲਤ 'm':	/* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
				/* Try to read %x,%x.  */

			ਜੇ (hex_ਮੰਜ਼ਿਲ_Int(&ptr, &addr)
			    && *ptr++ == ','
			    && hex_ਮੰਜ਼ਿਲ_Int(&ptr, &ਲੰਮਾਈ))	{
				mem2hex((ਅੱਖਰ *)addr, remcom_ਬਾਹਰਲਾ_ਬਫਰ, ਲੰਮਾਈ);
			} ਨਹੀਂਤਾਂ {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			ਰੋਕੋ;

		ਹਾਲਤ 'M': /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
			/* Try to read '%x,%x:'.  */

			ਜੇ (hex_ਮੰਜ਼ਿਲ_Int(&ptr, &addr)
			    && *ptr++ == ','
			    && hex_ਮੰਜ਼ਿਲ_Int(&ptr, &ਲੰਮਾਈ)
			    && *ptr++ == ':') {
				hex2mem(ptr, (ਅੱਖਰ *)addr, ਲੰਮਾਈ);
				strcpy(remcom_ਬਾਹਰਲਾ_ਬਫਰ, "OK");
			} ਨਹੀਂਤਾਂ {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			ਰੋਕੋ;

		ਹਾਲਤ 'k':    /* kill the program, actually return to monitor */
			kd.extype = KGDBEXIT_KILL;
			*regs = ਇੰਦਰਾਜ_regs;
			ਜਾਓ doexit;

		ਹਾਲਤ 'C':    /* CSS  continue with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		ਹਾਲਤ 'c':    /* cAA..AA  Continue; address AA..AA optional */
			/* try to read optional parameter, pc unchanged if no parm */
			kd.extype = KGDBEXIT_CONTINUE;

			ਜੇ (hex_ਮੰਜ਼ਿਲ_Int(&ptr, &addr)) {
				kd.exaddr = addr;
				kd.extype |= KGDBEXIT_WITHADDR;
			}

			ਜਾਓ doexit;

		ਹਾਲਤ 'S':    /* SSS  single step with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		ਹਾਲਤ 's':
			kd.extype = KGDBEXIT_SINGLE;

			ਜੇ (hex_ਮੰਜ਼ਿਲ_Int(&ptr, &addr)) {
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
			kgdb_ਸਰਗਰਮ = 0;
			kgdb_interruptible(1);
			ਵਾਪਸ (1);

		ਹਾਲਤ 'r':		/* Reset (if user process..exit ???)*/
			panic("kgdb reset.");
			ਰੋਕੋ;

		ਹਾਲਤ 'P':    /* Pr=v  set reg r to value v (r and v are hex) */
			ਜੇ (hex_ਮੰਜ਼ਿਲ_Int(&ptr, &addr)
			    && *ptr++ == '='
			    && ((ਲੰਮਾਈ = ਲੜੀ_ਲੰਬਾਈ(ptr)) & 1) == 0) {
				hex2mem(ptr, remcom_Reg_ਬਫਰ, ਲੰਮਾਈ/2);
				kgdb_putreg(regs, addr,
					remcom_Reg_ਬਫਰ, ਲੰਮਾਈ/2);
				strcpy(remcom_ਬਾਹਰਲਾ_ਬਫਰ,"OK");
			} ਨਹੀਂਤਾਂ {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			ਰੋਕੋ;
		}			/* switch */

		ਜੇ (errnum != 0)
			sprintf(remcom_ਬਾਹਰਲਾ_ਬਫਰ, "E%02d", errnum);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ KGDB_DEBUG
		ਜੇ (kdebug)
			ਲਿਖੋ("kgdb: remcomOutBuffer: %s\n", remcom_ਬਾਹਰਲਾ_ਬਫਰ);
#ਜੇ_ਅੰਤ

		/* reply to the request */
		putpacket((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)&remcom_ਬਾਹਰਲਾ_ਬਫਰ);

	} /* while(1) */
}

/*
 * kgdb_init must be called *after* the
 * monitor is relocated into ram
 */
ਪੂਰਨ kgdb_init(ਖਾਲੀ)
{
	puts("KGDB:  ");

	kgdb_serial_init();
	debugger_exception_handler = handle_exception;
	initialized = 1;

	putDebugStr("kgdb ready\n");
	puts("ready\n");

	ਵਾਪਸ 0;
}

ਖਾਲੀ
kgdb_error(ਪੂਰਨ errnum)
{
	longjmp_on_fault = 0;
	kgdb_longjmp(error_jmp_buf, errnum);
	panic("kgdb_error: longjmp failed!\n");
}

/* Output string in GDB O-packet format if GDB has connected. If nothing
   output, returns 0 (caller must then handle output). */
ਪੂਰਨ
kgdb_output_string (ਅਚਲ ਅੱਖਰ* s, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਗਿਣਤੀ_2)
{
	ਅੱਖਰ ਬਫਰ[512];

	ਗਿਣਤੀ_2 = (ਗਿਣਤੀ_2 <= (ਆਕਾਰ(ਬਫਰ) / 2 - 2))
		? ਗਿਣਤੀ_2 : (ਆਕਾਰ(ਬਫਰ) / 2 - 2);

	ਬਫਰ[0] = 'O';
	mem2hex ((ਅੱਖਰ *)s, &ਬਫਰ[1], ਗਿਣਤੀ_2);
	putpacket((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)&ਬਫਰ);

	ਵਾਪਸ 1;
}

ਖਾਲੀ
breakpoint(ਖਾਲੀ)
{
	ਜੇ (!initialized) {
		ਲਿਖੋ("breakpoint() called b4 kgdb init\n");
		ਵਾਪਸ;
	}

	kgdb_breakpoint(0, 0);
}

ਪੂਰਨ
do_kgdb(ਬਣਤਰ cmd_tbl *cmdtp, ਪੂਰਨ flag, ਪੂਰਨ argc, ਅੱਖਰ *ਅਚਲ argv[])
{
    ਲਿਖੋ("Entering KGDB mode via exception handler...\n\n");
    kgdb_breakpoint(argc - 1, argv + 1);
    ਲਿਖੋ("\nReturned from KGDB mode\n");
    ਵਾਪਸ 0;
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
