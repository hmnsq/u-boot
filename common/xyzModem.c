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
ساكن بنية
{
  صحيح *__chan;
  unsigned حرف pkt[1024], *bufp;
  صحيح pad_pending;	/* trailing ^Z run withheld from the previous block */
  صحيح pad_deliver;	/* ^Z bytes to emit before the next data */
  unsigned حرف blk, cblk, crc1, crc2;
  unsigned حرف next_blk;	/* Expected block */
  صحيح len, mode, total_retries;
  صحيح total_SOH, total_STX, total_CAN;
  bool crc_mode, at_eof, tx_ack;
  bool first_xmodem_packet;
  ulong initial_time, timeout;
  unsigned long file_length, read_length;
} xyz;

#define xyzModem_CHAR_TIMEOUT            2000	/* 2 seconds */
#define xyzModem_MAX_RETRIES             20
#define xyzModem_MAX_RETRIES_WITH_CRC    10
#define xyzModem_CAN_COUNT                3	/* Wait for 3 CAN before quitting */

تعريفنوع صحيح cyg_int32;
ساكن صحيح
CYGACC_COMM_IF_GETC_TIMEOUT (حرف chan, حرف *c)
{

  ulong now = get_timer(0);
  schedule();
  ܟܡܐ (!tstc ())
    {
      ܐܢ (get_timer(now) > xyzModem_CHAR_TIMEOUT)
        اكسر;
    }
  ܐܢ (tstc ())
    {
      *c = getchar();
      ܦܢܐ 1;
    }
  ܦܢܐ 0;
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
  ܦܢܐ (((c >= '0') && (c <= '9')) ||
	  ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

/* Convert a single hex nibble */
__inline__ ساكن صحيح
_from_hex (حرف c)
{
  صحيح ret = 0;

  ܐܢ ((c >= '0') && (c <= '9'))
    {
      ret = (c - '0');
    }
  ܐܚܪܢܐ ܐܢ ((c >= 'a') && (c <= 'f'))
    {
      ret = (c - 'a' + 0x0a);
    }
  ܐܚܪܢܐ ܐܢ ((c >= 'A') && (c <= 'F'))
    {
      ret = (c - 'A' + 0x0A);
    }
  ܦܢܐ ret;
}

/* Convert a character to lower case */
__inline__ ساكن حرف
_tolower (حرف c)
{
  ܐܢ ((c >= 'A') && (c <= 'Z'))
    {
      c = (c - 'A') + 'a';
    }
  ܦܢܐ c;
}

/* Parse (scan) a number */
ساكن bool
parse_num (حرف *s, unsigned long *val, حرف **es, حرف *delim)
{
  bool first = true;
  صحيح radix = 10;
  حرف c;
  unsigned long result = 0;
  صحيح digit;

  ܟܡܐ (*s == ' ')
    s++;
  ܟܡܐ (*s)
    {
      ܐܢ (first && (s[0] == '0') && (_tolower (s[1]) == 'x'))
	{
	  radix = 16;
	  s += 2;
	}
      first = false;
      c = *s++;
      ܐܢ (_is_hex (c) && ((digit = _from_hex (c)) < radix))
	{
	  /* Valid digit */
	  result = (result * radix) + digit;
	}
      ܐܚܪܢܐ
	{
	  ܐܢ (delim != (حرف *) 0)
	    {
	      /* See if this character is one of the delimiters */
	      حرف *dp = delim;
	      ܟܡܐ (*dp && (c != *dp))
		dp++;
	      ܐܢ (*dp)
		اكسر;		/* Found a good delimiter */
	    }
	  ܦܢܐ false;		/* Malformatted number */
	}
    }
  *val = result;
  ܐܢ (es != (حرف **) 0)
    {
      *es = s;
    }
  ܦܢܐ true;
}

#ܐܢ defined(DEBUG) && !CONFIG_IS_ENABLED(USE_TINY_PRINTF)
/*
 * Note: this debug setup works by storing the strings in a fixed buffer
 */
ساكن حرف zm_debug_buf[8192];
ساكن حرف *zm_out = zm_debug_buf;
ساكن حرف *zm_out_start = zm_debug_buf;

ساكن صحيح
zm_dprintf(حرف *fmt, ...)
{
	صحيح len;
	va_list args;

	va_start(args, fmt);
	len = diag_vsprintf(zm_out, fmt, args);
	va_end(args);
	zm_out += len;
	ܦܢܐ len;
}

ساكن فراغ
zm_flush (فراغ)
{
  zm_out = zm_out_start;
}

ساكن فراغ
zm_dump_buf (فراغ *buf, صحيح len)
{

}

ساكن unsigned حرف zm_buf[2048];
ساكن unsigned حرف *zm_bp;

ساكن فراغ
zm_new (فراغ)
{
  zm_bp = zm_buf;
}

ساكن فراغ
zm_save (unsigned حرف c)
{
  *zm_bp++ = c;
}

ساكن فراغ
zm_dump (صحيح line)
{
  zm_dprintf ("Packet at line: %d\n", line);
  zm_dump_buf (zm_buf, zm_bp - zm_buf);
}

#define ZM_DEBUG(x) x
#ܐܚܪܢܐ
#define ZM_DEBUG(x)
#endif

/* Wait for the line to go idle */
ساكن فراغ
xyzModem_flush (فراغ)
{
  صحيح res;
  حرف c;
  ܟܡܐ (true)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ܐܢ (!res)
	ܦܢܐ;
    }
}

ساكن صحيح
xyzModem_get_hdr (فراغ)
{
  حرف c;
  صحيح res;
  bool hdr_found = false;
  صحيح i, can_total, hdr_chars;
  unsigned short cksum;

  ZM_DEBUG (zm_new ());
  /* Find the start of a header */
  can_total = 0;
  hdr_chars = 0;

  ܐܢ (xyz.tx_ack)
    {
      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
      xyz.tx_ack = false;
    }
  ܟܡܐ (!hdr_found)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      ܐܢ (res)
	{
	  hdr_chars++;
	  بدل (c)
	    {
	    حالة SOH:
	      xyz.total_SOH++;
	      fallthrough;
	    حالة STX:
	      ܐܢ (c == STX)
		xyz.total_STX++;
	      hdr_found = true;
	      اكسر;
	    حالة CAN:
	    حالة ETX:
	      xyz.total_CAN++;
	      ZM_DEBUG (zm_dump (__LINE__));
	      ܐܢ (++can_total == xyzModem_CAN_COUNT)
		{
		  ܦܢܐ xyzModem_cancel;
		}
	      ܐܚܪܢܐ
		{
		  /* Wait for multiple CAN to avoid early quits */
		  اكسر;
		}
	    حالة EOT:
	      /* EOT only supported if no noise */
	      ܐܢ (hdr_chars == 1)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK on EOT #%d\n", __LINE__));
		  ZM_DEBUG (zm_dump (__LINE__));
		  ܦܢܐ xyzModem_eof;
		}
	    افتراضي:
	      /* Ignore, waiting for start of header */
	      ;
	    }
	}
      ܐܚܪܢܐ
	{
	  /* Data stream timed out */
	  xyzModem_flush ();	/* Toss any current input */
	  ZM_DEBUG (zm_dump (__LINE__));
	  CYGACC_CALL_IF_DELAY_US ((cyg_int32) 250000);
	  ܦܢܐ xyzModem_timeout;
	}
    }

  /* Header found, now read the data */
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (حرف *) &xyz.blk);
  ZM_DEBUG (zm_save (xyz.blk));
  ܐܢ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ܦܢܐ xyzModem_timeout;
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (حرف *) &xyz.cblk);
  ZM_DEBUG (zm_save (xyz.cblk));
  ܐܢ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ܦܢܐ xyzModem_timeout;
    }
  xyz.len = (c == SOH) ? 128 : 1024;
  xyz.bufp = xyz.pkt;
  لكل (i = 0; i < xyz.len; i++)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      ܐܢ (res)
	{
	  xyz.pkt[i] = c;
	}
      ܐܚܪܢܐ
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  ܦܢܐ xyzModem_timeout;
	}
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (حرف *) &xyz.crc1);
  ZM_DEBUG (zm_save (xyz.crc1));
  ܐܢ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ܦܢܐ xyzModem_timeout;
    }
  ܐܢ (xyz.crc_mode)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (حرف *) &xyz.crc2);
      ZM_DEBUG (zm_save (xyz.crc2));
      ܐܢ (!res)
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  ܦܢܐ xyzModem_timeout;
	}
    }
  ZM_DEBUG (zm_dump (__LINE__));
  /* Validate the message */
  ܐܢ ((xyz.blk ^ xyz.cblk) != (unsigned حرف) 0xFF)
    {
      ZM_DEBUG (zm_dprintf
		("Framing error - blk: %x/%x/%x\n", xyz.blk, xyz.cblk,
		 (xyz.blk ^ xyz.cblk)));
      ZM_DEBUG (zm_dump_buf (xyz.pkt, xyz.len));
      xyzModem_flush ();
      ܦܢܐ xyzModem_frame;
    }
  /* Verify checksum/CRC */
  ܐܢ (xyz.crc_mode)
    {
      cksum = crc16_ccitt(0, xyz.pkt, xyz.len);
      ܐܢ (cksum != ((xyz.crc1 << 8) | xyz.crc2))
	{
	  ZM_DEBUG (zm_dprintf ("CRC error - recvd: %02x%02x, computed: %x\n",
				xyz.crc1, xyz.crc2, cksum & 0xFFFF));
	  ܦܢܐ xyzModem_cksum;
	}
    }
  ܐܚܪܢܐ
    {
      cksum = 0;
      لكل (i = 0; i < xyz.len; i++)
	{
	  cksum += xyz.pkt[i];
	}
      ܐܢ (xyz.crc1 != (cksum & 0xFF))
	{
	  ZM_DEBUG (zm_dprintf
		    ("Checksum error - recvd: %x, computed: %x\n", xyz.crc1,
		     cksum & 0xFF));
	  ܦܢܐ xyzModem_cksum;
	}
    }
  /* If we get here, the message passes [structural] muster */
  ܦܢܐ 0;
}

