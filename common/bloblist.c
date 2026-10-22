// SPDX-License-Identifier: GPL-2.0+ BSD-3-Clause
/*
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_BLOBLIST

#അന്തര്ഭാവ <bloblist.h>
#അന്തര്ഭാവ <display_options.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <malloc.h>
#അന്തര്ഭാവ <mapmem.h>
#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <tables_csum.h>
#അന്തര്ഭാവ <asm/global_data.h>
#അന്തര്ഭാവ <u-boot/crc.h>

/*
 * A bloblist is a single contiguous chunk of memory with a header
 * (struct bloblist_hdr) and a number of blobs in it.
 *
 * Each blob starts on a BLOBLIST_ALIGN boundary relative to the start of the
 * bloblist and consists of a struct bloblist_rec, some padding to the required
 * alignment for the blog and then the actual data. The padding ensures that the
 * start address of the data in each blob is aligned as required. Note that
 * each blob's *data* is aligned to BLOBLIST_ALIGN regardless of the alignment
 * of the bloblist itself or the blob header.
 */

DECLARE_GLOBAL_DATA_PTR;

സ്ഥിതം ഘടന tag_name {
	എണ്ണൽ bloblist_tag_t tag;
	സ്ഥിരം അക്ഷരം *name;
} tag_name[] = {
	{ BLOBLISTT_VOID, "(void)" },

	/* BLOBLISTT_AREA_FIRMWARE_TOP */
	{ BLOBLISTT_CONTROL_FDT, "Control FDT" },
	{ BLOBLISTT_HOB_BLOCK, "HOB block" },
	{ BLOBLISTT_HOB_LIST, "HOB list" },
	{ BLOBLISTT_ACPI_TABLES, "ACPI tables for x86" },
	{ BLOBLISTT_TPM_EVLOG, "TPM event log defined by TCG EFI" },
	{ BLOBLISTT_TPM_CRB_BASE, "TPM Command Response Buffer address" },
	{ BLOBLISTT_FDT_OVERLAY, "DT overlay" },

	/* BLOBLISTT_AREA_FIRMWARE */
	{ BLOBLISTT_TPM2_TCG_LOG, "TPM v2 log space" },
	{ BLOBLISTT_TCPA_LOG, "TPM log space" },
	{ BLOBLISTT_ACPI_GNVS, "ACPI GNVS" },

	/* BLOBLISTT_AREA_TF */
	{ BLOBLISTT_OPTEE_PAGABLE_PART, "OP-TEE pagable part" },

	/* BLOBLISTT_AREA_OTHER */
	{ BLOBLISTT_INTEL_VBT, "Intel Video-BIOS table" },
	{ BLOBLISTT_SMBIOS_TABLES, "SMBIOS tables for x86" },
	{ BLOBLISTT_VBOOT_CTX, "Chrome OS vboot context" },

	/* BLOBLISTT_PROJECT_AREA */
	{ BLOBLISTT_U_BOOT_SPL_HANDOFF, "SPL hand-off" },
	{ BLOBLISTT_VBE, "VBE" },
	{ BLOBLISTT_U_BOOT_VIDEO, "SPL video handoff" },

	/* BLOBLISTT_VENDOR_AREA */
};

സ്ഥിരം അക്ഷരം *bloblist_tag_name(എണ്ണൽ bloblist_tag_t tag)
{
	പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = 0; i < ARRAY_SIZE(tag_name); i++) {
		എങ്കിൽ (tag_name[i].tag == tag)
			തിരികെ tag_name[i].name;
	}

	തിരികെ "invalid";
}

സ്ഥിതം ഘടന bloblist_rec *bloblist_first_blob(ഘടന bloblist_hdr *hdr)
{
	എങ്കിൽ (hdr->used_size <= hdr->hdr_size)
		തിരികെ ശൂന്യ;
	തിരികെ (ഘടന bloblist_rec *)((ശൂന്യം *)hdr + hdr->hdr_size);
}

