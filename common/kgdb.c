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

#لغو_تعریف KGDB_DEBUG

/*
 * BUFMAX defines the maximum number of characters in inbound/outbound buffers
 */
#تعریف_کن BUFMAX 1024
ساكن حرف remcom_درون_میانگیر[BUFMAX];
ساكن حرف remcom_بیرون_میانگیر[BUFMAX];
ساكن حرف remcom_Reg_میانگیر[BUFMAX];

ساكن صحيح initialized = 0;
ساكن صحيح kgdb_فعال;
ساكن بنية pt_regs مدخل_regs;
ساكن دراز error_jmp_buf[BUFMAX/2];
ساكن صحيح longjmp_on_fault = 0;
#اگر_تعریف_شده KGDB_DEBUG
ساكن صحيح kdebug = 1;
#پایان_اگر

ساكن ثابت حرف hexchars[]="0123456789abcdef";

/* Convert ch from a hex digit to an int */
ساكن صحيح
hex(بیعلامت حرف ch)
{
	إذا (ch >= 'a' && ch <= 'f')
		أرجع ch-'a'+10;
	إذا (ch >= '0' && ch <= '9')
		أرجع ch-'0';
	إذا (ch >= 'A' && ch <= 'F')
		أرجع ch-'A'+10;
	أرجع -1;
}

/* Convert the memory pointed to by mem into hex, placing result in buf.
 * Return a pointer to the last char put in buf (null).
 */
ساكن بیعلامت حرف *
mem2hex(حرف *mem, حرف *buf, صحيح شمار)
{
	حرف *tmp;
	بیعلامت حرف ch;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory copy.  Hex conversion will work against this one.
	 */
	tmp = buf + شمار;
	longjmp_on_fault = 1;

	کپی_حافظه(tmp, mem, شمار);

	طالما (شمار-- > 0) {
		ch = *tmp++;
		*buf++ = hexchars[ch >> 4];
		*buf++ = hexchars[ch & 0xf];
	}
	*buf = 0;
	longjmp_on_fault = 0;
	أرجع (بیعلامت حرف *)buf;
}