ساكن
ulong
xyzModem_get_initial_timeout (فراغ)
{
  /* timeout is in seconds, non-positive timeout value is infinity */
#ܐܢ CONFIG_IS_ENABLED(ENV_SUPPORT)
  ثابت حرف *timeout_str = env_get("loadxy_timeout");
  ܐܢ (timeout_str)
    ܦܢܐ 1000 * simple_strtol(timeout_str, عدم, 10);
#endif
  ܦܢܐ 1000 * CONFIG_CMD_LOADXY_TIMEOUT;
}

صحيح
xyzModem_stream_open (connection_info_t * info, صحيح *err)
{
  صحيح stat = 0;
  صحيح retries = xyzModem_MAX_RETRIES;
  صحيح crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;

/*    ZM_DEBUG(zm_out = zm_out_start); */
#ifdef xyzModem_zmodem
  ܐܢ (info->mode == xyzModem_zmodem)
    {
      *err = xyzModem_noZmodem;
      ܦܢܐ -1;
    }
#endif

/* TODO: CHECK ! */
  صحيح dummy = 0;
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

  ܐܢ (xyz.mode == xyzModem_xmodem)
    {
      /* X-modem doesn't have an information header - exit here */
      xyz.first_xmodem_packet = true;
      xyz.next_blk = 1;
      ܦܢܐ 0;
    }

  ܟܡܐ (!(xyz.timeout && get_timer(xyz.initial_time) > xyz.timeout))
    {
      ܐܢ (--retries <= 0)
        {
          retries = xyzModem_MAX_RETRIES;
          crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;
          xyz.crc_mode = true;
        }
      stat = xyzModem_get_hdr ();
      ܐܢ (stat == 0)
	{
	  /* Y-modem file information header */
	  ܐܢ (xyz.blk == 0)
	    {
	      /* skip filename */
	      ܟܡܐ (*xyz.bufp++);
	      /* get the length */
	      parse_num ((حرف *) xyz.bufp, &xyz.file_length, عدم, " ");
	      /* The rest of the file name data block quietly discarded */
	      xyz.tx_ack = true;
	    }
	  xyz.next_blk = 1;
	  xyz.len = 0;
	  ܦܢܐ 0;
	}
      ܐܚܪܢܐ ܐܢ (stat == xyzModem_timeout)
	{
	  ܐܢ (--crc_retries <= 0)
	    xyz.crc_mode = false;
	  CYGACC_CALL_IF_DELAY_US (5 * 100000);	/* Extra delay for startup */
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	  xyz.total_retries++;
	  ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	}
      ܐܢ (stat == xyzModem_cancel)
	{
	  اكسر;
	}
    }
  *err = stat;
  ZM_DEBUG (zm_flush ());
  ܦܢܐ -1;
}

