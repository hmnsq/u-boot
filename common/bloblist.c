// SPDX-License-Identifier: GPL-2.0+ BSD-3-Clause
/*
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ವ್ಯಾಖ್ಯಾನಿಸು LOG_CATEGORY	LOGC_BLOBLIST

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

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

ಸ್ಥಿರ ರಚನೆ ಟ್ಯಾಗ್_name {
	ಎಣಿಕೆ bloblist_ಟ್ಯಾಗ್_t ಟ್ಯಾಗ್;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
} ಟ್ಯಾಗ್_name[] = {
	{ BLOBLISTT_VOID, "(void)" },

	/* BLOBLISTT_AREA_FIRMWARE_TOP */
	{ BLOBLISTT_CONTROL_FDT, "Control FDT" },
	{ BLOBLISTT_HOB_ಖಂಡ, "HOB block" },
	{ BLOBLISTT_HOB_LIST, "HOB list" },
	{ BLOBLISTT_ACPI_TABLES, "ACPI tables for x86" },
	{ BLOBLISTT_TPM_EVLOG, "TPM event log defined by TCG EFI" },
	{ BLOBLISTT_TPM_CRB_ತಳ, "TPM Command Response Buffer address" },
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

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *bloblist_ಟ್ಯಾಗ್_name(ಎಣಿಕೆ bloblist_ಟ್ಯಾಗ್_t ಟ್ಯಾಗ್)
{
	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(ಟ್ಯಾಗ್_name); i++) {
		ಆದರೆ (ಟ್ಯಾಗ್_name[i].ಟ್ಯಾಗ್ == ಟ್ಯಾಗ್)
			ಹಿಂತಿರುಗಿ ಟ್ಯಾಗ್_name[i].name;
	}

	ಹಿಂತಿರುಗಿ "invalid";
}

