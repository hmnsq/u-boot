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
  ຈຳນວນເຕັມ *__chan;
  unsigned var_na pkt[1024], *bufp;
  ຈຳນວນເຕັມ pad_pending;	/* trailing ^Z run withheld from the previous block */
  ຈຳນວນເຕັມ pad_deliver;	/* ^Z bytes to emit before the next data */
  unsigned var_na blk, cblk, crc1, crc2;
  unsigned var_na next_blk;	/* Expected block */
  ຈຳນວນເຕັມ len, mode, total_retries;
  ຈຳນວນເຕັມ total_SOH, total_STX, total_CAN;
  bool crc_mode, at_eof, tx_ack;
  bool first_xmodem_packet;
  ulong initial_time, timeout;
  unsigned long file_length, read_length;
} xyz;

#define xyzModem_CHAR_TIMEOUT            2000	/* 2 seconds */
#define xyzModem_MAX_RETRIES             20
#define xyzModem_MAX_RETRIES_WITH_CRC    10
#define xyzModem_CAN_COUNT                3	/* Wait for 3 CAN before quitting */

typedef ຈຳນວນເຕັມ cyg_int32;
static ຈຳນວນເຕັມ
CYGACC_COMM_IF_GETC_TIMEOUT (var_na chan, var_na *c)
{

  ulong now = get_timer(0);
  schedule();
  ໃນຂະນະທີ່ (!tstc ())
    {
      ຖ້າ (get_timer(now) > xyzModem_CHAR_TIMEOUT)
        bha_nga;
    }
  ຖ້າ (tstc ())
    {
      *c = getchar();
      ສົ່ງຄືນ 1;
    }
  ສົ່ງຄືນ 0;
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
  ສົ່ງຄືນ (((c >= '0') && (c <= '9')) ||
	  ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

/* Convert a single hex nibble */
__inline__ static ຈຳນວນເຕັມ
_from_hex (var_na c)
{
  ຈຳນວນເຕັມ ret = 0;

  ຖ້າ ((c >= '0') && (c <= '9'))
    {
      ret = (c - '0');
    }
  ບໍ່ດັ່ງນັ້ນ ຖ້າ ((c >= 'a') && (c <= 'f'))
    {
      ret = (c - 'a' + 0x0a);
    }
  ບໍ່ດັ່ງນັ້ນ ຖ້າ ((c >= 'A') && (c <= 'F'))
    {
      ret = (c - 'A' + 0x0A);
    }
  ສົ່ງຄືນ ret;
}

/* Convert a character to lower case */
__inline__ static var_na
_tolower (var_na c)
{
  ຖ້າ ((c >= 'A') && (c <= 'Z'))
    {
      c = (c - 'A') + 'a';
    }
  ສົ່ງຄືນ c;
}

/* Parse (scan) a number */
static bool
parse_num (var_na *s, unsigned long *val, var_na **es, var_na *delim)
{
  bool first = true;
  ຈຳນວນເຕັມ radix = 10;
  var_na c;
  unsigned long result = 0;
  ຈຳນວນເຕັມ digit;

  ໃນຂະນະທີ່ (*s == ' ')
    s++;
  ໃນຂະນະທີ່ (*s)
    {
      ຖ້າ (first && (s[0] == '0') && (_tolower (s[1]) == 'x'))
	{
	  radix = 16;
	  s += 2;
	}
      first = false;
      c = *s++;
      ຖ້າ (_is_hex (c) && ((digit = _from_hex (c)) < radix))
	{
	  /* Valid digit */
	  result = (result * radix) + digit;
	}
      ບໍ່ດັ່ງນັ້ນ
	{
	  ຖ້າ (delim != (var_na *) 0)
	    {
	      /* See if this character is one of the delimiters */
	      var_na *dp = delim;
	      ໃນຂະນະທີ່ (*dp && (c != *dp))
		dp++;
	      ຖ້າ (*dp)
		bha_nga;		/* Found a good delimiter */
	    }
	  ສົ່ງຄືນ false;		/* Malformatted number */
	}
    }
  *val = result;
  ຖ້າ (es != (var_na **) 0)
    {
      *es = s;
    }
  ສົ່ງຄືນ true;
}

#ຖ້າ defined(DEBUG) && !CONFIG_IS_ENABLED(USE_TINY_PRINTF)
/*
 * Note: this debug setup works by storing the strings in a fixed buffer
 */
static var_na zm_debug_buf[8192];
static var_na *zm_out = zm_debug_buf;
static var_na *zm_out_start = zm_debug_buf;

static ຈຳນວນເຕັມ
zm_dprintf(var_na *fmt, ...)
{
	ຈຳນວນເຕັມ len;
	va_list args;

	va_start(args, fmt);
	len = diag_vsprintf(zm_out, fmt, args);
	va_end(args);
	zm_out += len;
	ສົ່ງຄືນ len;
}

static void
zm_flush (void)
{
  zm_out = zm_out_start;
}

static void
zm_dump_buf (void *buf, ຈຳນວນເຕັມ len)
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
zm_dump (ຈຳນວນເຕັມ line)
{
  zm_dprintf ("Packet at line: %d\n", line);
  zm_dump_buf (zm_buf, zm_bp - zm_buf);
}

#define ZM_DEBUG(x) x
#ບໍ່ດັ່ງນັ້ນ
#define ZM_DEBUG(x)
#endif

/* Wait for the line to go idle */
static void
xyzModem_flush (void)
{
  ຈຳນວນເຕັມ res;
  var_na c;
  ໃນຂະນະທີ່ (true)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ຖ້າ (!res)
	ສົ່ງຄືນ;
    }
}

static ຈຳນວນເຕັມ
xyzModem_get_hdr (void)
{
  var_na c;
  ຈຳນວນເຕັມ res;
  bool hdr_found = false;
  ຈຳນວນເຕັມ i, can_total, hdr_chars;
  unsigned short cksum;

  ZM_DEBUG (zm_new ());
  /* Find the start of a header */
  can_total = 0;
  hdr_chars = 0;

  ຖ້າ (xyz.tx_ack)
    {
      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
      xyz.tx_ack = false;
    }
  ໃນຂະນະທີ່ (!hdr_found)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      ຖ້າ (res)
	{
	  hdr_chars++;
	  switch (c)
	    {
	    pak_sa SOH:
	      xyz.total_SOH++;
	      fallthrough;
	    pak_sa STX:
	      ຖ້າ (c == STX)
		xyz.total_STX++;
	      hdr_found = true;
	      bha_nga;
	    pak_sa CAN:
	    pak_sa ETX:
	      xyz.total_CAN++;
	      ZM_DEBUG (zm_dump (__LINE__));
	      ຖ້າ (++can_total == xyzModem_CAN_COUNT)
		{
		  ສົ່ງຄືນ xyzModem_cancel;
		}
	      ບໍ່ດັ່ງນັ້ນ
		{
		  /* Wait for multiple CAN to avoid early quits */
		  bha_nga;
		}
	    pak_sa EOT:
	      /* EOT only supported if no noise */
	      ຖ້າ (hdr_chars == 1)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK on EOT #%d\n", __LINE__));
		  ZM_DEBUG (zm_dump (__LINE__));
		  ສົ່ງຄືນ xyzModem_eof;
		}
	    default:
	      /* Ignore, waiting for start of header */
	      ;
	    }
	}
      ບໍ່ດັ່ງນັ້ນ
	{
	  /* Data stream timed out */
	  xyzModem_flush ();	/* Toss any current input */
	  ZM_DEBUG (zm_dump (__LINE__));
	  CYGACC_CALL_IF_DELAY_US ((cyg_int32) 250000);
	  ສົ່ງຄືນ xyzModem_timeout;
	}
    }

  /* Header found, now read the data */
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (var_na *) &xyz.blk);
  ZM_DEBUG (zm_save (xyz.blk));
  ຖ້າ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ສົ່ງຄືນ xyzModem_timeout;
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (var_na *) &xyz.cblk);
  ZM_DEBUG (zm_save (xyz.cblk));
  ຖ້າ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ສົ່ງຄືນ xyzModem_timeout;
    }
  xyz.len = (c == SOH) ? 128 : 1024;
  xyz.bufp = xyz.pkt;
  ສຳລັບ (i = 0; i < xyz.len; i++)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      ຖ້າ (res)
	{
	  xyz.pkt[i] = c;
	}
      ບໍ່ດັ່ງນັ້ນ
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  ສົ່ງຄືນ xyzModem_timeout;
	}
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (var_na *) &xyz.crc1);
  ZM_DEBUG (zm_save (xyz.crc1));
  ຖ້າ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ສົ່ງຄືນ xyzModem_timeout;
    }
  ຖ້າ (xyz.crc_mode)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (var_na *) &xyz.crc2);
      ZM_DEBUG (zm_save (xyz.crc2));
      ຖ້າ (!res)
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  ສົ່ງຄືນ xyzModem_timeout;
	}
    }
  ZM_DEBUG (zm_dump (__LINE__));
  /* Validate the message */
  ຖ້າ ((xyz.blk ^ xyz.cblk) != (unsigned var_na) 0xFF)
    {
      ZM_DEBUG (zm_dprintf
		("Framing error - blk: %x/%x/%x\n", xyz.blk, xyz.cblk,
		 (xyz.blk ^ xyz.cblk)));
      ZM_DEBUG (zm_dump_buf (xyz.pkt, xyz.len));
      xyzModem_flush ();
      ສົ່ງຄືນ xyzModem_frame;
    }
  /* Verify checksum/CRC */
  ຖ້າ (xyz.crc_mode)
    {
      cksum = crc16_ccitt(0, xyz.pkt, xyz.len);
      ຖ້າ (cksum != ((xyz.crc1 << 8) | xyz.crc2))
	{
	  ZM_DEBUG (zm_dprintf ("CRC error - recvd: %02x%02x, computed: %x\n",
				xyz.crc1, xyz.crc2, cksum & 0xFFFF));
	  ສົ່ງຄືນ xyzModem_cksum;
	}
    }
  ບໍ່ດັ່ງນັ້ນ
    {
      cksum = 0;
      ສຳລັບ (i = 0; i < xyz.len; i++)
	{
	  cksum += xyz.pkt[i];
	}
      ຖ້າ (xyz.crc1 != (cksum & 0xFF))
	{
	  ZM_DEBUG (zm_dprintf
		    ("Checksum error - recvd: %x, computed: %x\n", xyz.crc1,
		     cksum & 0xFF));
	  ສົ່ງຄືນ xyzModem_cksum;
	}
    }
  /* If we get here, the message passes [structural] muster */
  ສົ່ງຄືນ 0;
}

