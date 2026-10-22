// SPDX-License-Identifier: GPL-2.0+
/*
 * This code is based on a version (aka dlmalloc) of malloc/free/realloc written
 * by Doug Lea and released to the public domain, as explained at
 * http://creativecommons.org/publicdomain/zero/1.0/-
 *
 * The original code is available at http://gee.cs.oswego.edu/pub/misc/
 * as file malloc-2.6.6.c.
 */

#اگر CONFIG_IS_ENABLED(UNIT_TEST)
#تعریف_کرو DEBUG
#اگر_ختم

#شامل <log.h>
#شامل <asm/global_data.h>

#شامل <malloc.h>
#شامل <mapmem.h>
#شامل <string.h>
#شامل <asm/io.h>
#شامل <valgrind/memcheck.h>

#اگر_تعریف_شدہ DEBUG
#اگر __STD_C
ساکن خالی malloc_update_mallinfo (خالی);
خالی malloc_stats (خالی);
#ورنہ
ساکن خالی malloc_update_mallinfo ();
خالی malloc_stats();
#اگر_ختم
#اگر_ختم	/* DEBUG */

DECLARE_GLOBAL_ڈیٹا_PTR;

#اگر_تعریف_شدہ MCHECK_ڈھیر_PROTECTION
 #تعریف_کرو STATIC_IF_MCHECK ساکن
 #تعریف_منسوخ MALLOC_COPY
 #تعریف_منسوخ MALLOC_ZERO
ساکن اندرونی خالی MALLOC_ZERO(خالی *p, size_t sz) { memset(p, 0, sz); }
ساکن اندرونی خالی MALLOC_COPY(خالی *dest, ثابت خالی *src, size_t sz) { حافظہ_نقل(dest, src, sz); }
#ورنہ
 #تعریف_کرو STATIC_IF_MCHECK
 #تعریف_کرو mALLOc_impl mALLOc
 #تعریف_کرو fREe_impl fREe
 #تعریف_کرو rEALLOc_impl rEALLOc
 #تعریف_کرو mEMALIGn_impl mEMALIGn
 #تعریف_کرو cALLOc_impl cALLOc
#اگر_ختم

/*
  Emulation of sbrk for WIN32
  All code within the ifdef WIN32 is untested by me.

  Thanks to Martin Fong and others for supplying this.
*/

#اگر_تعریف_شدہ WIN32

#تعریف_کرو Align_صفحہ(جوڑو) (((جوڑو) + (malloc_getpagesize-1)) & \
~(malloc_getpagesize-1))
#تعریف_کرو AlignPage64K(جوڑو) (((جوڑو) + (0x10000 - 1)) & ~(0x10000 - 1))

/* resrve 64MB to insure large contiguous space */
#تعریف_کرو RESERVED_سائز (1024*1024*64)
#تعریف_کرو اگلا_سائز (2048*1024)
#تعریف_کرو چوٹی_حافظہ ((غیرعلامتی طویل)2*1024*1024*1024)

ساخت GmListElement;
قسمتعریف ساخت GmListElement GmListElement;

ساخت GmListElement
{
	GmListElement* اگلا;
	خالی* بنیاد;
};

ساکن GmListElement* سر = 0;
ساکن غیرعلامتی صحیح g_اگلا_پتہ = 0;
ساکن غیرعلامتی صحیح g_پتہ_بنیاد = 0;
ساکن غیرعلامتی صحیح g_Allocated_سائز = 0;

ساکن
GmListElement* makeGmListElement (خالی* bas)
{
	GmListElement* this;
	this = (GmListElement*)(خالی*)LocalAlloc (0, حجم (GmListElement));
	أكد (this);
	اگر (this)
	{
		this->بنیاد = bas;
		this->اگلا = سر;
		سر = this;
	}
	واپس this;
}

خالی gcleanup (خالی)
{
	BOOL rval;
	أكد ( (سر == تہی) || (سر->بنیاد == (خالی*)g_پتہ_بنیاد));
	اگر (g_پتہ_بنیاد && (g_اگلا_پتہ - g_پتہ_بنیاد))
	{
		rval = VirtualFree ((خالی*)g_پتہ_بنیاد,
							g_اگلا_پتہ - g_پتہ_بنیاد,
							MEM_DECOMMIT);
	أكد (rval);
	}
	جب (سر)
	{
		GmListElement* اگلا = سر->اگلا;
		rval = VirtualFree (سر->بنیاد, 0, MEM_RELEASE);
		أكد (rval);
		LocalFree (سر);
		سر = اگلا;
	}
}

ساکن
خالی* ڈھونڈو_Region (خالی* آغاز_پتہ, غیرعلامتی طویل سائز)
{
	حافظہ_BASIC_INFORMATION info;
	اگر (سائز >= چوٹی_حافظہ) واپس تہی;

	جب ((غیرعلامتی طویل)آغاز_پتہ + سائز < چوٹی_حافظہ)
	{
		VirtualQuery (آغاز_پتہ, &info, حجم (info));
		اگر ((info.State == MEM_FREE) && (info.Region_سائز >= سائز))
			واپس آغاز_پتہ;
		ورنہ
		{
			/* Requested region is not available so see if the */
			/* next region is available.  Set 'start_address' */
			/* to the next region and call 'VirtualQuery()' */
			/* again. */

			آغاز_پتہ = (حرف*)info.بنیاد_پتہ + info.Region_سائز;

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

			آغاز_پتہ =
				(خالی *) AlignPage64K((غیرعلامتی طویل) آغاز_پتہ);
		}
	}
	واپس تہی;

}

خالی* wsbrk (طویل سائز)
{
	خالی* tmp;
	اگر (سائز > 0)
	{
		اگر (g_پتہ_بنیاد == 0)
		{
			g_Allocated_سائز = max (RESERVED_سائز, Align_صفحہ (سائز));
			g_اگلا_پتہ = g_پتہ_بنیاد =
				(غیرعلامتی صحیح)VirtualAlloc (تہی, g_Allocated_سائز,
											MEM_RESERVE, صفحہ_NOACCESS);
		} ورنہ اگر (Align_صفحہ (g_اگلا_پتہ + سائز) > (g_پتہ_بنیاد +
g_Allocated_سائز))
		{
			طویل new_سائز = max (اگلا_سائز, Align_صفحہ (سائز));
			خالی* new_پتہ = (خالی*)(g_پتہ_بنیاد+g_Allocated_سائز);
			کرو
			{
				new_پتہ = ڈھونڈو_Region (new_پتہ, new_سائز);

				اگر (!new_پتہ)
					واپس (خالی*)-1;

				g_پتہ_بنیاد = g_اگلا_پتہ =
					(غیرعلامتی صحیح)VirtualAlloc (new_پتہ, new_سائز,
												MEM_RESERVE, صفحہ_NOACCESS);
				/* repeat in case of race condition */
				/* The region that we found has been snagged */
				/* by another thread */
			}
			جب (g_پتہ_بنیاد == 0);

			أكد (new_پتہ == (خالی*)g_پتہ_بنیاد);

			g_Allocated_سائز = new_سائز;

			اگر (!makeGmListElement ((خالی*)g_پتہ_بنیاد))
				واپس (خالی*)-1;
		}
		اگر ((سائز + g_اگلا_پتہ) > Align_صفحہ (g_اگلا_پتہ))
		{
			خالی* res;
			res = VirtualAlloc ((خالی*)Align_صفحہ (g_اگلا_پتہ),
								(سائز + g_اگلا_پتہ -
								 Align_صفحہ (g_اگلا_پتہ)),
								MEM_COMMIT, صفحہ_READWRITE);
			اگر (!res)
				واپس (خالی*)-1;
		}
		tmp = (خالی*)g_اگلا_پتہ;
		g_اگلا_پتہ = (غیرعلامتی صحیح)tmp + سائز;
		واپس tmp;
	}
	ورنہ اگر (سائز < 0)
	{
		غیرعلامتی صحیح alignedGoal = Align_صفحہ (g_اگلا_پتہ + سائز);
		/* Trim by releasing the virtual memory */
		اگر (alignedGoal >= g_پتہ_بنیاد)
		{
			VirtualFree ((خالی*)alignedGoal, g_اگلا_پتہ - alignedGoal,
						 MEM_DECOMMIT);
			g_اگلا_پتہ = g_اگلا_پتہ + سائز;
			واپس (خالی*)g_اگلا_پتہ;
		}
		ورنہ
		{
			VirtualFree ((خالی*)g_پتہ_بنیاد, g_اگلا_پتہ - g_پتہ_بنیاد,
						 MEM_DECOMMIT);
			g_اگلا_پتہ = g_پتہ_بنیاد;
			واپس (خالی*)-1;
		}
	}
	ورنہ
	{
		واپس (خالی*)g_اگلا_پتہ;
	}
}

#اگر_ختم

/*
  Type declarations
*/

ساخت malloc_chunk
{
  اندرونی_سائز_T پچھلا_سائز; /* Size of previous chunk (if free). */
  اندرونی_سائز_T سائز;      /* Size in bytes, including overhead. */
  ساخت malloc_chunk* fd;   /* double links -- used only if free. */
  ساخت malloc_chunk* bk;
} __attribute__((__may_alias__)) ;

قسمتعریف ساخت malloc_chunk* mchunkptr;

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

#تعریف_کرو سائز_SZ                (حجم(اندرونی_سائز_T))
#تعریف_کرو MALLOC_ALIGNMENT       (سائز_SZ + سائز_SZ)
#تعریف_کرو MALLOC_ALIGN_نقاب      (MALLOC_ALIGNMENT - 1)
#تعریف_کرو MINSIZE                (حجم(ساخت malloc_chunk))

/* conversion from malloc headers to user pointers, and back */

#تعریف_کرو chunk2mem(p)   ((Void_t*)((حرف*)(p) + 2*سائز_SZ))
#تعریف_کرو mem2chunk(mem) ((mchunkptr)((حرف*)(mem) - 2*سائز_SZ))

/* pad request bytes into a usable size */

