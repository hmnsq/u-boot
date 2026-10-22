// SPDX-License-Identifier: eCos-2.0
/*
 *==========================================================================
 *
 *      xyzModem.c
 *
 *      RedBoot stream handler for xyzModem protocol
 *
 *==========================================================================
 *#####DESCRIPTIONBEGIN####
 *
 * Author(s):    gthomas
 * Contributors: gthomas, tsmith, Yoshinori Sato
 * Date:         2000-07-14
 * Purpose:
 * Description:
 *
 * This code is part of RedBoot (tm).
 *
 *####DESCRIPTIONEND####
 *
 *==========================================================================
 */
#ضمن <xyzModem.h>
#ضمن <stdarg.h>
#ضمن <time.h>
#ضمن <u-boot/crc.h>
#ضمن <watchdog.h>
#ضمن <env.h>
#ضمن <vsprintf.h>

/* Assumption - run xyzModem protocol over the console port */

/* Values magic to the protocol */
#تعریف_کن SOH 0x01
#تعریف_کن STX 0x02
#تعریف_کن ETX 0x03		/* ^C for interrupt */
#تعریف_کن EOT 0x04
#تعریف_کن ACK 0x06
#تعریف_کن BSP 0x08
#تعریف_کن NAK 0x15
#تعریف_کن CAN 0x18
#تعریف_کن EOF 0x1A		/* ^Z for DOS officionados */

/* Data & state local to the protocol */
ساكن بنية
{
  صحيح *__chan;
  بیعلامت حرف pkt[1024], *bufp;
  صحيح pad_pending;	/* trailing ^Z run withheld from the previous block */
  صحيح pad_deliver;	/* ^Z bytes to emit before the next data */
  بیعلامت حرف blk, cblk, crc1, crc2;
  بیعلامت حرف بعدی_blk;	/* Expected block */
  صحيح len, نما, مجموع_retries;
  صحيح مجموع_SOH, مجموع_STX, مجموع_CAN;
  bool بررسی_چرخهای_نما, at_eof, tx_ack;
  bool اولی_xmodem_packet;
  ulong initial_زمان, timeout;
  بیعلامت دراز file_درازی, read_درازی;
} xyz;

#تعریف_کن xyzModem_CHAR_TIMEOUT            2000	/* 2 seconds */
#تعریف_کن xyzModem_MAX_RETRIES             20
#تعریف_کن xyz_Modem_MAX_RETRIES_WITH_بررسی_چرخهای    10
#تعریف_کن xyz_Modem_CAN_شمار                3	/* Wait for 3 CAN before quitting */

تعريفنوع صحيح cyg_int32;
ساكن صحيح
CYGACC_COMM_IF_GETC_TIMEOUT (حرف chan, حرف *c)
{

  ulong now = get_timer(0);
  زمانبندی();
  طالما (!tstc ())
    {
      إذا (get_timer(now) > xyzModem_CHAR_TIMEOUT)
        اكسر;
    }
  إذا (tstc ())
    {
      *c = getchar();
      أرجع 1;
    }
  أرجع 0;
}

ساكن فراغ
CYGACC_COMM_IF_PUTC (حرف x, حرف y)
{
  putc (y);
}

