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
#شامل <xyzModem.h>
#شامل <stdarg.h>
#شامل <time.h>
#شامل <u-boot/crc.h>
#شامل <watchdog.h>
#شامل <env.h>
#شامل <vsprintf.h>

/* Assumption - run xyzModem protocol over the console port */

/* Values magic to the protocol */
#تعریف_کرو SOH 0x01
#تعریف_کرو STX 0x02
#تعریف_کرو ETX 0x03		/* ^C for interrupt */
#تعریف_کرو EOT 0x04
#تعریف_کرو ACK 0x06
#تعریف_کرو BSP 0x08
#تعریف_کرو NAK 0x15
#تعریف_کرو CAN 0x18
#تعریف_کرو EOF 0x1A		/* ^Z for DOS officionados */

/* Data & state local to the protocol */
ساکن ساخت
{
  صحیح *__chan;
  غیرعلامتی حرف pkt[1024], *bufp;
  صحیح pad_pending;	/* trailing ^Z run withheld from the previous block */
  صحیح pad_deliver;	/* ^Z bytes to emit before the next data */
  غیرعلامتی حرف blk, cblk, crc1, crc2;
  غیرعلامتی حرف اگلا_blk;	/* Expected block */
  صحیح len, منوال, کل_retries;
  صحیح کل_SOH, کل_STX, کل_CAN;
  bool چکری_جانچ_منوال, at_eof, tx_ack;
  bool پہلا_xmodem_packet;
  ulong initial_وقت, timeout;
  غیرعلامتی طویل file_لمبائی, read_لمبائی;
} xyz;

#تعریف_کرو xyzModem_CHAR_TIMEOUT            2000	/* 2 seconds */
#تعریف_کرو xyzModem_MAX_RETRIES             20
#تعریف_کرو xyz_Modem_MAX_RETRIES_WITH_چکری_جانچ    10
#تعریف_کرو xyz_Modem_CAN_گنتی                3	/* Wait for 3 CAN before quitting */

قسمتعریف صحیح cyg_int32;
ساکن صحیح
CYGACC_COMM_IF_GETC_TIMEOUT (حرف chan, حرف *c)
{

  ulong now = get_timer(0);
  جدول_وقت();
  جب (!tstc ())
    {
      اگر (get_timer(now) > xyzModem_CHAR_TIMEOUT)
        توڑو;
    }
  اگر (tstc ())
    {
      *c = getchar();
      واپس 1;
    }
  واپس 0;
}

ساکن خالی
CYGACC_COMM_IF_PUTC (حرف x, حرف y)
{
  putc (y);
}

