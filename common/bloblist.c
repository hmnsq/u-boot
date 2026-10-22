// SPDX-License-Identifier: GPL-2.0+ BSD-3-Clause
/*
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#परिभाषय LOG_CATEGORY	LOGC_BLOBLIST

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

DECLARE_GLOBAL_दत्त_PTR;

स्थिर संरचना चिह्नकम्_name {
	गणना bloblist_चिह्नकम्_t चिह्नकम्;
	ध्रुव वर्ण *name;
} चिह्नकम्_name[] = {
	{ BLOBLISTT_VOID, "(void)" },

	/* BLOBLISTT_AREA_FIRMWARE_TOP */
	{ BLOBLISTT_CONTROL_FDT, "Control FDT" },
	{ BLOBLISTT_HOB_खण्डम्, "HOB block" },
	{ BLOBLISTT_HOB_LIST, "HOB list" },
	{ BLOBLISTT_ACPI_TABLES, "ACPI tables for x86" },
	{ BLOBLISTT_TPM_EVLOG, "TPM event log defined by TCG EFI" },
	{ BLOBLISTT_TPM_CRB_आधारः, "TPM Command Response Buffer address" },
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

ध्रुव वर्ण *bloblist_चिह्नकम्_name(गणना bloblist_चिह्नकम्_t चिह्नकम्)
{
	पूर्णाङ्क i;

	प्रति (i = 0; i < ARRAY_आकार(चिह्नकम्_name); i++) {
		यदि (चिह्नकम्_name[i].चिह्नकम् == चिह्नकम्)
			प्रत्यागम चिह्नकम्_name[i].name;
	}

	प्रत्यागम "invalid";
}

स्थिर संरचना bloblist_rec *bloblist_first_blob(संरचना bloblist_hdr *hdr)
{
	यदि (hdr->प्रयुक्तम्_आकार <= hdr->hdr_आकार)
		प्रत्यागम रिक्त;
	प्रत्यागम (संरचना bloblist_rec *)((शून्य *)hdr + hdr->hdr_आकार);
}

स्थिर अन्तःस्थ uint rec_hdr_आकार(संरचना bloblist_rec *rec)
{
	प्रत्यागम (rec->चिह्नकम्_and_hdr_आकार & BLOBLISTR_HDR_आकार_आवरणम्) >>
		BLOBLISTR_HDR_आकार_सरणम्;
}

स्थिर अन्तःस्थ uint rec_चिह्नकम्(संरचना bloblist_rec *rec)
{
	प्रत्यागम (rec->चिह्नकम्_and_hdr_आकार & BLOBLISTR_चिह्नकम्_आवरणम्) >>
		BLOBLISTR_चिह्नकम्_सरणम्;
}

स्थिर अन्तःस्थ शून्य void_blob(संरचना bloblist_rec *rec)
{
	यदि (rec_चिह्नकम्(rec) == BLOBLISTT_VOID)
		प्रत्यागम;
	rec->चिह्नकम्_and_hdr_आकार = BLOBLISTT_VOID |
				परिमाण(*rec) << BLOBLISTR_HDR_आकार_सरणम्;
}

स्थिर अन्तःस्थ संरचना bloblist_rec *rec_from_blob(शून्य *blob)
{
	प्रत्यागम (blob - परिमाण(संरचना bloblist_rec));
}

स्थिर ulong bloblist_blob_अन्तः_ofs(संरचना bloblist_hdr *hdr,
				   संरचना bloblist_rec *rec)
{
	ulong विस्थापनम्;

	विस्थापनम् = (शून्य *)rec - (शून्य *)hdr;
	/*
	 * The data section of next TE should start from an address aligned
	 * to 1 << hdr->align_log2.
	 */
	विस्थापनम् += rec_hdr_आकार(rec) + rec->आकार;
	विस्थापनम् = आवर्तनम्_ऊर्ध्वम्(विस्थापनम् + rec_hdr_आकार(rec), 1 << hdr->align_log2);
	विस्थापनम् -= rec_hdr_आकार(rec);

	प्रत्यागम विस्थापनम्;
}

स्थिर संरचना bloblist_rec *bloblist_अग्र_blob(संरचना bloblist_hdr *hdr,
					       संरचना bloblist_rec *rec)
{
	ulong विस्थापनम् = bloblist_blob_अन्तः_ofs(hdr, rec);

	यदि (विस्थापनम् >= hdr->प्रयुक्तम्_आकार)
		प्रत्यागम रिक्त;
	प्रत्यागम (संरचना bloblist_rec *)((शून्य *)hdr + विस्थापनम्);
}

