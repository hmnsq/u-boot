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
#അന്തര്ഭാവ <xyzModem.h>
#അന്തര്ഭാവ <stdarg.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <u-boot/crc.h>
#അന്തര്ഭാവ <watchdog.h>
#അന്തര്ഭാവ <env.h>
#അന്തര്ഭാവ <vsprintf.h>

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
സ്ഥിതം ഘടന
{
  പൂർണ്ണസംഖ്യ *__chan;
  unsigned അക്ഷരം pkt[1024], *bufp;
  പൂർണ്ണസംഖ്യ pad_pending;	/* trailing ^Z run withheld from the previous block */
  പൂർണ്ണസംഖ്യ pad_deliver;	/* ^Z bytes to emit before the next data */
  unsigned അക്ഷരം blk, cblk, crc1, crc2;
  unsigned അക്ഷരം next_blk;	/* Expected block */
  പൂർണ്ണസംഖ്യ len, mode, total_retries;
  പൂർണ്ണസംഖ്യ total_SOH, total_STX, total_CAN;
  bool crc_mode, at_eof, tx_ack;
  bool first_xmodem_packet;
  ulong initial_time, timeout;
  unsigned long file_length, read_length;
} xyz;

#define xyzModem_CHAR_TIMEOUT            2000	/* 2 seconds */
#define xyzModem_MAX_RETRIES             20
#define xyzModem_MAX_RETRIES_WITH_CRC    10
#define xyz_Modem_CAN_എണ്ണം                3	/* Wait for 3 CAN before quitting */

തരനിർവചനം പൂർണ്ണസംഖ്യ cyg_int32;
സ്ഥിതം പൂർണ്ണസംഖ്യ
CYGACC_COMM_IF_GETC_TIMEOUT (അക്ഷരം chan, അക്ഷരം *c)
{

  ulong now = get_timer(0);
  schedule();
  ആകുമ്പോൾ (!tstc ())
    {
      എങ്കിൽ (get_timer(now) > xyzModem_CHAR_TIMEOUT)
        നിർത്തുക;
    }
  എങ്കിൽ (tstc ())
    {
      *c = getchar();
      തിരികെ 1;
    }
  തിരികെ 0;
}

സ്ഥിതം ശൂന്യം
CYGACC_COMM_IF_PUTC (അക്ഷരം x, അക്ഷരം y)
{
  putc (y);
}

