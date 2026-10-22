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
#অন্তর্ভাৱ <xyzModem.h>
#অন্তর্ভাৱ <stdarg.h>
#অন্তর্ভাৱ <time.h>
#অন্তর্ভাৱ <u-boot/crc.h>
#অন্তর্ভাৱ <watchdog.h>
#অন্তর্ভাৱ <env.h>
#অন্তর্ভাৱ <vsprintf.h>

/* Assumption - run xyzModem protocol over the console port */

/* Values magic to the protocol */
#define SOH 0x01
#define STX 0x02
#define ETX 0x03		/* ^C for interrupt */
#define EOT 0x04
#define ACK 0x06
#define BSP 0x08
#define NAK 0x15
#define CAN 0x18
#define EOF 0x1A		/* ^Z for DOS officionados */

/* Data & state local to the protocol */
স্থির সংরচনা
{
  পূর্ণাঙ্ক *__chan;
  unsigned ৱর্ণ pkt[1024], *bufp;
  পূর্ণাঙ্ক pad_pending;	/* trailing ^Z run withheld from the previous block */
  পূর্ণাঙ্ক pad_deliver;	/* ^Z bytes to emit before the next data */
  unsigned ৱর্ণ blk, cblk, crc1, crc2;
  unsigned ৱর্ণ next_blk;	/* Expected block */
  পূর্ণাঙ্ক len, mode, total_retries;
  পূর্ণাঙ্ক total_SOH, total_STX, total_CAN;
  bool crc_mode, at_eof, tx_ack;
  bool first_xmodem_packet;
  ulong initial_time, timeout;
  unsigned long file_length, read_length;
} xyz;

#define xyzModem_CHAR_TIMEOUT            2000	/* 2 seconds */
#define xyzModem_MAX_RETRIES             20
#define xyzModem_MAX_RETRIES_WITH_CRC    10
#define xyz_Modem_CAN_গুনতি                3	/* Wait for 3 CAN before quitting */

প্রকারনির্দেশ পূর্ণাঙ্ক cyg_int32;
স্থির পূর্ণাঙ্ক
CYGACC_COMM_IF_GETC_TIMEOUT (ৱর্ণ chan, ৱর্ণ *c)
{

  ulong now = get_timer(0);
  schedule();
  যেতিয়ালৈ (!tstc ())
    {
      যদি (get_timer(now) > xyzModem_CHAR_TIMEOUT)
        ভঙ্গ;
    }
  যদি (tstc ())
    {
      *c = getchar();
      ঘূৰাই 1;
    }
  ঘূৰাই 0;
}

স্থির শূন্য
CYGACC_COMM_IF_PUTC (ৱর্ণ x, ৱর্ণ y)
{
  putc (y);
}

