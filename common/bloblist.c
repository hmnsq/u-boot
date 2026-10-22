// SPDX-License-Identifier: GPL-2.0+ BSD-3-Clause
/*
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_BLOBLIST

#अन्तर्भाव <bloblist.h>
#अन्तर्भाव <display_options.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <mapmem.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <tables_csum.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <u-boot/crc.h>

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

स्थिर संरचना tag_name {
	गणना bloblist_tag_t tag;
	ध्रुव वर्ण *name;
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

ध्रुव वर्ण *bloblist_tag_name(गणना bloblist_tag_t tag)
{
	पूर्णाङ्क i;

	प्रति (i = 0; i < ARRAY_SIZE(tag_name); i++) {
		जइ (tag_name[i].tag == tag)
			प्रत्यागम tag_name[i].name;
	}

	प्रत्यागम "invalid";
}

स्थिर संरचना bloblist_rec *bloblist_first_blob(संरचना bloblist_hdr *hdr)
{
	जइ (hdr->used_size <= hdr->hdr_size)
		प्रत्यागम रिक्त;
	प्रत्यागम (संरचना bloblist_rec *)((शून्य *)hdr + hdr->hdr_size);
}

स्थिर अन्तःस्थ uint rec_hdr_size(संरचना bloblist_rec *rec)
{
	प्रत्यागम (rec->tag_and_hdr_size & BLOBLISTR_HDR_SIZE_MASK) >>
		BLOBLISTR_HDR_SIZE_SHIFT;
}

स्थिर अन्तःस्थ uint rec_tag(संरचना bloblist_rec *rec)
{
	प्रत्यागम (rec->tag_and_hdr_size & BLOBLISTR_TAG_MASK) >>
		BLOBLISTR_TAG_SHIFT;
}

स्थिर अन्तःस्थ शून्य void_blob(संरचना bloblist_rec *rec)
{
	जइ (rec_tag(rec) == BLOBLISTT_VOID)
		प्रत्यागम;
	rec->tag_and_hdr_size = BLOBLISTT_VOID |
				परिमाण(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
}

स्थिर अन्तःस्थ संरचना bloblist_rec *rec_from_blob(शून्य *blob)
{
	प्रत्यागम (blob - परिमाण(संरचना bloblist_rec));
}

स्थिर ulong bloblist_blob_end_ofs(संरचना bloblist_hdr *hdr,
				   संरचना bloblist_rec *rec)
{
	ulong offset;

	offset = (शून्य *)rec - (शून्य *)hdr;
	/*
	 * The data section of next TE should start from an address aligned
	 * to 1 << hdr->align_log2.
	 */
	offset += rec_hdr_size(rec) + rec->size;
	offset = round_up(offset + rec_hdr_size(rec), 1 << hdr->align_log2);
	offset -= rec_hdr_size(rec);

	प्रत्यागम offset;
}

स्थिर संरचना bloblist_rec *bloblist_next_blob(संरचना bloblist_hdr *hdr,
					       संरचना bloblist_rec *rec)
{
	ulong offset = bloblist_blob_end_ofs(hdr, rec);

	जइ (offset >= hdr->used_size)
		प्रत्यागम रिक्त;
	प्रत्यागम (संरचना bloblist_rec *)((शून्य *)hdr + offset);
}

#define foreach_rec(_rec, _hdr) \
	प्रति (_rec = bloblist_first_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_next_blob(_hdr, _rec))

स्थिर संरचना bloblist_rec *bloblist_findrec(uint tag)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;
	संरचना bloblist_rec *rec;

	जइ (!hdr)
		प्रत्यागम रिक्त;

	foreach_rec(rec, hdr) {
		जइ (rec_tag(rec) == tag)
			प्रत्यागम rec;
	}

	प्रत्यागम रिक्त;
}

