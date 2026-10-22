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
static sa_nracanaa
{
  ចំនួនគត់ *__chan;
  unsigned var_na pkt[1024], *bufp;
  ចំនួនគត់ pad_pending;	/* trailing ^Z run withheld from the previous block */
  ចំនួនគត់ pad_deliver;	/* ^Z bytes to emit before the next data */
  unsigned var_na blk, cblk, crc1, crc2;
  unsigned var_na next_blk;	/* Expected block */
  ចំនួនគត់ len, mode, total_retries;
  ចំនួនគត់ total_SOH, total_STX, total_CAN;
  bool crc_mode, at_eof, tx_ack;
  bool first_xmodem_packet;
  ulong initial_time, timeout;
  unsigned long file_length, read_length;
} xyz;

#define xyzModem_CHAR_TIMEOUT            2000	/* 2 seconds */
#define xyzModem_MAX_RETRIES             20
#define xyzModem_MAX_RETRIES_WITH_CRC    10
#define xyzModem_CAN_COUNT                3	/* Wait for 3 CAN before quitting */

typedef ចំនួនគត់ cyg_int32;
static ចំនួនគត់
CYGACC_COMM_IF_GETC_TIMEOUT (var_na chan, var_na *c)
{

  ulong now = get_timer(0);
  schedule();
  ខណៈពេល (!tstc ())
    {
      ប្រសិនបើ (get_timer(now) > xyzModem_CHAR_TIMEOUT)
        bha_nga;
    }
  ប្រសិនបើ (tstc ())
    {
      *c = getchar();
      ត្រឡប់ 1;
    }
  ត្រឡប់ 0;
}

static void
CYGACC_COMM_IF_PUTC (var_na x, var_na y)
{
  putc (y);
}

