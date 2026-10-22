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
#شامل <command.h>
#شامل <dm.h>
#شامل <env.h>
#شامل <log.h>
#شامل <malloc.h>
#شامل <mapmem.h>
#شامل <hw_sha.h>
#شامل <asm/cache.h>
#شامل <asm/global_data.h>
#شامل <asm/io.h>
#شامل <linux/errno.h>
#شامل <u-boot/hash.h>
#وگرنه
#شامل "mkimage.h"
#شامل <linux/compiler_attributes.h>
#شامل <time.h>
#شامل <linux/kconfig.h>
#endif /* !USE_HOSTCC*/

#شامل <hash.h>
#شامل <image.h>
#شامل <u-boot/crc.h>
#شامل <u-boot/sha1.h>
#شامل <u-boot/sha256.h>
#شامل <u-boot/sha512.h>
#شامل <u-boot/md5.h>
#شامل <u-boot/sm3.h>

ایستا صحیح __maybe_unused hash_init_sha1(ساختار hash_algo *algo, تهی **ctxp)
{
	sha1_context *ctx = malloc(اندازه(sha1_context));
	sha1_starts(ctx);
	*ctxp = ctx;
	بازگشت 0;
}

ایستا صحیح __maybe_unused hash_update_sha1(ساختار hash_algo *algo, تهی *ctx,
					   ثابت تهی *buf, unsigned صحیح size,
					   صحیح is_last)
{
	sha1_update((sha1_context *)ctx, buf, size);
	بازگشت 0;
}

ایستا صحیح __maybe_unused hash_finish_sha1(ساختار hash_algo *algo, تهی *ctx,
					   تهی *dest_buf, صحیح size)
{
	اگر (size < algo->digest_size)
		بازگشت -1;

	sha1_finish((sha1_context *)ctx, dest_buf);
	آزاد(ctx);
	بازگشت 0;
}

ایستا صحیح __maybe_unused hash_init_sha256(ساختار hash_algo *algo, تهی **ctxp)
{
	sha256_context *ctx = malloc(اندازه(sha256_context));
	sha256_starts(ctx);
	*ctxp = ctx;
	بازگشت 0;
}

ایستا صحیح __maybe_unused hash_update_sha256(ساختار hash_algo *algo, تهی *ctx,
					     ثابت تهی *buf, uint size,
					     صحیح is_last)
{
	sha256_update((sha256_context *)ctx, buf, size);
	بازگشت 0;
}

ایستا صحیح __maybe_unused hash_finish_sha256(ساختار hash_algo *algo, تهی *ctx,
					     تهی *dest_buf, صحیح size)
{
	اگر (size < algo->digest_size)
		بازگشت -1;

	sha256_finish((sha256_context *)ctx, dest_buf);
	آزاد(ctx);
	بازگشت 0;
}

ایستا صحیح __maybe_unused hash_init_sha384(ساختار hash_algo *algo, تهی **ctxp)
{
	sha512_context *ctx = malloc(اندازه(sha512_context));
	sha384_starts(ctx);
	*ctxp = ctx;
	بازگشت 0;
}

ایستا صحیح __maybe_unused hash_update_sha384(ساختار hash_algo *algo, تهی *ctx,
					     ثابت تهی *buf, uint size,
					     صحیح is_last)
{
	sha384_update((sha512_context *)ctx, buf, size);
	بازگشت 0;
}

ایستا صحیح __maybe_unused hash_finish_sha384(ساختار hash_algo *algo, تهی *ctx,
					     تهی *dest_buf, صحیح size)
{
	اگر (size < algo->digest_size)
		بازگشت -1;

	sha384_finish((sha512_context *)ctx, dest_buf);
	آزاد(ctx);
	بازگشت 0;
}

ایستا صحیح __maybe_unused hash_init_sha512(ساختار hash_algo *algo, تهی **ctxp)
{
	sha512_context *ctx = malloc(اندازه(sha512_context));
	sha512_starts(ctx);
	*ctxp = ctx;
	بازگشت 0;
}

ایستا صحیح __maybe_unused hash_update_sha512(ساختار hash_algo *algo, تهی *ctx,
					     ثابت تهی *buf, uint size,
					     صحیح is_last)
{
	sha512_update((sha512_context *)ctx, buf, size);
	بازگشت 0;
}

