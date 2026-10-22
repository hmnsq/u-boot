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
#அந்தர்பாவ <command.h>
#அந்தர்பாவ <dm.h>
#அந்தர்பாவ <env.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <malloc.h>
#அந்தர்பாவ <mapmem.h>
#அந்தர்பாவ <hw_sha.h>
#அந்தர்பாவ <asm/cache.h>
#அந்தர்பாவ <asm/global_data.h>
#அந்தர்பாவ <asm/io.h>
#அந்தர்பாவ <linux/errno.h>
#அந்தர்பாவ <u-boot/hash.h>
#இல்லையெனில்
#அந்தர்பாவ "mkimage.h"
#அந்தர்பாவ <linux/compiler_attributes.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <linux/kconfig.h>
#endif /* !USE_HOSTCC*/

#அந்தர்பாவ <hash.h>
#அந்தர்பாவ <image.h>
#அந்தர்பாவ <u-boot/crc.h>
#அந்தர்பாவ <u-boot/sha1.h>
#அந்தர்பாவ <u-boot/sha256.h>
#அந்தர்பாவ <u-boot/sha512.h>
#அந்தர்பாவ <u-boot/md5.h>
#அந்தர்பாவ <u-boot/sm3.h>

நிலையான முழு __maybe_unused hash_init_sha1(கட்டமைப்பு hash_algo *algo, வெறுமை **ctxp)
{
	sha1_context *ctx = malloc(அளவு(sha1_context));
	sha1_starts(ctx);
	*ctxp = ctx;
	திரும்பு 0;
}

நிலையான முழு __maybe_unused hash_update_sha1(கட்டமைப்பு hash_algo *algo, வெறுமை *ctx,
					   மாறிலி வெறுமை *buf, unsigned முழு size,
					   முழு is_last)
{
	sha1_update((sha1_context *)ctx, buf, size);
	திரும்பு 0;
}

நிலையான முழு __maybe_unused hash_finish_sha1(கட்டமைப்பு hash_algo *algo, வெறுமை *ctx,
					   வெறுமை *dest_buf, முழு size)
{
	எனில் (size < algo->digest_size)
		திரும்பு -1;

	sha1_finish((sha1_context *)ctx, dest_buf);
	விடுவி(ctx);
	திரும்பு 0;
}

நிலையான முழு __maybe_unused hash_init_sha256(கட்டமைப்பு hash_algo *algo, வெறுமை **ctxp)
{
	sha256_context *ctx = malloc(அளவு(sha256_context));
	sha256_starts(ctx);
	*ctxp = ctx;
	திரும்பு 0;
}

நிலையான முழு __maybe_unused hash_update_sha256(கட்டமைப்பு hash_algo *algo, வெறுமை *ctx,
					     மாறிலி வெறுமை *buf, uint size,
					     முழு is_last)
{
	sha256_update((sha256_context *)ctx, buf, size);
	திரும்பு 0;
}

நிலையான முழு __maybe_unused hash_finish_sha256(கட்டமைப்பு hash_algo *algo, வெறுமை *ctx,
					     வெறுமை *dest_buf, முழு size)
{
	எனில் (size < algo->digest_size)
		திரும்பு -1;

	sha256_finish((sha256_context *)ctx, dest_buf);
	விடுவி(ctx);
	திரும்பு 0;
}

நிலையான முழு __maybe_unused hash_init_sha384(கட்டமைப்பு hash_algo *algo, வெறுமை **ctxp)
{
	sha512_context *ctx = malloc(அளவு(sha512_context));
	sha384_starts(ctx);
	*ctxp = ctx;
	திரும்பு 0;
}

நிலையான முழு __maybe_unused hash_update_sha384(கட்டமைப்பு hash_algo *algo, வெறுமை *ctx,
					     மாறிலி வெறுமை *buf, uint size,
					     முழு is_last)
{
	sha384_update((sha512_context *)ctx, buf, size);
	திரும்பு 0;
}

நிலையான முழு __maybe_unused hash_finish_sha384(கட்டமைப்பு hash_algo *algo, வெறுமை *ctx,
					     வெறுமை *dest_buf, முழு size)
{
	எனில் (size < algo->digest_size)
		திரும்பு -1;

	sha384_finish((sha512_context *)ctx, dest_buf);
	விடுவி(ctx);
	திரும்பு 0;
}

