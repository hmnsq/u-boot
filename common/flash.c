// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define DEBUG */

#অন্তর্ভাব <flash.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <u-boot/uuid.h>
#অন্তর্ভাব <linux/string.h>

#অন্তর্ভাব <mtd/cfi_flash.h>

/*-----------------------------------------------------------------------
 * Functions
 */

/*-----------------------------------------------------------------------
 * Set protection status for monitor sectors
 *
 * The monitor is always located in the _first_ Flash bank.
 * If necessary you have to map the second bank at lower addresses.
 */
শূন্য
flash_protect(পূর্ণাঙ্ক flag, ulong from, ulong to, flash_info_t *info)
{
	ulong b_end;
	short s_end;
	পূর্ণাঙ্ক i;

	/* Do nothing if input data is bad. */
	যদি (!info || info->sector_গুনতি == 0 || info->size == 0 || to < from) {
		প্রত্যাগম;
	}

	s_end = info->sector_গুনতি - 1;	/* index of last sector */
	b_end = info->start[0] + info->size - 1;	/* bank end address */

	debug("%s %s: from 0x%08lX to 0x%08lX\n", __func__,
	      (flag & FLAG_PROTECT_SET) ? "ON" :
	      (flag & FLAG_PROTECT_CLEAR) ? "OFF" : "???",
	      from, to);

	/* There is nothing to do if we have no data about the flash
	 * or the protect range and flash range don't overlap.
	 */
	যদি (info->flash_id == FLASH_UNKNOWN ||
	    to < info->start[0] || from > b_end) {
		প্রত্যাগম;
	}

	প্রতি (i=0; i<info->sector_গুনতি; ++i) {
		ulong end;		/* last address in current sect	*/

		end = (i == s_end) ? b_end : info->start[i + 1] - 1;

		/* Update protection if any part of the sector
		 * is in the specified range.
		 */
		যদি (from <= end && to >= info->start[i]) {
			যদি (flag & FLAG_PROTECT_CLEAR) {
#যদি defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 0);
#অন্যথা
				info->protect[i] = 0;
#endif	/* CONFIG_SYS_FLASH_PROTECTION */
				debug("protect off %d\n", i);
			}
			অন্যথা যদি (flag & FLAG_PROTECT_SET) {
#যদি defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 1);
#অন্যথা
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
	পূর্ণাঙ্ক i;

	প্রতি (i = 0, info = &flash_info[0]; i < CFI_FLASH_BANKS; ++i, ++info) {
		যদি (info->flash_id != FLASH_UNKNOWN &&
		    addr >= info->start[0] &&
		    /* WARNING - The '- 1' is needed if the flash
		     * is at the end of the address space, since
		     * info->start[0] + info->size wraps back to 0.
		     * Please don't change this unless you understand this.
		     */
		    addr <= info->start[0] + info->size - 1) {
			প্রত্যাগম (info);
		}
	}

	প্রত্যাগম (রিক্ত);
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
পূর্ণাঙ্ক
flash_write(বর্ণ *src, ulong addr, ulong cnt)
{
	পূর্ণাঙ্ক i;
	ulong         end        = addr + cnt - 1;
	flash_info_t *info_first = addr2info(addr);
	flash_info_t *info_last  = addr2info(end);
	flash_info_t *info;
	__maybe_unused বর্ণ *src_orig = src;
	__maybe_unused বর্ণ *addr_orig = (বর্ণ *)addr;
	__maybe_unused ulong cnt_orig = cnt;

	যদি (cnt == 0) {
		প্রত্যাগম (FL_ERR_OK);
	}

	যদি (!info_first || !info_last) {
		প্রত্যাগম (FL_ERR_INVAL);
	}

	প্রতি (info = info_first; info <= info_last; ++info) {
		ulong b_end = info->start[0] + info->size;	/* bank end addr */
		short s_end = info->sector_গুনতি - 1;
		প্রতি (i=0; i<info->sector_গুনতি; ++i) {
			ulong e_addr = (i == s_end) ? b_end : info->start[i + 1];

			যদি ((end >= info->start[i]) && (addr < e_addr) &&
			    (info->protect[i] != 0) ) {
				প্রত্যাগম (FL_ERR_PROTECTED);
			}
		}
	}

	/* finally write data to flash */
	প্রতি (info = info_first; info <= info_last && cnt>0; ++info) {
		ulong len;

		len = info->start[0] + info->size - addr;
		যদি (len > cnt)
			len = cnt;
		যদি ((i = write_buff(info, (uchar *)src, addr, len)) != 0) {
			প্রত্যাগম (i);
		}
		cnt  -= len;
		addr += len;
		src  += len;
	}

#যদি defined(CONFIG_FLASH_VERIFY)
	যদি (memcmp(src_orig, addr_orig, cnt_orig)) {
		printf("\nVerify failed!\n");
		প্রত্যাগম FL_ERR_PROG_ERROR;
	}
#endif /* CONFIG_SYS_FLASH_VERIFY_AFTER_WRITE */

	প্রত্যাগম (FL_ERR_OK);
}

/*-----------------------------------------------------------------------
 */

শূন্য flash_perror(পূর্ণাঙ্ক err)
{
	বিকল্প (err) {
	পক্ষ FL_ERR_OK:
		ভঙ্গ;
	পক্ষ FL_ERR_TIMEOUT:
		puts ("Timeout writing to Flash\n");
		ভঙ্গ;
	পক্ষ FL_ERR_NOT_ERASED:
		puts ("Flash not Erased\n");
		ভঙ্গ;
	পক্ষ FL_ERR_PROTECTED:
		puts ("Can't write to protected Flash sectors\n");
		ভঙ্গ;
	পক্ষ FL_ERR_INVAL:
		puts ("Outside available Flash\n");
		ভঙ্গ;
	পক্ষ FL_ERR_ALIGN:
		puts ("Start and/or end address not on sector boundary\n");
		ভঙ্গ;
	পক্ষ FL_ERR_UNKNOWN_FLASH_VENDOR:
		puts ("Unknown Vendor of Flash\n");
		ভঙ্গ;
	পক্ষ FL_ERR_UNKNOWN_FLASH_TYPE:
		puts ("Unknown Type of Flash\n");
		ভঙ্গ;
	পক্ষ FL_ERR_PROG_ERROR:
		puts ("General Flash Programming Error\n");
		ভঙ্গ;
	পক্ষ FL_ERR_ABORTED:
		puts("Flash Programming Aborted\n");
		ভঙ্গ;
	স্বভাব:
		printf ("%s[%d] FIXME: rc=%d\n", __FILE__, __LINE__, err);
		ভঙ্গ;
	}
}