/* Validate a hex character */
__inline__ স্থির bool
_is_hex (ৱর্ণ c)
{
  ঘূৰাই (((c >= '0') && (c <= '9')) ||
	  ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

/* Convert a single hex nibble */
__inline__ স্থির পূর্ণাঙ্ক
_from_hex (ৱর্ণ c)
{
  পূর্ণাঙ্ক ret = 0;

  যদি ((c >= '0') && (c <= '9'))
    {
      ret = (c - '0');
    }
  অন্যথা যদি ((c >= 'a') && (c <= 'f'))
    {
      ret = (c - 'a' + 0x0a);
    }
  অন্যথা যদি ((c >= 'A') && (c <= 'F'))
    {
      ret = (c - 'A' + 0x0A);
    }
  ঘূৰাই ret;
}

/* Convert a character to lower case */
__inline__ স্থির ৱর্ণ
_tolower (ৱর্ণ c)
{
  যদি ((c >= 'A') && (c <= 'Z'))
    {
      c = (c - 'A') + 'a';
    }
  ঘূৰাই c;
}

/* Parse (scan) a number */
স্থির bool
parse_অঙ্ক (ৱর্ণ *s, unsigned long *val, ৱর্ণ **es, ৱর্ণ *delim)
{
  bool first = true;
  পূর্ণাঙ্ক radix = 10;
  ৱর্ণ c;
  unsigned long result = 0;
  পূর্ণাঙ্ক digit;

  যেতিয়ালৈ (*s == ' ')
    s++;
  যেতিয়ালৈ (*s)
    {
      যদি (first && (s[0] == '0') && (_tolower (s[1]) == 'x'))
	{
	  radix = 16;
	  s += 2;
	}
      first = false;
      c = *s++;
      যদি (_is_hex (c) && ((digit = _from_hex (c)) < radix))
	{
	  /* Valid digit */
	  result = (result * radix) + digit;
	}
      অন্যথা
	{
	  যদি (delim != (ৱর্ণ *) 0)
	    {
	      /* See if this character is one of the delimiters */
	      ৱর্ণ *dp = delim;
	      যেতিয়ালৈ (*dp && (c != *dp))
		dp++;
	      যদি (*dp)
		ভঙ্গ;		/* Found a good delimiter */
	    }
	  ঘূৰাই false;		/* Malformatted number */
	}
    }
  *val = result;
  যদি (es != (ৱর্ণ **) 0)
    {
      *es = s;
    }
  ঘূৰাই true;
}

#যদি defined(DEBUG) && !CONFIG_IS_ENABLED(USE_TINY_PRINTF)
/*
 * Note: this debug setup works by storing the strings in a fixed buffer
 */
স্থির ৱর্ণ zm_debug_buf[8192];
স্থির ৱর্ণ *zm_out = zm_debug_buf;
স্থির ৱর্ণ *zm_out_start = zm_debug_buf;

স্থির পূর্ণাঙ্ক
zm_dprintf(ৱর্ণ *fmt, ...)
{
	পূর্ণাঙ্ক len;
	va_list args;

	va_start(args, fmt);
	len = diag_vsprintf(zm_out, fmt, args);
	va_end(args);
	zm_out += len;
	ঘূৰাই len;
}

স্থির শূন্য
zm_flush (শূন্য)
{
  zm_out = zm_out_start;
}

স্থির শূন্য
zm_dump_buf (শূন্য *buf, পূর্ণাঙ্ক len)
{

}

স্থির unsigned ৱর্ণ zm_buf[2048];
স্থির unsigned ৱর্ণ *zm_bp;

স্থির শূন্য
zm_new (শূন্য)
{
  zm_bp = zm_buf;
}

স্থির শূন্য
zm_save (unsigned ৱর্ণ c)
{
  *zm_bp++ = c;
}

স্থির শূন্য
zm_dump (পূর্ণাঙ্ক সাৰি)
{
  zm_dprintf ("Packet at line: %d\n", সাৰি);
  zm_dump_buf (zm_buf, zm_bp - zm_buf);
}

#define ZM_DEBUG(x) x
#অন্যথা
#define ZM_DEBUG(x)
#endif

/* Wait for the line to go idle */
স্থির শূন্য
xyzModem_flush (শূন্য)
{
  পূর্ণাঙ্ক res;
  ৱর্ণ c;
  যেতিয়ালৈ (true)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      যদি (!res)
	ঘূৰাই;
    }
}

স্থির পূর্ণাঙ্ক
xyzModem_get_hdr (শূন্য)
{
  ৱর্ণ c;
  পূর্ণাঙ্ক res;
  bool hdr_found = false;
  পূর্ণাঙ্ক i, can_total, hdr_chars;
  unsigned short cksum;

  ZM_DEBUG (zm_new ());
  /* Find the start of a header */
  can_total = 0;
  hdr_chars = 0;

  যদি (xyz.tx_ack)
    {
      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
      xyz.tx_ack = false;
    }
  যেতিয়ালৈ (!hdr_found)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      যদি (res)
	{
	  hdr_chars++;
	  ৱিকল্প (c)
	    {
	    পক্ষ SOH:
	      xyz.total_SOH++;
	      fallthrough;
	    পক্ষ STX:
	      যদি (c == STX)
		xyz.total_STX++;
	      hdr_found = true;
	      ভঙ্গ;
	    পক্ষ CAN:
	    পক্ষ ETX:
	      xyz.total_CAN++;
	      ZM_DEBUG (zm_dump (__LINE__));
	      যদি (++can_total == xyz_Modem_CAN_গুনতি)
		{
		  ঘূৰাই xyzModem_cancel;
		}
	      অন্যথা
		{
		  /* Wait for multiple CAN to avoid early quits */
		  ভঙ্গ;
		}
	    পক্ষ EOT:
	      /* EOT only supported if no noise */
	      যদি (hdr_chars == 1)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK on EOT #%d\n", __LINE__));
		  ZM_DEBUG (zm_dump (__LINE__));
		  ঘূৰাই xyzModem_eof;
		}
	    স্ৱভাৱ:
	      /* Ignore, waiting for start of header */
	      ;
	    }
	}
      অন্যথা
	{
	  /* Data stream timed out */
	  xyzModem_flush ();	/* Toss any current input */
	  ZM_DEBUG (zm_dump (__LINE__));
	  CYGACC_CALL_IF_DELAY_US ((cyg_int32) 250000);
	  ঘূৰাই xyzModem_timeout;
	}
    }

  /* Header found, now read the data */
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (ৱর্ণ *) &xyz.blk);
  ZM_DEBUG (zm_save (xyz.blk));
  যদি (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ঘূৰাই xyzModem_timeout;
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (ৱর্ণ *) &xyz.cblk);
  ZM_DEBUG (zm_save (xyz.cblk));
  যদি (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ঘূৰাই xyzModem_timeout;
    }
  xyz.len = (c == SOH) ? 128 : 1024;
  xyz.bufp = xyz.pkt;
  বাবে (i = 0; i < xyz.len; i++)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      যদি (res)
	{
	  xyz.pkt[i] = c;
	}
      অন্যথা
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  ঘূৰাই xyzModem_timeout;
	}
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (ৱর্ণ *) &xyz.crc1);
  ZM_DEBUG (zm_save (xyz.crc1));
  যদি (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ঘূৰাই xyzModem_timeout;
    }
  যদি (xyz.crc_mode)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (ৱর্ণ *) &xyz.crc2);
      ZM_DEBUG (zm_save (xyz.crc2));
      যদি (!res)
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  ঘূৰাই xyzModem_timeout;
	}
    }
  ZM_DEBUG (zm_dump (__LINE__));
  /* Validate the message */
  যদি ((xyz.blk ^ xyz.cblk) != (unsigned ৱর্ণ) 0xFF)
    {
      ZM_DEBUG (zm_dprintf
		("Framing error - blk: %x/%x/%x\n", xyz.blk, xyz.cblk,
		 (xyz.blk ^ xyz.cblk)));
      ZM_DEBUG (zm_dump_buf (xyz.pkt, xyz.len));
      xyzModem_flush ();
      ঘূৰাই xyzModem_frame;
    }
  /* Verify checksum/CRC */
  যদি (xyz.crc_mode)
    {
      cksum = crc16_ccitt(0, xyz.pkt, xyz.len);
      যদি (cksum != ((xyz.crc1 << 8) | xyz.crc2))
	{
	  ZM_DEBUG (zm_dprintf ("CRC error - recvd: %02x%02x, computed: %x\n",
				xyz.crc1, xyz.crc2, cksum & 0xFFFF));
	  ঘূৰাই xyzModem_cksum;
	}
    }
  অন্যথা
    {
      cksum = 0;
      বাবে (i = 0; i < xyz.len; i++)
	{
	  cksum += xyz.pkt[i];
	}
      যদি (xyz.crc1 != (cksum & 0xFF))
	{
	  ZM_DEBUG (zm_dprintf
		    ("Checksum error - recvd: %x, computed: %x\n", xyz.crc1,
		     cksum & 0xFF));
	  ঘূৰাই xyzModem_cksum;
	}
    }
  /* If we get here, the message passes [structural] muster */
  ঘূৰাই 0;
}