நிலையான முழு __maybe_unused hash_init_sha512(கட்டமைப்பு hash_algo *algo, வெறுமை **ctxp)
{
	sha512_context *ctx = malloc(அளவு(sha512_context));
	sha512_starts(ctx);
	*ctxp = ctx;
	திரும்பு 0;
}

நிலையான முழு __maybe_unused hash_update_sha512(கட்டமைப்பு hash_algo *algo, வெறுமை *ctx,
					     மாறிலி வெறுமை *buf, uint size,
					     முழு is_last)
{
	sha512_update((sha512_context *)ctx, buf, size);
	திரும்பு 0;
}

நிலையான முழு __maybe_unused hash_finish_sha512(கட்டமைப்பு hash_algo *algo, வெறுமை *ctx,
					     வெறுமை *dest_buf, முழு size)
{
	எனில் (size < algo->digest_size)
		திரும்பு -1;

	sha512_finish((sha512_context *)ctx, dest_buf);
	விடுவி(ctx);
	திரும்பு 0;
}

நிலையான முழு __maybe_unused hash_init_sm3(கட்டமைப்பு hash_algo *algo, வெறுமை **ctxp)
{
	கட்டமைப்பு sm3_context *ctx = malloc(அளவு(கட்டமைப்பு sm3_context));

	sm3_init(ctx);
	*ctxp = ctx;
	திரும்பு 0;
}

நிலையான முழு __maybe_unused hash_update_sm3(கட்டமைப்பு hash_algo *algo, வெறுமை *ctx,
					  மாறிலி வெறுமை *buf, uint size,
					  முழு is_last)
{
	sm3_update((கட்டமைப்பு sm3_context *)ctx, buf, size);
	திரும்பு 0;
}

நிலையான முழு __maybe_unused hash_finish_sm3(கட்டமைப்பு hash_algo *algo, வெறுமை *ctx,
					  வெறுமை *dest_buf, முழு size)
{
	எனில் (size < algo->digest_size)
		திரும்பு -1;

	sm3_final((கட்டமைப்பு sm3_context *)ctx, dest_buf);
	விடுவி(ctx);
	திரும்பு 0;
}

நிலையான முழு __maybe_unused hash_init_crc16_ccitt(கட்டமைப்பு hash_algo *algo,
						வெறுமை **ctxp)
{
	uint16_t *ctx = malloc(அளவு(uint16_t));
	*ctx = 0;
	*ctxp = ctx;
	திரும்பு 0;
}

நிலையான முழு __maybe_unused hash_update_crc16_ccitt(கட்டமைப்பு hash_algo *algo,
						  வெறுமை *ctx, மாறிலி வெறுமை *buf,
						  unsigned முழு size,
						  முழு is_last)
{
	*((uint16_t *)ctx) = crc16_ccitt(*((uint16_t *)ctx), buf, size);
	திரும்பு 0;
}

நிலையான முழு __maybe_unused hash_finish_crc16_ccitt(கட்டமைப்பு hash_algo *algo,
						  வெறுமை *ctx, வெறுமை *dest_buf,
						  முழு size)
{
	எனில் (size < algo->digest_size)
		திரும்பு -1;

	*((uint16_t *)dest_buf) = *((uint16_t *)ctx);
	விடுவி(ctx);
	திரும்பு 0;
}

நிலையான முழு __maybe_unused hash_init_crc32(கட்டமைப்பு hash_algo *algo, வெறுமை **ctxp)
{
	uint32_t *ctx = malloc(அளவு(uint32_t));
	*ctx = 0;
	*ctxp = ctx;
	திரும்பு 0;
}

நிலையான முழு __maybe_unused hash_update_crc32(கட்டமைப்பு hash_algo *algo, வெறுமை *ctx,
					    மாறிலி வெறுமை *buf, unsigned முழு size,
					    முழு is_last)
{
	*((uint32_t *)ctx) = crc32(*((uint32_t *)ctx), buf, size);
	திரும்பு 0;
}

நிலையான முழு __maybe_unused hash_finish_crc32(கட்டமைப்பு hash_algo *algo, வெறுமை *ctx,
					    வெறுமை *dest_buf, முழு size)
{
	எனில் (size < algo->digest_size)
		திரும்பு -1;

	*((uint32_t *)dest_buf) = *((uint32_t *)ctx);
	விடுவி(ctx);
	திரும்பு 0;
}

