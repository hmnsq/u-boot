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
#അന്തര്ഭാവ <command.h>
#അന്തര്ഭാവ <dm.h>
#അന്തര്ഭാവ <env.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <malloc.h>
#അന്തര്ഭാവ <mapmem.h>
#അന്തര്ഭാവ <hw_sha.h>
#അന്തര്ഭാവ <asm/cache.h>
#അന്തര്ഭാവ <asm/global_data.h>
#അന്തര്ഭാവ <asm/io.h>
#അന്തര്ഭാവ <linux/errno.h>
#അന്തര്ഭാവ <u-boot/hash.h>
#അല്ലെങ്കിൽ
#അന്തര്ഭാവ "mkimage.h"
#അന്തര്ഭാവ <linux/compiler_attributes.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <linux/kconfig.h>
#endif /* !USE_HOSTCC*/

#അന്തര്ഭാവ <hash.h>
#അന്തര്ഭാവ <image.h>
#അന്തര്ഭാവ <u-boot/crc.h>
#അന്തര്ഭാവ <u-boot/sha1.h>
#അന്തര്ഭാവ <u-boot/sha256.h>
#അന്തര്ഭാവ <u-boot/sha512.h>
#അന്തര്ഭാവ <u-boot/md5.h>
#അന്തര്ഭാവ <u-boot/sm3.h>

