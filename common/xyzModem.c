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
#ಅನ್ತರ್ಭಾವ <xyzModem.h>
#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <u-boot/crc.h>
#ಅನ್ತರ್ಭಾವ <watchdog.h>
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <vsprintf.h>

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
ಸ್ಥಿರ ಸಂರಚನಾ
{
  ಪೂರ್ಣಾಙ್ಕ *__chan;
  unsigned ವರ್ಣ pkt[1024], *bufp;
  ಪೂರ್ಣಾಙ್ಕ pad_pending;	/* trailing ^Z run withheld from the previous block */
  ಪೂರ್ಣಾಙ್ಕ pad_deliver;	/* ^Z bytes to emit before the next data */
  unsigned ವರ್ಣ blk, cblk, crc1, crc2;
  unsigned ವರ್ಣ next_blk;	/* Expected block */
  ಪೂರ್ಣಾಙ್ಕ len, mode, total_retries;
  ಪೂರ್ಣಾಙ್ಕ total_SOH, total_STX, total_CAN;
  bool crc_mode, at_eof, tx_ack;
  bool first_xmodem_packet;
  ulong initial_time, timeout;
  unsigned long file_length, read_length;
} xyz;

#define xyzModem_CHAR_TIMEOUT            2000	/* 2 seconds */
#define xyzModem_MAX_RETRIES             20
#define xyzModem_MAX_RETRIES_WITH_CRC    10
#define xyz_Modem_CAN_ಎಣಿಕೆ                3	/* Wait for 3 CAN before quitting */

