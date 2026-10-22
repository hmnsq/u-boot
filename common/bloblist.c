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
	स्थिरांक अक्षर *name;
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

स्थिरांक अक्षर *bloblist_tag_name(गणना bloblist_tag_t tag)
{
	पूर्णांक i;

	साठी (i = 0; i < ARRAY_SIZE(tag_name); i++) {
		जर (tag_name[i].tag == tag)
			परत tag_name[i].name;
	}

	परत "invalid";
}

स्थिर संरचना bloblist_rec *bloblist_first_blob(संरचना bloblist_hdr *hdr)
{
	जर (hdr->used_size <= hdr->hdr_size)
		परत रिक्त;
	परत (संरचना bloblist_rec *)((शून्य *)hdr + hdr->hdr_size);
}

स्थिर ओळीत uint rec_hdr_size(संरचना bloblist_rec *rec)
{
	परत (rec->tag_and_hdr_size & BLOBLISTR_HDR_SIZE_MASK) >>
		BLOBLISTR_HDR_SIZE_SHIFT;
}

स्थिर ओळीत uint rec_tag(संरचना bloblist_rec *rec)
{
	परत (rec->tag_and_hdr_size & BLOBLISTR_TAG_MASK) >>
		BLOBLISTR_TAG_SHIFT;
}

स्थिर ओळीत शून्य void_blob(संरचना bloblist_rec *rec)
{
	जर (rec_tag(rec) == BLOBLISTT_VOID)
		परत;
	rec->tag_and_hdr_size = BLOBLISTT_VOID |
				आकार(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
}

स्थिर ओळीत संरचना bloblist_rec *rec_from_blob(शून्य *blob)
{
	परत (blob - आकार(संरचना bloblist_rec));
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

	परत offset;
}

स्थिर संरचना bloblist_rec *bloblist_next_blob(संरचना bloblist_hdr *hdr,
					       संरचना bloblist_rec *rec)
{
	ulong offset = bloblist_blob_end_ofs(hdr, rec);

	जर (offset >= hdr->used_size)
		परत रिक्त;
	परत (संरचना bloblist_rec *)((शून्य *)hdr + offset);
}

#define foreach_rec(_rec, _hdr) \
	साठी (_rec = bloblist_first_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_next_blob(_hdr, _rec))

स्थिर संरचना bloblist_rec *bloblist_findrec(uint tag)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;
	संरचना bloblist_rec *rec;

	जर (!hdr)
		परत रिक्त;

	foreach_rec(rec, hdr) {
		जर (rec_tag(rec) == tag)
			परत rec;
	}

	परत रिक्त;
}

