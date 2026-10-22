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

#അന്തര്ഭാവ <asm/ptrace.h>

#അന്തര്ഭാവ <kgdb.h>
#അന്തര്ഭാവ <command.h>

#undef KGDB_DEBUG

/*
 * BUFMAX defines the maximum number of characters in inbound/outbound buffers
 */
#define BUFMAX 1024
സ്ഥിതം അക്ഷരം remcomInBuffer[BUFMAX];
സ്ഥിതം അക്ഷരം remcomOutBuffer[BUFMAX];
സ്ഥിതം അക്ഷരം remcomRegBuffer[BUFMAX];

സ്ഥിതം പൂർണ്ണസംഖ്യ initialized = 0;
സ്ഥിതം പൂർണ്ണസംഖ്യ kgdb_active;
സ്ഥിതം ഘടന pt_regs entry_regs;
സ്ഥിതം long error_jmp_buf[BUFMAX/2];
സ്ഥിതം പൂർണ്ണസംഖ്യ longjmp_on_fault = 0;
#ifdef KGDB_DEBUG
സ്ഥിതം പൂർണ്ണസംഖ്യ kdebug = 1;
#endif

സ്ഥിതം സ്ഥിരം അക്ഷരം hexchars[]="0123456789abcdef";

/* Convert ch from a hex digit to an int */
സ്ഥിതം പൂർണ്ണസംഖ്യ
hex(unsigned അക്ഷരം ch)
{
	എങ്കിൽ (ch >= 'a' && ch <= 'f')
		തിരികെ ch-'a'+10;
	എങ്കിൽ (ch >= '0' && ch <= '9')
		തിരികെ ch-'0';
	എങ്കിൽ (ch >= 'A' && ch <= 'F')
		തിരികെ ch-'A'+10;
	തിരികെ -1;
}

/* Convert the memory pointed to by mem into hex, placing result in buf.
 * Return a pointer to the last char put in buf (null).
 */
സ്ഥിതം unsigned അക്ഷരം *
mem2hex(അക്ഷരം *mem, അക്ഷരം *buf, പൂർണ്ണസംഖ്യ എണ്ണം)
{
	അക്ഷരം *tmp;
	unsigned അക്ഷരം ch;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory copy.  Hex conversion will work against this one.
	 */
	tmp = buf + എണ്ണം;
	longjmp_on_fault = 1;

	memcpy(tmp, mem, എണ്ണം);

	ആകുമ്പോൾ (എണ്ണം-- > 0) {
		ch = *tmp++;
		*buf++ = hexchars[ch >> 4];
		*buf++ = hexchars[ch & 0xf];
	}
	*buf = 0;
	longjmp_on_fault = 0;
	തിരികെ (unsigned അക്ഷരം *)buf;
}

