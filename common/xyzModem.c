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
#অন্তর্ভাব <xyzModem.h>
#অন্তর্ভাব <stdarg.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <u-boot/crc.h>
#অন্তর্ভাব <watchdog.h>
#অন্তর্ভাব <env.h>
#অন্তর্ভাব <vsprintf.h>

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
স্থির কাঠামো
{
  পূর্ণসংখ্যা *__chan;
  unsigned অক্ষর pkt[1024], *bufp;
  পূর্ণসংখ্যা pad_pending;	/* trailing ^Z run withheld from the previous block */
  পূর্ণসংখ্যা pad_deliver;	/* ^Z bytes to emit before the next data */
  unsigned অক্ষর blk, cblk, crc1, crc2;
  unsigned অক্ষর পরবর্তী_blk;	/* Expected block */
  পূর্ণসংখ্যা len, mode, যোগ_retries;
  পূর্ণসংখ্যা যোগ_SOH, যোগ_STX, যোগ_CAN;
  bool crc_mode, at_eof, tx_ack;
  bool first_xmodem_packet;
  ulong initial_time, timeout;
  unsigned long file_দৈর্ঘ্য, read_দৈর্ঘ্য;
} xyz;

#define xyzModem_CHAR_TIMEOUT            2000	/* 2 seconds */
#define xyzModem_MAX_RETRIES             20
#define xyzModem_MAX_RETRIES_WITH_CRC    10
#define xyz_Modem_CAN_গুনতি                3	/* Wait for 3 CAN before quitting */

প্রকারলেখ পূর্ণসংখ্যা cyg_int32;
স্থির পূর্ণসংখ্যা
CYGACC_COMM_IF_GETC_TIMEOUT (অক্ষর chan, অক্ষর *c)
{

  ulong now = get_timer(0);
  schedule();
  যখন (!tstc ())
    {
      যদি (get_timer(now) > xyzModem_CHAR_TIMEOUT)
        বিরতি;
    }
  যদি (tstc ())
    {
      *c = getchar();
      ফেরত 1;
    }
  ফেরত 0;
}

স্থির শূন্য
CYGACC_COMM_IF_PUTC (অক্ষর x, অক্ষর y)
{
  putc (y);
}

/* Validate a hex character */
__inline__ স্থির bool
_is_hex (অক্ষর c)
{
  ফেরত (((c >= '0') && (c <= '9')) ||
	  ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

/* Convert a single hex nibble */
__inline__ স্থির পূর্ণসংখ্যা
_from_hex (অক্ষর c)
{
  পূর্ণসংখ্যা ret = 0;

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
  ফেরত ret;
}

/* Convert a character to lower case */
__inline__ স্থির অক্ষর
_tolower (অক্ষর c)
{
  যদি ((c >= 'A') && (c <= 'Z'))
    {
      c = (c - 'A') + 'a';
    }
  ফেরত c;
}

/* Parse (scan) a number */
স্থির bool
parse_অঙ্ক (অক্ষর *s, unsigned long *val, অক্ষর **es, অক্ষর *delim)
{
  bool first = true;
  পূর্ণসংখ্যা radix = 10;
  অক্ষর c;
  unsigned long ফলাফল = 0;
  পূর্ণসংখ্যা digit;

  যখন (*s == ' ')
    s++;
  যখন (*s)
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
	  ফলাফল = (ফলাফল * radix) + digit;
	}
      অন্যথা
	{
	  যদি (delim != (অক্ষর *) 0)
	    {
	      /* See if this character is one of the delimiters */
	      অক্ষর *dp = delim;
	      যখন (*dp && (c != *dp))
		dp++;
	      যদি (*dp)
		বিরতি;		/* Found a good delimiter */
	    }
	  ফেরত false;		/* Malformatted number */
	}
    }
  *val = ফলাফল;
  যদি (es != (অক্ষর **) 0)
    {
      *es = s;
    }
  ফেরত true;
}

#যদি defined(DEBUG) && !CONFIG_IS_ENABLED(USE_TINY_PRINTF)
/*
 * Note: this debug setup works by storing the strings in a fixed buffer
 */
