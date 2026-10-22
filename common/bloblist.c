// SPDX-License-Identifier: GPL-2.0+ BSD-3-Clause
/*
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LOG_CATEGORY	LOGC_BLOBLIST

#ਅਨ੍ਤਰ੍ਭਾਵ <bloblist.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <display_options.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mapmem.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <tables_csum.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <u-boot/crc.h>

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

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

ਸਥਿਰ ਬਣਤਰ ਟੈਗ_name {
	ਗਿਣਤੀ bloblist_ਟੈਗ_t ਟੈਗ;
	ਅਚਲ ਅੱਖਰ *name;
} ਟੈਗ_name[] = {
	{ BLOBLISTT_VOID, "(void)" },

	/* BLOBLISTT_AREA_FIRMWARE_TOP */
	{ BLOBLISTT_CONTROL_FDT, "Control FDT" },
	{ BLOBLISTT_HOB_ਖੰਡ, "HOB block" },
	{ BLOBLISTT_HOB_LIST, "HOB list" },
	{ BLOBLISTT_ACPI_TABLES, "ACPI tables for x86" },
	{ BLOBLISTT_TPM_EVLOG, "TPM event log defined by TCG EFI" },
	{ BLOBLISTT_TPM_CRB_ਅਧਾਰ, "TPM Command Response Buffer address" },
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

ਅਚਲ ਅੱਖਰ *bloblist_ਟੈਗ_name(ਗਿਣਤੀ bloblist_ਟੈਗ_t ਟੈਗ)
{
	ਪੂਰਨ i;

	ਲਈ (i = 0; i < ARRAY_ਮਾਪ(ਟੈਗ_name); i++) {
		ਜੇ (ਟੈਗ_name[i].ਟੈਗ == ਟੈਗ)
			ਵਾਪਸ ਟੈਗ_name[i].name;
	}

	ਵਾਪਸ "invalid";
}

