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
#include <xyzModem.h>
#include <stdarg.h>
#include <time.h>
#include <u-boot/crc.h>
#include <watchdog.h>
#include <env.h>
#include <vsprintf.h>

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
static struct
{
  ціле *__chan;
  unsigned char pkt[1024], *bufp;
  ціле pad_pending;	/* trailing ^Z run withheld from the previous block */
  ціле pad_deliver;	/* ^Z bytes to emit before the next data */
  unsigned char blk, cblk, crc1, crc2;
  unsigned char next_blk;	/* Expected block */
  ціле len, mode, total_retries;
  ціле total_SOH, total_STX, total_CAN;
  bool crc_mode, at_eof, tx_ack;
  bool first_xmodem_packet;
  ulong initial_time, timeout;
  unsigned long file_length, read_length;
} xyz;

#define xyzModem_CHAR_TIMEOUT            2000	/* 2 seconds */
#define xyzModem_MAX_RETRIES             20
#define xyzModem_MAX_RETRIES_WITH_CRC    10
#define xyzModem_CAN_COUNT                3	/* Wait for 3 CAN before quitting */

typedef ціле cyg_int32;
static ціле
CYGACC_COMM_IF_GETC_TIMEOUT (char chan, char *c)
{

  ulong now = get_timer(0);
  schedule();
  поки (!tstc ())
    {
      якщо (get_timer(now) > xyzModem_CHAR_TIMEOUT)
        break;
    }
  якщо (tstc ())
    {
      *c = getchar();
      повернути 1;
    }
  повернути 0;
}

static void
CYGACC_COMM_IF_PUTC (char x, char y)
{
  putc (y);
}