स्थिर पूर्णांक bloblist_addrec(uint tag, पूर्णांक size, पूर्णांक align_log2,
			   संरचना bloblist_rec **recp)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;
	संरचना bloblist_rec *rec;
	phys_addr_t data_start, aligned_start;
	phys_size_t new_alloced;

	जर (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	data_start = map_to_sysmem(hdr) + hdr->used_size + आकार(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_start = ALIGN(data_start, 1U << align_log2) - data_start;

	/* If we need to create a dummy record, create it */
	जर (aligned_start) {
		पूर्णांक void_size = aligned_start - आकार(*rec);
		संरचना bloblist_rec *vrec;
		पूर्णांक ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_size, 0, &vrec);
		जर (ret)
			परत log_संदेश_ret("void", ret);

		/* start the record after that */
		data_start = map_to_sysmem(hdr) + hdr->used_size + आकार(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = data_start - map_to_sysmem(hdr) +
		ALIGN(size, 1U << align_log2);

	जर (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		परत log_संदेश_ret("bloblist add", -ENOSPC);
	}
	rec = (शून्य *)hdr + hdr->used_size;

	rec->tag_and_hdr_size = tag | आकार(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
	rec->size = size;

	/* Zero the record data */
	स_रखो((शून्य *)rec + rec_hdr_size(rec), '\0', rec->size);

	hdr->used_size = new_alloced;
	*recp = rec;

	परत 0;
}

स्थिर पूर्णांक bloblist_ensurerec(uint tag, संरचना bloblist_rec **recp, पूर्णांक size,
			      पूर्णांक align_log2)
{
	संरचना bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	जर (rec) {
		जर (size && size != rec->size) {
			*recp = rec;
			परत -ESPIPE;
		}
	} अन्यथा {
		पूर्णांक ret;

		ret = bloblist_addrec(tag, size, align_log2, &rec);
		जर (ret)
			परत ret;
	}
	*recp = rec;

	परत 0;
}

स्थिर पूर्णांक bloblist_get_blob_data_offset(uint tag)
{
	निवड (tag) {
	स्थिती BLOBLISTT_FDT_OVERLAY:
		परत आकार(संरचना dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	परत 0;
}

शून्य *bloblist_find(uint tag, पूर्णांक size)
{
	शून्य *blob = रिक्त;
	पूर्णांक blob_size;

	blob = bloblist_get_blob(tag, &blob_size);

	जर (size && size != blob_size)
		परत रिक्त;

	परत blob;
}

शून्य *bloblist_get_blob(uint tag, पूर्णांक *sizep)
{
	संरचना bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	जर (!rec)
		परत रिक्त;

	*sizep = rec->size;

	परत (शून्य *)rec + rec_hdr_size(rec);
}

पूर्णांक bloblist_apply_blobs(uint tag, पूर्णांक (*func)(शून्य **data, पूर्णांक size))
{
	संरचना bloblist_hdr *hdr = gd->bloblist;
	संरचना bloblist_rec *rec;

	जर (!func || !hdr)
		परत -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		जर (rec_tag(rec) == tag) {
			पूर्णांक ret;
			पूर्णांक tag = rec_tag(rec);
			शून्य *blob = (शून्य *)rec + rec_hdr_size(rec);
			पूर्णांक dat_off = bloblist_get_blob_data_offset(tag);

			blob += dat_off;
			ret = func(&blob, rec->size - dat_off);
			जर (ret) {
				log_err("Failed to apply blob with tag %d\n",
					tag);
				परत ret;
			}

			rec = rec_from_blob(blob - dat_off);
			जर (!rec) {
				log_err("Blob corrupted\n");
				परत -ENOENT;
			}

			/* Mark applied blob record as void */
			void_blob(rec);
		}
	}

	परत 0;
}

शून्य *bloblist_add(uint tag, पूर्णांक size, पूर्णांक align_log2)
{
	संरचना bloblist_rec *rec;

	जर (bloblist_addrec(tag, size, align_log2, &rec))
		परत रिक्त;

	परत (शून्य *)rec + rec_hdr_size(rec);
}

पूर्णांक bloblist_ensure_size(uint tag, पूर्णांक size, पूर्णांक align_log2, शून्य **blobp)
{
	संरचना bloblist_rec *rec;
	पूर्णांक ret;

	ret = bloblist_ensurerec(tag, &rec, size, align_log2);
	जर (ret)
		परत ret;
	*blobp = (शून्य *)rec + rec_hdr_size(rec);

	परत 0;
}

शून्य *bloblist_ensure(uint tag, पूर्णांक size)
{
	संरचना bloblist_rec *rec;

	जर (bloblist_ensurerec(tag, &rec, size, 0))
		परत रिक्त;

	परत (शून्य *)rec + rec_hdr_size(rec);
}

पूर्णांक bloblist_ensure_size_ret(uint tag, पूर्णांक *sizep, शून्य **blobp)
{
	संरचना bloblist_rec *rec;
	पूर्णांक ret;

	ret = bloblist_ensurerec(tag, &rec, *sizep, 0);
	जर (ret == -ESPIPE)
		*sizep = rec->size;
	अन्यथा जर (ret)
		परत ret;
	*blobp = (शून्य *)rec + rec_hdr_size(rec);

	परत 0;
}

स्थिर पूर्णांक bloblist_resize_rec(संरचना bloblist_hdr *hdr,
			       संरचना bloblist_rec *rec,
			       पूर्णांक new_size)
{
	पूर्णांक expand_by;	/* Number of bytes to expand by (-ve to contract) */
	पूर्णांक new_alloced;
	ulong next_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_size - rec->size, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->used_size + expand_by, BLOBLIST_BLOB_ALIGN);
	जर (new_size < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_size);
		परत log_संदेश_ret("size", -EINVAL);
	}
	जर (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", new_size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		परत log_संदेश_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	next_ofs = bloblist_blob_end_ofs(hdr, rec);
	जर (next_ofs != hdr->used_size) {
		स_हटाओ((शून्य *)hdr + next_ofs + expand_by,
			(शून्य *)hdr + next_ofs, hdr->used_size - next_ofs);
	}
	hdr->used_size = new_alloced;

	/* Zero the new part of the blob */
	जर (expand_by > 0) {
		स_रखो((शून्य *)rec + rec_hdr_size(rec) + rec->size, '\0',
		       new_size - rec->size);
	}

	/* Update the size of this blob */
	rec->size = new_size;

	परत 0;
}

पूर्णांक bloblist_resize(uint tag, पूर्णांक new_size)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;
	संरचना bloblist_rec *rec;
	पूर्णांक ret;

	rec = bloblist_findrec(tag);
	जर (!rec)
		परत log_संदेश_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_size);
	जर (ret)
		परत log_संदेश_ret("resize", ret);

	परत 0;
}

स्थिर u32 bloblist_calc_chksum(संरचना bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = table_compute_checksum(hdr, hdr->used_size);
	chksum += hdr->chksum;

	परत chksum;
}

पूर्णांक bloblist_new(ulong addr, uint size, uint flags, uint align_log2)
{
	संरचना bloblist_hdr *hdr;

	जर (size < आकार(*hdr))
		परत log_ret(-ENOSPC);
	जर (addr & (BLOBLIST_ALIGN - 1))
		परत log_ret(-EFAULT);
	hdr = map_sysmem(addr, size);
	स_रखो(hdr, '\0', आकार(*hdr));
	hdr->version = BLOBLIST_VERSION;
	hdr->hdr_size = आकार(*hdr);
	hdr->flags = flags;
	hdr->magic = BLOBLIST_MAGIC;
	hdr->used_size = hdr->hdr_size;
	hdr->total_size = size;
	hdr->align_log2 = align_log2 ? align_log2 : BLOBLIST_BLOB_ALIGN_LOG2;
	hdr->chksum = 0;
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	परत 0;
}

पूर्णांक bloblist_check(ulong addr, uint size)
{
	संरचना bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, आकार(*hdr));
	जर (hdr->magic != BLOBLIST_MAGIC)
		परत log_संदेश_ret("Bad magic", -ENOENT);
	जर (hdr->version != BLOBLIST_VERSION)
		परत log_संदेश_ret("Bad version", -EPROTONOSUPPORT);
	जर (!hdr->total_size || (size && hdr->total_size > size))
		परत log_संदेश_ret("Bad total size", -EFBIG);
	जर (hdr->used_size > hdr->total_size)
		परत log_संदेश_ret("Bad used size", -ENOENT);
	जर (hdr->hdr_size != आकार(संरचना bloblist_hdr))
		परत log_संदेश_ret("Bad header size", -ENOENT);

	chksum = bloblist_calc_chksum(hdr);
	जर (hdr->chksum != chksum) {
		log_err("Checksum %x != %x\n", hdr->chksum, chksum);
		परत log_संदेश_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	परत 0;
}

पूर्णांक bloblist_finish(शून्य)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->used_size,
		  (ulong)map_to_sysmem(hdr));

	परत 0;
}

