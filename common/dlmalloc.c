// SPDX-License-Identifier: GPL-2.0+
/*
 * This code is based on a version (aka dlmalloc) of malloc/free/realloc written
 * by Doug Lea and released to the public domain, as explained at
 * http://creativecommons.org/publicdomain/zero/1.0/-
 *
 * The original code is available at http://gee.cs.oswego.edu/pub/misc/
 * as file malloc-2.6.6.c.
 */

#ಆದರೆ CONFIG_IS_ENABLED(UNIT_TEST)
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>

#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <asm/io.h>
#ಅನ್ತರ್ಭಾವ <valgrind/memcheck.h>

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DEBUG
#ಆದರೆ __STD_C
ಸ್ಥಿರ ಶೂನ್ಯ malloc_update_mallinfo (ಶೂನ್ಯ);
ಶೂನ್ಯ malloc_stats (ಶೂನ್ಯ);
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಶೂನ್ಯ malloc_update_mallinfo ();
ಶೂನ್ಯ malloc_stats();
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ	/* DEBUG */

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ MCHECK_ರಾಶಿ_PROTECTION
 #ವ್ಯಾಖ್ಯಾನಿಸು STATIC_IF_MCHECK ಸ್ಥಿರ
 #ವ್ಯಾಖ್ಯಾನ_ರದ್ದು MALLOC_COPY
 #ವ್ಯಾಖ್ಯಾನ_ರದ್ದು MALLOC_ZERO
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ MALLOC_ZERO(ಶೂನ್ಯ *p, size_t sz) { memset(p, 0, sz); }
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ MALLOC_COPY(ಶೂನ್ಯ *dest, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *src, size_t sz) { ಸ್ಮರಣೆ_ನಕಲು(dest, src, sz); }
#ಇಲ್ಲದಿದ್ದರೆ
 #ವ್ಯಾಖ್ಯಾನಿಸು STATIC_IF_MCHECK
 #ವ್ಯಾಖ್ಯಾನಿಸು mALLOc_impl mALLOc
 #ವ್ಯಾಖ್ಯಾನಿಸು fREe_impl fREe
 #ವ್ಯಾಖ್ಯಾನಿಸು rEALLOc_impl rEALLOc
 #ವ್ಯಾಖ್ಯಾನಿಸು mEMALIGn_impl mEMALIGn
 #ವ್ಯಾಖ್ಯಾನಿಸು cALLOc_impl cALLOc
#ಆದರೆ_ಅಂತ್ಯ

/*
  Emulation of sbrk for WIN32
  All code within the ifdef WIN32 is untested by me.

  Thanks to Martin Fong and others for supplying this.
*/

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ WIN32

#ವ್ಯಾಖ್ಯಾನಿಸು Align_ಪುಟ(ಕೂಡಿಸು) (((ಕೂಡಿಸು) + (malloc_getpagesize-1)) & \
~(malloc_getpagesize-1))
#ವ್ಯಾಖ್ಯಾನಿಸು AlignPage64K(ಕೂಡಿಸು) (((ಕೂಡಿಸು) + (0x10000 - 1)) & ~(0x10000 - 1))

/* resrve 64MB to insure large contiguous space */
#ವ್ಯಾಖ್ಯಾನಿಸು RESERVED_ಗಾತ್ರ (1024*1024*64)
#ವ್ಯಾಖ್ಯಾನಿಸು ಮುಂದಿನ_ಗಾತ್ರ (2048*1024)
#ವ್ಯಾಖ್ಯಾನಿಸು ತುದಿ_ಸ್ಮರಣೆ ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)2*1024*1024*1024)

ರಚನೆ GmListElement;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ GmListElement GmListElement;

ರಚನೆ GmListElement
{
	GmListElement* ಮುಂದಿನ;
	ಶೂನ್ಯ* ತಳ;
};

ಸ್ಥಿರ GmListElement* ತಲೆ = 0;
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ g_ಮುಂದಿನ_ವಿಳಾಸ = 0;
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ g_ವಿಳಾಸ_ತಳ = 0;
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ g_Allocated_ಗಾತ್ರ = 0;

ಸ್ಥಿರ
GmListElement* makeGmListElement (ಶೂನ್ಯ* bas)
{
	GmListElement* this;
	this = (GmListElement*)(ಶೂನ್ಯ*)LocalAlloc (0, ಗಾತ್ರ (GmListElement));
	ಪ್ರತಿಜ್ಞಾ (this);
	ಆದರೆ (this)
	{
		this->ತಳ = bas;
		this->ಮುಂದಿನ = ತಲೆ;
		ತಲೆ = this;
	}
	ಹಿಂತಿರುಗಿ this;
}

ಶೂನ್ಯ gcleanup (ಶೂನ್ಯ)
{
	BOOL rval;
	ಪ್ರತಿಜ್ಞಾ ( (ತಲೆ == ಖಾಲಿ) || (ತಲೆ->ತಳ == (ಶೂನ್ಯ*)g_ವಿಳಾಸ_ತಳ));
	ಆದರೆ (g_ವಿಳಾಸ_ತಳ && (g_ಮುಂದಿನ_ವಿಳಾಸ - g_ವಿಳಾಸ_ತಳ))
	{
		rval = VirtualFree ((ಶೂನ್ಯ*)g_ವಿಳಾಸ_ತಳ,
							g_ಮುಂದಿನ_ವಿಳಾಸ - g_ವಿಳಾಸ_ತಳ,
							MEM_DECOMMIT);
	ಪ್ರತಿಜ್ಞಾ (rval);
	}
	ಆಗ (ತಲೆ)
	{
		GmListElement* ಮುಂದಿನ = ತಲೆ->ಮುಂದಿನ;
		rval = VirtualFree (ತಲೆ->ತಳ, 0, MEM_RELEASE);
		ಪ್ರತಿಜ್ಞಾ (rval);
		LocalFree (ತಲೆ);
		ತಲೆ = ಮುಂದಿನ;
	}
}

ಸ್ಥಿರ
ಶೂನ್ಯ* ಕಂಡುಹಿಡಿ_Region (ಶೂನ್ಯ* ಆರಂಭ_ವಿಳಾಸ, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಗಾತ್ರ_2)
{
	ಸ್ಮರಣೆ_BASIC_INFORMATION info;
	ಆದರೆ (ಗಾತ್ರ_2 >= ತುದಿ_ಸ್ಮರಣೆ) ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆಗ ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)ಆರಂಭ_ವಿಳಾಸ + ಗಾತ್ರ_2 < ತುದಿ_ಸ್ಮರಣೆ)
	{
		VirtualQuery (ಆರಂಭ_ವಿಳಾಸ, &info, ಗಾತ್ರ (info));
		ಆದರೆ ((info.State == MEM_FREE) && (info.Region_ಗಾತ್ರ >= ಗಾತ್ರ_2))
			ಹಿಂತಿರುಗಿ ಆರಂಭ_ವಿಳಾಸ;
		ಇಲ್ಲದಿದ್ದರೆ
		{
			/* Requested region is not available so see if the */
			/* next region is available.  Set 'start_address' */
			/* to the next region and call 'VirtualQuery()' */
			/* again. */

			ಆರಂಭ_ವಿಳಾಸ = (ಅಕ್ಷರ*)info.ತಳ_ವಿಳಾಸ + info.Region_ಗಾತ್ರ;

			/* Make sure we start looking for the next region */
			/* on the *next* 64K boundary.  Otherwise, even if */
			/* the new region is free according to */
			/* 'VirtualQuery()', the subsequent call to */
			/* 'VirtualAlloc()' (which follows the call to */
			/* this routine in 'wsbrk()') will round *down* */
			/* the requested address to a 64K boundary which */
			/* we already know is an address in the */
			/* unavailable region.  Thus, the subsequent call */
			/* to 'VirtualAlloc()' will fail and bring us back */
			/* here, causing us to go into an infinite loop. */

			ಆರಂಭ_ವಿಳಾಸ =
				(ಶೂನ್ಯ *) AlignPage64K((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) ಆರಂಭ_ವಿಳಾಸ);
		}
	}
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;

}

ಶೂನ್ಯ* wsbrk (ದೀರ್ಘ ಗಾತ್ರ_2)
{
	ಶೂನ್ಯ* tmp;
	ಆದರೆ (ಗಾತ್ರ_2 > 0)
	{
		ಆದರೆ (g_ವಿಳಾಸ_ತಳ == 0)
		{
			g_Allocated_ಗಾತ್ರ = max (RESERVED_ಗಾತ್ರ, Align_ಪುಟ (ಗಾತ್ರ_2));
			g_ಮುಂದಿನ_ವಿಳಾಸ = g_ವಿಳಾಸ_ತಳ =
				(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)VirtualAlloc (ಖಾಲಿ, g_Allocated_ಗಾತ್ರ,
											MEM_RESERVE, ಪುಟ_NOACCESS);
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Align_ಪುಟ (g_ಮುಂದಿನ_ವಿಳಾಸ + ಗಾತ್ರ_2) > (g_ವಿಳಾಸ_ತಳ +
g_Allocated_ಗಾತ್ರ))
		{
			ದೀರ್ಘ new_ಗಾತ್ರ = max (ಮುಂದಿನ_ಗಾತ್ರ, Align_ಪುಟ (ಗಾತ್ರ_2));
			ಶೂನ್ಯ* new_ವಿಳಾಸ = (ಶೂನ್ಯ*)(g_ವಿಳಾಸ_ತಳ+g_Allocated_ಗಾತ್ರ);
			ಮಾಡು
			{
				new_ವಿಳಾಸ = ಕಂಡುಹಿಡಿ_Region (new_ವಿಳಾಸ, new_ಗಾತ್ರ);

				ಆದರೆ (!new_ವಿಳಾಸ)
					ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ*)-1;

				g_ವಿಳಾಸ_ತಳ = g_ಮುಂದಿನ_ವಿಳಾಸ =
					(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)VirtualAlloc (new_ವಿಳಾಸ, new_ಗಾತ್ರ,
												MEM_RESERVE, ಪುಟ_NOACCESS);
				/* repeat in case of race condition */
				/* The region that we found has been snagged */
				/* by another thread */
			}
			ಆಗ (g_ವಿಳಾಸ_ತಳ == 0);

			ಪ್ರತಿಜ್ಞಾ (new_ವಿಳಾಸ == (ಶೂನ್ಯ*)g_ವಿಳಾಸ_ತಳ);

			g_Allocated_ಗಾತ್ರ = new_ಗಾತ್ರ;

			ಆದರೆ (!makeGmListElement ((ಶೂನ್ಯ*)g_ವಿಳಾಸ_ತಳ))
				ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ*)-1;
		}
		ಆದರೆ ((ಗಾತ್ರ_2 + g_ಮುಂದಿನ_ವಿಳಾಸ) > Align_ಪುಟ (g_ಮುಂದಿನ_ವಿಳಾಸ))
		{
			ಶೂನ್ಯ* res;
			res = VirtualAlloc ((ಶೂನ್ಯ*)Align_ಪುಟ (g_ಮುಂದಿನ_ವಿಳಾಸ),
								(ಗಾತ್ರ_2 + g_ಮುಂದಿನ_ವಿಳಾಸ -
								 Align_ಪುಟ (g_ಮುಂದಿನ_ವಿಳಾಸ)),
								MEM_COMMIT, ಪುಟ_READWRITE);
			ಆದರೆ (!res)
				ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ*)-1;
		}
		tmp = (ಶೂನ್ಯ*)g_ಮುಂದಿನ_ವಿಳಾಸ;
		g_ಮುಂದಿನ_ವಿಳಾಸ = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)tmp + ಗಾತ್ರ_2;
		ಹಿಂತಿರುಗಿ tmp;
	}
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಗಾತ್ರ_2 < 0)
	{
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ alignedGoal = Align_ಪುಟ (g_ಮುಂದಿನ_ವಿಳಾಸ + ಗಾತ್ರ_2);
		/* Trim by releasing the virtual memory */
		ಆದರೆ (alignedGoal >= g_ವಿಳಾಸ_ತಳ)
		{
			VirtualFree ((ಶೂನ್ಯ*)alignedGoal, g_ಮುಂದಿನ_ವಿಳಾಸ - alignedGoal,
						 MEM_DECOMMIT);
			g_ಮುಂದಿನ_ವಿಳಾಸ = g_ಮುಂದಿನ_ವಿಳಾಸ + ಗಾತ್ರ_2;
			ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ*)g_ಮುಂದಿನ_ವಿಳಾಸ;
		}
		ಇಲ್ಲದಿದ್ದರೆ
		{
			VirtualFree ((ಶೂನ್ಯ*)g_ವಿಳಾಸ_ತಳ, g_ಮುಂದಿನ_ವಿಳಾಸ - g_ವಿಳಾಸ_ತಳ,
						 MEM_DECOMMIT);
			g_ಮುಂದಿನ_ವಿಳಾಸ = g_ವಿಳಾಸ_ತಳ;
			ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ*)-1;
		}
	}
	ಇಲ್ಲದಿದ್ದರೆ
	{
		ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ*)g_ಮುಂದಿನ_ವಿಳಾಸ;
	}
}

#ಆದರೆ_ಅಂತ್ಯ

/*
  Type declarations
*/

ರಚನೆ malloc_chunk
{
  ಆಂತರಿಕ_ಗಾತ್ರ_T prev_ಗಾತ್ರ; /* Size of previous chunk (if free). */
  ಆಂತರಿಕ_ಗಾತ್ರ_T ಗಾತ್ರ_2;      /* Size in bytes, including overhead. */
  ರಚನೆ malloc_chunk* fd;   /* double links -- used only if free. */
  ರಚನೆ malloc_chunk* bk;
} __attribute__((__may_alias__)) ;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ malloc_chunk* mchunkptr;

