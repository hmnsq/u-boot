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

#અન્તર્ભાવ <asm/ptrace.h>

#અન્તર્ભાવ <kgdb.h>
#અન્તર્ભાવ <command.h>

#undef KGDB_DEBUG

/*
 * BUFMAX defines the maximum number of characters in inbound/outbound buffers
 */
#define BUFMAX 1024
સ્થિર અક્ષર remcomInBuffer[BUFMAX];
સ્થિર અક્ષર remcomOutBuffer[BUFMAX];
સ્થિર અક્ષર remcomRegBuffer[BUFMAX];

સ્થિર પૂર્ણાંક initialized = 0;
સ્થિર પૂર્ણાંક kgdb_active;
સ્થિર સંરચના pt_regs entry_regs;
સ્થિર long error_jmp_buf[BUFMAX/2];
સ્થિર પૂર્ણાંક longjmp_on_fault = 0;
#ifdef KGDB_DEBUG
સ્થિર પૂર્ણાંક kdebug = 1;
#endif

સ્થિર અચલ અક્ષર hexchars[]="0123456789abcdef";

/* Convert ch from a hex digit to an int */
સ્થિર પૂર્ણાંક
hex(unsigned અક્ષર ch)
{
	જો (ch >= 'a' && ch <= 'f')
		પાછા ch-'a'+10;
	જો (ch >= '0' && ch <= '9')
		પાછા ch-'0';
	જો (ch >= 'A' && ch <= 'F')
		પાછા ch-'A'+10;
	પાછા -1;
}

/* Convert the memory pointed to by mem into hex, placing result in buf.
 * Return a pointer to the last char put in buf (null).
 */
સ્થિર unsigned અક્ષર *
mem2hex(અક્ષર *mem, અક્ષર *buf, પૂર્ણાંક ગણતરી)
{
	અક્ષર *tmp;
	unsigned અક્ષર ch;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory copy.  Hex conversion will work against this one.
	 */
	tmp = buf + ગણતરી;
	longjmp_on_fault = 1;

	memcpy(tmp, mem, ગણતરી);

	જ્યારે (ગણતરી-- > 0) {
		ch = *tmp++;
		*buf++ = hexchars[ch >> 4];
		*buf++ = hexchars[ch & 0xf];
	}
	*buf = 0;
	longjmp_on_fault = 0;
	પાછા (unsigned અક્ષર *)buf;
}

