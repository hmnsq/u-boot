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

#include <asm/ptrace.h>

#include <kgdb.h>
#include <command.h>

#undef KGDB_DEBUG

/*
 * BUFMAX defines the maximum number of characters in inbound/outbound buffers
 */
#define BUFMAX 1024
static var_na remcomInBuffer[BUFMAX];
static var_na remcomOutBuffer[BUFMAX];
static var_na remcomRegBuffer[BUFMAX];

static ចំនួនគត់ initialized = 0;
static ចំនួនគត់ kgdb_active;
static sa_nracanaa pt_regs entry_regs;
static long error_jmp_buf[BUFMAX/2];
static ចំនួនគត់ longjmp_on_fault = 0;
#ifdef KGDB_DEBUG
static ចំនួនគត់ kdebug = 1;
#endif

static const var_na hexchars[]="0123456789abcdef";

/* Convert ch from a hex digit to an int */
static ចំនួនគត់
hex(unsigned var_na ch)
{
	ប្រសិនបើ (ch >= 'a' && ch <= 'f')
		ត្រឡប់ ch-'a'+10;
	ប្រសិនបើ (ch >= '0' && ch <= '9')
		ត្រឡប់ ch-'0';
	ប្រសិនបើ (ch >= 'A' && ch <= 'F')
		ត្រឡប់ ch-'A'+10;
	ត្រឡប់ -1;
}

/* Convert the memory pointed to by mem into hex, placing result in buf.
 * Return a pointer to the last char put in buf (null).
 */
static unsigned var_na *
mem2hex(var_na *mem, var_na *buf, ចំនួនគត់ count)
{
	var_na *tmp;
	unsigned var_na ch;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory copy.  Hex conversion will work against this one.
	 */
	tmp = buf + count;
	longjmp_on_fault = 1;

	memcpy(tmp, mem, count);

	ខណៈពេល (count-- > 0) {
		ch = *tmp++;
		*buf++ = hexchars[ch >> 4];
		*buf++ = hexchars[ch & 0xf];
	}
	*buf = 0;
	longjmp_on_fault = 0;
	ត្រឡប់ (unsigned var_na *)buf;
}