ಪ್ರಕಾರನಿರ್ದೇಶ ಪೂರ್ಣಾಙ್ಕ cyg_int32;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
CYGACC_COMM_IF_GETC_TIMEOUT (ವರ್ಣ chan, ವರ್ಣ *c)
{

  ulong now = get_timer(0);
  schedule();
  ಯಾವತ್ (!tstc ())
    {
      ಯದಿ (get_timer(now) > xyzModem_CHAR_TIMEOUT)
        ಭಙ್ಗ;
    }
  ಯದಿ (tstc ())
    {
      *c = getchar();
      ಪ್ರತ್ಯಾಗಮ 1;
    }
  ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ
CYGACC_COMM_IF_PUTC (ವರ್ಣ x, ವರ್ಣ y)
{
  putc (y);
}

/* Validate a hex character */
__inline__ ಸ್ಥಿರ bool
_is_hex (ವರ್ಣ c)
{
  ಪ್ರತ್ಯಾಗಮ (((c >= '0') && (c <= '9')) ||
	  ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

/* Convert a single hex nibble */
__inline__ ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
_from_hex (ವರ್ಣ c)
{
  ಪೂರ್ಣಾಙ್ಕ ret = 0;

  ಯದಿ ((c >= '0') && (c <= '9'))
    {
      ret = (c - '0');
    }
  ಅನ್ಯಥಾ ಯದಿ ((c >= 'a') && (c <= 'f'))
    {
      ret = (c - 'a' + 0x0a);
    }
  ಅನ್ಯಥಾ ಯದಿ ((c >= 'A') && (c <= 'F'))
    {
      ret = (c - 'A' + 0x0A);
    }
  ಪ್ರತ್ಯಾಗಮ ret;
}

/* Convert a character to lower case */
__inline__ ಸ್ಥಿರ ವರ್ಣ
_tolower (ವರ್ಣ c)
{
  ಯದಿ ((c >= 'A') && (c <= 'Z'))
    {
      c = (c - 'A') + 'a';
    }
  ಪ್ರತ್ಯಾಗಮ c;
}

/* Parse (scan) a number */
ಸ್ಥಿರ bool
parse_ಅಂಕೆ (ವರ್ಣ *s, unsigned long *val, ವರ್ಣ **es, ವರ್ಣ *delim)
{
  bool first = true;
  ಪೂರ್ಣಾಙ್ಕ radix = 10;
  ವರ್ಣ c;
  unsigned long result = 0;
  ಪೂರ್ಣಾಙ್ಕ digit;

  ಯಾವತ್ (*s == ' ')
    s++;
  ಯಾವತ್ (*s)
    {
      ಯದಿ (first && (s[0] == '0') && (_tolower (s[1]) == 'x'))
	{
	  radix = 16;
	  s += 2;
	}
      first = false;
      c = *s++;
      ಯದಿ (_is_hex (c) && ((digit = _from_hex (c)) < radix))
	{
	  /* Valid digit */
	  result = (result * radix) + digit;
	}
      ಅನ್ಯಥಾ
	{
	  ಯದಿ (delim != (ವರ್ಣ *) 0)
	    {
	      /* See if this character is one of the delimiters */
	      ವರ್ಣ *dp = delim;
	      ಯಾವತ್ (*dp && (c != *dp))
		dp++;
	      ಯದಿ (*dp)
		ಭಙ್ಗ;		/* Found a good delimiter */
	    }
	  ಪ್ರತ್ಯಾಗಮ false;		/* Malformatted number */
	}
    }
  *val = result;
  ಯದಿ (es != (ವರ್ಣ **) 0)
    {
      *es = s;
    }
  ಪ್ರತ್ಯಾಗಮ true;
}

#ಯದಿ defined(DEBUG) && !CONFIG_IS_ENABLED(USE_TINY_PRINTF)
/*
 * Note: this debug setup works by storing the strings in a fixed buffer
 */
ಸ್ಥಿರ ವರ್ಣ zm_debug_buf[8192];
ಸ್ಥಿರ ವರ್ಣ *zm_out = zm_debug_buf;
ಸ್ಥಿರ ವರ್ಣ *zm_out_start = zm_debug_buf;

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
zm_dprintf(ವರ್ಣ *fmt, ...)
{
	ಪೂರ್ಣಾಙ್ಕ len;
	va_list args;

	va_start(args, fmt);
	len = diag_vsprintf(zm_out, fmt, args);
	va_end(args);
	zm_out += len;
	ಪ್ರತ್ಯಾಗಮ len;
}

ಸ್ಥಿರ ಶೂನ್ಯ
zm_flush (ಶೂನ್ಯ)
{
  zm_out = zm_out_start;
}

ಸ್ಥಿರ ಶೂನ್ಯ
zm_dump_buf (ಶೂನ್ಯ *buf, ಪೂರ್ಣಾಙ್ಕ len)
{

}

ಸ್ಥಿರ unsigned ವರ್ಣ zm_buf[2048];
ಸ್ಥಿರ unsigned ವರ್ಣ *zm_bp;

ಸ್ಥಿರ ಶೂನ್ಯ
zm_new (ಶೂನ್ಯ)
{
  zm_bp = zm_buf;
}

ಸ್ಥಿರ ಶೂನ್ಯ
zm_save (unsigned ವರ್ಣ c)
{
  *zm_bp++ = c;
}

ಸ್ಥಿರ ಶೂನ್ಯ
zm_dump (ಪೂರ್ಣಾಙ್ಕ ಸಾಲು)
{
  zm_dprintf ("Packet at line: %d\n", ಸಾಲು);
  zm_dump_buf (zm_buf, zm_bp - zm_buf);
}

#define ZM_DEBUG(x) x
#ಅನ್ಯಥಾ
#define ZM_DEBUG(x)
#endif

/* Wait for the line to go idle */
ಸ್ಥಿರ ಶೂನ್ಯ
xyzModem_flush (ಶೂನ್ಯ)
{
  ಪೂರ್ಣಾಙ್ಕ res;
  ವರ್ಣ c;
  ಯಾವತ್ (true)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ಯದಿ (!res)
	ಪ್ರತ್ಯಾಗಮ;
    }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
xyzModem_get_hdr (ಶೂನ್ಯ)
{
  ವರ್ಣ c;
  ಪೂರ್ಣಾಙ್ಕ res;
  bool hdr_found = false;
  ಪೂರ್ಣಾಙ್ಕ i, can_total, hdr_chars;
  unsigned short cksum;

  ZM_DEBUG (zm_new ());
  /* Find the start of a header */
  can_total = 0;
  hdr_chars = 0;

  ಯದಿ (xyz.tx_ack)
    {
      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
      xyz.tx_ack = false;
    }
  ಯಾವತ್ (!hdr_found)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      ಯದಿ (res)
	{
	  hdr_chars++;
	  ವಿಕಲ್ಪ (c)
	    {
	    ಪಕ್ಷ SOH:
	      xyz.total_SOH++;
	      fallthrough;
	    ಪಕ್ಷ STX:
	      ಯದಿ (c == STX)
		xyz.total_STX++;
	      hdr_found = true;
	      ಭಙ್ಗ;
	    ಪಕ್ಷ CAN:
	    ಪಕ್ಷ ETX:
	      xyz.total_CAN++;
	      ZM_DEBUG (zm_dump (__LINE__));
	      ಯದಿ (++can_total == xyz_Modem_CAN_ಎಣಿಕೆ)
		{
		  ಪ್ರತ್ಯಾಗಮ xyzModem_cancel;
		}
	      ಅನ್ಯಥಾ
		{
		  /* Wait for multiple CAN to avoid early quits */
		  ಭಙ್ಗ;
		}
	    ಪಕ್ಷ EOT:
	      /* EOT only supported if no noise */
	      ಯದಿ (hdr_chars == 1)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK on EOT #%d\n", __LINE__));
		  ZM_DEBUG (zm_dump (__LINE__));
		  ಪ್ರತ್ಯಾಗಮ xyzModem_eof;
		}
	    ಸ್ವಭಾವ:
	      /* Ignore, waiting for start of header */
	      ;
	    }
	}
      ಅನ್ಯಥಾ
	{
	  /* Data stream timed out */
	  xyzModem_flush ();	/* Toss any current input */
	  ZM_DEBUG (zm_dump (__LINE__));
	  CYGACC_CALL_IF_DELAY_US ((cyg_int32) 250000);
	  ಪ್ರತ್ಯಾಗಮ xyzModem_timeout;
	}
    }

  /* Header found, now read the data */
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (ವರ್ಣ *) &xyz.blk);
  ZM_DEBUG (zm_save (xyz.blk));
  ಯದಿ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ಪ್ರತ್ಯಾಗಮ xyzModem_timeout;
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (ವರ್ಣ *) &xyz.cblk);
  ZM_DEBUG (zm_save (xyz.cblk));
  ಯದಿ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ಪ್ರತ್ಯಾಗಮ xyzModem_timeout;
    }
  xyz.len = (c == SOH) ? 128 : 1024;
  xyz.bufp = xyz.pkt;
  ಪ್ರತಿ (i = 0; i < xyz.len; i++)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      ಯದಿ (res)
	{
	  xyz.pkt[i] = c;
	}
      ಅನ್ಯಥಾ
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  ಪ್ರತ್ಯಾಗಮ xyzModem_timeout;
	}
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (ವರ್ಣ *) &xyz.crc1);
  ZM_DEBUG (zm_save (xyz.crc1));
  ಯದಿ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ಪ್ರತ್ಯಾಗಮ xyzModem_timeout;
    }
  ಯದಿ (xyz.crc_mode)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (ವರ್ಣ *) &xyz.crc2);
      ZM_DEBUG (zm_save (xyz.crc2));
      ಯದಿ (!res)
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  ಪ್ರತ್ಯಾಗಮ xyzModem_timeout;
	}
    }
  ZM_DEBUG (zm_dump (__LINE__));
  /* Validate the message */
  ಯದಿ ((xyz.blk ^ xyz.cblk) != (unsigned ವರ್ಣ) 0xFF)
    {
      ZM_DEBUG (zm_dprintf
		("Framing error - blk: %x/%x/%x\n", xyz.blk, xyz.cblk,
		 (xyz.blk ^ xyz.cblk)));
      ZM_DEBUG (zm_dump_buf (xyz.pkt, xyz.len));
      xyzModem_flush ();
      ಪ್ರತ್ಯಾಗಮ xyzModem_frame;
    }
  /* Verify checksum/CRC */
  ಯದಿ (xyz.crc_mode)
    {
      cksum = crc16_ccitt(0, xyz.pkt, xyz.len);
      ಯದಿ (cksum != ((xyz.crc1 << 8) | xyz.crc2))
	{
	  ZM_DEBUG (zm_dprintf ("CRC error - recvd: %02x%02x, computed: %x\n",
				xyz.crc1, xyz.crc2, cksum & 0xFFFF));
	  ಪ್ರತ್ಯಾಗಮ xyzModem_cksum;
	}
    }
  ಅನ್ಯಥಾ
    {
      cksum = 0;
      ಪ್ರತಿ (i = 0; i < xyz.len; i++)
	{
	  cksum += xyz.pkt[i];
	}
      ಯದಿ (xyz.crc1 != (cksum & 0xFF))
	{
	  ZM_DEBUG (zm_dprintf
		    ("Checksum error - recvd: %x, computed: %x\n", xyz.crc1,
		     cksum & 0xFF));
	  ಪ್ರತ್ಯಾಗಮ xyzModem_cksum;
	}
    }
  /* If we get here, the message passes [structural] muster */
  ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ
ulong
xyzModem_get_initial_timeout (ಶೂನ್ಯ)
{
  /* timeout is in seconds, non-positive timeout value is infinity */
#ಯದಿ CONFIG_IS_ENABLED(ENV_SUPPORT)
  ಧ್ರುವ ವರ್ಣ *timeout_str = env_get("loadxy_timeout");
  ಯದಿ (timeout_str)
    ಪ್ರತ್ಯಾಗಮ 1000 * simple_strtol(timeout_str, ರಿಕ್ತ, 10);
#endif
  ಪ್ರತ್ಯಾಗಮ 1000 * CONFIG_CMD_LOADXY_TIMEOUT;
}

ಪೂರ್ಣಾಙ್ಕ
xyzModem_stream_open (connection_info_t * info, ಪೂರ್ಣಾಙ್ಕ *err)
{
  ಪೂರ್ಣಾಙ್ಕ stat = 0;
  ಪೂರ್ಣಾಙ್ಕ retries = xyzModem_MAX_RETRIES;
  ಪೂರ್ಣಾಙ್ಕ crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;

/*    ZM_DEBUG(zm_out = zm_out_start); */
#ifdef xyzModem_zmodem
  ಯದಿ (info->mode == xyzModem_zmodem)
    {
      *err = xyzModem_noZmodem;
      ಪ್ರತ್ಯಾಗಮ -1;
    }
#endif

/* TODO: CHECK ! */
  ಪೂರ್ಣಾಙ್ಕ dummy = 0;
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

  ಯದಿ (xyz.mode == xyzModem_xmodem)
    {
      /* X-modem doesn't have an information header - exit here */
      xyz.first_xmodem_packet = true;
      xyz.next_blk = 1;
      ಪ್ರತ್ಯಾಗಮ 0;
    }

  ಯಾವತ್ (!(xyz.timeout && get_timer(xyz.initial_time) > xyz.timeout))
    {
      ಯದಿ (--retries <= 0)
        {
          retries = xyzModem_MAX_RETRIES;
          crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;
          xyz.crc_mode = true;
        }
      stat = xyzModem_get_hdr ();
      ಯದಿ (stat == 0)
	{
	  /* Y-modem file information header */
	  ಯದಿ (xyz.blk == 0)
	    {
	      /* skip filename */
	      ಯಾವತ್ (*xyz.bufp++);
	      /* get the length */
	      parse_ಅಂಕೆ ((ವರ್ಣ *) xyz.bufp, &xyz.file_length, ರಿಕ್ತ, " ");
	      /* The rest of the file name data block quietly discarded */
	      xyz.tx_ack = true;
	    }
	  xyz.next_blk = 1;
	  xyz.len = 0;
	  ಪ್ರತ್ಯಾಗಮ 0;
	}
      ಅನ್ಯಥಾ ಯದಿ (stat == xyzModem_timeout)
	{
	  ಯದಿ (--crc_retries <= 0)
	    xyz.crc_mode = false;
	  CYGACC_CALL_IF_DELAY_US (5 * 100000);	/* Extra delay for startup */
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	  xyz.total_retries++;
	  ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	}
      ಯದಿ (stat == xyzModem_cancel)
	{
	  ಭಙ್ಗ;
	}
    }
  *err = stat;
  ZM_DEBUG (zm_flush ());
  ಪ್ರತ್ಯಾಗಮ -1;
}