/*
 * These are the hash algorithms we support.  If we have hardware acceleration
 * is enable we will use that, otherwise a software version of the algorithm.
 * Note that algorithm names must be in lower case.
 */
நிலையான கட்டமைப்பு hash_algo hash_algo[] = {
#எனில் CONFIG_IS_ENABLED(MD5)
	{
		.name		= "md5",
		.digest_size	= MD5_SUM_LEN,
		.chunk_size	= CHUNKSZ_MD5,
		.hash_func_ws	= md5_wd,
	},
#endif
#எனில் CONFIG_IS_ENABLED(SHA1)
	{
		.name		= "sha1",
		.digest_size	= SHA1_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA1,
#எனில் CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha1,
#இல்லையெனில்
		.hash_func_ws	= sha1_csum_wd,
#endif
#எனில் CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#இல்லையெனில்
		.hash_init	= hash_init_sha1,
		.hash_update	= hash_update_sha1,
		.hash_finish	= hash_finish_sha1,
#endif
	},
#endif
#எனில் CONFIG_IS_ENABLED(SHA256)
	{
		.name		= "sha256",
		.digest_size	= SHA256_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA256,
#எனில் CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha256,
#இல்லையெனில்
		.hash_func_ws	= sha256_csum_wd,
#endif
#எனில் CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#இல்லையெனில்
		.hash_init	= hash_init_sha256,
		.hash_update	= hash_update_sha256,
		.hash_finish	= hash_finish_sha256,
#endif
	},
#endif
#எனில் CONFIG_IS_ENABLED(SHA384)
	{
		.name		= "sha384",
		.digest_size	= SHA384_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA384,
#எனில் CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha384,
#இல்லையெனில்
		.hash_func_ws	= sha384_csum_wd,
#endif
#எனில் CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#இல்லையெனில்
		.hash_init	= hash_init_sha384,
		.hash_update	= hash_update_sha384,
		.hash_finish	= hash_finish_sha384,
#endif
	},
#endif
#எனில் CONFIG_IS_ENABLED(SHA512)
	{
		.name		= "sha512",
		.digest_size	= SHA512_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA512,
#எனில் CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha512,
#இல்லையெனில்
		.hash_func_ws	= sha512_csum_wd,
#endif
#எனில் CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#இல்லையெனில்
		.hash_init	= hash_init_sha512,
		.hash_update	= hash_update_sha512,
		.hash_finish	= hash_finish_sha512,
#endif
	},
#endif
#எனில் CONFIG_IS_ENABLED(SM3)
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
#எனில் CONFIG_IS_ENABLED(CRC16)
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
#எனில் CONFIG_IS_ENABLED(CRC8) && IS_ENABLED(CONFIG_HASH_CRC8)
	{
		.name		= "crc8",
		.digest_size	= 1,
		.chunk_size	= CHUNKSZ_CRC32,
		.hash_func_ws	= crc8_wd_buf,
	},
#endif
#எனில் CONFIG_IS_ENABLED(CRC32)
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
#எனில் CONFIG_IS_ENABLED(SHA256) || IS_ENABLED(CONFIG_CMD_SHA1SUM) || \
	CONFIG_IS_ENABLED(CRC32_VERIFY) || IS_ENABLED(CONFIG_CMD_HASH) || \
	CONFIG_IS_ENABLED(SHA384) || CONFIG_IS_ENABLED(SHA512) || \
	IS_ENABLED(CONFIG_CMD_MD5SUM) || CONFIG_IS_ENABLED(SM3)
#define multi_hash()	1
#இல்லையெனில்
#define multi_hash()	0
#endif

முழு hash_lookup_algo(மாறிலி எழுத்து *algo_name, கட்டமைப்பு hash_algo **algop)
{
	முழு i;

	க்கு (i = 0; i < ARRAY_SIZE(hash_algo); i++) {
		எனில் (!strcmp(algo_name, hash_algo[i].name)) {
			*algop = &hash_algo[i];
			திரும்பு 0;
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	திரும்பு -EPROTONOSUPPORT;
}

முழு hash_progressive_lookup_algo(மாறிலி எழுத்து *algo_name,
				 கட்டமைப்பு hash_algo **algop)
{
	முழு i;

	க்கு (i = 0; i < ARRAY_SIZE(hash_algo); i++) {
		எனில் (!strcmp(algo_name, hash_algo[i].name)) {
			எனில் (hash_algo[i].hash_init) {
				*algop = &hash_algo[i];
				திரும்பு 0;
			}
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	திரும்பு -EPROTONOSUPPORT;
}

#ifndef USE_HOSTCC
முழு hash_parse_string(மாறிலி எழுத்து *algo_name, மாறிலி எழுத்து *str, uint8_t *result)
{
	கட்டமைப்பு hash_algo *algo;
	முழு ret;
	முழு i;

	ret = hash_lookup_algo(algo_name, &algo);
	எனில் (ret)
		திரும்பு ret;

	க்கு (i = 0; i < algo->digest_size; i++) {
		எழுத்து chr[3];

		strlcpy(chr, &str[i * 2], 3);
		result[i] = hextoul(chr, வெற்று);
	}

	திரும்பு 0;
}

முழு hash_block(மாறிலி எழுத்து *algo_name, மாறிலி வெறுமை *data, unsigned முழு len,
	       uint8_t *output, முழு *output_size)
{
	கட்டமைப்பு hash_algo *algo;
	முழு ret;

	ret = hash_lookup_algo(algo_name, &algo);
	எனில் (ret)
		திரும்பு ret;

	எனில் (output_size && *output_size < algo->digest_size) {
		debug("Output buffer size %d too small (need %d bytes)",
		      *output_size, algo->digest_size);
		திரும்பு -ENOSPC;
	}
	எனில் (output_size)
		*output_size = algo->digest_size;
	algo->hash_func_ws(data, len, output, algo->chunk_size);

	திரும்பு 0;
}

#எனில் !defined(CONFIG_XPL_BUILD) && (defined(CONFIG_CMD_HASH) || \
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
நிலையான வெறுமை store_result(கட்டமைப்பு hash_algo *algo, மாறிலி uint8_t *sum,
			 மாறிலி எழுத்து *dest, முழு allow_env_vars)
{
	unsigned முழு i;
	முழு env_var = 0;

	/*
	 * If environment variables are allowed, then we assume that 'dest'
	 * is an environment variable, unless it starts with *, in which
	 * case we assume it is an address. If not allowed, it is always an
	 * address. This is to support the crc32 command.
	 */
	எனில் (allow_env_vars) {
		எனில் (*dest == '*')
			dest++;
		இல்லையெனில்
			env_var = 1;
	}

	எனில் (env_var) {
		எழுத்து str_output[HASH_MAX_DIGEST_SIZE * 2 + 1];
		எழுத்து *str_ptr = str_output;

		க்கு (i = 0; i < algo->digest_size; i++) {
			sprintf(str_ptr, "%02x", sum[i]);
			str_ptr += 2;
		}
		*str_ptr = '\0';
		env_set(dest, str_output);
	} இல்லையெனில் {
		ulong addr;
		வெறுமை *buf;

		addr = hextoul(dest, வெற்று);
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
நிலையான முழு parse_verify_sum(கட்டமைப்பு hash_algo *algo, எழுத்து *verify_str,
			    uint8_t *vsum, முழு allow_env_vars)
{
	முழு env_var = 0;

	/* See comment above in store_result() */
	எனில் (allow_env_vars) {
		எனில் (*verify_str == '*')
			verify_str++;
		இல்லையெனில்
			env_var = 1;
	}

	எனில் (!env_var) {
		ulong addr;
		வெறுமை *buf;

		addr = hextoul(verify_str, வெற்று);
		buf = map_sysmem(addr, algo->digest_size);
		memcpy(vsum, buf, algo->digest_size);
	} இல்லையெனில் {
		எழுத்து *vsum_str;
		முழு digits = algo->digest_size * 2;

		/*
		 * As with the original code from sha1sum.c, we assume that a
		 * string which matches the digest size exactly is a hex
		 * string and not an environment variable.
		 */
		எனில் (strlen(verify_str) == digits)
			vsum_str = verify_str;
		இல்லையெனில் {
			vsum_str = env_get(verify_str);
			எனில் (vsum_str == வெற்று || strlen(vsum_str) != digits) {
				printf("Expected %d hex digits in env var\n",
				       digits);
				திரும்பு 1;
			}
		}

		hash_parse_string(algo->name, vsum_str, vsum);
	}
	திரும்பு 0;
}

நிலையான வெறுமை hash_show(கட்டமைப்பு hash_algo *algo, ulong addr, ulong len, uint8_t *output)
{
	முழு i;

	printf("%s for %08lx ... %08lx ==> ", algo->name, addr, addr + len - 1);
	க்கு (i = 0; i < algo->digest_size; i++)
		printf("%02x", output[i]);
}

முழு hash_command(மாறிலி எழுத்து *algo_name, முழு flags, கட்டமைப்பு cmd_tbl *cmdtp,
		 முழு flag, முழு argc, எழுத்து *மாறிலி argv[])
{
	ulong addr, len;

	எனில் ((argc < 2) || ((flags & HASH_FLAG_VERIFY) && (argc < 3)))
		திரும்பு CMD_RET_USAGE;

	addr = hextoul(*argv++, வெற்று);
	len = hextoul(*argv++, வெற்று);

	எனில் (multi_hash()) {
		கட்டமைப்பு hash_algo *algo;
		u8 *output;
		uint8_t vsum[HASH_MAX_DIGEST_SIZE];
		வெறுமை *buf;

		எனில் (hash_lookup_algo(algo_name, &algo)) {
			printf("Unknown hash algorithm '%s'\n", algo_name);
			திரும்பு CMD_RET_USAGE;
		}
		argc -= 2;

		எனில் (algo->digest_size > HASH_MAX_DIGEST_SIZE) {
			puts("HASH_MAX_DIGEST_SIZE exceeded\n");
			திரும்பு 1;
		}

		output = memalign(ARCH_DMA_MINALIGN,
				  அளவு(uint32_t) * HASH_MAX_DIGEST_SIZE);
		எனில் (!output)
			திரும்பு CMD_RET_FAILURE;

		buf = map_sysmem(addr, len);
		எனில் (CONFIG_IS_ENABLED(DM_HASH)) {
			எண்ணிக்கை HASH_ALGO hash_algo;
			முழு ret;

			hash_algo = hash_algo_lookup_by_name(algo_name);
			எனில் (hash_algo != HASH_ALGO_INVALID) {
				ret = hash_digest_wd_lookup(hash_algo, buf, len,
							    output,
							    algo->chunk_size);
				எனில் (ret && ret != -ENODEV && ret != -EOPNOTSUPP) {
					unmap_sysmem(buf);
					விடுவி(output);
					திரும்பு CMD_RET_FAILURE;
				}
				எனில் (!ret)
					செல் done;
			}
		}
		algo->hash_func_ws(buf, len, output, algo->chunk_size);
done:
		unmap_sysmem(buf);

		/* Try to avoid code bloat when verify is not needed */
#எனில் defined(CONFIG_CRC32_VERIFY) || defined(CONFIG_SHA1SUM_VERIFY) || \
	defined(CONFIG_MD5SUM_VERIFY) || defined(CONFIG_HASH_VERIFY)
		எனில் (flags & HASH_FLAG_VERIFY) {
#இல்லையெனில்
		எனில் (0) {
#endif
			எனில் (parse_verify_sum(algo, *argv, vsum,
					flags & HASH_FLAG_ENV)) {
				printf("ERROR: %s does not contain a valid "
					"%s sum\n", *argv, algo->name);
				விடுவி(output);
				திரும்பு 1;
			}
			எனில் (memcmp(output, vsum, algo->digest_size) != 0) {
				முழு i;

				hash_show(algo, addr, len, output);
				printf(" != ");
				க்கு (i = 0; i < algo->digest_size; i++)
					printf("%02x", vsum[i]);
				puts(" ** ERROR **\n");
				விடுவி(output);
				திரும்பு 1;
			}
		} இல்லையெனில் {
			hash_show(algo, addr, len, output);
			printf("\n");

			எனில் (argc) {
				store_result(algo, output, *argv,
					flags & HASH_FLAG_ENV);
			}
		}

		விடுவி(output);

	/* Horrible code size hack for boards that just want crc32 */
	} இல்லையெனில் {
		ulong crc;
		ulong *ptr;

		crc = crc32_wd(0, (மாறிலி uchar *)addr, len, CHUNKSZ_CRC32);

		printf("CRC32 for %08lx ... %08lx ==> %08lx\n",
				addr, addr + len - 1, crc);

		எனில் (argc >= 3) {
			ptr = (ulong *)hextoul(argv[0], வெற்று);
			*ptr = crc;
		}
	}

	திரும்பு 0;
}
#endif /* CONFIG_CMD_HASH || CONFIG_CMD_SHA1SUM || CONFIG_CMD_CRC32) */
#endif /* !USE_HOSTCC */