ਸਥਿਰ ਬਣਤਰ bloblist_rec *bloblist_ਪਹਿਲਾ_blob(ਬਣਤਰ bloblist_hdr *hdr)
{
	ਜੇ (hdr->ਵਰਤਿਆ_ਮਾਪ <= hdr->hdr_ਮਾਪ)
		ਵਾਪਸ ਨਿਰਾ;
	ਵਾਪਸ (ਬਣਤਰ bloblist_rec *)((ਖਾਲੀ *)hdr + hdr->hdr_ਮਾਪ);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ uint rec_hdr_ਮਾਪ(ਬਣਤਰ bloblist_rec *rec)
{
	ਵਾਪਸ (rec->ਟੈਗ_and_hdr_ਮਾਪ & BLOBLISTR_HDR_ਮਾਪ_ਨਕਾਬ) >>
		BLOBLISTR_HDR_ਮਾਪ_ਖਿਸਕਾਓ;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ uint rec_ਟੈਗ(ਬਣਤਰ bloblist_rec *rec)
{
	ਵਾਪਸ (rec->ਟੈਗ_and_hdr_ਮਾਪ & BLOBLISTR_ਟੈਗ_ਨਕਾਬ) >>
		BLOBLISTR_ਟੈਗ_ਖਿਸਕਾਓ;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ void_blob(ਬਣਤਰ bloblist_rec *rec)
{
	ਜੇ (rec_ਟੈਗ(rec) == BLOBLISTT_VOID)
		ਵਾਪਸ;
	rec->ਟੈਗ_and_hdr_ਮਾਪ = BLOBLISTT_VOID |
				ਆਕਾਰ(*rec) << BLOBLISTR_HDR_ਮਾਪ_ਖਿਸਕਾਓ;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਣਤਰ bloblist_rec *rec_from_blob(ਖਾਲੀ *blob)
{
	ਵਾਪਸ (blob - ਆਕਾਰ(ਬਣਤਰ bloblist_rec));
}

ਸਥਿਰ ulong bloblist_blob_ਸਿਰਾ_ofs(ਬਣਤਰ bloblist_hdr *hdr,
				   ਬਣਤਰ bloblist_rec *rec)
{
	ulong ਵਿੱਥ;

	ਵਿੱਥ = (ਖਾਲੀ *)rec - (ਖਾਲੀ *)hdr;
	/*
	 * The data section of next TE should start from an address aligned
	 * to 1 << hdr->align_log2.
	 */
	ਵਿੱਥ += rec_hdr_ਮਾਪ(rec) + rec->ਮਾਪ;
	ਵਿੱਥ = ਗੇੜਾ_ਉੱਪਰ(ਵਿੱਥ + rec_hdr_ਮਾਪ(rec), 1 << hdr->align_log2);
	ਵਿੱਥ -= rec_hdr_ਮਾਪ(rec);

	ਵਾਪਸ ਵਿੱਥ;
}

ਸਥਿਰ ਬਣਤਰ bloblist_rec *bloblist_ਅਗਲਾ_blob(ਬਣਤਰ bloblist_hdr *hdr,
					       ਬਣਤਰ bloblist_rec *rec)
{
	ulong ਵਿੱਥ = bloblist_blob_ਸਿਰਾ_ofs(hdr, rec);

	ਜੇ (ਵਿੱਥ >= hdr->ਵਰਤਿਆ_ਮਾਪ)
		ਵਾਪਸ ਨਿਰਾ;
	ਵਾਪਸ (ਬਣਤਰ bloblist_rec *)((ਖਾਲੀ *)hdr + ਵਿੱਥ);
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ foreach_rec(_rec, _hdr) \
	ਲਈ (_rec = bloblist_ਪਹਿਲਾ_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_ਅਗਲਾ_blob(_hdr, _rec))

ਸਥਿਰ ਬਣਤਰ bloblist_rec *bloblist_findrec(uint ਟੈਗ)
{
	ਬਣਤਰ bloblist_hdr *hdr = gd->bloblist;
	ਬਣਤਰ bloblist_rec *rec;

	ਜੇ (!hdr)
		ਵਾਪਸ ਨਿਰਾ;

	foreach_rec(rec, hdr) {
		ਜੇ (rec_ਟੈਗ(rec) == ਟੈਗ)
			ਵਾਪਸ rec;
	}

	ਵਾਪਸ ਨਿਰਾ;
}

ਸਥਿਰ ਪੂਰਨ bloblist_addrec(uint ਟੈਗ, ਪੂਰਨ ਮਾਪ, ਪੂਰਨ align_log2,
			   ਬਣਤਰ bloblist_rec **recp)
{
	ਬਣਤਰ bloblist_hdr *hdr = gd->bloblist;
	ਬਣਤਰ bloblist_rec *rec;
	phys_addr_t ਅੰਕੜੇ_ਸ਼ੁਰੂ, aligned_ਸ਼ੁਰੂ;
	phys_ਮਾਪ_t new_alloced;

	ਜੇ (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	ਅੰਕੜੇ_ਸ਼ੁਰੂ = map_ਮੰਜ਼ਿਲ_sysmem(hdr) + hdr->ਵਰਤਿਆ_ਮਾਪ + ਆਕਾਰ(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_ਸ਼ੁਰੂ = ALIGN(ਅੰਕੜੇ_ਸ਼ੁਰੂ, 1U << align_log2) - ਅੰਕੜੇ_ਸ਼ੁਰੂ;

	/* If we need to create a dummy record, create it */
	ਜੇ (aligned_ਸ਼ੁਰੂ) {
		ਪੂਰਨ void_ਮਾਪ = aligned_ਸ਼ੁਰੂ - ਆਕਾਰ(*rec);
		ਬਣਤਰ bloblist_rec *vrec;
		ਪੂਰਨ ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_ਮਾਪ, 0, &vrec);
		ਜੇ (ret)
			ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("void", ret);

		/* start the record after that */
		ਅੰਕੜੇ_ਸ਼ੁਰੂ = map_ਮੰਜ਼ਿਲ_sysmem(hdr) + hdr->ਵਰਤਿਆ_ਮਾਪ + ਆਕਾਰ(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = ਅੰਕੜੇ_ਸ਼ੁਰੂ - map_ਮੰਜ਼ਿਲ_sysmem(hdr) +
		ALIGN(ਮਾਪ, 1U << align_log2);

	ਜੇ (new_alloced > hdr->ਕੁੱਲ_ਮਾਪ) {
		log_err("Failed to allocate %x bytes\n", ਮਾਪ);
		log_err("Used size=%x, total size=%x\n",
			hdr->ਵਰਤਿਆ_ਮਾਪ, hdr->ਕੁੱਲ_ਮਾਪ);
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("bloblist add", -ENOSPC);
	}
	rec = (ਖਾਲੀ *)hdr + hdr->ਵਰਤਿਆ_ਮਾਪ;

	rec->ਟੈਗ_and_hdr_ਮਾਪ = ਟੈਗ | ਆਕਾਰ(*rec) << BLOBLISTR_HDR_ਮਾਪ_ਖਿਸਕਾਓ;
	rec->ਮਾਪ = ਮਾਪ;

	/* Zero the record data */
	memset((ਖਾਲੀ *)rec + rec_hdr_ਮਾਪ(rec), '\0', rec->ਮਾਪ);

	hdr->ਵਰਤਿਆ_ਮਾਪ = new_alloced;
	*recp = rec;

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ bloblist_ensurerec(uint ਟੈਗ, ਬਣਤਰ bloblist_rec **recp, ਪੂਰਨ ਮਾਪ,
			      ਪੂਰਨ align_log2)
{
	ਬਣਤਰ bloblist_rec *rec;

	rec = bloblist_findrec(ਟੈਗ);
	ਜੇ (rec) {
		ਜੇ (ਮਾਪ && ਮਾਪ != rec->ਮਾਪ) {
			*recp = rec;
			ਵਾਪਸ -ESPIPE;
		}
	} ਨਹੀਂਤਾਂ {
		ਪੂਰਨ ret;

		ret = bloblist_addrec(ਟੈਗ, ਮਾਪ, align_log2, &rec);
		ਜੇ (ret)
			ਵਾਪਸ ret;
	}
	*recp = rec;

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ bloblist_get_blob_ਅੰਕੜੇ_ਵਿੱਥ(uint ਟੈਗ)
{
	ਚੋਣ (ਟੈਗ) {
	ਹਾਲਤ BLOBLISTT_FDT_OVERLAY:
		ਵਾਪਸ ਆਕਾਰ(ਬਣਤਰ dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	ਵਾਪਸ 0;
}

ਖਾਲੀ *bloblist_ਲੱਭੋ(uint ਟੈਗ, ਪੂਰਨ ਮਾਪ)
{
	ਖਾਲੀ *blob = ਨਿਰਾ;
	ਪੂਰਨ blob_ਮਾਪ;

	blob = bloblist_get_blob(ਟੈਗ, &blob_ਮਾਪ);

	ਜੇ (ਮਾਪ && ਮਾਪ != blob_ਮਾਪ)
		ਵਾਪਸ ਨਿਰਾ;

	ਵਾਪਸ blob;
}

ਖਾਲੀ *bloblist_get_blob(uint ਟੈਗ, ਪੂਰਨ *sizep)
{
	ਬਣਤਰ bloblist_rec *rec;

	rec = bloblist_findrec(ਟੈਗ);
	ਜੇ (!rec)
		ਵਾਪਸ ਨਿਰਾ;

	*sizep = rec->ਮਾਪ;

	ਵਾਪਸ (ਖਾਲੀ *)rec + rec_hdr_ਮਾਪ(rec);
}

ਪੂਰਨ bloblist_apply_blobs(uint ਟੈਗ, ਪੂਰਨ (*func)(ਖਾਲੀ **ਅੰਕੜੇ, ਪੂਰਨ ਮਾਪ))
{
	ਬਣਤਰ bloblist_hdr *hdr = gd->bloblist;
	ਬਣਤਰ bloblist_rec *rec;

	ਜੇ (!func || !hdr)
		ਵਾਪਸ -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		ਜੇ (rec_ਟੈਗ(rec) == ਟੈਗ) {
			ਪੂਰਨ ret;
			ਪੂਰਨ ਟੈਗ = rec_ਟੈਗ(rec);
			ਖਾਲੀ *blob = (ਖਾਲੀ *)rec + rec_hdr_ਮਾਪ(rec);
			ਪੂਰਨ dat_off = bloblist_get_blob_ਅੰਕੜੇ_ਵਿੱਥ(ਟੈਗ);

			blob += dat_off;
			ret = func(&blob, rec->ਮਾਪ - dat_off);
			ਜੇ (ret) {
				log_err("Failed to apply blob with tag %d\n",
					ਟੈਗ);
				ਵਾਪਸ ret;
			}

			rec = rec_from_blob(blob - dat_off);
			ਜੇ (!rec) {
				log_err("Blob corrupted\n");
				ਵਾਪਸ -ENOENT;
			}

			/* Mark applied blob record as void */
			void_blob(rec);
		}
	}

	ਵਾਪਸ 0;
}

ਖਾਲੀ *bloblist_ਜੋੜੋ(uint ਟੈਗ, ਪੂਰਨ ਮਾਪ, ਪੂਰਨ align_log2)
{
	ਬਣਤਰ bloblist_rec *rec;

	ਜੇ (bloblist_addrec(ਟੈਗ, ਮਾਪ, align_log2, &rec))
		ਵਾਪਸ ਨਿਰਾ;

	ਵਾਪਸ (ਖਾਲੀ *)rec + rec_hdr_ਮਾਪ(rec);
}

ਪੂਰਨ bloblist_ensure_ਮਾਪ(uint ਟੈਗ, ਪੂਰਨ ਮਾਪ, ਪੂਰਨ align_log2, ਖਾਲੀ **blobp)
{
	ਬਣਤਰ bloblist_rec *rec;
	ਪੂਰਨ ret;

	ret = bloblist_ensurerec(ਟੈਗ, &rec, ਮਾਪ, align_log2);
	ਜੇ (ret)
		ਵਾਪਸ ret;
	*blobp = (ਖਾਲੀ *)rec + rec_hdr_ਮਾਪ(rec);

	ਵਾਪਸ 0;
}

ਖਾਲੀ *bloblist_ensure(uint ਟੈਗ, ਪੂਰਨ ਮਾਪ)
{
	ਬਣਤਰ bloblist_rec *rec;

	ਜੇ (bloblist_ensurerec(ਟੈਗ, &rec, ਮਾਪ, 0))
		ਵਾਪਸ ਨਿਰਾ;

	ਵਾਪਸ (ਖਾਲੀ *)rec + rec_hdr_ਮਾਪ(rec);
}

ਪੂਰਨ bloblist_ensure_ਮਾਪ_ret(uint ਟੈਗ, ਪੂਰਨ *sizep, ਖਾਲੀ **blobp)
{
	ਬਣਤਰ bloblist_rec *rec;
	ਪੂਰਨ ret;

	ret = bloblist_ensurerec(ਟੈਗ, &rec, *sizep, 0);
	ਜੇ (ret == -ESPIPE)
		*sizep = rec->ਮਾਪ;
	ਨਹੀਂਤਾਂ ਜੇ (ret)
		ਵਾਪਸ ret;
	*blobp = (ਖਾਲੀ *)rec + rec_hdr_ਮਾਪ(rec);

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ bloblist_resize_rec(ਬਣਤਰ bloblist_hdr *hdr,
			       ਬਣਤਰ bloblist_rec *rec,
			       ਪੂਰਨ new_ਮਾਪ)
{
	ਪੂਰਨ expand_by;	/* Number of bytes to expand by (-ve to contract) */
	ਪੂਰਨ new_alloced;
	ulong ਅਗਲਾ_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_ਮਾਪ - rec->ਮਾਪ, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->ਵਰਤਿਆ_ਮਾਪ + expand_by, BLOBLIST_BLOB_ALIGN);
	ਜੇ (new_ਮਾਪ < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_ਮਾਪ);
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("size", -EINVAL);
	}
	ਜੇ (new_alloced > hdr->ਕੁੱਲ_ਮਾਪ) {
		log_err("Failed to allocate %x bytes\n", new_ਮਾਪ);
		log_err("Used size=%x, total size=%x\n",
			hdr->ਵਰਤਿਆ_ਮਾਪ, hdr->ਕੁੱਲ_ਮਾਪ);
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	ਅਗਲਾ_ofs = bloblist_blob_ਸਿਰਾ_ofs(hdr, rec);
	ਜੇ (ਅਗਲਾ_ofs != hdr->ਵਰਤਿਆ_ਮਾਪ) {
		memmove((ਖਾਲੀ *)hdr + ਅਗਲਾ_ofs + expand_by,
			(ਖਾਲੀ *)hdr + ਅਗਲਾ_ofs, hdr->ਵਰਤਿਆ_ਮਾਪ - ਅਗਲਾ_ofs);
	}
	hdr->ਵਰਤਿਆ_ਮਾਪ = new_alloced;

	/* Zero the new part of the blob */
	ਜੇ (expand_by > 0) {
		memset((ਖਾਲੀ *)rec + rec_hdr_ਮਾਪ(rec) + rec->ਮਾਪ, '\0',
		       new_ਮਾਪ - rec->ਮਾਪ);
	}

	/* Update the size of this blob */
	rec->ਮਾਪ = new_ਮਾਪ;

	ਵਾਪਸ 0;
}

ਪੂਰਨ bloblist_resize(uint ਟੈਗ, ਪੂਰਨ new_ਮਾਪ)
{
	ਬਣਤਰ bloblist_hdr *hdr = gd->bloblist;
	ਬਣਤਰ bloblist_rec *rec;
	ਪੂਰਨ ret;

	rec = bloblist_findrec(ਟੈਗ);
	ਜੇ (!rec)
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_ਮਾਪ);
	ਜੇ (ret)
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("resize", ret);

	ਵਾਪਸ 0;
}

ਸਥਿਰ u32 bloblist_calc_chksum(ਬਣਤਰ bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = ਸਾਰਣੀ_compute_checksum(hdr, hdr->ਵਰਤਿਆ_ਮਾਪ);
	chksum += hdr->chksum;

	ਵਾਪਸ chksum;
}

ਪੂਰਨ bloblist_new(ulong addr, uint ਮਾਪ, uint flags, uint align_log2)
{
	ਬਣਤਰ bloblist_hdr *hdr;

	ਜੇ (ਮਾਪ < ਆਕਾਰ(*hdr))
		ਵਾਪਸ log_ret(-ENOSPC);
	ਜੇ (addr & (BLOBLIST_ALIGN - 1))
		ਵਾਪਸ log_ret(-EFAULT);
	hdr = map_sysmem(addr, ਮਾਪ);
	memset(hdr, '\0', ਆਕਾਰ(*hdr));
	hdr->version = BLOBLIST_ਸੰਸਕਰਣ;
	hdr->hdr_ਮਾਪ = ਆਕਾਰ(*hdr);
	hdr->flags = flags;
	hdr->magic = BLOBLIST_MAGIC;
	hdr->ਵਰਤਿਆ_ਮਾਪ = hdr->hdr_ਮਾਪ;
	hdr->ਕੁੱਲ_ਮਾਪ = ਮਾਪ;
	hdr->align_log2 = align_log2 ? align_log2 : BLOBLIST_BLOB_ALIGN_LOG2;
	hdr->chksum = 0;
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	ਵਾਪਸ 0;
}

ਪੂਰਨ bloblist_check(ulong addr, uint ਮਾਪ)
{
	ਬਣਤਰ bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, ਆਕਾਰ(*hdr));
	ਜੇ (hdr->magic != BLOBLIST_MAGIC)
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("Bad magic", -ENOENT);
	ਜੇ (hdr->version != BLOBLIST_ਸੰਸਕਰਣ)
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("Bad version", -EPROTONOSUPPORT);
	ਜੇ (!hdr->ਕੁੱਲ_ਮਾਪ || (ਮਾਪ && hdr->ਕੁੱਲ_ਮਾਪ > ਮਾਪ))
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("Bad total size", -EFBIG);
	ਜੇ (hdr->ਵਰਤਿਆ_ਮਾਪ > hdr->ਕੁੱਲ_ਮਾਪ)
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("Bad used size", -ENOENT);
	ਜੇ (hdr->hdr_ਮਾਪ != ਆਕਾਰ(ਬਣਤਰ bloblist_hdr))
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("Bad header size", -ENOENT);

	chksum = bloblist_calc_chksum(hdr);
	ਜੇ (hdr->chksum != chksum) {
		log_err("Checksum %x != %x\n", hdr->chksum, chksum);
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	ਵਾਪਸ 0;
}

ਪੂਰਨ bloblist_ਅੰਤ(ਖਾਲੀ)
{
	ਬਣਤਰ bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->ਵਰਤਿਆ_ਮਾਪ,
		  (ulong)map_ਮੰਜ਼ਿਲ_sysmem(hdr));

	ਵਾਪਸ 0;
}

ulong bloblist_get_ਅਧਾਰ(ਖਾਲੀ)
{
	ਵਾਪਸ map_ਮੰਜ਼ਿਲ_sysmem(gd->bloblist);
}

ulong bloblist_get_ਮਾਪ(ਖਾਲੀ)
{
	ਬਣਤਰ bloblist_hdr *hdr = gd->bloblist;

	ਵਾਪਸ hdr->ਵਰਤਿਆ_ਮਾਪ;
}

ulong bloblist_get_ਕੁੱਲ_ਮਾਪ(ਖਾਲੀ)
{
	ਬਣਤਰ bloblist_hdr *hdr = gd->bloblist;

	ਵਾਪਸ hdr->ਕੁੱਲ_ਮਾਪ;
}

ਖਾਲੀ bloblist_get_stats(ulong *basep, ulong *tsizep, ulong *usizep)
{
	ਬਣਤਰ bloblist_hdr *hdr = gd->bloblist;

	*basep = map_ਮੰਜ਼ਿਲ_sysmem(gd->bloblist);
	*tsizep = hdr->ਕੁੱਲ_ਮਾਪ;
	*usizep = hdr->ਵਰਤਿਆ_ਮਾਪ;
}

ਸਥਿਰ ਖਾਲੀ show_ਮੁੱਲ(ਅਚਲ ਅੱਖਰ *prompt, ulong ਮੁੱਲ)
{
	ਲਿਖੋ("%s:%*s %-5lx  ", prompt, 10 - (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(prompt), "", ਮੁੱਲ);
	print_ਮਾਪ(ਮੁੱਲ, "\n");
}

ਖਾਲੀ bloblist_show_stats(ਖਾਲੀ)
{
	ulong ਅਧਾਰ, tsize, usize;

	bloblist_get_stats(&ਅਧਾਰ, &tsize, &usize);
	ਲਿਖੋ("base:       %lx\n", ਅਧਾਰ);
	show_ਮੁੱਲ("total size", tsize);
	show_ਮੁੱਲ("used size", usize);
	show_ਮੁੱਲ("free", tsize - usize);
}

ਖਾਲੀ bloblist_show_list(ਖਾਲੀ)
{
	ਬਣਤਰ bloblist_hdr *hdr = gd->bloblist;
	ਬਣਤਰ bloblist_rec *rec;

	ਲਿਖੋ("%-8s  %8s   Tag Name\n", "Address", "Size");
	ਲਈ (rec = bloblist_ਪਹਿਲਾ_blob(hdr); rec;
	     rec = bloblist_ਅਗਲਾ_blob(hdr, rec)) {
		ਲਿਖੋ("%08lx  %8x  %4x %s\n",
		       (ulong)map_ਮੰਜ਼ਿਲ_sysmem((ਖਾਲੀ *)rec + rec_hdr_ਮਾਪ(rec)),
		       rec->ਮਾਪ, rec_ਟੈਗ(rec),
		       bloblist_ਟੈਗ_name(rec_ਟੈਗ(rec)));
	}
}

ਪੂਰਨ bloblist_reloc(ਖਾਲੀ *ਮੰਜ਼ਿਲ, uint ਮੰਜ਼ਿਲ_ਮਾਪ)
{
	ਬਣਤਰ bloblist_hdr *hdr;

	ਜੇ (!ਮੰਜ਼ਿਲ_ਮਾਪ)
		ਵਾਪਸ 0;

	ਜੇ (ਮੰਜ਼ਿਲ_ਮਾਪ < gd->bloblist->ਕੁੱਲ_ਮਾਪ)
		ਵਾਪਸ -ENOSPC;

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਮੰਜ਼ਿਲ, gd->bloblist, gd->bloblist->ਕੁੱਲ_ਮਾਪ);
	hdr = ਮੰਜ਼ਿਲ;
	hdr->ਕੁੱਲ_ਮਾਪ = ਮੰਜ਼ਿਲ_ਮਾਪ;
	gd->bloblist = ਮੰਜ਼ਿਲ;

	ਵਾਪਸ 0;
}

/*
 * Weak default function for getting bloblist from boot args.
 */
ਪੂਰਨ __weak xferlist_from_boot_arg(ulong __always_unused *addr)
{
	ਵਾਪਸ -ENOENT;
}

bool bloblist_exists(ਖਾਲੀ)
{
	ਪੂਰਨ ret;
	ulong addr = 0;

	/* Check if a valid transfer list passed in */
	ਜੇ (!xferlist_from_boot_arg(&addr))
		ਜਾਓ ਮਿਲਿਆ;

	/*
	 * If Firmware Handoff is mandatory but no transfer list is
	 * observed, report it as an error.
	 */
	ਜੇ (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		ਵਾਪਸ false;

	/*
	 * We have checked for a valid transfer list being passed. At this
	 * point, if we do not have a fixed address for the bloblist, we cannot
	 * be provided with one.
	 */
	ਜੇ (xpl_is_ਪਹਿਲਾ_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		ਵਾਪਸ false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_ਮਾਪ);
	ਜੇ (!ret)
		ਜਾਓ ਮਿਲਿਆ;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	ਵਾਪਸ false;

ਮਿਲਿਆ:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#ਜੇ_ਅੰਤ
	ਵਾਪਸ true;
}

ਪੂਰਨ bloblist_init(ਖਾਲੀ)
{
	ਪੂਰਨ ret;
	ulong addr = 0, ਮਾਪ = CONFIG_BLOBLIST_ਮਾਪ;

	ਜੇ (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->ਕੁੱਲ_ਮਾਪ, gd->bloblist);
		ਵਾਪਸ 0;
	}

	/*
	 * If Firmware Handoff is mandatory but no transfer list has been
	 * observed by fdtdec_setup, report it as an error.
	 */
	ਜੇ (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		ਵਾਪਸ -ENOENT;

	/*
	 * If we don't have an existing bloblist, we either need
	 * to allocate one now, or initialize the fixed address
	 * space as a bloblist.
	 */
	ਜੇ (CONFIG_IS_ENABLED(BLOBLIST_ALLOC)) {
		ਖਾਲੀ *ptr = memalign(BLOBLIST_ALIGN, ਮਾਪ);

		ਜੇ (!ptr)
			ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("alloc", -ENOMEM);
		addr = map_ਮੰਜ਼ਿਲ_sysmem(ptr);
	} ਨਹੀਂਤਾਂ
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", ਮਾਪ,
		  addr);
	ret = bloblist_new(addr, ਮਾਪ, 0, 0);
	ਜੇ (ret)
		ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("ini", ret);

	ਵਾਪਸ 0;
}

ਪੂਰਨ bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	ਪੂਰਨ ret;

	ਜੇ ((IS_ENABLED(CONFIG_64_ਦੋਆਧਾਰੀ_ਅੰਕ) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64_ਦੋਆਧਾਰੀ_ਅੰਕ) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_ਖਿਸਕਾਓ_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_ਨਕਾਬ) << BLOBLIST_REGCONV_ਖਿਸਕਾਓ_64));
	} ਨਹੀਂਤਾਂ {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_ਖਿਸਕਾਓ_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_ਨਕਾਬ) << BLOBLIST_REGCONV_ਖਿਸਕਾਓ_32));
	}

	ਜੇ (rzero || rsig != sigval)
		ਵਾਪਸ -EIO;

	ret = bloblist_check(xlist, 0);
	ਜੇ (ret)
		ਵਾਪਸ ret;

	ਜੇ (rfdt != (ulong)bloblist_ਲੱਭੋ(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = ਨਿਰਾ;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		ਵਾਪਸ -EIO;
	}

	ਵਾਪਸ 0;
}
