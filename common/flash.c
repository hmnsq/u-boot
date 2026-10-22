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
flash_protect(পূর্ণসংখ্যা flag, ulong from, ulong to, flash_info_t *info)
{
	ulong b_end;
	short s_end;
	পূর্ণসংখ্যা i;

	/* Do nothing if input data is bad. */
	যদি (!info || info->sector_গুনতি == 0 || info->আকার_2 == 0 || to < from) {
		ফেরত;
	}

	s_end = info->sector_গুনতি - 1;	/* index of last sector */
	b_end = info->start[0] + info->আকার_2 - 1;	/* bank end address */

	debug("%s %s: from 0x%08lX to 0x%08lX\n", __func__,
	      (flag & FLAG_PROTECT_SET) ? "ON" :
	      (flag & FLAG_PROTECT_CLEAR) ? "OFF" : "???",
	      from, to);

	/* There is nothing to do if we have no data about the flash
	 * or the protect range and flash range don't overlap.
	 */
	যদি (info->flash_id == FLASH_UNKNOWN ||
	    to < info->start[0] || from > b_end) {
		ফেরত;
	}

	জন্য (i=0; i<info->sector_গুনতি; ++i) {
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
	পূর্ণসংখ্যা i;

	জন্য (i = 0, info = &flash_info[0]; i < CFI_FLASH_BANKS; ++i, ++info) {
		যদি (info->flash_id != FLASH_UNKNOWN &&
		    addr >= info->start[0] &&
		    /* WARNING - The '- 1' is needed if the flash
		     * is at the end of the address space, since
		     * info->start[0] + info->size wraps back to 0.
		     * Please don't change this unless you understand this.
		     */
		    addr <= info->start[0] + info->আকার_2 - 1) {
			ফেরত (info);
		}
	}

	ফেরত (ফাঁকা);
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
পূর্ণসংখ্যা
flash_write(অক্ষর *src, ulong addr, ulong cnt)
{
	পূর্ণসংখ্যা i;
	ulong         end        = addr + cnt - 1;
	flash_info_t *info_first = addr2info(addr);
	flash_info_t *info_last  = addr2info(end);
	flash_info_t *info;
	__maybe_unused অক্ষর *src_orig = src;
	__maybe_unused অক্ষর *addr_orig = (অক্ষর *)addr;
	__maybe_unused ulong cnt_orig = cnt;

	যদি (cnt == 0) {
		ফেরত (FL_ERR_OK);
	}

	যদি (!info_first || !info_last) {
		ফেরত (FL_ERR_INVAL);
	}

	জন্য (info = info_first; info <= info_last; ++info) {
		ulong b_end = info->start[0] + info->আকার_2;	/* bank end addr */
		short s_end = info->sector_গুনতি - 1;
		জন্য (i=0; i<info->sector_গুনতি; ++i) {
			ulong e_addr = (i == s_end) ? b_end : info->start[i + 1];

			যদি ((end >= info->start[i]) && (addr < e_addr) &&
			    (info->protect[i] != 0) ) {
				ফেরত (FL_ERR_PROTECTED);
			}
		}
	}

	/* finally write data to flash */
	জন্য (info = info_first; info <= info_last && cnt>0; ++info) {
		ulong len;

		len = info->start[0] + info->আকার_2 - addr;
		যদি (len > cnt)
			len = cnt;
		যদি ((i = write_buff(info, (uchar *)src, addr, len)) != 0) {
			ফেরত (i);
		}
		cnt  -= len;
		addr += len;
		src  += len;
	}

#যদি defined(CONFIG_FLASH_VERIFY)
	যদি (memcmp(src_orig, addr_orig, cnt_orig)) {
		printf("\nVerify failed!\n");
		ফেরত FL_ERR_PROG_ERROR;
	}
#endif /* CONFIG_SYS_FLASH_VERIFY_AFTER_WRITE */

	ফেরত (FL_ERR_OK);
}

/*-----------------------------------------------------------------------
 */

শূন্য flash_perror(পূর্ণসংখ্যা err)
{
	নির্বাচন (err) {
	ক্ষেত্র FL_ERR_OK:
		বিরতি;
	ক্ষেত্র FL_ERR_TIMEOUT:
		puts ("Timeout writing to Flash\n");
		বিরতি;
	ক্ষেত্র FL_ERR_NOT_ERASED:
		puts ("Flash not Erased\n");
		বিরতি;
	ক্ষেত্র FL_ERR_PROTECTED:
		puts ("Can't write to protected Flash sectors\n");
		বিরতি;
	ক্ষেত্র FL_ERR_INVAL:
		puts ("Outside available Flash\n");
		বিরতি;
	ক্ষেত্র FL_ERR_ALIGN:
		puts ("Start and/or end address not on sector boundary\n");
		বিরতি;
	ক্ষেত্র FL_ERR_UNKNOWN_FLASH_VENDOR:
		puts ("Unknown Vendor of Flash\n");
		বিরতি;
	ক্ষেত্র FL_ERR_UNKNOWN_FLASH_TYPE:
		puts ("Unknown Type of Flash\n");
		বিরতি;
	ক্ষেত্র FL_ERR_PROG_ERROR:
		puts ("General Flash Programming Error\n");
		বিরতি;
	ক্ষেত্র FL_ERR_ABORTED:
		puts("Flash Programming Aborted\n");
		বিরতি;
	পূর্বনির্ধারিত:
		printf ("%s[%d] FIXME: rc=%d\n", __FILE__, __LINE__, err);
		বিরতি;
	}
}
