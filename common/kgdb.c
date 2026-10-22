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

#تعریف_لغوه KGDB_DEBUG

/*
 * BUFMAX defines the maximum number of characters in inbound/outbound buffers
 */
#تعریف_کړه BUFMAX 1024
ساكن حرف remcom_دننه_بفر[BUFMAX];
ساكن حرف remcom_بهر_بفر[BUFMAX];
ساكن حرف remcom_Reg_بفر[BUFMAX];

ساكن صحيح initialized = 0;
ساكن صحيح kgdb_فعال;
ساكن بنية pt_regs ثبت_regs;
ساكن اوږد error_jmp_buf[BUFMAX/2];
ساكن صحيح longjmp_on_fault = 0;
#که_تعریف_شوی KGDB_DEBUG
ساكن صحيح kdebug = 1;
#د_که_پای

ساكن ثابت حرف hexchars[]="0123456789abcdef";

/* Convert ch from a hex digit to an int */
ساكن صحيح
hex(بینښه حرف ch)
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
ساكن بینښه حرف *
mem2hex(حرف *mem, حرف *buf, صحيح شمېر)
{
	حرف *tmp;
	بینښه حرف ch;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory copy.  Hex conversion will work against this one.
	 */
	tmp = buf + شمېر;
	longjmp_on_fault = 1;

	حافظه_نقل(tmp, mem, شمېر);

	طالما (شمېر-- > 0) {
		ch = *tmp++;
		*buf++ = hexchars[ch >> 4];
		*buf++ = hexchars[ch & 0xf];
	}
	*buf = 0;
	longjmp_on_fault = 0;
	أرجع (بینښه حرف *)buf;
}