/*

   malloc_chunk details:

    (The following includes lightly edited explanations by Colin Plumb.)

    Chunks of memory are maintained using a `boundary tag' method as
    described in e.g., Knuth or Standish.  (See the paper by Paul
    Wilson ftp://ftp.cs.utexas.edu/pub/garbage/allocsrv.ps for a
    survey of such techniques.)  Sizes of free chunks are stored both
    in the front of each chunk and at the end.  This makes
    consolidating fragmented chunks into bigger chunks very fast.  The
    size fields also hold bits representing whether chunks are free or
    in use.

    An allocated chunk looks like this:

    chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	    |             Size of previous chunk, if allocated            | |
	    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	    |             Size of chunk, in bytes                         |P|
      mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	    |             User data starts here...                          .
	    .                                                               .
	    .             (malloc_usable_space() bytes)                     .
	    .                                                               |
nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	    |             Size of chunk                                     |
	    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    Where "chunk" is the front of the chunk for the purpose of most of
    the malloc code, but "mem" is the pointer that is returned to the
    user.  "Nextchunk" is the beginning of the next contiguous chunk.

    Chunks always begin on even word boundries, so the mem portion
    (which is returned to the user) is also on an even word boundary, and
    thus double-word aligned.

    Free chunks are stored in circular doubly-linked lists, and look like this:

    chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	    |             Size of previous chunk                            |
	    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `head:' |             Size of chunk, in bytes                         |P|
      mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	    |             Forward pointer to next chunk in list             |
	    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	    |             Back pointer to previous chunk in list            |
	    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	    |             Unused space (may be 0 bytes long)                .
	    .                                                               .
	    .                                                               |

nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `foot:' |             Size of chunk, in bytes                           |
	    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    The P (PREV_INUSE) bit, stored in the unused low-order bit of the
    chunk size (which is always a multiple of two words), is an in-use
    bit for the *previous* chunk.  If that bit is *clear*, then the
    word before the current chunk size contains the previous chunk
    size, and can be used to find the front of the previous chunk.
    (The very first chunk allocated always has this bit set,
    preventing access to non-existent (or non-owned) memory.)

    Note that the `foot' of the current chunk is actually represented
    as the prev_size of the NEXT chunk. (This makes it easier to
    deal with alignments etc).

    The two exceptions to all this are

     1. The special chunk `top', which doesn't bother using the
	trailing size field since there is no
	next contiguous chunk that would have to index off it. (After
	initialization, `top' is forced to always exist.  If it would
	become less than MINSIZE bytes long, it is replenished via
	malloc_extend_top.)

     2. Chunks allocated via mmap, which have the second-lowest-order
	bit (IS_MMAPPED) set in their size fields.  Because they are
	never merged or traversed from any other chunk, they have no
	foot size or inuse information.

    Available chunks are kept in any of several places (all declared below):

    * `av': An array of chunks serving as bin headers for consolidated
       chunks. Each bin is doubly linked.  The bins are approximately
       proportionally (log) spaced.  There are a lot of these bins
       (128). This may look excessive, but works very well in
       practice.  All procedures maintain the invariant that no
       consolidated chunk physically borders another one. Chunks in
       bins are kept in size order, with ties going to the
       approximately least recently used chunk.

       The chunks in each bin are maintained in decreasing sorted order by
       size.  This is irrelevant for the small bins, which all contain
       the same-sized chunks, but facilitates best-fit allocation for
       larger chunks. (These lists are just sequential. Keeping them in
       order almost never requires enough traversal to warrant using
       fancier ordered data structures.)  Chunks of the same size are
       linked with the most recently freed at the front, and allocations
       are taken from the back.  This results in LRU or FIFO allocation
       order, which tends to give each chunk an equal opportunity to be
       consolidated with adjacent freed chunks, resulting in larger free
       chunks and less fragmentation.

    * `top': The top-most available chunk (i.e., the one bordering the
       end of available memory) is treated specially. It is never
       included in any bin, is used only if no other chunk is
       available, and is released back to the system if it is very
       large (see M_TRIM_THRESHOLD).

    * `last_remainder': A bin holding only the remainder of the
       most recently split (non-top) chunk. This bin is checked
       before other non-fitting chunks, so as to provide better
       locality for runs of sequentially allocated chunks.

    *  Implicitly, through the host system's memory mapping tables.
       If supported, requests greater than a threshold are usually
       serviced via calls to mmap, and then later released via munmap.

*/

/*  sizes, alignments */

#ವ್ಯಾಖ್ಯಾನಿಸು ಗಾತ್ರ_SZ                (ಗಾತ್ರ(ಆಂತರಿಕ_ಗಾತ್ರ_T))
#ವ್ಯಾಖ್ಯಾನಿಸು MALLOC_ALIGNMENT       (ಗಾತ್ರ_SZ + ಗಾತ್ರ_SZ)
#ವ್ಯಾಖ್ಯಾನಿಸು MALLOC_ALIGN_ಮುಖವಾಡ      (MALLOC_ALIGNMENT - 1)
#ವ್ಯಾಖ್ಯಾನಿಸು MINSIZE                (ಗಾತ್ರ(ರಚನೆ malloc_chunk))

/* conversion from malloc headers to user pointers, and back */

#ವ್ಯಾಖ್ಯಾನಿಸು chunk2mem(p)   ((Void_t*)((ಅಕ್ಷರ*)(p) + 2*ಗಾತ್ರ_SZ))
#ವ್ಯಾಖ್ಯಾನಿಸು mem2chunk(mem) ((mchunkptr)((ಅಕ್ಷರ*)(mem) - 2*ಗಾತ್ರ_SZ))

/* pad request bytes into a usable size */

#ವ್ಯಾಖ್ಯಾನಿಸು request2size(req) \
 ((((req) + (ಗಾತ್ರ_SZ + MALLOC_ALIGN_ಮುಖವಾಡ)) < \
  (MINSIZE + MALLOC_ALIGN_ಮುಖವಾಡ)) ? MINSIZE : \
   (((req) + (ಗಾತ್ರ_SZ + MALLOC_ALIGN_ಮುಖವಾಡ)) & ~(MALLOC_ALIGN_ಮುಖವಾಡ)))

/* Check if m has acceptable alignment */

#ವ್ಯಾಖ್ಯಾನಿಸು aligned_OK(m)    (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)((m)) & (MALLOC_ALIGN_ಮುಖವಾಡ)) == 0)

/*
  Physical chunk operations
*/

/* size field is or'ed with PREV_INUSE when previous adjacent chunk in use */

#ವ್ಯಾಖ್ಯಾನಿಸು PREV_INUSE 0x1

/* size field is or'ed with IS_MMAPPED if the chunk was obtained with mmap() */

#ವ್ಯಾಖ್ಯಾನಿಸು IS_MMAPPED 0x2

/* Bits to mask off when extracting size */

#ವ್ಯಾಖ್ಯಾನಿಸು ಗಾತ್ರ_ಬಿಟ್ಟುಗಳು (PREV_INUSE|IS_MMAPPED)

/* Ptr to next physical malloc_chunk. */

#ವ್ಯಾಖ್ಯಾನಿಸು ಮುಂದಿನ_chunk(p) ((mchunkptr)( ((ಅಕ್ಷರ*)(p)) + ((p)->ಗಾತ್ರ_2 & ~PREV_INUSE) ))

/* Ptr to previous physical malloc_chunk */

#ವ್ಯಾಖ್ಯಾನಿಸು prev_chunk(p)\
   ((mchunkptr)( ((ಅಕ್ಷರ*)(p)) - ((p)->prev_ಗಾತ್ರ) ))

/* Treat space at ptr + offset as a chunk */

#ವ್ಯಾಖ್ಯಾನಿಸು chunk_at_ಸ್ಥಾನಾಂತರ(p, s)  ((mchunkptr)(((ಅಕ್ಷರ*)(p)) + (s)))

/*
  Dealing with use bits
*/

/* extract p's inuse bit */

#ವ್ಯಾಖ್ಯಾನಿಸು inuse(p)\
((((mchunkptr)(((ಅಕ್ಷರ*)(p))+((p)->ಗಾತ್ರ_2 & ~PREV_INUSE)))->ಗಾತ್ರ_2) & PREV_INUSE)

/* extract inuse bit of previous chunk */

#ವ್ಯಾಖ್ಯಾನಿಸು prev_inuse(p)  ((p)->ಗಾತ್ರ_2 & PREV_INUSE)

/* check for mmap()'ed chunk */

#ವ್ಯಾಖ್ಯಾನಿಸು chunk_is_mmapped(p) ((p)->ಗಾತ್ರ_2 & IS_MMAPPED)

/* set/clear chunk as in use without otherwise disturbing */

#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_inuse(p)\
((mchunkptr)(((ಅಕ್ಷರ*)(p)) + ((p)->ಗಾತ್ರ_2 & ~PREV_INUSE)))->ಗಾತ್ರ_2 |= PREV_INUSE

#ವ್ಯಾಖ್ಯಾನಿಸು clear_inuse(p)\
((mchunkptr)(((ಅಕ್ಷರ*)(p)) + ((p)->ಗಾತ್ರ_2 & ~PREV_INUSE)))->ಗಾತ್ರ_2 &= ~(PREV_INUSE)

/* check/set/clear inuse bits in known places */

#ವ್ಯಾಖ್ಯಾನಿಸು inuse_ಬಿಟ್_at_ಸ್ಥಾನಾಂತರ(p, s)\
 (((mchunkptr)(((ಅಕ್ಷರ*)(p)) + (s)))->ಗಾತ್ರ_2 & PREV_INUSE)

#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_inuse_ಬಿಟ್_at_ಸ್ಥಾನಾಂತರ(p, s)\
 (((mchunkptr)(((ಅಕ್ಷರ*)(p)) + (s)))->ಗಾತ್ರ_2 |= PREV_INUSE)

#ವ್ಯಾಖ್ಯಾನಿಸು clear_inuse_ಬಿಟ್_at_ಸ್ಥಾನಾಂತರ(p, s)\
 (((mchunkptr)(((ಅಕ್ಷರ*)(p)) + (s)))->ಗಾತ್ರ_2 &= ~(PREV_INUSE))

/*
  Dealing with size fields
*/

/* Get size, ignoring use bits */

#ವ್ಯಾಖ್ಯಾನಿಸು chunksize(p)          ((p)->ಗಾತ್ರ_2 & ~(ಗಾತ್ರ_ಬಿಟ್ಟುಗಳು))

/* Set size at head, without disturbing its use bit */

#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_ತಲೆ_ಗಾತ್ರ(p, s)   ((p)->ಗಾತ್ರ_2 = (((p)->ಗಾತ್ರ_2 & PREV_INUSE) | (s)))

/* Set size/use ignoring previous bits in header */

#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_ತಲೆ(p, s)        ((p)->ಗಾತ್ರ_2 = (s))

/* Set size at footer (only when chunk is not in use) */

#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_foot(p, s)   (((mchunkptr)((ಅಕ್ಷರ*)(p) + (s)))->prev_ಗಾತ್ರ = (s))

/*
   Bins

    The bins, `av_' are an array of pairs of pointers serving as the
    heads of (initially empty) doubly-linked lists of chunks, laid out
    in a way so that each pair can be treated as if it were in a
    malloc_chunk. (This way, the fd/bk offsets for linking bin heads
    and chunks are the same).

    Bins for sizes < 512 bytes contain chunks of all the same size, spaced
    8 bytes apart. Larger bins are approximately logarithmically
    spaced. (See the table below.) The `av_' array is never mentioned
    directly in the code, but instead via bin access macros.

    Bin layout:

    64 bins of size       8
    32 bins of size      64
    16 bins of size     512
     8 bins of size    4096
     4 bins of size   32768
     2 bins of size  262144
     1 bin  of size what's left

    There is actually a little bit of slop in the numbers in bin_index
    for the sake of speed. This makes no difference elsewhere.

    The special chunks `top' and `last_remainder' get their own bins,
    (this is implemented via yet more trickery with the av_ array),
    although `top' is never properly linked to its bin since it is
    always handled specially.

*/

#ವ್ಯಾಖ್ಯಾನಿಸು NAV             128   /* number of bins */

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ malloc_chunk* mbinptr;

/* access macros */

#ವ್ಯಾಖ್ಯಾನಿಸು bin_at(i)      ((mbinptr)((ಅಕ್ಷರ*)&(av_[2*(i) + 2]) - 2*ಗಾತ್ರ_SZ))
#ವ್ಯಾಖ್ಯಾನಿಸು ಮುಂದಿನ_bin(b)    ((mbinptr)((ಅಕ್ಷರ*)(b) + 2 * ಗಾತ್ರ(mbinptr)))
#ವ್ಯಾಖ್ಯಾನಿಸು prev_bin(b)    ((mbinptr)((ಅಕ್ಷರ*)(b) - 2 * ಗಾತ್ರ(mbinptr)))

/*
   The first 2 bins are never indexed. The corresponding av_ cells are instead
   used for bookkeeping. This is not to save space, but to simplify
   indexing, maintain locality, and avoid some initialization tests.
*/

#ವ್ಯಾಖ್ಯಾನಿಸು ತುದಿ            (av_[2])          /* The topmost chunk */
#ವ್ಯಾಖ್ಯಾನಿಸು ಕೊನೆಯ_remainder (bin_at(1))       /* remainder from last split */

/*
   Because top initially points to its own bin with initial
   zero size, thus forcing extension on the first malloc request,
   we avoid having any special code in malloc to check whether
   it even exists yet. But we still need to in malloc_extend_top.
*/

#ವ್ಯಾಖ್ಯಾನಿಸು initial_ತುದಿ    ((mchunkptr)(bin_at(0)))

/* Helper macro to initialize bins */

#ವ್ಯಾಖ್ಯಾನಿಸು IAV(i)  bin_at(i), bin_at(i)

ಸ್ಥಿರ mbinptr av_[NAV * 2 + 2] = {
 ಖಾಲಿ, ಖಾಲಿ,
 IAV(0),   IAV(1),   IAV(2),   IAV(3),   IAV(4),   IAV(5),   IAV(6),   IAV(7),
 IAV(8),   IAV(9),   IAV(10),  IAV(11),  IAV(12),  IAV(13),  IAV(14),  IAV(15),
 IAV(16),  IAV(17),  IAV(18),  IAV(19),  IAV(20),  IAV(21),  IAV(22),  IAV(23),
 IAV(24),  IAV(25),  IAV(26),  IAV(27),  IAV(28),  IAV(29),  IAV(30),  IAV(31),
 IAV(32),  IAV(33),  IAV(34),  IAV(35),  IAV(36),  IAV(37),  IAV(38),  IAV(39),
 IAV(40),  IAV(41),  IAV(42),  IAV(43),  IAV(44),  IAV(45),  IAV(46),  IAV(47),
 IAV(48),  IAV(49),  IAV(50),  IAV(51),  IAV(52),  IAV(53),  IAV(54),  IAV(55),
 IAV(56),  IAV(57),  IAV(58),  IAV(59),  IAV(60),  IAV(61),  IAV(62),  IAV(63),
 IAV(64),  IAV(65),  IAV(66),  IAV(67),  IAV(68),  IAV(69),  IAV(70),  IAV(71),
 IAV(72),  IAV(73),  IAV(74),  IAV(75),  IAV(76),  IAV(77),  IAV(78),  IAV(79),
 IAV(80),  IAV(81),  IAV(82),  IAV(83),  IAV(84),  IAV(85),  IAV(86),  IAV(87),
 IAV(88),  IAV(89),  IAV(90),  IAV(91),  IAV(92),  IAV(93),  IAV(94),  IAV(95),
 IAV(96),  IAV(97),  IAV(98),  IAV(99),  IAV(100), IAV(101), IAV(102), IAV(103),
 IAV(104), IAV(105), IAV(106), IAV(107), IAV(108), IAV(109), IAV(110), IAV(111),
 IAV(112), IAV(113), IAV(114), IAV(115), IAV(116), IAV(117), IAV(118), IAV(119),
 IAV(120), IAV(121), IAV(122), IAV(123), IAV(124), IAV(125), IAV(126), IAV(127)
};

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_MALLOC_DEFAULT_ಗಮ್ಯ_INIT
ಸ್ಥಿರ ಶೂನ್ಯ malloc_init(ಶೂನ್ಯ);
#ಆದರೆ_ಅಂತ್ಯ

ulong mem_malloc_ಆರಂಭ = 0;
ulong mem_malloc_ಕೊನೆ = 0;
ulong mem_malloc_brk = 0;

ಸ್ಥಿರ bool malloc_testing;	/* enable test mode */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ malloc_max_allocs;	/* return NULL after this many calls to malloc() */

ಶೂನ್ಯ *sbrk(ptrdiff_t increment)
{
	ulong old = mem_malloc_brk;
	ulong new = old + increment;

	ಆದರೆ ((new < mem_malloc_ಆರಂಭ) || (new > mem_malloc_ಕೊನೆ))
		ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ *)MORECORE_FAILURE;

	/*
	 * if we are giving memory back make sure we clear it out since
	 * we set MORECORE_CLEARS to 1
	 */
	ಆದರೆ (increment < 0)
		memset((ಶೂನ್ಯ *)new, 0, -increment);

	mem_malloc_brk = new;

	ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ *)old;
}

ಶೂನ್ಯ mem_malloc_init(ulong ಆರಂಭ, ulong ಗಾತ್ರ_2)
{
	mem_malloc_ಆರಂಭ = (ulong)map_sysmem(ಆರಂಭ, ಗಾತ್ರ_2);
	mem_malloc_ಕೊನೆ = mem_malloc_ಆರಂಭ + ಗಾತ್ರ_2;
	mem_malloc_brk = mem_malloc_ಆರಂಭ;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_MALLOC_DEFAULT_ಗಮ್ಯ_INIT
	malloc_init();
#ಆದರೆ_ಅಂತ್ಯ

	debug("using memory %#lx-%#lx for malloc()\n", mem_malloc_ಆರಂಭ,
	      mem_malloc_ಕೊನೆ);
#ಆದರೆ CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
	memset((ಶೂನ್ಯ *)mem_malloc_ಆರಂಭ, 0x0, ಗಾತ್ರ_2);
#ಆದರೆ_ಅಂತ್ಯ
}