ಸ್ಥಿರ ರಚನೆ bloblist_rec *bloblist_ಮೊದಲ_blob(ರಚನೆ bloblist_hdr *hdr)
{
	ಆದರೆ (hdr->ಬಳಸಿದ_ಗಾತ್ರ <= hdr->hdr_ಗಾತ್ರ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	ಹಿಂತಿರುಗಿ (ರಚನೆ bloblist_rec *)((ಶೂನ್ಯ *)hdr + hdr->hdr_ಗಾತ್ರ);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ uint rec_hdr_ಗಾತ್ರ(ರಚನೆ bloblist_rec *rec)
{
	ಹಿಂತಿರುಗಿ (rec->ಟ್ಯಾಗ್_and_hdr_ಗಾತ್ರ & BLOBLISTR_HDR_ಗಾತ್ರ_ಮುಖವಾಡ) >>
		BLOBLISTR_HDR_ಗಾತ್ರ_ಸರಿಸು;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ uint rec_ಟ್ಯಾಗ್(ರಚನೆ bloblist_rec *rec)
{
	ಹಿಂತಿರುಗಿ (rec->ಟ್ಯಾಗ್_and_hdr_ಗಾತ್ರ & BLOBLISTR_ಟ್ಯಾಗ್_ಮುಖವಾಡ) >>
		BLOBLISTR_ಟ್ಯಾಗ್_ಸರಿಸು;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ void_blob(ರಚನೆ bloblist_rec *rec)
{
	ಆದರೆ (rec_ಟ್ಯಾಗ್(rec) == BLOBLISTT_VOID)
		ಹಿಂತಿರುಗಿ;
	rec->ಟ್ಯಾಗ್_and_hdr_ಗಾತ್ರ = BLOBLISTT_VOID |
				ಗಾತ್ರ(*rec) << BLOBLISTR_HDR_ಗಾತ್ರ_ಸರಿಸು;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ರಚನೆ bloblist_rec *rec_from_blob(ಶೂನ್ಯ *blob)
{
	ಹಿಂತಿರುಗಿ (blob - ಗಾತ್ರ(ರಚನೆ bloblist_rec));
}

ಸ್ಥಿರ ulong bloblist_blob_ಕೊನೆ_ofs(ರಚನೆ bloblist_hdr *hdr,
				   ರಚನೆ bloblist_rec *rec)
{
	ulong ಸ್ಥಾನಾಂತರ;

	ಸ್ಥಾನಾಂತರ = (ಶೂನ್ಯ *)rec - (ಶೂನ್ಯ *)hdr;
	/*
	 * The data section of next TE should start from an address aligned
	 * to 1 << hdr->align_log2.
	 */
	ಸ್ಥಾನಾಂತರ += rec_hdr_ಗಾತ್ರ(rec) + rec->ಗಾತ್ರ_2;
	ಸ್ಥಾನಾಂತರ = ಸುತ್ತು_ಮೇಲೆ(ಸ್ಥಾನಾಂತರ + rec_hdr_ಗಾತ್ರ(rec), 1 << hdr->align_log2);
	ಸ್ಥಾನಾಂತರ -= rec_hdr_ಗಾತ್ರ(rec);

	ಹಿಂತಿರುಗಿ ಸ್ಥಾನಾಂತರ;
}

ಸ್ಥಿರ ರಚನೆ bloblist_rec *bloblist_ಮುಂದಿನ_blob(ರಚನೆ bloblist_hdr *hdr,
					       ರಚನೆ bloblist_rec *rec)
{
	ulong ಸ್ಥಾನಾಂತರ = bloblist_blob_ಕೊನೆ_ofs(hdr, rec);

	ಆದರೆ (ಸ್ಥಾನಾಂತರ >= hdr->ಬಳಸಿದ_ಗಾತ್ರ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	ಹಿಂತಿರುಗಿ (ರಚನೆ bloblist_rec *)((ಶೂನ್ಯ *)hdr + ಸ್ಥಾನಾಂತರ);
}

#ವ್ಯಾಖ್ಯಾನಿಸು foreach_rec(_rec, _hdr) \
	ಗಾಗಿ (_rec = bloblist_ಮೊದಲ_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_ಮುಂದಿನ_blob(_hdr, _rec))

ಸ್ಥಿರ ರಚನೆ bloblist_rec *bloblist_findrec(uint ಟ್ಯಾಗ್)
{
	ರಚನೆ bloblist_hdr *hdr = gd->bloblist;
	ರಚನೆ bloblist_rec *rec;

	ಆದರೆ (!hdr)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	foreach_rec(rec, hdr) {
		ಆದರೆ (rec_ಟ್ಯಾಗ್(rec) == ಟ್ಯಾಗ್)
			ಹಿಂತಿರುಗಿ rec;
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ bloblist_addrec(uint ಟ್ಯಾಗ್, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2, ಪೂರ್ಣಾಂಕ align_log2,
			   ರಚನೆ bloblist_rec **recp)
{
	ರಚನೆ bloblist_hdr *hdr = gd->bloblist;
	ರಚನೆ bloblist_rec *rec;
	phys_addr_t ದತ್ತಾಂಶ_ಆರಂಭ, aligned_ಆರಂಭ;
	phys_ಗಾತ್ರ_t new_alloced;

	ಆದರೆ (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	ದತ್ತಾಂಶ_ಆರಂಭ = map_ಗಮ್ಯ_sysmem(hdr) + hdr->ಬಳಸಿದ_ಗಾತ್ರ + ಗಾತ್ರ(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_ಆರಂಭ = ALIGN(ದತ್ತಾಂಶ_ಆರಂಭ, 1U << align_log2) - ದತ್ತಾಂಶ_ಆರಂಭ;

	/* If we need to create a dummy record, create it */
	ಆದರೆ (aligned_ಆರಂಭ) {
		ಪೂರ್ಣಾಂಕ void_ಗಾತ್ರ = aligned_ಆರಂಭ - ಗಾತ್ರ(*rec);
		ರಚನೆ bloblist_rec *vrec;
		ಪೂರ್ಣಾಂಕ ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_ಗಾತ್ರ, 0, &vrec);
		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("void", ret);

		/* start the record after that */
		ದತ್ತಾಂಶ_ಆರಂಭ = map_ಗಮ್ಯ_sysmem(hdr) + hdr->ಬಳಸಿದ_ಗಾತ್ರ + ಗಾತ್ರ(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = ದತ್ತಾಂಶ_ಆರಂಭ - map_ಗಮ್ಯ_sysmem(hdr) +
		ALIGN(ಗಾತ್ರ_2, 1U << align_log2);

	ಆದರೆ (new_alloced > hdr->ಒಟ್ಟು_ಗಾತ್ರ) {
		log_err("Failed to allocate %x bytes\n", ಗಾತ್ರ_2);
		log_err("Used size=%x, total size=%x\n",
			hdr->ಬಳಸಿದ_ಗಾತ್ರ, hdr->ಒಟ್ಟು_ಗಾತ್ರ);
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("bloblist add", -ENOSPC);
	}
	rec = (ಶೂನ್ಯ *)hdr + hdr->ಬಳಸಿದ_ಗಾತ್ರ;

	rec->ಟ್ಯಾಗ್_and_hdr_ಗಾತ್ರ = ಟ್ಯಾಗ್ | ಗಾತ್ರ(*rec) << BLOBLISTR_HDR_ಗಾತ್ರ_ಸರಿಸು;
	rec->ಗಾತ್ರ_2 = ಗಾತ್ರ_2;

	/* Zero the record data */
	memset((ಶೂನ್ಯ *)rec + rec_hdr_ಗಾತ್ರ(rec), '\0', rec->ಗಾತ್ರ_2);

	hdr->ಬಳಸಿದ_ಗಾತ್ರ = new_alloced;
	*recp = rec;

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ bloblist_ensurerec(uint ಟ್ಯಾಗ್, ರಚನೆ bloblist_rec **recp, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2,
			      ಪೂರ್ಣಾಂಕ align_log2)
{
	ರಚನೆ bloblist_rec *rec;

	rec = bloblist_findrec(ಟ್ಯಾಗ್);
	ಆದರೆ (rec) {
		ಆದರೆ (ಗಾತ್ರ_2 && ಗಾತ್ರ_2 != rec->ಗಾತ್ರ_2) {
			*recp = rec;
			ಹಿಂತಿರುಗಿ -ESPIPE;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಪೂರ್ಣಾಂಕ ret;

		ret = bloblist_addrec(ಟ್ಯಾಗ್, ಗಾತ್ರ_2, align_log2, &rec);
		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ ret;
	}
	*recp = rec;

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ bloblist_get_blob_ದತ್ತಾಂಶ_ಸ್ಥಾನಾಂತರ(uint ಟ್ಯಾಗ್)
{
	ಆಯ್ಕೆ (ಟ್ಯಾಗ್) {
	ಸ್ಥಿತಿ BLOBLISTT_FDT_OVERLAY:
		ಹಿಂತಿರುಗಿ ಗಾತ್ರ(ರಚನೆ dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ *bloblist_ಕಂಡುಹಿಡಿ(uint ಟ್ಯಾಗ್, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	ಶೂನ್ಯ *blob = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ blob_ಗಾತ್ರ;

	blob = bloblist_get_blob(ಟ್ಯಾಗ್, &blob_ಗಾತ್ರ);

	ಆದರೆ (ಗಾತ್ರ_2 && ಗಾತ್ರ_2 != blob_ಗಾತ್ರ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ blob;
}

ಶೂನ್ಯ *bloblist_get_blob(uint ಟ್ಯಾಗ್, ಪೂರ್ಣಾಂಕ *sizep)
{
	ರಚನೆ bloblist_rec *rec;

	rec = bloblist_findrec(ಟ್ಯಾಗ್);
	ಆದರೆ (!rec)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	*sizep = rec->ಗಾತ್ರ_2;

	ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ *)rec + rec_hdr_ಗಾತ್ರ(rec);
}

ಪೂರ್ಣಾಂಕ bloblist_apply_blobs(uint ಟ್ಯಾಗ್, ಪೂರ್ಣಾಂಕ (*func)(ಶೂನ್ಯ **ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2))
{
	ರಚನೆ bloblist_hdr *hdr = gd->bloblist;
	ರಚನೆ bloblist_rec *rec;

	ಆದರೆ (!func || !hdr)
		ಹಿಂತಿರುಗಿ -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		ಆದರೆ (rec_ಟ್ಯಾಗ್(rec) == ಟ್ಯಾಗ್) {
			ಪೂರ್ಣಾಂಕ ret;
			ಪೂರ್ಣಾಂಕ ಟ್ಯಾಗ್ = rec_ಟ್ಯಾಗ್(rec);
			ಶೂನ್ಯ *blob = (ಶೂನ್ಯ *)rec + rec_hdr_ಗಾತ್ರ(rec);
			ಪೂರ್ಣಾಂಕ dat_off = bloblist_get_blob_ದತ್ತಾಂಶ_ಸ್ಥಾನಾಂತರ(ಟ್ಯಾಗ್);

			blob += dat_off;
			ret = func(&blob, rec->ಗಾತ್ರ_2 - dat_off);
			ಆದರೆ (ret) {
				log_err("Failed to apply blob with tag %d\n",
					ಟ್ಯಾಗ್);
				ಹಿಂತಿರುಗಿ ret;
			}

			rec = rec_from_blob(blob - dat_off);
			ಆದರೆ (!rec) {
				log_err("Blob corrupted\n");
				ಹಿಂತಿರುಗಿ -ENOENT;
			}

			/* Mark applied blob record as void */
			void_blob(rec);
		}
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ *bloblist_ಕೂಡಿಸು(uint ಟ್ಯಾಗ್, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2, ಪೂರ್ಣಾಂಕ align_log2)
{
	ರಚನೆ bloblist_rec *rec;

	ಆದರೆ (bloblist_addrec(ಟ್ಯಾಗ್, ಗಾತ್ರ_2, align_log2, &rec))
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ *)rec + rec_hdr_ಗಾತ್ರ(rec);
}

ಪೂರ್ಣಾಂಕ bloblist_ensure_ಗಾತ್ರ(uint ಟ್ಯಾಗ್, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2, ಪೂರ್ಣಾಂಕ align_log2, ಶೂನ್ಯ **blobp)
{
	ರಚನೆ bloblist_rec *rec;
	ಪೂರ್ಣಾಂಕ ret;

	ret = bloblist_ensurerec(ಟ್ಯಾಗ್, &rec, ಗಾತ್ರ_2, align_log2);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;
	*blobp = (ಶೂನ್ಯ *)rec + rec_hdr_ಗಾತ್ರ(rec);

	ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ *bloblist_ensure(uint ಟ್ಯಾಗ್, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	ರಚನೆ bloblist_rec *rec;

	ಆದರೆ (bloblist_ensurerec(ಟ್ಯಾಗ್, &rec, ಗಾತ್ರ_2, 0))
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ *)rec + rec_hdr_ಗಾತ್ರ(rec);
}

ಪೂರ್ಣಾಂಕ bloblist_ensure_ಗಾತ್ರ_ret(uint ಟ್ಯಾಗ್, ಪೂರ್ಣಾಂಕ *sizep, ಶೂನ್ಯ **blobp)
{
	ರಚನೆ bloblist_rec *rec;
	ಪೂರ್ಣಾಂಕ ret;

	ret = bloblist_ensurerec(ಟ್ಯಾಗ್, &rec, *sizep, 0);
	ಆದರೆ (ret == -ESPIPE)
		*sizep = rec->ಗಾತ್ರ_2;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;
	*blobp = (ಶೂನ್ಯ *)rec + rec_hdr_ಗಾತ್ರ(rec);

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ bloblist_resize_rec(ರಚನೆ bloblist_hdr *hdr,
			       ರಚನೆ bloblist_rec *rec,
			       ಪೂರ್ಣಾಂಕ new_ಗಾತ್ರ)
{
	ಪೂರ್ಣಾಂಕ expand_by;	/* Number of bytes to expand by (-ve to contract) */
	ಪೂರ್ಣಾಂಕ new_alloced;
	ulong ಮುಂದಿನ_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_ಗಾತ್ರ - rec->ಗಾತ್ರ_2, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->ಬಳಸಿದ_ಗಾತ್ರ + expand_by, BLOBLIST_BLOB_ALIGN);
	ಆದರೆ (new_ಗಾತ್ರ < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_ಗಾತ್ರ);
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("size", -EINVAL);
	}
	ಆದರೆ (new_alloced > hdr->ಒಟ್ಟು_ಗಾತ್ರ) {
		log_err("Failed to allocate %x bytes\n", new_ಗಾತ್ರ);
		log_err("Used size=%x, total size=%x\n",
			hdr->ಬಳಸಿದ_ಗಾತ್ರ, hdr->ಒಟ್ಟು_ಗಾತ್ರ);
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	ಮುಂದಿನ_ofs = bloblist_blob_ಕೊನೆ_ofs(hdr, rec);
	ಆದರೆ (ಮುಂದಿನ_ofs != hdr->ಬಳಸಿದ_ಗಾತ್ರ) {
		memmove((ಶೂನ್ಯ *)hdr + ಮುಂದಿನ_ofs + expand_by,
			(ಶೂನ್ಯ *)hdr + ಮುಂದಿನ_ofs, hdr->ಬಳಸಿದ_ಗಾತ್ರ - ಮುಂದಿನ_ofs);
	}
	hdr->ಬಳಸಿದ_ಗಾತ್ರ = new_alloced;

	/* Zero the new part of the blob */
	ಆದರೆ (expand_by > 0) {
		memset((ಶೂನ್ಯ *)rec + rec_hdr_ಗಾತ್ರ(rec) + rec->ಗಾತ್ರ_2, '\0',
		       new_ಗಾತ್ರ - rec->ಗಾತ್ರ_2);
	}

	/* Update the size of this blob */
	rec->ಗಾತ್ರ_2 = new_ಗಾತ್ರ;

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ bloblist_resize(uint ಟ್ಯಾಗ್, ಪೂರ್ಣಾಂಕ new_ಗಾತ್ರ)
{
	ರಚನೆ bloblist_hdr *hdr = gd->bloblist;
	ರಚನೆ bloblist_rec *rec;
	ಪೂರ್ಣಾಂಕ ret;

	rec = bloblist_findrec(ಟ್ಯಾಗ್);
	ಆದರೆ (!rec)
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_ಗಾತ್ರ);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("resize", ret);

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ u32 bloblist_calc_chksum(ರಚನೆ bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = ಕೋಷ್ಟಕ_compute_checksum(hdr, hdr->ಬಳಸಿದ_ಗಾತ್ರ);
	chksum += hdr->chksum;

	ಹಿಂತಿರುಗಿ chksum;
}

ಪೂರ್ಣಾಂಕ bloblist_new(ulong addr, uint ಗಾತ್ರ_2, uint flags, uint align_log2)
{
	ರಚನೆ bloblist_hdr *hdr;

	ಆದರೆ (ಗಾತ್ರ_2 < ಗಾತ್ರ(*hdr))
		ಹಿಂತಿರುಗಿ log_ret(-ENOSPC);
	ಆದರೆ (addr & (BLOBLIST_ALIGN - 1))
		ಹಿಂತಿರುಗಿ log_ret(-EFAULT);
	hdr = map_sysmem(addr, ಗಾತ್ರ_2);
	memset(hdr, '\0', ಗಾತ್ರ(*hdr));
	hdr->version = BLOBLIST_ಆವೃತ್ತಿ;
	hdr->hdr_ಗಾತ್ರ = ಗಾತ್ರ(*hdr);
	hdr->flags = flags;
	hdr->magic = BLOBLIST_MAGIC;
	hdr->ಬಳಸಿದ_ಗಾತ್ರ = hdr->hdr_ಗಾತ್ರ;
	hdr->ಒಟ್ಟು_ಗಾತ್ರ = ಗಾತ್ರ_2;
	hdr->align_log2 = align_log2 ? align_log2 : BLOBLIST_BLOB_ALIGN_LOG2;
	hdr->chksum = 0;
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ bloblist_check(ulong addr, uint ಗಾತ್ರ_2)
{
	ರಚನೆ bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, ಗಾತ್ರ(*hdr));
	ಆದರೆ (hdr->magic != BLOBLIST_MAGIC)
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("Bad magic", -ENOENT);
	ಆದರೆ (hdr->version != BLOBLIST_ಆವೃತ್ತಿ)
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("Bad version", -EPROTONOSUPPORT);
	ಆದರೆ (!hdr->ಒಟ್ಟು_ಗಾತ್ರ || (ಗಾತ್ರ_2 && hdr->ಒಟ್ಟು_ಗಾತ್ರ > ಗಾತ್ರ_2))
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("Bad total size", -EFBIG);
	ಆದರೆ (hdr->ಬಳಸಿದ_ಗಾತ್ರ > hdr->ಒಟ್ಟು_ಗಾತ್ರ)
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("Bad used size", -ENOENT);
	ಆದರೆ (hdr->hdr_ಗಾತ್ರ != ಗಾತ್ರ(ರಚನೆ bloblist_hdr))
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("Bad header size", -ENOENT);

	chksum = bloblist_calc_chksum(hdr);
	ಆದರೆ (hdr->chksum != chksum) {
		log_err("Checksum %x != %x\n", hdr->chksum, chksum);
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ bloblist_ಮುಕ್ತಾಯ(ಶೂನ್ಯ)
{
	ರಚನೆ bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->ಬಳಸಿದ_ಗಾತ್ರ,
		  (ulong)map_ಗಮ್ಯ_sysmem(hdr));

	ಹಿಂತಿರುಗಿ 0;
}

ulong bloblist_get_ತಳ(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ map_ಗಮ್ಯ_sysmem(gd->bloblist);
}

ulong bloblist_get_ಗಾತ್ರ(ಶೂನ್ಯ)
{
	ರಚನೆ bloblist_hdr *hdr = gd->bloblist;

	ಹಿಂತಿರುಗಿ hdr->ಬಳಸಿದ_ಗಾತ್ರ;
}

ulong bloblist_get_ಒಟ್ಟು_ಗಾತ್ರ(ಶೂನ್ಯ)
{
	ರಚನೆ bloblist_hdr *hdr = gd->bloblist;

	ಹಿಂತಿರುಗಿ hdr->ಒಟ್ಟು_ಗಾತ್ರ;
}

ಶೂನ್ಯ bloblist_get_stats(ulong *basep, ulong *tsizep, ulong *usizep)
{
	ರಚನೆ bloblist_hdr *hdr = gd->bloblist;

	*basep = map_ಗಮ್ಯ_sysmem(gd->bloblist);
	*tsizep = hdr->ಒಟ್ಟು_ಗಾತ್ರ;
	*usizep = hdr->ಬಳಸಿದ_ಗಾತ್ರ;
}

ಸ್ಥಿರ ಶೂನ್ಯ show_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *prompt, ulong ಮೌಲ್ಯ)
{
	ಬರೆ("%s:%*s %-5lx  ", prompt, 10 - (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(prompt), "", ಮೌಲ್ಯ);
	print_ಗಾತ್ರ(ಮೌಲ್ಯ, "\n");
}

ಶೂನ್ಯ bloblist_show_stats(ಶೂನ್ಯ)
{
	ulong ತಳ, tsize, usize;

	bloblist_get_stats(&ತಳ, &tsize, &usize);
	ಬರೆ("base:       %lx\n", ತಳ);
	show_ಮೌಲ್ಯ("total size", tsize);
	show_ಮೌಲ್ಯ("used size", usize);
	show_ಮೌಲ್ಯ("free", tsize - usize);
}

ಶೂನ್ಯ bloblist_show_list(ಶೂನ್ಯ)
{
	ರಚನೆ bloblist_hdr *hdr = gd->bloblist;
	ರಚನೆ bloblist_rec *rec;

	ಬರೆ("%-8s  %8s   Tag Name\n", "Address", "Size");
	ಗಾಗಿ (rec = bloblist_ಮೊದಲ_blob(hdr); rec;
	     rec = bloblist_ಮುಂದಿನ_blob(hdr, rec)) {
		ಬರೆ("%08lx  %8x  %4x %s\n",
		       (ulong)map_ಗಮ್ಯ_sysmem((ಶೂನ್ಯ *)rec + rec_hdr_ಗಾತ್ರ(rec)),
		       rec->ಗಾತ್ರ_2, rec_ಟ್ಯಾಗ್(rec),
		       bloblist_ಟ್ಯಾಗ್_name(rec_ಟ್ಯಾಗ್(rec)));
	}
}

ಪೂರ್ಣಾಂಕ bloblist_reloc(ಶೂನ್ಯ *ಗಮ್ಯ, uint ಗಮ್ಯ_ಗಾತ್ರ)
{
	ರಚನೆ bloblist_hdr *hdr;

	ಆದರೆ (!ಗಮ್ಯ_ಗಾತ್ರ)
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (ಗಮ್ಯ_ಗಾತ್ರ < gd->bloblist->ಒಟ್ಟು_ಗಾತ್ರ)
		ಹಿಂತಿರುಗಿ -ENOSPC;

	ಸ್ಮರಣೆ_ನಕಲು(ಗಮ್ಯ, gd->bloblist, gd->bloblist->ಒಟ್ಟು_ಗಾತ್ರ);
	hdr = ಗಮ್ಯ;
	hdr->ಒಟ್ಟು_ಗಾತ್ರ = ಗಮ್ಯ_ಗಾತ್ರ;
	gd->bloblist = ಗಮ್ಯ;

	ಹಿಂತಿರುಗಿ 0;
}

/*
 * Weak default function for getting bloblist from boot args.
 */
ಪೂರ್ಣಾಂಕ __weak xferlist_from_boot_arg(ulong __always_unused *addr)
{
	ಹಿಂತಿರುಗಿ -ENOENT;
}

bool bloblist_exists(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ ret;
	ulong addr = 0;

	/* Check if a valid transfer list passed in */
	ಆದರೆ (!xferlist_from_boot_arg(&addr))
		ಹೋಗು ಸಿಕ್ಕಿತು;

	/*
	 * If Firmware Handoff is mandatory but no transfer list is
	 * observed, report it as an error.
	 */
	ಆದರೆ (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		ಹಿಂತಿರುಗಿ false;

	/*
	 * We have checked for a valid transfer list being passed. At this
	 * point, if we do not have a fixed address for the bloblist, we cannot
	 * be provided with one.
	 */
	ಆದರೆ (xpl_is_ಮೊದಲ_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		ಹಿಂತಿರುಗಿ false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_ಗಾತ್ರ);
	ಆದರೆ (!ret)
		ಹೋಗು ಸಿಕ್ಕಿತು;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	ಹಿಂತಿರುಗಿ false;

ಸಿಕ್ಕಿತು:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ true;
}

ಪೂರ್ಣಾಂಕ bloblist_init(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ ret;
	ulong addr = 0, ಗಾತ್ರ_2 = CONFIG_BLOBLIST_ಗಾತ್ರ;

	ಆದರೆ (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->ಒಟ್ಟು_ಗಾತ್ರ, gd->bloblist);
		ಹಿಂತಿರುಗಿ 0;
	}

	/*
	 * If Firmware Handoff is mandatory but no transfer list has been
	 * observed by fdtdec_setup, report it as an error.
	 */
	ಆದರೆ (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		ಹಿಂತಿರುಗಿ -ENOENT;

	/*
	 * If we don't have an existing bloblist, we either need
	 * to allocate one now, or initialize the fixed address
	 * space as a bloblist.
	 */
	ಆದರೆ (CONFIG_IS_ENABLED(BLOBLIST_ALLOC)) {
		ಶೂನ್ಯ *ptr = memalign(BLOBLIST_ALIGN, ಗಾತ್ರ_2);

		ಆದರೆ (!ptr)
			ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("alloc", -ENOMEM);
		addr = map_ಗಮ್ಯ_sysmem(ptr);
	} ಇಲ್ಲದಿದ್ದರೆ
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", ಗಾತ್ರ_2,
		  addr);
	ret = bloblist_new(addr, ಗಾತ್ರ_2, 0, 0);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("ini", ret);

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	ಪೂರ್ಣಾಂಕ ret;

	ಆದರೆ ((IS_ENABLED(CONFIG_64_ಬಿಟ್) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64_ಬಿಟ್) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_ಸರಿಸು_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_ಮುಖವಾಡ) << BLOBLIST_REGCONV_ಸರಿಸು_64));
	} ಇಲ್ಲದಿದ್ದರೆ {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_ಸರಿಸು_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_ಮುಖವಾಡ) << BLOBLIST_REGCONV_ಸರಿಸು_32));
	}

	ಆದರೆ (rzero || rsig != sigval)
		ಹಿಂತಿರುಗಿ -EIO;

	ret = bloblist_check(xlist, 0);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;

	ಆದರೆ (rfdt != (ulong)bloblist_ಕಂಡುಹಿಡಿ(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = ಖಾಲಿ;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		ಹಿಂತಿರುಗಿ -EIO;
	}

	ಹಿಂತಿರುಗಿ 0;
}