स्थिर पूर्णाङ्क bloblist_addrec(uint tag, पूर्णाङ्क size, पूर्णाङ्क align_log2,
			   संरचना bloblist_rec **recp)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;
	संरचना bloblist_rec *rec;
	phys_addr_t data_start, aligned_start;
	phys_size_t new_alloced;

	जइ (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	data_start = map_to_sysmem(hdr) + hdr->used_size + परिमाण(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_start = ALIGN(data_start, 1U << align_log2) - data_start;

	/* If we need to create a dummy record, create it */
	जइ (aligned_start) {
		पूर्णाङ्क void_size = aligned_start - परिमाण(*rec);
		संरचना bloblist_rec *vrec;
		पूर्णाङ्क ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_size, 0, &vrec);
		जइ (ret)
			प्रत्यागम log_सन्देशः_ret("void", ret);

		/* start the record after that */
		data_start = map_to_sysmem(hdr) + hdr->used_size + परिमाण(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = data_start - map_to_sysmem(hdr) +
		ALIGN(size, 1U << align_log2);

	जइ (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		प्रत्यागम log_सन्देशः_ret("bloblist add", -ENOSPC);
	}
	rec = (शून्य *)hdr + hdr->used_size;

	rec->tag_and_hdr_size = tag | परिमाण(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
	rec->size = size;

	/* Zero the record data */
	memset((शून्य *)rec + rec_hdr_size(rec), '\0', rec->size);

	hdr->used_size = new_alloced;
	*recp = rec;

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क bloblist_ensurerec(uint tag, संरचना bloblist_rec **recp, पूर्णाङ्क size,
			      पूर्णाङ्क align_log2)
{
	संरचना bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	जइ (rec) {
		जइ (size && size != rec->size) {
			*recp = rec;
			प्रत्यागम -ESPIPE;
		}
	} अण्णहा {
		पूर्णाङ्क ret;

		ret = bloblist_addrec(tag, size, align_log2, &rec);
		जइ (ret)
			प्रत्यागम ret;
	}
	*recp = rec;

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क bloblist_get_blob_data_offset(uint tag)
{
	विकल्प (tag) {
	पक्ष BLOBLISTT_FDT_OVERLAY:
		प्रत्यागम परिमाण(संरचना dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	प्रत्यागम 0;
}

शून्य *bloblist_find(uint tag, पूर्णाङ्क size)
{
	शून्य *blob = रिक्त;
	पूर्णाङ्क blob_size;

	blob = bloblist_get_blob(tag, &blob_size);

	जइ (size && size != blob_size)
		प्रत्यागम रिक्त;

	प्रत्यागम blob;
}

शून्य *bloblist_get_blob(uint tag, पूर्णाङ्क *sizep)
{
	संरचना bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	जइ (!rec)
		प्रत्यागम रिक्त;

	*sizep = rec->size;

	प्रत्यागम (शून्य *)rec + rec_hdr_size(rec);
}

पूर्णाङ्क bloblist_apply_blobs(uint tag, पूर्णाङ्क (*func)(शून्य **data, पूर्णाङ्क size))
{
	संरचना bloblist_hdr *hdr = gd->bloblist;
	संरचना bloblist_rec *rec;

	जइ (!func || !hdr)
		प्रत्यागम -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		जइ (rec_tag(rec) == tag) {
			पूर्णाङ्क ret;
			पूर्णाङ्क tag = rec_tag(rec);
			शून्य *blob = (शून्य *)rec + rec_hdr_size(rec);
			पूर्णाङ्क dat_off = bloblist_get_blob_data_offset(tag);

			blob += dat_off;
			ret = func(&blob, rec->size - dat_off);
			जइ (ret) {
				log_err("Failed to apply blob with tag %d\n",
					tag);
				प्रत्यागम ret;
			}

			rec = rec_from_blob(blob - dat_off);
			जइ (!rec) {
				log_err("Blob corrupted\n");
				प्रत्यागम -ENOENT;
			}

			/* Mark applied blob record as void */
			void_blob(rec);
		}
	}

	प्रत्यागम 0;
}

शून्य *bloblist_add(uint tag, पूर्णाङ्क size, पूर्णाङ्क align_log2)
{
	संरचना bloblist_rec *rec;

	जइ (bloblist_addrec(tag, size, align_log2, &rec))
		प्रत्यागम रिक्त;

	प्रत्यागम (शून्य *)rec + rec_hdr_size(rec);
}

पूर्णाङ्क bloblist_ensure_size(uint tag, पूर्णाङ्क size, पूर्णाङ्क align_log2, शून्य **blobp)
{
	संरचना bloblist_rec *rec;
	पूर्णाङ्क ret;

	ret = bloblist_ensurerec(tag, &rec, size, align_log2);
	जइ (ret)
		प्रत्यागम ret;
	*blobp = (शून्य *)rec + rec_hdr_size(rec);

	प्रत्यागम 0;
}

शून्य *bloblist_ensure(uint tag, पूर्णाङ्क size)
{
	संरचना bloblist_rec *rec;

	जइ (bloblist_ensurerec(tag, &rec, size, 0))
		प्रत्यागम रिक्त;

	प्रत्यागम (शून्य *)rec + rec_hdr_size(rec);
}

पूर्णाङ्क bloblist_ensure_size_ret(uint tag, पूर्णाङ्क *sizep, शून्य **blobp)
{
	संरचना bloblist_rec *rec;
	पूर्णाङ्क ret;

	ret = bloblist_ensurerec(tag, &rec, *sizep, 0);
	जइ (ret == -ESPIPE)
		*sizep = rec->size;
	अण्णहा जइ (ret)
		प्रत्यागम ret;
	*blobp = (शून्य *)rec + rec_hdr_size(rec);

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क bloblist_resize_rec(संरचना bloblist_hdr *hdr,
			       संरचना bloblist_rec *rec,
			       पूर्णाङ्क new_size)
{
	पूर्णाङ्क expand_by;	/* Number of bytes to expand by (-ve to contract) */
	पूर्णाङ्क new_alloced;
	ulong next_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_size - rec->size, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->used_size + expand_by, BLOBLIST_BLOB_ALIGN);
	जइ (new_size < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_size);
		प्रत्यागम log_सन्देशः_ret("size", -EINVAL);
	}
	जइ (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", new_size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		प्रत्यागम log_सन्देशः_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	next_ofs = bloblist_blob_end_ofs(hdr, rec);
	जइ (next_ofs != hdr->used_size) {
		memmove((शून्य *)hdr + next_ofs + expand_by,
			(शून्य *)hdr + next_ofs, hdr->used_size - next_ofs);
	}
	hdr->used_size = new_alloced;

	/* Zero the new part of the blob */
	जइ (expand_by > 0) {
		memset((शून्य *)rec + rec_hdr_size(rec) + rec->size, '\0',
		       new_size - rec->size);
	}

	/* Update the size of this blob */
	rec->size = new_size;

	प्रत्यागम 0;
}

पूर्णाङ्क bloblist_resize(uint tag, पूर्णाङ्क new_size)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;
	संरचना bloblist_rec *rec;
	पूर्णाङ्क ret;

	rec = bloblist_findrec(tag);
	जइ (!rec)
		प्रत्यागम log_सन्देशः_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_size);
	जइ (ret)
		प्रत्यागम log_सन्देशः_ret("resize", ret);

	प्रत्यागम 0;
}

स्थिर u32 bloblist_calc_chksum(संरचना bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = table_compute_checksum(hdr, hdr->used_size);
	chksum += hdr->chksum;

	प्रत्यागम chksum;
}

पूर्णाङ्क bloblist_new(ulong addr, uint size, uint flags, uint align_log2)
{
	संरचना bloblist_hdr *hdr;

	जइ (size < परिमाण(*hdr))
		प्रत्यागम log_ret(-ENOSPC);
	जइ (addr & (BLOBLIST_ALIGN - 1))
		प्रत्यागम log_ret(-EFAULT);
	hdr = map_sysmem(addr, size);
	memset(hdr, '\0', परिमाण(*hdr));
	hdr->version = BLOBLIST_VERSION;
	hdr->hdr_size = परिमाण(*hdr);
	hdr->flags = flags;
	hdr->magic = BLOBLIST_MAGIC;
	hdr->used_size = hdr->hdr_size;
	hdr->total_size = size;
	hdr->align_log2 = align_log2 ? align_log2 : BLOBLIST_BLOB_ALIGN_LOG2;
	hdr->chksum = 0;
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	प्रत्यागम 0;
}

पूर्णाङ्क bloblist_check(ulong addr, uint size)
{
	संरचना bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, परिमाण(*hdr));
	जइ (hdr->magic != BLOBLIST_MAGIC)
		प्रत्यागम log_सन्देशः_ret("Bad magic", -ENOENT);
	जइ (hdr->version != BLOBLIST_VERSION)
		प्रत्यागम log_सन्देशः_ret("Bad version", -EPROTONOSUPPORT);
	जइ (!hdr->total_size || (size && hdr->total_size > size))
		प्रत्यागम log_सन्देशः_ret("Bad total size", -EFBIG);
	जइ (hdr->used_size > hdr->total_size)
		प्रत्यागम log_सन्देशः_ret("Bad used size", -ENOENT);
	जइ (hdr->hdr_size != परिमाण(संरचना bloblist_hdr))
		प्रत्यागम log_सन्देशः_ret("Bad header size", -ENOENT);

	chksum = bloblist_calc_chksum(hdr);
	जइ (hdr->chksum != chksum) {
		log_err("Checksum %x != %x\n", hdr->chksum, chksum);
		प्रत्यागम log_सन्देशः_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	प्रत्यागम 0;
}

पूर्णाङ्क bloblist_finish(शून्य)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->used_size,
		  (ulong)map_to_sysmem(hdr));

	प्रत्यागम 0;
}

ulong bloblist_get_base(शून्य)
{
	प्रत्यागम map_to_sysmem(gd->bloblist);
}

ulong bloblist_get_size(शून्य)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;

	प्रत्यागम hdr->used_size;
}

ulong bloblist_get_total_size(शून्य)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;

	प्रत्यागम hdr->total_size;
}