স্থির
ulong
xyzModem_get_initial_timeout (শূন্য)
{
  /* timeout is in seconds, non-positive timeout value is infinity */
#যদি CONFIG_IS_ENABLED(ENV_SUPPORT)
  ধ্রুৱ ৱর্ণ *timeout_str = env_get("loadxy_timeout");
  যদি (timeout_str)
    ঘূৰাই 1000 * simple_strtol(timeout_str, রিক্ত, 10);
#endif
  ঘূৰাই 1000 * CONFIG_CMD_LOADXY_TIMEOUT;
}

পূর্ণাঙ্ক
xyzModem_stream_open (connection_info_t * info, পূর্ণাঙ্ক *err)
{
  পূর্ণাঙ্ক stat = 0;
  পূর্ণাঙ্ক retries = xyzModem_MAX_RETRIES;
  পূর্ণাঙ্ক crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;

/*    ZM_DEBUG(zm_out = zm_out_start); */
#ifdef xyzModem_zmodem
  যদি (info->mode == xyzModem_zmodem)
    {
      *err = xyzModem_noZmodem;
      ঘূৰাই -1;
    }
#endif

/* TODO: CHECK ! */
  পূর্ণাঙ্ক dummy = 0;
  xyz.__chan = &dummy;
  xyz.len = 0;
  xyz.pad_pending = 0;
  xyz.pad_deliver = 0;
  xyz.crc_mode = true;
  xyz.at_eof = false;
  xyz.tx_ack = false;
  xyz.mode = info->mode;
  xyz.total_retries = 0;
  xyz.total_SOH = 0;
  xyz.total_STX = 0;
  xyz.total_CAN = 0;
  xyz.read_length = 0;
  xyz.file_length = 0;
  xyz.first_xmodem_packet = false;
  xyz.initial_time = get_timer(0);
  xyz.timeout = xyzModem_get_initial_timeout();

  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));

  যদি (xyz.mode == xyzModem_xmodem)
    {
      /* X-modem doesn't have an information header - exit here */
      xyz.first_xmodem_packet = true;
      xyz.next_blk = 1;
      ঘূৰাই 0;
    }

  যেতিয়ালৈ (!(xyz.timeout && get_timer(xyz.initial_time) > xyz.timeout))
    {
      যদি (--retries <= 0)
        {
          retries = xyzModem_MAX_RETRIES;
          crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;
          xyz.crc_mode = true;
        }
      stat = xyzModem_get_hdr ();
      যদি (stat == 0)
	{
	  /* Y-modem file information header */
	  যদি (xyz.blk == 0)
	    {
	      /* skip filename */
	      যেতিয়ালৈ (*xyz.bufp++);
	      /* get the length */
	      parse_অঙ্ক ((ৱর্ণ *) xyz.bufp, &xyz.file_length, রিক্ত, " ");
	      /* The rest of the file name data block quietly discarded */
	      xyz.tx_ack = true;
	    }
	  xyz.next_blk = 1;
	  xyz.len = 0;
	  ঘূৰাই 0;
	}
      অন্যথা যদি (stat == xyzModem_timeout)
	{
	  যদি (--crc_retries <= 0)
	    xyz.crc_mode = false;
	  CYGACC_CALL_IF_DELAY_US (5 * 100000);	/* Extra delay for startup */
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	  xyz.total_retries++;
	  ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	}
      যদি (stat == xyzModem_cancel)
	{
	  ভঙ্গ;
	}
    }
  *err = stat;
  ZM_DEBUG (zm_flush ());
  ঘূৰাই -1;
}