/* field-extraction macros */

#ವ್ಯಾಖ್ಯಾನಿಸು ಮೊದಲ(b) ((b)->fd)
#ವ್ಯಾಖ್ಯಾನಿಸು ಕೊನೆಯ(b)  ((b)->bk)

/*
  Indexing into bins
*/

#ವ್ಯಾಖ್ಯಾನಿಸು bin_ಸೂಚ್ಯಂಕ(sz)                                                          \
(((((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(sz)) >> 9) ==    0) ?       (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(sz)) >>  3): \
 ((((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(sz)) >> 9) <=    4) ?  56 + (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(sz)) >>  6): \
 ((((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(sz)) >> 9) <=   20) ?  91 + (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(sz)) >>  9): \
 ((((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(sz)) >> 9) <=   84) ? 110 + (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(sz)) >> 12): \
 ((((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(sz)) >> 9) <=  340) ? 119 + (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(sz)) >> 15): \
 ((((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(sz)) >> 9) <= 1364) ? 124 + (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(sz)) >> 18): \
					  126)
/*
  bins for chunks < 512 are all spaced 8 bytes apart, and hold
  identically sized chunks. This is exploited in malloc.
*/

#ವ್ಯಾಖ್ಯಾನಿಸು MAX_SMALLBIN         63
#ವ್ಯಾಖ್ಯಾನಿಸು MAX_SMALLBIN_ಗಾತ್ರ   512
#ವ್ಯಾಖ್ಯಾನಿಸು SMALLBIN_ಅಗಲ        8

#ವ್ಯಾಖ್ಯಾನಿಸು smallbin_ಸೂಚ್ಯಂಕ(sz)  (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(sz)) >> 3)

/*
   Requests are `small' if both the corresponding and the next bin are small
*/

#ವ್ಯಾಖ್ಯಾನಿಸು is_small_request(nb) (nb < MAX_SMALLBIN_ಗಾತ್ರ - SMALLBIN_ಅಗಲ)

/*
    To help compensate for the large number of bins, a one-level index
    structure is used for bin-by-bin searching.  `binblocks' is a
    one-word bitvector recording whether groups of BINBLOCKWIDTH bins
    have any (possibly) non-empty bins, so they can be skipped over
    all at once during during traversals. The bits are NOT always
    cleared as soon as all bins in a block are empty, but instead only
    when all are noticed to be empty during traversal in malloc.
*/

#ವ್ಯಾಖ್ಯಾನಿಸು BINBLOCKWIDTH     4   /* bins per block */

#ವ್ಯಾಖ್ಯಾನಿಸು binblocks_r     ((ಆಂತರಿಕ_ಗಾತ್ರ_T)av_[1]) /* bitvector of nonempty blocks */
#ವ್ಯಾಖ್ಯಾನಿಸು binblocks_w     (av_[1])

/* bin<->block macros */

#ವ್ಯಾಖ್ಯಾನಿಸು idx2binblock(ix)    ((ಚಿಹ್ನೆರಹಿತ)1 << (ix / BINBLOCKWIDTH))
#ವ್ಯಾಖ್ಯಾನಿಸು mark_binblock(ii)   (binblocks_w = (mbinptr)(binblocks_r | idx2binblock(ii)))
#ವ್ಯಾಖ್ಯಾನಿಸು clear_binblock(ii)  (binblocks_w = (mbinptr)(binblocks_r & ~(idx2binblock(ii))))

/*  Other static bookkeeping data */

/* variables holding tunable values */

ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ trim_ಮಿತಿ   = DEFAULT_TRIM_ಮಿತಿ;
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ತುದಿ_pad          = DEFAULT_ತುದಿ_PAD;
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ  n_mmaps_max      = DEFAULT_MMAP_MAX;
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ mmap_ಮಿತಿ   = DEFAULT_MMAP_ಮಿತಿ;

/* The first value returned from sbrk */
ಸ್ಥಿರ ಅಕ್ಷರ* sbrk_ತಳ = (ಅಕ್ಷರ*)(-1);

/* The maximum memory obtained from system via sbrk */
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ max_sbrked_mem = 0;

/* The maximum via either sbrk or mmap */
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ max_ಒಟ್ಟು_mem = 0;

/* internal working copy of mallinfo */
ಸ್ಥಿರ ರಚನೆ mallinfo current_mallinfo = {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* The total memory obtained from system via sbrk */
#ವ್ಯಾಖ್ಯಾನಿಸು sbrked_mem  (current_mallinfo.arena)

/* Tracking mmaps */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DEBUG
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ n_mmaps = 0;
#ಆದರೆ_ಅಂತ್ಯ	/* DEBUG */
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ mmapped_mem = 0;
#ಆದರೆ HAVE_MMAP
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ max_n_mmaps = 0;
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ max_mmapped_mem = 0;
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_MALLOC_DEFAULT_ಗಮ್ಯ_INIT
ಸ್ಥಿರ ಶೂನ್ಯ malloc_init(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ i, j;

	debug("bins (av_ array) are at %p\n", (ಶೂನ್ಯ *)av_);

	av_[0] = ಖಾಲಿ; av_[1] = ಖಾಲಿ;
	ಗಾಗಿ (i = 2, j = 2; i < NAV * 2 + 2; i += 2, j++) {
		av_[i] = bin_at(j - 2);
		av_[i + 1] = bin_at(j - 2);

		/* Just print the first few bins so that
		 * we can see there are alright.
		 */
		ಆದರೆ (i < 10)
			debug("av_[%d]=%lx av_[%d]=%lx\n",
			      i, (ulong)av_[i],
			      i + 1, (ulong)av_[i + 1]);
	}

	/* Init the static bookkeeping as well */
	sbrk_ತಳ = (ಅಕ್ಷರ *)(-1);
	max_sbrked_mem = 0;
	max_ಒಟ್ಟು_mem = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DEBUG
	memset((ಶೂನ್ಯ *)&current_mallinfo, 0, ಗಾತ್ರ(ರಚನೆ mallinfo));
#ಆದರೆ_ಅಂತ್ಯ
}
#ಆದರೆ_ಅಂತ್ಯ

/*
  Debugging support
*/

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DEBUG

/*
  These routines make a number of assertions about the states
  of data structures that should be true at all times. If any
  are not true, it's very likely that a user program has somehow
  trashed memory. (It's also possible that there is a coding error
  in malloc. In which case, please report it!)
*/

#ಆದರೆ __STD_C
ಸ್ಥಿರ ಶೂನ್ಯ do_check_chunk(mchunkptr p)
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಶೂನ್ಯ do_check_chunk(p) mchunkptr p;
#ಆದರೆ_ಅಂತ್ಯ
{
  ಆಂತರಿಕ_ಗಾತ್ರ_T sz = p->ಗಾತ್ರ_2 & ~PREV_INUSE;

  /* No checkable chunk is mmapped */
  ಪ್ರತಿಜ್ಞಾ(!chunk_is_mmapped(p));

  /* Check for legal address ... */
  ಪ್ರತಿಜ್ಞಾ((ಅಕ್ಷರ*)p >= sbrk_ತಳ);
  ಆದರೆ (p != ತುದಿ)
    ಪ್ರತಿಜ್ಞಾ((ಅಕ್ಷರ*)p + sz <= (ಅಕ್ಷರ*)ತುದಿ);
  ಇಲ್ಲದಿದ್ದರೆ
    ಪ್ರತಿಜ್ಞಾ((ಅಕ್ಷರ*)p + sz <= sbrk_ತಳ + sbrked_mem);

}

#ಆದರೆ __STD_C
ಸ್ಥಿರ ಶೂನ್ಯ do_check_free_chunk(mchunkptr p)
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಶೂನ್ಯ do_check_free_chunk(p) mchunkptr p;
#ಆದರೆ_ಅಂತ್ಯ
{
  ಆಂತರಿಕ_ಗಾತ್ರ_T sz = p->ಗಾತ್ರ_2 & ~PREV_INUSE;
  mchunkptr ಮುಂದಿನ = chunk_at_ಸ್ಥಾನಾಂತರ(p, sz);

  do_check_chunk(p);

  /* Check whether it claims to be free ... */
  ಪ್ರತಿಜ್ಞಾ(!inuse(p));

  /* Unless a special marker, must have OK fields */
  ಆದರೆ ((ದೀರ್ಘ)sz >= (ದೀರ್ಘ)MINSIZE)
  {
    ಪ್ರತಿಜ್ಞಾ((sz & MALLOC_ALIGN_ಮುಖವಾಡ) == 0);
    ಪ್ರತಿಜ್ಞಾ(aligned_OK(chunk2mem(p)));
    /* ... matching footer field */
    ಪ್ರತಿಜ್ಞಾ(ಮುಂದಿನ->prev_ಗಾತ್ರ == sz);
    /* ... and is fully consolidated */
    ಪ್ರತಿಜ್ಞಾ(prev_inuse(p));
    ಪ್ರತಿಜ್ಞಾ (ಮುಂದಿನ == ತುದಿ || inuse(ಮುಂದಿನ));

    /* ... and has minimally sane links */
    ಪ್ರತಿಜ್ಞಾ(p->fd->bk == p);
    ಪ್ರತಿಜ್ಞಾ(p->bk->fd == p);
  }
  ಇಲ್ಲದಿದ್ದರೆ /* markers are always of size SIZE_SZ */
    ಪ್ರತಿಜ್ಞಾ(sz == ಗಾತ್ರ_SZ);
}

#ಆದರೆ __STD_C
ಸ್ಥಿರ ಶೂನ್ಯ do_check_inuse_chunk(mchunkptr p)
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಶೂನ್ಯ do_check_inuse_chunk(p) mchunkptr p;
#ಆದರೆ_ಅಂತ್ಯ
{
  mchunkptr ಮುಂದಿನ = ಮುಂದಿನ_chunk(p);
  do_check_chunk(p);

  /* Check whether it claims to be in use ... */
  ಪ್ರತಿಜ್ಞಾ(inuse(p));

  /* ... and is surrounded by OK chunks.
    Since more things can be checked with free chunks than inuse ones,
    if an inuse chunk borders them and debug is on, it's worth doing them.
  */
  ಆದರೆ (!prev_inuse(p))
  {
    mchunkptr prv = prev_chunk(p);
    ಪ್ರತಿಜ್ಞಾ(ಮುಂದಿನ_chunk(prv) == p);
    do_check_free_chunk(prv);
  }
  ಆದರೆ (ಮುಂದಿನ == ತುದಿ)
  {
    ಪ್ರತಿಜ್ಞಾ(prev_inuse(ಮುಂದಿನ));
    ಪ್ರತಿಜ್ಞಾ(chunksize(ಮುಂದಿನ) >= MINSIZE);
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!inuse(ಮುಂದಿನ))
    do_check_free_chunk(ಮುಂದಿನ);

}

#ಆದರೆ __STD_C
ಸ್ಥಿರ ಶೂನ್ಯ do_check_malloced_chunk(mchunkptr p, ಆಂತರಿಕ_ಗಾತ್ರ_T s)
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಶೂನ್ಯ do_check_malloced_chunk(p, s) mchunkptr p; ಆಂತರಿಕ_ಗಾತ್ರ_T s;
#ಆದರೆ_ಅಂತ್ಯ
{
  ಆಂತರಿಕ_ಗಾತ್ರ_T sz = p->ಗಾತ್ರ_2 & ~PREV_INUSE;
  ದೀರ್ಘ ಸ್ಥಳ = sz - s;

  do_check_inuse_chunk(p);

  /* Legal size ... */
  ಪ್ರತಿಜ್ಞಾ((ದೀರ್ಘ)sz >= (ದೀರ್ಘ)MINSIZE);
  ಪ್ರತಿಜ್ಞಾ((sz & MALLOC_ALIGN_ಮುಖವಾಡ) == 0);
  ಪ್ರತಿಜ್ಞಾ(ಸ್ಥಳ >= 0);
  ಪ್ರತಿಜ್ಞಾ(ಸ್ಥಳ < (ದೀರ್ಘ)MINSIZE);

  /* ... and alignment */
  ಪ್ರತಿಜ್ಞಾ(aligned_OK(chunk2mem(p)));

  /* ... and was allocated at front of an available chunk */
  ಪ್ರತಿಜ್ಞಾ(prev_inuse(p));

}

#ವ್ಯಾಖ್ಯಾನಿಸು check_free_chunk(P)  do_check_free_chunk(P)
#ವ್ಯಾಖ್ಯಾನಿಸು check_inuse_chunk(P) do_check_inuse_chunk(P)
#ವ್ಯಾಖ್ಯಾನಿಸು check_chunk(P) do_check_chunk(P)
#ವ್ಯಾಖ್ಯಾನಿಸು check_malloced_chunk(P,N) do_check_malloced_chunk(P,N)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು check_free_chunk(P)
#ವ್ಯಾಖ್ಯಾನಿಸು check_inuse_chunk(P)
#ವ್ಯಾಖ್ಯಾನಿಸು check_chunk(P)
#ವ್ಯಾಖ್ಯಾನಿಸು check_malloced_chunk(P,N)
#ಆದರೆ_ಅಂತ್ಯ

/*
  Macro-based internal utilities
*/

/*
  Linking chunks in bin lists.
  Call these only with variables, not arbitrary expressions, as arguments.
*/

/*
  Place chunk p of size s in its bin, in size order,
  putting it ahead of others of same size.
*/

#ವ್ಯಾಖ್ಯಾನಿಸು frontlink(P, S, IDX, BK, FD)                                          \
{                                                                             \
  ಆದರೆ (S < MAX_SMALLBIN_ಗಾತ್ರ)                                                  \
  {                                                                           \
    IDX = smallbin_ಸೂಚ್ಯಂಕ(S);                                                  \
    mark_binblock(IDX);                                                       \
    BK = bin_at(IDX);                                                         \
    FD = BK->fd;                                                              \
    P->bk = BK;                                                               \
    P->fd = FD;                                                               \
    FD->bk = BK->fd = P;                                                      \
  }                                                                           \
  ಇಲ್ಲದಿದ್ದರೆ                                                                        \
  {                                                                           \
    IDX = bin_ಸೂಚ್ಯಂಕ(S);                                                       \
    BK = bin_at(IDX);                                                         \
    FD = BK->fd;                                                              \
    ಆದರೆ (FD == BK) mark_binblock(IDX);                                         \
    ಇಲ್ಲದಿದ್ದರೆ                                                                      \
    {                                                                         \
      ಆಗ (FD != BK && S < chunksize(FD)) FD = FD->fd;                      \
      BK = FD->bk;                                                            \
    }                                                                         \
    P->bk = BK;                                                               \
    P->fd = FD;                                                               \
    FD->bk = BK->fd = P;                                                      \
  }                                                                           \
}

/* take a chunk off a list */

#ವ್ಯಾಖ್ಯಾನಿಸು unlink(P, BK, FD)                                                     \
{                                                                             \
  BK = P->bk;                                                                 \
  FD = P->fd;                                                                 \
  FD->bk = BK;                                                                \
  BK->fd = FD;                                                                \
}                                                                             \

/* Place p as the last remainder */

#ವ್ಯಾಖ್ಯಾನಿಸು link_ಕೊನೆಯ_remainder(P)                                                \
{                                                                             \
  ಕೊನೆಯ_remainder->fd = ಕೊನೆಯ_remainder->bk =  P;                               \
  P->fd = P->bk = ಕೊನೆಯ_remainder;                                             \
}

/* Clear the last_remainder bin */

#ವ್ಯಾಖ್ಯಾನಿಸು clear_ಕೊನೆಯ_remainder \
  (ಕೊನೆಯ_remainder->fd = ಕೊನೆಯ_remainder->bk = ಕೊನೆಯ_remainder)

/* Routines dealing with mmap(). */

#ಆದರೆ HAVE_MMAP

#ಆದರೆ __STD_C
ಸ್ಥಿರ mchunkptr mmap_chunk(size_t ಗಾತ್ರ_2)
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ mchunkptr mmap_chunk(ಗಾತ್ರ_2) size_t ಗಾತ್ರ_2;
#ಆದರೆ_ಅಂತ್ಯ
{
  size_t ಪುಟ_ಮುಖವಾಡ = malloc_getpagesize - 1;
  mchunkptr p;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ MAP_ANONYMOUS
  ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fd = -1;
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ(n_mmaps >= n_mmaps_max) ಹಿಂತಿರುಗಿ 0; /* too many regions */

  /* For mmapped chunks, the overhead is one SIZE_SZ unit larger, because
   * there is no following chunk whose prev_size field could be used.
   */
  ಗಾತ್ರ_2 = (ಗಾತ್ರ_2 + ಗಾತ್ರ_SZ + ಪುಟ_ಮುಖವಾಡ) & ~ಪುಟ_ಮುಖವಾಡ;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ MAP_ANONYMOUS
  p = (mchunkptr)mmap(0, ಗಾತ್ರ_2, PROT_READ|PROT_WRITE,
		      MAP_ಖಾಸಗಿ|MAP_ANONYMOUS, -1, 0);
#ಇಲ್ಲದಿದ್ದರೆ /* !MAP_ANONYMOUS */
  ಆದರೆ (fd < 0)
  {
    fd = open("/dev/zero", O_RDWR);
    ಆದರೆ(fd < 0) ಹಿಂತಿರುಗಿ 0;
  }
  p = (mchunkptr)mmap(0, ಗಾತ್ರ_2, PROT_READ|PROT_WRITE, MAP_ಖಾಸಗಿ, fd, 0);
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ(p == (mchunkptr)-1) ಹಿಂತಿರುಗಿ 0;

  n_mmaps++;
  ಆದರೆ (n_mmaps > max_n_mmaps) max_n_mmaps = n_mmaps;

  /* We demand that eight bytes into a page must be 8-byte aligned. */
  ಪ್ರತಿಜ್ಞಾ(aligned_OK(chunk2mem(p)));

  /* The offset to the start of the mmapped region is stored
   * in the prev_size field of the chunk; normally it is zero,
   * but that can be changed in memalign().
   */
  p->prev_ಗಾತ್ರ = 0;
  ಗಣ_ತಲೆ(p, ಗಾತ್ರ_2|IS_MMAPPED);

  mmapped_mem += ಗಾತ್ರ_2;
  ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)mmapped_mem > (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)max_mmapped_mem)
    max_mmapped_mem = mmapped_mem;
  ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(mmapped_mem + sbrked_mem) > (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)max_ಒಟ್ಟು_mem)
    max_ಒಟ್ಟು_mem = mmapped_mem + sbrked_mem;
  ಹಿಂತಿರುಗಿ p;
}

