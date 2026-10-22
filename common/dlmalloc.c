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
#تعریف_کن DEBUG
#پایان_اگر

#ضمن <log.h>
#ضمن <asm/global_data.h>

#ضمن <malloc.h>
#ضمن <mapmem.h>
#ضمن <string.h>
#ضمن <asm/io.h>
#ضمن <valgrind/memcheck.h>

#اگر_تعریف_شده DEBUG
#إذا __STD_C
ساكن فراغ malloc_update_mallinfo (فراغ);
فراغ malloc_stats (فراغ);
#وإلا
ساكن فراغ malloc_update_mallinfo ();
فراغ malloc_stats();
#پایان_اگر
#پایان_اگر	/* DEBUG */

DECLARE_GLOBAL_داده_PTR;

#اگر_تعریف_شده MCHECK_انباشته_PROTECTION
 #تعریف_کن STATIC_IF_MCHECK ساكن
 #لغو_تعریف MALLOC_COPY
 #لغو_تعریف MALLOC_ZERO
ساكن مضمن فراغ MALLOC_ZERO(فراغ *p, size_t sz) { memset(p, 0, sz); }
ساكن مضمن فراغ MALLOC_COPY(فراغ *dest, ثابت فراغ *src, size_t sz) { کپی_حافظه(dest, src, sz); }
#وإلا
 #تعریف_کن STATIC_IF_MCHECK
 #تعریف_کن mALLOc_impl mALLOc
 #تعریف_کن fREe_impl fREe
 #تعریف_کن rEALLOc_impl rEALLOc
 #تعریف_کن mEMALIGn_impl mEMALIGn
 #تعریف_کن cALLOc_impl cALLOc
#پایان_اگر

/*
  Emulation of sbrk for WIN32
  All code within the ifdef WIN32 is untested by me.

  Thanks to Martin Fong and others for supplying this.
*/

#اگر_تعریف_شده WIN32

#تعریف_کن Align_صفحه(جمع_کن) (((جمع_کن) + (malloc_getpagesize-1)) & \
~(malloc_getpagesize-1))
#تعریف_کن AlignPage64K(جمع_کن) (((جمع_کن) + (0x10000 - 1)) & ~(0x10000 - 1))

/* resrve 64MB to insure large contiguous space */
#تعریف_کن RESERVED_اندازه (1024*1024*64)
#تعریف_کن بعدی_اندازه (2048*1024)
#تعریف_کن بالاترین_حافظه ((بیعلامت دراز)2*1024*1024*1024)

بنية GmListElement;
تعريفنوع بنية GmListElement GmListElement;

بنية GmListElement
{
	GmListElement* بعدی;
	فراغ* پایه;
};

ساكن GmListElement* سر = 0;
ساكن بیعلامت صحيح g_بعدی_آدرس = 0;
ساكن بیعلامت صحيح g_آدرس_پایه = 0;
ساكن بیعلامت صحيح g_Allocated_اندازه = 0;

ساكن
GmListElement* makeGmListElement (فراغ* bas)
{
	GmListElement* this;
	this = (GmListElement*)(فراغ*)LocalAlloc (0, حجم (GmListElement));
	أكد (this);
	إذا (this)
	{
		this->پایه = bas;
		this->بعدی = سر;
		سر = this;
	}
	أرجع this;
}

فراغ gcleanup (فراغ)
{
	BOOL rval;
	أكد ( (سر == عدم) || (سر->پایه == (فراغ*)g_آدرس_پایه));
	إذا (g_آدرس_پایه && (g_بعدی_آدرس - g_آدرس_پایه))
	{
		rval = VirtualFree ((فراغ*)g_آدرس_پایه,
							g_بعدی_آدرس - g_آدرس_پایه,
							MEM_DECOMMIT);
	أكد (rval);
	}
	طالما (سر)
	{
		GmListElement* بعدی = سر->بعدی;
		rval = VirtualFree (سر->پایه, 0, MEM_RELEASE);
		أكد (rval);
		LocalFree (سر);
		سر = بعدی;
	}
}

ساكن
فراغ* پیدا_کن_Region (فراغ* شروع_آدرس, بیعلامت دراز اندازه)
{
	حافظه_BASIC_INFORMATION info;
	إذا (اندازه >= بالاترین_حافظه) أرجع عدم;

	طالما ((بیعلامت دراز)شروع_آدرس + اندازه < بالاترین_حافظه)
	{
		VirtualQuery (شروع_آدرس, &info, حجم (info));
		إذا ((info.State == MEM_FREE) && (info.Region_اندازه >= اندازه))
			أرجع شروع_آدرس;
		وإلا
		{
			/* Requested region is not available so see if the */
			/* next region is available.  Set 'start_address' */
			/* to the next region and call 'VirtualQuery()' */
			/* again. */

			شروع_آدرس = (حرف*)info.پایه_آدرس + info.Region_اندازه;

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

			شروع_آدرس =
				(فراغ *) AlignPage64K((بیعلامت دراز) شروع_آدرس);
		}
	}
	أرجع عدم;

}

فراغ* wsbrk (دراز اندازه)
{
	فراغ* tmp;
	إذا (اندازه > 0)
	{
		إذا (g_آدرس_پایه == 0)
		{
			g_Allocated_اندازه = max (RESERVED_اندازه, Align_صفحه (اندازه));
			g_بعدی_آدرس = g_آدرس_پایه =
				(بیعلامت صحيح)VirtualAlloc (عدم, g_Allocated_اندازه,
											MEM_RESERVE, صفحه_NOACCESS);
		} وإلا إذا (Align_صفحه (g_بعدی_آدرس + اندازه) > (g_آدرس_پایه +
g_Allocated_اندازه))
		{
			دراز new_اندازه = max (بعدی_اندازه, Align_صفحه (اندازه));
			فراغ* new_آدرس = (فراغ*)(g_آدرس_پایه+g_Allocated_اندازه);
			افعل
			{
				new_آدرس = پیدا_کن_Region (new_آدرس, new_اندازه);

				إذا (!new_آدرس)
					أرجع (فراغ*)-1;

				g_آدرس_پایه = g_بعدی_آدرس =
					(بیعلامت صحيح)VirtualAlloc (new_آدرس, new_اندازه,
												MEM_RESERVE, صفحه_NOACCESS);
				/* repeat in case of race condition */
				/* The region that we found has been snagged */
				/* by another thread */
			}
			طالما (g_آدرس_پایه == 0);

			أكد (new_آدرس == (فراغ*)g_آدرس_پایه);

			g_Allocated_اندازه = new_اندازه;

			إذا (!makeGmListElement ((فراغ*)g_آدرس_پایه))
				أرجع (فراغ*)-1;
		}
		إذا ((اندازه + g_بعدی_آدرس) > Align_صفحه (g_بعدی_آدرس))
		{
			فراغ* res;
			res = VirtualAlloc ((فراغ*)Align_صفحه (g_بعدی_آدرس),
								(اندازه + g_بعدی_آدرس -
								 Align_صفحه (g_بعدی_آدرس)),
								MEM_COMMIT, صفحه_READWRITE);
			إذا (!res)
				أرجع (فراغ*)-1;
		}
		tmp = (فراغ*)g_بعدی_آدرس;
		g_بعدی_آدرس = (بیعلامت صحيح)tmp + اندازه;
		أرجع tmp;
	}
	وإلا إذا (اندازه < 0)
	{
		بیعلامت صحيح alignedGoal = Align_صفحه (g_بعدی_آدرس + اندازه);
		/* Trim by releasing the virtual memory */
		إذا (alignedGoal >= g_آدرس_پایه)
		{
			VirtualFree ((فراغ*)alignedGoal, g_بعدی_آدرس - alignedGoal,
						 MEM_DECOMMIT);
			g_بعدی_آدرس = g_بعدی_آدرس + اندازه;
			أرجع (فراغ*)g_بعدی_آدرس;
		}
		وإلا
		{
			VirtualFree ((فراغ*)g_آدرس_پایه, g_بعدی_آدرس - g_آدرس_پایه,
						 MEM_DECOMMIT);
			g_بعدی_آدرس = g_آدرس_پایه;
			أرجع (فراغ*)-1;
		}
	}
	وإلا
	{
		أرجع (فراغ*)g_بعدی_آدرس;
	}
}

#پایان_اگر

/*
  Type declarations
*/