/* Validate a hex character */
__inline__ static bool
_is_hex (char c)
{
  повернути (((c >= '0') && (c <= '9')) ||
	  ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

/* Convert a single hex nibble */
__inline__ static ціле
_from_hex (char c)
{
  ціле ret = 0;

  якщо ((c >= '0') && (c <= '9'))
    {
      ret = (c - '0');
    }
  інакше якщо ((c >= 'a') && (c <= 'f'))
    {
      ret = (c - 'a' + 0x0a);
    }
  інакше якщо ((c >= 'A') && (c <= 'F'))
    {
      ret = (c - 'A' + 0x0A);
    }
  повернути ret;
}

/* Convert a character to lower case */
__inline__ static char
_tolower (char c)
{
  якщо ((c >= 'A') && (c <= 'Z'))
    {
      c = (c - 'A') + 'a';
    }
  повернути c;
}

/* Parse (scan) a number */
static bool
parse_num (char *s, unsigned long *val, char **es, char *delim)
{
  bool first = true;
  ціле radix = 10;
  char c;
  unsigned long result = 0;
  ціле digit;

  поки (*s == ' ')
    s++;
  поки (*s)
    {
      якщо (first && (s[0] == '0') && (_tolower (s[1]) == 'x'))
	{
	  radix = 16;
	  s += 2;
	}
      first = false;
      c = *s++;
      якщо (_is_hex (c) && ((digit = _from_hex (c)) < radix))
	{
	  /* Valid digit */
	  result = (result * radix) + digit;
	}
      інакше
	{
	  якщо (delim != (char *) 0)
	    {
	      /* See if this character is one of the delimiters */
	      char *dp = delim;
	      поки (*dp && (c != *dp))
		dp++;
	      якщо (*dp)
		break;		/* Found a good delimiter */
	    }
	  повернути false;		/* Malformatted number */
	}
    }
  *val = result;
  якщо (es != (char **) 0)
    {
      *es = s;
    }
  повернути true;
}

#якщо defined(DEBUG) && !CONFIG_IS_ENABLED(USE_TINY_PRINTF)
/*
 * Note: this debug setup works by storing the strings in a fixed buffer
 */
static char zm_debug_buf[8192];
static char *zm_out = zm_debug_buf;
static char *zm_out_start = zm_debug_buf;

static ціле
zm_dprintf(char *fmt, ...)
{
	ціле len;
	va_list args;

	va_start(args, fmt);
	len = diag_vsprintf(zm_out, fmt, args);
	va_end(args);
	zm_out += len;
	повернути len;
}

static void
zm_flush (void)
{
  zm_out = zm_out_start;
}

static void
zm_dump_buf (void *buf, ціле len)
{

}

static unsigned char zm_buf[2048];
static unsigned char *zm_bp;

static void
zm_new (void)
{
  zm_bp = zm_buf;
}

static void
zm_save (unsigned char c)
{
  *zm_bp++ = c;
}

static void
zm_dump (ціле line)
{
  zm_dprintf ("Packet at line: %d\n", line);
  zm_dump_buf (zm_buf, zm_bp - zm_buf);
}

#define ZM_DEBUG(x) x
#інакше
#define ZM_DEBUG(x)
#endif

/* Wait for the line to go idle */
static void
xyzModem_flush (void)
{
  ціле res;
  char c;
  поки (true)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      якщо (!res)
	повернути;
    }
}

static ціле
xyzModem_get_hdr (void)
{
  char c;
  ціле res;
  bool hdr_found = false;
  ціле i, can_total, hdr_chars;
  unsigned short cksum;

  ZM_DEBUG (zm_new ());
  /* Find the start of a header */
  can_total = 0;
  hdr_chars = 0;

  якщо (xyz.tx_ack)
    {
      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
      xyz.tx_ack = false;
    }
  поки (!hdr_found)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      якщо (res)
	{
	  hdr_chars++;
	  switch (c)
	    {
	    case SOH:
	      xyz.total_SOH++;
	      fallthrough;
	    case STX:
	      якщо (c == STX)
		xyz.total_STX++;
	      hdr_found = true;
	      break;
	    case CAN:
	    case ETX:
	      xyz.total_CAN++;
	      ZM_DEBUG (zm_dump (__LINE__));
	      якщо (++can_total == xyzModem_CAN_COUNT)
		{
		  повернути xyzModem_cancel;
		}
	      інакше
		{
		  /* Wait for multiple CAN to avoid early quits */
		  break;
		}
	    case EOT:
	      /* EOT only supported if no noise */
	      якщо (hdr_chars == 1)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK on EOT #%d\n", __LINE__));
		  ZM_DEBUG (zm_dump (__LINE__));
		  повернути xyzModem_eof;
		}
	    default:
	      /* Ignore, waiting for start of header */
	      ;
	    }
	}
      інакше
	{
	  /* Data stream timed out */
	  xyzModem_flush ();	/* Toss any current input */
	  ZM_DEBUG (zm_dump (__LINE__));
	  CYGACC_CALL_IF_DELAY_US ((cyg_int32) 250000);
	  повернути xyzModem_timeout;
	}
    }

  /* Header found, now read the data */
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (char *) &xyz.blk);
  ZM_DEBUG (zm_save (xyz.blk));
  якщо (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      повернути xyzModem_timeout;
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (char *) &xyz.cblk);
  ZM_DEBUG (zm_save (xyz.cblk));
  якщо (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      повернути xyzModem_timeout;
    }
  xyz.len = (c == SOH) ? 128 : 1024;
  xyz.bufp = xyz.pkt;
  для (i = 0; i < xyz.len; i++)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      якщо (res)
	{
	  xyz.pkt[i] = c;
	}
      інакше
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  повернути xyzModem_timeout;
	}
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (char *) &xyz.crc1);
  ZM_DEBUG (zm_save (xyz.crc1));
  якщо (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      повернути xyzModem_timeout;
    }
  якщо (xyz.crc_mode)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (char *) &xyz.crc2);
      ZM_DEBUG (zm_save (xyz.crc2));
      якщо (!res)
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  повернути xyzModem_timeout;
	}
    }
  ZM_DEBUG (zm_dump (__LINE__));
  /* Validate the message */
  якщо ((xyz.blk ^ xyz.cblk) != (unsigned char) 0xFF)
    {
      ZM_DEBUG (zm_dprintf
		("Framing error - blk: %x/%x/%x\n", xyz.blk, xyz.cblk,
		 (xyz.blk ^ xyz.cblk)));
      ZM_DEBUG (zm_dump_buf (xyz.pkt, xyz.len));
      xyzModem_flush ();
      повернути xyzModem_frame;
    }
  /* Verify checksum/CRC */
  якщо (xyz.crc_mode)
    {
      cksum = crc16_ccitt(0, xyz.pkt, xyz.len);
      якщо (cksum != ((xyz.crc1 << 8) | xyz.crc2))
	{
	  ZM_DEBUG (zm_dprintf ("CRC error - recvd: %02x%02x, computed: %x\n",
				xyz.crc1, xyz.crc2, cksum & 0xFFFF));
	  повернути xyzModem_cksum;
	}
    }
  інакше
    {
      cksum = 0;
      для (i = 0; i < xyz.len; i++)
	{
	  cksum += xyz.pkt[i];
	}
      якщо (xyz.crc1 != (cksum & 0xFF))
	{
	  ZM_DEBUG (zm_dprintf
		    ("Checksum error - recvd: %x, computed: %x\n", xyz.crc1,
		     cksum & 0xFF));
	  повернути xyzModem_cksum;
	}
    }
  /* If we get here, the message passes [structural] muster */
  повернути 0;
}