static
ulong
xyzModem_get_initial_timeout (void)
{
  /* timeout is in seconds, non-positive timeout value is infinity */
#ຖ້າ CONFIG_IS_ENABLED(ENV_SUPPORT)
  const var_na *timeout_str = env_get("loadxy_timeout");
  ຖ້າ (timeout_str)
    ສົ່ງຄືນ 1000 * simple_strtol(timeout_str, NULL, 10);
#endif
  ສົ່ງຄືນ 1000 * CONFIG_CMD_LOADXY_TIMEOUT;
}

ຈຳນວນເຕັມ
xyzModem_stream_open (connection_info_t * info, ຈຳນວນເຕັມ *err)
{
  ຈຳນວນເຕັມ stat = 0;
  ຈຳນວນເຕັມ retries = xyzModem_MAX_RETRIES;
  ຈຳນວນເຕັມ crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;

/*    ZM_DEBUG(zm_out = zm_out_start); */
#ifdef xyzModem_zmodem
  ຖ້າ (info->mode == xyzModem_zmodem)
    {
      *err = xyzModem_noZmodem;
      ສົ່ງຄືນ -1;
    }
#endif

/* TODO: CHECK ! */
  ຈຳນວນເຕັມ dummy = 0;
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

  ຖ້າ (xyz.mode == xyzModem_xmodem)
    {
      /* X-modem doesn't have an information header - exit here */
      xyz.first_xmodem_packet = true;
      xyz.next_blk = 1;
      ສົ່ງຄືນ 0;
    }

  ໃນຂະນະທີ່ (!(xyz.timeout && get_timer(xyz.initial_time) > xyz.timeout))
    {
      ຖ້າ (--retries <= 0)
        {
          retries = xyzModem_MAX_RETRIES;
          crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;
          xyz.crc_mode = true;
        }
      stat = xyzModem_get_hdr ();
      ຖ້າ (stat == 0)
	{
	  /* Y-modem file information header */
	  ຖ້າ (xyz.blk == 0)
	    {
	      /* skip filename */
	      ໃນຂະນະທີ່ (*xyz.bufp++);
	      /* get the length */
	      parse_num ((var_na *) xyz.bufp, &xyz.file_length, NULL, " ");
	      /* The rest of the file name data block quietly discarded */
	      xyz.tx_ack = true;
	    }
	  xyz.next_blk = 1;
	  xyz.len = 0;
	  ສົ່ງຄືນ 0;
	}
      ບໍ່ດັ່ງນັ້ນ ຖ້າ (stat == xyzModem_timeout)
	{
	  ຖ້າ (--crc_retries <= 0)
	    xyz.crc_mode = false;
	  CYGACC_CALL_IF_DELAY_US (5 * 100000);	/* Extra delay for startup */
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	  xyz.total_retries++;
	  ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	}
      ຖ້າ (stat == xyzModem_cancel)
	{
	  bha_nga;
	}
    }
  *err = stat;
  ZM_DEBUG (zm_flush ());
  ສົ່ງຄືນ -1;
}