/* Validate a hex character */
__inline__ ساكن bool
_is_hex (حرف c)
{
  أرجع (((c >= '0') && (c <= '9')) ||
	  ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

/* Convert a single hex nibble */
__inline__ ساكن صحيح
_from_hex (حرف c)
{
  صحيح ret = 0;

  إذا ((c >= '0') && (c <= '9'))
    {
      ret = (c - '0');
    }
  وإلا إذا ((c >= 'a') && (c <= 'f'))
    {
      ret = (c - 'a' + 0x0a);
    }
  وإلا إذا ((c >= 'A') && (c <= 'F'))
    {
      ret = (c - 'A' + 0x0A);
    }
  أرجع ret;
}

/* Convert a character to lower case */
__inline__ ساكن حرف
_tolower (حرف c)
{
  إذا ((c >= 'A') && (c <= 'Z'))
    {
      c = (c - 'A') + 'a';
    }
  أرجع c;
}

/* Parse (scan) a number */
ساكن bool
parse_رقم (حرف *s, بیعلامت دراز *val, حرف **es, حرف *delim)
{
  bool اولی = true;
  صحيح radix = 10;
  حرف c;
  بیعلامت دراز نتیجه = 0;
  صحيح digit;

  طالما (*s == ' ')
    s++;
  طالما (*s)
    {
      إذا (اولی && (s[0] == '0') && (_tolower (s[1]) == 'x'))
	{
	  radix = 16;
	  s += 2;
	}
      اولی = false;
      c = *s++;
      إذا (_is_hex (c) && ((digit = _from_hex (c)) < radix))
	{
	  /* Valid digit */
	  نتیجه = (نتیجه * radix) + digit;
	}
      وإلا
	{
	  إذا (delim != (حرف *) 0)
	    {
	      /* See if this character is one of the delimiters */
	      حرف *dp = delim;
	      طالما (*dp && (c != *dp))
		dp++;
	      إذا (*dp)
		اكسر;		/* Found a good delimiter */
	    }
	  أرجع false;		/* Malformatted number */
	}
    }
  *val = نتیجه;
  إذا (es != (حرف **) 0)
    {
      *es = s;
    }
  أرجع true;
}

#إذا defined(DEBUG) && !CONFIG_IS_ENABLED(USE_TINY_PRINTF)
/*
 * Note: this debug setup works by storing the strings in a fixed buffer
 */
ساكن حرف zm_debug_buf[8192];
ساكن حرف *zm_بیرون = zm_debug_buf;
ساكن حرف *zm_بیرون_شروع = zm_debug_buf;

ساكن صحيح
zm_dprintf(حرف *fmt, ...)
{
	صحيح len;
	va_list آرگومانها;

	va_شروع(آرگومانها, fmt);
	len = diag_vsprintf(zm_بیرون, fmt, آرگومانها);
	va_انتها(آرگومانها);
	zm_بیرون += len;
	أرجع len;
}

ساكن فراغ
zm_flush (فراغ)
{
  zm_بیرون = zm_بیرون_شروع;
}

ساكن فراغ
zm_dump_buf (فراغ *buf, صحيح len)
{

}

ساكن بیعلامت حرف zm_buf[2048];
ساكن بیعلامت حرف *zm_bp;

ساكن فراغ
zm_new (فراغ)
{
  zm_bp = zm_buf;
}

ساكن فراغ
zm_save (بیعلامت حرف c)
{
  *zm_bp++ = c;
}

ساكن فراغ
zm_dump (صحيح سطر)
{
  zm_dprintf ("Packet at line: %d\n", سطر);
  zm_dump_buf (zm_buf, zm_bp - zm_buf);
}

#تعریف_کن ZM_DEBUG(x) x
#وإلا
#تعریف_کن ZM_DEBUG(x)
#پایان_اگر

/* Wait for the line to go idle */
ساكن فراغ
xyzModem_flush (فراغ)
{
  صحيح res;
  حرف c;
  طالما (true)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      إذا (!res)
	أرجع;
    }
}

ساكن صحيح
xyzModem_get_hdr (فراغ)
{
  حرف c;
  صحيح res;
  bool hdr_یافت = false;
  صحيح i, can_مجموع, hdr_chars;
  بیعلامت کوتاه cksum;

  ZM_DEBUG (zm_new ());
  /* Find the start of a header */
  can_مجموع = 0;
  hdr_chars = 0;

  إذا (xyz.tx_ack)
    {
      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
      xyz.tx_ack = false;
    }
  طالما (!hdr_یافت)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      إذا (res)
	{
	  hdr_chars++;
	  بدل (c)
	    {
	    حالة SOH:
	      xyz.مجموع_SOH++;
	      fallthrough;
	    حالة STX:
	      إذا (c == STX)
		xyz.مجموع_STX++;
	      hdr_یافت = true;
	      اكسر;
	    حالة CAN:
	    حالة ETX:
	      xyz.مجموع_CAN++;
	      ZM_DEBUG (zm_dump (__LINE__));
	      إذا (++can_مجموع == xyz_Modem_CAN_شمار)
		{
		  أرجع xyzModem_cancel;
		}
	      وإلا
		{
		  /* Wait for multiple CAN to avoid early quits */
		  اكسر;
		}
	    حالة EOT:
	      /* EOT only supported if no noise */
	      إذا (hdr_chars == 1)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK on EOT #%d\n", __LINE__));
		  ZM_DEBUG (zm_dump (__LINE__));
		  أرجع xyzModem_eof;
		}
	    افتراضي:
	      /* Ignore, waiting for start of header */
	      ;
	    }
	}
      وإلا
	{
	  /* Data stream timed out */
	  xyzModem_flush ();	/* Toss any current input */
	  ZM_DEBUG (zm_dump (__LINE__));
	  CYGACC_CALL_IF_DELAY_US ((cyg_int32) 250000);
	  أرجع xyzModem_timeout;
	}
    }

  /* Header found, now read the data */
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (حرف *) &xyz.blk);
  ZM_DEBUG (zm_save (xyz.blk));
  إذا (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      أرجع xyzModem_timeout;
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (حرف *) &xyz.cblk);
  ZM_DEBUG (zm_save (xyz.cblk));
  إذا (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      أرجع xyzModem_timeout;
    }
  xyz.len = (c == SOH) ? 128 : 1024;
  xyz.bufp = xyz.pkt;
  لكل (i = 0; i < xyz.len; i++)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      إذا (res)
	{
	  xyz.pkt[i] = c;
	}
      وإلا
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  أرجع xyzModem_timeout;
	}
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (حرف *) &xyz.crc1);
  ZM_DEBUG (zm_save (xyz.crc1));
  إذا (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      أرجع xyzModem_timeout;
    }
  إذا (xyz.بررسی_چرخهای_نما)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (حرف *) &xyz.crc2);
      ZM_DEBUG (zm_save (xyz.crc2));
      إذا (!res)
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  أرجع xyzModem_timeout;
	}
    }
  ZM_DEBUG (zm_dump (__LINE__));
  /* Validate the message */
  إذا ((xyz.blk ^ xyz.cblk) != (بیعلامت حرف) 0xFF)
    {
      ZM_DEBUG (zm_dprintf
		("Framing error - blk: %x/%x/%x\n", xyz.blk, xyz.cblk,
		 (xyz.blk ^ xyz.cblk)));
      ZM_DEBUG (zm_dump_buf (xyz.pkt, xyz.len));
      xyzModem_flush ();
      أرجع xyz_Modem_قاب;
    }
  /* Verify checksum/CRC */
  إذا (xyz.بررسی_چرخهای_نما)
    {
      cksum = crc16_ccitt(0, xyz.pkt, xyz.len);
      إذا (cksum != ((xyz.crc1 << 8) | xyz.crc2))
	{
	  ZM_DEBUG (zm_dprintf ("CRC error - recvd: %02x%02x, computed: %x\n",
				xyz.crc1, xyz.crc2, cksum & 0xFFFF));
	  أرجع xyzModem_cksum;
	}
    }
  وإلا
    {
      cksum = 0;
      لكل (i = 0; i < xyz.len; i++)
	{
	  cksum += xyz.pkt[i];
	}
      إذا (xyz.crc1 != (cksum & 0xFF))
	{
	  ZM_DEBUG (zm_dprintf
		    ("Checksum error - recvd: %x, computed: %x\n", xyz.crc1,
		     cksum & 0xFF));
	  أرجع xyzModem_cksum;
	}
    }
  /* If we get here, the message passes [structural] muster */
  أرجع 0;
}