#تعریف_کرو request2size(req) \
 ((((req) + (سائز_SZ + MALLOC_ALIGN_نقاب)) < \
  (MINSIZE + MALLOC_ALIGN_نقاب)) ? MINSIZE : \
   (((req) + (سائز_SZ + MALLOC_ALIGN_نقاب)) & ~(MALLOC_ALIGN_نقاب)))

/* Check if m has acceptable alignment */

#تعریف_کرو aligned_OK(m)    (((غیرعلامتی طویل)((m)) & (MALLOC_ALIGN_نقاب)) == 0)

/*
  Physical chunk operations
*/

/* size field is or'ed with PREV_INUSE when previous adjacent chunk in use */

#تعریف_کرو پچھلا_INUSE 0x1

/* size field is or'ed with IS_MMAPPED if the chunk was obtained with mmap() */

#تعریف_کرو IS_MMAPPED 0x2

/* Bits to mask off when extracting size */

#تعریف_کرو سائز_بٹ (پچھلا_INUSE|IS_MMAPPED)

/* Ptr to next physical malloc_chunk. */

#تعریف_کرو اگلا_chunk(p) ((mchunkptr)( ((حرف*)(p)) + ((p)->سائز & ~پچھلا_INUSE) ))

/* Ptr to previous physical malloc_chunk */

#تعریف_کرو پچھلا_chunk(p)\
   ((mchunkptr)( ((حرف*)(p)) - ((p)->پچھلا_سائز) ))

/* Treat space at ptr + offset as a chunk */

#تعریف_کرو chunk_at_انحراف(p, s)  ((mchunkptr)(((حرف*)(p)) + (s)))

/*
  Dealing with use bits
*/

/* extract p's inuse bit */

#تعریف_کرو inuse(p)\
((((mchunkptr)(((حرف*)(p))+((p)->سائز & ~پچھلا_INUSE)))->سائز) & پچھلا_INUSE)

/* extract inuse bit of previous chunk */

#تعریف_کرو پچھلا_inuse(p)  ((p)->سائز & پچھلا_INUSE)

/* check for mmap()'ed chunk */

#تعریف_کرو chunk_is_mmapped(p) ((p)->سائز & IS_MMAPPED)

/* set/clear chunk as in use without otherwise disturbing */

#تعریف_کرو سیٹ_inuse(p)\
((mchunkptr)(((حرف*)(p)) + ((p)->سائز & ~پچھلا_INUSE)))->سائز |= پچھلا_INUSE

#تعریف_کرو clear_inuse(p)\
((mchunkptr)(((حرف*)(p)) + ((p)->سائز & ~پچھلا_INUSE)))->سائز &= ~(پچھلا_INUSE)

/* check/set/clear inuse bits in known places */

#تعریف_کرو inuse_ثنائی_ہندسہ_at_انحراف(p, s)\
 (((mchunkptr)(((حرف*)(p)) + (s)))->سائز & پچھلا_INUSE)

#تعریف_کرو سیٹ_inuse_ثنائی_ہندسہ_at_انحراف(p, s)\
 (((mchunkptr)(((حرف*)(p)) + (s)))->سائز |= پچھلا_INUSE)

#تعریف_کرو clear_inuse_ثنائی_ہندسہ_at_انحراف(p, s)\
 (((mchunkptr)(((حرف*)(p)) + (s)))->سائز &= ~(پچھلا_INUSE))

/*
  Dealing with size fields
*/

/* Get size, ignoring use bits */

#تعریف_کرو chunksize(p)          ((p)->سائز & ~(سائز_بٹ))

/* Set size at head, without disturbing its use bit */

#تعریف_کرو سیٹ_سر_سائز(p, s)   ((p)->سائز = (((p)->سائز & پچھلا_INUSE) | (s)))

/* Set size/use ignoring previous bits in header */

#تعریف_کرو سیٹ_سر(p, s)        ((p)->سائز = (s))

/* Set size at footer (only when chunk is not in use) */

#تعریف_کرو سیٹ_foot(p, s)   (((mchunkptr)((حرف*)(p) + (s)))->پچھلا_سائز = (s))

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

#تعریف_کرو NAV             128   /* number of bins */

قسمتعریف ساخت malloc_chunk* mbinptr;

/* access macros */

#تعریف_کرو bin_at(i)      ((mbinptr)((حرف*)&(av_[2*(i) + 2]) - 2*سائز_SZ))
#تعریف_کرو اگلا_bin(b)    ((mbinptr)((حرف*)(b) + 2 * حجم(mbinptr)))
#تعریف_کرو پچھلا_bin(b)    ((mbinptr)((حرف*)(b) - 2 * حجم(mbinptr)))

/*
   The first 2 bins are never indexed. The corresponding av_ cells are instead
   used for bookkeeping. This is not to save space, but to simplify
   indexing, maintain locality, and avoid some initialization tests.
*/

#تعریف_کرو چوٹی            (av_[2])          /* The topmost chunk */
#تعریف_کرو آخری_remainder (bin_at(1))       /* remainder from last split */

/*
   Because top initially points to its own bin with initial
   zero size, thus forcing extension on the first malloc request,
   we avoid having any special code in malloc to check whether
   it even exists yet. But we still need to in malloc_extend_top.
*/

#تعریف_کرو initial_چوٹی    ((mchunkptr)(bin_at(0)))

/* Helper macro to initialize bins */

#تعریف_کرو IAV(i)  bin_at(i), bin_at(i)