/* convert the hex array pointed to by buf into binary to be placed in mem
 * return a pointer to the character AFTER the last byte fetched from buf.
*/
ساكن حرف *
hex2mem(حرف *buf, حرف *mem, صحيح شمار)
{
	صحيح hex_مقدار;
	حرف *tmp_raw, *tmp_hex;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory that is converted from hex.
	 */
	tmp_raw = buf + شمار * 2;
	tmp_hex = tmp_raw - 1;

	longjmp_on_fault = 1;
	طالما (tmp_hex >= buf) {
		tmp_raw--;
		hex_مقدار = hex(*tmp_hex--);
		إذا (hex_مقدار < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw = hex_مقدار;
		hex_مقدار = hex(*tmp_hex--);
		إذا (hex_مقدار < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw |= hex_مقدار << 4;

	}

	کپی_حافظه(mem, tmp_raw, شمار);

	kgdb_flush_cache_range((فراغ *)mem, (فراغ *)(mem+شمار));
	longjmp_on_fault = 0;

	أرجع buf;
}

/*
 * While we find nice hex chars, build an int.
 * Return number of chars processed.
 */
ساكن صحيح
hex_مقصد_Int(حرف **ptr, صحيح *int_مقدار)
{
	صحيح رقم_Chars = 0;
	صحيح hex_مقدار;

	*int_مقدار = 0;

	longjmp_on_fault = 1;
	طالما (**ptr) {
		hex_مقدار = hex(**ptr);
		إذا (hex_مقدار < 0)
			اكسر;

		*int_مقدار = (*int_مقدار << 4) | hex_مقدار;
		رقم_Chars ++;

		(*ptr)++;
	}
	longjmp_on_fault = 0;

	أرجع (رقم_Chars);
}

/* scan for the sequence $<data>#<checksum>     */
ساكن فراغ
getpacket(حرف *میانگیر)
{
	بیعلامت حرف checksum;
	بیعلامت حرف xmitcsum;
	صحيح i;
	صحيح شمار;
	بیعلامت حرف ch;

	افعل {
		/* wait around for the start character, ignore all other
		 * characters */
		طالما ((ch = (getDebugChar() & 0x7f)) != '$') {
#اگر_تعریف_شده KGDB_DEBUG
			إذا (kdebug)
				putc(ch);
#پایان_اگر
			;
		}

		checksum = 0;
		xmitcsum = -1;

		شمار = 0;

		/* now, read until a # or end of buffer is found */
		طالما (شمار < BUFMAX) {
			ch = getDebugChar() & 0x7f;
			إذا (ch == '#')
				اكسر;
			checksum = checksum + ch;
			میانگیر[شمار] = ch;
			شمار = شمار + 1;
		}

		إذا (شمار >= BUFMAX)
			استمر;

		میانگیر[شمار] = 0;

		إذا (ch == '#') {
			xmitcsum = hex(getDebugChar() & 0x7f) << 4;
			xmitcsum |= hex(getDebugChar() & 0x7f);
			إذا (checksum != xmitcsum)
				putDebugChar('-');	/* failed checksum */
			وإلا {
				putDebugChar('+'); /* successful transfer */
				/* if a sequence char is present, reply the ID */
				إذا (میانگیر[2] == ':') {
					putDebugChar(میانگیر[0]);
					putDebugChar(میانگیر[1]);
					/* remove sequence chars from buffer */
					شمار = طول_رشته(میانگیر);
					لكل (i=3; i <= شمار; i++)
						میانگیر[i-3] = میانگیر[i];
				}
			}
		}
	} طالما (checksum != xmitcsum);
}

/* send the packet in buffer.  */
ساكن فراغ
putpacket(بیعلامت حرف *میانگیر)
{
	بیعلامت حرف checksum;
	صحيح شمار;
	بیعلامت حرف ch, recv;

	/*  $<packet info>#<checksum>. */
	افعل {
		putDebugChar('$');
		checksum = 0;
		شمار = 0;

		طالما ((ch = میانگیر[شمار])) {
			putDebugChar(ch);
			checksum += ch;
			شمار += 1;
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
	صحيح درازی;
	حرف *ptr;
	kgdb_داده kd;
	صحيح i;

	إذا (!initialized) {
		چاپ_کن("kgdb: exception before kgdb is initialized! huh?\n");
		أرجع (0);
	}

	/* probably should check which exception occurred as well */
	إذا (longjmp_on_fault) {
		longjmp_on_fault = 0;
		kgdb_longjmp(error_jmp_buf, KGDBERR_MEMFAULT);
		panic("kgdb longjump failed!\n");
	}

	إذا (kgdb_فعال) {
		چاپ_کن("kgdb: unexpected exception from within kgdb\n");
		أرجع (0);
	}
	kgdb_فعال = 1;

	kgdb_interruptible(0);

	چاپ_کن("kgdb: handle_exception; trap [0x%x]\n", kgdb_trap(regs));

	إذا (kgdb_setjmp(error_jmp_buf) != 0)
		panic("kgdb: error or fault in entry init!\n");

	kgdb_enter(regs, &kd);

	مدخل_regs = *regs;

	ptr = remcom_بیرون_میانگیر;

	*ptr++ = 'T';

	*ptr++ = hexchars[kd.sigval >> 4];
	*ptr++ = hexchars[kd.sigval & 0xf];

	لكل (i = 0; i < kd.nregs; i++) {
		kgdb_reg *rp = &kd.regs[i];

		*ptr++ = hexchars[rp->رقم >> 4];
		*ptr++ = hexchars[rp->رقم & 0xf];
		*ptr++ = ':';
		ptr = (حرف *)mem2hex((حرف *)&rp->val, ptr, 4);
		*ptr++ = ';';
	}

	*ptr = 0;

#اگر_تعریف_شده KGDB_DEBUG
	إذا (kdebug)
		چاپ_کن("kgdb: remcomOutBuffer: %s\n", remcom_بیرون_میانگیر);
#پایان_اگر

	putpacket((بیعلامت حرف *)&remcom_بیرون_میانگیر);

	طالما (1) {
		متطاير صحيح errnum;

		remcom_بیرون_میانگیر[0] = 0;

		getpacket(remcom_درون_میانگیر);
		ptr = &remcom_درون_میانگیر[1];

#اگر_تعریف_شده KGDB_DEBUG
		إذا (kdebug)
			چاپ_کن("kgdb:  remcomInBuffer: %s\n", remcom_درون_میانگیر);
#پایان_اگر

		errnum = kgdb_setjmp(error_jmp_buf);

		إذا (errnum == 0) بدل (remcom_درون_میانگیر[0]) {

		حالة '?':               /* report most recent signal */
			remcom_بیرون_میانگیر[0] = 'S';
			remcom_بیرون_میانگیر[1] = hexchars[kd.sigval >> 4];
			remcom_بیرون_میانگیر[2] = hexchars[kd.sigval & 0xf];
			remcom_بیرون_میانگیر[3] = 0;
			اكسر;

#اگر_تعریف_شده KGDB_DEBUG
		حالة 'd':
			/* toggle debug flag */
			kdebug ^= 1;
			اكسر;
#پایان_اگر

		حالة 'g':	/* return the value of the CPU registers. */
			درازی = kgdb_getregs(regs, remcom_Reg_میانگیر, BUFMAX);
			mem2hex(remcom_Reg_میانگیر, remcom_بیرون_میانگیر, درازی);
			اكسر;

		حالة 'G':   /* set the value of the CPU registers */
			درازی = طول_رشته(ptr);
			إذا ((درازی & 1) != 0) kgdb_error(KGDBERR_BADPARAMS);
			hex2mem(ptr, remcom_Reg_میانگیر, درازی/2);
			kgdb_putregs(regs, remcom_Reg_میانگیر, درازی/2);
			strcpy(remcom_بیرون_میانگیر,"OK");
			اكسر;

		حالة 'm':	/* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
				/* Try to read %x,%x.  */

			إذا (hex_مقصد_Int(&ptr, &addr)
			    && *ptr++ == ','
			    && hex_مقصد_Int(&ptr, &درازی))	{
				mem2hex((حرف *)addr, remcom_بیرون_میانگیر, درازی);
			} وإلا {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			اكسر;

		حالة 'M': /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
			/* Try to read '%x,%x:'.  */

			إذا (hex_مقصد_Int(&ptr, &addr)
			    && *ptr++ == ','
			    && hex_مقصد_Int(&ptr, &درازی)
			    && *ptr++ == ':') {
				hex2mem(ptr, (حرف *)addr, درازی);
				strcpy(remcom_بیرون_میانگیر, "OK");
			} وإلا {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			اكسر;

		حالة 'k':    /* kill the program, actually return to monitor */
			kd.extype = KGDBEXIT_KILL;
			*regs = مدخل_regs;
			اذهب doexit;

		حالة 'C':    /* CSS  continue with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		حالة 'c':    /* cAA..AA  Continue; address AA..AA optional */
			/* try to read optional parameter, pc unchanged if no parm */
			kd.extype = KGDBEXIT_CONTINUE;

			إذا (hex_مقصد_Int(&ptr, &addr)) {
				kd.exaddr = addr;
				kd.extype |= KGDBEXIT_WITHADDR;
			}

			اذهب doexit;

		حالة 'S':    /* SSS  single step with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		حالة 's':
			kd.extype = KGDBEXIT_SINGLE;

			إذا (hex_مقصد_Int(&ptr, &addr)) {
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
			kgdb_فعال = 0;
			kgdb_interruptible(1);
			أرجع (1);

		حالة 'r':		/* Reset (if user process..exit ???)*/
			panic("kgdb reset.");
			اكسر;

		حالة 'P':    /* Pr=v  set reg r to value v (r and v are hex) */
			إذا (hex_مقصد_Int(&ptr, &addr)
			    && *ptr++ == '='
			    && ((درازی = طول_رشته(ptr)) & 1) == 0) {
				hex2mem(ptr, remcom_Reg_میانگیر, درازی/2);
				kgdb_putreg(regs, addr,
					remcom_Reg_میانگیر, درازی/2);
				strcpy(remcom_بیرون_میانگیر,"OK");
			} وإلا {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			اكسر;
		}			/* switch */

		إذا (errnum != 0)
			sprintf(remcom_بیرون_میانگیر, "E%02d", errnum);

#اگر_تعریف_شده KGDB_DEBUG
		إذا (kdebug)
			چاپ_کن("kgdb: remcomOutBuffer: %s\n", remcom_بیرون_میانگیر);
#پایان_اگر

		/* reply to the request */
		putpacket((بیعلامت حرف *)&remcom_بیرون_میانگیر);

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
kgdb_output_string (ثابت حرف* s, بیعلامت صحيح شمار)
{
	حرف میانگیر[512];

	شمار = (شمار <= (حجم(میانگیر) / 2 - 2))
		? شمار : (حجم(میانگیر) / 2 - 2);

	میانگیر[0] = 'O';
	mem2hex ((حرف *)s, &میانگیر[1], شمار);
	putpacket((بیعلامت حرف *)&میانگیر);

	أرجع 1;
}

فراغ
breakpoint(فراغ)
{
	إذا (!initialized) {
		چاپ_کن("breakpoint() called b4 kgdb init\n");
		أرجع;
	}

	kgdb_breakpoint(0, 0);
}

صحيح
do_kgdb(بنية cmd_tbl *cmdtp, صحيح flag, صحيح argc, حرف *ثابت argv[])
{
    چاپ_کن("Entering KGDB mode via exception handler...\n\n");
    kgdb_breakpoint(argc - 1, argv + 1);
    چاپ_کن("\nReturned from KGDB mode\n");
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