পূর্ণাঙ্ক
xyzModem_stream_read (ৱর্ণ *buf, পূর্ণাঙ্ক size, পূর্ণাঙ্ক *err)
{
  পূর্ণাঙ্ক stat, total, len;
  পূর্ণাঙ্ক retries;

  total = 0;
  stat = xyzModem_cancel;
  /* Try and get 'size' bytes into the buffer */
  যেতিয়ালৈ (!xyz.at_eof && xyz.len >= 0 && (size > 0))
    {
      যদি (xyz.len == 0 && xyz.pad_deliver == 0)
	{
	  retries = xyzModem_MAX_RETRIES;
	  যেতিয়ালৈ (retries-- > 0)
	    {
	      যদি (xyz.first_xmodem_packet && xyz.timeout &&
		  get_timer(xyz.initial_time) > xyz.timeout)
		{
		  *err = xyzModem_timeout;
		  xyz.len = -1;
		  ঘূৰাই total;
		}

	      stat = xyzModem_get_hdr ();
	      যদি (stat == 0)
		{
		  যদি (xyz.mode == xyzModem_xmodem && xyz.first_xmodem_packet)
		    xyz.first_xmodem_packet = false;
		  যদি (xyz.blk == xyz.next_blk)
		    {
		      xyz.tx_ack = true;
		      ZM_DEBUG (zm_dprintf
				("ACK block %d (%d)\n", xyz.blk, __LINE__));
		      xyz.next_blk = (xyz.next_blk + 1) & 0xFF;

		      যদি (xyz.mode == xyzModem_xmodem || xyz.file_length == 0)
			{
			  /* Without a file length, EOF padding can't be
			   * distinguished from block data until the next
			   * frame arrives and we can check for EOT/EOF.
			   */
			  xyz.pad_deliver = xyz.pad_pending;
			  xyz.pad_pending = 0;
			  যদি ((xyz.bufp[xyz.len - 1] == EOF) &&
			      (xyz.bufp[xyz.len - 2] == EOF) &&
			      (xyz.bufp[xyz.len - 3] == EOF))
			    {
			      যেতিয়ালৈ (xyz.len
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
		      যদি (xyz.mode != xyzModem_xmodem && 0 != xyz.file_length)
			{
			  xyz.read_length += xyz.len;
			  যদি (xyz.read_length > xyz.file_length)
			    {
			      xyz.len -= (xyz.read_length - xyz.file_length);
			    }
			}
		      ভঙ্গ;
		    }
		  অন্যথা যদি (xyz.blk == ((xyz.next_blk - 1) & 0xFF))
		    {
		      /* Just re-ACK this so sender will get on with it */
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      অনুৱর্ত;	/* Need new header */
		    }
		  অন্যথা
		    {
		      stat = xyzModem_sequence;
		    }
		}
	      যদি (stat == xyzModem_cancel)
		{
		  ভঙ্গ;
		}
	      যদি (stat == xyzModem_eof)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK (%d)\n", __LINE__));
		  যদি (xyz.mode == xyzModem_ymodem)
		    {
		      CYGACC_COMM_IF_PUTC (*xyz.__chan,
					   (xyz.crc_mode ? 'C' : NAK));
		      xyz.total_retries++;
		      ZM_DEBUG (zm_dprintf ("Reading Final Header\n"));
		      stat = xyzModem_get_hdr ();
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ZM_DEBUG (zm_dprintf ("FINAL ACK (%d)\n", __LINE__));
		    }
		  অন্যথা
		    stat = 0;

		  xyz.pad_pending = 0;	/* EOT: withheld ^Z run was padding */
		  xyz.at_eof = true;
		  ভঙ্গ;
		}
	      CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	      xyz.total_retries++;
	      ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	    }
	  যদি (stat < 0 && (!xyz.first_xmodem_packet || stat != xyzModem_timeout))
	    {
	      *err = stat;
	      xyz.len = -1;
	      ঘূৰাই total;
	    }
	}
      /* A ^Z run withheld from the previous block turned out to be data */
      যদি (xyz.pad_deliver > 0)
	{
	  len = xyz.pad_deliver;
	  যদি (size < len)
	    len = size;
	  memset (buf, EOF, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.pad_deliver -= len;
	}
      /* Don't "read" data from the EOF protocol package */
      যদি (!xyz.at_eof && xyz.len > 0)
	{
	  len = xyz.len;
	  যদি (size < len)
	    len = size;
	  memcpy (buf, xyz.bufp, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.len -= len;
	  xyz.bufp += len;
	}
    }
  ঘূৰাই total;
}

শূন্য
xyzModem_stream_close (পূর্ণাঙ্ক *err)
{
  ZM_DEBUG (zm_dprintf
    ("xyzModem - %s mode, %d(SOH)/%d(STX)/%d(CAN) packets, %d retries\n",
     xyz.crc_mode ? "CRC" : "Cksum", xyz.total_SOH, xyz.total_STX,
     xyz.total_CAN, xyz.total_retries));
  ZM_DEBUG (zm_flush ());
}

/* Need to be able to clean out the input buffer, so have to take the */
/* getc */
শূন্য
xyzModem_stream_terminate (bool abort, পূর্ণাঙ্ক (*getc) (শূন্য))
{
  পূর্ণাঙ্ক c;

  যদি (abort)
    {
      ZM_DEBUG (zm_dprintf ("!!!! TRANSFER ABORT !!!!\n"));
      ৱিকল্প (xyz.mode)
	{
	পক্ষ xyzModem_xmodem:
	পক্ষ xyzModem_ymodem:
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
	  ভঙ্গ;
#ifdef xyzModem_zmodem
	পক্ষ xyzModem_zmodem:
	  /* Might support it some day I suppose. */
#endif
	  ভঙ্গ;
	}
    }
  অন্যথা
    {
      ZM_DEBUG (zm_dprintf ("Engaging cleanup mode...\n"));
      /*
       * Consume any trailing crap left in the inbuffer from
       * previous received blocks. Since very few files are an exact multiple
       * of the transfer block size, there will almost always be some gunk here.
       * If we don't eat it now, RedBoot will think the user typed it.
       */
      ZM_DEBUG (zm_dprintf ("Trailing gunk:\n"));
      যেতিয়ালৈ ((c = (*getc) ()) > -1)
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

ৱর্ণ *
xyzModem_error (পূর্ণাঙ্ক err)
{
  ৱিকল্প (err)
    {
    পক্ষ xyzModem_access:
      ঘূৰাই "Can't access file";
      ভঙ্গ;
    পক্ষ xyzModem_noZmodem:
      ঘূৰাই "Sorry, zModem not available yet";
      ভঙ্গ;
    পক্ষ xyzModem_timeout:
      ঘূৰাই "Timed out";
      ভঙ্গ;
    পক্ষ xyzModem_eof:
      ঘূৰাই "End of file";
      ভঙ্গ;
    পক্ষ xyzModem_cancel:
      ঘূৰাই "Cancelled";
      ভঙ্গ;
    পক্ষ xyzModem_frame:
      ঘূৰাই "Invalid framing";
      ভঙ্গ;
    পক্ষ xyzModem_cksum:
      ঘূৰাই "CRC/checksum error";
      ভঙ্গ;
    পক্ষ xyzModem_sequence:
      ঘূৰাই "Block sequence error";
      ভঙ্গ;
    স্ৱভাৱ:
      ঘূৰাই "Unknown error";
      ভঙ্গ;
    }
}

/*
 * RedBoot interface
 */
