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
#ಅನ್ತರ್ಭಾವ <command.h>
#ಅನ್ತರ್ಭಾವ <dm.h>
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <hw_sha.h>
#ಅನ್ತರ್ಭಾವ <asm/cache.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <asm/io.h>
#ಅನ್ತರ್ಭಾವ <linux/errno.h>
#ಅನ್ತರ್ಭಾವ <u-boot/hash.h>
#ಅನ್ಯಥಾ
#ಅನ್ತರ್ಭಾವ "mkimage.h"
#ಅನ್ತರ್ಭಾವ <linux/compiler_attributes.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <linux/kconfig.h>
#endif /* !USE_HOSTCC*/

#ಅನ್ತರ್ಭಾವ <hash.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <u-boot/crc.h>
#ಅನ್ತರ್ಭಾವ <u-boot/sha1.h>
#ಅನ್ತರ್ಭಾವ <u-boot/sha256.h>
#ಅನ್ತರ್ಭಾವ <u-boot/sha512.h>
#ಅನ್ತರ್ಭಾವ <u-boot/md5.h>
#ಅನ್ತರ್ಭಾವ <u-boot/sm3.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __maybe_unused hash_init_sha1(ಸಂರಚನಾ hash_algo *algo, ಶೂನ್ಯ **ctxp)
{
	sha1_context *ctx = malloc(ಪರಿಮಾಣ(sha1_context));
	sha1_starts(ctx);
	*ctxp = ctx;
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __maybe_unused hash_update_sha1(ಸಂರಚನಾ hash_algo *algo, ಶೂನ್ಯ *ctx,
					   ಧ್ರುವ ಶೂನ್ಯ *buf, unsigned ಪೂರ್ಣಾಙ್ಕ size,
					   ಪೂರ್ಣಾಙ್ಕ is_last)
{
	sha1_update((sha1_context *)ctx, buf, size);
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __maybe_unused hash_finish_sha1(ಸಂರಚನಾ hash_algo *algo, ಶೂನ್ಯ *ctx,
					   ಶೂನ್ಯ *dest_buf, ಪೂರ್ಣಾಙ್ಕ size)
{
	ಯದಿ (size < algo->digest_size)
		ಪ್ರತ್ಯಾಗಮ -1;

	sha1_finish((sha1_context *)ctx, dest_buf);
	ಮುಕ್ತ(ctx);
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __maybe_unused hash_init_sha256(ಸಂರಚನಾ hash_algo *algo, ಶೂನ್ಯ **ctxp)
{
	sha256_context *ctx = malloc(ಪರಿಮಾಣ(sha256_context));
	sha256_starts(ctx);
	*ctxp = ctx;
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __maybe_unused hash_update_sha256(ಸಂರಚನಾ hash_algo *algo, ಶೂನ್ಯ *ctx,
					     ಧ್ರುವ ಶೂನ್ಯ *buf, uint size,
					     ಪೂರ್ಣಾಙ್ಕ is_last)
{
	sha256_update((sha256_context *)ctx, buf, size);
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __maybe_unused hash_finish_sha256(ಸಂರಚನಾ hash_algo *algo, ಶೂನ್ಯ *ctx,
					     ಶೂನ್ಯ *dest_buf, ಪೂರ್ಣಾಙ್ಕ size)
{
	ಯದಿ (size < algo->digest_size)
		ಪ್ರತ್ಯಾಗಮ -1;

	sha256_finish((sha256_context *)ctx, dest_buf);
	ಮುಕ್ತ(ctx);
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __maybe_unused hash_init_sha384(ಸಂರಚನಾ hash_algo *algo, ಶೂನ್ಯ **ctxp)
{
	sha512_context *ctx = malloc(ಪರಿಮಾಣ(sha512_context));
	sha384_starts(ctx);
	*ctxp = ctx;
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __maybe_unused hash_update_sha384(ಸಂರಚನಾ hash_algo *algo, ಶೂನ್ಯ *ctx,
					     ಧ್ರುವ ಶೂನ್ಯ *buf, uint size,
					     ಪೂರ್ಣಾಙ್ಕ is_last)
{
	sha384_update((sha512_context *)ctx, buf, size);
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __maybe_unused hash_finish_sha384(ಸಂರಚನಾ hash_algo *algo, ಶೂನ್ಯ *ctx,
					     ಶೂನ್ಯ *dest_buf, ಪೂರ್ಣಾಙ್ಕ size)
{
	ಯದಿ (size < algo->digest_size)
		ಪ್ರತ್ಯಾಗಮ -1;

	sha384_finish((sha512_context *)ctx, dest_buf);
	ಮುಕ್ತ(ctx);
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __maybe_unused hash_init_sha512(ಸಂರಚನಾ hash_algo *algo, ಶೂನ್ಯ **ctxp)
{
	sha512_context *ctx = malloc(ಪರಿಮಾಣ(sha512_context));
	sha512_starts(ctx);
	*ctxp = ctx;
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __maybe_unused hash_update_sha512(ಸಂರಚನಾ hash_algo *algo, ಶೂನ್ಯ *ctx,
					     ಧ್ರುವ ಶೂನ್ಯ *buf, uint size,
					     ಪೂರ್ಣಾಙ್ಕ is_last)
{
	sha512_update((sha512_context *)ctx, buf, size);
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __maybe_unused hash_finish_sha512(ಸಂರಚನಾ hash_algo *algo, ಶೂನ್ಯ *ctx,
					     ಶೂನ್ಯ *dest_buf, ಪೂರ್ಣಾಙ್ಕ size)
{
	ಯದಿ (size < algo->digest_size)
		ಪ್ರತ್ಯಾಗಮ -1;

	sha512_finish((sha512_context *)ctx, dest_buf);
	ಮುಕ್ತ(ctx);
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __maybe_unused hash_init_sm3(ಸಂರಚನಾ hash_algo *algo, ಶೂನ್ಯ **ctxp)
{
	ಸಂರಚನಾ sm3_context *ctx = malloc(ಪರಿಮಾಣ(ಸಂರಚನಾ sm3_context));

	sm3_init(ctx);
	*ctxp = ctx;
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __maybe_unused hash_update_sm3(ಸಂರಚನಾ hash_algo *algo, ಶೂನ್ಯ *ctx,
					  ಧ್ರುವ ಶೂನ್ಯ *buf, uint size,
					  ಪೂರ್ಣಾಙ್ಕ is_last)
{
	sm3_update((ಸಂರಚನಾ sm3_context *)ctx, buf, size);
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __maybe_unused hash_finish_sm3(ಸಂರಚನಾ hash_algo *algo, ಶೂನ್ಯ *ctx,
					  ಶೂನ್ಯ *dest_buf, ಪೂರ್ಣಾಙ್ಕ size)
{
	ಯದಿ (size < algo->digest_size)
		ಪ್ರತ್ಯಾಗಮ -1;

	sm3_final((ಸಂರಚನಾ sm3_context *)ctx, dest_buf);
	ಮುಕ್ತ(ctx);
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __maybe_unused hash_init_crc16_ccitt(ಸಂರಚನಾ hash_algo *algo,
						ಶೂನ್ಯ **ctxp)
{
	uint16_t *ctx = malloc(ಪರಿಮಾಣ(uint16_t));
	*ctx = 0;
	*ctxp = ctx;
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __maybe_unused hash_update_crc16_ccitt(ಸಂರಚನಾ hash_algo *algo,
						  ಶೂನ್ಯ *ctx, ಧ್ರುವ ಶೂನ್ಯ *buf,
						  unsigned ಪೂರ್ಣಾಙ್ಕ size,
						  ಪೂರ್ಣಾಙ್ಕ is_last)
{
	*((uint16_t *)ctx) = crc16_ccitt(*((uint16_t *)ctx), buf, size);
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __maybe_unused hash_finish_crc16_ccitt(ಸಂರಚನಾ hash_algo *algo,
						  ಶೂನ್ಯ *ctx, ಶೂನ್ಯ *dest_buf,
						  ಪೂರ್ಣಾಙ್ಕ size)
{
	ಯದಿ (size < algo->digest_size)
		ಪ್ರತ್ಯಾಗಮ -1;

	*((uint16_t *)dest_buf) = *((uint16_t *)ctx);
	ಮುಕ್ತ(ctx);
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __maybe_unused hash_init_crc32(ಸಂರಚನಾ hash_algo *algo, ಶೂನ್ಯ **ctxp)
{
	uint32_t *ctx = malloc(ಪರಿಮಾಣ(uint32_t));
	*ctx = 0;
	*ctxp = ctx;
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __maybe_unused hash_update_crc32(ಸಂರಚನಾ hash_algo *algo, ಶೂನ್ಯ *ctx,
					    ಧ್ರುವ ಶೂನ್ಯ *buf, unsigned ಪೂರ್ಣಾಙ್ಕ size,
					    ಪೂರ್ಣಾಙ್ಕ is_last)
{
	*((uint32_t *)ctx) = crc32(*((uint32_t *)ctx), buf, size);
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __maybe_unused hash_finish_crc32(ಸಂರಚನಾ hash_algo *algo, ಶೂನ್ಯ *ctx,
					    ಶೂನ್ಯ *dest_buf, ಪೂರ್ಣಾಙ್ಕ size)
{
	ಯದಿ (size < algo->digest_size)
		ಪ್ರತ್ಯಾಗಮ -1;

	*((uint32_t *)dest_buf) = *((uint32_t *)ctx);
	ಮುಕ್ತ(ctx);
	ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * These are the hash algorithms we support.  If we have hardware acceleration
 * is enable we will use that, otherwise a software version of the algorithm.
 * Note that algorithm names must be in lower case.
 */
ಸ್ಥಿರ ಸಂರಚನಾ hash_algo hash_algo[] = {
#ಯದಿ CONFIG_IS_ENABLED(MD5)
	{
		.name		= "md5",
		.digest_size	= MD5_SUM_LEN,
		.chunk_size	= CHUNKSZ_MD5,
		.hash_func_ws	= md5_wd,
	},
#endif
#ಯದಿ CONFIG_IS_ENABLED(SHA1)
	{
		.name		= "sha1",
		.digest_size	= SHA1_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA1,
#ಯದಿ CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha1,
#ಅನ್ಯಥಾ
		.hash_func_ws	= sha1_csum_wd,
#endif
#ಯದಿ CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#ಅನ್ಯಥಾ
		.hash_init	= hash_init_sha1,
		.hash_update	= hash_update_sha1,
		.hash_finish	= hash_finish_sha1,
#endif
	},
#endif
#ಯದಿ CONFIG_IS_ENABLED(SHA256)
	{
		.name		= "sha256",
		.digest_size	= SHA256_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA256,
#ಯದಿ CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha256,
#ಅನ್ಯಥಾ
		.hash_func_ws	= sha256_csum_wd,
#endif
#ಯದಿ CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#ಅನ್ಯಥಾ
		.hash_init	= hash_init_sha256,
		.hash_update	= hash_update_sha256,
		.hash_finish	= hash_finish_sha256,
#endif
	},
#endif
#ಯದಿ CONFIG_IS_ENABLED(SHA384)
	{
		.name		= "sha384",
		.digest_size	= SHA384_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA384,
#ಯದಿ CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha384,
#ಅನ್ಯಥಾ
		.hash_func_ws	= sha384_csum_wd,
#endif
#ಯದಿ CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#ಅನ್ಯಥಾ
		.hash_init	= hash_init_sha384,
		.hash_update	= hash_update_sha384,
		.hash_finish	= hash_finish_sha384,
#endif
	},
#endif
#ಯದಿ CONFIG_IS_ENABLED(SHA512)
	{
		.name		= "sha512",
		.digest_size	= SHA512_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA512,
#ಯದಿ CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha512,
#ಅನ್ಯಥಾ
		.hash_func_ws	= sha512_csum_wd,
#endif
#ಯದಿ CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#ಅನ್ಯಥಾ
		.hash_init	= hash_init_sha512,
		.hash_update	= hash_update_sha512,
		.hash_finish	= hash_finish_sha512,
#endif
	},
#endif
#ಯದಿ CONFIG_IS_ENABLED(SM3)
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
#ಯದಿ CONFIG_IS_ENABLED(CRC16)
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
#ಯದಿ CONFIG_IS_ENABLED(CRC8) && IS_ENABLED(CONFIG_HASH_CRC8)
	{
		.name		= "crc8",
		.digest_size	= 1,
		.chunk_size	= CHUNKSZ_CRC32,
		.hash_func_ws	= crc8_wd_buf,
	},
#endif
#ಯದಿ CONFIG_IS_ENABLED(CRC32)
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
#ಯದಿ CONFIG_IS_ENABLED(SHA256) || IS_ENABLED(CONFIG_CMD_SHA1SUM) || \
	CONFIG_IS_ENABLED(CRC32_VERIFY) || IS_ENABLED(CONFIG_CMD_HASH) || \
	CONFIG_IS_ENABLED(SHA384) || CONFIG_IS_ENABLED(SHA512) || \
	IS_ENABLED(CONFIG_CMD_MD5SUM) || CONFIG_IS_ENABLED(SM3)
#define multi_hash()	1
#ಅನ್ಯಥಾ
#define multi_hash()	0
#endif

ಪೂರ್ಣಾಙ್ಕ hash_lookup_algo(ಧ್ರುವ ವರ್ಣ *algo_name, ಸಂರಚನಾ hash_algo **algop)
{
	ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(hash_algo); i++) {
		ಯದಿ (!strcmp(algo_name, hash_algo[i].name)) {
			*algop = &hash_algo[i];
			ಪ್ರತ್ಯಾಗಮ 0;
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	ಪ್ರತ್ಯಾಗಮ -EPROTONOSUPPORT;
}

ಪೂರ್ಣಾಙ್ಕ hash_progressive_lookup_algo(ಧ್ರುವ ವರ್ಣ *algo_name,
				 ಸಂರಚನಾ hash_algo **algop)
{
	ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(hash_algo); i++) {
		ಯದಿ (!strcmp(algo_name, hash_algo[i].name)) {
			ಯದಿ (hash_algo[i].hash_init) {
				*algop = &hash_algo[i];
				ಪ್ರತ್ಯಾಗಮ 0;
			}
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	ಪ್ರತ್ಯಾಗಮ -EPROTONOSUPPORT;
}

#ifndef USE_HOSTCC
ಪೂರ್ಣಾಙ್ಕ hash_parse_string(ಧ್ರುವ ವರ್ಣ *algo_name, ಧ್ರುವ ವರ್ಣ *str, uint8_t *result)
{
	ಸಂರಚನಾ hash_algo *algo;
	ಪೂರ್ಣಾಙ್ಕ ret;
	ಪೂರ್ಣಾಙ್ಕ i;

	ret = hash_lookup_algo(algo_name, &algo);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;

	ಪ್ರತಿ (i = 0; i < algo->digest_size; i++) {
		ವರ್ಣ chr[3];

		strlcpy(chr, &str[i * 2], 3);
		result[i] = hextoul(chr, ರಿಕ್ತ);
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ hash_block(ಧ್ರುವ ವರ್ಣ *algo_name, ಧ್ರುವ ಶೂನ್ಯ *data, unsigned ಪೂರ್ಣಾಙ್ಕ len,
	       uint8_t *output, ಪೂರ್ಣಾಙ್ಕ *output_size)
{
	ಸಂರಚನಾ hash_algo *algo;
	ಪೂರ್ಣಾಙ್ಕ ret;

	ret = hash_lookup_algo(algo_name, &algo);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;

	ಯದಿ (output_size && *output_size < algo->digest_size) {
		debug("Output buffer size %d too small (need %d bytes)",
		      *output_size, algo->digest_size);
		ಪ್ರತ್ಯಾಗಮ -ENOSPC;
	}
	ಯದಿ (output_size)
		*output_size = algo->digest_size;
	algo->hash_func_ws(data, len, output, algo->chunk_size);

	ಪ್ರತ್ಯಾಗಮ 0;
}

#ಯದಿ !defined(CONFIG_XPL_BUILD) && (defined(CONFIG_CMD_HASH) || \
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
ಸ್ಥಿರ ಶೂನ್ಯ store_result(ಸಂರಚನಾ hash_algo *algo, ಧ್ರುವ uint8_t *sum,
			 ಧ್ರುವ ವರ್ಣ *dest, ಪೂರ್ಣಾಙ್ಕ allow_env_vars)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ i;
	ಪೂರ್ಣಾಙ್ಕ env_var = 0;

	/*
	 * If environment variables are allowed, then we assume that 'dest'
	 * is an environment variable, unless it starts with *, in which
	 * case we assume it is an address. If not allowed, it is always an
	 * address. This is to support the crc32 command.
	 */
	ಯದಿ (allow_env_vars) {
		ಯದಿ (*dest == '*')
			dest++;
		ಅನ್ಯಥಾ
			env_var = 1;
	}

	ಯದಿ (env_var) {
		ವರ್ಣ str_output[HASH_MAX_DIGEST_SIZE * 2 + 1];
		ವರ್ಣ *str_ptr = str_output;

		ಪ್ರತಿ (i = 0; i < algo->digest_size; i++) {
			sprintf(str_ptr, "%02x", sum[i]);
			str_ptr += 2;
		}
		*str_ptr = '\0';
		env_set(dest, str_output);
	} ಅನ್ಯಥಾ {
		ulong addr;
		ಶೂನ್ಯ *buf;

		addr = hextoul(dest, ರಿಕ್ತ);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_verify_sum(ಸಂರಚನಾ hash_algo *algo, ವರ್ಣ *verify_str,
			    uint8_t *vsum, ಪೂರ್ಣಾಙ್ಕ allow_env_vars)
{
	ಪೂರ್ಣಾಙ್ಕ env_var = 0;

	/* See comment above in store_result() */
	ಯದಿ (allow_env_vars) {
		ಯದಿ (*verify_str == '*')
			verify_str++;
		ಅನ್ಯಥಾ
			env_var = 1;
	}

	ಯದಿ (!env_var) {
		ulong addr;
		ಶೂನ್ಯ *buf;

		addr = hextoul(verify_str, ರಿಕ್ತ);
		buf = map_sysmem(addr, algo->digest_size);
		memcpy(vsum, buf, algo->digest_size);
	} ಅನ್ಯಥಾ {
		ವರ್ಣ *vsum_str;
		ಪೂರ್ಣಾಙ್ಕ digits = algo->digest_size * 2;

		/*
		 * As with the original code from sha1sum.c, we assume that a
		 * string which matches the digest size exactly is a hex
		 * string and not an environment variable.
		 */
		ಯದಿ (strlen(verify_str) == digits)
			vsum_str = verify_str;
		ಅನ್ಯಥಾ {
			vsum_str = env_get(verify_str);
			ಯದಿ (vsum_str == ರಿಕ್ತ || strlen(vsum_str) != digits) {
				printf("Expected %d hex digits in env var\n",
				       digits);
				ಪ್ರತ್ಯಾಗಮ 1;
			}
		}

		hash_parse_string(algo->name, vsum_str, vsum);
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ hash_show(ಸಂರಚನಾ hash_algo *algo, ulong addr, ulong len, uint8_t *output)
{
	ಪೂರ್ಣಾಙ್ಕ i;

	printf("%s for %08lx ... %08lx ==> ", algo->name, addr, addr + len - 1);
	ಪ್ರತಿ (i = 0; i < algo->digest_size; i++)
		printf("%02x", output[i]);
}

ಪೂರ್ಣಾಙ್ಕ hash_command(ಧ್ರುವ ವರ್ಣ *algo_name, ಪೂರ್ಣಾಙ್ಕ flags, ಸಂರಚನಾ cmd_tbl *cmdtp,
		 ಪೂರ್ಣಾಙ್ಕ flag, ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *ಧ್ರುವ argv[])
{
	ulong addr, len;

	ಯದಿ ((argc < 2) || ((flags & HASH_FLAG_VERIFY) && (argc < 3)))
		ಪ್ರತ್ಯಾಗಮ CMD_RET_USAGE;

	addr = hextoul(*argv++, ರಿಕ್ತ);
	len = hextoul(*argv++, ರಿಕ್ತ);

	ಯದಿ (multi_hash()) {
		ಸಂರಚನಾ hash_algo *algo;
		u8 *output;
		uint8_t vsum[HASH_MAX_DIGEST_SIZE];
		ಶೂನ್ಯ *buf;

		ಯದಿ (hash_lookup_algo(algo_name, &algo)) {
			printf("Unknown hash algorithm '%s'\n", algo_name);
			ಪ್ರತ್ಯಾಗಮ CMD_RET_USAGE;
		}
		argc -= 2;

		ಯದಿ (algo->digest_size > HASH_MAX_DIGEST_SIZE) {
			puts("HASH_MAX_DIGEST_SIZE exceeded\n");
			ಪ್ರತ್ಯಾಗಮ 1;
		}

		output = memalign(ARCH_DMA_MINALIGN,
				  ಪರಿಮಾಣ(uint32_t) * HASH_MAX_DIGEST_SIZE);
		ಯದಿ (!output)
			ಪ್ರತ್ಯಾಗಮ CMD_RET_FAILURE;

		buf = map_sysmem(addr, len);
		ಯದಿ (CONFIG_IS_ENABLED(DM_HASH)) {
			ಗಣನಾ HASH_ALGO hash_algo;
			ಪೂರ್ಣಾಙ್ಕ ret;

			hash_algo = hash_algo_lookup_by_name(algo_name);
			ಯದಿ (hash_algo != HASH_ALGO_INVALID) {
				ret = hash_digest_wd_lookup(hash_algo, buf, len,
							    output,
							    algo->chunk_size);
				ಯದಿ (ret && ret != -ENODEV && ret != -EOPNOTSUPP) {
					unmap_sysmem(buf);
					ಮುಕ್ತ(output);
					ಪ್ರತ್ಯಾಗಮ CMD_RET_FAILURE;
				}
				ಯದಿ (!ret)
					ಗಚ್ಛ done;
			}
		}
		algo->hash_func_ws(buf, len, output, algo->chunk_size);
done:
		unmap_sysmem(buf);

		/* Try to avoid code bloat when verify is not needed */
#ಯದಿ defined(CONFIG_CRC32_VERIFY) || defined(CONFIG_SHA1SUM_VERIFY) || \
	defined(CONFIG_MD5SUM_VERIFY) || defined(CONFIG_HASH_VERIFY)
		ಯದಿ (flags & HASH_FLAG_VERIFY) {
#ಅನ್ಯಥಾ
		ಯದಿ (0) {
#endif
			ಯದಿ (parse_verify_sum(algo, *argv, vsum,
					flags & HASH_FLAG_ENV)) {
				printf("ERROR: %s does not contain a valid "
					"%s sum\n", *argv, algo->name);
				ಮುಕ್ತ(output);
				ಪ್ರತ್ಯಾಗಮ 1;
			}
			ಯದಿ (memcmp(output, vsum, algo->digest_size) != 0) {
				ಪೂರ್ಣಾಙ್ಕ i;

				hash_show(algo, addr, len, output);
				printf(" != ");
				ಪ್ರತಿ (i = 0; i < algo->digest_size; i++)
					printf("%02x", vsum[i]);
				puts(" ** ERROR **\n");
				ಮುಕ್ತ(output);
				ಪ್ರತ್ಯಾಗಮ 1;
			}
		} ಅನ್ಯಥಾ {
			hash_show(algo, addr, len, output);
			printf("\n");

			ಯದಿ (argc) {
				store_result(algo, output, *argv,
					flags & HASH_FLAG_ENV);
			}
		}

		ಮುಕ್ತ(output);

	/* Horrible code size hack for boards that just want crc32 */
	} ಅನ್ಯಥಾ {
		ulong crc;
		ulong *ptr;

		crc = crc32_wd(0, (ಧ್ರುವ uchar *)addr, len, CHUNKSZ_CRC32);

		printf("CRC32 for %08lx ... %08lx ==> %08lx\n",
				addr, addr + len - 1, crc);

		ಯದಿ (argc >= 3) {
			ptr = (ulong *)hextoul(argv[0], ರಿಕ್ತ);
			*ptr = crc;
		}
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif /* CONFIG_CMD_HASH || CONFIG_CMD_SHA1SUM || CONFIG_CMD_CRC32) */
#endif /* !USE_HOSTCC */