স্থির অক্ষর zm_debug_buf[8192];
স্থির অক্ষর *zm_out = zm_debug_buf;
স্থির অক্ষর *zm_out_start = zm_debug_buf;

স্থির পূর্ণসংখ্যা
zm_dprintf(অক্ষর *fmt, ...)
{
	পূর্ণসংখ্যা len;
	va_list args;

	va_start(args, fmt);
	len = diag_vsprintf(zm_out, fmt, args);
	va_end(args);
	zm_out += len;
	ফেরত len;
}

স্থির শূন্য
zm_flush (শূন্য)
{
  zm_out = zm_out_start;
}

স্থির শূন্য
zm_dump_buf (শূন্য *buf, পূর্ণসংখ্যা len)
{

}

স্থির unsigned অক্ষর zm_buf[2048];
স্থির unsigned অক্ষর *zm_bp;

স্থির শূন্য
zm_new (শূন্য)
{
  zm_bp = zm_buf;
}

স্থির শূন্য
zm_save (unsigned অক্ষর c)
{
  *zm_bp++ = c;
}

স্থির শূন্য
zm_dump (পূর্ণসংখ্যা সারি)
{
  zm_dprintf ("Packet at line: %d\n", সারি);
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
  পূর্ণসংখ্যা res;
  অক্ষর c;
  যখন (true)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      যদি (!res)
	ফেরত;
    }
}

স্থির পূর্ণসংখ্যা
xyzModem_get_hdr (শূন্য)
{
  অক্ষর c;
  পূর্ণসংখ্যা res;
  bool hdr_found = false;
  পূর্ণসংখ্যা i, can_যোগ, hdr_chars;
  unsigned short cksum;

  ZM_DEBUG (zm_new ());
  /* Find the start of a header */
  can_যোগ = 0;
  hdr_chars = 0;

  যদি (xyz.tx_ack)
    {
      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
      xyz.tx_ack = false;
    }
  যখন (!hdr_found)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      যদি (res)
	{
	  hdr_chars++;
	  নির্বাচন (c)
	    {
	    ক্ষেত্র SOH:
	      xyz.যোগ_SOH++;
	      fallthrough;
	    ক্ষেত্র STX:
	      যদি (c == STX)
		xyz.যোগ_STX++;
	      hdr_found = true;
	      বিরতি;
	    ক্ষেত্র CAN:
	    ক্ষেত্র ETX:
	      xyz.যোগ_CAN++;
	      ZM_DEBUG (zm_dump (__LINE__));
	      যদি (++can_যোগ == xyz_Modem_CAN_গুনতি)
		{
		  ফেরত xyzModem_cancel;
		}
	      অন্যথা
		{
		  /* Wait for multiple CAN to avoid early quits */
		  বিরতি;
		}
	    ক্ষেত্র EOT:
	      /* EOT only supported if no noise */
	      যদি (hdr_chars == 1)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK on EOT #%d\n", __LINE__));
		  ZM_DEBUG (zm_dump (__LINE__));
		  ফেরত xyzModem_eof;
		}
	    পূর্বনির্ধারিত:
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
	  ফেরত xyzModem_timeout;
	}
    }

  /* Header found, now read the data */
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (অক্ষর *) &xyz.blk);
  ZM_DEBUG (zm_save (xyz.blk));
  যদি (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ফেরত xyzModem_timeout;
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (অক্ষর *) &xyz.cblk);
  ZM_DEBUG (zm_save (xyz.cblk));
  যদি (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ফেরত xyzModem_timeout;
    }
  xyz.len = (c == SOH) ? 128 : 1024;
  xyz.bufp = xyz.pkt;
  জন্য (i = 0; i < xyz.len; i++)
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
	  ফেরত xyzModem_timeout;
	}
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (অক্ষর *) &xyz.crc1);
  ZM_DEBUG (zm_save (xyz.crc1));
  যদি (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ফেরত xyzModem_timeout;
    }
  যদি (xyz.crc_mode)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (অক্ষর *) &xyz.crc2);
      ZM_DEBUG (zm_save (xyz.crc2));
      যদি (!res)
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  ফেরত xyzModem_timeout;
	}
    }
  ZM_DEBUG (zm_dump (__LINE__));
  /* Validate the message */
  যদি ((xyz.blk ^ xyz.cblk) != (unsigned অক্ষর) 0xFF)
    {
      ZM_DEBUG (zm_dprintf
		("Framing error - blk: %x/%x/%x\n", xyz.blk, xyz.cblk,
		 (xyz.blk ^ xyz.cblk)));
      ZM_DEBUG (zm_dump_buf (xyz.pkt, xyz.len));
      xyzModem_flush ();
      ফেরত xyzModem_frame;
    }
  /* Verify checksum/CRC */
  যদি (xyz.crc_mode)
    {
      cksum = crc16_ccitt(0, xyz.pkt, xyz.len);
      যদি (cksum != ((xyz.crc1 << 8) | xyz.crc2))
	{
	  ZM_DEBUG (zm_dprintf ("CRC error - recvd: %02x%02x, computed: %x\n",
				xyz.crc1, xyz.crc2, cksum & 0xFFFF));
	  ফেরত xyzModem_cksum;
	}
    }
  অন্যথা
    {
      cksum = 0;
      জন্য (i = 0; i < xyz.len; i++)
	{
	  cksum += xyz.pkt[i];
	}
      যদি (xyz.crc1 != (cksum & 0xFF))
	{
	  ZM_DEBUG (zm_dprintf
		    ("Checksum error - recvd: %x, computed: %x\n", xyz.crc1,
		     cksum & 0xFF));
	  ফেরত xyzModem_cksum;
	}
    }
  /* If we get here, the message passes [structural] muster */
  ফেরত 0;
}

