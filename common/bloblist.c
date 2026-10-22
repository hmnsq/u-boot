// SPDX-License-Identifier: GPL-2.0+ BSD-3-Clause
/*
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_BLOBLIST

#include <bloblist.h>
#include <display_options.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <spl.h>
#include <tables_csum.h>
#include <asm/global_data.h>
#include <u-boot/crc.h>

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

static sa_nracanaa tag_name {
	ga_nanaa bloblist_tag_t tag;
	const var_na *name;
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

const var_na *bloblist_tag_name(ga_nanaa bloblist_tag_t tag)
{
	ຈຳນວນເຕັມ i;

	ສຳລັບ (i = 0; i < ARRAY_SIZE(tag_name); i++) {
		ຖ້າ (tag_name[i].tag == tag)
			ສົ່ງຄືນ tag_name[i].name;
	}

	ສົ່ງຄືນ "invalid";
}

static sa_nracanaa bloblist_rec *bloblist_first_blob(sa_nracanaa bloblist_hdr *hdr)
{
	ຖ້າ (hdr->used_size <= hdr->hdr_size)
		ສົ່ງຄືນ NULL;
	ສົ່ງຄືນ (sa_nracanaa bloblist_rec *)((void *)hdr + hdr->hdr_size);
}

static anta_hstha uint rec_hdr_size(sa_nracanaa bloblist_rec *rec)
{
	ສົ່ງຄືນ (rec->tag_and_hdr_size & BLOBLISTR_HDR_SIZE_MASK) >>
		BLOBLISTR_HDR_SIZE_SHIFT;
}

static anta_hstha uint rec_tag(sa_nracanaa bloblist_rec *rec)
{
	ສົ່ງຄືນ (rec->tag_and_hdr_size & BLOBLISTR_TAG_MASK) >>
		BLOBLISTR_TAG_SHIFT;
}

static anta_hstha void void_blob(sa_nracanaa bloblist_rec *rec)
{
	ຖ້າ (rec_tag(rec) == BLOBLISTT_VOID)
		ສົ່ງຄືນ;
	rec->tag_and_hdr_size = BLOBLISTT_VOID |
				parimaa_na(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
}

static anta_hstha sa_nracanaa bloblist_rec *rec_from_blob(void *blob)
{
	ສົ່ງຄືນ (blob - parimaa_na(sa_nracanaa bloblist_rec));
}

static ulong bloblist_blob_end_ofs(sa_nracanaa bloblist_hdr *hdr,
				   sa_nracanaa bloblist_rec *rec)
{
	ulong offset;

	offset = (void *)rec - (void *)hdr;
	/*
	 * The data section of next TE should start from an address aligned
	 * to 1 << hdr->align_log2.
	 */
	offset += rec_hdr_size(rec) + rec->size;
	offset = round_up(offset + rec_hdr_size(rec), 1 << hdr->align_log2);
	offset -= rec_hdr_size(rec);

	ສົ່ງຄືນ offset;
}

static sa_nracanaa bloblist_rec *bloblist_next_blob(sa_nracanaa bloblist_hdr *hdr,
					       sa_nracanaa bloblist_rec *rec)
{
	ulong offset = bloblist_blob_end_ofs(hdr, rec);

	ຖ້າ (offset >= hdr->used_size)
		ສົ່ງຄືນ NULL;
	ສົ່ງຄືນ (sa_nracanaa bloblist_rec *)((void *)hdr + offset);
}

#define foreach_rec(_rec, _hdr) \
	ສຳລັບ (_rec = bloblist_first_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_next_blob(_hdr, _rec))

static sa_nracanaa bloblist_rec *bloblist_findrec(uint tag)
{
	sa_nracanaa bloblist_hdr *hdr = gd->bloblist;
	sa_nracanaa bloblist_rec *rec;

	ຖ້າ (!hdr)
		ສົ່ງຄືນ NULL;

	foreach_rec(rec, hdr) {
		ຖ້າ (rec_tag(rec) == tag)
			ສົ່ງຄືນ rec;
	}

	ສົ່ງຄືນ NULL;
}