static
ulong
xyzModem_get_initial_timeout (void)
{
  /* timeout is in seconds, non-positive timeout value is infinity */
#якщо CONFIG_IS_ENABLED(ENV_SUPPORT)
  const char *timeout_str = env_get("loadxy_timeout");
  якщо (timeout_str)
    повернути 1000 * simple_strtol(timeout_str, NULL, 10);
#endif
  повернути 1000 * CONFIG_CMD_LOADXY_TIMEOUT;
}

ціле
xyzModem_stream_open (connection_info_t * info, ціле *err)
{
  ціле stat = 0;
  ціле retries = xyzModem_MAX_RETRIES;
  ціле crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;

/*    ZM_DEBUG(zm_out = zm_out_start); */
#ifdef xyzModem_zmodem
  якщо (info->mode == xyzModem_zmodem)
    {
      *err = xyzModem_noZmodem;
      повернути -1;
    }
#endif

/* TODO: CHECK ! */
  ціле dummy = 0;
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

  якщо (xyz.mode == xyzModem_xmodem)
    {
      /* X-modem doesn't have an information header - exit here */
      xyz.first_xmodem_packet = true;
      xyz.next_blk = 1;
      повернути 0;
    }

  поки (!(xyz.timeout && get_timer(xyz.initial_time) > xyz.timeout))
    {
      якщо (--retries <= 0)
        {
          retries = xyzModem_MAX_RETRIES;
          crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;
          xyz.crc_mode = true;
        }
      stat = xyzModem_get_hdr ();
      якщо (stat == 0)
	{
	  /* Y-modem file information header */
	  якщо (xyz.blk == 0)
	    {
	      /* skip filename */
	      поки (*xyz.bufp++);
	      /* get the length */
	      parse_num ((char *) xyz.bufp, &xyz.file_length, NULL, " ");
	      /* The rest of the file name data block quietly discarded */
	      xyz.tx_ack = true;
	    }
	  xyz.next_blk = 1;
	  xyz.len = 0;
	  повернути 0;
	}
      інакше якщо (stat == xyzModem_timeout)
	{
	  якщо (--crc_retries <= 0)
	    xyz.crc_mode = false;
	  CYGACC_CALL_IF_DELAY_US (5 * 100000);	/* Extra delay for startup */
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	  xyz.total_retries++;
	  ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	}
      якщо (stat == xyzModem_cancel)
	{
	  break;
	}
    }
  *err = stat;
  ZM_DEBUG (zm_flush ());
  повернути -1;
}