স্থির
ulong
xyzModem_get_initial_timeout (শূন্য)
{
  /* timeout is in seconds, non-positive timeout value is infinity */
#যদি CONFIG_IS_ENABLED(ENV_SUPPORT)
  ধ্রুবক অক্ষর *timeout_str = env_get("loadxy_timeout");
  যদি (timeout_str)
    ফেরত 1000 * simple_strtol(timeout_str, ফাঁকা, 10);
#endif
  ফেরত 1000 * CONFIG_CMD_LOADXY_TIMEOUT;
}

পূর্ণসংখ্যা
xyzModem_stream_open (connection_info_t * info, পূর্ণসংখ্যা *err)
{
  পূর্ণসংখ্যা stat = 0;
  পূর্ণসংখ্যা retries = xyzModem_MAX_RETRIES;
  পূর্ণসংখ্যা crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;

/*    ZM_DEBUG(zm_out = zm_out_start); */
#ifdef xyzModem_zmodem
  যদি (info->mode == xyzModem_zmodem)
    {
      *err = xyzModem_noZmodem;
      ফেরত -1;
    }
#endif

/* TODO: CHECK ! */
  পূর্ণসংখ্যা dummy = 0;
  xyz.__chan = &dummy;
  xyz.len = 0;
  xyz.pad_pending = 0;
  xyz.pad_deliver = 0;
  xyz.crc_mode = true;
  xyz.at_eof = false;
  xyz.tx_ack = false;
  xyz.mode = info->mode;
  xyz.যোগ_retries = 0;
  xyz.যোগ_SOH = 0;
  xyz.যোগ_STX = 0;
  xyz.যোগ_CAN = 0;
  xyz.read_দৈর্ঘ্য = 0;
  xyz.file_দৈর্ঘ্য = 0;
  xyz.first_xmodem_packet = false;
  xyz.initial_time = get_timer(0);
  xyz.timeout = xyzModem_get_initial_timeout();

  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));

  যদি (xyz.mode == xyzModem_xmodem)
    {
      /* X-modem doesn't have an information header - exit here */
      xyz.first_xmodem_packet = true;
      xyz.পরবর্তী_blk = 1;
      ফেরত 0;
    }

  যখন (!(xyz.timeout && get_timer(xyz.initial_time) > xyz.timeout))
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
	      যখন (*xyz.bufp++);
	      /* get the length */
	      parse_অঙ্ক ((অক্ষর *) xyz.bufp, &xyz.file_দৈর্ঘ্য, ফাঁকা, " ");
	      /* The rest of the file name data block quietly discarded */
	      xyz.tx_ack = true;
	    }
	  xyz.পরবর্তী_blk = 1;
	  xyz.len = 0;
	  ফেরত 0;
	}
      অন্যথা যদি (stat == xyzModem_timeout)
	{
	  যদি (--crc_retries <= 0)
	    xyz.crc_mode = false;
	  CYGACC_CALL_IF_DELAY_US (5 * 100000);	/* Extra delay for startup */
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	  xyz.যোগ_retries++;
	  ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	}
      যদি (stat == xyzModem_cancel)
	{
	  বিরতি;
	}
    }
  *err = stat;
  ZM_DEBUG (zm_flush ());
  ফেরত -1;
}

