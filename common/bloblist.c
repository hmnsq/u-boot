// SPDX-License-Identifier: GPL-2.0+ BSD-3-Clause
/*
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_BLOBLIST

#ಅನ್ತರ್ಭಾವ <bloblist.h>
#ಅನ್ತರ್ಭಾವ <display_options.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <tables_csum.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <u-boot/crc.h>

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

ಸ್ಥಿರ ಸಂರಚನಾ tag_name {
	ಗಣನಾ bloblist_tag_t tag;
	ಧ್ರುವ ವರ್ಣ *name;
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

ಧ್ರುವ ವರ್ಣ *bloblist_tag_name(ಗಣನಾ bloblist_tag_t tag)
{
	ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(tag_name); i++) {
		ಯದಿ (tag_name[i].tag == tag)
			ಪ್ರತ್ಯಾಗಮ tag_name[i].name;
	}

	ಪ್ರತ್ಯಾಗಮ "invalid";
}

ಸ್ಥಿರ ಸಂರಚನಾ bloblist_rec *bloblist_first_blob(ಸಂರಚನಾ bloblist_hdr *hdr)
{
	ಯದಿ (hdr->used_size <= hdr->hdr_size)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	ಪ್ರತ್ಯಾಗಮ (ಸಂರಚನಾ bloblist_rec *)((ಶೂನ್ಯ *)hdr + hdr->hdr_size);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ uint rec_hdr_size(ಸಂರಚನಾ bloblist_rec *rec)
{
	ಪ್ರತ್ಯಾಗಮ (rec->tag_and_hdr_size & BLOBLISTR_HDR_SIZE_MASK) >>
		BLOBLISTR_HDR_SIZE_SHIFT;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ uint rec_tag(ಸಂರಚನಾ bloblist_rec *rec)
{
	ಪ್ರತ್ಯಾಗಮ (rec->tag_and_hdr_size & BLOBLISTR_TAG_MASK) >>
		BLOBLISTR_TAG_SHIFT;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ void_blob(ಸಂರಚನಾ bloblist_rec *rec)
{
	ಯದಿ (rec_tag(rec) == BLOBLISTT_VOID)
		ಪ್ರತ್ಯಾಗಮ;
	rec->tag_and_hdr_size = BLOBLISTT_VOID |
				ಪರಿಮಾಣ(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಸಂರಚನಾ bloblist_rec *rec_from_blob(ಶೂನ್ಯ *blob)
{
	ಪ್ರತ್ಯಾಗಮ (blob - ಪರಿಮಾಣ(ಸಂರಚನಾ bloblist_rec));
}

ಸ್ಥಿರ ulong bloblist_blob_end_ofs(ಸಂರಚನಾ bloblist_hdr *hdr,
				   ಸಂರಚನಾ bloblist_rec *rec)
{
	ulong offset;

	offset = (ಶೂನ್ಯ *)rec - (ಶೂನ್ಯ *)hdr;
	/*
	 * The data section of next TE should start from an address aligned
	 * to 1 << hdr->align_log2.
	 */
	offset += rec_hdr_size(rec) + rec->size;
	offset = round_up(offset + rec_hdr_size(rec), 1 << hdr->align_log2);
	offset -= rec_hdr_size(rec);

	ಪ್ರತ್ಯಾಗಮ offset;
}

ಸ್ಥಿರ ಸಂರಚನಾ bloblist_rec *bloblist_next_blob(ಸಂರಚನಾ bloblist_hdr *hdr,
					       ಸಂರಚನಾ bloblist_rec *rec)
{
	ulong offset = bloblist_blob_end_ofs(hdr, rec);

	ಯದಿ (offset >= hdr->used_size)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	ಪ್ರತ್ಯಾಗಮ (ಸಂರಚನಾ bloblist_rec *)((ಶೂನ್ಯ *)hdr + offset);
}

#define foreach_rec(_rec, _hdr) \
	ಪ್ರತಿ (_rec = bloblist_first_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_next_blob(_hdr, _rec))

