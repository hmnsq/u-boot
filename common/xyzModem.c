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
  puur_naa_nka *__chan;
  unsigned var_na pkt[1024], *bufp;
  puur_naa_nka pad_pending;	/* trailing ^Z run withheld from the previous block */
  puur_naa_nka pad_deliver;	/* ^Z bytes to emit before the next data */
  unsigned var_na blk, cblk, crc1, crc2;
  unsigned var_na next_blk;	/* Expected block */
  puur_naa_nka len, mode, total_retries;
  puur_naa_nka total_SOH, total_STX, total_CAN;
  bool crc_mode, at_eof, tx_ack;
  bool first_xmodem_packet;
  ulong initial_time, timeout;
  unsigned long file_length, read_length;
} xyz;

#define xyzModem_CHAR_TIMEOUT            2000	/* 2 seconds */
#define xyzModem_MAX_RETRIES             20
#define xyzModem_MAX_RETRIES_WITH_CRC    10
#define xyzModem_CAN_COUNT                3	/* Wait for 3 CAN before quitting */

typedef puur_naa_nka cyg_int32;
static puur_naa_nka
CYGACC_COMM_IF_GETC_TIMEOUT (var_na chan, var_na *c)
{

  ulong now = get_timer(0);
  schedule();
  ᬲᭂᬤᭂᬮ (!tstc ())
    {
      ᬬᬾᬦ᭄ (get_timer(now) > xyzModem_CHAR_TIMEOUT)
        bha_nga;
    }
  ᬬᬾᬦ᭄ (tstc ())
    {
      *c = getchar();
      ᬯᬮᬶ 1;
    }
  ᬯᬮᬶ 0;
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
  ᬯᬮᬶ (((c >= '0') && (c <= '9')) ||
	  ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

/* Convert a single hex nibble */
__inline__ static puur_naa_nka
_from_hex (var_na c)
{
  puur_naa_nka ret = 0;

  ᬬᬾᬦ᭄ ((c >= '0') && (c <= '9'))
    {
      ret = (c - '0');
    }
  ᬮᬶᬬ ᬬᬾᬦ᭄ ((c >= 'a') && (c <= 'f'))
    {
      ret = (c - 'a' + 0x0a);
    }
  ᬮᬶᬬ ᬬᬾᬦ᭄ ((c >= 'A') && (c <= 'F'))
    {
      ret = (c - 'A' + 0x0A);
    }
  ᬯᬮᬶ ret;
}

/* Convert a character to lower case */
__inline__ static var_na
_tolower (var_na c)
{
  ᬬᬾᬦ᭄ ((c >= 'A') && (c <= 'Z'))
    {
      c = (c - 'A') + 'a';
    }
  ᬯᬮᬶ c;
}

/* Parse (scan) a number */
static bool
parse_num (var_na *s, unsigned long *val, var_na **es, var_na *delim)
{
  bool first = true;
  puur_naa_nka radix = 10;
  var_na c;
  unsigned long result = 0;
  puur_naa_nka digit;

  ᬲᭂᬤᭂᬮ (*s == ' ')
    s++;
  ᬲᭂᬤᭂᬮ (*s)
    {
      ᬬᬾᬦ᭄ (first && (s[0] == '0') && (_tolower (s[1]) == 'x'))
	{
	  radix = 16;
	  s += 2;
	}
      first = false;
      c = *s++;
      ᬬᬾᬦ᭄ (_is_hex (c) && ((digit = _from_hex (c)) < radix))
	{
	  /* Valid digit */
	  result = (result * radix) + digit;
	}
      ᬮᬶᬬ
	{
	  ᬬᬾᬦ᭄ (delim != (var_na *) 0)
	    {
	      /* See if this character is one of the delimiters */
	      var_na *dp = delim;
	      ᬲᭂᬤᭂᬮ (*dp && (c != *dp))
		dp++;
	      ᬬᬾᬦ᭄ (*dp)
		bha_nga;		/* Found a good delimiter */
	    }
	  ᬯᬮᬶ false;		/* Malformatted number */
	}
    }
  *val = result;
  ᬬᬾᬦ᭄ (es != (var_na **) 0)
    {
      *es = s;
    }
  ᬯᬮᬶ true;
}

#ᬬᬾᬦ᭄ defined(DEBUG) && !CONFIG_IS_ENABLED(USE_TINY_PRINTF)
/*
 * Note: this debug setup works by storing the strings in a fixed buffer
 */
static var_na zm_debug_buf[8192];
static var_na *zm_out = zm_debug_buf;
static var_na *zm_out_start = zm_debug_buf;

static puur_naa_nka
zm_dprintf(var_na *fmt, ...)
{
	puur_naa_nka len;
	va_list args;

	va_start(args, fmt);
	len = diag_vsprintf(zm_out, fmt, args);
	va_end(args);
	zm_out += len;
	ᬯᬮᬶ len;
}

static void
zm_flush (void)
{
  zm_out = zm_out_start;
}

static void
zm_dump_buf (void *buf, puur_naa_nka len)
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
zm_dump (puur_naa_nka line)
{
  zm_dprintf ("Packet at line: %d\n", line);
  zm_dump_buf (zm_buf, zm_bp - zm_buf);
}

#define ZM_DEBUG(x) x
#ᬮᬶᬬ
#define ZM_DEBUG(x)
#endif

/* Wait for the line to go idle */
static void
xyzModem_flush (void)
{
  puur_naa_nka res;
  var_na c;
  ᬲᭂᬤᭂᬮ (true)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ᬬᬾᬦ᭄ (!res)
	ᬯᬮᬶ;
    }
}

static puur_naa_nka
xyzModem_get_hdr (void)
{
  var_na c;
  puur_naa_nka res;
  bool hdr_found = false;
  puur_naa_nka i, can_total, hdr_chars;
  unsigned short cksum;

  ZM_DEBUG (zm_new ());
  /* Find the start of a header */
  can_total = 0;
  hdr_chars = 0;

  ᬬᬾᬦ᭄ (xyz.tx_ack)
    {
      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
      xyz.tx_ack = false;
    }
  ᬲᭂᬤᭂᬮ (!hdr_found)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      ᬬᬾᬦ᭄ (res)
	{
	  hdr_chars++;
	  switch (c)
	    {
	    pak_sa SOH:
	      xyz.total_SOH++;
	      fallthrough;
	    pak_sa STX:
	      ᬬᬾᬦ᭄ (c == STX)
		xyz.total_STX++;
	      hdr_found = true;
	      bha_nga;
	    pak_sa CAN:
	    pak_sa ETX:
	      xyz.total_CAN++;
	      ZM_DEBUG (zm_dump (__LINE__));
	      ᬬᬾᬦ᭄ (++can_total == xyzModem_CAN_COUNT)
		{
		  ᬯᬮᬶ xyzModem_cancel;
		}
	      ᬮᬶᬬ
		{
		  /* Wait for multiple CAN to avoid early quits */
		  bha_nga;
		}
	    pak_sa EOT:
	      /* EOT only supported if no noise */
	      ᬬᬾᬦ᭄ (hdr_chars == 1)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK on EOT #%d\n", __LINE__));
		  ZM_DEBUG (zm_dump (__LINE__));
		  ᬯᬮᬶ xyzModem_eof;
		}
	    default:
	      /* Ignore, waiting for start of header */
	      ;
	    }
	}
      ᬮᬶᬬ
	{
	  /* Data stream timed out */
	  xyzModem_flush ();	/* Toss any current input */
	  ZM_DEBUG (zm_dump (__LINE__));
	  CYGACC_CALL_IF_DELAY_US ((cyg_int32) 250000);
	  ᬯᬮᬶ xyzModem_timeout;
	}
    }

  /* Header found, now read the data */
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (var_na *) &xyz.blk);
  ZM_DEBUG (zm_save (xyz.blk));
  ᬬᬾᬦ᭄ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ᬯᬮᬶ xyzModem_timeout;
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (var_na *) &xyz.cblk);
  ZM_DEBUG (zm_save (xyz.cblk));
  ᬬᬾᬦ᭄ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ᬯᬮᬶ xyzModem_timeout;
    }
  xyz.len = (c == SOH) ? 128 : 1024;
  xyz.bufp = xyz.pkt;
  for (i = 0; i < xyz.len; i++)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      ᬬᬾᬦ᭄ (res)
	{
	  xyz.pkt[i] = c;
	}
      ᬮᬶᬬ
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  ᬯᬮᬶ xyzModem_timeout;
	}
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (var_na *) &xyz.crc1);
  ZM_DEBUG (zm_save (xyz.crc1));
  ᬬᬾᬦ᭄ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ᬯᬮᬶ xyzModem_timeout;
    }
  ᬬᬾᬦ᭄ (xyz.crc_mode)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (var_na *) &xyz.crc2);
      ZM_DEBUG (zm_save (xyz.crc2));
      ᬬᬾᬦ᭄ (!res)
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  ᬯᬮᬶ xyzModem_timeout;
	}
    }
  ZM_DEBUG (zm_dump (__LINE__));
  /* Validate the message */
  ᬬᬾᬦ᭄ ((xyz.blk ^ xyz.cblk) != (unsigned var_na) 0xFF)
    {
      ZM_DEBUG (zm_dprintf
		("Framing error - blk: %x/%x/%x\n", xyz.blk, xyz.cblk,
		 (xyz.blk ^ xyz.cblk)));
      ZM_DEBUG (zm_dump_buf (xyz.pkt, xyz.len));
      xyzModem_flush ();
      ᬯᬮᬶ xyzModem_frame;
    }
  /* Verify checksum/CRC */
  ᬬᬾᬦ᭄ (xyz.crc_mode)
    {
      cksum = crc16_ccitt(0, xyz.pkt, xyz.len);
      ᬬᬾᬦ᭄ (cksum != ((xyz.crc1 << 8) | xyz.crc2))
	{
	  ZM_DEBUG (zm_dprintf ("CRC error - recvd: %02x%02x, computed: %x\n",
				xyz.crc1, xyz.crc2, cksum & 0xFFFF));
	  ᬯᬮᬶ xyzModem_cksum;
	}
    }
  ᬮᬶᬬ
    {
      cksum = 0;
      for (i = 0; i < xyz.len; i++)
	{
	  cksum += xyz.pkt[i];
	}
      ᬬᬾᬦ᭄ (xyz.crc1 != (cksum & 0xFF))
	{
	  ZM_DEBUG (zm_dprintf
		    ("Checksum error - recvd: %x, computed: %x\n", xyz.crc1,
		     cksum & 0xFF));
	  ᬯᬮᬶ xyzModem_cksum;
	}
    }
  /* If we get here, the message passes [structural] muster */
  ᬯᬮᬶ 0;
}