ساكن
ulong
xyzModem_get_initial_timeout (فراغ)
{
  /* timeout is in seconds, non-positive timeout value is infinity */
#إذا CONFIG_IS_ENABLED(ENV_SUPPORT)
  ثابت حرف *timeout_str = env_get("loadxy_timeout");
  إذا (timeout_str)
    أرجع 1000 * simple_strtol(timeout_str, عدم, 10);
#پایان_اگر
  أرجع 1000 * CONFIG_CMD_LOADXY_TIMEOUT;
}

صحيح
xyz_Modem_جریان_open (connection_info_t * info, صحيح *err)
{
  صحيح stat = 0;
  صحيح retries = xyzModem_MAX_RETRIES;
  صحيح بررسی_چرخهای_retries = xyz_Modem_MAX_RETRIES_WITH_بررسی_چرخهای;

/*    ZM_DEBUG(zm_out = zm_out_start); */
#اگر_تعریف_شده xyzModem_zmodem
  إذا (info->نما == xyzModem_zmodem)
    {
      *err = xyzModem_noZmodem;
      أرجع -1;
    }
#پایان_اگر

/* TODO: CHECK ! */
  صحيح dummy = 0;
  xyz.__chan = &dummy;
  xyz.len = 0;
  xyz.pad_pending = 0;
  xyz.pad_deliver = 0;
  xyz.بررسی_چرخهای_نما = true;
  xyz.at_eof = false;
  xyz.tx_ack = false;
  xyz.نما = info->نما;
  xyz.مجموع_retries = 0;
  xyz.مجموع_SOH = 0;
  xyz.مجموع_STX = 0;
  xyz.مجموع_CAN = 0;
  xyz.read_درازی = 0;
  xyz.file_درازی = 0;
  xyz.اولی_xmodem_packet = false;
  xyz.initial_زمان = get_timer(0);
  xyz.timeout = xyzModem_get_initial_timeout();

  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.بررسی_چرخهای_نما ? 'C' : NAK));

  إذا (xyz.نما == xyzModem_xmodem)
    {
      /* X-modem doesn't have an information header - exit here */
      xyz.اولی_xmodem_packet = true;
      xyz.بعدی_blk = 1;
      أرجع 0;
    }

  طالما (!(xyz.timeout && get_timer(xyz.initial_زمان) > xyz.timeout))
    {
      إذا (--retries <= 0)
        {
          retries = xyzModem_MAX_RETRIES;
          بررسی_چرخهای_retries = xyz_Modem_MAX_RETRIES_WITH_بررسی_چرخهای;
          xyz.بررسی_چرخهای_نما = true;
        }
      stat = xyzModem_get_hdr ();
      إذا (stat == 0)
	{
	  /* Y-modem file information header */
	  إذا (xyz.blk == 0)
	    {
	      /* skip filename */
	      طالما (*xyz.bufp++);
	      /* get the length */
	      parse_رقم ((حرف *) xyz.bufp, &xyz.file_درازی, عدم, " ");
	      /* The rest of the file name data block quietly discarded */
	      xyz.tx_ack = true;
	    }
	  xyz.بعدی_blk = 1;
	  xyz.len = 0;
	  أرجع 0;
	}
      وإلا إذا (stat == xyzModem_timeout)
	{
	  إذا (--بررسی_چرخهای_retries <= 0)
	    xyz.بررسی_چرخهای_نما = false;
	  CYGACC_CALL_IF_DELAY_US (5 * 100000);	/* Extra delay for startup */
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.بررسی_چرخهای_نما ? 'C' : NAK));
	  xyz.مجموع_retries++;
	  ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	}
      إذا (stat == xyzModem_cancel)
	{
	  اكسر;
	}
    }
  *err = stat;
  ZM_DEBUG (zm_flush ());
  أرجع -1;
}

