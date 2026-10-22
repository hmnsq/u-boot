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

স্থির পূর্ণসংখ্যা __maybe_unused hash_init_sha1(কাঠামো hash_algo *algo, শূন্য **ctxp)
{
	sha1_context *ctx = malloc(আকার(sha1_context));
	sha1_starts(ctx);
	*ctxp = ctx;
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা __maybe_unused hash_update_sha1(কাঠামো hash_algo *algo, শূন্য *ctx,
					   ধ্রুবক শূন্য *buf, unsigned পূর্ণসংখ্যা আকার_2,
					   পূর্ণসংখ্যা is_last)
{
	sha1_update((sha1_context *)ctx, buf, আকার_2);
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা __maybe_unused hash_finish_sha1(কাঠামো hash_algo *algo, শূন্য *ctx,
					   শূন্য *dest_buf, পূর্ণসংখ্যা আকার_2)
{
	যদি (আকার_2 < algo->digest_আকার)
		ফেরত -1;

	sha1_finish((sha1_context *)ctx, dest_buf);
	মুক্ত(ctx);
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা __maybe_unused hash_init_sha256(কাঠামো hash_algo *algo, শূন্য **ctxp)
{
	sha256_context *ctx = malloc(আকার(sha256_context));
	sha256_starts(ctx);
	*ctxp = ctx;
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা __maybe_unused hash_update_sha256(কাঠামো hash_algo *algo, শূন্য *ctx,
					     ধ্রুবক শূন্য *buf, uint আকার_2,
					     পূর্ণসংখ্যা is_last)
{
	sha256_update((sha256_context *)ctx, buf, আকার_2);
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা __maybe_unused hash_finish_sha256(কাঠামো hash_algo *algo, শূন্য *ctx,
					     শূন্য *dest_buf, পূর্ণসংখ্যা আকার_2)
{
	যদি (আকার_2 < algo->digest_আকার)
		ফেরত -1;

	sha256_finish((sha256_context *)ctx, dest_buf);
	মুক্ত(ctx);
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা __maybe_unused hash_init_sha384(কাঠামো hash_algo *algo, শূন্য **ctxp)
{
	sha512_context *ctx = malloc(আকার(sha512_context));
	sha384_starts(ctx);
	*ctxp = ctx;
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা __maybe_unused hash_update_sha384(কাঠামো hash_algo *algo, শূন্য *ctx,
					     ধ্রুবক শূন্য *buf, uint আকার_2,
					     পূর্ণসংখ্যা is_last)
{
	sha384_update((sha512_context *)ctx, buf, আকার_2);
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা __maybe_unused hash_finish_sha384(কাঠামো hash_algo *algo, শূন্য *ctx,
					     শূন্য *dest_buf, পূর্ণসংখ্যা আকার_2)
{
	যদি (আকার_2 < algo->digest_আকার)
		ফেরত -1;

	sha384_finish((sha512_context *)ctx, dest_buf);
	মুক্ত(ctx);
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা __maybe_unused hash_init_sha512(কাঠামো hash_algo *algo, শূন্য **ctxp)
{
	sha512_context *ctx = malloc(আকার(sha512_context));
	sha512_starts(ctx);
	*ctxp = ctx;
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা __maybe_unused hash_update_sha512(কাঠামো hash_algo *algo, শূন্য *ctx,
					     ধ্রুবক শূন্য *buf, uint আকার_2,
					     পূর্ণসংখ্যা is_last)
{
	sha512_update((sha512_context *)ctx, buf, আকার_2);
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা __maybe_unused hash_finish_sha512(কাঠামো hash_algo *algo, শূন্য *ctx,
					     শূন্য *dest_buf, পূর্ণসংখ্যা আকার_2)
{
	যদি (আকার_2 < algo->digest_আকার)
		ফেরত -1;

	sha512_finish((sha512_context *)ctx, dest_buf);
	মুক্ত(ctx);
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা __maybe_unused hash_init_sm3(কাঠামো hash_algo *algo, শূন্য **ctxp)
{
	কাঠামো sm3_context *ctx = malloc(আকার(কাঠামো sm3_context));

	sm3_init(ctx);
	*ctxp = ctx;
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা __maybe_unused hash_update_sm3(কাঠামো hash_algo *algo, শূন্য *ctx,
					  ধ্রুবক শূন্য *buf, uint আকার_2,
					  পূর্ণসংখ্যা is_last)
{
	sm3_update((কাঠামো sm3_context *)ctx, buf, আকার_2);
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা __maybe_unused hash_finish_sm3(কাঠামো hash_algo *algo, শূন্য *ctx,
					  শূন্য *dest_buf, পূর্ণসংখ্যা আকার_2)
{
	যদি (আকার_2 < algo->digest_আকার)
		ফেরত -1;

	sm3_final((কাঠামো sm3_context *)ctx, dest_buf);
	মুক্ত(ctx);
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা __maybe_unused hash_init_crc16_ccitt(কাঠামো hash_algo *algo,
						শূন্য **ctxp)
{
	uint16_t *ctx = malloc(আকার(uint16_t));
	*ctx = 0;
	*ctxp = ctx;
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা __maybe_unused hash_update_crc16_ccitt(কাঠামো hash_algo *algo,
						  শূন্য *ctx, ধ্রুবক শূন্য *buf,
						  unsigned পূর্ণসংখ্যা আকার_2,
						  পূর্ণসংখ্যা is_last)
{
	*((uint16_t *)ctx) = crc16_ccitt(*((uint16_t *)ctx), buf, আকার_2);
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা __maybe_unused hash_finish_crc16_ccitt(কাঠামো hash_algo *algo,
						  শূন্য *ctx, শূন্য *dest_buf,
						  পূর্ণসংখ্যা আকার_2)
{
	যদি (আকার_2 < algo->digest_আকার)
		ফেরত -1;

	*((uint16_t *)dest_buf) = *((uint16_t *)ctx);
	মুক্ত(ctx);
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা __maybe_unused hash_init_crc32(কাঠামো hash_algo *algo, শূন্য **ctxp)
{
	uint32_t *ctx = malloc(আকার(uint32_t));
	*ctx = 0;
	*ctxp = ctx;
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা __maybe_unused hash_update_crc32(কাঠামো hash_algo *algo, শূন্য *ctx,
					    ধ্রুবক শূন্য *buf, unsigned পূর্ণসংখ্যা আকার_2,
					    পূর্ণসংখ্যা is_last)
{
	*((uint32_t *)ctx) = crc32(*((uint32_t *)ctx), buf, আকার_2);
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা __maybe_unused hash_finish_crc32(কাঠামো hash_algo *algo, শূন্য *ctx,
					    শূন্য *dest_buf, পূর্ণসংখ্যা আকার_2)
{
	যদি (আকার_2 < algo->digest_আকার)
		ফেরত -1;

	*((uint32_t *)dest_buf) = *((uint32_t *)ctx);
	মুক্ত(ctx);
	ফেরত 0;
}

/*
 * These are the hash algorithms we support.  If we have hardware acceleration
 * is enable we will use that, otherwise a software version of the algorithm.
 * Note that algorithm names must be in lower case.
 */
স্থির কাঠামো hash_algo hash_algo[] = {
#যদি CONFIG_IS_ENABLED(MD5)
	{
		.name		= "md5",
		.digest_আকার	= MD5_যোগ_LEN,
		.chunk_আকার	= CHUNKSZ_MD5,
		.hash_func_ws	= md5_wd,
	},
#endif
#যদি CONFIG_IS_ENABLED(SHA1)
	{
		.name		= "sha1",
		.digest_আকার	= SHA1_যোগ_LEN,
		.chunk_আকার	= CHUNKSZ_SHA1,
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
		.digest_আকার	= SHA256_যোগ_LEN,
		.chunk_আকার	= CHUNKSZ_SHA256,
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
		.digest_আকার	= SHA384_যোগ_LEN,
		.chunk_আকার	= CHUNKSZ_SHA384,
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
		.digest_আকার	= SHA512_যোগ_LEN,
		.chunk_আকার	= CHUNKSZ_SHA512,
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
		.digest_আকার	= SM3_DIGEST_আকার,
		.chunk_আকার	= SM3_BLOCK_আকার,
		.hash_func_ws	= sm3_csum_wd,
		.hash_init	= hash_init_sm3,
		.hash_update	= hash_update_sm3,
		.hash_finish	= hash_finish_sm3,
	},
#endif
#যদি CONFIG_IS_ENABLED(CRC16)
	{
		.name		= "crc16-ccitt",
		.digest_আকার	= 2,
		.chunk_আকার	= CHUNKSZ,
		.hash_func_ws	= crc16_ccitt_wd_buf,
		.hash_init	= hash_init_crc16_ccitt,
		.hash_update	= hash_update_crc16_ccitt,
		.hash_finish	= hash_finish_crc16_ccitt,
	},
#endif
#যদি CONFIG_IS_ENABLED(CRC8) && IS_ENABLED(CONFIG_HASH_CRC8)
	{
		.name		= "crc8",
		.digest_আকার	= 1,
		.chunk_আকার	= CHUNKSZ_CRC32,
		.hash_func_ws	= crc8_wd_buf,
	},
#endif
#যদি CONFIG_IS_ENABLED(CRC32)
	{
		.name		= "crc32",
		.digest_আকার	= 4,
		.chunk_আকার	= CHUNKSZ_CRC32,
		.hash_func_ws	= crc32_wd_buf,
		.hash_init	= hash_init_crc32,
		.hash_update	= hash_update_crc32,
		.hash_finish	= hash_finish_crc32,
	},
#endif
};

/* Try to minimize code size for boards that don't want much hashing */
#যদি CONFIG_IS_ENABLED(SHA256) || IS_ENABLED(CONFIG_CMD_SHA1_যোগ) || \
	CONFIG_IS_ENABLED(CRC32_VERIFY) || IS_ENABLED(CONFIG_CMD_HASH) || \
	CONFIG_IS_ENABLED(SHA384) || CONFIG_IS_ENABLED(SHA512) || \
	IS_ENABLED(CONFIG_CMD_MD5_যোগ) || CONFIG_IS_ENABLED(SM3)
#define multi_hash()	1
#অন্যথা
#define multi_hash()	0
#endif

পূর্ণসংখ্যা hash_lookup_algo(ধ্রুবক অক্ষর *algo_name, কাঠামো hash_algo **algop)
{
	পূর্ণসংখ্যা i;

	জন্য (i = 0; i < ARRAY_আকার(hash_algo); i++) {
		যদি (!strcmp(algo_name, hash_algo[i].name)) {
			*algop = &hash_algo[i];
			ফেরত 0;
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	ফেরত -EPROTONOSUPPORT;
}

পূর্ণসংখ্যা hash_progressive_lookup_algo(ধ্রুবক অক্ষর *algo_name,
				 কাঠামো hash_algo **algop)
{
	পূর্ণসংখ্যা i;

	জন্য (i = 0; i < ARRAY_আকার(hash_algo); i++) {
		যদি (!strcmp(algo_name, hash_algo[i].name)) {
			যদি (hash_algo[i].hash_init) {
				*algop = &hash_algo[i];
				ফেরত 0;
			}
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	ফেরত -EPROTONOSUPPORT;
}

#ifndef USE_HOSTCC
পূর্ণসংখ্যা hash_parse_string(ধ্রুবক অক্ষর *algo_name, ধ্রুবক অক্ষর *str, uint8_t *ফলাফল)
{
	কাঠামো hash_algo *algo;
	পূর্ণসংখ্যা ret;
	পূর্ণসংখ্যা i;

	ret = hash_lookup_algo(algo_name, &algo);
	যদি (ret)
		ফেরত ret;

	জন্য (i = 0; i < algo->digest_আকার; i++) {
		অক্ষর chr[3];

		strlcpy(chr, &str[i * 2], 3);
		ফলাফল[i] = hextoul(chr, ফাঁকা);
	}

	ফেরত 0;
}

পূর্ণসংখ্যা hash_block(ধ্রুবক অক্ষর *algo_name, ধ্রুবক শূন্য *তথ্য, unsigned পূর্ণসংখ্যা len,
	       uint8_t *output, পূর্ণসংখ্যা *output_আকার)
{
	কাঠামো hash_algo *algo;
	পূর্ণসংখ্যা ret;

	ret = hash_lookup_algo(algo_name, &algo);
	যদি (ret)
		ফেরত ret;

	যদি (output_আকার && *output_আকার < algo->digest_আকার) {
		debug("Output buffer size %d too small (need %d bytes)",
		      *output_আকার, algo->digest_আকার);
		ফেরত -ENOSPC;
	}
	যদি (output_আকার)
		*output_আকার = algo->digest_আকার;
	algo->hash_func_ws(তথ্য, len, output, algo->chunk_আকার);

	ফেরত 0;
}

#যদি !defined(CONFIG_XPL_BUILD) && (defined(CONFIG_CMD_HASH) || \
	defined(CONFIG_CMD_SHA1_যোগ) || defined(CONFIG_CMD_CRC32)) || \
	defined(CONFIG_CMD_MD5_যোগ)
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
স্থির শূন্য store_ফলাফল(কাঠামো hash_algo *algo, ধ্রুবক uint8_t *যোগ,
			 ধ্রুবক অক্ষর *dest, পূর্ণসংখ্যা allow_env_vars)
{
	unsigned পূর্ণসংখ্যা i;
	পূর্ণসংখ্যা env_var = 0;

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
		অক্ষর str_output[HASH_MAX_DIGEST_আকার * 2 + 1];
		অক্ষর *str_ptr = str_output;

		জন্য (i = 0; i < algo->digest_আকার; i++) {
			sprintf(str_ptr, "%02x", যোগ[i]);
			str_ptr += 2;
		}
		*str_ptr = '\0';
		env_set(dest, str_output);
	} অন্যথা {
		ulong addr;
		শূন্য *buf;

		addr = hextoul(dest, ফাঁকা);
		buf = map_sysmem(addr, algo->digest_আকার);
		memcpy(buf, যোগ, algo->digest_আকার);
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
স্থির পূর্ণসংখ্যা parse_verify_যোগ(কাঠামো hash_algo *algo, অক্ষর *verify_str,
			    uint8_t *vsum, পূর্ণসংখ্যা allow_env_vars)
{
	পূর্ণসংখ্যা env_var = 0;

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

		addr = hextoul(verify_str, ফাঁকা);
		buf = map_sysmem(addr, algo->digest_আকার);
		memcpy(vsum, buf, algo->digest_আকার);
	} অন্যথা {
		অক্ষর *vsum_str;
		পূর্ণসংখ্যা digits = algo->digest_আকার * 2;

		/*
		 * As with the original code from sha1sum.c, we assume that a
		 * string which matches the digest size exactly is a hex
		 * string and not an environment variable.
		 */
		যদি (strlen(verify_str) == digits)
			vsum_str = verify_str;
		অন্যথা {
			vsum_str = env_get(verify_str);
			যদি (vsum_str == ফাঁকা || strlen(vsum_str) != digits) {
				printf("Expected %d hex digits in env var\n",
				       digits);
				ফেরত 1;
			}
		}

		hash_parse_string(algo->name, vsum_str, vsum);
	}
	ফেরত 0;
}

স্থির শূন্য hash_show(কাঠামো hash_algo *algo, ulong addr, ulong len, uint8_t *output)
{
	পূর্ণসংখ্যা i;

	printf("%s for %08lx ... %08lx ==> ", algo->name, addr, addr + len - 1);
	জন্য (i = 0; i < algo->digest_আকার; i++)
		printf("%02x", output[i]);
}

পূর্ণসংখ্যা hash_command(ধ্রুবক অক্ষর *algo_name, পূর্ণসংখ্যা flags, কাঠামো cmd_tbl *cmdtp,
		 পূর্ণসংখ্যা flag, পূর্ণসংখ্যা argc, অক্ষর *ধ্রুবক argv[])
{
	ulong addr, len;

	যদি ((argc < 2) || ((flags & HASH_FLAG_VERIFY) && (argc < 3)))
		ফেরত CMD_RET_USAGE;

	addr = hextoul(*argv++, ফাঁকা);
	len = hextoul(*argv++, ফাঁকা);

	যদি (multi_hash()) {
		কাঠামো hash_algo *algo;
		u8 *output;
		uint8_t vsum[HASH_MAX_DIGEST_আকার];
		শূন্য *buf;

		যদি (hash_lookup_algo(algo_name, &algo)) {
			printf("Unknown hash algorithm '%s'\n", algo_name);
			ফেরত CMD_RET_USAGE;
		}
		argc -= 2;

		যদি (algo->digest_আকার > HASH_MAX_DIGEST_আকার) {
			puts("HASH_MAX_DIGEST_SIZE exceeded\n");
			ফেরত 1;
		}

		output = memalign(ARCH_DMA_MINALIGN,
				  আকার(uint32_t) * HASH_MAX_DIGEST_আকার);
		যদি (!output)
			ফেরত CMD_RET_FAILURE;

		buf = map_sysmem(addr, len);
		যদি (CONFIG_IS_ENABLED(DM_HASH)) {
			গণনা HASH_ALGO hash_algo;
			পূর্ণসংখ্যা ret;

			hash_algo = hash_algo_lookup_by_name(algo_name);
			যদি (hash_algo != HASH_ALGO_INVALID) {
				ret = hash_digest_wd_lookup(hash_algo, buf, len,
							    output,
							    algo->chunk_আকার);
				যদি (ret && ret != -ENODEV && ret != -EOPNOTSUPP) {
					unmap_sysmem(buf);
					মুক্ত(output);
					ফেরত CMD_RET_FAILURE;
				}
				যদি (!ret)
					যাও done;
			}
		}
		algo->hash_func_ws(buf, len, output, algo->chunk_আকার);
done:
		unmap_sysmem(buf);

		/* Try to avoid code bloat when verify is not needed */
#যদি defined(CONFIG_CRC32_VERIFY) || defined(CONFIG_SHA1_যোগ_VERIFY) || \
	defined(CONFIG_MD5_যোগ_VERIFY) || defined(CONFIG_HASH_VERIFY)
		যদি (flags & HASH_FLAG_VERIFY) {
#অন্যথা
		যদি (0) {
#endif
			যদি (parse_verify_যোগ(algo, *argv, vsum,
					flags & HASH_FLAG_ENV)) {
				printf("ERROR: %s does not contain a valid "
					"%s sum\n", *argv, algo->name);
				মুক্ত(output);
				ফেরত 1;
			}
			যদি (memcmp(output, vsum, algo->digest_আকার) != 0) {
				পূর্ণসংখ্যা i;

				hash_show(algo, addr, len, output);
				printf(" != ");
				জন্য (i = 0; i < algo->digest_আকার; i++)
					printf("%02x", vsum[i]);
				puts(" ** ERROR **\n");
				মুক্ত(output);
				ফেরত 1;
			}
		} অন্যথা {
			hash_show(algo, addr, len, output);
			printf("\n");

			যদি (argc) {
				store_ফলাফল(algo, output, *argv,
					flags & HASH_FLAG_ENV);
			}
		}

		মুক্ত(output);

	/* Horrible code size hack for boards that just want crc32 */
	} অন্যথা {
		ulong crc;
		ulong *ptr;

		crc = crc32_wd(0, (ধ্রুবক uchar *)addr, len, CHUNKSZ_CRC32);

		printf("CRC32 for %08lx ... %08lx ==> %08lx\n",
				addr, addr + len - 1, crc);

		যদি (argc >= 3) {
			ptr = (ulong *)hextoul(argv[0], ফাঁকা);
			*ptr = crc;
		}
	}

	ফেরত 0;
}
#endif /* CONFIG_CMD_HASH || CONFIG_CMD_SHA1SUM || CONFIG_CMD_CRC32) */
#endif /* !USE_HOSTCC */