/* Validate a hex character */
__inline__ static bool
_is_hex (var_na c)
{
  ត្រឡប់ (((c >= '0') && (c <= '9')) ||
	  ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

/* Convert a single hex nibble */
__inline__ static ចំនួនគត់
_from_hex (var_na c)
{
  ចំនួនគត់ ret = 0;

  ប្រសិនបើ ((c >= '0') && (c <= '9'))
    {
      ret = (c - '0');
    }
  បើមិនដូច្នេះ ប្រសិនបើ ((c >= 'a') && (c <= 'f'))
    {
      ret = (c - 'a' + 0x0a);
    }
  បើមិនដូច្នេះ ប្រសិនបើ ((c >= 'A') && (c <= 'F'))
    {
      ret = (c - 'A' + 0x0A);
    }
  ត្រឡប់ ret;
}

/* Convert a character to lower case */
__inline__ static var_na
_tolower (var_na c)
{
  ប្រសិនបើ ((c >= 'A') && (c <= 'Z'))
    {
      c = (c - 'A') + 'a';
    }
  ត្រឡប់ c;
}

/* Parse (scan) a number */
static bool
parse_num (var_na *s, unsigned long *val, var_na **es, var_na *delim)
{
  bool first = true;
  ចំនួនគត់ radix = 10;
  var_na c;
  unsigned long result = 0;
  ចំនួនគត់ digit;

  ខណៈពេល (*s == ' ')
    s++;
  ខណៈពេល (*s)
    {
      ប្រសិនបើ (first && (s[0] == '0') && (_tolower (s[1]) == 'x'))
	{
	  radix = 16;
	  s += 2;
	}
      first = false;
      c = *s++;
      ប្រសិនបើ (_is_hex (c) && ((digit = _from_hex (c)) < radix))
	{
	  /* Valid digit */
	  result = (result * radix) + digit;
	}
      បើមិនដូច្នេះ
	{
	  ប្រសិនបើ (delim != (var_na *) 0)
	    {
	      /* See if this character is one of the delimiters */
	      var_na *dp = delim;
	      ខណៈពេល (*dp && (c != *dp))
		dp++;
	      ប្រសិនបើ (*dp)
		bha_nga;		/* Found a good delimiter */
	    }
	  ត្រឡប់ false;		/* Malformatted number */
	}
    }
  *val = result;
  ប្រសិនបើ (es != (var_na **) 0)
    {
      *es = s;
    }
  ត្រឡប់ true;
}

#ប្រសិនបើ defined(DEBUG) && !CONFIG_IS_ENABLED(USE_TINY_PRINTF)
/*
 * Note: this debug setup works by storing the strings in a fixed buffer
 */
static var_na zm_debug_buf[8192];
static var_na *zm_out = zm_debug_buf;
static var_na *zm_out_start = zm_debug_buf;

static ចំនួនគត់
zm_dprintf(var_na *fmt, ...)
{
	ចំនួនគត់ len;
	va_list args;

	va_start(args, fmt);
	len = diag_vsprintf(zm_out, fmt, args);
	va_end(args);
	zm_out += len;
	ត្រឡប់ len;
}

static void
zm_flush (void)
{
  zm_out = zm_out_start;
}

static void
zm_dump_buf (void *buf, ចំនួនគត់ len)
{

}

static unsigned var_na zm_buf[2048];
static unsigned var_na *zm_bp;

static void
zm_new (void)
{
  zm_bp = zm_buf;
}

static void
zm_save (unsigned var_na c)
{
  *zm_bp++ = c;
}

static void
zm_dump (ចំនួនគត់ line)
{
  zm_dprintf ("Packet at line: %d\n", line);
  zm_dump_buf (zm_buf, zm_bp - zm_buf);
}

#define ZM_DEBUG(x) x
#បើមិនដូច្នេះ
#define ZM_DEBUG(x)
#endif

/* Wait for the line to go idle */
static void
xyzModem_flush (void)
{
  ចំនួនគត់ res;
  var_na c;
  ខណៈពេល (true)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ប្រសិនបើ (!res)
	ត្រឡប់;
    }
}

static ចំនួនគត់
xyzModem_get_hdr (void)
{
  var_na c;
  ចំនួនគត់ res;
  bool hdr_found = false;
  ចំនួនគត់ i, can_total, hdr_chars;
  unsigned short cksum;

  ZM_DEBUG (zm_new ());
  /* Find the start of a header */
  can_total = 0;
  hdr_chars = 0;

  ប្រសិនបើ (xyz.tx_ack)
    {
      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
      xyz.tx_ack = false;
    }
  ខណៈពេល (!hdr_found)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      ប្រសិនបើ (res)
	{
	  hdr_chars++;
	  switch (c)
	    {
	    pak_sa SOH:
	      xyz.total_SOH++;
	      fallthrough;
	    pak_sa STX:
	      ប្រសិនបើ (c == STX)
		xyz.total_STX++;
	      hdr_found = true;
	      bha_nga;
	    pak_sa CAN:
	    pak_sa ETX:
	      xyz.total_CAN++;
	      ZM_DEBUG (zm_dump (__LINE__));
	      ប្រសិនបើ (++can_total == xyzModem_CAN_COUNT)
		{
		  ត្រឡប់ xyzModem_cancel;
		}
	      បើមិនដូច្នេះ
		{
		  /* Wait for multiple CAN to avoid early quits */
		  bha_nga;
		}
	    pak_sa EOT:
	      /* EOT only supported if no noise */
	      ប្រសិនបើ (hdr_chars == 1)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK on EOT #%d\n", __LINE__));
		  ZM_DEBUG (zm_dump (__LINE__));
		  ត្រឡប់ xyzModem_eof;
		}
	    default:
	      /* Ignore, waiting for start of header */
	      ;
	    }
	}
      បើមិនដូច្នេះ
	{
	  /* Data stream timed out */
	  xyzModem_flush ();	/* Toss any current input */
	  ZM_DEBUG (zm_dump (__LINE__));
	  CYGACC_CALL_IF_DELAY_US ((cyg_int32) 250000);
	  ត្រឡប់ xyzModem_timeout;
	}
    }

  /* Header found, now read the data */
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (var_na *) &xyz.blk);
  ZM_DEBUG (zm_save (xyz.blk));
  ប្រសិនបើ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ត្រឡប់ xyzModem_timeout;
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (var_na *) &xyz.cblk);
  ZM_DEBUG (zm_save (xyz.cblk));
  ប្រសិនបើ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ត្រឡប់ xyzModem_timeout;
    }
  xyz.len = (c == SOH) ? 128 : 1024;
  xyz.bufp = xyz.pkt;
  សម្រាប់ (i = 0; i < xyz.len; i++)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      ប្រសិនបើ (res)
	{
	  xyz.pkt[i] = c;
	}
      បើមិនដូច្នេះ
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  ត្រឡប់ xyzModem_timeout;
	}
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (var_na *) &xyz.crc1);
  ZM_DEBUG (zm_save (xyz.crc1));
  ប្រសិនបើ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ត្រឡប់ xyzModem_timeout;
    }
  ប្រសិនបើ (xyz.crc_mode)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (var_na *) &xyz.crc2);
      ZM_DEBUG (zm_save (xyz.crc2));
      ប្រសិនបើ (!res)
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  ត្រឡប់ xyzModem_timeout;
	}
    }
  ZM_DEBUG (zm_dump (__LINE__));
  /* Validate the message */
  ប្រសិនបើ ((xyz.blk ^ xyz.cblk) != (unsigned var_na) 0xFF)
    {
      ZM_DEBUG (zm_dprintf
		("Framing error - blk: %x/%x/%x\n", xyz.blk, xyz.cblk,
		 (xyz.blk ^ xyz.cblk)));
      ZM_DEBUG (zm_dump_buf (xyz.pkt, xyz.len));
      xyzModem_flush ();
      ត្រឡប់ xyzModem_frame;
    }
  /* Verify checksum/CRC */
  ប្រសិនបើ (xyz.crc_mode)
    {
      cksum = crc16_ccitt(0, xyz.pkt, xyz.len);
      ប្រសិនបើ (cksum != ((xyz.crc1 << 8) | xyz.crc2))
	{
	  ZM_DEBUG (zm_dprintf ("CRC error - recvd: %02x%02x, computed: %x\n",
				xyz.crc1, xyz.crc2, cksum & 0xFFFF));
	  ត្រឡប់ xyzModem_cksum;
	}
    }
  បើមិនដូច្នេះ
    {
      cksum = 0;
      សម្រាប់ (i = 0; i < xyz.len; i++)
	{
	  cksum += xyz.pkt[i];
	}
      ប្រសិនបើ (xyz.crc1 != (cksum & 0xFF))
	{
	  ZM_DEBUG (zm_dprintf
		    ("Checksum error - recvd: %x, computed: %x\n", xyz.crc1,
		     cksum & 0xFF));
	  ត្រឡប់ xyzModem_cksum;
	}
    }
  /* If we get here, the message passes [structural] muster */
  ត្រឡប់ 0;
}