ಪೂರ್ಣಾಙ್ಕ
xyzModem_stream_read (ವರ್ಣ *buf, ಪೂರ್ಣಾಙ್ಕ size, ಪೂರ್ಣಾಙ್ಕ *err)
{
  ಪೂರ್ಣಾಙ್ಕ stat, total, len;
  ಪೂರ್ಣಾಙ್ಕ retries;

  total = 0;
  stat = xyzModem_cancel;
  /* Try and get 'size' bytes into the buffer */
  ಯಾವತ್ (!xyz.at_eof && xyz.len >= 0 && (size > 0))
    {
      ಯದಿ (xyz.len == 0 && xyz.pad_deliver == 0)
	{
	  retries = xyzModem_MAX_RETRIES;
	  ಯಾವತ್ (retries-- > 0)
	    {
	      ಯದಿ (xyz.first_xmodem_packet && xyz.timeout &&
		  get_timer(xyz.initial_time) > xyz.timeout)
		{
		  *err = xyzModem_timeout;
		  xyz.len = -1;
		  ಪ್ರತ್ಯಾಗಮ total;
		}

	      stat = xyzModem_get_hdr ();
	      ಯದಿ (stat == 0)
		{
		  ಯದಿ (xyz.mode == xyzModem_xmodem && xyz.first_xmodem_packet)
		    xyz.first_xmodem_packet = false;
		  ಯದಿ (xyz.blk == xyz.next_blk)
		    {
		      xyz.tx_ack = true;
		      ZM_DEBUG (zm_dprintf
				("ACK block %d (%d)\n", xyz.blk, __LINE__));
		      xyz.next_blk = (xyz.next_blk + 1) & 0xFF;

		      ಯದಿ (xyz.mode == xyzModem_xmodem || xyz.file_length == 0)
			{
			  /* Without a file length, EOF padding can't be
			   * distinguished from block data until the next
			   * frame arrives and we can check for EOT/EOF.
			   */
			  xyz.pad_deliver = xyz.pad_pending;
			  xyz.pad_pending = 0;
			  ಯದಿ ((xyz.bufp[xyz.len - 1] == EOF) &&
			      (xyz.bufp[xyz.len - 2] == EOF) &&
			      (xyz.bufp[xyz.len - 3] == EOF))
			    {
			      ಯಾವತ್ (xyz.len
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
		      ಯದಿ (xyz.mode != xyzModem_xmodem && 0 != xyz.file_length)
			{
			  xyz.read_length += xyz.len;
			  ಯದಿ (xyz.read_length > xyz.file_length)
			    {
			      xyz.len -= (xyz.read_length - xyz.file_length);
			    }
			}
		      ಭಙ್ಗ;
		    }
		  ಅನ್ಯಥಾ ಯದಿ (xyz.blk == ((xyz.next_blk - 1) & 0xFF))
		    {
		      /* Just re-ACK this so sender will get on with it */
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ಅನುವರ್ತ;	/* Need new header */
		    }
		  ಅನ್ಯಥಾ
		    {
		      stat = xyzModem_sequence;
		    }
		}
	      ಯದಿ (stat == xyzModem_cancel)
		{
		  ಭಙ್ಗ;
		}
	      ಯದಿ (stat == xyzModem_eof)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK (%d)\n", __LINE__));
		  ಯದಿ (xyz.mode == xyzModem_ymodem)
		    {
		      CYGACC_COMM_IF_PUTC (*xyz.__chan,
					   (xyz.crc_mode ? 'C' : NAK));
		      xyz.total_retries++;
		      ZM_DEBUG (zm_dprintf ("Reading Final Header\n"));
		      stat = xyzModem_get_hdr ();
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ZM_DEBUG (zm_dprintf ("FINAL ACK (%d)\n", __LINE__));
		    }
		  ಅನ್ಯಥಾ
		    stat = 0;

		  xyz.pad_pending = 0;	/* EOT: withheld ^Z run was padding */
		  xyz.at_eof = true;
		  ಭಙ್ಗ;
		}
	      CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	      xyz.total_retries++;
	      ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	    }
	  ಯದಿ (stat < 0 && (!xyz.first_xmodem_packet || stat != xyzModem_timeout))
	    {
	      *err = stat;
	      xyz.len = -1;
	      ಪ್ರತ್ಯಾಗಮ total;
	    }
	}
      /* A ^Z run withheld from the previous block turned out to be data */
      ಯದಿ (xyz.pad_deliver > 0)
	{
	  len = xyz.pad_deliver;
	  ಯದಿ (size < len)
	    len = size;
	  memset (buf, EOF, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.pad_deliver -= len;
	}
      /* Don't "read" data from the EOF protocol package */
      ಯದಿ (!xyz.at_eof && xyz.len > 0)
	{
	  len = xyz.len;
	  ಯದಿ (size < len)
	    len = size;
	  memcpy (buf, xyz.bufp, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.len -= len;
	  xyz.bufp += len;
	}
    }
  ಪ್ರತ್ಯಾಗಮ total;
}

ಶೂನ್ಯ
xyzModem_stream_close (ಪೂರ್ಣಾಙ್ಕ *err)
{
  ZM_DEBUG (zm_dprintf
    ("xyzModem - %s mode, %d(SOH)/%d(STX)/%d(CAN) packets, %d retries\n",
     xyz.crc_mode ? "CRC" : "Cksum", xyz.total_SOH, xyz.total_STX,
     xyz.total_CAN, xyz.total_retries));
  ZM_DEBUG (zm_flush ());
}

/* Need to be able to clean out the input buffer, so have to take the */
/* getc */
ಶೂನ್ಯ
xyzModem_stream_terminate (bool abort, ಪೂರ್ಣಾಙ್ಕ (*getc) (ಶೂನ್ಯ))
{
  ಪೂರ್ಣಾಙ್ಕ c;

  ಯದಿ (abort)
    {
      ZM_DEBUG (zm_dprintf ("!!!! TRANSFER ABORT !!!!\n"));
      ವಿಕಲ್ಪ (xyz.mode)
	{
	ಪಕ್ಷ xyzModem_xmodem:
	ಪಕ್ಷ xyzModem_ymodem:
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
	  ಭಙ್ಗ;
#ifdef xyzModem_zmodem
	ಪಕ್ಷ xyzModem_zmodem:
	  /* Might support it some day I suppose. */
#endif
	  ಭಙ್ಗ;
	}
    }
  ಅನ್ಯಥಾ
    {
      ZM_DEBUG (zm_dprintf ("Engaging cleanup mode...\n"));
      /*
       * Consume any trailing crap left in the inbuffer from
       * previous received blocks. Since very few files are an exact multiple
       * of the transfer block size, there will almost always be some gunk here.
       * If we don't eat it now, RedBoot will think the user typed it.
       */
      ZM_DEBUG (zm_dprintf ("Trailing gunk:\n"));
      ಯಾವತ್ ((c = (*getc) ()) > -1)
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

ವರ್ಣ *
xyzModem_error (ಪೂರ್ಣಾಙ್ಕ err)
{
  ವಿಕಲ್ಪ (err)
    {
    ಪಕ್ಷ xyzModem_access:
      ಪ್ರತ್ಯಾಗಮ "Can't access file";
      ಭಙ್ಗ;
    ಪಕ್ಷ xyzModem_noZmodem:
      ಪ್ರತ್ಯಾಗಮ "Sorry, zModem not available yet";
      ಭಙ್ಗ;
    ಪಕ್ಷ xyzModem_timeout:
      ಪ್ರತ್ಯಾಗಮ "Timed out";
      ಭಙ್ಗ;
    ಪಕ್ಷ xyzModem_eof:
      ಪ್ರತ್ಯಾಗಮ "End of file";
      ಭಙ್ಗ;
    ಪಕ್ಷ xyzModem_cancel:
      ಪ್ರತ್ಯಾಗಮ "Cancelled";
      ಭಙ್ಗ;
    ಪಕ್ಷ xyzModem_frame:
      ಪ್ರತ್ಯಾಗಮ "Invalid framing";
      ಭಙ್ಗ;
    ಪಕ್ಷ xyzModem_cksum:
      ಪ್ರತ್ಯಾಗಮ "CRC/checksum error";
      ಭಙ್ಗ;
    ಪಕ್ಷ xyzModem_sequence:
      ಪ್ರತ್ಯಾಗಮ "Block sequence error";
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ "Unknown error";
      ಭಙ್ಗ;
    }
}

/*
 * RedBoot interface
 */
