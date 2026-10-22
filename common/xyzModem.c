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
#وصف_ڪريو SOH 0x01
#وصف_ڪريو STX 0x02
#وصف_ڪريو ETX 0x03		/* ^C for interrupt */
#وصف_ڪريو EOT 0x04
#وصف_ڪريو ACK 0x06
#وصف_ڪريو BSP 0x08
#وصف_ڪريو NAK 0x15
#وصف_ڪريو CAN 0x18
#وصف_ڪريو EOF 0x1A		/* ^Z for DOS officionados */

/* Data & state local to the protocol */
ساكن بنية
{
  صحيح *__chan;
  بنانشان حرف pkt[1024], *bufp;
  صحيح pad_pending;	/* trailing ^Z run withheld from the previous block */
  صحيح pad_deliver;	/* ^Z bytes to emit before the next data */
  بنانشان حرف blk, cblk, crc1, crc2;
  بنانشان حرف ايندڙ_blk;	/* Expected block */
  صحيح len, گهڻائي, ڪل_retries;
  صحيح ڪل_SOH, ڪل_STX, ڪل_CAN;
  bool چڪري_جاچ_گهڻائي, at_eof, tx_ack;
  bool پهريون_xmodem_packet;
  ulong initial_وقت, timeout;
  بنانشان ڊگهو file_ڊيگهائي, read_ڊيگهائي;
} xyz;

#وصف_ڪريو xyzModem_CHAR_TIMEOUT            2000	/* 2 seconds */
#وصف_ڪريو xyzModem_MAX_RETRIES             20
#وصف_ڪريو xyz_Modem_MAX_RETRIES_WITH_چڪري_جاچ    10
#وصف_ڪريو xyz_Modem_CAN_ڳڻپ                3	/* Wait for 3 CAN before quitting */