ایستا صحیح __maybe_unused hash_finish_sha512(ساختار hash_algo *algo, تهی *ctx,
					     تهی *dest_buf, صحیح size)
{
	اگر (size < algo->digest_size)
		بازگشت -1;

	sha512_finish((sha512_context *)ctx, dest_buf);
	آزاد(ctx);
	بازگشت 0;
}

ایستا صحیح __maybe_unused hash_init_sm3(ساختار hash_algo *algo, تهی **ctxp)
{
	ساختار sm3_context *ctx = malloc(اندازه(ساختار sm3_context));

	sm3_init(ctx);
	*ctxp = ctx;
	بازگشت 0;
}

ایستا صحیح __maybe_unused hash_update_sm3(ساختار hash_algo *algo, تهی *ctx,
					  ثابت تهی *buf, uint size,
					  صحیح is_last)
{
	sm3_update((ساختار sm3_context *)ctx, buf, size);
	بازگشت 0;
}

ایستا صحیح __maybe_unused hash_finish_sm3(ساختار hash_algo *algo, تهی *ctx,
					  تهی *dest_buf, صحیح size)
{
	اگر (size < algo->digest_size)
		بازگشت -1;

	sm3_final((ساختار sm3_context *)ctx, dest_buf);
	آزاد(ctx);
	بازگشت 0;
}

ایستا صحیح __maybe_unused hash_init_crc16_ccitt(ساختار hash_algo *algo,
						تهی **ctxp)
{
	uint16_t *ctx = malloc(اندازه(uint16_t));
	*ctx = 0;
	*ctxp = ctx;
	بازگشت 0;
}

ایستا صحیح __maybe_unused hash_update_crc16_ccitt(ساختار hash_algo *algo,
						  تهی *ctx, ثابت تهی *buf,
						  unsigned صحیح size,
						  صحیح is_last)
{
	*((uint16_t *)ctx) = crc16_ccitt(*((uint16_t *)ctx), buf, size);
	بازگشت 0;
}

ایستا صحیح __maybe_unused hash_finish_crc16_ccitt(ساختار hash_algo *algo,
						  تهی *ctx, تهی *dest_buf,
						  صحیح size)
{
	اگر (size < algo->digest_size)
		بازگشت -1;

	*((uint16_t *)dest_buf) = *((uint16_t *)ctx);
	آزاد(ctx);
	بازگشت 0;
}

ایستا صحیح __maybe_unused hash_init_crc32(ساختار hash_algo *algo, تهی **ctxp)
{
	uint32_t *ctx = malloc(اندازه(uint32_t));
	*ctx = 0;
	*ctxp = ctx;
	بازگشت 0;
}

ایستا صحیح __maybe_unused hash_update_crc32(ساختار hash_algo *algo, تهی *ctx,
					    ثابت تهی *buf, unsigned صحیح size,
					    صحیح is_last)
{
	*((uint32_t *)ctx) = crc32(*((uint32_t *)ctx), buf, size);
	بازگشت 0;
}

ایستا صحیح __maybe_unused hash_finish_crc32(ساختار hash_algo *algo, تهی *ctx,
					    تهی *dest_buf, صحیح size)
{
	اگر (size < algo->digest_size)
		بازگشت -1;

	*((uint32_t *)dest_buf) = *((uint32_t *)ctx);
	آزاد(ctx);
	بازگشت 0;
}

/*
 * These are the hash algorithms we support.  If we have hardware acceleration
 * is enable we will use that, otherwise a software version of the algorithm.
 * Note that algorithm names must be in lower case.
 */
