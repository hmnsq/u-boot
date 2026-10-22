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

static struct tag_name {
	enum bloblist_tag_t tag;
	const char *name;
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

const char *bloblist_tag_name(enum bloblist_tag_t tag)
{
	цяло i;

	за (i = 0; i < ARRAY_SIZE(tag_name); i++) {
		ако (tag_name[i].tag == tag)
			върни tag_name[i].name;
	}

	върни "invalid";
}

static struct bloblist_rec *bloblist_first_blob(struct bloblist_hdr *hdr)
{
	ако (hdr->used_size <= hdr->hdr_size)
		върни NULL;
	върни (struct bloblist_rec *)((void *)hdr + hdr->hdr_size);
}

static inline uint rec_hdr_size(struct bloblist_rec *rec)
{
	върни (rec->tag_and_hdr_size & BLOBLISTR_HDR_SIZE_MASK) >>
		BLOBLISTR_HDR_SIZE_SHIFT;
}

static inline uint rec_tag(struct bloblist_rec *rec)
{
	върни (rec->tag_and_hdr_size & BLOBLISTR_TAG_MASK) >>
		BLOBLISTR_TAG_SHIFT;
}

static inline void void_blob(struct bloblist_rec *rec)
{
	ако (rec_tag(rec) == BLOBLISTT_VOID)
		върни;
	rec->tag_and_hdr_size = BLOBLISTT_VOID |
				sizeof(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
}

static inline struct bloblist_rec *rec_from_blob(void *blob)
{
	върни (blob - sizeof(struct bloblist_rec));
}

static ulong bloblist_blob_end_ofs(struct bloblist_hdr *hdr,
				   struct bloblist_rec *rec)
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

	върни offset;
}

static struct bloblist_rec *bloblist_next_blob(struct bloblist_hdr *hdr,
					       struct bloblist_rec *rec)
{
	ulong offset = bloblist_blob_end_ofs(hdr, rec);

	ако (offset >= hdr->used_size)
		върни NULL;
	върни (struct bloblist_rec *)((void *)hdr + offset);
}

#define foreach_rec(_rec, _hdr) \
	за (_rec = bloblist_first_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_next_blob(_hdr, _rec))

static struct bloblist_rec *bloblist_findrec(uint tag)
{
	struct bloblist_hdr *hdr = gd->bloblist;
	struct bloblist_rec *rec;

	ако (!hdr)
		върни NULL;

	foreach_rec(rec, hdr) {
		ако (rec_tag(rec) == tag)
			върни rec;
	}

	върни NULL;
}

static цяло bloblist_addrec(uint tag, цяло size, цяло align_log2,
			   struct bloblist_rec **recp)
{
	struct bloblist_hdr *hdr = gd->bloblist;
	struct bloblist_rec *rec;
	phys_addr_t data_start, aligned_start;
	phys_size_t new_alloced;