/* Validate a hex character */
__inline__ ساکن bool
_is_hex (حرف c)
{
  واپس (((c >= '0') && (c <= '9')) ||
	  ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

/* Convert a single hex nibble */
__inline__ ساکن صحیح
_from_hex (حرف c)
{
  صحیح ret = 0;

  اگر ((c >= '0') && (c <= '9'))
    {
      ret = (c - '0');
    }
  ورنہ اگر ((c >= 'a') && (c <= 'f'))
    {
      ret = (c - 'a' + 0x0a);
    }
  ورنہ اگر ((c >= 'A') && (c <= 'F'))
    {
      ret = (c - 'A' + 0x0A);
    }
  واپس ret;
}

/* Convert a character to lower case */
__inline__ ساکن حرف
_tolower (حرف c)
{
  اگر ((c >= 'A') && (c <= 'Z'))
    {
      c = (c - 'A') + 'a';
    }
  واپس c;
}

/* Parse (scan) a number */
ساکن bool
parse_عدد (حرف *s, غیرعلامتی طویل *val, حرف **es, حرف *delim)
{
  bool پہلا = true;
  صحیح radix = 10;
  حرف c;
  غیرعلامتی طویل نتیجہ = 0;
  صحیح digit;

  جب (*s == ' ')
    s++;
  جب (*s)
    {
      اگر (پہلا && (s[0] == '0') && (_tolower (s[1]) == 'x'))
	{
	  radix = 16;
	  s += 2;
	}
      پہلا = false;
      c = *s++;
      اگر (_is_hex (c) && ((digit = _from_hex (c)) < radix))
	{
	  /* Valid digit */
	  نتیجہ = (نتیجہ * radix) + digit;
	}
      ورنہ
	{
	  اگر (delim != (حرف *) 0)
	    {
	      /* See if this character is one of the delimiters */
	      حرف *dp = delim;
	      جب (*dp && (c != *dp))
		dp++;
	      اگر (*dp)
		توڑو;		/* Found a good delimiter */
	    }
	  واپس false;		/* Malformatted number */
	}
    }
  *val = نتیجہ;
  اگر (es != (حرف **) 0)
    {
      *es = s;
    }
  واپس true;
}

#اگر defined(DEBUG) && !CONFIG_IS_ENABLED(USE_TINY_PRINTF)
/*
 * Note: this debug setup works by storing the strings in a fixed buffer
 */
ساکن حرف zm_debug_buf[8192];
ساکن حرف *zm_برآمد = zm_debug_buf;
ساکن حرف *zm_برآمد_آغاز = zm_debug_buf;

ساکن صحیح
zm_dprintf(حرف *fmt, ...)
{
	صحیح len;
	va_list دلائل;

	va_آغاز(دلائل, fmt);
	len = diag_vsprintf(zm_برآمد, fmt, دلائل);
	va_انتہا(دلائل);
	zm_برآمد += len;
	واپس len;
}

ساکن خالی
zm_flush (خالی)
{
  zm_برآمد = zm_برآمد_آغاز;
}

ساکن خالی
zm_dump_buf (خالی *buf, صحیح len)
{

}

ساکن غیرعلامتی حرف zm_buf[2048];
ساکن غیرعلامتی حرف *zm_bp;

ساکن خالی
zm_new (خالی)
{
  zm_bp = zm_buf;
}

ساکن خالی
zm_save (غیرعلامتی حرف c)
{
  *zm_bp++ = c;
}

ساکن خالی
zm_dump (صحیح سطر)
{
  zm_dprintf ("Packet at line: %d\n", سطر);
  zm_dump_buf (zm_buf, zm_bp - zm_buf);
}

#تعریف_کرو ZM_DEBUG(x) x
#ورنہ
#تعریف_کرو ZM_DEBUG(x)
#اگر_ختم

/* Wait for the line to go idle */
ساکن خالی
xyzModem_flush (خالی)
{
  صحیح res;
  حرف c;
  جب (true)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      اگر (!res)
	واپس;
    }
}

ساکن صحیح
xyzModem_get_hdr (خالی)
{
  حرف c;
  صحیح res;
  bool hdr_ملا = false;
  صحیح i, can_کل, hdr_chars;
  غیرعلامتی مختصر cksum;

  ZM_DEBUG (zm_new ());
  /* Find the start of a header */
  can_کل = 0;
  hdr_chars = 0;

  اگر (xyz.tx_ack)
    {
      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
      xyz.tx_ack = false;
    }
  جب (!hdr_ملا)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      اگر (res)
	{
	  hdr_chars++;
	  انتخاب (c)
	    {
	    حالت SOH:
	      xyz.کل_SOH++;
	      fallthrough;
	    حالت STX:
	      اگر (c == STX)
		xyz.کل_STX++;
	      hdr_ملا = true;
	      توڑو;
	    حالت CAN:
	    حالت ETX:
	      xyz.کل_CAN++;
	      ZM_DEBUG (zm_dump (__LINE__));
	      اگر (++can_کل == xyz_Modem_CAN_گنتی)
		{
		  واپس xyzModem_cancel;
		}
	      ورنہ
		{
		  /* Wait for multiple CAN to avoid early quits */
		  توڑو;
		}
	    حالت EOT:
	      /* EOT only supported if no noise */
	      اگر (hdr_chars == 1)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK on EOT #%d\n", __LINE__));
		  ZM_DEBUG (zm_dump (__LINE__));
		  واپس xyzModem_eof;
		}
	    طےشدہ:
	      /* Ignore, waiting for start of header */
	      ;
	    }
	}
      ورنہ
	{
	  /* Data stream timed out */
	  xyzModem_flush ();	/* Toss any current input */
	  ZM_DEBUG (zm_dump (__LINE__));
	  CYGACC_CALL_IF_DELAY_US ((cyg_int32) 250000);
	  واپس xyzModem_timeout;
	}
    }

  /* Header found, now read the data */
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (حرف *) &xyz.blk);
  ZM_DEBUG (zm_save (xyz.blk));
  اگر (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      واپس xyzModem_timeout;
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (حرف *) &xyz.cblk);
  ZM_DEBUG (zm_save (xyz.cblk));
  اگر (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      واپس xyzModem_timeout;
    }
  xyz.len = (c == SOH) ? 128 : 1024;
  xyz.bufp = xyz.pkt;
  برائے (i = 0; i < xyz.len; i++)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      اگر (res)
	{
	  xyz.pkt[i] = c;
	}
      ورنہ
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  واپس xyzModem_timeout;
	}
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (حرف *) &xyz.crc1);
  ZM_DEBUG (zm_save (xyz.crc1));
  اگر (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      واپس xyzModem_timeout;
    }
  اگر (xyz.چکری_جانچ_منوال)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (حرف *) &xyz.crc2);
      ZM_DEBUG (zm_save (xyz.crc2));
      اگر (!res)
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  واپس xyzModem_timeout;
	}
    }
  ZM_DEBUG (zm_dump (__LINE__));
  /* Validate the message */
  اگر ((xyz.blk ^ xyz.cblk) != (غیرعلامتی حرف) 0xFF)
    {
      ZM_DEBUG (zm_dprintf
		("Framing error - blk: %x/%x/%x\n", xyz.blk, xyz.cblk,
		 (xyz.blk ^ xyz.cblk)));
      ZM_DEBUG (zm_dump_buf (xyz.pkt, xyz.len));
      xyzModem_flush ();
      واپس xyz_Modem_چوکھٹا;
    }
  /* Verify checksum/CRC */
  اگر (xyz.چکری_جانچ_منوال)
    {
      cksum = crc16_ccitt(0, xyz.pkt, xyz.len);
      اگر (cksum != ((xyz.crc1 << 8) | xyz.crc2))
	{
	  ZM_DEBUG (zm_dprintf ("CRC error - recvd: %02x%02x, computed: %x\n",
				xyz.crc1, xyz.crc2, cksum & 0xFFFF));
	  واپس xyzModem_cksum;
	}
    }
  ورنہ
    {
      cksum = 0;
      برائے (i = 0; i < xyz.len; i++)
	{
	  cksum += xyz.pkt[i];
	}
      اگر (xyz.crc1 != (cksum & 0xFF))
	{
	  ZM_DEBUG (zm_dprintf
		    ("Checksum error - recvd: %x, computed: %x\n", xyz.crc1,
		     cksum & 0xFF));
	  واپس xyzModem_cksum;
	}
    }
  /* If we get here, the message passes [structural] muster */
  واپس 0;
}

