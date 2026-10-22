// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define DEBUG */

#شامل <flash.h>
#شامل <log.h>
#شامل <u-boot/uuid.h>
#شامل <linux/string.h>

#شامل <mtd/cfi_flash.h>

/*-----------------------------------------------------------------------
 * Functions
 */

/*-----------------------------------------------------------------------
 * Set protection status for monitor sectors
 *
 * The monitor is always located in the _first_ Flash bank.
 * If necessary you have to map the second bank at lower addresses.
 */
خالی
flash_protect(صحیح flag, ulong from, ulong مقصد, flash_info_t *info)
{
	ulong b_انتہا;
	مختصر s_انتہا;
	صحیح i;

	/* Do nothing if input data is bad. */
	اگر (!info || info->sector_گنتی == 0 || info->سائز == 0 || مقصد < from) {
		واپس;
	}

	s_انتہا = info->sector_گنتی - 1;	/* index of last sector */
	b_انتہا = info->آغاز[0] + info->سائز - 1;	/* bank end address */

	debug("%s %s: from 0x%08lX to 0x%08lX\n", __func__,
	      (flag & FLAG_PROTECT_سیٹ) ? "ON" :
	      (flag & FLAG_PROTECT_CLEAR) ? "OFF" : "???",
	      from, مقصد);

	/* There is nothing to do if we have no data about the flash
	 * or the protect range and flash range don't overlap.
	 */
	اگر (info->flash_id == FLASH_UNKNOWN ||
	    مقصد < info->آغاز[0] || from > b_انتہا) {
		واپس;
	}

	برائے (i=0; i<info->sector_گنتی; ++i) {
		ulong انتہا_2;		/* last address in current sect	*/

		انتہا_2 = (i == s_انتہا) ? b_انتہا : info->آغاز[i + 1] - 1;

		/* Update protection if any part of the sector
		 * is in the specified range.
		 */
		اگر (from <= انتہا_2 && مقصد >= info->آغاز[i]) {
			اگر (flag & FLAG_PROTECT_CLEAR) {
#اگر defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 0);
#ورنہ
				info->protect[i] = 0;
#اگر_ختم	/* CONFIG_SYS_FLASH_PROTECTION */
				debug("protect off %d\n", i);
			}
			ورنہ اگر (flag & FLAG_PROTECT_سیٹ) {
#اگر defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 1);
#ورنہ
				info->protect[i] = 1;
#اگر_ختم	/* CONFIG_SYS_FLASH_PROTECTION */
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
	صحیح i;

	برائے (i = 0, info = &flash_info[0]; i < CFI_FLASH_BANKS; ++i, ++info) {
		اگر (info->flash_id != FLASH_UNKNOWN &&
		    addr >= info->آغاز[0] &&
		    /* WARNING - The '- 1' is needed if the flash
		     * is at the end of the address space, since
		     * info->start[0] + info->size wraps back to 0.
		     * Please don't change this unless you understand this.
		     */
		    addr <= info->آغاز[0] + info->سائز - 1) {
			واپس (info);
		}
	}

	واپس (تہی);
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
صحیح
flash_write(حرف *src, ulong addr, ulong cnt)
{
	صحیح i;
	ulong         انتہا_2        = addr + cnt - 1;
	flash_info_t *info_پہلا = addr2info(addr);
	flash_info_t *info_آخری  = addr2info(انتہا_2);
	flash_info_t *info;
	__maybe_unused حرف *src_orig = src;
	__maybe_unused حرف *addr_orig = (حرف *)addr;
	__maybe_unused ulong cnt_orig = cnt;

	اگر (cnt == 0) {
		واپس (FL_ERR_OK);
	}

	اگر (!info_پہلا || !info_آخری) {
		واپس (FL_ERR_INVAL);
	}

	برائے (info = info_پہلا; info <= info_آخری; ++info) {
		ulong b_انتہا = info->آغاز[0] + info->سائز;	/* bank end addr */
		مختصر s_انتہا = info->sector_گنتی - 1;
		برائے (i=0; i<info->sector_گنتی; ++i) {
			ulong e_addr = (i == s_انتہا) ? b_انتہا : info->آغاز[i + 1];

			اگر ((انتہا_2 >= info->آغاز[i]) && (addr < e_addr) &&
			    (info->protect[i] != 0) ) {
				واپس (FL_ERR_PROTECTED);
			}
		}
	}

	/* finally write data to flash */
	برائے (info = info_پہلا; info <= info_آخری && cnt>0; ++info) {
		ulong len;

		len = info->آغاز[0] + info->سائز - addr;
		اگر (len > cnt)
			len = cnt;
		اگر ((i = write_buff(info, (uchar *)src, addr, len)) != 0) {
			واپس (i);
		}
		cnt  -= len;
		addr += len;
		src  += len;
	}

#اگر defined(CONFIG_FLASH_VERIFY)
	اگر (memcmp(src_orig, addr_orig, cnt_orig)) {
		لکھو("\nVerify failed!\n");
		واپس FL_ERR_PROG_ERROR;
	}
#اگر_ختم /* CONFIG_SYS_FLASH_VERIFY_AFTER_WRITE */

	واپس (FL_ERR_OK);
}

/*-----------------------------------------------------------------------
 */

خالی flash_perror(صحیح err)
{
	انتخاب (err) {
	حالت FL_ERR_OK:
		توڑو;
	حالت FL_ERR_TIMEOUT:
		puts ("Timeout writing to Flash\n");
		توڑو;
	حالت FL_ERR_NOT_ERASED:
		puts ("Flash not Erased\n");
		توڑو;
	حالت FL_ERR_PROTECTED:
		puts ("Can't write to protected Flash sectors\n");
		توڑو;
	حالت FL_ERR_INVAL:
		puts ("Outside available Flash\n");
		توڑو;
	حالت FL_ERR_ALIGN:
		puts ("Start and/or end address not on sector boundary\n");
		توڑو;
	حالت FL_ERR_UNKNOWN_FLASH_VENDOR:
		puts ("Unknown Vendor of Flash\n");
		توڑو;
	حالت FL_ERR_UNKNOWN_FLASH_TYPE:
		puts ("Unknown Type of Flash\n");
		توڑو;
	حالت FL_ERR_PROG_ERROR:
		puts ("General Flash Programming Error\n");
		توڑو;
	حالت FL_ERR_ABORTED:
		puts("Flash Programming Aborted\n");
		توڑو;
	طےشدہ:
		لکھو ("%s[%d] FIXME: rc=%d\n", __FILE__, __LINE__, err);
		توڑو;
	}
}
