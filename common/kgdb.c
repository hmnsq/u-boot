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

#ضمن <asm/ptrace.h>

#ضمن <kgdb.h>
#ضمن <command.h>

#undef KGDB_DEBUG

/*
 * BUFMAX defines the maximum number of characters in inbound/outbound buffers
 */
#define BUFMAX 1024
ساكن حرف remcomInBuffer[BUFMAX];
ساكن حرف remcomOutBuffer[BUFMAX];
ساكن حرف remcomRegBuffer[BUFMAX];

ساكن صحيح initialized = 0;
ساكن صحيح kgdb_active;
ساكن بنية pt_regs entry_regs;
ساكن long error_jmp_buf[BUFMAX/2];
ساكن صحيح longjmp_on_fault = 0;
#ifdef KGDB_DEBUG
ساكن صحيح kdebug = 1;
#endif

ساكن ثابت حرف hexchars[]="0123456789abcdef";

/* Convert ch from a hex digit to an int */
ساكن صحيح
hex(unsigned حرف ch)
{
	𐤀𐤌 (ch >= 'a' && ch <= 'f')
		أرجع ch-'a'+10;
	𐤀𐤌 (ch >= '0' && ch <= '9')
		أرجع ch-'0';
	𐤀𐤌 (ch >= 'A' && ch <= 'F')
		أرجع ch-'A'+10;
	أرجع -1;
}

/* Convert the memory pointed to by mem into hex, placing result in buf.
 * Return a pointer to the last char put in buf (null).
 */
ساكن unsigned حرف *
mem2hex(حرف *mem, حرف *buf, صحيح count)
{
	حرف *tmp;
	unsigned حرف ch;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory copy.  Hex conversion will work against this one.
	 */
	tmp = buf + count;
	longjmp_on_fault = 1;

	memcpy(tmp, mem, count);

	طالما (count-- > 0) {
		ch = *tmp++;
		*buf++ = hexchars[ch >> 4];
		*buf++ = hexchars[ch & 0xf];
	}
	*buf = 0;
	longjmp_on_fault = 0;
	أرجع (unsigned حرف *)buf;
}