تعريفنوع صحيح cyg_int32;
ساكن صحيح
CYGACC_COMM_IF_GETC_TIMEOUT (حرف chan, حرف *c)
{

  ulong now = get_timer(0);
  وقت_جدول();
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
parse_انگ (حرف *s, بنانشان ڊگهو *val, حرف **es, حرف *delim)
{
  bool پهريون = true;
  صحيح radix = 10;
  حرف c;
  بنانشان ڊگهو نتيجو = 0;
  صحيح digit;

  طالما (*s == ' ')
    s++;
  طالما (*s)
    {
      إذا (پهريون && (s[0] == '0') && (_tolower (s[1]) == 'x'))
	{
	  radix = 16;
	  s += 2;
	}
      پهريون = false;
      c = *s++;
      إذا (_is_hex (c) && ((digit = _from_hex (c)) < radix))
	{
	  /* Valid digit */
	  نتيجو = (نتيجو * radix) + digit;
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
  *val = نتيجو;
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
ساكن حرف *zm_ٻاهريون = zm_debug_buf;
ساكن حرف *zm_ٻاهريون_شروعات = zm_debug_buf;

ساكن صحيح
zm_dprintf(حرف *fmt, ...)
{
	صحيح len;
	va_list دليلون;

	va_شروعات(دليلون, fmt);
	len = diag_vsprintf(zm_ٻاهريون, fmt, دليلون);
	va_پڇاڙي(دليلون);
	zm_ٻاهريون += len;
	أرجع len;
}

ساكن فراغ
zm_flush (فراغ)
{
  zm_ٻاهريون = zm_ٻاهريون_شروعات;
}

ساكن فراغ
zm_dump_buf (فراغ *buf, صحيح len)
{

}

ساكن بنانشان حرف zm_buf[2048];
ساكن بنانشان حرف *zm_bp;

ساكن فراغ
zm_new (فراغ)
{
  zm_bp = zm_buf;
}

ساكن فراغ
zm_save (بنانشان حرف c)
{
  *zm_bp++ = c;
}

ساكن فراغ
zm_dump (صحيح سٽ)
{
  zm_dprintf ("Packet at line: %d\n", سٽ);
  zm_dump_buf (zm_buf, zm_bp - zm_buf);
}

#وصف_ڪريو ZM_DEBUG(x) x
#وإلا
#وصف_ڪريو ZM_DEBUG(x)
#جيڪڏهن_پورو

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
  bool hdr_مليو = false;
  صحيح i, can_ڪل, hdr_chars;
  بنانشان ننڍو cksum;

  ZM_DEBUG (zm_new ());
  /* Find the start of a header */
  can_ڪل = 0;
  hdr_chars = 0;

  إذا (xyz.tx_ack)
    {
      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
      xyz.tx_ack = false;
    }
  طالما (!hdr_مليو)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      إذا (res)
	{
	  hdr_chars++;
	  بدل (c)
	    {
	    حالة SOH:
	      xyz.ڪل_SOH++;
	      fallthrough;
	    حالة STX:
	      إذا (c == STX)
		xyz.ڪل_STX++;
	      hdr_مليو = true;
	      اكسر;
	    حالة CAN:
	    حالة ETX:
	      xyz.ڪل_CAN++;
	      ZM_DEBUG (zm_dump (__LINE__));
	      إذا (++can_ڪل == xyz_Modem_CAN_ڳڻپ)
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
  إذا (xyz.چڪري_جاچ_گهڻائي)
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
  إذا ((xyz.blk ^ xyz.cblk) != (بنانشان حرف) 0xFF)
    {
      ZM_DEBUG (zm_dprintf
		("Framing error - blk: %x/%x/%x\n", xyz.blk, xyz.cblk,
		 (xyz.blk ^ xyz.cblk)));
      ZM_DEBUG (zm_dump_buf (xyz.pkt, xyz.len));
      xyzModem_flush ();
      أرجع xyz_Modem_چوکٺو;
    }
  /* Verify checksum/CRC */
  إذا (xyz.چڪري_جاچ_گهڻائي)
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
#جيڪڏهن_پورو
  أرجع 1000 * CONFIG_CMD_LOADXY_TIMEOUT;
}

صحيح
xyz_Modem_وهڪرو_open (connection_info_t * info, صحيح *err)
{
  صحيح stat = 0;
  صحيح retries = xyzModem_MAX_RETRIES;
  صحيح چڪري_جاچ_retries = xyz_Modem_MAX_RETRIES_WITH_چڪري_جاچ;

/*    ZM_DEBUG(zm_out = zm_out_start); */
#جيڪڏهن_وصف_ٿيل xyzModem_zmodem
  إذا (info->گهڻائي == xyzModem_zmodem)
    {
      *err = xyzModem_noZmodem;
      أرجع -1;
    }
#جيڪڏهن_پورو

/* TODO: CHECK ! */
  صحيح dummy = 0;
  xyz.__chan = &dummy;
  xyz.len = 0;
  xyz.pad_pending = 0;
  xyz.pad_deliver = 0;
  xyz.چڪري_جاچ_گهڻائي = true;
  xyz.at_eof = false;
  xyz.tx_ack = false;
  xyz.گهڻائي = info->گهڻائي;
  xyz.ڪل_retries = 0;
  xyz.ڪل_SOH = 0;
  xyz.ڪل_STX = 0;
  xyz.ڪل_CAN = 0;
  xyz.read_ڊيگهائي = 0;
  xyz.file_ڊيگهائي = 0;
  xyz.پهريون_xmodem_packet = false;
  xyz.initial_وقت = get_timer(0);
  xyz.timeout = xyzModem_get_initial_timeout();

  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.چڪري_جاچ_گهڻائي ? 'C' : NAK));

  إذا (xyz.گهڻائي == xyzModem_xmodem)
    {
      /* X-modem doesn't have an information header - exit here */
      xyz.پهريون_xmodem_packet = true;
      xyz.ايندڙ_blk = 1;
      أرجع 0;
    }

  طالما (!(xyz.timeout && get_timer(xyz.initial_وقت) > xyz.timeout))
    {
      إذا (--retries <= 0)
        {
          retries = xyzModem_MAX_RETRIES;
          چڪري_جاچ_retries = xyz_Modem_MAX_RETRIES_WITH_چڪري_جاچ;
          xyz.چڪري_جاچ_گهڻائي = true;
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
	      parse_انگ ((حرف *) xyz.bufp, &xyz.file_ڊيگهائي, عدم, " ");
	      /* The rest of the file name data block quietly discarded */
	      xyz.tx_ack = true;
	    }
	  xyz.ايندڙ_blk = 1;
	  xyz.len = 0;
	  أرجع 0;
	}
      وإلا إذا (stat == xyzModem_timeout)
	{
	  إذا (--چڪري_جاچ_retries <= 0)
	    xyz.چڪري_جاچ_گهڻائي = false;
	  CYGACC_CALL_IF_DELAY_US (5 * 100000);	/* Extra delay for startup */
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.چڪري_جاچ_گهڻائي ? 'C' : NAK));
	  xyz.ڪل_retries++;
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
xyz_Modem_وهڪرو_read (حرف *buf, صحيح سائيز, صحيح *err)
{
  صحيح stat, ڪل, len;
  صحيح retries;

  ڪل = 0;
  stat = xyzModem_cancel;
  /* Try and get 'size' bytes into the buffer */
  طالما (!xyz.at_eof && xyz.len >= 0 && (سائيز > 0))
    {
      إذا (xyz.len == 0 && xyz.pad_deliver == 0)
	{
	  retries = xyzModem_MAX_RETRIES;
	  طالما (retries-- > 0)
	    {
	      إذا (xyz.پهريون_xmodem_packet && xyz.timeout &&
		  get_timer(xyz.initial_وقت) > xyz.timeout)
		{
		  *err = xyzModem_timeout;
		  xyz.len = -1;
		  أرجع ڪل;
		}

	      stat = xyzModem_get_hdr ();
	      إذا (stat == 0)
		{
		  إذا (xyz.گهڻائي == xyzModem_xmodem && xyz.پهريون_xmodem_packet)
		    xyz.پهريون_xmodem_packet = false;
		  إذا (xyz.blk == xyz.ايندڙ_blk)
		    {
		      xyz.tx_ack = true;
		      ZM_DEBUG (zm_dprintf
				("ACK block %d (%d)\n", xyz.blk, __LINE__));
		      xyz.ايندڙ_blk = (xyz.ايندڙ_blk + 1) & 0xFF;

		      إذا (xyz.گهڻائي == xyzModem_xmodem || xyz.file_ڊيگهائي == 0)
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
		      إذا (xyz.گهڻائي != xyzModem_xmodem && 0 != xyz.file_ڊيگهائي)
			{
			  xyz.read_ڊيگهائي += xyz.len;
			  إذا (xyz.read_ڊيگهائي > xyz.file_ڊيگهائي)
			    {
			      xyz.len -= (xyz.read_ڊيگهائي - xyz.file_ڊيگهائي);
			    }
			}
		      اكسر;
		    }
		  وإلا إذا (xyz.blk == ((xyz.ايندڙ_blk - 1) & 0xFF))
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
		  إذا (xyz.گهڻائي == xyzModem_ymodem)
		    {
		      CYGACC_COMM_IF_PUTC (*xyz.__chan,
					   (xyz.چڪري_جاچ_گهڻائي ? 'C' : NAK));
		      xyz.ڪل_retries++;
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
	      CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.چڪري_جاچ_گهڻائي ? 'C' : NAK));
	      xyz.ڪل_retries++;
	      ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	    }
	  إذا (stat < 0 && (!xyz.پهريون_xmodem_packet || stat != xyzModem_timeout))
	    {
	      *err = stat;
	      xyz.len = -1;
	      أرجع ڪل;
	    }
	}
      /* A ^Z run withheld from the previous block turned out to be data */
      إذا (xyz.pad_deliver > 0)
	{
	  len = xyz.pad_deliver;
	  إذا (سائيز < len)
	    len = سائيز;
	  memset (buf, EOF, len);
	  سائيز -= len;
	  buf += len;
	  ڪل += len;
	  xyz.pad_deliver -= len;
	}
      /* Don't "read" data from the EOF protocol package */
      إذا (!xyz.at_eof && xyz.len > 0)
	{
	  len = xyz.len;
	  إذا (سائيز < len)
	    len = سائيز;
	  يادگيري_نقل (buf, xyz.bufp, len);
	  سائيز -= len;
	  buf += len;
	  ڪل += len;
	  xyz.len -= len;
	  xyz.bufp += len;
	}
    }
  أرجع ڪل;
}

فراغ
xyz_Modem_وهڪرو_close (صحيح *err)
{
  ZM_DEBUG (zm_dprintf
    ("xyzModem - %s mode, %d(SOH)/%d(STX)/%d(CAN) packets, %d retries\n",
     xyz.چڪري_جاچ_گهڻائي ? "CRC" : "Cksum", xyz.ڪل_SOH, xyz.ڪل_STX,
     xyz.ڪل_CAN, xyz.ڪل_retries));
  ZM_DEBUG (zm_flush ());
}

/* Need to be able to clean out the input buffer, so have to take the */
/* getc */
فراغ
xyz_Modem_وهڪرو_terminate (bool abort, صحيح (*getc) (فراغ))
{
  صحيح c;

  إذا (abort)
    {
      ZM_DEBUG (zm_dprintf ("!!!! TRANSFER ABORT !!!!\n"));
      بدل (xyz.گهڻائي)
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
#جيڪڏهن_وصف_ٿيل xyzModem_zmodem
	حالة xyzModem_zmodem:
	  /* Might support it some day I suppose. */
#جيڪڏهن_پورو
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
    حالة xyz_Modem_چوکٺو:
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