#परिभाषय foreach_rec(_rec, _hdr) \
	प्रति (_rec = bloblist_first_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_अग्र_blob(_hdr, _rec))

स्थिर संरचना bloblist_rec *bloblist_findrec(uint चिह्नकम्)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;
	संरचना bloblist_rec *rec;

	यदि (!hdr)
		प्रत्यागम रिक्त;

	foreach_rec(rec, hdr) {
		यदि (rec_चिह्नकम्(rec) == चिह्नकम्)
			प्रत्यागम rec;
	}

	प्रत्यागम रिक्त;
}

स्थिर पूर्णाङ्क bloblist_addrec(uint चिह्नकम्, पूर्णाङ्क आकार, पूर्णाङ्क align_log2,
			   संरचना bloblist_rec **recp)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;
	संरचना bloblist_rec *rec;
	phys_addr_t दत्त_start, aligned_start;
	phys_आकार_t new_alloced;

	यदि (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	दत्त_start = map_गन्तव्यम्_sysmem(hdr) + hdr->प्रयुक्तम्_आकार + परिमाण(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_start = ALIGN(दत्त_start, 1U << align_log2) - दत्त_start;

	/* If we need to create a dummy record, create it */
	यदि (aligned_start) {
		पूर्णाङ्क void_आकार = aligned_start - परिमाण(*rec);
		संरचना bloblist_rec *vrec;
		पूर्णाङ्क ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_आकार, 0, &vrec);
		यदि (ret)
			प्रत्यागम log_सन्देशः_ret("void", ret);

		/* start the record after that */
		दत्त_start = map_गन्तव्यम्_sysmem(hdr) + hdr->प्रयुक्तम्_आकार + परिमाण(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = दत्त_start - map_गन्तव्यम्_sysmem(hdr) +
		ALIGN(आकार, 1U << align_log2);

	यदि (new_alloced > hdr->योग_आकार) {
		log_err("Failed to allocate %x bytes\n", आकार);
		log_err("Used size=%x, total size=%x\n",
			hdr->प्रयुक्तम्_आकार, hdr->योग_आकार);
		प्रत्यागम log_सन्देशः_ret("bloblist add", -ENOSPC);
	}
	rec = (शून्य *)hdr + hdr->प्रयुक्तम्_आकार;

	rec->चिह्नकम्_and_hdr_आकार = चिह्नकम् | परिमाण(*rec) << BLOBLISTR_HDR_आकार_सरणम्;
	rec->आकार = आकार;

	/* Zero the record data */
	स्मृतिं_पूरय((शून्य *)rec + rec_hdr_आकार(rec), '\0', rec->आकार);

	hdr->प्रयुक्तम्_आकार = new_alloced;
	*recp = rec;

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क bloblist_ensurerec(uint चिह्नकम्, संरचना bloblist_rec **recp, पूर्णाङ्क आकार,
			      पूर्णाङ्क align_log2)
{
	संरचना bloblist_rec *rec;

	rec = bloblist_findrec(चिह्नकम्);
	यदि (rec) {
		यदि (आकार && आकार != rec->आकार) {
			*recp = rec;
			प्रत्यागम -ESPIPE;
		}
	} अन्यथा {
		पूर्णाङ्क ret;

		ret = bloblist_addrec(चिह्नकम्, आकार, align_log2, &rec);
		यदि (ret)
			प्रत्यागम ret;
	}
	*recp = rec;

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क bloblist_get_blob_दत्त_विस्थापनम्(uint चिह्नकम्)
{
	विकल्प (चिह्नकम्) {
	पक्ष BLOBLISTT_FDT_OVERLAY:
		प्रत्यागम परिमाण(संरचना dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	प्रत्यागम 0;
}

शून्य *bloblist_find(uint चिह्नकम्, पूर्णाङ्क आकार)
{
	शून्य *blob = रिक्त;
	पूर्णाङ्क blob_आकार;

	blob = bloblist_get_blob(चिह्नकम्, &blob_आकार);

	यदि (आकार && आकार != blob_आकार)
		प्रत्यागम रिक्त;

	प्रत्यागम blob;
}

शून्य *bloblist_get_blob(uint चिह्नकम्, पूर्णाङ्क *sizep)
{
	संरचना bloblist_rec *rec;

	rec = bloblist_findrec(चिह्नकम्);
	यदि (!rec)
		प्रत्यागम रिक्त;

	*sizep = rec->आकार;

	प्रत्यागम (शून्य *)rec + rec_hdr_आकार(rec);
}

पूर्णाङ्क bloblist_apply_blobs(uint चिह्नकम्, पूर्णाङ्क (*func)(शून्य **दत्त, पूर्णाङ्क आकार))
{
	संरचना bloblist_hdr *hdr = gd->bloblist;
	संरचना bloblist_rec *rec;

	यदि (!func || !hdr)
		प्रत्यागम -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		यदि (rec_चिह्नकम्(rec) == चिह्नकम्) {
			पूर्णाङ्क ret;
			पूर्णाङ्क चिह्नकम् = rec_चिह्नकम्(rec);
			शून्य *blob = (शून्य *)rec + rec_hdr_आकार(rec);
			पूर्णाङ्क dat_off = bloblist_get_blob_दत्त_विस्थापनम्(चिह्नकम्);

			blob += dat_off;
			ret = func(&blob, rec->आकार - dat_off);
			यदि (ret) {
				log_err("Failed to apply blob with tag %d\n",
					चिह्नकम्);
				प्रत्यागम ret;
			}

			rec = rec_from_blob(blob - dat_off);
			यदि (!rec) {
				log_err("Blob corrupted\n");
				प्रत्यागम -ENOENT;
			}

			/* Mark applied blob record as void */
			void_blob(rec);
		}
	}

	प्रत्यागम 0;
}

शून्य *bloblist_सङ्कलय(uint चिह्नकम्, पूर्णाङ्क आकार, पूर्णाङ्क align_log2)
{
	संरचना bloblist_rec *rec;

	यदि (bloblist_addrec(चिह्नकम्, आकार, align_log2, &rec))
		प्रत्यागम रिक्त;

	प्रत्यागम (शून्य *)rec + rec_hdr_आकार(rec);
}

पूर्णाङ्क bloblist_ensure_आकार(uint चिह्नकम्, पूर्णाङ्क आकार, पूर्णाङ्क align_log2, शून्य **blobp)
{
	संरचना bloblist_rec *rec;
	पूर्णाङ्क ret;

	ret = bloblist_ensurerec(चिह्नकम्, &rec, आकार, align_log2);
	यदि (ret)
		प्रत्यागम ret;
	*blobp = (शून्य *)rec + rec_hdr_आकार(rec);

	प्रत्यागम 0;
}

शून्य *bloblist_ensure(uint चिह्नकम्, पूर्णाङ्क आकार)
{
	संरचना bloblist_rec *rec;

	यदि (bloblist_ensurerec(चिह्नकम्, &rec, आकार, 0))
		प्रत्यागम रिक्त;

	प्रत्यागम (शून्य *)rec + rec_hdr_आकार(rec);
}

पूर्णाङ्क bloblist_ensure_आकार_ret(uint चिह्नकम्, पूर्णाङ्क *sizep, शून्य **blobp)
{
	संरचना bloblist_rec *rec;
	पूर्णाङ्क ret;

	ret = bloblist_ensurerec(चिह्नकम्, &rec, *sizep, 0);
	यदि (ret == -ESPIPE)
		*sizep = rec->आकार;
	अन्यथा यदि (ret)
		प्रत्यागम ret;
	*blobp = (शून्य *)rec + rec_hdr_आकार(rec);

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क bloblist_resize_rec(संरचना bloblist_hdr *hdr,
			       संरचना bloblist_rec *rec,
			       पूर्णाङ्क new_आकार)
{
	पूर्णाङ्क expand_by;	/* Number of bytes to expand by (-ve to contract) */
	पूर्णाङ्क new_alloced;
	ulong अग्र_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_आकार - rec->आकार, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->प्रयुक्तम्_आकार + expand_by, BLOBLIST_BLOB_ALIGN);
	यदि (new_आकार < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_आकार);
		प्रत्यागम log_सन्देशः_ret("size", -EINVAL);
	}
	यदि (new_alloced > hdr->योग_आकार) {
		log_err("Failed to allocate %x bytes\n", new_आकार);
		log_err("Used size=%x, total size=%x\n",
			hdr->प्रयुक्तम्_आकार, hdr->योग_आकार);
		प्रत्यागम log_सन्देशः_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	अग्र_ofs = bloblist_blob_अन्तः_ofs(hdr, rec);
	यदि (अग्र_ofs != hdr->प्रयुक्तम्_आकार) {
		memmove((शून्य *)hdr + अग्र_ofs + expand_by,
			(शून्य *)hdr + अग्र_ofs, hdr->प्रयुक्तम्_आकार - अग्र_ofs);
	}
	hdr->प्रयुक्तम्_आकार = new_alloced;

	/* Zero the new part of the blob */
	यदि (expand_by > 0) {
		स्मृतिं_पूरय((शून्य *)rec + rec_hdr_आकार(rec) + rec->आकार, '\0',
		       new_आकार - rec->आकार);
	}

	/* Update the size of this blob */
	rec->आकार = new_आकार;

	प्रत्यागम 0;
}

पूर्णाङ्क bloblist_resize(uint चिह्नकम्, पूर्णाङ्क new_आकार)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;
	संरचना bloblist_rec *rec;
	पूर्णाङ्क ret;

	rec = bloblist_findrec(चिह्नकम्);
	यदि (!rec)
		प्रत्यागम log_सन्देशः_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_आकार);
	यदि (ret)
		प्रत्यागम log_सन्देशः_ret("resize", ret);

	प्रत्यागम 0;
}

स्थिर u32 bloblist_calc_chksum(संरचना bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = सारणी_compute_checksum(hdr, hdr->प्रयुक्तम्_आकार);
	chksum += hdr->chksum;

	प्रत्यागम chksum;
}

पूर्णाङ्क bloblist_new(ulong addr, uint आकार, uint flags, uint align_log2)
{
	संरचना bloblist_hdr *hdr;

	यदि (आकार < परिमाण(*hdr))
		प्रत्यागम log_ret(-ENOSPC);
	यदि (addr & (BLOBLIST_ALIGN - 1))
		प्रत्यागम log_ret(-EFAULT);
	hdr = map_sysmem(addr, आकार);
	स्मृतिं_पूरय(hdr, '\0', परिमाण(*hdr));
	hdr->version = BLOBLIST_संस्करणम्;
	hdr->hdr_आकार = परिमाण(*hdr);
	hdr->flags = flags;
	hdr->magic = BLOBLIST_MAGIC;
	hdr->प्रयुक्तम्_आकार = hdr->hdr_आकार;
	hdr->योग_आकार = आकार;
	hdr->align_log2 = align_log2 ? align_log2 : BLOBLIST_BLOB_ALIGN_LOG2;
	hdr->chksum = 0;
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	प्रत्यागम 0;
}

पूर्णाङ्क bloblist_check(ulong addr, uint आकार)
{
	संरचना bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, परिमाण(*hdr));
	यदि (hdr->magic != BLOBLIST_MAGIC)
		प्रत्यागम log_सन्देशः_ret("Bad magic", -ENOENT);
	यदि (hdr->version != BLOBLIST_संस्करणम्)
		प्रत्यागम log_सन्देशः_ret("Bad version", -EPROTONOSUPPORT);
	यदि (!hdr->योग_आकार || (आकार && hdr->योग_आकार > आकार))
		प्रत्यागम log_सन्देशः_ret("Bad total size", -EFBIG);
	यदि (hdr->प्रयुक्तम्_आकार > hdr->योग_आकार)
		प्रत्यागम log_सन्देशः_ret("Bad used size", -ENOENT);
	यदि (hdr->hdr_आकार != परिमाण(संरचना bloblist_hdr))
		प्रत्यागम log_सन्देशः_ret("Bad header size", -ENOENT);

	chksum = bloblist_calc_chksum(hdr);
	यदि (hdr->chksum != chksum) {
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
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->प्रयुक्तम्_आकार,
		  (ulong)map_गन्तव्यम्_sysmem(hdr));

	प्रत्यागम 0;
}

ulong bloblist_get_आधारः(शून्य)
{
	प्रत्यागम map_गन्तव्यम्_sysmem(gd->bloblist);
}

ulong bloblist_get_आकार(शून्य)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;

	प्रत्यागम hdr->प्रयुक्तम्_आकार;
}

ulong bloblist_get_योग_आकार(शून्य)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;

	प्रत्यागम hdr->योग_आकार;
}