صحيح
xyzModem_stream_read (حرف *buf, صحيح size, صحيح *err)
{
  صحيح stat, total, len;
  صحيح retries;

  total = 0;
  stat = xyzModem_cancel;
  /* Try and get 'size' bytes into the buffer */
  ܟܡܐ (!xyz.at_eof && xyz.len >= 0 && (size > 0))
    {
      ܐܢ (xyz.len == 0 && xyz.pad_deliver == 0)
	{
	  retries = xyzModem_MAX_RETRIES;
	  ܟܡܐ (retries-- > 0)
	    {
	      ܐܢ (xyz.first_xmodem_packet && xyz.timeout &&
		  get_timer(xyz.initial_time) > xyz.timeout)
		{
		  *err = xyzModem_timeout;
		  xyz.len = -1;
		  ܦܢܐ total;
		}

	      stat = xyzModem_get_hdr ();
	      ܐܢ (stat == 0)
		{
		  ܐܢ (xyz.mode == xyzModem_xmodem && xyz.first_xmodem_packet)
		    xyz.first_xmodem_packet = false;
		  ܐܢ (xyz.blk == xyz.next_blk)
		    {
		      xyz.tx_ack = true;
		      ZM_DEBUG (zm_dprintf
				("ACK block %d (%d)\n", xyz.blk, __LINE__));
		      xyz.next_blk = (xyz.next_blk + 1) & 0xFF;

		      ܐܢ (xyz.mode == xyzModem_xmodem || xyz.file_length == 0)
			{
			  /* Without a file length, EOF padding can't be
			   * distinguished from block data until the next
			   * frame arrives and we can check for EOT/EOF.
			   */
			  xyz.pad_deliver = xyz.pad_pending;
			  xyz.pad_pending = 0;
			  ܐܢ ((xyz.bufp[xyz.len - 1] == EOF) &&
			      (xyz.bufp[xyz.len - 2] == EOF) &&
			      (xyz.bufp[xyz.len - 3] == EOF))
			    {
			      ܟܡܐ (xyz.len
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
		      ܐܢ (xyz.mode != xyzModem_xmodem && 0 != xyz.file_length)
			{
			  xyz.read_length += xyz.len;
			  ܐܢ (xyz.read_length > xyz.file_length)
			    {
			      xyz.len -= (xyz.read_length - xyz.file_length);
			    }
			}
		      اكسر;
		    }
		  ܐܚܪܢܐ ܐܢ (xyz.blk == ((xyz.next_blk - 1) & 0xFF))
		    {
		      /* Just re-ACK this so sender will get on with it */
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      استمر;	/* Need new header */
		    }
		  ܐܚܪܢܐ
		    {
		      stat = xyzModem_sequence;
		    }
		}
	      ܐܢ (stat == xyzModem_cancel)
		{
		  اكسر;
		}
	      ܐܢ (stat == xyzModem_eof)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK (%d)\n", __LINE__));
		  ܐܢ (xyz.mode == xyzModem_ymodem)
		    {
		      CYGACC_COMM_IF_PUTC (*xyz.__chan,
					   (xyz.crc_mode ? 'C' : NAK));
		      xyz.total_retries++;
		      ZM_DEBUG (zm_dprintf ("Reading Final Header\n"));
		      stat = xyzModem_get_hdr ();
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ZM_DEBUG (zm_dprintf ("FINAL ACK (%d)\n", __LINE__));
		    }
		  ܐܚܪܢܐ
		    stat = 0;

		  xyz.pad_pending = 0;	/* EOT: withheld ^Z run was padding */
		  xyz.at_eof = true;
		  اكسر;
		}
	      CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	      xyz.total_retries++;
	      ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	    }
	  ܐܢ (stat < 0 && (!xyz.first_xmodem_packet || stat != xyzModem_timeout))
	    {
	      *err = stat;
	      xyz.len = -1;
	      ܦܢܐ total;
	    }
	}
      /* A ^Z run withheld from the previous block turned out to be data */
      ܐܢ (xyz.pad_deliver > 0)
	{
	  len = xyz.pad_deliver;
	  ܐܢ (size < len)
	    len = size;
	  memset (buf, EOF, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.pad_deliver -= len;
	}
      /* Don't "read" data from the EOF protocol package */
      ܐܢ (!xyz.at_eof && xyz.len > 0)
	{
	  len = xyz.len;
	  ܐܢ (size < len)
	    len = size;
	  memcpy (buf, xyz.bufp, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.len -= len;
	  xyz.bufp += len;
	}
    }
  ܦܢܐ total;
}

فراغ
xyzModem_stream_close (صحيح *err)
{
  ZM_DEBUG (zm_dprintf
    ("xyzModem - %s mode, %d(SOH)/%d(STX)/%d(CAN) packets, %d retries\n",
     xyz.crc_mode ? "CRC" : "Cksum", xyz.total_SOH, xyz.total_STX,
     xyz.total_CAN, xyz.total_retries));
  ZM_DEBUG (zm_flush ());
}

/* Need to be able to clean out the input buffer, so have to take the */
/* getc */
فراغ
xyzModem_stream_terminate (bool abort, صحيح (*getc) (فراغ))
{
  صحيح c;

  ܐܢ (abort)
    {
      ZM_DEBUG (zm_dprintf ("!!!! TRANSFER ABORT !!!!\n"));
      بدل (xyz.mode)
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
#ifdef xyzModem_zmodem
	حالة xyzModem_zmodem:
	  /* Might support it some day I suppose. */
#endif
	  اكسر;
	}
    }
  ܐܚܪܢܐ
    {
      ZM_DEBUG (zm_dprintf ("Engaging cleanup mode...\n"));
      /*
       * Consume any trailing crap left in the inbuffer from
       * previous received blocks. Since very few files are an exact multiple
       * of the transfer block size, there will almost always be some gunk here.
       * If we don't eat it now, RedBoot will think the user typed it.
       */
      ZM_DEBUG (zm_dprintf ("Trailing gunk:\n"));
      ܟܡܐ ((c = (*getc) ()) > -1)
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
      ܦܢܐ "Can't access file";
      اكسر;
    حالة xyzModem_noZmodem:
      ܦܢܐ "Sorry, zModem not available yet";
      اكسر;
    حالة xyzModem_timeout:
      ܦܢܐ "Timed out";
      اكسر;
    حالة xyzModem_eof:
      ܦܢܐ "End of file";
      اكسر;
    حالة xyzModem_cancel:
      ܦܢܐ "Cancelled";
      اكسر;
    حالة xyzModem_frame:
      ܦܢܐ "Invalid framing";
      اكسر;
    حالة xyzModem_cksum:
      ܦܢܐ "CRC/checksum error";
      اكسر;
    حالة xyzModem_sequence:
      ܦܢܐ "Block sequence error";
      اكسر;
    افتراضي:
      ܦܢܐ "Unknown error";
      اكسر;
    }
}

/*
 * RedBoot interface
 */
