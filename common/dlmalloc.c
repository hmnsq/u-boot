// SPDX-License-Identifier: GPL-2.0+
/*
 * This code is based on a version (aka dlmalloc) of malloc/free/realloc written
 * by Doug Lea and released to the public domain, as explained at
 * http://creativecommons.org/publicdomain/zero/1.0/-
 *
 * The original code is available at http://gee.cs.oswego.edu/pub/misc/
 * as file malloc-2.6.6.c.
 */

#إذا CONFIG_IS_ENABLED(UNIT_TEST)
#وصف_ڪريو DEBUG
#جيڪڏهن_پورو

#ضمن <log.h>
#ضمن <asm/global_data.h>

#ضمن <malloc.h>
#ضمن <mapmem.h>
#ضمن <string.h>
#ضمن <asm/io.h>
#ضمن <valgrind/memcheck.h>

#جيڪڏهن_وصف_ٿيل DEBUG
#إذا __STD_C
ساكن فراغ malloc_update_mallinfo (فراغ);
فراغ malloc_stats (فراغ);
#وإلا
ساكن فراغ malloc_update_mallinfo ();
فراغ malloc_stats();
#جيڪڏهن_پورو
#جيڪڏهن_پورو	/* DEBUG */

DECLARE_GLOBAL_معلومات_PTR;

#جيڪڏهن_وصف_ٿيل MCHECK_ڍير_PROTECTION
 #وصف_ڪريو STATIC_IF_MCHECK ساكن
 #وصف_رد MALLOC_COPY
 #وصف_رد MALLOC_ZERO
ساكن مضمن فراغ MALLOC_ZERO(فراغ *p, size_t sz) { memset(p, 0, sz); }
ساكن مضمن فراغ MALLOC_COPY(فراغ *dest, ثابت فراغ *src, size_t sz) { يادگيري_نقل(dest, src, sz); }
#وإلا
 #وصف_ڪريو STATIC_IF_MCHECK
 #وصف_ڪريو mALLOc_impl mALLOc
 #وصف_ڪريو fREe_impl fREe
 #وصف_ڪريو rEALLOc_impl rEALLOc
 #وصف_ڪريو mEMALIGn_impl mEMALIGn
 #وصف_ڪريو cALLOc_impl cALLOc
#جيڪڏهن_پورو

/*
  Emulation of sbrk for WIN32
  All code within the ifdef WIN32 is untested by me.

  Thanks to Martin Fong and others for supplying this.
*/

#جيڪڏهن_وصف_ٿيل WIN32

#وصف_ڪريو Align_صفحو(جوڙيو) (((جوڙيو) + (malloc_getpagesize-1)) & \
~(malloc_getpagesize-1))
#وصف_ڪريو AlignPage64K(جوڙيو) (((جوڙيو) + (0x10000 - 1)) & ~(0x10000 - 1))

/* resrve 64MB to insure large contiguous space */
#وصف_ڪريو RESERVED_سائيز (1024*1024*64)
#وصف_ڪريو ايندڙ_سائيز (2048*1024)
#وصف_ڪريو چوٽي_يادگيري ((بنانشان ڊگهو)2*1024*1024*1024)

بنية GmListElement;
تعريفنوع بنية GmListElement GmListElement;

بنية GmListElement
{
	GmListElement* ايندڙ;
	فراغ* بنياد;
};

ساكن GmListElement* سر = 0;
ساكن بنانشان صحيح g_ايندڙ_پتو = 0;
ساكن بنانشان صحيح g_پتو_بنياد = 0;
ساكن بنانشان صحيح g_Allocated_سائيز = 0;

ساكن
GmListElement* makeGmListElement (فراغ* bas)
{
	GmListElement* this;
	this = (GmListElement*)(فراغ*)LocalAlloc (0, حجم (GmListElement));
	أكد (this);
	إذا (this)
	{
		this->بنياد = bas;
		this->ايندڙ = سر;
		سر = this;
	}
	أرجع this;
}

فراغ gcleanup (فراغ)
{
	BOOL rval;
	أكد ( (سر == عدم) || (سر->بنياد == (فراغ*)g_پتو_بنياد));
	إذا (g_پتو_بنياد && (g_ايندڙ_پتو - g_پتو_بنياد))
	{
		rval = VirtualFree ((فراغ*)g_پتو_بنياد,
							g_ايندڙ_پتو - g_پتو_بنياد,
							MEM_DECOMMIT);
	أكد (rval);
	}
	طالما (سر)
	{
		GmListElement* ايندڙ = سر->ايندڙ;
		rval = VirtualFree (سر->بنياد, 0, MEM_RELEASE);
		أكد (rval);
		LocalFree (سر);
		سر = ايندڙ;
	}
}

ساكن
فراغ* ڳوليو_Region (فراغ* شروعات_پتو, بنانشان ڊگهو سائيز)
{
	يادگيري_BASIC_INFORMATION info;
	إذا (سائيز >= چوٽي_يادگيري) أرجع عدم;

	طالما ((بنانشان ڊگهو)شروعات_پتو + سائيز < چوٽي_يادگيري)
	{
		VirtualQuery (شروعات_پتو, &info, حجم (info));
		إذا ((info.State == MEM_FREE) && (info.Region_سائيز >= سائيز))
			أرجع شروعات_پتو;
		وإلا
		{
			/* Requested region is not available so see if the */
			/* next region is available.  Set 'start_address' */
			/* to the next region and call 'VirtualQuery()' */
			/* again. */

			شروعات_پتو = (حرف*)info.بنياد_پتو + info.Region_سائيز;

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

			شروعات_پتو =
				(فراغ *) AlignPage64K((بنانشان ڊگهو) شروعات_پتو);
		}
	}
	أرجع عدم;

}

فراغ* wsbrk (ڊگهو سائيز)
{
	فراغ* tmp;
	إذا (سائيز > 0)
	{
		إذا (g_پتو_بنياد == 0)
		{
			g_Allocated_سائيز = max (RESERVED_سائيز, Align_صفحو (سائيز));
			g_ايندڙ_پتو = g_پتو_بنياد =
				(بنانشان صحيح)VirtualAlloc (عدم, g_Allocated_سائيز,
											MEM_RESERVE, صفحو_NOACCESS);
		} وإلا إذا (Align_صفحو (g_ايندڙ_پتو + سائيز) > (g_پتو_بنياد +
g_Allocated_سائيز))
		{
			ڊگهو new_سائيز = max (ايندڙ_سائيز, Align_صفحو (سائيز));
			فراغ* new_پتو = (فراغ*)(g_پتو_بنياد+g_Allocated_سائيز);
			افعل
			{
				new_پتو = ڳوليو_Region (new_پتو, new_سائيز);

				إذا (!new_پتو)
					أرجع (فراغ*)-1;

				g_پتو_بنياد = g_ايندڙ_پتو =
					(بنانشان صحيح)VirtualAlloc (new_پتو, new_سائيز,
												MEM_RESERVE, صفحو_NOACCESS);
				/* repeat in case of race condition */
				/* The region that we found has been snagged */
				/* by another thread */
			}
			طالما (g_پتو_بنياد == 0);

			أكد (new_پتو == (فراغ*)g_پتو_بنياد);

			g_Allocated_سائيز = new_سائيز;

			إذا (!makeGmListElement ((فراغ*)g_پتو_بنياد))
				أرجع (فراغ*)-1;
		}
		إذا ((سائيز + g_ايندڙ_پتو) > Align_صفحو (g_ايندڙ_پتو))
		{
			فراغ* res;
			res = VirtualAlloc ((فراغ*)Align_صفحو (g_ايندڙ_پتو),
								(سائيز + g_ايندڙ_پتو -
								 Align_صفحو (g_ايندڙ_پتو)),
								MEM_COMMIT, صفحو_READWRITE);
			إذا (!res)
				أرجع (فراغ*)-1;
		}
		tmp = (فراغ*)g_ايندڙ_پتو;
		g_ايندڙ_پتو = (بنانشان صحيح)tmp + سائيز;
		أرجع tmp;
	}
	وإلا إذا (سائيز < 0)
	{
		بنانشان صحيح alignedGoal = Align_صفحو (g_ايندڙ_پتو + سائيز);
		/* Trim by releasing the virtual memory */
		إذا (alignedGoal >= g_پتو_بنياد)
		{
			VirtualFree ((فراغ*)alignedGoal, g_ايندڙ_پتو - alignedGoal,
						 MEM_DECOMMIT);
			g_ايندڙ_پتو = g_ايندڙ_پتو + سائيز;
			أرجع (فراغ*)g_ايندڙ_پتو;
		}
		وإلا
		{
			VirtualFree ((فراغ*)g_پتو_بنياد, g_ايندڙ_پتو - g_پتو_بنياد,
						 MEM_DECOMMIT);
			g_ايندڙ_پتو = g_پتو_بنياد;
			أرجع (فراغ*)-1;
		}
	}
	وإلا
	{
		أرجع (فراغ*)g_ايندڙ_پتو;
	}
}

#جيڪڏهن_پورو

/*
  Type declarations
*/

بنية malloc_chunk
{
  اندروني_سائيز_T prev_سائيز; /* Size of previous chunk (if free). */
  اندروني_سائيز_T سائيز;      /* Size in bytes, including overhead. */
  بنية malloc_chunk* fd;   /* double links -- used only if free. */
  بنية malloc_chunk* bk;
} __attribute__((__may_alias__)) ;

تعريفنوع بنية malloc_chunk* mchunkptr;

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

#وصف_ڪريو سائيز_SZ                (حجم(اندروني_سائيز_T))
#وصف_ڪريو MALLOC_ALIGNMENT       (سائيز_SZ + سائيز_SZ)
#وصف_ڪريو MALLOC_ALIGN_نقاب      (MALLOC_ALIGNMENT - 1)
#وصف_ڪريو MINSIZE                (حجم(بنية malloc_chunk))

/* conversion from malloc headers to user pointers, and back */

#وصف_ڪريو chunk2mem(p)   ((Void_t*)((حرف*)(p) + 2*سائيز_SZ))
#وصف_ڪريو mem2chunk(mem) ((mchunkptr)((حرف*)(mem) - 2*سائيز_SZ))

/* pad request bytes into a usable size */