/* convert the hex array pointed to by buf into binary to be placed in mem
 * return a pointer to the character AFTER the last byte fetched from buf.
*/
ساكن حرف *
hex2mem(حرف *buf, حرف *mem, صحيح شمېر)
{
	صحيح hex_ارزښت;
	حرف *tmp_raw, *tmp_hex;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory that is converted from hex.
	 */
	tmp_raw = buf + شمېر * 2;
	tmp_hex = tmp_raw - 1;

	longjmp_on_fault = 1;
	طالما (tmp_hex >= buf) {
		tmp_raw--;
		hex_ارزښت = hex(*tmp_hex--);
		إذا (hex_ارزښت < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw = hex_ارزښت;
		hex_ارزښت = hex(*tmp_hex--);
		إذا (hex_ارزښت < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw |= hex_ارزښت << 4;

	}

	حافظه_نقل(mem, tmp_raw, شمېر);

	kgdb_flush_cache_range((فراغ *)mem, (فراغ *)(mem+شمېر));
	longjmp_on_fault = 0;

	أرجع buf;
}

/*
 * While we find nice hex chars, build an int.
 * Return number of chars processed.
 */
ساكن صحيح
hex_منزل_Int(حرف **ptr, صحيح *int_ارزښت)
{
	صحيح عدد_Chars = 0;
	صحيح hex_ارزښت;

	*int_ارزښت = 0;

	longjmp_on_fault = 1;
	طالما (**ptr) {
		hex_ارزښت = hex(**ptr);
		إذا (hex_ارزښت < 0)
			اكسر;

		*int_ارزښت = (*int_ارزښت << 4) | hex_ارزښت;
		عدد_Chars ++;

		(*ptr)++;
	}
	longjmp_on_fault = 0;

	أرجع (عدد_Chars);
}

/* scan for the sequence $<data>#<checksum>     */
ساكن فراغ
getpacket(حرف *بفر)
{
	بینښه حرف checksum;
	بینښه حرف xmitcsum;
	صحيح i;
	صحيح شمېر;
	بینښه حرف ch;

	افعل {
		/* wait around for the start character, ignore all other
		 * characters */
		طالما ((ch = (getDebugChar() & 0x7f)) != '$') {
#که_تعریف_شوی KGDB_DEBUG
			إذا (kdebug)
				putc(ch);
#د_که_پای
			;
		}

		checksum = 0;
		xmitcsum = -1;

		شمېر = 0;

		/* now, read until a # or end of buffer is found */
		طالما (شمېر < BUFMAX) {
			ch = getDebugChar() & 0x7f;
			إذا (ch == '#')
				اكسر;
			checksum = checksum + ch;
			بفر[شمېر] = ch;
			شمېر = شمېر + 1;
		}

		إذا (شمېر >= BUFMAX)
			استمر;

		بفر[شمېر] = 0;

		إذا (ch == '#') {
			xmitcsum = hex(getDebugChar() & 0x7f) << 4;
			xmitcsum |= hex(getDebugChar() & 0x7f);
			إذا (checksum != xmitcsum)
				putDebugChar('-');	/* failed checksum */
			وإلا {
				putDebugChar('+'); /* successful transfer */
				/* if a sequence char is present, reply the ID */
				إذا (بفر[2] == ':') {
					putDebugChar(بفر[0]);
					putDebugChar(بفر[1]);
					/* remove sequence chars from buffer */
					شمېر = مزي_اوږدوالی(بفر);
					لكل (i=3; i <= شمېر; i++)
						بفر[i-3] = بفر[i];
				}
			}
		}
	} طالما (checksum != xmitcsum);
}

/* send the packet in buffer.  */
ساكن فراغ
putpacket(بینښه حرف *بفر)
{
	بینښه حرف checksum;
	صحيح شمېر;
	بینښه حرف ch, recv;

	/*  $<packet info>#<checksum>. */
	افعل {
		putDebugChar('$');
		checksum = 0;
		شمېر = 0;

		طالما ((ch = بفر[شمېر])) {
			putDebugChar(ch);
			checksum += ch;
			شمېر += 1;
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
	صحيح اوږدتیا;
	حرف *ptr;
	kgdb_معلومات kd;
	صحيح i;

	إذا (!initialized) {
		چاپ_کړه("kgdb: exception before kgdb is initialized! huh?\n");
		أرجع (0);
	}

	/* probably should check which exception occurred as well */
	إذا (longjmp_on_fault) {
		longjmp_on_fault = 0;
		kgdb_longjmp(error_jmp_buf, KGDBERR_MEMFAULT);
		panic("kgdb longjump failed!\n");
	}

	إذا (kgdb_فعال) {
		چاپ_کړه("kgdb: unexpected exception from within kgdb\n");
		أرجع (0);
	}
	kgdb_فعال = 1;

	kgdb_interruptible(0);

	چاپ_کړه("kgdb: handle_exception; trap [0x%x]\n", kgdb_trap(regs));

	إذا (kgdb_setjmp(error_jmp_buf) != 0)
		panic("kgdb: error or fault in entry init!\n");

	kgdb_enter(regs, &kd);

	ثبت_regs = *regs;

	ptr = remcom_بهر_بفر;

	*ptr++ = 'T';

	*ptr++ = hexchars[kd.sigval >> 4];
	*ptr++ = hexchars[kd.sigval & 0xf];

	لكل (i = 0; i < kd.nregs; i++) {
		kgdb_reg *rp = &kd.regs[i];

		*ptr++ = hexchars[rp->عدد >> 4];
		*ptr++ = hexchars[rp->عدد & 0xf];
		*ptr++ = ':';
		ptr = (حرف *)mem2hex((حرف *)&rp->val, ptr, 4);
		*ptr++ = ';';
	}

	*ptr = 0;

#که_تعریف_شوی KGDB_DEBUG
	إذا (kdebug)
		چاپ_کړه("kgdb: remcomOutBuffer: %s\n", remcom_بهر_بفر);
#د_که_پای

	putpacket((بینښه حرف *)&remcom_بهر_بفر);

	طالما (1) {
		متطاير صحيح errnum;

		remcom_بهر_بفر[0] = 0;

		getpacket(remcom_دننه_بفر);
		ptr = &remcom_دننه_بفر[1];

#که_تعریف_شوی KGDB_DEBUG
		إذا (kdebug)
			چاپ_کړه("kgdb:  remcomInBuffer: %s\n", remcom_دننه_بفر);
#د_که_پای

		errnum = kgdb_setjmp(error_jmp_buf);

		إذا (errnum == 0) بدل (remcom_دننه_بفر[0]) {

		حالة '?':               /* report most recent signal */
			remcom_بهر_بفر[0] = 'S';
			remcom_بهر_بفر[1] = hexchars[kd.sigval >> 4];
			remcom_بهر_بفر[2] = hexchars[kd.sigval & 0xf];
			remcom_بهر_بفر[3] = 0;
			اكسر;

#که_تعریف_شوی KGDB_DEBUG
		حالة 'd':
			/* toggle debug flag */
			kdebug ^= 1;
			اكسر;
#د_که_پای

		حالة 'g':	/* return the value of the CPU registers. */
			اوږدتیا = kgdb_getregs(regs, remcom_Reg_بفر, BUFMAX);
			mem2hex(remcom_Reg_بفر, remcom_بهر_بفر, اوږدتیا);
			اكسر;

		حالة 'G':   /* set the value of the CPU registers */
			اوږدتیا = مزي_اوږدوالی(ptr);
			إذا ((اوږدتیا & 1) != 0) kgdb_error(KGDBERR_BADPARAMS);
			hex2mem(ptr, remcom_Reg_بفر, اوږدتیا/2);
			kgdb_putregs(regs, remcom_Reg_بفر, اوږدتیا/2);
			strcpy(remcom_بهر_بفر,"OK");
			اكسر;

		حالة 'm':	/* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
				/* Try to read %x,%x.  */

			إذا (hex_منزل_Int(&ptr, &addr)
			    && *ptr++ == ','
			    && hex_منزل_Int(&ptr, &اوږدتیا))	{
				mem2hex((حرف *)addr, remcom_بهر_بفر, اوږدتیا);
			} وإلا {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			اكسر;

		حالة 'M': /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
			/* Try to read '%x,%x:'.  */

			إذا (hex_منزل_Int(&ptr, &addr)
			    && *ptr++ == ','
			    && hex_منزل_Int(&ptr, &اوږدتیا)
			    && *ptr++ == ':') {
				hex2mem(ptr, (حرف *)addr, اوږدتیا);
				strcpy(remcom_بهر_بفر, "OK");
			} وإلا {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			اكسر;

		حالة 'k':    /* kill the program, actually return to monitor */
			kd.extype = KGDBEXIT_KILL;
			*regs = ثبت_regs;
			اذهب doexit;

		حالة 'C':    /* CSS  continue with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		حالة 'c':    /* cAA..AA  Continue; address AA..AA optional */
			/* try to read optional parameter, pc unchanged if no parm */
			kd.extype = KGDBEXIT_CONTINUE;

			إذا (hex_منزل_Int(&ptr, &addr)) {
				kd.exaddr = addr;
				kd.extype |= KGDBEXIT_WITHADDR;
			}

			اذهب doexit;

		حالة 'S':    /* SSS  single step with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		حالة 's':
			kd.extype = KGDBEXIT_SINGLE;

			إذا (hex_منزل_Int(&ptr, &addr)) {
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
			إذا (hex_منزل_Int(&ptr, &addr)
			    && *ptr++ == '='
			    && ((اوږدتیا = مزي_اوږدوالی(ptr)) & 1) == 0) {
				hex2mem(ptr, remcom_Reg_بفر, اوږدتیا/2);
				kgdb_putreg(regs, addr,
					remcom_Reg_بفر, اوږدتیا/2);
				strcpy(remcom_بهر_بفر,"OK");
			} وإلا {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			اكسر;
		}			/* switch */

		إذا (errnum != 0)
			sprintf(remcom_بهر_بفر, "E%02d", errnum);

#که_تعریف_شوی KGDB_DEBUG
		إذا (kdebug)
			چاپ_کړه("kgdb: remcomOutBuffer: %s\n", remcom_بهر_بفر);
#د_که_پای

		/* reply to the request */
		putpacket((بینښه حرف *)&remcom_بهر_بفر);

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
kgdb_output_string (ثابت حرف* s, بینښه صحيح شمېر)
{
	حرف بفر[512];

	شمېر = (شمېر <= (حجم(بفر) / 2 - 2))
		? شمېر : (حجم(بفر) / 2 - 2);

	بفر[0] = 'O';
	mem2hex ((حرف *)s, &بفر[1], شمېر);
	putpacket((بینښه حرف *)&بفر);

	أرجع 1;
}

فراغ
breakpoint(فراغ)
{
	إذا (!initialized) {
		چاپ_کړه("breakpoint() called b4 kgdb init\n");
		أرجع;
	}

	kgdb_breakpoint(0, 0);
}

صحيح
do_kgdb(بنية cmd_tbl *cmdtp, صحيح flag, صحيح argc, حرف *ثابت argv[])
{
    چاپ_کړه("Entering KGDB mode via exception handler...\n\n");
    kgdb_breakpoint(argc - 1, argv + 1);
    چاپ_کړه("\nReturned from KGDB mode\n");
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