शून्य bloblist_get_stats(ulong *basep, ulong *tsizep, ulong *usizep)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;

	*basep = map_to_sysmem(gd->bloblist);
	*tsizep = hdr->total_size;
	*usizep = hdr->used_size;
}

स्थिर शून्य show_मानम्(ध्रुव वर्ण *prompt, ulong मानम्)
{
	printf("%s:%*s %-5lx  ", prompt, 10 - (पूर्णाङ्क)strlen(prompt), "", मानम्);
	print_size(मानम्, "\n");
}

शून्य bloblist_show_stats(शून्य)
{
	ulong base, tsize, usize;

	bloblist_get_stats(&base, &tsize, &usize);
	printf("base:       %lx\n", base);
	show_मानम्("total size", tsize);
	show_मानम्("used size", usize);
	show_मानम्("free", tsize - usize);
}

शून्य bloblist_show_list(शून्य)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;
	संरचना bloblist_rec *rec;

	printf("%-8s  %8s   Tag Name\n", "Address", "Size");
	प्रति (rec = bloblist_first_blob(hdr); rec;
	     rec = bloblist_next_blob(hdr, rec)) {
		printf("%08lx  %8x  %4x %s\n",
		       (ulong)map_to_sysmem((शून्य *)rec + rec_hdr_size(rec)),
		       rec->size, rec_tag(rec),
		       bloblist_tag_name(rec_tag(rec)));
	}
}