صحيح
xyz_Modem_جریان_read (حرف *buf, صحيح اندازه, صحيح *err)
{
  صحيح stat, مجموع, len;
  صحيح retries;

  مجموع = 0;
  stat = xyzModem_cancel;
  /* Try and get 'size' bytes into the buffer */
  طالما (!xyz.at_eof && xyz.len >= 0 && (اندازه > 0))
    {
      إذا (xyz.len == 0 && xyz.pad_deliver == 0)
	{
	  retries = xyzModem_MAX_RETRIES;
	  طالما (retries-- > 0)
	    {
	      إذا (xyz.اولی_xmodem_packet && xyz.timeout &&
		  get_timer(xyz.initial_زمان) > xyz.timeout)
		{
		  *err = xyzModem_timeout;
		  xyz.len = -1;
		  أرجع مجموع;
		}

	      stat = xyzModem_get_hdr ();
	      إذا (stat == 0)
		{
		  إذا (xyz.نما == xyzModem_xmodem && xyz.اولی_xmodem_packet)
		    xyz.اولی_xmodem_packet = false;
		  إذا (xyz.blk == xyz.بعدی_blk)
		    {
		      xyz.tx_ack = true;
		      ZM_DEBUG (zm_dprintf
				("ACK block %d (%d)\n", xyz.blk, __LINE__));
		      xyz.بعدی_blk = (xyz.بعدی_blk + 1) & 0xFF;

		      إذا (xyz.نما == xyzModem_xmodem || xyz.file_درازی == 0)
			{
			  /* Without a file length, EOF padding can't be
			   * distinguished from block data until the next
			   * frame arrives and we can check for EOT/EOF.
			   */
			  xyz.pad_deliver = xyz.pad_pending;
			  xyz.pad_pending = 0;
			  إذا ((xyz.bufp[xyz.len - 1] == EOF) &&
			      (xyz.bufp[xyz.len - 2] == EOF) &&
			      (xyz.bufp[xyz.len - 3] == EOF))
			    {
			      طالما (xyz.len
				     && (xyz.bufp[xyz.len - 1] == EOF))
				{
				  xyz.len--;
				  xyz.pad_pending++;
				}
			    }
			}

		      /*
		       * See if accumulated length exceeds that of the file.
		       * If so, reduce size (i.e., cut out pad bytes)
		       * Only do this for Y-modem (and Z-modem should it ever
		       * be supported since it can fall back to Y-modem mode).
		       */
		      إذا (xyz.نما != xyzModem_xmodem && 0 != xyz.file_درازی)
			{
			  xyz.read_درازی += xyz.len;
			  إذا (xyz.read_درازی > xyz.file_درازی)
			    {
			      xyz.len -= (xyz.read_درازی - xyz.file_درازی);
			    }
			}
		      اكسر;
		    }
		  وإلا إذا (xyz.blk == ((xyz.بعدی_blk - 1) & 0xFF))
		    {
		      /* Just re-ACK this so sender will get on with it */
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      استمر;	/* Need new header */
		    }
		  وإلا
		    {
		      stat = xyzModem_sequence;
		    }
		}
	      إذا (stat == xyzModem_cancel)
		{
		  اكسر;
		}
	      إذا (stat == xyzModem_eof)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK (%d)\n", __LINE__));
		  إذا (xyz.نما == xyzModem_ymodem)
		    {
		      CYGACC_COMM_IF_PUTC (*xyz.__chan,
					   (xyz.بررسی_چرخهای_نما ? 'C' : NAK));
		      xyz.مجموع_retries++;
		      ZM_DEBUG (zm_dprintf ("Reading Final Header\n"));
		      stat = xyzModem_get_hdr ();
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ZM_DEBUG (zm_dprintf ("FINAL ACK (%d)\n", __LINE__));
		    }
		  وإلا
		    stat = 0;

		  xyz.pad_pending = 0;	/* EOT: withheld ^Z run was padding */
		  xyz.at_eof = true;
		  اكسر;
		}
	      CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.بررسی_چرخهای_نما ? 'C' : NAK));
	      xyz.مجموع_retries++;
	      ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	    }
	  إذا (stat < 0 && (!xyz.اولی_xmodem_packet || stat != xyzModem_timeout))
	    {
	      *err = stat;
	      xyz.len = -1;
	      أرجع مجموع;
	    }
	}
      /* A ^Z run withheld from the previous block turned out to be data */
      إذا (xyz.pad_deliver > 0)
	{
	  len = xyz.pad_deliver;
	  إذا (اندازه < len)
	    len = اندازه;
	  memset (buf, EOF, len);
	  اندازه -= len;
	  buf += len;
	  مجموع += len;
	  xyz.pad_deliver -= len;
	}
      /* Don't "read" data from the EOF protocol package */
      إذا (!xyz.at_eof && xyz.len > 0)
	{
	  len = xyz.len;
	  إذا (اندازه < len)
	    len = اندازه;
	  کپی_حافظه (buf, xyz.bufp, len);
	  اندازه -= len;
	  buf += len;
	  مجموع += len;
	  xyz.len -= len;
	  xyz.bufp += len;
	}
    }
  أرجع مجموع;
}