शून्य bloblist_get_stats(ulong *basep, ulong *tsizep, ulong *usizep)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;

	*basep = map_गन्तव्यम्_sysmem(gd->bloblist);
	*tsizep = hdr->योग_आकार;
	*usizep = hdr->प्रयुक्तम्_आकार;
}

स्थिर शून्य show_मानम्(ध्रुव वर्ण *prompt, ulong मानम्)
{
	प्रारूप_लिख("%s:%*s %-5lx  ", prompt, 10 - (पूर्णाङ्क)वाक्यदैर्घ्यम्(prompt), "", मानम्);
	print_आकार(मानम्, "\n");
}

शून्य bloblist_show_stats(शून्य)
{
	ulong आधारः, tsize, usize;

	bloblist_get_stats(&आधारः, &tsize, &usize);
	प्रारूप_लिख("base:       %lx\n", आधारः);
	show_मानम्("total size", tsize);
	show_मानम्("used size", usize);
	show_मानम्("free", tsize - usize);
}

शून्य bloblist_show_list(शून्य)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;
	संरचना bloblist_rec *rec;

	प्रारूप_लिख("%-8s  %8s   Tag Name\n", "Address", "Size");
	प्रति (rec = bloblist_first_blob(hdr); rec;
	     rec = bloblist_अग्र_blob(hdr, rec)) {
		प्रारूप_लिख("%08lx  %8x  %4x %s\n",
		       (ulong)map_गन्तव्यम्_sysmem((शून्य *)rec + rec_hdr_आकार(rec)),
		       rec->आकार, rec_चिह्नकम्(rec),
		       bloblist_चिह्नकम्_name(rec_चिह्नकम्(rec)));
	}
}