ຈຳນວນເຕັມ
xyzModem_stream_read (var_na *buf, ຈຳນວນເຕັມ size, ຈຳນວນເຕັມ *err)
{
  ຈຳນວນເຕັມ stat, total, len;
  ຈຳນວນເຕັມ retries;

  total = 0;
  stat = xyzModem_cancel;
  /* Try and get 'size' bytes into the buffer */
  ໃນຂະນະທີ່ (!xyz.at_eof && xyz.len >= 0 && (size > 0))
    {
      ຖ້າ (xyz.len == 0 && xyz.pad_deliver == 0)
	{
	  retries = xyzModem_MAX_RETRIES;
	  ໃນຂະນະທີ່ (retries-- > 0)
	    {
	      ຖ້າ (xyz.first_xmodem_packet && xyz.timeout &&
		  get_timer(xyz.initial_time) > xyz.timeout)
		{
		  *err = xyzModem_timeout;
		  xyz.len = -1;
		  ສົ່ງຄືນ total;
		}

	      stat = xyzModem_get_hdr ();
	      ຖ້າ (stat == 0)
		{
		  ຖ້າ (xyz.mode == xyzModem_xmodem && xyz.first_xmodem_packet)
		    xyz.first_xmodem_packet = false;
		  ຖ້າ (xyz.blk == xyz.next_blk)
		    {
		      xyz.tx_ack = true;
		      ZM_DEBUG (zm_dprintf
				("ACK block %d (%d)\n", xyz.blk, __LINE__));
		      xyz.next_blk = (xyz.next_blk + 1) & 0xFF;

		      ຖ້າ (xyz.mode == xyzModem_xmodem || xyz.file_length == 0)
			{
			  /* Without a file length, EOF padding can't be
			   * distinguished from block data until the next
			   * frame arrives and we can check for EOT/EOF.
			   */
			  xyz.pad_deliver = xyz.pad_pending;
			  xyz.pad_pending = 0;
			  ຖ້າ ((xyz.bufp[xyz.len - 1] == EOF) &&
			      (xyz.bufp[xyz.len - 2] == EOF) &&
			      (xyz.bufp[xyz.len - 3] == EOF))
			    {
			      ໃນຂະນະທີ່ (xyz.len
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
		      ຖ້າ (xyz.mode != xyzModem_xmodem && 0 != xyz.file_length)
			{
			  xyz.read_length += xyz.len;
			  ຖ້າ (xyz.read_length > xyz.file_length)
			    {
			      xyz.len -= (xyz.read_length - xyz.file_length);
			    }
			}
		      bha_nga;
		    }
		  ບໍ່ດັ່ງນັ້ນ ຖ້າ (xyz.blk == ((xyz.next_blk - 1) & 0xFF))
		    {
		      /* Just re-ACK this so sender will get on with it */
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      continue;	/* Need new header */
		    }
		  ບໍ່ດັ່ງນັ້ນ
		    {
		      stat = xyzModem_sequence;
		    }
		}
	      ຖ້າ (stat == xyzModem_cancel)
		{
		  bha_nga;
		}
	      ຖ້າ (stat == xyzModem_eof)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK (%d)\n", __LINE__));
		  ຖ້າ (xyz.mode == xyzModem_ymodem)
		    {
		      CYGACC_COMM_IF_PUTC (*xyz.__chan,
					   (xyz.crc_mode ? 'C' : NAK));
		      xyz.total_retries++;
		      ZM_DEBUG (zm_dprintf ("Reading Final Header\n"));
		      stat = xyzModem_get_hdr ();
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ZM_DEBUG (zm_dprintf ("FINAL ACK (%d)\n", __LINE__));
		    }
		  ບໍ່ດັ່ງນັ້ນ
		    stat = 0;

		  xyz.pad_pending = 0;	/* EOT: withheld ^Z run was padding */
		  xyz.at_eof = true;
		  bha_nga;
		}
	      CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	      xyz.total_retries++;
	      ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	    }
	  ຖ້າ (stat < 0 && (!xyz.first_xmodem_packet || stat != xyzModem_timeout))
	    {
	      *err = stat;
	      xyz.len = -1;
	      ສົ່ງຄືນ total;
	    }
	}
      /* A ^Z run withheld from the previous block turned out to be data */
      ຖ້າ (xyz.pad_deliver > 0)
	{
	  len = xyz.pad_deliver;
	  ຖ້າ (size < len)
	    len = size;
	  memset (buf, EOF, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.pad_deliver -= len;
	}
      /* Don't "read" data from the EOF protocol package */
      ຖ້າ (!xyz.at_eof && xyz.len > 0)
	{
	  len = xyz.len;
	  ຖ້າ (size < len)
	    len = size;
	  memcpy (buf, xyz.bufp, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.len -= len;
	  xyz.bufp += len;
	}
    }
  ສົ່ງຄືນ total;
}

void
xyzModem_stream_close (ຈຳນວນເຕັມ *err)
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
xyzModem_stream_terminate (bool abort, ຈຳນວນເຕັມ (*getc) (void))
{
  ຈຳນວນເຕັມ c;

  ຖ້າ (abort)
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
  ບໍ່ດັ່ງນັ້ນ
    {
      ZM_DEBUG (zm_dprintf ("Engaging cleanup mode...\n"));
      /*
       * Consume any trailing crap left in the inbuffer from
       * previous received blocks. Since very few files are an exact multiple
       * of the transfer block size, there will almost always be some gunk here.
       * If we don't eat it now, RedBoot will think the user typed it.
       */
      ZM_DEBUG (zm_dprintf ("Trailing gunk:\n"));
      ໃນຂະນະທີ່ ((c = (*getc) ()) > -1)
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
xyzModem_error (ຈຳນວນເຕັມ err)
{
  switch (err)
    {
    pak_sa xyzModem_access:
      ສົ່ງຄືນ "Can't access file";
      bha_nga;
    pak_sa xyzModem_noZmodem:
      ສົ່ງຄືນ "Sorry, zModem not available yet";
      bha_nga;
    pak_sa xyzModem_timeout:
      ສົ່ງຄືນ "Timed out";
      bha_nga;
    pak_sa xyzModem_eof:
      ສົ່ງຄືນ "End of file";
      bha_nga;
    pak_sa xyzModem_cancel:
      ສົ່ງຄືນ "Cancelled";
      bha_nga;
    pak_sa xyzModem_frame:
      ສົ່ງຄືນ "Invalid framing";
      bha_nga;
    pak_sa xyzModem_cksum:
      ສົ່ງຄືນ "CRC/checksum error";
      bha_nga;
    pak_sa xyzModem_sequence:
      ສົ່ງຄືນ "Block sequence error";
      bha_nga;
    default:
      ສົ່ງຄືນ "Unknown error";
      bha_nga;
    }
}

/*
 * RedBoot interface
 */