فراغ
xyz_Modem_جریان_close (صحيح *err)
{
  ZM_DEBUG (zm_dprintf
    ("xyzModem - %s mode, %d(SOH)/%d(STX)/%d(CAN) packets, %d retries\n",
     xyz.بررسی_چرخهای_نما ? "CRC" : "Cksum", xyz.مجموع_SOH, xyz.مجموع_STX,
     xyz.مجموع_CAN, xyz.مجموع_retries));
  ZM_DEBUG (zm_flush ());
}

/* Need to be able to clean out the input buffer, so have to take the */
/* getc */
فراغ
xyz_Modem_جریان_terminate (bool abort, صحيح (*getc) (فراغ))
{
  صحيح c;

  إذا (abort)
    {
      ZM_DEBUG (zm_dprintf ("!!!! TRANSFER ABORT !!!!\n"));
      بدل (xyz.نما)
	{
	حالة xyzModem_xmodem:
	حالة xyzModem_ymodem:
	  /* The X/YMODEM Spec seems to suggest that multiple CAN followed by an equal */
	  /* number of Backspaces is a friendly way to get the other end to abort. */
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, CAN);
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, CAN);
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, CAN);
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, CAN);
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, BSP);
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, BSP);
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, BSP);
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, BSP);
	  /* Now consume the rest of what's waiting on the line. */
	  ZM_DEBUG (zm_dprintf ("Flushing serial line.\n"));
	  xyzModem_flush ();
	  xyz.at_eof = true;
	  اكسر;