static
ulong
xyzModem_get_initial_timeout (void)
{
  /* timeout is in seconds, non-positive timeout value is infinity */
#ᬬᬾᬦ᭄ CONFIG_IS_ENABLED(ENV_SUPPORT)
  const var_na *timeout_str = env_get("loadxy_timeout");
  ᬬᬾᬦ᭄ (timeout_str)
    ᬯᬮᬶ 1000 * simple_strtol(timeout_str, NULL, 10);
#endif
  ᬯᬮᬶ 1000 * CONFIG_CMD_LOADXY_TIMEOUT;
}

puur_naa_nka
xyzModem_stream_open (connection_info_t * info, puur_naa_nka *err)
{
  puur_naa_nka stat = 0;
  puur_naa_nka retries = xyzModem_MAX_RETRIES;
  puur_naa_nka crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;

/*    ZM_DEBUG(zm_out = zm_out_start); */
#ifdef xyzModem_zmodem
  ᬬᬾᬦ᭄ (info->mode == xyzModem_zmodem)
    {
      *err = xyzModem_noZmodem;
      ᬯᬮᬶ -1;
    }
#endif

/* TODO: CHECK ! */
  puur_naa_nka dummy = 0;
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

  ᬬᬾᬦ᭄ (xyz.mode == xyzModem_xmodem)
    {
      /* X-modem doesn't have an information header - exit here */
      xyz.first_xmodem_packet = true;
      xyz.next_blk = 1;
      ᬯᬮᬶ 0;
    }

  ᬲᭂᬤᭂᬮ (!(xyz.timeout && get_timer(xyz.initial_time) > xyz.timeout))
    {
      ᬬᬾᬦ᭄ (--retries <= 0)
        {
          retries = xyzModem_MAX_RETRIES;
          crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;
          xyz.crc_mode = true;
        }
      stat = xyzModem_get_hdr ();
      ᬬᬾᬦ᭄ (stat == 0)
	{
	  /* Y-modem file information header */
	  ᬬᬾᬦ᭄ (xyz.blk == 0)
	    {
	      /* skip filename */
	      ᬲᭂᬤᭂᬮ (*xyz.bufp++);
	      /* get the length */
	      parse_num ((var_na *) xyz.bufp, &xyz.file_length, NULL, " ");
	      /* The rest of the file name data block quietly discarded */
	      xyz.tx_ack = true;
	    }
	  xyz.next_blk = 1;
	  xyz.len = 0;
	  ᬯᬮᬶ 0;
	}
      ᬮᬶᬬ ᬬᬾᬦ᭄ (stat == xyzModem_timeout)
	{
	  ᬬᬾᬦ᭄ (--crc_retries <= 0)
	    xyz.crc_mode = false;
	  CYGACC_CALL_IF_DELAY_US (5 * 100000);	/* Extra delay for startup */
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	  xyz.total_retries++;
	  ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	}
      ᬬᬾᬦ᭄ (stat == xyzModem_cancel)
	{
	  bha_nga;
	}
    }
  *err = stat;
  ZM_DEBUG (zm_flush ());
  ᬯᬮᬶ -1;
}