/* convert the hex array pointed to by buf into binary to be placed in mem
 * return a pointer to the character AFTER the last byte fetched from buf.
*/
સ્થિર અક્ષર *
hex2mem(અક્ષર *buf, અક્ષર *mem, પૂર્ણાંક ગણતરી)
{
	પૂર્ણાંક hex_કિંમત;
	અક્ષર *tmp_raw, *tmp_hex;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory that is converted from hex.
	 */
	tmp_raw = buf + ગણતરી * 2;
	tmp_hex = tmp_raw - 1;

	longjmp_on_fault = 1;
	જ્યારે (tmp_hex >= buf) {
		tmp_raw--;
		hex_કિંમત = hex(*tmp_hex--);
		જો (hex_કિંમત < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw = hex_કિંમત;
		hex_કિંમત = hex(*tmp_hex--);
		જો (hex_કિંમત < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw |= hex_કિંમત << 4;

	}

	memcpy(mem, tmp_raw, ગણતરી);

	kgdb_flush_cache_range((શૂન્ય *)mem, (શૂન્ય *)(mem+ગણતરી));
	longjmp_on_fault = 0;

	પાછા buf;
}

/*
 * While we find nice hex chars, build an int.
 * Return number of chars processed.
 */
સ્થિર પૂર્ણાંક
hexToInt(અક્ષર **ptr, પૂર્ણાંક *int_કિંમત)
{
	પૂર્ણાંક અંક_Chars = 0;
	પૂર્ણાંક hex_કિંમત;

	*int_કિંમત = 0;

	longjmp_on_fault = 1;
	જ્યારે (**ptr) {
		hex_કિંમત = hex(**ptr);
		જો (hex_કિંમત < 0)
			તોડો;

		*int_કિંમત = (*int_કિંમત << 4) | hex_કિંમત;
		અંક_Chars ++;

		(*ptr)++;
	}
	longjmp_on_fault = 0;

	પાછા (અંક_Chars);
}

/* scan for the sequence $<data>#<checksum>     */
સ્થિર શૂન્ય
getpacket(અક્ષર *buffer)
{
	unsigned અક્ષર checksum;
	unsigned અક્ષર xmitcsum;
	પૂર્ણાંક i;
	પૂર્ણાંક ગણતરી;
	unsigned અક્ષર ch;

	કરો {
		/* wait around for the start character, ignore all other
		 * characters */
		જ્યારે ((ch = (getDebugChar() & 0x7f)) != '$') {
#ifdef KGDB_DEBUG
			જો (kdebug)
				putc(ch);
#endif
			;
		}

		checksum = 0;
		xmitcsum = -1;

		ગણતરી = 0;

		/* now, read until a # or end of buffer is found */
		જ્યારે (ગણતરી < BUFMAX) {
			ch = getDebugChar() & 0x7f;
			જો (ch == '#')
				તોડો;
			checksum = checksum + ch;
			buffer[ગણતરી] = ch;
			ગણતરી = ગણતરી + 1;
		}

		જો (ગણતરી >= BUFMAX)
			ચાલુ;

		buffer[ગણતરી] = 0;

		જો (ch == '#') {
			xmitcsum = hex(getDebugChar() & 0x7f) << 4;
			xmitcsum |= hex(getDebugChar() & 0x7f);
			જો (checksum != xmitcsum)
				putDebugChar('-');	/* failed checksum */
			નહીંતર {
				putDebugChar('+'); /* successful transfer */
				/* if a sequence char is present, reply the ID */
				જો (buffer[2] == ':') {
					putDebugChar(buffer[0]);
					putDebugChar(buffer[1]);
					/* remove sequence chars from buffer */
					ગણતરી = strlen(buffer);
					માટે (i=3; i <= ગણતરી; i++)
						buffer[i-3] = buffer[i];
				}
			}
		}
	} જ્યારે (checksum != xmitcsum);
}

/* send the packet in buffer.  */
સ્થિર શૂન્ય
putpacket(unsigned અક્ષર *buffer)
{
	unsigned અક્ષર checksum;
	પૂર્ણાંક ગણતરી;
	unsigned અક્ષર ch, recv;

	/*  $<packet info>#<checksum>. */
	કરો {
		putDebugChar('$');
		checksum = 0;
		ગણતરી = 0;

		જ્યારે ((ch = buffer[ગણતરી])) {
			putDebugChar(ch);
			checksum += ch;
			ગણતરી += 1;
		}

		putDebugChar('#');
		putDebugChar(hexchars[checksum >> 4]);
		putDebugChar(hexchars[checksum & 0xf]);
		recv = getDebugChar();
	} જ્યારે ((recv & 0x7f) != '+');
}

/*
 * This function does all command processing for interfacing to gdb.
 */
સ્થિર પૂર્ણાંક
handle_exception (સંરચના pt_regs *regs)
{
	પૂર્ણાંક addr;
	પૂર્ણાંક length;
	અક્ષર *ptr;
	kgdb_data kd;
	પૂર્ણાંક i;

	જો (!initialized) {
		printf("kgdb: exception before kgdb is initialized! huh?\n");
		પાછા (0);
	}

	/* probably should check which exception occurred as well */
	જો (longjmp_on_fault) {
		longjmp_on_fault = 0;
		kgdb_longjmp(error_jmp_buf, KGDBERR_MEMFAULT);
		panic("kgdb longjump failed!\n");
	}

	જો (kgdb_active) {
		printf("kgdb: unexpected exception from within kgdb\n");
		પાછા (0);
	}
	kgdb_active = 1;

	kgdb_interruptible(0);

	printf("kgdb: handle_exception; trap [0x%x]\n", kgdb_trap(regs));

	જો (kgdb_setjmp(error_jmp_buf) != 0)
		panic("kgdb: error or fault in entry init!\n");

	kgdb_enter(regs, &kd);

	entry_regs = *regs;

	ptr = remcomOutBuffer;

	*ptr++ = 'T';

	*ptr++ = hexchars[kd.sigval >> 4];
	*ptr++ = hexchars[kd.sigval & 0xf];

	માટે (i = 0; i < kd.nregs; i++) {
		kgdb_reg *rp = &kd.regs[i];

		*ptr++ = hexchars[rp->અંક >> 4];
		*ptr++ = hexchars[rp->અંક & 0xf];
		*ptr++ = ':';
		ptr = (અક્ષર *)mem2hex((અક્ષર *)&rp->val, ptr, 4);
		*ptr++ = ';';
	}

	*ptr = 0;

#ifdef KGDB_DEBUG
	જો (kdebug)
		printf("kgdb: remcomOutBuffer: %s\n", remcomOutBuffer);
#endif

	putpacket((unsigned અક્ષર *)&remcomOutBuffer);

	જ્યારે (1) {
		અસ્થિર પૂર્ણાંક errnum;

		remcomOutBuffer[0] = 0;

		getpacket(remcomInBuffer);
		ptr = &remcomInBuffer[1];

#ifdef KGDB_DEBUG
		જો (kdebug)
			printf("kgdb:  remcomInBuffer: %s\n", remcomInBuffer);
#endif

		errnum = kgdb_setjmp(error_jmp_buf);

		જો (errnum == 0) પસંદગી (remcomInBuffer[0]) {

		સ્થિતિ '?':               /* report most recent signal */
			remcomOutBuffer[0] = 'S';
			remcomOutBuffer[1] = hexchars[kd.sigval >> 4];
			remcomOutBuffer[2] = hexchars[kd.sigval & 0xf];
			remcomOutBuffer[3] = 0;
			તોડો;

#ifdef KGDB_DEBUG
		સ્થિતિ 'd':
			/* toggle debug flag */
			kdebug ^= 1;
			તોડો;
#endif

		સ્થિતિ 'g':	/* return the value of the CPU registers. */
			length = kgdb_getregs(regs, remcomRegBuffer, BUFMAX);
			mem2hex(remcomRegBuffer, remcomOutBuffer, length);
			તોડો;

		સ્થિતિ 'G':   /* set the value of the CPU registers */
			length = strlen(ptr);
			જો ((length & 1) != 0) kgdb_error(KGDBERR_BADPARAMS);
			hex2mem(ptr, remcomRegBuffer, length/2);
			kgdb_putregs(regs, remcomRegBuffer, length/2);
			strcpy(remcomOutBuffer,"OK");
			તોડો;

		સ્થિતિ 'm':	/* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
				/* Try to read %x,%x.  */

			જો (hexToInt(&ptr, &addr)
			    && *ptr++ == ','
			    && hexToInt(&ptr, &length))	{
				mem2hex((અક્ષર *)addr, remcomOutBuffer, length);
			} નહીંતર {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			તોડો;

		સ્થિતિ 'M': /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
			/* Try to read '%x,%x:'.  */

			જો (hexToInt(&ptr, &addr)
			    && *ptr++ == ','
			    && hexToInt(&ptr, &length)
			    && *ptr++ == ':') {
				hex2mem(ptr, (અક્ષર *)addr, length);
				strcpy(remcomOutBuffer, "OK");
			} નહીંતર {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			તોડો;

		સ્થિતિ 'k':    /* kill the program, actually return to monitor */
			kd.extype = KGDBEXIT_KILL;
			*regs = entry_regs;
			જાઓ doexit;

		સ્થિતિ 'C':    /* CSS  continue with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		સ્થિતિ 'c':    /* cAA..AA  Continue; address AA..AA optional */
			/* try to read optional parameter, pc unchanged if no parm */
			kd.extype = KGDBEXIT_CONTINUE;

			જો (hexToInt(&ptr, &addr)) {
				kd.exaddr = addr;
				kd.extype |= KGDBEXIT_WITHADDR;
			}

			જાઓ doexit;

		સ્થિતિ 'S':    /* SSS  single step with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		સ્થિતિ 's':
			kd.extype = KGDBEXIT_SINGLE;

			જો (hexToInt(&ptr, &addr)) {
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
			પાછા (1);

		સ્થિતિ 'r':		/* Reset (if user process..exit ???)*/
			panic("kgdb reset.");
			તોડો;

		સ્થિતિ 'P':    /* Pr=v  set reg r to value v (r and v are hex) */
			જો (hexToInt(&ptr, &addr)
			    && *ptr++ == '='
			    && ((length = strlen(ptr)) & 1) == 0) {
				hex2mem(ptr, remcomRegBuffer, length/2);
				kgdb_putreg(regs, addr,
					remcomRegBuffer, length/2);
				strcpy(remcomOutBuffer,"OK");
			} નહીંતર {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			તોડો;
		}			/* switch */

		જો (errnum != 0)
			sprintf(remcomOutBuffer, "E%02d", errnum);

#ifdef KGDB_DEBUG
		જો (kdebug)
			printf("kgdb: remcomOutBuffer: %s\n", remcomOutBuffer);
#endif

		/* reply to the request */
		putpacket((unsigned અક્ષર *)&remcomOutBuffer);

	} /* while(1) */
}

/*
 * kgdb_init must be called *after* the
 * monitor is relocated into ram
 */
પૂર્ણાંક kgdb_init(શૂન્ય)
{
	puts("KGDB:  ");

	kgdb_serial_init();
	debugger_exception_handler = handle_exception;
	initialized = 1;

	putDebugStr("kgdb ready\n");
	puts("ready\n");

	પાછા 0;
}

શૂન્ય
kgdb_error(પૂર્ણાંક errnum)
{
	longjmp_on_fault = 0;
	kgdb_longjmp(error_jmp_buf, errnum);
	panic("kgdb_error: longjmp failed!\n");
}

/* Output string in GDB O-packet format if GDB has connected. If nothing
   output, returns 0 (caller must then handle output). */
પૂર્ણાંક
kgdb_output_string (અચલ અક્ષર* s, unsigned પૂર્ણાંક ગણતરી)
{
	અક્ષર buffer[512];

	ગણતરી = (ગણતરી <= (કદ(buffer) / 2 - 2))
		? ગણતરી : (કદ(buffer) / 2 - 2);

	buffer[0] = 'O';
	mem2hex ((અક્ષર *)s, &buffer[1], ગણતરી);
	putpacket((unsigned અક્ષર *)&buffer);

	પાછા 1;
}

શૂન્ય
breakpoint(શૂન્ય)
{
	જો (!initialized) {
		printf("breakpoint() called b4 kgdb init\n");
		પાછા;
	}

	kgdb_breakpoint(0, 0);
}

પૂર્ણાંક
do_kgdb(સંરચના cmd_tbl *cmdtp, પૂર્ણાંક flag, પૂર્ણાંક argc, અક્ષર *અચલ argv[])
{
    printf("Entering KGDB mode via exception handler...\n\n");
    kgdb_breakpoint(argc - 1, argv + 1);
    printf("\nReturned from KGDB mode\n");
    પાછા 0;
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