#ಆದರೆ __STD_C
ಸ್ಥಿರ ಶೂನ್ಯ munmap_chunk(mchunkptr p)
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಶೂನ್ಯ munmap_chunk(p) mchunkptr p;
#ಆದರೆ_ಅಂತ್ಯ
{
  ಆಂತರಿಕ_ಗಾತ್ರ_T ಗಾತ್ರ_2 = chunksize(p);
  ಪೂರ್ಣಾಂಕ ret;

  ಪ್ರತಿಜ್ಞಾ (chunk_is_mmapped(p));
  ಪ್ರತಿಜ್ಞಾ(! ((ಅಕ್ಷರ*)p >= sbrk_ತಳ && (ಅಕ್ಷರ*)p < sbrk_ತಳ + sbrked_mem));
  ಪ್ರತಿಜ್ಞಾ((n_mmaps > 0));
  ಪ್ರತಿಜ್ಞಾ(((p->prev_ಗಾತ್ರ + ಗಾತ್ರ_2) & (malloc_getpagesize-1)) == 0);

  n_mmaps--;
  mmapped_mem -= (ಗಾತ್ರ_2 + p->prev_ಗಾತ್ರ);

  ret = munmap((ಅಕ್ಷರ *)p - p->prev_ಗಾತ್ರ, ಗಾತ್ರ_2 + p->prev_ಗಾತ್ರ);

  /* munmap returns non-zero on failure */
  ಪ್ರತಿಜ್ಞಾ(ret == 0);
}

#ಆದರೆ HAVE_MREMAP

#ಆದರೆ __STD_C
ಸ್ಥಿರ mchunkptr mremap_chunk(mchunkptr p, size_t new_ಗಾತ್ರ)
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ mchunkptr mremap_chunk(p, new_ಗಾತ್ರ) mchunkptr p; size_t new_ಗಾತ್ರ;
#ಆದರೆ_ಅಂತ್ಯ
{
  size_t ಪುಟ_ಮುಖವಾಡ = malloc_getpagesize - 1;
  ಆಂತರಿಕ_ಗಾತ್ರ_T ಸ್ಥಾನಾಂತರ = p->prev_ಗಾತ್ರ;
  ಆಂತರಿಕ_ಗಾತ್ರ_T ಗಾತ್ರ_2 = chunksize(p);
  ಅಕ್ಷರ *cp;

  ಪ್ರತಿಜ್ಞಾ (chunk_is_mmapped(p));
  ಪ್ರತಿಜ್ಞಾ(! ((ಅಕ್ಷರ*)p >= sbrk_ತಳ && (ಅಕ್ಷರ*)p < sbrk_ತಳ + sbrked_mem));
  ಪ್ರತಿಜ್ಞಾ((n_mmaps > 0));
  ಪ್ರತಿಜ್ಞಾ(((ಗಾತ್ರ_2 + ಸ್ಥಾನಾಂತರ) & (malloc_getpagesize-1)) == 0);

  /* Note the extra SIZE_SZ overhead as in mmap_chunk(). */
  new_ಗಾತ್ರ = (new_ಗಾತ್ರ + ಸ್ಥಾನಾಂತರ + ಗಾತ್ರ_SZ + ಪುಟ_ಮುಖವಾಡ) & ~ಪುಟ_ಮುಖವಾಡ;

  cp = (ಅಕ್ಷರ *)mremap((ಅಕ್ಷರ *)p - ಸ್ಥಾನಾಂತರ, ಗಾತ್ರ_2 + ಸ್ಥಾನಾಂತರ, new_ಗಾತ್ರ, 1);

  ಆದರೆ (cp == (ಅಕ್ಷರ *)-1) ಹಿಂತಿರುಗಿ 0;

  p = (mchunkptr)(cp + ಸ್ಥಾನಾಂತರ);

  ಪ್ರತಿಜ್ಞಾ(aligned_OK(chunk2mem(p)));

  ಪ್ರತಿಜ್ಞಾ((p->prev_ಗಾತ್ರ == ಸ್ಥಾನಾಂತರ));
  ಗಣ_ತಲೆ(p, (new_ಗಾತ್ರ - ಸ್ಥಾನಾಂತರ)|IS_MMAPPED);

  mmapped_mem -= ಗಾತ್ರ_2 + ಸ್ಥಾನಾಂತರ;
  mmapped_mem += new_ಗಾತ್ರ;
  ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)mmapped_mem > (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)max_mmapped_mem)
    max_mmapped_mem = mmapped_mem;
  ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(mmapped_mem + sbrked_mem) > (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)max_ಒಟ್ಟು_mem)
    max_ಒಟ್ಟು_mem = mmapped_mem + sbrked_mem;
  ಹಿಂತಿರುಗಿ p;
}

#ಆದರೆ_ಅಂತ್ಯ /* HAVE_MREMAP */

#ಆದರೆ_ಅಂತ್ಯ /* HAVE_MMAP */

/*
  Extend the top-most chunk by obtaining memory from system.
  Main interface to sbrk (but see also malloc_trim).
*/

#ಆದರೆ __STD_C
ಸ್ಥಿರ ಶೂನ್ಯ malloc_extend_ತುದಿ(ಆಂತರಿಕ_ಗಾತ್ರ_T nb)
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಶೂನ್ಯ malloc_extend_ತುದಿ(nb) ಆಂತರಿಕ_ಗಾತ್ರ_T nb;
#ಆದರೆ_ಅಂತ್ಯ
{
  ಅಕ್ಷರ*     brk;                  /* return value from sbrk */
  ಆಂತರಿಕ_ಗಾತ್ರ_T front_misalign; /* unusable bytes at front of sbrked space */
  ಆಂತರಿಕ_ಗಾತ್ರ_T correction;     /* bytes for 2nd sbrk call */
  ಅಕ್ಷರ*     new_brk;              /* return of 2nd sbrk call */
  ಆಂತರಿಕ_ಗಾತ್ರ_T ತುದಿ_ಗಾತ್ರ;       /* new size of top chunk */

  mchunkptr old_ತುದಿ     = ತುದಿ;  /* Record state of old top */
  ಆಂತರಿಕ_ಗಾತ್ರ_T old_ತುದಿ_ಗಾತ್ರ = chunksize(old_ತುದಿ);
  ಅಕ್ಷರ*     old_ಕೊನೆ      = (ಅಕ್ಷರ*)(chunk_at_ಸ್ಥಾನಾಂತರ(old_ತುದಿ, old_ತುದಿ_ಗಾತ್ರ));

  /* Pad request with top_pad plus minimal overhead */

  ಆಂತರಿಕ_ಗಾತ್ರ_T    sbrk_ಗಾತ್ರ     = nb + ತುದಿ_pad + MINSIZE;
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ pagesz    = malloc_getpagesize;

  /* If not the first time through, round to preserve page boundary */
  /* Otherwise, we need to correct to a page size below anyway. */
  /* (We also correct below if an intervening foreign sbrk call.) */

  ಆದರೆ (sbrk_ತಳ != (ಅಕ್ಷರ*)(-1))
    sbrk_ಗಾತ್ರ = (sbrk_ಗಾತ್ರ + (pagesz - 1)) & ~(pagesz - 1);

  brk = (ಅಕ್ಷರ*)(MORECORE (sbrk_ಗಾತ್ರ));

  /* Fail if sbrk failed or if a foreign sbrk call killed our space */
  ಆದರೆ (brk == (ಅಕ್ಷರ*)(MORECORE_FAILURE) ||
      (brk < old_ಕೊನೆ && old_ತುದಿ != initial_ತುದಿ))
    ಹಿಂತಿರುಗಿ;

  sbrked_mem += sbrk_ಗಾತ್ರ;

  ಆದರೆ (brk == old_ಕೊನೆ) /* can just add bytes to current top */
  {
    ತುದಿ_ಗಾತ್ರ = sbrk_ಗಾತ್ರ + old_ತುದಿ_ಗಾತ್ರ;
    ಗಣ_ತಲೆ(ತುದಿ, ತುದಿ_ಗಾತ್ರ | PREV_INUSE);
  }
  ಇಲ್ಲದಿದ್ದರೆ
  {
    ಆದರೆ (sbrk_ತಳ == (ಅಕ್ಷರ*)(-1))  /* First time through. Record base */
      sbrk_ತಳ = brk;
    ಇಲ್ಲದಿದ್ದರೆ  /* Someone else called sbrk().  Count those bytes as sbrked_mem. */
      sbrked_mem += brk - (ಅಕ್ಷರ*)old_ಕೊನೆ;

    /* Guarantee alignment of first new chunk made from this space */
    front_misalign = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)chunk2mem(brk) & MALLOC_ALIGN_ಮುಖವಾಡ;
    ಆದರೆ (front_misalign > 0)
    {
      correction = (MALLOC_ALIGNMENT) - front_misalign;
      brk += correction;
    }
    ಇಲ್ಲದಿದ್ದರೆ
      correction = 0;

    /* Guarantee the next brk will be at a page boundary */

    correction += ((((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(brk + sbrk_ಗಾತ್ರ))+(pagesz-1)) &
		   ~(pagesz - 1)) - ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(brk + sbrk_ಗಾತ್ರ));

    /* Allocate correction */
    new_brk = (ಅಕ್ಷರ*)(MORECORE (correction));
    ಆದರೆ (new_brk == (ಅಕ್ಷರ*)(MORECORE_FAILURE)) ಹಿಂತಿರುಗಿ;

    sbrked_mem += correction;

    ತುದಿ = (mchunkptr)brk;
    ತುದಿ_ಗಾತ್ರ = new_brk - brk + correction;
    ಗಣ_ತಲೆ(ತುದಿ, ತುದಿ_ಗಾತ್ರ | PREV_INUSE);

    ಆದರೆ (old_ತುದಿ != initial_ತುದಿ)
    {

      /* There must have been an intervening foreign sbrk call. */
      /* A double fencepost is necessary to prevent consolidation */

      /* If not enough space to do this, then user did something very wrong */
      ಆದರೆ (old_ತುದಿ_ಗಾತ್ರ < MINSIZE)
      {
	ಗಣ_ತಲೆ(ತುದಿ, PREV_INUSE); /* will force null return from malloc */
	ಹಿಂತಿರುಗಿ;
      }

      /* Also keep size a multiple of MALLOC_ALIGNMENT */
      old_ತುದಿ_ಗಾತ್ರ = (old_ತುದಿ_ಗಾತ್ರ - 3*ಗಾತ್ರ_SZ) & ~MALLOC_ALIGN_ಮುಖವಾಡ;
      ಗಣ_ತಲೆ_ಗಾತ್ರ(old_ತುದಿ, old_ತುದಿ_ಗಾತ್ರ);
      chunk_at_ಸ್ಥಾನಾಂತರ(old_ತುದಿ, old_ತುದಿ_ಗಾತ್ರ          )->ಗಾತ್ರ_2 =
	ಗಾತ್ರ_SZ|PREV_INUSE;
      chunk_at_ಸ್ಥಾನಾಂತರ(old_ತುದಿ, old_ತುದಿ_ಗಾತ್ರ + ಗಾತ್ರ_SZ)->ಗಾತ್ರ_2 =
	ಗಾತ್ರ_SZ|PREV_INUSE;
      /* If possible, release the rest. */
      ಆದರೆ (old_ತುದಿ_ಗಾತ್ರ >= MINSIZE)
	fREe(chunk2mem(old_ತುದಿ));
    }
  }

  ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)sbrked_mem > (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)max_sbrked_mem)
    max_sbrked_mem = sbrked_mem;
  ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(mmapped_mem + sbrked_mem) > (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)max_ಒಟ್ಟು_mem)
    max_ಒಟ್ಟು_mem = mmapped_mem + sbrked_mem;

  /* We always land on a page boundary */
  ಪ್ರತಿಜ್ಞಾ(((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)((ಅಕ್ಷರ*)ತುದಿ + ತುದಿ_ಗಾತ್ರ) & (pagesz - 1)) == 0);
}

/* Main public routines */

