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
	จำนวนเต็ม i;

	สำหรับ (i = 0; i < ARRAY_SIZE(tag_name); i++) {
		ถ้า (tag_name[i].tag == tag)
			คืน tag_name[i].name;
	}

	คืน "invalid";
}

static sa_nracanaa bloblist_rec *bloblist_first_blob(sa_nracanaa bloblist_hdr *hdr)
{
	ถ้า (hdr->used_size <= hdr->hdr_size)
		คืน NULL;
	คืน (sa_nracanaa bloblist_rec *)((void *)hdr + hdr->hdr_size);
}

static anta_hstha uint rec_hdr_size(sa_nracanaa bloblist_rec *rec)
{
	คืน (rec->tag_and_hdr_size & BLOBLISTR_HDR_SIZE_MASK) >>
		BLOBLISTR_HDR_SIZE_SHIFT;
}

static anta_hstha uint rec_tag(sa_nracanaa bloblist_rec *rec)
{
	คืน (rec->tag_and_hdr_size & BLOBLISTR_TAG_MASK) >>
		BLOBLISTR_TAG_SHIFT;
}

static anta_hstha void void_blob(sa_nracanaa bloblist_rec *rec)
{
	ถ้า (rec_tag(rec) == BLOBLISTT_VOID)
		คืน;
	rec->tag_and_hdr_size = BLOBLISTT_VOID |
				parimaa_na(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
}

static anta_hstha sa_nracanaa bloblist_rec *rec_from_blob(void *blob)
{
	คืน (blob - parimaa_na(sa_nracanaa bloblist_rec));
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

	คืน offset;
}

static sa_nracanaa bloblist_rec *bloblist_next_blob(sa_nracanaa bloblist_hdr *hdr,
					       sa_nracanaa bloblist_rec *rec)
{
	ulong offset = bloblist_blob_end_ofs(hdr, rec);

	ถ้า (offset >= hdr->used_size)
		คืน NULL;
	คืน (sa_nracanaa bloblist_rec *)((void *)hdr + offset);
}

#define foreach_rec(_rec, _hdr) \
	สำหรับ (_rec = bloblist_first_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_next_blob(_hdr, _rec))

static sa_nracanaa bloblist_rec *bloblist_findrec(uint tag)
{
	sa_nracanaa bloblist_hdr *hdr = gd->bloblist;
	sa_nracanaa bloblist_rec *rec;

	ถ้า (!hdr)
		คืน NULL;

	foreach_rec(rec, hdr) {
		ถ้า (rec_tag(rec) == tag)
			คืน rec;
	}

	คืน NULL;
}

static จำนวนเต็ม bloblist_addrec(uint tag, จำนวนเต็ม size, จำนวนเต็ม align_log2,
			   sa_nracanaa bloblist_rec **recp)
{
	sa_nracanaa bloblist_hdr *hdr = gd->bloblist;
	sa_nracanaa bloblist_rec *rec;
	phys_addr_t data_start, aligned_start;
	phys_size_t new_alloced;

	ถ้า (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	data_start = map_to_sysmem(hdr) + hdr->used_size + parimaa_na(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_start = ALIGN(data_start, 1U << align_log2) - data_start;

	/* If we need to create a dummy record, create it */
	ถ้า (aligned_start) {
		จำนวนเต็ม void_size = aligned_start - parimaa_na(*rec);
		sa_nracanaa bloblist_rec *vrec;
		จำนวนเต็ม ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_size, 0, &vrec);
		ถ้า (ret)
			คืน log_msg_ret("void", ret);

		/* start the record after that */
		data_start = map_to_sysmem(hdr) + hdr->used_size + parimaa_na(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = data_start - map_to_sysmem(hdr) +
		ALIGN(size, 1U << align_log2);

	ถ้า (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		คืน log_msg_ret("bloblist add", -ENOSPC);
	}
	rec = (void *)hdr + hdr->used_size;

	rec->tag_and_hdr_size = tag | parimaa_na(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
	rec->size = size;

	/* Zero the record data */
	memset((void *)rec + rec_hdr_size(rec), '\0', rec->size);

	hdr->used_size = new_alloced;
	*recp = rec;

	คืน 0;
}

static จำนวนเต็ม bloblist_ensurerec(uint tag, sa_nracanaa bloblist_rec **recp, จำนวนเต็ม size,
			      จำนวนเต็ม align_log2)
{
	sa_nracanaa bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	ถ้า (rec) {
		ถ้า (size && size != rec->size) {
			*recp = rec;
			คืน -ESPIPE;
		}
	} มิฉะนั้น {
		จำนวนเต็ม ret;

		ret = bloblist_addrec(tag, size, align_log2, &rec);
		ถ้า (ret)
			คืน ret;
	}
	*recp = rec;

	คืน 0;
}

static จำนวนเต็ม bloblist_get_blob_data_offset(uint tag)
{
	switch (tag) {
	pak_sa BLOBLISTT_FDT_OVERLAY:
		คืน parimaa_na(sa_nracanaa dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	คืน 0;
}

void *bloblist_find(uint tag, จำนวนเต็ม size)
{
	void *blob = NULL;
	จำนวนเต็ม blob_size;

	blob = bloblist_get_blob(tag, &blob_size);

	ถ้า (size && size != blob_size)
		คืน NULL;

	คืน blob;
}

void *bloblist_get_blob(uint tag, จำนวนเต็ม *sizep)
{
	sa_nracanaa bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	ถ้า (!rec)
		คืน NULL;

	*sizep = rec->size;

	คืน (void *)rec + rec_hdr_size(rec);
}

จำนวนเต็ม bloblist_apply_blobs(uint tag, จำนวนเต็ม (*func)(void **data, จำนวนเต็ม size))
{
	sa_nracanaa bloblist_hdr *hdr = gd->bloblist;
	sa_nracanaa bloblist_rec *rec;

	ถ้า (!func || !hdr)
		คืน -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		ถ้า (rec_tag(rec) == tag) {
			จำนวนเต็ม ret;
			จำนวนเต็ม tag = rec_tag(rec);
			void *blob = (void *)rec + rec_hdr_size(rec);
			จำนวนเต็ม dat_off = bloblist_get_blob_data_offset(tag);

			blob += dat_off;
			ret = func(&blob, rec->size - dat_off);
			ถ้า (ret) {
				log_err("Failed to apply blob with tag %d\n",
					tag);
				คืน ret;
			}

			rec = rec_from_blob(blob - dat_off);
			ถ้า (!rec) {
				log_err("Blob corrupted\n");
				คืน -ENOENT;
			}

			/* Mark applied blob record as void */
			void_blob(rec);
		}
	}

	คืน 0;
}

void *bloblist_add(uint tag, จำนวนเต็ม size, จำนวนเต็ม align_log2)
{
	sa_nracanaa bloblist_rec *rec;

	ถ้า (bloblist_addrec(tag, size, align_log2, &rec))
		คืน NULL;

	คืน (void *)rec + rec_hdr_size(rec);
}

จำนวนเต็ม bloblist_ensure_size(uint tag, จำนวนเต็ม size, จำนวนเต็ม align_log2, void **blobp)
{
	sa_nracanaa bloblist_rec *rec;
	จำนวนเต็ม ret;

	ret = bloblist_ensurerec(tag, &rec, size, align_log2);
	ถ้า (ret)
		คืน ret;
	*blobp = (void *)rec + rec_hdr_size(rec);

	คืน 0;
}

void *bloblist_ensure(uint tag, จำนวนเต็ม size)
{
	sa_nracanaa bloblist_rec *rec;

	ถ้า (bloblist_ensurerec(tag, &rec, size, 0))
		คืน NULL;

	คืน (void *)rec + rec_hdr_size(rec);
}

จำนวนเต็ม bloblist_ensure_size_ret(uint tag, จำนวนเต็ม *sizep, void **blobp)
{
	sa_nracanaa bloblist_rec *rec;
	จำนวนเต็ม ret;

	ret = bloblist_ensurerec(tag, &rec, *sizep, 0);
	ถ้า (ret == -ESPIPE)
		*sizep = rec->size;
	มิฉะนั้น ถ้า (ret)
		คืน ret;
	*blobp = (void *)rec + rec_hdr_size(rec);

	คืน 0;
}

static จำนวนเต็ม bloblist_resize_rec(sa_nracanaa bloblist_hdr *hdr,
			       sa_nracanaa bloblist_rec *rec,
			       จำนวนเต็ม new_size)
{
	จำนวนเต็ม expand_by;	/* Number of bytes to expand by (-ve to contract) */
	จำนวนเต็ม new_alloced;
	ulong next_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_size - rec->size, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->used_size + expand_by, BLOBLIST_BLOB_ALIGN);
	ถ้า (new_size < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_size);
		คืน log_msg_ret("size", -EINVAL);
	}
	ถ้า (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", new_size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		คืน log_msg_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	next_ofs = bloblist_blob_end_ofs(hdr, rec);
	ถ้า (next_ofs != hdr->used_size) {
		memmove((void *)hdr + next_ofs + expand_by,
			(void *)hdr + next_ofs, hdr->used_size - next_ofs);
	}
	hdr->used_size = new_alloced;

	/* Zero the new part of the blob */
	ถ้า (expand_by > 0) {
		memset((void *)rec + rec_hdr_size(rec) + rec->size, '\0',
		       new_size - rec->size);
	}

	/* Update the size of this blob */
	rec->size = new_size;

	คืน 0;
}

จำนวนเต็ม bloblist_resize(uint tag, จำนวนเต็ม new_size)
{
	sa_nracanaa bloblist_hdr *hdr = gd->bloblist;
	sa_nracanaa bloblist_rec *rec;
	จำนวนเต็ม ret;

	rec = bloblist_findrec(tag);
	ถ้า (!rec)
		คืน log_msg_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_size);
	ถ้า (ret)
		คืน log_msg_ret("resize", ret);

	คืน 0;
}

static u32 bloblist_calc_chksum(sa_nracanaa bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = table_compute_checksum(hdr, hdr->used_size);
	chksum += hdr->chksum;

	คืน chksum;
}

จำนวนเต็ม bloblist_new(ulong addr, uint size, uint flags, uint align_log2)
{
	sa_nracanaa bloblist_hdr *hdr;

	ถ้า (size < parimaa_na(*hdr))
		คืน log_ret(-ENOSPC);
	ถ้า (addr & (BLOBLIST_ALIGN - 1))
		คืน log_ret(-EFAULT);
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

	คืน 0;
}

จำนวนเต็ม bloblist_check(ulong addr, uint size)
{
	sa_nracanaa bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, parimaa_na(*hdr));
	ถ้า (hdr->magic != BLOBLIST_MAGIC)
		คืน log_msg_ret("Bad magic", -ENOENT);
	ถ้า (hdr->version != BLOBLIST_VERSION)
		คืน log_msg_ret("Bad version", -EPROTONOSUPPORT);
	ถ้า (!hdr->total_size || (size && hdr->total_size > size))
		คืน log_msg_ret("Bad total size", -EFBIG);
	ถ้า (hdr->used_size > hdr->total_size)
		คืน log_msg_ret("Bad used size", -ENOENT);
	ถ้า (hdr->hdr_size != parimaa_na(sa_nracanaa bloblist_hdr))
		คืน log_msg_ret("Bad header size", -ENOENT);

	chksum = bloblist_calc_chksum(hdr);
	ถ้า (hdr->chksum != chksum) {
		log_err("Checksum %x != %x\n", hdr->chksum, chksum);
		คืน log_msg_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	คืน 0;
}

จำนวนเต็ม bloblist_finish(void)
{
	sa_nracanaa bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->used_size,
		  (ulong)map_to_sysmem(hdr));

	คืน 0;
}

ulong bloblist_get_base(void)
{
	คืน map_to_sysmem(gd->bloblist);
}

ulong bloblist_get_size(void)
{
	sa_nracanaa bloblist_hdr *hdr = gd->bloblist;

	คืน hdr->used_size;
}

ulong bloblist_get_total_size(void)
{
	sa_nracanaa bloblist_hdr *hdr = gd->bloblist;

	คืน hdr->total_size;
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
	printf("%s:%*s %-5lx  ", prompt, 10 - (จำนวนเต็ม)strlen(prompt), "", value);
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
	สำหรับ (rec = bloblist_first_blob(hdr); rec;
	     rec = bloblist_next_blob(hdr, rec)) {
		printf("%08lx  %8x  %4x %s\n",
		       (ulong)map_to_sysmem((void *)rec + rec_hdr_size(rec)),
		       rec->size, rec_tag(rec),
		       bloblist_tag_name(rec_tag(rec)));
	}
}

จำนวนเต็ม bloblist_reloc(void *to, uint to_size)
{
	sa_nracanaa bloblist_hdr *hdr;

	ถ้า (!to_size)
		คืน 0;

	ถ้า (to_size < gd->bloblist->total_size)
		คืน -ENOSPC;

	memcpy(to, gd->bloblist, gd->bloblist->total_size);
	hdr = to;
	hdr->total_size = to_size;
	gd->bloblist = to;

	คืน 0;
}

/*
 * Weak default function for getting bloblist from boot args.
 */
จำนวนเต็ม __weak xferlist_from_boot_arg(ulong __always_unused *addr)
{
	คืน -ENOENT;
}

bool bloblist_exists(void)
{
	จำนวนเต็ม ret;
	ulong addr = 0;

	/* Check if a valid transfer list passed in */
	ถ้า (!xferlist_from_boot_arg(&addr))
		goto found;

	/*
	 * If Firmware Handoff is mandatory but no transfer list is
	 * observed, report it as an error.
	 */
	ถ้า (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		คืน false;

	/*
	 * We have checked for a valid transfer list being passed. At this
	 * point, if we do not have a fixed address for the bloblist, we cannot
	 * be provided with one.
	 */
	ถ้า (xpl_is_first_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		คืน false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_SIZE);
	ถ้า (!ret)
		goto found;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	คืน false;

found:
#ifdef DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#endif
	คืน true;
}

จำนวนเต็ม bloblist_init(void)
{
	จำนวนเต็ม ret;
	ulong addr = 0, size = CONFIG_BLOBLIST_SIZE;

	ถ้า (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->total_size, gd->bloblist);
		คืน 0;
	}

	/*
	 * If Firmware Handoff is mandatory but no transfer list has been
	 * observed by fdtdec_setup, report it as an error.
	 */
	ถ้า (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		คืน -ENOENT;

	/*
	 * If we don't have an existing bloblist, we either need
	 * to allocate one now, or initialize the fixed address
	 * space as a bloblist.
	 */
	ถ้า (CONFIG_IS_ENABLED(BLOBLIST_ALLOC)) {
		void *ptr = memalign(BLOBLIST_ALIGN, size);

		ถ้า (!ptr)
			คืน log_msg_ret("alloc", -ENOMEM);
		addr = map_to_sysmem(ptr);
	} มิฉะนั้น
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", size,
		  addr);
	ret = bloblist_new(addr, size, 0, 0);
	ถ้า (ret)
		คืน log_msg_ret("ini", ret);

	คืน 0;
}

จำนวนเต็ม bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	จำนวนเต็ม ret;

	ถ้า ((IS_ENABLED(CONFIG_64BIT) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64BIT) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_SHIFT_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_64));
	} มิฉะนั้น {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_SHIFT_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_32));
	}

	ถ้า (rzero || rsig != sigval)
		คืน -EIO;

	ret = bloblist_check(xlist, 0);
	ถ้า (ret)
		คืน ret;

	ถ้า (rfdt != (ulong)bloblist_find(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = NULL;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		คืน -EIO;
	}

	คืน 0;
}