ساکن mbinptr av_[NAV * 2 + 2] = {
 تہی, تہی,
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

#اگر_تعریف_شدہ CONFIG_SYS_MALLOC_DEFAULT_مقصد_INIT
ساکن خالی malloc_init(خالی);
#اگر_ختم

ulong mem_malloc_آغاز = 0;
ulong mem_malloc_انتہا = 0;
ulong mem_malloc_brk = 0;

ساکن bool malloc_testing;	/* enable test mode */
ساکن صحیح malloc_max_allocs;	/* return NULL after this many calls to malloc() */

خالی *sbrk(ptrdiff_t increment)
{
	ulong old = mem_malloc_brk;
	ulong new = old + increment;

	اگر ((new < mem_malloc_آغاز) || (new > mem_malloc_انتہا))
		واپس (خالی *)MORECORE_FAILURE;

	/*
	 * if we are giving memory back make sure we clear it out since
	 * we set MORECORE_CLEARS to 1
	 */
	اگر (increment < 0)
		memset((خالی *)new, 0, -increment);

	mem_malloc_brk = new;

	واپس (خالی *)old;
}

خالی mem_malloc_init(ulong آغاز, ulong سائز)
{
	mem_malloc_آغاز = (ulong)map_sysmem(آغاز, سائز);
	mem_malloc_انتہا = mem_malloc_آغاز + سائز;
	mem_malloc_brk = mem_malloc_آغاز;

#اگر_تعریف_شدہ CONFIG_SYS_MALLOC_DEFAULT_مقصد_INIT
	malloc_init();
#اگر_ختم

	debug("using memory %#lx-%#lx for malloc()\n", mem_malloc_آغاز,
	      mem_malloc_انتہا);
#اگر CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
	memset((خالی *)mem_malloc_آغاز, 0x0, سائز);
#اگر_ختم
}

/* field-extraction macros */

#تعریف_کرو پہلا(b) ((b)->fd)
#تعریف_کرو آخری(b)  ((b)->bk)

/*
  Indexing into bins
*/

#تعریف_کرو bin_اشاریہ(sz)                                                          \
(((((غیرعلامتی طویل)(sz)) >> 9) ==    0) ?       (((غیرعلامتی طویل)(sz)) >>  3): \
 ((((غیرعلامتی طویل)(sz)) >> 9) <=    4) ?  56 + (((غیرعلامتی طویل)(sz)) >>  6): \
 ((((غیرعلامتی طویل)(sz)) >> 9) <=   20) ?  91 + (((غیرعلامتی طویل)(sz)) >>  9): \
 ((((غیرعلامتی طویل)(sz)) >> 9) <=   84) ? 110 + (((غیرعلامتی طویل)(sz)) >> 12): \
 ((((غیرعلامتی طویل)(sz)) >> 9) <=  340) ? 119 + (((غیرعلامتی طویل)(sz)) >> 15): \
 ((((غیرعلامتی طویل)(sz)) >> 9) <= 1364) ? 124 + (((غیرعلامتی طویل)(sz)) >> 18): \
					  126)
/*
  bins for chunks < 512 are all spaced 8 bytes apart, and hold
  identically sized chunks. This is exploited in malloc.
*/

#تعریف_کرو MAX_SMALLBIN         63
#تعریف_کرو MAX_SMALLBIN_سائز   512
#تعریف_کرو SMALLBIN_چوڑائی        8

#تعریف_کرو smallbin_اشاریہ(sz)  (((غیرعلامتی طویل)(sz)) >> 3)

/*
   Requests are `small' if both the corresponding and the next bin are small
*/

#تعریف_کرو is_small_request(nb) (nb < MAX_SMALLBIN_سائز - SMALLBIN_چوڑائی)

/*
    To help compensate for the large number of bins, a one-level index
    structure is used for bin-by-bin searching.  `binblocks' is a
    one-word bitvector recording whether groups of BINBLOCKWIDTH bins
    have any (possibly) non-empty bins, so they can be skipped over
    all at once during during traversals. The bits are NOT always
    cleared as soon as all bins in a block are empty, but instead only
    when all are noticed to be empty during traversal in malloc.
*/

#تعریف_کرو BINBLOCKWIDTH     4   /* bins per block */

#تعریف_کرو binblocks_r     ((اندرونی_سائز_T)av_[1]) /* bitvector of nonempty blocks */
#تعریف_کرو binblocks_w     (av_[1])

/* bin<->block macros */

#تعریف_کرو idx2binblock(ix)    ((غیرعلامتی)1 << (ix / BINBLOCKWIDTH))
#تعریف_کرو mark_binblock(ii)   (binblocks_w = (mbinptr)(binblocks_r | idx2binblock(ii)))
#تعریف_کرو clear_binblock(ii)  (binblocks_w = (mbinptr)(binblocks_r & ~(idx2binblock(ii))))

/*  Other static bookkeeping data */

/* variables holding tunable values */

ساکن غیرعلامتی طویل trim_حد   = DEFAULT_TRIM_حد;
ساکن غیرعلامتی طویل چوٹی_pad          = DEFAULT_چوٹی_PAD;
ساکن غیرعلامتی صحیح  n_mmaps_max      = DEFAULT_MMAP_MAX;
ساکن غیرعلامتی طویل mmap_حد   = DEFAULT_MMAP_حد;

/* The first value returned from sbrk */
ساکن حرف* sbrk_بنیاد = (حرف*)(-1);

/* The maximum memory obtained from system via sbrk */
ساکن غیرعلامتی طویل max_sbrked_mem = 0;

/* The maximum via either sbrk or mmap */
ساکن غیرعلامتی طویل max_کل_mem = 0;

/* internal working copy of mallinfo */
ساکن ساخت mallinfo current_mallinfo = {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* The total memory obtained from system via sbrk */
#تعریف_کرو sbrked_mem  (current_mallinfo.arena)

/* Tracking mmaps */

#اگر_تعریف_شدہ DEBUG
ساکن غیرعلامتی صحیح n_mmaps = 0;
#اگر_ختم	/* DEBUG */
ساکن غیرعلامتی طویل mmapped_mem = 0;
#اگر HAVE_MMAP
ساکن غیرعلامتی صحیح max_n_mmaps = 0;
ساکن غیرعلامتی طویل max_mmapped_mem = 0;
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_SYS_MALLOC_DEFAULT_مقصد_INIT
ساکن خالی malloc_init(خالی)
{
	صحیح i, j;

	debug("bins (av_ array) are at %p\n", (خالی *)av_);

	av_[0] = تہی; av_[1] = تہی;
	برائے (i = 2, j = 2; i < NAV * 2 + 2; i += 2, j++) {
		av_[i] = bin_at(j - 2);
		av_[i + 1] = bin_at(j - 2);

		/* Just print the first few bins so that
		 * we can see there are alright.
		 */
		اگر (i < 10)
			debug("av_[%d]=%lx av_[%d]=%lx\n",
			      i, (ulong)av_[i],
			      i + 1, (ulong)av_[i + 1]);
	}

	/* Init the static bookkeeping as well */
	sbrk_بنیاد = (حرف *)(-1);
	max_sbrked_mem = 0;
	max_کل_mem = 0;
#اگر_تعریف_شدہ DEBUG
	memset((خالی *)&current_mallinfo, 0, حجم(ساخت mallinfo));
#اگر_ختم
}
#اگر_ختم

/*
  Debugging support
*/

#اگر_تعریف_شدہ DEBUG

/*
  These routines make a number of assertions about the states
  of data structures that should be true at all times. If any
  are not true, it's very likely that a user program has somehow
  trashed memory. (It's also possible that there is a coding error
  in malloc. In which case, please report it!)
*/

#اگر __STD_C
ساکن خالی do_check_chunk(mchunkptr p)
#ورنہ
ساکن خالی do_check_chunk(p) mchunkptr p;
#اگر_ختم
{
  اندرونی_سائز_T sz = p->سائز & ~پچھلا_INUSE;

  /* No checkable chunk is mmapped */
  أكد(!chunk_is_mmapped(p));

  /* Check for legal address ... */
  أكد((حرف*)p >= sbrk_بنیاد);
  اگر (p != چوٹی)
    أكد((حرف*)p + sz <= (حرف*)چوٹی);
  ورنہ
    أكد((حرف*)p + sz <= sbrk_بنیاد + sbrked_mem);

}

#اگر __STD_C
ساکن خالی do_check_free_chunk(mchunkptr p)
#ورنہ
ساکن خالی do_check_free_chunk(p) mchunkptr p;
#اگر_ختم
{
  اندرونی_سائز_T sz = p->سائز & ~پچھلا_INUSE;
  mchunkptr اگلا = chunk_at_انحراف(p, sz);

  do_check_chunk(p);

  /* Check whether it claims to be free ... */
  أكد(!inuse(p));

  /* Unless a special marker, must have OK fields */
  اگر ((طویل)sz >= (طویل)MINSIZE)
  {
    أكد((sz & MALLOC_ALIGN_نقاب) == 0);
    أكد(aligned_OK(chunk2mem(p)));
    /* ... matching footer field */
    أكد(اگلا->پچھلا_سائز == sz);
    /* ... and is fully consolidated */
    أكد(پچھلا_inuse(p));
    أكد (اگلا == چوٹی || inuse(اگلا));

    /* ... and has minimally sane links */
    أكد(p->fd->bk == p);
    أكد(p->bk->fd == p);
  }
  ورنہ /* markers are always of size SIZE_SZ */
    أكد(sz == سائز_SZ);
}

#اگر __STD_C
ساکن خالی do_check_inuse_chunk(mchunkptr p)
#ورنہ
ساکن خالی do_check_inuse_chunk(p) mchunkptr p;
#اگر_ختم
{
  mchunkptr اگلا = اگلا_chunk(p);
  do_check_chunk(p);

  /* Check whether it claims to be in use ... */
  أكد(inuse(p));

  /* ... and is surrounded by OK chunks.
    Since more things can be checked with free chunks than inuse ones,
    if an inuse chunk borders them and debug is on, it's worth doing them.
  */
  اگر (!پچھلا_inuse(p))
  {
    mchunkptr prv = پچھلا_chunk(p);
    أكد(اگلا_chunk(prv) == p);
    do_check_free_chunk(prv);
  }
  اگر (اگلا == چوٹی)
  {
    أكد(پچھلا_inuse(اگلا));
    أكد(chunksize(اگلا) >= MINSIZE);
  }
  ورنہ اگر (!inuse(اگلا))
    do_check_free_chunk(اگلا);

}

#اگر __STD_C
ساکن خالی do_check_malloced_chunk(mchunkptr p, اندرونی_سائز_T s)
#ورنہ
ساکن خالی do_check_malloced_chunk(p, s) mchunkptr p; اندرونی_سائز_T s;
#اگر_ختم
{
  اندرونی_سائز_T sz = p->سائز & ~پچھلا_INUSE;
  طویل گنجائش = sz - s;

  do_check_inuse_chunk(p);

  /* Legal size ... */
  أكد((طویل)sz >= (طویل)MINSIZE);
  أكد((sz & MALLOC_ALIGN_نقاب) == 0);
  أكد(گنجائش >= 0);
  أكد(گنجائش < (طویل)MINSIZE);

  /* ... and alignment */
  أكد(aligned_OK(chunk2mem(p)));

  /* ... and was allocated at front of an available chunk */
  أكد(پچھلا_inuse(p));

}

#تعریف_کرو check_free_chunk(P)  do_check_free_chunk(P)
#تعریف_کرو check_inuse_chunk(P) do_check_inuse_chunk(P)
#تعریف_کرو check_chunk(P) do_check_chunk(P)
#تعریف_کرو check_malloced_chunk(P,N) do_check_malloced_chunk(P,N)
#ورنہ
#تعریف_کرو check_free_chunk(P)
#تعریف_کرو check_inuse_chunk(P)
#تعریف_کرو check_chunk(P)
#تعریف_کرو check_malloced_chunk(P,N)
#اگر_ختم

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

#تعریف_کرو frontlink(P, S, IDX, BK, FD)                                          \
{                                                                             \
  اگر (S < MAX_SMALLBIN_سائز)                                                  \
  {                                                                           \
    IDX = smallbin_اشاریہ(S);                                                  \
    mark_binblock(IDX);                                                       \
    BK = bin_at(IDX);                                                         \
    FD = BK->fd;                                                              \
    P->bk = BK;                                                               \
    P->fd = FD;                                                               \
    FD->bk = BK->fd = P;                                                      \
  }                                                                           \
  ورنہ                                                                        \
  {                                                                           \
    IDX = bin_اشاریہ(S);                                                       \
    BK = bin_at(IDX);                                                         \
    FD = BK->fd;                                                              \
    اگر (FD == BK) mark_binblock(IDX);                                         \
    ورنہ                                                                      \
    {                                                                         \
      جب (FD != BK && S < chunksize(FD)) FD = FD->fd;                      \
      BK = FD->bk;                                                            \
    }                                                                         \
    P->bk = BK;                                                               \
    P->fd = FD;                                                               \
    FD->bk = BK->fd = P;                                                      \
  }                                                                           \
}

/* take a chunk off a list */

#تعریف_کرو unlink(P, BK, FD)                                                     \
{                                                                             \
  BK = P->bk;                                                                 \
  FD = P->fd;                                                                 \
  FD->bk = BK;                                                                \
  BK->fd = FD;                                                                \
}                                                                             \

/* Place p as the last remainder */

#تعریف_کرو link_آخری_remainder(P)                                                \
{                                                                             \
  آخری_remainder->fd = آخری_remainder->bk =  P;                               \
  P->fd = P->bk = آخری_remainder;                                             \
}

/* Clear the last_remainder bin */

#تعریف_کرو clear_آخری_remainder \
  (آخری_remainder->fd = آخری_remainder->bk = آخری_remainder)

/* Routines dealing with mmap(). */

#اگر HAVE_MMAP

#اگر __STD_C
ساکن mchunkptr mmap_chunk(size_t سائز)
#ورنہ
ساکن mchunkptr mmap_chunk(سائز) size_t سائز;
#اگر_ختم
{
  size_t صفحہ_نقاب = malloc_getpagesize - 1;
  mchunkptr p;

#اگر_غیر_تعریف_شدہ MAP_ANONYMOUS
  ساکن صحیح fd = -1;
#اگر_ختم

  اگر(n_mmaps >= n_mmaps_max) واپس 0; /* too many regions */

  /* For mmapped chunks, the overhead is one SIZE_SZ unit larger, because
   * there is no following chunk whose prev_size field could be used.
   */
  سائز = (سائز + سائز_SZ + صفحہ_نقاب) & ~صفحہ_نقاب;

#اگر_تعریف_شدہ MAP_ANONYMOUS
  p = (mchunkptr)mmap(0, سائز, PROT_READ|PROT_WRITE,
		      MAP_نجی|MAP_ANONYMOUS, -1, 0);
#ورنہ /* !MAP_ANONYMOUS */
  اگر (fd < 0)
  {
    fd = open("/dev/zero", O_RDWR);
    اگر(fd < 0) واپس 0;
  }
  p = (mchunkptr)mmap(0, سائز, PROT_READ|PROT_WRITE, MAP_نجی, fd, 0);
#اگر_ختم

  اگر(p == (mchunkptr)-1) واپس 0;

  n_mmaps++;
  اگر (n_mmaps > max_n_mmaps) max_n_mmaps = n_mmaps;

  /* We demand that eight bytes into a page must be 8-byte aligned. */
  أكد(aligned_OK(chunk2mem(p)));

  /* The offset to the start of the mmapped region is stored
   * in the prev_size field of the chunk; normally it is zero,
   * but that can be changed in memalign().
   */
  p->پچھلا_سائز = 0;
  سیٹ_سر(p, سائز|IS_MMAPPED);

  mmapped_mem += سائز;
  اگر ((غیرعلامتی طویل)mmapped_mem > (غیرعلامتی طویل)max_mmapped_mem)
    max_mmapped_mem = mmapped_mem;
  اگر ((غیرعلامتی طویل)(mmapped_mem + sbrked_mem) > (غیرعلامتی طویل)max_کل_mem)
    max_کل_mem = mmapped_mem + sbrked_mem;
  واپس p;
}

#اگر __STD_C
ساکن خالی munmap_chunk(mchunkptr p)
#ورنہ
ساکن خالی munmap_chunk(p) mchunkptr p;
#اگر_ختم
{
  اندرونی_سائز_T سائز = chunksize(p);
  صحیح ret;

  أكد (chunk_is_mmapped(p));
  أكد(! ((حرف*)p >= sbrk_بنیاد && (حرف*)p < sbrk_بنیاد + sbrked_mem));
  أكد((n_mmaps > 0));
  أكد(((p->پچھلا_سائز + سائز) & (malloc_getpagesize-1)) == 0);

  n_mmaps--;
  mmapped_mem -= (سائز + p->پچھلا_سائز);

  ret = munmap((حرف *)p - p->پچھلا_سائز, سائز + p->پچھلا_سائز);

  /* munmap returns non-zero on failure */
  أكد(ret == 0);
}

#اگر HAVE_MREMAP

#اگر __STD_C
ساکن mchunkptr mremap_chunk(mchunkptr p, size_t new_سائز)
#ورنہ
ساکن mchunkptr mremap_chunk(p, new_سائز) mchunkptr p; size_t new_سائز;
#اگر_ختم
{
  size_t صفحہ_نقاب = malloc_getpagesize - 1;
  اندرونی_سائز_T انحراف = p->پچھلا_سائز;
  اندرونی_سائز_T سائز = chunksize(p);
  حرف *cp;

  أكد (chunk_is_mmapped(p));
  أكد(! ((حرف*)p >= sbrk_بنیاد && (حرف*)p < sbrk_بنیاد + sbrked_mem));
  أكد((n_mmaps > 0));
  أكد(((سائز + انحراف) & (malloc_getpagesize-1)) == 0);

  /* Note the extra SIZE_SZ overhead as in mmap_chunk(). */
  new_سائز = (new_سائز + انحراف + سائز_SZ + صفحہ_نقاب) & ~صفحہ_نقاب;

  cp = (حرف *)mremap((حرف *)p - انحراف, سائز + انحراف, new_سائز, 1);

  اگر (cp == (حرف *)-1) واپس 0;

  p = (mchunkptr)(cp + انحراف);

  أكد(aligned_OK(chunk2mem(p)));

  أكد((p->پچھلا_سائز == انحراف));
  سیٹ_سر(p, (new_سائز - انحراف)|IS_MMAPPED);

  mmapped_mem -= سائز + انحراف;
  mmapped_mem += new_سائز;
  اگر ((غیرعلامتی طویل)mmapped_mem > (غیرعلامتی طویل)max_mmapped_mem)
    max_mmapped_mem = mmapped_mem;
  اگر ((غیرعلامتی طویل)(mmapped_mem + sbrked_mem) > (غیرعلامتی طویل)max_کل_mem)
    max_کل_mem = mmapped_mem + sbrked_mem;
  واپس p;
}

#اگر_ختم /* HAVE_MREMAP */

#اگر_ختم /* HAVE_MMAP */

/*
  Extend the top-most chunk by obtaining memory from system.
  Main interface to sbrk (but see also malloc_trim).
*/

#اگر __STD_C
ساکن خالی malloc_extend_چوٹی(اندرونی_سائز_T nb)
#ورنہ
ساکن خالی malloc_extend_چوٹی(nb) اندرونی_سائز_T nb;
#اگر_ختم
{
  حرف*     brk;                  /* return value from sbrk */
  اندرونی_سائز_T front_misalign; /* unusable bytes at front of sbrked space */
  اندرونی_سائز_T correction;     /* bytes for 2nd sbrk call */
  حرف*     new_brk;              /* return of 2nd sbrk call */
  اندرونی_سائز_T چوٹی_سائز;       /* new size of top chunk */

  mchunkptr old_چوٹی     = چوٹی;  /* Record state of old top */
  اندرونی_سائز_T old_چوٹی_سائز = chunksize(old_چوٹی);
  حرف*     old_انتہا      = (حرف*)(chunk_at_انحراف(old_چوٹی, old_چوٹی_سائز));

  /* Pad request with top_pad plus minimal overhead */

  اندرونی_سائز_T    sbrk_سائز     = nb + چوٹی_pad + MINSIZE;
  غیرعلامتی طویل pagesz    = malloc_getpagesize;

  /* If not the first time through, round to preserve page boundary */
  /* Otherwise, we need to correct to a page size below anyway. */
  /* (We also correct below if an intervening foreign sbrk call.) */

  اگر (sbrk_بنیاد != (حرف*)(-1))
    sbrk_سائز = (sbrk_سائز + (pagesz - 1)) & ~(pagesz - 1);

  brk = (حرف*)(MORECORE (sbrk_سائز));

  /* Fail if sbrk failed or if a foreign sbrk call killed our space */
  اگر (brk == (حرف*)(MORECORE_FAILURE) ||
      (brk < old_انتہا && old_چوٹی != initial_چوٹی))
    واپس;

  sbrked_mem += sbrk_سائز;

  اگر (brk == old_انتہا) /* can just add bytes to current top */
  {
    چوٹی_سائز = sbrk_سائز + old_چوٹی_سائز;
    سیٹ_سر(چوٹی, چوٹی_سائز | پچھلا_INUSE);
  }
  ورنہ
  {
    اگر (sbrk_بنیاد == (حرف*)(-1))  /* First time through. Record base */
      sbrk_بنیاد = brk;
    ورنہ  /* Someone else called sbrk().  Count those bytes as sbrked_mem. */
      sbrked_mem += brk - (حرف*)old_انتہا;

    /* Guarantee alignment of first new chunk made from this space */
    front_misalign = (غیرعلامتی طویل)chunk2mem(brk) & MALLOC_ALIGN_نقاب;
    اگر (front_misalign > 0)
    {
      correction = (MALLOC_ALIGNMENT) - front_misalign;
      brk += correction;
    }
    ورنہ
      correction = 0;

    /* Guarantee the next brk will be at a page boundary */

    correction += ((((غیرعلامتی طویل)(brk + sbrk_سائز))+(pagesz-1)) &
		   ~(pagesz - 1)) - ((غیرعلامتی طویل)(brk + sbrk_سائز));

    /* Allocate correction */
    new_brk = (حرف*)(MORECORE (correction));
    اگر (new_brk == (حرف*)(MORECORE_FAILURE)) واپس;

    sbrked_mem += correction;

    چوٹی = (mchunkptr)brk;
    چوٹی_سائز = new_brk - brk + correction;
    سیٹ_سر(چوٹی, چوٹی_سائز | پچھلا_INUSE);

    اگر (old_چوٹی != initial_چوٹی)
    {

      /* There must have been an intervening foreign sbrk call. */
      /* A double fencepost is necessary to prevent consolidation */

      /* If not enough space to do this, then user did something very wrong */
      اگر (old_چوٹی_سائز < MINSIZE)
      {
	سیٹ_سر(چوٹی, پچھلا_INUSE); /* will force null return from malloc */
	واپس;
      }

      /* Also keep size a multiple of MALLOC_ALIGNMENT */
      old_چوٹی_سائز = (old_چوٹی_سائز - 3*سائز_SZ) & ~MALLOC_ALIGN_نقاب;
      سیٹ_سر_سائز(old_چوٹی, old_چوٹی_سائز);
      chunk_at_انحراف(old_چوٹی, old_چوٹی_سائز          )->سائز =
	سائز_SZ|پچھلا_INUSE;
      chunk_at_انحراف(old_چوٹی, old_چوٹی_سائز + سائز_SZ)->سائز =
	سائز_SZ|پچھلا_INUSE;
      /* If possible, release the rest. */
      اگر (old_چوٹی_سائز >= MINSIZE)
	fREe(chunk2mem(old_چوٹی));
    }
  }

  اگر ((غیرعلامتی طویل)sbrked_mem > (غیرعلامتی طویل)max_sbrked_mem)
    max_sbrked_mem = sbrked_mem;
  اگر ((غیرعلامتی طویل)(mmapped_mem + sbrked_mem) > (غیرعلامتی طویل)max_کل_mem)
    max_کل_mem = mmapped_mem + sbrked_mem;

  /* We always land on a page boundary */
  أكد(((غیرعلامتی طویل)((حرف*)چوٹی + چوٹی_سائز) & (pagesz - 1)) == 0);
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
#اگر __STD_C
Void_t* mALLOc_impl(size_t بائٹیں)
#ورنہ
Void_t* mALLOc_impl(بائٹیں) size_t بائٹیں;
#اگر_ختم
{
  mchunkptr victim;                  /* inspected/selected chunk */
  اندرونی_سائز_T victim_سائز;       /* its size */
  صحیح       idx;                     /* index for bin traversal */
  mbinptr   bin;                     /* associated bin */
  mchunkptr remainder;               /* remainder from a split */
  طویل      remainder_سائز;          /* its size */
  صحیح       remainder_اشاریہ;         /* its bin index */
  غیرعلامتی طویل بلاک;               /* block traverser bit */
  صحیح       startidx;                /* first bin of a traversed block */
  mchunkptr fwd;                     /* misc temp for linking */
  mchunkptr bck;                     /* misc temp for linking */
  mbinptr q;                         /* misc temp */

  اندرونی_سائز_T nb;

#اگر CONFIG_IS_ENABLED(SYS_MALLOC_F)
	اگر (!(gd->flags & GD_FLG_بھرا_MALLOC_INIT))
		واپس malloc_simple(بائٹیں);
#اگر_ختم

  اگر (CONFIG_IS_ENABLED(UNIT_TEST) && malloc_testing) {
    اگر (--malloc_max_allocs < 0)
      واپس تہی;
  }

  /* check if mem_malloc_init() was run */
  اگر ((mem_malloc_آغاز == 0) && (mem_malloc_انتہا == 0)) {
    /* not initialized yet */
    واپس تہی;
  }

  اگر (بائٹیں > CONFIG_SYS_MALLOC_LEN || (طویل)بائٹیں < 0)
     واپس تہی;

  nb = request2size(بائٹیں);  /* padded request size; */

  /* Check for exact match in a bin */

  اگر (is_small_request(nb))  /* Faster version for small requests */
  {
    idx = smallbin_اشاریہ(nb);

    /* No traversal or size check necessary for small bins.  */

    q = bin_at(idx);
    victim = آخری(q);

    /* Also scan the next one, since it would have a remainder < MINSIZE */
    اگر (victim == q)
    {
      q = اگلا_bin(q);
      victim = آخری(q);
    }
    اگر (victim != q)
    {
      victim_سائز = chunksize(victim);
      unlink(victim, bck, fwd);
      سیٹ_inuse_ثنائی_ہندسہ_at_انحراف(victim, victim_سائز);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_بلاک(chunk2mem(victim), بائٹیں, سائز_SZ, false);
      واپس chunk2mem(victim);
    }

    idx += 2; /* Set for bin scan below. We've already scanned 2 bins. */

  }
  ورنہ
  {
    idx = bin_اشاریہ(nb);
    bin = bin_at(idx);

    برائے (victim = آخری(bin); victim != bin; victim = victim->bk)
    {
      victim_سائز = chunksize(victim);
      remainder_سائز = victim_سائز - nb;

      اگر (remainder_سائز >= (طویل)MINSIZE) /* too big */
      {
	--idx; /* adjust to rescan below after checking last remainder */
	توڑو;
      }

      ورنہ اگر (remainder_سائز >= 0) /* exact fit */
      {
	unlink(victim, bck, fwd);
	سیٹ_inuse_ثنائی_ہندسہ_at_انحراف(victim, victim_سائز);
	check_malloced_chunk(victim, nb);
        VALGRIND_MALLOCLIKE_بلاک(chunk2mem(victim), بائٹیں, سائز_SZ, false);
	واپس chunk2mem(victim);
      }
    }

    ++idx;

  }

  /* Try to use the last split-off remainder */

  اگر ( (victim = آخری_remainder->fd) != آخری_remainder)
  {
    victim_سائز = chunksize(victim);
    remainder_سائز = victim_سائز - nb;

    اگر (remainder_سائز >= (طویل)MINSIZE) /* re-split */
    {
      remainder = chunk_at_انحراف(victim, nb);
      سیٹ_سر(victim, nb | پچھلا_INUSE);
      link_آخری_remainder(remainder);
      سیٹ_سر(remainder, remainder_سائز | پچھلا_INUSE);
      سیٹ_foot(remainder, remainder_سائز);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_بلاک(chunk2mem(victim), بائٹیں, سائز_SZ, false);
      واپس chunk2mem(victim);
    }

    clear_آخری_remainder;

    اگر (remainder_سائز >= 0)  /* exhaust */
    {
      سیٹ_inuse_ثنائی_ہندسہ_at_انحراف(victim, victim_سائز);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_بلاک(chunk2mem(victim), بائٹیں, سائز_SZ, false);
      واپس chunk2mem(victim);
    }

    /* Else place in bin */

    frontlink(victim, victim_سائز, remainder_اشاریہ, bck, fwd);
  }

  /*
     If there are any possibly nonempty big-enough blocks,
     search for best fitting chunk by scanning bins in blockwidth units.
  */

  اگر ( (بلاک = idx2binblock(idx)) <= binblocks_r)
  {

    /* Get to the first marked block */

    اگر ( (بلاک & binblocks_r) == 0)
    {
      /* force to an even block boundary */
      idx = (idx & ~(BINBLOCKWIDTH - 1)) + BINBLOCKWIDTH;
      بلاک <<= 1;
      جب ((بلاک & binblocks_r) == 0)
      {
	idx += BINBLOCKWIDTH;
	بلاک <<= 1;
      }
    }

    /* For each possibly nonempty block ... */
    برائے (;;)
    {
      startidx = idx;          /* (track incomplete blocks) */
      q = bin = bin_at(idx);

      /* For each bin in this block ... */
      کرو
      {
	/* Find and use first big enough chunk ... */

	برائے (victim = آخری(bin); victim != bin; victim = victim->bk)
	{
	  victim_سائز = chunksize(victim);
	  remainder_سائز = victim_سائز - nb;

	  اگر (remainder_سائز >= (طویل)MINSIZE) /*بانٹو*/
	  {
	    remainder = chunk_at_انحراف(victim, nb);
	    سیٹ_سر(victim, nb | پچھلا_INUSE);
	    unlink(victim, bck, fwd);
	    link_آخری_remainder(remainder);
	    سیٹ_سر(remainder, remainder_سائز | پچھلا_INUSE);
	    سیٹ_foot(remainder, remainder_سائز);
	    check_malloced_chunk(victim, nb);
	    VALGRIND_MALLOCLIKE_بلاک(chunk2mem(victim), بائٹیں, سائز_SZ, false);
	    واپس chunk2mem(victim);
	  }

	  ورنہ اگر (remainder_سائز >= 0)  /* take */
	  {
	    سیٹ_inuse_ثنائی_ہندسہ_at_انحراف(victim, victim_سائز);
	    unlink(victim, bck, fwd);
	    check_malloced_chunk(victim, nb);
	    VALGRIND_MALLOCLIKE_بلاک(chunk2mem(victim), بائٹیں, سائز_SZ, false);
	    واپس chunk2mem(victim);
	  }

	}

       bin = اگلا_bin(bin);

      } جب ((++idx & (BINBLOCKWIDTH - 1)) != 0);

      /* Clear out the block bit. */

      کرو   /* Possibly backtrack to try to clear a partial block */
      {
	اگر ((startidx & (BINBLOCKWIDTH - 1)) == 0)
	{
	  av_[1] = (mbinptr)(binblocks_r & ~بلاک);
	  توڑو;
	}
	--startidx;
       q = پچھلا_bin(q);
      } جب (پہلا(q) == q);

      /* Get to the next possibly nonempty block */

      اگر ( (بلاک <<= 1) <= binblocks_r && (بلاک != 0) )
      {
	جب ((بلاک & binblocks_r) == 0)
	{
	  idx += BINBLOCKWIDTH;
	  بلاک <<= 1;
	}
      }
      ورنہ
	توڑو;
    }
  }

  /* Try to use top chunk */

  /* Require that there be a remainder, ensuring top always exists  */
  اگر ( (remainder_سائز = chunksize(چوٹی) - nb) < (طویل)MINSIZE)
  {

#اگر HAVE_MMAP
    /* If big and would otherwise need to extend, try to use mmap instead */
    اگر ((غیرعلامتی طویل)nb >= (غیرعلامتی طویل)mmap_حد &&
	(victim = mmap_chunk(nb)))
      VALGRIND_MALLOCLIKE_بلاک(chunk2mem(victim), بائٹیں, سائز_SZ, false);
      واپس chunk2mem(victim);
#اگر_ختم

    /* Try to extend */
    malloc_extend_چوٹی(nb);
    اگر ( (remainder_سائز = chunksize(چوٹی) - nb) < (طویل)MINSIZE)
      واپس تہی; /* propagate failure */
  }

  victim = چوٹی;
  سیٹ_سر(victim, nb | پچھلا_INUSE);
  چوٹی = chunk_at_انحراف(victim, nb);
  سیٹ_سر(چوٹی, remainder_سائز | پچھلا_INUSE);
  check_malloced_chunk(victim, nb);
  VALGRIND_MALLOCLIKE_بلاک(chunk2mem(victim), بائٹیں, سائز_SZ, false);
  واپس chunk2mem(victim);

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
#اگر __STD_C
خالی fREe_impl(Void_t* mem)
#ورنہ
خالی fREe_impl(mem) Void_t* mem;
#اگر_ختم
{
  mchunkptr p;         /* chunk corresponding to mem */
  اندرونی_سائز_T hd;  /* its head field */
  اندرونی_سائز_T sz;  /* its size */
  صحیح       idx;       /* its bin index */
  mchunkptr اگلا;      /* next contiguous chunk */
  اندرونی_سائز_T nextsz; /* its size */
  اندرونی_سائز_T prevsz; /* size of previous contiguous chunk */
  mchunkptr bck;       /* misc temp for linking */
  mchunkptr fwd;       /* misc temp for linking */
  صحیح       islr;      /* track whether merging with last_remainder */

#اگر CONFIG_IS_ENABLED(SYS_MALLOC_F)
	/* free() is a no-op - all the memory will be freed on relocation */
	اگر (!(gd->flags & GD_FLG_بھرا_MALLOC_INIT)) {
		VALGRIND_FREELIKE_بلاک(mem, سائز_SZ);
		واپس;
	}
#اگر_ختم

  اگر (mem == تہی)                              /* free(0) has no effect */
    واپس;

  p = mem2chunk(mem);
  hd = p->سائز;

#اگر HAVE_MMAP
  اگر (hd & IS_MMAPPED)                       /* release mmapped memory. */
  {
    munmap_chunk(p);
    واپس;
  }
#اگر_ختم

  check_inuse_chunk(p);

  sz = hd & ~پچھلا_INUSE;
  اگلا = chunk_at_انحراف(p, sz);
  nextsz = chunksize(اگلا);
  VALGRIND_FREELIKE_بلاک(mem, سائز_SZ);

  اگر (اگلا == چوٹی)                            /* merge with top */
  {
    sz += nextsz;

    اگر (!(hd & پچھلا_INUSE))                    /* consolidate backward */
    {
      prevsz = p->پچھلا_سائز;
      p = chunk_at_انحراف(p, -((طویل) prevsz));
      sz += prevsz;
      unlink(p, bck, fwd);
    }

    سیٹ_سر(p, sz | پچھلا_INUSE);
    چوٹی = p;
    اگر ((غیرعلامتی طویل)(sz) >= (غیرعلامتی طویل)trim_حد)
      malloc_trim(چوٹی_pad);
    واپس;
  }

  سیٹ_سر(اگلا, nextsz);                    /* clear inuse bit */

  islr = 0;

  اگر (!(hd & پچھلا_INUSE))                    /* consolidate backward */
  {
    prevsz = p->پچھلا_سائز;
    p = chunk_at_انحراف(p, -((طویل) prevsz));
    sz += prevsz;

    اگر (p->fd == آخری_remainder)             /* keep as last_remainder */
      islr = 1;
    ورنہ
      unlink(p, bck, fwd);
  }

  اگر (!(inuse_ثنائی_ہندسہ_at_انحراف(اگلا, nextsz)))   /* consolidate forward */
  {
    sz += nextsz;

    اگر (!islr && اگلا->fd == آخری_remainder)  /* re-insert last_remainder */
    {
      islr = 1;
      link_آخری_remainder(p);
    }
    ورنہ
      unlink(اگلا, bck, fwd);
  }

  سیٹ_سر(p, sz | پچھلا_INUSE);
  سیٹ_foot(p, sz);
  اگر (!islr)
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
#اگر __STD_C
Void_t* rEALLOc_impl(Void_t* oldmem, size_t بائٹیں)
#ورنہ
Void_t* rEALLOc_impl(oldmem, بائٹیں) Void_t* oldmem; size_t بائٹیں;
#اگر_ختم
{
  اندرونی_سائز_T    nb;      /* padded request size */

  mchunkptr oldp;             /* chunk corresponding to oldmem */
  اندرونی_سائز_T    oldsize; /* its size */

  mchunkptr newp;             /* chunk to return */
  اندرونی_سائز_T    newsize; /* its size */
  Void_t*   newmem;           /* corresponding user mem */

  mchunkptr اگلا;             /* next contiguous chunk after oldp */
  اندرونی_سائز_T  nextsize;  /* its size */

  mchunkptr پچھلا;             /* previous contiguous chunk before oldp */
  اندرونی_سائز_T  prevsize;  /* its size */

  mchunkptr remainder;        /* holds split off extra space from newp */
  اندرونی_سائز_T  remainder_سائز;   /* its size */

  mchunkptr bck;              /* misc temp for linking */
  mchunkptr fwd;              /* misc temp for linking */

#اگر_تعریف_شدہ REALLOC_ZERO_بائٹیں_FREES
  اگر (!بائٹیں) {
	fREe_impl(oldmem);
	واپس تہی;
  }
#اگر_ختم

  اگر (بائٹیں > CONFIG_SYS_MALLOC_LEN || (طویل)بائٹیں < 0)
     واپس تہی;

  /* realloc of null is supposed to be same as malloc */
  اگر (oldmem == تہی) واپس mALLOc_impl(بائٹیں);

#اگر CONFIG_IS_ENABLED(SYS_MALLOC_F)
	اگر (!(gd->flags & GD_FLG_بھرا_MALLOC_INIT)) {
		/* This is harder to support and should not be needed */
		panic("pre-reloc realloc() is not supported");
	}
#اگر_ختم
  اگر (CONFIG_IS_ENABLED(UNIT_TEST) && malloc_testing) {
    اگر (--malloc_max_allocs < 0)
      واپس تہی;
  }

  newp    = oldp    = mem2chunk(oldmem);
  newsize = oldsize = chunksize(oldp);

  nb = request2size(بائٹیں);

#اگر HAVE_MMAP
  اگر (chunk_is_mmapped(oldp))
  {
#اگر HAVE_MREMAP
    newp = mremap_chunk(oldp, nb);
    اگر(newp) واپس chunk2mem(newp);
#اگر_ختم
    /* Note the extra SIZE_SZ overhead. */
    اگر(oldsize - سائز_SZ >= nb) واپس oldmem; /* do nothing */
    /* Must alloc, copy, free. */
    newmem = mALLOc_impl(بائٹیں);
    اگر (!newmem)
	واپس تہی; /* propagate failure */
    MALLOC_COPY(newmem, oldmem, oldsize - 2*سائز_SZ);
    munmap_chunk(oldp);
    واپس newmem;
  }
#اگر_ختم

  check_inuse_chunk(oldp);

  اگر ((طویل)(oldsize) < (طویل)(nb))
  {

    /* Try expanding forward */

    اگلا = chunk_at_انحراف(oldp, oldsize);
    اگر (اگلا == چوٹی || !inuse(اگلا))
    {
      nextsize = chunksize(اگلا);

      /* Forward into top only if a remainder */
      اگر (اگلا == چوٹی)
      {
	اگر ((طویل)(nextsize + newsize) >= (طویل)(nb + MINSIZE))
	{
	  newsize += nextsize;
	  چوٹی = chunk_at_انحراف(oldp, nb);
	  سیٹ_سر(چوٹی, (newsize - nb) | پچھلا_INUSE);
	  سیٹ_سر_سائز(oldp, nb);
	  VALGRIND_RESIZEINPLACE_بلاک(chunk2mem(oldp), 0, بائٹیں, سائز_SZ);
	  VALGRIND_MAKE_MEM_DEFINED(chunk2mem(oldp), بائٹیں);
	  واپس chunk2mem(oldp);
	}
      }

      /* Forward into next chunk */
      ورنہ اگر (((طویل)(nextsize + newsize) >= (طویل)(nb)))
      {
	unlink(اگلا, bck, fwd);
	newsize  += nextsize;
	VALGRIND_RESIZEINPLACE_بلاک(chunk2mem(oldp), 0, بائٹیں, سائز_SZ);
	VALGRIND_MAKE_MEM_DEFINED(chunk2mem(oldp), بائٹیں);
	جاؤ بانٹو;
      }
    }
    ورنہ
    {
      اگلا = تہی;
      nextsize = 0;
    }

    /* Try shifting backwards. */

    اگر (!پچھلا_inuse(oldp))
    {
      پچھلا = پچھلا_chunk(oldp);
      prevsize = chunksize(پچھلا);

      /* try forward + backward first to save a later consolidation */

      اگر (اگلا != تہی)
      {
	/* into top */
	اگر (اگلا == چوٹی)
	{
	  اگر ((طویل)(nextsize + prevsize + newsize) >= (طویل)(nb + MINSIZE))
	  {
	    unlink(پچھلا, bck, fwd);
	    newp = پچھلا;
	    newsize += prevsize + nextsize;
	    newmem = chunk2mem(newp);
	    VALGRIND_MALLOCLIKE_بلاک(newmem, بائٹیں, سائز_SZ, false);
	    MALLOC_COPY(newmem, oldmem, oldsize - سائز_SZ);
	    چوٹی = chunk_at_انحراف(newp, nb);
	    سیٹ_سر(چوٹی, (newsize - nb) | پچھلا_INUSE);
	    سیٹ_سر_سائز(newp, nb);
	    VALGRIND_FREELIKE_بلاک(oldmem, سائز_SZ);
	    واپس newmem;
	  }
	}

	/* into next chunk */
	ورنہ اگر (((طویل)(nextsize + prevsize + newsize) >= (طویل)(nb)))
	{
	  unlink(اگلا, bck, fwd);
	  unlink(پچھلا, bck, fwd);
	  newp = پچھلا;
	  newsize += nextsize + prevsize;
	  newmem = chunk2mem(newp);
	  VALGRIND_MALLOCLIKE_بلاک(newmem, بائٹیں, سائز_SZ, false);
	  MALLOC_COPY(newmem, oldmem, oldsize - سائز_SZ);
	  جاؤ بانٹو;
	}
      }

      /* backward only */
      اگر (پچھلا != تہی && (طویل)(prevsize + newsize) >= (طویل)nb)
      {
	unlink(پچھلا, bck, fwd);
	newp = پچھلا;
	newsize += prevsize;
	newmem = chunk2mem(newp);
	VALGRIND_MALLOCLIKE_بلاک(newmem, بائٹیں, سائز_SZ, false);
	MALLOC_COPY(newmem, oldmem, oldsize - سائز_SZ);
	جاؤ بانٹو;
      }
    }

    /* Must allocate */

    newmem = mALLOc_impl (بائٹیں);

    اگر (newmem == تہی)  /* propagate failure */
      واپس تہی;

    /* Avoid copy if newp is next chunk after oldp. */
    /* (This can only happen when new chunk is sbrk'ed.) */

    اگر ( (newp = mem2chunk(newmem)) == اگلا_chunk(oldp))
    {
      newsize += chunksize(newp);
      newp = oldp;
      جاؤ بانٹو;
    }

    /* Otherwise copy, free, and exit */
    MALLOC_COPY(newmem, oldmem, oldsize - سائز_SZ);
    fREe_impl(oldmem);
    واپس newmem;
  } ورنہ {
    VALGRIND_RESIZEINPLACE_بلاک(oldmem, 0, بائٹیں, سائز_SZ);
    VALGRIND_MAKE_MEM_DEFINED(oldmem, بائٹیں);
  }

 بانٹو:  /* split off extra room in old or expanded chunk */

  اگر (newsize - nb >= MINSIZE) /* split off remainder */
  {
    remainder = chunk_at_انحراف(newp, nb);
    remainder_سائز = newsize - nb;
    سیٹ_سر_سائز(newp, nb);
    سیٹ_سر(remainder, remainder_سائز | پچھلا_INUSE);
    سیٹ_inuse_ثنائی_ہندسہ_at_انحراف(remainder, remainder_سائز);
    VALGRIND_MALLOCLIKE_بلاک(chunk2mem(remainder), remainder_سائز, سائز_SZ,
			      false);
    fREe_impl(chunk2mem(remainder)); /* let free() deal with it */
  }
  ورنہ
  {
    سیٹ_سر_سائز(newp, newsize);
    سیٹ_inuse_ثنائی_ہندسہ_at_انحراف(newp, newsize);
  }

  check_inuse_chunk(newp);
  واپس chunk2mem(newp);
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
#اگر __STD_C
Void_t* mEMALIGn_impl(size_t alignment, size_t بائٹیں)
#ورنہ
Void_t* mEMALIGn_impl(alignment, بائٹیں) size_t alignment; size_t بائٹیں;
#اگر_ختم
{
  اندرونی_سائز_T    nb;      /* padded  request size */
  حرف*     m;                /* memory returned by malloc call */
  mchunkptr p;                /* corresponding chunk */
  حرف*     brk;              /* alignment point within p */
  mchunkptr newp;             /* chunk to return */
  اندرونی_سائز_T  newsize;   /* its size */
  اندرونی_سائز_T  leadsize;  /* leading space befor alignment point */
  mchunkptr remainder;        /* spare room at end to split off */
  طویل      remainder_سائز;   /* its size */

  اگر (بائٹیں > CONFIG_SYS_MALLOC_LEN || (طویل)بائٹیں < 0)
     واپس تہی;

#اگر CONFIG_IS_ENABLED(SYS_MALLOC_F)
	اگر (!(gd->flags & GD_FLG_بھرا_MALLOC_INIT)) {
		واپس memalign_simple(alignment, بائٹیں);
	}
#اگر_ختم

  /* If need less alignment than we give anyway, just relay to malloc */

  اگر (alignment <= MALLOC_ALIGNMENT) واپس mALLOc_impl(بائٹیں);

  /* Otherwise, ensure that it is at least a minimum chunk size */

  اگر (alignment <  MINSIZE) alignment = MINSIZE;

  /* Call malloc with worst case padding to hit alignment. */

  nb = request2size(بائٹیں);
  m  = (حرف*)(mALLOc_impl(nb + alignment + MINSIZE));

  /*
  * The attempt to over-allocate (with a size large enough to guarantee the
  * ability to find an aligned region within allocated memory) failed.
  *
  * Try again, this time only allocating exactly the size the user wants. If
  * the allocation now succeeds and just happens to be aligned, we can still
  * fulfill the user's request.
  */
  اگر (m == تہی) {
    size_t extra, extra2;
    /*
     * Use bytes not nb, since mALLOc internally calls request2size too, and
     * each call increases the size to allocate, to account for the header.
     */
    m  = (حرف*)(mALLOc_impl(بائٹیں));
    /* Aligned -> return it */
    اگر ((((غیرعلامتی طویل)(m)) % alignment) == 0)
      واپس m;
    /*
     * Otherwise, try again, requesting enough extra space to be able to
     * acquire alignment.
     */
    fREe_impl(m);
    /* Add in extra bytes to match misalignment of unexpanded allocation */
    extra = alignment - (((غیرعلامتی طویل)(m)) % alignment);
    m  = (حرف*)(mALLOc_impl(بائٹیں + extra));
    /*
     * m might not be the same as before. Validate that the previous value of
     * extra still works for the current value of m.
     * If (!m), extra2=alignment so 
     */
    اگر (m) {
      extra2 = alignment - (((غیرعلامتی طویل)(m)) % alignment);
      اگر (extra2 > extra) {
        fREe_impl(m);
        m = تہی;
      }
    }
    /* Fall through to original NULL check and chunk splitting logic */
  }

  اگر (m == تہی) واپس تہی; /* propagate failure */

  p = mem2chunk(m);

  اگر ((((غیرعلامتی طویل)(m)) % alignment) == 0) /* aligned */
  {
#اگر HAVE_MMAP
    اگر(chunk_is_mmapped(p))
      واپس chunk2mem(p); /* nothing more to do */
#اگر_ختم
  }
  ورنہ /* misaligned */
  {
    /*
      Find an aligned spot inside chunk.
      Since we need to give back leading space in a chunk of at
      least MINSIZE, if the first calculation places us at
      a spot with less than MINSIZE leader, we can move to the
      next aligned spot -- we've allocated enough total room so that
      this is always possible.
    */

    brk = (حرف*)mem2chunk(((غیرعلامتی طویل)(m + alignment - 1)) & -((علامتی) alignment));
    اگر ((طویل)(brk - (حرف*)(p)) < MINSIZE) brk = brk + alignment;

    newp = (mchunkptr)brk;
    leadsize = brk - (حرف*)(p);
    newsize = chunksize(p) - leadsize;

#اگر HAVE_MMAP
    اگر(chunk_is_mmapped(p))
    {
      newp->پچھلا_سائز = p->پچھلا_سائز + leadsize;
      سیٹ_سر(newp, newsize|IS_MMAPPED);
      واپس chunk2mem(newp);
    }
#اگر_ختم

    /* give back leader, use the rest */

    سیٹ_سر(newp, newsize | پچھلا_INUSE);
    سیٹ_inuse_ثنائی_ہندسہ_at_انحراف(newp, newsize);
    سیٹ_سر_سائز(p, leadsize);
    fREe_impl(chunk2mem(p));
    p = newp;
    VALGRIND_MALLOCLIKE_بلاک(chunk2mem(p), بائٹیں, سائز_SZ, false);

    أكد (newsize >= nb && (((غیرعلامتی طویل)(chunk2mem(p))) % alignment) == 0);
  }

  /* Also give back spare room at the end */

  remainder_سائز = chunksize(p) - nb;

  اگر (remainder_سائز >= (طویل)MINSIZE)
  {
    remainder = chunk_at_انحراف(p, nb);
    سیٹ_سر(remainder, remainder_سائز | پچھلا_INUSE);
    سیٹ_سر_سائز(p, nb);
    VALGRIND_MALLOCLIKE_بلاک(chunk2mem(remainder), remainder_سائز, سائز_SZ,
			      false);
    fREe_impl(chunk2mem(remainder));
  }

  check_inuse_chunk(p);
  واپس chunk2mem(p);

}

/*
    valloc just invokes memalign with alignment argument equal
    to the page size of the system (or as near to this as can
    be figured out from all the includes/defines above.)
*/

#اگر __STD_C
Void_t* vALLOc(size_t بائٹیں)
#ورنہ
Void_t* vALLOc(بائٹیں) size_t بائٹیں;
#اگر_ختم
{
  واپس mEMALIGn (malloc_getpagesize, بائٹیں);
}

/*
  pvalloc just invokes valloc for the nearest pagesize
  that will accommodate request
*/

#اگر __STD_C
Void_t* pvALLOc(size_t بائٹیں)
#ورنہ
Void_t* pvALLOc(بائٹیں) size_t بائٹیں;
#اگر_ختم
{
  size_t pagesize = malloc_getpagesize;
  واپس mEMALIGn (pagesize, (بائٹیں + pagesize - 1) & ~(pagesize - 1));
}

/*

  calloc calls malloc, then zeroes out the allocated chunk.

*/

STATIC_IF_MCHECK
#اگر __STD_C
Void_t* cALLOc_impl(size_t n, size_t elem_سائز)
#ورنہ
Void_t* cALLOc_impl(n, elem_سائز) size_t n; size_t elem_سائز;
#اگر_ختم
{
  mchunkptr p;
  اندرونی_سائز_T csz;

  اندرونی_سائز_T sz = n * elem_سائز;

  /* check if expand_top called, in which case don't need to clear */
#اگر CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
#اگر MORECORE_CLEARS
  mchunkptr oldtop = چوٹی;
  اندرونی_سائز_T oldtopsize = chunksize(چوٹی);
#اگر_ختم
#اگر_ختم
  Void_t* mem = mALLOc_impl (sz);

  اگر ((طویل)n < 0) واپس تہی;

  اگر (mem == تہی)
    واپس تہی;
  ورنہ
  {
#اگر CONFIG_IS_ENABLED(SYS_MALLOC_F)
	اگر (!(gd->flags & GD_FLG_بھرا_MALLOC_INIT)) {
		memset(mem, 0, sz);
		واپس mem;
	}
#اگر_ختم
    p = mem2chunk(mem);

    /* Two optional cases in which clearing not necessary */

#اگر HAVE_MMAP
    اگر (chunk_is_mmapped(p)) واپس mem;
#اگر_ختم

    csz = chunksize(p);

#اگر CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
#اگر MORECORE_CLEARS
    اگر (p == oldtop && csz > oldtopsize)
    {
      /* clear only the bytes from non-freshly-sbrked memory */
      csz = oldtopsize;
    }
#اگر_ختم
#اگر_ختم

    MALLOC_ZERO(mem, csz - سائز_SZ);
    VALGRIND_MAKE_MEM_DEFINED(mem, sz);
    واپس mem;
  }
}

/*

  cfree just calls free. It is needed/defined on some systems
  that pair it with calloc, presumably for odd historical reasons.

*/

#اگر !defined(اندرونی_LINUX_C_LIB) || !defined(__ELF__)
#اگر __STD_C
خالی cfree(Void_t *mem)
#ورنہ
خالی cfree(mem) Void_t *mem;
#اگر_ختم
{
  fREe(mem);
}
#اگر_ختم

#اگر_تعریف_شدہ MCHECK_ڈھیر_PROTECTION
 #شامل "mcheck_core.inc.h"
 #اگر !__STD_C
  #غلطی "must have __STD_C"
 #اگر_ختم

Void_t *mALLOc(size_t بائٹیں)
{
	mcheck_pedantic_prehook();
	size_t fullsz = mcheck_alloc_prehook(بائٹیں);
	خالی *p = mALLOc_impl(fullsz);

	اگر (!p)
		واپس p;
	واپس mcheck_alloc_posthook(p, بائٹیں);
}

خالی fREe(Void_t *mem) { fREe_impl(mcheck_free_prehook(mem)); }

Void_t *rEALLOc(Void_t *oldmem, size_t بائٹیں)
{
	mcheck_pedantic_prehook();
	اگر (بائٹیں == 0) {
		اگر (oldmem)
			fREe(oldmem);
		واپس تہی;
	}

	اگر (oldmem == تہی)
		واپس mALLOc(بائٹیں);

	خالی *p = mcheck_reallocfree_prehook(oldmem);
	size_t newsz = mcheck_alloc_prehook(بائٹیں);

	p = rEALLOc_impl(p, newsz);
	اگر (!p)
		واپس p;
	واپس mcheck_alloc_noclean_posthook(p, بائٹیں);
}

Void_t *mEMALIGn(size_t alignment, size_t بائٹیں)
{
	mcheck_pedantic_prehook();
	size_t fullsz = mcheck_memalign_prehook(alignment, بائٹیں);
	خالی *p = mEMALIGn_impl(alignment, fullsz);

	اگر (!p)
		واپس p;
	واپس mcheck_memalign_posthook(alignment, p, بائٹیں);
}

// pvALLOc, vALLOc - redirect to mEMALIGn, defined here, so they need no wrapping.

Void_t *cALLOc(size_t n, size_t elem_سائز)
{
	mcheck_pedantic_prehook();
	// NB: here is no overflow check.
	size_t fullsz = mcheck_alloc_prehook(n * elem_سائز);
	خالی *p = cALLOc_impl(1, fullsz);

	اگر (!p)
		واپس p;
	واپس mcheck_alloc_noclean_posthook(p, n * elem_سائز);
}

// mcheck API {
صحیح mcheck_pedantic(mcheck_abortfunc_t f)
{
	mcheck_initialize(f, 1);
	واپس 0;
}

صحیح mcheck(mcheck_abortfunc_t f)
{
	mcheck_initialize(f, 0);
	واپس 0;
}

خالی mcheck_check_all(خالی) { mcheck_pedantic_check(); }

شمار mcheck_status mprobe(خالی *__ptr) { واپس mcheck_mprobe(__ptr); }
// mcheck API }
#اگر_ختم

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

#اگر __STD_C
صحیح malloc_trim(size_t pad)
#ورنہ
صحیح malloc_trim(pad) size_t pad;
#اگر_ختم
{
  طویل  چوٹی_سائز;        /* Amount of top-most memory */
  طویل  extra;           /* Amount to release */
  حرف* current_brk;     /* address returned by pre-check sbrk call */
  حرف* new_brk;         /* address returned by negative sbrk call */

  غیرعلامتی طویل pagesz = malloc_getpagesize;

  چوٹی_سائز = chunksize(چوٹی);
  extra = ((چوٹی_سائز - pad - MINSIZE + (pagesz-1)) / pagesz - 1) * pagesz;

  اگر (extra < (طویل)pagesz)  /* Not enough memory to release */
    واپس 0;

  ورنہ
  {
    /* Test to make sure no one else called sbrk */
    current_brk = (حرف*)(MORECORE (0));
    اگر (current_brk != (حرف*)(چوٹی) + چوٹی_سائز)
      واپس 0;     /* Apparently we don't own memory; must fail */

    ورنہ
    {
      new_brk = (حرف*)(MORECORE (-extra));

      اگر (new_brk == (حرف*)(MORECORE_FAILURE)) /* sbrk failed? */
      {
	/* Try to figure out what we have */
	current_brk = (حرف*)(MORECORE (0));
	چوٹی_سائز = current_brk - (حرف*)چوٹی;
	اگر (چوٹی_سائز >= (طویل)MINSIZE) /* if not, we are very very dead! */
	{
	  sbrked_mem = current_brk - sbrk_بنیاد;
	  سیٹ_سر(چوٹی, چوٹی_سائز | پچھلا_INUSE);
	}
	check_chunk(چوٹی);
	واپس 0;
      }

      ورنہ
      {
	/* Success. Adjust top accordingly. */
	سیٹ_سر(چوٹی, (چوٹی_سائز - extra) | پچھلا_INUSE);
	sbrked_mem -= extra;
	check_chunk(چوٹی);
	واپس 1;
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

#اگر __STD_C
size_t malloc_usable_size(Void_t* mem)
#ورنہ
size_t malloc_usable_size(mem) Void_t* mem;
#اگر_ختم
{
  mchunkptr p;
  اگر (mem == تہی)
    واپس 0;
  ورنہ
  {
    p = mem2chunk(mem);
    اگر(!chunk_is_mmapped(p))
    {
      اگر (!inuse(p)) واپس 0;
      check_inuse_chunk(p);
      واپس chunksize(p) - سائز_SZ;
    }
    واپس chunksize(p) - 2*سائز_SZ;
  }
}

/* Utility to update current_mallinfo for malloc_stats and mallinfo() */

#اگر_تعریف_شدہ DEBUG
ساکن خالی malloc_update_mallinfo(خالی)
{
  صحیح i;
  mbinptr b;
  mchunkptr p;
#اگر_تعریف_شدہ DEBUG
  mchunkptr q;
#اگر_ختم

  اندرونی_سائز_T avail = chunksize(چوٹی);
  صحیح   navail = ((طویل)(avail) >= (طویل)MINSIZE)? 1 : 0;

  برائے (i = 1; i < NAV; ++i)
  {
    b = bin_at(i);
    برائے (p = آخری(b); p != b; p = p->bk)
    {
#اگر_تعریف_شدہ DEBUG
      check_free_chunk(p);
      برائے (q = اگلا_chunk(p);
	   q < چوٹی && inuse(q) && (طویل)(chunksize(q)) >= (طویل)MINSIZE;
	   q = اگلا_chunk(q))
	check_inuse_chunk(q);
#اگر_ختم
      avail += chunksize(p);
      navail++;
    }
  }

  current_mallinfo.ordblks = navail;
  current_mallinfo.uordblks = sbrked_mem - avail;
  current_mallinfo.fordblks = avail;
  current_mallinfo.hblks = n_mmaps;
  current_mallinfo.hblkhd = mmapped_mem;
  current_mallinfo.keepcost = chunksize(چوٹی);

}
#اگر_ختم	/* DEBUG */

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

#اگر_تعریف_شدہ DEBUG
خالی malloc_stats(خالی)
{
  malloc_update_mallinfo();
  لکھو("max system bytes = %10u\n",
	  (غیرعلامتی صحیح)(max_کل_mem));
  لکھو("system bytes     = %10u\n",
	  (غیرعلامتی صحیح)(sbrked_mem + mmapped_mem));
  لکھو("in use bytes     = %10u\n",
	  (غیرعلامتی صحیح)(current_mallinfo.uordblks + mmapped_mem));
#اگر HAVE_MMAP
  لکھو("max mmap regions = %10u\n",
	  (غیرعلامتی صحیح)max_n_mmaps);
#اگر_ختم
}
#اگر_ختم	/* DEBUG */

/*
  mallinfo returns a copy of updated current mallinfo.
*/

#اگر_تعریف_شدہ DEBUG
ساخت mallinfo mALLINFo(خالی)
{
  malloc_update_mallinfo();
  واپس current_mallinfo;
}
#اگر_ختم	/* DEBUG */

/*
  mallopt:

    mallopt is the general SVID/XPG interface to tunable parameters.
    The format is to provide a (parameter-number, parameter-value) pair.
    mallopt then sets the corresponding parameter to the argument
    value if it can (i.e., so long as the value is meaningful),
    and returns 1 if successful else 0.

    See descriptions of tunable parameters above.

*/

#اگر __STD_C
صحیح mALLOPt(صحیح param_نمبر, صحیح قدر)
#ورنہ
صحیح mALLOPt(param_نمبر, قدر) صحیح param_نمبر; صحیح قدر;
#اگر_ختم
{
  انتخاب(param_نمبر)
  {
    حالت M_TRIM_THRESHOLD:
      trim_حد = قدر; واپس 1;
    حالت M_TOP_PAD:
      چوٹی_pad = قدر; واپس 1;
    حالت M_MMAP_THRESHOLD:
      mmap_حد = قدر; واپس 1;
    حالت M_MMAP_MAX:
#اگر HAVE_MMAP
      n_mmaps_max = قدر; واپس 1;
#ورنہ
      اگر (قدر != 0) واپس 0; ورنہ  n_mmaps_max = قدر; واپس 1;
#اگر_ختم

    طےشدہ:
      واپس 0;
  }
}

صحیح initf_malloc(خالی)
{
#اگر CONFIG_IS_ENABLED(SYS_MALLOC_F)
	أكد(gd->malloc_بنیاد);	/* Set up by crt0.S */
	gd->malloc_حد_بندی = CONFIG_VAL(SYS_MALLOC_F_LEN);
	gd->malloc_ptr = 0;
#اگر_ختم

	واپس 0;
}

خالی malloc_enable_testing(صحیح max_allocs)
{
	malloc_testing = true;
	malloc_max_allocs = max_allocs;
}

خالی malloc_disable_testing(خالی)
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