ساکن
ulong
xyzModem_get_initial_timeout (خالی)
{
  /* timeout is in seconds, non-positive timeout value is infinity */
#اگر CONFIG_IS_ENABLED(ENV_SUPPORT)
  ثابت حرف *timeout_str = env_get("loadxy_timeout");
  اگر (timeout_str)
    واپس 1000 * simple_strtol(timeout_str, تہی, 10);
#اگر_ختم
  واپس 1000 * CONFIG_CMD_LOADXY_TIMEOUT;
}

صحیح
xyz_Modem_دھارا_open (connection_info_t * info, صحیح *err)
{
  صحیح stat = 0;
  صحیح retries = xyzModem_MAX_RETRIES;
  صحیح چکری_جانچ_retries = xyz_Modem_MAX_RETRIES_WITH_چکری_جانچ;

/*    ZM_DEBUG(zm_out = zm_out_start); */
#اگر_تعریف_شدہ xyzModem_zmodem
  اگر (info->منوال == xyzModem_zmodem)
    {
      *err = xyzModem_noZmodem;
      واپس -1;
    }
#اگر_ختم

/* TODO: CHECK ! */
  صحیح dummy = 0;
  xyz.__chan = &dummy;
  xyz.len = 0;
  xyz.pad_pending = 0;
  xyz.pad_deliver = 0;
  xyz.چکری_جانچ_منوال = true;
  xyz.at_eof = false;
  xyz.tx_ack = false;
  xyz.منوال = info->منوال;
  xyz.کل_retries = 0;
  xyz.کل_SOH = 0;
  xyz.کل_STX = 0;
  xyz.کل_CAN = 0;
  xyz.read_لمبائی = 0;
  xyz.file_لمبائی = 0;
  xyz.پہلا_xmodem_packet = false;
  xyz.initial_وقت = get_timer(0);
  xyz.timeout = xyzModem_get_initial_timeout();

  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.چکری_جانچ_منوال ? 'C' : NAK));

  اگر (xyz.منوال == xyzModem_xmodem)
    {
      /* X-modem doesn't have an information header - exit here */
      xyz.پہلا_xmodem_packet = true;
      xyz.اگلا_blk = 1;
      واپس 0;
    }

  جب (!(xyz.timeout && get_timer(xyz.initial_وقت) > xyz.timeout))
    {
      اگر (--retries <= 0)
        {
          retries = xyzModem_MAX_RETRIES;
          چکری_جانچ_retries = xyz_Modem_MAX_RETRIES_WITH_چکری_جانچ;
          xyz.چکری_جانچ_منوال = true;
        }
      stat = xyzModem_get_hdr ();
      اگر (stat == 0)
	{
	  /* Y-modem file information header */
	  اگر (xyz.blk == 0)
	    {
	      /* skip filename */
	      جب (*xyz.bufp++);
	      /* get the length */
	      parse_عدد ((حرف *) xyz.bufp, &xyz.file_لمبائی, تہی, " ");
	      /* The rest of the file name data block quietly discarded */
	      xyz.tx_ack = true;
	    }
	  xyz.اگلا_blk = 1;
	  xyz.len = 0;
	  واپس 0;
	}
      ورنہ اگر (stat == xyzModem_timeout)
	{
	  اگر (--چکری_جانچ_retries <= 0)
	    xyz.چکری_جانچ_منوال = false;
	  CYGACC_CALL_IF_DELAY_US (5 * 100000);	/* Extra delay for startup */
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.چکری_جانچ_منوال ? 'C' : NAK));
	  xyz.کل_retries++;
	  ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	}
      اگر (stat == xyzModem_cancel)
	{
	  توڑو;
	}
    }
  *err = stat;
  ZM_DEBUG (zm_flush ());
  واپس -1;
}