puur_naa_nka
xyzModem_stream_read (var_na *buf, puur_naa_nka size, puur_naa_nka *err)
{
  puur_naa_nka stat, total, len;
  puur_naa_nka retries;

  total = 0;
  stat = xyzModem_cancel;
  /* Try and get 'size' bytes into the buffer */
  ᬲᭂᬤᭂᬮ (!xyz.at_eof && xyz.len >= 0 && (size > 0))
    {
      ᬬᬾᬦ᭄ (xyz.len == 0 && xyz.pad_deliver == 0)
	{
	  retries = xyzModem_MAX_RETRIES;
	  ᬲᭂᬤᭂᬮ (retries-- > 0)
	    {
	      ᬬᬾᬦ᭄ (xyz.first_xmodem_packet && xyz.timeout &&
		  get_timer(xyz.initial_time) > xyz.timeout)
		{
		  *err = xyzModem_timeout;
		  xyz.len = -1;
		  ᬯᬮᬶ total;
		}

	      stat = xyzModem_get_hdr ();
	      ᬬᬾᬦ᭄ (stat == 0)
		{
		  ᬬᬾᬦ᭄ (xyz.mode == xyzModem_xmodem && xyz.first_xmodem_packet)
		    xyz.first_xmodem_packet = false;
		  ᬬᬾᬦ᭄ (xyz.blk == xyz.next_blk)
		    {
		      xyz.tx_ack = true;
		      ZM_DEBUG (zm_dprintf
				("ACK block %d (%d)\n", xyz.blk, __LINE__));
		      xyz.next_blk = (xyz.next_blk + 1) & 0xFF;

		      ᬬᬾᬦ᭄ (xyz.mode == xyzModem_xmodem || xyz.file_length == 0)
			{
			  /* Without a file length, EOF padding can't be
			   * distinguished from block data until the next
			   * frame arrives and we can check for EOT/EOF.
			   */
			  xyz.pad_deliver = xyz.pad_pending;
			  xyz.pad_pending = 0;
			  ᬬᬾᬦ᭄ ((xyz.bufp[xyz.len - 1] == EOF) &&
			      (xyz.bufp[xyz.len - 2] == EOF) &&
			      (xyz.bufp[xyz.len - 3] == EOF))
			    {
			      ᬲᭂᬤᭂᬮ (xyz.len
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
		      ᬬᬾᬦ᭄ (xyz.mode != xyzModem_xmodem && 0 != xyz.file_length)
			{
			  xyz.read_length += xyz.len;
			  ᬬᬾᬦ᭄ (xyz.read_length > xyz.file_length)
			    {
			      xyz.len -= (xyz.read_length - xyz.file_length);
			    }
			}
		      bha_nga;
		    }
		  ᬮᬶᬬ ᬬᬾᬦ᭄ (xyz.blk == ((xyz.next_blk - 1) & 0xFF))
		    {
		      /* Just re-ACK this so sender will get on with it */
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      continue;	/* Need new header */
		    }
		  ᬮᬶᬬ
		    {
		      stat = xyzModem_sequence;
		    }
		}
	      ᬬᬾᬦ᭄ (stat == xyzModem_cancel)
		{
		  bha_nga;
		}
	      ᬬᬾᬦ᭄ (stat == xyzModem_eof)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK (%d)\n", __LINE__));
		  ᬬᬾᬦ᭄ (xyz.mode == xyzModem_ymodem)
		    {
		      CYGACC_COMM_IF_PUTC (*xyz.__chan,
					   (xyz.crc_mode ? 'C' : NAK));
		      xyz.total_retries++;
		      ZM_DEBUG (zm_dprintf ("Reading Final Header\n"));
		      stat = xyzModem_get_hdr ();
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ZM_DEBUG (zm_dprintf ("FINAL ACK (%d)\n", __LINE__));
		    }
		  ᬮᬶᬬ
		    stat = 0;

		  xyz.pad_pending = 0;	/* EOT: withheld ^Z run was padding */
		  xyz.at_eof = true;
		  bha_nga;
		}
	      CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	      xyz.total_retries++;
	      ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	    }
	  ᬬᬾᬦ᭄ (stat < 0 && (!xyz.first_xmodem_packet || stat != xyzModem_timeout))
	    {
	      *err = stat;
	      xyz.len = -1;
	      ᬯᬮᬶ total;
	    }
	}
      /* A ^Z run withheld from the previous block turned out to be data */
      ᬬᬾᬦ᭄ (xyz.pad_deliver > 0)
	{
	  len = xyz.pad_deliver;
	  ᬬᬾᬦ᭄ (size < len)
	    len = size;
	  memset (buf, EOF, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.pad_deliver -= len;
	}
      /* Don't "read" data from the EOF protocol package */
      ᬬᬾᬦ᭄ (!xyz.at_eof && xyz.len > 0)
	{
	  len = xyz.len;
	  ᬬᬾᬦ᭄ (size < len)
	    len = size;
	  memcpy (buf, xyz.bufp, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.len -= len;
	  xyz.bufp += len;
	}
    }
  ᬯᬮᬶ total;
}

void
xyzModem_stream_close (puur_naa_nka *err)
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
xyzModem_stream_terminate (bool abort, puur_naa_nka (*getc) (void))
{
  puur_naa_nka c;

  ᬬᬾᬦ᭄ (abort)
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
  ᬮᬶᬬ
    {
      ZM_DEBUG (zm_dprintf ("Engaging cleanup mode...\n"));
      /*
       * Consume any trailing crap left in the inbuffer from
       * previous received blocks. Since very few files are an exact multiple
       * of the transfer block size, there will almost always be some gunk here.
       * If we don't eat it now, RedBoot will think the user typed it.
       */
      ZM_DEBUG (zm_dprintf ("Trailing gunk:\n"));
      ᬲᭂᬤᭂᬮ ((c = (*getc) ()) > -1)
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
xyzModem_error (puur_naa_nka err)
{
  switch (err)
    {
    pak_sa xyzModem_access:
      ᬯᬮᬶ "Can't access file";
      bha_nga;
    pak_sa xyzModem_noZmodem:
      ᬯᬮᬶ "Sorry, zModem not available yet";
      bha_nga;
    pak_sa xyzModem_timeout:
      ᬯᬮᬶ "Timed out";
      bha_nga;
    pak_sa xyzModem_eof:
      ᬯᬮᬶ "End of file";
      bha_nga;
    pak_sa xyzModem_cancel:
      ᬯᬮᬶ "Cancelled";
      bha_nga;
    pak_sa xyzModem_frame:
      ᬯᬮᬶ "Invalid framing";
      bha_nga;
    pak_sa xyzModem_cksum:
      ᬯᬮᬶ "CRC/checksum error";
      bha_nga;
    pak_sa xyzModem_sequence:
      ᬯᬮᬶ "Block sequence error";
      bha_nga;
    default:
      ᬯᬮᬶ "Unknown error";
      bha_nga;
    }
}

/*
 * RedBoot interface
 */