/* Validate a hex character */
__inline__ സ്ഥിതം bool
_is_hex (അക്ഷരം c)
{
  തിരികെ (((c >= '0') && (c <= '9')) ||
	  ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

/* Convert a single hex nibble */
__inline__ സ്ഥിതം പൂർണ്ണസംഖ്യ
_from_hex (അക്ഷരം c)
{
  പൂർണ്ണസംഖ്യ ret = 0;

  എങ്കിൽ ((c >= '0') && (c <= '9'))
    {
      ret = (c - '0');
    }
  അല്ലെങ്കിൽ എങ്കിൽ ((c >= 'a') && (c <= 'f'))
    {
      ret = (c - 'a' + 0x0a);
    }
  അല്ലെങ്കിൽ എങ്കിൽ ((c >= 'A') && (c <= 'F'))
    {
      ret = (c - 'A' + 0x0A);
    }
  തിരികെ ret;
}

/* Convert a character to lower case */
__inline__ സ്ഥിതം അക്ഷരം
_tolower (അക്ഷരം c)
{
  എങ്കിൽ ((c >= 'A') && (c <= 'Z'))
    {
      c = (c - 'A') + 'a';
    }
  തിരികെ c;
}

/* Parse (scan) a number */
സ്ഥിതം bool
parse_അക്കം (അക്ഷരം *s, unsigned long *val, അക്ഷരം **es, അക്ഷരം *delim)
{
  bool first = true;
  പൂർണ്ണസംഖ്യ radix = 10;
  അക്ഷരം c;
  unsigned long result = 0;
  പൂർണ്ണസംഖ്യ digit;

  ആകുമ്പോൾ (*s == ' ')
    s++;
  ആകുമ്പോൾ (*s)
    {
      എങ്കിൽ (first && (s[0] == '0') && (_tolower (s[1]) == 'x'))
	{
	  radix = 16;
	  s += 2;
	}
      first = false;
      c = *s++;
      എങ്കിൽ (_is_hex (c) && ((digit = _from_hex (c)) < radix))
	{
	  /* Valid digit */
	  result = (result * radix) + digit;
	}
      അല്ലെങ്കിൽ
	{
	  എങ്കിൽ (delim != (അക്ഷരം *) 0)
	    {
	      /* See if this character is one of the delimiters */
	      അക്ഷരം *dp = delim;
	      ആകുമ്പോൾ (*dp && (c != *dp))
		dp++;
	      എങ്കിൽ (*dp)
		നിർത്തുക;		/* Found a good delimiter */
	    }
	  തിരികെ false;		/* Malformatted number */
	}
    }
  *val = result;
  എങ്കിൽ (es != (അക്ഷരം **) 0)
    {
      *es = s;
    }
  തിരികെ true;
}

#എങ്കിൽ defined(DEBUG) && !CONFIG_IS_ENABLED(USE_TINY_PRINTF)
/*
 * Note: this debug setup works by storing the strings in a fixed buffer
 */
സ്ഥിതം അക്ഷരം zm_debug_buf[8192];
സ്ഥിതം അക്ഷരം *zm_out = zm_debug_buf;
സ്ഥിതം അക്ഷരം *zm_out_start = zm_debug_buf;

സ്ഥിതം പൂർണ്ണസംഖ്യ
zm_dprintf(അക്ഷരം *fmt, ...)
{
	പൂർണ്ണസംഖ്യ len;
	va_list args;

	va_start(args, fmt);
	len = diag_vsprintf(zm_out, fmt, args);
	va_end(args);
	zm_out += len;
	തിരികെ len;
}

സ്ഥിതം ശൂന്യം
zm_flush (ശൂന്യം)
{
  zm_out = zm_out_start;
}

സ്ഥിതം ശൂന്യം
zm_dump_buf (ശൂന്യം *buf, പൂർണ്ണസംഖ്യ len)
{

}

സ്ഥിതം unsigned അക്ഷരം zm_buf[2048];
സ്ഥിതം unsigned അക്ഷരം *zm_bp;

സ്ഥിതം ശൂന്യം
zm_new (ശൂന്യം)
{
  zm_bp = zm_buf;
}

സ്ഥിതം ശൂന്യം
zm_save (unsigned അക്ഷരം c)
{
  *zm_bp++ = c;
}

സ്ഥിതം ശൂന്യം
zm_dump (പൂർണ്ണസംഖ്യ വരി)
{
  zm_dprintf ("Packet at line: %d\n", വരി);
  zm_dump_buf (zm_buf, zm_bp - zm_buf);
}

#define ZM_DEBUG(x) x
#അല്ലെങ്കിൽ
#define ZM_DEBUG(x)
#endif

/* Wait for the line to go idle */
സ്ഥിതം ശൂന്യം
xyzModem_flush (ശൂന്യം)
{
  പൂർണ്ണസംഖ്യ res;
  അക്ഷരം c;
  ആകുമ്പോൾ (true)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      എങ്കിൽ (!res)
	തിരികെ;
    }
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
xyzModem_get_hdr (ശൂന്യം)
{
  അക്ഷരം c;
  പൂർണ്ണസംഖ്യ res;
  bool hdr_found = false;
  പൂർണ്ണസംഖ്യ i, can_total, hdr_chars;
  unsigned short cksum;

  ZM_DEBUG (zm_new ());
  /* Find the start of a header */
  can_total = 0;
  hdr_chars = 0;

  എങ്കിൽ (xyz.tx_ack)
    {
      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
      xyz.tx_ack = false;
    }
  ആകുമ്പോൾ (!hdr_found)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      എങ്കിൽ (res)
	{
	  hdr_chars++;
	  തിരഞ്ഞെടുപ്പ് (c)
	    {
	    അവസ്ഥ SOH:
	      xyz.total_SOH++;
	      fallthrough;
	    അവസ്ഥ STX:
	      എങ്കിൽ (c == STX)
		xyz.total_STX++;
	      hdr_found = true;
	      നിർത്തുക;
	    അവസ്ഥ CAN:
	    അവസ്ഥ ETX:
	      xyz.total_CAN++;
	      ZM_DEBUG (zm_dump (__LINE__));
	      എങ്കിൽ (++can_total == xyz_Modem_CAN_എണ്ണം)
		{
		  തിരികെ xyzModem_cancel;
		}
	      അല്ലെങ്കിൽ
		{
		  /* Wait for multiple CAN to avoid early quits */
		  നിർത്തുക;
		}
	    അവസ്ഥ EOT:
	      /* EOT only supported if no noise */
	      എങ്കിൽ (hdr_chars == 1)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK on EOT #%d\n", __LINE__));
		  ZM_DEBUG (zm_dump (__LINE__));
		  തിരികെ xyzModem_eof;
		}
	    സ്ഥിരസ്ഥിതി:
	      /* Ignore, waiting for start of header */
	      ;
	    }
	}
      അല്ലെങ്കിൽ
	{
	  /* Data stream timed out */
	  xyzModem_flush ();	/* Toss any current input */
	  ZM_DEBUG (zm_dump (__LINE__));
	  CYGACC_CALL_IF_DELAY_US ((cyg_int32) 250000);
	  തിരികെ xyzModem_timeout;
	}
    }

  /* Header found, now read the data */
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (അക്ഷരം *) &xyz.blk);
  ZM_DEBUG (zm_save (xyz.blk));
  എങ്കിൽ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      തിരികെ xyzModem_timeout;
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (അക്ഷരം *) &xyz.cblk);
  ZM_DEBUG (zm_save (xyz.cblk));
  എങ്കിൽ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      തിരികെ xyzModem_timeout;
    }
  xyz.len = (c == SOH) ? 128 : 1024;
  xyz.bufp = xyz.pkt;
  വേണ്ടി (i = 0; i < xyz.len; i++)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      എങ്കിൽ (res)
	{
	  xyz.pkt[i] = c;
	}
      അല്ലെങ്കിൽ
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  തിരികെ xyzModem_timeout;
	}
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (അക്ഷരം *) &xyz.crc1);
  ZM_DEBUG (zm_save (xyz.crc1));
  എങ്കിൽ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      തിരികെ xyzModem_timeout;
    }
  എങ്കിൽ (xyz.crc_mode)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (അക്ഷരം *) &xyz.crc2);
      ZM_DEBUG (zm_save (xyz.crc2));
      എങ്കിൽ (!res)
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  തിരികെ xyzModem_timeout;
	}
    }
  ZM_DEBUG (zm_dump (__LINE__));
  /* Validate the message */
  എങ്കിൽ ((xyz.blk ^ xyz.cblk) != (unsigned അക്ഷരം) 0xFF)
    {
      ZM_DEBUG (zm_dprintf
		("Framing error - blk: %x/%x/%x\n", xyz.blk, xyz.cblk,
		 (xyz.blk ^ xyz.cblk)));
      ZM_DEBUG (zm_dump_buf (xyz.pkt, xyz.len));
      xyzModem_flush ();
      തിരികെ xyzModem_frame;
    }
  /* Verify checksum/CRC */
  എങ്കിൽ (xyz.crc_mode)
    {
      cksum = crc16_ccitt(0, xyz.pkt, xyz.len);
      എങ്കിൽ (cksum != ((xyz.crc1 << 8) | xyz.crc2))
	{
	  ZM_DEBUG (zm_dprintf ("CRC error - recvd: %02x%02x, computed: %x\n",
				xyz.crc1, xyz.crc2, cksum & 0xFFFF));
	  തിരികെ xyzModem_cksum;
	}
    }
  അല്ലെങ്കിൽ
    {
      cksum = 0;
      വേണ്ടി (i = 0; i < xyz.len; i++)
	{
	  cksum += xyz.pkt[i];
	}
      എങ്കിൽ (xyz.crc1 != (cksum & 0xFF))
	{
	  ZM_DEBUG (zm_dprintf
		    ("Checksum error - recvd: %x, computed: %x\n", xyz.crc1,
		     cksum & 0xFF));
	  തിരികെ xyzModem_cksum;
	}
    }
  /* If we get here, the message passes [structural] muster */
  തിരികെ 0;
}

