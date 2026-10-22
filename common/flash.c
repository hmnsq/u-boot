// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define DEBUG */

#include <flash.h>
#include <log.h>
#include <u-boot/uuid.h>
#include <linux/string.h>

#include <mtd/cfi_flash.h>

/*-----------------------------------------------------------------------
 * Functions
 */

/*-----------------------------------------------------------------------
 * Set protection status for monitor sectors
 *
 * The monitor is always located in the _first_ Flash bank.
 * If necessary you have to map the second bank at lower addresses.
 */
void
flash_protect(පූර්ණ_සංඛ්‍යා flag, ulong from, ulong to, flash_info_t *info)
{
	ulong b_end;
	short s_end;
	පූර්ණ_සංඛ්‍යා i;

	/* Do nothing if input data is bad. */
	නම් (!info || info->sector_count == 0 || info->size == 0 || to < from) {
		ආපසු;
	}

	s_end = info->sector_count - 1;	/* index of last sector */
	b_end = info->start[0] + info->size - 1;	/* bank end address */

	debug("%s %s: from 0x%08lX to 0x%08lX\n", __func__,
	      (flag & FLAG_PROTECT_SET) ? "ON" :
	      (flag & FLAG_PROTECT_CLEAR) ? "OFF" : "???",
	      from, to);

	/* There is nothing to do if we have no data about the flash
	 * or the protect range and flash range don't overlap.
	 */
	නම් (info->flash_id == FLASH_UNKNOWN ||
	    to < info->start[0] || from > b_end) {
		ආපසු;
	}

	සඳහා (i=0; i<info->sector_count; ++i) {
		ulong end;		/* last address in current sect	*/

		end = (i == s_end) ? b_end : info->start[i + 1] - 1;

		/* Update protection if any part of the sector
		 * is in the specified range.
		 */
		නම් (from <= end && to >= info->start[i]) {
			නම් (flag & FLAG_PROTECT_CLEAR) {
#නම් defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 0);
#නැතහොත්
				info->protect[i] = 0;
#endif	/* CONFIG_SYS_FLASH_PROTECTION */
				debug("protect off %d\n", i);
			}
			නැතහොත් නම් (flag & FLAG_PROTECT_SET) {
#නම් defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 1);
#නැතහොත්
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
	පූර්ණ_සංඛ්‍යා i;

	සඳහා (i = 0, info = &flash_info[0]; i < CFI_FLASH_BANKS; ++i, ++info) {
		නම් (info->flash_id != FLASH_UNKNOWN &&
		    addr >= info->start[0] &&
		    /* WARNING - The '- 1' is needed if the flash
		     * is at the end of the address space, since
		     * info->start[0] + info->size wraps back to 0.
		     * Please don't change this unless you understand this.
		     */
		    addr <= info->start[0] + info->size - 1) {
			ආපසු (info);
		}
	}

	ආපසු (NULL);
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
පූර්ණ_සංඛ්‍යා
flash_write(var_na *src, ulong addr, ulong cnt)
{
	පූර්ණ_සංඛ්‍යා i;
	ulong         end        = addr + cnt - 1;
	flash_info_t *info_first = addr2info(addr);
	flash_info_t *info_last  = addr2info(end);
	flash_info_t *info;
	__maybe_unused var_na *src_orig = src;
	__maybe_unused var_na *addr_orig = (var_na *)addr;
	__maybe_unused ulong cnt_orig = cnt;

	නම් (cnt == 0) {
		ආපසු (FL_ERR_OK);
	}

	නම් (!info_first || !info_last) {
		ආපසු (FL_ERR_INVAL);
	}

	සඳහා (info = info_first; info <= info_last; ++info) {
		ulong b_end = info->start[0] + info->size;	/* bank end addr */
		short s_end = info->sector_count - 1;
		සඳහා (i=0; i<info->sector_count; ++i) {
			ulong e_addr = (i == s_end) ? b_end : info->start[i + 1];

			නම් ((end >= info->start[i]) && (addr < e_addr) &&
			    (info->protect[i] != 0) ) {
				ආපසු (FL_ERR_PROTECTED);
			}
		}
	}

	/* finally write data to flash */
	සඳහා (info = info_first; info <= info_last && cnt>0; ++info) {
		ulong len;

		len = info->start[0] + info->size - addr;
		නම් (len > cnt)
			len = cnt;
		නම් ((i = write_buff(info, (uchar *)src, addr, len)) != 0) {
			ආපසු (i);
		}
		cnt  -= len;
		addr += len;
		src  += len;
	}

#නම් defined(CONFIG_FLASH_VERIFY)
	නම් (memcmp(src_orig, addr_orig, cnt_orig)) {
		printf("\nVerify failed!\n");
		ආපසු FL_ERR_PROG_ERROR;
	}
#endif /* CONFIG_SYS_FLASH_VERIFY_AFTER_WRITE */

	ආපසු (FL_ERR_OK);
}

/*-----------------------------------------------------------------------
 */

void flash_perror(පූර්ණ_සංඛ්‍යා err)
{
	switch (err) {
	pak_sa FL_ERR_OK:
		bha_nga;
	pak_sa FL_ERR_TIMEOUT:
		puts ("Timeout writing to Flash\n");
		bha_nga;
	pak_sa FL_ERR_NOT_ERASED:
		puts ("Flash not Erased\n");
		bha_nga;
	pak_sa FL_ERR_PROTECTED:
		puts ("Can't write to protected Flash sectors\n");
		bha_nga;
	pak_sa FL_ERR_INVAL:
		puts ("Outside available Flash\n");
		bha_nga;
	pak_sa FL_ERR_ALIGN:
		puts ("Start and/or end address not on sector boundary\n");
		bha_nga;
	pak_sa FL_ERR_UNKNOWN_FLASH_VENDOR:
		puts ("Unknown Vendor of Flash\n");
		bha_nga;
	pak_sa FL_ERR_UNKNOWN_FLASH_TYPE:
		puts ("Unknown Type of Flash\n");
		bha_nga;
	pak_sa FL_ERR_PROG_ERROR:
		puts ("General Flash Programming Error\n");
		bha_nga;
	pak_sa FL_ERR_ABORTED:
		puts("Flash Programming Aborted\n");
		bha_nga;
	default:
		printf ("%s[%d] FIXME: rc=%d\n", __FILE__, __LINE__, err);
		bha_nga;
	}
}