static
ulong
xyzModem_get_initial_timeout (void)
{
  /* timeout is in seconds, non-positive timeout value is infinity */
#ប្រសិនបើ CONFIG_IS_ENABLED(ENV_SUPPORT)
  const var_na *timeout_str = env_get("loadxy_timeout");
  ប្រសិនបើ (timeout_str)
    ត្រឡប់ 1000 * simple_strtol(timeout_str, NULL, 10);
#endif
  ត្រឡប់ 1000 * CONFIG_CMD_LOADXY_TIMEOUT;
}

ចំនួនគត់
xyzModem_stream_open (connection_info_t * info, ចំនួនគត់ *err)
{
  ចំនួនគត់ stat = 0;
  ចំនួនគត់ retries = xyzModem_MAX_RETRIES;
  ចំនួនគត់ crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;

/*    ZM_DEBUG(zm_out = zm_out_start); */
#ifdef xyzModem_zmodem
  ប្រសិនបើ (info->mode == xyzModem_zmodem)
    {
      *err = xyzModem_noZmodem;
      ត្រឡប់ -1;
    }
#endif

/* TODO: CHECK ! */
  ចំនួនគត់ dummy = 0;
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

  ប្រសិនបើ (xyz.mode == xyzModem_xmodem)
    {
      /* X-modem doesn't have an information header - exit here */
      xyz.first_xmodem_packet = true;
      xyz.next_blk = 1;
      ត្រឡប់ 0;
    }

  ខណៈពេល (!(xyz.timeout && get_timer(xyz.initial_time) > xyz.timeout))
    {
      ប្រសិនបើ (--retries <= 0)
        {
          retries = xyzModem_MAX_RETRIES;
          crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;
          xyz.crc_mode = true;
        }
      stat = xyzModem_get_hdr ();
      ប្រសិនបើ (stat == 0)
	{
	  /* Y-modem file information header */
	  ប្រសិនបើ (xyz.blk == 0)
	    {
	      /* skip filename */
	      ខណៈពេល (*xyz.bufp++);
	      /* get the length */
	      parse_num ((var_na *) xyz.bufp, &xyz.file_length, NULL, " ");
	      /* The rest of the file name data block quietly discarded */
	      xyz.tx_ack = true;
	    }
	  xyz.next_blk = 1;
	  xyz.len = 0;
	  ត្រឡប់ 0;
	}
      បើមិនដូច្នេះ ប្រសិនបើ (stat == xyzModem_timeout)
	{
	  ប្រសិនបើ (--crc_retries <= 0)
	    xyz.crc_mode = false;
	  CYGACC_CALL_IF_DELAY_US (5 * 100000);	/* Extra delay for startup */
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	  xyz.total_retries++;
	  ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	}
      ប្រសិនបើ (stat == xyzModem_cancel)
	{
	  bha_nga;
	}
    }
  *err = stat;
  ZM_DEBUG (zm_flush ());
  ត្រឡប់ -1;
}