पूर्णाङ्क bloblist_reloc(शून्य *गन्तव्यम्, uint गन्तव्यम्_आकार)
{
	संरचना bloblist_hdr *hdr;

	यदि (!गन्तव्यम्_आकार)
		प्रत्यागम 0;

	यदि (गन्तव्यम्_आकार < gd->bloblist->योग_आकार)
		प्रत्यागम -ENOSPC;

	स्मृतिं_प्रतिलिख(गन्तव्यम्, gd->bloblist, gd->bloblist->योग_आकार);
	hdr = गन्तव्यम्;
	hdr->योग_आकार = गन्तव्यम्_आकार;
	gd->bloblist = गन्तव्यम्;

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
	यदि (!xferlist_from_boot_arg(&addr))
		गच्छ प्राप्तम्;

	/*
	 * If Firmware Handoff is mandatory but no transfer list is
	 * observed, report it as an error.
	 */
	यदि (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		प्रत्यागम false;

	/*
	 * We have checked for a valid transfer list being passed. At this
	 * point, if we do not have a fixed address for the bloblist, we cannot
	 * be provided with one.
	 */
	यदि (xpl_is_first_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		प्रत्यागम false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_आकार);
	यदि (!ret)
		गच्छ प्राप्तम्;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	प्रत्यागम false;

प्राप्तम्:
#यदि_परिभाषितम् DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#यदि_अन्तः
	प्रत्यागम true;
}

पूर्णाङ्क bloblist_init(शून्य)
{
	पूर्णाङ्क ret;
	ulong addr = 0, आकार = CONFIG_BLOBLIST_आकार;

	यदि (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->योग_आकार, gd->bloblist);
		प्रत्यागम 0;
	}

	/*
	 * If Firmware Handoff is mandatory but no transfer list has been
	 * observed by fdtdec_setup, report it as an error.
	 */
	यदि (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		प्रत्यागम -ENOENT;

	/*
	 * If we don't have an existing bloblist, we either need
	 * to allocate one now, or initialize the fixed address
	 * space as a bloblist.
	 */
	यदि (CONFIG_IS_ENABLED(BLOBLIST_ALLOC)) {
		शून्य *ptr = memalign(BLOBLIST_ALIGN, आकार);

		यदि (!ptr)
			प्रत्यागम log_सन्देशः_ret("alloc", -ENOMEM);
		addr = map_गन्तव्यम्_sysmem(ptr);
	} अन्यथा
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", आकार,
		  addr);
	ret = bloblist_new(addr, आकार, 0, 0);
	यदि (ret)
		प्रत्यागम log_सन्देशः_ret("ini", ret);

	प्रत्यागम 0;
}

पूर्णाङ्क bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	पूर्णाङ्क ret;

	यदि ((IS_ENABLED(CONFIG_64BIT) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64BIT) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_सरणम्_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_आवरणम्) << BLOBLIST_REGCONV_सरणम्_64));
	} अन्यथा {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_सरणम्_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_आवरणम्) << BLOBLIST_REGCONV_सरणम्_32));
	}

	यदि (rzero || rsig != sigval)
		प्रत्यागम -EIO;

	ret = bloblist_check(xlist, 0);
	यदि (ret)
		प्रत्यागम ret;

	यदि (rfdt != (ulong)bloblist_find(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = रिक्त;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		प्रत्यागम -EIO;
	}

	प्रत्यागम 0;
}