ಸ್ಥಿರ ಸಂರಚನಾ bloblist_rec *bloblist_findrec(uint tag)
{
	ಸಂರಚನಾ bloblist_hdr *hdr = gd->bloblist;
	ಸಂರಚನಾ bloblist_rec *rec;

	ಯದಿ (!hdr)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	foreach_rec(rec, hdr) {
		ಯದಿ (rec_tag(rec) == tag)
			ಪ್ರತ್ಯಾಗಮ rec;
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ bloblist_addrec(uint tag, ಪೂರ್ಣಾಙ್ಕ size, ಪೂರ್ಣಾಙ್ಕ align_log2,
			   ಸಂರಚನಾ bloblist_rec **recp)
{
	ಸಂರಚನಾ bloblist_hdr *hdr = gd->bloblist;
	ಸಂರಚನಾ bloblist_rec *rec;
	phys_addr_t data_start, aligned_start;
	phys_size_t new_alloced;

	ಯದಿ (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	data_start = map_to_sysmem(hdr) + hdr->used_size + ಪರಿಮಾಣ(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_start = ALIGN(data_start, 1U << align_log2) - data_start;

	/* If we need to create a dummy record, create it */
	ಯದಿ (aligned_start) {
		ಪೂರ್ಣಾಙ್ಕ void_size = aligned_start - ಪರಿಮಾಣ(*rec);
		ಸಂರಚನಾ bloblist_rec *vrec;
		ಪೂರ್ಣಾಙ್ಕ ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_size, 0, &vrec);
		ಯದಿ (ret)
			ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("void", ret);

		/* start the record after that */
		data_start = map_to_sysmem(hdr) + hdr->used_size + ಪರಿಮಾಣ(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = data_start - map_to_sysmem(hdr) +
		ALIGN(size, 1U << align_log2);

	ಯದಿ (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("bloblist add", -ENOSPC);
	}
	rec = (ಶೂನ್ಯ *)hdr + hdr->used_size;

	rec->tag_and_hdr_size = tag | ಪರಿಮಾಣ(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
	rec->size = size;

	/* Zero the record data */
	memset((ಶೂನ್ಯ *)rec + rec_hdr_size(rec), '\0', rec->size);

	hdr->used_size = new_alloced;
	*recp = rec;

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ bloblist_ensurerec(uint tag, ಸಂರಚನಾ bloblist_rec **recp, ಪೂರ್ಣಾಙ್ಕ size,
			      ಪೂರ್ಣಾಙ್ಕ align_log2)
{
	ಸಂರಚನಾ bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	ಯದಿ (rec) {
		ಯದಿ (size && size != rec->size) {
			*recp = rec;
			ಪ್ರತ್ಯಾಗಮ -ESPIPE;
		}
	} ಅನ್ಯಥಾ {
		ಪೂರ್ಣಾಙ್ಕ ret;

		ret = bloblist_addrec(tag, size, align_log2, &rec);
		ಯದಿ (ret)
			ಪ್ರತ್ಯಾಗಮ ret;
	}
	*recp = rec;

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ bloblist_get_blob_data_offset(uint tag)
{
	ವಿಕಲ್ಪ (tag) {
	ಪಕ್ಷ BLOBLISTT_FDT_OVERLAY:
		ಪ್ರತ್ಯಾಗಮ ಪರಿಮಾಣ(ಸಂರಚನಾ dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಶೂನ್ಯ *bloblist_find(uint tag, ಪೂರ್ಣಾಙ್ಕ size)
{
	ಶೂನ್ಯ *blob = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ blob_size;

	blob = bloblist_get_blob(tag, &blob_size);

	ಯದಿ (size && size != blob_size)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಪ್ರತ್ಯಾಗಮ blob;
}

ಶೂನ್ಯ *bloblist_get_blob(uint tag, ಪೂರ್ಣಾಙ್ಕ *sizep)
{
	ಸಂರಚನಾ bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	ಯದಿ (!rec)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	*sizep = rec->size;

	ಪ್ರತ್ಯಾಗಮ (ಶೂನ್ಯ *)rec + rec_hdr_size(rec);
}

ಪೂರ್ಣಾಙ್ಕ bloblist_apply_blobs(uint tag, ಪೂರ್ಣಾಙ್ಕ (*func)(ಶೂನ್ಯ **data, ಪೂರ್ಣಾಙ್ಕ size))
{
	ಸಂರಚನಾ bloblist_hdr *hdr = gd->bloblist;
	ಸಂರಚನಾ bloblist_rec *rec;

	ಯದಿ (!func || !hdr)
		ಪ್ರತ್ಯಾಗಮ -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		ಯದಿ (rec_tag(rec) == tag) {
			ಪೂರ್ಣಾಙ್ಕ ret;
			ಪೂರ್ಣಾಙ್ಕ tag = rec_tag(rec);
			ಶೂನ್ಯ *blob = (ಶೂನ್ಯ *)rec + rec_hdr_size(rec);
			ಪೂರ್ಣಾಙ್ಕ dat_off = bloblist_get_blob_data_offset(tag);

			blob += dat_off;
			ret = func(&blob, rec->size - dat_off);
			ಯದಿ (ret) {
				log_err("Failed to apply blob with tag %d\n",
					tag);
				ಪ್ರತ್ಯಾಗಮ ret;
			}

			rec = rec_from_blob(blob - dat_off);
			ಯದಿ (!rec) {
				log_err("Blob corrupted\n");
				ಪ್ರತ್ಯಾಗಮ -ENOENT;
			}

			/* Mark applied blob record as void */
			void_blob(rec);
		}
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಶೂನ್ಯ *bloblist_add(uint tag, ಪೂರ್ಣಾಙ್ಕ size, ಪೂರ್ಣಾಙ್ಕ align_log2)
{
	ಸಂರಚನಾ bloblist_rec *rec;

	ಯದಿ (bloblist_addrec(tag, size, align_log2, &rec))
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಪ್ರತ್ಯಾಗಮ (ಶೂನ್ಯ *)rec + rec_hdr_size(rec);
}

ಪೂರ್ಣಾಙ್ಕ bloblist_ensure_size(uint tag, ಪೂರ್ಣಾಙ್ಕ size, ಪೂರ್ಣಾಙ್ಕ align_log2, ಶೂನ್ಯ **blobp)
{
	ಸಂರಚನಾ bloblist_rec *rec;
	ಪೂರ್ಣಾಙ್ಕ ret;

	ret = bloblist_ensurerec(tag, &rec, size, align_log2);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;
	*blobp = (ಶೂನ್ಯ *)rec + rec_hdr_size(rec);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಶೂನ್ಯ *bloblist_ensure(uint tag, ಪೂರ್ಣಾಙ್ಕ size)
{
	ಸಂರಚನಾ bloblist_rec *rec;

	ಯದಿ (bloblist_ensurerec(tag, &rec, size, 0))
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಪ್ರತ್ಯಾಗಮ (ಶೂನ್ಯ *)rec + rec_hdr_size(rec);
}

ಪೂರ್ಣಾಙ್ಕ bloblist_ensure_size_ret(uint tag, ಪೂರ್ಣಾಙ್ಕ *sizep, ಶೂನ್ಯ **blobp)
{
	ಸಂರಚನಾ bloblist_rec *rec;
	ಪೂರ್ಣಾಙ್ಕ ret;

	ret = bloblist_ensurerec(tag, &rec, *sizep, 0);
	ಯದಿ (ret == -ESPIPE)
		*sizep = rec->size;
	ಅನ್ಯಥಾ ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;
	*blobp = (ಶೂನ್ಯ *)rec + rec_hdr_size(rec);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ bloblist_resize_rec(ಸಂರಚನಾ bloblist_hdr *hdr,
			       ಸಂರಚನಾ bloblist_rec *rec,
			       ಪೂರ್ಣಾಙ್ಕ new_size)
{
	ಪೂರ್ಣಾಙ್ಕ expand_by;	/* Number of bytes to expand by (-ve to contract) */
	ಪೂರ್ಣಾಙ್ಕ new_alloced;
	ulong next_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_size - rec->size, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->used_size + expand_by, BLOBLIST_BLOB_ALIGN);
	ಯದಿ (new_size < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_size);
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("size", -EINVAL);
	}
	ಯದಿ (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", new_size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	next_ofs = bloblist_blob_end_ofs(hdr, rec);
	ಯದಿ (next_ofs != hdr->used_size) {
		memmove((ಶೂನ್ಯ *)hdr + next_ofs + expand_by,
			(ಶೂನ್ಯ *)hdr + next_ofs, hdr->used_size - next_ofs);
	}
	hdr->used_size = new_alloced;

	/* Zero the new part of the blob */
	ಯದಿ (expand_by > 0) {
		memset((ಶೂನ್ಯ *)rec + rec_hdr_size(rec) + rec->size, '\0',
		       new_size - rec->size);
	}

	/* Update the size of this blob */
	rec->size = new_size;

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ bloblist_resize(uint tag, ಪೂರ್ಣಾಙ್ಕ new_size)
{
	ಸಂರಚನಾ bloblist_hdr *hdr = gd->bloblist;
	ಸಂರಚನಾ bloblist_rec *rec;
	ಪೂರ್ಣಾಙ್ಕ ret;

	rec = bloblist_findrec(tag);
	ಯದಿ (!rec)
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_size);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("resize", ret);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ u32 bloblist_calc_chksum(ಸಂರಚನಾ bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = table_compute_checksum(hdr, hdr->used_size);
	chksum += hdr->chksum;

	ಪ್ರತ್ಯಾಗಮ chksum;
}

ಪೂರ್ಣಾಙ್ಕ bloblist_new(ulong addr, uint size, uint flags, uint align_log2)
{
	ಸಂರಚನಾ bloblist_hdr *hdr;

	ಯದಿ (size < ಪರಿಮಾಣ(*hdr))
		ಪ್ರತ್ಯಾಗಮ log_ret(-ENOSPC);
	ಯದಿ (addr & (BLOBLIST_ALIGN - 1))
		ಪ್ರತ್ಯಾಗಮ log_ret(-EFAULT);
	hdr = map_sysmem(addr, size);
	memset(hdr, '\0', ಪರಿಮಾಣ(*hdr));
	hdr->version = BLOBLIST_VERSION;
	hdr->hdr_size = ಪರಿಮಾಣ(*hdr);
	hdr->flags = flags;
	hdr->magic = BLOBLIST_MAGIC;
	hdr->used_size = hdr->hdr_size;
	hdr->total_size = size;
	hdr->align_log2 = align_log2 ? align_log2 : BLOBLIST_BLOB_ALIGN_LOG2;
	hdr->chksum = 0;
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ bloblist_check(ulong addr, uint size)
{
	ಸಂರಚನಾ bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, ಪರಿಮಾಣ(*hdr));
	ಯದಿ (hdr->magic != BLOBLIST_MAGIC)
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("Bad magic", -ENOENT);
	ಯದಿ (hdr->version != BLOBLIST_VERSION)
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("Bad version", -EPROTONOSUPPORT);
	ಯದಿ (!hdr->total_size || (size && hdr->total_size > size))
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("Bad total size", -EFBIG);
	ಯದಿ (hdr->used_size > hdr->total_size)
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("Bad used size", -ENOENT);
	ಯದಿ (hdr->hdr_size != ಪರಿಮಾಣ(ಸಂರಚನಾ bloblist_hdr))
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("Bad header size", -ENOENT);

	chksum = bloblist_calc_chksum(hdr);
	ಯದಿ (hdr->chksum != chksum) {
		log_err("Checksum %x != %x\n", hdr->chksum, chksum);
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ bloblist_finish(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->used_size,
		  (ulong)map_to_sysmem(hdr));

	ಪ್ರತ್ಯಾಗಮ 0;
}

ulong bloblist_get_base(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ map_to_sysmem(gd->bloblist);
}

ulong bloblist_get_size(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ bloblist_hdr *hdr = gd->bloblist;

	ಪ್ರತ್ಯಾಗಮ hdr->used_size;
}

ulong bloblist_get_total_size(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ bloblist_hdr *hdr = gd->bloblist;

	ಪ್ರತ್ಯಾಗಮ hdr->total_size;
}

ಶೂನ್ಯ bloblist_get_stats(ulong *basep, ulong *tsizep, ulong *usizep)
{
	ಸಂರಚನಾ bloblist_hdr *hdr = gd->bloblist;

	*basep = map_to_sysmem(gd->bloblist);
	*tsizep = hdr->total_size;
	*usizep = hdr->used_size;
}

ಸ್ಥಿರ ಶೂನ್ಯ show_ಮೌಲ್ಯ(ಧ್ರುವ ವರ್ಣ *prompt, ulong ಮೌಲ್ಯ)
{
	printf("%s:%*s %-5lx  ", prompt, 10 - (ಪೂರ್ಣಾಙ್ಕ)strlen(prompt), "", ಮೌಲ್ಯ);
	print_size(ಮೌಲ್ಯ, "\n");
}

ಶೂನ್ಯ bloblist_show_stats(ಶೂನ್ಯ)
{
	ulong base, tsize, usize;

	bloblist_get_stats(&base, &tsize, &usize);
	printf("base:       %lx\n", base);
	show_ಮೌಲ್ಯ("total size", tsize);
	show_ಮೌಲ್ಯ("used size", usize);
	show_ಮೌಲ್ಯ("free", tsize - usize);
}

ಶೂನ್ಯ bloblist_show_list(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ bloblist_hdr *hdr = gd->bloblist;
	ಸಂರಚನಾ bloblist_rec *rec;

	printf("%-8s  %8s   Tag Name\n", "Address", "Size");
	ಪ್ರತಿ (rec = bloblist_first_blob(hdr); rec;
	     rec = bloblist_next_blob(hdr, rec)) {
		printf("%08lx  %8x  %4x %s\n",
		       (ulong)map_to_sysmem((ಶೂನ್ಯ *)rec + rec_hdr_size(rec)),
		       rec->size, rec_tag(rec),
		       bloblist_tag_name(rec_tag(rec)));
	}
}

ಪೂರ್ಣಾಙ್ಕ bloblist_reloc(ಶೂನ್ಯ *to, uint to_size)
{
	ಸಂರಚನಾ bloblist_hdr *hdr;

	ಯದಿ (!to_size)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (to_size < gd->bloblist->total_size)
		ಪ್ರತ್ಯಾಗಮ -ENOSPC;

	memcpy(to, gd->bloblist, gd->bloblist->total_size);
	hdr = to;
	hdr->total_size = to_size;
	gd->bloblist = to;

	ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * Weak default function for getting bloblist from boot args.
 */
ಪೂರ್ಣಾಙ್ಕ __weak xferlist_from_boot_arg(ulong __always_unused *addr)
{
	ಪ್ರತ್ಯಾಗಮ -ENOENT;
}

bool bloblist_exists(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ ret;
	ulong addr = 0;

	/* Check if a valid transfer list passed in */
	ಯದಿ (!xferlist_from_boot_arg(&addr))
		ಗಚ್ಛ found;

	/*
	 * If Firmware Handoff is mandatory but no transfer list is
	 * observed, report it as an error.
	 */
	ಯದಿ (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		ಪ್ರತ್ಯಾಗಮ false;

	/*
	 * We have checked for a valid transfer list being passed. At this
	 * point, if we do not have a fixed address for the bloblist, we cannot
	 * be provided with one.
	 */
	ಯದಿ (xpl_is_first_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		ಪ್ರತ್ಯಾಗಮ false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_SIZE);
	ಯದಿ (!ret)
		ಗಚ್ಛ found;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	ಪ್ರತ್ಯಾಗಮ false;

found:
#ifdef DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#endif
	ಪ್ರತ್ಯಾಗಮ true;
}

ಪೂರ್ಣಾಙ್ಕ bloblist_init(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ ret;
	ulong addr = 0, size = CONFIG_BLOBLIST_SIZE;

	ಯದಿ (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->total_size, gd->bloblist);
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	/*
	 * If Firmware Handoff is mandatory but no transfer list has been
	 * observed by fdtdec_setup, report it as an error.
	 */
	ಯದಿ (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		ಪ್ರತ್ಯಾಗಮ -ENOENT;

	/*
	 * If we don't have an existing bloblist, we either need
	 * to allocate one now, or initialize the fixed address
	 * space as a bloblist.
	 */
	ಯದಿ (CONFIG_IS_ENABLED(BLOBLIST_ALLOC)) {
		ಶೂನ್ಯ *ptr = memalign(BLOBLIST_ALIGN, size);

		ಯದಿ (!ptr)
			ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("alloc", -ENOMEM);
		addr = map_to_sysmem(ptr);
	} ಅನ್ಯಥಾ
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", size,
		  addr);
	ret = bloblist_new(addr, size, 0, 0);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("ini", ret);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	ಪೂರ್ಣಾಙ್ಕ ret;

	ಯದಿ ((IS_ENABLED(CONFIG_64BIT) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64BIT) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_SHIFT_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_64));
	} ಅನ್ಯಥಾ {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_SHIFT_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_32));
	}

	ಯದಿ (rzero || rsig != sigval)
		ಪ್ರತ್ಯಾಗಮ -EIO;

	ret = bloblist_check(xlist, 0);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;

	ಯದಿ (rfdt != (ulong)bloblist_find(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = ರಿಕ್ತ;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		ಪ್ರತ್ಯಾಗಮ -EIO;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}