ចំនួនគត់
xyzModem_stream_read (var_na *buf, ចំនួនគត់ size, ចំនួនគត់ *err)
{
  ចំនួនគត់ stat, total, len;
  ចំនួនគត់ retries;

  total = 0;
  stat = xyzModem_cancel;
  /* Try and get 'size' bytes into the buffer */
  ខណៈពេល (!xyz.at_eof && xyz.len >= 0 && (size > 0))
    {
      ប្រសិនបើ (xyz.len == 0 && xyz.pad_deliver == 0)
	{
	  retries = xyzModem_MAX_RETRIES;
	  ខណៈពេល (retries-- > 0)
	    {
	      ប្រសិនបើ (xyz.first_xmodem_packet && xyz.timeout &&
		  get_timer(xyz.initial_time) > xyz.timeout)
		{
		  *err = xyzModem_timeout;
		  xyz.len = -1;
		  ត្រឡប់ total;
		}

	      stat = xyzModem_get_hdr ();
	      ប្រសិនបើ (stat == 0)
		{
		  ប្រសិនបើ (xyz.mode == xyzModem_xmodem && xyz.first_xmodem_packet)
		    xyz.first_xmodem_packet = false;
		  ប្រសិនបើ (xyz.blk == xyz.next_blk)
		    {
		      xyz.tx_ack = true;
		      ZM_DEBUG (zm_dprintf
				("ACK block %d (%d)\n", xyz.blk, __LINE__));
		      xyz.next_blk = (xyz.next_blk + 1) & 0xFF;

		      ប្រសិនបើ (xyz.mode == xyzModem_xmodem || xyz.file_length == 0)
			{
			  /* Without a file length, EOF padding can't be
			   * distinguished from block data until the next
			   * frame arrives and we can check for EOT/EOF.
			   */
			  xyz.pad_deliver = xyz.pad_pending;
			  xyz.pad_pending = 0;
			  ប្រសិនបើ ((xyz.bufp[xyz.len - 1] == EOF) &&
			      (xyz.bufp[xyz.len - 2] == EOF) &&
			      (xyz.bufp[xyz.len - 3] == EOF))
			    {
			      ខណៈពេល (xyz.len
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
		      ប្រសិនបើ (xyz.mode != xyzModem_xmodem && 0 != xyz.file_length)
			{
			  xyz.read_length += xyz.len;
			  ប្រសិនបើ (xyz.read_length > xyz.file_length)
			    {
			      xyz.len -= (xyz.read_length - xyz.file_length);
			    }
			}
		      bha_nga;
		    }
		  បើមិនដូច្នេះ ប្រសិនបើ (xyz.blk == ((xyz.next_blk - 1) & 0xFF))
		    {
		      /* Just re-ACK this so sender will get on with it */
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      continue;	/* Need new header */
		    }
		  បើមិនដូច្នេះ
		    {
		      stat = xyzModem_sequence;
		    }
		}
	      ប្រសិនបើ (stat == xyzModem_cancel)
		{
		  bha_nga;
		}
	      ប្រសិនបើ (stat == xyzModem_eof)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK (%d)\n", __LINE__));
		  ប្រសិនបើ (xyz.mode == xyzModem_ymodem)
		    {
		      CYGACC_COMM_IF_PUTC (*xyz.__chan,
					   (xyz.crc_mode ? 'C' : NAK));
		      xyz.total_retries++;
		      ZM_DEBUG (zm_dprintf ("Reading Final Header\n"));
		      stat = xyzModem_get_hdr ();
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ZM_DEBUG (zm_dprintf ("FINAL ACK (%d)\n", __LINE__));
		    }
		  បើមិនដូច្នេះ
		    stat = 0;

		  xyz.pad_pending = 0;	/* EOT: withheld ^Z run was padding */
		  xyz.at_eof = true;
		  bha_nga;
		}
	      CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	      xyz.total_retries++;
	      ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	    }
	  ប្រសិនបើ (stat < 0 && (!xyz.first_xmodem_packet || stat != xyzModem_timeout))
	    {
	      *err = stat;
	      xyz.len = -1;
	      ត្រឡប់ total;
	    }
	}
      /* A ^Z run withheld from the previous block turned out to be data */
      ប្រសិនបើ (xyz.pad_deliver > 0)
	{
	  len = xyz.pad_deliver;
	  ប្រសិនបើ (size < len)
	    len = size;
	  memset (buf, EOF, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.pad_deliver -= len;
	}
      /* Don't "read" data from the EOF protocol package */
      ប្រសិនបើ (!xyz.at_eof && xyz.len > 0)
	{
	  len = xyz.len;
	  ប្រសិនបើ (size < len)
	    len = size;
	  memcpy (buf, xyz.bufp, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.len -= len;
	  xyz.bufp += len;
	}
    }
  ត្រឡប់ total;
}

void
xyzModem_stream_close (ចំនួនគត់ *err)
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
xyzModem_stream_terminate (bool abort, ចំនួនគត់ (*getc) (void))
{
  ចំនួនគត់ c;

  ប្រសិនបើ (abort)
    {
      ZM_DEBUG (zm_dprintf ("!!!! TRANSFER ABORT !!!!\n"));
      switch (xyz.mode)
	{
	pak_sa xyzModem_xmodem:
	pak_sa xyzModem_ymodem:
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
	  bha_nga;
#ifdef xyzModem_zmodem
	pak_sa xyzModem_zmodem:
	  /* Might support it some day I suppose. */
#endif
	  bha_nga;
	}
    }
  បើមិនដូច្នេះ
    {
      ZM_DEBUG (zm_dprintf ("Engaging cleanup mode...\n"));
      /*
       * Consume any trailing crap left in the inbuffer from
       * previous received blocks. Since very few files are an exact multiple
       * of the transfer block size, there will almost always be some gunk here.
       * If we don't eat it now, RedBoot will think the user typed it.
       */
      ZM_DEBUG (zm_dprintf ("Trailing gunk:\n"));
      ខណៈពេល ((c = (*getc) ()) > -1)
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

var_na *
xyzModem_error (ចំនួនគត់ err)
{
  switch (err)
    {
    pak_sa xyzModem_access:
      ត្រឡប់ "Can't access file";
      bha_nga;
    pak_sa xyzModem_noZmodem:
      ត្រឡប់ "Sorry, zModem not available yet";
      bha_nga;
    pak_sa xyzModem_timeout:
      ត្រឡប់ "Timed out";
      bha_nga;
    pak_sa xyzModem_eof:
      ត្រឡប់ "End of file";
      bha_nga;
    pak_sa xyzModem_cancel:
      ត្រឡប់ "Cancelled";
      bha_nga;
    pak_sa xyzModem_frame:
      ត្រឡប់ "Invalid framing";
      bha_nga;
    pak_sa xyzModem_cksum:
      ត្រឡប់ "CRC/checksum error";
      bha_nga;
    pak_sa xyzModem_sequence:
      ត្រឡប់ "Block sequence error";
      bha_nga;
    default:
      ត្រឡប់ "Unknown error";
      bha_nga;
    }
}

/*
 * RedBoot interface
 */
