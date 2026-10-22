// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define DEBUG */

#ଅନ୍ତର୍ଭାଵ <flash.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <u-boot/uuid.h>
#ଅନ୍ତର୍ଭାଵ <linux/string.h>

#ଅନ୍ତର୍ଭାଵ <mtd/cfi_flash.h>

/*-----------------------------------------------------------------------
 * Functions
 */

/*-----------------------------------------------------------------------
 * Set protection status for monitor sectors
 *
 * The monitor is always located in the _first_ Flash bank.
 * If necessary you have to map the second bank at lower addresses.
 */
ଶୂନ୍ୟ
flash_protect(ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag, ulong from, ulong to, flash_info_t *info)
{
	ulong b_end;
	short s_end;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	/* Do nothing if input data is bad. */
	ଯଦି (!info || info->sector_ଗଣନା == 0 || info->size == 0 || to < from) {
		ଫେରନ୍ତୁ;
	}

	s_end = info->sector_ଗଣନା - 1;	/* index of last sector */
	b_end = info->start[0] + info->size - 1;	/* bank end address */

	debug("%s %s: from 0x%08lX to 0x%08lX\n", __func__,
	      (flag & FLAG_PROTECT_SET) ? "ON" :
	      (flag & FLAG_PROTECT_CLEAR) ? "OFF" : "???",
	      from, to);

	/* There is nothing to do if we have no data about the flash
	 * or the protect range and flash range don't overlap.
	 */
	ଯଦି (info->flash_id == FLASH_UNKNOWN ||
	    to < info->start[0] || from > b_end) {
		ଫେରନ୍ତୁ;
	}

	ପାଇଁ (i=0; i<info->sector_ଗଣନା; ++i) {
		ulong end;		/* last address in current sect	*/

		end = (i == s_end) ? b_end : info->start[i + 1] - 1;

		/* Update protection if any part of the sector
		 * is in the specified range.
		 */
		ଯଦି (from <= end && to >= info->start[i]) {
			ଯଦି (flag & FLAG_PROTECT_CLEAR) {
#ଯଦି defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 0);
#ଅନ୍ୟଥା
				info->protect[i] = 0;
#endif	/* CONFIG_SYS_FLASH_PROTECTION */
				debug("protect off %d\n", i);
			}
			ଅନ୍ୟଥା ଯଦି (flag & FLAG_PROTECT_SET) {
#ଯଦି defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 1);
#ଅନ୍ୟଥା
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
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପାଇଁ (i = 0, info = &flash_info[0]; i < CFI_FLASH_BANKS; ++i, ++info) {
		ଯଦି (info->flash_id != FLASH_UNKNOWN &&
		    addr >= info->start[0] &&
		    /* WARNING - The '- 1' is needed if the flash
		     * is at the end of the address space, since
		     * info->start[0] + info->size wraps back to 0.
		     * Please don't change this unless you understand this.
		     */
		    addr <= info->start[0] + info->size - 1) {
			ଫେରନ୍ତୁ (info);
		}
	}

	ଫେରନ୍ତୁ (ଖାଲି);
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
flash_write(ଅକ୍ଷର *src, ulong addr, ulong cnt)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ulong         end        = addr + cnt - 1;
	flash_info_t *info_first = addr2info(addr);
	flash_info_t *info_last  = addr2info(end);
	flash_info_t *info;
	__maybe_unused ଅକ୍ଷର *src_orig = src;
	__maybe_unused ଅକ୍ଷର *addr_orig = (ଅକ୍ଷର *)addr;
	__maybe_unused ulong cnt_orig = cnt;

	ଯଦି (cnt == 0) {
		ଫେରନ୍ତୁ (FL_ERR_OK);
	}

	ଯଦି (!info_first || !info_last) {
		ଫେରନ୍ତୁ (FL_ERR_INVAL);
	}

	ପାଇଁ (info = info_first; info <= info_last; ++info) {
		ulong b_end = info->start[0] + info->size;	/* bank end addr */
		short s_end = info->sector_ଗଣନା - 1;
		ପାଇଁ (i=0; i<info->sector_ଗଣନା; ++i) {
			ulong e_addr = (i == s_end) ? b_end : info->start[i + 1];

			ଯଦି ((end >= info->start[i]) && (addr < e_addr) &&
			    (info->protect[i] != 0) ) {
				ଫେରନ୍ତୁ (FL_ERR_PROTECTED);
			}
		}
	}

	/* finally write data to flash */
	ପାଇଁ (info = info_first; info <= info_last && cnt>0; ++info) {
		ulong len;

		len = info->start[0] + info->size - addr;
		ଯଦି (len > cnt)
			len = cnt;
		ଯଦି ((i = write_buff(info, (uchar *)src, addr, len)) != 0) {
			ଫେରନ୍ତୁ (i);
		}
		cnt  -= len;
		addr += len;
		src  += len;
	}

#ଯଦି defined(CONFIG_FLASH_VERIFY)
	ଯଦି (memcmp(src_orig, addr_orig, cnt_orig)) {
		printf("\nVerify failed!\n");
		ଫେରନ୍ତୁ FL_ERR_PROG_ERROR;
	}
#endif /* CONFIG_SYS_FLASH_VERIFY_AFTER_WRITE */

	ଫେରନ୍ତୁ (FL_ERR_OK);
}

/*-----------------------------------------------------------------------
 */

ଶୂନ୍ୟ flash_perror(ପୂର୍ଣ୍ଣସଂଖ୍ୟା err)
{
	ବାଛ (err) {
	ସ୍ଥିତି FL_ERR_OK:
		ଭାଙ୍ଗ;
	ସ୍ଥିତି FL_ERR_TIMEOUT:
		puts ("Timeout writing to Flash\n");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି FL_ERR_NOT_ERASED:
		puts ("Flash not Erased\n");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି FL_ERR_PROTECTED:
		puts ("Can't write to protected Flash sectors\n");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି FL_ERR_INVAL:
		puts ("Outside available Flash\n");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି FL_ERR_ALIGN:
		puts ("Start and/or end address not on sector boundary\n");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି FL_ERR_UNKNOWN_FLASH_VENDOR:
		puts ("Unknown Vendor of Flash\n");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି FL_ERR_UNKNOWN_FLASH_TYPE:
		puts ("Unknown Type of Flash\n");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି FL_ERR_PROG_ERROR:
		puts ("General Flash Programming Error\n");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି FL_ERR_ABORTED:
		puts("Flash Programming Aborted\n");
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		printf ("%s[%d] FIXME: rc=%d\n", __FILE__, __LINE__, err);
		ଭାଙ୍ଗ;
	}
}