/* convert the hex array pointed to by buf into binary to be placed in mem
 * return a pointer to the character AFTER the last byte fetched from buf.
*/
ساكن حرف *
hex2mem(حرف *buf, حرف *mem, صحيح count)
{
	صحيح hexValue;
	حرف *tmp_raw, *tmp_hex;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory that is converted from hex.
	 */
	tmp_raw = buf + count * 2;
	tmp_hex = tmp_raw - 1;

	longjmp_on_fault = 1;
	طالما (tmp_hex >= buf) {
		tmp_raw--;
		hexValue = hex(*tmp_hex--);
		𐤀𐤌 (hexValue < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw = hexValue;
		hexValue = hex(*tmp_hex--);
		𐤀𐤌 (hexValue < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw |= hexValue << 4;

	}

	memcpy(mem, tmp_raw, count);

	kgdb_flush_cache_range((فراغ *)mem, (فراغ *)(mem+count));
	longjmp_on_fault = 0;

	أرجع buf;
}

/*
 * While we find nice hex chars, build an int.
 * Return number of chars processed.
 */
ساكن صحيح
hexToInt(حرف **ptr, صحيح *intValue)
{
	صحيح numChars = 0;
	صحيح hexValue;

	*intValue = 0;

	longjmp_on_fault = 1;
	طالما (**ptr) {
		hexValue = hex(**ptr);
		𐤀𐤌 (hexValue < 0)
			اكسر;

		*intValue = (*intValue << 4) | hexValue;
		numChars ++;

		(*ptr)++;
	}
	longjmp_on_fault = 0;

	أرجع (numChars);
}

/* scan for the sequence $<data>#<checksum>     */
ساكن فراغ
getpacket(حرف *buffer)
{
	unsigned حرف checksum;
	unsigned حرف xmitcsum;
	صحيح i;
	صحيح count;
	unsigned حرف ch;

	𐤐𐤏𐤋 {
		/* wait around for the start character, ignore all other
		 * characters */
		طالما ((ch = (getDebugChar() & 0x7f)) != '$') {
#ifdef KGDB_DEBUG
			𐤀𐤌 (kdebug)
				putc(ch);
#endif
			;
		}

		checksum = 0;
		xmitcsum = -1;

		count = 0;

		/* now, read until a # or end of buffer is found */
		طالما (count < BUFMAX) {
			ch = getDebugChar() & 0x7f;
			𐤀𐤌 (ch == '#')
				اكسر;
			checksum = checksum + ch;
			buffer[count] = ch;
			count = count + 1;
		}

		𐤀𐤌 (count >= BUFMAX)
			استمر;

		buffer[count] = 0;

		𐤀𐤌 (ch == '#') {
			xmitcsum = hex(getDebugChar() & 0x7f) << 4;
			xmitcsum |= hex(getDebugChar() & 0x7f);
			𐤀𐤌 (checksum != xmitcsum)
				putDebugChar('-');	/* failed checksum */
			وإلا {
				putDebugChar('+'); /* successful transfer */
				/* if a sequence char is present, reply the ID */
				𐤀𐤌 (buffer[2] == ':') {
					putDebugChar(buffer[0]);
					putDebugChar(buffer[1]);
					/* remove sequence chars from buffer */
					count = strlen(buffer);
					لكل (i=3; i <= count; i++)
						buffer[i-3] = buffer[i];
				}
			}
		}
	} طالما (checksum != xmitcsum);
}

/* send the packet in buffer.  */
ساكن فراغ
putpacket(unsigned حرف *buffer)
{
	unsigned حرف checksum;
	صحيح count;
	unsigned حرف ch, recv;

	/*  $<packet info>#<checksum>. */
	𐤐𐤏𐤋 {
		putDebugChar('$');
		checksum = 0;
		count = 0;

		طالما ((ch = buffer[count])) {
			putDebugChar(ch);
			checksum += ch;
			count += 1;
		}

		putDebugChar('#');
		putDebugChar(hexchars[checksum >> 4]);
		putDebugChar(hexchars[checksum & 0xf]);
		recv = getDebugChar();
	} طالما ((recv & 0x7f) != '+');
}

/*
 * This function does all command processing for interfacing to gdb.
 */
ساكن صحيح
handle_exception (بنية pt_regs *regs)
{
	صحيح addr;
	صحيح length;
	حرف *ptr;
	kgdb_data kd;
	صحيح i;

	𐤀𐤌 (!initialized) {
		printf("kgdb: exception before kgdb is initialized! huh?\n");
		أرجع (0);
	}

	/* probably should check which exception occurred as well */
	𐤀𐤌 (longjmp_on_fault) {
		longjmp_on_fault = 0;
		kgdb_longjmp(error_jmp_buf, KGDBERR_MEMFAULT);
		panic("kgdb longjump failed!\n");
	}

	𐤀𐤌 (kgdb_active) {
		printf("kgdb: unexpected exception from within kgdb\n");
		أرجع (0);
	}
	kgdb_active = 1;

	kgdb_interruptible(0);

	printf("kgdb: handle_exception; trap [0x%x]\n", kgdb_trap(regs));

	𐤀𐤌 (kgdb_setjmp(error_jmp_buf) != 0)
		panic("kgdb: error or fault in entry init!\n");

	kgdb_enter(regs, &kd);

	entry_regs = *regs;

	ptr = remcomOutBuffer;

	*ptr++ = 'T';

	*ptr++ = hexchars[kd.sigval >> 4];
	*ptr++ = hexchars[kd.sigval & 0xf];

	لكل (i = 0; i < kd.nregs; i++) {
		kgdb_reg *rp = &kd.regs[i];

		*ptr++ = hexchars[rp->num >> 4];
		*ptr++ = hexchars[rp->num & 0xf];
		*ptr++ = ':';
		ptr = (حرف *)mem2hex((حرف *)&rp->val, ptr, 4);
		*ptr++ = ';';
	}

	*ptr = 0;

#ifdef KGDB_DEBUG
	𐤀𐤌 (kdebug)
		printf("kgdb: remcomOutBuffer: %s\n", remcomOutBuffer);
#endif

	putpacket((unsigned حرف *)&remcomOutBuffer);

	طالما (1) {
		متطاير صحيح errnum;

		remcomOutBuffer[0] = 0;

		getpacket(remcomInBuffer);
		ptr = &remcomInBuffer[1];

#ifdef KGDB_DEBUG
		𐤀𐤌 (kdebug)
			printf("kgdb:  remcomInBuffer: %s\n", remcomInBuffer);
#endif

		errnum = kgdb_setjmp(error_jmp_buf);

		𐤀𐤌 (errnum == 0) بدل (remcomInBuffer[0]) {

		حالة '?':               /* report most recent signal */
			remcomOutBuffer[0] = 'S';
			remcomOutBuffer[1] = hexchars[kd.sigval >> 4];
			remcomOutBuffer[2] = hexchars[kd.sigval & 0xf];
			remcomOutBuffer[3] = 0;
			اكسر;

#ifdef KGDB_DEBUG
		حالة 'd':
			/* toggle debug flag */
			kdebug ^= 1;
			اكسر;
#endif

		حالة 'g':	/* return the value of the CPU registers. */
			length = kgdb_getregs(regs, remcomRegBuffer, BUFMAX);
			mem2hex(remcomRegBuffer, remcomOutBuffer, length);
			اكسر;

		حالة 'G':   /* set the value of the CPU registers */
			length = strlen(ptr);
			𐤀𐤌 ((length & 1) != 0) kgdb_error(KGDBERR_BADPARAMS);
			hex2mem(ptr, remcomRegBuffer, length/2);
			kgdb_putregs(regs, remcomRegBuffer, length/2);
			strcpy(remcomOutBuffer,"OK");
			اكسر;

		حالة 'm':	/* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
				/* Try to read %x,%x.  */

			𐤀𐤌 (hexToInt(&ptr, &addr)
			    && *ptr++ == ','
			    && hexToInt(&ptr, &length))	{
				mem2hex((حرف *)addr, remcomOutBuffer, length);
			} وإلا {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			اكسر;

		حالة 'M': /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
			/* Try to read '%x,%x:'.  */

			𐤀𐤌 (hexToInt(&ptr, &addr)
			    && *ptr++ == ','
			    && hexToInt(&ptr, &length)
			    && *ptr++ == ':') {
				hex2mem(ptr, (حرف *)addr, length);
				strcpy(remcomOutBuffer, "OK");
			} وإلا {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			اكسر;

		حالة 'k':    /* kill the program, actually return to monitor */
			kd.extype = KGDBEXIT_KILL;
			*regs = entry_regs;
			اذهب doexit;

		حالة 'C':    /* CSS  continue with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		حالة 'c':    /* cAA..AA  Continue; address AA..AA optional */
			/* try to read optional parameter, pc unchanged if no parm */
			kd.extype = KGDBEXIT_CONTINUE;

			𐤀𐤌 (hexToInt(&ptr, &addr)) {
				kd.exaddr = addr;
				kd.extype |= KGDBEXIT_WITHADDR;
			}

			اذهب doexit;

		حالة 'S':    /* SSS  single step with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		حالة 's':
			kd.extype = KGDBEXIT_SINGLE;

			𐤀𐤌 (hexToInt(&ptr, &addr)) {
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
			أرجع (1);

		حالة 'r':		/* Reset (if user process..exit ???)*/
			panic("kgdb reset.");
			اكسر;

		حالة 'P':    /* Pr=v  set reg r to value v (r and v are hex) */
			𐤀𐤌 (hexToInt(&ptr, &addr)
			    && *ptr++ == '='
			    && ((length = strlen(ptr)) & 1) == 0) {
				hex2mem(ptr, remcomRegBuffer, length/2);
				kgdb_putreg(regs, addr,
					remcomRegBuffer, length/2);
				strcpy(remcomOutBuffer,"OK");
			} وإلا {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			اكسر;
		}			/* switch */

		𐤀𐤌 (errnum != 0)
			sprintf(remcomOutBuffer, "E%02d", errnum);

#ifdef KGDB_DEBUG
		𐤀𐤌 (kdebug)
			printf("kgdb: remcomOutBuffer: %s\n", remcomOutBuffer);
#endif

		/* reply to the request */
		putpacket((unsigned حرف *)&remcomOutBuffer);

	} /* while(1) */
}

/*
 * kgdb_init must be called *after* the
 * monitor is relocated into ram
 */
صحيح kgdb_init(فراغ)
{
	puts("KGDB:  ");

	kgdb_serial_init();
	debugger_exception_handler = handle_exception;
	initialized = 1;

	putDebugStr("kgdb ready\n");
	puts("ready\n");

	أرجع 0;
}

فراغ
kgdb_error(صحيح errnum)
{
	longjmp_on_fault = 0;
	kgdb_longjmp(error_jmp_buf, errnum);
	panic("kgdb_error: longjmp failed!\n");
}

/* Output string in GDB O-packet format if GDB has connected. If nothing
   output, returns 0 (caller must then handle output). */
صحيح
kgdb_output_string (ثابت حرف* s, unsigned صحيح count)
{
	حرف buffer[512];

	count = (count <= (حجم(buffer) / 2 - 2))
		? count : (حجم(buffer) / 2 - 2);

	buffer[0] = 'O';
	mem2hex ((حرف *)s, &buffer[1], count);
	putpacket((unsigned حرف *)&buffer);

	أرجع 1;
}

فراغ
breakpoint(فراغ)
{
	𐤀𐤌 (!initialized) {
		printf("breakpoint() called b4 kgdb init\n");
		أرجع;
	}

	kgdb_breakpoint(0, 0);
}

صحيح
do_kgdb(بنية cmd_tbl *cmdtp, صحيح flag, صحيح argc, حرف *ثابت argv[])
{
    printf("Entering KGDB mode via exception handler...\n\n");
    kgdb_breakpoint(argc - 1, argv + 1);
    printf("\nReturned from KGDB mode\n");
    أرجع 0;
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