পূর্ণসংখ্যা
xyzModem_stream_read (অক্ষর *buf, পূর্ণসংখ্যা আকার_2, পূর্ণসংখ্যা *err)
{
  পূর্ণসংখ্যা stat, যোগ_2, len;
  পূর্ণসংখ্যা retries;

  যোগ_2 = 0;
  stat = xyzModem_cancel;
  /* Try and get 'size' bytes into the buffer */
  যখন (!xyz.at_eof && xyz.len >= 0 && (আকার_2 > 0))
    {
      যদি (xyz.len == 0 && xyz.pad_deliver == 0)
	{
	  retries = xyzModem_MAX_RETRIES;
	  যখন (retries-- > 0)
	    {
	      যদি (xyz.first_xmodem_packet && xyz.timeout &&
		  get_timer(xyz.initial_time) > xyz.timeout)
		{
		  *err = xyzModem_timeout;
		  xyz.len = -1;
		  ফেরত যোগ_2;
		}

	      stat = xyzModem_get_hdr ();
	      যদি (stat == 0)
		{
		  যদি (xyz.mode == xyzModem_xmodem && xyz.first_xmodem_packet)
		    xyz.first_xmodem_packet = false;
		  যদি (xyz.blk == xyz.পরবর্তী_blk)
		    {
		      xyz.tx_ack = true;
		      ZM_DEBUG (zm_dprintf
				("ACK block %d (%d)\n", xyz.blk, __LINE__));
		      xyz.পরবর্তী_blk = (xyz.পরবর্তী_blk + 1) & 0xFF;

		      যদি (xyz.mode == xyzModem_xmodem || xyz.file_দৈর্ঘ্য == 0)
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
			      যখন (xyz.len
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
		      যদি (xyz.mode != xyzModem_xmodem && 0 != xyz.file_দৈর্ঘ্য)
			{
			  xyz.read_দৈর্ঘ্য += xyz.len;
			  যদি (xyz.read_দৈর্ঘ্য > xyz.file_দৈর্ঘ্য)
			    {
			      xyz.len -= (xyz.read_দৈর্ঘ্য - xyz.file_দৈর্ঘ্য);
			    }
			}
		      বিরতি;
		    }
		  অন্যথা যদি (xyz.blk == ((xyz.পরবর্তী_blk - 1) & 0xFF))
		    {
		      /* Just re-ACK this so sender will get on with it */
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      চালু;	/* Need new header */
		    }
		  অন্যথা
		    {
		      stat = xyzModem_sequence;
		    }
		}
	      যদি (stat == xyzModem_cancel)
		{
		  বিরতি;
		}
	      যদি (stat == xyzModem_eof)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK (%d)\n", __LINE__));
		  যদি (xyz.mode == xyzModem_ymodem)
		    {
		      CYGACC_COMM_IF_PUTC (*xyz.__chan,
					   (xyz.crc_mode ? 'C' : NAK));
		      xyz.যোগ_retries++;
		      ZM_DEBUG (zm_dprintf ("Reading Final Header\n"));
		      stat = xyzModem_get_hdr ();
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ZM_DEBUG (zm_dprintf ("FINAL ACK (%d)\n", __LINE__));
		    }
		  অন্যথা
		    stat = 0;

		  xyz.pad_pending = 0;	/* EOT: withheld ^Z run was padding */
		  xyz.at_eof = true;
		  বিরতি;
		}
	      CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	      xyz.যোগ_retries++;
	      ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	    }
	  যদি (stat < 0 && (!xyz.first_xmodem_packet || stat != xyzModem_timeout))
	    {
	      *err = stat;
	      xyz.len = -1;
	      ফেরত যোগ_2;
	    }
	}
      /* A ^Z run withheld from the previous block turned out to be data */
      যদি (xyz.pad_deliver > 0)
	{
	  len = xyz.pad_deliver;
	  যদি (আকার_2 < len)
	    len = আকার_2;
	  memset (buf, EOF, len);
	  আকার_2 -= len;
	  buf += len;
	  যোগ_2 += len;
	  xyz.pad_deliver -= len;
	}
      /* Don't "read" data from the EOF protocol package */
      যদি (!xyz.at_eof && xyz.len > 0)
	{
	  len = xyz.len;
	  যদি (আকার_2 < len)
	    len = আকার_2;
	  memcpy (buf, xyz.bufp, len);
	  আকার_2 -= len;
	  buf += len;
	  যোগ_2 += len;
	  xyz.len -= len;
	  xyz.bufp += len;
	}
    }
  ফেরত যোগ_2;
}

