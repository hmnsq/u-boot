// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define DEBUG */

#समावेश <flash.h>
#समावेश <log.h>
#समावेश <u-boot/uuid.h>
#समावेश <linux/string.h>

#समावेश <mtd/cfi_flash.h>

/*-----------------------------------------------------------------------
 * Functions
 */

/*-----------------------------------------------------------------------
 * Set protection status for monitor sectors
 *
 * The monitor is always located in the _first_ Flash bank.
 * If necessary you have to map the second bank at lower addresses.
 */
व्योम
flash_protect(पूर्णांक flag, ulong from, ulong to, flash_info_t *info)
{
	ulong b_end;
	लघु s_end;
	पूर्णांक i;

	/* Do nothing if input data is bad. */
	अगर (!info || info->sector_गिनती == 0 || info->आकार == 0 || to < from) {
		वापस;
	}

	s_end = info->sector_गिनती - 1;	/* index of last sector */
	b_end = info->start[0] + info->आकार - 1;	/* bank end address */

	debug("%s %s: from 0x%08lX to 0x%08lX\n", __func__,
	      (flag & FLAG_PROTECT_SET) ? "ON" :
	      (flag & FLAG_PROTECT_CLEAR) ? "OFF" : "???",
	      from, to);

	/* There is nothing to do if we have no data about the flash
	 * or the protect range and flash range don't overlap.
	 */
	अगर (info->flash_id == FLASH_UNKNOWN ||
	    to < info->start[0] || from > b_end) {
		वापस;
	}

	क्रम (i=0; i<info->sector_गिनती; ++i) {
		ulong end;		/* last address in current sect	*/

		end = (i == s_end) ? b_end : info->start[i + 1] - 1;

		/* Update protection if any part of the sector
		 * is in the specified range.
		 */
		अगर (from <= end && to >= info->start[i]) {
			अगर (flag & FLAG_PROTECT_CLEAR) {
#अगर defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 0);
#अन्यथा
				info->protect[i] = 0;
#पूर्ण_अगर	/* CONFIG_SYS_FLASH_PROTECTION */
				debug("protect off %d\n", i);
			}
			अन्यथा अगर (flag & FLAG_PROTECT_SET) {
#अगर defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 1);
#अन्यथा
				info->protect[i] = 1;
#पूर्ण_अगर	/* CONFIG_SYS_FLASH_PROTECTION */
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
	पूर्णांक i;

	क्रम (i = 0, info = &flash_info[0]; i < CFI_FLASH_BANKS; ++i, ++info) {
		अगर (info->flash_id != FLASH_UNKNOWN &&
		    addr >= info->start[0] &&
		    /* WARNING - The '- 1' is needed if the flash
		     * is at the end of the address space, since
		     * info->start[0] + info->size wraps back to 0.
		     * Please don't change this unless you understand this.
		     */
		    addr <= info->start[0] + info->आकार - 1) {
			वापस (info);
		}
	}

	वापस (शून्य);
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
पूर्णांक
flash_write(अक्षर *src, ulong addr, ulong cnt)
{
	पूर्णांक i;
	ulong         end        = addr + cnt - 1;
	flash_info_t *info_first = addr2info(addr);
	flash_info_t *info_last  = addr2info(end);
	flash_info_t *info;
	__maybe_unused अक्षर *src_orig = src;
	__maybe_unused अक्षर *addr_orig = (अक्षर *)addr;
	__maybe_unused ulong cnt_orig = cnt;

	अगर (cnt == 0) {
		वापस (FL_ERR_OK);
	}

	अगर (!info_first || !info_last) {
		वापस (FL_ERR_INVAL);
	}

	क्रम (info = info_first; info <= info_last; ++info) {
		ulong b_end = info->start[0] + info->आकार;	/* bank end addr */
		लघु s_end = info->sector_गिनती - 1;
		क्रम (i=0; i<info->sector_गिनती; ++i) {
			ulong e_addr = (i == s_end) ? b_end : info->start[i + 1];

			अगर ((end >= info->start[i]) && (addr < e_addr) &&
			    (info->protect[i] != 0) ) {
				वापस (FL_ERR_PROTECTED);
			}
		}
	}

	/* finally write data to flash */
	क्रम (info = info_first; info <= info_last && cnt>0; ++info) {
		ulong len;

		len = info->start[0] + info->आकार - addr;
		अगर (len > cnt)
			len = cnt;
		अगर ((i = write_buff(info, (uchar *)src, addr, len)) != 0) {
			वापस (i);
		}
		cnt  -= len;
		addr += len;
		src  += len;
	}

#अगर defined(CONFIG_FLASH_VERIFY)
	अगर (स_भेद(src_orig, addr_orig, cnt_orig)) {
		म_लिखो("\nVerify failed!\n");
		वापस FL_ERR_PROG_ERROR;
	}
#पूर्ण_अगर /* CONFIG_SYS_FLASH_VERIFY_AFTER_WRITE */

	वापस (FL_ERR_OK);
}

/*-----------------------------------------------------------------------
 */

व्योम flash_perror(पूर्णांक err)
{
	चयन (err) {
	हाल FL_ERR_OK:
		अवरोध;
	हाल FL_ERR_TIMEOUT:
		माला_दो ("Timeout writing to Flash\n");
		अवरोध;
	हाल FL_ERR_NOT_ERASED:
		माला_दो ("Flash not Erased\n");
		अवरोध;
	हाल FL_ERR_PROTECTED:
		माला_दो ("Can't write to protected Flash sectors\n");
		अवरोध;
	हाल FL_ERR_INVAL:
		माला_दो ("Outside available Flash\n");
		अवरोध;
	हाल FL_ERR_ALIGN:
		माला_दो ("Start and/or end address not on sector boundary\n");
		अवरोध;
	हाल FL_ERR_UNKNOWN_FLASH_VENDOR:
		माला_दो ("Unknown Vendor of Flash\n");
		अवरोध;
	हाल FL_ERR_UNKNOWN_FLASH_TYPE:
		माला_दो ("Unknown Type of Flash\n");
		अवरोध;
	हाल FL_ERR_PROG_ERROR:
		माला_दो ("General Flash Programming Error\n");
		अवरोध;
	हाल FL_ERR_ABORTED:
		माला_दो("Flash Programming Aborted\n");
		अवरोध;
	शेष:
		म_लिखो ("%s[%d] FIXME: rc=%d\n", __खाता__, __LINE__, err);
		अवरोध;
	}
}
