// SPDX-License-Identifier: GPL-2.0+ BSD-3-Clause
/*
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_BLOBLIST

#ଅନ୍ତର୍ଭାଵ <bloblist.h>
#ଅନ୍ତର୍ଭାଵ <display_options.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <malloc.h>
#ଅନ୍ତର୍ଭାଵ <mapmem.h>
#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <tables_csum.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>
#ଅନ୍ତର୍ଭାଵ <u-boot/crc.h>

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

ସ୍ଥିର ସଂରଚନା tag_name {
	ଗଣନା bloblist_tag_t tag;
	ଅଚଳ ଅକ୍ଷର *name;
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

ଅଚଳ ଅକ୍ଷର *bloblist_tag_name(ଗଣନା bloblist_tag_t tag)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପାଇଁ (i = 0; i < ARRAY_SIZE(tag_name); i++) {
		ଯଦି (tag_name[i].tag == tag)
			ଫେରନ୍ତୁ tag_name[i].name;
	}

	ଫେରନ୍ତୁ "invalid";
}

ସ୍ଥିର ସଂରଚନା bloblist_rec *bloblist_first_blob(ସଂରଚନା bloblist_hdr *hdr)
{
	ଯଦି (hdr->used_size <= hdr->hdr_size)
		ଫେରନ୍ତୁ ଖାଲି;
	ଫେରନ୍ତୁ (ସଂରଚନା bloblist_rec *)((ଶୂନ୍ୟ *)hdr + hdr->hdr_size);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ uint rec_hdr_size(ସଂରଚନା bloblist_rec *rec)
{
	ଫେରନ୍ତୁ (rec->tag_and_hdr_size & BLOBLISTR_HDR_SIZE_MASK) >>
		BLOBLISTR_HDR_SIZE_SHIFT;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ uint rec_tag(ସଂରଚନା bloblist_rec *rec)
{
	ଫେରନ୍ତୁ (rec->tag_and_hdr_size & BLOBLISTR_TAG_MASK) >>
		BLOBLISTR_TAG_SHIFT;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ void_blob(ସଂରଚନା bloblist_rec *rec)
{
	ଯଦି (rec_tag(rec) == BLOBLISTT_VOID)
		ଫେରନ୍ତୁ;
	rec->tag_and_hdr_size = BLOBLISTT_VOID |
				ଆକାର(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ସଂରଚନା bloblist_rec *rec_from_blob(ଶୂନ୍ୟ *blob)
{
	ଫେରନ୍ତୁ (blob - ଆକାର(ସଂରଚନା bloblist_rec));
}

ସ୍ଥିର ulong bloblist_blob_end_ofs(ସଂରଚନା bloblist_hdr *hdr,
				   ସଂରଚନା bloblist_rec *rec)
{
	ulong offset;

	offset = (ଶୂନ୍ୟ *)rec - (ଶୂନ୍ୟ *)hdr;
	/*
	 * The data section of next TE should start from an address aligned
	 * to 1 << hdr->align_log2.
	 */
	offset += rec_hdr_size(rec) + rec->size;
	offset = round_up(offset + rec_hdr_size(rec), 1 << hdr->align_log2);
	offset -= rec_hdr_size(rec);

	ଫେରନ୍ତୁ offset;
}

ସ୍ଥିର ସଂରଚନା bloblist_rec *bloblist_next_blob(ସଂରଚନା bloblist_hdr *hdr,
					       ସଂରଚନା bloblist_rec *rec)
{
	ulong offset = bloblist_blob_end_ofs(hdr, rec);

	ଯଦି (offset >= hdr->used_size)
		ଫେରନ୍ତୁ ଖାଲି;
	ଫେରନ୍ତୁ (ସଂରଚନା bloblist_rec *)((ଶୂନ୍ୟ *)hdr + offset);
}

#define foreach_rec(_rec, _hdr) \
	ପାଇଁ (_rec = bloblist_first_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_next_blob(_hdr, _rec))

