// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012 The Chromium OS Authors.
 *
 * (C) Copyright 2011
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef USE_HOSTCC
#ଅନ୍ତର୍ଭାଵ <command.h>
#ଅନ୍ତର୍ଭାଵ <dm.h>
#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <malloc.h>
#ଅନ୍ତର୍ଭାଵ <mapmem.h>
#ଅନ୍ତର୍ଭାଵ <hw_sha.h>
#ଅନ୍ତର୍ଭାଵ <asm/cache.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>
#ଅନ୍ତର୍ଭାଵ <asm/io.h>
#ଅନ୍ତର୍ଭାଵ <linux/errno.h>
#ଅନ୍ତର୍ଭାଵ <u-boot/hash.h>
#ଅନ୍ୟଥା
#ଅନ୍ତର୍ଭାଵ "mkimage.h"
#ଅନ୍ତର୍ଭାଵ <linux/compiler_attributes.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <linux/kconfig.h>
#endif /* !USE_HOSTCC*/

#ଅନ୍ତର୍ଭାଵ <hash.h>
#ଅନ୍ତର୍ଭାଵ <image.h>
#ଅନ୍ତର୍ଭାଵ <u-boot/crc.h>
#ଅନ୍ତର୍ଭାଵ <u-boot/sha1.h>
#ଅନ୍ତର୍ଭାଵ <u-boot/sha256.h>
#ଅନ୍ତର୍ଭାଵ <u-boot/sha512.h>
#ଅନ୍ତର୍ଭାଵ <u-boot/md5.h>
#ଅନ୍ତର୍ଭାଵ <u-boot/sm3.h>

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused hash_init_sha1(ସଂରଚନା hash_algo *algo, ଶୂନ୍ୟ **ctxp)
{
	sha1_context *ctx = malloc(ଆକାର(sha1_context));
	sha1_starts(ctx);
	*ctxp = ctx;
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused hash_update_sha1(ସଂରଚନା hash_algo *algo, ଶୂନ୍ୟ *ctx,
					   ଅଚଳ ଶୂନ୍ୟ *buf, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା size,
					   ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_last)
{
	sha1_update((sha1_context *)ctx, buf, size);
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused hash_finish_sha1(ସଂରଚନା hash_algo *algo, ଶୂନ୍ୟ *ctx,
					   ଶୂନ୍ୟ *dest_buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	ଯଦି (size < algo->digest_size)
		ଫେରନ୍ତୁ -1;

	sha1_finish((sha1_context *)ctx, dest_buf);
	ମୁକ୍ତ(ctx);
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused hash_init_sha256(ସଂରଚନା hash_algo *algo, ଶୂନ୍ୟ **ctxp)
{
	sha256_context *ctx = malloc(ଆକାର(sha256_context));
	sha256_starts(ctx);
	*ctxp = ctx;
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused hash_update_sha256(ସଂରଚନା hash_algo *algo, ଶୂନ୍ୟ *ctx,
					     ଅଚଳ ଶୂନ୍ୟ *buf, uint size,
					     ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_last)
{
	sha256_update((sha256_context *)ctx, buf, size);
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused hash_finish_sha256(ସଂରଚନା hash_algo *algo, ଶୂନ୍ୟ *ctx,
					     ଶୂନ୍ୟ *dest_buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	ଯଦି (size < algo->digest_size)
		ଫେରନ୍ତୁ -1;

	sha256_finish((sha256_context *)ctx, dest_buf);
	ମୁକ୍ତ(ctx);
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused hash_init_sha384(ସଂରଚନା hash_algo *algo, ଶୂନ୍ୟ **ctxp)
{
	sha512_context *ctx = malloc(ଆକାର(sha512_context));
	sha384_starts(ctx);
	*ctxp = ctx;
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused hash_update_sha384(ସଂରଚନା hash_algo *algo, ଶୂନ୍ୟ *ctx,
					     ଅଚଳ ଶୂନ୍ୟ *buf, uint size,
					     ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_last)
{
	sha384_update((sha512_context *)ctx, buf, size);
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused hash_finish_sha384(ସଂରଚନା hash_algo *algo, ଶୂନ୍ୟ *ctx,
					     ଶୂନ୍ୟ *dest_buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	ଯଦି (size < algo->digest_size)
		ଫେରନ୍ତୁ -1;

	sha384_finish((sha512_context *)ctx, dest_buf);
	ମୁକ୍ତ(ctx);
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused hash_init_sha512(ସଂରଚନା hash_algo *algo, ଶୂନ୍ୟ **ctxp)
{
	sha512_context *ctx = malloc(ଆକାର(sha512_context));
	sha512_starts(ctx);
	*ctxp = ctx;
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused hash_update_sha512(ସଂରଚନା hash_algo *algo, ଶୂନ୍ୟ *ctx,
					     ଅଚଳ ଶୂନ୍ୟ *buf, uint size,
					     ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_last)
{
	sha512_update((sha512_context *)ctx, buf, size);
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused hash_finish_sha512(ସଂରଚନା hash_algo *algo, ଶୂନ୍ୟ *ctx,
					     ଶୂନ୍ୟ *dest_buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	ଯଦି (size < algo->digest_size)
		ଫେରନ୍ତୁ -1;

	sha512_finish((sha512_context *)ctx, dest_buf);
	ମୁକ୍ତ(ctx);
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused hash_init_sm3(ସଂରଚନା hash_algo *algo, ଶୂନ୍ୟ **ctxp)
{
	ସଂରଚନା sm3_context *ctx = malloc(ଆକାର(ସଂରଚନା sm3_context));

	sm3_init(ctx);
	*ctxp = ctx;
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused hash_update_sm3(ସଂରଚନା hash_algo *algo, ଶୂନ୍ୟ *ctx,
					  ଅଚଳ ଶୂନ୍ୟ *buf, uint size,
					  ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_last)
{
	sm3_update((ସଂରଚନା sm3_context *)ctx, buf, size);
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused hash_finish_sm3(ସଂରଚନା hash_algo *algo, ଶୂନ୍ୟ *ctx,
					  ଶୂନ୍ୟ *dest_buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	ଯଦି (size < algo->digest_size)
		ଫେରନ୍ତୁ -1;

	sm3_final((ସଂରଚନା sm3_context *)ctx, dest_buf);
	ମୁକ୍ତ(ctx);
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused hash_init_crc16_ccitt(ସଂରଚନା hash_algo *algo,
						ଶୂନ୍ୟ **ctxp)
{
	uint16_t *ctx = malloc(ଆକାର(uint16_t));
	*ctx = 0;
	*ctxp = ctx;
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused hash_update_crc16_ccitt(ସଂରଚନା hash_algo *algo,
						  ଶୂନ୍ୟ *ctx, ଅଚଳ ଶୂନ୍ୟ *buf,
						  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା size,
						  ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_last)
{
	*((uint16_t *)ctx) = crc16_ccitt(*((uint16_t *)ctx), buf, size);
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused hash_finish_crc16_ccitt(ସଂରଚନା hash_algo *algo,
						  ଶୂନ୍ୟ *ctx, ଶୂନ୍ୟ *dest_buf,
						  ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	ଯଦି (size < algo->digest_size)
		ଫେରନ୍ତୁ -1;

	*((uint16_t *)dest_buf) = *((uint16_t *)ctx);
	ମୁକ୍ତ(ctx);
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused hash_init_crc32(ସଂରଚନା hash_algo *algo, ଶୂନ୍ୟ **ctxp)
{
	uint32_t *ctx = malloc(ଆକାର(uint32_t));
	*ctx = 0;
	*ctxp = ctx;
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused hash_update_crc32(ସଂରଚନା hash_algo *algo, ଶୂନ୍ୟ *ctx,
					    ଅଚଳ ଶୂନ୍ୟ *buf, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା size,
					    ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_last)
{
	*((uint32_t *)ctx) = crc32(*((uint32_t *)ctx), buf, size);
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused hash_finish_crc32(ସଂରଚନା hash_algo *algo, ଶୂନ୍ୟ *ctx,
					    ଶୂନ୍ୟ *dest_buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	ଯଦି (size < algo->digest_size)
		ଫେରନ୍ତୁ -1;

	*((uint32_t *)dest_buf) = *((uint32_t *)ctx);
	ମୁକ୍ତ(ctx);
	ଫେରନ୍ତୁ 0;
}

/*
 * These are the hash algorithms we support.  If we have hardware acceleration
 * is enable we will use that, otherwise a software version of the algorithm.
 * Note that algorithm names must be in lower case.
 */
ସ୍ଥିର ସଂରଚନା hash_algo hash_algo[] = {
#ଯଦି CONFIG_IS_ENABLED(MD5)
	{
		.name		= "md5",
		.digest_size	= MD5_SUM_LEN,
		.chunk_size	= CHUNKSZ_MD5,
		.hash_func_ws	= md5_wd,
	},
#endif
#ଯଦି CONFIG_IS_ENABLED(SHA1)
	{
		.name		= "sha1",
		.digest_size	= SHA1_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA1,
#ଯଦି CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha1,
#ଅନ୍ୟଥା
		.hash_func_ws	= sha1_csum_wd,
#endif
#ଯଦି CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#ଅନ୍ୟଥା
		.hash_init	= hash_init_sha1,
		.hash_update	= hash_update_sha1,
		.hash_finish	= hash_finish_sha1,
#endif
	},
#endif
#ଯଦି CONFIG_IS_ENABLED(SHA256)
	{
		.name		= "sha256",
		.digest_size	= SHA256_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA256,
#ଯଦି CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha256,
#ଅନ୍ୟଥା
		.hash_func_ws	= sha256_csum_wd,
#endif
#ଯଦି CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#ଅନ୍ୟଥା
		.hash_init	= hash_init_sha256,
		.hash_update	= hash_update_sha256,
		.hash_finish	= hash_finish_sha256,
#endif
	},
#endif
#ଯଦି CONFIG_IS_ENABLED(SHA384)
	{
		.name		= "sha384",
		.digest_size	= SHA384_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA384,
#ଯଦି CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha384,
#ଅନ୍ୟଥା
		.hash_func_ws	= sha384_csum_wd,
#endif
#ଯଦି CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#ଅନ୍ୟଥା
		.hash_init	= hash_init_sha384,
		.hash_update	= hash_update_sha384,
		.hash_finish	= hash_finish_sha384,
#endif
	},
#endif
#ଯଦି CONFIG_IS_ENABLED(SHA512)
	{
		.name		= "sha512",
		.digest_size	= SHA512_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA512,
#ଯଦି CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha512,
#ଅନ୍ୟଥା
		.hash_func_ws	= sha512_csum_wd,
#endif
#ଯଦି CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#ଅନ୍ୟଥା
		.hash_init	= hash_init_sha512,
		.hash_update	= hash_update_sha512,
		.hash_finish	= hash_finish_sha512,
#endif
	},
#endif
#ଯଦି CONFIG_IS_ENABLED(SM3)
	{
		.name		= "sm3_256",
		.digest_size	= SM3_DIGEST_SIZE,
		.chunk_size	= SM3_BLOCK_SIZE,
		.hash_func_ws	= sm3_csum_wd,
		.hash_init	= hash_init_sm3,
		.hash_update	= hash_update_sm3,
		.hash_finish	= hash_finish_sm3,
	},
#endif
#ଯଦି CONFIG_IS_ENABLED(CRC16)
	{
		.name		= "crc16-ccitt",
		.digest_size	= 2,
		.chunk_size	= CHUNKSZ,
		.hash_func_ws	= crc16_ccitt_wd_buf,
		.hash_init	= hash_init_crc16_ccitt,
		.hash_update	= hash_update_crc16_ccitt,
		.hash_finish	= hash_finish_crc16_ccitt,
	},
#endif
#ଯଦି CONFIG_IS_ENABLED(CRC8) && IS_ENABLED(CONFIG_HASH_CRC8)
	{
		.name		= "crc8",
		.digest_size	= 1,
		.chunk_size	= CHUNKSZ_CRC32,
		.hash_func_ws	= crc8_wd_buf,
	},
#endif
#ଯଦି CONFIG_IS_ENABLED(CRC32)
	{
		.name		= "crc32",
		.digest_size	= 4,
		.chunk_size	= CHUNKSZ_CRC32,
		.hash_func_ws	= crc32_wd_buf,
		.hash_init	= hash_init_crc32,
		.hash_update	= hash_update_crc32,
		.hash_finish	= hash_finish_crc32,
	},
#endif
};

/* Try to minimize code size for boards that don't want much hashing */
#ଯଦି CONFIG_IS_ENABLED(SHA256) || IS_ENABLED(CONFIG_CMD_SHA1SUM) || \
	CONFIG_IS_ENABLED(CRC32_VERIFY) || IS_ENABLED(CONFIG_CMD_HASH) || \
	CONFIG_IS_ENABLED(SHA384) || CONFIG_IS_ENABLED(SHA512) || \
	IS_ENABLED(CONFIG_CMD_MD5SUM) || CONFIG_IS_ENABLED(SM3)
#define multi_hash()	1
#ଅନ୍ୟଥା
#define multi_hash()	0
#endif

ପୂର୍ଣ୍ଣସଂଖ୍ୟା hash_lookup_algo(ଅଚଳ ଅକ୍ଷର *algo_name, ସଂରଚନା hash_algo **algop)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପାଇଁ (i = 0; i < ARRAY_SIZE(hash_algo); i++) {
		ଯଦି (!strcmp(algo_name, hash_algo[i].name)) {
			*algop = &hash_algo[i];
			ଫେରନ୍ତୁ 0;
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	ଫେରନ୍ତୁ -EPROTONOSUPPORT;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା hash_progressive_lookup_algo(ଅଚଳ ଅକ୍ଷର *algo_name,
				 ସଂରଚନା hash_algo **algop)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପାଇଁ (i = 0; i < ARRAY_SIZE(hash_algo); i++) {
		ଯଦି (!strcmp(algo_name, hash_algo[i].name)) {
			ଯଦି (hash_algo[i].hash_init) {
				*algop = &hash_algo[i];
				ଫେରନ୍ତୁ 0;
			}
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	ଫେରନ୍ତୁ -EPROTONOSUPPORT;
}

#ifndef USE_HOSTCC
ପୂର୍ଣ୍ଣସଂଖ୍ୟା hash_parse_string(ଅଚଳ ଅକ୍ଷର *algo_name, ଅଚଳ ଅକ୍ଷର *str, uint8_t *result)
{
	ସଂରଚନା hash_algo *algo;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ret = hash_lookup_algo(algo_name, &algo);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;

	ପାଇଁ (i = 0; i < algo->digest_size; i++) {
		ଅକ୍ଷର chr[3];

		strlcpy(chr, &str[i * 2], 3);
		result[i] = hextoul(chr, ଖାଲି);
	}

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା hash_block(ଅଚଳ ଅକ୍ଷର *algo_name, ଅଚଳ ଶୂନ୍ୟ *data, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା len,
	       uint8_t *output, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *output_size)
{
	ସଂରଚନା hash_algo *algo;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ret = hash_lookup_algo(algo_name, &algo);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;

	ଯଦି (output_size && *output_size < algo->digest_size) {
		debug("Output buffer size %d too small (need %d bytes)",
		      *output_size, algo->digest_size);
		ଫେରନ୍ତୁ -ENOSPC;
	}
	ଯଦି (output_size)
		*output_size = algo->digest_size;
	algo->hash_func_ws(data, len, output, algo->chunk_size);

	ଫେରନ୍ତୁ 0;
}

#ଯଦି !defined(CONFIG_XPL_BUILD) && (defined(CONFIG_CMD_HASH) || \
	defined(CONFIG_CMD_SHA1SUM) || defined(CONFIG_CMD_CRC32)) || \
	defined(CONFIG_CMD_MD5SUM)
/**
 * store_result: Store the resulting sum to an address or variable
 *
 * @algo:		Hash algorithm being used
 * @sum:		Hash digest (algo->digest_size bytes)
 * @dest:		Destination, interpreted as a hex address if it starts
 *			with * (or allow_env_vars is 0) or otherwise as an
 *			environment variable.
 * @allow_env_vars:	non-zero to permit storing the result to an
 *			variable environment
 */
ସ୍ଥିର ଶୂନ୍ୟ store_result(ସଂରଚନା hash_algo *algo, ଅଚଳ uint8_t *sum,
			 ଅଚଳ ଅକ୍ଷର *dest, ପୂର୍ଣ୍ଣସଂଖ୍ୟା allow_env_vars)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା env_var = 0;

	/*
	 * If environment variables are allowed, then we assume that 'dest'
	 * is an environment variable, unless it starts with *, in which
	 * case we assume it is an address. If not allowed, it is always an
	 * address. This is to support the crc32 command.
	 */
	ଯଦି (allow_env_vars) {
		ଯଦି (*dest == '*')
			dest++;
		ଅନ୍ୟଥା
			env_var = 1;
	}

	ଯଦି (env_var) {
		ଅକ୍ଷର str_output[HASH_MAX_DIGEST_SIZE * 2 + 1];
		ଅକ୍ଷର *str_ptr = str_output;

		ପାଇଁ (i = 0; i < algo->digest_size; i++) {
			sprintf(str_ptr, "%02x", sum[i]);
			str_ptr += 2;
		}
		*str_ptr = '\0';
		env_set(dest, str_output);
	} ଅନ୍ୟଥା {
		ulong addr;
		ଶୂନ୍ୟ *buf;

		addr = hextoul(dest, ଖାଲି);
		buf = map_sysmem(addr, algo->digest_size);
		memcpy(buf, sum, algo->digest_size);
		unmap_sysmem(buf);
	}
}

/**
 * parse_verify_sum: Parse a hash verification parameter
 *
 * @algo:		Hash algorithm being used
 * @verify_str:		Argument to parse. If it starts with * then it is
 *			interpreted as a hex address containing the hash.
 *			If the length is exactly the right number of hex digits
 *			for the digest size, then we assume it is a hex digest.
 *			Otherwise we assume it is an environment variable, and
 *			look up its value (it must contain a hex digest).
 * @vsum:		Returns binary digest value (algo->digest_size bytes)
 * @allow_env_vars:	non-zero to permit storing the result to an environment
 *			variable. If 0 then verify_str is assumed to be an
 *			address, and the * prefix is not expected.
 * Return: 0 if ok, non-zero on error
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_verify_sum(ସଂରଚନା hash_algo *algo, ଅକ୍ଷର *verify_str,
			    uint8_t *vsum, ପୂର୍ଣ୍ଣସଂଖ୍ୟା allow_env_vars)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା env_var = 0;

	/* See comment above in store_result() */
	ଯଦି (allow_env_vars) {
		ଯଦି (*verify_str == '*')
			verify_str++;
		ଅନ୍ୟଥା
			env_var = 1;
	}

	ଯଦି (!env_var) {
		ulong addr;
		ଶୂନ୍ୟ *buf;

		addr = hextoul(verify_str, ଖାଲି);
		buf = map_sysmem(addr, algo->digest_size);
		memcpy(vsum, buf, algo->digest_size);
	} ଅନ୍ୟଥା {
		ଅକ୍ଷର *vsum_str;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା digits = algo->digest_size * 2;

		/*
		 * As with the original code from sha1sum.c, we assume that a
		 * string which matches the digest size exactly is a hex
		 * string and not an environment variable.
		 */
		ଯଦି (strlen(verify_str) == digits)
			vsum_str = verify_str;
		ଅନ୍ୟଥା {
			vsum_str = env_get(verify_str);
			ଯଦି (vsum_str == ଖାଲି || strlen(vsum_str) != digits) {
				printf("Expected %d hex digits in env var\n",
				       digits);
				ଫେରନ୍ତୁ 1;
			}
		}

		hash_parse_string(algo->name, vsum_str, vsum);
	}
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଶୂନ୍ୟ hash_show(ସଂରଚନା hash_algo *algo, ulong addr, ulong len, uint8_t *output)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	printf("%s for %08lx ... %08lx ==> ", algo->name, addr, addr + len - 1);
	ପାଇଁ (i = 0; i < algo->digest_size; i++)
		printf("%02x", output[i]);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା hash_command(ଅଚଳ ଅକ୍ଷର *algo_name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ସଂରଚନା cmd_tbl *cmdtp,
		 ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag, ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *ଅଚଳ argv[])
{
	ulong addr, len;

	ଯଦି ((argc < 2) || ((flags & HASH_FLAG_VERIFY) && (argc < 3)))
		ଫେରନ୍ତୁ CMD_RET_USAGE;

	addr = hextoul(*argv++, ଖାଲି);
	len = hextoul(*argv++, ଖାଲି);

	ଯଦି (multi_hash()) {
		ସଂରଚନା hash_algo *algo;
		u8 *output;
		uint8_t vsum[HASH_MAX_DIGEST_SIZE];
		ଶୂନ୍ୟ *buf;

		ଯଦି (hash_lookup_algo(algo_name, &algo)) {
			printf("Unknown hash algorithm '%s'\n", algo_name);
			ଫେରନ୍ତୁ CMD_RET_USAGE;
		}
		argc -= 2;

		ଯଦି (algo->digest_size > HASH_MAX_DIGEST_SIZE) {
			puts("HASH_MAX_DIGEST_SIZE exceeded\n");
			ଫେରନ୍ତୁ 1;
		}

		output = memalign(ARCH_DMA_MINALIGN,
				  ଆକାର(uint32_t) * HASH_MAX_DIGEST_SIZE);
		ଯଦି (!output)
			ଫେରନ୍ତୁ CMD_RET_FAILURE;

		buf = map_sysmem(addr, len);
		ଯଦି (CONFIG_IS_ENABLED(DM_HASH)) {
			ଗଣନା HASH_ALGO hash_algo;
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

			hash_algo = hash_algo_lookup_by_name(algo_name);
			ଯଦି (hash_algo != HASH_ALGO_INVALID) {
				ret = hash_digest_wd_lookup(hash_algo, buf, len,
							    output,
							    algo->chunk_size);
				ଯଦି (ret && ret != -ENODEV && ret != -EOPNOTSUPP) {
					unmap_sysmem(buf);
					ମୁକ୍ତ(output);
					ଫେରନ୍ତୁ CMD_RET_FAILURE;
				}
				ଯଦି (!ret)
					ଯାଅ done;
			}
		}
		algo->hash_func_ws(buf, len, output, algo->chunk_size);
done:
		unmap_sysmem(buf);

		/* Try to avoid code bloat when verify is not needed */
#ଯଦି defined(CONFIG_CRC32_VERIFY) || defined(CONFIG_SHA1SUM_VERIFY) || \
	defined(CONFIG_MD5SUM_VERIFY) || defined(CONFIG_HASH_VERIFY)
		ଯଦି (flags & HASH_FLAG_VERIFY) {
#ଅନ୍ୟଥା
		ଯଦି (0) {
#endif
			ଯଦି (parse_verify_sum(algo, *argv, vsum,
					flags & HASH_FLAG_ENV)) {
				printf("ERROR: %s does not contain a valid "
					"%s sum\n", *argv, algo->name);
				ମୁକ୍ତ(output);
				ଫେରନ୍ତୁ 1;
			}
			ଯଦି (memcmp(output, vsum, algo->digest_size) != 0) {
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

				hash_show(algo, addr, len, output);
				printf(" != ");
				ପାଇଁ (i = 0; i < algo->digest_size; i++)
					printf("%02x", vsum[i]);
				puts(" ** ERROR **\n");
				ମୁକ୍ତ(output);
				ଫେରନ୍ତୁ 1;
			}
		} ଅନ୍ୟଥା {
			hash_show(algo, addr, len, output);
			printf("\n");

			ଯଦି (argc) {
				store_result(algo, output, *argv,
					flags & HASH_FLAG_ENV);
			}
		}

		ମୁକ୍ତ(output);

	/* Horrible code size hack for boards that just want crc32 */
	} ଅନ୍ୟଥା {
		ulong crc;
		ulong *ptr;

		crc = crc32_wd(0, (ଅଚଳ uchar *)addr, len, CHUNKSZ_CRC32);

		printf("CRC32 for %08lx ... %08lx ==> %08lx\n",
				addr, addr + len - 1, crc);

		ଯଦି (argc >= 3) {
			ptr = (ulong *)hextoul(argv[0], ଖାଲି);
			*ptr = crc;
		}
	}

	ଫେରନ୍ତୁ 0;
}
#endif /* CONFIG_CMD_HASH || CONFIG_CMD_SHA1SUM || CONFIG_CMD_CRC32) */
#endif /* !USE_HOSTCC */
