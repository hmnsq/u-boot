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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ USE_HOSTCC
#ਅਨ੍ਤਰ੍ਭਾਵ <command.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mapmem.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <hw_sha.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/cache.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/io.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <u-boot/hash.h>
#ਨਹੀਂਤਾਂ
#ਅਨ੍ਤਰ੍ਭਾਵ "mkimage.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/compiler_attributes.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/kconfig.h>
#ਜੇ_ਅੰਤ /* !USE_HOSTCC*/

#ਅਨ੍ਤਰ੍ਭਾਵ <hash.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <u-boot/crc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <u-boot/sha1.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <u-boot/sha256.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <u-boot/sha512.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <u-boot/md5.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <u-boot/sm3.h>

ਸਥਿਰ ਪੂਰਨ __maybe_unused hash_init_sha1(ਬਣਤਰ hash_algo *algo, ਖਾਲੀ **ctxp)
{
	sha1_context *ctx = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(sha1_context));
	sha1_starts(ctx);
	*ctxp = ctx;
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ __maybe_unused hash_update_sha1(ਬਣਤਰ hash_algo *algo, ਖਾਲੀ *ctx,
					   ਅਚਲ ਖਾਲੀ *buf, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਮਾਪ,
					   ਪੂਰਨ is_ਆਖਰੀ)
{
	sha1_update((sha1_context *)ctx, buf, ਮਾਪ);
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ __maybe_unused hash_ਅੰਤ_sha1(ਬਣਤਰ hash_algo *algo, ਖਾਲੀ *ctx,
					   ਖਾਲੀ *dest_buf, ਪੂਰਨ ਮਾਪ)
{
	ਜੇ (ਮਾਪ < algo->digest_ਮਾਪ)
		ਵਾਪਸ -1;

	sha1_ਅੰਤ((sha1_context *)ctx, dest_buf);
	ਮੁਕਤ(ctx);
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ __maybe_unused hash_init_sha256(ਬਣਤਰ hash_algo *algo, ਖਾਲੀ **ctxp)
{
	sha256_context *ctx = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(sha256_context));
	sha256_starts(ctx);
	*ctxp = ctx;
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ __maybe_unused hash_update_sha256(ਬਣਤਰ hash_algo *algo, ਖਾਲੀ *ctx,
					     ਅਚਲ ਖਾਲੀ *buf, uint ਮਾਪ,
					     ਪੂਰਨ is_ਆਖਰੀ)
{
	sha256_update((sha256_context *)ctx, buf, ਮਾਪ);
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ __maybe_unused hash_ਅੰਤ_sha256(ਬਣਤਰ hash_algo *algo, ਖਾਲੀ *ctx,
					     ਖਾਲੀ *dest_buf, ਪੂਰਨ ਮਾਪ)
{
	ਜੇ (ਮਾਪ < algo->digest_ਮਾਪ)
		ਵਾਪਸ -1;

	sha256_ਅੰਤ((sha256_context *)ctx, dest_buf);
	ਮੁਕਤ(ctx);
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ __maybe_unused hash_init_sha384(ਬਣਤਰ hash_algo *algo, ਖਾਲੀ **ctxp)
{
	sha512_context *ctx = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(sha512_context));
	sha384_starts(ctx);
	*ctxp = ctx;
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ __maybe_unused hash_update_sha384(ਬਣਤਰ hash_algo *algo, ਖਾਲੀ *ctx,
					     ਅਚਲ ਖਾਲੀ *buf, uint ਮਾਪ,
					     ਪੂਰਨ is_ਆਖਰੀ)
{
	sha384_update((sha512_context *)ctx, buf, ਮਾਪ);
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ __maybe_unused hash_ਅੰਤ_sha384(ਬਣਤਰ hash_algo *algo, ਖਾਲੀ *ctx,
					     ਖਾਲੀ *dest_buf, ਪੂਰਨ ਮਾਪ)
{
	ਜੇ (ਮਾਪ < algo->digest_ਮਾਪ)
		ਵਾਪਸ -1;

	sha384_ਅੰਤ((sha512_context *)ctx, dest_buf);
	ਮੁਕਤ(ctx);
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ __maybe_unused hash_init_sha512(ਬਣਤਰ hash_algo *algo, ਖਾਲੀ **ctxp)
{
	sha512_context *ctx = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(sha512_context));
	sha512_starts(ctx);
	*ctxp = ctx;
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ __maybe_unused hash_update_sha512(ਬਣਤਰ hash_algo *algo, ਖਾਲੀ *ctx,
					     ਅਚਲ ਖਾਲੀ *buf, uint ਮਾਪ,
					     ਪੂਰਨ is_ਆਖਰੀ)
{
	sha512_update((sha512_context *)ctx, buf, ਮਾਪ);
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ __maybe_unused hash_ਅੰਤ_sha512(ਬਣਤਰ hash_algo *algo, ਖਾਲੀ *ctx,
					     ਖਾਲੀ *dest_buf, ਪੂਰਨ ਮਾਪ)
{
	ਜੇ (ਮਾਪ < algo->digest_ਮਾਪ)
		ਵਾਪਸ -1;

	sha512_ਅੰਤ((sha512_context *)ctx, dest_buf);
	ਮੁਕਤ(ctx);
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ __maybe_unused hash_init_sm3(ਬਣਤਰ hash_algo *algo, ਖਾਲੀ **ctxp)
{
	ਬਣਤਰ sm3_context *ctx = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ sm3_context));

	sm3_init(ctx);
	*ctxp = ctx;
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ __maybe_unused hash_update_sm3(ਬਣਤਰ hash_algo *algo, ਖਾਲੀ *ctx,
					  ਅਚਲ ਖਾਲੀ *buf, uint ਮਾਪ,
					  ਪੂਰਨ is_ਆਖਰੀ)
{
	sm3_update((ਬਣਤਰ sm3_context *)ctx, buf, ਮਾਪ);
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ __maybe_unused hash_ਅੰਤ_sm3(ਬਣਤਰ hash_algo *algo, ਖਾਲੀ *ctx,
					  ਖਾਲੀ *dest_buf, ਪੂਰਨ ਮਾਪ)
{
	ਜੇ (ਮਾਪ < algo->digest_ਮਾਪ)
		ਵਾਪਸ -1;

	sm3_final((ਬਣਤਰ sm3_context *)ctx, dest_buf);
	ਮੁਕਤ(ctx);
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ __maybe_unused hash_init_crc16_ccitt(ਬਣਤਰ hash_algo *algo,
						ਖਾਲੀ **ctxp)
{
	uint16_t *ctx = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(uint16_t));
	*ctx = 0;
	*ctxp = ctx;
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ __maybe_unused hash_update_crc16_ccitt(ਬਣਤਰ hash_algo *algo,
						  ਖਾਲੀ *ctx, ਅਚਲ ਖਾਲੀ *buf,
						  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਮਾਪ,
						  ਪੂਰਨ is_ਆਖਰੀ)
{
	*((uint16_t *)ctx) = crc16_ccitt(*((uint16_t *)ctx), buf, ਮਾਪ);
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ __maybe_unused hash_ਅੰਤ_crc16_ccitt(ਬਣਤਰ hash_algo *algo,
						  ਖਾਲੀ *ctx, ਖਾਲੀ *dest_buf,
						  ਪੂਰਨ ਮਾਪ)
{
	ਜੇ (ਮਾਪ < algo->digest_ਮਾਪ)
		ਵਾਪਸ -1;

	*((uint16_t *)dest_buf) = *((uint16_t *)ctx);
	ਮੁਕਤ(ctx);
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ __maybe_unused hash_init_ਚੱਕਰੀ_ਜਾਂਚ_32(ਬਣਤਰ hash_algo *algo, ਖਾਲੀ **ctxp)
{
	uint32_t *ctx = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(uint32_t));
	*ctx = 0;
	*ctxp = ctx;
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ __maybe_unused hash_update_ਚੱਕਰੀ_ਜਾਂਚ_32(ਬਣਤਰ hash_algo *algo, ਖਾਲੀ *ctx,
					    ਅਚਲ ਖਾਲੀ *buf, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਮਾਪ,
					    ਪੂਰਨ is_ਆਖਰੀ)
{
	*((uint32_t *)ctx) = ਚੱਕਰੀ_ਜਾਂਚ_32(*((uint32_t *)ctx), buf, ਮਾਪ);
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ __maybe_unused hash_ਅੰਤ_ਚੱਕਰੀ_ਜਾਂਚ_32(ਬਣਤਰ hash_algo *algo, ਖਾਲੀ *ctx,
					    ਖਾਲੀ *dest_buf, ਪੂਰਨ ਮਾਪ)
{
	ਜੇ (ਮਾਪ < algo->digest_ਮਾਪ)
		ਵਾਪਸ -1;

	*((uint32_t *)dest_buf) = *((uint32_t *)ctx);
	ਮੁਕਤ(ctx);
	ਵਾਪਸ 0;
}

/*
 * These are the hash algorithms we support.  If we have hardware acceleration
 * is enable we will use that, otherwise a software version of the algorithm.
 * Note that algorithm names must be in lower case.
 */
ਸਥਿਰ ਬਣਤਰ hash_algo hash_algo[] = {
#ਜੇ CONFIG_IS_ENABLED(MD5)
	{
		.name		= "md5",
		.digest_ਮਾਪ	= MD5_ਜੋੜ_LEN,
		.chunk_ਮਾਪ	= CHUNKSZ_MD5,
		.hash_func_ws	= md5_wd,
	},
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(SHA1)
	{
		.name		= "sha1",
		.digest_ਮਾਪ	= SHA1_ਜੋੜ_LEN,
		.chunk_ਮਾਪ	= CHUNKSZ_SHA1,
#ਜੇ CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha1,
#ਨਹੀਂਤਾਂ
		.hash_func_ws	= sha1_csum_wd,
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_ਅੰਤ	= hw_sha_ਅੰਤ,
#ਨਹੀਂਤਾਂ
		.hash_init	= hash_init_sha1,
		.hash_update	= hash_update_sha1,
		.hash_ਅੰਤ	= hash_ਅੰਤ_sha1,
#ਜੇ_ਅੰਤ
	},
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(SHA256)
	{
		.name		= "sha256",
		.digest_ਮਾਪ	= SHA256_ਜੋੜ_LEN,
		.chunk_ਮਾਪ	= CHUNKSZ_SHA256,
#ਜੇ CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha256,
#ਨਹੀਂਤਾਂ
		.hash_func_ws	= sha256_csum_wd,
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_ਅੰਤ	= hw_sha_ਅੰਤ,
#ਨਹੀਂਤਾਂ
		.hash_init	= hash_init_sha256,
		.hash_update	= hash_update_sha256,
		.hash_ਅੰਤ	= hash_ਅੰਤ_sha256,
#ਜੇ_ਅੰਤ
	},
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(SHA384)
	{
		.name		= "sha384",
		.digest_ਮਾਪ	= SHA384_ਜੋੜ_LEN,
		.chunk_ਮਾਪ	= CHUNKSZ_SHA384,
#ਜੇ CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha384,
#ਨਹੀਂਤਾਂ
		.hash_func_ws	= sha384_csum_wd,
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_ਅੰਤ	= hw_sha_ਅੰਤ,
#ਨਹੀਂਤਾਂ
		.hash_init	= hash_init_sha384,
		.hash_update	= hash_update_sha384,
		.hash_ਅੰਤ	= hash_ਅੰਤ_sha384,
#ਜੇ_ਅੰਤ
	},
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(SHA512)
	{
		.name		= "sha512",
		.digest_ਮਾਪ	= SHA512_ਜੋੜ_LEN,
		.chunk_ਮਾਪ	= CHUNKSZ_SHA512,
#ਜੇ CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha512,
#ਨਹੀਂਤਾਂ
		.hash_func_ws	= sha512_csum_wd,
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_ਅੰਤ	= hw_sha_ਅੰਤ,
#ਨਹੀਂਤਾਂ
		.hash_init	= hash_init_sha512,
		.hash_update	= hash_update_sha512,
		.hash_ਅੰਤ	= hash_ਅੰਤ_sha512,
#ਜੇ_ਅੰਤ
	},
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(SM3)
	{
		.name		= "sm3_256",
		.digest_ਮਾਪ	= SM3_DIGEST_ਮਾਪ,
		.chunk_ਮਾਪ	= SM3_ਖੰਡ_ਮਾਪ,
		.hash_func_ws	= sm3_csum_wd,
		.hash_init	= hash_init_sm3,
		.hash_update	= hash_update_sm3,
		.hash_ਅੰਤ	= hash_ਅੰਤ_sm3,
	},
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(CRC16)
	{
		.name		= "crc16-ccitt",
		.digest_ਮਾਪ	= 2,
		.chunk_ਮਾਪ	= CHUNKSZ,
		.hash_func_ws	= crc16_ccitt_wd_buf,
		.hash_init	= hash_init_crc16_ccitt,
		.hash_update	= hash_update_crc16_ccitt,
		.hash_ਅੰਤ	= hash_ਅੰਤ_crc16_ccitt,
	},
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(CRC8) && IS_ENABLED(CONFIG_HASH_CRC8)
	{
		.name		= "crc8",
		.digest_ਮਾਪ	= 1,
		.chunk_ਮਾਪ	= CHUNKSZ_ਚੱਕਰੀ_ਜਾਂਚ_32,
		.hash_func_ws	= crc8_wd_buf,
	},
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(ਚੱਕਰੀ_ਜਾਂਚ_32_2)
	{
		.name		= "crc32",
		.digest_ਮਾਪ	= 4,
		.chunk_ਮਾਪ	= CHUNKSZ_ਚੱਕਰੀ_ਜਾਂਚ_32,
		.hash_func_ws	= ਚੱਕਰੀ_ਜਾਂਚ_32_wd_buf,
		.hash_init	= hash_init_ਚੱਕਰੀ_ਜਾਂਚ_32,
		.hash_update	= hash_update_ਚੱਕਰੀ_ਜਾਂਚ_32,
		.hash_ਅੰਤ	= hash_ਅੰਤ_ਚੱਕਰੀ_ਜਾਂਚ_32,
	},
#ਜੇ_ਅੰਤ
};

/* Try to minimize code size for boards that don't want much hashing */
#ਜੇ CONFIG_IS_ENABLED(SHA256) || IS_ENABLED(CONFIG_CMD_SHA1_ਜੋੜ) || \
	CONFIG_IS_ENABLED(ਚੱਕਰੀ_ਜਾਂਚ_32_VERIFY) || IS_ENABLED(CONFIG_CMD_HASH) || \
	CONFIG_IS_ENABLED(SHA384) || CONFIG_IS_ENABLED(SHA512) || \
	IS_ENABLED(CONFIG_CMD_MD5_ਜੋੜ) || CONFIG_IS_ENABLED(SM3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ multi_hash()	1
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ multi_hash()	0
#ਜੇ_ਅੰਤ

ਪੂਰਨ hash_lookup_algo(ਅਚਲ ਅੱਖਰ *algo_name, ਬਣਤਰ hash_algo **algop)
{
	ਪੂਰਨ i;

	ਲਈ (i = 0; i < ARRAY_ਮਾਪ(hash_algo); i++) {
		ਜੇ (!strcmp(algo_name, hash_algo[i].name)) {
			*algop = &hash_algo[i];
			ਵਾਪਸ 0;
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	ਵਾਪਸ -EPROTONOSUPPORT;
}

ਪੂਰਨ hash_progressive_lookup_algo(ਅਚਲ ਅੱਖਰ *algo_name,
				 ਬਣਤਰ hash_algo **algop)
{
	ਪੂਰਨ i;

	ਲਈ (i = 0; i < ARRAY_ਮਾਪ(hash_algo); i++) {
		ਜੇ (!strcmp(algo_name, hash_algo[i].name)) {
			ਜੇ (hash_algo[i].hash_init) {
				*algop = &hash_algo[i];
				ਵਾਪਸ 0;
			}
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	ਵਾਪਸ -EPROTONOSUPPORT;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ USE_HOSTCC
ਪੂਰਨ hash_parse_string(ਅਚਲ ਅੱਖਰ *algo_name, ਅਚਲ ਅੱਖਰ *str, uint8_t *ਨਤੀਜਾ)
{
	ਬਣਤਰ hash_algo *algo;
	ਪੂਰਨ ret;
	ਪੂਰਨ i;

	ret = hash_lookup_algo(algo_name, &algo);
	ਜੇ (ret)
		ਵਾਪਸ ret;

	ਲਈ (i = 0; i < algo->digest_ਮਾਪ; i++) {
		ਅੱਖਰ chr[3];

		strlcpy(chr, &str[i * 2], 3);
		ਨਤੀਜਾ[i] = hextoul(chr, ਨਿਰਾ);
	}

	ਵਾਪਸ 0;
}

ਪੂਰਨ hash_ਖੰਡ(ਅਚਲ ਅੱਖਰ *algo_name, ਅਚਲ ਖਾਲੀ *ਅੰਕੜੇ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ len,
	       uint8_t *output, ਪੂਰਨ *output_ਮਾਪ)
{
	ਬਣਤਰ hash_algo *algo;
	ਪੂਰਨ ret;

	ret = hash_lookup_algo(algo_name, &algo);
	ਜੇ (ret)
		ਵਾਪਸ ret;

	ਜੇ (output_ਮਾਪ && *output_ਮਾਪ < algo->digest_ਮਾਪ) {
		debug("Output buffer size %d too small (need %d bytes)",
		      *output_ਮਾਪ, algo->digest_ਮਾਪ);
		ਵਾਪਸ -ENOSPC;
	}
	ਜੇ (output_ਮਾਪ)
		*output_ਮਾਪ = algo->digest_ਮਾਪ;
	algo->hash_func_ws(ਅੰਕੜੇ, len, output, algo->chunk_ਮਾਪ);

	ਵਾਪਸ 0;
}

#ਜੇ !defined(CONFIG_XPL_BUILD) && (defined(CONFIG_CMD_HASH) || \
	defined(CONFIG_CMD_SHA1_ਜੋੜ) || defined(CONFIG_CMD_ਚੱਕਰੀ_ਜਾਂਚ_32)) || \
	defined(CONFIG_CMD_MD5_ਜੋੜ)
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
ਸਥਿਰ ਖਾਲੀ store_ਨਤੀਜਾ(ਬਣਤਰ hash_algo *algo, ਅਚਲ uint8_t *ਜੋੜ,
			 ਅਚਲ ਅੱਖਰ *dest, ਪੂਰਨ allow_env_vars)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
	ਪੂਰਨ env_var = 0;

	/*
	 * If environment variables are allowed, then we assume that 'dest'
	 * is an environment variable, unless it starts with *, in which
	 * case we assume it is an address. If not allowed, it is always an
	 * address. This is to support the crc32 command.
	 */
	ਜੇ (allow_env_vars) {
		ਜੇ (*dest == '*')
			dest++;
		ਨਹੀਂਤਾਂ
			env_var = 1;
	}

	ਜੇ (env_var) {
		ਅੱਖਰ str_output[HASH_MAX_DIGEST_ਮਾਪ * 2 + 1];
		ਅੱਖਰ *str_ptr = str_output;

		ਲਈ (i = 0; i < algo->digest_ਮਾਪ; i++) {
			sprintf(str_ptr, "%02x", ਜੋੜ[i]);
			str_ptr += 2;
		}
		*str_ptr = '\0';
		env_ਸੈੱਟ(dest, str_output);
	} ਨਹੀਂਤਾਂ {
		ulong addr;
		ਖਾਲੀ *buf;

		addr = hextoul(dest, ਨਿਰਾ);
		buf = map_sysmem(addr, algo->digest_ਮਾਪ);
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(buf, ਜੋੜ, algo->digest_ਮਾਪ);
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
ਸਥਿਰ ਪੂਰਨ parse_verify_ਜੋੜ(ਬਣਤਰ hash_algo *algo, ਅੱਖਰ *verify_str,
			    uint8_t *vsum, ਪੂਰਨ allow_env_vars)
{
	ਪੂਰਨ env_var = 0;

	/* See comment above in store_result() */
	ਜੇ (allow_env_vars) {
		ਜੇ (*verify_str == '*')
			verify_str++;
		ਨਹੀਂਤਾਂ
			env_var = 1;
	}

	ਜੇ (!env_var) {
		ulong addr;
		ਖਾਲੀ *buf;

		addr = hextoul(verify_str, ਨਿਰਾ);
		buf = map_sysmem(addr, algo->digest_ਮਾਪ);
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(vsum, buf, algo->digest_ਮਾਪ);
	} ਨਹੀਂਤਾਂ {
		ਅੱਖਰ *vsum_str;
		ਪੂਰਨ digits = algo->digest_ਮਾਪ * 2;

		/*
		 * As with the original code from sha1sum.c, we assume that a
		 * string which matches the digest size exactly is a hex
		 * string and not an environment variable.
		 */
		ਜੇ (ਲੜੀ_ਲੰਬਾਈ(verify_str) == digits)
			vsum_str = verify_str;
		ਨਹੀਂਤਾਂ {
			vsum_str = env_get(verify_str);
			ਜੇ (vsum_str == ਨਿਰਾ || ਲੜੀ_ਲੰਬਾਈ(vsum_str) != digits) {
				ਲਿਖੋ("Expected %d hex digits in env var\n",
				       digits);
				ਵਾਪਸ 1;
			}
		}

		hash_parse_string(algo->name, vsum_str, vsum);
	}
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ hash_show(ਬਣਤਰ hash_algo *algo, ulong addr, ulong len, uint8_t *output)
{
	ਪੂਰਨ i;

	ਲਿਖੋ("%s for %08lx ... %08lx ==> ", algo->name, addr, addr + len - 1);
	ਲਈ (i = 0; i < algo->digest_ਮਾਪ; i++)
		ਲਿਖੋ("%02x", output[i]);
}

ਪੂਰਨ hash_command(ਅਚਲ ਅੱਖਰ *algo_name, ਪੂਰਨ flags, ਬਣਤਰ cmd_tbl *cmdtp,
		 ਪੂਰਨ flag, ਪੂਰਨ argc, ਅੱਖਰ *ਅਚਲ argv[])
{
	ulong addr, len;

	ਜੇ ((argc < 2) || ((flags & HASH_FLAG_VERIFY) && (argc < 3)))
		ਵਾਪਸ CMD_RET_USAGE;

	addr = hextoul(*argv++, ਨਿਰਾ);
	len = hextoul(*argv++, ਨਿਰਾ);

	ਜੇ (multi_hash()) {
		ਬਣਤਰ hash_algo *algo;
		u8 *output;
		uint8_t vsum[HASH_MAX_DIGEST_ਮਾਪ];
		ਖਾਲੀ *buf;

		ਜੇ (hash_lookup_algo(algo_name, &algo)) {
			ਲਿਖੋ("Unknown hash algorithm '%s'\n", algo_name);
			ਵਾਪਸ CMD_RET_USAGE;
		}
		argc -= 2;

		ਜੇ (algo->digest_ਮਾਪ > HASH_MAX_DIGEST_ਮਾਪ) {
			puts("HASH_MAX_DIGEST_SIZE exceeded\n");
			ਵਾਪਸ 1;
		}

		output = memalign(ARCH_DMA_MINALIGN,
				  ਆਕਾਰ(uint32_t) * HASH_MAX_DIGEST_ਮਾਪ);
		ਜੇ (!output)
			ਵਾਪਸ CMD_RET_FAILURE;

		buf = map_sysmem(addr, len);
		ਜੇ (CONFIG_IS_ENABLED(DM_HASH)) {
			ਗਿਣਤੀ HASH_ALGO hash_algo;
			ਪੂਰਨ ret;

			hash_algo = hash_algo_lookup_by_name(algo_name);
			ਜੇ (hash_algo != HASH_ALGO_INVALID) {
				ret = hash_digest_wd_lookup(hash_algo, buf, len,
							    output,
							    algo->chunk_ਮਾਪ);
				ਜੇ (ret && ret != -ENODEV && ret != -EOPNOTSUPP) {
					unmap_sysmem(buf);
					ਮੁਕਤ(output);
					ਵਾਪਸ CMD_RET_FAILURE;
				}
				ਜੇ (!ret)
					ਜਾਓ ਪੂਰਾ;
			}
		}
		algo->hash_func_ws(buf, len, output, algo->chunk_ਮਾਪ);
ਪੂਰਾ:
		unmap_sysmem(buf);

		/* Try to avoid code bloat when verify is not needed */
#ਜੇ defined(CONFIG_ਚੱਕਰੀ_ਜਾਂਚ_32_VERIFY) || defined(CONFIG_SHA1_ਜੋੜ_VERIFY) || \
	defined(CONFIG_MD5_ਜੋੜ_VERIFY) || defined(CONFIG_HASH_VERIFY)
		ਜੇ (flags & HASH_FLAG_VERIFY) {
#ਨਹੀਂਤਾਂ
		ਜੇ (0) {
#ਜੇ_ਅੰਤ
			ਜੇ (parse_verify_ਜੋੜ(algo, *argv, vsum,
					flags & HASH_FLAG_ENV)) {
				ਲਿਖੋ("ERROR: %s does not contain a valid "
					"%s sum\n", *argv, algo->name);
				ਮੁਕਤ(output);
				ਵਾਪਸ 1;
			}
			ਜੇ (memcmp(output, vsum, algo->digest_ਮਾਪ) != 0) {
				ਪੂਰਨ i;

				hash_show(algo, addr, len, output);
				ਲਿਖੋ(" != ");
				ਲਈ (i = 0; i < algo->digest_ਮਾਪ; i++)
					ਲਿਖੋ("%02x", vsum[i]);
				puts(" ** ERROR **\n");
				ਮੁਕਤ(output);
				ਵਾਪਸ 1;
			}
		} ਨਹੀਂਤਾਂ {
			hash_show(algo, addr, len, output);
			ਲਿਖੋ("\n");

			ਜੇ (argc) {
				store_ਨਤੀਜਾ(algo, output, *argv,
					flags & HASH_FLAG_ENV);
			}
		}

		ਮੁਕਤ(output);

	/* Horrible code size hack for boards that just want crc32 */
	} ਨਹੀਂਤਾਂ {
		ulong ਚੱਕਰੀ_ਜਾਂਚ;
		ulong *ptr;

		ਚੱਕਰੀ_ਜਾਂਚ = ਚੱਕਰੀ_ਜਾਂਚ_32_wd(0, (ਅਚਲ uchar *)addr, len, CHUNKSZ_ਚੱਕਰੀ_ਜਾਂਚ_32);

		ਲਿਖੋ("CRC32 for %08lx ... %08lx ==> %08lx\n",
				addr, addr + len - 1, ਚੱਕਰੀ_ਜਾਂਚ);

		ਜੇ (argc >= 3) {
			ptr = (ulong *)hextoul(argv[0], ਨਿਰਾ);
			*ptr = ਚੱਕਰੀ_ਜਾਂਚ;
		}
	}

	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ /* CONFIG_CMD_HASH || CONFIG_CMD_SHA1SUM || CONFIG_CMD_CRC32) */
#ਜੇ_ਅੰਤ /* !USE_HOSTCC */
