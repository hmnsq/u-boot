// SPDX-License-Identifier: GPL-2.0+ BSD-3-Clause
/*
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#నిర్వచించు LOG_CATEGORY	LOGC_BLOBLIST

#అన్తర్భావ <bloblist.h>
#అన్తర్భావ <display_options.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <malloc.h>
#అన్తర్భావ <mapmem.h>
#అన్తర్భావ <spl.h>
#అన్తర్భావ <tables_csum.h>
#అన్తర్భావ <asm/global_data.h>
#అన్తర్భావ <u-boot/crc.h>

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

DECLARE_GLOBAL_దత్తాంశం_PTR;

స్థిర నిర్మాణం ట్యాగ్_name {
	గణన bloblist_ట్యాగ్_t ట్యాగ్;
	స్థిరరాశి అక్షరం *name;
} ట్యాగ్_name[] = {
	{ BLOBLISTT_VOID, "(void)" },

	/* BLOBLISTT_AREA_FIRMWARE_TOP */
	{ BLOBLISTT_CONTROL_FDT, "Control FDT" },
	{ BLOBLISTT_HOB_ఖండం, "HOB block" },
	{ BLOBLISTT_HOB_LIST, "HOB list" },
	{ BLOBLISTT_ACPI_TABLES, "ACPI tables for x86" },
	{ BLOBLISTT_TPM_EVLOG, "TPM event log defined by TCG EFI" },
	{ BLOBLISTT_TPM_CRB_పునాది, "TPM Command Response Buffer address" },
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

స్థిరరాశి అక్షరం *bloblist_ట్యాగ్_name(గణన bloblist_ట్యాగ్_t ట్యాగ్)
{
	పూర్ణాంకం i;

	కొరకు (i = 0; i < ARRAY_కొలత(ట్యాగ్_name); i++) {
		ఒకవేళ (ట్యాగ్_name[i].ట్యాగ్ == ట్యాగ్)
			తిరిగి ట్యాగ్_name[i].name;
	}

	తిరిగి "invalid";
}

స్థిర నిర్మాణం bloblist_rec *bloblist_మొదటి_blob(నిర్మాణం bloblist_hdr *hdr)
{
	ఒకవేళ (hdr->వాడిన_కొలత <= hdr->hdr_కొలత)
		తిరిగి ఖాళీ;
	తిరిగి (నిర్మాణం bloblist_rec *)((శూన్య *)hdr + hdr->hdr_కొలత);
}

స్థిర అంతఃవరుస uint rec_hdr_కొలత(నిర్మాణం bloblist_rec *rec)
{
	తిరిగి (rec->ట్యాగ్_and_hdr_కొలత & BLOBLISTR_HDR_కొలత_ముసుగు) >>
		BLOBLISTR_HDR_కొలత_జరుపు;
}

స్థిర అంతఃవరుస uint rec_ట్యాగ్(నిర్మాణం bloblist_rec *rec)
{
	తిరిగి (rec->ట్యాగ్_and_hdr_కొలత & BLOBLISTR_ట్యాగ్_ముసుగు) >>
		BLOBLISTR_ట్యాగ్_జరుపు;
}

స్థిర అంతఃవరుస శూన్య void_blob(నిర్మాణం bloblist_rec *rec)
{
	ఒకవేళ (rec_ట్యాగ్(rec) == BLOBLISTT_VOID)
		తిరిగి;
	rec->ట్యాగ్_and_hdr_కొలత = BLOBLISTT_VOID |
				పరిమాణం(*rec) << BLOBLISTR_HDR_కొలత_జరుపు;
}

స్థిర అంతఃవరుస నిర్మాణం bloblist_rec *rec_from_blob(శూన్య *blob)
{
	తిరిగి (blob - పరిమాణం(నిర్మాణం bloblist_rec));
}

స్థిర ulong bloblist_blob_చివర_ofs(నిర్మాణం bloblist_hdr *hdr,
				   నిర్మాణం bloblist_rec *rec)
{
	ulong స్థానభ్రంశం;

	స్థానభ్రంశం = (శూన్య *)rec - (శూన్య *)hdr;
	/*
	 * The data section of next TE should start from an address aligned
	 * to 1 << hdr->align_log2.
	 */
	స్థానభ్రంశం += rec_hdr_కొలత(rec) + rec->కొలత;
	స్థానభ్రంశం = విడత_పైకి(స్థానభ్రంశం + rec_hdr_కొలత(rec), 1 << hdr->align_log2);
	స్థానభ్రంశం -= rec_hdr_కొలత(rec);

	తిరిగి స్థానభ్రంశం;
}