ulong bloblist_get_base(शून्य)
{
	परत map_to_sysmem(gd->bloblist);
}

ulong bloblist_get_size(शून्य)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;

	परत hdr->used_size;
}

ulong bloblist_get_total_size(शून्य)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;

	परत hdr->total_size;
}

शून्य bloblist_get_stats(ulong *basep, ulong *tsizep, ulong *usizep)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;

	*basep = map_to_sysmem(gd->bloblist);
	*tsizep = hdr->total_size;
	*usizep = hdr->used_size;
}

स्थिर शून्य show_मूल्य(स्थिरांक अक्षर *prompt, ulong मूल्य)
{
	म_लिखो("%s:%*s %-5lx  ", prompt, 10 - (पूर्णांक)म_माप(prompt), "", मूल्य);
	print_size(मूल्य, "\n");
}

शून्य bloblist_show_stats(शून्य)
{
	ulong base, tsize, usize;

	bloblist_get_stats(&base, &tsize, &usize);
	म_लिखो("base:       %lx\n", base);
	show_मूल्य("total size", tsize);
	show_मूल्य("used size", usize);
	show_मूल्य("free", tsize - usize);
}

शून्य bloblist_show_list(शून्य)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;
	संरचना bloblist_rec *rec;

	म_लिखो("%-8s  %8s   Tag Name\n", "Address", "Size");
	साठी (rec = bloblist_first_blob(hdr); rec;
	     rec = bloblist_next_blob(hdr, rec)) {
		म_लिखो("%08lx  %8x  %4x %s\n",
		       (ulong)map_to_sysmem((शून्य *)rec + rec_hdr_size(rec)),
		       rec->size, rec_tag(rec),
		       bloblist_tag_name(rec_tag(rec)));
	}
}