സ്ഥിതം പൂർണ്ണസംഖ്യ __maybe_unused hash_init_sha1(ഘടന hash_algo *algo, ശൂന്യം **ctxp)
{
	sha1_context *ctx = malloc(വലുപ്പം(sha1_context));
	sha1_starts(ctx);
	*ctxp = ctx;
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ __maybe_unused hash_update_sha1(ഘടന hash_algo *algo, ശൂന്യം *ctx,
					   സ്ഥിരം ശൂന്യം *buf, unsigned പൂർണ്ണസംഖ്യ size,
					   പൂർണ്ണസംഖ്യ is_last)
{
	sha1_update((sha1_context *)ctx, buf, size);
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ __maybe_unused hash_finish_sha1(ഘടന hash_algo *algo, ശൂന്യം *ctx,
					   ശൂന്യം *dest_buf, പൂർണ്ണസംഖ്യ size)
{
	എങ്കിൽ (size < algo->digest_size)
		തിരികെ -1;

	sha1_finish((sha1_context *)ctx, dest_buf);
	സ്വതന്ത്രം(ctx);
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ __maybe_unused hash_init_sha256(ഘടന hash_algo *algo, ശൂന്യം **ctxp)
{
	sha256_context *ctx = malloc(വലുപ്പം(sha256_context));
	sha256_starts(ctx);
	*ctxp = ctx;
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ __maybe_unused hash_update_sha256(ഘടന hash_algo *algo, ശൂന്യം *ctx,
					     സ്ഥിരം ശൂന്യം *buf, uint size,
					     പൂർണ്ണസംഖ്യ is_last)
{
	sha256_update((sha256_context *)ctx, buf, size);
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ __maybe_unused hash_finish_sha256(ഘടന hash_algo *algo, ശൂന്യം *ctx,
					     ശൂന്യം *dest_buf, പൂർണ്ണസംഖ്യ size)
{
	എങ്കിൽ (size < algo->digest_size)
		തിരികെ -1;

	sha256_finish((sha256_context *)ctx, dest_buf);
	സ്വതന്ത്രം(ctx);
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ __maybe_unused hash_init_sha384(ഘടന hash_algo *algo, ശൂന്യം **ctxp)
{
	sha512_context *ctx = malloc(വലുപ്പം(sha512_context));
	sha384_starts(ctx);
	*ctxp = ctx;
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ __maybe_unused hash_update_sha384(ഘടന hash_algo *algo, ശൂന്യം *ctx,
					     സ്ഥിരം ശൂന്യം *buf, uint size,
					     പൂർണ്ണസംഖ്യ is_last)
{
	sha384_update((sha512_context *)ctx, buf, size);
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ __maybe_unused hash_finish_sha384(ഘടന hash_algo *algo, ശൂന്യം *ctx,
					     ശൂന്യം *dest_buf, പൂർണ്ണസംഖ്യ size)
{
	എങ്കിൽ (size < algo->digest_size)
		തിരികെ -1;

	sha384_finish((sha512_context *)ctx, dest_buf);
	സ്വതന്ത്രം(ctx);
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ __maybe_unused hash_init_sha512(ഘടന hash_algo *algo, ശൂന്യം **ctxp)
{
	sha512_context *ctx = malloc(വലുപ്പം(sha512_context));
	sha512_starts(ctx);
	*ctxp = ctx;
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ __maybe_unused hash_update_sha512(ഘടന hash_algo *algo, ശൂന്യം *ctx,
					     സ്ഥിരം ശൂന്യം *buf, uint size,
					     പൂർണ്ണസംഖ്യ is_last)
{
	sha512_update((sha512_context *)ctx, buf, size);
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ __maybe_unused hash_finish_sha512(ഘടന hash_algo *algo, ശൂന്യം *ctx,
					     ശൂന്യം *dest_buf, പൂർണ്ണസംഖ്യ size)
{
	എങ്കിൽ (size < algo->digest_size)
		തിരികെ -1;

	sha512_finish((sha512_context *)ctx, dest_buf);
	സ്വതന്ത്രം(ctx);
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ __maybe_unused hash_init_sm3(ഘടന hash_algo *algo, ശൂന്യം **ctxp)
{
	ഘടന sm3_context *ctx = malloc(വലുപ്പം(ഘടന sm3_context));

	sm3_init(ctx);
	*ctxp = ctx;
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ __maybe_unused hash_update_sm3(ഘടന hash_algo *algo, ശൂന്യം *ctx,
					  സ്ഥിരം ശൂന്യം *buf, uint size,
					  പൂർണ്ണസംഖ്യ is_last)
{
	sm3_update((ഘടന sm3_context *)ctx, buf, size);
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ __maybe_unused hash_finish_sm3(ഘടന hash_algo *algo, ശൂന്യം *ctx,
					  ശൂന്യം *dest_buf, പൂർണ്ണസംഖ്യ size)
{
	എങ്കിൽ (size < algo->digest_size)
		തിരികെ -1;

	sm3_final((ഘടന sm3_context *)ctx, dest_buf);
	സ്വതന്ത്രം(ctx);
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ __maybe_unused hash_init_crc16_ccitt(ഘടന hash_algo *algo,
						ശൂന്യം **ctxp)
{
	uint16_t *ctx = malloc(വലുപ്പം(uint16_t));
	*ctx = 0;
	*ctxp = ctx;
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ __maybe_unused hash_update_crc16_ccitt(ഘടന hash_algo *algo,
						  ശൂന്യം *ctx, സ്ഥിരം ശൂന്യം *buf,
						  unsigned പൂർണ്ണസംഖ്യ size,
						  പൂർണ്ണസംഖ്യ is_last)
{
	*((uint16_t *)ctx) = crc16_ccitt(*((uint16_t *)ctx), buf, size);
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ __maybe_unused hash_finish_crc16_ccitt(ഘടന hash_algo *algo,
						  ശൂന്യം *ctx, ശൂന്യം *dest_buf,
						  പൂർണ്ണസംഖ്യ size)
{
	എങ്കിൽ (size < algo->digest_size)
		തിരികെ -1;

	*((uint16_t *)dest_buf) = *((uint16_t *)ctx);
	സ്വതന്ത്രം(ctx);
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ __maybe_unused hash_init_crc32(ഘടന hash_algo *algo, ശൂന്യം **ctxp)
{
	uint32_t *ctx = malloc(വലുപ്പം(uint32_t));
	*ctx = 0;
	*ctxp = ctx;
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ __maybe_unused hash_update_crc32(ഘടന hash_algo *algo, ശൂന്യം *ctx,
					    സ്ഥിരം ശൂന്യം *buf, unsigned പൂർണ്ണസംഖ്യ size,
					    പൂർണ്ണസംഖ്യ is_last)
{
	*((uint32_t *)ctx) = crc32(*((uint32_t *)ctx), buf, size);
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ __maybe_unused hash_finish_crc32(ഘടന hash_algo *algo, ശൂന്യം *ctx,
					    ശൂന്യം *dest_buf, പൂർണ്ണസംഖ്യ size)
{
	എങ്കിൽ (size < algo->digest_size)
		തിരികെ -1;

	*((uint32_t *)dest_buf) = *((uint32_t *)ctx);
	സ്വതന്ത്രം(ctx);
	തിരികെ 0;
}

/*
 * These are the hash algorithms we support.  If we have hardware acceleration
 * is enable we will use that, otherwise a software version of the algorithm.
 * Note that algorithm names must be in lower case.
 */
സ്ഥിതം ഘടന hash_algo hash_algo[] = {
#എങ്കിൽ CONFIG_IS_ENABLED(MD5)
	{
		.name		= "md5",
		.digest_size	= MD5_SUM_LEN,
		.chunk_size	= CHUNKSZ_MD5,
		.hash_func_ws	= md5_wd,
	},
#endif
#എങ്കിൽ CONFIG_IS_ENABLED(SHA1)
	{
		.name		= "sha1",
		.digest_size	= SHA1_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA1,
#എങ്കിൽ CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha1,
#അല്ലെങ്കിൽ
		.hash_func_ws	= sha1_csum_wd,
#endif
#എങ്കിൽ CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#അല്ലെങ്കിൽ
		.hash_init	= hash_init_sha1,
		.hash_update	= hash_update_sha1,
		.hash_finish	= hash_finish_sha1,
#endif
	},
#endif
#എങ്കിൽ CONFIG_IS_ENABLED(SHA256)
	{
		.name		= "sha256",
		.digest_size	= SHA256_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA256,
#എങ്കിൽ CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha256,
#അല്ലെങ്കിൽ
		.hash_func_ws	= sha256_csum_wd,
#endif
#എങ്കിൽ CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#അല്ലെങ്കിൽ
		.hash_init	= hash_init_sha256,
		.hash_update	= hash_update_sha256,
		.hash_finish	= hash_finish_sha256,
#endif
	},
#endif
#എങ്കിൽ CONFIG_IS_ENABLED(SHA384)
	{
		.name		= "sha384",
		.digest_size	= SHA384_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA384,
#എങ്കിൽ CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha384,
#അല്ലെങ്കിൽ
		.hash_func_ws	= sha384_csum_wd,
#endif
#എങ്കിൽ CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#അല്ലെങ്കിൽ
		.hash_init	= hash_init_sha384,
		.hash_update	= hash_update_sha384,
		.hash_finish	= hash_finish_sha384,
#endif
	},
#endif
#എങ്കിൽ CONFIG_IS_ENABLED(SHA512)
	{
		.name		= "sha512",
		.digest_size	= SHA512_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA512,
#എങ്കിൽ CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha512,
#അല്ലെങ്കിൽ
		.hash_func_ws	= sha512_csum_wd,
#endif
#എങ്കിൽ CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#അല്ലെങ്കിൽ
		.hash_init	= hash_init_sha512,
		.hash_update	= hash_update_sha512,
		.hash_finish	= hash_finish_sha512,
#endif
	},
#endif
#എങ്കിൽ CONFIG_IS_ENABLED(SM3)
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
#എങ്കിൽ CONFIG_IS_ENABLED(CRC16)
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
#എങ്കിൽ CONFIG_IS_ENABLED(CRC8) && IS_ENABLED(CONFIG_HASH_CRC8)
	{
		.name		= "crc8",
		.digest_size	= 1,
		.chunk_size	= CHUNKSZ_CRC32,
		.hash_func_ws	= crc8_wd_buf,
	},
#endif
#എങ്കിൽ CONFIG_IS_ENABLED(CRC32)
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
#എങ്കിൽ CONFIG_IS_ENABLED(SHA256) || IS_ENABLED(CONFIG_CMD_SHA1SUM) || \
	CONFIG_IS_ENABLED(CRC32_VERIFY) || IS_ENABLED(CONFIG_CMD_HASH) || \
	CONFIG_IS_ENABLED(SHA384) || CONFIG_IS_ENABLED(SHA512) || \
	IS_ENABLED(CONFIG_CMD_MD5SUM) || CONFIG_IS_ENABLED(SM3)
#define multi_hash()	1
#അല്ലെങ്കിൽ
#define multi_hash()	0
#endif

പൂർണ്ണസംഖ്യ hash_lookup_algo(സ്ഥിരം അക്ഷരം *algo_name, ഘടന hash_algo **algop)
{
	പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = 0; i < ARRAY_SIZE(hash_algo); i++) {
		എങ്കിൽ (!strcmp(algo_name, hash_algo[i].name)) {
			*algop = &hash_algo[i];
			തിരികെ 0;
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	തിരികെ -EPROTONOSUPPORT;
}

പൂർണ്ണസംഖ്യ hash_progressive_lookup_algo(സ്ഥിരം അക്ഷരം *algo_name,
				 ഘടന hash_algo **algop)
{
	പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = 0; i < ARRAY_SIZE(hash_algo); i++) {
		എങ്കിൽ (!strcmp(algo_name, hash_algo[i].name)) {
			എങ്കിൽ (hash_algo[i].hash_init) {
				*algop = &hash_algo[i];
				തിരികെ 0;
			}
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	തിരികെ -EPROTONOSUPPORT;
}

#ifndef USE_HOSTCC
പൂർണ്ണസംഖ്യ hash_parse_string(സ്ഥിരം അക്ഷരം *algo_name, സ്ഥിരം അക്ഷരം *str, uint8_t *result)
{
	ഘടന hash_algo *algo;
	പൂർണ്ണസംഖ്യ ret;
	പൂർണ്ണസംഖ്യ i;

	ret = hash_lookup_algo(algo_name, &algo);
	എങ്കിൽ (ret)
		തിരികെ ret;

	വേണ്ടി (i = 0; i < algo->digest_size; i++) {
		അക്ഷരം chr[3];

		strlcpy(chr, &str[i * 2], 3);
		result[i] = hextoul(chr, ശൂന്യ);
	}

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ hash_block(സ്ഥിരം അക്ഷരം *algo_name, സ്ഥിരം ശൂന്യം *data, unsigned പൂർണ്ണസംഖ്യ len,
	       uint8_t *output, പൂർണ്ണസംഖ്യ *output_size)
{
	ഘടന hash_algo *algo;
	പൂർണ്ണസംഖ്യ ret;

	ret = hash_lookup_algo(algo_name, &algo);
	എങ്കിൽ (ret)
		തിരികെ ret;

	എങ്കിൽ (output_size && *output_size < algo->digest_size) {
		debug("Output buffer size %d too small (need %d bytes)",
		      *output_size, algo->digest_size);
		തിരികെ -ENOSPC;
	}
	എങ്കിൽ (output_size)
		*output_size = algo->digest_size;
	algo->hash_func_ws(data, len, output, algo->chunk_size);

	തിരികെ 0;
}

#എങ്കിൽ !defined(CONFIG_XPL_BUILD) && (defined(CONFIG_CMD_HASH) || \
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
സ്ഥിതം ശൂന്യം store_result(ഘടന hash_algo *algo, സ്ഥിരം uint8_t *sum,
			 സ്ഥിരം അക്ഷരം *dest, പൂർണ്ണസംഖ്യ allow_env_vars)
{
	unsigned പൂർണ്ണസംഖ്യ i;
	പൂർണ്ണസംഖ്യ env_var = 0;

	/*
	 * If environment variables are allowed, then we assume that 'dest'
	 * is an environment variable, unless it starts with *, in which
	 * case we assume it is an address. If not allowed, it is always an
	 * address. This is to support the crc32 command.
	 */
	എങ്കിൽ (allow_env_vars) {
		എങ്കിൽ (*dest == '*')
			dest++;
		അല്ലെങ്കിൽ
			env_var = 1;
	}

	എങ്കിൽ (env_var) {
		അക്ഷരം str_output[HASH_MAX_DIGEST_SIZE * 2 + 1];
		അക്ഷരം *str_ptr = str_output;

		വേണ്ടി (i = 0; i < algo->digest_size; i++) {
			sprintf(str_ptr, "%02x", sum[i]);
			str_ptr += 2;
		}
		*str_ptr = '\0';
		env_set(dest, str_output);
	} അല്ലെങ്കിൽ {
		ulong addr;
		ശൂന്യം *buf;

		addr = hextoul(dest, ശൂന്യ);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ parse_verify_sum(ഘടന hash_algo *algo, അക്ഷരം *verify_str,
			    uint8_t *vsum, പൂർണ്ണസംഖ്യ allow_env_vars)
{
	പൂർണ്ണസംഖ്യ env_var = 0;

	/* See comment above in store_result() */
	എങ്കിൽ (allow_env_vars) {
		എങ്കിൽ (*verify_str == '*')
			verify_str++;
		അല്ലെങ്കിൽ
			env_var = 1;
	}

	എങ്കിൽ (!env_var) {
		ulong addr;
		ശൂന്യം *buf;

		addr = hextoul(verify_str, ശൂന്യ);
		buf = map_sysmem(addr, algo->digest_size);
		memcpy(vsum, buf, algo->digest_size);
	} അല്ലെങ്കിൽ {
		അക്ഷരം *vsum_str;
		പൂർണ്ണസംഖ്യ digits = algo->digest_size * 2;

		/*
		 * As with the original code from sha1sum.c, we assume that a
		 * string which matches the digest size exactly is a hex
		 * string and not an environment variable.
		 */
		എങ്കിൽ (strlen(verify_str) == digits)
			vsum_str = verify_str;
		അല്ലെങ്കിൽ {
			vsum_str = env_get(verify_str);
			എങ്കിൽ (vsum_str == ശൂന്യ || strlen(vsum_str) != digits) {
				printf("Expected %d hex digits in env var\n",
				       digits);
				തിരികെ 1;
			}
		}

		hash_parse_string(algo->name, vsum_str, vsum);
	}
	തിരികെ 0;
}

സ്ഥിതം ശൂന്യം hash_show(ഘടന hash_algo *algo, ulong addr, ulong len, uint8_t *output)
{
	പൂർണ്ണസംഖ്യ i;

	printf("%s for %08lx ... %08lx ==> ", algo->name, addr, addr + len - 1);
	വേണ്ടി (i = 0; i < algo->digest_size; i++)
		printf("%02x", output[i]);
}

പൂർണ്ണസംഖ്യ hash_command(സ്ഥിരം അക്ഷരം *algo_name, പൂർണ്ണസംഖ്യ flags, ഘടന cmd_tbl *cmdtp,
		 പൂർണ്ണസംഖ്യ flag, പൂർണ്ണസംഖ്യ argc, അക്ഷരം *സ്ഥിരം argv[])
{
	ulong addr, len;

	എങ്കിൽ ((argc < 2) || ((flags & HASH_FLAG_VERIFY) && (argc < 3)))
		തിരികെ CMD_RET_USAGE;

	addr = hextoul(*argv++, ശൂന്യ);
	len = hextoul(*argv++, ശൂന്യ);

	എങ്കിൽ (multi_hash()) {
		ഘടന hash_algo *algo;
		u8 *output;
		uint8_t vsum[HASH_MAX_DIGEST_SIZE];
		ശൂന്യം *buf;

		എങ്കിൽ (hash_lookup_algo(algo_name, &algo)) {
			printf("Unknown hash algorithm '%s'\n", algo_name);
			തിരികെ CMD_RET_USAGE;
		}
		argc -= 2;

		എങ്കിൽ (algo->digest_size > HASH_MAX_DIGEST_SIZE) {
			puts("HASH_MAX_DIGEST_SIZE exceeded\n");
			തിരികെ 1;
		}

		output = memalign(ARCH_DMA_MINALIGN,
				  വലുപ്പം(uint32_t) * HASH_MAX_DIGEST_SIZE);
		എങ്കിൽ (!output)
			തിരികെ CMD_RET_FAILURE;

		buf = map_sysmem(addr, len);
		എങ്കിൽ (CONFIG_IS_ENABLED(DM_HASH)) {
			എണ്ണൽ HASH_ALGO hash_algo;
			പൂർണ്ണസംഖ്യ ret;

			hash_algo = hash_algo_lookup_by_name(algo_name);
			എങ്കിൽ (hash_algo != HASH_ALGO_INVALID) {
				ret = hash_digest_wd_lookup(hash_algo, buf, len,
							    output,
							    algo->chunk_size);
				എങ്കിൽ (ret && ret != -ENODEV && ret != -EOPNOTSUPP) {
					unmap_sysmem(buf);
					സ്വതന്ത്രം(output);
					തിരികെ CMD_RET_FAILURE;
				}
				എങ്കിൽ (!ret)
					പോകുക done;
			}
		}
		algo->hash_func_ws(buf, len, output, algo->chunk_size);
done:
		unmap_sysmem(buf);

		/* Try to avoid code bloat when verify is not needed */
#എങ്കിൽ defined(CONFIG_CRC32_VERIFY) || defined(CONFIG_SHA1SUM_VERIFY) || \
	defined(CONFIG_MD5SUM_VERIFY) || defined(CONFIG_HASH_VERIFY)
		എങ്കിൽ (flags & HASH_FLAG_VERIFY) {
#അല്ലെങ്കിൽ
		എങ്കിൽ (0) {
#endif
			എങ്കിൽ (parse_verify_sum(algo, *argv, vsum,
					flags & HASH_FLAG_ENV)) {
				printf("ERROR: %s does not contain a valid "
					"%s sum\n", *argv, algo->name);
				സ്വതന്ത്രം(output);
				തിരികെ 1;
			}
			എങ്കിൽ (memcmp(output, vsum, algo->digest_size) != 0) {
				പൂർണ്ണസംഖ്യ i;

				hash_show(algo, addr, len, output);
				printf(" != ");
				വേണ്ടി (i = 0; i < algo->digest_size; i++)
					printf("%02x", vsum[i]);
				puts(" ** ERROR **\n");
				സ്വതന്ത്രം(output);
				തിരികെ 1;
			}
		} അല്ലെങ്കിൽ {
			hash_show(algo, addr, len, output);
			printf("\n");

			എങ്കിൽ (argc) {
				store_result(algo, output, *argv,
					flags & HASH_FLAG_ENV);
			}
		}

		സ്വതന്ത്രം(output);

	/* Horrible code size hack for boards that just want crc32 */
	} അല്ലെങ്കിൽ {
		ulong crc;
		ulong *ptr;

		crc = crc32_wd(0, (സ്ഥിരം uchar *)addr, len, CHUNKSZ_CRC32);

		printf("CRC32 for %08lx ... %08lx ==> %08lx\n",
				addr, addr + len - 1, crc);

		എങ്കിൽ (argc >= 3) {
			ptr = (ulong *)hextoul(argv[0], ശൂന്യ);
			*ptr = crc;
		}
	}

	തിരികെ 0;
}
#endif /* CONFIG_CMD_HASH || CONFIG_CMD_SHA1SUM || CONFIG_CMD_CRC32) */
#endif /* !USE_HOSTCC */