స్థిర నిర్మాణం bloblist_rec *bloblist_తదుపరి_blob(నిర్మాణం bloblist_hdr *hdr,
					       నిర్మాణం bloblist_rec *rec)
{
	ulong స్థానభ్రంశం = bloblist_blob_చివర_ofs(hdr, rec);

	ఒకవేళ (స్థానభ్రంశం >= hdr->వాడిన_కొలత)
		తిరిగి ఖాళీ;
	తిరిగి (నిర్మాణం bloblist_rec *)((శూన్య *)hdr + స్థానభ్రంశం);
}

#నిర్వచించు foreach_rec(_rec, _hdr) \
	కొరకు (_rec = bloblist_మొదటి_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_తదుపరి_blob(_hdr, _rec))

స్థిర నిర్మాణం bloblist_rec *bloblist_findrec(uint ట్యాగ్)
{
	నిర్మాణం bloblist_hdr *hdr = gd->bloblist;
	నిర్మాణం bloblist_rec *rec;

	ఒకవేళ (!hdr)
		తిరిగి ఖాళీ;

	foreach_rec(rec, hdr) {
		ఒకవేళ (rec_ట్యాగ్(rec) == ట్యాగ్)
			తిరిగి rec;
	}

	తిరిగి ఖాళీ;
}