/*
  Malloc Algorthim:

    The requested size is first converted into a usable form, `nb'.
    This currently means to add 4 bytes overhead plus possibly more to
    obtain 8-byte alignment and/or to obtain a size of at least
    MINSIZE (currently 16 bytes), the smallest allocatable size.
    (All fits are considered `exact' if they are within MINSIZE bytes.)

    From there, the first successful of the following steps is taken:

      1. The bin corresponding to the request size is scanned, and if
	 a chunk of exactly the right size is found, it is taken.

      2. The most recently remaindered chunk is used if it is big
	 enough.  This is a form of (roving) first fit, used only in
	 the absence of exact fits. Runs of consecutive requests use
	 the remainder of the chunk used for the previous such request
	 whenever possible. This limited use of a first-fit style
	 allocation strategy tends to give contiguous chunks
	 coextensive lifetimes, which improves locality and can reduce
	 fragmentation in the long run.

      3. Other bins are scanned in increasing size order, using a
	 chunk big enough to fulfill the request, and splitting off
	 any remainder.  This search is strictly by best-fit; i.e.,
	 the smallest (with ties going to approximately the least
	 recently used) chunk that fits is selected.

      4. If large enough, the chunk bordering the end of memory
	 (`top') is split off. (This use of `top' is in accord with
	 the best-fit search rule.  In effect, `top' is treated as
	 larger (and thus less well fitting) than any other available
	 chunk since it can be extended to be as large as necessary
	 (up to system limitations).

      5. If the request size meets the mmap threshold and the
	 system supports mmap, and there are few enough currently
	 allocated mmapped regions, and a call to mmap succeeds,
	 the request is allocated via direct memory mapping.

      6. Otherwise, the top of memory is extended by
	 obtaining more space from the system (normally using sbrk,
	 but definable to anything else via the MORECORE macro).
	 Memory is gathered from the system (in system page-sized
	 units) in a way that allows chunks obtained across different
	 sbrk calls to be consolidated, but does not require
	 contiguous memory. Thus, it should be safe to intersperse
	 mallocs with other sbrk calls.

      All allocations are made from the the `lowest' part of any found
      chunk. (The implementation invariant is that prev_inuse is
      always true of any allocated chunk; i.e., that each allocated
      chunk borders either a previously allocated and still in-use chunk,
      or the base of its memory arena.)

*/

STATIC_IF_MCHECK
#ಆದರೆ __STD_C
Void_t* mALLOc_impl(size_t ಬೈಟುಗಳು)
#ಇಲ್ಲದಿದ್ದರೆ
Void_t* mALLOc_impl(ಬೈಟುಗಳು) size_t ಬೈಟುಗಳು;
#ಆದರೆ_ಅಂತ್ಯ
{
  mchunkptr victim;                  /* inspected/selected chunk */
  ಆಂತರಿಕ_ಗಾತ್ರ_T victim_ಗಾತ್ರ;       /* its size */
  ಪೂರ್ಣಾಂಕ       idx;                     /* index for bin traversal */
  mbinptr   bin;                     /* associated bin */
  mchunkptr remainder;               /* remainder from a split */
  ದೀರ್ಘ      remainder_ಗಾತ್ರ;          /* its size */
  ಪೂರ್ಣಾಂಕ       remainder_ಸೂಚ್ಯಂಕ;         /* its bin index */
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಖಂಡ;               /* block traverser bit */
  ಪೂರ್ಣಾಂಕ       startidx;                /* first bin of a traversed block */
  mchunkptr fwd;                     /* misc temp for linking */
  mchunkptr bck;                     /* misc temp for linking */
  mbinptr q;                         /* misc temp */

  ಆಂತರಿಕ_ಗಾತ್ರ_T nb;

#ಆದರೆ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ಆದರೆ (!(gd->flags & GD_FLG_ತುಂಬಿದ_MALLOC_INIT))
		ಹಿಂತಿರುಗಿ malloc_simple(ಬೈಟುಗಳು);
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ (CONFIG_IS_ENABLED(UNIT_TEST) && malloc_testing) {
    ಆದರೆ (--malloc_max_allocs < 0)
      ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  }

  /* check if mem_malloc_init() was run */
  ಆದರೆ ((mem_malloc_ಆರಂಭ == 0) && (mem_malloc_ಕೊನೆ == 0)) {
    /* not initialized yet */
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  }

  ಆದರೆ (ಬೈಟುಗಳು > CONFIG_SYS_MALLOC_LEN || (ದೀರ್ಘ)ಬೈಟುಗಳು < 0)
     ಹಿಂತಿರುಗಿ ಖಾಲಿ;

  nb = request2size(ಬೈಟುಗಳು);  /* padded request size; */

  /* Check for exact match in a bin */

  ಆದರೆ (is_small_request(nb))  /* Faster version for small requests */
  {
    idx = smallbin_ಸೂಚ್ಯಂಕ(nb);

    /* No traversal or size check necessary for small bins.  */

    q = bin_at(idx);
    victim = ಕೊನೆಯ(q);

    /* Also scan the next one, since it would have a remainder < MINSIZE */
    ಆದರೆ (victim == q)
    {
      q = ಮುಂದಿನ_bin(q);
      victim = ಕೊನೆಯ(q);
    }
    ಆದರೆ (victim != q)
    {
      victim_ಗಾತ್ರ = chunksize(victim);
      unlink(victim, bck, fwd);
      ಗಣ_inuse_ಬಿಟ್_at_ಸ್ಥಾನಾಂತರ(victim, victim_ಗಾತ್ರ);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_ಖಂಡ(chunk2mem(victim), ಬೈಟುಗಳು, ಗಾತ್ರ_SZ, false);
      ಹಿಂತಿರುಗಿ chunk2mem(victim);
    }

    idx += 2; /* Set for bin scan below. We've already scanned 2 bins. */

  }
  ಇಲ್ಲದಿದ್ದರೆ
  {
    idx = bin_ಸೂಚ್ಯಂಕ(nb);
    bin = bin_at(idx);

    ಗಾಗಿ (victim = ಕೊನೆಯ(bin); victim != bin; victim = victim->bk)
    {
      victim_ಗಾತ್ರ = chunksize(victim);
      remainder_ಗಾತ್ರ = victim_ಗಾತ್ರ - nb;

      ಆದರೆ (remainder_ಗಾತ್ರ >= (ದೀರ್ಘ)MINSIZE) /* too big */
      {
	--idx; /* adjust to rescan below after checking last remainder */
	ನಿಲ್ಲಿಸು;
      }

      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (remainder_ಗಾತ್ರ >= 0) /* exact fit */
      {
	unlink(victim, bck, fwd);
	ಗಣ_inuse_ಬಿಟ್_at_ಸ್ಥಾನಾಂತರ(victim, victim_ಗಾತ್ರ);
	check_malloced_chunk(victim, nb);
        VALGRIND_MALLOCLIKE_ಖಂಡ(chunk2mem(victim), ಬೈಟುಗಳು, ಗಾತ್ರ_SZ, false);
	ಹಿಂತಿರುಗಿ chunk2mem(victim);
      }
    }

    ++idx;

  }

  /* Try to use the last split-off remainder */

  ಆದರೆ ( (victim = ಕೊನೆಯ_remainder->fd) != ಕೊನೆಯ_remainder)
  {
    victim_ಗಾತ್ರ = chunksize(victim);
    remainder_ಗಾತ್ರ = victim_ಗಾತ್ರ - nb;

    ಆದರೆ (remainder_ಗಾತ್ರ >= (ದೀರ್ಘ)MINSIZE) /* re-split */
    {
      remainder = chunk_at_ಸ್ಥಾನಾಂತರ(victim, nb);
      ಗಣ_ತಲೆ(victim, nb | PREV_INUSE);
      link_ಕೊನೆಯ_remainder(remainder);
      ಗಣ_ತಲೆ(remainder, remainder_ಗಾತ್ರ | PREV_INUSE);
      ಗಣ_foot(remainder, remainder_ಗಾತ್ರ);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_ಖಂಡ(chunk2mem(victim), ಬೈಟುಗಳು, ಗಾತ್ರ_SZ, false);
      ಹಿಂತಿರುಗಿ chunk2mem(victim);
    }

    clear_ಕೊನೆಯ_remainder;

    ಆದರೆ (remainder_ಗಾತ್ರ >= 0)  /* exhaust */
    {
      ಗಣ_inuse_ಬಿಟ್_at_ಸ್ಥಾನಾಂತರ(victim, victim_ಗಾತ್ರ);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_ಖಂಡ(chunk2mem(victim), ಬೈಟುಗಳು, ಗಾತ್ರ_SZ, false);
      ಹಿಂತಿರುಗಿ chunk2mem(victim);
    }

    /* Else place in bin */

    frontlink(victim, victim_ಗಾತ್ರ, remainder_ಸೂಚ್ಯಂಕ, bck, fwd);
  }

  /*
     If there are any possibly nonempty big-enough blocks,
     search for best fitting chunk by scanning bins in blockwidth units.
  */

  ಆದರೆ ( (ಖಂಡ = idx2binblock(idx)) <= binblocks_r)
  {

    /* Get to the first marked block */

    ಆದರೆ ( (ಖಂಡ & binblocks_r) == 0)
    {
      /* force to an even block boundary */
      idx = (idx & ~(BINBLOCKWIDTH - 1)) + BINBLOCKWIDTH;
      ಖಂಡ <<= 1;
      ಆಗ ((ಖಂಡ & binblocks_r) == 0)
      {
	idx += BINBLOCKWIDTH;
	ಖಂಡ <<= 1;
      }
    }

    /* For each possibly nonempty block ... */
    ಗಾಗಿ (;;)
    {
      startidx = idx;          /* (track incomplete blocks) */
      q = bin = bin_at(idx);

      /* For each bin in this block ... */
      ಮಾಡು
      {
	/* Find and use first big enough chunk ... */

	ಗಾಗಿ (victim = ಕೊನೆಯ(bin); victim != bin; victim = victim->bk)
	{
	  victim_ಗಾತ್ರ = chunksize(victim);
	  remainder_ಗಾತ್ರ = victim_ಗಾತ್ರ - nb;

	  ಆದರೆ (remainder_ಗಾತ್ರ >= (ದೀರ್ಘ)MINSIZE) /*ವಿಭಜಿಸು*/
	  {
	    remainder = chunk_at_ಸ್ಥಾನಾಂತರ(victim, nb);
	    ಗಣ_ತಲೆ(victim, nb | PREV_INUSE);
	    unlink(victim, bck, fwd);
	    link_ಕೊನೆಯ_remainder(remainder);
	    ಗಣ_ತಲೆ(remainder, remainder_ಗಾತ್ರ | PREV_INUSE);
	    ಗಣ_foot(remainder, remainder_ಗಾತ್ರ);
	    check_malloced_chunk(victim, nb);
	    VALGRIND_MALLOCLIKE_ಖಂಡ(chunk2mem(victim), ಬೈಟುಗಳು, ಗಾತ್ರ_SZ, false);
	    ಹಿಂತಿರುಗಿ chunk2mem(victim);
	  }

	  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (remainder_ಗಾತ್ರ >= 0)  /* take */
	  {
	    ಗಣ_inuse_ಬಿಟ್_at_ಸ್ಥಾನಾಂತರ(victim, victim_ಗಾತ್ರ);
	    unlink(victim, bck, fwd);
	    check_malloced_chunk(victim, nb);
	    VALGRIND_MALLOCLIKE_ಖಂಡ(chunk2mem(victim), ಬೈಟುಗಳು, ಗಾತ್ರ_SZ, false);
	    ಹಿಂತಿರುಗಿ chunk2mem(victim);
	  }

	}

       bin = ಮುಂದಿನ_bin(bin);

      } ಆಗ ((++idx & (BINBLOCKWIDTH - 1)) != 0);

      /* Clear out the block bit. */

      ಮಾಡು   /* Possibly backtrack to try to clear a partial block */
      {
	ಆದರೆ ((startidx & (BINBLOCKWIDTH - 1)) == 0)
	{
	  av_[1] = (mbinptr)(binblocks_r & ~ಖಂಡ);
	  ನಿಲ್ಲಿಸು;
	}
	--startidx;
       q = prev_bin(q);
      } ಆಗ (ಮೊದಲ(q) == q);

      /* Get to the next possibly nonempty block */

      ಆದರೆ ( (ಖಂಡ <<= 1) <= binblocks_r && (ಖಂಡ != 0) )
      {
	ಆಗ ((ಖಂಡ & binblocks_r) == 0)
	{
	  idx += BINBLOCKWIDTH;
	  ಖಂಡ <<= 1;
	}
      }
      ಇಲ್ಲದಿದ್ದರೆ
	ನಿಲ್ಲಿಸು;
    }
  }

  /* Try to use top chunk */

  /* Require that there be a remainder, ensuring top always exists  */
  ಆದರೆ ( (remainder_ಗಾತ್ರ = chunksize(ತುದಿ) - nb) < (ದೀರ್ಘ)MINSIZE)
  {

#ಆದರೆ HAVE_MMAP
    /* If big and would otherwise need to extend, try to use mmap instead */
    ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)nb >= (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)mmap_ಮಿತಿ &&
	(victim = mmap_chunk(nb)))
      VALGRIND_MALLOCLIKE_ಖಂಡ(chunk2mem(victim), ಬೈಟುಗಳು, ಗಾತ್ರ_SZ, false);
      ಹಿಂತಿರುಗಿ chunk2mem(victim);
#ಆದರೆ_ಅಂತ್ಯ

    /* Try to extend */
    malloc_extend_ತುದಿ(nb);
    ಆದರೆ ( (remainder_ಗಾತ್ರ = chunksize(ತುದಿ) - nb) < (ದೀರ್ಘ)MINSIZE)
      ಹಿಂತಿರುಗಿ ಖಾಲಿ; /* propagate failure */
  }

  victim = ತುದಿ;
  ಗಣ_ತಲೆ(victim, nb | PREV_INUSE);
  ತುದಿ = chunk_at_ಸ್ಥಾನಾಂತರ(victim, nb);
  ಗಣ_ತಲೆ(ತುದಿ, remainder_ಗಾತ್ರ | PREV_INUSE);
  check_malloced_chunk(victim, nb);
  VALGRIND_MALLOCLIKE_ಖಂಡ(chunk2mem(victim), ಬೈಟುಗಳು, ಗಾತ್ರ_SZ, false);
  ಹಿಂತಿರುಗಿ chunk2mem(victim);

}

/*

  free() algorithm :

    cases:

       1. free(0) has no effect.

       2. If the chunk was allocated via mmap, it is release via munmap().

       3. If a returned chunk borders the current high end of memory,
	  it is consolidated into the top, and if the total unused
	  topmost memory exceeds the trim threshold, malloc_trim is
	  called.

       4. Other chunks are consolidated as they arrive, and
	  placed in corresponding bins. (This includes the case of
	  consolidating with the current `last_remainder').

*/