ایستا ساختار hash_algo hash_algo[] = {
#اگر CONFIG_IS_ENABLED(MD5)
	{
		.name		= "md5",
		.digest_size	= MD5_SUM_LEN,
		.chunk_size	= CHUNKSZ_MD5,
		.hash_func_ws	= md5_wd,
	},
#endif
#اگر CONFIG_IS_ENABLED(SHA1)
	{
		.name		= "sha1",
		.digest_size	= SHA1_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA1,
#اگر CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha1,
#وگرنه
		.hash_func_ws	= sha1_csum_wd,
#endif
#اگر CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#وگرنه
		.hash_init	= hash_init_sha1,
		.hash_update	= hash_update_sha1,
		.hash_finish	= hash_finish_sha1,
#endif
	},
#endif
#اگر CONFIG_IS_ENABLED(SHA256)
	{
		.name		= "sha256",
		.digest_size	= SHA256_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA256,
#اگر CONFIG_IS_ENABLED(SHA_HW_ACCEL)
		.hash_func_ws	= hw_sha256,
#وگرنه
		.hash_func_ws	= sha256_csum_wd,
#endif
#اگر CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#وگرنه
		.hash_init	= hash_init_sha256,
		.hash_update	= hash_update_sha256,
		.hash_finish	= hash_finish_sha256,
#endif
	},
#endif
#اگر CONFIG_IS_ENABLED(SHA384)
	{
		.name		= "sha384",
		.digest_size	= SHA384_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA384,
#اگر CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha384,
#وگرنه
		.hash_func_ws	= sha384_csum_wd,
#endif
#اگر CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#وگرنه
		.hash_init	= hash_init_sha384,
		.hash_update	= hash_update_sha384,
		.hash_finish	= hash_finish_sha384,
#endif
	},
#endif
#اگر CONFIG_IS_ENABLED(SHA512)
	{
		.name		= "sha512",
		.digest_size	= SHA512_SUM_LEN,
		.chunk_size	= CHUNKSZ_SHA512,
#اگر CONFIG_IS_ENABLED(SHA512_HW_ACCEL)
		.hash_func_ws	= hw_sha512,
#وگرنه
		.hash_func_ws	= sha512_csum_wd,
#endif
#اگر CONFIG_IS_ENABLED(SHA512_HW_ACCEL) && CONFIG_IS_ENABLED(SHA_PROG_HW_ACCEL)
		.hash_init	= hw_sha_init,
		.hash_update	= hw_sha_update,
		.hash_finish	= hw_sha_finish,
#وگرنه
		.hash_init	= hash_init_sha512,
		.hash_update	= hash_update_sha512,
		.hash_finish	= hash_finish_sha512,
#endif
	},
#endif
#اگر CONFIG_IS_ENABLED(SM3)
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
#اگر CONFIG_IS_ENABLED(CRC16)
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
#اگر CONFIG_IS_ENABLED(CRC8) && IS_ENABLED(CONFIG_HASH_CRC8)
	{
		.name		= "crc8",
		.digest_size	= 1,
		.chunk_size	= CHUNKSZ_CRC32,
		.hash_func_ws	= crc8_wd_buf,
	},
#endif
#اگر CONFIG_IS_ENABLED(CRC32)
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
#اگر CONFIG_IS_ENABLED(SHA256) || IS_ENABLED(CONFIG_CMD_SHA1SUM) || \
	CONFIG_IS_ENABLED(CRC32_VERIFY) || IS_ENABLED(CONFIG_CMD_HASH) || \
	CONFIG_IS_ENABLED(SHA384) || CONFIG_IS_ENABLED(SHA512) || \
	IS_ENABLED(CONFIG_CMD_MD5SUM) || CONFIG_IS_ENABLED(SM3)
#define multi_hash()	1
#وگرنه
#define multi_hash()	0
#endif

صحیح hash_lookup_algo(ثابت نویسه *algo_name, ساختار hash_algo **algop)
{
	صحیح i;

	برای (i = 0; i < ARRAY_SIZE(hash_algo); i++) {
		اگر (!strcmp(algo_name, hash_algo[i].name)) {
			*algop = &hash_algo[i];
			بازگشت 0;
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	بازگشت -EPROTONOSUPPORT;
}

صحیح hash_progressive_lookup_algo(ثابت نویسه *algo_name,
				 ساختار hash_algo **algop)
{
	صحیح i;

	برای (i = 0; i < ARRAY_SIZE(hash_algo); i++) {
		اگر (!strcmp(algo_name, hash_algo[i].name)) {
			اگر (hash_algo[i].hash_init) {
				*algop = &hash_algo[i];
				بازگشت 0;
			}
		}
	}

	debug("Unknown hash algorithm '%s'\n", algo_name);
	بازگشت -EPROTONOSUPPORT;
}

#ifndef USE_HOSTCC
صحیح hash_parse_string(ثابت نویسه *algo_name, ثابت نویسه *str, uint8_t *result)
{
	ساختار hash_algo *algo;
	صحیح ret;
	صحیح i;

	ret = hash_lookup_algo(algo_name, &algo);
	اگر (ret)
		بازگشت ret;

	برای (i = 0; i < algo->digest_size; i++) {
		نویسه chr[3];

		strlcpy(chr, &str[i * 2], 3);
		result[i] = hextoul(chr, پوچ);
	}

	بازگشت 0;
}

صحیح hash_block(ثابت نویسه *algo_name, ثابت تهی *data, unsigned صحیح len,
	       uint8_t *output, صحیح *output_size)
{
	ساختار hash_algo *algo;
	صحیح ret;

	ret = hash_lookup_algo(algo_name, &algo);
	اگر (ret)
		بازگشت ret;

	اگر (output_size && *output_size < algo->digest_size) {
		debug("Output buffer size %d too small (need %d bytes)",
		      *output_size, algo->digest_size);
		بازگشت -ENOSPC;
	}
	اگر (output_size)
		*output_size = algo->digest_size;
	algo->hash_func_ws(data, len, output, algo->chunk_size);

	بازگشت 0;
}

#اگر !defined(CONFIG_XPL_BUILD) && (defined(CONFIG_CMD_HASH) || \
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
ایستا تهی store_result(ساختار hash_algo *algo, ثابت uint8_t *sum,
			 ثابت نویسه *dest, صحیح allow_env_vars)
{
	unsigned صحیح i;
	صحیح env_var = 0;

	/*
	 * If environment variables are allowed, then we assume that 'dest'
	 * is an environment variable, unless it starts with *, in which
	 * case we assume it is an address. If not allowed, it is always an
	 * address. This is to support the crc32 command.
	 */
	اگر (allow_env_vars) {
		اگر (*dest == '*')
			dest++;
		وگرنه
			env_var = 1;
	}

	اگر (env_var) {
		نویسه str_output[HASH_MAX_DIGEST_SIZE * 2 + 1];
		نویسه *str_ptr = str_output;

		برای (i = 0; i < algo->digest_size; i++) {
			sprintf(str_ptr, "%02x", sum[i]);
			str_ptr += 2;
		}
		*str_ptr = '\0';
		env_set(dest, str_output);
	} وگرنه {
		ulong addr;
		تهی *buf;

		addr = hextoul(dest, پوچ);
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
ایستا صحیح parse_verify_sum(ساختار hash_algo *algo, نویسه *verify_str,
			    uint8_t *vsum, صحیح allow_env_vars)
{
	صحیح env_var = 0;

	/* See comment above in store_result() */
	اگر (allow_env_vars) {
		اگر (*verify_str == '*')
			verify_str++;
		وگرنه
			env_var = 1;
	}

	اگر (!env_var) {
		ulong addr;
		تهی *buf;

		addr = hextoul(verify_str, پوچ);
		buf = map_sysmem(addr, algo->digest_size);
		memcpy(vsum, buf, algo->digest_size);
	} وگرنه {
		نویسه *vsum_str;
		صحیح digits = algo->digest_size * 2;

		/*
		 * As with the original code from sha1sum.c, we assume that a
		 * string which matches the digest size exactly is a hex
		 * string and not an environment variable.
		 */
		اگر (strlen(verify_str) == digits)
			vsum_str = verify_str;
		وگرنه {
			vsum_str = env_get(verify_str);
			اگر (vsum_str == پوچ || strlen(vsum_str) != digits) {
				printf("Expected %d hex digits in env var\n",
				       digits);
				بازگشت 1;
			}
		}

		hash_parse_string(algo->name, vsum_str, vsum);
	}
	بازگشت 0;
}

ایستا تهی hash_show(ساختار hash_algo *algo, ulong addr, ulong len, uint8_t *output)
{
	صحیح i;

	printf("%s for %08lx ... %08lx ==> ", algo->name, addr, addr + len - 1);
	برای (i = 0; i < algo->digest_size; i++)
		printf("%02x", output[i]);
}

صحیح hash_command(ثابت نویسه *algo_name, صحیح flags, ساختار cmd_tbl *cmdtp,
		 صحیح flag, صحیح argc, نویسه *ثابت argv[])
{
	ulong addr, len;

	اگر ((argc < 2) || ((flags & HASH_FLAG_VERIFY) && (argc < 3)))
		بازگشت CMD_RET_USAGE;

	addr = hextoul(*argv++, پوچ);
	len = hextoul(*argv++, پوچ);

	اگر (multi_hash()) {
		ساختار hash_algo *algo;
		u8 *output;
		uint8_t vsum[HASH_MAX_DIGEST_SIZE];
		تهی *buf;

		اگر (hash_lookup_algo(algo_name, &algo)) {
			printf("Unknown hash algorithm '%s'\n", algo_name);
			بازگشت CMD_RET_USAGE;
		}
		argc -= 2;

		اگر (algo->digest_size > HASH_MAX_DIGEST_SIZE) {
			puts("HASH_MAX_DIGEST_SIZE exceeded\n");
			بازگشت 1;
		}

		output = memalign(ARCH_DMA_MINALIGN,
				  اندازه(uint32_t) * HASH_MAX_DIGEST_SIZE);
		اگر (!output)
			بازگشت CMD_RET_FAILURE;

		buf = map_sysmem(addr, len);
		اگر (CONFIG_IS_ENABLED(DM_HASH)) {
			شمارش HASH_ALGO hash_algo;
			صحیح ret;

			hash_algo = hash_algo_lookup_by_name(algo_name);
			اگر (hash_algo != HASH_ALGO_INVALID) {
				ret = hash_digest_wd_lookup(hash_algo, buf, len,
							    output,
							    algo->chunk_size);
				اگر (ret && ret != -ENODEV && ret != -EOPNOTSUPP) {
					unmap_sysmem(buf);
					آزاد(output);
					بازگشت CMD_RET_FAILURE;
				}
				اگر (!ret)
					برو done;
			}
		}
		algo->hash_func_ws(buf, len, output, algo->chunk_size);
done:
		unmap_sysmem(buf);

		/* Try to avoid code bloat when verify is not needed */
#اگر defined(CONFIG_CRC32_VERIFY) || defined(CONFIG_SHA1SUM_VERIFY) || \
	defined(CONFIG_MD5SUM_VERIFY) || defined(CONFIG_HASH_VERIFY)
		اگر (flags & HASH_FLAG_VERIFY) {
#وگرنه
		اگر (0) {
#endif
			اگر (parse_verify_sum(algo, *argv, vsum,
					flags & HASH_FLAG_ENV)) {
				printf("ERROR: %s does not contain a valid "
					"%s sum\n", *argv, algo->name);
				آزاد(output);
				بازگشت 1;
			}
			اگر (memcmp(output, vsum, algo->digest_size) != 0) {
				صحیح i;

				hash_show(algo, addr, len, output);
				printf(" != ");
				برای (i = 0; i < algo->digest_size; i++)
					printf("%02x", vsum[i]);
				puts(" ** ERROR **\n");
				آزاد(output);
				بازگشت 1;
			}
		} وگرنه {
			hash_show(algo, addr, len, output);
			printf("\n");

			اگر (argc) {
				store_result(algo, output, *argv,
					flags & HASH_FLAG_ENV);
			}
		}

		آزاد(output);

	/* Horrible code size hack for boards that just want crc32 */
	} وگرنه {
		ulong crc;
		ulong *ptr;

		crc = crc32_wd(0, (ثابت uchar *)addr, len, CHUNKSZ_CRC32);

		printf("CRC32 for %08lx ... %08lx ==> %08lx\n",
				addr, addr + len - 1, crc);

		اگر (argc >= 3) {
			ptr = (ulong *)hextoul(argv[0], پوچ);
			*ptr = crc;
		}
	}

	بازگشت 0;
}
#endif /* CONFIG_CMD_HASH || CONFIG_CMD_SHA1SUM || CONFIG_CMD_CRC32) */
#endif /* !USE_HOSTCC */