స్థిర పూర్ణాంకం bloblist_addrec(uint ట్యాగ్, పూర్ణాంకం కొలత, పూర్ణాంకం align_log2,
			   నిర్మాణం bloblist_rec **recp)
{
	నిర్మాణం bloblist_hdr *hdr = gd->bloblist;
	నిర్మాణం bloblist_rec *rec;
	phys_addr_t దత్తాంశం_ప్రారంభం, aligned_ప్రారంభం;
	phys_కొలత_t new_alloced;

	ఒకవేళ (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	దత్తాంశం_ప్రారంభం = map_గమ్యం_sysmem(hdr) + hdr->వాడిన_కొలత + పరిమాణం(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_ప్రారంభం = ALIGN(దత్తాంశం_ప్రారంభం, 1U << align_log2) - దత్తాంశం_ప్రారంభం;

	/* If we need to create a dummy record, create it */
	ఒకవేళ (aligned_ప్రారంభం) {
		పూర్ణాంకం void_కొలత = aligned_ప్రారంభం - పరిమాణం(*rec);
		నిర్మాణం bloblist_rec *vrec;
		పూర్ణాంకం ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_కొలత, 0, &vrec);
		ఒకవేళ (ret)
			తిరిగి log_సందేశం_ret("void", ret);

		/* start the record after that */
		దత్తాంశం_ప్రారంభం = map_గమ్యం_sysmem(hdr) + hdr->వాడిన_కొలత + పరిమాణం(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = దత్తాంశం_ప్రారంభం - map_గమ్యం_sysmem(hdr) +
		ALIGN(కొలత, 1U << align_log2);

	ఒకవేళ (new_alloced > hdr->మొత్తం_కొలత) {
		log_err("Failed to allocate %x bytes\n", కొలత);
		log_err("Used size=%x, total size=%x\n",
			hdr->వాడిన_కొలత, hdr->మొత్తం_కొలత);
		తిరిగి log_సందేశం_ret("bloblist add", -ENOSPC);
	}
	rec = (శూన్య *)hdr + hdr->వాడిన_కొలత;

	rec->ట్యాగ్_and_hdr_కొలత = ట్యాగ్ | పరిమాణం(*rec) << BLOBLISTR_HDR_కొలత_జరుపు;
	rec->కొలత = కొలత;

	/* Zero the record data */
	memset((శూన్య *)rec + rec_hdr_కొలత(rec), '\0', rec->కొలత);

	hdr->వాడిన_కొలత = new_alloced;
	*recp = rec;

	తిరిగి 0;
}

స్థిర పూర్ణాంకం bloblist_ensurerec(uint ట్యాగ్, నిర్మాణం bloblist_rec **recp, పూర్ణాంకం కొలత,
			      పూర్ణాంకం align_log2)
{
	నిర్మాణం bloblist_rec *rec;

	rec = bloblist_findrec(ట్యాగ్);
	ఒకవేళ (rec) {
		ఒకవేళ (కొలత && కొలత != rec->కొలత) {
			*recp = rec;
			తిరిగి -ESPIPE;
		}
	} లేకపోతే {
		పూర్ణాంకం ret;

		ret = bloblist_addrec(ట్యాగ్, కొలత, align_log2, &rec);
		ఒకవేళ (ret)
			తిరిగి ret;
	}
	*recp = rec;

	తిరిగి 0;
}

స్థిర పూర్ణాంకం bloblist_get_blob_దత్తాంశం_స్థానభ్రంశం(uint ట్యాగ్)
{
	ఎంపిక (ట్యాగ్) {
	స్థితి BLOBLISTT_FDT_OVERLAY:
		తిరిగి పరిమాణం(నిర్మాణం dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	తిరిగి 0;
}

శూన్య *bloblist_కనుగొను(uint ట్యాగ్, పూర్ణాంకం కొలత)
{
	శూన్య *blob = ఖాళీ;
	పూర్ణాంకం blob_కొలత;

	blob = bloblist_get_blob(ట్యాగ్, &blob_కొలత);

	ఒకవేళ (కొలత && కొలత != blob_కొలత)
		తిరిగి ఖాళీ;

	తిరిగి blob;
}

శూన్య *bloblist_get_blob(uint ట్యాగ్, పూర్ణాంకం *sizep)
{
	నిర్మాణం bloblist_rec *rec;

	rec = bloblist_findrec(ట్యాగ్);
	ఒకవేళ (!rec)
		తిరిగి ఖాళీ;

	*sizep = rec->కొలత;

	తిరిగి (శూన్య *)rec + rec_hdr_కొలత(rec);
}

పూర్ణాంకం bloblist_apply_blobs(uint ట్యాగ్, పూర్ణాంకం (*func)(శూన్య **దత్తాంశం, పూర్ణాంకం కొలత))
{
	నిర్మాణం bloblist_hdr *hdr = gd->bloblist;
	నిర్మాణం bloblist_rec *rec;

	ఒకవేళ (!func || !hdr)
		తిరిగి -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		ఒకవేళ (rec_ట్యాగ్(rec) == ట్యాగ్) {
			పూర్ణాంకం ret;
			పూర్ణాంకం ట్యాగ్ = rec_ట్యాగ్(rec);
			శూన్య *blob = (శూన్య *)rec + rec_hdr_కొలత(rec);
			పూర్ణాంకం dat_off = bloblist_get_blob_దత్తాంశం_స్థానభ్రంశం(ట్యాగ్);

			blob += dat_off;
			ret = func(&blob, rec->కొలత - dat_off);
			ఒకవేళ (ret) {
				log_err("Failed to apply blob with tag %d\n",
					ట్యాగ్);
				తిరిగి ret;
			}

			rec = rec_from_blob(blob - dat_off);
			ఒకవేళ (!rec) {
				log_err("Blob corrupted\n");
				తిరిగి -ENOENT;
			}

			/* Mark applied blob record as void */
			void_blob(rec);
		}
	}

	తిరిగి 0;
}

శూన్య *bloblist_కూడు(uint ట్యాగ్, పూర్ణాంకం కొలత, పూర్ణాంకం align_log2)
{
	నిర్మాణం bloblist_rec *rec;

	ఒకవేళ (bloblist_addrec(ట్యాగ్, కొలత, align_log2, &rec))
		తిరిగి ఖాళీ;

	తిరిగి (శూన్య *)rec + rec_hdr_కొలత(rec);
}

పూర్ణాంకం bloblist_ensure_కొలత(uint ట్యాగ్, పూర్ణాంకం కొలత, పూర్ణాంకం align_log2, శూన్య **blobp)
{
	నిర్మాణం bloblist_rec *rec;
	పూర్ణాంకం ret;

	ret = bloblist_ensurerec(ట్యాగ్, &rec, కొలత, align_log2);
	ఒకవేళ (ret)
		తిరిగి ret;
	*blobp = (శూన్య *)rec + rec_hdr_కొలత(rec);

	తిరిగి 0;
}

శూన్య *bloblist_ensure(uint ట్యాగ్, పూర్ణాంకం కొలత)
{
	నిర్మాణం bloblist_rec *rec;

	ఒకవేళ (bloblist_ensurerec(ట్యాగ్, &rec, కొలత, 0))
		తిరిగి ఖాళీ;

	తిరిగి (శూన్య *)rec + rec_hdr_కొలత(rec);
}

పూర్ణాంకం bloblist_ensure_కొలత_ret(uint ట్యాగ్, పూర్ణాంకం *sizep, శూన్య **blobp)
{
	నిర్మాణం bloblist_rec *rec;
	పూర్ణాంకం ret;

	ret = bloblist_ensurerec(ట్యాగ్, &rec, *sizep, 0);
	ఒకవేళ (ret == -ESPIPE)
		*sizep = rec->కొలత;
	లేకపోతే ఒకవేళ (ret)
		తిరిగి ret;
	*blobp = (శూన్య *)rec + rec_hdr_కొలత(rec);

	తిరిగి 0;
}

స్థిర పూర్ణాంకం bloblist_resize_rec(నిర్మాణం bloblist_hdr *hdr,
			       నిర్మాణం bloblist_rec *rec,
			       పూర్ణాంకం new_కొలత)
{
	పూర్ణాంకం expand_by;	/* Number of bytes to expand by (-ve to contract) */
	పూర్ణాంకం new_alloced;
	ulong తదుపరి_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_కొలత - rec->కొలత, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->వాడిన_కొలత + expand_by, BLOBLIST_BLOB_ALIGN);
	ఒకవేళ (new_కొలత < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_కొలత);
		తిరిగి log_సందేశం_ret("size", -EINVAL);
	}
	ఒకవేళ (new_alloced > hdr->మొత్తం_కొలత) {
		log_err("Failed to allocate %x bytes\n", new_కొలత);
		log_err("Used size=%x, total size=%x\n",
			hdr->వాడిన_కొలత, hdr->మొత్తం_కొలత);
		తిరిగి log_సందేశం_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	తదుపరి_ofs = bloblist_blob_చివర_ofs(hdr, rec);
	ఒకవేళ (తదుపరి_ofs != hdr->వాడిన_కొలత) {
		memmove((శూన్య *)hdr + తదుపరి_ofs + expand_by,
			(శూన్య *)hdr + తదుపరి_ofs, hdr->వాడిన_కొలత - తదుపరి_ofs);
	}
	hdr->వాడిన_కొలత = new_alloced;

	/* Zero the new part of the blob */
	ఒకవేళ (expand_by > 0) {
		memset((శూన్య *)rec + rec_hdr_కొలత(rec) + rec->కొలత, '\0',
		       new_కొలత - rec->కొలత);
	}

	/* Update the size of this blob */
	rec->కొలత = new_కొలత;

	తిరిగి 0;
}

పూర్ణాంకం bloblist_resize(uint ట్యాగ్, పూర్ణాంకం new_కొలత)
{
	నిర్మాణం bloblist_hdr *hdr = gd->bloblist;
	నిర్మాణం bloblist_rec *rec;
	పూర్ణాంకం ret;

	rec = bloblist_findrec(ట్యాగ్);
	ఒకవేళ (!rec)
		తిరిగి log_సందేశం_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_కొలత);
	ఒకవేళ (ret)
		తిరిగి log_సందేశం_ret("resize", ret);

	తిరిగి 0;
}

స్థిర u32 bloblist_calc_chksum(నిర్మాణం bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = పట్టిక_compute_checksum(hdr, hdr->వాడిన_కొలత);
	chksum += hdr->chksum;

	తిరిగి chksum;
}

పూర్ణాంకం bloblist_new(ulong addr, uint కొలత, uint flags, uint align_log2)
{
	నిర్మాణం bloblist_hdr *hdr;

	ఒకవేళ (కొలత < పరిమాణం(*hdr))
		తిరిగి log_ret(-ENOSPC);
	ఒకవేళ (addr & (BLOBLIST_ALIGN - 1))
		తిరిగి log_ret(-EFAULT);
	hdr = map_sysmem(addr, కొలత);
	memset(hdr, '\0', పరిమాణం(*hdr));
	hdr->version = BLOBLIST_సంస్కరణ;
	hdr->hdr_కొలత = పరిమాణం(*hdr);
	hdr->flags = flags;
	hdr->magic = BLOBLIST_MAGIC;
	hdr->వాడిన_కొలత = hdr->hdr_కొలత;
	hdr->మొత్తం_కొలత = కొలత;
	hdr->align_log2 = align_log2 ? align_log2 : BLOBLIST_BLOB_ALIGN_LOG2;
	hdr->chksum = 0;
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	తిరిగి 0;
}

పూర్ణాంకం bloblist_check(ulong addr, uint కొలత)
{
	నిర్మాణం bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, పరిమాణం(*hdr));
	ఒకవేళ (hdr->magic != BLOBLIST_MAGIC)
		తిరిగి log_సందేశం_ret("Bad magic", -ENOENT);
	ఒకవేళ (hdr->version != BLOBLIST_సంస్కరణ)
		తిరిగి log_సందేశం_ret("Bad version", -EPROTONOSUPPORT);
	ఒకవేళ (!hdr->మొత్తం_కొలత || (కొలత && hdr->మొత్తం_కొలత > కొలత))
		తిరిగి log_సందేశం_ret("Bad total size", -EFBIG);
	ఒకవేళ (hdr->వాడిన_కొలత > hdr->మొత్తం_కొలత)
		తిరిగి log_సందేశం_ret("Bad used size", -ENOENT);
	ఒకవేళ (hdr->hdr_కొలత != పరిమాణం(నిర్మాణం bloblist_hdr))
		తిరిగి log_సందేశం_ret("Bad header size", -ENOENT);

	chksum = bloblist_calc_chksum(hdr);
	ఒకవేళ (hdr->chksum != chksum) {
		log_err("Checksum %x != %x\n", hdr->chksum, chksum);
		తిరిగి log_సందేశం_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	తిరిగి 0;
}

పూర్ణాంకం bloblist_ముగింపు(శూన్య)
{
	నిర్మాణం bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->వాడిన_కొలత,
		  (ulong)map_గమ్యం_sysmem(hdr));

	తిరిగి 0;
}

ulong bloblist_get_పునాది(శూన్య)
{
	తిరిగి map_గమ్యం_sysmem(gd->bloblist);
}

ulong bloblist_get_కొలత(శూన్య)
{
	నిర్మాణం bloblist_hdr *hdr = gd->bloblist;

	తిరిగి hdr->వాడిన_కొలత;
}

ulong bloblist_get_మొత్తం_కొలత(శూన్య)
{
	నిర్మాణం bloblist_hdr *hdr = gd->bloblist;

	తిరిగి hdr->మొత్తం_కొలత;
}

శూన్య bloblist_get_stats(ulong *basep, ulong *tsizep, ulong *usizep)
{
	నిర్మాణం bloblist_hdr *hdr = gd->bloblist;

	*basep = map_గమ్యం_sysmem(gd->bloblist);
	*tsizep = hdr->మొత్తం_కొలత;
	*usizep = hdr->వాడిన_కొలత;
}

స్థిర శూన్య show_విలువ(స్థిరరాశి అక్షరం *prompt, ulong విలువ)
{
	రాయి("%s:%*s %-5lx  ", prompt, 10 - (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(prompt), "", విలువ);
	print_కొలత(విలువ, "\n");
}

శూన్య bloblist_show_stats(శూన్య)
{
	ulong పునాది, tsize, usize;

	bloblist_get_stats(&పునాది, &tsize, &usize);
	రాయి("base:       %lx\n", పునాది);
	show_విలువ("total size", tsize);
	show_విలువ("used size", usize);
	show_విలువ("free", tsize - usize);
}

శూన్య bloblist_show_list(శూన్య)
{
	నిర్మాణం bloblist_hdr *hdr = gd->bloblist;
	నిర్మాణం bloblist_rec *rec;

	రాయి("%-8s  %8s   Tag Name\n", "Address", "Size");
	కొరకు (rec = bloblist_మొదటి_blob(hdr); rec;
	     rec = bloblist_తదుపరి_blob(hdr, rec)) {
		రాయి("%08lx  %8x  %4x %s\n",
		       (ulong)map_గమ్యం_sysmem((శూన్య *)rec + rec_hdr_కొలత(rec)),
		       rec->కొలత, rec_ట్యాగ్(rec),
		       bloblist_ట్యాగ్_name(rec_ట్యాగ్(rec)));
	}
}

పూర్ణాంకం bloblist_reloc(శూన్య *గమ్యం, uint గమ్యం_కొలత)
{
	నిర్మాణం bloblist_hdr *hdr;

	ఒకవేళ (!గమ్యం_కొలత)
		తిరిగి 0;

	ఒకవేళ (గమ్యం_కొలత < gd->bloblist->మొత్తం_కొలత)
		తిరిగి -ENOSPC;

	జ్ఞాపకం_నకలు(గమ్యం, gd->bloblist, gd->bloblist->మొత్తం_కొలత);
	hdr = గమ్యం;
	hdr->మొత్తం_కొలత = గమ్యం_కొలత;
	gd->bloblist = గమ్యం;

	తిరిగి 0;
}

/*
 * Weak default function for getting bloblist from boot args.
 */
పూర్ణాంకం __weak xferlist_from_boot_arg(ulong __always_unused *addr)
{
	తిరిగి -ENOENT;
}

bool bloblist_exists(శూన్య)
{
	పూర్ణాంకం ret;
	ulong addr = 0;

	/* Check if a valid transfer list passed in */
	ఒకవేళ (!xferlist_from_boot_arg(&addr))
		వెళ్ళు దొరికింది;

	/*
	 * If Firmware Handoff is mandatory but no transfer list is
	 * observed, report it as an error.
	 */
	ఒకవేళ (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		తిరిగి false;

	/*
	 * We have checked for a valid transfer list being passed. At this
	 * point, if we do not have a fixed address for the bloblist, we cannot
	 * be provided with one.
	 */
	ఒకవేళ (xpl_is_మొదటి_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		తిరిగి false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_కొలత);
	ఒకవేళ (!ret)
		వెళ్ళు దొరికింది;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	తిరిగి false;

దొరికింది:
#నిర్వచితమైతే DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#అయితే_ముగింపు
	తిరిగి true;
}

పూర్ణాంకం bloblist_init(శూన్య)
{
	పూర్ణాంకం ret;
	ulong addr = 0, కొలత = CONFIG_BLOBLIST_కొలత;

	ఒకవేళ (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->మొత్తం_కొలత, gd->bloblist);
		తిరిగి 0;
	}

	/*
	 * If Firmware Handoff is mandatory but no transfer list has been
	 * observed by fdtdec_setup, report it as an error.
	 */
	ఒకవేళ (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		తిరిగి -ENOENT;

	/*
	 * If we don't have an existing bloblist, we either need
	 * to allocate one now, or initialize the fixed address
	 * space as a bloblist.
	 */
	ఒకవేళ (CONFIG_IS_ENABLED(BLOBLIST_ALLOC)) {
		శూన్య *ptr = memalign(BLOBLIST_ALIGN, కొలత);

		ఒకవేళ (!ptr)
			తిరిగి log_సందేశం_ret("alloc", -ENOMEM);
		addr = map_గమ్యం_sysmem(ptr);
	} లేకపోతే
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", కొలత,
		  addr);
	ret = bloblist_new(addr, కొలత, 0, 0);
	ఒకవేళ (ret)
		తిరిగి log_సందేశం_ret("ini", ret);

	తిరిగి 0;
}

పూర్ణాంకం bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	పూర్ణాంకం ret;

	ఒకవేళ ((IS_ENABLED(CONFIG_64_బిట్టు) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64_బిట్టు) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_జరుపు_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_ముసుగు) << BLOBLIST_REGCONV_జరుపు_64));
	} లేకపోతే {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_జరుపు_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_ముసుగు) << BLOBLIST_REGCONV_జరుపు_32));
	}

	ఒకవేళ (rzero || rsig != sigval)
		తిరిగి -EIO;

	ret = bloblist_check(xlist, 0);
	ఒకవేళ (ret)
		తిరిగి ret;

	ఒకవేళ (rfdt != (ulong)bloblist_కనుగొను(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = ఖాళీ;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		తిరిగి -EIO;
	}

	తిరిగి 0;
}