/* convert the hex array pointed to by buf into binary to be placed in mem
 * return a pointer to the character AFTER the last byte fetched from buf.
*/
സ്ഥിതം അക്ഷരം *
hex2mem(അക്ഷരം *buf, അക്ഷരം *mem, പൂർണ്ണസംഖ്യ എണ്ണം)
{
	പൂർണ്ണസംഖ്യ hex_മൂല്യം;
	അക്ഷരം *tmp_raw, *tmp_hex;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory that is converted from hex.
	 */
	tmp_raw = buf + എണ്ണം * 2;
	tmp_hex = tmp_raw - 1;

	longjmp_on_fault = 1;
	ആകുമ്പോൾ (tmp_hex >= buf) {
		tmp_raw--;
		hex_മൂല്യം = hex(*tmp_hex--);
		എങ്കിൽ (hex_മൂല്യം < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw = hex_മൂല്യം;
		hex_മൂല്യം = hex(*tmp_hex--);
		എങ്കിൽ (hex_മൂല്യം < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw |= hex_മൂല്യം << 4;

	}

	memcpy(mem, tmp_raw, എണ്ണം);

	kgdb_flush_cache_range((ശൂന്യം *)mem, (ശൂന്യം *)(mem+എണ്ണം));
	longjmp_on_fault = 0;

	തിരികെ buf;
}

/*
 * While we find nice hex chars, build an int.
 * Return number of chars processed.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ
hexToInt(അക്ഷരം **ptr, പൂർണ്ണസംഖ്യ *int_മൂല്യം)
{
	പൂർണ്ണസംഖ്യ അക്കം_Chars = 0;
	പൂർണ്ണസംഖ്യ hex_മൂല്യം;

	*int_മൂല്യം = 0;

	longjmp_on_fault = 1;
	ആകുമ്പോൾ (**ptr) {
		hex_മൂല്യം = hex(**ptr);
		എങ്കിൽ (hex_മൂല്യം < 0)
			നിർത്തുക;

		*int_മൂല്യം = (*int_മൂല്യം << 4) | hex_മൂല്യം;
		അക്കം_Chars ++;

		(*ptr)++;
	}
	longjmp_on_fault = 0;

	തിരികെ (അക്കം_Chars);
}

/* scan for the sequence $<data>#<checksum>     */
സ്ഥിതം ശൂന്യം
getpacket(അക്ഷരം *buffer)
{
	unsigned അക്ഷരം checksum;
	unsigned അക്ഷരം xmitcsum;
	പൂർണ്ണസംഖ്യ i;
	പൂർണ്ണസംഖ്യ എണ്ണം;
	unsigned അക്ഷരം ch;

	ചെയ്യുക {
		/* wait around for the start character, ignore all other
		 * characters */
		ആകുമ്പോൾ ((ch = (getDebugChar() & 0x7f)) != '$') {
#ifdef KGDB_DEBUG
			എങ്കിൽ (kdebug)
				putc(ch);
#endif
			;
		}

		checksum = 0;
		xmitcsum = -1;

		എണ്ണം = 0;

		/* now, read until a # or end of buffer is found */
		ആകുമ്പോൾ (എണ്ണം < BUFMAX) {
			ch = getDebugChar() & 0x7f;
			എങ്കിൽ (ch == '#')
				നിർത്തുക;
			checksum = checksum + ch;
			buffer[എണ്ണം] = ch;
			എണ്ണം = എണ്ണം + 1;
		}

		എങ്കിൽ (എണ്ണം >= BUFMAX)
			തുടരുക;

		buffer[എണ്ണം] = 0;

		എങ്കിൽ (ch == '#') {
			xmitcsum = hex(getDebugChar() & 0x7f) << 4;
			xmitcsum |= hex(getDebugChar() & 0x7f);
			എങ്കിൽ (checksum != xmitcsum)
				putDebugChar('-');	/* failed checksum */
			അല്ലെങ്കിൽ {
				putDebugChar('+'); /* successful transfer */
				/* if a sequence char is present, reply the ID */
				എങ്കിൽ (buffer[2] == ':') {
					putDebugChar(buffer[0]);
					putDebugChar(buffer[1]);
					/* remove sequence chars from buffer */
					എണ്ണം = strlen(buffer);
					വേണ്ടി (i=3; i <= എണ്ണം; i++)
						buffer[i-3] = buffer[i];
				}
			}
		}
	} ആകുമ്പോൾ (checksum != xmitcsum);
}

/* send the packet in buffer.  */
സ്ഥിതം ശൂന്യം
putpacket(unsigned അക്ഷരം *buffer)
{
	unsigned അക്ഷരം checksum;
	പൂർണ്ണസംഖ്യ എണ്ണം;
	unsigned അക്ഷരം ch, recv;

	/*  $<packet info>#<checksum>. */
	ചെയ്യുക {
		putDebugChar('$');
		checksum = 0;
		എണ്ണം = 0;

		ആകുമ്പോൾ ((ch = buffer[എണ്ണം])) {
			putDebugChar(ch);
			checksum += ch;
			എണ്ണം += 1;
		}

		putDebugChar('#');
		putDebugChar(hexchars[checksum >> 4]);
		putDebugChar(hexchars[checksum & 0xf]);
		recv = getDebugChar();
	} ആകുമ്പോൾ ((recv & 0x7f) != '+');
}

/*
 * This function does all command processing for interfacing to gdb.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ
handle_exception (ഘടന pt_regs *regs)
{
	പൂർണ്ണസംഖ്യ addr;
	പൂർണ്ണസംഖ്യ length;
	അക്ഷരം *ptr;
	kgdb_data kd;
	പൂർണ്ണസംഖ്യ i;

	എങ്കിൽ (!initialized) {
		printf("kgdb: exception before kgdb is initialized! huh?\n");
		തിരികെ (0);
	}

	/* probably should check which exception occurred as well */
	എങ്കിൽ (longjmp_on_fault) {
		longjmp_on_fault = 0;
		kgdb_longjmp(error_jmp_buf, KGDBERR_MEMFAULT);
		panic("kgdb longjump failed!\n");
	}

	എങ്കിൽ (kgdb_active) {
		printf("kgdb: unexpected exception from within kgdb\n");
		തിരികെ (0);
	}
	kgdb_active = 1;

	kgdb_interruptible(0);

	printf("kgdb: handle_exception; trap [0x%x]\n", kgdb_trap(regs));

	എങ്കിൽ (kgdb_setjmp(error_jmp_buf) != 0)
		panic("kgdb: error or fault in entry init!\n");

	kgdb_enter(regs, &kd);

	entry_regs = *regs;

	ptr = remcomOutBuffer;

	*ptr++ = 'T';

	*ptr++ = hexchars[kd.sigval >> 4];
	*ptr++ = hexchars[kd.sigval & 0xf];

	വേണ്ടി (i = 0; i < kd.nregs; i++) {
		kgdb_reg *rp = &kd.regs[i];

		*ptr++ = hexchars[rp->അക്കം >> 4];
		*ptr++ = hexchars[rp->അക്കം & 0xf];
		*ptr++ = ':';
		ptr = (അക്ഷരം *)mem2hex((അക്ഷരം *)&rp->val, ptr, 4);
		*ptr++ = ';';
	}

	*ptr = 0;

#ifdef KGDB_DEBUG
	എങ്കിൽ (kdebug)
		printf("kgdb: remcomOutBuffer: %s\n", remcomOutBuffer);
#endif

	putpacket((unsigned അക്ഷരം *)&remcomOutBuffer);

	ആകുമ്പോൾ (1) {
		അസ്ഥിര പൂർണ്ണസംഖ്യ errnum;

		remcomOutBuffer[0] = 0;

		getpacket(remcomInBuffer);
		ptr = &remcomInBuffer[1];

#ifdef KGDB_DEBUG
		എങ്കിൽ (kdebug)
			printf("kgdb:  remcomInBuffer: %s\n", remcomInBuffer);
#endif

		errnum = kgdb_setjmp(error_jmp_buf);

		എങ്കിൽ (errnum == 0) തിരഞ്ഞെടുപ്പ് (remcomInBuffer[0]) {

		അവസ്ഥ '?':               /* report most recent signal */
			remcomOutBuffer[0] = 'S';
			remcomOutBuffer[1] = hexchars[kd.sigval >> 4];
			remcomOutBuffer[2] = hexchars[kd.sigval & 0xf];
			remcomOutBuffer[3] = 0;
			നിർത്തുക;

#ifdef KGDB_DEBUG
		അവസ്ഥ 'd':
			/* toggle debug flag */
			kdebug ^= 1;
			നിർത്തുക;
#endif

		അവസ്ഥ 'g':	/* return the value of the CPU registers. */
			length = kgdb_getregs(regs, remcomRegBuffer, BUFMAX);
			mem2hex(remcomRegBuffer, remcomOutBuffer, length);
			നിർത്തുക;

		അവസ്ഥ 'G':   /* set the value of the CPU registers */
			length = strlen(ptr);
			എങ്കിൽ ((length & 1) != 0) kgdb_error(KGDBERR_BADPARAMS);
			hex2mem(ptr, remcomRegBuffer, length/2);
			kgdb_putregs(regs, remcomRegBuffer, length/2);
			strcpy(remcomOutBuffer,"OK");
			നിർത്തുക;

		അവസ്ഥ 'm':	/* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
				/* Try to read %x,%x.  */

			എങ്കിൽ (hexToInt(&ptr, &addr)
			    && *ptr++ == ','
			    && hexToInt(&ptr, &length))	{
				mem2hex((അക്ഷരം *)addr, remcomOutBuffer, length);
			} അല്ലെങ്കിൽ {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			നിർത്തുക;

		അവസ്ഥ 'M': /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
			/* Try to read '%x,%x:'.  */

			എങ്കിൽ (hexToInt(&ptr, &addr)
			    && *ptr++ == ','
			    && hexToInt(&ptr, &length)
			    && *ptr++ == ':') {
				hex2mem(ptr, (അക്ഷരം *)addr, length);
				strcpy(remcomOutBuffer, "OK");
			} അല്ലെങ്കിൽ {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			നിർത്തുക;

		അവസ്ഥ 'k':    /* kill the program, actually return to monitor */
			kd.extype = KGDBEXIT_KILL;
			*regs = entry_regs;
			പോകുക doexit;

		അവസ്ഥ 'C':    /* CSS  continue with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		അവസ്ഥ 'c':    /* cAA..AA  Continue; address AA..AA optional */
			/* try to read optional parameter, pc unchanged if no parm */
			kd.extype = KGDBEXIT_CONTINUE;

			എങ്കിൽ (hexToInt(&ptr, &addr)) {
				kd.exaddr = addr;
				kd.extype |= KGDBEXIT_WITHADDR;
			}

			പോകുക doexit;

		അവസ്ഥ 'S':    /* SSS  single step with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		അവസ്ഥ 's':
			kd.extype = KGDBEXIT_SINGLE;

			എങ്കിൽ (hexToInt(&ptr, &addr)) {
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
			kgdb_active = 0;
			kgdb_interruptible(1);
			തിരികെ (1);

		അവസ്ഥ 'r':		/* Reset (if user process..exit ???)*/
			panic("kgdb reset.");
			നിർത്തുക;

		അവസ്ഥ 'P':    /* Pr=v  set reg r to value v (r and v are hex) */
			എങ്കിൽ (hexToInt(&ptr, &addr)
			    && *ptr++ == '='
			    && ((length = strlen(ptr)) & 1) == 0) {
				hex2mem(ptr, remcomRegBuffer, length/2);
				kgdb_putreg(regs, addr,
					remcomRegBuffer, length/2);
				strcpy(remcomOutBuffer,"OK");
			} അല്ലെങ്കിൽ {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			നിർത്തുക;
		}			/* switch */

		എങ്കിൽ (errnum != 0)
			sprintf(remcomOutBuffer, "E%02d", errnum);

#ifdef KGDB_DEBUG
		എങ്കിൽ (kdebug)
			printf("kgdb: remcomOutBuffer: %s\n", remcomOutBuffer);
#endif

		/* reply to the request */
		putpacket((unsigned അക്ഷരം *)&remcomOutBuffer);

	} /* while(1) */
}

/*
 * kgdb_init must be called *after* the
 * monitor is relocated into ram
 */
പൂർണ്ണസംഖ്യ kgdb_init(ശൂന്യം)
{
	puts("KGDB:  ");

	kgdb_serial_init();
	debugger_exception_handler = handle_exception;
	initialized = 1;

	putDebugStr("kgdb ready\n");
	puts("ready\n");

	തിരികെ 0;
}

ശൂന്യം
kgdb_error(പൂർണ്ണസംഖ്യ errnum)
{
	longjmp_on_fault = 0;
	kgdb_longjmp(error_jmp_buf, errnum);
	panic("kgdb_error: longjmp failed!\n");
}

/* Output string in GDB O-packet format if GDB has connected. If nothing
   output, returns 0 (caller must then handle output). */
പൂർണ്ണസംഖ്യ
kgdb_output_string (സ്ഥിരം അക്ഷരം* s, unsigned പൂർണ്ണസംഖ്യ എണ്ണം)
{
	അക്ഷരം buffer[512];

	എണ്ണം = (എണ്ണം <= (വലുപ്പം(buffer) / 2 - 2))
		? എണ്ണം : (വലുപ്പം(buffer) / 2 - 2);

	buffer[0] = 'O';
	mem2hex ((അക്ഷരം *)s, &buffer[1], എണ്ണം);
	putpacket((unsigned അക്ഷരം *)&buffer);

	തിരികെ 1;
}

ശൂന്യം
breakpoint(ശൂന്യം)
{
	എങ്കിൽ (!initialized) {
		printf("breakpoint() called b4 kgdb init\n");
		തിരികെ;
	}

	kgdb_breakpoint(0, 0);
}

പൂർണ്ണസംഖ്യ
do_kgdb(ഘടന cmd_tbl *cmdtp, പൂർണ്ണസംഖ്യ flag, പൂർണ്ണസംഖ്യ argc, അക്ഷരം *സ്ഥിരം argv[])
{
    printf("Entering KGDB mode via exception handler...\n\n");
    kgdb_breakpoint(argc - 1, argv + 1);
    printf("\nReturned from KGDB mode\n");
    തിരികെ 0;
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