#اگر_تعریف_شده xyzModem_zmodem
	حالة xyzModem_zmodem:
	  /* Might support it some day I suppose. */
#پایان_اگر
	  اكسر;
	}
    }
  وإلا
    {
      ZM_DEBUG (zm_dprintf ("Engaging cleanup mode...\n"));
      /*
       * Consume any trailing crap left in the inbuffer from
       * previous received blocks. Since very few files are an exact multiple
       * of the transfer block size, there will almost always be some gunk here.
       * If we don't eat it now, RedBoot will think the user typed it.
       */
      ZM_DEBUG (zm_dprintf ("Trailing gunk:\n"));
      طالما ((c = (*getc) ()) > -1)
        ;
      ZM_DEBUG (zm_dprintf ("\n"));
      /*
       * Make a small delay to give terminal programs like minicom
       * time to get control again after their file transfer program
       * exits.
       */
      CYGACC_CALL_IF_DELAY_US ((cyg_int32) 250000);
    }
}

حرف *
xyzModem_error (صحيح err)
{
  بدل (err)
    {
    حالة xyzModem_access:
      أرجع "Can't access file";
      اكسر;
    حالة xyzModem_noZmodem:
      أرجع "Sorry, zModem not available yet";
      اكسر;
    حالة xyzModem_timeout:
      أرجع "Timed out";
      اكسر;
    حالة xyzModem_eof:
      أرجع "End of file";
      اكسر;
    حالة xyzModem_cancel:
      أرجع "Cancelled";
      اكسر;
    حالة xyz_Modem_قاب:
      أرجع "Invalid framing";
      اكسر;
    حالة xyzModem_cksum:
      أرجع "CRC/checksum error";
      اكسر;
    حالة xyzModem_sequence:
      أرجع "Block sequence error";
      اكسر;
    افتراضي:
      أرجع "Unknown error";
      اكسر;
    }
}

/*
 * RedBoot interface
 */