സ്ഥിതം
ulong
xyzModem_get_initial_timeout (ശൂന്യം)
{
  /* timeout is in seconds, non-positive timeout value is infinity */
#എങ്കിൽ CONFIG_IS_ENABLED(ENV_SUPPORT)
  സ്ഥിരം അക്ഷരം *timeout_str = env_get("loadxy_timeout");
  എങ്കിൽ (timeout_str)
    തിരികെ 1000 * simple_strtol(timeout_str, ശൂന്യ, 10);
#endif
  തിരികെ 1000 * CONFIG_CMD_LOADXY_TIMEOUT;
}

പൂർണ്ണസംഖ്യ
xyzModem_stream_open (connection_info_t * info, പൂർണ്ണസംഖ്യ *err)
{
  പൂർണ്ണസംഖ്യ stat = 0;
  പൂർണ്ണസംഖ്യ retries = xyzModem_MAX_RETRIES;
  പൂർണ്ണസംഖ്യ crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;

/*    ZM_DEBUG(zm_out = zm_out_start); */
#ifdef xyzModem_zmodem
  എങ്കിൽ (info->mode == xyzModem_zmodem)
    {
      *err = xyzModem_noZmodem;
      തിരികെ -1;
    }
#endif

/* TODO: CHECK ! */
  പൂർണ്ണസംഖ്യ dummy = 0;
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

  എങ്കിൽ (xyz.mode == xyzModem_xmodem)
    {
      /* X-modem doesn't have an information header - exit here */
      xyz.first_xmodem_packet = true;
      xyz.next_blk = 1;
      തിരികെ 0;
    }

  ആകുമ്പോൾ (!(xyz.timeout && get_timer(xyz.initial_time) > xyz.timeout))
    {
      എങ്കിൽ (--retries <= 0)
        {
          retries = xyzModem_MAX_RETRIES;
          crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;
          xyz.crc_mode = true;
        }
      stat = xyzModem_get_hdr ();
      എങ്കിൽ (stat == 0)
	{
	  /* Y-modem file information header */
	  എങ്കിൽ (xyz.blk == 0)
	    {
	      /* skip filename */
	      ആകുമ്പോൾ (*xyz.bufp++);
	      /* get the length */
	      parse_അക്കം ((അക്ഷരം *) xyz.bufp, &xyz.file_length, ശൂന്യ, " ");
	      /* The rest of the file name data block quietly discarded */
	      xyz.tx_ack = true;
	    }
	  xyz.next_blk = 1;
	  xyz.len = 0;
	  തിരികെ 0;
	}
      അല്ലെങ്കിൽ എങ്കിൽ (stat == xyzModem_timeout)
	{
	  എങ്കിൽ (--crc_retries <= 0)
	    xyz.crc_mode = false;
	  CYGACC_CALL_IF_DELAY_US (5 * 100000);	/* Extra delay for startup */
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	  xyz.total_retries++;
	  ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	}
      എങ്കിൽ (stat == xyzModem_cancel)
	{
	  നിർത്തുക;
	}
    }
  *err = stat;
  ZM_DEBUG (zm_flush ());
  തിരികെ -1;
}

