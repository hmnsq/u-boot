// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define DEBUG */

#ضمن <flash.h>
#ضمن <log.h>
#ضمن <u-boot/uuid.h>
#ضمن <linux/string.h>

#ضمن <mtd/cfi_flash.h>

/*-----------------------------------------------------------------------
 * Functions
 */

/*-----------------------------------------------------------------------
 * Set protection status for monitor sectors
 *
 * The monitor is always located in the _first_ Flash bank.
 * If necessary you have to map the second bank at lower addresses.
 */
فراغ
flash_protect(صحيح flag, ulong from, ulong منزل, flash_info_t *info)
{
	ulong b_پای_ټکی;
	لنډ s_پای_ټکی;
	صحيح i;

	/* Do nothing if input data is bad. */
	إذا (!info || info->sector_شمېر == 0 || info->اندازه == 0 || منزل < from) {
		أرجع;
	}

	s_پای_ټکی = info->sector_شمېر - 1;	/* index of last sector */
	b_پای_ټکی = info->پیل[0] + info->اندازه - 1;	/* bank end address */

	debug("%s %s: from 0x%08lX to 0x%08lX\n", __func__,
	      (flag & FLAG_PROTECT_سېټ) ? "ON" :
	      (flag & FLAG_PROTECT_CLEAR) ? "OFF" : "???",
	      from, منزل);

	/* There is nothing to do if we have no data about the flash
	 * or the protect range and flash range don't overlap.
	 */
	إذا (info->flash_id == FLASH_UNKNOWN ||
	    منزل < info->پیل[0] || from > b_پای_ټکی) {
		أرجع;
	}

	لكل (i=0; i<info->sector_شمېر; ++i) {
		ulong پای_ټکی_2;		/* last address in current sect	*/

		پای_ټکی_2 = (i == s_پای_ټکی) ? b_پای_ټکی : info->پیل[i + 1] - 1;

		/* Update protection if any part of the sector
		 * is in the specified range.
		 */
		إذا (from <= پای_ټکی_2 && منزل >= info->پیل[i]) {
			إذا (flag & FLAG_PROTECT_CLEAR) {
#إذا defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 0);
#وإلا
				info->protect[i] = 0;
#د_که_پای	/* CONFIG_SYS_FLASH_PROTECTION */
				debug("protect off %d\n", i);
			}
			وإلا إذا (flag & FLAG_PROTECT_سېټ) {
#إذا defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 1);
#وإلا
				info->protect[i] = 1;
#د_که_پای	/* CONFIG_SYS_FLASH_PROTECTION */
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
	صحيح i;

	لكل (i = 0, info = &flash_info[0]; i < CFI_FLASH_BANKS; ++i, ++info) {
		إذا (info->flash_id != FLASH_UNKNOWN &&
		    addr >= info->پیل[0] &&
		    /* WARNING - The '- 1' is needed if the flash
		     * is at the end of the address space, since
		     * info->start[0] + info->size wraps back to 0.
		     * Please don't change this unless you understand this.
		     */
		    addr <= info->پیل[0] + info->اندازه - 1) {
			أرجع (info);
		}
	}

	أرجع (عدم);
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
صحيح
flash_write(حرف *src, ulong addr, ulong cnt)
{
	صحيح i;
	ulong         پای_ټکی_2        = addr + cnt - 1;
	flash_info_t *info_لومړی = addr2info(addr);
	flash_info_t *info_وروستی  = addr2info(پای_ټکی_2);
	flash_info_t *info;
	__maybe_unused حرف *src_orig = src;
	__maybe_unused حرف *addr_orig = (حرف *)addr;
	__maybe_unused ulong cnt_orig = cnt;

	إذا (cnt == 0) {
		أرجع (FL_ERR_OK);
	}

	إذا (!info_لومړی || !info_وروستی) {
		أرجع (FL_ERR_INVAL);
	}

	لكل (info = info_لومړی; info <= info_وروستی; ++info) {
		ulong b_پای_ټکی = info->پیل[0] + info->اندازه;	/* bank end addr */
		لنډ s_پای_ټکی = info->sector_شمېر - 1;
		لكل (i=0; i<info->sector_شمېر; ++i) {
			ulong e_addr = (i == s_پای_ټکی) ? b_پای_ټکی : info->پیل[i + 1];

			إذا ((پای_ټکی_2 >= info->پیل[i]) && (addr < e_addr) &&
			    (info->protect[i] != 0) ) {
				أرجع (FL_ERR_PROTECTED);
			}
		}
	}

	/* finally write data to flash */
	لكل (info = info_لومړی; info <= info_وروستی && cnt>0; ++info) {
		ulong len;

		len = info->پیل[0] + info->اندازه - addr;
		إذا (len > cnt)
			len = cnt;
		إذا ((i = write_buff(info, (uchar *)src, addr, len)) != 0) {
			أرجع (i);
		}
		cnt  -= len;
		addr += len;
		src  += len;
	}

#إذا defined(CONFIG_FLASH_VERIFY)
	إذا (memcmp(src_orig, addr_orig, cnt_orig)) {
		چاپ_کړه("\nVerify failed!\n");
		أرجع FL_ERR_PROG_ERROR;
	}
#د_که_پای /* CONFIG_SYS_FLASH_VERIFY_AFTER_WRITE */

	أرجع (FL_ERR_OK);
}

/*-----------------------------------------------------------------------
 */

فراغ flash_perror(صحيح err)
{
	بدل (err) {
	حالة FL_ERR_OK:
		اكسر;
	حالة FL_ERR_TIMEOUT:
		puts ("Timeout writing to Flash\n");
		اكسر;
	حالة FL_ERR_NOT_ERASED:
		puts ("Flash not Erased\n");
		اكسر;
	حالة FL_ERR_PROTECTED:
		puts ("Can't write to protected Flash sectors\n");
		اكسر;
	حالة FL_ERR_INVAL:
		puts ("Outside available Flash\n");
		اكسر;
	حالة FL_ERR_ALIGN:
		puts ("Start and/or end address not on sector boundary\n");
		اكسر;
	حالة FL_ERR_UNKNOWN_FLASH_VENDOR:
		puts ("Unknown Vendor of Flash\n");
		اكسر;
	حالة FL_ERR_UNKNOWN_FLASH_TYPE:
		puts ("Unknown Type of Flash\n");
		اكسر;
	حالة FL_ERR_PROG_ERROR:
		puts ("General Flash Programming Error\n");
		اكسر;
	حالة FL_ERR_ABORTED:
		puts("Flash Programming Aborted\n");
		اكسر;
	افتراضي:
		چاپ_کړه ("%s[%d] FIXME: rc=%d\n", __FILE__, __LINE__, err);
		اكسر;
	}
}