STATIC_IF_MCHECK
#ಆದರೆ __STD_C
ಶೂನ್ಯ fREe_impl(Void_t* mem)
#ಇಲ್ಲದಿದ್ದರೆ
ಶೂನ್ಯ fREe_impl(mem) Void_t* mem;
#ಆದರೆ_ಅಂತ್ಯ
{
  mchunkptr p;         /* chunk corresponding to mem */
  ಆಂತರಿಕ_ಗಾತ್ರ_T hd;  /* its head field */
  ಆಂತರಿಕ_ಗಾತ್ರ_T sz;  /* its size */
  ಪೂರ್ಣಾಂಕ       idx;       /* its bin index */
  mchunkptr ಮುಂದಿನ;      /* next contiguous chunk */
  ಆಂತರಿಕ_ಗಾತ್ರ_T nextsz; /* its size */
  ಆಂತರಿಕ_ಗಾತ್ರ_T prevsz; /* size of previous contiguous chunk */
  mchunkptr bck;       /* misc temp for linking */
  mchunkptr fwd;       /* misc temp for linking */
  ಪೂರ್ಣಾಂಕ       islr;      /* track whether merging with last_remainder */

#ಆದರೆ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	/* free() is a no-op - all the memory will be freed on relocation */
	ಆದರೆ (!(gd->flags & GD_FLG_ತುಂಬಿದ_MALLOC_INIT)) {
		VALGRIND_FREELIKE_ಖಂಡ(mem, ಗಾತ್ರ_SZ);
		ಹಿಂತಿರುಗಿ;
	}
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ (mem == ಖಾಲಿ)                              /* free(0) has no effect */
    ಹಿಂತಿರುಗಿ;

  p = mem2chunk(mem);
  hd = p->ಗಾತ್ರ_2;

#ಆದರೆ HAVE_MMAP
  ಆದರೆ (hd & IS_MMAPPED)                       /* release mmapped memory. */
  {
    munmap_chunk(p);
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ

  check_inuse_chunk(p);

  sz = hd & ~PREV_INUSE;
  ಮುಂದಿನ = chunk_at_ಸ್ಥಾನಾಂತರ(p, sz);
  nextsz = chunksize(ಮುಂದಿನ);
  VALGRIND_FREELIKE_ಖಂಡ(mem, ಗಾತ್ರ_SZ);

  ಆದರೆ (ಮುಂದಿನ == ತುದಿ)                            /* merge with top */
  {
    sz += nextsz;

    ಆದರೆ (!(hd & PREV_INUSE))                    /* consolidate backward */
    {
      prevsz = p->prev_ಗಾತ್ರ;
      p = chunk_at_ಸ್ಥಾನಾಂತರ(p, -((ದೀರ್ಘ) prevsz));
      sz += prevsz;
      unlink(p, bck, fwd);
    }

    ಗಣ_ತಲೆ(p, sz | PREV_INUSE);
    ತುದಿ = p;
    ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(sz) >= (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)trim_ಮಿತಿ)
      malloc_trim(ತುದಿ_pad);
    ಹಿಂತಿರುಗಿ;
  }

  ಗಣ_ತಲೆ(ಮುಂದಿನ, nextsz);                    /* clear inuse bit */

  islr = 0;

  ಆದರೆ (!(hd & PREV_INUSE))                    /* consolidate backward */
  {
    prevsz = p->prev_ಗಾತ್ರ;
    p = chunk_at_ಸ್ಥಾನಾಂತರ(p, -((ದೀರ್ಘ) prevsz));
    sz += prevsz;

    ಆದರೆ (p->fd == ಕೊನೆಯ_remainder)             /* keep as last_remainder */
      islr = 1;
    ಇಲ್ಲದಿದ್ದರೆ
      unlink(p, bck, fwd);
  }

  ಆದರೆ (!(inuse_ಬಿಟ್_at_ಸ್ಥಾನಾಂತರ(ಮುಂದಿನ, nextsz)))   /* consolidate forward */
  {
    sz += nextsz;

    ಆದರೆ (!islr && ಮುಂದಿನ->fd == ಕೊನೆಯ_remainder)  /* re-insert last_remainder */
    {
      islr = 1;
      link_ಕೊನೆಯ_remainder(p);
    }
    ಇಲ್ಲದಿದ್ದರೆ
      unlink(ಮುಂದಿನ, bck, fwd);
  }

  ಗಣ_ತಲೆ(p, sz | PREV_INUSE);
  ಗಣ_foot(p, sz);
  ಆದರೆ (!islr)
    frontlink(p, sz, idx, bck, fwd);
}

/*

  Realloc algorithm:

    Chunks that were obtained via mmap cannot be extended or shrunk
    unless HAVE_MREMAP is defined, in which case mremap is used.
    Otherwise, if their reallocation is for additional space, they are
    copied.  If for less, they are just left alone.

    Otherwise, if the reallocation is for additional space, and the
    chunk can be extended, it is, else a malloc-copy-free sequence is
    taken.  There are several different ways that a chunk could be
    extended. All are tried:

       * Extending forward into following adjacent free chunk.
       * Shifting backwards, joining preceding adjacent space
       * Both shifting backwards and extending forward.
       * Extending into newly sbrked space

    Unless the #define REALLOC_ZERO_BYTES_FREES is set, realloc with a
    size argument of zero (re)allocates a minimum-sized chunk.

    If the reallocation is for less space, and the new request is for
    a `small' (<512 bytes) size, then the newly unused space is lopped
    off and freed.

    The old unix realloc convention of allowing the last-free'd chunk
    to be used as an argument to realloc is no longer supported.
    I don't know of any programs still relying on this feature,
    and allowing it would also allow too many other incorrect
    usages of realloc to be sensible.

*/

STATIC_IF_MCHECK
#ಆದರೆ __STD_C
Void_t* rEALLOc_impl(Void_t* oldmem, size_t ಬೈಟುಗಳು)
#ಇಲ್ಲದಿದ್ದರೆ
Void_t* rEALLOc_impl(oldmem, ಬೈಟುಗಳು) Void_t* oldmem; size_t ಬೈಟುಗಳು;
#ಆದರೆ_ಅಂತ್ಯ
{
  ಆಂತರಿಕ_ಗಾತ್ರ_T    nb;      /* padded request size */

  mchunkptr oldp;             /* chunk corresponding to oldmem */
  ಆಂತರಿಕ_ಗಾತ್ರ_T    oldsize; /* its size */

  mchunkptr newp;             /* chunk to return */
  ಆಂತರಿಕ_ಗಾತ್ರ_T    newsize; /* its size */
  Void_t*   newmem;           /* corresponding user mem */

  mchunkptr ಮುಂದಿನ;             /* next contiguous chunk after oldp */
  ಆಂತರಿಕ_ಗಾತ್ರ_T  nextsize;  /* its size */

  mchunkptr prev;             /* previous contiguous chunk before oldp */
  ಆಂತರಿಕ_ಗಾತ್ರ_T  prevsize;  /* its size */

  mchunkptr remainder;        /* holds split off extra space from newp */
  ಆಂತರಿಕ_ಗಾತ್ರ_T  remainder_ಗಾತ್ರ;   /* its size */

  mchunkptr bck;              /* misc temp for linking */
  mchunkptr fwd;              /* misc temp for linking */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ REALLOC_ZERO_ಬೈಟುಗಳು_FREES
  ಆದರೆ (!ಬೈಟುಗಳು) {
	fREe_impl(oldmem);
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ (ಬೈಟುಗಳು > CONFIG_SYS_MALLOC_LEN || (ದೀರ್ಘ)ಬೈಟುಗಳು < 0)
     ಹಿಂತಿರುಗಿ ಖಾಲಿ;

  /* realloc of null is supposed to be same as malloc */
  ಆದರೆ (oldmem == ಖಾಲಿ) ಹಿಂತಿರುಗಿ mALLOc_impl(ಬೈಟುಗಳು);

#ಆದರೆ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ಆದರೆ (!(gd->flags & GD_FLG_ತುಂಬಿದ_MALLOC_INIT)) {
		/* This is harder to support and should not be needed */
		panic("pre-reloc realloc() is not supported");
	}
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ (CONFIG_IS_ENABLED(UNIT_TEST) && malloc_testing) {
    ಆದರೆ (--malloc_max_allocs < 0)
      ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  }

  newp    = oldp    = mem2chunk(oldmem);
  newsize = oldsize = chunksize(oldp);

  nb = request2size(ಬೈಟುಗಳು);

#ಆದರೆ HAVE_MMAP
  ಆದರೆ (chunk_is_mmapped(oldp))
  {
#ಆದರೆ HAVE_MREMAP
    newp = mremap_chunk(oldp, nb);
    ಆದರೆ(newp) ಹಿಂತಿರುಗಿ chunk2mem(newp);
#ಆದರೆ_ಅಂತ್ಯ
    /* Note the extra SIZE_SZ overhead. */
    ಆದರೆ(oldsize - ಗಾತ್ರ_SZ >= nb) ಹಿಂತಿರುಗಿ oldmem; /* do nothing */
    /* Must alloc, copy, free. */
    newmem = mALLOc_impl(ಬೈಟುಗಳು);
    ಆದರೆ (!newmem)
	ಹಿಂತಿರುಗಿ ಖಾಲಿ; /* propagate failure */
    MALLOC_COPY(newmem, oldmem, oldsize - 2*ಗಾತ್ರ_SZ);
    munmap_chunk(oldp);
    ಹಿಂತಿರುಗಿ newmem;
  }
#ಆದರೆ_ಅಂತ್ಯ

  check_inuse_chunk(oldp);

  ಆದರೆ ((ದೀರ್ಘ)(oldsize) < (ದೀರ್ಘ)(nb))
  {

    /* Try expanding forward */

    ಮುಂದಿನ = chunk_at_ಸ್ಥಾನಾಂತರ(oldp, oldsize);
    ಆದರೆ (ಮುಂದಿನ == ತುದಿ || !inuse(ಮುಂದಿನ))
    {
      nextsize = chunksize(ಮುಂದಿನ);

      /* Forward into top only if a remainder */
      ಆದರೆ (ಮುಂದಿನ == ತುದಿ)
      {
	ಆದರೆ ((ದೀರ್ಘ)(nextsize + newsize) >= (ದೀರ್ಘ)(nb + MINSIZE))
	{
	  newsize += nextsize;
	  ತುದಿ = chunk_at_ಸ್ಥಾನಾಂತರ(oldp, nb);
	  ಗಣ_ತಲೆ(ತುದಿ, (newsize - nb) | PREV_INUSE);
	  ಗಣ_ತಲೆ_ಗಾತ್ರ(oldp, nb);
	  VALGRIND_RESIZEINPLACE_ಖಂಡ(chunk2mem(oldp), 0, ಬೈಟುಗಳು, ಗಾತ್ರ_SZ);
	  VALGRIND_MAKE_MEM_DEFINED(chunk2mem(oldp), ಬೈಟುಗಳು);
	  ಹಿಂತಿರುಗಿ chunk2mem(oldp);
	}
      }

      /* Forward into next chunk */
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (((ದೀರ್ಘ)(nextsize + newsize) >= (ದೀರ್ಘ)(nb)))
      {
	unlink(ಮುಂದಿನ, bck, fwd);
	newsize  += nextsize;
	VALGRIND_RESIZEINPLACE_ಖಂಡ(chunk2mem(oldp), 0, ಬೈಟುಗಳು, ಗಾತ್ರ_SZ);
	VALGRIND_MAKE_MEM_DEFINED(chunk2mem(oldp), ಬೈಟುಗಳು);
	ಹೋಗು ವಿಭಜಿಸು;
      }
    }
    ಇಲ್ಲದಿದ್ದರೆ
    {
      ಮುಂದಿನ = ಖಾಲಿ;
      nextsize = 0;
    }

    /* Try shifting backwards. */

    ಆದರೆ (!prev_inuse(oldp))
    {
      prev = prev_chunk(oldp);
      prevsize = chunksize(prev);

      /* try forward + backward first to save a later consolidation */

      ಆದರೆ (ಮುಂದಿನ != ಖಾಲಿ)
      {
	/* into top */
	ಆದರೆ (ಮುಂದಿನ == ತುದಿ)
	{
	  ಆದರೆ ((ದೀರ್ಘ)(nextsize + prevsize + newsize) >= (ದೀರ್ಘ)(nb + MINSIZE))
	  {
	    unlink(prev, bck, fwd);
	    newp = prev;
	    newsize += prevsize + nextsize;
	    newmem = chunk2mem(newp);
	    VALGRIND_MALLOCLIKE_ಖಂಡ(newmem, ಬೈಟುಗಳು, ಗಾತ್ರ_SZ, false);
	    MALLOC_COPY(newmem, oldmem, oldsize - ಗಾತ್ರ_SZ);
	    ತುದಿ = chunk_at_ಸ್ಥಾನಾಂತರ(newp, nb);
	    ಗಣ_ತಲೆ(ತುದಿ, (newsize - nb) | PREV_INUSE);
	    ಗಣ_ತಲೆ_ಗಾತ್ರ(newp, nb);
	    VALGRIND_FREELIKE_ಖಂಡ(oldmem, ಗಾತ್ರ_SZ);
	    ಹಿಂತಿರುಗಿ newmem;
	  }
	}

	/* into next chunk */
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (((ದೀರ್ಘ)(nextsize + prevsize + newsize) >= (ದೀರ್ಘ)(nb)))
	{
	  unlink(ಮುಂದಿನ, bck, fwd);
	  unlink(prev, bck, fwd);
	  newp = prev;
	  newsize += nextsize + prevsize;
	  newmem = chunk2mem(newp);
	  VALGRIND_MALLOCLIKE_ಖಂಡ(newmem, ಬೈಟುಗಳು, ಗಾತ್ರ_SZ, false);
	  MALLOC_COPY(newmem, oldmem, oldsize - ಗಾತ್ರ_SZ);
	  ಹೋಗು ವಿಭಜಿಸು;
	}
      }

      /* backward only */
      ಆದರೆ (prev != ಖಾಲಿ && (ದೀರ್ಘ)(prevsize + newsize) >= (ದೀರ್ಘ)nb)
      {
	unlink(prev, bck, fwd);
	newp = prev;
	newsize += prevsize;
	newmem = chunk2mem(newp);
	VALGRIND_MALLOCLIKE_ಖಂಡ(newmem, ಬೈಟುಗಳು, ಗಾತ್ರ_SZ, false);
	MALLOC_COPY(newmem, oldmem, oldsize - ಗಾತ್ರ_SZ);
	ಹೋಗು ವಿಭಜಿಸು;
      }
    }

    /* Must allocate */

    newmem = mALLOc_impl (ಬೈಟುಗಳು);

    ಆದರೆ (newmem == ಖಾಲಿ)  /* propagate failure */
      ಹಿಂತಿರುಗಿ ಖಾಲಿ;

    /* Avoid copy if newp is next chunk after oldp. */
    /* (This can only happen when new chunk is sbrk'ed.) */

    ಆದರೆ ( (newp = mem2chunk(newmem)) == ಮುಂದಿನ_chunk(oldp))
    {
      newsize += chunksize(newp);
      newp = oldp;
      ಹೋಗು ವಿಭಜಿಸು;
    }

    /* Otherwise copy, free, and exit */
    MALLOC_COPY(newmem, oldmem, oldsize - ಗಾತ್ರ_SZ);
    fREe_impl(oldmem);
    ಹಿಂತಿರುಗಿ newmem;
  } ಇಲ್ಲದಿದ್ದರೆ {
    VALGRIND_RESIZEINPLACE_ಖಂಡ(oldmem, 0, ಬೈಟುಗಳು, ಗಾತ್ರ_SZ);
    VALGRIND_MAKE_MEM_DEFINED(oldmem, ಬೈಟುಗಳು);
  }

 ವಿಭಜಿಸು:  /* split off extra room in old or expanded chunk */

  ಆದರೆ (newsize - nb >= MINSIZE) /* split off remainder */
  {
    remainder = chunk_at_ಸ್ಥಾನಾಂತರ(newp, nb);
    remainder_ಗಾತ್ರ = newsize - nb;
    ಗಣ_ತಲೆ_ಗಾತ್ರ(newp, nb);
    ಗಣ_ತಲೆ(remainder, remainder_ಗಾತ್ರ | PREV_INUSE);
    ಗಣ_inuse_ಬಿಟ್_at_ಸ್ಥಾನಾಂತರ(remainder, remainder_ಗಾತ್ರ);
    VALGRIND_MALLOCLIKE_ಖಂಡ(chunk2mem(remainder), remainder_ಗಾತ್ರ, ಗಾತ್ರ_SZ,
			      false);
    fREe_impl(chunk2mem(remainder)); /* let free() deal with it */
  }
  ಇಲ್ಲದಿದ್ದರೆ
  {
    ಗಣ_ತಲೆ_ಗಾತ್ರ(newp, newsize);
    ಗಣ_inuse_ಬಿಟ್_at_ಸ್ಥಾನಾಂತರ(newp, newsize);
  }

  check_inuse_chunk(newp);
  ಹಿಂತಿರುಗಿ chunk2mem(newp);
}

/*

  memalign algorithm:

    memalign requests more than enough space from malloc, finds a spot
    within that chunk that meets the alignment request, and then
    possibly frees the leading and trailing space.

    The alignment argument must be a power of two. This property is not
    checked by memalign, so misuse may result in random runtime errors.

    8-byte alignment is guaranteed by normal malloc calls, so don't
    bother calling memalign with an argument of 8 or less.

    Overreliance on memalign is a sure way to fragment space.

*/