صحیح
xyz_Modem_دھارا_read (حرف *buf, صحیح سائز, صحیح *err)
{
  صحیح stat, کل, len;
  صحیح retries;

  کل = 0;
  stat = xyzModem_cancel;
  /* Try and get 'size' bytes into the buffer */
  جب (!xyz.at_eof && xyz.len >= 0 && (سائز > 0))
    {
      اگر (xyz.len == 0 && xyz.pad_deliver == 0)
	{
	  retries = xyzModem_MAX_RETRIES;
	  جب (retries-- > 0)
	    {
	      اگر (xyz.پہلا_xmodem_packet && xyz.timeout &&
		  get_timer(xyz.initial_وقت) > xyz.timeout)
		{
		  *err = xyzModem_timeout;
		  xyz.len = -1;
		  واپس کل;
		}

	      stat = xyzModem_get_hdr ();
	      اگر (stat == 0)
		{
		  اگر (xyz.منوال == xyzModem_xmodem && xyz.پہلا_xmodem_packet)
		    xyz.پہلا_xmodem_packet = false;
		  اگر (xyz.blk == xyz.اگلا_blk)
		    {
		      xyz.tx_ack = true;
		      ZM_DEBUG (zm_dprintf
				("ACK block %d (%d)\n", xyz.blk, __LINE__));
		      xyz.اگلا_blk = (xyz.اگلا_blk + 1) & 0xFF;

		      اگر (xyz.منوال == xyzModem_xmodem || xyz.file_لمبائی == 0)
			{
			  /* Without a file length, EOF padding can't be
			   * distinguished from block data until the next
			   * frame arrives and we can check for EOT/EOF.
			   */
			  xyz.pad_deliver = xyz.pad_pending;
			  xyz.pad_pending = 0;
			  اگر ((xyz.bufp[xyz.len - 1] == EOF) &&
			      (xyz.bufp[xyz.len - 2] == EOF) &&
			      (xyz.bufp[xyz.len - 3] == EOF))
			    {
			      جب (xyz.len
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
		      اگر (xyz.منوال != xyzModem_xmodem && 0 != xyz.file_لمبائی)
			{
			  xyz.read_لمبائی += xyz.len;
			  اگر (xyz.read_لمبائی > xyz.file_لمبائی)
			    {
			      xyz.len -= (xyz.read_لمبائی - xyz.file_لمبائی);
			    }
			}
		      توڑو;
		    }
		  ورنہ اگر (xyz.blk == ((xyz.اگلا_blk - 1) & 0xFF))
		    {
		      /* Just re-ACK this so sender will get on with it */
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      جاری;	/* Need new header */
		    }
		  ورنہ
		    {
		      stat = xyzModem_sequence;
		    }
		}
	      اگر (stat == xyzModem_cancel)
		{
		  توڑو;
		}
	      اگر (stat == xyzModem_eof)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK (%d)\n", __LINE__));
		  اگر (xyz.منوال == xyzModem_ymodem)
		    {
		      CYGACC_COMM_IF_PUTC (*xyz.__chan,
					   (xyz.چکری_جانچ_منوال ? 'C' : NAK));
		      xyz.کل_retries++;
		      ZM_DEBUG (zm_dprintf ("Reading Final Header\n"));
		      stat = xyzModem_get_hdr ();
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ZM_DEBUG (zm_dprintf ("FINAL ACK (%d)\n", __LINE__));
		    }
		  ورنہ
		    stat = 0;

		  xyz.pad_pending = 0;	/* EOT: withheld ^Z run was padding */
		  xyz.at_eof = true;
		  توڑو;
		}
	      CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.چکری_جانچ_منوال ? 'C' : NAK));
	      xyz.کل_retries++;
	      ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	    }
	  اگر (stat < 0 && (!xyz.پہلا_xmodem_packet || stat != xyzModem_timeout))
	    {
	      *err = stat;
	      xyz.len = -1;
	      واپس کل;
	    }
	}
      /* A ^Z run withheld from the previous block turned out to be data */
      اگر (xyz.pad_deliver > 0)
	{
	  len = xyz.pad_deliver;
	  اگر (سائز < len)
	    len = سائز;
	  memset (buf, EOF, len);
	  سائز -= len;
	  buf += len;
	  کل += len;
	  xyz.pad_deliver -= len;
	}
      /* Don't "read" data from the EOF protocol package */
      اگر (!xyz.at_eof && xyz.len > 0)
	{
	  len = xyz.len;
	  اگر (سائز < len)
	    len = سائز;
	  حافظہ_نقل (buf, xyz.bufp, len);
	  سائز -= len;
	  buf += len;
	  کل += len;
	  xyz.len -= len;
	  xyz.bufp += len;
	}
    }
  واپس کل;
}

