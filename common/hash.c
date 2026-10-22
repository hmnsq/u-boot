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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ USE_HOSTCC
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
#ಇಲ್ಲದಿದ್ದರೆ
#ಅನ್ತರ್ಭಾವ "mkimage.h"
#ಅನ್ತರ್ಭಾವ <linux/compiler_attributes.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <linux/kconfig.h>
#ಆದರೆ_ಅಂತ್ಯ /* !USE_HOSTCC*/

#ಅನ್ತರ್ಭಾವ <hash.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <u-boot/crc.h>
#ಅನ್ತರ್ಭಾವ <u-boot/sha1.h>
#ಅನ್ತರ್ಭಾವ <u-boot/sha256.h>
#ಅನ್ತರ್ಭಾವ <u-boot/sha512.h>
#ಅನ್ತರ್ಭಾವ <u-boot/md5.h>
#ಅನ್ತರ್ಭಾವ <u-boot/sm3.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __maybe_unused hash_init_sha1(ರಚನೆ hash_algo *algo, ಶೂನ್ಯ **ctxp)
{
	sha1_context *ctx = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(sha1_context));
	sha1_starts(ctx);
	*ctxp = ctx;
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __maybe_unused hash_update_sha1(ರಚನೆ hash_algo *algo, ಶೂನ್ಯ *ctx,
					   ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *buf, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2,
					   ಪೂರ್ಣಾಂಕ is_ಕೊನೆಯ)
{
	sha1_update((sha1_context *)ctx, buf, ಗಾತ್ರ_2);
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __maybe_unused hash_ಮುಕ್ತಾಯ_sha1(ರಚನೆ hash_algo *algo, ಶೂನ್ಯ *ctx,
					   ಶೂನ್ಯ *dest_buf, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	ಆದರೆ (ಗಾತ್ರ_2 < algo->digest_ಗಾತ್ರ)
		ಹಿಂತಿರುಗಿ -1;

	sha1_ಮುಕ್ತಾಯ((sha1_context *)ctx, dest_buf);
	ಬಿಡುಗಡೆ(ctx);
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __maybe_unused hash_init_sha256(ರಚನೆ hash_algo *algo, ಶೂನ್ಯ **ctxp)
{
	sha256_context *ctx = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(sha256_context));
	sha256_starts(ctx);
	*ctxp = ctx;
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __maybe_unused hash_update_sha256(ರಚನೆ hash_algo *algo, ಶೂನ್ಯ *ctx,
					     ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *buf, uint ಗಾತ್ರ_2,
					     ಪೂರ್ಣಾಂಕ is_ಕೊನೆಯ)
{
	sha256_update((sha256_context *)ctx, buf, ಗಾತ್ರ_2);
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __maybe_unused hash_ಮುಕ್ತಾಯ_sha256(ರಚನೆ hash_algo *algo, ಶೂನ್ಯ *ctx,
					     ಶೂನ್ಯ *dest_buf, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	ಆದರೆ (ಗಾತ್ರ_2 < algo->digest_ಗಾತ್ರ)
		ಹಿಂತಿರುಗಿ -1;

	sha256_ಮುಕ್ತಾಯ((sha256_context *)ctx, dest_buf);
	ಬಿಡುಗಡೆ(ctx);
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __maybe_unused hash_init_sha384(ರಚನೆ hash_algo *algo, ಶೂನ್ಯ **ctxp)
{
	sha512_context *ctx = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(sha512_context));
	sha384_starts(ctx);
	*ctxp = ctx;
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __maybe_unused hash_update_sha384(ರಚನೆ hash_algo *algo, ಶೂನ್ಯ *ctx,
					     ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *buf, uint ಗಾತ್ರ_2,
					     ಪೂರ್ಣಾಂಕ is_ಕೊನೆಯ)
{
	sha384_update((sha512_context *)ctx, buf, ಗಾತ್ರ_2);
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __maybe_unused hash_ಮುಕ್ತಾಯ_sha384(ರಚನೆ hash_algo *algo, ಶೂನ್ಯ *ctx,
					     ಶೂನ್ಯ *dest_buf, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	ಆದರೆ (ಗಾತ್ರ_2 < algo->digest_ಗಾತ್ರ)
		ಹಿಂತಿರುಗಿ -1;

	sha384_ಮುಕ್ತಾಯ((sha512_context *)ctx, dest_buf);
	ಬಿಡುಗಡೆ(ctx);
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __maybe_unused hash_init_sha512(ರಚನೆ hash_algo *algo, ಶೂನ್ಯ **ctxp)
{
	sha512_context *ctx = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(sha512_context));
	sha512_starts(ctx);
	*ctxp = ctx;
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __maybe_unused hash_update_sha512(ರಚನೆ hash_algo *algo, ಶೂನ್ಯ *ctx,
					     ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *buf, uint ಗಾತ್ರ_2,
					     ಪೂರ್ಣಾಂಕ is_ಕೊನೆಯ)
{
	sha512_update((sha512_context *)ctx, buf, ಗಾತ್ರ_2);
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __maybe_unused hash_ಮುಕ್ತಾಯ_sha512(ರಚನೆ hash_algo *algo, ಶೂನ್ಯ *ctx,
					     ಶೂನ್ಯ *dest_buf, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	ಆದರೆ (ಗಾತ್ರ_2 < algo->digest_ಗಾತ್ರ)
		ಹಿಂತಿರುಗಿ -1;

	sha512_ಮುಕ್ತಾಯ((sha512_context *)ctx, dest_buf);
	ಬಿಡುಗಡೆ(ctx);
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __maybe_unused hash_init_sm3(ರಚನೆ hash_algo *algo, ಶೂನ್ಯ **ctxp)
{
	ರಚನೆ sm3_context *ctx = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ sm3_context));

	sm3_init(ctx);
	*ctxp = ctx;
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __maybe_unused hash_update_sm3(ರಚನೆ hash_algo *algo, ಶೂನ್ಯ *ctx,
					  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *buf, uint ಗಾತ್ರ_2,
					  ಪೂರ್ಣಾಂಕ is_ಕೊನೆಯ)
{
	sm3_update((ರಚನೆ sm3_context *)ctx, buf, ಗಾತ್ರ_2);
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __maybe_unused hash_ಮುಕ್ತಾಯ_sm3(ರಚನೆ hash_algo *algo, ಶೂನ್ಯ *ctx,
					  ಶೂನ್ಯ *dest_buf, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	ಆದರೆ (ಗಾತ್ರ_2 < algo->digest_ಗಾತ್ರ)
		ಹಿಂತಿರುಗಿ -1;

	sm3_final((ರಚನೆ sm3_context *)ctx, dest_buf);
	ಬಿಡುಗಡೆ(ctx);
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __maybe_unused hash_init_crc16_ccitt(ರಚನೆ hash_algo *algo,
						ಶೂನ್ಯ **ctxp)
{
	uint16_t *ctx = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(uint16_t));
	*ctx = 0;
	*ctxp = ctx;
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __maybe_unused hash_update_crc16_ccitt(ರಚನೆ hash_algo *algo,
						  ಶೂನ್ಯ *ctx, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *buf,
						  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2,
						  ಪೂರ್ಣಾಂಕ is_ಕೊನೆಯ)
{
	*((uint16_t *)ctx) = crc16_ccitt(*((uint16_t *)ctx), buf, ಗಾತ್ರ_2);
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __maybe_unused hash_ಮುಕ್ತಾಯ_crc16_ccitt(ರಚನೆ hash_algo *algo,
						  ಶೂನ್ಯ *ctx, ಶೂನ್ಯ *dest_buf,
						  ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	ಆದರೆ (ಗಾತ್ರ_2 < algo->digest_ಗಾತ್ರ)
		ಹಿಂತಿರುಗಿ -1;

	*((uint16_t *)dest_buf) = *((uint16_t *)ctx);
	ಬಿಡುಗಡೆ(ctx);
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __maybe_unused hash_init_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32(ರಚನೆ hash_algo *algo, ಶೂನ್ಯ **ctxp)
{
	uint32_t *ctx = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(uint32_t));
	*ctx = 0;
	*ctxp = ctx;
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __maybe_unused hash_update_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32(ರಚನೆ hash_algo *algo, ಶೂನ್ಯ *ctx,
					    ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *buf, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2,
					    ಪೂರ್ಣಾಂಕ is_ಕೊನೆಯ)
{
	*((uint32_t *)ctx) = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32(*((uint32_t *)ctx), buf, ಗಾತ್ರ_2);
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __maybe_unused hash_ಮುಕ್ತಾಯ_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32(ರಚನೆ hash_algo *algo, ಶೂನ್ಯ *ctx,
					    ಶೂನ್ಯ *dest_buf, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	ಆದರೆ (ಗಾತ್ರ_2 < algo->digest_ಗಾತ್ರ)
		ಹಿಂತಿರುಗಿ -1;

	*((uint32_t *)dest_buf) = *((uint32_t *)ctx);
	ಬಿಡುಗಡೆ(ctx);
	ಹಿಂತಿರುಗಿ 0;
}

/*
 * These are the hash algorithms we support.  If we have hardware acceleration
 * is enable we will use that, otherwise a software version of the algorithm.
 * Note that algorithm names must be in lower case.
 */
ಸ್ಥಿರ ರಚನೆ hash_algo hash_algo[] = {
#ಆದರೆ CONFIG_IS_ENABLED(MD5)
	{
		.name		= "md5",
		.digest_ಗಾತ್ರ	= MD5_ಮೊತ್ತ_LEN,
		.chunk_ಗಾತ್ರ	= CHUNKSZ_MD5,
		.hash_func_ws	= md5_wd,
	},
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(SHA1)
	{
		.name		= "sha1",
		.digest_ಗಾತ್ರ	= SHA1_ಮೊತ್ತ_LEN,
		.chunk_ಗಾತ್ರ	= CHUNKSZ_SHA1,
#ಆದರೆ CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha1,
#ಇಲ್ಲದಿದ್ದರೆ
		.hash_func_ws	= sha1_csum_wd,
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_ಮುಕ್ತಾಯ	= hw_sha_ಮುಕ್ತಾಯ,
#ಇಲ್ಲದಿದ್ದರೆ
		.hash_init	= hash_init_sha1,
		.hash_update	= hash_update_sha1,
		.hash_ಮುಕ್ತಾಯ	= hash_ಮುಕ್ತಾಯ_sha1,
#ಆದರೆ_ಅಂತ್ಯ
	},
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(SHA256)
	{
		.name		= "sha256",
		.digest_ಗಾತ್ರ	= SHA256_ಮೊತ್ತ_LEN,
		.chunk_ಗಾತ್ರ	= CHUNKSZ_SHA256,
#ಆದರೆ CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha256,
#ಇಲ್ಲದಿದ್ದರೆ
		.hash_func_ws	= sha256_csum_wd,
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_ಮುಕ್ತಾಯ	= hw_sha_ಮುಕ್ತಾಯ,
#ಇಲ್ಲದಿದ್ದರೆ
		.hash_init	= hash_init_sha256,
		.hash_update	= hash_update_sha256,
		.hash_ಮುಕ್ತಾಯ	= hash_ಮುಕ್ತಾಯ_sha256,
#ಆದರೆ_ಅಂತ್ಯ
	},
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(SHA384)
	{
		.name		= "sha384",
		.digest_ಗಾತ್ರ	= SHA384_ಮೊತ್ತ_LEN,
		.chunk_ಗಾತ್ರ	= CHUNKSZ_SHA384,
#ಆದರೆ CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha384,
#ಇಲ್ಲದಿದ್ದರೆ
		.hash_func_ws	= sha384_csum_wd,
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_ಮುಕ್ತಾಯ	= hw_sha_ಮುಕ್ತಾಯ,
#ಇಲ್ಲದಿದ್ದರೆ
		.hash_init	= hash_init_sha384,
		.hash_update	= hash_update_sha384,
		.hash_ಮುಕ್ತಾಯ	= hash_ಮುಕ್ತಾಯ_sha384,
#ಆದರೆ_ಅಂತ್ಯ
	},
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(SHA512)
	{
		.name		= "sha512",
		.digest_ಗಾತ್ರ	= SHA512_ಮೊತ್ತ_LEN,
		.chunk_ಗಾತ್ರ	= CHUNKSZ_SHA512,
#ಆದರೆ CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha512,
#ಇಲ್ಲದಿದ್ದರೆ
		.hash_func_ws	= sha512_csum_wd,
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_ಮುಕ್ತಾಯ	= hw_sha_ಮುಕ್ತಾಯ,
#ಇಲ್ಲದಿದ್ದರೆ
		.hash_init	= hash_init_sha512,
		.hash_update	= hash_update_sha512,
		.hash_ಮುಕ್ತಾಯ	= hash_ಮುಕ್ತಾಯ_sha512,
#ಆದರೆ_ಅಂತ್ಯ
	},
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(SM3)
	{
		.name		= "sm3_256",
		.digest_ಗಾತ್ರ	= SM3_DIGEST_ಗಾತ್ರ,
		.chunk_ಗಾತ್ರ	= SM3_ಖಂಡ_ಗಾತ್ರ,
		.hash_func_ws	= sm3_csum_wd,
		.hash_init	= hash_init_sm3,
		.hash_update	= hash_update_sm3,
		.hash_ಮುಕ್ತಾಯ	= hash_ಮುಕ್ತಾಯ_sm3,
	},
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(CRC16)
	{
		.name		= "crc16-ccitt",
		.digest_ಗಾತ್ರ	= 2,
		.chunk_ಗಾತ್ರ	= CHUNKSZ,
		.hash_func_ws	= crc16_ccitt_wd_buf,
		.hash_init	= hash_init_crc16_ccitt,
		.hash_update	= hash_update_crc16_ccitt,
		.hash_ಮುಕ್ತಾಯ	= hash_ಮುಕ್ತಾಯ_crc16_ccitt,
	},
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(CRC8) && IS_ENABLED(CONFIG_HASH_CRC8)
	{
		.name		= "crc8",
		.digest_ಗಾತ್ರ	= 1,
		.chunk_ಗಾತ್ರ	= CHUNKSZ_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32,
		.hash_func_ws	= crc8_wd_buf,
	},
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2)
	{
		.name		= "crc32",
		.digest_ಗಾತ್ರ	= 4,
		.chunk_ಗಾತ್ರ	= CHUNKSZ_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32,
		.hash_func_ws	= ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_wd_buf,
		.hash_init	= hash_init_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32,
		.hash_update	= hash_update_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32,
		.hash_ಮುಕ್ತಾಯ	= hash_ಮುಕ್ತಾಯ_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32,
	},
#ಆದರೆ_ಅಂತ್ಯ
};

/* Try to minimize code size for boards that don't want much hashing */
#ಆದರೆ CONFIG_IS_ENABLED(SHA256) || IS_ENABLED(CONFIG_CMD_SHA1_ಮೊತ್ತ) || \
	CONFIG_IS_ENABLED(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_VERIFY) || IS_ENABLED(CONFIG_CMD_HASH) || \
	CONFIG_IS_ENABLED(SHA384) || CONFIG_IS_ENABLED(SHA512) || \
	IS_ENABLED(CONFIG_CMD_MD5_ಮೊತ್ತ) || CONFIG_IS_ENABLED(SM3)
#ವ್ಯಾಖ್ಯಾನಿಸು multi_hash()	1
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು multi_hash()	0
#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ hash_lookup_algo(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *algo_name, ರಚನೆ hash_algo **algop)
{
	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(hash_algo); i++) {
		ಆದರೆ (!strcmp(algo_name, hash_algo[i].name)) {
			*algop = &hash_algo[i];
			ಹಿಂತಿರುಗಿ 0;
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	ಹಿಂತಿರುಗಿ -EPROTONOSUPPORT;
}

ಪೂರ್ಣಾಂಕ hash_progressive_lookup_algo(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *algo_name,
				 ರಚನೆ hash_algo **algop)
{
	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(hash_algo); i++) {
		ಆದರೆ (!strcmp(algo_name, hash_algo[i].name)) {
			ಆದರೆ (hash_algo[i].hash_init) {
				*algop = &hash_algo[i];
				ಹಿಂತಿರುಗಿ 0;
			}
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	ಹಿಂತಿರುಗಿ -EPROTONOSUPPORT;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ USE_HOSTCC
ಪೂರ್ಣಾಂಕ hash_parse_string(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *algo_name, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, uint8_t *ಫಲಿತಾಂಶ)
{
	ರಚನೆ hash_algo *algo;
	ಪೂರ್ಣಾಂಕ ret;
	ಪೂರ್ಣಾಂಕ i;

	ret = hash_lookup_algo(algo_name, &algo);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;

	ಗಾಗಿ (i = 0; i < algo->digest_ಗಾತ್ರ; i++) {
		ಅಕ್ಷರ chr[3];

		strlcpy(chr, &str[i * 2], 3);
		ಫಲಿತಾಂಶ[i] = hextoul(chr, ಖಾಲಿ);
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ hash_ಖಂಡ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *algo_name, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ len,
	       uint8_t *output, ಪೂರ್ಣಾಂಕ *output_ಗಾತ್ರ)
{
	ರಚನೆ hash_algo *algo;
	ಪೂರ್ಣಾಂಕ ret;

	ret = hash_lookup_algo(algo_name, &algo);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;

	ಆದರೆ (output_ಗಾತ್ರ && *output_ಗಾತ್ರ < algo->digest_ಗಾತ್ರ) {
		debug("Output buffer size %d too small (need %d bytes)",
		      *output_ಗಾತ್ರ, algo->digest_ಗಾತ್ರ);
		ಹಿಂತಿರುಗಿ -ENOSPC;
	}
	ಆದರೆ (output_ಗಾತ್ರ)
		*output_ಗಾತ್ರ = algo->digest_ಗಾತ್ರ;
	algo->hash_func_ws(ದತ್ತಾಂಶ, len, output, algo->chunk_ಗಾತ್ರ);

	ಹಿಂತಿರುಗಿ 0;
}

#ಆದರೆ !defined(CONFIG_XPL_BUILD) && (defined(CONFIG_CMD_HASH) || \
	defined(CONFIG_CMD_SHA1_ಮೊತ್ತ) || defined(CONFIG_CMD_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32)) || \
	defined(CONFIG_CMD_MD5_ಮೊತ್ತ)
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
ಸ್ಥಿರ ಶೂನ್ಯ store_ಫಲಿತಾಂಶ(ರಚನೆ hash_algo *algo, ಸ್ಥಿರಾಂಕ uint8_t *ಮೊತ್ತ,
			 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dest, ಪೂರ್ಣಾಂಕ allow_env_vars)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
	ಪೂರ್ಣಾಂಕ env_var = 0;

	/*
	 * If environment variables are allowed, then we assume that 'dest'
	 * is an environment variable, unless it starts with *, in which
	 * case we assume it is an address. If not allowed, it is always an
	 * address. This is to support the crc32 command.
	 */
	ಆದರೆ (allow_env_vars) {
		ಆದರೆ (*dest == '*')
			dest++;
		ಇಲ್ಲದಿದ್ದರೆ
			env_var = 1;
	}

	ಆದರೆ (env_var) {
		ಅಕ್ಷರ str_output[HASH_MAX_DIGEST_ಗಾತ್ರ * 2 + 1];
		ಅಕ್ಷರ *str_ptr = str_output;

		ಗಾಗಿ (i = 0; i < algo->digest_ಗಾತ್ರ; i++) {
			sprintf(str_ptr, "%02x", ಮೊತ್ತ[i]);
			str_ptr += 2;
		}
		*str_ptr = '\0';
		env_ಗಣ(dest, str_output);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ulong addr;
		ಶೂನ್ಯ *buf;

		addr = hextoul(dest, ಖಾಲಿ);
		buf = map_sysmem(addr, algo->digest_ಗಾತ್ರ);
		ಸ್ಮರಣೆ_ನಕಲು(buf, ಮೊತ್ತ, algo->digest_ಗಾತ್ರ);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_verify_ಮೊತ್ತ(ರಚನೆ hash_algo *algo, ಅಕ್ಷರ *verify_str,
			    uint8_t *vsum, ಪೂರ್ಣಾಂಕ allow_env_vars)
{
	ಪೂರ್ಣಾಂಕ env_var = 0;

	/* See comment above in store_result() */
	ಆದರೆ (allow_env_vars) {
		ಆದರೆ (*verify_str == '*')
			verify_str++;
		ಇಲ್ಲದಿದ್ದರೆ
			env_var = 1;
	}

	ಆದರೆ (!env_var) {
		ulong addr;
		ಶೂನ್ಯ *buf;

		addr = hextoul(verify_str, ಖಾಲಿ);
		buf = map_sysmem(addr, algo->digest_ಗಾತ್ರ);
		ಸ್ಮರಣೆ_ನಕಲು(vsum, buf, algo->digest_ಗಾತ್ರ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಅಕ್ಷರ *vsum_str;
		ಪೂರ್ಣಾಂಕ digits = algo->digest_ಗಾತ್ರ * 2;

		/*
		 * As with the original code from sha1sum.c, we assume that a
		 * string which matches the digest size exactly is a hex
		 * string and not an environment variable.
		 */
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(verify_str) == digits)
			vsum_str = verify_str;
		ಇಲ್ಲದಿದ್ದರೆ {
			vsum_str = env_get(verify_str);
			ಆದರೆ (vsum_str == ಖಾಲಿ || ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(vsum_str) != digits) {
				ಬರೆ("Expected %d hex digits in env var\n",
				       digits);
				ಹಿಂತಿರುಗಿ 1;
			}
		}

		hash_parse_string(algo->name, vsum_str, vsum);
	}
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ hash_show(ರಚನೆ hash_algo *algo, ulong addr, ulong len, uint8_t *output)
{
	ಪೂರ್ಣಾಂಕ i;

	ಬರೆ("%s for %08lx ... %08lx ==> ", algo->name, addr, addr + len - 1);
	ಗಾಗಿ (i = 0; i < algo->digest_ಗಾತ್ರ; i++)
		ಬರೆ("%02x", output[i]);
}

ಪೂರ್ಣಾಂಕ hash_command(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *algo_name, ಪೂರ್ಣಾಂಕ flags, ರಚನೆ cmd_tbl *cmdtp,
		 ಪೂರ್ಣಾಂಕ flag, ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ argv[])
{
	ulong addr, len;

	ಆದರೆ ((argc < 2) || ((flags & HASH_FLAG_VERIFY) && (argc < 3)))
		ಹಿಂತಿರುಗಿ CMD_RET_USAGE;

	addr = hextoul(*argv++, ಖಾಲಿ);
	len = hextoul(*argv++, ಖಾಲಿ);

	ಆದರೆ (multi_hash()) {
		ರಚನೆ hash_algo *algo;
		u8 *output;
		uint8_t vsum[HASH_MAX_DIGEST_ಗಾತ್ರ];
		ಶೂನ್ಯ *buf;

		ಆದರೆ (hash_lookup_algo(algo_name, &algo)) {
			ಬರೆ("Unknown hash algorithm '%s'\n", algo_name);
			ಹಿಂತಿರುಗಿ CMD_RET_USAGE;
		}
		argc -= 2;

		ಆದರೆ (algo->digest_ಗಾತ್ರ > HASH_MAX_DIGEST_ಗಾತ್ರ) {
			puts("HASH_MAX_DIGEST_SIZE exceeded\n");
			ಹಿಂತಿರುಗಿ 1;
		}

		output = memalign(ARCH_DMA_MINALIGN,
				  ಗಾತ್ರ(uint32_t) * HASH_MAX_DIGEST_ಗಾತ್ರ);
		ಆದರೆ (!output)
			ಹಿಂತಿರುಗಿ CMD_RET_FAILURE;

		buf = map_sysmem(addr, len);
		ಆದರೆ (CONFIG_IS_ENABLED(DM_HASH)) {
			ಎಣಿಕೆ HASH_ALGO hash_algo;
			ಪೂರ್ಣಾಂಕ ret;

			hash_algo = hash_algo_lookup_by_name(algo_name);
			ಆದರೆ (hash_algo != HASH_ALGO_INVALID) {
				ret = hash_digest_wd_lookup(hash_algo, buf, len,
							    output,
							    algo->chunk_ಗಾತ್ರ);
				ಆದರೆ (ret && ret != -ENODEV && ret != -EOPNOTSUPP) {
					unmap_sysmem(buf);
					ಬಿಡುಗಡೆ(output);
					ಹಿಂತಿರುಗಿ CMD_RET_FAILURE;
				}
				ಆದರೆ (!ret)
					ಹೋಗು ಮುಗಿದ;
			}
		}
		algo->hash_func_ws(buf, len, output, algo->chunk_ಗಾತ್ರ);
ಮುಗಿದ:
		unmap_sysmem(buf);

		/* Try to avoid code bloat when verify is not needed */
#ಆದರೆ defined(CONFIG_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_VERIFY) || defined(CONFIG_SHA1_ಮೊತ್ತ_VERIFY) || \
	defined(CONFIG_MD5_ಮೊತ್ತ_VERIFY) || defined(CONFIG_HASH_VERIFY)
		ಆದರೆ (flags & HASH_FLAG_VERIFY) {
#ಇಲ್ಲದಿದ್ದರೆ
		ಆದರೆ (0) {
#ಆದರೆ_ಅಂತ್ಯ
			ಆದರೆ (parse_verify_ಮೊತ್ತ(algo, *argv, vsum,
					flags & HASH_FLAG_ENV)) {
				ಬರೆ("ERROR: %s does not contain a valid "
					"%s sum\n", *argv, algo->name);
				ಬಿಡುಗಡೆ(output);
				ಹಿಂತಿರುಗಿ 1;
			}
			ಆದರೆ (memcmp(output, vsum, algo->digest_ಗಾತ್ರ) != 0) {
				ಪೂರ್ಣಾಂಕ i;

				hash_show(algo, addr, len, output);
				ಬರೆ(" != ");
				ಗಾಗಿ (i = 0; i < algo->digest_ಗಾತ್ರ; i++)
					ಬರೆ("%02x", vsum[i]);
				puts(" ** ERROR **\n");
				ಬಿಡುಗಡೆ(output);
				ಹಿಂತಿರುಗಿ 1;
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			hash_show(algo, addr, len, output);
			ಬರೆ("\n");

			ಆದರೆ (argc) {
				store_ಫಲಿತಾಂಶ(algo, output, *argv,
					flags & HASH_FLAG_ENV);
			}
		}

		ಬಿಡುಗಡೆ(output);

	/* Horrible code size hack for boards that just want crc32 */
	} ಇಲ್ಲದಿದ್ದರೆ {
		ulong ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ;
		ulong *ptr;

		ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_wd(0, (ಸ್ಥಿರಾಂಕ uchar *)addr, len, CHUNKSZ_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32);

		ಬರೆ("CRC32 for %08lx ... %08lx ==> %08lx\n",
				addr, addr + len - 1, ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ);

		ಆದರೆ (argc >= 3) {
			ptr = (ulong *)hextoul(argv[0], ಖಾಲಿ);
			*ptr = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ;
		}
	}

	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_CMD_HASH || CONFIG_CMD_SHA1SUM || CONFIG_CMD_CRC32) */
#ಆದರೆ_ಅಂತ್ಯ /* !USE_HOSTCC */