	ако (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	data_start = map_to_sysmem(hdr) + hdr->used_size + sizeof(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_start = ALIGN(data_start, 1U << align_log2) - data_start;

	/* If we need to create a dummy record, create it */
	ако (aligned_start) {
		цяло void_size = aligned_start - sizeof(*rec);
		struct bloblist_rec *vrec;
		цяло ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_size, 0, &vrec);
		ако (ret)
			върни log_msg_ret("void", ret);

		/* start the record after that */
		data_start = map_to_sysmem(hdr) + hdr->used_size + sizeof(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = data_start - map_to_sysmem(hdr) +
		ALIGN(size, 1U << align_log2);

	ако (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		върни log_msg_ret("bloblist add", -ENOSPC);
	}
	rec = (void *)hdr + hdr->used_size;

	rec->tag_and_hdr_size = tag | sizeof(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
	rec->size = size;

	/* Zero the record data */
	memset((void *)rec + rec_hdr_size(rec), '\0', rec->size);

	hdr->used_size = new_alloced;
	*recp = rec;

	върни 0;
}

static цяло bloblist_ensurerec(uint tag, struct bloblist_rec **recp, цяло size,
			      цяло align_log2)
{
	struct bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	ако (rec) {
		ако (size && size != rec->size) {
			*recp = rec;
			върни -ESPIPE;
		}
	} иначе {
		цяло ret;

		ret = bloblist_addrec(tag, size, align_log2, &rec);
		ако (ret)
			върни ret;
	}
	*recp = rec;

	върни 0;
}

static цяло bloblist_get_blob_data_offset(uint tag)
{
	switch (tag) {
	case BLOBLISTT_FDT_OVERLAY:
		върни sizeof(struct dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	върни 0;
}

void *bloblist_find(uint tag, цяло size)
{
	void *blob = NULL;
	цяло blob_size;

	blob = bloblist_get_blob(tag, &blob_size);

	ако (size && size != blob_size)
		върни NULL;

	върни blob;
}

void *bloblist_get_blob(uint tag, цяло *sizep)
{
	struct bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	ако (!rec)
		върни NULL;

	*sizep = rec->size;

	върни (void *)rec + rec_hdr_size(rec);
}

цяло bloblist_apply_blobs(uint tag, цяло (*func)(void **data, цяло size))
{
	struct bloblist_hdr *hdr = gd->bloblist;
	struct bloblist_rec *rec;

	ако (!func || !hdr)
		върни -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		ако (rec_tag(rec) == tag) {
			цяло ret;
			цяло tag = rec_tag(rec);
			void *blob = (void *)rec + rec_hdr_size(rec);
			цяло dat_off = bloblist_get_blob_data_offset(tag);

			blob += dat_off;
			ret = func(&blob, rec->size - dat_off);
			ако (ret) {
				log_err("Failed to apply blob with tag %d\n",
					tag);
				върни ret;
			}

			rec = rec_from_blob(blob - dat_off);
			ако (!rec) {
				log_err("Blob corrupted\n");
				върни -ENOENT;
			}

			/* Mark applied blob record as void */
			void_blob(rec);
		}
	}

	върни 0;
}

void *bloblist_add(uint tag, цяло size, цяло align_log2)
{
	struct bloblist_rec *rec;

	ако (bloblist_addrec(tag, size, align_log2, &rec))
		върни NULL;

	върни (void *)rec + rec_hdr_size(rec);
}

цяло bloblist_ensure_size(uint tag, цяло size, цяло align_log2, void **blobp)
{
	struct bloblist_rec *rec;
	цяло ret;

	ret = bloblist_ensurerec(tag, &rec, size, align_log2);
	ако (ret)
		върни ret;
	*blobp = (void *)rec + rec_hdr_size(rec);

	върни 0;
}

void *bloblist_ensure(uint tag, цяло size)
{
	struct bloblist_rec *rec;

	ако (bloblist_ensurerec(tag, &rec, size, 0))
		върни NULL;

	върни (void *)rec + rec_hdr_size(rec);
}

цяло bloblist_ensure_size_ret(uint tag, цяло *sizep, void **blobp)
{
	struct bloblist_rec *rec;
	цяло ret;

	ret = bloblist_ensurerec(tag, &rec, *sizep, 0);
	ако (ret == -ESPIPE)
		*sizep = rec->size;
	иначе ако (ret)
		върни ret;
	*blobp = (void *)rec + rec_hdr_size(rec);

	върни 0;
}

static цяло bloblist_resize_rec(struct bloblist_hdr *hdr,
			       struct bloblist_rec *rec,
			       цяло new_size)
{
	цяло expand_by;	/* Number of bytes to expand by (-ve to contract) */
	цяло new_alloced;
	ulong next_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_size - rec->size, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->used_size + expand_by, BLOBLIST_BLOB_ALIGN);
	ако (new_size < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_size);
		върни log_msg_ret("size", -EINVAL);
	}
	ако (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", new_size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		върни log_msg_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	next_ofs = bloblist_blob_end_ofs(hdr, rec);
	ако (next_ofs != hdr->used_size) {
		memmove((void *)hdr + next_ofs + expand_by,
			(void *)hdr + next_ofs, hdr->used_size - next_ofs);
	}
	hdr->used_size = new_alloced;

	/* Zero the new part of the blob */
	ако (expand_by > 0) {
		memset((void *)rec + rec_hdr_size(rec) + rec->size, '\0',
		       new_size - rec->size);
	}

	/* Update the size of this blob */
	rec->size = new_size;

	върни 0;
}

цяло bloblist_resize(uint tag, цяло new_size)
{
	struct bloblist_hdr *hdr = gd->bloblist;
	struct bloblist_rec *rec;
	цяло ret;

	rec = bloblist_findrec(tag);
	ако (!rec)
		върни log_msg_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_size);
	ако (ret)
		върни log_msg_ret("resize", ret);

	върни 0;
}

static u32 bloblist_calc_chksum(struct bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = table_compute_checksum(hdr, hdr->used_size);
	chksum += hdr->chksum;

	върни chksum;
}

цяло bloblist_new(ulong addr, uint size, uint flags, uint align_log2)
{
	struct bloblist_hdr *hdr;

	ако (size < sizeof(*hdr))
		върни log_ret(-ENOSPC);
	ако (addr & (BLOBLIST_ALIGN - 1))
		върни log_ret(-EFAULT);
	hdr = map_sysmem(addr, size);
	memset(hdr, '\0', sizeof(*hdr));
	hdr->version = BLOBLIST_VERSION;
	hdr->hdr_size = sizeof(*hdr);
	hdr->flags = flags;
	hdr->magic = BLOBLIST_MAGIC;
	hdr->used_size = hdr->hdr_size;
	hdr->total_size = size;
	hdr->align_log2 = align_log2 ? align_log2 : BLOBLIST_BLOB_ALIGN_LOG2;
	hdr->chksum = 0;
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	върни 0;
}

цяло bloblist_check(ulong addr, uint size)
{
	struct bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, sizeof(*hdr));
	ако (hdr->magic != BLOBLIST_MAGIC)
		върни log_msg_ret("Bad magic", -ENOENT);
	ако (hdr->version != BLOBLIST_VERSION)
		върни log_msg_ret("Bad version", -EPROTONOSUPPORT);
	ако (!hdr->total_size || (size && hdr->total_size > size))
		върни log_msg_ret("Bad total size", -EFBIG);
	ако (hdr->used_size > hdr->total_size)
		върни log_msg_ret("Bad used size", -ENOENT);
	ако (hdr->hdr_size != sizeof(struct bloblist_hdr))
		върни log_msg_ret("Bad header size", -ENOENT);

	chksum = bloblist_calc_chksum(hdr);
	ако (hdr->chksum != chksum) {
		log_err("Checksum %x != %x\n", hdr->chksum, chksum);
		върни log_msg_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	върни 0;
}

цяло bloblist_finish(void)
{
	struct bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->used_size,
		  (ulong)map_to_sysmem(hdr));

	върни 0;
}

ulong bloblist_get_base(void)
{
	върни map_to_sysmem(gd->bloblist);
}

ulong bloblist_get_size(void)
{
	struct bloblist_hdr *hdr = gd->bloblist;

	върни hdr->used_size;
}

ulong bloblist_get_total_size(void)
{
	struct bloblist_hdr *hdr = gd->bloblist;

	върни hdr->total_size;
}

void bloblist_get_stats(ulong *basep, ulong *tsizep, ulong *usizep)
{
	struct bloblist_hdr *hdr = gd->bloblist;

	*basep = map_to_sysmem(gd->bloblist);
	*tsizep = hdr->total_size;
	*usizep = hdr->used_size;
}

static void show_value(const char *prompt, ulong value)
{
	printf("%s:%*s %-5lx  ", prompt, 10 - (цяло)strlen(prompt), "", value);
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
	struct bloblist_hdr *hdr = gd->bloblist;
	struct bloblist_rec *rec;

	printf("%-8s  %8s   Tag Name\n", "Address", "Size");
	за (rec = bloblist_first_blob(hdr); rec;
	     rec = bloblist_next_blob(hdr, rec)) {
		printf("%08lx  %8x  %4x %s\n",
		       (ulong)map_to_sysmem((void *)rec + rec_hdr_size(rec)),
		       rec->size, rec_tag(rec),
		       bloblist_tag_name(rec_tag(rec)));
	}
}

цяло bloblist_reloc(void *to, uint to_size)
{
	struct bloblist_hdr *hdr;

	ако (!to_size)
		върни 0;

	ако (to_size < gd->bloblist->total_size)
		върни -ENOSPC;

	memcpy(to, gd->bloblist, gd->bloblist->total_size);
	hdr = to;
	hdr->total_size = to_size;
	gd->bloblist = to;

	върни 0;
}

/*
 * Weak default function for getting bloblist from boot args.
 */
цяло __weak xferlist_from_boot_arg(ulong __always_unused *addr)
{
	върни -ENOENT;
}

bool bloblist_exists(void)
{
	цяло ret;
	ulong addr = 0;

	/* Check if a valid transfer list passed in */
	ако (!xferlist_from_boot_arg(&addr))
		goto found;

	/*
	 * If Firmware Handoff is mandatory but no transfer list is
	 * observed, report it as an error.
	 */
	ако (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		върни false;

	/*
	 * We have checked for a valid transfer list being passed. At this
	 * point, if we do not have a fixed address for the bloblist, we cannot
	 * be provided with one.
	 */
	ако (xpl_is_first_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		върни false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_SIZE);
	ако (!ret)
		goto found;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	върни false;

found:
#ifdef DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#endif
	върни true;
}

цяло bloblist_init(void)
{
	цяло ret;
	ulong addr = 0, size = CONFIG_BLOBLIST_SIZE;

	ако (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->total_size, gd->bloblist);
		върни 0;
	}

	/*
	 * If Firmware Handoff is mandatory but no transfer list has been
	 * observed by fdtdec_setup, report it as an error.
	 */
	ако (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		върни -ENOENT;

	/*
	 * If we don't have an existing bloblist, we either need
	 * to allocate one now, or initialize the fixed address
	 * space as a bloblist.
	 */
	ако (CONFIG_IS_ENABLED(BLOBLIST_ALLOC)) {
		void *ptr = memalign(BLOBLIST_ALIGN, size);

		ако (!ptr)
			върни log_msg_ret("alloc", -ENOMEM);
		addr = map_to_sysmem(ptr);
	} иначе
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", size,
		  addr);
	ret = bloblist_new(addr, size, 0, 0);
	ако (ret)
		върни log_msg_ret("ini", ret);

	върни 0;
}

цяло bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	цяло ret;

	ако ((IS_ENABLED(CONFIG_64BIT) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64BIT) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_SHIFT_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_64));
	} иначе {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_SHIFT_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_32));
	}

	ако (rzero || rsig != sigval)
		върни -EIO;

	ret = bloblist_check(xlist, 0);
	ако (ret)
		върни ret;

	ако (rfdt != (ulong)bloblist_find(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = NULL;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		върни -EIO;
	}

	върни 0;
}