പൂർണ്ണസംഖ്യ
xyzModem_stream_read (അക്ഷരം *buf, പൂർണ്ണസംഖ്യ size, പൂർണ്ണസംഖ്യ *err)
{
  പൂർണ്ണസംഖ്യ stat, total, len;
  പൂർണ്ണസംഖ്യ retries;

  total = 0;
  stat = xyzModem_cancel;
  /* Try and get 'size' bytes into the buffer */
  ആകുമ്പോൾ (!xyz.at_eof && xyz.len >= 0 && (size > 0))
    {
      എങ്കിൽ (xyz.len == 0 && xyz.pad_deliver == 0)
	{
	  retries = xyzModem_MAX_RETRIES;
	  ആകുമ്പോൾ (retries-- > 0)
	    {
	      എങ്കിൽ (xyz.first_xmodem_packet && xyz.timeout &&
		  get_timer(xyz.initial_time) > xyz.timeout)
		{
		  *err = xyzModem_timeout;
		  xyz.len = -1;
		  തിരികെ total;
		}

	      stat = xyzModem_get_hdr ();
	      എങ്കിൽ (stat == 0)
		{
		  എങ്കിൽ (xyz.mode == xyzModem_xmodem && xyz.first_xmodem_packet)
		    xyz.first_xmodem_packet = false;
		  എങ്കിൽ (xyz.blk == xyz.next_blk)
		    {
		      xyz.tx_ack = true;
		      ZM_DEBUG (zm_dprintf
				("ACK block %d (%d)\n", xyz.blk, __LINE__));
		      xyz.next_blk = (xyz.next_blk + 1) & 0xFF;

		      എങ്കിൽ (xyz.mode == xyzModem_xmodem || xyz.file_length == 0)
			{
			  /* Without a file length, EOF padding can't be
			   * distinguished from block data until the next
			   * frame arrives and we can check for EOT/EOF.
			   */
			  xyz.pad_deliver = xyz.pad_pending;
			  xyz.pad_pending = 0;
			  എങ്കിൽ ((xyz.bufp[xyz.len - 1] == EOF) &&
			      (xyz.bufp[xyz.len - 2] == EOF) &&
			      (xyz.bufp[xyz.len - 3] == EOF))
			    {
			      ആകുമ്പോൾ (xyz.len
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
		      എങ്കിൽ (xyz.mode != xyzModem_xmodem && 0 != xyz.file_length)
			{
			  xyz.read_length += xyz.len;
			  എങ്കിൽ (xyz.read_length > xyz.file_length)
			    {
			      xyz.len -= (xyz.read_length - xyz.file_length);
			    }
			}
		      നിർത്തുക;
		    }
		  അല്ലെങ്കിൽ എങ്കിൽ (xyz.blk == ((xyz.next_blk - 1) & 0xFF))
		    {
		      /* Just re-ACK this so sender will get on with it */
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      തുടരുക;	/* Need new header */
		    }
		  അല്ലെങ്കിൽ
		    {
		      stat = xyzModem_sequence;
		    }
		}
	      എങ്കിൽ (stat == xyzModem_cancel)
		{
		  നിർത്തുക;
		}
	      എങ്കിൽ (stat == xyzModem_eof)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK (%d)\n", __LINE__));
		  എങ്കിൽ (xyz.mode == xyzModem_ymodem)
		    {
		      CYGACC_COMM_IF_PUTC (*xyz.__chan,
					   (xyz.crc_mode ? 'C' : NAK));
		      xyz.total_retries++;
		      ZM_DEBUG (zm_dprintf ("Reading Final Header\n"));
		      stat = xyzModem_get_hdr ();
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ZM_DEBUG (zm_dprintf ("FINAL ACK (%d)\n", __LINE__));
		    }
		  അല്ലെങ്കിൽ
		    stat = 0;

		  xyz.pad_pending = 0;	/* EOT: withheld ^Z run was padding */
		  xyz.at_eof = true;
		  നിർത്തുക;
		}
	      CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	      xyz.total_retries++;
	      ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	    }
	  എങ്കിൽ (stat < 0 && (!xyz.first_xmodem_packet || stat != xyzModem_timeout))
	    {
	      *err = stat;
	      xyz.len = -1;
	      തിരികെ total;
	    }
	}
      /* A ^Z run withheld from the previous block turned out to be data */
      എങ്കിൽ (xyz.pad_deliver > 0)
	{
	  len = xyz.pad_deliver;
	  എങ്കിൽ (size < len)
	    len = size;
	  memset (buf, EOF, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.pad_deliver -= len;
	}
      /* Don't "read" data from the EOF protocol package */
      എങ്കിൽ (!xyz.at_eof && xyz.len > 0)
	{
	  len = xyz.len;
	  എങ്കിൽ (size < len)
	    len = size;
	  memcpy (buf, xyz.bufp, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.len -= len;
	  xyz.bufp += len;
	}
    }
  തിരികെ total;
}

ശൂന്യം
xyzModem_stream_close (പൂർണ്ണസംഖ്യ *err)
{
  ZM_DEBUG (zm_dprintf
    ("xyzModem - %s mode, %d(SOH)/%d(STX)/%d(CAN) packets, %d retries\n",
     xyz.crc_mode ? "CRC" : "Cksum", xyz.total_SOH, xyz.total_STX,
     xyz.total_CAN, xyz.total_retries));
  ZM_DEBUG (zm_flush ());
}

/* Need to be able to clean out the input buffer, so have to take the */
/* getc */
ശൂന്യം
xyzModem_stream_terminate (bool abort, പൂർണ്ണസംഖ്യ (*getc) (ശൂന്യം))
{
  പൂർണ്ണസംഖ്യ c;

  എങ്കിൽ (abort)
    {
      ZM_DEBUG (zm_dprintf ("!!!! TRANSFER ABORT !!!!\n"));
      തിരഞ്ഞെടുപ്പ് (xyz.mode)
	{
	അവസ്ഥ xyzModem_xmodem:
	അവസ്ഥ xyzModem_ymodem:
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
	  നിർത്തുക;
#ifdef xyzModem_zmodem
	അവസ്ഥ xyzModem_zmodem:
	  /* Might support it some day I suppose. */
#endif
	  നിർത്തുക;
	}
    }
  അല്ലെങ്കിൽ
    {
      ZM_DEBUG (zm_dprintf ("Engaging cleanup mode...\n"));
      /*
       * Consume any trailing crap left in the inbuffer from
       * previous received blocks. Since very few files are an exact multiple
       * of the transfer block size, there will almost always be some gunk here.
       * If we don't eat it now, RedBoot will think the user typed it.
       */
      ZM_DEBUG (zm_dprintf ("Trailing gunk:\n"));
      ആകുമ്പോൾ ((c = (*getc) ()) > -1)
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

അക്ഷരം *
xyzModem_error (പൂർണ്ണസംഖ്യ err)
{
  തിരഞ്ഞെടുപ്പ് (err)
    {
    അവസ്ഥ xyzModem_access:
      തിരികെ "Can't access file";
      നിർത്തുക;
    അവസ്ഥ xyzModem_noZmodem:
      തിരികെ "Sorry, zModem not available yet";
      നിർത്തുക;
    അവസ്ഥ xyzModem_timeout:
      തിരികെ "Timed out";
      നിർത്തുക;
    അവസ്ഥ xyzModem_eof:
      തിരികെ "End of file";
      നിർത്തുക;
    അവസ്ഥ xyzModem_cancel:
      തിരികെ "Cancelled";
      നിർത്തുക;
    അവസ്ഥ xyzModem_frame:
      തിരികെ "Invalid framing";
      നിർത്തുക;
    അവസ്ഥ xyzModem_cksum:
      തിരികെ "CRC/checksum error";
      നിർത്തുക;
    അവസ്ഥ xyzModem_sequence:
      തിരികെ "Block sequence error";
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      തിരികെ "Unknown error";
      നിർത്തുക;
    }
}

/*
 * RedBoot interface
 */