STATIC_IF_MCHECK
#ಆದರೆ __STD_C
Void_t* mEMALIGn_impl(size_t alignment, size_t ಬೈಟುಗಳು)
#ಇಲ್ಲದಿದ್ದರೆ
Void_t* mEMALIGn_impl(alignment, ಬೈಟುಗಳು) size_t alignment; size_t ಬೈಟುಗಳು;
#ಆದರೆ_ಅಂತ್ಯ
{
  ಆಂತರಿಕ_ಗಾತ್ರ_T    nb;      /* padded  request size */
  ಅಕ್ಷರ*     m;                /* memory returned by malloc call */
  mchunkptr p;                /* corresponding chunk */
  ಅಕ್ಷರ*     brk;              /* alignment point within p */
  mchunkptr newp;             /* chunk to return */
  ಆಂತರಿಕ_ಗಾತ್ರ_T  newsize;   /* its size */
  ಆಂತರಿಕ_ಗಾತ್ರ_T  leadsize;  /* leading space befor alignment point */
  mchunkptr remainder;        /* spare room at end to split off */
  ದೀರ್ಘ      remainder_ಗಾತ್ರ;   /* its size */

  ಆದರೆ (ಬೈಟುಗಳು > CONFIG_SYS_MALLOC_LEN || (ದೀರ್ಘ)ಬೈಟುಗಳು < 0)
     ಹಿಂತಿರುಗಿ ಖಾಲಿ;

#ಆದರೆ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ಆದರೆ (!(gd->flags & GD_FLG_ತುಂಬಿದ_MALLOC_INIT)) {
		ಹಿಂತಿರುಗಿ memalign_simple(alignment, ಬೈಟುಗಳು);
	}
#ಆದರೆ_ಅಂತ್ಯ

  /* If need less alignment than we give anyway, just relay to malloc */

  ಆದರೆ (alignment <= MALLOC_ALIGNMENT) ಹಿಂತಿರುಗಿ mALLOc_impl(ಬೈಟುಗಳು);

  /* Otherwise, ensure that it is at least a minimum chunk size */

  ಆದರೆ (alignment <  MINSIZE) alignment = MINSIZE;

  /* Call malloc with worst case padding to hit alignment. */

  nb = request2size(ಬೈಟುಗಳು);
  m  = (ಅಕ್ಷರ*)(mALLOc_impl(nb + alignment + MINSIZE));

  /*
  * The attempt to over-allocate (with a size large enough to guarantee the
  * ability to find an aligned region within allocated memory) failed.
  *
  * Try again, this time only allocating exactly the size the user wants. If
  * the allocation now succeeds and just happens to be aligned, we can still
  * fulfill the user's request.
  */
  ಆದರೆ (m == ಖಾಲಿ) {
    size_t extra, extra2;
    /*
     * Use bytes not nb, since mALLOc internally calls request2size too, and
     * each call increases the size to allocate, to account for the header.
     */
    m  = (ಅಕ್ಷರ*)(mALLOc_impl(ಬೈಟುಗಳು));
    /* Aligned -> return it */
    ಆದರೆ ((((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(m)) % alignment) == 0)
      ಹಿಂತಿರುಗಿ m;
    /*
     * Otherwise, try again, requesting enough extra space to be able to
     * acquire alignment.
     */
    fREe_impl(m);
    /* Add in extra bytes to match misalignment of unexpanded allocation */
    extra = alignment - (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(m)) % alignment);
    m  = (ಅಕ್ಷರ*)(mALLOc_impl(ಬೈಟುಗಳು + extra));
    /*
     * m might not be the same as before. Validate that the previous value of
     * extra still works for the current value of m.
     * If (!m), extra2=alignment so 
     */
    ಆದರೆ (m) {
      extra2 = alignment - (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(m)) % alignment);
      ಆದರೆ (extra2 > extra) {
        fREe_impl(m);
        m = ಖಾಲಿ;
      }
    }
    /* Fall through to original NULL check and chunk splitting logic */
  }

  ಆದರೆ (m == ಖಾಲಿ) ಹಿಂತಿರುಗಿ ಖಾಲಿ; /* propagate failure */

  p = mem2chunk(m);

  ಆದರೆ ((((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(m)) % alignment) == 0) /* aligned */
  {
#ಆದರೆ HAVE_MMAP
    ಆದರೆ(chunk_is_mmapped(p))
      ಹಿಂತಿರುಗಿ chunk2mem(p); /* nothing more to do */
#ಆದರೆ_ಅಂತ್ಯ
  }
  ಇಲ್ಲದಿದ್ದರೆ /* misaligned */
  {
    /*
      Find an aligned spot inside chunk.
      Since we need to give back leading space in a chunk of at
      least MINSIZE, if the first calculation places us at
      a spot with less than MINSIZE leader, we can move to the
      next aligned spot -- we've allocated enough total room so that
      this is always possible.
    */

    brk = (ಅಕ್ಷರ*)mem2chunk(((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(m + alignment - 1)) & -((ಚಿಹ್ನೆಯುಳ್ಳ) alignment));
    ಆದರೆ ((ದೀರ್ಘ)(brk - (ಅಕ್ಷರ*)(p)) < MINSIZE) brk = brk + alignment;

    newp = (mchunkptr)brk;
    leadsize = brk - (ಅಕ್ಷರ*)(p);
    newsize = chunksize(p) - leadsize;

#ಆದರೆ HAVE_MMAP
    ಆದರೆ(chunk_is_mmapped(p))
    {
      newp->prev_ಗಾತ್ರ = p->prev_ಗಾತ್ರ + leadsize;
      ಗಣ_ತಲೆ(newp, newsize|IS_MMAPPED);
      ಹಿಂತಿರುಗಿ chunk2mem(newp);
    }
#ಆದರೆ_ಅಂತ್ಯ

    /* give back leader, use the rest */

    ಗಣ_ತಲೆ(newp, newsize | PREV_INUSE);
    ಗಣ_inuse_ಬಿಟ್_at_ಸ್ಥಾನಾಂತರ(newp, newsize);
    ಗಣ_ತಲೆ_ಗಾತ್ರ(p, leadsize);
    fREe_impl(chunk2mem(p));
    p = newp;
    VALGRIND_MALLOCLIKE_ಖಂಡ(chunk2mem(p), ಬೈಟುಗಳು, ಗಾತ್ರ_SZ, false);

    ಪ್ರತಿಜ್ಞಾ (newsize >= nb && (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(chunk2mem(p))) % alignment) == 0);
  }

  /* Also give back spare room at the end */

  remainder_ಗಾತ್ರ = chunksize(p) - nb;

  ಆದರೆ (remainder_ಗಾತ್ರ >= (ದೀರ್ಘ)MINSIZE)
  {
    remainder = chunk_at_ಸ್ಥಾನಾಂತರ(p, nb);
    ಗಣ_ತಲೆ(remainder, remainder_ಗಾತ್ರ | PREV_INUSE);
    ಗಣ_ತಲೆ_ಗಾತ್ರ(p, nb);
    VALGRIND_MALLOCLIKE_ಖಂಡ(chunk2mem(remainder), remainder_ಗಾತ್ರ, ಗಾತ್ರ_SZ,
			      false);
    fREe_impl(chunk2mem(remainder));
  }

  check_inuse_chunk(p);
  ಹಿಂತಿರುಗಿ chunk2mem(p);

}

/*
    valloc just invokes memalign with alignment argument equal
    to the page size of the system (or as near to this as can
    be figured out from all the includes/defines above.)
*/

#ಆದರೆ __STD_C
Void_t* vALLOc(size_t ಬೈಟುಗಳು)
#ಇಲ್ಲದಿದ್ದರೆ
Void_t* vALLOc(ಬೈಟುಗಳು) size_t ಬೈಟುಗಳು;
#ಆದರೆ_ಅಂತ್ಯ
{
  ಹಿಂತಿರುಗಿ mEMALIGn (malloc_getpagesize, ಬೈಟುಗಳು);
}

/*
  pvalloc just invokes valloc for the nearest pagesize
  that will accommodate request
*/

#ಆದರೆ __STD_C
Void_t* pvALLOc(size_t ಬೈಟುಗಳು)
#ಇಲ್ಲದಿದ್ದರೆ
Void_t* pvALLOc(ಬೈಟುಗಳು) size_t ಬೈಟುಗಳು;
#ಆದರೆ_ಅಂತ್ಯ
{
  size_t pagesize = malloc_getpagesize;
  ಹಿಂತಿರುಗಿ mEMALIGn (pagesize, (ಬೈಟುಗಳು + pagesize - 1) & ~(pagesize - 1));
}

/*

  calloc calls malloc, then zeroes out the allocated chunk.

*/

STATIC_IF_MCHECK
#ಆದರೆ __STD_C
Void_t* cALLOc_impl(size_t n, size_t elem_ಗಾತ್ರ)
#ಇಲ್ಲದಿದ್ದರೆ
Void_t* cALLOc_impl(n, elem_ಗಾತ್ರ) size_t n; size_t elem_ಗಾತ್ರ;
#ಆದರೆ_ಅಂತ್ಯ
{
  mchunkptr p;
  ಆಂತರಿಕ_ಗಾತ್ರ_T csz;

  ಆಂತರಿಕ_ಗಾತ್ರ_T sz = n * elem_ಗಾತ್ರ;

  /* check if expand_top called, in which case don't need to clear */
#ಆದರೆ CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
#ಆದರೆ MORECORE_CLEARS
  mchunkptr oldtop = ತುದಿ;
  ಆಂತರಿಕ_ಗಾತ್ರ_T oldtopsize = chunksize(ತುದಿ);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
  Void_t* mem = mALLOc_impl (sz);

  ಆದರೆ ((ದೀರ್ಘ)n < 0) ಹಿಂತಿರುಗಿ ಖಾಲಿ;

  ಆದರೆ (mem == ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  ಇಲ್ಲದಿದ್ದರೆ
  {
#ಆದರೆ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ಆದರೆ (!(gd->flags & GD_FLG_ತುಂಬಿದ_MALLOC_INIT)) {
		memset(mem, 0, sz);
		ಹಿಂತಿರುಗಿ mem;
	}
#ಆದರೆ_ಅಂತ್ಯ
    p = mem2chunk(mem);

    /* Two optional cases in which clearing not necessary */

#ಆದರೆ HAVE_MMAP
    ಆದರೆ (chunk_is_mmapped(p)) ಹಿಂತಿರುಗಿ mem;
#ಆದರೆ_ಅಂತ್ಯ

    csz = chunksize(p);

#ಆದರೆ CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
#ಆದರೆ MORECORE_CLEARS
    ಆದರೆ (p == oldtop && csz > oldtopsize)
    {
      /* clear only the bytes from non-freshly-sbrked memory */
      csz = oldtopsize;
    }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

    MALLOC_ZERO(mem, csz - ಗಾತ್ರ_SZ);
    VALGRIND_MAKE_MEM_DEFINED(mem, sz);
    ಹಿಂತಿರುಗಿ mem;
  }
}

/*

  cfree just calls free. It is needed/defined on some systems
  that pair it with calloc, presumably for odd historical reasons.

*/

