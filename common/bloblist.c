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
	整数 i;

	为 (i = 0; i < ARRAY_SIZE(tag_name); i++) {
		如果 (tag_name[i].tag == tag)
			返回 tag_name[i].name;
	}

	返回 "invalid";
}

static struct bloblist_rec *bloblist_first_blob(struct bloblist_hdr *hdr)
{
	如果 (hdr->used_size <= hdr->hdr_size)
		返回 NULL;
	返回 (struct bloblist_rec *)((void *)hdr + hdr->hdr_size);
}

static inline uint rec_hdr_size(struct bloblist_rec *rec)
{
	返回 (rec->tag_and_hdr_size & BLOBLISTR_HDR_SIZE_MASK) >>
		BLOBLISTR_HDR_SIZE_SHIFT;
}

static inline uint rec_tag(struct bloblist_rec *rec)
{
	返回 (rec->tag_and_hdr_size & BLOBLISTR_TAG_MASK) >>
		BLOBLISTR_TAG_SHIFT;
}

static inline void void_blob(struct bloblist_rec *rec)
{
	如果 (rec_tag(rec) == BLOBLISTT_VOID)
		返回;
	rec->tag_and_hdr_size = BLOBLISTT_VOID |
				sizeof(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
}

static inline struct bloblist_rec *rec_from_blob(void *blob)
{
	返回 (blob - sizeof(struct bloblist_rec));
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

	返回 offset;
}

static struct bloblist_rec *bloblist_next_blob(struct bloblist_hdr *hdr,
					       struct bloblist_rec *rec)
{
	ulong offset = bloblist_blob_end_ofs(hdr, rec);

	如果 (offset >= hdr->used_size)
		返回 NULL;
	返回 (struct bloblist_rec *)((void *)hdr + offset);
}

#define foreach_rec(_rec, _hdr) \
	为 (_rec = bloblist_first_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_next_blob(_hdr, _rec))

static struct bloblist_rec *bloblist_findrec(uint tag)
{
	struct bloblist_hdr *hdr = gd->bloblist;
	struct bloblist_rec *rec;

	如果 (!hdr)
		返回 NULL;

	foreach_rec(rec, hdr) {
		如果 (rec_tag(rec) == tag)
			返回 rec;
	}

	返回 NULL;
}

static 整数 bloblist_addrec(uint tag, 整数 size, 整数 align_log2,
			   struct bloblist_rec **recp)
{
	struct bloblist_hdr *hdr = gd->bloblist;
	struct bloblist_rec *rec;
	phys_addr_t data_start, aligned_start;
	phys_size_t new_alloced;

	如果 (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	data_start = map_to_sysmem(hdr) + hdr->used_size + sizeof(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_start = ALIGN(data_start, 1U << align_log2) - data_start;

	/* If we need to create a dummy record, create it */
	如果 (aligned_start) {
		整数 void_size = aligned_start - sizeof(*rec);
		struct bloblist_rec *vrec;
		整数 ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_size, 0, &vrec);
		如果 (ret)
			返回 log_msg_ret("void", ret);

		/* start the record after that */
		data_start = map_to_sysmem(hdr) + hdr->used_size + sizeof(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = data_start - map_to_sysmem(hdr) +
		ALIGN(size, 1U << align_log2);

	如果 (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		返回 log_msg_ret("bloblist add", -ENOSPC);
	}
	rec = (void *)hdr + hdr->used_size;

	rec->tag_and_hdr_size = tag | sizeof(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
	rec->size = size;

	/* Zero the record data */
	memset((void *)rec + rec_hdr_size(rec), '\0', rec->size);

	hdr->used_size = new_alloced;
	*recp = rec;

	返回 0;
}

static 整数 bloblist_ensurerec(uint tag, struct bloblist_rec **recp, 整数 size,
			      整数 align_log2)
{
	struct bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	如果 (rec) {
		如果 (size && size != rec->size) {
			*recp = rec;
			返回 -ESPIPE;
		}
	} 否则 {
		整数 ret;

		ret = bloblist_addrec(tag, size, align_log2, &rec);
		如果 (ret)
			返回 ret;
	}
	*recp = rec;

	返回 0;
}

static 整数 bloblist_get_blob_data_offset(uint tag)
{
	switch (tag) {
	case BLOBLISTT_FDT_OVERLAY:
		返回 sizeof(struct dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	返回 0;
}

void *bloblist_find(uint tag, 整数 size)
{
	void *blob = NULL;
	整数 blob_size;

	blob = bloblist_get_blob(tag, &blob_size);

	如果 (size && size != blob_size)
		返回 NULL;

	返回 blob;
}

void *bloblist_get_blob(uint tag, 整数 *sizep)
{
	struct bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	如果 (!rec)
		返回 NULL;

	*sizep = rec->size;

	返回 (void *)rec + rec_hdr_size(rec);
}

整数 bloblist_apply_blobs(uint tag, 整数 (*func)(void **data, 整数 size))
{
	struct bloblist_hdr *hdr = gd->bloblist;
	struct bloblist_rec *rec;

	如果 (!func || !hdr)
		返回 -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		如果 (rec_tag(rec) == tag) {
			整数 ret;
			整数 tag = rec_tag(rec);
			void *blob = (void *)rec + rec_hdr_size(rec);
			整数 dat_off = bloblist_get_blob_data_offset(tag);

			blob += dat_off;
			ret = func(&blob, rec->size - dat_off);
			如果 (ret) {
				log_err("Failed to apply blob with tag %d\n",
					tag);
				返回 ret;
			}

			rec = rec_from_blob(blob - dat_off);
			如果 (!rec) {
				log_err("Blob corrupted\n");
				返回 -ENOENT;
			}

			/* Mark applied blob record as void */
			void_blob(rec);
		}
	}

	返回 0;
}

void *bloblist_add(uint tag, 整数 size, 整数 align_log2)
{
	struct bloblist_rec *rec;

	如果 (bloblist_addrec(tag, size, align_log2, &rec))
		返回 NULL;

	返回 (void *)rec + rec_hdr_size(rec);
}

整数 bloblist_ensure_size(uint tag, 整数 size, 整数 align_log2, void **blobp)
{
	struct bloblist_rec *rec;
	整数 ret;

	ret = bloblist_ensurerec(tag, &rec, size, align_log2);
	如果 (ret)
		返回 ret;
	*blobp = (void *)rec + rec_hdr_size(rec);

	返回 0;
}

void *bloblist_ensure(uint tag, 整数 size)
{
	struct bloblist_rec *rec;

	如果 (bloblist_ensurerec(tag, &rec, size, 0))
		返回 NULL;

	返回 (void *)rec + rec_hdr_size(rec);
}

整数 bloblist_ensure_size_ret(uint tag, 整数 *sizep, void **blobp)
{
	struct bloblist_rec *rec;
	整数 ret;

	ret = bloblist_ensurerec(tag, &rec, *sizep, 0);
	如果 (ret == -ESPIPE)
		*sizep = rec->size;
	否则 如果 (ret)
		返回 ret;
	*blobp = (void *)rec + rec_hdr_size(rec);

	返回 0;
}

static 整数 bloblist_resize_rec(struct bloblist_hdr *hdr,
			       struct bloblist_rec *rec,
			       整数 new_size)
{
	整数 expand_by;	/* Number of bytes to expand by (-ve to contract) */
	整数 new_alloced;
	ulong next_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_size - rec->size, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->used_size + expand_by, BLOBLIST_BLOB_ALIGN);
	如果 (new_size < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_size);
		返回 log_msg_ret("size", -EINVAL);
	}
	如果 (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", new_size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		返回 log_msg_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	next_ofs = bloblist_blob_end_ofs(hdr, rec);
	如果 (next_ofs != hdr->used_size) {
		memmove((void *)hdr + next_ofs + expand_by,
			(void *)hdr + next_ofs, hdr->used_size - next_ofs);
	}
	hdr->used_size = new_alloced;

	/* Zero the new part of the blob */
	如果 (expand_by > 0) {
		memset((void *)rec + rec_hdr_size(rec) + rec->size, '\0',
		       new_size - rec->size);
	}

	/* Update the size of this blob */
	rec->size = new_size;

	返回 0;
}

整数 bloblist_resize(uint tag, 整数 new_size)
{
	struct bloblist_hdr *hdr = gd->bloblist;
	struct bloblist_rec *rec;
	整数 ret;

	rec = bloblist_findrec(tag);
	如果 (!rec)
		返回 log_msg_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_size);
	如果 (ret)
		返回 log_msg_ret("resize", ret);

	返回 0;
}

static u32 bloblist_calc_chksum(struct bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = table_compute_checksum(hdr, hdr->used_size);
	chksum += hdr->chksum;

	返回 chksum;
}

整数 bloblist_new(ulong addr, uint size, uint flags, uint align_log2)
{
	struct bloblist_hdr *hdr;

	如果 (size < sizeof(*hdr))
		返回 log_ret(-ENOSPC);
	如果 (addr & (BLOBLIST_ALIGN - 1))
		返回 log_ret(-EFAULT);
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

	返回 0;
}

整数 bloblist_check(ulong addr, uint size)
{
	struct bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, sizeof(*hdr));
	如果 (hdr->magic != BLOBLIST_MAGIC)
		返回 log_msg_ret("Bad magic", -ENOENT);
	如果 (hdr->version != BLOBLIST_VERSION)
		返回 log_msg_ret("Bad version", -EPROTONOSUPPORT);
	如果 (!hdr->total_size || (size && hdr->total_size > size))
		返回 log_msg_ret("Bad total size", -EFBIG);
	如果 (hdr->used_size > hdr->total_size)
		返回 log_msg_ret("Bad used size", -ENOENT);
	如果 (hdr->hdr_size != sizeof(struct bloblist_hdr))
		返回 log_msg_ret("Bad header size", -ENOENT);

	chksum = bloblist_calc_chksum(hdr);
	如果 (hdr->chksum != chksum) {
		log_err("Checksum %x != %x\n", hdr->chksum, chksum);
		返回 log_msg_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	返回 0;
}

整数 bloblist_finish(void)
{
	struct bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->used_size,
		  (ulong)map_to_sysmem(hdr));

	返回 0;
}

ulong bloblist_get_base(void)
{
	返回 map_to_sysmem(gd->bloblist);
}

ulong bloblist_get_size(void)
{
	struct bloblist_hdr *hdr = gd->bloblist;

	返回 hdr->used_size;
}

ulong bloblist_get_total_size(void)
{
	struct bloblist_hdr *hdr = gd->bloblist;

	返回 hdr->total_size;
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
	printf("%s:%*s %-5lx  ", prompt, 10 - (整数)strlen(prompt), "", value);
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
	为 (rec = bloblist_first_blob(hdr); rec;
	     rec = bloblist_next_blob(hdr, rec)) {
		printf("%08lx  %8x  %4x %s\n",
		       (ulong)map_to_sysmem((void *)rec + rec_hdr_size(rec)),
		       rec->size, rec_tag(rec),
		       bloblist_tag_name(rec_tag(rec)));
	}
}

整数 bloblist_reloc(void *to, uint to_size)
{
	struct bloblist_hdr *hdr;

	如果 (!to_size)
		返回 0;

	如果 (to_size < gd->bloblist->total_size)
		返回 -ENOSPC;

	memcpy(to, gd->bloblist, gd->bloblist->total_size);
	hdr = to;
	hdr->total_size = to_size;
	gd->bloblist = to;

	返回 0;
}

/*
 * Weak default function for getting bloblist from boot args.
 */
整数 __weak xferlist_from_boot_arg(ulong __always_unused *addr)
{
	返回 -ENOENT;
}

bool bloblist_exists(void)
{
	整数 ret;
	ulong addr = 0;

	/* Check if a valid transfer list passed in */
	如果 (!xferlist_from_boot_arg(&addr))
		goto found;

	/*
	 * If Firmware Handoff is mandatory but no transfer list is
	 * observed, report it as an error.
	 */
	如果 (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		返回 false;

	/*
	 * We have checked for a valid transfer list being passed. At this
	 * point, if we do not have a fixed address for the bloblist, we cannot
	 * be provided with one.
	 */
	如果 (xpl_is_first_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		返回 false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_SIZE);
	如果 (!ret)
		goto found;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	返回 false;

found:
#ifdef DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#endif
	返回 true;
}

整数 bloblist_init(void)
{
	整数 ret;
	ulong addr = 0, size = CONFIG_BLOBLIST_SIZE;

	如果 (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->total_size, gd->bloblist);
		返回 0;
	}

	/*
	 * If Firmware Handoff is mandatory but no transfer list has been
	 * observed by fdtdec_setup, report it as an error.
	 */
	如果 (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		返回 -ENOENT;

	/*
	 * If we don't have an existing bloblist, we either need
	 * to allocate one now, or initialize the fixed address
	 * space as a bloblist.
	 */
	如果 (CONFIG_IS_ENABLED(BLOBLIST_ALLOC)) {
		void *ptr = memalign(BLOBLIST_ALIGN, size);

		如果 (!ptr)
			返回 log_msg_ret("alloc", -ENOMEM);
		addr = map_to_sysmem(ptr);
	} 否则
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", size,
		  addr);
	ret = bloblist_new(addr, size, 0, 0);
	如果 (ret)
		返回 log_msg_ret("ini", ret);

	返回 0;
}

整数 bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	整数 ret;

	如果 ((IS_ENABLED(CONFIG_64BIT) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64BIT) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_SHIFT_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_64));
	} 否则 {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_SHIFT_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_32));
	}

	如果 (rzero || rsig != sigval)
		返回 -EIO;

	ret = bloblist_check(xlist, 0);
	如果 (ret)
		返回 ret;

	如果 (rfdt != (ulong)bloblist_find(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = NULL;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		返回 -EIO;
	}

	返回 0;
}