पूर्णांक bloblist_reloc(शून्य *to, uint to_size)
{
	संरचना bloblist_hdr *hdr;

	जर (!to_size)
		परत 0;

	जर (to_size < gd->bloblist->total_size)
		परत -ENOSPC;

	स_नकल(to, gd->bloblist, gd->bloblist->total_size);
	hdr = to;
	hdr->total_size = to_size;
	gd->bloblist = to;

	परत 0;
}

/*
 * Weak default function for getting bloblist from boot args.
 */
पूर्णांक __weak xferlist_from_boot_arg(ulong __always_unused *addr)
{
	परत -ENOENT;
}

bool bloblist_exists(शून्य)
{
	पूर्णांक ret;
	ulong addr = 0;

	/* Check if a valid transfer list passed in */
	जर (!xferlist_from_boot_arg(&addr))
		जा found;

	/*
	 * If Firmware Handoff is mandatory but no transfer list is
	 * observed, report it as an error.
	 */
	जर (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		परत false;

	/*
	 * We have checked for a valid transfer list being passed. At this
	 * point, if we do not have a fixed address for the bloblist, we cannot
	 * be provided with one.
	 */
	जर (xpl_is_first_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		परत false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_SIZE);
	जर (!ret)
		जा found;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	परत false;

found:
#ifdef DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#endif
	परत true;
}

पूर्णांक bloblist_init(शून्य)
{
	पूर्णांक ret;
	ulong addr = 0, size = CONFIG_BLOBLIST_SIZE;

	जर (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->total_size, gd->bloblist);
		परत 0;
	}

	/*
	 * If Firmware Handoff is mandatory but no transfer list has been
	 * observed by fdtdec_setup, report it as an error.
	 */
	जर (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		परत -ENOENT;

	/*
	 * If we don't have an existing bloblist, we either need
	 * to allocate one now, or initialize the fixed address
	 * space as a bloblist.
	 */
	जर (CONFIG_IS_ENABLED(BLOBLIST_ALLOC)) {
		शून्य *ptr = memalign(BLOBLIST_ALIGN, size);

		जर (!ptr)
			परत log_संदेश_ret("alloc", -ENOMEM);
		addr = map_to_sysmem(ptr);
	} अन्यथा
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", size,
		  addr);
	ret = bloblist_new(addr, size, 0, 0);
	जर (ret)
		परत log_संदेश_ret("ini", ret);

	परत 0;
}

पूर्णांक bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	पूर्णांक ret;

	जर ((IS_ENABLED(CONFIG_64BIT) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64BIT) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_SHIFT_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_64));
	} अन्यथा {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_SHIFT_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_32));
	}

	जर (rzero || rsig != sigval)
		परत -EIO;

	ret = bloblist_check(xlist, 0);
	जर (ret)
		परत ret;

	जर (rfdt != (ulong)bloblist_find(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = रिक्त;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		परत -EIO;
	}

	परत 0;
}