بنية malloc_chunk
{
  داخلی_اندازه_T prev_اندازه; /* Size of previous chunk (if free). */
  داخلی_اندازه_T اندازه;      /* Size in bytes, including overhead. */
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

#تعریف_کن اندازه_SZ                (حجم(داخلی_اندازه_T))
#تعریف_کن MALLOC_ALIGNMENT       (اندازه_SZ + اندازه_SZ)
#تعریف_کن MALLOC_ALIGN_ماسک      (MALLOC_ALIGNMENT - 1)
#تعریف_کن MINSIZE                (حجم(بنية malloc_chunk))

/* conversion from malloc headers to user pointers, and back */

#تعریف_کن chunk2mem(p)   ((Void_t*)((حرف*)(p) + 2*اندازه_SZ))
#تعریف_کن mem2chunk(mem) ((mchunkptr)((حرف*)(mem) - 2*اندازه_SZ))

/* pad request bytes into a usable size */

#تعریف_کن request2size(req) \
 ((((req) + (اندازه_SZ + MALLOC_ALIGN_ماسک)) < \
  (MINSIZE + MALLOC_ALIGN_ماسک)) ? MINSIZE : \
   (((req) + (اندازه_SZ + MALLOC_ALIGN_ماسک)) & ~(MALLOC_ALIGN_ماسک)))

/* Check if m has acceptable alignment */

#تعریف_کن aligned_OK(m)    (((بیعلامت دراز)((m)) & (MALLOC_ALIGN_ماسک)) == 0)

/*
  Physical chunk operations
*/

/* size field is or'ed with PREV_INUSE when previous adjacent chunk in use */

#تعریف_کن PREV_INUSE 0x1

/* size field is or'ed with IS_MMAPPED if the chunk was obtained with mmap() */

#تعریف_کن IS_MMAPPED 0x2

/* Bits to mask off when extracting size */

#تعریف_کن اندازه_بیتها (PREV_INUSE|IS_MMAPPED)

/* Ptr to next physical malloc_chunk. */

#تعریف_کن بعدی_chunk(p) ((mchunkptr)( ((حرف*)(p)) + ((p)->اندازه & ~PREV_INUSE) ))

/* Ptr to previous physical malloc_chunk */

#تعریف_کن prev_chunk(p)\
   ((mchunkptr)( ((حرف*)(p)) - ((p)->prev_اندازه) ))

/* Treat space at ptr + offset as a chunk */

#تعریف_کن chunk_at_جابه_جایی(p, s)  ((mchunkptr)(((حرف*)(p)) + (s)))

/*
  Dealing with use bits
*/

/* extract p's inuse bit */

#تعریف_کن inuse(p)\
((((mchunkptr)(((حرف*)(p))+((p)->اندازه & ~PREV_INUSE)))->اندازه) & PREV_INUSE)

/* extract inuse bit of previous chunk */

#تعریف_کن prev_inuse(p)  ((p)->اندازه & PREV_INUSE)

/* check for mmap()'ed chunk */

#تعریف_کن chunk_is_mmapped(p) ((p)->اندازه & IS_MMAPPED)

/* set/clear chunk as in use without otherwise disturbing */

#تعریف_کن مجموعه_inuse(p)\
((mchunkptr)(((حرف*)(p)) + ((p)->اندازه & ~PREV_INUSE)))->اندازه |= PREV_INUSE

#تعریف_کن clear_inuse(p)\
((mchunkptr)(((حرف*)(p)) + ((p)->اندازه & ~PREV_INUSE)))->اندازه &= ~(PREV_INUSE)

/* check/set/clear inuse bits in known places */

#تعریف_کن inuse_بیت_at_جابه_جایی(p, s)\
 (((mchunkptr)(((حرف*)(p)) + (s)))->اندازه & PREV_INUSE)

#تعریف_کن مجموعه_inuse_بیت_at_جابه_جایی(p, s)\
 (((mchunkptr)(((حرف*)(p)) + (s)))->اندازه |= PREV_INUSE)

#تعریف_کن clear_inuse_بیت_at_جابه_جایی(p, s)\
 (((mchunkptr)(((حرف*)(p)) + (s)))->اندازه &= ~(PREV_INUSE))

/*
  Dealing with size fields
*/

/* Get size, ignoring use bits */

#تعریف_کن chunksize(p)          ((p)->اندازه & ~(اندازه_بیتها))

/* Set size at head, without disturbing its use bit */

#تعریف_کن مجموعه_سر_اندازه(p, s)   ((p)->اندازه = (((p)->اندازه & PREV_INUSE) | (s)))

/* Set size/use ignoring previous bits in header */

#تعریف_کن مجموعه_سر(p, s)        ((p)->اندازه = (s))

/* Set size at footer (only when chunk is not in use) */

#تعریف_کن مجموعه_foot(p, s)   (((mchunkptr)((حرف*)(p) + (s)))->prev_اندازه = (s))

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

#تعریف_کن NAV             128   /* number of bins */

تعريفنوع بنية malloc_chunk* mbinptr;

/* access macros */

#تعریف_کن bin_at(i)      ((mbinptr)((حرف*)&(av_[2*(i) + 2]) - 2*اندازه_SZ))
#تعریف_کن بعدی_bin(b)    ((mbinptr)((حرف*)(b) + 2 * حجم(mbinptr)))
#تعریف_کن prev_bin(b)    ((mbinptr)((حرف*)(b) - 2 * حجم(mbinptr)))

/*
   The first 2 bins are never indexed. The corresponding av_ cells are instead
   used for bookkeeping. This is not to save space, but to simplify
   indexing, maintain locality, and avoid some initialization tests.
*/

#تعریف_کن بالاترین            (av_[2])          /* The topmost chunk */
#تعریف_کن آخرین_remainder (bin_at(1))       /* remainder from last split */

/*
   Because top initially points to its own bin with initial
   zero size, thus forcing extension on the first malloc request,
   we avoid having any special code in malloc to check whether
   it even exists yet. But we still need to in malloc_extend_top.
*/

#تعریف_کن initial_بالاترین    ((mchunkptr)(bin_at(0)))

/* Helper macro to initialize bins */

#تعریف_کن IAV(i)  bin_at(i), bin_at(i)

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

#اگر_تعریف_شده CONFIG_SYS_MALLOC_DEFAULT_مقصد_INIT
ساكن فراغ malloc_init(فراغ);
#پایان_اگر

ulong mem_malloc_شروع = 0;
ulong mem_malloc_انتها = 0;
ulong mem_malloc_brk = 0;

ساكن bool malloc_testing;	/* enable test mode */
ساكن صحيح malloc_max_allocs;	/* return NULL after this many calls to malloc() */

فراغ *sbrk(ptrdiff_t increment)
{
	ulong old = mem_malloc_brk;
	ulong new = old + increment;

	إذا ((new < mem_malloc_شروع) || (new > mem_malloc_انتها))
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

فراغ mem_malloc_init(ulong شروع, ulong اندازه)
{
	mem_malloc_شروع = (ulong)map_sysmem(شروع, اندازه);
	mem_malloc_انتها = mem_malloc_شروع + اندازه;
	mem_malloc_brk = mem_malloc_شروع;

#اگر_تعریف_شده CONFIG_SYS_MALLOC_DEFAULT_مقصد_INIT
	malloc_init();
#پایان_اگر

	debug("using memory %#lx-%#lx for malloc()\n", mem_malloc_شروع,
	      mem_malloc_انتها);
#إذا CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
	memset((فراغ *)mem_malloc_شروع, 0x0, اندازه);
#پایان_اگر
}

/* field-extraction macros */

#تعریف_کن اولی(b) ((b)->fd)
#تعریف_کن آخرین(b)  ((b)->bk)

/*
  Indexing into bins
*/

#تعریف_کن bin_شاخص(sz)                                                          \
(((((بیعلامت دراز)(sz)) >> 9) ==    0) ?       (((بیعلامت دراز)(sz)) >>  3): \
 ((((بیعلامت دراز)(sz)) >> 9) <=    4) ?  56 + (((بیعلامت دراز)(sz)) >>  6): \
 ((((بیعلامت دراز)(sz)) >> 9) <=   20) ?  91 + (((بیعلامت دراز)(sz)) >>  9): \
 ((((بیعلامت دراز)(sz)) >> 9) <=   84) ? 110 + (((بیعلامت دراز)(sz)) >> 12): \
 ((((بیعلامت دراز)(sz)) >> 9) <=  340) ? 119 + (((بیعلامت دراز)(sz)) >> 15): \
 ((((بیعلامت دراز)(sz)) >> 9) <= 1364) ? 124 + (((بیعلامت دراز)(sz)) >> 18): \
					  126)
/*
  bins for chunks < 512 are all spaced 8 bytes apart, and hold
  identically sized chunks. This is exploited in malloc.
*/

#تعریف_کن MAX_SMALLBIN         63
#تعریف_کن MAX_SMALLBIN_اندازه   512
#تعریف_کن SMALLBIN_عرض        8

#تعریف_کن smallbin_شاخص(sz)  (((بیعلامت دراز)(sz)) >> 3)

/*
   Requests are `small' if both the corresponding and the next bin are small
*/

#تعریف_کن is_small_request(nb) (nb < MAX_SMALLBIN_اندازه - SMALLBIN_عرض)

/*
    To help compensate for the large number of bins, a one-level index
    structure is used for bin-by-bin searching.  `binblocks' is a
    one-word bitvector recording whether groups of BINBLOCKWIDTH bins
    have any (possibly) non-empty bins, so they can be skipped over
    all at once during during traversals. The bits are NOT always
    cleared as soon as all bins in a block are empty, but instead only
    when all are noticed to be empty during traversal in malloc.
*/

#تعریف_کن BINBLOCKWIDTH     4   /* bins per block */

#تعریف_کن binblocks_r     ((داخلی_اندازه_T)av_[1]) /* bitvector of nonempty blocks */
#تعریف_کن binblocks_w     (av_[1])

/* bin<->block macros */

#تعریف_کن idx2binblock(ix)    ((بیعلامت)1 << (ix / BINBLOCKWIDTH))
#تعریف_کن mark_binblock(ii)   (binblocks_w = (mbinptr)(binblocks_r | idx2binblock(ii)))
#تعریف_کن clear_binblock(ii)  (binblocks_w = (mbinptr)(binblocks_r & ~(idx2binblock(ii))))

/*  Other static bookkeeping data */

/* variables holding tunable values */

ساكن بیعلامت دراز trim_آستانه   = DEFAULT_TRIM_آستانه;
ساكن بیعلامت دراز بالاترین_pad          = DEFAULT_بالاترین_PAD;
ساكن بیعلامت صحيح  n_mmaps_max      = DEFAULT_MMAP_MAX;
ساكن بیعلامت دراز mmap_آستانه   = DEFAULT_MMAP_آستانه;

/* The first value returned from sbrk */
ساكن حرف* sbrk_پایه = (حرف*)(-1);

/* The maximum memory obtained from system via sbrk */
ساكن بیعلامت دراز max_sbrked_mem = 0;

/* The maximum via either sbrk or mmap */
ساكن بیعلامت دراز max_مجموع_mem = 0;

/* internal working copy of mallinfo */
ساكن بنية mallinfo current_mallinfo = {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* The total memory obtained from system via sbrk */
#تعریف_کن sbrked_mem  (current_mallinfo.arena)

/* Tracking mmaps */

#اگر_تعریف_شده DEBUG
ساكن بیعلامت صحيح n_mmaps = 0;
#پایان_اگر	/* DEBUG */
ساكن بیعلامت دراز mmapped_mem = 0;
#إذا HAVE_MMAP
ساكن بیعلامت صحيح max_n_mmaps = 0;
ساكن بیعلامت دراز max_mmapped_mem = 0;
#پایان_اگر

#اگر_تعریف_شده CONFIG_SYS_MALLOC_DEFAULT_مقصد_INIT
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
	sbrk_پایه = (حرف *)(-1);
	max_sbrked_mem = 0;
	max_مجموع_mem = 0;
#اگر_تعریف_شده DEBUG
	memset((فراغ *)&current_mallinfo, 0, حجم(بنية mallinfo));
#پایان_اگر
}
#پایان_اگر

/*
  Debugging support
*/

#اگر_تعریف_شده DEBUG

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
#پایان_اگر
{
  داخلی_اندازه_T sz = p->اندازه & ~PREV_INUSE;

  /* No checkable chunk is mmapped */
  أكد(!chunk_is_mmapped(p));

  /* Check for legal address ... */
  أكد((حرف*)p >= sbrk_پایه);
  إذا (p != بالاترین)
    أكد((حرف*)p + sz <= (حرف*)بالاترین);
  وإلا
    أكد((حرف*)p + sz <= sbrk_پایه + sbrked_mem);

}

#إذا __STD_C
ساكن فراغ do_check_free_chunk(mchunkptr p)
#وإلا
ساكن فراغ do_check_free_chunk(p) mchunkptr p;
#پایان_اگر
{
  داخلی_اندازه_T sz = p->اندازه & ~PREV_INUSE;
  mchunkptr بعدی = chunk_at_جابه_جایی(p, sz);

  do_check_chunk(p);

  /* Check whether it claims to be free ... */
  أكد(!inuse(p));

  /* Unless a special marker, must have OK fields */
  إذا ((دراز)sz >= (دراز)MINSIZE)
  {
    أكد((sz & MALLOC_ALIGN_ماسک) == 0);
    أكد(aligned_OK(chunk2mem(p)));
    /* ... matching footer field */
    أكد(بعدی->prev_اندازه == sz);
    /* ... and is fully consolidated */
    أكد(prev_inuse(p));
    أكد (بعدی == بالاترین || inuse(بعدی));

    /* ... and has minimally sane links */
    أكد(p->fd->bk == p);
    أكد(p->bk->fd == p);
  }
  وإلا /* markers are always of size SIZE_SZ */
    أكد(sz == اندازه_SZ);
}

#إذا __STD_C
ساكن فراغ do_check_inuse_chunk(mchunkptr p)
#وإلا
ساكن فراغ do_check_inuse_chunk(p) mchunkptr p;
#پایان_اگر
{
  mchunkptr بعدی = بعدی_chunk(p);
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
    أكد(بعدی_chunk(prv) == p);
    do_check_free_chunk(prv);
  }
  إذا (بعدی == بالاترین)
  {
    أكد(prev_inuse(بعدی));
    أكد(chunksize(بعدی) >= MINSIZE);
  }
  وإلا إذا (!inuse(بعدی))
    do_check_free_chunk(بعدی);

}

#إذا __STD_C
ساكن فراغ do_check_malloced_chunk(mchunkptr p, داخلی_اندازه_T s)
#وإلا
ساكن فراغ do_check_malloced_chunk(p, s) mchunkptr p; داخلی_اندازه_T s;
#پایان_اگر
{
  داخلی_اندازه_T sz = p->اندازه & ~PREV_INUSE;
  دراز جا = sz - s;

  do_check_inuse_chunk(p);

  /* Legal size ... */
  أكد((دراز)sz >= (دراز)MINSIZE);
  أكد((sz & MALLOC_ALIGN_ماسک) == 0);
  أكد(جا >= 0);
  أكد(جا < (دراز)MINSIZE);

  /* ... and alignment */
  أكد(aligned_OK(chunk2mem(p)));

  /* ... and was allocated at front of an available chunk */
  أكد(prev_inuse(p));

}

#تعریف_کن check_free_chunk(P)  do_check_free_chunk(P)
#تعریف_کن check_inuse_chunk(P) do_check_inuse_chunk(P)
#تعریف_کن check_chunk(P) do_check_chunk(P)
#تعریف_کن check_malloced_chunk(P,N) do_check_malloced_chunk(P,N)
#وإلا
#تعریف_کن check_free_chunk(P)
#تعریف_کن check_inuse_chunk(P)
#تعریف_کن check_chunk(P)
#تعریف_کن check_malloced_chunk(P,N)
#پایان_اگر

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

#تعریف_کن frontlink(P, S, IDX, BK, FD)                                          \
{                                                                             \
  إذا (S < MAX_SMALLBIN_اندازه)                                                  \
  {                                                                           \
    IDX = smallbin_شاخص(S);                                                  \
    mark_binblock(IDX);                                                       \
    BK = bin_at(IDX);                                                         \
    FD = BK->fd;                                                              \
    P->bk = BK;                                                               \
    P->fd = FD;                                                               \
    FD->bk = BK->fd = P;                                                      \
  }                                                                           \
  وإلا                                                                        \
  {                                                                           \
    IDX = bin_شاخص(S);                                                       \
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

#تعریف_کن unlink(P, BK, FD)                                                     \
{                                                                             \
  BK = P->bk;                                                                 \
  FD = P->fd;                                                                 \
  FD->bk = BK;                                                                \
  BK->fd = FD;                                                                \
}                                                                             \

/* Place p as the last remainder */

#تعریف_کن link_آخرین_remainder(P)                                                \
{                                                                             \
  آخرین_remainder->fd = آخرین_remainder->bk =  P;                               \
  P->fd = P->bk = آخرین_remainder;                                             \
}

/* Clear the last_remainder bin */

#تعریف_کن clear_آخرین_remainder \
  (آخرین_remainder->fd = آخرین_remainder->bk = آخرین_remainder)

/* Routines dealing with mmap(). */

#إذا HAVE_MMAP

#إذا __STD_C
ساكن mchunkptr mmap_chunk(size_t اندازه)
#وإلا
ساكن mchunkptr mmap_chunk(اندازه) size_t اندازه;
#پایان_اگر
{
  size_t صفحه_ماسک = malloc_getpagesize - 1;
  mchunkptr p;

#اگر_تعریف_نشده MAP_ANONYMOUS
  ساكن صحيح fd = -1;
#پایان_اگر

  إذا(n_mmaps >= n_mmaps_max) أرجع 0; /* too many regions */

  /* For mmapped chunks, the overhead is one SIZE_SZ unit larger, because
   * there is no following chunk whose prev_size field could be used.
   */
  اندازه = (اندازه + اندازه_SZ + صفحه_ماسک) & ~صفحه_ماسک;

#اگر_تعریف_شده MAP_ANONYMOUS
  p = (mchunkptr)mmap(0, اندازه, PROT_READ|PROT_WRITE,
		      MAP_خصوصی|MAP_ANONYMOUS, -1, 0);
#وإلا /* !MAP_ANONYMOUS */
  إذا (fd < 0)
  {
    fd = open("/dev/zero", O_RDWR);
    إذا(fd < 0) أرجع 0;
  }
  p = (mchunkptr)mmap(0, اندازه, PROT_READ|PROT_WRITE, MAP_خصوصی, fd, 0);
#پایان_اگر

  إذا(p == (mchunkptr)-1) أرجع 0;

  n_mmaps++;
  إذا (n_mmaps > max_n_mmaps) max_n_mmaps = n_mmaps;

  /* We demand that eight bytes into a page must be 8-byte aligned. */
  أكد(aligned_OK(chunk2mem(p)));

  /* The offset to the start of the mmapped region is stored
   * in the prev_size field of the chunk; normally it is zero,
   * but that can be changed in memalign().
   */
  p->prev_اندازه = 0;
  مجموعه_سر(p, اندازه|IS_MMAPPED);

  mmapped_mem += اندازه;
  إذا ((بیعلامت دراز)mmapped_mem > (بیعلامت دراز)max_mmapped_mem)
    max_mmapped_mem = mmapped_mem;
  إذا ((بیعلامت دراز)(mmapped_mem + sbrked_mem) > (بیعلامت دراز)max_مجموع_mem)
    max_مجموع_mem = mmapped_mem + sbrked_mem;
  أرجع p;
}

#إذا __STD_C
ساكن فراغ munmap_chunk(mchunkptr p)
#وإلا
ساكن فراغ munmap_chunk(p) mchunkptr p;
#پایان_اگر
{
  داخلی_اندازه_T اندازه = chunksize(p);
  صحيح ret;

  أكد (chunk_is_mmapped(p));
  أكد(! ((حرف*)p >= sbrk_پایه && (حرف*)p < sbrk_پایه + sbrked_mem));
  أكد((n_mmaps > 0));
  أكد(((p->prev_اندازه + اندازه) & (malloc_getpagesize-1)) == 0);

  n_mmaps--;
  mmapped_mem -= (اندازه + p->prev_اندازه);

  ret = munmap((حرف *)p - p->prev_اندازه, اندازه + p->prev_اندازه);

  /* munmap returns non-zero on failure */
  أكد(ret == 0);
}

#إذا HAVE_MREMAP

#إذا __STD_C
ساكن mchunkptr mremap_chunk(mchunkptr p, size_t new_اندازه)
#وإلا
ساكن mchunkptr mremap_chunk(p, new_اندازه) mchunkptr p; size_t new_اندازه;
#پایان_اگر
{
  size_t صفحه_ماسک = malloc_getpagesize - 1;
  داخلی_اندازه_T جابه_جایی = p->prev_اندازه;
  داخلی_اندازه_T اندازه = chunksize(p);
  حرف *cp;

  أكد (chunk_is_mmapped(p));
  أكد(! ((حرف*)p >= sbrk_پایه && (حرف*)p < sbrk_پایه + sbrked_mem));
  أكد((n_mmaps > 0));
  أكد(((اندازه + جابه_جایی) & (malloc_getpagesize-1)) == 0);

  /* Note the extra SIZE_SZ overhead as in mmap_chunk(). */
  new_اندازه = (new_اندازه + جابه_جایی + اندازه_SZ + صفحه_ماسک) & ~صفحه_ماسک;

  cp = (حرف *)mremap((حرف *)p - جابه_جایی, اندازه + جابه_جایی, new_اندازه, 1);

  إذا (cp == (حرف *)-1) أرجع 0;

  p = (mchunkptr)(cp + جابه_جایی);

  أكد(aligned_OK(chunk2mem(p)));

  أكد((p->prev_اندازه == جابه_جایی));
  مجموعه_سر(p, (new_اندازه - جابه_جایی)|IS_MMAPPED);

  mmapped_mem -= اندازه + جابه_جایی;
  mmapped_mem += new_اندازه;
  إذا ((بیعلامت دراز)mmapped_mem > (بیعلامت دراز)max_mmapped_mem)
    max_mmapped_mem = mmapped_mem;
  إذا ((بیعلامت دراز)(mmapped_mem + sbrked_mem) > (بیعلامت دراز)max_مجموع_mem)
    max_مجموع_mem = mmapped_mem + sbrked_mem;
  أرجع p;
}

#پایان_اگر /* HAVE_MREMAP */

#پایان_اگر /* HAVE_MMAP */

/*
  Extend the top-most chunk by obtaining memory from system.
  Main interface to sbrk (but see also malloc_trim).
*/

#إذا __STD_C
ساكن فراغ malloc_extend_بالاترین(داخلی_اندازه_T nb)
#وإلا
ساكن فراغ malloc_extend_بالاترین(nb) داخلی_اندازه_T nb;
#پایان_اگر
{
  حرف*     brk;                  /* return value from sbrk */
  داخلی_اندازه_T front_misalign; /* unusable bytes at front of sbrked space */
  داخلی_اندازه_T correction;     /* bytes for 2nd sbrk call */
  حرف*     new_brk;              /* return of 2nd sbrk call */
  داخلی_اندازه_T بالاترین_اندازه;       /* new size of top chunk */

  mchunkptr old_بالاترین     = بالاترین;  /* Record state of old top */
  داخلی_اندازه_T old_بالاترین_اندازه = chunksize(old_بالاترین);
  حرف*     old_انتها      = (حرف*)(chunk_at_جابه_جایی(old_بالاترین, old_بالاترین_اندازه));

  /* Pad request with top_pad plus minimal overhead */

  داخلی_اندازه_T    sbrk_اندازه     = nb + بالاترین_pad + MINSIZE;
  بیعلامت دراز pagesz    = malloc_getpagesize;

  /* If not the first time through, round to preserve page boundary */
  /* Otherwise, we need to correct to a page size below anyway. */
  /* (We also correct below if an intervening foreign sbrk call.) */

  إذا (sbrk_پایه != (حرف*)(-1))
    sbrk_اندازه = (sbrk_اندازه + (pagesz - 1)) & ~(pagesz - 1);

  brk = (حرف*)(MORECORE (sbrk_اندازه));

  /* Fail if sbrk failed or if a foreign sbrk call killed our space */
  إذا (brk == (حرف*)(MORECORE_FAILURE) ||
      (brk < old_انتها && old_بالاترین != initial_بالاترین))
    أرجع;

  sbrked_mem += sbrk_اندازه;

  إذا (brk == old_انتها) /* can just add bytes to current top */
  {
    بالاترین_اندازه = sbrk_اندازه + old_بالاترین_اندازه;
    مجموعه_سر(بالاترین, بالاترین_اندازه | PREV_INUSE);
  }
  وإلا
  {
    إذا (sbrk_پایه == (حرف*)(-1))  /* First time through. Record base */
      sbrk_پایه = brk;
    وإلا  /* Someone else called sbrk().  Count those bytes as sbrked_mem. */
      sbrked_mem += brk - (حرف*)old_انتها;

    /* Guarantee alignment of first new chunk made from this space */
    front_misalign = (بیعلامت دراز)chunk2mem(brk) & MALLOC_ALIGN_ماسک;
    إذا (front_misalign > 0)
    {
      correction = (MALLOC_ALIGNMENT) - front_misalign;
      brk += correction;
    }
    وإلا
      correction = 0;

    /* Guarantee the next brk will be at a page boundary */

    correction += ((((بیعلامت دراز)(brk + sbrk_اندازه))+(pagesz-1)) &
		   ~(pagesz - 1)) - ((بیعلامت دراز)(brk + sbrk_اندازه));

    /* Allocate correction */
    new_brk = (حرف*)(MORECORE (correction));
    إذا (new_brk == (حرف*)(MORECORE_FAILURE)) أرجع;

    sbrked_mem += correction;

    بالاترین = (mchunkptr)brk;
    بالاترین_اندازه = new_brk - brk + correction;
    مجموعه_سر(بالاترین, بالاترین_اندازه | PREV_INUSE);

    إذا (old_بالاترین != initial_بالاترین)
    {

      /* There must have been an intervening foreign sbrk call. */
      /* A double fencepost is necessary to prevent consolidation */

      /* If not enough space to do this, then user did something very wrong */
      إذا (old_بالاترین_اندازه < MINSIZE)
      {
	مجموعه_سر(بالاترین, PREV_INUSE); /* will force null return from malloc */
	أرجع;
      }

      /* Also keep size a multiple of MALLOC_ALIGNMENT */
      old_بالاترین_اندازه = (old_بالاترین_اندازه - 3*اندازه_SZ) & ~MALLOC_ALIGN_ماسک;
      مجموعه_سر_اندازه(old_بالاترین, old_بالاترین_اندازه);
      chunk_at_جابه_جایی(old_بالاترین, old_بالاترین_اندازه          )->اندازه =
	اندازه_SZ|PREV_INUSE;
      chunk_at_جابه_جایی(old_بالاترین, old_بالاترین_اندازه + اندازه_SZ)->اندازه =
	اندازه_SZ|PREV_INUSE;
      /* If possible, release the rest. */
      إذا (old_بالاترین_اندازه >= MINSIZE)
	fREe(chunk2mem(old_بالاترین));
    }
  }

  إذا ((بیعلامت دراز)sbrked_mem > (بیعلامت دراز)max_sbrked_mem)
    max_sbrked_mem = sbrked_mem;
  إذا ((بیعلامت دراز)(mmapped_mem + sbrked_mem) > (بیعلامت دراز)max_مجموع_mem)
    max_مجموع_mem = mmapped_mem + sbrked_mem;

  /* We always land on a page boundary */
  أكد(((بیعلامت دراز)((حرف*)بالاترین + بالاترین_اندازه) & (pagesz - 1)) == 0);
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
Void_t* mALLOc_impl(size_t بایتها)
#وإلا
Void_t* mALLOc_impl(بایتها) size_t بایتها;
#پایان_اگر
{
  mchunkptr victim;                  /* inspected/selected chunk */
  داخلی_اندازه_T victim_اندازه;       /* its size */
  صحيح       idx;                     /* index for bin traversal */
  mbinptr   bin;                     /* associated bin */
  mchunkptr remainder;               /* remainder from a split */
  دراز      remainder_اندازه;          /* its size */
  صحيح       remainder_شاخص;         /* its bin index */
  بیعلامت دراز بلوک;               /* block traverser bit */
  صحيح       startidx;                /* first bin of a traversed block */
  mchunkptr fwd;                     /* misc temp for linking */
  mchunkptr bck;                     /* misc temp for linking */
  mbinptr q;                         /* misc temp */

  داخلی_اندازه_T nb;

#إذا CONFIG_IS_ENABLED(SYS_MALLOC_F)
	إذا (!(gd->flags & GD_FLG_پر_MALLOC_INIT))
		أرجع malloc_simple(بایتها);
#پایان_اگر

  إذا (CONFIG_IS_ENABLED(UNIT_TEST) && malloc_testing) {
    إذا (--malloc_max_allocs < 0)
      أرجع عدم;
  }

  /* check if mem_malloc_init() was run */
  إذا ((mem_malloc_شروع == 0) && (mem_malloc_انتها == 0)) {
    /* not initialized yet */
    أرجع عدم;
  }

  إذا (بایتها > CONFIG_SYS_MALLOC_LEN || (دراز)بایتها < 0)
     أرجع عدم;

  nb = request2size(بایتها);  /* padded request size; */

  /* Check for exact match in a bin */

  إذا (is_small_request(nb))  /* Faster version for small requests */
  {
    idx = smallbin_شاخص(nb);

    /* No traversal or size check necessary for small bins.  */

    q = bin_at(idx);
    victim = آخرین(q);

    /* Also scan the next one, since it would have a remainder < MINSIZE */
    إذا (victim == q)
    {
      q = بعدی_bin(q);
      victim = آخرین(q);
    }
    إذا (victim != q)
    {
      victim_اندازه = chunksize(victim);
      unlink(victim, bck, fwd);
      مجموعه_inuse_بیت_at_جابه_جایی(victim, victim_اندازه);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_بلوک(chunk2mem(victim), بایتها, اندازه_SZ, false);
      أرجع chunk2mem(victim);
    }

    idx += 2; /* Set for bin scan below. We've already scanned 2 bins. */

  }
  وإلا
  {
    idx = bin_شاخص(nb);
    bin = bin_at(idx);

    لكل (victim = آخرین(bin); victim != bin; victim = victim->bk)
    {
      victim_اندازه = chunksize(victim);
      remainder_اندازه = victim_اندازه - nb;

      إذا (remainder_اندازه >= (دراز)MINSIZE) /* too big */
      {
	--idx; /* adjust to rescan below after checking last remainder */
	اكسر;
      }

      وإلا إذا (remainder_اندازه >= 0) /* exact fit */
      {
	unlink(victim, bck, fwd);
	مجموعه_inuse_بیت_at_جابه_جایی(victim, victim_اندازه);
	check_malloced_chunk(victim, nb);
        VALGRIND_MALLOCLIKE_بلوک(chunk2mem(victim), بایتها, اندازه_SZ, false);
	أرجع chunk2mem(victim);
      }
    }

    ++idx;

  }

  /* Try to use the last split-off remainder */

  إذا ( (victim = آخرین_remainder->fd) != آخرین_remainder)
  {
    victim_اندازه = chunksize(victim);
    remainder_اندازه = victim_اندازه - nb;

    إذا (remainder_اندازه >= (دراز)MINSIZE) /* re-split */
    {
      remainder = chunk_at_جابه_جایی(victim, nb);
      مجموعه_سر(victim, nb | PREV_INUSE);
      link_آخرین_remainder(remainder);
      مجموعه_سر(remainder, remainder_اندازه | PREV_INUSE);
      مجموعه_foot(remainder, remainder_اندازه);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_بلوک(chunk2mem(victim), بایتها, اندازه_SZ, false);
      أرجع chunk2mem(victim);
    }

    clear_آخرین_remainder;

    إذا (remainder_اندازه >= 0)  /* exhaust */
    {
      مجموعه_inuse_بیت_at_جابه_جایی(victim, victim_اندازه);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_بلوک(chunk2mem(victim), بایتها, اندازه_SZ, false);
      أرجع chunk2mem(victim);
    }

    /* Else place in bin */

    frontlink(victim, victim_اندازه, remainder_شاخص, bck, fwd);
  }

  /*
     If there are any possibly nonempty big-enough blocks,
     search for best fitting chunk by scanning bins in blockwidth units.
  */

  إذا ( (بلوک = idx2binblock(idx)) <= binblocks_r)
  {

    /* Get to the first marked block */

    إذا ( (بلوک & binblocks_r) == 0)
    {
      /* force to an even block boundary */
      idx = (idx & ~(BINBLOCKWIDTH - 1)) + BINBLOCKWIDTH;
      بلوک <<= 1;
      طالما ((بلوک & binblocks_r) == 0)
      {
	idx += BINBLOCKWIDTH;
	بلوک <<= 1;
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

	لكل (victim = آخرین(bin); victim != bin; victim = victim->bk)
	{
	  victim_اندازه = chunksize(victim);
	  remainder_اندازه = victim_اندازه - nb;

	  إذا (remainder_اندازه >= (دراز)MINSIZE) /*تقسیم*/
	  {
	    remainder = chunk_at_جابه_جایی(victim, nb);
	    مجموعه_سر(victim, nb | PREV_INUSE);
	    unlink(victim, bck, fwd);
	    link_آخرین_remainder(remainder);
	    مجموعه_سر(remainder, remainder_اندازه | PREV_INUSE);
	    مجموعه_foot(remainder, remainder_اندازه);
	    check_malloced_chunk(victim, nb);
	    VALGRIND_MALLOCLIKE_بلوک(chunk2mem(victim), بایتها, اندازه_SZ, false);
	    أرجع chunk2mem(victim);
	  }

	  وإلا إذا (remainder_اندازه >= 0)  /* take */
	  {
	    مجموعه_inuse_بیت_at_جابه_جایی(victim, victim_اندازه);
	    unlink(victim, bck, fwd);
	    check_malloced_chunk(victim, nb);
	    VALGRIND_MALLOCLIKE_بلوک(chunk2mem(victim), بایتها, اندازه_SZ, false);
	    أرجع chunk2mem(victim);
	  }

	}

       bin = بعدی_bin(bin);

      } طالما ((++idx & (BINBLOCKWIDTH - 1)) != 0);

      /* Clear out the block bit. */

      افعل   /* Possibly backtrack to try to clear a partial block */
      {
	إذا ((startidx & (BINBLOCKWIDTH - 1)) == 0)
	{
	  av_[1] = (mbinptr)(binblocks_r & ~بلوک);
	  اكسر;
	}
	--startidx;
       q = prev_bin(q);
      } طالما (اولی(q) == q);

      /* Get to the next possibly nonempty block */

      إذا ( (بلوک <<= 1) <= binblocks_r && (بلوک != 0) )
      {
	طالما ((بلوک & binblocks_r) == 0)
	{
	  idx += BINBLOCKWIDTH;
	  بلوک <<= 1;
	}
      }
      وإلا
	اكسر;
    }
  }

  /* Try to use top chunk */

  /* Require that there be a remainder, ensuring top always exists  */
  إذا ( (remainder_اندازه = chunksize(بالاترین) - nb) < (دراز)MINSIZE)
  {

#إذا HAVE_MMAP
    /* If big and would otherwise need to extend, try to use mmap instead */
    إذا ((بیعلامت دراز)nb >= (بیعلامت دراز)mmap_آستانه &&
	(victim = mmap_chunk(nb)))
      VALGRIND_MALLOCLIKE_بلوک(chunk2mem(victim), بایتها, اندازه_SZ, false);
      أرجع chunk2mem(victim);
#پایان_اگر

    /* Try to extend */
    malloc_extend_بالاترین(nb);
    إذا ( (remainder_اندازه = chunksize(بالاترین) - nb) < (دراز)MINSIZE)
      أرجع عدم; /* propagate failure */
  }

  victim = بالاترین;
  مجموعه_سر(victim, nb | PREV_INUSE);
  بالاترین = chunk_at_جابه_جایی(victim, nb);
  مجموعه_سر(بالاترین, remainder_اندازه | PREV_INUSE);
  check_malloced_chunk(victim, nb);
  VALGRIND_MALLOCLIKE_بلوک(chunk2mem(victim), بایتها, اندازه_SZ, false);
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
#پایان_اگر
{
  mchunkptr p;         /* chunk corresponding to mem */
  داخلی_اندازه_T hd;  /* its head field */
  داخلی_اندازه_T sz;  /* its size */
  صحيح       idx;       /* its bin index */
  mchunkptr بعدی;      /* next contiguous chunk */
  داخلی_اندازه_T nextsz; /* its size */
  داخلی_اندازه_T prevsz; /* size of previous contiguous chunk */
  mchunkptr bck;       /* misc temp for linking */
  mchunkptr fwd;       /* misc temp for linking */
  صحيح       islr;      /* track whether merging with last_remainder */

#إذا CONFIG_IS_ENABLED(SYS_MALLOC_F)
	/* free() is a no-op - all the memory will be freed on relocation */
	إذا (!(gd->flags & GD_FLG_پر_MALLOC_INIT)) {
		VALGRIND_FREELIKE_بلوک(mem, اندازه_SZ);
		أرجع;
	}
#پایان_اگر

  إذا (mem == عدم)                              /* free(0) has no effect */
    أرجع;

  p = mem2chunk(mem);
  hd = p->اندازه;

#إذا HAVE_MMAP
  إذا (hd & IS_MMAPPED)                       /* release mmapped memory. */
  {
    munmap_chunk(p);
    أرجع;
  }
#پایان_اگر

  check_inuse_chunk(p);

  sz = hd & ~PREV_INUSE;
  بعدی = chunk_at_جابه_جایی(p, sz);
  nextsz = chunksize(بعدی);
  VALGRIND_FREELIKE_بلوک(mem, اندازه_SZ);

  إذا (بعدی == بالاترین)                            /* merge with top */
  {
    sz += nextsz;

    إذا (!(hd & PREV_INUSE))                    /* consolidate backward */
    {
      prevsz = p->prev_اندازه;
      p = chunk_at_جابه_جایی(p, -((دراز) prevsz));
      sz += prevsz;
      unlink(p, bck, fwd);
    }

    مجموعه_سر(p, sz | PREV_INUSE);
    بالاترین = p;
    إذا ((بیعلامت دراز)(sz) >= (بیعلامت دراز)trim_آستانه)
      malloc_trim(بالاترین_pad);
    أرجع;
  }

  مجموعه_سر(بعدی, nextsz);                    /* clear inuse bit */

  islr = 0;

  إذا (!(hd & PREV_INUSE))                    /* consolidate backward */
  {
    prevsz = p->prev_اندازه;
    p = chunk_at_جابه_جایی(p, -((دراز) prevsz));
    sz += prevsz;

    إذا (p->fd == آخرین_remainder)             /* keep as last_remainder */
      islr = 1;
    وإلا
      unlink(p, bck, fwd);
  }

  إذا (!(inuse_بیت_at_جابه_جایی(بعدی, nextsz)))   /* consolidate forward */
  {
    sz += nextsz;

    إذا (!islr && بعدی->fd == آخرین_remainder)  /* re-insert last_remainder */
    {
      islr = 1;
      link_آخرین_remainder(p);
    }
    وإلا
      unlink(بعدی, bck, fwd);
  }

  مجموعه_سر(p, sz | PREV_INUSE);
  مجموعه_foot(p, sz);
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
Void_t* rEALLOc_impl(Void_t* oldmem, size_t بایتها)
#وإلا
Void_t* rEALLOc_impl(oldmem, بایتها) Void_t* oldmem; size_t بایتها;
#پایان_اگر
{
  داخلی_اندازه_T    nb;      /* padded request size */

  mchunkptr oldp;             /* chunk corresponding to oldmem */
  داخلی_اندازه_T    oldsize; /* its size */

  mchunkptr newp;             /* chunk to return */
  داخلی_اندازه_T    newsize; /* its size */
  Void_t*   newmem;           /* corresponding user mem */

  mchunkptr بعدی;             /* next contiguous chunk after oldp */
  داخلی_اندازه_T  nextsize;  /* its size */

  mchunkptr prev;             /* previous contiguous chunk before oldp */
  داخلی_اندازه_T  prevsize;  /* its size */

  mchunkptr remainder;        /* holds split off extra space from newp */
  داخلی_اندازه_T  remainder_اندازه;   /* its size */

  mchunkptr bck;              /* misc temp for linking */
  mchunkptr fwd;              /* misc temp for linking */

#اگر_تعریف_شده REALLOC_ZERO_بایتها_FREES
  إذا (!بایتها) {
	fREe_impl(oldmem);
	أرجع عدم;
  }
#پایان_اگر

  إذا (بایتها > CONFIG_SYS_MALLOC_LEN || (دراز)بایتها < 0)
     أرجع عدم;

  /* realloc of null is supposed to be same as malloc */
  إذا (oldmem == عدم) أرجع mALLOc_impl(بایتها);

#إذا CONFIG_IS_ENABLED(SYS_MALLOC_F)
	إذا (!(gd->flags & GD_FLG_پر_MALLOC_INIT)) {
		/* This is harder to support and should not be needed */
		panic("pre-reloc realloc() is not supported");
	}
#پایان_اگر
  إذا (CONFIG_IS_ENABLED(UNIT_TEST) && malloc_testing) {
    إذا (--malloc_max_allocs < 0)
      أرجع عدم;
  }

  newp    = oldp    = mem2chunk(oldmem);
  newsize = oldsize = chunksize(oldp);

  nb = request2size(بایتها);

#إذا HAVE_MMAP
  إذا (chunk_is_mmapped(oldp))
  {
#إذا HAVE_MREMAP
    newp = mremap_chunk(oldp, nb);
    إذا(newp) أرجع chunk2mem(newp);
#پایان_اگر
    /* Note the extra SIZE_SZ overhead. */
    إذا(oldsize - اندازه_SZ >= nb) أرجع oldmem; /* do nothing */
    /* Must alloc, copy, free. */
    newmem = mALLOc_impl(بایتها);
    إذا (!newmem)
	أرجع عدم; /* propagate failure */
    MALLOC_COPY(newmem, oldmem, oldsize - 2*اندازه_SZ);
    munmap_chunk(oldp);
    أرجع newmem;
  }
#پایان_اگر

  check_inuse_chunk(oldp);

  إذا ((دراز)(oldsize) < (دراز)(nb))
  {

    /* Try expanding forward */

    بعدی = chunk_at_جابه_جایی(oldp, oldsize);
    إذا (بعدی == بالاترین || !inuse(بعدی))
    {
      nextsize = chunksize(بعدی);

      /* Forward into top only if a remainder */
      إذا (بعدی == بالاترین)
      {
	إذا ((دراز)(nextsize + newsize) >= (دراز)(nb + MINSIZE))
	{
	  newsize += nextsize;
	  بالاترین = chunk_at_جابه_جایی(oldp, nb);
	  مجموعه_سر(بالاترین, (newsize - nb) | PREV_INUSE);
	  مجموعه_سر_اندازه(oldp, nb);
	  VALGRIND_RESIZEINPLACE_بلوک(chunk2mem(oldp), 0, بایتها, اندازه_SZ);
	  VALGRIND_MAKE_MEM_DEFINED(chunk2mem(oldp), بایتها);
	  أرجع chunk2mem(oldp);
	}
      }

      /* Forward into next chunk */
      وإلا إذا (((دراز)(nextsize + newsize) >= (دراز)(nb)))
      {
	unlink(بعدی, bck, fwd);
	newsize  += nextsize;
	VALGRIND_RESIZEINPLACE_بلوک(chunk2mem(oldp), 0, بایتها, اندازه_SZ);
	VALGRIND_MAKE_MEM_DEFINED(chunk2mem(oldp), بایتها);
	اذهب تقسیم;
      }
    }
    وإلا
    {
      بعدی = عدم;
      nextsize = 0;
    }

    /* Try shifting backwards. */

    إذا (!prev_inuse(oldp))
    {
      prev = prev_chunk(oldp);
      prevsize = chunksize(prev);

      /* try forward + backward first to save a later consolidation */

      إذا (بعدی != عدم)
      {
	/* into top */
	إذا (بعدی == بالاترین)
	{
	  إذا ((دراز)(nextsize + prevsize + newsize) >= (دراز)(nb + MINSIZE))
	  {
	    unlink(prev, bck, fwd);
	    newp = prev;
	    newsize += prevsize + nextsize;
	    newmem = chunk2mem(newp);
	    VALGRIND_MALLOCLIKE_بلوک(newmem, بایتها, اندازه_SZ, false);
	    MALLOC_COPY(newmem, oldmem, oldsize - اندازه_SZ);
	    بالاترین = chunk_at_جابه_جایی(newp, nb);
	    مجموعه_سر(بالاترین, (newsize - nb) | PREV_INUSE);
	    مجموعه_سر_اندازه(newp, nb);
	    VALGRIND_FREELIKE_بلوک(oldmem, اندازه_SZ);
	    أرجع newmem;
	  }
	}

	/* into next chunk */
	وإلا إذا (((دراز)(nextsize + prevsize + newsize) >= (دراز)(nb)))
	{
	  unlink(بعدی, bck, fwd);
	  unlink(prev, bck, fwd);
	  newp = prev;
	  newsize += nextsize + prevsize;
	  newmem = chunk2mem(newp);
	  VALGRIND_MALLOCLIKE_بلوک(newmem, بایتها, اندازه_SZ, false);
	  MALLOC_COPY(newmem, oldmem, oldsize - اندازه_SZ);
	  اذهب تقسیم;
	}
      }

      /* backward only */
      إذا (prev != عدم && (دراز)(prevsize + newsize) >= (دراز)nb)
      {
	unlink(prev, bck, fwd);
	newp = prev;
	newsize += prevsize;
	newmem = chunk2mem(newp);
	VALGRIND_MALLOCLIKE_بلوک(newmem, بایتها, اندازه_SZ, false);
	MALLOC_COPY(newmem, oldmem, oldsize - اندازه_SZ);
	اذهب تقسیم;
      }
    }

    /* Must allocate */

    newmem = mALLOc_impl (بایتها);

    إذا (newmem == عدم)  /* propagate failure */
      أرجع عدم;

    /* Avoid copy if newp is next chunk after oldp. */
    /* (This can only happen when new chunk is sbrk'ed.) */

    إذا ( (newp = mem2chunk(newmem)) == بعدی_chunk(oldp))
    {
      newsize += chunksize(newp);
      newp = oldp;
      اذهب تقسیم;
    }

    /* Otherwise copy, free, and exit */
    MALLOC_COPY(newmem, oldmem, oldsize - اندازه_SZ);
    fREe_impl(oldmem);
    أرجع newmem;
  } وإلا {
    VALGRIND_RESIZEINPLACE_بلوک(oldmem, 0, بایتها, اندازه_SZ);
    VALGRIND_MAKE_MEM_DEFINED(oldmem, بایتها);
  }

 تقسیم:  /* split off extra room in old or expanded chunk */

  إذا (newsize - nb >= MINSIZE) /* split off remainder */
  {
    remainder = chunk_at_جابه_جایی(newp, nb);
    remainder_اندازه = newsize - nb;
    مجموعه_سر_اندازه(newp, nb);
    مجموعه_سر(remainder, remainder_اندازه | PREV_INUSE);
    مجموعه_inuse_بیت_at_جابه_جایی(remainder, remainder_اندازه);
    VALGRIND_MALLOCLIKE_بلوک(chunk2mem(remainder), remainder_اندازه, اندازه_SZ,
			      false);
    fREe_impl(chunk2mem(remainder)); /* let free() deal with it */
  }
  وإلا
  {
    مجموعه_سر_اندازه(newp, newsize);
    مجموعه_inuse_بیت_at_جابه_جایی(newp, newsize);
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
Void_t* mEMALIGn_impl(size_t alignment, size_t بایتها)
#وإلا
Void_t* mEMALIGn_impl(alignment, بایتها) size_t alignment; size_t بایتها;
#پایان_اگر
{
  داخلی_اندازه_T    nb;      /* padded  request size */
  حرف*     m;                /* memory returned by malloc call */
  mchunkptr p;                /* corresponding chunk */
  حرف*     brk;              /* alignment point within p */
  mchunkptr newp;             /* chunk to return */
  داخلی_اندازه_T  newsize;   /* its size */
  داخلی_اندازه_T  leadsize;  /* leading space befor alignment point */
  mchunkptr remainder;        /* spare room at end to split off */
  دراز      remainder_اندازه;   /* its size */

  إذا (بایتها > CONFIG_SYS_MALLOC_LEN || (دراز)بایتها < 0)
     أرجع عدم;

#إذا CONFIG_IS_ENABLED(SYS_MALLOC_F)
	إذا (!(gd->flags & GD_FLG_پر_MALLOC_INIT)) {
		أرجع memalign_simple(alignment, بایتها);
	}
#پایان_اگر

  /* If need less alignment than we give anyway, just relay to malloc */

  إذا (alignment <= MALLOC_ALIGNMENT) أرجع mALLOc_impl(بایتها);

  /* Otherwise, ensure that it is at least a minimum chunk size */

  إذا (alignment <  MINSIZE) alignment = MINSIZE;

  /* Call malloc with worst case padding to hit alignment. */

  nb = request2size(بایتها);
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
    m  = (حرف*)(mALLOc_impl(بایتها));
    /* Aligned -> return it */
    إذا ((((بیعلامت دراز)(m)) % alignment) == 0)
      أرجع m;
    /*
     * Otherwise, try again, requesting enough extra space to be able to
     * acquire alignment.
     */
    fREe_impl(m);
    /* Add in extra bytes to match misalignment of unexpanded allocation */
    extra = alignment - (((بیعلامت دراز)(m)) % alignment);
    m  = (حرف*)(mALLOc_impl(بایتها + extra));
    /*
     * m might not be the same as before. Validate that the previous value of
     * extra still works for the current value of m.
     * If (!m), extra2=alignment so 
     */
    إذا (m) {
      extra2 = alignment - (((بیعلامت دراز)(m)) % alignment);
      إذا (extra2 > extra) {
        fREe_impl(m);
        m = عدم;
      }
    }
    /* Fall through to original NULL check and chunk splitting logic */
  }

  إذا (m == عدم) أرجع عدم; /* propagate failure */

  p = mem2chunk(m);

  إذا ((((بیعلامت دراز)(m)) % alignment) == 0) /* aligned */
  {
#إذا HAVE_MMAP
    إذا(chunk_is_mmapped(p))
      أرجع chunk2mem(p); /* nothing more to do */
#پایان_اگر
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

    brk = (حرف*)mem2chunk(((بیعلامت دراز)(m + alignment - 1)) & -((علامتدار) alignment));
    إذا ((دراز)(brk - (حرف*)(p)) < MINSIZE) brk = brk + alignment;

    newp = (mchunkptr)brk;
    leadsize = brk - (حرف*)(p);
    newsize = chunksize(p) - leadsize;

#إذا HAVE_MMAP
    إذا(chunk_is_mmapped(p))
    {
      newp->prev_اندازه = p->prev_اندازه + leadsize;
      مجموعه_سر(newp, newsize|IS_MMAPPED);
      أرجع chunk2mem(newp);
    }
#پایان_اگر

    /* give back leader, use the rest */

    مجموعه_سر(newp, newsize | PREV_INUSE);
    مجموعه_inuse_بیت_at_جابه_جایی(newp, newsize);
    مجموعه_سر_اندازه(p, leadsize);
    fREe_impl(chunk2mem(p));
    p = newp;
    VALGRIND_MALLOCLIKE_بلوک(chunk2mem(p), بایتها, اندازه_SZ, false);

    أكد (newsize >= nb && (((بیعلامت دراز)(chunk2mem(p))) % alignment) == 0);
  }

  /* Also give back spare room at the end */

  remainder_اندازه = chunksize(p) - nb;

  إذا (remainder_اندازه >= (دراز)MINSIZE)
  {
    remainder = chunk_at_جابه_جایی(p, nb);
    مجموعه_سر(remainder, remainder_اندازه | PREV_INUSE);
    مجموعه_سر_اندازه(p, nb);
    VALGRIND_MALLOCLIKE_بلوک(chunk2mem(remainder), remainder_اندازه, اندازه_SZ,
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
Void_t* vALLOc(size_t بایتها)
#وإلا
Void_t* vALLOc(بایتها) size_t بایتها;
#پایان_اگر
{
  أرجع mEMALIGn (malloc_getpagesize, بایتها);
}

/*
  pvalloc just invokes valloc for the nearest pagesize
  that will accommodate request
*/

#إذا __STD_C
Void_t* pvALLOc(size_t بایتها)
#وإلا
Void_t* pvALLOc(بایتها) size_t بایتها;
#پایان_اگر
{
  size_t pagesize = malloc_getpagesize;
  أرجع mEMALIGn (pagesize, (بایتها + pagesize - 1) & ~(pagesize - 1));
}

/*

  calloc calls malloc, then zeroes out the allocated chunk.

*/

STATIC_IF_MCHECK
#إذا __STD_C
Void_t* cALLOc_impl(size_t n, size_t elem_اندازه)
#وإلا
Void_t* cALLOc_impl(n, elem_اندازه) size_t n; size_t elem_اندازه;
#پایان_اگر
{
  mchunkptr p;
  داخلی_اندازه_T csz;

  داخلی_اندازه_T sz = n * elem_اندازه;

  /* check if expand_top called, in which case don't need to clear */
#إذا CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
#إذا MORECORE_CLEARS
  mchunkptr oldtop = بالاترین;
  داخلی_اندازه_T oldtopsize = chunksize(بالاترین);
#پایان_اگر
#پایان_اگر
  Void_t* mem = mALLOc_impl (sz);

  إذا ((دراز)n < 0) أرجع عدم;

  إذا (mem == عدم)
    أرجع عدم;
  وإلا
  {
#إذا CONFIG_IS_ENABLED(SYS_MALLOC_F)
	إذا (!(gd->flags & GD_FLG_پر_MALLOC_INIT)) {
		memset(mem, 0, sz);
		أرجع mem;
	}
#پایان_اگر
    p = mem2chunk(mem);

    /* Two optional cases in which clearing not necessary */

#إذا HAVE_MMAP
    إذا (chunk_is_mmapped(p)) أرجع mem;
#پایان_اگر

    csz = chunksize(p);

#إذا CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
#إذا MORECORE_CLEARS
    إذا (p == oldtop && csz > oldtopsize)
    {
      /* clear only the bytes from non-freshly-sbrked memory */
      csz = oldtopsize;
    }
#پایان_اگر
#پایان_اگر

    MALLOC_ZERO(mem, csz - اندازه_SZ);
    VALGRIND_MAKE_MEM_DEFINED(mem, sz);
    أرجع mem;
  }
}

/*

  cfree just calls free. It is needed/defined on some systems
  that pair it with calloc, presumably for odd historical reasons.

*/

#إذا !defined(داخلی_LINUX_C_LIB) || !defined(__ELF__)
#إذا __STD_C
فراغ cfree(Void_t *mem)
#وإلا
فراغ cfree(mem) Void_t *mem;
#پایان_اگر
{
  fREe(mem);
}
#پایان_اگر

#اگر_تعریف_شده MCHECK_انباشته_PROTECTION
 #ضمن "mcheck_core.inc.h"
 #إذا !__STD_C
  #خطا "must have __STD_C"
 #پایان_اگر

Void_t *mALLOc(size_t بایتها)
{
	mcheck_pedantic_prehook();
	size_t fullsz = mcheck_alloc_prehook(بایتها);
	فراغ *p = mALLOc_impl(fullsz);

	إذا (!p)
		أرجع p;
	أرجع mcheck_alloc_posthook(p, بایتها);
}

فراغ fREe(Void_t *mem) { fREe_impl(mcheck_free_prehook(mem)); }

Void_t *rEALLOc(Void_t *oldmem, size_t بایتها)
{
	mcheck_pedantic_prehook();
	إذا (بایتها == 0) {
		إذا (oldmem)
			fREe(oldmem);
		أرجع عدم;
	}

	إذا (oldmem == عدم)
		أرجع mALLOc(بایتها);

	فراغ *p = mcheck_reallocfree_prehook(oldmem);
	size_t newsz = mcheck_alloc_prehook(بایتها);

	p = rEALLOc_impl(p, newsz);
	إذا (!p)
		أرجع p;
	أرجع mcheck_alloc_noclean_posthook(p, بایتها);
}

Void_t *mEMALIGn(size_t alignment, size_t بایتها)
{
	mcheck_pedantic_prehook();
	size_t fullsz = mcheck_memalign_prehook(alignment, بایتها);
	فراغ *p = mEMALIGn_impl(alignment, fullsz);

	إذا (!p)
		أرجع p;
	أرجع mcheck_memalign_posthook(alignment, p, بایتها);
}

// pvALLOc, vALLOc - redirect to mEMALIGn, defined here, so they need no wrapping.

Void_t *cALLOc(size_t n, size_t elem_اندازه)
{
	mcheck_pedantic_prehook();
	// NB: here is no overflow check.
	size_t fullsz = mcheck_alloc_prehook(n * elem_اندازه);
	فراغ *p = cALLOc_impl(1, fullsz);

	إذا (!p)
		أرجع p;
	أرجع mcheck_alloc_noclean_posthook(p, n * elem_اندازه);
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
#پایان_اگر

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
#پایان_اگر
{
  دراز  بالاترین_اندازه;        /* Amount of top-most memory */
  دراز  extra;           /* Amount to release */
  حرف* current_brk;     /* address returned by pre-check sbrk call */
  حرف* new_brk;         /* address returned by negative sbrk call */

  بیعلامت دراز pagesz = malloc_getpagesize;

  بالاترین_اندازه = chunksize(بالاترین);
  extra = ((بالاترین_اندازه - pad - MINSIZE + (pagesz-1)) / pagesz - 1) * pagesz;

  إذا (extra < (دراز)pagesz)  /* Not enough memory to release */
    أرجع 0;

  وإلا
  {
    /* Test to make sure no one else called sbrk */
    current_brk = (حرف*)(MORECORE (0));
    إذا (current_brk != (حرف*)(بالاترین) + بالاترین_اندازه)
      أرجع 0;     /* Apparently we don't own memory; must fail */

    وإلا
    {
      new_brk = (حرف*)(MORECORE (-extra));

      إذا (new_brk == (حرف*)(MORECORE_FAILURE)) /* sbrk failed? */
      {
	/* Try to figure out what we have */
	current_brk = (حرف*)(MORECORE (0));
	بالاترین_اندازه = current_brk - (حرف*)بالاترین;
	إذا (بالاترین_اندازه >= (دراز)MINSIZE) /* if not, we are very very dead! */
	{
	  sbrked_mem = current_brk - sbrk_پایه;
	  مجموعه_سر(بالاترین, بالاترین_اندازه | PREV_INUSE);
	}
	check_chunk(بالاترین);
	أرجع 0;
      }

      وإلا
      {
	/* Success. Adjust top accordingly. */
	مجموعه_سر(بالاترین, (بالاترین_اندازه - extra) | PREV_INUSE);
	sbrked_mem -= extra;
	check_chunk(بالاترین);
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
#پایان_اگر
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
      أرجع chunksize(p) - اندازه_SZ;
    }
    أرجع chunksize(p) - 2*اندازه_SZ;
  }
}

/* Utility to update current_mallinfo for malloc_stats and mallinfo() */

#اگر_تعریف_شده DEBUG
ساكن فراغ malloc_update_mallinfo(فراغ)
{
  صحيح i;
  mbinptr b;
  mchunkptr p;
#اگر_تعریف_شده DEBUG
  mchunkptr q;
#پایان_اگر

  داخلی_اندازه_T avail = chunksize(بالاترین);
  صحيح   navail = ((دراز)(avail) >= (دراز)MINSIZE)? 1 : 0;

  لكل (i = 1; i < NAV; ++i)
  {
    b = bin_at(i);
    لكل (p = آخرین(b); p != b; p = p->bk)
    {
#اگر_تعریف_شده DEBUG
      check_free_chunk(p);
      لكل (q = بعدی_chunk(p);
	   q < بالاترین && inuse(q) && (دراز)(chunksize(q)) >= (دراز)MINSIZE;
	   q = بعدی_chunk(q))
	check_inuse_chunk(q);
#پایان_اگر
      avail += chunksize(p);
      navail++;
    }
  }

  current_mallinfo.ordblks = navail;
  current_mallinfo.uordblks = sbrked_mem - avail;
  current_mallinfo.fordblks = avail;
  current_mallinfo.hblks = n_mmaps;
  current_mallinfo.hblkhd = mmapped_mem;
  current_mallinfo.keepcost = chunksize(بالاترین);

}
#پایان_اگر	/* DEBUG */

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

#اگر_تعریف_شده DEBUG
فراغ malloc_stats(فراغ)
{
  malloc_update_mallinfo();
  چاپ_کن("max system bytes = %10u\n",
	  (بیعلامت صحيح)(max_مجموع_mem));
  چاپ_کن("system bytes     = %10u\n",
	  (بیعلامت صحيح)(sbrked_mem + mmapped_mem));
  چاپ_کن("in use bytes     = %10u\n",
	  (بیعلامت صحيح)(current_mallinfo.uordblks + mmapped_mem));
#إذا HAVE_MMAP
  چاپ_کن("max mmap regions = %10u\n",
	  (بیعلامت صحيح)max_n_mmaps);
#پایان_اگر
}
#پایان_اگر	/* DEBUG */

/*
  mallinfo returns a copy of updated current mallinfo.
*/

#اگر_تعریف_شده DEBUG
بنية mallinfo mALLINFo(فراغ)
{
  malloc_update_mallinfo();
  أرجع current_mallinfo;
}
#پایان_اگر	/* DEBUG */

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
صحيح mALLOPt(صحيح param_عدد, صحيح مقدار)
#وإلا
صحيح mALLOPt(param_عدد, مقدار) صحيح param_عدد; صحيح مقدار;
#پایان_اگر
{
  بدل(param_عدد)
  {
    حالة M_TRIM_THRESHOLD:
      trim_آستانه = مقدار; أرجع 1;
    حالة M_TOP_PAD:
      بالاترین_pad = مقدار; أرجع 1;
    حالة M_MMAP_THRESHOLD:
      mmap_آستانه = مقدار; أرجع 1;
    حالة M_MMAP_MAX:
#إذا HAVE_MMAP
      n_mmaps_max = مقدار; أرجع 1;
#وإلا
      إذا (مقدار != 0) أرجع 0; وإلا  n_mmaps_max = مقدار; أرجع 1;
#پایان_اگر

    افتراضي:
      أرجع 0;
  }
}

صحيح initf_malloc(فراغ)
{
#إذا CONFIG_IS_ENABLED(SYS_MALLOC_F)
	أكد(gd->malloc_پایه);	/* Set up by crt0.S */
	gd->malloc_حد = CONFIG_VAL(SYS_MALLOC_F_LEN);
	gd->malloc_ptr = 0;
#پایان_اگر

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