শূন্য
xyzModem_stream_close (পূর্ণসংখ্যা *err)
{
  ZM_DEBUG (zm_dprintf
    ("xyzModem - %s mode, %d(SOH)/%d(STX)/%d(CAN) packets, %d retries\n",
     xyz.crc_mode ? "CRC" : "Cksum", xyz.যোগ_SOH, xyz.যোগ_STX,
     xyz.যোগ_CAN, xyz.যোগ_retries));
  ZM_DEBUG (zm_flush ());
}

/* Need to be able to clean out the input buffer, so have to take the */
/* getc */
শূন্য
xyzModem_stream_terminate (bool abort, পূর্ণসংখ্যা (*getc) (শূন্য))
{
  পূর্ণসংখ্যা c;

  যদি (abort)
    {
      ZM_DEBUG (zm_dprintf ("!!!! TRANSFER ABORT !!!!\n"));
      নির্বাচন (xyz.mode)
	{
	ক্ষেত্র xyzModem_xmodem:
	ক্ষেত্র xyzModem_ymodem:
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
	  বিরতি;
#ifdef xyzModem_zmodem
	ক্ষেত্র xyzModem_zmodem:
	  /* Might support it some day I suppose. */
#endif
	  বিরতি;
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
      যখন ((c = (*getc) ()) > -1)
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

অক্ষর *
xyzModem_error (পূর্ণসংখ্যা err)
{
  নির্বাচন (err)
    {
    ক্ষেত্র xyzModem_access:
      ফেরত "Can't access file";
      বিরতি;
    ক্ষেত্র xyzModem_noZmodem:
      ফেরত "Sorry, zModem not available yet";
      বিরতি;
    ক্ষেত্র xyzModem_timeout:
      ফেরত "Timed out";
      বিরতি;
    ক্ষেত্র xyzModem_eof:
      ফেরত "End of file";
      বিরতি;
    ক্ষেত্র xyzModem_cancel:
      ফেরত "Cancelled";
      বিরতি;
    ক্ষেত্র xyzModem_frame:
      ফেরত "Invalid framing";
      বিরতি;
    ক্ষেত্র xyzModem_cksum:
      ফেরত "CRC/checksum error";
      বিরতি;
    ক্ষেত্র xyzModem_sequence:
      ফেরত "Block sequence error";
      বিরতি;
    পূর্বনির্ধারিত:
      ফেরত "Unknown error";
      বিরতি;
    }
}

/*
 * RedBoot interface
 */