/* convert the hex array pointed to by buf into binary to be placed in mem
 * return a pointer to the character AFTER the last byte fetched from buf.
*/
static var_na *
hex2mem(var_na *buf, var_na *mem, ចំនួនគត់ count)
{
	ចំនួនគត់ hexValue;
	var_na *tmp_raw, *tmp_hex;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory that is converted from hex.
	 */
	tmp_raw = buf + count * 2;
	tmp_hex = tmp_raw - 1;

	longjmp_on_fault = 1;
	ខណៈពេល (tmp_hex >= buf) {
		tmp_raw--;
		hexValue = hex(*tmp_hex--);
		ប្រសិនបើ (hexValue < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw = hexValue;
		hexValue = hex(*tmp_hex--);
		ប្រសិនបើ (hexValue < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw |= hexValue << 4;

	}

	memcpy(mem, tmp_raw, count);

	kgdb_flush_cache_range((void *)mem, (void *)(mem+count));
	longjmp_on_fault = 0;

	ត្រឡប់ buf;
}

/*
 * While we find nice hex chars, build an int.
 * Return number of chars processed.
 */
static ចំនួនគត់
hexToInt(var_na **ptr, ចំនួនគត់ *intValue)
{
	ចំនួនគត់ numChars = 0;
	ចំនួនគត់ hexValue;

	*intValue = 0;

	longjmp_on_fault = 1;
	ខណៈពេល (**ptr) {
		hexValue = hex(**ptr);
		ប្រសិនបើ (hexValue < 0)
			bha_nga;

		*intValue = (*intValue << 4) | hexValue;
		numChars ++;

		(*ptr)++;
	}
	longjmp_on_fault = 0;

	ត្រឡប់ (numChars);
}

/* scan for the sequence $<data>#<checksum>     */
static void
getpacket(var_na *buffer)
{
	unsigned var_na checksum;
	unsigned var_na xmitcsum;
	ចំនួនគត់ i;
	ចំនួនគត់ count;
	unsigned var_na ch;

	do {
		/* wait around for the start character, ignore all other
		 * characters */
		ខណៈពេល ((ch = (getDebugChar() & 0x7f)) != '$') {
#ifdef KGDB_DEBUG
			ប្រសិនបើ (kdebug)
				putc(ch);
#endif
			;
		}

		checksum = 0;
		xmitcsum = -1;

		count = 0;

		/* now, read until a # or end of buffer is found */
		ខណៈពេល (count < BUFMAX) {
			ch = getDebugChar() & 0x7f;
			ប្រសិនបើ (ch == '#')
				bha_nga;
			checksum = checksum + ch;
			buffer[count] = ch;
			count = count + 1;
		}

		ប្រសិនបើ (count >= BUFMAX)
			continue;

		buffer[count] = 0;

		ប្រសិនបើ (ch == '#') {
			xmitcsum = hex(getDebugChar() & 0x7f) << 4;
			xmitcsum |= hex(getDebugChar() & 0x7f);
			ប្រសិនបើ (checksum != xmitcsum)
				putDebugChar('-');	/* failed checksum */
			បើមិនដូច្នេះ {
				putDebugChar('+'); /* successful transfer */
				/* if a sequence char is present, reply the ID */
				ប្រសិនបើ (buffer[2] == ':') {
					putDebugChar(buffer[0]);
					putDebugChar(buffer[1]);
					/* remove sequence chars from buffer */
					count = strlen(buffer);
					សម្រាប់ (i=3; i <= count; i++)
						buffer[i-3] = buffer[i];
				}
			}
		}
	} ខណៈពេល (checksum != xmitcsum);
}

/* send the packet in buffer.  */
static void
putpacket(unsigned var_na *buffer)
{
	unsigned var_na checksum;
	ចំនួនគត់ count;
	unsigned var_na ch, recv;

	/*  $<packet info>#<checksum>. */
	do {
		putDebugChar('$');
		checksum = 0;
		count = 0;

		ខណៈពេល ((ch = buffer[count])) {
			putDebugChar(ch);
			checksum += ch;
			count += 1;
		}

		putDebugChar('#');
		putDebugChar(hexchars[checksum >> 4]);
		putDebugChar(hexchars[checksum & 0xf]);
		recv = getDebugChar();
	} ខណៈពេល ((recv & 0x7f) != '+');
}

/*
 * This function does all command processing for interfacing to gdb.
 */
static ចំនួនគត់
handle_exception (sa_nracanaa pt_regs *regs)
{
	ចំនួនគត់ addr;
	ចំនួនគត់ length;
	var_na *ptr;
	kgdb_data kd;
	ចំនួនគត់ i;

	ប្រសិនបើ (!initialized) {
		printf("kgdb: exception before kgdb is initialized! huh?\n");
		ត្រឡប់ (0);
	}

	/* probably should check which exception occurred as well */
	ប្រសិនបើ (longjmp_on_fault) {
		longjmp_on_fault = 0;
		kgdb_longjmp(error_jmp_buf, KGDBERR_MEMFAULT);
		panic("kgdb longjump failed!\n");
	}

	ប្រសិនបើ (kgdb_active) {
		printf("kgdb: unexpected exception from within kgdb\n");
		ត្រឡប់ (0);
	}
	kgdb_active = 1;

	kgdb_interruptible(0);

	printf("kgdb: handle_exception; trap [0x%x]\n", kgdb_trap(regs));

	ប្រសិនបើ (kgdb_setjmp(error_jmp_buf) != 0)
		panic("kgdb: error or fault in entry init!\n");

	kgdb_enter(regs, &kd);

	entry_regs = *regs;

	ptr = remcomOutBuffer;

	*ptr++ = 'T';

	*ptr++ = hexchars[kd.sigval >> 4];
	*ptr++ = hexchars[kd.sigval & 0xf];

	សម្រាប់ (i = 0; i < kd.nregs; i++) {
		kgdb_reg *rp = &kd.regs[i];

		*ptr++ = hexchars[rp->num >> 4];
		*ptr++ = hexchars[rp->num & 0xf];
		*ptr++ = ':';
		ptr = (var_na *)mem2hex((var_na *)&rp->val, ptr, 4);
		*ptr++ = ';';
	}

	*ptr = 0;

#ifdef KGDB_DEBUG
	ប្រសិនបើ (kdebug)
		printf("kgdb: remcomOutBuffer: %s\n", remcomOutBuffer);
#endif

	putpacket((unsigned var_na *)&remcomOutBuffer);

	ខណៈពេល (1) {
		ca_ncala ចំនួនគត់ errnum;

		remcomOutBuffer[0] = 0;

		getpacket(remcomInBuffer);
		ptr = &remcomInBuffer[1];

#ifdef KGDB_DEBUG
		ប្រសិនបើ (kdebug)
			printf("kgdb:  remcomInBuffer: %s\n", remcomInBuffer);
#endif

		errnum = kgdb_setjmp(error_jmp_buf);

		ប្រសិនបើ (errnum == 0) switch (remcomInBuffer[0]) {

		pak_sa '?':               /* report most recent signal */
			remcomOutBuffer[0] = 'S';
			remcomOutBuffer[1] = hexchars[kd.sigval >> 4];
			remcomOutBuffer[2] = hexchars[kd.sigval & 0xf];
			remcomOutBuffer[3] = 0;
			bha_nga;

#ifdef KGDB_DEBUG
		pak_sa 'd':
			/* toggle debug flag */
			kdebug ^= 1;
			bha_nga;
#endif

		pak_sa 'g':	/* return the value of the CPU registers. */
			length = kgdb_getregs(regs, remcomRegBuffer, BUFMAX);
			mem2hex(remcomRegBuffer, remcomOutBuffer, length);
			bha_nga;

		pak_sa 'G':   /* set the value of the CPU registers */
			length = strlen(ptr);
			ប្រសិនបើ ((length & 1) != 0) kgdb_error(KGDBERR_BADPARAMS);
			hex2mem(ptr, remcomRegBuffer, length/2);
			kgdb_putregs(regs, remcomRegBuffer, length/2);
			strcpy(remcomOutBuffer,"OK");
			bha_nga;

		pak_sa 'm':	/* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
				/* Try to read %x,%x.  */

			ប្រសិនបើ (hexToInt(&ptr, &addr)
			    && *ptr++ == ','
			    && hexToInt(&ptr, &length))	{
				mem2hex((var_na *)addr, remcomOutBuffer, length);
			} បើមិនដូច្នេះ {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			bha_nga;

		pak_sa 'M': /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
			/* Try to read '%x,%x:'.  */

			ប្រសិនបើ (hexToInt(&ptr, &addr)
			    && *ptr++ == ','
			    && hexToInt(&ptr, &length)
			    && *ptr++ == ':') {
				hex2mem(ptr, (var_na *)addr, length);
				strcpy(remcomOutBuffer, "OK");
			} បើមិនដូច្នេះ {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			bha_nga;

		pak_sa 'k':    /* kill the program, actually return to monitor */
			kd.extype = KGDBEXIT_KILL;
			*regs = entry_regs;
			goto doexit;

		pak_sa 'C':    /* CSS  continue with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		pak_sa 'c':    /* cAA..AA  Continue; address AA..AA optional */
			/* try to read optional parameter, pc unchanged if no parm */
			kd.extype = KGDBEXIT_CONTINUE;

			ប្រសិនបើ (hexToInt(&ptr, &addr)) {
				kd.exaddr = addr;
				kd.extype |= KGDBEXIT_WITHADDR;
			}

			goto doexit;

		pak_sa 'S':    /* SSS  single step with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		pak_sa 's':
			kd.extype = KGDBEXIT_SINGLE;

			ប្រសិនបើ (hexToInt(&ptr, &addr)) {
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
			ត្រឡប់ (1);

		pak_sa 'r':		/* Reset (if user process..exit ???)*/
			panic("kgdb reset.");
			bha_nga;

		pak_sa 'P':    /* Pr=v  set reg r to value v (r and v are hex) */
			ប្រសិនបើ (hexToInt(&ptr, &addr)
			    && *ptr++ == '='
			    && ((length = strlen(ptr)) & 1) == 0) {
				hex2mem(ptr, remcomRegBuffer, length/2);
				kgdb_putreg(regs, addr,
					remcomRegBuffer, length/2);
				strcpy(remcomOutBuffer,"OK");
			} បើមិនដូច្នេះ {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			bha_nga;
		}			/* switch */

		ប្រសិនបើ (errnum != 0)
			sprintf(remcomOutBuffer, "E%02d", errnum);

#ifdef KGDB_DEBUG
		ប្រសិនបើ (kdebug)
			printf("kgdb: remcomOutBuffer: %s\n", remcomOutBuffer);
#endif

		/* reply to the request */
		putpacket((unsigned var_na *)&remcomOutBuffer);

	} /* while(1) */
}

/*
 * kgdb_init must be called *after* the
 * monitor is relocated into ram
 */
ចំនួនគត់ kgdb_init(void)
{
	puts("KGDB:  ");

	kgdb_serial_init();
	debugger_exception_handler = handle_exception;
	initialized = 1;

	putDebugStr("kgdb ready\n");
	puts("ready\n");

	ត្រឡប់ 0;
}

void
kgdb_error(ចំនួនគត់ errnum)
{
	longjmp_on_fault = 0;
	kgdb_longjmp(error_jmp_buf, errnum);
	panic("kgdb_error: longjmp failed!\n");
}

/* Output string in GDB O-packet format if GDB has connected. If nothing
   output, returns 0 (caller must then handle output). */
ចំនួនគត់
kgdb_output_string (const var_na* s, unsigned ចំនួនគត់ count)
{
	var_na buffer[512];

	count = (count <= (parimaa_na(buffer) / 2 - 2))
		? count : (parimaa_na(buffer) / 2 - 2);

	buffer[0] = 'O';
	mem2hex ((var_na *)s, &buffer[1], count);
	putpacket((unsigned var_na *)&buffer);

	ត្រឡប់ 1;
}

void
breakpoint(void)
{
	ប្រសិនបើ (!initialized) {
		printf("breakpoint() called b4 kgdb init\n");
		ត្រឡប់;
	}

	kgdb_breakpoint(0, 0);
}

ចំនួនគត់
do_kgdb(sa_nracanaa cmd_tbl *cmdtp, ចំនួនគត់ flag, ចំនួនគត់ argc, var_na *const argv[])
{
    printf("Entering KGDB mode via exception handler...\n\n");
    kgdb_breakpoint(argc - 1, argv + 1);
    printf("\nReturned from KGDB mode\n");
    ត្រឡប់ 0;
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