#ಆದರೆ !defined(ಆಂತರಿಕ_LINUX_C_LIB) || !defined(__ELF__)
#ಆದರೆ __STD_C
ಶೂನ್ಯ cfree(Void_t *mem)
#ಇಲ್ಲದಿದ್ದರೆ
ಶೂನ್ಯ cfree(mem) Void_t *mem;
#ಆದರೆ_ಅಂತ್ಯ
{
  fREe(mem);
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ MCHECK_ರಾಶಿ_PROTECTION
 #ಅನ್ತರ್ಭಾವ "mcheck_core.inc.h"
 #ಆದರೆ !__STD_C
  #ದೋಷ "must have __STD_C"
 #ಆದರೆ_ಅಂತ್ಯ

Void_t *mALLOc(size_t ಬೈಟುಗಳು)
{
	mcheck_pedantic_prehook();
	size_t fullsz = mcheck_alloc_prehook(ಬೈಟುಗಳು);
	ಶೂನ್ಯ *p = mALLOc_impl(fullsz);

	ಆದರೆ (!p)
		ಹಿಂತಿರುಗಿ p;
	ಹಿಂತಿರುಗಿ mcheck_alloc_posthook(p, ಬೈಟುಗಳು);
}

ಶೂನ್ಯ fREe(Void_t *mem) { fREe_impl(mcheck_free_prehook(mem)); }

Void_t *rEALLOc(Void_t *oldmem, size_t ಬೈಟುಗಳು)
{
	mcheck_pedantic_prehook();
	ಆದರೆ (ಬೈಟುಗಳು == 0) {
		ಆದರೆ (oldmem)
			fREe(oldmem);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಆದರೆ (oldmem == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ mALLOc(ಬೈಟುಗಳು);

	ಶೂನ್ಯ *p = mcheck_reallocfree_prehook(oldmem);
	size_t newsz = mcheck_alloc_prehook(ಬೈಟುಗಳು);

	p = rEALLOc_impl(p, newsz);
	ಆದರೆ (!p)
		ಹಿಂತಿರುಗಿ p;
	ಹಿಂತಿರುಗಿ mcheck_alloc_noclean_posthook(p, ಬೈಟುಗಳು);
}

Void_t *mEMALIGn(size_t alignment, size_t ಬೈಟುಗಳು)
{
	mcheck_pedantic_prehook();
	size_t fullsz = mcheck_memalign_prehook(alignment, ಬೈಟುಗಳು);
	ಶೂನ್ಯ *p = mEMALIGn_impl(alignment, fullsz);

	ಆದರೆ (!p)
		ಹಿಂತಿರುಗಿ p;
	ಹಿಂತಿರುಗಿ mcheck_memalign_posthook(alignment, p, ಬೈಟುಗಳು);
}

// pvALLOc, vALLOc - redirect to mEMALIGn, defined here, so they need no wrapping.

Void_t *cALLOc(size_t n, size_t elem_ಗಾತ್ರ)
{
	mcheck_pedantic_prehook();
	// NB: here is no overflow check.
	size_t fullsz = mcheck_alloc_prehook(n * elem_ಗಾತ್ರ);
	ಶೂನ್ಯ *p = cALLOc_impl(1, fullsz);

	ಆದರೆ (!p)
		ಹಿಂತಿರುಗಿ p;
	ಹಿಂತಿರುಗಿ mcheck_alloc_noclean_posthook(p, n * elem_ಗಾತ್ರ);
}

// mcheck API {
ಪೂರ್ಣಾಂಕ mcheck_pedantic(mcheck_abortfunc_t f)
{
	mcheck_initialize(f, 1);
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ mcheck(mcheck_abortfunc_t f)
{
	mcheck_initialize(f, 0);
	ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ mcheck_check_all(ಶೂನ್ಯ) { mcheck_pedantic_check(); }

ಎಣಿಕೆ mcheck_status mprobe(ಶೂನ್ಯ *__ptr) { ಹಿಂತಿರುಗಿ mcheck_mprobe(__ptr); }
// mcheck API }
#ಆದರೆ_ಅಂತ್ಯ

/*

    Malloc_trim gives memory back to the system (via negative
    arguments to sbrk) if there is unused memory at the `high' end of
    the malloc pool. You can call this after freeing large blocks of
    memory to potentially reduce the system-level memory requirements
    of a program. However, it cannot guarantee to reduce memory. Under
    some allocation patterns, some large free blocks of memory will be
    locked between two used chunks, so they cannot be given back to
    the system.

    The `pad' argument to malloc_trim represents the amount of free
    trailing space to leave untrimmed. If this argument is zero,
    only the minimum amount of memory to maintain internal data
    structures will be left (one page or less). Non-zero arguments
    can be supplied to maintain enough trailing space to service
    future expected allocations without having to re-obtain memory
    from the system.

    Malloc_trim returns 1 if it actually released any memory, else 0.

*/

#ಆದರೆ __STD_C
ಪೂರ್ಣಾಂಕ malloc_trim(size_t pad)
#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ malloc_trim(pad) size_t pad;
#ಆದರೆ_ಅಂತ್ಯ
{
  ದೀರ್ಘ  ತುದಿ_ಗಾತ್ರ;        /* Amount of top-most memory */
  ದೀರ್ಘ  extra;           /* Amount to release */
  ಅಕ್ಷರ* current_brk;     /* address returned by pre-check sbrk call */
  ಅಕ್ಷರ* new_brk;         /* address returned by negative sbrk call */

  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ pagesz = malloc_getpagesize;

  ತುದಿ_ಗಾತ್ರ = chunksize(ತುದಿ);
  extra = ((ತುದಿ_ಗಾತ್ರ - pad - MINSIZE + (pagesz-1)) / pagesz - 1) * pagesz;

  ಆದರೆ (extra < (ದೀರ್ಘ)pagesz)  /* Not enough memory to release */
    ಹಿಂತಿರುಗಿ 0;

  ಇಲ್ಲದಿದ್ದರೆ
  {
    /* Test to make sure no one else called sbrk */
    current_brk = (ಅಕ್ಷರ*)(MORECORE (0));
    ಆದರೆ (current_brk != (ಅಕ್ಷರ*)(ತುದಿ) + ತುದಿ_ಗಾತ್ರ)
      ಹಿಂತಿರುಗಿ 0;     /* Apparently we don't own memory; must fail */

    ಇಲ್ಲದಿದ್ದರೆ
    {
      new_brk = (ಅಕ್ಷರ*)(MORECORE (-extra));

      ಆದರೆ (new_brk == (ಅಕ್ಷರ*)(MORECORE_FAILURE)) /* sbrk failed? */
      {
	/* Try to figure out what we have */
	current_brk = (ಅಕ್ಷರ*)(MORECORE (0));
	ತುದಿ_ಗಾತ್ರ = current_brk - (ಅಕ್ಷರ*)ತುದಿ;
	ಆದರೆ (ತುದಿ_ಗಾತ್ರ >= (ದೀರ್ಘ)MINSIZE) /* if not, we are very very dead! */
	{
	  sbrked_mem = current_brk - sbrk_ತಳ;
	  ಗಣ_ತಲೆ(ತುದಿ, ತುದಿ_ಗಾತ್ರ | PREV_INUSE);
	}
	check_chunk(ತುದಿ);
	ಹಿಂತಿರುಗಿ 0;
      }

      ಇಲ್ಲದಿದ್ದರೆ
      {
	/* Success. Adjust top accordingly. */
	ಗಣ_ತಲೆ(ತುದಿ, (ತುದಿ_ಗಾತ್ರ - extra) | PREV_INUSE);
	sbrked_mem -= extra;
	check_chunk(ತುದಿ);
	ಹಿಂತಿರುಗಿ 1;
      }
    }
  }
}

/*
  malloc_usable_size:

    This routine tells you how many bytes you can actually use in an
    allocated chunk, which may be more than you requested (although
    often not). You can use this many bytes without worrying about
    overwriting other allocated objects. Not a particularly great
    programming practice, but still sometimes useful.

*/

#ಆದರೆ __STD_C
size_t malloc_usable_size(Void_t* mem)
#ಇಲ್ಲದಿದ್ದರೆ
size_t malloc_usable_size(mem) Void_t* mem;
#ಆದರೆ_ಅಂತ್ಯ
{
  mchunkptr p;
  ಆದರೆ (mem == ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ 0;
  ಇಲ್ಲದಿದ್ದರೆ
  {
    p = mem2chunk(mem);
    ಆದರೆ(!chunk_is_mmapped(p))
    {
      ಆದರೆ (!inuse(p)) ಹಿಂತಿರುಗಿ 0;
      check_inuse_chunk(p);
      ಹಿಂತಿರುಗಿ chunksize(p) - ಗಾತ್ರ_SZ;
    }
    ಹಿಂತಿರುಗಿ chunksize(p) - 2*ಗಾತ್ರ_SZ;
  }
}

/* Utility to update current_mallinfo for malloc_stats and mallinfo() */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DEBUG
ಸ್ಥಿರ ಶೂನ್ಯ malloc_update_mallinfo(ಶೂನ್ಯ)
{
  ಪೂರ್ಣಾಂಕ i;
  mbinptr b;
  mchunkptr p;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DEBUG
  mchunkptr q;
#ಆದರೆ_ಅಂತ್ಯ

  ಆಂತರಿಕ_ಗಾತ್ರ_T avail = chunksize(ತುದಿ);
  ಪೂರ್ಣಾಂಕ   navail = ((ದೀರ್ಘ)(avail) >= (ದೀರ್ಘ)MINSIZE)? 1 : 0;

  ಗಾಗಿ (i = 1; i < NAV; ++i)
  {
    b = bin_at(i);
    ಗಾಗಿ (p = ಕೊನೆಯ(b); p != b; p = p->bk)
    {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DEBUG
      check_free_chunk(p);
      ಗಾಗಿ (q = ಮುಂದಿನ_chunk(p);
	   q < ತುದಿ && inuse(q) && (ದೀರ್ಘ)(chunksize(q)) >= (ದೀರ್ಘ)MINSIZE;
	   q = ಮುಂದಿನ_chunk(q))
	check_inuse_chunk(q);
#ಆದರೆ_ಅಂತ್ಯ
      avail += chunksize(p);
      navail++;
    }
  }

  current_mallinfo.ordblks = navail;
  current_mallinfo.uordblks = sbrked_mem - avail;
  current_mallinfo.fordblks = avail;
  current_mallinfo.hblks = n_mmaps;
  current_mallinfo.hblkhd = mmapped_mem;
  current_mallinfo.keepcost = chunksize(ತುದಿ);

}
#ಆದರೆ_ಅಂತ್ಯ	/* DEBUG */

/*

  malloc_stats:

    Prints on the amount of space obtain from the system (both
    via sbrk and mmap), the maximum amount (which may be more than
    current if malloc_trim and/or munmap got called), the maximum
    number of simultaneous mmap regions used, and the current number
    of bytes allocated via malloc (or realloc, etc) but not yet
    freed. (Note that this is the number of bytes allocated, not the
    number requested. It will be larger than the number requested
    because of alignment and bookkeeping overhead.)

*/

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DEBUG
ಶೂನ್ಯ malloc_stats(ಶೂನ್ಯ)
{
  malloc_update_mallinfo();
  ಬರೆ("max system bytes = %10u\n",
	  (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)(max_ಒಟ್ಟು_mem));
  ಬರೆ("system bytes     = %10u\n",
	  (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)(sbrked_mem + mmapped_mem));
  ಬರೆ("in use bytes     = %10u\n",
	  (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)(current_mallinfo.uordblks + mmapped_mem));
#ಆದರೆ HAVE_MMAP
  ಬರೆ("max mmap regions = %10u\n",
	  (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)max_n_mmaps);
#ಆದರೆ_ಅಂತ್ಯ
}
#ಆದರೆ_ಅಂತ್ಯ	/* DEBUG */

/*
  mallinfo returns a copy of updated current mallinfo.
*/

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DEBUG
ರಚನೆ mallinfo mALLINFo(ಶೂನ್ಯ)
{
  malloc_update_mallinfo();
  ಹಿಂತಿರುಗಿ current_mallinfo;
}
#ಆದರೆ_ಅಂತ್ಯ	/* DEBUG */

/*
  mallopt:

    mallopt is the general SVID/XPG interface to tunable parameters.
    The format is to provide a (parameter-number, parameter-value) pair.
    mallopt then sets the corresponding parameter to the argument
    value if it can (i.e., so long as the value is meaningful),
    and returns 1 if successful else 0.

    See descriptions of tunable parameters above.

*/

#ಆದರೆ __STD_C
ಪೂರ್ಣಾಂಕ mALLOPt(ಪೂರ್ಣಾಂಕ param_ಸಂಖ್ಯೆ, ಪೂರ್ಣಾಂಕ ಮೌಲ್ಯ)
#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ mALLOPt(param_ಸಂಖ್ಯೆ, ಮೌಲ್ಯ) ಪೂರ್ಣಾಂಕ param_ಸಂಖ್ಯೆ; ಪೂರ್ಣಾಂಕ ಮೌಲ್ಯ;
#ಆದರೆ_ಅಂತ್ಯ
{
  ಆಯ್ಕೆ(param_ಸಂಖ್ಯೆ)
  {
    ಸ್ಥಿತಿ M_TRIM_THRESHOLD:
      trim_ಮಿತಿ = ಮೌಲ್ಯ; ಹಿಂತಿರುಗಿ 1;
    ಸ್ಥಿತಿ M_TOP_PAD:
      ತುದಿ_pad = ಮೌಲ್ಯ; ಹಿಂತಿರುಗಿ 1;
    ಸ್ಥಿತಿ M_MMAP_THRESHOLD:
      mmap_ಮಿತಿ = ಮೌಲ್ಯ; ಹಿಂತಿರುಗಿ 1;
    ಸ್ಥಿತಿ M_MMAP_MAX:
#ಆದರೆ HAVE_MMAP
      n_mmaps_max = ಮೌಲ್ಯ; ಹಿಂತಿರುಗಿ 1;
#ಇಲ್ಲದಿದ್ದರೆ
      ಆದರೆ (ಮೌಲ್ಯ != 0) ಹಿಂತಿರುಗಿ 0; ಇಲ್ಲದಿದ್ದರೆ  n_mmaps_max = ಮೌಲ್ಯ; ಹಿಂತಿರುಗಿ 1;
#ಆದರೆ_ಅಂತ್ಯ

    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ 0;
  }
}

ಪೂರ್ಣಾಂಕ initf_malloc(ಶೂನ್ಯ)
{
#ಆದರೆ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ಪ್ರತಿಜ್ಞಾ(gd->malloc_ತಳ);	/* Set up by crt0.S */
	gd->malloc_ಎಲ್ಲೆ = CONFIG_VAL(SYS_MALLOC_F_LEN);
	gd->malloc_ptr = 0;
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ malloc_enable_testing(ಪೂರ್ಣಾಂಕ max_allocs)
{
	malloc_testing = true;
	malloc_max_allocs = max_allocs;
}

ಶೂನ್ಯ malloc_disable_testing(ಶೂನ್ಯ)
{
	malloc_testing = false;
}

/*

History:

    V2.6.6 Sun Dec  5 07:42:19 1999  Doug Lea  (dl at gee)
      * return null for negative arguments
      * Added Several WIN32 cleanups from Martin C. Fong <mcfong@yahoo.com>
	 * Add 'LACKS_SYS_PARAM_H' for those systems without 'sys/param.h'
	  (e.g. WIN32 platforms)
	 * Cleanup up header file inclusion for WIN32 platforms
	 * Cleanup code to avoid Microsoft Visual C++ compiler complaints
	 * Add 'USE_DL_PREFIX' to quickly allow co-existence with existing
	   memory allocation routines
	 * Set 'malloc_getpagesize' for WIN32 platforms (needs more work)
	 * Use 'assert' rather than 'ASSERT' in WIN32 code to conform to
	   usage of 'assert' in non-WIN32 code
	 * Improve WIN32 'sbrk()' emulation's 'findRegion()' routine to
	   avoid infinite loop
      * Always call 'fREe()' rather than 'free()'

    V2.6.5 Wed Jun 17 15:57:31 1998  Doug Lea  (dl at gee)
      * Fixed ordering problem with boundary-stamping

    V2.6.3 Sun May 19 08:17:58 1996  Doug Lea  (dl at gee)
      * Added pvalloc, as recommended by H.J. Liu
      * Added 64bit pointer support mainly from Wolfram Gloger
      * Added anonymously donated WIN32 sbrk emulation
      * Malloc, calloc, getpagesize: add optimizations from Raymond Nijssen
      * malloc_extend_top: fix mask error that caused wastage after
	foreign sbrks
      * Add linux mremap support code from HJ Liu

    V2.6.2 Tue Dec  5 06:52:55 1995  Doug Lea  (dl at gee)
      * Integrated most documentation with the code.
      * Add support for mmap, with help from
	Wolfram Gloger (Gloger@lrz.uni-muenchen.de).
      * Use last_remainder in more cases.
      * Pack bins using idea from  colin@nyx10.cs.du.edu
      * Use ordered bins instead of best-fit threshhold
      * Eliminate block-local decls to simplify tracing and debugging.
      * Support another case of realloc via move into top
      * Fix error occuring when initial sbrk_base not word-aligned.
      * Rely on page size for units instead of SBRK_UNIT to
	avoid surprises about sbrk alignment conventions.
      * Add mallinfo, mallopt. Thanks to Raymond Nijssen
	(raymond@es.ele.tue.nl) for the suggestion.
      * Add `pad' argument to malloc_trim and top_pad mallopt parameter.
      * More precautions for cases where other routines call sbrk,
	courtesy of Wolfram Gloger (Gloger@lrz.uni-muenchen.de).
      * Added macros etc., allowing use in linux libc from
	H.J. Lu (hjl@gnu.ai.mit.edu)
      * Inverted this history list

    V2.6.1 Sat Dec  2 14:10:57 1995  Doug Lea  (dl at gee)
      * Re-tuned and fixed to behave more nicely with V2.6.0 changes.
      * Removed all preallocation code since under current scheme
	the work required to undo bad preallocations exceeds
	the work saved in good cases for most test programs.
      * No longer use return list or unconsolidated bins since
	no scheme using them consistently outperforms those that don't
	given above changes.
      * Use best fit for very large chunks to prevent some worst-cases.
      * Added some support for debugging

    V2.6.0 Sat Nov  4 07:05:23 1995  Doug Lea  (dl at gee)
      * Removed footers when chunks are in use. Thanks to
	Paul Wilson (wilson@cs.texas.edu) for the suggestion.

    V2.5.4 Wed Nov  1 07:54:51 1995  Doug Lea  (dl at gee)
      * Added malloc_trim, with help from Wolfram Gloger
	(wmglo@Dent.MED.Uni-Muenchen.DE).

    V2.5.3 Tue Apr 26 10:16:01 1994  Doug Lea  (dl at g)

    V2.5.2 Tue Apr  5 16:20:40 1994  Doug Lea  (dl at g)
      * realloc: try to expand in both directions
      * malloc: swap order of clean-bin strategy;
      * realloc: only conditionally expand backwards
      * Try not to scavenge used bins
      * Use bin counts as a guide to preallocation
      * Occasionally bin return list chunks in first scan
      * Add a few optimizations from colin@nyx10.cs.du.edu

    V2.5.1 Sat Aug 14 15:40:43 1993  Doug Lea  (dl at g)
      * faster bin computation & slightly different binning
      * merged all consolidations to one part of malloc proper
	 (eliminating old malloc_find_space & malloc_clean_bin)
      * Scan 2 returns chunks (not just 1)
      * Propagate failure in realloc if malloc returns 0
      * Add stuff to allow compilation on non-ANSI compilers
	  from kpv@research.att.com

    V2.5 Sat Aug  7 07:41:59 1993  Doug Lea  (dl at g.oswego.edu)
      * removed potential for odd address access in prev_chunk
      * removed dependency on getpagesize.h
      * misc cosmetics and a bit more internal documentation
      * anticosmetics: mangled names in macros to evade debugger strangeness
      * tested on sparc, hp-700, dec-mips, rs6000
	  with gcc & native cc (hp, dec only) allowing
	  Detlefs & Zorn comparison study (in SIGPLAN Notices.)

    Trial version Fri Aug 28 13:14:29 1992  Doug Lea  (dl at g.oswego.edu)
      * Based loosely on libg++-1.2X malloc. (It retains some of the overall
	 structure of old version,  but most details differ.)

*/
