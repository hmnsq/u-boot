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
#অন্তর্ভাব <command.h>
#অন্তর্ভাব <dm.h>
#অন্তর্ভাব <env.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <malloc.h>
#অন্তর্ভাব <mapmem.h>
#অন্তর্ভাব <hw_sha.h>
#অন্তর্ভাব <asm/cache.h>
#অন্তর্ভাব <asm/global_data.h>
#অন্তর্ভাব <asm/io.h>
#অন্তর্ভাব <linux/errno.h>
#অন্তর্ভাব <u-boot/hash.h>
#অন্যথা
#অন্তর্ভাব "mkimage.h"
#অন্তর্ভাব <linux/compiler_attributes.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <linux/kconfig.h>
#endif /* !USE_HOSTCC*/

#অন্তর্ভাব <hash.h>
#অন্তর্ভাব <image.h>
#অন্তর্ভাব <u-boot/crc.h>
#অন্তর্ভাব <u-boot/sha1.h>
#অন্তর্ভাব <u-boot/sha256.h>
#অন্তর্ভাব <u-boot/sha512.h>
#অন্তর্ভাব <u-boot/md5.h>
#অন্তর্ভাব <u-boot/sm3.h>

স্থির পূর্ণাঙ্ক __maybe_unused hash_init_sha1(সংরচনা hash_algo *algo, শূন্য **ctxp)
{
	sha1_context *ctx = malloc(পরিমাণ(sha1_context));
	sha1_starts(ctx);
	*ctxp = ctx;
	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক __maybe_unused hash_update_sha1(সংরচনা hash_algo *algo, শূন্য *ctx,
					   ধ্রুব শূন্য *buf, unsigned পূর্ণাঙ্ক size,
					   পূর্ণাঙ্ক is_last)
{
	sha1_update((sha1_context *)ctx, buf, size);
	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক __maybe_unused hash_finish_sha1(সংরচনা hash_algo *algo, শূন্য *ctx,
					   শূন্য *dest_buf, পূর্ণাঙ্ক size)
{
	যদি (size < algo->digest_size)
		প্রত্যাগম -1;

	sha1_finish((sha1_context *)ctx, dest_buf);
	মুক্ত(ctx);
	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক __maybe_unused hash_init_sha256(সংরচনা hash_algo *algo, শূন্য **ctxp)
{
	sha256_context *ctx = malloc(পরিমাণ(sha256_context));
	sha256_starts(ctx);
	*ctxp = ctx;
	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক __maybe_unused hash_update_sha256(সংরচনা hash_algo *algo, শূন্য *ctx,
					     ধ্রুব শূন্য *buf, uint size,
					     পূর্ণাঙ্ক is_last)
{
	sha256_update((sha256_context *)ctx, buf, size);
	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক __maybe_unused hash_finish_sha256(সংরচনা hash_algo *algo, শূন্য *ctx,
					     শূন্য *dest_buf, পূর্ণাঙ্ক size)
{
	যদি (size < algo->digest_size)
		প্রত্যাগম -1;

	sha256_finish((sha256_context *)ctx, dest_buf);
	মুক্ত(ctx);
	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক __maybe_unused hash_init_sha384(সংরচনা hash_algo *algo, শূন্য **ctxp)
{
	sha512_context *ctx = malloc(পরিমাণ(sha512_context));
	sha384_starts(ctx);
	*ctxp = ctx;
	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক __maybe_unused hash_update_sha384(সংরচনা hash_algo *algo, শূন্য *ctx,
					     ধ্রুব শূন্য *buf, uint size,
					     পূর্ণাঙ্ক is_last)
{
	sha384_update((sha512_context *)ctx, buf, size);
	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক __maybe_unused hash_finish_sha384(সংরচনা hash_algo *algo, শূন্য *ctx,
					     শূন্য *dest_buf, পূর্ণাঙ্ক size)
{
	যদি (size < algo->digest_size)
		প্রত্যাগম -1;

	sha384_finish((sha512_context *)ctx, dest_buf);
	মুক্ত(ctx);
	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক __maybe_unused hash_init_sha512(সংরচনা hash_algo *algo, শূন্য **ctxp)
{
	sha512_context *ctx = malloc(পরিমাণ(sha512_context));
	sha512_starts(ctx);
	*ctxp = ctx;
	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক __maybe_unused hash_update_sha512(সংরচনা hash_algo *algo, শূন্য *ctx,
					     ধ্রুব শূন্য *buf, uint size,
					     পূর্ণাঙ্ক is_last)
{
	sha512_update((sha512_context *)ctx, buf, size);
	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক __maybe_unused hash_finish_sha512(সংরচনা hash_algo *algo, শূন্য *ctx,
					     শূন্য *dest_buf, পূর্ণাঙ্ক size)
{
	যদি (size < algo->digest_size)
		প্রত্যাগম -1;

	sha512_finish((sha512_context *)ctx, dest_buf);
	মুক্ত(ctx);
	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক __maybe_unused hash_init_sm3(সংরচনা hash_algo *algo, শূন্য **ctxp)
{
	সংরচনা sm3_context *ctx = malloc(পরিমাণ(সংরচনা sm3_context));

	sm3_init(ctx);
	*ctxp = ctx;
	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক __maybe_unused hash_update_sm3(সংরচনা hash_algo *algo, শূন্য *ctx,
					  ধ্রুব শূন্য *buf, uint size,
					  পূর্ণাঙ্ক is_last)
{
	sm3_update((সংরচনা sm3_context *)ctx, buf, size);
	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক __maybe_unused hash_finish_sm3(সংরচনা hash_algo *algo, শূন্য *ctx,
					  শূন্য *dest_buf, পূর্ণাঙ্ক size)
{
	যদি (size < algo->digest_size)
		প্রত্যাগম -1;

	sm3_final((সংরচনা sm3_context *)ctx, dest_buf);
	মুক্ত(ctx);
	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক __maybe_unused hash_init_crc16_ccitt(সংরচনা hash_algo *algo,
						শূন্য **ctxp)
{
	uint16_t *ctx = malloc(পরিমাণ(uint16_t));
	*ctx = 0;
	*ctxp = ctx;
	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক __maybe_unused hash_update_crc16_ccitt(সংরচনা hash_algo *algo,
						  শূন্য *ctx, ধ্রুব শূন্য *buf,
						  unsigned পূর্ণাঙ্ক size,
						  পূর্ণাঙ্ক is_last)
{
	*((uint16_t *)ctx) = crc16_ccitt(*((uint16_t *)ctx), buf, size);
	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক __maybe_unused hash_finish_crc16_ccitt(সংরচনা hash_algo *algo,
						  শূন্য *ctx, শূন্য *dest_buf,
						  পূর্ণাঙ্ক size)
{
	যদি (size < algo->digest_size)
		প্রত্যাগম -1;

	*((uint16_t *)dest_buf) = *((uint16_t *)ctx);
	মুক্ত(ctx);
	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক __maybe_unused hash_init_crc32(সংরচনা hash_algo *algo, শূন্য **ctxp)
{
	uint32_t *ctx = malloc(পরিমাণ(uint32_t));
	*ctx = 0;
	*ctxp = ctx;
	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক __maybe_unused hash_update_crc32(সংরচনা hash_algo *algo, শূন্য *ctx,
					    ধ্রুব শূন্য *buf, unsigned পূর্ণাঙ্ক size,
					    পূর্ণাঙ্ক is_last)
{
	*((uint32_t *)ctx) = crc32(*((uint32_t *)ctx), buf, size);
	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক __maybe_unused hash_finish_crc32(সংরচনা hash_algo *algo, শূন্য *ctx,
					    শূন্য *dest_buf, পূর্ণাঙ্ক size)
{
	যদি (size < algo->digest_size)
		প্রত্যাগম -1;

	*((uint32_t *)dest_buf) = *((uint32_t *)ctx);
	মুক্ত(ctx);
	প্রত্যাগম 0;
}

/*
 * These are the hash algorithms we support.  If we have hardware acceleration
 * is enable we will use that, otherwise a software version of the algorithm.
 * Note that algorithm names must be in lower case.
 */
স্থির সংরচনা hash_algo hash_algo[] = {
#যদি CONFIG_IS_ENABLED(MD5)
	{
		.name		= "md5",
		.digest_size	= MD5_SUM_LEN,
		.chunk_size	= CHUNKSZ_MD5,
		.hash_func_ws	= md5_wd,
	},
#endif
#যদি CONFIG_IS_ENABLED(SHA1)
	{
		.name		= "sha1",
		.digest_size	= SHA1_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA1,
#যদি CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha1,
#অন্যথা
		.hash_func_ws	= sha1_csum_wd,
#endif
#যদি CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#অন্যথা
		.hash_init	= hash_init_sha1,
		.hash_update	= hash_update_sha1,
		.hash_finish	= hash_finish_sha1,
#endif
	},
#endif
#যদি CONFIG_IS_ENABLED(SHA256)
	{
		.name		= "sha256",
		.digest_size	= SHA256_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA256,
#যদি CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha256,
#অন্যথা
		.hash_func_ws	= sha256_csum_wd,
#endif
#যদি CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#অন্যথা
		.hash_init	= hash_init_sha256,
		.hash_update	= hash_update_sha256,
		.hash_finish	= hash_finish_sha256,
#endif
	},
#endif
#যদি CONFIG_IS_ENABLED(SHA384)
	{
		.name		= "sha384",
		.digest_size	= SHA384_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA384,
#যদি CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha384,
#অন্যথা
		.hash_func_ws	= sha384_csum_wd,
#endif
#যদি CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#অন্যথা
		.hash_init	= hash_init_sha384,
		.hash_update	= hash_update_sha384,
		.hash_finish	= hash_finish_sha384,
#endif
	},
#endif
#যদি CONFIG_IS_ENABLED(SHA512)
	{
		.name		= "sha512",
		.digest_size	= SHA512_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA512,
#যদি CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha512,
#অন্যথা
		.hash_func_ws	= sha512_csum_wd,
#endif
#যদি CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#অন্যথা
		.hash_init	= hash_init_sha512,
		.hash_update	= hash_update_sha512,
		.hash_finish	= hash_finish_sha512,
#endif
	},
#endif
#যদি CONFIG_IS_ENABLED(SM3)
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
#যদি CONFIG_IS_ENABLED(CRC16)
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
#যদি CONFIG_IS_ENABLED(CRC8) && IS_ENABLED(CONFIG_HASH_CRC8)
	{
		.name		= "crc8",
		.digest_size	= 1,
		.chunk_size	= CHUNKSZ_CRC32,
		.hash_func_ws	= crc8_wd_buf,
	},
#endif
#যদি CONFIG_IS_ENABLED(CRC32)
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
#যদি CONFIG_IS_ENABLED(SHA256) || IS_ENABLED(CONFIG_CMD_SHA1SUM) || \
	CONFIG_IS_ENABLED(CRC32_VERIFY) || IS_ENABLED(CONFIG_CMD_HASH) || \
	CONFIG_IS_ENABLED(SHA384) || CONFIG_IS_ENABLED(SHA512) || \
	IS_ENABLED(CONFIG_CMD_MD5SUM) || CONFIG_IS_ENABLED(SM3)
#define multi_hash()	1
#অন্যথা
#define multi_hash()	0
#endif

পূর্ণাঙ্ক hash_lookup_algo(ধ্রুব বর্ণ *algo_name, সংরচনা hash_algo **algop)
{
	পূর্ণাঙ্ক i;

	প্রতি (i = 0; i < ARRAY_SIZE(hash_algo); i++) {
		যদি (!strcmp(algo_name, hash_algo[i].name)) {
			*algop = &hash_algo[i];
			প্রত্যাগম 0;
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	প্রত্যাগম -EPROTONOSUPPORT;
}

পূর্ণাঙ্ক hash_progressive_lookup_algo(ধ্রুব বর্ণ *algo_name,
				 সংরচনা hash_algo **algop)
{
	পূর্ণাঙ্ক i;

	প্রতি (i = 0; i < ARRAY_SIZE(hash_algo); i++) {
		যদি (!strcmp(algo_name, hash_algo[i].name)) {
			যদি (hash_algo[i].hash_init) {
				*algop = &hash_algo[i];
				প্রত্যাগম 0;
			}
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	প্রত্যাগম -EPROTONOSUPPORT;
}

#ifndef USE_HOSTCC
পূর্ণাঙ্ক hash_parse_string(ধ্রুব বর্ণ *algo_name, ধ্রুব বর্ণ *str, uint8_t *result)
{
	সংরচনা hash_algo *algo;
	পূর্ণাঙ্ক ret;
	পূর্ণাঙ্ক i;

	ret = hash_lookup_algo(algo_name, &algo);
	যদি (ret)
		প্রত্যাগম ret;

	প্রতি (i = 0; i < algo->digest_size; i++) {
		বর্ণ chr[3];

		strlcpy(chr, &str[i * 2], 3);
		result[i] = hextoul(chr, রিক্ত);
	}

	প্রত্যাগম 0;
}

পূর্ণাঙ্ক hash_block(ধ্রুব বর্ণ *algo_name, ধ্রুব শূন্য *data, unsigned পূর্ণাঙ্ক len,
	       uint8_t *output, পূর্ণাঙ্ক *output_size)
{
	সংরচনা hash_algo *algo;
	পূর্ণাঙ্ক ret;

	ret = hash_lookup_algo(algo_name, &algo);
	যদি (ret)
		প্রত্যাগম ret;

	যদি (output_size && *output_size < algo->digest_size) {
		debug("Output buffer size %d too small (need %d bytes)",
		      *output_size, algo->digest_size);
		প্রত্যাগম -ENOSPC;
	}
	যদি (output_size)
		*output_size = algo->digest_size;
	algo->hash_func_ws(data, len, output, algo->chunk_size);

	প্রত্যাগম 0;
}

#যদি !defined(CONFIG_XPL_BUILD) && (defined(CONFIG_CMD_HASH) || \
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
স্থির শূন্য store_result(সংরচনা hash_algo *algo, ধ্রুব uint8_t *sum,
			 ধ্রুব বর্ণ *dest, পূর্ণাঙ্ক allow_env_vars)
{
	unsigned পূর্ণাঙ্ক i;
	পূর্ণাঙ্ক env_var = 0;

	/*
	 * If environment variables are allowed, then we assume that 'dest'
	 * is an environment variable, unless it starts with *, in which
	 * case we assume it is an address. If not allowed, it is always an
	 * address. This is to support the crc32 command.
	 */
	যদি (allow_env_vars) {
		যদি (*dest == '*')
			dest++;
		অন্যথা
			env_var = 1;
	}

	যদি (env_var) {
		বর্ণ str_output[HASH_MAX_DIGEST_SIZE * 2 + 1];
		বর্ণ *str_ptr = str_output;

		প্রতি (i = 0; i < algo->digest_size; i++) {
			sprintf(str_ptr, "%02x", sum[i]);
			str_ptr += 2;
		}
		*str_ptr = '\0';
		env_set(dest, str_output);
	} অন্যথা {
		ulong addr;
		শূন্য *buf;

		addr = hextoul(dest, রিক্ত);
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
স্থির পূর্ণাঙ্ক parse_verify_sum(সংরচনা hash_algo *algo, বর্ণ *verify_str,
			    uint8_t *vsum, পূর্ণাঙ্ক allow_env_vars)
{
	পূর্ণাঙ্ক env_var = 0;

	/* See comment above in store_result() */
	যদি (allow_env_vars) {
		যদি (*verify_str == '*')
			verify_str++;
		অন্যথা
			env_var = 1;
	}

	যদি (!env_var) {
		ulong addr;
		শূন্য *buf;

		addr = hextoul(verify_str, রিক্ত);
		buf = map_sysmem(addr, algo->digest_size);
		memcpy(vsum, buf, algo->digest_size);
	} অন্যথা {
		বর্ণ *vsum_str;
		পূর্ণাঙ্ক digits = algo->digest_size * 2;

		/*
		 * As with the original code from sha1sum.c, we assume that a
		 * string which matches the digest size exactly is a hex
		 * string and not an environment variable.
		 */
		যদি (strlen(verify_str) == digits)
			vsum_str = verify_str;
		অন্যথা {
			vsum_str = env_get(verify_str);
			যদি (vsum_str == রিক্ত || strlen(vsum_str) != digits) {
				printf("Expected %d hex digits in env var\n",
				       digits);
				প্রত্যাগম 1;
			}
		}

		hash_parse_string(algo->name, vsum_str, vsum);
	}
	প্রত্যাগম 0;
}

স্থির শূন্য hash_show(সংরচনা hash_algo *algo, ulong addr, ulong len, uint8_t *output)
{
	পূর্ণাঙ্ক i;

	printf("%s for %08lx ... %08lx ==> ", algo->name, addr, addr + len - 1);
	প্রতি (i = 0; i < algo->digest_size; i++)
		printf("%02x", output[i]);
}

পূর্ণাঙ্ক hash_command(ধ্রুব বর্ণ *algo_name, পূর্ণাঙ্ক flags, সংরচনা cmd_tbl *cmdtp,
		 পূর্ণাঙ্ক flag, পূর্ণাঙ্ক argc, বর্ণ *ধ্রুব argv[])
{
	ulong addr, len;

	যদি ((argc < 2) || ((flags & HASH_FLAG_VERIFY) && (argc < 3)))
		প্রত্যাগম CMD_RET_USAGE;

	addr = hextoul(*argv++, রিক্ত);
	len = hextoul(*argv++, রিক্ত);

	যদি (multi_hash()) {
		সংরচনা hash_algo *algo;
		u8 *output;
		uint8_t vsum[HASH_MAX_DIGEST_SIZE];
		শূন্য *buf;

		যদি (hash_lookup_algo(algo_name, &algo)) {
			printf("Unknown hash algorithm '%s'\n", algo_name);
			প্রত্যাগম CMD_RET_USAGE;
		}
		argc -= 2;

		যদি (algo->digest_size > HASH_MAX_DIGEST_SIZE) {
			puts("HASH_MAX_DIGEST_SIZE exceeded\n");
			প্রত্যাগম 1;
		}

		output = memalign(ARCH_DMA_MINALIGN,
				  পরিমাণ(uint32_t) * HASH_MAX_DIGEST_SIZE);
		যদি (!output)
			প্রত্যাগম CMD_RET_FAILURE;

		buf = map_sysmem(addr, len);
		যদি (CONFIG_IS_ENABLED(DM_HASH)) {
			গণনা HASH_ALGO hash_algo;
			পূর্ণাঙ্ক ret;

			hash_algo = hash_algo_lookup_by_name(algo_name);
			যদি (hash_algo != HASH_ALGO_INVALID) {
				ret = hash_digest_wd_lookup(hash_algo, buf, len,
							    output,
							    algo->chunk_size);
				যদি (ret && ret != -ENODEV && ret != -EOPNOTSUPP) {
					unmap_sysmem(buf);
					মুক্ত(output);
					প্রত্যাগম CMD_RET_FAILURE;
				}
				যদি (!ret)
					গচ্ছ done;
			}
		}
		algo->hash_func_ws(buf, len, output, algo->chunk_size);
done:
		unmap_sysmem(buf);

		/* Try to avoid code bloat when verify is not needed */
#যদি defined(CONFIG_CRC32_VERIFY) || defined(CONFIG_SHA1SUM_VERIFY) || \
	defined(CONFIG_MD5SUM_VERIFY) || defined(CONFIG_HASH_VERIFY)
		যদি (flags & HASH_FLAG_VERIFY) {
#অন্যথা
		যদি (0) {
#endif
			যদি (parse_verify_sum(algo, *argv, vsum,
					flags & HASH_FLAG_ENV)) {
				printf("ERROR: %s does not contain a valid "
					"%s sum\n", *argv, algo->name);
				মুক্ত(output);
				প্রত্যাগম 1;
			}
			যদি (memcmp(output, vsum, algo->digest_size) != 0) {
				পূর্ণাঙ্ক i;

				hash_show(algo, addr, len, output);
				printf(" != ");
				প্রতি (i = 0; i < algo->digest_size; i++)
					printf("%02x", vsum[i]);
				puts(" ** ERROR **\n");
				মুক্ত(output);
				প্রত্যাগম 1;
			}
		} অন্যথা {
			hash_show(algo, addr, len, output);
			printf("\n");

			যদি (argc) {
				store_result(algo, output, *argv,
					flags & HASH_FLAG_ENV);
			}
		}

		মুক্ত(output);

	/* Horrible code size hack for boards that just want crc32 */
	} অন্যথা {
		ulong crc;
		ulong *ptr;

		crc = crc32_wd(0, (ধ্রুব uchar *)addr, len, CHUNKSZ_CRC32);

		printf("CRC32 for %08lx ... %08lx ==> %08lx\n",
				addr, addr + len - 1, crc);

		যদি (argc >= 3) {
			ptr = (ulong *)hextoul(argv[0], রিক্ত);
			*ptr = crc;
		}
	}

	প্রত্যাগম 0;
}
#endif /* CONFIG_CMD_HASH || CONFIG_CMD_SHA1SUM || CONFIG_CMD_CRC32) */
#endif /* !USE_HOSTCC */
