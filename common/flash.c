// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define DEBUG */

#अन्तर्भाव <flash.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <u-boot/uuid.h>
#अन्तर्भाव <linux/string.h>

#अन्तर्भाव <mtd/cfi_flash.h>

/*-----------------------------------------------------------------------
 * Functions
 */

/*-----------------------------------------------------------------------
 * Set protection status for monitor sectors
 *
 * The monitor is always located in the _first_ Flash bank.
 * If necessary you have to map the second bank at lower addresses.
 */
शून्य
flash_protect(पूर्णांक flag, ulong from, ulong to, flash_info_t *info)
{
	ulong b_end;
	लघु s_end;
	पूर्णांक i;

	/* Do nothing if input data is bad. */
	जर (!info || info->sector_मोजणी == 0 || info->size == 0 || to < from) {
		परत;
	}

	s_end = info->sector_मोजणी - 1;	/* index of last sector */
	b_end = info->start[0] + info->size - 1;	/* bank end address */

	debug("%s %s: from 0x%08lX to 0x%08lX\n", __func__,
	      (flag & FLAG_PROTECT_SET) ? "ON" :
	      (flag & FLAG_PROTECT_CLEAR) ? "OFF" : "???",
	      from, to);

	/* There is nothing to do if we have no data about the flash
	 * or the protect range and flash range don't overlap.
	 */
	जर (info->flash_id == FLASH_UNKNOWN ||
	    to < info->start[0] || from > b_end) {
		परत;
	}

	साठी (i=0; i<info->sector_मोजणी; ++i) {
		ulong end;		/* last address in current sect	*/

		end = (i == s_end) ? b_end : info->start[i + 1] - 1;

		/* Update protection if any part of the sector
		 * is in the specified range.
		 */
		जर (from <= end && to >= info->start[i]) {
			जर (flag & FLAG_PROTECT_CLEAR) {
#जर defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 0);
#अन्यथा
				info->protect[i] = 0;
#endif	/* CONFIG_SYS_FLASH_PROTECTION */
				debug("protect off %d\n", i);
			}
			अन्यथा जर (flag & FLAG_PROTECT_SET) {
#जर defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 1);
#अन्यथा
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
	पूर्णांक i;

	साठी (i = 0, info = &flash_info[0]; i < CFI_FLASH_BANKS; ++i, ++info) {
		जर (info->flash_id != FLASH_UNKNOWN &&
		    addr >= info->start[0] &&
		    /* WARNING - The '- 1' is needed if the flash
		     * is at the end of the address space, since
		     * info->start[0] + info->size wraps back to 0.
		     * Please don't change this unless you understand this.
		     */
		    addr <= info->start[0] + info->size - 1) {
			परत (info);
		}
	}

	परत (रिक्त);
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

	जर (cnt == 0) {
		परत (FL_ERR_OK);
	}

	जर (!info_first || !info_last) {
		परत (FL_ERR_INVAL);
	}

	साठी (info = info_first; info <= info_last; ++info) {
		ulong b_end = info->start[0] + info->size;	/* bank end addr */
		लघु s_end = info->sector_मोजणी - 1;
		साठी (i=0; i<info->sector_मोजणी; ++i) {
			ulong e_addr = (i == s_end) ? b_end : info->start[i + 1];

			जर ((end >= info->start[i]) && (addr < e_addr) &&
			    (info->protect[i] != 0) ) {
				परत (FL_ERR_PROTECTED);
			}
		}
	}

	/* finally write data to flash */
	साठी (info = info_first; info <= info_last && cnt>0; ++info) {
		ulong len;

		len = info->start[0] + info->size - addr;
		जर (len > cnt)
			len = cnt;
		जर ((i = write_buff(info, (uchar *)src, addr, len)) != 0) {
			परत (i);
		}
		cnt  -= len;
		addr += len;
		src  += len;
	}

#जर defined(CONFIG_FLASH_VERIFY)
	जर (स_भेद(src_orig, addr_orig, cnt_orig)) {
		म_लिखो("\nVerify failed!\n");
		परत FL_ERR_PROG_ERROR;
	}
#endif /* CONFIG_SYS_FLASH_VERIFY_AFTER_WRITE */

	परत (FL_ERR_OK);
}

/*-----------------------------------------------------------------------
 */

शून्य flash_perror(पूर्णांक err)
{
	निवड (err) {
	स्थिती FL_ERR_OK:
		थांबा;
	स्थिती FL_ERR_TIMEOUT:
		माला_दो ("Timeout writing to Flash\n");
		थांबा;
	स्थिती FL_ERR_NOT_ERASED:
		माला_दो ("Flash not Erased\n");
		थांबा;
	स्थिती FL_ERR_PROTECTED:
		माला_दो ("Can't write to protected Flash sectors\n");
		थांबा;
	स्थिती FL_ERR_INVAL:
		माला_दो ("Outside available Flash\n");
		थांबा;
	स्थिती FL_ERR_ALIGN:
		माला_दो ("Start and/or end address not on sector boundary\n");
		थांबा;
	स्थिती FL_ERR_UNKNOWN_FLASH_VENDOR:
		माला_दो ("Unknown Vendor of Flash\n");
		थांबा;
	स्थिती FL_ERR_UNKNOWN_FLASH_TYPE:
		माला_दो ("Unknown Type of Flash\n");
		थांबा;
	स्थिती FL_ERR_PROG_ERROR:
		माला_दो ("General Flash Programming Error\n");
		थांबा;
	स्थिती FL_ERR_ABORTED:
		माला_दो("Flash Programming Aborted\n");
		थांबा;
	पूर्वनिर्धारित:
		म_लिखो ("%s[%d] FIXME: rc=%d\n", __FILE__, __LINE__, err);
		थांबा;
	}
}