ціле
xyzModem_stream_read (char *buf, ціле size, ціле *err)
{
  ціле stat, total, len;
  ціле retries;

  total = 0;
  stat = xyzModem_cancel;
  /* Try and get 'size' bytes into the buffer */
  поки (!xyz.at_eof && xyz.len >= 0 && (size > 0))
    {
      якщо (xyz.len == 0 && xyz.pad_deliver == 0)
	{
	  retries = xyzModem_MAX_RETRIES;
	  поки (retries-- > 0)
	    {
	      якщо (xyz.first_xmodem_packet && xyz.timeout &&
		  get_timer(xyz.initial_time) > xyz.timeout)
		{
		  *err = xyzModem_timeout;
		  xyz.len = -1;
		  повернути total;
		}

	      stat = xyzModem_get_hdr ();
	      якщо (stat == 0)
		{
		  якщо (xyz.mode == xyzModem_xmodem && xyz.first_xmodem_packet)
		    xyz.first_xmodem_packet = false;
		  якщо (xyz.blk == xyz.next_blk)
		    {
		      xyz.tx_ack = true;
		      ZM_DEBUG (zm_dprintf
				("ACK block %d (%d)\n", xyz.blk, __LINE__));
		      xyz.next_blk = (xyz.next_blk + 1) & 0xFF;

		      якщо (xyz.mode == xyzModem_xmodem || xyz.file_length == 0)
			{
			  /* Without a file length, EOF padding can't be
			   * distinguished from block data until the next
			   * frame arrives and we can check for EOT/EOF.
			   */
			  xyz.pad_deliver = xyz.pad_pending;
			  xyz.pad_pending = 0;
			  якщо ((xyz.bufp[xyz.len - 1] == EOF) &&
			      (xyz.bufp[xyz.len - 2] == EOF) &&
			      (xyz.bufp[xyz.len - 3] == EOF))
			    {
			      поки (xyz.len
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
		      якщо (xyz.mode != xyzModem_xmodem && 0 != xyz.file_length)
			{
			  xyz.read_length += xyz.len;
			  якщо (xyz.read_length > xyz.file_length)
			    {
			      xyz.len -= (xyz.read_length - xyz.file_length);
			    }
			}
		      break;
		    }
		  інакше якщо (xyz.blk == ((xyz.next_blk - 1) & 0xFF))
		    {
		      /* Just re-ACK this so sender will get on with it */
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      continue;	/* Need new header */
		    }
		  інакше
		    {
		      stat = xyzModem_sequence;
		    }
		}
	      якщо (stat == xyzModem_cancel)
		{
		  break;
		}
	      якщо (stat == xyzModem_eof)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK (%d)\n", __LINE__));
		  якщо (xyz.mode == xyzModem_ymodem)
		    {
		      CYGACC_COMM_IF_PUTC (*xyz.__chan,
					   (xyz.crc_mode ? 'C' : NAK));
		      xyz.total_retries++;
		      ZM_DEBUG (zm_dprintf ("Reading Final Header\n"));
		      stat = xyzModem_get_hdr ();
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ZM_DEBUG (zm_dprintf ("FINAL ACK (%d)\n", __LINE__));
		    }
		  інакше
		    stat = 0;

		  xyz.pad_pending = 0;	/* EOT: withheld ^Z run was padding */
		  xyz.at_eof = true;
		  break;
		}
	      CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	      xyz.total_retries++;
	      ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	    }
	  якщо (stat < 0 && (!xyz.first_xmodem_packet || stat != xyzModem_timeout))
	    {
	      *err = stat;
	      xyz.len = -1;
	      повернути total;
	    }
	}
      /* A ^Z run withheld from the previous block turned out to be data */
      якщо (xyz.pad_deliver > 0)
	{
	  len = xyz.pad_deliver;
	  якщо (size < len)
	    len = size;
	  memset (buf, EOF, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.pad_deliver -= len;
	}
      /* Don't "read" data from the EOF protocol package */
      якщо (!xyz.at_eof && xyz.len > 0)
	{
	  len = xyz.len;
	  якщо (size < len)
	    len = size;
	  memcpy (buf, xyz.bufp, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.len -= len;
	  xyz.bufp += len;
	}
    }
  повернути total;
}

void
xyzModem_stream_close (ціле *err)
{
  ZM_DEBUG (zm_dprintf
    ("xyzModem - %s mode, %d(SOH)/%d(STX)/%d(CAN) packets, %d retries\n",
     xyz.crc_mode ? "CRC" : "Cksum", xyz.total_SOH, xyz.total_STX,
     xyz.total_CAN, xyz.total_retries));
  ZM_DEBUG (zm_flush ());
}

/* Need to be able to clean out the input buffer, so have to take the */
/* getc */
void
xyzModem_stream_terminate (bool abort, ціле (*getc) (void))
{
  ціле c;

  якщо (abort)
    {
      ZM_DEBUG (zm_dprintf ("!!!! TRANSFER ABORT !!!!\n"));
      switch (xyz.mode)
	{
	case xyzModem_xmodem:
	case xyzModem_ymodem:
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
	  break;
#ifdef xyzModem_zmodem
	case xyzModem_zmodem:
	  /* Might support it some day I suppose. */
#endif
	  break;
	}
    }
  інакше
    {
      ZM_DEBUG (zm_dprintf ("Engaging cleanup mode...\n"));
      /*
       * Consume any trailing crap left in the inbuffer from
       * previous received blocks. Since very few files are an exact multiple
       * of the transfer block size, there will almost always be some gunk here.
       * If we don't eat it now, RedBoot will think the user typed it.
       */
      ZM_DEBUG (zm_dprintf ("Trailing gunk:\n"));
      поки ((c = (*getc) ()) > -1)
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

char *
xyzModem_error (ціле err)
{
  switch (err)
    {
    case xyzModem_access:
      повернути "Can't access file";
      break;
    case xyzModem_noZmodem:
      повернути "Sorry, zModem not available yet";
      break;
    case xyzModem_timeout:
      повернути "Timed out";
      break;
    case xyzModem_eof:
      повернути "End of file";
      break;
    case xyzModem_cancel:
      повернути "Cancelled";
      break;
    case xyzModem_frame:
      повернути "Invalid framing";
      break;
    case xyzModem_cksum:
      повернути "CRC/checksum error";
      break;
    case xyzModem_sequence:
      повернути "Block sequence error";
      break;
    default:
      повернути "Unknown error";
      break;
    }
}

/*
 * RedBoot interface
 */