#وصف_ڪريو request2size(req) \
 ((((req) + (سائيز_SZ + MALLOC_ALIGN_نقاب)) < \
  (MINSIZE + MALLOC_ALIGN_نقاب)) ? MINSIZE : \
   (((req) + (سائيز_SZ + MALLOC_ALIGN_نقاب)) & ~(MALLOC_ALIGN_نقاب)))

/* Check if m has acceptable alignment */

#وصف_ڪريو aligned_OK(m)    (((بنانشان ڊگهو)((m)) & (MALLOC_ALIGN_نقاب)) == 0)

/*
  Physical chunk operations
*/

/* size field is or'ed with PREV_INUSE when previous adjacent chunk in use */

#وصف_ڪريو PREV_INUSE 0x1

/* size field is or'ed with IS_MMAPPED if the chunk was obtained with mmap() */

#وصف_ڪريو IS_MMAPPED 0x2

/* Bits to mask off when extracting size */

#وصف_ڪريو سائيز_بٽ (PREV_INUSE|IS_MMAPPED)

/* Ptr to next physical malloc_chunk. */

#وصف_ڪريو ايندڙ_chunk(p) ((mchunkptr)( ((حرف*)(p)) + ((p)->سائيز & ~PREV_INUSE) ))

/* Ptr to previous physical malloc_chunk */

#وصف_ڪريو prev_chunk(p)\
   ((mchunkptr)( ((حرف*)(p)) - ((p)->prev_سائيز) ))

/* Treat space at ptr + offset as a chunk */

#وصف_ڪريو chunk_at_وٿي(p, s)  ((mchunkptr)(((حرف*)(p)) + (s)))

/*
  Dealing with use bits
*/

/* extract p's inuse bit */

#وصف_ڪريو inuse(p)\
((((mchunkptr)(((حرف*)(p))+((p)->سائيز & ~PREV_INUSE)))->سائيز) & PREV_INUSE)

/* extract inuse bit of previous chunk */

#وصف_ڪريو prev_inuse(p)  ((p)->سائيز & PREV_INUSE)

/* check for mmap()'ed chunk */

#وصف_ڪريو chunk_is_mmapped(p) ((p)->سائيز & IS_MMAPPED)

/* set/clear chunk as in use without otherwise disturbing */

#وصف_ڪريو سيٽ_inuse(p)\
((mchunkptr)(((حرف*)(p)) + ((p)->سائيز & ~PREV_INUSE)))->سائيز |= PREV_INUSE

#وصف_ڪريو clear_inuse(p)\
((mchunkptr)(((حرف*)(p)) + ((p)->سائيز & ~PREV_INUSE)))->سائيز &= ~(PREV_INUSE)

/* check/set/clear inuse bits in known places */

#وصف_ڪريو inuse_ٻٽو_انگ_at_وٿي(p, s)\
 (((mchunkptr)(((حرف*)(p)) + (s)))->سائيز & PREV_INUSE)

#وصف_ڪريو سيٽ_inuse_ٻٽو_انگ_at_وٿي(p, s)\
 (((mchunkptr)(((حرف*)(p)) + (s)))->سائيز |= PREV_INUSE)

#وصف_ڪريو clear_inuse_ٻٽو_انگ_at_وٿي(p, s)\
 (((mchunkptr)(((حرف*)(p)) + (s)))->سائيز &= ~(PREV_INUSE))

/*
  Dealing with size fields
*/

/* Get size, ignoring use bits */

#وصف_ڪريو chunksize(p)          ((p)->سائيز & ~(سائيز_بٽ))

/* Set size at head, without disturbing its use bit */

#وصف_ڪريو سيٽ_سر_سائيز(p, s)   ((p)->سائيز = (((p)->سائيز & PREV_INUSE) | (s)))

/* Set size/use ignoring previous bits in header */

#وصف_ڪريو سيٽ_سر(p, s)        ((p)->سائيز = (s))

/* Set size at footer (only when chunk is not in use) */

#وصف_ڪريو سيٽ_foot(p, s)   (((mchunkptr)((حرف*)(p) + (s)))->prev_سائيز = (s))

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

#وصف_ڪريو NAV             128   /* number of bins */

تعريفنوع بنية malloc_chunk* mbinptr;

/* access macros */

#وصف_ڪريو bin_at(i)      ((mbinptr)((حرف*)&(av_[2*(i) + 2]) - 2*سائيز_SZ))
#وصف_ڪريو ايندڙ_bin(b)    ((mbinptr)((حرف*)(b) + 2 * حجم(mbinptr)))
#وصف_ڪريو prev_bin(b)    ((mbinptr)((حرف*)(b) - 2 * حجم(mbinptr)))

/*
   The first 2 bins are never indexed. The corresponding av_ cells are instead
   used for bookkeeping. This is not to save space, but to simplify
   indexing, maintain locality, and avoid some initialization tests.
*/

#وصف_ڪريو چوٽي            (av_[2])          /* The topmost chunk */
#وصف_ڪريو آخري_remainder (bin_at(1))       /* remainder from last split */

/*
   Because top initially points to its own bin with initial
   zero size, thus forcing extension on the first malloc request,
   we avoid having any special code in malloc to check whether
   it even exists yet. But we still need to in malloc_extend_top.
*/

#وصف_ڪريو initial_چوٽي    ((mchunkptr)(bin_at(0)))

/* Helper macro to initialize bins */

#وصف_ڪريو IAV(i)  bin_at(i), bin_at(i)