पूर्णाङ्क bloblist_reloc(शून्य *to, uint to_size)
{
	संरचना bloblist_hdr *hdr;

	जइ (!to_size)
		प्रत्यागम 0;

	जइ (to_size < gd->bloblist->total_size)
		प्रत्यागम -ENOSPC;

	memcpy(to, gd->bloblist, gd->bloblist->total_size);
	hdr = to;
	hdr->total_size = to_size;
	gd->bloblist = to;

	प्रत्यागम 0;
}

/*
 * Weak default function for getting bloblist from boot args.
 */
पूर्णाङ्क __weak xferlist_from_boot_arg(ulong __always_unused *addr)
{
	प्रत्यागम -ENOENT;
}

bool bloblist_exists(शून्य)
{
	पूर्णाङ्क ret;
	ulong addr = 0;

	/* Check if a valid transfer list passed in */
	जइ (!xferlist_from_boot_arg(&addr))
		गच्छ found;

	/*
	 * If Firmware Handoff is mandatory but no transfer list is
	 * observed, report it as an error.
	 */
	जइ (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		प्रत्यागम false;

	/*
	 * We have checked for a valid transfer list being passed. At this
	 * point, if we do not have a fixed address for the bloblist, we cannot
	 * be provided with one.
	 */
	जइ (xpl_is_first_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		प्रत्यागम false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_SIZE);
	जइ (!ret)
		गच्छ found;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	प्रत्यागम false;

found:
#ifdef DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#endif
	प्रत्यागम true;
}

पूर्णाङ्क bloblist_init(शून्य)
{
	पूर्णाङ्क ret;
	ulong addr = 0, size = CONFIG_BLOBLIST_SIZE;

	जइ (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->total_size, gd->bloblist);
		प्रत्यागम 0;
	}

	/*
	 * If Firmware Handoff is mandatory but no transfer list has been
	 * observed by fdtdec_setup, report it as an error.
	 */
	जइ (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		प्रत्यागम -ENOENT;

	/*
	 * If we don't have an existing bloblist, we either need
	 * to allocate one now, or initialize the fixed address
	 * space as a bloblist.
	 */
	जइ (CONFIG_IS_ENABLED(BLOBLIST_ALLOC)) {
		शून्य *ptr = memalign(BLOBLIST_ALIGN, size);

		जइ (!ptr)
			प्रत्यागम log_सन्देशः_ret("alloc", -ENOMEM);
		addr = map_to_sysmem(ptr);
	} अण्णहा
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", size,
		  addr);
	ret = bloblist_new(addr, size, 0, 0);
	जइ (ret)
		प्रत्यागम log_सन्देशः_ret("ini", ret);

	प्रत्यागम 0;
}

पूर्णाङ्क bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	पूर्णाङ्क ret;

	जइ ((IS_ENABLED(CONFIG_64BIT) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64BIT) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_SHIFT_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_64));
	} अण्णहा {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_SHIFT_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_32));
	}

	जइ (rzero || rsig != sigval)
		प्रत्यागम -EIO;

	ret = bloblist_check(xlist, 0);
	जइ (ret)
		प्रत्यागम ret;

	जइ (rfdt != (ulong)bloblist_find(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = रिक्त;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		प्रत्यागम -EIO;
	}

	प्रत्यागम 0;
}