ସ୍ଥିର ସଂରଚନା bloblist_rec *bloblist_findrec(uint tag)
{
	ସଂରଚନା bloblist_hdr *hdr = gd->bloblist;
	ସଂରଚନା bloblist_rec *rec;

	ଯଦି (!hdr)
		ଫେରନ୍ତୁ ଖାଲି;

	foreach_rec(rec, hdr) {
		ଯଦି (rec_tag(rec) == tag)
			ଫେରନ୍ତୁ rec;
	}

	ଫେରନ୍ତୁ ଖାଲି;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା bloblist_addrec(uint tag, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size, ପୂର୍ଣ୍ଣସଂଖ୍ୟା align_log2,
			   ସଂରଚନା bloblist_rec **recp)
{
	ସଂରଚନା bloblist_hdr *hdr = gd->bloblist;
	ସଂରଚନା bloblist_rec *rec;
	phys_addr_t data_start, aligned_start;
	phys_size_t new_alloced;

	ଯଦି (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	data_start = map_to_sysmem(hdr) + hdr->used_size + ଆକାର(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_start = ALIGN(data_start, 1U << align_log2) - data_start;

	/* If we need to create a dummy record, create it */
	ଯଦି (aligned_start) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା void_size = aligned_start - ଆକାର(*rec);
		ସଂରଚନା bloblist_rec *vrec;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_size, 0, &vrec);
		ଯଦି (ret)
			ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("void", ret);

		/* start the record after that */
		data_start = map_to_sysmem(hdr) + hdr->used_size + ଆକାର(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = data_start - map_to_sysmem(hdr) +
		ALIGN(size, 1U << align_log2);

	ଯଦି (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("bloblist add", -ENOSPC);
	}
	rec = (ଶୂନ୍ୟ *)hdr + hdr->used_size;

	rec->tag_and_hdr_size = tag | ଆକାର(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
	rec->size = size;

	/* Zero the record data */
	memset((ଶୂନ୍ୟ *)rec + rec_hdr_size(rec), '\0', rec->size);

	hdr->used_size = new_alloced;
	*recp = rec;

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା bloblist_ensurerec(uint tag, ସଂରଚନା bloblist_rec **recp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size,
			      ପୂର୍ଣ୍ଣସଂଖ୍ୟା align_log2)
{
	ସଂରଚନା bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	ଯଦି (rec) {
		ଯଦି (size && size != rec->size) {
			*recp = rec;
			ଫେରନ୍ତୁ -ESPIPE;
		}
	} ଅନ୍ୟଥା {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

		ret = bloblist_addrec(tag, size, align_log2, &rec);
		ଯଦି (ret)
			ଫେରନ୍ତୁ ret;
	}
	*recp = rec;

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା bloblist_get_blob_data_offset(uint tag)
{
	ବାଛ (tag) {
	ସ୍ଥିତି BLOBLISTT_FDT_OVERLAY:
		ଫେରନ୍ତୁ ଆକାର(ସଂରଚନା dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ *bloblist_find(uint tag, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	ଶୂନ୍ୟ *blob = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା blob_size;

	blob = bloblist_get_blob(tag, &blob_size);

	ଯଦି (size && size != blob_size)
		ଫେରନ୍ତୁ ଖାଲି;

	ଫେରନ୍ତୁ blob;
}

ଶୂନ୍ୟ *bloblist_get_blob(uint tag, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *sizep)
{
	ସଂରଚନା bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	ଯଦି (!rec)
		ଫେରନ୍ତୁ ଖାଲି;

	*sizep = rec->size;

	ଫେରନ୍ତୁ (ଶୂନ୍ୟ *)rec + rec_hdr_size(rec);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bloblist_apply_blobs(uint tag, ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*func)(ଶୂନ୍ୟ **data, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size))
{
	ସଂରଚନା bloblist_hdr *hdr = gd->bloblist;
	ସଂରଚନା bloblist_rec *rec;

	ଯଦି (!func || !hdr)
		ଫେରନ୍ତୁ -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		ଯଦି (rec_tag(rec) == tag) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା tag = rec_tag(rec);
			ଶୂନ୍ୟ *blob = (ଶୂନ୍ୟ *)rec + rec_hdr_size(rec);
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା dat_off = bloblist_get_blob_data_offset(tag);

			blob += dat_off;
			ret = func(&blob, rec->size - dat_off);
			ଯଦି (ret) {
				log_err("Failed to apply blob with tag %d\n",
					tag);
				ଫେରନ୍ତୁ ret;
			}

			rec = rec_from_blob(blob - dat_off);
			ଯଦି (!rec) {
				log_err("Blob corrupted\n");
				ଫେରନ୍ତୁ -ENOENT;
			}

			/* Mark applied blob record as void */
			void_blob(rec);
		}
	}

	ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ *bloblist_add(uint tag, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size, ପୂର୍ଣ୍ଣସଂଖ୍ୟା align_log2)
{
	ସଂରଚନା bloblist_rec *rec;

	ଯଦି (bloblist_addrec(tag, size, align_log2, &rec))
		ଫେରନ୍ତୁ ଖାଲି;

	ଫେରନ୍ତୁ (ଶୂନ୍ୟ *)rec + rec_hdr_size(rec);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bloblist_ensure_size(uint tag, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size, ପୂର୍ଣ୍ଣସଂଖ୍ୟା align_log2, ଶୂନ୍ୟ **blobp)
{
	ସଂରଚନା bloblist_rec *rec;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ret = bloblist_ensurerec(tag, &rec, size, align_log2);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;
	*blobp = (ଶୂନ୍ୟ *)rec + rec_hdr_size(rec);

	ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ *bloblist_ensure(uint tag, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	ସଂରଚନା bloblist_rec *rec;

	ଯଦି (bloblist_ensurerec(tag, &rec, size, 0))
		ଫେରନ୍ତୁ ଖାଲି;

	ଫେରନ୍ତୁ (ଶୂନ୍ୟ *)rec + rec_hdr_size(rec);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bloblist_ensure_size_ret(uint tag, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *sizep, ଶୂନ୍ୟ **blobp)
{
	ସଂରଚନା bloblist_rec *rec;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ret = bloblist_ensurerec(tag, &rec, *sizep, 0);
	ଯଦି (ret == -ESPIPE)
		*sizep = rec->size;
	ଅନ୍ୟଥା ଯଦି (ret)
		ଫେରନ୍ତୁ ret;
	*blobp = (ଶୂନ୍ୟ *)rec + rec_hdr_size(rec);

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା bloblist_resize_rec(ସଂରଚନା bloblist_hdr *hdr,
			       ସଂରଚନା bloblist_rec *rec,
			       ପୂର୍ଣ୍ଣସଂଖ୍ୟା new_size)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା expand_by;	/* Number of bytes to expand by (-ve to contract) */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା new_alloced;
	ulong next_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_size - rec->size, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->used_size + expand_by, BLOBLIST_BLOB_ALIGN);
	ଯଦି (new_size < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_size);
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("size", -EINVAL);
	}
	ଯଦି (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", new_size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	next_ofs = bloblist_blob_end_ofs(hdr, rec);
	ଯଦି (next_ofs != hdr->used_size) {
		memmove((ଶୂନ୍ୟ *)hdr + next_ofs + expand_by,
			(ଶୂନ୍ୟ *)hdr + next_ofs, hdr->used_size - next_ofs);
	}
	hdr->used_size = new_alloced;

	/* Zero the new part of the blob */
	ଯଦି (expand_by > 0) {
		memset((ଶୂନ୍ୟ *)rec + rec_hdr_size(rec) + rec->size, '\0',
		       new_size - rec->size);
	}

	/* Update the size of this blob */
	rec->size = new_size;

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bloblist_resize(uint tag, ପୂର୍ଣ୍ଣସଂଖ୍ୟା new_size)
{
	ସଂରଚନା bloblist_hdr *hdr = gd->bloblist;
	ସଂରଚନା bloblist_rec *rec;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	rec = bloblist_findrec(tag);
	ଯଦି (!rec)
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_size);
	ଯଦି (ret)
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("resize", ret);

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର u32 bloblist_calc_chksum(ସଂରଚନା bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = table_compute_checksum(hdr, hdr->used_size);
	chksum += hdr->chksum;

	ଫେରନ୍ତୁ chksum;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bloblist_new(ulong addr, uint size, uint flags, uint align_log2)
{
	ସଂରଚନା bloblist_hdr *hdr;

	ଯଦି (size < ଆକାର(*hdr))
		ଫେରନ୍ତୁ log_ret(-ENOSPC);
	ଯଦି (addr & (BLOBLIST_ALIGN - 1))
		ଫେରନ୍ତୁ log_ret(-EFAULT);
	hdr = map_sysmem(addr, size);
	memset(hdr, '\0', ଆକାର(*hdr));
	hdr->version = BLOBLIST_VERSION;
	hdr->hdr_size = ଆକାର(*hdr);
	hdr->flags = flags;
	hdr->magic = BLOBLIST_MAGIC;
	hdr->used_size = hdr->hdr_size;
	hdr->total_size = size;
	hdr->align_log2 = align_log2 ? align_log2 : BLOBLIST_BLOB_ALIGN_LOG2;
	hdr->chksum = 0;
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bloblist_check(ulong addr, uint size)
{
	ସଂରଚନା bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, ଆକାର(*hdr));
	ଯଦି (hdr->magic != BLOBLIST_MAGIC)
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("Bad magic", -ENOENT);
	ଯଦି (hdr->version != BLOBLIST_VERSION)
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("Bad version", -EPROTONOSUPPORT);
	ଯଦି (!hdr->total_size || (size && hdr->total_size > size))
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("Bad total size", -EFBIG);
	ଯଦି (hdr->used_size > hdr->total_size)
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("Bad used size", -ENOENT);
	ଯଦି (hdr->hdr_size != ଆକାର(ସଂରଚନା bloblist_hdr))
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("Bad header size", -ENOENT);

	chksum = bloblist_calc_chksum(hdr);
	ଯଦି (hdr->chksum != chksum) {
		log_err("Checksum %x != %x\n", hdr->chksum, chksum);
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bloblist_finish(ଶୂନ୍ୟ)
{
	ସଂରଚନା bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->used_size,
		  (ulong)map_to_sysmem(hdr));

	ଫେରନ୍ତୁ 0;
}

ulong bloblist_get_base(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ map_to_sysmem(gd->bloblist);
}

ulong bloblist_get_size(ଶୂନ୍ୟ)
{
	ସଂରଚନା bloblist_hdr *hdr = gd->bloblist;

	ଫେରନ୍ତୁ hdr->used_size;
}

ulong bloblist_get_total_size(ଶୂନ୍ୟ)
{
	ସଂରଚନା bloblist_hdr *hdr = gd->bloblist;

	ଫେରନ୍ତୁ hdr->total_size;
}

ଶୂନ୍ୟ bloblist_get_stats(ulong *basep, ulong *tsizep, ulong *usizep)
{
	ସଂରଚନା bloblist_hdr *hdr = gd->bloblist;

	*basep = map_to_sysmem(gd->bloblist);
	*tsizep = hdr->total_size;
	*usizep = hdr->used_size;
}

ସ୍ଥିର ଶୂନ୍ୟ show_ମୂଲ୍ୟ(ଅଚଳ ଅକ୍ଷର *prompt, ulong ମୂଲ୍ୟ)
{
	printf("%s:%*s %-5lx  ", prompt, 10 - (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(prompt), "", ମୂଲ୍ୟ);
	print_size(ମୂଲ୍ୟ, "\n");
}

ଶୂନ୍ୟ bloblist_show_stats(ଶୂନ୍ୟ)
{
	ulong base, tsize, usize;

	bloblist_get_stats(&base, &tsize, &usize);
	printf("base:       %lx\n", base);
	show_ମୂଲ୍ୟ("total size", tsize);
	show_ମୂଲ୍ୟ("used size", usize);
	show_ମୂଲ୍ୟ("free", tsize - usize);
}

ଶୂନ୍ୟ bloblist_show_list(ଶୂନ୍ୟ)
{
	ସଂରଚନା bloblist_hdr *hdr = gd->bloblist;
	ସଂରଚନା bloblist_rec *rec;

	printf("%-8s  %8s   Tag Name\n", "Address", "Size");
	ପାଇଁ (rec = bloblist_first_blob(hdr); rec;
	     rec = bloblist_next_blob(hdr, rec)) {
		printf("%08lx  %8x  %4x %s\n",
		       (ulong)map_to_sysmem((ଶୂନ୍ୟ *)rec + rec_hdr_size(rec)),
		       rec->size, rec_tag(rec),
		       bloblist_tag_name(rec_tag(rec)));
	}
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bloblist_reloc(ଶୂନ୍ୟ *to, uint to_size)
{
	ସଂରଚନା bloblist_hdr *hdr;

	ଯଦି (!to_size)
		ଫେରନ୍ତୁ 0;

	ଯଦି (to_size < gd->bloblist->total_size)
		ଫେରନ୍ତୁ -ENOSPC;

	memcpy(to, gd->bloblist, gd->bloblist->total_size);
	hdr = to;
	hdr->total_size = to_size;
	gd->bloblist = to;

	ଫେରନ୍ତୁ 0;
}

/*
 * Weak default function for getting bloblist from boot args.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା __weak xferlist_from_boot_arg(ulong __always_unused *addr)
{
	ଫେରନ୍ତୁ -ENOENT;
}

bool bloblist_exists(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
	ulong addr = 0;

	/* Check if a valid transfer list passed in */
	ଯଦି (!xferlist_from_boot_arg(&addr))
		ଯାଅ found;

	/*
	 * If Firmware Handoff is mandatory but no transfer list is
	 * observed, report it as an error.
	 */
	ଯଦି (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		ଫେରନ୍ତୁ false;

	/*
	 * We have checked for a valid transfer list being passed. At this
	 * point, if we do not have a fixed address for the bloblist, we cannot
	 * be provided with one.
	 */
	ଯଦି (xpl_is_first_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		ଫେରନ୍ତୁ false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_SIZE);
	ଯଦି (!ret)
		ଯାଅ found;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	ଫେରନ୍ତୁ false;

found:
#ifdef DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#endif
	ଫେରନ୍ତୁ true;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bloblist_init(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
	ulong addr = 0, size = CONFIG_BLOBLIST_SIZE;

	ଯଦି (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->total_size, gd->bloblist);
		ଫେରନ୍ତୁ 0;
	}

	/*
	 * If Firmware Handoff is mandatory but no transfer list has been
	 * observed by fdtdec_setup, report it as an error.
	 */
	ଯଦି (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		ଫେରନ୍ତୁ -ENOENT;

	/*
	 * If we don't have an existing bloblist, we either need
	 * to allocate one now, or initialize the fixed address
	 * space as a bloblist.
	 */
	ଯଦି (CONFIG_IS_ENABLED(BLOBLIST_ALLOC)) {
		ଶୂନ୍ୟ *ptr = memalign(BLOBLIST_ALIGN, size);

		ଯଦି (!ptr)
			ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("alloc", -ENOMEM);
		addr = map_to_sysmem(ptr);
	} ଅନ୍ୟଥା
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", size,
		  addr);
	ret = bloblist_new(addr, size, 0, 0);
	ଯଦି (ret)
		ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("ini", ret);

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ଯଦି ((IS_ENABLED(CONFIG_64BIT) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64BIT) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_SHIFT_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_64));
	} ଅନ୍ୟଥା {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_SHIFT_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_32));
	}

	ଯଦି (rzero || rsig != sigval)
		ଫେରନ୍ତୁ -EIO;

	ret = bloblist_check(xlist, 0);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;

	ଯଦି (rfdt != (ulong)bloblist_find(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = ଖାଲି;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		ଫେରନ୍ତୁ -EIO;
	}

	ଫେରନ୍ତୁ 0;
}