ساكن mbinptr av_[NAV * 2 + 2] = {
 عدم, عدم,
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

#جيڪڏهن_وصف_ٿيل CONFIG_SYS_MALLOC_DEFAULT_منزل_INIT
ساكن فراغ malloc_init(فراغ);
#جيڪڏهن_پورو

ulong mem_malloc_شروعات = 0;
ulong mem_malloc_پڇاڙي = 0;
ulong mem_malloc_brk = 0;

ساكن bool malloc_testing;	/* enable test mode */
ساكن صحيح malloc_max_allocs;	/* return NULL after this many calls to malloc() */

فراغ *sbrk(ptrdiff_t increment)
{
	ulong old = mem_malloc_brk;
	ulong new = old + increment;

	إذا ((new < mem_malloc_شروعات) || (new > mem_malloc_پڇاڙي))
		أرجع (فراغ *)MORECORE_FAILURE;

	/*
	 * if we are giving memory back make sure we clear it out since
	 * we set MORECORE_CLEARS to 1
	 */
	إذا (increment < 0)
		memset((فراغ *)new, 0, -increment);

	mem_malloc_brk = new;

	أرجع (فراغ *)old;
}

فراغ mem_malloc_init(ulong شروعات, ulong سائيز)
{
	mem_malloc_شروعات = (ulong)map_sysmem(شروعات, سائيز);
	mem_malloc_پڇاڙي = mem_malloc_شروعات + سائيز;
	mem_malloc_brk = mem_malloc_شروعات;

#جيڪڏهن_وصف_ٿيل CONFIG_SYS_MALLOC_DEFAULT_منزل_INIT
	malloc_init();
#جيڪڏهن_پورو

	debug("using memory %#lx-%#lx for malloc()\n", mem_malloc_شروعات,
	      mem_malloc_پڇاڙي);
#إذا CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
	memset((فراغ *)mem_malloc_شروعات, 0x0, سائيز);
#جيڪڏهن_پورو
}

/* field-extraction macros */

#وصف_ڪريو پهريون(b) ((b)->fd)
#وصف_ڪريو آخري(b)  ((b)->bk)

/*
  Indexing into bins
*/

#وصف_ڪريو bin_اشاريو(sz)                                                          \
(((((بنانشان ڊگهو)(sz)) >> 9) ==    0) ?       (((بنانشان ڊگهو)(sz)) >>  3): \
 ((((بنانشان ڊگهو)(sz)) >> 9) <=    4) ?  56 + (((بنانشان ڊگهو)(sz)) >>  6): \
 ((((بنانشان ڊگهو)(sz)) >> 9) <=   20) ?  91 + (((بنانشان ڊگهو)(sz)) >>  9): \
 ((((بنانشان ڊگهو)(sz)) >> 9) <=   84) ? 110 + (((بنانشان ڊگهو)(sz)) >> 12): \
 ((((بنانشان ڊگهو)(sz)) >> 9) <=  340) ? 119 + (((بنانشان ڊگهو)(sz)) >> 15): \
 ((((بنانشان ڊگهو)(sz)) >> 9) <= 1364) ? 124 + (((بنانشان ڊگهو)(sz)) >> 18): \
					  126)
/*
  bins for chunks < 512 are all spaced 8 bytes apart, and hold
  identically sized chunks. This is exploited in malloc.
*/

#وصف_ڪريو MAX_SMALLBIN         63
#وصف_ڪريو MAX_SMALLBIN_سائيز   512
#وصف_ڪريو SMALLBIN_ويڪر        8

#وصف_ڪريو smallbin_اشاريو(sz)  (((بنانشان ڊگهو)(sz)) >> 3)

/*
   Requests are `small' if both the corresponding and the next bin are small
*/

#وصف_ڪريو is_small_request(nb) (nb < MAX_SMALLBIN_سائيز - SMALLBIN_ويڪر)

/*
    To help compensate for the large number of bins, a one-level index
    structure is used for bin-by-bin searching.  `binblocks' is a
    one-word bitvector recording whether groups of BINBLOCKWIDTH bins
    have any (possibly) non-empty bins, so they can be skipped over
    all at once during during traversals. The bits are NOT always
    cleared as soon as all bins in a block are empty, but instead only
    when all are noticed to be empty during traversal in malloc.
*/

#وصف_ڪريو BINBLOCKWIDTH     4   /* bins per block */

#وصف_ڪريو binblocks_r     ((اندروني_سائيز_T)av_[1]) /* bitvector of nonempty blocks */
#وصف_ڪريو binblocks_w     (av_[1])

/* bin<->block macros */

#وصف_ڪريو idx2binblock(ix)    ((بنانشان)1 << (ix / BINBLOCKWIDTH))
#وصف_ڪريو mark_binblock(ii)   (binblocks_w = (mbinptr)(binblocks_r | idx2binblock(ii)))
#وصف_ڪريو clear_binblock(ii)  (binblocks_w = (mbinptr)(binblocks_r & ~(idx2binblock(ii))))

/*  Other static bookkeeping data */

/* variables holding tunable values */

ساكن بنانشان ڊگهو trim_حد   = DEFAULT_TRIM_حد;
ساكن بنانشان ڊگهو چوٽي_pad          = DEFAULT_چوٽي_PAD;
ساكن بنانشان صحيح  n_mmaps_max      = DEFAULT_MMAP_MAX;
ساكن بنانشان ڊگهو mmap_حد   = DEFAULT_MMAP_حد;

/* The first value returned from sbrk */
ساكن حرف* sbrk_بنياد = (حرف*)(-1);

/* The maximum memory obtained from system via sbrk */
ساكن بنانشان ڊگهو max_sbrked_mem = 0;

/* The maximum via either sbrk or mmap */
ساكن بنانشان ڊگهو max_ڪل_mem = 0;

/* internal working copy of mallinfo */
ساكن بنية mallinfo current_mallinfo = {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* The total memory obtained from system via sbrk */
#وصف_ڪريو sbrked_mem  (current_mallinfo.arena)

/* Tracking mmaps */

#جيڪڏهن_وصف_ٿيل DEBUG
ساكن بنانشان صحيح n_mmaps = 0;
#جيڪڏهن_پورو	/* DEBUG */
ساكن بنانشان ڊگهو mmapped_mem = 0;
#إذا HAVE_MMAP
ساكن بنانشان صحيح max_n_mmaps = 0;
ساكن بنانشان ڊگهو max_mmapped_mem = 0;
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل CONFIG_SYS_MALLOC_DEFAULT_منزل_INIT
ساكن فراغ malloc_init(فراغ)
{
	صحيح i, j;

	debug("bins (av_ array) are at %p\n", (فراغ *)av_);

	av_[0] = عدم; av_[1] = عدم;
	لكل (i = 2, j = 2; i < NAV * 2 + 2; i += 2, j++) {
		av_[i] = bin_at(j - 2);
		av_[i + 1] = bin_at(j - 2);

		/* Just print the first few bins so that
		 * we can see there are alright.
		 */
		إذا (i < 10)
			debug("av_[%d]=%lx av_[%d]=%lx\n",
			      i, (ulong)av_[i],
			      i + 1, (ulong)av_[i + 1]);
	}

	/* Init the static bookkeeping as well */
	sbrk_بنياد = (حرف *)(-1);
	max_sbrked_mem = 0;
	max_ڪل_mem = 0;
#جيڪڏهن_وصف_ٿيل DEBUG
	memset((فراغ *)&current_mallinfo, 0, حجم(بنية mallinfo));
#جيڪڏهن_پورو
}
#جيڪڏهن_پورو

/*
  Debugging support
*/

#جيڪڏهن_وصف_ٿيل DEBUG

/*
  These routines make a number of assertions about the states
  of data structures that should be true at all times. If any
  are not true, it's very likely that a user program has somehow
  trashed memory. (It's also possible that there is a coding error
  in malloc. In which case, please report it!)
*/

#إذا __STD_C
ساكن فراغ do_check_chunk(mchunkptr p)
#وإلا
ساكن فراغ do_check_chunk(p) mchunkptr p;
#جيڪڏهن_پورو
{
  اندروني_سائيز_T sz = p->سائيز & ~PREV_INUSE;

  /* No checkable chunk is mmapped */
  أكد(!chunk_is_mmapped(p));

  /* Check for legal address ... */
  أكد((حرف*)p >= sbrk_بنياد);
  إذا (p != چوٽي)
    أكد((حرف*)p + sz <= (حرف*)چوٽي);
  وإلا
    أكد((حرف*)p + sz <= sbrk_بنياد + sbrked_mem);

}

#إذا __STD_C
ساكن فراغ do_check_free_chunk(mchunkptr p)
#وإلا
ساكن فراغ do_check_free_chunk(p) mchunkptr p;
#جيڪڏهن_پورو
{
  اندروني_سائيز_T sz = p->سائيز & ~PREV_INUSE;
  mchunkptr ايندڙ = chunk_at_وٿي(p, sz);

  do_check_chunk(p);

  /* Check whether it claims to be free ... */
  أكد(!inuse(p));

  /* Unless a special marker, must have OK fields */
  إذا ((ڊگهو)sz >= (ڊگهو)MINSIZE)
  {
    أكد((sz & MALLOC_ALIGN_نقاب) == 0);
    أكد(aligned_OK(chunk2mem(p)));
    /* ... matching footer field */
    أكد(ايندڙ->prev_سائيز == sz);
    /* ... and is fully consolidated */
    أكد(prev_inuse(p));
    أكد (ايندڙ == چوٽي || inuse(ايندڙ));

    /* ... and has minimally sane links */
    أكد(p->fd->bk == p);
    أكد(p->bk->fd == p);
  }
  وإلا /* markers are always of size SIZE_SZ */
    أكد(sz == سائيز_SZ);
}

#إذا __STD_C
ساكن فراغ do_check_inuse_chunk(mchunkptr p)
#وإلا
ساكن فراغ do_check_inuse_chunk(p) mchunkptr p;
#جيڪڏهن_پورو
{
  mchunkptr ايندڙ = ايندڙ_chunk(p);
  do_check_chunk(p);

  /* Check whether it claims to be in use ... */
  أكد(inuse(p));

  /* ... and is surrounded by OK chunks.
    Since more things can be checked with free chunks than inuse ones,
    if an inuse chunk borders them and debug is on, it's worth doing them.
  */
  إذا (!prev_inuse(p))
  {
    mchunkptr prv = prev_chunk(p);
    أكد(ايندڙ_chunk(prv) == p);
    do_check_free_chunk(prv);
  }
  إذا (ايندڙ == چوٽي)
  {
    أكد(prev_inuse(ايندڙ));
    أكد(chunksize(ايندڙ) >= MINSIZE);
  }
  وإلا إذا (!inuse(ايندڙ))
    do_check_free_chunk(ايندڙ);

}

#إذا __STD_C
ساكن فراغ do_check_malloced_chunk(mchunkptr p, اندروني_سائيز_T s)
#وإلا
ساكن فراغ do_check_malloced_chunk(p, s) mchunkptr p; اندروني_سائيز_T s;
#جيڪڏهن_پورو
{
  اندروني_سائيز_T sz = p->سائيز & ~PREV_INUSE;
  ڊگهو گنجائش = sz - s;

  do_check_inuse_chunk(p);

  /* Legal size ... */
  أكد((ڊگهو)sz >= (ڊگهو)MINSIZE);
  أكد((sz & MALLOC_ALIGN_نقاب) == 0);
  أكد(گنجائش >= 0);
  أكد(گنجائش < (ڊگهو)MINSIZE);

  /* ... and alignment */
  أكد(aligned_OK(chunk2mem(p)));

  /* ... and was allocated at front of an available chunk */
  أكد(prev_inuse(p));

}

#وصف_ڪريو check_free_chunk(P)  do_check_free_chunk(P)
#وصف_ڪريو check_inuse_chunk(P) do_check_inuse_chunk(P)
#وصف_ڪريو check_chunk(P) do_check_chunk(P)
#وصف_ڪريو check_malloced_chunk(P,N) do_check_malloced_chunk(P,N)
#وإلا
#وصف_ڪريو check_free_chunk(P)
#وصف_ڪريو check_inuse_chunk(P)
#وصف_ڪريو check_chunk(P)
#وصف_ڪريو check_malloced_chunk(P,N)
#جيڪڏهن_پورو

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

#وصف_ڪريو frontlink(P, S, IDX, BK, FD)                                          \
{                                                                             \
  إذا (S < MAX_SMALLBIN_سائيز)                                                  \
  {                                                                           \
    IDX = smallbin_اشاريو(S);                                                  \
    mark_binblock(IDX);                                                       \
    BK = bin_at(IDX);                                                         \
    FD = BK->fd;                                                              \
    P->bk = BK;                                                               \
    P->fd = FD;                                                               \
    FD->bk = BK->fd = P;                                                      \
  }                                                                           \
  وإلا                                                                        \
  {                                                                           \
    IDX = bin_اشاريو(S);                                                       \
    BK = bin_at(IDX);                                                         \
    FD = BK->fd;                                                              \
    إذا (FD == BK) mark_binblock(IDX);                                         \
    وإلا                                                                      \
    {                                                                         \
      طالما (FD != BK && S < chunksize(FD)) FD = FD->fd;                      \
      BK = FD->bk;                                                            \
    }                                                                         \
    P->bk = BK;                                                               \
    P->fd = FD;                                                               \
    FD->bk = BK->fd = P;                                                      \
  }                                                                           \
}

/* take a chunk off a list */

#وصف_ڪريو unlink(P, BK, FD)                                                     \
{                                                                             \
  BK = P->bk;                                                                 \
  FD = P->fd;                                                                 \
  FD->bk = BK;                                                                \
  BK->fd = FD;                                                                \
}                                                                             \

/* Place p as the last remainder */

#وصف_ڪريو link_آخري_remainder(P)                                                \
{                                                                             \
  آخري_remainder->fd = آخري_remainder->bk =  P;                               \
  P->fd = P->bk = آخري_remainder;                                             \
}

/* Clear the last_remainder bin */

#وصف_ڪريو clear_آخري_remainder \
  (آخري_remainder->fd = آخري_remainder->bk = آخري_remainder)

/* Routines dealing with mmap(). */

#إذا HAVE_MMAP

#إذا __STD_C
ساكن mchunkptr mmap_chunk(size_t سائيز)
#وإلا
ساكن mchunkptr mmap_chunk(سائيز) size_t سائيز;
#جيڪڏهن_پورو
{
  size_t صفحو_نقاب = malloc_getpagesize - 1;
  mchunkptr p;

#جيڪڏهن_وصف_نه_ٿيل MAP_ANONYMOUS
  ساكن صحيح fd = -1;
#جيڪڏهن_پورو

  إذا(n_mmaps >= n_mmaps_max) أرجع 0; /* too many regions */

  /* For mmapped chunks, the overhead is one SIZE_SZ unit larger, because
   * there is no following chunk whose prev_size field could be used.
   */
  سائيز = (سائيز + سائيز_SZ + صفحو_نقاب) & ~صفحو_نقاب;

#جيڪڏهن_وصف_ٿيل MAP_ANONYMOUS
  p = (mchunkptr)mmap(0, سائيز, PROT_READ|PROT_WRITE,
		      MAP_ذاتي|MAP_ANONYMOUS, -1, 0);
#وإلا /* !MAP_ANONYMOUS */
  إذا (fd < 0)
  {
    fd = open("/dev/zero", O_RDWR);
    إذا(fd < 0) أرجع 0;
  }
  p = (mchunkptr)mmap(0, سائيز, PROT_READ|PROT_WRITE, MAP_ذاتي, fd, 0);
#جيڪڏهن_پورو

  إذا(p == (mchunkptr)-1) أرجع 0;

  n_mmaps++;
  إذا (n_mmaps > max_n_mmaps) max_n_mmaps = n_mmaps;

  /* We demand that eight bytes into a page must be 8-byte aligned. */
  أكد(aligned_OK(chunk2mem(p)));

  /* The offset to the start of the mmapped region is stored
   * in the prev_size field of the chunk; normally it is zero,
   * but that can be changed in memalign().
   */
  p->prev_سائيز = 0;
  سيٽ_سر(p, سائيز|IS_MMAPPED);

  mmapped_mem += سائيز;
  إذا ((بنانشان ڊگهو)mmapped_mem > (بنانشان ڊگهو)max_mmapped_mem)
    max_mmapped_mem = mmapped_mem;
  إذا ((بنانشان ڊگهو)(mmapped_mem + sbrked_mem) > (بنانشان ڊگهو)max_ڪل_mem)
    max_ڪل_mem = mmapped_mem + sbrked_mem;
  أرجع p;
}

#إذا __STD_C
ساكن فراغ munmap_chunk(mchunkptr p)
#وإلا
ساكن فراغ munmap_chunk(p) mchunkptr p;
#جيڪڏهن_پورو
{
  اندروني_سائيز_T سائيز = chunksize(p);
  صحيح ret;

  أكد (chunk_is_mmapped(p));
  أكد(! ((حرف*)p >= sbrk_بنياد && (حرف*)p < sbrk_بنياد + sbrked_mem));
  أكد((n_mmaps > 0));
  أكد(((p->prev_سائيز + سائيز) & (malloc_getpagesize-1)) == 0);

  n_mmaps--;
  mmapped_mem -= (سائيز + p->prev_سائيز);

  ret = munmap((حرف *)p - p->prev_سائيز, سائيز + p->prev_سائيز);

  /* munmap returns non-zero on failure */
  أكد(ret == 0);
}

#إذا HAVE_MREMAP

#إذا __STD_C
ساكن mchunkptr mremap_chunk(mchunkptr p, size_t new_سائيز)
#وإلا
ساكن mchunkptr mremap_chunk(p, new_سائيز) mchunkptr p; size_t new_سائيز;
#جيڪڏهن_پورو
{
  size_t صفحو_نقاب = malloc_getpagesize - 1;
  اندروني_سائيز_T وٿي = p->prev_سائيز;
  اندروني_سائيز_T سائيز = chunksize(p);
  حرف *cp;

  أكد (chunk_is_mmapped(p));
  أكد(! ((حرف*)p >= sbrk_بنياد && (حرف*)p < sbrk_بنياد + sbrked_mem));
  أكد((n_mmaps > 0));
  أكد(((سائيز + وٿي) & (malloc_getpagesize-1)) == 0);

  /* Note the extra SIZE_SZ overhead as in mmap_chunk(). */
  new_سائيز = (new_سائيز + وٿي + سائيز_SZ + صفحو_نقاب) & ~صفحو_نقاب;

  cp = (حرف *)mremap((حرف *)p - وٿي, سائيز + وٿي, new_سائيز, 1);

  إذا (cp == (حرف *)-1) أرجع 0;

  p = (mchunkptr)(cp + وٿي);

  أكد(aligned_OK(chunk2mem(p)));

  أكد((p->prev_سائيز == وٿي));
  سيٽ_سر(p, (new_سائيز - وٿي)|IS_MMAPPED);

  mmapped_mem -= سائيز + وٿي;
  mmapped_mem += new_سائيز;
  إذا ((بنانشان ڊگهو)mmapped_mem > (بنانشان ڊگهو)max_mmapped_mem)
    max_mmapped_mem = mmapped_mem;
  إذا ((بنانشان ڊگهو)(mmapped_mem + sbrked_mem) > (بنانشان ڊگهو)max_ڪل_mem)
    max_ڪل_mem = mmapped_mem + sbrked_mem;
  أرجع p;
}

#جيڪڏهن_پورو /* HAVE_MREMAP */

#جيڪڏهن_پورو /* HAVE_MMAP */

/*
  Extend the top-most chunk by obtaining memory from system.
  Main interface to sbrk (but see also malloc_trim).
*/

#إذا __STD_C
ساكن فراغ malloc_extend_چوٽي(اندروني_سائيز_T nb)
#وإلا
ساكن فراغ malloc_extend_چوٽي(nb) اندروني_سائيز_T nb;
#جيڪڏهن_پورو
{
  حرف*     brk;                  /* return value from sbrk */
  اندروني_سائيز_T front_misalign; /* unusable bytes at front of sbrked space */
  اندروني_سائيز_T correction;     /* bytes for 2nd sbrk call */
  حرف*     new_brk;              /* return of 2nd sbrk call */
  اندروني_سائيز_T چوٽي_سائيز;       /* new size of top chunk */

  mchunkptr old_چوٽي     = چوٽي;  /* Record state of old top */
  اندروني_سائيز_T old_چوٽي_سائيز = chunksize(old_چوٽي);
  حرف*     old_پڇاڙي      = (حرف*)(chunk_at_وٿي(old_چوٽي, old_چوٽي_سائيز));

  /* Pad request with top_pad plus minimal overhead */

  اندروني_سائيز_T    sbrk_سائيز     = nb + چوٽي_pad + MINSIZE;
  بنانشان ڊگهو pagesz    = malloc_getpagesize;

  /* If not the first time through, round to preserve page boundary */
  /* Otherwise, we need to correct to a page size below anyway. */
  /* (We also correct below if an intervening foreign sbrk call.) */

  إذا (sbrk_بنياد != (حرف*)(-1))
    sbrk_سائيز = (sbrk_سائيز + (pagesz - 1)) & ~(pagesz - 1);

  brk = (حرف*)(MORECORE (sbrk_سائيز));

  /* Fail if sbrk failed or if a foreign sbrk call killed our space */
  إذا (brk == (حرف*)(MORECORE_FAILURE) ||
      (brk < old_پڇاڙي && old_چوٽي != initial_چوٽي))
    أرجع;

  sbrked_mem += sbrk_سائيز;

  إذا (brk == old_پڇاڙي) /* can just add bytes to current top */
  {
    چوٽي_سائيز = sbrk_سائيز + old_چوٽي_سائيز;
    سيٽ_سر(چوٽي, چوٽي_سائيز | PREV_INUSE);
  }
  وإلا
  {
    إذا (sbrk_بنياد == (حرف*)(-1))  /* First time through. Record base */
      sbrk_بنياد = brk;
    وإلا  /* Someone else called sbrk().  Count those bytes as sbrked_mem. */
      sbrked_mem += brk - (حرف*)old_پڇاڙي;

    /* Guarantee alignment of first new chunk made from this space */
    front_misalign = (بنانشان ڊگهو)chunk2mem(brk) & MALLOC_ALIGN_نقاب;
    إذا (front_misalign > 0)
    {
      correction = (MALLOC_ALIGNMENT) - front_misalign;
      brk += correction;
    }
    وإلا
      correction = 0;

    /* Guarantee the next brk will be at a page boundary */

    correction += ((((بنانشان ڊگهو)(brk + sbrk_سائيز))+(pagesz-1)) &
		   ~(pagesz - 1)) - ((بنانشان ڊگهو)(brk + sbrk_سائيز));

    /* Allocate correction */
    new_brk = (حرف*)(MORECORE (correction));
    إذا (new_brk == (حرف*)(MORECORE_FAILURE)) أرجع;

    sbrked_mem += correction;

    چوٽي = (mchunkptr)brk;
    چوٽي_سائيز = new_brk - brk + correction;
    سيٽ_سر(چوٽي, چوٽي_سائيز | PREV_INUSE);

    إذا (old_چوٽي != initial_چوٽي)
    {

      /* There must have been an intervening foreign sbrk call. */
      /* A double fencepost is necessary to prevent consolidation */

      /* If not enough space to do this, then user did something very wrong */
      إذا (old_چوٽي_سائيز < MINSIZE)
      {
	سيٽ_سر(چوٽي, PREV_INUSE); /* will force null return from malloc */
	أرجع;
      }

      /* Also keep size a multiple of MALLOC_ALIGNMENT */
      old_چوٽي_سائيز = (old_چوٽي_سائيز - 3*سائيز_SZ) & ~MALLOC_ALIGN_نقاب;
      سيٽ_سر_سائيز(old_چوٽي, old_چوٽي_سائيز);
      chunk_at_وٿي(old_چوٽي, old_چوٽي_سائيز          )->سائيز =
	سائيز_SZ|PREV_INUSE;
      chunk_at_وٿي(old_چوٽي, old_چوٽي_سائيز + سائيز_SZ)->سائيز =
	سائيز_SZ|PREV_INUSE;
      /* If possible, release the rest. */
      إذا (old_چوٽي_سائيز >= MINSIZE)
	fREe(chunk2mem(old_چوٽي));
    }
  }

  إذا ((بنانشان ڊگهو)sbrked_mem > (بنانشان ڊگهو)max_sbrked_mem)
    max_sbrked_mem = sbrked_mem;
  إذا ((بنانشان ڊگهو)(mmapped_mem + sbrked_mem) > (بنانشان ڊگهو)max_ڪل_mem)
    max_ڪل_mem = mmapped_mem + sbrked_mem;

  /* We always land on a page boundary */
  أكد(((بنانشان ڊگهو)((حرف*)چوٽي + چوٽي_سائيز) & (pagesz - 1)) == 0);
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
#إذا __STD_C
Void_t* mALLOc_impl(size_t بائيٽون)
#وإلا
Void_t* mALLOc_impl(بائيٽون) size_t بائيٽون;
#جيڪڏهن_پورو
{
  mchunkptr victim;                  /* inspected/selected chunk */
  اندروني_سائيز_T victim_سائيز;       /* its size */
  صحيح       idx;                     /* index for bin traversal */
  mbinptr   bin;                     /* associated bin */
  mchunkptr remainder;               /* remainder from a split */
  ڊگهو      remainder_سائيز;          /* its size */
  صحيح       remainder_اشاريو;         /* its bin index */
  بنانشان ڊگهو بلاڪ;               /* block traverser bit */
  صحيح       startidx;                /* first bin of a traversed block */
  mchunkptr fwd;                     /* misc temp for linking */
  mchunkptr bck;                     /* misc temp for linking */
  mbinptr q;                         /* misc temp */

  اندروني_سائيز_T nb;

#إذا CONFIG_IS_ENABLED(SYS_MALLOC_F)
	إذا (!(gd->flags & GD_FLG_ڀريل_MALLOC_INIT))
		أرجع malloc_simple(بائيٽون);
#جيڪڏهن_پورو

  إذا (CONFIG_IS_ENABLED(UNIT_TEST) && malloc_testing) {
    إذا (--malloc_max_allocs < 0)
      أرجع عدم;
  }

  /* check if mem_malloc_init() was run */
  إذا ((mem_malloc_شروعات == 0) && (mem_malloc_پڇاڙي == 0)) {
    /* not initialized yet */
    أرجع عدم;
  }

  إذا (بائيٽون > CONFIG_SYS_MALLOC_LEN || (ڊگهو)بائيٽون < 0)
     أرجع عدم;

  nb = request2size(بائيٽون);  /* padded request size; */

  /* Check for exact match in a bin */

  إذا (is_small_request(nb))  /* Faster version for small requests */
  {
    idx = smallbin_اشاريو(nb);

    /* No traversal or size check necessary for small bins.  */

    q = bin_at(idx);
    victim = آخري(q);

    /* Also scan the next one, since it would have a remainder < MINSIZE */
    إذا (victim == q)
    {
      q = ايندڙ_bin(q);
      victim = آخري(q);
    }
    إذا (victim != q)
    {
      victim_سائيز = chunksize(victim);
      unlink(victim, bck, fwd);
      سيٽ_inuse_ٻٽو_انگ_at_وٿي(victim, victim_سائيز);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_بلاڪ(chunk2mem(victim), بائيٽون, سائيز_SZ, false);
      أرجع chunk2mem(victim);
    }

    idx += 2; /* Set for bin scan below. We've already scanned 2 bins. */

  }
  وإلا
  {
    idx = bin_اشاريو(nb);
    bin = bin_at(idx);

    لكل (victim = آخري(bin); victim != bin; victim = victim->bk)
    {
      victim_سائيز = chunksize(victim);
      remainder_سائيز = victim_سائيز - nb;

      إذا (remainder_سائيز >= (ڊگهو)MINSIZE) /* too big */
      {
	--idx; /* adjust to rescan below after checking last remainder */
	اكسر;
      }

      وإلا إذا (remainder_سائيز >= 0) /* exact fit */
      {
	unlink(victim, bck, fwd);
	سيٽ_inuse_ٻٽو_انگ_at_وٿي(victim, victim_سائيز);
	check_malloced_chunk(victim, nb);
        VALGRIND_MALLOCLIKE_بلاڪ(chunk2mem(victim), بائيٽون, سائيز_SZ, false);
	أرجع chunk2mem(victim);
      }
    }

    ++idx;

  }

  /* Try to use the last split-off remainder */

  إذا ( (victim = آخري_remainder->fd) != آخري_remainder)
  {
    victim_سائيز = chunksize(victim);
    remainder_سائيز = victim_سائيز - nb;

    إذا (remainder_سائيز >= (ڊگهو)MINSIZE) /* re-split */
    {
      remainder = chunk_at_وٿي(victim, nb);
      سيٽ_سر(victim, nb | PREV_INUSE);
      link_آخري_remainder(remainder);
      سيٽ_سر(remainder, remainder_سائيز | PREV_INUSE);
      سيٽ_foot(remainder, remainder_سائيز);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_بلاڪ(chunk2mem(victim), بائيٽون, سائيز_SZ, false);
      أرجع chunk2mem(victim);
    }

    clear_آخري_remainder;

    إذا (remainder_سائيز >= 0)  /* exhaust */
    {
      سيٽ_inuse_ٻٽو_انگ_at_وٿي(victim, victim_سائيز);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_بلاڪ(chunk2mem(victim), بائيٽون, سائيز_SZ, false);
      أرجع chunk2mem(victim);
    }

    /* Else place in bin */

    frontlink(victim, victim_سائيز, remainder_اشاريو, bck, fwd);
  }

  /*
     If there are any possibly nonempty big-enough blocks,
     search for best fitting chunk by scanning bins in blockwidth units.
  */

  إذا ( (بلاڪ = idx2binblock(idx)) <= binblocks_r)
  {

    /* Get to the first marked block */

    إذا ( (بلاڪ & binblocks_r) == 0)
    {
      /* force to an even block boundary */
      idx = (idx & ~(BINBLOCKWIDTH - 1)) + BINBLOCKWIDTH;
      بلاڪ <<= 1;
      طالما ((بلاڪ & binblocks_r) == 0)
      {
	idx += BINBLOCKWIDTH;
	بلاڪ <<= 1;
      }
    }

    /* For each possibly nonempty block ... */
    لكل (;;)
    {
      startidx = idx;          /* (track incomplete blocks) */
      q = bin = bin_at(idx);

      /* For each bin in this block ... */
      افعل
      {
	/* Find and use first big enough chunk ... */

	لكل (victim = آخري(bin); victim != bin; victim = victim->bk)
	{
	  victim_سائيز = chunksize(victim);
	  remainder_سائيز = victim_سائيز - nb;

	  إذا (remainder_سائيز >= (ڊگهو)MINSIZE) /*ورهايو*/
	  {
	    remainder = chunk_at_وٿي(victim, nb);
	    سيٽ_سر(victim, nb | PREV_INUSE);
	    unlink(victim, bck, fwd);
	    link_آخري_remainder(remainder);
	    سيٽ_سر(remainder, remainder_سائيز | PREV_INUSE);
	    سيٽ_foot(remainder, remainder_سائيز);
	    check_malloced_chunk(victim, nb);
	    VALGRIND_MALLOCLIKE_بلاڪ(chunk2mem(victim), بائيٽون, سائيز_SZ, false);
	    أرجع chunk2mem(victim);
	  }

	  وإلا إذا (remainder_سائيز >= 0)  /* take */
	  {
	    سيٽ_inuse_ٻٽو_انگ_at_وٿي(victim, victim_سائيز);
	    unlink(victim, bck, fwd);
	    check_malloced_chunk(victim, nb);
	    VALGRIND_MALLOCLIKE_بلاڪ(chunk2mem(victim), بائيٽون, سائيز_SZ, false);
	    أرجع chunk2mem(victim);
	  }

	}

       bin = ايندڙ_bin(bin);

      } طالما ((++idx & (BINBLOCKWIDTH - 1)) != 0);

      /* Clear out the block bit. */

      افعل   /* Possibly backtrack to try to clear a partial block */
      {
	إذا ((startidx & (BINBLOCKWIDTH - 1)) == 0)
	{
	  av_[1] = (mbinptr)(binblocks_r & ~بلاڪ);
	  اكسر;
	}
	--startidx;
       q = prev_bin(q);
      } طالما (پهريون(q) == q);

      /* Get to the next possibly nonempty block */

      إذا ( (بلاڪ <<= 1) <= binblocks_r && (بلاڪ != 0) )
      {
	طالما ((بلاڪ & binblocks_r) == 0)
	{
	  idx += BINBLOCKWIDTH;
	  بلاڪ <<= 1;
	}
      }
      وإلا
	اكسر;
    }
  }

  /* Try to use top chunk */

  /* Require that there be a remainder, ensuring top always exists  */
  إذا ( (remainder_سائيز = chunksize(چوٽي) - nb) < (ڊگهو)MINSIZE)
  {

#إذا HAVE_MMAP
    /* If big and would otherwise need to extend, try to use mmap instead */
    إذا ((بنانشان ڊگهو)nb >= (بنانشان ڊگهو)mmap_حد &&
	(victim = mmap_chunk(nb)))
      VALGRIND_MALLOCLIKE_بلاڪ(chunk2mem(victim), بائيٽون, سائيز_SZ, false);
      أرجع chunk2mem(victim);
#جيڪڏهن_پورو

    /* Try to extend */
    malloc_extend_چوٽي(nb);
    إذا ( (remainder_سائيز = chunksize(چوٽي) - nb) < (ڊگهو)MINSIZE)
      أرجع عدم; /* propagate failure */
  }

  victim = چوٽي;
  سيٽ_سر(victim, nb | PREV_INUSE);
  چوٽي = chunk_at_وٿي(victim, nb);
  سيٽ_سر(چوٽي, remainder_سائيز | PREV_INUSE);
  check_malloced_chunk(victim, nb);
  VALGRIND_MALLOCLIKE_بلاڪ(chunk2mem(victim), بائيٽون, سائيز_SZ, false);
  أرجع chunk2mem(victim);

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
#إذا __STD_C
فراغ fREe_impl(Void_t* mem)
#وإلا
فراغ fREe_impl(mem) Void_t* mem;
#جيڪڏهن_پورو
{
  mchunkptr p;         /* chunk corresponding to mem */
  اندروني_سائيز_T hd;  /* its head field */
  اندروني_سائيز_T sz;  /* its size */
  صحيح       idx;       /* its bin index */
  mchunkptr ايندڙ;      /* next contiguous chunk */
  اندروني_سائيز_T nextsz; /* its size */
  اندروني_سائيز_T prevsz; /* size of previous contiguous chunk */
  mchunkptr bck;       /* misc temp for linking */
  mchunkptr fwd;       /* misc temp for linking */
  صحيح       islr;      /* track whether merging with last_remainder */

#إذا CONFIG_IS_ENABLED(SYS_MALLOC_F)
	/* free() is a no-op - all the memory will be freed on relocation */
	إذا (!(gd->flags & GD_FLG_ڀريل_MALLOC_INIT)) {
		VALGRIND_FREELIKE_بلاڪ(mem, سائيز_SZ);
		أرجع;
	}
#جيڪڏهن_پورو

  إذا (mem == عدم)                              /* free(0) has no effect */
    أرجع;

  p = mem2chunk(mem);
  hd = p->سائيز;

#إذا HAVE_MMAP
  إذا (hd & IS_MMAPPED)                       /* release mmapped memory. */
  {
    munmap_chunk(p);
    أرجع;
  }
#جيڪڏهن_پورو

  check_inuse_chunk(p);

  sz = hd & ~PREV_INUSE;
  ايندڙ = chunk_at_وٿي(p, sz);
  nextsz = chunksize(ايندڙ);
  VALGRIND_FREELIKE_بلاڪ(mem, سائيز_SZ);

  إذا (ايندڙ == چوٽي)                            /* merge with top */
  {
    sz += nextsz;

    إذا (!(hd & PREV_INUSE))                    /* consolidate backward */
    {
      prevsz = p->prev_سائيز;
      p = chunk_at_وٿي(p, -((ڊگهو) prevsz));
      sz += prevsz;
      unlink(p, bck, fwd);
    }

    سيٽ_سر(p, sz | PREV_INUSE);
    چوٽي = p;
    إذا ((بنانشان ڊگهو)(sz) >= (بنانشان ڊگهو)trim_حد)
      malloc_trim(چوٽي_pad);
    أرجع;
  }

  سيٽ_سر(ايندڙ, nextsz);                    /* clear inuse bit */

  islr = 0;

  إذا (!(hd & PREV_INUSE))                    /* consolidate backward */
  {
    prevsz = p->prev_سائيز;
    p = chunk_at_وٿي(p, -((ڊگهو) prevsz));
    sz += prevsz;

    إذا (p->fd == آخري_remainder)             /* keep as last_remainder */
      islr = 1;
    وإلا
      unlink(p, bck, fwd);
  }

  إذا (!(inuse_ٻٽو_انگ_at_وٿي(ايندڙ, nextsz)))   /* consolidate forward */
  {
    sz += nextsz;

    إذا (!islr && ايندڙ->fd == آخري_remainder)  /* re-insert last_remainder */
    {
      islr = 1;
      link_آخري_remainder(p);
    }
    وإلا
      unlink(ايندڙ, bck, fwd);
  }

  سيٽ_سر(p, sz | PREV_INUSE);
  سيٽ_foot(p, sz);
  إذا (!islr)
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
#إذا __STD_C
Void_t* rEALLOc_impl(Void_t* oldmem, size_t بائيٽون)
#وإلا
Void_t* rEALLOc_impl(oldmem, بائيٽون) Void_t* oldmem; size_t بائيٽون;
#جيڪڏهن_پورو
{
  اندروني_سائيز_T    nb;      /* padded request size */

  mchunkptr oldp;             /* chunk corresponding to oldmem */
  اندروني_سائيز_T    oldsize; /* its size */

  mchunkptr newp;             /* chunk to return */
  اندروني_سائيز_T    newsize; /* its size */
  Void_t*   newmem;           /* corresponding user mem */

  mchunkptr ايندڙ;             /* next contiguous chunk after oldp */
  اندروني_سائيز_T  nextsize;  /* its size */

  mchunkptr prev;             /* previous contiguous chunk before oldp */
  اندروني_سائيز_T  prevsize;  /* its size */

  mchunkptr remainder;        /* holds split off extra space from newp */
  اندروني_سائيز_T  remainder_سائيز;   /* its size */

  mchunkptr bck;              /* misc temp for linking */
  mchunkptr fwd;              /* misc temp for linking */

#جيڪڏهن_وصف_ٿيل REALLOC_ZERO_بائيٽون_FREES
  إذا (!بائيٽون) {
	fREe_impl(oldmem);
	أرجع عدم;
  }
#جيڪڏهن_پورو

  إذا (بائيٽون > CONFIG_SYS_MALLOC_LEN || (ڊگهو)بائيٽون < 0)
     أرجع عدم;

  /* realloc of null is supposed to be same as malloc */
  إذا (oldmem == عدم) أرجع mALLOc_impl(بائيٽون);

#إذا CONFIG_IS_ENABLED(SYS_MALLOC_F)
	إذا (!(gd->flags & GD_FLG_ڀريل_MALLOC_INIT)) {
		/* This is harder to support and should not be needed */
		panic("pre-reloc realloc() is not supported");
	}
#جيڪڏهن_پورو
  إذا (CONFIG_IS_ENABLED(UNIT_TEST) && malloc_testing) {
    إذا (--malloc_max_allocs < 0)
      أرجع عدم;
  }

  newp    = oldp    = mem2chunk(oldmem);
  newsize = oldsize = chunksize(oldp);

  nb = request2size(بائيٽون);

#إذا HAVE_MMAP
  إذا (chunk_is_mmapped(oldp))
  {
#إذا HAVE_MREMAP
    newp = mremap_chunk(oldp, nb);
    إذا(newp) أرجع chunk2mem(newp);
#جيڪڏهن_پورو
    /* Note the extra SIZE_SZ overhead. */
    إذا(oldsize - سائيز_SZ >= nb) أرجع oldmem; /* do nothing */
    /* Must alloc, copy, free. */
    newmem = mALLOc_impl(بائيٽون);
    إذا (!newmem)
	أرجع عدم; /* propagate failure */
    MALLOC_COPY(newmem, oldmem, oldsize - 2*سائيز_SZ);
    munmap_chunk(oldp);
    أرجع newmem;
  }
#جيڪڏهن_پورو

  check_inuse_chunk(oldp);

  إذا ((ڊگهو)(oldsize) < (ڊگهو)(nb))
  {

    /* Try expanding forward */

    ايندڙ = chunk_at_وٿي(oldp, oldsize);
    إذا (ايندڙ == چوٽي || !inuse(ايندڙ))
    {
      nextsize = chunksize(ايندڙ);

      /* Forward into top only if a remainder */
      إذا (ايندڙ == چوٽي)
      {
	إذا ((ڊگهو)(nextsize + newsize) >= (ڊگهو)(nb + MINSIZE))
	{
	  newsize += nextsize;
	  چوٽي = chunk_at_وٿي(oldp, nb);
	  سيٽ_سر(چوٽي, (newsize - nb) | PREV_INUSE);
	  سيٽ_سر_سائيز(oldp, nb);
	  VALGRIND_RESIZEINPLACE_بلاڪ(chunk2mem(oldp), 0, بائيٽون, سائيز_SZ);
	  VALGRIND_MAKE_MEM_DEFINED(chunk2mem(oldp), بائيٽون);
	  أرجع chunk2mem(oldp);
	}
      }

      /* Forward into next chunk */
      وإلا إذا (((ڊگهو)(nextsize + newsize) >= (ڊگهو)(nb)))
      {
	unlink(ايندڙ, bck, fwd);
	newsize  += nextsize;
	VALGRIND_RESIZEINPLACE_بلاڪ(chunk2mem(oldp), 0, بائيٽون, سائيز_SZ);
	VALGRIND_MAKE_MEM_DEFINED(chunk2mem(oldp), بائيٽون);
	اذهب ورهايو;
      }
    }
    وإلا
    {
      ايندڙ = عدم;
      nextsize = 0;
    }

    /* Try shifting backwards. */

    إذا (!prev_inuse(oldp))
    {
      prev = prev_chunk(oldp);
      prevsize = chunksize(prev);

      /* try forward + backward first to save a later consolidation */

      إذا (ايندڙ != عدم)
      {
	/* into top */
	إذا (ايندڙ == چوٽي)
	{
	  إذا ((ڊگهو)(nextsize + prevsize + newsize) >= (ڊگهو)(nb + MINSIZE))
	  {
	    unlink(prev, bck, fwd);
	    newp = prev;
	    newsize += prevsize + nextsize;
	    newmem = chunk2mem(newp);
	    VALGRIND_MALLOCLIKE_بلاڪ(newmem, بائيٽون, سائيز_SZ, false);
	    MALLOC_COPY(newmem, oldmem, oldsize - سائيز_SZ);
	    چوٽي = chunk_at_وٿي(newp, nb);
	    سيٽ_سر(چوٽي, (newsize - nb) | PREV_INUSE);
	    سيٽ_سر_سائيز(newp, nb);
	    VALGRIND_FREELIKE_بلاڪ(oldmem, سائيز_SZ);
	    أرجع newmem;
	  }
	}

	/* into next chunk */
	وإلا إذا (((ڊگهو)(nextsize + prevsize + newsize) >= (ڊگهو)(nb)))
	{
	  unlink(ايندڙ, bck, fwd);
	  unlink(prev, bck, fwd);
	  newp = prev;
	  newsize += nextsize + prevsize;
	  newmem = chunk2mem(newp);
	  VALGRIND_MALLOCLIKE_بلاڪ(newmem, بائيٽون, سائيز_SZ, false);
	  MALLOC_COPY(newmem, oldmem, oldsize - سائيز_SZ);
	  اذهب ورهايو;
	}
      }

      /* backward only */
      إذا (prev != عدم && (ڊگهو)(prevsize + newsize) >= (ڊگهو)nb)
      {
	unlink(prev, bck, fwd);
	newp = prev;
	newsize += prevsize;
	newmem = chunk2mem(newp);
	VALGRIND_MALLOCLIKE_بلاڪ(newmem, بائيٽون, سائيز_SZ, false);
	MALLOC_COPY(newmem, oldmem, oldsize - سائيز_SZ);
	اذهب ورهايو;
      }
    }

    /* Must allocate */

    newmem = mALLOc_impl (بائيٽون);

    إذا (newmem == عدم)  /* propagate failure */
      أرجع عدم;

    /* Avoid copy if newp is next chunk after oldp. */
    /* (This can only happen when new chunk is sbrk'ed.) */

    إذا ( (newp = mem2chunk(newmem)) == ايندڙ_chunk(oldp))
    {
      newsize += chunksize(newp);
      newp = oldp;
      اذهب ورهايو;
    }

    /* Otherwise copy, free, and exit */
    MALLOC_COPY(newmem, oldmem, oldsize - سائيز_SZ);
    fREe_impl(oldmem);
    أرجع newmem;
  } وإلا {
    VALGRIND_RESIZEINPLACE_بلاڪ(oldmem, 0, بائيٽون, سائيز_SZ);
    VALGRIND_MAKE_MEM_DEFINED(oldmem, بائيٽون);
  }

 ورهايو:  /* split off extra room in old or expanded chunk */

  إذا (newsize - nb >= MINSIZE) /* split off remainder */
  {
    remainder = chunk_at_وٿي(newp, nb);
    remainder_سائيز = newsize - nb;
    سيٽ_سر_سائيز(newp, nb);
    سيٽ_سر(remainder, remainder_سائيز | PREV_INUSE);
    سيٽ_inuse_ٻٽو_انگ_at_وٿي(remainder, remainder_سائيز);
    VALGRIND_MALLOCLIKE_بلاڪ(chunk2mem(remainder), remainder_سائيز, سائيز_SZ,
			      false);
    fREe_impl(chunk2mem(remainder)); /* let free() deal with it */
  }
  وإلا
  {
    سيٽ_سر_سائيز(newp, newsize);
    سيٽ_inuse_ٻٽو_انگ_at_وٿي(newp, newsize);
  }

  check_inuse_chunk(newp);
  أرجع chunk2mem(newp);
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
#إذا __STD_C
Void_t* mEMALIGn_impl(size_t alignment, size_t بائيٽون)
#وإلا
Void_t* mEMALIGn_impl(alignment, بائيٽون) size_t alignment; size_t بائيٽون;
#جيڪڏهن_پورو
{
  اندروني_سائيز_T    nb;      /* padded  request size */
  حرف*     m;                /* memory returned by malloc call */
  mchunkptr p;                /* corresponding chunk */
  حرف*     brk;              /* alignment point within p */
  mchunkptr newp;             /* chunk to return */
  اندروني_سائيز_T  newsize;   /* its size */
  اندروني_سائيز_T  leadsize;  /* leading space befor alignment point */
  mchunkptr remainder;        /* spare room at end to split off */
  ڊگهو      remainder_سائيز;   /* its size */

  إذا (بائيٽون > CONFIG_SYS_MALLOC_LEN || (ڊگهو)بائيٽون < 0)
     أرجع عدم;

#إذا CONFIG_IS_ENABLED(SYS_MALLOC_F)
	إذا (!(gd->flags & GD_FLG_ڀريل_MALLOC_INIT)) {
		أرجع memalign_simple(alignment, بائيٽون);
	}
#جيڪڏهن_پورو

  /* If need less alignment than we give anyway, just relay to malloc */

  إذا (alignment <= MALLOC_ALIGNMENT) أرجع mALLOc_impl(بائيٽون);

  /* Otherwise, ensure that it is at least a minimum chunk size */

  إذا (alignment <  MINSIZE) alignment = MINSIZE;

  /* Call malloc with worst case padding to hit alignment. */

  nb = request2size(بائيٽون);
  m  = (حرف*)(mALLOc_impl(nb + alignment + MINSIZE));

  /*
  * The attempt to over-allocate (with a size large enough to guarantee the
  * ability to find an aligned region within allocated memory) failed.
  *
  * Try again, this time only allocating exactly the size the user wants. If
  * the allocation now succeeds and just happens to be aligned, we can still
  * fulfill the user's request.
  */
  إذا (m == عدم) {
    size_t extra, extra2;
    /*
     * Use bytes not nb, since mALLOc internally calls request2size too, and
     * each call increases the size to allocate, to account for the header.
     */
    m  = (حرف*)(mALLOc_impl(بائيٽون));
    /* Aligned -> return it */
    إذا ((((بنانشان ڊگهو)(m)) % alignment) == 0)
      أرجع m;
    /*
     * Otherwise, try again, requesting enough extra space to be able to
     * acquire alignment.
     */
    fREe_impl(m);
    /* Add in extra bytes to match misalignment of unexpanded allocation */
    extra = alignment - (((بنانشان ڊگهو)(m)) % alignment);
    m  = (حرف*)(mALLOc_impl(بائيٽون + extra));
    /*
     * m might not be the same as before. Validate that the previous value of
     * extra still works for the current value of m.
     * If (!m), extra2=alignment so 
     */
    إذا (m) {
      extra2 = alignment - (((بنانشان ڊگهو)(m)) % alignment);
      إذا (extra2 > extra) {
        fREe_impl(m);
        m = عدم;
      }
    }
    /* Fall through to original NULL check and chunk splitting logic */
  }

  إذا (m == عدم) أرجع عدم; /* propagate failure */

  p = mem2chunk(m);

  إذا ((((بنانشان ڊگهو)(m)) % alignment) == 0) /* aligned */
  {
#إذا HAVE_MMAP
    إذا(chunk_is_mmapped(p))
      أرجع chunk2mem(p); /* nothing more to do */
#جيڪڏهن_پورو
  }
  وإلا /* misaligned */
  {
    /*
      Find an aligned spot inside chunk.
      Since we need to give back leading space in a chunk of at
      least MINSIZE, if the first calculation places us at
      a spot with less than MINSIZE leader, we can move to the
      next aligned spot -- we've allocated enough total room so that
      this is always possible.
    */

    brk = (حرف*)mem2chunk(((بنانشان ڊگهو)(m + alignment - 1)) & -((نشانوارو) alignment));
    إذا ((ڊگهو)(brk - (حرف*)(p)) < MINSIZE) brk = brk + alignment;

    newp = (mchunkptr)brk;
    leadsize = brk - (حرف*)(p);
    newsize = chunksize(p) - leadsize;

#إذا HAVE_MMAP
    إذا(chunk_is_mmapped(p))
    {
      newp->prev_سائيز = p->prev_سائيز + leadsize;
      سيٽ_سر(newp, newsize|IS_MMAPPED);
      أرجع chunk2mem(newp);
    }
#جيڪڏهن_پورو

    /* give back leader, use the rest */

    سيٽ_سر(newp, newsize | PREV_INUSE);
    سيٽ_inuse_ٻٽو_انگ_at_وٿي(newp, newsize);
    سيٽ_سر_سائيز(p, leadsize);
    fREe_impl(chunk2mem(p));
    p = newp;
    VALGRIND_MALLOCLIKE_بلاڪ(chunk2mem(p), بائيٽون, سائيز_SZ, false);

    أكد (newsize >= nb && (((بنانشان ڊگهو)(chunk2mem(p))) % alignment) == 0);
  }

  /* Also give back spare room at the end */

  remainder_سائيز = chunksize(p) - nb;

  إذا (remainder_سائيز >= (ڊگهو)MINSIZE)
  {
    remainder = chunk_at_وٿي(p, nb);
    سيٽ_سر(remainder, remainder_سائيز | PREV_INUSE);
    سيٽ_سر_سائيز(p, nb);
    VALGRIND_MALLOCLIKE_بلاڪ(chunk2mem(remainder), remainder_سائيز, سائيز_SZ,
			      false);
    fREe_impl(chunk2mem(remainder));
  }

  check_inuse_chunk(p);
  أرجع chunk2mem(p);

}

/*
    valloc just invokes memalign with alignment argument equal
    to the page size of the system (or as near to this as can
    be figured out from all the includes/defines above.)
*/

#إذا __STD_C
Void_t* vALLOc(size_t بائيٽون)
#وإلا
Void_t* vALLOc(بائيٽون) size_t بائيٽون;
#جيڪڏهن_پورو
{
  أرجع mEMALIGn (malloc_getpagesize, بائيٽون);
}

/*
  pvalloc just invokes valloc for the nearest pagesize
  that will accommodate request
*/

#إذا __STD_C
Void_t* pvALLOc(size_t بائيٽون)
#وإلا
Void_t* pvALLOc(بائيٽون) size_t بائيٽون;
#جيڪڏهن_پورو
{
  size_t pagesize = malloc_getpagesize;
  أرجع mEMALIGn (pagesize, (بائيٽون + pagesize - 1) & ~(pagesize - 1));
}

/*

  calloc calls malloc, then zeroes out the allocated chunk.

*/

STATIC_IF_MCHECK
#إذا __STD_C
Void_t* cALLOc_impl(size_t n, size_t elem_سائيز)
#وإلا
Void_t* cALLOc_impl(n, elem_سائيز) size_t n; size_t elem_سائيز;
#جيڪڏهن_پورو
{
  mchunkptr p;
  اندروني_سائيز_T csz;

  اندروني_سائيز_T sz = n * elem_سائيز;

  /* check if expand_top called, in which case don't need to clear */
#إذا CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
#إذا MORECORE_CLEARS
  mchunkptr oldtop = چوٽي;
  اندروني_سائيز_T oldtopsize = chunksize(چوٽي);
#جيڪڏهن_پورو
#جيڪڏهن_پورو
  Void_t* mem = mALLOc_impl (sz);

  إذا ((ڊگهو)n < 0) أرجع عدم;

  إذا (mem == عدم)
    أرجع عدم;
  وإلا
  {
#إذا CONFIG_IS_ENABLED(SYS_MALLOC_F)
	إذا (!(gd->flags & GD_FLG_ڀريل_MALLOC_INIT)) {
		memset(mem, 0, sz);
		أرجع mem;
	}
#جيڪڏهن_پورو
    p = mem2chunk(mem);

    /* Two optional cases in which clearing not necessary */

#إذا HAVE_MMAP
    إذا (chunk_is_mmapped(p)) أرجع mem;
#جيڪڏهن_پورو

    csz = chunksize(p);

#إذا CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
#إذا MORECORE_CLEARS
    إذا (p == oldtop && csz > oldtopsize)
    {
      /* clear only the bytes from non-freshly-sbrked memory */
      csz = oldtopsize;
    }
#جيڪڏهن_پورو
#جيڪڏهن_پورو

    MALLOC_ZERO(mem, csz - سائيز_SZ);
    VALGRIND_MAKE_MEM_DEFINED(mem, sz);
    أرجع mem;
  }
}

/*

  cfree just calls free. It is needed/defined on some systems
  that pair it with calloc, presumably for odd historical reasons.

*/

#إذا !defined(اندروني_LINUX_C_LIB) || !defined(__ELF__)
#إذا __STD_C
فراغ cfree(Void_t *mem)
#وإلا
فراغ cfree(mem) Void_t *mem;
#جيڪڏهن_پورو
{
  fREe(mem);
}
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل MCHECK_ڍير_PROTECTION
 #ضمن "mcheck_core.inc.h"
 #إذا !__STD_C
  #غلطي "must have __STD_C"
 #جيڪڏهن_پورو

Void_t *mALLOc(size_t بائيٽون)
{
	mcheck_pedantic_prehook();
	size_t fullsz = mcheck_alloc_prehook(بائيٽون);
	فراغ *p = mALLOc_impl(fullsz);

	إذا (!p)
		أرجع p;
	أرجع mcheck_alloc_posthook(p, بائيٽون);
}

فراغ fREe(Void_t *mem) { fREe_impl(mcheck_free_prehook(mem)); }

Void_t *rEALLOc(Void_t *oldmem, size_t بائيٽون)
{
	mcheck_pedantic_prehook();
	إذا (بائيٽون == 0) {
		إذا (oldmem)
			fREe(oldmem);
		أرجع عدم;
	}

	إذا (oldmem == عدم)
		أرجع mALLOc(بائيٽون);

	فراغ *p = mcheck_reallocfree_prehook(oldmem);
	size_t newsz = mcheck_alloc_prehook(بائيٽون);

	p = rEALLOc_impl(p, newsz);
	إذا (!p)
		أرجع p;
	أرجع mcheck_alloc_noclean_posthook(p, بائيٽون);
}

Void_t *mEMALIGn(size_t alignment, size_t بائيٽون)
{
	mcheck_pedantic_prehook();
	size_t fullsz = mcheck_memalign_prehook(alignment, بائيٽون);
	فراغ *p = mEMALIGn_impl(alignment, fullsz);

	إذا (!p)
		أرجع p;
	أرجع mcheck_memalign_posthook(alignment, p, بائيٽون);
}

// pvALLOc, vALLOc - redirect to mEMALIGn, defined here, so they need no wrapping.

Void_t *cALLOc(size_t n, size_t elem_سائيز)
{
	mcheck_pedantic_prehook();
	// NB: here is no overflow check.
	size_t fullsz = mcheck_alloc_prehook(n * elem_سائيز);
	فراغ *p = cALLOc_impl(1, fullsz);

	إذا (!p)
		أرجع p;
	أرجع mcheck_alloc_noclean_posthook(p, n * elem_سائيز);
}

// mcheck API {
صحيح mcheck_pedantic(mcheck_abortfunc_t f)
{
	mcheck_initialize(f, 1);
	أرجع 0;
}

صحيح mcheck(mcheck_abortfunc_t f)
{
	mcheck_initialize(f, 0);
	أرجع 0;
}

فراغ mcheck_check_all(فراغ) { mcheck_pedantic_check(); }

تعداد mcheck_status mprobe(فراغ *__ptr) { أرجع mcheck_mprobe(__ptr); }
// mcheck API }
#جيڪڏهن_پورو

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

#إذا __STD_C
صحيح malloc_trim(size_t pad)
#وإلا
صحيح malloc_trim(pad) size_t pad;
#جيڪڏهن_پورو
{
  ڊگهو  چوٽي_سائيز;        /* Amount of top-most memory */
  ڊگهو  extra;           /* Amount to release */
  حرف* current_brk;     /* address returned by pre-check sbrk call */
  حرف* new_brk;         /* address returned by negative sbrk call */

  بنانشان ڊگهو pagesz = malloc_getpagesize;

  چوٽي_سائيز = chunksize(چوٽي);
  extra = ((چوٽي_سائيز - pad - MINSIZE + (pagesz-1)) / pagesz - 1) * pagesz;

  إذا (extra < (ڊگهو)pagesz)  /* Not enough memory to release */
    أرجع 0;

  وإلا
  {
    /* Test to make sure no one else called sbrk */
    current_brk = (حرف*)(MORECORE (0));
    إذا (current_brk != (حرف*)(چوٽي) + چوٽي_سائيز)
      أرجع 0;     /* Apparently we don't own memory; must fail */

    وإلا
    {
      new_brk = (حرف*)(MORECORE (-extra));

      إذا (new_brk == (حرف*)(MORECORE_FAILURE)) /* sbrk failed? */
      {
	/* Try to figure out what we have */
	current_brk = (حرف*)(MORECORE (0));
	چوٽي_سائيز = current_brk - (حرف*)چوٽي;
	إذا (چوٽي_سائيز >= (ڊگهو)MINSIZE) /* if not, we are very very dead! */
	{
	  sbrked_mem = current_brk - sbrk_بنياد;
	  سيٽ_سر(چوٽي, چوٽي_سائيز | PREV_INUSE);
	}
	check_chunk(چوٽي);
	أرجع 0;
      }

      وإلا
      {
	/* Success. Adjust top accordingly. */
	سيٽ_سر(چوٽي, (چوٽي_سائيز - extra) | PREV_INUSE);
	sbrked_mem -= extra;
	check_chunk(چوٽي);
	أرجع 1;
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

#إذا __STD_C
size_t malloc_usable_size(Void_t* mem)
#وإلا
size_t malloc_usable_size(mem) Void_t* mem;
#جيڪڏهن_پورو
{
  mchunkptr p;
  إذا (mem == عدم)
    أرجع 0;
  وإلا
  {
    p = mem2chunk(mem);
    إذا(!chunk_is_mmapped(p))
    {
      إذا (!inuse(p)) أرجع 0;
      check_inuse_chunk(p);
      أرجع chunksize(p) - سائيز_SZ;
    }
    أرجع chunksize(p) - 2*سائيز_SZ;
  }
}

/* Utility to update current_mallinfo for malloc_stats and mallinfo() */

#جيڪڏهن_وصف_ٿيل DEBUG
ساكن فراغ malloc_update_mallinfo(فراغ)
{
  صحيح i;
  mbinptr b;
  mchunkptr p;
#جيڪڏهن_وصف_ٿيل DEBUG
  mchunkptr q;
#جيڪڏهن_پورو

  اندروني_سائيز_T avail = chunksize(چوٽي);
  صحيح   navail = ((ڊگهو)(avail) >= (ڊگهو)MINSIZE)? 1 : 0;

  لكل (i = 1; i < NAV; ++i)
  {
    b = bin_at(i);
    لكل (p = آخري(b); p != b; p = p->bk)
    {
#جيڪڏهن_وصف_ٿيل DEBUG
      check_free_chunk(p);
      لكل (q = ايندڙ_chunk(p);
	   q < چوٽي && inuse(q) && (ڊگهو)(chunksize(q)) >= (ڊگهو)MINSIZE;
	   q = ايندڙ_chunk(q))
	check_inuse_chunk(q);
#جيڪڏهن_پورو
      avail += chunksize(p);
      navail++;
    }
  }

  current_mallinfo.ordblks = navail;
  current_mallinfo.uordblks = sbrked_mem - avail;
  current_mallinfo.fordblks = avail;
  current_mallinfo.hblks = n_mmaps;
  current_mallinfo.hblkhd = mmapped_mem;
  current_mallinfo.keepcost = chunksize(چوٽي);

}
#جيڪڏهن_پورو	/* DEBUG */

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

#جيڪڏهن_وصف_ٿيل DEBUG
فراغ malloc_stats(فراغ)
{
  malloc_update_mallinfo();
  لکو("max system bytes = %10u\n",
	  (بنانشان صحيح)(max_ڪل_mem));
  لکو("system bytes     = %10u\n",
	  (بنانشان صحيح)(sbrked_mem + mmapped_mem));
  لکو("in use bytes     = %10u\n",
	  (بنانشان صحيح)(current_mallinfo.uordblks + mmapped_mem));
#إذا HAVE_MMAP
  لکو("max mmap regions = %10u\n",
	  (بنانشان صحيح)max_n_mmaps);
#جيڪڏهن_پورو
}
#جيڪڏهن_پورو	/* DEBUG */

/*
  mallinfo returns a copy of updated current mallinfo.
*/

#جيڪڏهن_وصف_ٿيل DEBUG
بنية mallinfo mALLINFo(فراغ)
{
  malloc_update_mallinfo();
  أرجع current_mallinfo;
}
#جيڪڏهن_پورو	/* DEBUG */

/*
  mallopt:

    mallopt is the general SVID/XPG interface to tunable parameters.
    The format is to provide a (parameter-number, parameter-value) pair.
    mallopt then sets the corresponding parameter to the argument
    value if it can (i.e., so long as the value is meaningful),
    and returns 1 if successful else 0.

    See descriptions of tunable parameters above.

*/

#إذا __STD_C
صحيح mALLOPt(صحيح param_عدد, صحيح قدر)
#وإلا
صحيح mALLOPt(param_عدد, قدر) صحيح param_عدد; صحيح قدر;
#جيڪڏهن_پورو
{
  بدل(param_عدد)
  {
    حالة M_TRIM_THRESHOLD:
      trim_حد = قدر; أرجع 1;
    حالة M_TOP_PAD:
      چوٽي_pad = قدر; أرجع 1;
    حالة M_MMAP_THRESHOLD:
      mmap_حد = قدر; أرجع 1;
    حالة M_MMAP_MAX:
#إذا HAVE_MMAP
      n_mmaps_max = قدر; أرجع 1;
#وإلا
      إذا (قدر != 0) أرجع 0; وإلا  n_mmaps_max = قدر; أرجع 1;
#جيڪڏهن_پورو

    افتراضي:
      أرجع 0;
  }
}

صحيح initf_malloc(فراغ)
{
#إذا CONFIG_IS_ENABLED(SYS_MALLOC_F)
	أكد(gd->malloc_بنياد);	/* Set up by crt0.S */
	gd->malloc_حد_بندي = CONFIG_VAL(SYS_MALLOC_F_LEN);
	gd->malloc_ptr = 0;
#جيڪڏهن_پورو

	أرجع 0;
}

فراغ malloc_enable_testing(صحيح max_allocs)
{
	malloc_testing = true;
	malloc_max_allocs = max_allocs;
}

فراغ malloc_disable_testing(فراغ)
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