خالی
xyz_Modem_دھارا_close (صحیح *err)
{
  ZM_DEBUG (zm_dprintf
    ("xyzModem - %s mode, %d(SOH)/%d(STX)/%d(CAN) packets, %d retries\n",
     xyz.چکری_جانچ_منوال ? "CRC" : "Cksum", xyz.کل_SOH, xyz.کل_STX,
     xyz.کل_CAN, xyz.کل_retries));
  ZM_DEBUG (zm_flush ());
}

/* Need to be able to clean out the input buffer, so have to take the */
/* getc */
خالی
xyz_Modem_دھارا_terminate (bool abort, صحیح (*getc) (خالی))
{
  صحیح c;

  اگر (abort)
    {
      ZM_DEBUG (zm_dprintf ("!!!! TRANSFER ABORT !!!!\n"));
      انتخاب (xyz.منوال)
	{
	حالت xyzModem_xmodem:
	حالت xyzModem_ymodem:
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
	  توڑو;
#اگر_تعریف_شدہ xyzModem_zmodem
	حالت xyzModem_zmodem:
	  /* Might support it some day I suppose. */
#اگر_ختم
	  توڑو;
	}
    }
  ورنہ
    {
      ZM_DEBUG (zm_dprintf ("Engaging cleanup mode...\n"));
      /*
       * Consume any trailing crap left in the inbuffer from
       * previous received blocks. Since very few files are an exact multiple
       * of the transfer block size, there will almost always be some gunk here.
       * If we don't eat it now, RedBoot will think the user typed it.
       */
      ZM_DEBUG (zm_dprintf ("Trailing gunk:\n"));
      جب ((c = (*getc) ()) > -1)
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
xyzModem_error (صحیح err)
{
  انتخاب (err)
    {
    حالت xyzModem_access:
      واپس "Can't access file";
      توڑو;
    حالت xyzModem_noZmodem:
      واپس "Sorry, zModem not available yet";
      توڑو;
    حالت xyzModem_timeout:
      واپس "Timed out";
      توڑو;
    حالت xyzModem_eof:
      واپس "End of file";
      توڑو;
    حالت xyzModem_cancel:
      واپس "Cancelled";
      توڑو;
    حالت xyz_Modem_چوکھٹا:
      واپس "Invalid framing";
      توڑو;
    حالت xyzModem_cksum:
      واپس "CRC/checksum error";
      توڑو;
    حالت xyzModem_sequence:
      واپس "Block sequence error";
      توڑو;
    طےشدہ:
      واپس "Unknown error";
      توڑو;
    }
}

/*
 * RedBoot interface
 */