സ്ഥിതം വരിയിൽ uint rec_hdr_size(ഘടന bloblist_rec *rec)
{
	തിരികെ (rec->tag_and_hdr_size & BLOBLISTR_HDR_SIZE_MASK) >>
		BLOBLISTR_HDR_SIZE_SHIFT;
}

സ്ഥിതം വരിയിൽ uint rec_tag(ഘടന bloblist_rec *rec)
{
	തിരികെ (rec->tag_and_hdr_size & BLOBLISTR_TAG_MASK) >>
		BLOBLISTR_TAG_SHIFT;
}

സ്ഥിതം വരിയിൽ ശൂന്യം void_blob(ഘടന bloblist_rec *rec)
{
	എങ്കിൽ (rec_tag(rec) == BLOBLISTT_VOID)
		തിരികെ;
	rec->tag_and_hdr_size = BLOBLISTT_VOID |
				വലുപ്പം(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
}

സ്ഥിതം വരിയിൽ ഘടന bloblist_rec *rec_from_blob(ശൂന്യം *blob)
{
	തിരികെ (blob - വലുപ്പം(ഘടന bloblist_rec));
}

സ്ഥിതം ulong bloblist_blob_end_ofs(ഘടന bloblist_hdr *hdr,
				   ഘടന bloblist_rec *rec)
{
	ulong offset;

	offset = (ശൂന്യം *)rec - (ശൂന്യം *)hdr;
	/*
	 * The data section of next TE should start from an address aligned
	 * to 1 << hdr->align_log2.
	 */
	offset += rec_hdr_size(rec) + rec->size;
	offset = round_up(offset + rec_hdr_size(rec), 1 << hdr->align_log2);
	offset -= rec_hdr_size(rec);

	തിരികെ offset;
}

സ്ഥിതം ഘടന bloblist_rec *bloblist_next_blob(ഘടന bloblist_hdr *hdr,
					       ഘടന bloblist_rec *rec)
{
	ulong offset = bloblist_blob_end_ofs(hdr, rec);

	എങ്കിൽ (offset >= hdr->used_size)
		തിരികെ ശൂന്യ;
	തിരികെ (ഘടന bloblist_rec *)((ശൂന്യം *)hdr + offset);
}

#define foreach_rec(_rec, _hdr) \
	വേണ്ടി (_rec = bloblist_first_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_next_blob(_hdr, _rec))

സ്ഥിതം ഘടന bloblist_rec *bloblist_findrec(uint tag)
{
	ഘടന bloblist_hdr *hdr = gd->bloblist;
	ഘടന bloblist_rec *rec;

	എങ്കിൽ (!hdr)
		തിരികെ ശൂന്യ;

	foreach_rec(rec, hdr) {
		എങ്കിൽ (rec_tag(rec) == tag)
			തിരികെ rec;
	}

	തിരികെ ശൂന്യ;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ bloblist_addrec(uint tag, പൂർണ്ണസംഖ്യ size, പൂർണ്ണസംഖ്യ align_log2,
			   ഘടന bloblist_rec **recp)
{
	ഘടന bloblist_hdr *hdr = gd->bloblist;
	ഘടന bloblist_rec *rec;
	phys_addr_t data_start, aligned_start;
	phys_size_t new_alloced;

	എങ്കിൽ (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	data_start = map_to_sysmem(hdr) + hdr->used_size + വലുപ്പം(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_start = ALIGN(data_start, 1U << align_log2) - data_start;

	/* If we need to create a dummy record, create it */
	എങ്കിൽ (aligned_start) {
		പൂർണ്ണസംഖ്യ void_size = aligned_start - വലുപ്പം(*rec);
		ഘടന bloblist_rec *vrec;
		പൂർണ്ണസംഖ്യ ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_size, 0, &vrec);
		എങ്കിൽ (ret)
			തിരികെ log_സന്ദേശം_ret("void", ret);

		/* start the record after that */
		data_start = map_to_sysmem(hdr) + hdr->used_size + വലുപ്പം(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = data_start - map_to_sysmem(hdr) +
		ALIGN(size, 1U << align_log2);

	എങ്കിൽ (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		തിരികെ log_സന്ദേശം_ret("bloblist add", -ENOSPC);
	}
	rec = (ശൂന്യം *)hdr + hdr->used_size;

	rec->tag_and_hdr_size = tag | വലുപ്പം(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
	rec->size = size;

	/* Zero the record data */
	memset((ശൂന്യം *)rec + rec_hdr_size(rec), '\0', rec->size);

	hdr->used_size = new_alloced;
	*recp = rec;

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ bloblist_ensurerec(uint tag, ഘടന bloblist_rec **recp, പൂർണ്ണസംഖ്യ size,
			      പൂർണ്ണസംഖ്യ align_log2)
{
	ഘടന bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	എങ്കിൽ (rec) {
		എങ്കിൽ (size && size != rec->size) {
			*recp = rec;
			തിരികെ -ESPIPE;
		}
	} അല്ലെങ്കിൽ {
		പൂർണ്ണസംഖ്യ ret;

		ret = bloblist_addrec(tag, size, align_log2, &rec);
		എങ്കിൽ (ret)
			തിരികെ ret;
	}
	*recp = rec;

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ bloblist_get_blob_data_offset(uint tag)
{
	തിരഞ്ഞെടുപ്പ് (tag) {
	അവസ്ഥ BLOBLISTT_FDT_OVERLAY:
		തിരികെ വലുപ്പം(ഘടന dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	തിരികെ 0;
}

ശൂന്യം *bloblist_find(uint tag, പൂർണ്ണസംഖ്യ size)
{
	ശൂന്യം *blob = ശൂന്യ;
	പൂർണ്ണസംഖ്യ blob_size;

	blob = bloblist_get_blob(tag, &blob_size);

	എങ്കിൽ (size && size != blob_size)
		തിരികെ ശൂന്യ;

	തിരികെ blob;
}

ശൂന്യം *bloblist_get_blob(uint tag, പൂർണ്ണസംഖ്യ *sizep)
{
	ഘടന bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	എങ്കിൽ (!rec)
		തിരികെ ശൂന്യ;

	*sizep = rec->size;

	തിരികെ (ശൂന്യം *)rec + rec_hdr_size(rec);
}

പൂർണ്ണസംഖ്യ bloblist_apply_blobs(uint tag, പൂർണ്ണസംഖ്യ (*func)(ശൂന്യം **data, പൂർണ്ണസംഖ്യ size))
{
	ഘടന bloblist_hdr *hdr = gd->bloblist;
	ഘടന bloblist_rec *rec;

	എങ്കിൽ (!func || !hdr)
		തിരികെ -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		എങ്കിൽ (rec_tag(rec) == tag) {
			പൂർണ്ണസംഖ്യ ret;
			പൂർണ്ണസംഖ്യ tag = rec_tag(rec);
			ശൂന്യം *blob = (ശൂന്യം *)rec + rec_hdr_size(rec);
			പൂർണ്ണസംഖ്യ dat_off = bloblist_get_blob_data_offset(tag);

			blob += dat_off;
			ret = func(&blob, rec->size - dat_off);
			എങ്കിൽ (ret) {
				log_err("Failed to apply blob with tag %d\n",
					tag);
				തിരികെ ret;
			}

			rec = rec_from_blob(blob - dat_off);
			എങ്കിൽ (!rec) {
				log_err("Blob corrupted\n");
				തിരികെ -ENOENT;
			}

			/* Mark applied blob record as void */
			void_blob(rec);
		}
	}

	തിരികെ 0;
}

ശൂന്യം *bloblist_add(uint tag, പൂർണ്ണസംഖ്യ size, പൂർണ്ണസംഖ്യ align_log2)
{
	ഘടന bloblist_rec *rec;

	എങ്കിൽ (bloblist_addrec(tag, size, align_log2, &rec))
		തിരികെ ശൂന്യ;

	തിരികെ (ശൂന്യം *)rec + rec_hdr_size(rec);
}

പൂർണ്ണസംഖ്യ bloblist_ensure_size(uint tag, പൂർണ്ണസംഖ്യ size, പൂർണ്ണസംഖ്യ align_log2, ശൂന്യം **blobp)
{
	ഘടന bloblist_rec *rec;
	പൂർണ്ണസംഖ്യ ret;

	ret = bloblist_ensurerec(tag, &rec, size, align_log2);
	എങ്കിൽ (ret)
		തിരികെ ret;
	*blobp = (ശൂന്യം *)rec + rec_hdr_size(rec);

	തിരികെ 0;
}

ശൂന്യം *bloblist_ensure(uint tag, പൂർണ്ണസംഖ്യ size)
{
	ഘടന bloblist_rec *rec;

	എങ്കിൽ (bloblist_ensurerec(tag, &rec, size, 0))
		തിരികെ ശൂന്യ;

	തിരികെ (ശൂന്യം *)rec + rec_hdr_size(rec);
}

പൂർണ്ണസംഖ്യ bloblist_ensure_size_ret(uint tag, പൂർണ്ണസംഖ്യ *sizep, ശൂന്യം **blobp)
{
	ഘടന bloblist_rec *rec;
	പൂർണ്ണസംഖ്യ ret;

	ret = bloblist_ensurerec(tag, &rec, *sizep, 0);
	എങ്കിൽ (ret == -ESPIPE)
		*sizep = rec->size;
	അല്ലെങ്കിൽ എങ്കിൽ (ret)
		തിരികെ ret;
	*blobp = (ശൂന്യം *)rec + rec_hdr_size(rec);

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ bloblist_resize_rec(ഘടന bloblist_hdr *hdr,
			       ഘടന bloblist_rec *rec,
			       പൂർണ്ണസംഖ്യ new_size)
{
	പൂർണ്ണസംഖ്യ expand_by;	/* Number of bytes to expand by (-ve to contract) */
	പൂർണ്ണസംഖ്യ new_alloced;
	ulong next_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_size - rec->size, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->used_size + expand_by, BLOBLIST_BLOB_ALIGN);
	എങ്കിൽ (new_size < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_size);
		തിരികെ log_സന്ദേശം_ret("size", -EINVAL);
	}
	എങ്കിൽ (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", new_size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		തിരികെ log_സന്ദേശം_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	next_ofs = bloblist_blob_end_ofs(hdr, rec);
	എങ്കിൽ (next_ofs != hdr->used_size) {
		memmove((ശൂന്യം *)hdr + next_ofs + expand_by,
			(ശൂന്യം *)hdr + next_ofs, hdr->used_size - next_ofs);
	}
	hdr->used_size = new_alloced;

	/* Zero the new part of the blob */
	എങ്കിൽ (expand_by > 0) {
		memset((ശൂന്യം *)rec + rec_hdr_size(rec) + rec->size, '\0',
		       new_size - rec->size);
	}

	/* Update the size of this blob */
	rec->size = new_size;

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ bloblist_resize(uint tag, പൂർണ്ണസംഖ്യ new_size)
{
	ഘടന bloblist_hdr *hdr = gd->bloblist;
	ഘടന bloblist_rec *rec;
	പൂർണ്ണസംഖ്യ ret;

	rec = bloblist_findrec(tag);
	എങ്കിൽ (!rec)
		തിരികെ log_സന്ദേശം_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_size);
	എങ്കിൽ (ret)
		തിരികെ log_സന്ദേശം_ret("resize", ret);

	തിരികെ 0;
}

സ്ഥിതം u32 bloblist_calc_chksum(ഘടന bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = table_compute_checksum(hdr, hdr->used_size);
	chksum += hdr->chksum;

	തിരികെ chksum;
}

പൂർണ്ണസംഖ്യ bloblist_new(ulong addr, uint size, uint flags, uint align_log2)
{
	ഘടന bloblist_hdr *hdr;

	എങ്കിൽ (size < വലുപ്പം(*hdr))
		തിരികെ log_ret(-ENOSPC);
	എങ്കിൽ (addr & (BLOBLIST_ALIGN - 1))
		തിരികെ log_ret(-EFAULT);
	hdr = map_sysmem(addr, size);
	memset(hdr, '\0', വലുപ്പം(*hdr));
	hdr->version = BLOBLIST_VERSION;
	hdr->hdr_size = വലുപ്പം(*hdr);
	hdr->flags = flags;
	hdr->magic = BLOBLIST_MAGIC;
	hdr->used_size = hdr->hdr_size;
	hdr->total_size = size;
	hdr->align_log2 = align_log2 ? align_log2 : BLOBLIST_BLOB_ALIGN_LOG2;
	hdr->chksum = 0;
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ bloblist_check(ulong addr, uint size)
{
	ഘടന bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, വലുപ്പം(*hdr));
	എങ്കിൽ (hdr->magic != BLOBLIST_MAGIC)
		തിരികെ log_സന്ദേശം_ret("Bad magic", -ENOENT);
	എങ്കിൽ (hdr->version != BLOBLIST_VERSION)
		തിരികെ log_സന്ദേശം_ret("Bad version", -EPROTONOSUPPORT);
	എങ്കിൽ (!hdr->total_size || (size && hdr->total_size > size))
		തിരികെ log_സന്ദേശം_ret("Bad total size", -EFBIG);
	എങ്കിൽ (hdr->used_size > hdr->total_size)
		തിരികെ log_സന്ദേശം_ret("Bad used size", -ENOENT);
	എങ്കിൽ (hdr->hdr_size != വലുപ്പം(ഘടന bloblist_hdr))
		തിരികെ log_സന്ദേശം_ret("Bad header size", -ENOENT);

	chksum = bloblist_calc_chksum(hdr);
	എങ്കിൽ (hdr->chksum != chksum) {
		log_err("Checksum %x != %x\n", hdr->chksum, chksum);
		തിരികെ log_സന്ദേശം_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ bloblist_finish(ശൂന്യം)
{
	ഘടന bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->used_size,
		  (ulong)map_to_sysmem(hdr));

	തിരികെ 0;
}

ulong bloblist_get_base(ശൂന്യം)
{
	തിരികെ map_to_sysmem(gd->bloblist);
}

ulong bloblist_get_size(ശൂന്യം)
{
	ഘടന bloblist_hdr *hdr = gd->bloblist;

	തിരികെ hdr->used_size;
}

ulong bloblist_get_total_size(ശൂന്യം)
{
	ഘടന bloblist_hdr *hdr = gd->bloblist;

	തിരികെ hdr->total_size;
}

ശൂന്യം bloblist_get_stats(ulong *basep, ulong *tsizep, ulong *usizep)
{
	ഘടന bloblist_hdr *hdr = gd->bloblist;

	*basep = map_to_sysmem(gd->bloblist);
	*tsizep = hdr->total_size;
	*usizep = hdr->used_size;
}

സ്ഥിതം ശൂന്യം show_മൂല്യം(സ്ഥിരം അക്ഷരം *prompt, ulong മൂല്യം)
{
	printf("%s:%*s %-5lx  ", prompt, 10 - (പൂർണ്ണസംഖ്യ)strlen(prompt), "", മൂല്യം);
	print_size(മൂല്യം, "\n");
}

ശൂന്യം bloblist_show_stats(ശൂന്യം)
{
	ulong base, tsize, usize;

	bloblist_get_stats(&base, &tsize, &usize);
	printf("base:       %lx\n", base);
	show_മൂല്യം("total size", tsize);
	show_മൂല്യം("used size", usize);
	show_മൂല്യം("free", tsize - usize);
}

ശൂന്യം bloblist_show_list(ശൂന്യം)
{
	ഘടന bloblist_hdr *hdr = gd->bloblist;
	ഘടന bloblist_rec *rec;

	printf("%-8s  %8s   Tag Name\n", "Address", "Size");
	വേണ്ടി (rec = bloblist_first_blob(hdr); rec;
	     rec = bloblist_next_blob(hdr, rec)) {
		printf("%08lx  %8x  %4x %s\n",
		       (ulong)map_to_sysmem((ശൂന്യം *)rec + rec_hdr_size(rec)),
		       rec->size, rec_tag(rec),
		       bloblist_tag_name(rec_tag(rec)));
	}
}

പൂർണ്ണസംഖ്യ bloblist_reloc(ശൂന്യം *to, uint to_size)
{
	ഘടന bloblist_hdr *hdr;

	എങ്കിൽ (!to_size)
		തിരികെ 0;

	എങ്കിൽ (to_size < gd->bloblist->total_size)
		തിരികെ -ENOSPC;

	memcpy(to, gd->bloblist, gd->bloblist->total_size);
	hdr = to;
	hdr->total_size = to_size;
	gd->bloblist = to;

	തിരികെ 0;
}

/*
 * Weak default function for getting bloblist from boot args.
 */
പൂർണ്ണസംഖ്യ __weak xferlist_from_boot_arg(ulong __always_unused *addr)
{
	തിരികെ -ENOENT;
}

bool bloblist_exists(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ ret;
	ulong addr = 0;

	/* Check if a valid transfer list passed in */
	എങ്കിൽ (!xferlist_from_boot_arg(&addr))
		പോകുക found;

	/*
	 * If Firmware Handoff is mandatory but no transfer list is
	 * observed, report it as an error.
	 */
	എങ്കിൽ (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		തിരികെ false;

	/*
	 * We have checked for a valid transfer list being passed. At this
	 * point, if we do not have a fixed address for the bloblist, we cannot
	 * be provided with one.
	 */
	എങ്കിൽ (xpl_is_first_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		തിരികെ false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_SIZE);
	എങ്കിൽ (!ret)
		പോകുക found;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	തിരികെ false;

found:
#ifdef DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#endif
	തിരികെ true;
}

പൂർണ്ണസംഖ്യ bloblist_init(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ ret;
	ulong addr = 0, size = CONFIG_BLOBLIST_SIZE;

	എങ്കിൽ (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->total_size, gd->bloblist);
		തിരികെ 0;
	}

	/*
	 * If Firmware Handoff is mandatory but no transfer list has been
	 * observed by fdtdec_setup, report it as an error.
	 */
	എങ്കിൽ (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		തിരികെ -ENOENT;

	/*
	 * If we don't have an existing bloblist, we either need
	 * to allocate one now, or initialize the fixed address
	 * space as a bloblist.
	 */
	എങ്കിൽ (CONFIG_IS_ENABLED(BLOBLIST_ALLOC)) {
		ശൂന്യം *ptr = memalign(BLOBLIST_ALIGN, size);

		എങ്കിൽ (!ptr)
			തിരികെ log_സന്ദേശം_ret("alloc", -ENOMEM);
		addr = map_to_sysmem(ptr);
	} അല്ലെങ്കിൽ
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", size,
		  addr);
	ret = bloblist_new(addr, size, 0, 0);
	എങ്കിൽ (ret)
		തിരികെ log_സന്ദേശം_ret("ini", ret);

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	പൂർണ്ണസംഖ്യ ret;

	എങ്കിൽ ((IS_ENABLED(CONFIG_64BIT) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64BIT) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_SHIFT_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_64));
	} അല്ലെങ്കിൽ {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_SHIFT_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_32));
	}

	എങ്കിൽ (rzero || rsig != sigval)
		തിരികെ -EIO;

	ret = bloblist_check(xlist, 0);
	എങ്കിൽ (ret)
		തിരികെ ret;

	എങ്കിൽ (rfdt != (ulong)bloblist_find(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = ശൂന്യ;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		തിരികെ -EIO;
	}

	തിരികെ 0;
}