static ຈຳນວນເຕັມ bloblist_addrec(uint tag, ຈຳນວນເຕັມ size, ຈຳນວນເຕັມ align_log2,
			   sa_nracanaa bloblist_rec **recp)
{
	sa_nracanaa bloblist_hdr *hdr = gd->bloblist;
	sa_nracanaa bloblist_rec *rec;
	phys_addr_t data_start, aligned_start;
	phys_size_t new_alloced;

	ຖ້າ (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	data_start = map_to_sysmem(hdr) + hdr->used_size + parimaa_na(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_start = ALIGN(data_start, 1U << align_log2) - data_start;

	/* If we need to create a dummy record, create it */
	ຖ້າ (aligned_start) {
		ຈຳນວນເຕັມ void_size = aligned_start - parimaa_na(*rec);
		sa_nracanaa bloblist_rec *vrec;
		ຈຳນວນເຕັມ ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_size, 0, &vrec);
		ຖ້າ (ret)
			ສົ່ງຄືນ log_msg_ret("void", ret);

		/* start the record after that */
		data_start = map_to_sysmem(hdr) + hdr->used_size + parimaa_na(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = data_start - map_to_sysmem(hdr) +
		ALIGN(size, 1U << align_log2);

	ຖ້າ (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		ສົ່ງຄືນ log_msg_ret("bloblist add", -ENOSPC);
	}
	rec = (void *)hdr + hdr->used_size;

	rec->tag_and_hdr_size = tag | parimaa_na(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
	rec->size = size;

	/* Zero the record data */
	memset((void *)rec + rec_hdr_size(rec), '\0', rec->size);

	hdr->used_size = new_alloced;
	*recp = rec;

	ສົ່ງຄືນ 0;
}

static ຈຳນວນເຕັມ bloblist_ensurerec(uint tag, sa_nracanaa bloblist_rec **recp, ຈຳນວນເຕັມ size,
			      ຈຳນວນເຕັມ align_log2)
{
	sa_nracanaa bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	ຖ້າ (rec) {
		ຖ້າ (size && size != rec->size) {
			*recp = rec;
			ສົ່ງຄືນ -ESPIPE;
		}
	} ບໍ່ດັ່ງນັ້ນ {
		ຈຳນວນເຕັມ ret;

		ret = bloblist_addrec(tag, size, align_log2, &rec);
		ຖ້າ (ret)
			ສົ່ງຄືນ ret;
	}
	*recp = rec;

	ສົ່ງຄືນ 0;
}

static ຈຳນວນເຕັມ bloblist_get_blob_data_offset(uint tag)
{
	switch (tag) {
	pak_sa BLOBLISTT_FDT_OVERLAY:
		ສົ່ງຄືນ parimaa_na(sa_nracanaa dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	ສົ່ງຄືນ 0;
}

void *bloblist_find(uint tag, ຈຳນວນເຕັມ size)
{
	void *blob = NULL;
	ຈຳນວນເຕັມ blob_size;

	blob = bloblist_get_blob(tag, &blob_size);

	ຖ້າ (size && size != blob_size)
		ສົ່ງຄືນ NULL;

	ສົ່ງຄືນ blob;
}

void *bloblist_get_blob(uint tag, ຈຳນວນເຕັມ *sizep)
{
	sa_nracanaa bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	ຖ້າ (!rec)
		ສົ່ງຄືນ NULL;

	*sizep = rec->size;

	ສົ່ງຄືນ (void *)rec + rec_hdr_size(rec);
}

ຈຳນວນເຕັມ bloblist_apply_blobs(uint tag, ຈຳນວນເຕັມ (*func)(void **data, ຈຳນວນເຕັມ size))
{
	sa_nracanaa bloblist_hdr *hdr = gd->bloblist;
	sa_nracanaa bloblist_rec *rec;

	ຖ້າ (!func || !hdr)
		ສົ່ງຄືນ -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		ຖ້າ (rec_tag(rec) == tag) {
			ຈຳນວນເຕັມ ret;
			ຈຳນວນເຕັມ tag = rec_tag(rec);
			void *blob = (void *)rec + rec_hdr_size(rec);
			ຈຳນວນເຕັມ dat_off = bloblist_get_blob_data_offset(tag);

			blob += dat_off;
			ret = func(&blob, rec->size - dat_off);
			ຖ້າ (ret) {
				log_err("Failed to apply blob with tag %d\n",
					tag);
				ສົ່ງຄືນ ret;
			}

			rec = rec_from_blob(blob - dat_off);
			ຖ້າ (!rec) {
				log_err("Blob corrupted\n");
				ສົ່ງຄືນ -ENOENT;
			}

			/* Mark applied blob record as void */
			void_blob(rec);
		}
	}

	ສົ່ງຄືນ 0;
}

void *bloblist_add(uint tag, ຈຳນວນເຕັມ size, ຈຳນວນເຕັມ align_log2)
{
	sa_nracanaa bloblist_rec *rec;

	ຖ້າ (bloblist_addrec(tag, size, align_log2, &rec))
		ສົ່ງຄືນ NULL;

	ສົ່ງຄືນ (void *)rec + rec_hdr_size(rec);
}

ຈຳນວນເຕັມ bloblist_ensure_size(uint tag, ຈຳນວນເຕັມ size, ຈຳນວນເຕັມ align_log2, void **blobp)
{
	sa_nracanaa bloblist_rec *rec;
	ຈຳນວນເຕັມ ret;

	ret = bloblist_ensurerec(tag, &rec, size, align_log2);
	ຖ້າ (ret)
		ສົ່ງຄືນ ret;
	*blobp = (void *)rec + rec_hdr_size(rec);

	ສົ່ງຄືນ 0;
}

void *bloblist_ensure(uint tag, ຈຳນວນເຕັມ size)
{
	sa_nracanaa bloblist_rec *rec;

	ຖ້າ (bloblist_ensurerec(tag, &rec, size, 0))
		ສົ່ງຄືນ NULL;

	ສົ່ງຄືນ (void *)rec + rec_hdr_size(rec);
}

ຈຳນວນເຕັມ bloblist_ensure_size_ret(uint tag, ຈຳນວນເຕັມ *sizep, void **blobp)
{
	sa_nracanaa bloblist_rec *rec;
	ຈຳນວນເຕັມ ret;

	ret = bloblist_ensurerec(tag, &rec, *sizep, 0);
	ຖ້າ (ret == -ESPIPE)
		*sizep = rec->size;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (ret)
		ສົ່ງຄືນ ret;
	*blobp = (void *)rec + rec_hdr_size(rec);

	ສົ່ງຄືນ 0;
}

static ຈຳນວນເຕັມ bloblist_resize_rec(sa_nracanaa bloblist_hdr *hdr,
			       sa_nracanaa bloblist_rec *rec,
			       ຈຳນວນເຕັມ new_size)
{
	ຈຳນວນເຕັມ expand_by;	/* Number of bytes to expand by (-ve to contract) */
	ຈຳນວນເຕັມ new_alloced;
	ulong next_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_size - rec->size, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->used_size + expand_by, BLOBLIST_BLOB_ALIGN);
	ຖ້າ (new_size < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_size);
		ສົ່ງຄືນ log_msg_ret("size", -EINVAL);
	}
	ຖ້າ (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", new_size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		ສົ່ງຄືນ log_msg_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	next_ofs = bloblist_blob_end_ofs(hdr, rec);
	ຖ້າ (next_ofs != hdr->used_size) {
		memmove((void *)hdr + next_ofs + expand_by,
			(void *)hdr + next_ofs, hdr->used_size - next_ofs);
	}
	hdr->used_size = new_alloced;

	/* Zero the new part of the blob */
	ຖ້າ (expand_by > 0) {
		memset((void *)rec + rec_hdr_size(rec) + rec->size, '\0',
		       new_size - rec->size);
	}

	/* Update the size of this blob */
	rec->size = new_size;

	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ bloblist_resize(uint tag, ຈຳນວນເຕັມ new_size)
{
	sa_nracanaa bloblist_hdr *hdr = gd->bloblist;
	sa_nracanaa bloblist_rec *rec;
	ຈຳນວນເຕັມ ret;

	rec = bloblist_findrec(tag);
	ຖ້າ (!rec)
		ສົ່ງຄືນ log_msg_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_size);
	ຖ້າ (ret)
		ສົ່ງຄືນ log_msg_ret("resize", ret);

	ສົ່ງຄືນ 0;
}

static u32 bloblist_calc_chksum(sa_nracanaa bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = table_compute_checksum(hdr, hdr->used_size);
	chksum += hdr->chksum;

	ສົ່ງຄືນ chksum;
}

ຈຳນວນເຕັມ bloblist_new(ulong addr, uint size, uint flags, uint align_log2)
{
	sa_nracanaa bloblist_hdr *hdr;

	ຖ້າ (size < parimaa_na(*hdr))
		ສົ່ງຄືນ log_ret(-ENOSPC);
	ຖ້າ (addr & (BLOBLIST_ALIGN - 1))
		ສົ່ງຄືນ log_ret(-EFAULT);
	hdr = map_sysmem(addr, size);
	memset(hdr, '\0', parimaa_na(*hdr));
	hdr->version = BLOBLIST_VERSION;
	hdr->hdr_size = parimaa_na(*hdr);
	hdr->flags = flags;
	hdr->magic = BLOBLIST_MAGIC;
	hdr->used_size = hdr->hdr_size;
	hdr->total_size = size;
	hdr->align_log2 = align_log2 ? align_log2 : BLOBLIST_BLOB_ALIGN_LOG2;
	hdr->chksum = 0;
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ bloblist_check(ulong addr, uint size)
{
	sa_nracanaa bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, parimaa_na(*hdr));
	ຖ້າ (hdr->magic != BLOBLIST_MAGIC)
		ສົ່ງຄືນ log_msg_ret("Bad magic", -ENOENT);
	ຖ້າ (hdr->version != BLOBLIST_VERSION)
		ສົ່ງຄືນ log_msg_ret("Bad version", -EPROTONOSUPPORT);
	ຖ້າ (!hdr->total_size || (size && hdr->total_size > size))
		ສົ່ງຄືນ log_msg_ret("Bad total size", -EFBIG);
	ຖ້າ (hdr->used_size > hdr->total_size)
		ສົ່ງຄືນ log_msg_ret("Bad used size", -ENOENT);
	ຖ້າ (hdr->hdr_size != parimaa_na(sa_nracanaa bloblist_hdr))
		ສົ່ງຄືນ log_msg_ret("Bad header size", -ENOENT);

	chksum = bloblist_calc_chksum(hdr);
	ຖ້າ (hdr->chksum != chksum) {
		log_err("Checksum %x != %x\n", hdr->chksum, chksum);
		ສົ່ງຄືນ log_msg_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ bloblist_finish(void)
{
	sa_nracanaa bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->used_size,
		  (ulong)map_to_sysmem(hdr));

	ສົ່ງຄືນ 0;
}

ulong bloblist_get_base(void)
{
	ສົ່ງຄືນ map_to_sysmem(gd->bloblist);
}

ulong bloblist_get_size(void)
{
	sa_nracanaa bloblist_hdr *hdr = gd->bloblist;

	ສົ່ງຄືນ hdr->used_size;
}

ulong bloblist_get_total_size(void)
{
	sa_nracanaa bloblist_hdr *hdr = gd->bloblist;

	ສົ່ງຄືນ hdr->total_size;
}

void bloblist_get_stats(ulong *basep, ulong *tsizep, ulong *usizep)
{
	sa_nracanaa bloblist_hdr *hdr = gd->bloblist;

	*basep = map_to_sysmem(gd->bloblist);
	*tsizep = hdr->total_size;
	*usizep = hdr->used_size;
}

static void show_value(const var_na *prompt, ulong value)
{
	printf("%s:%*s %-5lx  ", prompt, 10 - (ຈຳນວນເຕັມ)strlen(prompt), "", value);
	print_size(value, "\n");
}

void bloblist_show_stats(void)
{
	ulong base, tsize, usize;

	bloblist_get_stats(&base, &tsize, &usize);
	printf("base:       %lx\n", base);
	show_value("total size", tsize);
	show_value("used size", usize);
	show_value("free", tsize - usize);
}

void bloblist_show_list(void)
{
	sa_nracanaa bloblist_hdr *hdr = gd->bloblist;
	sa_nracanaa bloblist_rec *rec;

	printf("%-8s  %8s   Tag Name\n", "Address", "Size");
	ສຳລັບ (rec = bloblist_first_blob(hdr); rec;
	     rec = bloblist_next_blob(hdr, rec)) {
		printf("%08lx  %8x  %4x %s\n",
		       (ulong)map_to_sysmem((void *)rec + rec_hdr_size(rec)),
		       rec->size, rec_tag(rec),
		       bloblist_tag_name(rec_tag(rec)));
	}
}

ຈຳນວນເຕັມ bloblist_reloc(void *to, uint to_size)
{
	sa_nracanaa bloblist_hdr *hdr;

	ຖ້າ (!to_size)
		ສົ່ງຄືນ 0;

	ຖ້າ (to_size < gd->bloblist->total_size)
		ສົ່ງຄືນ -ENOSPC;

	memcpy(to, gd->bloblist, gd->bloblist->total_size);
	hdr = to;
	hdr->total_size = to_size;
	gd->bloblist = to;

	ສົ່ງຄືນ 0;
}

/*
 * Weak default function for getting bloblist from boot args.
 */
ຈຳນວນເຕັມ __weak xferlist_from_boot_arg(ulong __always_unused *addr)
{
	ສົ່ງຄືນ -ENOENT;
}

bool bloblist_exists(void)
{
	ຈຳນວນເຕັມ ret;
	ulong addr = 0;

	/* Check if a valid transfer list passed in */
	ຖ້າ (!xferlist_from_boot_arg(&addr))
		goto found;

	/*
	 * If Firmware Handoff is mandatory but no transfer list is
	 * observed, report it as an error.
	 */
	ຖ້າ (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		ສົ່ງຄືນ false;

	/*
	 * We have checked for a valid transfer list being passed. At this
	 * point, if we do not have a fixed address for the bloblist, we cannot
	 * be provided with one.
	 */
	ຖ້າ (xpl_is_first_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		ສົ່ງຄືນ false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_SIZE);
	ຖ້າ (!ret)
		goto found;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	ສົ່ງຄືນ false;

found:
#ifdef DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#endif
	ສົ່ງຄືນ true;
}

ຈຳນວນເຕັມ bloblist_init(void)
{
	ຈຳນວນເຕັມ ret;
	ulong addr = 0, size = CONFIG_BLOBLIST_SIZE;

	ຖ້າ (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->total_size, gd->bloblist);
		ສົ່ງຄືນ 0;
	}

	/*
	 * If Firmware Handoff is mandatory but no transfer list has been
	 * observed by fdtdec_setup, report it as an error.
	 */
	ຖ້າ (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		ສົ່ງຄືນ -ENOENT;

	/*
	 * If we don't have an existing bloblist, we either need
	 * to allocate one now, or initialize the fixed address
	 * space as a bloblist.
	 */
	ຖ້າ (CONFIG_IS_ENABLED(BLOBLIST_ALLOC)) {
		void *ptr = memalign(BLOBLIST_ALIGN, size);

		ຖ້າ (!ptr)
			ສົ່ງຄືນ log_msg_ret("alloc", -ENOMEM);
		addr = map_to_sysmem(ptr);
	} ບໍ່ດັ່ງນັ້ນ
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", size,
		  addr);
	ret = bloblist_new(addr, size, 0, 0);
	ຖ້າ (ret)
		ສົ່ງຄືນ log_msg_ret("ini", ret);

	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	ຈຳນວນເຕັມ ret;

	ຖ້າ ((IS_ENABLED(CONFIG_64BIT) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64BIT) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_SHIFT_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_64));
	} ບໍ່ດັ່ງນັ້ນ {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_SHIFT_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_32));
	}

	ຖ້າ (rzero || rsig != sigval)
		ສົ່ງຄືນ -EIO;

	ret = bloblist_check(xlist, 0);
	ຖ້າ (ret)
		ສົ່ງຄືນ ret;

	ຖ້າ (rfdt != (ulong)bloblist_find(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = NULL;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		ສົ່ງຄືນ -EIO;
	}

	ສົ່ງຄືນ 0;
}
