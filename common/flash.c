// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define DEBUG */

#அந்தர்பாவ <flash.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <u-boot/uuid.h>
#அந்தர்பாவ <linux/string.h>

#அந்தர்பாவ <mtd/cfi_flash.h>

/*-----------------------------------------------------------------------
 * Functions
 */

/*-----------------------------------------------------------------------
 * Set protection status for monitor sectors
 *
 * The monitor is always located in the _first_ Flash bank.
 * If necessary you have to map the second bank at lower addresses.
 */
வெறுமை
flash_protect(முழு flag, ulong from, ulong to, flash_info_t *info)
{
	ulong b_end;
	short s_end;
	முழு i;

	/* Do nothing if input data is bad. */
	எனில் (!info || info->sector_மொத்தம் == 0 || info->size == 0 || to < from) {
		திரும்பு;
	}

	s_end = info->sector_மொத்தம் - 1;	/* index of last sector */
	b_end = info->start[0] + info->size - 1;	/* bank end address */

	debug("%s %s: from 0x%08lX to 0x%08lX\n", __func__,
	      (flag & FLAG_PROTECT_SET) ? "ON" :
	      (flag & FLAG_PROTECT_CLEAR) ? "OFF" : "???",
	      from, to);

	/* There is nothing to do if we have no data about the flash
	 * or the protect range and flash range don't overlap.
	 */
	எனில் (info->flash_id == FLASH_UNKNOWN ||
	    to < info->start[0] || from > b_end) {
		திரும்பு;
	}

	க்கு (i=0; i<info->sector_மொத்தம்; ++i) {
		ulong end;		/* last address in current sect	*/

		end = (i == s_end) ? b_end : info->start[i + 1] - 1;

		/* Update protection if any part of the sector
		 * is in the specified range.
		 */
		எனில் (from <= end && to >= info->start[i]) {
			எனில் (flag & FLAG_PROTECT_CLEAR) {
#எனில் defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 0);
#இல்லையெனில்
				info->protect[i] = 0;
#endif	/* CONFIG_SYS_FLASH_PROTECTION */
				debug("protect off %d\n", i);
			}
			இல்லையெனில் எனில் (flag & FLAG_PROTECT_SET) {
#எனில் defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 1);
#இல்லையெனில்
				info->protect[i] = 1;
#endif	/* CONFIG_SYS_FLASH_PROTECTION */
				debug("protect on %d\n", i);
			}
		}
	}
}

/*-----------------------------------------------------------------------
 */

flash_info_t *
addr2info(ulong addr)
{
	flash_info_t *info;
	முழு i;

	க்கு (i = 0, info = &flash_info[0]; i < CFI_FLASH_BANKS; ++i, ++info) {
		எனில் (info->flash_id != FLASH_UNKNOWN &&
		    addr >= info->start[0] &&
		    /* WARNING - The '- 1' is needed if the flash
		     * is at the end of the address space, since
		     * info->start[0] + info->size wraps back to 0.
		     * Please don't change this unless you understand this.
		     */
		    addr <= info->start[0] + info->size - 1) {
			திரும்பு (info);
		}
	}

	திரும்பு (வெற்று);
}

/*-----------------------------------------------------------------------
 * Copy memory to flash.
 * Make sure all target addresses are within Flash bounds,
 * and no protected sectors are hit.
 * Returns:
 * FL_ERR_OK          0 - OK
 * FL_ERR_TIMEOUT     1 - write timeout
 * FL_ERR_NOT_ERASED  2 - Flash not erased
 * FL_ERR_PROTECTED   4 - target range includes protected sectors
 * FL_ERR_INVAL       8 - target address not in Flash memory
 * FL_ERR_ALIGN       16 - target address not aligned on boundary
 *			   (only some targets require alignment)
 */
முழு
flash_write(எழுத்து *src, ulong addr, ulong cnt)
{
	முழு i;
	ulong         end        = addr + cnt - 1;
	flash_info_t *info_first = addr2info(addr);
	flash_info_t *info_last  = addr2info(end);
	flash_info_t *info;
	__maybe_unused எழுத்து *src_orig = src;
	__maybe_unused எழுத்து *addr_orig = (எழுத்து *)addr;
	__maybe_unused ulong cnt_orig = cnt;

	எனில் (cnt == 0) {
		திரும்பு (FL_ERR_OK);
	}

	எனில் (!info_first || !info_last) {
		திரும்பு (FL_ERR_INVAL);
	}

	க்கு (info = info_first; info <= info_last; ++info) {
		ulong b_end = info->start[0] + info->size;	/* bank end addr */
		short s_end = info->sector_மொத்தம் - 1;
		க்கு (i=0; i<info->sector_மொத்தம்; ++i) {
			ulong e_addr = (i == s_end) ? b_end : info->start[i + 1];

			எனில் ((end >= info->start[i]) && (addr < e_addr) &&
			    (info->protect[i] != 0) ) {
				திரும்பு (FL_ERR_PROTECTED);
			}
		}
	}

	/* finally write data to flash */
	க்கு (info = info_first; info <= info_last && cnt>0; ++info) {
		ulong len;

		len = info->start[0] + info->size - addr;
		எனில் (len > cnt)
			len = cnt;
		எனில் ((i = write_buff(info, (uchar *)src, addr, len)) != 0) {
			திரும்பு (i);
		}
		cnt  -= len;
		addr += len;
		src  += len;
	}

#எனில் defined(CONFIG_FLASH_VERIFY)
	எனில் (memcmp(src_orig, addr_orig, cnt_orig)) {
		printf("\nVerify failed!\n");
		திரும்பு FL_ERR_PROG_ERROR;
	}
#endif /* CONFIG_SYS_FLASH_VERIFY_AFTER_WRITE */

	திரும்பு (FL_ERR_OK);
}

/*-----------------------------------------------------------------------
 */

வெறுமை flash_perror(முழு err)
{
	தேர்வு (err) {
	நிலை FL_ERR_OK:
		நிறுத்து;
	நிலை FL_ERR_TIMEOUT:
		puts ("Timeout writing to Flash\n");
		நிறுத்து;
	நிலை FL_ERR_NOT_ERASED:
		puts ("Flash not Erased\n");
		நிறுத்து;
	நிலை FL_ERR_PROTECTED:
		puts ("Can't write to protected Flash sectors\n");
		நிறுத்து;
	நிலை FL_ERR_INVAL:
		puts ("Outside available Flash\n");
		நிறுத்து;
	நிலை FL_ERR_ALIGN:
		puts ("Start and/or end address not on sector boundary\n");
		நிறுத்து;
	நிலை FL_ERR_UNKNOWN_FLASH_VENDOR:
		puts ("Unknown Vendor of Flash\n");
		நிறுத்து;
	நிலை FL_ERR_UNKNOWN_FLASH_TYPE:
		puts ("Unknown Type of Flash\n");
		நிறுத்து;
	நிலை FL_ERR_PROG_ERROR:
		puts ("General Flash Programming Error\n");
		நிறுத்து;
	நிலை FL_ERR_ABORTED:
		puts("Flash Programming Aborted\n");
		நிறுத்து;
	இயல்பு:
		printf ("%s[%d] FIXME: rc=%d\n", __FILE__, __LINE__, err);
		நிறுத்து;
	}
}
